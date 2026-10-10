// A villager's free conversation ("free talk"): the topic groups FreeA..FreeI, the approach item
// offers ApC, and the Etc lines. .text 80039B38..8003F354. See include/game/game/d_npc_talk_free.hpp.
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_talk_free.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_friend.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_wifi.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// ---- label tables (read by talk_c::getMsgLabel(TALK_FREE, group, idx)) ----
const char *l_freeALabels[7] = {
    "FreeA_Clothes", "FreeA_Always", "FreeA_Dress", "FreeA_Friend", "FreeA_Memory", "FreeA_Rumor", "FreeA_Jinx",
};
const char *l_freeASeasonLabels[7] = {
    "FreeA_0229", "FreeA_0401", "FreeA_Weather", "FreeA_MoonJPN", "FreeA_MoonKOR", "FreeA_MoonUSA", "FreeA_MoonEUR",
};
const char *l_freeBLabels[8] = {
    "FreeB_Bug", "FreeB_Fassion", "FreeB_Fish", "FreeB_Fossil",
    "FreeB_Gardening", "FreeB_Interior", "FreeB_Party", "FreeB_Hint",
};
const char *l_apcLabels[4] = {"ApC_Present", "ApC_Sell", "ApC_Trade", "ApC_Want"};
const char *l_freeDLabels[2] = {"FreeD_Moving2", NULL};
const char *l_freeELabels[2] = {"FreeE_Event", "FreeE_Snpc"};
const char *l_freeFLabels[4] = {"FreeF_Building", "FreeF_House1", "FreeF_House2", "FreeF_Inside"};
const char *l_freeGLabels[3] = {"FreeG_Host", "FreeG_Visitor", "FreeG_JinxV"};
const char *l_freeHLabels[6] = {
    "FreeH_Harvest", "FreeH_Halloween", "FreeH_Countdown", "FreeH_Fmarket", "FreeH_Xmas", "FreeH_Easter",
};
const char *l_freeILabels[2] = {"FreeI_Tunekichi", "FreeI_Tunekichi"};

// 80039B38
BOOL dAcNpcNml_c::talk_c::isLeapDay() {
    dTime_c *now = dTime_c::getCurrent();
    if (now->month == 1 && now->mday == 29) {
        return TRUE;
    }
    return FALSE;
}

// 80039B7C
BOOL dAcNpcNml_c::talk_c::isAprilFools() {
    return dEvent::isOngoing(EVENT_APRIL_FOOLS_DAY) != FALSE;
}

// 80039BAC
BOOL dAcNpcNml_c::talk_c::isWeatherTopic() {
    dTime_c *now = dTime_c::getCurrent();
    BOOL clear = !fn_801C9C08(now);
    if (clear && (now->hour >= 6 || now->hour < 4)) {
        return TRUE;
    }
    if (getWeatherPhaseB() == PLAY_WEATHER_PHASE_SOON) {
        return TRUE;
    }
    return (u32)getWeatherPhaseA() <= PLAY_WEATHER_PHASE_SOON;
}

// 80039C34
BOOL dAcNpcNml_c::talk_c::isMoonJPN() {
    return dEvent::isActive(EVENT_JP_AUTUMN_MOON) != FALSE;
}

// 80039C64
BOOL dAcNpcNml_c::talk_c::isMoonKOR() {
    return dEvent::isActive(EVENT_KR_DAEBOREUM) != FALSE;
}

// 80039C94
BOOL dAcNpcNml_c::talk_c::isMoonUSA() {
    return dEvent::isActive(EVENT_NA_AUTUMN_MOON) != FALSE;
}

// 80039CC4
BOOL dAcNpcNml_c::talk_c::isMoonEUR() {
    return dEvent::isActive(EVENT_EU_AUTUMN_MOON) != FALSE;
}

// 80039CF4
int dAcNpcNml_c::talk_c::getSpecialDay() {
    static const condFunc l_specialDays[7] = {
        &talk_c::isLeapDay, &talk_c::isAprilFools, &talk_c::isWeatherTopic, &talk_c::isMoonJPN,
        &talk_c::isMoonKOR, &talk_c::isMoonUSA, &talk_c::isMoonEUR,
    };
    for (int i = 0; i < 7; i++) {
        if (l_specialDays[i] && (this->*l_specialDays[i])()) {
            return i;
        }
    }
    return 7;
}

// 80039D7C
BOOL dAcNpcNml_c::talk_c::isEventDay(BOOL withMoon) {
    static const int l_events[30] = {
        EVENT_TOY_DAY, EVENT_HARVEST_FESTIVAL, EVENT_HALLOWEEN, EVENT_COUNTDOWN, EVENT_NEW_YEARS_DAY,
        EVENT_VALENTINES_DAY, EVENT_PLAYER_BIRTHDAY_0, EVENT_PLAYER_BIRTHDAY_1, EVENT_PLAYER_BIRTHDAY_2,
        EVENT_PLAYER_BIRTHDAY_3, EVENT_FLEA_MARKET, EVENT_FISHING_TOURNEY, EVENT_BUG_OFF, EVENT_FIREWORKS,
        EVENT_FESTIVALE, EVENT_BUNNY_DAY, EVENT_JP_SETSUBUN, EVENT_JP_GIRLS_DAY, EVENT_JP_CHILDRENS_DAY,
        EVENT_JP_TANABATA, EVENT_NA_GROUNDHOG_DAY, EVENT_NA_NATURE_DAY, EVENT_NA_LABOR_DAY,
        EVENT_NA_EXPLORERS_DAY, EVENT_EU_MIDSUMMERS_DAY, EVENT_EU_NAUGHTY_OR_NICE_DAY,
        EVENT_EU_MIDWINTERS_DAY, EVENT_KR_LUNAR_NEW_YEAR, EVENT_KR_ARBOR_DAY, EVENT_KR_TEACHERS_DAY,
    };
    static const int l_moonEvents[5] = {
        EVENT_JP_AUTUMN_MOON, EVENT_KR_DAEBOREUM, EVENT_NA_AUTUMN_MOON, EVENT_EU_AUTUMN_MOON, EVENT_APRIL_FOOLS_DAY,
    };
    for (u32 i = 0; i < 30; i++) {
        if (dEvent::isActive((dQuestEvent_e)l_events[i])) {
            return TRUE;
        }
    }
    if (withMoon) {
        for (u32 i = 0; i < 5; i++) {
            if (dEvent::isActive((dQuestEvent_e)l_moonEvents[i])) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static inline BOOL isVisitor(dPrivateData_c *player) {
    BOOL visitor = TRUE;
    bool online = (fn_800DCEDC() && fn_800DCF30() > 1) || player == NULL;
    if (!online && player->mPID.isFromTown()) {
        visitor = FALSE;
    }
    return visitor;
}
// 80039E38
const u8 *dAcNpcNml_c::talk_c::getFreeWeights() {
    // A, A2, H, B, ApC, D, E, F, G, I
    // (the first three rows, the only ones with an ApC weight, are unused)
    static const u8 l_weights0[10] = {25, 0, 0, 25, 10, 0, 15, 20, 0, 5};
    static const u8 l_weights1[10] = {15, 30, 0, 15, 5, 0, 10, 20, 0, 5};
    static const u8 l_weights2[10] = {15, 0, 50, 10, 5, 0, 5, 10, 0, 5};
    static const u8 l_town[10] = {25, 0, 0, 25, 0, 0, 20, 25, 0, 5};
    static const u8 l_townSpecial[10] = {15, 35, 0, 15, 0, 0, 15, 15, 0, 5};
    static const u8 l_townEvent[10] = {20, 0, 50, 10, 0, 0, 5, 10, 0, 5};
    static const u8 l_moving[10] = {15, 0, 0, 15, 0, 50, 10, 10, 0, 0};
    static const u8 l_visitor[10] = {25, 0, 0, 20, 0, 0, 0, 20, 35, 0};
    static const u8 l_visitorSpecial[10] = {20, 20, 0, 15, 0, 0, 0, 15, 30, 0};
    static const u8 l_visitorEvent[10] = {10, 0, 50, 10, 0, 0, 0, 5, 25, 0};
    BOOL special = (u32)getSpecialDay() < 7 ? TRUE : FALSE;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL visitor = isVisitor(player);
    dAnimalBlock_c *block = &dSaveData_c::getRaw()->mAnimals.mTown;
    dAnimal_c *animal = getAnimal();
    BOOL moving = animal != NULL ? block->isMoveOutAnimal(&animal->mID) : FALSE;
    BOOL event = isEventDay(FALSE);
    if (visitor) {
        if (event) {
            return l_visitorEvent;
        }
        return special ? l_visitorSpecial : l_visitor;
    }
    if (moving) {
        return l_moving;
    }
    if (event) {
        return l_townEvent;
    }
    return special ? l_townSpecial : l_town;
}

// 80039F74
int dAcNpcNml_c::talk_c::pickFriendTown() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dFriendList_c *friends = player != NULL ? &player->mFriends : NULL;
    const dSaveTownList_c *list;
    const dFriend_c *pal;
    int town = -1;
    if (friends != NULL) {
        list = &dSaveData_c::getTown()->mTownList;
        u32 count = 0;
        for (u32 i = 0; i < SAVE_TOWN_LIST_NUM; i++) {
            const dSaveTownListEntry_c *const otherValue = list->getTown(i);
            const dSaveTownListEntry_c *other = otherValue;
            if (other != NULL && other->mLand.isValid()) {
                for (int j = 0; j < FRIEND_NUM; j++) {
                    const dFriend_c *const friendValue = friends->getFriend(j);
                    pal = friendValue;
                    if (pal != NULL && pal->mPID.isValid()) {
                        bool same = pal->mPID.land == other->mLand;
                        if (same) {
                            f32 chance = 100.0f / (count + 1);
                            if (cM::rndF(100.0f) <= chance) {
                                town = i;
                            }
                            count++;
                            break;
                        }
                    }
                }
            }
        }
    }
    return town;
}

// 8003A0FC
dAnimalMemory_c *dAcNpcNml_c::talk_c::pickOtherMemory() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
    const dPersonalID_c *other;
    dAnimal_c *animal = getAnimal();
    dAnimalMemory_c *found = NULL;
    if (pid != NULL && pid->isValid() && animal != NULL) {
        u32 count = 0;
        for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
            dAnimalMemory_c *mem = animal->getMemory2(i);
            if (mem != NULL && mem->mPlayer.isValid()) {
                other = &mem->mPlayer;
                if (*other != *pid) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        found = mem;
                    }
                    count++;
                }
            }
        }
    }
    return found;
}

