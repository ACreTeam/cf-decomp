#include <game/game/d_a_npc_sp_soncho.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/sLib/s_lib.hpp>

// Tortimer dozing at the town hall. Functions are in target .text order.

namespace dANpcSpSoncho {
// .text 0x5C
BASE_PROFILE(A_NPC_SP_SONCHO, dAcNpcSpSoncho_c);

// The current player's house in the town (NULL: none yet).
static inline dHome_c *getPlayerHome() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer());
}
} // namespace dANpcSpSoncho

// .text 0x204
int dAcNpcSpSoncho_c::preCreate() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (dANpcSpSoncho::getPlayerHome() != NULL &&
        !player->isFlag0(PRIVATE_FLAG0_JOB_CHANGED)) {
        mParam = SP_NPC_SONCHO_EVENT;
    }
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setRecept(&mSonchoTalk);
    return SUCCEEDED;
}

// .text 0x2B4
int dAcNpcSpSoncho_c::create() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    static const dState::mode_c<dAcNpcSpSoncho_c>::Entry modes[] = {
        {"待機", &dAcNpcSpSoncho_c::initWait, &dAcNpcSpSoncho_c::executeWait},
        {"デモ待機", &dAcNpcSpSoncho_c::initDemoWait,
         &dAcNpcSpSoncho_c::executeDemoWait},
        {"会話", &dAcNpcSpSoncho_c::initTalk, &dAcNpcSpSoncho_c::executeTalk},
        {"居眠り１", &dAcNpcSpSoncho_c::initDoze1,
         &dAcNpcSpSoncho_c::executeDoze1},
        {"居眠り２", &dAcNpcSpSoncho_c::initDoze2,
         &dAcNpcSpSoncho_c::executeDoze2},
        {"居眠り３", &dAcNpcSpSoncho_c::initDoze3,
         &dAcNpcSpSoncho_c::executeDoze3},
    };
    dState::mode_c<dAcNpcSpSoncho_c>::initialize("dAcNpcSpSoncho_c", modes,
                                                 ARRAY_SIZE(modes), false);

    if (dANpcSpSoncho::getPlayerHome() == NULL ||
        player->isFlag0(PRIVATE_FLAG0_JOB_CHANGED)) {
        // asleep in front of the town hall
        mPos.set(208.0f, 0.0f, 240.0f);
        mAngle.set(0, 0, 0);
        mAngle3D.set(0, 0, 0);
        mCheckGround = 0;
        mAnmSet.setAnm0(HMN_ANM_TTI_SLP1);
        mAnmSet.setAnm1(HMN_ANM_TTI_SLP1);
        mAnmSet.setAnm2(HMN_ANM_TTI_SLP1);
        changeMode(&dAcNpcSpSoncho_c::executeDoze1);
    }
    if (dNpcMng_c::isHolidayNpcToday() || dNpcMng_c::_8074EACD) {
        return FAILED;
    }
    return SUCCEEDED;
}

// .text 0x568
int dAcNpcSpSoncho_c::doDelete() {
    return dAcNpcSp_c::doDelete() != 0;
}

// .text 0x594
int dAcNpcSpSoncho_c::vtB0() {
    dState::base_c<dAcNpcSpSoncho_c>::execute();
    return SUCCEEDED;
}

// .text 0x5E8
void dAcNpcSpSoncho_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeMode(&dAcNpcSpSoncho_c::executeDemoWait);
        break;
    case DEMO_CMD_TALK:
        mSonchoTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpSoncho_c::executeTalk);
        break;
    case DEMO_CMD_SPEAK:
        mSonchoTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeMode(&dAcNpcSpSoncho_c::executeTalk);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        changeMode(&dAcNpcSpSoncho_c::executeWait);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

// .text 0xB1C: the partner's talk is over.
void dAcNpcSpSoncho_c::vt84() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
}

// .text 0xB44
void dAcNpcSpSoncho_c::initWait() {
    mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                      cNpcLookRange);
}

// .text 0xB6C
void dAcNpcSpSoncho_c::executeWait() {
}

// .text 0xB70
void dAcNpcSpSoncho_c::initDemoWait() {
}

// .text 0xB74
void dAcNpcSpSoncho_c::executeDemoWait() {
}

// .text 0xB78
void dAcNpcSpSoncho_c::initTalk() {
    mAng angle = getTalkAngle();
    mAng speed = getTalkTurnSpeed();
    mTalk.requestTurnTalk(angle, speed, FALSE);
}

// .text 0xBDC
void dAcNpcSpSoncho_c::executeTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
    }
}

