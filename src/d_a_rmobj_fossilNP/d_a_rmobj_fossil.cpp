#include <game/game/d_a_rmobj_fossil.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>

// The museum fossils. Functions are in target .text order.

namespace dARmobjFossil {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_FOSSIL, daRmobjFossil_c);

// The fossil nodes of the models, in item order (from ITEM_IDX_AMBER).
static const char *l_fossilNodes[] = {
    "fos_amber", "fos_ammo",  "fos_droop", "fos_egg",   "fos_fern", "fos_footp",
    "fos_fbird", "fos_pekin", "fos_shark", "fos_trilo", "trex_a",   "trex_b",
    "trex_c",    "tri_a",     "tri_b",     "tri_c",     "mam_a",    "mam_b",
    "ank_a",     "ank_b",     "ank_c",     "apt_a",     "apt_b",    "apt_c",
    "dimet_a",   "dimet_b",   "dimet_c",   "igano_a",   "igano_b",  "igano_c",
    "saber_a",   "saber_b",   "pachy_a",   "pachy_b",   "pachy_c",  "para_a",
    "para_b",    "para_c",    "sei_a",     "sei_b",     "sei_c",    "sei_d",
    "suz_a",     "suz_b",     "suz_c",     "ste_a",     "ste_b",    "ste_c",
    "pte_a",     "pte_c",     "pte_b",     "iku_a",     "iku_b",    "raptor_a",
    "raptor_b",  "sty_a",     "sty_b",     "sty_c",
};
} // namespace dARmobjFossil

// .text 0x4CC
int daRmobjFossil_c::create() {
    const char *name = dScene::getCurrentScene() == dScene::SCENE_RM_MM_FOSSIL0
                           ? "obj_ms_fossil_a"
                           : "obj_ms_fossil_b";
    setDemoID(0);
    if (mModel.create(name, 0)) {
        nw4r::g3d::ResFile file(mModel.getData());
        nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
        for (u32 i = 0; i < ARRAY_SIZE(dARmobjFossil::l_fossilNodes); i++) {
            dItem::Item fossil(dItem::ITEM_IDX_AMBER, i, FALSE);
            if (!dSaveData_c::getTown()->mMuseum.isDonated(fossil)) {
                nw4r::g3d::ResNode node =
                    mdl.GetResNode(dARmobjFossil::l_fossilNodes[i]);
                if (node.IsValid()) {
                    node.SetVisibility(false);
                }
                // The amber has a second node.
                if (i == 0) {
                    nw4r::g3d::ResNode amber = mdl.GetResNode("amber_b");
                    if (amber.IsValid()) {
                        amber.SetVisibility(false);
                    }
                }
            }
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x604
int daRmobjFossil_c::execute() {
    return SUCCEEDED;
}

// .text 0x60C
int daRmobjFossil_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x634
int daRmobjFossil_c::doDelete() {
    if (mModel.remove()) {
        if (mBg0.mActive) {
            dBGCF::getMvbgList()->remove(&mBg0);
        }
        if (mBg1.mActive) {
            dBGCF::getMvbgList()->remove(&mBg1);
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
