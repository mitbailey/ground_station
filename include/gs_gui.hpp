/**
 * @file gs_gui.hpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef GS_GUI_HPP
#define GS_GUI_HPP

#define SEC *1000000
#define MAX_DATA_SIZE 46
#define ACS_UPD_DATARATE 100

/**
 * @brief Numeric identifiers for determining what module a command is for.
 * 
 */
enum MODULE_ID
{
    INVALID_ID = 0x0,
    ACS_ID = 0x1,
    EPS_ID = 0x2,
    XBAND_ID = 0x3,
    SW_UPD_ID = 0xf,
    ACS_UPD_ID = 0xe,
    SYS_VER_MAGIC = 0xd,
    SYS_RESTART_PROG = 0xff,
    SYS_REBOOT = 0xfe,
    SYS_CLEAN_SHBYTES = 0xfd
};

// Function magic for system restart command, replaces .cmd value.
#define SYS_RESTART_FUNC_MAGIC 0x3c
// Data value for system restart command, replaces .data[] values.
#define SYS_RESTART_FUNC_VAL 0x2fa45d2002d54af2

// Function magic for system reboot command, replaces .cmd value.
#define SYS_REBOOT_FUNC_MAGIC 0x9d
// Data value for system restart command, replaces .data[] values.
#define SYS_REBOOT_FUNC_VAL 0x36a45d2002d54af0

/**
 * @brief Function magic for software update command, replaces .cmd value.
 * 
 */
enum SW_UPD_FUNC_ID
{
    SW_UPD_FUNC_MAGIC = 0x87,
};

// Data value for software update command, replaces .data[] values.
#define SW_UPD_VALID_MAGIC 0x2489f750228d2e4fL

/**
 * @brief ID values for each ACS command.
 * 
 */
enum ACS_FUNC_ID
{
    ACS_INVALID_ID = 0x0,
    ACS_GET_MOI,
    ACS_SET_MOI,
    ACS_GET_IMOI,
    ACS_SET_IMOI,
    ACS_GET_DIPOLE,
    ACS_SET_DIPOLE,
    ACS_GET_TSTEP,
    ACS_SET_TSTEP,
    ACS_GET_MEASURE_TIME,
    ACS_SET_MEASURE_TIME,
    ACS_GET_LEEWAY,
    ACS_SET_LEEWAY,
    ACS_GET_WTARGET,
    ACS_SET_WTARGET,
    ACS_GET_DETUMBLE_ANG,
    ACS_SET_DETUMBLE_ANG,
    ACS_GET_SUN_ANGLE,
    ACS_SET_SUN_ANGLE
};

/**
 * @brief ID values for each EPS command.
 * 
 */
enum EPS_FUNC_ID
{
    EPS_INVALID_ID = 0x0,
    EPS_GET_MIN_HK = 0x1,
    EPS_GET_VBATT,
    EPS_GET_SYS_CURR,
    EPS_GET_OUTPOWER,
    EPS_GET_VSUN,
    EPS_GET_VSUN_ALL,
    EPS_GET_ISUN,
    EPS_GET_LOOP_TIMER,
    EPS_SET_LOOP_TIMER,
};

/**
 * @brief ID values for each XBAND command.
 * 
 */
enum XBAND_FUNC_ID
{
    XBAND_INVALID_ID = 0x0,
    XBAND_SET_TX,
    XBAND_SET_RX,
    XBAND_DO_TX,
    XBAND_DO_RX,
    XBAND_DISABLE,
    XBAND_SET_MAX_ON,
    XBAND_GET_MAX_ON,
    XBAND_SET_TMP_SHDN,
    XBAND_GET_TMP_SHDN,
    XBAND_SET_TMP_OP,
    XBAND_GET_TMP_OP,
    XBAND_GET_LOOP_TIME,
    XBAND_SET_LOOP_TIME,
};

/**
 * @brief Command structure that SPACE-HAUC receives.
 * 
 */
typedef struct __attribute__((packed))
{
    uint8_t mod;
    uint8_t cmd;
    int unused;
    int data_size;
    unsigned char data[46];
} cmd_input_t;

/**
 * @brief ImGui full-size integer interface; holds integers until cast.
 * 
 */
typedef struct __attribute__((packed))
{
    int mod;
    int cmd;
} cmd_input_holder_t;

/**
 * @brief Command structure that SPACE-HAUC transmits to Ground.
 * 
 */
typedef struct __attribute__((packed))
{
    uint8_t mod;            // 1
    uint8_t cmd;            // 1
    int retval;             // 4
    int data_size;          // 4
    unsigned char data[46]; // 46
} cmd_output_t;

/**
 * @brief Used for asynchronous transmission of acs_upd requests.
 * 
 */
typedef struct __attribute__((packed))
{
    cmd_input_t cmd_input[1];
    bool ready;
} acs_upd_input_t;

/**
 * @brief Set of possible data ACS can set.
 * 
 */
typedef struct __attribute__((packed))
{
    float moi[9];
    float imoi[9];
    float dipole;
    uint8_t tstep;
    uint8_t measure_time;
    uint8_t leeway;
    float wtarget;
    uint8_t detumble_angle;
    uint8_t sun_angle;
} acs_set_data_t;

/**
 * @brief ImGui full-size integer interface; holds integers until cast.
 * 
 */
