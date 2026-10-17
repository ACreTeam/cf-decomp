#include <game/framework/f_profile_name.hpp>
#include <game/game/d_obj_bal.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_world.hpp>
#include <revolution/SC.h>
#include <cstdio>
#include <game/game/d_a_player.hpp>

CUSTOM_BASE_PROFILE(OBJ_BAL, dObjBal_c, fProf::OBJ_BAL,
                    fProf::DRAW_ORDER::OBJ_BAL);

static dObjBal_c *l_this;

int dObjBal_c::create() {
    if (!mRes.load("Layout/outdoor.arc")) {
        return NOT_READY;
    }
    l_this = this;
    mLyt.build(&mRes, "outdoor_balloon_00.brlyt");
    mLyt.mPriority = 2;
    initAnm();
    mState = STATE_WAIT;
    mLyt.calc();
    return SUCCEEDED;
}

int dObjBal_c::doDelete() {
    if (!mRes.remove()) {
        return NOT_READY;
    }
    l_this = nullptr;
    return SUCCEEDED;
}

int dObjBal_c::execute() {
    static void (dObjBal_c::*l_stateFuncs[])() = {
        &dObjBal_c::stateWait,
        &dObjBal_c::stateIn,
        &dObjBal_c::stateShow,
        &dObjBal_c::stateOut,
    };
    if (dDemo::isType(dDemo::TYPE_ANY)) {
        hide();
    }
    (this->*l_stateFuncs[mState])();
    calcPos();
    mAnm.play();
    mLyt.calc();
    return SUCCEEDED;
}

int dObjBal_c::draw() {
    if (mFlags & FLAG_SHOW) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

void dObjBal_c::calcPos() {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == nullptr) {
        return;
    }
    mVec3_c pos = player->mPos;
    mVec3_c curved;
    dWorld::curvePosition(&curved, &pos);
    f32 x, y;
    dCamera::worldToScreen(dCamera_c::sInstance, &x, &y, &curved);
    mVec2_c screen;
    screen.x = -304.0f + 608.0f * x;
    screen.y = 80.0f + (-228.0f + 456.0f * (1.0f - y));
    mLyt.mPos.x = screen.x;
    mLyt.mPos.y = screen.y;
}

void dObjBal_c::initAnm() {
    static const char *l_anmName[] = {
        "outdoor_balloon_00_bln00_In.brlan",
        "outdoor_balloon_00_bln00_Out.brlan",
        "outdoor_balloon_00_bln01_In.brlan",
        "outdoor_balloon_00_bln01_Out.brlan",
    };
    mAnm.init(&mLyt, &mRes, l_anmName, 4);
    mAnm.start(1);
    mAnm.setToEnd();
    mAnm.start(3);
    mAnm.setToEnd();
    mText[0].setTextBox(mLyt.findTextBox("T_msg_00"));
    mText[1].setTextBox(mLyt.findTextBox("T_msg_01"));
    nw4r::lyt::Pane *body = mLyt.findPane("blnFc_00");
    mBaseWidth = body->GetScale().x * body->GetSize().width;
    mBaseX = mLyt.findPane("N_blnPosition_00")->GetTranslate().x;
}

void dObjBal_c::startAnm(int anm) {
    if (mFlags & FLAG_LOWER) {
        anm += 2;
    }
    mAnm.start(anm);
}

void dObjBal_c::stateWait() {
    if (!mReqItem.isValid()) {
        return;
    }
    mItem = mReqItem;
    if (mLyt.mPos.y > 70.0f) {
        mFlags |= FLAG_LOWER;
    } else {
        mFlags &= ~FLAG_LOWER;
    }
    mLyt.setVisible("N_bln_00_all", !(mFlags & FLAG_LOWER));
    mLyt.setVisible("N_bln_01_all", mFlags & FLAG_LOWER);
    int bln = (mFlags & FLAG_LOWER) != 0;
    setWord(bln);
    setWidth(bln);
    setPos(bln);
    startAnm(ANM_IN);
    mFlags |= FLAG_SHOW;
    mState = STATE_IN;
}

void dObjBal_c::stateIn() {
    if (mAnm.isEnd()) {
        mState = STATE_SHOW;
    }
}

void dObjBal_c::stateShow() {
    if (mReqItem.isNotSame(mItem)) {
        startAnm(ANM_OUT);
        // As in the original: back to STATE_WAIT, not STATE_OUT (stateOut is
        // never reached).
        mState = STATE_WAIT;
    }
}

void dObjBal_c::stateOut() {
    if (mAnm.isEnd()) {
        mFlags &= ~FLAG_SHOW;
        mState = STATE_WAIT;
    }
}

void dObjBal_c::setWord(int bln) {
    dItem::name_c name(mItem);
    m2d::itemWord_c *text = &mText[bln];
    text->clear();
    text->copy(&name, 0);
    text->update();
}

void dObjBal_c::setWidth(int bln) {
    f32 width = mText[bln].calcWidth(nullptr, 0);
    width += 25.0f;
    width *= 1.18f;
    mVec2_c scale;
    scale.x = width / mBaseWidth;
    scale.y = 1.0f;
    char name[40];
    sprintf(name, "N_blnBody_%02d", bln);
    mLyt.findPane(name)->SetScale(scale);
}

void dObjBal_c::setPos(int bln) {
    char name[40];
    sprintf(name, "N_blnPosition_%02d", bln);
    nw4r::lyt::Pane *pos = mLyt.findPane(name);
    sprintf(name, "blnFc_%02d", bln);
    nw4r::lyt::Pane *body = mLyt.findPane(name);
    mVec3_c trans = pos->GetTranslate();
    trans.x = mBaseX;
    pos->SetTranslate(trans);
    startAnm(ANM_IN);
    mAnm.setToEnd();
    mLyt.calc();
    mVec3_c rightBottom;
    m2d::getScreenRightBottom(&rightBottom, body);
    f32 right = rightBottom.x;
    f32 limit = 292.0f;
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        right *= 1.368421f;
        limit *= 1.368421f;
    }
    if (limit < right) {
        trans.x = mBaseX + (limit - right);
        pos->SetTranslate(trans);
    }
}

void dObjBal_c::show(const dItem::Item &item) {
    l_this->mReqItem = item;
}

void dObjBal_c::hide() {
    l_this->mReqItem = dItem::ITEM_ID_NONE;
}

dObjBal_c::~dObjBal_c() {
}
