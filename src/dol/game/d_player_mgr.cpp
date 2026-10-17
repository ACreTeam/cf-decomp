// Player manager: player actor and player data access for the local players
// and visitors. .text 800FBB58..8010263C.
// Names inferred from behavior and callers; unresolved requests retain their address names.
#include <game/game/d_demo.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/game/d_hand_item.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_hmn_head_item_mng.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_face.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/nw4r/math/math_triangular.h>
#include <cstring>
#include <cmath>
#include <game/game/d_net.hpp>
#include <game/game/d_ground_attr.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_a_player.hpp>

// Dependencies whose owners are not recovered yet.

extern "C" {
extern u8 lbl_8074EA7D;

BOOL fn_802C4404(dMiiCreateID_c *mii, u16 *out);

}

// The field task code (d_fg_mng_task.inc, #included by d_field_assessment.cpp).
void fgMngTask_procGroupAt(u8 x, u8 z, u8 layer);               // 800A3044
void fgMngTask_releaseAt(u8 scene, int x, int z, u32 layer);    // 800A3118

namespace dPlayerMgr {

const u8 sRemoteActionDelayFrames = 60;
const u8 sRemoteActionAlternateDelayFrames = 60;
const f32 sRemoteMoveSpeed = 2.0f;
const f32 sDefaultMoveSpeed = 0.75f;
const f32 sPlayerValue80750A7C = 12.0f;

// 8074E600 / 8074E602, set by the __sinit: look-at limits (lookAt, also read by d_a_player).
s16 sLookPitchMax = mAng::fromDegree(10.0f);
s16 sLookYawMax = mAng::fromDegree(45.0f);

// 8074AF10
u8 sSnowEffectWearThreshold = 0x3F;

// 805D2440
Mgr_c gManager;

// 800FBB58
Mgr_c::Mgr_c() {
    memset(mActors, 0, sizeof(mActors));
    for (int i = 0; i < PLAYER_INDEX_COUNT; i++) {
        mChangeState[i] = NULL;
        _10[i] = 0;
        clearPlayerSlot(i);
    }
    _20 = 0;
    resetTimer();
}

// 800FBBEC
Mgr_c::~Mgr_c() {}

// 800FBC48
mVec3_c *getPosition(int idx) {
    dAcPlayer_c *player = getPlayerActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return &player->mPos;
}

// 800FBC7C
dAcPlayer_c *getPlayerActor(int idx) {
    return getActor(idx);
}

// 800FBC80
dAcPlayer_c *getPlayerActorRaw(int idx) {
    return getActorRaw(idx);
}

// 800FBC84
dAcPlayer_c *getNpcTargetActor(int idx) {
    return getActor(idx);
}

// 800FBC88
mVec3_c *getSoundPosition(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return &player->mMdlCallback.mSoundPos;
}

// 800FBCBC
BOOL getMoveAngle(s16 *out) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (out != NULL && player->getMoveSpeed()) {
        *out = player->mMoveAngle;
        return TRUE;
    }
    return FALSE;
}

// 800FBD28
s16 getYaw(int idx) {
    dAcPlayer_c *player = getPlayerActor(idx);
    if (player == NULL) {
        return 0;
    }
    return player->mAngle.y;
}

// 800FBD5C
BOOL getUnitCoordinates(u32 *a, u32 *b, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    *a = player->mUnitX;
    *b = player->mUnitZ;
    return TRUE;
}

// 800FBDD4
mVec3_c *getHandPosition(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return &player->mMdlCallback.mHandPos;
}

// 800FBE08
void *getHandTransform(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return player->mMdlCallback.mHandMtx;
}

// 800FBE3C
BOOL canStartDemo() {
    if (gManager.mLocked) {
        return FALSE;
    }
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return isDemoStartState(player->mState);
}

// 800FBE98
BOOL canStartNetSave() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (isDemoStartState(state)) {
        return TRUE;
    }
    switch (state) {
    case STATE_FISHING_WAIT:
    case 0x24:
    case 0x26:
    case 0x28:
    case 0x5A:
    case 0x99:
        player->mActorFlags2 |= 0x4;
        return TRUE;
    case STATE_FISHING_PUT_BACK:
    case 0x5B:
        return TRUE;
    }
    return FALSE;
}

// 800FBF84
BOOL isDemoStartState(int state) {
    switch (state) {
    case STATE_IDLE:
    case STATE_WALK:
    case 0x23:
    case STATE_BED_WAIT:
    case STATE_CHAIR_WAIT:
        return TRUE;
    }
    return FALSE;
}

// 800FBFCC
void startDemoWait() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL && player->mState != STATE_EXIT_ROOM) {
        changeState(STATE_DEMO_WAIT, PLAYER_INDEX_CURRENT);
    }
}

// 800FC010
BOOL requestDemoWait() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_DEMO_WAIT) {
        return TRUE;
    }
    return changeState(STATE_DEMO_WAIT, PLAYER_INDEX_CURRENT);
}

// 800FC064
void setScriptedMove(BOOL on, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player != NULL) {
        if (on) {
            player->mActorFlags |= ACTOR_FLAG_SCRIPTED_MOVE;
        } else {
            player->mActorFlags &= ~ACTOR_FLAG_SCRIPTED_MOVE;
        }
    }
}

// 800FC0C0
u32 isScriptedMove(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->mActorFlags >> 20) & 1;
}

// 800FC0F8
BOOL watchShotBalloon() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_SLINGSHOT_WATCH) {
        player->mActorFlags2 |= ACTOR_FLAG2_SHOT_BALLOON;
        return TRUE;
    }
    if (state != STATE_SLINGSHOT_SHOOT) {
        return FALSE;
    }
    if (changeState(STATE_SLINGSHOT_WATCH, PLAYER_INDEX_CURRENT)) {
        player->mActorFlags2 |= ACTOR_FLAG2_SHOT_BALLOON;
        return TRUE;
    }
    return FALSE;
}

// 800FC194
BOOL landBalloonPresent(const mVec3_c *splashPos) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->mActorFlags2;
    player->mActorFlags2 = flags & ~ACTOR_FLAG2_SHOT_BALLOON;
    if (flags & ACTOR_FLAG2_SHOT_UFO) {
        return TRUE;
    }
    int state = player->mState;
    if (state == STATE_IDLE || state == STATE_SURPRISED) {
        return TRUE;
    }
    if (state != STATE_SLINGSHOT_WATCH) {
        return FALSE;
    }
    if (splashPos == NULL) {
        return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
    }
    if (changeState(STATE_SURPRISED, PLAYER_INDEX_CURRENT)) {
        s16 angle = dActor_c::targetAngleY(&player->mPos, splashPos);
        player->mTurnFrames = sLib::distanceAngle(angle, player->mAngle.y) / 10;
        player->mTargetAngle = angle;
        return TRUE;
    }
    return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
}

// 800FC2A4
BOOL watchShotUfo() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_SLINGSHOT_WATCH) {
        player->mActorFlags2 |= ACTOR_FLAG2_SHOT_UFO;
        return TRUE;
    }
    if (state != STATE_SLINGSHOT_SHOOT) {
        return FALSE;
    }
    if (changeState(STATE_SLINGSHOT_WATCH, PLAYER_INDEX_CURRENT)) {
        player->mActorFlags2 |= ACTOR_FLAG2_SHOT_UFO;
        return TRUE;
    }
    return FALSE;
}

// 800FC340
BOOL finishWatchUfo() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->mActorFlags2;
    player->mActorFlags2 = flags & ~ACTOR_FLAG2_SHOT_UFO;
    if (flags & ACTOR_FLAG2_SHOT_BALLOON) {
        return TRUE;
    }
    if (player->mState != STATE_SLINGSHOT_WATCH) {
        return FALSE;
    }
    return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
}

// 800FC3B0
BOOL listenToKK() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(STATE_LISTEN_KK, PLAYER_INDEX_CURRENT);
}

// 800FC404
void refreshEquippedUmbrellaDesign() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return;
    }
    if (isChangingEquipment()) {
        return;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return;
    }
    if (lbl_8074EA7D != 1) {
        return;
    }
    int slot = fn_8019AFE4() & 7;
    dItem::Item held = current->mEquipment.mHeld;
    if (!isDesignSlot(slot, &held)) {
        return;
    }
    if (dHmnToolBank_c::getItemToolType(&held) == 0xA && dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held.mId);
        dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held.mId);
    }
}

// 800FC4C0
BOOL startFortuneHit() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(STATE_FORTUNE_HIT, PLAYER_INDEX_CURRENT);
}

// 800FC514
BOOL startFortuneFaint() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_FORTUNE_HIT) {
        return FALSE;
    }
    return changeState(STATE_FORTUNE_FAINT, PLAYER_INDEX_CURRENT);
}

// 800FC568
void lock() {
    gManager.mLocked = TRUE;
}

// 800FC580
void unlock() {
    gManager.mLocked = FALSE;
}

// 800FC598
void holdToolAction() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL && (player->mActorFlags & ACTOR_FLAG_TOOL_OUT)) {
        int kind = player->mpToolBank->getToolType();
        if (kind != HMN_TOOL_UMBRELLA && kind != HMN_TOOL_BALLOON && kind != HMN_TOOL_WINDMILL) {
            gManager.mToolActionHeld = TRUE;
        }
    }
}

// 800FC604
void clearToolActionHold() {
    gManager.mToolActionHeld = FALSE;
}

// 800FC61C
BOOL isToolActionHeld() {
    return gManager.mToolActionHeld;
}

// 800FC630
BOOL enableCurrentActor() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->mActorFlags &= ~ACTOR_FLAG_DISABLED;
    return TRUE;
}

// 800FC674
BOOL refreshEquippedDesign(int slot) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    slot &= 7;
    BOOL shirt = isDesignSlot(slot, &current->mEquipment.mShirt);
    BOOL hat = isDesignSlot(slot, &current->mEquipment.mHat);
    BOOL held = isDesignSlot(slot, &current->mEquipment.mHeld);
    if (!shirt && !hat && !held) {
        return TRUE;
    }
    if (!shirt && !hat && held) {
        player->mActorFlags2 |= 0x20;
        if (!changeHeldItem(&current->mEquipment.mHeld)) {
            player->mActorFlags2 &= ~0x20;
        }
    } else {
        dItem::Item *pShirt = shirt ? &current->mEquipment.mShirt : NULL;
        dItem::Item *pHat = hat ? &current->mEquipment.mHat : NULL;
        dItem::Item *pHeld = held ? &current->mEquipment.mHeld : NULL;
        player->mActorFlags2 |= 0x20;
        if (!changeOutfitAndHeldItem(pShirt, pHat, pHeld)) {
            player->mActorFlags2 &= ~0x20;
        }
    }
    return TRUE;
}

