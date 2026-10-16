// The player manager process (dPlayerMngr_c). .text 801B76BC..801B96A4, .data
// 804FC938..804FC9A0, .bss 80625940..806259E0, .sdata 8074C318..8074C330, .sbss
// 8074EB78..8074EB88, .sdata2 80751980..807519A0. See
// include/game/game/d_player_mngr.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

CUSTOM_BASE_PROFILE(PLAYER_MNGR, dPlayerMngr_c, fProf::PLAYER_MNGR,
                    fProf::DRAW_ORDER::PLAYER_MNGR);

u8 dPlayerMngr_c::sRemoteScene[MEMBER_NUM] = {
    dScene::SCENE_NUM, dScene::SCENE_NUM, dScene::SCENE_NUM, dScene::SCENE_NUM};
u8 dPlayerMngr_c::sRemoteState[MEMBER_NUM] = {
    dPlayerMgr::STATE_NONE, dPlayerMgr::STATE_NONE, dPlayerMgr::STATE_NONE,
    dPlayerMgr::STATE_NONE};

dPlayerMngr_c::equip_c dPlayerMngr_c::sEquip[PART_NUM];
dPlayerMngr_c::actionParam_c dPlayerMngr_c::sRemoteParam[MEMBER_NUM];
u32 dPlayerMngr_c::sRemoteInfo[MEMBER_NUM];

dPlayerMngr_c *dPlayerMngr_c::sInstance;
u8 dPlayerMngr_c::sEquipChecked[MEMBER_NUM];
u8 dPlayerMngr_c::sSyncRequest;

// 801B76EC
dPlayerMngr_c::equip_c::equip_c() {
    for (int i = 0; i < MEMBER_NUM; i++) {
        mSyncItem[i] = dItem::ITEM_ID_NONE;
        mItem[i] = dItem::ITEM_ID_NONE;
        mState[i] = EQUIP_STATE_NONE;
        mSyncState[i] = EQUIP_STATE_NONE;
    }
}

// 801B773C
dPlayerMngr_c::dPlayerMngr_c() {
    for (int i = 0; i < MEMBER_NUM; i++) {
        mCreated[i] = FALSE;
    }
    mSavedSlot = dPlayerMgr::PLAYER_SLOT_NONE;
}

// 801B77F4
dPlayerMngr_c::~dPlayerMngr_c() {
}

// 801B789C
BOOL dPlayerMngr_c::create() {
    sInstance = this;
    for (int i = 0; i < MEMBER_NUM; i++) {
        mCloth[i].setSlot(i);
        mHeadItem[i].setSlot(i);
        mToolBank[i].create(i);
    }
    createVisitorActor();
    if (!syncEquipment()) {
        return FALSE;
    }
    if (dNet::isSessionActive()) {
        u8 prev = dScene::getPrevScene();
        // The save demo or one of the three gate connection demos.
        if (prev == dScene::SCENE_DM_SAVE ||
            prev == dScene::SCENE_DM_CHKP_CNNCT ||
            prev == dScene::SCENE_DM_CHKP_DCNNCT ||
            prev == dScene::SCENE_DM_CHKP_MISS) {
            for (int i = 0; i < MEMBER_NUM; i++) {
                if (i != dNet::getMyNo() && dNet::isMember(i)) {
                    u32 info;
                    u8 scene;
                    u8 state;
                    if (getRemoteStateInfo(&info, &scene, &state, i) &&
                        state != 0 && scene == dScene::getCurrentScene() &&
                        scene == sRemoteScene[i] && info == sRemoteInfo[i]) {
                        mPresent[i] = TRUE;
                    }
                }
            }
        }
    }
    return TRUE;
}