// .text 0xC1C
void dAcNpcSpSoncho_c::initDoze1() {
    mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    mDozeCount = DOZE_LOOPS;
}

// .text 0xC60: the bubble at his head; after the count, a nod.
void dAcNpcSpSoncho_c::executeDoze1() {
    mMtx_c mtx(mNodeMtx12);
    mVec3_c ofs(0.0f, -3.0f, 0.0f);
    if (ofs != mVec3_c::Zero) {
        PSMTXMultVecSR(mtx, ofs, ofs);
        mMtx_c trans;
        PSMTXTrans(trans, ofs.x, ofs.y, ofs.z);
        PSMTXConcat(trans, mtx, mtx);
    }
    mSleepEffect.create("afm_owl_sleep", &mtx, 0);
    f32 last = getModel()->mAnm.mFrameMax;
    f32 frame = getModel()->mAnm.getFrame();
    if (frame == last - 1.0f) {
        if (sLib::calcTimer(&mDozeCount) == 0) {
            changeMode(&dAcNpcSpSoncho_c::executeDoze2);
        }
    }
}

// .text 0xF24
void dAcNpcSpSoncho_c::initDoze2() {
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_TTI_SLP2, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
}

// .text 0xF4C
void dAcNpcSpSoncho_c::executeDoze2() {
    if (mAction.mCanChange) {
        changeMode(&dAcNpcSpSoncho_c::executeDoze3);
    }
}

// .text 0x10B4
void dAcNpcSpSoncho_c::initDoze3() {
    mAction.requestAnm(PRIORITY_IDLE, HMN_ANM_TTI_SLP3, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
}

// .text 0x10DC
void dAcNpcSpSoncho_c::executeDoze3() {
    if (mAction.mCanChange) {
        changeMode(&dAcNpcSpSoncho_c::executeDoze1);
    }
}

// .text 0x1244
dAcNpcSpSoncho_c::talk_c::talk_c(dAcNpcSpSoncho_c *npc) : mpOwner(npc) {
    mMsgNo = 0;
}

// .text 0x1298
void dAcNpcSpSoncho_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
}

// .text 0x12CC
void dAcNpcSpSoncho_c::talk_c::start(dActor_c *target) {
    mpActActor = target;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (!player->isFlag0(PRIVATE_FLAG0_TORTIMER_MET)) {
        player->setFlag0(PRIVATE_FLAG0_TORTIMER_MET);
        if (player->isFlag0(PRIVATE_FLAG0_JOINED_AS_VISITOR)) {
            mMsgNo = MSG_FIRST_VISITOR;
        } else {
            mMsgNo = MSG_FIRST;
        }
        dItem::Item pellyKey((u16)SP_NPC_POST_GIRL);
        dAcNpc_c *partner =
            static_cast<dAcNpc_c *>(dNpcList::getSpByKey(pellyKey));
        if (partner == NULL) {
            dItem::Item phyllisKey((u16)SP_NPC_POST_GIRL2);
            partner = static_cast<dAcNpc_c *>(dNpcList::getSpByKey(phyllisKey));
        }
        if (partner != NULL) {
            setPartner(partner);
        }
    } else {
        mMsgNo = MSG_AGAIN;
    }
}

// .text 0x139C
void dAcNpcSpSoncho_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_soncho_0";
    info->mCode = mMsgNo;
}

// .text 0x13B4: "Pelican!" pleases Pelly.
void dAcNpcSpSoncho_c::talk_c::onAnswer(int arg) {
    int answer = mpController->getAnswer();
    if (mMessageCode == MSG_FIRST && answer == ANSWER_PELICAN) {
        dItem::Item pellyKey((u16)SP_NPC_POST_GIRL);
        dAcNpc_c *pelly =
            static_cast<dAcNpc_c *>(dNpcList::getSpByKey(pellyKey));
        if (pelly != NULL) {
            // emotion 0x1D: not named yet (her delight)
            pelly->mAction.requestEmotion(PRIORITY_IDLE, 0x1D, cNpcMorphFrames);
        }
    }
}

// .text 0x1420
void dAcNpcSpSoncho_c::talk_c::onCustomTag(int no) {
    switch (no) {
    case CUSTOM_PARTNER_WAIT: {
        dItem::Item pellyKey((u16)SP_NPC_POST_GIRL);
        dAcNpc_c *pelly =
            static_cast<dAcNpc_c *>(dNpcList::getSpByKey(pellyKey));
        if (pelly != NULL) {
            pelly->stopManpu();
            pelly->mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
        }
        break;
    }
    }
}

// .text 0x1488
int dAcNpcSpSoncho_c::getEarType() {
    return NPC_EAR_TYPE_TORTIMER;
}