// 800FC7E8
BOOL canStartIdleDemo() {
    if (gManager.mLocked) {
        return FALSE;
    }
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return (u32)player->mState <= STATE_WALK;
}

// 800FC858
BOOL hasCurrentActor() {
    return getActor(PLAYER_INDEX_CURRENT) != NULL;
}

// 800FC888
BOOL isNetSaveWaitState() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_IDLE || state == STATE_WALK || state == STATE_CHAIR_WAIT || state == STATE_BED_WAIT) {
        return TRUE;
    }
    return FALSE;
}

// 800FC8EC
void setDemoWait(BOOL a) {
    if (a) {
        startDemoWait();
    } else {
        endDemoWait();
    }
}

// 800FC8FC
BOOL flinch() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != STATE_DEMO_WAIT && state != STATE_FLINCH) {
        return FALSE;
    }
    return changeState(STATE_FLINCH, PLAYER_INDEX_CURRENT);
}

// 800FC958
BOOL turnToFront() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(STATE_TURN_FRONT, PLAYER_INDEX_CURRENT);
}

// 800FC9AC
BOOL isFacingFront() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_TURN_FRONT && player->mAngle.y == -0x8000) {
        return TRUE;
    }
    return FALSE;
}

// 800FCA04
BOOL beginDemoWait() {
    startDemoWait();
    return TRUE;
}

// 800FCA28
BOOL canStartMovementDemo() {
    if (gManager.mLocked) {
        return FALSE;
    }
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    switch (state) {
    case STATE_IDLE:
    case STATE_WALK:
    case STATE_RUN:
        return TRUE;
    }
    return FALSE;
}

// 800FCAA0
BOOL requestDemoStart() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (canStartMovementDemo() == FALSE) {
        return FALSE;
    }
    return changeState(STATE_DEMO_START, PLAYER_INDEX_CURRENT);
}

// 800FCAF4
BOOL showCatch(const dItem::Item *item) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_SHOW_CATCH) {
        return FALSE;
    }
    int kind = item->getKind();
    if (kind == dItem::KIND_INSECT) {
        player->mActorFlags |= ACTOR_FLAG_INSECT;
    } else if (kind == dItem::KIND_FISH) {
        player->mActorFlags &= ~ACTOR_FLAG_INSECT;
    } else {
        return FALSE;
    }
    player->mActionItem = item->mId;
    return changeState(STATE_SHOW_CATCH, PLAYER_INDEX_CURRENT);
}

// 800FCBA8
BOOL finishShowCatch() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOW_CATCH) {
        return FALSE;
    }
    player->mNextState = 6;
    return TRUE;
}

// 800FCBFC
BOOL isShowCatchFinished() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOW_CATCH) {
        return FALSE;
    }
    if (player->mAnmId == 0x53 || player->mAnmId == 0x54) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800FCC74
BOOL showCatchAt(const dItem::Item *item, const mVec3_c *pos) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_SHOW_CATCH_AT) {
        return TRUE;
    }
    int kind = item->getKind();
    if (kind == dItem::KIND_INSECT) {
        player->mActorFlags |= ACTOR_FLAG_INSECT;
    } else if (kind == dItem::KIND_FISH) {
        player->mActorFlags &= ~ACTOR_FLAG_INSECT;
    } else {
        return FALSE;
    }
    player->mActionItem = item->mId;
    player->mActorFlags |= 0x1000000;
    if (pos != NULL) {
        player->mShowPos = *pos;
    }
    return changeState(STATE_SHOW_CATCH_AT, PLAYER_INDEX_CURRENT);
}

// 800FCD60
BOOL putItemInHole(u32 unitX, u32 unitZ, BOOL fromTask) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->mTargetUnitX = unitX;
    player->mTargetUnitZ = unitZ;
    player->mActorFlags |= 0x1000000;
    if (fromTask) {
        player->mActorFlags |= 0x80000000;
    } else {
        player->mActorFlags &= ~0x80000000;
    }
    return changeState(STATE_PUT_ITEM_IN_HOLE, PLAYER_INDEX_CURRENT);
}

// Invalid hide-bone values leave the model visible.
static inline int getHideBone(const dItem::BITM *bitm) {
    int v = (s8)bitm->m_hideBone;
    return (u32)v < dItem::HIDE_BONE_COUNT ? v : dItem::HIDE_BONE_NONE;
}

// 800FCDFC
BOOL changeOutfitAndHeldItem(const dItem::Item *shirt, const dItem::Item *hat, const dItem::Item *held) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (isChangingEquipment()) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }

    if (shirt != NULL && shirt->mId != dItem::ITEM_ID_NONE) {
        int tmp = 0;
        if (dHmnClothMng_c::isOrgCloth(&tmp, shirt)) {
            player->mEquipFlags |= EQUIPMENT_CHANGE_CLOTH;
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt->mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt->mId);
        }
    }

    if (hat != NULL) {
        int bone = dItem::HIDE_BONE_NONE;
        if (hat->mId != dItem::ITEM_ID_NONE) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*hat);
            if (bitm != NULL) {
                bone = getHideBone(bitm);
            }
        }
        if (setHatHideBone(bone, player)) {
            if (current->mMiiMask != 0) {
                dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
            }
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HAT, hat->mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HAT, hat->mId);
        }
    }

    if (held != NULL) {
        if (held->mId == dItem::ITEM_ID_NONE) {
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
                player->mHeldItemChange = HELD_ITEM_CHANGE_REMOVE;
            }
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        } else {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
            if (bitm != NULL && getHideBone(bitm) == dItem::HIDE_BONE_HAND) {
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
                    player->mHeldItemChange = HELD_ITEM_CHANGE_EQUIP;
                }
                dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
                dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
            }
        }
    }

    player->_223C = 1;
    player->_2238 = 1;
    player->mNextState = 8;
    return changeState(STATE_CHANGE_OUTFIT_AND_HELD, PLAYER_INDEX_CURRENT);
}

// 800FD058
BOOL eatItem(const dItem::Item *item) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    switch (bitm->getEdibility()) {
    case dItem::EDIBILITY_FOOD:
        player->mActionItem = item->mId;
        return changeState(STATE_EAT, PLAYER_INDEX_CURRENT);
    case dItem::EDIBILITY_SPECIAL:
        player->mActionItem = item->mId;
        return changeState(STATE_EAT_SPECIAL, PLAYER_INDEX_CURRENT);
    }
    return FALSE;
}

// 800FD130
BOOL removeMaskForEating() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_CLEAR_BEE_STING);
    return changeState(STATE_EAT_REMOVE_MASK, PLAYER_INDEX_CURRENT);
}

// 800FD17C
BOOL endDemoWait() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return TRUE;
    }

    switch (player->mState) {
    case STATE_DEMO_WAIT:
        if (player->mAnmId == 0x31) {
            if (changeState(STATE_CHAIR_WAIT, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        } else if (player->mAnmId == 0x37) {
            if (changeState(STATE_BED_WAIT, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        }
        switch (player->mReactionKind) {
        case STATE_RUN:
        case STATE_MOVE_TO:
        case STATE_TURN_TO:
            if (changeState(STATE_REACT, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
            break;
        case STATE_DEMO_START:
            player->mReactionKind = 8;
            break;
        }
        break;
    case STATE_CHANGE_HELD_ITEM_DEMO:
        clearDemoFlags(player);
        return TRUE;
    case STATE_GET_UP:
    case 0x2D:
    case STATE_CHEST_CLOSE:
    case STATE_CHAIR_POSE:
    case 0x3A:
    case STATE_SHOW_CATCH_AT:
    case STATE_PHONE_HANG_UP:
    case STATE_SHOE_SHINE_END:
        clearDemoFlags(player);
        return TRUE;
    case STATE_ENTER_ROOM:
    case 0x81:
    case 0x83:
        if (player->mActorFlags2 & ACTOR_FLAG2_COCKROACH) {
            player->mActorFlags2 &= ~ACTOR_FLAG2_COCKROACH;
            if (changeState(STATE_SURPRISED, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        }
        break;
    case STATE_EXIT_DOOR:
        if (dNpcInfo::tanukichi_c::get()->mJobDone != 0) {
            if (changeState(STATE_REACT, PLAYER_INDEX_CURRENT)) {
                player->mReactionKind = REACTION_JOB_DONE;
                clearDemoFlags(player);
                dNpcInfo::tanukichi_c *info = dNpcInfo::tanukichi_c::get();
                info->mJobDone = 0;
                player->mActorFlags |= ACTOR_FLAG_RESUME_BGM;
                return TRUE;
            }
        } else if (isBedHairPending()) {
            clearBedHairPending();
            clearDemoFlags(player);
            return changeState(STATE_BED_HAIR, PLAYER_INDEX_CURRENT);
        }
        break;
    case STATE_SCENE_START:
    case 0xAA:
        return TRUE;
    }

    Mgr_c::getCurrentPlayer();
    u32 flags = player->mActorFlags;
    if ((flags & ACTOR_FLAG_TOOL_OUT) && !(flags & ACTOR_FLAG_TOOL_ACTIVE) && !isToolActionHeld()) {
        if (player->_208C >= 1.0f) {
            player->mActorFlags |= ACTOR_FLAG_TOOL_ACTIVE;
            if (changeState(STATE_IDLE, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        } else {
            int state = player->mpToolBank->getToolType() == HMN_TOOL_UMBRELLA ? 0x7B : 0x21;
            if (changeState(state, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        }
        if (changeState(STATE_IDLE, PLAYER_INDEX_CURRENT)) {
            clearDemoFlags(player);
            player->mActorFlags |= ACTOR_FLAG_RESUME_BGM;
            return TRUE;
        }
    } else {
        if (changeState(STATE_IDLE, PLAYER_INDEX_CURRENT)) {
            clearDemoFlags(player);
            player->mActorFlags |= ACTOR_FLAG_RESUME_BGM;
            return TRUE;
        }
    }
    return FALSE;
}

// 800FD510
BOOL getChairPosition(mVec3_c *out, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_CHAIR_WAIT) {
        return FALSE;
    }
    if (out != NULL) {
        out->set(player->mPos.x, player->mPos.y, player->mPos.z);
    }
    return TRUE;
}

// Shared by the "walk to (x, z)" requests: target position at the player's height.
static inline void setTargetXZ(dAcPlayer_c *player, f32 x, f32 z) {
    mVec3_c target(x, player->mPos.y, z);
    player->mTargetPos = target;
}

// 800FD588
BOOL sitOnChair(s16 angle, int type, f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == STATE_IDLE || state == STATE_WALK || (state == 0x23 && (player->mActorFlags2 & 0x4000)))) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mSeatSide = type;
    return changeState(STATE_SIT_DOWN, PLAYER_INDEX_CURRENT);
}

// 800FD664
BOOL isChairAnimation() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x31;
}

// 800FD6A8
BOOL getBedAnimationPosition(mVec3_c *out, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mAnmId != 0x37) {
        return FALSE;
    }
    if (out != NULL) {
        out->set(player->mPos.x, player->mPos.y, player->mPos.z);
    }
    return TRUE;
}

// 800FD720
BOOL getBedPosition(mVec3_c *out, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_BED_WAIT) {
        return FALSE;
    }
    if (out != NULL) {
        out->set(player->mPos.x, player->mPos.y, player->mPos.z);
    }
    return TRUE;
}

// 800FD798
BOOL lieOnBed(s16 angle, int type, f32 x, f32 z, int arg) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == STATE_IDLE || state == STATE_WALK || (state == 0x23 && (player->mActorFlags2 & 0x4000)))) {
        return FALSE;
    }
    if (type != LIE_FROM_BACK && type != LIE_FROM_FRONT) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mSeatSide = type;
    return changeState(STATE_LIE_DOWN, PLAYER_INDEX_CURRENT);
}

// 800FD88C
BOOL getUpFromBed(int type, int arg) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != STATE_BED_WAIT && state != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (type != 1 && type != 2) {
        return FALSE;
    }
    player->mSeatSide = type;
    player->mSeatPose = type == 1 ? 1 : 3;
    return changeState(STATE_GET_UP, PLAYER_INDEX_CURRENT);
}

// 800FD924
BOOL changeSeatedPose(int type) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != STATE_DEMO_WAIT && state != STATE_CHANGE_HAIR && state != STATE_CHANGE_MII_MASK) {
        return FALSE;
    }
    int value;
    switch (type) {
    case 0:
        value = 0;
        break;
    case 1:
        value = 1;
        break;
    case 2:
        value = 3;
        break;
    default:
        return FALSE;
    }
    player->mSeatPose = value;
    player->mSeatSide = type;
    return changeState(STATE_CHAIR_POSE, PLAYER_INDEX_CURRENT);
}

