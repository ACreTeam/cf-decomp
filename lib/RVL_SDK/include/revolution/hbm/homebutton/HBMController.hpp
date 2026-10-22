#ifndef HOMEBUTTON_CONTROLLER_H
#define HOMEBUTTON_CONTROLLER_H

#include "revolution/hbm/homebutton/HBMCommon.hpp"
#include "revolution/hbm/homebutton/HBMRemoteSpk.hpp"
#include "revolution/hbm/nw4hbm/snd/SoundArchivePlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundHandle.h"
#include "revolution/mtx.h"
#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSTime.h"
#include "revolution/types.h"
#include "revolution/wpad.h"

struct HBController {
    /* 0x00 */ int chan;
    /* 0x04 */ f32 spVol;
    /* 0x08 */ f32 x;
    /* 0x0C */ f32 y;
    /* 0x10 */ u32 trig;
    /* 0x14 */ u32 hold;
    /* 0x18 */ u32 release;
    /* 0x1C */ bool rumble;
}; // size = 0x20

namespace homebutton {

class Controller {
public:
    Controller(int chan, RemoteSpk* spk);
    ~Controller();

    HBController* getController();
    int getChan() const;
    f32 getSpeakerVol() const;
    RemoteSpk* getRemoteSpk() const { return remotespk; }
    bool isRumbling() { return mHBController.rumble; }
    bool getBatteryFlag() const;
    void setEnableRumble(bool flag) { mRumbleFlag = flag; }

    void setSpeakerVol(f32 vol);
    void setRumble() { mHBController.rumble = true; }
    void clrRumble() { mHBController.rumble = false; }

    s32 getInfoAsync(WPADInfo* info);
    bool isPlayReady() const;
    bool isPlayingSound() const;
    bool isPlayingSoundId(int id) const;

    void setKpad(const HBMKPadData* con, bool updatePos);
    void setInValidPos();
    void clrBatteryFlag();
    void clrKpadButton();

    void connect();
    void disconnect();

    void initSound();
    void updateSound();
#if defined(VERSION_R4QK01)
    void playSound(int id);
    void playSound(nw4hbm::snd::SoundArchivePlayer*, int id) { playSound(id); }
#else
    void playSound(nw4hbm::snd::SoundArchivePlayer* pSoundArchivePlayer, int id);
#endif

    void soundOn();
    void soundOff(int msec);

    void startMotor();
    void stopMotor();

    void initCallback();
    void clearCallback();

private:
    static void wpadConnectCallback(WPADChannel chan, WPADResult result);
    static void wpadExtensionCallback(WPADChannel chan, s32 result);
    static void soundOnCallback(OSAlarm* alm, OSContext* context);
    static void ControllerCallback(WPADChannel chan, WPADResult result);

private:
    /* 0x00 */ HBController mHBController;
#if !defined(VERSION_R4QK01)
    /* 0x20 */ nw4hbm::snd::SoundHandle mSoundHandle;
#endif
    /* 0x24 (K: 0x20) */ RemoteSpk* remotespk;
    /* 0x28 (K: 0x24) */ WPADConnectCallback mOldConnectCallback;
    /* 0x2C (K: 0x28) */ WPADExtensionCallback mOldExtensionCallback;
    /* 0x30 */ OSTime mPlaySoundTime;
    /* 0x38 */ OSTime mStopSoundTime;
    /* 0x40 */ bool mCallbackFlag;
    /* 0x41 */ bool mSoundOffFlag;
    /* 0x42 */ bool mCheckSoundTimeFlag;
    /* 0x43 */ bool mCheckSoundIntervalFlag;
    /* 0x44 */ bool mRumbleFlag;

private:
    static bool sBatteryFlag[WPAD_MAX_CONTROLLERS];
    static OSAlarm sAlarm[WPAD_MAX_CONTROLLERS];
    static OSAlarm sAlarmSoundOff[WPAD_MAX_CONTROLLERS];
    static Controller* sThis[WPAD_MAX_CONTROLLERS];
}; // size = 0x48

} // namespace homebutton

#endif
