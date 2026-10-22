#pragma once

#include <lib/egg/core/eggBitFlag.h>
#include <lib/egg/core/eggDisposer.h>
#include <revolution/mem.h>
#include <revolution/os.h>

namespace EGG {

class Allocator;
class FrmHeap;
class ExpHeap;
class Thread;
class Heap;

struct HeapAllocArg {
    int userArg; // 0x00
    u32 size;    // 0x04
    int align;   // 0x08
    Heap *heap;  // 0x0C

    HeapAllocArg() : userArg(0), size(0), align(0), heap(NULL) {}
};
typedef void (*HeapAllocCallback)(HeapAllocArg &arg);

struct HeapErrorArg {
    const char *msg;
    void *userdata;

    HeapErrorArg() {}
};
typedef void (*ErrorCallback)(HeapErrorArg *arg);
typedef void (*HeapCreateCallback)(Heap *heap);
typedef void (*HeapDestroyCallback)(Heap *heap);

class Heap : public Disposer {
public:
    enum EHeapKind {
        HEAP_KIND_NONE,
        HEAP_KIND_EXPAND,
        HEAP_KIND_FRAME,
        HEAP_KIND_UNIT,
        HEAP_KIND_ASSERT
    };

    Heap(MEMiHeapHead *heapHead);
    virtual ~Heap();

    virtual EHeapKind getHeapKind() const = 0;
    virtual void initAllocator(Allocator *allocator, long align) = 0;
    virtual void *alloc(size_t size, long align) = 0;
    virtual void free(void *block) = 0;
    virtual void destroy() = 0;
    virtual int resizeForMBlock(void *block, size_t size) = 0;
    virtual u32 getTotalFreeSize() = 0;
    virtual size_t getAllocatableSize(long align) = 0;
    virtual size_t adjust() = 0;

    static void initialize();
    static Heap *findHeap(MEMiHeapHead *heapHandle);
    static void *alloc(size_t size, int align, Heap *heap);
    static void free(void *block, Heap *heap);
    static Heap *findContainHeap(const void *block);
    static void dumpAll();

    Heap *findParentHeap();
    void dispose();
    void dump();
    Heap *becomeCurrentHeap();
    Heap *becomeCurrentHeapWithoutLock();

    void *getStartAddress() {
        return this;
    }
    void *getEndAddress() {
        return mHeapHandle->heapEnd;
    }

    void disableAllocation() {
        mFlags.setBit(BIT_DISABLE_ALLOC);
    }
    void enableAllocation() {
        mFlags.resetBit(BIT_DISABLE_ALLOC);
    }

    void setName(const char *name) {
        mpName = name;
    }

    void appendDisposer(Disposer *disposer) { nw4r::ut::List_Append(&mDisposerList, disposer); }
    void removeDisposer(Disposer *disposer) { nw4r::ut::List_Remove(&mDisposerList, disposer); }

    static FrmHeap *dynamicCastToFrm(Heap *heap) {
        if (heap != nullptr && heap->getHeapKind() == HEAP_KIND_FRAME) {
            return (FrmHeap *) heap;
        }
        return nullptr;
    }

    static Heap *getCurrentHeap() {
        return sCurrentHeap;
    }

protected:
    MEMiHeapHead *mHeapHandle; // 0x10
    void *mParentBlock;        // 0x14
    Heap *mParentHeap;         // 0x18

    enum {
        BIT_DISABLE_ALLOC
    };
    TBitFlag<u16> mFlags; // 0x1C

    NW4R_UT_LIST_LINK_DECL();         // 0x20
    nw4r::ut::List mDisposerList;     // 0x28
    const char *mpName;               // 0x34

    static nw4r::ut::List sHeapList;
    static OSMutex sRootMutex;
    static Heap *sCurrentHeap;
    static int sIsHeapListInitialized;
    static Heap *sAllocatableHeap;
    static ErrorCallback sErrorCallback;
    static HeapAllocCallback sAllocCallback;
    static void *sErrorCallbackArg;
    static void *sAllocCallbackArg;
    static HeapCreateCallback sCreateCallback;
    static HeapDestroyCallback sDestroyCallback;
    static Thread *sAllocatableThread;
};

} // namespace EGG

void *operator new(size_t);
void *operator new(size_t, void *);
void *operator new(size_t, int);
void *operator new(size_t, EGG::Heap *, int);
void *operator new[](size_t);
void *operator new[](size_t, EGG::Heap *, int);
void operator delete(void *);
void operator delete[](void *);
