// The fishing float and the slingshot shot. See include/game/game/d_uki.hpp.
#include <game/game/d_uki.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_fltg.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <nw4r/g3d.h>
#include <nw4r/ef.h>
#include <nw4r/math.h>
#include <string.h>

#define UKI_HEAP_SIZE 0xA00

static dUkiBank_c l_bank;

dUki_c::dUki_c() {
    mSlot = UKI_SLOT_NUM;
    mKind = KIND_NONE;
    mNum = UKI_SHOT_NUM;
    mMdlIdx = 2;
    mHeap = NULL;
}

dUki_c::~dUki_c() {}

void dUki_c::init(int slot) {
    if (mHeap == NULL) {
        mHeap = mHeap::createFrmHeap(UKI_HEAP_SIZE, dHeap::hmnToolHeap_p, "dUkiBank_c::m_heap_p", 0x20, mHeap::OPT_NONE);
        mAllocator.attach(mHeap, 0x20);
        mSlot = slot;
        l_bank.setUki(slot, this);
    }
}

void dUki_c::remove() {
    if (mHeap != NULL) {
        removeMdl();
        l_bank.setUki(mSlot, NULL);
        mSlot = UKI_SLOT_NUM;
        mHeap::destroyFrmHeap(mHeap);
        mHeap = NULL;
    }
}

void dUki_c::create(int kind, const dItem::Item *item) {
    if (kind >= KIND_NONE) {
        return;
    }
    if (l_bank.mResData == NULL) {
        return;
    }
    nw4r::g3d::ResFile file(l_bank.mResData);
    if (!file.IsValid() || !file.HasResMdl()) {
        return;
    }
    reset();
    nw4r::g3d::ResMdl mdl = file.GetResMdl(kind);
    if (!mdl.IsValid()) {
        return;
    }
    if (kind == KIND_PACHINKO) {
        switch (item->getKind()) {
        case dItem::KIND_PACHINKO:
            mNum = 0;
            break;
        case dItem::KIND_SILVER_PACHINKO:
            mNum = 1;
            break;
        case dItem::KIND_GOLD_PACHINKO:
            mNum = 2;
            break;
        }
        for (int i = 0; i <= mNum; i++) {
            mShotMdl[i].create(mdl, &mAllocator, 0, 1, NULL);
        }
        setState(STATE_SHOT_HOLD);
    } else {
        switch (kind) {
        case KIND_ROD:
            mNum = 0;
            break;
        case KIND_SILVER_ROD:
            mNum = 1;
            break;
        case KIND_GOLD_ROD:
            mNum = 2;
            break;
        }
        if (mMdlIdx == 0) {
            mMdlIdx = 1;
        } else {
            mMdlIdx = 0;
        }
        mRodMdl[mMdlIdx].create(mdl, &mAllocator, 8, 1, NULL);
        setState(STATE_HOLD);
    }
    mKind = kind;
}

void dUki_c::putAway() {
    reset();
}

void dUki_c::calc(const mMtx_c *mtx) {
    if (mtx == NULL) {
        return;
    }
    mRodMtx = *mtx;
    mMtx_c mtxs[UKI_SHOT_NUM];
    mtxs[0] = *mtx;
    switch (mState) {
    case STATE_NONE:
        return;
    case STATE_HOLD:
        executeHold(mtxs);
        break;
    case STATE_RETURN:
        executeReturn(mtxs);
        break;
    case STATE_SWING:
        executeSwing(mtxs);
        break;
    case STATE_CAST:
        executeCast(mtxs);
        break;
    case STATE_FLOAT:
        executeFloat(mtxs);
        break;
    case STATE_DROP:
        executeDrop(mtxs);
        break;
    case STATE_BITE:
        executeBite(mtxs);
        break;
    case STATE_HOOKED:
        executeHooked(mtxs);
        break;
    case STATE_REEL:
        executeReel(mtxs);
        break;
    case STATE_REEL_EMPTY:
        executeReelEmpty(mtxs);
        break;
    case STATE_SHOT_HOLD:
        executeShotHold();
        break;
    case STATE_SHOT:
        executeShot(mtxs);
        break;
    }
    mSound.setPos(&mPos[0]);
    mTimer++;
    if (mKind == KIND_PACHINKO) {
        for (int i = 0; i <= mNum; i++) {
            mShotMdl[i].setLocalMtx(&mtxs[i]);
            mShotMdl[i].calc(false);
        }
    } else {
        mRodMdl[mMdlIdx].setLocalMtx(&mtxs[0]);
        mRodMdl[mMdlIdx].calc(false);
    }
}

