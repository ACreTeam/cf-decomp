#ifndef METROTRK_NUBEVENT_H
#define METROTRK_NUBEVENT_H


#include <MetroTRK/trk_bool.h>
#include <MetroTRK/msgbuf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    DSError TRKInitializeEventQueue(void);
    bool TRKGetNextEvent(NubEvent* event);
    DSError TRKPostEvent(NubEvent* event);
    void TRKConstructEvent(NubEvent* event, int eventType);
    void TRKDestructEvent(NubEvent* event);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_NUBEVENT_H