// 801B7A08
BOOL dPlayerMngr_c::doDelete() {
    if (sInstance != NULL && !syncLoad()) {
        return FALSE;
    }
    for (int i = 0; i < MEMBER_NUM; i++) {
        getPlayerToolBank(i)->remove();
        mCreated[i] = FALSE;
        mPresent[i] = FALSE;
    }
    if (dNet::isSessionActive()) {
        for (int i = 0; i < MEMBER_NUM; i++) {
            if (i != dNet::getMyNo()) {
                u32 info;
                u8 scene;
                u8 state;
                if (getRemoteStateInfo(&info, &scene, &state, i)) {
                    if (scene != dScene::SCENE_DM_SAVE &&
                        scene != dScene::SCENE_DM_CHKP_CNNCT &&
                        scene != dScene::SCENE_DM_CHKP_DCNNCT &&
                        scene != dScene::SCENE_DM_CHKP_MISS) {
                        if (state == 0) {
                            sRemoteScene[i] = dScene::SCENE_NUM;
                        } else {
                            sRemoteScene[i] = scene;
                        }
                        sRemoteInfo[i] = info;
                    }
                } else {
                    sRemoteScene[i] = dScene::SCENE_NUM;
                }
            }
        }
    }
    if (mSlotOverridden) {
        mSlotOverridden = FALSE;
        dPlayerMgr::setPlayerSlot(1, mSavedSlot);
    }
    sInstance = NULL;
    return TRUE;
}

// 801B7B60
BOOL dPlayerMngr_c::draw() {
    return TRUE;
}

// 801B7B68
BOOL dPlayerMngr_c::execute() {
    updateRemoteActors();
    for (int i = 0; i < MEMBER_NUM; i++) {
        executeEquip(i, dEquip_c::PART_SHIRT);
        executeEquip(i, dEquip_c::PART_HAT);
        executeEquip(i, dEquip_c::PART_ACCESSORY);
        executeEquip(i, dEquip_c::PART_HELD);
    }
    return TRUE;
}

// 801B7BF0
u32 dPlayerMngr_c::makeActorParam(int member, u32 state) {
    return state | (member << ACTOR_PARAM_MEMBER_SHIFT);
}

// 801B7BFC
u32 dPlayerMngr_c::setActorParamRemote(int remote, u32 param) {
    if (remote) {
        return param | ACTOR_PARAM_REMOTE;
    }
    return param & ~ACTOR_PARAM_REMOTE;
}

// 801B7C10
// state: the player state the actor starts in.
dActor_c *dPlayerMngr_c::createActor(int member, int remote, u32 state,
                                     const mVec3_c *pos, const mAng3_c *angle) {
    if (member >= MEMBER_NUM) {
        return NULL;
    }
    u32 param = setActorParamRemote(remote, makeActorParam(member, state));
    return dActor_c::construct(PLAYER_PROFILE, dScene::getSceneParent(), param,
                               pos, angle);
}

// 801B7C94
void dPlayerMngr_c::createVisitorActor() {
    if (!dNet::isSessionActive()) {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            return;
        }
        int room;
        int home =
            dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
        if (home == -1) {
            return;
        }
        if (room != HOME_SCENE_LOFT) {
            return;
        }
        dHome_c *house = dSaveData_c::getTown()->mHomes.getHome(home);
        if (house == NULL) {
            return;
        }
        dPersonalID_c *owner = house->getOwner();
        if (!owner->isValid()) {
            return;
        }
        dPersonalID_c *pid = &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID;
        if (*owner == *pid) {
            return;
        }
        for (int i = 0; i < MEMBER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
            if (player == NULL) {
                continue;
            }
            pid = &player->mPID;
            if (*owner == *pid && !mCreated[1] &&
                dPlayerMgr::getPlayerActorRaw(1) == NULL) {
                mSavedSlot = dPlayerMgr::getPlayerSlot(1);
                dPlayerMgr::setPlayerSlot(1, i);
                mSlotOverridden = TRUE;
                mVec3_c pos(240.0f, 0.0f, 384.0f);
                pos.y = dBGCF::getGroundY(&pos, FALSE);
                dPrivateData_c *data = dPlayerMgr::Mgr_c::getNetPlayerRaw(1);
                if (data != NULL && (data->mEquipment.mShirt.isOrgDesign() ||
                                     data->mEquipment.mHat.isOrgDesign())) {
                    dPlayerMgr::clearPlayerEquipmentManagers(1);
                }
                createActor(1, 1, dPlayerMgr::STATE_BED_WAIT, &pos, NULL);
                mCreated[1] = TRUE;
                return;
            }
        }
    } else {
        if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
            return;
        }
        int member;
        if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT ||
            dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
            member = dSvMgr_c::mCurrentVisitor;
        } else {
            member = dNet::getLeaveMember();
        }
        if (member == dNet::getMyNo()) {
            return;
        }
        if ((u32)member >= MEMBER_NUM) {
            return;
        }
        if (mCreated[member]) {
            return;
        }
        if (dPlayerMgr::getPlayerActorRaw(member) != NULL) {
            return;
        }
        mVec3_c pos(256.0f, 0.0f, 464.0f);
        mAng3_c angle(0, -0x8000, 0);
        if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_CNNCT) {
            pos.set(256.0f, 0.0f, 48.0f);
            angle.set(0, 0, 0);
            dPlayerMgr::clearPlayerEquipmentManagers(member);
        } else if (dScene::getCurrentScene() == dScene::SCENE_DM_CHKP_MISS) {
            pos.set(256.0f, 0.0f, 0.0f);
            angle.set(0, 0, 0);
            dPlayerMgr::clearPlayerEquipmentManagers(member);
        } else {
            pos.set(256.0f, 0.0f, 304.0f);
            angle.set(0, -0x8000, 0);
        }
        createActor(member, 1, dPlayerMgr::STATE_DEMO_WAIT, &pos, &angle);
        mCreated[member] = TRUE;
    }
}

