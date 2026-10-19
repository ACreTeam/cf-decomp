#include <game/game/d_a_npc_sp_racco.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_world.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/nw4r/ef/ef_particlemanager.h>

// Pascal the sea otter. Functions are in target .text order.

namespace dANpcSpRacco {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_RACCO, dAcNpcSpRakosuke_c);

static const dAcNpcSpRakosuke_c::State_s l_states[] = {
    {&dAcNpcSpRakosuke_c::initIdle, &dAcNpcSpRakosuke_c::executeIdle},
    {&dAcNpcSpRakosuke_c::initLook, &dAcNpcSpRakosuke_c::executeLook},
    {&dAcNpcSpRakosuke_c::initRemote, &dAcNpcSpRakosuke_c::executeRemote},
    {&dAcNpcSpRakosuke_c::initDive, &dAcNpcSpRakosuke_c::executeDive},
    {&dAcNpcSpRakosuke_c::initSwim, &dAcNpcSpRakosuke_c::executeSwim},
    {&dAcNpcSpRakosuke_c::initTalk, &dAcNpcSpRakosuke_c::executeTalk},
    {&dAcNpcSpRakosuke_c::initRemoteTalk,
     &dAcNpcSpRakosuke_c::executeRemoteTalk},
};

// .text 0x210: the particle callback of the dive splash: a drop that falls
// on the water or the ground makes a splash there.
static void splashCallback(nw4r::ef::Particle *ptcl, u32 kind) {
    // The ground splash only for drops this close above the ground.
    static const f32 l_groundDist = 32.0f;
    // Unreferenced (folded into nothing); kept for the .rodata tail.
    static const int l_unused = 5;
    if (ptcl != NULL) {
        mVec3_c pos;
        if (dScene::isOutdoorScene((u8)dScene::getCurrentScene())) {
            mVec3_c src;
            src = ptcl->mParameter.mPosition;
            dWorld::toFieldPosition(&pos, &src);
        } else {
            pos = ptcl->mParameter.mPosition;
        }
        dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_TOP, 0, 0);
        if (chk.mWater != BG_WATER_NONE) {
            if (chk.isUnderWater(pos.y)) {
                pos.y = chk.mWaterY;
                dEffect::create("afi_hny_watersplash_b", &pos, NULL, NULL);
                if (ptcl->mParticleManager != NULL) {
                    ptcl->mParticleManager->RetireParticle(ptcl);
                }
            }
        } else if (pos.y <= chk.getHeight(FALSE)) {
            const char *name = "afi_hny_watersplash_a";
            f32 ground = chk.getHeight(FALSE);
            if (nw4r::math::FAbs(ground - pos.y) <= l_groundDist) {
                pos.y = ground;
                dEffect::create(name, &pos, NULL, NULL);
                if (ptcl->mParticleManager != NULL) {
                    ptcl->mParticleManager->RetireParticle(ptcl);
                }
            }
        }
    }
}
} // namespace dANpcSpRacco

// .text 0x388
void dAcNpcSpRakosuke_c::onDemoCmd(int cmd, u8 member) {
    _1B70 = member;
    int permit = requestPermit(cmd, member, TRUE);
    switch (cmd) {
    case DEMO_CMD_START:
        if (permit == PERMIT_REMOTE_START || permit == PERMIT_LOCAL_START) {
            changeState(STATE_LOOK);
        }
        break;
    case DEMO_CMD_TALK:
        if (permit == PERMIT_REMOTE_TALK) {
            changeState(STATE_REMOTE_TALK);
        } else if (permit == PERMIT_LOCAL_TALK) {
            mRaccoTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        }
        break;
    case DEMO_CMD_END:
        if ((permit == PERMIT_END_SINGLE || permit == PERMIT_END_MULTI) &&
            mState == STATE_DIVE) {
            changeState(STATE_SWIM);
        }
        break;
    case CMD_RELEASE_PERMIT:
        if (permit == PERMIT_RELEASED && mState == STATE_DIVE) {
            changeState(STATE_SWIM);
        }
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0x4A8
bool dAcNpcSpRakosuke_c::canInteract(dDemoActor_c *actor) {
    if ((isLocalOwner() && mState != STATE_IDLE) ||
        (!isLocalOwner() && mState != STATE_REMOTE)) {
        return false;
    }
    return dAcNpcSp_c::canInteract(actor);
}

// .text 0x524
void dAcNpcSpRakosuke_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpRacco::l_states[state].mInit)();
}

