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

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
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