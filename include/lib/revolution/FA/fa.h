#ifndef RVL_SDK_FA_H
#define RVL_SDK_FA_H

// PrFILE2 "FA" front end (SD card file system API), see lib/prfile2.
#include <lib/revolution/FA/fa_types.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FAError FAInit(int flag);
FAError FARegistCB(u8 device, FAInsertCallback insertCallback, FAEjectCallback ejectCallback);

FAError FAAttach(u8 device, char* fsPathNAND, u32 fsSizeNAND, FADrvTbl* driveTable);
FAError FADetach(s8 drive);

FAError FAMount(s8 drive);
FAError FAUnmount(s8 drive, u32 force);

FAError FAErrnum();

FAFILE* FACreate(const char* fileName, int mode);
size_t FAFwrite(const void* buffer, size_t size, size_t count, FAFILE* stream);
FAError FAFclose(FAFILE* stream);

FAError FAFsfirst(const char* dirName, u8 attr, FADta* dta);
FAError FAFsnext(FADta* dta);

FAError FAMkdir(const char* dirName);
FAError FARmdir(const char* dirName);
FAError FARemove(const char* fileName);

#ifdef __cplusplus
}
#endif

#endif
