#pragma once

#include <types.h>

// d_demo (8018E...): demo control. The message controller formerly declared here is dMsg::Comp_c
// (d_msg_comp.hpp).
extern "C" {
BOOL fn_8018ED3C(); // 8018ED3C (d_demo): starts the online save demo (d_net, the save step)
BOOL fn_8018EDE4(); // 8018EDE4 (d_demo): ends it, if running (d_net)
void fn_8018E674(); // 8018E674 (d_demo): (d_reset soft reset)
BOOL fn_8018F438(int type); // 8018F438 (d_demo): a demo of this type is running
BOOL fn_8018F4A8(); // 8018F4A8 (d_demo): a demo of type 2, 3, 8 or 0x10 is running (d_money)
}
