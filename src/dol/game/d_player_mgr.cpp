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

// Dependencies whose owners are not recovered yet.

extern "C" {
extern u8 lbl_8074EA7D;

BOOL fn_802C4404(dMiiCreateID_c *mii, u16 *out);
BOOL fn_802B3BD0(int a, u16 mii, int b);
int fn_802C5A0C(dPlayerMgr::MiiData_c *out, int a, int b, u16 mii);

}

// The field task code (d_fg_mng_task.inc, #included by d_field_assessment.cpp).
void fgMngTask_procGroupAt(u8 x, u8 z, u8 layer);               // 800A3044
void fgMngTask_releaseAt(u8 scene, int x, int z, u32 layer);    // 800A3118

namespace dPlayerMgr {

// 8074E600 / 8074E602, set by the __sinit: look-at limits (lookAt, also read by d_a_player).
s16 sLookPitchMax = cLib::deg2bin(10.0f);
s16 sLookYawMax = cLib::deg2bin(45.0f);

// 8074AF10
static u8 sSnowEffectWearThreshold = 0x3F;

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
    dPlayerActor_c *player = getPlayerActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return &player->mPos;
}

// 800FBC7C
dPlayerActor_c *getPlayerActor(int idx) {
    return getActor(idx);
}

// 800FBC80
dPlayerActor_c *getPlayerActorRaw(int idx) {
    return getActorRaw(idx);
}

// 800FBC84
dPlayerActor_c *getNpcTargetActor(int idx) {
    return getActor(idx);
}

// 800FBC88
mVec3_c *getSoundPosition(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return (mVec3_c *)player->_060C;
}

// 800FBCBC
BOOL getMoveAngle(s16 *out) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (out != NULL && player->_22EC != 0.0f) {
        *out = player->_22F8;
        return TRUE;
    }
    return FALSE;
}

// 800FBD28
s16 getYaw(int idx) {
    dPlayerActor_c *player = getPlayerActor(idx);
    if (player == NULL) {
        return 0;
    }
    return player->mAngle.y;
}

// 800FBD5C
BOOL getUnitCoordinates(u32 *a, u32 *b, int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    *a = player->_2284;
    *b = player->_2288;
    return TRUE;
}

// 800FBDD4
mVec3_c *getHandPosition(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return (mVec3_c *)player->_0624;
}

// 800FBE08
void *getHandTransform(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    return player->_06F0;
}

// 800FBE3C
BOOL canStartDemo() {
    if (gManager.mLocked) {
        return FALSE;
    }
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return isDemoStartState(player->mState);
}

// 800FBE98
BOOL canStartNetSave() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (isDemoStartState(state)) {
        return TRUE;
    }
    switch (state) {
    case 0x1A:
    case 0x24:
    case 0x26:
    case 0x28:
    case 0x5A:
    case 0x99:
        player->_2274 |= 0x4;
        return TRUE;
    case 0x1D:
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL && player->mState != 0x49) {
        changeState(STATE_DEMO_WAIT, PLAYER_INDEX_CURRENT);
    }
}

// 800FC010
BOOL requestDemoWait() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_DEMO_WAIT) {
        return TRUE;
    }
    return changeState(STATE_DEMO_WAIT, PLAYER_INDEX_CURRENT);
}

// 800FC064
void fn_800FC064(BOOL on, int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player != NULL) {
        if (on) {
            player->_2270 |= 0x100000;
        } else {
            player->_2270 &= ~0x100000;
        }
    }
}

// 800FC0C0
u32 fn_800FC0C0(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 20) & 1;
}

// 800FC0F8
BOOL fn_800FC0F8() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x9B) {
        player->_2274 |= 0x100;
        return TRUE;
    }
    if (state != 0x9A) {
        return FALSE;
    }
    if (changeState(0x9B, PLAYER_INDEX_CURRENT)) {
        player->_2274 |= 0x100;
        return TRUE;
    }
    return FALSE;
}

// 800FC194
BOOL fn_800FC194(const mVec3_c *target) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->_2274;
    player->_2274 = flags & ~0x100;
    if (flags & 0x200) {
        return TRUE;
    }
    int state = player->mState;
    if (state == STATE_IDLE || state == 0x61) {
        return TRUE;
    }
    if (state != 0x9B) {
        return FALSE;
    }
    if (target == NULL) {
        return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
    }
    if (changeState(0x61, PLAYER_INDEX_CURRENT)) {
        s16 angle = dActor_c::targetAngleY(&player->mPos, target);
        player->_22D0 = sLib::distanceAngle(angle, player->mAngle.y) / 10;
        player->_22C4 = angle;
        return TRUE;
    }
    return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
}

// 800FC2A4
BOOL fn_800FC2A4() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x9B) {
        player->_2274 |= 0x200;
        return TRUE;
    }
    if (state != 0x9A) {
        return FALSE;
    }
    if (changeState(0x9B, PLAYER_INDEX_CURRENT)) {
        player->_2274 |= 0x200;
        return TRUE;
    }
    return FALSE;
}

// 800FC340
BOOL fn_800FC340() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->_2274;
    player->_2274 = flags & ~0x200;
    if (flags & 0x100) {
        return TRUE;
    }
    if (player->mState != 0x9B) {
        return FALSE;
    }
    return changeState(STATE_IDLE, PLAYER_INDEX_CURRENT);
}

// 800FC3B0
BOOL fn_800FC3B0() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(0xA1, PLAYER_INDEX_CURRENT);
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
    if (current == NULL || lbl_8074EA7D != 1) {
        return;
    }
    dItem::Item held = current->mEquipment.mHeld;
    if (!isDesignSlot(fn_8019AFE4() & 7, &held)) {
        return;
    }
    if (dHmnToolBank_c::getItemToolType(&held) == 0xA && dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held.mId);
        dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held.mId);
    }
}

// 800FC4C0
BOOL fn_800FC4C0() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(0xA4, PLAYER_INDEX_CURRENT);
}

