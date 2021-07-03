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
    glfwSwapInterval(1); // Enables V-Sync.

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

    // Used by the Authentication Control Panel
    char password_buffer[64];
    memset(password_buffer, 0x0, 64);

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

        if (ImGui::Begin("Authentication Control Panel"))
        {
            ImGui::Text("ACCESS LEVEL %d GRANTED", authentication_access_level);

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

        static int ACS_command = ACS_INVALID_ID;
        static int EPS_command = EPS_INVALID_ID;
        static int XBAND_command = XBAND_INVALID_ID;
        static int UPD_command = INVALID_ID;
        static int SYS_command = INVALID_ID;

        static bool allow_transmission = false;
        static bool allow_receiving = true;

        static cmd_input_t ACS_command_input = {.mod = INVALID_ID, .cmd = ACS_INVALID_ID, .unused = 0, .data_size = 0};
        static cmd_input_t EPS_command_input = {.mod = INVALID_ID, .cmd = EPS_INVALID_ID, .unused = 0, .data_size = 0};
        static cmd_input_t XBAND_command_input = {.mod = INVALID_ID, .cmd = XBAND_INVALID_ID, .unused = 0, .data_size = 0};
        static cmd_input_t UPD_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
        static cmd_input_t SYS_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};

        if (ImGui::Begin("Communications Control Panel"))
        {
            ImGui::Checkbox("Attitude Control System", &ACS_window); // Contains ACS and ACS_UPD
            ImGui::Checkbox("Electrical Power Supply", &EPS_window);
            ImGui::Checkbox("X-Band", &XBAND_window);
            ImGui::Checkbox("Software Updater", &SW_UPD_window);
            ImGui::Checkbox("System Control", &SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

            ImGui::Separator();

            ImGui::Text("Queued Command");

            ImGui::Text("ACS: %d, EPS: %d, XBAND: %d, UPD: %d, SYS: %d", ACS_command, EPS_command, XBAND_command, UPD_command, SYS_command);

            ImGui::Separator();

            if (authentication_access_level >= 0)
            {
                ImGui::Checkbox("Unlock Transmissions", &allow_transmission);
            }
            else
            {
                ImGui::Text("ACCESS DENIED");
            }

            ImGui::End();
        }

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
                    ImGui::Text("ACCESS DENIED");
                }

                // TODO: Temporary, very much a placeholder. Needs to actually figure out the proper arugment formats and turn the inputs (from individual boxes per argument) to binary data before inputting. Also, this needs to display the data back to the user in a meaningful and useful format.
                ImGui::InputText("Data", (char *)ACS_command_input.data, 46); // Has to be desired number of characters + 1 (\0?)

                ACS_command_input.mod = ACS_ID;
                ACS_command_input.cmd = ACS_command;
                ACS_command_input.data_size = 46; // TEMPORARY
                // TODO: Change the .data_size to getting the actual size of data.

                ImGui::Separator();

                ImGui::Text("Transmit");

                if (allow_transmission)
                {

                    ImGui::Text("Queued Transmission");
                    ImGui::Text("Module ID:      0x%x", ACS_command_input.mod);
                    ImGui::Text("Command ID:     0x%x", ACS_command_input.cmd);
                    ImGui::Text("Data size:      0x%x", ACS_command_input.data_size);
                    ImGui::Text("Data:           ");
                    for (int i = 0; i < ACS_command_input.data_size; i++)
                    {
                        ImGui::Text("%x", ACS_command_input.data[i]);
                        ImGui::SameLine(0, 0);
                    }
                }
                else
                {
                    ImGui::Text("TRANSMISSIONS LOCKED");
                }

                ImGui::End();
            }
        }

        if (EPS_window)
        {
            if (ImGui::Begin("EPS Operations"))
            {
                ImGui::Text("Retrieval Commands");

                ImGui::RadioButton("Get Minimal Housekeeping", &EPS_command, EPS_GET_MIN_HK);
                ImGui::RadioButton("Get Battery Voltage", &EPS_command, EPS_GET_VBATT);
                ImGui::RadioButton("Get System Current", &EPS_command, EPS_GET_SYS_CURR);
                ImGui::RadioButton("Get Power Out", &EPS_command, EPS_GET_OUTPOWER);
                ImGui::RadioButton("Get Solar Voltage", &EPS_command, EPS_GET_VSUN);
                ImGui::RadioButton("Get Solar Voltage (All)", &EPS_command, EPS_GET_VSUN_ALL);
                ImGui::RadioButton("Get ISUN", &EPS_command, EPS_GET_ISUN);
                ImGui::RadioButton("Get Loop Timer", &EPS_command, EPS_GET_LOOP_TIMER);

                ImGui::Separator();

                ImGui::Text("Set Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set Loop Timer", &EPS_command, EPS_SET_LOOP_TIMER);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::End();
            }
        }

        if (XBAND_window)
        {
            if (ImGui::Begin("X-Band Operations"))
            {

                ImGui::Text("Retrieval Commands");

                ImGui::RadioButton("Get MAX ON", &XBAND_command, XBAND_GET_MAX_ON);
                ImGui::RadioButton("Get TMP SHDN", &XBAND_command, XBAND_GET_TMP_SHDN);
                ImGui::RadioButton("Get TMP OP", &XBAND_command, XBAND_GET_TMP_OP);
                ImGui::RadioButton("Get Loop Time", &XBAND_command, XBAND_GET_LOOP_TIME);

                ImGui::Separator();

                ImGui::Text("Set Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set Transmit", &XBAND_command, XBAND_SET_TX);
                    ImGui::RadioButton("Set Receive", &XBAND_command, XBAND_SET_RX);
                    ImGui::RadioButton("Set MAX ON", &XBAND_command, XBAND_SET_MAX_ON);
                    ImGui::RadioButton("Set TMP SHDN", &XBAND_command, XBAND_SET_TMP_SHDN);
                    ImGui::RadioButton("Set TMP OP", &XBAND_command, XBAND_SET_TMP_OP);
                    ImGui::RadioButton("Set LOOP TIME", &XBAND_command, XBAND_SET_LOOP_TIME);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::Separator();

                ImGui::Text("Perform Commands");

                if (authentication_access_level > 1)
                {
                    ImGui::RadioButton("Transmit", &XBAND_command, XBAND_DO_TX);
                    ImGui::RadioButton("Receive", &XBAND_command, XBAND_DO_RX);
                    ImGui::RadioButton("Disable", &XBAND_command, XBAND_DISABLE);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::End();
            }
        }

        // Handles software updates.
        if (SW_UPD_window)
        {
            if (ImGui::Begin("Software Updater Control Panel"))
            {
                ImGui::Text("Retrieval Commands");

                ImGui::Separator();

                ImGui::Text("Set Commands");

                if (authentication_access_level > 1)
                {
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::End();
            }
        }

        // Handles
        // SYS_VER_MAGIC = 0xd,
        // SYS_RESTART_PROG = 0xff,
        // SYS_REBOOT = 0xfe,
        // SYS_CLEAN_SHBYTES = 0xfd
        if (SYS_CTRL_window)
        {
            if (ImGui::Begin("System Control Panel"))
            {
                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Version Magic", &SYS_command, SYS_VER_MAGIC);
                    ImGui::RadioButton("Restart Program", &SYS_command, SYS_RESTART_PROG);
                    ImGui::RadioButton("Reboot", &SYS_command, SYS_REBOOT);
                    ImGui::RadioButton("Clean SHBYTES", &SYS_command, SYS_CLEAN_SHBYTES);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

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
