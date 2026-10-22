#include <game/game/d_menu_star.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <revolution/sc.h>

// .text:0x5C
BOOL dMenuStar_c::load() {
    BOOL loaded = TRUE;
    if (!mRes.load("Layout/star.arc")) {
        loaded = FALSE;
    }
    if (!mOrder.create(&mStar)) {
        loaded = FALSE;
    }
    return loaded;
}

// .text:0xC4
void dMenuStar_c::create() {
    mWinLyt.build(&mRes, "star_win_00.brlyt");
    initAnms();
    mWinLyt.calc();
    mWinLyt.mPriority = LYT_PRIORITY;
    mCursorLyt.build(&mRes, "star_cursor_00.brlyt");
    mCursorLyt.mPriority = LYT_PRIORITY;
    mZClear.set(-228.0f, 228.0f, -304.0f, 304.0f, -2.0f);
    mFlags |= FLAG_CALC;
    dCamera::useMainFovy();
    mOrder.init();
    mTargetScale = 1.0f;
}

// .text:0x190
void dMenuStar_c::draw() {
    mZClear.entry();
    if (mFlags & FLAG_CALC) {
        mFlags &= ~FLAG_CALC;
        mWinLyt.calc();
        mVec3_c pos = m2d::getTotalPos(mWinLyt.findPane("N_star_win_all"));
        f32 halfWidth = 200.0f;
        if (SCGetAspectRatio() != SC_ASPECT_RATIO_16x9) {
            halfWidth *= 1.368421f;
        }
        mWinZClear.set(250.0 + pos.y, -250.0f + pos.y, -halfWidth + pos.x,
                       halfWidth + pos.x, -499.9f);
    }
    mWinZClear.entry();
    if (mFlags & FLAG_DRAW_CURSOR) {
        mFlags &= ~FLAG_DRAW_CURSOR;
        mCursorLyt.entry();
    }
    mWinLyt.entry();
    m2d::addCallback(&mOrder);
}

// .text:0x2A8
BOOL dMenuStar_c::remove() {
    BOOL removed = TRUE;
    if (!mRes.remove()) {
        removed = FALSE;
    }
    if (!mOrder.unload()) {
        removed = FALSE;
    }
    if (removed == TRUE) {
        dCamera::resetSecondaryFovy();
    }
    return removed;
}

// .text:0x310
void dMenuStar_c::execute() {
    calcAnms();
    calcScale();
    calcTurn();
    mOrder.calc();
}

// .text:0x354
void dMenuStar_c::startWinAnm(int anm) {
    static const char *winAnmNames[WIN_ANM_NUM] = {
        "star_win_00_star_win_TrnsIn.brlan",
        "star_win_00_star_win_TrnsOut.brlan",
    };
    mWinLyt.setAnm(&mWinAnm, winAnmNames[anm], nullptr);
    mFlags |= FLAG_WIN_ANM;
}

// .text:0x3A8
BOOL dMenuStar_c::isWinAnmEnd() {
    return !(mFlags & FLAG_WIN_ANM);
}

// .text:0x3BC
bool dMenuStar_c::setCursorAnm(int anm) {
    return mCursorAnm.setNext(anm);
}

// .text:0x3C4
BOOL dMenuStar_c::isCursorAnmEnd() {
    return mCursorAnm.isEnd();
}

// .text:0x3CC
void dMenuStar_c::initAnms() {
    static const char *btnAnmNames[BTN_ANM_NUM] = {
        "star_win_00_crsr_In.brlan",    "star_win_00_crsr_Out.brlan",
        "star_win_00_crsr_Psh.brlan",   "star_win_00_crsr_CanPsh.brlan",
        "star_win_00_crsr_NoPsh.brlan",
    };
    static const char *cursorAnmNames[CURSOR_ANM_NUM] = {
        "star_win_00_crsr_Fnsh.brlan",
        "star_win_00_crsr_Strt.brlan",
    };
    for (int i = 0; i < BTN_NUM; i++) {
        mBtnAnm[i].init(&mWinLyt, &mRes, btnAnmNames, BTN_ANM_NUM);
        mBtnAnm[i].setNumber(i);
        mBtnAnm[i].setNext(BTN_ANM_OUT);
    }
    mCursorAnm.init(&mWinLyt, &mRes, cursorAnmNames, CURSOR_ANM_NUM);
    mCursorAnm.start(CURSOR_ANM_START);
    mCursorAnm.setToEnd();
}

// .text:0x488
void dMenuStar_c::calcAnms() {
    BOOL calc = FALSE;
    if (mFlags & FLAG_WIN_ANM) {
        if (mWinAnm.mFrameCtrl.isStop()) {
            mFlags &= ~FLAG_WIN_ANM;
        } else {
            mWinAnm.play();
        }
        calc = TRUE;
    }
    for (int i = 0; i < BTN_NUM; i++) {
        calc |= mBtnAnm[i].play();
    }
    calc |= mCursorAnm.play();
    if (calc) {
        mFlags |= FLAG_CALC;
    }
}

// .text:0x544
int dMenuStar_c::hitCheck(const mVec2_c *pos) {
    mVec2_c p = *pos;
    int btn = m2d::hitButton(&p, &mWinLyt, 0, BTN_NUM - 1, -1, -1, 1.0f);
    if (btn == BTN_ZOOM_IN && !canZoomIn()) {
        btn = BTN_NONE;
    }
    if (btn == BTN_ZOOM_OUT && !canZoomOut()) {
        btn = BTN_NONE;
    }
    return btn;
}

// .text:0x5E8
void dMenuStar_c::setSelect(int btn, BOOL select) {
    mBtnAnm[btn].setNext(select == FALSE);
}

