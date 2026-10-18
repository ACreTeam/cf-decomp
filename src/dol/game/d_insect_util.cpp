// .text 800BDDE0..800C0F80, .sdata2 807507C8..80750828.
#include <game/game/d_insect_util.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_weather.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/egg/math/eggQuat.h>
#include <float.h>
#include <game/game/d_a_player.hpp>

#define UNIT_OF(v) ((int)(v) >> 5)

namespace dInsectUtil {

static inline void toUnit(int *x, int *z, f32 fx, f32 fz) {
    *x = UNIT_OF(fx);
    *z = UNIT_OF(fz);
}

static inline int toUnitPos(f32 v) {
    return UNIT_OF(v);
}

static inline void setQuatIdentity(EGG::Quatf *q) {
    mVec3_c zero(0.0f, 0.0f, 0.0f);
    dBalloonString::setQuaternion(q, zero, 1.0f);
}

bool canPass(const mVec3_c *start, const mVec3_c *end) {
    dBGCF::lineChk_s chk;
    mVec3_c to(*end);
    if (dBGCF::checkLine(&chk, &to, start, 0x33) == TRUE) {
        mVec3_c normal(chk.mNormal);
        mVec3_c up(0.0f, 1.0f, 0.0f);
        if (up.dot(normal) <= 0.1f) {
            return false;
        }
    }
    return true;
}

void scare_c::init() {
    clear();
    setLimit(0);
    setEscapeLimit(0);
}

void scare_c::clear() {
    mValue = 0.0f;
}

void scare_c::setLimit(int limit) {
    mLimit = limit;
}

void scare_c::setEscapeLimit(int limit) {
    mEscapeLimit = limit;
}

void scare_c::add(f32 speed, f32 rate) {
    f32 add;
    if (speed < 0.01f) {
        add = -1.0f;
    } else {
        add = 1.2f * (speed * speed);
        add *= rate;
    }
    mValue += add;
    mValue = mValue < 0.0f ? 0.0f : mValue;
    if (1000.0f <= mValue) {
        mValue = 1000.0f;
    }
}

bool scare_c::isScared() const {
    return mLimit <= mValue;
}

bool scare_c::isEscaping() const {
    if (hasEscapeLimit() == true && mEscapeLimit <= mValue) {
        return true;
    }
    return false;
}

bool scare_c::hasEscapeLimit() const {
    return (std::fabs(mEscapeLimit) <= FLT_EPSILON) != true;
}

f32 scare_c::getRate() const {
    f32 rate = mValue / mLimit;
    rate = rate > 1.0f ? 1.0f : rate;
    return rate < 0.0f ? 0.0f : rate;
}

itemSearch_c::itemSearch_c() : mCheck(NULL), mPos(0.0f, 0.0f, 0.0f) {}

itemSearch_c::itemSearch_c(itemCheckFunc check) : mCheck(check), mPos(0.0f, 0.0f, 0.0f) {}

itemSearch_c::~itemSearch_c() {}

void itemSearch_c::search(const mVec3_c &center, f32 radius) {
    mPos.set(0.0f, 0.0f, 0.0f);
    playerCheck_c player;
    player.setPos(center);
    int unitX = toUnitPos(center.x);
    int unitZ = toUnitPos(center.z);
    int width = getWidth(radius);
    int x0 = unitX - (width >> 1);
    int z0 = unitZ - (width >> 1);
    for (int x = x0; x < x0 + width; x++) {
        for (int z = z0; z < z0 + width; z++) {
            mVec3_c ground;
            dFdBase_c::getUnitGroundPos(ground, x, z);
            f32 dy = ground.y - center.y;
            if (-64.0f < dy && dy < 0.1f) {
                int blockX = x >> 4;
                int blockZ = z >> 4;
                int localX = x - blockX * 16;
                int localZ = z - blockZ * 16;
                dFdBlock_c *block = dField::getInfo(0)->getBlock(blockX, blockZ);
                if (block != NULL) {
                    dItem::Item *item = block->getItem(localX, localZ, 0);
                    if (item != NULL) {
                        mVec3_c pos;
                        dFdBase_c::getUnitCenterPos(pos, x, z);
                        if (!dField::getInfo(1)->isBuried(x, z) && mCheck(item, &pos) == true) {
                            mVec3_c diff = pos - center;
                            if (EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z) <= radius &&
                                !player.isOnUnit(x, z, player.getNearest())) {
                                pos.y = dBGCF::getGroundY(&pos, FALSE);
                                mPos = pos;
                                mItem = *item;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

bool itemSearch_c::isFound() const {
    return !mPos.isZero();
}

void itemSearch_c::getPos(mVec3_c *out) const {
    *out = mPos;
}

int itemSearch_c::getWidth(f32 radius) const {
    int n = (int)radius / 32;
    if (32.0f < radius && (int)radius % 32 != 0) {
        n++;
    }
    return n * 2 + 1;
}

siteSearch_c::siteSearch_c() {
    mPos.set(0.0f, 0.0f, 0.0f);
}

siteSearch_c::~siteSearch_c() {}

void siteSearch_c::search(mVec3_c &center, f32 radius) {
    mPos.set(0.0f, 0.0f, 0.0f);
    int unitX = toUnitPos(center.x);
    int unitZ = toUnitPos(center.z);
    int width = getWidth(radius);
    int x0 = unitX - (width >> 1);
    int z0 = unitZ - (width >> 1);
    for (int x = x0; x < x0 + width; x++) {
        for (int z = z0; z < z0 + width; z++) {
            if (dSaveBuildingList_c::get()->isBuildSite(x, z) == true) {
                dItem::Item building;
                building = dSaveBuildingList_c::get()->getAt(x, z, 1);
                if (!building.isExtId()) {
                    mVec3_c ground;
                    dFdBase_c::getUnitGroundPos(ground, x, z);
                    mVec3_c diff = center - ground;
                    f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
                    if (dist <= radius && dist <= 10000.0f) {
                        mPos = ground;
                    }
                }
            }
        }
    }
}

bool siteSearch_c::isFound() const {
    return !mPos.isZero();
}

mVec3_c siteSearch_c::getPos() {
    mVec3_c pos(mPos);
    return pos;
}

f32 siteSearch_c::getRadius() const {
    return 40.5f;
}

int siteSearch_c::getWidth(f32 radius) const {
    int n = (int)radius / 32;
    if (32.0f < radius && (int)radius % 32 != 0) {
        n++;
    }
    return n * 2 + 1;
}

playerCheck_c::playerCheck_c() {
    mPos.set(0.0f, 0.0f, 0.0f);
}

playerCheck_c::~playerCheck_c() {}

bool playerCheck_c::isValid(int player) const {
    return dPlayerMgr::getPlayerActor(player) != NULL;
}

void playerCheck_c::setPos(const mVec3_c &pos) {
    mPos = pos;
}

bool playerCheck_c::isNetPlayer(int player) const {
    if (isValid(player) == true) {
        return dPlayerMgr::isUsingNet(player);
    }
    return FALSE;
}

bool playerCheck_c::isNear(int player, f32 radius) const {
    if (isValid(player) == true && getDistance(player) <= radius) {
        return true;
    }
    return false;
}

bool playerCheck_c::isShaking(const mVec3_c &pos, int player) const {
    int unitX = 0;
    int unitZ = 0;
    if (dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, player) == 1) {
        int x, z;
        toUnit(&x, &z, pos.x, pos.z);
        if (x == unitX && z == unitZ) {
            return true;
        }
    }
    return false;
}

bool playerCheck_c::isShakingNear(const mVec3_c &pos, int player) const {
    int unitX = 0;
    int unitZ = 0;
    if (dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, player) == 2) {
        int x, z;
        toUnit(&x, &z, pos.x, pos.z);
        if (x == unitX && z == unitZ) {
            return true;
        }
    }
    return false;
}

BOOL playerCheck_c::isDigging(const mVec3_c &pos, int) const {
    int unitX = 0;
    int unitZ = 0;
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        if (dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, i) == 5) {
            int x, z;
            toUnit(&x, &z, pos.x, pos.z);
            if (x == unitX && z == unitZ) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static inline dItem::Item *getUnitItem(const int &unitX, const int &unitZ) {
    return dField::getInfo(0)->getItem(unitX, unitZ, 0);
}

static inline u32 isStone(u16 id) {
    return id >= dItem::FG_STONE_FIRST && id <= dItem::FG_STONE_LAST;
}

BOOL playerCheck_c::isHittingStone(const mVec3_c &pos) const {
    int unitX = 0;
    int unitZ = 0;
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        int kind = dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, i);
        if (kind == 3 || kind == 6) {
            int x, z;
            toUnit(&x, &z, pos.x, pos.z);
            if (x == unitX && z == unitZ) {
                dItem::Item *item = getUnitItem(unitX, unitZ);
                if (item != NULL && isStone(item->mId) == TRUE) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

bool playerCheck_c::isDisturbing(f32 radius) const {
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        if (isValid(i) == true && isNear(i, radius) == true) {
            if (dPlayerMgr::isMakingNoise(i) == TRUE) {
                return true;
            }
            if (dPlayerMgr::isActingOnUnit(i) == TRUE) {
                int unitX = 0;
                int unitZ = 0;
                if (dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, i) == 0) {
                    int x, z;
                    toUnit(&x, &z, mPos.x, mPos.z);
                    if (x == unitX && z == unitZ) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void playerCheck_c::getPlayerPos(mVec3_c *out, int player) const {
    if (isValid(player) == true) {
        *out = *dPlayerMgr::getPosition(player);
    }
}

f32 playerCheck_c::getPlayerSpeed(int player) const {
    if (isValid(player) == true) {
        return dPlayerMgr::getPlayerActor(player)->getSpeedF();
    }
    return -1.0f;
}

f32 playerCheck_c::getDistance(int player) const {
    if (isValid(player) == true) {
        mVec3_c pos;
        getPlayerPos(&pos, player);
        mVec3_c diff = pos - mPos;
        return EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    }
    return -1.0f;
}

mAng playerCheck_c::getPlayerAngle(int player) const {
    if (isValid(player) == true) {
        return dPlayerMgr::getYaw(player);
    }
    return 0;
}

mAng playerCheck_c::getAngleTo(int player) const {
    if (isValid(player) == true) {
        mVec3_c pos;
        getPlayerPos(&pos, player);
        return cLib::targetAngleY(mPos, pos);
    }
    return 0;
}

int playerCheck_c::getNearest() const {
    f32 best = 10000.0f;
    int nearest = INSECT_UTIL_PLAYER_NUM;
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        if (isValid(i) == true) {
            f32 dist = getDistance(i);
            if (dist <= best) {
                best = dist;
                nearest = i;
            }
        }
    }
    return nearest;
}

int playerCheck_c::getFastest(f32 radius) const {
    f32 best = -1.0f;
    int fastest = INSECT_UTIL_PLAYER_NUM;
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        if (isValid(i) == true && isNear(i, radius) == true) {
            f32 speed = getPlayerSpeed(i);
            if (best <= speed) {
                best = speed;
                fastest = i;
            }
        }
    }
    return fastest;
}

int playerCheck_c::getStoneHitter(const mVec3_c &pos) const {
    int unitX = 0;
    int unitZ = 0;
    for (int i = 0; i < INSECT_UTIL_PLAYER_NUM; i++) {
        int kind = dPlayerMgr::getActionUnitAndKind(&unitX, &unitZ, i);
        if (kind == 3 || kind == 6) {
            int x, z;
            toUnit(&x, &z, pos.x, pos.z);
            if (x == unitX && z == unitZ) {
                dItem::Item *item = getUnitItem(unitX, unitZ);
                if (item != NULL && isStone(item->mId) == TRUE) {
                    return i;
                }
            }
        }
    }
    return INSECT_UTIL_PLAYER_NUM;
}

bool playerCheck_c::isOnUnit(int unitX, int unitZ, int player) const {
    if (isValid(player) == true) {
        mVec3_c pos;
        getPlayerPos(&pos, player);
        int x, z;
        toUnit(&x, &z, pos.x, pos.z);
        if (unitX == x && unitZ == z) {
            return true;
        }
    }
    return false;
}

npcCheck_c::npcCheck_c() : mNpc(NULL) {
    mPos.set(0.0f, 0.0f, 0.0f);
}

npcCheck_c::~npcCheck_c() {}

void npcCheck_c::setPos(const mVec3_c &pos) {
    mPos = pos;
    findNearest();
}

bool npcCheck_c::isNear(f32 radius) const {
    if (mNpc != NULL && getDistance() <= radius) {
        return true;
    }
    return false;
}

void npcCheck_c::getNpcPos(mVec3_c *out) const {
    if (mNpc != NULL) {
        *out = *mNpc->getPosP();
    }
}

f32 npcCheck_c::getNpcSpeed() const {
    if (mNpc != NULL) {
        return mNpc->getSpeedF();
    }
    return 0.0f;
}

f32 npcCheck_c::getDistance() const {
    if (mNpc != NULL) {
        mVec3_c pos;
        getNpcPos(&pos);
        mVec3_c diff = mPos - pos;
        return EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
    }
    return 0.0f;
}

mAng npcCheck_c::getAngleTo() const {
    if (mNpc != NULL) {
        mVec3_c pos;
        getNpcPos(&pos);
        return cLib::targetAngleY(mPos, pos);
    }
    return 0;
}

void npcCheck_c::findNearest() {
    int num = dNpcList::countNml();
    f32 best = 10000.0f;
    for (int i = 0; i < num; i++) {
        dDemoActor_c *npc = dNpcList::getNml(i);
        if (npc != NULL) {
            mVec3_c pos;
            pos = *npc->getPosP();
            mVec3_c diff = mPos - pos;
            f32 dist = EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
            if (dist < best) {
                best = dist;
                mNpc = npc;
            }
        }
    }
}

axis_c::axis_c() {
    mSide.set(1.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 1.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 1.0f);
}

axis_c::~axis_c() {}

void axis_c::setIdentity() {
    mSide.set(1.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 1.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 1.0f);
}

void axis_c::normalize() {
    mSide.normalize();
    mUp.normalize();
    mFront.normalize();
}

void axis_c::setUp(const mVec3_c &up) {
    mUp = up;
    if (nw4r::math::FAbs(mUp.dot(mSide)) == 1.0f) {
        mSide = mUp.cross(mFront);
        mFront = mSide.cross(mUp);
    } else {
        mFront = mSide.cross(mUp);
        mSide = mUp.cross(mFront);
    }
    normalize();
}

void axis_c::setFront(const mVec3_c &front) {
    mFront = front;
    if (nw4r::math::FAbs(mFront.dot(mSide)) == 1.0f) {
        mSide = mUp.cross(mFront);
        mUp = mFront.cross(mSide);
    } else {
        mUp = mFront.cross(mSide);
        mSide = mUp.cross(mFront);
    }
    normalize();
}

void axis_c::rotateX(mAng angle) {
    f32 rad = angle.mAngle * mAng::AngleToRadianCoefficient;
    EGG::Quatf q;
    setQuatIdentity(&q);
    q.setAxisRotation(mSide, rad);
    mUp = fn_8044540C(&q, &mUp);
    mFront = fn_8044540C(&q, &mFront);
    setUp(mUp);
}

void axis_c::rotateY(mAng angle) {
    f32 rad = angle.mAngle * mAng::AngleToRadianCoefficient;
    EGG::Quatf q;
    setQuatIdentity(&q);
    q.setAxisRotation(mUp, rad);
    mFront = fn_8044540C(&q, &mFront);
    mSide = fn_8044540C(&q, &mSide);
    setFront(mFront);
}

void axis_c::rotateZ(mAng angle) {
    f32 rad = angle.mAngle * mAng::AngleToRadianCoefficient;
    EGG::Quatf q;
    setQuatIdentity(&q);
    q.setAxisRotation(mFront, rad);
    mUp = fn_8044540C(&q, &mUp);
    mSide = fn_8044540C(&q, &mSide);
    setUp(mUp);
}

void axis_c::set(const Mtx mtx) {
    f32 sideX = mtx[0][0], sideY = mtx[1][0], sideZ = mtx[2][0];
    f32 upX = mtx[0][1], upY = mtx[1][1], upZ = mtx[2][1];
    f32 frontX = mtx[0][2], frontY = mtx[1][2], frontZ = mtx[2][2];
    mSide.x = sideX;
    mSide.y = sideY;
    mSide.z = sideZ;
    mUp.x = upX;
    mUp.y = upY;
    mUp.z = upZ;
    mFront.x = frontX;
    mFront.y = frontY;
    mFront.z = frontZ;
    normalize();
}

void getMtx(Mtx out, const axis_c &axis) {
    PSMTXIdentity(out);
    out[0][0] = axis.mSide.x;
    out[1][0] = axis.mSide.y;
    out[2][0] = axis.mSide.z;
    out[0][1] = axis.mUp.x;
    out[1][1] = axis.mUp.y;
    out[2][1] = axis.mUp.z;
    out[0][2] = axis.mFront.x;
    out[1][2] = axis.mFront.y;
    out[2][2] = axis.mFront.z;
}

mAng axis_c::getAngleX() const {
    return cM::atan2s(-mFront.y, EGG::Mathf::sqrt(mFront.x * mFront.x + mFront.z * mFront.z));
}

mAng axis_c::getAngleY() const {
    mAng x = getAngleX();
    if (x.mAngle == 0x4000 || x.mAngle == -0x4000) {
        return cM::atan2s(-mSide.z, mSide.x);
    }
    return cM::atan2s(mFront.x, mFront.z);
}

mAng axis_c::getAngleZ() const {
    mAng x = getAngleX();
    if (x.mAngle == 0x4000 || x.mAngle == -0x4000) {
        return 0;
    }
    return cM::atan2s(mSide.y, mUp.y);
}

int secToFrame(f32 sec) {
    return 60.0f * sec;
}

BOOL getGroundMovePos(mVec3_c *out, mVec3_c &from, const mVec3_c &to, u32 steps, BOOL noStoneBus, BOOL noSea,
                      f32 minDy, f32 maxDy) {
    mVec3_c last(0.0f, 0.0f, 0.0f);
    mVec3_c prev(0.0f, 0.0f, 0.0f);
    mVec3_c prev2(0.0f, 0.0f, 0.0f);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    u8 count = 0;
    mVec3_c dir = to - from;
    for (u8 i = 0; i < steps; i++) {
        prev2 = prev;
        prev = last;
        f32 t = (f32)(i + 1) / (f32)steps;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
        mVec3_c pos = from + t * dir;
        pos.y = dBGCF::getGroundY(&pos, TRUE);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        f32 dy = chk.getHeight(TRUE) - from.y;
        bool inRange = false;
        if (minDy <= dy && dy <= maxDy) {
            inRange = true;
        }
        mVec3_c normal(chk.mFloor.mNormal);
        bool steep = nw4r::math::FAbs(normal.dot(up)) < 0.95f;
        if (inRange == true || steep == true) {
            bool open = isStructure(pos) == 0;
            if (!open) {
                break;
            }
            if (dBGCF::isNearFall(&pos)) {
                break;
            }
            if (noSea && chk.mWater == BG_WATER_SEA) {
                break;
            }
            bool free = isColumn(pos) == false;
            if (!free) {
                break;
            }
            int unitX, unitZ;
            toUnit(&unitX, &unitZ, pos.x, pos.z);
            if (noStoneBus && dBGCF::getAttr(unitX, unitZ) == BG_ATTR_STONE_BUS) {
                break;
            }
            last = pos;
            count++;
        } else {
            bool above = maxDy <= dy;
            if (above != true) {
                break;
            }
            if (steep) {
                break;
            }
            mVec3_c start;
            if (i == 0) {
                start = from;
            } else {
                start = last;
            }
            mVec3_c end;
            end = pos;
            f32 y = from.y + 0.5f * dy;
            end.y = y;
            start.y = y;
            if (canPass(&start, &end) != true) {
                break;
            }
            bool open = isStructure(pos) == 0;
            if (!open) {
                break;
            }
            last = pos;
            count++;
        }
    }
    *out = prev2;
    if (steps > 2) {
        if (count < 2) {
            return FALSE;
        }
        *out = prev;
    } else {
        *out = last;
        if (last.isZero() == true) {
            return FALSE;
        }
    }
    return TRUE;
}

static inline bool isColumnNear(int x, int z, u8 i, u8 j) {
    const int unitX = x;
    const int unitZ = z;
    return isColumn(unitX + i - 1, unitZ + j - 1);
}

void getLandPos(mVec3_c *out, const mVec3_c &pos, u32 mode, BOOL noStoneBus) {
    int unitX = toUnitPos(pos.x);
    int unitZ = toUnitPos(pos.z);
    int x, z;
    BOOL open = FALSE;
    u32 columns = 0;
    if (isColumn(unitX, unitZ - 1) == true) {
        columns = 1;
    }
    if (isColumn(unitX - 1, unitZ) == true) {
        columns++;
    }
    if (isColumn(unitX + 1, unitZ) == true) {
        columns++;
    }
    if (isColumn(unitX, unitZ + 1) == true) {
        columns++;
    }
    if (columns < 4) {
        open = TRUE;
    }
    mVec3_c found[8];
    u8 num = 0;
    for (u8 i = 0; i < 3; i++) {
        for (u8 j = 0; j < 3; j++) {
            if (i == 1 && j == 1) {
                continue;
            }
            x = unitX + i - 1;
            z = unitZ + j - 1;
            mVec3_c center;
            dFdBase_c::getUnitCenterPos(center, x, z);
            f32 groundY = dBGCF::getGroundY(&center, TRUE);
            dBGCF::groundChk_c chk(&center, 0, 0, 0);
            if (!(groundY <= pos.y)) {
                continue;
            }
            if (mode == 1) {
                bool free = !isStructure(center);
                if (!free) {
                    continue;
                }
                if (dBGCF::isNearFall(&center)) {
                    continue;
                }
                if (chk.mWater == BG_WATER_SEA) {
                    continue;
                }
                if (noStoneBus && dBGCF::getAttr(x, z) == BG_ATTR_STONE_BUS) {
                    continue;
                }
                center.y = groundY;
                *out = center;
                found[num++] = center;
            } else {
                if (!(pos.y - groundY < dBGCF::cWaterY0)) {
                    continue;
                }
                bool free = !isStructure(center);
                if (!free) {
                    continue;
                }
                if (dBGCF::isNearFall(&center)) {
                    continue;
                }
                if (open && isColumnNear(unitX, unitZ, i, j)) {
                    continue;
                }
                if (noStoneBus && dBGCF::getAttr(x, z) == BG_ATTR_STONE_BUS) {
                    continue;
                }
                center.y = groundY;
                *out = center;
                found[num++] = center;
            }
        }
    }
    if (num == 0) {
        *out = pos;
    } else {
        u8 idx = getRandom(0, num);
        *out = found[idx];
    }
}

BOOL isExcludedFlower(const mVec3_c *pos) {
    return FALSE;
}

bool isRaining() {
    if (dWeather_c::sInstance != NULL) {
        int weather = dWeather_c::sInstance->mWeather;
        return weather == dWeather::HEAVY_RAIN || weather == dWeather::RAIN;
    }
    return TRUE;
}

bool isSnowing() {
    if (dWeather_c::sInstance != NULL) {
        int weather = dWeather_c::sInstance->mWeather;
        return weather == dWeather::HEAVY_SNOW || weather == dWeather::SNOW;
    }
    return TRUE;
}

void moveTo(mVec3_c *pos, mAng *angle, const mVec3_c &target, f32 speed, f32 turnDeg) {
    s16 targetAngle = cLib::targetAngleY(*pos, target);
    s16 step = turnDeg * mAng::DegreeToAngleCoefficient;
    sLib::addCalcAngle(&angle->mAngle, targetAngle, 1, step, step);
    mVec3_c move(0.0f, 0.0f, speed);
    move.rotY(*angle);
    *pos += move;
}

void easeMove(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 total, int frame) {
    f32 t = (f32)frame / (f32)total;
    t = t > 1.0f ? 1.0f : t;
    f32 rate = cLib::easeOut(t, 2.0f);
    *out = from + rate * (to - from);
}

void easeMoveXZ(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 total, int frame) {
    f32 t = (f32)frame / (f32)total;
    t = t > 1.0f ? 1.0f : t;
    f32 rate = cLib::easeOut(t, 2.0f);
    out->x = from.x + rate * (to.x - from.x);
    out->z = from.z + rate * (to.z - from.z);
}

void flyTo(mVec3_c *pos, axis_c *axis, const mVec3_c &target, const mVec3_c &up, f32 speed, f32 turnRate) {
    axis->setUp(up);
    mVec3_c front = axis->getFront();
    mVec3_c dir = target - *pos;
    dir.y = 0.0f;
    dir.normalize();
    mVec3_c diff = dir - front;
    front += turnRate * diff;
    front.normalize();
    axis->setFront(front);
    mVec3_c move = front;
    move *= speed;
    *pos += move;
}

BOOL getWaterMovePos(mVec3_c *out, const mVec3_c &from, const mVec3_c &to, u32 steps) {
    mVec3_c last(0.0f, 0.0f, 0.0f);
    mVec3_c prev(0.0f, 0.0f, 0.0f);
    mVec3_c prev2(0.0f, 0.0f, 0.0f);
    mVec3_c up(0.0f, 1.0f, 0.0f);
    u8 count = 0;
    mVec3_c dir = to - from;
    for (u8 i = 0; i < steps; i++) {
        prev2 = prev;
        prev = last;
        f32 t = (f32)(i + 1) / (f32)steps;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }
        mVec3_c pos = from + t * dir;
        pos.y = dBGCF::getGroundY(&pos, TRUE);
        dBGCF::groundChk_c chk(&pos, 1, 0, 0);
        if (chk.mWater == BG_WATER_NONE || chk.mWater != BG_WATER_RIVER) {
            break;
        }
        f32 waterY = chk.mWaterY;
        if (from.y - waterY < 10.0f) {
            if (dBGCF::isNearFall(&pos)) {
                break;
            }
            last = pos;
            count++;
        }
    }
    *out = prev2;
    if (steps > 2) {
        if (count < 2) {
            return FALSE;
        }
        *out = prev2;
    } else {
        *out = last;
        if (count == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

void getWaterPos(mVec3_c *out, const mVec3_c &pos) {
    int unitX, unitZ;
    toUnit(&unitX, &unitZ, pos.x, pos.z);
    int x;
    mVec3_c found[8];
    u8 num = 0;
    for (u8 i = 0; i < 3; i++) {
        x = unitX + i - 1;
        for (u8 j = 0; j < 3; j++) {
            if (i == 1 && j == 1) {
                continue;
            }
            mVec3_c center;
            dFdBase_c::getUnitCenterPos(center, x, unitZ + j - 1);
            dBGCF::groundChk_c chk(&center, 0, 0, 0);
            if (chk.mWater == BG_WATER_NONE || chk.mWater != BG_WATER_RIVER) {
                continue;
            }
            f32 waterY = chk.mWaterY;
            if (!(pos.y - waterY < 10.0f)) {
                continue;
            }
            if (dBGCF::isNearFall(&center)) {
                continue;
            }
            center.y = chk.getHeight(FALSE);
            found[num++] = center;
        }
    }
    if (num == 0) {
        *out = pos;
    } else {
        u8 idx = getRandom(0, num);
        *out = found[idx];
    }
}

dItem::Item *getItem(const mVec3_c *pos) {
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, *pos);
    dFdBlock_c *block = dField::getInfo(0)->getBlock(blockX, blockZ);
    if (block != NULL) {
        return block->getItem(unitX, unitZ, 0);
    }
    return NULL;
}

int getDecayFrames(f32 rate) {
    f32 clamped = rate < 0.05f ? 0.05f : rate; // unused: the logs below take rate
    return nw4r::math::FAbs(EGG::Mathf::log10(0.05f) / EGG::Mathf::log10(rate));
}

bool isColumn(int unitX, int unitZ) {
    f32 radius;
    f32 height;
    int attr;
    if (dBGCF::getColumnAttr(unitX, unitZ, &radius, &height, &attr) && attr == 1) {
        return true;
    }
    return false;
}

bool isColumn(mVec3_c pos) {
    int unitX, unitZ;
    toUnit(&unitX, &unitZ, pos.x, pos.z);
    return isColumn(unitX, unitZ);
}

u32 isStructure(mVec3_c pos) {
    int unitX, unitZ;
    toUnit(&unitX, &unitZ, pos.x, pos.z);
    return dSTR::getTownUnitAttr()->getAttr(unitX, unitZ) & dFdUnitAttr_c::STR_COL;
}

#pragma force_active on
bool isSameUnit(const mVec3_c &a, const mVec3_c &b) {
    int ax, az, bx, bz;
    toUnit(&ax, &az, a.x, a.z);
    toUnit(&bx, &bz, b.x, b.z);
    if (ax == bx && az == bz) {
        return true;
    }
    return false;
}
#pragma force_active reset

u8 getRandom(u8 min, u8 max) {
    return min + cM::rndF(max - min);
}

} // namespace dInsectUtil
