#include <nw4r/ut.h>

#include <revolution/os.h>

namespace nw4r {
namespace ut {

NW4R_UT_RTTI_DEF_DERIVED(DvdLockedFileStream, DvdFileStream);

OSMutex DvdLockedFileStream::sMutex;
OSThreadQueue DvdLockedFileStream::sThreadQueue;
bool DvdLockedFileStream::sInitialized = false;

void DvdLockedFileStream::InitMutex_() {
    BOOL enabled = OSDisableInterrupts();

    if (!sInitialized) {
        OSInitMutex(&sMutex);
        OSInitThreadQueue(&sThreadQueue);
        sInitialized = true;
    }

    OSRestoreInterrupts(enabled);
}

DvdLockedFileStream::DvdLockedFileStream(s32 entryNum)
    : DvdFileStream(entryNum), mCancelFlag(false) {
    InitMutex_();
}

DvdLockedFileStream::DvdLockedFileStream(const DVDFileInfo* openedFile,
                                         bool closeEnable)
    : DvdFileStream(openedFile, closeEnable), mCancelFlag(false) {
    InitMutex_();
}

DvdLockedFileStream::~DvdLockedFileStream() {}

void DvdLockedFileStream::Close() {
    DvdFileStream::Close();
    mCancelFlag = false;
}

s32 DvdLockedFileStream::Read(void* buf, u32 length) {
    if (!LockMutex()) {
        return DVD_RESULT_CANCELED;
    }

    s32 ret = DvdFileStream::Read(buf, length);
    UnlockMutex();
    return ret;
}

s32 DvdLockedFileStream::Peek(void* buf, u32 length) {
    if (!LockMutex()) {
        return DVD_RESULT_CANCELED;
    }

    s32 ret = DvdFileStream::Peek(buf, length);
    UnlockMutex();
    return ret;
}

void DvdLockedFileStream::Cancel() {
    CancelMutex();
    DvdFileStream::Cancel();
}

bool DvdLockedFileStream::LockMutex() {
    BOOL enabled = OSDisableInterrupts();

    while (!OSTryLockMutex(&sMutex)) {
        OSSleepThread(&sThreadQueue);

        if (mCancelFlag) {
            OSRestoreInterrupts(enabled);
            return false;
        }
    }

    OSRestoreInterrupts(enabled);
    return true;
}

void DvdLockedFileStream::UnlockMutex() {
    BOOL enabled = OSDisableInterrupts();
    OSUnlockMutex(&sMutex);
    OSWakeupThread(&sThreadQueue);
    OSRestoreInterrupts(enabled);
}

void DvdLockedFileStream::CancelMutex() {
    BOOL enabled = OSDisableInterrupts();
    mCancelFlag = true;
    OSWakeupThread(&sThreadQueue);
    OSRestoreInterrupts(enabled);
}

} // namespace ut
} // namespace nw4r
