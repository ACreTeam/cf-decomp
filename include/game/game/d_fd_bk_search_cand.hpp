#pragma once
#include <game/game/d_search_cand_base.hpp>

// The field's acres. The users build it from the field size (fn_80190C44(0)->mBlockW / mBlockH).
class dFdBkSearchCand_c : public dSearchCandXZCore_c {
public:
    dFdBkSearchCand_c(int width, int height) : dSearchCandXZCore_c(width, height, mBuf) {}

    /* 0x1C */ u32 mBuf[(BLOCK_TOTAL_NUM + 31) / 32];
}; // size 0x24

class dFdBkSearchCandCb_c : public dFdBkSearchCand_c {
public:
    typedef BOOL (*Func)(int x, int z, void *user);

    dFdBkSearchCandCb_c(int width, int height, Func func, void *user)
        : dFdBkSearchCand_c(width, height), mUser(user), mFunc(func) {}
    virtual BOOL check(int x, int z);                        // 8015F71C

    /* 0x24 */ void *mUser;
    /* 0x28 */ Func mFunc;
}; // size 0x2C
