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
#include <pthread.h>

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
    /// ///////////////////////

    // Main loop prep.
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Used by the Authentication Control Panel
    // char password_buffer[64];
    // memset(password_buffer, 0x0, 64);

    // Main loop.
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resizing, etc.).
        glfwPollEvents();

        // Start the Dear ImGui frame.
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // // Debug window (default).
        // {
        //     ImGui::Text("%.1f FPS %.1f", ImGui::GetIO().Framerate, ImGui::GetTime());
        // }

        // Level 0: Basic access, can retrieve data from acs_upd.
        // Level 1: Team Member access, can execute Data-down commands.
        // Level 2: Priority access, can set some values.
        // Level 3: Project Manager access, can update flight software, edit critical systems.
        static bool AUTH_control_panel = true;
        // static uint8_t authentication_access_level = 0;

        // static bool auth_done = false;
        static pthread_t auth_thread_id;

        static auth_t auth = {0};

        if (AUTH_control_panel)
        {
            if (ImGui::Begin("Authentication Control Panel", &AUTH_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (auth.busy)
                {
                    ImGui::Text("PROCESSING...");
                    ImGui::InputTextWithHint("", "Enter Password", auth.password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
                }
                else
                {
                    switch (auth.access_level)
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

                    if (ImGui::InputTextWithHint("", "Enter Password", auth.password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        pthread_create(&auth_thread_id, NULL, gs_gui_check_password, &auth);
                    }
                }

                if (ImGui::Button("DEAUTHENTICATE"))
                {
                    auth.access_level = 0;
                }
            }
            ImGui::End();
        }

        static bool allow_transmission = false;
        static bool allow_receiving = true;

        static bool ACS_window = false;
        static int ACS_command = ACS_INVALID_ID;
        static cmd_input_t ACS_command_input = {.mod = INVALID_ID, .cmd = ACS_INVALID_ID, .unused = 0, .data_size = 0};
        static acs_set_data_t acs_set_data = {0};
        static acs_get_bool_t acs_get_bool = {0};

        static bool acs_rxtx_automated = false;
        static bool acs_rxtx_automated_thread_alive = false;
        static int acs_automated_rate = 100;
        static pthread_t acs_thread_id;
        static acs_upd_input_t acs_update_data = {0};

        if (ACS_window)
        {
            if (ImGui::Begin("ACS Operations", &ACS_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {

                    if (auth.access_level > 0)
                    {
                        // ImGui::Text("Data-down Commands");

                        if (ImGui::ArrowButton("get_moi_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get MOI");

                        if (ImGui::ArrowButton("get_imoi_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get IMOI");

                        if (ImGui::ArrowButton("get_dipole_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Dipole");

                        if (ImGui::ArrowButton("get_timestep_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Timestep");

                        if (ImGui::ArrowButton("get_measure_time_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Measure Time");

                        if (ImGui::ArrowButton("get_leeway_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Leeway");

                        if (ImGui::ArrowButton("get_wtarget_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get W-Target");

                        if (ImGui::ArrowButton("get_detumble_angle_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Detumble Angle");

                        if (ImGui::ArrowButton("get_sun_angle_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
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

                    if (auth.access_level > 1)
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
                        if (ImGui::InputInt("Timestep (ms)", (int *)&acs_set_data.tstep))
                        {
                            if (acs_set_data.tstep > 255)
                            {
                                acs_set_data.tstep = 255;
                            }
                            else if (acs_set_data.tstep < 0)
                            {
                                acs_set_data.tstep = 0;
                            }
                        }

                        ImGui::RadioButton("Set Measure Time", &ACS_command, ACS_SET_MEASURE_TIME); // 1x uint8_t
                        if (ImGui::InputInt("Measure Time (ms)", (int *)&acs_set_data.measure_time))
                        {
                            if (acs_set_data.measure_time > 255)
                            {
                                acs_set_data.measure_time = 255;
                            }
                            else if (acs_set_data.measure_time < 0)
                            {
                                acs_set_data.measure_time = 0;
                            }
                        }

                        ImGui::RadioButton("Set Leeway", &ACS_command, ACS_SET_LEEWAY); // 1x uint8_t
                        if (ImGui::InputInt("Leeway Factor", (int *)&acs_set_data.leeway))
                        {
                            if (acs_set_data.leeway > 255)
                            {
                                acs_set_data.leeway = 255;
                            }
                            else if (acs_set_data.leeway < 0)
                            {
                                acs_set_data.leeway = 0;
                            }
                        }

                        ImGui::RadioButton("Set W-Target", &ACS_command, ACS_SET_WTARGET); // 1x float
                        ImGui::InputFloat("W-Target", &acs_set_data.wtarget);
                        ImGui::RadioButton("Set Detumble Angle", &ACS_command, ACS_SET_DETUMBLE_ANG); // 1x uint8_t
                        if (ImGui::InputInt("Angle", (int *)&acs_set_data.detumble_angle))
                        {
                            if (acs_set_data.detumble_angle > 255)
                            {
                                acs_set_data.detumble_angle = 255;
                            }
                            else if (acs_set_data.detumble_angle < 0)
                            {
                                acs_set_data.detumble_angle = 0;
                            }
                        }
                        ImGui::RadioButton("Set Sun Angle", &ACS_command, ACS_SET_SUN_ANGLE); // 1x uint8_t
                        if (ImGui::InputInt("Sun Angle", (int *)&acs_set_data.sun_angle))
                        {
                            if (acs_set_data.sun_angle > 255)
                            {
                                acs_set_data.sun_angle = 255;
                            }
                            else if (acs_set_data.sun_angle < 0)
                            {
                                acs_set_data.sun_angle = 0;
                            }
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
                        // ImGui::InputInt("Rate (ms)", &acs_automated_rate);

                        ImGui::Text("(%s) Polling for Data-down every %d ms.", acs_rxtx_automated ? "ON" : "OFF", acs_automated_rate);

                        if (acs_rxtx_automated)
                        {
                            if (!acs_rxtx_automated_thread_alive)
                            {
                                acs_update_data.ready = false;
                                pthread_create(&acs_thread_id, NULL, gs_acs_update_data_handler, &acs_update_data);
                            }
                            else if (acs_update_data.ready)
                            {
                                printf("Getting and printing ACS update data...");
                                acs_update_data.ready = false;
                            }
                        }

                        // else if (auto_get_is_ready) <-- example
                        // TODO: Send data to TX device every so-many-milliseconds.

                        ImGui::Unindent();
                        ImGui::Text("Send Command [Data-up]");
                        ImGui::Indent();
                        if (auth.access_level > 1)
                        {
                            ImGui::Text("Queued Transmission");
                            ImGui::Text("Module ID:      0x%02x", ACS_command_input.mod);
                            ImGui::Text("Command ID:     0x%02x", ACS_command_input.cmd);
                            ImGui::Text("Unused:         0x%08x", ACS_command_input.unused);
                            ImGui::Text("Data size:      0x%08x", ACS_command_input.data_size);

                            if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
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
                                    ACS_command_input.data[0] = acs_set_data.dipole;
                                    ACS_command_input.data_size = sizeof(float);
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
                                    ACS_command_input.data[0] = acs_set_data.wtarget;
                                    ACS_command_input.data_size = sizeof(float);
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
                                    printf("ERROR!");
                                    ACS_command_input.data_size = -1;
                                    break;
                                }
                                }
                                ImGui::Unindent();

                                // TODO: Replace this with actually sending the communication to a TX device.

                                gs_transmit(&ACS_command_input);

                                // printf("Pretending to send the following data-up command to SPACE-HAUC:\n");
                                // printf("0x%02x 0x%02x 0x%08x 0x%08x", ACS_command_input.mod, ACS_command_input.cmd, ACS_command_input.unused, ACS_command_input.data_size);
                                // for (int i = 0; i < ACS_command_input.data_size; i++)
                                // {
                                //     printf(" 0x%02x", ACS_command_input.data[i]);
                                // }
                                // printf("\n");
                            }
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

        static bool EPS_window = false;
        static int EPS_command = EPS_INVALID_ID;
        static cmd_input_t EPS_command_input = {.mod = INVALID_ID, .cmd = EPS_INVALID_ID, .unused = 0, .data_size = 0};
        static eps_set_data_t eps_set_data = {0};
        static eps_get_bool_t eps_get_bool = {0};

        if (EPS_window)
        {
            if (ImGui::Begin("EPS Operations", &EPS_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                // ImGui::Text("Data-down Commands");
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {

                    if (auth.access_level > 0)
                    {
                        if (ImGui::ArrowButton("get_min_hk_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Minimal Housekeeping");

                        if (ImGui::ArrowButton("get_vbatt_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Battery Voltage");

                        if (ImGui::ArrowButton("get_sys_curr_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get System Current");

                        if (ImGui::ArrowButton("get_power_out_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Power Out");

                        if (ImGui::ArrowButton("get_solar_voltage_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Solar Voltage");

                        if (ImGui::ArrowButton("get_all_solar_voltage_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Solar Voltage (All)");

                        if (ImGui::ArrowButton("get_isun_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get ISUN");

                        if (ImGui::ArrowButton("get_loop_timer_button", ImGuiDir_Right))
                        {
                            printf("Pretending to poll SPACE-HAUC...\n");
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

                // ImGui::Text("Data-up Commands");
                if (ImGui::CollapsingHeader("Data-up Commands"))
                {
                    if (auth.access_level > 1)
                    {
                        ImGui::RadioButton("Set Loop Timer", &EPS_command, EPS_SET_LOOP_TIMER);
                        ImGui::InputInt("Loop Time (seconds)", (int *)&eps_set_data.loop_timer);
                    }
                    else
                    {
                        ImGui::Text("ACCESS DENIED");
                    }
                }

                EPS_command_input.mod = EPS_ID;
                EPS_command_input.cmd = EPS_command;

                ImGui::Separator();

                // ImGui::Text("Transmit");
                if (ImGui::CollapsingHeader("Transmit"))
                {
                    if (allow_transmission)
                    {
                        ImGui::Text("Send Command [Data-up]");
                        ImGui::Indent();
                        if (auth.access_level > 1)
                        {
                            ImGui::Text("Queued Transmission");
                            ImGui::Text("Module ID:     0x%02x", EPS_command_input.mod);
                            ImGui::Text("Command ID:    0x%02x", EPS_command_input.cmd);
                            ImGui::Text("Unused:        0x%08x", EPS_command_input.unused);
                            ImGui::Text("Data size:     0x%08x", EPS_command_input.data_size);

                            if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
                            {
                                switch (EPS_command_input.cmd)
                                {
                                case EPS_SET_LOOP_TIMER:
                                {
                                    EPS_command_input.data[0] = eps_set_data.loop_timer;
                                    EPS_command_input.data_size = sizeof(int);
                                    break;
                                }
                                default:
                                {
                                    printf("ERROR!");
                                    EPS_command_input.data_size = -1;
                                    break;
                                }
                                }
                                ImGui::Unindent();

                                gs_transmit(&EPS_command_input);

                                // printf("Pretending to send the following data-up command to SPACE-HAUC:\n");
                                // printf("0x%02x 0x%02x 0x%08x 0x%08x", EPS_command_input.mod, EPS_command_input.cmd, EPS_command_input.unused, EPS_command_input.data_size);
                                // for (int i = 0; i < EPS_command_input.data_size; i++)
                                // {
                                //     printf(" 0x%02x", EPS_command_input.data[i]);
                                // }
                                // printf("\n");
                            }
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

        static bool XBAND_window = false;
        static int XBAND_command = XBAND_INVALID_ID;
        static cmd_input_t XBAND_command_input = {.mod = INVALID_ID, .cmd = XBAND_INVALID_ID, .unused = 0, .data_size = 0};
        static xband_set_data_array_t xband_set_data = {0};
        static xband_tx_data_t xband_tx_data = {0};
        static xband_rxtx_data_t xband_rxtx_data = {0};
        static xband_get_bool_t xband_get_bool = {0};

        if (XBAND_window)
        {
            if (ImGui::Begin("X-Band Operations", &XBAND_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                // ImGui::Text("Data-down Commands");
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {
                    if (auth.access_level > 0)
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

                // ImGui::Text("Data-up Commands");
                if (ImGui::CollapsingHeader("Data-up Commands"))
                {
                    if (auth.access_level > 1)
                    {
                        ImGui::RadioButton("Set Transmit", &XBAND_command, XBAND_SET_TX);
                        ImGui::InputFloat("TX LO", &xband_set_data.TX.LO);
                        ImGui::InputFloat("TX bw", &xband_set_data.TX.bw);
                        ImGui::InputInt("TX Samp", (int *)&xband_set_data.TX.samp);
                        ImGui::InputInt("TX Phy Gain", (int *)&xband_set_data.TX.phy_gain);
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
                        ImGui::InputInt4("TX Phase [0]  [1]  [2]  [3]", (int *)&xband_set_data.TX.phase[0]);
                        ImGui::InputInt4("TX Phase [4]  [5]  [6]  [7]", (int *)&xband_set_data.TX.phase[4]);
                        ImGui::InputInt4("TX Phase [8]  [9]  [10] [11]", (int *)&xband_set_data.TX.phase[8]);
                        ImGui::InputInt4("TX Phase [12] [13] [14] [15]", (int *)&xband_set_data.TX.phase[12]);

                        ImGui::RadioButton("Set Receive", &XBAND_command, XBAND_SET_RX);
                        ImGui::InputFloat("RX LO", &xband_set_data.RX.LO);
                        ImGui::InputFloat("RX bw", &xband_set_data.RX.bw);
                        ImGui::InputInt("RX Samp", (int *)&xband_set_data.RX.samp);
                        ImGui::InputInt("RX Phy Gain", (int *)&xband_set_data.RX.phy_gain);
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
                }

                ImGui::Separator();

                // ImGui::Text("Actionable Commands");
                if (ImGui::CollapsingHeader("Executable Commands"))
                {
                    if (auth.access_level > 1)
                    {
                        // TODO: 
                        ImGui::RadioButton("Transmit", &XBAND_command, XBAND_DO_TX);
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
                        ImGui::Text("Send Command");
                        ImGui::Indent();
                        if (auth.access_level > 1)
                        {
                            ImGui::Text("Queued Transmission");
                            ImGui::Text("Module ID: 0x%02x", XBAND_command_input.mod);
                            ImGui::Text("Command ID: 0x%02x", XBAND_command_input.cmd);
                            ImGui::Text("Unused: 0x%02x", XBAND_command_input.unused);
                            ImGui::Text("Data size: 0x%02x", XBAND_command_input.data_size);

                            if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
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
break;
                                }
                                case XBAND_DO_RX:
                                {
                                    break;
                                }
                                case XBAND_DISABLE:
                                {
                                    break;
                                }
                                case XBAND_SET_MAX_ON:
                                {
                                    break;
                                }
                                case XBAND_SET_TMP_SHDN:
                                {
                                    break;
                                }
                                case XBAND_SET_TMP_OP:
                                {
                                    break;
                                }
                                case XBAND_SET_LOOP_TIME:
                                {
                                    break;
                                }
                                }
                            }
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

        static bool SW_UPD_window = false;
        static int UPD_command = INVALID_ID;
        static cmd_input_t UPD_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
        static char upd_filename_buffer[256] = {0};

        // Handles software updates.
        if (SW_UPD_window)
        {
            if (ImGui::Begin("Software Updater Control Panel", &SW_UPD_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                // Needs buttons. Values are just #defines and magic values

                ImGui::Text("Name of the file to send:");
                ImGui::InputTextWithHint("", "Name of File", upd_filename_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue);

                ImGui::Text("Queued file name: %s", upd_filename_buffer);

                ImGui::Separator();

                if (auth.access_level > 1)
                {
                    ImGui::Button("BEGIN UPDATE");
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                // Maybe some information on the current status of an update can go here.
            }
            ImGui::End();
        }

        static bool SYS_CTRL_window = false;
        // static int SYS_command = INVALID_ID;
        static cmd_input_t SYS_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};

        // Handles
        // SYS_VER_MAGIC = 0xd,
        // SYS_RESTART_PROG = 0xff,
        // SYS_REBOOT = 0xfe,
        // SYS_CLEAN_SHBYTES = 0xfd
        if (SYS_CTRL_window)
        {
            if (ImGui::Begin("System Control Panel", &SYS_CTRL_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (auth.access_level > 2)
                {
                    ImGui::RadioButton("Version Magic", &SYS_command_input.mod, SYS_VER_MAGIC);
                    ImGui::RadioButton("Restart Program", &SYS_command_input.mod, SYS_RESTART_PROG);
                    ImGui::RadioButton("Reboot", &SYS_command_input.mod, SYS_REBOOT);
                    ImGui::RadioButton("Clean SHBYTES", &SYS_command_input.mod, SYS_CLEAN_SHBYTES);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }

                ImGui::Separator();

                if (auth.access_level > 2)
                {
                    if (allow_transmission)
                    {
                        ImGui::Text("Executable Commands");
                        ImGui::Indent();
                        if (auth.access_level > 2)
                        {
                            ImGui::Text("Queued Transmission");
                            ImGui::Text("Module ID:     0x%02x", SYS_command_input.mod);
                            ImGui::Text("Command ID:    0x%02x", SYS_command_input.cmd);
                            ImGui::Text("Unused:        0x%08x", SYS_command_input.unused);
                            ImGui::Text("Data size:     0x%08x", SYS_command_input.data_size);

                            if (ImGui::Button("SEND COMMAND"))
                            {
                                switch (SYS_command_input.mod)
                                {
                                case SYS_VER_MAGIC:
                                {
                                    SYS_command_input.cmd = 0x0; // Not really necessary.
                                    break;
                                }
                                case SYS_RESTART_PROG:
                                {
                                    SYS_command_input.cmd = SYS_RESTART_FUNC_MAGIC;
                                    SYS_command_input.data[0] = SYS_RESTART_FUNC_VAL;
                                    break;
                                }
                                case SYS_REBOOT:
                                {
                                    SYS_command_input.cmd = SYS_REBOOT_FUNC_MAGIC;
                                    SYS_command_input.data[0] = SYS_REBOOT_FUNC_VAL;
                                    break;
                                }
                                case SYS_CLEAN_SHBYTES:
                                {
                                    SYS_command_input.cmd = SYS_CLEAN_SHBYTES;
                                    break;
                                }
                                default:
                                {
                                    printf("ERROR!");
                                    SYS_command_input.data_size = -1;
                                    break;
                                }
                                }
                                ImGui::Unindent();

                                gs_transmit(&SYS_command_input);

                                // printf("Pretending to send the following data-up command to SPACE-HAUC:\n");
                                // printf("0x%02x 0x%02x 0x%08x 0x%08x", SYS_command_input.mod, SYS_command_input.cmd, SYS_command_input.unused, SYS_command_input.data_size);
                                // for (int i = 0; i < SYS_command_input.data_size; i++)
                                // {
                                //     printf(" 0x%02x", SYS_command_input.data[i]);
                                // }
                                // printf("\n");
                            }
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
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            ImGui::End();
        }

        static bool COMMS_control_panel = true;

        if (COMMS_control_panel)
        {
            if (ImGui::Begin("Communications Control Panel", &COMMS_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar /*| ImGuiWindowFlags_MenuBar*/))
            {
                ImGui::Checkbox("Attitude Control System", &ACS_window); // Contains ACS and ACS_UPD
                ImGui::Checkbox("Electrical Power Supply", &EPS_window);
                ImGui::Checkbox("X-Band", &XBAND_window);
                ImGui::Checkbox("Software Updater", &SW_UPD_window);
                ImGui::Checkbox("System Control", &SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

                ImGui::Separator();

                if (auth.access_level >= 0)
                {
                    ImGui::Checkbox("Unlock Transmissions", &allow_transmission);
                }
                else
                {
                    ImGui::Text("ACCESS DENIED");
                }
            }
            ImGui::End();
        }

        static bool User_Manual = false;

        if (User_Manual)
        {
            if (ImGui::Begin("User Manual", &User_Manual, ImGuiWindowFlags_AlwaysVerticalScrollbar /* | ImGuiWindowFlags_HorizontalScrollbar*/))
            {
                ImGui::BeginTabBar("Help Menu Tab Bar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
                if (ImGui::BeginTabItem("Authentication"))
                {
                    ImGui::TextWrapped("Window");
                    ImGui::Indent();

                    ImGui::TextWrapped("The Authentication Control Panel window is visible by default. Its visibility can be toggled by pressing the Authentication button in the Main Toolbar at the top of the screen.");

                    ImGui::TextWrapped("This window displays the current access level, a password-entry field, and a <DEAUTHENTICATE> button.");

                    ImGui::TextWrapped("Elements of the Ground Station graphical program which are blocked due to insufficient access level will display the following text: \"ACCESS DENIED\".");

                    ImGui::TextWrapped("To enter your password, click on the text entry field and press [ENTER] when finished. The system will process your input and update your access level accordingly. Please note that incorrect inputs are punished with a 2.5 second delay.");

                    ImGui::TextWrapped("To revoke authentication, press the <DEAUTHENTICATE> button. This will not reset any previously set values within the program, but will simply deny further access.");

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

                if (ImGui::BeginTabItem("Communications"))
                {
                    ImGui::TextWrapped("Command Types");
                    ImGui::Indent();

                    ImGui::TextWrapped("Data-down");
                    ImGui::Indent();
                    ImGui::TextWrapped("Commands where data will be sent down from SPACE-HAUC to the Ground Station. Typically, these are commands that poll one or more values.");
                    ImGui::Unindent();

                    ImGui::TextWrapped("Data-down");
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

                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::Button("Authentication"))
            {
                AUTH_control_panel = !AUTH_control_panel;
            }
            if (ImGui::Button("Communications"))
            {
                COMMS_control_panel = !COMMS_control_panel;
            }
            if (ImGui::Button("User Manual"))
            {
                User_Manual = !User_Manual;
            }

            // ImGui::Text("Uptime: %d \t\t Framerate: %f", ImGui::GetTime(), ImGui::GetIO().Framerate);
        }
        ImGui::EndMainMenuBar();

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
