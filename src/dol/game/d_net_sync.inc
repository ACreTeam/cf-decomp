// d_net's #included handler code (.text 800DE0E4..800E3E20, after d_net.cpp's
// __sinit). The file name is provisional. Its functions fill d_net's .rodata
// dispatch tables (13 player/time handlers, 131 receive handlers, 186 record
// copy + 129 record clear handlers, and small state tables).
#include <game/game/d_private_data.hpp>

// The size of a player's dPrivateData_c as sent (0x86C0) and the transfer
// chunking of the town.
#define NET_PLAYER_DATA_SIZE sizeof(dPrivateData_c)
#define NET_DL_BLOCK_SIZE 0x2000

// dSaveOption_c bits of a player (dPrivateData_c::_83E8; d_save_data.cpp's
// getOption).
static inline dSaveOption_c *getNetPlayerOption(dPrivateData_c *player) {
    return (dSaveOption_c *)&player->_83E8;
}

namespace dNet {

// ---- lbl_80473E10: player / time handlers (int member, const u8 *data, u32
// size) ----

// 800DE0E4
void recvDirectHello(int member, const u8 *data, u32 size) {
    if (!g_dNet.mHelloHold.isOn(member) && !g_dNet.mHelloRecv.isOn(member)) {
        g_dNet.mHelloRecv.on(member);
        if (!g_dNet.mHelloReply.isOn(member)) {
            g_dNet.mHelloReply.on(member);
        }
    }
}

// 800DE170
void recvDirectHelloAck(int member, const u8 *data, u32 size) {
    if (!g_dNet.mHelloHold.isOn(member) && g_dNet.mHelloSend.isOn(member)) {
        g_dNet.mHelloSend.off(member);
    }
}

// 800DE1DC
void recvDirectJoinRequest(int member, const u8 *data, u32 size) {
    getNetRequest()->setKind(member, 0);
    getNetRequest()->mRequests.on(member);
}

// 800DE224
void recvDirectJoinResult(int member, const u8 *data, u32 size) {
    int myNo = getNetMyNo()->get();
    if (size != 0) {
        getNetRequest()->setStep(1);
        u8 bits = 0;
        cLib::memCpy(&bits, data, sizeof(bits));
        for (int i = 0; i < 4U; i++) {
            if (bits & 1) {
                getNetAccepted()->on(i);
            } else if (myNo == i) {
                getNetAccepted()->on(myNo);
            }
            bits >>= 1;
        }
        getNetRequest()->mGrantedKind.set(0);
        getNetRequest()->mGrantedMember.set(myNo);
        getNetWatched()->on(member);
    } else {
        getNetRequest()->setStep(0);
    }
}

// 800DE328
void recvDirectXferStart(int member, const u8 *data, u32 size) {
    getNetTransfer()->setReady();
}

// 800DE34C
#ifndef BUGFIXES
void recvDirectXferPlayer(int member, const u8 *data, u32 size) {
    int myNo = getNetMyNo()->get();
    cLib::memCpy(dPlayerMgr::Mgr_c::getNetPlayer(member), data,
                 NET_PLAYER_DATA_SIZE);
    cLib::memCpy(getNetTransfer()->mExclude, data + NET_PLAYER_DATA_SIZE,
                 sizeof(getNetTransfer()->mExclude));
    getNetTransfer()->mItemMaskRecv = 1;
    s64 time;
    cLib::memCpy(&time, data + NET_PLAYER_DATA_SIZE + 0x20, sizeof(time));
    setMemberTime(member, time);
    if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
        dNetMgr_c::connect(member);
    }
    u8 dlNum;
    cLib::memCpy(&dlNum, data + NET_PLAYER_DATA_SIZE + 0x28, sizeof(dlNum));
    data += NET_PLAYER_DATA_SIZE + 0x29;
    for (u32 i = 0; i < dlNum; i++) {
        hasDlItem((void *)data);
        data += NET_DL_BLOCK_SIZE;
    }
    u8 hasAnimal;
    cLib::memCpy(&hasAnimal, data, sizeof(hasAnimal));
    data++;
    if (hasAnimal) {
        if (myNo == 0) {
            fn_800F4840((const dAnimal_c *)data, member);
        }
        data += sizeof(dAnimal_c);
    } else if (myNo == 0) {
        fn_800F4840(NULL, member);
    }
    u8 hasMii;
    cLib::memCpy(&hasMii, data, sizeof(hasMii));
    data++;
    if (hasMii) {
        dMiiIcon::setMemberMii(data, member);
        data += 0x54;
    } else {
        dMiiIcon::setMemberMii(NULL, member);
    }
    u8 scene;
    cLib::memCpy(&scene, data, sizeof(scene));
    getNetTransfer()->setScene(scene);
    if (myNo != 0) {
        getNetWatched()->on(member);
    }
}
#else
void recvDirectXferPlayer(int member, const u8 *data, u32 size) {
    const u32 prefix = NET_PLAYER_DATA_SIZE + 0x29;
    if (member < 0 || member >= NET_MEMBER_MAX || data == NULL ||
        size < prefix + 3) {
        return;
    }
    u8 dlNum = data[prefix - 1];
    u32 left = size - prefix;
    if (dlNum > 2 || dlNum > left / NET_DL_BLOCK_SIZE) {
        return;
    }
    u32 dlBytes = dlNum * NET_DL_BLOCK_SIZE;
    if (left - dlBytes < 3) {
        return;
    }
    const u8 *tail = data + prefix + dlBytes;
    u8 hasAnimal = tail[0];
    if (hasAnimal > 1 ||
        left - dlBytes - 1 < (hasAnimal ? sizeof(dAnimal_c) : 0) + 2) {
        return;
    }
    tail++;
    if (hasAnimal) {
        tail += sizeof(dAnimal_c);
    }
    u8 hasMii = tail[0];
    if (hasMii > 1 ||
        (u32)(data + size - tail) != 1 + (hasMii ? 0x54 : 0) + 1) {
        return;
    }
    int myNo = getNetMyNo()->get();
    cLib::memCpy(dPlayerMgr::Mgr_c::getNetPlayer(member), data,
                 NET_PLAYER_DATA_SIZE);
    cLib::memCpy(getNetTransfer()->mExclude, data + NET_PLAYER_DATA_SIZE,
                 sizeof(getNetTransfer()->mExclude));
    getNetTransfer()->mItemMaskRecv = 1;
    s64 time;
    cLib::memCpy(&time, data + NET_PLAYER_DATA_SIZE + 0x20, sizeof(time));
    setMemberTime(member, time);
    if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
        dNetMgr_c::connect(member);
    }
    const u8 *dl = data + prefix;
    for (u32 i = 0; i < dlNum; i++) {
        hasDlItem((void *)dl);
        dl += NET_DL_BLOCK_SIZE;
    }
    tail = dl + 1;
    if (hasAnimal) {
        if (myNo == 0) {
            fn_800F4840((const dAnimal_c *)tail, member);
        }
        tail += sizeof(dAnimal_c);
    } else if (myNo == 0) {
        fn_800F4840(NULL, member);
    }
    tail++;
    if (hasMii) {
        dMiiIcon::setMemberMii(tail, member);
        tail += 0x54;
    } else {
        dMiiIcon::setMemberMii(NULL, member);
    }
    getNetTransfer()->setScene(*tail);
    if (myNo != 0) {
        getNetWatched()->on(member);
    }
}
#endif

// 800DE51C
#ifndef BUGFIXES
void recvDirectXferItemMask(int member, const u8 *data, u32 size) {
    cLib::memCpy(getNetTransfer()->mExclude, data,
                 sizeof(getNetTransfer()->mExclude));
    getNetTransfer()->mItemMaskRecv = 1;
}
#else
void recvDirectXferItemMask(int member, const u8 *data, u32 size) {
    if (data == NULL || size != sizeof(getNetTransfer()->mExclude)) {
        return;
    }
    cLib::memCpy(getNetTransfer()->mExclude, data, size);
    getNetTransfer()->mItemMaskRecv = 1;
}
#endif

// 800DE564
#ifndef BUGFIXES
void recvDirectXferTown(int member, const u8 *data, u32 size) {
    int myNo = getNetMyNo()->get();
    u32 offset;
    cLib::memCpy(&offset, data, sizeof(offset));
    data += sizeof(offset);
    size -= sizeof(offset);
    if (offset < 0x1635) {
        s64 time;
        cLib::memCpy(&time, data, sizeof(time));
        setMemberTime(member, time);
        u8 value;
        cLib::memCpy(&value, data + 8, sizeof(value));
        dPlayerMgr::setPlayerSlot(0, value);
        cLib::memCpy(getNetSyncBuf()->getBuf0(), data + 9, 2);
        cLib::memCpy(getNetPacket()->getRecordBuf(), data + 0xB, 0x15CA);
        getNetPacket()->clearRecordFlags();
        s64 pending;
        cLib::memCpy(&pending, data + 0x15D5, sizeof(pending));
        dTime_c::setPendingTicks(pending);
        g_dNet.mTimePending = 1;
        u8 hasMii;
        cLib::memCpy(&hasMii, data + 0x15DD, sizeof(hasMii));
        if (hasMii) {
            dMiiIcon::setMemberMii(data + 0x15DE, member);
        } else {
            dMiiIcon::setMemberMii(NULL, member);
        }
        data += 0x1632;
        size -= 0x1632;
        for (int i = 0; i < 4U; i++) {
            if (i != myNo) {
                u8 leave;
                u8 req;
                cLib::memCpy(&req, data, sizeof(req));
                u8 sceneNo = req & 0x7F;
                leave = (req >> 7) & 1;
                dNetScene_c *scene = getNetScene();
                scene->setScene(i, sceneNo, 0);
                getNetScene()->setOwner(i, leave, 0);
                data++;
                size--;
            } else {
                u8 sceneNo = dScene::getCurrentScene();
                getNetScene()->setScene(i, sceneNo, 0);
                getNetScene()->setOwner(i, 1, 0);
            }
        }
    }
    if (offset >= 0x1635) {
        offset -= 0x1635;
    }
    cLib::memCpy((u8 *)dSaveData_c::getTown() + offset, data, size);
    if (offset + size >= sizeof(dSaveTown_c)) {
        if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
            dNetMgr_c::connect(member);
        }
        getNetTransfer()->mReady[0] = 1;
    }
}
#else
void recvDirectXferTown(int member, const u8 *data, u32 size) {
    if (member != 0 || getNetMyNo()->get() == 0 || data == NULL ||
        size < sizeof(u32)) {
        return;
    }
    u32 wireOffset;
    cLib::memCpy(&wireOffset, data, sizeof(wireOffset));
    data += sizeof(wireOffset);
    size -= sizeof(wireOffset);
    dNetTransfer_c *transfer = getNetTransfer();
    if (wireOffset != transfer->mTownRecv || size == 0) {
        return;
    }
    const u32 prefix = 0x1635;
    const u32 townSize = sizeof(dSaveTown_c);
    u32 townOffset;
    u32 townLength;
    if (wireOffset == 0) {
        if (size <= prefix) {
            return;
        }
        townOffset = 0;
        townLength = size - prefix;
    } else {
        if (wireOffset < prefix) {
            return;
        }
        townOffset = wireOffset - prefix;
        townLength = size;
    }
    if (townOffset >= townSize || townLength > townSize - townOffset) {
        return;
    }
    int myNo = getNetMyNo()->get();
    if (wireOffset == 0) {
        s64 time;
        cLib::memCpy(&time, data, sizeof(time));
        setMemberTime(member, time);
        u8 value = data[8];
        dPlayerMgr::setPlayerSlot(0, value);
        cLib::memCpy(getNetSyncBuf()->getBuf0(), data + 9, 2);
        cLib::memCpy(getNetPacket()->getRecordBuf(), data + 0xB, 0x15CA);
        getNetPacket()->clearRecordFlags();
        s64 pending;
        cLib::memCpy(&pending, data + 0x15D5, sizeof(pending));
        dTime_c::setPendingTicks(pending);
        g_dNet.mTimePending = 1;
        if (data[0x15DD]) {
            dMiiIcon::setMemberMii(data + 0x15DE, member);
        } else {
            dMiiIcon::setMemberMii(NULL, member);
        }
        data += 0x1632;
        for (int i = 0; i < NET_MEMBER_MAX; i++) {
            if (i != myNo) {
                u8 req = *data++;
                dNetScene_c *scene = getNetScene();
                scene->setScene(i, req & 0x7F, 0);
                scene->setOwner(i, (req >> 7) & 1, 0);
            } else {
                u8 sceneNo = dScene::getCurrentScene();
                getNetScene()->setScene(i, sceneNo, 0);
                getNetScene()->setOwner(i, 1, 0);
            }
        }
    }
    cLib::memCpy((u8 *)dSaveData_c::getTown() + townOffset, data, townLength);
    transfer->mTownRecv += size;
    if (townOffset + townLength == townSize) {
        if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
            dNetMgr_c::connect(member);
        }
        transfer->mReady[0] = 1;
    }
}
#endif

// 800DE7AC
#ifndef BUGFIXES
void recvDirectXferPlayerReply(int member, const u8 *data, u32 size) {
    getNetMyNo()->get();
    cLib::memCpy(dPlayerMgr::Mgr_c::getNetPlayer(member), data,
                 NET_PLAYER_DATA_SIZE);
    s64 time;
    cLib::memCpy(&time, data + NET_PLAYER_DATA_SIZE, sizeof(time));
    setMemberTime(member, time);
    u8 hasMii;
    cLib::memCpy(&hasMii, data + NET_PLAYER_DATA_SIZE + 8, sizeof(hasMii));
    if (hasMii) {
        dMiiIcon::setMemberMii(data + NET_PLAYER_DATA_SIZE + 9, member);
    } else {
        dMiiIcon::setMemberMii(NULL, member);
    }
    if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
        dNetMgr_c::connect(member);
    }
    getNetTransfer()->mReady[member] = 1;
}
#else
void recvDirectXferPlayerReply(int member, const u8 *data, u32 size) {
    const u32 prefix = NET_PLAYER_DATA_SIZE + sizeof(s64) + 1;
    if (member < 0 || member >= NET_MEMBER_MAX || data == NULL ||
        size < prefix) {
        return;
    }
    u8 hasMii = data[prefix - 1];
    if (hasMii > 1 || size != prefix + (hasMii ? 0x54 : 0)) {
        return;
    }
    cLib::memCpy(dPlayerMgr::Mgr_c::getNetPlayer(member), data,
                 NET_PLAYER_DATA_SIZE);
    s64 time;
    cLib::memCpy(&time, data + NET_PLAYER_DATA_SIZE, sizeof(time));
    setMemberTime(member, time);
    if (hasMii) {
        dMiiIcon::setMemberMii(data + prefix, member);
    } else {
        dMiiIcon::setMemberMii(NULL, member);
    }
    if (!getNetPlayerOption(dPlayerMgr::Mgr_c::getNetPlayer(member))->mVoiceChat) {
        dNetMgr_c::connect(member);
    }
    getNetTransfer()->mReady[member] = 1;
}
#endif

