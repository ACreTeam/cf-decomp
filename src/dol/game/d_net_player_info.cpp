// This console's player names for the net (dNetPlayerInfo::Info_c). .text 800AC154..800AC1CC.
// See include/game/game/d_net_player_info.hpp.
#include <game/game/d_net_player_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <string.h>

namespace dNetPlayerInfo {

// 800AC154
void Info_c::clear() {
    memset(this, 0, sizeof(*this));
}

// 800AC160
void Info_c::set() {
    clear();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    memcpy(mPlayerName, player->mPID.player.mName, sizeof(mPlayerName));
    memcpy(mTownName, player->mPID.land.mName, sizeof(mTownName));
    memcpy(mFriendName, player->mFriends.mName, sizeof(mFriendName));
}

} // namespace dNetPlayerInfo
