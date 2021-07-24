/**
 * @file gs_gui.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <GLFW/glfw3.h>
#include "backend/imgui_impl_glfw.h"
#include "backend/imgui_impl_opengl2.h"
#include "imgui/imgui.h"
#include "implot/implot.h"
#include "gs_gui.hpp"

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

/// ScrollBuf Class
ScrollBuf::ScrollBuf()
{
    max_sz = 600;
    ofst = 0;
    data.reserve(max_sz);
}

ScrollBuf::ScrollBuf(int max_size)
{
    max_sz = max_size;
    ofst = 0;
    data.reserve(max_sz);
}

void ScrollBuf::AddPoint(float x, float y)
{
    if (data.size() < max_sz)
        data.push_back(ImVec2(x, y));
    else
    {
        data[ofst] = ImVec2(x, y);
        ofst = (ofst + 1) % max_sz;
    }
}

void ScrollBuf::Erase()
{
    if (data.size() > 0)
    {
        data.shrink(0);
        ofst = 0;
    }
}

float ScrollBuf::Max()
{
    float max = data[0].y;
    for (int i = 0; i < data.size(); i++)
        if (data[i].y > max)
            max = data[i].y;
    return max;
}

float ScrollBuf::Min()
{
    float min = data[0].y;
    for (int i = 0; i < data.size(); i++)
        if (data[i].y < min)
            min = data[i].y;
    return min;
}
/// ///

/// ACSRollingBuffer Class
ACSRollingBuffer::ACSRollingBuffer()
{
    x_index = 0;

    acs_upd_output_t dummy[1];
    memset(dummy, 0x0, sizeof(acs_upd_output_t));

    // Avoids a crash.
    addValueSet(*dummy);

    pthread_mutex_init(&acs_upd_inhibitor, NULL);
}

void ACSRollingBuffer::addValueSet(acs_upd_output_t data)
{
    ct.AddPoint(x_index, data.ct);
    mode.AddPoint(x_index, data.mode);
    bx.AddPoint(x_index, data.bx);
    by.AddPoint(x_index, data.by);
    bz.AddPoint(x_index, data.bz);
    wx.AddPoint(x_index, data.wx);
    wy.AddPoint(x_index, data.wy);
    wz.AddPoint(x_index, data.wz);
    sx.AddPoint(x_index, data.sx);
    sy.AddPoint(x_index, data.sy);
    sz.AddPoint(x_index, data.sz);
    vbatt.AddPoint(x_index, data.vbatt);
    vboost.AddPoint(x_index, data.vboost);
    cursun.AddPoint(x_index, data.cursun);
    cursys.AddPoint(x_index, data.cursys);

    x_index += 0.1;
}

ACSRollingBuffer::~ACSRollingBuffer()
{
    pthread_mutex_destroy(&acs_upd_inhibitor);
}
/// ///

/// ClientServerFrame Class
ClientServerFrame::ClientServerFrame(CLIENTSERVER_FRAME_TYPE type, int payload_size)
{
    if (type < 0)
    {
        printf("ClientServerFrame initialized with error type (%d).\n", (int)type);
        return;
    }

    if (payload_size > CLIENTSERVER_MAX_PAYLOAD_SIZE)
    {
        printf("Cannot allocate payload larger than %d bytes.\n", CLIENTSERVER_MAX_PAYLOAD_SIZE);
        return;
    }

    this->payload_size = payload_size;
    this->type = type;
    // TODO: Set the mode properly.
    mode = CS_MODE_ERROR; 
    crc1 = -1;
    crc2 = -1;
    guid = CLIENTSERVER_FRAME_GUID;
    termination = 0xAAAA;

    // payload = (unsigned char *)malloc(this->payload_size);
    memset(payload, 0x0, this->payload_size);
}

ClientServerFrame::~ClientServerFrame()
{
    free(payload);
}

int ClientServerFrame::storePayload(CLIENTSERVER_FRAME_ENDPOINT endpoint, void *data, int size)
{
    if (size > payload_size)
    {
        printf("Cannot store data of size larger than allocated payload size (%d > %d).\n", size, payload_size);
        return -1;
    }

    memcpy(payload, data, size);

    crc1 = crc16(payload, payload_size);
    crc2 = crc16(payload, payload_size);

    this->endpoint = endpoint;

    return 1;
}

int ClientServerFrame::retrievePayload(unsigned char *data_space, int size)
{
    if (size != payload_size)
    {
        printf("Data space size not equal to payload size (%d != %d).\n", size, payload_size);
        return -1;
    }

    memcpy(data_space, payload, payload_size);

    return 1;
}

int ClientServerFrame::getPayloadSize()
{
    return payload_size;
}

CLIENTSERVER_FRAME_TYPE ClientServerFrame::getType()
{
    return type;
}

int ClientServerFrame::checkIntegrity()
{
    if (guid != CLIENTSERVER_FRAME_GUID)
    {
        return -1;
    }
    else if (endpoint < 0)
    {
        return -2;
    }
    else if (mode < 0)
    {
        return -3;
    }
    else if (payload_size < 0 || payload_size > CLIENTSERVER_MAX_PAYLOAD_SIZE)
    {
        return -4;
    }
    else if (type < 0)
    {
        return -5;
    }
    else if (crc1 != crc2)
    {
        return -6;
    }
    else if (crc1 != crc16(payload, payload_size))
    {
        return -7;
    }
    else if (termination != 0xAAAA)
    {
        return -8;
    }
}

void ClientServerFrame::print()
{
    printf("GUID ------------ 0x%04x\n", guid);
    printf("Endpoint -------- %d\n", endpoint);
    printf("Mode ------------ %d\n", mode);
    printf("Payload Size ---- %d\n", payload_size);
    printf("Type ------------ %d\n", type);
    printf("CRC1 ------------ 0x%04x\n", crc1);
    printf("Payload ---- (HEX)");
    for(int i = 0; i < payload_size; i++)
    {
        printf(" 0x%04x", payload[i]);
    }
    printf("\n");
    printf("CRC2 ------------ 0x%04x\n", crc2);
    printf("Termination ----- 0x%04x\n", termination);
}

int ClientServerFrame::send()
{
    printf("Pretending to send the following:\n");
    print();

    // TODO: Make write_to_server(...) real.
    // write_to_server(this, sizeof(ClientServerFrame));

    return 1;
}
/// ///

/**
 * @brief 
 * 
 * From:
 * https://github.com/sunipkmukherjee/comic-mon/blob/master/guimain.cpp
 * with minor modifications.
 * 
 * @param socket 
 * @param address 
 * @param socket_size 
 * @param tout_s 
 * @return int 
 */
int connect_w_tout(int socket, const struct sockaddr *address, socklen_t socket_size, int tout_s)
{
    int res;
    long arg;
    fd_set myset;
    struct timeval tv;
    int valopt;
    socklen_t lon;

    // Set non-blocking.
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
    {
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
        return -1;
    }
    arg |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
        return -1;
    }
    // Trying to connect with timeout.
    res = connect(socket, address, socket_size);
    if (res < 0)
    {
        if (errno == EINPROGRESS)
        {
            fprintf(stderr, "EINPROGRESS in connect() - selecting\n");
            do
            {
                if (tout_s > 0)
                {
                    tv.tv_sec = tout_s;
                }
                else
                {
                    tv.tv_sec = 1; // Minimum 1 second.
                }
                tv.tv_usec = 0;
                FD_ZERO(&myset);
                FD_SET(socket, &myset);
                res = select(socket + 1, NULL, &myset, NULL, &tv);
                if (res < 0 && errno != EINTR)
                {
                    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
                    return -1;
                }
                else if (res > 0)
                {
                    // Socket selected for write.
                    lon = sizeof(int);
                    if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)(&valopt), &lon) < 0)
                    {
                        fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno));
                        return -1;
                    }

                    // Check the value returned...
                    if (valopt)
                    {
                        fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt));
                        return -1;
                    }
                    break;
                }
                else
                {
                    fprintf(stderr, "Timeout in select() - Cancelling!\n");
                    return -1;
                }
            } while (1);
        } 
        else
        {
            fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
            return -1;
        }
    }
    // Set to blocking mode again...
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
    {
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
        return -1;
    }
    arg &= (~O_NONBLOCK);
    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
        return -1;
    }
    return socket;
}

