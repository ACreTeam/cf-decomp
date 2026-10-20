#pragma once

#include <revolution/net_fwd.h>
#include <revolution/os/OSTime.h>
#include <revolution/types.h>

#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

s64 DWCi_Np_GetTimeInSeconds(void);
struct tm* DWCi_Np_SecondsToCalendarTime(s64* theTime);
u64 DWCi_Np_GetConsoleId(void);

#define DWCi_Np_TicksToMilliSeconds OSTicksToMilliseconds
#define DWCi_Np_MilliSecondsToTicks(a) OSMillisecondsToTicks((u64)(a))
#define SO_NtoHs SONtoHs

typedef OSTime DWCTick;

static inline DWCTick DWCi_Np_GetTick(void)
{
    return OSGetTime();
}

static inline void DWCi_Np_GetMacAddress(u8* macAddress)
{
    NETGetWirelessMacAddress(macAddress);
}

// the copy every DWCi_Np_CpuCopy* of this revision calls
void DWCi_Np_CPUCopyFast(void*, const void*, u32);

static inline void DWCi_Np_CpuCopy8(
    register const void* srcp, register void* dstp, register u32 size)
{
    DWCi_Np_CPUCopyFast(dstp, srcp, size);
}

static inline void DWCi_Np_CpuCopy16(
    register const void* srcp, register void* dstp, register u32 size)
{
    DWCi_Np_CPUCopyFast(dstp, srcp, size);
}

static inline void DWCi_Np_CpuCopy32(
    register const void* srcp, register void* dstp, register u32 size)
{
    DWCi_Np_CPUCopyFast(dstp, srcp, size);
}

static inline void DWCi_Np_CpuClear8(void* dest, u32 size)
{
    memset(dest, 0, size);
}

static inline void DWCi_Np_CpuClear16(void* dest, u32 size)
{
    memset(dest, 0, size);
}

static inline void DWCi_Np_CpuClear32(void* dest, u32 size)
{
    memset(dest, 0, size);
}

static inline u16 DWCi_HtoLEs(u16 data)
{
    return (u16)((data >> 8) | (data << 8));
}

static inline u32 DWCi_HtoLEl(u32 data)
{
    u32 tmp;
    tmp = ((data >> 8) & 0x00ff00ff) | ((data << 8) & 0xff00ff00);
    return (tmp >> 16) | (tmp << 16);
}

static inline u16 DWCi_LEtoHs(u16 data)
{
    return DWCi_HtoLEs(data);
}

static inline u32 DWCi_LEtoHl(u32 data)
{
    return DWCi_HtoLEl(data);
}

static inline void DWCi_Np_ToLE(const void* srcp, void* dstp, u32 size)
{
    u32 i;

    for (i = 0; i < size / 4; i++)
    {
        ((u32*)dstp)[i] = DWCi_HtoLEl(((u32*)srcp)[i]);
    }
}

#ifdef __cplusplus
}
#endif
