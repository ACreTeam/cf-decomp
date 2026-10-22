#pragma once

#include <nw4r/ut.h>
#include <revolution/OS.h>

namespace EGG {

class Heap;
class ThreadMeter;

class Thread {
public:
    Thread(u32 stackSize, int messageCount, int priority, Heap *heap);
    Thread(OSThread *osThread, int messageCount);
    virtual ~Thread();
    virtual void *run();
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();
    static Thread *findThread(OSThread *osThread);

    OSThread *getOSThread() const { return mOSThread; }
    Heap *getAllocatableHeap() const { return mAllocatableHeap; }
    void setCurrentHeap(Heap *heap);

    void resume() { OSResumeThread(mOSThread); }
    BOOL sendMessage(OSMessage message) { return OSSendMessage(&mMesgQueue, message, 0); }
    OSMessage waitMessageBlock() {
        OSMessage message;
        OSReceiveMessage(&mMesgQueue, &message, OS_MSG_BLOCKING);
        return message;
    }

    static ThreadMeter *sThreadMeter; // 8074FC18

private:
    void setCommonMesgQueue(int messageCount, Heap *heap);
    static void switchThreadCallback(OSThread *from, OSThread *to);
    static void *start(void *thread);

    Heap *mContainingHeap;     // 0x04
    OSThread *mOSThread;       // 0x08
    OSMessageQueue mMesgQueue; // 0x0C
    OSMessage *mMesgBuffer;    // 0x2C
    int mMesgCount;            // 0x30
    void *mStackMemory;        // 0x34
    u32 mStackSize;            // 0x38
    Heap *mAllocatableHeap;    // 0x3C
    Heap *mCurrentHeap;        // 0x40
    nw4r::ut::Link mLink;      // 0x44

    static nw4r::ut::List sThreadList;
};

} // namespace EGG
