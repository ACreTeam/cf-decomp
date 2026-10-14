#pragma once

// Helpers shared by the insect actors (d_insect_field, d_insect_museum, d_a_cockroach).

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <revolution/MTX.h>

class dDemoActor_c;

namespace dInsectUtil {

#define INSECT_UTIL_PLAYER_NUM 4

class scare_c {
public:
    void init();
    void clear();
    void setLimit(int limit);
    void setEscapeLimit(int limit);
    void add(f32 speed, f32 rate);
    bool isScared() const;
    bool isEscaping() const;
    bool hasEscapeLimit() const;
    f32 getRate() const;

    /* 0x0 */ f32 mValue;
    /* 0x4 */ f32 mLimit;
    /* 0x8 */ f32 mEscapeLimit;
};

typedef bool (*itemCheckFunc)(dItem::Item *item, const mVec3_c *pos);

class itemSearch_c {
public:
    itemSearch_c();
    itemSearch_c(itemCheckFunc check);
    ~itemSearch_c();
    void search(const mVec3_c &center, f32 radius);
    bool isFound() const;
    void getPos(mVec3_c *out) const;
    int getWidth(f32 radius) const;

    /* 0x00 */ itemCheckFunc mCheck;
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ dItem::Item mItem;
};

class siteSearch_c {
public:
    siteSearch_c();
    ~siteSearch_c();
    void search(mVec3_c &center, f32 radius);
    bool isFound() const;
    void setPos(mVec3_c &pos);
    f32 getRadius() const;
    int getWidth(f32 radius) const;

    /* 0x0 */ mVec3_c mPos;
};

class playerCheck_c {
public:
    playerCheck_c();
    ~playerCheck_c();
    bool isValid(int player) const;
    void setPos(const mVec3_c &pos);
    BOOL isNetPlayer(int player) const;
    bool isNear(int player, f32 radius) const;
    bool isShaking(const mVec3_c &pos, int player) const;
    bool isShakingNear(const mVec3_c &pos, int player) const;
    bool isDigging(const mVec3_c &pos) const;
    bool isHittingStone(const mVec3_c &pos) const;
    bool isDisturbing(f32 radius) const;
    void getPlayerPos(mVec3_c *out, int player) const;
    f32 getPlayerSpeed(int player) const;
    f32 getDistance(int player) const;
    mAng getPlayerAngle(int player) const;
    mAng getAngleTo(int player) const;
    int getNearest() const;
    int getFastest(f32 radius) const;
    int getStoneHitter(const mVec3_c &pos) const;
    bool isOnUnit(int unitX, int unitZ, int player) const;

    /* 0x0 */ mVec3_c mPos;
};

class npcCheck_c {
public:
    npcCheck_c();
    ~npcCheck_c();
    void setPos(const mVec3_c &pos);
    bool isNear(f32 radius) const;
    void getNpcPos(mVec3_c *out) const;
    f32 getNpcSpeed() const;
    f32 getDistance() const;
    mAng getAngleTo() const;
    void findNearest();

    /* 0x00 */ dDemoActor_c *mNpc;
    /* 0x04 */ mVec3_c mPos;
};

class axis_c {
public:
    axis_c();
    ~axis_c();
    void setIdentity();
    void normalize();
    void setUp(const mVec3_c &up);
    void setFront(const mVec3_c &front);
    void rotateX(mAng angle);
    void rotateY(mAng angle);
    void rotateZ(mAng angle);
    void set(const Mtx mtx);
    mAng getAngleX() const;
    mAng getAngleY() const;
    mAng getAngleZ() const;
    mVec3_c getFront() const;

    /* 0x00 */ mVec3_c mSide;
    /* 0x0C */ mVec3_c mUp;
    /* 0x18 */ mVec3_c mFront;
};

bool canPass(const mVec3_c *start, const mVec3_c *end);
void getMtx(Mtx out, const axis_c &axis);
int secToFrame(f32 sec);
BOOL getGroundMovePos(mVec3_c *out, mVec3_c &from, const mVec3_c &to, u32 steps, BOOL noStoneBus, BOOL noSea,
                      f32 minDy, f32 maxDy);
void getLandPos(mVec3_c *out, const mVec3_c &pos, u32 mode, BOOL noStoneBus);
BOOL isExcludedFlower(dItem::Item *item);
BOOL isRaining();
BOOL isSnowing();
void moveTo(mVec3_c *pos, mAng *angle, const mVec3_c &target, f32 speed, f32 turnDeg);
void easeMove(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 total, int frame);
void easeMoveXZ(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 total, int frame);
void flyTo(mVec3_c *pos, axis_c *axis, const mVec3_c &target, const mVec3_c &up, f32 speed, f32 turnRate);
BOOL getWaterMovePos(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 steps);
void getWaterPos(mVec3_c *out, const mVec3_c &pos);
dItem::Item *getItem(const mVec3_c *pos);
int getDecayFrames(f32 rate);
bool isColumn(int unitX, int unitZ);
bool isColumn(mVec3_c pos);
u32 isStructure(mVec3_c pos);
bool isSameUnit(const mVec3_c &a, const mVec3_c &b);
u8 getRandom(u8 min, u8 max);

} // namespace dInsectUtil
