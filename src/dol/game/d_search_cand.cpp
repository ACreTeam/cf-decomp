// Candidate sets (dSearchCandCore_c and the field / acre-unit callback searches).
// See include/game/game/d_search_cand.hpp and notes/d_search_cand.txt.
// .text 8015F2B0..8015F748, .data 804F0598..804F0628, .sdata 8074B290..8074B2A0,
// .sdata2 80750E18..80750E20.
#include <game/game/d_search_cand.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

// Random float in [0, max) from rnd (unsplit TU at 80107A20).
extern "C" float fn_80107EC8(cRandom_c *rnd, float max);

// 8015F2B0
dSearchCandCore_c::dSearchCandCore_c(int num, u32 *bits) {
    mNum = num;
    mWords = num / 32 + ((num & 31) != 0);
    mBits = bits;
}

// 8015F2E8
void dSearchCandCore_c::clear() {
    memset(mBits, 0, mWords * sizeof(u32));
    mCount = 0;
}

// 8015F32C
void dSearchCandCore_c::setAll() {
    memset(mBits, 0xFFFF, mWords * sizeof(u32));
    mCount = mNum;
}

// 8015F374
int dSearchCandCore_c::getNth(int n) {
    for (int i = 0; i < mNum; i++) {
        if (isSet(i)) {
            if (n == 0) {
                return i;
            }
            n--;
        }
    }
    return -1;
}

// 8015F3F4
int dSearchCandCore_c::getRandom() {
    return getNth(mCount * cM::rnd());
}

// 8015F458
int dSearchCandCore_c::getRandom(cRandom_c *rnd) {
    return getNth(fn_80107EC8(rnd, mCount));
}

// 8015F4BC
void dSearchCandCore_c::search() {
    for (int i = 0; i < mNum; i++) {
        if (check(i)) {
            add(i);
        }
    }
}

// 8015F530
BOOL dSearchCandCore_c::isSet(int idx) {
    return (mBits[idx >> 5] & (1 << (idx & 31))) != 0;
}

// 8015F560
void dSearchCandCore_c::add(int idx) {
    if (!isSet(idx)) {
        mBits[idx >> 5] |= 1 << (idx & 31);
        mCount++;
    }
}

// 8015F5D0
void dSearchCandCore_c::remove(int idx) {
    if (isSet(idx)) {
        mBits[idx >> 5] &= ~(1 << (idx & 31));
        mCount--;
    }
}

// 8015F640
BOOL dBkUnitSearchCandCb_c::check(int x, int z) {
    if (mFunc != NULL) {
        return mFunc(mBlockX, mBlockZ, x, z, mUser);
    }
    return FALSE;
}

// 8015F674
void dSearchCandXZCore_c::removeBorder() {
    for (int x = 0; x < mWidth; x++) {
        for (int z = 0; z < mHeight; z++) {
            if (x == 0 || z == 0 || x == mWidth - 1 || z == mHeight - 1) {
                remove(x + z * mWidth);
            }
        }
    }
}

// 8015F71C
BOOL dFdBkSearchCandCb_c::check(int x, int z) {
    if (mFunc != NULL) {
        return mFunc(x, z, mUser);
    }
    return FALSE;
}
