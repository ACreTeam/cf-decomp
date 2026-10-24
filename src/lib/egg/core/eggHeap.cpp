#include <lib/egg/core/eggHeap.h>

#include <lib/egg/core/eggDisposer.h>
#include <lib/egg/core/eggThread.h>
#include <stddef.h>

namespace EGG {

nw4r::ut::List Heap::sHeapList;
OSMutex Heap::sRootMutex;
Heap *Heap::sCurrentHeap;
int Heap::sIsHeapListInitialized;
Heap *Heap::sAllocatableHeap;
ErrorCallback Heap::sErrorCallback;
HeapAllocCallback Heap::sAllocCallback;
void *Heap::sErrorCallbackArg;
void *Heap::sAllocCallbackArg;
HeapCreateCallback Heap::sCreateCallback;
HeapDestroyCallback Heap::sDestroyCallback;
Thread *Heap::sAllocatableThread;

#define SIZE_MB ((float)0x100000)

void Heap::initialize() {
    nw4r::ut::List_Init(&sHeapList, offsetof(Heap, link));
    OSInitMutex(&sRootMutex);
    sIsHeapListInitialized = true;
}

Heap::Heap(MEMiHeapHead *heapHandle) : Disposer(), mHeapHandle(heapHandle) {
    mParentBlock = NULL;
    mParentHeap = NULL;
    mpName = "NoName";
    mFlags = 0;

    nw4r::ut::List_Init(&mDisposerList, offsetof(Disposer, mLink));

    OSLockMutex(&sRootMutex);
    nw4r::ut::List_Append(&sHeapList, this);
    OSUnlockMutex(&sRootMutex);
}

Heap::~Heap() {
    OSLockMutex(&sRootMutex);
    nw4r::ut::List_Remove(&sHeapList, this);
    OSUnlockMutex(&sRootMutex);
}

void *Heap::alloc(size_t size, int align, Heap *heap) {
    Heap *currentHeap = sCurrentHeap;
    Thread *currentThread = Thread::findThread(OSGetCurrentThread());

    if (sAllocatableThread != NULL) {
        OSGetCurrentThread();
    }

    if (currentThread != NULL && currentThread->getAllocatableHeap() != NULL) {
        heap = currentHeap = currentThread->getAllocatableHeap();
    }

    if (sAllocatableHeap != NULL) {
        if (currentHeap != NULL && heap == NULL) {
            heap = currentHeap;
        }

        if (heap != sAllocatableHeap) {
            OSReport("cannot allocate from heap %x(%s) : allocatable heap is %x(%s)\n", heap,
                     heap->getName(), sAllocatableHeap, sAllocatableHeap->getName());

            const Heap *primary_heap = currentThread ? currentThread->getAllocatableHeap() : NULL;

            OSReport("\tthread heap=%x\n", primary_heap);

            const char *primary_heap_name =
                currentThread ? (currentThread->getAllocatableHeap() ?
                                     currentThread->getAllocatableHeap()->mpName :
                                     "none") :
                                "none";
            OSReport("\tthread heap=%s\n", primary_heap_name);
            if (sErrorCallback) {
                HeapErrorArg cb;
                cb.msg = "disable_but";
                cb.userdata = sErrorCallbackArg;
                sErrorCallback(&cb);
            }
            dumpAll();
            return NULL;
        }
    }

    if (sAllocCallback) {
        HeapAllocArg arg;
        arg.heap = heap ? heap : currentHeap;
        arg.size = size;
        arg.align = align;
        arg.userArg = (int)sAllocCallbackArg;
        sAllocCallback(arg);
    }

    if (heap != NULL) {
        return heap->alloc(size, align);
    }

    if (currentHeap != NULL) {
        int arena_end;
        void *allocated = currentHeap->alloc(size, align);

        if (allocated == NULL) {
            arena_end = (int)currentHeap->mHeapHandle->heapEnd;
            int max_avail = currentHeap->getAllocatableSize(4);
            int heap_size = arena_end - (u32)currentHeap;
            OSReport("heap (%p):(%.1fMBytes free %d)->alloc(size(%d:%.1fMBytes),%d "
                     "align)\n",
                     currentHeap, static_cast<f32>(heap_size) / SIZE_MB, max_avail, size,
                     static_cast<f32>(size) / SIZE_MB, align);

            dumpAll();
        }

        return allocated;
    }

    OSReport("cannot allocate %d from heap %x\n", size, heap);
    dumpAll();

    return NULL;
}

Heap *Heap::findHeap(MEMiHeapHead *heapHandle) {
    Heap *found = NULL;
    OSLockMutex(&sRootMutex);
    if (sIsHeapListInitialized) {
        Heap *it = NULL;
        while ((it = (Heap *)nw4r::ut::List_GetNext(&sHeapList, it)) != NULL) {
            if (it->mHeapHandle == heapHandle) {
                found = it;
                break;
            }
        }
    }
    OSUnlockMutex(&sRootMutex);
    return found;
}

Heap *Heap::findParentHeap() {
    return mParentHeap;
}

Heap *Heap::findContainHeap(const void *block) {
    Heap *containingHeap = NULL;
    MEMiHeapHead *memContainHeap = MEMFindContainHeap(block);
    if (memContainHeap) {
        containingHeap = findHeap(memContainHeap);
    }
    return containingHeap;
}

void Heap::free(void *block, Heap *heap) {
    if (heap == NULL) {
        MEMiHeapHead *containHeap = MEMFindContainHeap(block);
        if (!containHeap) {
            return;
        }
        heap = findHeap(containHeap);
        if (heap == NULL) {
            return;
        }
    }
    heap->free(block);
}

void Heap::dispose() {
    Disposer *it;
    while ((it = (Disposer *)nw4r::ut::List_GetNext(&mDisposerList, NULL)) != NULL) {
        it->~Disposer();
    }
}

void Heap::dump() {}

void Heap::dumpAll() {
    Heap *heap = NULL;
    u32 mem[2] = {0, 0};

    while ((heap = (Heap *)nw4r::ut::List_GetNext(&sHeapList, heap)) != NULL) {
        Heap *childHeap = NULL;
        if ((u32)heap < 0x90000000) {
            mem[0] += heap->getAllocatableSize(4);
        } else {
            mem[1] += heap->getAllocatableSize(4);
        }

        while ((childHeap = (Heap *)nw4r::ut::List_GetNext(&sHeapList, childHeap)) != NULL) {
            if (heap->mParentHeap == childHeap) {
                break;
            }
        }
    }
}

Heap *Heap::becomeCurrentHeap() {
    OSLockMutex(&sRootMutex);
    Heap *oldCurrentHeap = sCurrentHeap;
    sCurrentHeap = this;
    OSUnlockMutex(&sRootMutex);
    return oldCurrentHeap;
}

Heap *Heap::becomeCurrentHeapWithoutLock() {
    Heap *oldCurrentHeap = sCurrentHeap;
    sCurrentHeap = this;
    DCStoreRange(&sCurrentHeap, sizeof(sCurrentHeap));
    return oldCurrentHeap;
}

} // namespace EGG

void *operator new(size_t size) {
    return EGG::Heap::alloc(size, 4, NULL);
}
void *operator new(size_t, void *p) {
    return p;
}
void *operator new(size_t size, int align) {
    return EGG::Heap::alloc(size, align, NULL);
}
void *operator new(size_t size, EGG::Heap *heap, int align) {
    return EGG::Heap::alloc(size, align, heap);
}
void *operator new[](size_t size) {
    return EGG::Heap::alloc(size, 4, NULL);
}
void *operator new[](size_t size, EGG::Heap *heap, int align) {
    return EGG::Heap::alloc(size, align, heap);
}
void operator delete(void *p) {
    EGG::Heap::free(p, NULL);
}
void operator delete[](void *p) {
    EGG::Heap::free(p, NULL);
}
