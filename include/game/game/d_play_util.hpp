#pragma once

#include <types.h>

// Play helpers next to the scene manager: weather time windows, actor / player list searches and
// the net sync of 8 shared records. Source: src/dol/game/d_play_util.cpp (.text
// 80160EA4..80161CC4). All names are inferred.

// An actor in dPlayActorMng_c's list.
class dPlayActor_c {
public:
    virtual void vf00();
    virtual void vf04();
    virtual void vf08();
    virtual void vf0C();
    virtual void vf10();
    virtual void vf14();
    virtual BOOL isActive(); // 0x20

    /* 0x004 */ u8 _004[0x1EC - 0x4];
    /* 0x1EC */ dPlayActor_c *mNext;
    /* 0x1F0 */ u8 _1F0[0x23B - 0x1F0];
    /* 0x23B */ u8 m23B; // players: not in the play
};

typedef BOOL (*dPlayActorFunc)(dPlayActor_c *actor, void *arg);

#define PLAY_PLAYER_NUM 8

// The actor manager (lbl_8074E838).
class dPlayActorMng_c {
public:
    BOOL findActorIf(dPlayActorFunc func, void *arg); // 801614F0: the first match
    BOOL forEachActiveActor(dPlayActorFunc func, void *arg); // 8016156C: TRUE if any matched
    BOOL findPlayerIf(dPlayActorFunc func, void *arg); // 8016160C

    /* 0x000 */ u8 _000[0x138];
    /* 0x138 */ dPlayActor_c *mPlayers[PLAY_PLAYER_NUM];
    /* 0x158 */ u8 _158[0x764 - 0x158];
    /* 0x764 */ dPlayActor_c *mActors;
};

#define PLAY_SYNC_REC_NUM 8
#define PLAY_SYNC_REC_ID 0x1C // the shared record id of record 0 (fn_800DD64C / fn_800DD5F8)

enum {
    PLAY_SYNC_STATE_NONE,
    PLAY_SYNC_STATE_CLAIMED,
};

// A shared record (8 bytes; the bits are packed from the low bit of byte 0 up).
struct dPlaySyncRec_c {
    u32 getA() const { return b0 & 0x7F; } // 7 bits
    u32 getC() const { return ((b0 >> 7) & 1) + (b1 & 0x3F) * 2; } // 7 bits
    int getMember() const { return b7 & 7; }
    void setMember(u32 member) { b7 = (b7 & 0xF8) | member; }
    void setState(u32 state) { b7 = (b7 & 0x3F) | (state << 6); }

    /* 0x0 */ u8 b0;
    /* 0x1 */ u8 b1;
    /* 0x2 */ u8 b2;
    /* 0x3 */ u8 b3;
    /* 0x4 */ u8 b4;
    /* 0x5 */ u8 b5;
    /* 0x6 */ u8 b6;
    /* 0x7 */ u8 b7; // the member (bits 0..2) and the state (bits 6..7)
}; // size 0x8

struct dPlaySyncRecBuf_c {
    dPlaySyncRecBuf_c() {
        mRec.b0 = 0;
        mRec.b1 = 0;
        mRec.b2 = 0;
        mRec.b3 = 0;
        mRec.b4 = 0;
        mRec.b5 = 0;
        mRec.b6 = 0;
        mRec.b7 = 0;
        _08 = 0;
        _09 = 0;
        _0C = -1;
    }

    /* 0x0 */ dPlaySyncRec_c mRec;
    /* 0x8 */ u8 _08;
    /* 0x9 */ u8 _09;
    /* 0xC */ s32 _0C;
}; // size 0x10

// The host's copy of the records (lbl_8074E840).
struct dPlaySyncHost_c {
    /* 0x000 */ u8 _000[0x178];
    /* 0x178 */ dPlaySyncRecBuf_c mRecs[PLAY_SYNC_REC_NUM];
};

// The sync message (net packet 0x41, 4 bytes; the bits are packed from the low bit of byte 0 up).
struct dPlaySyncMsg_c {
    dPlaySyncMsg_c() {
        b0 = 0;
        b1 = 0;
        mMember = 0;
        b2 = 0;
    }

    u32 getIdx() const { return b0 & 0xF; }
    u32 getA() const { return ((b0 >> 4) & 0xF) + (b1 & 7) * 0x10; }
    u32 getC() const { return ((b1 >> 3) & 0x1F) + (b2 & 0xF) * 0x20; }
    void setIdx(u32 idx) { b0 = (b0 & 0xF0) | (idx & 0xF); }
    void setA(u32 a) {
        b0 = (b0 & 0x0F) | ((a & 0xF) << 4);
        b1 = (b1 & 0xF8) | ((a >> 4) & 7);
    }
    void setC(u32 c) {
        b1 = (b1 & 0x07) | ((c & 0x1F) << 3);
        b2 = (b2 & 0xF0) | ((c >> 5) & 0xF);
    }
    void setType(u32 type) { b2 = (b2 & 0x0F) | (type << 4); }

    /* 0x0 */ u8 b0;
    /* 0x1 */ u8 b1;
    /* 0x2 */ u8 b2;
    /* 0x3 */ u8 mMember;
}; // size 0x4

extern void *lbl_8074E830; // the sky light
extern dPlayActorMng_c *lbl_8074E838;
extern dPlaySyncHost_c *lbl_8074E840;

// getWeatherPhaseA / getWeatherPhaseB results.
enum {
    PLAY_WEATHER_PHASE_PAST, // below the low mark
    PLAY_WEATHER_PHASE_SOON, // between the marks
    PLAY_WEATHER_PHASE_FAR, // above the high mark
    PLAY_WEATHER_PHASE_NONE, // out of season, or no window for the weather
};

BOOL calcWeatherHoursA(f32 *hours); // 80160EA4
BOOL calcWeatherHoursB(f32 *hours); // 801610A8
int getWeatherPhaseA(); // 801612D8: 3 if not in its season
int getWeatherPhaseB(); // 801613E4
void sendSyncRecClaim(dPlaySyncRec_c *rec, int idx); // 801616A4
void sendSyncRecRelease(const dPlaySyncRec_c *rec, int idx); // 8016177C
void claimSyncRec(dPlaySyncRec_c *rec, int idx); // 80161818
void clearSyncAcks(); // 80161874
void recvSyncRelease(const dPlaySyncMsg_c *msg, int member); // 80161900
void recvSyncAck(const dPlaySyncMsg_c *msg, int member); // 801619AC
void releaseSyncRec(int idx); // 80161B54
