// The UFO (REL d_a_ufoNP). See include/game/game/d_a_ufo.hpp.
// REL .text 0xCC..0x1528, .ctors 0x0..0x4, .rodata 0x0..0xE0, .data
// 0x0..0x210, .bss 0x8..0x30C.
#include <game/game/d_a_ufo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_save_visitor_npc.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_world.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/math/eggMath.h>
#include <cstring>

// The appearance check (executeHidden): at these minutes, with mChance in
// UFO_CHANCE_NUM (one more each miss, at most all but one).
#define UFO_MINUTE_A 2
#define UFO_MINUTE_B 7
#define UFO_CHANCE_NUM 8
// executeTurn: the wait at the route's end, in frames.
#define UFO_TURN_WAIT 300
// executeShot: frames before it flies off.
#define UFO_SHOT_WAIT 20
// executeFly: how far ahead it looks at the ground.
#define UFO_LOOK_AHEAD 128.0f
// The bob (executeState): its phase speed and amplitudes.
#define UFO_BOB_SPEED 0x200
#define UFO_BOB_AMP 5.0f
#define UFO_BOB_AMP_LEAVE 1.4f
// The wobble after the shot (initWobble), halved at each swing.
#define UFO_WOBBLE_AMP 4096.0f
#define UFO_WOBBLE_SPEED 0x900

// Debug parameters (the HIO tool's values).
class hostIO_c {
  public:
    hostIO_c() {
        mForceAppear = 0;
        mFlySpeed = 1.372f;
        mLeaveSpeed = 0.704f;
        mScale = 0.6f;
    }
    virtual ~hostIO_c() {
    }

    /* 0x04 */ u8 mForceAppear; // appear at the next check
    /* 0x08 */ f32 mFlySpeed;
    /* 0x0C */ f32 mLeaveSpeed; // after the shot
    /* 0x10 */ f32 mScale;
}; // size 0x14

// .text:0xCC
namespace dAUfo {
BASE_PROFILE(A_UFO, daUfo_c);
} // namespace dAUfo

static hostIO_c l_hostIO;

daUfo_c *daUfo_c::s_instance;

static u8 l_ufoBuf[sizeof(daUfo_c)] ALIGN(32);

// .text:0x1DC: the slingshot's shot hit the UFO.
static void hitCallback(dObjc::base_c *other) {
    const mVec3_c *pos = other->getPos();
    if (daUfo_c::s_instance != NULL) {
        daUfo_c::s_instance->mHitOffset = *pos - daUfo_c::s_instance->getPos();
        daUfo_c *ufo = daUfo_c::s_instance;
        f32 max = ufo->mRadius - 11.2f;
        if (std::fabs(ufo->mHitOffset.x) >= max) {
            if (ufo->mHitOffset.x >= 0.0f) {
                ufo->mHitOffset.x = max;
            } else {
                ufo->mHitOffset.x = -max;
            }
        }
    }
}

// .text:0x29C
int daUfo_c::create() {
    mRaccoIdx = -1;
    nw4r::g3d::ResFile file(dSTR::getBrresBank()->getBrres("ufo0"));
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mAllocator_c *allocator = dStrm_c::getAllocator();
    mMdl.create(mdl, allocator, 0, 1, NULL);
    mAnmChr.create(mdl, file.GetResAnmChr(0), allocator, NULL);
    mMdl.setAnm(mAnmChr);
    mAnmClr.create(mdl, file.GetResAnmClr("obj_ufo0_n"), allocator, NULL, 1);
    mMdl.setAnm(mAnmClr);
    dTime_c now = *dTime_c::getCurrent();
    mVisitorDay = dEvent::isVisitorHere(VISITOR_UFO);
    mChance = cM::rndInt(UFO_CHANCE_NUM);
    mLastMinute = now.min;
    mObjc.mHitFunc = hitCallback;
    setState(STATE_HIDDEN);
    s_instance = this;
    return SUCCEEDED;
}