// .text 0x55C: shares the state (gone: for the day).
void dAcNpcSpRakosuke_c::sendRecord(BOOL gone) {
    static const u8 l_status[] = {
        STATUS_BEACH, STATUS_BEACH, STATUS_BEACH, STATUS_DIVE,
        STATUS_SWIM,  STATUS_BEACH, STATUS_BEACH,
    };
    record_s rec;
    int unitX = 0;
    int unitZ = 0;
    int status;
    if (gone) {
        status = STATUS_GONE;
        mRecord.mUnitX = 0;
        mRecord.mUnitZ = 0;
        mRecord.mStatus = STATUS_GONE;
        dSaveData_c::getTown()->setFlag(SAVE_FLAG_RACCO_APPEARED);
    } else {
        status = l_status[mState];
        unitX = worldToUnit(mPos.x);
        unitZ = worldToUnit(mPos.z);
    }
    rec.mUnitX = unitX;
    rec.mUnitZ = unitZ;
    rec.mStatus = status;
    dNet::setRecord(NET_REC_RACCO, &rec, 0);
}

// .text 0x620
void dAcNpcSpRakosuke_c::recvRecord() {
    cLib::memCpy(&mRecord, dNet::getRecord(NET_REC_RACCO), sizeof(record_s));
}

// .text 0x660
void dAcNpcSpRakosuke_c::breakSnowmen() {
    daSnowMan_c::checkBreakAll(&mLookAt.mHeadPos, 24.0f);
}

// .text 0x670: steers dir away from the walls ahead.
void dAcNpcSpRakosuke_c::addWallPush(mVec3_c *dir) {
    u16 angle = mAngle.y.mAngle;
    f32 sin = nw4r::math::SinIdx(angle);
    f32 cos = nw4r::math::CosIdx(angle);
    f32 z = 16.0f * cos;
    mVec3_c ofs(16.0f * sin, 0.0f, z);
    mVec3_c pos = mPos + ofs;
    mVec3_c old = mLastPos + ofs;
    mVec3_c before = pos;
    mSwimAcch.check(60.0f, &pos, &old, mAngle.y, (int)this,
                    dBGCF::CHECK_FLOOR | dBGCF::CHECK_WALL |
                        dBGCF::CHECK_STEP_EXT_48 | dBGCF::CHECK_SET_POS |
                        dBGCF::CHECK_EDGE_WALLS,
                    TRUE);
    ofs = pos - before;
    if (ofs.normalizeRS()) {
        ofs *= 0.2f;
        *dir += ofs;
    }
}

// .text 0x808: the angle toward the sea from his beach unit.
void dAcNpcSpRakosuke_c::calcDiveAngle() {
    static const s16 l_angles[] = {
        -0x4000, // BG_ATTR_BR_RACCO_W
        0,       // BG_ATTR_BR_RACCO_S
        0x4000,  // BG_ATTR_BR_RACCO_E
        -0x2000, // BG_ATTR_BR_RACCO_SW
        0x2000,  // BG_ATTR_BR_RACCO_SE
    };
    dFdBase_c *field = dField::getInfo(FD_ID_CURRENT);
    int unitX = worldToUnit(mPos.x);
    int unitZ = worldToUnit(mPos.z);
    mAng angle = l_angles[field->getBgAttr(unitX, unitZ) - BG_ATTR_BR_RACCO_W];
    mVec3_c pos = mPos;
    pos.x += 64.0f * angle.sin();
    pos.z += 64.0f * angle.cos();
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_WATER, 0, 0);
    if (chk.mWater != BG_WATER_NONE) {
        mVec3_c dir = chk.mDir;
        s16 diff = (s16)(cM::atan2s(dir.x, dir.z) & 0xE000) - angle;
        if (diff >= 0x2000 || diff <= -0x2000) {
            angle = angle.mAngle + diff * 3 / 8;
        }
    }
    mDiveAngle = angle;
}

