/**
 * @file gs_gui.hpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief Contains function declarations for GUI-specific functions.
 * 
 * Mainly this file contains function declarations for _window functions and their helpers, which control the display and handle data of specific ImGui windows.
 * 
 * @version 0.1
 * @date 2021.06.30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef GS_GUI_HPP
#define GS_GUI_HPP

/**
 * @brief Handles the 'Transmit' section of panels, including display of queued data and the send button.
 * 
 * @param auth Current authentication object.
 * @param command_input The command to be augmented.
 * @return int Positive on success, negative on failure.
 */
int gs_gui_gs2sh_tx_handler(NetworkData *network_data, auth_t *auth, cmd_input_t *command_input, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param AUTH_control_panel 
 * @param auth 
 */
void gs_gui_authentication_control_panel_window(bool *AUTH_control_panel, auth_t *auth);

/**
 * @brief 
 * 
 * @param SETTINGS_window 
 * @param auth 
 */
void gs_gui_settings_window(bool *SETTINGS_window, auth_t *auth);

/**
 * @brief 
 * 
 * @param global_data 
 * @param ACS_window 
 * @param auth 
 * @param allow_transmission 
 */
void gs_gui_acs_window(global_data_t *global_data, bool *ACS_window, auth_t *auth, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param network_data 
 * @param EPS_window 
 * @param auth 
 * @param allow_transmission 
 */
void gs_gui_eps_window(NetworkData *network_data, bool *EPS_window, auth_t *auth, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param network_data 
 * @param XBAND_window 
 * @param auth 
 * @param allow_transmission 
 */
void gs_gui_xband_window(NetworkData *network_data, bool *XBAND_window, auth_t *auth, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param network_data 
 * @param SW_UPD_window 
 * @param auth 
 * @param allow_transmission 
 */
void gs_gui_sw_upd_window(NetworkData *network_data, bool *SW_UPD_window, auth_t *auth, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param network_data 
 * @param SYS_CTRL_window 
 * @param auth 
 * @param allow_transmission 
 */
void gs_gui_sys_ctrl_window(NetworkData *network_data, bool *SYS_CTRL_window, auth_t *auth, bool *allow_transmission);

/**
 * @brief 
 * 
 * @param RX_display 
 * @param global_data 
 */
void gs_gui_rx_display_window(bool *RX_display, global_data_t *global_data);

/**
 * @brief 
 * 
 * @param CONNS_manager 
 * @param auth 
 * @param allow_transmission 
 * @param network_data 
 */
void gs_gui_conns_manager_window(bool *CONNS_manager, auth_t *auth, bool *allow_transmission, NetworkData *network_data);

/**
 * @brief 
 * 
 * @param acs_rolbuf 
 * @param ACS_UPD_display 
 */
void gs_gui_acs_upd_display_window(ACSRollingBuffer *acs_rolbuf, bool *ACS_UPD_display);

/**
 * @brief 
 * 
 * @param DISP_control_panel 
 * @param ACS_window 
 * @param EPS_window 
 * @param XBAND_window 
 * @param SW_UPD_window 
 * @param SYS_CTRL_window 
 * @param RX_display 
 * @param ACS_UPD_display 
 * @param allow_transmission 
 * @param auth 
 */
void gs_gui_disp_control_panel_window(bool *DISP_control_panel, bool *ACS_window, bool *EPS_window, bool *XBAND_window, bool *SW_UPD_window, bool *SYS_CTRL_window, bool *RX_display, bool *ACS_UPD_display, bool *allow_transmission, auth_t *auth);

/**
 * @brief 
 * 
 * @param User_Manual 
 */
void gs_gui_user_manual_window(bool *User_Manual);

#endif // GS_GUI_HPP