#pragma once

// d_shutdown_fader (DOL TU d_shutdown_fader.cpp, .text 8015FDF0..801603A0): a full-screen color fader
// (dShutdownFader_c, named by its RTTI) that fades out at once and fades back in, plus the functions driving its one
// instance (namespace dShutdownFader; names inferred).

#include <types.h>
#include <game/mLib/m_fader_base.hpp>

class dShutdownFader_c : public mFaderBase_c {
public:
    dShutdownFader_c(mColor col, mFaderBase_c::EStatus status); // 8015FDF0
    virtual ~dShutdownFader_c();                                 // 801601BC
    virtual void setStatus(EStatus status);                     // 8015FE74
    virtual int calc();                                         // 8015FEAC: FADE_OUT is opaque at once
    virtual void draw();                                        // 8015FF48

    /* 0x14 */ f32 mWidthScale; // 16:9 / 4:3 on a wide screen, else 1
}; // size 0x18

namespace dShutdownFader {

BOOL startFadeOut(u16 outFrame, u16 inFrame); // 801601FC: when hidden; it fades back in over inFrame
BOOL isHidden();                               // 80160268
void calc();                                   // 801602A4: fades back in once opaque
void draw();                                   // 80160310
BOOL create();                                 // 80160348 (d_sys)

} // namespace dShutdownFader
