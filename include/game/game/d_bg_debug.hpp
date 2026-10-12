#pragma once

// d_bg_debug (DOL TU d_bg_debug.cpp, .text 80186E0C..801870D0): the BG debug process dBgDebug_c and its HostIO
// parameters dBGO_bgHostIO_c (both named by their RTTI, global scope). The process only forwards its
// create/delete steps to d_hr and d_shadow (and the city's auction save) in this build. Member names inferred.

#include <types.h>
#include <game/framework/f_base.hpp>

class dBgDebug_c : public fBase_c {
public:
    dBgDebug_c() {}
    virtual ~dBgDebug_c();  // 80186F60
    virtual int create();   // 80186E58
    virtual int doDelete(); // 80186EC4
    virtual int execute();  // 80186E98
    virtual int draw();     // 80186EBC

    // 80186F18: the debug override of the season footstep sound (dGroundAttr::getFootstepSe): 0 grass,
    // 1 leaves, 2 snow, anything else (always -1 here) uses the season.
    static s16 getGroundSeType();

    /* 0x64 */ u8 _64[0x6C - 0x64];
}; // size 0x6C

// The HostIO parameters (80623F90, set up by __sinit; nothing reads them in this build, field names unknown).
class dBGO_bgHostIO_c {
public:
    dBGO_bgHostIO_c() {
        _04 = 1;
        _06 = 1;
        _1D = 0;
        _1E = 0;
        _0D = 0;
        _0E = 0;
        _0F = 0;
        _10 = 0;
        _11 = 0;
        _12 = 0;
        _13 = 0;
        _14 = 0;
        _15 = 0;
        _16 = 0;
        _17 = 0;
        _18 = 0;
        _19 = 0;
        _1A = 0;
        _1C = 0;
        _1B = 0;
        _24 = 0;
        _25 = 0;
        _26 = 0;
        _27 = 0;
        _28 = 0;
        _20 = 8;
        _22 = 0x80;
        _2A = -1;
        _2C = -1;
        _0A = 0;
        _0B = 0;
        _09 = 0;
        _08 = 0;
        _0C = 1;
    }
    virtual ~dBGO_bgHostIO_c(); // 80186F20

    /* 0x04 */ s16 _04;
    /* 0x06 */ s16 _06;
    /* 0x08 */ u8 _08;
    /* 0x09 */ u8 _09;
    /* 0x0A */ u8 _0A;
    /* 0x0B */ u8 _0B;
    /* 0x0C */ u8 _0C;
    /* 0x0D */ u8 _0D;
    /* 0x0E */ u8 _0E;
    /* 0x0F */ u8 _0F;
    /* 0x10 */ u8 _10;
    /* 0x11 */ u8 _11;
    /* 0x12 */ u8 _12;
    /* 0x13 */ u8 _13;
    /* 0x14 */ u8 _14;
    /* 0x15 */ u8 _15;
    /* 0x16 */ u8 _16;
    /* 0x17 */ u8 _17;
    /* 0x18 */ u8 _18;
    /* 0x19 */ u8 _19;
    /* 0x1A */ u8 _1A;
    /* 0x1B */ u8 _1B;
    /* 0x1C */ u8 _1C;
    /* 0x1D */ u8 _1D;
    /* 0x1E */ u8 _1E;
    /* 0x20 */ s16 _20;
    /* 0x22 */ s16 _22;
    /* 0x24 */ u8 _24;
    /* 0x25 */ u8 _25;
    /* 0x26 */ u8 _26;
    /* 0x27 */ u8 _27;
    /* 0x28 */ u8 _28;
    /* 0x2A */ s16 _2A;
    /* 0x2C */ s16 _2C;
}; // size 0x30
