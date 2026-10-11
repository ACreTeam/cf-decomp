#pragma once

// The message controller's two-way npc select (TU d_analog_select.cpp, not decompiled). Class name and
// layout inferred from d_a_npc_nml's talk_c (800323F0 / 800324DC); it is dMsg::Comp_c::mAnalogSelect (+0x5FC4).
// Like dMsgSelect_c it has a second vtable at +0x48 (vtable 8049E3D0).

#include <types.h>
#include <game/game/d_select.hpp>

namespace dScript {
class Word_c;
}

// RTTI: dAnalogSelect::Comp_c : dSelect::BaseComp_c (dMsgSelectBase0_c; its virtuals are overridden here).
class dMsgAnalogSelect_c : public dMsgSelectBase0_c { // controller+0x5FC4: the two-way npc select ("sys_SELECT/SYS_SelectNPC")
public:
    dMsgAnalogSelect_c(); // 8000C188
    virtual ~dMsgAnalogSelect_c(); // 8000C228

    BOOL fn_8000C560(); // 8000C560: create step (controller create)
    BOOL fn_8000C5BC(); // 8000C5BC: delete step (controller doDelete)
    void fn_8000C658(); // 8000C658: execute (controller execute)
    void fn_8000C6AC(); // 8000C6AC: draw (controller draw)

    // Shows label (a message label) / code on side idx (0, 1).
    void fn_8000D304(int idx, dScript::Word_c *text); // 8000D304: text of side idx (dMsg::Seq_c 7:8)
    void fn_8000D340(int idx, u16 code, const char *label); // 8000D340
    void fn_8000D3A0(int idx, u16 code, int a3); // 8000D3A0: message code idx of 8 (dMsg::Seq_c 7:8)

    /* 0x048 */ u8 _048[0xC9C - 0x48];
}; // size 0xC9C (controller layout)