// 8003A284
const u8 *dAcNpcNml_c::talk_c::getGeneralWeights() {
    // Clothes, Always, Dress, Friend, Memory, Rumor, Jinx
    static const u8 l_wifiFriendRumorMem[8] = {15, 25, 15, 15, 15, 10, 5, 0};
    static const u8 l_wifiFriendRumor[8] = {0, 30, 20, 20, 15, 10, 5, 0};
    static const u8 l_wifiFriendMem[8] = {10, 25, 20, 20, 20, 0, 5, 0};
    static const u8 l_wifiFriend[8] = {15, 25, 20, 20, 10, 0, 10, 0};
    static const u8 l_wifiRumorMem[8] = {15, 30, 20, 20, 10, 5, 0, 0};
    static const u8 l_wifiRumor[8] = {15, 30, 20, 20, 5, 10, 0, 0};
    static const u8 l_wifiMem[8] = {15, 30, 20, 20, 15, 0, 0, 0};
    static const u8 l_wifi[8] = {15, 30, 25, 25, 5, 0, 0, 0};
    static const u8 l_offlineMem[8] = {10, 25, 25, 20, 20, 0, 0, 0};
    static const u8 l_offline[8] = {10, 30, 30, 20, 10, 0, 0, 0};
    static const u8 l_visitor[8] = {15, 35, 35, 15, 0, 0, 0, 0};
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    bool visitor = (fn_800DCEDC() && fn_800DCF30() > 1) || player == NULL || !player->mPID.isFromTown();
    if (visitor) {
        return l_visitor;
    }
    bool wifi = fn_80177D24();
    bool friendTown = pickFriendTown() != -1;
    bool memory = pickOtherMemory() != NULL;
    bool rumor = dSaveData_c::getRaw()->getRandomOtherTown() != -1;
    if (wifi) {
        if (friendTown) {
            if (rumor) {
                return memory ? l_wifiFriendRumorMem : l_wifiFriendRumor;
            }
            return memory ? l_wifiFriendMem : l_wifiFriend;
        }
        if (rumor) {
            return memory ? l_wifiRumorMem : l_wifiRumor;
        }
        return memory ? l_wifiMem : l_wifi;
    }
    return memory ? l_offlineMem : l_offline;
}

// The villager's memory of the current player remembers a shirt (FreeA_Clothes).
static inline bool hasLastCloth(const dAnimalMemory_c *mem) {
    return mem != NULL && mem->mLastClothes.isValid();
}

// 8003A3DC
int dAcNpcNml_c::talk_c::msgFreeClothes(msgInfo_s *info) {
    dItem::Item item;
    u16 code = 0;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = getAnimal();
    if (player != NULL && animal != NULL && mpMemory != NULL && !mpMemory->mFlags.mClothesTalked) {
        u8 looks = animal->mID.getLooks(1);
        u32 count = 0;
        const dItem::Item *last = hasLastCloth(mpMemory) ? &mpMemory->mLastClothes : NULL;
        const dItem::Item *shirt = &player->mEquipment.mShirt;
        if (shirt->isValid() && animal->mCloth.isNotSame(*shirt)) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*shirt);
            if (bitm != NULL && bitm->getKind() == dItem::KIND_CLOTH) {
                u16 msg = getNpcMsgBullfest(*shirt, looks);
                if (msg != 0) {
                    count = 1;
                    f32 chance = 100.0f / count;
                    if (cM::rndF(100.0f) <= chance) {
                        if (last != NULL && last->isSame(*shirt)) {
                            code = 4;
                        } else {
                            code = msg;
                        }
                        item = *shirt;
                    }
                }
            }
        }
        const dItem::Item *hat = &player->mEquipment.mHat;
        if (hat->isValid()) {
            u16 msg = getNpcMsgBullfest(*hat, looks);
            if (msg != 0) {
                count++;
                f32 chance = 100.0f / count;
                if (cM::rndF(100.0f) <= chance) {
                    if (last != NULL && last->isSame(*hat)) {
                        int bone = hat->getHideBone();
                        bool hides = bone == 7 || (u32)(bone - 2) <= 4;
                        if (hides) {
                            code = 1;
                        } else {
                            code = 2;
                        }
                    } else {
                        code = msg;
                    }
                    item = *hat;
                }
            }
        }
        const dItem::Item *acc = &player->mEquipment.mAcc;
        if (acc->isValid()) {
            u16 msg = getNpcMsgBullfest(*acc, looks);
            if (msg != 0) {
                count++;
                f32 chance = 100.0f / count;
                if (cM::rndF(100.0f) <= chance) {
                    if (last != NULL && last->isSame(*acc)) {
                        code = 3;
                    } else {
                        code = msg;
                    }
                    item = *acc;
                }
            }
        }
    }
    mItem0 = dItem::Item();
    if (code != 0 && item.isValid()) {
        const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_CLOTHES);
        if (label != NULL) {
            mItem0 = item;
            setItemName(&item, 0);
            u32 fashion = item.getFashion();
            dItem::nameFashion_c fashionName;
            setFashionName(&fashionName, fashion);
            getController()->setWord(1, &fashionName);
            int look = item.getStyle();
            dItem::nameLook_c lookName;
            setLookName(&lookName, look);
            getController()->setWord(2, &lookName);
            setLooksMsg(info, label, code);
            setStepProc(&talk_c::stepFreeClothes);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_CLOTHES);
            return TRUE;
        }
    }
    return FALSE;
}

// Word idx = the unit word of a player gender for the speaker's looks (fn_800F3F38), cleared when there
// is none. The u16 parameter makes the test a clrlwi. of the argument.
static inline void setUnitMsg(talk_c *talk, int idx, u16 unit) {
    if (unit == 0) {
        talk->clearWord(idx);
    } else {
        talk->getController()->fn_801A5874(idx, unit, "sys_STRING/STR_Unit");
    }
}

// The town list entry of mJinxTown (NULL: this town); the same inline helper as d_a_npc_nml.
inline dSaveTownListEntry_c *dAcNpcNml_c::talk_c::getJinxTown() {
    if ((u32)mJinxTown < SAVE_TOWN_LIST_NUM) {
        dSaveTown_c *town = dSaveData_c::getTown();
        return town->mTownList.getTown(mJinxTown);
    }
    return NULL;
}

// 8003A814
int dAcNpcNml_c::talk_c::msgFreeAlways(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_ALWAYS);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_ALWAYS);
        return TRUE;
    }
    return FALSE;
}

// 8003A894
void dAcNpcNml_c::talk_c::setDesignWords(const dPersonalID_c *creator, const wchar_t *designName, dAnimal_c *animal) {
    setPersonalName(creator, 0);
    u8 gender = creator->player.mGender;
    setUnitMsg(this, 1, fn_800F3F38(gender, animal->mID.getLooks(1)));
    setLandName(&creator->land, 2);
    dString::Word_c name;
    name.set(designName, 0);
    getController()->setWord(3, &name);
    if (creator->isValid()) {
        mMsgPersonal.clear();
        mMsgPersonal.copy(creator);
        setMemoryText(designName);
    }
}

// 8003A998
int dAcNpcNml_c::talk_c::msgFreeDress(msgInfo_s *info) {
    dAnimal_c *animal;
    const char *label;
    animal = getAnimal();
    if (animal != NULL) {
        label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_DRESS);
        if (label != NULL) {
            const dPersonalID_c *creator = animal->mClothDesign.getCreator();
            const dLandID_c *townLand = dSaveData_c::getTownLand();
            int code = 1;
            if (animal->isWearingOrgCloth() && creator->isValid()) {
                dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
                const dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
                if (pid != NULL && *pid == *creator) {
                    code = 2;
                } else {
                    BOOL same = creator->land == *townLand;
                    code = !same + 3;
                }
                setDesignWords(creator, animal->mClothDesign.mName, animal);
            }
            setLooksMsg(info, label, code);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_DRESS);
            return TRUE;
        }
    }
    return FALSE;
}

