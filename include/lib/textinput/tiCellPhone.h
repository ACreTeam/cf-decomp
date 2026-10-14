#pragma once

// textinput::keyboard::cellphonetype: the cell phone keypad (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>

namespace textinput {
namespace keyboard {
namespace cellphonetype {

// A key of the keypad: the characters it cycles through.
struct KeyData {
    /* 0x00 */ u8 _00[0x14];
    /* 0x14 */ wchar_t mChars[1];
};

// The character being picked by pressing a key repeatedly.
struct Candidate {
    /* 0x00 */ int mIndex;
    /* 0x04 */ int _04;
    /* 0x08 */ u8 _08[0x10 - 0x08];
    /* 0x10 */ KeyData *mpKey;
}; // size 0x14

// The command a key sends (CommandReceiver::INPUT_COMMAND_KEY).
struct KeyCommand {
    /* 0x00 */ wchar_t mChar;
    /* 0x04 */ u32 mFlags;
    /* 0x08 */ KeyData *mpKey;
};

class Base : public KeyboardBase {
public:
    virtual void vt_0x38();
    virtual void draw();                       // 0x3C
    virtual int vt_0x40() { return mCandidate._04; } // 0x40
    virtual void vt_0x44();
    virtual void vt_0x48();
    virtual void vt_0x4C();
    virtual void vt_0x50();
    virtual void vt_0x54();
    virtual void vt_0x58();
    virtual void vt_0x5C();
    virtual void vt_0x60();
    virtual void vt_0x64();
    virtual void refreshState();               // 0x68
    virtual void vt_0x6C();
    virtual void vt_0x70();
    virtual void vt_0x74();
    virtual void vt_0x78();
    virtual void vt_0x7C();
    virtual void vt_0x80();
    virtual void vt_0x84();

    /* 0x14 */ Candidate mCandidate;
    /* 0x28 */ u8 _28[0x34 - 0x28];
}; // size 0x34

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
public:
    virtual ~LayoutByNW4R();
    virtual void draw();
    virtual void vt_0xFC();
    virtual void vt_0x100();
    virtual void vt_0x104();
    virtual void vt_0x108(int);
    virtual void vt_0x10C(bool);
};

} // namespace cellphonetype
} // namespace keyboard
} // namespace textinput
