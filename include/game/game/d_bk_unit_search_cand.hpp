#pragma once
#include <game/game/d_search_cand_base.hpp>

// The units of acre (mBlockX, mBlockZ), filtered by a callback.
class dBkUnitSearchCandCb_c : public dSearchCandXZ_c<UT_X_NUM, UT_Z_NUM> {
public:
    typedef BOOL (*Func)(int blockX, int blockZ, int x, int z, void *user);

    dBkUnitSearchCandCb_c(int blockX, int blockZ, Func func, void *user)
        : mBlockX(blockX), mBlockZ(blockZ), mUser(user), mFunc(func) {}
    virtual BOOL check(int x, int z);                        // 8015F640

    /* 0x3C */ int mBlockX;
    /* 0x40 */ int mBlockZ;
    /* 0x44 */ void *mUser;
    /* 0x48 */ Func mFunc;
}; // size 0x4C
