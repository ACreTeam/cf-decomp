#ifndef PRFILE2_FA_H
#define PRFILE2_FA_H

// FA front end of the standalone PrFILE2 (SD card file system API); the game-side declarations are in include/lib/revolution/FA/fa.h.

#include <prfile2/pf_stub.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 FAErrnum(void);
void* FACreate(const s8* fileName, s32 unk);
u32 FAFwrite(const void* buffer, u32 size, u32 count, void* stream);
s32 FAFclose(void* stream);
s32 FAFsfirst(const s8* dirName, u32 attr, void* dta);
s32 FAFsnext(void* dta);
s32 FAMkdir(const s8* dirName);
s32 FARmdir(const s8* dirName);
s32 FARemove(const s8* fileName);
s32 FAMount(s8 drive);
s32 FAUnmount(s8 drive, u32 force);

#ifdef __cplusplus
}
#endif
#endif
