// The online multiplayer session (members, host, packets, shared records).
// .text 800D3590..800E3E20, -sym on. The main .text ends with __sinit
// (800DDFAC); 800DE0E4..800E3E20 comes from src/dol/game/d_net_sync.inc (an
// #included file's code always lands after the main section; place the #include
// by where the data only the .inc uses sits). See include/game/game/d_net.hpp
// and build/net_work/BRIEF.md.
#include <game/game/d_net.hpp>
#include <game/game/d_reset.hpp>
// used by [A]
#include <lib/MSL/arith.h>
// used by d_net_sync.inc [E]
#include <game/cLib/c_lib.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo_sync.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_footmark.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_wifi.hpp>

// used by d_net_sync.inc [D]
#include <game/cLib/c_counter.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_clock.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_mii_icon.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_voice.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/game/d_sv_npc_pos.hpp>
#include <lib/egg/core/eggHeap.h>
#include <string.h>

// used by [B]
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_net_wc24.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_a_player.hpp>

// The free functions are in namespace dNet (d_net.hpp); the classes and the
// .inc handlers call them unqualified.
using namespace dNet;

// The included handler code: its tables open the .rodata (the include sits
// before every main-part table).
#include "d_net_sync.inc"

namespace dNet {

u32 getNetError() {
    return !dReset::Manage_c::GetInstance()->isResetting() ? g_dNet.mError : 0;
}

void setNetError(u32 flag) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        g_dNet.mError |= flag;
    }
}

void updateRecvCount(int no, u8 seq, BOOL check) {
    if (check) {
        u8 prev = g_dNet.mRecvCount[no];
        u8 diff = seq > prev ? (u8)(seq - prev) : (u8)(seq + 0x100 - prev);
        if (diff != 1) {
            setNetError(NET_ERROR_SEQUENCE);
        }
    }
    g_dNet.mRecvCount[no] = seq;
}

BOOL isBlockHeaderValid(int id, u32 size, int dest, u32 scene, int from) {
    BOOL ret = FALSE;
    if (id <= 0xff && size <= 0x3ffff && dest <= 7 && scene <= 0x7f &&
        from <= 3) {
        ret = TRUE;
    }
    return ret;
}

// The 6-byte header of a data block in the packet stream.
struct dNetBlockHeader {
    u8 mId : 8;    // the receive handler (lbl_80473E44)
    u8 mSizeL : 8; // the data size (18 bits)
    u8 mSizeM : 8;
    u8 mSizeH : 2;
    u8 mDest : 3;  // a member, NET_MEMBER_NONE (every member), 5, 6
    u8 mScene : 7; // the sender's scene
    u8 mFrom : 2;  // the sender
};

void writeBlockHeader(void *dst, int id, u32 size, int dest, u8 scene,
                      int from) {
    dNetBlockHeader hdr;
    isBlockHeaderValid(id, size, dest, scene, from);
    hdr.mSizeL = size;
    hdr.mSizeM = size >> 8;
    hdr.mSizeH = size >> 16;
    hdr.mId = id;
    hdr.mDest = dest;
    hdr.mScene = scene;
    hdr.mFrom = from;
    cLib::memCpy(dst, &hdr, sizeof(hdr));
}

BOOL readBlockHeader(const void *src, int *id, u32 *size, int *dest, u8 *scene,
                     int *from) {
    dNetBlockHeader hdr;
    cLib::memCpy(&hdr, src, sizeof(hdr));
    int i = hdr.mId;
    u32 s = (hdr.mSizeH << 16) | (hdr.mSizeM << 8) | hdr.mSizeL;
    int d = hdr.mDest;
    u8 sc = hdr.mScene;
    int f = hdr.mFrom;
    if (isBlockHeaderValid(i, s, d, sc, f)) {
        *size = s;
        *id = i;
        *dest = d;
        *scene = sc;
        *from = f;
        return TRUE;
    }
    return FALSE;
}

BOOL isMemberLinked(int no) {
    BOOL ret = FALSE;
    if (g_dNet.mHelloRecv.isOn(no) && !g_dNet.mHelloReply.isOn(no) &&
        g_dNet.mLinked.isOn(no) && !g_dNet.mHelloSend.isOn(no)) {
        ret = TRUE;
    }
    return ret;
}

BOOL isMemberConnected(int no) {
    if (!g_dNet.mHelloHold.isOn(no)) {
        u8 n = no;
        BOOL connected = FALSE;
        if (n != dNetMgr_c::sInstance->mInfo.mMyNo &&
            (dNetMgr_c::sInstance->mInfo.mMemberBits & (1 << n))) {
            connected = TRUE;
        }
        if (connected) {
            return TRUE;
        }
    } else if (isMemberLinked(no)) {
        return TRUE;
    }
    return FALSE;
}

BOOL canSendTo(int no, BOOL hasData, BOOL check) {
    if (!check || (check && isMemberLinked(no))) {
        if (dNetMgr_c::sInstance->canSend(no)) {
            if (hasData) {
                return getNetOutBuf()->getPending(no) < dNetInBuf_c::SLOT_NUM;
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL canSendToAccepted(BOOL hasData, dNetMemberMask_c *bits, BOOL check) {
    int myNo = getNetMyNo()->get();
    BOOL ret = TRUE;
    for (u32 i = 0; i < 4; i++) {
        if ((int)i != myNo && getNetAccepted()->isOn(i)) {
            if (!canSendTo(i, hasData, check) && ret) {
                ret = FALSE;
            }
            bits->on(i);
        }
    }
    return ret;
}

BOOL canSendToMembers(BOOL hasData, dNetMemberMask_c *bits, BOOL check) {
    int myNo = getNetMyNo()->get();
    BOOL ret = TRUE;
    for (u32 i = 0; i < 4; i++) {
        if ((int)i != myNo && isStayingMember(i)) {
            if (!canSendTo(i, hasData, check) && ret) {
                ret = FALSE;
            }
            bits->on(i);
        }
    }
    return ret;
}

int getDirectSeqMode(int kind) {
    if (kind < NET_DIRECT_NUM) {
        return lbl_80473E00[kind];
    }
    return 1;
}

void callDirectHandler(int no, const u8 *data, u32 size, int kind) {
    if (kind < NET_DIRECT_NUM) {
        lbl_80473E10[kind](no, data, size);
    }
}

#ifndef BUGFIXES
void callBlockHandlers(const u8 *data, u32 size) {
    if (data != NULL) {
        u32 pos = 0;
        while (pos + 6 <= size) {
            int id;
            u32 len;
            int dest;
            u8 scene;
            int from;
            if (!readBlockHeader(data, &id, &len, &dest, &scene, &from)) {
                break;
            }
            data += 6;
            pos += 6;
            if (id < NET_PACKET_NUM) {
                lbl_80473E44[id](from, data, len, dest, scene);
                data += len;
                pos += len;
            }
        }
    }
}
#else
static BOOL isBlockPayloadSizeValid(int id, const u8 *data, u32 len) {
    if (id < NET_PACKET_NUM && !isNetBlockPayloadValid(id, data, len)) {
        return FALSE;
    }
    switch (id) {
    case NET_PACKET_SCENE_SAVE_HOST:
        return len <= 0xF;
    case NET_PACKET_SCENE_LOAD:
        return len <= 0x15;
    case NET_PACKET_SCENE_EXIT_DATA:
    case NET_PACKET_SCENE_ENTER_DATA:
        return len <= 0x5;
    case NET_PACKET_SCENE_HANDOVER_DATA:
        return len <= 0xA;
    case NET_PACKET_SYNC_BUF_MEMBER:
        return len == 2;
    case NET_PACKET_SYNC_BUF:
        return len >= sizeof(dNetMemberMask_c) &&
               len <= sizeof(dNetMemberMask_c) + SYNC_BUF8_SIZE - 2;
    default:
        return TRUE;
    }
}

void callBlockHandlers(const u8 *data, u32 size, int sender) {
    if (data == NULL || sender < 0 || sender >= NET_MEMBER_MAX) {
        return;
    }
    const u8 *check = data;
    u32 remaining = size;
    while (remaining != 0) {
        if (remaining < sizeof(dNetBlockHeader)) {
            return;
        }
        int id;
        u32 len;
        int dest;
        u8 scene;
        int from;
        if (!readBlockHeader(check, &id, &len, &dest, &scene, &from) ||
            from != sender || len > remaining - sizeof(dNetBlockHeader) ||
            !isBlockPayloadSizeValid(id, check + sizeof(dNetBlockHeader),
                                     len)) {
            return;
        }
        check += sizeof(dNetBlockHeader) + len;
        remaining -= sizeof(dNetBlockHeader) + len;
    }
    while (size >= sizeof(dNetBlockHeader)) {
        int id;
        u32 len;
        int dest;
        u8 scene;
        int from;
        readBlockHeader(data, &id, &len, &dest, &scene, &from);
        data += sizeof(dNetBlockHeader);
        size -= sizeof(dNetBlockHeader);
        if (id < NET_PACKET_NUM) {
            lbl_80473E44[id](from, data, len, dest, scene);
        }
        data += len;
        size -= len;
    }
}
#endif

void copyRecord(int id, void *dst, const void *src, int arg) {
    if (id < NET_REC_NUM) {
        lbl_80474050[id](dst, src, arg);
    }
}

void clearRecord(int id) {
    if (id < NET_REC_NUM) {
        u32 size = getNetPacket()->getRecordSize(id);
        u8 *rec = getNetPacket()->getRecord(id);
        cLib::memSet(rec, 0, size);
        if (lbl_80474338[id] != NULL) {
            lbl_80474338[id](rec);
        }
        getNetPacket()->setRecordFlag(id, 0);
    }
}

void clearVisitor() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        int no = getNetMyNo()->get();
        getNetSyncBuf()->readBuf0(no);
        onVisitorClear(no, &g_dNet.mAccepted);
    }
}

void setSaveVisitor() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        onVisitorSet(getNetRequest()->mSaveMember.get(), &g_dNet.mAccepted);
    }
}

BOOL checkCanFinishLeave() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    return canFinishLeave(no, &mask);
}

void callOnLeave() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    onLeave(no, &mask);
}

void callOnMemberLeft() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    onMemberLeft(no, &mask);
}

void callOnLeaveAll() {
    dNetMemberMask_c mask;
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (isStayingMember(i)) {
            onLeave(i, &mask);
        }
    }
}

void callOnDisconnect() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    onDisconnect(no, &mask);
}

BOOL checkDisconnectDone() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    return isDisconnectDone(no, &mask);
}

void callOnMemberLeaving() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    onMemberLeaving(no, &mask);
}

BOOL checkMemberLeaveDone() {
    int no = getNetDisconnect()->mMember.get();
    dNetMemberMask_c mask = g_dNet.mMembers;
    mask.off(no);
    return isMemberLeaveDone(no, &mask);
}

void callOnDisconnectAll() {
    dNetMemberMask_c mask;
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (isStayingMember(i)) {
            onDisconnect(i, &mask);
        }
    }
}

BOOL checkDisconnectDoneAll() {
    dNetMemberMask_c mask;
    BOOL ret = TRUE;
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (isStayingMember(i)) {
            BOOL ok = isDisconnectDone(i, &mask);
            if (ret && !ok) {
                ret = FALSE;
            }
        }
    }
    return ret;
}

// The 2-byte header of a packet.
struct dNetPacketHeader {
    u8 mValid : 1;  // always set
    u8 mCredit : 2; // receive buffers freed since the last packet
    // a single message (mKind, handled at once) instead of a block stream
    u8 mDirect : 1;
    u8 mKind : 4;
    u8 mCount : 8; // the sender's packet counter
};

#ifndef BUGFIXES
void onNetMgrRecv(u8 no, const void *data, u32 size) {
    if (getNetReportedError() ||
        dReset::Manage_c::GetInstance()->isResetting()) {
        return;
    }
    dNetPacketHeader hdr;
    cLib::memCpy(&hdr, data, sizeof(hdr));
    s8 credit = -hdr.mCredit;
    if (credit < 0) {
        getNetOutBuf()->addPending(no, credit);
    }
    if (hdr.mDirect) {
        int kind = hdr.mKind;
        int seq = getDirectSeqMode(kind);
        if (seq > 0) {
            updateRecvCount(no, hdr.mCount, TRUE);
        } else if (seq == 0) {
            updateRecvCount(no, hdr.mCount, FALSE);
        }
        callDirectHandler(no, (const u8 *)data + 2, size - 2, kind);
    } else {
        updateRecvCount(no, hdr.mCount, TRUE);
        getNetInBuf()->push(no, getNetInBuf()->getSlot(no));
        getNetInBuf()->nextSlot(no);
        dNetMgr_c::sInstance->setRecvBuf(no, getNetInBuf()->getSlotBuf(no),
                                         0x16802);
    }
}
#else
void onNetMgrRecv(u8 no, const void *data, u32 size) {
    if (getNetReportedError() ||
        dReset::Manage_c::GetInstance()->isResetting() ||
        no >= NET_MEMBER_MAX || data == NULL ||
        size < sizeof(dNetPacketHeader) || size > NET_RECV_BUF_SIZE) {
        return;
    }
    dNetPacketHeader hdr;
    cLib::memCpy(&hdr, data, sizeof(hdr));
    if (hdr.mDirect && (hdr.mKind > NET_DIRECT_NUM ||
                        (hdr.mKind == NET_DIRECT_NUM && size != sizeof(hdr)))) {
        return;
    }
    if (!hdr.mDirect && size < sizeof(hdr) + sizeof(u16) * 2) {
        return;
    }
    if (hdr.mCredit > getNetOutBuf()->getPending(no)) {
        return;
    }
    s8 credit = -hdr.mCredit;
    if (credit < 0) {
        getNetOutBuf()->addPending(no, credit);
    }
    if (hdr.mDirect) {
        int kind = hdr.mKind;
        int seq = getDirectSeqMode(kind);
        if (seq > 0) {
            updateRecvCount(no, hdr.mCount, TRUE);
        } else if (seq == 0) {
            updateRecvCount(no, hdr.mCount, FALSE);
        }
        callDirectHandler(no, (const u8 *)data + sizeof(hdr),
                          size - sizeof(hdr), kind);
    } else {
        updateRecvCount(no, hdr.mCount, TRUE);
        getNetInBuf()->queuePacket(no, data, size);
    }
}
#endif

void onNetMgrJoin(u32 no) {
    if (dReset::Manage_c::GetInstance()->isResetting()) {
        return;
    }
    if (no == dNetMgr_c::sInstance->mInfo.mMyNo) {
        return;
    }
    if (!g_dNet.mHelloHold.isOn(no) && !g_dNet.mLinked.isOn(no)) {
        g_dNet.mLinked.on(no);
        if (!g_dNet.mHelloSend.isOn(no)) {
            g_dNet.mHelloSend.on(no);
        }
    }
}

void onNetMgrLeave(u32 no) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        g_dNet.mLinked.off(no);
        g_dNet.mHelloRecv.off(no);
        g_dNet.mHelloSend.off(no);
        g_dNet.mHelloReply.off(no);
    }
}

void onNetMgrEvent(int, int) {
}

int writePacketHeader(u8 **pp, int no, BOOL direct, int kind) {
    dNetPacketHeader hdr;
    cLib::memSet(&hdr, 0, sizeof(hdr));
    u8 credit = getNetInBuf()->getCredit(no);
    u32 d = direct != FALSE;
    int k = kind < NET_DIRECT_NUM ? kind : NET_DIRECT_NUM;
    hdr.mValid = 1;
    hdr.mCredit = credit;
    hdr.mDirect = d;
    hdr.mKind = k;
    hdr.mCount = g_dNet.mSendCount[no];
    u8 *p = *pp;
    cLib::memCpy(p, &hdr, sizeof(hdr));
    *pp = p + sizeof(hdr);
    return credit;
}

struct dNetStreamSizes {
    u16 mStream;
    u16 mExtra;
};

void writeStreamSizes(void *dst, u32 stream, u32 extra) {
    dNetStreamSizes sizes;
    sizes.mStream = stream;
    sizes.mExtra = extra;
    cLib::memCpy(dst, &sizes, sizeof(sizes));
}

BOOL isMemberDisconnected(int no) {
    return !dNetMgr_c::sInstance->isConnected(no);
}

BOOL sendDirectTo(int no, const void **datas, const u32 *sizes, u32 num,
                  int kind, BOOL wait, BOOL check, BOOL noCount) {
    if (canSendTo(no, num != 0, check)) {
        u32 total = 2;
        for (u32 i = 0; i < num; i++) {
            total += sizes[i];
        }
        if (total <= 0x16802) {
            getNetOutBuf()->resetSize(no);
            u8 *p = getNetOutBuf()->getWritePtr(no);
            u8 *start = p;
            int credit = writePacketHeader(&p, no, TRUE, kind);
            if (kind < NET_DIRECT_NUM) {
                for (u32 i = 0; i < num; i++) {
                    u32 size = sizes[i];
                    cLib::memCpy(p, datas[i], size);
                    p += size;
                }
            }
            BOOL ok1 = getNetOutBuf()->addSize(no, total);
            BOOL ok2 = dNetMgr_c::sInstance->send(no, start, total);
            if (!noCount) {
                g_dNet.mSendCount[no]++;
            }
            getNetInBuf()->addCredit(no, -credit);
            if (wait) {
                getNetPacket()->mSendWait[no]++;
            } else {
                getNetPacket()->mSendWait[no] = 0;
            }
            BOOL ret = FALSE;
            if (ok1 && ok2) {
                ret = TRUE;
            }
            return ret;
        }
    } else {
        getNetPacket()->mSendWait[no]++;
    }
    return FALSE;
}

BOOL sendDirect(int no, const void **datas, const u32 *sizes, u32 num, int kind,
                BOOL wait, BOOL check, BOOL noCount) {
    BOOL hasData = num != 0;
    dNetMemberMask_c mask;
    BOOL ok;
    if (no < NET_MEMBER_MAX) {
        ok = canSendTo(no, hasData, check);
        mask.on(no);
    } else {
        ok = canSendToAccepted(hasData, &mask, check);
    }
    if (ok) {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (mask.isOn(i)) {
                sendDirectTo(i, datas, sizes, num, kind, wait, check, noCount);
            }
        }
        return TRUE;
    } else {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (mask.isOn(i)) {
                getNetPacket()->mSendWait[i]++;
            }
        }
        return FALSE;
    }
}

int findNextGuest(u32 no) {
    for (; no < NET_MEMBER_MAX; no++) {
        if (no != 0 && isStayingMember(no)) {
            return no;
        }
    }
    return NET_MEMBER_NONE;
}

