#ifndef NW4R_SND_TASK_MANAGER_H
#define NW4R_SND_TASK_MANAGER_H

#include <nw4r/snd/snd_Task.h>

#include <revolution/os.h>

namespace nw4r {
namespace snd {
namespace detail {
class TaskManager {
  public:
    enum TaskPriority {
        PRIORITY_LOW,
        PRIORITY_MIDDLE,
        PRIORITY_HIGH,
    };

  public:
    static TaskManager &GetInstance();

    void AppendTask(Task *task, TaskPriority priority = PRIORITY_MIDDLE);

    Task *ExecuteTask();
    void CancelTask(Task *task);
    void CancelAllTask();

    void WaitTask();
    void CancelWaitTask();

  private:
    TaskManager();

    Task *GetNextTask();
    Task *GetNextTask(TaskPriority priority, bool doRemove);

    Task *PopTask();

  public:
    static int const PRIORITY_NUM = 3;

  private:
    Task::LinkList mTaskList[PRIORITY_NUM]; // size 0x24, offset 0x00
    Task *volatile mCurrentTask;            // size 0x04, offset 0x24
    bool mCancelWaitTaskFlag;
    /* 3 bytes padding */
    OSThreadQueue mAppendThreadQueue; // size 0x08, offset 0x2c
    OSThreadQueue mDoneThreadQueue;   // size 0x08, offset 0x34
}; // size 0x3c
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
