#pragma once

// The pointer cursor process (RTTI dObjCursor_c). Names other than the class name are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dObjCursor_c : public fBase_c {
public:
    enum ANM_e {
        ANM_ON,
        ANM_OFF,
        ANM_HOLD,
        ANM_RELEASE,
    };

    enum FLAG_e {
        FLAG_VISIBLE = 0x1,
        FLAG_HOLD = 0x2,
        FLAG_REQ_HOLD = 0x4,
        FLAG_REQ_RELEASE = 0x8,
        FLAG_SYNC_POS = 0x20,
        FLAG_RIPPLE = 0x40,
        FLAG_PRESS = 0x80,
        FLAG_PRESS_SE = 0x100,
    };

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~dObjCursor_c();

    void initAnm();
    void playAnm();
    void calcState();
    void calcPos();
    void calcAlpha();
    void calcRotate();

    static void hold();
    static void show();
    static void hide();
    static mVec2_c getPos();
    // Tests the pointer alpha (fades out while the pointer is off screen), not FLAG_VISIBLE: hide() does not clear it.
    static BOOL isVisible();

    /* 0x064 */ m2d::ResAccLoader_c mRes;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mAnm;
    /* 0x20C */ m2d::SimpleEx_c mRippleLyt;
    /* 0x2A4 */ m2d::Anm_c mRippleAnm;
    /* 0x2C8 */ nw4r::lyt::Pane *mpHand0;
    /* 0x2CC */ nw4r::lyt::Pane *mpHand1;
    /* 0x2D0 */ nw4r::lyt::Pane *mpShadow;
    /* 0x2D4 */ nw4r::lyt::Pane *mpHand;
}; // size 0x2D8
