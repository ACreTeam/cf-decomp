// The villager talk on the Harvest Festival: the ingredient request and the location remarks.
// .text 80045154..8004664C. See include/game/game/d_npc_talk_harvest.hpp.
#include <game/game/d_npc_talk_harvest.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_npc_pos.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// A location check of msgHarvestSpot: TRUE and *code set when the festival spot's acre matches.
typedef BOOL (talk_c::*locFunc)(u16 *code, int blockX, int blockZ, int unitX, int unitZ);

// 804A2BBC: label table of kind TALK_HARVEST (getMsgLabel)
const char *l_harvestLabels[3] = {l_Ai_Quest, l_Q_Cancel, "Ev_Harvest"};

// 80045154
const talk_c::hookFunc dAcNpcNml_c::talk_c::getHarvestChoice(u16 *code, u8 *flag, BOOL checkEvent) {
    BOOL ok = FALSE;
    if (!checkEvent || dEvent::isOngoing(EVENT_HARVEST_FESTIVAL)) {
        ok = TRUE;
    }
    u16 dummyCode = 1;
    u8 dummyFlag = 1;
    if (code == NULL) {
        code = &dummyCode;
    }
    if (flag == NULL) {
        flag = &dummyFlag;
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = getAnimal();
    if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_HARVEST_FESTIVAL || animal == NULL ||
        !animal->mEvent.isInEvent() || animal->mEvent.isFlag(1)) {
        ok = FALSE;
    }
    if (ok) {
        dItem::Item item(dItem::ITEM_IDX_KNIFE_AND_FORK);
        if (!countPocketsItem(&item, 0, NULL, NULL)) {
            ok = FALSE;
        }
    }
    if (ok) {
        *code = 0x13;
        *flag = 1;
        return &talk_c::selForkGive;
    }
    return NULL;
}

// 800452B0
int dAcNpcNml_c::talk_c::msgHarvest(msgInfo_s *info) {
    hookFunc proc = getHarvestChoice(NULL, NULL, TRUE);
    if (proc) {
        const char *label = getMsgLabel(TALK_HARVEST, 0, 0);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setProcSet(&l_talkEntrySets[TALK_HARVEST]);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80045390
void dAcNpcNml_c::talk_c::endHarvest(int arg) {
    recordTalk(NULL);
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 80045420
BOOL dAcNpcNml_c::talk_c::stepHarvest(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        u16 code = 1;
        u8 flag = 1;
        hookFunc proc = getHarvestChoice(&code, &flag, FALSE);
        if (proc) {
            clearChoice();
            setChoice(0, code, flag, proc);
            setChoice(1, 4, 3, &talk_c::selHarvestNo);
            setChoiceNum(2);
            setChoiceCancel(1);
            showChoice();
            return TRUE;
        }
    }
    return FALSE;
}

// 80045544
void dAcNpcNml_c::talk_c::selForkGive() {
    dItem::Item item(dItem::ITEM_IDX_KNIFE_AND_FORK);
    u16 mask = 0;
    countPocketsItem(&item, 0, &mask, NULL);
    mask = ~mask;
    reqSelectItem(mask, 0x22, 1);
    mResultProc = &talk_c::resForkGive;
}

// 800455D0
void dAcNpcNml_c::talk_c::resForkGive() {
    BOOL taken = FALSE;
    if (!isMenuInvalid()) {
        u32 slot = getMenuSelSlot();
        if (slot < 15) {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                dItem::Item item = player->mPockets[slot];
                player->clearPocket(slot);
                requestItemActEx(7, &item, 0, 0, 0, 2);
                mNextResultProc = &talk_c::resForkGiven;
                taken = TRUE;
            }
        }
    }
    if (!taken) {
        setMsgProc(&talk_c::msgForkCancel);
        startMsg();
        reqMsgClose();
    }
}

// 800456E0
int dAcNpcNml_c::talk_c::msgForkCancel(msgInfo_s *info) {
    setLooksMsg(info, getMsgLabel(TALK_HARVEST, 1, 0), 0);
    return TRUE;
}

// 8004573C
void dAcNpcNml_c::talk_c::resForkGiven() {
    setMsgProc(&talk_c::msgForkThanks);
    startMsg();
    reqMsgClose();
}

// 80045798
int dAcNpcNml_c::talk_c::msgForkThanks(msgInfo_s *info) {
    setLooksMsg(info, getMsgLabel(TALK_HARVEST, 2, 0), 1);
    setStepProc(&talk_c::stepForkThanks);
    return TRUE;
}

// 8004581C
BOOL dAcNpcNml_c::talk_c::stepForkThanks(int kind) {
    setStepProc(&talk_c::stepForkThanksWait);
    requestHandActD();
    return TRUE;
}

// 80045874
BOOL dAcNpcNml_c::talk_c::stepForkThanksWait(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgSpotIntro);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 800458EC
int dAcNpcNml_c::talk_c::msgSpotIntro(msgInfo_s *info) {
    setLooksMsg(info, getMsgLabel(TALK_HARVEST, 2, 0), 7);
    setStepProc(&talk_c::stepSpotIntro);
    return TRUE;
}

// 80045970
BOOL dAcNpcNml_c::talk_c::stepSpotIntro(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setMsgProc(&talk_c::msgHarvestSpot);
        startMsg();
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            animal->mEvent.setFlag(1);
            fn_800F0108(getNpcIdx(), animal->mEvent.mFlags);
        }
        return TRUE;
    }
    return FALSE;
}

