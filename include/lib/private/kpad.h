#ifndef PRIVATE_KPAD_H
#define PRIVATE_KPAD_H

// KPAD internals (Aug 2007 layout, 0x538-byte KPADInsideStatus: the Jun 2007 one plus the four play modes) used by RVL_SDK/src/kpad/KPAD.c.
// Adapted from wii-news-channel's include/revolution/kpad.h (CC0) onto the wii-ipl public headers.

#include <revolution/kpad.h>
#include <revolution/mtx.h>
#include <revolution/wpad.h>

#ifdef __cplusplus
extern "C" {
#endif

// wii-news-channel's WPAD names, mapped onto the wii-ipl wpad.h
#define WPAD_DPD_MAX_OBJECTS WPAD_MAX_DPD_OBJECTS
#define WPAD_ERR_NONE WPAD_ERR_OK
#define WPAD_FMT_CORE WPAD_FMT_CORE_BTN
#define WPAD_FMT_CORE_ACC WPAD_FMT_CORE_BTN_ACC
#define WPAD_FMT_CORE_ACC_DPD WPAD_FMT_CORE_BTN_ACC_DPD
#define WPAD_FMT_FREESTYLE WPAD_FMT_FS_BTN
#define WPAD_FMT_FREESTYLE_ACC WPAD_FMT_FS_BTN_ACC
#define WPAD_FMT_FREESTYLE_ACC_DPD WPAD_FMT_FS_BTN_ACC_DPD
#define WPAD_FMT_CLASSIC WPAD_FMT_CLASSIC_BTN
#define WPAD_FMT_CLASSIC_ACC WPAD_FMT_CLASSIC_BTN_ACC
#define WPAD_FMT_CLASSIC_ACC_DPD WPAD_FMT_CLASSIC_BTN_ACC_DPD
#define WPAD_SENSOR_BAR_POS_TOP WPAD_SENSOR_BAR_TOP
#define WPAD_STATE_SETUP 3
typedef WPADAccGravityUnit WPADAcc;
#define MTXRowCol(m, r, c) ((m)[r][c])
typedef s32 WPADChannel;
typedef void (*WPADSamplingCallback)(s32 chan);

#define KPAD_BUTTON_HOME WPAD_BUTTON_HOME

#define KPAD_BUTTON_MASK 0x0000ffff
#define KPAD_BUTTON_RPT 0x80000000

#define KPAD_BTN_RPT_TIME_MAX 20000
#define KPAD_BTN_NO_RPT_DELAY 40000

#define KPAD_CMOS_RESO_WX 128
#define KPAD_CMOS_RESO_WY 96
#define KPAD_CMOS_RESO_CX (KPAD_CMOS_RESO_WX / 2)
#define KPAD_CMOS_RESO_CY (KPAD_CMOS_RESO_WY / 2)

#define KPAD_DPD_RESO_WX WPAD_DPD_IMG_RESO_WX
#define KPAD_DPD_RESO_WY WPAD_DPD_IMG_RESO_WY
#define KPAD_DPD_RESO_CX (KPAD_DPD_RESO_WX / 2)
#define KPAD_DPD_RESO_CY (KPAD_DPD_RESO_WY / 2)

#define KPAD_USE_OBJECTS 2


typedef enum KPADPlayMode { KPAD_PLAY_MODE_LOOSE = 0, KPAD_PLAY_MODE_TIGHT } KPADPlayMode;

typedef struct KPADUnifiedWpadStatus {
    union {
        WPADStatus core;
        WPADFSStatus fs;
        WPADCLStatus cl;
    } u;
    u8 fmt;
    u8 padding;
} KPADUnifiedWpadStatus;

typedef struct KPADObject {
    Vec2 center;
    s8 error_fg;
    s8 state_fg;
    s8 _fg_1;
    s8 _fg_2;
} KPADObject;

typedef union KPADTmpStatus {
    KPADStatus k;
    KPADUnifiedWpadStatus w;
} KPADTmpStatus;

typedef void (*KPADControlDpdCallback)(s32 chan, s32 reason);
typedef WPADCallback KPADCallback;
typedef WPADChannel KPADChannel;

// Aug 2007 layout (0x538 bytes), checked against this DOL's KPAD.
#define KPAD_RING_BUFFER_SIZE 16

typedef struct KPADInsideStatus {
    KPADStatus status;

    f32 pos_play_radius;
    f32 pos_sensitivity;

    f32 hori_play_radius;
    f32 hori_sensitivity;

    f32 dist_play_radius;
    f32 dist_sensitivity;

    f32 acc_play_radius;
    f32 acc_sensitivity;
    f32 dist_org;
    Vec2 accXY_nrm_hori;
    Vec2 sec_nrm_hori;
    Vec2 center_org;
    f32 dpd2pos_scale;
    KPADObject kobj_sample[4];
    KPADObject kobj_regular[2];
    s16 valid_objs;
    u8 bufIdx;
    u8 bufCount;
    KPADUnifiedWpadStatus uniRingBuf[KPAD_RING_BUFFER_SIZE]; // at 0x110
    f32 sec_length;
    Vec2 sec_nrm;
    f32 sec_dist;
    f32 trust_sec_length;
    Vec hard_acc;
    Vec2 obj_horizon;
    Vec2 acc_horizon;
    Vec2 ah_circle_pos;
    u16 ah_circle_ct;
    u8 dpd_valid2_ct;
    s8 _fg_4;
    u16 btn_repeat_time;
    u16 btn_repeat_next;
    u16 btn_repeat_delay;
    u16 btn_repeat_pulse;
    u16 btn_cl_repeat_time;
    u16 btn_cl_repeat_next;
    KPADControlDpdCallback dpd_ctrl_callback;
    f32 acc_scale_x;
    f32 acc_scale_y;
    f32 acc_scale_z;
    f32 fs_acc_scale_x;
    f32 fs_acc_scale_y;
    f32 fs_acc_scale_z;
    Vec2 kobj_frame_min;
    Vec2 kobj_frame_max;
    f32 err_dist_speed_1;
    f32 err_dist_speedM_1;
    f32 ah_circle_radius2;
    f32 dist_vv1;
    f32 err_dist_min;
    WPADSamplingCallback appSamplingCallback;
    u8 readLocked;
    u8 resetReq;
    u8 exResetReq;
    u8 dpdIssued;
    u8 dpdEnabled;
    u8 dpdCmd;
    u8 dpdPreCallbackDone;
    u8 dpdPostCallbackDone;
    u8 aimReq;
    u8 aimEnabled;
    u8 fsAccRevise;
    u8 padding;
    KPADPlayMode pos_play_mode;
    KPADPlayMode hori_play_mode;
    KPADPlayMode dist_play_mode;
    KPADPlayMode acc_play_mode;
} KPADInsideStatus; // size 0x538


void KPADReset(void);

extern KPADInsideStatus inside_kpads[];

#ifdef __cplusplus
}
#endif

#endif
