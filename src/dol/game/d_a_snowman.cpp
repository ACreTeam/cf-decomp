// The snowballs and the snowmen built from them. See
// include/game/game/d_a_snowman.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_msg.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_date_event.hpp>
#include <game/game/d_world.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <revolution/MTX.h>
#include <cmath>
#include <game/game/d_a_player.hpp>

static const f32 cJumpFrames = 60;

static const f32 cScaleMin = dSvDateEvent::cScaleMin;
static const f32 cScaleMax = dSvDateEvent::cScaleMax;
static const f32 cScaleRange = cScaleMax - cScaleMin;
static const f32 cStackScaleMin = 1.25f * cScaleMin;
static const f32 cRadiusMin = 15.5f * cScaleMin;
static const f32 cRadiusMax = 15.5f * cScaleMax;
static const f32 cJumpTime = cJumpFrames;
static const f32 cJumpRate = 1.0f / cJumpFrames;

struct daSnowManParam_c {
    daSnowManParam_c()
        : mGravityMin(-0.24f), mGravityMax(-0.24f), mGravityAir(-0.1f),
          mSinkRate(0.02f), mWaterFriction(0.975f), mFlowRate(0.004f),
          mMaxFall(-32.0f), mStopSpeed(0.005f), mHoleDepthMin(-8.0f),
          mHoleDepthMax(-3.5f), mPushDot(0.75f), mFriction(0.97f),
          mHitPush(0.15f), mPushRate(0.15f), mRollRateMin(0.6f),
          mPushRateMin(0.0f), mWallBounce(0.93f), mPushDistance(4.0f),
          mPushFollow(0.1f), _4C(90.0f) {
    }

    /* 0x00 */ f32 mGravityMin;
    /* 0x04 */ f32 mGravityMax;
    /* 0x08 */ f32 mGravityAir;
    /* 0x0C */ f32 mSinkRate;
    /* 0x10 */ f32 mWaterFriction;
    /* 0x14 */ f32 mFlowRate;
    /* 0x18 */ f32 mMaxFall;
    /* 0x1C */ f32 mStopSpeed;
    /* 0x20 */ f32 mHoleDepthMin;
    /* 0x24 */ f32 mHoleDepthMax;
    /* 0x28 */ f32 mPushDot;
    /* 0x2C */ f32 mFriction;
    /* 0x30 */ f32 mHitPush;
    /* 0x34 */ f32 mPushRate;
    /* 0x38 */ f32 mRollRateMin;
    /* 0x3C */ f32 mPushRateMin;
    /* 0x40 */ f32 mWallBounce;
    /* 0x44 */ f32 mPushDistance;
    /* 0x48 */ f32 mPushFollow;
    /* 0x4C */ f32 _4C;
}; // size 0x50

static daSnowManParam_c l_param;

inline f32 daSnowMan_c::getGravity() const {
    f32 t = (mScale - cScaleMin) / cScaleRange;
    return l_param.mGravityMax * t + l_param.mGravityMin * (1.0f - t);
}

daSnowMan_c::State_s daSnowMan_c::sStates[STATE_NUM] = {
    {&daSnowMan_c::initRoll, &daSnowMan_c::executeRoll,
     &daSnowMan_c::finalRoll},
    {&daSnowMan_c::initSink, &daSnowMan_c::executeSink,
     &daSnowMan_c::finalSink},
    {&daSnowMan_c::initJump, &daSnowMan_c::executeJump,
     &daSnowMan_c::finalJump},
    {&daSnowMan_c::initSettle, &daSnowMan_c::executeSettle,
     &daSnowMan_c::finalSettle},
    {&daSnowMan_c::initBuilt, &daSnowMan_c::executeBuilt,
     &daSnowMan_c::finalBuilt},
    {&daSnowMan_c::initBase, &daSnowMan_c::executeBase,
     &daSnowMan_c::finalBase},
    {&daSnowMan_c::initSnowman, &daSnowMan_c::executeSnowman,
     &daSnowMan_c::finalSnowman},
    {&daSnowMan_c::initTalk, &daSnowMan_c::executeTalk,
     &daSnowMan_c::finalTalk},
    {&daSnowMan_c::initTalkEnd, &daSnowMan_c::executeTalkEnd,
     &daSnowMan_c::finalTalkEnd},
};

daSnowMan_c *daSnowMan_c::sBall[BALL_NUM];
daSnowMan_c::ball_c daSnowMan_c::sBallInfo[BALL_NUM];

static dDvd::CommonBank_c<dDvd::brresBank_c> l_bank("/Snowball/fg_snow.brres");

static inline f32 toScale(u8 scale) {
    return dSvDateEvent::cScaleMin +
           scale * (dSvDateEvent::cScaleMax - dSvDateEvent::cScaleMin) / 255.0f;
}

static inline BOOL isHole(const dItem::Item *item) {
    BOOL result = FALSE;
    if (item->mId >= dItem::FG_HOLE && item->mId <= dItem::FG_HOLE) {
        result = TRUE;
    }
    return result;
}

// 801811D0
CUSTOM_BASE_PROFILE(SNOWMAN, daSnowMan_c, fProf::SNOWMAN,
                    fProf::DRAW_ORDER::SNOWMAN);

// 801812F4
int daSnowMan_c::create() {
    EGG::ExpHeap *heap = dHeap::localHeap_p;
    if (!l_bank.attach(&mResId, heap)) {
        return NOT_READY;
    }
    mAllocator.attach(heap, 0x20);
    mBodyMdl.create(
        nw4r::g3d::ResFile(l_bank.getData()).GetResMdl("fg_snow_body"),
        &mAllocator, 0, 1, NULL);
    mFaceMdl.create(
        nw4r::g3d::ResFile(l_bank.getData()).GetResMdl("fg_snow_face"),
        &mAllocator, 0, 1, NULL);
    mPos.y = dBGCF::getGroundY(&mPos, FALSE);
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mGroundPos = mPos;
    mAccelY = l_param.mGravityMin;
    mMaxSpeedY = l_param.mMaxFall;
    setDemoID((u16)mParam);
    u32 param = mParam;
    if (param < BALL_NUM) {
        mAcch.init();
        mOnSlope = false;
        mSlopeNormal = mVec3_c::Ey;
        mSlopeY = 0.0f;
        mpMdl = &mBodyMdl;
        ball_c *info = &sBallInfo[mParam];
        mScale = info->mScale;
        mRadius = getRadius();
        mSink = 0.0f;
        mAccelY = getGravity();
        mRollDist = 2560.0f * (mScale - cScaleMin) / cScaleRange;
        mKeep = true;
        mHitting = false;
        mPushStarted = false;
        mWasPushed = false;
        mPushed = false;
        mGrowRate = 0.1f;
        mRotation.set(1.0f, 0.0f, 0.0f, 0.0f);
        mRotSpeed.set(1.0f, 0.0f, 0.0f, 0.0f);
        sBall[mParam] = this;
        mJustBuilt = true;
        mInAir = false;
        mHit = false;
        mWeight = 0x80;
        changeState(STATE_ROLL);
    } else {
        dSvDateEvent::Event_c *rec =
            &dSaveData_c::getTown()->mSnowmen[param - SNOWMAN];
        if (!rec->mValid) {
            return NOT_READY;
        }
        int days = rec->diffDaysAdjusted(dTime_c::getCurrent());
        f32 rate = 1.0f;
        if (days < 0 || days >= dSvDateEvent::SNOWMAN_NUM) {
            deleteRequest();
            return NOT_READY;
        }
        switch (days) {
        case 1:
            rate = 2.0f / 3.0f;
            break;
        case 2:
            rate = 1.0f / 3.0f;
            break;
        }
        mFace = getFace(toScale(rec->mScale1), toScale(rec->mScale2));
        mHeadScale = rate * toScale(rec->mScale1);
        mBodyScale = rate * toScale(rec->mScale2);
        mJustBuilt = false;
        mWeight = 0xFF;
        changeState(STATE_SNOWMAN);
    }
    return SUCCEEDED;
}