// 801B8048
void dPlayerMngr_c::updateRemoteActors() {
    if (!dNet::isSessionActive()) {
        return;
    }
    if (dScene::getCurrentScene() == dScene::SCENE_DM_SAVE) {
        return;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
        return;
    }
    for (int i = 0; i < MEMBER_NUM; i++) {
        if (i == dNet::getMyNo()) {
            continue;
        }
        if (mPresent[i]) {
            u32 info;
            u8 scene;
            u8 state;
            if (getRemoteStateInfo(&info, &scene, &state, i) &&
                info != sRemoteInfo[i]) {
                mPresent[i] = FALSE;
            }
            if (mPresent[i] && !mCreated[i] &&
                dPlayerMgr::getPlayerActorRaw(i) == NULL &&
                dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT) !=
                    NULL &&
                scene == dScene::getCurrentScene()) {
                state--;
                int type = getRemoteCreateState(state);
                f32 x;
                f32 z;
                s16 rotY;
                getRemoteTransform(&x, &z, &rotY, i);
                mVec3_c pos(x, 0.0f, z);
                pos.y = dBGCF::getGroundY(&pos, FALSE);
                mAng3_c angle(0, rotY, 0);
                createActor(i, 0, type, &pos, &angle);
                mCreated[i] = TRUE;
            }
        }
        if (!mCreated[i] && dPlayerMgr::getPlayerActorRaw(i) == NULL) {
            if (dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT) ==
                NULL) {
                continue;
            }
            u32 info;
            u8 scene;
            u8 state;
            if (!getRemoteStateInfo(&info, &scene, &state, i) || state == 0 ||
                scene != dScene::getCurrentScene()) {
                continue;
            }
            state--;
            f32 x;
            f32 z;
            s16 rotY;
            getRemoteTransform(&x, &z, &rotY, i);
            mVec3_c pos(x, 0.0f, z);
            pos.y = dBGCF::getGroundY(&pos, FALSE);
            mAng3_c angle(0, rotY, 0);
            createActor(i, 0, state, &pos, &angle);
            mCreated[i] = TRUE;
        } else if (mCreated[i] == TRUE &&
                   dPlayerMgr::getPlayerActorRaw(i) != NULL && !mPresent[i]) {
            dPlayerActor_c *actor = dPlayerMgr::getPlayerActorRaw(i);
            if (actor->getLifecycleState() != DELETING) {
                u32 info;
                u8 scene;
                u8 state;
                if (getRemoteStateInfo(&info, &scene, &state, i) &&
                    scene != dScene::getCurrentScene()) {
                    actor->deleteRequest();
                    mCreated[i] = FALSE;
                    mToolBank[i].putAway();
                }
            }
        }
    }
}

// 801B835C
void dPlayerMngr_c::queueEquip(int idx, u32 part, int item) {
    if (idx == dNet::getMyNo() || part >= PART_NUM || (u32)idx >= MEMBER_NUM) {
        return;
    }
    if (part == dEquip_c::PART_HELD) {
        equip_c *equip = &sEquip[part];
        equip->mState[idx] = EQUIP_STATE_LOAD;
        equip->mItem[idx] = item;
    } else if (!sEquipChecked[idx]) {
        equip_c *equip = &sEquip[part];
        equip->mState[idx] = EQUIP_STATE_LOAD;
        equip->mItem[idx] = item;
    }
}

