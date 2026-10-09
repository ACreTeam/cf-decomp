#pragma once

// The message system (TU d_msg.cpp): the receiver dMsg::Rcpt_c is in d_msg_rcpt.hpp, the controller's
// methods are declared in d_demo.hpp (dDemo_c). Only what d_a_npc uses.

#include <types.h>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_rcpt.hpp>

extern "C" {
// Ends the current message step: mode -> controller+0x6C98, next state -> +0x6C64 (same declaration
// as in d_sv_runtime.hpp).
void fn_801A316C(void *demo, int mode); // 801A316C
}
