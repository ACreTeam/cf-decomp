#pragma once

// Furniture (TU d_ftr.cpp): the furniture actors' shared state, room unit map and online sync.

#include <types.h>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

namespace dItem {
struct BITM;
}
class daFTR_c;

namespace dFTR {

// The furniture interface of daFTR_c (at +0x158).
class alwaysAc_c {
public:
    virtual void vf08() = 0;
    virtual int sendEvent(int event) = 0;
    virtual void vf10() = 0;
    virtual f32 getTopY() = 0;
    virtual BOOL isPosTop() = 0;
    virtual BOOL setSwitch(int sw, int param) = 0;
    virtual int getSwitchBit(int sw) = 0;
    virtual BOOL isSyncDisabled() = 0;
    virtual int getSwitchParam() = 0;
    virtual void vf2C() = 0;
    virtual BOOL isSwitchOn() = 0;
    virtual void vf34() = 0;
    virtual int getLayer() = 0;
    virtual BOOL push(const mAng &angle) = 0;
    virtual BOOL move(const mVec3_c *dir) = 0;
    virtual BOOL pull(const mAng &angle) = 0;
    virtual BOOL rotateLeft(const mAng &angle) = 0;
    virtual BOOL rotateRight(const mAng &angle) = 0;
    virtual const dItem::BITM *getBITM() = 0;
    virtual BOOL startHold() = 0;
    virtual dItem::Item getItem() = 0;
    virtual BOOL getUnit(int *x, int *z, int layer, s16 *angle) = 0;
    virtual BOOL isMoving() = 0;
    virtual void vf64() = 0;
    virtual BOOL canSetSwitch() = 0;
    virtual int isMusic() = 0;
    virtual BOOL isLamp() = 0;
    virtual BOOL isSwitchable() { return FALSE; }
    virtual void vf78() = 0;
};

class alwaysBank_c : public dDvd::arcBank_c {
public:
    alwaysBank_c() {}
    virtual void onLoaded();

    BOOL load(void *heap);
    void *getFile(u32 idx);

    static alwaysBank_c *get();
}; // size 0x74

// The units a furniture covers by size (BITM m_ftrSize) and rotation (the item id's low 2 bits).
struct shape_c {
    shape_c *set(dItem::Item item);
    u32 getNum() const;
    const int *getOfs(u32 i) const;
    void getCenterPos(mVec3_c *pos, int x, int z);

    /* 0x0 */ s32 mSize;
    /* 0x4 */ s32 mRot;
}; // size 0x8

// A member's furniture hold (online, no two members hold furniture close together).
struct lock_c {
    ~lock_c() {}
    lock_c();

    void clear();
    BOOL set(u8 scene, const mVec3_c &pos);
    BOOL reset();

    /* 0x00 */ u8 mActive;
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ u8 mScene;
}; // size 0x14

class lockList_c {
public:
    ~lockList_c();
    void clear();
    BOOL set(int member, u8 scene, const mVec3_c &pos);
    BOOL reset(int member);
    BOOL check(u32 member, u8 scene, const mVec3_c &pos);
    BOOL lock(int member, u8 scene, const mVec3_c &pos);

    /* 0x00 */ lock_c mLock[4];
}; // size 0x50

// The furniture handle (list_c index) on each unit of the room, by layer.
class unitMap_c {
public:
    void clear();
    BOOL set(daFTR_c *ftr, int x, int z, int layer);
    BOOL remove(daFTR_c *ftr, int x, int z, int layer);
    int getHandle(int x, int z, int layer);
    daFTR_c *get(int x, int z, int layer);
    daFTR_c *get(const mVec3_c *pos, int layer);

    /* 0x000 */ u8 mHandle[2][16][16];
}; // size 0x200

// The furniture actors of the room (up to dScene::getSceneParamA of the scene).
class list_c {
public:
    list_c() { clear(); }

    void clear();
    int add(daFTR_c *ftr);
    BOOL remove(daFTR_c *ftr);
    u32 getNum();
    u32 getFreeNum();
    int find(daFTR_c *ftr);
    daFTR_c *get(u32 handle);

    /* 0x000 */ daFTR_c *mFtr[64];
}; // size 0x100

// A furniture the player moves.
struct hold_c {
    hold_c();

    void clear();
    void set(int handle, const mVec3_c &a, const mVec3_c &b, f32 c, const mVec3_c &d, const mVec3_c &e, s16 angle,
             u8 flag);

    /* 0x00 */ int mHandle;
    /* 0x04 */ mVec3_c _04;
    /* 0x10 */ mVec3_c _10;
    /* 0x1C */ f32 _1C;
    /* 0x20 */ mVec3_c _20;
    /* 0x2C */ mVec3_c _2C;
    /* 0x38 */ s16 mAngle;
    /* 0x3A */ u8 _3A;
}; // size 0x3C

class holdList_c {
public:
    holdList_c() { clear(); }

