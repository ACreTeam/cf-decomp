#include <lib/egg/core/eggThread.h>

#include <lib/egg/core/eggHeap.h>
#include <lib/egg/core/eggThreadMeter.h>
#include <stddef.h>

namespace EGG {

nw4r::ut::List Thread::sThreadList;
ThreadMeter *Thread::sThreadMeter;

Thread::Thread(u32 stackSize, int messageCount, int priority, Heap *heap) {
    if (heap == NULL) {
        heap = Heap::getCurrentHeap();
    }

    mContainingHeap = heap;
    mStackSize = stackSize & ~31;
    mStackMemory = Heap::alloc(mStackSize, 32, mContainingHeap);
    mOSThread = (OSThread *)Heap::alloc(sizeof(OSThread), 32, mContainingHeap);

    OSCreateThread(mOSThread, (OSThreadFunc)start, this, (u8 *)mStackMemory + mStackSize,
                   mStackSize, priority, 1);

    mAllocatableHeap = NULL;
    mCurrentHeap = NULL;
    setCommonMesgQueue(messageCount, mContainingHeap);
}

Thread::Thread(OSThread *osThread, int messageCount) : mContainingHeap(NULL), mOSThread(osThread) {
    mStackSize = (u8 *)osThread->stackBase - (u8 *)osThread->stackEnd;
    mStackMemory = osThread->stackEnd;

    mAllocatableHeap = NULL;
    mCurrentHeap = NULL;
    setCommonMesgQueue(messageCount, Heap::getCurrentHeap());
}

Thread::~Thread() {
    nw4r::ut::List_Remove(&sThreadList, this);

    if (mContainingHeap != NULL) {
        if (!OSIsThreadTerminated(mOSThread)) {
            OSDetachThread(mOSThread);
            OSCancelThread(mOSThread);
        }

        Heap::free(mStackMemory, mContainingHeap);
        Heap::free(mOSThread, mContainingHeap);
    }

    Heap::free(mMesgBuffer, NULL);
}

Thread *Thread::findThread(OSThread *osThread) {
    Thread *it = NULL;
    while ((it = (Thread *)nw4r::ut::List_GetNext(&sThreadList, it)) != NULL) {
        if (it->mOSThread == osThread) {
            return it;
        }
    }
    return NULL;
}

void Thread::initialize() {
    nw4r::ut::List_Init(&sThreadList, offsetof(Thread, mLink));
    OSSetSwitchThreadCallback(switchThreadCallback);
}

void Thread::setCurrentHeap(Heap *heap) {
    OSDisableScheduler();
    OSThread *thread = mOSThread;
    if (OSGetCurrentThread() != thread) {
        mCurrentHeap = heap;
    } else if (heap != NULL) {
        if (mCurrentHeap == NULL) {
            mCurrentHeap = Heap::getCurrentHeap();
        }
        heap->becomeCurrentHeapWithoutLock();
    } else if (mCurrentHeap != NULL) {
        mCurrentHeap->becomeCurrentHeapWithoutLock();
        mCurrentHeap = NULL;
    }
    OSEnableScheduler();
}

void Thread::switchThreadCallback(OSThread *from, OSThread *to) {
    Thread *fromThread = from != NULL ? findThread(from) : NULL;
    Thread *toThread = to != NULL ? findThread(to) : NULL;

    if (fromThread != NULL) {
        fromThread->onExit();
        if (fromThread->mCurrentHeap != NULL) {
            Heap *current = Heap::getCurrentHeap();
            fromThread->mCurrentHeap->becomeCurrentHeapWithoutLock();
            fromThread->mCurrentHeap = current;
        }
    }

    ThreadMeter::onSwitchThread(from, to);

    if (toThread != NULL) {
        if (toThread->mCurrentHeap != NULL) {
            Heap *current = Heap::getCurrentHeap();
            toThread->mCurrentHeap->becomeCurrentHeapWithoutLock();
            toThread->mCurrentHeap = current;
        }
        toThread->onEnter();
    }
}

void Thread::setCommonMesgQueue(int messageCount, Heap *heap) {
    mMesgCount = messageCount;
    mMesgBuffer = (OSMessage *)Heap::alloc(messageCount * sizeof(OSMessage), 4, heap);
    OSInitMessageQueue(&mMesgQueue, mMesgBuffer, mMesgCount);
    nw4r::ut::List_Append(&sThreadList, this);
}

void *Thread::start(void *thread) {
    return ((Thread *)thread)->run();
}

void *Thread::run() {
    return NULL;
}

void ThreadMeter::onSwitchThread(OSThread *from, OSThread *to) {
    if (Thread::sThreadMeter == NULL) {
        return;
    }

    FrameRecord *frame = &Thread::sThreadMeter->mFrames[Thread::sThreadMeter->mCurrentFrame];
    if (frame->mState != 1) {
        return;
    }

    u32 tick = OSGetTick();
    Slice *slices = frame->mpSlices;
    int last = frame->mSliceCount - 1;
    if (slices[last].mpThread == from) {
        slices[last].mEndTick = tick;
    }

    if (to != NULL && frame->mSliceCount < Thread::sThreadMeter->mMaxSlices) {
        frame->mpSlices[frame->mSliceCount].mpThread = to;
        frame->mpSlices[frame->mSliceCount].mStartTick = tick;
        frame->mSliceCount++;
    }
}

} // namespace EGG
