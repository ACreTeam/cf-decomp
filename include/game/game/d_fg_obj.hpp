#pragma once

// d_fg_obj (DOL TU d_fg_obj.cpp, .text 800A70AC..800A7368): the field object resources. Class names from the
// RTTI: dFgObj::alwaysBank_c (the archive d_s_boot_static loads into the fg heap at boot) and fgObjRes_c (global
// scope; the season's /FgObj/fgObjSeason%02d.brres, whose tree and grass palettes are handed to the effect
// textures when it loads). Member and function names inferred.
// Class order: MWCC emits the vtables/RTTI in reverse order (fgObjRes_c's first in the target's .data).

#include <types.h>
#include <game/game/d_dvd.hpp>

namespace dFgObj {

class alwaysBank_c : public dDvd::bank_c {
public:
    virtual ~alwaysBank_c() {} // 800A72CC
    inline BOOL load(void *heap); // defined in d_s_boot_static (/FgObj/fgItem.brres)
    virtual void onLoaded() {  // 800A7328
        nw4r::g3d::ResFile res(mpData);
        res.Init();
        res.Bind(res);
    }
};

alwaysBank_c *getAlwaysBank(); // 800A70AC

} // namespace dFgObj

// "/FgObj/fgObjSeason%02d.brres" (the save's season).
class fgObjRes_c : public dDvd::bank_c {
public:
    fgObjRes_c() {}
    virtual ~fgObjRes_c();   // 800A71F4
    virtual void onLoaded(); // 800A711C: binds the brres and gives its palettes to the effect textures

    BOOL load(void *heap); // 800A70B8
};

extern fgObjRes_c g_fgObjRes; // 8058B680: the season brres (d_fg_draw, d_fgobj_manager)