// 801817F0
int daSnowMan_c::execute() {
    mSound.setPos(&mPos);
    executeState();
    calcMtx();
    return SUCCEEDED;
}

// 80181854
int daSnowMan_c::draw() {
    mAng angle;
    if (mParam < BALL_NUM) {
        f32 radius = getRadius();
        mMtx_c mtx;
        mpMdl->getLocalMtx(&mtx);
        mSphere_c sphere(mVec3_c(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]),
                         radius);
        if (dCamera_c::sInstance->mCullFrustum.intersectSphere(&sphere)) {
            mpMdl->entry();
            if (mState != STATE_SINK) {
                angle = 0;
                dShadow::requestRound(&mPos, 0, &angle, 24.0f * mScale, 1.0f,
                                      0.5f, 0.0f, 1.0f);
            }
        }
    } else {
        mFaceMdl.entry();
        mBodyMdl.entry();
        angle = 0;
        dShadow::requestRound(&mPos, 0, &angle, 24.0f * mBodyScale, 1.0f, 0.5f,
                              0.0f, 1.0f);
    }
    return SUCCEEDED;
}

// 801819B0
int daSnowMan_c::doDelete() {
    if (!mDeleted) {
        if (mParam < BALL_NUM) {
            if (mKeep) {
                sBallInfo[mParam].set(mPos, mScale);
            } else {
                sBallInfo[mParam].mActive = false;
                if (!mJustBuilt) {
                    int x = worldToUnit(mPos.x);
                    int z = worldToUnit(mPos.z);
                    if (mFgSet) {
                        dField::getInfo(0)->setItemFromId(dItem::ITEM_ID_NONE,
                                                          x, z, 0);
                    }
                    if (!placeFg(x, z)) {
                        placeFgAround(x, z);
                    }
                }
            }
            if (mParam < BALL_NUM) {
                sBall[mParam] = NULL;
            }
        }
        mDeleted = true;
    }
    if (!l_bank.detach(&mResId)) {
        return NOT_READY;
    }
    mBodyMdl.remove();
    mFaceMdl.remove();
    return SUCCEEDED;
}

// 80181BB4
BOOL daSnowMan_c::getWallNormal(dBGCF::acch_c *acch, mVec3_c *normal) {
    dBGCF::acchWall_c *wall = acch->getWall();
    normal->set(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < wall->getNum(); i++) {
        mVec3_c dir(0.0f, 0.0f, 1.0f);
        dir.rotY(wall->mAngle[i]);
        *normal += dir;
    }
    *normal /= wall->getNum();
    return normal->normalizeRS();
}

// The sender and paper literals become anonymous .sdata temporaries.
static inline void setupMail(dMail_c *mail, u16 kindValue, const u8 &sender,
                             const dPersonalID_c *to, const int &paper) {
    const u16 kind = kindValue;
    mail->setupSystem(&kind, "MAIL_NPC_snowman", &sender, to, &paper);
}

// 80181CEC
BOOL daSnowMan_c::sendMail() {
    static const int l_presents[] = {
        dItem::ITEM_IDX_SNOWMAN_FRIDGE,   dItem::ITEM_IDX_SNOWMAN_TABLE,
        dItem::ITEM_IDX_SNOWMAN_BED,      dItem::ITEM_IDX_SNOWMAN_CHAIR,
        dItem::ITEM_IDX_SNOWMAN_LAMP,     dItem::ITEM_IDX_SNOWMAN_SOFA,
        dItem::ITEM_IDX_SNOWMAN_TV,       dItem::ITEM_IDX_SNOWMAN_DRESSER,
        dItem::ITEM_IDX_SNOWMAN_WARDROBE, dItem::ITEM_IDX_SNOWMAN_CLOCK,
        dItem::ITEM_IDX_SNOWMAN_CARPET,   dItem::ITEM_IDX_SNOWMAN_WALL,
        dItem::ITEM_IDX_SNOWMAN,
    };
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    dSearchCand_c<13> cand;
    cand.setAll();
    for (int i = 0; i < 13; i++) {
        dItem::Item item;
        item.setFromIndex(l_presents[i]);
        if (player->mCatalog.isRegistered(dItem::Item(item))) {
            cand.remove(i);
        }
    }
    static dMail_c l_mail;
    dPersonalID_c to = player->mPID;
    dItem::Item present;
    int idx;
    if (cand.getCount() > 0) {
        idx = cand.getRandom();
    } else {
        idx = cM::rndInt(13);
    }
    present.setFromIndex(l_presents[idx]);
    dLetter::setItemName(0, present);
    setupMail(&l_mail, idx + 1, 5, &to, dItem::ITEM_IDX_SNOWMAN_PAPER);
    l_mail.setPresent(present.mId, 0xFF);
    if (dPostOffice::add(&l_mail)) {
        return TRUE;
    }
    return FALSE;
}

// 80181F4C
f32 daSnowMan_c::getPushRate() const {
    return l_param.mPushRateMin + (1.0f - l_param.mPushRateMin) *
                                      (cScaleMax - mScale) /
                                      (cScaleMax - cStackScaleMin);
}

// 80181F84
f32 daSnowMan_c::getRollRate() const {
    return l_param.mRollRateMin + (1.0f - l_param.mRollRateMin) *
                                      (cScaleMax - mScale) /
                                      (cScaleMax - cScaleMin);
}