// 800DE894
#ifndef BUGFIXES
void recvDirectXferItems(int member, const u8 *data, u32 size) {
    u8 flag;
    cLib::memCpy(&flag, data, sizeof(flag));
    data++;
    size--;
    for (u32 i = 0; i < 0x100; i++) {
        if (size < NET_DL_BLOCK_SIZE) {
            break;
        }
        isDlBlockUsed((void *)data);
        data += NET_DL_BLOCK_SIZE;
        size -= NET_DL_BLOCK_SIZE;
    }
    if (flag) {
        getNetTransfer()->mItemsRecv = 1;
    }
}
#else
void recvDirectXferItems(int member, const u8 *data, u32 size) {
    if (data == NULL || size < 1 || (size - 1) % NET_DL_BLOCK_SIZE != 0 ||
        (size - 1) / NET_DL_BLOCK_SIZE > 11 || data[0] > 1) {
        return;
    }
    u8 flag = *data++;
    size--;
    while (size != 0) {
        isDlBlockUsed((void *)data);
        data += NET_DL_BLOCK_SIZE;
        size -= NET_DL_BLOCK_SIZE;
    }
    if (flag) {
        getNetTransfer()->mItemsRecv = 1;
    }
}
#endif

// 800DE924
void recvDirectXferTownRequest(int member, const u8 *data, u32 size) {
    getNetTransfer()->mTownReqRecv = 1;
}

// 800DE954
void recvDirectJoinDoneHost(int member, const u8 *data, u32 size) {
    getNetJoinDone()->setAllDone(1);
}

// 800DE97C
void recvDirectJoinDone(int member, const u8 *data, u32 size) {
    getNetJoinDone()->setDone(member, 1);
}

// ---- lbl_80473E44: receive handlers by packet id (int member, const u8 *data,
// u32 size, u32 arg, u8 scene) ----

// 800DE9B4: NET_PACKET_NONE (0x00)
void recvNone(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
}

// 800DE9B8: NET_PACKET_JOIN_DONE (0x01)
void recvJoinDone(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetJoinDone()->setDone(member, 1);
}

// 800DE9F0: NET_PACKET_JOIN_DONE_HOST (0x02)
void recvJoinDoneHost(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetJoinDone()->setAllDone(1);
}

// 800DEA18: NET_PACKET_JOIN_REQUEST (0x03)
void recvJoinRequest(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 kind;
    cLib::memCpy(&kind, data, sizeof(kind));
    getNetRequest()->mRequests.on(member);
    getNetRequest()->setKind(member, kind);
}

// 800DEA6C: NET_PACKET_JOIN_SAVE_REPLY (0x04)
void recvJoinSaveReply(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    u8 step;
    cLib::memCpy(&step, data, sizeof(step));
    getNetRequest()->setMemberStep(member, step);
}

// 800DEAB0: NET_PACKET_JOIN_SAVE_PROMPT (0x05)
void recvJoinSavePrompt(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 value;
    cLib::memCpy(&value, data, sizeof(value));
    int kind = value & 3;
    int saveMember = (value >> 2) & 3;
    getNetRequest()->setSaveStep(1);
    getNetRequest()->mSaveKind.set(kind);
    getNetRequest()->mSaveMember.set(saveMember);
}

// 800DEB20: NET_PACKET_JOIN_SAVE_CANCEL (0x06)
void recvJoinSaveCancel(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    getNetRequest()->setSaveStep(5);
}

// 800DEB48: NET_PACKET_JOIN_CANCEL_DONE (0x07)
void recvJoinCancelDone(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    getNetRequest()->setMemberStep(member, 0);
}

// 800DEB80: NET_PACKET_JOIN_RESULT (0x08)
void recvJoinResult(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 step;
    cLib::memCpy(&step, data, sizeof(step));
    getNetRequest()->setStep(step);
}

// 800DEBB4: NET_PACKET_JOIN_RELEASE (0x09)
void recvJoinRelease(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetRequest()->mBusy.off(member);
}

// 800DEBEC: NET_PACKET_JOIN_COMMIT (0x0A)
void recvJoinCommit(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    int saveMember = getNetRequest()->mSaveMember.get();
    int kind = getNetRequest()->mSaveKind.get();
    if (kind == 0) {
        getNetAccepted()->on(saveMember);
    }
    getNetRequest()->mGrantedKind.set(kind);
    getNetRequest()->mGrantedMember.set(saveMember);
    getNetRequest()->setSaveStep(6);
}

// 800DEC74: NET_PACKET_SCENE_REQUEST (0x0B)
void recvSceneRequest(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNetSceneReq_c value;
    cLib::memCpy(&value, data, sizeof(value));
    if (value.mLeave) {
        getNetScene()->removeOrder(member);
    } else {
        getNetScene()->addOrder(member, value.mScene);
    }
}

// 800DECD8: NET_PACKET_SCENE_LEADER (0x0C)
void recvSceneLeader(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNetMemberScene_c leader;
    cLib::memCpy(&leader, data, sizeof(leader));
    getNetScene()->setLeader(leader);
    dNetMemberScene_c copy;
    copy.set(leader);
    int owner = copy.getMember();
    int me = getMyNo();
    if (owner == me) {
        getNetScene()->mLeaderAcks.on(me);
    } else {
        getNetPacket()->begin();
        getNetPacket()->send(NET_PACKET_SCENE_LEADER_ACK, owner);
    }
}

// 800DED70: NET_PACKET_SCENE_NO_LEADER (0x0D)
void recvSceneNoLeader(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    getNetScene()->clearLeader();
}

// 800DED94: NET_PACKET_SCENE_LEADER_ACK (0x0E)
void recvSceneLeaderAck(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    getNetScene()->mLeaderAcks.on(member);
}

// 800DEDCC: NET_PACKET_SCENE_STATE (0x0F)
void recvSceneState(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNetSceneMsg_c msg;
    cLib::memCpy(&msg, data, sizeof(msg));
    int who = msg.mMember;
    int ackTo = msg.mAckTo;
    if (msg.mHasScene) {
        getNetScene()->setScene(who, msg.mScene, 0);
    }
    if (msg.mHasFlag) {
        getNetScene()->setOwner(who, msg.mFlag, 0);
    }
    if (ackTo < NET_MEMBER_MAX) {
        getNetPacket()->begin();
        getNetPacket()->send(NET_PACKET_SCENE_STATE_ACK, ackTo);
    }
}

// 800DEE78: NET_PACKET_SCENE_STATE_ACK (0x10)
void recvSceneStateAck(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    getNetScene()->mMsgAcks.on(member);
}

// 800DEEB0: NET_PACKET_SCENE_SAVE_HOST (0x11)
void recvSceneSaveHost(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    getNetSceneClose()->setData(data, size);
    u8 sceneNo = getNetScene()->getScene(member);
    getNetSceneClose()->read(member, sceneNo);
    getNetSceneClose()->init();
}

// 800DEF28: NET_PACKET_SCENE_LOAD_REQUEST (0x12)
void recvSceneLoadRequest(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 sceneNo = getNetScene()->getLeaderScene();
    getNetSceneOpen()->write(member, sceneNo);
    getNetPacket()->begin();
    u32 bufSize = getNetSceneOpen()->getSize();
    if (bufSize != 0) {
        getNetPacket()->add(getNetSceneOpen()->getBuf(), bufSize);
    }
    getNetPacket()->send(NET_PACKET_SCENE_LOAD, member);
}

// 800DEFC4: NET_PACKET_SCENE_LOAD (0x13)
void recvSceneLoad(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetSceneOpen()->setData(data, size);
    getNetSceneOpen()->setRecv(1);
}

// 800DF014: NET_PACKET_SCENE_EXIT_REQUEST (0x14)
void recvSceneExitRequest(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    getNetScene()->mLeaving = member;
    getNetPacket()->begin();
    getNetPacket()->send(NET_PACKET_SCENE_EXIT_ACK, member);
}

// 800DF05C: NET_PACKET_SCENE_EXIT_ACK (0x15)
void recvSceneExitAck(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetScene()->setLeaveAcked(1);
}

// 800DF084: NET_PACKET_SCENE_EXIT_DATA (0x16)
void recvSceneExitData(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    getNetSceneLeave()->setData(data, size);
    u8 sceneNo = getNetScene()->getScene(member);
    getNetSceneLeave()->read(member, sceneNo);
    getNetSceneLeave()->init();
}

// 800DF0FC: NET_PACKET_SCENE_ENTER_REQUEST (0x17)
void recvSceneEnterRequest(int member, const u8 *data, u32 size, u32 arg,
                           u8 scene) {
    getNetSceneEnter()->mRequester = member;
}

// 800DF12C: NET_PACKET_SCENE_ENTER_DATA (0x18)
void recvSceneEnterData(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    getNetSceneEnter()->setData(data, size);
    getNetSceneEnter()->setRecv(1);
}

// 800DF17C: NET_PACKET_SCENE_HANDOVER_REQUEST (0x19)
void recvSceneHandoverRequest(int member, const u8 *data, u32 size, u32 arg,
                              u8 scene) {
    getNetHoldBuf()->mOldOwner = member;
    getNetHoldBuf()->mHoldState = 0;
    getNetPacket()->begin();
    getNetPacket()->send(NET_PACKET_SCENE_HANDOVER_ACK, member);
}

// 800DF1D8: NET_PACKET_SCENE_HANDOVER_ACK (0x1A)
void recvSceneHandoverAck(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    getNetScene()->mHandoverAcks.on(member);
}

// 800DF210: NET_PACKET_SCENE_HANDOVER_DATA (0x1B)
void recvSceneHandoverData(int member, const u8 *data, u32 size, u32 arg,
                           u8 scene) {
    int me = getMyNo();
    getNetSceneHandover()->setData(data, size);
    u8 sceneNo = getNetScene()->getScene(me);
    getNetSceneHandover()->read(me, member, sceneNo);
    getNetSceneHandover()->init();
}

// 800DF29C: NET_PACKET_SAVE_DONE (0x1C)
void recvSaveDone(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    getNetSaveEnd()->mSaved.on(member);
}

// 800DF2D4: NET_PACKET_SAVE_DONE_ALL (0x1D)
void recvSaveDoneAll(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    int me = getMyNo();
    getNetSaveEnd()->mSaved.on(me);
}

// 800DF310: NET_PACKET_SYNC_BUF_MEMBER (0x1E)
void recvSyncBufMember(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    cLib::memCpy(getNetSyncBuf()->getBuf4(member), data, 2);
    getNetSyncBuf()->readBuf4(member);
    if (getNetSaveEnd()->getStep() == 1) {
        getNetSaveEnd()->mSaved.on(member);
    }
}

// 800DF388: NET_PACKET_SYNC_BUF (0x1F)
#ifndef BUGFIXES
void recvSyncBuf(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNetMemberMask_c mask;
    cLib::memCpy(&mask, data, sizeof(mask));
    getNetSyncBuf()->unpack(mask, data + sizeof(mask), size - sizeof(mask));
}
#else
void recvSyncBuf(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    if (data == NULL || size < sizeof(dNetMemberMask_c)) {
        return;
    }
    dNetMemberMask_c mask;
    cLib::memCpy(&mask, data, sizeof(mask));
    getNetSyncBuf()->unpack(mask, data + sizeof(mask), size - sizeof(mask));
}
#endif

// 800DF3E8: NET_PACKET_DISCONNECT_START (0x20)
void recvDisconnectStart(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    int me = getMyNo();
    getNetDisconnect()->mAcks.on(me);
}

// 800DF424: NET_PACKET_DISCONNECT_ACK (0x21)
void recvDisconnectAck(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    getNetDisconnect()->mAcks.on(member);
}

// 800DF45C: NET_PACKET_DISCONNECT_READY (0x22)
void recvDisconnectReady(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    getNetDisconnect()->mReady.on(member);
}

// 800DF494: NET_PACKET_DISCONNECT_GO (0x23)
void recvDisconnectGo(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    int me = getMyNo();
    getNetDisconnect()->mReady.on(me);
}

// 800DF4D0: NET_PACKET_PLAYER_MNGR_25 (0x25)
void recvPlayerMngr25(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 a;
    cLib::memCpy(&a, data, sizeof(a));
    u16 b;
    cLib::memCpy(&b, data + 1, sizeof(b));
    dPlayerMgr::receiveEquipment(member, a, b);
}

// 800DF530: NET_PACKET_PLAYER_MNGR_26 (0x26)
void recvPlayerMngr26(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 a;
    cLib::memCpy(&a, data, sizeof(a));
    u16 b;
    cLib::memCpy(&b, data + 1, sizeof(b));
    dPlayerMgr::queueRemoteEquipment(member, a, b);
}

// 800DF590: NET_PACKET_PLAYER_STATE (0x27)
void recvPlayerState(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    if (member == getMyNo()) {
        return;
    }
    int value;
    cLib::memCpy(&value, data, sizeof(value));
    u8 state;
    cLib::memCpy(&state, data + 4, sizeof(state));
    mVec3_c param;
    cLib::memCpy(&param, data + 5, 9);
    if (state == 0x60) {
        fgMngTask_finishActive(member);
    }
    if (scene == dScene::getCurrentScene()) {
        dPlayerActor_c *player = dPlayerMgr::getPlayerActorRaw(member);
        if (player != NULL &&
            player->getLifecycleState() == fBase_c::DELETING) {
            dPlayerMgr::updateActionFieldTasks(member, scene, state, (const u8 *)&param);
        } else if (player != NULL && !dPlayerMgr::fn_800FC0C0(member)) {
            int x;
            u8 b;
            u8 c;
            if (dPlayerMgr::getRemoteStateInfo(&x, &b, &c, member)) {
                c = c - 1;
            } else {
                c = 0xAB;
            }
            if (c != 0 || dPlayerMgr::isSyncValueAhead(value, x)) {
                dPlayerMgr::setRemoteActionSync(member, c, x);
            }
            dPlayerMgr::setNetworkPosition(member, &param);
            dPlayerMgr::changeNetPlayerState(member, state);
            dPlayerMgr::setRemoteSyncValue(member, value);
            if (state == 0xE || state == 0x46 || state == 0x53) {
                dPlayerMgr::cacheRemoteState(member, state);
                dPlayerMgr::cacheRemoteParams(member, (const u8 *)&param);
            }
        } else {
            dPlayerMgr::cacheUnavailableActorAction(member, scene, state, (const u8 *)&param);
        }
    } else {
        dPlayerMgr::cacheOtherSceneAction(member, scene, state, (const u8 *)&param);
    }
}