// .text:0x470
int daUfo_c::execute() {
    mRadius = 48.0f * l_hostIO.mScale;
    mSnd.setPos(&mPos);
    executeState();
    mAnmChr.play();
    mAnmClr.play();
    // Hit by the slingshot's shot (mask TYPE_ONCE | 0x2000, as for the
    // balloons' type 0x4000).
    mObjc.set(this, mRadius, 32.0f * l_hostIO.mScale, 0x8000, 0x40002000, 0, 0);
    mObjc.regist();
    return SUCCEEDED;
}

static inline void requestShadow(const mVec3_c *pos, mAng angle = mAng(0)) {
    dShadow::requestRound(pos, 8, &angle, 40.0f * l_hostIO.mScale, 1.0f, 1.0f,
                          0.0f, 1.0f);
}

// .text:0x53C
int daUfo_c::draw() {
    mMdl.setScale(l_hostIO.mScale, l_hostIO.mScale, l_hostIO.mScale);
    if (mState != STATE_HIDDEN) {
        mVec3_c pos = mPos;
        pos.y += mBobAmp * mAng(mBobPhase).sin();
        mVec3_c curved;
        mAng tilt = dWorld::curvePosition(&curved, &pos);
        mMtx_c mtx;
        PSMTXTrans(mtx, curved.x, curved.y, curved.z);
        mtx.XrotM(tilt);
        mtx.ZrotM(mAng(mWobbleAmp * mAng(mWobblePhase).sin()));
        mtx.YrotM(mAngle.y);
        if (!mHitOffset.isZero()) {
            mVec3_c hit;
            mVec3_c local = mHitOffset;
            local.y += 5.3333335f;
            PSMTXMultVec(mtx, local, hit);
            dWorld::toFieldPosition(&hit, &hit);
            mHitPos = hit;
        }
        mMdl.setLocalMtx(&mtx);
        mMdl.calc(false);
        mMdl.entry();
        requestShadow(&pos);
    }
    return SUCCEEDED;
}

// .text:0x760
int daUfo_c::doDelete() {
    mMdl.remove();
    mAnmChr.remove();
    mAnmClr.remove();
    s_instance = NULL;
    return SUCCEEDED;
}

// .text:0x7D4
void *daUfo_c::operator new(size_t size) {
    memset(l_ufoBuf, 0, sizeof(l_ufoBuf));
    return l_ufoBuf;
}

// .text:0x810
void daUfo_c::operator delete(void *mem) {
}

// .text:0x814
void daUfo_c::initWobble() {
    mWobbleAmp = UFO_WOBBLE_AMP;
    mWobblePhase = 0;
    mWobbleSpeed = UFO_WOBBLE_SPEED;
}

// .text:0x834: the wobble halves at each swing.
void daUfo_c::calcWobble() {
    if (mWobbleAmp != 0.0f) {
        mAng prev = mWobblePhase;
        mWobblePhase += mWobbleSpeed;
        const s16 cur = mWobblePhase;
        if (prev.sin() * mAng(cur).sin() < 0.0f) {
            mWobbleAmp *= 0.5f;
        }
    }
}

// .text:0x8F4
void daUfo_c::calcExplosion() {
    if (mHitPos.isZero()) {
        return;
    }
    if (!mExploding) {
        mExploding = TRUE;
        mExplosion.create("af_ufo_exp", &mHitPos, NULL, NULL);
    } else {
        mExplosion.followEffect(&mHitPos, NULL, NULL);
    }
}

static const daUfo_c::InitFunc l_initFuncs[daUfo_c::STATE_NUM] = {
    &daUfo_c::initHidden, &daUfo_c::initEnter, &daUfo_c::initFly,
    &daUfo_c::initTurn,   &daUfo_c::initShot,  &daUfo_c::initLeave,
};

