// Hand tool models. .text 800B9BDC..800BD450. See include/game/game/d_hmn_tool_mng.hpp.
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_bg.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <string.h>

// Tilt of the windmill's wheel (sbss 8074E480).
static s16 l_windmillTilt = mAng::fromDegree(-22.5f);

static dHmnToolMng_c l_toolMng;

// BITM::m_hideBone, clamped like the other users (values >= 0xB become 0xA).
static inline int getHideBone(const dItem::BITM *bitm) {
    int v = (s8)bitm->m_hideBone;
    return (u32)v < 0xB ? v : 0xA;
}

// 800B9BDC
dHmnToolMng_c::data_c::data_c() {
    mCur = 0;
    mLoadState[0] = LOAD_NONE;
    mLoadState[1] = LOAD_NONE;
    memset(mpHeap, 0, sizeof(mpHeap));
    mState = STATE_IDLE;
    mItemId = dItem::ITEM_ID_NONE;
    mReqItem = dItem::ITEM_ID_NONE;
    mLocked = BUFFER_NUM;
}

// 800B9C90
dHmnToolMng_c::data_c::~data_c() {}

// 800B9D14
dHmnToolMng_c::dHmnToolMng_c() {}

// 800B9D5C
dHmnToolMng_c::~dHmnToolMng_c() {}

// 800B9DC0
BOOL dHmnToolMng_c::createHeap(EGG::Heap *parent) {
    data_c *data = mData;
    for (int i = 0; i < SLOT_NUM; i++, data++) {
        for (int j = 0; j < BUFFER_NUM; j++) {
            if (data->mpHeap[j] == NULL) {
                data->mpHeap[j] =
                    mHeap::createFrmHeap(0x9100, parent, "dHmnToolMng_c::createHeap::data", 0x20, mHeap::OPT_NONE);
            }
        }
    }
    return TRUE;
}

// 800B9E54
void dHmnToolMng_c::clearPlayerItems() {
    for (int i = 0; i < PLAYER_SLOT_NUM; i++) {
        mData[i].mItemId = dItem::ITEM_ID_NONE;
    }
}

// 800B9E70
void dHmnToolMng_c::clearItem(int slot) {
    mData[slot].mItemId = dItem::ITEM_ID_NONE;
}

// 800B9E88
BOOL dHmnToolMng_c::load(data_c *data, dPrivateData_c *priv, dDesign_c *design) {
    if (data == NULL) {
        return FALSE;
    }
    int buf = getLoadBuffer(data);
    if (!data->mReqItem.isValid()) {
        data->mLoadState[buf] = LOAD_DONE;
        return TRUE;
    }
    if (data->mLoadState[buf] <= LOAD_BUSY) {
        if (data->mModel[buf].loadItem(data->mReqItem, data->mpHeap[buf])) {
            u32 idx = 0;
            if (dHmnToolBank_c::isOrgUmbrella(&idx, &data->mReqItem)) {
                BOOL ok;
                if (priv != NULL) {
                    ok = data->mDesign[buf].loadDesign(&priv->mOrgDesigns.mDesigns[idx & 7], data->mpHeap[buf], 0x20, 0x80);
                } else if (design != NULL) {
                    ok = data->mDesign[buf].loadDesign(design, data->mpHeap[buf], 0x20, 0x80);
                } else {
                    return TRUE;
                }
                if (ok) {
                    data->mLoadState[buf] = LOAD_DONE;
                    bindDesign(data);
                    return TRUE;
                }
                data->mLoadState[buf] = LOAD_BUSY;
                return FALSE;
            }
            data->mLoadState[buf] = LOAD_DONE;
            return TRUE;
        }
        data->mLoadState[buf] = LOAD_BUSY;
        return FALSE;
    }
    return TRUE;
}