BOOL isSceneChanging() {
    BOOL ret = FALSE;
    if (dScene::getCurrentScene() >= NET_SCENE_NONE ||
        dScene::getNextScene() < NET_SCENE_NONE) {
        ret = TRUE;
    } else if (getSceneChangeState()) {
        ret = TRUE;
    } else if (getNetScene()->hasOrder()) {
        ret = TRUE;
    }
    return ret;
}

} // namespace dNet

void dNetSceneCloseEntry_c::check(u32 idx, u32 size) {
}

void dNetSceneCloseEntry_c::set(const dNetSceneCloseEntry_c &other) {
    check(other.mIdx, other.mSize);
    *this = other;
}

void dNetSceneCloseEntry_c::set(u32 idx, u32 size) {
    check(idx, size);
    mIdx = idx;
    mSize = size;
}

u32 dNetSceneCloseEntry_c::getIdx() const {
    return mIdx;
}

u32 dNetSceneCloseEntry_c::getSize() const {
    return mSize;
}

dNetSceneCloseEntry_c *dNetSceneCloseEntry_c::getData() {
    return this;
}

void dNetSceneOpenEntry_c::check(u32 idx, u32 size) {
}

void dNetSceneOpenEntry_c::set(const dNetSceneOpenEntry_c &other) {
    check(other.mIdx, other.mSize);
    *this = other;
}

void dNetSceneOpenEntry_c::set(u32 idx, u32 size) {
    check(idx, size);
    mIdx = idx;
    mSize = size;
}

u32 dNetSceneOpenEntry_c::getIdx() const {
    return mIdx;
}

u32 dNetSceneOpenEntry_c::getSize() const {
    return mSize;
}

dNetSceneOpenEntry_c *dNetSceneOpenEntry_c::getData() {
    return this;
}

void dNetSceneLeaveEntry_c::check(u32 idx, u32 size) {
}

void dNetSceneLeaveEntry_c::set(const dNetSceneLeaveEntry_c &other) {
    check(other.mIdx, other.mSize);
    *this = other;
}

void dNetSceneLeaveEntry_c::set(u32 idx, u32 size) {
    check(idx, size);
    mIdx = idx;
    mSize = size;
}

u32 dNetSceneLeaveEntry_c::getIdx() const {
    return mIdx;
}

u32 dNetSceneLeaveEntry_c::getSize() const {
    return mSize;
}

dNetSceneLeaveEntry_c *dNetSceneLeaveEntry_c::getData() {
    return this;
}

void dNetSceneEnterEntry_c::check(u32 idx, u32 size) {
}

void dNetSceneEnterEntry_c::set(const dNetSceneEnterEntry_c &other) {
    check(other.mIdx, other.mSize);
    *this = other;
}

void dNetSceneEnterEntry_c::set(u32 idx, u32 size) {
    check(idx, size);
    mIdx = idx;
    mSize = size;
}

u32 dNetSceneEnterEntry_c::getIdx() const {
    return mIdx;
}

u32 dNetSceneEnterEntry_c::getSize() const {
    return mSize;
}

dNetSceneEnterEntry_c *dNetSceneEnterEntry_c::getData() {
    return this;
}

void dNetSceneHandoverEntry_c::check(u32 idx, u32 size) {
}

void dNetSceneHandoverEntry_c::set(const dNetSceneHandoverEntry_c &other) {
    check(other.mIdx, other.mSize);
    *this = other;
}

void dNetSceneHandoverEntry_c::set(u32 idx, u32 size) {
    check(idx, size);
    mIdx = idx;
    mSize = size;
}

u32 dNetSceneHandoverEntry_c::getIdx() const {
    return mIdx;
}

u32 dNetSceneHandoverEntry_c::getSize() const {
    return mSize;
}

dNetSceneHandoverEntry_c *dNetSceneHandoverEntry_c::getData() {
    return this;
}

dNetMemberMask_c::dNetMemberMask_c() {
    clear();
}

u8 dNetMemberMask_c::bit(int member) {
    return 1 << member;
}

void dNetMemberMask_c::on(int member) {
    u8 old = mBits;
    mBits |= bit(member);
    if (old != mBits) {
        mCount++;
    }
}

void dNetMemberMask_c::off(int member) {
    u8 old = mBits;
    mBits &= ~bit(member);
    if (old != mBits) {
        mCount--;
    }
}

void dNetMemberMask_c::clear() {
    mBits = 0;
    mCount = 0;
}

bool dNetMemberMask_c::isOn(int member) const {
    return (mBits & bit(member)) != 0;
}

u8 dNetMemberMask_c::count() const {
    return mCount;
}

void dNetMemberMask_c::resetAcks(BOOL on) {
    u32 myNo = getMyNo();
    if (isSessionActive()) {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (i == myNo) {
                if (on) {
                    this->on(i);
                } else {
                    off(i);
                }
            } else if (isStayingMember(i)) {
                off(i);
            } else {
                this->on(i);
            }
        }
    } else {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (i == myNo) {
                if (on) {
                    this->on(i);
                } else {
                    off(i);
                }
            } else {
                this->on(i);
            }
        }
    }
}

bool dNetMemberMask_c::isAll() const {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (!isOn(i)) {
            return false;
        }
    }
    return true;
}

dNetMemberNo_c::dNetMemberNo_c() {
    clear();
}

int dNetMemberNo_c::get() const {
    return mNo;
}

void dNetMemberNo_c::set(int no) {
    mNo = no;
}

void dNetMemberNo_c::clear() {
    set(NET_MEMBER_NONE);
}

dNetIndex_c::dNetIndex_c() {
    clear();
}

int dNetIndex_c::get() const {
    return mValue;
}

void dNetIndex_c::set(int v) {
    mValue = v;
}

void dNetIndex_c::clear() {
    set(4);
}

void dNetMemberScene_c::init() {
    mMember = NET_MEMBER_NONE;
    mScene = NET_SCENE_NONE;
}

void dNetMemberScene_c::set(const dNetMemberScene_c &other) {
    mMember = other.mMember;
    mScene = other.mScene;
}

void dNetMemberScene_c::set(u32 member, u8 scene) {
    mMember = member;
    mScene = scene;
}

u8 dNetMemberScene_c::getScene() const {
    return mScene;
}

u32 dNetMemberScene_c::getMember() const {
    return mMember;
}

dNetMemberScene_c *dNetMemberScene_c::getData() {
    return this;
}

dNetSceneOrder_c::dNetSceneOrder_c() {
    init();
}

void dNetSceneOrder_c::init() {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        clear(i);
    }
}

void dNetSceneOrder_c::clear(int member) {
    mScene[member] = NET_SCENE_NONE;
    mOrder[member] = 4;
}

void dNetSceneOrder_c::setScene(int member, u8 v) {
    mScene[member] = v;
}

u8 dNetSceneOrder_c::getScene(int member) const {
    return mScene[member];
}

void dNetSceneOrder_c::setOrder(int member, u32 v) {
    mOrder[member] = v;
}

u32 dNetSceneOrder_c::getOrder(int member) const {
    return mOrder[member];
}

void dNetSceneMsg_c::set(int member, BOOL hasScene, u8 scene, BOOL hasFlag,
                         BOOL flag, int ackTo) {
    mMember = member;
    mHasScene = hasScene;
    mScene = scene;
    mHasFlag = hasFlag;
    mFlag = flag;
    mAckTo = ackTo;
}

dNetMemberScenes_c::dNetMemberScenes_c() {
    init();
}

void dNetMemberScenes_c::init() {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        clear(i);
    }
}

void dNetMemberScenes_c::clear(int member) {
    setScene(member, NET_SCENE_NONE);
    setOwner(member, 0);
}

void dNetMemberScenes_c::setScene(int member, u8 v) {
    mScene[member] = v;
}

u8 dNetMemberScenes_c::getScene(int member) const {
    return mScene[member];
}

void dNetMemberScenes_c::setOwner(int member, int v) {
    mOwner[member] = v;
}

int dNetMemberScenes_c::isOwner(int member) const {
    return mOwner[member];
}

int dNetMemberScenes_c::findOwner(u8 scene) const {
    if (scene < NET_SCENE_NONE) {
        if (scene == mScene[0] && mOwner[0]) {
            return 0;
        }
        if (scene == mScene[1] && mOwner[1]) {
            return 1;
        }
        if (scene == mScene[2] && mOwner[2]) {
            return 2;
        }
        if (scene == mScene[3] && mOwner[3]) {
            return 3;
        }
    }
    return NET_MEMBER_NONE;
}

int dNetMemberScenes_c::findNonOwner(u8 scene, u32 start) const {
    if (scene < NET_SCENE_NONE) {
        for (u32 i = start; i < NET_MEMBER_MAX; i++) {
            if (scene == mScene[i] && !mOwner[i]) {
                return i;
            }
        }
    }
    return NET_MEMBER_NONE;
}

int dNetMemberScenes_c::countMembers(u8 scene) const {
    int n = 0;
    if (scene < NET_SCENE_NONE) {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (isStayingMember(i) && scene == mScene[i]) {
                n++;
            }
        }
    }
    return n;
}

bool dNetMemberScenes_c::isAllIn(u8 scene) const {
    int num = getMemberNum();
    return countMembers(scene) == num;
}

void dNetInBuf_c::create(EGG::Heap *heap) {
#ifndef BUGFIXES
    mBuffer = (u8 *)heap->alloc(
        NET_RECV_BUF_SIZE * (NET_MEMBER_MAX - 1) * SLOT_NUM, 4);
#else
    mBuffer = (u8 *)heap->alloc(
        NET_RECV_BUF_SIZE * (NET_MEMBER_MAX - 1) * (SLOT_NUM + 1), 4);
#endif
}

void dNetInBuf_c::init() {
    clearBufs();
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mSlot[i] = 0;
        mReadSlot[i] = 0;
        mNum[i] = 0;
        mCredit[i] = 0;
#ifdef BUGFIXES
        for (u32 j = 0; j < SLOT_NUM; j++) {
            mDataSize[i][j] = 0;
        }
#endif
    }
}

void dNetInBuf_c::clearMember(int member) {
    mReadSlot[member] = 0;
    mNum[member] = 0;
    mCredit[member] = 0;
#ifdef BUGFIXES
    for (u32 j = 0; j < SLOT_NUM; j++) {
        mDataSize[member][j] = 0;
    }
#endif
}

#ifdef BUGFIXES
u8 *dNetInBuf_c::getStagingBuf(int index) {
    return mBuffer +
           NET_RECV_BUF_SIZE * ((NET_MEMBER_MAX - 1) * SLOT_NUM + index);
}

BOOL dNetInBuf_c::queuePacket(int member, const void *data, u32 size) {
    if (member < 0 || member >= NET_MEMBER_MAX || !mSetup || data == NULL ||
        size > NET_RECV_BUF_SIZE) {
        return FALSE;
    }
    dNetMgr_c::sInstance->lock(0);
    if (mNum[member] >= SLOT_NUM || mSlotBuf[member][mSlot[member]] == NULL) {
        dNetMgr_c::sInstance->unlock(0);
        return FALSE;
    }
    int slot = mSlot[member];
    cLib::memCpy(mSlotBuf[member][slot], data, size);
    mDataSize[member][slot] = size - sizeof(dNetPacketHeader);
    if (mNum[member] == 0) {
        mReadSlot[member] = slot;
    }
    mNum[member]++;
    nextSlot(member);
    dNetMgr_c::sInstance->unlock(0);
    return TRUE;
}

u32 dNetInBuf_c::getDataSize(int member) const {
    return mDataSize[member][mReadSlot[member]];
}
#endif

u8 *dNetInBuf_c::getBufferAddr() {
    return mBuffer;
}

void dNetInBuf_c::setMyNo(int myNo) {
    if (mSetup) {
        return;
    }
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        u8 *buf = mBuffer;
        if (i < myNo) {
            buf += i * NET_RECV_BUF_SIZE * SLOT_NUM;
        } else if (i > myNo) {
            buf += (i - 1) * NET_RECV_BUF_SIZE * SLOT_NUM;
        } else {
            buf = NULL;
        }
        for (u32 j = 0; j < SLOT_NUM; j++) {
            mSlotBuf[i][j] = buf;
            if (buf != NULL) {
                buf += NET_RECV_BUF_SIZE;
            }
        }
    }
    mSetup = true;
}

void dNetInBuf_c::clearBufs() {
    if (!mSetup) {
        return;
    }
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        for (int j = 0; j < SLOT_NUM; j++) {
            mSlotBuf[i][j] = NULL;
        }
    }
    mSetup = false;
}

void dNetInBuf_c::nextSlot(int member) {
    if (++mSlot[member] >= SLOT_NUM) {
        mSlot[member] = 0;
    }
}

int dNetInBuf_c::getSlot(int member) const {
    return mSlot[member];
}

u8 *dNetInBuf_c::getSlotBuf(int member) {
    return mSlotBuf[member][getSlot(member)];
}

u8 *dNetInBuf_c::getData(int member) {
    u8 *data = NULL;
    dNetMgr_c::sInstance->lock(0);
    if (mNum[member] != 0) {
        data = mSlotBuf[member][mReadSlot[member]];
        if (data != NULL) {
            data += 2;
        }
    }
    dNetMgr_c::sInstance->unlock(0);
    return data;
}

void dNetInBuf_c::push(int member, int slot) {
    dNetMgr_c::sInstance->lock(0);
    if (mNum[member] == 0) {
        mReadSlot[member] = slot;
    }
    mNum[member]++;
    dNetMgr_c::sInstance->unlock(0);
}

void dNetInBuf_c::pop(int member) {
    dNetMgr_c::sInstance->lock(0);
    u32 slot = mReadSlot[member];
    slot++;
    if (slot >= SLOT_NUM) {
        slot = 0;
    }
    mReadSlot[member] = slot;
    mNum[member]--;
    dNetMgr_c::sInstance->unlock(0);
}

int dNetInBuf_c::getCredit(int member) const {
    return mCredit[member];
}

void dNetInBuf_c::addCredit(int member, s8 n) {
    u8 a = abs(n);
    if (n > 0) {
        mCredit[member] += a;
    } else if (n < 0) {
        mCredit[member] -= a;
    }
}

void dNetOutBuf_c::create(EGG::Heap *heap) {
    mBuffer = (u8 *)heap->alloc(NET_RECV_BUF_SIZE * (NET_MEMBER_MAX - 1), 4);
}

void dNetOutBuf_c::init() {
    resetSizes();
    resetPendings();
    clearBufs();
}

void dNetOutBuf_c::clearMember(int member) {
    resetSize(member);
    resetPending(member);
}

void dNetOutBuf_c::setMyNo(int myNo) {
    if (mSetup) {
        return;
    }
    u8 *buf = mBuffer;
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (i != myNo) {
            mBuf[i] = buf;
            buf += NET_RECV_BUF_SIZE;
        } else {
            mBuf[i] = NULL;
        }
    }
    mSetup = true;
}

void dNetOutBuf_c::clearBufs() {
    if (!mSetup) {
        return;
    }
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mBuf[i] = NULL;
    }
    mSetup = false;
}

u8 *dNetOutBuf_c::getWritePtr(int member) const {
    return mBuf[member] + mSize[member];
}

BOOL dNetOutBuf_c::addSize(int member, u32 size) {
    u32 n = size + mSize[member];
    if (n <= NET_RECV_BUF_SIZE) {
        mSize[member] = n;
        return TRUE;
    }
    return FALSE;
}

void dNetOutBuf_c::resetSize(int member) {
    mSize[member] = 0;
}

void dNetOutBuf_c::resetSizes() {
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mSize[i] = 0;
    }
}

void dNetOutBuf_c::addPending(int member, s8 n) {
    u8 a = abs(n);
    dNetMgr_c::sInstance->lock(0);
    if (n > 0) {
        mPending[member] += a;
    } else if (n < 0) {
        mPending[member] -= a;
    }
    dNetMgr_c::sInstance->unlock(0);
}

void dNetOutBuf_c::resetPending(int member) {
    mPending[member] = 0;
}

void dNetOutBuf_c::resetPendings() {
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mPending[i] = 0;
    }
}

u32 dNetOutBuf_c::getPending(int member) const {
    return mPending[member];
}

void dNetPacket_c::create(EGG::Heap *heap) {
    mStream = (u8 *)heap->alloc(0x16800, 4);
    mRecordBuf = (u8 *)heap->alloc(0x15CA, 4);
    u32 size = getMaxRecordSize() + 8;
    mWork = (u8 *)heap->alloc(size, 4);
    mWorkSize = size;
    mRecordFlags = (u8 *)heap->alloc(NET_REC_NUM, 4);
}

void dNetPacket_c::init() {
    clearStream();
    clearRecords();
    mFrame = 0;
    mSuspended = 0;
    mLeaving.clear();
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mSendWait[i] = 0;
        mIdleFrames[i] = 0;
    }
}

void dNetPacket_c::clearMember(int member) {
    mSendWait[member] = 0;
    mIdleFrames[member] = 0;
}

