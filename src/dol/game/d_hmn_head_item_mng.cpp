// Player head items (caps and accessories). .text 800B81F0..800B8EE0.
// See include/game/game/d_hmn_head_item_mng.hpp.
#include <game/game/d_hmn_head_item_mng.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <nw4r/g3d/res/g3d_resnode.h>
#include <string.h>

static dHmnHeadItemMng_c l_headItemMng;

// 800B81F0
dHmnHeadItemMng_c::data_c::data_c() {
    mCur = 0;
    mLoadState[0] = LOAD_NONE;
    mLoadState[1] = LOAD_NONE;
    memset(mpHeap, 0, sizeof(mpHeap));
    mState = STATE_IDLE;
    mItemId = dItem::ITEM_ID_NONE;
    mReqItem = dItem::ITEM_ID_NONE;
    mLocked = BUFFER_NUM;
}

// 800B82A4
dHmnHeadItemMng_c::data_c::~data_c() {}

// 800B8328
dHmnHeadItemMng_c::dHmnHeadItemMng_c() {}

// 800B8370
dHmnHeadItemMng_c::~dHmnHeadItemMng_c() {}

// 800B83D4
BOOL dHmnHeadItemMng_c::createHeap(EGG::Heap *parent) {
    data_c *cap = mData[KIND_CAP];
    data_c *acc = mData[KIND_ACCESSORY];
    for (int i = 0; i < SLOT_NUM; i++, cap++, acc++) {
        for (int j = 0; j < BUFFER_NUM; j++) {
            if (cap->mpHeap[j] == NULL) {
                cap->mpHeap[j] =
                    mHeap::createFrmHeap(0x5380, parent, "dHmnHeadItemMng_c::createHeap::cap", 0x20, mHeap::OPT_NONE);
            }
            if (acc->mpHeap[j] == NULL) {
                acc->mpHeap[j] = mHeap::createFrmHeap(0x5000, parent, "dHmnHeadItemMng_c::createHeap::accessory",
                                                      0x20, mHeap::OPT_NONE);
            }
        }
    }
    return TRUE;
}

// 800B84A0
void dHmnHeadItemMng_c::clearPlayerItems() {
    for (int i = 0; i < SLOT_NUM; i++) {
        for (int kind = 0; kind < KIND_NUM; kind++) {
            mData[kind][i].mItemId = dItem::ITEM_ID_NONE;
        }
    }
}

// 800B84CC
void dHmnHeadItemMng_c::clearItem(int slot) {
    mData[KIND_CAP][slot].mItemId = dItem::ITEM_ID_NONE;
}

// 800B84E4
dHmnHeadItem_c::dHmnHeadItem_c() {}

// 800B84E8
dHmnHeadItem_c::~dHmnHeadItem_c() {}

// 800B8528
void dHmnHeadItem_c::setSlot(u8 slot) {
    mSlot = slot;
}

// 800B8530
BOOL dHmnHeadItem_c::requestCap(const dItem::Item *item, dPrivateData_c *priv) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm == NULL) {
            return TRUE;
        }
        int bone = (s8)bitm->m_hideBone;
        bone = (u32)bone < 0xB ? bone : 0xA;
        if (bone >= 8 || bone < 0) {
            return TRUE;
        }
    }
    int idx;
    BOOL isOrg = FALSE;
    if (dHmnHeadItemMng_c::isOrgCap(&idx, item)) {
        isOrg = TRUE;
    }
    return request(dHmnHeadItemMng_c::KIND_CAP, item, priv, isOrg);
}

// 800B860C
BOOL dHmnHeadItem_c::requestAccessory(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm == NULL) {
            return TRUE;
        }
        int bone = (s8)bitm->m_hideBone;
        bone = (u32)bone < 0xB ? bone : 0xA;
        if (bone != 8) {
            return TRUE;
        }
    }
    return request(dHmnHeadItemMng_c::KIND_ACCESSORY, item, NULL, FALSE);
}