// 800BA00C
BOOL dHmnToolMng_c::release(data_c *data) {
    if (data == NULL) {
        return FALSE;
    }
    int buf = getLoadBuffer(data);
    u8 state = data->mLoadState[buf];
    if (state == LOAD_BUSY || state == LOAD_DONE) {
        if (data->mModel[buf].release() == TRUE && data->mDesign[buf].release() == TRUE) {
            if (data->mpHeap[buf] != NULL) {
                data->mpHeap[buf]->free(3);
            }
            data->mLoadState[buf] = LOAD_NONE;
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 800BA0DC
int dHmnToolMng_c::getLoadBuffer(data_c *data) {
    if (data->mLocked == data->mCur) {
        return data->mCur == 0;
    }
    return data->mCur;
}

// 800BA0F8
void dHmnToolBank_c::myMdlCallback_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {
    if (nodeId == mNodeId) {
        mMtx_c mtx;
        anmRes->GetRotTrans(&mtx);
        mtx.YrotM(l_windmillTilt);
        mtx.XrotM(mRot);
        anmRes->SetRotTrans(&mtx);
    }
}

// 800BA178
void dHmnToolBank_c::myMdlCallback_c::timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl) {}

// 800BA17C
void dHmnToolBank_c::myMdlCallback_c::timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl) {}

// 800BA180
dHmnToolBank_c::dHmnToolBank_c() : mpHeap(NULL), mpBalloon(NULL), mType(dHmnToolMng_c::SLOT_NUM) {}

// 800BA2C4
dHmnToolBank_c::~dHmnToolBank_c() {}

// 800BA3C8
void dHmnToolBank_c::create(int slot) {
    if (mpHeap == NULL) {
        mpHeap = mHeap::createFrmHeap(0x1200, lbl_8074E468, "dHmnToolBank_c::m_heap_p", 0x20, mHeap::OPT_NONE);
        mAllocator.attach(mpHeap, 0x20);
        mType = slot;
        mMdlCreated = FALSE;
        mAnmCreated = FALSE;
        mVisCreated = FALSE;
        mTexSrtCreated = FALSE;
        mDigMdlCreated = FALSE;
        mDigAnmCreated = FALSE;
        mDigVisCreated = FALSE;
        mItem = dItem::ITEM_ID_NONE;
    }
    mFloat.fn_8016E0C0(slot);
}

// 800BA474
BOOL dHmnToolBank_c::fn_800BA474(const dItem::Item *item, int a, int b) {
    if (mType >= dHmnToolMng_c::SLOT_NUM) {
        return TRUE;
    }
    dHmnToolMng_c::data_c *data = l_toolMng.getData(mType);
    if (item->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm == NULL) {
            return TRUE;
        }
        if (getHideBone(bitm) != 9) {
            return TRUE;
        }
    }

    if (data->mState == dHmnToolMng_c::STATE_IDLE) {
        if (data->mLoadState[data->mCur] == dHmnToolMng_c::LOAD_DONE) {
            if (getAxeIdx(*item) != -1) {
                dItem::Item cur(data->mItemId);
                if (getAxeIdx(cur) != -1) {
                    data->mItemId = item->mId;
                    return TRUE;
                }
            } else {
                int type = getItemToolType(item);
                if (type == HMN_TOOL_BALLOON) {
                    dItem::Item cur(data->mItemId);
                    if (getItemToolType(&cur) == HMN_TOOL_BALLOON) {
                        bool isNew = (u32)dItem::seeker_c::get()->findLike(*item) <= 7;
                        bool wasNew = (u32)dItem::seeker_c::get()->findLike(cur) <= 7;
                        if (isNew == wasNew) {
                            data->mItemId = item->mId;
                            return TRUE;
                        }
                    }
                } else if (type == HMN_TOOL_WINDMILL) {
                    dItem::Item cur(data->mItemId);
                    if (getItemToolType(&cur) == HMN_TOOL_WINDMILL) {
                        data->mItemId = item->mId;
                        return TRUE;
                    }
                }
            }
        }
        if (data->mItemId == item->mId) {
            u32 idx = 0;
            if (!isOrgUmbrella(&idx, item) && data->mLoadState[data->mCur] == dHmnToolMng_c::LOAD_DONE) {
                if (item->mId == dItem::ITEM_ID_NONE) {
                    return TRUE;
                }
                if (hasResMdl()) {
                    return TRUE;
                }
            }
        }
        data->mReqItem = *item;
        data->mState = dHmnToolMng_c::STATE_RELEASE;
    }

    switch (data->mState) {
    case dHmnToolMng_c::STATE_RELEASE:
        if (l_toolMng.release(data)) {
            data->mState = dHmnToolMng_c::STATE_LOAD;
        }
        break;
    case dHmnToolMng_c::STATE_LOAD:
        // TODO: a/b are really dPrivateData_c * / dDesign_c * (header signature kept for d_a_npc).
        if (l_toolMng.load(data, (dPrivateData_c *)a, (dDesign_c *)b)) {
            if (item->mId == data->mReqItem.mId) {
                int buf = l_toolMng.getLoadBuffer(data);
                if (data->mLocked == data->mCur) {
                    data->mCur = buf;
                }
                data->mItemId = data->mReqItem.mId;
                data->mReqItem = dItem::ITEM_ID_NONE;
                data->mState = dHmnToolMng_c::STATE_IDLE;
                return TRUE;
            }
            data->mReqItem = *item;
            data->mState = dHmnToolMng_c::STATE_RELEASE;
        }
        break;
    }
    return FALSE;
}

// 800BA798
void dHmnToolBank_c::lock() {
    if (mType < dHmnToolMng_c::SLOT_NUM) {
        dHmnToolMng_c::data_c *data = l_toolMng.getData(mType);
        data->mLocked = data->mCur;
    }
}

// 800BA7C0
void *dHmnToolBank_c::getResFile(BOOL locked) {
    if (mType >= dHmnToolMng_c::SLOT_NUM) {
        return NULL;
    }
    dHmnToolMng_c::data_c *data = l_toolMng.getData(mType);
    int buf = locked ? data->mLocked : data->mCur;
    if (data->mLoadState[buf] == dHmnToolMng_c::LOAD_DONE) {
        if (locked) {
            return data->mModel[buf].getData();
        }
        if (data->mItemId == dItem::ITEM_ID_NONE) {
            return NULL;
        }
        return data->mModel[buf].getData();
    }
    return NULL;
}