// 800DF760: NET_PACKET_PLAYER_MNGR_24 (0x24)
void recvPlayerMngr24(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 value;
    cLib::memCpy(&value, data, sizeof(value));
    dPlayerMgr::receiveAppearance(member, value);
}

// 800DF7A0: NET_PACKET_FG_MNG_REQ (0x28)
void recvFgMngReq(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dFgMngReq_c req;
    cLib::memCpy(&req, data, sizeof(req));
    int valid = fgMngProc_isChangeRequestValid(&req, scene);
    req.send((u8)valid, member, scene);
}

// 800DF804: NET_PACKET_FG_MNG_CMD (0x29)
void recvFgMngCmd(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    if (!isHost()) {
        dFgMngCmd_c cmd;
        cLib::memCpy(&cmd, data, sizeof(cmd));
        fgMngCmd_apply(&cmd, size);
    }
}

// 800DF860: NET_PACKET_FG_MNG_TASK_DONE (0x2A)
void recvFgMngTaskDone(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    dFgMngTaskMsg_c msg;
    cLib::memCpy(&msg, data, sizeof(msg));
    dFdAsPos_c pos(msg.mPos >> 8, msg.mPos & 0xFF);
    int idx = fgMngLock_find(msg.mScene, &pos, msg.mLayer);
    if (idx >= 0) {
        dFgMngLock_c *lock = fgMngLock_get(idx);
        lock->mPendingPlayers &= ~(1 << member);
    }
}

// 800DF8E4: NET_PACKET_FG_MNG_UNIT_FG (0x2B)
void recvFgMngUnitFg(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u16 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    fgMngProc_recvUnitFg(buf);
}

// 800DF914: NET_PACKET_FTR_HOLD_REQUEST (0x33)
void recvFtrHoldRequest(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvHoldRequest(buf, member);
}

// 800DF954: NET_PACKET_FTR_HOLD_OK (0x34)
void recvFtrHoldOk(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dFTR::recvHoldAnswer(TRUE);
}

// 800DF95C: NET_PACKET_FTR_HOLD_NG (0x35)
void recvFtrHoldNg(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dFTR::recvHoldAnswer(FALSE);
}

// 800DF964: NET_PACKET_FTR_HOLD_RELEASE (0x36)
void recvFtrHoldRelease(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    dFTR::recvHoldRelease(member, 1);
}

// 800DF96C: NET_PACKET_FTR_ACTION2 (0x38)
void recvFtrAction2(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvRoomFlag(buf);
}

// 800DF99C: NET_PACKET_FTR_ACTION (0x37)
void recvFtrAction(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvSwitch(buf);
}

// 800DF9CC: NET_PACKET_FTR_PUT (0x39)
void recvFtrPut(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvPutFtr(scene, buf);
}

// 800DFA0C: NET_PACKET_FTR_REMOVE (0x3A)
void recvFtrRemove(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvRemoveFtr(scene, buf);
}

// 800DFA4C: NET_PACKET_FTR_SET_UNIT (0x3B)
void recvFtrSetUnit(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvUnitItem(scene, buf);
}

// 800DFA8C: NET_PACKET_FTR_OWNER (0x3C)
void recvFtrOwner(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dFTR::recvOwnerSwitch(buf);
}

// 800DFABC: NET_PACKET_STR_LOCK (0x31)
void recvStrLock(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 value;
    cLib::memCpy(&value, data, sizeof(value));
    dSTR::recvLock(&value, member);
}

// 800DFAFC: NET_PACKET_STR_UNLOCK (0x32)
void recvStrUnlock(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 value;
    cLib::memCpy(&value, data, sizeof(value));
    dSTR::recvUnlock(&value, member);
}

// 800DFB3C: NET_PACKET_PATTERN_COPY (0x3E)
void recvPatternCopy(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    dPatternCopy::recvPacket(buf, member);
}

// 800DFB7C: NET_PACKET_PATTERN_COPY_ACK (0x3F)
void recvPatternCopyAck(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 value;
    cLib::memCpy(&value, data, sizeof(value));
    dPatternCopy::recvAck(value != 0, member);
}

// 800DFBC8: NET_PACKET_ROOM_ITEM (0x3D) (dBgUtil::sendRoomItem)
void recvRoomItem(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dBgUtil::roomItemMsg_c msg;
    cLib::memCpy(&msg, data, sizeof(msg));
    dBgUtil::recvRoomItem(&msg);
}

} // namespace dNet

// 8074E510: overrides the NET_PACKET_INSECT_FIELD handler when set.
typedef void (*dNetPingFunc)(int member);
dNetPingFunc lbl_8074E510;

namespace dNet {

// The NET_PACKET_INSECT_FIELD message.
struct dNetPingMsg_c {
    dNetPingMsg_c() {
        b0 = 0;
        b1 = 0;
        b2 = 0;
        b3 = 0;
    }
    u32 getIdx() const {
        return (b0 >> 2) & 0xF;
    }
    u32 getValue() const {
        return ((b0 >> 6) & 3) + (b1 & 0x1F) * 4;
    }
    void setType(u32 type) {
        b0 &= ~0x03;
        b0 |= type & 3;
    }
    void setIdx(u32 idx) {
        b0 &= ~0x3C;
        b0 |= (idx & 0xF) << 2;
    }
    void setValue(u32 value) {
        b0 &= ~0xC0;
        b0 |= (value & 3) << 6;
        b1 &= ~0x1F;
        b1 |= (value >> 2) & 0x1F;
    }
    void setFlags(u32 flags) {
        b1 &= ~0xE0;
        b1 |= (flags & 7) << 5;
    }

    /* 0x0 */ u8 b0;
    /* 0x1 */ u8 b1;
    /* 0x2 */ u8 b2;
    /* 0x3 */ u8 b3;
};

// 800DFBF8: NET_PACKET_INSECT_FIELD (0x40)
void recvInsectField(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    if (lbl_8074E510 != NULL) {
        lbl_8074E510(member);
        return;
    }
    dNetPingMsg_c out;
    dNetPingMsg_c in;
    cLib::memCpy(&in, data, sizeof(in));
    out.setType(3);
    out.setIdx(in.getIdx());
    out.setValue(in.getValue());
    out.setFlags(0);
    beginPacket();
    addPacket(&out, sizeof(out));
    sendPacket(NET_PACKET_INSECT_FIELD, member);
}

// 800DFCCC: NET_PACKET_PLAY_SYNC (0x41) (dPlaySyncMsg_c)
void recvPlaySync(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPlaySyncMsg_c out;
    dPlaySyncMsg_c msg;
    cLib::memCpy(&msg, data, sizeof(msg));
    switch (msg.getType()) {
    case 0: {
        u32 owner = msg.getMember();
        if (owner == getMyNo()) {
            recvSyncRelease(&msg, getMyNo());
            recvSyncRelease(&msg, member);
        } else {
            out.setType(1);
            out.setIdx(msg.getIdx());
            out.setA(msg.getA());
            out.setC(msg.getC());
            out.setMember(msg.getMember());
            beginPacket();
            addPacket(&out, sizeof(out));
            sendPacket(NET_PACKET_PLAY_SYNC, owner);
        }
        break;
    }
    case 1:
        recvSyncRelease(&msg, member);
        break;
    case 2:
        if (getMyNo() == 0) {
            recvSyncAck(&msg, getMyNo());
            recvSyncAck(&msg, member);
        } else {
            out.setType(3);
            out.setIdx(msg.getIdx());
            out.setA(msg.getA());
            out.setC(msg.getC());
            out.setMember(msg.getMember());
            beginPacket();
            addPacket(&out, sizeof(out));
            sendPacket(NET_PACKET_PLAY_SYNC, 0);
        }
        break;
    case 3:
        recvSyncAck(&msg, member);
        break;
    case 4:
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN) && isSceneOwner()) {
            dPlaySyncRecBuf_c buf;
            int id = msg.getIdx() + PLAY_SYNC_REC_ID;
            cLib::memCpy(&buf, getRecord(id), sizeof(dPlaySyncRec_c));
            buf.mRec.setState(PLAY_SYNC_STATE_NONE);
            setRecord(id, &buf, 0);
        }
        break;
    }
}

// 800DFFB4: NET_PACKET_SHOP_SALE (0x42)
void recvShopSale(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dShopSale_c sale;
    cLib::memCpy(&sale, data, sizeof(sale));
    dShopLayout_applySale(sale, member, scene);
}

// 800E0014: NET_PACKET_POST_SYNC (0x2D)
void recvPostSync(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPostOffice::onSyncPacket(member, data);
}

// 800E0018: NET_PACKET_MAIL (0x2E)
void recvMail(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPostOffice::onMailPacket(member, data);
}

// 800E001C: NET_PACKET_NOTICE_ADD (0x2F)
void recvNoticeAdd(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNotice::receiveAddNotice(member, data);
}

// 800E0020: NET_PACKET_NOTICE_REMOVE (0x30)
void recvNoticeRemove(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNotice::receiveRemoveNotice(member, data);
}

// 800E0024: NET_PACKET_CHAT (0x52)
void recvChat(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dObjChat::receiveText(data, size, member);
}

// 800E0038: NET_PACKET_CHAT_PRESET (0x53)
void recvChatPreset(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dObjChat::receivePreset(data, member);
}

// 800E0048: NET_PACKET_CLOCK (0x54)
void recvClock(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dClock::receive(member, data);
}

// 800E004C: NET_PACKET_VOICE (0x61)
void recvVoice(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dObjVoice_c::receive(member, data);
}

// 800E0050: NET_PACKET_RECYCLE_BIN (0x62)
void recvRecycleBin(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPostOffice::onRecycleBin(member, data);
}

// 800E0054: NET_PACKET_RECYCLE_BIN_ACK (0x63)
void recvRecycleBinAck(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    dPostOffice::onRecycleAck(member);
}

// 800E0058: NET_PACKET_MELODY (0x67)
void recvMelody(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPostOffice::onMelody(member, data);
}

// 800E005C: NET_PACKET_MELODY_ACK (0x68)
void recvMelodyAck(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dPostOffice::onMelodyAck(member);
}

// 800E0060: NET_PACKET_DEMO_SYNC (0x2C)
void recvDemoSync(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dDemoSync::receive(member, data, size);
}

// 800E0064: NET_PACKET_DAILY_RAND (0x51)
void recvDailyRand(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 value = *data;
    dSaveData_c::getTown()->mDailyRand.mValue = value;
}

// 800E0098: NET_PACKET_NPC_SPOT_A_FLAG (0x4B)
void recvNpcSpotAFlag(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mNpcSpotA.mFlag = 1;
}

// 800E00C4: NET_PACKET_NPC_SPOT_A (0x4C)
void recvNpcSpotA(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNpcInfo::jingle_c::get()->move();
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, buf[1], buf[2]);
    pos.y = 0.0f;
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mNpcSpotA.set(buf[0], &pos);
}

// 800E0134: NET_PACKET_NPC_SPOT_B_FLAG (0x4D)
void recvNpcSpotBFlag(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mNpcSpotB.mFlag = 1;
}

// 800E0160: NET_PACKET_NPC_SPOT_B (0x4E)
void recvNpcSpotB(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dSaveData_c::getTown()->mNpcSpotB.set(buf[0], buf[1]);
}

// 800E01A0: NET_PACKET_HARVEST_SPOT_FLAG (0x4F)
void recvHarvestSpotFlag(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    dSaveData_c::getTown()->mHarvestSpot.mFlags |= dSvNpcPos::SIDE_POS_FLAG_EVENT;
}

// 800E01D0: NET_PACKET_HARVEST_SPOT (0x50)
void recvHarvestSpot(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNpcInfo::franklin_c::get()->relocate();
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, buf[0], buf[1]);
    pos.y = 0.0f;
    dSaveData_c::getTown()->mHarvestSpot.set(&pos);
}

// 800E023C: NET_PACKET_NPC_EVENT_START (0x45)
void recvNpcEventStart(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    dNpcInfo::totakeke_c::get()->setPlaying(TRUE);
    dNpcInfo::totakeke_c *info = dNpcInfo::totakeke_c::get();
    info->mIsRequester = 0;
}

// 800E0270: NET_PACKET_NPC_EVENT_DATA (0x46)
void recvNpcEventData(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[0x28];
    cLib::memCpy(buf, data, sizeof(buf));
    dNpcInfo::totakeke_c::get()->fromPacket((const dNpcInfo::totakekePacket_c *)buf);
}

// 800E02A4: NET_PACKET_NPC_EVENT_JOIN (0x47)
void recvNpcEventJoin(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    dNpcInfo::totakeke_c::get()->mAudienceNum++;
    dNpcInfo::totakeke_c *info = dNpcInfo::totakeke_c::get();
    info->mHadAudience = 1;
}

// 800E02DC: NET_PACKET_NPC_EVENT_PLAY (0x48)
void recvNpcEventPlay(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNpcInfo::totakeke_c *info = dNpcInfo::totakeke_c::get();
    info->mPlayReq = 1;
}

// 800E0304: NET_PACKET_NPC_EVENT_END (0x49)
void recvNpcEventEnd(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dNpcInfo::totakeke_c::get()->setPlaying(FALSE);
    dNpcInfo::totakeke_c *info = dNpcInfo::totakeke_c::get();
    info->mPerforming = 0;
}

// 800E0338: NET_PACKET_NPC_EVENT_LEAVE (0x4A)
void recvNpcEventLeave(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    dNpcInfo::totakeke_c::get()->mAudienceDoneNum++;
}

// 800E0364: NET_PACKET_FISHING_RECORD (0x43)
void recvFishingRecord(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    dSaveTown_c *town = dSaveData_c::getTown();
    memcpy(&town->mFishing, data, sizeof(town->mFishing));
}

