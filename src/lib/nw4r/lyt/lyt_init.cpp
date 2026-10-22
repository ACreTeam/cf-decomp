#include <nw4r/lyt.h>

#include <revolution/os.h>

namespace {

NW4R_LIB_VERSION(LYT, "May 15 2008", "05:33:36", "0x4199_60831"); // City Folk's build

} // namespace

namespace nw4r {
namespace lyt {

void LytInit() {
    OSRegisterVersion(NW4R_LYT_Version_);
    OSInitFastCast();
}

} // namespace lyt
} // namespace nw4r
