#ifndef RVL_SDK_PUBLIC_HBM_H
#define RVL_SDK_PUBLIC_HBM_H
#include <types.h>

#include <revolution/kpad.h>
#include <revolution/mem.h>
#include <revolution/mtx.h>
#include <revolution/wpad.h>
#ifdef __cplusplus
extern "C" {
#endif

// The HOME button menu library (lib/RVL_SDK/hbm/homebutton). Only what the game uses.

typedef enum {
    HBM_SELECT_NULL = -1,
    HBM_SELECT_HOMEBTN, // closed with the HOME button / "Close"
    HBM_SELECT_BTN1,    // "Wii Menu"
    HBM_SELECT_BTN2,    // "Reset"
    HBM_SELECT_BTN3,
    HBM_SELECT_BTN4,
    HBM_SELECT_MAX
} HBMSelectBtnNum;

typedef int (*HBMSoundCallback)(int evt, int num);

typedef struct HBMDataInfo {
    void *layoutBuf;                 // at 0x0
    void *spkSeBuf;                  // at 0x4
    void *msgBuf;                    // at 0x8
    void *configBuf;                 // at 0xC
    void *mem;                       // at 0x10
    HBMSoundCallback sound_callback; // at 0x14
    int backFlag;                    // at 0x18
    int region;                      // at 0x1C
    int cursor;                      // at 0x20
    int messageFlag;                 // at 0x24
    u32 configBufSize;               // at 0x28
    u32 memSize;                     // at 0x2C
    f32 frameDelta;                  // at 0x30
    Vec2 adjust;                     // at 0x34
    MEMAllocator *pAllocator;        // at 0x3C
} HBMDataInfo;

typedef struct HBMKPadData {
    KPADStatus *kpad; // at 0x0
    Vec2 pos;         // at 0x4
    u32 use_devtype;  // at 0xC
} HBMKPadData;

typedef struct HBMControllerData {
    HBMKPadData wiiCon[WPAD_MAX_CONTROLLERS]; // at 0x0
} HBMControllerData;

void HBMCreate(const HBMDataInfo *pHBInfo);
void HBMInit(void);
HBMSelectBtnNum HBMCalc(const HBMControllerData *pController);
void HBMDraw(void);
void HBMSetAdjustFlag(BOOL flag);
void HBMStartBlackOut(void);
void HBMSetBlackOutColor(u8 red, u8 green, u8 blue);
void HBMSetVIBlack(BOOL value);
void HBMCreateSound(void *soundData, void *memBuf, u32 memSize);
void HBMUpdateSound(void);

#ifdef __cplusplus
}
#endif
#endif
