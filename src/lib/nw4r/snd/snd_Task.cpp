#include <nw4r/snd/snd_Task.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
Task::~Task() {
    NW4RAssert_Line(24, !mBusyFlag);
}
} // namespace detail
} // namespace snd
} // namespace nw4r
