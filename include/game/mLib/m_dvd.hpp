#pragma once

#include <types.h>
#include <revolution/GX.h>
#include <lib/egg/core/eggHeap.h>

class mDvd_command_c {
public:
    /* 0x00 */ mDvd_command_c *mpNext;
    /* 0x04 */ bool mDone;

    mDvd_command_c(); // 802AD780
    virtual ~mDvd_command_c(); // 802AD4F0
    virtual BOOL execute() = 0;

    void destroy(); // 802AD79C
    bool isDone() const { return mDone; }

    static void *operator new(size_t size); // 802AD530
    static void operator delete(void *ptr); // 802AD53C
}; // size 0xC, vptr at 0x08

class mDvd_callback_c : public mDvd_command_c {
public:
    typedef void *(*Callback)(void *);

    mDvd_callback_c(Callback callback, void *param); // 802AD814
    virtual ~mDvd_callback_c(); // 802AD7BC
    virtual BOOL execute() override; // 802AD8F4

    static mDvd_callback_c *create(Callback callback, void *param); // 802AD878

    /* 0x0C */ Callback mpCallback;
    /* 0x10 */ void *mpParam;
    /* 0x14 */ void *mpResult;
}; // size 0x18

class mDvd_toMainRam_c : public mDvd_command_c {
public:
    mDvd_toMainRam_c(u8 allocDirection); // 802AD944
    virtual ~mDvd_toMainRam_c(); // 802ADAF4
    virtual BOOL execute() override; // 802ADA58

    static mDvd_toMainRam_c *create(const char *path, u8 allocDirection, EGG::Heap *heap); // 802AD9B8

    /* 0x0C */ u8 mAllocDirection;
    /* 0x10 */ s32 mEntryNum;
    /* 0x14 */ void *mpData;
    /* 0x18 */ s32 mSize; // Bytes read, rounded up to 32.
    /* 0x1C */ u32 mFileSize; // Original file size before rounding.
    /* 0x20 */ EGG::Heap *mpHeap;
}; // size 0x24

namespace mDvd {
void create(int priority, EGG::Heap *commandHeap, EGG::Heap *archiveHeap, const GXColor *perfColor); // 802AD464
s32 convertPathToEntrynum(const char *path); // 802AD4EC
}
