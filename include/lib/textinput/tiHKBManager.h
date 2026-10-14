#pragma once

// textinput::input: the USB keyboard input (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
namespace input {

// A key event of the USB keyboard.
struct KeyEvent {
    /* 0x00 */ u8 _00[0x04];
    /* 0x04 */ u32 mMode; // bit 0: pressed
    /* 0x08 */ u8 _08[0x0C - 0x08];
    /* 0x0C */ wchar_t mCode;
};

// Gets the key events of the USB keyboard; listeners are chained after HKBManager's root listener.
class KBDListener {
public:
    KBDListener() : mpPrev(NULL), mpNext(NULL) {}
    virtual ~KBDListener() {}                       // 0x08
    virtual void vt_0x0C() {}                       // 0x0C
    virtual void vt_0x10() {}                       // 0x10
    virtual void onKey(KeyEvent *event) {}    // 0x14

    void append(KBDListener *listener);             // 80315ADC: adds listener at the end of the chain
    void remove(KBDListener *listener);             // 80315AFC

    /* 0x04 */ KBDListener *mpPrev;
    /* 0x08 */ KBDListener *mpNext;
}; // size 0xC

class HKBManager {
public:
    void Initialize(); // 80315FF0
    void Update();     // 80316324

    /* 0x00 */ u8 _00[0xEC];
    /* 0xEC */ KBDListener mListenerRoot;
    /* 0xF8 */ u8 _F8[0xFC - 0xF8];

    static HKBManager sInstance;
}; // size 0xFC

} // namespace input
} // namespace textinput
