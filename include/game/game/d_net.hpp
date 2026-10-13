#pragma once

// d_net: the online multiplayer session (members 0..3, host, packets, shared
// records). TU src/dol/game/d_net.cpp (+ src/dol/game/d_net_sync.inc), .text
// 800D3590..800E3E20, -sym on. The free functions are in namespace dNet
// (callers use dNet::name); the packet ids are dNetPacket_e (NET_PACKET_*), the
// direct packet kinds dNetDirect_e (NET_DIRECT_*).
//
// The whole state is one 0x660 global (805B2160, dNet_c below) made of small
// sub-objects. Each sub-object class has its methods together in .text, in the
// order of the members (the classes are defined in member order), and the code
// reaches a sub-object through an out-of-line accessor (getNetMyNo.. returning
// &g_dNet.member) even inside the class's own methods. The sub-objects keep
// their size / alignment (the static_asserts below).

#include <string.h>
#include <types.h>
#include <game/game/d_net_player_info.hpp>

namespace EGG {
class Heap;
}

// Member numbers are 0..3; 4 (NET_MEMBER_NONE) means "no member" (also the
// "every member" packet destination). Scene numbers use 0x44 (NET_SCENE_NONE)
// for "none".
#define NET_MEMBER_MAX 4
#define NET_MEMBER_NONE 4
// the host's member number (isHost)
#define NET_MEMBER_HOST 0
#define NET_SCENE_NONE 0x44
// Block destinations besides a member / NET_MEMBER_NONE (the routing switch in
// dNetPacket_c::sendFrame).
// the owner of the sender's scene
#define NET_DEST_SCENE_OWNER 5
// the other members in the sender's scene (sent by its owner)
#define NET_DEST_SCENE_OTHERS 6

// dNet_c::mError bits (setNetError); 0x4 is never set in CF.
// the packet stream is full (dNetPacket_c::begin / add)
#define NET_ERROR_OVERFLOW 0x1
// no data from a member for 0xE10 frames
#define NET_ERROR_TIMEOUT 0x2
// a member's packet sequence number skipped (updateRecvCount)
#define NET_ERROR_SEQUENCE 0x8
// a member of mWatched is no longer connected
#define NET_ERROR_LOST 0x10

// The size of one receive buffer given to d_net_mgr (setupNetMgr).
#define NET_RECV_BUF_SIZE 0x16802

