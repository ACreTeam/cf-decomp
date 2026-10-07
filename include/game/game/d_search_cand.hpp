#pragma once
// The candidate sets (see d_search_cand_base.hpp). The definition order of these classes sets the
// order of their weak vtables / RTTI in the users (d_search_cand.cpp: dBkUnitSearchCandCb_c first).
#include <game/game/d_search_cand_base.hpp>
#include <game/game/d_bk_unit_search_cand.hpp>
#include <game/game/d_fd_bk_search_cand.hpp>

// Every unit of the field, border acres included (fn_80190C44(1): 112 x 112). Header-only: the ctor is inlined into the
// users and the class has no vtable of its own (check(x, z) stays pure). Its weak RTTI is kept in
// the TU after d_event. Derived (town layout, .text ..8014BD80): nearBkCand_c, reserveCand_c
// (insidePlHsBkCand_c, ufoCand_c, lightHouseCand_c), plHsCand_c.
class dFdGutSearchCand_c : public dSearchCandXZCore_c {
public:
    dFdGutSearchCand_c(dFdBase_c *info = fn_80190C44(1))
        : dSearchCandXZCore_c(info->mUnitW, info->mUnitH, mBuf) {
        mInfo = fn_80190C44(1);
    }
    // Every unit of a dFdBase_c's grid; the derived class sets mInfo (raccoCand_c).
    explicit dFdGutSearchCand_c(const dFdBase_c *fd) : dSearchCandXZCore_c(fd->mUnitW, fd->mUnitW, mBuf) {}
    // The derived class sets mInfo (d_save_building's candidates).
    dFdGutSearchCand_c(int width, int height) : dSearchCandXZCore_c(width, height, mBuf) {}

    /* 0x01C */ u32 mBuf[(BLOCK_X_NUM * UT_X_NUM * BLOCK_Z_NUM * UT_Z_NUM + 31) / 32];
    /* 0x63C */ dFdBase_c *mInfo;
}; // size 0x640

