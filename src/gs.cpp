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
#include "sw_update_packdef.h"
#include "phy.hpp"

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// /**
//  * @brief 
//  * 
//  * From:
//  * https://github.com/sunipkmukherjee/comic-mon/blob/master/guimain.cpp
//  * with minor modifications.
//  * 
//  * @param socket 
//  * @param address 
//  * @param socket_size 
//  * @param tout_s 
//  * @return int 
//  */
// int gs_connect(int socket, const struct sockaddr *address, socklen_t socket_size, int tout_s)
// {
//     int res;
//     long arg;
//     fd_set myset;
//     struct timeval tv;
//     int valopt;
//     socklen_t lon;

//     // Set non-blocking.
//     if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
//     {
//         dbprintlf(RED_FG "Error fcntl(..., F_GETFL)");
//         erprintlf(errno);
//         return -1;
//     }
//     arg |= O_NONBLOCK;
//     if (fcntl(socket, F_SETFL, arg) < 0)
//     {
//         dbprintlf(RED_FG "Error fcntl(..., F_SETFL)");
//         erprintlf(errno);
//         return -1;
//     }

//     // Trying to connect with timeout.
//     res = connect(socket, address, socket_size);
//     if (res < 0)
//     {
//         if (errno == EINPROGRESS)
//         {
//             dbprintlf(YELLOW_FG "EINPROGRESS in connect() - selecting");
//             do
//             {
//                 if (tout_s > 0)
//                 {
//                     tv.tv_sec = tout_s;
//                 }
//                 else
//                 {
//                     tv.tv_sec = 1; // Minimum 1 second.
//                 }
//                 tv.tv_usec = 0;
//                 FD_ZERO(&myset);
//                 FD_SET(socket, &myset);
//                 res = select(socket + 1, NULL, &myset, NULL, &tv);
//                 if (res < 0 && errno != EINTR)
//                 {
//                     dbprintlf(RED_FG "Error connecting.");
//                     erprintlf(errno);
//                     return -1;
//                 }
//                 else if (res > 0)
//                 {
//                     // Socket selected for write.
//                     lon = sizeof(int);
//                     if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (void *)(&valopt), &lon) < 0)
//                     {
//                         dbprintlf(RED_FG "Error in getsockopt()");
//                         erprintlf(errno);
//                         return -1;
//                     }

//                     // Check the value returned...
//                     if (valopt)
//                     {
//                         dbprintlf(RED_FG "Error in delayed connection()");
//                         erprintlf(valopt);
//                         return -1;
//                     }
//                     break;
//                 }
//                 else
//                 {
//                     dbprintlf(RED_FG "Timeout in select(), cancelling!");
//                     return -1;
//                 }
//             } while (1);
//         }
//         else
//         {
//             fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno));
//             dbprintlf(RED_FG "Error connecting.");
//             erprintlf(errno);
//             return -1;
//         }
//     }
//     // Set to blocking mode again...
//     if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
//     {
//         dbprintlf("Error fcntl(..., F_GETFL)");
//         erprintlf(errno);
//         return -1;
//     }
//     arg &= (~O_NONBLOCK);
//     if (fcntl(socket, F_SETFL, arg) < 0)
//     {
//         dbprintlf("Error fcntl(..., F_GETFL)");
//         erprintlf(errno);
//         return -1;
//     }
//     return socket;
// }

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
    global_data_t *global = (global_data_t *)global_data_vp;

    cmd_input_t acs_cmd[1];
    memset(acs_cmd, 0x0, sizeof(cmd_input_t));

    acs_cmd->mod = ACS_UPD_ID;
    acs_cmd->unused = 0x0;
    acs_cmd->data_size = 0x0;
    memset(acs_cmd->data, 0x0, MAX_DATA_SIZE);

    // Transmit an ACS update request to the server.
    // gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, acs_cmd, sizeof(cmd_input_t));
    NetFrame *network_frame = new NetFrame((unsigned char *)acs_cmd, sizeof(cmd_input_t), NetType::DATA, NetVertex::ROOFUHF);
    network_frame->sendFrame(global->network_data);
    delete network_frame;

    // !WARN! Any faster than 0.5 seconds seems to break the Network.
    usleep(ACS_UPDATE_FREQUENCY SEC);

    pthread_mutex_unlock(&global->acs_rolbuf->acs_upd_inhibitor);

    return NULL;
}

