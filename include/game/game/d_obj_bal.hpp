#pragma once

// The outdoor item name balloon (RTTI dObjBal_c). Names other than the class name are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_m2d.hpp>

class dObjBal_c : public fBase_c {
public:
    enum STATE_e {
        STATE_WAIT,
        STATE_IN,
        STATE_SHOW,
        STATE_OUT,
    };

    enum ANM_e {
        ANM_IN,
        ANM_OUT,
    };

    enum FLAG_e {
        FLAG_SHOW = 0x1,
        FLAG_LOWER = 0x2,
    };

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~dObjBal_c();

    void calcPos();
    void initAnm();
    void startAnm(int anm);
    void stateWait();
    void stateIn();
    void stateShow();
    void stateOut();
    void setWord(int bln);
    void setWidth(int bln);
    void setPos(int bln);

    static void show(const dItem::Item &item);
    static void hide();

    /* 0x064 */ m2d::ResAccLoader_c mRes;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mAnm;
    /* 0x20C */ u32 mFlags;
    /* 0x210 */ dItem::Item mItem;
    /* 0x212 */ dItem::Item mReqItem;
    /* 0x214 */ m2d::itemWord_c mText[2];
    /* 0x2AC */ int mState;
    /* 0x2B0 */ f32 mBaseWidth;
    /* 0x2B4 */ f32 mBaseX;
}; // size 0x2B8