// .text 0x970
void dAcNpcSpRakosuke_c::fadeShadow(BOOL in) {
    int alpha = mShadowAlpha - 10;
    if (in) {
        alpha = mShadowAlpha + 10;
    }
    if (alpha >= 0xFF) {
        mShadowAlpha = 0xFF;
    } else if (alpha < 0) {
        mShadowAlpha = 0;
    } else {
        mShadowAlpha = alpha;
    }
    mVec3_c pos = mPos;
    dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_WATER, 0, 0);
    pos.y = chk.mWaterY;
    if (mShadow.create("af_npc_rakosuke_moya", &pos, &mAngle, NULL, 0)) {
        mShadow.setRegisterAlpha(mShadowAlpha, 0xFF, 1);
    }
}

// .text 0xA58: in front of the waterfall basin.
BOOL dAcNpcSpRakosuke_c::isNearFall() {
    static const f32 l_range = 96.0f;
    static const f32 l_rangeSq = l_range * l_range;
    dFishField_c *field = g_fishField;
    if (field == NULL) {
        return FALSE;
    }
    mVec3_c d = mPos;
    d -= field->mFallPos;
    if (d.x * d.x + d.z * d.z < l_rangeSq) {
        f32 dot = d.x * field->mFallNormal.x + d.z * field->mFallNormal.z;
        if (dot > -32.0f && dot < 0.1f) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0xB04
int dAcNpcSpRakosuke_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mRaccoTalk);
    return SUCCEEDED;
}

// .text 0xB50
int dAcNpcSpRakosuke_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (isLocalOwner()) {
        changeState(STATE_IDLE);
    } else {
        changeState(STATE_REMOTE);
    }
    recvRecord();
    BOOL gone = FALSE;
    if (mRecord.mStatus != STATUS_BEACH ||
        dSaveData_c::getRaw()->isFlag(SAVE_FLAG_RACCO_APPEARED)) {
        if (mRecord.mStatus == STATUS_GONE) {
            dSaveData_c::getTown()->setFlag(SAVE_FLAG_RACCO_APPEARED);
            gone = TRUE;
        } else if (dNpc::isPermitSceneOwner()) {
            dSaveData_c::getTown()->setFlag(SAVE_FLAG_RACCO_APPEARED);
            gone = TRUE;
        }
    }
    if (gone) {
        sendRecord(TRUE);
        return FAILED;
    }
    if (dNet::isSceneOwner()) {
        record_s rec;
        rec.mUnitX = 0;
        rec.mUnitZ = 0;
        rec.mStatus = STATUS_BEACH;
        dNet::setRecord(NET_REC_RACCO, &rec, 0);
        mRecord.mUnitX = 0;
        mRecord.mUnitZ = 0;
        mRecord.mStatus = STATUS_BEACH;
    }
    mVelocity.set(0.0f, 0.0f, 0.0f);
    if (mRecord.mStatus == STATUS_SWIM) {
        dFdBase_c::getUnitCenterPos(&mPos, mRecord.mUnitX, mRecord.mUnitZ);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
        mVec3_c dir = chk.mDir;
        if (dir.normalizeRS()) {
            dir *= 0.4f;
            dir.y = 0.0f;
            mVelocity = dir;
            s16 angle = cM::atan2s(mVelocity.x, mVelocity.z);
            mAngle.y = angle;
            mAngle3D.y = angle;
        }
        changeState(STATE_SWIM);
    }
    mTalked = 0;
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      64.0f);
    mLookAt.mYaw.mMax = 0x13FF;
    mLookAt.mFov = labs(0x13FF);
    if (mState == STATE_IDLE || mState == STATE_REMOTE) {
        moveFtrToPoliceBox();
    }
    return SUCCEEDED;
}