// Packet ids of the block stream (dNetBlockHeader::mId): sendPacket /
// dNetPacket_c::send(id, dest). The receiver dispatches lbl_80473E44[id]
// (callBlockHandlers). Destination: a member, NET_MEMBER_NONE (every member),
// NET_DEST_SCENE_OWNER or NET_DEST_SCENE_OTHERS.
enum dNetPacket_e {
    NET_PACKET_NONE = 0x00, // no-op handler; never sent
    // member -> host after the join transfer: dNetJoinDone_c::setDone(member,
    // 1) (same as direct kind 12)
    NET_PACKET_JOIN_DONE = 0x01,
    // host -> everyone after the join transfer: dNetJoinDone_c::setAllDone(1)
    // (same as direct kind 11)
    NET_PACKET_JOIN_DONE_HOST = 0x02,
    // member -> host: request (1 byte kind: 0 connect / 1 disconnect / 2
    // continue / 3 interrupt); dNetRequest_c mRequests.on + setKind
    NET_PACKET_JOIN_REQUEST = 0x03,
    // member -> host: answered the save prompt (1 byte ok);
    // dNetRequest_c::setMemberStep(member, ok)
    NET_PACKET_JOIN_SAVE_REPLY = 0x04,
    // host -> member: show the save prompt (1 byte: requester << 2 | kind);
    // setSaveStep(1)
    NET_PACKET_JOIN_SAVE_PROMPT = 0x05,
    // host -> member: request refused, close the prompt (setSaveStep(5))
    NET_PACKET_JOIN_SAVE_CANCEL = 0x06,
    // member -> host: cancel handled (setMemberStep(member, 0))
    NET_PACKET_JOIN_CANCEL_DONE = 0x07,
    // host -> requesting member (dNetRequest_c::sendResult): accepted / refused
    // (1 byte); dNetRequest_c::setStep
    NET_PACKET_JOIN_RESULT = 0x08,
    // member -> host (dNetRequest_c::executeLeave): no longer busy,
    // mBusy.off(member)
    NET_PACKET_JOIN_RELEASE = 0x09,
    // host -> other members: request accepted, go to the save scene
    // (setSaveStep(6))
    NET_PACKET_JOIN_COMMIT = 0x0A,
    // member -> host: dNetSceneReq_c, queue a scene change (addOrder) or leave
    // the queue (removeOrder)
    NET_PACKET_SCENE_REQUEST = 0x0B,
    // host -> everyone: dNetMemberScene_c of the member allowed to change scene
    // (setLeader); others answer 0x0E to it
    NET_PACKET_SCENE_LEADER = 0x0C,
    // host -> everyone: the queue is empty (clearLeader)
    NET_PACKET_SCENE_NO_LEADER = 0x0D,
    // -> the leader: sender saw the leader (mLeaderAcks.on(member))
    NET_PACKET_SCENE_LEADER_ACK = 0x0E,
    // dNetSceneMsg_c: a member's scene / owner flag; answered with 0x10 to
    // msg.mState when < 4
    NET_PACKET_SCENE_STATE = 0x0F,
    NET_PACKET_SCENE_STATE_ACK = 0x10, // ack of 0x0F (mMsgAcks.on(member))
    // last member leaving a scene -> host: channel-538 data, read on the host
    // (dNetSceneClose_c)
    NET_PACKET_SCENE_SAVE_HOST = 0x11,
    // first member entering a scene -> host: host writes dNetSceneOpen_c and
    // answers 0x13
    NET_PACKET_SCENE_LOAD_REQUEST = 0x12,
    // host -> member: channel-540 scene data (dNetSceneOpen_c::setData +
    // setRecv)
    NET_PACKET_SCENE_LOAD = 0x13,
    // non-owner leaving -> scene owner (NET_DEST_SCENE_OWNER): owner sets
    // dNetScene_c::mLeaving and answers 0x15
    NET_PACKET_SCENE_EXIT_REQUEST = 0x14,
    // owner -> leaver: dNetScene_c::setLeaveAcked(1)
    NET_PACKET_SCENE_EXIT_ACK = 0x15,
    // leaver -> scene owner: channel-54C data (dNetSceneLeave_c read)
    NET_PACKET_SCENE_EXIT_DATA = 0x16,
    // entering member -> a member already in the scene: request its data
    // (dNetSceneEnter_c::mRequester = member)
    NET_PACKET_SCENE_ENTER_REQUEST = 0x17,
    // scene member -> entrant: channel-554 data (dNetSceneEnter_c::setData +
    // setRecv)
    NET_PACKET_SCENE_ENTER_DATA = 0x18,
    // owner leaving -> the others in its scene (NET_DEST_SCENE_OTHERS):
    // getNetHoldBuf()->mOldOwner = member, mHoldState = 0; answered with 0x1A
    NET_PACKET_SCENE_HANDOVER_REQUEST = 0x19,
    // ack of 0x19 (mHandoverAcks.on(member))
    NET_PACKET_SCENE_HANDOVER_ACK = 0x1A,
    // old owner -> new owner (dNetSceneHandover_c::mNewOwner): channel-564 data
    NET_PACKET_SCENE_HANDOVER_DATA = 0x1B,
    // visitor -> host after the continue / interrupt save (dSvMgr_c
    // notifyHostSaveComplete): dNetSaveEnd_c mSaved.on(member)
    NET_PACKET_SAVE_DONE = 0x1C,
    // host -> everyone once every console saved (dSvMgr_c
    // waitAllMachinesSaved): dNetSaveEnd_c mSaved.on(self)
    NET_PACKET_SAVE_DONE_ALL = 0x1D,
    // member -> host: its 2-byte dNetSyncBuf_c buffer (getBuf4); also an ack in
    // dNetSaveEnd_c step 1
    NET_PACKET_SYNC_BUF_MEMBER = 0x1E,
    // host -> member: member mask + the sync buffers (dNetSyncBuf_c::send /
    // unpack)
    NET_PACKET_SYNC_BUF = 0x1F,
    // disconnecting member -> everyone (dNetDisconnect_c step 1): receiver sets
    // dNetDisconnect_c mAcks.on(self)
    NET_PACKET_DISCONNECT_START = 0x20,
    // member -> disconnecting member (dNetDisconnect_c step 5):
    // mAcks.on(member)
    NET_PACKET_DISCONNECT_ACK = 0x21,
    // visitor -> host (setDisconnectReady, used by the gatekeeper REL):
    // dNetDisconnect_c mReady.on(member)
    NET_PACKET_DISCONNECT_READY = 0x22,
    // host -> everyone (sendDisconnectGo, gatekeeper REL): dNetDisconnect_c
    // mReady.on(self)
    NET_PACKET_DISCONNECT_GO = 0x23,
    // d_player_mngr fn_801B9490 (1 byte) -> dPlayerMgr::receiveAppearance:
    // tanning value, save-request flag clear, or Mii-mask on/off.
    NET_PACKET_PLAYER_MNGR_24 = 0x24,
    // d_player_mngr fn_801B93B8 (u8 + u16) -> dPlayerMgr::receiveEquipment:
    // equipment part and item ID, applied to the remote player save data.
    NET_PACKET_PLAYER_MNGR_25 = 0x25,
    // d_player_mngr fn_801B9424 (u8 + u16) -> dPlayerMgr::queueRemoteEquipment:
    // equipment part and item ID, queued for the remote actor.
    NET_PACKET_PLAYER_MNGR_26 = 0x26,
    // player actor (d_a_player REL fn_80_2C64C) -> everyone: value, state,
    // 9-byte param; drives the remote player
    NET_PACKET_PLAYER_STATE = 0x27,
    // = NET_PACKET_FG_MNG_REQ: dFgMngReq_c to the host (fgobj REL fn_111_8D8C);
    // host validates and answers
    NET_PACKET_FG_MNG_REQ = 0x28,
    // = NET_PACKET_FG_MNG_CMD: dFgMngCmd_c (dFgMngReq_c::send, fgMngCmd_apply);
    // clients apply it
    NET_PACKET_FG_MNG_CMD = 0x29,
    // = NET_PACKET_FG_MNG_TASK_DONE: dFgMngTaskMsg_c (dFgMngTask_c::finish);
    // clears the lock's pending bit
    NET_PACKET_FG_MNG_TASK_DONE = 0x2A,
    // = NET_PACKET_FG_MNG_UNIT_FG: fgMngProc_setUnitFgSync ->
    // fgMngProc_recvUnitFg
    NET_PACKET_FG_MNG_UNIT_FG = 0x2B,
    // d_demo_sync: 5-byte demo sync request / answer (dDemoSync::receive)
    NET_PACKET_DEMO_SYNC = 0x2C,
    // dPostOffice::sendSyncState -> dPostOffice::onSyncPacket
    NET_PACKET_POST_SYNC = 0x2D,
    // dPostOffice::sendMail (dMail_c) -> dPostOffice::onMailPacket
    NET_PACKET_MAIL = 0x2E,
    // d_npc_notice fn_800ECD08: a dNotice_c (0x19A bytes) added to the notice
    // board
    NET_PACKET_NOTICE_ADD = 0x2F,
    // d_npc_notice fn_800ECD54: a notice index removed from the board
    NET_PACKET_NOTICE_REMOVE = 0x30,
    // d_str: claim / grant of a structure slot (1 byte, bit 6 = granted) ->
    // fn_801689A0; hedged
    NET_PACKET_STR_LOCK = 0x31,
    // d_str fn_80168DC0: release every slot the member holds -> fn_80168A4C;
    // hedged
    NET_PACKET_STR_UNLOCK = 0x32,
    // dFTR::requestHold: visitor -> host, reserve furniture at (x, z, layer);
    // host answers 0x34 / 0x35
    NET_PACKET_FTR_HOLD_REQUEST = 0x33,
    // dFTR::recvHoldRequest (host answer) -> dFTR::recvHoldAnswer(TRUE)
    NET_PACKET_FTR_HOLD_OK = 0x34,
    // dFTR::recvHoldRequest (host answer) -> dFTR::recvHoldAnswer(FALSE)
    NET_PACKET_FTR_HOLD_NG = 0x35,
    // dFTR::releaseHold: visitor -> host, drop the reservation (dFTR::recvHoldRelease)
    NET_PACKET_FTR_HOLD_RELEASE = 0x36,
    // dFTR::sendSwitch: a furniture's state at (scene, x, z); receiver relays
    // when bit 2 is set; hedged
    NET_PACKET_FTR_ACTION = 0x37,
    // dFTR::sendRoomFlag: 2-byte furniture update (dFTR::setRoomFlag); relayed when
    // bit 7 is set; hedged
    NET_PACKET_FTR_ACTION2 = 0x38,
    // dFTR::putFtr: a furniture item put into a room's field info
    NET_PACKET_FTR_PUT = 0x39,
    // dFTR::removeFtr: furniture removed (its units set to 0xFFF1)
    NET_PACKET_FTR_REMOVE = 0x3A,
    // dFTR::setUnitItem: one room unit's item
    // (dFdInfoBase_setItemFromWorldUnit)
    NET_PACKET_FTR_SET_UNIT = 0x3B,
    // dFTR::sendOwnerSwitch -> scene owner (NET_DEST_SCENE_OWNER): dFTR::setOwnerSwitch runs
    // on the owner; hedged
    NET_PACKET_FTR_OWNER = 0x3C,
    // dBgUtil::sendRoomItem -> dBgUtil::recvRoomItem
    NET_PACKET_ROOM_ITEM = 0x3D,
    // d_pattern_copy fn_80083044: copy a 0x880-byte design between slots;
    // receiver answers 0x3F
    NET_PACKET_PATTERN_COPY = 0x3E,
    // d_pattern_copy dPatternCopy::recvPacket: 1-byte answer to 0x3E (dPatternCopy::recvAck)
    NET_PACKET_PATTERN_COPY_ACK = 0x3F,
    // insect field REL (fn_116_53C4, fn_116_7DC4): dNetPingMsg_c; the REL hooks
    // lbl_8074E510, default handler replies type 3
    NET_PACKET_INSECT_FIELD = 0x40,
    // dPlaySyncMsg_c (d_play_util sendSyncRecClaim / sendSyncRecRelease /
    // recvSyncAck): sync record claim / release / ack
    NET_PACKET_PLAY_SYNC = 0x41,
    // dShopSale_c (dSaveShop_c / dSaveShopTailor_c::sellAndNotify) ->
    // dShopLayout_applySale
    NET_PACKET_SHOP_SALE = 0x42,
    // uomasa REL (Chip): fishing tourney record dSaveTown_c::mFishing
    NET_PACKET_FISHING_RECORD = 0x43,
    NET_PACKET_BUG_OFF = 0x44, // bug REL (Nat): dSaveTown_c::mBugOff
    // 0x45..0x4A: K.K.'s concert (dNpcInfo::totakeke_c)
    NET_PACKET_NPC_EVENT_START = 0x45,
    NET_PACKET_NPC_EVENT_DATA = 0x46,
    NET_PACKET_NPC_EVENT_JOIN = 0x47,
    NET_PACKET_NPC_EVENT_PLAY = 0x48,
    NET_PACKET_NPC_EVENT_END = 0x49,
    NET_PACKET_NPC_EVENT_LEAVE = 0x4A,
    // dItemPairRing_c::push: dSaveTown_c::mNpcSpotA.mFlag = 1
    NET_PACKET_NPC_SPOT_A_FLAG = 0x4B,
    // dNpcInfo::jingle_c::setSpot: dSaveTown_c::mNpcSpotA (dSvNpcPos::KindPos_c::set)
    NET_PACKET_NPC_SPOT_A = 0x4C,
    // pumpking REL (Jack): dSaveTown_c::mNpcSpotB.mFlag = 1
    NET_PACKET_NPC_SPOT_B_FLAG = 0x4D,
    // dNpcInfo::pumpking_c::setSpot: dSaveTown_c::mNpcSpotB (dSvNpcPos::Pos_c::set)
    NET_PACKET_NPC_SPOT_B = 0x4E,
    // franklin REL: dSaveTown_c::mHarvestSpot.mFlags |= SIDE_POS_FLAG_EVENT
    NET_PACKET_HARVEST_SPOT_FLAG = 0x4F,
    // dNpcInfo::franklin_c::setSpot: the Harvest Festival spot mHarvestSpot (dSvNpcPos::SidePos_c::set)
    NET_PACKET_HARVEST_SPOT = 0x50,
    // d_sv_daily_rand dSvDailyRand::Rand_c::change: dSaveTown_c::mDailyRand
    NET_PACKET_DAILY_RAND = 0x51,
    // dObjChat::postText: chat text (1 byte + u16 chars) -> receiveText
    NET_PACKET_CHAT = 0x52,
    // dObjChat::postPreset: preset group and index (2 bytes) -> receivePreset
    NET_PACKET_CHAT_PRESET = 0x53,
    // d_clock dClock::sendCmd: a 1-byte clock command (dClock::Cmd_e) -> dClock::receive
    NET_PACKET_CLOCK = 0x54,
    // d_npc fn_800EFFA0: a villager's held item (setHeldItem)
    NET_PACKET_ANIMAL_HELD_ITEM = 0x55,
    // d_npc fn_800F0054: mHeldItemChangeMinute
    NET_PACKET_ANIMAL_HELD_ITEM_TIME = 0x56,
    NET_PACKET_ANIMAL_EVENT_FLAGS = 0x57, // d_npc fn_800F0108: mEvent.mFlags
    // cockroach manager REL: clear a home room flag (home _15BA = 30 when none
    // are left)
    NET_PACKET_COCKROACH_REMOVE = 0x58,
    // d_npc fn_800F0208: dAnimalMemory_c::init for the sender + impression
    NET_PACKET_ANIMAL_MEMORY_INIT = 0x59,
    // d_npc fn_800F02A8: dAnimalMemory_c::set for the sender + impression
    NET_PACKET_ANIMAL_MEMORY_SET = 0x5A,
    // d_npc fn_800F039C: memory setFriendship
    NET_PACKET_ANIMAL_FRIENDSHIP = 0x5B,
    // d_npc fn_800F04E8: memory onEventFlag
    NET_PACKET_ANIMAL_MEMORY_FLAG_ON = 0x5C,
    // memory offEventFlag (no sender found)
    NET_PACKET_ANIMAL_MEMORY_FLAG_OFF = 0x5D,
    // d_npc fn_800F05C8: memory mFlags.mEventTalked = 1
    NET_PACKET_ANIMAL_EVENT_TALKED = 0x5E,
    // jonny REL (Gulliver): dSaveTown_c::setFlag(4)
    NET_PACKET_GULLIVER_FLAG = 0x5F,
    // jonny REL (Gulliver): the sender's PID -> dSaveTown_c::_072CEE
    NET_PACKET_GULLIVER_PLAYER = 0x60,
    // d_obj_voice dObjVoice_c::send: 1 byte -> dObjVoice_c::receive
    NET_PACKET_VOICE = 0x61,
    // dPostOffice::sendRecycleBin -> dPostOffice::onRecycleBin
    NET_PACKET_RECYCLE_BIN = 0x62,
    // dPostOffice::onRecycleBin -> dPostOffice::onRecycleAck
    NET_PACKET_RECYCLE_BIN_ACK = 0x63,
    // d_npc fn_800F0650: mHideAndSeek.clearInfo
    NET_PACKET_HIDE_AND_SEEK_CLEAR = 0x64,
    // d_npc fn_800F06A8: mHideAndSeek.mBase.mState
    NET_PACKET_HIDE_AND_SEEK_STATE = 0x65,
    // d_npc fn_800F0708: mHideAndSeek.setFlag
    NET_PACKET_HIDE_AND_SEEK_FLAG = 0x66,
    // dPostOffice::sendMelody (town tune) -> dPostOffice::onMelody
    NET_PACKET_MELODY = 0x67,
    // dPostOffice::onMelody -> dPostOffice::onMelodyAck
    NET_PACKET_MELODY_ACK = 0x68,
    // d_npc fn_800F0864: the furniture at (x, z) in the current room
    // (dFTR::sendEvent); hedged
    NET_PACKET_ANIMAL_FTR_TOUCH = 0x69,
    // d_npc fn_800F09B4: a villager's furniture slot set / cleared
    NET_PACKET_ANIMAL_FTR = 0x6A,
    NET_PACKET_ANIMAL_PLACE = 0x6B, // d_npc fn_800F0A84: mPlace
    // d_npc fn_800F0B64: mSpot x, z, type (group 1)
    NET_PACKET_ANIMAL_SPOT = 0x6C,
    // d_npc fn_800F0C5C: mSpot group + time
    NET_PACKET_ANIMAL_SPOT_TIME = 0x6D,
    NET_PACKET_ANIMAL_SPOT_CLEAR = 0x6E, // d_npc fn_800F0D0C: mSpot.clear
    // d_npc fn_800F0DD4: setFtr(slot, item)
    NET_PACKET_ANIMAL_FTR_SET = 0x6F,
    NET_PACKET_ANIMAL_NEW_ITEM_ADD = 0x70,   // d_npc fn_800F0E9C: addNewItem
    NET_PACKET_ANIMAL_NEW_ITEM_CLEAR = 0x71, // d_npc fn_800F0F3C: clearNewItems
    // d_npc fn_800F0FE4: removeNewItem
    NET_PACKET_ANIMAL_NEW_ITEM_REMOVE = 0x72,
    NET_PACKET_ANIMAL_WALL = 0x73,   // d_npc fn_800F109C: setWall
    NET_PACKET_ANIMAL_CARPET = 0x74, // d_npc fn_800F1154: setCarpet
    NET_PACKET_ANIMAL_MUSIC = 0x75,  // d_npc fn_800F120C: setMusic
    // d_npc fn_800F12D8: quest (kind 2 / 4 / 6) state raised
    NET_PACKET_ANIMAL_QUEST_STATE = 0x76,
    // d_npc fn_800F1388: clearQuestStarted
    NET_PACKET_ANIMAL_QUEST_CLEAR = 0x77,
    // d_npc fn_800F1418: clearPlaceChangeTime
    NET_PACKET_ANIMAL_PLACE_TIME_CLEAR = 0x78,
    NET_PACKET_ANIMAL_PRESENT = 0x79, // d_npc fn_800F1514: memory setPresent
    // d_npc fn_800F1638: dNpcPermit_c setState (+ flags, fn_800F1738 relay)
    NET_PACKET_PERMIT_STATE = 0x7A,
    // d_npc fn_800F16B0: dNpcPermit_c setState only
    NET_PACKET_PERMIT_SET_STATE = 0x7B,
    // d_npc fn_800F1738 (to one member): dNpcPermit_c setFlag(member)
    NET_PACKET_PERMIT_FLAG = 0x7C,
    // d_npc fn_800F17D0: receiver answers with fn_800F1838 (0x7E)
    NET_PACKET_PERMIT_QUERY = 0x7D,
    // d_npc fn_800F1838 (to one member): setFlag; when complete fn_800F18D0
    // (0x7F) + clear
    NET_PACKET_PERMIT_ANSWER = 0x7E,
    // d_npc fn_800F18D0: clear when mState matches
    NET_PACKET_PERMIT_CLEAR_STATE = 0x7F,
    // d_npc fn_800F1948: set2(state) when there is no state yet
    NET_PACKET_PERMIT_SET2 = 0x80,
    // d_npc fn_800F19C0: dNpcPermit_c clear
    NET_PACKET_PERMIT_CLEAR = 0x81,
    NET_PACKET_ANIMAL_CLOTH = 0x82, // d_npc fn_800F1A68: setCloth
    NET_PACKET_NUM = 0x83,          // receive handlers 0..0x82
};

