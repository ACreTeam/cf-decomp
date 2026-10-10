#pragma once

// The player head models (DOL TU d_hmn_head_mng.cpp, .text 800B8EE0..800B953C).
// "/Plyr/Head/Head.arc" holds the head models ("<no + kind * 27>.brres", 27 heads in 2 kinds);
// each character's head model is copied into one of four double-buffered slots of a file-local
// object in d_hmn_head_mng.cpp. Names other than the class names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>

// One character's head (1 byte; the player keeps one at +0xC28): an index into the buffers.
class dHmnHeadMng_c {
public:
    // "/Plyr/Head/Head.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800B94DC (weak)
        virtual void onLoaded(); // 800B9134

        void *getHead(u32 *size, int no, int kind); // 800B9170: "<no + kind * 27>.brres" (no < 27, kind < 2)
    };

    dHmnHeadMng_c();  // 800B92FC: mBufIdx = 4 (none)
    ~dHmnHeadMng_c(); // 800B9308
    void setBufIdx(u8 idx);           // 800B9348
    BOOL setHead(int no, int kind);   // 800B9350: copies the head into the other half of the buffer, then binds it
    void *getData() const;            // 800B93FC: the buffer's current data (NULL without a buffer)
    void bind();                      // 800B943C

    static u32 getWorkSize(); // 800B9484
    static BOOL load();       // 800B9488: loads the archive and allocates the buffers

    /* 0x0 */ u8 mBufIdx;
}; // size 0x1
