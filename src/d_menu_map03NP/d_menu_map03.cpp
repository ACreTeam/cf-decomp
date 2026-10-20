#include <game/game/d_menu_map03.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/mLib/m_pad.hpp>

namespace dMenuMap03 {
// .text:0x5C
BASE_PROFILE(MENU_MAP03, dMenuMap03_c)
} // namespace dMenuMap03

// .text:0x290
int dMenuMap03_c::create() {
    BOOL mapLoaded = load();
    BOOL loaded = mapLoaded & mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    createLayouts();
    mBtn.create();
    initButtons();
    initFrames();
    startAnm(&mWinLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mWinLyt.calc();
    return SUCCEEDED;
}

// .text:0x338
int dMenuMap03_c::doDelete() {
    if (mBtn.remove() == false) {
        return NOT_READY;
    }
    return remove();
}

// .text:0x380
int dMenuMap03_c::stateMain() {
    beginSubState();
    executeSubState();
    endSubState();
    return SUCCEEDED;
}

// .text:0x3C0
int dMenuMap03_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mWinLyt, "N_all");
    onFlag(FLAG_COPY_POS);
    return result;
}

// .text:0x41C
int dMenuMap03_c::execute() {
    int result = dMenuMap_c::execute();
    mBtn.execute();
    return result;
}

// .text:0x460
int dMenuMap03_c::draw() {
    int result = dMenuMap_c::draw();
    if (isFlag(FLAG_DRAW)) {
        mBtn.draw();
        calcFrames();
    }
    return result;
}

// .text:0x4C0
void dMenuMap03_c::subStateMain() {
    mBtn.clearSelect();
    BOOL pushed = mPad::g_currentCore->downTrigger(WPAD_BUTTON_A);
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        pushed = TRUE;
    }
    if (btn == -1) {
        btn = hitCheckBtn();
    }
    if (btn != -1) {
        mCursor = SLOT_BTN + btn;
        mBtn.setSelect(btn, TRUE, FALSE);
        if (pushed) {
            mBtn.push(btn, FALSE);
            setSubState(SUB_PUSH);
        }
    } else {
        mCursor = SLOT_NONE;
    }
}

// .text:0x5B4
void dMenuMap03_c::subStatePush() {
    if (!mBtn.isPushing()) {
        startAnm(&mWinLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    }
}

// .text:0x60C
void dMenuMap03_c::executeSubState() {
    typedef void (dMenuMap03_c::*Proc)();
    static Proc subStateProcs[SUB_NUM] = {
        &dMenuMap03_c::subStateMain,
        &dMenuMap03_c::subStatePush,
    };
    (this->*subStateProcs[mSubState])();
}

// .text:0x644
void dMenuMap03_c::beginSubState() {
}

// .text:0x648
void dMenuMap03_c::endSubState() {
}

// .text:0x64C
void dMenuMap03_c::initButtons() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_OK, MSG_OK, false);
}

// .text:0x6A8
int dMenuMap03_c::hitCheckBtn() {
    mVec2_c pos = dObjCursor_c::getPos();
    return mBtn.hitCheck(&pos);
}

// .text:0x6E4
void dMenuMap03_c::initFrames() {
    int i;
    static const char *frameAnms[FRAME_ANM_NUM] = {
        "map_cursor_00_cursor_In.brlan",
        "map_cursor_00_cursor_Out.brlan",
        "map_cursor_00_cursor_Psh.brlan",
    };
    for (i = 0; i < HOUSE_NUM; i++) {
        mFrame[i].build(&mRes, "map_cursor_00.brlyt");
        mFrame[i].setVisible("N_mapCursor", false);
        mFrameAnm[i].init(&mFrame[i], &mRes, frameAnms, FRAME_ANM_NUM);
        mFrameAnm[i].setNext(FRAME_ANM_OUT);
        mVec3_c pos = mIcons[i].findPane("N_mapIcn_all")->GetTranslate();
        mFrame[i].findPane("N_houseCursor")->SetTranslate(pos);
        mFrame[i].calc();
    }
    for (i = 0; i < HOUSE_NUM; i++) {
        dPrivateData_c *player =
            dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (player != nullptr && player->mPID.isValid()) {
            mFrame[i].setVisible("N_all", false);
        }
    }
}

// .text:0x838
void dMenuMap03_c::calcFrames() {
    int i;
    for (i = 0; i < HOUSE_NUM; i++) {
        if (mFrameAnm[i].play()) {
            onFlag(FLAG_CALC_FRAME);
        }
    }
    if (isFlag(FLAG_COPY_POS)) {
        offFlag(FLAG_COPY_POS);
        for (i = 0; i < HOUSE_NUM; i++) {
            mWinLyt.copyPos(&mFrame[i], "map_00", "N_all");
        }
        onFlag(FLAG_CALC_FRAME);
    }
    if (isFlag(FLAG_CALC_FRAME)) {
        offFlag(FLAG_CALC_FRAME);
        for (i = 0; i < HOUSE_NUM; i++) {
            mFrame[i].calc();
        }
    }
    for (i = 0; i < HOUSE_NUM; i++) {
        mFrame[i].entry();
    }
}
