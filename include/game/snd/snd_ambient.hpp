#pragma once

// dol/sound/SoundAmbient.cpp (801E2E58..801E39B0): the rain and sea ambience, SoundSystem::mAmbient. No RTTI;
// the class name is the TU's.

#include <types.h>
#include <lib/nw4r/snd.h>

class SoundAmbient {
public:
    SoundAmbient() : mWeather(0), mSeaDist(1000.0f) {}

    void calc();          // 801E2E58: per frame (SoundSystem): the rain, then calcSea
    void calcSea();       // 801E3380: the sea, by mSeaDist
    u32 getSeaSoundId();  // 801E3914: by the weather
    f32 getSeaVolume();   // 801E393C: by mSeaDist (0.1..1)

    // Set by SoundManager when a stage's sounds are set up, cleared on scene changes and resets.
    static bool sIsStageSet;

    /* 0x00 */ nw4r::snd::SoundHandle mRainHandles[2]; // light, heavy
    /* 0x08 */ nw4r::snd::SoundHandle mSeaHandle;
    /* 0x0C */ int mWeather;  // dWeather_c::_5884 (3/4: rain)
    /* 0x10 */ f32 mSeaDist;  // the player's distance to the sea (SoundSystem::setSeaDist)
}; // size 0x14