// 8003AB40
int dAcNpcNml_c::talk_c::msgFreeFriend(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_FRIEND);
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && label != NULL) {
        dAnimalBlock_c *block = &dSaveData_c::getTown()->mAnimals.mTown;
        const dAnmPersonalID_c *exclude[2] = {&animal->mID, NULL};
        dAnimal_c *first = block->pickRandomAnimal(exclude, 1, TRUE);
        if (first != NULL) {
            exclude[1] = &first->mID;
            dAnimal_c *second = block->pickRandomAnimal(exclude, 2, TRUE);
            if (second != NULL) {
                u8 gender1 = first->mID.getGender(1);
                u8 gender2 = second->mID.getGender(1);
                int relation = fn_800F3F34(first, second);
                u16 code;
                if (gender1 == gender2) {
                    if (gender1 == 0) {
                        code = 1;
                    } else {
                        code = 2;
                    }
                } else {
                    code = 3;
                }
                switch (relation) {
                case 2:
                    code += 6;
                    break;
                case 1:
                default:
                    code += 3;
                    break;
                case 0:
                    break;
                }
                setLooksMsg(info, label, code);
                setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_FRIEND);
                if (gender2 == 0) {
                    setAnmPersonalName(&second->mID, 0);
                    setAnmPersonalName(&first->mID, 1);
                } else {
                    setAnmPersonalName(&first->mID, 0);
                    setAnmPersonalName(&second->mID, 1);
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8003AD10
BOOL dAcNpcNml_c::talk_c::isMemLevel0(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (sameTown && mem->getFriendshipLevel() == 0) {
        return TRUE;
    }
    return FALSE;
}

// 8003AD4C
BOOL dAcNpcNml_c::talk_c::isMemLevel2(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (sameTown && mem->getFriendshipLevel() == 2) {
        return TRUE;
    }
    return FALSE;
}

// 8003AD88
BOOL dAcNpcNml_c::talk_c::isMemLevel1(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (sameTown && mem->getFriendshipLevel() == 1) {
        return TRUE;
    }
    return FALSE;
}

// 8003ADC4
BOOL dAcNpcNml_c::talk_c::isMemLetter(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (sameTown && animal->hasLetterFrom(&mem->mPlayer)) {
        return TRUE;
    }
    return FALSE;
}

// 8003AE0C
BOOL dAcNpcNml_c::talk_c::isMemPresent(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (sameTown && mem->mPresent.isValid()) {
        return TRUE;
    }
    return FALSE;
}

// 8003AE30
BOOL dAcNpcNml_c::talk_c::isMemVisitor(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (!sameTown && !mem->mFlags.mSameTown) {
        return TRUE;
    }
    return FALSE;
}

// 8003AE54
BOOL dAcNpcNml_c::talk_c::isMemAwayLetter(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (!sameTown && animal->hasLetterFrom(&mem->mPlayer)) {
        return TRUE;
    }
    return FALSE;
}

// 8003AE9C
BOOL dAcNpcNml_c::talk_c::isMemAwayLevel0(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (!sameTown && mem->mFlags.mSameTown && mem->getFriendshipLevel() == 0) {
        return TRUE;
    }
    return FALSE;
}

// 8003AEE4
BOOL dAcNpcNml_c::talk_c::isMemAwayLevel2(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (!sameTown && mem->mFlags.mSameTown && mem->getFriendshipLevel() == 2) {
        return TRUE;
    }
    return FALSE;
}

// 8003AF2C
BOOL dAcNpcNml_c::talk_c::isMemAwayLevel1(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    if (!sameTown && mem->mFlags.mSameTown && mem->getFriendshipLevel() == 1) {
        return TRUE;
    }
    return FALSE;
}

// 8003AF74
BOOL dAcNpcNml_c::talk_c::isFromOtherTown(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown) {
    const dLandID_c *prevLand = &animal->mPrevLand;
    const dLandID_c *townLand = &dSaveData_c::getRaw()->mLandID;
    if (prevLand->isValid() && *prevLand != *townLand) {
        return TRUE;
    }
    return FALSE;
}

// 8003B018
void dAcNpcNml_c::talk_c::setMemoryWords(const dPersonalID_c *pid, const dLandID_c *land, u32 impress, const dItem::Item *item, dAnimal_c *animal) {
    int unit = 0;
    if (pid != NULL && pid->isValid()) {
        setPersonalName(pid, 0);
        if (animal != NULL) {
            u8 gender = pid->player.mGender;
            unit = fn_800F3F38(gender, animal->mID.getLooks(1));
        }
        mMsgPersonal.copy(pid);
    }
    setUnitMsg(this, 1, unit);
    if (land->isValid()) {
        setLandName(land, 2);
        mMsgLand.copy(land);
    }
    u16 level = impress < 49 ? impress + 1 : 100;
    getController()->fn_801A5874(3, level, "sys_STRING/STR_Impress");
    if (level != 100) {
        mImpression = impress;
    }
    if (item->isValid()) {
        setItemName(item, 4);
        mItem4 = *item;
    }
}

// 8003B174
int dAcNpcNml_c::talk_c::msgFreeMemory(msgInfo_s *info) {
    typedef BOOL (*memFilter)(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);
    static const memFilter l_filters[12] = {
        NULL,
        &talk_c::isMemLevel0, &talk_c::isMemLevel2, &talk_c::isMemLevel1, &talk_c::isMemLetter,
        &talk_c::isMemPresent, &talk_c::isMemVisitor, &talk_c::isMemAwayLetter, &talk_c::isMemAwayLevel0,
        &talk_c::isMemAwayLevel2, &talk_c::isMemAwayLevel1, &talk_c::isFromOtherTown,
    };
    const dLandID_c *memLand;
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_MEMORY);
    dAnimal_c *animal = getAnimal();
    if (label != NULL && animal != NULL) {
        u16 code = 1;
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        const dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
        if (pid != NULL) {
            const dPersonalID_c *exclude = pid;
            dAnimalMemory_c *mem = animal->getRandomMemory(&exclude, 1, TRUE);
            if (mem != NULL) {
                const dLandID_c *townLand = &dSaveData_c::getRaw()->mLandID;
                memLand = mem->getPlayerLand();
                BOOL sameTown = *townLand == *memLand;
                u32 count = 0;
                for (int i = 0; i < 12; i++) {
                    if (l_filters[i] != NULL && l_filters[i](mem, animal, sameTown)) {
                        f32 chance = 100.0f / (count + 1);
                        if (cM::rndF(100.0f) <= chance) {
                            code = i + 1;
                        }
                        count++;
                    }
                }
                if (code != 1) {
                    if (code == 12) {
                        dItem::Item none;
                        setMemoryWords(NULL, &animal->mPrevLand, 50, &none, animal);
                    } else {
                        setMemoryWords(&mem->mPlayer, memLand, mem->mImpression, &mem->mPresent, animal);
                    }
                }
            }
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_MEMORY);
        return TRUE;
    }
    return FALSE;
}

// 8003B3B4
int dAcNpcNml_c::talk_c::msgFreeRumor(msgInfo_s *info) {
    int townIdx = dSaveData_c::getRaw()->getRandomOtherTown();
    dSaveOtherTown_c *other;
    u8 stage;
    const dSaveBuildingList_c *list;
    u16 code = 0;
    const dLandID_c *land = NULL;
    const dPersonalID_c *player = NULL;
    dAnmPersonalID_c *villager = NULL;
    u32 impress = 49;
    if ((u32)townIdx < 4) {
        other = &dSaveData_c::getRaw()->mOtherTowns[townIdx];
        if (other->mLand.isValid()) {
            land = &other->mLand;
            u32 mask = 0;
            int num = 0;
            if (other->mPlayer.isValid() && ((dAnmPersonalID_c *)other->mVillager)->isValid() && other->mImpression < 49) {
                impress = other->mImpression;
                mask |= 1;
                player = &other->mPlayer;
                villager = (dAnmPersonalID_c *)other->mVillager;
                num = 1;
            }
            stage = other->mShopStage;
            u8 townStage = (u8)dSaveData_c::getTown()->mShops.mShop.mStage;
            if (townStage != stage) {
                switch (stage) {
                case SHOP_STAGE_NOOK_N_GO:
                    mask |= 2;
                    num++;
                    break;
                case SHOP_STAGE_NOOKWAY:
                    mask |= 4;
                    num++;
                    break;
                case SHOP_STAGE_NOOKINGTONS:
                    mask |= 8;
                    num++;
                    break;
                }
            }
            if (other->mHasBridge) {
                const dSaveMainField_c *field = &dSaveData_c::getRaw()->mMainField;
                BOOL built = field->mBridgeBlockX == -1 && field->mBridgeBlockZ == -1;
                mask = built ? mask | 0x20 : mask | 0x10;
                num++;
            }
            int x = 0;
            int z = 0;
            list = &dSaveData_c::getRaw()->mBuilding.mList;
            if (other->mHasFountain) {
                dItem::Item fountain((u16)BUILDING_FOUNTAIN);
                bool built = list->getPos(&x, &z, &fountain, 1);
                mask = built ? mask | 0x80 : mask | 0x40;
                num++;
            }
            if (other->mHasWindmill) {
                dItem::Item windmill((u16)BUILDING_WINDMILL);
                bool built = list->getPos(&x, &z, &windmill, 1);
                mask = built ? mask | 0x200 : mask | 0x100;
                num++;
            }
            if (other->mHasLighthouse) {
                dItem::Item lighthouse((u16)BUILDING_LIGHTHOUSE);
                bool built = list->getPos(&x, &z, &lighthouse, 1);
                mask = built ? mask | 0x800 : mask | 0x400;
                num++;
            }
            if (other->_103 == 0) {
                switch (fgMngProc_assessLiveTown()) {
                case dFdAssess_c::TOWN_RANK_BAD:
                    mask |= 0x1000;
                    num++;
                    break;
                case dFdAssess_c::TOWN_RANK_POOR:
                    mask |= 0x2000;
                    num++;
                    break;
                case dFdAssess_c::TOWN_RANK_FAIR:
                    mask |= 0x4000;
                    num++;
                    break;
                case dFdAssess_c::TOWN_RANK_GOOD:
                    mask |= 0x8000;
                    num++;
                    break;
                case dFdAssess_c::TOWN_RANK_PERFECT:
                    mask |= 0x10000;
                    num++;
                    break;
                }
            }
            u32 bit = pickRandomBit(mask, num, 17);
            if (bit < 17) {
                code = bit + 1;
            }
        }
    }
    if (code != 0) {
        const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_RUMOR);
        if (label != NULL) {
            if (land != NULL) {
                setLandName(land, 0);
            }
            dAnimal_c *animal = getAnimal();
            if (player != NULL) {
                setPersonalName(player, 1);
                if (animal != NULL) {
                    u8 gender = player->player.mGender;
                    setUnitMsg(this, 2, fn_800F3F38(gender, animal->mID.getLooks(1)));
                }
            }
            if (impress < 49) {
                getController()->fn_801A5874(3, impress + 1, "sys_STRING/STR_Impress");
            }
            if (villager != NULL) {
                setAnmPersonalName(villager, 4);
                if (animal != NULL) {
                    setUnitMsg(this, 5, fn_800F3F38(villager->getGender(1), animal->mID.getLooks(1)));
                }
            }
            setLooksMsg(info, label, code);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_RUMOR);
            mRumorTown = townIdx;
            return TRUE;
        }
    }
    return FALSE;
}