// 80181FBC
BOOL daSnowMan_c::push(f32 speed, mVec3_c *pos, s16 *angle, f32 *speedF,
                       mAng dir) {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == NULL) {
        return FALSE;
    }
    if (mScale < cStackScaleMin) {
        mPushStarted = false;
        return FALSE;
    }
    if (mState != STATE_ROLL) {
        return FALSE;
    }
    if (mOnSlope) {
        return FALSE;
    }
    mAng pushAngle = dir;
    mVec3_c front = mVec3_c::Ez;
    front.rotY(dir);
    f32 dot = front.dot(mSpeed);
    if (mWasPushed) {
        pushAngle = mPushAngle;
        sLib::addCalcAngle(&pushAngle.mAngle, dir, 8, 0x1555);
    } else if (dot > l_param.mPushDot) {
        if (mPushStarted) {
            return FALSE;
        }
        mPushStarted = true;
    }
    if (mSink > 0.1f) {
        return FALSE;
    }
    if (speed < (mWasPushed ? 0.05f : 0.2f)) {
        return FALSE;
    }
    mVec3_c playerPos = player->mPos;
    mVec3_c diff = mPos - playerPos;
    f32 reach = getRadius() + 16.0f;
    f32 maxDist;
    if (mWasPushed) {
        maxDist = 8.0f + reach;
    } else {
        maxDist = reach;
    }
    f32 dist = diff.xzLen();
    if (dist > maxDist) {
        return FALSE;
    }
    if (!diff.normalizeRS()) {
        return FALSE;
    }
    mVec3_c normal;
    if (getGroundNormal(&normal)) {
        if (front.dot(normal) > 0.1f) {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    if (diff.dot(front) < mAng(mWasPushed ? 0x4000 : 0x1555).cos()) {
        return FALSE;
    }
    front = mVec3_c::Ez;
    front.rotY(pushAngle);
    if (diff.dot(front) < mAng(0x4000).cos()) {
        return FALSE;
    }
    f32 push = speed * l_param.mPushRate;
    f32 accel = push * mGrowRate;
    f32 accelX = accel * pushAngle.sin();
    f32 accelZ = accel * pushAngle.cos();
    f32 rate = getRollRate();
    mSpeed.x += accelX * rate;
    mSpeed.z += accelZ * rate;
    s16 target = cM::atan2s(diff.x, diff.z);
    sLib::addCalcAngle(angle, target, 2, 0x71C);
    f32 d = dist;
    sLib::chase(&d, reach - l_param.mPushDistance,
                mSpeedF + l_param.mPushFollow);
    mVec3_c ofs(0.0f, 0.0f, d);
    ofs.rotY(target);
    playerPos.x = mPos.x - ofs.x;
    playerPos.z = mPos.z - ofs.z;
    *speedF = mSpeedF;
    *pos = playerPos;
    mPushed = true;
    mPushAngle = pushAngle;
    int t = (mScale - cScaleMin) / cScaleRange;
    int s = 1.0f - t;
    int rateGrow = 0.125 * t + 0.5f * s;
    sLib::addCalc2(&mGrowRate, 1.0f, rateGrow, 1.0f);
    return TRUE;
}

// 8018256C
BOOL daSnowMan_c::canRoll() {
    if (mState == STATE_ROLL && !mHitting) {
        mVec3_c normal;
        if (getGroundNormal(&normal) && normal.dot(mVec3_c::Ey) > 0.9396926f &&
            !mWasPushed && !mPushed) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80182620
BOOL daSnowMan_c::hit(const mVec3_c *pos) {
    if (canRoll()) {
        mSpeed *= 0.9f;
        mHitPos = *pos;
        mHit = true;
        return TRUE;
    }
    return FALSE;
}

// 801826B4
bool daSnowMan_c::getWallNormal(mVec3_c *normal) {
    if (mAcch.isWallHit() && normal != NULL && getWallNormal(&mAcch, normal)) {
        return true;
    }
    return false;
}

static inline bool isSteep(const mVec3_c &normal) {
    mVec3_c up(0.0f, 1.0f, 0.0f);
    return normal.dot(up) < mAng(0xE39).cos();
}

// 80182700
void daSnowMan_c::initRoll() {
}

// 80182704
void daSnowMan_c::executeRoll() {
    daSnowMan_c *other;
    dFdBase_c *field;
    BOOL rolling = FALSE;
    other = getOther();
    field = dField::getInfo(0);
    if (other != NULL) {
        BOOL soil = field->getDigType(&other->mPos) == 0;
        BOOL flat = !mOnSlope && !other->mOnSlope;
        if (mObjc.isHit(dObjc::TYPE_SNOWBALL) && other->mScale >= cStackScaleMin &&
            mPushed && soil && flat && !dDemo::isType(dDemo::TYPE_ANY) &&
            dDemo::requestSpeak(this, NULL)) {
            clearDemoState();
            changeState(STATE_JUMP);
            other->changeState(STATE_SETTLE);
        }
    }
    f32 radius = getRadius();
    f32 height = radius;
    if (!mPushed) {
        sLib::chase(&mSink, 0.0f, 0.2f);
        height = radius - mSink;
    }
    if (mObjc.mHitType != 0) {
        mPos += *mObjc.getPush();
        if (!mPushed) {
            mSpeed += getPushRate() * (l_param.mHitPush * *mObjc.getPush());
        }
        if (!mHitting && mScale < cStackScaleMin) {
            mSound.startSound(0x17E3);
        }
        mHitting = true;
    } else {
        mHitting = false;
    }
    mSpeed.y =
        mSpeed.y + mAccelY < mMaxSpeedY ? mMaxSpeedY : mSpeed.y + mAccelY;
    if (mHit) {
        mPos = mHitPos;
    } else {
        mPos += mSpeed;
    }
    if (mWasPushed || mSpeed.xzLen() > l_param.mStopSpeed) {
        mSpeed.x *= l_param.mFriction;
        mSpeed.z *= l_param.mFriction;
    } else {
        mSpeed.x = 0.0f;
        mSpeed.z = 0.0f;
    }
    mObjc.set(this, height, 2.0f * radius, dObjc::TYPE_SNOWBALL,
              mWasPushed ? 0x40007E : dObjc::MASK_DEFAULT, mWeight, 0);
    mObjc.mParam = (u32)this;
    if (!checkGround()) {
        doBreak();
        return;
    }
    if (mAcch.isWallHit()) {
        mVec3_c normal;
        if (getWallNormal(&mAcch, &normal)) {
            f32 dot = mSpeed.dot(normal);
            if (dot < 0.0f) {
                mSpeed -= 1.1f * (dot * normal);
                if (!mPushed && !mWasPushed &&
                    !(mAcch.mPrevHitFlags & dBGCF::HIT_WALL)) {
                    if (dot < -1.0) {
                        doBreak();
                        return;
                    }
                    mSound.startSound(0x17E4);
                }
            }
        }
    }
    mVec3_c groundNormal;
    BOOL onGround = getGroundNormal(&groundNormal);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    if (onGround) {
        f32 dot = mSpeed.dot(groundNormal);
        if (mInAir && dot < -2.0f) {
            doBreak();
            return;
        }
        mSpeed -= dot * groundNormal;
        if (mPushed) {
            mVec3_c gravity(0.0f, mAccelY, 0.0f);
            gravity -= gravity.dot(groundNormal) * groundNormal;
            mSpeed -= l_param.mWallBounce * gravity;
        } else {
            if (isSteep(groundNormal)) {
                rolling = TRUE;
            }
        }
        mInAir = false;
    } else {
        if (!mInAir) {
            mSound.startSound(0x17E5);
        }
        mInAir = true;
        mPushed = false;
    }
    if (chk.mWater != 0) {
        mKeep = false;
        mPushed = false;
        if (mAcch.mHitFlags & dBGCF::HIT_UNDER_WATER) {
            changeState(STATE_SINK);
            return;
        }
    }
    mObjc.regist();
    mVec3_c move = mPos - mLastPos;
    f32 dist = move.xzLen();
    mSpeedF = dist;
    if (onGround) {
        if (dist > l_param.mStopSpeed) {
            f32 rot = dist / radius;
            mVec3_c axis(groundNormal.y * move.z - groundNormal.z * move.y,
                         groundNormal.z * move.x - groundNormal.x * move.z,
                         groundNormal.x * move.y - groundNormal.y * move.x);
            if (axis.normalizeRS()) {
                mRotSpeed.setAxisRotation(axis, rot);
            }
        } else {
            mRotSpeed.set(1.0f, 0.0f, 0.0f, 0.0f);
        }
    }
    calcRotation();
    BOOL snowy = field->getGrassWear(&mPos, TRUE) >= 0x80;
    if (snowy) {
        sLib::chase(&mRollDist, 2560.0f, dist);
    } else {
        sLib::chase(&mRollDist, 0.0f, 0.5f * dist);
    }
    if (!mPushed) {
        mGrowRate = 0.1f;
    } else if (dist > 0.1f) {
        rolling = TRUE;
    }
    if (rolling) {
        mSound.holdSound(0x17E8);
    }
    mWasPushed = mPushed;
    mPushed = false;
    mHit = false;
    mScale = cScaleMin + cScaleRange * (mRollDist / 2560.0f);
}

// 80183128
void daSnowMan_c::finalRoll() {
}

// 8018312C
static BOOL isInFallBasin(const mVec3_c *pos) {
    dFishField_c *field = g_fishField;
    if (field == NULL) {
        return FALSE;
    }
    mVec3_c d = *pos;
    d -= field->mFallPos;
    if (d.x * d.x + d.z * d.z < 9216.0f) {
        f32 dot = d.x * field->mFallNormal.x + d.z * field->mFallNormal.z;
        if (dot > -32.0f && dot < 0.1f) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801831CC
void daSnowMan_c::initSink() {
    f32 range = 0.025f;
    mInAir = false;
    mTimer = cM::rndInt(120) + 60;
    EGG::Quatf q;
    q.set(1.0f, 0.0f, 0.0f, 0.0f);
    mRotSpeed.slerpTo(q, 0.125f, mRotSpeed);
    g_fishField->scareFish(-1.0f, &mPos);
    mVec3_c pos = mPos;
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_TOP, 0, 0);
    pos.y = chk.mWaterY;
    setSplashEffect(&pos);
    mSound.startSound(0x17E9);
    mAlpha = 0;
}

static inline f32 getSlopeRate(f32 t) {
    return 0.2f + 0.5f * t;
}

// 801832B0
void daSnowMan_c::executeSink() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 1, 0);
    f32 depth = getDepth(chk.mWaterY);
    mVec3_c groundNormal;
    BOOL onGround = getGroundNormal(&groundNormal);
    if (!(mAcch.mHitFlags & dBGCF::HIT_UNDER_WATER)) {
        mAccelY = l_param.mGravityAir;
        if (!mInAir) {
            dBGCF::acch_c acch;
            if (!mAcch.isWallHit()) {
                dBGCF::acch_c *collision = &acch;
                mVec3_c pos = mPos;
                pos.y = chk.getHeight(FALSE);
                collision->check(16.0f, &pos, &mPos, mAngle.y, 0,
                                 dBGCF::CHECK_WALL, TRUE);
                if (collision->isWallHit()) {
                    mVec3_c normal;
                    if (getWallNormal(collision, &normal)) {
                        if (!mOnWall) {
                            mSpeed += 0.25f * normal;
                        }
                        mOnWall = true;
                        mWallNormal = normal;
                    }
                }
            }
        }
        if (mOnWall) {
            mVec3_c slope;
            f32 radius = getRadius();
            mVec3_c d = mPos - mGroundPos;
            f32 t = d.dot(mWallNormal) / radius;
            if (t > 1.0f || t < 0.0f) {
                mOnWall = false;
                mOnSlope = false;
                return;
            }
            f32 k = getSlopeRate(t);
            slope = k * mWallNormal;
            slope.y = 1.0f - k;
            if (slope.normalizeRS()) {
                mOnSlope = true;
                mSlopeNormal = slope;
            }
        }
        mInAir = true;
    } else {
        mInAir = false;
        mGroundPos = mPos;
    }
    if (mInAir) {
        mSpeed.y =
            mSpeed.y + mAccelY < mMaxSpeedY ? mMaxSpeedY : mSpeed.y + mAccelY;
        if (onGround) {
            mSpeed -= mSpeed.dot(groundNormal) * groundNormal;
            f32 radius = getRadius();
            f32 len = mSpeed.xzLen();
            if (len > l_param.mStopSpeed) {
                f32 rot = len / radius;
                mVec3_c axis(
                    groundNormal.y * mSpeed.z - groundNormal.z * mSpeed.y,
                    groundNormal.z * mSpeed.x - groundNormal.x * mSpeed.z,
                    groundNormal.x * mSpeed.y - groundNormal.y * mSpeed.x);
                if (axis.normalizeRS()) {
                    mRotSpeed.setAxisRotation(axis, rot);
                }
            }
        }
        mPos += mSpeed;
    } else {
        f32 lift = -mAccelY;
        f32 sink = l_param.mSinkRate * depth;
        mSpeed.y -= sink > lift ? lift : sink;
        mSpeed *= l_param.mWaterFriction;
        mVec3_c flow = chk.mDir;
        if (flow.normalizeRS()) {
            mSpeed += l_param.mFlowRate * flow;
        }
        if (sLib::calcTimer(&mTimer) == 0) {
            f32 range = 0.025f;
            mAng3_c ang(mAng::fromDegree(cM::rndRange(-range, range)),
                        mAng::fromDegree(cM::rndRange(-range, range)),
                        mAng::fromDegree(cM::rndRange(-range, range)));
            mRotSpeed.setRPY(ang.x.radian(), ang.y.radian(), ang.z.radian());
            mTimer = cM::rndInt(120) + 60;
        }
        if (sLib::chase(&mScale, 0.0f, 0.00075f)) {
            deleteRequest();
            return;
        }
        EGG::Quatf q;
        q.set(1.0f, 0.0f, 0.0f, 0.0f);
        mRotSpeed.slerpTo(q, 0.01f, mRotSpeed);
        mPos.x += mSpeed.x;
        mPos.z += mSpeed.z;
        mSlopeY += mSpeed.y;
        if (!(mAcch.mHitFlags & dBGCF::HIT_GROUND)) {
            mPos.y += -0.1f;
        }
        mVec3_c pos = mPos;
        dBGCF::groundChk_c water(&pos, dBGCF::LAYER_WATER, 0, 0);
        pos.y = water.mWaterY;
        mVec3_c scale(mScale, mScale, mScale);
        if (mEffect.create("af_fg_snowball_moya", &pos, NULL, &scale, 0)) {
            if (isInFallBasin(&pos)) {
                sLib::chase(&mAlpha, 0, 10);
            } else {
                sLib::chase(&mAlpha, 0xFF, 10);
            }
            mEffect.setRegisterAlpha(mAlpha, 0xFF, 1);
        }
    }
    calcRotation();
    f32 radius = getRadius();
    mAcch.check(radius, &mPos, &mLastPos, mAngle.y, (int)this,
                dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS |
                    dBGCF::CHECK_NO_WAVE_WATER,
                TRUE);
}

// 80183AA8
void daSnowMan_c::finalSink() {
}

// 80183AAC
void daSnowMan_c::initJump() {
    mVec3_c pos = getOther()->mPos;
    dFdBase_c::snapToUnitGround(&mTargetPos, &pos);
    mStartPos = mPos;
    f32 otherScale = getOther()->mScale;
    mTargetPos.y =
        mTargetPos.y + 2.0f * (15.5f * otherScale) - 0.25f * (15.5f * mScale);
    mVec3_c d = mTargetPos - mStartPos;
    mSpeedF = cJumpRate * d.xzLen();
    mSound.startSound(0x17E6);
    mTimer = cJumpFrames;
}

// 80183BE0
void daSnowMan_c::executeJump() {
    if (--mTimer != 0) {
        f32 t = cJumpRate * (cJumpFrames - mTimer);
        f32 x = 2.0f * t - 1.0f;
        f32 h = 1.0f - x * x;
        EGG::Quatf q;
        lerpPos(t);
        mPos.y += 48.0f * h;
        q.set(1.0f, 0.0f, 0.0f, 0.0f);
        mRotation.slerpTo(q, 1.0f / mTimer, mRotation);
    } else {
        mPos = mTargetPos;
        mRotation.set(1.0f, 0.0f, 0.0f, 0.0f);
        mSound.startSound(0x17EA);
        changeState(STATE_TALK);
    }
}

// 80183DDC
void daSnowMan_c::finalJump() {
    mWobbleAngle = 0;
    mWobble = 1.0f;
    mKeep = false;
    mpMdl = &mFaceMdl;
    mFgSet = save(mScale, getOther()->mScale);
    if (mFace == 0) {
        sendMail();
    }
}

// 80183E5C
void daSnowMan_c::initSettle() {
    mVec3_c pos = mPos;
    dFdBase_c::snapToUnitGround(&mTargetPos, &pos);
    mStartPos = mPos;
    mVec3_c d = mTargetPos - mStartPos;
    mSpeedF = cJumpRate * d.xzLen();
    mWeight = 0xFF;
    mTimer = cJumpFrames;
}

// 80183F10
void daSnowMan_c::executeSettle() {
    f32 radius = getRadius();
    if (--mTimer != 0) {
        f32 t = cJumpRate * (cJumpFrames - mTimer);
        lerpPos(t);
        mVec3_c move = mPos - mLastPos;
        f32 rot = move.xzLen() / radius;
        mVec3_c axis(mVec3_c::Ey.y * move.z - mVec3_c::Ey.z * move.y,
                     mVec3_c::Ey.z * move.x - mVec3_c::Ey.x * move.z,
                     mVec3_c::Ey.x * move.y - mVec3_c::Ey.y * move.x);
        if (axis.normalizeRS()) {
            mRotSpeed.setAxisRotation(axis, rot);
            EGG::Quatf rotation;
            dBalloonString::multiplyQuaternion(&rotation, &mRotSpeed,
                                               &mRotation);
            mRotation = EGG::Quatf(rotation);
            mRotation.normalise();
        }
    } else {
        mPos = mTargetPos;
        changeState(STATE_BASE);
    }
    mObjc.set(this, radius, 2.0f * radius, dObjc::TYPE_SNOWMAN, dObjc::MASK_DEFAULT, mWeight, 0);
    mObjc.mParam = 0;
    mObjc.regist();
}

// 801841B4
void daSnowMan_c::finalSettle() {
    mKeep = false;
    mWobbleAngle = 0;
    mWobble = 1.0f;
}

// 801841CC
void daSnowMan_c::initBuilt() {
}

// 801841D0
void daSnowMan_c::executeBuilt() {
    mPos = mTargetPos;
    if (!mFgSet) {
        mFgSet = setFg();
    }
}

// 80184224
void daSnowMan_c::finalBuilt() {
}

// 80184228
void daSnowMan_c::initBase() {
}

// 8018422C
void daSnowMan_c::executeBase() {
    registObjc(15.5f * mScale);
}

// 8018429C
void daSnowMan_c::finalBase() {
}

// 801842A0
void daSnowMan_c::initSnowman() {
}

// 801842A4
void daSnowMan_c::executeSnowman() {
    registObjc(15.5f * mBodyScale);
}

// 80184314
void daSnowMan_c::finalSnowman() {
}

// 80184318
void daSnowMan_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_snowman");
    setMessageCode((mJustBuilt ? 1 : 2) + mFace * 2);
    dHmnName::Word_c name;
    dString::loadString(&name, dNpcInfo::SP_NAME_SNOWMAN, "sys_STRING/STR_SPNpc_name");
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    getController()->requestStart(0);
}

// 801843EC
void daSnowMan_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            changeState(STATE_TALK_END);
        }
    }
    if (mParam >= SNOWMAN) {
        registObjc(15.5f * mBodyScale);
    }
}