void dUki_c::draw() {
    if (mState == STATE_NONE || mState == STATE_SHOT_HOLD) {
        return;
    }
    if (mKind == KIND_PACHINKO) {
        for (int i = 0; i <= mNum; i++) {
            mShotMdl[i].entry();
        }
    } else {
        mRodMdl[mMdlIdx].entry();
    }
}

void dUki_c::setMdlMtx(const mMtx_c *mtx) {
    if (mState != STATE_NONE && mtx != NULL) {
        if (mKind == KIND_PACHINKO) {
            for (int i = 0; i <= mNum; i++) {
                mShotMdl[i].setLocalMtx(mtx);
            }
        } else {
            mRodMdl[mMdlIdx].setLocalMtx(mtx);
        }
    }
}

void dUki_c::removeMdl() {
    if (mState != STATE_NONE) {
        setState(STATE_NONE);
        if (mKind == KIND_PACHINKO) {
            for (int i = 0; i <= mNum; i++) {
                mShotMdl[i].remove();
            }
        } else {
            mRodMdl[mMdlIdx].remove();
        }
        mKind = KIND_NONE;
        mNum = UKI_SHOT_NUM;
    }
}

void dUki_c::reset() {
    if (mState != STATE_NONE) {
        removeMdl();
        if (mHeap != NULL) {
            mHeap->free(3);
        }
        mAllocator.attach(mHeap, 0x20);
    }
}

void dUki_c::initHold() {
    mFollowTip = false;
    setMatAlpha(TRUE, NULL);
}

void dUki_c::executeHold(mMtx_c *mtx) {
    calcTipMtx(mtx);
    mPos[0].set(mtx->_03, mtx->_13, mtx->_23);
    dWorld::toFieldPosition(&mPos[0], &mPos[0]);
}

void dUki_c::initReturn() {
    initTipPos();
}

void dUki_c::executeReturn(mMtx_c *mtx) {
    BOOL done = calcReturn();
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
    if (done) {
        setState(STATE_HOLD);
    }
}

void dUki_c::initSwing() {
    mFollowTip = true;
}

void dUki_c::executeSwing(mMtx_c *mtx) {
    if (mFollowTip) {
        mFollowTip = false;
        initTipPos();
    }
    BOOL done = calcSwing();
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
    if (done) {
        setState(STATE_CAST);
    }
}

void dUki_c::initCast() {
    mCastRate = 0.035f;
    mVec3_c *player = dPlayerMgr::getPosition(mSlot);
    if (player != NULL) {
        dBGCF::groundChk_c chk(&mTarget, 0, 0, 0);
        if (player->y - chk.getWaterY() > 32.0f) {
            mCastRate = 0.029f;
        }
    }
}

void dUki_c::executeCast(mMtx_c *mtx) {
    BOOL done = calcCast();
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
    if (done) {
        setState(STATE_FLOAT);
    }
}

void dUki_c::initFloat() {
    if (mState == STATE_HOLD) {
        mFollowTip = true;
        mAlpha = 0xFF;
    } else {
        mFollowTip = false;
        mAlpha = getFallAlpha(&mPos[0]);
    }
    mFade = false;
    if (mState == STATE_CAST) {
        mSound.startSound(SE_FloatIn);
        createSplashEffect("af_itm_uki_potya", &mPos[0]);
    } else if (mState == STATE_BITE) {
        createWaterEffect("af_itm_uki_hamon_1time", &mPos[0]);
        mFade = true;
    }
    mRipple = false;
}

static inline f32 getDistXZSq(const mVec3_c &a, const mVec3_c &b) {
    mVec3_c diff = a - b;
    return diff.x * diff.x + diff.z * diff.z;
}

