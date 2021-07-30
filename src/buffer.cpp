/**
 * @file buffer.cpp
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief 
 * @version See Git tags for version information.
 * @date 2021.07.30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "buffer.hpp"

ScrollBuf::ScrollBuf()
{
    max_sz = 600;
    ofst = 0;
    data.reserve(max_sz);
}

ScrollBuf::ScrollBuf(int max_size)
{
    max_sz = max_size;
    ofst = 0;
    data.reserve(max_sz);
}

void ScrollBuf::AddPoint(float x, float y)
{
    if (data.size() < max_sz)
        data.push_back(ImVec2(x, y));
    else
    {
        data[ofst] = ImVec2(x, y);
        ofst = (ofst + 1) % max_sz;
    }
}

void ScrollBuf::Erase()
{
    if (data.size() > 0)
    {
        data.shrink(0);
        ofst = 0;
    }
}

float ScrollBuf::Max()
{
    float max = data[0].y;
    for (int i = 0; i < data.size(); i++)
        if (data[i].y > max)
            max = data[i].y;
    return max;
}

float ScrollBuf::Min()
{
    float min = data[0].y;
    for (int i = 0; i < data.size(); i++)
        if (data[i].y < min)
            min = data[i].y;
    return min;
}

ACSRollingBuffer::ACSRollingBuffer()
{
    x_index = 0;

    acs_upd_output_t dummy[1];
    memset(dummy, 0x0, sizeof(acs_upd_output_t));

    // Avoids a crash.
    addValueSet(*dummy);

    pthread_mutex_init(&acs_upd_inhibitor, NULL);
}

void ACSRollingBuffer::addValueSet(acs_upd_output_t data)
{
    ct.AddPoint(x_index, data.ct);
    mode.AddPoint(x_index, data.mode);
    bx.AddPoint(x_index, data.bx);
    by.AddPoint(x_index, data.by);
    bz.AddPoint(x_index, data.bz);
    wx.AddPoint(x_index, data.wx);
    wy.AddPoint(x_index, data.wy);
    wz.AddPoint(x_index, data.wz);
    sx.AddPoint(x_index, data.sx);
    sy.AddPoint(x_index, data.sy);
    sz.AddPoint(x_index, data.sz);
    vbatt.AddPoint(x_index, data.vbatt);
    vboost.AddPoint(x_index, data.vboost);
    cursun.AddPoint(x_index, data.cursun);
    cursys.AddPoint(x_index, data.cursys);

    x_index += 0.1;
}

ACSRollingBuffer::~ACSRollingBuffer()
{
    pthread_mutex_destroy(&acs_upd_inhibitor);
}