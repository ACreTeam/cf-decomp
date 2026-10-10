#pragma once

// d_item_sel.cpp (not decompiled). Only its .bss is declared here.

#include <types.h>

// 8059FF80 (.bss, 0x10): the default item filter passed to the item pickers (fn_800C60B4, fn_800F4608).
// d_fg_mng_task.inc views it as dFgMngItemFilter_c.
extern "C" u8 lbl_8059FF80[0x10];

namespace dItem {
struct Item;
}

// 800C60B4: picks num random items into out from the kind ranges of table (tableNum {kind, ..} int
// records), filtered by filter (lbl_8059FF80: the default) and excluding exclude[0..excludeNum).
extern "C" int fn_800C60B4(dItem::Item *out, int num, const void *table, int tableNum, const void *filter,
                           const dItem::Item *exclude, int excludeNum, int);

// A {kind, sub} record of fn_800C60B4's table.
struct dItemSelRange_c {
    dItemSelRange_c(int kind, int sub) : mKind(kind), mSub(sub) {}

    /* 0x0 */ int mKind;
    /* 0x4 */ int mSub;
};

// A filter argument of fn_800C60B4 (same layout as lbl_8059FF80). _0 is a catalog or NULL.
struct dItemSelFilter_c {
    dItemSelFilter_c(const void *catalog, int a, int b) : _0(catalog), _4(a), _8(b) {}

    /* 0x0 */ const void *_0;
    /* 0x4 */ int _4;
    /* 0x8 */ int _8;
};