// 800E03A4: NET_PACKET_BUG_OFF (0x44)
void recvBugOff(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dSaveTown_c *town = dSaveData_c::getTown();
    memcpy(&town->mBugOff, data, sizeof(town->mBugOff));
}

// 800E03E4: NET_PACKET_ANIMAL_HELD_ITEM (0x55)
void recvAnimalHeldItem(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800EFF60(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        animal->setHeldItem(&item);
    }
}

// 800E0454: NET_PACKET_ANIMAL_HELD_ITEM_TIME (0x56)
void recvAnimalHeldItemTime(int member, const u8 *data, u32 size, u32 arg,
                            u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    u8 value = 0;
    fn_800F0020(&idx, &value, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        animal->mHeldItemChangeMinute = value;
    }
}

// 800E04C0: NET_PACKET_ANIMAL_EVENT_FLAGS (0x57)
void recvAnimalEventFlags(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    u8 value = 0;
    fn_800F00D4(&idx, &value, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        animal->mEvent.mFlags = value;
    }
}

// 800E052C: NET_PACKET_ANIMAL_MEMORY_INIT (0x59)
void recvAnimalMemoryInit(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[5];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    u8 impression = 0x32;
    u16 count = 0;
    u8 friendship = 0;
    fn_800F0188(&idx, &kind, &impression, &count, &friendship, buf);
    dPersonalID_c *pid;
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayerRaw(member);
            if (player != NULL) {
                pid = &player->mPID;
                if (pid->isValid()) {
                    memory->init(pid, friendship, count, NULL, NULL);
                    memory->mImpression = impression;
                }
            }
        }
    }
}

// 800E0620: NET_PACKET_ANIMAL_MEMORY_SET (0x5A)
void recvAnimalMemorySet(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    u8 buf[5];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    u8 impression = 0x32;
    u16 count = 0;
    u8 friendship = 0;
    fn_800F0188(&idx, &kind, &impression, &count, &friendship, buf);
    dPersonalID_c *pid;
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayerRaw(member);
            if (player != NULL) {
                pid = &player->mPID;
                if (pid->isValid()) {
                    memory->set(friendship, count, pid, NULL, NULL);
                    memory->mImpression = impression;
                }
            }
        }
    }
}

// 800E0714: NET_PACKET_ANIMAL_FRIENDSHIP (0x5B)
void recvAnimalFriendship(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    u8 value = 0;
    fn_800F0348(&idx, &kind, &value, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->setFriendship(value);
        }
    }
}

// 800E07BC: NET_PACKET_ANIMAL_MEMORY_FLAG_ON (0x5C)
void recvAnimalMemoryFlagOn(int member, const u8 *data, u32 size, u32 arg,
                            u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    int bit = 0;
    fn_800F0494(&idx, &kind, &bit, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->onEventFlag(bit);
        }
    }
}

// 800E0860: NET_PACKET_ANIMAL_MEMORY_FLAG_OFF (0x5D)
void recvAnimalMemoryFlagOff(int member, const u8 *data, u32 size, u32 arg,
                             u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    int bit = 0;
    fn_800F0494(&idx, &kind, &bit, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->offEventFlag(bit);
        }
    }
}

// 800E0904: NET_PACKET_ANIMAL_EVENT_TALKED (0x5E)
void recvAnimalEventTalked(int member, const u8 *data, u32 size, u32 arg,
                           u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    fn_800F0580(&idx, &kind, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->mFlags.mEventTalked = 1;
        }
    }
}

// 800E099C: NET_PACKET_HIDE_AND_SEEK_CLEAR (0x64)
void recvHideAndSeekClear(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 value = 0;
    cLib::memCpy(&value, data, sizeof(value));
    dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek.clearInfo();
}

// 800E09DC: NET_PACKET_HIDE_AND_SEEK_STATE (0x65)
void recvHideAndSeekState(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 value = 0;
    cLib::memCpy(&value, data, sizeof(value));
    dQuestPlayerAnimal_c *quest =
        &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
    if (quest->mBase.isActive()) {
        quest->mBase.mState = value;
    }
}

// 800E0A34: NET_PACKET_HIDE_AND_SEEK_FLAG (0x66)
void recvHideAndSeekFlag(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    u8 value = 3;
    cLib::memCpy(&value, data, sizeof(value));
    dQuestPlayerAnimal_c *quest =
        &dSaveData_c::getTown()->mAnimals.mTown.mHideAndSeek;
    if (quest->mBase.isActive()) {
        quest->setFlag(value);
    }
}

// 800E0A94: NET_PACKET_ANIMAL_FTR_TOUCH (0x69)
void recvAnimalFtrTouch(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    u8 roomScene = dScene::SCENE_NUM;
    int x = -1;
    int z = -1;
    fn_800F0770(&roomScene, &x, &z, buf);
    if (roomScene < dScene::SCENE_NUM && roomScene == dScene::getCurrentScene()) {
        int handle = dFTR::getUnitMap()->getHandle(x, z, 0);
        if (handle != -1) {
            dFTR::sendEvent(handle);
        }
    }
}

// 800E0B24: NET_PACKET_ANIMAL_FTR (0x6A)
void recvAnimalFtr(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    int slot = 10;
    fn_800F08EC(&idx, &item, &slot, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid() && slot < 10U) {
        if (item == dItem::ITEM_ID_NONE) {
            animal->clearFtr(slot);
            animal->clearFtrBoxed(slot);
        } else {
            animal->setFtr(slot, &item);
        }
    }
}

// 800E0BF0: NET_PACKET_ANIMAL_PLACE (0x6B)
void recvAnimalPlace(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int place = 5;
    fn_800F0A3C(&idx, &place, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid() && place < 5U) {
        animal->mPlace = place;
    }
}

// 800E0C80: NET_PACKET_ANIMAL_SPOT (0x6C)
void recvAnimalSpot(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int x = -1;
    int z = -1;
    u32 type = 4;
    fn_800F0AFC(&idx, &x, &z, &type, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid() && type < 4 && x != -1 &&
        z != -1) {
        int spotX = x;
        int spotZ = z;
        animal->mSpot.mX = spotX;
        animal->mSpot.mZ = spotZ;
        animal->mSpot.mType = type;
        animal->mSpot.mGroup = 1;
    }
}

// 800E0D4C: NET_PACKET_ANIMAL_SPOT_TIME (0x6D)
void recvAnimalSpotTime(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 buf[10];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    u32 group = 2;
    s64 time = 0;
    fn_800F0BFC(&idx, &group, &time, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid() && group < 2) {
        animal->mSpot.mGroup = group;
        animal->mSpot.setTimeRaw(&time);
    }
}

// 800E0DF8: NET_PACKET_ANIMAL_SPOT_CLEAR (0x6E)
void recvAnimalSpotClear(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    u8 buf[1];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    fn_800F0CE4(&idx, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->mSpot.clear();
    }
}

// 800E0E74: NET_PACKET_ANIMAL_FTR_SET (0x6F)
void recvAnimalFtrSet(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int slot = 10;
    dItem::Item item;
    fn_800F0D74(&idx, &slot, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->setFtr(slot, &item);
    }
}

// 800E0F14: NET_PACKET_ANIMAL_NEW_ITEM_ADD (0x70)
void recvAnimalNewItemAdd(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F0E5C(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->addNewItem(&item);
    }
}

// 800E0FA4: NET_PACKET_ANIMAL_NEW_ITEM_CLEAR (0x71)
void recvAnimalNewItemClear(int member, const u8 *data, u32 size, u32 arg,
                            u8 scene) {
    u8 buf[1];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    fn_800F0F14(&idx, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->clearNewItems();
    }
}

// 800E1020: NET_PACKET_ANIMAL_NEW_ITEM_REMOVE (0x72)
void recvAnimalNewItemRemove(int member, const u8 *data, u32 size, u32 arg,
                             u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F0FA4(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->removeNewItem(&item);
    }
}

// 800E10B0: NET_PACKET_ANIMAL_WALL (0x73)
void recvAnimalWall(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F105C(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->setWall(&item);
    }
}

// 800E1140: NET_PACKET_ANIMAL_CARPET (0x74)
void recvAnimalCarpet(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F1114(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->setCarpet(&item);
    }
}

// 800E11D0: NET_PACKET_ANIMAL_MUSIC (0x75)
void recvAnimalMusic(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F11CC(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->setMusic(&item);
    }
}

// 800E1260: NET_PACKET_ANIMAL_QUEST_STATE (0x76)
void recvAnimalQuestState(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = 0x15;
    u8 state = 0;
    fn_800F1284(&idx, &kind, &state, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        dQuestBase_c *quest = &animal->mQuest.mQuest.mBase;
        if (quest->isActive() && kind == quest->getKind()) {
            switch (kind) {
            case 2:
                if (quest->mState < state) {
                    quest->mState = state;
                }
                break;
            case 4:
                if (quest->mState < state) {
                    quest->mState = state;
                }
                break;
            case 6:
                if (quest->mState < state) {
                    quest->mState = state;
                }
                break;
            }
        }
    }
}

// 800E1378: NET_PACKET_ANIMAL_QUEST_CLEAR (0x77)
void recvAnimalQuestClear(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[1];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    fn_800F1360(&idx, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->clearQuestStarted();
    }
}

// 800E13F4: NET_PACKET_ANIMAL_PLACE_TIME_CLEAR (0x78)
void recvAnimalPlaceTimeClear(int member, const u8 *data, u32 size, u32 arg,
                              u8 scene) {
    u8 buf[1];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    fn_800F13F0(&idx, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->clearPlaceChangeTime();
    }
}

// 800E1470: NET_PACKET_ANIMAL_PRESENT (0x79)
void recvAnimalPresent(int member, const u8 *data, u32 size, u32 arg,
                       u8 scene) {
    u8 buf[4];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    int kind = -1;
    dItem::Item item;
    fn_800F1480(&idx, &kind, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        dAnimalMemory_c *memory = animal->getMemory(kind);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->setPresent(&item);
        }
    }
}

// 800E1530: NET_PACKET_PERMIT_STATE (0x7A)
void recvPermitState(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    int state = 4;
    fn_800F159C(&key, &state, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL) {
        permit->setState(state);
        int me = getMyNo();
        if (state == me) {
            permit->setFlag(me);
            permit->setFlag(member);
            if (permit->fn_800F5094()) {
                permit->mFlags = 0;
            }
        } else if (state < 4U) {
            fn_800F1738(&key, state);
        }
    }
}

// 800E1604: NET_PACKET_PERMIT_SET_STATE (0x7B)
void recvPermitSetState(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    int state = 4;
    fn_800F159C(&key, &state, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL) {
        permit->setState(state);
    }
}

// 800E1668: NET_PACKET_PERMIT_FLAG (0x7C)
void recvPermitFlag(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    fn_800F1728(&key, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL) {
        permit->setFlag(member);
        if (permit->fn_800F5094()) {
            permit->mFlags = 0;
        }
    }
}

// 800E16F0: NET_PACKET_PERMIT_QUERY (0x7D)
void recvPermitQuery(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    fn_800F1728(&key, buf);
    if (fn_800F54E0(&key) != NULL) {
        fn_800F1838(&key, member);
    }
}

// 800E1758: NET_PACKET_PERMIT_ANSWER (0x7E)
void recvPermitAnswer(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    fn_800F1728(&key, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL) {
        int me = getMyNo();
        if (me == permit->mState) {
            permit->setFlag(member);
            if (permit->fn_800F5094()) {
                fn_800F18D0(&key, me);
                permit->clear();
            }
        }
    }
}

// 800E180C: NET_PACKET_PERMIT_CLEAR_STATE (0x7F)
void recvPermitClearState(int member, const u8 *data, u32 size, u32 arg,
                          u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    int state = 4;
    fn_800F159C(&key, &state, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL && permit->mState == state) {
        permit->clear();
    }
}

// 800E187C: NET_PACKET_PERMIT_SET2 (0x80)
void recvPermitSet2(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    int state = 4;
    fn_800F159C(&key, &state, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL && !permit->hasState()) {
        permit->set2(state);
    }
}

// 800E18FC: NET_PACKET_PERMIT_CLEAR (0x81)
void recvPermitClear(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[2];
    cLib::memCpy(buf, data, sizeof(buf));
    dItem::Item key;
    fn_800F1728(&key, buf);
    dNpcPermit_c *permit = fn_800F54E0(&key);
    if (permit != NULL) {
        permit->clear();
    }
}

// 800E1950: NET_PACKET_ANIMAL_CLOTH (0x82)
void recvAnimalCloth(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    u8 buf[3];
    cLib::memCpy(buf, data, sizeof(buf));
    int idx = -1;
    dItem::Item item;
    fn_800F1A28(&idx, &item, buf);
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        animal->setCloth(&item);
    }
}

// 800E19E0: NET_PACKET_COCKROACH_REMOVE (0x58)
#ifndef BUGFIXES
void recvCockroachRemove(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    u8 value = 0;
    cLib::memCpy(&value, data, sizeof(value));
    int home = value >> 6;
    int room = (value >> 4) & 3;
    int flag = value & 0xF;
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mHomes.getHome(home)->getRoom(room)->clearFlag(flag);
    dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
    u8 num = h->getRoom(1)->countFlags() + h->getRoom(0)->countFlags() +
             h->getRoom(2)->countFlags();
    if (num == 0) {
        h->_15BA = 30;
    }
}
#else
void recvCockroachRemove(int member, const u8 *data, u32 size, u32 arg,
                         u8 scene) {
    if (data == NULL || size != 1) {
        return;
    }
    u8 value = *data;
    int home = value >> 6;
    int room = (value >> 4) & 3;
    int flag = value & 0xF;
    if (home >= PLAYER_NUM || room >= HOME_ROOM_NUM || flag >= 3) {
        return;
    }
    dHome_c *h = dSaveData_c::getTown()->mHomes.getHome(home);
    h->getRoom(room)->clearFlag(flag);
    u8 num = h->getRoom(1)->countFlags() + h->getRoom(0)->countFlags() +
             h->getRoom(2)->countFlags();
    if (num == 0) {
        h->_15BA = 30;
    }
}
#endif

// 800E1AC0: NET_PACKET_GULLIVER_FLAG (0x5F)
void recvGulliverFlag(int member, const u8 *data, u32 size, u32 arg, u8 scene) {
    dSaveData_c::getTown()->setFlag(4);
}

// 800E1AE8: NET_PACKET_GULLIVER_PLAYER (0x60)
void recvGulliverPlayer(int member, const u8 *data, u32 size, u32 arg,
                        u8 scene) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayerRaw(member);
    dSaveData_c::getTown()->_072CEE.copy(&player->mPID);
}

