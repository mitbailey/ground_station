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
    bool showOtherWindow = false;

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

        // Create a window called "Sample Window".
        if (ImGui::Begin("Sample Window"))
        {
            if (ImGui::Button("Toggle Window"))
            {
                showOtherWindow = !showOtherWindow;
            }

            ImGui::Text("%.1f FPS %.1f", ImGui::GetIO().Framerate, ImGui::GetTime());

            ImGui::End();
        }

        // Other window.
        if (showOtherWindow)
        {
            if (ImGui::Begin("Other Window", &showOtherWindow))
            {
                ImGui::Text("This is another window.");

                ImGui::End();
            }
        }

        static bool show_win_communications = true;

        static bool gs_mod_select_acs = false;
        static bool gs_mod_select_eps = false;
        static bool gs_mod_select_xband = false;
        static bool gs_mod_select_update = false;
        static bool gs_mod_select_acs_update = false;
        static bool gs_mod_select_sys_version = false;
        static bool gs_mod_select_sys_restart_program = false;
        static bool gs_mod_select_sys_reboot = false;
        static bool gs_mod_select_sys_clean_shbytes = false;

        static bool gs_cmd_select_acs_get = true;
        static bool gs_cmd_select_acs_moi_id = false;
        static bool gs_cmd_select_acs_imoi_id = false;
        static bool gs_cmd_select_acs_dipole = false;
        static bool gs_cmd_select_acs_tstep = false;
        static bool gs_cmd_select_acs_measure_time = false;
        static bool gs_cmd_select_acs_leeway = false;
        static bool gs_cmd_select_acs_wtarget = false;
        static bool gs_cmd_select_acs_detumble_ang = false;
        static bool gs_cmd_select_acs_sun_angle = false;

        static bool gs_cmd_select_eps_get = true;
        static bool gs_cmd_select_eps_min_hk = false;
        static bool gs_cmd_select_eps_vbatt = false;
        static bool gs_cmd_select_eps_sys_curr = false;
        static bool gs_cmd_select_eps_outpower = false;
        static bool gs_cmd_select_eps_vsun = false;
        static bool gs_cmd_select_eps_vsun_all = false;
        static bool gs_cmd_select_eps_isun = false;
        static bool gs_cmd_select_eps_loop_timer = false;

        static bool gs_cmd_select_xband_get = true;
        static bool gs_cmd_select_xband_tx = false;
        static bool gs_cmd_select_xband_rx = false;
        static bool gs_cmd_select_xband_max_on = false;
        static bool gs_cmd_select_xband_tmp_shdn = false;
        static bool gs_cmd_select_xband_tmp_op = false;
        static bool gs_cmd_select_xband_loop_time = false;

        static int current_mod_id = INVALID_ID;
        static int current_cmd_id = INVALID_ID;

        // Possibly clean up this boolean mess using (Jimmyee commented on Nov 10, 2017).
        // https://github.com/ocornut/imgui/issues/1422

        if (ImGui::Begin("Communications"), &show_win_communications)
        {
            if (ImGui::BeginMenu("Module ID"))
            {
                ImGui::RadioButton("ACS (0x1)", &current_mod_id, ACS_ID);
                ImGui::RadioButton("EPS (0x2)", &current_mod_id, EPS_ID);
                ImGui::RadioButton("XBAND (0x3)", &current_mod_id, XBAND_ID);
                ImGui::RadioButton("Software Update (0xf)", &current_mod_id, SW_UPD_ID);
                ImGui::RadioButton("ACS Update (0xe)", &current_mod_id, ACS_UPD_ID);
                ImGui::RadioButton("System Version (0xd)", &current_mod_id, SYS_VER_MAGIC);
                ImGui::RadioButton("System Restart Program (0xff)", &current_mod_id, SYS_RESTART_PROG);
                ImGui::RadioButton("System Reboot (0xfe)", &current_mod_id, SYS_REBOOT);
                ImGui::RadioButton("System Clean SHBYTES (0xfd)", &current_mod_id, SYS_CLEAN_SHBYTES);

                ImGui::EndMenu();
            }

            ImGui::Text("Module ID: 0x%x", current_mod_id);

            if (gs_mod_select_acs)
            {
                current_mod_id = ACS_ID;
                static char acs_mode[18]; // "Current Mode: GET\0"
                static bool acs_mode_init = true;
                if (acs_mode_init)
                {
                    snprintf(acs_mode, sizeof(acs_mode), "Current Mode: %s", gs_cmd_select_acs_get ? "GET" : "SET");
                    acs_mode_init = false;
                }
                if (ImGui::Button(acs_mode))
                {
                    gs_cmd_select_acs_get = !gs_cmd_select_acs_get;
                    snprintf(acs_mode, sizeof(acs_mode), "Current Mode: %s", gs_cmd_select_acs_get ? "GET" : "SET");
                    // Reset all values when we switch between get and set.
                    gs_cmd_select_acs_moi_id = false;
                    gs_cmd_select_acs_imoi_id = false;
                    gs_cmd_select_acs_dipole = false;
                    gs_cmd_select_acs_tstep = false;
                    gs_cmd_select_acs_measure_time = false;
                    gs_cmd_select_acs_leeway = false;
                    gs_cmd_select_acs_wtarget = false;
                    gs_cmd_select_acs_detumble_ang = false;
                    gs_cmd_select_acs_sun_angle = false;
                }

                if (gs_cmd_select_acs_get)
                {

                    if (ImGui::BeginMenu("Command ID (Get)"))
                    {
                        ImGui::MenuItem("Get MOI ID", NULL, &gs_cmd_select_acs_moi_id);
                        ImGui::MenuItem("Get IMOI ID", NULL, &gs_cmd_select_acs_imoi_id);
                        ImGui::MenuItem("Get Dipole", NULL, &gs_cmd_select_acs_dipole);
                        ImGui::MenuItem("Get Timestep", NULL, &gs_cmd_select_acs_tstep);
                        ImGui::MenuItem("Get Measure Time", NULL, &gs_cmd_select_acs_measure_time);
                        ImGui::MenuItem("Get Leeway", NULL, &gs_cmd_select_acs_leeway);
                        ImGui::MenuItem("Get W-Target", NULL, &gs_cmd_select_acs_wtarget);
                        ImGui::MenuItem("Get Detumble Angle", NULL, &gs_cmd_select_acs_detumble_ang);
                        ImGui::MenuItem("Get Sun Angle", NULL, &gs_cmd_select_acs_sun_angle);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_acs_moi_id)
                    {
                        current_cmd_id = ACS_GET_MOI_ID;
                    }
                    else if (gs_cmd_select_acs_imoi_id)
                    {
                        current_cmd_id = ACS_GET_IMOI_ID;
                    }
                    else if (gs_cmd_select_acs_dipole)
                    {
                        current_cmd_id = ACS_GET_DIPOLE;
                    }
                    else if (gs_cmd_select_acs_tstep)
                    {
                        current_cmd_id = ACS_GET_TSTEP;
                    }
                    else if (gs_cmd_select_acs_measure_time)
                    {
                        current_cmd_id = ACS_GET_MEASURE_TIME;
                    }
                    else if (gs_cmd_select_acs_leeway)
                    {
                        current_cmd_id = ACS_GET_LEEWAY;
                    }
                    else if (gs_cmd_select_acs_wtarget)
                    {
                        current_cmd_id = ACS_GET_WTARGET;
                    }
                    else if (gs_cmd_select_acs_detumble_ang)
                    {
                        current_cmd_id = ACS_GET_DETUMBLE_ANG;
                    }
                    else if (gs_cmd_select_acs_sun_angle)
                    {
                        current_cmd_id = ACS_GET_SUN_ANGLE;
                    }
                }
                else
                {
                    if (ImGui::BeginMenu("Command ID (Set)"))
                    {
                        ImGui::MenuItem("Set MOI ID", NULL, &gs_cmd_select_acs_moi_id);
                        ImGui::MenuItem("Set IMOI ID", NULL, &gs_cmd_select_acs_imoi_id);
                        ImGui::MenuItem("Set Dipole", NULL, &gs_cmd_select_acs_dipole);
                        ImGui::MenuItem("Set Timestep", NULL, &gs_cmd_select_acs_tstep);
                        ImGui::MenuItem("Set Measure Time", NULL, &gs_cmd_select_acs_measure_time);
                        ImGui::MenuItem("Set Leeway", NULL, &gs_cmd_select_acs_leeway);
                        ImGui::MenuItem("Set W-Target", NULL, &gs_cmd_select_acs_wtarget);
                        ImGui::MenuItem("Set Detumble Angle", NULL, &gs_cmd_select_acs_detumble_ang);
                        ImGui::MenuItem("Set Sun Angle", NULL, &gs_cmd_select_acs_sun_angle);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_acs_moi_id)
                    {
                        current_cmd_id = ACS_SET_MOI_ID;
                    }
                    else if (gs_cmd_select_acs_imoi_id)
                    {
                        current_cmd_id = ACS_SET_IMOI_ID;
                    }
                    else if (gs_cmd_select_acs_dipole)
                    {
                        current_cmd_id = ACS_SET_DIPOLE;
                    }
                    else if (gs_cmd_select_acs_tstep)
                    {
                        current_cmd_id = ACS_SET_TSTEP;
                    }
                    else if (gs_cmd_select_acs_measure_time)
                    {
                        current_cmd_id = ACS_SET_MEASURE_TIME;
                    }
                    else if (gs_cmd_select_acs_leeway)
                    {
                        current_cmd_id = ACS_SET_LEEWAY;
                    }
                    else if (gs_cmd_select_acs_wtarget)
                    {
                        current_cmd_id = ACS_SET_WTARGET;
                    }
                    else if (gs_cmd_select_acs_detumble_ang)
                    {
                        current_cmd_id = ACS_SET_DETUMBLE_ANG;
                    }
                    else if (gs_cmd_select_acs_sun_angle)
                    {
                        current_cmd_id = ACS_SET_SUN_ANGLE;
                    }
                    else
                    {
                        current_cmd_id = INVALID_ID;
                    }
                }
            }
            else if (gs_mod_select_eps)
            {
                current_mod_id = 0x2;
                static char eps_mode[18]; // "Current Mode: GET\0"
                static bool eps_mode_init = true;
                if (eps_mode_init)
                {
                    snprintf(eps_mode, sizeof(eps_mode), "Current Mode: %s", gs_cmd_select_eps_get ? "GET" : "SET");
                    eps_mode_init = false;
                }
                if (ImGui::Button(eps_mode))
                {
                    gs_cmd_select_eps_get = !gs_cmd_select_eps_get;
                    snprintf(eps_mode, sizeof(eps_mode), "Current Mode: %s", gs_cmd_select_eps_get ? "GET" : "SET");

                    // Reset values.
                    gs_cmd_select_eps_min_hk = false;
                    gs_cmd_select_eps_vbatt = false;
                    gs_cmd_select_eps_sys_curr = false;
                    gs_cmd_select_eps_outpower = false;
                    gs_cmd_select_eps_vsun = false;
                    gs_cmd_select_eps_vsun_all = false;
                    gs_cmd_select_eps_isun = false;
                    gs_cmd_select_eps_loop_timer = false;
                }

                if (gs_cmd_select_eps_get)
                {
                    if (ImGui::BeginMenu("Command ID (Get)"))
                    {
                        ImGui::MenuItem("Get Minimal Housekeeping Data", NULL, &gs_cmd_select_eps_min_hk);
                        ImGui::MenuItem("Get Battery Voltage", NULL, &gs_cmd_select_eps_vbatt);
                        ImGui::MenuItem("Get System Current", NULL, &gs_cmd_select_eps_sys_curr);
                        ImGui::MenuItem("Get Power Out", NULL, &gs_cmd_select_eps_outpower);
                        ImGui::MenuItem("Get VSun", NULL, &gs_cmd_select_eps_vsun);
                        ImGui::MenuItem("Get VSun All", NULL, &gs_cmd_select_eps_vsun_all);
                        ImGui::MenuItem("Get ISun", NULL, &gs_cmd_select_eps_isun);
                        ImGui::MenuItem("Get Loop Timer", NULL, &gs_cmd_select_eps_loop_timer);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_eps_min_hk)
                    {
                        current_cmd_id = EPS_GET_MIN_HK;
                    }
                    else if (gs_cmd_select_eps_vbatt)
                    {
                        current_cmd_id = EPS_GET_VBATT;
                    }
                    else if (gs_cmd_select_eps_sys_curr)
                    {
                        current_cmd_id = EPS_GET_SYS_CURR;
                    }
                    else if (gs_cmd_select_eps_outpower)
                    {
                        current_cmd_id = EPS_GET_OUTPOWER;
                    }
                    else if (gs_cmd_select_eps_vsun)
                    {
                        current_cmd_id = EPS_GET_VSUN;
                    }
                    else if (gs_cmd_select_eps_vsun_all)
                    {
                        current_cmd_id = EPS_GET_VSUN_ALL;
                    }
                    else if (gs_cmd_select_eps_isun)
                    {
                        current_cmd_id = EPS_GET_ISUN;
                    }
                    else if (gs_cmd_select_eps_loop_timer)
                    {
                        current_cmd_id = EPS_GET_LOOP_TIMER;
                    }
                    else
                    {
                        current_cmd_id = INVALID_ID;
                    }
                }
                else
                {
                    if (ImGui::BeginMenu("Command ID (Set)"))
                    {
                        ImGui::MenuItem("Set Loop Timer", NULL, &gs_cmd_select_eps_loop_timer);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_eps_loop_timer)
                    {
                        current_cmd_id = EPS_SET_LOOP_TIMER;
                    }
                    else
                    {
                        current_cmd_id = INVALID_ID;
                    }
                }
            }
            else if (gs_mod_select_xband)
            {
                current_mod_id = 0x3;
                static char xband_mode[18]; // "Current Mode: GET\0"
                static bool xband_mode_init = true;
                if (xband_mode_init)
                {
                    snprintf(xband_mode, sizeof(xband_mode), "Current Mode: %s", gs_cmd_select_xband_get ? "GET" : "SET");
                    xband_mode_init = false;
                }
                if (ImGui::Button(xband_mode))
                {
                    gs_cmd_select_xband_get = !gs_cmd_select_xband_get;
                    snprintf(xband_mode, sizeof(xband_mode), "Current Mode: %s", gs_cmd_select_xband_get ? "GET" : "SET");

                    // Reset values.
                    gs_cmd_select_xband_tx = false;
                    gs_cmd_select_xband_rx = false;
                    gs_cmd_select_xband_max_on = false;
                    gs_cmd_select_xband_tmp_shdn = false;
                    gs_cmd_select_xband_tmp_op = false;
                    gs_cmd_select_xband_loop_time = false;
                }

                if (gs_cmd_select_xband_get)
                {
                    if (ImGui::BeginMenu("Command ID (Get)"))
                    {
                        ImGui::MenuItem("Get Max On", NULL, &gs_cmd_select_xband_max_on);
                        ImGui::MenuItem("Get TMP SHDN", NULL, &gs_cmd_select_xband_tmp_shdn);
                        ImGui::MenuItem("Get TMP OP", NULL, &gs_cmd_select_xband_tmp_op);
                        ImGui::MenuItem("Get Loop Time", NULL, &gs_cmd_select_xband_loop_time);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_xband_max_on)
                    {
                        current_cmd_id = XBAND_GET_MAX_ON;
                    }
                    else if (gs_cmd_select_xband_tmp_shdn)
                    {
                        current_cmd_id = XBAND_GET_TMP_SHDN;
                    }
                    else if (gs_cmd_select_xband_tmp_op)
                    {
                        current_cmd_id = XBAND_GET_TMP_OP;
                    }
                    else if (gs_cmd_select_xband_loop_time)
                    {
                        current_cmd_id = XBAND_GET_LOOP_TIME;
                    }
                    else
                    {
                        current_cmd_id = INVALID_ID;
                    }
                }
                else
                {
                    if (ImGui::BeginMenu("Command ID (Set)"))
                    {
                        ImGui::MenuItem("Set Transmit", NULL, &gs_cmd_select_xband_tx);
                        ImGui::MenuItem("Set Receive", NULL, &gs_cmd_select_xband_rx);
                        ImGui::MenuItem("Set Max On", NULL, &gs_cmd_select_xband_max_on);
                        ImGui::MenuItem("Set TMP SHDN", NULL, &gs_cmd_select_xband_tmp_shdn);
                        ImGui::MenuItem("Set TMP OP", NULL, &gs_cmd_select_xband_tmp_op);
                        ImGui::MenuItem("Set Loop Time", NULL, &gs_cmd_select_xband_loop_time);

                        ImGui::EndMenu();
                    }

                    if (gs_cmd_select_xband_tx)
                    {
                        current_cmd_id = XBAND_SET_TX;
                    }
                    else if (gs_cmd_select_xband_rx)
                    {
                        current_cmd_id = XBAND_SET_RX;
                    }
                    else if (gs_cmd_select_xband_max_on)
                    {
                        current_cmd_id = XBAND_SET_MAX_ON;
                    }
                    else if (gs_cmd_select_xband_tmp_shdn)
                    {
                        current_cmd_id = XBAND_SET_TMP_SHDN;
                    }
                    else if (gs_cmd_select_xband_tmp_op)
                    {
                        current_cmd_id = XBAND_SET_TMP_OP;
                    }
                    else if (gs_cmd_select_xband_loop_time)
                    {
                        current_cmd_id = XBAND_SET_LOOP_TIME;
                    }
                    else
                    {
                        current_cmd_id = INVALID_ID;
                    }
                }
            }
            else if (gs_mod_select_update)
            {
                current_mod_id = 0xf;
            }
            else if (gs_mod_select_acs_update)
            {
                current_mod_id = 0xe;
            }
            else if (gs_mod_select_sys_version)
            {
                current_mod_id = 0xd;
            }
            else if (gs_mod_select_sys_restart_program)
            {
                current_mod_id = 0xff;
            }
            else if (gs_mod_select_sys_reboot)
            {
                current_mod_id = 0xfe;
            }
            else if (gs_mod_select_sys_clean_shbytes)
            {
                current_mod_id = 0xfd;
            }

            ImGui::Text("IDs: 0x%x 0x%x", current_mod_id, current_cmd_id);

            ImGui::End();
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
