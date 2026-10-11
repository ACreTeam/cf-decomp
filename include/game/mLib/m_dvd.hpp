#pragma once
#include <types.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggArchive.h>

class mDvd_command_c {
public:
    virtual ~mDvd_command_c();
    virtual void execute();
    virtual void init();

    void destroy(); // 802AD79C
    void waitDone() const;

    bool isDone() const { return mDone; }

    /* 0x04 */ bool mDone;
    /* 0x08 */ int mReadLength;
}; // size 0xC

class mDvd_callback_c : public mDvd_command_c {
public:
    static mDvd_callback_c *create(void *(*callback)(void *), void *param);
    static mDvd_callback_c *createOrDie(void *(*callback)(void *), void *param);
};

class mDvd_mountMemArchive_c : public mDvd_command_c {
public:
    void *getArcBinary() const;

    static mDvd_mountMemArchive_c *create(const char *name, u8 allocDirection, EGG::Heap *heap);

    u8 mAllocDirection;
    u8 mCompressionType;
    int mEntryNum;
    EGG::Archive *mpArchive;
    EGG::Heap *mpHeap;
    unsigned long mArchiveSize;
};

class mDvd_toMainRam_c : public mDvd_command_c {
public:
    static mDvd_toMainRam_c *create(const char *path, u8 allocDirection, EGG::Heap *heap); // 802AD9B8

    /* 0x0C */ u8 mAllocDirection;
    /* 0x10 */ s32 mEntryNum;
    /* 0x14 */ void *mpData;
    /* 0x18 */ s32 mSize;
    /* 0x1C */ u32 _1C;
    /* 0x20 */ EGG::Heap *mpHeap;
}; // size 0x24
