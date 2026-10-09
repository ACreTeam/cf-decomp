#ifndef RVL_SDK_MIX_H
#define RVL_SDK_MIX_H
#include <revolution/AX/AXVPB.h>
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// lib/RVL_SDK/mix/mix.c (not decompiled). Only what the game uses so far.
void MIXInitChannel(AXVPB* axvpb, u32 mode, int input, int auxA, int auxB, int auxC, int pan,
                    int span, int fader);

#ifdef __cplusplus
}
#endif
#endif
