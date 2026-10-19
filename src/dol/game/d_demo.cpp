// The demo process and request queue (dDemo_c, dDemo). .text
// 8018DAC8..8018F588, .ctors 8046582C, .rodata 8047BF40 (type table), .data
// 804F98D8 (vtable), .sdata 8074BF18 (profile / RTTI), .sbss
// 8074E9D0..8074E9E8. See include/game/game/d_demo.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_demo.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_cd.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_demo_sync.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_req_pool.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_a_player.hpp>

CUSTOM_BASE_PROFILE(DEMO, dDemo_c, fProf::DEMO, fProf::DRAW_ORDER::DEMO);

namespace dDemo {

static u16 sFlags;
static Req_c *sCurrent;
static dNetReqPool::ReqList_c sReqList;
static u32 sTalkActorID;  // demo ID of the target of the running talk demo
                          // (getTalkActor)
static int sTalkCooldown; // frames left before a new TYPE_TALK may start

// 8018DB14
BOOL isFlag(u16 flag) {
    return (sFlags & flag) != 0;
}

// 8018DB2C
void onFlag(u16 flag) {
    sFlags |= flag;
}

// 8018DB3C
void offFlag(u16 flag) {
    sFlags &= ~flag;
}

} // namespace dDemo

using namespace dDemo;

// 8018DB4C
int dDemo_c::create() {
    initList();
    sFlags = 0;
    sTalkCooldown = 0;
    dDemoSync::init();
    return SUCCEEDED;
}

// 8018DB80
int dDemo_c::doDelete() {
    return SUCCEEDED;
}

// 8018DB88
int dDemo_c::execute() {
    if (dReset::Manage_c::GetInstance()->isResetMode()) {
        sReqList.clear();
        return SUCCEEDED;
    }
    execCurrent();
    endCurrent();
    selectNext();
    startCurrent();
    sReqList.clear();
    if (sTalkCooldown > 0) {
        sTalkCooldown--;
    }
    dDemoSync::update();
    return SUCCEEDED;
}

// 8018DC2C
int dDemo_c::draw() {
    return SUCCEEDED;
}

