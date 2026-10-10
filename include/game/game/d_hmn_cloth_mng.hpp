#pragma once

// Human clothing (shirt) models. DOL TU d_hmn_cloth_mng.cpp, .text 800B6CF0..800B76BC.
// One global manager (805982F4) holds a double-buffered model slot per wearer: slots 0..3 are the
// players, 4..8 the other humans. Clients (dHmnCloth_c) only hold a slot index.
// Class name from the heap string "dHmnClothMng_c::createHeap::data"; every other name here is
// inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <lib/egg/core/eggFrmHeap.h>

class dDesign_c;
class dPrivateData_c;

class dHmnClothMng_c {
public:
    enum {
        SLOT_NUM = 9,       // 4 players + 5 others
        PLAYER_SLOT_NUM = 4,
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

    // One wearer's model slot ("::data" in the heap name).
    class data_c {
    public:
        data_c();  // 800B6CF0
        ~data_c(); // 800B6D8C

        /* 0x00 */ dItem::resLoader_c mLoader[BUFFER_NUM];
        /* 0xE0 */ dItem::Item mReqItem;               // being loaded
        /* 0xE4 */ EGG::FrmHeap *mpHeap[BUFFER_NUM];
        /* 0xEC */ u16 mItemId;                        // loaded and shown (no default ctor)
        /* 0xEE */ u8 mLoadState[BUFFER_NUM];
        /* 0xF0 */ u8 mState;
        /* 0xF1 */ u8 mCur;                            // buffer holding mItem
        /* 0xF2 */ u8 mLocked;                         // buffer in use by the model (2 = none)
    }; // size 0xF4

    dHmnClothMng_c();  // 800B6DF0
    ~dHmnClothMng_c(); // 800B6E38

    BOOL createHeap(EGG::Heap *parent); // 800B6E9C
    void clearPlayerItems();            // 800B6F2C
    void clearItem(int slot);           // 800B6F48

    static BOOL isCloth(const dItem::Item *item);                // 800B72A4: kind is KIND_CLOTH
    static BOOL isOrgCloth(int *design, const dItem::Item *item); // 800B7310: an original-design shirt

    BOOL load(data_c *data, dPrivateData_c *priv, dDesign_c *design); // 800B73C8
    BOOL release(data_c *data);                                     // 800B750C
    int getLoadBuffer(data_c *data);                                // 800B75C0
    void finish(data_c *data);                                      // 800B75DC

    data_c *getData(int slot) { return &mData[slot]; }

    static BOOL create(EGG::Heap *parent); // 800B7648
    static void clearPlayers();            // 800B7658
    static void clear(int slot);           // 800B7664

    /* 0x000 */ data_c mData[SLOT_NUM];
}; // size 0x894

// A client of one dHmnClothMng_c slot (held by humans; non-polymorphic).
class dHmnCloth_c {
public:
    dHmnCloth_c();  // 800B6F60
    ~dHmnCloth_c(); // 800B6F6C

    void setSlot(int slot);                                                    // 800B6FAC
    BOOL request(const dItem::Item *item, dPrivateData_c *priv, dDesign_c *design); // 800B6FB4: TRUE when done
    void lock();                                                               // 800B71C4
    BOOL syncLoad();                                                           // 800B71EC
    void *getResFile();                                                        // 800B7248: model brres, or NULL

    /* 0x0 */ u8 mSlot; // dHmnClothMng_c::SLOT_NUM = none
}; // size 0x1