// 800FD9E0
BOOL openChest(int type, s16 angle, f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_CHEST_OPEN || player->mState == STATE_CHEST_WAIT || player->mState == STATE_CHEST_CLOSE) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mChestType = type;
    return changeState(STATE_CHEST_OPEN, PLAYER_INDEX_CURRENT);
}

// 800FDAA4
BOOL closeChest(int type) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_CHEST_CLOSE || state != STATE_CHEST_WAIT) {
        return FALSE;
    }
    player->mChestType = type;
    return changeState(STATE_CHEST_CLOSE, PLAYER_INDEX_CURRENT);
}

// 800FDB10
BOOL hasActionActor() {
    return getActor(PLAYER_INDEX_CURRENT) != NULL;
}

// 800FDB40
void keepAnimation() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        player->mActorFlags |= ACTOR_FLAG_KEEP_ANIMATION;
    }
}

// 800FDB78
BOOL exitDoor(int type) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_EXIT_DOOR) {
        return TRUE;
    }
    player->mDoorType = type;
    return changeState(STATE_EXIT_DOOR, PLAYER_INDEX_CURRENT);
}

// 800FDBDC
void putAwayToolForTalk(u32 mode) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return;
    }
    if (!(player->mActorFlags & ACTOR_FLAG_TOOL_OUT)) {
        return;
    }
    if (mode >= 2) {
        player->mActorFlags &= ~ACTOR_FLAG_TOOL_ACTIVE;
        return;
    }
    if (isToolActionHeld()) {
        player->mActorFlags &= ~ACTOR_FLAG_TOOL_ACTIVE;
        clearToolActionHold();
        return;
    }
    int kind = player->mpToolBank->getToolType();
    if (kind == HMN_TOOL_UMBRELLA || kind == HMN_TOOL_BALLOON || kind == HMN_TOOL_WINDMILL) {
        if (mode == 0) {
            if (kind == HMN_TOOL_UMBRELLA) {
                changeState(STATE_CLOSE_UMBRELLA, PLAYER_INDEX_CURRENT);
            } else {
                changeState(STATE_PUT_AWAY_TOOL, PLAYER_INDEX_CURRENT);
            }
        } else {
            player->mActorFlags &= ~ACTOR_FLAG_TOOL_ACTIVE;
        }
    } else {
        changeState(STATE_PUT_AWAY_TOOL, PLAYER_INDEX_CURRENT);
    }
}

// 800FDCC0
BOOL canFinishToolAction() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return TRUE;
    }
    return !(player->mActorFlags & ACTOR_FLAG_TOOL_ACTIVE);
}

// 800FDD04
BOOL isSceneTransitionState() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_DEMO_WAIT && (dDemo::isType(dDemo::TYPE_SCENE_START) || dDemo::isType(dDemo::TYPE_EXITSCENE))) {
        return TRUE;
    }
    if (state == STATE_EXIT_ROOM || state == STATE_ENTER_ROOM) {
        return TRUE;
    }
    return FALSE;
}

// 800FDD8C
BOOL changeHair(u32 hair, u32 color) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (hair >= 0x1A) {
        return FALSE;
    }
    if (color >= 8) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    current->mHair = hair;
    current->mHairColor = color;
    if (current->mMiiMask != 0) {
        dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
    }
    return changeState(STATE_CHANGE_HAIR, PLAYER_INDEX_CURRENT);
}

// 800FDE3C
BOOL drinkCoffee() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_DRINK_COFFEE, PLAYER_INDEX_CURRENT);
}

// 800FDE7C
BOOL isCoffeeAnimationFinished() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_DRINK_COFFEE && player->mAnmId == 0xB2) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800FDEE8
BOOL finishCoffee() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_FINISH_COFFEE, PLAYER_INDEX_CURRENT);
}

// 800FDF28
f32 getCoffeeAnimationFrame() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return 0.0f;
    }
    if (player->mAnmId == 0xB2) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            return anm->getFrame();
        }
    }
    return 0.0f;
}

// 800FDF7C
BOOL prepareMiiMask() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    u16 mii = 0;
    if (!current->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        if (!canUseMiiMask()) {
            return FALSE;
        }
        if (!fn_802C4404(&current->mMiiId, &mii)) {
            return FALSE;
        }
    }
    player->mMiiMask = mii;
    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_ON);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HAT, dItem::ITEM_ID_NONE);
    dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HAT, dItem::ITEM_ID_NONE);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, dItem::ITEM_ID_NONE);
    dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, dItem::ITEM_ID_NONE);
    return TRUE;
}

// 800FE094
BOOL putOnMiiMask() {
    if (prepareMiiMask()) {
        return changeState(STATE_MII_MASK, PLAYER_INDEX_CURRENT);
    }
    return FALSE;
}

// 800FE0D0
BOOL takeOffMiiMask() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (current->mMiiMask != 0) {
        dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
    }
    return changeState(STATE_MII_MASK, PLAYER_INDEX_CURRENT);
}

// 800FE140
BOOL putOnMiiMaskImmediately() {
    if (prepareMiiMask()) {
        return changeState(STATE_MII_MASK_IMMEDIATE, PLAYER_INDEX_CURRENT);
    }
    return FALSE;
}

// 800FE17C
BOOL changeMiiMask(u16 mii) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (!canUseMiiMask()) {
        return FALSE;
    }
    if (!mFace::isValidData(RFLDataSource_Official, mii, nullptr)) {
        return FALSE;
    }
    MiiData_c data;
    if (RFLGetCharInfo((RFLCharInfo *)&data, RFLDataSource_Official, nullptr, mii) != RFLErrcode_Success) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    current->mMiiId = *(const dMiiCreateID_c *)data.mCreateID;
    player->mMiiMask = mii;
    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_ON);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HAT, dItem::ITEM_ID_NONE);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, dItem::ITEM_ID_NONE);
    return changeState(STATE_CHANGE_MII_MASK, PLAYER_INDEX_CURRENT);
}

// 800FE2D4
BOOL takeOffMiiMaskImmediately() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (current->mMiiMask == 0) {
        return FALSE;
    }
    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
    return changeState(STATE_MII_MASK_IMMEDIATE, PLAYER_INDEX_CURRENT);
}

// 800FE360
u32 canUseMiiMask() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return 0;
    }
    return (player->mActorFlags >> 10) & 1;
}

// 800FE39C
BOOL enterDoor(s16 angle, int type, int target, f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_ENTER_DOOR) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mDoorType = type;
    player->mDoorTarget = target;
    if (target == -1) {
        return changeState(STATE_ENTER_DOOR, PLAYER_INDEX_CURRENT);
    }
    return changeState(STATE_WALK_THROUGH_DOOR, PLAYER_INDEX_CURRENT);
}

// 800FE484
BOOL walkThroughDoor(s16 angle, int target, f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_WALK_THROUGH_DOOR) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mActorFlags2 |= 0x20;
    player->mDoorTarget = target;
    return changeState(STATE_WALK_THROUGH_DOOR, PLAYER_INDEX_CURRENT);
}