#ifdef BUGFIXES
// Minimum payloads for handlers that read fixed fields before using size.
BOOL isNetBlockPayloadValid(int id, const u8 *data, u32 size) {
    u32 minimum = 0;
    switch (id) {
    case NET_PACKET_JOIN_REQUEST:
    case NET_PACKET_JOIN_SAVE_REPLY:
    case NET_PACKET_JOIN_SAVE_PROMPT:
    case NET_PACKET_JOIN_RESULT:
    case NET_PACKET_POST_SYNC:
    case NET_PACKET_NOTICE_REMOVE:
    case NET_PACKET_DAILY_RAND:
    case NET_PACKET_PLAYER_MNGR_24:
    case NET_PACKET_STR_LOCK:
    case NET_PACKET_STR_UNLOCK:
    case NET_PACKET_PATTERN_COPY_ACK:
    case NET_PACKET_COCKROACH_REMOVE:
    case NET_PACKET_HIDE_AND_SEEK_CLEAR:
    case NET_PACKET_HIDE_AND_SEEK_STATE:
    case NET_PACKET_HIDE_AND_SEEK_FLAG:
    case NET_PACKET_ANIMAL_SPOT_CLEAR:
    case NET_PACKET_ANIMAL_NEW_ITEM_CLEAR:
    case NET_PACKET_ANIMAL_QUEST_CLEAR:
    case NET_PACKET_ANIMAL_PLACE_TIME_CLEAR:
        minimum = 1;
        break;
    case NET_PACKET_SYNC_BUF_MEMBER:
    case NET_PACKET_FTR_ACTION2:
    case NET_PACKET_FTR_OWNER:
    case NET_PACKET_NPC_SPOT_B:
    case NET_PACKET_HARVEST_SPOT:
    case NET_PACKET_ANIMAL_HELD_ITEM_TIME:
    case NET_PACKET_ANIMAL_EVENT_FLAGS:
    case NET_PACKET_ANIMAL_EVENT_TALKED:
    case NET_PACKET_ANIMAL_PLACE:
    case NET_PACKET_PERMIT_FLAG:
    case NET_PACKET_PERMIT_QUERY:
    case NET_PACKET_PERMIT_ANSWER:
    case NET_PACKET_PERMIT_CLEAR:
        minimum = 2;
        break;
    case NET_PACKET_PLAYER_MNGR_25:
    case NET_PACKET_PLAYER_MNGR_26:
    case NET_PACKET_FTR_HOLD_REQUEST:
    case NET_PACKET_ANIMAL_HELD_ITEM:
    case NET_PACKET_ANIMAL_FRIENDSHIP:
    case NET_PACKET_ANIMAL_MEMORY_FLAG_ON:
    case NET_PACKET_ANIMAL_MEMORY_FLAG_OFF:
    case NET_PACKET_ANIMAL_FTR_TOUCH:
    case NET_PACKET_ANIMAL_NEW_ITEM_ADD:
    case NET_PACKET_ANIMAL_NEW_ITEM_REMOVE:
    case NET_PACKET_ANIMAL_WALL:
    case NET_PACKET_ANIMAL_CARPET:
    case NET_PACKET_ANIMAL_MUSIC:
    case NET_PACKET_ANIMAL_QUEST_STATE:
    case NET_PACKET_PERMIT_STATE:
    case NET_PACKET_PERMIT_SET_STATE:
    case NET_PACKET_PERMIT_CLEAR_STATE:
    case NET_PACKET_PERMIT_SET2:
    case NET_PACKET_ANIMAL_CLOTH:
        minimum = 3;
        break;
    case NET_PACKET_FG_MNG_UNIT_FG:
    case NET_PACKET_FTR_ACTION:
    case NET_PACKET_FTR_PUT:
    case NET_PACKET_FTR_REMOVE:
    case NET_PACKET_FTR_SET_UNIT:
    case NET_PACKET_PATTERN_COPY:
    case NET_PACKET_NPC_SPOT_A:
    case NET_PACKET_ANIMAL_FTR:
    case NET_PACKET_ANIMAL_SPOT:
    case NET_PACKET_ANIMAL_FTR_SET:
    case NET_PACKET_ANIMAL_PRESENT:
    case NET_PACKET_INSECT_FIELD:
        minimum = 4;
        break;
    case NET_PACKET_ANIMAL_MEMORY_INIT:
    case NET_PACKET_ANIMAL_MEMORY_SET:
        minimum = 5;
        break;
    case NET_PACKET_ANIMAL_SPOT_TIME:
        minimum = 10;
        break;
    case NET_PACKET_PLAYER_STATE:
        minimum = 14;
        break;
    case NET_PACKET_NPC_EVENT_DATA:
        minimum = 0x28;
        break;
    case NET_PACKET_RECYCLE_BIN:
        minimum = RECYCLE_BIN_ITEM_NUM * sizeof(u16);
        break;
    case NET_PACKET_MELODY:
        minimum = 16;
        break;
    case NET_PACKET_MAIL:
        minimum = sizeof(dMail_c);
        break;
    case NET_PACKET_NOTICE_ADD:
        minimum = sizeof(dNotice_c);
        break;
    case NET_PACKET_FISHING_RECORD:
        minimum = sizeof(((dSaveTown_c *)0)->mFishing);
        break;
    case NET_PACKET_BUG_OFF:
        minimum = sizeof(((dSaveTown_c *)0)->mBugOff);
        break;
    case NET_PACKET_SCENE_REQUEST:
        minimum = sizeof(dNetSceneReq_c);
        break;
    case NET_PACKET_SCENE_LEADER:
        minimum = sizeof(dNetMemberScene_c);
        break;
    case NET_PACKET_SCENE_STATE:
        minimum = sizeof(dNetSceneMsg_c);
        break;
    case NET_PACKET_SYNC_BUF:
        minimum = sizeof(dNetMemberMask_c);
        break;
    case NET_PACKET_FG_MNG_REQ:
        minimum = sizeof(dFgMngReq_c);
        break;
    case NET_PACKET_FG_MNG_CMD:
        minimum = sizeof(dFgMngCmd_c);
        break;
    case NET_PACKET_FG_MNG_TASK_DONE:
        minimum = sizeof(dFgMngTaskMsg_c);
        break;
    case NET_PACKET_ROOM_ITEM:
        minimum = sizeof(dBgUtil::roomItemMsg_c);
        break;
    case NET_PACKET_PLAY_SYNC:
        minimum = sizeof(dPlaySyncMsg_c);
        break;
    case NET_PACKET_SHOP_SALE:
        minimum = sizeof(dShopSale_c);
        break;
    default:
        break;
    }
    if (size < minimum) {
        return FALSE;
    }
    if (id == NET_PACKET_PLAY_SYNC) {
        dPlaySyncMsg_c msg;
        cLib::memCpy(&msg, data, sizeof(msg));
        return msg.getIdx() < PLAY_SYNC_REC_NUM && msg.getType() <= 4;
    }
    if (id == NET_PACKET_COCKROACH_REMOVE) {
        u8 value = data[0];
        return size == 1 && (value >> 6) < PLAYER_NUM &&
               ((value >> 4) & 3) < HOME_ROOM_NUM && (value & 0xF) < 3;
    }
    return TRUE;
}
#endif

} // namespace dNet

// The receive handler kinds of lbl_80473E10 (getDirectSeqMode).
const s8 lbl_80473E00[NET_DIRECT_NUM] = {-1, 0, 1, 1, 1, 1, 1,
                                         1,  1, 1, 1, 1, 1};

// The player / time handlers (callDirectHandler).
const dNetRecvFunc lbl_80473E10[NET_DIRECT_NUM] = {
    recvDirectHello,           recvDirectHelloAck,
    recvDirectJoinRequest,     recvDirectJoinResult,
    recvDirectXferStart,       recvDirectXferPlayer,
    recvDirectXferItemMask,    recvDirectXferTown,
    recvDirectXferPlayerReply, recvDirectXferItems,
    recvDirectXferTownRequest, recvDirectJoinDoneHost,
    recvDirectJoinDone,
};

// The receive handlers by packet id (callBlockHandlers).
const dNetDataFunc lbl_80473E44[NET_PACKET_NUM] = {
    recvNone,
    recvJoinDone,
    recvJoinDoneHost,
    recvJoinRequest,
    recvJoinSaveReply,
    recvJoinSavePrompt,
    recvJoinSaveCancel,
    recvJoinCancelDone,
    recvJoinResult,
    recvJoinRelease,
    recvJoinCommit,
    recvSceneRequest,
    recvSceneLeader,
    recvSceneNoLeader,
    recvSceneLeaderAck,
    recvSceneState,
    recvSceneStateAck,
    recvSceneSaveHost,
    recvSceneLoadRequest,
    recvSceneLoad,
    recvSceneExitRequest,
    recvSceneExitAck,
    recvSceneExitData,
    recvSceneEnterRequest,
    recvSceneEnterData,
    recvSceneHandoverRequest,
    recvSceneHandoverAck,
    recvSceneHandoverData,
    recvSaveDone,
    recvSaveDoneAll,
    recvSyncBufMember,
    recvSyncBuf,
    recvDisconnectStart,
    recvDisconnectAck,
    recvDisconnectReady,
    recvDisconnectGo,
    recvPlayerMngr24,
    recvPlayerMngr25,
    recvPlayerMngr26,
    recvPlayerState,
    recvFgMngReq,
    recvFgMngCmd,
    recvFgMngTaskDone,
    recvFgMngUnitFg,
    recvDemoSync,
    recvPostSync,
    recvMail,
    recvNoticeAdd,
    recvNoticeRemove,
    recvStrLock,
    recvStrUnlock,
    recvFtrHoldRequest,
    recvFtrHoldOk,
    recvFtrHoldNg,
    recvFtrHoldRelease,
    recvFtrAction,
    recvFtrAction2,
    recvFtrPut,
    recvFtrRemove,
    recvFtrSetUnit,
    recvFtrOwner,
    recvRoomItem,
    recvPatternCopy,
    recvPatternCopyAck,
    recvInsectField,
    recvPlaySync,
    recvShopSale,
    recvFishingRecord,
    recvBugOff,
    recvNpcEventStart,
    recvNpcEventData,
    recvNpcEventJoin,
    recvNpcEventPlay,
    recvNpcEventEnd,
    recvNpcEventLeave,
    recvNpcSpotAFlag,
    recvNpcSpotA,
    recvNpcSpotBFlag,
    recvNpcSpotB,
    recvHarvestSpotFlag,
    recvHarvestSpot,
    recvDailyRand,
    recvChat,
    recvChatPreset,
    recvClock,
    recvAnimalHeldItem,
    recvAnimalHeldItemTime,
    recvAnimalEventFlags,
    recvCockroachRemove,
    recvAnimalMemoryInit,
    recvAnimalMemorySet,
    recvAnimalFriendship,
    recvAnimalMemoryFlagOn,
    recvAnimalMemoryFlagOff,
    recvAnimalEventTalked,
    recvGulliverFlag,
    recvGulliverPlayer,
    recvVoice,
    recvRecycleBin,
    recvRecycleBinAck,
    recvHideAndSeekClear,
    recvHideAndSeekState,
    recvHideAndSeekFlag,
    recvMelody,
    recvMelodyAck,
    recvAnimalFtrTouch,
    recvAnimalFtr,
    recvAnimalPlace,
    recvAnimalSpot,
    recvAnimalSpotTime,
    recvAnimalSpotClear,
    recvAnimalFtrSet,
    recvAnimalNewItemAdd,
    recvAnimalNewItemClear,
    recvAnimalNewItemRemove,
    recvAnimalWall,
    recvAnimalCarpet,
    recvAnimalMusic,
    recvAnimalQuestState,
    recvAnimalQuestClear,
    recvAnimalPlaceTimeClear,
    recvAnimalPresent,
    recvPermitState,
    recvPermitSetState,
    recvPermitFlag,
    recvPermitQuery,
    recvPermitAnswer,
    recvPermitClearState,
    recvPermitSet2,
    recvPermitClear,
    recvAnimalCloth,
};

// The shared record handlers (lbl_80474050 copy, lbl_80474338 clear), the
// session event hooks and the data channel handlers, then their tables
// (declared in d_net.hpp).

// Copy handler of a record whose data is its whole size.
#define D_NET_COPY_REC(name, id)                                               \
    void name(void *dst, const void *src, int) {                               \
        cLib::memCpy(dst, src, getRecordSize(id));                             \
    }

// Copy handler of a daub record: the daub's current data (src is NULL).
#define D_NET_COPY_DAUB(name, id)                                              \
    void name(void *dst, const void *, int) {                                  \
        fn_800EFEDC(dst, id);                                                  \
    }

// The source of record 3 (10 bytes): three values.
struct dNetRec3Src_c {
    /* 0x0 */ const u32 *m0;
    /* 0x4 */ const u32 *m4;
    /* 0x8 */ const u16 *m8;
}; // size 0xC

// The source of records 4..7 (8 bytes): its first and last word.
struct dNetRecMemberASrc_c {
    /* 0x0 */ u32 m0;
    /* 0x4 */ u32 m4;
    /* 0x8 */ u32 m8;
}; // size 0xC

