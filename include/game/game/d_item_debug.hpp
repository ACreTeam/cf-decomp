#pragma once

// Item debugging (DOL TU d_item_debug.cpp, .text 800C578C..800C5C30): mostly stubs in the release build, plus
// a host IO block (RTTI "hostIO_c") that loads test items from "/TestAddItem/testAddItem.arc" with its
// loader (RTTI "dItemDebug::addPackLoader_c"). Function and member names are inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>

namespace dItemDebug {

BOOL create();    // 800C578C: TRUE
void execute();   // 800C5794: runs the host IO's test item load
BOOL draw();      // 800C5874: TRUE
BOOL loadRes();   // 800C59D0: FALSE (d_s_boot)
BOOL checkRes();  // 800C59D8: FALSE (d_s_boot)

// The downloaded item list shown by the debugger (empty in the release build; one file-local instance).
class dlList_c {
public:
    dlList_c() { clear(); }

    void clear();                // 800C59E0 (also from d_item's s_debugFlag)
    void update();               // 800C59E4: after d_item indexed the downloaded items
    void add(dItem::Item item);  // 800C59E8: after d_item indexed one downloaded item
    static dlList_c *get();      // 800C59EC

    /* 0x00 */ u8 mData[0x100];
}; // size 0x100

} // namespace dItemDebug

// d_item's per-file debug flag (s_debugFlag, defined in d_item.cpp; read by d_item_debug's host IO).
class debugFlag_c {
public:
    debugFlag_c() {
        mEnabled = 0;
        dItemDebug::dlList_c::get()->clear();
    }

    u8 mEnabled;
};

extern debugFlag_c s_debugFlag;
