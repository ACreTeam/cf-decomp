// Copying / swapping original designs (dPatternCopy). .text 80082F24..8008350C, .bss 80583A60..80584B60,
// .sdata 80749E28..80749E30, .sdata2 80750568..80750570. See include/game/game/d_pattern_copy.hpp.
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <revolution/os/OSCache.h>
#include <string.h>

namespace dPatternCopy {

// The members that answered the last swap / copy (bit per member; 0xFFFF: none pending).
static u32 l_swapAck = 0xFFFF;
static u32 l_copyAck = 0xFFFF;

static const u8 l_ackSwap = FALSE; // NET_PACKET_PATTERN_COPY_ACK: a swap was done
static const u8 l_ackCopy = TRUE;  // a copy was done

static dDesign_c l_clipboard;
static dDesign_c l_swapTmp;

// 80082F24
dDesign_c *getClipboard() {
    return &l_clipboard;
}

// 80082F30
dDesign_c *getDesign(int kind, u32 idx) {
    switch (kind) {
        case PATTERN_PLAYER_0:
        case PATTERN_PLAYER_1:
        case PATTERN_PLAYER_2:
        case PATTERN_PLAYER_3: {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(kind);
            if (player != nullptr) {
                return &player->mOrgDesigns.mDesigns[idx % PLAYER_ORG_DESIGN_COUNT];
            }
            return nullptr;
        }
        case PATTERN_TAILOR:
            return dSaveData_c::getTown()->mShops.mShopTailor.getDesign(idx % PLAYER_ORG_DESIGN_COUNT);
        case PATTERN_TOWN_FLAG:
            return &dSaveData_c::getTown()->mBuilding.mTownFlag;
        case PATTERN_CLIPBOARD:
            return getClipboard();
        case PATTERN_MY_PLAYER: {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
            if (player != nullptr) {
                return &player->mOrgDesigns.mDesigns[idx % PLAYER_ORG_DESIGN_COUNT];
            }
            return nullptr;
        }
        case PATTERN_HOME: {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            dHome_c *home = homes->getHome(homes->findCurrentPlayer());
            if (home != nullptr) {
                return &home->mDesign;
            }
            return nullptr;
        }
    }
    return nullptr;
}

// 80083044
BOOL copy(int srcKind, u32 srcIdx, int dstKind, u32 dstIdx, BOOL sync) {
    int netSrcKind = toNetKind(srcKind);
    int netDstKind = toNetKind(dstKind);
    dDesign_c *src = getDesign(netSrcKind, srcIdx);
    dDesign_c *dst = getDesign(netDstKind, dstIdx);
    if (src != nullptr && dst != nullptr) {
        memcpy(dst, src, sizeof(dDesign_c));
        DCStoreRangeNoSync(dst, sizeof(dDesign_c));
        if (sync && dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            if (!isCopyAcked()) {
                return FALSE;
            }
            l_copyAck = 1 << dNet::getMyNo();
            packet_s packet;
            packet.mSrcKind = netSrcKind;
            packet.mSrcIdx = srcIdx;
            packet.mDstKind = netDstKind;
            packet.mDstIdx = dstIdx;
            packet.mSwap = FALSE;
            dNet::beginPacket();
            dNet::addPacket(&packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_PATTERN_COPY, NET_MEMBER_NONE);
        } else {
            resetCopyAck();
        }
        return TRUE;
    }
    return FALSE;
}

// 8008316C
BOOL swap(int kindA, u32 idxA, int kindB, u32 idxB, BOOL sync) {
    int netKindA = toNetKind(kindA);
    int netKindB = toNetKind(kindB);
    dDesign_c *a = getDesign(netKindA, idxA);
    dDesign_c *b = getDesign(netKindB, idxB);
    if (a != nullptr && b != nullptr) {
        memcpy(&l_swapTmp, a, sizeof(dDesign_c));
        memcpy(a, b, sizeof(dDesign_c));
        memcpy(b, &l_swapTmp, sizeof(dDesign_c));
        DCStoreRangeNoSync(a, sizeof(dDesign_c));
        DCStoreRangeNoSync(b, sizeof(dDesign_c));
        if (sync && dNet::isSessionActive() && dNet::getMemberNum() > 1) {
            if (!isSwapAcked()) {
                return FALSE;
            }
            l_swapAck = 1 << dNet::getMyNo();
            packet_s packet;
            packet.mSrcKind = netKindA;
            packet.mSrcIdx = idxA;
            packet.mDstKind = netKindB;
            packet.mDstIdx = idxB;
            packet.mSwap = TRUE;
            dNet::beginPacket();
            dNet::addPacket(&packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_PATTERN_COPY, NET_MEMBER_NONE);
        } else {
            resetSwapAck();
        }
        return TRUE;
    }
    return FALSE;
}

// 800832C8
void resetCopyAck() {
    l_copyAck = 0xFFFF;
}

// 800832D8
void resetSwapAck() {
    l_swapAck = 0xFFFF;
}

// 800832E8
BOOL isCopyAcked() {
    if (dNet::isSessionActive()) {
        u32 count = (l_copyAck & 1) + ((l_copyAck >> 1) & 1) + ((l_copyAck >> 2) & 1) + ((l_copyAck >> 3) & 1);
        return count >= dNet::getMemberNum();
    }
    return TRUE;
}

// 80083358
BOOL isSwapAcked() {
    if (dNet::isSessionActive()) {
        u32 count = (l_swapAck & 1) + ((l_swapAck >> 1) & 1) + ((l_swapAck >> 2) & 1) + ((l_swapAck >> 3) & 1);
        return count >= dNet::getMemberNum();
    }
    return TRUE;
}

// 800833C8
int toNetKind(int kind) {
    if (kind == PATTERN_MY_PLAYER) {
        return dNet::getMyNo();
    }
    return kind;
}

// 800833D8
void recvPacket(const void *data, int member) {
    packet_s packet = *(const packet_s *)data;
    int srcKind = packet.mSrcKind;
    u32 srcIdx = packet.mSrcIdx;
    int dstKind = packet.mDstKind;
    u32 dstIdx = packet.mDstIdx;
    if (packet.mSwap) {
        swap(srcKind, srcIdx, dstKind, dstIdx, FALSE);
        dNet::beginPacket();
        dNet::addPacket(&l_ackSwap, sizeof(l_ackSwap));
        dNet::sendPacket(NET_PACKET_PATTERN_COPY_ACK, member);
    } else {
        copy(srcKind, srcIdx, dstKind, dstIdx, FALSE);
        dNet::beginPacket();
        dNet::addPacket(&l_ackCopy, sizeof(l_ackCopy));
        dNet::sendPacket(NET_PACKET_PATTERN_COPY_ACK, member);
    }
}

// 80083468
void recvAck(BOOL copied, int member) {
    if (copied) {
        l_copyAck |= 1 << member;
        if (isCopyAcked()) {
            resetCopyAck();
        }
    } else {
        l_swapAck |= 1 << member;
        if (isSwapAcked()) {
            resetSwapAck();
        }
    }
}

} // namespace dPatternCopy