#ifndef BUGFIXES
void dNetPacket_c::receive() {
    if (getSessionActive() && !isNetStopped()) {
        int myNo = getNetMyNo()->get();
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if ((int)i != myNo && isStayingMember(i)) {
                u32 n = 0;
                const u8 *data;
                while (true) {
                    data = getNetInBuf()->getData(i);
                    if (data == NULL || n >= dNetInBuf_c::SLOT_NUM) {
                        break;
                    }
                    dNetStreamSizes sizes;
                    cLib::memCpy(&sizes, data, sizeof(sizes));
                    u32 recSize = sizes.mStream;
                    u16 extraSize = sizes.mExtra;
                    data += sizeof(sizes);
                    if (recSize != 0) {
                        u32 pos = 0;
                        const u8 *p = data;
                        while (pos + 1 <= recSize) {
                            u8 b;
                            cLib::memCpy(&b, p, 1);
                            int id = b;
                            if (id >= NET_REC_NUM) {
                                break;
                            }
                            p++;
                            pos++;
                            u8 *rec = getRecord(id);
                            u32 size = getRecordSize(id);
                            cLib::memCpy(rec, p, size);
                            p += size;
                            pos += size;
                        }
                        data += recSize;
                    }
                    if (extraSize != 0) {
                        callBlockHandlers(data, extraSize);
                    }
                    getNetInBuf()->pop(i);
                    getNetInBuf()->addCredit(i, 1);
                    n++;
                }
            }
        }
        if (getNetHoldBuf()->mOldOwner < NET_MEMBER_MAX &&
            getNetHoldBuf()->mHoldState == 2) {
            u8 scene = getNetScene()->getScene(myNo);
            if (myNo == getNetScene()->findOwner(scene)) {
                u32 size = getNetHoldBuf()->getSize();
                if (size != 0) {
                    callBlockHandlers(getNetHoldBuf()->getBuf(), size);
                }
                getNetHoldBuf()->init();
            }
        }
    }
}
#else
void dNetPacket_c::receive() {
    if (!getSessionActive() || isNetStopped()) {
        return;
    }
    int myNo = getNetMyNo()->get();
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if ((int)i == myNo || !isStayingMember(i)) {
            continue;
        }
        u32 n = 0;
        while (n < dNetInBuf_c::SLOT_NUM) {
            const u8 *data = getNetInBuf()->getData(i);
            if (data == NULL) {
                break;
            }
            u32 payloadSize = getNetInBuf()->getDataSize(i);
            if (payloadSize >= sizeof(dNetStreamSizes)) {
                dNetStreamSizes sizes;
                cLib::memCpy(&sizes, data, sizeof(sizes));
                u32 recSize = sizes.mStream;
                u32 extraSize = sizes.mExtra;
                data += sizeof(sizes);
                payloadSize -= sizeof(sizes);
                if (recSize <= payloadSize &&
                    extraSize == payloadSize - recSize) {
                    const u8 *p = data;
                    u32 left = recSize;
                    BOOL valid = TRUE;
                    while (left != 0) {
                        u8 id = *p++;
                        left--;
                        if (id >= NET_REC_NUM || getRecordSize(id) > left) {
                            valid = FALSE;
                            break;
                        }
                        u32 len = getRecordSize(id);
                        p += len;
                        left -= len;
                    }
                    if (valid) {
                        p = data;
                        left = recSize;
                        while (left != 0) {
                            u8 id = *p++;
                            u32 len = getRecordSize(id);
                            cLib::memCpy(getRecord(id), p, len);
                            p += len;
                            left -= len + 1;
                        }
                        if (extraSize != 0) {
                            callBlockHandlers(data + recSize, extraSize, i);
                        }
                    }
                }
            }
            getNetInBuf()->pop(i);
            getNetInBuf()->addCredit(i, 1);
            n++;
        }
    }
    if (getNetHoldBuf()->mOldOwner < NET_MEMBER_MAX &&
        getNetHoldBuf()->mHoldState == 2) {
        u8 scene = getNetScene()->getScene(myNo);
        if (myNo == getNetScene()->findOwner(scene)) {
            u32 size = getNetHoldBuf()->getSize();
            if (size != 0) {
                callBlockHandlers(getNetHoldBuf()->getBuf(), size, myNo);
            }
            getNetHoldBuf()->init();
        }
    }
}
#endif

void dNetPacket_c::sendFrame() {
    if (isNetStopped()) {
        return;
    }
    if (getNetMyNo()->get() < NET_MEMBER_MAX) {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (!g_dNet.mHelloHold.isOn(i)) {
                if (g_dNet.mHelloReply.isOn(i)) {
                    if (sendDirect(i, NULL, NULL, 0, NET_DIRECT_HELLO_ACK,
                                   FALSE, FALSE, FALSE)) {
                        g_dNet.mHelloReply.off(i);
                    }
                } else if (g_dNet.mHelloSend.isOn(i)) {
                    sendDirect(i, NULL, NULL, 0, NET_DIRECT_HELLO, FALSE, FALSE,
                               TRUE);
                }
            }
        }
    }
    if (getMemberNum() > 1) {
        dNetMemberMask_c mask;
        if (canSendToMembers(FALSE, &mask, TRUE)) {
            int myNo = getNetMyNo()->get();
            BOOL keepSend = FALSE;
            u32 sendSize = 0;
            int sendTo = NET_MEMBER_NONE;
            if (getNetHoldBuf()->mEnterMember < NET_MEMBER_MAX ||
                getNetHoldBuf()->mOldOwner < NET_MEMBER_MAX) {
                if (getNetHoldBuf()->mHoldState != 2) {
                    keepSend = TRUE;
                    sendSize = getNetHoldBuf()->getSize();
                } else if (getNetHoldBuf()->mEnterMember < NET_MEMBER_MAX) {
                    sendTo = getNetHoldBuf()->mEnterMember;
                } else if (getNetHoldBuf()->mOldOwner < NET_MEMBER_MAX) {
                    u8 scene = getNetScene()->getScene(myNo);
                    int member = getNetScene()->findOwner(scene);
                    if (member != myNo && member < NET_MEMBER_MAX) {
                        sendTo = member;
                    }
                }
            }

            u8 *start[NET_MEMBER_MAX];
            u32 size[NET_MEMBER_MAX];
            int credit[NET_MEMBER_MAX];
            u8 *ptr[NET_MEMBER_MAX];
            u32 extraSize[NET_MEMBER_MAX];
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if ((int)i == myNo) {
                    ptr[i] = NULL;
                    start[i] = NULL;
                } else if (!isStayingMember(i)) {
                    ptr[i] = NULL;
                    start[i] = NULL;
                } else {
                    getNetOutBuf()->resetSize(i);
                    start[i] = ptr[i] = getNetOutBuf()->getWritePtr(i);
                }
                size[i] = 0;
                credit[i] = 0;
            }
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (ptr[i] != NULL) {
                    ptr[i] += 6;
                }
            }

            u32 recSize = 0;
            for (u32 i = 0; i < NET_REC_NUM; i++) {
                if (getRecordFlag(i)) {
                    u8 *rec = getRecord(i);
                    u32 recordSize = getRecordSize(i);
                    for (u32 j = 0; j < NET_MEMBER_MAX; j++) {
                        if (ptr[j] != NULL) {
                            u8 b = i;
                            cLib::memCpy(ptr[j], &b, 1);
                            ptr[j]++;
                            cLib::memCpy(ptr[j], rec, recordSize);
                            ptr[j] += recordSize;
                        }
                    }
                    recSize += recordSize + 1;
                }
            }

            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                extraSize[i] = 0;
            }
            if (sendTo < NET_MEMBER_MAX) {
                u32 n = getNetHoldBuf()->getSize();
                if (n != 0) {
                    cLib::memCpy(ptr[sendTo], getNetHoldBuf()->getBuf(), n);
                    ptr[sendTo] += n;
                    extraSize[sendTo] += n;
                }
            }

            u32 pos = 0;
            u32 streamSize = getStreamSize();
            u8 *block = getStream();
            while (pos + 6 <= streamSize) {
                int id;
                u32 blockSize;
                int dest;
                u8 scene;
                int from;
                if (!readBlockHeader(block, &id, &blockSize, &dest, &scene,
                                     &from)) {
                    break;
                }
                int to = dest;
                switch (to) {
                case 0:
                case 1:
                case 2:
                case 3:
                    if (ptr[to] != NULL) {
                        cLib::memCpy(ptr[to], block, blockSize + 6);
                        ptr[to] += blockSize + 6;
                        extraSize[to] += blockSize + 6;
                    }
                    break;
                case NET_MEMBER_NONE:
                    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                        if (ptr[i] != NULL) {
                            cLib::memCpy(ptr[i], block, blockSize + 6);
                            ptr[i] += blockSize + 6;
                            extraSize[i] += blockSize + 6;
                        }
                    }
                    break;
                case NET_DEST_SCENE_OWNER: {
                    u8 myScene = getNetScene()->getScene(myNo);
                    if (myScene < NET_SCENE_NONE) {
                        int member = getNetScene()->findOwner(myScene);
                        if (member < NET_MEMBER_MAX) {
                            if (member != myNo) {
                                if (ptr[member] != NULL) {
                                    if (getNetHoldBuf()->mOldOwner <
                                            NET_MEMBER_MAX &&
                                        getNetHoldBuf()->mHoldState != 2) {
                                        getNetHoldBuf()->add(block,
                                                             blockSize + 6);
                                    } else {
                                        cLib::memCpy(ptr[member], block,
                                                     blockSize + 6);
                                        ptr[member] += blockSize + 6;
                                        extraSize[member] += blockSize + 6;
                                    }
                                }
                            } else if (getNetHoldBuf()->mHoldState == 1 &&
                                       getNetHoldBuf()->mOldOwner <
                                           NET_MEMBER_MAX &&
                                       isSceneOwner()) {
                                getNetHoldBuf()->add(block, blockSize + 6);
                            }
                        }
                    }
                    break;
                }
                case NET_DEST_SCENE_OTHERS:
                    if (getNetScene()->isOwner(myNo)) {
                        u8 myScene = getNetScene()->getScene(myNo);
                        if (myScene < NET_SCENE_NONE) {
                            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                                if (ptr[i] != NULL) {
                                    if (myScene == getNetScene()->getScene(i)) {
                                        if ((int)i != getNetScene()->mLeaving) {
                                            cLib::memCpy(ptr[i], block,
                                                         blockSize + 6);
                                            ptr[i] += blockSize + 6;
                                            extraSize[i] += blockSize + 6;
                                        }
                                    } else if ((int)i == getNetHoldBuf()
                                                             ->mEnterMember &&
                                               getNetHoldBuf()->mHoldState !=
                                                   2) {
                                        getNetHoldBuf()->add(block,
                                                             blockSize + 6);
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                block += blockSize + 6;
                pos += blockSize + 6;
            }

            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (start[i] != NULL) {
                    u8 *p = start[i];
                    if (recSize + extraSize[i] != 0) {
                        size[i] = recSize + 6 + extraSize[i];
                        credit[i] =
                            writePacketHeader(&p, i, FALSE, NET_DIRECT_NUM);
                        writeStreamSizes(p, recSize, extraSize[i]);
                    } else {
                        size[i] = 2;
                        credit[i] =
                            writePacketHeader(&p, i, TRUE, NET_DIRECT_NUM);
                    }
                }
            }

            BOOL canSend = TRUE;
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (start[i] != NULL && size[i] > 2 &&
                    getNetOutBuf()->getPending(i) >= dNetInBuf_c::SLOT_NUM) {
                    canSend = FALSE;
                    break;
                }
            }

            if (canSend) {
                for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                    if (start[i] != NULL) {
                        if (size[i] <= 2 && credit[i] == 0 &&
                            mIdleFrames[i] < 600) {
                            mIdleFrames[i]++;
                        } else if (dNetMgr_c::sInstance->send(i, start[i],
                                                              size[i])) {
                            g_dNet.mSendCount[i]++;
                            mIdleFrames[i] = 0;
                        }
                        mSendWait[i] = 0;
                        getNetInBuf()->addCredit(i, -credit[i]);
                        if (size[i] > 2) {
                            getNetOutBuf()->addPending(i, 1);
                        }
                    }
                }
                clearRecordFlags();
                clearStream();
                getNetScene()->executeAfterSend();
                getNetSaveEnd()->executeAfterSend();
                getNetDisconnect()->executeAfterSend();
                getNetPacket()->mFrame = cCounter_c::m_gameFrame;
                if (sendTo < NET_MEMBER_MAX) {
                    getNetHoldBuf()->init();
                } else if (getNetHoldBuf()->mOldOwner < NET_MEMBER_MAX &&
                           getNetHoldBuf()->mHoldState == 1) {
                    getNetHoldBuf()->mHoldState = 2;
                }
            } else {
                for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                    if (start[i] != NULL) {
                        if (credit[i] != 0 || mIdleFrames[i] >= 600) {
                            if (sendDirect(i, NULL, NULL, 0, NET_DIRECT_NUM,
                                           TRUE, TRUE, FALSE)) {
                                mIdleFrames[i] = 0;
                            }
                        } else {
                            mIdleFrames[i]++;
                        }
                        mSendWait[i]++;
                    }
                }
                if (keepSend) {
                    getNetHoldBuf()->setSize(sendSize);
                }
            }
        } else {
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (mask.isOn(i)) {
                    mSendWait[i]++;
                    mIdleFrames[i]++;
                }
            }
        }
    } else {
        clearRecordFlags();
        clearStream();
        getNetScene()->executeAfterSend();
        getNetSaveEnd()->executeAfterSend();
        getNetDisconnect()->executeAfterSend();
        getNetPacket()->mFrame = cCounter_c::m_gameFrame;
    }
}

u8 *dNetPacket_c::getStream() const {
    return mStream;
}

u32 dNetPacket_c::getStreamSize() const {
    return mBlock - mStream;
}

void dNetPacket_c::clearStream() {
    mBlock = mStream;
    mStreamEnd = mStream;
}

bool dNetPacket_c::canAdd(u32 size) const {
    u32 used = mStreamEnd - mStream;
    return used + size <= 0x16800;
}

void dNetPacket_c::begin() {
    if (!isNetStopped() && !(getNetError() & NET_ERROR_OVERFLOW)) {
        if (canAdd(6)) {
            mStreamEnd += 6;
        } else {
            setNetError(NET_ERROR_OVERFLOW);
        }
    }
}

void dNetPacket_c::add(const void *data, u32 size) {
    if (!isNetStopped() && !(getNetError() & NET_ERROR_OVERFLOW)) {
        if (canAdd(size)) {
            cLib::memCpy(mStreamEnd, data, size);
            mStreamEnd += size;
        } else {
            setNetError(NET_ERROR_OVERFLOW);
        }
    }
}

void dNetPacket_c::send(int id, int member) {
    if (!isNetStopped() && !(getNetError() & NET_ERROR_OVERFLOW)) {
        u32 size = mStreamEnd - mBlock;
        if (size >= 6) {
            writeBlockHeader(mBlock, id, size - 6, member, dScene::getCurrentScene(),
                             getNetMyNo()->get());
            mBlock = mStreamEnd;
        }
    }
}

void dNetPacket_c::setRecord(int id, void *data, int arg) {
    if (isNetStopped()) {
        return;
    }
    u8 *rec = getRecord(id);
    u32 size = getRecordSize(id);
    for (int i = 0; i < 4; i++) {
        mWork[i] = 0xFE;
        mWork[size + 4 + i] = 0xFE;
    }
    u8 *work = mWork + 4;
    cLib::memCpy(work, rec, size);
    copyRecord(id, work, data, arg);
    for (int i = 0; i < 4; i++) {
        if (mWork[i] != 0xFE || mWork[size + 4 + i] != 0xFE) {
            return;
        }
    }
    if (cLib::memCmp(work, rec, size) != 0) {
        cLib::memCpy(rec, work, size);
        setRecordFlag(id, 1);
    }
}

void dNetPacket_c::clearRecords() {
    for (u32 id = 0; id < NET_REC_NUM; id++) {
        clearRecord(id);
    }
}

u8 *dNetPacket_c::getRecordBuf() const {
    return mRecordBuf;
}

u32 dNetPacket_c::getRecordSize(int id) {
    if (id < NET_REC_NUM) {
        return lbl_80474620[id];
    }
    return 0;
}

u32 dNetPacket_c::getMaxRecordSize() {
    u32 max = 0;
    for (u32 i = 0; i < NET_REC_NUM; i++) {
        u32 size = getRecordSize(i);
        if (max < size) {
            max = size;
        }
    }
    return max;
}

u8 *dNetPacket_c::getRecord(int id) const {
    if (id < NET_REC_NUM && mRecordBuf != NULL) {
        return mRecordBuf + lbl_80474908[id];
    }
    return NULL;
}

int dNetPacket_c::getRecordFlag(int id) const {
    if (id < NET_REC_NUM && mRecordBuf != NULL) {
        return mRecordFlags[id];
    }
    return 0;
}

void dNetPacket_c::setRecordFlag(int id, u8 flag) {
    if (id < NET_REC_NUM && mRecordBuf != NULL) {
        mRecordFlags[id] = flag;
    }
}

void dNetPacket_c::clearRecordFlags() {
    for (u32 i = 0; i < NET_REC_NUM; i++) {
        setRecordFlag(i, 0);
    }
}

void dNetHoldBuf_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0x16800, 4);
}

void dNetHoldBuf_c::init() {
    mSize = 0;
    mEnterMember = 4;
    mOldOwner = 4;
    mHoldState = 0;
}

void dNetHoldBuf_c::clearMember(int member) {
}

u8 *dNetHoldBuf_c::getBuf() const {
    return mBuf;
}

u32 dNetHoldBuf_c::getSize() const {
    return mSize;
}

void dNetHoldBuf_c::setSize(u32 size) {
    mSize = size;
}

void dNetHoldBuf_c::add(const void *data, u32 size) {
    u32 end = mSize + size;
    if (end <= 0x16800) {
        cLib::memCpy(mBuf + mSize, data, size);
        mSize = end;
    }
}

void dNetRequest_c::init() {
    for (int i = 0; i < 4; i++) {
        mState[i] = 0;
        mMemberStep[i] = 0;
    }
    mStep = 0;
    mMyKind.clear();
    mSaveStep = 0;
    mSaveTimer = 0;
    mSaveKind.clear();
    mSaveMember.clear();
    mRequests.clear();
    mGrantedKind.clear();
    mGrantedMember.clear();
    mMsgOn = 0;
    mMsgTimer = 0;
    mMsgEndTimer = 0;
    mBusy.clear();
    mLeave = 0;
}

void dNetRequest_c::clearMember(int member) {
    mState[member] = 0;
    mMemberStep[member] = 0;
    mRequests.off(member);
    mBusy.off(member);
}

