/**
 * @file gs_gui.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief Contains function definitions for GUI-specific functions.
 * 
 * Mainly this file contains function declarations for _window functions, which control the display and handle data of specific ImGui windows.
 * 
 * @version 0.1
 * @date 2021.06.30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "gs.hpp"
#include "gs_gui.hpp"

int gs_gui_gs2sh_tx_handler(NetworkData *network_data, auth_t *auth, cmd_input_t *command_input, bool *allow_transmission)
{
    if (*allow_transmission)
    {
        ImGui::Text("Send Command");
        ImGui::Indent();
        if (auth->access_level > 1)
        {
            ImGui::Text("Queued Transmission");
            ImGui::Text("Module ID ----- 0x%02x", command_input->mod);
            ImGui::Text("Command ID ---- 0x%02x", command_input->cmd);
            ImGui::Text("Unused -------- 0x%02x", command_input->unused);
            ImGui::Text("Data size ----- 0x%02x", command_input->data_size);
            ImGui::Text("Data ---------- ");

            for (int i = 0; i < command_input->data_size; i++)
            {
                ImGui::SameLine(0.0F, 0.0F);
                ImGui::Text("%02x", command_input->data[i]);
            }

            ImGui::Unindent();
            if (ImGui::Button("SEND DATA-UP TRANSMISSION"))
            {
                // Send the transmission.
                gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, command_input, sizeof(cmd_input_t));
            }
        }
    }
    else
    {
        ImGui::Text("TRANSMISSIONS LOCKED");
    }

    return 1;
}

void gs_gui_authentication_control_panel_window(bool *AUTH_control_panel, auth_t *auth)
{
    static pthread_t auth_thread_id;

    if (ImGui::Begin("Authentication Control Panel", AUTH_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (auth->busy)
        {
            ImGui::Text("PROCESSING...");
            ImGui::InputTextWithHint("", "Enter Password", auth->password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
        }
        else
        {
            switch (auth->access_level)
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

            if (ImGui::InputTextWithHint("", "Enter Password", auth->password, 64, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                pthread_create(&auth_thread_id, NULL, gs_check_password, auth);
            }
        }

        if (ImGui::Button("DEAUTHENTICATE"))
        {
            auth->access_level = 0;
        }
    }
    ImGui::End();
}

// TODO: Add functionality.
void gs_gui_settings_window(bool *SETTINGS_window, auth_t *auth)
{
    if (ImGui::Begin("Settings", SETTINGS_window))
    {
        // static int test1 = 0, test2 = 0, test3 = 0;
        // ImGui::Columns(3, "first_column_set", true);
        // ImGui::InputInt("test1", &test1);
        // ImGui::NextColumn();
        // ImGui::InputInt("test2", &test2);
        // ImGui::NextColumn();
        // ImGui::InputInt("test3", &test3);
    }
    ImGui::End();
}

void gs_gui_acs_window(global_data_t *global_data, bool *ACS_window, auth_t *auth, bool *allow_transmission)
{
    static int ACS_command = ACS_INVALID_ID;
    static cmd_input_t ACS_command_input = {.mod = INVALID_ID, .cmd = ACS_INVALID_ID, .unused = 0, .data_size = 0};
    static acs_set_data_t acs_set_data = {0};
    static acs_set_data_holder_t acs_set_data_holder = {0};

    static bool acs_rxtx_automated = false;
    static pthread_t acs_thread_id;

    if (ImGui::Begin("ACS Operations", ACS_window))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {

            if (auth->access_level > 0)
            {

                if (ImGui::ArrowButton("get_moi_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_MOI;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get MOI");

                if (ImGui::ArrowButton("get_imoi_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_IMOI;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get IMOI");

                if (ImGui::ArrowButton("get_dipole_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_DIPOLE;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Dipole");

                if (ImGui::ArrowButton("get_timestep_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_TSTEP;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Timestep");

                if (ImGui::ArrowButton("get_measure_time_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_MEASURE_TIME;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Measure Time");

                if (ImGui::ArrowButton("get_leeway_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_LEEWAY;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Leeway");

                if (ImGui::ArrowButton("get_wtarget_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_WTARGET;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get W-Target");

                if (ImGui::ArrowButton("get_detumble_angle_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_DETUMBLE_ANG;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Detumble Angle");

                if (ImGui::ArrowButton("get_sun_angle_button", ImGuiDir_Right))
                {
                    ACS_command_input.mod = ACS_ID;
                    ACS_command_input.cmd = ACS_GET_SUN_ANGLE;
                    ACS_command_input.unused = 0x0;
                    ACS_command_input.data_size = 0x0;
                    memset(ACS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(global_data->network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &ACS_command_input, sizeof(cmd_input_t));
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
            if (auth->access_level > 1)
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

                ImGui::Text("(%s) Polling for Data-down every 100 ms.", acs_rxtx_automated ? "ON" : "OFF");

                // TODO: Spawn a thread to execute gs_acs_update_data_handler(...) once.
                // If the operator wants to activate the automatic ACS update system...
                if (acs_rxtx_automated)
                {
                    // Spawn a new thread to run
                    if (pthread_mutex_trylock(&global_data->acs_rolbuf->acs_upd_inhibitor) == 0)
                    {
                        pthread_create(&acs_thread_id, NULL, gs_acs_update_thread, global_data);
                    }
                }

                ImGui::Unindent();
                ImGui::Text("Send Command [Data-up]");
                ImGui::Indent();
                if (auth->access_level > 1)
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
                        ACS_command_input.data_size = sizeof(float);
                        memcpy(ACS_command_input.data, &acs_set_data.dipole, ACS_command_input.data_size);
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
                        ACS_command_input.data_size = sizeof(float);
                        memcpy(ACS_command_input.data, &acs_set_data.wtarget, ACS_command_input.data_size);
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
                        ACS_command_input.data_size = -1;
                        break;
                    }
                    }
                    ImGui::Unindent();

                    gs_gui_gs2sh_tx_handler(global_data->network_data, auth, &ACS_command_input, allow_transmission);
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

void gs_gui_eps_window(NetworkData *network_data, bool *EPS_window, auth_t *auth, bool *allow_transmission)
{
    static int EPS_command = EPS_INVALID_ID;
    static cmd_input_t EPS_command_input = {.mod = INVALID_ID, .cmd = EPS_INVALID_ID, .unused = 0, .data_size = 0};
    static eps_set_data_t eps_set_data = {0};

    if (ImGui::Begin("EPS Operations", EPS_window))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {

            if (auth->access_level > 0)
            {
                if (ImGui::ArrowButton("get_min_hk_button", ImGuiDir_Right))
                {
                    EPS_command_input.mod = EPS_ID;
                    EPS_command_input.cmd = EPS_GET_MIN_HK;
                    EPS_command_input.unused = 0x0;
                    EPS_command_input.data_size = 0x0;
                    memset(EPS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &EPS_command_input, sizeof(cmd_input_t));
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

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 1)
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

        if (ImGui::CollapsingHeader("Transmit"))
        {
            if (allow_transmission)
            {
                ImGui::Indent();
                if (auth->access_level > 1)
                {
                    switch (EPS_command_input.cmd)
                    {
                    case EPS_SET_LOOP_TIMER:
                    {
                        // TODO: Remove all .data[0] = some_variable, because this will only set the first byte of .data because its in section of bytes.
                        // TODO: ~DO NOT REMOVE~ UNLESS it is a one-byte kind of data, ie uint8_t.
                        EPS_command_input.data_size = sizeof(int);
                        memcpy(EPS_command_input.data, &eps_set_data.loop_timer, EPS_command_input.data_size);
                        break;
                    }
                    default:
                    {
                        EPS_command_input.data_size = -1;
                        break;
                    }
                    }
                    ImGui::Unindent();

                    gs_gui_gs2sh_tx_handler(network_data, auth, &EPS_command_input, allow_transmission);
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

void gs_gui_xband_window(NetworkData *network_data, bool *XBAND_window, auth_t *auth, bool *allow_transmission)
{
    static int XBAND_command = XBAND_INVALID_ID;
    static cmd_input_t XBAND_command_input = {.mod = INVALID_ID, .cmd = XBAND_INVALID_ID, .unused = 0, .data_size = 0};
    static xband_set_data_array_t xband_set_data = {0};
    static xband_tx_data_t xband_tx_data = {0};
    static xband_tx_data_holder_t xband_tx_data_holder = {0};
    static xband_rxtx_data_t xband_rxtx_data = {0};
    static xband_rxtx_data_holder_t xband_rxtx_data_holder = {0};
    static xband_get_bool_t xband_get_bool = {0};

    if (ImGui::Begin("X-Band Operations", XBAND_window))
    {
        // TODO: Change to arrow-button implementation.
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {
            if (auth->access_level > 0)
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

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 1)
            {
                ImGui::Indent();

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

                ImGui::Separator();

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
            if (auth->access_level > 1)
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
                if (auth->access_level > 1)
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

                    gs_gui_gs2sh_tx_handler(network_data, auth, &XBAND_command_input, allow_transmission);
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

void gs_gui_sw_upd_window(NetworkData *network_data, bool *SW_UPD_window, auth_t *auth, bool *allow_transmission)
{
    static cmd_input_t UPD_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
    static char upd_filename_buffer[256] = {0};

    if (ImGui::Begin("Software Updater Control Panel", SW_UPD_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        // Needs buttons. Values are just #defines and magic values

        ImGui::Text("Name of the file to send:");
        ImGui::InputTextWithHint("", "Name of File", upd_filename_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue);

        // NOTE: Queued file name not sent with software update command, probably doesn't need to be here at all. This is something the Ground Station's version of SW_UPDATE will have to handle.
        ImGui::Text("Queued file name: %s", upd_filename_buffer);

        ImGui::Separator();

        if (auth->access_level > 1)
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
                gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &UPD_command_input, sizeof(cmd_input_t));
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

void gs_gui_sys_ctrl_window(NetworkData *network_data, bool *SYS_CTRL_window, auth_t *auth, bool *allow_transmission)
{
    // static int SYS_command = INVALID_ID;
    static cmd_input_t SYS_command_input = {.mod = INVALID_ID, .cmd = INVALID_ID, .unused = 0, .data_size = 0};
    static cmd_input_holder_t SYS_command_input_holder = {0};

    if (ImGui::Begin("System Control Panel", SYS_CTRL_window, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (ImGui::CollapsingHeader("Data-down Commands"))
        {
            if (auth->access_level > 0)
            {
                if (ImGui::ArrowButton("get_version_magic", ImGuiDir_Right))
                {
                    SYS_command_input.mod = SYS_VER_MAGIC;
                    SYS_command_input.cmd = 0x0;
                    SYS_command_input.unused = 0x0;
                    SYS_command_input.data_size = 0x0;
                    memset(SYS_command_input.data, 0x0, MAX_DATA_SIZE);
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_ROOFUHF, &SYS_command_input, sizeof(cmd_input_t));
                }
                ImGui::SameLine();
                ImGui::Text("Get Version Magic");
            }
        }

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Data-up Commands"))
        {
            if (auth->access_level > 2)
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
                if (auth->access_level > 2)
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

                    gs_gui_gs2sh_tx_handler(network_data, auth, &SYS_command_input, allow_transmission);
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

void gs_gui_rx_display_window(bool *RX_display, global_data_t *global_data)
{
    if (ImGui::Begin("Plaintext RX Display"), RX_display)
    {
        ImGui::Text("ACKNOWLEDGEMENT");
        ImGui::Separator();
        ImGui::Text("N/ACK ---------- %d", global_data->cs_ack->ack);
        ImGui::Text("Code ----------- %d", global_data->cs_ack->code);
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("UHF CONFIGURATION");
        ImGui::Separator();
        ImGui::Text("WIP");
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("X-BAND CONFIGURATION");
        ImGui::Separator();
        ImGui::Text("WIP");
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("SPACE-HAUC COMMAND OUTPUT");
        ImGui::Separator();
        ImGui::Text("Module ---------- %d", global_data->cmd_output->mod);
        ImGui::Text("Command --------- %d", global_data->cmd_output->cmd);
        ImGui::Text("Return Value ---- %d", global_data->cmd_output->retval);
        ImGui::Text("Data Size ------- %d", global_data->cmd_output->data_size);
        ImGui::Text("Data (hex) ------ ");
        for (int i = 0; i < global_data->cmd_output->data_size; i++)
        {
            ImGui::SameLine(0.0F, 0.0F);
            ImGui::Text("%2x", global_data->cmd_output->data[i]);
        }
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("RADIOS STATUS");
        ImGui::Text("Haystack --------- %d", ((global_data->netstat & 0x10) == 0x10) ? 1 : 0);
        ImGui::Text("Roof UHF --------- %d", ((global_data->netstat & 0x40) == 0x40) ? 1 : 0);
        ImGui::Text("Roof X-BAND ------ %d", ((global_data->netstat & 0x20) == 0x20) ? 1 : 0);
        ImGui::Separator();

        ImGui::Separator();
        ImGui::Separator();
    }
    ImGui::End();
}

void gs_gui_conns_manager_window(bool *CONNS_manager, auth_t *auth, bool *allow_transmission, NetworkData *network_data)
{
    if (ImGui::Begin("Connections Manager", CONNS_manager))
    {
        if (auth->access_level >= 0)
        {
            // static int port = LISTENING_PORT;
            // static char ipaddr[16] = LISTENING_IP_ADDRESS;

            auto flag = ImGuiInputTextFlags_ReadOnly;
            if (!(network_data->connection_ready))
            {
                flag = (ImGuiInputTextFlags_)0;
            }
            
            static char destination_ipv4[32];
            static bool first_pass = true;
            if (first_pass)
            {
                strcpy(destination_ipv4, SERVER_IP); // defaults to our own RX ip
                first_pass = false;
            }
            static int destination_port = SERVER_PORT; // defaults to the correct server listening port
            ImGui::InputText("IP Address", destination_ipv4, sizeof(destination_ipv4), flag);
            ImGui::InputInt("Port", &destination_port, 0, 0, flag);

            if (!(network_data->connection_ready) || network_data->socket < 0)
            {
                if (ImGui::Button("Connect"))
                {
                    network_data->serv_ip->sin_port = htons(destination_port);
                    if ((network_data->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    {
                        printf("\nSocket creation error.\n");
                        fflush(stdout);
                        // return -1;
                    }
                    if (inet_pton(AF_INET, destination_ipv4, &network_data->serv_ip->sin_addr) <= 0)
                    {
                        printf("\nInvalid address; Address not supported.\n");
                    }
                    if (connect_w_tout(network_data->socket, (struct sockaddr *)network_data->serv_ip, sizeof(network_data->serv_ip), 1) < 0)
                    {
                        printf("\nConnection failed!\n");
                    }
                    else
                    {
                        network_data->connection_ready = true;
                    }
                }
            }
            else
            {
                if (ImGui::Button("Disconnect"))
                {
                    close(network_data->socket);
                    network_data->socket = -1;
                    network_data->connection_ready = false;
                }
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            // // TODO: This isn't totally relevant for what we're doing here, but its an example of how to send something over the socket connection. Remove it eventually.
            if (network_data->connection_ready && network_data->socket > 0)
            {
                if (ImGui::Button("SEND TEST FRAME"))
                {
                    gs_transmit(network_data, CS_TYPE_DATA, CS_ENDPOINT_CLIENT, NULL, 0);
                }
            }

            if (network_data->connection_ready && network_data->socket > 0)
            {
                static int jpg_qty;
                if (ImGui::InputInt("JPEG Quality", &jpg_qty, 1, 10))
                {
                    static char msg[1024];
                    int sz = snprintf(msg, 1024, "CMD_JPEG_SET_QUALITY%d", jpg_qty);
                    send(network_data->socket, msg, sz, 0);
                }
            }
        }
        ImGui::End();
    }
}

void gs_gui_acs_upd_display_window(ACSRollingBuffer *acs_rolbuf, bool *ACS_UPD_display)
{
    if (ImGui::Begin("ACS Update Display", ACS_UPD_display))
    {
        // The implemented method of displaying the ACS update data includes a locally-global class (ACSDisplayData) with data that this window will display. The data is set by gs_receive.
        // NOTE: This window must be opened independent of ACS's automated data retrieval option.

        // TODO: Remove these random data-adders and implement the actual data adding in gs_receive. ~Removed, but moved to gs_receive.

        // TODO: Remove this once testing is complete.
        // acs_rolbuf->x_index += 0.1;

        double start_time = acs_rolbuf->x_index - 60;
        if (start_time < 0)
        {
            start_time = 0;
        }

        if (ImGui::CollapsingHeader("CT / Mode Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->mode.Min(), acs_rolbuf->ct.Min()), getMax(acs_rolbuf->mode.Max(), acs_rolbuf->ct.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("CT / Mode Graph"))
            {

                ImPlot::PlotLine("CT", &acs_rolbuf->ct.data[0].x, &acs_rolbuf->ct.data[0].y, acs_rolbuf->ct.data.size(), acs_rolbuf->ct.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("Mode", &acs_rolbuf->mode.data[0].x, &acs_rolbuf->mode.data[0].y, acs_rolbuf->mode.data.size(), acs_rolbuf->mode.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("B (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->bx.Min(), acs_rolbuf->by.Min(), acs_rolbuf->bz.Min()), getMax(acs_rolbuf->bx.Max(), acs_rolbuf->by.Max(), acs_rolbuf->bz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("B (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->bx.data[0].x, &acs_rolbuf->bx.data[0].y, acs_rolbuf->bx.data.size(), acs_rolbuf->bx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->by.data[0].x, &acs_rolbuf->by.data[0].y, acs_rolbuf->by.data.size(), acs_rolbuf->by.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->bz.data[0].x, &acs_rolbuf->bz.data[0].y, acs_rolbuf->bz.data.size(), acs_rolbuf->bz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("W (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->wx.Min(), acs_rolbuf->wy.Min(), acs_rolbuf->wz.Min()), getMax(acs_rolbuf->wx.Max(), acs_rolbuf->wy.Max(), acs_rolbuf->wz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("W (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->wx.data[0].x, &acs_rolbuf->wx.data[0].y, acs_rolbuf->wx.data.size(), acs_rolbuf->wx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->wy.data[0].x, &acs_rolbuf->wy.data[0].y, acs_rolbuf->wy.data.size(), acs_rolbuf->wy.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->wz.data[0].x, &acs_rolbuf->wz.data[0].y, acs_rolbuf->wz.data.size(), acs_rolbuf->wz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("S (x, y, z) Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->sx.Min(), acs_rolbuf->sy.Min(), acs_rolbuf->sz.Min()), getMax(acs_rolbuf->sx.Max(), acs_rolbuf->sy.Max(), acs_rolbuf->sz.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("S (x, y, z) Graph"))
            {

                ImPlot::PlotLine("x", &acs_rolbuf->sx.data[0].x, &acs_rolbuf->sx.data[0].y, acs_rolbuf->sx.data.size(), acs_rolbuf->sx.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("y", &acs_rolbuf->sy.data[0].x, &acs_rolbuf->sy.data[0].y, acs_rolbuf->sy.data.size(), acs_rolbuf->sy.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("z", &acs_rolbuf->sz.data[0].x, &acs_rolbuf->sz.data[0].y, acs_rolbuf->sz.data.size(), acs_rolbuf->sz.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("Battery Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->vbatt.Min(), acs_rolbuf->vboost.Min()), getMax(acs_rolbuf->vbatt.Max(), acs_rolbuf->vbatt.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("Battery Graph"))
            {

                ImPlot::PlotLine("VBatt", &acs_rolbuf->vbatt.data[0].x, &acs_rolbuf->vbatt.data[0].y, acs_rolbuf->vbatt.data.size(), acs_rolbuf->vbatt.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("VBoost", &acs_rolbuf->vboost.data[0].x, &acs_rolbuf->vboost.data[0].y, acs_rolbuf->vboost.data.size(), acs_rolbuf->vboost.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }

        if (ImGui::CollapsingHeader("Solar Current Graph"))
        {
            ImPlot::SetNextPlotLimits(start_time, acs_rolbuf->x_index, getMin(acs_rolbuf->cursun.Min(), acs_rolbuf->cursun.Min()), getMax(acs_rolbuf->cursys.Max(), acs_rolbuf->cursys.Max()), ImGuiCond_Always);
            if (ImPlot::BeginPlot("Solar Current Graph"))
            {

                ImPlot::PlotLine("CurSun", &acs_rolbuf->cursun.data[0].x, &acs_rolbuf->cursun.data[0].y, acs_rolbuf->cursun.data.size(), acs_rolbuf->cursun.ofst, 2 * sizeof(float));

                ImPlot::PlotLine("CurSys", &acs_rolbuf->cursys.data[0].x, &acs_rolbuf->cursys.data[0].y, acs_rolbuf->cursys.data.size(), acs_rolbuf->cursys.ofst, 2 * sizeof(float));

                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();
}

void gs_gui_disp_control_panel_window(bool *DISP_control_panel, bool *ACS_window, bool *EPS_window, bool *XBAND_window, bool *SW_UPD_window, bool *SYS_CTRL_window, bool *RX_display, bool *ACS_UPD_display, bool *allow_transmission, auth_t *auth)
{
    if (ImGui::Begin("Displays Control Panel", DISP_control_panel, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar /*| ImGuiWindowFlags_MenuBar*/))
    {
        ImGui::Checkbox("Attitude Control System", ACS_window); // Contains ACS and ACS_UPD
        ImGui::Checkbox("Electrical Power Supply", EPS_window);
        ImGui::Checkbox("X-Band", XBAND_window);
        ImGui::Checkbox("Software Updater", SW_UPD_window);
        ImGui::Checkbox("System Control", SYS_CTRL_window); // Contains SYS_VER, SYS_REBOOT, SYS_CLEAN_SHBYTES

        ImGui::Separator();

        ImGui::Checkbox("Plaintext RX Display", RX_display);
        ImGui::Checkbox("ACS Update Display", ACS_UPD_display);

        ImGui::Separator();

        if (auth->access_level >= 0)
        {
            ImGui::Checkbox("Unlock Transmissions", allow_transmission);
        }
        else
        {
            ImGui::Text("ACCESS DENIED");
        }
    }
    ImGui::End();
}

void gs_gui_user_manual_window(bool *User_Manual)
{
    if (ImGui::Begin("User Manual", User_Manual, ImGuiWindowFlags_AlwaysVerticalScrollbar /* | ImGuiWindowFlags_HorizontalScrollbar*/))
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