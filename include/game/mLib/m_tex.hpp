#pragma once
#include <types.h>

// Editable textures: a CPU-side view of GX texture data in its tiled layout (mTex). The class
// names come from the RTTI ("mTex::base_c", "mTex::edit8b_c"); members and functions are inferred.
// The implementation is in the unsplit .text around 802B5F10..802B6484 (before mFader_c), with
// 4-bit (8 x 8 tiles), 8-bit (8 x 4) and 16-bit (4 x 4) variants; only what d_footmark needs is
// declared here.
namespace mTex {

class base_c {
public:
    base_c() { init(4, 4, 4, 4); }

    // Sets the size and the tile size, and computes the tile counts.
    void init(u32 width, u32 height, u32 tileW, u32 tileH); // 802B5F10
    // The byte (texel) offset of (x, y) in the tiled data, or -1 when out of range.
    int getOffset(u32 x, u32 y) const; // 802B5FC0

protected:
    /* 0x00 */ u32 mWidth;
    /* 0x04 */ u32 mHeight;
    /* 0x08 */ u32 mTileW;
    /* 0x0C */ u32 mTileH;
    /* 0x10 */ u32 mTileSize;  // mTileW * mTileH
    /* 0x14 */ u32 mTilesX;    // mWidth / mTileW, rounded up
    /* 0x18 */ u32 mTilesY;    // mHeight / mTileH, rounded up
    /* 0x1C */ u32 mTileNum;   // mTilesX * mTilesY

public:
    /* 0x20 vtable */
    virtual ~base_c() {}
}; // size 0x24

// 8-bit texels (GX_TF_I8 / CI8 layout: 8 x 4 tiles).
class edit8b_c : public base_c {
public:
    edit8b_c(u32 width, u32 height, void *data) { init(width, height, data); }
    virtual ~edit8b_c() {}
    // Writes a texel, optionally flushing it from the data cache. FALSE when out of range.
    virtual BOOL set(u32 x, u32 y, u8 value, BOOL flush); // 802B62A8

    void init(u32 width, u32 height, void *data); // 802B6214
    u8 get(u32 x, u32 y); // 802B6258: the texel, 0 when out of range
    void flush(); // 802B632C: DCStoreRangeNoSync over the whole texture

protected:
    /* 0x24 */ u8 *mpData;
}; // size 0x28

} // namespace mTex
