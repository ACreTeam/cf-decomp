// d_balloon.cpp: tutorial speech balloon. .text 80065524..80066018,
// .rodata 8046CDC0..8046CE20, .data 804A50B0..804A51A8,
// .sdata 80749B98..80749BD8, .sdata2 80750338..80750370.
#define D_BALLOON_SOURCE
#include <game/game/d_balloon.hpp>
#undef D_BALLOON_SOURCE
#include <lib/egg/math/eggMatrix.h>
#include <lib/revolution/MTX/mtxvec.h>
#include <lib/revolution/SC/scapi.h>

static const char *lBalloonPane = "bln_00";
static const char *lShadowPane = "blnSdw_00";
static const char *lTextPane = "T_msg_00";
static char lAllPaneName[] = "N_all";
static char lTailPane0[] = "blnTail_00";
static char lTailPane1[] = "blnTail_01";
static char lTailShadowPane0[] = "blnTailSdw_00";
static char lTailShadowPane1[] = "blnTailSdw_01";

typedef void (dBalloon_c::*BalloonStateFunc)();
static const BalloonStateFunc lStateFuncs[4][2] = {
    {&dBalloon_c::stateHideInit, &dBalloon_c::stateHide},
    {&dBalloon_c::stateOpenInit, &dBalloon_c::stateOpen},
    {&dBalloon_c::stateShowInit, &dBalloon_c::stateShow},
    {&dBalloon_c::stateCloseInit, &dBalloon_c::stateClose},
};

// 80065524
dBalloon_c::dBalloon_c() {}

// 8006559C
dBalloon_c::~dBalloon_c() {}

// 8006562C
BOOL dBalloon_c::loadArc() { return mResAcc.load("Layout/bal.arc"); }

// 8006563C
BOOL dBalloon_c::build() {
    mLyt.build(&mResAcc, "balloon_00.brlyt");
    bool built = mLyt.setAnm(&mAnm, "balloon_00_blnIn.brlan", NULL);
    mState = STATE_HIDE;
    mNextState = STATE_NONE;
    mLyt.mPriority = 0x84;
    nw4r::lyt::TextBox *box = mLyt.findTextBox(lTextPane);
    _2BC = box->GetTranslate().x - 0.5f * box->GetSize().width;
    mWord.setTextBox(box);
    setTail(0);
    _2C8 = mpTail->GetTranslate().x;
    setColor(0);
    mFlags = 0;
    return built;
}

// 80065718
BOOL dBalloon_c::remove() { return mResAcc.remove() != 0; }

// 80065748
int dBalloon_c::execute() {
    runState();
    changeState();
    return 1;
}

// 80065780
int dBalloon_c::draw() {
    if (mState != STATE_HIDE) {
        mLyt.entry();
    }
    return 1;
}

// 800657B4
void dBalloon_c::setPos(mVec2_c pos) {
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        pos.x += 56.99998f;
    } else {
        pos.x += 78.0f;
    }
    if (mTail == 0) {
        pos.y -= 55.0f;
    } else {
        pos.y += 55.0f;
    }
    nw4r::lyt::Pane *all = mLyt.findPane(lAllPaneName);
    all->SetTranslate(mVec3_c(pos, 0.0f));
    mVec3_c tailPos = mpTail->GetTranslate();
    tailPos.x = _2C8;
    mpTail->SetTranslate(tailPos);
    tailPos.x += 4.0f;
    mpTailSdw->SetTranslate(tailPos);
}

// 800658C8
void dBalloon_c::updateWidth() {
    mWidth = mWord.calcWidth(NULL, 0);
    if (mWidth < 20.0f) {
        mWidth = 20.0f;
    }
    f32 endWidth = 44.0f + 46.0f * (mWidth - 20.0f) / 280.0f;
    f32 totalWidth = endWidth + mWidth;
    nw4r::lyt::Pane *pane = mLyt.findPane(lBalloonPane);
    mVec2_c scale(totalWidth / pane->GetSize().width, 1.0f);
    pane->SetScale(scale);
    mVec3_c pos = pane->GetTranslate();
    pos.x = _2BC + 0.5f * (totalWidth - endWidth);
    pane->SetTranslate(pos);
    nw4r::lyt::Pane *shadow = mLyt.findPane(lShadowPane);
    shadow->SetScale(scale);
    pos.x += 4.0f;
    pos.y -= 4.0f;
    shadow->SetTranslate(pos);
}

// 80065A04
void dBalloon_c::setWord(const dScript::Word_c *word) {
    mWord.clear();
    mWord.copy(word, 0);
    mWord.update();
    updateWidth();
}

