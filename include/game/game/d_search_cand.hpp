#pragma once

// Candidate sets: a bit per candidate index, filled by search() through a virtual check(), then
// picked from at random. Class names are from the RTTI; method names are inferred.
// Source: src/dol/game/d_search_cand.cpp (.text 8015F2B0..8015F748). See notes/d_search_cand.txt.
//
// dSearchCandCore_c                 1D set over [0, mNum)
//   dSearchCand_c<N>                with its own N-bit buffer (dTheater::dSearchProgram*_c)
//   dSearchCandXZCore_c             2D set over a width x height grid, check(x, z)
//     dSearchCandXZ_c<W, H>         with its own W*H-bit buffer
//       dBkUnitSearchCandCb_c       16x16 units of one acre, callback filter
//     dFdBkSearchCand_c             the field's acres (up to 64)
//       dFdBkSearchCandCb_c         callback filter
//     dFdGutSearchCand_c            every unit of the field (users: town layout, 80146D80..)

#include <types.h>
#include <game/game/d_random.hpp>
#include <game/game/d_field_info.hpp>

// Field grid sizes returned by fn_80190C44 (class not recovered; only the fields used here).
struct dFdGridInfo_c {
    /* 0x00 */ u8 _00[8];
    /* 0x08 */ int mBlockW; // acres (BLOCK_X_NUM)
    /* 0x0C */ int mBlockH;
    /* 0x10 */ int mUnitW;  // units (BLOCK_X_NUM * UT_X_NUM)
    /* 0x14 */ int mUnitH;
};
extern "C" dFdGridInfo_c *fn_80190C44(int idx); // 80190C44

class dSearchCandCore_c {
public:
    dSearchCandCore_c(int num, u32 *bits);                   // 8015F2B0
    virtual BOOL check(int idx) { return TRUE; }             // 80091764 (weak)

    void clear();                                            // 8015F2E8: no candidates
    void setAll();                                           // 8015F32C: every index a candidate
    int getNth(int n);                                       // 8015F374: index of the nth candidate, or -1
    int getRandom();                                         // 8015F3F4: cM::rnd
    int getRandom(dRandom_c *rnd);                           // 8015F458
    void search();                                           // 8015F4BC: add every index check() accepts
    BOOL isSet(int idx);                                     // 8015F530
    void add(int idx);                                       // 8015F560
    void remove(int idx);                                    // 8015F5D0

    /* 0x04 */ int mNum;    // number of indices
    /* 0x08 */ int mWords;  // u32 words in mBits
    /* 0x0C */ u32 *mBits;
    /* 0x10 */ int mCount;  // candidates set
}; // size 0x14

template <int N>
class dSearchCand_c : public dSearchCandCore_c {
public:
    dSearchCand_c() : dSearchCandCore_c(N, mBuf) {}

    /* 0x14 */ u32 mBuf[(N + 31) / 32];
};

class dSearchCandXZCore_c : public dSearchCandCore_c {
public:
    dSearchCandXZCore_c(int width, int height, u32 *bits)
        : dSearchCandCore_c(width * height, bits), mWidth(width), mHeight(height) {}
    virtual BOOL check(int idx) { return check(idx % mWidth, idx / mWidth); } // 8008EDC0 (weak)
    virtual BOOL check(int x, int z) = 0;

    void removeBorder();                                     // 8015F674: the outer ring of cells

    /* 0x14 */ int mWidth;
    /* 0x18 */ int mHeight;
}; // size 0x1C

template <int W, int H>
class dSearchCandXZ_c : public dSearchCandXZCore_c {
public:
    dSearchCandXZ_c() : dSearchCandXZCore_c(W, H, mBuf) {}

    /* 0x1C */ u32 mBuf[(W * H + 31) / 32];
};

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

// Every unit of the field, border acres included (fn_80190C44(1): 112 x 112). Header-only: the ctor is inlined into the
// users and the class has no vtable of its own (check(x, z) stays pure). Its weak RTTI is kept in
// the TU after d_event. Derived (town layout, .text ..8014BD80): nearBkCand_c, reserveCand_c
// (insidePlHsBkCand_c, ufoCand_c, lightHouseCand_c), plHsCand_c.
class dFdGutSearchCand_c : public dSearchCandXZCore_c {
public:
    dFdGutSearchCand_c(dFdGridInfo_c *info = fn_80190C44(1))
        : dSearchCandXZCore_c(info->mUnitW, info->mUnitH, mBuf) {
        mInfo = fn_80190C44(1);
    }

    /* 0x01C */ u32 mBuf[(BLOCK_X_NUM * UT_X_NUM * BLOCK_Z_NUM * UT_Z_NUM + 31) / 32];
    /* 0x63C */ dFdGridInfo_c *mInfo;
}; // size 0x640