// 800BA84C
BOOL dHmnToolBank_c::isAnmLoop() {
    static const u8 sTable[HMN_TOOL_NUM] = {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int type = getToolType();
    if (type >= HMN_TOOL_NUM) {
        return FALSE;
    }
    return sTable[type];
}

// 800BA888
int dHmnToolBank_c::getToolType() {
    return getItemToolType(&mItem);
}

// 800BA890
int dHmnToolBank_c::getItemToolType(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return HMN_TOOL_NONE;
    }
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return HMN_TOOL_NONE;
    }
    switch (bitm->getKind()) {
    case dItem::KIND_SCOOP:
    case dItem::KIND_SILVER_SCOOP:
    case dItem::KIND_GOLD_SCOOP:
        return HMN_TOOL_SCOOP;
    case dItem::KIND_AXE:
    case dItem::KIND_SILVER_AXE:
    case dItem::KIND_GOLD_AXE:
        return HMN_TOOL_AXE;
    case dItem::KIND_FISHINGROD:
    case dItem::KIND_SILVER_FISHINGROD:
    case dItem::KIND_GOLD_FISHINGROD:
        return HMN_TOOL_ROD;
    case dItem::KIND_NET:
    case dItem::KIND_SILVER_NET:
    case dItem::KIND_GOLD_NET:
        return HMN_TOOL_NET;
    case dItem::KIND_WATERING:
    case dItem::KIND_SILVER_WATERING:
    case dItem::KIND_GOLD_WATERING:
        return HMN_TOOL_WATERING;
    case dItem::KIND_PACHINKO:
    case dItem::KIND_GOLD_PACHINKO:
    case dItem::KIND_SILVER_PACHINKO:
        return HMN_TOOL_PACHINKO;
    case dItem::KIND_FLOWER:
        if (getHideBone(bitm) == 9) {
            return HMN_TOOL_FLOWER;
        }
        break;
    case dItem::KIND_CRACKER:
        return HMN_TOOL_CRACKER;
    case dItem::KIND_HANABI:
        return HMN_TOOL_HANABI;
    case dItem::KIND_ORG_UMB:
    case dItem::KIND_UMBRELLA:
        return HMN_TOOL_UMBRELLA;
    case dItem::KIND_BALLOON:
        return HMN_TOOL_BALLOON;
    case dItem::KIND_WINDMILL:
        return HMN_TOOL_WINDMILL;
    case dItem::KIND_SYABON:
        return HMN_TOOL_SYABON;
    }
    return HMN_TOOL_NONE;
}

// 800BA9AC
void dHmnToolBank_c::remove() {
    mFloat.fn_8016E148();
    if (mpHeap != NULL) {
        removeModels();
        mHeap::destroyFrmHeap(mpHeap);
        mpHeap = NULL;
    }
}

// 800BAA00
void dHmnToolBank_c::fn_800BAA00(const mMtx_c *mtx) {
    if (mMdlCreated && mtx != NULL) {
        if (mAnmCreated) {
            mMdl.play();
        }
        if (mTexSrtCreated) {
            mTexSrt.play();
        }
        if (mDigAnmCreated) {
            mDigMdl0.play();
            mDigMdl1.play();
        }
        if (mDigVisCreated) {
            mDigVis0.play();
            mDigVis1.play();
        }
        if (mpBalloon != NULL) {
            if (mpBalloon->mFly) {
                setBalloonColor(mpBalloon->mAlpha);
            }
            // TODO: the balloon writes the hand matrix back; the parameter is const only because
            // d_a_npc's declaration is kept until the symbol is renamed.
            *const_cast<mMtx_c *>(mtx) = *mpBalloon->calc(mtx);
        }
        int type = getToolType();
        if (type == HMN_TOOL_WINDMILL) {
            calcWindmill();
        }
        mMdl.setLocalMtx(mtx);
        mMdl.calc(false);
        if (type == HMN_TOOL_ROD || type == HMN_TOOL_PACHINKO) {
            mMtx_c tip;
            getNodeMtx(&tip, 2);
            mFloat.fn_8016E380(&tip);
        } else if (type == HMN_TOOL_SCOOP) {
            mMtx_c digMtx;
            fn_800FFF2C(&digMtx, &mDigPos, &mDigAngle);
            if (mDigMode == 1) {
                if (mDigAnmCreated && mDigAnm0.isStop()) {
                    mDigMode = 0;
                }
                mDigMdl0.setLocalMtx(&digMtx);
                mDigMdl0.calc(false);
            } else if (mDigMode == 2) {
                if (mDigAnmCreated && mDigAnm1.isStop()) {
                    mDigMode = 0;
                }
                mDigMdl1.setLocalMtx(&digMtx);
                mDigMdl1.calc(false);
            }
        }
    }
}

// 800BAC9C
void dHmnToolBank_c::fn_800BAC9C() {
    if (mMdlCreated) {
        mMdl.entry();
        if (mpBalloon != NULL) {
            mpBalloon->entry();
        }
        int type = getToolType();
        if (type == HMN_TOOL_ROD || type == HMN_TOOL_PACHINKO) {
            mFloat.fn_8016E5D0();
        } else if (type == HMN_TOOL_SCOOP && mDigMdlCreated) {
            if (mDigMode == 1) {
                mDigMdl0.entry();
            } else if (mDigMode == 2) {
                mDigMdl1.entry();
            }
        }
    }
}

// 800BAD74
void dHmnToolBank_c::fn_800BAD74() {
    int type = getToolType();
    if (type == HMN_TOOL_ROD || type == HMN_TOOL_PACHINKO) {
        mFloat.fn_8016E7E4();
    }
    resetHeap();
    nw4r::g3d::ResFile file(getResFile(FALSE));
    if (file.IsValid() && file.fn_8023487C()) {
        mItem = l_toolMng.getData(mType)->mItemId;
        type = getToolType();
        nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
        mMdl.create(mdl, &mAllocator, 0, 1, NULL);
        mMdlCreated = TRUE;
        lock();
        nw4r::g3d::ResAnmChr anm(NULL);
        BOOL bodyAnm = FALSE;
        if (type == HMN_TOOL_UMBRELLA) {
            if (mType <= 3) {
                anm = dHmnAnm_c::getResAnmChr(0x1A4);
            } else {
                anm = dHmnAnm_c::getResAnmChr(0x1A5);
            }
            bodyAnm = TRUE;
        } else if (type == HMN_TOOL_FLOWER) {
            anm = dHmnAnm_c::getResAnmChr(0x1BB);
            bodyAnm = TRUE;
        }
        if (type != HMN_TOOL_WINDMILL && type != HMN_TOOL_BALLOON && type != HMN_TOOL_SCOOP &&
            (bodyAnm || file.fn_802348C8()))
        {
            if (!bodyAnm) {
                anm = file.GetResAnmChr(0);
            }
            if (anm.IsValid()) {
                mAnm.create(mdl, anm, &mAllocator, NULL);
                mAnm.mPlayMode = m3d::FORWARD_LOOP;
                mMdl.setAnm(mAnm);
                if (!isAnmLoop()) {
                    mAnm.setRate(0.0f);
                    if (type == HMN_TOOL_UMBRELLA) {
                        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
                    }
                }
                mAnmCreated = TRUE;
            }
        }
        mMdl.setCallback(NULL);
        setupAnm(type, &file, &mdl);
    }
}

