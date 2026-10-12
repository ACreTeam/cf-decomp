#pragma once

// The speech balloon layout (DOL TU d_balloon.cpp, .text 80065524..80066018, not decompiled; RTTI "dBalloon_c").
// "Layout/bal.arc", "balloon_00.brlyt". Only what d_obj_tutorial uses; all names but the class name are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

class dBalloon_c {
public:
    // States (run through the ptmf table 8046CDC0); STATE_NONE means no state change is requested.
    enum STATE_e {
        STATE_HIDE,
        STATE_OPEN,
        STATE_SHOW,
        STATE_CLOSE,
        STATE_NONE,
    };

    enum FLAG_e {
        FLAG_1 = 1, // tested by the open/close states
    };

    dBalloon_c();          // 80065524
    virtual ~dBalloon_c(); // 8006559C

    BOOL loadArc(); // 8006562C: "Layout/bal.arc"
    BOOL build();   // 8006563C: the layout, its animations and text box; state STATE_HIDE
    BOOL remove();  // 80065718: frees the archive
    int execute();  // 80065748: applies the requested state, runs the current one; returns 1
    int draw();     // 80065780: draws unless hidden; returns 1
    void setPos(mVec2_c pos); // 800657B4: the screen position (adjusted for the aspect ratio and the tail)
    void setWord(const dScript::Word_c *word); // 80065A04: the text, resized to fit
    void setColor(int color); // 80065EFC: "balloon_00_blnBlue.brlan" / "balloon_00_blnYlow.brlan"
    void setTail(int tail);   // 80065F5C: shows tail 0 ("blnTail_00") or 1 ("blnTail_01")

    int getState() const { return mState; }
    void setNextState(int state) { mNextState = state; }
    void setPriority(u8 priority) { mLyt.mPriority = priority; }
    void onFlag(u32 flag) { mFlags |= flag; }

    /* 0x004 */ int mState;
    /* 0x008 */ int mNextState;
    /* 0x00C */ m2d::ResAccLoader_c mResAcc;
    /* 0x0E0 */ m2d::SimpleEx_c mLyt;
    /* 0x178 */ m2d::Anm_c mAnm;
    /* 0x19C */ m2d::Anm_c mColorAnm;
    /* 0x1C0 */ nw4r::lyt::Pane *mpTail;
    /* 0x1C4 */ nw4r::lyt::Pane *mpTailSdw;
    /* 0x1C8 */ m2d::stringWord_c mWord;
    /* 0x2BC */ f32 _2BC;
    /* 0x2C0 */ f32 mWidth;
    /* 0x2C4 */ u32 mFlags;
    /* 0x2C8 */ f32 _2C8;
    /* 0x2CC */ int mTail;
}; // size 0x2D0