// Kinds of a direct packet (dNetPacketHeader::mDirect set, mKind): one message
// handled at once by lbl_80473E10[kind] (callDirectHandler), with the sequence
// check mode lbl_80473E00[kind] (getDirectSeqMode). sendDirect / sendDirectTo /
// writePacketHeader take it. NET_DIRECT_NUM (13) doubles as "no message": the
// header of a block stream packet, or an empty direct packet that only returns
// credits.
enum dNetDirect_e {
    // recvDirectHello: sent each frame (noCount) while mHelloSend: receiver
    // sets
    // mHelloRecv (+ mHelloReply); sequence not checked (lbl_80473E00 = -1)
    NET_DIRECT_HELLO = 0,
    // recvDirectHelloAck: sent while mHelloReply: receiver clears mHelloSend
    // (stops
    // its HELLOs); sequence stored unchecked (0)
    NET_DIRECT_HELLO_ACK = 1,
    // recvDirectJoinRequest: not-yet-member -> host: connect request
    // (setKind(member, 0),
    // mRequests.on)
    NET_DIRECT_JOIN_REQUEST = 2,
    // recvDirectJoinResult: host -> not-yet-member (dNetRequest_c::sendResult):
    // empty =
    // refused, 1 byte member mask = accepted
    NET_DIRECT_JOIN_RESULT = 3,
    // recvDirectXferStart: host -> joiner (dNetTransfer_c::sendStart):
    // dNetTransfer_c::setReady
    NET_DIRECT_XFER_START = 4,
    // recvDirectXferPlayer: joiner -> everyone (dNetTransfer_c::sendPlayer()):
    // player
    // data, item mask, time, DL items, animal, Mii, scene
    NET_DIRECT_XFER_PLAYER = 5,
    // recvDirectXferItemMask: dNetTransfer_c::sendItemMask: mExclude (0x20
    // bytes)
    NET_DIRECT_XFER_ITEM_MASK = 6,
    // recvDirectXferTown: host -> joiner (dNetTransfer_c::sendTown): a town
    // chunk; the
    // first carries time, sync buffer, shared records, scenes
    NET_DIRECT_XFER_TOWN = 7,
    // recvDirectXferPlayerReply: member -> joiner
    // (dNetTransfer_c::sendPlayer(member)):
    // player data, time, Mii; mReady[member] = 1
    NET_DIRECT_XFER_PLAYER_REPLY = 8,
    // recvDirectXferItems: dNetTransfer_c::sendItems: up to 11 DL item blocks
    // (0x2000),
    // flag byte = last part (mItemsRecv)
    NET_DIRECT_XFER_ITEMS = 9,
    // recvDirectXferTownRequest: joiner -> host (dNetTransfer_c::sendTownReq,
    // after
    // requestTown): mTownReqRecv = 1, host goes on to send the town
    NET_DIRECT_XFER_TOWN_REQUEST = 10,
    // recvDirectJoinDoneHost: dNetJoinDone_c::setAllDone(1); no sender found
    // (NET_PACKET_JOIN_DONE_HOST does the same through the stream)
    NET_DIRECT_JOIN_DONE_HOST = 11,
    // recvDirectJoinDone: joiner -> host (dNetTransfer_c::sendFinish):
    // dNetJoinDone_c::setDone(member, 1) (same as NET_PACKET_JOIN_DONE)
    NET_DIRECT_JOIN_DONE = 12,
    // also "none": stream packets and empty direct packets
    NET_DIRECT_NUM = 13,
};

// dNetScene_c::mState: the scene change of this console (getSceneChangeState /
// setSceneChangeState; d_scene requests it, d_scene_base runs the scene delete
// / create steps). Leaving the current scene takes one of three routes: CLOSE
// (the owner, alone: the close channel goes to the host), LEAVE (not the owner:
// the leave channel goes to the owner) or HANDOVER (the owner with others: the
// handover channel goes to the new owner). Entering takes the enter channel
// from the scene's owner, or the open channel from the host.
enum dNetSceneState_e {
    NET_SCENE_STATE_NONE = 0,
    // a scene change was requested (d_scene); execute queues it
    NET_SCENE_STATE_REQUEST = 1,
    // waits to be the leader with all mLeaderAcks, then CLOSE / LEAVE /
    // HANDOVER
    NET_SCENE_STATE_WAIT_TURN = 2,
    // owner and alone in the scene: d_scene_base writes the close channel
    NET_SCENE_STATE_CLOSE = 3,
    // post-delete: the close channel to the host (NET_PACKET_SCENE_SAVE_HOST)
    NET_SCENE_STATE_CLOSE_SEND = 4,
    // not the owner: waits mLeaveAcked, d_scene_base writes the leave channel
    NET_SCENE_STATE_LEAVE = 5,
    NET_SCENE_STATE_LEAVE_WRITTEN = 6, // post-delete
    // after-send: the leave channel to the owner (NET_PACKET_SCENE_EXIT_DATA)
    NET_SCENE_STATE_LEAVE_SEND = 7,
    // owner with others: waits mHandoverAcks, d_scene_base writes the handover
    // channel
    NET_SCENE_STATE_HANDOVER = 8,
    NET_SCENE_STATE_HANDOVER_WRITTEN = 9, // post-delete
    // after-send: the handover channel to the new owner
    // (NET_PACKET_SCENE_HANDOVER_DATA)
    NET_SCENE_STATE_HANDOVER_SEND = 10,
    // waits mMsgAcks, then asks the scene's owner (WAIT_ENTER) or the host
    // (WAIT_OPEN)
    NET_SCENE_STATE_ENTER = 11,
    // waits for the open channel (dNetSceneOpen_c), reads it
    NET_SCENE_STATE_WAIT_OPEN = 12,
    // waits for the enter channel (dNetSceneEnter_c), reads it
    NET_SCENE_STATE_WAIT_ENTER = 13,
    NET_SCENE_STATE_CREATE = 14, // d_scene_base: enterLeaderScene
    // d_scene_base creates the scene (-> CREATED once createBase succeeds)
    NET_SCENE_STATE_CREATING = 15,
    // waits mMsgAcks, then leaves the order queue
    NET_SCENE_STATE_CREATED = 16,
    // waits until this console is no longer the leader, then NONE
    NET_SCENE_STATE_END = 17,
};

// --------------------------------------------------------------------------------------------------
// Small value types (methods 800D4ECC..800D55CC, agent A)

// The 4-byte header of a data channel entry (an index and a size), five classes
// of the same shape (800D4AE4..800D4EC8), used by dNetSceneClose_c,
// dNetSceneOpen_c, dNetSceneLeave_c, dNetSceneEnter_c and dNetSceneHandover_c
// in that order.
class dNetSceneCloseEntry_c {
  public:
    static void check(u32 idx, u32 size);         // 800D4AE4 (nothing)
    void set(const dNetSceneCloseEntry_c &other); // 800D4AE8
    void set(u32 idx, u32 size);                  // 800D4B34
    u32 getIdx() const;                           // 800D4B90
    u32 getSize() const;                          // 800D4B9C
    dNetSceneCloseEntry_c *getData();             // 800D4BA8

    /* 0x0 */ u32 mIdx : 8;
    /* 0x0 */ u32 mSize : 24;
}; // size 0x4

class dNetSceneOpenEntry_c {
  public:
    static void check(u32 idx, u32 size);        // 800D4BAC (nothing)
    void set(const dNetSceneOpenEntry_c &other); // 800D4BB0
    void set(u32 idx, u32 size);                 // 800D4BFC
    u32 getIdx() const;                          // 800D4C58
    u32 getSize() const;                         // 800D4C64
    dNetSceneOpenEntry_c *getData();             // 800D4C70

    /* 0x0 */ u32 mIdx : 8;
    /* 0x0 */ u32 mSize : 24;
}; // size 0x4

class dNetSceneLeaveEntry_c {
  public:
    static void check(u32 idx, u32 size);         // 800D4C74 (nothing)
    void set(const dNetSceneLeaveEntry_c &other); // 800D4C78
    void set(u32 idx, u32 size);                  // 800D4CC4
    u32 getIdx() const;                           // 800D4D20
    u32 getSize() const;                          // 800D4D2C
    dNetSceneLeaveEntry_c *getData();             // 800D4D38

    /* 0x0 */ u32 mIdx : 8;
    /* 0x0 */ u32 mSize : 24;
}; // size 0x4

class dNetSceneEnterEntry_c {
  public:
    static void check(u32 idx, u32 size);         // 800D4D3C (nothing)
    void set(const dNetSceneEnterEntry_c &other); // 800D4D40
    void set(u32 idx, u32 size);                  // 800D4D8C
    u32 getIdx() const;                           // 800D4DE8
    u32 getSize() const;                          // 800D4DF4
    dNetSceneEnterEntry_c *getData();             // 800D4E00

    /* 0x0 */ u32 mIdx : 8;
    /* 0x0 */ u32 mSize : 24;
}; // size 0x4

class dNetSceneHandoverEntry_c {
  public:
    static void check(u32 idx, u32 size);            // 800D4E04 (nothing)
    void set(const dNetSceneHandoverEntry_c &other); // 800D4E08
    void set(u32 idx, u32 size);                     // 800D4E54
    u32 getIdx() const;                              // 800D4EB0
    u32 getSize() const;                             // 800D4EBC
    dNetSceneHandoverEntry_c *getData();             // 800D4EC8

    /* 0x0 */ u32 mIdx : 8;
    /* 0x0 */ u32 mSize : 24;
}; // size 0x4

// A set of members (800D4ECC..800D5140): bit i = member i, mCount = number of
// bits set.
class dNetMemberMask_c {
  public:
    dNetMemberMask_c();          // 800D4ECC
    static u8 bit(int member);   // 800D4EFC
    void on(int member);         // 800D4F0C
    void off(int member);        // 800D4F6C
    void clear();                // 800D4FCC
    bool isOn(int member) const; // 800D4FDC
    u8 count() const;            // 800D5024
    // 800D502C: own member per `on`, the others by isStayingMember
    void resetAcks(BOOL on);
    bool isAll() const; // 800D5140: all 4 members set

    /* 0x0 */ u8 mBits;
    /* 0x1 */ u8 mCount;
}; // size 0x2

// A member number, NET_MEMBER_NONE when unset (800D51A0..800D51E0).
class dNetMemberNo_c {
  public:
    dNetMemberNo_c(); // 800D51A0
    int get() const;  // 800D51D0
    void set(int no); // 800D51D8
    void clear();     // 800D51E0: NET_MEMBER_NONE

    /* 0x0 */ u8 mNo;
}; // size 0x1

// A second 1-byte value type of the same shape (800D51E8..800D5228), 4 when
// unset.
class dNetIndex_c {
  public:
    dNetIndex_c();   // 800D51E8
    int get() const; // 800D5218
    void set(int v); // 800D5220
    void clear();    // 800D5228: 4

    /* 0x0 */ u8 mValue;
}; // size 0x1

// A member and its scene (800D5230..800D5274), the NET_PACKET_SCENE_LEADER
// payload.
class dNetMemberScene_c {
  public:
    // 800D5230: NET_MEMBER_NONE, NET_SCENE_NONE
    void init();
    void set(const dNetMemberScene_c &other); // 800D5244
    void set(u32 member, u8 scene);           // 800D5258
    u8 getScene() const;                      // 800D5264
    u32 getMember() const;                    // 800D526C
    // 800D5274: `return this` (called, not inlined)
    dNetMemberScene_c *getData();

    /* 0x0 */ u32 mMember;
    /* 0x4 */ u8 mScene;
}; // size 0x8

// Per member: a scene and an order number (800D5278..800D5328).
class dNetSceneOrder_c {
  public:
    dNetSceneOrder_c();              // 800D5278
    void init();                     // 800D52A8
    void clear(int member);          // 800D52F4: NET_SCENE_NONE, order 4
    void setScene(int member, u8 v); // 800D530C
    u8 getScene(int member) const;   // 800D5314
    // 800D531C (u32: fn_800DACB8 / fn_800DADD8 pass it unextended)
    void setOrder(int member, u32 v);
    // 800D5328 (u32: compared with cmplwi, never extended)
    u32 getOrder(int member) const;

    /* 0x0 */ u8 mScene[NET_MEMBER_MAX];
    /* 0x4 */ u8 mOrder[NET_MEMBER_MAX];
}; // size 0x8

// Per member: a scene and a flag (800D5378..800D55CC).
class dNetMemberScenes_c {
  public:
    dNetMemberScenes_c();            // 800D5378
    void init();                     // 800D53A8
    void clear(int member);          // 800D53F4
    void setScene(int member, u8 v); // 800D5440
    u8 getScene(int member) const;   // 800D5448
    // 800D5450 (int: fn_800DAB04 passes it unextended)
    void setOwner(int member, int v);
    // 800D545C (int: tested with cmpwi, never extended)
    int isOwner(int member) const;
    // 800D5468: the member in `scene` that owns it
    int findOwner(u8 scene) const;
    // 800D54F8: a member in `scene` that does not own it
    int findNonOwner(u8 scene, u32 start) const;
    // 800D5544: present members (isStayingMember) in `scene`
    int countMembers(u8 scene) const;
    // 800D55CC: countMembers == getMemberNum()
    bool isAllIn(u8 scene) const;

    /* 0x0 */ u8 mScene[NET_MEMBER_MAX];
    /* 0x4 */ u8 mOwner[NET_MEMBER_MAX]; // owns (runs) the scene it is in
}; // size 0x8

// The 4-byte bit-packed message built by set (800D5334), the
// NET_PACKET_SCENE_STATE payload.
struct dNetSceneMsg_c {
    // 800D5334
    void set(int member, BOOL hasScene, u8 scene, BOOL hasFlag, BOOL flag,
             int ackTo);