// 800FC514
BOOL fn_800FC514() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0xA4) {
        return FALSE;
    }
    return changeState(0xA5, PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL && (player->_2270 & 0x10)) {
        int kind = player->_0C34->getToolType();
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
u8 isToolActionHeld() {
    return gManager.mToolActionHeld;
}

// 800FC630
BOOL enableCurrentActor() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->_2270 &= ~ACTOR_FLAG_DISABLED;
    return TRUE;
}

// 800FC674
BOOL refreshEquippedDesign(int slot) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
        player->_2274 |= 0x20;
        if (!changeHeldItem(&current->mEquipment.mHeld)) {
            player->_2274 &= ~0x20;
        }
    } else {
        dItem::Item *pShirt = shirt ? &current->mEquipment.mShirt : NULL;
        dItem::Item *pHat = hat ? &current->mEquipment.mHat : NULL;
        dItem::Item *pHeld = held ? &current->mEquipment.mHeld : NULL;
        player->_2274 |= 0x20;
        if (!changeOutfitAndHeldItem(pShirt, pHat, pHeld)) {
            player->_2274 &= ~0x20;
        }
    }
    return TRUE;
}

// 800FC7E8
BOOL canStartIdleDemo() {
    if (gManager.mLocked) {
        return FALSE;
    }
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState <= STATE_WALK;
}

// 800FC858
BOOL hasCurrentActor() {
    return getActor(PLAYER_INDEX_CURRENT) != NULL;
}

// 800FC888
BOOL isNetSaveWaitState() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
BOOL setDemoWait(BOOL a) {
    if (a) {
        startDemoWait();
        return TRUE; // the target tail-calls the void startDemoWait; r3 is whatever it left
    }
    return endDemoWait();
}

// 800FC8FC
BOOL fn_800FC8FC() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != STATE_DEMO_WAIT && state != 0x88) {
        return FALSE;
    }
    return changeState(0x88, PLAYER_INDEX_CURRENT);
}

// 800FC958
BOOL fn_800FC958() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(0x9E, PLAYER_INDEX_CURRENT);
}

// 800FC9AC
BOOL fn_800FC9AC() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9E && player->mAngle.y == -0x8000) {
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state >= STATE_IDLE && state < STATE_MOVE_TO) {
        return TRUE;
    }
    return FALSE;
}

// 800FCAA0
BOOL requestDemoStart() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (!canStartMovementDemo()) {
        return FALSE;
    }
    return changeState(STATE_DEMO_START, PLAYER_INDEX_CURRENT);
}

// 800FCAF4
BOOL showCatch(const dItem::Item *item) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_SHOW_CATCH) {
        return FALSE;
    }
    int kind = item->getKind();
    if (kind == dItem::KIND_INSECT) {
        player->_2270 |= ACTOR_FLAG_INSECT;
    } else if (kind == dItem::KIND_FISH) {
        player->_2270 &= ~ACTOR_FLAG_INSECT;
    } else {
        return FALSE;
    }
    player->_218A = item->mId;
    return changeState(STATE_SHOW_CATCH, PLAYER_INDEX_CURRENT);
}

// 800FCBA8
BOOL finishShowCatch() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOW_CATCH) {
        return FALSE;
    }
    player->_2234 = 6;
    return TRUE;
}

// 800FCBFC
BOOL isShowCatchFinished() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOW_CATCH) {
        return FALSE;
    }
    if ((u32)(player->mAnmId - 0x53) <= 1) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800FCC74
BOOL showCatchAt(const dItem::Item *item, const mVec3_c *pos) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == STATE_SHOW_CATCH_AT) {
        return TRUE;
    }
    int kind = item->getKind();
    if (kind == dItem::KIND_INSECT) {
        player->_2270 |= ACTOR_FLAG_INSECT;
    } else if (kind == dItem::KIND_FISH) {
        player->_2270 &= ~ACTOR_FLAG_INSECT;
    } else {
        return FALSE;
    }
    player->_218A = item->mId;
    player->_2270 |= 0x1000000;
    if (pos != NULL) {
        player->_2074 = *pos;
    }
    return changeState(STATE_SHOW_CATCH_AT, PLAYER_INDEX_CURRENT);
}

// 800FCD60
BOOL fn_800FCD60(u32 a, u32 b, BOOL c) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->_2190 = a;
    player->_2194 = b;
    player->_2270 |= 0x1000000;
    if (c) {
        player->_2270 |= 0x80000000;
    } else {
        player->_2270 &= ~0x80000000;
    }
    return changeState(0x44, PLAYER_INDEX_CURRENT);
}

// Invalid hide-bone values leave the model visible.
static inline int getHideBone(const dItem::BITM *bitm) {
    int v = (s8)bitm->m_hideBone;
    return (u32)v < dItem::HIDE_BONE_COUNT ? v : dItem::HIDE_BONE_NONE;
}

// 800FCDFC
BOOL changeOutfitAndHeldItem(const dItem::Item *shirt, const dItem::Item *hat, const dItem::Item *held) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
        int tmp;
        if (dHmnClothMng_c::isOrgCloth(&tmp, shirt)) {
            player->_22B4 |= EQUIPMENT_CHANGE_CLOTH;
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
                player->_22DA = HELD_ITEM_CHANGE_REMOVE;
            }
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        } else {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
            if (bitm != NULL && getHideBone(bitm) == dItem::HIDE_BONE_HAND) {
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
                    player->_22DA = HELD_ITEM_CHANGE_EQUIP;
                }
                dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
                dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
            }
        }
    }

    player->_223C = 1;
    player->_2238 = 1;
    player->_2234 = 8;
    return changeState(STATE_CHANGE_OUTFIT_AND_HELD, PLAYER_INDEX_CURRENT);
}

