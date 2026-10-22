#ifndef HOMEBUTTON_BASE_R4QK01_H
#define HOMEBUTTON_BASE_R4QK01_H

#include "revolution/ax.h"
#include "revolution/axfx.h"
#include "revolution/hbm/homebutton/HBMCommon.hpp"
#include "revolution/hbm/homebutton/HBMAxSound.hpp"
#include "revolution/hbm/homebutton/HBMController.hpp"
#include "revolution/hbm/homebutton/HBMGUIManager.hpp"
#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"
#include "revolution/hbm/nw4hbm/lyt/layout.h"
#include "revolution/wpad.h"

#include <new.h>
#include <revolution/types.h>

struct HBMDataInfoEx;

namespace nw4hbm
{

namespace lyt
{
class ArcResourceAccessor;
class ArcResourceLink;
class Layout;
class MultiArcResourceAccessor;
class Pane;
} // namespace lyt

namespace ut
{
class ResFont;
}

} // namespace nw4hbm

namespace homebutton
{

class HomeButton;
class Controller;
class GroupAnmController;
class RemoteSpk;

class HomeButtonEventHandler : public gui::EventHandler
{
public:
    HomeButtonEventHandler(homebutton::HomeButton* pHomeButton)
        : mpHomeButton(pHomeButton)
    {
    }

    virtual void onEvent(u32 uID, u32 uEvent, void* pData);

    homebutton::HomeButton* getHomeButton() { return mpHomeButton; }

private:
    /* 0x00 (base) */
    HomeButton* mpHomeButton;
}; // size = 0x0C

class HomeButton
{
private:
    typedef enum
    {
        /* 0 */ eSeq_Normal,
        /* 1 */ eSeq_Control,
        /* 2 */ eSeq_Cmn,
    } eSeq;

    class BlackFader
    {
    public:
        BlackFader(int maxFrame)
        {
            init(maxFrame);
            setColor(0, 0, 0);
            flag = true;
        }

        void setColor(u8 r, u8 g, u8 b)
        {
            red_ = r;
            green_ = g;
            blue_ = b;
        }

        int getFrame() const { return frame_; }
        int getMaxFrame() const { return maxFrame_; }
        void start() { state_ = 1; }
        int getFadeColorEnable() const { return flag; }
        void setFadeColorEnable(bool value) { flag = value; }

        bool isDone();
        void init(int maxFrame);
        void calc();
        void draw();

    private:
        static const int scMaxAlpha;

        int frame_;
        int maxFrame_;
        int state_;
        bool flag;
        u8 red_;
        u8 green_;
        u8 blue_;
    }; // size = 0x10

public:
    HomeButton(const HBMDataInfo* dataInfo);
    ~HomeButton();

    int getVolume();
    HBMSelectBtnNum getSelectBtnNum();
    bool isActive() const;
    bool isUpBarActive() const;
    bool isDownBarActive();
    void setAdjustFlag(int flag);
    void setForcusSE();
    void setSimpleSyncAlarm(int type);
    void setSpeakerAlarm(int chan, int msec);
    void setVolume(int vol);
    bool getVibFlag();
    int getPaneNo(const nw4hbm::lyt::Pane*);
    void setVibFlag(bool flag);
    void create();
    void init();
    void calc(const HBMControllerData* pController);
    void draw();
    void update(const HBMControllerData* pController);
    void updateTrigPane();
    void startPointEvent(const nw4hbm::lyt::Pane* pPane, void* pData);
    void startLeftEvent(const nw4hbm::lyt::Pane* pPane);
    void startTrigEvent(const nw4hbm::lyt::Pane* pPane);
    int findAnimator(int pane, int anm);
    int findGroupAnimator(int pane, int anm);
    void callSimpleSyncCallback(s32 result, s32 num);
    void startBlackOut();
    void setBlackOutColor(u8 red, u8 green, u8 blue);
    void setVIBlack(BOOL flag);

    const HBMDataInfo* getHBMDataInfo() { return mpHBInfo; }
    Controller* getController(int chan) { return mpController[chan]; }
    const char* getFuncPaneName(int no) { return scFuncTouchPaneName[no]; }
    const char* getPaneName(int no) { return scBtnName[no]; }
    bool getReassignedFlag() const { return mReassignedFlag; }
    HomeButtonEventHandler* getEventHandler() const { return mpHomeButtonEventHandler; }
    void setEndSimpleSyncFlag(bool flag) { mEndSimpleSyncFlag = flag; }
    void setReassignedFlag(bool flag) { mReassignedFlag = flag; }
    void setSimpleSyncFlag(bool flag) { mSimpleSyncFlag = flag; }

    static void createInstance(const HBMDataInfo* dataInfo);
    static HomeButton* getInstance();
    static void deleteInstance();

private:
    void init_battery(const HBMControllerData* pController);
    void calc_battery(int chan);
    void reset_battery();

    void init_sound();
    void fadeout_sound(f32 gain);

    void init_msg();
    void init_vib();
    void init_volume();

    void set_config();
    void set_text();