    /* 0x0 */ u32 mMember : 2;
    /* 0x0 */ u8 mHasScene : 1;
    /* 0x1 */ u8 mScene : 7;
    /* 0x1 */ u8 mHasFlag : 1;
    /* 0x2 */ u8 mFlag : 1;
    // the member that answers with NET_PACKET_SCENE_STATE_ACK (>= 4: none)
    /* 0x2 */ u32 mAckTo : 3;
}; // size 0x4

// --------------------------------------------------------------------------------------------------
// The sub-objects of dNet_c, in member order. Layouts from the field accesses;
// refine freely.

// +0x014: the receive buffers (800D5628..800D59EC). One 0x16802-byte buffer (a
// packet with its 2-byte header) per other member; the members' packets are
// queued (mNum) and read in order (getData).
class dNetInBuf_c {
  public:
    enum { SLOT_NUM = 1 }; // buffers per member

    void create(EGG::Heap *heap); // 800D5628
    void init();                  // 800D5674
    void clearMember(int member); // 800D56E4
    // 800D56FC: mBuffer (setupNetMgr hands it to d_net_mgr)
    u8 *getBufferAddr();
    // 800D5704: hands out the buffers (none for this console)
    void setMyNo(int myNo);
    void clearBufs();              // 800D57B8
    void nextSlot(int member);     // 800D57E0
    int getSlot(int member) const; // 800D5808
    // 800D5814: the buffer the next packet is received into
    u8 *getSlotBuf(int member);
    // 800D5860: the oldest received packet (after its header)
    u8 *getData(int member);
    // 800D58F0: a packet was received into `slot`
    void push(int member, int slot);
    // 800D5968: the oldest packet was handled
    void pop(int member);
    int getCredit(int member) const;  // 800D59E0
    void addCredit(int member, s8 n); // 800D59EC
#ifdef BUGFIXES
    u8 *getStagingBuf(int index);
    BOOL queuePacket(int member, const void *data, u32 size);
    u32 getDataSize(int member) const;
#endif

    /* 0x00 */ u8 *mBuffer;
    /* 0x04 */ u8 *mSlotBuf[NET_MEMBER_MAX][SLOT_NUM];
    /* 0x14 */ u8 mSetup;
    /* 0x15 */ u8 mSlot[NET_MEMBER_MAX];     // the next receive slot
    /* 0x19 */ u8 mReadSlot[NET_MEMBER_MAX]; // the oldest unread slot
    /* 0x1D */ u8 mNum[NET_MEMBER_MAX];      // unread packets
    // handled packets not yet reported to the sender
    /* 0x21 */ u8 mCredit[NET_MEMBER_MAX];
#ifdef BUGFIXES
    u32 mDataSize[NET_MEMBER_MAX][SLOT_NUM];
#endif
}; // size 0x28

// +0x03C: the send buffers (800D5A68..800D5D30). One 0x16802-byte buffer per
// other member; a member's buffer can be reused once the member reported the
// packet handled (mPending, see dNetInBuf_c mCredit).
class dNetOutBuf_c {
  public:
    void create(EGG::Heap *heap); // 800D5A68
    void init();                  // 800D5AB4
    void clearMember(int member); // 800D5AF0
    // 800D5B34: hands out the buffers (none for this console)
    void setMyNo(int myNo);
    void clearBufs();                   // 800D5BCC
    u8 *getWritePtr(int member) const;  // 800D5BF4
    BOOL addSize(int member, u32 size); // 800D5C0C
    void resetSize(int member);         // 800D5C40
    void resetSizes();                  // 800D5C54
    void addPending(int member, s8 n);  // 800D5C6C
    void resetPending(int member);      // 800D5D08
    void resetPendings();               // 800D5D18
    u32 getPending(int member) const;   // 800D5D30

    /* 0x00 */ u8 *mBuffer;
    /* 0x04 */ u8 *mBuf[NET_MEMBER_MAX];
    /* 0x14 */ u32 mSize[NET_MEMBER_MAX];
    /* 0x24 */ u8 mSetup;
    // packets sent and not yet reported handled
    /* 0x25 */ u8 mPending[NET_MEMBER_MAX];
}; // size 0x2C

// +0x068 (800D5D3C..800D6FAC): the packet stream and the shared records. A
// frame's blocks (6-byte header + data, begin / add / send) and the changed
// shared records are sent by sendFrame as one packet per member, and received
// ones are applied by receive.
class dNetPacket_c {
  public:
    void create(EGG::Heap *heap);         // 800D5D3C
    void init();                          // 800D5E08
    void clearMember(int member);         // 800D5E78
    void receive();                       // 800D5E90: receive (receiveNetStage)
    void sendFrame();                     // 800D6070: send (updateNetStage)
    u8 *getStream() const;                // 800D6AE8
    u32 getStreamSize() const;            // 800D6AF0
    void clearStream();                   // 800D6B00
    bool canAdd(u32 size) const;          // 800D6B10
    void begin();                         // 800D6B40
    void add(const void *data, u32 size); // 800D6BAC
    void send(int id, int member);        // 800D6C40
    void setRecord(int id, void *data, int arg); // 800D6CDC (setRecord)
    void clearRecords();                 // 800D6E70: clears every shared record
    u8 *getRecordBuf() const;            // 800D6EAC
    static u32 getRecordSize(int id);    // 800D6EB4: shared record size
    static u32 getMaxRecordSize();       // 800D6ED8
    u8 *getRecord(int id) const;         // 800D6F30: shared record
    int getRecordFlag(int id) const;     // 800D6F64
    void setRecordFlag(int id, u8 flag); // 800D6F8C
    void clearRecordFlags();             // 800D6FAC

    // the blocks of this frame (0x16800)
    /* 0x00 */ u8 *mStream;
    /* 0x04 */ u8 *mBlock; // the current block's header
    // the write position the shared records (offsets: getRecord)
    /* 0x08 */ u8 *mStreamEnd;
    /* 0x0C */ u8 *mRecordBuf;
    // a record copy between guard bytes (setRecord)
    /* 0x10 */ u8 *mWork;
    /* 0x14 */ u32 mWorkSize;
    // per shared record cCounter_c::m_gameFrame of the last update
    /* 0x18 */ u8 *mRecordFlags;
    /* 0x1C */ u32 mFrame;
    /* 0x20 */ u8 mSuspended;
    /* 0x21 */ dNetMemberNo_c mLeaving;
    // per member: frames without data (> 0xE10: error 2)
    /* 0x24 */ u32 mSendWait[NET_MEMBER_MAX];
    /* 0x34 */ u32 mIdleFrames[NET_MEMBER_MAX];
}; // size 0x44

// +0x0AC (800D6FFC..800D7080, B): a 0x16800-byte buffer of blocks held back
// while this console's scene changes owner (mOldOwner) or a member enters it
// (mEnterMember); dNetPacket_c::sendFrame flushes it, dNetPacket_c::receive
// applies it.
class dNetHoldBuf_c {
  public:
    void create(EGG::Heap *heap);         // 800D6FFC
    void init();                          // 800D7048
    void clearMember(int member);         // 800D7064 (nothing)
    u8 *getBuf() const;                   // 800D7068
    u32 getSize() const;                  // 800D7070
    void setSize(u32 size);               // 800D7078
    void add(const void *data, u32 size); // 800D7080: ignored if it doesn't fit

    /* 0x00 */ u8 *mBuf;
    /* 0x04 */ u32 mSize;
    /* 0x08 */ s32 mEnterMember; // the member entering the owner's scene
    // the scene's previous owner (NET_PACKET_SCENE_HANDOVER_REQUEST)
    /* 0x0C */ s32 mOldOwner;
    /* 0x10 */ s32 mHoldState; // 0..2 (2: flush)
}; // size 0x14

// +0x0C0 (800D70D8..800D7F7C, B): the request / save state machine. A member's
// request (mKind: 0 connect, 1 disconnect, 2 continue, 3 interrupt; also the
// save kind) runs on the host (member 0) through per member steps
// (mMemberStep); the save step (mSaveStep) shows the save message and saves.
class dNetRequest_c {
  public:
    void init();                         // 800D70D8
    void clearMember(int member);        // 800D7180
    void execute();                      // 800D71D8: update (updateNetStage)
    void executeLeave();                 // 800D7CB8: (postNetStage)
    void setState(int member, u8 state); // 800D7D34
    int getState(int member) const;      // 800D7D40
    // 800D7D4C: first member with a state, else 4
    int findActive() const;
    BOOL sendResult(int member, BOOL ok);    // 800D7DAC
    void setKind(int member, int kind);      // 800D7F2C
    int getKind(int member) const;           // 800D7F40
    void setStep(u8 step);                   // 800D7F4C
    int getStep() const;                     // 800D7F54
    void setMemberStep(int member, u8 step); // 800D7F5C
    int getMemberStep(int member) const;     // 800D7F68
    void setSaveStep(u8 step);               // 800D7F74
    int getSaveStep() const;                 // 800D7F7C

    /* 0x00 */ dNetIndex_c mGrantedKind;
    /* 0x01 */ dNetMemberNo_c mGrantedMember;
    /* 0x02 */ u8 mState[NET_MEMBER_MAX];
    /* 0x06 */ dNetMemberMask_c mRequests;
    /* 0x08 */ dNetIndex_c mKind[NET_MEMBER_MAX];
    /* 0x0C */ u8 mStep;
    /* 0x0D */ dNetIndex_c mMyKind;
    /* 0x0E */ u8 mMemberStep[NET_MEMBER_MAX];
    /* 0x12 */ u8 mSaveStep;
    /* 0x14 */ u32 mSaveTimer;
    /* 0x18 */ dNetIndex_c mSaveKind;
    /* 0x19 */ dNetMemberNo_c mSaveMember;
    /* 0x1A */ u8 mMsgOn;
    /* 0x1C */ u32 mMsgTimer;
    /* 0x20 */ u32 mMsgEndTimer;
    /* 0x24 */ dNetMemberMask_c mBusy;
    /* 0x26 */ u8 mLeave;
}; // size 0x28

// +0x0E8 (800D7F84..800D8FFC, B): the data exchange after a member joined
// (start): the new member's player, item mask, town (0x74C15 bytes in parts),
// player again and the sendable items (mItems), through the direct kinds
// NET_DIRECT_XFER_* (4..10) and NET_DIRECT_JOIN_DONE.
class dNetTransfer_c {
  public:
    dNetTransfer_c() {
        memset(mExclude, 0, sizeof(mExclude));
    }

    void init();                  // 800D7F84
    void clearMember(int member); // 800D8064
    void execute();               // 800D8080: update (updateNetStage)
    void start();                 // 800D855C
    void setStep(u8 step);        // 800D85E0
    int getStep() const;          // 800D85E8
    void collectItems();          // 800D85F0
    BOOL sendStart(int member);   // 800D8664: NET_DIRECT_XFER_START
    // 800D8688: NET_DIRECT_XFER_PLAYER to everyone
    BOOL sendPlayer();
    BOOL sendItemMask(int member); // 800D896C: NET_DIRECT_XFER_ITEM_MASK
    // 800D8A24: NET_DIRECT_XFER_TOWN, the next part; TRUE when all is sent
    BOOL sendTown(int member);
    BOOL sendPlayer(int member); // 800D8CB4: NET_DIRECT_XFER_PLAYER_REPLY
    // 800D8DB4: NET_DIRECT_XFER_ITEMS, the next 11 items; TRUE when all are
    // sent
    BOOL sendItems(int member);
    BOOL sendTownReq();      // 800D8FA0: NET_DIRECT_XFER_TOWN_REQUEST
    BOOL sendFinish();       // 800D8FC4: NET_DIRECT_JOIN_DONE
    void setScene(u8 scene); // 800D8FE8
    void setReady();         // 800D8FF0
    BOOL isAllReady() const; // 800D8FFC

    /* 0x000 */ u8 mStep;
    /* 0x001 */ u8 mExclude[0x20]; // item mask (countToCstmSendable)
    /* 0x021 */ u8 mItemMaskRecv;
    /* 0x024 */ u32 mItemSent;
    /* 0x028 */ u32 mItemNum;
    /* 0x02C */ void *mItems[0x100];
    /* 0x42C */ u8 mItemsRecv;
    /* 0x430 */ u32 mTownSent;
    /* 0x434 */ u8 mReady[NET_MEMBER_MAX];
    /* 0x438 */ u8 mTownReqSend;
    /* 0x439 */ u8 mTownReqRecv;
    /* 0x43A */ u8 mFinish;
    /* 0x43B */ u8 mScene;
    /* 0x43C */ u8 mStartRecv;
#ifdef BUGFIXES
    u32 mTownRecv;
#endif
}; // size 0x440

