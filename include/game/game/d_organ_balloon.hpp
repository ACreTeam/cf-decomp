#ifndef D_ORGAN_BALLOON_HPP
#define D_ORGAN_BALLOON_HPP

#include <game/framework/f_base.hpp>

// The balloons of the day (REL d_organ_balloonNP, profile ORGAN_BALLOON, heap
// "dOrganBalloon::m_frmHeap"): three balloon strings and a color picked at
// random at create. Only what other RELs use is declared; the REL itself is
// not decompiled yet. Class name from RTTI ("dOrganBalloon_c").
class dOrganBalloon_c : public fBase_c {
  public:
    /* 0x064 */ u8 _064[0x154 - 0x64];
    /* 0x154 */ int mColor; // the balloon color (anmMatClr frame), random

    static dOrganBalloon_c *s_instance; // .bss 0x8, set at create
}; // size 0x158

#endif
