/**
 * @file gs_guimain.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief Ground Station GUI
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
#include <unistd.h>

int main(int, char **)
{
    ////////// INIT ///////////
    // Setup the window.
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(1280, 720, "SPACE-HAUC Ground Station", NULL, NULL);

    if (window == NULL)
    {
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // Enables V-Sync.

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enables keyboard navigation.

    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();

    // Setup platform / renderer backends.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // return 1;
    ///////////////////////////

    // Main loop prep.
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resizing, etc.).
        glfwPollEvents();

        // Start the Dear ImGui frame.
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Debug window (default).
        {
            ImGui::Text("%.1f FPS %.1f", ImGui::GetIO().Framerate, ImGui::GetTime());
        }

        // Level 0: Basic access, can retrieve data.
        // Level 1: Advanced access, can set some values.
        // Level 2: Project Manager access, can update flight software, edit critical systems.
        static int authentication_access_level = 0;
        // static bool authenticating = false;

        if (ImGui::Begin("Authentication Control Panel"))
        {
            ImGui::Text("ACCESS LEVEL %d GRANTED", authentication_access_level);

            char password_buffer[64];
            ImGui::InputText("", password_buffer, 64, ImGuiInputTextFlags_Password);
            ImGui::SameLine();
            if (ImGui::Button("AUTHENTICATE"))
            {
                authentication_access_level = gs_gui_check_password(password_buffer);
            }

            ImGui::End();
        }

        static bool ACS_window = false;
        static bool EPS_window = false;
        static bool XBAND_window = false;
        static bool SW_UPD_window = false;
        static bool SYS_CTRL_window = false;
        static bool COMMS_SENDER_window = false;

        if (ImGui::Begin("Communications Control Panel"))
        {
            ImGui::Checkbox("Attitude Control System", &ACS_window); // Contains ACS and ACS_UPD
            ImGui::Checkbox("Electrical Power Supply", &EPS_window);
            ImGui::Checkbox("X-Band", &XBAND_window);
            ImGui::Checkbox("Software Updater", &SW_UPD_window);
            ImGui::Checkbox("System Control", &SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

            ImGui::Separator();

            ImGui::Checkbox("Communications Sender", &COMMS_SENDER_window);

            // if (authentication_access_level < 2)
            // {
            //     SW_UPD_window = false;
            //     SYS_CTRL_window = false;
            // }

            ImGui::End();
        }

        static int ACS_command = ACS_INVALID_ID;

        if (ACS_window)
        {
            if (ImGui::Begin("ACS Operations"))
            {
                ImGui::Text("Retrieval Commands");

                ImGui::RadioButton("Get MOI ID", &ACS_command, ACS_GET_MOI);
                ImGui::RadioButton("Get IMOI ID", &ACS_command, ACS_GET_IMOI);
                ImGui::RadioButton("Get Dipole", &ACS_command, ACS_GET_DIPOLE);
                ImGui::RadioButton("Get Timestep", &ACS_command, ACS_GET_TSTEP);
                ImGui::RadioButton("Get Measure Time", &ACS_command, ACS_GET_MEASURE_TIME);
                ImGui::RadioButton("Get Leeway", &ACS_command, ACS_GET_LEEWAY);
                ImGui::RadioButton("Get W-Target", &ACS_command, ACS_GET_WTARGET);
                ImGui::RadioButton("Get Detumble Angle", &ACS_command, ACS_GET_DETUMBLE_ANG);
                ImGui::RadioButton("Get Sun Angle", &ACS_command, ACS_GET_SUN_ANGLE);

                ImGui::Separator();

                ImGui::Text("Set Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set MOI ID", &ACS_command, ACS_SET_MOI);
                    ImGui::RadioButton("Set IMOI ID", &ACS_command, ACS_SET_IMOI);
                    ImGui::RadioButton("Set Dipole", &ACS_command, ACS_SET_DIPOLE);
                    ImGui::RadioButton("Set Timestep", &ACS_command, ACS_SET_TSTEP);
                    ImGui::RadioButton("Set Measure Time", &ACS_command, ACS_SET_MEASURE_TIME);
                    ImGui::RadioButton("Set Leeway", &ACS_command, ACS_SET_LEEWAY);
                    ImGui::RadioButton("Set W-Target", &ACS_command, ACS_SET_WTARGET);
                    ImGui::RadioButton("Set Detumble Angle", &ACS_command, ACS_SET_DETUMBLE_ANG);
                    ImGui::RadioButton("Set Sun Angle", &ACS_command, ACS_SET_SUN_ANGLE);
                }
                else
                {
                    ImGui::Text("ACCESS LEVEL 1 OR 2 REQUIRED!");
                }

                ImGui::End();
            }
        }

        if (EPS_window)
        {
            if (ImGui::Begin("EPS Operations"))
            {
                ImGui::End();
            }
        }

        if (XBAND_window)
        {
            if (ImGui::Begin("X-Band Operations"))
            {
                ImGui::End();
            }
        }

        if (SW_UPD_window)
        {
            if (ImGui::Begin("Software Updater Control Panel"))
            {
                ImGui::End();
            }
        }

        if (SYS_CTRL_window)
        {
            if (ImGui::Begin("System Control Panel"))
            {
                ImGui::End();
            }
        }

        if (COMMS_SENDER_window)
        {
            if (ImGui::Begin("Communications Sender Operations"))
            {
                ImGui::Text("Level 3 authentication required!");

                ImGui::End();
            }
        }

        // Send Command Button Here

        // Rendering.
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    // Cleanup.
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
}