// 800FE550
BOOL enterDoorDirect(s16 angle, int type, BOOL walkThrough, f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_ENTER_DOOR) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->mTargetAngle = angle;
    player->mDoorType = type;
    if (walkThrough) {
        return changeState(STATE_WALK_THROUGH_DOOR, PLAYER_INDEX_CURRENT);
    }
    return changeState(STATE_ENTER_DOOR, PLAYER_INDEX_CURRENT);
}

// 800FE634
BOOL isDoorAnimation() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int anm = player->mAnmId;
    if (anm == 0x5 || anm == 0x8) {
        return TRUE;
    }
    return FALSE;
}

// 800FE688
void getDemoWaitPose(int *outState, mVec3_c *outPos, s16 *outAngle) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return;
    }
    int state = player->mState;
    switch (state) {
    case STATE_BED_WAIT:
    case STATE_CHAIR_WAIT:
        break;
    case STATE_DEMO_WAIT: {
        int anm = player->mAnmId;
        if (anm == 0x31) {
            state = STATE_CHAIR_WAIT;
        } else if (anm == 0x37) {
            state = STATE_BED_WAIT;
        }
        break;
    }
    default:
        state = STATE_DEMO_WAIT;
        break;
    }
    *outState = state;
    *outPos = player->mPos;
    *outAngle = player->mAngle.y;
}

// 800FE74C
BOOL moveToEscorted(const mVec3_c *pos, int idx, f32 speed) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    player->mActorFlags2 |= ACTOR_FLAG2_ESCORTED;
    BOOL result = moveTo(pos, idx, speed);
    if (!result) {
        player->mActorFlags2 &= ~ACTOR_FLAG2_ESCORTED;
    }
    return result;
}

// 800FE7E0
BOOL moveTo(const mVec3_c *pos, int idx, f32 speed) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_MOVE_TO) {
        return TRUE;
    }
    if (state != STATE_DEMO_WAIT) {
        return FALSE;
    }
    player->mTargetPos = *pos;
    player->mTargetSpeed = speed;
    return changeState(STATE_MOVE_TO, idx);
}

// 800FE88C
BOOL turnTo(s16 angle, int idx) {
    return turnToInFrames(angle, 500, idx);
}

// 800FE89C
BOOL turnToInFrames(s16 angle, int frames, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_TURN_TO) {
        return TRUE;
    }
    if (state != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (frames <= 0) {
        frames = 500;
    }
    player->mTargetAngle = angle;
    player->mTurnFrames = frames;
    return changeState(STATE_TURN_TO, idx);
}

// 800FE938
BOOL lookAt(const mVec3_c *target, int a, BOOL pitch, int c) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (a <= 0) {
        a = 200;
    }
    if (c <= 0) {
        c = 200;
    }
    s16 yaw = 0;
    s16 tilt = 0;
    if (target != NULL) {
        s16 targetYaw = dActor_c::targetAngleY(&player->mPos, target);
        yaw = targetYaw - player->mAngle.y;
        if (yaw > sLookYawMax) {
            yaw = sLookYawMax;
        } else if (yaw < -sLookYawMax) {
            yaw = -sLookYawMax;
        }
        if (pitch) {
            mVec3_c diff = player->mMdlCallback.mHeadPos - *target;
            f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
            f32 dy = target->y - player->mMdlCallback.mHeadPos.y;
            tilt = cM::atan2s(dy, dist);
            if (tilt > sLookPitchMax) {
                tilt = sLookPitchMax;
            } else if (tilt < -sLookPitchMax) {
                tilt = -sLookPitchMax;
            }
        }
    }
    player->mLookYaw = yaw;
    player->mLookTilt = tilt;
    player->_22CA = a;
    player->_22CC = c;
    player->mActorFlags |= ACTOR_FLAG_LOOK_AT;
    return TRUE;
}

// 800FEA9C
BOOL isMovingTo(int idx) {
    return isState(0x3, idx);
}

// 800FEAA8
BOOL isTurningTo(int idx) {
    return isState(0x4, idx);
}

// 800FEAB4
BOOL isIdle(int idx) {
    return isState(0x0, idx);
}

// 800FEAC0
BOOL isWalking(int idx) {
    return isState(0x1, idx);
}

// 800FEACC
BOOL isWaitingForDemo(int idx) {
    return isState(0x8, idx);
}

// 800FEAD8
BOOL isStartingDemo(int idx) {
    return isState(0x7, idx);
}

// 800FEAE4
BOOL isState(int state, int idx) {
    dAcPlayer_c *player = getActorRaw(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == state;
}

// 800FEB34
int getState(int idx) {
    dAcPlayer_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 0xAB;
    }
    return player->mState;
}

// 800FEB68
BOOL changeShirt(const dItem::Item *shirt) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mShirt = *shirt;
    return changeEquipment(&equip, EQUIPMENT_CHANGE_SHIRT, EQUIPMENT_CHANGE_NORMAL);
}

// 800FEBE4
BOOL changeHat(const dItem::Item *hat) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mHat = *hat;
    return changeEquipment(&equip, EQUIPMENT_CHANGE_HAT, EQUIPMENT_CHANGE_NORMAL);
}

// 800FEC60
BOOL changeAccessory(const dItem::Item *acc) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mAcc = *acc;
    if (!equip.isAccessoryVisible()) {
        equip.mHat = dItem::ITEM_ID_NONE;
    }
    return changeEquipment(&equip, EQUIPMENT_CHANGE_ACCESSORY, EQUIPMENT_CHANGE_NORMAL);
}

// 800FECF8
BOOL changeHeldItem(const dItem::Item *held) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (isChangingEquipment()) {
        return FALSE;
    }
    if (held->mId == dItem::ITEM_ID_NONE) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) || (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) && dHmnToolBank_c::isHandTool(held))) {
            player->mHeldItemChange = HELD_ITEM_CHANGE_REMOVE;
            player->_223C = 1;
        }

        dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        player->mNextState = 8;
        player->mActorFlags |= 0x1000000;
        return changeState(STATE_CHANGE_HELD_ITEM, PLAYER_INDEX_CURRENT);
    } else {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
        if (bitm == NULL) {
            return FALSE;
        }
        if (getHideBone(bitm) != dItem::HIDE_BONE_HAND) {
            return FALSE;
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) || (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) && dHmnToolBank_c::isHandTool(held))) {
            player->mHeldItemChange = HELD_ITEM_CHANGE_EQUIP;
            player->_223C = 1;
        }
    }
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    player->mNextState = 8;
    player->mActorFlags |= 0x1000000;
    return changeState(STATE_CHANGE_HELD_ITEM, PLAYER_INDEX_CURRENT);
}

// 800FEEB0
BOOL isChangingEquipment() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case STATE_CHANGE_HELD_ITEM:
    case STATE_MII_MASK_IMMEDIATE:
    case STATE_CHANGE_DESIGN:
    case STATE_CHANGE_EQUIPMENT_QUICK:
    case STATE_TRICK_CHANGE:
    case STATE_CHANGE_HELD_ITEM_DEMO:
    case STATE_CHANGE_OUTFIT_AND_HELD:
        return TRUE;
    case STATE_CHANGE_EQUIPMENT:
        if (player->mAnmId == 0x24) {
            return TRUE;
        }
        if (player->mAnmId == 0x26 && player->getAnm()->getFrame() < 57.0f) {
            return TRUE;
        }
        break;
    case STATE_MII_MASK:
    case STATE_EAT_REMOVE_MASK:
        if (player->getAnm()->getFrame() < 57.0f) {
            return TRUE;
        }
        break;
    case STATE_CHANGE_HAIR:
    case STATE_CHANGE_MII_MASK:
        if (!player->getAnm()->isStop() || (player->mAnmId != 0xA6 && player->mAnmId != 0xA3)) {
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// 800FEFF4
mVec3_c *getToolOrPlayerPosition(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    dUki_c *obj = &player->mpToolBank->mFloat;
    if (obj != NULL && obj->mState != 0) {
        return &obj->mPos[0];
    }
    return &player->mPos;
}

// 800FF048
mVec3_c *getCurrentSoundPositionRaw() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return NULL;
    }
    return &player->mMdlCallback.mSoundPos;
}

// 800FF080
BOOL getGroundPositionAhead(mVec3_c *out) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    mAng angle = player->mAngle.y;
    out->x = player->mPos.x + 12.0f * angle.sin();
    out->z = player->mPos.z + 12.0f * angle.cos();
    mVec3_c pos(out->x, player->mPos.y, out->z);
    out->y = dBGCF::getGroundY(&pos, FALSE);
    return TRUE;
}

// 800FF150
u32 isMakingNoise(int idx) {
    dAcPlayer_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->mActorFlags >> 13) & 1;
}

// 800FF188
u32 isActingOnUnit(int idx) {
    dAcPlayer_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->mActorFlags2 >> 7) & 1;
}

// 800FF1C0
int getActionUnitAndKind(int *x, int *z, int idx) {
    dAcPlayer_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 7;
    }
    if (x == NULL || z == NULL) {
        return 7;
    }
    if (!(player->mActorFlags2 & ACTOR_FLAG2_UNIT_ACTION) && !(player->mActorFlags & ACTOR_FLAG_NOISY)) {
        return 7;
    }
    switch (player->mState) {
    case STATE_WALK:
        *x = player->mActionUnitX;
        *z = player->mActionUnitZ;
        return 0;
    case STATE_SHAKE_TREE_END:
        getActionUnitCoordinates(player, x, z);
        return 1;
    case 0x12:
        getActionUnitCoordinates(player, x, z);
        return 2;
    case 0x13:
        getActionUnitCoordinates(player, x, z);
        return 3;
    case 0xD:
        *x = (int)player->mPos.x >> 5;
        *z = (int)player->mPos.z >> 5;
        return 4;
    case 0x40:
        getActionUnitCoordinates(player, x, z);
        return 5;
    case 0x41:
        getActionUnitCoordinates(player, x, z);
        return 6;
    }
    return 7;
}