// .text:0x600
BOOL dMenuStar_c::isBtnAnmEnd(int btn) {
    return mBtnAnm[btn].isEnd();
}

// .text:0x610
void dMenuStar_c::push(int btn) {
    mBtnAnm[btn].setNext(BTN_ANM_PUSH);
}

// .text:0x624
void dMenuStar_c::move(int btn) {
    BOOL playSe = TRUE;
    // This unused snapshot creates the shared pointer temporary before the
    // angle capture. MWCC removes the rate load later; keep the capture order.
    f32 zoomRate = mOrder.mpStar->mZoomRate;
    f32 zoom = mOrder.mpStar->mZoom;
    s16 angle = mOrder.mAngle;
    switch (btn) {
    case BTN_ZOOM_IN:
        if (zoom <= -0.55f) {
            mBtnAnm[BTN_ZOOM_OUT].startEnd(BTN_ANM_CAN_PUSH);
            mFlags |= FLAG_CALC;
        }
        if (zoom >= 0.85f) {
            zoom = 0.85f;
            playSe = FALSE;
        } else {
            zoom += 0.02f;
            if (zoom >= 0.85f) {
                zoom = 0.85f;
                playSe = FALSE;
                mBtnAnm[BTN_ZOOM_IN].startEnd(BTN_ANM_OUT);
                mWinLyt.calc();
                mBtnAnm[BTN_ZOOM_IN].startEnd(BTN_ANM_NO_PUSH);
                mFlags |= FLAG_CALC;
            }
        }
        mOrder.mpStar->mZoom = zoom;
        break;
    case BTN_ZOOM_OUT:
        if (zoom >= 0.85f) {
            mBtnAnm[BTN_ZOOM_IN].startEnd(BTN_ANM_CAN_PUSH);
            mFlags |= FLAG_CALC;
        }
        if (zoom <= -0.55f) {
            zoom = -0.55f;
            playSe = FALSE;
        } else {
            zoom -= 0.02f;
            if (zoom <= -0.55f) {
                zoom = -0.55f;
                playSe = FALSE;
                mBtnAnm[BTN_ZOOM_OUT].startEnd(BTN_ANM_OUT);
                mWinLyt.calc();
                mBtnAnm[BTN_ZOOM_OUT].startEnd(BTN_ANM_NO_PUSH);
                mFlags |= FLAG_CALC;
            }
        }
        mOrder.mpStar->mZoom = zoom;
        break;
    case BTN_TURN_LEFT:
        mOrder.mAngle = angle + TURN_STEP;
        break;
    case BTN_TURN_RIGHT:
        mOrder.mAngle = angle - TURN_STEP;
        break;
    }
    if (playSe) {
        holdSe(SE_Lev_Seiza_DispMove);
    }
}

// .text:0x818
bool dMenuStar_c::canZoomIn() {
    return mOrder.mpStar->mZoom < 0.85f;
}

// .text:0x838
bool dMenuStar_c::canZoomOut() {
    return mOrder.mpStar->mZoom > -0.55f;
}

// .text:0x858
void dMenuStar_c::calcScale() {
    f32 scale = mOrder.mScale;
    if (mTargetScale != scale) {
        sLib::addCalc(&scale, mTargetScale, 0.09f, 0.5f, 0.025f);
        mOrder.mScale = scale;
    }
}

// .text:0x8B8
void dMenuStar_c::setAngle(const dStarSign_c *sign) {
    mTargetAngle = dStarDraw_c::getSignAngle(sign);
    mOrder.mAngle = mTargetAngle;
}

// .text:0x8F4
void dMenuStar_c::startTurn(const dStarSign_c *sign) {
    mTargetAngle = dStarDraw_c::getSignAngle(sign);
    mFlags |= FLAG_TURN;
}

// .text:0x938
bool dMenuStar_c::isTurning() {
    u16 turn = mFlags & FLAG_TURN;
    return turn;
}

// .text:0x950
void dMenuStar_c::calcTurn() {
    if (mFlags & FLAG_TURN) {
        mAng angle = mOrder.mAngle;
        s16 left =
            sLib::addCalcAngle(&angle.mAngle, mTargetAngle, 4, 1000, 400);
        mOrder.setAngle(angle);
        if (left == 0) {
            mFlags &= ~FLAG_TURN;
        }
    }
}

// .text:0x9C0
void dMenuStar_c::setCursorPos(const mVec2_c *pos) {
    mFlags |= FLAG_DRAW_CURSOR;
    mCursorLyt.mPos.x = pos->x;
    mCursorLyt.mPos.y = pos->y;
    mCursorLyt.calc();
}

// .text:0x9F0
BOOL dMenuStar_c::starOrder_c::create(dStarDraw_c *star) {
    mpStar = star;
    if (star->create(dHeap::localHeap_p)) {
        mpStar->mMode = dStarDraw_c::MODE_CHART;
        return TRUE;
    }
    return FALSE;
}

// .text:0xA50
bool dMenuStar_c::starOrder_c::unload() {
    return mpStar->unload();
}

// .text:0xA58
void dMenuStar_c::starOrder_c::init() {
    mScale = 1.0f;
    mpStar->mZoom = 0.0f;
    mAngle = dStarDraw_c::getSkyAngle(dTime_c::getCurrent());
}

// .text:0xAAC
void dMenuStar_c::starOrder_c::calc() {
    mpStar->mZoomRate = mScale;
    mVec3_c pos(0.0f, -460.0f, -1300.0f);
    mpStar->calc(&pos, getAngle());
    mpStar->mAlpha = 0xFF;
}

// .text:0xB24
void dMenuStar_c::starOrder_c::execute() {
    mpStar->setPriorityDraw(2, 3);
    mpStar->entry();
}
