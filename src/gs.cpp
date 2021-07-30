/**
 * @file gs.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief Contains class and function defintions.
 * 
 * Contains data-handling code and does not create or augment the GUI directly.
 * 
 * @version 0.1
 * @date 2021.07.26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ifaddrs.h>
#include "gs.hpp"
#include "meb_debug.hpp"

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

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
int gs_connect(int socket, const struct sockaddr *address, socklen_t socket_size, int tout_s)
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
        dbprintlf(RED_FG "Error fcntl(..., F_GETFL)");
        erprintlf(errno);
        return -1;
    }
    arg |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        dbprintlf(RED_FG "Error fcntl(..., F_SETFL)");
        erprintlf(errno);
        return -1;
    }

    // Trying to connect with timeout.
    res = connect(socket, address, socket_size);
    if (res < 0)
    {
        if (errno == EINPROGRESS)
        {
            dbprintlf(YELLOW_FG "EINPROGRESS in connect() - selecting");
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
                    dbprintlf(RED_FG "Error connecting.");
                    erprintlf(errno);
                    return -1;
                }
                else if (res > 0)
                {
                    // Socket selected for write.
                    lon = sizeof(int);
                    if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)(&valopt), &lon) < 0)
                    {
                        dbprintlf(RED_FG "Error in getsockopt()");
                        erprintlf(errno);
                        return -1;
                    }

                    // Check the value returned...
                    if (valopt)
                    {
                        dbprintlf(RED_FG "Error in delayed connection()");
                        erprintlf(valopt);
                        return -1;
                    }
                    break;
                }
                else
                {
                    dbprintlf(RED_FG "Timeout in select(), cancelling!");
                    return -1;
                }
            } while (1);
        }
        else
        {
            fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
            dbprintlf(RED_FG "Error connecting.");
            erprintlf(errno);
            return -1;
        }
    }
    // Set to blocking mode again...
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
    {
        dbprintlf("Error fcntl(..., F_GETFL)");
        erprintlf(errno);
        return -1;
    }
    arg &= (~O_NONBLOCK);
    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        dbprintlf("Error fcntl(..., F_GETFL)");
        erprintlf(errno);
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

void *gs_check_password(void *auth)
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

void *gs_acs_update_thread(void *global_data_vp)
{
    global_data_t *global_data = (global_data_t *)global_data_vp;

    cmd_input_t acs_cmd[1];
    memset(acs_cmd, 0x0, sizeof(cmd_input_t));

    acs_cmd->mod = ACS_UPD_ID;
    acs_cmd->unused = 0x0;
    acs_cmd->data_size = 0x0;
    memset(acs_cmd->data, 0x0, MAX_DATA_SIZE);

    usleep(100000);

    // Transmit an ACS update request to the server.
    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, acs_cmd, sizeof(cmd_input_t));

    // Receive an ACS update from the server.
    // gs_receive(global_data->acs_rolbuf);

    pthread_mutex_unlock(&global_data->acs_rolbuf->acs_upd_inhibitor);

    return NULL;
}

int gs_transmit(NetworkData *network_data, NETWORK_FRAME_TYPE type, NETWORK_FRAME_ENDPOINT endpoint, void *data, int data_size)
{
    if (data_size < 0)
    {
        printf("Error: data_size is %d.\n", data_size);
        printf("Cancelling transmit.\n");
        return -1;
    }

    // Create a NetworkFrame to send our data in.
    NetworkFrame *clientserver_frame = new NetworkFrame(type, data_size);
    clientserver_frame->storePayload(endpoint, data, data_size);

    clientserver_frame->sendFrame(network_data);

    return 1;
}

void *gs_polling_thread(void *args)
{
    global_data_t *global_data = (global_data_t *) args;
    NetworkData *network_data = global_data->network_data;
    
    while (network_data->rx_active)
    {
        if (network_data->connection_ready)
        {
            NetworkFrame *null_frame = new NetworkFrame(CS_TYPE_NULL, 0x0);
            null_frame->storePayload(CS_ENDPOINT_SERVER, NULL, 0);

            send(network_data->socket, null_frame, sizeof(NetworkFrame), 0);
            delete null_frame;
        }

        usleep(15 SEC);
    }

    
    dbprintlf(FATAL "GS_POLLING_THREAD IS EXITING!");
    return NULL;
}

// Updated, referenced "void *rcv_thr(void *sock)" from line 338 of: https://github.com/sunipkmukherjee/comic-mon/blob/master/guimain.cpp
// Also see: https://github.com/mitbailey/socket_server
// TODO: Any premature returns from the RX Thread should be changed to somehow managing the failure. However, in the event that the thread does stop, this needs to be made obvious to the user and there should exist a "Manual RX Thread Restart" function.
void *gs_rx_thread(void *args)
{
    // Convert the passed void pointer into something useful; in this case, global_data_t.
    global_data_t *global_data = (global_data_t *)args;
    NetworkData *network_data = global_data->network_data;

    while (network_data->rx_active)
    {
        if (!network_data->connection_ready)
        {
            sleep(5);
            continue;
        }

        int read_size = 0;

        while (read_size >= 0 && network_data->rx_active)
        {
            char buffer[sizeof(NetworkFrame) * 2];
            memset(buffer, 0x0, sizeof(buffer));

            dbprintlf("Beginning recv...");
            read_size = recv(network_data->socket, buffer, sizeof(buffer), 0);
            dbprintlf("Read %d bytes.", read_size);

            if (read_size > 0)
            {
                dbprintf("RECEIVED (hex): ");
                for (int i = 0; i < read_size; i++)
                {
                    printf("%02x", buffer[i]);
                }
                printf("(END)\n");

                // Parse the data.
                // Map a NetworkFrame onto the data; this allows us to use the class' functions on the data.
                NetworkFrame *clientserver_frame = (NetworkFrame *)buffer;

                // Check if we've received data in the form of a NetworkFrame.
                if (clientserver_frame->checkIntegrity() < 0)
                {
                    dbprintlf("Integrity check failed (%d).", clientserver_frame->checkIntegrity());
                    continue;
                }
                dbprintlf("Integrity check successful.");

                // TODO: Write this data to the GUI in some meaningful manner.
                global_data->netstat = clientserver_frame->getNetstat();
                global_data->last_contact = ImGui::GetTime();
                // For now, just print the Netstat.
                uint8_t netstat = clientserver_frame->getNetstat();
                dbprintlf(BLUE_FG "NETWORK STATUS");
                dbprintf("GUI Client ----- ");
                ((netstat & 0x80) == 0x80) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Roof UHF ------- ");
                ((netstat & 0x40) == 0x40) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Roof X-Band ---- ");
                ((netstat & 0x20) == 0x20) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Haystack ------- ");
                ((netstat & 0x10) == 0x10) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");

                // Extract the payload into a buffer.
                int payload_size = clientserver_frame->getPayloadSize();
                unsigned char *payload = (unsigned char *)malloc(payload_size);
                if (clientserver_frame->retrievePayload(payload, payload_size) < 0)
                {
                    dbprintlf("Error retrieving data.");
                    continue;
                }

                NETWORK_FRAME_TYPE type = clientserver_frame->getType();
                // Based on what we got, set things to display the data.
                switch (type)
                {
                case CS_TYPE_NULL:
                { // Will have status data.
                    dbprintlf("Received NULL frame.");
                    // global_data->netstat = clientserver_frame->getNetstat();
                    break;
                }
                case CS_TYPE_ACK:
                case CS_TYPE_NACK:
                {
                    dbprintlf("Received N/ACK.");
                    memcpy(global_data->cs_ack, payload, payload_size);
                    break;
                }
                case CS_TYPE_CONFIG_UHF:
                {
                    dbprintlf("Received UHF Config.");
                    memcpy(global_data->cs_config_uhf, payload, payload_size);
                    break;
                }
                case CS_TYPE_CONFIG_XBAND:
                {
                    dbprintlf("Received X-Band Config.");
                    memcpy(global_data->cs_config_xband, payload, payload_size);
                    break;
                }
                case CS_TYPE_DATA: // Data type is just cmd_output_t (SH->GS)
                {
                    dbprintlf("Received Data.");
                    // TODO: Assuming all 'data'-type frame payloads incoming to the Client is in the form of a from-SPACE-HAUC cmd_output_t. May be a poor assumption.
                    // If this is not an ACS Update...
                    if (((cmd_output_t *)payload)->mod != ACS_UPD_ID)
                    {
                        memcpy(global_data->cmd_output, payload, payload_size);
                    }
                    else
                    { // If it is an ACS update...
                        global_data->acs_rolbuf->addValueSet(*((acs_upd_output_t *)payload));
                    }
                    break;
                }
                case CS_TYPE_ERROR:
                default:
                {
                    break;
                }
                }
                free(payload);
            }
            else
            {
                break;
            }
        }
        if (read_size == 0)
        {
            dbprintlf(RED_BG "Connection forcibly closed by the server.");
            network_data->connection_ready = false;
            continue;
        }
        else if (errno == EAGAIN)
        {
            dbprintlf(YELLOW_BG "Active connection timed-out (%d).", read_size);
            network_data->connection_ready = false;
            continue;
        }
        erprintlf(errno);
    }

    return NULL;
}