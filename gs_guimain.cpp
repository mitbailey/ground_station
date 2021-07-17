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

// TODO: Add receive functionality where necessary.

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

    GLFWwindow *window = glfwCreateWindow(1280, 720, "SPACE-HAUC Ground Station: Graphical Interface Client", NULL, NULL);

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
    ACSDisplayData* acs_display_data = new ACSDisplayData();

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
        static acs_set_data_holder_t acs_set_data_holder = {0};
        static acs_get_bool_t acs_get_bool = {0};

        static bool acs_rxtx_automated = false;
        static bool acs_rxtx_automated_thread_alive = false;
        static int acs_automated_rate = ACS_UPD_DATARATE;
        static pthread_t acs_thread_id;
        static acs_upd_input_t acs_update_data = {0};

        if (ACS_window)
        {
            if (ImGui::Begin("ACS Operations", &ACS_window))
            {
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {

                    if (auth.access_level > 0)
                    {
                        // ImGui::Text("Data-down Commands");

                        if (ImGui::ArrowButton("get_moi_button", ImGuiDir_Right))
                        {
                            //printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_MOI;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get MOI");

                        if (ImGui::ArrowButton("get_imoi_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_IMOI;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get IMOI");

                        if (ImGui::ArrowButton("get_dipole_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_DIPOLE;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Dipole");

                        if (ImGui::ArrowButton("get_timestep_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_TSTEP;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Timestep");

                        if (ImGui::ArrowButton("get_measure_time_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_MEASURE_TIME;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Measure Time");

                        if (ImGui::ArrowButton("get_leeway_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_LEEWAY;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Leeway");

                        if (ImGui::ArrowButton("get_wtarget_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_WTARGET;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get W-Target");

                        if (ImGui::ArrowButton("get_detumble_angle_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_DETUMBLE_ANG;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Detumble Angle");

                        if (ImGui::ArrowButton("get_sun_angle_button", ImGuiDir_Right))
                        {
                            // printf("Pretending to poll SPACE-HAUC...\n");
                            ACS_command_input.mod = ACS_ID;
                            ACS_command_input.cmd = ACS_GET_SUN_ANGLE;
                            ACS_command_input.unused = 0x0;
                            ACS_command_input.data_size = 0x0;
                            memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&ACS_command_input);
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
                        if (ImGui::InputInt("Timestep (ms)", &acs_set_data_holder.tstep))
                        {
                            if (acs_set_data_holder.tstep > 255)
                            {
                                acs_set_data_holder.tstep = 255;
                            }
                            else if (acs_set_data_holder.tstep < 0)
                            {
                                acs_set_data_holder.tstep = 0;
                            }
                            acs_set_data.tstep = (uint8_t)acs_set_data_holder.tstep;
                        }

                        ImGui::RadioButton("Set Measure Time", &ACS_command, ACS_SET_MEASURE_TIME); // 1x uint8_t
                        if (ImGui::InputInt("Measure Time (ms)", &acs_set_data_holder.measure_time))
                        {
                            if (acs_set_data_holder.measure_time > 255)
                            {
                                acs_set_data_holder.measure_time = 255;
                            }
                            else if (acs_set_data_holder.measure_time < 0)
                            {
                                acs_set_data_holder.measure_time = 0;
                            }
                            acs_set_data.measure_time = (uint8_t)acs_set_data_holder.measure_time;
                        }

                        ImGui::RadioButton("Set Leeway", &ACS_command, ACS_SET_LEEWAY); // 1x uint8_t
                        if (ImGui::InputInt("Leeway Factor", &acs_set_data_holder.leeway))
                        {
                            if (acs_set_data_holder.leeway > 255)
                            {
                                acs_set_data_holder.leeway = 255;
                            }
                            else if (acs_set_data_holder.leeway < 0)
                            {
                                acs_set_data_holder.leeway = 0;
                            }
                            acs_set_data.leeway = (uint8_t)acs_set_data_holder.leeway;
                        }

                        ImGui::RadioButton("Set W-Target", &ACS_command, ACS_SET_WTARGET); // 1x float
                        ImGui::InputFloat("W-Target", &acs_set_data.wtarget);
                        ImGui::RadioButton("Set Detumble Angle", &ACS_command, ACS_SET_DETUMBLE_ANG); // 1x uint8_t
                        if (ImGui::InputInt("Angle", &acs_set_data_holder.detumble_angle))
                        {
                            if (acs_set_data_holder.detumble_angle > 255)
                            {
                                acs_set_data_holder.detumble_angle = 255;
                            }
                            else if (acs_set_data_holder.detumble_angle < 0)
                            {
                                acs_set_data_holder.detumble_angle = 0;
                            }
                            acs_set_data.detumble_angle = (uint8_t)acs_set_data_holder.detumble_angle;
                        }
                        ImGui::RadioButton("Set Sun Angle", &ACS_command, ACS_SET_SUN_ANGLE); // 1x uint8_t
                        if (ImGui::InputInt("Sun Angle", &acs_set_data_holder.sun_angle))
                        {
                            if (acs_set_data_holder.sun_angle > 255)
                            {
                                acs_set_data_holder.sun_angle = 255;
                            }
                            else if (acs_set_data_holder.sun_angle < 0)
                            {
                                acs_set_data_holder.sun_angle = 0;
                            }
                            acs_set_data.sun_angle = (uint8_t)acs_set_data_holder.sun_angle;
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

                        ImGui::Unindent();
                        ImGui::Text("Send Command [Data-up]");
                        ImGui::Indent();
                        if (auth.access_level > 1)
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
                                // ACS_command_input.data[0] = acs_set_data.dipole;
                                ACS_command_input.data_size = sizeof(float);
                                memcpy(ACS_command_input.data, &acs_set_data.dipole, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_TSTEP:
                            {
                                ACS_command_input.data[0] = (uint8_t)acs_set_data.tstep;
                                ACS_command_input.data_size = sizeof(uint8_t);
                                // memcpy(&ACS_command_input.data[0], &acs_set_data.tstep, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_MEASURE_TIME:
                            {
                                ACS_command_input.data[0] = (uint8_t)acs_set_data.measure_time;
                                ACS_command_input.data_size = sizeof(uint8_t);
                                // memcpy(&ACS_command_input.data[0], &acs_set_data.measure_time, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_LEEWAY:
                            {
                                ACS_command_input.data[0] = (uint8_t)acs_set_data.leeway;
                                ACS_command_input.data_size = sizeof(uint8_t);
                                // memcpy(&ACS_command_input.data[0], &acs_set_data.leeway, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_WTARGET:
                            {
                                // ACS_command_input.data[0] = acs_set_data.wtarget;
                                ACS_command_input.data_size = sizeof(float);
                                memcpy(ACS_command_input.data, &acs_set_data.wtarget, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_DETUMBLE_ANG:
                            {
                                ACS_command_input.data[0] = (uint8_t)acs_set_data.detumble_angle;
                                ACS_command_input.data_size = sizeof(uint8_t);
                                // memcpy(&ACS_command_input.data[0], &acs_set_data.detumble_angle, ACS_command_input.data_size);
                                break;
                            }
                            case ACS_SET_SUN_ANGLE:
                            {
                                ACS_command_input.data[0] = (uint8_t)acs_set_data.sun_angle;
                                ACS_command_input.data_size = sizeof(uint8_t);
                                // memcpy(&ACS_command_input.data[0], &acs_set_data.sun_angle, ACS_command_input.data_size);
                                break;
                            }
                            default:
                            {
                                // printf("ACS ID ERROR!\n");
                                ACS_command_input.data_size = -1;
                                break;
                            }
                            }
                            ImGui::Unindent();

                            gs_gui_transmissions_handler(&auth, &ACS_command_input);
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
            if (ImGui::Begin("EPS Operations", &EPS_window))
            {
                // ImGui::Text("Data-down Commands");
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {

                    if (auth.access_level > 0)
                    {
                        if (ImGui::ArrowButton("get_min_hk_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_MIN_HK;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Minimal Housekeeping");

                        if (ImGui::ArrowButton("get_vbatt_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_VBATT;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Battery Voltage");

                        if (ImGui::ArrowButton("get_sys_curr_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_SYS_CURR;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get System Current");

                        if (ImGui::ArrowButton("get_power_out_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_OUTPOWER;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Power Out");

                        if (ImGui::ArrowButton("get_solar_voltage_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_VSUN;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Solar Voltage");

                        if (ImGui::ArrowButton("get_all_solar_voltage_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_VSUN_ALL;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Solar Voltage (All)");

                        if (ImGui::ArrowButton("get_isun_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_ISUN;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get ISUN");

                        if (ImGui::ArrowButton("get_loop_timer_button", ImGuiDir_Right))
                        {
                            EPS_command_input.mod = EPS_ID;
                            EPS_command_input.cmd = EPS_GET_LOOP_TIMER;
                            EPS_command_input.unused = 0x0;
                            EPS_command_input.data_size = 0x0;
                            memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&EPS_command_input);
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
                        ImGui::InputInt("Loop Time (seconds)", &eps_set_data.loop_timer);
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
                        ImGui::Indent();
                        if (auth.access_level > 1)
                        {
                            switch (EPS_command_input.cmd)
                            {
                            case EPS_SET_LOOP_TIMER:
                            {
                                // TODO: Remove all .data[0] = some_variable, because this will only set the first byte of .data because its in section of bytes.
                                // TODO: ~DO NOT REMOVE~ UNLESS it is a one-byte kind of data, ie uint8_t.
                                // EPS_command_input.data[0] = eps_set_data.loop_timer;
                                EPS_command_input.data_size = sizeof(int);
                                memcpy(EPS_command_input.data, &eps_set_data.loop_timer, EPS_command_input.data_size);
                                break;
                            }
                            default:
                            {
                                // printf("ERROR!");
                                EPS_command_input.data_size = -1;
                                break;
                            }
                            }
                            ImGui::Unindent();

                            gs_gui_transmissions_handler(&auth, &EPS_command_input);
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
        static xband_tx_data_holder_t xband_tx_data_holder = {0};
        static xband_rxtx_data_t xband_rxtx_data = {0};
        static xband_rxtx_data_holder_t xband_rxtx_data_holder = {0};
        static xband_get_bool_t xband_get_bool = {0};

        if (XBAND_window)
        {
            if (ImGui::Begin("X-Band Operations", &XBAND_window))
            {
                // ImGui::Text("Data-down Commands");
                // TODO: Change to arrow-button implementation.
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
                        ImGui::Indent();

                        // if (ImGui::CollapsingHeader("Set Transmit"))
                        // {
                        ImGui::RadioButton("Set Transmit", &XBAND_command, XBAND_SET_TX);
                        ImGui::InputFloat("TX LO", &xband_set_data.TX.LO);
                        ImGui::InputFloat("TX bw", &xband_set_data.TX.bw);
                        ImGui::InputInt("TX Samp", &xband_set_data.TXH.samp);
                        if (xband_set_data.TXH.samp > 0xFFFF)
                        {
                            xband_set_data.TXH.samp = 0xFFFF;
                        }
                        else if (xband_set_data.TXH.samp < 0)
                        {
                            xband_set_data.TXH.samp = 0;
                        }
                        xband_set_data.TX.samp = (uint16_t)xband_set_data.TXH.samp;

                        ImGui::InputInt("TX Phy Gain", &xband_set_data.TXH.phy_gain);
                        if (xband_set_data.TXH.phy_gain > 0xFF)
                        {
                            xband_set_data.TXH.phy_gain = 0xFF;
                        }
                        else if (xband_set_data.TXH.phy_gain < 0)
                        {
                            xband_set_data.TXH.phy_gain = 0;
                        }
                        xband_set_data.TX.phy_gain = (uint8_t)xband_set_data.TXH.phy_gain;

                        ImGui::InputInt("TX Adar Gain", &xband_set_data.TXH.adar_gain);
                        if (xband_set_data.TXH.adar_gain > 0xFF)
                        {
                            xband_set_data.TXH.adar_gain = 0xFF;
                        }
                        else if (xband_set_data.TXH.adar_gain < 0)
                        {
                            xband_set_data.TXH.adar_gain = 0;
                        }
                        xband_set_data.TX.adar_gain = (uint8_t)xband_set_data.TXH.adar_gain;

                        if (ImGui::BeginMenu("TX Filter Selection"))
                        {
                            ImGui::RadioButton("m_6144.ftr", &xband_set_data.TXH.ftr, 0);
                            ImGui::RadioButton("m_3072.ftr", &xband_set_data.TXH.ftr, 1);
                            ImGui::RadioButton("m_1000.ftr", &xband_set_data.TXH.ftr, 2);
                            ImGui::RadioButton("m_lte5.ftr", &xband_set_data.TXH.ftr, 3);
                            ImGui::RadioButton("m_lte1.ftr", &xband_set_data.TXH.ftr, 4);

                            xband_set_data.TX.ftr = (uint8_t)xband_set_data.TXH.ftr;

                            ImGui::EndMenu();
                        }
                        ImGui::InputInt4("TX Phase [0]  [1]  [2]  [3]", &xband_set_data.TXH.phase[0]);
                        ImGui::InputInt4("TX Phase [4]  [5]  [6]  [7]", &xband_set_data.TXH.phase[4]);
                        ImGui::InputInt4("TX Phase [8]  [9]  [10] [11]", &xband_set_data.TXH.phase[8]);
                        ImGui::InputInt4("TX Phase [12] [13] [14] [15]", &xband_set_data.TXH.phase[12]);
                        for (int i = 0; i < 16; i++)
                        {
                            if (xband_set_data.TXH.phase[i] > 32767)
                            {
                                xband_set_data.TXH.phase[i] = 32767;
                            }
                            else if (xband_set_data.TXH.phase[i] < -32768)
                            {
                                xband_set_data.TXH.phase[i] = -32768;
                            }
                            xband_set_data.TX.phase[i] = (short)xband_set_data.TXH.phase[i];
                        }
                        // }
                        ImGui::Separator();

                        // if (ImGui::CollapsingHeader("Set Receive"))
                        // {
                        ImGui::RadioButton("Set Receive", &XBAND_command, XBAND_SET_RX);
                        ImGui::InputFloat("RX LO", &xband_set_data.RX.LO);
                        ImGui::InputFloat("RX bw", &xband_set_data.RX.bw);
                        ImGui::InputInt("RX Samp", &xband_set_data.RXH.samp);
                        if (xband_set_data.RXH.samp > 0xFFFF)
                        {
                            xband_set_data.RXH.samp = 0xFFFF;
                        }
                        else if (xband_set_data.RXH.samp < 0)
                        {
                            xband_set_data.RXH.samp = 0;
                        }
                        xband_set_data.RX.samp = (uint16_t)xband_set_data.RXH.samp;

                        ImGui::InputInt("RX Phy Gain", &xband_set_data.RXH.phy_gain);
                        if (xband_set_data.RXH.phy_gain > 0xFF)
                        {
                            xband_set_data.RXH.phy_gain = 0xFF;
                        }
                        else if (xband_set_data.RXH.phy_gain < 0)
                        {
                            xband_set_data.RXH.phy_gain = 0;
                        }
                        xband_set_data.RX.phy_gain = (uint8_t)xband_set_data.RXH.phy_gain;

                        ImGui::InputInt("RX Adar Gain", &xband_set_data.RXH.adar_gain);
                        if (xband_set_data.RXH.adar_gain > 0xFF)
                        {
                            xband_set_data.RXH.adar_gain = 0xFF;
                        }
                        else if (xband_set_data.RXH.adar_gain < 0)
                        {
                            xband_set_data.RXH.adar_gain = 0;
                        }
                        xband_set_data.RX.adar_gain = (uint8_t)xband_set_data.RXH.adar_gain;

                        if (ImGui::BeginMenu("RX Filter Selection"))
                        {
                            ImGui::RadioButton("m_6144.ftr", &xband_set_data.RXH.ftr, 0);
                            ImGui::RadioButton("m_3072.ftr", &xband_set_data.RXH.ftr, 1);
                            ImGui::RadioButton("m_1000.ftr", &xband_set_data.RXH.ftr, 2);
                            ImGui::RadioButton("m_lte5.ftr", &xband_set_data.RXH.ftr, 3);
                            ImGui::RadioButton("m_lte1.ftr", &xband_set_data.RXH.ftr, 4);

                            xband_set_data.RX.ftr = (uint8_t)xband_set_data.RXH.ftr;

                            ImGui::EndMenu();
                        }
                        ImGui::InputInt4("RX Phase [0]  [1]  [2]  [3]", &xband_set_data.RXH.phase[0]);
                        ImGui::InputInt4("RX Phase [4]  [5]  [6]  [7]", &xband_set_data.RXH.phase[4]);
                        ImGui::InputInt4("RX Phase [8]  [9]  [10] [11]", &xband_set_data.RXH.phase[8]);
                        ImGui::InputInt4("RX Phase [12] [13] [14] [15]", &xband_set_data.RXH.phase[12]);
                        for (int i = 0; i < 16; i++)
                        {
                            if (xband_set_data.RXH.phase[i] > 32767)
                            {
                                xband_set_data.RXH.phase[i] = 32767;
                            }
                            else if (xband_set_data.RXH.phase[i] < -32768)
                            {
                                xband_set_data.RXH.phase[i] = -32768;
                            }
                            xband_set_data.RX.phase[i] = (short)xband_set_data.RXH.phase[i];
                        }
                        // }
                        ImGui::Separator();

                        ImGui::RadioButton("Set MAX ON", &XBAND_command, XBAND_SET_MAX_ON);
                        ImGui::InputInt("Max On", &xband_rxtx_data_holder.max_on);
                        if (xband_rxtx_data_holder.max_on > 0xFF)
                        {
                            xband_rxtx_data_holder.max_on = 0xFF;
                        }
                        else if (xband_rxtx_data_holder.max_on < 0)
                        {
                            xband_rxtx_data_holder.max_on = 0;
                        }
                        xband_rxtx_data.max_on = (uint8_t)xband_rxtx_data_holder.max_on;

                        ImGui::RadioButton("Set TMP SHDN", &XBAND_command, XBAND_SET_TMP_SHDN);
                        ImGui::InputInt("TMP SHDN", &xband_rxtx_data_holder.tmp_shdn);
                        if (xband_rxtx_data_holder.tmp_shdn > 0xFF)
                        {
                            xband_rxtx_data_holder.tmp_shdn = 0xFF;
                        }
                        else if (xband_rxtx_data_holder.tmp_shdn < 0)
                        {
                            xband_rxtx_data_holder.tmp_shdn = 0;
                        }
                        xband_rxtx_data.tmp_shdn = (uint8_t)xband_rxtx_data_holder.tmp_shdn;

                        ImGui::RadioButton("Set TMP OP", &XBAND_command, XBAND_SET_TMP_OP);
                        ImGui::InputInt("TMP OP", &xband_rxtx_data_holder.tmp_op);
                        if (xband_rxtx_data_holder.tmp_op > 0xFF)
                        {
                            xband_rxtx_data_holder.tmp_op = 0xFF;
                        }
                        else if (xband_rxtx_data_holder.tmp_op < 0)
                        {
                            xband_rxtx_data_holder.tmp_op = 0;
                        }
                        xband_rxtx_data.tmp_op = (uint8_t)xband_rxtx_data_holder.tmp_op;

                        ImGui::RadioButton("Set Loop Time", &XBAND_command, XBAND_SET_LOOP_TIME);
                        ImGui::InputInt("Loop Time", &xband_rxtx_data_holder.loop_time);
                        if (xband_rxtx_data_holder.loop_time > 0xFF)
                        {
                            xband_rxtx_data_holder.loop_time = 0xFF;
                        }
                        else if (xband_rxtx_data_holder.loop_time < 0)
                        {
                            xband_rxtx_data_holder.loop_time = 0;
                        }
                        xband_rxtx_data.loop_time = (uint8_t)xband_rxtx_data_holder.loop_time;

                        ImGui::Unindent();
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
                        ImGui::InputInt("TX type", &xband_tx_data_holder.type);
                        if (xband_tx_data_holder.type > 255)
                        {
                            xband_tx_data_holder.type = 255;
                        }
                        else if (xband_tx_data_holder.type < 0)
                        {
                            xband_tx_data_holder.type = 0;
                        }
                        xband_tx_data.type = (uint8_t)xband_tx_data_holder.type;

                        ImGui::InputInt("TX f_id", &xband_tx_data.f_id);
                        ImGui::InputInt("TX mtu", &xband_tx_data.mtu);
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
                        ImGui::Indent();
                        if (auth.access_level > 1)
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
                                memcpy(XBAND_command_input.data, &xband_tx_data, sizeof(xband_tx_data_t));
                                XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                                break;
                            }
                            case XBAND_DO_RX:
                            {
                                // Not yet implemented on SPACE-HAUC.
                                printf("This functionality is not implemented as it does not yet exist on SPACE-HAUC.\n");
                                break;
                            }
                            case XBAND_DISABLE:
                            {
                                // Not yet implemented on SPACE-HAUC.
                                printf("This functionality is not implemented as it does not yet exist on SPACE-HAUC.\n");
                                break;
                            }
                            case XBAND_SET_MAX_ON:
                            {
                                // cmd_parser expects an int8_t for data.
                                XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.max_on;
                                XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                                break;
                            }
                            case XBAND_SET_TMP_SHDN:
                            {
                                // cmd_parser expects an int8_t for data.
                                XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_shdn;
                                XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                                break;
                            }
                            case XBAND_SET_TMP_OP:
                            {
                                // cmd_parser expects an int8_t for data.
                                XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_op;
                                XBAND_command_input.data_size = sizeof(xband_tx_data_t);
                                break;
                            }
                            case XBAND_SET_LOOP_TIME:
                            {
                                // cmd_parser expects an int8_t for data.
                                // XBAND_command_input is a structure that is transmitted.
                                // .data contains the data expected by SH's cmd_parser
                                // xband_rxtx_data is just a convenient structure for collecting user input
                                XBAND_command_input.data[0] = (uint8_t)xband_rxtx_data.tmp_op;
                                break;
                            }
                            default:
                            {
                                XBAND_command_input.data_size = -1;
                            }
                            }

                            gs_gui_transmissions_handler(&auth, &XBAND_command_input);
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

                // NOTE: Queued file name not sent with software update command, probably doesn't need to be here at all. This is something the Ground Station's version of SW_UPDATE will have to handle.
                ImGui::Text("Queued file name: %s", upd_filename_buffer);

                ImGui::Separator();

                if (auth.access_level > 1)
                {
                    if (ImGui::Button("BEGIN UPDATE"))
                    {
                        // Sets values for the software update command structure.
                        UPD_command_input.mod = SW_UPD_ID;
                        UPD_command_input.cmd = SW_UPD_FUNC_MAGIC;
                        // UPD_command_input.data[0] = SW_UPD_VALID_MAGIC;
                        long sw_upd_valid_magic_temp = SW_UPD_VALID_MAGIC;
                        memcpy(UPD_command_input.data, &sw_upd_valid_magic_temp, sizeof(long));

                        // Transmits the software update command.
                        gs_transmit(&UPD_command_input);
                    }
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
        static cmd_input_holder_t SYS_command_input_holder = {0};

        // Handles
        // SYS_VER_MAGIC = 0xd,
        // SYS_RESTART_PROG = 0xff,
        // SYS_REBOOT = 0xfe,
        // SYS_CLEAN_SHBYTES = 0xfd
        if (SYS_CTRL_window)
        {
            if (ImGui::Begin("System Control Panel", &SYS_CTRL_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (ImGui::CollapsingHeader("Data-down Commands"))
                {
                    if (auth.access_level > 0)
                    {
                        if (ImGui::ArrowButton("get_version_magic", ImGuiDir_Right))
                        {
                            SYS_command_input.mod = SYS_VER_MAGIC;
                            SYS_command_input.cmd = 0x0;
                            SYS_command_input.unused = 0x0;
                            SYS_command_input.data_size = 0x0;
                            memset(SYS_command_input.data, 0x0, MAX_DATA_SIZE);
                            gs_transmit(&SYS_command_input);
                        }
                        ImGui::SameLine();
                        ImGui::Text("Get Version Magic");
                    }
                }

                ImGui::Separator();

                if (ImGui::CollapsingHeader("Data-up Commands"))
                {
                    if (auth.access_level > 2)
                    {
                        // NOTE: Since IMGUI only accepts full integers, we have to use a temporary full integer structure to hold them before converting to uint8_t, which is what SPACE-HAUC requires.
                        ImGui::RadioButton("Restart Program", &SYS_command_input_holder.mod, SYS_RESTART_PROG);
                        ImGui::RadioButton("Reboot Flight", &SYS_command_input_holder.mod, SYS_REBOOT);
                        ImGui::RadioButton("Clean SHBYTES", &SYS_command_input_holder.mod, SYS_CLEAN_SHBYTES);

                        SYS_command_input.mod = (uint8_t)SYS_command_input_holder.mod;
                    }
                    else
                    {
                        ImGui::Text("ACCESS DENIED");
                    }
                }

                ImGui::Separator();

                if (ImGui::CollapsingHeader("Transmit"))
                {
                    if (allow_transmission)
                    {
                        ImGui::Indent();
                        if (auth.access_level > 2)
                        {
                            switch (SYS_command_input.mod)
                            {
                            case SYS_RESTART_PROG:
                            {
                                SYS_command_input.data_size = 0;
                                break;
                            }
                            case SYS_REBOOT:
                            {
                                SYS_command_input.data_size = sizeof(uint64_t);
                                SYS_command_input.cmd = SYS_REBOOT_FUNC_MAGIC;
                                long temp = SYS_REBOOT_FUNC_VAL;
                                memcpy(SYS_command_input.data, &temp, SYS_command_input.data_size);

                                SYS_command_input.unused = 0x0;

                                break;
                            }
                            case SYS_CLEAN_SHBYTES:
                            {
                                SYS_command_input.data_size = 0;
                                SYS_command_input.cmd = SYS_CLEAN_SHBYTES;

                                SYS_command_input.unused = 0x0;
                                memset(SYS_command_input.data, 0x0, MAX_DATA_SIZE);
                                break;
                            }
                            default:
                            {
                                // printf("SYS ERROR!\n");
                                SYS_command_input.data_size = -1;
                                break;
                            }
                            }
                            ImGui::Unindent();

                            gs_gui_transmissions_handler(&auth, &SYS_command_input);
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

        static bool RX_display = false;

        if (RX_display)
        {
            if (ImGui::Begin("Plaintext RX Display"), &RX_display)
            {

            }
            ImGui::End();
        }

        static bool ACS_UPD_display = false;

        if (ACS_UPD_display)
        {
            if (ImGui::Begin("ACS Update Display"), &ACS_UPD_display)
            {
                // TODO: Implement some method of displaying the ACS update data. Probably a good idea to make a locally-global class with data that this window displays. The data is set by gs_receive.
                // NOTE: This window must be opened independent of ACS's automated data retrieval option.

                if (acs_display_data->status())
                {
                    // TODO: Display the data.
                }
            }   
            ImGui::End();
        }

        static bool DISP_control_panel = true;

        if (DISP_control_panel)
        {
            if (ImGui::Begin("Displays Control Panel", &DISP_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar /*| ImGuiWindowFlags_MenuBar*/))
            {
                ImGui::Checkbox("Attitude Control System", &ACS_window); // Contains ACS and ACS_UPD
                ImGui::Checkbox("Electrical Power Supply", &EPS_window);
                ImGui::Checkbox("X-Band", &XBAND_window);
                ImGui::Checkbox("Software Updater", &SW_UPD_window);
                ImGui::Checkbox("System Control", &SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

                ImGui::Separator();

                ImGui::Checkbox("Plaintext RX Display", &RX_display);
                ImGui::Checkbox("ACS Update Display", &ACS_UPD_display);

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
                if (ImGui::BeginTabItem("Getting Started"))
                {
                    ImGui::TextWrapped("Overview");

                    ImGui::Indent();

                    // TODO: Implement a display to show the data retrieved when a data-down command is sent.
                    ImGui::TextWrapped("The SPACE-HAUC Ground Station Client is provided to allow an operator to interface with the SPACE-HAUC satellite.");

                    ImGui::TextWrapped("Data-retrieval commands, referred to as 'data-down,' are sent automatically when the corresponding Arrow Button is pressed. These can be found in the corresponding 'Data-down' drop-down sections of the three 'Operations' panels (ACS, EPS, or XBAND). Once the data is received, it is displayed in the [NO DISPLAY IMPLEMENTED YET].");

                    ImGui::TextWrapped("Value-setting commands, referred to as 'data-up,' are accessed in the 'Data-up' drop-down sections of the three 'Operations' panels. Unlike the 'data-down' commands, 'data-up' commands do not send automatically. Instead, the operator must choose which 'data-up' command they would like to make active using the Radio Buttons. Any number of 'data-up' command arguments can be edited simultaneously, but only one 'data-up' command can be selected for transmission at a time. The arguments are not cleared until the program is restarted.");

                    ImGui::TextWrapped("Once the transmit functionality is 'unlocked' via the 'Communications Control' panel, the operator can check the currently queued command in the 'Transmit' drop-down section. Pressing 'SEND DATA-UP TRANSMISSION' will confirm and send the queued transmission. Any return value or data received as a result of the 'data-up' transmission will also be displayed in the [NO DISPLAY IMPLEMENTED YET].");

                    ImGui::Unindent();
                    ImGui::Separator();

                    ImGui::TextWrapped("Window Interaction");
                    ImGui::Indent();

                    ImGui::TextWrapped("The Menu Bar at the top of the screen has buttons to toggle the visibility of the Authentication, Communications, and User Manual windows. Within the Communications window are checkboxes to toggle the visibility of the Attitude Control System, Electrical Power Supply, X-Band, Software Updater, and System Control windows.");

                    ImGui::TextWrapped("Some windows are a fixed size, while larger windows are able to be resized by the user. All windows have an 'X' in the top right-hand corner to hide the window. Windows can also be hidden the same way they are shown as previously mentioned.");

                    ImGui::Unindent();

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Authentication"))
                {
                    ImGui::TextWrapped("Window");
                    ImGui::Indent();

                    ImGui::TextWrapped("The Authentication Control Panel window is visible by default. Its visibility can be toggled by pressing the Authentication button in the Main Toolbar at the top of the screen.");

                    ImGui::TextWrapped("This window displays the current access level, a password-entry field, and a <DEAUTHENTICATE> button.");

                    ImGui::TextWrapped("Elements of the Ground Station graphical program which are blocked due to insufficient access level will display the following text: \"ACCESS DENIED\".");

                    ImGui::TextWrapped("To enter your password, click on the text entry field and press [ENTER] when finished. The system will process your input and update your access level accordingly. Please note that incorrect inputs are punished with a 2.5 second delay.");

                    ImGui::TextWrapped("To revoke authentication, press the <DEAUTHENTICATE> button. This will not reset any previously set values within the program, but will simply deny further access. This should be used by the operator to lock the application when they are not present to prohibit accidental or malicious acts.");

                    ImGui::TextWrapped("The Authentication Control Panel does not need to be visible to maintain access, and it is recommended to toggle the visibility off once it is no longer needed by pressing the 'Authentication' button on the Menu Bar at the top of the screen.");

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

                if (ImGui::BeginTabItem("Displays Control"))
                {
                    ImGui::TextWrapped("Window");
                    ImGui::Indent();

                    ImGui::TextWrapped("The Displays Control window is unique, as it is the only window which allows for toggling the visibility of other windows. The Displays Control window itself can be toggled by the 'Displays Control' button on the Menu Bar at the top of the screen.");

                    ImGui::TextWrapped("The checkboxes each enable or disable the visibility of their respective windows. From top to bottom, the first section of checkboxes control the visibility of the Attitude Control System, Electrical Power Supply, X-Band, Software Updater, and System Control input windows. The second section of checkboxes controls the visibility of data output windows, which display the data the Client receives from SPACE-HAUC. Toggling the visibility has no effect on the data or actions being performed, except that it disallows any further user interaction with that window's elements (until it is made visible again, of course).");

                    ImGui::TextWrapped("Also in this window is the 'Enable Transmissions' checkbox. This acts as a safety which disallows all data-up transmissions when it is unchecked. This should remain unchecked at ALL TIMES unless one or more data-up transmissions are being made.");

                    ImGui::Unindent();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Communications"))
                {
                    ImGui::Unindent();
                    ImGui::Separator();

                    ImGui::TextWrapped("Command Types");
                    ImGui::Indent();

                    ImGui::TextWrapped("Data-down");
                    ImGui::Indent();
                    ImGui::TextWrapped("Commands where data will be sent down from SPACE-HAUC to the Ground Station. Typically, these are commands that poll one or more values.");
                    ImGui::Unindent();

                    ImGui::TextWrapped("Data-up");
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

                if (ImGui::BeginTabItem("Copyright"))
                {
                    ImGui::TextWrapped("Mit Bailey Copyright (c) 2021");

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("ACS"))
                {
                    ImGui::TextWrapped("Attitude Control System");
                    ImGui::Indent();

                    ImGui::Unindent();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("EPS"))
                {
                    ImGui::TextWrapped("Electrical Power Supply");
                    ImGui::Indent();

                    ImGui::Unindent();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("XBAND"))
                {
                    ImGui::TextWrapped("X-Band");
                    ImGui::Indent();

                    ImGui::Unindent();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("UPD"))
                {
                    ImGui::TextWrapped("Software Updater");
                    ImGui::Indent();

                    ImGui::Unindent();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("SYS"))
                {
                    ImGui::TextWrapped("System Control");
                    ImGui::Indent();

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
            if (ImGui::Button("Displays Control"))
            {
                DISP_control_panel = !DISP_control_panel;
            }
            if (ImGui::Button("User Manual"))
            {
                User_Manual = !User_Manual;
            }

            ImGui::Text("Uptime: %.02f \t\t Framerate: %.02f", ImGui::GetTime(), ImGui::GetIO().Framerate);
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