void dUki_c::executeFloat(mMtx_c *mtx) {
    if (mFollowTip) {
        updateTip();
    }
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        if (mFade) {
            if (setMatAlpha(FALSE, &chk)) {
                mFade = false;
            }
        }
        f32 flowX = chk.mDir.x;
        f32 flowZ = chk.mDir.z;
        mVec3_c *player = dPlayerMgr::getPosition(mSlot);
        mVec3_c effPos;
        if (player != NULL && getDistXZSq(*player, mPos[0]) < 20000.0f) {
            f32 aheadX = mPos[0].x + 18.0f * flowX;
            f32 aheadZ = mPos[0].z + 18.0f * flowZ;
            mVec3_c ahead(aheadX, 0.0f, aheadZ);
            dBGCF::groundChk_c aheadChk(&ahead, 0, 0, 0);
            if (aheadChk.mAttr != BG_ATTR_BRIDGE) {
                flowX = 0.15f * flowX;
                flowZ = 0.15f * flowZ;
                f32 x = mPos[0].x + flowX;
                f32 z = mPos[0].z + flowZ;
                mVec3_c next(x, mPos[0].y, z);
                f32 nextGround = dBGCF::getGroundY(&next, 0);
                f32 ground = dBGCF::getGroundY(&mPos[0], 0);
                mPos[0].x = x;
                mPos[0].z = z;
                if (ground > nextGround) {
                    setState(STATE_DROP);
                    PSMTXIdentity(*mtx);
                    mAng angle(0);
                    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
                    return;
                }
            } else {
                if (mPos[0].y == chk.getWaterY()) {
                    mPos[0].y += 0.5f;
                }
                mRipple = true;
            }
        } else {
            if (mPos[0].y == chk.getWaterY()) {
                mPos[0].y += 0.5f;
            }
            if (player != NULL) {
                s16 angle = dActor_c::targetAngleY(&mPos[0], player);
                mPos[0].x += 0.15f * nw4r::math::SinIdx(angle);
                mPos[0].z += 0.15f * nw4r::math::CosIdx(angle);
            }
            mRipple = true;
        }
        if (mPos[0].y < chk.getWaterY() - 10.0f) {
            mPos[0].y = chk.getWaterY() - 10.0f;
        }
        sLib::chase(&mPos[0].y, chk.getWaterY(), 0.05f);
        effPos.set(mPos[0].x, chk.getWaterY(), mPos[0].z);
        mAng3_c effAngle(0, cM::atan2s(flowX, flowZ), 0);
        mEffect1.create("af_itm_uki_moya", &effPos, &effAngle, NULL, 0);
        if (mAlpha < 0xFF) {
            mAlpha = getFallAlpha(&mPos[0]);
        }
        mEffect1.setRegisterAlpha(mAlpha, 0xFF, 1);
        if (mRipple) {
            mEffect0.create("af_itm_uki_hamon_loop", &effPos, &effAngle, NULL, 0);
            mEffect0.setRegisterAlpha(mAlpha, 0xFF, 1);
        }
    }
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
}

void dUki_c::exitFloat() {
    setMatAlpha(TRUE, NULL);
}

void dUki_c::initDrop() {}

void dUki_c::executeDrop(mMtx_c *mtx) {
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    f32 flowX = 0.15f * chk.mDir.x;
    f32 flowZ = 0.15f * chk.mDir.z;
    if (chk.mWater != BG_WATER_NONE) {
        f32 x = mPos[0].x + flowX;
        f32 z = mPos[0].z + flowZ;
        mPos[0].x = x;
        mPos[0].z = z;
        if (mTimer >= 120) {
            mVec3_c pos(x, mPos[0].y, z);
            f32 ground = dBGCF::getGroundY(&pos, 0);
            f32 t = mTimer - 120;
            mPos[0].y -= t * (0.0015f * t);
            if (mPos[0].y < ground) {
                mPos[0].y = ground;
                setState(STATE_FLOAT);
            }
        }
    }
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
}

void dUki_c::initBite() {
    if (mState == STATE_HOLD) {
        mFollowTip = true;
    } else {
        mFollowTip = false;
    }
    mAngX = 0;
    mAngXTarget = 0x400;
    mSound.startSound(SE_FloatSink);
    createSplashEffect("af_itm_uki_potya", &mPos[0]);
    if (mSlot == dNet::getMyNo()) {
        dFishFldShadow_c *fish = getFish();
        if (fish != NULL) {
            switch (fish->mParam.mSize) {
            case FISH_SIZE_TINY:
                dRumble::start(5, false);
                break;
            case FISH_SIZE_SMALL:
                dRumble::start(5, false);
                break;
            case FISH_SIZE_MEDIUM:
                dRumble::start(6, false);
                break;
            case FISH_SIZE_NARROW:
                dRumble::start(6, false);
                break;
            case FISH_SIZE_LARGE:
                dRumble::start(6, false);
                break;
            case FISH_SIZE_XLARGE:
                dRumble::start(6, false);
                break;
            case FISH_SIZE_HUGE:
                dRumble::start(7, false);
                break;
            case FISH_SIZE_FIN:
                dRumble::start(7, false);
                break;
            }
        }
    }
}

