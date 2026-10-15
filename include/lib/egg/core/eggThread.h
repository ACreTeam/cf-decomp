#pragma once

#include <lib/revolution/OS.h>

namespace EGG {

class Heap;
class ThreadMeter;

class Thread {
public:
    Thread(OSThread *, int); // 80440B74
    Thread(u32 stackSize, int messageCount, int priority, Heap *heap); // 80440AA4
    virtual ~Thread(); // 80440BE8
    virtual void *run();
    virtual void onEnter() {} // 802ADBB4
    virtual void onExit() {} // 802ADBB0

    OSThread *getOSThread() const { return mpOSThread; }
    void setCurrentHeap(Heap *heap); // 80440D40

    /* 0x04 */ Heap *mpHeap;
    /* 0x08 */ OSThread *mpOSThread;
    /* 0x0C */ u8 _0C[0x40];

    static void initialize();
    static ThreadMeter *sThreadMeter; // 8074FC18
}; // size 0x4C

} // namespace EGG