typedef struct __attribute__((packed))
{
    int tstep;
    int measure_time;
    int leeway;
    int detumble_angle;
    int sun_angle;
} acs_set_data_holder_t;

/**
 * @brief Set of possible booleans ACS can use.
 * 
 */
typedef struct __attribute__((packed))
{
    bool moi;
    bool imoi;
    bool dipole;
    bool tstep;
    bool measure_time;
    bool leeway;
    bool wtarget;
    bool detumble_angle;
    bool sun_angle;
} acs_get_bool_t;

/**
 * @brief Set of possible data EPS can set.
 * 
 */
typedef struct __attribute__((packed))
{
    int loop_timer;
} eps_set_data_t;

/**
 * @brief Set of possible booleans EPS can use.
 * 
 */
typedef struct __attribute__((packed))
{
    bool min_hk;
    bool vbatt;
    bool sys_curr;
    bool outpower;
    bool vsun;
    bool vsun_all;
    bool isun;
    bool loop_timer;
} eps_get_bool_t;

/**
 * @brief X-Band data structure.
 * 
 * From line 113 of https://github.com/SPACE-HAUC/shflight/blob/flight_test/src/cmd_parser.c
 * Used for:
 *  XBAND_SET_TX
 *  XBAND_SET_RX
 * 
 */
typedef struct __attribute__((packed))
{
    float LO;
    float bw;
    uint16_t samp;
    uint8_t phy_gain;
    uint8_t adar_gain;
    uint8_t ftr;
    short phase[16];
} xband_set_data_t;

/**
 * @brief ImGui full-size integer interface; holds integers until cast.
 * 
 */
typedef struct __attribute__((packed))
{
    int samp;
    int phy_gain;
    int adar_gain;
    int ftr;
    int phase[16];
} xband_set_data_holder_t;

/**
 * @brief Set of possible data XBAND can set.
 * 
 */
typedef struct __attribute__((packed))
{
    xband_set_data_t RX;
    xband_set_data_holder_t RXH;
    xband_set_data_t TX;
    xband_set_data_holder_t TXH;
} xband_set_data_array_t;

/**
 * @brief X-Band TX structure.
 * 
 * Used for:
 *  XBAND_DO_TX
 * 
 */
typedef struct __attribute__((packed))
{
    uint8_t type; // 0 -> text, 1 -> image          1
    int f_id;     // -1 -> random, 0 -> max         4
    int mtu;      // 0 -> 128                       4
} xband_tx_data_t;

/**
 * @brief ImGui full-size integer interface; holds integers until cast.
 * 
 */
typedef struct __attribute__((packed))
{
    int type;
} xband_tx_data_holder_t;

/**
 * @brief General data for X-Band.
 * 
 * Used for:
 *  XBAND_SET_MAX_ON
 *  XBAND_SET_TMP_SHDN
 *  XBAND_SET_TMP_OP
 *  XBAND_SET_LOOP_TIME
 * 
 */
typedef struct __attribute__((packed))
{
    uint8_t max_on;
    uint8_t tmp_shdn;
    uint8_t tmp_op;
    uint8_t loop_time;
} xband_rxtx_data_t;

/**
 * @brief ImGui full-size integer interface; holds integers until cast. 
 * 
 */
typedef struct __attribute__((packed))
{
    int max_on;
    int tmp_shdn;
    int tmp_op;
    int loop_time;
} xband_rxtx_data_holder_t;

/**
 * @brief Set of possible booleans XBAND can use.
 * 
 */
typedef struct __attribute__((packed))
{
    bool max_on;
    bool tmp_shdn;
    bool tmp_op;
    bool loop_time;
} xband_get_bool_t;

/**
 * @brief Authentication structure, used to store auth-related information.
 * 
 */
typedef struct __attribute__((packed))
{
    bool busy;
    uint8_t access_level;
    char password[64];
} auth_t;

/**
 * @brief Default ImGui callback function.
 * 
 * @param error 
 * @param description 
 */
void glfw_error_callback(int error, const char *description);

/**
 * @brief Checks the validity of the user-input password candidate.
 * 
 * @param arg The entered password candidate.
 * @return void* Pointer to the original argument.
 */
void *gs_gui_check_password(void *arg);

/**
 * @brief Does the actual password checking.
 * 
 * @param message Data to be checked.
 * @return unsigned int 
 */
unsigned int gs_helper(unsigned char *a);

/**
 * @brief Handles the asynchronous sending of the ACS update data retrieval command.
 * 
 * @param vp Pointer to the acs_upd_input_t structure.
 * @return void* Pointer to the original argument.
 */
void *gs_acs_update_data_handler(void *vp);

/**
 * @brief Transmits data to SPACE-HAUC.
 * 
 * @param input The data to transmit.
 * @return int Positive on success, negative on failure.
 */
int gs_transmit(cmd_input_t *input);

/**
 * @brief Handles the 'Transmit' section of panels, including display of queued data and the send button.
 * 
 * @param auth Current authentication object.
 * @param command_input The command to be augmented.
 * @return int Positive on success, negative on failure.
 */
int gs_gui_transmissions_handler(auth_t *auth, cmd_input_t *command_input);

/**
 * @brief Handles receiving data from SPACE-HAUC.
 * 
 * TODO: CURRENTLY UNIMPLEMENTED. Implement, and deploy where needed.
 * 
 * @param output 
 * @return int 
 */
int gs_receive(cmd_output_t *output);

#endif // GS_GUI_HPP