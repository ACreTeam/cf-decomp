#ifndef METROTRK_TARGSUPP_H
#define METROTRK_TARGSUPP_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    u32 TRKAccessFile(u32, u32, u32*, u8*);
    u32 TRKOpenFile(u32, u32, u8, u32*);
    u32 TRKCloseFile(u32, u32);
    u32 TRKPositionFile(u32, u32, u32*, u8);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_TARGSUPP_H