// 800FF354
BOOL requestBeeSting(int *outState) {
    int tmp;
    if (outState == NULL) {
        outState = &tmp;
    }
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        *outState = STATE_NONE;
        return FALSE;
    }
    *outState = player->mState;
    if (player->mActorFlags & ACTOR_FLAG_DEMO) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_FISHING_WAIT) {
        dUki_c::setFish(dNet::getMyNo(), NULL);
        *outState = STATE_FISHING_PUT_BACK;
        changeState(STATE_FISHING_PUT_BACK, PLAYER_INDEX_CURRENT);
        return FALSE;
    }
    if (state != STATE_IDLE && state != STATE_WALK) {
        return FALSE;
    }
    if (!dDemo::request9()) {
        return FALSE;
    }
    Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_BEE_STUNG);
    *outState = STATE_BEE_STING;
    return changeState(STATE_BEE_STING, PLAYER_INDEX_CURRENT);
}

// 800FF444
BOOL changeEquipment(const dEquip_c *equip, int mode, int kind) {
    if (equip == NULL) {
        return FALSE;
    }
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (isChangingEquipment()) {
        return FALSE;
    }
    if (kind == EQUIPMENT_CHANGE_PUT_ON_MII_MASK) {
        player->mEquipChangeMode = mode;
        return putOnMiiMaskImmediately();
    }
    if (kind == EQUIPMENT_CHANGE_TAKE_OFF_MII_MASK) {
        player->mEquipChangeMode = mode;
        return takeOffMiiMaskImmediately();
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }

    int tmp;
    BOOL changeShirt = FALSE;
    BOOL changeHat = FALSE;
    BOOL changeAcc = FALSE;

    dItem::Item shirt = equip->mShirt;
    if (shirt.isNotSame(current->mEquipment.mShirt) || ((mode == EQUIPMENT_CHANGE_SHIRT || mode == EQUIPMENT_CHANGE_DESIGN) && dHmnClothMng_c::isOrgCloth(&tmp, &shirt))) {
        changeShirt = TRUE;
    }
    dItem::Item hat = equip->mHat;
    if (hat.isNotSame(current->mEquipment.mHat) || ((mode == EQUIPMENT_CHANGE_HAT || mode == EQUIPMENT_CHANGE_DESIGN) && dHmnHeadItemMng_c::isOrgCap(&tmp, &hat))) {
        changeHat = TRUE;
    }
    dItem::Item acc = equip->mAcc;
    if (acc.isNotSame(current->mEquipment.mAcc)) {
        changeAcc = TRUE;
    }

    if (changeShirt && shirt.mId != dItem::ITEM_ID_NONE) {
        int tmp2 = 0;
        if (dHmnClothMng_c::isCloth(&shirt) || dHmnClothMng_c::isOrgCloth(&tmp2, &shirt)) {
            player->mEquipFlags |= EQUIPMENT_CHANGE_CLOTH;
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt.mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt.mId);
        }
    }

    if (changeHat) {
        int bone = dItem::HIDE_BONE_NONE;
        if (hat != dItem::ITEM_ID_NONE) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(hat);
            if (bitm != NULL) {
                bone = getHideBone(bitm);
            }
        }
        if (setHatHideBone(bone, player)) {
            if (current->mMiiMask != 0) {
                dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
            }
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HAT, hat.mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HAT, hat.mId);
        }
    }

    if (changeAcc) {
        if (acc == dItem::ITEM_ID_NONE) {
            player->mEquipFlags = (player->mEquipFlags & ~EQUIPMENT_SHOW_ACCESSORY) | EQUIPMENT_HIDE_ACCESSORY;
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
        } else {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(acc);
            if (bitm != NULL && getHideBone(bitm) == dItem::HIDE_BONE_ACCESSORY) {
                player->mEquipFlags = (player->mEquipFlags & ~EQUIPMENT_HIDE_ACCESSORY) | EQUIPMENT_SHOW_ACCESSORY;
                if (current->mMiiMask != 0) {
                    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
                }
                dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
                dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
            }
        }
    }

    switch (mode) {
    case EQUIPMENT_CHANGE_SHIRT:
    case EQUIPMENT_CHANGE_HAT:
    case EQUIPMENT_CHANGE_ACCESSORY:
        player->_2238 = 1;
        player->mNextState = 8;
        return changeState(STATE_CHANGE_EQUIPMENT, PLAYER_INDEX_CURRENT);
    case 4:
    case EQUIPMENT_CHANGE_DESIGN:
        player->_2238 = 1;
        player->mNextState = 8;
        return changeState(STATE_CHANGE_DESIGN, PLAYER_INDEX_CURRENT);
    case 0:
        player->_2238 = 1;
        return changeState(STATE_CHANGE_EQUIPMENT_QUICK, PLAYER_INDEX_CURRENT);
    case EQUIPMENT_CHANGE_TRICK:
        player->_2238 = 1;
        player->mNextState = 8;
        return changeState(STATE_TRICK_CHANGE, PLAYER_INDEX_CURRENT);
    }
    return FALSE;
}

// 800FF840
BOOL isSceneReadyForBgm() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return TRUE;
    }
    if (player->mState == STATE_SCENE_START) {
        return TRUE;
    }
    if ((player->mActorFlags & ACTOR_FLAG_RESUME_BGM) && dBG::getMdlBank()->isReady()) {
        return TRUE;
    }
    return FALSE;
}

// 800FF8B0
BOOL canHoldEquippedItem() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current != NULL && current->mEquipment.mHeld.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) || (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) && dHmnToolBank_c::isHandTool(&current->mEquipment.mHeld))) {
        return TRUE;
    }
    return FALSE;
}

// 80475A00: per hairstyle bit mask (hasHairFeature).
static const u16 sHairBits[26] = {
    0x0C0, 0x060, 0x180, 0x208, 0x102, 0x00A, 0x005, 0x081, 0x210, 0x006, 0x044, 0x030, 0x400,
    0x028, 0x084, 0x021, 0x240, 0x003, 0x084, 0x300, 0x110, 0x042, 0x011, 0x108, 0x0A0, 0x400,
};

// 800FF94C
BOOL hasHairFeature(u8 hair, int bit) {
    int bits = sHairBits[hair];
    return (bits & (1 << bit)) != 0;
}

// 800FF978
BOOL currentHairHasFeature(int bit) {
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    return hasHairFeature(current->mHair, bit);
}

// 80475A34 / 80475A40: getShoeColor, by gender.
static const u8 sMaleShoeColors[10] = {9, 1, 8, 2, 7, 4, 6, 5, 3, 0};
static const u8 sFemaleShoeColors[10] = {9, 4, 8, 0, 6, 1, 7, 3, 5, 2};

// 800FF9C4
u8 getShoeColor(u32 idx, u32 gender) {
    if (idx >= 10) {
        return 0;
    }
    dPrivateData_c *player = Mgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return 0;
    }
    if (gender == 2) {
        gender = player->mShoeKind;
    }
    if (gender == 0) {
        return sMaleShoeColors[idx];
    }
    return sFemaleShoeColors[idx];
}

// 800FFA50
BOOL isATriggeredFacing(const mVec3_c *pos, int maxAngle, f32 maxDist) {
    if (dPad::isATriggered() && isFacingPosition(pos, maxAngle, maxDist)) {
        return TRUE;
    }
    return FALSE;
}

// 800FFAC0
BOOL isPointerWalking() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if ((player->mActorFlags & ACTOR_FLAG_POINTER_WALK) && player->mState == STATE_WALK) {
        return TRUE;
    }
    return FALSE;
}

// 800FFB18
BOOL isToolState() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x18:
    case 0x19:
    case STATE_FISHING_WAIT:
    case 0x1B:
    case 0x1C:
    case STATE_FISHING_PUT_BACK:
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case STATE_PUT_ITEM_IN_HOLE:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x7D:
    case 0x89:
    case 0x8A:
    case 0x8D:
    case 0x8E:
    case 0x99:
    case STATE_SLINGSHOT_SHOOT:
    case STATE_SLINGSHOT_WATCH:
    case 0xA2:
        return TRUE;
    }
    return FALSE;
}

// 800FFBE4
BOOL isShakingTree() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_SHAKE_TREE || player->mState == STATE_SHAKE_TREE_END) {
        return TRUE;
    }
    return FALSE;
}

// 800FFC38
BOOL isPickUpState() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0x51:
    case 0x52:
    case 0x53:
    case STATE_PICK_UP:
    case 0x7E:
        return TRUE;
    }
    return FALSE;
}

// 800FFCA8
BOOL isFurnitureState() {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case STATE_CHAIR_POSE:
    case 0x3A:
    case 0x3B:
        return TRUE;
    }
    return FALSE;
}

// 800FFD10
BOOL isActionItemState() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0xE || state == 0x1F) {
        return TRUE;
    }
    return FALSE;
}

// 800FFD64
u16 getActiveActionItem() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return dItem::ITEM_ID_NONE;
    }
    int state = player->mState;
    if (state != 0xE && state != 0x1F) {
        return dItem::ITEM_ID_NONE;
    }
    return player->mActionItem;
}

// 800FFDC0
BOOL isFacingPosition(const mVec3_c *target, int maxAngle, f32 maxDist) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    const mVec3_c *pos = &player->mPos;
    maxDist = maxDist * maxDist;
    mVec3_c diff = *pos - *target;
    f32 dist = diff.x * diff.x + diff.z * diff.z;
    if (dist == 0.0f) {
        return TRUE;
    }
    if (dist > maxDist) {
        return FALSE;
    }
    s16 angle = dActor_c::targetAngleY(pos, target);
    return sLib::distanceAngle(angle, player->mAngle.y) <= maxAngle;
}

// 800FFEAC
void makeCurvedYawMatrix(mMtx_c *mtx, const mVec3_c *position, const mAng *angle) {
    mVec3_c pos;
    u16 tilt = dWorld::curvePositionSimple(&pos, position);
    PSMTXTrans(*mtx, pos.x, pos.y, pos.z);
    mtx->XrotM(mAng(tilt));
    mtx->YrotM(*angle);
}

// 800FFF2C
void makeCurvedAngleMatrix(mMtx_c *mtx, const mVec3_c *position, const mAng3_c *angle) {
    mVec3_c pos;
    s16 tilt = dWorld::curvePositionSimple(&pos, position);
    PSMTXTrans(*mtx, pos.x, pos.y, pos.z);
    mtx->XrotM(mAng(tilt + angle->x.mAngle));
    mtx->ZrotM(angle->z);
    mtx->YrotM(angle->y);
}

