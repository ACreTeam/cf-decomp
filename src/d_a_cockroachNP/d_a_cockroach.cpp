// A cockroach in a player's house room (REL d_a_cockroachNP). See include/game/game/d_a_cockroach.hpp.
#include <game/game/d_a_cockroach.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_cockroach_manager.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_m3d.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/math/eggMath.h>

namespace dACockroach {

struct Param_s {
    /* 0x00 */ int mWaitSecMax;
    /* 0x04 */ int mWaitSecMin;
    /* 0x08 */ f32 mWaitBrake;
    /* 0x0C */ f32 mTargetDistMax;
    /* 0x10 */ f32 mTargetDistMin;
    /* 0x14 */ int mRunSecMax;
    /* 0x18 */ int mRunSecMin;
    /* 0x1C */ f32 mRunSpeed;
    /* 0x20 */ f32 mRunAccel;
    /* 0x24 */ f32 mRunBrake;
    /* 0x28 */ f32 _28;
    /* 0x2C */ f32 mGravity;
    /* 0x30 */ f32 mJumpSpeed;
    /* 0x34 */ f32 mScareDist;
    /* 0x38 */ u8 mFadeSpeed;
    /* 0x3C */ f32 _3C;
}; // size 0x40

static Param_s l_param = {8, 4, 0.5f, 16.0f, 4.0f, 2, 1, 2.0f, 1.0f, 0.5f, 2.0f, -0.15f, 2.0f, 50.0f, 4, 0.0f};

BASE_PROFILE(A_COCKROACH, daCockroach_c);

} // namespace dACockroach

using dACockroach::l_param;

static const daCockroach_c::StateFunc sExecuteFuncs[daCockroach_c::STATE_NUM] = {
    &daCockroach_c::executeWait, &daCockroach_c::executeRun, &daCockroach_c::executeJump,
    &daCockroach_c::executeStepped, &daCockroach_c::executeKilled,
};

static const daCockroach_c::StateFunc sInitFuncs[daCockroach_c::STATE_NUM] = {
    &daCockroach_c::initWait, &daCockroach_c::initRun, &daCockroach_c::initJump,
    &daCockroach_c::initStepped, &daCockroach_c::initKilled,
};

const f32 daCockroachBase_c::sRadius = 5.0f;

// 0xFC
daCockroach_c::daCockroach_c()
    : mState(STATE_RUN), mPrevState(STATE_RUN), mStateFunc(NULL), mWaitTime(0), mRunTime(0), mRunLimit(0),
      mRunFrames(0), mAnmRate(0.0f), _214(0), mWallFrames(0), mPrevAngle(0), mTargetSpread(DEG_TO_ANGLE(30)) {}

// 0x194
daCockroach_c::~daCockroach_c() {}

// 0x1F4
int daCockroach_c::create() {
    if (!daCockroachBase_c::create()) {
        return FALSE;
    }
    changeState(STATE_RUN);
    if (dNet::isSessionActive() == true) {
        mAccelY = 0.0f;
    }
    return TRUE;
}

// 0x258
// The target re-reads mState for changeState (merged only after register allocation): no IR common subexpressions.
#pragma opt_common_subs off
int daCockroach_c::execute() {
    mVec3_c camPos = dCamera_c::sInstance->getEyePos();
    mSnd.setPos(&camPos);
    mPrevState = mState;
    (this->*sExecuteFuncs[mState])();
    mTimer++;
    checkState();
    if (mPrevState != mState) {
        changeState(mState);
    }
    daCockroachBase_c::execute();
    return TRUE;
}
#pragma opt_common_subs reset

// 0x2F8
int daCockroach_c::draw() {
    daCockroachBase_c::draw();
    return TRUE;
}

// 0x31C
void daCockroach_c::initWait() {
    mTimer = 0;
    mState = STATE_WAIT;
    mStateFunc = &daCockroach_c::executeWait;
    f32 frame = getAnmFrame();
    setAnm(ANM_WAIT);
    setAnmRate(mAnmRate);
    setAnmFrame(frame);
    mPos.y = getGroundY();
    mWaitTime = cM::rndRange<u16>(l_param.mWaitSecMin * 60, l_param.mWaitSecMax * 60);
}