void dUki_c::executeBite(mMtx_c *mtx) {
    if (mFollowTip) {
        updateTip();
    }
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        f32 waterY = chk.getWaterY();
        sLib::chase(&mPos[0].y, -3.5f + waterY, 1.0f);
        setMatAlpha(FALSE, &chk);
    }
    mAngY += 400;
    if (sLib::addCalcAngle(&mAngX, mAngXTarget, 20, 0x1000, 300) == 0) {
        mAngXTarget *= -1;
        if (mAngXTarget > 0) {
            mAngXTarget = cM::rndInt(400) + 0x400;
        } else {
            mAngXTarget = -0x400 - cM::rndInt(400);
        }
    }
    PSMTXIdentity(*mtx);
    mAng angleY(mAngY);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angleY);
    mtx->XrotM(mAngX);
}

void dUki_c::exitBite(u32 next) {
    if (next != STATE_FLOAT) {
        setMatAlpha(TRUE, NULL);
    }
}

void dUki_c::initHooked() {
    if (mState == STATE_HOLD) {
        mFollowTip = true;
    } else {
        mFollowTip = false;
    }
    mAngX = 0;
    mAngXTarget = -0x7FF;
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        mPos[0].y = chk.getWaterY();
    }
    mHookPos = mPos[0];
    mFishPos = mPos[0];
    mPullRadius = 1.5f;
    dFishFldShadow_c *fish = getFish();
    if (fish != NULL) {
        switch (fish->mParam.mSize) {
        case FISH_SIZE_MEDIUM:
        case FISH_SIZE_LARGE:
        case FISH_SIZE_NARROW:
            mPullRadius = 3.3f;
            break;
        case FISH_SIZE_XLARGE:
        case FISH_SIZE_HUGE:
        case FISH_SIZE_FIN:
            mPullRadius = 5.2f;
            break;
        }
    }
    mEffect0.create("af_itm_uki_fishing01", &mPos[0], NULL, NULL, 0);
    mEffect1.create("af_itm_uki_fishing02", &mPos[0], NULL, NULL, 0);
    mEffect1.setParticleCallback(particleCallback, 0);
}

void dUki_c::executeHooked(mMtx_c *mtx) {
    if (mFollowTip) {
        updateTip();
    }
    mAng angle = dActor_c::targetAngleY(&mPos[0], &mFishPos);
    angle -= mAng(0x3FFF);
    f32 radius = 0.5f * mTimer;
    if (radius > mPullRadius) {
        radius = mPullRadius;
    }
    mHookPos.x = mPos[0].x + radius * angle.sin();
    mHookPos.z = mPos[0].z + radius * angle.cos();
    dBGCF::groundChk_c chk(&mHookPos, 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        if (sLib::chase(&mHookPos.y, chk.getWaterY() - 1.0f, 0.1f)) {
            mHookPos.y = chk.getWaterY() - 2.0f;
        }
    }
    mAngY = angle + mAng(0x4000);
    if (sLib::addCalcAngle(&mAngX, mAngXTarget, 10, 0x1500, 500) == 0) {
        mAngXTarget = -0x7FF - cM::rndInt(1000);
    }
    mEffect0.create("af_itm_uki_fishing01", &mHookPos, NULL, NULL, 0);
    mEffect1.create("af_itm_uki_fishing02", &mHookPos, NULL, NULL, 0);
    PSMTXIdentity(*mtx);
    mAng angleY(mAngY);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mHookPos, &angleY);
    mtx->XrotM(mAngX);
}

void dUki_c::initReel() {
    if (mState == STATE_HOLD) {
        updateTip();
    }
    mAngY = 0;
    mAngX = 0;
    mReelPos.set(mRodMtx._03, mRodMtx._13, mRodMtx._23);
    dWorld::toFieldPosition(&mReelPos, &mReelPos);
    mReelPos.y += 7.5f;
    mTarget = mPos[0];
    mSound.startSound(SE_FloatOut);
    createSplashEffect("af_itm_uki_catch", &mPos[0]);
}

void dUki_c::executeReel(mMtx_c *mtx) {
    BOOL done = calcReel();
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
    if (done) {
        setState(STATE_HOLD);
    }
}

void dUki_c::initReelEmpty() {
    if (mState == STATE_HOLD) {
        updateTip();
    }
    mReelPos.set(mRodMtx._03, mRodMtx._13, mRodMtx._23);
    dWorld::toFieldPosition(&mReelPos, &mReelPos);
    mTarget = mPos[0];
    mSound.startSound(SE_FloatOut);
    createWaterEffect("af_itm_uki_catch", &mPos[0]);
}