namespace dDemo {

// A demo type's functions (l_typeFuncs). check: may the queued request start
// now; start / execute: TRUE when done; end: TRUE when the request may be
// freed.
struct TypeFuncs {
    BOOL (*check)(Req_c *req);
    BOOL (*start)(Req_c *req);
    BOOL (*execute)(Req_c *req);
    BOOL (*end)(Req_c *req);
};

// 8018DC34
static BOOL checkNone(Req_c *req) {
    return FALSE;
}

// 8018DC3C
static BOOL startNone(Req_c *req) {
    return TRUE;
}

// 8018DC44
static BOOL execNone(Req_c *req) {
    return TRUE;
}

// 8018DC4C
static BOOL endNone(Req_c *req) {
    return TRUE;
}

// 8018DC54
static BOOL checkMenu(Req_c *req) {
    if (!dMenuMng_c::isFree()) {
        return FALSE;
    }
    if (!dPlayerMgr::requestDemoStart()) {
        return FALSE;
    }
    u8 menu = req->mParam;
    if (menu != 0x79 && dMenuMng_c::open(menu, TRUE)) {
        dMenuMng_c::startMenuBgm(menu);
    }
    return TRUE;
}

// 8018DCD0
static BOOL startMenu(Req_c *req) {
    return TRUE;
}

// 8018DCD8
static BOOL execMenu(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018DCEC
static BOOL endMenu(Req_c *req) {
    BOOL ret = dPlayerMgr::endDemoWait();
    fn_8019A7B4();
    dMenuMng_c::endMenuBgm();
    return ret;
}

// 8018DD24
BOOL requestMenu(u8 menu) {
    dDemoActor_c *player = (dDemoActor_c *)dScene::getScenePlayer();
    if (player == NULL) {
        return FALSE;
    }
    return request(0, player->getDemoID(), TYPE_MENU, 2, menu);
}

// 8018DD7C
void finishMenu() {
    sCurrent->mStep = STEP_DONE;
}

// 8018DD8C
BOOL requestPlayerDemo(int type) {
    dDemoActor_c *player = (dDemoActor_c *)dScene::getScenePlayer();
    if (player == NULL) {
        return FALSE;
    }
    return request(0, player->getDemoID(), type, 4, 0);
}

// 8018DDE4
BOOL isActive(int type) {
    if (isType(type) && sCurrent->mStep != STEP_DONE) {
        return TRUE;
    }
    return FALSE;
}

// 8018DE2C
BOOL finish(int type) {
    if (!isType(type)) {
        return FALSE;
    }
    sCurrent->mStep = STEP_DONE;
    return TRUE;
}

// 8018DE70: asks the host for the target's demo slot
static BOOL requestSync(Req_c *req) {
    switch (dDemoSync::checkRequest(req->mTargetID)) {
    case DEMO_SYNC_CHECK_OK:
        req->mStep = STEP_START;
        break;
    case DEMO_SYNC_CHECK_BUSY:
        return FALSE;
    case DEMO_SYNC_CHECK_REQUEST:
        req->mStep = STEP_WAIT_SYNC;
        break;
    }
    dDemoSync::request(req->mTargetID);
    return TRUE;
}

// 8018DEE8: the target is run by the scene owner: a guest asks it for the start
// check
static BOOL requestStart(Req_c *req) {
    if (dDemoSync::isRemote()) {
        dDemoSync::sendStart(req->mTargetID, req->mCheck);
        req->mStep = STEP_WAIT_SYNC;
    } else {
        if (dNet::isSessionActive() && req->mCheck == DEMO_CMD_SPEAK) {
            dDemoActor_c *actor = dDemoActor_c::findByDemoID(req->mTargetID);
            if (!actor->canSpeak(dPlayerMgr::getPlayerActor(4))) {
                return FALSE;
            }
        }
        req->mStep = STEP_START;
    }
    return TRUE;
}

// 8018DF9C
static BOOL startTalkDemo(Req_c *req) {
    BOOL ok;
    if (dDemoActor_c::findByDemoID(req->mTargetID)->isDemoNetSync()) {
        ok = requestStart(req);
    } else {
        ok = requestSync(req);
    }
    if (ok) {
        sTalkActorID = req->mTargetID;
        dPlayerMgr::startDemoWait();
    }
    return ok;
}

// 8018E010
void sendCommand(dDemoActor_c *actor, u8 cmd) {
    if (actor->isDemoNetSync() && dDemoSync::isRemote()) {
        dDemoSync::sendCommand(actor->getDemoID(), cmd);
    }
    actor->onDemoCmd(cmd, DEMO_SYNC_MEMBER_SELF);
}

// 8018E088
static void sendTargetCommand(Req_c *req, u8 cmd) {
    dDemoActor_c *actor = dDemoActor_c::findByDemoID(req->mTargetID);
    if (actor != NULL) {
        if (actor->isDemoNetSync() && dDemoSync::isRemote()) {
            dDemoSync::sendCommand(actor->getDemoID(), cmd);
        }
        actor->onDemoCmd(cmd, DEMO_SYNC_MEMBER_SELF);
    }
    sTalkActorID = 0;
}

// 8018E118
static BOOL endTalkDemo(Req_c *req) {
    if (!dPlayerMgr::endDemoWait()) {
        return FALSE;
    }
    if (req->mStep != STEP_REJECTED) {
        sendTargetCommand(req, DEMO_CMD_END);
    }
    if (req->mStep == STEP_DONE) {
        dDemoSync::release();
    }
    return TRUE;
}

// 8018E180
static BOOL checkTalk(Req_c *req) {
    if (sTalkCooldown > 0) {
        return FALSE;
    }
    dDemoActor_c *target = dDemoActor_c::findByDemoID(req->mTargetID);
    dDemoActor_c *src = dDemoActor_c::findByDemoID(req->mSrcID);
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    req->mCheck = DEMO_CMD_TALK;
    if (target == NULL) {
        target = dDemoActor_c::findInteraction(src);
        if (target == NULL) {
            return FALSE;
        }
        req->mTargetID = target->getDemoID();
    } else if (!target->checkInteraction(src)) {
        return FALSE;
    }
    if (startTalkDemo(req)) {
        return TRUE;
    }
    return FALSE;
}

// 8018E264
static BOOL startTalk(Req_c *req) {
    dDemoActor_c *actor = dDemoActor_c::findByDemoID(req->mTargetID);
    switch (req->mStep) {
    case STEP_WAIT_SYNC:
        switch (dDemoSync::getState()) {
        case DEMO_SYNC_ACCEPT:
            req->mStep = STEP_START;
            break;
        case DEMO_SYNC_REJECT:
            req->mStep = STEP_REJECTED;
            return TRUE;
        }
        return FALSE;
    default:
        switch (req->mStep) {
        case STEP_START: {
            actor->onDemoCmd(DEMO_CMD_START, DEMO_SYNC_MEMBER_SELF);
            u8 state = actor->getDemoState();
            if (req->mCheck == DEMO_CMD_SPEECH) {
                state = 2;
            }
            if (state == 2) {
                if (!dPlayerMgr::requestDemoWait()) {
                    return FALSE;
                }
                req->mStep = STEP_RUN;
                sendCommand(actor, req->mCheck);
                return TRUE;
            }
            dPlayerMgr::putAwayToolForTalk(state);
            req->mStep = STEP_WAIT_PLAYER;
            return FALSE;
        }
        case STEP_WAIT_PLAYER:
            if (dPlayerMgr::canFinishToolAction()) {
                req->mStep = STEP_RUN;
                sendCommand(actor, req->mCheck);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

// 8018E3B8
static BOOL execTalk(Req_c *req) {
    switch (req->mStep) {
    case STEP_DONE:
    case STEP_REJECTED:
        return TRUE;
    case STEP_RUN:
        break;
    }
    return FALSE;
}

// 8018E3EC
static BOOL endTalk(Req_c *req) {
    if (!endTalkDemo(req)) {
        return FALSE;
    }
    sTalkCooldown = 10;
    return TRUE;
}

// 8018E428
BOOL requestTalk(dDemoActor_c *src, dDemoActor_c *target) {
    u32 targetID = 0;
    if (target != NULL) {
        targetID = target->getDemoID();
    }
    return request(src->getDemoID(), targetID, TYPE_TALK, 3, 0);
}

// 8018E48C
BOOL finishTalk(dDemoActor_c *actor) {
    if (!isTalking()) {
        return FALSE;
    }
    Req_c *req = sCurrent;
    if (req->mTargetID != actor->getDemoID()) {
        return FALSE;
    }
    req->mStep = STEP_DONE;
    return TRUE;
}

// 8018E4FC
static BOOL checkSpeak(Req_c *req) {
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    req->mCheck = DEMO_CMD_SPEAK;
    return startTalkDemo(req) != FALSE;
}

// 8018E554
static BOOL startSpeak(Req_c *req) {
    return startTalk(req);
}

// 8018E558
static BOOL execSpeak(Req_c *req) {
    return execTalk(req);
}

// 8018E55C
static BOOL endSpeak(Req_c *req) {
    return endTalkDemo(req);
}

// 8018E560
BOOL requestSpeak(dDemoActor_c *actor, dDemoActor_c *target) {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == NULL) {
        return FALSE;
    }
    return request(player->getDemoID(), actor->getDemoID(), TYPE_SPEAK, 3, 0);
}

// 8018E5D4
static BOOL checkSceneStart(Req_c *req) {
    return FALSE;
}

// 8018E5DC
static BOOL startSceneStart(Req_c *req) {
    return TRUE;
}

// 8018E5E4
static BOOL execSceneStart(Req_c *req) {
    if (isHoldStart()) {
        return FALSE;
    }
    switch (req->mStep) {
    case STEP_DONE:
        return TRUE;
    case STEP_RUN:
        break;
    }
    return FALSE;
}

// 8018E648
static BOOL endSceneStart(Req_c *req) {
    return dPlayerMgr::endDemoWait() != FALSE;
}

// 8018E674
void reset() {
    sFlags = 0;
    initList();
    dDemoSync::init();
}

// 8018E6A0
void startScene() {
    initList();
    Req_c *req = dNetReqPool::alloc();
    u32 code = dSTR::getCurBuildingCode();
    req->mSrcID = code;
    req->mTargetID = 0;
    req->mState = REQ_STATE_EXECUTE;
    req->mStep = STEP_START;
    req->mPriority = 1;
    if (code != 0) {
        req->mType = TYPE_ENTER;
        req->mStep = STEP_ENTER;
    } else {
        req->mType = TYPE_SCENE_START;
        req->mStep = STEP_RUN;
    }
    sCurrent = req;
    if (isDayChangeWarp()) {
        onFlag(FLAG_SILENCE);
    }
}

// 8018E734
BOOL finishSceneStart() {
    if (!isType(TYPE_SCENE_START) && !isType(TYPE_ENTER)) {
        return FALSE;
    }
    Req_c *req = sCurrent;
    if (req->mStep == STEP_RUN) {
        req->mStep = STEP_DONE;
        return TRUE;
    }
    return FALSE;
}

// 8018E79C
void onHoldStart() {
    onFlag(FLAG_HOLD_START);
}

// 8018E7A4
void offHoldStart() {
    offFlag(FLAG_HOLD_START);
}

// 8018E7AC
BOOL isHoldStart() {
    return isFlag(FLAG_HOLD_START);
}

// 8018E7B4
static BOOL checkExitScene(Req_c *req) {
    dDemoActor_c *actor = dDemoActor_c::findByDemoID(req->mSrcID);
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    dPlayerMgr::startDemoWait();
    if (actor != NULL) {
        actor->onDemoCmd(DEMO_CMD_EXIT, DEMO_SYNC_MEMBER_SELF);
    }
    dCd_c::getSceneChange()->requestExit(req->mParam, 2, 2);
    return TRUE;
}

// 8018E844
static BOOL startExitScene(Req_c *req) {
    return TRUE;
}

// 8018E84C
static BOOL execExitScene(Req_c *req) {
    return FALSE;
}

// 8018E854
static BOOL endExitScene(Req_c *req) {
    return TRUE;
}

// 8018E85C
BOOL requestExitScene(dDemoActor_c *actor, int exit) {
    u32 id = 0;
    if (actor != NULL) {
        id = actor->getDemoID();
    }
    return request(id, 0, TYPE_EXITSCENE, 1, exit);
}

// 8018E8B0
static BOOL checkEnter(Req_c *req) {
    return FALSE;
}

// 8018E8B8
static BOOL startEnter(Req_c *req) {
    return TRUE;
}

// 8018E8C0
static BOOL execEnter(Req_c *req) {
    if (req->mStep == STEP_ENTER) {
        if (dDemoActor_c::findByDemoID(req->mSrcID) == NULL) {
            return FALSE;
        }
        switch (dDemoSync::checkRequest(req->mSrcID)) {
        case DEMO_SYNC_CHECK_BUSY:
            break;
        case DEMO_SYNC_CHECK_REQUEST:
            dDemoSync::request(req->mSrcID);
            req->mStep = STEP_ENTER_WAIT;
            break;
        case DEMO_SYNC_CHECK_OK:
            req->mStep = STEP_ENTER_START;
            break;
        }
    }
    if (req->mStep == STEP_ENTER_WAIT) {
        switch (dDemoSync::getState()) {
        case DEMO_SYNC_ACCEPT:
            req->mStep = STEP_ENTER_START;
            break;
        case DEMO_SYNC_REJECT:
            dDemoSync::request(req->mSrcID);
            break;
        }
    }
    if (req->mStep == STEP_ENTER_START) {
        dDemoActor_c *actor = dDemoActor_c::findByDemoID(req->mSrcID);
        req->mStep = STEP_RUN;
        actor->onDemoCmd(DEMO_CMD_ENTER, DEMO_SYNC_MEMBER_SELF);
    }
    if (req->mStep == STEP_DONE) {
        return TRUE;
    }
    return FALSE;
}

// 8018E9D8
static BOOL endEnter(Req_c *req) {
    if (endSceneStart(req)) {
        dDemoSync::release();
        return TRUE;
    }
    return FALSE;
}

// 8018EA10
static BOOL check7(Req_c *req) {
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    dPlayerMgr::startDemoWait();
    return TRUE;
}

// 8018EA48
static BOOL start7(Req_c *req) {
    return TRUE;
}

// 8018EA50
static BOOL exec7(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018EA64
static BOOL end7(Req_c *req) {
    return dPlayerMgr::endDemoWait();
}

// 8018EA68
BOOL request7() {
    return requestPlayerDemo(TYPE_7);
}

// 8018EA70
BOOL isActive7() {
    return isActive(TYPE_7);
}

// 8018EA78
BOOL finish7() {
    return finish(TYPE_7);
}

// 8018EA80
static BOOL checkReport(Req_c *req) {
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    req->mCheck = DEMO_CMD_REPORT;
    return startTalkDemo(req) != FALSE;
}

// 8018EAD8
static BOOL startReport(Req_c *req) {
    return startTalk(req);
}

// 8018EADC
static BOOL execReport(Req_c *req) {
    return execTalk(req);
}

// 8018EAE0
static BOOL endReport(Req_c *req) {
    return endTalkDemo(req);
}

// 8018EAE4
BOOL requestReport(dDemoActor_c *actor, dDemoActor_c *target) {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == NULL) {
        return FALSE;
    }
    return request(player->getDemoID(), actor->getDemoID(), TYPE_REPORT, 3, 0);
}

// 8018EB58
static BOOL check9(Req_c *req) {
    req->mStep = STEP_RUN;
    return TRUE;
}

// 8018EB68
static BOOL start9(Req_c *req) {
    return TRUE;
}

// 8018EB70
static BOOL exec9(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018EB84
static BOOL end9(Req_c *req) {
    return TRUE;
}

// 8018EB8C
BOOL request9() {
    if (isType(TYPE_ANY)) {
        return FALSE;
    }
    return request(0, 0, TYPE_9, 0, 0);
}

// 8018EBD8
BOOL finish9() {
    if (!isActive9()) {
        return FALSE;
    }
    sCurrent->mStep = STEP_DONE;
    return TRUE;
}

// 8018EC18
BOOL isActive9() {
    return isActive(TYPE_9);
}

// 8018EC20
static BOOL check10(Req_c *req) {
    if (!dPlayerMgr::canStartIdleDemo()) {
        return FALSE;
    }
    if (!dPlayerMgr::hasCurrentActor()) {
        return FALSE;
    }
    req->mStep = STEP_START;
    return TRUE;
}

// 8018EC7C
static BOOL start10(Req_c *req) {
    return TRUE;
}

// 8018EC84
static BOOL exec10(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018EC98
static BOOL end10(Req_c *req) {
    return TRUE;
}

// 8018ECA0
BOOL request10() {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == NULL) {
        return FALSE;
    }
    return request(0, player->getDemoID(), TYPE_10, 1, 0);
}

// 8018ECF0
static BOOL checkSave(Req_c *req) {
    return FALSE;
}

// 8018ECF8
static BOOL startSave(Req_c *req) {
    return TRUE;
}

// 8018ED00
static BOOL execSave(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018ED14
static BOOL endSave(Req_c *req) {
    dPlayerMgr::setDemoWait(FALSE);
    return TRUE;
}

// 8018ED3C
BOOL startNetSave() {
    if (dPlayerMgr::getPlayerActor(4) == NULL) {
        return FALSE;
    }
    if (isType(TYPE_ANY)) {
        return FALSE;
    }
    if (!dPlayerMgr::canStartNetSave()) {
        return FALSE;
    }
    if (!dPlayerMgr::isNetSaveWaitState()) {
        return FALSE;
    }
    dPlayerMgr::setDemoWait(TRUE);
    Req_c *req = dNetReqPool::alloc();
    req->mSrcID = 0;
    req->mTargetID = 0;
    req->mState = REQ_STATE_EXECUTE;
    req->mStep = STEP_START;
    req->mType = TYPE_NET_SAVE;
    sCurrent = req;
    return TRUE;
}

// 8018EDE4
BOOL endNetSave() {
    if (isType(TYPE_NET_SAVE)) {
        sCurrent->mStep = STEP_DONE;
        return TRUE;
    }
    return FALSE;
}

// 8018EE28
static BOOL checkDayChange(Req_c *req) {
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    onDayChangeReady();
    dPlayerMgr::startDemoWait();
    return FALSE;
}

// 8018EE64
static BOOL startDayChange(Req_c *req) {
    return TRUE;
}

// 8018EE6C
static BOOL execDayChange(Req_c *req) {
    return FALSE;
}

// 8018EE74
static BOOL endDayChange(Req_c *req) {
    return TRUE;
}

// 8018EE7C
BOOL requestDayChange() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(),
                            dScene::SCENE_ATTR_DEMO)) {
        return FALSE;
    }
    if (isNoDayChange()) {
        return FALSE;
    }
    offFlag(FLAG_DAY_CHANGE_READY);
    offFlag(FLAG_DAY_CHANGE_WARP);
    return requestPlayerDemo(TYPE_DAY_CHANGE);
}

// 8018EEE4
void onDayChangeReady() {
    onFlag(FLAG_DAY_CHANGE_READY);
}

// 8018EEEC
BOOL isDayChangeReady() {
    return isFlag(FLAG_DAY_CHANGE_READY);
}

// 8018EEF4
void startDayChangeWarp() {
    offFlag(FLAG_DAY_CHANGE_READY);
    onFlag(FLAG_DAY_CHANGE_WARP);
}

// 8018EF20
BOOL isDayChangeWarp() {
    return isFlag(FLAG_DAY_CHANGE_WARP);
}

// 8018EF28
void offDayChangeWarp() {
    offFlag(FLAG_DAY_CHANGE_WARP);
}

// 8018EF30
BOOL isSilence() {
    return isFlag(FLAG_SILENCE);
}

// 8018EF38
void offSilence() {
    offFlag(FLAG_SILENCE);
}

// 8018EF40
static BOOL checkEvent(Req_c *req) {
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    dPlayerMgr::startDemoWait();
    return TRUE;
}

// 8018EF78
static BOOL startEvent(Req_c *req) {
    return TRUE;
}

// 8018EF80
static BOOL execEvent(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018EF94
static BOOL endEvent(Req_c *req) {
    return dPlayerMgr::endDemoWait();
}

// 8018EF98
BOOL requestEvent() {
    return requestPlayerDemo(TYPE_EVENT);
}

// 8018EFA0
BOOL isActiveEvent() {
    return isActive(TYPE_EVENT);
}

// 8018EFA8
BOOL finishEvent() {
    return finish(TYPE_EVENT);
}

// 8018EFB0
BOOL isNoDayChange() {
    return isFlag(FLAG_NO_DAY_CHANGE);
}

// 8018EFB8: also TYPE_KEYBOARD's
static BOOL checkUitag(Req_c *req) {
    if (!dPlayerMgr::requestDemoStart()) {
        return FALSE;
    }
    dPlayerMgr::startDemoWait();
    return TRUE;
}

// 8018EFF0
static BOOL startUitag(Req_c *req) {
    return TRUE;
}

// 8018EFF8
static BOOL execUitag(Req_c *req) {
    return req->mStep == STEP_DONE;
}

// 8018F00C
static BOOL endUitag(Req_c *req) {
    return dPlayerMgr::endDemoWait();
}

// 8018F010
BOOL requestUitag() {
    return requestPlayerDemo(TYPE_UITAG);
}

// 8018F018
BOOL isActiveUitag() {
    return isActive(TYPE_UITAG);
}

// 8018F020
BOOL finishUitag() {
    return finish(TYPE_UITAG);
}

// 8018F028
BOOL requestKeyboard() {
    return requestPlayerDemo(TYPE_KEYBOARD);
}

// 8018F030
BOOL isActiveKeyboard() {
    return isActive(TYPE_KEYBOARD);
}

// 8018F038
BOOL finishKeyboard() {
    return finish(TYPE_KEYBOARD);
}

// 8018F040
static BOOL checkSpeech(Req_c *req) {
    dDemoActor_c *target = dDemoActor_c::findByDemoID(req->mTargetID);
    if (target == NULL) {
        return FALSE;
    }
    dDemoActor_c *src = dDemoActor_c::findByDemoID(req->mSrcID);
    if (!dPlayerMgr::canStartDemo()) {
        return FALSE;
    }
    req->mCheck = DEMO_CMD_SPEECH;
    if (target->canSpeech(src)) {
        req->mCheck = DEMO_CMD_SPEECH;
    } else {
        return FALSE;
    }
    return startTalkDemo(req) != FALSE;
}

// 8018F108
static BOOL startSpeech(Req_c *req) {
    return startTalk(req);
}

// 8018F10C
static BOOL execSpeech(Req_c *req) {
    return execTalk(req);
}

// 8018F110
static BOOL endSpeech(Req_c *req) {
    return endTalkDemo(req);
}

// 8018F114
BOOL requestSpeech(dDemoActor_c *src, dDemoActor_c *target) {
    return request(src->getDemoID(), target->getDemoID(), TYPE_SPEECH, 3, 0);
}

// 8018F16C
void initList() {
    sReqList.init();
    dNetReqPool::init();
    sCurrent = NULL;
    sTalkActorID = 0;
}

static const TypeFuncs l_typeFuncs[TYPE_NUM] = {
    {checkNone, startNone, execNone, endNone},
    {checkMenu, startMenu, execMenu, endMenu},
    {checkTalk, startTalk, execTalk, endTalk},
    {checkSpeak, startSpeak, execSpeak, endSpeak},
    {checkSceneStart, startSceneStart, execSceneStart, endSceneStart},
    {checkExitScene, startExitScene, execExitScene, endExitScene},
    {checkEnter, startEnter, execEnter, endEnter},
    {check7, start7, exec7, end7},
    {checkReport, startReport, execReport, endReport},
    {check9, start9, exec9, end9},
    {check10, start10, exec10, end10},
    {checkSave, startSave, execSave, endSave},
    {checkDayChange, startDayChange, execDayChange, endDayChange},
    {checkEvent, startEvent, execEvent, endEvent},
    {checkUitag, startUitag, execUitag, endUitag},
    {checkUitag, startUitag, execUitag, endUitag},
    {checkSpeech, startSpeech, execSpeech, endSpeech},
};

} // namespace dDemo

// 8018F1A0
void dDemo_c::selectNext() {
    if (sCurrent != NULL) {
        return;
    }
    for (Req_c *req = (Req_c *)sReqList.getFirst(); req != NULL;
         req = (Req_c *)req->getNext()) {
        if (l_typeFuncs[req->mType].check(req)) {
            sReqList.removeLineNode(req);
            sCurrent = req;
            req->mState = REQ_STATE_START;
            return;
        }
    }
}

// 8018F230
void dDemo_c::startCurrent() {
    if (sCurrent == NULL || sCurrent->getState() != REQ_STATE_START) {
        return;
    }

    if (l_typeFuncs[sCurrent->mType].start(sCurrent)) {
        sCurrent->mState = REQ_STATE_EXECUTE;
    }
}

// 8018F29C
void dDemo_c::execCurrent() {
    if (sCurrent == NULL || sCurrent->getState() != REQ_STATE_EXECUTE) {
        return;
    }

    if (l_typeFuncs[sCurrent->mType].execute(sCurrent)) {
        sCurrent->mState = REQ_STATE_END;
    }
}

// 8018F308
void dDemo_c::endCurrent() {
    if (sCurrent == NULL || sCurrent->getState() != REQ_STATE_END) {
        return;
    }

    if (l_typeFuncs[sCurrent->mType].end(sCurrent)) {
        clearCurrent();
    }
}

// 8018F380
void dDemo_c::clearCurrent() {
    sCurrent->clear();
    sCurrent = NULL;
}

namespace dDemo {

// 8018F3AC
BOOL request(u32 srcID, u32 targetID, u8 type, u8 priority, u8 param) {
    Req_c *req = dNetReqPool::alloc();
    if (req == NULL) {
        return FALSE;
    }
    req->mSrcID = srcID;
    req->mTargetID = targetID;
    req->mType = type;
    req->mPriority = priority;
    req->mParam = param;
    req->mStep = STEP_INIT;
    req->mCheck = 0;
    req->mState = REQ_STATE_QUEUED;
    sReqList.insertByPriority(req);
    return TRUE;
}

// 8018F438
BOOL isType(u8 type) {
    if (sCurrent == NULL) {
        return FALSE;
    }
    if (type == TYPE_ANY) {
        return TRUE;
    }
    return sCurrent->mType == type;
}

// 8018F470
dDemoActor_c *getTalkActor() {
    if (isTalking()) {
        return dDemoActor_c::findByDemoID(sTalkActorID);
    }
    return NULL;
}

// 8018F4A8
BOOL isTalking() {
    BOOL ret = FALSE;
    if (isType(TYPE_TALK) || isType(TYPE_SPEAK) || isType(TYPE_REPORT) ||
        isType(TYPE_SPEECH)) {
        ret = TRUE;
    }
    return ret;
}

} // namespace dDemo
