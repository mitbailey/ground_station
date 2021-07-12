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

#define SEC * 1000000
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
 * @brief Holds full integers from the user before we cast them to uint8_t.
 * 
 */
typedef struct
{
    int mod;
    int cmd;
} cmd_input_holder_t;

/**
 * @brief Command structure that SPACE-HAUC transmits.
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

// typedef struct __attribute__((packed))
// {
//     uint8_t ct; // count
//     uint8_t mode; // ACS mode
//     __fp16 bx;
//     __fp16 by;
//     __fp16 bz;
//     __fp16 wx;
//     __fp16 wy;
//     __fp16 wz;
//     __fp16 sx;
//     __fp16 sy;
//     __fp16 sz;
//     uint16_t vbatt;
//     uint16_t vboost;
//     uint16_t cursun;
//     uint16_t cursys;
// } acs_uhf_packet;

// typedef struct __attribute__((packed))
// {
//     bool ready;
// } acs_uhf_packet;

/**
 * @brief 
 * 
 */
typedef struct
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
 * @brief Holds full integers from the user before we cast them to uint8_t.
 * 
 */
typedef struct
{
    int tstep;
    int measure_time;
    int leeway;
    int detumble_angle;
    int sun_angle;
} acs_set_data_holder_t;

/**
 * @brief 
 * 
 */
typedef struct
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
 * @brief 
 * 
 */
typedef struct
{
    int loop_timer;
} eps_set_data_t;

/**
 * @brief 
 * 
 */
typedef struct
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

// From line 113 of https://github.com/SPACE-HAUC/shflight/blob/flight_test/src/cmd_parser.c
// Used for:
// XBAND_SET_TX
// XBAND_SET_RX
// Not used for:       (uses this instead)
// XBAND_DO_TX          xband_tx_data
// XBAND_SET_MAX_ON     uint8_t
// XBAND_SET_TMP_SHDN   uint8_t
// XBAND_SET_TMP_OP     uint8_t
// XBAND_SET_LOOP_TIME  uint8_t
/**
 * @brief 
 * 
 */
typedef struct
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
 * @brief Holds full integers from the user before we cast them to uint8_t.
 * 
 */
typedef struct
{
    int samp;
    int phy_gain;
    int adar_gain;
    int ftr;
    int phase[16];
} xband_set_data_holder_t;

typedef struct
{
    xband_set_data_t RX;
    xband_set_data_holder_t RXH;
    xband_set_data_t TX;
    xband_set_data_holder_t TXH;
} xband_set_data_array_t;

// Used for:
// XBAND_DO_TX          xband_tx_data
/**
 * @brief 
 * 
 */
typedef struct
{
    uint8_t type; // 0 -> text, 1 -> image
    int f_id;     // -1 -> random, 0 -> max
    int mtu;      // 0 -> 128
} xband_tx_data_t;

/**
 * @brief Holds full integers from the user before we cast them to uint8_t.
 * 
 */
typedef struct
{
    int type;
} xband_tx_data_holder_t;

// Used for:
// XBAND_SET_MAX_ON     uint8_t
// XBAND_SET_TMP_SHDN   uint8_t
// XBAND_SET_TMP_OP     uint8_t
// XBAND_SET_LOOP_TIME  uint8_t
/**
 * @brief 
 * 
 */
typedef struct
{
    uint8_t max_on;
    uint8_t tmp_shdn;
    uint8_t tmp_op;
    uint8_t loop_time;
} xband_rxtx_data_t;

typedef struct
{
    int max_on;
    int tmp_shdn;
    int tmp_op;
    int loop_time;
} xband_rxtx_data_holder_t;

/**
 * @brief 
 * 
 */
typedef struct
{
    bool max_on;
    bool tmp_shdn;
    bool tmp_op;
    bool loop_time;
} xband_get_bool_t;

/**
 * @brief 
 * 
 */
typedef struct
{
    bool busy;
    uint8_t access_level;
    char password[64];
} auth_t;

/**
 * @brief 
 * 
 * @param error 
 * @param description 
 */
void glfw_error_callback(int error, const char* description);

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void* gs_gui_check_password(void* arg);

/**
 * @brief 
 * 
 * @param message 
 * @return unsigned int 
 */
unsigned int gs_helper(unsigned char *message);

/**
 * @brief 
 * 
 * @param vp 
 * @return void* 
 */
void* gs_acs_update_data_handler(void* vp);

/**
 * @brief 
 * 
 * @param input 
 * @return int 
 */
int gs_transmit(cmd_input_t *input);

/**
 * @brief 
 * 
 * @param output 
 * @return int 
 */
int gs_receive(cmd_output_t *output);

// int gs_gui_init(GLFWwindow *window);

#endif // GS_GUI_HPP