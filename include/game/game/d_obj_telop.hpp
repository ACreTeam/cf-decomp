#pragma once

// The scrolling message banner (RTTI dObjTelop_c). Names other than the class name are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

// dObjTelop_c::request messages (sys_2D/SYS_2D_menu texts).
enum dTelopMsg_e {
    TELOP_NONE,
    TELOP_TIMER_START,         // "Start timer!"
    TELOP_TIMER_STOP,          // "Stop timer!"
    TELOP_TIMER_END,           // "Time's up!"
    TELOP_HIDE_AND_SEEK_START, // "Start hide-and-seek!"
    TELOP_HIDE_AND_SEEK_END,   // "Time's up!"
    TELOP_NET_ARRIVE,          // "Looks like someone's here!"
    TELOP_NET_LEAVE,           // "Someone's leaving."
    TELOP_NET_SAVE,            // "Saving now..."
    TELOP_NET_CLOSE,           // "Everyone's going home."
    TELOP_NET_CANCEL,          // "Canceled"
    TELOP_NET_STOP_TALK,       // "Please halt conversations."
    TELOP_NET_CLOSE_WINDOW,    // "Close the window."
    TELOP_NO_REMOTE,           // the Wii Remote was disconnected
    TELOP_LOW_BATTERY,         // the Wii Remote batteries are low
    TELOP_EXTENSION,           // an unsupported extension controller is connected
    TELOP_MIC_CHAT_START,      // "Now starting Mic Chat."
    TELOP_MIC_CHAT_UNAVAILABLE, // "You can't use Mic Chat right now."
    TELOP_MSG_NUM,
};

class dObjTelop_c : public fBase_c {
public:
    enum ANM_e {
        ANM_IN,
        ANM_OUT,
    };

    enum FLAG_e {
        FLAG_SHOW = 0x1,
        FLAG_CALC = 0x2,
    };

    enum WPAD_STATE_e {
        WPAD_STATE_WAIT,
        WPAD_STATE_CHECK,
        WPAD_STATE_INFO,
    };

    enum NET_CHECK_e {
        NET_CHECK_NONE,
        NET_CHECK_HOST,
        NET_CHECK_ALL,
    };

    // The requested message (l_req).
    class Req_c {
    public:
        void clear();
        void calc();
        void setTimer();
        static int getPriority(int msg);
        void request(int msg);
        void lock();
        void unlock();
        BOOL isLocked();
        void setNetCheckHost();
        void setNetCheckAll();
        void checkNet();

        /* 0x0 */ int mTimer;
        /* 0x4 */ int mMsg;
        /* 0x8 */ u8 mLock;
        /* 0x9 */ u8 mNetCheck;
    };

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~dObjTelop_c();

    void stateMain();
    void setMsg();
    void setPos();
    void initAnm();
    void playAnm();
    void wpadWait();
    void wpadCheck();
    void wpadInfo();
    void calcWpad();

    static void request(int msg);
    static void lock();
    static void unlock();
    static BOOL isLocked();
    static void setNetCheckHost();
    static void setNetCheckAll();

    /* 0x064 */ m2d::ResAccLoader_c mRes;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mAnm;
    /* 0x20C */ f32 mScrollX;
    /* 0x210 */ f32 mTextWidth;
    /* 0x214 */ int mMsg;
    /* 0x218 */ int mWpadTimer;
    /* 0x21C */ u16 mFlags;
    /* 0x21E */ u8 mState;
    /* 0x21F */ u8 mNoRemoteCnt;
    /* 0x220 */ u8 mExtensionCnt;
    /* 0x221 */ u8 mWpadState;
    /* 0x222 */ u8 mLowBatteryCnt;
}; // size 0x224
