#pragma once

// textinput::candidatebox: the conversion candidate box (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>

namespace textinput {
namespace candidatebox {

class LayoutByNW4R;

// The side that opens the candidate box (the input form).
class CandidateBoxCaller {
public:
    virtual void vt_0x08();
    virtual void vt_0x0C();
    virtual void setCandidateBox(LayoutByNW4R *candidateBox); // 0x10
};

class Base : public CommandSender {
public:
    virtual void vt_0x20();
    virtual void draw();     // 0x24
    virtual void turnOn();   // 0x28
    virtual void turnOff();  // 0x2C
    virtual void makeEmpty(); // 0x30
    virtual void vt_0x34();
    virtual void vt_0x38();
    virtual void vt_0x3C();
    virtual void vt_0x40();
    virtual void vt_0x44();
    virtual void vt_0x48();

    /* 0x10 */ u8 _10[0x17F4 - 0x10];
}; // size 0x17F4

class LayoutByNW4R : public Base, public nw4rmanager::Layout, public nw4rmanager::AnmObserver {
public:
    virtual ~LayoutByNW4R();
    virtual void draw();
    virtual void vt_0xCC();
    virtual void vt_0xD0();
    virtual void onLeftScroll();          // 0xD4
    virtual void vt_0xD8();
    virtual void vt_0xDC();
    virtual void vt_0xE0();
    virtual void changeSelectEmpty_compatible(); // 0xE4
    virtual void changeSelect();          // 0xE8
    virtual void vt_0xEC();
    virtual void vt_0xF0();
    virtual void vt_0xF4();
    virtual void vt_0xF8(bool flag) { mFlag1B88 = flag; } // 0xF8

    /* 0x18A0 */ u8 _18A0[0x1B88 - 0x18A0];
    /* 0x1B88 */ bool mFlag1B88;
};

} // namespace candidatebox
} // namespace textinput
