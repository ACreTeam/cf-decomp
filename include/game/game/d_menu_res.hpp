#pragma once

// The menu modules (TU d_menu_res.cpp, .text 800CF77C..800CFB5C): the 63 menu RELs (d_menu_*NP, DynamicModuleControl
// in dHeap::menuDylHeap_p) with a reference count each; a menu references the modules it needs (a list per
// menu id), and update() links / unlinks them to match. Used by d_menu_mng. No RTTI: the namespace is from the
// file name, the function names are inferred.

#include <types.h>

namespace dMenuRes {

// The modules, in table order (the REL names without "NP").
enum MenuRes_e {
    MENU_RES_ITEM, // 0 "d_menu_item"
    MENU_RES_ITEMXX, // 1 "d_menu_itemxx"
    MENU_RES_ITEM00, // 2 "d_menu_item00"
    MENU_RES_ITEM01, // 3 "d_menu_item01"
    MENU_RES_ITEM02, // 4 "d_menu_item02"
    MENU_RES_ITEM03, // 5 "d_menu_item03"
    MENU_RES_ITEM04, // 6 "d_menu_item04"
    MENU_RES_ITEM05, // 7 "d_menu_item05"
    MENU_RES_ITEM06, // 8 "d_menu_item06"
    MENU_RES_LTR, // 9 "d_menu_ltr"
    MENU_RES_FRIEND, // 10 "d_menu_friend"
    MENU_RES_FRIEND01, // 11 "d_menu_friend01"
    MENU_RES_FRIEND02, // 12 "d_menu_friend02"
    MENU_RES_FRIEND03, // 13 "d_menu_friend03"
    MENU_RES_FRIEND04, // 14 "d_menu_friend04"
    MENU_RES_FRIEND05, // 15 "d_menu_friend05"
    MENU_RES_FRIEND06, // 16 "d_menu_friend06"
    MENU_RES_FRIEND07, // 17 "d_menu_friend07"
    MENU_RES_FRIEND08, // 18 "d_menu_friend08"
    MENU_RES_PAPER, // 19 "d_menu_paper"
    MENU_RES_KEYTEST, // 20 "d_menu_keytest"
    MENU_RES_DESI, // 21 "d_menu_desi"
    MENU_RES_DESI00, // 22 "d_menu_desi00"
    MENU_RES_EDIT, // 23 "d_menu_edit"
    MENU_RES_CHAT, // 24 "d_menu_chat"
    MENU_RES_INSC, // 25 "d_menu_insc"
    MENU_RES_HAN, // 26 "d_menu_han"
    MENU_RES_MAP, // 27 "d_menu_map"
    MENU_RES_MAP00, // 28 "d_menu_map00"
    MENU_RES_MAP01, // 29 "d_menu_map01"
    MENU_RES_MAP02, // 30 "d_menu_map02"
    MENU_RES_MAP03, // 31 "d_menu_map03"
    MENU_RES_PHOTO, // 32 "d_menu_photo"
    MENU_RES_PHOTO01, // 33 "d_menu_photo01"
    MENU_RES_CATALOG, // 34 "d_menu_catalog"
    MENU_RES_NUMBER, // 35 "d_menu_number"
    MENU_RES_NUMBER00, // 36 "d_menu_number00"
    MENU_RES_MUSEUM, // 37 "d_menu_museum"
    MENU_RES_TIME00, // 38 "d_menu_time00"
    MENU_RES_D_PAPER, // 39 "d_paper"
    MENU_RES_LCRSR, // 40 "d_menu_lcrsr"
    MENU_RES_WRITE, // 41 "d_menu_write"
    MENU_RES_BANK, // 42 "d_menu_bank"
    MENU_RES_KABU, // 43 "d_menu_kabu"
    MENU_RES_MSLCT, // 44 "d_menu_mslct"
    MENU_RES_MII, // 45 "d_menu_mii"
    MENU_RES_DESI01, // 46 "d_menu_desi01"
    MENU_RES_MUSIC, // 47 "d_menu_music"
    MENU_RES_POST00, // 48 "d_menu_post00"
    MENU_RES_POST01, // 49 "d_menu_post01"
    MENU_RES_DESI02, // 50 "d_menu_desi02"
    MENU_RES_DESI03, // 51 "d_menu_desi03"
    MENU_RES_NOTICE, // 52 "d_menu_notice"
    MENU_RES_NOTICE00, // 53 "d_menu_notice00"
    MENU_RES_NOTICE01, // 54 "d_menu_notice01"
    MENU_RES_MELODY, // 55 "d_menu_melody"
    MENU_RES_TITLE, // 56 "d_menu_title"
    MENU_RES_DEBUG00, // 57 "d_menu_debug00"
    MENU_RES_STAR, // 58 "d_menu_star"
    MENU_RES_STAR00, // 59 "d_menu_star00"
    MENU_RES_STAR01, // 60 "d_menu_star01"
    MENU_RES_POINT, // 61 "d_menu_point"
    MENU_RES_HANIWA, // 62 "d_menu_haniwa"

    MENU_RES_NUM,
    MENU_RES_END = MENU_RES_NUM, // ends a menu's module list
};

#define MENU_RES_MENU_NUM 122 // menu ids (the last has no list)

void create();               // 800CF77C: constructs the module controls (in dHeap::menuDylHeap_p)
BOOL destroy();              // 800CF824: FALSE if a module could not be unlinked
BOOL update();               // 800CF958: TRUE once every referenced module is linked and the rest unlinked
void addRef(u8 res);         // 800CFA5C
void release(u8 res);        // 800CFA90
void addMenuRef(u8 menu);    // 800CFAC4: addRef on each module of the menu
void releaseMenuRef(u8 menu); // 800CFB10

} // namespace dMenuRes
