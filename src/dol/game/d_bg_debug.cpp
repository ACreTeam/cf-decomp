// d_bg_debug: the BG debug process (dBgDebug_c) and its HostIO parameters (dBGO_bgHostIO_c).
// .text 80186E0C..801870D0, .ctors 80465820..80465824. See include/game/game/d_bg_debug.hpp.
#include <game/game/d_bg_debug.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_random_field.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_sv_auc.hpp>
#include <game/mLib/m_vec.hpp>

CUSTOM_BASE_PROFILE(BG_DEBUG, dBgDebug_c, 0xF2, 0xB);

// 80623F5C, 80623F74: unused.
static mVec3_c l_unused0;
static mVec3_c l_unused1;

static dBGO_bgHostIO_c l_hostIO; // 80623F90

// 80186E58
int dBgDebug_c::create() {
    fn_800AC278();
    if (isCityScene(getCurrentScene())) {
        dSvAuc_c::create();
    }
    fn_801C8664();
    return 1;
}

// 80186E98
int dBgDebug_c::execute() {
    fn_800AC280();
    return 1;
}

// 80186EBC
int dBgDebug_c::draw() {
    return 1;
}

// 80186EC4
int dBgDebug_c::doDelete() {
    if (dRF::finishCreate()) {
        fn_800AC284();
        if (isCityScene(getCurrentScene())) {
            dSvAuc_c::resetChanged();
        }
        fn_801C866C();
        return 1;
    }
    return 0;
}

// 80186F18
s16 dBgDebug_c::getGroundSeType() {
    return -1;
}

// 80186F20
dBGO_bgHostIO_c::~dBGO_bgHostIO_c() {}

// 80186F60
dBgDebug_c::~dBgDebug_c() {}