// 800FD058
BOOL eatItem(const dItem::Item *item) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    int edibility = (s8)bitm->m_edibility;
    if ((u32)edibility >= dItem::EDIBILITY_COUNT) {
        edibility = dItem::EDIBILITY_NONE;
    }
    switch (edibility) {
    case dItem::EDIBILITY_FOOD:
        player->_218A = item->mId;
        return changeState(STATE_EAT, PLAYER_INDEX_CURRENT);
    case dItem::EDIBILITY_SPECIAL:
        player->_218A = item->mId;
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
        switch (player->_22DD) {
        case STATE_RUN:
        case STATE_MOVE_TO:
        case STATE_TURN_TO:
            if (changeState(0x84, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
            break;
        case STATE_DEMO_START:
            player->_22DD = 8;
            break;
        }
        break;
    case 0xA:
        if (dNpcInfo::tanukichi_c::get()->mJobDone != 0) {
            if (changeState(0x84, PLAYER_INDEX_CURRENT)) {
                player->_22DD = 1;
                clearDemoFlags(player);
                dNpcInfo::tanukichi_c::get()->mJobDone = 0;
                player->_2270 |= 0x200000;
                return TRUE;
            }
        } else if (isBedHairPending()) {
            clearBedHairPending();
            clearDemoFlags(player);
            return changeState(STATE_BED_HAIR, PLAYER_INDEX_CURRENT);
        }
        break;
    case STATE_CHANGE_HELD_ITEM_DEMO:
        clearDemoFlags(player);
        return TRUE;
    case STATE_GET_UP:
    case 0x2D:
    case 0x34:
    case 0x39:
    case 0x3A:
    case STATE_SHOW_CATCH_AT:
    case 0x87:
    case STATE_SHOE_SHINE_END:
        clearDemoFlags(player);
        return TRUE;
    case 0x4A:
    case 0x81:
    case 0x83:
        if (player->_2274 & 0x400) {
            player->_2274 &= ~0x400;
            if (changeState(0x61, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        }
        break;
    case 0x8C:
    case 0xAA:
        return TRUE;
    }

    Mgr_c::getCurrentPlayer();
    u32 flags = player->_2270;
    if ((flags & 0x10) && !(flags & 0x20) && !isToolActionHeld()) {
        if (player->_208C >= 1.0f) {
            player->_2270 |= 0x20;
            if (changeState(STATE_IDLE, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        } else {
            int state = player->_0C34->getToolType() == HMN_TOOL_UMBRELLA ? 0x7B : 0x21;
            if (changeState(state, PLAYER_INDEX_CURRENT)) {
                clearDemoFlags(player);
                return TRUE;
            }
        }
    }
    if (changeState(STATE_IDLE, PLAYER_INDEX_CURRENT)) {
        clearDemoFlags(player);
        player->_2270 |= 0x200000;
        return TRUE;
    }
    return FALSE;
}

// 800FD510
BOOL getChairPosition(mVec3_c *out, int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_CHAIR_WAIT) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// Shared by the "walk to (x, z)" requests: target position at the player's height.
static inline void setTargetXZ(dPlayerActor_c *player, f32 x, f32 z) {
    mVec3_c target(x, player->mPos.y, z);
    player->_2068 = target;
}

// 800FD588
BOOL sitOnChair(s16 angle, int type, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == STATE_IDLE || state == STATE_WALK || (state == 0x23 && (player->_2274 & 0x4000)))) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_224C = type;
    return changeState(STATE_SIT_DOWN, PLAYER_INDEX_CURRENT);
}

// 800FD664
BOOL isChairAnimation() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x31;
}

// 800FD6A8
BOOL getBedAnimationPosition(mVec3_c *out, int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mAnmId != 0x37) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// 800FD720
BOOL getBedPosition(mVec3_c *out, int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_BED_WAIT) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// 800FD798
BOOL lieOnBed(s16 angle, int type, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == STATE_IDLE || state == STATE_WALK || (state == 0x23 && (player->_2274 & 0x4000)))) {
        return FALSE;
    }
    if (type != 1 && type != 2) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_224C = type;
    return changeState(STATE_LIE_DOWN, PLAYER_INDEX_CURRENT);
}

// 800FD88C
BOOL getUpFromBed(int type, int arg) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    player->_224C = type;
    player->_2248 = type == 1 ? 1 : 3;
    return changeState(STATE_GET_UP, PLAYER_INDEX_CURRENT);
}

// 800FD924
BOOL changeSeatedPose(int type) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    player->_2248 = value;
    player->_224C = type;
    return changeState(0x39, PLAYER_INDEX_CURRENT);
}

// 800FD9E0
BOOL fn_800FD9E0(int type, s16 angle, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x32) <= 2) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2254 = type;
    return changeState(0x32, PLAYER_INDEX_CURRENT);
}

// 800FDAA4
BOOL fn_800FDAA4(int type) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x34 || state != 0x33) {
        return FALSE;
    }
    player->_2254 = type;
    return changeState(0x34, PLAYER_INDEX_CURRENT);
}

// 800FDB10
BOOL hasActionActor() {
    return getActor(PLAYER_INDEX_CURRENT) != NULL;
}

// 800FDB40
void fn_800FDB40() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        player->_2270 |= 0x100;
    }
}

// 800FDB78
BOOL fn_800FDB78(int type) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0xA) {
        return TRUE;
    }
    player->_2244 = type;
    return changeState(0xA, PLAYER_INDEX_CURRENT);
}

// 800FDBDC
void fn_800FDBDC(u32 mode) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL || !(player->_2270 & 0x10)) {
        return;
    }
    if (mode >= 2) {
        player->_2270 &= ~0x20;
        return;
    }
    if (isToolActionHeld()) {
        player->_2270 &= ~0x20;
        clearToolActionHold();
        return;
    }
    int kind = player->_0C34->getToolType();
    if ((u32)(kind - HMN_TOOL_UMBRELLA) <= 2) {
        if (mode == 0) {
            if (kind == HMN_TOOL_UMBRELLA) {
                changeState(0x7C, PLAYER_INDEX_CURRENT);
            } else {
                changeState(0x22, PLAYER_INDEX_CURRENT);
            }
        } else {
            player->_2270 &= ~0x20;
        }
    } else {
        changeState(0x22, PLAYER_INDEX_CURRENT);
    }
}

// 800FDCC0
BOOL canFinishToolAction() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return TRUE;
    }
    return (player->_2270 & 0x20) == 0;
}

