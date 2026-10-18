// The bulletin board's pigeon (REL d_a_bbs_birdNP). See
// include/game/game/d_a_bbs_bird.hpp.
#include <game/game/d_a_bbs_bird.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_str.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_notice.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_world.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <cmath>

// Debug parameters (the HIO tool's values).
class hostIO_c {
  public:
    hostIO_c() {
        _08 = 0;
        mScareDist = 56;
        mFlyPitch = 90;
        mFlySpeed = 4.0f;
        mOffsetY = 50.0f;
        mOffsetZ = 2.0f;
        mTiltX = 0;
        mWalkDist = 8.0f;
        mRangeX = 16.0f;
        mTurnAngle = DEG_TO_ANGLE(60);
        mMoveFrameStart = 5;
        mMoveFrameNum = 10;
        mPeckInterval = 45;
        mScale = 0.725f;
    }

    /* 0x00 */ s16 mScareDist; // STATE_NOTICE this close to the player
    /* 0x04 */ f32 mFlySpeed;
    /* 0x08 */ u8 _08;
    /* 0x0A */ s16 mFlyPitch; // degrees
    /* 0x0C */ s16 mTurnAngle;
    /* 0x10 */ f32 mOffsetY; // the perch above the board's position
    /* 0x14 */ f32 mOffsetZ;
    /* 0x18 */ s16 mTiltX; // degrees
    /* 0x1C */ f32 mWalkDist;
    /* 0x20 */ f32 mRangeX;         // how far it walks either side
    /* 0x24 */ s16 mMoveFrameStart; // the walk / turn anim frames that move it
    /* 0x26 */ s16 mMoveFrameNum;
    /* 0x28 */ s16 mPeckInterval; // frames
    /* 0x2C */ f32 mScale;

    virtual ~hostIO_c() {
    }
}; // size 0x34

namespace dABbsBird {
BASE_PROFILE(A_BBS_BIRD, daBbsBird_c);
} // namespace dABbsBird

// The perch's x offset from the board's position (mHomePos).
#define PERCH_OFFSET_X (-16.0f)

static hostIO_c l_hostIO;

daBbsBird_c *daBbsBird_c::s_instance;
u8 daBbsBird_c::s_strDrawn;

// .text:0x178
BOOL daBbsBird_c::isNight() {
    dTime_c now = *dTime_c::getCurrent();
    u8 hour = now.hour;
    return hour < TIME_DAY_START_HOUR || hour >= TIME_NIGHT_START_HOUR;
}