// .text 0xDAC
int dAcNpcSpRakosuke_c::doDelete() {
    if (mRecord.mStatus != STATUS_BEACH && dNet::isSceneOwner()) {
        BOOL alone = TRUE;
        for (int i = 0; i < NET_MEMBER_MAX; i++) {
            if (i != dNet::getMyNo() && dNet::isMember(i) &&
                dNet::getMemberScene(i) == dScene::SCENE_FIELD) {
                alone = FALSE;
                break;
            }
        }
        if (alone) {
            sendRecord(TRUE);
        }
    }
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0xE6C
int dAcNpcSpRakosuke_c::vtB0() {
    recvRecord();
    if (mState < STATE_COUNT) {
        (this->*dANpcSpRacco::l_states[mState].mExecute)();
    }
    if (mRecord.mStatus == STATUS_GONE) {
        deleteRequest();
    }
    return SUCCEEDED;
}

// .text 0xEE0
void dAcNpcSpRakosuke_c::initIdle() {
    mAction.requestWait(PRIORITY_IDLE);
    mShadowAlpha = 0;
}

// .text 0xF24 (+ a dead blr at 0xF38)
void dAcNpcSpRakosuke_c::executeIdle() {
    if (mRecord.mStatus != STATUS_BEACH) {
        changeState(STATE_DIVE);
    }
}

// .text 0xF3C
void dAcNpcSpRakosuke_c::initLook() {
    mAction.requestTurn(PRIORITY_DEMO, getAngleYToPlayer(_1B70), l_turnSpeed,
                        0);
    mStep = 0;
    mWarpDist = 0.0f;
}

// .text 0xFAC
void dAcNpcSpRakosuke_c::stepLook() {
    if (mAction.mActionId == ACTION_TURN && mAction.mCanChange) {
        mAction.requestWait(PRIORITY_DEMO);
        mStep++;
    }
}

namespace dANpcSpRacco {
static const dAcNpcSpRakosuke_c::Proc l_lookSteps[] = {
    &dAcNpcSpRakosuke_c::stepLook};
} // namespace dANpcSpRacco

// .text 0x100C
void dAcNpcSpRakosuke_c::executeLook() {
    if (mStep < ARRAY_SIZE(dANpcSpRacco::l_lookSteps)) {
        (this->*dANpcSpRacco::l_lookSteps[mStep])();
    }
    if (mRecord.mStatus != STATUS_BEACH) {
        changeState(STATE_DIVE);
    }
}

// .text 0x1070
void dAcNpcSpRakosuke_c::initRemote() {
    mWarpDist = 0.0f;
}

// .text 0x1080
void dAcNpcSpRakosuke_c::executeRemote() {
    if (isLocalOwner()) {
        if (isPermitSelf()) {
            fn_80013474();
            mRaccoTalk.start(dPlayerMgr::getNpcTargetActor(
                dPlayerMgr::PLAYER_INDEX_CURRENT));
            changeState(STATE_TALK);
        } else if (checkLocalOwner()) {
            fn_800134A4();
            changeState(STATE_IDLE);
        }
    } else if (mRecord.mStatus != STATUS_BEACH) {
        changeState(STATE_DIVE);
    }
}

// .text 0x1130
void dAcNpcSpRakosuke_c::initRemoteTalk() {
    mWarpDist = 0.0f;
}

// .text 0x1140 (+ a dead blr at 0x1154)
void dAcNpcSpRakosuke_c::executeRemoteTalk() {
    if (mRecord.mStatus != STATUS_BEACH) {
        changeState(STATE_DIVE);
    }
}

// .text 0x1158
void dAcNpcSpRakosuke_c::initDive() {
    calcDiveAngle();
    mWarpDist = 0.0f;
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    mAcceptMoveReq = 0;
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SEO_DIVE_BEFORE1, 0.0f, 0.0f,
                       m3d::PLAYMODE_INHERIT, 0);
    mStep = 0;
    mAngle.y = mAngle.y.mAngle + 0x8000;
    mAngle3D.y = mAngle.y;
    mSpeedF = 0.0f;
    mAccelY = 0.0f;
    mMaxSpeedY = 0.0f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mAudioObj.startSound(SE_Osborne_jump_fall_1_Bakuten);
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_RACCO_APPEARED);
}