// 800FDD04
BOOL isSceneTransitionState() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == STATE_DEMO_WAIT && (dDemo::isType(dDemo::TYPE_SCENE_START) || dDemo::isType(dDemo::TYPE_EXITSCENE))) {
        return TRUE;
    }
    if ((u32)(state - 0x49) <= 1) {
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    u16 mii = 0;
    if (!current->isFlag0(0x28)) {
        if (!canUseMiiMask()) {
            return FALSE;
        }
        if (!fn_802C4404(&current->mMiiId, &mii)) {
            return FALSE;
        }
    }
    player->_22B6 = mii;
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    if (!canUseMiiMask()) {
        return FALSE;
    }
    if (!fn_802B3BD0(0, mii, 0)) {
        return FALSE;
    }
    MiiData_c data;
    if (fn_802C5A0C(&data, 0, 0, mii) != 0) {
        return FALSE;
    }
    dPrivateData_c *current = Mgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    memcpy(&current->mMiiId, data.mCreateID, sizeof(data.mCreateID));
    player->_22B6 = mii;
    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_ON);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HAT, dItem::ITEM_ID_NONE);
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, dItem::ITEM_ID_NONE);
    return changeState(STATE_CHANGE_MII_MASK, PLAYER_INDEX_CURRENT);
}

// 800FE2D4
BOOL takeOffMiiMaskImmediately() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 10) & 1;
}

// 800FE39C
BOOL fn_800FE39C(s16 angle, int type, int target, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2244 = type;
    player->_2298 = target;
    if (target == -1) {
        return changeState(0x9, PLAYER_INDEX_CURRENT);
    }
    return changeState(0xC, PLAYER_INDEX_CURRENT);
}

// 800FE484
BOOL fn_800FE484(s16 angle, int target, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0xC) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2274 |= 0x20;
    player->_2298 = target;
    return changeState(0xC, PLAYER_INDEX_CURRENT);
}

// 800FE550
BOOL fn_800FE550(s16 angle, int type, BOOL useC, f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2244 = type;
    if (useC) {
        return changeState(0xC, PLAYER_INDEX_CURRENT);
    }
    return changeState(0x9, PLAYER_INDEX_CURRENT);
}

// 800FE634
BOOL fn_800FE634() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return;
    }
    int state = player->mState;
    switch (state) {
    case STATE_BED_WAIT:
    case STATE_CHAIR_WAIT:
        break;
    case STATE_DEMO_WAIT:
        if (player->mAnmId == 0x31) {
            state = STATE_CHAIR_WAIT;
        } else if (player->mAnmId == 0x37) {
            state = STATE_BED_WAIT;
        }
        break;
    default:
        state = STATE_DEMO_WAIT;
        break;
    }
    *outState = state;
    *outPos = player->mPos;
    *outAngle = player->mAngle.y;
}

// 800FE74C
BOOL fn_800FE74C(const mVec3_c *pos, int idx, f32 speed) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    player->_2274 |= 0x8000;
    BOOL result = moveTo(pos, idx, speed);
    if (!result) {
        player->_2274 &= ~0x8000;
    }
    return result;
}

// 800FE7E0
BOOL moveTo(const mVec3_c *pos, int idx, f32 speed) {
    dPlayerActor_c *player = getActor(idx);
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
    player->_2068 = *pos;
    player->_21BC = speed;
    return changeState(STATE_MOVE_TO, idx);
}

// 800FE88C
BOOL turnTo(s16 angle, int idx) {
    return turnToInFrames(angle, 500, idx);
}

// 800FE89C
BOOL turnToInFrames(s16 angle, int frames, int idx) {
    dPlayerActor_c *player = getActor(idx);
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
    player->_22C4 = angle;
    player->_22D0 = frames;
    return changeState(STATE_TURN_TO, idx);
}

// 800FE938
BOOL lookAt(const mVec3_c *target, int a, BOOL pitch, int c) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
        yaw = (s16)(dActor_c::targetAngleY(&player->mPos, target) - player->mAngle.y);
        if (yaw > sLookYawMax) {
            yaw = sLookYawMax;
        } else if (yaw < -sLookYawMax) {
            yaw = -sLookYawMax;
        }
        if (pitch) {
            mVec3_c diff = player->_0654 - *target;
            f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
            tilt = cM::atan2s(target->y - player->_0654.y, dist);
            if (tilt > sLookPitchMax) {
                tilt = sLookPitchMax;
            } else if (tilt < -sLookPitchMax) {
                tilt = -sLookPitchMax;
            }
        }
    }
    player->_22C8 = yaw;
    player->_22C6 = tilt;
    player->_22CA = a;
    player->_22CC = c;
    player->_2270 |= ACTOR_FLAG_LOOK_AT;
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
    dPlayerActor_c *player = getActorRaw(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == state;
}

// 800FEB34
int getState(int idx) {
    dPlayerActor_c *player = getActorRaw(idx);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (isChangingEquipment()) {
        return FALSE;
    }
    if (held->mId == dItem::ITEM_ID_NONE) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) || (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) && dHmnToolBank_c::isHandTool(held))) {
            player->_22DA = HELD_ITEM_CHANGE_REMOVE;
            player->_223C = 1;
        }
    } else {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
        if (bitm == NULL) {
            return FALSE;
        }
        if (getHideBone(bitm) != dItem::HIDE_BONE_HAND) {
            return FALSE;
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) || (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) && dHmnToolBank_c::isHandTool(held))) {
            player->_22DA = HELD_ITEM_CHANGE_EQUIP;
            player->_223C = 1;
        }
    }
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    player->_2234 = 8;
    player->_2270 |= 0x1000000;
    return changeState(STATE_CHANGE_HELD_ITEM, PLAYER_INDEX_CURRENT);
}

