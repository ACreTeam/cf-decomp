#pragma once

// The cockroaches of a player's house room (REL d_cockroach_managerNP, profile COCKROACH_MANAGER, made by
// dFtrm_c::createCockroach). The house room's flags (dHomeRoom_c) count the cockroaches left; the manager spawns
// them (daCockroach_c, d_a_cockroachNP) and keeps one record per cockroach of every room, shared over the net.

#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/mLib/m_allocator.hpp>

class dHomeRoom_c;
class daCockroachBase_c;

// The cockroaches of a room.
#define COCKROACH_ROOM_NUM 3
// The houses' rooms, each with COCKROACH_ROOM_NUM records.
#define COCKROACH_HOUSE_ROOM_NUM 3
// The records of a house.
#define COCKROACH_HOUSE_RECORD_NUM (COCKROACH_HOUSE_ROOM_NUM * COCKROACH_ROOM_NUM)
// One record per cockroach of every room of the 4 houses.
#define COCKROACH_RECORD_NUM 36

// A cockroach's state, packed into 5 bytes (bit 0 is the low bit of byte 0): the record index (6 bits,
// RECORD_IDX_NONE when unused), the daCockroach_c state (3 bits), the position on the room floor (2 x 12 bits) and
// the player that killed it (3 bits, PLAYER_NONE while alive).
class dCockroachRecord_c {
public:
    enum {
        RECORD_IDX_NONE = 0x3F,
        PLAYER_NONE = 4,
    };

    dCockroachRecord_c() {
        mData[0] = 0;
        mData[1] = 0;
        mData[2] = 0;
        mData[3] = 0;
        mData[4] = 0;
    }

    u8 getIdx() const { return mData[0] & 0x3F; }
    u8 getState() const { return ((mData[0] >> 6) & 3) + ((mData[1] & 1) << 2); }
    u16 getX() const { return ((mData[1] >> 1) & 0x7F) + ((mData[2] & 0x1F) << 7); }
    u16 getZ() const { return ((mData[2] >> 5) & 7) + (mData[3] << 3) + ((mData[4] & 1) << 11); }
    u8 getPlayer() const { return (mData[4] >> 1) & 7; }

    void setIdx(u8 idx) { mData[0] = (idx & 0x3F) | (mData[0] & 0xC0); }
    void setState(int state) {
        mData[0] = ((state & 3) << 6) | (mData[0] & 0x3F);
        mData[1] = ((state >> 2) & 1) | (mData[1] & 0xFE);
    }
    void setX(int x) {
        mData[1] = ((x & 0x7F) << 1) | (mData[1] & 1);
        mData[2] = (mData[2] & ~0x1F) | ((x >> 7) & 0x1F);
    }
    void setZ(int z) {
        mData[2] = ((z & 7) << 5) | (mData[2] & 0x1F);
        mData[3] = (mData[3] & ~0xFF) | ((z >> 3) & 0xFF);
        mData[4] = ((z >> 11) & 1) | (mData[4] & 0xFE);
    }
    void setPlayer(int player) { mData[4] = (mData[4] & ~0xE) | ((player & 7) << 1); }

    /* 0x0 */ u8 mData[5];
}; // size 0x5

// The records of all rooms (net records NET_REC_HOUSE + index). The scene owner sends its room's records, the
// other consoles take them.
class dCockroachRecordList_c {
public:
    dCockroachRecordList_c();
    ~dCockroachRecordList_c();

    int create();
    int execute();
    int doDelete();
    void set(const dCockroachRecord_c *record);
    dCockroachRecord_c *get(u8 idx);
    void clear();
    void send();
    void receive();

    /* 0x00 */ dCockroachRecord_c mRecords[COCKROACH_RECORD_NUM];
}; // size 0xB4

class dCockroachManager_c : public dDemoActor_c, public dMsg::Rcpt_c {
public:
    dCockroachManager_c();
    virtual ~dCockroachManager_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void onDemoCmd(int cmd, u8 member);

    mAllocator_c *getAllocator();
    daCockroachBase_c *createCockroach(const mVec3_c *pos, const mAng3_c *angle, u8 idx);
    void kill(u8 idx);
    BOOL hasCockroaches();
    BOOL isSpawned(); // BOOL: the player REL compares the result as a word (cmpwi)
    dCockroachRecord_c *getRecord(u8 idx);
    void setRecord(const dCockroachRecord_c *record);
    mVec3_c getAreaMin() const;
    mVec3_c getAreaMax() const;
    u32 isOutside(const mVec3_c &pos);
    u32 isOnFurniture(const mVec3_c &pos);
    void spawn();
    void spawnRemote();
    u8 getRecordIdx(u8 i);
    u8 makeKillPacket(int house, int room, int i);
    dHomeRoom_c *getRoom();
    bool startPlayerReaction();
    void executeTalk();

    static dCockroachManager_c *sInstance;

    /* 0x120 */ EGG::Heap *mpHeap;
    /* 0x124 */ mAllocator_c mAllocator;
    /* 0x140 */ int mHouse;
    /* 0x144 */ int mRoom;
    /* 0x148 */ daCockroachBase_c *mpCockroach[COCKROACH_ROOM_NUM];
    /* 0x154 */ dCockroachRecordList_c mRecords;
    /* 0x208 */ mVec3_c mAreaMin;
    /* 0x214 */ mVec3_c mAreaMax;
    /* 0x220 */ u8 mTalkRequested;
    /* 0x221 */ u8 mTalkEnded;
    /* 0x222 */ u8 mAllKilled;
    /* 0x223 */ u8 mSpawned;
    /* 0x224 */ int mTalkStep;
}; // size 0x228
