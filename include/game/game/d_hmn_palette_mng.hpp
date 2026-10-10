#pragma once

// The per-character palette manager (DOL TU d_hmn_palette_mng.cpp, .text 800B95E8..800B9BDC).
// The palettes come from "/Plyr/Pltt/Palette.arc" (one "<id>.brplt" per palette, 0x5C of them);
// each palette type has three buffers the selected palettes are copied into. Used by d_a_player
// (its instance at +0xC29).
// The archive and the buffers live in a file-local object in d_hmn_palette_mng.cpp.
// Names other than the class names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <nw4r/g3d.h>

class dHmnPaletteMng_c {
public:
    // "/Plyr/Pltt/Palette.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800B9B7C
        virtual void onLoaded(); // 800B97F0

        void *getPltt(u32 *size, int plttId); // 800B982C: "<plttId>.brplt" in the archive
    };

    dHmnPaletteMng_c();  // 800B99B0: mType = 4 (none)
    ~dHmnPaletteMng_c(); // 800B99BC
    void setType(int type);                     // 800B99FC
    BOOL setPltt(int plttId, int idx);          // 800B9A04: copies palette plttId into buffer idx
    nw4r::g3d::ResFile getResFile(int idx) const; // 800B9AA8: buffer idx (null for type 4)
    void initResFile(int idx) const;            // 800B9AF0

    static u32 getWorkSize(); // 800B9B24
    static BOOL load();       // 800B9B28

    /* 0x0 */ u8 mType;
}; // size 0x1