// 800B86B0
BOOL dHmnHeadItem_c::request(int kind, const dItem::Item *item, dPrivateData_c *priv, BOOL isOrg) const {
    if (mSlot >= dHmnHeadItemMng_c::SLOT_NUM) {
        return TRUE;
    }
    if (kind != dHmnHeadItemMng_c::KIND_CAP && kind != dHmnHeadItemMng_c::KIND_ACCESSORY) {
        return TRUE;
    }

    dHmnHeadItemMng_c::data_c *data = l_headItemMng.getData(kind, mSlot);
    if (data->mState == dHmnHeadItemMng_c::STATE_IDLE) {
        if (data->mItemId == item->mId && !isOrg && data->mLoadState[data->mCur] == dHmnHeadItemMng_c::LOAD_DONE) {
            if (item->mId == dItem::ITEM_ID_NONE) {
                return TRUE;
            }
            if (getResFile(kind) != NULL) {
                return TRUE;
            }
        }
        data->mReqItem = *item;
        data->mState = dHmnHeadItemMng_c::STATE_RELEASE;
    }

    switch (data->mState) {
    case dHmnHeadItemMng_c::STATE_RELEASE:
        if (l_headItemMng.release(data)) {
            data->mState = dHmnHeadItemMng_c::STATE_LOAD;
        }
        break;
    case dHmnHeadItemMng_c::STATE_LOAD:
        if (l_headItemMng.load(data, priv)) {
            if (item->mId == data->mReqItem.mId) {
                l_headItemMng.finish(data);
                return TRUE;
            }
            data->mReqItem = *item;
            data->mState = dHmnHeadItemMng_c::STATE_RELEASE;
        }
        break;
    }
    return FALSE;
}

// 800B8864
void dHmnHeadItem_c::lockCap() {
    lock(dHmnHeadItemMng_c::KIND_CAP);
}

// 800B886C
void dHmnHeadItem_c::lockAccessory() {
    lock(dHmnHeadItemMng_c::KIND_ACCESSORY);
}

// 800B8874
void dHmnHeadItem_c::lock(int kind) {
    if (kind >= dHmnHeadItemMng_c::KIND_NUM || kind < 0) {
        return;
    }
    if (mSlot >= dHmnHeadItemMng_c::SLOT_NUM) {
        return;
    }
    dHmnHeadItemMng_c::data_c *data = l_headItemMng.getData(kind, mSlot);
    data->mLocked = data->mCur;
}

// 800B88B8
void *dHmnHeadItem_c::getCapResFile() {
    return getResFile(dHmnHeadItemMng_c::KIND_CAP);
}

// 800B88C0
BOOL dHmnHeadItem_c::syncLoad() {
    if (mSlot >= dHmnHeadItemMng_c::SLOT_NUM) {
        return TRUE;
    }
    BOOL capDone = FALSE;
    BOOL accDone = FALSE;

    dHmnHeadItemMng_c::data_c *data = l_headItemMng.getData(dHmnHeadItemMng_c::KIND_CAP, mSlot);
    if (data == NULL) {
        return TRUE;
    }
    int buf = l_headItemMng.getLoadBuffer(data);
    if (data->mLoader[buf].fn_800862C8() && data->mTexLoader[buf].fn_800862C8()) {
        capDone = TRUE;
    }

    data = l_headItemMng.getData(dHmnHeadItemMng_c::KIND_ACCESSORY, mSlot);
    if (data == NULL) {
        return TRUE;
    }
    buf = l_headItemMng.getLoadBuffer(data);
    if (data->mLoader[buf].fn_800862C8() && data->mTexLoader[buf].fn_800862C8()) {
        accDone = TRUE;
    }

    return capDone && accDone;
}

// 800B89CC
BOOL dHmnHeadItemMng_c::isOrgCap(int *design, const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    if (bitm->getKind() != dItem::KIND_ORG_CAP) {
        return FALSE;
    }
    int idx = dItem::seeker_c::get()->findLike(*item);
    if (idx == -1) {
        return FALSE;
    }
    *design = idx & 7;
    return TRUE;
}

