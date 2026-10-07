#ifndef D_WEATHER_H
#define D_WEATHER_H

#include <types.h>

// The weather manager (lbl_8074EBE8). Its TU (around 801C98CC) is not split yet; names are inferred.
struct dUnk8074EBE8_c {
    u8 _0000[0x5884];
    int _5884; // the weather: 3/4 rain, 5/6 snow
};

extern "C" {
extern dUnk8074EBE8_c *lbl_8074EBE8;

int fn_801C98CC(dUnk8074EBE8_c *weather); // the current weather (_5884)
}

#endif