float getMin(float a, float b)
{
    if (a > b)
    {
        return b;
    }
    else
    {
        return a;
    }
}

float getMin(float a, float b, float c)
{
    if (a < b)
    {
        if (a < c)
        {
            return a;
        }
        else
        {
            return c;
        }
    }
    else
    {
        if (b < c)
        {
            return b;
        }
        else
        {
            return c;
        }
    }
}

float getMax(float a, float b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

float getMax(float a, float b, float c)
{
    if (a > b)
    {
        if (a > c)
        {
            return a;
        }
        else
        {
            return c;
        }
    }
    else
    {
        if (b > c)
        {
            return b;
        }
        else
        {
            return c;
        }
    }
}

void *gs_gui_check_password(void *auth)
{
    auth_t *lauth = (auth_t *)auth;
    lauth->busy = true;

    // Generate hash.
    // Check hash against valid hashes.
    // If valid, grant access.
    // Return access level granted.

    if (gs_helper(lauth->password) == -07723727136)
    {
        usleep(0.25 SEC);
        lauth->access_level = 1;
    }
    else if (gs_helper(lauth->password) == 013156200030)
    {
        usleep(0.25 SEC);
        lauth->access_level = 2;
    }
    else if (gs_helper(lauth->password) == 05657430216)
    {
        usleep(0.25 SEC);
        lauth->access_level = 3;
    }
    else
    {
        usleep(2.5 SEC);
        lauth->access_level = 0;
    }

    memset(lauth->password, 0x0, strlen(lauth->password));

    lauth->busy = false;
    return auth;
}

// int gs_gui_check_password(char* password)
// {
//     // Generate hash.
//     // Check hash against valid hashes.
//     // If valid, grant access.
//     // Return access level granted.
//     int retval = 0;
//     if (gs_helper((unsigned char *) password) == -07723727136)
//     {
//         retval = 1;
//     }
//     else if (gs_helper((unsigned char *) password) == 013156200030)
//     {
//         retval = 2;
//     }
//     else
//     {
//         retval = 0;
//     }

//     memset(password, 0x0, strlen(password));
//     return retval;
// }

// Mildly Obfuscated
int gs_helper(void *aa)
{
    char *a = (char *)aa;

    int b, c;
    unsigned int d, e, f;

    b = 0;
    e = (unsigned int)4294967295U;
    while (b[a] != 0)
    {
        d = b[a];
        e = e ^ d;
        for (c = 7; c >= 0; c--)
        {
            f = -(e & 1);
            e = (e >> 1) ^ ((unsigned int)3988292384U & f);
        }
        b = b + 1;
    }
    return ~e;
}

void *gs_acs_update_thread(void *vp)
{
    ACSRollingBuffer *acs_rolbuf = (ACSRollingBuffer *)vp;

    cmd_input_t acs_cmd[1];
    memset(acs_cmd, 0x0, sizeof(cmd_input_t));

    acs_cmd->mod = ACS_ID;
    acs_cmd->cmd = ACS_UPD_ID;
    acs_cmd->unused = 0x0;
    acs_cmd->data_size = 0x0;
    memset(acs_cmd->data, 0x0, MAX_DATA_SIZE);

    usleep(100000);

    // Transmit an ACS update request to the server.
    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, acs_cmd, sizeof(cmd_input_t));

    // Receive an ACS update from the server.
    gs_receive(acs_rolbuf);

    pthread_mutex_unlock(&acs_rolbuf->acs_upd_inhibitor);

    return vp;
}

int gs_transmit(CLIENTSERVER_FRAME_TYPE type, CLIENTSERVER_FRAME_ENDPOINT endpoint, void *data, int data_size)
{
    if (data_size < 0)
    {
        printf("Error: data_size is %d.\n", data_size);
        printf("Cancelling transmit.\n");
        return -1;
    }

    // Create a ClientServerFrame to send our data in.
    ClientServerFrame *clientserver_frame = new ClientServerFrame(type, data_size);
    clientserver_frame->storePayload(endpoint, data, data_size);

    clientserver_frame->send();

    return 1;
}

int gs_gui_gs2sh_tx_handler(auth_t *auth, cmd_input_t *command_input)
{
    ImGui::Text("Send Command");
    ImGui::Indent();
    if (auth->access_level > 1)
    {
        ImGui::Text("Queued Transmission");
        ImGui::Text("Module ID ----- 0x%02x", command_input->mod);
        ImGui::Text("Command ID ---- 0x%02x", command_input->cmd);
        ImGui::Text("Unused -------- 0x%02x", command_input->unused);
        ImGui::Text("Data size ----- 0x%02x", command_input->data_size);
        ImGui::Text("Data ---------- ");

        for (int i = 0; i < command_input->data_size; i++)
        {
            ImGui::SameLine(0.0F, 0.0F);
            ImGui::Text("%02x", command_input->data[i]);
        }

        ImGui::Unindent();
        if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
        {
            // Send the transmission.
            gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, command_input, sizeof(cmd_input_t));
        }
    }

    return 1;
}

// TODO: Update, looking at "void *rcv_thr(void *sock)" from line 338 of: https://github.com/sunipkmukherjee/comic-mon/blob/master/guimain.cpp
void *gs_rx_thread(void *args)
{
    // Convert the passed void pointer into something useful; in this case, a struct of whatever arguments gs_rx_thread(...) will need.
    // TODO: Create a rx_thread_args_t struct.
    global_data_t *global_data = (global_data_t *) args;

    int buffer_size = CLIENTSERVER_MAX_PAYLOAD_SIZE + 0x80;
    unsigned char buffer[buffer_size];

    while (global_data->rx_active)
    {
        printf("Beginning receive...\n");

        // TODO: Implement port reading.
        // int retval = read(buffer, buffer_size);
        // if (retval < 0)
        // {
        //     printf("Error while receiving.\n");
        // }
        // else if (retval == 0)
        // {
        //     printf("Received nothing.\n");
        // }

        // TODO: Parse the data.
        ClientServerFrame *clientserver_frame = (ClientServerFrame *) buffer;
        if (clientserver_frame->checkIntegrity() < 0)
        {
            printf("Integrity check failed.\n");
            continue;
        }
        printf("Integrity check successful.\n");

        int payload_size = clientserver_frame->getPayloadSize();
        unsigned char *payload = (unsigned char *)malloc(payload_size);
        if (clientserver_frame->retrievePayload(payload, payload_size) < 0)
        {
            printf("Error retrieving data.");
            continue;
        }

        CLIENTSERVER_FRAME_TYPE type = clientserver_frame->getType();
        // TODO: Based on what we got, set things to display the data.
        switch (type)
        {
        case CS_TYPE_NULL:
        {
            printf("Received NULL frame.\n");
            break;
        }
        case CS_TYPE_ACK:
        case CS_TYPE_NACK:
        {
            // cs_ack_t *cs_ack = (cs_ack_t *) payload;
            memcpy(global_data->cs_ack, payload, payload_size);
            break;
        }
        case CS_TYPE_CONFIG:
        {
            break;
        }
        case CS_TYPE_DATA: // Data type is just cmd_output_t (SH->GS)
        {
            // // TODO: Remove this block once debugging is complete.
            // cmd_output_t *cmd_output = (cmd_output_t *) payload;
            // cmd_output->

            memcpy(global_data->cmd_output, payload, payload_size);
            break;
        }
        case CS_TYPE_STATUS:
        {
            cs_status_t *status = (cs_status_t *)payload;

            break;
        }
        case CS_TYPE_ERROR:
        default:
        {
            break;
        }
        }
    }

    return;
}

