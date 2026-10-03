#pragma once

#include <types.h>
#include <game/game/d_dsn.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>

// Player houses. Defined in src/dol/game/d_home.cpp (.text 8013CB30..8013E618).
// There is no RTTI for these; the names follow the GC "home" naming and are
// inferred, as are most member names.

class dPrivateData_c;

#define HOME_ROOM_NUM 3
#define HOME_LAYER_NUM 2
#define HOME_SIZE_NUM 5

// One 16x16 layer of a room (floor items, then items placed on top).
struct dHomeLayer_c {
    void clear(); // 8013CB30: every slot to ITEM_ID_NONE

    // The getter goes through a byte offset: indexing mItems directly makes MWCC
    // step a pointer through the loop in hasInvalidItem instead of recomputing it.
    const dItem::Item &get(int z, int x) const {
        return *(const dItem::Item *)((const u8 *)this + (x + z * 16) * sizeof(dItem::Item));
    }
    void set(int z, int x, const dItem::Item &item) { mItems[z][x] = item; }

    /* 0x000 */ dItem::Item mItems[16][16];
}; // size 0x200

// The classes below (dHomeActiveFtr_c, dHomeGyroids_c, dHomeRoomMap_c) are defined in the
// unsplit TU around 80110F30; only their inline ctors live here.

// Furniture active (switched on) bits for one layer: 16x16 bits, one u16 per row.
// Starts with every bit set; dHomeRoom_c::init clears the default furniture's tiles.
struct dHomeActiveFtr_c {
    dHomeActiveFtr_c() { clear(); }

    void clear(); // 80110F30
    BOOL isSet(int x, int z); // 80110F7C
    void set(int x, int z, BOOL on); // 80110F94

    /* 0x00 */ u16 mRows[16];
}; // size 0x20

// Gyroids (KIND_HANIWA) in the room: up to 8 positions, each with a 4-bit value
// (the low nibble of the furniture-state parameter; likely the tune/variant).
struct dHomeGyroids_c {
    dHomeGyroids_c() { clear(); }

    void clear(); // 80110FDC
    int get(int x, int z); // 80111058: value at (x, z)
    BOOL remove(int x, int z); // 801111D8
    BOOL add(int x, int z, int value); // 80111278 (inferred: looks the spot up with get first)

    struct Pos {
        u8 x : 4;
        u8 z : 4;
    };
    /* 0x0 */ Pos mPos[8];
    /* 0x8 */ u8 mActive; // bit per entry
    /* 0x9 */ u8 mValues[4]; // a nibble per entry
}; // size 0xD

// Per-room furniture state, written by the furniture-state handler fn_800A9718 when
// the room is not the current scene: active bits per layer plus the gyroids. The out-of-line copy of the
// ctor is 801119C4 (used with __construct_array for the globals at 805EC5D0).
struct dHomeRoomMap_c {
    dHomeRoomMap_c() { clear(); }

    void clear(); // 801113F8
    BOOL isSet(int x, int z, int layer); // 80111458
    void set(int x, int z, int layer, BOOL on); // 80111464

    /* 0x00 */ dHomeActiveFtr_c mActiveFtr[HOME_LAYER_NUM];
    /* 0x40 */ dHomeGyroids_c mGyroids;
}; // size 0x4E

// A room of a house. The save data also keeps two standalone rooms
// (dSaveData_c::_0636F0 / _0641F0).
struct dHomeRoom_c {
    void clear(); // 8013CBCC
    void recycleItems(); // 8013CC78: sends the room's items to the town's recycle bin, then clears it
    static void getDefaultItems(u16 *out, u32 style, u32 room); // 8013CF7C: {main room item, wallpaper, carpet}
    void init(int style, int room); // 8013D06C
    BOOL isValidLayer(int layer); // 8013D1A4
    dHomeLayer_c *getLayer(int layer); // 8013D1C0
    const dHomeLayer_c *getLayer(int layer) const; // 8013D210
    BOOL hasInvalidItem() const; // 8013D260: an item with no BITM entry
    dHomeLayer_c *getLayerItems(int layer); // 8013D384 (same code as getLayer)
    u8 countFlags(); // 8013D3D4
    void clearFlag(int flag); // 8013D3F8
    BOOL isFlag(int flag) const; // 8013D430
    void setFlags(u8 flags); // 8013D470