void dUki_c::executeReelEmpty(mMtx_c *mtx) {
    BOOL done = calcReelEmpty();
    PSMTXIdentity(*mtx);
    mAng angle(0);
    dPlayerMgr::makeCurvedYawMatrix(mtx, &mPos[0], &angle);
    if (done) {
        setState(STATE_HOLD);
    }
}

void dUki_c::initShotHold() {
    mMtx_c mtx(mRodMtx);
    mTip.set(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(&mTip, &mTip);
    for (int i = 0; i <= mNum; i++) {
        mPos[i] = mTip;
    }
}

void dUki_c::executeShotHold() {
    mPos[0].set(mRodMtx._03, mRodMtx._13, mRodMtx._23);
    dWorld::toFieldPosition(&mPos[0], &mPos[0]);
}

void dUki_c::initShot() {
    mAngX = 0x4000;
    if (!dNet::isSessionActive()) {
        dFltg::actor_c *target = NULL;
        for (u32 i = 0; i < dFltg::getCount(); i++) {
            target = dFltg::getNth(i);
        }
        if (target != NULL) {
            mVec3_c pos = *target->getPosP();
            if (std::fabs(mTip.x - pos.x) <= 2.0f * lbl_80750520) {
                f32 dz = mTip.z - pos.z;
                if (dz >= 0.0f && dz <= lbl_80750520) {
                    mAngX = cM::atan2s(pos.y - mTip.y, dz);
                    if (mAngX > 0x4000 || mAngX < 0x238E) {
                        mAngX = 0x4000;
                    }
                }
            }
        }
    }
    switch (mNum) {
    case 0:
        mPos[0] = mTip;
        break;
    case 1:
        mPos[0].set(mTip.x - 8.0f, mTip.y, mTip.z);
        mPos[1].set(mTip.x + 8.0f, mTip.y, mTip.z);
        break;
    case 2:
        for (int i = 0; i <= mNum; i++) {
            mPos[i] = mTip;
        }
        break;
    }
    for (int i = 0; i <= mNum; i++) {
        mObjc[i].mOnceHit = 0;
        mObjc[i].set(mPos[i], 3.0f, 5.0f, 0x40002000, 0xC000, 0x80, 0);
    }
    mSound.startSound(SE_SlingShot_Fly);
}

void dUki_c::executeShot(mMtx_c *mtx) {
    BOOL done = calcShot();
    for (int i = 0; i <= mNum; i++) {
        PSMTXIdentity(mtx[i]);
        mAng angle(0);
        dPlayerMgr::makeCurvedYawMatrix(&mtx[i], &mPos[i], &angle);
    }
    if (done) {
        setState(STATE_SHOT_HOLD);
    }
}

void dUki_c::setState(u32 state) {
    if (state == mState) {
        return;
    }
    if (state != STATE_NONE) {
        if (mState == STATE_FLOAT) {
            exitFloat();
        } else if (mState == STATE_BITE) {
            exitBite(state);
        }
    }
    switch (state) {
    case STATE_HOLD:
        initHold();
        break;
    case STATE_RETURN:
        initReturn();
        break;
    case STATE_SWING:
        initSwing();
        break;
    case STATE_CAST:
        initCast();
        break;
    case STATE_FLOAT:
        initFloat();
        break;
    case STATE_DROP:
        initDrop();
        break;
    case STATE_BITE:
        initBite();
        break;
    case STATE_HOOKED:
        initHooked();
        break;
    case STATE_REEL:
        initReel();
        break;
    case STATE_REEL_EMPTY:
        initReelEmpty();
        break;
    case STATE_SHOT_HOLD:
        initShotHold();
        break;
    case STATE_SHOT:
        initShot();
        break;
    }
    mState = state;
    mTimer = 0;
}

void dUki_c::nibble() {
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        mPos[0].y += -1.2f;
    }
    mVec3_c pos(mPos[0].x, chk.getWaterY(), mPos[0].z);
    dEffect::create("af_itm_uki_hamon_1time", &pos, NULL, NULL);
    if (mSlot == dNet::getMyNo()) {
        dFishFldShadow_c *fish = getFish();
        if (fish != NULL) {
            switch (fish->mParam.mSize) {
            case FISH_SIZE_TINY:
                dRumble::start(2, false);
                break;
            case FISH_SIZE_SMALL:
                dRumble::start(2, false);
                break;
            case FISH_SIZE_MEDIUM:
                dRumble::start(3, false);
                break;
            case FISH_SIZE_LARGE:
                dRumble::start(3, false);
                break;
            case FISH_SIZE_NARROW:
                dRumble::start(3, false);
                break;
            case FISH_SIZE_XLARGE:
                dRumble::start(3, false);
                break;
            case FISH_SIZE_HUGE:
                dRumble::start(4, false);
                break;
            case FISH_SIZE_FIN:
                dRumble::start(4, false);
                break;
            }
        }
    }
}

