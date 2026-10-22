#ifndef METROTRK_MUTEX_TRK_H
#define METROTRK_MUTEX_TRK_H

#include <MetroTRK/dserror.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned int DSMutex;

    DSError TRKReleaseMutex(void* mutex);
    DSError TRKAcquireMutex(void* mutex);
    DSError TRKInitializeMutex(void* mutex);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_MUTEX_TRK_H