// int gs_transmit(NetworkData *network_data, NETWORK_FRAME_TYPE type, NETWORK_FRAME_ENDPOINT endpoint, void *data, int data_size)
// {
//     if (data_size < 0)
//     {
//         printf("Error: data_size is %d.\n", data_size);
//         printf("Cancelling transmit.\n");
//         return -1;
//     }

//     // Create a NetworkFrame to send our data in.
//     NetworkFrame *clientserver_frame = new NetworkFrame(type, data_size);
//     clientserver_frame->storePayload(endpoint, data, data_size);

//     clientserver_frame->sendFrame(network_data);

//     return 1;
// }

// void *gs_polling_thread(void *args)
// {
//     global_data_t *global_data = (global_data_t *)args;
//     NetworkData *network_data = global_data->network_data;

//     while (network_data->rx_active)
//     {
//         if (network_data->connection_ready)
//         {
//             NetworkFrame *null_frame = new NetworkFrame(CS_TYPE_NULL, 0x0);
//             null_frame->storePayload(CS_ENDPOINT_SERVER, NULL, 0);

//             // send(network_data->socket, null_frame, sizeof(NetworkFrame), 0);
//             null_frame->sendFrame(network_data);
//             delete null_frame;
//         }

//         usleep(SERVER_POLL_RATE SEC);
//     }

//     dbprintlf(FATAL "GS_POLLING_THREAD IS EXITING!");
//     return NULL;
// }

