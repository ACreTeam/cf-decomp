// The town event announcer (dEventMng_c). .text 8018F588..8018FE10, .data
// 804F9930..804F9AE8, .sdata 8074BF30..8074BF40. See
// include/game/game/d_event_mng.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_event_mng.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>

CUSTOM_BASE_PROFILE(EVENT_MNG, dEventMng_c, fProf::EVENT_MNG,
                    fProf::DRAW_ORDER::EVENT_MNG);

static dEventMng_c::StateFunc l_states[] = {
    &dEventMng_c::executeWait,
    &dEventMng_c::executeOpen,
    &dEventMng_c::executeShow,
    &dEventMng_c::executeCountdown,
};

// 8018F5F4
int dEventMng_c::create() {
    dPostOffice::update();
    mState = STATE_WAIT;
    mMsgCode = 1;
    return 1;
}

// 8018F634
int dEventMng_c::doDelete() {
    return 1;
}

// 8018F63C
int dEventMng_c::execute() {
    (this->*l_states[mState])();
    return 1;
}

// 8018F678
int dEventMng_c::draw() {
    return 1;
}

// 8018F680
void dEventMng_c::executeWait() {
    if (dDemo::isActiveEvent()) {
        if (mKind == KIND_COUNTDOWN) {
            mTimer = 1200;
            dTime_c now = *dTime_c::getCurrent();
            mTimer -= now.sec * 60;
            dCamera_c::sInstance->mRequestedMode =
                dCamera_c::MODE_LOOK_UP; // the countdown fireworks
            mState = STATE_COUNTDOWN;
        } else {
            startMessage();
        }
        return;
    }
    if (dScene::isCityScene(dScene::getCurrentScene())) {
        return;
    }
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_DEMO)) {
        return;
    }
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_TOWN)) {
        return;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return;
    }
    if (player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        return;
    }
    int idx = findEvent(dEvent::getDailyId());
    if (idx == -1) {
        idx = findEventNow();
        if (idx == -1) {
            return;
        }
        u16 code = getStartMsg(idx);
        mMsgCode = code;
        mEventIdx = idx;
        mKind = KIND_START;
        if (code == 0) {
            dEvent::setDailyId(getEventId(idx));
        } else {
            dDemo::requestEvent();
        }
        return;
    }
    if (isEventNow(idx)) {
        return;
    }
    mMsgCode = getEndMsg(idx);
    mKind = KIND_END;
    if (idx == EVENT_IDX_COUNTDOWN) {
        dTime_c now = *dTime_c::getCurrent();
        if (now.hour == 0 && now.min == 0 && now.sec <= 5) {
            mKind = KIND_COUNTDOWN;
            if (dDemo::isType(dDemo::TYPE_MENU)) {
                fn_8019A7AC(); // d_menu_mng: sets dMenuMng_c flag 0x10 (meaning
                               // unknown)
            }
        }
    }
    if (mMsgCode == 0) {
        dEvent::resetDailyId();
    } else {
        dDemo::requestEvent();
    }
}

// 8018F90C
void dEventMng_c::executeOpen() {
    dMsg::Comp_c *comp = mRcpt.getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            mState = STATE_SHOW;
        }
    }
}

// 8018F97C
void dEventMng_c::executeShow() {
    dMsg::Comp_c *comp = mRcpt.getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            dDemo::finishEvent();
            endMessage();
        }
    }
}

// 8018F9F0
void dEventMng_c::executeCountdown() {
    if (mTimer > 0) {
        mTimer--;
        return;
    }
    startMessage();
}

// 8018FA10
void dEventMng_c::startMessage() {
    dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
    comp->attachActor(&mRcpt);
    mRcpt.setMessageLabel("sys_MESS/SYS_Announcement");
    mRcpt.setMessageCode(mMsgCode);
    comp->requestStart(0);
    mState = STATE_OPEN;
    switch (mKind) {
    case KIND_END:
    case KIND_COUNTDOWN:
        dEvent::resetDailyId();
        break;
    case KIND_START:
        dEvent::setDailyId(getEventId(mEventIdx));
        break;
    }
}

