#ifndef METROTRK_MSGHNDLR_H
#define METROTRK_MSGHNDLR_H

#include <MetroTRK/trk.h>

#ifdef __cplusplus
extern "C"
{
#endif

    BOOL GetTRKConnected(void);
    void SetTRKConnected(BOOL connected);
    DSError TRKDoSetOption(TRKBuffer* buffer);
    DSError TRKDoStop(TRKBuffer* buffer);
    DSError TRKDoStep(TRKBuffer* buffer);
    DSError TRKDoContinue(TRKBuffer* buffer);
    DSError TRKDoWriteRegisters(TRKBuffer* buffer);
    DSError TRKDoReadRegisters(TRKBuffer* buffer);
    DSError TRKDoWriteMemory(TRKBuffer* buffer);
    DSError TRKDoReadMemory(TRKBuffer* buffer);
    DSError TRKDoSupportMask(TRKBuffer* buffer);
    DSError TRKDoVersions(TRKBuffer* buffer);
    DSError TRKDoOverride(TRKBuffer* buffer);
    DSError TRKDoReset(TRKBuffer* buffer);
    DSError TRKDoDisconnect(TRKBuffer* buffer);
    DSError TRKDoConnect(TRKBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_MSGHNDLR_H
