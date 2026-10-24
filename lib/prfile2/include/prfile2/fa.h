#ifndef PRFILE2_FA_H
#define PRFILE2_FA_H

// FA front end of the standalone PrFILE2 (SD card file system API); the game-side declarations are in include/lib/revolution/FA/fa.h.

#include <prfile2/pdm_api.h>
#include <prfile2/pf_stub.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FA_DEVICE_SD = 0,
    FA_DEVICE_USB,
    FA_DEVICE_NAND,
    FA_DEVICE_CUSTOM,  // the caller's drive table already holds an open partition
    FA_DEVICE_NUM,
};

enum {
    FA_RET_ERR = -1,
    FA_RET_INVALID_PARAM = -2,
    FA_RET_UNMOUNT_FORCED = 1,
};

struct PF_DRV_TBL;

extern PDM_DISK* gOpenDisk[PDM_DRIVE_COUNT];
extern PDM_PARTITION* gOpenPartition[PDM_DRIVE_COUNT];

s32 FAInit(u32 config);
s32 FAAttach(u8 device, const s8* nand_path, u32 nand_size, struct PF_DRV_TBL* p_drv_tbl);
s32 FADetach(s8 drive);
s32 FAErrnum(void);
void* FACreate(const s8* fileName, s32 mode);
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
