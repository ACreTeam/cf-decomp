#pragma once
#include <lib/egg/core/eggDisposer.h>
#include <nw4r/ut.h>
#include <revolution/ARC.h>

namespace EGG {
class Heap;

class Archive : public Disposer {
public:
    struct FileInfo {
        FileInfo() : mFileOffset(0), mFileSize(0) {}
        int mFileOffset;
        int mFileSize;
    };

    Archive();
    virtual ~Archive();

    static Archive *mount(void *arcStart, Heap *heap, int alignment);
    void unmount();
    void *getFile(const char *path, FileInfo *fileInfo);

private:
    bool initHandle(void *arcStart, int status);
    bool open(const char *path, ARCFileInfo *info) { return ARCOpen(&mHandle, path, info); }
    static Archive *findArchive(void *arcStart);
    static void appendList(Archive *archive);
    static void removeList(Archive *archive);

    int mStatus;           // 0x10
    int mRefCount;         // 0x14
    ARCHandle mHandle;     // 0x18
    nw4r::ut::Link mLink;  // 0x34

    static bool sIsArchiveListInitialized;
    static nw4r::ut::List sArchiveList;
};
} // namespace EGG
