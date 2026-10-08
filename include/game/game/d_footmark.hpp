#pragma once
#include <types.h>
#include <game/mLib/m_tex.hpp>
#include <game/game/d_bg.hpp>

// Footmarks: the grass wear texture of the field (dol/game/d_footmark.cpp, .text
// 800A773C..800A858C). See notes/d_footmark.txt. The class name comes from the RTTI
// ("dFootmark::Editor_c") and the heap name ("dFootmark::Editor_c::m_frmHeap_p"); every other name
// is inferred.
//
// The texture is 128 x 128 texels (one per unit of a 7 x 7 block field), 8-bit: the grass wear of
// each unit, 0 for bare ground. Walking lowers it (dFdBase_c::wearGrass -> addPos), the daily
// update (updateDay) raises it again from the neighbouring units, and every value stays within
// the BG's limits (dBGCF::getGrassMin / getGrassMax). Each change is mirrored into the save
// (dSaveMainField_c::getBlockGrassWearUnits, 16 x 16 per block). The grass brres (dBG::grassBank_c)
// gives each block type's starting texture. d_bg points the grass model's texture at mPixels.

class mVec3_c;
namespace EGG {
class Heap;
class FrmHeap;
} // namespace EGG

namespace dFootmark {

enum {
    TEX_SIZE = 128, // texels per side (units of 7 blocks, rounded up)
};

class Editor_c : public dBG::grassBank_c, public mTex::edit8b_c {
public:
    Editor_c() : mTex::edit8b_c(0, 0, NULL), m_frmHeap_p(NULL) { init(); }
    virtual ~Editor_c() {}

    // The wear of a unit (0 = bare ground); 0 outside the texture.
    int getWear(int x, int z) { return get(x, z); }

    void clearMarks();  // 800A773C: both mark maps
    void clearMarks2(); // 800A7784
    void clearPixels(); // 800A7794: 0xFF everywhere
    void init();        // 800A77A4: town field, cleared texture and marks
    BOOL isMarked(int x, int z) const; // 800A77FC
    void setMarked(int x, int z);      // 800A7830
    // Copies a block's 16 x 16 wear values (the block's units start at blockX * 16, blockZ * 16).
    // The border blocks of the town are written as 0.
    void setBlock(const u8 *units, u32 blockX, u32 blockZ); // 800A7864
    u8 *getSaveUnits(int blockX, int blockZ) const;          // 800A7994: the block's units in the save
    virtual void onLoaded();                                 // 800A79E0: the grass brres is loaded
    BOOL create(EGG::Heap *heap, int fieldId);              // 800A7B18: makes the heap, loads the brres
    BOOL destroy();                                          // 800A7B9C
    void loadSave();                                         // 800A7BF8: the texture from the save
    void setUnit(int x, int z, int value, BOOL flush);       // 800A7C84: clamped, written to the save too
    u8 getMin(int x, int z) const;                           // 800A7DA0
    u8 getMax(int x, int z) const;                           // 800A7E10
    // Adds delta to a unit once until the marks are cleared (always with force).
    void addUnit(int x, int z, int delta, BOOL force, BOOL flush);              // 800A7E80
    void addPos(const mVec3_c *pos, int delta, BOOL force, BOOL flush);         // 800A7F14
    void updateDay(int days, int arg, BOOL flush);                              // 800A7F9C
    void clampAll();                                                            // 800A822C

private:
    BOOL isBorderBlock(u32 blockX, u32 blockZ) const;
    // The bg slot of the field: 1 for the town, else 0.
    void setCurrentBg() const;

    /* 0x0080 */ u8 mPixels[TEX_SIZE * TEX_SIZE] ALIGN(32);
    /* 0x4080 */ u8 mMarks[TEX_SIZE * TEX_SIZE / 8];  // a bit per unit: changed since clearMarks
    /* 0x4880 */ u8 mMarks2[TEX_SIZE * TEX_SIZE / 8]; // a bit per unit (only cleared here)
    /* 0x5080 */ int mFieldId;                          // fn_80190C44 index (FD_ID_*)
    /* 0x5084 */ EGG::FrmHeap *m_frmHeap_p;
}; // size 0x50A0

Editor_c *getEditor(); // 800A835C

} // namespace dFootmark
