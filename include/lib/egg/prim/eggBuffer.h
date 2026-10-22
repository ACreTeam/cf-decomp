#pragma once

#include <types.h>
#include <lib/egg/core/eggHeap.h>

namespace EGG {

// Virtual interface and layout shared with the Wii Sports EGG buffer.
template <typename T> class TBuffer {
  public:
    TBuffer() : mSize(0), mBuffer(NULL) {
    }

    virtual ~TBuffer() {
        if (mBuffer != NULL) {
            delete[] mBuffer;
            mBuffer = NULL;
        }
    }

    virtual void allocate(int size, int align = 0) {
        mSize = size;
        if (align != 0) {
            mBuffer = new T[mSize];
        } else {
            mBuffer = new T[mSize];
        }
        flush();
        onAllocate(NULL);
    }

    virtual void allocate(int size, Heap *heap, int align = 0) {
        mSize = size;
        if (heap == NULL) {
            heap = Heap::getCurrentHeap();
        }
        if (align != 0) {
            mBuffer = new (heap, 4) T[mSize];
        } else {
            mBuffer = new (heap, 4) T[mSize];
        }
        flush();
        onAllocate(heap);
    }

    virtual void onAllocate(Heap *) {
    }
    virtual void errRangeOver() const;

    T &operator()(int i) {
        checkRange(i);
        return mBuffer[i];
    }

    int getSize() const {
        return mSize;
    }

  private:
    void flush() {
    }

    void checkRange(int i) const {
        if (!isRangeValid(i)) {
            errRangeOver();
        }
    }

    bool isRangeValid(int i) const {
        return i >= 0 && i < mSize;
    }

    int mSize;  // 0x4
    T *mBuffer; // 0x8
};

template <typename T> void TBuffer<T>::errRangeOver() const {
}

} // namespace EGG