// .text 0x123C: turn to the sea, then dive.
void dAcNpcSpRakosuke_c::stepDiveTurn() {
    mAng angle = mAngle.y;
    s16 left = 1.0f + (23.0f - getModel()->mAnm.getFrame());
    if (left <= 0) {
        angle = mDiveAngle;
        if (left == 0) {
            mAudioObj.startSound(SE_Osborne_jump_fall_2_Landing);
        }
    } else {
        sLib::addCalcAngle(&angle.mAngle, mDiveAngle, left, 0x7FFF);
    }
    int action = mAction.mActionId;
    mAngle.y = angle;
    mAngle3D.y = mAngle.y;
    if (action == ACTION_ANM && mAction.mCanChange) {
        static const int l_diveAnms[] = {
            HMN_ANM_SEO_DIVE1,
            HMN_ANM_SEO_DIVE2,
            HMN_ANM_SEO_DIVE3,
        };
        int anmIndex = cM::rndInt(3);
        mAction.requestAnm(PRIORITY_DEMO, l_diveAnms[anmIndex], 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mCheckGround = 0;
        mVelocity.set(0.0f, 0.0f, 0.0f);
        mStep = 1;
    }
}

// .text 0x1364
void dAcNpcSpRakosuke_c::stepDive() {
    int frame = getModel()->mAnm.getFrame() - 1.0f;
    if (frame == 4) {
        u16 angle = mAngle.y.mAngle;
        f32 sin = nw4r::math::SinIdx(angle);
        f32 cos = nw4r::math::CosIdx(angle);
        f32 vz = 0.56f * cos;
        mVelocity.set(0.56f * sin, 0.0f, vz);
        mAudioObj.startSound(SE_Osborne_jump_fall_3_Fall);
    } else if (frame == 45) {
        mVec3_c pos = mLookAt.mHeadPos;
        dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_WATER, 0, 0);
        pos.y = chk.mWaterY;
        dEffect::create("af_npc_rakosuke_wc", &pos, NULL, NULL);
        dEffect::create("af_hny_mizutama_L2", &pos, NULL, NULL,
                        dANpcSpRacco::splashCallback, 0);
        mDrawShadow = 0;
        mAudioObj.startSound(SE_Osborne_water_in);
    }
    if (frame >= 90) {
        fadeShadow(TRUE);
    }
    if (frame >= 35) {
        breakSnowmen();
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        if (mTalked) {
            dDemo::finishTalk(this);
            mStep++;
        } else {
            changeState(STATE_SWIM);
        }
    }
}

namespace dANpcSpRacco {
static const dAcNpcSpRakosuke_c::Proc l_diveSteps[] = {
    &dAcNpcSpRakosuke_c::stepDiveTurn,
    &dAcNpcSpRakosuke_c::stepDive,
};
} // namespace dANpcSpRacco

// .text 0x1560
void dAcNpcSpRakosuke_c::executeDive() {
    if (mStep < ARRAY_SIZE(dANpcSpRacco::l_diveSteps)) {
        (this->*dANpcSpRacco::l_diveSteps[mStep])();
    }
    mPos += mVelocity;
    if (isLocalOwnerHost() && dNet::isSceneOwner()) {
        fn_800134A4();
    }
}

// .text 0x1600
void dAcNpcSpRakosuke_c::initSwim() {
    mShadowAlpha = 0xFF;
    mWarpDist = 0.0f;
    mLookAt.setFront(PRIORITY_IDLE, l_lookPitchStep, l_lookYawStep);
    mObjc.mType &= ~dObjc::MASK_DEFAULT;
    mAcceptMoveReq = 0;
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_SEO_SWIM1, 0.0f, 0.0f,
                       m3d::PLAYMODE_INHERIT, 0);
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
    mPos.y = chk.mWaterY;
    mStep = 0;
    mCheckGround = 0;
    mDrawShadow = 0;
    if (mVelocity.normalizeRS()) {
        mVelocity *= 0.4f;
    }
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_RACCO_APPEARED);
}

