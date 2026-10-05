// Field ground item layouts. See include/game/game/d_fg_data.hpp and notes/d_fg_data.txt.
// .text 8009176C..80091AF8, .ctors 8046566C..80465670, .data 804E17F0..804E1808,
// .bss 80588D10..80588D58.
#include <game/game/d_fg_data.hpp>

// Not split yet (C linkage keeps the target name).
extern "C" void *lbl_8074E3F0; // 8074E3F0: heap the file is loaded into

// 80588D1C
static dFgData_c sFgData;

// 8009176C
int dFgData_c::getFileSize() {
    return 0x300A;
}

// 80091774
BOOL dFgData_c::load() {
    dFgData_c *fgData = &sFgData;
    if (fgData->mData == NULL && lbl_8074E3F0 != NULL) {
        fgData->mData = fgData->mLoader.request("/FgData/fgdata.bin", 0, lbl_8074E3F0);
        if (fgData->mData == NULL) {
            return FALSE;
        }
        fgData->setup();
    }
    return TRUE;
}

// 800917EC
dFgData_c::Header *dFgData_c::getHeader(int section) {
    return (Header *)mData + section;
}

// 800917FC
void dFgData_c::setup() {
    if (mData != NULL) {
        Section *sect = mSections;
        u8 *start = (u8 *)mData + sizeof(Header) * FG_DATA_SECTION_NUM;
        u32 offset = 0;
        for (int i = 0; i < FG_DATA_SECTION_NUM; i++, sect++) {
            Header *header = getHeader(i);
            sect->mCounts = (u16 *)(start + offset);
            sect->mNum = header->mNum;
            if (header->mSize > sect->mNum * sizeof(u16)) {
                sect->mRecords = (Record *)(sect->mCounts + sect->mNum);
            } else {
                sect->mRecords = NULL;
            }
            offset += header->mSize;
        }
    }
}

// 800918A8
dFgData_c::Record *dFgData_c::getRecords(u32 *num, u32 idx, int section) {
    if (section >= FG_DATA_SECTION_NUM) {
        return NULL;
    }
    Section *sect = &mSections[section];
    if (idx < sect->mNum && sect->mCounts != NULL && sect->mRecords != NULL) {
        u16 *count = sect->mCounts;
        u16 *end = count + idx;
        u32 first = 0;
        for (; count != end; count++) {
            first += *count;
        }
        *num = *count;
        if (*num != 0) {
            return sect->mRecords + first;
        }
    }
    return NULL;
}

static inline BOOL isValidUnit(u8 x, u8 z) {
    BOOL ok = FALSE;
    if (x < 16 && z < 16) {
        ok = TRUE;
    }
    return ok;
}

// 80091930
BOOL dFgData_c::getLayout(u16 *items, u32 idx, int section) {
    if (items != NULL) {
        u32 num = 0;
        Record *rec = getRecords(&num, idx, section);
        if (rec != NULL && num != 0) {
            for (Record *end = rec + num; rec != end; rec++) {
                if (isValidUnit(rec->mUnitX, rec->mUnitZ)) {
                    items[(rec->mUnitZ << 4) + rec->mUnitX] = rec->mItem;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 800919E4
BOOL dFgData_getLayout0(u16 *items, int idx) {
    if (idx < 0xF1) {
        return sFgData.getLayout(items, idx, 0);
    }
    return FALSE;
}

// 80091A10
BOOL dFgData_getLayout1(u16 *items, int idx) {
    return sFgData.getLayout(items, idx, 1);
}

// 80091A2C
BOOL dFgData_getLayout2(u16 *items, int idx) {
    return sFgData.getLayout(items, idx, 2);
}
