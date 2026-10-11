// Archive / resource loaders (dDvd::loader_c, bank_c, arcBank_c). .text 80085E8C..80086558,
// .data 804DE838..804DE860 (the bank_c and loader_c vtables). See include/game/game/d_dvd.hpp.
#include <game/game/d_dvd.hpp>
#include <lib/revolution/OS.h>
#include <cstring>

namespace dDvd {

// 80085E8C
loader_c::loader_c() {
    mpCommand = NULL;
    mStatus = -1;
    mpHeap = NULL;
    mpData = NULL;
}

// 80085EB4
loader_c::~loader_c() {}

// 80085EF4
void *loader_c::request(const char *path, u8 mountDirection, void *heap) {
    if (mpData != NULL) {
        return mpData;
    }
    if (mpCommand == NULL) {
        mStatus = -2;
        mpCommand = mDvd_toMainRam_c::create(path, mountDirection, (EGG::Heap *)heap);
        mpHeap = heap;
    }
    if (mpCommand != NULL && mpCommand->mDone == 1) {
        mpData = mpCommand->mpData;
        mStatus = mpCommand->mSize;
        mpCommand->destroy();
        mpCommand = NULL;
        return mpData;
    }
    return NULL;
}

// 80085FAC
BOOL loader_c::release() {
    if (!cancelRequest()) {
        return FALSE;
    }
    mpData = NULL;
    mpHeap = NULL;
    mStatus = -1;
    return TRUE;
}

// 80086000
void loader_c::freeData() {
    if (mpHeap != NULL && mpData != NULL) {
        ((EGG::Heap *)mpHeap)->free(mpData);
        mpData = NULL;
    }
}

// 8008605C
BOOL loader_c::cancelRequest() {
    if (mpCommand != NULL) {
        if (mpCommand->mDone != 1) {
            return FALSE;
        }
        mpData = mpCommand->mpData;
        freeData();
        mpCommand->destroy();
        mpCommand = NULL;
    }
    return TRUE;
}

// 800860D0
bank_c::bank_c() {
    mEntry = 0xFF;
    mLoading = 0;
}

// 8008611C
s32 loader_c::getSize() const {
    if (mStatus < 0) {
        return -1;
    }
    return mStatus;
}

// 80086134
BOOL bank_c::load(const char *path, void *heap, u8 entry) {
    if (mpData != NULL) {
        return TRUE;
    }
    if (mEntry == 0xFF) {
        mEntry = entry;
        strncpy(mPath, path, sizeof(mPath) - 1);
    }
    if (mpData == NULL) {
        mLoading = 1;
        mpData = request(path, entry, heap);
        if (mpData != NULL) {
            onLoaded();
            mLoading = 0;
            return TRUE;
        }
    }
    return FALSE;
}

// 8008620C
BOOL bank_c::unload(BOOL wait) {
    if (wait) {
        if (!release()) {
            return FALSE;
        }
    } else if (mLoading) {
        if (!load(mPath, mpHeap, mEntry)) {
            return FALSE;
        }
    }
    if (mpData != NULL) {
        freeData();
    }
    if (release()) {
        mEntry = 0xFF;
        mLoading = 0;
        return TRUE;
    }
    return FALSE;
}

// 800862C8
BOOL bank_c::checkLoaded() {
    if (mLoading) {
        if (!load(mPath, mpHeap, mEntry)) {
            return FALSE;
        }
    }
    mEntry = 0xFF;
    mLoading = 0;
    return TRUE;
}

// 80086330
s32 arcBank_c::getArcFileSize(const char *name, void *arc) {
    if (arc != NULL) {
        ARCFileInfo info;
        if (ARCOpen(&mHandle, name, &info)) {
            s32 size = ARCGetLength(&info);
            ARCClose(&info);
            return size;
        }
        return -2;
    }
    return -1;
}

// 80086398
u32 arcBank_c::getFileSize(const char *name) {
    return getArcFileSize(name, mpData);
}

// 800863A0
void *arcBank_c::getFile(const char *name, u32 *size) {
    if (mpData == NULL) {
        return NULL;
    }
    if (!mArcReady) {
        return NULL;
    }
    u32 tmpSize;
    if (size == NULL) {
        size = &tmpSize;
    }
    *size = 0;
    if (ARCConvertPathToEntrynum(&mHandle, name) != -1) {
        ARCFileInfo info;
        if (ARCOpen(&mHandle, name, &info)) {
            *size = ARCGetLength(&info);
            void *data = ARCGetStartAddrInMem(&info);
            ARCClose(&info);
            return data;
        }
    }
    return NULL;
}

// 80086474
BOOL arcBank_c::copyFile(void *dst, const char *name) {
    u32 size;
    void *data = getFile(name, &size);
    if (data != NULL) {
        memcpy(dst, data, size);
        DCFlushRange(dst, size);
        return TRUE;
    }
    return FALSE;
}

// 800864D8
BOOL arcBank_c::bindFile(void *dst, const char *name) {
    if (copyFile(dst, name)) {
        nw4r::g3d::ResFile res(dst);
        res.Init();
        res.Bind(res);
        return TRUE;
    }
    return FALSE;
}

// 80086544
BOOL arcBank_c::closeArc() {
    mArcReady = 0;
    return unload(FALSE);
}

// 80086554
void bank_c::onLoaded() {}

} // namespace dDvd
