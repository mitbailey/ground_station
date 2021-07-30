/**
 * @file buffer.hpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief 
 * @version See Git tags for version information.
 * @date 2021.07.30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <pthread.h>
#include <stdint.h>
#include "implot/implot.h"

#define MAX_ROLLBUF_LEN 500

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
#ifndef __fp16
#define __fp16 uint16_t
#endif // __fp16
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

// From https://github.com/SPACE-HAUC/mtq_tester/blob/master/guimain.cpp
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

class ACSRollingBuffer
{
public:
    ACSRollingBuffer();

    ~ACSRollingBuffer();

    /**
     * @brief Adds a value set to the rolling buffer.
     * 
     * @param data The data to be copied into the buffer.
     */
    void addValueSet(acs_upd_output_t data);

    // Separated by the graphs they'll appear in.
    ScrollBuf ct, mode;
    ScrollBuf bx, by, bz;
    ScrollBuf wx, wy, wz;
    ScrollBuf sx, sy, sz;
    ScrollBuf vbatt, vboost;
    ScrollBuf cursun, cursys;

    float x_index;

    pthread_mutex_t acs_upd_inhibitor;
};

#endif // BUFFER_HPP