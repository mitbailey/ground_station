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

    // TODO: Change to actually transmitting.
    printf("Pretending to transmit the following:\n");
    printf("0x%02x 0x%02x 0x%08x 0x%08x", input->mod, input->cmd, input->unused, input->data_size);
    for (int i = 0; i < input->data_size; i++)
    {
        printf(" 0x%02x", input->data[i]);
    }
    printf("\n");
    
    return 1;
}

int gs_receive(cmd_output_t *output)
{
    
}

// // TODO: Change each printout to actually send data to TX device.
// void* gs_acs_data_down_handler(void* vp/*acs_get_bool_t *acs_get_bool*/)
// {
//     acs_get_bool_t *acs = (acs_get_bool_t *) vp;

//     if (acs->moi)
//     {
//         printf("Pretending to poll for acs moi.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_MOI, 0x0, 0x0);
//     }
//     if (acs->imoi)
//     {
//         printf("Pretending to poll for acs imoi.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_IMOI, 0x0, 0x0);
//     }
//     if (acs->dipole)
//     {
//         printf("Pretending to poll for acs dipole.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_DIPOLE, 0x0, 0x0);
//     }
//     if (acs->tstep)
//     {
//         printf("Pretending to poll for acs tstep.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_TSTEP, 0x0, 0x0);
//     }
//     if (acs->measure_time)
//     {
//         printf("Pretending to poll for acs measure_time.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_MEASURE_TIME, 0x0, 0x0);
//     }
//     if (acs->leeway)
//     {
//         printf("Pretending to poll for acs leeway.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_LEEWAY, 0x0, 0x0);
//     }
//     if (acs->wtarget)
//     {
//         printf("Pretending to poll for acs wtarget.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_WTARGET, 0x0, 0x0);
//     }
//     if (acs->detumble_angle)
//     {
//         printf("Pretending to poll for acs detumble_angle.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_DETUMBLE_ANG, 0x0, 0x0);
//     }
//     if (acs->sun_angle)
//     {
//         printf("Pretending to poll for acs sun_angle.\n");
//         printf("mod: 0x%02x, cmd: 0x%02x, unused: 0x%02x, data_size: 0x%02x\n", ACS_ID, ACS_GET_SUN_ANGLE, 0x0, 0x0);
//     }

//     return vp;
// }

// unsigned int crc32b(unsigned char *message) {
//    int i, j;
//    unsigned int byte, crc, mask;

//    i = 0;
//    crc = 0xFFFFFFFF;
//    while (message[i] != 0) {
//       byte = message[i];            // Get next byte.
//       crc = crc ^ byte;
//       for (j = 7; j >= 0; j--) {    // Do eight times.
//          mask = -(crc & 1);
//          crc = (crc >> 1) ^ (0xEDB88320 & mask);
//       }
//       i = i + 1;
//    }
//    return ~crc;
// }

// int gs_gui_init(GLFWwindow *window)
// {
//     // Setup the window.
//     glfwSetErrorCallback(glfw_error_callback);
//     if (!glfwInit())
//     {
//         return -1;
//     }

//     window = glfwCreateWindow(1280, 720, "SPACE-HAUC Ground Station", NULL, NULL);

//     if (window == NULL)
//     {
//         return -1;
//     }

//     glfwMakeContextCurrent(window);
//     // glfwSwapInterval(1); // Enables V-Sync.

//     // Setup Dear ImGui context.
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO &io = ImGui::GetIO();
//     (void)io;

//     io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enables keyboard navigation.

//     // Setup Dear ImGui style.
//     ImGui::StyleColorsDark();

//     // Setup platform / renderer backends.
//     ImGui_ImplGlfw_InitForOpenGL(window, true);
//     ImGui_ImplOpenGL2_Init();

//     return 1;
// }