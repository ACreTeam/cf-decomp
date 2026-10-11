#pragma once

// d_shutter_fader (DOL TU d_shutter_fader.cpp, .text 801603A0..80160AE4): the shutter fader (dShutterFader_c, named by
// its RTTI): eight textured blades that close in from the screen edges. Its 0x80 x 0x80 IA8 blade texture comes from a
// .brres (loadTexture, d_s_boot_static). Member names are inferred. The helpers driving its instance are in
// d_side_wipe_fader.cpp.

#include <types.h>
#include <game/mLib/m_wipe_fader.hpp>

class dShutterFader_c : public mWipeFader_c {
public:
    dShutterFader_c();                // 801603A0
    virtual ~dShutterFader_c() {}     // 80160AA4
    virtual void draw();              // 80160488
    virtual void calcMtx();           // 80160480: the identity

    static void loadTexture(void *brres); // 801603EC: texture 0 of the .brres into sTexture
    void init();                          // 80160460: setTexture(sTexture, 0x80, 0x80)
}; // size 0x58
