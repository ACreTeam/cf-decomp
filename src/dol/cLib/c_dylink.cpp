#include <game/cLib/c_dylink.hpp>
#include <lib/revolution/os/OSContext.h>
#include <runtime/rel.h>
#include <cstdio>

DynamicModuleControlBase *DynamicModuleControlBase::mFirst;
DynamicModuleControlBase *DynamicModuleControlBase::mLast;

// 802AA11C
DynamicModuleControlBase::~DynamicModuleControlBase() {
    forceUnlink();
    if (mpPrev != nullptr) {
        mpPrev->mpNext = mpNext;
    }
    if (mpNext != nullptr) {
        mpNext->mpPrev = mpPrev;
    }
    if (mFirst == this) {
        mFirst = mpNext;
    }
    if (mLast == this) {
        mLast = mpPrev;
    }
    mpNext = nullptr;
    mpPrev = nullptr;
}

// 802AA1D8
DynamicModuleControlBase::DynamicModuleControlBase() : mUsageCount(0), mLinkCount(0), mpNext(nullptr) {
    if (mFirst == nullptr) {
        mFirst = this;
    }
    mpPrev = mLast;
    if (mpPrev != nullptr) {
        mpPrev->mpNext = this;
    }
    mLast = this;
}

// 802AA220
bool DynamicModuleControlBase::link() {
    if (mUsageCount == 0) {
        // Only load and link if this module isn't already loaded
        do_load();
        if (!do_link()) {
            return false;
        }
        if (mLinkCount < 0xffff) {
            mLinkCount++;
        }
    }
    if (mUsageCount < 0xffff) {
        mUsageCount++;
    }
    return true;
}

// 802AA2B4
bool DynamicModuleControlBase::unlink() {
    if (mUsageCount != 0) {
        if (--mUsageCount == 0) {
            do_unlink();
            do_unload();
        }
    }
    return true;
}

// 802AA320
bool DynamicModuleControlBase::load_async() {
    if (mUsageCount == 0) {
        return do_load_async();
    }
    return true;
}

// 802AA344
bool DynamicModuleControlBase::forceUnlink() {
    if (mUsageCount != 0) {
        mUsageCount = 0;
        do_unlink();
        do_unload();
    }
    return true;
}

// 802AA3A8
void DynamicModuleControlBase::dumpAll() {
    for (DynamicModuleControlBase *module = mFirst; module != nullptr; module = module->mpNext) {
        u16 links = module->mLinkCount;
        u16 usage = module->mUsageCount;
        if (links != 0 || usage != 0) {
            module->getModuleSize();
            module->getModuleName();
            module->dump();
        }
    }
}

const char *DynamicModuleControl::sModulesDir = "/rels";

size_t DynamicModuleControl::sAllocBytes;
EGG::DvdFile *DynamicModuleControl::sDvdFile;
EGG::ExpHeap *DynamicModuleControl::sDylinkHeap;

// 802AA434
DynamicModuleControl::DynamicModuleControl(const char *name, EGG::ExpHeap *heap) : DynamicModuleControlBase(),
    mpModule(nullptr),
    mpBss(nullptr),
    mPrologReturn(0),
    mModuleName(name),
    mType(MODULE_TYPE_UNKNOWN),
    mLoadCount(0),
    mChecksum(0),
    mModuleSize(0),
    mpDvdCallback(nullptr),
    mpHeap(heap) {
}

// 802AA4B4
DynamicModuleControl::~DynamicModuleControl() {}

// 802AA50C
void DynamicModuleControl::initialize(EGG::ExpHeap *heap) {
    sDylinkHeap = heap;
    sDvdFile = new(heap, 4) EGG::DvdFile();
    sAllocBytes = sizeof(EGG::DvdFile);
}

// 802AA554
void *DynamicModuleControl::callback(void *self) {
    DynamicModuleControl *dmc = (DynamicModuleControl *) self;
    return (void *) dmc->do_load();
}

// 802AA564
u16 calcSum2(const u16 *data, size_t count) {
    u16 sum = 0;
    while (count > 0) {
        sum += *data;
        count -= 2;
        data++;
    }
    return sum;
}

// 802AA58C
bool DynamicModuleControl::do_load() {
    // Check if already loaded
    if (mpModule != nullptr) {
        return true;
    }

    int attempts = 0;
    if (mpHeap == nullptr) {
        mpHeap = sDylinkHeap;
    }

    // On retry, free memory from the previous attempt
retry_load:
    if (mpModule != nullptr) {
        mpHeap->free(mpModule);
        mpModule = nullptr;
    }

    char buf[0x40];
    snprintf(buf, ARRAY_SIZE(buf), "%s/%sNP.rel", sModulesDir, mModuleName);
    if (mpModule == nullptr) {
        if (sDvdFile != nullptr && sDvdFile->open(buf)) {
            size_t size = sDvdFile->getFileSize();
            if (size != 0) {
                size = ROUND_UP(size, 32);
                mpModule = (OSModuleHeader *) mpHeap->alloc(size, 32);
                if (mpModule != nullptr) {
                    if (sDvdFile->readData(mpModule, size, 0) >= 0) {
                        mModuleSize = 0;
                        mType = MODULE_TYPE_DVD | 8;
                    } else {
                        mpHeap->free(mpModule);
                        mpModule = nullptr;
                    }
                }
            }
            sDvdFile->close();
        }
    }
    if (mpModule == nullptr) {
        return false;
    }

    // Retained checksum verification for load paths that set mModuleSize.
    if (mModuleSize > 0) {
        u16 actualChecksum = calcSum2((u16 *) mpModule, mModuleSize);

        // On first load, load the file again to verify
        if (mLoadCount == 0) {
            mChecksum = actualChecksum;
            mLoadCount++;
            goto retry_load;
        }

        // If the checksum fails, try to read the data 2 more times before quitting
        if (actualChecksum != mChecksum) {
            mLoadCount = 0;
            if (++attempts < 3) {
                goto retry_load;
            }
            return false;
        }
        if (mLoadCount < 255) {
            mLoadCount++;
        }
    } else {
        if (mLoadCount < 255) {
            mLoadCount++;
        }
    }

    return true;
}

