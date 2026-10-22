#ifndef PRIVATE_NWC24_MAIL_BOX_H
#define PRIVATE_NWC24_MAIL_BOX_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24MsgBoard.h>
#include <revolution/nwc24/NWC24Types.h>

#include <private/nwc24/NWC24File.h>
#include <private/nwc24/NWC24MsgObj.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24i_MBOX_DIR_LENGTH 64
#define NWC24i_MBOX_MAX_SIZE 0x31C00

#define NWC24i_MSG_ID_MAX 1000000

typedef struct NWC24iMBCHeader {
    u32 magic;         // 0x00
    u32 version;       // 0x04
    u32 numMsgs;       // 0x08
    u32 capacity;      // 0x0C
    u32 totalMsgSize;  // 0x10
    u32 fileSize;      // 0x14
    u32 nextMsgId;     // 0x18
    u32 freeChain;     // 0x1C
    u32 oldestMsgId;   // 0x20
    u32 freeSpace;     // 0x24
    u32 reserved[12];  // 0x28
    char lastUIDL[40]; // 0x58
} NWC24iMBCHeader;

typedef struct NWC24iMBCEntry {
    u32 id;       // 0x00
    u32 flags;    // 0x04
    u32 length;   // 0x08
    u32 appId;    // 0x0C
    u32 unk10;    // 0x10
    u32 tag;      // 0x14
    u32 command;  // 0x18
    u32 unk1C;    // 0x1C
    NWC24iAddr from;
    s32 createTime;
    u32 unk2C;
    u8 numTo;
    u8 numAttached;
    u16 groupId;
    u32 fromField;
    u32 toField;
    u32 subject;
    u32 contentType;
    u32 txEncoding;
    NWC24Data text;
    NWC24Data attached[NWC24i_MSG_ATTACHMENT_MAX];
    u32 attachedSize[NWC24i_MSG_ATTACHMENT_MAX];
    NWC24MIMEType attachedType[NWC24i_MSG_ATTACHMENT_MAX];
    NWC24Data dwcId;
    u32 unk78;
    u32 unk7C;
} NWC24iMBCEntry;

NWC24Err NWC24iOpenMBox();
NWC24Err NWC24iInitMBox();
NWC24Err NWC24iMBoxGetCtrlFilePath(NWC24MBoxType id, char* pDst, s32 maxlen);
NWC24Err NWC24iMBoxOpenNewMsg(NWC24MBoxType id, NWC24File* pFile, u32* pMsgID);
NWC24Err NWC24iMBoxCloseMsg(NWC24File* pFile);
NWC24Err NWC24iMBoxCancelMsg(NWC24File* pFile, NWC24MBoxType id, u32 msgId);
NWC24Err NWC24iMBoxAddMsgObj(NWC24MBoxType id, NWC24iMsgObj* pMsg);
NWC24Err NWC24iMBoxFlushHeader(NWC24MBoxType id);
NWC24Err NWC24iMBoxCheck(NWC24MBoxType id, u32 size);
NWC24Err NWC24iMBoxOpenStoredMsg(NWC24MBoxType id, u32 msgId, NWC24File* pFile);

BOOL NWC24iIsMsgObjReadable(NWC24iMBCEntry* pEntry);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_MAIL_BOX_H
