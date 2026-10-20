#pragma once

// The menus' dialog box (RTTI dMenuSlctB_c). Names other than the class name are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dMenuSlctB_c {
public:
    enum { BTN_NUM = 3 };

    enum WIN_ANM_e {
        WIN_ANM_IN,
        WIN_ANM_OUT,
        WIN_ANM_DOWN_IN,
        WIN_ANM_DOWN_OUT,
        WIN_ANM_IN_BLACK,
        WIN_ANM_OUT_BLACK,
    };

    // setColor (the dialog window animation).
    enum COLOR_e {
        COLOR_ORANGE,
        COLOR_RED,
    };

    // The text position (fitLines).
    enum POS_e {
        POS_UPPER,
        POS_CENTER,
    };

    enum FLAG_e {
        FLAG_DRAW = 0x1,
        FLAG_CALC = 0x2,
        FLAG_WIN_ANM = 0x4,
        FLAG_PUSH = 0x200,
        FLAG_WAIT = 0x400,
    };

    dMenuSlctB_c();
    virtual ~dMenuSlctB_c();

    bool load();
    bool remove();
    bool execute();
    bool draw();
    void open(int anm, BOOL noSe, BOOL wait);
    void setColor(int color);
    void setLayout(int pos, int lines);
    void fitLines(int pos);
    bool isWinAnmEnd();
    int hitCheck(const mVec2_c *pos);
    void setSelect(int btn, BOOL select);
    void clearSelect();
    void push(int btn);
    bool isPushEnd();
    BOOL checkInput();
    void initBtnAnm();
    void calcAnm();
    void setMsg(u16 msgNo, const char *group);
    void setBtnVisible(int btn, bool visible);
    void setBtn(int btn, u16 msgNo, const char *group, int se);
    bool create();

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::Anm_c mWinAnm;
    /* 0x0C0 */ m2d::Anm_c mPosAnm;
    /* 0x0E4 */ m2d::Anm_c mColorAnm;
    /* 0x108 */ m2d::Anm_c mLinesAnm;
    /* 0x12C */ m2d::AnmEx2_c mBtnAnm[BTN_NUM];
    /* 0x1F8 */ m2d::Anm_c mPushAnm;
    /* 0x21C */ m2d::Anm_c mWaitAnm;
    /* 0x240 */ u32 mFlags;
    /* 0x244 */ int mPushedBtn;
    /* 0x248 */ m2d::stringWordL_c mMsg;
    /* 0x404 */ m2d::stringWord_c mBtnText[BTN_NUM];
    /* 0x6E0 */ int mBtnSe[BTN_NUM];
    /* 0x6EC */ u8 mBtnEnable[BTN_NUM];
};