// 8003B858
void dAcNpcNml_c::talk_c::setJinxWords(const dSaveJinx_c *jinx, const dLandID_c *land) {
    int condition = jinx->mCondition;
    u8 value = jinx->mValue;
    switch (condition) {
    case JINX_COND_CATCH_BUG: {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, value, FALSE);
        setItemName(&item, 0);
        break;
    }
    case JINX_COND_CATCH_FISH: {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, value, FALSE);
        setItemName(&item, 0);
        break;
    }
    }
    if (land != NULL && land->isValid()) {
        setLandName(land, 1);
    }
    switch (condition) {
    case JINX_COND_PULL_WEEDS:
    case JINX_COND_PLANT_TREES:
    case JINX_COND_PLANT_FLOWERS:
        formatNumber(value, 0, 12, dScript::NUM_FORMAT_REGION);
        break;
    }
}

// 8003B960
int dAcNpcNml_c::talk_c::msgFreeJinx(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_JINX);
    int townIdx = pickFriendTown();
    dSaveTownListEntry_c *town =
        (u32)townIdx < SAVE_TOWN_LIST_NUM ? dSaveData_c::getTown()->mTownList.getTown(townIdx) : NULL;
    const dSaveJinx_c *jinx = town != NULL ? &town->mJinx : NULL;
    if (label != NULL && jinx != NULL) {
        setLooksMsg(info, label, (int)cM::rndF(3.0f) + 1);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A, FREE_A_JINX);
        mJinxTown = townIdx;
        setStepProc(&talk_c::stepJinx);
        setJinxWords(jinx, &town->mLand);
        return TRUE;
    }
    return FALSE;
}

// 8003BA84
int dAcNpcNml_c::talk_c::msgFreeGeneral(msgInfo_s *info) {
    static const msgFunc l_freeATopics[7] = {
        &talk_c::msgFreeClothes, &talk_c::msgFreeAlways, &talk_c::msgFreeDress, &talk_c::msgFreeFriend,
        &talk_c::msgFreeMemory, &talk_c::msgFreeRumor, &talk_c::msgFreeJinx,
    };
    const u8 *weights = getGeneralWeights();
    int idx = 1;
    if (weights != NULL) {
        int rnd = cM::rndF(100.0f);
        for (int i = 0; i < 7; i++) {
            if (rnd < weights[i]) {
                idx = i;
                break;
            }
            rnd -= weights[i];
        }
    }
    if (!(this->*l_freeATopics[idx])(info)) {
        msgFreeAlways(info);
    }
    return TRUE;
}

// 8003BBC8
int dAcNpcNml_c::talk_c::msgFree0229(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_0229);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_0229);
        return TRUE;
    }
    return FALSE;
}

// 8003BC48
int dAcNpcNml_c::talk_c::msgFree0401(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_0401);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_0229); // (sic) recorded as the 0229 topic
        const dLandID_c *townLand;
        const dLandID_c *land;
        const dLandID_c *found = NULL;
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            townLand = &dSaveData_c::getRaw()->mLandID;
            u32 count = 0;
            for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
                dAnimalMemory_c *mem = animal->getMemory2(i);
                if (mem != NULL && mem->mPlayer.isValid()) {
                    land = &mem->mPlayer.land;
                    if (land->isValid()) {
                        bool same = *land == *townLand;
                        if (!same) {
                            f32 chance = 100.0f / (count + 1);
                            if (cM::rndF(100.0f) <= chance) {
                                found = land;
                            }
                            count++;
                        }
                    }
                }
            }
        }
        if (found != NULL) {
            setLandName(found, 0);
        } else {
            getController()->fn_801A5874(0, 101, "sys_STRING/STR_Unit");
        }
        return TRUE;
    }
    return FALSE;
}

// 8003BE24
int dAcNpcNml_c::talk_c::msgFreeWeather(msgInfo_s *info) {
    int code = 0;
    dTime_c *now = dTime_c::getCurrent();
    if (cM::rndF(100.0f) < 50.0f) {
        BOOL clear = !fn_801C9C08(now);
        if (clear) {
            int hour = now->hour;
            if (hour >= 6 && hour < 12) {
                code = 1;
            } else if (hour >= 12 && hour < 19) {
                code = 2;
            } else if (hour >= 19 || hour < 4) {
                code = 3;
            }
        } else if (getWeatherPhaseB() == PLAY_WEATHER_PHASE_SOON) {
            code = 4;
        } else {
            int phase = getWeatherPhaseA();
            if (phase == PLAY_WEATHER_PHASE_PAST) {
                code = 5;
            } else if (phase == PLAY_WEATHER_PHASE_SOON) {
                code = 6;
            }
        }
    }
    if (code != 0) {
        const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_WEATHER);
        if (label != NULL) {
            setLooksMsg(info, label, code);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_WEATHER);
            return TRUE;
        }
    }
    return FALSE;
}

// 8003BF70
int dAcNpcNml_c::talk_c::msgFreeMoonJPN(msgInfo_s *info) {
    dTime_c *now = dTime_c::getCurrent();
    int code;
    if (now->hour >= 6 && now->hour < 18) {
        code = 1;
    } else {
        code = 2;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_MOON_JPN);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_MOON_JPN);
        return TRUE;
    }
    return FALSE;
}

// 8003C01C
int dAcNpcNml_c::talk_c::msgFreeMoonKOR(msgInfo_s *info) {
    dTime_c *now = dTime_c::getCurrent();
    int code;
    if (now->hour >= 6 && now->hour < 18) {
        code = 1;
    } else {
        code = 2;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_MOON_KOR);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_MOON_KOR);
        return TRUE;
    }
    return FALSE;
}

// 8003C0C8
int dAcNpcNml_c::talk_c::msgFreeMoonUSA(msgInfo_s *info) {
    dTime_c *now = dTime_c::getCurrent();
    int code;
    if (now->hour >= 6 && now->hour < 18) {
        code = 1;
    } else {
        code = 2;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_MOON_USA);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_MOON_USA);
        return TRUE;
    }
    return FALSE;
}

// 8003C174
int dAcNpcNml_c::talk_c::msgFreeMoonEUR(msgInfo_s *info) {
    dTime_c *now = dTime_c::getCurrent();
    int code;
    if (now->hour >= 6 && now->hour < 18) {
        code = 1;
    } else {
        code = 2;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_A2, FREE_A2_MOON_EUR);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_A2, FREE_A2_MOON_EUR);
        return TRUE;
    }
    return FALSE;
}

// 8003C220
int dAcNpcNml_c::talk_c::msgFreeSpecialDay(msgInfo_s *info) {
    static const msgFunc l_specialTopics[7] = {
        &talk_c::msgFree0229, &talk_c::msgFree0401, &talk_c::msgFreeWeather, &talk_c::msgFreeMoonJPN,
        &talk_c::msgFreeMoonKOR, &talk_c::msgFreeMoonUSA, &talk_c::msgFreeMoonEUR,
    };
    int idx = getSpecialDay();
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        idx = 7;
    }
    if ((u32)idx < 7 && l_specialTopics[idx]) {
        return (this->*l_specialTopics[idx])(info);
    }
    return FALSE;
}