// 80045A20
BOOL dAcNpcNml_c::talk_c::checkSpotNpcBlock(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    int x = -1;
    int z = -1;
    if (isCurrentSceneAttr(SCENE_ATTR_VILLAGER_HOUSE)) {
        u32 idx = getNpcIdx();
        if (idx < 10) {
            int houseX = 0;
            int houseZ = 0;
            dSaveTown_c *town = dSaveData_c::getTown();
            if (town->mBuilding.mList.getNpcHousePos(&houseX, &houseZ, idx)) {
                x = houseX >> 4;
                z = houseZ >> 4;
            }
        }
    } else {
        const mVec3_c *pos = mpNpc != NULL ? mpNpc->getPosP() : NULL;
        if (pos != NULL) {
            x = (int)pos->x >> 9;
            z = (int)pos->z >> 9;
        }
    }
    if (x == blockX && z == blockZ) {
        *code = 0xB;
        return TRUE;
    }
    return FALSE;
}

// 80045B38
BOOL dAcNpcNml_c::talk_c::checkSpotPlayerHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    u32 home = dSaveData_c::getRaw()->mHomes.findCurrentPlayer();
    if (home < 4) {
        int houseX = 0;
        int houseZ = 0;
        dSaveTown_c *town = dSaveData_c::getTown();
        if (town->mBuilding.mList.getPlayerHousePos(&houseX, &houseZ, home)) {
            int x = houseX >> 4;
            int z = houseZ >> 4;
            if (x == blockX && z == blockZ) {
                *code = 0xC;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Player house home, through the const accessor.
static inline const dHome_c *getHouse(const dHomeList_c *homes, u32 home) {
    return homes->getHome(home);
}

// 80045C00
BOOL dAcNpcNml_c::talk_c::checkSpotOtherHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    int self = fn_801017B8();
    dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
    const dPersonalID_c *owner;
    for (int i = 0; i < 4; i++) {
        if (i == self) {
            continue;
        }
        u32 home = homes->findPlayer(i);
        if (home >= 4) {
            continue;
        }
        int houseX = 0;
        int houseZ = 0;
        dSaveTown_c *town = dSaveData_c::getTown();
        if (!town->mBuilding.mList.getPlayerHousePos(&houseX, &houseZ, home)) {
            continue;
        }
        int x = houseX >> 4;
        int z = houseZ >> 4;
        if (x != blockX || z != blockZ) {
            continue;
        }
        *code = 0xD;
        const dHome_c *house = getHouse(homes, home);
        if (house != NULL && house->mOwner.isValid()) {
            owner = &house->mOwner;
            if (owner->isValid()) {
                setPersonalName(owner, 0);
                dAnimal_c *animal = getAnimal();
                if (animal != NULL) {
                    u8 gender = owner->player.mGender;
                    u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                    if (unit == 0) {
                        clearWord(1);
                    } else {
                        getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 80045D8C
BOOL dAcNpcNml_c::talk_c::checkSpotNpcHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    int self = getNpcIdx();
    dSaveBuildingList_c *list = &dSaveData_c::getTown()->mBuilding.mList;
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    for (int i = 0; i < 10; i++) {
        if (i == self) {
            continue;
        }
        dAnimal_c *other = block->getAnimalConst(i);
        if (other == NULL || !other->mID.isValid()) {
            continue;
        }
        int houseX = 0;
        int houseZ = 0;
        if (!list->getNpcHousePos(&houseX, &houseZ, i)) {
            continue;
        }
        int x = houseX >> 4;
        int z = houseZ >> 4;
        if (x != blockX || z != blockZ) {
            continue;
        }
        *code = 0xE;
        setAnmPersonalName(&other->mID, 2);
        dAnimal_c *animal = getAnimal();
        int unit = 0;
        if (animal != NULL && animal->mID.isValid()) {
            u8 looks = animal->mID.getLooks(1);
            unit = fn_800F3F38(other->mID.getGender(1), looks);
        }
        if ((u16)unit == 0) {
            clearWord(3);
        } else {
            getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
        }
        return TRUE;
    }
    return FALSE;
}

// 80045F14
BOOL dAcNpcNml_c::talk_c::checkSpotLandmark(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    static const int l_blockFlags[6] = {BLOCK_KIND_FLAG_SHOP,   BLOCK_KIND_FLAG_TAILOR, BLOCK_KIND_FLAG_TOWN_HALL,
                                        BLOCK_KIND_FLAG_MUSEUM, BLOCK_KIND_FLAG_GATE,   0};
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL) {
        const dFdBlock_c *block = fd->getBlock(blockX, blockZ);
        if (block != NULL) {
            for (u32 i = 0; i < 5; i++) {
                if (block->hasFlag(l_blockFlags[i])) {
                    *code = i + 0xF;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 80045FC8
BOOL dAcNpcNml_c::talk_c::checkSpotWater(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL) {
        const dFdBlock_c *block = fd->getBlock(blockX, blockZ);
        if (block != NULL) {
            if (block->hasFlag(BLOCK_KIND_FLAG_FALL)) {
                *code = 0x14;
                return TRUE;
            }
            if (block->hasFlag(BLOCK_KIND_FLAG_RACCO)) {
                *code = 0x15;
                return TRUE;
            }
            if (block->hasPond()) {
                *code = 0x20;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80046098
BOOL dAcNpcNml_c::talk_c::checkSpotEmptyHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
    for (int i = 0; i < 4; i++) {
        int player = homes->getPlayerOfHome(i);
        BOOL owned = player >= 0 && player < 4;
        if (!owned) {
            int houseX = 0;
            int houseZ = 0;
            dSaveTown_c *town = dSaveData_c::getTown();
            if (town->mBuilding.mList.getPlayerHousePos(&houseX, &houseZ, i)) {
                int x = houseX >> 4;
                int z = houseZ >> 4;
                if (x == blockX && z == blockZ) {
                    *code = 0x16;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// 8004617C
BOOL dAcNpcNml_c::talk_c::checkSpotBuildSite(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL) {
        const dFdBlock_c *block = fd->getBlock(blockX, blockZ);
        if (block != NULL) {
            for (int uz = 0; uz < 16; uz++) {
                for (int ux = 0; ux < 16; ux++) {
                    if (block->getItem(0, 0, 0) != NULL) {
                        int x = (blockX << 4) + ux;
                        int z = (blockZ << 4) + uz;
                        if (dSaveBuildingList_c::get()->isBuildSite(x, z)) {
                            dItem::Item building = dSaveBuildingList_c::get()->getAt(x, z, 1);
                            if (building.mId == dItem::ITEM_ID_NONE) {
                                *code = 0x17;
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

// 8004627C
BOOL dAcNpcNml_c::talk_c::checkSpotBeach(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL && fd->hasBlockFlag(blockX, blockZ, BLOCK_KIND_FLAG_BEACH)) {
        *code = 0x18;
        return TRUE;
    }
    return FALSE;
}

// 800462F8
BOOL dAcNpcNml_c::talk_c::checkSpotGateLine(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL) {
        const dFdBlock_c *block = fd->findBlock(BLOCK_KIND_FLAG_GATE);
        if (block != NULL) {
            int z0 = block->mBlockZ;
            int x0 = block->mBlockX;
            if (z0 == blockZ) {
                if (blockX > 0 && blockX < x0) {
                    *code = 0x19;
                    return TRUE;
                }
                if (blockX < fd->getBlockW() - 1 && blockX > x0) {
                    *code = 0x1A;
                    return TRUE;
                }
            } else if (blockX == x0 && blockZ > z0 && blockZ < fd->getBlockH() - 1) {
                *code = 0x1B;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 800463F0
BOOL dAcNpcNml_c::talk_c::checkSpotEdge(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    const dFdBase_c *fd = fn_80190C44(1);
    if (fd != NULL) {
        int w = fd->getBlockW();
        int h = fd->getBlockH();
        if (blockX == 1 && blockZ > 1 && blockZ < h - 1) {
            *code = 0x1C;
            return TRUE;
        }
        if (blockX == w - 2 && blockZ > 1 && blockZ < h - 1) {
            *code = 0x1D;
            return TRUE;
        }
    }
    return FALSE;
}

// 800464A8
BOOL dAcNpcNml_c::talk_c::checkSpotDefault(u16 *code, int blockX, int blockZ, int unitX, int unitZ) {
    if (!dEvent::isOngoing(EVENT_HARVEST_FESTIVAL)) {
        *code = 0x1F;
    } else {
        *code = 0x1E;
    }
    return TRUE;
}

// 800464F8
int dAcNpcNml_c::talk_c::msgHarvestSpot(msgInfo_s *info) {
    static const locFunc l_checks[12] = {
        &talk_c::checkSpotNpcBlock, &talk_c::checkSpotPlayerHouse, &talk_c::checkSpotOtherHouse, &talk_c::checkSpotNpcHouse,
        &talk_c::checkSpotLandmark, &talk_c::checkSpotWater, &talk_c::checkSpotEmptyHouse, &talk_c::checkSpotBuildSite,
        &talk_c::checkSpotBeach, &talk_c::checkSpotGateLine, &talk_c::checkSpotEdge, &talk_c::checkSpotDefault,
    };
    const char *label = getMsgLabel(TALK_HARVEST, 2, 0);
    u16 code = 0;
    dSaveTown_c *town = dSaveData_c::getTown();
    mVec3_c pos = fn_801506F8(town->mHarvestSpot);
    int blockX = 0;
    int blockZ = 0;
    int unitX = 0;
    int unitZ = 0;
    dFdBase_c::posToBlockUnit(&blockX, &blockZ, &unitX, &unitZ, &pos);
    for (int i = 0; i < 12; i++) {
        if (l_checks[i] && (this->*l_checks[i])(&code, blockX, blockZ, unitX, unitZ)) {
            break;
        }
    }
    if (code == 0) {
        code = 0x1F;
    }
    setLooksMsg(info, label, code);
    return TRUE;
}

// 8004660C
void dAcNpcNml_c::talk_c::selHarvestNo() {
    setProcSet(&l_talkEntrySets[TALK_FREE]);
    startMsg();
}
