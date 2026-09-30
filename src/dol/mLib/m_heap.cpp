#include <game/mlib/m_heap.hpp>
#include <constants/sjis_constants.h>

EGG::Heap *mHeap::s_SavedCurrentHeap;

EGG::ExpHeap *mHeap::g_gameHeap;
EGG::ExpHeap *mHeap::g_archiveHeap;
EGG::ExpHeap *mHeap::g_commandHeap;
EGG::ExpHeap *mHeap::g_dylinkHeap;
EGG::AssertHeap *mHeap::g_assertHeap;

u16 mHeap::GetOptFlag(AllocOptBit_t opt) {
    u16 ret = OPT_NONE;

    if (opt & OPT_CLEAR_ALLOC) {
        ret = MEM_HEAP_OPT_CLEAR_ALLOC;
    }

    if (opt & OPT_DEBUG_FILL) {
        ret |= MEM_HEAP_OPT_DEBUG_FILL;
    }

    if (opt & OPT_THREAD_SAFE) {
        ret |= MEM_HEAP_OPT_CAN_LOCK;
    }

    return ret;
}

EGG::Heap *mHeap::setCurrentHeap(EGG::Heap *heap) {
    return heap->becomeCurrentHeap();
}

EGG::ExpHeap *mHeap::createExpHeap(size_t size, EGG::Heap *parent, const char *name, ulong align, AllocOptBit_t opt) {
    if (parent == nullptr) {
        parent = EGG::Heap::getCurrentHeap();
    }

    if (align < 0x20) {
        align = 0x20;
    }

    if (size != -1) {
        size = expHeapCost(size, align);
    } else {
        size = parent->getAllocatableSize(align);
    }

    void *buffer = parent->alloc(size, align);
    EGG::ExpHeap *heap;

    if (buffer == nullptr) {
        return nullptr;
    }

    heap = EGG::ExpHeap::create(buffer, size, GetOptFlag(opt));
    if (heap == nullptr) {
        parent->free(buffer);
    } else if (name != nullptr) {
        heap->setName(name);
    }

    return heap;
}

size_t mHeap::expHeapCost(size_t size, ulong align) {
    return size + nw4r::ut::RoundUp<size_t>(sizeof(EGG::ExpHeap) + MEM_EXP_HEAP_HEAD_SIZE, align);
}

EGG::FrmHeap *mHeap::createFrmHeap(size_t size, EGG::Heap *parent, const char *name, ulong align, AllocOptBit_t opt) {
    if (parent == nullptr) {
        parent = EGG::Heap::getCurrentHeap();
    }

    if (align < 0x20) {
        align = 0x20;
    }

    if (size != -1) {
        size = frmHeapCost(size, align);
    } else {
        size = parent->getAllocatableSize(align);
    }

    void *buffer = parent->alloc(size, align);
    EGG::FrmHeap *heap;

    if (buffer == nullptr) {
        return nullptr;
    }

    heap = EGG::FrmHeap::create(buffer, size, GetOptFlag(opt));
    if (heap == nullptr) {
        parent->free(buffer);
    } else if (name != nullptr) {
        heap->setName(name);
    }

    return heap;
}

void mHeap::destroyFrmHeap(EGG::FrmHeap *heap) {
    if (heap != nullptr) {
        heap->destroy();
    }
}

size_t mHeap::adjustFrmHeap(EGG::FrmHeap *heap) {
    if (heap == nullptr) {
        return 0;
    }

    size_t freeSpace = heap->adjust();
    size_t minCost = frmHeapCost(0, 4);
    if (freeSpace >= minCost) {
        freeSpace -= minCost;
    }

    return freeSpace;
}

size_t mHeap::frmHeapCost(size_t size, ulong align) {
    return size + nw4r::ut::RoundUp<size_t>(sizeof(EGG::FrmHeap) + MEM_FRM_HEAP_HEAD_SIZE, align);
}

void mHeap::saveCurrentHeap() {
    s_SavedCurrentHeap = EGG::Heap::getCurrentHeap();
}

void mHeap::restoreCurrentHeap() {
    s_SavedCurrentHeap->becomeCurrentHeap();
    s_SavedCurrentHeap = nullptr;
}

EGG::FrmHeap *mHeap::createFrmHeapToCurrent(size_t size, EGG::Heap *parent, const char *name, ulong align, AllocOptBit_t opt) {
    EGG::FrmHeap *heap = createFrmHeap(size, parent, name, align, opt);
    if (heap == nullptr) {
        return nullptr;
    }

    s_SavedCurrentHeap = EGG::Heap::getCurrentHeap();
    setCurrentHeap(heap);
    return heap;
}

EGG::Heap *mHeap::createGameHeap(size_t size, EGG::Heap *parent) {
    g_gameHeap = EGG::ExpHeap::create(size, parent, MEM_HEAP_OPT_CAN_LOCK);
    g_gameHeap->setAllocMode(MEM_EXP_HEAP_ALLOC_FAST);
    g_gameHeap->setName(GAME_HEAP_NAME);
    return g_gameHeap;
}

EGG::Heap *mHeap::createArchiveHeap(size_t size, EGG::Heap *parent) {
    g_archiveHeap = EGG::ExpHeap::create(size, parent, MEM_HEAP_OPT_CAN_LOCK);
    g_archiveHeap->setAllocMode(MEM_EXP_HEAP_ALLOC_FAST);
    g_archiveHeap->setName(ARCHIVE_HEAP_NAME);
    return g_archiveHeap;
}

EGG::Heap *mHeap::createCommandHeap(size_t size, EGG::Heap *parent) {
    g_commandHeap = EGG::ExpHeap::create(size, parent, MEM_HEAP_OPT_CAN_LOCK);
    g_commandHeap->setAllocMode(MEM_EXP_HEAP_ALLOC_FAST);
    g_commandHeap->setName(COMMAND_HEAP_NAME);
    return g_commandHeap;
}

EGG::Heap *mHeap::createDylinkHeap(size_t size, EGG::Heap *parent) {
    g_dylinkHeap = EGG::ExpHeap::create(size, parent, MEM_HEAP_OPT_CAN_LOCK);
    g_dylinkHeap->setAllocMode(MEM_EXP_HEAP_ALLOC_FAST);
    g_dylinkHeap->setName(DYLINK_HEAP_NAME);
    return g_dylinkHeap;
}

EGG::Heap *mHeap::createAssertHeap(EGG::Heap *parent) {
    size_t size = EGG::AssertHeap::getMinSizeForCreate();
    g_assertHeap = EGG::AssertHeap::create(size, parent);
    g_assertHeap->setName(ASSERT_HEAP_NAME);
    return g_assertHeap;
}
