// The villager talk during the fishing tourney (EVENT_FISHING_TOURNEY, "Ev_Fishing").
// .text 800470E4..800476E0. See include/game/game/d_npc_talk_fishing.hpp.
#include <game/game/d_npc_talk_fishing.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_ev_fishing.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

static const char l_Ev_Fishing[] = "Ev_Fishing"; // 8046C7A8

// 804A2CA0: label per state (getMsgLabel kind TALK_FISHING)
const char *l_fishingLabels[FISHING_STATE_NONE] = {
    l_Ev_Fishing, l_Ev_Fishing, l_Ev_Fishing, l_Ev_Fishing, l_Ev_Fishing, l_Ev_Fishing, l_Ev_Fishing,
};

// 800470E4
int dAcNpcNml_c::talk_c::getFishingTalkState() {
    dAnimalSave_c *animals;
    u8 *record;
    dPersonalID_c *pid;
    BOOL active = dEvent::isOngoing(EVENT_FISHING_TOURNEY) != FALSE;
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        active = FALSE;
    }
    animals = &dSaveData_c::getTown()->mAnimals;
    dAnimal_c *animal = getAnimal();
    if (active) {
        if ((dQuestEvent_e)animals->mTown.mEventId != EVENT_FISHING_TOURNEY || animal == NULL ||
            !animal->mEvent.isInEvent()) {
            active = FALSE;
        }
    }

    int state = FISHING_STATE_NONE;
    record = dSaveData_c::getRaw()->_0632F0;
    dAnmPersonalID_c *leaderAnimal = fn_801533E0(record, 0);
    dPersonalID_c *leaderPlayer = fn_801533C8(record, 0);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dNpcEntry_c *entry = getEntry();
    if (active) {
        if (mpMemory == NULL || !mpMemory->isEventFlag(0)) {
            if (!fn_80153410(record).isValid()) {
                state = FISHING_STATE_FIRST;
            } else {
                state = FISHING_STATE_FIRST_FISH;
            }
        } else if (leaderAnimal->isValid()) {
            if (animal->mID == *leaderAnimal) {
                if (entry == NULL || !entry->_28C.mEventTalked) {
                    state = FISHING_STATE_NPC_LEADS;
                } else {
                    state = FISHING_STATE_NPC_LEADS_AGAIN;
                }
            } else {
                state = FISHING_STATE_OTHER_NPC_LEADS;
            }
        } else if (leaderPlayer->isValid()) {
            if (player != NULL && (pid = &player->mPID, *pid == *leaderPlayer)) {
                state = FISHING_STATE_PLAYER_LEADS;
            } else {
                state = FISHING_STATE_OTHER_PLAYER_LEADS;
            }
        }
    }
    return state;
}

// 8004736C
const dAcNpcNml_c::talk_c::procSet_s *dAcNpcNml_c::talk_c::getFishingProcSet() {
    if ((u32)getFishingTalkState() < FISHING_STATE_NONE) {
        return &l_talkEntrySets[TALK_FISHING];
    }
    return NULL;
}

// 800473A4
int dAcNpcNml_c::talk_c::msgFishing(msgInfo_s *info) {
    u8 *record;
    int state = getFishingTalkState();
    u8 gender;
    record = dSaveData_c::getRaw()->_0632F0;
    dItem::Item item = fn_80153410(record);
    dAnmPersonalID_c *leaderAnimal = fn_801533E0(record, 0);
    dPersonalID_c *leaderPlayer = fn_801533C8(record, 0);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dNpcEntry_c *entry = getEntry();
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = getAnimal();
    if ((u32)state < FISHING_STATE_NONE) {
        const char *label = getMsgLabel(TALK_FISHING, state, 0);
        if (label != NULL) {
            setLooksMsg(info, label, state + 1);
            setProcSet(&l_talkEntrySets[TALK_FISHING]);
            if (state == FISHING_STATE_NPC_LEADS && entry != NULL) {
                entry->_28C.mEventTalked = TRUE;
            }
            if (item.isValid()) {
                setItemName(&item, 0);
            }
            gender = GENDER_OTHER;
            if (leaderAnimal->isValid()) {
                setAnmPersonalName(leaderAnimal, 3);
                gender = leaderAnimal->getGender(1);
            } else if (leaderPlayer->isValid()) {
                setPersonalName(leaderPlayer, 3);
                gender = leaderPlayer->player.mGender;
            }
            if (animal != NULL && gender < GENDER_OTHER) {
                u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                if (unit == 0) {
                    clearWord(4);
                } else {
                    getController()->fn_801A5874(4, unit, "sys_STRING/STR_Unit");
                }
            }
            dItem::Item fish = fn_8015341C(record, 0);
            if (fish.isValid()) {
                setItemName(&fish, 5);
            } else if (item.isValid()) {
                setItemName(&item, 5);
            }
            setDecimal(fn_8015343C(fn_801533F8(record, 0)), 1, 1);

            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80047628
void dAcNpcNml_c::talk_c::endFishing(int arg) {
    recordTalk(NULL);
    if (mpMemory != NULL) {
        mpMemory->onEventFlag(0);
        dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
        if (npc != NULL) {
            fn_800F04E8(npc->getNpcIdx(), mMemoryIdx, 0);
        }
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