// 801B8428
BOOL dPlayerMngr_c::requestEquip(int idx, u8 part, u16 item) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
    switch (part) {
    case dEquip_c::PART_SHIRT: {
        dItem::Item shirt(item);
        return mCloth[idx].request(&shirt, player, NULL);
    }
    case dEquip_c::PART_HAT: {
        dItem::Item hat(item);
        return mHeadItem[idx].requestCap(&hat, player);
    }
    case dEquip_c::PART_ACCESSORY: {
        dItem::Item acc(item);
        return mHeadItem[idx].requestAccessory(&acc);
    }
    case dEquip_c::PART_HELD: {
        dItem::Item held(item);
        return mToolBank[idx].request(&held, player, NULL);
    }
    }
    return TRUE;
}

// 801B8520
void dPlayerMngr_c::executeEquip(int idx, u8 part) {
    u8 *state = &sEquip[part].mState[idx];
    switch (*state) {
    case EQUIP_STATE_NONE:
        break;
    case EQUIP_STATE_LOAD:
        if (requestEquip(idx, part, sEquip[part].mItem[idx])) {
            *state = EQUIP_STATE_LOADED;
        }
        break;
    case EQUIP_STATE_CHANGE:
        if (requestEquip(idx, part, sEquip[part].mItem[idx])) {
            *state = EQUIP_STATE_CHANGED;
        }
        break;
    case EQUIP_STATE_LOADED:
    case EQUIP_STATE_CHANGED:
        break;
    }
}

// 801B85C0
BOOL dPlayerMngr_c::isEquipReady(u32 idx, int part) {
    if (idx >= MEMBER_NUM) {
        return TRUE;
    }
    switch (sEquip[part].mState[idx]) {
    case EQUIP_STATE_NONE:
    case EQUIP_STATE_LOAD:
    case EQUIP_STATE_CHANGE:
        return FALSE;
    case EQUIP_STATE_LOADED:
    case EQUIP_STATE_CHANGED:
        return TRUE;
    }
    return TRUE;
}

// 801B8620
BOOL dPlayerMngr_c::checkEquip(u32 idx, int part) {
    if (sInstance == NULL) {
        return FALSE;
    }
    sEquipChecked[idx] = TRUE;
    if (idx >= MEMBER_NUM) {
        return TRUE;
    }
    equip_c *equip = &sEquip[part];
    u8 *state = &equip->mState[idx];
    if (*state == EQUIP_STATE_LOADED || *state == EQUIP_STATE_CHANGED) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
        switch (part) {
        case dEquip_c::PART_SHIRT:
            if (equip->mItem[idx] == player->mEquipment.mShirt.getId()) {
                *state = EQUIP_STATE_CHANGED;
                return TRUE;
            }
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mShirt.mId;
            break;
        case dEquip_c::PART_HAT:
            if (equip->mItem[idx] == player->mEquipment.mHat.getId()) {
                *state = EQUIP_STATE_CHANGED;
                return TRUE;
            }
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mHat.mId;
            break;
        case dEquip_c::PART_ACCESSORY:
            if (equip->mItem[idx] == player->mEquipment.mAcc.getId()) {
                *state = EQUIP_STATE_CHANGED;
                return TRUE;
            }
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mAcc.mId;
            break;
        case dEquip_c::PART_HELD: {
            u16 held = player->mEquipment.mHeld.mId;
            if (held == equip->mItem[idx]) {
                if (held == dItem::ITEM_ID_NONE) {
                    *state = EQUIP_STATE_LOADED;
                    return TRUE;
                }
                if (sInstance->mToolBank[idx].hasResMdl()) {
                    *state = EQUIP_STATE_LOADED;
                    return TRUE;
                }
            }
            *state = EQUIP_STATE_LOAD;
            equip->mItem[idx] = held;
            break;
        }
        }
    } else if (*state != EQUIP_STATE_CHANGE) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
        switch (part) {
        case dEquip_c::PART_SHIRT:
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mShirt.mId;
            break;
        case dEquip_c::PART_HAT:
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mHat.mId;
            break;
        case dEquip_c::PART_ACCESSORY:
            *state = EQUIP_STATE_CHANGE;
            equip->mItem[idx] = player->mEquipment.mAcc.mId;
            break;
        case dEquip_c::PART_HELD:
            *state = EQUIP_STATE_LOAD;
            equip->mItem[idx] = player->mEquipment.mHeld.mId;
            break;
        }
    }
    return FALSE;
}

