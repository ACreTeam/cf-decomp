#pragma once

// textinput::keyboard::pctype: the PC (QWERTY) keyboard (see
// tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>
#include <lib/textinput/tiString.h>

namespace textinput {
namespace keyboard {
namespace pctype {

// The modifier / input mode state of the keyboard.
class KeyState {
  public:
    void setInputMode(u32 mode); // 802E6F88: no-op if unchanged

    /* 0x00 */ u32 _00 : 25;
    /* 0x00 */ u32 mCapsOn : 1;
    /* 0x00 */ u32 _1A : 2;
    /* 0x00 */ u32 mShift : 4; // bit 0: shift, bit 1: caps
    /* 0x04 */ int mInputType;
    /* 0x08 */ u32 mInputMode : 28;
    /* 0x08 */ u32 _08 : 4;
};

class Base : public KeyboardBase {
  public:
    enum TranslateMode {
        TM_Direct,
        TM_Hiragana,
        TM_Katakana,
    };

    virtual void inputCharCode(); // 0x38
    virtual void vt_0x3C();
    virtual void vt_0x40();
    virtual bool isCapsOn() {
        return mKeyState.mCapsOn;
    } // 0x44
    virtual bool isABC() {
        return mKeyState.mInputType == 0;
    }                              // 0x48
    virtual void setABC(bool abc); // 0x4C
    virtual void vt_0x50();
    virtual void vt_0x54();
    virtual TranslateMode getTranslateMode() const; // 0x58
    virtual void setTranslateMode(TranslateMode mode); // 0x5C
    virtual void vt_0x60();
    virtual void vt_0x64();
    virtual void vt_0x68();
    virtual void vt_0x6C();
    virtual void vt_0x70();
    virtual void refreshState(); // 0x74
    virtual void vt_0x78();
    virtual void vt_0x7C();
    virtual void vt_0x80();
    virtual void vt_0x84();
    virtual void vt_0x88();
    virtual void vt_0x8C();

    /* 0x14 */ u8 _14[0x20 - 0x14];
    /* 0x20 */ KeyState mKeyState;
    /* 0x2C */ u8 _2C[0x3C - 0x2C];
    /* 0x3C */ bool mbKanaLock;
    /* 0x3D */ u8 _3D[0x44 - 0x3D];
}; // size 0x44

class LayoutByNW4R : public Base,
                     public nw4rmanager::Layout,
                     public nw4rmanager::AnmObserver {
  public:
    virtual ~LayoutByNW4R();
    virtual void draw(); // 0x110
    virtual void vt_0x114();
    virtual void vt_0x118();
    virtual void vt_0x11C();
    virtual void vt_0x120(int);
    virtual void vt_0x124(bool);
    virtual void vt_0x128();
    virtual void vt_0x12C();
    virtual void vt_0x130();
    virtual void vt_0x134();
    virtual void vt_0x138();
    virtual void vt_0x13C();
    virtual void vt_0x140();
    virtual void vt_0x144();

    /* 0xF0 */ u8 _F0[0xFC - 0xF0];
    bool isLineFeedEnabled() const {
        return mbEnableLineFeed;
    }
    /* 0xFC */ bool mbEnableLineFeed;
};

} // namespace pctype
} // namespace keyboard
} // namespace textinput