// 800BAFB4
void dHmnToolBank_c::setupAnm(int type, nw4r::g3d::ResFile *file, nw4r::g3d::ResMdl *mdl) {
    switch (type) {
    case HMN_TOOL_ROD: {
        int grade = 0;
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mItem);
        if (bitm != NULL) {
            int kind = bitm->getKind();
            if (kind == dItem::KIND_SILVER_FISHINGROD) {
                grade = 1;
            } else if (kind == dItem::KIND_GOLD_FISHINGROD) {
                grade = 2;
            }
        }
        mFloat.fn_8016E1AC(grade, &mItem);
        break;
    }
    case HMN_TOOL_AXE:
        setupAxe(file, mdl);
        break;
    case HMN_TOOL_HANABI: {
        nw4r::g3d::ResAnmTexSrt srt = file->GetResAnmTexSrt(0);
        if (srt.IsValid()) {
            mTexSrt.create(*mdl, srt, &mAllocator, NULL, 1);
            mTexSrt.setRate(0.0f, 0);
            mTexSrt.setPlayMode(m3d::FORWARD_ONCE, 0);
            mMdl.setAnm(mTexSrt);
            mTexSrtCreated = TRUE;
        }
        break;
    }
    case HMN_TOOL_PACHINKO:
        mFloat.fn_8016E1AC(3, &mItem);
        break;
    case HMN_TOOL_BALLOON: {
        nw4r::g3d::ResAnmClr clr = file->GetResAnmClr(0);
        if (clr.IsValid()) {
            mMatClr.create(*mdl, clr, &mAllocator, NULL, 1);
            mColor = dItem::seeker_c::get()->findLike(mItem) & 7;
            mMatClr.setFrame(mColor, 0);
            mMatClr.setPlayMode(m3d::FORWARD_ONCE, 0);
            mMdl.setAnm(mMatClr);
        }
        mpBalloon = dBalloonString_c::fn_80068108(mpHeap);
        setBalloonColor(0xFF);
        break;
    }
    case HMN_TOOL_WINDMILL: {
        nw4r::g3d::ResAnmTexSrt srt = file->GetResAnmTexSrt(0);
        if (srt.IsValid()) {
            mTexSrt.create(*mdl, srt, &mAllocator, NULL, 1);
            u8 idx = dItem::seeker_c::get()->findLike(mItem) & 7;
            mTexSrt.setFrame(idx, 0);
            mTexSrt.setRate(0.0f, 0);
            mTexSrt.setPlayMode(m3d::FORWARD_ONCE, 0);
            mMdl.setAnm(mTexSrt);
            mTexSrtCreated = TRUE;
            mWindState = 0;
            mWindSpeed = 0.0f;
            mWindAngle = 0;
            mCallback.mRot = 0;
        }
        nw4r::g3d::ResNode node = mdl->GetResNode("rot_windmill");
        mCallback.mNodeId = node.GetID();
        mMdl.setCallback(&mCallback);
        break;
    }
    case HMN_TOOL_SCOOP:
        setupDig(file);
        break;
    }
}

// 800BB2E4
void dHmnToolBank_c::setupAxe(nw4r::g3d::ResFile *file, nw4r::g3d::ResMdl *mdl) {
    if (mItem.getKind() == dItem::KIND_AXE) {
        if (file->fn_80234914()) {
            nw4r::g3d::ResAnmVis vis = file->GetResAnmVis(0);
            if (vis.IsValid()) {
                mVis.create(*mdl, vis, &mAllocator, NULL);
                mVis.mPlayMode = m3d::FORWARD_ONCE;
                mMdl.setAnm(mVis);
                mVisCreated = TRUE;
                s8 idx = 0;
                if (mType < dHmnToolMng_c::PLAYER_SLOT_NUM) {
                    dPrivateData_c *priv = dPlayerMgr_c::getNetPlayer(mType);
                    if (priv != NULL) {
                        idx = getAxeIdx(priv->mEquipment.mHeld);
                    }
                    if (idx == -1) {
                        idx = 0;
                    }
                }
                mVis.setFrame(idx);
            }
        }
        mAxe = TRUE;
    }
}

// 800BB410
void dHmnToolBank_c::setupDig(nw4r::g3d::ResFile *file) {
    mDigMode = 0;
    if (mType <= 3) {
        nw4r::g3d::ResMdl mdl0 = file->GetResMdl(1);
        mDigMdl0.create(mdl0, &mAllocator, 0, 1, NULL);
        nw4r::g3d::ResMdl mdl1 = file->GetResMdl(2);
        mDigMdl1.create(mdl1, &mAllocator, 0, 1, NULL);
        mMtx_c mtx;
        PSMTXIdentity(mtx);
        mDigMdl0.setLocalMtx(&mtx);
        mDigMdl1.setLocalMtx(&mtx);
        mDigMdlCreated = TRUE;
        nw4r::g3d::ResAnmChr chr = file->GetResAnmChr("t_dig");
        if (chr.IsValid()) {
            mDigAnm0.create(mdl0, chr, &mAllocator, NULL);
            mDigAnm0.mPlayMode = m3d::FORWARD_ONCE;
            mDigAnm0.setFrame(0.0f);
            mDigMdl0.setAnm(mDigAnm0);
            mDigAnm1.create(mdl1, chr, &mAllocator, NULL);
            mDigAnm1.mPlayMode = m3d::FORWARD_ONCE;
            mDigAnm1.setFrame(0.0f);
            mDigMdl1.setAnm(mDigAnm1);
            mDigAnmCreated = TRUE;
        }
        nw4r::g3d::ResAnmVis vis = file->GetResAnmVis("t_dig");
        if (vis.IsValid()) {
            mDigVis0.create(mdl0, vis, &mAllocator, NULL);
            mDigVis0.mPlayMode = m3d::FORWARD_ONCE;
            mDigVis0.setFrame(0.0f);
            mDigMdl0.setAnm(mDigVis0);
            mDigVis1.create(mdl1, vis, &mAllocator, NULL);
            mDigVis1.mPlayMode = m3d::FORWARD_ONCE;
            mDigVis1.setFrame(0.0f);
            mDigMdl1.setAnm(mDigVis1);
            mDigVisCreated = TRUE;
        }
    }
}

