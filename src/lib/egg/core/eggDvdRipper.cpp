#include <lib/egg/core/eggDvdRipper.h>

#include <revolution/OS.h>
#include <revolution/VI.h>

namespace EGG {

DvdRipper::DvdRipperCallback DvdRipper::sCallback = NULL;
bool DvdRipper::sErrorRetry = true;

inline DvdRipper::Arg::Arg() {
    _04 = 0;
    _08 = 0;
    _0c = 1;
    _10 = 0;
    _14 = _18;
}

void *DvdRipper::loadToMainRAM(s32 entryNum, void *buffer, Heap *heap, EAllocDirection direction,
                               u32 offset, u32 *readSize, u32 *fileSize) {
    DvdFile dvdFile;
    if (!dvdFile.open(entryNum)) {
        return NULL;
    }
    return loadToMainRAM(&dvdFile, buffer, heap, direction, offset, readSize, fileSize);
}

void *DvdRipper::loadToMainRAM(DvdFile *dvdFile, void *buffer, Heap *heap,
                               EAllocDirection direction, u32 offset, u32 *readSize,
                               u32 *fileSize) {
    u32 roundedFileSize, roundedFileSize2;
    bool allocatedFromHeap = false;

    if (sCallback != NULL) {
        Arg arg;
        sCallback(arg);
    }

    u32 exactFileSize = dvdFile->getFileSize();
    if (fileSize != NULL) {
        *fileSize = exactFileSize;
    }
    roundedFileSize = (exactFileSize + 31) & ~31;

    if (buffer == NULL) {
        s32 align = direction == ALLOC_DIRECTION_HEAD ? 32 : -32;
        buffer = Heap::alloc(roundedFileSize - offset, align, heap);
        allocatedFromHeap = true;
    }

    if (buffer == NULL) {
        return NULL;
    }

    if (offset != 0) {
        u8 buf[64];
        u8 *buf_ptr = (u8 *)(((s32)&buf + 31) & ~31);
        while (true) {
            s32 result = DVDReadPrio(dvdFile->getFileInfo(), buf_ptr, 32, offset, 2);
            if (result >= 0) {
                break;
            }

            if (result == -3 || !sErrorRetry) {
                if (allocatedFromHeap) {
                    Heap::free(buffer, NULL);
                }
                return NULL;
            }

            VIWaitForRetrace();
        }

        DCInvalidateRange(buf_ptr, 32);
    }

    roundedFileSize2 = roundedFileSize - offset;
    while (true) {
        s32 result = DVDReadPrio(dvdFile->getFileInfo(), buffer, roundedFileSize2, offset, 2);
        if (result >= 0) {
            break;
        }

        if (result == -3 || !sErrorRetry) {
            if (allocatedFromHeap) {
                Heap::free(buffer, NULL);
            }
            return NULL;
        }

        VIWaitForRetrace();
    }

    if (readSize) {
        *readSize = roundedFileSize2;
    }

    return buffer;
}

} // namespace EGG
