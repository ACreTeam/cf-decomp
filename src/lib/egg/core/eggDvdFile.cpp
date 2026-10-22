#include <lib/egg/core/eggDvdFile.h>

#include <stddef.h>

namespace EGG {

bool DvdFile::sIsInitialized;
nw4r::ut::List DvdFile::sDvdList;

void DvdFile::initialize() {
    if (!sIsInitialized) {
        nw4r::ut::List_Init(&sDvdList, offsetof(DvdFile, mNode));
        sIsInitialized = true;
    }
}

DvdFile::DvdFile() {
    initiate();
}

DvdFile::~DvdFile() {
    close();
}

void DvdFile::initiate() {
    mFileInfo.dvdFile = this;
    OSInitMutex(&mMutex);
    OSInitMutex(&_20);
    OSInitMessageQueue(&mMsgQueue, &mMsg, 1);
    OSInitMessageQueue(&_7C, &_9C, 1);
    mThread = NULL;
    _38 = 0;
}

bool DvdFile::open(s32 entryNum) {
    if (!mIsOpen && entryNum != -1) {
        mIsOpen = DVDFastOpen(entryNum, &mFileInfo);
        if (mIsOpen) {
            nw4r::ut::List_Append(&sDvdList, this);
            DVDGetCommandBlockStatus(&mFileInfo.cb);
        }
    }
    return mIsOpen;
}

bool DvdFile::open(const char *path) {
    s32 entryNum = DVDConvertPathToEntrynum(path);
    return open(entryNum);
}

bool DvdFile::open(const char *path, void *) {
    return open(path);
}

void DvdFile::close() {
    if (mIsOpen && DVDClose(&mFileInfo)) {
        mIsOpen = false;
        nw4r::ut::List_Remove(&sDvdList, this);
    }
}

s32 DvdFile::readData(void *buffer, s32 length, s32 offset) {
    OSLockMutex(&mMutex);

    if (mThread != NULL) {
        OSUnlockMutex(&mMutex);
        return -1;
    }

    mThread = OSGetCurrentThread();
    s32 result = -1;
    if (DVDReadAsyncPrio(&mFileInfo, buffer, length, offset, &doneProcess, 2)) {
        result = sync();
    }
    mThread = NULL;

    OSUnlockMutex(&mMutex);
    return result;
}

s32 DvdFile::writeData(const void *buffer, s32 length, s32 offset) {
    return -1;
}

s32 DvdFile::sync() {
    OSLockMutex(&mMutex);

    OSMessage message;
    OSReceiveMessage(&mMsgQueue, &message, OS_MESSAGE_BLOCK);

    mThread = NULL;

    OSUnlockMutex(&mMutex);

    return (s32)message;
}

void DvdFile::doneProcess(s32 result, DVDFileInfo *fileInfo) {
    DvdFile *dvdFile = static_cast<FileInfo *>(fileInfo)->dvdFile;
    OSSendMessage(&dvdFile->mMsgQueue, (OSMessage)result, 0);
}

} // namespace EGG