// 800BB63C
void dHmnToolBank_c::fn_800BB63C() {
    mWindSpeed = 0.0f;
    int type = getToolType();
    if (type == HMN_TOOL_ROD || type == HMN_TOOL_PACHINKO) {
        mFloat.fn_8016E37C();
    }
    resetHeap();
}

// 800BB690
void dHmnToolBank_c::setAnmRate(f32 rate) {
    if (mAnmCreated) {
        mAnm.setRate(rate);
    }
}

// 800BB6A8
f32 dHmnToolBank_c::getAnmRate() {
    if (mAnmCreated == FALSE) {
        return 0.0f;
    }
    return mAnm.getRate();
}

// 800BB6C8
void dHmnToolBank_c::setAnmFrame(f32 frame) {
    if (mAnmCreated) {
        mAnm.setFrame(frame);
    }
}

// 800BB6E0
f32 dHmnToolBank_c::getAnmFrame() {
    if (mAnmCreated == FALSE) {
        return 0.0f;
    }
    return mAnm.getFrame();
}

// 800BB700
f32 dHmnToolBank_c::getAnmFrameMax() {
    if (mAnmCreated == FALSE) {
        return 1.0f;
    }
    return mAnm.mFrameMax;
}

// 800BB71C
bool dHmnToolBank_c::isAnmStop() {
    if (mAnmCreated == FALSE) {
        return true;
    }
    return mAnm.isStop();
}

// 800BB73C
bool dHmnToolBank_c::checkAnmFrame(f32 frame) {
    if (mAnmCreated == FALSE) {
        return false;
    }
    return mAnm.checkFrame(frame);
}

// 800BB75C
BOOL dHmnToolBank_c::getNodeMtx(mMtx_c *mtx, ulong idx) {
    if (!mMdlCreated) {
        return FALSE;
    }
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    if (idx >= mdl.GetResNodeNumEntries()) {
        return FALSE;
    }
    nw4r::g3d::ResNode node = mdl.GetResNode(idx);
    mMdl.getNodeWorldMtx(node.GetID(), mtx);
    return TRUE;
}

// 800BB804
void dHmnToolBank_c::setLocalMtx(const mMtx_c *mtx) {
    if (mMdlCreated && mtx != NULL) {
        mMdl.setLocalMtx(mtx);
        int type = getToolType();
        if (type == HMN_TOOL_ROD || type == HMN_TOOL_PACHINKO) {
            mFloat.fn_8016E680(mtx);
        }
    }
}

// 800BB878
void dHmnToolBank_c::setAnm(const char *name, m3d::playMode_e mode, f32 blend) {
    if (!mMdlCreated || mItem.isSame(dItem::ITEM_ID_NONE)) {
        return;
    }
    nw4r::g3d::ResFile file(getResFile(TRUE));
    if (!file.IsValid() || !file.fn_8023487C()) {
        return;
    }
    if (!file.fn_802348C8()) {
        return;
    }
    nw4r::g3d::ResAnmChr anm = file.GetResAnmChr(name);
    if (!anm.IsValid()) {
        return;
    }
    mAnm.setAnm(mMdl, anm, mode);
    mMdl.setAnm(mAnm, blend);
}

// 800BB96C
void dHmnToolBank_c::setBodyAnm(u32 anmId, m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_NET) {
        if (anmId < 0x1A7 || anmId > 0x1BA) {
            return;
        }
        nw4r::g3d::ResAnmChr anm = dHmnAnm_c::getResAnmChr(anmId);
        if (anm.IsValid()) {
            mAnm.setAnm(mMdl, anm, mode);
            mMdl.setAnm(mAnm, blend);
        }
    }
}

// 800BBA10
void dHmnToolBank_c::setNetWait(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_NET) {
        setAnm("tf_net_wait1", mode, blend);
    }
}

// 800BBA74
void dHmnToolBank_c::setNetAnm1B6(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B6, mode, blend);
}

// 800BBA80
void dHmnToolBank_c::setNetAnm1AA(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AA, mode, blend);
}

// 800BBA8C
void dHmnToolBank_c::setNetAnm1A9(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1A9, mode, blend);
}

// 800BBA98
void dHmnToolBank_c::setNetAnm1A7(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1A7, mode, blend);
}

// 800BBAA4
void dHmnToolBank_c::setNetAnm1B3(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B3, mode, blend);
}

// 800BBAB0
void dHmnToolBank_c::setNetAnm1B7(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B7, mode, blend);
}

// 800BBABC
void dHmnToolBank_c::setNetAnm1B8(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B8, mode, blend);
}

// 800BBAC8
void dHmnToolBank_c::setNetAnm1AF(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AF, mode, blend);
}

// 800BBAD4
void dHmnToolBank_c::setNetAnm1B0(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B0, mode, blend);
}