// 801B8898
void dPlayerMngr_c::setLocalEquip(int idx, u8 part, u16 item) {
    if (idx == dNet::getMyNo()) {
        sEquip[part].mItem[idx] = item;
        sEquip[part].mState[idx] = EQUIP_STATE_LOAD;
        sendEquipQueue(part, item);
    }
}

// 801B8918: the reload a remote player's actor needs for a part:
// EQUIP_STATE_NONE when nothing changed, EQUIP_STATE_LOAD when the part was
// taken off, EQUIP_STATE_CHANGE when it changed.
int dPlayerMngr_c::updateEquip(int idx, int part) {
    equip_c *equip = &sEquip[part];
    u8 *state = &equip->mState[idx];
    if (*state == EQUIP_STATE_CHANGE) {
        *state = EQUIP_STATE_LOAD;
        return EQUIP_STATE_NONE;
    }
    if (sEquipChecked[idx]) {
        *state = EQUIP_STATE_LOADED;
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
        switch (part) {
        case dEquip_c::PART_SHIRT:
            if (equip->mItem[idx] != player->mEquipment.mShirt.getId()) {
                *state = EQUIP_STATE_LOAD;
                equip->mItem[idx] = player->mEquipment.mShirt.mId;
                return EQUIP_STATE_CHANGE;
            }
            break;
        case dEquip_c::PART_HAT:
            if (equip->mItem[idx] != player->mEquipment.mHat.getId()) {
                *state = EQUIP_STATE_LOAD;
                u16 item = player->mEquipment.mHat.mId;
                equip->mItem[idx] = item;
                return item == dItem::ITEM_ID_NONE ? EQUIP_STATE_LOAD
                                                   : EQUIP_STATE_CHANGE;
            }
            break;
        case dEquip_c::PART_ACCESSORY:
            if (equip->mItem[idx] != player->mEquipment.mAcc.getId()) {
                *state = EQUIP_STATE_LOAD;
                u16 item = player->mEquipment.mAcc.mId;
                equip->mItem[idx] = item;
                return item == dItem::ITEM_ID_NONE ? EQUIP_STATE_LOAD
                                                   : EQUIP_STATE_CHANGE;
            }
            break;
        case dEquip_c::PART_HELD:
            if (equip->mItem[idx] != player->mEquipment.mHeld.getId()) {
                *state = EQUIP_STATE_LOAD;
                u16 item = player->mEquipment.mHeld.mId;
                equip->mItem[idx] = item;
                return item == dItem::ITEM_ID_NONE ? EQUIP_STATE_LOAD
                                                   : EQUIP_STATE_CHANGE;
            }
            break;
        }
    }
    return EQUIP_STATE_NONE;
}