// 8018FAB0
void dEventMng_c::endMessage() {
    if (mKind == KIND_COUNTDOWN) {
        dCamera_c::sInstance->mRequestedMode =
            dCamera_c::sInstance->mDefaultMode;
    }
    mRcpt.getController()->detachActor();
    mState = STATE_WAIT;
}

// 8018FB00
int dEventMng_c::findEvent(int eventId) {
    for (int i = 0; i < EVENT_NUM; i++) {
        if (eventId == getEventId(i)) {
            return i;
        }
    }
    return -1;
}

static int l_eventIds[dEventMng_c::EVENT_NUM] = {
    EVENT_TOY_DAY,   EVENT_HARVEST_FESTIVAL, EVENT_HALLOWEEN,
    EVENT_COUNTDOWN, EVENT_FISHING_TOURNEY,  EVENT_BUG_OFF,
    EVENT_FIREWORKS, EVENT_FLEA_MARKET,      EVENT_FESTIVALE,
};
static int l_startHours[dEventMng_c::EVENT_NUM] = {20, 15, 18, 23, 9,
                                                   9,  19, 6,  6};
static int l_startMins[dEventMng_c::EVENT_NUM] = {0, 0, 0, 30, 0, 0, 0, 0, 0};
static int l_endHours[dEventMng_c::EVENT_NUM] = {0, 21, 0, 0, 18, 18, 0, 4, 4};
static int l_endMins[dEventMng_c::EVENT_NUM] = {0, 0, 0, 0, 0, 0, 0, 35, 35};
static u16 l_openMsgs[dEventMng_c::EVENT_NUM] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
static u16 l_reminderMsgs[dEventMng_c::EVENT_NUM] = {12, 13, 14, 15, 16,
                                                     17, 18, 19, 20};
static u16 l_endMsgs[dEventMng_c::EVENT_NUM] = {22, 23, 24, 25, 26,
                                                27, 28, 0,  0};

// 8018FB6C
int dEventMng_c::getEventId(int idx) {
    return l_eventIds[idx];
}

// 8018FB80
BOOL dEventMng_c::isEventNow(int idx) {
    dQuestEvent_e event = (dQuestEvent_e)getEventId(idx);
    if (!dEvent::isActive(event)) {
        return FALSE;
    }
    return dEvent::isNowBetween(getStartHour(idx), getStartMin(idx),
                                getEndHour(idx), getEndMin(idx));
}

// 8018FC28
int dEventMng_c::findEventNow() {
    for (int i = 0; i < EVENT_NUM; i++) {
        if (isEventNow(i)) {
            return i;
        }
    }
    return -1;
}

// 8018FC88
int dEventMng_c::getStartHour(int idx) {
    return l_startHours[idx];
}

// 8018FC9C
int dEventMng_c::getStartMin(int idx) {
    return l_startMins[idx];
}

// 8018FCB0
int dEventMng_c::getEndHour(int idx) {
    return l_endHours[idx];
}

// 8018FCC4
int dEventMng_c::getEndMin(int idx) {
    return l_endMins[idx];
}

// 8018FCD8
u16 dEventMng_c::getStartMsg(int idx) {
    if (dEvent::isNowBefore(getStartHour(idx), getStartMin(idx) + 5) == TRUE) {
        return l_openMsgs[idx];
    }
    if (!dEvent::isNowBefore(getEndHour(idx), getEndMin(idx) - 5)) {
        return 0;
    }
    return l_reminderMsgs[idx];
}

// 8018FDA0
u16 dEventMng_c::getEndMsg(int idx) {
    return l_endMsgs[idx];
}

// 8018FDB4
dEventMng_c::~dEventMng_c() {
}