// 0x3DC
void daCockroach_c::initRun() {
    mTimer = 0;
    mState = STATE_RUN;
    mStateFunc = &daCockroach_c::executeRun;
    f32 frame = getAnmFrame();
    setAnm(ANM_WAIT);
    setAnmRate(mAnmRate);
    setAnmFrame(frame);
    mPos.y = getGroundY();
    setTarget();
    mRunTime = cM::rndRange<u16>(l_param.mRunSecMin * 60, l_param.mRunSecMax * 60);
}

// 0x4A8
void daCockroach_c::initJump() {
    mTimer = 0;
    mState = STATE_JUMP;
    mStateFunc = &daCockroach_c::executeJump;
    setAnm(ANM_FLY);
    setAnmRate(mAnmRate);
    setAnmFrame(0.0f);
    mSpeed.y = l_param.mJumpSpeed;
    mSnd.startSound(SE_GokiburiJump);
}

// 0x53C
void daCockroach_c::initStepped() {
    mTimer = 0;
    mState = STATE_STEPPED;
    mStateFunc = &daCockroach_c::executeStepped;
    setAnm(ANM_DIE00);
    setAnmRate(mAnmRate);
    setAnmFrame(0.0f);
    mPos.y = getGroundY();
    mSnd.startSound(SE_GokiburiFumu);
    dEffect::create("af_goki_dead", &mPos, &mAngle, NULL);
}

// 0x5E4
void daCockroach_c::initKilled() {
    mTimer = 0;
    mState = STATE_KILLED;
    mStateFunc = &daCockroach_c::executeKilled;
    setAnm(ANM_WAIT);
    setAnmRate(mAnmRate);
    setAnmFrame(0.0f);
    mPos.y = getGroundY();
}

// 0x668
void daCockroach_c::executeWait() {
    mSpeedF *= l_param.mWaitBrake;
    calcSpeed();
    mPos += mSpeed;
    mPos.y = getGroundY();
    mAnmRate = mSpeedF / (0.001f + l_param.mRunSpeed);
    setAnmRate(mAnmRate);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 5.0f);
    }
}

// 0x74C
void daCockroach_c::executeRun() {
    mRunFrames++;
    mPrevAngle = mAngle3D.y.mAngle;
    sLib::addCalcAngle(&mAngle3D.y.mAngle, cLib::targetAngleY(mPos, mTargetPos), 5, DEG_TO_ANGLE(7));
    mSpeedF *= l_param.mRunBrake;
    mSpeedF += l_param.mRunAccel;
    calcSpeed();
    mPos += mSpeed;
    mPos.y = getGroundY();
    mAnmRate = mSpeedF / (0.001f + l_param.mRunSpeed);
    setAnmRate(mAnmRate);
    mSnd.holdSound(SE_GokiburiRun);
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        if (mTargetPos.xzDistTo(mPos) <= mSpeedF / 2.0f) {
            mPos -= mSpeed;
            mAngle3D.y = mPrevAngle;
        }
        mVec3_c min = dCockroachManager_c::sInstance->getAreaMin();
        mVec3_c max = dCockroachManager_c::sInstance->getAreaMax();
        mPos.x = mPos.x < min.x + 5.0f ? min.x + 5.0f : (mPos.x > max.x - 5.0f ? max.x - 5.0f : mPos.x);
        // Z bounds computed up front into locals, max first (x is a plain ternary in the target).
        f32 maxZ = max.z - 5.0f;
        f32 minZ = min.z + 5.0f;
        mPos.z = mPos.z < minZ ? minZ : (mPos.z > maxZ ? maxZ : mPos.z);
    }
}

// 0x978
void daCockroach_c::executeJump() {
    mSpeedF *= l_param.mRunBrake;
    mSpeedF += l_param.mRunAccel;
    calcSpeed();
    mPos += mSpeed;
}