// // This needs to act similarly to the cmd_parser.
// // TODO: If the data is from ACS update, be sure to set the values in the ACS Update data class!
int gs_receive(ACSRollingBuffer *acs_rolbuf)
{
//     client_frame_t output[1];
//     memset(output, 0x0, sizeof(client_frame_t));

//     // TODO: Read in data to output.
//     // receive into 'output'

//     // This function receives some data, which we assume fits as a client_frame_t, called output.

//     // TODO: REMOVE this code block once debug testing is complete.
//     output->guid = CLIENT_FRAME_GUID;
//     output->crc1 = crc16(output->payload, SIZE_FRAME_PAYLOAD);
//     output->crc2 = crc16(output->payload, SIZE_FRAME_PAYLOAD);

//     if (output->guid != CLIENT_FRAME_GUID)
//     {
//         printf("GUID Error: 0x%04x\n", output->guid);
//         return -1;
//     }
//     else if (output->crc1 != output->crc2)
//     {
//         printf("CRC Error: 0x%04x != 0x%04x\n", output->crc1, output->crc2);
//         return -2;
//     }
//     else if (output->crc1 != crc16(output->payload, SIZE_FRAME_PAYLOAD))
//     {
//         printf("CRC Error: 0x%04x != 0x%04x\n", output->crc1, crc16(output->payload, SIZE_FRAME_PAYLOAD));
//         return -3;
//     }

//     cmd_output_t *payload = (cmd_output_t *)output->payload;

//     // TODO: REMOVE this code block once debug testing is complete.
//     payload->mod = ACS_UPD_ID;

//     // All data received goes one of two places: the ACS Update Data Display window, or the plaintext trash heap window.
//     if (payload->mod == ACS_UPD_ID)
//     {
//         // Set the data in acs_display_data to the data in output->payload.
//         acs_upd_output_t *acs_upd_output = (acs_upd_output_t *)payload->data;

//         // TODO: REMOVE this block of code once debug testing is complete.
//         acs_upd_output->ct = rand() % 0xf;
//         acs_upd_output->mode = rand() % 0xf;
//         acs_upd_output->bx = rand() % 0xffff;
//         acs_upd_output->by = rand() % 0xffff;
//         acs_upd_output->bz = rand() % 0xffff;
//         acs_upd_output->wx = rand() % 0xffff;
//         acs_upd_output->wy = rand() % 0xffff;
//         acs_upd_output->wz = rand() % 0xffff;
//         acs_upd_output->sx = rand() % 0xffff;
//         acs_upd_output->sy = rand() % 0xffff;
//         acs_upd_output->sz = rand() % 0xffff;
//         acs_upd_output->vbatt = rand() % 0xff;
//         acs_upd_output->vboost = rand() % 0xff;
//         acs_upd_output->cursun = rand() % 0xff;
//         acs_upd_output->cursys = rand() % 0xff;

//         // Add the ACS update data
//         acs_rolbuf->addValueSet(*acs_upd_output);

//         // Set the ready flag.
//         // acs_display_data->ready = true;
//     }
//     else
//     {
//         // TODO: Handle all other data by printing it out into a single window. This should probably be handled with another local-global like acs_display_data.
//     }

    return 1;
}

