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

#include "imgui/imgui.h"
#include "backend/imgui_impl_glfw.h"
#include "backend/imgui_impl_opengl2.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "gs_gui.hpp"
#include <stdlib.h>
#include <unistd.h>

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

/// ACSDisplayData Class
ACSDisplayData::ACSDisplayData()
{
    memset(data, 0x0, sizeof(acs_upd_output_t));
    ready = false;
}

/// ///

void *gs_gui_check_password(void *auth)
{
    auth_t *lauth = (auth_t *)auth;
    lauth->busy = true;

    // Generate hash.
    // Check hash against valid hashes.
    // If valid, grant access.
    // Return access level granted.

    if (gs_helper((unsigned char *)lauth->password) == -07723727136)
    {
        usleep(0.25 SEC);
        lauth->access_level = 1;
    }
    else if (gs_helper((unsigned char *)lauth->password) == 013156200030)
    {
        usleep(0.25 SEC);
        lauth->access_level = 2;
    }
    else if (gs_helper((unsigned char *)lauth->password) == 05657430216)
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

/**
 * @brief 
 * 
 * @param password 
 * @return int Authentication access level allowed.
 */
int gs_gui_check_password_old(char *password)
{
    // Generate hash.
    // Check hash against valid hashes.
    // If valid, grant access.
    // Return access level granted.

    int retval = 0;

    // PLACEHOLDERS
    const char *valid_level_1 = "grantlevel1";
    const char *valid_level_2 = "grantlevel2";

    if (strcmp(valid_level_1, password) == 0)
    {
        retval = 1;
    }
    else if (strcmp(valid_level_2, password) == 0)
    {
        retval = 2;
    }
    else
    {
        retval = 0;
    }

    memset(password, 0x0, strlen(password));
    return retval;
}

// Mildly Obfuscated
unsigned int gs_helper(unsigned char *a)
{
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

void *gs_acs_update_data_handler(void *vp)
{
    acs_upd_input_t *acs = (acs_upd_input_t *)vp;

    acs->cmd_input->mod = ACS_ID;
    acs->cmd_input->cmd = ACS_UPD_ID;
    acs->cmd_input->unused = 0x0;
    acs->cmd_input->data_size = 0x0;
    memset(acs->cmd_input->data, 0x0, MAX_DATA_SIZE);

    usleep(100);

    gs_transmit(acs->cmd_input);

    acs->ready = true;

    return vp;
}

int gs_transmit(cmd_input_t *input)
{
    if (input->data_size < 0)
    {
        printf("Error: input->data_size is %d.\n", input->data_size);
        printf("Cancelling transmit.\n");
        return;
    }

    // Create a client_frame_t object and stuff our data into it.
    client_frame_t tx_frame[1];
    memset(tx_frame, 0x0, SIZE_FRAME_PAYLOAD);
    memcpy(tx_frame->payload, input, SIZE_FRAME_PAYLOAD);
    tx_frame->guid = CLIENT_FRAME_GUID;
    tx_frame->crc1 = crc16(tx_frame->payload, SIZE_FRAME_PAYLOAD);
    tx_frame->crc2 = tx_frame->crc1;
    tx_frame->termination = 0xAAAA;

    // TODO: Change to actually transmitting.
    printf("Pretending to transmit the following:\n");

    printf(FRAME_COLOR "____FRAME HEADER____\n" RESET_COLOR);
    printf("test"
           "guid --------- 0x%04x\n",
           tx_frame->guid);
    printf("crc1 --------- 0x%04x\n", tx_frame->crc1);
    printf(PAYLOAD_COLOR "____PAYLOAD_________\n" RESET_COLOR);
    printf("mod ---------- 0x%02x\n", input->mod);
    printf("cmd ---------- 0x%02x\n", input->cmd);
    printf("unused ------- 0x%02x\n", input->unused);
    printf("data_size ---- 0x%08x\n", input->data_size);
    printf("data --------- ");
    for (int i = 0; i < input->data_size; i++)
    {
        printf("0x%02x ", input->data[i]);
    }
    printf("(END)\n");
    printf(FRAME_COLOR "____FRAME FOOTER____\n" RESET_COLOR);
    printf("crc2 --------- 0x%04x\n", tx_frame->crc2);
    printf("term. -------- 0x%04x\n", tx_frame->termination);
    printf("\n");

    printf("____PAYLOAD AS CHAR____\n");
    printf("mod ---------- %d\n", input->mod);
    printf("cmd ---------- %d\n", input->cmd);
    printf("unused ------- %d\n", input->unused);
    printf("data_size ---- %d\n", input->data_size);
    printf("data --------- ");
    for (int i = 0; i < input->data_size; i++)
    {
        if (input->data[i] <= 0x20 || input->data[i] >= 0x7F)
        {
            printf("[0x%02x]", input->data[i]);
        }
        else
        {
            printf("%c", input->data[i]);
        }
    }
    printf("(END)\n");
    printf("\n");

    return 1;
}
// gs_gui_transmissions_handler(&auth, &XBAND_command_input);
int gs_gui_transmissions_handler(auth_t *auth, cmd_input_t *command_input)
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
        sizeof(xband_tx_data_t);
        ImGui::Unindent();
        if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
        {
            // Send the transmission.
            gs_transmit(command_input);
        }
    }

    return 1;
}

// This needs to act similarly to the cmd_parser.
// TODO: If the data is from ACS update, be sure to set the values in the ACS Update data class!
int gs_receive(client_frame_t *output, ACSDisplayData* acs_display_data)
{
    if (output->guid != CLIENT_FRAME_GUID)
    {
        printf("GUID Error: 0x%04x\n", output->guid);
        return -1;
    }
    else if (output->crc1 != output->crc2)
    {
        printf("CRC Error: 0x%04x != 0x%04x\n", output->crc1, output->crc2);
        return -2;
    }
    else if (output->crc1 != crc16(output->payload, SIZE_FRAME_PAYLOAD))
    {
        printf("CRC Error: 0x%04x != 0x%04x\n", output->crc1, crc16(output->payload, SIZE_FRAME_PAYLOAD));
        return -3;
    }

    cmd_output_t *payload = (cmd_output_t *)output->payload;

    // All data received goes one of two places: the ACS Update Data Display window, or the plaintext trash heap window.
    if (payload->mod == ACS_UPD_ID)
    {
        // Set the data in acs_display_data to the data in output->payload.
        memcpy(acs_display_data->data, payload->data, SIZE_FRAME_PAYLOAD);
        
        // Set the ready flag.
        acs_display_data->ready = true;
    }
    else
    {
        // TODO: Handle all other data by printing it out into a single window. This should probably be handled with another local-global like acs_display_data.
    }

    return 1;
}