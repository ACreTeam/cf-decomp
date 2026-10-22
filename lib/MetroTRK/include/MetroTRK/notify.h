#ifndef METROTRK_NOTIFY_H
#define METROTRK_NOTIFY_H

#include <MetroTRK/dserror.h>
#include <MetroTRK/trkenum.h>

#ifdef __cplusplus
extern "C"
{
#endif

    DSError TRKDoNotifyStopped(MessageCommandID cmd);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_NOTIFY_H
