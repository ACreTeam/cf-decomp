#ifndef NW4R_SND_TASK_H
#define NW4R_SND_TASK_H

#include <nw4r/ut/ut_algorithm.h>
#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
namespace detail {
class Task : private ut::NonCopyable {
  public:
    typedef ut::LinkList<Task, 0x04> LinkList;

  public:
    Task() : mBusyFlag(false) {
    }
    virtual ~Task();

    virtual void Execute() = 0;
    virtual void Cancel() = 0;
    virtual void OnCancel() = 0;

  private:
    /* base NonCopyable */      // size 0x00, offset 0x00
    /* vtable */                // size 0x04, offset 0x00
    ut::LinkListNode mTaskLink; // size 0x08, offset 0x04
    bool volatile mBusyFlag;    // size 0x01, offset 0x0c
                                /* 3 bytes padding */

  private:
    friend class TaskManager;
}; // size 0x10
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