// .text 0x1710
void dAcNpcSpRakosuke_c::stepSwim() {
    dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
    mVec3_c dir = chk.mDir;
    addWallPush(&dir);
    mPos.y = chk.mWaterY;
    if (dir.normalizeRS()) {
        f32 speed = 0.4f;
        if (!dNet::isSceneOwner()) {
            // Catch up with the scene owner's position.
            int unitX = worldToUnit(mPos.x);
            int unitZ = worldToUnit(mPos.z);
            BOOL behind = FALSE;
            if (unitZ < mRecord.mUnitZ) {
                behind = TRUE;
            } else if (unitZ == mRecord.mUnitZ && unitX != mRecord.mUnitX) {
                if (mAngle.y >= 0) {
                    if (unitX < mRecord.mUnitX) {
                        behind = TRUE;
                    }
                } else if (unitX > mRecord.mUnitX) {
                    behind = TRUE;
                }
            }
            if (behind) {
                speed *= 1.5f;
            }
        }
        dir *= speed;
        sLib::addCalc(&mVelocity.x, dir.x, 0.01f, speed, 0.0f);
        sLib::addCalc(&mVelocity.z, dir.z, 0.01f, speed, 0.0f);
    }
    mVec3_c next = mPos + mVelocity;
    dBGCF::groundChk_c nextChk(&next, dBGCF::LAYER_WATER, 0, 0);
    mVelocity.y = 0.0f;
    if (nextChk.getWaterY() < chk.getWaterY()) {
        mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_SEO_WATERFALL1, 0.0f, 14.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        u16 angle = mAngle.y.mAngle;
        f32 sin = nw4r::math::SinIdx(angle);
        f32 cos = nw4r::math::CosIdx(angle);
        mStep = 1;
        f32 vz = 0.4f * cos;
        mVelocity.set(0.4f * sin, 0.0f, vz);
    }
    if (isNearFall()) {
        fadeShadow(FALSE);
    } else {
        fadeShadow(TRUE);
    }
    // Off the south end of the map (unit row 104): gone for the day.
    if (worldToUnit(mPos.z) >= 0x68) {
        sendRecord(TRUE);
        deleteRequest();
    }
}

// .text 0x19A4: down the waterfall.
void dAcNpcSpRakosuke_c::stepWaterfall() {
    int frame = getModel()->mAnm.getFrame() - 1.0f;
    if (frame == 42.0f) {
        mVec3_c pos = mLookAt.mHeadPos;
        dBGCF::groundChk_c chk(&pos, dBGCF::LAYER_WATER, 0, 0);
        pos.y = chk.mWaterY;
        dEffect::create("af_npc_rakosuke_wc", &pos, NULL, NULL);
        dEffect::create("af_hny_mizutama_L2", &pos, NULL, NULL,
                        dANpcSpRacco::splashCallback, 0);
        mAudioObj.startSound(SE_Osborne_TakitsuboFallIn);
    }
    if (frame >= 100) {
        fadeShadow(TRUE);
    } else {
        fadeShadow(FALSE);
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_SEO_SWIM1, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        dBGCF::groundChk_c chk(&mPos, dBGCF::LAYER_WATER, 0, 0);
        mPos.y = chk.mWaterY;
        mStep = 0;
    }
}

namespace dANpcSpRacco {
static const dAcNpcSpRakosuke_c::Proc l_swimSteps[] = {
    &dAcNpcSpRakosuke_c::stepSwim,
    &dAcNpcSpRakosuke_c::stepWaterfall,
};
} // namespace dANpcSpRacco

// .text 0x1B48
void dAcNpcSpRakosuke_c::executeSwim() {
    if (mStep < ARRAY_SIZE(dANpcSpRacco::l_swimSteps)) {
        (this->*dANpcSpRacco::l_swimSteps[mStep])();
    }
    mPos += mVelocity;
    s16 angle = cM::atan2s(mVelocity.x, mVelocity.z);
    mAngle.y = angle;
    mAngle3D.y = angle;
    g_fishField->scareFish(-1.0f, &mPos);
    breakSnowmen();
    if (dNet::isSceneOwner() && mRecord.mStatus != STATUS_GONE) {
        sendRecord(FALSE);
    }
}

// .text 0x1C2C
void dAcNpcSpRakosuke_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
    mStep = 0;
    mWarpDist = 0.0f;
}

// .text 0x1CA4
void dAcNpcSpRakosuke_c::stepTalk() {
    if (!mTalk.isBusy()) {
        changeState(STATE_DIVE);
        mTalked = 1;
        sendRecord(FALSE);
    }
}

