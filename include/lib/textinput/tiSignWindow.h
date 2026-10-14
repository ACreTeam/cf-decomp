#pragma once

// textinput::keyboard::signtype: the symbol window (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>

namespace textinput {
namespace keyboard {
namespace signtype {

class Base : public KeyboardBase {
public:
    virtual void vt_0x38();
    virtual void vt_0x3C();
    virtual void vt_0x40();
    virtual void vt_0x44();
    virtual void vt_0x48();
    virtual void close(); // 0x4C

protected:
    /* 0x14 */ u8 _14[0x24 - 0x14];
}; // size 0x24

class LayoutByNW4R : public Base, public nw4rmanager::Layout, public nw4rmanager::AnmObserver {
public:
    virtual ~LayoutByNW4R();
    virtual void draw() { nw4rmanager::Layout::draw(); } // 0xD0
    virtual void vt_0xD4();
    virtual bool isActive() { return mbActive; }         // 0xD8

protected:
    /* 0xD0 */ bool mbActive;
};

} // namespace signtype
} // namespace keyboard
} // namespace textinput
