#ifndef HBM_COMPAT_H
#define HBM_COMPAT_H

/* Definitions that mscharged-decomp's SDK headers provide and the wii-ipl SDK headers lack. */
#include <revolution/types.h>

typedef unsigned short int byte2_t;
typedef unsigned long int byte4_t;

typedef unsigned char char_t;
typedef unsigned char byte_t;
typedef unsigned char char8_t;
typedef void (*AIDMACallback)();
typedef s32 WPADChannel;
typedef s32 WPADResult;
typedef u32 WPADDeviceType;
#define WPAD_ESUCCESS 0
#define WPAD_ENODEV -1
#define WPAD_ECOMM -2
#define WPAD_DEV_INITIALIZING WPAD_DEV_UNKNOWN
#define OSGetAlarmUserDataAny(type_, alarm_) ((type_)(OSGetAlarmUserData(alarm_)))

#ifndef ALIGN
#define ALIGN(x) __attribute__((aligned(x)))
#endif
#ifndef DECOMP_DONT_INLINE
#define DECOMP_DONT_INLINE __attribute__((never_inline))
#endif
#define OS_MESSAGE_NO_FLAGS 0
#define OSSetAlarmUserDataAny(alarm_, data_) OSSetAlarmUserData(alarm_, (void*)(data_))

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifdef __cplusplus
extern "C" {
#endif
BOOL AXIsInit(void);
double log10(double);
void WPADSetCallbackByKPAD(BOOL);
u8 WPADGetRadioSensitivity(s32);
#ifdef __cplusplus
}
#endif

#endif