namespace dNet {

D_NET_COPY_REC(copyRec00, 0)
D_NET_COPY_REC(copyRec01, 1)
D_NET_COPY_REC(copyRec02, 2)

void copyRec03(void *dst, const void *src, int) {
    const dNetRec3Src_c *s = (const dNetRec3Src_c *)src;
    cLib::memCpy(dst, s->m0, 4);
    cLib::memCpy((u8 *)dst + 4, s->m4, 4);
    cLib::memCpy((u8 *)dst + 8, s->m8, 2);
}

void copyRecMemberA(void *dst, const void *src) {
    const dNetRecMemberASrc_c *s = (const dNetRecMemberASrc_c *)src;
    u8 *p = (u8 *)dst;
    cLib::memCpy(p, &s->m0, 4);
    cLib::memCpy(p + 4, &s->m8, 4);
}

void copyRecMemberA0(void *dst, const void *src, int) {
    copyRecMemberA(dst, src);
}
void copyRecMemberA1(void *dst, const void *src, int) {
    copyRecMemberA(dst, src);
}
void copyRecMemberA2(void *dst, const void *src, int) {
    copyRecMemberA(dst, src);
}
void copyRecMemberA3(void *dst, const void *src, int) {
    copyRecMemberA(dst, src);
}

void copyRecMemberB(void *dst, const void *src) {
    cLib::memCpy(dst, src, 2);
}

void copyRecMemberB0(void *dst, const void *src, int) {
    copyRecMemberB(dst, src);
}
void copyRecMemberB1(void *dst, const void *src, int) {
    copyRecMemberB(dst, src);
}
void copyRecMemberB2(void *dst, const void *src, int) {
    copyRecMemberB(dst, src);
}
void copyRecMemberB3(void *dst, const void *src, int) {
    copyRecMemberB(dst, src);
}

void copyRecBySize(void *dst, const void *src, int id) {
    cLib::memCpy(dst, src, getRecordSize(id));
}

void copyRecMemberC0(void *dst, const void *src, int) {
    copyRecBySize(dst, src, NET_REC_MEMBER_C + 0);
}
void copyRecMemberC1(void *dst, const void *src, int) {
    copyRecBySize(dst, src, NET_REC_MEMBER_C + 1);
}
void copyRecMemberC2(void *dst, const void *src, int) {
    copyRecBySize(dst, src, NET_REC_MEMBER_C + 2);
}
void copyRecMemberC3(void *dst, const void *src, int) {
    copyRecBySize(dst, src, NET_REC_MEMBER_C + 3);
}

D_NET_COPY_REC(copyRecInsect0, NET_REC_INSECT + 0)
D_NET_COPY_REC(copyRecInsect1, NET_REC_INSECT + 1)
D_NET_COPY_REC(copyRecInsect2, NET_REC_INSECT + 2)
D_NET_COPY_REC(copyRecInsect3, NET_REC_INSECT + 3)
D_NET_COPY_REC(copyRecInsect4, NET_REC_INSECT + 4)
D_NET_COPY_REC(copyRecInsect5, NET_REC_INSECT + 5)
D_NET_COPY_REC(copyRecInsect6, NET_REC_INSECT + 6)
D_NET_COPY_REC(copyRecInsect7, NET_REC_INSECT + 7)
D_NET_COPY_REC(copyRecInsect8, NET_REC_INSECT + 8)
D_NET_COPY_REC(copyRecInsect9, NET_REC_INSECT + 9)
D_NET_COPY_REC(copyRecInsect10, NET_REC_INSECT + 10)
D_NET_COPY_REC(copyRecInsect11, NET_REC_INSECT + 11)
D_NET_COPY_REC(copyRecPlaySync0, PLAY_SYNC_REC_ID + 0)
D_NET_COPY_REC(copyRecPlaySync1, PLAY_SYNC_REC_ID + 1)
D_NET_COPY_REC(copyRecPlaySync2, PLAY_SYNC_REC_ID + 2)
D_NET_COPY_REC(copyRecPlaySync3, PLAY_SYNC_REC_ID + 3)
D_NET_COPY_REC(copyRecPlaySync4, PLAY_SYNC_REC_ID + 4)
D_NET_COPY_REC(copyRecPlaySync5, PLAY_SYNC_REC_ID + 5)
D_NET_COPY_REC(copyRecPlaySync6, PLAY_SYNC_REC_ID + 6)
D_NET_COPY_REC(copyRecPlaySync7, PLAY_SYNC_REC_ID + 7)
D_NET_COPY_REC(copyRec24, 0x24)
D_NET_COPY_REC(copyRec25, 0x25)
D_NET_COPY_REC(copyRec26, 0x26)
D_NET_COPY_REC(copyRec27, 0x27)
D_NET_COPY_REC(copyRec28, 0x28)
D_NET_COPY_REC(copyRec29, 0x29)
D_NET_COPY_REC(copyRec2A, 0x2A)

D_NET_COPY_DAUB(copyRecDaub0, NET_REC_DAUB + 0)
D_NET_COPY_DAUB(copyRecDaub1, NET_REC_DAUB + 1)
D_NET_COPY_DAUB(copyRecDaub2, NET_REC_DAUB + 2)
D_NET_COPY_DAUB(copyRecDaub3, NET_REC_DAUB + 3)
D_NET_COPY_DAUB(copyRecDaub4, NET_REC_DAUB + 4)
D_NET_COPY_DAUB(copyRecDaub5, NET_REC_DAUB + 5)
D_NET_COPY_DAUB(copyRecDaub6, NET_REC_DAUB + 6)
D_NET_COPY_DAUB(copyRecDaub7, NET_REC_DAUB + 7)
D_NET_COPY_DAUB(copyRecDaub8, NET_REC_DAUB + 8)
D_NET_COPY_DAUB(copyRecDaub9, NET_REC_DAUB + 9)
D_NET_COPY_DAUB(copyRecDaub10, NET_REC_DAUB + 10)
D_NET_COPY_DAUB(copyRecDaub11, NET_REC_DAUB + 11)
D_NET_COPY_DAUB(copyRecDaub12, NET_REC_DAUB + 12)
D_NET_COPY_DAUB(copyRecDaub13, NET_REC_DAUB + 13)
D_NET_COPY_DAUB(copyRecDaub14, NET_REC_DAUB + 14)
D_NET_COPY_DAUB(copyRecDaub15, NET_REC_DAUB + 15)
D_NET_COPY_DAUB(copyRecDaub16, NET_REC_DAUB + 16)
D_NET_COPY_DAUB(copyRecDaub17, NET_REC_DAUB + 17)
D_NET_COPY_DAUB(copyRecDaub18, NET_REC_DAUB + 18)
D_NET_COPY_DAUB(copyRecDaub19, NET_REC_DAUB + 19)
D_NET_COPY_DAUB(copyRecDaub20, NET_REC_DAUB + 20)
D_NET_COPY_DAUB(copyRecDaub21, NET_REC_DAUB + 21)
D_NET_COPY_DAUB(copyRecDaub22, NET_REC_DAUB + 22)
D_NET_COPY_DAUB(copyRecDaub23, NET_REC_DAUB + 23)
D_NET_COPY_DAUB(copyRecDaub24, NET_REC_DAUB + 24)
D_NET_COPY_DAUB(copyRecDaub25, NET_REC_DAUB + 25)
D_NET_COPY_DAUB(copyRecDaub26, NET_REC_DAUB + 26)
D_NET_COPY_DAUB(copyRecDaub27, NET_REC_DAUB + 27)
D_NET_COPY_DAUB(copyRecDaub28, NET_REC_DAUB + 28)
D_NET_COPY_DAUB(copyRecDaub29, NET_REC_DAUB + 29)
D_NET_COPY_DAUB(copyRecDaub30, NET_REC_DAUB + 30)
D_NET_COPY_DAUB(copyRecDaub31, NET_REC_DAUB + 31)
D_NET_COPY_DAUB(copyRecDaub32, NET_REC_DAUB + 32)
D_NET_COPY_DAUB(copyRecDaub33, NET_REC_DAUB + 33)
D_NET_COPY_DAUB(copyRecDaub34, NET_REC_DAUB + 34)
D_NET_COPY_DAUB(copyRecDaub35, NET_REC_DAUB + 35)
D_NET_COPY_DAUB(copyRecDaub36, NET_REC_DAUB + 36)
D_NET_COPY_DAUB(copyRecDaub37, NET_REC_DAUB + 37)
D_NET_COPY_DAUB(copyRecDaub38, NET_REC_DAUB + 38)
D_NET_COPY_DAUB(copyRecDaub39, NET_REC_DAUB + 39)
D_NET_COPY_DAUB(copyRecDaub40, NET_REC_DAUB + 40)
D_NET_COPY_DAUB(copyRecDaub41, NET_REC_DAUB + 41)
D_NET_COPY_DAUB(copyRecDaub42, NET_REC_DAUB + 42)
D_NET_COPY_DAUB(copyRecDaub43, NET_REC_DAUB + 43)
D_NET_COPY_DAUB(copyRecDaub44, NET_REC_DAUB + 44)
D_NET_COPY_DAUB(copyRecDaub45, NET_REC_DAUB + 45)
D_NET_COPY_DAUB(copyRecDaub46, NET_REC_DAUB + 46)
D_NET_COPY_DAUB(copyRecDaub47, NET_REC_DAUB + 47)
D_NET_COPY_DAUB(copyRecDaub48, NET_REC_DAUB + 48)
D_NET_COPY_DAUB(copyRecDaub49, NET_REC_DAUB + 49)
D_NET_COPY_DAUB(copyRecDaub50, NET_REC_DAUB + 50)
D_NET_COPY_DAUB(copyRecDaub51, NET_REC_DAUB + 51)
D_NET_COPY_DAUB(copyRecDaub52, NET_REC_DAUB + 52)
D_NET_COPY_DAUB(copyRecDaub53, NET_REC_DAUB + 53)
D_NET_COPY_DAUB(copyRecDaub54, NET_REC_DAUB + 54)
D_NET_COPY_DAUB(copyRecDaub55, NET_REC_DAUB + 55)
D_NET_COPY_DAUB(copyRecDaub56, NET_REC_DAUB + 56)
D_NET_COPY_DAUB(copyRecDaub57, NET_REC_DAUB + 57)
D_NET_COPY_DAUB(copyRecDaub58, NET_REC_DAUB + 58)
D_NET_COPY_DAUB(copyRecDaub59, NET_REC_DAUB + 59)
D_NET_COPY_DAUB(copyRecDaub60, NET_REC_DAUB + 60)
D_NET_COPY_DAUB(copyRecDaub61, NET_REC_DAUB + 61)
D_NET_COPY_DAUB(copyRecDaub62, NET_REC_DAUB + 62)
D_NET_COPY_DAUB(copyRecDaub63, NET_REC_DAUB + 63)
D_NET_COPY_DAUB(copyRecDaub64, NET_REC_DAUB + 64)
D_NET_COPY_DAUB(copyRecDaub65, NET_REC_DAUB + 65)
D_NET_COPY_DAUB(copyRecDaub66, NET_REC_DAUB + 66)
D_NET_COPY_DAUB(copyRecDaub67, NET_REC_DAUB + 67)
D_NET_COPY_DAUB(copyRecDaub68, NET_REC_DAUB + 68)
D_NET_COPY_DAUB(copyRecDaub69, NET_REC_DAUB + 69)
D_NET_COPY_DAUB(copyRecDaub70, NET_REC_DAUB + 70)
D_NET_COPY_DAUB(copyRecDaub71, NET_REC_DAUB + 71)
D_NET_COPY_DAUB(copyRecDaub72, NET_REC_DAUB + 72)
D_NET_COPY_DAUB(copyRecDaub73, NET_REC_DAUB + 73)
D_NET_COPY_DAUB(copyRecDaub74, NET_REC_DAUB + 74)
D_NET_COPY_DAUB(copyRecDaub75, NET_REC_DAUB + 75)
D_NET_COPY_DAUB(copyRecDaub76, NET_REC_DAUB + 76)
D_NET_COPY_DAUB(copyRecDaub77, NET_REC_DAUB + 77)
D_NET_COPY_DAUB(copyRecDaub78, NET_REC_DAUB + 78)
D_NET_COPY_DAUB(copyRecDaub79, NET_REC_DAUB + 79)
D_NET_COPY_DAUB(copyRecDaub80, NET_REC_DAUB + 80)
D_NET_COPY_DAUB(copyRecDaub81, NET_REC_DAUB + 81)
D_NET_COPY_DAUB(copyRecDaub82, NET_REC_DAUB + 82)
D_NET_COPY_DAUB(copyRecDaub83, NET_REC_DAUB + 83)
D_NET_COPY_DAUB(copyRecDaub84, NET_REC_DAUB + 84)
D_NET_COPY_DAUB(copyRecDaub85, NET_REC_DAUB + 85)
D_NET_COPY_DAUB(copyRecDaub86, NET_REC_DAUB + 86)
D_NET_COPY_DAUB(copyRecDaub87, NET_REC_DAUB + 87)
D_NET_COPY_DAUB(copyRecDaub88, NET_REC_DAUB + 88)
D_NET_COPY_DAUB(copyRecDaub89, NET_REC_DAUB + 89)
D_NET_COPY_DAUB(copyRecDaub90, NET_REC_DAUB + 90)
D_NET_COPY_DAUB(copyRecDaub91, NET_REC_DAUB + 91)
D_NET_COPY_DAUB(copyRecDaub92, NET_REC_DAUB + 92)
D_NET_COPY_DAUB(copyRecDaub93, NET_REC_DAUB + 93)
D_NET_COPY_DAUB(copyRecDaub94, NET_REC_DAUB + 94)
D_NET_COPY_DAUB(copyRecDaub95, NET_REC_DAUB + 95)
D_NET_COPY_DAUB(copyRecDaub96, NET_REC_DAUB + 96)
D_NET_COPY_DAUB(copyRecDaub97, NET_REC_DAUB + 97)
D_NET_COPY_DAUB(copyRecDaub98, NET_REC_DAUB + 98)
D_NET_COPY_DAUB(copyRecDaub99, NET_REC_DAUB + 99)
D_NET_COPY_DAUB(copyRecDaub100, NET_REC_DAUB + 100)
D_NET_COPY_DAUB(copyRecDaub101, NET_REC_DAUB + 101)
D_NET_COPY_DAUB(copyRecDaub102, NET_REC_DAUB + 102)
D_NET_COPY_DAUB(copyRecDaub103, NET_REC_DAUB + 103)
D_NET_COPY_DAUB(copyRecDaub104, NET_REC_DAUB + 104)
D_NET_COPY_DAUB(copyRecDaub105, NET_REC_DAUB + 105)
D_NET_COPY_DAUB(copyRecDaub106, NET_REC_DAUB + 106)

D_NET_COPY_REC(copyRecHouse0_0_0, NET_REC_HOUSE + 0)
D_NET_COPY_REC(copyRecHouse0_0_1, NET_REC_HOUSE + 1)
D_NET_COPY_REC(copyRecHouse0_0_2, NET_REC_HOUSE + 2)
D_NET_COPY_REC(copyRecHouse0_1_0, NET_REC_HOUSE + 3)
D_NET_COPY_REC(copyRecHouse0_1_1, NET_REC_HOUSE + 4)
D_NET_COPY_REC(copyRecHouse0_1_2, NET_REC_HOUSE + 5)
D_NET_COPY_REC(copyRecHouse0_2_0, NET_REC_HOUSE + 6)
D_NET_COPY_REC(copyRecHouse0_2_1, NET_REC_HOUSE + 7)
D_NET_COPY_REC(copyRecHouse0_2_2, NET_REC_HOUSE + 8)
D_NET_COPY_REC(copyRecHouse1_0_0, NET_REC_HOUSE + 9)
D_NET_COPY_REC(copyRecHouse1_0_1, NET_REC_HOUSE + 10)
D_NET_COPY_REC(copyRecHouse1_0_2, NET_REC_HOUSE + 11)
D_NET_COPY_REC(copyRecHouse1_1_0, NET_REC_HOUSE + 12)
D_NET_COPY_REC(copyRecHouse1_1_1, NET_REC_HOUSE + 13)
D_NET_COPY_REC(copyRecHouse1_1_2, NET_REC_HOUSE + 14)
D_NET_COPY_REC(copyRecHouse1_2_0, NET_REC_HOUSE + 15)
D_NET_COPY_REC(copyRecHouse1_2_1, NET_REC_HOUSE + 16)
D_NET_COPY_REC(copyRecHouse1_2_2, NET_REC_HOUSE + 17)
D_NET_COPY_REC(copyRecHouse2_0_0, NET_REC_HOUSE + 18)
D_NET_COPY_REC(copyRecHouse2_0_1, NET_REC_HOUSE + 19)
D_NET_COPY_REC(copyRecHouse2_0_2, NET_REC_HOUSE + 20)
D_NET_COPY_REC(copyRecHouse2_1_0, NET_REC_HOUSE + 21)
D_NET_COPY_REC(copyRecHouse2_1_1, NET_REC_HOUSE + 22)
D_NET_COPY_REC(copyRecHouse2_1_2, NET_REC_HOUSE + 23)
D_NET_COPY_REC(copyRecHouse2_2_0, NET_REC_HOUSE + 24)
D_NET_COPY_REC(copyRecHouse2_2_1, NET_REC_HOUSE + 25)
D_NET_COPY_REC(copyRecHouse2_2_2, NET_REC_HOUSE + 26)
D_NET_COPY_REC(copyRecHouse3_0_0, NET_REC_HOUSE + 27)
D_NET_COPY_REC(copyRecHouse3_0_1, NET_REC_HOUSE + 28)
D_NET_COPY_REC(copyRecHouse3_0_2, NET_REC_HOUSE + 29)
D_NET_COPY_REC(copyRecHouse3_1_0, NET_REC_HOUSE + 30)
D_NET_COPY_REC(copyRecHouse3_1_1, NET_REC_HOUSE + 31)
D_NET_COPY_REC(copyRecHouse3_1_2, NET_REC_HOUSE + 32)
D_NET_COPY_REC(copyRecHouse3_2_0, NET_REC_HOUSE + 33)
D_NET_COPY_REC(copyRecHouse3_2_1, NET_REC_HOUSE + 34)
D_NET_COPY_REC(copyRecHouse3_2_2, NET_REC_HOUSE + 35)

void clearRec00(void *) {
}
void clearRec02(void *) {
}

void clearRecInsect0(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect1(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect2(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect3(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect4(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect5(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect6(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect7(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect8(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect9(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect10(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}
void clearRecInsect11(void *rec) {
    dInsectInfo::initCatchData((dInsectCatchData_c *)rec);
}

void clearRecPlaySync0(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync1(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync2(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync3(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync4(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync5(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync6(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}
void clearRecPlaySync7(void *rec) {
    dFishInfo::initCatchData((dFishCatchData_c *)rec);
}

void clearRecDaub0(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 0);
}
void clearRecDaub1(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 1);
}
void clearRecDaub2(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 2);
}
void clearRecDaub3(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 3);
}
void clearRecDaub4(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 4);
}
void clearRecDaub5(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 5);
}
void clearRecDaub6(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 6);
}
void clearRecDaub7(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 7);
}
void clearRecDaub8(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 8);
}
void clearRecDaub9(void *rec) {
    fn_800EFDD0(rec, NET_REC_DAUB + 9);
}

void clearRecDaub10(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 10);
}
void clearRecDaub11(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 11);
}
void clearRecDaub12(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 12);
}
void clearRecDaub13(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 13);
}
void clearRecDaub14(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 14);
}
void clearRecDaub15(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 15);
}
void clearRecDaub16(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 16);
}
void clearRecDaub17(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 17);
}
void clearRecDaub18(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 18);
}
void clearRecDaub19(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 19);
}
void clearRecDaub20(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 20);
}
void clearRecDaub21(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 21);
}
void clearRecDaub22(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 22);
}
void clearRecDaub23(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 23);
}
void clearRecDaub24(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 24);
}
void clearRecDaub25(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 25);
}
void clearRecDaub26(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 26);
}
void clearRecDaub27(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 27);
}
void clearRecDaub28(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 28);
}
void clearRecDaub29(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 29);
}
void clearRecDaub30(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 30);
}
void clearRecDaub31(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 31);
}
void clearRecDaub32(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 32);
}
void clearRecDaub33(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 33);
}
void clearRecDaub34(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 34);
}
void clearRecDaub35(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 35);
}
void clearRecDaub36(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 36);
}
void clearRecDaub37(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 37);
}
void clearRecDaub38(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 38);
}
void clearRecDaub39(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 39);
}
void clearRecDaub40(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 40);
}
void clearRecDaub41(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 41);
}
void clearRecDaub42(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 42);
}
void clearRecDaub43(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 43);
}
void clearRecDaub44(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 44);
}
void clearRecDaub45(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 45);
}
void clearRecDaub46(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 46);
}
void clearRecDaub47(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 47);
}
void clearRecDaub48(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 48);
}
void clearRecDaub49(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 49);
}
void clearRecDaub50(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 50);
}
void clearRecDaub51(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 51);
}
void clearRecDaub52(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 52);
}
void clearRecDaub53(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 53);
}
void clearRecDaub54(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 54);
}
void clearRecDaub55(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 55);
}
void clearRecDaub56(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 56);
}
void clearRecDaub57(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 57);
}
void clearRecDaub58(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 58);
}
void clearRecDaub59(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 59);
}
void clearRecDaub60(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 60);
}
void clearRecDaub61(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 61);
}
void clearRecDaub62(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 62);
}
void clearRecDaub63(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 63);
}
void clearRecDaub64(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 64);
}
void clearRecDaub65(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 65);
}
void clearRecDaub66(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 66);
}
void clearRecDaub67(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 67);
}
void clearRecDaub68(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 68);
}
void clearRecDaub69(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 69);
}
void clearRecDaub70(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 70);
}
void clearRecDaub71(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 71);
}
void clearRecDaub72(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 72);
}
void clearRecDaub73(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 73);
}
void clearRecDaub74(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 74);
}
void clearRecDaub75(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 75);
}
void clearRecDaub76(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 76);
}
void clearRecDaub77(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 77);
}
void clearRecDaub78(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 78);
}
void clearRecDaub79(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 79);
}
void clearRecDaub80(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 80);
}
void clearRecDaub81(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 81);
}
void clearRecDaub82(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 82);
}
void clearRecDaub83(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 83);
}
void clearRecDaub84(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 84);
}
void clearRecDaub85(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 85);
}
void clearRecDaub86(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 86);
}
void clearRecDaub87(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 87);
}
void clearRecDaub88(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 88);
}
void clearRecDaub89(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 89);
}
void clearRecDaub90(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 90);
}
void clearRecDaub91(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 91);
}
void clearRecDaub92(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 92);
}
void clearRecDaub93(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 93);
}
void clearRecDaub94(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 94);
}
void clearRecDaub95(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 95);
}
void clearRecDaub96(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 96);
}
void clearRecDaub97(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 97);
}
void clearRecDaub98(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 98);
}
void clearRecDaub99(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 99);
}
void clearRecDaub100(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 100);
}
void clearRecDaub101(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 101);
}
void clearRecDaub102(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 102);
}
void clearRecDaub103(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 103);
}
void clearRecDaub104(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 104);
}
void clearRecDaub105(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 105);
}
void clearRecDaub106(void *rec) {
    fn_800EFE68(rec, NET_REC_DAUB + 106);
}

