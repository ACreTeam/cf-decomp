#include <game/game/d_a_rmobj_mirrorball.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/sLib/s_lib.hpp>

// The Roost's stage. Functions are in target .text order.

namespace dARmobjMirrorball {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_MIRRORBALL, daRmobjMirrorBall_c);
} // namespace dARmobjMirrorball

daRmobjMirrorBall_c *daRmobjMirrorBall_c::sInstance;

// .text 0x1C4
int daRmobjMirrorBall_c::create() {
    setDemoID(0);
    bool created = true;
    created &= mModel.create("obj_cafe_stage", 0);
    dFixSpotSwitchLight_c *light = dFixSpotSwitchLight_c::sInstance;
    if (created && light != NULL) {
        light->setOnImmediate(1);
        mModel.setAnm("obj_cafe_stage", m3d::FORWARD_LOOP, 0.0f);
        mLight0 = 1.0f;
        mLight1 = 0.0f;
        mLight0On = 1;
        mLight1On = 0;
        sInstance = this;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x298
int daRmobjMirrorBall_c::execute() {
    static const char *const materials[] = {"m_cf_mlight", "m_cf_mlight1"};
    // The materials lit by mLight1 (all of them; mLight0 is unused).
    static const int light1Num = 2;
    if (mLight0On) {
        sLib::chase(&mLight0, 1.0f, 1.0f / 120.0f);
    } else {
        sLib::chase(&mLight0, 0.0f, 1.0f / 120.0f);
    }
    if (mLight1On) {
        sLib::chase(&mLight1, 1.0f, 1.0f / 30.0f);
    } else {
        sLib::chase(&mLight1, 0.0f, 1.0f / 30.0f);
    }
    mModel.play();
    nw4r::g3d::ResFile file(mModel.getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    for (int i = 0; i < ARRAY_SIZE(materials); i++) {
        nw4r::g3d::ResMatTevColor tev =
            mdl.GetResMat(materials[i]).GetResMatTevColor();
        GXColor color;
        tev.GXGetTevKColor(GX_KCOLOR1, &color);
        if (i < light1Num) {
            color.a = 255.0f * mLight1;
        } else {
            color.a = 255.0f * mLight0;
        }
        tev.GXSetTevKColor(GX_KCOLOR1, color);
        tev.DCStore(false);
    }
    return SUCCEEDED;
}

// .text 0x424
int daRmobjMirrorBall_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x44C
int daRmobjMirrorBall_c::doDelete() {
    sInstance = NULL;
    return mModel.remove();
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
