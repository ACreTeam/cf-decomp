#pragma once

// d_cd (DOL TU d_cd.cpp, .text 8018D778..8018DAC8): dCd_c (named by its RTTI "dCd_c"), the New Year's Eve
// countdown process (profile CD, execute order 0xB, draw order 4). On Dec 31 it plays a tick sound effect as
// each second's digit changes during the last minute before midnight, and a chime at 00:00:00 on Jan 1.
// "Cd" = countdown and all member names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>

namespace dScene { class dSceneChange_c; }

class dCd_c : public fBase_c {
public:
    virtual ~dCd_c();     // 8018DA60
    virtual int create(); // 8018D7C4
    virtual int execute(); // 8018D81C

    static dScene::dSceneChange_c *getSceneChange(); // 8018DABC: &dScene::gSceneChange (d_demo)

    u8 mPrevDigit;   // 0x64: last second digit seen
    u8 mDigit;       // 0x65: current second digit (0..9)
    u8 mMuted;       // 0x66: set on create, so the first digit change after creation is silent
    u8 mNewYearDone; // 0x67: the 00:00:00 Jan 1 sound effect was played
    u8 mEnabled;     // 0x68: the current scene allows the countdown (l_sceneEnabled)
}; // size 0x6C
