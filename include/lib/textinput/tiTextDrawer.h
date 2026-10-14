#pragma once

// textinput::textdrawer: draws the edited text (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <nw4r/ut.h>
#include <nw4r/math.h>

namespace textinput {
namespace textdrawer {

class Base : public nw4r::ut::CharWriter {
public:
    virtual ~Base();                                     // 0x08
    virtual void create(MEMAllocator *allocator);        // 0x0C
    virtual void vt_0x10();
    virtual void vt_0x14();
    virtual void vt_0x18();
    virtual void vt_0x1C();
    virtual void vt_0x20();
    virtual void vt_0x24();
    virtual void vt_0x28();
    virtual void vt_0x2C();
    virtual void vt_0x30();
    virtual void vt_0x34();
    virtual nw4r::math::VEC2 getScale() const = 0;       // 0x38
    virtual void setSecretModeOn(bool secretMode) { mbSecretMode = secretMode; } // 0x3C
    virtual void vt_0x40();
    virtual void vt_0x44();
    virtual void vt_0x48();
    virtual void vt_0x4C();
    virtual void vt_0x50();
    virtual void vt_0x54();
    virtual void vt_0x58();
    virtual void vt_0x5C();
    virtual void vt_0x60();
    virtual void vt_0x64();
    virtual void vt_0x68();
    virtual void vt_0x6C();
    virtual void vt_0x70();
    virtual void vt_0x74();
    virtual void vt_0x78();
    virtual void vt_0x7C();
    virtual void vt_0x80();
    virtual void vt_0x84();
    virtual void vt_0x88();
    virtual void vt_0x8C();
    virtual void vt_0x90();
    virtual void vt_0x94();

    /* 0x50 */ u8 _50[0xC8 - 0x50];
    /* 0xC8 */ bool mbSecretMode;
}; // size 0xCC (as far as the input form needs)

} // namespace textdrawer
} // namespace textinput