// 8003C2BC
int dAcNpcNml_c::talk_c::msgFreeBug(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_BUG);
    if (label != NULL) {
        int code = 2;
        if (isHoldingTool(HMN_TOOL_NET)) {
            code = 1;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_BUG);
        const dTime_c &now = *dTime_c::getCurrent();
        dItem::Item first(dItem::ITEM_IDX_ANT);
        dItem::Item second(dItem::ITEM_IDX_FLY);
        int firstIdx = dInsectInfo::getRandomByRarity(now, 4, 0, FALSE);
        if ((u32)firstIdx < 64) {
            first.setFromIndex(dItem::ITEM_IDX_COMMON_BUTTERFLY,firstIdx, FALSE);
        }
        int secondIdx = dInsectInfo::getRandomByRarity(now, 4, 0, FALSE);
        for (int tries = 3; secondIdx == firstIdx && tries != 0; tries--) {
            secondIdx = dInsectInfo::getRandomByRarity(now, 4, 0, FALSE);
        }
        if (secondIdx != firstIdx && (u32)secondIdx < 64) {
            second.setFromIndex(dItem::ITEM_IDX_COMMON_BUTTERFLY,secondIdx, FALSE);
        }
        setItemName(&first, 0);
        setItemName(&second, 1);
        mItem4 = first;
        mItem5 = second;
        return TRUE;
    }
    return FALSE;
}

// 8003C458
int dAcNpcNml_c::talk_c::msgFreeFashion(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_FASHION);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_FASHION);
        return TRUE;
    }
    return FALSE;
}

// 8003C4D8
int dAcNpcNml_c::talk_c::msgFreeFish(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_FISH);
    if (label != NULL) {
        int code = 2;
        if (isHoldingTool(HMN_TOOL_ROD)) {
            code = 1;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_FISH);
        const dTime_c &now = *dTime_c::getCurrent();
        dItem::Item first(dItem::ITEM_IDX_CRUCIAN_CARP);
        dItem::Item second(dItem::ITEM_IDX_SEA_BASS);
        int firstIdx = dFishInfo::getRandomByRarity(now, 4, 0, FALSE);
        if ((u32)firstIdx < 64) {
            first.setFromIndex(dItem::ITEM_IDX_BITTERLING,firstIdx, FALSE);
        }
        int secondIdx = dFishInfo::getRandomByRarity(now, 4, 0, FALSE);
        for (int tries = 3; secondIdx == firstIdx && tries != 0; tries--) {
            secondIdx = dFishInfo::getRandomByRarity(now, 4, 0, FALSE);
        }
        if (secondIdx != firstIdx && (u32)secondIdx < 64) {
            second.setFromIndex(dItem::ITEM_IDX_BITTERLING,secondIdx, FALSE);
        }
        setItemName(&first, 0);
        setItemName(&second, 1);
        mItem4 = first;
        mItem5 = second;
        return TRUE;
    }
    return FALSE;
}

// 8003C674
int dAcNpcNml_c::talk_c::msgFreeFossil(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_FOSSIL);
    if (label != NULL) {
        int code = 2;
        if (isHoldingTool(HMN_TOOL_SCOOP)) {
            code = 1;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_FOSSIL);
        dItem::Item item(dItem::ITEM_IDX_AMBER);
        dItem::Item fossil = dItem::seeker_c::get()->getRandomFossil(0);
        if (fossil.isValid()) {
            item = fossil;
        }
        setItemName(&item, 0);
        mItem4 = item;
        return TRUE;
    }
    return FALSE;
}

// 8003C770
int dAcNpcNml_c::talk_c::msgFreeGardening(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_GARDENING);
    if (label != NULL) {
        int code = 2;
        if (isHoldingTool(HMN_TOOL_WATERING)) {
            code = 1;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_GARDENING);
        return TRUE;
    }
    return FALSE;
}

// 8003C820
int dAcNpcNml_c::talk_c::msgFreeInterior(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_INTERIOR);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_INTERIOR);
        return TRUE;
    }
    return FALSE;
}

// 8003C8A0
int dAcNpcNml_c::talk_c::msgFreeParty(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_PARTY);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_PARTY);
        return TRUE;
    }
    return FALSE;
}

// 8003C920
int dAcNpcNml_c::talk_c::msgFreeHint(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_B, FREE_B_HINT);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_B, FREE_B_HINT);
        return TRUE;
    }
    return FALSE;
}

// 8003C9A0
int dAcNpcNml_c::talk_c::msgFreeHobby(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    int hobby = animal != NULL ? animal->mQuest.mWish.mKind : 7; // the wish kind, 7: none
    f32 rnd = cM::rndF(100.0f);
    int ret = FALSE;
    if (hobby != 7 && rnd < 70.0f) {
        switch (hobby) {
        case 0:
            ret = msgFreeBug(info);
            break;
        case 1:
            ret = msgFreeFish(info);
            break;
        case 2:
            ret = msgFreeFossil(info);
            break;
        case 3:
            ret = msgFreeFashion(info);
            break;
        case 4:
            ret = msgFreeInterior(info);
            break;
        case 5:
            ret = msgFreeParty(info);
            break;
        case 6:
            ret = msgFreeGardening(info);
            break;
        }
    }
    if (!ret) {
        msgFreeHint(info);
    }
    return TRUE;
}

// 8003CAB8
int dAcNpcNml_c::talk_c::msgApcPresent(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_APC, APC_PRESENT);
    if (label != NULL) {
        setHookProc(&talk_c::endApPresent);
        setLooksMsg(info, label, 0);
        return TRUE;
    }
    return FALSE;
}

// 8003CB58
int dAcNpcNml_c::talk_c::msgApcSell(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_APC, APC_SELL);
    if (label != NULL) {
        setHookProc(&talk_c::endApSell);
        setLooksMsg(info, label, 0);
        return TRUE;
    }
    return FALSE;
}

// 8003CBF8
int dAcNpcNml_c::talk_c::msgApcTrade(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_APC, APC_TRADE);
    if (label != NULL) {
        setHookProc(&talk_c::endApTrade);
        setLooksMsg(info, label, 0);
        return TRUE;
    }
    return FALSE;
}

// 8003CC98
int dAcNpcNml_c::talk_c::msgApcWant(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_APC, APC_WANT);
    if (label != NULL) {
        setHookProc(&talk_c::endApWant);
        setLooksMsg(info, label, 0);
        return TRUE;
    }
    return FALSE;
}

// 8003CD38
const u8 *dAcNpcNml_c::talk_c::getApcWeights() {
    // Present, Sell, Trade, Want
    static const u8 l_richRoomItem[4] = {25, 25, 25, 25};
    static const u8 l_richRoom[4] = {20, 80, 0, 0};
    static const u8 l_richItem[4] = {0, 0, 50, 50};
    static const u8 l_roomItem[4] = {20, 0, 40, 40};
    static const u8 l_room[4] = {100, 0, 0, 0};
    static const u8 l_item[4] = {0, 0, 50, 50};
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return NULL;
    }
    if (player->getPocketMoney() >= 2000) {
        if (player->findEmptyPocket(0) != -1) {
            dItem::Item item = pickWantedPocketItem(this, player);
            if (item.isValid()) {
                return l_richRoomItem;
            }
            return l_richRoom;
        }
        dItem::Item item = pickWantedPocketItem(this, player);
        if (item.isValid()) {
            return l_richItem;
        }
    } else {
        if (player->findEmptyPocket(0) != -1) {
            dItem::Item item = pickWantedPocketItem(this, player);
            if (item.isValid()) {
                return l_roomItem;
            }
            return l_room;
        }
        dItem::Item item = pickWantedPocketItem(this, player);
        if (item.isValid()) {
            return l_item;
        }
    }
    return NULL;
}

// 8003CE58
int dAcNpcNml_c::talk_c::msgApc(msgInfo_s *info) {
    static const msgFunc l_apcTopics[4] = {
        &talk_c::msgApcPresent, &talk_c::msgApcSell, &talk_c::msgApcTrade, &talk_c::msgApcWant,
    };
    const u8 *weights = getApcWeights();
    int idx = 4;
    if (weights != NULL) {
        int rnd = cM::rndInt(100);
        for (int i = 0; i < 4; i++) {
            if (rnd < weights[i]) {
                idx = i;
                break;
            }
            rnd -= weights[i];
        }
    }
    if ((u32)idx < 4) {
        return (this->*l_apcTopics[idx])(info);
    }
    return FALSE;
}

// 8003CF40
int dAcNpcNml_c::talk_c::msgFreeMoving(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    BOOL moving = FALSE;
    if (animal != NULL && mpMemory != NULL && mpMemory->mFlags.mMoveOutTold) {
        moving = TRUE;
    }
    if (moving && !dSaveData_c::getAnimalBlock()->isMoveOutAnimal(&animal->mID)) {
        moving = FALSE;
    }
    if (moving) {
        const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_D, FREE_D_MOVING);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_D, FREE_D_MOVING);
            mReqProc[0] = &talk_c::reqApMoveOut;
            return TRUE;
        }
    }
    return FALSE;
}