bool dUki_c::isInWater() const {
    return mState == STATE_FLOAT || mState == STATE_DROP || mState == STATE_BITE;
}

BOOL dUki_c::isJustLanded() const {
    if (mState == STATE_FLOAT && mTimer < 10) {
        return TRUE;
    }
    return FALSE;
}

void dUki_c::shoot(const mVec3_c *pos) {
    mTip.set(pos->x, 22.0f + pos->y, 6.0f + pos->z);
    setState(STATE_SHOT);
}

void dUki_c::setFishPos(const mVec3_c *pos) {
    mFishPos = *pos;
}

dFishFldShadow_c *dUki_c::getFish() const {
    if (mSlot > 3) {
        return NULL;
    }
    return l_bank.mFish[mSlot];
}

void dUki_c::setTarget(const mVec3_c *pos) {
    mTarget.set(pos->x, pos->y, pos->z);
}

void dUki_c::setPos(const mVec3_c *pos) {
    mPos[0].set(pos->x, pos->y, pos->z);
}

void dUki_c::setRodMtx(const mMtx_c *mtx) {
    mRodMtx = *mtx;
}

void dUki_c::setPos(f32 x, f32 y, f32 z) {
    mPos[0].set(x, y, z);
}

BOOL dUki_c::calcCast() {
    dBGCF::groundChk_c chk(&mPos[0], 0, 0, 0);
    if (chk.isUnderWater(mPos[0].y)) {
        if (mPos[0].y < chk.getWaterY() - 32.0f) {
            mPos[0].set(mTarget.x, chk.getWaterY(), mTarget.z);
        }
        return TRUE;
    }
    f32 t = mTimer;
    f32 dy = 0.1f * t - t * (0.0025f * t);
    if (dy < -5.0f) {
        dy = -5.0f;
    }
    mPos[0].y += dy;
    if (dy > 0.0f) {
        cLib::addCalcPosXZ(&mPos[0], mTarget, mCastRate, 5.0f, 1.0f);
    } else {
        cLib::chasePosXZ(&mPos[0], mTarget, 0.9f);
    }
    return FALSE;
}