// 800FEEB0
BOOL isChangingEquipment() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case STATE_CHANGE_EQUIPMENT:
        if (player->mAnmId == 0x24) {
            return TRUE;
        }
        if (player->mAnmId == 0x26 && player->getAnm()->getFrame() < 57.0f) {
            return TRUE;
        }
        break;
    case STATE_CHANGE_HELD_ITEM:
    case STATE_MII_MASK_IMMEDIATE:
    case 0x4D:
    case 0x4E:
    case 0x8B:
    case STATE_CHANGE_HELD_ITEM_DEMO:
    case STATE_CHANGE_OUTFIT_AND_HELD:
        return TRUE;
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
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return NULL;
    }
    u8 *obj = player->_0C34 != NULL ? (u8 *)player->_0C34 + 0x1C : NULL;
    if (obj != NULL && obj[0x447] != 0) {
        return (mVec3_c *)(obj + 0x270);
    }
    return &player->mPos;
}

// 800FF048
mVec3_c *getCurrentSoundPositionRaw() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return NULL;
    }
    return (mVec3_c *)player->_060C;
}

// 800FF080
BOOL getGroundPositionAhead(mVec3_c *out) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    s16 angle = player->mAngle.y;
    out->x = player->mPos.x + 12.0f * nw4r::math::SinFIdx((1.0f / 256.0f) * angle);
    out->z = player->mPos.z + 12.0f * nw4r::math::CosFIdx((1.0f / 256.0f) * angle);
    mVec3_c pos(out->x, player->mPos.y, out->z);
    out->y = dBGCF::getGroundY(&pos, FALSE);
    return TRUE;
}

// 800FF150
u32 fn_800FF150(int idx) {
    dPlayerActor_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 13) & 1;
}

// 800FF188
u32 fn_800FF188(int idx) {
    dPlayerActor_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2274 >> 7) & 1;
}

// 800FF1C0
int getActionUnitAndKind(int *x, int *z, int idx) {
    dPlayerActor_c *player = getActorRaw(idx);
    if (player == NULL) {
        return 7;
    }
    if (x == NULL || z == NULL) {
        return 7;
    }
    if (!(player->_2274 & 0x80) && !(player->_2270 & 0x2000)) {
        return 7;
    }
    switch (player->mState) {
    case STATE_WALK:
        *x = player->_228C;
        *z = player->_2290;
        return 0;
    case 0x3D:
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
BOOL fn_800FF354(int *outState) {
    int tmp;
    if (outState == NULL) {
        outState = &tmp;
    }
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        *outState = STATE_NONE;
        return FALSE;
    }
    *outState = player->mState;
    if (player->_2270 & 1) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x1A) {
        dUki_c::setFish(dNet::getMyNo(), NULL);
        *outState = 0x1D;
        changeState(0x1D, PLAYER_INDEX_CURRENT);
        return FALSE;
    }
    if (state != STATE_IDLE && state != STATE_WALK) {
        return FALSE;
    }
    if (!dDemo::request9()) {
        return FALSE;
    }
    Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_BEE_STUNG);
    *outState = 0x62;
    return changeState(0x62, PLAYER_INDEX_CURRENT);
}

// 800FF444
BOOL changeEquipment(const dEquip_c *equip, int mode, int kind) {
    if (equip == NULL) {
        return FALSE;
    }
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (isChangingEquipment()) {
        return FALSE;
    }
    if (kind == EQUIPMENT_CHANGE_PUT_ON_MII_MASK) {
        player->_22E4 = mode;
        return putOnMiiMaskImmediately();
    }
    if (kind == EQUIPMENT_CHANGE_TAKE_OFF_MII_MASK) {
        player->_22E4 = mode;
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
    if (!shirt.isSame(current->mEquipment.mShirt) || ((mode == EQUIPMENT_CHANGE_SHIRT || mode == EQUIPMENT_CHANGE_DESIGN) && dHmnClothMng_c::isOrgCloth(&tmp, &shirt))) {
        changeShirt = TRUE;
    }
    dItem::Item hat = equip->mHat;
    if (!hat.isSame(current->mEquipment.mHat) || ((mode == EQUIPMENT_CHANGE_HAT || mode == EQUIPMENT_CHANGE_DESIGN) && dHmnHeadItemMng_c::isOrgCap(&tmp, &hat))) {
        changeHat = TRUE;
    }
    dItem::Item acc = equip->mAcc;
    if (!acc.isSame(current->mEquipment.mAcc)) {
        changeAcc = TRUE;
    }

    if (changeShirt && shirt.mId != dItem::ITEM_ID_NONE) {
        int tmp2;
        if (dHmnClothMng_c::isCloth(&shirt) || dHmnClothMng_c::isOrgCloth(&tmp2, &shirt)) {
            player->_22B4 |= EQUIPMENT_CHANGE_CLOTH;
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt.mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_SHIRT, shirt.mId);
        }
    }

    if (changeHat) {
        int bone = dItem::HIDE_BONE_NONE;
        if (hat.mId != dItem::ITEM_ID_NONE) {
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
        if (acc.mId == dItem::ITEM_ID_NONE) {
            player->_22B4 = (player->_22B4 & ~EQUIPMENT_SHOW_ACCESSORY) | EQUIPMENT_HIDE_ACCESSORY;
            dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
            dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
        } else {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(acc);
            if (bitm != NULL && getHideBone(bitm) == dItem::HIDE_BONE_ACCESSORY) {
                player->_22B4 = (player->_22B4 & ~EQUIPMENT_HIDE_ACCESSORY) | EQUIPMENT_SHOW_ACCESSORY;
                if (current->mMiiMask != 0) {
                    dPlayerMngr_c::setLocalAppearance(dNet::getMyNo(), APPEARANCE_MII_MASK_OFF);
                }
                dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
                dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_ACCESSORY, acc.mId);
            }
        }
    }

    switch (mode) {
    case 0:
        player->_2238 = 1;
        return changeState(0x4E, PLAYER_INDEX_CURRENT);
    case EQUIPMENT_CHANGE_SHIRT:
    case EQUIPMENT_CHANGE_HAT:
    case EQUIPMENT_CHANGE_ACCESSORY:
        player->_2238 = 1;
        player->_2234 = 8;
        return changeState(STATE_CHANGE_EQUIPMENT, PLAYER_INDEX_CURRENT);
    case 4:
    case EQUIPMENT_CHANGE_DESIGN:
        player->_2238 = 1;
        player->_2234 = 8;
        return changeState(0x4D, PLAYER_INDEX_CURRENT);
    case 6:
        player->_2238 = 1;
        player->_2234 = 8;
        return changeState(0x8B, PLAYER_INDEX_CURRENT);
    }
    return FALSE;
}

