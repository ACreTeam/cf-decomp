#pragma once

// Player head items (caps and accessories). DOL TU d_hmn_head_item_mng.cpp, .text 800B81F0..800B8EE0.
// One global manager (80598D00) holds a double-buffered model slot per player and kind. A cap slot
// also loads an original design as its texture. Clients (dHmnHeadItem_c) only hold a player index.
// Class name from the heap strings "dHmnHeadItemMng_c::createHeap::cap" / "::accessory"; every
// other name here is inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <nw4r/g3d/res/g3d_resmdl.h>

class dPrivateData_c;

class dHmnHeadItemMng_c {
public:
    enum {
        KIND_CAP = 0,
        KIND_ACCESSORY = 1,
        KIND_NUM = 2,

        SLOT_NUM = 4, // players
        BUFFER_NUM = 2,
    };

    // Load states of one buffer (mLoadState).
    enum {
        LOAD_NONE = 0,
        LOAD_BUSY = 1,
        LOAD_DONE = 2,
    };

    // Request states of a slot (mState).
    enum {
        STATE_IDLE = 0,
        STATE_RELEASE = 1,
        STATE_LOAD = 2,
    };

    // One player's model slot of one kind.
    class data_c {
    public:
        data_c();  // 800B81F0
        ~data_c(); // 800B82A4

        /* 0x000 */ dItem::resLoader_c mLoader[BUFFER_NUM];    // the model
        /* 0x0E0 */ dItem::resLoader_c mTexLoader[BUFFER_NUM]; // an original design (caps)
        /* 0x1C0 */ dItem::Item mReqItem;                      // being loaded
        /* 0x1C4 */ EGG::FrmHeap *mpHeap[BUFFER_NUM];
        /* 0x1CC */ u16 mItemId;                               // loaded and shown (no default ctor)
        /* 0x1CE */ u8 mLoadState[BUFFER_NUM];
        /* 0x1D0 */ u8 mState;
        /* 0x1D1 */ u8 mCur;                                   // buffer holding mItemId
        /* 0x1D2 */ u8 mLocked;                                // buffer in use by the model (2 = none)
    }; // size 0x1D4

    dHmnHeadItemMng_c();  // 800B8328
    ~dHmnHeadItemMng_c(); // 800B8370

    BOOL createHeap(EGG::Heap *parent); // 800B83D4
    void clearPlayerItems();            // 800B84A0
    void clearItem(int slot);           // 800B84CC: the cap only

    static BOOL isOrgCap(int *design, const dItem::Item *item); // 800B89CC: an original-design cap

    BOOL load(data_c *data, dPrivateData_c *priv);      // 800B8B08
    BOOL release(data_c *data);                         // 800B8C58
    int getLoadBuffer(data_c *data);                    // 800B8D28
    void setupModel(data_c *data, dPrivateData_c *priv); // 800B8D44: design texture, boy/girl nodes

    data_c *getData(int kind, int slot) { return &mData[kind][slot]; }
    void finish(data_c *data) {
        int buf = getLoadBuffer(data);
        if (data->mLocked == data->mCur) {
            data->mCur = buf;
        }
        data->mItemId = data->mReqItem.mId;
        data->mReqItem = dItem::ITEM_ID_NONE;
        data->mState = STATE_IDLE;
    }

    static BOOL create(EGG::Heap *parent); // 800B8E6C
    static void clearPlayers();            // 800B8E7C
    static void clear(int slot);           // 800B8E88

    /* 0x000 */ data_c mData[KIND_NUM][SLOT_NUM];
}; // size 0xEA0

// A client of one player's dHmnHeadItemMng_c slots (non-polymorphic).
class dHmnHeadItem_c {
public:
    dHmnHeadItem_c();  // 800B84E4
    ~dHmnHeadItem_c(); // 800B84E8

    void setSlot(u8 slot);                                             // 800B8528
    BOOL requestCap(const dItem::Item *item, dPrivateData_c *priv);    // 800B8530: TRUE when done
    BOOL requestAccessory(const dItem::Item *item);                    // 800B860C: TRUE when done
    BOOL request(int kind, const dItem::Item *item, dPrivateData_c *priv, BOOL isOrg) const; // 800B86B0
    void lockCap();                                                    // 800B8864
    void lockAccessory();                                              // 800B886C
    void lock(int kind);                                               // 800B8874
    void *getCapResFile();                                             // 800B88B8
    BOOL syncLoad();                                                   // 800B88C0
    void *getAccessoryResFile();                                       // 800B8A7C
    void *getResFile(int kind) const;                                     // 800B8A84: model brres, or NULL

    /* 0x0 */ u8 mSlot;
}; // size 0x1
