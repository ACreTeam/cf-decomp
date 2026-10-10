#pragma once

// dol/sound/SoundVoice.cpp (801D8618..801DC544, not decompiled). No RTTI; the class name is the
// TU's. Only what is used so far; methods keep their fn_ names.

#include <types.h>
#include <lib/nw4r/snd.h>

class SoundVoice {
public:
    SoundVoice();  // 801D8618: picks the voice language from getLanguage()
    ~SoundVoice(); // 801D86EC

    /* 0x00 */ nw4r::snd::SoundHandle mHandle;
    /* 0x04 */ u8 _04[0x24 - 0x04];
}; // size 0x24