// 0x9F8
void daCockroach_c::executeStepped() {
    if (!mKillSent) {
        dCockroachManager_c::sInstance->kill(mRecordIdx);
        mKillSent = TRUE;
    }
    int alpha = mAlpha - l_param.mFadeSpeed;
    mAlpha = alpha < 0 ? 0 : alpha;
    if (mAlpha == 0) {
        deleteRequest();
    }
}

// 0xA70
void daCockroach_c::executeKilled() {
    mSpeedF *= 0.95f;
    calcSpeed();
    mPos += mSpeed;
    mAnmRate = mSpeedF / (0.001f + l_param.mRunSpeed);
    if (mTimer <= 60) {
        if (0.1f < mSpeedF) {
            mSnd.holdSound(SE_GokiburiRun);
        }
        setAnmRate(mAnmRate);
        if (mTimer == 60) {
            setAnm(ANM_DIE01);
            setAnmRate(1.0f);
            setAnmFrame(0.0f);
        }
    } else {
        int alpha = mAlpha - l_param.mFadeSpeed;
        mAlpha = alpha < 0 ? 0 : alpha;
        if (mTimer > 120) {
            deleteRequest();
        }
    }
}

// 0xB98
void daCockroach_c::checkState() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
        int state = record->getState();
        if (state == STATE_STEPPED || state == STATE_KILLED) {
            if (record->getPlayer() == dNet::getMyNo()) {
                mState = STATE_STEPPED;
            } else {
                mState = STATE_KILLED;
            }
        } else {
            mState = state;
        }
        mTargetPos.x = record->getX();
        mTargetPos.z = record->getZ();
        return;
    }

    if (mState == STATE_WAIT) {
        if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
            dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
            record->setX(mPos.x);
            record->setZ(mPos.z);
            dCockroachManager_c::sInstance->setRecord(record);
        }
        if (isSteppedOn() == true || isCrushed() == true) {
            if (dNet::isSessionActive() == true) {
                if (mPlayer == dNet::getMyNo()) {
                    mState = STATE_STEPPED;
                } else {
                    mState = STATE_KILLED;
                }
            } else {
                mState = STATE_STEPPED;
            }
        } else if (mWaitTime <= mTimer) {
            mState = STATE_RUN;
        } else {
            int player = mPlayerCheck.getNearest();
            if (isPlayerNear(l_param.mScareDist, player) == true) {
                mAng away = mPlayerCheck.getAngleTo(player);
                s16 angle = mAngle3D.y.mAngle;
                mTargetSpread = DEG_TO_ANGLE(10);
                mAngle3D.y = away.mAngle - 0x8000;
                setTarget();
                mAngle3D.y = angle;
                mTargetSpread = DEG_TO_ANGLE(30);
                mTimer = 0;
                mPrevState = mState = STATE_RUN;
                mStateFunc = &daCockroach_c::executeRun;
                f32 frame = getAnmFrame();
                setAnm(ANM_WAIT);
                setAnmRate(mAnmRate);
                setAnmFrame(frame);
                mPos.y = getGroundY();
                mRunTime = cM::rndRange<u16>(l_param.mRunSecMin * 60, l_param.mRunSecMax * 60);
                if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
                    dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
                    record->setState(mState);
                    record->setPlayer(mPlayer);
                    dCockroachManager_c::sInstance->setRecord(record);
                }
            }
        }
    } else if (mState == STATE_RUN) {
        if (isPlayerNear(16.0f, dPlayerMgr::PLAYER_INDEX_CURRENT) == true &&
            isPlayerIdle(dPlayerMgr::PLAYER_INDEX_CURRENT) == true && !dNet::isSessionActive()) {
            mState = STATE_JUMP;
        } else if (isSteppedOn() == true || isCrushed() == true) {
            if (dNet::isSessionActive() == true) {
                if (mPlayer == dNet::getMyNo()) {
                    mState = STATE_STEPPED;
                } else {
                    mState = STATE_KILLED;
                }
            } else {
                mState = STATE_STEPPED;
            }
        } else if (mRunLimit <= mRunFrames) {
            if (mRunTime <= mTimer) {
                mState = STATE_WAIT;
            } else {
                setTarget();
            }
        }
        if (isWallBumped() == true) {
            mVec3_c wall = getWallDir();
            mVec3_c dir = mPos - mLastPos;
            dir.y = 0.0f;
            dir.normalize();
            mVec3_c turn = wall + dir;
            if (-0.5f < dir.dot(wall)) {
                mAngle3D.y = cM::atan2s(turn.x, turn.z);
            } else {
                dBGCF::lineChk_s line;
                mVec3_c start = mLastPos;
                mVec3_c end;
                mVec3_c min = dCockroachManager_c::sInstance->getAreaMin();
                mVec3_c max = dCockroachManager_c::sInstance->getAreaMax();
                mVec3_c side(0.0f, 0.0f, 100.0f);
                side.rotY(mAngle3D.y.mAngle + 0x4000);
                end = mLastPos + side;
                if (!dBGCF::checkLineSteps(&line, &end, &start, 0x33)) {
                    mAngle3D.y += 0x4000;
                } else {
                    f32 leftDist = end.xzDistTo(start);
                    side.set(0.0f, 0.0f, 100.0f);
                    side.rotY(mAngle3D.y.mAngle - 0x4000);
                    end = mLastPos + side;
                    dBGCF::checkLineSteps(&line, &end, &start, 0x33);
                    if (end.xzDistTo(start) <= leftDist) {
                        mAngle3D.y += 0x4000;
                    } else {
                        mAngle3D.y -= 0x4000;
                    }
                }
            }
            mTargetSpread = DEG_TO_ANGLE(10);
            setTarget();
            mTargetSpread = DEG_TO_ANGLE(30);
            mPos -= mSpeed;
            mAngle3D.y = mPrevAngle;
        }
        if (isWallBlocked() == true) {
            if (++mWallFrames >= 60) {
                mWallFrames = 0;
                mAngle3D.y += 0x8000;
                setTarget();
                mAngle3D.y -= 0x8000;
            }
        } else {
            mWallFrames = 0;
        }
    } else if (mState == STATE_JUMP) {
        if (isCrushed() == true) {
            if (dNet::isSessionActive() == true) {
                if (mPlayer == dNet::getMyNo()) {
                    mState = STATE_STEPPED;
                } else {
                    mState = STATE_KILLED;
                }
            } else {
                mState = STATE_STEPPED;
            }
        }
        if (mPos.y < getGroundY()) {
            mState = STATE_RUN;
        }
    }
}

