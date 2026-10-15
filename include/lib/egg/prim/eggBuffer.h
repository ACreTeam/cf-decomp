#pragma once

namespace EGG {

class Heap;

// Virtual interface and layout shared with the Wii Sports EGG buffer.
template <typename T> class TBuffer {
public:
    TBuffer();
    virtual ~TBuffer();
    virtual void allocate(int size, int align = 0);
    virtual void allocate(int size, Heap *heap, int align = 0);
    virtual void onAllocate(Heap *heap);
    virtual void errRangeOver() const;

private:
    int mSize;  // 0x4
    T *mBuffer; // 0x8
};

template <typename T> void TBuffer<T>::errRangeOver() const {}

} // namespace EGG
