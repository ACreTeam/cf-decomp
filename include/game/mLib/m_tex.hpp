#pragma once
#include <types.h>

// Editable textures: a CPU-side view of GX texture data in its tiled layout (mTex). The class
// names come from the RTTI ("mTex::base_c", "mTex::edit4b_c", "mTex::edit8b_c", "mTex::edit16b_c");
// members and functions are inferred. Implementation: src/dol/mLib/m_tex.cpp (802B5F10..802B6484),
// with 4-bit (8 x 8 tiles), 8-bit (8 x 4) and 16-bit (4 x 4) variants.
namespace mTex {

class base_c {
public:
    base_c() { init(4, 4, 4, 4); }

    // Sets the size and the tile size, and computes the tile counts.
    void init(u32 width, u32 height, u32 tileW, u32 tileH); // 802B5F10
    // The index of the tile containing (x, y).
    u32 getTileIdx(u32 x, u32 y) const; // 802B5F74
    // The index of (x, y) inside its tile.
    u32 getInTileIdx(u32 x, u32 y) const; // 802B5F94
    // The byte (texel) offset of (x, y) in the tiled data, or -1 when out of range.
    int getOffset(u32 x, u32 y) const; // 802B5FC0

    bool isInside(u32 x, u32 y) const { return x < mWidth && y < mHeight; }
    // The size of the tiled data in texels (the size rounded up to whole tiles).
    u32 getTexelNum() const { return (mTileH * mTilesY) * (mTilesX * mTileW); }

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

// 4-bit texels (GX_TF_I4 / CI4 layout: 8 x 8 tiles, two texels per byte, high nibble first).
class edit4b_c : public base_c {
public:
    edit4b_c(u32 width, u32 height, void *data) { init(width, height, data); }
    virtual ~edit4b_c() {}
    // Writes a texel, optionally flushing it from the data cache. FALSE when out of range.
    virtual BOOL set(u32 x, u32 y, u8 value, BOOL flush); // 802B6118

    void init(u32 width, u32 height, void *data); // 802B6068
    u8 get(u32 x, u32 y) const; // 802B60AC: the texel, 0 when out of range
    void flush(); // 802B61D8: DCStoreRangeNoSync over the whole texture

protected:
    /* 0x24 */ u8 *mpData;
}; // size 0x28

// 8-bit texels (GX_TF_I8 / CI8 layout: 8 x 4 tiles).
class edit8b_c : public base_c {
public:
    edit8b_c(u32 width, u32 height, void *data) { init(width, height, data); }
    virtual ~edit8b_c() {}
    // Writes a texel, optionally flushing it from the data cache. FALSE when out of range.
    virtual BOOL set(u32 x, u32 y, u8 value, BOOL flush); // 802B62A8

    void init(u32 width, u32 height, void *data); // 802B6214
    u8 get(u32 x, u32 y) const; // 802B6258: the texel, 0 when out of range
    void flush(); // 802B632C: DCStoreRangeNoSync over the whole texture

protected:
    /* 0x24 */ u8 *mpData;
}; // size 0x28

// 16-bit texels (GX_TF_RGB565 / RGB5A3 / IA8 layout: 4 x 4 tiles).
class edit16b_c : public base_c {
public:
    edit16b_c(u32 width, u32 height, void *data) { init(width, height, data); }
    virtual ~edit16b_c() {}
    // Writes a texel, optionally flushing it from the data cache. FALSE when out of range.
    virtual BOOL set(u32 x, u32 y, u16 value, BOOL flush); // 802B63FC

    void init(u32 width, u32 height, void *data); // 802B6364
    u16 get(u32 x, u32 y) const; // 802B63A8: the texel, 0 when out of range

protected:
    /* 0x24 */ u16 *mpData;
}; // size 0x28

} // namespace mTex