// +0x528 (800D90A0..800D9368, B): the end of a session with a save (start:
// continue or interrupt). mSaved: the consoles that saved (NET_PACKET_SAVE_DONE
// / NET_PACKET_SAVE_DONE_ALL).
class dNetSaveEnd_c {
  public:
    void init();                  // 800D90A0
    void clearMember(int member); // 800D90B0
    void start(BOOL interrupt);   // 800D90B8
    void execute();               // 800D9170: update (updateNetStage)
    void executeAfterSend();      // 800D9300 (dNetPacket_c::sendFrame)
    void setStep(u8 step);        // 800D9360
    int getStep() const;          // 800D9368

    /* 0x0 */ u8 mStep;
    /* 0x1 */ dNetMemberMask_c mSaved;
}; // size 0x3

// +0x52B (800D9370..800D98A8, B): a member disconnects (request kind 1, start).
class dNetDisconnect_c {
  public:
    void init();                  // 800D9370
    void clearMember(int member); // 800D93B8
    void start(BOOL fromJoin);    // 800D9400
    void execute();               // 800D950C: update (updateNetStage)
    BOOL canDeleteScene();        // 800D9728: (isNetPostReady)
    void executePostDelete();     // 800D9784: (postNetStage)
    void executeAfterSend();      // 800D9820 (dNetPacket_c::sendFrame)
    void setStep(u8 step);        // 800D98A0
    int getStep() const;          // 800D98A8

    /* 0x0 */ u8 mStep;
    /* 0x1 */ dNetMemberNo_c mMember;
    /* 0x2 */ dNetMemberMask_c mAcks;
    // NET_PACKET_DISCONNECT_READY / _GO (setDisconnectReady)
    /* 0x4 */ dNetMemberMask_c mReady;
}; // size 0x6

// +0x531 (800D98B0..800D99EC, B): the members that finished their join, and the
// host's "all done".
class dNetJoinDone_c {
  public:
    void init();                    // 800D98B0
    void clearMember(int member);   // 800D98CC
    void setDone(int member, u8 v); // 800D98D8
    // 800D98E0: own member and the absent ones set
    void reset();
    BOOL isAllDone() const; // 800D996C: set for every present member
    void setAllDone(u8 v);  // 800D99E4
    u8 getAllDone() const;  // 800D99EC

    /* 0x0 */ u8 mDone[NET_MEMBER_MAX];
    /* 0x4 */ u8 mAllDone;
}; // size 0x5

// The data channels (+0x538..+0x564, 800D99F4..800DA650, B): a buffer of
// entries (dNetSceneCloseEntry_c..E_c header + data), filled by write() from
// the entry handlers of the channel's send table (an entry is left out when its
// handler returns < 0) and applied by read() through the receive table. The
// tables are in d_net.cpp, their handlers (dNetSendFunc .. below) in
// d_net_sync.inc.

// +0x538: the close channel, 3 entries (lbl_80474BF0),
// NET_PACKET_SCENE_SAVE_HOST.
class dNetSceneClose_c {
  public:
    void create(EGG::Heap *heap); // 800D99F4
    void init();                  // 800D9A3C
    void clearMember(int member); // 800D9A48 (nothing)
    u8 *getBuf() const;           // 800D9A4C
    u32 getSize() const;          // 800D9A54
    // 800D9A5C: NET_PACKET_SCENE_SAVE_HOST handler
    void setData(const void *data, u32 size);
    static u32 getEntrySize(int i);   // 800D9AAC
    void write(int member, u8 scene); // 800D9AD0
    // 800D9BAC: NET_PACKET_SCENE_SAVE_HOST handler
    void read(int member, u8 scene);

    /* 0x0 */ u8 *mBuf;
    /* 0x4 */ u32 mSize;
}; // size 0x8

// +0x540: the open channel, 5 entries (lbl_80474C14),
// NET_PACKET_SCENE_LOAD_REQUEST / NET_PACKET_SCENE_LOAD.
class dNetSceneOpen_c {
  public:
    void create(EGG::Heap *heap); // 800D9CA8
    void init();                  // 800D9CF0
    void clearMember(int member); // 800D9D00 (nothing)
    u8 *getBuf() const;           // 800D9D04
    u32 getSize() const;          // 800D9D0C
    // 800D9D14: NET_PACKET_SCENE_LOAD handler
    void setData(const void *data, u32 size);
    void setRecv(u8 recv);          // 800D9D64
    int isRecv() const;             // 800D9D6C
    static u32 getEntrySize(int i); // 800D9D74
    // 800D9D98: NET_PACKET_SCENE_LOAD_REQUEST handler
    void write(int member, u8 scene);
    void read(int member, u8 scene); // 800D9E74

    /* 0x0 */ u8 *mBuf;
    /* 0x4 */ u32 mSize;
    /* 0x8 */ u8 mRecv;
}; // size 0xC

// +0x54C: the leave channel, 1 entry (lbl_807508CC),
// NET_PACKET_SCENE_EXIT_DATA.
class dNetSceneLeave_c {
  public:
    void create(EGG::Heap *heap); // 800D9F70
    void init();                  // 800D9FB8
    void clearMember(int member); // 800D9FC4 (nothing)
    u8 *getBuf() const;           // 800D9FC8
    u32 getSize() const;          // 800D9FD0
    // 800D9FD8: NET_PACKET_SCENE_EXIT_DATA handler
    void setData(const void *data, u32 size);
    static u32 getEntrySize(int i);   // 800DA028
    void write(int member, u8 scene); // 800DA048
    // 800DA0F8: NET_PACKET_SCENE_EXIT_DATA handler
    void read(int member, u8 scene);

    /* 0x0 */ u8 *mBuf;
    /* 0x4 */ u32 mSize;
}; // size 0x8

// +0x554: the enter channel, 1 entry (lbl_807508D8),
// NET_PACKET_SCENE_ENTER_DATA.
class dNetSceneEnter_c {
  public:
    void create(EGG::Heap *heap); // 800DA1F0
    void init();                  // 800DA238
    void clearMember(int member); // 800DA250 (nothing)
    u8 *getBuf() const;           // 800DA254
    u32 getSize() const;          // 800DA25C
    // 800DA264: NET_PACKET_SCENE_ENTER_DATA handler
    void setData(const void *data, u32 size);
    // 800DA2B4: NET_PACKET_SCENE_ENTER_DATA handler
    void setRecv(u8 recv);
    int isRecv() const;               // 800DA2BC
    static u32 getEntrySize(int i);   // 800DA2C4
    void write(int member, u8 scene); // 800DA2E4
    void read(int member, u8 scene);  // 800DA394

    /* 0x0 */ u8 *mBuf;
    /* 0x4 */ u32 mSize;
    /* 0x8 */ u8 mRecv;
    /* 0xC */ s32 mRequester; // a member
}; // size 0x10

// +0x564: the handover channel, 2 entries per member (lbl_807508E8),
// NET_PACKET_SCENE_HANDOVER_DATA.
class dNetSceneHandover_c {
  public:
    void create(EGG::Heap *heap); // 800DA48C
    void init();                  // 800DA4D4
    void clearMember(int member); // 800DA4E8 (nothing)
    u8 *getBuf() const;           // 800DA4EC
    u32 getSize() const;          // 800DA4F4
    // 800DA4FC: NET_PACKET_SCENE_HANDOVER_DATA handler
    void setData(const void *data, u32 size);
    static u32 getEntrySize(int i); // 800DA54C
    void write(u8 scene);           // 800DA56C: for mNewOwner
    // 800DA650: NET_PACKET_SCENE_HANDOVER_DATA handler
    void read(int self, int from, u8 scene);

    /* 0x0 */ u8 *mBuf;
    /* 0x4 */ u32 mSize;
    /* 0x8 */ s32 mNewOwner; // a member
}; // size 0xC

// A one-byte scene request (NET_PACKET_SCENE_REQUEST): enter `mScene`, or leave
// (mLeave).
struct dNetSceneReq_c {
    /* 0x0 */ u8 mLeave : 1;
    /* 0x0 */ u8 mScene : 7;
}; // size 0x1

// +0x570: which scene each member is in, and the scene change state machine
// (800DA754..800DB78C).
class dNetScene_c {
  public:
    void init(int);               // 800DA754 (B)
    void clearMember(int member); // 800DA7D8
    void setLeaveAcked(bool v);   // 800DA830
    bool getLeaveAcked() const;   // 800DA838
    bool isHandoverAcked() const; // 800DA840
    // 800DA8A0: init, only member 0 in the current scene
    void reset();
    void enter(int member, u8 scene);         // 800DA900
    int getState() const;                     // 800DA950
    void setState(int state);                 // 800DA958
    void clearScenes(int arg);                // 800DA960
    void clearScene(int member, int arg);     // 800DA9BC
    void setScene(int member, u8 scene, int); // 800DAA1C
    // 800DAAFC (u8: a tail call of dNetMemberScenes_c::getScene)
    u8 getScene(int member) const;
    void setOwner(int member, int owner, int);   // 800DAB04
    int isOwner(int member) const;               // 800DAC90
    int findOwner(u8 scene) const;               // 800DAC98
    int findNonOwner(u8 scene, int start) const; // 800DACA0
    int countMembers(u8 scene) const;            // 800DACA8
    bool isAllIn(u8 scene) const;                // 800DACB0
    // 800DACB8: queue a scene change request (host)
    void addOrder(int member, u8 scene);
    void removeOrder(int member); // 800DADD8
    // 800DAF20: the member whose scene change runs
    int getLeader() const;
    u8 getLeaderScene() const;                       // 800DAF98
    bool hasOrder() const;                           // 800DB004
    void setLeader(const dNetMemberScene_c &leader); // 800DB064
    void clearLeader();                              // 800DB06C
    void enterLeaderScene();                         // 800DB074
    // 800DB164: receive stage (receiveNetStage)
    void executeRecv();
    // 800DB504: update stage (updateNetStage)
    void execute();
    // 800DB5E8: post stage (postNetStage)
    void executePostDelete();
    // 800DB78C: send stage (dNetPacket_c::sendFrame)
    void executeAfterSend();

    /* 0x00 */ s32 mState;
    /* 0x04 */ dNetMemberScenes_c mScenes;
    /* 0x0C */ dNetSceneOrder_c mOrder;
    /* 0x14 */ dNetMemberScene_c mLeader;
    /* 0x1C */ dNetMemberMask_c mLeaderAcks;
    /* 0x1E */ dNetMemberMask_c mMsgAcks;
    /* 0x20 */ s32 mLeaving;
    /* 0x24 */ bool mLeaveAcked;
    /* 0x25 */ dNetMemberMask_c mHandoverAcks;
}; // size 0x28

// The handlers of the dNetSyncBuf_c tables (in d_net_sync.inc).
typedef void (*dNetSyncFunc)(int member, u8 *data, u32 size);
typedef BOOL (*dNetSyncPackFunc)(int member, const dNetMemberMask_c *mask,
                                 u8 *dst, u32 size);
typedef void (*dNetSyncUnpackFunc)(const dNetMemberMask_c &mask, const u8 *data,
                                   u32 size);

#define SYNC_BUF0_SIZE 2
#define SYNC_BUF4_SIZE 2
#define SYNC_BUF8_SIZE 0x3074

// +0x598: per member data buffers, the last one sent with NET_PACKET_SYNC_BUF
// (800DBA94..800DBFEC). Each buffer is split into records by the tables
// 80750900.., 80750918.., 80474C50.. (sizes, handlers).
class dNetSyncBuf_c {
  public:
    void create(EGG::Heap *heap);             // 800DBA94
    void init();                              // 800DBB2C
    void clearMember(int member);             // 800DBB44
    u8 *getBuf0() const;                      // 800DBB58
    static u32 getBuf0Size(int i);            // 800DBB60
    u8 *getBuf0Data(int i) const;             // 800DBB80
    void writeBuf0(int arg);                  // 800DBBB0
    void readBuf0(int arg);                   // 800DBC2C
    u8 *getBuf4(int member) const;            // 800DBCA8
    static u32 getBuf4Size(int i);            // 800DBCCC
    u8 *getBuf4Data(int member, int i) const; // 800DBCEC
    void writeBuf4(int member);               // 800DBD28
    void readBuf4(int member);                // 800DBDA8
    u8 *getBuf8(int member) const;            // 800DBE28
    static u32 getBuf8Size(int i);            // 800DBE4C
    // 800DBE70: the mask, then the records it selects
    void pack(int member, const dNetMemberMask_c *mask);
    // 800DBF34: NET_PACKET_SYNC_BUF (D: r4 is the
    void unpack(const dNetMemberMask_c &mask, const u8 *data, u32 size);
    // address of a local dNetMemberMask_c, passed on to the handlers)
    // 800DBFEC: NET_PACKET_SYNC_BUF to every other member
    void send(const dNetMemberMask_c *mask);