// Session event hooks (declared in d_net.hpp).
void onSessionStart() {
    fgMngSync_reset();
    fgMngTask_resetAll();
}

void onSessionEnd() {
    fgMngSync_reset();
    fgMngTask_resetAll();
}

void onVisitorClear(int, const dNetMemberMask_c *) {
    dDemoSync::init();
    dSvMgr_c::mCurrentVisitor = NET_MEMBER_NONE;
    clearAllRoomMaps();
}

void onVisitorSet(int member, const dNetMemberMask_c *) {
    dDemoSync::init();
    dSvMgr_c::mCurrentVisitor = member;
    clearAllRoomMaps();
}

BOOL canFinishLeave(int, const dNetMemberMask_c *) {
    return TRUE;
}

void onLeave(int, const dNetMemberMask_c *) {
    dDemoSync::init();
    clearAllRoomMaps();
    dFootmark::getEditor()->clearMarks2();
}

void onMemberLeft(int member, const dNetMemberMask_c *) {
    dDemoSync::init();
    clearAllRoomMaps();
    if (isHost()) {
        dItem::Item item((u16)0xFFF1);
        dSTR::unlock(&item, member);
    }
    dFootmark::getEditor()->clearMarks2();
    clearRecord(member + NET_REC_MEMBER_A);
    clearRecord(member + NET_REC_MEMBER_B);
    clearRecord(member + NET_REC_MEMBER_C);
}

void onDisconnect(int, const dNetMemberMask_c *) {
}

BOOL isDisconnectDone(int, const dNetMemberMask_c *) {
    return dWifi::updateSave() != 0;
}

void onMemberLeaving(int, const dNetMemberMask_c *) {
}

BOOL isMemberLeaveDone(int, const dNetMemberMask_c *) {
    return dWifi::updateSave() != 0;
}

// Channel lbl_80474BF0 / lbl_80474BFC / lbl_80474C08.
int sendClose0(int, void *, int, int) {
    return -1;
}

int sendClosePairRing(int, void *, int, int value) {
    if (dSaveData_c::getTown()->mNpcSpotA.mFlag &&
        value == dSaveData_c::getTown()->mNpcSpotA.mKind) {
        dNpcInfo::jingle_c::get()->move();
        dNpcInfo::jingle_c::get()->setSpot();
    }
    return -1;
}

int sendClose2(int, void *, int, int) {
    return fn_800F55BC();
}

void applyClose0(int, const void *, u32, int) {
}
void applyClose1(int, const void *, u32, int) {
}
void applyClose2(int, const void *, u32, int) {
}

// Channel lbl_80474C14 / lbl_80474C28 / lbl_80474C3C. Entering the town resets
// the insect slots, the fish records and the house room records.
int sendOpen0(int, void *, int, u8) {
    return -1;
}

int sendOpenInsects(int, void *, int, u8 scene) {
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN)) {
        dInsectCatchData_c init;
        init.mTypeSerial = 0;
        init.mSerial = 0;
        init.mPosXZ = 0;
        init.mPosZ = 0;
        init.mState = 0;
        init.mTypeSerial |= INSECT_NET_TYPE_NONE;
        init.mState |= INSECT_NET_STATE_NORMAL;
        init.mPosX = 0;
        for (int i = 0; i < INSECT_NET_SLOT_NUM; i++) {
            setRecord(i + NET_REC_INSECT, &init, 0);
        }
        return 0;
    }
    return -1;
}

int sendOpenFish(int, void *, int, u8 scene) {
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_TOWN)) {
        dFishCatchData_c init;
        init.mType = 0;
        init._02 = 0;
        init._03 = 0;
        init._04 = 0;
        init._05 = 0;
        init._06 = 0;
        init._07 = 0;
        init._08 = 0;
        init._09 = 0;
        init._0C = -1;
        init.mType |= FISH_TYPE_NUM;
        init._01 = 0;
        init._07 |= 4;
        for (int i = 0; i < PLAY_SYNC_REC_NUM; i++) {
            setRecord(i + PLAY_SYNC_REC_ID, &init, 0);
        }
        clearSyncAcks();
        return 0;
    }
    return -1;
}

int sendOpenHouseRoom(int, void *, int, u8 scene) {
    if (dSceneCheck::isHouseRoomScene(scene) == 1) {
        int house = dSceneCheck::getSceneHouse(scene);
        if (house != -1) {
            int room = dSceneCheck::getSceneRoom(scene);
            if (room != -1) {
                u8 base = house * 9 + room * 3;
                u8 rec[5];
                rec[0] = 0;
                rec[1] = 0;
                rec[0] |= 0x3F;
                rec[2] = 0;
                rec[3] = 0;
                rec[4] = 0;
                for (u32 i = 0; i < 3; i++) {
                    setRecord(base + i + NET_REC_HOUSE, rec, 0);
                }
                return 0;
            }
        }
    }
    return -1;
}

int sendOpen4(int, void *, int, u8) {
    return 0;
}

void applyOpen0(int, const void *, u32, u8) {
}

void applyOpenInsects(int, const void *, u32, u8) {
    lbl_8074E83C = 1;
}

void applyOpenFish(int, const void *, u32, u8) {
    lbl_8074E844 = 1;
}

void applyOpenHouseRoom(int, const void *, u32, u8 scene) {
    if (dSceneCheck::isHouseRoomScene(scene) == 1) {
        int house = dSceneCheck::getSceneHouse(scene);
        if (house != -1) {
            int room = dSceneCheck::getSceneRoom(scene);
            if (room != -1) {
                dSceneCheck::setRoomOpen(house, room);
            }
        }
    }
}

void applyOpen4(int arg, const void *data, u32 size, u8 scene) {
    fn_800F55B4(arg, (int)data, size, scene);
}

// One-entry channels 807508CC / 807508D0 / 807508D4 and 807508D8 / 807508DC /
// 807508E0.
int sendLeave0(int, void *, int, u8) {
    return -1;
}

void applyLeave0(int, const void *, u32, u8) {
}

int sendEnter0(int, void *, int, u8) {
    return -1;
}

void applyEnter0(int, const void *, u32, u8) {
}

// Channel 807508E8 / 807508F0 / 807508F8.
int sendHandover0(int, int, void *, int, u8) {
    return -1;
}

int sendHandoverFlag(int, int, void *buf, int, u8) {
    u8 on = TRUE;
    cLib::memCpy(buf, &on, sizeof(on));
    return sizeof(on);
}

void applyHandover0(int, int, const void *, u32, u8) {
}

void applyHandoverFlag(int, int, const void *data, u32, u8) {
    u8 on = FALSE;
    cLib::memCpy(&on, data, sizeof(on));
}