// 0x13AC
void daCockroach_c::changeState(int state) {
    mState = state;
    (this->*sInitFuncs[state])();
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
        record->setState(mState);
        record->setPlayer(mPlayer);
        dCockroachManager_c::sInstance->setRecord(record);
    }
}

// 0x1464
mVec3_c daCockroach_c::calcTargetPos() {
    if (dNet::isSessionActive() == true && !dNet::isSceneOwner()) {
        mVec3_c pos(0.0f, 0.0f, 0.0f);
        dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
        pos.x = record->getX();
        pos.z = record->getZ();
        return pos;
    }
    mAng angle(mAngle3D.y.mAngle + (int)cM::rndCenterF(mTargetSpread));
    f32 dist = 32.0f + cM::rndRange<f32>(l_param.mTargetDistMin, l_param.mTargetDistMax);
    mVec3_c pos(0.0f, 0.0f, dist);
    pos.rotY(angle);
    pos += mPos;
    mVec3_c min = dCockroachManager_c::sInstance->getAreaMin();
    mVec3_c max = dCockroachManager_c::sInstance->getAreaMax();
    pos.x = pos.x < min.x ? min.x : (pos.x > max.x ? max.x : pos.x);
    // Z bounds copied to locals: loads max.z before the branch like the target (x does not).
    f32 minZ = min.z;
    f32 maxZ = max.z;
    pos.z = pos.z < minZ ? minZ : (pos.z > maxZ ? maxZ : pos.z);
    if (pos.xzDistTo(mPos) < 16.0f) {
        dCockroachManager_c *mgr = dCockroachManager_c::sInstance;
        int unitX = worldToUnit(mPos.x);
        int unitZ = worldToUnit(mPos.z);
        mVec3_c up;
        dFdBase_c::getUnitCenterPos(&up, unitX, unitZ - 1);
        if (!mgr->isOutside(up) && !mgr->isOnFurniture(up)) {
            pos = up;
        }
        mVec3_c down;
        dFdBase_c::getUnitCenterPos(&down, unitX, unitZ + 1);
        if (!mgr->isOutside(down) && !mgr->isOnFurniture(down)) {
            pos = down;
        }
        mVec3_c right;
        dFdBase_c::getUnitCenterPos(&right, unitX + 1, unitZ);
        if (!mgr->isOutside(right) && !mgr->isOnFurniture(right)) {
            pos = right;
        }
        mVec3_c left;
        dFdBase_c::getUnitCenterPos(&left, unitX - 1, unitZ);
        if (!mgr->isOutside(left) && !mgr->isOnFurniture(left)) {
            pos = left;
        }
    }
    if (dNet::isSessionActive() == true && dNet::isSceneOwner() == TRUE) {
        dCockroachRecord_c *record = dCockroachManager_c::sInstance->getRecord(mRecordIdx);
        record->setX(pos.x);
        record->setZ(pos.z);
        dCockroachManager_c::sInstance->setRecord(record);
    }
    return pos;
}