// 800BBAE0
void dHmnToolBank_c::setNetAnm1B1(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B1, mode, blend);
}

// 800BBAEC
void dHmnToolBank_c::setNetAnm1B2(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B2, mode, blend);
}

// 800BBAF8
void dHmnToolBank_c::setNetAnm1B5(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B5, mode, blend);
}

// 800BBB04
void dHmnToolBank_c::setNetAnm1AD(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AD, mode, blend);
}

// 800BBB10
void dHmnToolBank_c::setNetAnm1AB(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AB, mode, blend);
}

// 800BBB1C
void dHmnToolBank_c::setNetAnm1AC(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AC, mode, blend);
}

// 800BBB28
void dHmnToolBank_c::setNetAnm1AE(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1AE, mode, blend);
}

// 800BBB34
void dHmnToolBank_c::setNetAnm1B9(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B9, mode, blend);
}

// 800BBB40
void dHmnToolBank_c::setNetAnm1B4(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1B4, mode, blend);
}

// 800BBB4C
void dHmnToolBank_c::setNetAnm1A8(m3d::playMode_e mode, f32 blend) {
    setBodyAnm(0x1A8, mode, blend);
}

// 800BBB58
void dHmnToolBank_c::setPoleWait(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("tf_pole_wait1", mode, blend);
        mFloat.fn_8016FC44(1);
    }
}

// 800BBBC8
void dHmnToolBank_c::setPoleSwing(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_swing1", mode, blend);
        mFloat.fn_8016FC44(3);
    }
}

// 800BBC38
void dHmnToolBank_c::setPoleSuka(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_suka1", mode, blend);
        mFloat.fn_8016FC44(2);
    }
}

// 800BBCA8
void dHmnToolBank_c::setPoleWaitNibble(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_wait_nibble1", mode, blend);
    }
}

// 800BBD0C
void dHmnToolBank_c::setPoleHit(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_hit1", mode, blend);
        mFloat.fn_8016FC44(8);
    }
}

// 800BBD7C
void dHmnToolBank_c::setPolePutback(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_putback1", mode, blend);
        mFloat.fn_8016FC44(9);
    }
}

// 800BBDEC
void dHmnToolBank_c::setPoleFail(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_fail1", mode, blend);
        mFloat.fn_8016FC44(10);
    }
}

// 800BBE5C
void dHmnToolBank_c::setPoleGet1(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_get1", mode, blend);
        mFloat.fn_8016FC44(9);
    }
}

// 800BBECC
void dHmnToolBank_c::setPoleGet2(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_get2", mode, blend);
    }
}

// 800BBF30
void dHmnToolBank_c::setPolePutaway(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_putaway1", mode, blend);
    }
}

// 800BBF94
void dHmnToolBank_c::setPoleComplete(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_complete1", mode, blend);
    }
}

// 800BBFF8
void dHmnToolBank_c::setPoleTumbleDown(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_tumble_downB1", mode, blend);
    }
}

// 800BC05C
void dHmnToolBank_c::setPoleTumbleUp(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_tumble_upB1", mode, blend);
    }
}

// 800BC0C0
void dHmnToolBank_c::setPoleStingDown(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_ROD) {
        setAnm("ts_pole_sting_downB1", mode, blend);
    }
}

// 800BC124
void dHmnToolBank_c::openUmbrella() {
    if (getToolType() == HMN_TOOL_UMBRELLA) {
        mAnm.setFrame(0.0f);
        mAnm.setRate(1.0f);
        mAnm.mPlayMode = m3d::FORWARD_ONCE;
    }
}

// 800BC178
void dHmnToolBank_c::closeUmbrella() {
    if (getToolType() == HMN_TOOL_UMBRELLA) {
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        mAnm.setRate(1.0f);
        mAnm.mPlayMode = m3d::REVERSE_ONCE;
    }
}

// 800BC1D4
void dHmnToolBank_c::startHanabi() {
    if (getToolType() == HMN_TOOL_HANABI) {
        if (mAnmCreated) {
            mAnm.setFrame(0.0f);
            mAnm.setRate(1.0f);
            mAnm.mPlayMode = m3d::FORWARD_ONCE;
        }
        if (mTexSrtCreated) {
            mTexSrt.setFrame(0.0f, 0);
            mTexSrt.setRate(1.0f, 0);
            mTexSrt.setPlayMode(m3d::FORWARD_ONCE, 0);
        }
    }
}

// 800BC270
void dHmnToolBank_c::setSlingShoot1(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_PACHINKO) {
        setAnm("ts_sling_shoot1", mode, blend);
    }
}

// 800BC2D4
void dHmnToolBank_c::setSlingShoot2(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_PACHINKO) {
        setAnm("ts_sling_shoot2", mode, blend);
    }
}

// 800BC338
void dHmnToolBank_c::setSlingShoot3(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_PACHINKO) {
        setAnm("ts_sling_shoot3", mode, blend);
    }
}

// 800BC39C
void dHmnToolBank_c::setSlingWait(m3d::playMode_e mode, f32 blend) {
    if (getToolType() == HMN_TOOL_PACHINKO) {
        setAnm("tf_sling_wait1", mode, blend);
    }
}

// 800BC400
void dHmnToolBank_c::setBalloonItem(const dItem::Item *item) {
    if (mpBalloon != NULL && !mpBalloon->mFly) {
        mpBalloon->fn_80067BF0(*item);
    }
}

// 800BC448
void dHmnToolBank_c::calcBalloonInit(const mMtx_c *mtx) {
    if (mpBalloon != NULL) {
        mpBalloon->calcInit(mtx);
    }
}

// 800BC468
u8 dHmnToolBank_c::isBalloonFly() {
    if (mpBalloon == NULL) {
        return FALSE;
    }
    return mpBalloon->mFly;
}