// 800FF840
BOOL isSceneReadyForBgm() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return TRUE;
    }
    if (player->mState == 0x8C) {
        return TRUE;
    }
    if ((player->_2270 & 0x200000) && dBG::getMdlBank()->isReady()) {
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
    return (sHairBits[hair] & (1 << bit)) != 0;
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
BOOL fn_800FFA50(const mVec3_c *a, int b, f32 f) {
    if (dPad::isATriggered() && isFacingPosition(a, b, f)) {
        return TRUE;
    }
    return FALSE;
}

// 800FFAC0
BOOL fn_800FFAC0() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if ((player->_2270 & 0x40000000) && player->mState == STATE_WALK) {
        return TRUE;
    }
    return FALSE;
}

// 800FFB18
BOOL fn_800FFB18() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0xD && state < 0x17) || (state >= 0x18 && state < 0x21) || (state >= 0x3E && state < 0x48) ||
        state == 0x7D || (state >= 0x89 && state < 0x8B) || (state >= 0x8D && state < 0x8F) ||
        (state >= 0x99 && state < STATE_DRINK_COFFEE) || state == 0xA2) {
        return TRUE;
    }
    return FALSE;
}

// 800FFBE4
BOOL fn_800FFBE4() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return (u32)(player->mState - 0x3C) <= 1;
}

// 800FFC38
BOOL fn_800FFC38() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x51 && state < 0x54) || state == 0x5D || state == 0x7E) {
        return TRUE;
    }
    return FALSE;
}

// 800FFCA8
BOOL fn_800FFCA8() {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x23 && state < 0x32) || (state >= STATE_SIT_DOWN && state < 0x3C)) {
        return TRUE;
    }
    return FALSE;
}

// 800FFD10
BOOL isActionItemState() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return dItem::ITEM_ID_NONE;
    }
    int state = player->mState;
    if (state != 0xE && state != 0x1F) {
        return dItem::ITEM_ID_NONE;
    }
    return player->_218A;
}

// 800FFDC0
BOOL isFacingPosition(const mVec3_c *target, int maxAngle, f32 maxDist) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    maxDist = maxDist * maxDist;
    mVec3_c diff = player->mPos - *target;
    f32 dist = diff.x * diff.x + diff.z * diff.z;
    if (dist == 0.0f) {
        return TRUE;
    }
    if (dist > maxDist) {
        return FALSE;
    }
    s16 angle = dActor_c::targetAngleY(&player->mPos, target);
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
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mAnmId - 0x3C) <= 1) {
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
BOOL setHatHideBone(u32 bone, dPlayerActor_c *player) {
    if (player == NULL) {
        return FALSE;
    }
    switch (bone) {
    case 0:
    case 1:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x1;
        break;
    case 2:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x2;
        break;
    case 3:
    case 4:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x4;
        break;
    case 5:
    case 6:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x8;
        break;
    case 7:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x10;
        break;
    case dItem::HIDE_BONE_NONE:
        player->_22B4 = (player->_22B4 & ~0x1F) | 0x20;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// 80100410
void clearDemoFlags(dPlayerActor_c *player) {
    if (player != NULL) {
        player->_2270 &= ~1;
        player->_2270 &= ~0x100;
    }
}

// 8010042C
void getActionUnitCoordinates(dPlayerActor_c *player, int *x, int *z) {
    *x = player->_2190;
    *z = player->_2194;
}

// 80100440
BOOL receiveItem(const dItem::Item *item, int b, int c, int d, dActor_c *partner, f32 f1, f32 f2, f32 f3) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194A2C(item, b, c, d, player, partner, getItemExchangeOffsetY(), f1, 0.0f, f2, 0.0f, f3)) {
        return FALSE;
    }
    if (!fn_80194AA0(0, player, 16.0f)) {
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
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194A2C(item, b, c, d, player, partner, getItemExchangeOffsetY(), f1, 0.0f, f2, 0.0f, 0.0f)) {
        return FALSE;
    }
    if (!fn_80194AA0(1, player, 16.0f)) {
        return FALSE;
    }
    return changeState(STATE_GIVE_ITEM, PLAYER_INDEX_CURRENT);
}

// 80100644
BOOL fn_80100644(BOOL a) {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    if (a) {
        return changeState(0x66, PLAYER_INDEX_CURRENT);
    }
    return changeState(0x67, PLAYER_INDEX_CURRENT);
}

// 801006A8
BOOL fn_801006A8() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194AA0(fn_80194B30(), player, 16.0f)) {
        return FALSE;
    }
    return changeState(0x6E, PLAYER_INDEX_CURRENT);
}

// 80100714
BOOL setShoeColor(u8 shoeColor, u8 b) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
    player->_2234 = 0x1E;
    return TRUE;
}

// 801007AC
BOOL fn_801007AC() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x85) <= 2) {
        return FALSE;
    }
    return changeState(0x85, PLAYER_INDEX_CURRENT);
}

// 80100804
BOOL fn_80100804() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x87 || state != 0x86) {
        return FALSE;
    }
    return changeState(0x87, PLAYER_INDEX_CURRENT);
}

// 80100860
BOOL fn_80100860() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x1A) {
        dUki_c::setFish(dNet::getMyNo(), NULL);
        changeState(0x1D, PLAYER_INDEX_CURRENT);
        return FALSE;
    }
    if (state != STATE_IDLE && state != STATE_WALK) {
        return FALSE;
    }
    return changeState(0x6F, PLAYER_INDEX_CURRENT);
}

// 801008E4
BOOL isUsingNet(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->_2270 & 0x20)) {
        return FALSE;
    }
    if (player->_0C34 != NULL && player->_0C34->getToolType() == HMN_TOOL_NET) {
        return TRUE;
    }
    return FALSE;
}

// 8010094C
BOOL isRunAnimation(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x3;
}

