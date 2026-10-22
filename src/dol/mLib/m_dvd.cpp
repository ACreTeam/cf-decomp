#include <game/mLib/m_dvd.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_perf.hpp>
#include <lib/egg/core/eggThread.h>
#include <lib/egg/core/eggDvdRipper.h>
#include <revolution/dvd.h>

namespace mDvd {

class CommandList_c {
public:
    CommandList_c();
    BOOL send();
    BOOL receive();
    mDvd_command_c *getFirst();
    void append(mDvd_command_c *command);
    void remove(mDvd_command_c *command);
    void main();
    void *run();

    /* 0x00 */ OSMessageQueue mQueue;
    /* 0x20 */ OSMessage mMessage;
    /* 0x24 */ OSMutex mMutex;
    /* 0x3C */ mDvd_command_c *mpFirst;
    /* 0x40 */ mDvd_command_c *mpLast;
}; // size 0x44

class MyThread_c : public EGG::Thread {
public:
    MyThread_c(int priority);
    virtual ~MyThread_c() {} // 802ADB4C
    virtual void *run() override;
};

class Command_c {
public:
    void *run();
    mDvd_command_c *mpCommand;
};

static CommandList_c l_commands;
static u8 l_defaultDirection = 1;
OSThread *g_dvdThread;
MyThread_c *g_thread;
EGG::Heap *g_commandHeap;
EGG::Heap *g_archiveHeap;

// 802AD3E8
void *CommandList_c::run() {
    main();
    return nullptr;
}

// 802AD40C
MyThread_c::MyThread_c(int priority) : EGG::Thread(0x4000, 0, priority, nullptr) {}

// 802AD458
void *MyThread_c::run() {
    return l_commands.run();
}

// 802AD464
void create(int priority, EGG::Heap *commandHeap, EGG::Heap *archiveHeap, const GXColor *perfColor) {
    g_commandHeap = commandHeap;
    g_archiveHeap = archiveHeap;
    g_thread = new MyThread_c(priority);
    OSThread *thread = g_thread->getOSThread();
    g_dvdThread = thread;
    if (mHeap::g_assertHeap != nullptr) {
        g_thread->setCurrentHeap(mHeap::g_assertHeap);
    }
    mPerf::setThreadColor(thread, perfColor);
    OSResumeThread(g_thread->getOSThread());
}

// 802AD4EC
s32 convertPathToEntrynum(const char *path) {
    return DVDConvertPathToEntrynum(path);
}

} // namespace mDvd

// 802AD4F0
mDvd_command_c::~mDvd_command_c() {}

// 802AD530
void *mDvd_command_c::operator new(size_t size) {
    return EGG::Heap::alloc(size, -4, mDvd::g_commandHeap);
}

// 802AD53C
void mDvd_command_c::operator delete(void *ptr) {
    EGG::Heap::free(ptr, mDvd::g_commandHeap);
}

