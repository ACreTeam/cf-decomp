#include <game/game/d_travel_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_mii_icon.hpp>
#include <stddef.h>
#include <game/game/d_field_assessment.hpp>
#include <game/cLib/c_lib.hpp>

int addPlayerDlItems(u8 *blocks, dPrivateData_c *player);
int countPlayerDlItems(u8 *blocks);
int countTownDlItems(u8 *blocks, dPrivateData_c *player);

namespace dTravelData {
static inline dItem::Item &getPocket(dPrivateData_c *player, u32 i) {
    return player->mPockets[i];
}
static inline u32 pocketMask(dPrivateData_c *player) {
    int i;
    u32 mask = 0;
    for (i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (getPocket(player, i).mId != dItem::ITEM_ID_NONE) {
            mask |= 1 << i;
        }
    }
    return ((mask >> 24 & 0xFF) | (mask >> 8 & 0xFF00)) |
           ((mask << 24 & 0xFF000000) | (mask << 8 & 0xFF0000));
}

// 80084840
Data_c::Data_c() {}
// 80084910
Data_c::~Data_c() {}

// 80084990
void Data_c::prepareDeparture() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dSaveOption::store(player->getOption());
    mPlayer.copy(player);
    mPlayer.setFlag0(0x28);
    mPlayer._55F4++;
    if (dNpcInfo::gatekeeperA_c::get()->mEscorting) mPlayer.mHost.mFlagB = 1;
    mPocketMaskLE = pocketMask(player);
    addPlayerDlItems((u8 *)mPlayerItems.mItems, player);
    mJinxOutcome = JINX_OUTCOME_NUM;
    void *mii = dMiiIcon::getTravelMii();
    if (mii) {
        cLib::memCpy(mMii, mii, sizeof(mMii));
        mHasMii = 1;
    } else mHasMii = 0;
    mOutgoingAnimal.clear();
    if (!dNpc::takeMovedAnimal(&mOutgoingAnimal)) mOutgoingAnimal.clear();
    mIncomingAnimal.clear();
}

// 80084B88
void Data_c::backupDeparture() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dPlayerMgr::Mgr_c::getPlayer(5)->copy(player);
    player->mEquipment.mHat = dItem::ITEM_ID_NONE;
    player->mEquipment.mAcc = dItem::ITEM_ID_NONE;
    player->mEquipment.mHeld = dItem::ITEM_ID_NONE;
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        dItem::Item item;
        item = dItem::ITEM_ID_NONE;
        player->setPocket(&item, i, 0);
    }
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) player->clearLetter(i);
    player->mBells = 0;
    player->setFlag0(0x28);
    if (!dPrivateData_c::isFlag0Any(dSaveData_c::getTown()->mPlayers, 0x4B)) player->setFlag0(0x4B);
    player->_55F4++;
    player->clearFlag0(PRIVATE_FLAG0_BEE_STUNG);
    fgMngProc_backupSaveFlag1A();
    dSaveData_c::getTown()->backupFlag6();
    dNpc::removeMovedAnimal(&mOutgoingAnimal.mID);
    fgMngProc_stashFg94();
}

// 80084CA4
void Data_c::cancelDeparture() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dPrivateData_c *backup = dPlayerMgr::Mgr_c::getPlayer(5);
    player->copy(backup);
    backup->clear();
    fgMngProc_restoreSaveFlag1A();
    dNpc::addMovedAnimal(&mOutgoingAnimal);
    mOutgoingAnimal.clear();
    fgMngProc_restoreFg94();
}

// 80084D24
void Data_c::prepareVisit() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dSaveOption::store(player->getOption());
    mPlayer.copy(player);
    mPlayer.startHostVisit(32, 10, 0);
    mPocketMaskLE = pocketMask(player);
    addPlayerDlItems((u8 *)mPlayerItems.mItems, player);
    if (dActivityLog::isDone()) mJinxOutcome = dActivityLog::getJinx()->mOutcome;
    else mJinxOutcome = JINX_OUTCOME_NUM;
    void *mii = dMiiIcon::getTravelMii();
    if (mii) {
        cLib::memCpy(mMii, mii, sizeof(mMii));
        mHasMii = 1;
    } else mHasMii = 0;
    mOutgoingAnimal.clear();
    if (!dNpc::takeMovedAnimal(&mOutgoingAnimal)) mOutgoingAnimal.clear();
    mIncomingAnimal.clear();
}