// Fixed size channel 80750900 / 80750908 / 80750910.
void fillSyncBuf0_0(int, u8 *data, u32) {
    *data = TRUE;
}

void fillSyncBuf0_1(int, u8 *data, u32) {
    *data = TRUE;
}

void applySyncBuf0_0(int, u8 *, u32) {
}

void applySyncBuf0_1(int, u8 *, u32) {
    fn_800F55B0();
}

// Per member channel lbl_80474C50 / lbl_80474C5C / lbl_80474C68.
BOOL packSyncBuf8None(int, const dNetMemberMask_c *, u8 *, u32) {
    return FALSE;
}

BOOL packSyncBuf8Host(int member, const dNetMemberMask_c *mask, u8 *dst,
                      u32 size) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(member);
    BOOL ok = FALSE;
    if (mask->isOn(member)) {
        ok = player->startHostVisit(0x10, 10, 1);
    }
    if (ok) {
        dPrivateData_c::loadHostFromTown();
        memcpy(dst, &player->mHost, size);
    }
    return ok;
}

BOOL packSyncBuf8Animal(int member, const dNetMemberMask_c *mask, u8 *dst,
                        u32) {
    if (!mask->isOn(member) || member == 0) {
        return FALSE;
    }
    return fn_800F4878((dAnimal_c *)dst, member) != 0;
}

void unpackSyncBuf8None(const dNetMemberMask_c &, const u8 *, u32) {
}

void unpackSyncBuf8Host(const dNetMemberMask_c &, const u8 *data, u32) {
    memcpy(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mHost, data,
           sizeof(dPrivateHost_c));
}

void unpackSyncBuf8Animal(const dNetMemberMask_c &, const u8 *data, u32) {
    fn_800F4890((const dAnimal_c *)data);
}

// Fixed size channel 80750918 / 80750920 / 80750928.
void fillSyncBuf4_0(int, u8 *data, u32) {
    *data = TRUE;
}

void fillSyncBuf4LogFlag(int, u8 *data, u32) {
    *data = dActivityLog::isDone();
}

void applySyncBuf4_0(int, u8 *, u32) {
}

void applySyncBuf4LogFlag(int, u8 *data, u32) {
    dActivityLog::applyVisitorDone(*data != 0);
}

} // namespace dNet

// The record tables (lbl_80474050 .. lbl_80474908, by record id).
// clang-format off
const dNetCopyFunc lbl_80474050[NET_REC_NUM] = {
    copyRec00, copyRec01, copyRec02, copyRec03,
    copyRecMemberA0, copyRecMemberA1, copyRecMemberA2, copyRecMemberA3,
    copyRecMemberB0, copyRecMemberB1, copyRecMemberB2, copyRecMemberB3,
    copyRecMemberC0, copyRecMemberC1, copyRecMemberC2, copyRecMemberC3,
    copyRecInsect0, copyRecInsect1, copyRecInsect2, copyRecInsect3,
    copyRecInsect4, copyRecInsect5, copyRecInsect6, copyRecInsect7,
    copyRecInsect8, copyRecInsect9, copyRecInsect10, copyRecInsect11,
    copyRecPlaySync0, copyRecPlaySync1, copyRecPlaySync2, copyRecPlaySync3,
    copyRecPlaySync4, copyRecPlaySync5, copyRecPlaySync6, copyRecPlaySync7,
    copyRec24, copyRec25, copyRec26, copyRec27,
    copyRec28, copyRec29, copyRec2A, copyRecDaub0,
    copyRecDaub1, copyRecDaub2, copyRecDaub3, copyRecDaub4,
    copyRecDaub5, copyRecDaub6, copyRecDaub7, copyRecDaub8,
    copyRecDaub9, copyRecDaub10, copyRecDaub11, copyRecDaub12,
    copyRecDaub13, copyRecDaub14, copyRecDaub15, copyRecDaub16,
    copyRecDaub17, copyRecDaub18, copyRecDaub19, copyRecDaub20,
    copyRecDaub21, copyRecDaub22, copyRecDaub23, copyRecDaub24,
    copyRecDaub25, copyRecDaub26, copyRecDaub27, copyRecDaub28,
    copyRecDaub29, copyRecDaub30, copyRecDaub31, copyRecDaub32,
    copyRecDaub33, copyRecDaub34, copyRecDaub35, copyRecDaub36,
    copyRecDaub37, copyRecDaub38, copyRecDaub39, copyRecDaub40,
    copyRecDaub41, copyRecDaub42, copyRecDaub43, copyRecDaub44,
    copyRecDaub45, copyRecDaub46, copyRecDaub47, copyRecDaub48,
    copyRecDaub49, copyRecDaub50, copyRecDaub51, copyRecDaub52,
    copyRecDaub53, copyRecDaub54, copyRecDaub55, copyRecDaub56,
    copyRecDaub57, copyRecDaub58, copyRecDaub59, copyRecDaub60,
    copyRecDaub61, copyRecDaub62, copyRecDaub63, copyRecDaub64,
    copyRecDaub65, copyRecDaub66, copyRecDaub67, copyRecDaub68,
    copyRecDaub69, copyRecDaub70, copyRecDaub71, copyRecDaub72,
    copyRecDaub73, copyRecDaub74, copyRecDaub75, copyRecDaub76,
    copyRecDaub77, copyRecDaub78, copyRecDaub79, copyRecDaub80,
    copyRecDaub81, copyRecDaub82, copyRecDaub83, copyRecDaub84,
    copyRecDaub85, copyRecDaub86, copyRecDaub87, copyRecDaub88,
    copyRecDaub89, copyRecDaub90, copyRecDaub91, copyRecDaub92,
    copyRecDaub93, copyRecDaub94, copyRecDaub95, copyRecDaub96,
    copyRecDaub97, copyRecDaub98, copyRecDaub99, copyRecDaub100,
    copyRecDaub101, copyRecDaub102, copyRecDaub103, copyRecDaub104,
    copyRecDaub105, copyRecDaub106, copyRecHouse0_0_0, copyRecHouse0_0_1,
    copyRecHouse0_0_2, copyRecHouse0_1_0, copyRecHouse0_1_1, copyRecHouse0_1_2,
    copyRecHouse0_2_0, copyRecHouse0_2_1, copyRecHouse0_2_2, copyRecHouse1_0_0,
    copyRecHouse1_0_1, copyRecHouse1_0_2, copyRecHouse1_1_0, copyRecHouse1_1_1,
    copyRecHouse1_1_2, copyRecHouse1_2_0, copyRecHouse1_2_1, copyRecHouse1_2_2,
    copyRecHouse2_0_0, copyRecHouse2_0_1, copyRecHouse2_0_2, copyRecHouse2_1_0,
    copyRecHouse2_1_1, copyRecHouse2_1_2, copyRecHouse2_2_0, copyRecHouse2_2_1,
    copyRecHouse2_2_2, copyRecHouse3_0_0, copyRecHouse3_0_1, copyRecHouse3_0_2,
    copyRecHouse3_1_0, copyRecHouse3_1_1, copyRecHouse3_1_2, copyRecHouse3_2_0,
    copyRecHouse3_2_1, copyRecHouse3_2_2,
};
// clang-format on

// clang-format off
const dNetClearFunc lbl_80474338[NET_REC_NUM] = {
    clearRec00, NULL, clearRec02, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    clearRecInsect0, clearRecInsect1, clearRecInsect2, clearRecInsect3,
    clearRecInsect4, clearRecInsect5, clearRecInsect6, clearRecInsect7,
    clearRecInsect8, clearRecInsect9, clearRecInsect10, clearRecInsect11,
    clearRecPlaySync0, clearRecPlaySync1, clearRecPlaySync2, clearRecPlaySync3,
    clearRecPlaySync4, clearRecPlaySync5, clearRecPlaySync6, clearRecPlaySync7,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, clearRecDaub0,
    clearRecDaub1, clearRecDaub2, clearRecDaub3, clearRecDaub4,
    clearRecDaub5, clearRecDaub6, clearRecDaub7, clearRecDaub8,
    clearRecDaub9, clearRecDaub10, clearRecDaub11, clearRecDaub12,
    clearRecDaub13, clearRecDaub14, clearRecDaub15, clearRecDaub16,
    clearRecDaub17, clearRecDaub18, clearRecDaub19, clearRecDaub20,
    clearRecDaub21, clearRecDaub22, clearRecDaub23, clearRecDaub24,
    clearRecDaub25, clearRecDaub26, clearRecDaub27, clearRecDaub28,
    clearRecDaub29, clearRecDaub30, clearRecDaub31, clearRecDaub32,
    clearRecDaub33, clearRecDaub34, clearRecDaub35, clearRecDaub36,
    clearRecDaub37, clearRecDaub38, clearRecDaub39, clearRecDaub40,
    clearRecDaub41, clearRecDaub42, clearRecDaub43, clearRecDaub44,
    clearRecDaub45, clearRecDaub46, clearRecDaub47, clearRecDaub48,
    clearRecDaub49, clearRecDaub50, clearRecDaub51, clearRecDaub52,
    clearRecDaub53, clearRecDaub54, clearRecDaub55, clearRecDaub56,
    clearRecDaub57, clearRecDaub58, clearRecDaub59, clearRecDaub60,
    clearRecDaub61, clearRecDaub62, clearRecDaub63, clearRecDaub64,
    clearRecDaub65, clearRecDaub66, clearRecDaub67, clearRecDaub68,
    clearRecDaub69, clearRecDaub70, clearRecDaub71, clearRecDaub72,
    clearRecDaub73, clearRecDaub74, clearRecDaub75, clearRecDaub76,
    clearRecDaub77, clearRecDaub78, clearRecDaub79, clearRecDaub80,
    clearRecDaub81, clearRecDaub82, clearRecDaub83, clearRecDaub84,
    clearRecDaub85, clearRecDaub86, clearRecDaub87, clearRecDaub88,
    clearRecDaub89, clearRecDaub90, clearRecDaub91, clearRecDaub92,
    clearRecDaub93, clearRecDaub94, clearRecDaub95, clearRecDaub96,
    clearRecDaub97, clearRecDaub98, clearRecDaub99, clearRecDaub100,
    clearRecDaub101, clearRecDaub102, clearRecDaub103, clearRecDaub104,
    clearRecDaub105, clearRecDaub106, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
// clang-format on

// clang-format off
const u32 lbl_80474620[NET_REC_NUM] = {
    0x1, 0x4, 0x1, 0xA, 0x8, 0x8, 0x8, 0x8, // 0x00
    0x2, 0x2, 0x2, 0x2, 0xF, 0xF, 0xF, 0xF, // 0x08
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, // 0x10
    0x6, 0x6, 0x6, 0x6, 0x8, 0x8, 0x8, 0x8, // 0x18
    0x8, 0x8, 0x8, 0x8, 0x1, 0x2, 0x1, 0x1, // 0x20
    0x1, 0x1, 0x2, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x28
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x30
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x38
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x40
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x48
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x50
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x58
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x60
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x68
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x70
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x78
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x80
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, // 0x88
    0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x8, 0x8, // 0x90
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // 0x98
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // 0xA0
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // 0xA8
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // 0xB0
    0x8, 0x8, // 0xB8
};
// clang-format on

// clang-format off
const u32 lbl_80474908[NET_REC_NUM] = {
    0x0, 0x1, 0x5, 0x6, 0x10, 0x18, 0x20, 0x28, // 0x00
    0x30, 0x32, 0x34, 0x36, 0x38, 0x47, 0x56, 0x65, // 0x08
    0x74, 0x7A, 0x80, 0x86, 0x8C, 0x92, 0x98, 0x9E, // 0x10
    0xA4, 0xAA, 0xB0, 0xB6, 0xBC, 0xC4, 0xCC, 0xD4, // 0x18
    0xDC, 0xE4, 0xEC, 0xF4, 0xFC, 0xFD, 0xFF, 0x100, // 0x20
    0x101, 0x102, 0x103, 0x105, 0x134, 0x163, 0x192, 0x1C1, // 0x28
    0x1F0, 0x21F, 0x24E, 0x27D, 0x2AC, 0x2DB, 0x30A, 0x339, // 0x30
    0x368, 0x397, 0x3C6, 0x3F5, 0x424, 0x453, 0x482, 0x4B1, // 0x38
    0x4E0, 0x50F, 0x53E, 0x56D, 0x59C, 0x5CB, 0x5FA, 0x629, // 0x40
    0x658, 0x687, 0x6B6, 0x6E5, 0x714, 0x743, 0x772, 0x7A1, // 0x48
    0x7D0, 0x7FF, 0x82E, 0x85D, 0x88C, 0x8BB, 0x8EA, 0x919, // 0x50
    0x948, 0x977, 0x9A6, 0x9D5, 0xA04, 0xA33, 0xA62, 0xA91, // 0x58
    0xAC0, 0xAEF, 0xB1E, 0xB4D, 0xB7C, 0xBAB, 0xBDA, 0xC09, // 0x60
    0xC38, 0xC67, 0xC96, 0xCC5, 0xCF4, 0xD23, 0xD52, 0xD81, // 0x68
    0xDB0, 0xDDF, 0xE0E, 0xE3D, 0xE6C, 0xE9B, 0xECA, 0xEF9, // 0x70
    0xF28, 0xF57, 0xF86, 0xFB5, 0xFE4, 0x1013, 0x1042, 0x1071, // 0x78
    0x10A0, 0x10CF, 0x10FE, 0x112D, 0x115C, 0x118B, 0x11BA, 0x11E9, // 0x80
    0x1218, 0x1247, 0x1276, 0x12A5, 0x12D4, 0x1303, 0x1332, 0x1361, // 0x88
    0x1390, 0x13BF, 0x13EE, 0x141D, 0x144C, 0x147B, 0x14AA, 0x14B2, // 0x90
    0x14BA, 0x14C2, 0x14CA, 0x14D2, 0x14DA, 0x14E2, 0x14EA, 0x14F2, // 0x98
    0x14FA, 0x1502, 0x150A, 0x1512, 0x151A, 0x1522, 0x152A, 0x1532, // 0xA0
    0x153A, 0x1542, 0x154A, 0x1552, 0x155A, 0x1562, 0x156A, 0x1572, // 0xA8
    0x157A, 0x1582, 0x158A, 0x1592, 0x159A, 0x15A2, 0x15AA, 0x15B2, // 0xB0
    0x15BA, 0x15C2, // 0xB8
};
// clang-format on
