#include <game/game/d_menu_keytest.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>

namespace dMenuKeytest {
// .text:0x5C
BASE_PROFILE(MENU_KEYTEST, dMenuKeytest_c)
} // namespace dMenuKeytest

// .text:0xD8
int dMenuKeytest_c::create() {
    mKbd.init(dKbd::Mng_c::INIT_MODE_PAD);
    BOOL kbdLoaded = mKbd.loadRes();
#if VERSION == VERSION_RUUK01_01
    BOOL loaded = kbdLoaded & mRes.load("Layout/keyboard_free.arc");
#else
    BOOL loaded = kbdLoaded & mRes.load("Layout/keyboard_add.arc");
#endif
    if (!loaded) {
        return NOT_READY;
    }
    mKbd.createDirect();
    mLyt.build(&mRes, "textBox_free_00.brlyt");
    mLyt.calc();
    mLyt.findTextBox("T_textBox_00")->AllocStringBuffer(TEXT_SIZE);
    setState(STATE_MAIN);
    setSubState(SUB_OPEN);
    return SUCCEEDED;
}

// .text:0x1C0
int dMenuKeytest_c::doDelete() {
    if (!mKbd.remove(dKbd::Mng_c::REMOVE_MODE_SAVE_SETTING)) {
        return NOT_READY;
    }
    bool removed = mRes.remove();
    return removed;
}

// .text:0x218
int dMenuKeytest_c::stateMain() {
    calcKeyTest();
    if (!isFlag(FLAG_PAUSE)) {
        executeSubState();
    }
    calcKbd();
    return SUCCEEDED;
}

// .text:0x26C
int dMenuKeytest_c::draw() {
    mKbd.entry();
    mLyt.entry();
    return SUCCEEDED;
}

// .text:0x2A8
void dMenuKeytest_c::subStateOpen() {
    mKbd.open(0, 0);
    if (mKbd.isOK()) {
        mKbd.setMemoMode2();
        setSubState(SUB_CHECK);
    } else if (mKbd.isCancel()) {
        mKbd.setMemoMode2();
        setSubState(SUB_CHECK);
    }
}

// .text:0x344
void dMenuKeytest_c::subStateCheck() {
    if (mKbd.isMemoEmpty()) {
        setState(STATE_CLOSE);
        dDemo::finishMenu();
    }
}

// .text:0x38C
void dMenuKeytest_c::subStateIdle() {
}

// .text:0x390
void dMenuKeytest_c::executeSubState() {
    typedef void (dMenuKeytest_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuKeytest_c::subStateOpen,
        &dMenuKeytest_c::subStateCheck,
        &dMenuKeytest_c::subStateIdle,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x3C8
void dMenuKeytest_c::calcKeyTest() {
}

// .text:0x3CC
void dMenuKeytest_c::calcKbd() {
    mKbd.calc();
}
