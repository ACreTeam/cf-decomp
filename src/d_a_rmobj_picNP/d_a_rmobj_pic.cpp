#include <game/game/d_a_rmobj_pic.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>

// The museum paintings. Functions are in target .text order.

namespace dARmobjPic {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_PIC, daRmobjPic_c);

// The painting nodes of the gallery models, in item order (from
// ITEM_IDX_DYNAMIC_PAINTING).
static const char *l_artNodes[] = {
    "art_iki",         "art_velazq",    "art_becomecalm", "art_bravely",
    "art_famous",      "art_fantastic", "art_rembra",     "art_manet",
    "art_goodlooking", "art_intimacy",  "art_matchless",  "art_goya",
    "art_interesting", "art_splendid",  "art_stylish",    "art_lively",
    "art_unspeakable", "art_valuable",  "art_wonderful",  "art_powerful",
    "art_fine",        "art_feminin",   "art_admirable",
};
} // namespace dARmobjPic

// .text 0x1C4
int daRmobjPic_c::create() {
    const char *name = dScene::getCurrentScene() == dScene::SCENE_RM_MM_PIC0
                           ? "obj_art_A"
                           : "obj_art_B";
    setDemoID(0);
    if (mModel.create(name, 0)) {
        nw4r::g3d::ResFile file(mModel.getData());
        nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
        for (u32 i = 0; i < ARRAY_SIZE(dARmobjPic::l_artNodes); i++) {
            dItem::Item art(dItem::ITEM_IDX_DYNAMIC_PAINTING, i, FALSE);
            if (!dSaveData_c::getTown()->mMuseum.isDonated(art)) {
                nw4r::g3d::ResNode node =
                    mdl.GetResNode(dARmobjPic::l_artNodes[i]);
                if (node.IsValid()) {
                    node.SetVisibility(false);
                }
            }
        }
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x2D0
int daRmobjPic_c::execute() {
    return SUCCEEDED;
}

// .text 0x2D8
int daRmobjPic_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x300
int daRmobjPic_c::doDelete() {
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