// 80065A60
void dBalloon_c::clampOnScreen() {
    mLyt.calc();
    nw4r::lyt::TextBox *box = mWord.mpTextBox;
    f32 shift = 0.0f;
    const nw4r::math::MTX34 &global = box->GetGlobalMtx();
    EGG::Matrix34f matrix(global._00, global._01, global._02, global._03,
        global._10, global._11, global._12, global._13,
        global._20, global._21, global._22, global._23);
    Vec point;
    point.x = -box->GetSize().width * 0.5f;
    point.y = 0.0f;
    point.z = 0.0f;
    PSMTXMultVec(matrix.m, &point, &point);
    if (point.x < -276.0f) {
        shift = -276.0f - point.x;
    }
    if (shift == 0.0f) {
        point.x = mWidth + -box->GetSize().width * 0.5f;
        point.y = 0.0f;
        point.z = 0.0f;
        PSMTXMultVec(matrix.m, &point, &point);
        if (point.x > 276.0f) {
            shift = 276.0f - point.x;
        }
    }
    if (shift != 0.0f) {
        nw4r::lyt::Pane *all = mLyt.findPane(lAllPaneName);
        mVec3_c &allPos = *reinterpret_cast<mVec3_c *>(&point);
        allPos = all->GetTranslate();
        allPos.x += shift;
        all->SetTranslate(allPos);
        if (shift > 14.0f) {
            shift = 14.0f;
        }
        mVec3_c tailPos = mpTail->GetTranslate();
        tailPos.x -= shift;
        mpTail->SetTranslate(tailPos);
        tailPos = mpTailSdw->GetTranslate();
        tailPos.x -= shift;
        mpTailSdw->SetTranslate(tailPos);
    }
}

// 80065C54
void dBalloon_c::changeState() {
    if (mNextState != STATE_NONE) {
        mState = mNextState;
        if (lStateFuncs[mNextState][0] != NULL) {
            (this->*lStateFuncs[mNextState][0])();
        }
        mNextState = STATE_NONE;
    }
}

// 80065CD0
void dBalloon_c::runState() {
    if (lStateFuncs[mState][1] != NULL) {
        (this->*lStateFuncs[mState][1])();
    }
}

// 80065D40
void dBalloon_c::stateHideInit() {}
// 80065D44
void dBalloon_c::stateHide() {}

// 80065D48
void dBalloon_c::stateOpenInit() {
    mLyt.changeAnm(&mAnm, "balloon_00_blnIn.brlan", NULL);
    if (mFlags & FLAG_1) {
        mAnm.setToEnd();
        mLyt.calc();
        clampOnScreen();
        mAnm.setFrame(0.0f);
    }
    mAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mLyt.calc();
}

// 80065DE0
void dBalloon_c::stateOpen() {
    mAnm.play();
    if (mFlags & FLAG_1) {
        clampOnScreen();
    }
    mLyt.calc();
    if (mAnm.mFrameCtrl.isStop()) {
        mNextState = STATE_SHOW;
    }
}

// 80065E50
void dBalloon_c::stateShowInit() {}
// 80065E54
void dBalloon_c::stateShow() {}

// 80065E58
void dBalloon_c::stateCloseInit() {
    mLyt.changeAnm(&mAnm, "balloon_00_blnOut.brlan", NULL);
    mAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
}

// 80065EA0
void dBalloon_c::stateClose() {
    mAnm.play();
    mLyt.calc();
    if (mAnm.mFrameCtrl.isStop()) {
        mNextState = STATE_HIDE;
    }
}

// 80065EFC
// char lColorBlue[] = "balloon_00_blnBlue.brlan";
// char lColorYellow[] = "balloon_00_blnYlow.brlan";
void dBalloon_c::setColor(int color) {
    static const char *lColorAnms[] = {
        "balloon_00_blnBlue.brlan",
        "balloon_00_blnYlow.brlan",
    };
    mLyt.setAnm(&mColorAnm, lColorAnms[color], NULL);
    mColorAnm.setToEnd();
    mLyt.calc();
}

// 80065F5C
void dBalloon_c::setTail(int tail) {
    static const char *lTailPanes[] = {lTailPane0, lTailPane1};
    static const char *lTailShadowPanes[] = {lTailShadowPane0, lTailShadowPane1};
    mTail = tail;
    mLyt.setVisible(lTailPanes[tail], true);
    mLyt.setVisible(lTailShadowPanes[tail], true);
    mpTail = mLyt.findPane(lTailPanes[tail]);
    mpTailSdw = mLyt.findPane(lTailShadowPanes[tail]);
    int other = mTail == 0 ? 1 : 0;
    mLyt.setVisible(lTailPanes[other], false);
    mLyt.setVisible(lTailShadowPanes[other], false);
}
