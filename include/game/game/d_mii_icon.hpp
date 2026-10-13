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
BOOL fn_8008B8DC();           // 8008B8DC: its worker thread is suspended (idle) (d_reset_save)
}

// Names inferred from d_travel_data: the 0x4C-byte travel copy of the Mii.
namespace dMiiIcon {
void *getTravelMii();             // 8008B8E8: NULL when no travel Mii is available
void setTravelMii(const void *data); // 8008B8F8: NULL clears the travel Mii
}
