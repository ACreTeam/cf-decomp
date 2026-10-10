// The villager talk during the Bug-Off (EVENT_BUG_OFF, "Ev_Bug").
// .text 8004664C..80046BD0. See include/game/game/d_npc_talk_bug.hpp.
#include <game/game/d_npc_talk_bug.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bug_off.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

static const char l_Ev_Bug[] = "Ev_Bug"; // 80750080

// 804A2C60: label per state (getMsgLabel kind TALK_BUG)
const char *l_bugLabels[BUGOFF_STATE_NONE] = {
    l_Ev_Bug, l_Ev_Bug, l_Ev_Bug, l_Ev_Bug, l_Ev_Bug, l_Ev_Bug,
};

// 8004664C
int dAcNpcNml_c::talk_c::getBugOffTalkState() {
    dAnimalSave_c *animals;
    dPersonalID_c *pid;
    BOOL active = dEvent::isOngoing(EVENT_BUG_OFF) != FALSE;
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        active = FALSE;
    }
    animals = &dSaveData_c::getTown()->mAnimals;
    dAnimal_c *animal = getAnimal();
    if (active) {
        if ((dQuestEvent_e)animals->mTown.mEventId != EVENT_BUG_OFF || animal == NULL || !animal->mEvent.isInEvent()) {
            active = FALSE;
        }
    }

    int state = BUGOFF_STATE_NONE;
    dBugOff_c *bugOff = &dSaveData_c::getRaw()->mBugOff;
    dAnmPersonalID_c *leaderAnimal = bugOff->getAnimal(0);
    dPersonalID_c *leaderPlayer = bugOff->getPlayer(0);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dNpcEntry_c *entry = getEntry();
    if (active) {
        if (mpMemory == NULL || !mpMemory->isEventFlag(0)) {
            state = BUGOFF_STATE_FIRST;
        } else if (leaderAnimal->isValid()) {
            if (animal->mID == *leaderAnimal) {
                if (entry == NULL || !entry->_28C.mEventTalked) {
                    state = BUGOFF_STATE_NPC_LEADS;
                } else {
                    state = BUGOFF_STATE_NPC_LEADS_AGAIN;
                }
            } else {
                state = BUGOFF_STATE_OTHER_NPC_LEADS;
            }
        } else if (leaderPlayer->isValid()) {
            if (player != NULL && (pid = &player->mPID, *pid == *leaderPlayer)) {
                state = BUGOFF_STATE_PLAYER_LEADS;
            } else {
                state = BUGOFF_STATE_OTHER_PLAYER_LEADS;
            }
        }
    }
    return state;
}

// 800468B4
const dAcNpcNml_c::talk_c::procSet_s *dAcNpcNml_c::talk_c::getBugOffProcSet() {
    if ((u32)getBugOffTalkState() < BUGOFF_STATE_NONE) {
        return &l_talkEntrySets[TALK_BUG];
    }
    return NULL;
}

// 800468EC
int dAcNpcNml_c::talk_c::msgBugOff(msgInfo_s *info) {
    u8 gender;
    int state = getBugOffTalkState();
    if ((u32)state < BUGOFF_STATE_NONE) {
        const char *label = getMsgLabel(TALK_BUG, state, 0);
        if (label != NULL) {
            u16 code = state + 1;
            if (code >= 2) {
                code++;
            }
            setLooksMsg(info, label, code);
            setProcSet(&l_talkEntrySets[TALK_BUG]);
            dNpcEntry_c *entry = getEntry();
            if (state == BUGOFF_STATE_NPC_LEADS && entry != NULL) {
                entry->_28C.mEventTalked = TRUE;
            }

            dBugOff_c *bugOff = &dSaveData_c::getRaw()->mBugOff;
            dAnmPersonalID_c *leaderAnimal = bugOff->getAnimal(0);
            dPersonalID_c *leaderPlayer = bugOff->getPlayer(0);
            dPlayerMgr_c::getCurrentPlayer();
            dAnimal_c *animal = getAnimal();
            dItem::Item item = bugOff->getItem(0);
            if (item.isValid()) {
                setItemName(&item, 0);
            }
            formatNumber(bugOff->getScore(0), 1, 6, dScript::NUM_FORMAT_REGION);
            gender = GENDER_OTHER;
            if (leaderAnimal->isValid()) {
                setAnmPersonalName(leaderAnimal, 2);
                gender = leaderAnimal->getGender(1);
            } else if (leaderPlayer->isValid()) {
                setPersonalName(leaderPlayer, 2);
                gender = leaderPlayer->player.mGender;
            }
            if (animal != NULL && gender < GENDER_OTHER) {
                u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                if (unit == 0) {
                    clearWord(3);
                } else {
                    getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
                }
            }

            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80046B18
void dAcNpcNml_c::talk_c::endBugOff(int arg) {
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