// 8003D04C
const u8 *dAcNpcNml_c::talk_c::getTownWeights() {
    // Event, Snpc
    static const u8 l_event[2] = {50, 50};
    static const u8 l_moving[2] = {25, 75};
    static const u8 l_default[2] = {50, 50};
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    dAnimal_c *animal = getAnimal();
    BOOL moving = animal != NULL ? block->isMoveOutAnimal(&animal->mID) : FALSE;
    if (isEventDay(TRUE)) {
        return l_event;
    }
    return moving ? l_moving : l_default;
}

// 8003D0DC
int dAcNpcNml_c::talk_c::findEventDay(int event, u32 from, u32 to, const dTime_c &time) {
    if (from < to) {
        u32 swap = from;
        from = to;
        to = swap;
    }
    for (u32 day = from; day >= to; day--) {
        if (dEvent::isEventWithin((dQuestEvent_e)event, time, day, day)) {
            return day;
        }
    }
    return -1;
}

// 8003D180
int dAcNpcNml_c::talk_c::msgFreeEvent(msgInfo_s *info) {
    dTime_c time = *dTime_c::getCurrent();
    time.add(0, -6, 0, 0);
    int code = 0;
    int fireworks = dEvent::getFireworksState(time);
    if (dEvent::isEventWithin(EVENT_FISHING_TOURNEY, time, 1, 1)) {
        code = 3;
    } else if (dEvent::isEventWithin(EVENT_BUG_OFF, time, 1, 1)) {
        code = 5;
    } else if (dEvent::isEventWithin(EVENT_FLEA_MARKET, time, 1, 1)) {
        code = 7;
    } else if (dEvent::isEventWithin(EVENT_VALENTINES_DAY, time, 1, 1)) {
        code = 9;
    } else if (dEvent::isEventWithin(EVENT_FESTIVALE, time, 1, 1)) {
        code = 11;
    } else if (fireworks == 2) {
        code = 15;
    } else if (fireworks == 3) {
        code = 16;
    } else if (fireworks == 4) {
        code = 17;
    } else if (dEvent::isEventWithin(EVENT_HALLOWEEN, time, 1, 1)) {
        code = 19;
    } else if (dEvent::isEventWithin(EVENT_HARVEST_FESTIVAL, time, 1, 1)) {
        code = 21;
    } else if (dEvent::isEventWithin(EVENT_TOY_DAY, time, 1, 1)) {
        code = 23;
    } else if (dEvent::isEventWithin(EVENT_COUNTDOWN, time, 1, 1)) {
        code = 25;
    }
    if (code == 0) {
        int nearest = 8;
        int day = findEventDay(EVENT_FISHING_TOURNEY, 5, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 2;
        }
        day = findEventDay(EVENT_BUG_OFF, 5, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 4;
        }
        day = findEventDay(EVENT_FLEA_MARKET, 6, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 6;
        }
        day = findEventDay(EVENT_VALENTINES_DAY, 7, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 8;
        }
        day = findEventDay(EVENT_FESTIVALE, 7, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 10;
        }
        if (fireworks == 1) {
            day = findEventDay(EVENT_FIREWORKS, 6, 2, time);
            if (day != -1 && day < nearest) {
                nearest = day;
                code = 14;
            }
        }
        day = findEventDay(EVENT_HALLOWEEN, 6, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 18;
        }
        day = findEventDay(EVENT_HARVEST_FESTIVAL, 7, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 20;
        }
        day = findEventDay(EVENT_TOY_DAY, 6, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 22;
        }
        day = findEventDay(EVENT_COUNTDOWN, 5, 2, time);
        if (day != -1 && day < nearest) {
            nearest = day;
            code = 24;
        }
    }
    if (code == 0) {
        code = 1;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_E, FREE_E_EVENT);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_E, FREE_E_EVENT);
        return TRUE;
    }
    return FALSE;
}

// A special npc and the FreeE_Snpc message code about it.
struct freeSnpc_s {
    /* 0x0 */ u16 mNpc;
    /* 0x2 */ u16 mCode;
};

// 8003D5C0
int dAcNpcNml_c::talk_c::msgFreeSnpc(msgInfo_s *info) {
    static const freeSnpc_s l_townNpcs[6] = {
        {0x8025, 13}, {0x804C, 14}, {0x8024, 15}, {0x801E, 17}, {0x804A, 18}, {0x804D, 19},
    };
    static const freeSnpc_s l_visitorNpcs[10] = {
        {0x8013, 2}, {0x8001, 3}, {0x8015, 4}, {0x8016, 5}, {0x8007, 6},
        {0x8019, 7}, {0x801A, 8}, {0x801C, 9}, {0x801D, 10}, {0x8023, 11},
    };
    u16 code = 0;
    const freeSnpc_s *npc = l_townNpcs;
    for (u32 i = 0; i < 6; i++, npc++) {
        bool here = fn_801AD174(dItem::Item(npc->mNpc));
        if (here) {
            code = npc->mCode;
            break;
        }
    }
    if (code == 0) {
        dItem::Item any;
        bool here = fn_801AD228(any);
        if (here) {
            code = 12;
        }
    }
    if (code == 0) {
        npc = l_visitorNpcs;
        for (u32 i = 0; i < 10; i++, npc++) {
            bool here = fn_801AD304(dItem::Item(npc->mNpc));
            if (here) {
                code = npc->mCode;
                break;
            }
        }
    }
    if (code == 0) {
        code = 1;
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_E, FREE_E_SNPC);
    if (label != NULL) {
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_E, FREE_E_SNPC);
        return TRUE;
    }
    return FALSE;
}

// 8003D710
int dAcNpcNml_c::talk_c::msgFreeTown(msgInfo_s *info) {
    static const msgFunc l_freeETopics[2] = {&talk_c::msgFreeEvent, &talk_c::msgFreeSnpc};
    static const msgFunc l_freeEEventTopics[2] = {&talk_c::msgFreeGeneral, &talk_c::msgFreeSnpc};
    const msgFunc *topics = isEventDay(TRUE) == FALSE ? l_freeETopics : l_freeEEventTopics;
    const u8 *const selectedWeights = getTownWeights();
    const u8 *weights = selectedWeights;
    int idx = 1;
    if (weights != NULL) {
        int rnd = cM::rndF(100.0f);
        for (int i = 0; i < 2; i++) {
            if (rnd < weights[i]) {
                idx = i;
                break;
            }
            rnd -= weights[i];
        }
    }
    return (this->*topics[idx])(info);
}

// 8003D7D0
BOOL dAcNpcNml_c::talk_c::isBuildingNear(const dItem::Item *building, int xMin, int xMax, int zMin, int zMax) {
    int x = 0;
    int z = 0;
    dItem::Item item = *building;
    dSaveTown_c *town = dSaveData_c::getTown();
    bool found = town->mBuilding.mList.getPos(&x, &z, &item, 1);
    if (found && x >= xMin && x <= xMax && z >= zMin && z <= zMax) {
        return TRUE;
    }
    return FALSE;
}

// A building of the town and the block range around the villager it is looked for in.
struct freeBuilding_s {
    /* 0x0 */ u16 mItem;
    /* 0x4 */ int mXMaxAdd;
    /* 0x8 */ int mXMinAdd;
};

// 8003D894
int dAcNpcNml_c::talk_c::msgFreePlace(msgInfo_s *info) {
    static const freeBuilding_s l_buildings[5] = {
        {BUILDING_SHOP, 0, 0}, {BUILDING_TAILOR, 0, 0}, {BUILDING_TOWN_HALL, 5, -5}, {BUILDING_MUSEUM, 5, -5},
        {BUILDING_BUS_STOP, 3, 0},
    };
    u8 scene = getCurrentScene();
    if (isSceneAttr(scene, SCENE_ATTR_VILLAGER_HOUSE)) {
        const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_F, FREE_F_INSIDE);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_F, FREE_F_INSIDE);
            return TRUE;
        }
    }
    if (!isSceneAttr(scene, SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    dAcNpc_c *npc = mpNpc;
    if (npc == NULL) {
        return FALSE;
    }
    dHomeList_c *homes;
    int myHome;
    const dPersonalID_c *owner;
    const freeBuilding_s *building;
    f32 x = npc->getPosP()->x;
    f32 z = npc->getPosP()->z;
    int blockX = (int)x >> 5;
    int blockZ = (int)z >> 5;
    dItem::Item item;
    int xMin = blockX - 5;
    int xMax = blockX + 5;
    int zMin = blockZ - 7;
    int zMax = blockZ + 1;
    homes = &dSaveData_c::getTown()->mHomes;
    myHome = homes->findCurrentPlayer();
    if ((u32)myHome < 4) {
        item = dSaveBuildingList_c::getPlayerHouseId(myHome);
        if (isBuildingNear(&item, xMin, xMax, zMin, zMax)) {
            const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_F, FREE_F_HOUSE1);
            if (label != NULL) {
                setLooksMsg(info, label, 0);
                setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_F, FREE_F_HOUSE1);
                return TRUE;
            }
        }
    }
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_F, FREE_F_HOUSE2);
    if (label != NULL) {
        for (int i = 0; i < 4; i++) {
            if (i == myHome) {
                continue;
            }
            item = dSaveBuildingList_c::getPlayerHouseId(i);
            if (isBuildingNear(&item, xMin, xMax, zMin, zMax)) {
                u16 code = 6;
                dHome_c *home = homes->getHome(i);
                if (home != NULL && home->mOwner.isValid() && home->mSize < 5) {
                    owner = &home->mOwner;
                    code = home->mSize + 1;
                    if (owner != NULL && owner->isValid()) {
                        setPersonalName(owner, 0);
                        dAnimal_c *animal = getAnimal();
                        if (animal != NULL) {
                            u8 gender = owner->player.mGender;
                            setUnitMsg(this, 1, fn_800F3F38(gender, animal->mID.getLooks(1)));
                        }
                        mMsgPersonal.copy(owner);
                    }
                }
                setLooksMsg(info, label, code);
                setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_F, FREE_F_HOUSE2);
                return TRUE;
            }
        }
    }
    label = getMsgLabel(TALK_FREE, FREE_GROUP_F, FREE_F_BUILDING);
    if (label != NULL) {
        u16 code = 1;
        building = l_buildings;
        for (u32 i = 0; i < 5; i++, building++) {
            item = building->mItem;
            if (isBuildingNear(&item, xMin + building->mXMinAdd, xMax + building->mXMaxAdd, zMin, zMax)) {
                code = i + 2;
                break;
            }
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_F, FREE_F_BUILDING);
        return TRUE;
    }
    return FALSE;
}

