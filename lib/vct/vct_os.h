#ifndef VCT_OS_H
#define VCT_OS_H

#include <revolution/os.h>
#include <revolution/types.h>

// NITRO tick conversions as the Wii build of the library has them: 64-bit ticks
// from OSGetTime and conversions based on the 0x800000F8 bus clock word.
#define VCT_BUS_CLOCK (*(u32*)0x800000F8)
#define VCT_TIMER_CLOCK (VCT_BUS_CLOCK / 4)

#define VCT_MSEC_TO_TICKS(ms) ((u64)(ms) * (VCT_TIMER_CLOCK / 1000))
#define VCT_USEC_TO_TICKS(us) (((u64)(us) * (VCT_TIMER_CLOCK / 125000)) / 8)
#define VCT_TICKS_TO_MSEC(ticks) ((s64)(ticks) / ((s64)VCT_TIMER_CLOCK / 1000))

#endif
