#ifndef PRIVATE_NWC24_MSG_COMMIT_H
#define PRIVATE_NWC24_MSG_COMMIT_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24MsgBoard.h>

#include <private/nwc24/NWC24MsgObj.h>

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24CommitMsgInternal(NWC24iMsgObj* pMsg, NWC24MBoxType id);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_MSG_COMMIT_H
