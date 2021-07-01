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

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

/**
 * @brief 
 * 
 * @param password 
 * @return int Authentication access level allowed.
 */
int gs_gui_check_password(char* password)
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