void dNetRequest_c::execute() {
    if (!isNetStopped() && isJoined()) {
        int self = getNetMyNo()->get();
        if (getStep() == 2) {
            if (mGrantedMember.get() < 4) {
                setStep(0);
            } else if (self == 0) {
                mRequests.on(0);
                setKind(0, mMyKind.get());
                setStep(3);
            } else if (isStayingMember(self)) {
                u8 kind = mMyKind.get();
                getNetPacket()->begin();
                getNetPacket()->add(&kind, 1);
                getNetPacket()->send(NET_PACKET_JOIN_REQUEST, 0);
                setStep(3);
            } else if (isMemberConnected(0)) {
                if (sendDirect(0, NULL, NULL, 0, NET_DIRECT_JOIN_REQUEST, 0, 1,
                               0)) {
                    setStep(3);
                }
            } else {
                setStep(0);
            }
        }

        int step = getSaveStep();
        if (step == 1) {
            mSaveTimer = 0;
            mMsgOn = 1;
            mMsgTimer = 180;
            mMsgEndTimer = 0;
            setSaveStep(2);
        } else if (step == 2) {
            if (++mSaveTimer > 600) {
                mMsgOn = 0;
                setSaveStep(3);
            } else if (dDemo::startNetSave()) {
                setSaveStep(4);
            }
        } else if (step == 3 || step == 4) {
            if (self == 0) {
                if (step == 3) {
                    setMemberStep(0, 0);
                } else {
                    setMemberStep(0, 1);
                }
            } else {
                u8 ok = step != 3;
                getNetPacket()->begin();
                getNetPacket()->add(&ok, 1);
                getNetPacket()->send(NET_PACKET_JOIN_SAVE_REPLY, 0);
            }
            setSaveStep(0);
        } else if (step == 5) {
            dDemo::endNetSave();
            mMsgEndTimer = 300;
            if (self == 0) {
                getNetRequest()->setMemberStep(0, 0);
            } else {
                getNetPacket()->begin();
                getNetPacket()->send(NET_PACKET_JOIN_CANCEL_DONE, 0);
            }
            setSaveStep(0);
        } else if (step == 6) {
            if (mMsgTimer == 0) {
                setSaveStep(7);
            }
        } else if (step == 7) {
            if (mSaveKind.get() != 3) {
                dNand::getSceneChange()->setReturnExitHere(0.0f);
            }
            fn_800DDE7C();
            if (mSaveKind.get() == 1) {
                dScene::requestSceneChange(0x40, 0, 0);
            } else {
                dScene::requestSceneChange(0x39, 0, 5);
            }
            mMsgOn = 0;
            mMsgTimer = 0;
            mMsgEndTimer = 0;
            switch (mSaveKind.get()) {
            case 0:
                if (self == 0) {
                    dSvMgr_c::requestSaveConnectNetHst();
                } else {
                    dSvMgr_c::requestSaveConnectNetVst();
                }
                getNetTransfer()->start();
                break;
            case 2:
                if (self != 0) {
                    dSvMgr_c::requestSaveContinueNetVst();
                    getNetSaveEnd()->start(FALSE);
                }
                break;
            case 3:
                if (self != 0) {
                    dSvMgr_c::requestSaveInterruptNetVst();
                    getNetSaveEnd()->start(TRUE);
                }
                break;
            case 1:
                getNetDisconnect()->start(TRUE);
                break;
            }
            setSaveStep(0);
        }

        if (getNetMyNo()->get() == 0) {
            int active = findActive();
            if (active < 4) {
                if (getState(active) == 1) {
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (getMemberStep(i) == 2) {
                            if (i == 0) {
                                setSaveStep(1);
                                mSaveKind.set(getKind(active));
                                mSaveMember.set(active);
                            } else {
                                u8 data =
                                    ((u8)active << 2) | (u8)getKind(active);
                                getNetPacket()->begin();
                                getNetPacket()->add(&data, 1);
                                getNetPacket()->send(
                                    NET_PACKET_JOIN_SAVE_PROMPT, i);
                            }
                            setMemberStep(i, 3);
                        }
                    }
                    setState(active, 2);
                }
                if (getState(active) == 2) {
                    BOOL ok = TRUE;
                    BOOL all = TRUE;
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        int st = getMemberStep(i);
                        if (st != 1) {
                            if (st == 0) {
                                if (all) {
                                    all = FALSE;
                                }
                            } else {
                                ok = FALSE;
                                break;
                            }
                        }
                    }
                    if (getKind(active) == 0 && !isMemberConnected(active)) {
                        all = FALSE;
                    }
                    if (isSceneChanging()) {
                        all = FALSE;
                    }
                    if (ok) {
                        if (all) {
                            if (getKind(active) == 0) {
                                getNetWatched()->on(active);
                                getNetJoinDone()->reset();
                            }
                            setState(active, 4);
                        } else {
                            setState(active, 3);
                            for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                                if (getMemberStep(i) == 1) {
                                    if (i == active || !isStayingMember(i)) {
                                        setMemberStep(i, 0);
                                    } else {
                                        setMemberStep(i, 4);
                                    }
                                }
                            }
                        }
                    }
                }
                if (getState(active) == 3) {
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (getMemberStep(i) == 4) {
                            if (i == 0) {
                                setSaveStep(5);
                            } else {
                                getNetPacket()->begin();
                                getNetPacket()->send(
                                    NET_PACKET_JOIN_SAVE_CANCEL, i);
                            }
                            setMemberStep(i, 5);
                        }
                    }
                }
                if (getState(active) == 3) {
                    BOOL done = TRUE;
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (getMemberStep(i) != 0) {
                            done = FALSE;
                            break;
                        }
                    }
                    if (done && sendResult(active, FALSE)) {
                        setState(active, 0);
                        mRequests.off(active);
                        mBusy.clear();
                        g_dNet.mHelloHold.off(active);
                    }
                } else if (getState(active) == 4 && sendResult(active, TRUE)) {
                    g_dNet.mHelloHold.off(active);
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (i == active || !isStayingMember(i)) {
                            if (getKind(active) == 0) {
                                getNetAccepted()->on(active);
                            }
                        } else if (i == 0) {
                            setSaveStep(6);
                        } else {
                            getNetPacket()->begin();
                            getNetPacket()->send(NET_PACKET_JOIN_COMMIT, i);
                        }
                    }
                    mGrantedKind.set(getKind(active));
                    mGrantedMember.set(active);
                    setState(active, 0);
                    mRequests.off(active);
                }
            }

            if (mBusy.count() == 0 && findActive() >= 4) {
                int found = 4;
                for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                    if (mRequests.isOn(i)) {
                        found = i;
                        break;
                    }
                }
                if (found < 4) {
                    int kind = getKind(found);
                    setState(found, 1);
                    if (kind == 0) {
                        g_dNet.mHelloHold.on(found);
                    }
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (i == found) {
                            if (kind != 1) {
                                mBusy.on(i);
                            }
                        } else if (isStayingMember(i)) {
                            mBusy.on(i);
                        }
                    }
                    for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                        if (i == found || !isStayingMember(i)) {
                            setMemberStep(i, 1);
                        } else {
                            setMemberStep(i, 2);
                        }
                    }
                }
            }
            if (mBusy.count() != 0) {
                int act = findActive();
                for (int i = 0; i < ARRAY_SIZE(mMemberStep); i++) {
                    if (mRequests.isOn(i) && i != act && sendResult(i, FALSE)) {
                        mRequests.off(i);
                    }
                }
            }
        }
    }

    if (isSessionActive() && mMsgOn) {
        if (mMsgEndTimer != 0) {
            dObjTelop_c::request(TELOP_NET_CANCEL);
            if (--mMsgEndTimer == 0) {
                mMsgOn = 0;
            }
        } else {
            int kind = mSaveKind.get();
            BOOL shown = FALSE;
            switch (kind) {
            case 0:
                shown = TRUE;
                dObjTelop_c::request(TELOP_NET_ARRIVE);
                break;
            case 1:
                shown = TRUE;
                dObjTelop_c::request(TELOP_NET_LEAVE);
                break;
            case 2:
                shown = TRUE;
                dObjTelop_c::request(TELOP_NET_SAVE);
                break;
            case 3:
                shown = TRUE;
                dObjTelop_c::request(TELOP_NET_CLOSE);
                break;
            }
            if (shown && mMsgTimer != 0) {
                mMsgTimer--;
            }
        }
    }
}

void dNetRequest_c::executeLeave() {
    if (!isNetStopped() && mLeave) {
        if (isHost()) {
            mBusy.off(0);
        } else {
            getNetPacket()->begin();
            getNetPacket()->send(NET_PACKET_JOIN_RELEASE, 0);
        }
        mLeave = 0;
    }
}

void dNetRequest_c::setState(int member, u8 state) {
    mState[member] = state;
}

int dNetRequest_c::getState(int member) const {
    return mState[member];
}

int dNetRequest_c::findActive() const {
    for (u32 i = 0; i < 4; i++) {
        if (getState(i) != 0) {
            return i;
        }
    }
    return 4;
}

BOOL dNetRequest_c::sendResult(int member, BOOL flag) {
    if (member == getNetMyNo()->get()) {
        if (flag) {
            setStep(1);
        } else {
            setStep(0);
        }
        return TRUE;
    }
    if (isStayingMember(member)) {
        u8 data = flag != 0;
        getNetPacket()->begin();
        getNetPacket()->add(&data, 1);
        getNetPacket()->send(NET_PACKET_JOIN_RESULT, member);
        return TRUE;
    }
    if (isMemberConnected(member)) {
        u8 mask = 0;
        const void *data = &mask;
        u32 size = 1;
        int num = 1;
        if (flag) {
            for (u32 i = 0; i < 4; i++) {
                if (isStayingMember(i)) {
                    mask |= getNetMembers()->bit(i);
                }
            }
        } else {
            num = 0;
        }
        if (sendDirect(member, &data, &size, num, NET_DIRECT_JOIN_RESULT, 0, 1,
                       0)) {
            return TRUE;
        }
    } else {
        return TRUE;
    }
    return FALSE;
}

void dNetRequest_c::setKind(int member, int kind) {
    mKind[member].set(kind);
}

int dNetRequest_c::getKind(int member) const {
    return mKind[member].get();
}

void dNetRequest_c::setStep(u8 step) {
    mStep = step;
}

int dNetRequest_c::getStep() const {
    return mStep;
}

void dNetRequest_c::setMemberStep(int member, u8 state) {
    mMemberStep[member] = state;
}

int dNetRequest_c::getMemberStep(int member) const {
    return mMemberStep[member];
}

void dNetRequest_c::setSaveStep(u8 step) {
    mSaveStep = step;
}

int dNetRequest_c::getSaveStep() const {
    return mSaveStep;
}

void dNetTransfer_c::init() {
    mStep = 0;
    mItemMaskRecv = 0;
    mItemSent = 0;
    mItemNum = 0;
    for (u32 i = 0; i < ARRAY_SIZE(mItems); i++) {
        mItems[i] = NULL;
    }
    mItemsRecv = 0;
    mTownSent = 0;
    for (int i = 0; i < ARRAY_SIZE(mReady); i++) {
        mReady[i] = 0;
    }
    mTownReqSend = 0;
    mTownReqRecv = 0;
    mFinish = 0;
    mScene = NET_SCENE_NONE;
    mStartRecv = 0;
#ifdef BUGFIXES
    mTownRecv = 0;
#endif
}

void dNetTransfer_c::clearMember(int member) {
    mItems[member] = NULL;
    mReady[member] = 0;
}

void dNetTransfer_c::execute() {
    if (isNetStopped()) {
        return;
    }
    int self = getNetMyNo()->get();
    BOOL wait = FALSE;
    if (mTownReqSend) {
        if (sendTownReq()) {
            mTownReqSend = 0;
        } else {
            wait = TRUE;
        }
    }
    switch (getStep()) {
    case 1:
        if (mStartRecv) {
            BOOL ready = TRUE;
            for (int i = 0; i < ARRAY_SIZE(mReady); i++) {
                if (i != self && getNetAccepted()->isOn(i) &&
                    !isMemberLinked(i)) {
                    ready = FALSE;
                    break;
                }
            }
            if (ready) {
                setStep(2);
            }
        }
        break;
    case 2:
        if (!wait && sendPlayer()) {
            setStep(3);
        }
        break;
    case 3:
        if (mItemMaskRecv) {
            collectItems();
            setStep(4);
        }
        break;
    case 4:
        if (!wait && sendItems(NET_MEMBER_NONE)) {
            setStep(15);
        }
        break;
    case 5:
        if (!wait && isAllInScene(0x39) &&
            sendStart(getNetRequest()->mGrantedMember.get())) {
            setStep(6);
        }
        break;
    case 6:
        if (mItemMaskRecv) {
            if (findNextGuest(0) == NET_MEMBER_NONE) {
                setStep(7);
            } else {
                setStep(9);
            }
        }
        break;
    case 7:
        if (!wait && sendItemMask(getNetRequest()->mGrantedMember.get())) {
            collectItems();
            setStep(8);
        }
        break;
    case 8:
        if (!wait && sendItems(getNetRequest()->mGrantedMember.get())) {
            setStep(9);
        }
        break;
    case 9:
        if (mTownReqRecv) {
            setStep(10);
        }
        break;
    case 10:
        if (!wait && !countMembersInScene(0x39) &&
            getNetScene()->getLeader() >= 4 &&
            sendTown(getNetRequest()->mGrantedMember.get())) {
            setStep(15);
        }
        break;
    case 11:
        if (mItemMaskRecv) {
            if (self == findNextGuest(0)) {
                setStep(12);
            } else {
                setStep(14);
            }
        }
        break;
    case 12:
        if (!wait && sendItemMask(getNetRequest()->mGrantedMember.get())) {
            collectItems();
            setStep(13);
        }
        break;
    case 13:
        if (!wait && sendItems(getNetRequest()->mGrantedMember.get())) {
            setStep(14);
        }
        break;
    case 14:
        if (!wait && sendPlayer(getNetRequest()->mGrantedMember.get())) {
            setStep(15);
        }
        break;
    case 15:
        break;
    }

    if (mFinish) {
        int member = getNetRequest()->mGrantedMember.get();
        if (self != member) {
            u8 scene = getNetTransfer()->mScene;
            getNetMembers()->on(member);
            getNetScene()->enter(member, scene);
            init();
            if (self == 0) {
                getNetPacket()->begin();
                getNetPacket()->send(NET_PACKET_JOIN_DONE_HOST,
                                     NET_MEMBER_NONE);
            } else {
                getNetPacket()->begin();
                getNetPacket()->send(NET_PACKET_JOIN_DONE, 0);
            }
        } else if (sendFinish()) {
            for (int i = 0; i < ARRAY_SIZE(mReady); i++) {
                if (getNetAccepted()->isOn(i)) {
                    getNetMembers()->on(i);
                } else {
                    getNetMembers()->off(i);
                }
            }
            setSessionActive(1);
            init();
        }
    }
}

void dNetTransfer_c::start() {
    int member = getNetRequest()->mGrantedMember.get();
    int self = getNetMyNo()->get();
    if (self == member) {
        setStep(1);
    } else if (self == 0) {
        setStep(5);
    } else {
        setStep(11);
    }
}

void dNetTransfer_c::setStep(u8 step) {
    mStep = step;
}

int dNetTransfer_c::getStep() const {
    return mStep;
}

void dNetTransfer_c::collectItems() {
    mItemNum = countToCstmSendable(mExclude);
    for (u32 i = 0; i < ARRAY_SIZE(mItems); i++) {
        void *item = getToCstmSendable(i, mExclude);
        if (item == NULL) {
            return;
        }
        mItems[i] = item;
    }
}

BOOL dNetTransfer_c::sendStart(int member) {
    return sendDirect(member, NULL, NULL, 0, NET_DIRECT_XFER_START, 0, 1, 0);
}

BOOL dNetTransfer_c::sendPlayer() {
    int self = getNetMyNo()->get();
    dNetMemberMask_c failed;
    if (canSendToAccepted(0, &failed, 1)) {
        const void *data[11] = {NULL};
        u32 size[11] = {0};
        u8 itemMask[0x20];
        u64 key;
        u8 equipNum;
        u8 hasAnimal;
        u8 hasMii;
        u8 scene;

        data[0] = dPlayerMgr::Mgr_c::getCurrentPlayer();
        size[0] = 0x86C0;
        memset(itemMask, 0, sizeof(itemMask));
        buildDlItemMask(itemMask);
        data[1] = itemMask;
        size[1] = sizeof(itemMask);
        key = dNetWc24::g_wc24.mUserId;
        data[2] = &key;
        size[2] = sizeof(key);
        equipNum = countPlEquipSendable();
        data[3] = &equipNum;
        size[3] = sizeof(equipNum);
        u32 i = 0;
        int n = 4;
        for (; i < equipNum; i++) {
            data[n] = getPlEquipSendable(i);
            size[n] = 0x2000;
            n++;
        }
        data[n] = &hasAnimal;
        size[n] = sizeof(hasAnimal);
        n++;
        dNpc::bufferedAnimal_c *animal = dNpc::getBufferedAnimal();
        if (animal == NULL) {
            hasAnimal = FALSE;
        } else {
            hasAnimal = TRUE;
            data[n] = animal;
            size[n++] = 0x3040;
        }
        hasMii = dMiiIcon::hasMyMii();
        data[n] = &hasMii;
        size[n] = sizeof(hasMii);
        n++;
        if (hasMii) {
            data[n] = dMiiIcon::getMyMii();
            size[n++] = 0x54;
        }
        scene = dScene::getCurrentScene();
        data[n] = &scene;
        size[n] = sizeof(scene);
        if (sendDirect(NET_MEMBER_NONE, data, size, n + 1,
                       NET_DIRECT_XFER_PLAYER, 0, 1, 0)) {
            dNpc::clearBufferedAnimal();
            for (u32 no = 0; no < NET_MEMBER_MAX; no++) {
                if (no != 0 && no != self && getNetAccepted()->isOn(no)) {
                    getNetWatched()->on(no);
                }
            }
            return TRUE;
        }
    } else {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (failed.isOn(i)) {
                getNetPacket()->mSendWait[i]++;
            }
        }
    }
    return FALSE;
}

BOOL dNetTransfer_c::sendItemMask(int member) {
    if (canSendTo(member, 0, 1)) {
        u8 itemMask[0x20];
        memset(itemMask, 0, sizeof(itemMask));
        buildDlItemMask(itemMask);
        const void *data = itemMask;
        u32 size = sizeof(itemMask);
        if (sendDirect(member, &data, &size, 1, NET_DIRECT_XFER_ITEM_MASK, 0, 1,
                       0)) {
            return TRUE;
        }
    } else {
        getNetPacket()->mSendWait[member]++;
    }
    return FALSE;
}

BOOL dNetTransfer_c::sendTown(int member) {
    if (mTownSent < 0x74C15) {
        if (canSendTo(member, 0, 1)) {
            const void *data[12];
            u32 size[12];
            u64 key;
            s64 ticks;
            u8 flags[NET_MEMBER_MAX];
            u8 player;
            u8 hasMii;

            int n;
            u32 rest;
            u32 sent;
            rest = 0x167FC;
            sent = 0;
            n = 1;
            data[0] = &mTownSent;
            size[0] = sizeof(mTownSent);
            key = dNetWc24::g_wc24.mUserId;
            player = dPlayerMgr::getCurrentPlayerSlot();
            ticks = dTime_c::getCurrentTicks();
            hasMii = dMiiIcon::hasMyMii();
            if (mTownSent < 0x1635) {
                data[1] = &key;
                size[1] = sizeof(key);
                data[2] = &player;
                size[2] = sizeof(player);
                getNetSyncBuf()->writeBuf0(member);
                data[3] = getNetSyncBuf()->getBuf0();
                size[3] = 2;
                data[4] = getNetPacket()->getRecordBuf();
                size[4] = 0x15CA;
                data[5] = &ticks;
                size[5] = sizeof(ticks);
                data[6] = &hasMii;
                size[6] = sizeof(hasMii);
                data[7] = dMiiIcon::getMyMii();
                size[7] = 0x54;
                int i = 0;
                n = 8;
                sent = 0x1632;
                rest -= 0x1632;
                for (; i < ARRAY_SIZE(flags); i++) {
                    if (i != member) {
                        u8 flag = getNetScene()->isOwner(i) ? 0x80 : 0;
                        u8 scene = (u8)getNetScene()->getScene(i);
                        data[n] = &flags[i];
                        size[n] = sizeof(flags[i]);
                        flags[i] = flag | scene;
                        n++;
                        rest--;
                        sent++;
                    }
                }
            }
            if (rest != 0) {
                u32 offset = mTownSent < 0x1635 ? 0 : mTownSent - 0x1635;
                u32 len = 0x735E0 - offset;
                if (len > rest) {
                    len = rest;
                }
                const u8 *raw = (u8 *)dSaveData_c::getRaw();
                const int slot = n++;
                data[slot] = raw + offset;
                size[slot] = len;
                sent += len;
            }
            if (sendDirect(member, data, size, n, NET_DIRECT_XFER_TOWN, 0, 1,
                           0)) {
                mTownSent += sent;
            }
        } else {
            getNetPacket()->mSendWait[member]++;
        }
    }
    return mTownSent >= 0x74C15;
}