// Updated, referenced "void *rcv_thr(void *sock)" from line 338 of: https://github.com/sunipkmukherjee/comic-mon/blob/master/guimain.cpp
// Also see: https://github.com/mitbailey/socket_server
void *gs_rx_thread(void *args)
{
    // Convert the passed void pointer into something useful; in this case, global_data_t.
    global_data_t *global_data = (global_data_t *)args;
    NetDataClient *network_data = global_data->network_data;

    while (network_data->recv_active)
    {
        if (!network_data->connection_ready)
        {
            sleep(5);
            continue;
        }

        int read_size = 0;

        while (read_size >= 0 && network_data->recv_active)
        {
            char buffer[sizeof(NetFrame) * 2];
            memset(buffer, 0x0, sizeof(buffer));

            dbprintlf(BLUE_BG "Waiting to receive...");
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
                NetFrame *network_frame = (NetFrame *)buffer;

                // Check if we've received data in the form of a NetworkFrame.
                if (network_frame->validate() < 0)
                {
                    dbprintlf("Integrity check failed (%d).", network_frame->validate());
                    continue;
                }
                dbprintlf("Integrity check successful.");

                global_data->netstat = network_frame->getNetstat();
                global_data->last_contact = ImGui::GetTime();
                // For now, just print the Netstat.
                uint8_t netstat = network_frame->getNetstat();
                dbprintlf(BLUE_FG "NETWORK STATUS");
                dbprintf("GUI Client ----- ");
                ((netstat & 0x80) == 0x80) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Roof UHF ------- ");
                ((netstat & 0x40) == 0x40) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Roof X-Band ---- ");
                ((netstat & 0x20) == 0x20) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Haystack ------- ");
                ((netstat & 0x10) == 0x10) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");
                dbprintf("Track ---------- ");
                ((netstat & 0x5) == 0x5) ? printf(GREEN_FG "ONLINE" RESET_ALL "\n") : printf(RED_FG "OFFLINE" RESET_ALL "\n");

                // Extract the payload into a buffer.
                int payload_size = network_frame->getPayloadSize();
                unsigned char *payload = (unsigned char *)malloc(payload_size);
                if (network_frame->retrievePayload(payload, payload_size) < 0)
                {
                    dbprintlf("Error retrieving data.");
                    continue;
                }

                // Based on what we got, set things to display the data.
                switch (network_frame->getType())
                {
                case NetType::POLL:
                { // Will have status data.
                    dbprintlf("Received NULL frame.");
                    // global_data->netstat = clientserver_frame->getNetstat();
                    break;
                }
                case NetType::ACK:
                {
                    dbprintlf("Received ACK.");
                    memcpy(global_data->cs_ack, payload, payload_size);
                    break;
                }
                case NetType::NACK:
                {
                    dbprintlf("Received N/ACK.");
                    memcpy(global_data->cs_ack, payload, payload_size);

                    if (((cs_ack_t *)payload)->code == NACK_NO_UHF)
                    {
                        // Immediately cancel all ongoing software updates, since the Roof UHF is complaining that it cannot use the UHF.
                        dbprintlf(RED_FG "Roof UHF responded saying that it cannot access UHF communications at this time. Halting all software updates.");
                        global_data->sw_updating = false;
                    }

                    break;
                }
                case NetType::UHF_CONFIG:
                {
                    dbprintlf("Received UHF Config.");
                    memcpy(global_data->cs_config_uhf, payload, payload_size);
                    break;
                }
                case NetType::XBAND_CONFIG:
                {
                    dbprintlf("Received X-Band Config.");
                    memcpy(global_data->cs_config_xband, payload, payload_size);
                    break;
                }
                case NetType::DATA: // Data type is just cmd_output_t (SH->GS)
                {
                    // ASSERTION: All 'DATA'-type frame payloads incoming to the client is in the form of a from-SPACE-HAUC cmd_output_t.

                    if (((cmd_output_t *)payload)->mod == SW_UPD_ID)
                    { // If this is part of an sw_update...
                        // If we can't get the lock, only wait for one second.
                        struct timespec timeout;
                        clock_gettime(CLOCK_REALTIME, &timeout);
                        timeout.tv_sec += 1;

                        if (pthread_mutex_timedlock(global_data->sw_output_lock, &timeout) == 0)
                        {
                            memcpy(global_data->sw_output, payload, payload_size);
                            global_data->sw_output_fresh = true;
                            pthread_mutex_unlock(global_data->sw_output_lock);
                        }
                        else
                        {
                            dbprintlf(RED_FG "Failed to acquire sw_data_lock.");
                        }
                    }
                    else if (((cmd_output_t *)payload)->mod != ACS_UPD_ID)
                    { // If this is not an ACS Update...
                        memcpy(global_data->cmd_output, payload, payload_size);
                    }
                    else
                    { // If it is an ACS update...
                        global_data->acs_rolbuf->addValueSet(*((acs_upd_output_t *)payload));
                    }
                    break;
                }
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
            strcpy(network_data->disconnect_reason, "SERVER-FORCED");
            network_data->connection_ready = false;
            continue;
        }
        else if (errno == EAGAIN)
        {
            dbprintlf(YELLOW_BG "Active connection timed-out (%d).", read_size);
            strcpy(network_data->disconnect_reason, "TIMED-OUT");
            network_data->connection_ready = false;
            continue;
        }
        erprintlf(errno);
    }

    network_data->recv_active = false;
    dbprintlf(FATAL "DANGER! RECEIVE THREAD IS RETURNING!");
    return NULL;
}

// TODO: Make this into a thread so that the rest of the program will continue running.
// NOTE: The RX thread copies all SW-related data into global_data->sw_output and sets the new_sw_data flag.
void *gs_sw_send_file_thread(void *args)
{
    global_data_t *global = (global_data_t *)args;
    dbprintlf("Checkpoint 1.");
    char directory[20];
    snprintf(directory, 20, global->directory);
    dbprintlf("Checkpoint 2.");
    char filename[20];
    snprintf(filename, 20, global->filename);
    dbprintlf("Checkpoint 3.");

    // int gs_sw_send_file(global_data_t *global_data, const char directory[], const char filename[], bool *done_upld)
    // {
    if (!global->network_data->connection_ready)
    {
        dbprintlf("Checkpoint 4.");
        dbprintlf(RED_FG "Connection is not ready: update aborted.");
        global->sw_updating = false;
        return NULL;
    }
    if (filename == NULL)
    {
        dbprintlf("Checkpoint 4.");
        dbprintlf(RED_FG "File name not supplied.");
        global->sw_updating = false;
        return NULL;
    }
    else if (strlen(filename) >= SW_UPD_FN_SIZE)
    {
        dbprintlf("Checkpoint 4.");
        dbprintlf(RED_FG "File name too long.");
        global->sw_updating = false;
        return NULL;
    }
    dbprintlf("Checkpoint 4.");
    char directory_filename[SW_UPD_FN_SIZE + 32];
    snprintf(directory_filename, sizeof(directory_filename), "%s%s", directory, filename);
    FILE *bin_fp = fopen(directory_filename, "rb");

    if (bin_fp == NULL)
    {
        dbprintlf(RED_FG "Could not open %s.", directory_filename);
        global->sw_updating = false;
        return NULL;
    }

    // Find the size of the binary file.
    fseek(bin_fp, 0, SEEK_END);
    ssize_t file_size = ftell(bin_fp);
    fseek(bin_fp, 0, SEEK_SET);

    dbprintlf("Beginning send of %s (%d bytes).", directory_filename, file_size);

    ssize_t sent_bytes = gs_sw_get_sent_bytes(filename);

    if (sent_bytes < 0)
    {
        dbprintlf(RED_FG "Failed to retrieve sent bytes for %s (%d).", directory_filename, sent_bytes);
        global->sw_updating = false;
        return NULL;
    }

    volatile int sent_packets = (sent_bytes / SW_UPD_DATA_SIZE_MAX) + ((sent_bytes % SW_UPD_DATA_SIZE_MAX) > 0);
    ssize_t fn_sz = strlen(filename) + 1;
    int send_attempts = 0;
    int max_packets = (file_size / SW_UPD_DATA_SIZE_MAX) + ((file_size % SW_UPD_DATA_SIZE_MAX) > 0);
    ssize_t retval = 0;

    // Out initial state is to begin by sending primers until we get a good reply.
    sw_upd_mode mode = primer;

    // Primers, headers, and buffers.
    char rd_buf[SW_UPD_PACKET_SIZE];
    char wr_buf[SW_UPD_PACKET_SIZE];

    dbprintlf("Entering file transfer phase.");

    // Outer loop. Runs until we have sent the entire file.
    while ((mode != finish) && global->sw_updating)
    {
        // Each loop we should set sent_bytes and sent_packets.
        sent_bytes = gs_sw_get_sent_bytes(filename);
        sent_packets = (sent_bytes / SW_UPD_DATA_SIZE_MAX) + ((sent_bytes % SW_UPD_DATA_SIZE_MAX) > 0);
        global->sw_upd_packet = sent_packets;

        ssize_t in_sz = 0;

        // Remember to clean your memory and drink your Ovaltine.
        memset(wr_buf, 0x0, SW_UPD_PACKET_SIZE);

        switch (mode)
        {
        case primer:
        {
            sw_upd_startresume_t *sr_pmr = (sw_upd_startresume_t *)wr_buf;
            sw_upd_startresume_reply_t *sr_rep = (sw_upd_startresume_reply_t *)rd_buf;

            sr_pmr->cmd = SW_UPD_SRID;
            strcpy(sr_pmr->filename, filename);
            sr_pmr->fid = 1;
            sr_pmr->sent_bytes = sent_bytes;
            sr_pmr->total_bytes = file_size;

            // Send the START/RESUME primer, and get back a reply
            for (send_attempts = 0; (send_attempts < SW_UPD_MAX_SEND_ATTEMPTS) && global->sw_updating; send_attempts++)
            {
                dbprintlf("Sending S/R primer.");

                // retval = gs_transmit(global->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, wr_buf, SW_UPD_PACKET_SIZE);
                NetFrame *network_frame = new NetFrame((unsigned char *)wr_buf, SW_UPD_PACKET_SIZE, NetType::DATA, NetVertex::ROOFUHF);
                network_frame->sendFrame(global->network_data);
                delete network_frame;

                if (retval <= 0)
                {
                    dbprintlf(RED_FG "S/R primer writing failed (%d).", retval);
                    continue;
                }

                dbprintlf("Will await N/ACK.");
                memset(rd_buf, 0x0, SW_UPD_PACKET_SIZE);

                // TODO: Figure out a better way to wait for new data.
                while (!global->sw_output_fresh && global->sw_updating)
                {
                    dbprintlf("Waiting for response...");
                    usleep(0.1 SEC);
                }
                if (!global->sw_updating)
                {
                    // Update aborted.
                    dbprintlf(YELLOW_FG "Update aborted.");
                    return NULL;
                }

                // Timeout after waiting for 15 seconds.
                struct timespec timeout;
                clock_gettime(CLOCK_REALTIME, &timeout);
                timeout.tv_sec += 15;

                int retval = pthread_mutex_timedlock(global->sw_output_lock, &timeout);

                if (retval == 0)
                {
                    memcpy(rd_buf, global->sw_output->data, global->sw_output->data_size);
                    global->sw_output_fresh = false;
                    pthread_mutex_unlock(global->sw_output_lock);
                }
                else if (retval == ETIMEDOUT)
                {
                    // NOTE: Continues if timeout.
                    dbprintlf(YELLOW_FG "Lock acquisition timed out: failed to receive a response from SPACE-HAUC.");
                    continue;
                }
                else
                {
                    dbprintlf(FATAL "Failed to acquire sw_data_lock.");
                    continue;
                }

                // TODO: Figure out how to ask for a repeat if necessary.
                // Repeats are sent like so:
                // gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, rept_cmd, sizeof(rept_cmd));

                if (!memcmp(rept_cmd, rd_buf, 5))
                { // We read in a REPT CMD, so repeat last.
                    dbprintlf(YELLOW_FG "Repeat of previous transmission requested.");
                    continue;
                }

                if (sr_rep->cmd != SW_UPD_SRID)
                {
                    continue;
                }
                else if (memcmp((char *)sr_rep->filename, filename, fn_sz) != 0)
                {
                    continue;
                }
                else if (sr_rep->recv_bytes != sent_bytes)
                {
                    // We should yield to SH here.
                    gs_sw_set_sent_bytes(filename, sr_rep->recv_bytes);
                    sent_bytes = sr_rep->recv_bytes;
                    sent_packets = (sent_bytes / SW_UPD_DATA_SIZE_MAX) + ((sent_bytes % SW_UPD_DATA_SIZE_MAX) > 0);

                    continue;
                }
                else if (sr_rep->total_packets != max_packets)
                {
                    continue;
                }
                else
                {
                    mode = data;
                    break;
                }
            }
            if (!global->sw_updating)
            {
                // Update aborted.
                dbprintlf(YELLOW_FG "Update aborted.");
                return NULL;
            }

            break; // case primer
        }

        case data:
        {

            memset(wr_buf, 0x0, SW_UPD_PACKET_SIZE);
            sw_upd_data_t *dt_hdr = (sw_upd_data_t *)wr_buf;

            sw_upd_data_reply_t *dt_rep = (sw_upd_data_reply_t *)rd_buf;

            for (send_attempts = 0; (send_attempts < SW_UPD_MAX_SEND_ATTEMPTS) && global->sw_updating; send_attempts++)
            {
                fseek(bin_fp, sent_packets * SW_UPD_DATA_SIZE_MAX, SEEK_SET);
                in_sz = fread(wr_buf + sizeof(sw_upd_data_t), 0x1, SW_UPD_DATA_SIZE_MAX, bin_fp);

                if (in_sz <= 0)
                {
                    dbprintlf(RED_FG "Reached EOF when retrieving packet %d.", sent_packets);
                    break;
                }

                dt_hdr->cmd = SW_UPD_DTID;
                dt_hdr->packet_number = sent_packets;
                dt_hdr->total_bytes = file_size;
                dt_hdr->data_size = in_sz;

                memcpy(wr_buf, dt_hdr, sizeof(sw_upd_data_t));

                // retval = gs_transmit(global->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, wr_buf, SW_UPD_PACKET_SIZE);
                NetFrame *network_frame = new NetFrame((unsigned char *)wr_buf,  SW_UPD_PACKET_SIZE, NetType::DATA, NetVertex::ROOFUHF);
                network_frame->sendFrame(global->network_data);
                delete network_frame;

                if (retval <= 0)
                {
                    dbprintlf(RED_FG "DATA packet writing failed (%d).", retval);
                    continue;
                }

                dbprintlf("Will await N/ACK.");
                memset(rd_buf, 0x0, SW_UPD_PACKET_SIZE);

                // TODO: Figure out a better way to wait for new data.
                while (!global->sw_output_fresh && global->sw_updating)
                {
                    dbprintlf("Waiting for response...");
                    usleep(0.1 SEC);
                }

                // Timeout after waiting for 15 seconds.
                struct timespec timeout;
                clock_gettime(CLOCK_REALTIME, &timeout);
                timeout.tv_sec += 15;

                int retval = pthread_mutex_timedlock(global->sw_output_lock, &timeout);

                if (retval == 0)
                {
                    memcpy(rd_buf, global->sw_output->data, global->sw_output->data_size);
                    global->sw_output_fresh = false;
                    pthread_mutex_unlock(global->sw_output_lock);
                }
                else if (retval == ETIMEDOUT)
                {
                    // NOTE: Continues if timeout.
                    dbprintlf(YELLOW_FG "Lock acquisition timed out: failed to receive a response from SPACE-HAUC.");
                    continue;
                }
                else
                {
                    dbprintlf(FATAL "Failed to acquire sw_data_lock.");
                    continue;
                }

                // TODO: Figure out how to ask for a repeat if necessary.
                // Repeats are sent like so:
                // gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, rept_cmd, sizeof(rept_cmd));

                if (!memcmp(rept_cmd, rd_buf, 5))
                { // We read in a REPT CMD, so repeat last.
                    dbprintlf(YELLOW_FG "Repeat of previous transmission requested.");
                    continue;
                }

                if (dt_rep->cmd != SW_UPD_DTID)
                {
                    continue;
                }
                else if (dt_rep->packet_number != sent_packets)
                {
                    mode = primer;
                    break;
                }
                else if (dt_rep->total_packets != max_packets)
                {
                    continue;
                }
                else
                {
                    sent_bytes += dt_hdr->data_size;
                    gs_sw_set_sent_bytes(filename, sent_bytes);
                    sent_packets++;

                    if (sent_bytes >= file_size)
                    {
                        mode = transfer_complete;
                    }

                    break;
                }
            }
            if (!global->sw_updating)
            {
                // Update aborted.
                dbprintlf(YELLOW_FG "Update aborted.");
                return NULL;
            }
            break; // case data
        }

        case transfer_complete:
        {
            if (sent_bytes == file_size)
            {
                // Complete
                dbprintlf("File transfer complete with %ld/%ld bytes of %s having been successfully sent and confirmed per packet.", sent_bytes, file_size, filename);
            }
            else if (global->sw_updating)
            {
                // Interrupted
                dbprintlf(YELLOW_FG "File transfer interrupted with %ld/%ld bytes of %s having been successfully sent and confirmed per packet.", sent_bytes, file_size, filename);
            }
            else if (sent_bytes <= 0)
            {
                // Error
                dbprintlf(RED_FG "An error has been encountered with %ld/%ld bytes of %s sent.", sent_bytes, file_size, filename);
                global->sw_updating = false;
                return NULL;
            }
            else
            {
                /// NOTE: Will reach this case if (recv_bytes != file_size).
                // ???
                dbprintlf(FATAL "Confused.");
                global->sw_updating = false;
                return NULL;
            }

            mode = confirmation;

            break; // case transfer_complete
        }

        case confirmation:
        {
            sw_upd_conf_t *cf_hdr = (sw_upd_conf_t *)wr_buf;
            sw_upd_conf_reply_t *cf_rep = (sw_upd_conf_reply_t *)rd_buf;

            cf_hdr->cmd = SW_UPD_CFID;
            cf_hdr->packet_number = sent_packets;
            cf_hdr->total_packets = max_packets;

            checksum_md5(directory_filename, cf_hdr->hash, 32);

            // retval = gs_transmit(global->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, wr_buf, SW_UPD_PACKET_SIZE);
            NetFrame *network_frame = new NetFrame((unsigned char *)wr_buf,  SW_UPD_PACKET_SIZE, NetType::DATA, NetVertex::ROOFUHF);
            network_frame->sendFrame(global->network_data);
            delete network_frame;

            if (retval <= 0)
            {
                dbprintlf(RED_FG "CF header writing failed (%d).", retval);
                continue;
            }

            dbprintlf("Will await N/ACK.");
            memset(rd_buf, 0x0, SW_UPD_PACKET_SIZE);

            // TODO: Figure out a better way to wait for new data.
            while (!global->sw_output_fresh && global->sw_updating)
            {
                dbprintlf("Waiting for response...");
                usleep(0.1 SEC);
            }
            if (!global->sw_updating)
            {
                // Update aborted.
                dbprintlf(YELLOW_FG "Update aborted.");
                return NULL;
            }

            // Timeout after waiting for 15 seconds.
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 15;

            int retval = pthread_mutex_timedlock(global->sw_output_lock, &timeout);

            if (retval == 0)
            {
                memcpy(rd_buf, global->sw_output->data, global->sw_output->data_size);
                global->sw_output_fresh = false;
                pthread_mutex_unlock(global->sw_output_lock);
            }
            else if (retval == ETIMEDOUT)
            {
                // NOTE: Continues if timeout.
                dbprintlf(YELLOW_FG "Lock acquisition timed out: failed to receive a response from SPACE-HAUC.");
                continue;
            }
            else
            {
                dbprintlf(FATAL "Failed to acquire sw_data_lock.");
                continue;
            }

            // TODO: Figure out how to ask for a repeat if necessary.
            // Repeats are sent like so:
            // gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, rept_cmd, sizeof(rept_cmd));

            if (!memcmp(rept_cmd, rd_buf, 5))
            { // We read in a REPT CMD, so repeat last.
                dbprintlf(YELLOW_FG "Repeat of previous transmission requested.");
                continue;
            }

            if (cf_rep->cmd != SW_UPD_CFID)
            {
                break;
            }
            else if (cf_rep->total_packets != max_packets)
            {
                break;
            }
            else if (cf_rep->request_packet == REQ_PKT_RESEND)
            {
                break;
            }
            else if (cf_rep->request_packet >= 0)
            {
                if (cf_rep->request_packet > max_packets)
                {
                    break;
                }
                else if (cf_rep->request_packet <= max_packets)
                {
                    sent_packets = cf_rep->request_packet;
                    sent_bytes = SW_UPD_DATA_SIZE_MAX * sent_packets;
                    gs_sw_set_sent_bytes(filename, sent_bytes);
                    mode = primer;
                    break;
                }
            }
            else if (memcmp(cf_rep->hash, cf_hdr->hash, SW_UPD_HASH_SIZE) != 0)
            {
                dbprintlf(FATAL "Restarting file transfer.");
                sent_packets = 0;
                sent_bytes = 0;
                gs_sw_set_sent_bytes(filename, 0);
                mode = primer;
            }
            else
            {
                dbprintlf(BLUE_BG "File transfer complete.");
                mode = finish;
            }

            break; // case confirmation
        }

        case finish:
        {
            dbprintlf("The file transfer is now complete.");
            global->sw_upd_packet = -1;
            break; // case finish
        }
        }
    }

    global->sw_updating = false;
    return NULL;
}

ssize_t gs_sw_get_sent_bytes(const char filename[])
{
    // Read from {filename}.sent_bytes to see if this file was already mid-transfer and needs to continue at some specific point.
    char filename_bytes[128];
    snprintf(filename_bytes, 128, "%s.%s", filename, "gsbytes");

    int bytes_fp = -1;
    ssize_t sent_bytes = 0;

    if (access(filename_bytes, F_OK) == 0)
    {
        // File exists.
        bytes_fp = open(filename_bytes, O_RDONLY);
        if (bytes_fp < 3)
        {
            dbprintlf(RED_FG "%s exists but could not be opened.", filename_bytes);
            return ERR_FILE_OPEN;
        }
        lseek(bytes_fp, 0, SEEK_SET);
        if (read(bytes_fp, &sent_bytes, sizeof(ssize_t)) != sizeof(ssize_t))
        {
            dbprintlf(RED_FG "Error reading sent_bytes.");
        }
        dbprintlf(YELLOW_FG "%ld bytes of current transfer previously received by SH.", sent_bytes);
        close(bytes_fp);
    }
    else
    {
        // File does not exist.
        bytes_fp = open(filename_bytes, O_CREAT | O_EXCL, 0755);
        if (bytes_fp < 3)
        {
            dbprintlf(RED_FG "%s does not exist and could not be created.", filename_bytes);
            return ERR_FILE_OPEN;
        }
        lseek(bytes_fp, 0, SEEK_SET);
        int retval = write(bytes_fp, &sent_bytes, sizeof(ssize_t));
        if (retval != sizeof(ssize_t))
        {
            dbprintlf(RED_FG "Error %d", retval);
        }
        dbprintlf(YELLOW_FG "%s does not exist. Assuming transfer should start at packet 0.", filename_bytes);
        close(bytes_fp);
    }
    sync();
    return sent_bytes;
}

int gs_sw_set_sent_bytes(const char filename[], ssize_t sent_bytes)
{
    // Overwrite {filename}.bytes to contain {sent_bytes}.
    char filename_bytes[128];
    snprintf(filename_bytes, 128, "%s.%s", filename, "gsbytes");

    int bytes_fp = -1;
    bytes_fp = open(filename_bytes, O_RDWR | O_TRUNC);
    if (bytes_fp < 3)
    {
        dbprintlf(RED_FG "Could not open %s for overwriting.", filename_bytes);
        return ERR_FILE_OPEN;
    }
    lseek(bytes_fp, 0, SEEK_SET);
    if (write(bytes_fp, &sent_bytes, sizeof(ssize_t)) != sizeof(ssize_t))
    {
        dbprintlf(RED_FG "Could not write to %s", filename_bytes);
    }
    close(bytes_fp);

    sync();
    return 1;
}
