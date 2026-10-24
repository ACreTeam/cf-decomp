#ifndef NW4R_SND_FX_BASE_H
#define NW4R_SND_FX_BASE_H

#include <types.h>

#include <nw4r/snd/snd_global.h>

#include <nw4r/ut/ut_NonCopyable.h>
#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
class FxBase : private ut::NonCopyable {
  public:
    typedef ut::LinkList<FxBase, 0x04> LinkList;

  public:
    virtual ~FxBase() {
    }

    virtual bool StartUp();
    virtual void Shutdown();
    virtual void UpdateBuffer(int, void **, u32, SampleFormat, f32, OutputMode);
    virtual void OnChangeOutputMode();

  private:
    /* base ut::NonCopyable */ // size 0x00, offset 0x00
    /* vtable */               // size 0x04, offset 0x00
  public:
    ut::LinkListNode mFxLink; // size 0x08, offset 0x04
}; // size 0x0c
} // namespace snd
} // namespace nw4r

#endif