BOOL dNetTransfer_c::sendPlayer(int member) {
    if (canSendTo(member, 0, 1)) {
        const void *data[4];
        u32 size[4];
        u64 key;
        u8 hasMii;
        int n;
        data[0] = dPlayerMgr::Mgr_c::getCurrentPlayer();
        size[0] = 0x86C0;
        key = dNetWc24::g_wc24.mUserId;
        data[1] = &key;
        size[1] = sizeof(key);
        hasMii = dMiiIcon::hasMyMii();
        data[2] = &hasMii;
        size[2] = sizeof(hasMii);
        n = 3;
        if (hasMii) {
            data[3] = dMiiIcon::getMyMii();
            size[3] = 0x54;
            n = 4;
        }
        if (sendDirect(member, data, size, n, NET_DIRECT_XFER_PLAYER_REPLY, 0,
                       1, 0)) {
            return TRUE;
        }
    } else {
        getNetPacket()->mSendWait[member]++;
    }
    return FALSE;
}

BOOL dNetTransfer_c::sendItems(int member) {
    if (mItemSent < mItemNum || mItemNum == 0) {
        dNetMemberMask_c failed;
        BOOL ok;
        if (member < NET_MEMBER_MAX) {
            ok = canSendTo(member, 0, 1);
            failed.on(member);
        } else {
            ok = canSendToAccepted(0, &failed, 1);
        }
        if (ok) {
            u32 num;
            if (mItemNum != 0) {
                num = 11;
                if (mItemNum - mItemSent < 11) {
                    num = mItemNum - mItemSent;
                }
            } else {
                num = 0;
            }
            u8 last;
            const void *data[12];
            u32 size[12];
            last = mItemSent + num >= mItemNum;
            u32 index = mItemSent;
            data[0] = &last;
            size[0] = sizeof(last);
            for (u32 i = 0; i < num; i++) {
                data[i + 1] = getToCstmSendable(index, mExclude);
                size[i + 1] = 0x2000;
                index++;
            }
            if (sendDirect(member, data, size, num + 1, NET_DIRECT_XFER_ITEMS,
                           0, 1, 0)) {
                mItemSent += num;
                if (mItemNum == 0) {
                    return TRUE;
                }
            }
        } else {
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (failed.isOn(i)) {
                    getNetPacket()->mSendWait[i]++;
                }
            }
        }
    }
    if (mItemNum == 0) {
        return FALSE;
    }
    return mItemSent >= mItemNum;
}

BOOL dNetTransfer_c::sendTownReq() {
    return sendDirect(0, NULL, NULL, 0, NET_DIRECT_XFER_TOWN_REQUEST, 0, 1, 0);
}

BOOL dNetTransfer_c::sendFinish() {
    return sendDirect(0, NULL, NULL, 0, NET_DIRECT_JOIN_DONE, 0, 1, 0);
}

void dNetTransfer_c::setScene(u8 scene) {
    mScene = scene;
}

void dNetTransfer_c::setReady() {
    mStartRecv = 1;
}

BOOL dNetTransfer_c::isAllReady() const {
    int self = getNetMyNo()->get();
    for (int i = 0; i < ARRAY_SIZE(mReady); i++) {
        if (i != self && getNetAccepted()->isOn(i)) {
            if (mReady[i]) {
                if (!dMiiIcon::isMemberDone(i)) {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void dNetSaveEnd_c::init() {
    mStep = 0;
    mSaved.clear();
}

void dNetSaveEnd_c::clearMember(int member) {
    mSaved.off(member);
}

void dNetSaveEnd_c::start(BOOL interrupt) {
    if (isHost()) {
        mSaved.resetAcks(TRUE);
        if (interrupt) {
            getNetSyncBuf()->send(getNetMembers());
            setStep(1);
        } else {
            setStep(0);
        }
    } else {
        mSaved.resetAcks(FALSE);
        if (interrupt) {
            setStep(2);
        } else {
            setStep(0);
        }
    }
}

void dNetSaveEnd_c::execute() {
    switch (getStep()) {
    case 1:
        if (getNetScene()->getLeader() >= 4 && mSaved.isAll()) {
            getNetPacket()->mSuspended = 1;
            callOnLeaveAll();
            getNetWatched()->clear();
            setStep(5);
        }
        break;
    case 2:
        if (isAllInScene(0x39) && getNetScene()->getLeader() >= 4) {
            int self = getNetMyNo()->get();
            getNetSyncBuf()->writeBuf4(self);
            getNetPacket()->begin();
            getNetPacket()->add(getNetSyncBuf()->getBuf4(self), 2);
            getNetPacket()->send(NET_PACKET_SYNC_BUF_MEMBER, 0);
            getNetSyncBuf()->init();
            setStep(3);
        }
        break;
    case 4:
        if (isMemberDisconnected(0)) {
            callOnLeaveAll();
            setStep(5);
        }
        break;
    case 5:
        if (closeNetConnection()) {
            callOnDisconnectAll();
            setStep(6);
        }
        break;
    case 6:
        if (checkDisconnectDoneAll()) {
            endSession(0);
        }
        break;
    }
}

void dNetSaveEnd_c::executeAfterSend() {
    switch (getStep()) {
    case 3:
        getNetPacket()->mSuspended = 1;
        getNetWatched()->clear();
        setStep(4);
        break;
    }
}

void dNetSaveEnd_c::setStep(u8 step) {
    mStep = step;
}

int dNetSaveEnd_c::getStep() const {
    return mStep;
}

void dNetDisconnect_c::init() {
    mMember.clear();
    mStep = 0;
    mAcks.clear();
    mReady.clear();
}

void dNetDisconnect_c::clearMember(int member) {
    mAcks.off(member);
    mReady.off(member);
}

void dNetDisconnect_c::start(BOOL fromJoin) {
    if (!fromJoin) {
        getNetRequest()->mGrantedKind.set(1);
        int self = getNetMyNo()->get();
        getNetRequest()->mGrantedMember.set(self);
    }
    int member = getNetRequest()->mGrantedMember.get();
    if (member == getMyNo()) {
        mMember.set(member);
        mAcks.resetAcks(TRUE);
        setStep(1);
    } else {
        if (isHost()) {
            dNetMemberMask_c mask;
            mask.on(member);
            getNetSyncBuf()->send(&mask);
        }
        mMember.set(member);
        mAcks.resetAcks(FALSE);
        mReady.resetAcks(FALSE);
        mReady.on(member);
        setStep(5);
    }
}

void dNetDisconnect_c::execute() {
    int step = getStep();
    if (step == 0) {
        return;
    }
    int member = mMember.get();
    int self = getNetMyNo()->get();
    switch (step) {
    case 1: {
        u32 expected = getMemberNum() - 1;
        u32 arrived = getNetScene()->countMembers(0x40);
        if (arrived == expected && getNetScene()->getLeader() >= 4) {
            getNetSyncBuf()->writeBuf4(self);
            getNetPacket()->begin();
            getNetPacket()->add(getNetSyncBuf()->getBuf4(self), 2);
            getNetPacket()->send(NET_PACKET_SYNC_BUF_MEMBER, 0);
            getNetSyncBuf()->init();
            getNetPacket()->begin();
            getNetPacket()->send(NET_PACKET_DISCONNECT_START, NET_MEMBER_NONE);
            setStep(2);
        }
        break;
    }
    case 2:
        if (mAcks.isAll()) {
            getNetPacket()->mLeaving.set(self);
            getNetPacket()->mSuspended = 1;
            getNetWatched()->clear();
            setStep(3);
        }
        break;
    case 3:
        if (closeNetConnection()) {
            callOnDisconnect();
            setStep(4);
        }
        break;
    case 5:
        if (getNetScene()->getLeader() >= 4 && mAcks.isOn(self)) {
            getNetPacket()->begin();
            getNetPacket()->send(NET_PACKET_DISCONNECT_ACK, member);
            if (self == 0) {
                g_dNet.mHelloHold.on(member);
            }
            setStep(6);
        }
        break;
    case 7:
        if (checkMemberLeaveDone() && !isMemberConnected(member)) {
            setStep(8);
        }
        break;
    }
}

BOOL dNetDisconnect_c::canDeleteScene() {
    int step = getStep();
    BOOL ret = TRUE;
    switch (step) {
    case 4:
        if (!checkCanFinishLeave()) {
            ret = FALSE;
        }
        if (!checkDisconnectDone()) {
            ret = FALSE;
        }
        break;
    }
    return ret;
}

void dNetDisconnect_c::executePostDelete() {
    switch (getStep()) {
    case 4:
        callOnLeave();
        terminateNetSession(0);
        break;
    case 8:
        callOnMemberLeft();
        int member = mMember.get();
        removeNetMember(member);
        dMiiIcon::removeMember(member);
        clearRequestMember();
        getNetPacket()->mLeaving.clear();
        getNetPacket()->mSuspended = 0;
        init();
        break;
    }
}

void dNetDisconnect_c::executeAfterSend() {
    if (getStep() == 6) {
        int member = mMember.get();
        getNetPacket()->mLeaving.set(member);
        getNetScene()->clearMember(member);
        getNetWatched()->off(member);
        callOnMemberLeaving();
        setStep(7);
    }
}

void dNetDisconnect_c::setStep(u8 step) {
    mStep = step;
}

int dNetDisconnect_c::getStep() const {
    return mStep;
}

void dNetJoinDone_c::init() {
    for (int i = 0; i < ARRAY_SIZE(mDone); i++) {
        mDone[i] = 0;
    }
    mAllDone = 0;
}

void dNetJoinDone_c::clearMember(int member) {
    mDone[member] = 0;
}

void dNetJoinDone_c::setDone(int member, u8 v) {
    mDone[member] = v;
}

void dNetJoinDone_c::reset() {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (i == 0) {
            mDone[i] = TRUE;
        } else if (getNetAccepted()->isOn(i)) {
            mDone[i] = FALSE;
        } else {
            mDone[i] = TRUE;
        }
    }
}

BOOL dNetJoinDone_c::isAllDone() const {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (i != 0 && getNetAccepted()->isOn(i) && !mDone[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

void dNetJoinDone_c::setAllDone(u8 v) {
    mAllDone = v;
}

u8 dNetJoinDone_c::getAllDone() const {
    return mAllDone;
}

void dNetSceneClose_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0xF, 4);
}

void dNetSceneClose_c::init() {
    mSize = 0;
}

void dNetSceneClose_c::clearMember(int member) {
}

u8 *dNetSceneClose_c::getBuf() const {
    return mBuf;
}

u32 dNetSceneClose_c::getSize() const {
    return mSize;
}

void dNetSceneClose_c::setData(const void *data, u32 size) {
    if (size <= 0xF) {
        if (size != 0) {
            cLib::memCpy(mBuf, data, size);
        }
        mSize = size;
    }
}

// lbl_80474BF0 / lbl_80474BFC / lbl_80474C08 (fn_800D9AAC / fn_800D9AD0 /
// fn_800D9BAC)
static const int lbl_80474BF0[3] = {1, 1, 1};
static const dNetSendFuncB lbl_80474BFC[3] = {sendClose0, sendClosePairRing,
                                              sendClose2};
static const dNetApplyFuncB lbl_80474C08[3] = {applyClose0, applyClose1,
                                               applyClose2};

u32 dNetSceneClose_c::getEntrySize(int i) {
    if (i < 3) {
        return lbl_80474BF0[i];
    }
    return 0;
}

void dNetSceneClose_c::write(int member, u8 scene) {
    u8 *dst = mBuf;
    u32 size = 0;
    for (u32 i = 0; i < ARRAY_SIZE(lbl_80474BFC); i++) {
        u8 *entryDst = dst;
        u32 entrySize = size;
        dst += sizeof(dNetSceneCloseEntry_c);
        size += sizeof(dNetSceneCloseEntry_c);
        int len = lbl_80474BFC[i](member, dst, getEntrySize(i), scene);
        if (len >= 0) {
            dNetSceneCloseEntry_c entry;
            u32 length = len;
            entry.set(i, length);
            cLib::memCpy(entryDst, entry.getData(),
                         sizeof(dNetSceneCloseEntry_c));
            dst += length;
            size += length;
        } else {
            dst = entryDst;
            size = entrySize;
        }
    }
    mSize = size;
}

#ifndef BUGFIXES
void dNetSceneClose_c::read(int member, u8 scene) {
    u32 size = getSize();
    if (size != 0) {
        const u8 *src = getBuf();
        for (;;) {
            if (size < sizeof(dNetSceneCloseEntry_c)) {
                break;
            }
            dNetSceneCloseEntry_c raw;
            cLib::memCpy(&raw, src, sizeof(dNetSceneCloseEntry_c));
            src += sizeof(dNetSceneCloseEntry_c);
            size -= sizeof(dNetSceneCloseEntry_c);
            dNetSceneCloseEntry_c entry;
            entry.set(raw);
            int i = entry.getIdx();
            if (i < 3) {
                dNetApplyFuncB func = lbl_80474C08[i];
                u32 len = entry.getSize();
                if (len != 0) {
                    func(member, src, len, scene);
                    src += len;
                    size -= len;
                } else {
                    func(member, NULL, 0, scene);
                }
            }
        }
    }
}
#else
void dNetSceneClose_c::read(int member, u8 scene) {
    const u8 *src = getBuf();
    u32 size = getSize();
    while (size >= sizeof(dNetSceneCloseEntry_c)) {
        dNetSceneCloseEntry_c entry;
        cLib::memCpy(&entry, src, sizeof(entry));
        src += sizeof(entry);
        size -= sizeof(entry);
        u32 idx = entry.getIdx();
        u32 len = entry.getSize();
        if (len > size || idx >= 3 || len > getEntrySize(idx)) {
            return;
        }
        lbl_80474C08[idx](member, len ? src : NULL, len, scene);
        src += len;
        size -= len;
    }
}
#endif

void dNetSceneOpen_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0x15, 4);
}

void dNetSceneOpen_c::init() {
    mSize = 0;
    mRecv = 0;
}

void dNetSceneOpen_c::clearMember(int member) {
}

u8 *dNetSceneOpen_c::getBuf() const {
    return mBuf;
}

u32 dNetSceneOpen_c::getSize() const {
    return mSize;
}

void dNetSceneOpen_c::setData(const void *data, u32 size) {
    if (size <= 0x15) {
        if (size != 0) {
            cLib::memCpy(mBuf, data, size);
        }
        mSize = size;
    }
}

void dNetSceneOpen_c::setRecv(u8 flag) {
    mRecv = flag;
}

int dNetSceneOpen_c::isRecv() const {
    return mRecv;
}

// lbl_80474C14 / lbl_80474C28 / lbl_80474C3C (fn_800D9D74 / fn_800D9D98 /
// fn_800D9E74)
static const int lbl_80474C14[5] = {1, 0, 0, 0, 0};
static const dNetSendFunc lbl_80474C28[5] = {
    sendOpen0, sendOpenInsects, sendOpenFish, sendOpenHouseRoom, sendOpen4};
static const dNetApplyFunc lbl_80474C3C[5] = {applyOpen0, applyOpenInsects,
                                              applyOpenFish, applyOpenHouseRoom,
                                              applyOpen4};

u32 dNetSceneOpen_c::getEntrySize(int i) {
    if (i < 5) {
        return lbl_80474C14[i];
    }
    return 0;
}

void dNetSceneOpen_c::write(int member, u8 scene) {
    u8 *dst = mBuf;
    u32 size = 0;
    for (u32 i = 0; i < ARRAY_SIZE(lbl_80474C28); i++) {
        u8 *entryDst = dst;
        u32 entrySize = size;
        dst += sizeof(dNetSceneOpenEntry_c);
        size += sizeof(dNetSceneOpenEntry_c);
        int len = lbl_80474C28[i](member, dst, getEntrySize(i), scene);
        if (len >= 0) {
            dNetSceneOpenEntry_c entry;
            u32 length = len;
            entry.set(i, length);
            cLib::memCpy(entryDst, entry.getData(),
                         sizeof(dNetSceneOpenEntry_c));
            dst += length;
            size += length;
        } else {
            dst = entryDst;
            size = entrySize;
        }
    }
    mSize = size;
}

#ifndef BUGFIXES
void dNetSceneOpen_c::read(int member, u8 scene) {
    u32 size = getSize();
    if (size != 0) {
        const u8 *src = getBuf();
        for (;;) {
            if (size < sizeof(dNetSceneOpenEntry_c)) {
                break;
            }
            dNetSceneOpenEntry_c raw;
            cLib::memCpy(&raw, src, sizeof(dNetSceneOpenEntry_c));
            src += sizeof(dNetSceneOpenEntry_c);
            size -= sizeof(dNetSceneOpenEntry_c);
            dNetSceneOpenEntry_c entry;
            entry.set(raw);
            int i = entry.getIdx();
            if (i < 5) {
                dNetApplyFunc func = lbl_80474C3C[i];
                u32 len = entry.getSize();
                if (len != 0) {
                    func(member, src, len, scene);
                    src += len;
                    size -= len;
                } else {
                    func(member, NULL, 0, scene);
                }
            }
        }
    }
}
#else
void dNetSceneOpen_c::read(int member, u8 scene) {
    const u8 *src = getBuf();
    u32 size = getSize();
    while (size >= sizeof(dNetSceneOpenEntry_c)) {
        dNetSceneOpenEntry_c entry;
        cLib::memCpy(&entry, src, sizeof(entry));
        src += sizeof(entry);
        size -= sizeof(entry);
        u32 idx = entry.getIdx();
        u32 len = entry.getSize();
        if (len > size || idx >= 5 || len > getEntrySize(idx)) {
            return;
        }
        lbl_80474C3C[idx](member, len ? src : NULL, len, scene);
        src += len;
        size -= len;
    }
}
#endif

void dNetSceneLeave_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0x5, 4);
}

void dNetSceneLeave_c::init() {
    mSize = 0;
}

void dNetSceneLeave_c::clearMember(int member) {
}

u8 *dNetSceneLeave_c::getBuf() const {
    return mBuf;
}

u32 dNetSceneLeave_c::getSize() const {
    return mSize;
}

void dNetSceneLeave_c::setData(const void *data, u32 size) {
    if (size <= 0x5) {
        if (size != 0) {
            cLib::memCpy(mBuf, data, size);
        }
        mSize = size;
    }
}

// .sdata2 807508CC / 807508D0 / 807508D4 (fn_800DA028 / fn_800DA048 /
// fn_800DA0F8)
static const int lbl_807508CC[1] = {1};
static const dNetSendFunc lbl_807508D0[1] = {sendLeave0};
static const dNetApplyFunc lbl_807508D4[1] = {applyLeave0};

u32 dNetSceneLeave_c::getEntrySize(int i) {
    if (i < 1) {
        return lbl_807508CC[i];
    }
    return 0;
}

void dNetSceneLeave_c::write(int member, u8 scene) {
    u8 *dst = mBuf;
    u32 size = 0;
    for (u32 i = 0; i < ARRAY_SIZE(lbl_807508D0); i++) {
        u8 *entryDst = dst;
        u32 entrySize = size;
        dst += sizeof(dNetSceneLeaveEntry_c);
        size += sizeof(dNetSceneLeaveEntry_c);
        int len = lbl_807508D0[i](member, dst, getEntrySize(i), scene);
        if (len >= 0) {
            dNetSceneLeaveEntry_c entry;
            entry.set(i, len);
            cLib::memCpy(entryDst, entry.getData(),
                         sizeof(dNetSceneLeaveEntry_c));
            dst += len;
            size += len;
        } else {
            dst = entryDst;
            size = entrySize;
        }
    }
    mSize = size;
}

#ifndef BUGFIXES
void dNetSceneLeave_c::read(int member, u8 scene) {
    u32 size = getSize();
    if (size != 0) {
        const u8 *src = getBuf();
        for (;;) {
            if (size < sizeof(dNetSceneLeaveEntry_c)) {
                break;
            }
            dNetSceneLeaveEntry_c raw;
            cLib::memCpy(&raw, src, sizeof(dNetSceneLeaveEntry_c));
            src += sizeof(dNetSceneLeaveEntry_c);
            size -= sizeof(dNetSceneLeaveEntry_c);
            dNetSceneLeaveEntry_c entry;
            entry.set(raw);
            int i = entry.getIdx();
            if (i < 1) {
                dNetApplyFunc func = lbl_807508D4[i];
                u32 len = entry.getSize();
                if (len != 0) {
                    func(member, src, len, scene);
                    src += len;
                    size -= len;
                } else {
                    func(member, NULL, 0, scene);
                }
            }
        }
    }
}
#else
void dNetSceneLeave_c::read(int member, u8 scene) {
    const u8 *src = getBuf();
    u32 size = getSize();
    while (size >= sizeof(dNetSceneLeaveEntry_c)) {
        dNetSceneLeaveEntry_c entry;
        cLib::memCpy(&entry, src, sizeof(entry));
        src += sizeof(entry);
        size -= sizeof(entry);
        u32 idx = entry.getIdx();
        u32 len = entry.getSize();
        if (len > size || idx >= 1 || len > getEntrySize(idx)) {
            return;
        }
        lbl_807508D4[idx](member, len ? src : NULL, len, scene);
        src += len;
        size -= len;
    }
}
#endif

void dNetSceneEnter_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0x5, 4);
}