// 0x18D0
void daCockroach_c::setTarget() {
    mRunFrames = 0;
    mTargetPos = calcTargetPos();
    mRunLimit = 1.0f + mPos.xzDistTo(mTargetPos) / l_param.mRunSpeed;
}

// 0x1978
daCockroachBase_c::daCockroachBase_c()
    : mTargetPos(0.0f, 0.0f, 0.0f), mTimer(0), mRecordIdx(0), mAlpha(0xFF), mKillSent(FALSE),
      mPlayer(dCockroachRecord_c::PLAYER_NONE), mCheckBg(TRUE), mFurnitureFrames(0), mOutsideFrames(0), mOwner(FALSE) {}

// 0x1A70
daCockroachBase_c::~daCockroachBase_c() {}

// 0x1B20
int daCockroachBase_c::create() {
    mRecordIdx = mParam;
    nw4r::g3d::ResFile res(getResFile());
    if (!res.IsValid()) {
        return FALSE;
    }
    createModel(&res);
    mSpeedF = 0.0f;
    mAccelY = l_param.mGravity;
    mMaxSpeedY = -0.8f;
    return TRUE;
}

// 0x1BA0
int daCockroachBase_c::execute() {
    mAngle = mAngle3D;
    dBGCF::mvbg_c mvbg;
    static mVec3_c sRoomCenter(256.0f, 0.0f, 448.0f);
    dBGCF::getMvbgList()->add(64.0f, 0.0f, 128.0f, &mvbg, sRoomCenter, 0, NULL);
    if (dNet::isSessionActive() == true) {
        if (dNet::isSceneOwner() == TRUE) {
            mAcch.check(sRadius, &mPos, &mLastPos, mAngle.y, 0, 0x17, TRUE);
        }
    } else if (mCheckBg == TRUE) {
        mAcch.check(sRadius, &mPos, &mLastPos, mAngle.y, 0, 0x17, TRUE);
    }
    dBGCF::getMvbgList()->remove(&mvbg);
    mPlayerCheck.setPos(mPos);
    setAlpha(mAlpha);
    mAnmVis.play();
    mMdl.play();
    if (isRemote() == true && dCockroachManager_c::sInstance->isOnFurniture(mPos) == TRUE) {
        mVec3_c pos = getNearestFreePos(mPos);
        mPos = mLastPos = pos;
        mPos.y = mLastPos.y = getGroundY();
    }
    if (dNet::isSessionActive() == true) {
        mOwner = dNet::isSceneOwner();
    }
    return TRUE;
}

