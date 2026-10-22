#ifndef METROTRK_DEBUGGERDRIVER_H
#define METROTRK_DEBUGGERDRIVER_H



#include <private/os/OSInterrupt.h>
#include <MetroTRK/trk_bool.h>
#include <revolution/os/OSInterrupt.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void DBInitComm(u8** flagOut, __OSInterruptHandler handler);
    void DBInitInterrupts(void);
    u32 DBQueryData(void);
    bool DBRead(void* dst, u32 size);
    bool DBWrite(const void* src, u32 size);
    void DBOpen(void);
    void DBClose(void);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_DEBUGGERDRIVER_H
