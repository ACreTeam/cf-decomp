#include <game/game/d_menu_map02.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/snd/snd_id.hpp>

namespace dMenuMap02 {
// .text:0x5C
BASE_PROFILE(MENU_MAP02, dMenuMap02_c)
} // namespace dMenuMap02

// .text:0x290
int dMenuMap02_c::create() {
    int loaded = load();
    loaded &= mBtn.load();
    if (!loaded) {
        return NOT_READY;
    }
    createLayouts();
    mBtn.create();
    initButtons();
    initSpots();
    startAnm(&mWinLyt, "N_all", ANM_WIN_IN, STATE_MAIN, TRUE);
    mWinLyt.calc();
    return SUCCEEDED;
}

// .text:0x338
int dMenuMap02_c::doDelete() {
    if (mBtn.remove() == FALSE) {
        return NOT_READY;
    }
    return remove();
}

// .text:0x380
int dMenuMap02_c::stateMain() {
    preSubState();
    executeSubState();
    postSubState();
    return SUCCEEDED;
}

// .text:0x3C0
int dMenuMap02_c::stateAnm() {
    int result = dMenuBase_c::stateAnm();
    mBtn.setPos(&mWinLyt, "N_all");
    onFlag(FLAG_SPOT_POS);
    return result;
}

// .text:0x41C
int dMenuMap02_c::execute() {
    int result = dMenuMap_c::execute();
    mBtn.execute();
    return result;
}

// .text:0x460
int dMenuMap02_c::draw() {
    int result = dMenuMap_c::draw();
    if (isFlag(FLAG_DRAW)) {
        mBtn.draw();
        calcSpots();
    }
    return result;
}

// .text:0x4C0
void dMenuMap02_c::subStateMain() {
    if (mCursor != SLOT_NONE) {
        setSelect(mCursor, FALSE);
    }
    int btn = mBtn.checkPad(TRUE, TRUE);
    if (btn != -1) {
        mCursor = btn + SLOT_BTN;
        setSelect(mCursor, TRUE);
        push();
    } else {
        mCursor = hitCheckCursor();
        if (mCursor != SLOT_NONE) {
            setSelect(mCursor, TRUE);
            if (mPad::g_currentCore->downTrigger(WPAD_BUTTON_A)) {
                push();
            }
        }
    }
}

// .text:0x584
void dMenuMap02_c::subStatePush() {
    if (isPushEnd()) {
        onPushEnd();
    }
}

// .text:0x5C0
void dMenuMap02_c::executeSubState() {
    typedef void (dMenuMap02_c::*Proc)();
    static Proc subStateProcs[] = {&dMenuMap02_c::subStateMain,
                                   &dMenuMap02_c::subStatePush};
    (this->*subStateProcs[mSubState])();
}

// .text:0x5F8
void dMenuMap02_c::preSubState() {
}

// .text:0x5FC
void dMenuMap02_c::postSubState() {
}

// .text:0x600
void dMenuMap02_c::setSelect(int slot, BOOL on) {
    if (isBtn(slot)) {
        mBtn.setSelect(slot - SLOT_BTN, on, FALSE);
    }
    if (isSpot(slot)) {
        mSpotAnm[slot - SLOT_SPOT].setNext(on == FALSE);
    }
}

// .text:0x6B0
int dMenuMap02_c::hitCheckCursor() {
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    mVec2_c pos = dObjCursor_c::getPos();
    int btn = mBtn.hitCheck(&pos);
    if (btn != -1) {
        return btn + SLOT_BTN;
    }
    for (int i = 0; i < SPOT_NUM; i++) {
        nw4r::lyt::Pane *pane = mSpotLyt[i].findPane("B_mapCursor");
        if (m2d::isVisible(pane) && m2d::isInPaneRect(pane, 0.0f, &pos)) {
            return i + SLOT_SPOT;
        }
    }
    return SLOT_NONE;
}

// .text:0x778
void dMenuMap02_c::push() {
    if (isBtn(mCursor)) {
        mBtn.push(mCursor - SLOT_BTN, FALSE);
    } else if (isSpot(mCursor)) {
        mSpotAnm[mCursor - SLOT_SPOT].setNext(SPOT_ANM_PUSH);
        playSePush();
    } else {
        return;
    }
    setSubState(SUB_PUSH);
}