void dNetSceneEnter_c::init() {
    mSize = 0;
    mRecv = 0;
    mRequester = NET_MEMBER_NONE;
}

void dNetSceneEnter_c::clearMember(int member) {
}

u8 *dNetSceneEnter_c::getBuf() const {
    return mBuf;
}

u32 dNetSceneEnter_c::getSize() const {
    return mSize;
}

void dNetSceneEnter_c::setData(const void *data, u32 size) {
    if (size <= 0x5) {
        if (size != 0) {
            cLib::memCpy(mBuf, data, size);
        }
        mSize = size;
    }
}

void dNetSceneEnter_c::setRecv(u8 flag) {
    mRecv = flag;
}

int dNetSceneEnter_c::isRecv() const {
    return mRecv;
}

// .sdata2 807508D8 / 807508DC / 807508E0 (fn_800DA2C4 / fn_800DA2E4 /
// fn_800DA394)
static const int lbl_807508D8[1] = {1};
static const dNetSendFunc lbl_807508DC[1] = {sendEnter0};
static const dNetApplyFunc lbl_807508E0[1] = {applyEnter0};

u32 dNetSceneEnter_c::getEntrySize(int i) {
    if (i < 1) {
        return lbl_807508D8[i];
    }
    return 0;
}

void dNetSceneEnter_c::write(int member, u8 scene) {
    u8 *dst = mBuf;
    u32 size = 0;
    for (u32 i = 0; i < ARRAY_SIZE(lbl_807508DC); i++) {
        u8 *entryDst = dst;
        u32 entrySize = size;
        dst += sizeof(dNetSceneEnterEntry_c);
        size += sizeof(dNetSceneEnterEntry_c);
        int len = lbl_807508DC[i](member, dst, getEntrySize(i), scene);
        if (len >= 0) {
            dNetSceneEnterEntry_c entry;
            entry.set(i, len);
            cLib::memCpy(entryDst, entry.getData(),
                         sizeof(dNetSceneEnterEntry_c));
            dst += len;
            size += len;
        } else {
            dst = entryDst;
            size = entrySize;
        }
    }
    mSize = size;
}

#ifndef BUGFIXES
void dNetSceneEnter_c::read(int member, u8 scene) {
    u32 size = getSize();
    if (size != 0) {
        const u8 *src = getBuf();
        for (;;) {
            if (size < sizeof(dNetSceneEnterEntry_c)) {
                break;
            }
            dNetSceneEnterEntry_c raw;
            cLib::memCpy(&raw, src, sizeof(dNetSceneEnterEntry_c));
            src += sizeof(dNetSceneEnterEntry_c);
            size -= sizeof(dNetSceneEnterEntry_c);
            dNetSceneEnterEntry_c entry;
            entry.set(raw);
            int i = entry.getIdx();
            if (i < 1) {
                dNetApplyFunc func = lbl_807508E0[i];
                u32 len = entry.getSize();
                if (len != 0) {
                    func(member, src, len, scene);
                    src += len;
                    size -= len;
                } else {
                    func(member, NULL, 0, scene);
                }
            }
        }
    }
}
#else
void dNetSceneEnter_c::read(int member, u8 scene) {
    const u8 *src = getBuf();
    u32 size = getSize();
    while (size >= sizeof(dNetSceneEnterEntry_c)) {
        dNetSceneEnterEntry_c entry;
        cLib::memCpy(&entry, src, sizeof(entry));
        src += sizeof(entry);
        size -= sizeof(entry);
        u32 idx = entry.getIdx();
        u32 len = entry.getSize();
        if (len > size || idx >= 1 || len > getEntrySize(idx)) {
            return;
        }
        lbl_807508E0[idx](member, len ? src : NULL, len, scene);
        src += len;
        size -= len;
    }
}
#endif

void dNetSceneHandover_c::create(EGG::Heap *heap) {
    mBuf = (u8 *)heap->alloc(0xA, 4);
}

void dNetSceneHandover_c::init() {
    mSize = 0;
    mNewOwner = NET_MEMBER_NONE;
}

void dNetSceneHandover_c::clearMember(int member) {
}

u8 *dNetSceneHandover_c::getBuf() const {
    return mBuf;
}

u32 dNetSceneHandover_c::getSize() const {
    return mSize;
}

void dNetSceneHandover_c::setData(const void *data, u32 size) {
    if (size <= 0xA) {
        if (size != 0) {
            cLib::memCpy(mBuf, data, size);
        }
        mSize = size;
    }
}

// .sdata2 807508E8 / 807508F0 / 807508F8 (fn_800DA54C / fn_800DA56C /
// fn_800DA650)
static const int lbl_807508E8[2] = {1, 1};
static const dNetMemberSendFunc lbl_807508F0[2] = {sendHandover0,
                                                   sendHandoverFlag};
static const dNetMemberApplyFunc lbl_807508F8[2] = {applyHandover0,
                                                    applyHandoverFlag};

u32 dNetSceneHandover_c::getEntrySize(int i) {
    if (i < 2) {
        return lbl_807508E8[i];
    }
    return 0;
}

void dNetSceneHandover_c::write(u8 scene) {
    u8 *dst = mBuf;
    u32 size = 0;
    int to = mNewOwner;
    int self = getMyNo();
    for (u32 i = 0; i < ARRAY_SIZE(lbl_807508F0); i++) {
        u8 *entryDst = dst;
        u32 entrySize = size;
        dst += sizeof(dNetSceneHandoverEntry_c);
        size += sizeof(dNetSceneHandoverEntry_c);
        int len = lbl_807508F0[i](to, self, dst, getEntrySize(i), scene);
        if (len >= 0) {
            dNetSceneHandoverEntry_c entry;
            u32 length = len;
            entry.set(i, length);
            cLib::memCpy(entryDst, entry.getData(),
                         sizeof(dNetSceneHandoverEntry_c));
            dst += length;
            size += length;
        } else {
            dst = entryDst;
            size = entrySize;
        }
    }
    mSize = size;
}

#ifndef BUGFIXES
void dNetSceneHandover_c::read(int self, int from, u8 scene) {
    u32 size = getSize();
    if (size != 0) {
        const u8 *src = getBuf();
        for (;;) {
            if (size < sizeof(dNetSceneHandoverEntry_c)) {
                break;
            }
            dNetSceneHandoverEntry_c raw;
            cLib::memCpy(&raw, src, sizeof(dNetSceneHandoverEntry_c));
            src += sizeof(dNetSceneHandoverEntry_c);
            size -= sizeof(dNetSceneHandoverEntry_c);
            dNetSceneHandoverEntry_c entry;
            entry.set(raw);
            int i = entry.getIdx();
            if (i < 2) {
                dNetMemberApplyFunc func = lbl_807508F8[i];
                u32 len = entry.getSize();
                if (len != 0) {
                    func(self, from, src, len, scene);
                    src += len;
                    size -= len;
                } else {
                    func(self, from, NULL, 0, scene);
                }
            }
        }
    }
}
#else
void dNetSceneHandover_c::read(int self, int from, u8 scene) {
    const u8 *src = getBuf();
    u32 size = getSize();
    while (size >= sizeof(dNetSceneHandoverEntry_c)) {
        dNetSceneHandoverEntry_c entry;
        cLib::memCpy(&entry, src, sizeof(entry));
        src += sizeof(entry);
        size -= sizeof(entry);
        u32 idx = entry.getIdx();
        u32 len = entry.getSize();
        if (len > size || idx >= 2 || len > getEntrySize(idx) ||
            (idx == 1 && len != 1)) {
            return;
        }
        lbl_807508F8[idx](self, from, len ? src : NULL, len, scene);
        src += len;
        size -= len;
    }
}
#endif

void dNetScene_c::init(int arg) {
    mState = NET_SCENE_STATE_NONE;
    mOrder.init();
    clearScenes(arg);
    mLeaderAcks.clear();
    mLeader.init();
    mMsgAcks.clear();
    mLeaveAcked = 0;
    mLeaving = NET_MEMBER_NONE;
    mHandoverAcks.clear();
}

void dNetScene_c::clearMember(int member) {
    mOrder.clear(member);
    clearScene(member, 0);
    mHandoverAcks.off(member);
}

void dNetScene_c::setLeaveAcked(bool v) {
    mLeaveAcked = v;
}

bool dNetScene_c::getLeaveAcked() const {
    return mLeaveAcked;
}

bool dNetScene_c::isHandoverAcked() const {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (!mHandoverAcks.isOn(i)) {
            return false;
        }
    }
    return true;
}

void dNetScene_c::reset() {
    init(0);
    setScene(0, dScene::getCurrentScene(), 0);
    setOwner(0, 1, 0);
}

void dNetScene_c::enter(int member, u8 scene) {
    setScene(member, scene, 0);
    setOwner(member, 1, 0);
}

int dNetScene_c::getState() const {
    return mState;
}

void dNetScene_c::setState(int state) {
    mState = state;
}

void dNetScene_c::clearScenes(int arg) {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        clearScene(i, arg);
    }
}

void dNetScene_c::clearScene(int member, int arg) {
    setScene(member, NET_SCENE_NONE, arg);
    setOwner(member, 0, arg);
}

void dNetScene_c::setScene(int member, u8 scene, int) {
    int member20 = mLeaving;
    int memberAC = getNetHoldBuf()->mEnterMember;
    u8 old = NET_SCENE_NONE;
    BOOL check = FALSE;
    if (member20 == member || memberAC == member) {
        check = TRUE;
    }
    if (check) {
        old = getScene(member);
    }
    mScenes.setScene(member, scene);
    if (check && old != getScene(member)) {
        if (member20 == member) {
            mLeaving = NET_MEMBER_NONE;
        }
        if (memberAC == member && getNetHoldBuf()->mHoldState != 2) {
            getNetHoldBuf()->mHoldState = 2;
        }
    }
}

u8 dNetScene_c::getScene(int member) const {
    return mScenes.getScene(member);
}

void dNetScene_c::setOwner(int member, int flag, int) {
    mScenes.setOwner(member, flag);
    int memberAC = getNetHoldBuf()->mOldOwner;
    if (memberAC < NET_MEMBER_MAX && getNetHoldBuf()->mHoldState == 0) {
        int my = getMyNo();
        int found = findOwner(getScene(my));
        if (found < NET_MEMBER_MAX && found != memberAC) {
            if (found == my) {
                getNetHoldBuf()->mHoldState = 1;
            } else {
                getNetHoldBuf()->mHoldState = 2;
            }
            if (found == my) {
                dNetSceneMsg_c msgA;
                msgA.set(memberAC, 1, NET_SCENE_NONE, 1, 0, NET_MEMBER_NONE);
                dNetSceneMsg_c msgB;
                msgB.set(my, 0, NET_SCENE_NONE, 1, 1, memberAC);
                for (int i = 0; i < ARRAY_SIZE(mScenes.mScene); i++) {
                    if (memberAC != i && my != i && isStayingMember(i)) {
                        getNetPacket()->begin();
                        getNetPacket()->add(&msgA, sizeof(msgA));
                        getNetPacket()->send(NET_PACKET_SCENE_STATE, i);
                        getNetPacket()->begin();
                        getNetPacket()->add(&msgB, sizeof(msgB));
                        getNetPacket()->send(NET_PACKET_SCENE_STATE, i);
                    }
                }
            }
        }
    }
}

int dNetScene_c::isOwner(int member) const {
    return mScenes.isOwner(member);
}

int dNetScene_c::findOwner(u8 scene) const {
    return mScenes.findOwner(scene);
}

int dNetScene_c::findNonOwner(u8 scene, int start) const {
    return mScenes.findNonOwner(scene, start);
}

int dNetScene_c::countMembers(u8 scene) const {
    return mScenes.countMembers(scene);
}

bool dNetScene_c::isAllIn(u8 scene) const {
    return mScenes.isAllIn(scene);
}

void dNetScene_c::addOrder(int member, u8 scene) {
    u32 order;
    if (getLeader() < NET_MEMBER_MAX) {
        order = 1;
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            u32 o = mOrder.getOrder(i);
            if (o < NET_MEMBER_MAX && order <= o) {
                order = o + 1;
            }
        }
    } else {
        order = 0;
    }
    mOrder.setOrder(member, order);
    mOrder.setScene(member, scene);
    if (order == 0) {
        dNetMemberScene_c msg;
        msg.set(member, scene);
        getNetPacket()->begin();
        getNetPacket()->add(msg.getData(), sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_LEADER, NET_MEMBER_NONE);
        if (member == 0) {
            getNetScene()->mLeaderAcks.on(0);
        } else {
            getNetPacket()->begin();
            getNetPacket()->send(NET_PACKET_SCENE_LEADER_ACK, member);
        }
    }
}

void dNetScene_c::removeOrder(int member) {
    mOrder.setOrder(member, NET_MEMBER_NONE);
    mOrder.setScene(member, NET_SCENE_NONE);
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        u32 o = mOrder.getOrder(i);
        if (o < NET_MEMBER_MAX) {
            mOrder.setOrder(i, o - 1);
        }
    }
    int leader = getLeader();
    if (leader < NET_MEMBER_MAX) {
        dNetMemberScene_c msg;
        msg.set(leader, mOrder.getScene(leader));
        getNetPacket()->begin();
        getNetPacket()->add(msg.getData(), sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_LEADER, NET_MEMBER_NONE);
        if (leader == 0) {
            getNetScene()->mLeaderAcks.on(0);
        } else {
            getNetPacket()->begin();
            getNetPacket()->send(NET_PACKET_SCENE_LEADER_ACK, leader);
        }
    } else {
        getNetPacket()->begin();
        getNetPacket()->send(NET_PACKET_SCENE_NO_LEADER, NET_MEMBER_NONE);
    }
}

int dNetScene_c::getLeader() const {
    if (isHost()) {
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (mOrder.getOrder(i) == 0) {
                return i;
            }
        }
        return NET_MEMBER_NONE;
    }
    return mLeader.getMember();
}

u8 dNetScene_c::getLeaderScene() const {
    int leader = getLeader();
    if (leader < NET_MEMBER_MAX) {
        if (isHost()) {
            return mOrder.getScene(leader);
        }
        return mLeader.getScene();
    }
    return NET_SCENE_NONE;
}

bool dNetScene_c::hasOrder() const {
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (mOrder.getOrder(i) < NET_MEMBER_MAX) {
            return true;
        }
    }
    return false;
}

void dNetScene_c::setLeader(const dNetMemberScene_c &leader) {
    mLeader.set(leader);
}

void dNetScene_c::clearLeader() {
    mLeader.init();
}

void dNetScene_c::enterLeaderScene() {
    mMsgAcks.resetAcks(TRUE);
    BOOL first;
    u8 scene;
    int my = getMyNo();
    scene = getLeaderScene();
    first = countMembers(scene) == 0;
    setScene(my, scene, 0);
    setOwner(my, first, 0);
    if (getMemberNum() > 1) {
        dNetSceneMsg_c msg;
        msg.set(my, 1, scene, 1, first, my);
        getNetPacket()->begin();
        getNetPacket()->add(&msg, sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_STATE, NET_MEMBER_NONE);
    }
}