// 80475A50: random item indices per gender (getRandomShirt).
static const int sRandomShirts[8][2] = {
    {0x376, 0x37D}, {0x37C, 0x3A8}, {0x383, 0x3AF}, {0x384, 0x3C2},
    {0x391, 0x3D6}, {0x3A6, 0x3F2}, {0x410, 0x44B}, {0x425, 0x44D},
};

// 800FFFCC
u16 getRandomShirt(int gender) {
    int r = cM::rndInt(800);
    dItem::Item item(sRandomShirts[r & 7][gender & 1]);
    if (item.getKind() != dItem::KIND_CLOTH) {
        item.setFromIndex(dItem::ITEM_IDX_FOUR_BALL_SHIRT);
    }
    return item.mId;
}

// 8010003C
void clearEquipmentManagers() {
    dPlayerMngr_c::clearAllEquip();
    dHmnToolMng_c::clearPlayers();
    dHmnHeadItemMng_c::clearPlayers();
    dHmnClothMng_c::clearPlayers();
}

// 80100068
void clearPlayerEquipmentManagers(int value) {
    dPlayerMngr_c::clearEquip(value);
    dHmnToolMng_c::clear((u8)value);
    dHmnHeadItemMng_c::clear((u8)value);
    dHmnClothMng_c::clear((u8)value);
}

// 801000AC
BOOL isDesignEquipped(int slot) {
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (isDesignSlot(slot, &current->mEquipment.mShirt) || isDesignSlot(slot, &current->mEquipment.mHat)) {
        return TRUE;
    }
    return FALSE;
}

// 80100128
BOOL getActionAnimationFrame(f32 *out, int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mAnmId == 0x3C || player->mAnmId == 0x3D) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            *out = anm->getFrame();
            return TRUE;
        }
    }
    return FALSE;
}

// 80100194
f32 getIntermediateGroundHeight(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c) {
    f32 hb = dBGCF::getGroundY(b, FALSE);
    f32 ha = dBGCF::getGroundY(a, FALSE);
    f32 hc = c->y;
    if ((hb <= ha && hb >= hc) || (hb >= ha && hb <= hc)) {
        return hb;
    }
    return hc;
}

// 80100234
dActor_c *createActor(int member, int remote, u32 state, const mVec3_c *pos, const mAng3_c *angle) {
    return dPlayerMngr_c::createActor(member, remote, state, pos, angle);
}

// 80100238
void setPlayerSlot(u32 idx, int value) {
    if (idx < PLAYER_INDEX_COUNT) {
        gManager.mSlots[idx] = value;
    }
}

// 80100258
void clearPlayerSlot(u32 idx) {
    if (idx < PLAYER_INDEX_COUNT) {
        gManager.mSlots[idx] = PLAYER_SLOT_NONE;
    }
}

// 8010027C
void clearPlayers() {
    clearGuests();
    for (int i = 0; i < PLAYER_INDEX_COUNT; i++) {
        clearPlayerSlot(i);
    }
}

// 801002BC
void clearSaveRequestFlags() {
    for (int i = 0; i < PLAYER_INDEX_COUNT; i++) {
        dPrivateData_c *player = dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player != NULL && player->mPID.isValid()) {
            player->clearFlag0(PRIVATE_FLAG0_BEE_STUNG);
        }
    }
}

// 80100330
int getPlayerSlot(u32 idx) {
    if (idx >= PLAYER_INDEX_COUNT) {
        return PLAYER_SLOT_NONE;
    }
    return gManager.mSlots[idx];
}

// 80100358
BOOL setHatHideBone(u32 bone, dAcPlayer_c *player) {
    if (player == NULL) {
        return FALSE;
    }
    switch (bone) {
    case 0:
    case 1:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x3F) | 0x1;
        break;
    case 2:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x3F) | 0x2;
        break;
    case 3:
    case 4:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x3F) | 0x4;
        break;
    case 5:
    case 6:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x3F) | 0x8;
        break;
    case 7:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x3F) | 0x10;
        break;
    case dItem::HIDE_BONE_NONE:
        player->mEquipFlags = (player->mEquipFlags & (u16)~0x1F) | 0x20;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// 80100410
void clearDemoFlags(dAcPlayer_c *player) {
    if (player != NULL) {
        player->mActorFlags &= ~ACTOR_FLAG_DEMO;
        player->mActorFlags &= ~ACTOR_FLAG_KEEP_ANIMATION;
    }
}

// 8010042C
void getActionUnitCoordinates(dAcPlayer_c *player, int *x, int *z) {
    *x = player->mTargetUnitX;
    *z = player->mTargetUnitZ;
}

// 80100440
BOOL receiveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2, f32 f3) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!dHandItem::start(item, b, c, d, player, (dDemoActor_c *)partner, getItemExchangeOffsetY(), f1, 0.0f, f2, 0.0f, f3)) {
        return FALSE;
    }
    if (dHandItem::request(dHandItem_c::STATE_LOAD, player, dHandItem::cDefaultReach) == FALSE) {
        return FALSE;
    }
    return changeState(STATE_RECEIVE_ITEM, PLAYER_INDEX_CURRENT);
}

// 80100548
f32 getItemExchangeOffsetY() {
    return -4.0f;
}

// 80100550
BOOL giveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!dHandItem::start(item, b, c, d, player, (dDemoActor_c *)partner, getItemExchangeOffsetY(), f1, 0.0f, f2, 0.0f, 0.0f)) {
        return FALSE;
    }
    if (dHandItem::request(dHandItem_c::STATE_APPEAR, player, dHandItem::cDefaultReach) == FALSE) {
        return FALSE;
    }
    return changeState(STATE_GIVE_ITEM, PLAYER_INDEX_CURRENT);
}

// 80100644
BOOL passHandItem(BOOL reach) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (reach) {
        return changeState(STATE_HAND_ITEM_REACH, PLAYER_INDEX_CURRENT);
    }
    return changeState(STATE_HAND_ITEM_TAKE, PLAYER_INDEX_CURRENT);
}

// 801006A8
BOOL returnHandItem() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (dHandItem::request(dHandItem::getUseType(), player, dHandItem::cDefaultReach) == FALSE) {
        return FALSE;
    }
    return changeState(STATE_HAND_ITEM_RETURN, PLAYER_INDEX_CURRENT);
}

// 80100714
BOOL setShoeColor(u8 shoeColor, u8 b) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOE_SHINE) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    current->mShoeColor = shoeColor;
    current->mShoeKind = b;
    player->mNextState = 0x1E;
    return TRUE;
}

// 801007AC
BOOL pickUpPhone() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_PHONE_PICK_UP || player->mState == STATE_PHONE_TALK || player->mState == STATE_PHONE_HANG_UP) {
        return FALSE;
    }
    return changeState(STATE_PHONE_PICK_UP, PLAYER_INDEX_CURRENT);
}

// 80100804
BOOL hangUpPhone() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_PHONE_HANG_UP || state != STATE_PHONE_TALK) {
        return FALSE;
    }
    return changeState(STATE_PHONE_HANG_UP, PLAYER_INDEX_CURRENT);
}

// 80100860
BOOL requestInsectBite() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_FISHING_WAIT) {
        dUki_c::setFish(dNet::getMyNo(), NULL);
        changeState(STATE_FISHING_PUT_BACK, PLAYER_INDEX_CURRENT);
        return FALSE;
    }
    if (state != STATE_IDLE && state != STATE_WALK) {
        return FALSE;
    }
    return changeState(STATE_INSECT_BITE, PLAYER_INDEX_CURRENT);
}

// 801008E4
BOOL isUsingNet(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->mActorFlags & ACTOR_FLAG_TOOL_ACTIVE)) {
        return FALSE;
    }
    if (player->mpToolBank != NULL && player->mpToolBank->getToolType() == HMN_TOOL_NET) {
        return TRUE;
    }
    return FALSE;
}

// 8010094C
BOOL isRunAnimation(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x3;
}

// 8010098C
BOOL startChat() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_CHAT) {
        return TRUE;
    }
    return changeState(STATE_CHAT, PLAYER_INDEX_CURRENT);
}

// 801009E0
BOOL playEmotion(u32 type) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (type >= 0x1E) {
        return FALSE;
    }
    if (player->mState == STATE_EMOTION) {
        return FALSE;
    }
    player->mEmotion = type;
    return changeState(STATE_EMOTION, PLAYER_INDEX_CURRENT);
}

// 80100A54
BOOL reactDebtPaid() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_REACT) {
        return FALSE;
    }
    player->mReactionKind = REACTION_DEBT_PAID;
    return changeState(STATE_REACT, PLAYER_INDEX_CURRENT);
}

// 80100AB0
BOOL isReactionFinished() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_REACT) {
        return FALSE;
    }
    if (player->mAnmId == 0x95) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80100B24
BOOL reactToCockroaches() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_REACT) {
        return FALSE;
    }
    player->mReactionKind = REACTION_COCKROACH;
    return changeState(STATE_REACT, PLAYER_INDEX_CURRENT);
}

// 80100B80
u32 isCockroachReactionFinished() {
    return isReactionFinished();
}

// 80100B84
BOOL openMailbox(f32 x, f32 z) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_MAILBOX_OPEN || player->mState == STATE_MAILBOX_CLOSE) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    return changeState(STATE_MAILBOX_OPEN, PLAYER_INDEX_CURRENT);
}

// 80100C28
BOOL closeMailbox() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_MAILBOX_CLOSE || state != STATE_MAILBOX_OPEN) {
        return FALSE;
    }
    return changeState(STATE_MAILBOX_CLOSE, PLAYER_INDEX_CURRENT);
}

// 80100C84
BOOL requestFaint(BOOL a) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mActorFlags & ACTOR_FLAG_DEMO) {
        return FALSE;
    }
    int state = player->mState;
    if (state != STATE_IDLE && state != STATE_WALK) {
        return FALSE;
    }
    if (!dDemo::request10()) {
        return FALSE;
    }
    if (!a) {
        player->mActorFlags2 |= 1;
    } else {
        player->mActorFlags2 &= ~1;
    }
    return changeState(STATE_FAINT, PLAYER_INDEX_CURRENT);
}

// 80100D44
BOOL isSafeFromInsects() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0x7:
    case 0x8:
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
        return TRUE;
    }
    if (player->mActorFlags & ACTOR_FLAG_DEMO) {
        return TRUE;
    }
    return dDemo::isType(dDemo::TYPE_9) != 0;
}

