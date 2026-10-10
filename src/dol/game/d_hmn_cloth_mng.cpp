// Human clothing models. .text 800B6CF0..800B76BC. See include/game/game/d_hmn_cloth_mng.hpp.
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_heap.hpp>
#include <string.h>

static dHmnClothMng_c l_clothMng;

// 800B6CF0
dHmnClothMng_c::data_c::data_c() {
    mCur = 0;
    mLoadState[0] = LOAD_NONE;
    mLoadState[1] = LOAD_NONE;
    memset(mpHeap, 0, sizeof(mpHeap));
    mState = STATE_IDLE;
    mItemId = dItem::ITEM_ID_NONE;
    mReqItem = dItem::ITEM_ID_NONE;
    mLocked = BUFFER_NUM;
}

// 800B6D8C
dHmnClothMng_c::data_c::~data_c() {}

// 800B6DF0
dHmnClothMng_c::dHmnClothMng_c() {}

// 800B6E38
dHmnClothMng_c::~dHmnClothMng_c() {}

// 800B6E9C
BOOL dHmnClothMng_c::createHeap(EGG::Heap *parent) {
    data_c *data = mData;
    for (int i = 0; i < SLOT_NUM; i++, data++) {
        for (int j = 0; j < BUFFER_NUM; j++) {
            if (data->mpHeap[j] == NULL) {
                data->mpHeap[j] =
                    mHeap::createFrmHeap(0x980, parent, "dHmnClothMng_c::createHeap::data", 0x20, mHeap::OPT_NONE);
            }
        }
    }
    return TRUE;
}

// 800B6F2C
void dHmnClothMng_c::clearPlayerItems() {
    for (int i = 0; i < PLAYER_SLOT_NUM; i++) {
        mData[i].mItemId = dItem::ITEM_ID_NONE;
    }
}

// 800B6F48
void dHmnClothMng_c::clearItem(int slot) {
    mData[slot].mItemId = dItem::ITEM_ID_NONE;
}

// 800B6F60
dHmnCloth_c::dHmnCloth_c() {
    mSlot = dHmnClothMng_c::SLOT_NUM;
}

// 800B6F6C
dHmnCloth_c::~dHmnCloth_c() {}

// 800B6FAC
void dHmnCloth_c::setSlot(int slot) {
    mSlot = slot;
}

// 800B6FB4
BOOL dHmnCloth_c::request(const dItem::Item *item, dPrivateData_c *priv, dDesign_c *design) {
    BOOL isOrg = FALSE;
    BOOL valid = TRUE;
    if (mSlot >= dHmnClothMng_c::SLOT_NUM) {
        valid = FALSE;
    }
    if (item->mId == dItem::ITEM_ID_NONE) {
        valid = FALSE;
    }
    if (!dHmnClothMng_c::isCloth(item)) {
        if (priv == NULL && design == NULL) {
            valid = FALSE;
        } else {
            int idx = 0;
            BOOL notOrg = !dHmnClothMng_c::isOrgCloth(&idx, item);
            if (notOrg) {
                valid = FALSE;
            }
            if (!notOrg) {
                isOrg = TRUE;
            }
        }
    }

    dHmnClothMng_c::data_c *data = l_clothMng.getData(mSlot);
    if (data->mState == dHmnClothMng_c::STATE_IDLE) {
        if (data->mItemId == item->mId && !isOrg && valid &&
            data->mLoadState[data->mCur] == dHmnClothMng_c::LOAD_DONE && getResFile() != NULL)
        {
            return TRUE;
        }
        data->mReqItem = *item;
        if (!valid) {
            data->mReqItem = dItem::Item(0x4A4);
        }
        data->mState = dHmnClothMng_c::STATE_RELEASE;
    }

    switch (data->mState) {
    case dHmnClothMng_c::STATE_RELEASE:
        if (l_clothMng.release(data)) {
            data->mState = dHmnClothMng_c::STATE_LOAD;
        }
        break;
    case dHmnClothMng_c::STATE_LOAD:
        if (l_clothMng.load(data, priv, design)) {
            if (item->mId == data->mReqItem.mId) {
                l_clothMng.finish(data);
                return TRUE;
            }
            if (!valid) {
                l_clothMng.finish(data);
                return TRUE;
            }
            data->mReqItem = *item;
            data->mState = dHmnClothMng_c::STATE_RELEASE;
        }
        break;
    default:
        return TRUE;
    }
    return FALSE;
}

