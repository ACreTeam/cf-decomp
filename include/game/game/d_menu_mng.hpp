#pragma once

// The menu manager (TU d_menu_mng.cpp, .text from 8019A264, not decompiled): menus opened from a
// message (item select, letters, text input ...) and their results. Names provisional.

#include <types.h>

class dMail_c;
class dMenuNd_c;
class dTime_c;
namespace m2d {
class ResAccIf_c;
}
namespace dItem {
class Item;
}

extern "C" {
void fn_8019B864(); // 8019B864: forgets the cached pocket selection (8074EA9C / 8074EAA0 = -1); d_private_data setPocket
BOOL fn_8019A5E4(); // 8019A5E4: 8074EA6C == 1 and 8074EA70 == 0 (d_money: the money may be shown)
void fn_8019A528(dMenuNd_c *node);                        // 8019A528: adds a menu (dMenuBase_c postCreate)
void fn_8019A56C(dMenuNd_c *node);                        // 8019A56C: removes a menu, releases its modules (postDelete)
BOOL fn_8019A608(u8 menu, BOOL a);                        // 8019A608: opens a menu (d_demo TYPE_MENU)
m2d::ResAccIf_c *fn_8019A6FC();                           // 8019A6FC: the menu window layout resources (sb_win)
m2d::ResAccIf_c *fn_8019A718();                           // 8019A718: the menu button layout resources (cmn_btn)
void fn_8019A7B4();                                       // 8019A7B4: fn_8019A7D4(0x10) (d_demo TYPE_MENU end)
void fn_8019B7D4(u8 menu);                                // 8019B7D4: starts the menu's BGM volume state
void fn_8019B824();                                       // 8019B824: ends the menu's BGM volume state
BOOL fn_8019A76C();                                       // 8019A76C: the menu is finished/closed
BOOL fn_8019A804(u16 item, u16 msgParam, u8 menuNo);      // 8019A804: item select menu
BOOL fn_8019A85C(u16 item, u16 msgParam);                 // 8019A85C: item select menu 0x2A
u16 fn_8019A8B4(BOOL (*filter)(void *pockets, int flag)); // 8019A8B4: pocket mask (15 slots) from a filter
u8 fn_8019A944();                                         // 8019A944: selected slot (8074EA82)
dItem::Item *fn_8019AAE4();                               // 8019AAE4: the selected items (80624240)
int fn_8019AAF0();                                        // 8019AAF0: number of selected slots
void fn_8019AB74();                                       // 8019AB74: removes the selected items from the pockets
u16 fn_8019AD30();                                        // 8019AD30 (8074C104)
void *fn_8019AD58();                                      // 8019AD58: &80624278
BOOL fn_8019ADD4();                                       // 8019ADD4
BOOL fn_8019AE00();                                       // 8019AE00
BOOL fn_8019AE2C();                                       // 8019AE2C
BOOL fn_8019AE58();                                       // 8019AE58
BOOL fn_8019AE84(int menu, u16 *buf, int maxLen);         // 8019AE84: text input
BOOL fn_8019AEE4();                                       // 8019AEE4: menu 0x35
BOOL fn_8019AEF0(u32 arg);                                // 8019AEF0: menu 0x49 with arg
BOOL fn_8019AF40(u8 menu);                                // 8019AF40: open a menu by id
BOOL fn_8019AF64(const dMail_c *mail);                    // 8019AF64: letter menu
u32 fn_8019AFCC();                                        // 8019AFCC (8074EA84)
u32 fn_8019AFD4();                                        // 8019AFD4 (8074EA84)
u32 fn_8019AFE4();                                        // 8019AFE4 (8074EA88)
u16 fn_8019B240();                                        // 8019B240 (8074C104)
u32 fn_8019B270();                                        // 8019B270 (8074EA88)
u32 fn_8019B278();                                        // 8019B278 (8074EA8C)
u32 fn_8019B28C();                                        // 8019B28C (8074EA88)
void fn_8019B294(u32 arg);                                // 8019B294: stores lbl_8074EA88
BOOL fn_8019B29C();                                       // 8019B29C
BOOL fn_8019B2C4();                                       // 8019B2C4
BOOL fn_8019B2EC();                                       // 8019B2EC
BOOL fn_8019B348();                                       // 8019B348: selection mask & 1
BOOL fn_8019B350();                                       // 8019B350: mask & 0x80
BOOL fn_8019B358();                                       // 8019B358: mask & 0x20
int fn_8019B360();                                        // 8019B360: number of fn_8019B3B8(0..3)
BOOL fn_8019B3B8(int slot);                               // 8019B3B8: mask & (2 << slot)
BOOL fn_8019B3DC();                                       // 8019B3DC: mask & 0x40
dTime_c fn_8019B494();                                    // 8019B494: the time entered in the time menu (copy of 80624EE0)
BOOL fn_8019B4EC();                                       // 8019B4EC
BOOL fn_8019B4F8();                                       // 8019B4F8
BOOL fn_8019B504();                                       // 8019B504
extern u8 lbl_8074EA7D;                                   // menu result state (1 = valid; no split yet)
}
