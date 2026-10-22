#ifndef PRIVATE_NWC24_PRIVATE_H
#define PRIVATE_NWC24_PRIVATE_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NWC24Data {
    union {
        const void* pData;  // 0x00
        u32 offset;
    };

    u32 size;  // 0x04
} NWC24Data;

void NWC24Data_Init(NWC24Data* data);
void NWC24Data_SetDataP(NWC24Data* data, const void* ptr, u32 size);

enum {
    NWC24_IOCTL_SUSPEND_SCHEDULER = 1,
    NWC24_IOCTL_TRY_SUSPEND_SCHEDULER = 2,
    NWC24_IOCTL_RESUME_SCHEDULER = 3,
    NWC24_IOCTL_GENERATE_USER_ID = 15,

    NWC24_IOCTL_SET_RTC_COUNTER = 23,
    NWC24_IOCTL_GET_TIME_DIFFERENCE = 24,

    NWC24_IOCTL_SHUTDOWN = 40,
};

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_PRIVATE_H
