// The villager talk at the fireworks show (EVENT_FIREWORKS, "Ev_Fireworks").
// .text 80046BD0..800470E4. See include/game/game/d_npc_talk_fireworks.hpp.
#include <game/game/d_npc_talk_fireworks.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

static const char l_Ev_Fireworks[] = "Ev_Fireworks"; // 8046C798

// 804A2C90: label per state (getMsgLabel kind TALK_FIREWORKS)
const char *l_fireworksLabels[FIREWORKS_STATE_NONE] = {
    l_Ev_Fireworks, l_Ev_Fireworks, l_Ev_Fireworks, l_Ev_Fireworks,
};

// 80046BD0
int dAcNpcNml_c::talk_c::getFireworksTalkState() {
    dAnimalSave_c *animals;
    BOOL active = FALSE;
    if (dEvent::isActive(EVENT_FIREWORKS) && !dEvent::isOver(EVENT_FIREWORKS)) {
        active = TRUE;
    }
    if (!isCurrentSceneAttr(SCENE_ATTR_TOWN)) {
        active = FALSE;
    }
    animals = &dSaveData_c::getTown()->mAnimals;
    dAnimal_c *animal = getAnimal();
    if (active) {
        if ((dQuestEvent_e)animals->mTown.mEventId != EVENT_FIREWORKS || animal == NULL || !animal->mEvent.isInEvent()) {
            active = FALSE;
        }
    }

    int state = FIREWORKS_STATE_NONE;
    if (active) {
        dTime_c *now = dTime_c::getCurrent();
        dTime_c start;
        if (dEvent::getStartTime(EVENT_FIREWORKS, &start)) {
            if (dEvent::isNotStarted(EVENT_FIREWORKS)) {
                // up to 30 minutes before the start
                start.add(0, 0, -30, 0);
                if (dTime_c::isSameOrAfter(*now, start)) {
                    state = FIREWORKS_STATE_BEFORE;
                }
            } else {
                start.add(0, 1, 0, 0);
                if (!dTime_c::isSameOrAfter(*now, start)) {
                    state = FIREWORKS_STATE_FIRST_HOUR;
                } else {
                    dTime_c end;
                    if (dEvent::getEndTime(EVENT_FIREWORKS, &end)) {
                        end.add(0, -1, 0, 0);
                        if (dTime_c::isSameOrAfter(*now, end)) {
                            state = FIREWORKS_STATE_LAST_HOUR;
                        }
                    }
                }
                if (state == FIREWORKS_STATE_NONE) {
                    state = FIREWORKS_STATE_MIDDLE;
                }
            }
        }
    }
    return state;
}

// 80046F60
const dAcNpcNml_c::talk_c::procSet_s *dAcNpcNml_c::talk_c::getFireworksProcSet() {
    if ((u32)getFireworksTalkState() < FIREWORKS_STATE_NONE) {
        return &l_talkEntrySets[TALK_FIREWORKS];
    }
    return NULL;
}

// 80046F98
int dAcNpcNml_c::talk_c::msgFireworks(msgInfo_s *info) {
    int state = getFireworksTalkState();
    if ((u32)state < FIREWORKS_STATE_NONE) {
        const char *label = getMsgLabel(TALK_FIREWORKS, state, 0);
        if (label != NULL) {
            setLooksMsg(info, label, state + 1);
            setProcSet(&l_talkEntrySets[TALK_FIREWORKS]);
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80047054
void dAcNpcNml_c::talk_c::endFireworks(int arg) {
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