// 80184524
void daSnowMan_c::finalTalk() {
}

// 80184528
void daSnowMan_c::initTalkEnd() {
    mVec3_c target = mPos;
    if (mParam >= SNOWMAN) {
        target.y += 15.5f * (mHeadScale + 2.0f * mBodyScale);
    } else {
        target.y += getRadius();
    }
    dCamera_c::sInstance->startTalkView(&target);
}

// 801845B0
void daSnowMan_c::executeTalkEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
    if (mParam >= SNOWMAN) {
        registObjc(15.5f * mBodyScale);
    }
}

// 8018468C
void daSnowMan_c::finalTalkEnd() {
    dCamera_c::sInstance->mRequestedMode = dCamera_c::sInstance->mDefaultMode;
    mJustBuilt = false;
}

static inline void blendSlope(mVec3_c *out, const mVec3_c &a, f32 ka,
                              const mVec3_c &b, f32 kb) {
    *out = ka * a + kb * b;
}

static inline f32 getDistanceSquaredXZ(const mVec3_c &a, const mVec3_c &b) {
    return (a - b).xzLenSq();
}

// 801846A4
BOOL daSnowMan_c::checkGround() {
    dItem::Item *item = dField::getInfo(0)->getItem(&mPos, 0);
    if (item != NULL && isHole(item)) {
        mVec3_c center;
        dFdBase_c::snapToUnitGround(&center, &mPos);
        f32 distSq = getDistanceSquaredXZ(center, mPos);
        f32 holeRadiusSq = 16.0f * 16.0f;
        if (distSq < holeRadiusSq) {
            mOnSlope = true;
            mVec3_c dir = center - mPos;
            mVec3_c up = mVec3_c::Ey;
            dir.y = 1.7320508f * dir.xzLen();
            if (!dir.normalizeRS()) {
                dir = mVec3_c::Ey;
            }
            f32 t = mRollDist / 2560.0f;
            f32 depth =
                t * l_param.mHoleDepthMax + (1.0f - t) * l_param.mHoleDepthMin;
            f32 w = nw4r::math::FSqrt(distSq) / 16.0f;
            f32 rest = 1.0f - w;
            blendSlope(&mSlopeNormal, up, rest, dir, w);
            mSlopeNormal.normalize();
            mSlopeY = depth * rest;
            if (distSq < 2.0f) {
                return FALSE;
            }
        } else {
            mOnSlope = false;
            mSlopeY = 0.0f;
        }
    } else {
        mOnSlope = false;
        mSlopeY = 0.0f;
    }
    f32 groundY = dBGCF::getGroundY(&mPos, FALSE);
    f32 dy = mPos.y - groundY;
    if (dy < 1.0f) {
        if (dy < -10.0f) {
            return FALSE;
        }
        mPos.y = groundY;
    }
    f32 radius = getRadius();
    mAcch.mStepHeight = 0.5f;
    mAcch.check(radius, &mPos, &mLastPos, mAngle.y, (int)this,
                dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS |
                    dBGCF::CHECK_NO_WAVE_WATER,
                TRUE);
    if (mAcch.mHitFlags & dBGCF::HIT_GROUND) {
        mGroundPos = mPos;
        mAccelY = getGravity();
        mMaxSpeedY = l_param.mMaxFall;
    } else {
        if (mAcch.mPrevHitFlags & dBGCF::HIT_GROUND) {
            mAccelY = 0.0f;
            mPos.y = mLastPos.y;
            dBGCF::acch_c acch;
            if (!mAcch.isWallHit()) {
                dBGCF::acch_c *collision = &acch;
                mVec3_c pos(mPos);
                pos.y -= radius;
                collision->check(radius, &pos, &mPos, mAngle.y, 0,
                                 dBGCF::CHECK_WALL, TRUE);
                if (collision->isWallHit()) {
                    mVec3_c normal;
                    if (getWallNormal(collision, &normal)) {
                        if (!mOnWall) {
                            mSpeed += 0.25f * normal;
                        }
                        mOnWall = true;
                        mWallNormal = normal;
                    }
                }
            }
        }
        if (mOnWall) {
            mVec3_c slope;
            f32 radius = 15.5f * mScale;
            mVec3_c d = mPos - mGroundPos;
            f32 t = d.dot(mWallNormal) / radius;
            if (t > 1.0f || t < 0.0f) {
                mOnWall = false;
                mOnSlope = false;
                return TRUE;
            }
            f32 k = getSlopeRate(t);
            slope = k * mWallNormal;
            slope.y = 1.0f - k;
            if (slope.normalizeRS()) {
                mOnSlope = true;
                mSlopeNormal = slope;
            }
        }
        sLib::addCalc2(&mAccelY, l_param.mGravityAir, 0.5f, 0.005f);
        mMaxSpeedY = l_param.mMaxFall;
    }
    return TRUE;
}

