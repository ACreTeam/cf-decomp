#include <game/game/d_a_rmobj_sample.hpp>
#include <game/framework/f_profile.hpp>

// The sample room object. Functions are in target .text order.

namespace dARmobjSample {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_SAMPLE, daRmobjSample_c);
} // namespace dARmobjSample

// .text 0x1C4
int daRmobjSample_c::create() {
    setDemoID(0);
    if (mModel.create("obj_r_box", 0)) {
        if (!mModel.setAnmChr("obj_r_box", m3d::FORWARD_LOOP, 0.0f)) {
            return FAILED;
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x248
int daRmobjSample_c::execute() {
    mModel.playMdl();
    return SUCCEEDED;
}

// .text 0x270
int daRmobjSample_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x298
int daRmobjSample_c::doDelete() {
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
