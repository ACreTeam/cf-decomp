#ifndef NW4R_SND_TASK_THREAD_H
#define NW4R_SND_TASK_THREAD_H

#include <types.h>

#include <revolution/os.h>

namespace nw4r {
namespace snd {
namespace detail {
class TaskThread {
  public:
    TaskThread();
    ~TaskThread();

    bool Create(s32 priority, void *stack, u32 stackSize);
    void Destroy();

  private:
    static void *ThreadFunc(void *arg);
    void ThreadProc();

  private:
    OSThread mThread; // size 0x318, offset 0x000
    u32 *mStackEnd;   // size 0x004, offset 0x318
    bool mFinishFlag;
    bool mCreateFlag; // size 0x001, offset 0x31d
                      /* 2 bytes padding */
}; // size 0x320
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