    /* 0x00 */ u8 *mBuf0; // SYNC_BUF0_SIZE
    /* 0x04 */ u8 *mBuf4; // members 1..3 x SYNC_BUF4_SIZE
    /* 0x08 */ u8 *mBuf8; // members 1..3 x SYNC_BUF8_SIZE
    /* 0x0C */ u32 mSize[NET_MEMBER_MAX];
}; // size 0x1C

// +0x5B4 (800DC0AC, 800DC0B8).
class dNetFlag_c {
  public:
    void init();                  // 800DC0AC
    void clearMember(int member); // 800DC0B8 (nothing)

    /* 0x0 */ u8 mStarted;
}; // size 0x1

// +0x5B8: this console's names: dNetPlayerInfo::Info_c (d_net_player_info).

// --------------------------------------------------------------------------------------------------
// The state (805B2160). The accessors are getNetMyNo..getNetPlayerInfo.
struct dNet_c {
    /* 0x000 */ dNetMemberNo_c mMyNo; // this console's member number
    /* 0x001 */ dNetMemberMask_c mAccepted;
    // the members present (count: getMemberNum)
    /* 0x003 */ dNetMemberMask_c mMembers;
    /* 0x005 */ dNetMemberMask_c mWatched;
    /* 0x007 */ dNetMemberMask_c mLinked;
    /* 0x009 */ dNetMemberMask_c mHelloRecv;
    /* 0x00B */ dNetMemberMask_c mHelloSend;
    /* 0x00D */ dNetMemberMask_c mHelloReply;
    /* 0x00F */ dNetMemberMask_c mHelloHold;
    /* 0x014 */ dNetInBuf_c mInBuf;
    /* 0x03C */ dNetOutBuf_c mOutBuf;
    /* 0x068 */ dNetPacket_c mPacket;
    /* 0x0AC */ dNetHoldBuf_c mHoldBuf;
    /* 0x0C0 */ dNetRequest_c mRequest;
    /* 0x0E8 */ dNetTransfer_c mTransfer;
    /* 0x528 */ dNetSaveEnd_c mSaveEnd;
    /* 0x52B */ dNetDisconnect_c mDisconnect;
    /* 0x531 */ dNetJoinDone_c mJoinDone;
    /* 0x538 */ dNetSceneClose_c mSceneClose;
    /* 0x540 */ dNetSceneOpen_c mSceneOpen;
    /* 0x54C */ dNetSceneLeave_c mSceneLeave;
    /* 0x554 */ dNetSceneEnter_c mSceneEnter;
    /* 0x564 */ dNetSceneHandover_c mSceneHandover;
    /* 0x570 */ dNetScene_c mScene;
    /* 0x598 */ dNetSyncBuf_c mSyncBuf;
    /* 0x5B4 */ dNetFlag_c mMgrStart;
    // an online session is active (isSessionActive)
    /* 0x5B5 */ bool mActive;
    /* 0x5B6 */ u8 mJoined; // mMyNo is valid (getMyNo)
    /* 0x5B8 */ dNetPlayerInfo::Info_c mPlayerInfo;
    // dTime_c::applyPending on the next postNetStage
    /* 0x61E */ u8 mTimePending;
    // 7 = none; dPlayerMgr::getCurrentPlayerSlot error bits (getNetError / setNetError)
    /* 0x620 */ s32 mSavedSlot;
    /* 0x624 */ s32 mError;
    /* 0x628 */ s32 mErrorReported;
    /* 0x62C */ u8 mSendCount[NET_MEMBER_MAX];
    /* 0x630 */ u8 mRecvCount[NET_MEMBER_MAX];
    /* 0x638 */ s64 mTime[NET_MEMBER_MAX]; // getMemberTime / setMemberTime
    /* 0x658 */ u8 mJoinStarted;
}; // size 0x660

extern dNet_c g_dNet; // 805B2160