// 802AA7AC
bool DynamicModuleControl::do_load_async() {
    if (mpDvdCallback == nullptr) {
        if (mpModule != nullptr) {
            return true;
        }
        mpDvdCallback = mDvd_callback_c::create(callback, this);
    }

    if (mpDvdCallback != nullptr && mpDvdCallback->mDone == true) {
        mpDvdCallback->destroy();
        mpDvdCallback = nullptr;
        return true;
    }

    return false;
}

// 802AA838
bool DynamicModuleControl::do_unload() {
    if (mpModule != nullptr) {
        mpHeap->free(mpModule);
        mpModule = nullptr;
    }
    return true;
}

// 802AA88C
void DynamicModuleControl::dump() {}

// 802AA890
bool DynamicModuleControl::do_link() {
    size_t fixSize;
    void *bssStart;
    size_t blockSize;
    if (mpModule == nullptr) {
        goto fail;
    }

    fixSize = ROUND_UP(mpModule->fixSize, 0x20);
    bssStart = (void *) ((u32) mpModule + fixSize);
    blockSize = EGG::ExpHeap::getSizeForMBlock(mpModule);
    if (blockSize == 0) {
        void *bss = mpHeap->alloc(mpModule->bssSize, 0x20);
        if (bss == nullptr) {
            goto fail;
        }
        mpBss = bss;
        if (OSLink(&mpModule->info, bss)) {
            goto linked;
        }
        goto fail;
    } else if (fixSize + mpModule->bssSize < blockSize) {
        if (!OSLinkFixed(&mpModule->info, bssStart)) {
            goto fail;
        }
        mpHeap->resizeForMBlock(mpModule, fixSize + mpModule->bssSize);
    } else if (mpHeap->resizeForMBlock(mpModule, fixSize + mpModule->bssSize) > 0) {
        if (OSLinkFixed(&mpModule->info, bssStart)) {
            goto linked;
        }
        goto fail;
    } else {
        void *bss = mpHeap->alloc(mpModule->bssSize, 0x20);
        if (bss == nullptr) {
            goto fail;
        }
        mpBss = bss;
        if (!OSLinkFixed(&mpModule->info, mpBss)) {
            goto fail;
        }
        mpHeap->resizeForMBlock(mpModule, fixSize);
    }
linked:
    sAllocBytes += getModuleSize();

    // The release build still formats the debug map path, but does not register it.
    char mapFileBuf[0x40];
    snprintf(mapFileBuf, ARRAY_SIZE(mapFileBuf), "%s/%sNP.map", "/maps", mModuleName);

    // Run prolog
    mPrologReturn = ((u32FctPtr) mpModule->prolog)();

    return true;

fail:
    mLoadCount = 0;
    if (mpBss != nullptr) {
        mpHeap->free(mpBss);
        mpBss = nullptr;
    }
    if (mpModule != nullptr) {
        mpHeap->free(mpModule);
        mpModule = nullptr;
    }
    return false;
}

// 802AAAC8
bool DynamicModuleControl::do_unlink() {
    ((voidFctPtr) mpModule->epilog)();

    if (!OSUnlink(&mpModule->info)) {
        return false;
    }

    sAllocBytes -= getModuleSize();
    if (mpBss != nullptr) {
        mpHeap->free(mpBss);
        mpBss = nullptr;
    }
    return true;
}

// 802AAB64
size_t DynamicModuleControl::getModuleSize() const {
    if (mpModule != nullptr) {
        size_t size = EGG::ExpHeap::getSizeForMBlock(mpModule);
        if (mpBss) {
            size += mpModule->bssSize;
        }
        return size;
    }
    return 0;
}

// 802AABC0
const char *DynamicModuleControl::getModuleTypeString() const {
    static const char *c_ModuleTypeStrings[] = { "????", "MEM", "ARAM", "DVD" };
    return c_ModuleTypeStrings[mType & 0b11];
}

// 802AABD8
void finalizeProlog() {}

// 802AABDC
void finalizeEpilog() {}

extern "C" {

// 802AABE0
void ModuleUnresolved() {
    u32 depth = 0;
    u32 *frame = (u32 *) OSGetStackPointer();
    while (frame != nullptr && frame != (u32 *) 0xFFFFFFFF && depth++ < 16) {
        frame = (u32 *) *frame;
    }
}

// 802AAC34
void ModuleConstructorsX(voidFctPtr *ctors) {
    while (*ctors != nullptr) {
        (*ctors)();
        ctors++;
    }
}

// 802AAC78
void ModuleDestructorsX(voidFctPtr *dtors) {
    while (*dtors != nullptr) {
        (*dtors)();
        dtors++;
    }
}

} // extern "C"
