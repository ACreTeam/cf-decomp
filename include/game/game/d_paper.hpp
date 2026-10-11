#pragma once

// d_paper (DOL TU d_paper.cpp, .text 800FABF4..800FACC0): the stationery a villager writes on, by personality (looks)
// and season. No RTTI: names inferred.

#include <types.h>

namespace dPaper {

u16 getPaper(u8 looks, u32 season);  // 800FABF4: a random stationery item (30%: from the season's list); 0xFFF1 if invalid
void fn_800FACBC();                   // 800FACBC: empty (d_s_boot_static)

} // namespace dPaper