// --------------------------------------------------------------------------------------------------
// The free functions. Their target names are unknown (stripped); they are
// grouped in namespace dNet, and other units call them as dNet::name.
namespace dNet {

// Main part internals (800D3590..800D4A68): errors, packet headers, sending,
// the d_net_mgr callbacks and the session hook wrappers. 800D3590: mError, 0
// while resetting (callers test the NET_ERROR_* bits)
u32 getNetError();
void setNetError(u32 flag); // 800D35EC: mError |= flag unless resetting
// 800D3654: stores the member's packet counter; NET_ERROR_SEQUENCE when check
// and it does not follow the previous one
void updateRecvCount(int no, u8 seq, BOOL check);
// 800D36DC: range check of id / size / dest / scene / from of the 6-byte block
// header
BOOL isBlockHeaderValid(int id, u32 size, int dest, u32 scene, int from);
// 800D3718: packs dNetBlockHeader into dst
void writeBlockHeader(void *dst, int id, u32 size, int dest, u8 scene,
                      int from);
// 800D37D0: unpacks dNetBlockHeader, TRUE if valid
BOOL readBlockHeader(const void *src, int *id, u32 *size, int *dest, u8 *scene,
                     int *from);
// 800D3890: mHelloRecv && !mHelloReply && mLinked && !mHelloSend: the handshake
// with the member is complete
BOOL isMemberLinked(int no);
// 800D392C: member not joining: d_net_mgr connected bit; joining (mHelloHold):
// isMemberLinked
BOOL isMemberConnected(int no);
// 800D39CC: (check: linked) && dNetMgr_c::canSend && (no data || a free out
// buffer)
BOOL canSendTo(int no, BOOL hasData, BOOL check);
// 800D3A64: canSendTo every other member of mAccepted, collecting them in bits
BOOL canSendToAccepted(BOOL hasData, dNetMemberMask_c *bits, BOOL check);
// 800D3B0C: canSendTo every other staying member (isStayingMember), collecting
// them in bits
BOOL canSendToMembers(BOOL hasData, dNetMemberMask_c *bits, BOOL check);
// 800D3BB0: lbl_80473E00[kind]: -1 no counter, 0 store it, 1 check it
// (updateRecvCount)
int getDirectSeqMode(int kind);
// 800D3BD4: lbl_80473E10[kind](member, data, size)
void callDirectHandler(int no, const u8 *data, u32 size, int kind);
// 800D3BF8: walks the packet's blocks: lbl_80473E44[id](from, data, len, dest,
// scene)
#ifndef BUGFIXES
void callBlockHandlers(const u8 *data, u32 size);
#else
void callBlockHandlers(const u8 *data, u32 size, int sender);
#endif
// 800D3CC8: lbl_80474050[id](dst, src, arg)
void copyRecord(int id, void *dst, const void *src, int arg);
// 800D3CF8: zeroes shared record id, runs its clear handler (lbl_80474338),
// flag 0
void clearRecord(int id);
// 800D3D98: sync buf0 handlers B for the own member + hook onVisitorClear
// (dSvMgr_c::mCurrentVisitor = none); gatekeeper REL
void clearVisitor();
// 800D3E14: hook onVisitorSet(mRequest.mSaveMember): dSvMgr_c::mCurrentVisitor
// = the member being saved; d_sv_mgr waitNewPlayer
void setSaveVisitor();
// 800D3E78: disconnect seq (dNetDisconnect_c) step 4 on the leaving console:
// hook canFinishLeave(leaver, others)
BOOL checkCanFinishLeave();
// 800D3EDC: disconnect seq post stage step 4 (leaving console, before
// terminateNetSession): hook onLeave(leaver, others)
void callOnLeave();
// 800D3F40: disconnect seq post stage step 8 (other consoles, before
// removeNetMember): hook onMemberLeft(leaver, others)
void callOnMemberLeft();
// 800D3FA4: session end seq (dNetSaveEnd_c steps 1 / 4): hook onLeave for every
// staying member
void callOnLeaveAll();
// 800D3FFC: disconnect seq step 3 after closeNetConnection: hook
// onDisconnect(leaver, others)
void callOnDisconnect();
// 800D4060: disconnect seq step 4: hook isDisconnectDone(leaver, others)
BOOL checkDisconnectDone();
// 800D40C4: disconnect seq send stage step 6 (other consoles): hook
// onMemberLeaving(leaver, others)
void callOnMemberLeaving();
// 800D4128: disconnect seq step 7 (other consoles): hook
// isMemberLeaveDone(leaver, others)
BOOL checkMemberLeaveDone();
// 800D418C: session end seq step 5 after closeNetConnection: hook onDisconnect
// for every staying member
void callOnDisconnectAll();
// 800D41E4: session end seq step 6: hook isDisconnectDone for every staying
// member
BOOL checkDisconnectDoneAll();
// 800D4260: d_net_mgr receive callback (setCallbacks recv): packet header, then
// direct handler or queued in dNetInBuf_c
void onNetMgrRecv(u8 no, const void *data, u32 size);
// 800D43D8: d_net_mgr join callback (setCallbacks join): mLinked / mHelloSend
// on
void onNetMgrJoin(u32 no);
// 800D44A0: d_net_mgr leave callback (setCallbacks leave): mLinked /
// mHelloRecv / mHelloSend / mHelloReply off
void onNetMgrLeave(u32 no);
// 800D4534: d_net_mgr callback (setCallbacks event), empty; the event is
// unknown
void onNetMgrEvent(int, int);
// 800D4538: writes the 2-byte dNetPacketHeader (valid, credit, direct, kind,
// count), returns the credit
int writePacketHeader(u8 **pp, int no, BOOL direct, int kind);
// 800D4614: writes the u16 stream / extra sizes (dNetStreamSizes)
void writeStreamSizes(void *dst, u32 stream, u32 extra);
// 800D4644: !dNetMgr_c::sInstance->isConnected(member)
BOOL isMemberDisconnected(int no);
// 800D4668: one direct (kind) packet to one member through d_net_mgr
// dNetMgr_c::send
BOOL sendDirectTo(int no, const void **datas, const u32 *sizes, u32 num,
                  int kind, BOOL wait, BOOL check, BOOL noCount);
// 800D48D0: sendDirectTo one member, or to every accepted member (member >=
// NET_MEMBER_MAX)
BOOL sendDirect(int no, const void **datas, const u32 *sizes, u32 num, int kind,
                BOOL wait, BOOL check, BOOL noCount);
// 800D4A0C: first member >= start that is not 0 and is staying, else
// NET_MEMBER_NONE
int findNextGuest(u32 no);
// 800D4A68: current scene invalid, a next scene set, a scene change state set,
// or a queued scene order
BOOL isSceneChanging();

// The accessors (800DC0BC..800DC2C8: &g_dNet.member and a few flags).
dNetMemberNo_c *getNetMyNo(); // 800DC0BC: &g_dNet.mMyNo (dNetMemberNo_c *)
// 800DC0C8: &g_dNet.mAccepted: members accepted into the session, incl. one
// still joining (NET_DIRECT_JOIN_RESULT)
dNetMemberMask_c *getNetAccepted();
// 800DC0D8: &g_dNet.mMembers: members fully joined (count = getMemberNum)
dNetMemberMask_c *getNetMembers();
// 800DC0E8: &g_dNet.mWatched: members whose loss raises NET_ERROR_LOST
dNetMemberMask_c *getNetWatched();
dNetInBuf_c *getNetInBuf();       // 800DC0F8: &g_dNet.mInBuf
dNetOutBuf_c *getNetOutBuf();     // 800DC108: &g_dNet.mOutBuf
dNetPacket_c *getNetPacket();     // 800DC118: &g_dNet.mPacket
dNetHoldBuf_c *getNetHoldBuf();   // 800DC128: &g_dNet.mHoldBuf
dNetRequest_c *getNetRequest();   // 800DC138: &g_dNet.mRequest
dNetTransfer_c *getNetTransfer(); // 800DC148: &g_dNet.mTransfer
// 800DC158: &g_dNet.mSaveEnd (session end with a save: host / visitor continue
// or interrupt)
dNetSaveEnd_c *getNetSaveEnd();
// 800DC168: &g_dNet.mDisconnect (a member disconnects: request kind 1)
dNetDisconnect_c *getNetDisconnect();
// 800DC178: &g_dNet.mJoinDone (per member "join done" flags + the host's:
// NET_DIRECT_JOIN_DONE*, NET_PACKET_JOIN_DONE*)
dNetJoinDone_c *getNetJoinDone();
// 800DC188: &g_dNet.mSceneClose (the close channel: the last member leaving a
// scene -> host, NET_PACKET_SCENE_SAVE_HOST)
dNetSceneClose_c *getNetSceneClose();
// 800DC198: &g_dNet.mSceneOpen (the open channel: host -> the first member
// entering a scene, NET_PACKET_SCENE_LOAD*)
dNetSceneOpen_c *getNetSceneOpen();
// 800DC1A8: &g_dNet.mSceneLeave (the leave channel: a non-owner leaving -> the
// owner, NET_PACKET_SCENE_EXIT_DATA)
dNetSceneLeave_c *getNetSceneLeave();
// 800DC1B8: &g_dNet.mSceneEnter (the enter channel: the owner -> an entering
// member, NET_PACKET_SCENE_ENTER_*)
dNetSceneEnter_c *getNetSceneEnter();
// 800DC1C8: &g_dNet.mSceneHandover (the handover channel: the old owner -> the
// new one, NET_PACKET_SCENE_HANDOVER_*)
dNetSceneHandover_c *getNetSceneHandover();
dNetScene_c *getNetScene();     // 800DC1D8: &g_dNet.mScene
dNetSyncBuf_c *getNetSyncBuf(); // 800DC1E8: &g_dNet.mSyncBuf
// 800DC1F8: &g_dNet.mMgrStart (started by openNetConnection, cleared by
// closeNetConnection)
dNetFlag_c *getNetMgrStart();
void setSessionActive(bool active); // 800DC208: g_dNet.mActive = active
// 800DC218: g_dNet.mActive (wrapped by isSessionActive)
bool getSessionActive();
// 800DC228: g_dNet.mMembers.isOn(member) (wrapped by isMember)
bool isMemberIn(int member);
// 800DC23C: isMemberIn(member) and not the member whose leave is running
// (mPacket.mLeaving)
BOOL isStayingMember(int member);
void setJoined(u8 joined); // 800DC288: g_dNet.mJoined
BOOL isJoined(); // 800DC298: g_dNet.mJoined (getMyNo returns 0 unless set)
// 800DC2A8: mPlayerInfo.set(): the current player's names
void setNetPlayerInfo();
void clearNetPlayerInfo(); // 800DC2B8: mPlayerInfo.clear()
// 800DC2C8: &g_dNet.mPlayerInfo (given to d_net_mgr)
dNetPlayerInfo::Info_c *getNetPlayerInfo();

// The session and the frame stages (800DC2D8..800DCAE0).
// 800DC2D8: allocates the sub-objects' buffers from heap, then resetNetState(1)
void createNetState(EGG::Heap *heap);
// 800DC380: clears every sub-object, mask and counter (arg: dNetScene_c::init)
void resetNetState(int arg);
// 800DC4FC: clears one member's state in every sub-object and mask (disconnect
// seq step 8)
void removeNetMember(int member);
// 800DC680: player info, receive buffers to d_net_mgr (dNetMgr_c::setup),
// registers the callbacks (dNetMgr_c::setCallbacks)
void setupNetMgr();
void setNetMyNo(int myNo); // 800DC738: mMyNo + in / out buffer setMyNo
// 800DC784: member 0 accepted + present, joined, active, scene reset, player
// slots, records cleared, hook onSessionStart
void startNetHostSession();
// 800DC830: hook onSessionEnd, player slot 0 restored, resetNetState, d_net_mgr
// dNetMgr_c::cleanup, Mii icons off
void terminateNetSession(int arg);
// 800DC88C: d_net_mgr receive, then packet receive + scene receive stage
void receiveNetStage();
// 800DC8F8: sub-object updates + send, then timeout / lost-member errors and
// the error report
void updateNetStage();
// 800DCA74: FALSE while the leaving console's disconnect seq cannot finish
// (dNetDisconnect_c::canDeleteScene)
BOOL isNetPostReady();
// 800DCAE0: pending time apply, join leave flag, scene post stage, disconnect
// seq post stage
void postNetStage();

// The public API (800DCB68..800DDF78): used by the other units (DOL and RELs).
// 800DCB68: d_heap createNetGameHeap: the size of the net game heap
u32 getNetHeapSize();
// 800DCBB0: d_sys fn_8016BA7C: createNetState(heap)
void createNet(EGG::Heap *heap);
void receiveNet();       // 800DCBB4: d_sys fn_8016C3C0: receiveNetStage
void updateNet();        // 800DCBB8: d_sys fn_8016C3C0: updateNetStage
BOOL isNetReadyToPost(); // 800DCBBC: d_s_stage REL: isNetPostReady
void postNet();          // 800DCBC0: d_scene_base fn_80155490: postNetStage
void initNetMgr();       // 800DCBC4: gatekeeper REL: setupNetMgr
// 800DCBC8: gatekeeper REL: sets the open flag, TRUE once dNetMgr_c::open
// reports the connection up
BOOL openNetConnection();
// 800DCC54: gatekeeper REL: dNetMgr_c::fn_800E51C8 (mgr state 5), then member 0
// and startNetHostSession
BOOL startHost();
// 800DCCEC: gatekeeper REL: dNetMgr_c::fn_800E5298, own member number from
// d_net_mgr, joined, net player copy
BOOL startGuest(int arg);
// 800DCDFC: d_sv_mgr stepSaveNormal_c::disconnectWiFi, gatekeeper:
// closeNetConnection, then endSession(arg)
BOOL disconnectNet(int arg);
// 800DCE44: TRUE once dNetMgr_c::close closed it (clears the open flag),
// else stops sending (mSuspended); d_reset_save
BOOL closeNetConnection();
// 800DCEB8: d_reset_save fn_80107C18: terminateNetSession(arg)
void endSession(int arg);
// 800DCEBC: dNetMgr_c::sInstance->mState > 1 (d_net_mgr state past idle);
// d_home_button, d_menu_friend08
BOOL isNetMgrRunning();
bool isSessionActive(); // 800DCEDC: mActive
// 800DCEE0: mPacket.mSuspended: sending stopped (leave / session end /
// closing); d_scene adjustChkpScene, dScene::dSceneChange_c::request
BOOL isNetStopped();
// 800DCF04: dNetDisconnect_c::mMember: the member whose leave is running;
// d_player_mngr, gatekeeper
int getLeaveMember();
bool isMember(int member); // 800DCF2C: isMemberIn(member)
u32 getMemberNum();        // 800DCF30: mMembers.count()
int getMyNo(); // 800DCF58: this console's member number, 0 unless joined
BOOL isHost(); // 800DCF90: getMyNo() == 0
// 800DCFB8: join step 2 + mMyKind = kind (0 connect, 1 disconnect, 2 continue,
// 3 interrupt: dNetRequest_c::execute save kinds)
BOOL sendNetRequest(u8 arg);
// 800DD044: sendNetRequest(0): connect save (requestSaveConnectNetHst / Vst);
// gatekeeper
BOOL requestConnect();
// 800DD04C: 1 accepted (join step 1), -1 refused / host lost, 0 pending;
// gatekeeper, d_sv_demo
int getRequestResult();
// 800DD0F8: sendNetRequest(2): requestSaveContinueNetVst; d_sv_demo
BOOL requestContinue();
// 800DD100: sendNetRequest(3): requestSaveInterruptNetVst; d_sv_demo
BOOL requestInterrupt();
// 800DD108: sendNetRequest(1): the disconnect sequence; gatekeeper
BOOL requestDisconnect();
// 800DD110: dNetTransfer_c::start + mJoinStarted (d_net_mgr flag cleared on the
// next scene); gatekeeper
void startJoinTransfer();
// 800DD170: save scene, requestSaveContinueNetHst / InterruptNetHst,
// dNetSaveEnd_c::start(interrupt); d_sv_demo
void startSaveEnd(BOOL interrupt);
// 800DD204: dNetDisconnect_c::start(FALSE): this console disconnects;
// gatekeeper
void startDisconnect();
// 800DD25C: dNetTransfer_c::isAllReady (every accepted member's player and Mii
// received); gatekeeper
BOOL isJoinPlayersReady();
// 800DD280: dNetTransfer_c::mItemsRecv (set when the last NET_DIRECT_XFER_ITEMS
// part arrived); gatekeeper
u8 isItemsReceived();
// 800DD2A4: request member valid, its Mii received, transfer past its start
// steps; d_sv_mgr waitNewPlayer
BOOL isNewPlayerReady();
// 800DD338: joining console: transfer step > 2 (own player sent); gatekeeper
BOOL isJoinPlayerSent();
BOOL isJoinTransferDone(); // 800DD370: transfer step 15; gatekeeper
// 800DD3A0: dNetTransfer_c::mTownReqSend: NET_DIRECT_XFER_TOWN_REQUEST to the
// host (its mTownReqRecv -> sendTown); gatekeeper
BOOL requestTown();
// 800DD3D4: dNetJoinDone_c::isAllDone (every present member finished its join);
// gatekeeper
BOOL isJoinDoneAll();
// 800DD3F8: dNetJoinDone_c::getAllDone (NET_DIRECT_JOIN_DONE_HOST /
// NET_PACKET_JOIN_DONE_HOST from the host); gatekeeper
u8 getJoinAllDone();
void resetJoinDone(); // 800DD41C: dNetJoinDone_c::init; gatekeeper
// 800DD440: dNetTransfer_c::mFinish: adds the joiner to mMembers / its scene
// (NET_PACKET_JOIN_DONE / _HOST, NET_DIRECT_JOIN_DONE); gatekeeper
BOOL finishJoin();
BOOL isJoinFinished(); // 800DD474: mFinish == 0; gatekeeper
// 800DD4A0: dNetRequest_c::mGrantedMember.clear() (the member of the accepted
// request); gatekeeper, d_sv_mgr
void clearRequestMember();
void beginPacket(); // 800DD4C8: dNetPacket_c::begin unless resetting
// 800DD518: dNetPacket_c::add unless resetting
void addPacket(const void *data, u32 size);
// 800DD588: dNetPacket_c::send(id, member) unless resetting
void sendPacket(int id, int member);
// 800DD5F8: dNetPacket_c::setRecord: stores shared record id and marks it; the
// send stage sends it
void setRecord(int id, void *data, int arg);
void *getRecord(int id);   // 800DD64C: shared record id
u32 getRecordSize(int id); // 800DD680: size of shared record id
// 800DD6B4: dNetScene_c::getState; d_scene, d_scene_base
int getSceneChangeState();
// 800DD6D8: dNetScene_c::setState; d_scene, d_scene_base
void setSceneChangeState(int state);
// 800DD70C: dNetScene_c::enterLeaderScene: own scene = the scene leader's,
// owner if first, NET_PACKET_SCENE_STATE; d_scene_base
void enterLeaderScene();
// 800DD75C: g_dNet.mSavedSlot: own town player number (dPlayerMgr::getCurrentPlayerSlot) saved at
// session start, 7 none; d_nand, d_reset_save
s32 getLocalPlayerNo();
// 800DD76C: mPacket.mFrame: cCounter_c::m_gameFrame of the last send stage;
// d_a_player
u32 getNetFrame();
// 800DD790: dNetSceneClose_c::write(arg, own scene); d_scene_base
void writeSceneClose(int arg);
// 800DD83C: dNetSceneLeave_c::write(arg, own scene); d_scene_base
void writeSceneLeave(int arg);
// 800DD8E8: dNetSceneHandover_c::write(own scene); d_scene_base
void writeSceneHandover();
// 800DD960: TRUE offline / unjoined; else whether this console owns (runs) its
// scene (FALSE without a scene). Fish / insect / NPC spawners
BOOL isSceneOwner();
int countMembersInScene(u8 scene); // 800DD9E0: dNetScene_c::countMembers(scene)
bool isAllInScene(u8 scene);       // 800DDA14: dNetScene_c::isAllIn(scene)
// 800DDA48: dNetScene_c::getScene(member); npc_out / npc_sp_racco RELs
u8 getMemberScene(int member);
// 800DDA7C: dNetSaveEnd_c::mSaved.isAll; d_sv_mgr
// stepSaveContinueNetHst_c::waitAllMachinesSaved
bool isSaveEndAllSaved();
// 800DDAA4: dNetSaveEnd_c::mSaved.isOn(own) (NET_PACKET_SAVE_DONE_ALL);
// d_sv_mgr stepSaveContinueNetVst_c::waitSaveDemoEnd
bool isSaveEndSaved();
void resetSaveEnd(); // 800DDAE4: dNetSaveEnd_c::init; d_sv_mgr
// 800DDB08: all in dScene::SCENE_DM_SAVE and no scene leader; d_sv_mgr
// stepSaveConnectNetHst / Vst_c::waitSavePermission
BOOL isConnectSavePermitted();
// 800DDB58: the same test; d_sv_mgr stepSaveContinueNetHst / Vst_c::waitCanSave
BOOL canSaveContinue();
// 800DDBA8: !mActive; d_sv_mgr stepSaveInterruptNetHst / Vst_c::waitCanSave
BOOL canSaveInterrupt();
// 800DDBD0: disconnect seq step > 3 (leaving console) / > 7 (others);
// gatekeeper
BOOL isDisconnectStarted();
// 800DDC50: host: dNetDisconnect_c::mReady.on(0); guest:
// NET_PACKET_DISCONNECT_READY to the host; gatekeeper
void setDisconnectReady();
// 800DDC9C: host: dNetDisconnect_c::mReady all set; guest: own bit
// (NET_PACKET_DISCONNECT_GO); gatekeeper
BOOL isDisconnectReady();
// 800DDD18: host: NET_PACKET_DISCONNECT_GO to every member; gatekeeper
void sendDisconnectGo();
// 800DDD50: mErrorReported, 0 while resetting; d_wifi_err
u32 getNetReportedError();
// 800DDDAC: mTime[member]; d_sv_mgr findFriend, friend menus, gatekeeper
s64 getMemberTime(int member);
// 800DDDC8: mTime[member] unless resetting
void setMemberTime(int member, s64 time);
// 800DDE4C: dNetRequest_c::mLeave: the post stage frees the request lock (host
// mBusy.off(0) / NET_PACKET_JOIN_RELEASE); d_sv_mgr, gatekeeper
void releaseRequest();
// 800DDF54: dNetFlag_c::mStarted (openNetConnection .. closeNetConnection);
// d_caution
BOOL isNetMgrStarted();
// 800DDF78: dNetRequest_c::mBusy empty; dNetMgr_c::isRequestIdle (tail call)
BOOL isRequestIdle();

// Shared record copy helpers (d_net_sync.inc).
// 800E1C6C: copy helper of the NET_REC_MEMBER_A copy handlers (4 + 4 bytes)
void copyRecMemberA(void *dst, const void *src);
// 800E1CC8: copy helper of the NET_REC_MEMBER_B copy handlers (2 bytes)
void copyRecMemberB(void *dst, const void *src);
// 800E1CE0: copy helper of the NET_REC_MEMBER_C copy handlers: memCpy(dst, src,
// getRecordSize(id))
void copyRecBySize(void *dst, const void *src, int id);

// Session event hooks (d_net_sync.inc): (member, the mask of the other
// members). 800E3708: hook from startNetHostSession: fgMngSync_reset,
// fgMngTask_resetAll
void onSessionStart();
// 800E372C: hook from terminateNetSession: fgMngSync_reset, fgMngTask_resetAll
void onSessionEnd();
// 800E3750: hook: demo sync reset, dSvMgr_c::mCurrentVisitor = none, room maps
// cleared
void onVisitorClear(int member, const dNetMemberMask_c *mask);
// 800E377C: hook: demo sync reset, dSvMgr_c::mCurrentVisitor = member, room
// maps cleared
void onVisitorSet(int member, const dNetMemberMask_c *mask);
// 800E37B0: hook: TRUE
BOOL canFinishLeave(int member, const dNetMemberMask_c *mask);
// 800E37B8: hook: demo sync reset, room maps + footmarks cleared (leaving
// console / every member at a session end)
void onLeave(int member, const dNetMemberMask_c *mask);
// 800E37E4: hook: as onLeave, the host also drops item 0xFFF1 for the member,
// clears its NET_REC_MEMBER_A / B / C records
void onMemberLeft(int member, const dNetMemberMask_c *mask);
// 800E3858: hook (empty) after the connection is closed
void onDisconnect(int member, const dNetMemberMask_c *mask);
// 800E385C: hook: dWifi::updateSave() != 0
BOOL isDisconnectDone(int member, const dNetMemberMask_c *mask);
// 800E3888: hook (empty)
void onMemberLeaving(int member, const dNetMemberMask_c *mask);
// 800E388C: hook: dWifi::updateSave() != 0
BOOL isMemberLeaveDone(int member, const dNetMemberMask_c *mask);

// Placeholders (meaning unclear).
bool fn_800DD818(); // 800DD818: dNetScene_c::getLeaveAcked
bool fn_800DD8C4(); // 800DD8C4: dNetScene_c::isHandoverAcked
void fn_800DDE7C(); // 800DDE7C: dNetMgr_c::setBusy
// 800DDE80: scene start: dNetMgr_c::clearBusy after the gate
// scenes, the save scene or a join transfer
void fn_800DDE80();

// Handlers of d_net_sync.inc used by the main part (the dNetSyncBuf_c tables).
void fillSyncBuf0_0(int member, u8 *data, u32 size);
void fillSyncBuf0_1(int member, u8 *data, u32 size);
void applySyncBuf0_0(int member, u8 *data, u32 size);
void applySyncBuf0_1(int member, u8 *data, u32 size);
BOOL packSyncBuf8None(int member, const dNetMemberMask_c *mask, u8 *dst,
                      u32 size);
BOOL packSyncBuf8Host(int member, const dNetMemberMask_c *mask, u8 *dst,
                      u32 size);
BOOL packSyncBuf8Animal(int member, const dNetMemberMask_c *mask, u8 *dst,
                        u32 size);
void unpackSyncBuf8None(const dNetMemberMask_c &mask, const u8 *data, u32 size);
void unpackSyncBuf8Host(const dNetMemberMask_c &mask, const u8 *data, u32 size);
void unpackSyncBuf8Animal(const dNetMemberMask_c &mask, const u8 *data,
                          u32 size);
void fillSyncBuf4_0(int member, u8 *data, u32 size);
void fillSyncBuf4LogFlag(int member, u8 *data, u32 size);
void applySyncBuf4_0(int member, u8 *data, u32 size);
void applySyncBuf4LogFlag(int member, u8 *data, u32 size);

} // namespace dNet