// 801B8AC4
BOOL dPlayerMngr_c::isEquipLoaded(int idx, u8 part) {
    if (sInstance == NULL) {
        return FALSE;
    }
    equip_c *equip = &sEquip[part];
    u8 *state = &equip->mState[idx];
    if (*state == EQUIP_STATE_CHANGE) {
        if (sInstance->requestEquip(idx, part, equip->mItem[idx])) {
            *state = EQUIP_STATE_CHANGED;
            return TRUE;
        }
        return FALSE;
    }
    if (*state == EQUIP_STATE_CHANGED) {
        return TRUE;
    }
    if (*state == EQUIP_STATE_LOAD && part == dEquip_c::PART_HELD) {
        if (sInstance->requestEquip(idx, part, equip->mItem[idx])) {
            *state = EQUIP_STATE_LOADED;
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 801B8BB8
u8 dPlayerMngr_c::isEquipChecked(int idx) {
    return sEquipChecked[idx];
}

// 801B8BC4
void dPlayerMngr_c::clearEquipChecked(int idx) {
    sEquipChecked[idx] = FALSE;
}

// 801B8BD4
dHmnCloth_c *dPlayerMngr_c::getCloth(u32 idx) {
    if (sInstance == NULL) {
        return NULL;
    }
    if (idx >= MEMBER_NUM) {
        return NULL;
    }
    return &sInstance->mCloth[idx];
}

// 801B8C04
dHmnHeadItem_c *dPlayerMngr_c::getHeadItem(u32 idx) {
    if (sInstance == NULL) {
        return NULL;
    }
    if (idx >= MEMBER_NUM) {
        return NULL;
    }
    return &sInstance->mHeadItem[idx];
}

// 801B8C34
dHmnToolBank_c *dPlayerMngr_c::getToolBank(u32 idx) {
    if (sInstance == NULL) {
        return NULL;
    }
    if (idx >= MEMBER_NUM) {
        return NULL;
    }
    return &sInstance->mToolBank[idx];
}

// 801B8C68
void dPlayerMngr_c::resetEquipStates(int idx) {
    resetEquipState(idx, dEquip_c::PART_SHIRT);
    resetEquipState(idx, dEquip_c::PART_HAT);
    resetEquipState(idx, dEquip_c::PART_ACCESSORY);
    resetEquipState(idx, dEquip_c::PART_HELD);
    sEquipChecked[idx] = FALSE;
}

// 801B8CC8
void dPlayerMngr_c::resetEquipState(int idx, int part) {
    u8 *state = &sEquip[part].mState[idx];
    if (*state == EQUIP_STATE_CHANGED) {
        *state = EQUIP_STATE_LOADED;
    } else if (*state == EQUIP_STATE_CHANGE) {
        *state = EQUIP_STATE_LOAD;
    }
}

// 801B8D08
BOOL dPlayerMngr_c::syncLoad() {
    BOOL done = TRUE;
    for (int i = 0; i < MEMBER_NUM; i++) {
        if (!mCloth[i].syncLoad()) {
            done = FALSE;
        }
        if (!mHeadItem[i].syncLoad()) {
            done = FALSE;
        }
        if (!mToolBank[i].syncLoad()) {
            done = FALSE;
        }
    }
    return done;
}

// 801B8DAC
BOOL dPlayerMngr_c::syncEquipment() {
    if (sSyncRequest && !dNet::isHost()) {
        BOOL pending = FALSE;
        for (int i = 0; i < MEMBER_NUM; i++) {
            if (i == dNet::getMyNo()) {
                continue;
            }
            if (mSynced[i]) {
                for (int part = 0; part < PART_NUM; part++) {
                    equip_c *equip = &sEquip[part];
                    if (equip->mSyncState[i] != EQUIP_STATE_CHANGED) {
                        if (requestEquip(i, part, equip->mSyncItem[i])) {
                            equip->mItem[i] = equip->mSyncItem[i];
                            equip->mSyncState[i] = EQUIP_STATE_CHANGED;
                        } else {
                            pending = TRUE;
                        }
                    }
                }
            } else if (dNet::isMember(i)) {
                mSynced[i] = TRUE;
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(i);
                int item = dItem::ITEM_ID_NONE;
                for (int part = 0; part < PART_NUM; part++) {
                    if (part == dEquip_c::PART_SHIRT) {
                        item = player->mEquipment.mShirt.mId;
                    } else if (part == dEquip_c::PART_HAT) {
                        item = player->mEquipment.mHat.mId;
                    } else if (part == dEquip_c::PART_ACCESSORY) {
                        item = player->mEquipment.mAcc.mId;
                    } else if (part == dEquip_c::PART_HELD) {
                        item = player->mEquipment.mHeld.mId;
                    }
                    equip_c *equip = &sEquip[part];
                    equip->mSyncItem[i] = item;
                    equip->mSyncState[i] = EQUIP_STATE_CHANGE;
                    if (requestEquip(i, part, item)) {
                        equip->mItem[i] = equip->mSyncItem[i];
                        equip->mSyncState[i] = EQUIP_STATE_CHANGED;
                    } else {
                        pending = TRUE;
                    }
                }
            }
        }
        if (pending) {
            return FALSE;
        }
        for (int i = 0; i < MEMBER_NUM; i++) {
            mSynced[i] = FALSE;
            sEquip[dEquip_c::PART_SHIRT].mSyncItem[i] = dItem::ITEM_ID_NONE;
            sEquip[dEquip_c::PART_HAT].mSyncItem[i] = dItem::ITEM_ID_NONE;
            sEquip[dEquip_c::PART_ACCESSORY].mSyncItem[i] = dItem::ITEM_ID_NONE;
            sEquip[dEquip_c::PART_HELD].mSyncItem[i] = dItem::ITEM_ID_NONE;
        }
    }
    sSyncRequest = FALSE;
    return TRUE;
}

// 801B8FD8
void dPlayerMngr_c::setLocalAppearance(int idx, int kind) {
    if (idx == dNet::getMyNo()) {
        applyAppearance(idx, kind);
        sendAppearance(kind);
    }
}

// 801B902C: kind is a tan value (0-15) or a dPlayerMgr::AppearanceChange.
void dPlayerMngr_c::applyAppearance(int idx, int kind) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
    if (player == NULL) {
        return;
    }
    switch (kind) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        player->mTan = kind;
        break;
    case dPlayerMgr::APPEARANCE_CLEAR_BEE_STING:
        player->clearFlag0(PRIVATE_FLAG0_BEE_STUNG);
        break;
    case dPlayerMgr::APPEARANCE_MII_MASK_ON:
        player->mMiiMask = TRUE;
        break;
    case dPlayerMgr::APPEARANCE_MII_MASK_OFF:
        player->mMiiMask = FALSE;
        break;
    }
}

// 801B90C0
void dPlayerMngr_c::receiveAppearance(int idx, int kind) {
    if (idx != dNet::getMyNo()) {
        applyAppearance(idx, kind);
    }
}

// 801B910C
void dPlayerMngr_c::setLocalPlayerEquip(int idx, int part, u16 item) {
    if (idx == dNet::getMyNo()) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
        if (player != NULL) {
            switch (part) {
            case dEquip_c::PART_SHIRT:
                player->mEquipment.mShirt = item;
                break;
            case dEquip_c::PART_HAT:
                player->mEquipment.mHat = item;
                break;
            case dEquip_c::PART_ACCESSORY:
                player->mEquipment.mAcc = item;
                break;
            case dEquip_c::PART_HELD:
                player->mEquipment.mHeld = item;
                break;
            }
            sendEquip(part, item);
        }
    }
}

