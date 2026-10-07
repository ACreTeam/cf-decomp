#pragma once

// dRF: the random town field generator. It builds the town's 7 x 7 acre layout for a new town
// (dSaveMainField_c::createField's block step) from a random candidate in BgData/Random/rndCand.bin.
// Source: src/dol/game/d_random_field.cpp (.text 80104C64..8010655C).
// Names: dRF::loader_c and dRF::l_frmHeap_p are the game's own (RTTI / heap name strings); the rest
// are inferred from the code.
//
// The layout works in block kinds (d_block_kind.hpp; BLOCK_KIND_NONE = none) and then picks a block
// type of each kind. Steps (generate, 80104FC8), repeated until every check passes:
//   setCandidate     an interior from rndCand.bin (0x496 candidates of 5 x 5 kind bytes)
//   setBorder        the border acres (BLOCK_KIND_BORDER_*, the SEA row); the north acre above
//                    BLOCK_KIND_RIVER_FALL gets the river (BORDER_N_RIVER), above GATE BORDER_N_GATE
//   addBridges       walks the river (sDirections) and makes a random acre of it a bridge
//                    (BLOCK_KIND_FLAG_BRIDGE), then one more for each branch
//   addRamps         a ramp (BLOCK_KIND_FLAG_RAMP) on one cliff acre on each side of the river
//   addRacco         BLOCK_KIND_FLAG_RACCO on one plain river acre (fn_80105340)
//   placeFacilities  TAILOR, SHOP, TOWN_HALL, MUSEUM on random PLAIN acres
//   setLevels        each acre's height level (cell_c::mFlag flips at each cliff), and the border
//                    acres next to a cliff (BORDER_W_CLIFF / BORDER_E_CLIFF)
//   setTypes         a block type for each kind, each type used at most once
//   hasPond          at least one sPondTypes acre inside the town

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_block_kind.hpp>

class dSaveMainField_c;
namespace EGG {
class FrmHeap;
}

namespace dRF {

#define RF_CELL_NUM (BLOCK_X_NUM * BLOCK_Z_NUM)
#define RF_CAND_NUM 0x496         // candidates in rndCand.bin
#define RF_CAND_SIZE 0x19         // 5 x 5 interior kinds per candidate
#define RF_TYPE_NUM 0xB8          // block types
#define RF_POND_TYPE_NUM 9        // sPondTypes (80475B58): PLAIN 0x33..0x35, GATE 0x38,
                                  // MUSEUM 0x3A / 0x3B, SHOP 0x3E, TAILOR 0x43 / 0x44

// One acre of the layout.
struct cell_c {
    cell_c() : mKind(BLOCK_KIND_NONE), mType(0), mFlag(0) {}

    void setKind(int kind) { mKind = kind; }

    int getCount() const;                            // 80104CC0: returns 0

    /* 0x0 */ int mKind;  // BLOCK_KIND_*
    /* 0x4 */ int mType;  // block type of that kind (setTypes)
    /* 0x8 */ u8 mFlag;   // height level; becomes dFdBlockId_c::mFlag
}; // size 0xC

// The 7 x 7 layout (a local of create(), so its cells are constructed there). getCell returns a
// function-local static dummy cell (guard 8074E62C, 805EBD70) for cells outside the grid.
class field_c {
public:
    static u32 getIndex(int x, int z);               // 80104D48: x + z * 7
    cell_c *getCell(int x, int z);                   // 801052C0
    static BOOL addKindFlags(cell_c *cell, u32 flags); // 80104C64: the kind with these BLOCK_KIND_FLAG_* added, if any

    BOOL generate(int *count);                       // 80104FC8: retries until valid; count = getCount()
    void write(dFdBlockId_c *blocks);                // 801050E8: into the field's block ids
    int getCount();                                  // 80104CC8: always 0 (sums cell_c::getCount)
    void setLevels();                                // 80104D58
    bool setTypes();                                 // 8010517C
    bool hasPond();                                  // 80105228: a sPondTypes acre inside the town
    bool placeKind(int kind);                        // 801053B4: replaces a random interior PLAIN acre
    bool addBridges();                               // 80105510
    bool addRamps();                                 // 80105940
    bool placeFacilities();                          // 80105AB8
    bool addRacco();                                 // 80105B3C
    bool setBorder();                                // 80105C3C
    bool setCandidate(int idx);                      // 80105F04: idx < 0 = random

    /* 0x000 */ cell_c mCells[RF_CELL_NUM];
}; // size 0x24C

BOOL canAddRacco(int kind);                          // 80105340: a plain river acre (no bridge, fall, cliff, beach)
BOOL getDirection(int *dx, int *dz, u32 kindFlags);  // 801054A4: sDirections lookup

// Block types already used (a bit per type).
BOOL isTypeUsed(const u8 *used, int type);           // 80105FD0
void setTypeUsed(u8 *used, int type);                // 80105FEC
int countTypes(const u8 *used, int kind, BOOL allowUsed); // 80106014
int getNthType(const u8 *used, int kind, BOOL allowUsed, int n); // 801060C8
int pickType(u8 *used, int kind);                    // 80106190: a random unused type, else any

BOOL create(dSaveMainField_c *field, int *count);   // 80106250: load rndCand.bin, generate, write
BOOL finishCreate();                                 // 8010640C: release the data and the heap
BOOL isPondType(int type);                           // 80106464: in sPondTypes

// The river's directions by kind flags (sDirections, 80475B7C).
struct direction_c {
    /* 0x00 */ int mFlags;  // BLOCK_KIND_FLAG_RIVER_*
    /* 0x04 */ int mDx;
    /* 0x08 */ int mDz;
    /* 0x0C */ u8 _0C[8];
}; // size 0x14

// rndCand.bin (l_loader, 805EBD18; built by the static initializer).
class loader_c : public dDvd::bank_c {
public:
    virtual ~loader_c() {}                           // 80106500 (weak)

    // Inline: its path string is emitted after the vtable and RTTI, as in the target's .data.
    BOOL load(EGG::FrmHeap *heap) { return bank_c::load("BgData/Random/rndCand.bin", heap, 0); }
}; // size 0x58

extern EGG::FrmHeap *l_frmHeap_p;                    // 8074E630 (defined after getCell's guard)

extern loader_c l_loader;                            // 805EBD18: rndCand.bin

} // namespace dRF
