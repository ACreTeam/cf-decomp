#pragma once

// The message controller's answer list (TU d_select.cpp, not decompiled). Class name and layout
// inferred from d_a_npc_nml's talk_c (8003226C / 800324DC); it is dMsg::Comp_c::mSelect (+0x5610).
// RTTI: dSelect::Comp_c (vtable 804F01C0) : dSelect::BComp_c (vtable 804F0234), which has a second
// vtable at +0x48 (dState::mode_c<dSelect::BComp_c>). The controller's ctor/dtor inline Comp_c's own
// ctor/dtor: they only add _9AC/_9B0 around BComp_c's out-of-line ones (8015B824 / 8015B8E4).

#include <types.h>
#include <game/game/d_string.hpp>

// An answer of dMsgSelect_c (fn_8015D4B0). Layout from dMsg::Seq_c::procSelect.
class dMsgSelectItem_c {
public:
    /* 0x000 */ u8 _000[8];
    /* 0x008 */ u16 mMsgCode;          // message code of the answer
    /* 0x00A */ u8 _00A[0x48 - 0xA];
    /* 0x048 */ dString::Word_c mWord; // the answer text
    /* 0x138 */ int mIsCancel;         // set when the message code was 0xFFFF
};

// dSelect::BaseComp_c (RTTI name only, 8049E4B8): also the first base of dAnalogSelect::Comp_c
// (dMsgAnalogSelect_c). The controller drives its current select through it (dMsg::Comp_c::mpCurSelect). [E]
class dMsgSelectBase0_c { // dSelect::BComp_c's first base (+0x00..+0x48; layout only)
public:
    virtual ~dMsgSelectBase0_c() {}
    virtual void vf0C(); // vtable +0x0C: opens the select ([E] 801A4518, Seq_c state 5 -> 6)
    virtual bool vf10(); // vtable +0x10: an answer was chosen ([E] 801A4518; then reset)
    virtual void reset(); // vtable +0x14 (dSelect::Comp_c: 8015D158), called by the controller's reset
    virtual void vf18(); // vtable +0x18
    virtual void vf1C(); // vtable +0x1C

    /* 0x004 */ u8 mCursor; // the chosen answer
    /* 0x005 */ u8 _005;
    /* 0x006 */ u16 mCode;       // [E] the chosen answer's message code (801A4518)
    /* 0x008 */ char mLabel[61]; // [E] the chosen answer's message label (801A4518)
};

class dMsgSelectState_c { // dState::mode_c<dSelect::BComp_c> (+0x48..+0x6C; layout only)
public:
    virtual ~dMsgSelectState_c() {}

    /* 0x04 */ u8 _04[0x24 - 0x4];
};

class dMsgSelectBase_c : public dMsgSelectBase0_c, public dMsgSelectState_c { // dSelect::BComp_c
public:
    dMsgSelectBase_c(); // 8015B824
    virtual ~dMsgSelectBase_c(); // 8015B8E4

    void fn_8015BCE0(); // 8015BCE0: execute (controller execute)
    void fn_8015BD44(); // 8015BD44: draw (controller draw)
    BOOL fn_8015E274(); // 8015E274: create step (controller create)
    BOOL fn_8015E2A0(); // 8015E2A0: delete step (controller doDelete)

    /* 0x06C */ u8 _06C[0x930 - 0x6C];
    /* 0x930 */ u32 mNum;     // number of answers
    /* 0x934 */ u8 _934[4];
    /* 0x938 */ int mCancel;  // answer taken on cancel (-1: none)
    /* 0x93C */ u8 _93C[0x9AC - 0x93C];
}; // size 0x9AC

class dMsgSelect_c : public dMsgSelectBase_c { // dSelect::Comp_c, controller+0x5610 (up to 5 answers)
public:
    dMsgSelect_c() : _9AC(0), _9B0(0) {}
    virtual ~dMsgSelect_c() {}
    // [E] Overrides (vtable 804F01C0); vf18 is the key function, so the vtable stays d_select's (d_msg's
    // .data has no copy).
    virtual void vf18(); // 8015E388
    virtual void vf1C(); // 8015E3D4

    // Sets answer idx to message code (a3..a6: always 0 from d_a_npc_nml).
    void fn_8015BF9C(int idx, u16 code, int a3, int a4, int a5, int a6); // 8015BF9C
    dMsgSelectItem_c *fn_8015D4B0(int idx); // 8015D4B0: answer idx (dMsg::Seq_c::procSelect)

    /* 0x9AC */ u32 _9AC;
    /* 0x9B0 */ u32 _9B0;
}; // size 0x9B4