// 8003DC3C
const u8 *dAcNpcNml_c::talk_c::getOnlineWeights() {
    // Host, Visitor, JinxV
    static const u8 l_ownTown[3] = {100, 0, 0};
    static const u8 l_otherTown[3] = {0, 90, 10};
    BOOL own;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dLandID_c *townLand = dSaveData_c::getTownLand();
    if (player != NULL) {
        own = (player->mPID.land == *townLand) ? 1 : 0;
    } else {
        own = false;
    }
    if (own) {
        return l_ownTown;
    }
    return l_otherTown;
}

// 8003DCDC
int dAcNpcNml_c::talk_c::msgFreeHost(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_G, FREE_G_HOST);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_G, FREE_G_HOST);
        return TRUE;
    }
    return FALSE;
}

// 8003DD5C
void dAcNpcNml_c::talk_c::setHostWords() {
    dPrivateData_c *host = dPlayerMgr_c::getNetPlayer(0);
    if (host != NULL) {
        const dPersonalID_c *pid = &host->mPID;
        if (pid->isValid()) {
            setPersonalName(pid, 0);
            dAnimal_c *animal = getAnimal();
            if (animal != NULL) {
                u8 gender = pid->player.mGender;
                setUnitMsg(this, 1, fn_800F3F38(gender, animal->mID.getLooks(1)));
            }
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        setLandName(&player->mPID.land, 2);
    }
}

// 8003DE3C
int dAcNpcNml_c::talk_c::msgFreeVisitor(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_G, FREE_G_VISITOR);
    if (label != NULL) {
        int code = 1;
        if (mpMemory != NULL && mpMemory->mFlags.mSameTown) {
            code = 2;
        }
        setHostWords();
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_G, FREE_G_VISITOR);
        return TRUE;
    }
    return FALSE;
}

// 8003DEF8
int dAcNpcNml_c::talk_c::msgFreeJinxV(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_G, FREE_G_JINXV);
    if (label != NULL) {
        setLooksMsg(info, label, (int)cM::rndF(3.0f) + 1);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_G, FREE_G_JINXV);
        mJinxTown = -1;
        setStepProc(&talk_c::stepJinx);
        dSaveTown_c *town = dSaveData_c::getTown();
        setJinxWords(&town->mJinx, NULL);
        return TRUE;
    }
    return FALSE;
}

// 8003DFE8
int dAcNpcNml_c::talk_c::msgFreeOnline(msgInfo_s *info) {
    static const msgFunc l_freeGTopics[3] = {&talk_c::msgFreeHost, &talk_c::msgFreeVisitor, &talk_c::msgFreeJinxV};
    const u8 *weights = getOnlineWeights();
    int idx = 0;
    if (weights != NULL) {
        int rnd = cM::rndF(100.0f);
        for (int i = 0; i < 3; i++) {
            if (rnd < weights[i]) {
                idx = i;
                break;
            }
            rnd -= weights[i];
        }
    }
    return (this->*l_freeGTopics[idx])(info);
}

// 8003E0B4
BOOL dAcNpcNml_c::talk_c::isInTownEvent(dQuestEvent_e event) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL || !animal->mEvent.isInEvent()) {
        return FALSE;
    }
    u8 townEvent = dSaveData_c::getTown()->mAnimals.mTown.mEventId;
    return (dQuestEvent_e)townEvent == event && dEvent::isActive((dQuestEvent_e)townEvent);
}

// 8003E13C
BOOL dAcNpcNml_c::talk_c::isHarvestTopic() {
    if (isInTownEvent(EVENT_HARVEST_FESTIVAL) && !dEvent::isOver(EVENT_HARVEST_FESTIVAL)) {
        return TRUE;
    }
    return FALSE;
}

// 8003E184
BOOL dAcNpcNml_c::talk_c::isHalloweenTopic() {
    if (isInTownEvent(EVENT_HALLOWEEN) && dEvent::isNotStarted(EVENT_HALLOWEEN)) {
        return TRUE;
    }
    return FALSE;
}

// 8003E1CC
BOOL dAcNpcNml_c::talk_c::isCountdownTopic() {
    if (isInTownEvent(EVENT_COUNTDOWN) && dEvent::isNotStarted(EVENT_COUNTDOWN)) {
        return TRUE;
    }
    return FALSE;
}

// 8003E214
BOOL dAcNpcNml_c::talk_c::isFmarketTopic() {
    if (isCurrentSceneAttr(SCENE_ATTR_TOWN) && isInTownEvent(EVENT_FLEA_MARKET)) {
        return TRUE;
    }
    return FALSE;
}

// 8003E26C
BOOL dAcNpcNml_c::talk_c::isXmasTopic() {
    if (isInTownEvent(EVENT_TOY_DAY) && !dEvent::isOver(EVENT_TOY_DAY)) {
        return TRUE;
    }
    return FALSE;
}

// 8003E2B4
BOOL dAcNpcNml_c::talk_c::isEasterTopic() {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    return dEvent::isOngoing(EVENT_BUNNY_DAY) != FALSE;
}

// 8003E308
int dAcNpcNml_c::talk_c::pickComingEvent() {
    static const condFunc l_freeHConds[6] = {
        &talk_c::isHarvestTopic, &talk_c::isHalloweenTopic, &talk_c::isCountdownTopic,
        &talk_c::isFmarketTopic, &talk_c::isXmasTopic, &talk_c::isEasterTopic,
    };
    int idx = 6;
    u32 count = 0;
    for (int i = 0; i < 6; i++) {
        if (l_freeHConds[i] && (this->*l_freeHConds[i])()) {
            count++;
            f32 chance = 100.0f / count;
            if (cM::rndF(100.0f) < chance) {
                idx = i;
            }
        }
    }
    return idx;
}

// 8003E3F0
int dAcNpcNml_c::talk_c::msgFreeHarvest(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_HARVEST);
    if (label != NULL) {
        int code = 1;
        if (dEvent::isOngoing(EVENT_HARVEST_FESTIVAL)) {
            code = 2;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_HARVEST);
        return TRUE;
    }
    return FALSE;
}

// 8003E49C
int dAcNpcNml_c::talk_c::msgFreeCountdown(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_COUNTDOWN);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_COUNTDOWN);
        dTime_c *now = dTime_c::getCurrent();
        setNumber4(now->year + 1, 0);
        mMsgYear = now->year + 1;
        return TRUE;
    }
    return FALSE;
}

// 8003E544
int dAcNpcNml_c::talk_c::msgFreeFmarket(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_FMARKET);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_FMARKET);
        return TRUE;
    }
    return FALSE;
}

// 8003E5C4
int dAcNpcNml_c::talk_c::msgFreeXmas(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_XMAS);
    if (label != NULL) {
        u16 code = 1;
        if (dEvent::isOngoing(EVENT_TOY_DAY)) {
            code = 3;
        }
        if (isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
            code++;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_XMAS);
        return TRUE;
    }
    return FALSE;
}

// 8003E688
int dAcNpcNml_c::talk_c::msgFreeEaster(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_EASTER);
    if (label != NULL) {
        setLooksMsg(info, label, 0);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_EASTER);
        return TRUE;
    }
    return FALSE;
}

// 8003E708
int dAcNpcNml_c::talk_c::msgFreeHalloween(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_H, FREE_H_HALLOWEEN);
    if (label != NULL) {
        int code = 1;
        if (isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
            code = 2;
        }
        setLooksMsg(info, label, code);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_H, FREE_H_HALLOWEEN);
        return TRUE;
    }
    return FALSE;
}

