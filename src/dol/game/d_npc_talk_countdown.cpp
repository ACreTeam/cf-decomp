// The villager talk at the New Year's countdown (EVENT_COUNTDOWN, "Ev_Countdown").
// .text 80043C34..80043EBC. See include/game/game/d_npc_talk_countdown.hpp.
#include <game/game/d_npc_talk_countdown.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_save_data.hpp>

static const char l_Ev_Countdown[] = "Ev_Countdown"; // 8046C6C0

// 804A2A90: label per state (getMsgLabel kind TALK_COUNTDOWN)
const char *l_countdownLabels[COUNTDOWN_STATE_NONE] = {
    l_Ev_Countdown, l_Ev_Countdown, l_Ev_Countdown, l_Ev_Countdown,
    l_Ev_Countdown, l_Ev_Countdown, l_Ev_Countdown,
};

// 80043C34
int dAcNpcNml_c::talk_c::msgCountdown(msgInfo_s *info) {
    BOOL inEvent = FALSE;
    if (dEvent::isOngoing(EVENT_COUNTDOWN) || dEvent::isOver(EVENT_COUNTDOWN)) {
        inEvent = TRUE;
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = getAnimal();
    if ((dQuestEvent_e)town->mAnimals.mTown.mEventId != EVENT_COUNTDOWN || animal == NULL || !animal->mEvent.isInEvent()) {
        inEvent = FALSE;
    }

    u32 state = COUNTDOWN_STATE_NONE;
    if (inEvent) {
        dTime_c *now = dTime_c::getCurrent();
        if (now->hour == 23) {
            if (now->min <= 29) {
                state = COUNTDOWN_STATE_2300;
            } else if (now->min <= 54) {
                state = COUNTDOWN_STATE_2330;
            } else if (now->min <= 58) {
                state = COUNTDOWN_STATE_2355;
            } else {
                state = COUNTDOWN_STATE_2359;
            }
        } else if (mpMemory != NULL && !mpMemory->mFlags.mEventTalked && dEvent::isOngoing(EVENT_COUNTDOWN)) {
            state = COUNTDOWN_STATE_FIRST;
        } else {
            state = COUNTDOWN_STATE_OTHER;
            if (now->hour < 2) {
                state = COUNTDOWN_STATE_NEW_YEAR;
            }
        }
    }

    if (state < COUNTDOWN_STATE_NONE) {
        const char *label = getMsgLabel(TALK_COUNTDOWN, state, 0);
        if (label != NULL) {
            setLooksMsg(info, label, state + 1);
            setProcSet(&l_talkEntrySets[TALK_COUNTDOWN]);
            if (mpMemory != NULL && state == COUNTDOWN_STATE_FIRST) {
                mpMemory->mFlags.mEventTalked = TRUE;
                dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
                if (npc != NULL) {
                    fn_800F05C8(npc->getNpcIdx(), mMemoryIdx);
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

// 80043E2C
void dAcNpcNml_c::talk_c::endCountdown(int arg) {
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
