#pragma once

// d_reset: the reset manager (src/dol/game/d_reset.cpp, .text 8010693C..80107A28), the NSMBW
// dReset::Manage_c design. It owns the black screen fader used for resets, takes the console's
// reset / power buttons (OS callbacks), the HOME menu's choices and the disc / NAND error states,
// and runs one of six modes per frame (the mode names are the game's own debug strings):
//
//   Normal     waits for a request (reset / power button, an error, a HOME menu result)
//   SoftReset  fades out, lets the scene code change to the title / boot scene (SetSoftResetScene,
//              called by dScene_c), resets the game state when the old scene is deleted
//              (PostDeleteScene) and fades back in once the new scene exists (SetSoftResetFinish)
//   HbmWait    the HOME menu is open: waits for its result
//   DiskWait   the disc is missing / wrong / unreadable
//   FatalError a fatal DVD / NAND error (nothing more to do)
//   SafetyWait fades out, waits until NAND, screenshots, WiiConnect24 and DS download are idle,
//              then shuts the hardware down and leaves the game (mExec: restart, reboot, Wii Menu,
//              data manager or power off)
//
// Class and method names are NSMBW's where the function matches (no RTTI here); others are inferred.

#include <types.h>
#include <revolution/DVD.h>
#include <lib/egg/gfx/eggColor.h>

namespace EGG {
class Heap;
class ColorFader;
} // namespace EGG

namespace dReset {

// 8074E638: the reset fader colour (black); d_home_button reads it too.
extern EGG::Color l_fadeColor;

class Manage_c {
public:
    enum Mode_e {
        MODE_NORMAL,
        MODE_SOFT_RESET,
        MODE_HBM_WAIT,
        MODE_DISK_WAIT,
        MODE_FATAL_ERROR,
        MODE_SAFETY_WAIT,

        MODE_NUM,
        MODE_NONE = MODE_NUM, // mModeInit: no mode change pending
    };

    // What SafetyWait finally does (Exec).
    enum Exec_e {
        EXEC_RESTART,      // OSRestart(0)
        EXEC_REBOOT,       // OSRebootSystem
        EXEC_RETURN_MENU,  // OSReturnToMenu
        EXEC_DATA_MANAGER, // OSReturnToDataManager
        EXEC_SHUTDOWN,     // OSShutdownSystem
        EXEC_NONE,
    };

    // mDiskCheck: the DVDCheckDiskAsync result of DiskCheckReset.
    enum DiskCheck_e {
        DISK_CHECK_BUSY,
        DISK_CHECK_OK,   // the disc is still in: soft reset
        DISK_CHECK_FAIL, // back to the Wii Menu
    };

    // mSoftResetStep: the scene change of a soft reset.
    enum SoftResetStep_e {
        SOFT_RESET_FADE,   // fading out; SetSoftResetScene changes the scene once it is safe
        SOFT_RESET_SCENE,  // the scene change is requested
        SOFT_RESET_FINISH, // the new scene was created (SetSoftResetFinish): fade back in
    };

    // mHbmReset / mHbmPower: closing the HOME menu for a reset / power button press.
    enum HbmClose_e {
        HBM_CLOSE_NONE,
        HBM_CLOSE_REQUEST,
        HBM_CLOSE_DONE,
    };

    typedef void (Manage_c::*ModeFunc)();

    Manage_c(EGG::Heap *heap);                // 80106990
    void Configure();                         // 80106A24: the fader and the OS callbacks
    void Reinit();                            // 80106ACC: back to Normal after a soft reset
    void Calculate();                         // 80106B5C
    void Draw();                              // 80106BA4
    void BootComplete(bool complete);         // 80106BB8
    bool IsSafeToChangeScene() const;             // 80106BC0
    void SetSoftResetFinish();                // 80106C74: dScene_c postCreate
    bool SetSoftResetScene();                 // 80106CE4: dScene_c, changes the scene when it is safe
    void PostDeleteScene();                   // 80106DF0: dScene_c postDelete, resets the game state
    bool IsFadeOutDone() const;               // 80106E94: the fader is opaque (black)
    void SetHbmReturnMenu();                  // 80106ECC: HOME menu "Wii Menu"
    void SetHbmReset();                       // 80106ED8: HOME menu "Reset"
    void SetHbmResetDone();                   // 80106EE4: the HOME menu closed for the reset button
    void SetHbmPowerDone();                   // 80106EF0: the HOME menu closed for the power button
    void RequestErrorReset(int type);         // 80106EFC: d_wifi_err
    void RequestDataManager();                // 80106F0C
    void SetResetDisable();                   // 80106F18: no fade-out while saving
    void SetResetEnable();                    // 80106F24
    void FinalizeExec();                      // 80106F30
    void Exec() const;                        // 80106F68
    void BootCompleteCheck();                 // 80106FB0
    void DiskCheckReset();                    // 80106FD4
    void ModeLog(const char *name) const;     // 80107088: (empty)
    void ModeCalc();                          // 8010708C
    void SetNextMode(int mode);               // 801071EC
    void SetSafetyWait(int exec);             // 801071F4
    void SetSoftReset();                      // 80107230
    void ModeInit_Normal();                   // 80107270
    void ModeProc_Normal();                   // 801072A8
    void ModeInit_SoftReset();                // 80107330
    void ModeProc_SoftReset();                // 80107390
    void ModeInit_HbmWait();                  // 80107400
    void ModeProc_HbmWait();                  // 80107408
    void ModeInit_DiskWait();                 // 80107560
    void ModeProc_DiskWait();                 // 8010756C
    void ModeInit_FatalError();               // 801075E4
    void ModeProc_FatalError();               // 801075F0
    void ModeInit_SafetyWait();               // 801075F4
    void ModeProc_SafetyWait();               // 80107630
    void SetResetCallback();                  // 801076DC
    void SetPowerCallback();                  // 801076E8
    void StartDiskCheck();                    // 801076F4
    void FinalizeGraphics();                  // 801077AC
    void FinalizeCache();                     // 80107834
    void StopControllers();                   // 80107838
    void ResumePadSpeaker();                  // 80107898
    void RequestFadeOut();                    // 8010789C
    void FadeOutCalc();                       // 801078A8
    static bool IsFatalError();                   // 80107948
    static bool IsDiskError();                    // 8010799C