void gs_gui_authentication_control_panel_window(bool *AUTH_control_panel, auth_t *auth)
{
    static pthread_t auth_thread_id;

    if (ImGui::Begin("Authentication Control Panel", AUTH_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (auth->busy)
        {
            ImGui::Text("PROCESSING...");
            ImGui::InputTextWithHint("", "Enter Password", auth->password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
        }
        else
        {
            switch (auth->access_level)
            {
            case 0:
                ImGui::Text("LOW LEVEL ACCESS");
                break;
            case 1:
                ImGui::Text("TEAM MEMBER ACCESS GRANTED");
                break;
            case 2:
                ImGui::Text("PRIORITY ACCESS GRANTED");
                break;
            case 3:
                ImGui::Text("PROJECT MANAGER ACCESS GRANTED");
                break;
            default:
                ImGui::Text("ERROR: UNKNOWN ACCESS");
                break;
            }

            if (ImGui::InputTextWithHint("", "Enter Password", auth->password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                pthread_create(&auth_thread_id, NULL, gs_gui_check_password, auth);
            }
        }

        if (ImGui::Button("DEAUTHENTICATE"))
        {
            auth->access_level = 0;
        }
    }
    ImGui::End();
}

// TODO: Add functionality.
void gs_gui_settings_window(bool *SETTINGS_window, auth_t *auth)
{
    if (ImGui::Begin("Settings", SETTINGS_window))
    {
        // static int test1 = 0, test2 = 0, test3 = 0;
        // ImGui::Columns(3, "first_column_set", true);
        // ImGui::InputInt("test1", &test1);
        // ImGui::NextColumn();
        // ImGui::InputInt("test2", &test2);
        // ImGui::NextColumn();
        // ImGui::InputInt("test3", &test3);
    }
    ImGui::End();
}

void gs_gui_acs_window(bool *ACS_window, auth_t *auth, ACSRollingBuffer *acs_rolbuf, bool *allow_transmission)
{
    static int ACS_command = ACS_INVALID_ID;
    static cmd_input_t ACS_command_input = {.mod = INVALID_ID, .cmd = ACS_INVALID_ID, .unused = 0, .data_size = 0};
    static acs_set_data_t acs_set_data = {0};
    static acs_set_data_holder_t acs_set_data_holder = {0};

    static bool acs_rxtx_automated = false;
    static pthread_t acs_thread_id;

    if (ImGui::Begin("ACS Operations", ACS_window))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {

            if (auth->access_level > 0)
            {

                if (ImGui::ArrowButton("get_moi_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_MOI;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get MOI");

                if (ImGui::ArrowButton("get_imoi_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_IMOI;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get IMOI");

                if (ImGui::ArrowButton("get_dipole_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_DIPOLE;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Dipole");

                if (ImGui::ArrowButton("get_timestep_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_TSTEP;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Timestep");

                if (ImGui::ArrowButton("get_measure_time_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_MEASURE_TIME;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Measure Time");

                if (ImGui::ArrowButton("get_leeway_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_LEEWAY;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Leeway");

                if (ImGui::ArrowButton("get_wtarget_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_WTARGET;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get W-Target");

                if (ImGui::ArrowButton("get_detumble_angle_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_DETUMBLE_ANG;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Detumble Angle");

                if (ImGui::ArrowButton("get_sun_angle_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_SUN_ANGLE;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Sun Angle");
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 1)
            {
                ImGui::RadioButton("Set MOI", &ACS_command, ACS_SET_MOI);    // 9x floats
                ImGui::InputFloat3("MOI [0] [1] [2]", &acs_set_data.moi[0]); // Sets 3 at a time... so... yeah.
                ImGui::InputFloat3("MOI [3] [4] [5]", &acs_set_data.moi[3]);
                ImGui::InputFloat3("MOI [6] [7] [8]", &acs_set_data.moi[6]);
                ImGui::RadioButton("Set IMOI", &ACS_command, ACS_SET_IMOI); // 9x floats
                ImGui::InputFloat3("IMOI [0] [1] [2]", &acs_set_data.imoi[0]);
                ImGui::InputFloat3("IMOI [3] [4] [5]", &acs_set_data.imoi[3]);
                ImGui::InputFloat3("IMOI [6] [7] [8]", &acs_set_data.imoi[6]);
                ImGui::RadioButton("Set Dipole", &ACS_command, ACS_SET_DIPOLE); // 1x float
                ImGui::InputFloat("Dipole Moment", &acs_set_data.dipole);
                ImGui::RadioButton("Set Timestep", &ACS_command, ACS_SET_TSTEP); // 1x uint8_t
                if (ImGui::InputInt("Timestep (ms)", &acs_set_data_holder.tstep))
                {
                    if (acs_set_data_holder.tstep > 255)
                    {
                        acs_set_data_holder.tstep = 255;
                    }
                    else if (acs_set_data_holder.tstep < 0)
                    {
                        acs_set_data_holder.tstep = 0;
                    }
                    acs_set_data.tstep = (uint8_t)acs_set_data_holder.tstep;
                }

                ImGui::RadioButton("Set Measure Time", &ACS_command, ACS_SET_MEASURE_TIME); // 1x uint8_t
                if (ImGui::InputInt("Measure Time (ms)", &acs_set_data_holder.measure_time))
                {
                    if (acs_set_data_holder.measure_time > 255)
                    {
                        acs_set_data_holder.measure_time = 255;
                    }
                    else if (acs_set_data_holder.measure_time < 0)
                    {
                        acs_set_data_holder.measure_time = 0;
                    }
                    acs_set_data.measure_time = (uint8_t)acs_set_data_holder.measure_time;
                }

                ImGui::RadioButton("Set Leeway", &ACS_command, ACS_SET_LEEWAY); // 1x uint8_t
                if (ImGui::InputInt("Leeway Factor", &acs_set_data_holder.leeway))
                {
                    if (acs_set_data_holder.leeway > 255)
                    {
                        acs_set_data_holder.leeway = 255;
                    }
                    else if (acs_set_data_holder.leeway < 0)
                    {
                        acs_set_data_holder.leeway = 0;
                    }
                    acs_set_data.leeway = (uint8_t)acs_set_data_holder.leeway;
                }

                ImGui::RadioButton("Set W-Target", &ACS_command, ACS_SET_WTARGET); // 1x float
                ImGui::InputFloat("W-Target", &acs_set_data.wtarget);
                ImGui::RadioButton("Set Detumble Angle", &ACS_command, ACS_SET_DETUMBLE_ANG); // 1x uint8_t
                if (ImGui::InputInt("Angle", &acs_set_data_holder.detumble_angle))
                {
                    if (acs_set_data_holder.detumble_angle > 255)
                    {
                        acs_set_data_holder.detumble_angle = 255;
                    }
                    else if (acs_set_data_holder.detumble_angle < 0)
                    {
                        acs_set_data_holder.detumble_angle = 0;
                    }
                    acs_set_data.detumble_angle = (uint8_t)acs_set_data_holder.detumble_angle;
                }
                ImGui::RadioButton("Set Sun Angle", &ACS_command, ACS_SET_SUN_ANGLE); // 1x uint8_t
                if (ImGui::InputInt("Sun Angle", &acs_set_data_holder.sun_angle))
                {
                    if (acs_set_data_holder.sun_angle > 255)
                    {
                        acs_set_data_holder.sun_angle = 255;
                    }
                    else if (acs_set_data_holder.sun_angle < 0)
                    {
                        acs_set_data_holder.sun_angle = 0;
                    }
                    acs_set_data.sun_angle = (uint8_t)acs_set_data_holder.sun_angle;
                }
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ACS_command_input.mod = ACS_ID;
        ACS_command_input.cmd = ACS_command;

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transmit"))
        {

            if (allow_transmission)
            {
                ImGui::Text("ACS Data-down Update");
                ImGui::Indent();
                ImGui::Checkbox("Automated", &acs_rxtx_automated);

                ImGui::Text("(%s) Polling for Data-down every 100 ms.", acs_rxtx_automated ? "ON" : "OFF");

                // TODO: Spawn a thread to execute gs_acs_update_data_handler(...) once.
                // If the operator wants to activate the automatic ACS update system...
                if (acs_rxtx_automated)
                {
                    // Spawn a new thread to run
                    if (pthread_mutex_trylock(&acs_rolbuf->acs_upd_inhibitor) == 0)
                    {
                        pthread_create(&acs_thread_id, NULL, gs_acs_update_thread, acs_rolbuf);
                    }
                }

                ImGui::Unindent();
                ImGui::Text("Send Command [Data-up]");
                ImGui::Indent();
                if (auth->access_level > 1)
                {
                    // Move the data into ACS_command_input.data
                    switch (ACS_command_input.cmd)
                    {
                    case ACS_SET_MOI:
                    {
                        memcpy(ACS_command_input.data, acs_set_data.moi, sizeof(float) * 9);
                        ACS_command_input.data_size = sizeof(float) * 9;
                        break;
                    }
                    case ACS_SET_IMOI:
                    {
                        memcpy(ACS_command_input.data, acs_set_data.imoi, sizeof(float) * 9);
                        ACS_command_input.data_size = sizeof(float) * 9;
                        break;
                    }
                    case ACS_SET_DIPOLE:
                    {
                        ACS_command_input.data_size = sizeof(float);
                        memcpy(ACS_command_input.data, &acs_set_data.dipole, ACS_command_input.data_size);
                        break;
                    }
                    case ACS_SET_TSTEP:
                    {
                        ACS_command_input.data[0] = (uint8_t)acs_set_data.tstep;
                        ACS_command_input.data_size = sizeof(uint8_t);
                        break;
                    }
                    case ACS_SET_MEASURE_TIME:
                    {
                        ACS_command_input.data[0] = (uint8_t)acs_set_data.measure_time;
                        ACS_command_input.data_size = sizeof(uint8_t);
                        break;
                    }
                    case ACS_SET_LEEWAY:
                    {
                        ACS_command_input.data[0] = (uint8_t)acs_set_data.leeway;
                        ACS_command_input.data_size = sizeof(uint8_t);
                        break;
                    }
                    case ACS_SET_WTARGET:
                    {
                        ACS_command_input.data_size = sizeof(float);
                        memcpy(ACS_command_input.data, &acs_set_data.wtarget, ACS_command_input.data_size);
                        break;
                    }
                    case ACS_SET_DETUMBLE_ANG:
                    {
                        ACS_command_input.data[0] = (uint8_t)acs_set_data.detumble_angle;
                        ACS_command_input.data_size = sizeof(uint8_t);
                        break;
                    }
                    case ACS_SET_SUN_ANGLE:
                    {
                        ACS_command_input.data[0] = (uint8_t)acs_set_data.sun_angle;
                        ACS_command_input.data_size = sizeof(uint8_t);
                        break;
                    }
                    default:
                    {
                        ACS_command_input.data_size = -1;
                        break;
                    }
                    }
                    ImGui::Unindent();

                    gs_gui_gs2sh_tx_handler(auth, &ACS_command_input);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            else
            {
                ImGui::Text("TRANSMISSIONS LOCKED");
            }
        }
    }
    ImGui::End();
}

void gs_gui_eps_window(bool *EPS_window, auth_t *auth, bool *allow_transmission)
{
    static int EPS_command = EPS_INVALID_ID;
    static cmd_input_t EPS_command_input = {.mod = INVALID_ID, .cmd = EPS_INVALID_ID, .unused = 0, .data_size = 0};
    static eps_set_data_t eps_set_data = {0};

    if (ImGui::Begin("EPS Operations", EPS_window))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {

            if (auth->access_level > 0)
            {
                if (ImGui::ArrowButton("get_min_hk_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_MIN_HK;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Minimal Housekeeping");

                if (ImGui::ArrowButton("get_vbatt_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_VBATT;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Battery Voltage");

                if (ImGui::ArrowButton("get_sys_curr_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_SYS_CURR;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get System Current");

                if (ImGui::ArrowButton("get_power_out_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_OUTPOWER;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Power Out");

                if (ImGui::ArrowButton("get_solar_voltage_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_VSUN;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Solar Voltage");

                if (ImGui::ArrowButton("get_all_solar_voltage_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_VSUN_ALL;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Solar Voltage (All)");

                if (ImGui::ArrowButton("get_isun_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_ISUN;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get ISUN");

                if (ImGui::ArrowButton("get_loop_timer_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_LOOP_TIMER;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &EPS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Loop Timer");
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 1)
            {
                ImGui::RadioButton("Set Loop Timer", &EPS_command, EPS_SET_LOOP_TIMER);
                ImGui::InputInt("Loop Time (seconds)", &eps_set_data.loop_timer);
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        EPS_command_input.mod = EPS_ID;
        EPS_command_input.cmd = EPS_command;

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transmit"))
        {
            if (allow_transmission)
            {
                ImGui::Indent();
                if (auth->access_level > 1)
                {
                    switch (EPS_command_input.cmd)
                    {
                    case EPS_SET_LOOP_TIMER:
                    {
                        // TODO: Remove all .data[0] = some_variable, because this will only set the first byte of .data because its in section of bytes.
                        // TODO: ~DO NOT REMOVE~ UNLESS it is a one-byte kind of data, ie uint8_t.
                        EPS_command_input.data_size = sizeof(int);
                        memcpy(EPS_command_input.data, &eps_set_data.loop_timer, EPS_command_input.data_size);
                        break;
                    }
                    default:
                    {
                        EPS_command_input.data_size = -1;
                        break;
                    }
                    }
                    ImGui::Unindent();

                    gs_gui_gs2sh_tx_handler(auth, &EPS_command_input);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            else
            {
                ImGui::Text("TRANSMISSIONS LOCKED");
            }
        }
    }
    ImGui::End();
}

void gs_gui_xband_window(bool *XBAND_window, auth_t *auth, bool *allow_transmission)
{
    static int XBAND_command = XBAND_INVALID_ID;
    static cmd_input_t XBAND_command_input = {.mod = INVALID_ID, .cmd = XBAND_INVALID_ID, .unused = 0, .data_size = 0};
    static xband_set_data_array_t xband_set_data = {0};
    static xband_tx_data_t xband_tx_data = {0};
    static xband_tx_data_holder_t xband_tx_data_holder = {0};
    static xband_rxtx_data_t xband_rxtx_data = {0};
    static xband_rxtx_data_holder_t xband_rxtx_data_holder = {0};
    static xband_get_bool_t xband_get_bool = {0};

    if (ImGui::Begin("X-Band Operations", XBAND_window))
    {
        // TODO: Change to arrow-button implementation.
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {
            if (auth->access_level > 0)
            {
                ImGui::Checkbox("Get MAX ON", &xband_get_bool.max_on);
                ImGui::Checkbox("Get TMP SHDN", &xband_get_bool.tmp_shdn);
                ImGui::Checkbox("Get TMP OP", &xband_get_bool.tmp_op);
                ImGui::Checkbox("Get Loop Time", &xband_get_bool.loop_time);
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 1)
            {
                ImGui::Indent();

                ImGui::RadioButton("Set Transmit", &XBAND_command, XBAND_SET_TX);
                ImGui::InputFloat("TX LO", &xband_set_data.TX.LO);
                ImGui::InputFloat("TX bw", &xband_set_data.TX.bw);
                ImGui::InputInt("TX Samp", &xband_set_data.TXH.samp);
                if (xband_set_data.TXH.samp > 0xFFFF)
                {
                    xband_set_data.TXH.samp = 0xFFFF;
                }
                else if (xband_set_data.TXH.samp < 0)
                {
                    xband_set_data.TXH.samp = 0;
                }
                xband_set_data.TX.samp = (uint16_t)xband_set_data.TXH.samp;

                ImGui::InputInt("TX Phy Gain", &xband_set_data.TXH.phy_gain);
                if (xband_set_data.TXH.phy_gain > 0xFF)
                {
                    xband_set_data.TXH.phy_gain = 0xFF;
                }
                else if (xband_set_data.TXH.phy_gain < 0)
                {
                    xband_set_data.TXH.phy_gain = 0;
                }
                xband_set_data.TX.phy_gain = (uint8_t)xband_set_data.TXH.phy_gain;

                ImGui::InputInt("TX Adar Gain", &xband_set_data.TXH.adar_gain);
                if (xband_set_data.TXH.adar_gain > 0xFF)
                {
                    xband_set_data.TXH.adar_gain = 0xFF;
                }
                else if (xband_set_data.TXH.adar_gain < 0)
                {
                    xband_set_data.TXH.adar_gain = 0;
                }
                xband_set_data.TX.adar_gain = (uint8_t)xband_set_data.TXH.adar_gain;

                if (ImGui::BeginMenu("TX Filter Selection"))
                {
                    ImGui::RadioButton("m_6144.ftr", &xband_set_data.TXH.ftr, 0);
                    ImGui::RadioButton("m_3072.ftr", &xband_set_data.TXH.ftr, 1);
                    ImGui::RadioButton("m_1000.ftr", &xband_set_data.TXH.ftr, 2);
                    ImGui::RadioButton("m_lte5.ftr", &xband_set_data.TXH.ftr, 3);
                    ImGui::RadioButton("m_lte1.ftr", &xband_set_data.TXH.ftr, 4);

                    xband_set_data.TX.ftr = (uint8_t)xband_set_data.TXH.ftr;

                    ImGui::EndMenu();
                }
                ImGui::InputInt4("TX Phase [0]  [1]  [2]  [3]", &xband_set_data.TXH.phase[0]);
                ImGui::InputInt4("TX Phase [4]  [5]  [6]  [7]", &xband_set_data.TXH.phase[4]);
                ImGui::InputInt4("TX Phase [8]  [9]  [10] [11]", &xband_set_data.TXH.phase[8]);
                ImGui::InputInt4("TX Phase [12] [13] [14] [15]", &xband_set_data.TXH.phase[12]);
                for (int i = 0; i < 16; i++)
                {
                    if (xband_set_data.TXH.phase[i] > 32767)
                    {
                        xband_set_data.TXH.phase[i] = 32767;
                    }
                    else if (xband_set_data.TXH.phase[i] < -32768)
                    {
                        xband_set_data.TXH.phase[i] = -32768;
                    }
                    xband_set_data.TX.phase[i] = (short)xband_set_data.TXH.phase[i];
                }

                ImGui::Separator();

                ImGui::RadioButton("Set Receive", &XBAND_command, XBAND_SET_RX);
                ImGui::InputFloat("RX LO", &xband_set_data.RX.LO);
                ImGui::InputFloat("RX bw", &xband_set_data.RX.bw);
                ImGui::InputInt("RX Samp", &xband_set_data.RXH.samp);
                if (xband_set_data.RXH.samp > 0xFFFF)
                {
                    xband_set_data.RXH.samp = 0xFFFF;
                }
                else if (xband_set_data.RXH.samp < 0)
                {
                    xband_set_data.RXH.samp = 0;
                }
                xband_set_data.RX.samp = (uint16_t)xband_set_data.RXH.samp;

                ImGui::InputInt("RX Phy Gain", &xband_set_data.RXH.phy_gain);
                if (xband_set_data.RXH.phy_gain > 0xFF)
                {
                    xband_set_data.RXH.phy_gain = 0xFF;
                }
                else if (xband_set_data.RXH.phy_gain < 0)
                {
                    xband_set_data.RXH.phy_gain = 0;
                }
                xband_set_data.RX.phy_gain = (uint8_t)xband_set_data.RXH.phy_gain;

                ImGui::InputInt("RX Adar Gain", &xband_set_data.RXH.adar_gain);
                if (xband_set_data.RXH.adar_gain > 0xFF)
                {
                    xband_set_data.RXH.adar_gain = 0xFF;
                }
                else if (xband_set_data.RXH.adar_gain < 0)
                {
                    xband_set_data.RXH.adar_gain = 0;
                }
                xband_set_data.RX.adar_gain = (uint8_t)xband_set_data.RXH.adar_gain;

                if (ImGui::BeginMenu("RX Filter Selection"))
                {
                    ImGui::RadioButton("m_6144.ftr", &xband_set_data.RXH.ftr, 0);
                    ImGui::RadioButton("m_3072.ftr", &xband_set_data.RXH.ftr, 1);
                    ImGui::RadioButton("m_1000.ftr", &xband_set_data.RXH.ftr, 2);
                    ImGui::RadioButton("m_lte5.ftr", &xband_set_data.RXH.ftr, 3);
                    ImGui::RadioButton("m_lte1.ftr", &xband_set_data.RXH.ftr, 4);

                    xband_set_data.RX.ftr = (uint8_t)xband_set_data.RXH.ftr;

                    ImGui::EndMenu();
                }
                ImGui::InputInt4("RX Phase [0]  [1]  [2]  [3]", &xband_set_data.RXH.phase[0]);
                ImGui::InputInt4("RX Phase [4]  [5]  [6]  [7]", &xband_set_data.RXH.phase[4]);
                ImGui::InputInt4("RX Phase [8]  [9]  [10] [11]", &xband_set_data.RXH.phase[8]);
                ImGui::InputInt4("RX Phase [12] [13] [14] [15]", &xband_set_data.RXH.phase[12]);
                for (int i = 0; i < 16; i++)
                {
                    if (xband_set_data.RXH.phase[i] > 32767)
                    {
                        xband_set_data.RXH.phase[i] = 32767;
                    }
                    else if (xband_set_data.RXH.phase[i] < -32768)
                    {
                        xband_set_data.RXH.phase[i] = -32768;
                    }
                    xband_set_data.RX.phase[i] = (short)xband_set_data.RXH.phase[i];
                }
                // }
                ImGui::Separator();

                ImGui::RadioButton("Set MAX ON", &XBAND_command, XBAND_SET_MAX_ON);
                ImGui::InputInt("Max On", &xband_rxtx_data_holder.max_on);
                if (xband_rxtx_data_holder.max_on > 0xFF)
                {
                    xband_rxtx_data_holder.max_on = 0xFF;
                }
                else if (xband_rxtx_data_holder.max_on < 0)
                {
                    xband_rxtx_data_holder.max_on = 0;
                }
                xband_rxtx_data.max_on = (uint8_t)xband_rxtx_data_holder.max_on;

                ImGui::RadioButton("Set TMP SHDN", &XBAND_command, XBAND_SET_TMP_SHDN);
                ImGui::InputInt("TMP SHDN", &xband_rxtx_data_holder.tmp_shdn);
                if (xband_rxtx_data_holder.tmp_shdn > 0xFF)
                {
                    xband_rxtx_data_holder.tmp_shdn = 0xFF;
                }
                else if (xband_rxtx_data_holder.tmp_shdn < 0)
                {
                    xband_rxtx_data_holder.tmp_shdn = 0;
                }
                xband_rxtx_data.tmp_shdn = (uint8_t)xband_rxtx_data_holder.tmp_shdn;

                ImGui::RadioButton("Set TMP OP", &XBAND_command, XBAND_SET_TMP_OP);
                ImGui::InputInt("TMP OP", &xband_rxtx_data_holder.tmp_op);
                if (xband_rxtx_data_holder.tmp_op > 0xFF)
                {
                    xband_rxtx_data_holder.tmp_op = 0xFF;
                }
                else if (xband_rxtx_data_holder.tmp_op < 0)
                {
                    xband_rxtx_data_holder.tmp_op = 0;
                }
                xband_rxtx_data.tmp_op = (uint8_t)xband_rxtx_data_holder.tmp_op;

                ImGui::RadioButton("Set Loop Time", &XBAND_command, XBAND_SET_LOOP_TIME);
                ImGui::InputInt("Loop Time", &xband_rxtx_data_holder.loop_time);
                if (xband_rxtx_data_holder.loop_time > 0xFF)
                {
                    xband_rxtx_data_holder.loop_time = 0xFF;
                }
                else if (xband_rxtx_data_holder.loop_time < 0)
                {
                    xband_rxtx_data_holder.loop_time = 0;
                }
                xband_rxtx_data.loop_time = (uint8_t)xband_rxtx_data_holder.loop_time;

                ImGui::Unindent();
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ImGui::Separator();

        // ImGui::Text("Actionable Commands");
        if (ImGui::CollapsingHeader("Executable Commands"))
        {
            if (auth->access_level > 1)
            {
                // TODO:
                ImGui::RadioButton("Transmit", &XBAND_command, XBAND_DO_TX);
                ImGui::InputInt("TX type", &xband_tx_data_holder.type);
                if (xband_tx_data_holder.type > 255)
                {
                    xband_tx_data_holder.type = 255;
                }
                else if (xband_tx_data_holder.type < 0)
                {
                    xband_tx_data_holder.type = 0;
                }
                xband_tx_data.type = (uint8_t)xband_tx_data_holder.type;

                ImGui::InputInt("TX f_id", &xband_tx_data.f_id);
                ImGui::InputInt("TX mtu", &xband_tx_data.mtu);
                ImGui::RadioButton("Receive", &XBAND_command, XBAND_DO_RX);
                ImGui::SameLine();
                ImGui::Text("(NYI)");
                ImGui::RadioButton("Disable", &XBAND_command, XBAND_DISABLE);
                ImGui::SameLine();
                ImGui::Text("(NYI)");
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        XBAND_command_input.mod = XBAND_ID;
        XBAND_command_input.cmd = XBAND_command;

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transmit"))
        {
            if (allow_transmission)
            {
                ImGui::Indent();
                if (auth->access_level > 1)
                {
                    switch (XBAND_command_input.cmd)
                    {
                    case XBAND_SET_TX:
                    {
                        memcpy(XBAND_command_input.data, &xband_set_data.TX, sizeof(xband_set_data_t));
                        XBAND_command_input.data_size = sizeof(xband_set_data_t);
                        break;
                    }
                    case XBAND_SET_RX:
                    {
                        memcpy(XBAND_command_input.data, &xband_set_data.RX, sizeof(xband_set_data_t));
                        XBAND_command_input.data_size = sizeof(xband_set_data_t);
                        break;
                    }
                    case XBAND_DO_TX:
                    {
                        memcpy(XBAND_command_input.data, &xband_tx_data, sizeof(xband_tx_data_t));
                        XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                        break;
                    }
                    case XBAND_DO_RX:
                    {
                        // Not yet implemented on SPACE-HAUC.
                        printf("This functionality is not implemented as it does not yet exist on SPACE-HAUC.\n");
                        break;
                    }
                    case XBAND_DISABLE:
                    {
                        // Not yet implemented on SPACE-HAUC.
                        printf("This functionality is not implemented as it does not yet exist on SPACE-HAUC.\n");
                        break;
                    }
                    case XBAND_SET_MAX_ON:
                    {
                        // cmd_parser expects an int8_t for data.
                        XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.max_on;
                        XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                        break;
                    }
                    case XBAND_SET_TMP_SHDN:
                    {
                        // cmd_parser expects an int8_t for data.
                        XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_shdn;
                        XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                        break;
                    }
                    case XBAND_SET_TMP_OP:
                    {
                        // cmd_parser expects an int8_t for data.
                        XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_op;
                        XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                        break;
                    }
                    case XBAND_SET_LOOP_TIME:
                    {
                        // cmd_parser expects an int8_t for data.
                        // XBAND_command_input is a structure that is transmitted.
                        // .data contains the data expected by SH's cmd_parser
                        // xband_rxtx_data is just a convenient structure for collecting user input
                        XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_op;
                        break;
                    }
                    default:
                    {
                        XBAND_command_input.data_size = -1;
                    }
                    }

                    gs_gui_gs2sh_tx_handler(auth, &XBAND_command_input);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            else
            {
                ImGui::Text("TRANSMISSIONS LOCKED");
            }
        }
    }
    ImGui::End();
}

void gs_gui_sw_upd_window(bool *SW_UPD_window, auth_t *auth, bool *allow_transmission)
{
    static cmd_input_t UPD_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
    static char upd_filename_buffer[256] = {0};

    if (ImGui::Begin("Software Updater Control Panel", SW_UPD_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        // Needs buttons. Values are just #defines and magic values

        ImGui::Text("Name of the file to send:");
        ImGui::InputTextWithHint("", "Name of File", upd_filename_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue);

        // NOTE: Queued file name not sent with software update command, probably doesn't need to be here at all. This is something the Ground Station's version of SW_UPDATE will have to handle.
        ImGui::Text("Queued file name: %s", upd_filename_buffer);

        ImGui::Separator();

        if (auth->access_level > 1)
        {
            if (ImGui::Button("BEGIN UPDATE"))
            {
                // Sets values for the software update command structure.
                UPD_command_input.mod = SW_UPD_ID;
                UPD_command_input.cmd = SW_UPD_FUNC_MAGIC;
                // UPD_command_input.data[0] = SW_UPD_VALID_MAGIC;
                long sw_upd_valid_magic_temp = SW_UPD_VALID_MAGIC;
                memcpy(UPD_command_input.data, &sw_upd_valid_magic_temp, sizeof(long));

                // Transmits the software update command.
                gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &UPD_command_input, sizeof(cmd_input_t));
            }
        }
        else
        {
            ImGui::Text("ACCESS DENIED");
        }

        // Maybe some information on the current status of an update can go here.
    }
    ImGui::End();
}

void gs_gui_sys_ctrl_window(bool *SYS_CTRL_window, auth_t *auth, bool *allow_transmission)
{
    // static int SYS_command = INVALID_ID;
    static cmd_input_t SYS_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
    static cmd_input_holder_t SYS_command_input_holder = {0};

    if (ImGui::Begin("System Control Panel", SYS_CTRL_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {
            if (auth->access_level > 0)
            {
                if (ImGui::ArrowButton("get_version_magic", ImGuiDir_Right))
                {
                    SYS_command_input.mod = SYS_VER_MAGIC;
                    SYS_command_input.cmd = 0x0;
                    SYS_command_input.unused = 0x0;
                    SYS_command_input.data_size = 0x0;
                    memset(SYS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(CS_TYPE_DATA, CS_ENDPOINT_SPACEHAUC, &SYS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Version Magic");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 2)
            {
                // NOTE: Since IMGUI only accepts full integers, we have to use a temporary full integer structure to hold them before converting to uint8_t, which is what SPACE-HAUC requires.
                ImGui::RadioButton("Restart Program", &SYS_command_input_holder.mod, SYS_RESTART_PROG);
                ImGui::RadioButton("Reboot Flight", &SYS_command_input_holder.mod, SYS_REBOOT);
                ImGui::RadioButton("Clean SHBYTES", &SYS_command_input_holder.mod, SYS_CLEAN_SHBYTES);

                SYS_command_input.mod = (uint8_t)SYS_command_input_holder.mod;
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Transmit"))
        {
            if (allow_transmission)
            {
                ImGui::Indent();
                if (auth->access_level > 2)
                {
                    switch (SYS_command_input.mod)
                    {
                    case SYS_RESTART_PROG:
                    {
                        SYS_command_input.data_size = 0;
                        break;
                    }
                    case SYS_REBOOT:
                    {
                        SYS_command_input.data_size = sizeof(uint64_t);
                        SYS_command_input.cmd = SYS_REBOOT_FUNC_MAGIC;
                        long temp = SYS_REBOOT_FUNC_VAL;
                        memcpy(SYS_command_input.data, &temp, SYS_command_input.data_size);

                        SYS_command_input.unused = 0x0;

                        break;
                    }
                    case SYS_CLEAN_SHBYTES:
                    {
                        SYS_command_input.data_size = 0;
                        SYS_command_input.cmd = SYS_CLEAN_SHBYTES;

                        SYS_command_input.unused = 0x0;
                        memset(SYS_command_input.data, 0x0, MAX_DATA_SIZE);
                        break;
                    }
                    default:
                    {
                        // printf("SYS ERROR!\n");
                        SYS_command_input.data_size = -1;
                        break;
                    }
                    }
                    ImGui::Unindent();

                    gs_gui_gs2sh_tx_handler(auth, &SYS_command_input);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            else
            {
                ImGui::Text("TRANSMISSIONS LOCKED");
            }
        }
    }
    ImGui::End();
}

void gs_gui_rx_display_window(bool *RX_display)
{
    if (ImGui::Begin("Plaintext RX Display"), RX_display)
    {
    }
    ImGui::End();
}

void gs_gui_acs_upd_display_window(bool *ACS_UPD_display, ACSRollingBuffer *acs_rolbuf)
{
    if (ImGui::Begin("ACS Update Display"), ACS_UPD_display)
    {
        // The implemented method of displaying the ACS update data includes a locally-global class (ACSDisplayData) with data that this window will display. The data is set by gs_receive.
        // NOTE: This window must be opened independent of ACS's automated data retrieval option.

        // TODO: Remove these random data-adders and implement the actual data adding in gs_receive. ~Removed, but moved to gs_receive.

        // TODO: Remove this once testing is complete.
        // acs_rolbuf->x_index += 0.1;

        double start_time = acs_rolbuf->x_index - 60;
        if (start_time < 0)
        {
            start_time = 0;
        }

        if (ImGui::CollapsingHeader("CT / Mode Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->mode.Min(), acs_rolbuf->ct.Min()), getMax(acs_rolbuf->mode.Max(), acs_rolbuf->ct.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("CT / Mode Graph"))
            {

                ImPlot::PlotLine("CT", &acs_rolbuf->ct.data[0].x, &acs_rolbuf->ct.data[0].y, acs_rolbuf->ct.data.size(), acs_rolbuf->ct.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("Mode", &acs_rolbuf->mode.data[0].x, &acs_rolbuf->mode.data[0].y, acs_rolbuf->mode.data.size(), acs_rolbuf->mode.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("B (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->bx.Min(), acs_rolbuf->by.Min(), acs_rolbuf->bz.Min()), getMax(acs_rolbuf->bx.Max(), acs_rolbuf->by.Max(), acs_rolbuf->bz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("B (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->bx.data[0].x, &acs_rolbuf->bx.data[0].y, acs_rolbuf->bx.data.size(), acs_rolbuf->bx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->by.data[0].x, &acs_rolbuf->by.data[0].y, acs_rolbuf->by.data.size(), acs_rolbuf->by.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->bz.data[0].x, &acs_rolbuf->bz.data[0].y, acs_rolbuf->bz.data.size(), acs_rolbuf->bz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("W (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->wx.Min(), acs_rolbuf->wy.Min(), acs_rolbuf->wz.Min()), getMax(acs_rolbuf->wx.Max(), acs_rolbuf->wy.Max(), acs_rolbuf->wz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("W (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->wx.data[0].x, &acs_rolbuf->wx.data[0].y, acs_rolbuf->wx.data.size(), acs_rolbuf->wx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->wy.data[0].x, &acs_rolbuf->wy.data[0].y, acs_rolbuf->wy.data.size(), acs_rolbuf->wy.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->wz.data[0].x, &acs_rolbuf->wz.data[0].y, acs_rolbuf->wz.data.size(), acs_rolbuf->wz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("S (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->sx.Min(), acs_rolbuf->sy.Min(), acs_rolbuf->sz.Min()), getMax(acs_rolbuf->sx.Max(), acs_rolbuf->sy.Max(), acs_rolbuf->sz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("S (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->sx.data[0].x, &acs_rolbuf->sx.data[0].y, acs_rolbuf->sx.data.size(), acs_rolbuf->sx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->sy.data[0].x, &acs_rolbuf->sy.data[0].y, acs_rolbuf->sy.data.size(), acs_rolbuf->sy.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->sz.data[0].x, &acs_rolbuf->sz.data[0].y, acs_rolbuf->sz.data.size(), acs_rolbuf->sz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("Battery Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->vbatt.Min(), acs_rolbuf->vboost.Min()), getMax(acs_rolbuf->vbatt.Max(), acs_rolbuf->vbatt.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("Battery Graph"))
            {

                ImPlot::PlotLine("VBatt", &acs_rolbuf->vbatt.data[0].x, &acs_rolbuf->vbatt.data[0].y, acs_rolbuf->vbatt.data.size(), acs_rolbuf->vbatt.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("VBoost", &acs_rolbuf->vboost.data[0].x, &acs_rolbuf->vboost.data[0].y, acs_rolbuf->vboost.data.size(), acs_rolbuf->vboost.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("Solar Current Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->cursun.Min(), acs_rolbuf->cursun.Min()), getMax(acs_rolbuf->cursys.Max(), acs_rolbuf->cursys.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("Solar Current Graph"))
            {

                ImPlot::PlotLine("CurSun", &acs_rolbuf->cursun.data[0].x, &acs_rolbuf->cursun.data[0].y, acs_rolbuf->cursun.data.size(), acs_rolbuf->cursun.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("CurSys", &acs_rolbuf->cursys.data[0].x, &acs_rolbuf->cursys.data[0].y, acs_rolbuf->cursys.data.size(), acs_rolbuf->cursys.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();
}

void gs_gui_disp_control_panel_window(bool *DISP_control_panel, bool *ACS_window, bool *EPS_window, bool *XBAND_window, bool *SW_UPD_window, bool *SYS_CTRL_window, bool *RX_display, bool *ACS_UPD_display, bool *allow_transmission, auth_t *auth)
{
    if (ImGui::Begin("Displays Control Panel", DISP_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar /*| ImGuiWindowFlags_MenuBar*/))
    {
        ImGui::Checkbox("Attitude Control System", ACS_window); // Contains ACS and ACS_UPD
        ImGui::Checkbox("Electrical Power Supply", EPS_window);
        ImGui::Checkbox("X-Band", XBAND_window);
        ImGui::Checkbox("Software Updater", SW_UPD_window);
        ImGui::Checkbox("System Control", SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

        ImGui::Separator();

        ImGui::Checkbox("Plaintext RX Display", RX_display);
        ImGui::Checkbox("ACS Update Display", ACS_UPD_display);

        ImGui::Separator();

        if (auth->access_level >= 0)
        {
            ImGui::Checkbox("Unlock Transmissions", allow_transmission);
        }
        else
        {
            ImGui::Text("ACCESS DENIED");
        }
    }
    ImGui::End();
}

void gs_gui_user_manual_window(bool *User_Manual)
{
    if (ImGui::Begin("User Manual", User_Manual, ImGuiWindowFlags_AlwaysVerticalScrollbar /* | ImGuiWindowFlags_HorizontalScrollbar*/))
    {
        ImGui::BeginTabBar("Help Menu Tab Bar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
        if (ImGui::BeginTabItem("Getting Started"))
        {
            ImGui::TextWrapped("Overview");

            ImGui::Indent();

            // TODO: Implement a display to show the data retrieved when a data-down command is sent.
            ImGui::TextWrapped("The SPACE-HAUC Ground Station Client is provided to allow an operator to interface with the SPACE-HAUC satellite.");

            ImGui::TextWrapped("Data-retrieval commands, referred to as 'data-down,' are sent automatically when the corresponding Arrow Button is pressed. These can be found in the corresponding 'Data-down' drop-down sections of the three 'Operations' panels (ACS, EPS, or XBAND). Once the data is received, it is displayed in the [NO DISPLAY IMPLEMENTED YET].");

            ImGui::TextWrapped("Value-setting commands, referred to as 'data-up,' are accessed in the 'Data-up' drop-down sections of the three 'Operations' panels. Unlike the 'data-down' commands, 'data-up' commands do not send automatically. Instead, the operator must choose which 'data-up' command they would like to make active using the Radio Buttons. Any number of 'data-up' command arguments can be edited simultaneously, but only one 'data-up' command can be selected for transmission at a time. The arguments are not cleared until the program is restarted.");

            ImGui::TextWrapped("Once the transmit functionality is 'unlocked' via the 'Communications Control' panel, the operator can check the currently queued command in the 'Transmit' drop-down section. Pressing 'SEND DATA-UP TRANSMISSION' will confirm and send the queued transmission. Any return value or data received as a result of the 'data-up' transmission will also be displayed in the [NO DISPLAY IMPLEMENTED YET].");

            ImGui::Unindent();
            ImGui::Separator();

            ImGui::TextWrapped("Window Interaction");
            ImGui::Indent();

            ImGui::TextWrapped("The Menu Bar at the top of the screen has buttons to toggle the visibility of the Authentication, Communications, and User Manual windows. Within the Communications window are checkboxes to toggle the visibility of the Attitude Control System, Electrical Power Supply, X-Band, Software Updater, and System Control windows.");

            ImGui::TextWrapped("Some windows are a fixed size, while larger windows are able to be resized by the user. All windows have an 'X' in the top right-hand corner to hide the window. Windows can also be hidden the same way they are shown as previously mentioned.");

            ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Authentication"))
        {
            ImGui::TextWrapped("Window");
            ImGui::Indent();

            ImGui::TextWrapped("The Authentication Control Panel window is visible by default. Its visibility can be toggled by pressing the Authentication button in the Main Toolbar at the top of the screen.");

            ImGui::TextWrapped("This window displays the current access level, a password-entry field, and a <DEAUTHENTICATE> button.");

            ImGui::TextWrapped("Elements of the Ground Station graphical program which are blocked due to insufficient access level will display the following text: \"ACCESS DENIED\".");

            ImGui::TextWrapped("To enter your password, click on the text entry field and press [ENTER] when finished. The system will process your input and update your access level accordingly. Please note that incorrect inputs are punished with a 2.5 second delay.");

            ImGui::TextWrapped("To revoke authentication, press the <DEAUTHENTICATE> button. This will not reset any previously set values within the program, but will simply deny further access. This should be used by the operator to lock the application when they are not present to prohibit accidental or malicious acts.");

            ImGui::TextWrapped("The Authentication Control Panel does not need to be visible to maintain access, and it is recommended to toggle the visibility off once it is no longer needed by pressing the 'Authentication' button on the Menu Bar at the top of the screen.");

            ImGui::Unindent();
            ImGui::Separator();

            ImGui::TextWrapped("Access Levels");
            ImGui::Indent();

            ImGui::TextWrapped("0 - Low Level Access");
            ImGui::Indent();
            ImGui::TextWrapped("The lowest level of access, this is granted immediately upon startup of the program. Only the basic ACS data update is available.");
            ImGui::Unindent();

            ImGui::TextWrapped("1 - Team Member Access");
            ImGui::Indent();
            ImGui::TextWrapped("This level is granted access to data-down commands.");
            ImGui::Unindent();

            ImGui::TextWrapped("2 - Priority Access");
            ImGui::Indent();
            ImGui::TextWrapped("This level is granted access to some non-mission-critical data-up commands.");
            ImGui::Unindent();

            ImGui::TextWrapped("3 - Project Manager Access");
            ImGui::Indent();
            ImGui::TextWrapped("This level of access is the highest possible, and is granted the ability to perform a software update and set mission critical values.");
            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Displays Control"))
        {
            ImGui::TextWrapped("Window");
            ImGui::Indent();

            ImGui::TextWrapped("The Displays Control window is unique, as it is the only window which allows for toggling the visibility of other windows. The Displays Control window itself can be toggled by the 'Displays Control' button on the Menu Bar at the top of the screen.");

            ImGui::TextWrapped("The checkboxes each enable or disable the visibility of their respective windows. From top to bottom, the first section of checkboxes control the visibility of the Attitude Control System, Electrical Power Supply, X-Band, Software Updater, and System Control input windows. The second section of checkboxes controls the visibility of data output windows, which display the data the Client receives from SPACE-HAUC. Toggling the visibility has no effect on the data or actions being performed, except that it disallows any further user interaction with that window's elements (until it is made visible again, of course).");

            ImGui::TextWrapped("Also in this window is the 'Enable Transmissions' checkbox. This acts as a safety which disallows all data-up transmissions when it is unchecked. This should remain unchecked at ALL TIMES unless one or more data-up transmissions are being made.");

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Communications"))
        {
            ImGui::Unindent();
            ImGui::Separator();

            ImGui::TextWrapped("Command Types");
            ImGui::Indent();

            ImGui::TextWrapped("Data-down");
            ImGui::Indent();
            ImGui::TextWrapped("Commands where data will be sent down from SPACE-HAUC to the Ground Station. Typically, these are commands that poll one or more values.");
            ImGui::Unindent();

            ImGui::TextWrapped("Data-up");
            ImGui::Indent();
            ImGui::TextWrapped("Commands where data will be sent up from the Ground Station to SPACE-HAUC. Typically, these are commands that will set values on-board the spacecraft.");
            ImGui::Unindent();

            ImGui::TextWrapped("Executable");
            ImGui::Indent();
            ImGui::TextWrapped("Commands that instruct SPACE-HAUC to perform some action.");
            ImGui::Unindent();

            ImGui::Unindent();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Copyright"))
        {
            ImGui::TextWrapped("Mit Bailey Copyright (c) 2021");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ACS"))
        {
            ImGui::TextWrapped("Attitude Control System");
            ImGui::Indent();

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("EPS"))
        {
            ImGui::TextWrapped("Electrical Power Supply");
            ImGui::Indent();

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("XBAND"))
        {
            ImGui::TextWrapped("X-Band");
            ImGui::Indent();

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("UPD"))
        {
            ImGui::TextWrapped("Software Updater");
            ImGui::Indent();

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("SYS"))
        {
            ImGui::TextWrapped("System Control");
            ImGui::Indent();

            ImGui::Unindent();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}