    void calc_fadeoutAnm();

    void update_controller(int id);
    void update_posController(int id);

    void reset_btn();
    void reset_control();
    void reset_guiManager(int num);
    void reset_window();

public:
    void play_sound(int id);
    void createSound(const void* soundData, void* work, u32 workSize) { InitAxSound(soundData, work, workSize); }
    void update_sound();

private:
    eSeq mSequence;
    const HBMDataInfo* mpHBInfo;
    const HBMDataInfoEx* mpHBInfoEx;
    int mButtonNum;
    int mAnmNum;
    int mState;
    int mSelectAnmNum;
    int mMsgCount;
    int mPaneCounter[14];
    int mPadDrawTime[WPAD_MAX_CONTROLLERS];
    int mForcusSEWaitTime;
    int mBar0AnmRev;
    int mBar1AnmRev;
    int mBar0AnmRevHold;
    int mBar1AnmRevHold;
    int mGetPadInfoTime;
    bool mControllerFlag[WPAD_MAX_CONTROLLERS];
    int mVolumeNum;
    int mConnectNum;
    bool mVibFlag;
    bool mControlFlag;
    bool mLetterFlag;
    bool mAdjustFlag;
    bool mReassignedFlag;
    bool mSimpleSyncFlag;
    bool mEndSimpleSyncFlag;
    bool mForthConnectFlag;
    bool mInitFlag;
    bool mForceSttInitProcFlag;
    bool mForceSttFadeInProcFlag;
    bool mEndInitSoundFlag;
    bool mForceStopSyncFlag;
    bool mForceEndMsgAnmFlag;
    bool mStartBlackOutFlag;
    int mSoundRetryCnt;
    int mDialogFlag[4];
    char* mpLayoutName;
    char* mpAnmName;
    HBMSelectBtnNum mSelectBtnNum;
    wchar_t* mpText[10][6];
    WPADInfo mWpadInfo[WPAD_MAX_CONTROLLERS];
    WPADSyncDeviceCallback mSimpleSyncCallback;
    f32 mOnPaneVibFrame[4];
    f32 mOnPaneVibWaitFrame[4];
    int mWaitStopMotorCount;
    int mDisConnectCount;
    nw4hbm::lyt::Layout* mpLayout;
    nw4hbm::lyt::Layout* mpCursorLayout[WPAD_MAX_CONTROLLERS];
    nw4hbm::lyt::ArcResourceAccessor* mpResAccessor;
    gui::PaneManager* mpPaneManager;
    HomeButtonEventHandler* mpHomeButtonEventHandler;
    nw4hbm::lyt::DrawInfo mDrawInfo;
    Controller* mpController[WPAD_MAX_CONTROLLERS];
    RemoteSpk* mpRemoteSpk;
    GroupAnmController* mpAnmController[12];
    GroupAnmController* mpGroupAnmController[74];
    GroupAnmController* mpPairGroupAnmController[15];
    BlackFader mFader;
    OSAlarm mAlarm[WPAD_MAX_CONTROLLERS];
    OSAlarm mSpeakerAlarm[WPAD_MAX_CONTROLLERS];
    OSAlarm mSimpleSyncAlarm;
    int iVISetBlackFlag;
    int iReConnectTime;
    int iReConnectTime2;
    u16 mAppVolume[3];
    AXFXAllocHook mAxFxAlloc;
    AXFXFreeHook mAxFxFree;
    AXFX_REVERBHI mAxFxReverb;
    AXAuxCallback mAuxCallback;
    void* mpAuxContext;
    f32 mFadeOutSeTime;

    // static members
private:
    static const int scReConnectTime;
    static const int scReConnectTime2;
    static const int scPadDrawWaitTime;
    static const int scGetPadInfoTime;
    static const int scForcusSEWaitTime;
    static const f32 scOnPaneVibTime;
    static const f32 scOnPaneVibWaitTime;
    static const int scWaitStopMotorTime;
    static const int scWaitDisConnectTime;
    static const int scSoundHeapSize_but2;
    static const int scSoundHeapSize_but3;
    static const int scSoundThreadPrio;
    static const int scDvdThreadPrio;

    static HomeButton* spHomeButtonObj;
    static OSMutex sMutex;

    static const char* scCursorLytName[WPAD_MAX_CONTROLLERS];
    static const char* scCursorPaneName;
    static const char* scCursorRotPaneName;
    static const char* scCursorSRotPaneName;

    static const char* scBtnName[4];
    static const char* scTxtName[4];
    static const char* scGrName[8];
    static const char* scAnimName[3];
    static const char* scPairGroupAnimName[15];
    static const char* scPairGroupName[15];
    static const char* scGroupAnimName[22];
    static const char* scGroupName[35];
    static const char* scFuncPaneName[5];
    static const char* scFuncTouchPaneName[10];
    static const char* scFuncTextPaneName[3];
    static const char* scBatteryPaneName[WPAD_MAX_CONTROLLERS][4];
}; // size = 0x7A0

} // namespace homebutton

#endif