// 80084F18
void Data_c::backupVisit() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(4);
    dPlayerMgr::Mgr_c::getPlayer(5)->copy(player);
    dActivityLog::backupTownBits();
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mActivity.setVisitorDone(dActivityLog::isDone());
    fgMngProc_backupSaveFlag1A();
    dSaveData_c::getTown()->backupFlag6();
    dNpc::removeMovedAnimal(&mOutgoingAnimal.mID);
    fgMngProc_stashFg94();
}

// 80084FA0
void Data_c::cancelVisit() {
    dPrivateData_c *backup = dPlayerMgr::Mgr_c::getPlayer(5);
    dPlayerMgr::Mgr_c::getPlayer(4)->copy(backup);
    backup->clear();
    dActivityLog::restoreTownBits();
    fgMngProc_restoreSaveFlag1A();
    dNpc::addMovedAnimal(&mOutgoingAnimal);
    mOutgoingAnimal.clear();
    fgMngProc_restoreFg94();
}

// 80085018
void Data_c::finishReturn() {
    int slot = dPlayerMgr::findPersonalIDSlot(&mPlayer.mPID);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(slot);
    player->copy(&mPlayer);
    dPlayerMgr::setPlayerSlot(0, slot);
    dSaveOption::store(player->getOption());
    dSaveData_c::getTown()->mTownHost = player->mHost;
    countPlayerDlItems((u8 *)mPlayerItems.mItems);
    countTownDlItems((u8 *)mTownItems.mItems, player);
    dActivityLog::applyTravelResult(mJinxOutcome);
    dNpc::addMovedAnimal(&mOutgoingAnimal);
    mOutgoingAnimal.clear();
    dNpc::addMovedAnimal(&mIncomingAnimal);
    mIncomingAnimal.clear();
}

// 800851A0
void Data_c::finishVisit() {
    dPrivateHost_c *host;
    dPrivateHost_c *townHost;
    dPrivateData_c *player;
    player = dPlayerMgr::Mgr_c::getPlayer(4);
    player->copy(&mPlayer);
    dPlayerMgr::setPlayerSlot(0, 4);
    dSaveOption::store(player->getOption());
    countPlayerDlItems((u8 *)mPlayerItems.mItems);
    countTownDlItems((u8 *)mTownItems.mItems, player);
    host = &player->mHost;
    townHost = &dSaveData_c::getTown()->mTownHost;
    if (host->mFlagB && host->mPID.land == dSaveData_c::getTown()->mLandID) {
        host->clear();
        townHost->clear();
        player->setFlag0(0x48);
        player->setFlag0(0x4A);
        player->clearFlag1(0x32);
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    dActivityLog::setJinx(&town->mActivity.mJinx);
    if (mHasMii) dMiiIcon::setTravelMii(mMii);
    else dMiiIcon::setTravelMii(NULL);
    dNpc::addMovedAnimal(&mOutgoingAnimal);
    mOutgoingAnimal.clear();
    dNpc::addMovedAnimal(&mIncomingAnimal);
    mIncomingAnimal.clear();
}

// 80085334
void Data_c::clear() {
    mPlayer.clear();
    mJinxOutcome = JINX_OUTCOME_NUM;
    mOutgoingAnimal.clear();
    mIncomingAnimal.clear();
}


typedef char CheckSize[sizeof(Data_c) == 0x647C0 ? 1 : -1];
typedef char CheckItems[offsetof(Data_c, mPlayerItems) == 0x1E004 ? 1 : -1];
typedef char CheckAnimal[offsetof(Data_c, mOutgoingAnimal) == 0x59060 ? 1 : -1];
typedef char CheckPlayer[offsetof(Data_c, mPlayer) == 0x5C0A0 ? 1 : -1];
typedef char CheckOutcome[offsetof(Data_c, mJinxOutcome) == 0x64760 ? 1 : -1];
} // namespace dTravelData