BOOL dUki_c::calcReel() {
    BOOL done = FALSE;
    f32 height = mTip.y - mTarget.y;
    f32 speed = (4.0f * (7.5f + height) - height) / 55.0f;
    f32 accel = (55.0f * speed - height) / 3025.0f;
    f32 t = mTimer;
    mPos[0].y = mTarget.y + (speed * t - t * (accel * t));
    if (t > 27.5f && mPos[0].y < mTip.y) {
        mPos[0].y = mTip.y;
        done = TRUE;
    }
    mMtx_c mtx(mRodMtx);
    calcTipMtx(&mtx);
    mReelPos.set(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(&mReelPos, &mReelPos);
    f32 rest = cLib::addCalcPosXZ(&mPos[0], mReelPos, 0.077f, 5.0f, 0.6f);
    BOOL result = FALSE;
    if (done && 0.0f == rest) {
        result = TRUE;
    }
    return result;
}

BOOL dUki_c::calcReelEmpty() {
    mMtx_c mtx(mRodMtx);
    calcTipMtx(&mtx);
    mReelPos.set(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(&mReelPos, &mReelPos);
    f32 restXZ = cLib::addCalcPosXZ(&mPos[0], mReelPos, 0.08f, 5.0f, 0.3f);
    f32 restY = sLib::addCalc(&mPos[0].y, mReelPos.y, 0.08f, 5.0f, 0.1f);
    BOOL result = FALSE;
    if (0.0f == restXZ && 0.0f == restY) {
        result = TRUE;
    }
    return result;
}

BOOL dUki_c::calcReturn() {
    if (calcSwing()) {
        mMtx_c mtx(mRodMtx);
        calcTipMtx(&mtx);
        mTip.set(mtx._03, mtx._13, mtx._23);
        dWorld::toFieldPosition(&mTip, &mTip);
        f32 restXZ = cLib::addCalcPosXZ(&mPos[0], mTip, 0.11f, 5.0f, 0.3f);
        f32 restY = sLib::addCalc(&mPos[0].y, mTip.y, 0.1f, 5.0f, 0.3f);
        if (0.0f == restXZ && 0.0f == restY) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL dUki_c::calcSwing() {
    if (mTimer >= 46) {
        return TRUE;
    }
    sLib::addCalc(&mPos[0].y, 15.0f + mTip.y, 0.05f, 5.0f, 0.05f);
    mAngY -= 0x352;
    mPos[0].x += 1.1f * mAng(mAngY).sin();
    mPos[0].z += 1.1f * mAng(mAngY).cos();
    return FALSE;
}

void dUki_c::initTipPos() {
    mMtx_c mtx(mRodMtx);
    calcTipMtx(&mtx);
    mTip.set(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(&mTip, &mTip);
    mPos[0] = mTip;
    mAngY = dActor_c::targetAngleY(&mTip, &mTarget) - 0x4000;
}

void dUki_c::calcTipMtx(mMtx_c *mtx) {
    mtx->ZrotM(-0x3BFF);
    mtx->XrotM(-0x27FF);
    mMtx_c trans;
    PSMTXTrans(trans, 0.0f, -2.3f, 1.0f);
    PSMTXConcat(*mtx, trans, *mtx);
}

void dUki_c::updateTip() {
    mMtx_c mtx(mRodMtx);
    calcTipMtx(&mtx);
    mTip.set(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(&mTip, &mTip);
    mPlayerYaw = dPlayerMgr::getYaw(mSlot);
}

BOOL dUki_c::setMatAlpha(bool markDirty, const dBGCF::groundChk_c *chk) {
    if (mState == STATE_NONE) {
        return TRUE;
    }
    BOOL opaque = FALSE;
    int alpha = 0xFF;
    u32 i;
    nw4r::ut::Color matColor;
    nw4r::ut::Color amb;
    if (!markDirty && chk != NULL && chk->mWater != BG_WATER_NONE) {
        f32 depth = chk->getWaterY() - mPos[0].y;
        if (depth > 0.0f) {
            alpha -= (int)((1.0f / 3.5f) * (159.0f * depth));
        }
    }
    nw4r::g3d::ResMdl mdl = mRodMdl[mMdlIdx].getResMdl();
    if (!mdl.IsValid()) {
        return FALSE;
    }
    u32 num = mdl.GetResMatNumEntries();
    for (i = 0; i < num; i++) {
        nw4r::g3d::ResMat mat = mdl.GetResMat(i);
        if (mat.IsValid()) {
            nw4r::g3d::ScnMdl *scnMdl = nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(mRodMdl[mMdlIdx].getScn());
            nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mat.ptr()->id);
            if (access.IsValid()) {
                nw4r::g3d::ResMatChan chan = access.GetResMatChan(markDirty);
                if (chan.IsValid()) {
                    amb = nw4r::ut::Color(alpha, 0xFF, 0xFF, 0xFF);
                    chan.GXSetChanAmbColor(GX_COLOR0, amb);
                    matColor = nw4r::ut::Color(alpha, 0xFF, 0xFF, 0xFF);
                    chan.GXSetChanMatColor(GX_COLOR0, matColor);
                    if ((u8)alpha == 0xFF) {
                        opaque = TRUE;
                    }
                }
            }
        }
    }
    return opaque;
}

BOOL dUki_c::calcShot() {
    static f32 s_spread = 3.5f * nw4r::math::SinIdx(0x71C);
    f32 spread;
    f32 dy = 3.5f * mAng(mAngX).sin();
    f32 dz = 3.5f * mAng(mAngX).cos();
    dz *= -1.0f;
    spread = s_spread;
    for (int i = 0; i <= mNum; i++) {
        mPos[i].y += dy;
        mPos[i].z += dz;
        if (mNum == 2) {
            if (i == 1) {
                mPos[i].x += spread;
            } else if (i == 2) {
                mPos[i].x -= spread;
            }
        }
        mObjc[i].mPos = mPos[i];
        if (mTimer < 40) {
            mObjc[i].regist();
        }
    }
    return mTimer > 60;
}

void dUki_c::particleCallback(nw4r::ef::Particle *particle, u32 arg) {
    if (particle == NULL) {
        return;
    }
    mVec3_c pos;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        mVec3_c src;
        src = particle->mParameter.mPosition;
        dWorld::toFieldPosition(&pos, &src);
    } else {
        pos = particle->mParameter.mPosition;
    }
    dBGCF::groundChk_c chk(&pos, 0, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        if (chk.isUnderWater(pos.y)) {
            pos.y = chk.getWaterY();
            dEffect::create("af_hny_hamon_S01", &pos, NULL, NULL);
            if (particle->mParticleManager != NULL) {
                particle->mParticleManager->RetireParticle(particle);
            }
        }
    } else if (pos.y <= chk.getHeight(FALSE)) {
        if (particle->mParticleManager != NULL) {
            particle->mParticleManager->RetireParticle(particle);
        }
    }
}

void dUki_c::createWaterEffect(const char *name, const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, 0, 0, 0);
    mVec3_c effPos(pos->x, chk.getWaterY(), pos->z);
    dEffect::create(name, &effPos, NULL, NULL);
}

void dUki_c::createSplashEffect(const char *name, const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, 0, 0, 0);
    mVec3_c effPos(pos->x, chk.getWaterY(), pos->z);
    dEffect::create(name, &effPos, NULL, NULL);
    createSplash(&effPos);
}

void dUki_c::createSplash(const mVec3_c *pos) {
    dEffect::create("af_hny_mizutama_S", pos, NULL, NULL, particleCallback, 0);
}

u8 dUki_c::getFallAlpha(const mVec3_c *pos) {
    dFishField_c *field = g_fishField;
    if (field == NULL) {
        return 0xFF;
    }
    mVec3_c diff = *pos;
    diff -= field->mFallPos;
    f32 distSq = diff.x * diff.x + diff.z * diff.z;
    if (distSq < 9216.0f) {
        f32 front = diff.x * field->mFallNormal.x + diff.z * field->mFallNormal.z;
        if (front >= 0.0f && front < 96.0f) {
            f32 alpha = (1.0f / 9216.0f) * (255.0f * distSq);
            if (alpha > 255.0f) {
                alpha = 255.0f;
            }
            return alpha;
        }
    }
    return 0xFF;
}

dUkiBank_c::dUkiBank_c() : mResData(NULL) {
    memset(mUki, 0, sizeof(mUki));
    memset(mFish, 0, sizeof(mFish));
}

dUkiBank_c::~dUkiBank_c() {}

BOOL dUkiBank_c::load() {
    if (l_bank.mResData == NULL && dHeap::UkiHeap_p != NULL) {
        l_bank.mResData = l_bank.mLoader.request("/ItemToolSub/Uki.brres", 0, dHeap::UkiHeap_p);
        if (l_bank.mResData == NULL) {
            return FALSE;
        }
        nw4r::g3d::ResFile file(l_bank.mResData);
        file.Init();
        file.Bind(file);
    }
    return TRUE;
}

u32 dUkiBank_c::getHeapSize() {
    return 0x6200;
}

void dUkiBank_c::setUki(int slot, dUki_c *uki) {
    if (slot < 0 || slot >= UKI_SLOT_NUM) {
        return;
    }
    mUki[slot] = uki;
}

dUki_c *dUkiBank_c::getUki(int player, dFishFldShadow_c *fish) {
    if (player > 3) {
        return NULL;
    }
    dUki_c *uki = l_bank.mUki[player];
    if (uki != NULL) {
        dFishFldShadow_c *own = uki->getFish();
        if (own == NULL || own == fish) {
            u8 state = uki->mState;
            if (state == dUki_c::STATE_FLOAT || state == dUki_c::STATE_BITE || state == dUki_c::STATE_HOOKED ||
                (own == fish && state == dUki_c::STATE_REEL)) {
                if (dPlayerMgr::getState(player) != 0x19) {
                    return uki;
                }
            }
        }
    }
    return NULL;
}

dUki_c *dUki_c::getFloat(int player, dFishFldShadow_c *fish) {
    return l_bank.getUki(player, fish);
}

void dUki_c::setFish(int player, dFishFldShadow_c *fish) {
    if (player > 3) {
        return;
    }
    l_bank.mFish[player] = fish;
}

u32 dUki_c::getBankHeapSize() {
    return l_bank.getHeapSize();
}

BOOL dUki_c::loadRes() {
    return l_bank.load();
}

int dUki_c::getHeapSize() {
    return UKI_HEAP_SIZE;
}

void *dUki_c::getResData() {
    if (l_bank.mResData != NULL) {
        return l_bank.mResData;
    }
    return NULL;
}
