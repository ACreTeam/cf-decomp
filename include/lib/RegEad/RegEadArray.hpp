#pragma once

// RegEad: EAD's regular-expression library, used by the NG-word (profanity)
// filter (dPrcMng_c). Patterns are Java-regex-like (\p{IsLu}, \p{InBasicLatin},
// POSIX names, \Q..\E, {n,m}); RegNfa compiles one into a Thompson NFA, RegDfa
// turns that into a DFA (subset construction), serializes it, and matches whole
// UTF-16 strings against it. Only RegDfa, RegNfa, RegNfaState and
// RegNfaStateCopier have known names (RTTI); every other name in these headers
// is invented.
//
// This header: the memory helpers and the growable array every container in the
// library uses.

#include <types.h>
#include <revolution/MEM.h>
#include <string.h>

// Placement new, inlined (the library has its own; no call to __nw__FUlPv).
inline void *operator new(size_t, void *ptr) {
    return ptr;
}

namespace RegEad {

// Heap used for all of the library's allocations (set by prcSetup).
// src: lib/RegEad/RegEadHeap.cpp (.text 801E90B8..801E90DC).
extern MEMiHeapHead *sHeap; // 8074EC70
// Set when an allocation fails; checked after nearly every container operation,
// cleared by RegNfa::compile and RegDfa::build. Its .sbss sits between
// RegEadHeap's and RegEadPrc's, padded apart from sHeap, so it is defined in a
// later TU; the split gives it to RegNfa.cpp (could also be RegLabel.cpp or
// RegNfaExec.cpp).
extern u8 gAllocFailed; // 8074EC78

void setHeap(MEMiHeapHead *heap); // 801E90B8
void *allocMem(u32 size);         // 801E90C0
void freeMem(void *ptr);          // 801E90D0

// Growable array. Capacity grows in blocks of BLOCK elements (16 for most
// element types, 4 for edges and DFA transitions). mOwns is cleared on the
// source of transferTo, so its destructor leaves the buffer to the new owner.
// All members are template instantiations (emitted weak after their first
// user). The bodies live in RegEadArray.inl, which only the TUs that
// instantiate them include; everywhere else the members are plain external
// calls.
template <typename T, int BLOCK = 16> class Array {
  public:
    // Plain pointers (indistinguishable from a wrapper class in the binary);
    // RegNfa::mpToken is one.
    typedef T *iterator;
    typedef const T *const_iterator;

    Array();
    Array(const Array &other);
    ~Array();

    Array &operator=(const Array &other); // deep copy
    bool operator==(const Array &other) const;

    u32 size() const {
        return mSize;
    }

    T &get(u32 idx) {
        return mData[idx];
    }

    const T &get(u32 idx) const {
        return mData[idx];
    }

    T &operator[](u32 i) {
        u32 idx = i;
        return get(idx);
    }
    const T &operator[](u32 i) const {
        u32 idx = i;
        return get(idx);
    }

    iterator begin() {
        return mData;
    }
    const_iterator begin() const {
        return mData;
    }
    iterator end() {
        return mData + mSize;
    }
    const_iterator end() const {
        return mData + mSize;
    }

    // Move: dst takes the buffer, this stops owning it.
    void transferTo(Array &__restrict dst);
    void clear(); // destroys the elements, frees the buffer (if owned)
    // Rounds n up to a multiple of BLOCK; sets gAllocFailed on failure.
    void reserve(u32 n);
    void resize(u32 n);       // reserve(n), size = n
    T *push_back(const T &v); // NULL on allocation failure
    T *insertSorted(const T &v);
    void erase(u32 i);
    bool contains(T v) const;

    /* 0x0 */ u32 mSize;
    /* 0x4 */ u32 mCapacity;
    /* 0x8 */ T *mData;
    /* 0xC */ bool mOwns;
};

} // namespace RegEad
