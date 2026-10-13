#pragma once

// Copying / swapping original designs between the places that hold them (the players' design lists, the
// Able Sisters' display, the town flag, a clipboard, the house), mirrored to the other members of an online
// session (NET_PACKET_PATTERN_COPY / _ACK). Source: src/dol/game/d_pattern_copy.cpp (.text 80082F24..8008350C).
// The namespace is from the file name; the function and kind names are inferred (no RTTI).

#include <types.h>

class dDesign_c;

namespace dPatternCopy {

// Where a design is (getDesign's kind). PATTERN_PLAYER_0 + n is net member n's player.
enum Kind_e {
    PATTERN_PLAYER_0 = 0, // 0..3: dPlayerMgr::Mgr_c::getNetPlayer(kind)'s designs [idx % 8]
    PATTERN_PLAYER_1 = 1,
    PATTERN_PLAYER_2 = 2,
    PATTERN_PLAYER_3 = 3,
    PATTERN_TAILOR = 4,    // the Able Sisters' display [idx % 8]
    PATTERN_TOWN_FLAG = 5, // dSaveBuilding_c::mTownFlag
    PATTERN_CLIPBOARD = 6, // getClipboard()
    PATTERN_MY_PLAYER = 7, // the current player's designs [idx % 8]; sent as this console's member number
    PATTERN_HOME = 8,      // the current player's house
};

// The packet of NET_PACKET_PATTERN_COPY (4 bytes).
struct packet_s {
    u32 mSrcKind : 4;
    u32 mSrcIdx : 10;
    u32 mDstKind : 4;
    u32 mDstIdx : 10;
    u32 mSwap : 1;
    u32 : 3;
};

dDesign_c *getClipboard();                       // 80082F24
dDesign_c *getDesign(int kind, u32 idx);         // 80082F30: NULL if there is none
// Copies src to dst (or swaps them); with sync in an online session, also on the other consoles
// (FALSE while the last one is still unanswered).
BOOL copy(int srcKind, u32 srcIdx, int dstKind, u32 dstIdx, BOOL sync); // 80083044
BOOL swap(int kindA, u32 idxA, int kindB, u32 idxB, BOOL sync);         // 8008316C
void resetCopyAck();                             // 800832C8
void resetSwapAck();                             // 800832D8
BOOL isCopyAcked();                              // 800832E8: every member answered the last copy
BOOL isSwapAcked();                              // 80083358
int toNetKind(int kind);                         // 800833C8: PATTERN_MY_PLAYER -> this console's member
void recvPacket(const void *data, int member);   // 800833D8: NET_PACKET_PATTERN_COPY (d_net)
void recvAck(BOOL copied, int member);           // 80083468: NET_PACKET_PATTERN_COPY_ACK (d_net)

} // namespace dPatternCopy