    hold_c *get(int layer);
    hold_c *find(daFTR_c *ftr);
    daFTR_c *getFtr(int layer);
    void startHold();
    BOOL isMoving();
    BOOL push(s16 angle);
    BOOL pull(s16 angle);
    BOOL pushLeft();
    BOOL pullLeft();
    BOOL pushRight();
    BOOL pullRight();
    BOOL move(s16 angle);
    mVec3_c *getDir(s16 angle);
    BOOL moveForward();
    BOOL rotateLeft();
    BOOL moveBack();
    BOOL rotateRight();
    void clear();
    BOOL set(int handle, f32 c, const mVec3_c &a, const mVec3_c &b, const mVec3_c &d, const mVec3_c &e, s16 angle,
             u8 flag);
    static BOOL isHoldScene();

    /* 0x00 */ hold_c mHold[2];
}; // size 0x78

enum {
    HOLD_STATE_PENDING,
    HOLD_STATE_ACCEPTED,
    HOLD_STATE_REJECTED,
};

typedef BOOL (*filter_t)(daFTR_c *ftr);

unitMap_c *getUnitMap();
list_c *getList();
void recvHoldRequest(const void *data, int member);
void recvHoldAnswer(BOOL ok);
void recvHoldRelease(int member, int arg);
BOOL isLamp(daFTR_c *ftr);
void setOwnerSwitch(u8 scene, int x, int z, u8 layer, BOOL arg);
void recvOwnerSwitch(const void *data);
void sendOwnerSwitch(u8 scene, int x, int z, u8 layer);
void sendOwnerSwitch(u8 scene, const mVec3_c *pos, u8 layer);
BOOL applySwitch(u8 scene, int x, int z, int layer, int sw, BOOL on, BOOL music, BOOL arg, u8 param);
BOOL sendSwitch(int x, int z, int layer, int sw, u8 param, BOOL arg, int member);
BOOL setSwitch(daFTR_c *ftr, int sw, int param, BOOL arg, BOOL send);
BOOL setSwitch(int x, int z, u8 layer, int sw, int param, BOOL arg, BOOL send);
BOOL requestSwitch(daFTR_c *ftr, int sw, u8 param, BOOL arg, BOOL send);
BOOL requestSwitch(int x, int z, u8 layer, int sw, u8 param, BOOL arg, BOOL send);
void recvSwitch(const void *data);
void sendRoomFlag(u8 scene, BOOL on, int member);
BOOL setRoomFlag(u8 scene, u8 on);
void setRoomFlag(u8 scene, u8 on, BOOL send);
void recvRoomFlag(const void *data);
void setUnitItem(u8 scene, int x, int z, dItem::Item *item, BOOL send);
void recvUnitItem(u8 scene, const void *data);
void putFtr(u8 scene, int x, int z, int layer, BOOL on, BOOL gyroid, int gyroidValue, const dItem::Item *item,
            u8 send);
void recvPutFtr(u8 scene, const void *data);
void removeFtr(u8 scene, int x, int z, int layer, BOOL on, const dItem::Item *item, BOOL upper, BOOL send);
void recvRemoveFtr(u8 scene, const void *data);
f32 getGroundY(const mVec3_c *pos);
f32 getGroundY(int x, int z);
f32 getRoomGroundY(int x, int z);
BOOL isEmptyOnTop(int x, int z);
holdList_c *getHoldList();
BOOL canHoldInScene();
void resetHold();
void requestHold(const mVec3_c *pos);
int getHoldState();
void releaseHold();
BOOL canNpcWalk(int x, int z);
BOOL isHoldDisabledScene();
void clearHomeFlags();
void clearHomeFlag();
void setSwitchAll(u32 sw, filter_t filter, int arg);
u32 countSwitch(filter_t filter, u32 sw);
daFTR_c *switchOffRandom(filter_t filter, int arg);
BOOL switchOffAll();
BOOL playMusic(const dItem::Item *item);
int sendEvent(int handle);
int sendSellEvent(int handle);
void disableHold();
void disableHoldOnly();
void enableHold();
BOOL isHoldDisabled();
BOOL isHoldDisabledOnly();
BOOL canWalk(int x, int z);
BOOL isExit(int x, int z);
BOOL canReachExit();

} // namespace dFTR

// The furniture actor (d_a_ftr / d_a_ftr_act).
class daFTR_c : public dDemoActor_c, public dBGCF::mvbg_c, public dFTR::alwaysAc_c, public dMsg::Rcpt_c {
public:
    virtual void vf188();
    virtual BOOL canWalkOn();
    virtual void vf190();
    virtual void vf194();
    virtual int getTopKind();
    virtual void vf19C();
    virtual void vf1A0();
    virtual void vf1A4();
    virtual void vf1A8();
    virtual void onSwitchSent();
    virtual u32 getSwitch();
    virtual void vf1B4();
    virtual void vf1B8();
    virtual BOOL canSwitch();
    virtual void vf1C0();
    virtual void changeSwitch(u32 sw, int arg);
};
