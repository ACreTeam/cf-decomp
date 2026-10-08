#pragma once
#include <types.h>
#include <game/game/d_dvd.hpp>

// Background (field model) resources, namespace dBG. The TU (dol/game/d_bg.cpp, .text
// 800684A4..80069D2C) is not decompiled yet; only what other sources need is declared. The class
// names come from the RTTI; members are inferred.
namespace dBG {

// The grass textures: one brres with a 16 x 16 grass wear texture per block type.
class grassBank_c : public dDvd::brresBank_c {
public:
    grassBank_c() {}
    virtual ~grassBank_c() {}

    BOOL load(void *heap); // 8006934C: bank_c::load of the grass brres
    u8 *getTexImage(int type); // 8006935C: the texel data of the block type's texture, NULL if not loaded
};

} // namespace dBG