static const daUfo_c::ExecFunc l_execFuncs[daUfo_c::STATE_NUM] = {
    &daUfo_c::executeHidden, &daUfo_c::executeEnter, &daUfo_c::executeFly,
    &daUfo_c::executeTurn,   &daUfo_c::executeShot,  &daUfo_c::executeLeave,
};

// .text:0x974
BOOL daUfo_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_initFuncs[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x9E0: also bobs up and down (slower after the shot).
void daUfo_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*l_execFuncs[mState])();
    }
    f32 amp = mState == STATE_LEAVE ? UFO_BOB_AMP_LEAVE : UFO_BOB_AMP;
    if (mBobAmp == 0.0f) {
        mBobAmp = amp;
    }
    if (mBobSpeed == 0) {
        mBobSpeed = UFO_BOB_SPEED;
    }
    mAng prev = mBobPhase;
    mBobPhase += mBobSpeed;
    const s16 cur = mBobPhase;
    if (prev.sin() * mAng(cur).sin() < 0.0f) {
        mBobAmp = amp;
        mBobSpeed = UFO_BOB_SPEED;
    }
}

// .text:0xAFC
BOOL daUfo_c::initHidden() {
    return dFltg::remove(this);
}

// .text:0xB00
void daUfo_c::executeHidden() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (l_hostIO.mForceAppear) {
        setState(STATE_ENTER);
        l_hostIO.mForceAppear = 0;
        dSaveData_c::getTown()->setFlag(SAVE_FLAG_UFO_FLOWN);
    } else if (player != NULL) {
        dTime_c now = *dTime_c::getCurrent();
        u32 min = now.min;
        if (mVisitorDay && !dNet::isSessionActive() &&
            !player->isFlag0(PRIVATE_FLAG0_TRAVELLING) &&
            !dSaveData_c::getRaw()->isFlag(SAVE_FLAG_UFO_FLOWN) &&
            mLastMinute != min &&
            (min == UFO_MINUTE_A || min == UFO_MINUTE_B)) {
            if (cM::rndInt(UFO_CHANCE_NUM) <= mChance) {
                if (setState(STATE_ENTER)) {
                    dSaveData_c::getTown()->setFlag(SAVE_FLAG_UFO_FLOWN);
                }
            } else {
                mChance++;
                if (mChance >= UFO_CHANCE_NUM) {
                    mChance = UFO_CHANCE_NUM - 1;
                }
            }
        }
        mLastMinute = min;
    }
}

// .text:0xC84
BOOL daUfo_c::initEnter() {
    mExploding = FALSE;
    mHitOffset.set(0.0f, 0.0f, 0.0f);
    mHitPos.set(0.0f, 0.0f, 0.0f);
    return TRUE;
}

// .text:0xCB4
void daUfo_c::executeEnter() {
    mFlyNum = 0;
    if (cM::rndInt(2) == 0) {
        mPos.x = dFdBase_c::getRaccoParamB();
        mEndX = dFdBase_c::getRaccoParamA();
        mToRight = FALSE;
    } else {
        mPos.x = dFdBase_c::getRaccoParamA();
        mEndX = dFdBase_c::getRaccoParamB();
        mToRight = TRUE;
    }
    mRaccoIdx = dFdBase_c::getOtherRaccoIdx(getRaccoIdx());
    mRouteZ = mPos.z = dFdBase_c::getRaccoDist(mRaccoIdx);
    mPos.y = dFltg::cFlyHeight + dBGCF::getEdgeGroundY(&mPos);
    dFltg::add(this);
    setState(STATE_FLY);
}

// .text:0xD80
BOOL daUfo_c::initFly() {
    mFlyNum++;
    return TRUE;
}

