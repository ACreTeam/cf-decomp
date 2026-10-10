#pragma once

// The player body models (DOL TU d_hmn_body_mng.cpp, .text 800B6750..800B6CF0).
// "/Plyr/Body/Body.arc" holds the body models ("<no>.brres", 2 of them); each character's body
// model is copied into one of four buffers of a file-local object in d_hmn_body_mng.cpp.
// Names other than the class names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>

// One character's body (1 byte; the player keeps one at +0xC2A): an index into the four body buffers.
class dHmnBodyMng_c {
public:
    // "/Plyr/Body/Body.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800B6C90 (weak)
        virtual void onLoaded(); // 800B6968

        void *getBody(u32 *size, int no); // 800B69A4: "<no>.brres" in the archive (no < 2)
    };

    dHmnBodyMng_c();  // 800B6AD4: mBufIdx = 4 (none)
    ~dHmnBodyMng_c(); // 800B6AE0
    void setBufIdx(u8 idx); // 800B6B20
    BOOL setBody(int no);   // 800B6B28: copies body file no into the buffer, then binds it
    void *getData() const;  // 800B6BB8: the buffer's data (NULL without a buffer)
    void bind();            // 800B6BF0

    static u32 getWorkSize(); // 800B6C38
    static BOOL load();       // 800B6C3C: loads the archive and allocates the buffers

    /* 0x0 */ u8 mBufIdx;
}; // size 0x1