// .text:0x838
BOOL dMenuMap02_c::isPushEnd() {
    if (isBtn(mCursor)) {
        return !mBtn.isPushing();
    }
    if (isSpot(mCursor)) {
        return mSpotAnm[mCursor - SLOT_SPOT].isEnd();
    }
    return TRUE;
}

// .text:0x8C8
void dMenuMap02_c::onPushEnd() {
    int slot = mCursor;
    if (isBtn(slot)) {
        startClose();
    } else if (isSpot(slot)) {
        startClose();
        dMenuMng_c::setResult(dMenuMng_c::RESULT_OK);
        dMenuMng_c::setPickedBlock(mSpotBlockX[slot - SLOT_SPOT],
                                   mSpotBlockZ[slot - SLOT_SPOT]);
    } else {
        setSubState(SUB_MAIN);
    }
}

// .text:0x98C
void dMenuMap02_c::startClose() {
    startAnm(&mWinLyt, "N_all", ANM_WIN_OUT, STATE_CLOSE, TRUE);
    dMenuMng_c::setResult(dMenuMng_c::RESULT_CANCEL);
}

// .text:0x9CC
void dMenuMap02_c::initButtons() {
    mBtn.setVisible(BTN_UNUSED_0, false);
    mBtn.setVisible(BTN_UNUSED_1, false);
    mBtn.setText(BTN_BACK, MSG_BACK, TRUE);
}

// .text:0xA28
void dMenuMap02_c::initSpots() {
    static const char *spotAnms[] = {
        "map_cursor_00_cursor_In.brlan",
        "map_cursor_00_cursor_Out.brlan",
        "map_cursor_00_cursor_Psh.brlan",
    };
    int i;
    int n;
    int x;
    int z;
    for (i = 0; i < SPOT_NUM; i++) {
        mSpotLyt[i].build(&mRes, "map_cursor_00.brlyt");
        mSpotLyt[i].calc();
        mSpotLyt[i].setVisible("N_all", false);
        mSpotLyt[i].setVisible("N_houseCursor", false);
        mSpotAnm[i].init(&mSpotLyt[i], &mRes, spotAnms, SPOT_ANM_NUM);
        mSpotAnm[i].setNext(SPOT_ANM_OUT);
        mSpotBlockZ[i] = NO_BLOCK;
        mSpotBlockX[i] = NO_BLOCK;
    }
    dFdBase_c *field = dField::getInfo(FD_ID_TOWN);
    f32 scaleX = MAP_SIZE / f32(field->mBlockW - 2);
    f32 scaleZ = MAP_SIZE / f32(field->mBlockH - 2);
    f32 half = 0.5f;
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    n = 0;
    for (z = 1; z < field->mBlockH - 1 && n < SPOT_NUM; z++) {
        for (x = 1; x < field->mBlockW - 1 && n < SPOT_NUM; x++) {
            if (field->isBlockVariant(x, z)) {
                mSpotBlockX[n] = x;
                mSpotBlockZ[n] = z;
                pos.x = (x - 1) * scaleX + half * (scaleX - MAP_SIZE);
                pos.y = -f32(z - 1) * scaleZ - half * (scaleZ - MAP_SIZE);
                mSpotLyt[n].findPane("N_mapCursor")->SetTranslate(pos);
                mSpotLyt[n].setVisible("N_all", true);
                n++;
            }
        }
    }
}

// .text:0xCC4
void dMenuMap02_c::calcSpots() {
    int i;
    for (i = 0; i < SPOT_NUM; i++) {
        if (mSpotAnm[i].play()) {
            onFlag(FLAG_SPOT_CALC);
        }
    }
    if (isFlag(FLAG_SPOT_POS)) {
        offFlag(FLAG_SPOT_POS);
        for (i = 0; i < SPOT_NUM; i++) {
            mWinLyt.copyPos(&mSpotLyt[i], "map_00", "N_all");
        }
        onFlag(FLAG_SPOT_CALC);
    }
    if (isFlag(FLAG_SPOT_CALC)) {
        offFlag(FLAG_SPOT_CALC);
        for (i = 0; i < SPOT_NUM; i++) {
            mSpotLyt[i].calc();
        }
    }
    for (i = 0; i < SPOT_NUM; i++) {
        mSpotLyt[i].entry();
    }
}

// .text:0xDF4
void dMenuMap02_c::playSePush() {
    playSe(SE_BRIDGE_Choice);
}