// .text:0xD94
void daUfo_c::executeFly() {
    static const f32 cClimbSpeed = 0.8f;
    f32 speed = l_hostIO.mFlySpeed;
    if (mToRight) {
        mPos.x += speed;
        if (mPos.x > mEndX) {
            if (mFlyNum <= 1) {
                setState(STATE_TURN);
            } else {
                setState(STATE_HIDDEN);
            }
        }
    } else {
        mPos.x -= speed;
        if (mPos.x < mEndX) {
            if (mFlyNum <= 1) {
                setState(STATE_TURN);
            } else {
                setState(STATE_HIDDEN);
            }
        }
    }
    mVec3_c ahead = mPos;
    if (mToRight) {
        ahead.x += UFO_LOOK_AHEAD;
    } else {
        ahead.x -= UFO_LOOK_AHEAD;
    }
    f32 height = dFltg::cFlyHeight + dBGCF::getEdgeGroundY(&ahead);
    if (height > mPos.y) {
        mPos.y += cClimbSpeed;
        if (mPos.y > height) {
            mPos.y = height;
        }
    } else {
        mVec3_c here = mPos;
        height = dFltg::cFlyHeight + dBGCF::getEdgeGroundY(&here);
        if (height < mPos.y) {
            mPos.y -= cClimbSpeed;
            if (mPos.y < height) {
                mPos.y = height;
            }
        }
    }
    mSnd.holdSound(SE_sky_UFO_fly);
    if (mObjc.mHitType != 0) {
        setState(STATE_SHOT);
    }
}

// .text:0xF4C
BOOL daUfo_c::initTurn() {
    mTurnWait = cM::rndInt(UFO_TURN_WAIT) + UFO_TURN_WAIT;
    return TRUE;
}

// .text:0xF88
void daUfo_c::executeTurn() {
    if (mTurnWait != 0) {
        mTurnWait--;
    }
    if (mTurnWait == 0 && setState(STATE_FLY)) {
        if (mToRight) {
            mEndX = dFdBase_c::getRaccoParamA();
            mToRight = FALSE;
        } else {
            mEndX = dFdBase_c::getRaccoParamB();
            mToRight = TRUE;
        }
        mRaccoIdx = dFdBase_c::getOtherRaccoIdx(getRaccoIdx());
        mRouteZ = mPos.z = dFdBase_c::getRaccoDist(mRaccoIdx);
        mPos.y = dFltg::cFlyHeight + dBGCF::getEdgeGroundY(&mPos);
    }
}

// .text:0x1054
BOOL daUfo_c::initShot() {
    if (dPlayerMgr::watchShotUfo()) {
        mSnd.startSound(SE_sky_UFO_hit);
        mShotWait = UFO_SHOT_WAIT;
        nw4r::g3d::ResFile file(dSTR::getBrresBank()->getBrres("ufo0"));
        mAnmClr.setAnm(mMdl, file.GetResAnmClr("obj_ufo0_w"), 0,
                       m3d::FORWARD_LOOP);
        mMdl.setAnm(mAnmClr);
        initWobble();
        int unitX;
        int unitZ;
        dBGCF::posToUnit(&unitX, &unitZ, &mPos);
        if (dFgobjManager::dropUfoPart(unitX, unitZ, mToRight)) {
            dSaveData_c::getTown()->setFlag(SAVE_FLAG_UFO_SHOT);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x113C
void daUfo_c::executeShot() {
    if (mShotWait != 0) {
        mShotWait--;
    }
    if (mShotWait == 0) {
        setState(STATE_LEAVE);
    }
    calcExplosion();
    calcWobble();
}

// .text:0x11A0
BOOL daUfo_c::initLeave() {
    return TRUE;
}

// .text:0x11A8
void daUfo_c::executeLeave() {
    calcWobble();
    f32 speed = l_hostIO.mLeaveSpeed;
    if (mToRight) {
        mPos.x += speed;
    } else {
        mPos.x -= speed;
    }
    calcExplosion();
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mVec3_c diff = mPos - player->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) >=
                lbl_80750528 &&
            dPlayerMgr::finishWatchUfo()) {
            setState(STATE_HIDDEN);
        }
    }
}
