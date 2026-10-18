#pragma once
#include <game/game/d_fd_bk_search_cand.hpp>

// The field's acres with a block flag (dFdBlock_c::hasFlag(mMask)). Defined
// (check) in both d_fish_fieldNP (0x1074) and d_insect_fieldNP (0xACC); neither
// constructs it. Include it where the class was defined: its vtable's position
// in .data follows the definition order.
class dBkAttrSearchCand_c : public dFdBkSearchCand_c {
  public:
    virtual BOOL check(int x, int z);

    /* 0x24 */ int mMask;
}; // size 0x28
