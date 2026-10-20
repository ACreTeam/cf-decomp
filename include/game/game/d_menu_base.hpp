#pragma once

// The menu base class (RTTI dMenuBase_c). Names other than the class name are inferred.

#include <types.h>
#include <game/cLib/c_line.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuBase_c;

class dMenuNd_c : public cLineNd_c {
public:
    /* 0x08 */ dMenuBase_c *mpOwner;
};

class dMenuBase_c : public fBase_c {
public:
    enum STATE_e {
        STATE_ANM,
        STATE_MAIN,
        STATE_2,
        STATE_3,
        STATE_CLOSE,
        STATE_NUM
    };

    enum ANM_e {
        ANM_WIN_IN,
        ANM_WIN_OUT,
        ANM_WIN_UP,
        ANM_WIN_DOWN,
        ANM_UPPER_WIN_IN,
        ANM_UPPER_WIN_OUT,
        ANM_NUM
    };

    dMenuBase_c();
    virtual ~dMenuBase_c();

    static const char *getAnmName(int anm);
    void setRepeat(int delay, int interval, int count);

    virtual int preCreate();
    virtual void postCreate(MAIN_STATE_e state);
    virtual int preDelete();
    virtual void postDelete(MAIN_STATE_e state);
    virtual int preExecute();
    virtual void postExecute(MAIN_STATE_e state);
    virtual int execute();

    void setState(u8 state);
    bool isState(u8 state);
    void changeNextState();
    void setNextState(u8 state);

    void startAnm(m2d::Simple_c *layout, const char *paneName, int anm, u8 nextState, BOOL playSe);

    virtual int stateAnm();
    virtual int stateMain() { return SUCCEEDED; }
    virtual int state2() { return SUCCEEDED; }
    virtual int state3() { return SUCCEEDED; }
    virtual int stateClose();

    bool calcAnm();

    void onFlag(int bit);
    void offFlag(int bit);
    bool isFlag(int bit);

    virtual void setSubState(u8 state);
    void setNextSubState(u8 state);
    void changeNextSubState();

    void setPostFlags(u32 flags);
    u32 getPostFlags();

    u8 getMenuNo() const { return mMenuNo; }

    static void playSeCursor();
    static void playSeDecide();
    static void playSeDecide2();
    static void playSeCancel();
    static void playSeCancel2();
    static void playSeSlideClick();
    static void playSeAPush();
    static void playSeSubMenuOpen();
    static void playSeSubMenuClose();
    static void playSeSubMenuPopUp();
    static void playSeSubMenuPush();
    static void playSePageTurn();
    static void playSePageTurnNo(int page);
    static void playSeItemLift();
    static void playSeItemDrop();
    static void playSeSelectWindowOpen();
    static void playSeListChoice();
    static void playSeCanNotLeave();
    static void playSeCanNot();

protected:
    /* 0x64 */ u8 mState;
    /* 0x65 */ u8 mNextState;
    /* 0x66 */ u8 mMenuNo;
    /* 0x67 */ u8 mNextSubState;
    /* 0x68 */ u8 mSubState;
    /* 0x6C */ dMenuNd_c mNode;
    /* 0x78 */ m2d::Anm_c mAnm;
    /* 0x9C */ m2d::Simple_c *mpAnmLayout;
    /* 0xA0 */ char mAnmPaneName[20];
    /* 0xB4 */ u32 mFlags;
    /* 0xB8 */ int mRepeatCnt[4];
    /* 0xC8 */ int mRepeatDelay;
    /* 0xCC */ int mRepeatInterval;
    /* 0xD0 */ int mRepeatCount;

    static u32 s_postFlags;
};

class dMenuMove_c {
public:
    static int getHoldFrames();
    void calcFollow();
    void startFollow();
    mVec2_c *getPos();
    void setOfs(const mVec2_c &ofs);
    bool isMoveEnd();
    bool calcMove();
    void startMove(const mVec2_c &target);
    void stop();
    void setPos(const mVec2_c &pos);

    /* 0x00 */ mVec2_c mOfs;
    /* 0x08 */ int mFollowCnt;
    /* 0x0C */ mVec2_c mPos;
    /* 0x14 */ mVec2_c mTarget;
    /* 0x1C */ int mMoveCnt;
};

