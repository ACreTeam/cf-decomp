// d_fg_obj: the field object resources (dFgObj::alwaysBank_c, fgObjRes_c). .text 800A70AC..800A7368,
// .ctors 80465678..8046567C, .rodata 8046F218..8046F228, .data 804E58E0..804E5AE8, .bss 8058B608..8058B6D8,
// .sdata 8074A3D0..8074A3E0. See include/game/game/d_fg_obj.hpp.
#include <game/game/d_fg_obj.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_scene.hpp>
#include <stdio.h>

static dFgObj::alwaysBank_c l_alwaysBank; // 8058B618
fgObjRes_c g_fgObjRes;                    // 8058B680

namespace dFgObj {

// 800A70AC
alwaysBank_c *getAlwaysBank() {
    return &l_alwaysBank;
}

} // namespace dFgObj

// 800A70B8
BOOL fgObjRes_c::load(void *heap) {
    char path[64];
    sprintf(path, "/FgObj/fgObjSeason%02d.brres", getSaveSeason());
    return bank_c::load(path, heap, 0);
}

// 804E5940: the palettes of the brres.
static const char *l_plttNames[] = {"pl_treeA", "pl_treeA_1", "pl_treeA_2", "pl_treeB", "pl_treeB", "pl_grass"};

enum { TEX_NUM = 15 };

// 804E5A38: the effect textures that take them.
static const char *l_texNames[TEX_NUM] = {
    "tex_treeA0_nml", "tex_treeA1_nml", "tex_treeA2_nml", "tex_treeB",      "tex_treeC",
    "tex_treeA0_chr", "tex_treeA1_chr", "tex_treeA2_chr", "tex_treeA0_snw", "tex_treeA1_snw",
    "tex_treeA2_snw", "tex_treeB_snw",  "tex_treeC_snw",  "tex_clover",     "tex_grass",
};

// 8046F218: the palette (l_plttNames) of each effect texture.
static const u8 l_texPltt[TEX_NUM] = {0, 1, 2, 3, 4, 0, 1, 2, 0, 1, 2, 3, 4, 5, 5};

// 800A711C
void fgObjRes_c::onLoaded() {
    nw4r::g3d::ResFile res(mpData);
    res.Init();
    res.Bind(res);
    nw4r::g3d::ResFile file(g_fgObjRes.mpData);
    for (int i = 0; i < TEX_NUM; i++) {
        nw4r::g3d::ResPltt pltt = file.GetResPltt(l_plttNames[l_texPltt[i]]);
        fn_800887FC(l_texNames[i], pltt.GetPlttData());
    }
}

// 800A71F4
fgObjRes_c::~fgObjRes_c() {}
