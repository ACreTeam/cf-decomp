#pragma once

#include <lib/egg/core/eggFile.h>
#include <nw4r/ut.h>
#include <revolution/DVD.h>
#include <revolution/OS.h>

namespace EGG {

class DvdFile : public File {
public:
    static void initialize();

    DvdFile();
    virtual ~DvdFile();

    virtual bool open(const char *path);
    virtual void close();
    virtual s32 readData(void *buffer, s32 length, s32 offset);
    virtual s32 writeData(const void *buffer, s32 length, s32 offset);
    virtual u32 getFileSize() const { return mFileInfo.size; }
    virtual bool open(s32 entryNum);
    virtual bool open(const char *path, void *);

    DVDFileInfo *getFileInfo() { return &mFileInfo; }

private:
    void initiate();
    s32 sync();
    static void doneProcess(s32 result, DVDFileInfo *fileInfo);

    struct FileInfo : public DVDFileInfo {
        DvdFile *dvdFile;
    };

    OSMutex mMutex;          // 0x08
    OSMutex _20;             // 0x20
    u32 _38;                 // 0x38
    FileInfo mFileInfo;      // 0x3C
    OSMessageQueue _7C;      // 0x7C
    OSMessage _9C;           // 0x9C
    OSMessageQueue mMsgQueue; // 0xA0
    OSMessage mMsg;          // 0xC0
    OSThread *mThread;       // 0xC4
    nw4r::ut::Link mNode;    // 0xC8

    static bool sIsInitialized;
    static nw4r::ut::List sDvdList;
};

} // namespace EGG
