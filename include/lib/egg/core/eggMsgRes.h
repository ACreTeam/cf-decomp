#pragma once

#include <types.h>

namespace EGG {

// A BMG message resource (.text 80449D3C..8044A11C on RUUE01). Taken from the Wii Sports decomp
// (kiwi515/ogws, include/egg/util/eggMsgRes.h), whose names come from a retail symbol map and its asserts.
// Differences from ogws:
// - the block structs are public: game code (dScript::Res_c users, dMsg) handles the BMG data;
// - this EGG keeps the extract / analyze helpers out of line (ogws inlines them) and adds isExistMsg;
// - no asserts in this build.
// The tables (cDataBlkKindCodeSet, cMainMsgIdBitNumSet, cMsgIdMaskSet) are at 8049C480.
class MsgRes {
public:
    enum EDataBlkKind {
        DATABLK_MSGINFO,
        DATABLK_MSGDATA,
        DATABLK_STRATTR,
        DATABLK_MSGID,
        DATABLK_FLOWCHART,
        DATABLK_FLOWLABEL,
        DATABLK_MAX
    };

    struct DataBlockHeader {
        u32 mSignature; // at 0x0: 'MESG'
        u32 mType;      // at 0x4: 'bmg1'
        u32 mDataSize;  // at 0x8
        u32 mNumBlocks; // at 0xC
    };

    struct DataBlock {
        u32 mKind;     // at 0x0
        u32 mDataSize; // at 0x4
    };

    struct MessageHeader : DataBlockHeader {
        enum EEncoding {
            ENCODING_1BYTE,
            ENCODING_2BYTE,
            ENCODING_SJIS,
            ENCODING_UTF8,
        };

        u8 mEncoding; // at 0x10
        char UNK_0x11[0x20 - 0x11];
    };

    struct MessageInfoDataBlock : DataBlock {
        // The first 8 bytes of an entry; the rest (mEntrySize) are game-specific attributes.
        struct Entry {
            u32 mDataOffset; // at 0x0
            u32 mAttribute;  // at 0x4
        };

        u16 mNumEntries;   // at 0x8
        u16 mEntrySize;    // at 0xA
        u16 mGroupID;      // at 0xC
        Entry mContent[1]; // at 0x10
    };

    struct MessageDataBlock : DataBlock {
        wchar_t mContent[1]; // at 0x8
    };

    struct StringAttributeDataBlock : DataBlock {};

    struct MessageIDDataBlock : DataBlock {
        u16 mNumEntries;    // at 0x8
        u8 mForm;           // at 0xA
        u8 mFormSupplement; // at 0xB
        char UNK_0xC[0x10 - 0xC];
        u32 mContent[1]; // at 0x10
    };

    struct FlowChartInfoDataBlock : DataBlock {};

    struct FlowLabelInfoDataBlock : DataBlock {};

    static const u16 cTagMark = '\x1A';
    static const u32 cDataBlkKindCodeSet[DATABLK_MAX];

    static const u32 cFormSupplementMax = 5;
    struct MsgIDMask {
        u32 groupMask; // at 0x0
        u32 indexMask; // at 0x4
    };
    static const u32 cMainMsgIdBitNumSet[cFormSupplementMax];
    static const MsgIDMask cMsgIdMaskSet[cFormSupplementMax];

protected:
    const MessageHeader *mMsgHeader;                // at 0x0
    const MessageInfoDataBlock *mMsgInfoDataBlk;    // at 0x4
    const MessageDataBlock *mMsgDataBlk;            // at 0x8
    const StringAttributeDataBlock *mMsgStrDataBlk; // at 0xC
    const MessageIDDataBlock *mMsgIDDataBlk;        // at 0x10
    const FlowChartInfoDataBlock *mMsgFlowChartBlk; // at 0x14
    const FlowLabelInfoDataBlock *mMsgFlowLabelBlk; // at 0x18
    // at 0x1C: vtable

public:
    MsgRes(const void *pData); // 80449D3C
    virtual ~MsgRes();         // 80449E94

    // 80449ED4: splits a cTagMark tag into its length, id (group/id word) and parameters
    static void analyzeTag(u16 code, const wchar_t *pTag, u8 *pTagLength, u32 *pTagID, void **ppParam);

    const wchar_t *getMsg(u32 group, u32 idx); // 80449F10
    BOOL isExistMsg(u32 group, u32 idx);       // 80449F4C (not in ogws)

protected:
    const MessageInfoDataBlock::Entry *getMsgEntry(u32 group, u32 idx); // 8044A034
    u32 getMsgID(u16 idx);                                              // 8044A108

private:
    u32 getDataBlkKind(const void *data) {
        return static_cast<const DataBlock *>(data)->mKind;
    }

    u32 getDataBlkSize(const void *dataBlkHeader) {
        return static_cast<const DataBlock *>(dataBlkHeader)->mDataSize;
    }

    void extractMsgHeader(const void *pData);            // 80449F78
    void extractMsgInfoDataBlk(const void *pData);       // 80449F80
    void extractMsgDataBlk(const void *pData);           // 80449F88
    void extractStrAttrDataBlk(const void *pData);       // 80449F90
    void extractMsgIDDataBlk(const void *pData);         // 80449F98
    void extractFlowChartInfoDataBlk(const void *pData); // 80449FA0
    void extractFlowLabelInfoDataBlk(const void *pData); // 80449FA8

    EDataBlkKind analyzeDataBlkKind(u32 kind); // 80449FB0
}; // size 0x20

} // namespace EGG
