#ifndef PRFILE2_PF_VOLUME_H
#define PRFILE2_PF_VOLUME_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// drv_char of PFVOL_setvolcfg / PFVOL_getvolcfg: the settings shared by all volumes
#define PF_DRV_CHAR_ALL (-1)

// return value of PFVOL_CheckCurrentDir: start_cluster is the current directory of the calling context
#define PF_VOL_CURRENT_DIR_MATCH (-1)

// mode of PFVOL_setupfsi
enum {
    PF_FSI_MODE_USE_ON = 0x1,
    PF_FSI_MODE_USE_OFF = 0x2,
    PF_FSI_MODE_UPDATE_ON = 0x4,
    PF_FSI_MODE_UPDATE_OFF = 0x8,
};

// mode of PFVOL_setclstlink / PFFILE_fsetclstlink
enum {
    PF_CLUSTER_LINK_DISABLE = 0,
    PF_CLUSTER_LINK_ENABLE = 1,
};

// PF_CLUSTER_LINK_VOL.flag
enum {
    PF_CLUSTER_LINK_FLAG_ENABLED = 0x1,
};

// mode of PFVOL_sync
enum {
    PF_SYNC_NORMAL = 0,
    PF_SYNC_FREE_CACHES = 1,
};

extern struct PF_VOLUME_SET pf_vol_set;
s32 PFVOL_DoMountVolume(struct PF_VOLUME* p_vol);
s32 PFVOL_p_unmount(struct PF_VOLUME* p_vol, u32 mode);
s32 PFVOL_p_format(struct PF_VOLUME* p_vol, const u8* param);
s32 PFVOL_p_setvol(struct PF_VOLUME* p_vol, const s8* label);
s32 PFVOL_p_getvol(struct PF_VOLUME* p_vol, struct PF_VOLTAB* p_voltab);
s32 PFVOL_p_rmvvol(struct PF_VOLUME* p_vol);
s32 PFVOL_InitModule(u32 config, void* param);
s32 PFVOL_CheckForRead(struct PF_VOLUME* p_vol);
s32 PFVOL_CheckForWrite(struct PF_VOLUME* p_vol);
s32 PFVOL_CheckCurrentDir(struct PF_VOLUME* p_vol, u32 start_cluster);
s32 PFVOL_SetCurrentDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_dir);
s32 PFVOL_GetCurrentDir(struct PF_VOLUME* p_vol, struct PF_DIR_ENT* p_current_dir);
void PFVOL_SetCurrentVolume(struct PF_VOLUME* p_vol);
struct PF_VOLUME* PFVOL_GetCurrentVolume(void);
struct PF_VOLUME* PFVOL_GetVolumeFromDrvChar(s8 drv_char);
s32 PFVOL_derrnum(s8 drv_char);
s32 PFVOL_errnum(void);
s32 PFVOL_setvol(s8 drv_char, struct PF_STR* p_str);
s32 PFVOL_getvol(s8 drv_char, struct PF_VOLTAB* p_voltab);
s32 PFVOL_rmvvol(s8 drv_char);
s32 PFVOL_getdev(s8 drv_char, struct PF_DEV_INF* dev_inf);
s32 PFVOL_buffering(s8 drv_char, u8 mode);
s32 PFVOL_attach(struct PF_DRV_TBL* p_drv, struct PF_ATTACH_PARAM* p_param, u32 nand_size);
s32 PFVOL_detach(s8 drv_char);
s32 PFVOL_format(s8 drv_char, const u8* param);
s32 PFVOL_mount(s8 drv_char);
s32 PFVOL_unmount(s8 drv_char, u32 mode);
s32 PFVOL_setupfsi(s8 drv_char, u16 mode);
s32 PFVOL_iswriteprotected(s8 drv_char, u32* p_result);
s32 PFVOL_setclstlink(s8 drv_char, u32 mode, struct PF_CLUSTER_LINK_RES* p_link);
s32 PFVOL_sync(s8 drv_char, u32 mode);
s32 PFVOL_settailbuf(s8 drv_char, u32 size, u32* buf);
s32 PFVOL_setvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg);
s32 PFVOL_getvolcfg(s8 drv_char, struct PF_VOL_CFG* p_cfg);
s32 PFVOL_setcode(struct PF_CHARCODE* p_codeset);
s32 PFVOL_regctx(void);
s32 PFVOL_unregctx(void);
s32 PFVOL_setencode(u32 mode);

#ifdef __cplusplus
}
#endif
#endif
