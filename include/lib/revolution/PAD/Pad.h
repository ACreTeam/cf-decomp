#ifndef RVL_SDK_PAD_H
#define RVL_SDK_PAD_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

extern u32 __PADSpec;

typedef struct PADStatus {
    u16 button;   // at 0x0
    s8 stickX;    // at 0x2
    s8 stickY;    // at 0x3
    s8 substickX; // at 0x4
    s8 substickY; // at 0x5
    u8 triggerL;  // at 0x6
    u8 triggerR;  // at 0x7
    u8 analogA;   // at 0x8
    u8 analogB;   // at 0x9
    s8 err;       // at 0xA
} PADStatus;

typedef enum {
    PAD_FLAG_NO_RECALIBRATE = (1 << 6),
} PADFlag;

BOOL __PADDisableRecalibration(BOOL disable);

u32 PADRead(PADStatus* status);
void PADClampCircle(PADStatus* status);

#ifdef __cplusplus
}
#endif
#endif