// 800BC484
BOOL dHmnToolBank_c::isBalloonGone() {
    dBalloonString_c *balloon = mpBalloon;
    if (balloon == NULL) {
        return TRUE;
    }
    return balloon->mFly && balloon->mTimer == 0;
}

// 800BC4BC
void dHmnToolBank_c::startFlower() {
    if (mAnmCreated && getToolType() == HMN_TOOL_FLOWER) {
        mAnm.setFrame(0.0f);
        mAnm.setRate(1.0f);
        mAnm.mPlayMode = m3d::FORWARD_ONCE;
    }
}

// 800BC51C
void dHmnToolBank_c::calcWind(const mVec3_c *pos) {
    if (getToolType() == HMN_TOOL_WINDMILL) {
        if (mWindState != 1) {
            mWindPos = *pos;
            mWindState = 1;
        } else {
            mVec3_c diff = mWindPos - *pos;
            f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
            if (dist >= 1.1f) {
                if (dist > 3.0f) {
                    dist = 3.0f;
                }
                sLib::chase(&mWindSpeed, dist, dist > mWindSpeed ? 1.1f : 0.015f);
            } else {
                sLib::chase(&mWindSpeed, 0.0f, 0.015f);
            }
            mWindAngle += (s16)(600.0f * (mWindSpeed * mWindSpeed));
            mCallback.mRot = mWindAngle;
            mWindPos = *pos;
        }
    }
}

// 800BC64C
void dHmnToolBank_c::stopWind() {
    if (getToolType() == HMN_TOOL_WINDMILL) {
        mWindState = 2;
    }
}

// 800BC688
void dHmnToolBank_c::calcWindmill() {
    switch (mWindState) {
    case 1:
        break;
    case 2:
        mWindAngle += (s16)(600.0f * (mWindSpeed * mWindSpeed));
        mCallback.mRot = mWindAngle;
        if (sLib::chase(&mWindSpeed, 0.0f, 0.015f)) {
            mWindState = 0;
        }
        break;
    }
}

// 800BC710
f32 dHmnToolBank_c::getWindRate() {
    return mWindSpeed / 3.0f;
}

// 800BC720
void dHmnToolBank_c::setDigAnm(const mVec3_c *pos, const mAng3_c *angle, int anm, BOOL dig, u8 alpha) {
    static const char *sAnmNames[] = {"t_dig", "t_dig_stump", "t_dig_get", "t_bury"};

    if (!mMdlCreated || mItem.isSame(dItem::ITEM_ID_NONE)) {
        return;
    }
    if (getToolType() != HMN_TOOL_SCOOP) {
        return;
    }
    nw4r::g3d::ResFile file(getResFile(TRUE));
    if (!file.IsValid() || !file.fn_8023487C()) {
        return;
    }
    const char *name = sAnmNames[anm];
    nw4r::g3d::ResAnmChr chr = file.GetResAnmChr(name);
    if (!chr.IsValid()) {
        return;
    }
    nw4r::g3d::ResAnmVis vis = file.GetResAnmVis(name);
    if (!vis.IsValid()) {
        return;
    }
    if (!mDigAnmCreated || !mDigVisCreated) {
        return;
    }

    mDigPos = *pos;
    mDigAngle = *angle;
    if (dig) {
        mDigMode = 1;
        nw4r::g3d::ResFile pack(dBG::getPackBank()->getData());
        if (pack.IsValid()) {
            mDigMdl0.getResMdl().ReleaseTexByName("tex_grass");
            mDigMdl0.getResMdl().ReleaseTexByName("tex_soil");
            mDigMdl0.getResMdl().Bind(pack);
        }
        nw4r::g3d::ResMdl mdl = mDigMdl0.getResMdl();
        nw4r::g3d::ResMat mat = mdl.GetResMat("mat_block");
        if (!mat.IsValid()) {
            return;
        }
        nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
        GXColor color;
        if (tev.GXGetTevColor(GX_TEVPREV, &color)) {
            color.a = alpha;
            tev.GXSetTevColor(GX_TEVPREV, color);
            tev.DCStore(false);
        }
        mDigAnm0.setAnm(mDigMdl0, chr, m3d::FORWARD_ONCE);
        mDigAnm0.setFrame(0.0f);
        mDigMdl0.setAnm(mDigAnm0);
        mDigVis0.setAnm(mDigMdl0, vis, m3d::FORWARD_ONCE);
        mDigVis0.setFrame(0.0f);
        mDigMdl0.setAnm(mDigVis0);
    } else {
        mDigMode = 2;
        mDigAnm1.setAnm(mDigMdl1, chr, m3d::FORWARD_ONCE);
        mDigAnm1.setFrame(0.0f);
        mDigMdl1.setAnm(mDigAnm1);
        mDigVis1.setAnm(mDigMdl1, vis, m3d::FORWARD_ONCE);
        mDigVis1.setFrame(0.0f);
        mDigMdl1.setAnm(mDigVis1);
    }
}

// 800BCA48
BOOL dHmnToolBank_c::hasResMdl() {
    nw4r::g3d::ResFile file(getResFile(FALSE));
    BOOL ret = FALSE;
    BOOL valid = FALSE;
    if (file.IsValid() && file.fn_8023487C()) {
        valid = TRUE;
    }
    if (valid) {
        if (file.GetResMdl(0).IsValid()) {
            ret = TRUE;
        }
    }
    return ret;
}