// 80100DD0
BOOL isScaringCockroaches(int idx) {
    dAcPlayer_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0x1:
    case 0x2:
    case 0x27:
    case 0x29:
        return TRUE;
    }
    return FALSE;
}

// 80100E3C
BOOL boardBus() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_BUS_BOARD, PLAYER_INDEX_CURRENT);
}

// 80100E7C
BOOL isOnBus() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == STATE_BUS_BOARDED;
}

// 80100EC0
BOOL rideBus() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_BUS_RIDE, PLAYER_INDEX_CURRENT);
}

// 80100F00
BOOL getOffBus(s16 angle, f32 frame) {
    dAcPlayer_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->mAngle.x = 0;
    player->mAngle.y = angle;
    player->mAngle.z = 0;
    if (changeState(STATE_BUS_GET_OFF, PLAYER_INDEX_CURRENT)) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            anm->setFrame(frame);
        }
        return TRUE;
    }
    return FALSE;
}

// 80100F98
BOOL isOffBus() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == STATE_BUS_GOT_OFF;
}

// 80100FDC
BOOL leaveBus() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_BUS_LEAVE, PLAYER_INDEX_CURRENT);
}

// 8010101C
u8 isAxeInFountain() {
    return gManager.mAxeInFountain;
}

// 80101030
void setAxeInFountain() {
    gManager.mAxeInFountain = TRUE;
}

// 80101048
void clearAxeInFountain() {
    gManager.mAxeInFountain = FALSE;
}

// 80101060
BOOL changeHeldItemForDemo(const dItem::Item *held) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    if (held->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
        if (bitm == NULL) {
            return FALSE;
        }
        if (getHideBone(bitm) != dItem::HIDE_BONE_HAND) {
            return FALSE;
        }
        player->mHeldItemChange = HELD_ITEM_CHANGE_EQUIP;
        player->_223C = 1;
        dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    }
    return changeState(STATE_CHANGE_HELD_ITEM_DEMO, PLAYER_INDEX_CURRENT);
}

// 80101168
BOOL receiveGoddessItem() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(STATE_GODDESS_ITEM, PLAYER_INDEX_CURRENT);
}

// 801011BC
BOOL canGoddessLeave() {
    return TRUE;
}

// 801011C4
BOOL getScriptedActionPosition(mVec3_c *out) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->mActorFlags & ACTOR_FLAG_TOOL_OUT)) {
        return FALSE;
    }
    if (player->mState != STATE_GODDESS_ITEM) {
        return FALSE;
    }
    if (player->mNextState != 0x20) {
        return FALSE;
    }
    *out = player->mScriptedActionPos;
    return TRUE;
}

// 8010125C
BOOL startShoeShine(s16 frames, f32 speed) {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    mVec3_c target(1082.0f, 0.0f, 390.0f);
    player->mTargetPos = target;
    player->mTargetSpeed = speed;
    player->mTurnFrames = frames;
    return changeState(STATE_SHOE_SHINE_START, PLAYER_INDEX_CURRENT);
}

// 801012FC
BOOL finishShoeShine() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOE_SHINE) {
        return FALSE;
    }
    return changeState(STATE_SHOE_SHINE_END, PLAYER_INDEX_CURRENT);
}

// 80101350
BOOL turnToTalkActor() {
    dAcPlayer_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_TURN_TO_TALKER) {
        return TRUE;
    }
    if (state != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(STATE_TURN_TO_TALKER, PLAYER_INDEX_CURRENT);
}

// 801013B4
BOOL beginDemo() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(STATE_DEMO_START, PLAYER_INDEX_CURRENT);
}

// 801013F4
u32 *getTimer() {
    return &gManager.mTimer;
}

// 80101408
void resetTimer() {
    gManager.mTimer = 0xD2F0;
}

// 80101424
void reset() {
    clearPlayers();
    resetTimer();
    gManager.mDailyHairUpdated = 0;
    clearBedHairPending();
    gManager.mDailyTanUpdated = 0;
    gManager.mCockroachNoticed = 0;
    gManager.mAxeInFountain = 0;
    gManager.mLocked = 0;
    gManager.mToolActionHeld = 0;
    gManager.mSaveRequestFlagsCleared = 0;
    clearEquipmentManagers();
}

// 80101490
BOOL isDailyHairUpdated() {
    return gManager.mDailyHairUpdated;
}

// 801014A4
void markDailyHairUpdated() {
    gManager.mDailyHairUpdated = TRUE;
}

// 801014BC
BOOL isBedHairPending() {
    return gManager.mBedHairPending;
}

// 801014D0
void setBedHairPending() {
    gManager.mBedHairPending = TRUE;
}

// 801014E8
void clearBedHairPending() {
    gManager.mBedHairPending = FALSE;
}

// 80101500
BOOL isDailyTanUpdated() {
    return gManager.mDailyTanUpdated;
}

// 80101514
void markDailyTanUpdated() {
    gManager.mDailyTanUpdated = TRUE;
}

// 8010152C
u8 isCockroachNoticed() {
    return gManager.mCockroachNoticed;
}

// 80101540
void markCockroachNoticed() {
    gManager.mCockroachNoticed = TRUE;
}

// 80101558
BOOL areSaveRequestFlagsCleared() {
    return gManager.mSaveRequestFlagsCleared;
}

// 8010156C
void markSaveRequestFlagsCleared() {
    gManager.mSaveRequestFlagsCleared = TRUE;
}

// 80101584
BOOL isDesignSlot(int slot, const dItem::Item *item) {
    if (item->isOrgDesign()) {
        int design = dItem::seeker_c::get()->findLike(*item) & 7;
        if (design == slot) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801015F4
u32 getHeapSize() {
    return 0xEE880;
}

// 80101600
u32 getGeneralHeapSize() {
    return 0x1800;
}

// 80101608
u32 getHeadHeapSize() {
    return 0xC00;
}

// 80101610
u32 getAccessoryHeapSize() {
    return 0x800;
}

// 80101618
u32 getMiiHeapSize() {
    return 0x38820;
}

// 80101624
dPrivateData_c *Mgr_c::getPlayer(int player) {
    BOOL inTown = FALSE;
    if (player >= 0 && player < PLAYER_SLOT_TOWN_COUNT) {
        inTown = TRUE;
    }
    if (inTown) {
        return dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, player);
    }
    return gManager.mGuests.get(player);
}

// 80101694
dPrivateData_c *Mgr_c::getNetPlayer(int member) {
    return getPlayer(getPlayerSlot(member));
}

// 801016B8
dPrivateData_c *Mgr_c::getPlayer(const dPersonalID_c *pid) {
    return getPlayer(findPersonalIDSlot(pid));
}

// 801016DC
dPrivateData_c *Mgr_c::getPlayerRaw(int player) {
    BOOL inTown = FALSE;
    if (player >= 0 && player < PLAYER_SLOT_TOWN_COUNT) {
        inTown = TRUE;
    }
    if (inTown) {
        return dPrivateData_c::getRaw(dSaveData_c::getRaw()->mPlayers, player);
    }
    return gManager.mGuests.getRaw(player);
}

// 8010174C
dPrivateData_c *Mgr_c::getNetPlayerRaw(int member) {
    return getPlayerRaw(getPlayerSlot(member));
}

// 80101770
dPrivateData_c *Mgr_c::getCurrentPlayer() {
    return getNetPlayer(dNet::getMyNo());
}

// 80101794
dPrivateData_c *Mgr_c::getCurrentPlayerRaw() {
    return getNetPlayerRaw(dNet::getMyNo());
}

// 801017B8
int getCurrentPlayerSlot() {
    return getPlayerSlot(dNet::getMyNo());
}

// 801017DC
void clearGuests() {
    gManager.mGuests.clear();
}

// 801017EC
int findPersonalIDSlot(const dPersonalID_c *pid) {
    int slot = PLAYER_SLOT_NONE;
    if (pid->isValid()) {
        int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, pid);
        if (idx == -1) {
            for (int i = 0; i < PLAYER_SLOT_GUEST_COUNT; i++) {
                const dPersonalID_c *guest = gManager.mGuests.mPlayers[i].getPersonalID();
                BOOL same = FALSE;
                if (pid->land == guest->land && pid->isSamePlayer(guest)) {
                    same = TRUE;
                }
                if (same) {
                    slot = i + PLAYER_SLOT_FIRST_GUEST;
                    break;
                }
            }
        } else {
            slot = idx;
        }
    }
    return slot;
}

// 801018DC
int findPlayerIDSlot(const dPlayerID_c *id) {
    int slot = PLAYER_SLOT_NONE;
    if (id->isValid()) {
        int idx = dPrivateData_c::findByPlayerID(dSaveData_c::getTown()->mPlayers, id);
        if (idx == -1) {
            for (int i = 0; i < PLAYER_SLOT_GUEST_COUNT; i++) {
                if (id->isSame(&gManager.mGuests.mPlayers[i].getPersonalID()->player)) {
                    slot = i + PLAYER_SLOT_FIRST_GUEST;
                    break;
                }
            }
        } else {
            slot = idx;
        }
    }
    return slot;
}

// 80101990
void setChangeStateCallback(Mgr_c::ChangeStateFn fn, int idx) {
    gManager.mChangeState[idx] = fn;
}

// 801019A4
void setActor(dAcPlayer_c *actor, u32 idx) {
    if (idx < PLAYER_INDEX_COUNT) {
        gManager.mActors[idx] = actor;
    }
}

// 801019C4
dAcPlayer_c *getActor(int idx) {
    if ((u32)idx > PLAYER_INDEX_CURRENT) {
        return NULL;
    }
    if (idx == PLAYER_INDEX_CURRENT) {
        idx = dNet::getMyNo();
    }
    dAcPlayer_c *actor = gManager.mActors[idx];
    if (actor != NULL && (actor->mActorFlags & ACTOR_FLAG_DISABLED)) {
        return NULL;
    }
    return actor;
}

// 80101A28
dAcPlayer_c *getActorRaw(int idx) {
    if ((u32)idx > PLAYER_INDEX_CURRENT) {
        return NULL;
    }
    if (idx == PLAYER_INDEX_CURRENT) {
        idx = dNet::getMyNo();
    }
    return gManager.mActors[idx];
}