namespace dANpcSpRacco {
static const dAcNpcSpRakosuke_c::Proc l_talkSteps[] = {
    &dAcNpcSpRakosuke_c::stepTalk};
} // namespace dANpcSpRacco

// .text 0x1CFC
void dAcNpcSpRakosuke_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpRacco::l_talkSteps)) {
        (this->*dANpcSpRacco::l_talkSteps[mStep])();
    }
}

// .text 0x1D3C
dAcNpcSpRakosuke_c::talk_c::talk_c(dAcNpcSpRakosuke_c *npc) : mpOwner(npc) {
    init();
}

// .text 0x1D9C
dAcNpcSpRakosuke_c::talk_c::~talk_c() {
}

// .text 0x1DF4
void dAcNpcSpRakosuke_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
}

// .text 0x1E5C
void dAcNpcSpRakosuke_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    mMsgNo = MSG_START;
}

// .text 0x1EAC
void dAcNpcSpRakosuke_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_rakosuke";
    info->mCode = mMsgNo;
}

// .text 0x1EC4
void dAcNpcSpRakosuke_c::talk_c::onMessageStart(int arg) {
}

namespace dANpcSpRacco {
// .text 0x1EC8: countPockets filter.
static BOOL isScallop(const dItem::Item *item) {
    return item->isSame(dItem::Item(dItem::ITEM_IDX_SCALLOP));
}
} // namespace dANpcSpRacco

// .text 0x1F08
void dAcNpcSpRakosuke_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_SHARE: {
        dMsg::Comp_c *controller = mpController;
        controller->setNextMsgCode(MSG_SAYING + cM::rndInt(MSG_SAYING_NUM));
        break;
    }
    case MSG_RIGHT_ON:
    case MSG_BLEW_MIND: {
        u16 mask;
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->countPockets(
                dANpcSpRacco::isScallop, &mask) > 0) {
            for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
                if (mask & (1 << i)) {
                    mScallopSlot = i;
                    break;
                }
            }
            mpController->setNextMsgCode(MSG_SCALLOP_ASK);
        }
        break;
    }
    case MSG_GIVE:
        if (dPlayerMgr::Mgr_c::getCurrentPlayer()->findEmptyPocket(0) >= 0) {
            mOnMessageEnd = &talk_c::giveTreasure;
            mOnRequestEnd = &talk_c::afterGive;
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x20D0
void dAcNpcSpRakosuke_c::talk_c::onAnswer(int arg) {
    int cursor = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_SCALLOP_ASK:
        if (cursor == 0) {
            dItem::Item none;
            dPlayerMgr::Mgr_c::getCurrentPlayer()->setPocket(
                &none, mScallopSlot, FALSE);
            requestScallop();
            mOnRequestEnd = &talk_c::requestGive;
        }
        break;
    }
}

// .text 0x215C
void dAcNpcSpRakosuke_c::talk_c::preExecute() {
}

// .text 0x2160
void dAcNpcSpRakosuke_c::talk_c::onTalkEnd() {
}

// .text 0x2164
void dAcNpcSpRakosuke_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x21C8
void dAcNpcSpRakosuke_c::talk_c::requestScallop() {
    dItem::Item scallop(dItem::ITEM_IDX_SCALLOP);
    requestItemActEx(6, &scallop, 0, 0, 0, 2);
}

// .text 0x221C
void dAcNpcSpRakosuke_c::talk_c::requestGive() {
    mpController->setNextMsgCode(MSG_GIVE);
}

// .text 0x2228
void dAcNpcSpRakosuke_c::talk_c::giveTreasure() {
    dItem::Item item;
    dItemSel::Range_c range(0x29, 0x14);
    dItemSel::select(&item, 1, &range, 1, &dItemSel::sDefaultFilter, NULL, 0,
                     NULL);
    requestItemAct(&item, 0, 0);
    dPlayerMgr::Mgr_c::getCurrentPlayer()->pickUp(&item, FALSE);
}

// .text 0x22B8
void dAcNpcSpRakosuke_c::talk_c::afterGive() {
}

// .text 0x22BC
int dAcNpcSpRakosuke_c::getEarType() {
    return NPC_EAR_TYPE_PASCAL;
}