// 800B8A7C
void *dHmnHeadItem_c::getAccessoryResFile() {
    return getResFile(dHmnHeadItemMng_c::KIND_ACCESSORY);
}

// 800B8A84
void *dHmnHeadItem_c::getResFile(int kind) const {
    if (kind >= dHmnHeadItemMng_c::KIND_NUM || kind < 0) {
        return NULL;
    }
    if (mSlot >= dHmnHeadItemMng_c::SLOT_NUM) {
        return NULL;
    }
    dHmnHeadItemMng_c::data_c *data = l_headItemMng.getData(kind, mSlot);
    u8 cur = data->mCur;
    if (data->mLoadState[cur] == dHmnHeadItemMng_c::LOAD_DONE) {
        if (data->mItemId == dItem::ITEM_ID_NONE) {
            return NULL;
        }
        return data->mLoader[cur].getData();
    }
    return NULL;
}

// 800B8B08
BOOL dHmnHeadItemMng_c::load(data_c *data, dPrivateData_c *priv) {
    if (data == NULL) {
        return FALSE;
    }
    int buf = getLoadBuffer(data);
    if (!data->mReqItem.isValid()) {
        data->mLoadState[buf] = LOAD_DONE;
        return TRUE;
    }
    if (data->mLoadState[buf] <= LOAD_BUSY) {
        if (data->mLoader[buf].loadItem(data->mReqItem, data->mpHeap[buf])) {
            int idx = 0;
            if (priv != NULL && isOrgCap(&idx, &data->mReqItem)) {
                if (data->mTexLoader[buf].loadDesign(&priv->mOrgDesigns.mDesigns[idx & 7], data->mpHeap[buf], 0x20,
                                                     0x80))
                {
                    data->mLoadState[buf] = LOAD_DONE;
                    setupModel(data, priv);
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

// 800B8C58
BOOL dHmnHeadItemMng_c::release(data_c *data) {
    if (data == NULL) {
        return FALSE;
    }
    int buf = getLoadBuffer(data);
    u8 state = data->mLoadState[buf];
    if (state == LOAD_BUSY || state == LOAD_DONE) {
        if (data->mLoader[buf].release() == TRUE && data->mTexLoader[buf].release() == TRUE) {
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

// 800B8D28
int dHmnHeadItemMng_c::getLoadBuffer(data_c *data) {
    if (data->mLocked == data->mCur) {
        return data->mCur == 0;
    }
    return data->mCur;
}

// 800B8D44
void dHmnHeadItemMng_c::setupModel(data_c *data, dPrivateData_c *priv) {
    int buf = getLoadBuffer(data);
    nw4r::g3d::ResFile file(data->mLoader[buf].getData());
    void *tex = data->mTexLoader[buf].getData();
    if (!file.IsValid() || tex == NULL) {
        return;
    }

    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mdl.ReleaseTexByName("cloth");
    mdl.ReleasePlttByName("cloth");
    file.Bind(nw4r::g3d::ResFile(tex));

    nw4r::g3d::ResNode boy = mdl.GetResNode("cap_md_boy_mdl");
    nw4r::g3d::ResNode girl = mdl.GetResNode("cap_md_girl_mdl");
    if (!boy.IsValid() || !girl.IsValid()) {
        return;
    }
    if (priv->mPID.player.mGender == 0) {
        boy.SetVisibility(true);
        girl.SetVisibility(false);
    } else {
        boy.SetVisibility(false);
        girl.SetVisibility(true);
    }
}

// 800B8E6C
BOOL dHmnHeadItemMng_c::create(EGG::Heap *parent) {
    return l_headItemMng.createHeap(parent);
}

// 800B8E7C
void dHmnHeadItemMng_c::clearPlayers() {
    l_headItemMng.clearPlayerItems();
}

// 800B8E88
void dHmnHeadItemMng_c::clear(int slot) {
    l_headItemMng.clearItem(slot);
}