// 80101A74
BOOL changeState(int state, int idx) {
    if (idx > PLAYER_INDEX_CURRENT) {
        return FALSE;
    }
    if (idx == PLAYER_INDEX_CURRENT) {
        idx = dNet::getMyNo();
    }
    Mgr_c::ChangeStateFn fn = gManager.mChangeState[idx];
    if (fn == NULL) {
        return FALSE;
    }
    fn(idx, state);
    return TRUE;
}

// 80101AEC
GuestPlayers_c::GuestPlayers_c() {}

// 80101B38
GuestPlayers_c::~GuestPlayers_c() {}

// 80101BA0
void GuestPlayers_c::clear() {
    for (int i = 0; i < PLAYER_SLOT_GUEST_COUNT; i++) {
        mPlayers[i].clear();
    }
}

// 80101BF0
int GuestPlayers_c::getIndex(int slot) {
    BOOL guest = FALSE;
    if (slot >= PLAYER_SLOT_FIRST_GUEST && slot < PLAYER_SLOT_NONE) {
        guest = TRUE;
    }
    if (!guest) {
        return -1;
    }
    return slot - PLAYER_SLOT_FIRST_GUEST;
}

// 80101C1C
dPrivateData_c *GuestPlayers_c::get(int slot) {
    int i = getIndex(slot);
    if (i != -1) {
        return &mPlayers[i];
    }
    return NULL;
}

// 80101C68
dPrivateData_c *GuestPlayers_c::getRaw(int slot) {
    int i = getIndex(slot);
    if (i != -1) {
        return &mPlayers[i];
    }
    return NULL;
}

// 80101CB4
void receiveEquipment(int idx, int part, int item) {
    dPlayerMngr_c::receiveEquip(idx, part, item);
}

// 80101CB8
void receiveAppearance(int idx, int kind) {
    dPlayerMngr_c::receiveAppearance(idx, kind);
}

// 80101CBC
void queueRemoteEquipment(int idx, int part, int item) {
    dPlayerMngr_c::queueEquip(idx, part, item);
}

// 80101CC0
void setRemoteActionParams(int idx, const dPlayerMngr_c::actionParam_c *value) {
    dAcPlayer_c *player = getActor(idx);
    if (player != NULL) {
        player->mRemoteActionParams = *value;
    }
}

// 80101D0C
void setRemoteActionSync(int idx, int a, int b) {
    dAcPlayer_c *player = getActor(idx);
    if (player != NULL && !isActionParamLocked(player->mState)) {
        player->mRemoteSyncA = a;
        player->mRemoteSyncB = b;
    }
}

// 80101D70
void setRemoteSyncValue(int idx, int value) {
    dAcPlayer_c *player = getActor(idx);
    if (player != NULL) {
        player->mSyncValue = value;
    }
}

// 80101DA8
void cacheRemoteState(int idx, int state) {
    dPlayerMngr_c::setRemoteState(idx, state);
}

// 80101DAC
void cacheRemoteParams(int idx, const u8 *value) {
    dPlayerMngr_c::setRemoteParam(idx, (const dPlayerMngr_c::actionParam_c *)value);
}

// 80101DB0
BOOL changeNetPlayerState(int idx, int state) {
    return changeState(state, idx);
}

// 80101DC0
BOOL getRemoteTransform(void *position, void *angle, void *flags, u32 member) {
    return dPlayerMngr_c::getRemoteTransform((f32 *)position, (f32 *)angle, (s16 *)flags, member);
}

// 80101DC4
BOOL getRemoteStateInfo(void *info, u8 *scene, u8 *b, u32 idx) {
    return dPlayerMngr_c::getRemoteStateInfo((u32 *)info, scene, b, idx);
}

// 80101DC8
void cacheUnavailableActorAction(int idx, u8 b, int state, const u8 *param) {
    cacheRemoteState(idx, state);
    cacheRemoteParams(idx, param);
    if (!updateActionFieldTasks(idx, b, state, param)) {
        return;
    }
}

// 80101E3C
void cacheOtherSceneAction(int idx, u8 b, int state, const u8 *param) {
    cacheRemoteState(idx, state);
    cacheRemoteParams(idx, param);
    if (!updateActionFieldTasks(idx, b, state, param)) {
        return;
    }
}

// 80101EB0
BOOL updateActionFieldTasks(int idx, u8 b, int state, const u8 *param) {
    switch (state) {
    case 0x12:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x13:
        if (param[5] != 0) {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case STATE_SHAKE_TREE:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case STATE_SHAKE_TREE_END:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x40:
        fgMngTask_procGroupAt(param[4], param[5], 0);
        break;
    case 0x41:
        if (param[5] != 0) {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x45:
        fgMngTask_procGroupAt(param[2], param[3], 0);
        break;
    case 0x46:
        if (param[4] != 0) {
            fgMngTask_releaseAt(b, param[2], param[3], 0);
        } else {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x52:
        if (param[4] != 0) {
            fgMngTask_releaseAt(b, param[2], param[3], 0);
        } else {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x53:
        if (param[4] != 0) {
            fgMngTask_releaseAt(b, param[2], param[3], 0);
        } else {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x55:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x57:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x59:
        fgMngTask_releaseAt(b, param[0], param[1], 0);
        break;
    case 0x5B:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x7E:
        fgMngTask_procGroupAt(param[2], param[3], param[4]);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// 80102138
BOOL isSyncValueAhead(int a, int b) {
    if (b > 0 && a > 0) {
        if (a > b) {
            return TRUE;
        }
    } else if (b < 0 && a < 0) {
        if (a > b) {
            return TRUE;
        }
    } else if ((b > 0 && a < 0) || (b < 0 && a > 0)) {
        if (b - a < 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80475A90: one flag per player state (isActionParamLocked).
static const u8 sActionParamLocked[STATE_COUNT] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// 801021B0
BOOL isActionParamLocked(u32 state) {
    if (state >= STATE_COUNT) {
        return 0;
    }
    return sActionParamLocked[state];
}

// 801021D0
int getWearSound(const dItem::Item *a, const dItem::Item *b) {
    if (a->mId != dItem::ITEM_ID_NONE) {
        switch (a->getFashion()) {
        case dItem::FASHION_RED_HERO:
        case dItem::FASHION_GREEN_HERO:
        case dItem::FASHION_BLUE_HERO:
        case dItem::FASHION_HEROINE:
            return SE_WearHero;
        }
    }
    if (b->mId != dItem::ITEM_ID_NONE) {
        switch (b->getFashion()) {
        case dItem::FASHION_RED_HERO:
        case dItem::FASHION_GREEN_HERO:
        case dItem::FASHION_BLUE_HERO:
        case dItem::FASHION_HEROINE:
            return SE_WearHero;
        }
        dItem::Item item(dItem::ITEM_IDX_PUMPKIN_HEAD);
        if (b->isSame(item)) {
            return SE_WearPumpkin;
        }
        item.setFromIndex(dItem::ITEM_IDX_KING_TUT_MASK);
        if (b->isSame(item)) {
            return SE_Tutankhamen;
        }
    }
    return SE_wear;
}

// 801022B8: splash/snow effect callback for createSurfaceEffects.
void onParticleCollision(nw4r::ef::Particle *obj, u32 kind) {
    if (obj == NULL) {
        return;
    }
    mVec3_c pos;
    if (dScene::isOutdoorScene((u8)dScene::getCurrentScene())) {
        mVec3_c src;
        src = obj->mParameter.mPosition;
        dWorld::toFieldPosition(&pos, &src);
    } else {
        pos = obj->mParameter.mPosition;
    }

    dBGCF::groundChk_c check(&pos, dBGCF::LAYER_TOP, 0, 0);
    if (check.mWater != BG_WATER_NONE) {
        if (check.isUnderWater(pos.y)) {
            pos.y = check.mWaterY;
            dEffect::create("afi_hny_watersplash_b", &pos, NULL, NULL);
            if (obj->mParticleManager != NULL) {
                obj->mParticleManager->RetireParticle(obj);
            }
        }
    } else {
        f32 ground = check.getHeight(TRUE);
        if (pos.y <= ground) {
            const char *name = kind == SURFACE_EFFECT_SNOW ? "afi_hny_snowbreak" : "afi_hny_watersplash_a";
            if (nw4r::math::FAbs(ground - pos.y) <= 5.0f) {
                pos.y = ground;
                dEffect::create(name, &pos, NULL, NULL);
                if (obj->mParticleManager != NULL) {
                    obj->mParticleManager->RetireParticle(obj);
                }
            }
        }
    }
}

// 80102434
int getSurfaceEffectKind(const mVec3_c *pos) {
    dBGCF::groundChk_c check(pos, dBGCF::LAYER_TOP, 0, 0);
    if (check.mWater == BG_WATER_SEA) {
        return SURFACE_EFFECT_SEA;
    }
    int attr = check.mAttr;
    if (dWeather_c::sInstance != NULL) {
        if (dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
            return SURFACE_EFFECT_RAIN;
        }
    }
    if (attr == BG_ATTR_SAND) {
        return SURFACE_EFFECT_SAND;
    }
    if (dGroundAttr::getSnowWear(pos) > sSnowEffectWearThreshold) {
        return SURFACE_EFFECT_SNOW;
    }
    return SURFACE_EFFECT_NORMAL;
}

// 801024F8
void createSurfaceEffects(const char *const *names, const mVec3_c *pos, const mAng3_c *angle,
                          const char *wetEffect, const char *snowEffect) {
    u32 kind = getSurfaceEffectKind(pos);
    dEffect::create(names[kind], pos, angle, NULL);
    if (kind == SURFACE_EFFECT_NORMAL || kind == SURFACE_EFFECT_SAND) {
        return;
    }
    BOOL wet = kind == SURFACE_EFFECT_SEA || kind == SURFACE_EFFECT_RAIN;
    const char *effect = wet ? wetEffect : snowEffect;
    dEffect::create(effect, pos, angle, NULL, onParticleCollision, kind);
}

// 801025B8
BOOL isBusy() {
    return FALSE;
}

// 801025C0: __sinit_d_player_mgr_cpp (the look-angle limits and gManager).


} // namespace dPlayerMgr