// .text:0x210
BOOL daBbsBird_c::loadModel() {
    if (!mRes.IsValid()) {
        mRes =
            nw4r::g3d::ResFile(dSTR::getBrresBank()->getBrres("obj_bbs_bird"));
    }
    if (mRes.IsValid()) {
        nw4r::g3d::ResMdl mdl = mRes.GetResMdl(0);
        mAllocator_c *allocator = dStrm_c::getAllocator();
        if (mMdl.getScn() == NULL) {
            mMdl.create(mdl, allocator, 0, 1, NULL);
        }
        if (!mAnm.IsBound()) {
            mAnm.create(mdl, mRes.GetResAnmChr(0), allocator, NULL);
            mAnm.mPlayMode = m3d::FORWARD_ONCE;
            mMdl.setAnm(mAnm);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x334
BOOL daBbsBird_c::setAnm(u32 anm, m3d::playMode_e mode, bool end) {
    static const char *names[ANM_NUM] = {
        "obj_bbs_bird_move", "obj_bbs_bird_pick", "obj_bbs_bird_fly"};
    if (anm < ANM_NUM) {
        nw4r::g3d::ResAnmChr anmChr = mRes.GetResAnmChr(names[anm]);
        if (anmChr.IsValid()) {
            mAnm.setAnm(mMdl, anmChr, mode);
            mMdl.setAnm(mAnm);
            if (end) {
                mAnm.setFrame(anmChr.GetNumFrame() - 1);
            }
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// .text:0x424
BOOL daBbsBird_c::playAnm() {
    if (mAnm.IsBound()) {
        if (mAnm.isStop()) {
            return TRUE;
        }
        mAnm.play();
        return FALSE;
    }
    return TRUE;
}

// .text:0x494
f32 daBbsBird_c::getFrame() const {
    if (mAnm.IsBound()) {
        return mAnm.getFrame();
    }
    return 0.0f;
}

// .text:0x4E0
void daBbsBird_c::entryMdl(mMtx_c *mtx) {
    mMdl.setLocalMtx(mtx);
    mMdl.calc(false);
    mMdl.entry();
}

// .text:0x530
void daBbsBird_c::removeMdl() {
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    if (mAnm.IsBound()) {
        mAnm.remove();
    }
}

inline void daBbsBird_c::setPerchPos() {
    mPos = mHomePos;
    mPos.x += PERCH_OFFSET_X;
    mPos.y += l_hostIO.mOffsetY;
    mPos.z += l_hostIO.mOffsetZ;
}

// .text:0x598
int daBbsBird_c::create() {
    if (loadModel()) {
        mHomePos = mPos;
        mPos.x += PERCH_OFFSET_X;
        mPos.y += l_hostIO.mOffsetY;
        mPos.z += l_hostIO.mOffsetZ;
        mMinX = mPos.x - l_hostIO.mRangeX;
        mMaxX = mPos.x + l_hostIO.mRangeX;
        mAngle.y = DEG_TO_ANGLE(90);
        checkNotice();
        s_instance = this;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x65C
int daBbsBird_c::execute() {
    executeState();
    mSnd.setPos(&mPos);
    f32 x = PERCH_OFFSET_X + mHomePos.x;
    mMinX = x - l_hostIO.mRangeX;
    mMaxX = l_hostIO.mRangeX + x;
    return SUCCEEDED;
}

// .text:0x6C8
int daBbsBird_c::draw() {
    if (s_strDrawn) {
        dSTR::viewArea_c area(dAStr::getViewFront(), dAStr::getViewBack(),
                              dAStr::getViewHalfWidth());
        if (mState != STATE_HIDE && area.isIn(mPos)) {
            mMdl.setScale(l_hostIO.mScale, l_hostIO.mScale, l_hostIO.mScale);
            mMtx_c mtx;
            calcMtx(&mtx);
            entryMdl(&mtx);
            drawEtc();
            drawShadow();
        }
    }
    s_strDrawn = FALSE;
    return SUCCEEDED;
}

// .text:0x7B8
int daBbsBird_c::doDelete() {
    removeMdl();
    s_instance = NULL;
    return SUCCEEDED;
}

// .text:0x7E8
void daBbsBird_c::drawShadow() {
    if (mState == STATE_HIDE) {
        return;
    }
    if (mState != STATE_FLY) {
        mVec3_c pos = mPos;
        pos.z += 4.0f;
        mAng angle = getAngleY();
        dShadow::requestRound(&pos, dShadow::DRAW_QUAD, &angle, 5.5f, 1.0f,
                              1.0f, 7.5f, 1.0f);
    } else {
        mAng angle = getAngleY();
        dShadow::requestRound(&mPos, 0, &angle, 5.5f, 1.0f, 1.0f, 7.5f, 1.0f);
    }
}

// .text:0x89C
void daBbsBird_c::drawEtc() {
}

// .text:0x8A0
void daBbsBird_c::checkPlayer() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL &&
        mPos.xzDistTo(player->getPos()) <= l_hostIO.mScareDist) {
        setState(STATE_NOTICE);
    }
}

// .text:0x944
void daBbsBird_c::checkNotice() {
    if (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) {
        if (dNotice::hasUnreadNotice()) {
            if (mFlewAway) {
                setState(STATE_RETURN);
            } else {
                setState(STATE_WAIT);
            }
        } else {
            setState(STATE_HIDE);
        }
    }
}

// .text:0x9C8
BOOL daBbsBird_c::isFacingSide() const {
    int half = l_hostIO.mTurnAngle >> 1;
    int right = sLib::distanceAngle(mAngle.y.mAngle, DEG_TO_ANGLE(90));
    int left = sLib::distanceAngle(mAngle.y.mAngle, -DEG_TO_ANGLE(90));
    return right < half || left < half;
}

// .text:0xA48
BOOL daBbsBird_c::chance(u32 percent) {
    return cM::rndInt(100) < percent;
}

// .text:0xA88
void daBbsBird_c::calcMtx(mMtx_c *mtx) {
    mVec3_c pos;
    mAng curve = dWorld::curvePosition(&pos, &mPos);
    s16 tilt = l_hostIO.mTiltX * ANGLE_PER_DEGREE_F;
    PSMTXTrans(*mtx, pos.x, pos.y, pos.z);
    mtx->XrotM(tilt + curve);
    mtx->YrotM(mAngle.y.mAngle);
    mtx->XrotM(mAngle.x);
}

// .text:0xB74
BOOL daBbsBird_c::setState(int state) {
    static const InitFunc initFuncs[STATE_NUM] = {
        &daBbsBird_c::initHide, &daBbsBird_c::initReturn,
        &daBbsBird_c::initWait, &daBbsBird_c::initWalk,
        &daBbsBird_c::initTurn, &daBbsBird_c::initNotice,
        &daBbsBird_c::initFly,
    };
    if (state < STATE_NUM && (this->*initFuncs[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0xBE0
void daBbsBird_c::executeState() {
    static const ExecFunc executeFuncs[STATE_NUM] = {
        &daBbsBird_c::executeHide, &daBbsBird_c::executeReturn,
        &daBbsBird_c::executeWait, &daBbsBird_c::executeWalk,
        &daBbsBird_c::executeTurn, &daBbsBird_c::executeNotice,
        &daBbsBird_c::executeFly,
    };
    if (mState < STATE_NUM) {
        (this->*executeFuncs[mState])();
    }
}

// .text:0xC20
BOOL daBbsBird_c::initHide() {
    return TRUE;
}

// .text:0xC28
void daBbsBird_c::executeHide() {
    static f32 sDist = 2.0f * lbl_80750520;
    static f32 sDistSq = sDist * sDist;
    if (dCamera_c::sCenter.xzDistSqTo(mHomePos) > sDistSq) {
        checkNotice();
    }
}

// .text:0xCE8
BOOL daBbsBird_c::initReturn() {
    return setAnm(ANM_MOVE, m3d::FORWARD_ONCE, false);
}

// .text:0xCF8
void daBbsBird_c::executeReturn() {
    if (setState(STATE_WAIT)) {
        setPerchPos();
        mAngle.y = DEG_TO_ANGLE(90);
        mTargetAngle = 0;
        mTurnStep = 0;
        mAngle.x = 0;
    }
}

// .text:0xD8C
BOOL daBbsBird_c::initWait() {
    if (setAnm(ANM_PICK, m3d::FORWARD_ONCE, true)) {
        int state = mState;
        mPecking = FALSE;
        if (state == STATE_WALK) {
            mTimer = cM::rndInt(4) + 1;
        } else if (chance(30) || isFacingSide()) {
            mTimer = cM::rndInt(12) + 12;
        } else {
            mTimer = cM::rndInt(180) + 180;
            mPecking = TRUE;
        }
    }
    if (mPos.x > mMaxX) {
        mPos.x = mMaxX;
    } else if (mPos.x < mMinX) {
        mPos.x = mMinX;
    }
    return TRUE;
}

// .text:0xE70
void daBbsBird_c::executeWait() {
    if (isNight()) {
        mSnd.holdSound(SE_LEV_midokudori_stay_Night);
    } else {
        mSnd.holdSound(SE_LEV_midokudori_stay);
    }
    if (mTimer != 0) {
        mTimer--;
    }
    if (mTimer == 0) {
        f32 toMax = std::fabs(mMaxX - mPos.x);
        f32 toMin = std::fabs(mMinX - mPos.x);
        if ((toMax < 1.0f && mAngle.y > 0) || (toMin < 1.0f && mAngle.y < 0)) {
            setState(STATE_TURN);
        } else if (isFacingSide()) {
            if (chance(82)) {
                setState(STATE_WALK);
            } else {
                setState(STATE_TURN);
            }
        } else {
            setState(STATE_TURN);
        }
    }
    if (mPecking) {
        u32 rem = mTimer % l_hostIO.mPeckInterval;
        if (mTimer >= l_hostIO.mPeckInterval && rem == 0 && chance(50)) {
            setAnm(ANM_PICK, m3d::FORWARD_ONCE, false);
        }
        playAnm();
    }
    checkPlayer();
}

// .text:0x100C
BOOL daBbsBird_c::initWalk() {
    if (setAnm(ANM_MOVE, m3d::FORWARD_ONCE, false)) {
        if (mAngle.y > 0) {
            mTargetX = mPos.x + l_hostIO.mWalkDist;
        } else {
            mTargetX = mPos.x - l_hostIO.mWalkDist;
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x1098
void daBbsBird_c::executeWalk() {
    mSnd.holdSound(SE_LEV_midokudori_stay);
    if (playAnm()) {
        mPos.x = mTargetX;
        if (setState(STATE_WAIT)) {
            return;
        }
    }
    if (getFrame() >= l_hostIO.mMoveFrameStart) {
        s16 num = l_hostIO.mMoveFrameNum;
        if (getFrame() < l_hostIO.mMoveFrameStart + num) {
            sLib::chase(&mPos.x, mTargetX, 8.0f / num);
        }
    }
    checkPlayer();
}

// .text:0x11A8
BOOL daBbsBird_c::initTurn() {
    if (setAnm(ANM_MOVE, m3d::FORWARD_ONCE, false)) {
        if (chance(70)) {
            if (mTurnPlus) {
                mTargetAngle = l_hostIO.mTurnAngle + mAngle.y.mAngle;
                mTurnStep = l_hostIO.mTurnAngle / l_hostIO.mMoveFrameNum;
            } else {
                mTargetAngle = mAngle.y.mAngle - l_hostIO.mTurnAngle;
                mTurnStep = -(l_hostIO.mTurnAngle / l_hostIO.mMoveFrameNum);
            }
        } else if (chance(50)) {
            mTargetAngle = l_hostIO.mTurnAngle + mAngle.y.mAngle;
            mTurnStep = l_hostIO.mTurnAngle / l_hostIO.mMoveFrameNum;
            mTurnPlus = TRUE;
        } else {
            mTargetAngle = mAngle.y.mAngle - l_hostIO.mTurnAngle;
            mTurnStep = -(l_hostIO.mTurnAngle / l_hostIO.mMoveFrameNum);
            mTurnPlus = FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x12EC
void daBbsBird_c::executeTurn() {
    mSnd.holdSound(SE_LEV_midokudori_stay);
    if (playAnm()) {
        mAngle.y = mTargetAngle;
        if (setState(STATE_WAIT)) {
            return;
        }
    }
    if (getFrame() >= l_hostIO.mMoveFrameStart &&
        getFrame() < l_hostIO.mMoveFrameStart + l_hostIO.mMoveFrameNum) {
        mAngle.y += mTurnStep;
    }
    checkPlayer();
}

// .text:0x13F0
BOOL daBbsBird_c::initNotice() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL && setAnm(ANM_MOVE, m3d::FORWARD_LOOP, false)) {
        mVec3_c dir = mPos - player->getPos();
        mTargetAngle = cM::atan2s(dir.x, dir.z);
        return TRUE;
    }
    return FALSE;
}

// .text:0x147C
void daBbsBird_c::executeNotice() {
    playAnm();
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 10, DEG_TO_ANGLE(67.5));
    if (sLib::distanceAngle(mAngle.y, mTargetAngle) < (int)DEG_TO_ANGLE(22.5)) {
        setState(STATE_FLY);
    }
}

// .text:0x14DC
BOOL daBbsBird_c::initFly() {
    if (setAnm(ANM_FLY, m3d::FORWARD_LOOP, false)) {
        if (isNight()) {
            mSnd.startSound(SE_midokudori_fly_out_Night);
        } else {
            mSnd.startSound(SE_midokudori_fly_out);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x1550
void daBbsBird_c::executeFly() {
    playAnm();
    mVec3_c speed(0.0f, 0.0f, l_hostIO.mFlySpeed);
    speed.rotX(mAngle.x);
    speed.rotY(mAngle.y);
    mPos += speed;
    sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 30, DEG_TO_ANGLE(67.5));
    sLib::addCalcAngle(&mAngle.x.mAngle, -l_hostIO.mFlyPitch * ANGLE_PER_DEGREE,
                       50, DEG_TO_ANGLE(67.5));
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL && mPos.xzDistTo(player->getPos()) > lbl_80750520) {
        setState(STATE_HIDE);
        mFlewAway = TRUE;
    }
    mAcch.check(8.0f, &mPos, &mLastPos, mAngle.y, (int)this,
                dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL | dBGCF::CHECK_SET_POS,
                TRUE);
}