// 8003E7B4
int dAcNpcNml_c::talk_c::msgFreeComingEvent(msgInfo_s *info) {
    static const msgFunc l_freeHTopics[6] = {
        &talk_c::msgFreeHarvest, &talk_c::msgFreeHalloween, &talk_c::msgFreeCountdown,
        &talk_c::msgFreeFmarket, &talk_c::msgFreeXmas, &talk_c::msgFreeEaster,
    };
    int idx = pickComingEvent();
    if ((u32)idx < 6 && l_freeHTopics[idx]) {
        return (this->*l_freeHTopics[idx])(info);
    }
    return FALSE;
}

// 8003E838
int dAcNpcNml_c::talk_c::msgFreeTunekichi(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_I, FREE_I_TUNEKICHI);
    if (label != NULL) {
        setLooksMsg(info, label, 1);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_I, FREE_I_TUNEKICHI);
        if (mpMemory != NULL && mpMemory->mPlayer.isValid()) {
            mpMemory->mFlags.mTunekichiTalked = TRUE;
        }
        return TRUE;
    }
    return FALSE;
}

// 8003E8E4
int dAcNpcNml_c::talk_c::reqTunekichiLetter() {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    if (mpMemory == NULL || !mpMemory->mPlayer.isValid()) {
        return FALSE;
    }
    if (!animal->sendTunekichiLetter(&mpMemory->mPlayer, TRUE, TRUE)) {
        mpMemory->mFlags.mTunekichiInvite = TRUE;
    }
    return FALSE;
}

// 8003E994
int dAcNpcNml_c::talk_c::msgFreeTunekichi2(msgInfo_s *info) {
    const char *label = getMsgLabel(TALK_FREE, FREE_GROUP_I, FREE_I_TUNEKICHI2);
    if (label != NULL) {
        setLooksMsg(info, label, 2);
        setTopic(dNpc::msgMemory_c::KIND_FREE, FREE_GROUP_I, FREE_I_TUNEKICHI2);
        if (mpMemory != NULL && mpMemory->mPlayer.isValid()) {
            mpMemory->mFlags.mTunekichiTalked = TRUE;
        }
        mReqProc[0] = &talk_c::reqTunekichiLetter;
        return TRUE;
    }
    return FALSE;
}

// 8003EA5C
int dAcNpcNml_c::talk_c::msgFreeKK(msgInfo_s *info) {
    static const msgFunc l_freeITopics[2] = {&talk_c::msgFreeTunekichi, &talk_c::msgFreeTunekichi2};
    // chance of the second letter talk by the number of players
    static const u8 l_playerChance[4] = {80, 70, 60, 50};
    BOOL ok = FALSE;
    BOOL valid = FALSE;
    if (mpMemory != NULL && mpMemory->mPlayer.isValid()) {
        valid = TRUE;
    }
    if (valid && !mpMemory->mFlags.mTunekichiInvite) {
        ok = TRUE;
    }
    u32 idx = 2;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimalBlock_c *block = dSaveData_c::getAnimalBlock();
    for (int i = 0; i < ANIMAL_NUM; i++) {
        dAnimal_c *animal = block->getAnimalConst(i);
        if (animal != NULL && animal->mID.isValid()) {
            dAnimalMemory_c *mem = animal->findMemory2(&player->mPID);
            if (mem != NULL && mem->mFlags.mTunekichiTalked) {
                ok = FALSE;
                break;
            }
        }
    }
    if (ok && player != NULL && player->mPID.isFromTown()) {
        if (player->isFlag0(0x14)) {
            if (cM::rndInt(100) < 50) {
                idx = 0;
            }
        } else if (player->isFlag0(0x77)) {
            u32 num = dPrivateData_c::count(dSaveData_c::getRaw()->mPlayers);
            if (num != 0 && num <= 4 && cM::rndInt(100) < l_playerChance[num - 1]) {
                idx = 1;
            }
        }
    }
    BOOL ret = FALSE;
    if (idx < 2 && l_freeITopics[idx]) {
        ret = (this->*l_freeITopics[idx])(info);
    }
    if (ret == FALSE) {
        return msgFreeGeneral(info);
    }
    return TRUE;
}

// 8003EC4C
int dAcNpcNml_c::talk_c::msgFree(msgInfo_s *info) {
    static const msgFunc l_freeGroups[10] = {
        &talk_c::msgFreeGeneral, &talk_c::msgFreeSpecialDay, &talk_c::msgFreeComingEvent, &talk_c::msgFreeHobby,
        &talk_c::msgApc, &talk_c::msgFreeMoving, &talk_c::msgFreeTown, &talk_c::msgFreePlace,
        &talk_c::msgFreeOnline, &talk_c::msgFreeKK,
    };
    setProcSet(&l_talkEntrySets[TALK_FREE]);
    const u8 *weights = getFreeWeights();
    int idx = 0;
    if (weights != NULL) {
        int rnd = cM::rndF(100.0f);
        for (int i = 0; i < 10; i++) {
            if (rnd < weights[i]) {
                idx = i;
                break;
            }
            rnd -= weights[i];
        }
    }
    if (!(this->*l_freeGroups[idx])(info)) {
        msgFreeGeneral(info);
    }
    return TRUE;
}

// 8003EDEC
void dAcNpcNml_c::talk_c::endFree(int arg) {
    recordTalk(NULL);
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// 8003EE44
BOOL dAcNpcNml_c::talk_c::stepJinx(int kind) {
    dDemo_c *ctrl = getController();
    if (ctrl != NULL && ctrl->mNextMsgCode == 0) {
        setProcs(&talk_c::msgJinx, NULL, &talk_c::stepJinx2);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8003EF14
int dAcNpcNml_c::talk_c::msgJinx(msgInfo_s *info) {
    const dSaveJinx_c *jinx;
    u16 code = FREE_JINX_MSG_CONDITION;
    dSaveTownListEntry_c *town = getJinxTown();
    if (town != NULL) {
        jinx = &town->mJinx;
    } else {
        dSaveTown_c *ownTown = dSaveData_c::getTown();
        jinx = &ownTown->mJinx;
    }
    const char *label = town != NULL ? getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_JINX)
                                     : getMsgLabel(TALK_FREE, FREE_GROUP_G, FREE_G_JINXV);
    if (jinx != NULL) {
        u32 condition = jinx->mCondition;
        if (condition < JINX_COND_NUM) {
            code = condition + FREE_JINX_MSG_CONDITION;
        }
    }
    setLooksMsg(info, label, code);
    return TRUE;
}

// 8003EFF8
BOOL dAcNpcNml_c::talk_c::stepJinx2(int kind) {
    dDemo_c *ctrl = getController();
    if (ctrl != NULL && ctrl->mNextMsgCode == 0) {
        setProcs(&talk_c::msgJinx2, NULL, NULL);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8003F0A0
int dAcNpcNml_c::talk_c::msgJinx2(msgInfo_s *info) {
    const dSaveJinx_c *jinx;
    u16 code = FREE_JINX_MSG_OUTCOME;
    dSaveTownListEntry_c *town = getJinxTown();
    if (town != NULL) {
        jinx = &town->mJinx;
    } else {
        dSaveTown_c *ownTown = dSaveData_c::getTown();
        jinx = &ownTown->mJinx;
    }
    const char *label = town != NULL ? getMsgLabel(TALK_FREE, FREE_GROUP_A, FREE_A_JINX)
                                     : getMsgLabel(TALK_FREE, FREE_GROUP_G, FREE_G_JINXV);
    if (jinx != NULL) {
        u32 outcome = jinx->mOutcome;
        if (outcome < JINX_OUTCOME_NUM) {
            code = outcome + FREE_JINX_MSG_OUTCOME;
        }
    }
    setLooksMsg(info, label, code);
    return TRUE;
}

// 8003F184
BOOL dAcNpcNml_c::talk_c::stepFreeClothes(int kind) {
    dDemo_c *ctrl = getController();
    if (ctrl != NULL && ctrl->mNextMsgCode == 0) {
        if (mpMemory != NULL) {
            mpMemory->mFlags.mClothesTalked = TRUE;
            if (mItem0.isValid()) {
                mpMemory->mLastClothes = mItem0;
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 8003F1D8
int dAcNpcNml_c::talk_c::msgEtcHit(msgInfo_s *info) {
    static const char l_Etc_Hit[] = "Etc_Hit";
    setLooksMsg(info, l_Etc_Hit, 0);
    setProcSet(&l_talkProcSets[TALK_PROC_ETC_HIT]);
    return TRUE;
}

// 8003F224
void dAcNpcNml_c::talk_c::endEtc(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}

// 8003F2B4
int dAcNpcNml_c::talk_c::msgEtcPush(msgInfo_s *info) {
    static const char l_Etc_Push[] = "Etc_Push";
    setLooksMsg(info, l_Etc_Push, 0);
    setProcSet(&l_talkProcSets[TALK_PROC_ETC_PUSH]);
    return TRUE;
}

// 8003F304
int dAcNpcNml_c::talk_c::msgEtcFlea(msgInfo_s *info) {
    static const char l_Etc_Flea[] = "Etc_Flea";
    setLooksMsg(info, l_Etc_Flea, 0);
    setProcSet(&l_talkProcSets[TALK_PROC_ETC_FLEA]);
    return TRUE;
}
