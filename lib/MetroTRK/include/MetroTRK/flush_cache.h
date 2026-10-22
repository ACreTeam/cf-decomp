#ifndef METROTRK_FLUSH_CACHE_H
#define METROTRK_FLUSH_CACHE_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void TRK_flush_cache(u32 address, u32 length);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_FLUSH_CACHE_H
