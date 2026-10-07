#pragma once

// Candidate sets: a bit per candidate index, filled by search() through a virtual check(), then
// picked from at random. Class names are from the RTTI; method names are inferred.
// The base classes (included by d_search_cand.hpp, d_bk_unit_search_cand.hpp and
// d_fd_bk_search_cand.hpp). Source: src/dol/game/d_search_cand.cpp (.text 8015F2B0..8015F748). See notes/d_search_cand.txt.
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

    int getCount() const { return mCount; }

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
    virtual BOOL check(int x, int z) { return TRUE; } // 8008EDE0 (weak)

    // A random candidate as (x, z); FALSE if none.
    BOOL getRandomXZ(int *x, int *z) {
        int idx = getRandom();
        if (idx < 0) {
            return FALSE;
        }
        *x = idx % mWidth;
        *z = idx / mWidth;
        return TRUE;
    }

    // The nth candidate as (x, z); FALSE if none.
    BOOL getNthXZ(int n, int *x, int *z) {
        int idx = getNth(n);
        if (idx < 0) {
            return FALSE;
        }
        *x = idx % mWidth;
        *z = idx / mWidth;
        return TRUE;
    }

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
