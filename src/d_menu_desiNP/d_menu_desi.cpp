#include <game/game/d_menu_desi.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_mii_icon.hpp>
#include <revolution/SC.h>
#include <stdio.h>

namespace dMenuDesi {
static const char *l_miiLogo[] = {"miiLogo", NULL};
static const char *l_miiFace = "miiFace";
static const char *l_playerFace = "playerFc";
static const char *l_miiButton = "miiBtn_Fc";
static const char *l_selection = "myDsgnFc";
static const char *l_previousSelection = "myDsgnFcBefore";

// .text:0x5C
BOOL Desi_c::load() {
    return mRes.load("Layout/desi_win.arc");
}
// .text:0x68
void Desi_c::create(BOOL pickup) {
    mLyt.build(&mRes, "slct_design_win_00.brlyt");
    initPanes();
    if (pickup) {
        mPickupLyt.build(&mRes, "pickUp_design_00.brlyt");
        mPickupMiiLyt.build(&mRes, "pickUp_Mii_00.brlyt");
        mFlags |= FLAG_PICKUP;
        mPickupLyt.mPriority = 0x84;
        mPickupMiiLyt.mPriority = 0x84;
    }
    mLyt.setAnm(&mScrollAnm, "slct_design_win_00_scrlIcon_00.brlan", NULL);
    mLyt.setAnm(&mMiiPushAnm, "slct_design_win_00_Mii_Psh.brlan", NULL);
    mMiiPushAnm.setToEnd();
    setMiiAnm(0);
}
// .text:0x140
BOOL Desi_c::remove() {
    if (mFlags & FLAG_CAPTURE) {
        mFlags &= ~FLAG_CAPTURE;
        mCapture[CAPTURE_MII].remove();
        mCapture[CAPTURE_PICKUP].remove();
    }
    return mRes.remove();
}
// .text:0x194
void Desi_c::calc() {
    mMiiPushAnm.play();
    mScrollAnm.play();
    mLyt.calc();
}
// .text:0x1E0
void Desi_c::draw() {
    mLyt.entry();
}
// .text:0x1E8
m2d::SimpleEx_c *Desi_c::getLayout() {
    return &mLyt;
}
// .text:0x1F0
void Desi_c::setPreviousSelection(int slot) {
    char name[40];
    sprintf(name, "N_myDsgn_%02d", slot);
    mLyt.setPosFrom(name, l_previousSelection, 0.0f, 0.0f);
    if (mPreviousSelection == -1) {
        mLyt.setVisible(l_previousSelection, true);
    }
    mPreviousSelection = slot;
}
// .text:0x284
int Desi_c::getPreviousSelection() {
    return mPreviousSelection;
}
// .text:0x28C
BOOL Desi_c::setSelection(int slot, BOOL visible) {
    char name[40];
    if (visible) {
        if (mSelection == -1) {
            mLyt.setVisible(l_selection, true);
        }
        if (slot != mSelection) {
            sprintf(name, "N_myDsgn_%02d", slot);
            mSelection = slot;
            mLyt.setPosFrom(name, l_selection, 0.0f, 0.0f);
            return TRUE;
        }
    } else if (slot == mSelection) {
        mLyt.setVisible(l_selection, false);
        mSelection = -1;
    }
    return FALSE;
}
// .text:0x364
void Desi_c::setFaceVisible(int kind, bool visible) {
    if (kind == 0) {
        mLyt.setVisible(l_playerFace, visible);
    } else {
        mLyt.setVisible(l_miiButton, visible);
    }
}
static char l_designPaneFormat[16] = "myDsgn_%02d";

// .text:0x38C
void Desi_c::initPanes() {
    char name[40];
    for (int i = 0; i < 8; ++i) {
        sprintf(name, l_designPaneFormat, i);
        mDesignPanes[i] = mLyt.findPane(name);
    }
    updateDesigns();
    mLyt.setVisible(l_playerFace, false);
    mLyt.setVisible(l_miiButton, false);
    mLyt.setVisible(l_miiLogo[0], false);
    mLyt.setVisible(l_miiFace, false);
    mLyt.setVisible(l_selection, false);
    mLyt.setVisible(l_previousSelection, false);
    mSelection = -1;
    mPreviousSelection = -1;
}

// .text:0x484
void Desi_c::updateDesigns() {
    char name[40];
    for (int i = 0; i < 8; ++i) {
        nw4r::lyt::TexMap *tex = m2d::getTexMaps(mDesignPanes[i]);
        dDesign_c *design = getDesign(i);
        tex->SetImage(design->getTexture());
        tex->SetPalette(design->getPalette());
        sprintf(name, "N_pro_%02d", i);
        mLyt.setVisible(name, design->mIsPro != 0);
    }
}
// .text:0x530
dDesign_c *Desi_c::getDesign(int slot) {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.getDesign(
        (u8)slot);
}
// .text:0x568
void Desi_c::setDesign(int slot, dDesign_c *design) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mOrgDesigns.setDesign((u8)slot,
                                                                 design);
}

