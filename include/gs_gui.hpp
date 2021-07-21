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
#define SIZE_FRAME_PAYLOAD 56
#define SIZE_FRAME 64
#define CLIENT_FRAME_GUID 0x1A1C
#define MAX_ROLLBUF_LEN 500

#ifndef dbprintlf
#define dbprintlf(format, ...)                                                                        \
    fprintf(stderr, "[%s | %s:%d] " format "\x1b[0m\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stderr);
#endif // dbprintlf

#define FRAME_COLOR "\x1b[91m"
#define PAYLOAD_COLOR "\x1b[94m"
#define RESET_COLOR "\x1b[0m"

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

typedef struct __attribute__((packed))
{
    uint16_t guid;
    uint16_t crc1;
    unsigned char payload[56]; // cmd_input_t or cmd_output_t goes here.
    uint16_t crc2;
    uint16_t termination;
} client_frame_t;

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
 * @brief The ACS update data format sent from SPACE-HAUC to Ground.
 * 
 *  else if (module_id == ACS_UPD_ID)
 *  {
 *      acs_upd.vbatt = eps_vbatt;
 *      acs_upd.vboost = eps_mvboost;
 *      acs_upd.cursun = eps_cursun;
 *      acs_upd.cursys = eps_cursys;
 *      output->retval = 1;
 *      output->data_size = sizeof(acs_uhf_packet);
 *      memcpy(output->data, &acs_upd, output->data_size);
 *  }
 * 
 * @return typedef struct 
 */
#ifndef COMPILING_SYSTEM
#define __fp16 uint16_t
#endif // COMPILING_SYSTEM
typedef struct __attribute__((packed))
{
    uint8_t ct;      // Set in acs.c.
    uint8_t mode;    // Set in acs.c.
    __fp16 bx;       // Set in acs.c.
    __fp16 by;       // Set in acs.c.
    __fp16 bz;       // Set in acs.c.
    __fp16 wx;       // Set in acs.c.
    __fp16 wy;       // Set in acs.c.
    __fp16 wz;       // Set in acs.c.
    __fp16 sx;       // Set in acs.c.
    __fp16 sy;       // Set in acs.c.
    __fp16 sz;       // Set in acs.c.
    uint16_t vbatt;  // Set in cmd_parser.
    uint16_t vboost; // Set in cmd_parser.
    uint16_t cursun; // Set in cmd_parser.
    uint16_t cursys; // Set in cmd_parser.
} acs_upd_output_t;

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

/// From https://github.com/SPACE-HAUC/mtq_tester/blob/master/guimain.cpp
class ScrollBuf
{
public:
    ScrollBuf();
    ScrollBuf(int max_size);

    void AddPoint(float x, float y);
    void Erase();
    float Max();
    float Min();

    int max_sz;
    int ofst;
    ImVector<ImVec2> data;
};
/// ///

// TODO: Implement this rolling buffer.
class ACSRollingBuffer
{
public:
    ACSRollingBuffer();

    /**
     * @brief Adds a value set to the rolling buffer.
     * 
     * @param data The data to be copied into the buffer.
     */
    void addValueSet(acs_upd_output_t data);

    /**
     * @brief Reads a value set from the rolling buffer.
     * 
     * @param data Memory to copy the data into.
     */
    // void getValueSet(acs_upd_output_t* data);

    // double minmax_ct_mode[2];
    // double minmax_b[2];
    // double minmax_w[2];
    // double minmax_s[2];
    // double minmax_v[2];
    // double minmax_cur[2];

    // Separated by the graphs they'll appear in.
    ScrollBuf ct, mode;
    ScrollBuf bx, by, bz;
    ScrollBuf wx, wy, wz;
    ScrollBuf sx, sy, sz;
    ScrollBuf vbatt, vboost;
    ScrollBuf cursun, cursys;

    float x_index;

    bool thread_finished;
private:

    // This is not any one data point, but rather each member should be set independently to the maximum seen.
    // acs_upd_output_t max_values;
    // This is not any one data point, but rather each member should be set independently to the minimum seen.
    // acs_upd_output_t min_values;

    // 'Head' index.
    // int read_index;
    // 'Tail' index.
    // int write_index;

    // Current number of items.
    // int length;
    // Maximum number of items.
    // int max_length;
};

float getMin(float a, float b);

float getMin(float a, float b, float c);

float getMax(float a, float b);

float getMax(float a, float b, float c);

// TODO: Overwrite old data regardless of ready status, but do not display data which is stale, ie isnt ready.
class ACSDisplayData
{
public:
    ACSDisplayData();
    
    acs_upd_output_t data[1];

    // This is so that we don't display the same data multiple times.
    bool ready;
};

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
 * @param output SPACE-HAUC's down-sent data, wrapped in a server-to-client frame.
 * @param acs_display_data Pointer to the class object holding data for the ACS display.
 * @return int 
 */
int gs_receive(ACSRollingBuffer* acs_rolbuf);

/**
 * @brief Generates a 16-bit CRC for the given data.
 * 
 * This is the CCITT CRC 16 polynomial X^16  + X^12  + X^5  + 1.
 * This works out to be 0x1021, but the way the algorithm works
 * lets us use 0x8408 (the reverse of the bit pattern).  The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
 * 
 * This is the same crc16 function used on-board SPACE-HAUC (line 116):
 * https://github.com/SPACE-HAUC/uhf_modem/blob/aa361d13cf1cef9b295a6cd5e2d51c7ae6d59637/uhf_modem.h
 * 
 * @param data_p 
 * @param length 
 * @return uint16_t 
 */
static inline uint16_t crc16(unsigned char *data_p, uint16_t length)
{
#define CRC16_POLY 0x8408
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;

    if (length == 0)
        return (~crc);

    do
    {
        for (i = 0, data = (unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ CRC16_POLY;
            else
                crc >>= 1;
        }
    } while (--length);

    crc = ~crc;
    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xff);

    return (crc);
}

#endif // GS_GUI_HPP