#pragma once

// The player face textures (DOL TU d_hmn_face_tex_mng.cpp, .text 800B7EF8..800B81F0).
// "/Plyr/FcTx/FaceTex.arc" holds one "<no>.brtex" texture file per face (0x36 of them); the whole
// archive stays loaded and each file is initialised once it is. The archive lives in a file-local
// object in d_hmn_face_tex_mng.cpp. Names other than the class names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>

class dHmnFaceTexMng_c {
public:
    // "/Plyr/FcTx/FaceTex.arc"
    class arc_c : public dDvd::arcBank_c {
    public:
        virtual ~arc_c() {} // 800B8190 (weak)
        virtual void onLoaded(); // 800B7FFC: initialises every face texture file

        void *getTex(u32 *size, int no); // 800B8078: "<no>.brtex" in the archive (no < 0x36)
    };

    static u32 getWorkSize(); // 800B80F8
    static BOOL load();       // 800B80FC
    static nw4r::g3d::ResFile getResFile(int no); // 800B8108
};