void dNetScene_c::executeRecv() {
    if (isNetStopped()) {
        return;
    }
    if (!isSessionActive()) {
        return;
    }
    int my = getMyNo();
    switch (getState()) {
    case NET_SCENE_STATE_WAIT_TURN: {
        int leader = getLeader();
        if (leader == my && mLeaderAcks.isAll()) {
            u8 scene = getScene(my);
            if (isOwner(my)) {
                int free = findNonOwner(scene, 0);
                if (free >= NET_MEMBER_MAX) {
                    setState(NET_SCENE_STATE_CLOSE);
                } else {
                    for (int i = 0; i < ARRAY_SIZE(mScenes.mScene); i++) {
                        if (my == i) {
                            mHandoverAcks.on(i);
                        } else if (isStayingMember(i)) {
                            if (scene == getScene(i)) {
                                mHandoverAcks.off(i);
                            } else {
                                mHandoverAcks.on(i);
                            }
                        } else {
                            mHandoverAcks.on(i);
                        }
                    }
                    getNetSceneHandover()->mNewOwner = free;
                    getNetPacket()->begin();
                    getNetPacket()->send(NET_PACKET_SCENE_HANDOVER_REQUEST,
                                         NET_DEST_SCENE_OTHERS);
                    setState(NET_SCENE_STATE_HANDOVER);
                }
            } else {
                getNetScene()->setLeaveAcked(0);
                getNetPacket()->begin();
                getNetPacket()->send(NET_PACKET_SCENE_EXIT_REQUEST,
                                     NET_DEST_SCENE_OWNER);
                setState(NET_SCENE_STATE_LEAVE);
            }
        }
        break;
    }
    case NET_SCENE_STATE_ENTER:
        if (mMsgAcks.isAll()) {
            u8 scene = getLeaderScene();
            int member = findOwner(scene);
            if (member < NET_MEMBER_MAX) {
                getNetPacket()->begin();
                getNetPacket()->send(NET_PACKET_SCENE_ENTER_REQUEST, member);
                getNetSceneEnter()->setRecv(0);
                setState(NET_SCENE_STATE_WAIT_ENTER);
            } else {
                if (isHost()) {
                    getNetSceneOpen()->write(0, scene);
                    getNetSceneOpen()->setRecv(1);
                } else {
                    getNetPacket()->begin();
                    getNetPacket()->send(NET_PACKET_SCENE_LOAD_REQUEST, 0);
                    getNetSceneOpen()->setRecv(0);
                }
                setState(NET_SCENE_STATE_WAIT_OPEN);
            }
        }
        break;
    case NET_SCENE_STATE_WAIT_OPEN:
        if (getNetSceneOpen()->isRecv()) {
            u8 scene = getLeaderScene();
            getNetSceneOpen()->read(my, scene);
            setState(NET_SCENE_STATE_CREATE);
        }
        break;
    case NET_SCENE_STATE_WAIT_ENTER:
        if (getNetSceneEnter()->isRecv()) {
            u8 scene = getNetScene()->getLeaderScene();
            getNetSceneEnter()->read(my, scene);
            setState(NET_SCENE_STATE_CREATE);
        }
        break;
    case NET_SCENE_STATE_CREATED:
        if (mMsgAcks.isAll()) {
            if (isHost()) {
                removeOrder(0);
            } else {
                dNetSceneReq_c req;
                req.mLeave = 1;
                req.mScene = NET_SCENE_NONE;
                getNetPacket()->begin();
                getNetPacket()->add(&req, sizeof(req));
                getNetPacket()->send(NET_PACKET_SCENE_REQUEST, 0);
            }
            setState(NET_SCENE_STATE_END);
        }
        break;
    case NET_SCENE_STATE_END: {
        int leader = getLeader();
        if (leader != my) {
            setState(NET_SCENE_STATE_NONE);
        }
        break;
    }
    }
}

void dNetScene_c::execute() {
    if (isNetStopped()) {
        return;
    }
    if (!isSessionActive()) {
        return;
    }
    int my = getMyNo();
    switch (getNetScene()->getState()) {
    case NET_SCENE_STATE_REQUEST:
        mLeaderAcks.resetAcks(FALSE);
        u8 scene = dScene::getNextScene();
        if (isHost()) {
            addOrder(my, scene);
        } else {
            dNetSceneReq_c req;
            req.mLeave = 0;
            req.mScene = scene;
            getNetPacket()->begin();
            getNetPacket()->add(&req, sizeof(req));
            getNetPacket()->send(NET_PACKET_SCENE_REQUEST, 0);
        }
        setState(NET_SCENE_STATE_WAIT_TURN);
        break;
    }
}

void dNetScene_c::executePostDelete() {
    if (isNetStopped()) {
        return;
    }
    switch (getState()) {
    case NET_SCENE_STATE_CLOSE_SEND: {
        int my = getMyNo();
        if (getMemberNum() > 1) {
            if (my != 0) {
                getNetPacket()->begin();
                u32 size = getNetSceneClose()->getSize();
                if (size != 0) {
                    getNetPacket()->add(getNetSceneClose()->getBuf(), size);
                }
                getNetPacket()->send(NET_PACKET_SCENE_SAVE_HOST, 0);
            } else {
                u8 scene = getNetScene()->getScene(0);
                getNetSceneClose()->read(0, scene);
            }
        }
        mMsgAcks.resetAcks(TRUE);
        clearScene(my, 0);
        if (getMemberNum() > 1) {
            dNetSceneMsg_c msg;
            msg.set(my, 1, NET_SCENE_NONE, 1, 0, my);
            getNetPacket()->begin();
            getNetPacket()->add(&msg, sizeof(msg));
            getNetPacket()->send(NET_PACKET_SCENE_STATE, NET_MEMBER_NONE);
        }
        getNetSceneClose()->init();
        setState(NET_SCENE_STATE_ENTER);
        break;
    }
    case NET_SCENE_STATE_LEAVE_WRITTEN:
        setState(NET_SCENE_STATE_LEAVE_SEND);
        break;
    case NET_SCENE_STATE_HANDOVER_WRITTEN:
        setState(NET_SCENE_STATE_HANDOVER_SEND);
        break;
    }
}

void dNetScene_c::executeAfterSend() {
    if (isNetStopped()) {
        return;
    }
    int state = getState();
    int my = getMyNo();
    switch (state) {
    case NET_SCENE_STATE_LEAVE_SEND: {
        my = getMyNo();
        mMsgAcks.resetAcks(TRUE);
        u8 scene = getNetScene()->getScene(my);
        int member = getNetScene()->findOwner(scene);
        getNetPacket()->begin();
        u32 size = getNetSceneLeave()->getSize();
        if (size != 0) {
            getNetPacket()->add(getNetSceneLeave()->getBuf(), size);
        }
        getNetPacket()->send(NET_PACKET_SCENE_EXIT_DATA, member);
        getNetSceneLeave()->init();
        clearScene(my, 0);
        dNetSceneMsg_c msg;
        msg.set(my, 1, NET_SCENE_NONE, 1, 0, my);
        getNetPacket()->begin();
        getNetPacket()->add(&msg, sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_STATE, NET_MEMBER_NONE);
        setState(NET_SCENE_STATE_ENTER);
        break;
    }
    case NET_SCENE_STATE_HANDOVER_SEND: {
        int member = getNetSceneHandover()->mNewOwner;
        mMsgAcks.resetAcks(TRUE);
        getNetPacket()->begin();
        u32 size = getNetSceneHandover()->getSize();
        if (size != 0) {
            getNetPacket()->add(getNetSceneHandover()->getBuf(), size);
        }
        getNetPacket()->send(NET_PACKET_SCENE_HANDOVER_DATA, member);
        getNetSceneHandover()->init();
        clearScene(my, 0);
        setOwner(member, 1, 0);
        dNetSceneMsg_c msg;
        msg.set(my, 1, NET_SCENE_NONE, 1, 0, NET_MEMBER_NONE);
        getNetPacket()->begin();
        getNetPacket()->add(&msg, sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_STATE, member);
        msg.set(member, 0, NET_SCENE_NONE, 1, 1, my);
        getNetPacket()->begin();
        getNetPacket()->add(&msg, sizeof(msg));
        getNetPacket()->send(NET_PACKET_SCENE_STATE, member);
        setState(NET_SCENE_STATE_ENTER);
        break;
    }
    }
    int member = getNetSceneEnter()->mRequester;
    if (member < NET_MEMBER_MAX) {
        u8 scene = getNetScene()->getLeaderScene();
        getNetSceneEnter()->write(member, scene);
        getNetPacket()->begin();
        u32 size = getNetSceneEnter()->getSize();
        if (size != 0) {
            getNetPacket()->add(getNetSceneEnter()->getBuf(), size);
        }
        getNetPacket()->send(NET_PACKET_SCENE_ENTER_DATA, member);
        getNetHoldBuf()->mEnterMember = member;
        getNetHoldBuf()->mHoldState = 0;
        getNetSceneEnter()->init();
    }
}

// ---- dNetSyncBuf_c

void dNetSyncBuf_c::create(EGG::Heap *heap) {
    mBuf0 = (u8 *)heap->alloc(SYNC_BUF0_SIZE, 4);
    mBuf4 = (u8 *)heap->alloc(SYNC_BUF4_SIZE * (NET_MEMBER_MAX - 1), 4);
    mBuf8 = (u8 *)heap->alloc(SYNC_BUF8_SIZE * (NET_MEMBER_MAX - 1), 4);
}

void dNetSyncBuf_c::init() {
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        mSize[i] = 0;
    }
}

void dNetSyncBuf_c::clearMember(int member) {
    mSize[member] = 0;
}

u8 *dNetSyncBuf_c::getBuf0() const {
    return mBuf0;
}

static const u32 l_buf0Size[] = {1, 1};
static const dNetSyncFunc l_buf0FuncA[] = {fillSyncBuf0_0, fillSyncBuf0_1};
static const dNetSyncFunc l_buf0FuncB[] = {applySyncBuf0_0, applySyncBuf0_1};
static u32 l_buf0Offset[] = {0, 1};

u32 dNetSyncBuf_c::getBuf0Size(int i) {
    if (i < 2) {
        return l_buf0Size[i];
    }
    return 0;
}

u8 *dNetSyncBuf_c::getBuf0Data(int i) const {
    if (i < 2 && mBuf0 != NULL) {
        return mBuf0 + l_buf0Offset[i];
    }
    return NULL;
}

void dNetSyncBuf_c::writeBuf0(int arg) {
    for (u32 i = 0; i < 2; i++) {
        l_buf0FuncA[i](arg, getBuf0Data(i), getBuf0Size(i));
    }
}

void dNetSyncBuf_c::readBuf0(int arg) {
    for (u32 i = 0; i < 2; i++) {
        l_buf0FuncB[i](arg, getBuf0Data(i), getBuf0Size(i));
    }
}

u8 *dNetSyncBuf_c::getBuf4(int member) const {
    if (member != 0) {
        return mBuf4 + (member - 1) * SYNC_BUF4_SIZE;
    }
    return NULL;
}

static const u32 l_buf4Size[] = {1, 1};
static const dNetSyncFunc l_buf4FuncA[] = {fillSyncBuf4_0, fillSyncBuf4LogFlag};
static const dNetSyncFunc l_buf4FuncB[] = {applySyncBuf4_0,
                                           applySyncBuf4LogFlag};
static u32 l_buf4Offset[] = {0, 1};

u32 dNetSyncBuf_c::getBuf4Size(int i) {
    if (i < 2) {
        return l_buf4Size[i];
    }
    return 0;
}

u8 *dNetSyncBuf_c::getBuf4Data(int member, int i) const {
    if (i < 2 && mBuf4 != NULL) {
        return mBuf4 + (member - 1) * SYNC_BUF4_SIZE + l_buf4Offset[i];
    }
    return NULL;
}

void dNetSyncBuf_c::writeBuf4(int member) {
    for (u32 i = 0; i < 2; i++) {
        l_buf4FuncA[i](member, getBuf4Data(member, i), getBuf4Size(i));
    }
}

void dNetSyncBuf_c::readBuf4(int member) {
    for (u32 i = 0; i < 2; i++) {
        l_buf4FuncB[i](member, getBuf4Data(member, i), getBuf4Size(i));
    }
}

u8 *dNetSyncBuf_c::getBuf8(int member) const {
    if (member != 0) {
        return mBuf8 + (member - 1) * SYNC_BUF8_SIZE;
    }
    return NULL;
}

static const u32 l_buf8Size[] = {1, 0x2E, 0x3040};
static const dNetSyncPackFunc l_buf8Pack[] = {
    packSyncBuf8None, packSyncBuf8Host, packSyncBuf8Animal};
static const dNetSyncUnpackFunc l_buf8Unpack[] = {
    unpackSyncBuf8None, unpackSyncBuf8Host, unpackSyncBuf8Animal};

u32 dNetSyncBuf_c::getBuf8Size(int i) {
    if (i < 3) {
        return l_buf8Size[i];
    }
    return 0;
}

void dNetSyncBuf_c::pack(int member, const dNetMemberMask_c *mask) {
    u8 *p = getBuf8(member);
    cLib::memCpy(p, mask, sizeof(*mask));
    u32 total;
    u32 i;
    p += 2;
    for (i = 0, total = 2; i < 3; i++) {
        u8 *flag = p;
        p++;
        total++;
        u32 size = getBuf8Size(i);
        BOOL ok = l_buf8Pack[i](member, mask, p, size);
        *flag = ok;
        if (ok) {
            p += size;
            total += size;
        }
    }
    mSize[member] = total;
}

#ifndef BUGFIXES
void dNetSyncBuf_c::unpack(const dNetMemberMask_c &mask, const u8 *data,
                           u32 size) {
    if (size == 0) {
        return;
    }
    for (u32 i = 0; size != 0; i++) {
        if (i >= 3) {
            break;
        }
        if (size > SYNC_BUF8_SIZE - 2) {
            break;
        }
        bool on = *data;
        data++;
        size--;
        if (on) {
            u32 n = getBuf8Size(i);
            l_buf8Unpack[i](mask, data, n);
            data += n;
            size -= n;
        }
    }
}
#else
void dNetSyncBuf_c::unpack(const dNetMemberMask_c &mask, const u8 *data,
                           u32 size) {
    if (data == NULL || size > SYNC_BUF8_SIZE - 2) {
        return;
    }
    for (u32 i = 0; i < 3 && size != 0; i++) {
        bool on = *data++;
        size--;
        if (on) {
            u32 n = getBuf8Size(i);
            if (n > size) {
                return;
            }
            l_buf8Unpack[i](mask, data, n);
            data += n;
            size -= n;
        }
    }
}
#endif

void dNetSyncBuf_c::send(const dNetMemberMask_c *mask) {
    for (int i = 0; i < ARRAY_SIZE(mSize); i++) {
        if (i != 0 && isStayingMember(i)) {
            pack(i, mask);
            getNetPacket()->begin();
            u32 size = mSize[i];
            if (size != 0) {
                getNetPacket()->add(getBuf8(i), size);
            }
            getNetPacket()->send(NET_PACKET_SYNC_BUF, i);
            mSize[i] = 0;
        }
    }
}

// ---- dNetFlag_c

void dNetFlag_c::init() {
    mStarted = 0;
}

void dNetFlag_c::clearMember(int) {
}

// ---- the state (805B2160) and its accessors

dNet_c g_dNet;