// 800BCAC8
void dHmnToolBank_c::removeModels() {
    if (mMdlCreated) {
        mMdl.setCallback(NULL);
        if (mAnmCreated) {
            mAnm.remove();
            mAnmCreated = FALSE;
        }
        if (mVisCreated) {
            mVis.remove();
            mVisCreated = FALSE;
        }
        if (mTexSrtCreated) {
            mTexSrt.remove();
            mTexSrtCreated = FALSE;
        }
        if (mMatClr.IsBound()) {
            mMatClr.remove();
        }
        mAxe = FALSE;
        mMdl.remove();
        mMdlCreated = FALSE;
        mItem = dItem::ITEM_ID_NONE;
        if (mDigAnmCreated) {
            mDigAnm0.remove();
            mDigAnm1.remove();
            mDigAnmCreated = FALSE;
        }
        if (mDigVisCreated) {
            mDigVis0.remove();
            mDigVis1.remove();
            mDigVisCreated = FALSE;
        }
        if (mDigMdlCreated) {
            if (mType <= 3) {
                mDigMdl0.getResMdl().ReleaseTexByName("tex_grass");
                mDigMdl0.getResMdl().ReleaseTexByName("tex_soil");
                nw4r::g3d::ResFile file(getResFile(TRUE));
                if (file.IsValid() && file.fn_8023487C()) {
                    file.Release();
                    file.Bind();
                }
            }
            mDigMdl0.remove();
            mDigMdl1.remove();
            mDigMdlCreated = FALSE;
        }
    }
    if (mpBalloon != NULL) {
        mpBalloon->fn_800680F4();
        mpBalloon = NULL;
    }
}

// 800BCD28
void dHmnToolBank_c::resetHeap() {
    if (mMdlCreated) {
        removeModels();
        if (mpHeap != NULL) {
            mpHeap->free(3);
        }
        mAllocator.attach(mpHeap, 0x20);
    }
}

// 800BCD84
void dHmnToolBank_c::setBalloonColor(u8 alpha) {
    if (mpBalloon == NULL) {
        return;
    }
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat(0);
    if (!mat.IsValid()) {
        return;
    }
    if (alpha < 0xFF) {
        nw4r::g3d::ResMatPix pix = mat.GetResMatPix();
        pix.GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        pix.GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        pix.GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        pix.DCStore(false);
    }
    GXColor color;
    nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
    tev.GXGetTevColor(GX_TEVPREV, &color);
    color.a = alpha;
    tev.GXSetTevColor(GX_TEVPREV, color);
    tev.DCStore(false);
    if (mMatClr.IsBound()) {
        mMatClr.setFrame(mColor, 0);
    }
}

// 800BCEF8
s8 dHmnToolBank_c::getAxeIdx(const dItem::Item &item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return -1;
    }
    if (bitm->getKind() != dItem::KIND_AXE) {
        return -1;
    }
    return dItem::seeker_c::get()->findLike(item);
}

// 800BCF84
void dHmnToolBank_c::setAxeItem(const dItem::Item &item) {
    if (!mAxe || !mVisCreated) {
        return;
    }
    s8 idx = getAxeIdx(item);
    if (idx != -1) {
        mVis.setFrame(idx);
        mItem = item;
    }
}

// 800BD010
u16 dHmnToolBank_c::getNextAxe() {
    s8 idx = getAxeIdx(mItem);
    if (idx == -1) {
        return dItem::Item(0x997).mId;
    }
    if (idx == 7) {
        return dItem::ITEM_ID_NONE;
    }
    return dItem::Item(0x996, (s8)(idx + 1), FALSE).mId;
}

// 800BD084
BOOL dHmnToolBank_c::isOrgUmbrella(u32 *design, const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    if (bitm->getKind() != dItem::KIND_ORG_UMB) {
        return FALSE;
    }
    int idx = dItem::seeker_c::get()->findLike(*item);
    if (idx == -1) {
        return FALSE;
    }
    *design = idx & 7;
    return TRUE;
}

// 800BD134
BOOL dHmnToolBank_c::isHandTool(const dItem::Item *item) {
    static const u8 sTable[HMN_TOOL_NUM] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};
    u32 type = getItemToolType(item);
    if (type >= HMN_TOOL_NUM) {
        return FALSE;
    }
    return sTable[type];
}

// 800BD170
BOOL dHmnToolBank_c::syncLoad() {
    if (mType >= dHmnToolMng_c::SLOT_NUM) {
        return TRUE;
    }
    dHmnToolMng_c::data_c *data = l_toolMng.getData(mType);
    if (data == NULL) {
        return TRUE;
    }
    int buf = l_toolMng.getLoadBuffer(data);
    if (data->mModel[buf].fn_800862C8() && data->mDesign[buf].fn_800862C8()) {
        return TRUE;
    }
    return FALSE;
}

// 800BD208
void dHmnToolMng_c::bindDesign(data_c *data) {
    int buf = getLoadBuffer(data);
    nw4r::g3d::ResFile file(data->mModel[buf].getData());
    void *design = data->mDesign[buf].getData();
    if (!file.IsValid() || design == NULL) {
        return;
    }
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mdl.ReleaseTexByName("cloth");
    mdl.ReleasePlttByName("cloth");
    file.Bind(nw4r::g3d::ResFile(design));
}

// 800BD294
int dHmnToolMng_c::getHeapSize() {
    return ((dUki_c::fn_801710F4() + 0x1A00) * 9 + 3) & ~3;
}

// 800BD2C8
BOOL dHmnToolMng_c::create(EGG::Heap *parent) {
    return l_toolMng.createHeap(parent);
}

// 800BD2D8
void dHmnToolMng_c::clearPlayers() {
    l_toolMng.clearPlayerItems();
}

// 800BD2E4
void dHmnToolMng_c::clear(int slot) {
    l_toolMng.clearItem(slot);
}

// 800BD2F4
int dHmnToolMng_c::getBankHeapSize() {
    return 0xAB00;
}
