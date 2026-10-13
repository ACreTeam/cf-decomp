#pragma once

// The menus' small dialog box (RTTI dMenuSlctC_c). Names other than the class name are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuSlctC_c {
public:
    enum { BTN_NUM = 2 };

    enum WIN_ANM_e {
        WIN_ANM_IN,
        WIN_ANM_OUT,
    };

    enum FLAG_e {
        FLAG_DRAW = 0x1,
        FLAG_CALC = 0x2,
        FLAG_WIN_ANM = 0x4,
        FLAG_PUSH = 0x200,
    };

    dMenuSlctC_c();
    virtual ~dMenuSlctC_c();

    bool load();
    bool remove();
    bool execute();
    bool draw();
    void open(int anm);
    bool isWinAnmEnd();
    int hitCheck(const mVec2_c *pos);
    void setSelect(int btn, BOOL select);
    void clearSelect();
    void push(int btn);
    bool isPushEnd();
    BOOL checkInput();
    void initBtnAnm();
    void calcAnm();
    void setVisible(BOOL visible);
    void setMsg(u16 msgNo);
    void setBtnVisible(int btn, bool visible);
    void setBtn(int btn, u16 msgNo, int se);
    bool create();

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::Anm_c mWinAnm;
    /* 0x0C0 */ m2d::AnmEx_c mBtnAnm[BTN_NUM];
    /* 0x138 */ m2d::Anm_c mPushAnm;
    /* 0x15C */ u32 mFlags;
    /* 0x160 */ int mPushedBtn;
    /* 0x164 */ m2d::stringWordL_c mMsg;
    /* 0x320 */ m2d::stringWord_c mBtnText[BTN_NUM];
    /* 0x508 */ int mBtnSe[BTN_NUM];
    /* 0x510 */ u8 mBtnEnable[BTN_NUM];
};
