// Include order: the weak RTTI / vtables in .data follow class completion (see
// d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj_cafe.hpp>
#include <game/framework/f_profile.hpp>

// The Roost guests' cups. Functions are in target .text order.

namespace dARmobjCafe {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_CAFE, daRmobjCafe_c);

u8 g_npcCreated;
u8 g_spNpcCreated;
} // namespace dARmobjCafe

// .text 0x3D8
int daRmobjCafe_c::create() {
    setDemoID(0);
    bool created = true;
    created &= mNpcCup.create("obj_cafe3", 0);
    created &= mNpcSaucer.create("obj_cafe4", 0);
    created &= mSpNpcCup.create("obj_cafe3", 0);
    created &= mSpNpcSaucer.create("obj_cafe4", 0);
    if (created) {
        mNpcCup.setAnmChr("obj_cafe3_12", m3d::FORWARD_ONCE, 0.0f);
        mNpcSaucer.setAnmChr("obj_cafe4_9", m3d::FORWARD_ONCE, 0.0f);
        mSpNpcCup.setAnmChr("obj_cafe3_11", m3d::FORWARD_ONCE, 0.0f);
        mSpNpcSaucer.setAnmChr("obj_cafe4_8", m3d::FORWARD_ONCE, 0.0f);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x4F8
int daRmobjCafe_c::execute() {
    static const char *cupNode = "cup2_mdl";
    static const char *steamEffect = "af_obj_coffee_npc";
    if (dARmobjCafe::g_npcCreated) {
        mNpcCup.playMdl();
        mNpcSaucer.playMdl();
        mVec3_c pos = *mNpcCup.getNodePos(cupNode);
        mNpcSteam.create(steamEffect, &pos, NULL, NULL, 0);
    }
    if (dARmobjCafe::g_spNpcCreated) {
        mSpNpcCup.playMdl();
        mSpNpcSaucer.playMdl();
        mVec3_c pos = *mSpNpcCup.getNodePos(cupNode);
        mSpNpcSteam.create(steamEffect, &pos, NULL, NULL, 0);
    }
    return SUCCEEDED;
}

// .text 0x5F4
int daRmobjCafe_c::draw() {
    if (dARmobjCafe::g_npcCreated) {
        mNpcCup.entry();
        mNpcSaucer.entry();
    }
    if (dARmobjCafe::g_spNpcCreated) {
        mSpNpcCup.entry();
        mSpNpcSaucer.entry();
    }
    return SUCCEEDED;
}

// .text 0x660
int daRmobjCafe_c::doDelete() {
    bool removed = true;
    removed &= mNpcCup.remove();
    removed &= mNpcSaucer.remove();
    removed &= mSpNpcCup.remove();
    removed &= mSpNpcSaucer.remove();
    return removed;
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
