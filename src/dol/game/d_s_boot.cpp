// The boot scene (dScBoot_c). .text 801C9EF4..801CA0F4. See include/game/game/d_s_boot.hpp.
#include <game/game/d_s_boot.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_s_boot_static.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_sys_gfx.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_3d/global.hpp>

CUSTOM_BASE_PROFILE(BOOT, dScBoot_c, SCENE_PROF_BOOT, 0xDE);

// 801C9F4C
void dScBoot_c::drawCallback() {
    m3d::calcView();
    m3d::drawOpa();
    m3d::drawXlu();
    m2d::draw();
}

// 801C9F78
int dScBoot_c::create() {
    dSysGfx::sDrawCallback = drawCallback;
    dSys::setFrameRate(1);
    return fn_8010875C(fn_80108698()) != FALSE;
}

// 801C9FBC
int dScBoot_c::preExecute() {
    int ret = dScene_c::preExecute();
    if (ret == NOT_READY) {
        fn_80108934(fn_80108698());
    }
    return ret;
}

// 801C9FFC
int dScBoot_c::execute() {
    fn_80108940(fn_80108698());
    return SUCCEEDED;
}

// 801CA024
int dScBoot_c::doDelete() {
    if (!fn_80108840(fn_80108698())) {
        return NOT_READY;
    }
    dSysGfx::sDrawCallback = NULL;
    return SUCCEEDED;
}

// 801CA064
int dScBoot_c::draw() {
    fn_80108A74(fn_80108698());
    return SUCCEEDED;
}

// 801CA08C
dScBoot_c::~dScBoot_c() {}

// 801CA0EC
int dScBoot_c::getKind() {
    return SCENE_KIND_BOOT;
}