namespace dNet {

dNetMemberNo_c *getNetMyNo() {
    return &g_dNet.mMyNo;
}

dNetMemberMask_c *getNetAccepted() {
    return &g_dNet.mAccepted;
}

dNetMemberMask_c *getNetMembers() {
    return &g_dNet.mMembers;
}

dNetMemberMask_c *getNetWatched() {
    return &g_dNet.mWatched;
}

dNetInBuf_c *getNetInBuf() {
    return &g_dNet.mInBuf;
}

dNetOutBuf_c *getNetOutBuf() {
    return &g_dNet.mOutBuf;
}

dNetPacket_c *getNetPacket() {
    return &g_dNet.mPacket;
}

dNetHoldBuf_c *getNetHoldBuf() {
    return &g_dNet.mHoldBuf;
}

dNetRequest_c *getNetRequest() {
    return &g_dNet.mRequest;
}

dNetTransfer_c *getNetTransfer() {
    return &g_dNet.mTransfer;
}

dNetSaveEnd_c *getNetSaveEnd() {
    return &g_dNet.mSaveEnd;
}

dNetDisconnect_c *getNetDisconnect() {
    return &g_dNet.mDisconnect;
}

dNetJoinDone_c *getNetJoinDone() {
    return &g_dNet.mJoinDone;
}

dNetSceneClose_c *getNetSceneClose() {
    return &g_dNet.mSceneClose;
}

dNetSceneOpen_c *getNetSceneOpen() {
    return &g_dNet.mSceneOpen;
}

dNetSceneLeave_c *getNetSceneLeave() {
    return &g_dNet.mSceneLeave;
}

dNetSceneEnter_c *getNetSceneEnter() {
    return &g_dNet.mSceneEnter;
}

dNetSceneHandover_c *getNetSceneHandover() {
    return &g_dNet.mSceneHandover;
}

dNetScene_c *getNetScene() {
    return &g_dNet.mScene;
}

dNetSyncBuf_c *getNetSyncBuf() {
    return &g_dNet.mSyncBuf;
}

dNetFlag_c *getNetMgrStart() {
    return &g_dNet.mMgrStart;
}

void setSessionActive(bool active) {
    g_dNet.mActive = active;
}

bool getSessionActive() {
    return g_dNet.mActive;
}

bool isMemberIn(int member) {
    return g_dNet.mMembers.isOn(member);
}

BOOL isStayingMember(int member) {
    if (member == getNetPacket()->mLeaving.get()) {
        return FALSE;
    }
    return isMemberIn(member);
}

void setJoined(u8 joined) {
    g_dNet.mJoined = joined;
}

BOOL isJoined() {
    return g_dNet.mJoined;
}

void setNetPlayerInfo() {
    g_dNet.mPlayerInfo.set();
}

void clearNetPlayerInfo() {
    g_dNet.mPlayerInfo.clear();
}

dNetPlayerInfo::Info_c *getNetPlayerInfo() {
    return &g_dNet.mPlayerInfo;
}

// ---- the session

void createNetState(EGG::Heap *heap) {
    getNetInBuf()->create(heap);
    getNetOutBuf()->create(heap);
    getNetPacket()->create(heap);
    getNetHoldBuf()->create(heap);
    getNetSyncBuf()->create(heap);
    getNetSceneClose()->create(heap);
    getNetSceneOpen()->create(heap);
    getNetSceneLeave()->create(heap);
    getNetSceneEnter()->create(heap);
    getNetSceneHandover()->create(heap);
    resetNetState(1);
}

static inline void clearMemberState(int member) {
    g_dNet.mTime[member] = 0;
    g_dNet.mSendCount[member] = g_dNet.mRecvCount[member] = 0;
}

void resetNetState(int arg) {
    for (int i = 0; i < NET_MEMBER_MAX; i++) {
        clearMemberState(i);
    }
    g_dNet.mJoinStarted = 0;
    g_dNet.mErrorReported = 0;
    g_dNet.mError = 0;
    g_dNet.mSavedSlot = 7;
    g_dNet.mTimePending = 0;
    clearNetPlayerInfo();
    setJoined(0);
    setSessionActive(0);
    getNetMgrStart()->init();
    getNetSyncBuf()->init();
    getNetScene()->init(arg);
    getNetSceneHandover()->init();
    getNetSceneEnter()->init();
    getNetSceneLeave()->init();
    getNetSceneOpen()->init();
    getNetSceneClose()->init();
    getNetJoinDone()->init();
    getNetDisconnect()->init();
    getNetSaveEnd()->init();
    getNetTransfer()->init();
    getNetRequest()->init();
    getNetHoldBuf()->init();
    getNetPacket()->init();
    getNetOutBuf()->init();
    getNetInBuf()->init();
    g_dNet.mHelloHold.clear();
    g_dNet.mHelloReply.clear();
    g_dNet.mHelloSend.clear();
    g_dNet.mHelloRecv.clear();
    g_dNet.mLinked.clear();
    getNetWatched()->clear();
    getNetMembers()->clear();
    getNetAccepted()->clear();
    getNetMyNo()->clear();
}

void removeNetMember(int member) {
    clearMemberState(member);
    getNetMgrStart()->clearMember(member);
    getNetSyncBuf()->clearMember(member);
    getNetScene()->clearMember(member);
    getNetSceneHandover()->clearMember(member);
    getNetSceneEnter()->clearMember(member);
    getNetSceneLeave()->clearMember(member);
    getNetSceneOpen()->clearMember(member);
    getNetSceneClose()->clearMember(member);
    getNetJoinDone()->clearMember(member);
    getNetDisconnect()->clearMember(member);
    getNetSaveEnd()->clearMember(member);
    getNetTransfer()->clearMember(member);
    getNetRequest()->clearMember(member);
    getNetHoldBuf()->clearMember(member);
    getNetPacket()->clearMember(member);
    getNetOutBuf()->clearMember(member);
    getNetInBuf()->clearMember(member);
    g_dNet.mHelloHold.off(member);
    g_dNet.mHelloReply.off(member);
    g_dNet.mHelloSend.off(member);
    g_dNet.mHelloRecv.off(member);
    g_dNet.mLinked.off(member);
    getNetWatched()->off(member);
    getNetMembers()->off(member);
    getNetAccepted()->off(member);
}

void setupNetMgr() {
    setNetPlayerInfo();
    u8 *bufs[NET_MEMBER_MAX - 1];
    u8 *buffer = getNetInBuf()->getBufferAddr();
    for (int i = 0; i < ARRAY_SIZE(bufs); i++) {
#ifndef BUGFIXES
        bufs[i] = buffer;
#else
        bufs[i] = getNetInBuf()->getStagingBuf(i);
#endif
        for (int j = 0; j < dNetInBuf_c::SLOT_NUM; j++) {
            buffer += NET_RECV_BUF_SIZE;
        }
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dNetMgr_c::sInstance->setup(
        &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.mUserData,
        player->mFriends.mKeys, FRIEND_NUM, NET_MEMBER_MAX, bufs,
        NET_RECV_BUF_SIZE);
    dNetMgr_c::sInstance->setCallbacks(onNetMgrRecv, onNetMgrJoin,
                                       onNetMgrEvent, NULL, getNetPlayerInfo(),
                                       sizeof(dNetPlayerInfo::Info_c), onNetMgrLeave);
}

void setNetMyNo(int myNo) {
    getNetMyNo()->set(myNo);
    getNetInBuf()->setMyNo(myNo);
    getNetOutBuf()->setMyNo(myNo);
}

void startNetHostSession() {
    getNetAccepted()->on(0);
    getNetMembers()->on(0);
    setJoined(1);
    setSessionActive(1);
    getNetScene()->reset();
    for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
        if (i == 0) {
            dPlayerMgr::setPlayerSlot(0, dPlayerMgr::getCurrentPlayerSlot());
        } else {
            dPlayerMgr::setPlayerSlot(i, i + 3);
        }
    }
    getNetPacket()->mFrame = cCounter_c::m_gameFrame;
    getNetPacket()->clearRecords();
    onSessionStart();
    dMiiIcon::startNet(0);
}

void terminateNetSession(int arg) {
    onSessionEnd();
    if (g_dNet.mSavedSlot < 7) {
        dPlayerMgr::setPlayerSlot(0, g_dNet.mSavedSlot);
    }
    resetNetState(arg);
    dNetMgr_c::sInstance->cleanup();
    dMiiIcon::endNet();
}

void receiveNetStage() {
    dNetMgr_c::sInstance->execute();
    if (!getNetReportedError() &&
        !dReset::Manage_c::GetInstance()->isResetting()) {
        getNetPacket()->receive();
        getNetScene()->executeRecv();
    }
}

void updateNetStage() {
    if (!getNetReportedError() &&
        !dReset::Manage_c::GetInstance()->isResetting()) {
        getNetScene()->execute();
        getNetRequest()->execute();
        getNetTransfer()->execute();
        getNetSaveEnd()->execute();
        getNetDisconnect()->execute();
        getNetPacket()->sendFrame();
    }
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (!(getNetError() & NET_ERROR_TIMEOUT)) {
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (getNetPacket()->mSendWait[i] > 0xE10) {
                    setNetError(NET_ERROR_TIMEOUT);
                    break;
                }
            }
        }
        if (!(getNetError() & NET_ERROR_LOST)) {
            for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
                if (getNetWatched()->isOn(i) && !isMemberConnected(i)) {
                    setNetError(NET_ERROR_LOST);
                    break;
                }
            }
        }
        s32 reported = g_dNet.mErrorReported;
        if (reported != g_dNet.mError) {
            g_dNet.mErrorReported = reported | g_dNet.mError;
        }
        if (reported != 0) {
            dResetSave::isNetClosed();
        }
    }
}

BOOL isNetPostReady() {
    BOOL ret = TRUE;
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (!getNetDisconnect()->canDeleteScene()) {
            ret = FALSE;
        }
    }
    return ret;
}

void postNetStage() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (g_dNet.mTimePending) {
            dTime_c::applyPending();
            g_dNet.mTimePending = 0;
        }
        getNetRequest()->executeLeave();
        getNetScene()->executePostDelete();
        getNetDisconnect()->executePostDelete();
    }
}

u32 getNetHeapSize() {
    u32 size = 0;
    size += 0x9EDDC;
    size += ROUND_UP(dNetPacket_c::getMaxRecordSize() + 8, 4);
    size += 0x1FA68;
    return size;
}

void createNet(EGG::Heap *heap) {
    createNetState(heap);
}

void receiveNet() {
    receiveNetStage();
}

void updateNet() {
    updateNetStage();
}

BOOL isNetReadyToPost() {
    return isNetPostReady();
}

void postNet() {
    postNetStage();
}

void initNetMgr() {
    setupNetMgr();
}

BOOL openNetConnection() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (getNetMgrStart()->mStarted == 0) {
            getNetMgrStart()->mStarted = 1;
        }
        if (dNetMgr_c::open()) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL startHost() {
    if (!dReset::Manage_c::GetInstance()->isResetting() &&
        dNetMgr_c::sInstance->fn_800E51C8(0)) {
        if (g_dNet.mSavedSlot >= 7) {
            g_dNet.mSavedSlot = dPlayerMgr::getCurrentPlayerSlot();
        }
        setNetMyNo(0);
        startNetHostSession();
        return TRUE;
    }
    return FALSE;
}

BOOL startGuest(int arg) {
    if (!dReset::Manage_c::GetInstance()->isResetting() &&
        dNetMgr_c::sInstance->fn_800E5298(arg)) {
        u8 my = dNetMgr_c::sInstance->mInfo.mMyNo;
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (g_dNet.mSavedSlot >= 7) {
            g_dNet.mSavedSlot = dPlayerMgr::getCurrentPlayerSlot();
        }
        setNetMyNo(my);
        setJoined(1);
        for (u32 i = 0; i < NET_MEMBER_MAX; i++) {
            if (i != 0) {
                dPlayerMgr::setPlayerSlot(i, i + 3);
            }
        }
        cLib::memCpy(dPlayerMgr::Mgr_c::getNetPlayer(my), player,
                     sizeof(dPrivateData_c));
        getNetPacket()->mFrame = cCounter_c::m_gameFrame;
        dMiiIcon::startNet(my);
        return TRUE;
    }
    return FALSE;
}

BOOL disconnectNet(int arg) {
    if (closeNetConnection()) {
        endSession(arg);
        return TRUE;
    }
    return FALSE;
}

BOOL closeNetConnection() {
    if (dNetMgr_c::close()) {
        if (getNetMgrStart()->mStarted) {
            getNetMgrStart()->mStarted = 0;
        }
        return TRUE;
    }
    if (!getNetPacket()->mSuspended) {
        getNetPacket()->mSuspended = 1;
    }
    return FALSE;
}

void endSession(int arg) {
    terminateNetSession(arg);
}

BOOL isNetMgrRunning() {
    return dNetMgr_c::sInstance->mState > 1;
}

bool isSessionActive() {
    return getSessionActive();
}

BOOL isNetStopped() {
    return getNetPacket()->mSuspended;
}

int getLeaveMember() {
    return getNetDisconnect()->mMember.get();
}

bool isMember(int member) {
    return isMemberIn(member);
}

u32 getMemberNum() {
    return getNetMembers()->count();
}

int getMyNo() {
    if (isJoined()) {
        return getNetMyNo()->get();
    }
    return 0;
}

BOOL isHost() {
    return getMyNo() == 0;
}

BOOL sendNetRequest(u8 arg) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if ((u32)getNetRequest()->getStep() <= 1) {
            getNetRequest()->setStep(2);
            getNetRequest()->mMyKind.set(arg);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL requestConnect() {
    return sendNetRequest(0);
}

int getRequestResult() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (getNetRequest()->mMyKind.get() == 0 && !isMemberConnected(0)) {
            getNetRequest()->setStep(0);
            return -1;
        }
        int state = getNetRequest()->getStep();
        if (state == 1) {
            return 1;
        }
        return -(state == 0);
    }
    return 0;
}

BOOL requestContinue() {
    return sendNetRequest(2);
}

BOOL requestInterrupt() {
    return sendNetRequest(3);
}

BOOL requestDisconnect() {
    return sendNetRequest(1);
}

void startJoinTransfer() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetTransfer()->start();
        g_dNet.mJoinStarted = 1;
    }
}

void startSaveEnd(BOOL interrupt) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        dNand::getSceneChange()->setReturnExitHere(0.0f);
        dScene::requestSceneChange(dScene::SCENE_DM_SAVE, 0, 5);
        fn_800DDE7C();
        if (interrupt) {
            dSvMgr_c::requestSaveInterruptNetHst();
        } else {
            dSvMgr_c::requestSaveContinueNetHst();
        }
        getNetSaveEnd()->start(interrupt);
    }
}

void startDisconnect() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetDisconnect()->start(0);
        fn_800DDE7C();
    }
}

BOOL isJoinPlayersReady() {
    return getNetTransfer()->isAllReady();
}

u8 isItemsReceived() {
    return getNetTransfer()->mItemsRecv;
}

BOOL isNewPlayerReady() {
    int member = getNetRequest()->mGrantedMember.get();
    if (member < NET_MEMBER_MAX && dMiiIcon::isMemberDone(member)) {
        int state = getNetTransfer()->getStep();
        if (getNetMyNo()->get() == 0) {
            return state > 6;
        }
        return state > 11;
    }
    return FALSE;
}

BOOL isJoinPlayerSent() {
    return getNetTransfer()->getStep() > 2;
}

BOOL isJoinTransferDone() {
    return getNetTransfer()->getStep() == 15;
}

BOOL requestTown() {
    getNetTransfer()->mTownReqSend = 1;
    return TRUE;
}

BOOL isJoinDoneAll() {
    return getNetJoinDone()->isAllDone();
}

u8 getJoinAllDone() {
    return getNetJoinDone()->getAllDone();
}

void resetJoinDone() {
    getNetJoinDone()->init();
}

BOOL finishJoin() {
    getNetTransfer()->mFinish = 1;
    return TRUE;
}

BOOL isJoinFinished() {
    return getNetTransfer()->mFinish == 0;
}

void clearRequestMember() {
    getNetRequest()->mGrantedMember.clear();
}

void beginPacket() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetPacket()->begin();
    }
}

void addPacket(const void *data, u32 size) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetPacket()->add(data, size);
    }
}

void sendPacket(int id, int member) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetPacket()->send(id, member);
    }
}

void setRecord(int id, void *data, int arg) {
    getNetPacket()->setRecord(id, data, arg);
}

void *getRecord(int id) {
    return getNetPacket()->getRecord(id);
}

u32 getRecordSize(int id) {
    return getNetPacket()->getRecordSize(id);
}

int getSceneChangeState() {
    return getNetScene()->getState();
}

void setSceneChangeState(int state) {
    getNetScene()->setState(state);
}

void enterLeaderScene() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        getNetScene()->enterLeaderScene();
    }
}

s32 getLocalPlayerNo() {
    return g_dNet.mSavedSlot;
}

u32 getNetFrame() {
    return getNetPacket()->mFrame;
}

void writeSceneClose(int arg) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        int my = getNetMyNo()->get();
        u8 scene = getNetScene()->getScene(my);
        getNetSceneClose()->write(arg, scene);
    }
}

bool fn_800DD818() {
    return getNetScene()->getLeaveAcked();
}

void writeSceneLeave(int arg) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        int my = getNetMyNo()->get();
        u8 scene = getNetScene()->getScene(my);
        getNetSceneLeave()->write(arg, scene);
    }
}

bool fn_800DD8C4() {
    return getNetScene()->isHandoverAcked();
}

void writeSceneHandover() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        int my = getNetMyNo()->get();
        u8 scene = getNetScene()->getScene(my);
        getNetSceneHandover()->write(scene);
    }
}

u32 isSceneOwner() {
    if (getSessionActive()) {
        int my = getNetMyNo()->get();
        if (my < NET_MEMBER_MAX) {
            if (getNetScene()->getScene(my) < NET_SCENE_NONE) {
                return getNetScene()->isOwner(my);
            }
            return FALSE;
        }
        return TRUE;
    }
    return TRUE;
}

int countMembersInScene(u8 scene) {
    return getNetScene()->countMembers(scene);
}

bool isAllInScene(u8 scene) {
    return getNetScene()->isAllIn(scene);
}

u8 getMemberScene(int member) {
    return getNetScene()->getScene(member);
}

bool isSaveEndAllSaved() {
    return getNetSaveEnd()->mSaved.isAll();
}

bool isSaveEndSaved() {
    int my = getNetMyNo()->get();
    return getNetSaveEnd()->mSaved.isOn(my);
}

void resetSaveEnd() {
    getNetSaveEnd()->init();
}

BOOL isConnectSavePermitted() {
    BOOL ret = FALSE;
    if (isAllInScene(dScene::SCENE_DM_SAVE) &&
        getNetScene()->getLeader() >= NET_MEMBER_MAX) {
        ret = TRUE;
    }
    return ret;
}

BOOL canSaveContinue() {
    BOOL ret = FALSE;
    if (isAllInScene(dScene::SCENE_DM_SAVE) &&
        getNetScene()->getLeader() >= NET_MEMBER_MAX) {
        ret = TRUE;
    }
    return ret;
}

BOOL canSaveInterrupt() {
    return getSessionActive() == 0;
}

BOOL isDisconnectStarted() {
    int member = getNetDisconnect()->mMember.get();
    if (member < NET_MEMBER_MAX) {
        int v = getNetDisconnect()->getStep();
        if (member == getMyNo()) {
            if (v > 3) {
                return TRUE;
            }
        } else if (v > 7) {
            return TRUE;
        }
    }
    return FALSE;
}

void setDisconnectReady() {
    if (isHost()) {
        getNetDisconnect()->mReady.on(0);
    } else {
        beginPacket();
        sendPacket(NET_PACKET_DISCONNECT_READY, 0);
    }
}

BOOL isDisconnectReady() {
    if (isHost()) {
        if (getNetDisconnect()->mReady.isAll()) {
            return TRUE;
        }
    } else {
        int my = getNetMyNo()->get();
        if (getNetDisconnect()->mReady.isOn(my)) {
            return TRUE;
        }
    }
    return FALSE;
}

void sendDisconnectGo() {
    if (isHost()) {
        beginPacket();
        sendPacket(NET_PACKET_DISCONNECT_GO, NET_MEMBER_NONE);
    }
}

u32 getNetReportedError() {
    return !dReset::Manage_c::GetInstance()->isResetting()
               ? g_dNet.mErrorReported
               : 0;
}

s64 getMemberTime(int member) {
    return g_dNet.mTime[member];
}

void setMemberTime(int member, s64 time) {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        g_dNet.mTime[member] = time;
    }
}

void releaseRequest() {
    getNetRequest()->mLeave = 1;
}

void fn_800DDE7C() {
    dNetMgr_c::setBusy();
}

// Leaving these scenes (the gate dScene::SCENE_RM_CHKP and
// dScene::SCENE_DM_CHKP_CNNCT..SCENE_DM_CHKP_MISS) runs dNetMgr_c::clearBusy.
// clang-format off
// clang-format off
static const u8 l_sceneSync[dScene::SCENE_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0,
};
// clang-format on
// clang-format on

void fn_800DDE80() {
    if (!dReset::Manage_c::GetInstance()->isResetting()) {
        if (getMemberNum() >= 2) {
            u8 prev = dScene::getPrevScene();
            if (prev < NET_SCENE_NONE) {
                if (g_dNet.mJoinStarted) {
                    dNetMgr_c::clearBusy();
                    g_dNet.mJoinStarted = 0;
                } else if (l_sceneSync[prev]) {
                    dNetMgr_c::clearBusy();
                } else if (prev == dScene::SCENE_DM_SAVE) {
                    u8 cur = dScene::getCurrentScene();
                    if (cur < NET_SCENE_NONE && !l_sceneSync[cur]) {
                        dNetMgr_c::clearBusy();
                    }
                }
            }
        }
    }
}

BOOL isNetMgrStarted() {
    return getNetMgrStart()->mStarted;
}

BOOL isRequestIdle() {
    return getNetRequest()->mBusy.count() == 0;
}

} // namespace dNet
