// The town state byte with per-villager player flags (dSvPlayerFlag::Flag_c). .text 801510A0..801511B4.
// See include/game/game/d_sv_player_flag.hpp.
#include <game/game/d_sv_player_flag.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>

namespace dSvPlayerFlag {

// 801510A0
void Flag_c::init() {
    if (cM::rndInt(2) == 0) {
        mState = STATE_FLOORING;
    } else {
        mState = STATE_WALLPAPER;
    }
}

// 801510EC
void Flag_c::clear(int idx) {
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mVillagerDailyBits &= ~(1 << idx);
}

// 8015112C
int Flag_c::get(int idx) {
    if (!dAcNpc_c::isPlayerFromThisTown()) {
        return 0;
    }
    if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_SAHARAH_ASKED)) {
        return 0;
    }
    int bits = dPlayerMgr::Mgr_c::getCurrentPlayer()->mVillagerDailyBits;
    if (bits & (1 << idx)) {
        return mState;
    }
    return 0;
}

} // namespace dSvPlayerFlag
