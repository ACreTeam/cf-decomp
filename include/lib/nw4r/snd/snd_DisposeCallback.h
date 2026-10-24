#ifndef NW4R_SND_DISPOSE_CALLBACK_H
#define NW4R_SND_DISPOSE_CALLBACK_H

#include <types.h>

#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
namespace detail {
class DisposeCallback {
  public:
    typedef ut::LinkList<DisposeCallback, 0x00> LinkList;

  public:
  public:
    ut::LinkListNode mDisposeLink; // size 0x08, offset 0x00
    /* vtable */                   // size 0x04, offset 0x08

  public:
    virtual ~DisposeCallback() {
    }
    virtual void InvalidateData(void const *pStart, void const *pEnd) = 0;
    virtual void InvalidateWaveData(void const *pStart, void const *pEnd) = 0;
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