// 801B91B8
void dPlayerMngr_c::receiveEquip(int idx, int part, int item) {
    if (idx != dNet::getMyNo()) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(idx);
        if (player != NULL) {
            switch (part) {
            case dEquip_c::PART_SHIRT:
                player->mEquipment.mShirt = item;
                break;
            case dEquip_c::PART_HAT:
                player->mEquipment.mHat = item;
                break;
            case dEquip_c::PART_ACCESSORY:
                player->mEquipment.mAcc = item;
                break;
            case dEquip_c::PART_HELD:
                player->mEquipment.mHeld = item;
                break;
            }
        }
    }
}

// 801B9258
BOOL dPlayerMngr_c::getRemoteTransform(f32 *x, f32 *z, s16 *angle, u32 member) {
    if (member == dNet::getMyNo() || !dNet::isSessionActive() ||
        member >= MEMBER_NUM) {
        return FALSE;
    }
    u8 *pos = (u8 *)dNet::getRecord(NET_REC_MEMBER_A + member);
    cLib::memCpy(x, pos, sizeof(f32));
    cLib::memCpy(z, pos + 4, sizeof(f32));
    cLib::memCpy(angle, dNet::getRecord(NET_REC_MEMBER_B + member),
                 sizeof(s16));
    return TRUE;
}

// 801B9304
BOOL dPlayerMngr_c::getRemoteStateInfo(u32 *info, u8 *scene, u8 *state,
                                       u32 member) {
    if (member == dNet::getMyNo() || !dNet::isSessionActive() ||
        member >= MEMBER_NUM) {
        return FALSE;
    }
    u8 *rec = (u8 *)dNet::getRecord(NET_REC_MEMBER_C + member);
    cLib::memCpy(info, rec, sizeof(u32));
    cLib::memCpy(scene, rec + 4, sizeof(u8));
    cLib::memCpy(state, rec + 5, sizeof(u8));
    return TRUE;
}

