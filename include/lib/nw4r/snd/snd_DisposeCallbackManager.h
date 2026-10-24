#ifndef NW4R_SND_DISPOSE_CALLBACK_MANAGER_H
#define NW4R_SND_DISPOSE_CALLBACK_MANAGER_H

#include <types.h>

#include <nw4r/ut/ut_LinkList.h>

#include <nw4r/snd/snd_DisposeCallback.h>

namespace nw4r {
namespace snd {
namespace detail {
class DisposeCallbackManager {
  public:
    static DisposeCallbackManager &GetInstance();

    void RegisterDisposeCallback(DisposeCallback *callback);
    void UnregisterDisposeCallback(DisposeCallback *callback);

    void Dispose(void *mem, u32 size, void *arg);
    void DisposeWave(void *mem, u32 size, void *arg);

  private:
    DisposeCallbackManager();

  private:
    DisposeCallback::LinkList mCallbackList; // size 0x0c, offset 0x00
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
