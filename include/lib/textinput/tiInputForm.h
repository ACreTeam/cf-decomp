#pragma once

// textinput::inputform: the text field (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiTextDrawer.h>
#include <lib/textinput/tiCandidateBox.h>
#include <lib/textinput/tiStringWithAtok.h>

namespace textinput {
namespace inputform {

class Base : public CommandReceiver, public textdrawer::Base, public candidatebox::CandidateBoxCaller {
public:
    virtual ~Base();
    virtual void vt_0xCC();
    virtual void vt_0xD0();
    virtual void vt_0xD4();
    virtual void vt_0xD8();
    virtual string::WithAtok *getAtokString() { return mpAtokString; } // 0xDC
    virtual void vt_0xE0();
    virtual void vt_0xE4();
    virtual void vt_0xE8();
    virtual void vt_0xEC();
    virtual void vt_0xF0();
    virtual void vt_0xF4();
    virtual void vt_0xF8();
    virtual void vt_0xFC();
    virtual void vt_0x100();
    virtual void vt_0x104();
    virtual void vt_0x108();
    virtual void vt_0x10C();
    virtual void vt_0x110();
    virtual void vt_0x114();
    virtual void vt_0x118();
    virtual void vt_0x11C();
    virtual void vt_0x120();
    virtual void vt_0x124();
    virtual void vt_0x128();
    virtual void vt_0x12C();
    virtual void vt_0x130();
    virtual void vt_0x134();
    virtual void vt_0x138();
    virtual void draw(); // 0x13C

    void setCursorPos(string::Decolated *string, u32 pos); // 802EFC24

    /* 0x0E0 */ u8 _0E0[0x193C - 0x0E0];
    /* 0x193C */ string::Decolated *mpString;
    /* 0x1940 */ string::WithAtok *mpAtokString;
    /* 0x1944 */ u8 _1944[0x19F0 - 0x1944];
}; // size 0x19F0

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
public:
    virtual ~LayoutByNW4R();
    virtual void draw();
};

} // namespace inputform
} // namespace textinput
