#pragma once

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_mtx.hpp>
#include <lib/egg/core/eggHeap.h>

// DOL TU d_balloon_string.cpp (not decompiled; RTTI "dBalloonString_c"): the balloon's string, created in the
// bank's heap. Only what d_hmn_tool_mng uses; virtual names past proc_c's are inferred.
class dBalloonString_c : public m3d::proc_c {
public:
    virtual ~dBalloonString_c();                         // 80066018
    virtual void entry();                                // 80067C0C
    virtual void drawXlu();                              // 80067C24
    virtual const mMtx_c *calc(const mMtx_c *mtx);       // 80067BA0: the hand matrix to use
    virtual const mMtx_c *calcInit(const mMtx_c *mtx);   // 80066358
    virtual const mMtx_c *calcHold(const mMtx_c *mtx);   // 80066670
    virtual const mMtx_c *calcFly(const mMtx_c *mtx);    // 8006706C

    void fn_80067BF0(dItem::Item item);             // release (fly away)
    void fn_800680F4();                             // delete
    static dBalloonString_c *fn_80068108(EGG::Heap *heap); // create

    /* 0x008 */ u8 _008[0x1C0 - 0x8];
    /* 0x1C0 */ s16 mTimer;
    /* 0x1C2 */ s16 mItemId;
    /* 0x1C4 */ u8 mAlpha;
    /* 0x1C5 */ u8 mInit;
    /* 0x1C6 */ u8 mFly; // released (flying away)
}; // size 0x1C8
