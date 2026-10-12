#pragma once

// dMsg::Voice_c (TU d_msg.cpp): the speaker's voice and mouth while the text is typed (dMsg::Comp_c +0x55F8).
// Name from RTTI (vtable 804FB6A0). Its own header, included by d_msg_word.hpp before dMsg::Word_c: the
// TU's vtables come out in reverse class-definition order, and Voice_c's follows Word_c's.
#include <types.h>

namespace dMsg {
class Comp_c;
}

namespace dMsg {

// Voice settings (Comp_c +0x55F8). (inline ctor/dtor: the controller ctor/dtor inline them)
class Voice_c {
public:
    virtual ~Voice_c(); // 8019FE2C (inline, body in d_msg.cpp)

    // Used by the controller.
    void create(Comp_c *comp); // 801A60B0: init (controller create): sets mpOwner
    void setChar(wchar_t c); // 801A60B8: the typed character (Comp_c::setVoiceChar)
    void reset(int arg); // 801A60D0: reset (arg unused)
    void calc(); // 801A60F8: calc: voices mPrevChar, opens/closes the speaker's mouth

    /* 0x04 */ wchar_t mPrevChar; // the character voiced by calc
    /* 0x06 */ wchar_t mChar;     // the last typed character
    /* 0x08 */ int mColor;          // text colour (system tag 0xFF:0, Seq_c); 4 (grey) with window type 0xD: no voice, mouth closed
    /* 0x0C */ Comp_c *mpOwner;
    /* 0x10 */ bool mMsgNoWait; // the message's speed is no-wait (speed == 1, Comp_c::initializePrepare; bool: srwi store)
    /* 0x11 */ u8 mTagNoWait; // Seq_c tags 6:2 / 6:3 (no-wait mode); cleared by Seq_c::start
    /* 0x12 */ u8 mPaused; // paused by the wait tag 6:0 (mouth closed); cleared by each typed character
    /* 0x13 */ u8 mCharSet;    // setChar: mPrevChar not voiced yet
    /* 0x14 */ f32 mScale;         // text scale (system tag 0xFF:1 / 100, Seq_c), passed to the voice (sndSpeakVoice); reset to 1.0f
}; // size 0x18

} // namespace dMsg