// --------------------------------------------------------------------------------------------------
// Shared records (agent E): ids 0..NET_REC_NUM-1, set with setRecord, read with
// getRecord. The record buffer (dNetPacket_c::mRecordBuf) holds them packed in
// id order (lbl_80474620: sizes, lbl_80474908: offsets). Each has a copy
// handler (lbl_80474050: fills the record from the data given to setRecord) and
// may have a clear handler (lbl_80474338: its empty value, set by clearRecord
// after it zeroed the record). The handlers and these four tables are in
// d_net_sync.inc.
#define NET_REC_NUM 0xBA
#define NET_REC_MEMBER_A 4  // 4..7: one per member (8 bytes)
#define NET_REC_MEMBER_B 8  // 8..11: one per member (2 bytes)
#define NET_REC_MEMBER_C 12 // 12..15: one per member (15 bytes)
// 0x10..0x1B: the insect net slots (dInsectCatchData_c, INSECT_NET_SLOT_NUM)
// 0x1C..0x23: the fish field records (PLAY_SYNC_REC_ID, dPlaySyncRec_c)
#define NET_REC_INSECT 0x10
// 0x2B..0x95: the npc daubs (dNpcDaub_c): 10 normal ones, then the specials
#define NET_REC_DAUB 0x2B
#define NET_REC_DAUB_SPECIAL 0x35
// 0x96..0xB9: player house rooms, house * 9 + room * 3 + 0..2 (8 bytes)
#define NET_REC_HOUSE 0x96

typedef void (*dNetCopyFunc)(void *dst, const void *src, int arg);
typedef void (*dNetClearFunc)(void *rec);

extern const dNetCopyFunc lbl_80474050[NET_REC_NUM]; // copy handlers
// clear handlers (NULL: none)
extern const dNetClearFunc lbl_80474338[NET_REC_NUM];
extern const u32 lbl_80474620[NET_REC_NUM]; // sizes
// offsets in the record buffer
extern const u32 lbl_80474908[NET_REC_NUM];

// Data channel handlers (d_net_sync.inc, agent E) for the B tables lbl_80474BF0
// .. 807508F8: per entry a value, a send handler that writes its data to buf
// and returns its size (< 0: nothing this time) and a receive handler that
// applies it (data NULL, size 0 for an empty entry).
typedef int (*dNetSendFunc)(int arg, void *buf, int size, u8 scene);
typedef void (*dNetApplyFunc)(int arg, const void *data, u32 size, u8 scene);
// lbl_80474BFC
typedef int (*dNetSendFuncB)(int arg, void *buf, int size, int value);
// lbl_80474C08
typedef void (*dNetApplyFuncB)(int arg, const void *data, u32 size, int value);
// 807508F0
typedef int (*dNetMemberSendFunc)(int member, int myNo, void *buf, int size,
                                  u8 scene);
// 807508F8
typedef void (*dNetMemberApplyFunc)(int member, int myNo, const void *data,
                                    u32 size, u8 scene);

namespace dNet {

// lbl_80474BF0 = {1, 1, 1}: lbl_80474BFC / lbl_80474C08.
// 800E38B8: -1
int sendClose0(int arg, void *buf, int size, int value);
// 800E38C0: the pair ring update, -1
int sendClosePairRing(int arg, void *buf, int size, int value);
// 800E3924: fn_800F55BC
int sendClose2(int arg, void *buf, int size, int value);
// 800E3928: nothing
void applyClose0(int arg, const void *data, u32 size, int value);
// 800E392C: nothing
void applyClose1(int arg, const void *data, u32 size, int value);
// 800E3930: nothing
void applyClose2(int arg, const void *data, u32 size, int value);
// lbl_80474C14 = {1, 0, 0, 0, 0}: lbl_80474C28 / lbl_80474C3C.
// 800E3934: -1
int sendOpen0(int arg, void *buf, int size, u8 scene);
// 800E393C: the town: resets the insect records
int sendOpenInsects(int arg, void *buf, int size, u8 scene);
// 800E39C4: the town: resets the fish records
int sendOpenFish(int arg, void *buf, int size, u8 scene);
// 800E3A68: a house room: resets its records
int sendOpenHouseRoom(int arg, void *buf, int size, u8 scene);
// 800E3B30: 0
int sendOpen4(int arg, void *buf, int size, u8 scene);
// 800E3B38: nothing
void applyOpen0(int arg, const void *data, u32 size, u8 scene);
// 800E3B3C: insects respawn
void applyOpenInsects(int arg, const void *data, u32 size, u8 scene);
// 800E3B48: fish respawn
void applyOpenFish(int arg, const void *data, u32 size, u8 scene);
// 800E3B54: flags the house room
void applyOpenHouseRoom(int arg, const void *data, u32 size, u8 scene);
// 800E3BC4: fn_800F55B4
void applyOpen4(int arg, const void *data, u32 size, u8 scene);
// 807508CC = {1}: 807508D0 / 807508D4; 807508D8 = {1}: 807508DC / 807508E0.
// 800E3BC8: -1
int sendLeave0(int arg, void *buf, int size, u8 scene);
// 800E3BD0: nothing
void applyLeave0(int arg, const void *data, u32 size, u8 scene);
// 800E3BD4: -1
int sendEnter0(int arg, void *buf, int size, u8 scene);
// 800E3BDC: nothing
void applyEnter0(int arg, const void *data, u32 size, u8 scene);
// 807508E8 = {1, 1}: 807508F0 / 807508F8.
// 800E3BE0: -1
int sendHandover0(int member, int myNo, void *buf, int size, u8 scene);
// 800E3BE8: a TRUE byte
int sendHandoverFlag(int member, int myNo, void *buf, int size, u8 scene);
// 800E3C20: nothing
void applyHandover0(int member, int myNo, const void *data, u32 size, u8 scene);
// 800E3C24: reads the byte
void applyHandoverFlag(int member, int myNo, const void *data, u32 size,
                       u8 scene);

} // namespace dNet

// --------------------------------------------------------------------------------------------------
// The packet handler tables (agent D, end of d_net_sync.inc's [D] part).

// lbl_80473E10
typedef void (*dNetRecvFunc)(int member, const u8 *data, u32 size);
// lbl_80473E44
typedef void (*dNetDataFunc)(int member, const u8 *data, u32 size, u32 arg,
                             u8 scene);

// per lbl_80473E10 handler: getDirectSeqMode
extern const s8 lbl_80473E00[NET_DIRECT_NUM];
// direct packet handlers (callDirectHandler)
extern const dNetRecvFunc lbl_80473E10[NET_DIRECT_NUM];
// receive handlers by packet id (callBlockHandlers)
extern const dNetDataFunc lbl_80473E44[NET_PACKET_NUM];