// 8010098C
BOOL fn_8010098C() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x70) {
        return TRUE;
    }
    return changeState(0x70, PLAYER_INDEX_CURRENT);
}

// 801009E0
BOOL playEmotion(u32 type) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (type >= 0x1E) {
        return FALSE;
    }
    if (player->mState == STATE_EMOTION) {
        return FALSE;
    }
    player->_22DE = type;
    return changeState(STATE_EMOTION, PLAYER_INDEX_CURRENT);
}

// 80100A54
BOOL fn_80100A54() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x84) {
        return FALSE;
    }
    player->_22DD = 0;
    return changeState(0x84, PLAYER_INDEX_CURRENT);
}

// 80100AB0
BOOL fn_80100AB0() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x84) {
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
BOOL fn_80100B24() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x84) {
        return FALSE;
    }
    player->_22DD = 6;
    return changeState(0x84, PLAYER_INDEX_CURRENT);
}

// 80100B80
BOOL fn_80100B80() {
    return fn_80100AB0();
}

// 80100B84
BOOL fn_80100B84(f32 x, f32 z) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x72) <= 1) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    return changeState(0x72, PLAYER_INDEX_CURRENT);
}

// 80100C28
BOOL fn_80100C28() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x73 || state != 0x72) {
        return FALSE;
    }
    return changeState(0x73, PLAYER_INDEX_CURRENT);
}

// 80100C84
BOOL fn_80100C84(BOOL a) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->_2270 & 1) {
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
        player->_2274 |= 1;
    } else {
        player->_2274 &= ~1;
    }
    return changeState(0x74, PLAYER_INDEX_CURRENT);
}

// 80100D44
BOOL fn_80100D44() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= STATE_DEMO_START && state < 0x9) || (state >= 0x58 && state < 0x5C)) {
        return TRUE;
    }
    if (player->_2270 & 1) {
        return TRUE;
    }
    return dDemo::isType(dDemo::TYPE_9) != 0;
}

// 80100DD0
BOOL fn_80100DD0(int idx) {
    dPlayerActor_c *player = getActor(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= STATE_WALK && state < STATE_MOVE_TO) || state == 0x27 || state == 0x29) {
        return TRUE;
    }
    return FALSE;
}

// 80100E3C
BOOL fn_80100E3C() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(0x8F, PLAYER_INDEX_CURRENT);
}

// 80100E7C
BOOL fn_80100E7C() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == 0x90;
}

// 80100EC0
BOOL fn_80100EC0() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(0x91, PLAYER_INDEX_CURRENT);
}

// 80100F00
BOOL fn_80100F00(s16 angle, f32 frame) {
    dPlayerActor_c *player = getActorRaw(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    player->mAngle.x = 0;
    player->mAngle.y = angle;
    player->mAngle.z = 0;
    if (changeState(0x92, PLAYER_INDEX_CURRENT)) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            anm->setFrame(frame);
        }
        return TRUE;
    }
    return FALSE;
}

// 80100F98
BOOL fn_80100F98() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == 0x93;
}

// 80100FDC
BOOL fn_80100FDC() {
    if (getActor(PLAYER_INDEX_CURRENT) == NULL) {
        return FALSE;
    }
    return changeState(0x94, PLAYER_INDEX_CURRENT);
}

// 8010101C
u8 fn_8010101C() {
    return gManager._194A8;
}

// 80101030
void fn_80101030() {
    gManager._194A8 = TRUE;
}

// 80101048
void fn_80101048() {
    gManager._194A8 = FALSE;
}

// 80101060
BOOL changeHeldItemForDemo(const dItem::Item *held) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
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
        player->_22DA = HELD_ITEM_CHANGE_EQUIP;
        player->_223C = 1;
        dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
        dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), dEquip_c::PART_HELD, held->mId);
    }
    return changeState(STATE_CHANGE_HELD_ITEM_DEMO, PLAYER_INDEX_CURRENT);
}

// 80101168
BOOL fn_80101168() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(0x95, PLAYER_INDEX_CURRENT);
}

// 801011BC
BOOL fn_801011BC() {
    return TRUE;
}

// 801011C4
BOOL getScriptedActionPosition(mVec3_c *out) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->_2270 & 0x10)) {
        return FALSE;
    }
    if (player->mState != 0x95) {
        return FALSE;
    }
    if (player->_2234 != 0x20) {
        return FALSE;
    }
    *out = player->_20EC;
    return TRUE;
}

// 8010125C
BOOL startShoeShine(s16 frames, f32 speed) {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_DEMO_WAIT) {
        return FALSE;
    }
    mVec3_c target(1082.0f, 0.0f, 390.0f);
    player->_2068 = target;
    player->_21BC = speed;
    player->_22D0 = frames;
    return changeState(STATE_SHOE_SHINE_START, PLAYER_INDEX_CURRENT);
}

// 801012FC
BOOL finishShoeShine() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != STATE_SHOE_SHINE) {
        return FALSE;
    }
    return changeState(STATE_SHOE_SHINE_END, PLAYER_INDEX_CURRENT);
}

