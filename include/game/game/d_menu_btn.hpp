#pragma once

// The menus' button bar (RTTI dMenuBtn_c). Names other than the class name are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuBtn_c {
public:
    enum { BTN_NUM = 3 };

    enum BTN_ANM_e {
        BTN_ANM_NO_PUSH,
        BTN_ANM_CAN_PUSH,
        BTN_ANM_PUSH,
    };

    enum FLAG_e {
        FLAG_DRAW = 0x1,
        FLAG_CALC = 0x2,
        FLAG_HIDDEN = 0x4,
        FLAG_DISABLED = 0x20,
        FLAG_PUSH = 0x1000,
        FLAG_ANM = 0x2000,
    };

    dMenuBtn_c();
    virtual ~dMenuBtn_c();

    bool load();
    bool create();
    bool remove();
    bool execute();
    bool draw();

    void setPos(m2d::SimpleEx_c *layout, const char *paneName);
    int hitCheck(const mVec2_c *pos);
    int checkPad(BOOL useB, BOOL usePlus);
    void setSelect(int btn, BOOL select, BOOL playSe);
    void clearSelect();
    BOOL push(int btn, BOOL noSe);
    bool isPushing();
    BOOL startAnm(int anm);
    bool isAnm();
    void setText(int btn, u16 msgNo, u8 isCancel);
    void setVisible(int btn, bool visible);
    void setDisable(int btn, BOOL disable);
    void setPriority(u8 priority);
    bool isEnable(int btn);
    void calcAnm();

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::AnmEx2_c mBtnAnm[BTN_NUM];
    /* 0x168 */ m2d::AnmEx2_c mInOutAnm[BTN_NUM];
    /* 0x234 */ m2d::stringWord_c mText[BTN_NUM];
    /* 0x510 */ m2d::Anm_c mAnm;
    /* 0x534 */ int mAnmNo;
    /* 0x538 */ u32 mFlags;
    /* 0x53C */ int mPushedBtn;
    /* 0x540 */ u8 mIsCancel[BTN_NUM];
};
