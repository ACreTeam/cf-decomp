#pragma once

// Field ground item layouts from /FgData/fgdata.bin: three sections of 16x16-unit block layouts,
// each a list of (item, unit x, unit z) records. Source: src/dol/game/d_fg_data.cpp
// (.text 8009176C..80091AF8). The names are inferred. See notes/d_fg_data.txt.

#include <types.h>
#include <game/game/d_dvd.hpp>

#define FG_DATA_SECTION_NUM 3

class dFgData_c {
public:
    // Section size table at the start of the file.
    struct Header {
        /* 0x0 */ u16 mSize; // bytes of the section
        /* 0x2 */ u16 mNum;  // layouts in the section
    }; // size 0x4

    // One placed item.
    struct Record {
        /* 0x0 */ u16 mItem;
        /* 0x2 */ u8 mUnitX;
        /* 0x3 */ u8 mUnitZ;
    }; // size 0x4

    struct Section {
        /* 0x0 */ u16 *mCounts;  // records per layout
        /* 0x4 */ Record *mRecords;
        /* 0x8 */ u16 mNum;      // layouts
    }; // size 0xC

    dFgData_c() : mData(NULL) {}

    static int getFileSize();                                       // 8009176C
    static BOOL load();                                             // 80091774: sFgData; TRUE once loaded (or no heap)
    Header *getHeader(int section);                                 // 800917EC
    void setup();                                                   // 800917FC
    Record *getRecords(u32 *num, u32 idx, int section);             // 800918A8
    BOOL getLayout(u16 *items, u32 idx, int section);               // 80091930: into a 16x16 unit grid

    /* 0x00 */ void *mData;
    /* 0x04 */ dDvd::loader_c mLoader;
    /* 0x18 */ Section mSections[FG_DATA_SECTION_NUM];
}; // size 0x3C

BOOL dFgData_getLayout0(u16 *items, int idx); // 800919E4: idx < 0xF1
BOOL dFgData_getLayout1(u16 *items, int idx); // 80091A10
BOOL dFgData_getLayout2(u16 *items, int idx); // 80091A2C