// 801B93B8
void dPlayerMngr_c::sendEquip(int part, int item) {
    u8 partId = part;
    u16 itemId = item;
    if (dNet::isSessionActive()) {
        u8 data[3];
        cLib::memCpy(&data[0], &partId, sizeof(u8));
        cLib::memCpy(&data[1], &itemId, sizeof(u16));
        dNet::beginPacket();
        dNet::addPacket(data, sizeof(data));
        dNet::sendPacket(NET_PACKET_PLAYER_MNGR_25, NET_MEMBER_NONE);
    }
}

// 801B9424
void dPlayerMngr_c::sendEquipQueue(int part, int item) {
    u8 partId = part;
    u16 itemId = item;
    if (dNet::isSessionActive()) {
        u8 data[3];
        cLib::memCpy(&data[0], &partId, sizeof(u8));
        cLib::memCpy(&data[1], &itemId, sizeof(u16));
        dNet::beginPacket();
        dNet::addPacket(data, sizeof(data));
        dNet::sendPacket(NET_PACKET_PLAYER_MNGR_26, NET_MEMBER_NONE);
    }
}

// 801B9490
void dPlayerMngr_c::sendAppearance(int kind) {
    u8 kindId = kind;
    if (dNet::isSessionActive()) {
        u8 data[3];
        cLib::memCpy(&data[0], &kindId, sizeof(u8));
        dNet::beginPacket();
        dNet::addPacket(data, sizeof(u8));
        dNet::sendPacket(NET_PACKET_PLAYER_MNGR_24, NET_MEMBER_NONE);
    }
}

// 801B94E8
void dPlayerMngr_c::setRemoteState(u32 idx, int state) {
    if (idx < MEMBER_NUM) {
        sRemoteState[idx] = state;
    }
}

// 801B94FC
u8 dPlayerMngr_c::getRemoteState(u32 idx) {
    if (idx >= MEMBER_NUM) {
        return 0;
    }
    return sRemoteState[idx];
}

// 801B9518
void dPlayerMngr_c::setRemoteParam(u32 idx, const actionParam_c *param) {
    if (idx < MEMBER_NUM) {
        sRemoteParam[idx] = *param;
    }
}

// 801B9548
BOOL dPlayerMngr_c::getRemoteParam(u32 idx, actionParam_c *param) {
    if (idx >= MEMBER_NUM) {
        return FALSE;
    }
    *param = sRemoteParam[idx];
    return TRUE;
}

// 801B9588
u8 dPlayerMngr_c::isRemotePresent(u32 idx) {
    if (sInstance == NULL || idx >= MEMBER_NUM) {
        return FALSE;
    }
    return sInstance->mPresent[idx];
}

// 801B95B0: the state a remote player's actor is created in, from the state it
// was in.
int dPlayerMngr_c::getRemoteCreateState(int state) {
    int ret = dPlayerMgr::STATE_IDLE;
    switch (state) {
    case dPlayerMgr::STATE_DEMO_WAIT:
    case dPlayerMgr::STATE_BED_WAIT:
    case dPlayerMgr::STATE_CHAIR_WAIT:
        ret = state;
        break;
    // 0x36, 0x37, 0x2B, 0x2F..0x31: unnamed sit-down / lie-down transition
    // states.
    case 0x36:
    case 0x37:
        ret = dPlayerMgr::STATE_CHAIR_WAIT;
        break;
    case 0x2B:
    case 0x2F:
    case 0x30:
    case 0x31:
        ret = dPlayerMgr::STATE_BED_WAIT;
        break;
    }
    return ret;
}

// 801B9610
void dPlayerMngr_c::requestSync() {
    sSyncRequest = TRUE;
}

// 801B961C
void dPlayerMngr_c::clearAllEquip() {
    for (int i = 0; i < MEMBER_NUM; i++) {
        clearEquip(i);
    }
}

// 801B9658
void dPlayerMngr_c::clearEquip(int idx) {
    for (int part = 0; part < PART_NUM; part++) {
        sEquip[part].mItem[idx] = dItem::ITEM_ID_NONE;
    }
}
