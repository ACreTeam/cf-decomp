#pragma once

// Bodies of the RegEad::Array members (see RegEadArray.hpp).

#include <lib/RegEad/RegEadArray.hpp>

namespace RegEad {

template <typename T, int BLOCK>
Array<T, BLOCK>::Array() : mSize(0), mCapacity(0), mData(NULL), mOwns(true) {
}

template <typename T, int BLOCK>
Array<T, BLOCK>::Array(const Array &other)
    : mSize(0), mCapacity(0), mData(NULL), mOwns(true) {
    *this = other;
}

template <typename T, int BLOCK> Array<T, BLOCK>::~Array() {
    clear();
}

template <typename T, int BLOCK>
Array<T, BLOCK> &Array<T, BLOCK>::operator=(const Array &other) {
    clear();
    if (other.mCapacity != 0) {
        reserve(other.mCapacity);
        if (gAllocFailed) {
            return *this;
        }
        for (u32 i = 0; i < other.mSize; i++) {
            T *p = new (&mData[i]) T();
            *p = other.mData[i];
        }
        mSize = other.mSize;
    }
    return *this;
}

template <typename T, int BLOCK> void Array<T, BLOCK>::clear() {
    if (mOwns) {
        for (u32 i = 0; i < mSize; i++) {
            mData[i].~T();
        }
        if (mData != NULL) {
            freeMem(mData);
            mData = NULL;
        }
        mSize = 0;
        mCapacity = 0;
        mOwns = true;
    }
}

template <typename T, int BLOCK> void Array<T, BLOCK>::reserve(u32 n) {
    u32 cap = ((n - 1) / BLOCK + 1) * BLOCK;
    if (mData != NULL) {
        if (mCapacity < cap) {
            T *p = (T *)allocMem(cap * sizeof(T));
            if (p == NULL) {
                gAllocFailed = true;
            } else {
                memcpy(p, mData, mCapacity * sizeof(T));
                freeMem(mData);
                mData = p;
                mCapacity = cap;
            }
        }
    } else {
        mData = (T *)allocMem(cap * sizeof(T));
        if (mData == NULL) {
            gAllocFailed = true;
        } else {
            mCapacity = cap;
        }
    }
}

template <typename T, int BLOCK> void Array<T, BLOCK>::resize(u32 n) {
    reserve(n);
    if (!gAllocFailed) {
        u32 old = mSize;
        if (n < old) {
            for (u32 i = n + 1; i < mSize; i++) {
                mData[i].~T();
            }
        } else {
            for (u32 i = old; i < n; i++) {
                new (&mData[i]) T;
            }
        }
        mSize = n;
    }
}

template <typename T, int BLOCK> T *Array<T, BLOCK>::push_back(const T &v) {
    reserve(mSize + 1);
    if (gAllocFailed) {
        return NULL;
    }
    T *p = new (&mData[mSize]) T;
    *p = const_cast<T &>(v);
    mSize++;
    return mData + mSize - 1;
}

template <typename T, int BLOCK> T *Array<T, BLOCK>::insertSorted(const T &v) {
    reserve(mSize + 1);
    if (gAllocFailed) {
        return NULL;
    }
    u32 n = mSize;
    u32 pos = 0;
    for (u32 i = 0; i < n; i++, pos++) {
        if (v < mData[i]) {
            break;
        }
    }
    for (u32 i = n; i > pos; i--) {
        T *dst = &mData[i];
        memcpy(dst, dst - 1, sizeof(T));
    }
    mData[pos] = v;
    mSize++;
    return &mData[pos];
}

template <typename T, int BLOCK> void Array<T, BLOCK>::erase(u32 idx) {
    mData[idx].~T();
    for (u32 i = idx; i < mSize - 1; i++) {
        T *dst = &mData[i];
        memcpy(dst, dst + 1, sizeof(T));
    }
    mSize--;
}

template <typename T, int BLOCK>
void Array<T, BLOCK>::transferTo(Array &__restrict dst) {
    dst.mSize = mSize;
    dst.mCapacity = mCapacity;
    dst.mData = mData;
    mOwns = false;
    dst.mOwns = true;
}

} // namespace RegEad
