// d_obj_tutorial.cpp: the tutorial balloon process. .text 801B6A88..801B700C, .ctors 80465848,
// .data 804FC7B0..804FC850, .bss 80625830..80625940, .sdata 8074C2E0..8074C300, .sbss 8074EB58..8074EB70.
// See include/game/game/d_obj_tutorial.hpp. Names other than the class names are inferred.
#include <game/game/d_obj_tutorial.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>

#define NO_REQUEST -1
#define NO_PRIORITY 0xFE

CUSTOM_BASE_PROFILE(OBJ_TUTORIAL, dObjTutorial_c, 0x126, 0x121);

// The requested balloon: set by request(), shown by update().
static int l_reqA = NO_REQUEST;
static int l_reqB = NO_REQUEST;
static u8 l_priority = NO_PRIORITY;
static u8 l_hideReq;
static mVec2_c l_pos;
static int l_tail;
static int l_color;
static m2d::stringWord_c l_word;

static void (dObjTutorial_c::*l_stateFuncs[])() = {
    &dObjTutorial_c::stateWait,
};

// 801B6ADC
int dObjTutorial_c::create() {
    bool loaded = mBalloon.loadArc();
    if (!loaded) {
        return NOT_READY;
    }
    mBalloon.build();
    mBalloon.onFlag(dBalloon_c::FLAG_1);
    mStateIdx = 0;
    l_reqA = NO_REQUEST;
    l_reqB = NO_REQUEST;
    l_hideReq = 0;
    l_priority = NO_PRIORITY;
    return SUCCEEDED;
}

// 801B6B5C
int dObjTutorial_c::doDelete() {
    bool ok = mBalloon.remove();
    return ok;
}

// 801B6B8C
int dObjTutorial_c::execute() {
    (this->*l_stateFuncs[mStateIdx])();
    update();
    return SUCCEEDED;
}

// 801B6BDC
int dObjTutorial_c::draw() {
    mBalloon.draw();
    return SUCCEEDED;
}

// 801B6C04
void dObjTutorial_c::stateWait() {}

// 801B6C08
void dObjTutorial_c::update() {
    if (l_priority != NO_PRIORITY) {
        mBalloon.setPriority(l_priority);
        l_priority = NO_PRIORITY;
    }
    if (l_hideReq) {
        l_hideReq = 0;
        l_reqA = NO_REQUEST;
    }
    switch (mBalloon.getState()) {
        case dBalloon_c::STATE_HIDE:
            if (l_reqA != NO_REQUEST) {
                mBalloon.setColor(l_color);
                mBalloon.setTail(l_tail);
                mBalloon.setWord(&l_word);
                mBalloon.setPos(l_pos);
                mShownA = l_reqA;
                mShownB = l_reqB;
                mBalloon.setNextState(dBalloon_c::STATE_OPEN);
            }
            break;
        case dBalloon_c::STATE_OPEN:
            break;
        case dBalloon_c::STATE_SHOW:
            if (l_reqA != mShownA || l_reqB != mShownB) {
                mBalloon.setNextState(dBalloon_c::STATE_CLOSE);
            }
            break;
        case dBalloon_c::STATE_CLOSE:
            break;
    }
    mBalloon.execute();
}

// 801B6D24
void dObjTutorial_c::hide() {
    l_hideReq = 1;
}

// 801B6D30
void dObjTutorial_c::setWord(const mVec2_c *pos, const dScript::Word_c *word, int tail) {
    l_pos = *pos;
    l_tail = tail;
    l_color = 0;
    l_word.clear();
    l_word.copy(word, 0);
}

// 801B6D9C
void dObjTutorial_c::setMsg(const mVec2_c *pos, u16 msgId, int tail) {
    l_pos = *pos;
    l_tail = tail;
    l_color = 1;
    l_word.clear();
    if (msgId == MENU_MSG_0) {
        dString::loadString(&l_word, 3, "sys_2D/SYS_2D_menu");
    } else if (msgId == MENU_MSG_1) {
        dString::loadString(&l_word, 0xEA, "sys_2D/SYS_2D_menu");
    } else if (msgId == MENU_MSG_2) {
        dString::loadString(&l_word, 5, "sys_2D/SYS_2D_menu");
    } else {
        if (msgId == TOWN_MSG) {
            dSaveData_c *raw = dSaveData_c::getRaw();
            dString::setTownWord(0, &raw->mLandID);
        }
        dString::loadString(&l_word, msgId, "sys_2D/SYS_2D_UIballoon");
    }
}

// 801B6E90
BOOL dObjTutorial_c::request(int a, int b, BOOL check) {
    if (check && isEnabled() == FALSE) {
        return FALSE;
    }
    l_hideReq = 0;
    if (l_reqA == a && l_reqB == b) {
        return FALSE;
    }
    l_reqA = a;
    l_reqB = b;
    return TRUE;
}

// 801B6F14
void dObjTutorial_c::setPriority(u8 priority) {
    l_priority = priority;
}

// 801B6F1C
BOOL dObjTutorial_c::isEnabled() {
    return dSaveOption::isTutorialOff() == 0;
}

// 801B6F44
dObjTutorial_c::~dObjTutorial_c() {}
