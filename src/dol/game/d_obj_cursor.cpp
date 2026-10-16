#include <game/framework/f_profile_name.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/revolution/WPAD/WPAD.h>

CUSTOM_BASE_PROFILE(OBJ_CURSOR, dObjCursor_c, fProf::OBJ_CURSOR,
                    fProf::DRAW_ORDER::OBJ_CURSOR);

static mVec2_c l_pos(0.0f, 0.0f);
static f32 l_alpha;
static f32 l_edgeRate;
static u16 l_flags;

int dObjCursor_c::create() {
    if (!mRes.load("Layout/pointer.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mRes, "Pointer.brlyt");
    mRippleLyt.build(&mRes, "ripple.brlyt");
    mLyt.mPriority = 0x88;
    mRippleLyt.mPriority = 0x88;
    mpHand0 = mLyt.findPane("N_hand00");
    mpHand1 = mLyt.findPane("N_hand01");
    mpShadow = mLyt.findPane("N_shadow");
    mpHand = mLyt.findPane("hand");
    initAnm();
    mLyt.calc();
    mRippleLyt.calc();
    l_flags = FLAG_VISIBLE;
    return SUCCEEDED;
}

int dObjCursor_c::doDelete() {
    return mRes.remove() != FALSE;
}

int dObjCursor_c::execute() {
    calcPos();
    calcAlpha();
    calcState();
    playAnm();
    calcRotate();
    if (l_flags & FLAG_VISIBLE) {
        mLyt.mPos.x = l_pos.x;
        mLyt.mPos.y = l_pos.y;
        if (!(l_flags & FLAG_RIPPLE)) {
            mRippleLyt.mPos.x = l_pos.x;
            mRippleLyt.mPos.y = l_pos.y;
        }
        mRippleLyt.calc();
        mLyt.calc();
    }
    return SUCCEEDED;
}

int dObjCursor_c::draw() {
    if (l_flags & FLAG_SYNC_POS) {
        l_flags &= ~FLAG_SYNC_POS;
        mLyt.mPos.x = l_pos.x;
        mLyt.mPos.y = l_pos.y;
        mLyt.calc();
    }
    if (l_flags & FLAG_VISIBLE) {
        mRippleLyt.entry();
        mLyt.entry();
    }
    return SUCCEEDED;
}

void dObjCursor_c::initAnm() {
    static const char *l_anmName[] = {
        "Pointer_on.brlan",
        "Pointer_off.brlan",
        "Pointer_hold.brlan",
        "Pointer_release.brlan",
    };
    mAnm.init(&mLyt, &mRes, l_anmName, 4);
    mAnm.start(ANM_OFF);
    mAnm.setToEnd();
    mRippleLyt.setAnm(&mRippleAnm, "ripple_ripple.brlan", nullptr);
    mRippleAnm.setToEnd();
    mRippleLyt.setVisible("ripple", true);
}

void dObjCursor_c::playAnm() {
    mAnm.play();
    mRippleAnm.play();
}

void dObjCursor_c::calcState() {
    if (!mPad::g_currentCore->down(WPAD_BUTTON_A)) {
        l_flags &= ~FLAG_PRESS_SE;
    }
    if (l_flags & FLAG_REQ_HOLD) {
        mAnm.setNext(ANM_HOLD);
        l_flags |= FLAG_HOLD;
        l_flags &= ~FLAG_REQ_HOLD;
    }
    if (l_flags & FLAG_REQ_RELEASE) {
        if (l_flags & FLAG_VISIBLE) {
            mAnm.setNext(ANM_RELEASE);
        } else {
            mAnm.startEnd(ANM_RELEASE);
            mLyt.calc();
        }
        l_flags &= ~FLAG_REQ_RELEASE;
    }
    if (l_flags & FLAG_HOLD) {
        if (mAnm.isEnd() && mAnm.mCurIdx == ANM_RELEASE) {
            mAnm.start(ANM_OFF);
            mAnm.setToEnd();
            l_flags &= ~FLAG_HOLD;
        }
        return;
    }
    bool held = mPad::g_currentCore->down(WPAD_BUTTON_A);
    mAnm.setNext(!held);
    if (dPlayerMgr::fn_800FFAC0()) {
        BOOL first = !(l_flags & FLAG_PRESS);
        l_flags |= FLAG_PRESS;
        if (first && !(l_flags & FLAG_PRESS_SE)) {
            playSe(SE_Pointing);
            l_flags |= FLAG_PRESS_SE;
        }
        if (mRippleAnm.mFrameCtrl.isStop()) {
            static mVec2_c l_smallScale(0.7f, 0.7f);
            static mVec2_c l_bigScale(1.0f, 1.0f);
            if (first) {
                mRippleLyt.getRootPane()->SetScale(l_bigScale);
                l_flags |= FLAG_RIPPLE;
            } else {
                mRippleLyt.getRootPane()->SetScale(l_smallScale);
                l_flags &= ~FLAG_RIPPLE;
            }
            mRippleAnm.setFrame(0.0f);
        }
    } else {
        l_flags &= ~FLAG_PRESS;
        if (mRippleAnm.mFrameCtrl.isStop()) {
            l_flags &= ~FLAG_RIPPLE;
        }
    }
}

void dObjCursor_c::calcPos() {
    BOOL invalid = FALSE;
    if (mPad::g_currentCore->getDpdValidFlag() > 0) {
        mVec2_c pos = dPad::getPointerPosition();
        l_pos = pos;
    } else {
        invalid = TRUE;
    }
    if (invalid) {
        sLib::chase(&l_alpha, 0.0f, 0.05f);
    } else {
        l_alpha = 1.0f;
    }

    BOOL edge = FALSE;
    if (dPlayerMgr::fn_800FFAC0()) {
        mLyt.mPos.x = l_pos.x;
        mLyt.mPos.y = l_pos.y;
        mLyt.calc();
        mVec3_c screen;
        m2d::getScreenPos(&screen, mpHand);
        if (screen.x - 30.0f < -304.0f) {
            l_pos.x += -304.0f - (screen.x - 30.0f);
            edge = TRUE;
        } else if (screen.x + 30.0f > 304.0f) {
            l_pos.x += 304.0f - (screen.x + 30.0f);
            edge = TRUE;
        }
        if (screen.y - 30.0f < -228.0f) {
            l_pos.y += -228.0f - (screen.y - 30.0f);
            edge = TRUE;
        } else if (screen.y + 30.0f > 228.0f) {
            l_pos.y += 228.0f - (screen.y + 30.0f);
            edge = TRUE;
        }
    }
    sLib::chase(&l_edgeRate, edge ? 1.0f : 0.0f, 0.05f);
}

void dObjCursor_c::calcAlpha() {
    f32 hand = 255.0f * l_alpha * (1.0f - l_edgeRate);
    f32 shadow = 100.0f * l_alpha * (1.0f - l_edgeRate);
    f32 hand01 = 255.0f * l_alpha * l_edgeRate;
    mLyt.setAlpha("shadow", (int)shadow);
    mLyt.setAlpha("hand", (int)hand);
    mLyt.setAlpha("hand01", (int)hand01);
}

void dObjCursor_c::calcRotate() {
    f32 deg;
    if (mAnm.mCurIdx == ANM_HOLD ||
        (mAnm.mCurIdx == ANM_RELEASE && !mAnm.isEnd())) {
        deg = 0.0f;
    } else {
        mVec2_c dir = dPad::getPointerDirection();
        f32 rad = cM::atan2f(dir.y, dir.x);
        deg = rad * mAng::RadianToDegreeCoefficient;
    }
    nw4r::math::VEC3 rot(0.0f, 0.0f, -deg);
    mpHand0->SetRotate(rot);
    mpHand1->SetRotate(rot);
    mpShadow->SetRotate(rot);
}

void dObjCursor_c::hold() {
    l_flags |= FLAG_REQ_HOLD;
    l_flags &= ~FLAG_REQ_RELEASE;
}

void dObjCursor_c::show() {
    if (l_flags & FLAG_HOLD) {
        l_flags |= FLAG_REQ_RELEASE;
    }
    l_flags |= FLAG_VISIBLE | FLAG_SYNC_POS;
}

void dObjCursor_c::hide() {
    if (l_flags & FLAG_HOLD) {
        l_flags |= FLAG_REQ_RELEASE;
    }
    l_flags &= ~FLAG_VISIBLE;
}

mVec2_c dObjCursor_c::getPos() {
    return l_pos;
}

BOOL dObjCursor_c::isVisible() {
    return l_alpha > 0.0f;
}

dObjCursor_c::~dObjCursor_c() {
}