    /* 0x000 */ dHomeLayer_c mLayers[HOME_LAYER_NUM];
    /* 0x400 */ dHomeRoomMap_c mMap;
    /* 0x44E */ dItem::Item mWallpaper; // default 0x2BE (getter fn_80077730)
    /* 0x450 */ dItem::Item mCarpet; // default 0x319 (getter fn_80077748)
    /* 0x452 */ dItem::Item mSong; // song playing on the room's stereo; set by 800A9890 as Item(0xEE, song) for furniture func 0xC (setter fn_80112BAC)
    /* 0x454 */ u8 _454;
    /* 0x455 */ u8 _455;
    /* 0x456 */ u8 _456;
    /* 0x457 */ u8 mFlags; // 3 bits
}; // size 0x458

// A player's house.
struct dHome_c {
    void clear(); // 8013D478
    void init(u32 style); // 8013D4FC: style 0..3 picks the default design and rooms
    BOOL hasRoom(u32 room); // 8013D5B8: by house size
    void recycleRoomItems(int room); // 8013D5FC
    BOOL setOwner(dPrivateData_c *player); // 8013D628: only if it has no owner yet
    BOOL setOwnerToCurrent(); // 8013D744
    BOOL isOwner(const dPersonalID_c *pid); // 8013D78C
    static BOOL isValidRoom(int room); // 8013D838
    dHomeRoom_c *getRoom(int room); // 8013D850
    int getExteriorId(); // 8013D8A8: by house size
    int getRoomId(int room); // 8013D8D0
    void update(int days, BOOL isCurrent); // 8013D920
    BOOL requestUpgrade(); // 8013D9D4: mNextSize = mSize + 1
    BOOL hasRoom456(); // 8013DA04
    BOOL hasFlags(); // 8013DA78
    void clearFlags(); // 8013DAE8
    void updateFlags(int days, u32 isCurrent); // 8013DB7C
    void setFlagCount(int count); // 8013DD5C
    u8 getFlagMask(u8 count); // 8013DE5C

    bool isPending() const { return _15BB.mRaw & 1; }

    /* 0x0000 */ dDesign_c mDesign;
    /* 0x0880 */ dPersonalID_c mOwner;
    /* 0x08AC */ dHomeRoom_c mRooms[HOME_ROOM_NUM];
    /* 0x15B4 */ u8 mSize; // 0..4
    /* 0x15B5 */ u8 mNextSize; // applied by update()
    /* 0x15B6 */ u8 _15B6;
    /* 0x15B7 */ u8 _15B7;
    /* 0x15B8 */ dItem::Item _15B8; // registered in the owner's catalog
    /* 0x15BA */ u8 _15BA; // days counter (30 after init)
    /* 0x15BB */ union {
        u8 mRaw;
        struct {
            u8 _0 : 6;
            u8 mApplied : 1; // set by update() when mPending was set
            u8 mPending : 1;
        } mBits;
    } _15BB;
}; // size 0x15C0

// The four houses in the save data (dSaveData_c::mHomes).
struct dHomeList_c {
    void initAll(); // 8013DE90
    void initPlayerHome(u32 player); // 8013DEE0
    int findOwner(dPrivateData_c *player); // 8013DF24: house index or -1
    int findPlayer(u32 player); // 8013DFA8
    int findCurrentPlayer(); // 8013E004
    static int getHomeFromScene(int *room, int scene); // 8013E04C
    dHome_c *getHome(u32 home); // 8013E0A8
    const dHome_c *getHome(u32 home) const; // 8013E0C4
    int getPlayerOfHome(u32 home); // 8013E0E0
    dPrivateData_c *getHomePlayer(u32 home); // 8013E158
    dPrivateData_c *getHomePlayerRaw(u32 home); // 8013E18C
    int countVacant() const; // 8013E1C0
    void updateAll(int days); // 8013E238

    /* 0x0000 */ dHome_c mHomes[PLAYER_NUM];
}; // size 0x5700

// The song playing in a room, by scene.
dItem::Item getCurrentRoomSong(); // 8013E2DC
BOOL setRoomSong(int scene, dItem::Item item); // 8013E384
BOOL setCurrentRoomSong(dItem::Item item); // 8013E424
BOOL clearRoomSong(u8 scene); // 8013E458
BOOL clearCurrentRoomSong(); // 8013E488
BOOL isSongInOtherRoom(int scene, const dItem::Item &item); // 8013E4B8
BOOL isSongInOtherCurrentRoom(const dItem::Item &item); // 8013E598
int getRoomFromScene(int scene); // 8013E5D0
dItem::Item getRoomSong(const dHomeRoom_c *room); // 8013E60C