// 800B71C4
void dHmnCloth_c::lock() {
    if (mSlot < dHmnClothMng_c::SLOT_NUM) {
        dHmnClothMng_c::data_c *data = l_clothMng.getData(mSlot);
        data->mLocked = data->mCur;
    }
}

// 800B71EC
BOOL dHmnCloth_c::syncLoad() {
    if (mSlot >= dHmnClothMng_c::SLOT_NUM) {
        return TRUE;
    }
    dHmnClothMng_c::data_c *data = l_clothMng.getData(mSlot);
    return data->mLoader[l_clothMng.getLoadBuffer(data)].fn_800862C8();
}

// 800B7248
void *dHmnCloth_c::getResFile() {
    if (mSlot >= dHmnClothMng_c::SLOT_NUM) {
        return NULL;
    }
    dHmnClothMng_c::data_c *data = l_clothMng.getData(mSlot);
    u8 cur = data->mCur;
    if (data->mLoadState[cur] == dHmnClothMng_c::LOAD_DONE && data->mItemId != dItem::ITEM_ID_NONE) {
        return data->mLoader[cur].getData();
    }
    return NULL;
}

// 800B72A4
BOOL dHmnClothMng_c::isCloth(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    if (bitm->getKind() != dItem::KIND_CLOTH) {
        return FALSE;
    }
    return TRUE;
}

// 800B7310
BOOL dHmnClothMng_c::isOrgCloth(int *design, const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    if (bitm->getKind() != dItem::KIND_ORG_CLOTH) {
        return FALSE;
    }
    int idx = dItem::seeker_c::get()->findLike(*item);
    if (idx == -1) {
        return FALSE;
    }
    if (design != NULL) {
        *design = idx & 7;
    }
    return TRUE;
}

// 800B73C8
BOOL dHmnClothMng_c::load(data_c *data, dPrivateData_c *priv, dDesign_c *design) {
    if (data == NULL) {
        return TRUE;
    }
    int buf = getLoadBuffer(data);
    if (data->mLoadState[buf] <= LOAD_BUSY) {
        BOOL ok;
        int idx = 0;
        if (isOrgCloth(&idx, &data->mReqItem)) {
            if (priv != NULL) {
                ok = data->mLoader[buf].loadDesign(&priv->mOrgDesigns.mDesigns[idx & 7], data->mpHeap[buf], 0x20, 0x80);
            } else if (design != NULL) {
                ok = data->mLoader[buf].loadDesign(design, data->mpHeap[buf], 0x20, 0x80);
            } else {
                ok = TRUE;
            }
        } else {
            ok = data->mLoader[buf].loadItem(data->mReqItem, data->mpHeap[buf]);
        }

        if (ok) {
            data->mLoadState[buf] = LOAD_DONE;
            return TRUE;
        }
        data->mLoadState[buf] = LOAD_BUSY;
        return FALSE;
    }
    return TRUE;
}

// 800B750C
BOOL dHmnClothMng_c::release(data_c *data) {
    if (data == NULL) {
        return FALSE;
    }
    int buf = getLoadBuffer(data);
    u8 state = data->mLoadState[buf];
    if (state == LOAD_BUSY || state == LOAD_DONE) {
        if (data->mLoader[buf].release() == TRUE) {
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

// 800B75C0
int dHmnClothMng_c::getLoadBuffer(data_c *data) {
    if (data->mLocked == data->mCur) {
        return data->mCur == 0;
    }
    return data->mCur;
}

// 800B75DC
void dHmnClothMng_c::finish(data_c *data) {
    if (data != NULL) {
        int buf = l_clothMng.getLoadBuffer(data);
        if (data->mLocked == data->mCur) {
            data->mCur = buf;
        }
        data->mItemId = data->mReqItem.mId;
        data->mReqItem = dItem::ITEM_ID_NONE;
        data->mState = STATE_IDLE;
    }
}

// 800B7648
BOOL dHmnClothMng_c::create(EGG::Heap *parent) {
    return l_clothMng.createHeap(parent);
}

// 800B7658
void dHmnClothMng_c::clearPlayers() {
    l_clothMng.clearPlayerItems();
}

// 800B7664
void dHmnClothMng_c::clear(int slot) {
    l_clothMng.clearItem(slot);
}
