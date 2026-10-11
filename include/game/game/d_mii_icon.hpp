#pragma once

// d_mii_icon (DOL TU d_mii_icon.cpp, not decompiled). Only what other TUs use.

#include <types.h>

extern "C" {
void fn_8008B838(int member); // 8008B838: (d_net, a member joined)
void fn_8008B84C();           // 8008B84C: (d_net, the session ended)
void fn_8008B85C(int member); // 8008B85C: (d_net, a member left)
u8 fn_8008B87C();             // 8008B87C: this console has a Mii to send (d_net)
void *fn_8008B88C();          // 8008B88C: this console's Mii data (0x54 bytes, d_net)
BOOL fn_8008B8C8(int member); // 8008B8C8: (d_net) member's Mii is received
void fn_8008B89C(const void *data, int member); // 8008B89C: member's received Mii data (0x54 bytes), NULL = none (d_net)
}
