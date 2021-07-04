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

        static acs_set_data_t acs_set_data = {0};
        static acs_get_bool_t acs_get_bool = {0};

        if (ACS_window)
        {
            if (ImGui::Begin("ACS Operations"))
            {
                ImGui::Text("Data-down Commands");

                ImGui::Checkbox("Get MOI", &acs_get_bool.moi);
                ImGui::Checkbox("Get IMOI", &acs_get_bool.imoi);
                ImGui::Checkbox("Get Dipole", &acs_get_bool.dipole);
                ImGui::Checkbox("Get Timestep", &acs_get_bool.tstep);
                ImGui::Checkbox("Get Measure Time", &acs_get_bool.measure_time);
                ImGui::Checkbox("Get Leeway", &acs_get_bool.leeway);
                ImGui::Checkbox("Get W-Target", &acs_get_bool.wtarget);
                ImGui::Checkbox("Get Detumble Angle", &acs_get_bool.detumble_angle);
                ImGui::Checkbox("Get Sun Angle", &acs_get_bool.sun_angle);

                ImGui::Separator();

                ImGui::Text("Data-up Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set MOI", &ACS_command, ACS_SET_MOI); // 9x floats
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
                    ImGui::InputInt("Timestep (ms)", (int *)&acs_set_data.tstep);
                    ImGui::RadioButton("Set Measure Time", &ACS_command, ACS_SET_MEASURE_TIME); // 1x uint8_t
                    ImGui::InputInt("Measure Time (ms)", (int *)&acs_set_data.measure_time);
                    ImGui::RadioButton("Set Leeway", &ACS_command, ACS_SET_LEEWAY); // 1x uint8_t
                    ImGui::InputInt("Leeway Factor", (int *)&acs_set_data.leeway);
                    ImGui::RadioButton("Set W-Target", &ACS_command, ACS_SET_WTARGET); // 1x float
                    ImGui::InputFloat("W-Target", &acs_set_data.wtarget);
                    ImGui::RadioButton("Set Detumble Angle", &ACS_command, ACS_SET_DETUMBLE_ANG); // 1x uint8_t
                    ImGui::InputInt("Angle", (int *)&acs_set_data.detumble_angle);
                    ImGui::RadioButton("Set Sun Angle", &ACS_command, ACS_SET_SUN_ANGLE); // 1x uint8_t
                    ImGui::InputInt("Sun Angle", (int *)&acs_set_data.sun_angle);
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
                        ImGui::Text("%2x", ACS_command_input.data[i]);
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

        static eps_set_data_t eps_set_data = {0};
        static eps_get_bool_t eps_get_bool = {0};

        if (EPS_window)
        {
            if (ImGui::Begin("EPS Operations"))
            {
                ImGui::Text("Data-down Commands");

                ImGui::Checkbox("Get Minimal Housekeeping", &eps_get_bool.min_hk);
                ImGui::Checkbox("Get Battery Voltage", &eps_get_bool.vbatt);
                ImGui::Checkbox("Get System Current", &eps_get_bool.sys_curr);
                ImGui::Checkbox("Get Power Out", &eps_get_bool.outpower);
                ImGui::Checkbox("Get Solar Voltage", &eps_get_bool.vsun);
                ImGui::Checkbox("Get Solar Voltage (All)", &eps_get_bool.vsun_all);
                ImGui::Checkbox("Get ISUN", &eps_get_bool.isun);
                ImGui::Checkbox("Get Loop Timer", &eps_get_bool.loop_timer);

                ImGui::Separator();

                ImGui::Text("Data-up Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set Loop Timer", &EPS_command, EPS_SET_LOOP_TIMER);
                    ImGui::InputInt("Loop Time (seconds)", (int *)&eps_set_data.loop_timer);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::End();
            }
        }

        static xband_set_data_array_t xband_set_data = {0};
        static xband_tx_data_t xband_tx_data = {0};
        static xband_rxtx_data_t xband_rxtx_data = {0};
        static xband_get_bool_t xband_get_bool = {0};

        if (XBAND_window)
        {
            if (ImGui::Begin("X-Band Operations"))
            {

                ImGui::Text("Data-down Commands");

                ImGui::Checkbox("Get MAX ON", &xband_get_bool.max_on);
                ImGui::Checkbox("Get TMP SHDN", &xband_get_bool.tmp_shdn);
                ImGui::Checkbox("Get TMP OP", &xband_get_bool.tmp_op);
                ImGui::Checkbox("Get Loop Time", &xband_get_bool.loop_time);

                ImGui::Separator();

                ImGui::Text("Data-up Commands");

                if (authentication_access_level > 0)
                {
                    ImGui::RadioButton("Set Transmit", &XBAND_command, XBAND_SET_TX);
                    ImGui::InputFloat("TX LO", &xband_set_data.TX.LO);
                    ImGui::InputFloat("TX bw", &xband_set_data.TX.bw);
                    ImGui::InputInt("TX Samp", (int *)&xband_set_data.TX.samp);
                    ImGui::InputInt("TX Phy Gain", (int *)&xband_set_data.TX.samp);
                    ImGui::InputInt("TX Adar Gain", (int *)&xband_set_data.TX.adar_gain);
                    if (ImGui::BeginMenu("TX Filter Selection"))
                    {
                        ImGui::RadioButton("m_6144.ftr", (int *)&xband_set_data.TX.ftr, 0);
                        ImGui::RadioButton("m_3072.ftr", (int *)&xband_set_data.TX.ftr, 1);
                        ImGui::RadioButton("m_1000.ftr", (int *)&xband_set_data.TX.ftr, 2);
                        ImGui::RadioButton("m_lte5.ftr", (int *)&xband_set_data.TX.ftr, 3);
                        ImGui::RadioButton("m_lte1.ftr", (int *)&xband_set_data.TX.ftr, 4);

                        ImGui::EndMenu();
                    }
                    ImGui::InputInt4("RX Phase [0]  [1]  [2]  [3]", (int *)&xband_set_data.RX.phase[0]);
                    ImGui::InputInt4("RX Phase [4]  [5]  [6]  [7]", (int *)&xband_set_data.RX.phase[4]);
                    ImGui::InputInt4("RX Phase [8]  [9]  [10] [11]", (int *)&xband_set_data.RX.phase[8]);
                    ImGui::InputInt4("RX Phase [12] [13] [14] [15]", (int *)&xband_set_data.RX.phase[12]);

                    ImGui::RadioButton("Set Receive", &XBAND_command, XBAND_SET_RX);
                    ImGui::InputFloat("RX LO", &xband_set_data.RX.LO);
                    ImGui::InputFloat("RX bw", &xband_set_data.RX.bw);
                    ImGui::InputInt("RX Samp", (int *)&xband_set_data.RX.samp);
                    ImGui::InputInt("RX Phy Gain", (int *)&xband_set_data.RX.samp);
                    ImGui::InputInt("RX Adar Gain", (int *)&xband_set_data.RX.adar_gain);
                    if (ImGui::BeginMenu("RX Filter Selection"))
                    {
                        ImGui::RadioButton("m_6144.ftr", (int *)&xband_set_data.RX.ftr, 0);
                        ImGui::RadioButton("m_3072.ftr", (int *)&xband_set_data.RX.ftr, 1);
                        ImGui::RadioButton("m_1000.ftr", (int *)&xband_set_data.RX.ftr, 2);
                        ImGui::RadioButton("m_lte5.ftr", (int *)&xband_set_data.RX.ftr, 3);
                        ImGui::RadioButton("m_lte1.ftr", (int *)&xband_set_data.RX.ftr, 4);

                        ImGui::EndMenu();
                    }
                    ImGui::InputInt4("RX Phase [0]  [1]  [2]  [3]", (int *)&xband_set_data.RX.phase[0]);
                    ImGui::InputInt4("RX Phase [4]  [5]  [6]  [7]", (int *)&xband_set_data.RX.phase[4]);
                    ImGui::InputInt4("RX Phase [8]  [9]  [10] [11]", (int *)&xband_set_data.RX.phase[8]);
                    ImGui::InputInt4("RX Phase [12] [13] [14] [15]", (int *)&xband_set_data.RX.phase[12]);

                    ImGui::RadioButton("Set MAX ON", &XBAND_command, XBAND_SET_MAX_ON);
                    ImGui::InputInt("Max On", (int *)&xband_rxtx_data.max_on);
                    ImGui::RadioButton("Set TMP SHDN", &XBAND_command, XBAND_SET_TMP_SHDN);
                    ImGui::InputInt("TMP SHDN", (int *)&xband_rxtx_data.tmp_shdn);
                    ImGui::RadioButton("Set TMP OP", &XBAND_command, XBAND_SET_TMP_OP);
                    ImGui::InputInt("TMP OP", (int *)&xband_rxtx_data.tmp_op);
                    ImGui::RadioButton("Set Loop Time", &XBAND_command, XBAND_SET_LOOP_TIME);
                    ImGui::InputInt("Loop Time", (int *)&xband_rxtx_data.loop_time);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::Separator();

                ImGui::Text("Actionable Commands");

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
                // Needs buttons. Values are just #defines and magic values

                // ImGui::Text("Retrieval Commands");

                // ImGui::Separator();

                // ImGui::Text("Set Commands");

                // if (authentication_access_level > 1)
                // {
                // }
                // else
                // {
                //     ImGui::Text("ACCESS DENIED");
                // }

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
