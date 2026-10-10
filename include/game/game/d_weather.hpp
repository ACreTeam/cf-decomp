#ifndef D_WEATHER_H
#define D_WEATHER_H

#include <types.h>

class dTime_c;

// The weather manager (lbl_8074EBE8). Its TU (around 801C98CC) is not split yet; names are inferred.
struct dUnk8074EBE8_c {
    u8 _0000[0x5884];
    int _5884; // the weather: 3/4 rain, 5/6 snow
};

extern "C" {
extern dUnk8074EBE8_c *lbl_8074EBE8;

int fn_801C98CC(dUnk8074EBE8_c *weather); // the current weather (_5884)
// Weather check by time (d_npc_talk_free isWeatherTopic: 0 allows the weather topic by the hour).
int fn_801C9C08(const dTime_c *time); // 801C9C08
}

#endif