// 0x1F58
int daCockroachBase_c::draw() {
    mMdl.calc(false);
    mMtx_c mtx;
    makeMtx(&mtx);
    mMdl.setLocalMtx(&mtx);
    mMdl.entry();
    return TRUE;
}

static inline bool isMdlCreated(const m3d::mdl_c &mdl) {
    return mdl.getScn();
}

// 0x1FC8
int daCockroachBase_c::doDelete() {
    if (isMdlCreated(mMdl) == true) {
        mMdl.remove();
    }
    if (mAnmChr.IsBound() == true) {
        mAnmChr.remove();
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.remove();
    }
    return TRUE;
}

// 0x2064
void *daCockroachBase_c::getResFile() {
    // Needed to match: the loaded pointer must live in a named local that expression propagation keeps (an enum
    // typed one does), or it swaps registers with file.
    enum LoadState_e { LOAD_STATE_NONE };
    void *file = NULL;
    LoadState_e state = (LoadState_e)(int)dFtrm_c::sLoader.getData();
    if (state != LOAD_STATE_NONE) {
        file = dFtrm_c::sLoader.getFile("room_goki.brres");
    }
    return file;
}

// 0x20AC
void daCockroachBase_c::createModel(nw4r::g3d::ResFile *res) {
    mAllocator_c *allocator = dCockroachManager_c::sInstance->getAllocator();
    nw4r::g3d::ResMdl mdl = res->GetResMdl(0);
    if (mMdl.getScn() == NULL) {
        mMdl.create(mdl, allocator, 0x140, 1, NULL);
    }
    if (!mAnmChr.IsBound()) {
        mAnmChr.create(mdl, res->GetResAnmChr(0), allocator, NULL);
        mMdl.setAnm(mAnmChr);
    }
    if (!mAnmVis.IsBound()) {
        mAnmVis.create(mdl, res->GetResAnmVis(0), allocator, NULL);
        mMdl.setAnm(mAnmVis);
    }
}

// 0x21E4
void daCockroachBase_c::setAnm(int anm) {
    const char *names[] = {"wait", "fly", "die00", "die01"};
    nw4r::g3d::ResFile res(getResFile());
    if (res.IsValid()) {
        const char *name = names[anm];
        mAnmChr.setAnm(mMdl, res.GetResAnmChr(name), m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmChr);
        mAnmVis.setAnm(mMdl, res.GetResAnmVis(name), m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmVis);
    }
}

// 0x22D0
void daCockroachBase_c::setAnmRate(f32 rate) {
    if (mAnmChr.mFrameMax <= rate) {
        rate = mAnmChr.mFrameMax;
    }
    mAnmChr.setRate(rate);
    mAnmVis.setRate(rate);
}

// 0x2338
void daCockroachBase_c::setAnmFrame(f32 frame) {
    mAnmChr.setFrame(frame);
    mAnmVis.setFrame(frame);
}

// 0x2380
f32 daCockroachBase_c::getAnmFrame() {
    return mAnmChr.getFrame();
}

// 0x2388
bool daCockroachBase_c::isWallBumped() {
    if (mAcch.isFrontWallHit() == true && !mAcch.wasFrontWallHit()) {
        return true;
    }
    return false;
}

// 0x23B4
bool daCockroachBase_c::isWallBlocked() {
    if (mAcch.isFrontWallHit() == true && mAcch.wasFrontWallHit() == true) {
        return true;
    }
    return false;
}

// 0x23E4
mVec3_c daCockroachBase_c::getWallDir() {
    mVec3_c max = dCockroachManager_c::sInstance->getAreaMax();
    if (max.z - 16.0f <= mPos.z) {
        mVec3_c back(0.0f, 0.0f, -1.0f);
        return back;
    }
    dBGCF::acchWall_c *wall = mAcch.getWall();
    mVec3_c dir(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < wall->getNum(); i++) {
        mVec3_c v(0.0f, 0.0f, 1.0f);
        v.rotY(wall->mAngle[i]);
        dir += v;
    }
    dir.normalize();
    if (dir.isZero() == true) {
        dir.set(0.0f, 0.0f, 1.0f);
    }
    return dir;
}

