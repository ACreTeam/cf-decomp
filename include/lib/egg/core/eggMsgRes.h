#pragma once

#include <types.h>

namespace EGG {

/// @unofficial
struct MsgEntry {
    u8 mPad[4];
    u16 mScale;
    u8 mFont;
};

// A BMG message resource (.text 80449D3C..8044A11C on RUUE01).
// getMsg / getMsgEntry are known names; the rest are inferred from the code.
class MsgRes {
protected:
    /* 0x00 */ const void *mpBinaryHeader;
    /* 0x04 */ const void *mpInf1;
    /* 0x08 */ const void *mpDat1;
    /* 0x0C */ const void *mpStr1;
    /* 0x10 */ const void *mpMid1;
    /* 0x14 */ const void *mpFlw1;
    /* 0x18 */ const void *mpFli1;
    // 0x1C: vtable (MWCC places it where the first virtual is declared)

public:
    // Block kinds, in the order of the magic table (8049C480)
    enum DataBlockKind {
        DATA_BLOCK_INF1,
        DATA_BLOCK_DAT1,
        DATA_BLOCK_STR1,
        DATA_BLOCK_MID1,
        DATA_BLOCK_FLW1,
        DATA_BLOCK_FLI1,
        DATA_BLOCK_UNKNOWN,
    };

    MsgRes(const void *p); // 80449D3C: finds the blocks of the BMG at p
    virtual ~MsgRes(); // 80449E94

    // 80449ED4: splits a 0x1A tag into its size, group/id word and parameters
    static void analyzeTag(u16 tag, const wchar_t *str, u8 *size, u32 *tagId, void **params);

    wchar_t *getMsg(ulong messageGroup, ulong messageID); // 80449F10
    BOOL isExistMsg(ulong messageGroup, ulong messageID); // 80449F4C

    void setBinaryHeader(const void *p); // 80449F78
    void setINF1Data(const void *p); // 80449F80
    void setDAT1Data(const void *p); // 80449F88
    void setSTR1Data(const void *p); // 80449F90
    void setMID1Data(const void *p); // 80449F98
    void setFLW1Data(const void *p); // 80449FA0
    void setFLI1Data(const void *p); // 80449FA8
    DataBlockKind analyzeDataBlockKind(ulong magic); // 80449FB0

    MsgEntry *getMsgEntry(ulong messageGroup, ulong messageID); // 8044A034
    u32 getMsgID(u16 index); // 8044A108: MID1 entry

}; // size 0x20

} // namespace EGG
