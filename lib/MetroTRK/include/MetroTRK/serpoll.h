#ifndef METROTRK_SERPOLL_H
#define METROTRK_SERPOLL_H

#include <MetroTRK/trk.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void TRKGetInput(void);
    MessageBufferID TRKTestForPacket(void);
    void TRKProcessInput(int bufferIdx);

    extern void* gTRKInputPendingPtr;

#ifdef __cplusplus
}
#endif

#endif // METROTRK_SERPOLL_H