// 80101350
BOOL fn_80101350() {
    dPlayerActor_c *player = getActor(PLAYER_INDEX_CURRENT);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x76) {
        return TRUE;
    }
    if (state != STATE_DEMO_WAIT) {
        return FALSE;
    }
    return changeState(0x76, PLAYER_INDEX_CURRENT);
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
    gManager._194A7 = 0;
    gManager._194A8 = 0;
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
u8 isBedHairPending() {
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
u8 fn_8010152C() {
    return gManager._194A7;
}

// 80101540
void fn_80101540() {
    gManager._194A7 = TRUE;
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
    if (item->isOrgDesign() && (dItem::seeker_c::get()->findLike(*item) & 7) == slot) {
        return TRUE;
    }
    return FALSE;
}

// 801015F4
u32 getHeapSize() {
    return 0xEE880;
}

// 80101600
u32 fn_80101600() {
    return 0x1800;
}

// 80101608
u32 fn_80101608() {
    return 0xC00;
}

// 80101610
u32 fn_80101610() {
    return 0x800;
}

// 80101618
u32 fn_80101618() {
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
                const dPersonalID_c *guest = &gManager.mGuests.mPlayers[i].mPID;
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
                if (id->isSame(&gManager.mGuests.mPlayers[i].mPID.player)) {
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
void setActor(dPlayerActor_c *actor, u32 idx) {
    if (idx < PLAYER_INDEX_COUNT) {
        gManager.mActors[idx] = actor;
    }
}

// 801019C4
dPlayerActor_c *getActor(int idx) {
    if ((u32)idx > PLAYER_INDEX_CURRENT) {
        return NULL;
    }
    if (idx == PLAYER_INDEX_CURRENT) {
        idx = dNet::getMyNo();
    }
    dPlayerActor_c *actor = gManager.mActors[idx];
    if (actor != NULL && (actor->_2270 & ACTOR_FLAG_DISABLED)) {
        return NULL;
    }
    return actor;
}

// 80101A28
dPlayerActor_c *getActorRaw(int idx) {
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
    if (i == -1) {
        return NULL;
    }
    return &mPlayers[i];
}

// 80101C68
dPrivateData_c *GuestPlayers_c::getRaw(int slot) {
    int i = getIndex(slot);
    if (i == -1) {
        return NULL;
    }
    return &mPlayers[i];
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
void setNetworkPosition(int idx, const mVec3_c *value) {
    dPlayerActor_c *player = getActor(idx);
    if (player != NULL) {
        player->_2308 = *value;
    }
}

// 80101D0C
void setRemoteActionSync(int idx, int a, int b) {
    dPlayerActor_c *player = getActor(idx);
    if (player != NULL && !isActionParamLocked(player->mState)) {
        player->_2230 = a;
        player->_226C = b;
    }
}

// 80101D70
void setRemoteSyncValue(int idx, int value) {
    dPlayerActor_c *player = getActor(idx);
    if (player != NULL) {
        player->_2280 = value;
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
BOOL cacheUnavailableActorAction(int idx, u8 b, int state, const u8 *param) {
    cacheRemoteState(idx, state);
    cacheRemoteParams(idx, param);
    return updateActionFieldTasks(idx, b, state, param);
}

// 80101E3C
BOOL cacheOtherSceneAction(int idx, u8 b, int state, const u8 *param) {
    cacheRemoteState(idx, state);
    cacheRemoteParams(idx, param);
    return updateActionFieldTasks(idx, b, state, param);
}

// 80101EB0
BOOL updateActionFieldTasks(int idx, u8 b, int state, const u8 *param) {
    switch (state) {
    case 0x12:
    case 0x3C:
    case 0x3D:
    case 0x55:
    case 0x57:
    case 0x5B:
        fgMngTask_procGroupAt(param[0], param[1], 0);
        break;
    case 0x13:
    case 0x41:
        if (param[5] != 0) {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x40:
        fgMngTask_procGroupAt(param[4], param[5], 0);
        break;
    case 0x45:
        fgMngTask_procGroupAt(param[2], param[3], 0);
        break;
    case 0x46:
    case 0x52:
    case 0x53:
        if (param[4] != 0) {
            fgMngTask_releaseAt(b, param[2], param[3], 0);
        } else {
            fgMngTask_procGroupAt(param[2], param[3], 0);
        }
        break;
    case 0x59:
        fgMngTask_releaseAt(b, param[0], param[1], 0);
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
        return FALSE;
    }
    if (b < 0 && a < 0) {
        if (a > b) {
            return TRUE;
        }
        return FALSE;
    }
    if ((b > 0 && a < 0) || (b < 0 && a > 0)) {
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
u8 isActionParamLocked(u32 state) {
    if (state >= STATE_COUNT) {
        return 0;
    }
    return sActionParamLocked[state];
}

// Fashion types 0x28 and 0x36..0x38 (getWearSound).
static inline BOOL isSpecialFashion(const dItem::Item *item) {
    int fashion = item->getFashion();
    if (fashion == dItem::FASHION_RED_HERO || (fashion >= dItem::FASHION_GREEN_HERO && fashion < dItem::FASHION_COUNT)) {
        return TRUE;
    }
    return FALSE;
}

// 801021D0
int getWearSound(const dItem::Item *a, const dItem::Item *b) {
    if (a->mId != dItem::ITEM_ID_NONE && isSpecialFashion(a)) {
        return SE_WearHero;
    }
    if (b->mId == dItem::ITEM_ID_NONE) {
        return SE_wear;
    }
    if (isSpecialFashion(b)) {
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
    return SE_wear;
}

// 801022B8: splash/snow effect callback for createSurfaceEffects.
void onParticleCollision(nw4r::ef::Particle *obj, u32 kind) {
    if (obj == NULL) {
        return;
    }
    mVec3_c pos;
    if (dScene::isOutdoorScene((u8)dScene::getCurrentScene())) {
        mVec3_c src = obj->mParameter.mPosition;
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
            if (fabsf(ground - pos.y) <= 5.0f) {
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
        int mode = dWeather_c::sInstance->mWeather;
        BOOL special = FALSE;
        if (mode == dWeather::HEAVY_RAIN || mode == dWeather::RAIN) {
            special = TRUE;
        }
        if (special) {
            return SURFACE_EFFECT_RAIN;
        }
    }
    if (attr == BG_ATTR_SAND) {
        return SURFACE_EFFECT_SAND;
    }
    if ((u8)dGroundAttr::getSnowWear(pos) > sSnowEffectWearThreshold) {
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
    BOOL wet = FALSE;
    if (kind == SURFACE_EFFECT_SEA || kind == SURFACE_EFFECT_RAIN) {
        wet = TRUE;
    }
    const char *effect = snowEffect;
    if (wet) {
        effect = wetEffect;
    }
    dEffect::create(effect, pos, angle, NULL, onParticleCollision, kind);
}

// 801025B8
BOOL fn_801025B8() {
    return FALSE;
}

// 801025C0: __sinit_d_player_mgr_cpp (the look-angle limits and gManager).


} // namespace dPlayerMgr
