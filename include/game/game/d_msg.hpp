#pragma once

// The message system (TU d_msg.cpp): the receiver dMsg::Rcpt_c is in d_msg_rcpt.hpp, the controller's
// methods are declared in d_demo.hpp (dDemo_c). Only what d_a_npc / d_a_npc_nml use.

#include <types.h>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_rcpt.hpp>

extern "C" {
// Ends the current message step: mode -> controller+0x6C98, next state -> +0x6C64 (same declaration
// as in d_sv_runtime.hpp).
void fn_801A316C(void *demo, int mode); // 801A316C
void fn_801A4E34(void *demo, const char *label); // 801A4E34: message label (also in d_sv_runtime.hpp)
void fn_801A4E44(void *demo, u16 code);          // 801A4E44: message code (also in d_sv_runtime.hpp)
void fn_801A5A00(void *demo);                    // 801A5A00: opens the answer list (dDemo_c::mSelect)
void fn_801A5A4C(void *demo);                    // 801A5A4C: opens the npc select (dDemo_c::mAnalogSelect)
}
