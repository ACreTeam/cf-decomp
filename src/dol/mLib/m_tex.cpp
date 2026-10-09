#include <game/mLib/m_tex.hpp>
#include <revolution/OS/OSCache.h>

namespace mTex {

void base_c::init(u32 width, u32 height, u32 tileW, u32 tileH) {
    mWidth = width;
    mHeight = height;
    mTileW = tileW;
    mTileH = tileH;
    mTileSize = tileW * tileH;
    if (tileW != 0) {
        mTilesX = (width + tileW - 1) / tileW;
    }
    if (mTileH != 0) {
        mTilesY = (mHeight + mTileH - 1) / mTileH;
    }
    mTileNum = mTilesX * mTilesY;
}

u32 base_c::getTileIdx(u32 x, u32 y) const {
    return x / mTileW + mTilesX * (y / mTileH);
}

u32 base_c::getInTileIdx(u32 x, u32 y) const {
    return x % mTileW + mTileW * (y % mTileH);
}

int base_c::getOffset(u32 x, u32 y) const {
    if (isInside(x, y)) {
        u32 tile = getTileIdx(x, y);
        u32 inTile = getInTileIdx(x, y);
        return inTile + tile * mTileSize;
    }
    return -1;
}

void edit4b_c::init(u32 width, u32 height, void *data) {
    base_c::init(width, height, 8, 8);
    mpData = static_cast<u8 *>(data);
}

u8 edit4b_c::get(u32 x, u32 y) const {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            return (mpData[offset / 2] >> (((offset + 1) & 1) * 4)) & 0xF;
        }
    }
    return 0;
}

BOOL edit4b_c::set(u32 x, u32 y, u8 value, BOOL flush) {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            mpData[offset / 2] &= ~(0xF << (((offset + 1) & 1) * 4));
            mpData[offset / 2] |= (value & 0xF) << (((offset + 1) & 1) * 4);
            if (flush) {
                DCStoreRangeNoSync(mpData + offset, 1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void edit4b_c::flush() {
    if (mpData != nullptr) {
        DCStoreRangeNoSync(mpData, getTexelNum() / 2);
    }
}

void edit8b_c::init(u32 width, u32 height, void *data) {
    base_c::init(width, height, 8, 4);
    mpData = static_cast<u8 *>(data);
}

u8 edit8b_c::get(u32 x, u32 y) const {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            return mpData[offset];
        }
    }
    return 0;
}

BOOL edit8b_c::set(u32 x, u32 y, u8 value, BOOL flush) {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            mpData[offset] = value;
            if (flush) {
                DCStoreRangeNoSync(mpData + offset, 1);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void edit8b_c::flush() {
    if (mpData != nullptr) {
        DCStoreRangeNoSync(mpData, getTexelNum());
    }
}

void edit16b_c::init(u32 width, u32 height, void *data) {
    base_c::init(width, height, 4, 4);
    mpData = static_cast<u16 *>(data);
}

u16 edit16b_c::get(u32 x, u32 y) const {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            return mpData[offset];
        }
    }
    return 0;
}

BOOL edit16b_c::set(u32 x, u32 y, u16 value, BOOL flush) {
    if (mpData != nullptr) {
        int offset = getOffset(x, y);
        if (offset != -1) {
            mpData[offset] = value;
            if (flush) {
                DCStoreRangeNoSync(mpData + offset, sizeof(u16));
            }
            return TRUE;
        }
    }
    return FALSE;
}

} // namespace mTex
