#include <lib/textinput/tiLayoutGather.h>
#include <lib/textinput/tiHKBManager.h>

namespace textinput {

LayoutGather::LayoutGather() {
    mbCapsLock = 0;
    mbPressedShift = 0;
    mbPressedShiftB = 0;
}

LayoutGather::~LayoutGather() {
}

void LayoutGather::changeCapsLock(const u8 &capsLock) {
    mbCapsLock = capsLock;
    if (mbCapsLock) {
        input::HKBManager::sInstance.SetModifierState(0x200, 0x200);
    } else {
        input::HKBManager::sInstance.SetModifierState(0x0, 0x200);
    }
}

} // namespace textinput
