/**
 * @file gs_guimain.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief Ground Station GUI Client
 * @version 0.1
 * @date 2021.06.30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// Implemented receive ( recv(...) ) functionality where as a constant thread.
// Implemented comic-mon-like socket send / receive functionality.
// Implemented __fp16 in acs_upd_output_t as uint16_t.
// Implemented https://github.com/SPACE-HAUC/modem/blob/master/src/guimain.cpp RX thread.
// Implemented proper memory management, including free()'s and destroy()'s.
// Implemented parsing of received ClientServerFrames properly.
// Received data displayed either as plaintext or graphs.
// TODO: Implement XBAND and UHF CLIENTSERVER structures, and receive / display / parse them properly.
// Now sends periodic null NetworkFrames to get the network status in 'netstat.'
// Removed accept() code, since this is not a server. Should only connect().
// TODO: Fix 'taking address of packed member' warnings.
// Removed all raw send(...) calls and replaced them with frame->sendFrame().

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "backend/imgui_impl_glfw.h"
#include "backend/imgui_impl_opengl2.h"
#include "gs.hpp"
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
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enables keyboard navigation.

    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();

    // This is how you set the style yourself.
    // ImGuiStyle *style = &ImGui::GetStyle();
    // style->Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    // Setup platform / renderer backends.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    /// ///////////////////////

    // Network connection setup.
    signal(SIGPIPE, SIG_IGN); // so that client does not die when server does

    // Main loop prep.
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    global_data_t global_data[1] = {0};
    global_data->acs_rolbuf = new ACSRollingBuffer();
    global_data->network_data = new NetworkData();
    global_data->network_data->rx_active = true;
    global_data->last_contact = -1.0;

    auth_t auth = {0};
    bool allow_transmission = false;
    bool AUTH_control_panel = true;
    bool SETTINGS_window = false;
    bool ACS_window = false;
    bool EPS_window = false;
    bool XBAND_window = false;
    bool SW_UPD_window = false;
    bool SYS_CTRL_window = false;
    bool RX_display = false;
    bool ACS_UPD_display = false;
    bool DISP_control_panel = true;
    bool CONNS_manager = true;
    bool User_Manual = false;

    // Set-up and start the RX thread.
    pthread_t rx_thread_id, polling_thread_id;
    pthread_create(&rx_thread_id, NULL, gs_rx_thread, global_data);
    pthread_create(&polling_thread_id, NULL, gs_polling_thread, global_data);

    // Start the receiver thread, passing it our acs_rolbuf (where we will read ACS Update data from) and (perhaps a cmd_output_t for all other data?).

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
        if (AUTH_control_panel)
        {
            gs_gui_authentication_control_panel_window(&AUTH_control_panel, &auth);
        }

        if (SETTINGS_window)
        {
            gs_gui_settings_window(&SETTINGS_window, &auth);
        }

        if (ACS_window)
        {
            gs_gui_acs_window(global_data, &ACS_window, &auth, &allow_transmission);
        }

        if (EPS_window)
        {
            gs_gui_eps_window(global_data->network_data, &ACS_window, &auth, &allow_transmission);
        }

        if (XBAND_window)
        {
            gs_gui_xband_window(global_data, &XBAND_window, &auth, &allow_transmission);
        }

        // Handles software updates.
        if (SW_UPD_window)
        {
            gs_gui_sw_upd_window(global_data->network_data, &XBAND_window, &auth, &allow_transmission);
        }

        // Handles
        // SYS_VER_MAGIC = 0xd,
        // SYS_RESTART_PROG = 0xff,
        // SYS_REBOOT = 0xfe,
        // SYS_CLEAN_SHBYTES = 0xfd
        if (SYS_CTRL_window)
        {
            gs_gui_sys_ctrl_window(global_data->network_data, &SYS_CTRL_window, &auth, &allow_transmission);
        }

        if (RX_display)
        {
            gs_gui_rx_display_window(&RX_display, global_data);
        }

        if (ACS_UPD_display)
        {
            gs_gui_acs_upd_display_window(global_data->acs_rolbuf, &ACS_UPD_display);
        }

        // Network Connections Manager
        if (CONNS_manager)
        {
            gs_gui_conns_manager_window(&CONNS_manager, &auth, &allow_transmission, global_data);
        }

        if (DISP_control_panel)
        {
            gs_gui_disp_control_panel_window(&DISP_control_panel, &ACS_window, &EPS_window, &XBAND_window, &SW_UPD_window, &SYS_CTRL_window, &RX_display, &ACS_UPD_display, &allow_transmission, &auth);
        }

        if (User_Manual)
        {
            gs_gui_user_manual_window(&User_Manual);
        }

        // The main menu bar located at the top of the screen.
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
            if (ImGui::Button("Connections"))
            {
                CONNS_manager = !CONNS_manager;
            }
            if (ImGui::Button("Settings"))
            {
                SETTINGS_window = !SETTINGS_window;
            }
            if (ImGui::Button("User Manual"))
            {
                User_Manual = !User_Manual;
            }

            ImGui::Text("\t\t Uptime: %.02f \t\t Framerate: %.02f", ImGui::GetTime(), ImGui::GetIO().Framerate);
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

    // Finished.
    void *retval;
    pthread_cancel(rx_thread_id);
    pthread_cancel(polling_thread_id);
    pthread_join(rx_thread_id, &retval);
    retval == PTHREAD_CANCELED ? printf("Good rx_thread_id join.\n") : printf("Bad rx_thread_id join.\n");
    pthread_join(polling_thread_id, &retval);
    retval == PTHREAD_CANCELED ? printf("Good polling_thread_id join.\n") : printf("Bad polling_thread_id join.\n");
    close(global_data->network_data->socket);
    delete global_data->acs_rolbuf;
    delete global_data->network_data;

    // Cleanup.
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
}