namespace mDvd {

// 802AD544
CommandList_c::CommandList_c() {
    mMessage = nullptr;
    mpFirst = nullptr;
    mpLast = nullptr;
    OSInitMessageQueue(&mQueue, &mMessage, 1);
    OSInitMutex(&mMutex);
}

// 802AD594
BOOL CommandList_c::send() {
    return OSSendMessage(&mQueue, nullptr, 0);
}

// 802AD5A0
BOOL CommandList_c::receive() {
    return OSReceiveMessage(&mQueue, nullptr, OS_MESSAGE_BLOCK);
}

// 802AD5AC
mDvd_command_c *CommandList_c::getFirst() {
    return mpFirst;
}

// 802AD5B4
void CommandList_c::append(mDvd_command_c *command) {
    OSLockMutex(&mMutex);
    command->mpNext = nullptr;
    if (mpLast == nullptr) {
        mpFirst = mpLast = command;
    } else {
        mpLast->mpNext = command;
        mpLast = command;
    }
    OSUnlockMutex(&mMutex);
    send();
}

// 802AD628
void CommandList_c::remove(mDvd_command_c *command) {
    OSLockMutex(&mMutex);
    mDvd_command_c *prev = nullptr;
    for (mDvd_command_c *node = mpFirst; node != nullptr; node = node->mpNext) {
        if (node == command) {
            if (prev != nullptr) {
                prev->mpNext = node->mpNext;
                if (mpLast == node) {
                    mpLast = prev;
                }
            } else if (mpFirst == mpLast) {
                mpFirst = nullptr;
                mpLast = nullptr;
            } else {
                mpFirst = node->mpNext;
            }
        }
        prev = node;
    }
    OSUnlockMutex(&mMutex);
    send();
}

// 802AD6E4
void *Command_c::run() {
    mpCommand->execute();
    return nullptr;
}

// 802AD718
void CommandList_c::main() {
    while (receive()) {
        Command_c task;
        while ((task.mpCommand = getFirst()) != nullptr) {
            remove(task.mpCommand);
            task.run();
        }
    }
}

} // namespace mDvd

// 802AD780
mDvd_command_c::mDvd_command_c() : mpNext(nullptr), mDone(false) {}

// 802AD79C
void mDvd_command_c::destroy() {
    delete this;
}

// 802AD7BC
mDvd_callback_c::~mDvd_callback_c() {}

// 802AD814
mDvd_callback_c::mDvd_callback_c(Callback callback, void *param)
    : mDvd_command_c(), mpCallback(callback), mpParam(param), mpResult(nullptr) {}

// 802AD878
mDvd_callback_c *mDvd_callback_c::create(Callback callback, void *param) {
    mDvd_callback_c *command = new mDvd_callback_c(callback, param);
    if (command != nullptr) {
        mDvd::l_commands.append(command);
    }
    return command;
}

// 802AD8F4
BOOL mDvd_callback_c::execute() {
    mpResult = mpCallback(mpParam);
    mDone = true;
    return mpResult != nullptr;
}

// 802AD944
mDvd_toMainRam_c::mDvd_toMainRam_c(u8 allocDirection) : mDvd_command_c() {
    if (allocDirection == 0) {
        allocDirection = mDvd::l_defaultDirection;
    }
    mAllocDirection = allocDirection;
    mEntryNum = -1;
    mpData = nullptr;
    mSize = 0;
    mFileSize = 0;
    mpHeap = nullptr;
}

// 802AD9B8
mDvd_toMainRam_c *mDvd_toMainRam_c::create(const char *path, u8 allocDirection, EGG::Heap *heap) {
    s32 entryNum = mDvd::convertPathToEntrynum(path);
    mDvd_toMainRam_c *command;
    if (entryNum == -1) {
        command = nullptr;
    } else {
        command = new mDvd_toMainRam_c(allocDirection);
        if (command != nullptr) {
            command->mEntryNum = entryNum;
            command->mpHeap = heap;
            mDvd::l_commands.append(command);
        }
    }
    return command;
}

// 802ADA58
BOOL mDvd_toMainRam_c::execute() {
    u32 size;
    u32 fileSize;
    EGG::Heap *heap = mpHeap != nullptr ? mpHeap : mDvd::g_archiveHeap;
    EGG::DvdRipper::EAllocDirection direction = mAllocDirection == 1
        ? EGG::DvdRipper::ALLOC_DIRECTION_HEAD : EGG::DvdRipper::ALLOC_DIRECTION_TAIL;
    mpData = EGG::DvdRipper::loadToMainRAM(mEntryNum, nullptr, heap, direction, 0, &size, &fileSize);
    if (mpData != nullptr) {
        mSize = size;
        mFileSize = fileSize;
    }
    mDone = true;
    return mpData != nullptr;
}

// 802ADAF4
mDvd_toMainRam_c::~mDvd_toMainRam_c() {}