// 80184C14
BOOL daSnowMan_c::getGroundNormal(mVec3_c *normal) {
    if (mAcch.mHitFlags & dBGCF::HIT_GROUND) {
        if (mOnSlope) {
            *normal = mSlopeNormal;
        } else {
            *normal = mAcch.mGroundNormal;
        }
        mOnWall = false;
        return TRUE;
    }
    if (mOnSlope) {
        *normal = mSlopeNormal;
        return TRUE;
    }
    return FALSE;
}

static inline BOOL isHoleItem(dItem::Item *item) {
    BOOL hole = FALSE;
    if (item != NULL && isHole(item)) {
        hole = TRUE;
    }
    return hole;
}

static inline BOOL isBlocked(dItem::Item *item, int x, int z) {
    BOOL column = FALSE;
    if (dBGCF::getColumnAttr(x, z, NULL, NULL, NULL) ||
        (item != NULL && item->isSnowman())) {
        column = TRUE;
    }
    BOOL blocked = FALSE;
    if (column && !isHoleItem(item)) {
        blocked = TRUE;
    }
    return blocked;
}

// 80184CA4
BOOL daSnowMan_c::canPlace(int x, int z) {
    dFdBase_c *field = getCurrentField();
    int playerX = -1;
    int playerZ = -1;
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player != NULL) {
        playerX = worldToUnit(player->mPos.x);
        playerZ = worldToUnit(player->mPos.z);
    }
    dItem::Item *item = field->getItem(x, z, 0);
    dFdUnitAttr_c *attr = field->getUnitAttr();
    if ((attr != NULL && (attr->getAttr(x, z) & dFdUnitAttr_c::STR_COL)) ||
        dBGCF::getRouteDirs(x, z) != 0) {
        return FALSE;
    }
    BOOL blocked = isBlocked(item, x, z);
    bool soil = field->getDigType(x, z) == 0;
    if ((x != playerX || z != playerZ) && dBGCF::canPutItem(x, z) && !blocked &&
        soil) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx != 0 || dz != 0) {
                    int ux = x + dx;
                    int uz = z + dz;
                    dItem::Item *around = field->getItem(ux, uz, 0);
                    BOOL aroundBlocked = isBlocked(around, ux, uz);
                    if (!dBGCF::canPutItem(ux, uz) ||
                        (attr != NULL &&
                         (attr->getAttr(ux, uz) & dFdUnitAttr_c::STR_COL)) ||
                        aroundBlocked) {
                        return FALSE;
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 80184F78
BOOL daSnowMan_c::setFg() {
    BOOL ret = FALSE;
    dFdBase_c *field = getCurrentField();
    int x = worldToUnit(mPos.x);
    int z = worldToUnit(mPos.z);
    dItem::Item *item = field->getItem(x, z, 0);
    if (item != NULL && item->mId == dItem::ITEM_ID_NONE) {
        dItem::Item fg(getFgId());
        ret = field->setItem(&fg, x, z, 0);
    }
    return ret;
}

static inline int getFreeSlot() {
    const dSvDateEvent::Event_c *records = dSaveData_c::getTown()->mSnowmen;
    for (int i = 0; i < dSvDateEvent::SNOWMAN_NUM; i++) {
        if (!records[i].mValid) {
            return i;
        }
    }
    return dSvDateEvent::SNOWMAN_NUM;
}

// 80185064
BOOL daSnowMan_c::save(f32 headScale, f32 bodyScale) {
    BOOL ret = FALSE;
    if (dField::getInfo(0) != NULL) {
        int slot = getFreeSlot();
        mSlot = slot;
        dSvDateEvent::Event_c *rec = &dSaveData_c::getTown()->mSnowmen[slot];
        rec->set(headScale, bodyScale);
        mFace = getFace(toScale(rec->mScale1), toScale(rec->mScale2));
        ret = setFg();
    }
    return ret;
}

// 801851B0
BOOL daSnowMan_c::placeFg(int x, int z) {
    if (canPlace(x, z)) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item fg(getFgId());
            dItem::Item *item = field->getItem(x, z, 0);
            dItem::Item old;
            if (item != NULL) {
                old = *item;
            }
            if (field->setItem(&fg, x, z, 0)) {
                lose(&old);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 801852C0
BOOL daSnowMan_c::placeFgAround(int x, int z) {
    static const s16 l_around[][2] = {
        {0, 1}, {1, 0}, {-1, 0}, {0, -1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1},
    };
    for (u32 i = 0; i < 8; i++) {
        if (placeFg(x + l_around[i][0], z + l_around[i][1])) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80185340
BOOL daSnowMan_c::lose(const dItem::Item *item) {
    if (item->isSame(dItem::Item())) {
    } else if (dItem::isRealItemId(item->mId) || item->hasFtrFunc()) {
        dSaveData_c::addToPoliceBox(*item);
        return TRUE;
    }
    return FALSE;
}

// 801853E4
BOOL daSnowMan_c::checkBreak(const mVec3_c *pos, f32 radius) {
    if (isState(STATE_SINK) || isState(STATE_ROLL)) {
        mVec3_c center(mPos.x, mPos.y + mSlopeY, mPos.z);
        f32 r = getRadius();
        center.y += r;
        mVec3_c d = center - *pos;
        f32 dist = radius + r;
        if (nw4r::math::VEC3LenSq(&d) < dist * dist) {
            doBreak();
            return TRUE;
        }
    }
    return FALSE;
}

// 801854C0
BOOL daSnowMan_c::breakBall() {
    if (isState(STATE_SINK) || isState(STATE_ROLL)) {
        doBreak();
        return TRUE;
    }
    return FALSE;
}

// 80185500
void daSnowMan_c::doBreak() {
    mVec3_c pos = mPos;
    mVec3_c scale(mScale, mScale, mScale);
    pos.y += getRadius();
    dEffect::create("af_fg_snowball_break", &pos, NULL, &scale);
    mKeep = false;
    mSound.startSound(0x17E7);
    deleteRequest();
}

// 80185598
void daSnowMan_c::splashCallback(nw4r::ef::Particle *ptcl, u32 kind) {
    static const char *const l_names[] = {"af_hny_hamon_S01",
                                          "afi_hny_watersplash_b"};
    if (ptcl != NULL) {
        mVec3_c pos;
        mVec3_c ptclPos;
        ptclPos = ptcl->mParameter.mPosition;
        dWorld::toFieldPosition(&pos, &ptclPos);
        dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 0, 0);
        if (chk.mWater != 0) {
            if (chk.isUnderWater(pos.y)) {
                pos.y = chk.mWaterY;
                dEffect::create(l_names[kind], &pos, NULL, NULL);
                if (ptcl->mParticleManager != NULL) {
                    ptcl->mParticleManager->RetireParticle(ptcl);
                }
            }
        } else if (pos.y <= chk.getHeight(FALSE)) {
            pos.y = chk.getHeight(FALSE);
            if (kind == 1) {
                dEffect::create("afi_hny_watersplash_a", &pos, NULL, NULL);
            }
            if (ptcl->mParticleManager != NULL) {
                ptcl->mParticleManager->RetireParticle(ptcl);
            }
        }
    }
}

// 801856CC
void daSnowMan_c::setSplashEffect(const mVec3_c *pos) {
    mVec3_c scale(mScale, mScale, mScale);
    dEffect::create("af_fg_snowball_wc", pos, NULL, &scale);
    u32 big = mScale > 1.0f;
    dEffect_c drops;
    drops.createEffect("af_hny_mizutama_L2", pos, NULL, NULL, splashCallback,
                       big);
    f32 t = (mScale - 0.5f) / 0.75f;
    f32 rate;
    if (t < 0.0f) {
        rate = 0.0f;
    } else if (t > 1.0f) {
        rate = 1.0f;
    } else {
        rate = t;
    }
    drops.vf38(0.6f + (1.0f - 0.6f) * rate);
    drops.vf3C(2.0f + 1.2f * rate);
    f32 size = 1.0f + 0.6f * rate;
    nw4r::math::VEC2 scale2(size, size);
    drops.vf5C(&scale2);
    drops.EGG::Effect::update();
}

// 80185850
void daSnowMan_c::calcMtx() {
    if (mParam >= SNOWMAN) {
        mVec3_c pos = mPos;
        f32 bodyRadius = 15.5f * mBodyScale;
        f32 headRadius = 15.5f * mHeadScale;
        mMtx_c bodyMtx;
        pos.y += bodyRadius;
        dActor_c::makeMtx(&bodyMtx, &pos, 0);
        mBodyMdl.setLocalMtx(&bodyMtx);
        mBodyMdl.setScale(mBodyScale, mBodyScale, mBodyScale);
        pos.y += bodyRadius + 0.75f * headRadius;
        mMtx_c headMtx;
        dActor_c::makeMtx(&headMtx, &pos, 0);
        mFaceMdl.setLocalMtx(&headMtx);
        mFaceMdl.setScale(mHeadScale, mHeadScale, mHeadScale);
    } else {
        mMtx_c mtx;
        mMtx_c rotMtx;
        fn_80444FE4(&rotMtx, &mRotation);
        if (isState(STATE_SINK) || isState(STATE_ROLL)) {
            mVec3_c center(mPos.x, mPos.y + mSlopeY, mPos.z);
            center.y += getRadius();
            dActor_c::makeMtx(&mtx, &center, 0);
            PSMTXConcat(mtx, rotMtx, mtx);
            mpMdl->setLocalMtx(&mtx);
            mpMdl->setScale(mScale, mScale, mScale);
        } else {
            sLib::addCalc0(&mWobble, 0.1f, 0.15f);
            mWobbleAngle += 0xE39;
            f32 amp = 0.05f * (4.0f * ((1.0f - mWobble) * mWobble));
            f32 scaleY = 1.0f + amp * mWobbleAngle.cos();
            f32 scaleXZ = 1.0f + amp * mWobbleAngle.sin();
            f32 ofs = scaleY * getRadius();
            if (mpMdl == &mFaceMdl && getOther() != NULL) {
                f32 otherScale = getOther()->mScale;
                ofs -= (1.0 - scaleY) * (15.5f * otherScale);
            }
            mVec3_c pos(mPos.x, mPos.y, mPos.z);
            pos.y += ofs;
            dActor_c::makeMtx(&mtx, &pos, 0);
            mMtx_c scaleMtx;
            PSMTXScale(scaleMtx, scaleXZ, scaleY, scaleXZ);
            PSMTXConcat(mtx, scaleMtx, mtx);
            PSMTXConcat(mtx, rotMtx, mtx);
            mpMdl->setLocalMtx(&mtx);
            mpMdl->setScale(mScale, mScale, mScale);
        }
    }
}

// 80185B9C
mVec3_c *daSnowMan_c::getDemoPosition() {
    if (mState == STATE_BUILT && getOther() != NULL) {
        return &getOther()->mPos;
    }
    return &mPos;
}

// 80185C04
bool daSnowMan_c::canInteract(dDemoActor_c *actor) {
    if (!(isState(STATE_SNOWMAN) || isState(STATE_BUILT) ||
          isState(STATE_JUMP))) {
        return false;
    }
    clearDemoState();
    if (actor == NULL) {
        return false;
    }
    if (isState(STATE_TALK_END) || isState(STATE_TALK)) {
        return false;
    }
    return true;
}

// 80185C94
void daSnowMan_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        changeState(STATE_TALK);
        break;
    case DEMO_CMD_END:
        if (mParam >= SNOWMAN) {
            changeState(STATE_SNOWMAN);
        } else {
            changeState(STATE_BUILT);
        }
        break;
    }
}

struct daSnowManUnitParam_c {
    /* 0x0 */ u8 mCheckSouth;
    /* 0x1 */ u8 mNeedEmpty;
};

// 80185E4C
BOOL daSnowMan_c::checkUnit(int blockX, int blockZ, int x, int z, void *user) {
    const daSnowManUnitParam_c *param = (const daSnowManUnitParam_c *)user;
    dFdBase_c *field = dField::getInfo(0);
    int unitX = (blockX << 4) + x;
    int unitZ = (blockZ << 4) + z;
    if (field->_24->isGroundFree(unitX, unitZ) &&
        field->getPlantType(blockX, blockZ, x, z) != 1 &&
        field->getPlantType(blockX, blockZ, x, z) != 0) {
        if (param->mNeedEmpty) {
            dItem::Item *item = field->getItem(unitX, unitZ, 0);
            if (item == NULL || item->mId != dItem::ITEM_ID_NONE) {
                return FALSE;
            }
        }
        if (!dBGCF::getColumnAttr(unitX, unitZ, NULL, NULL, NULL)) {
            if (param->mCheckSouth) {
                if (isBlockedUnit(field, unitX, ++unitZ) ||
                    isPondEdgeUnit(field, unitX, unitZ)) {
                    return FALSE;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80185FB0
BOOL daSnowMan_c::pushAll(f32 speed, mVec3_c *pos, s16 *angle, f32 *speedF,
                          mAng dir) {
    for (int i = 0; i < BALL_NUM; i++) {
        if (sBall[i] != NULL &&
            sBall[i]->push(speed, pos, angle, speedF, dir)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80186064
BOOL daSnowMan_c::isAtUnit(int x, int z) {
    for (int i = 0; i < BALL_NUM; i++) {
        daSnowMan_c *ball = sBall[i];
        if (ball != NULL) {
            int ballX = worldToUnit(ball->mPos.x);
            int ballZ = worldToUnit(ball->mPos.z);
            if (ballX == x && ballZ == z) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80186110
bool daSnowMan_c::find(mVec3_c *pos, int *idx, void *user, FindFunc func) {
    for (int i = 0; i < BALL_NUM; i++) {
        daSnowMan_c *ball = getBall(i);
        if (ball != NULL && ball->canRoll() && func != NULL &&
            func(&ball->mPos, i, user)) {
            *pos = ball->mPos;
            *idx = i;
            return true;
        }
    }
    return false;
}

// 801861D0
BOOL daSnowMan_c::checkBreakAll(const mVec3_c *pos, f32 radius) {
    BOOL ret = FALSE;
    for (int i = 0; i < BALL_NUM; i++) {
        if (sBall[i] != NULL && sBall[i]->checkBreak(pos, radius)) {
            ret = TRUE;
        }
    }
    return ret;
}

// 80186260
BOOL daSnowMan_c::placeBalls() {
    if (!dScene::isCurrentSceneAttr(5)) {
        return FALSE;
    }
    if (!dScene::isWinterSeasonType()) {
        return FALSE;
    }
    if (dNet::isSessionActive()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    for (int i = 0; i < dSvDateEvent::SNOWMAN_NUM; i++) {
        dSvDateEvent::Event_c *rec = &dSaveData_c::getTown()->mSnowmen[i];
        if (rec->mValid && rec->diffDaysAdjusted(&now) == 0) {
            for (int j = 0; j < BALL_NUM; j++) {
                sBallInfo[j].mActive = false;
            }
            return FALSE;
        }
    }
    int width = dField::getInfo(0)->mBlockW;
    int height = dField::getInfo(0)->mBlockH;
    dFdBkSearchCand_c acres(width, height);
    acres.setAll();
    acres.removeBorder();
    dFdUnitAttr_c *attr = dField::getInfo(0)->_24;
    for (int i = 0; i < BALL_NUM; i++) {
        ball_c *info = &sBallInfo[i];
        if (info->mActive) {
            mVec3_c pos = info->mPos;
            int x = worldToUnit(pos.x);
            int z = worldToUnit(pos.z);
            if (attr->getAttr(x, z)) {
                info->mActive = false;
            } else {
                acres.remove(((int)pos.z >> 9) * acres.mWidth +
                             ((int)pos.x >> 9));
            }
        }
    }
    f32 scale = dSvDateEvent::cScaleMin;
    BOOL firstTry = TRUE;
    for (int i = 0; i < BALL_NUM; i++) {
        ball_c *info = &sBallInfo[i];
        BOOL placed = FALSE;
        if (!info->mActive) {
            for (;;) {
                int blockX;
                int blockZ;
                while (acres.getRandomXZ(&blockX, &blockZ)) {
                    acres.remove(blockZ * acres.mWidth + blockX);
                    daSnowManUnitParam_c param;
                    param.mCheckSouth = TRUE;
                    param.mNeedEmpty = firstTry;
                    dBkUnitSearchCandCb_c units(blockX, blockZ, checkUnit,
                                                &param);
                    units.clear();
                    units.search();
                    int x;
                    int z;
                    if (units.getRandomXZ(&x, &z)) {
                        mVec3_c pos;
                        dFdBase_c::getUnitGroundPos(&pos, blockX, blockZ, x, z);
                        info->mActive = true;
                        placed = TRUE;
                        info->mScale = scale;
                        info->mPos = pos;
                        break;
                    }
                }
                if (placed || !firstTry) {
                    break;
                }
                firstTry = FALSE;
                acres.setAll();
                acres.removeBorder();
                for (int j = 0; j < i; j++) {
                    mVec3_c pos = sBallInfo[j].mPos;
                    acres.remove(((int)pos.z >> 9) * acres.mWidth +
                                 ((int)pos.x >> 9));
                }
            }
        }
    }
    for (int i = 0; i < BALL_NUM; i++) {
        ball_c *info = &sBallInfo[i];
        if (info->mActive) {
            mVec3_c pos = info->mPos;
            dActor_c::construct(fProf::SNOWMAN, dScene::getSceneParent(), i,
                                &pos, NULL);
        }
    }
    return TRUE;
}

// 80186708
BOOL daSnowMan_c::placeSnowmen() {
    if (!dScene::isCurrentSceneAttr(5)) {
        return FALSE;
    }
    if (!dScene::isWinterSeasonType()) {
        return FALSE;
    }
    dFdBase_c *field = dField::getInfo(0);
    int width = field->mBlockW;
    int height = field->mBlockH;
    for (int blockX = 0; blockX < width; blockX++) {
        for (int blockZ = 0; blockZ < height; blockZ++) {
            for (int x = 0; x < UT_X_NUM; x++) {
                for (int z = 0; z < UT_Z_NUM; z++) {
                    dItem::Item *item = field->getItem(blockX, blockZ, x, z, 0);
                    if (item != NULL && item->isSnowman()) {
                        int idx = item->getSnowmanIdx();
                        dSvDateEvent::Event_c *rec =
                            &dSaveData_c::getTown()->mSnowmen[idx];
                        mVec3_c pos;
                        dFdBase_c::getUnitGroundPos(&pos, blockX, blockZ, x, z);
                        dActor_c::construct(fProf::SNOWMAN,
                                            dScene::getSceneParent(),
                                            idx + SNOWMAN, &pos, NULL);
                    }
                }
            }
        }
    }
    return TRUE;
}

static inline dSvDateEvent::Event_c *getSnowman(int i) {
    return &dSaveData_c::getTown()->mSnowmen[i];
}

// 80186838
void daSnowMan_c::removeOld() {
    bool removed[dSvDateEvent::SNOWMAN_NUM] = {false, false, false};
    for (int i = 0; i < dSvDateEvent::SNOWMAN_NUM; i++) {
        dSvDateEvent::Event_c *rec = getSnowman(i);
        if (rec->mValid) {
            int days = rec->diffDays(dTime_c::getCurrent());
            if (days >= dSvDateEvent::SNOWMAN_NUM || days < 0) {
                rec->clear();
                removed[i] = true;
            }
        }
    }
    dFdBase_c *field = dField::getInfo(1);
    int width = field->mBlockW;
    int height = field->mBlockH;
    for (int blockX = 0; blockX < width; blockX++) {
        for (int blockZ = 0; blockZ < height; blockZ++) {
            for (int x = 0; x < UT_X_NUM; x++) {
                for (int z = 0; z < UT_Z_NUM; z++) {
                    dItem::Item *item = field->getItem(blockX, blockZ, x, z, 0);
                    if (item != NULL && item->isSnowman() &&
                        removed[item->getSnowmanIdx()]) {
                        dItem::Item none;
                        field->setItem(&none, blockX, blockZ, x, z, 0);
                    }
                }
            }
        }
    }
}

// 801869B0
void daSnowMan_c::clearBalls() {
    for (int i = 0; i < BALL_NUM; i++) {
        sBallInfo[i].mActive = false;
    }
}

// 801869C8
int daSnowMan_c::getFace(f32 headScale, f32 bodyScale) {
    f32 d = std::fabs(headScale / bodyScale - 0.85f);
    if (d <= 0.05f) {
        return 0;
    }
    if (d <= 0.15f) {
        return 1;
    }
    if (d <= 0.25f) {
        return 2;
    }
    return 3;
}
