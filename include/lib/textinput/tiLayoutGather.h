#pragma once

#include <types.h>

namespace textinput {

class LayoutGather {
public:
    LayoutGather();
    ~LayoutGather();

    void changeCapsLock(const u8 &capsLock);

    bool isHoldingShift() const { return isHWPressedShift() || isPressedShiftB(); }
    u8 isHWPressedShift() const { return mbPressedShift; }
    u8 isPressedShiftB() const { return mbPressedShiftB; }
    void setHWPressedShift(const u8 &shift) { mbPressedShift = shift; }

    class Singleton {
    public:
        static LayoutGather &getInstance() {
            static LayoutGather sGather;
            return sGather;
        }
    };

private:
    /* 0x0 */ u8 mbCapsLock : 1;
    /* 0x0 */ u8 mbPressedShift : 1;
    /* 0x0 */ u8 mbPressedShiftB : 1;
}; // size 0x1

} // namespace textinput
