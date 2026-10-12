// d_money.cpp: the pocket money display. .text 8019BD78..8019C414, .data 804FAB68..804FAC60,
// .sdata 8074C110..8074C128, .sbss 8074EAA8..8074EAB0. See include/game/game/d_money.hpp.
// Names other than the class name are inferred.
#include <game/game/d_money.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/WPAD/WPAD.h>

#define MONEY_ANM_NUM 2
#define MONEY_ANM_IN 0
#define MONEY_ANM_OUT 1
#define MONEY_DEMO_TYPE 0x11
#define MONEY_DIGITS 12
#define MONEY_COUNT_FRAMES 30
#define MONEY_SE_COUNT 0x16DB
#define MONEY_SE_COUNT_END 0x16DC

CUSTOM_BASE_PROFILE(MONEY, dMoney_c, 6, 0x125);

static const char *l_anmNames[MONEY_ANM_NUM] = {
    "inStore_money_00_moneyIn.brlan",
    "inStore_money_00_moneyOut.brlan",
};

static u32 l_flags;

// 8019BDF4
int dMoney_c::create() {
    if (!mResAcc.load("Layout/money.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mResAcc, "inStore_money_00.brlyt");
    mAnm.init(&mLyt, &mResAcc, l_anmNames, MONEY_ANM_NUM);
    mAnm.start(MONEY_ANM_OUT);
    mAnm.setToEnd();
    l_flags = 0;
    resetMoney();
    l_flags |= FLAG_CALC;
    return SUCCEEDED;
}

static void (dMoney_c::*l_stateFuncs[])() = {
    &dMoney_c::stateHidden,
    &dMoney_c::stateShown,
};

// 8019BE9C
int dMoney_c::doDelete() {
    bool ok = mResAcc.remove();
    return ok;
}

// 8019BECC
int dMoney_c::execute() {
    updateMoney();
    if (mAnm.play()) {
        l_flags |= FLAG_CALC;
        return SUCCEEDED;
    }
    (this->*l_stateFuncs[mState])();
    return SUCCEEDED;
}

// 8019BF40
int dMoney_c::draw() {
    if (l_flags & FLAG_CALC) {
        mLyt.calc();
    }
    if (l_flags & FLAG_SHOW) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

// 8019BFA0
void dMoney_c::stateShown() {
    if (!isShow()) {
        mAnm.setNext(MONEY_ANM_OUT);
        mState = STATE_HIDDEN;
    }
}

// 8019BFE8
void dMoney_c::stateHidden() {
    l_flags &= ~FLAG_SHOW;
    if (isShow()) {
        if (!isSceneAttr(getCurrentScene(), SCENE_ATTR_FACILITY)) {
            resetMoney();
        }
        l_flags |= FLAG_SHOW;
        mAnm.setNext(MONEY_ANM_IN);
        mState = STATE_SHOWN;
    }
}

// 8019C068
BOOL dMoney_c::isShow() {
    if (l_flags & FLAG_DISABLE) {
        return FALSE;
    }
    if (!fn_8019A5E4()) {
        return FALSE;
    }
    if (l_flags & FLAG_FORCE_SHOW) {
        return TRUE;
    }
    if (!isSceneAttr(getCurrentScene(), SCENE_ATTR_FACILITY)) {
        return FALSE;
    }
    if (fn_8018F438(MONEY_DEMO_TYPE) && fn_8018F4A8()) {
        return TRUE;
    }
    if (fn_801B3D70()) {
        return FALSE;
    }
    return !mPad::g_currentCore->down(WPAD_BUTTON_B);
}

// 8019C138
int dMoney_c::getMoney() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return 0;
    }
    return player->getPocketMoney();
}

// 8019C16C
void dMoney_c::setNumber() {
    mLyt.setNumberSep("T_money_00", mDispMoney, MONEY_DIGITS, dScript::NUM_FORMAT_REGION);
    l_flags |= FLAG_CALC;
}

// 8019C1B0
void dMoney_c::resetMoney() {
    mDispMoney = getMoney();
    mCountTimer = 0;
    setNumber();
}

// 8019C1F0
void dMoney_c::updateMoney() {
    int money = getMoney();
    if (mCountTimer > 0) {
        if (--mCountTimer == 0) {
            mDispMoney = money;
            if (l_flags & FLAG_SHOW) {
                playSe(MONEY_SE_COUNT_END);
            }
        } else {
            if (mDispMoney < money) {
                mDispMoney += mCountStep;
            } else {
                mDispMoney -= mCountStep;
            }
            if (l_flags & FLAG_SHOW) {
                fn_8000F928(MONEY_SE_COUNT);
            }
        }
        setNumber();
    } else if (money != mDispMoney) {
        if (isSceneAttr(getCurrentScene(), SCENE_ATTR_FACILITY) || (l_flags & FLAG_FORCE_SHOW)) {
            if (fn_8019A5E4()) {
                int diff;
                if (money > mDispMoney) {
                    diff = money - mDispMoney;
                } else {
                    diff = mDispMoney - money;
                }
                mCountTimer = MONEY_COUNT_FRAMES;
                mCountStep = diff / (mCountTimer - 1);
                if (mCountStep % 5 == 0) {
                    mCountStep--;
                }
                if (mCountStep <= 1) {
                    mCountTimer = diff;
                    mCountStep = 1;
                }
            }
        }
    }
}

// 8019C34C
void dMoney_c::onForceShow() {
    l_flags |= FLAG_FORCE_SHOW;
}

// 8019C35C
void dMoney_c::offForceShow() {
    l_flags &= ~FLAG_FORCE_SHOW;
}

// 8019C36C
void dMoney_c::onDisable() {
    l_flags |= FLAG_DISABLE;
}

// 8019C37C
void dMoney_c::offDisable() {
    l_flags &= ~FLAG_DISABLE;
}