    static void DiskCheckCallback(s32 result, DVDCommandBlock *block); // 80107704
    static void ResetCallback();                                       // 8010774C
    static void PowerCallback();                                       // 8010777C

    static void CreateInstance(EGG::Heap *heap); // 8010693C
    static Manage_c *GetInstance();              // 80106988

    // Inlined by other units.
    int getMode() const { return mMode; }
    // d_net, ...: a reset is under way (the game state is about to be reset, or the game is quitting).
    bool isResetting() const { return mSoftResetPending || mMode == MODE_SAFETY_WAIT; }
    // dScene_c: the soft reset still waits for its scene change.
    bool isSoftResetFade() const { return mMode == MODE_SOFT_RESET && mSoftResetStep == SOFT_RESET_FADE; }
    // d_demo, d_home_button: a soft reset or quitting is under way.
    bool isResetMode() const { return mMode == MODE_SOFT_RESET || mMode == MODE_SAFETY_WAIT; }
    // d_obj_telop: the console is about to power off.
    bool isShutdown() const { return mMode == MODE_SAFETY_WAIT && mExec == EXEC_SHUTDOWN; }

    static Manage_c *sInstance;        // 8074E63C
    static const ModeFunc sModeInit[]; // 80475C30
    static const ModeFunc sModeProc[]; // 80475C78

    /* 0x00 */ EGG::Heap *mpHeap;
    /* 0x04 */ EGG::ColorFader *mpFader;
    /* 0x08 */ int mMode;     // Mode_e
    /* 0x0C */ int mModeInit; // Mode_e: the next mode (MODE_NONE: none)
    /* 0x10 */ int mPrevMode; // Mode_e: DiskWait goes back to it
    /* 0x14 */ int mExec;     // Exec_e
    /* 0x18 */ int mDiskCheck; // DiskCheck_e
    /* 0x1C */ DVDCommandBlock mDiskCheckBlock;
    // Requests of this frame (cleared after the mode proc).
    /* 0x4C */ bool mHbmReset;      // the HOME menu chose "Reset"
    /* 0x4D */ bool mHbmReturnMenu; // the HOME menu chose "Wii Menu"
    /* 0x4E */ bool mResetButton;   // the reset button (OS reset callback)
    /* 0x4F */ bool mPowerButton;   // the power button (OS power callback)
    /* 0x50 */ bool mErrorReset;    // RequestErrorReset
    /* 0x51 */ bool mReturnMenu;
    /* 0x52 */ bool mDataManager;   // RequestDataManager
    /* 0x54 */ int mErrorResetType; // 1: the soft reset goes to profile 0xA2 instead of the title
    /* 0x58 */ int mHbmResetClose;  // HbmClose_e
    /* 0x5C */ int mHbmPowerClose;  // HbmClose_e
    /* 0x60 */ int mSoftResetStep;  // SoftResetStep_e
    /* 0x64 */ bool mBootComplete;  // the boot scene finished: a soft reset goes to the title
    /* 0x65 */ bool mSoftResetPending; // PostDeleteScene resets the game state
    /* 0x66 */ bool mSoftResetRequest; // SetSoftReset -> ModeInit_SoftReset
    /* 0x67 */ bool mFadeOutRequest;   // FadeOutCalc starts the fade-out (and mutes the sound)
    /* 0x68 */ bool mResetDisable;     // holds the fade-out back (saving)
}; // size 0x6C

} // namespace dReset