// 0x2584
bool daCockroachBase_c::isCrushed() {
    dCockroachManager_c *mgr = dCockroachManager_c::sInstance;
    if (mgr->isOutside(mPos) == TRUE) {
        if (++mOutsideFrames >= 3) {
            mPlayer = dNet::getMyNo();
            return true;
        }
    } else {
        mOutsideFrames = 0;
    }
    if (mgr->isOnFurniture(mPos) == TRUE) {
        if (++mFurnitureFrames >= 3) {
            mPlayer = dNet::getMyNo();
            return true;
        }
    } else {
        mFurnitureFrames = 0;
    }
    return false;
}

// 0x2648
bool daCockroachBase_c::isPlayerNear(f32 radius, int player) {
    if (dPlayerMgr::getPlayerActor(player) != NULL) {
        mVec3_c pos = *dPlayerMgr::getSoundPosition(player);
        mVec3_c d = mPos - pos;
        if (EGG::Math<f32>::sqrt(d.x * d.x + d.z * d.z) <= radius) {
            return true;
        }
    }
    return false;
}

// 0x2700
bool daCockroachBase_c::isPlayerIdle(int player) {
    return !dPlayerMgr::fn_80100DD0(player);
}

// 0x272C
bool daCockroachBase_c::isSteppedOn() {
    for (u32 i = 0; i < 4; i++) {
        if (!isPlayerIdle(i) && mPos.y < 0.1f + getGroundY() && isPlayerNear(16.0f, i) == true) {
            mPlayer = i;
            return true;
        }
    }
    return false;
}

// 0x27E4
f32 daCockroachBase_c::getGroundY() {
    return dBGCF::getGroundY(&mPos, FALSE);
}

// 0x27F0
void daCockroachBase_c::setAlpha(u8 alpha) {
    mMdl.setDrawMode(nw4r::g3d::RESMDL_DRAWMODE_SORT_XLU_Z);
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("m1");
    if (mat.IsValid()) {
        nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mMdl.getScn());
        nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mat.ptr()->id);
        if (access.IsValid()) {
            GXColor color;
            nw4r::g3d::ResMatTevColor tev = access.GetResMatTevColor();
            tev.GXGetTevKColor(GX_KCOLOR1, &color);
            color.a = alpha;
            tev.GXSetTevKColor(GX_KCOLOR1, color);
            tev.DCStore(false);
        }
    }
}

// 0x2918
bool daCockroachBase_c::isRemote() {
    if (dNet::isSessionActive() == true && !mOwner && dNet::isSceneOwner() == TRUE) {
        return true;
    }
    return false;
}

// 0x2970
mVec3_c daCockroachBase_c::getNearestFreePos(const mVec3_c &pos) {
    dFdBase_c *field = getCurrentField();
    f32 nearest = 10000.0f;
    mVec3_c result(0.0f, 0.0f, 0.0f);
    for (int z = 0; z < UT_Z_NUM; z++) {
        for (int x = 0; x < UT_X_NUM; x++) {
            if (field->canPutItem(x, z) == TRUE) {
                mVec3_c c;
                dFdBase_c::getUnitCenterPos(&c, x, z);
                if (!dCockroachManager_c::sInstance->isOnFurniture(c)) {
                    mVec3_c d = c - pos;
                    f32 dist = EGG::Math<f32>::sqrt(d.x * d.x + d.z * d.z);
                    if (dist < nearest) {
                        nearest = dist;
                        result = c;
                    }
                }
            }
        }
    }
    return result;
}

namespace dACockroach {

// Unused; referencing anmChrPart_c's vtable queues anmChr_c::getType before anmVis_c::getType among the weak
// functions (as in d_bgt_draw). MWCC drops the function itself.
static void unusedPartAnm() {
    m3d::anmChrPart_c anm;
}

} // namespace dACockroach
