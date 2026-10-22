#include <lib/egg/core/eggArchive.h>

#include <lib/egg/core/eggHeap.h>
#include <string.h>

namespace EGG {

bool Archive::sIsArchiveListInitialized;
nw4r::ut::List Archive::sArchiveList;

Archive::Archive() {
    mRefCount = 1;
    mStatus = 0;
    memset(&mHandle, 0, sizeof(ARCHandle));
    appendList(this);
}

Archive::~Archive() {
    removeList(this);
}

bool Archive::initHandle(void *arcStart, int status) {
    bool ok = ARCInitHandle(arcStart, &mHandle);
    if (ok) {
        mStatus = status;
    } else {
        mStatus = 0;
    }
    return ok;
}

Archive *Archive::findArchive(void *arcStart) {
    Archive *found = NULL;
    if (sIsArchiveListInitialized) {
        Archive *it = NULL;
        while ((it = (Archive *)nw4r::ut::List_GetNext(&sArchiveList, it)) != NULL) {
            if (it->mHandle.archiveStartAddr == arcStart) {
                found = it;
                break;
            }
        }
    }
    return found;
}

void Archive::appendList(Archive *archive) {
    if (!sIsArchiveListInitialized) {
        nw4r::ut::List_Init(&sArchiveList, offsetof(Archive, mLink));
        sIsArchiveListInitialized = true;
    }
    nw4r::ut::List_Append(&sArchiveList, archive);
}

void Archive::removeList(Archive *archive) {
    nw4r::ut::List_Remove(&sArchiveList, archive);
}

Archive *Archive::mount(void *arcStart, Heap *heap, int alignment) {
    Archive *archive = findArchive(arcStart);
    if (archive == NULL) {
        archive = new (heap, alignment) Archive();
        if (!archive->initHandle(arcStart, 1)) {
            delete archive;
            archive = NULL;
        }
    } else {
        archive->mRefCount++;
    }
    return archive;
}

void Archive::unmount() {
    if (mRefCount != 0 && --mRefCount == 0) {
        mStatus = 0;
        delete this;
    }
}

void *Archive::getFile(const char *path, FileInfo *fileInfo) {
    ARCFileInfo info;
    void *result = NULL;
    if (open(path, &info)) {
        if (mStatus == 1) {
            result = ARCGetStartAddrInMem(&info);
        }
        if (fileInfo != NULL) {
            fileInfo->mFileOffset = ARCGetStartOffset(&info);
            fileInfo->mFileSize = ARCGetLength(&info);
        }
    }
    ARCClose(&info);
    return result;
}

} // namespace EGG