// .text:0x5B0
int Desi_c::hitDesign(const mVec2_c &pos, BOOL setOffset, f32 scale) {
    m2d::hitArg_c hit;
    hit.mpHit = NULL;
    hit.mScale = scale;
    hit.mPos = pos;
    mLyt.patrolPane(m2d::isInPane, m2d::isRectButton, &hit);
    if (hit.mpHit) {
        int slot = m2d::getNameNumber(hit.mpHit->GetName(), 2);
        if (setOffset) {
            mVec3_c panePos;
            m2d::getScreenPos(&panePos, hit.mpHit);
            mVec2_c ofs;
            ofs.x = panePos.x - pos.x;
            ofs.y = panePos.y - pos.y;
            mMove.setOfs(ofs);
        }
        return slot;
    }
    return -1;
}
// .text:0x690
int Desi_c::hitFace(const mVec2_c &pos, BOOL setOffset, f32 scale) {
    m2d::hitArg_c hit;
    hit.mpHit = NULL;
    hit.mScale = scale;
    hit.mPos = pos;
    mLyt.patrolPane(m2d::isInPane, m2d::isCircleButton, &hit);
    if (hit.mpHit) {
        int kind = m2d::getNameNumber(hit.mpHit->GetName(), 2);
        if (kind == -1) {
            return 0;
        }
        if (mFlags & FLAG_CAPTURE) {
            if (setOffset) {
                mVec3_c panePos;
                m2d::getScreenPos(&panePos, hit.mpHit);
                mVec2_c ofs;
                ofs.x = panePos.x - pos.x;
                ofs.y = panePos.y - pos.y;
                mMove.setOfs(ofs);
            }
            return 1;
        }
        return -1;
    }
    return -1;
}
// .text:0x790
mVec2_c Desi_c::getDesignPos(int slot, f32 x, f32 y) {
    mVec2_c result(0.0f, 0.0f);
    char name[40];
    sprintf(name, "N_myDsgn_%02d", slot);
    mVec3_c pos;
    m2d::getScreenPos(&pos, mLyt.findPane(name));
    if (SCGetAspectRatio() == 1) {
        x *= 0.730769f;
    }
    result.x = pos.x + x;
    result.y = pos.y + y;
    return result;
}
// .text:0x85C
mVec2_c Desi_c::getFacePos(int kind, f32 x, f32 y) {
    mVec2_c result(0.0f, 0.0f);
    mVec3_c pos;
    m2d::getScreenPos(&pos,
                      mLyt.findPane(kind == 0 ? l_playerFace : l_miiFace));
    if (SCGetAspectRatio() == 1) {
        x *= 0.730769f;
    }
    result.x = pos.x + x;
    result.y = pos.y + y;
    return result;
}
// .text:0x920
void Desi_c::setDesignVisible(int slot, bool visible) {
    char name[40];
    sprintf(name, l_designPaneFormat, slot);
    mLyt.setVisible(name, visible);
    visible = visible & isProDesign(slot);
    sprintf(name, "N_pro_%02d", slot);
    mLyt.setVisible(name, visible);
}
// .text:0x9CC
bool Desi_c::isProDesign(int slot) {
    return getDesign(slot)->mIsPro != 0;
}
// .text:0x9FC
bool Desi_c::setupMii() {
    mCapture[CAPTURE_MII].clear();
    mCapture[CAPTURE_PICKUP].clear();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dMiiCreateID_c *id = &player->mMiiId;
    BOOL found = FALSE;
    u16 index;
    if (player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        if (dMiiIcon::getTravelDB()) {
            found = TRUE;
            index = 0xFFFE;
        }
    } else {
        if (fn_802C4404(id, &index)) {
            found = TRUE;
        }
    }
    if (found) {
        nw4r::lyt::TexMap *tex = m2d::getTexMaps(mLyt.findPane(l_miiFace));
        mCapture[CAPTURE_MII].set(index, 120, 120, tex->GetImage());
        mCapture[CAPTURE_MII].entry();
        if (mFlags & FLAG_PICKUP) {
            tex = m2d::getTexMaps(mPickupMiiLyt.findPane(l_miiFace));
            mCapture[CAPTURE_PICKUP].set(index, 120, 120, tex->GetImage());
            mCapture[CAPTURE_PICKUP].entry();
        }
        mFlags |= FLAG_CAPTURE;
    }
    bool logo;
    bool hasMii = (mFlags & FLAG_CAPTURE) != 0;
    logo = !hasMii;
    mLyt.setVisible(l_miiLogo[0], logo);
    mLyt.setVisible(l_miiFace, !logo);
    return hasMii;
}
// .text:0xB60
void Desi_c::setMiiVisible(bool visible) {
    mLyt.setVisible(l_miiFace, visible);
}
// .text:0xB74
void Desi_c::drawPickup() {
    if (mFlags & FLAG_PICKUP_MII) {
        mPickupMiiLyt.entry();
    } else {
        mPickupLyt.entry();
    }
}
// .text:0xB90
void Desi_c::calcPickup() {
    if (mFlags & FLAG_PICKUP_MII) {
        mVec2_c *pos = mMove.getPos();
        mPickupMiiLyt.mPos.x = pos->x;
        mPickupMiiLyt.mPos.y = pos->y;
        mPickupMiiLyt.calc();
    } else {
        mVec2_c *pos = mMove.getPos();
        mPickupLyt.mPos.x = pos->x;
        mPickupLyt.mPos.y = pos->y;
        mPickupLyt.calc();
    }
}
// .text:0xC20
bool Desi_c::calcMove() {
    return mMove.calcMove();
}
// .text:0xC28
void Desi_c::startMove(const mVec2_c &pos, BOOL mii) {
    mMove.startMove(pos);
    if (mii) {
        mFlags |= FLAG_PICKUP_MII;
    } else {
        mFlags &= ~FLAG_PICKUP_MII;
    }
}
// .text:0xC88
void Desi_c::calcFollow() {
    mMove.calcFollow();
}
// .text:0xC90
void Desi_c::startFollow(BOOL mii) {
    mMove.startFollow();
    if (mii) {
        mFlags |= FLAG_PICKUP_MII;
    } else {
        mFlags &= ~FLAG_PICKUP_MII;
    }
}
// .text:0xCF0
void Desi_c::setPickupDesign(dDesign_c *design) {
    nw4r::lyt::TexMap *tex = m2d::getTexMaps(mPickupLyt.findPane("myDsgn"));
    tex->SetImage(design->getTexture());
    tex->SetPalette(design->getPalette());
    mPickupLyt.setVisible("N_pro_00", design->mIsPro != 0);
}
// .text:0xD7C
void Desi_c::setOffset(const mVec2_c &pos) {
    mMove.setOfs(pos);
}
// .text:0xD84
void Desi_c::setPickupPos(const mVec2_c &pos) {
    mMove.setPos(pos);
    calcPickup();
}
// .text:0xDBC
void Desi_c::startLeaveAnm(int anm) {
    static const char *names[] = {"slct_design_win_00_leaveDsgn_In.brlan",
                                  "slct_design_win_00_leaveDsgn_Out.brlan"};
    mLyt.setAnm(&mLeaveAnm, names[anm], NULL);
    mFlags |= FLAG_LEAVE_ANM;
}
// .text:0xE10
BOOL Desi_c::calcLeaveAnm() {
    if (!(mFlags & FLAG_LEAVE_ANM)) {
        return TRUE;
    }
    if (mLeaveAnm.mFrameCtrl.isStop()) {
        return TRUE;
    }
    mLeaveAnm.play();
    return TRUE;
}
// .text:0xE70
void Desi_c::startMiiPush() {
    mMiiPushAnm.setFrame(0.0f);
}
// .text:0xE80
void Desi_c::setMiiAnm(int anm) {
    static const char *names[] = {"slct_design_win_00_Mii_On.brlan",
                                  "slct_design_win_00_Mii_Off.brlan"};
    mLyt.setAnm(&mMiiAnm, names[anm], NULL);
    mMiiAnm.setToEnd();
}
} // namespace dMenuDesi
