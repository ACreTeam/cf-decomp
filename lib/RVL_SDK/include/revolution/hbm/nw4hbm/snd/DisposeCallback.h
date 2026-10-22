#ifndef NW4HBM_SND_DISPOSE_CALLBACK_H
#define NW4HBM_SND_DISPOSE_CALLBACK_H

#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include <stddef.h>

namespace nw4hbm {
namespace snd {
namespace detail {

class DisposeCallback {
public:
    /* 0x00 */ ut::LinkListNode mDisposeLink;

    /* 0x08 */ virtual ~DisposeCallback() {};

    /* 0x0C */ virtual void InvalidateData(const void* start, const void* end) = 0;
    /* 0x10 */ virtual void InvalidateWaveData(const void* start, const void* end) = 0;
};

typedef ut::LinkList<DisposeCallback, offsetof(DisposeCallback, mDisposeLink)> DisposeCallbackList;

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
