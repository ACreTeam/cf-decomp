#pragma once

// The HOME button menu (TU d_home_button.cpp, .text 8017D89C..8017E7B0): loads the HomeButton2 files, opens
// the SDK's menu on HOME and shows the "not now" icon while it can't. Names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <lib/egg/core/eggController.h>
#include <nw4r/g3d.h>
#include <revolution/HBM.h>

namespace EGG {
class Heap;
}

class dHomeButton_c {
public:
    enum State_e {
        STATE_LOAD,  // the files are loading (d_s_boot_static calls load)
        STATE_READY, // HOME opens the menu
        STATE_OPEN,  // the HOME menu is shown
    };

    // dReset closes the menu for the console buttons.
    enum Close_e {
        CLOSE_NONE,
        CLOSE_RESET, // the reset button
        CLOSE_POWER, // the power button
    };

    // mFlags: why HOME can't open the menu now.
    enum Flag_e {
        FLAG_DISC_ERROR = 0x1,    // the drive failed or NAND is corrupt
        FLAG_CAUTION = 0x2,       // the disc error window is shown
        FLAG_RESETTING = 0x4,     // dReset soft resets or shuts down
        FLAG_BOOT = 0x8,          // until the boot scene is done
        FLAG_FADER = 0x10,        // the scene fader isn't hidden
        FLAG_NAND_WRITING = 0x40, // the save file is written
        FLAG_NETWORK = 0x80,      // the net manager runs or its error window is shown
        FLAG_DS_DOWNLOAD = 0x100, // the DS download is transferring
        FLAG_SAVING = 0x200,      // set by d_sv_mgr while saving

        // No icon either: HOME does nothing.
        FLAG_NO_ICON = FLAG_DISC_ERROR | FLAG_RESETTING | FLAG_BOOT,
    };

    dHomeButton_c(EGG::Heap *heap); // 8017D8F0

    static void create(EGG::Heap *heap);   // 8017D89C
    static dHomeButton_c *getInstance();   // 8017D8E8

    BOOL load();               // 8017DA00
    void execute();            // 8017DBD4
    void draw();               // 8017DC8C: the menu
    void setScreen();          // 8017DCBC
    void drawIcon();           // 8017DD20
    void initDataInfo();       // 8017DE40
    void initControllerData(); // 8017DF20
    void setLayoutName();      // 8017DF30
    void setSpeakerSeName();   // 8017E008
    void setMsgName();         // 8017E018
    void setConfigName();      // 8017E028
    void setIconName();        // 8017E038
    void setSoundName();       // 8017E048
    void calcController();     // 8017E058
    HBMKPadData *getKPadData(u32 chan) { return &mControllerData.wiiCon[chan]; }
    bool isHomeTrigger();      // 8017E250
    void open();               // 8017E26C
    void calcMenu();           // 8017E2B0
    void startIcon();          // 8017E374
    void calcIcon();           // 8017E3B8
    void resetIcon();          // 8017E500
    static void stopRumble();  // 8017E514
    BOOL requestClose(Close_e type); // 8017E570: FALSE unless the menu is open
    void updateFlags();        // 8017E5C8

    bool canShowIcon() const { return (mFlags & FLAG_NO_ICON) == 0; }

    void setFlag(u32 flag, BOOL on) {
        if (on != FALSE) {
            mFlags |= flag;
        } else {
            mFlags &= ~flag;
        }
    }

    /* 0x000 */ EGG::Heap *mHeap;
    /* 0x004 */ dDvd::loader_c mLayoutLoader;
    /* 0x018 */ dDvd::loader_c mSpeakerSeLoader;
    /* 0x02C */ dDvd::loader_c mMsgLoader;
    /* 0x040 */ dDvd::loader_c mConfigLoader;
    /* 0x054 */ dDvd::loader_c mIconLoader;
    /* 0x068 */ dDvd::loader_c mSoundLoader;
    /* 0x07C */ nw4r::g3d::ResTex mIconTex;
    /* 0x080 */ void *mSoundData;
    /* 0x084 */ u8 *mSoundBuf;
    /* 0x088 */ const char *mLayoutName;
    /* 0x08C */ const char *mSpeakerSeName;
    /* 0x090 */ const char *mMsgName;
    /* 0x094 */ const char *mConfigName;
    /* 0x098 */ const char *mIconName;
    /* 0x09C */ const char *mSoundName;
    /* 0x0A0 */ HBMDataInfo mDataInfo;
    /* 0x0E0 */ HBMControllerData mControllerData;
    /* 0x120 */ EGG::CoreStatus mCoreStatus;
    /* 0x1A4 */ int mState;     // State_e
    /* 0x1A8 */ int mCloseType; // Close_e
    /* 0x1AC */ u32 mFlags;     // Flag_e
    /* 0x1B0 */ u32 mIconTick;  // OSGetTick() when the icon was started (0: hidden)
    /* 0x1B4 */ f32 mIconAlpha;
}; // size 0x1B8
