// The whole save file in memory. .text 8010DC04..8010F124, .sbss 8074E6E0..8074E6E8.
// See notes/save_file_layout.txt.
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <lib/egg/core/eggHeap.h>
#include <cstring>
#include <cstddef>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_clock.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_field_process.hpp>
#include <game/game/d_footmark.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_weather.hpp>
#include <lib/revolution/OS/OSTime.h>

// Dependencies whose owners are not recovered yet.
extern "C" {


void fn_80101424();
void fn_800CAC20();
}

// Game heap (owner not recovered).

// 8074E6E0
dSaveData_c *dSaveData_c::sSaveData;

// 8010DC04
u32 dSaveData_c::getDLDataOffset() {
    return offsetof(dSaveData_c, mDLItems);
}

// 8010DC10
u16 dSaveData_c::getVersion() {
    return SAVE_VERSION;
}

// 8010DC18
dSaveData_c *dSaveData_c::get() {
    dSvMgr_c::isFullTransferComplete();
    return sSaveData;
}

// 8010DC3C
dSaveData_c *dSaveData_c::getRaw() {
    return sSaveData;
}

// 8010DC44
BOOL dSaveData_c::isGood() {
    if (!dPrivateData_c::isUserDataValidAll(mPlayers)) {
        return FALSE;
    }
    if (!isHeaderState2()) {
        return FALSE;
    }
    BOOL ok = isHeaderVersionOK() != FALSE;
    return ok;
}

// 8010DCB0
void dSaveData_c::initialize() {
    clearTown();
    dSaveDLItemList_c::get()->updateChecksum();
    mExtra.init();
}

// 8010DCF0
void dSaveData_c::prepareSave() {
    dSaveTown_c *town = getTown();
    dPrivateData_c *player = dPrivateData_c::getChecked(town->mPlayers, fn_801017B8());
    if (player != NULL) {
        mTownInfo.mEggPlayer.copy(&player->mPID);
        if (player->mPID.isFromTown()) {
            getTown()->backupFlag6();
        }
    }
    dSaveOption::storeToCurrentPlayer();
    dTime_c now = *dTime_c::getCurrent();
    mSaveTime.set(OSCalendarTimeToTicks(&now));
}

// 8010DDE0
void dSaveData_c::updateOnSave() {
    mFishing.advance(TRUE);
    mBugOff.advance(TRUE);
    mFishing.update();
    mFishing.checkDay();
    mBugOff.update();
    mBugOff.checkDay();
    mExtra.mDesignBoard.validate();
    fn_801002BC();
    fn_800F47F4();
    fgMngProc_clearFg94();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        player->clearFlag0(0x29);
        if (player->isFlag1(0x44)) {
            player->mDistPatternVersion = getExtra()->mDistPattern.mVersion;
            player->setFlag1(0x3B);
            player->clearFlag1(0x44);
        }
        player->mDaysPassed = 0;
    }
}

// 8010DED0
extern "C" void fn_8010DED0() {
    dPhoto::clear();
    fn_80101424();
    fn_800CAC20();
}

// 8010DEF8
void dSaveData_c::refreshTown() {
    dFdBase_c *fd = dField::getInfo(FD_ID_TOWN);
    if (fd != NULL) {
        ((dFdInfoSv_c *)fd)->updateTown();
    }
    dWeather::requestReset();
    dSTR::getTownUnitAttr()->setAll();
    dFootmark::getEditor()->loadSave();
    dActivityLog::setJinx(&dSaveData_c::getTown()->mActivity.mJinx);
    dField::reloadHomes();
    dTimeStamp_c lastDay = dSaveData_c::getTown()->mTownInfo.mLastDay;
    dEvent::buildSchedule(lastDay.get(), TRUE);
    dClock::resetTimer();
    dNpcMng_c::_8074EAD0 = 0;
    fn_8010003C();
}

// 8010DFC0
void dSaveData_c::setupAfterLoad() {
    dFdBase_c *fd = dField::getInfo(FD_ID_TOWN);
    if (fd != NULL) {
        ((dFdInfoSv_c *)fd)->updateTown();
    }
    dTime_c::loadOffset();
    dWeather::resetCache();
    dSTR::getTownUnitAttr()->setAll();
    dFootmark::getEditor()->loadSave();
    dActivityLog::applyResult();
    dField::reloadHomes();
    dTimeStamp_c lastDay = getTown()->mTownInfo.mLastDay;
    dEvent::buildSchedule(lastDay.get(), TRUE);
    u16 none = 0;
    dNet::setRecord(0x2A, &none, 0);
    dPrivateData_c::saveHostToTown();
    fn_800F47F4();
    dPlayerMgr_c::getCurrentPlayer()->clearFlag1(0x3D);
    clearAllRoomMaps();
    dSTR::resetLocks();
    fn_8010003C();
}

// 8010E0A8
void dSaveData_c::updateChecksum() {
    updateTownChecksum();
    dSaveDLItemList_c *items = dSaveDLItemList_c::get();
    items->mChecksum = items->calcChecksum();
    mExtra.updateChecksum();
}

// 8010E0F8
BOOL dSaveData_c::isExtraGood(int arg) {
    if (!isTownChecksumOK(arg)) {
        return FALSE;
    }
    if (!mExtra.isChecksumOK(arg)) {
        return FALSE;
    }
    u16 version = mItemVersion;
    if (version != dItem::BITM::getVersion()) {
        return FALSE;
    }
    u32 stamp = mBuilding.mList.mChecksum;
    BOOL same = mBuilding.mList.calcChecksum() == stamp;
    if (!same) {
        return FALSE;
    }
    BOOL ok = dSaveDLItemList_c::getRaw()->isChecksumOK() != FALSE;
    return ok;
}

// 8010E1B8
dSaveDLItemList_c *dSaveData_c::getDLData() {
    dSvMgr_c::isDLDataTransferComplete();
    return &get()->mDLItems;
}

// 8010E1E4
dSaveTown_c *dSaveData_c::getTown() {
    dSvMgr_c::isTownTransferComplete();
    return get();
}

// 8010E208
dSaveExtra_c *dSaveData_c::getExtra() {
    dSvMgr_c::isTownTransferComplete();
    return &get()->mExtra;
}

// 8010E234
dSaveData_c *dSaveData_c::getRaw2() {
    return sSaveData;
}

// 8010E23C
u32 dSaveData_c::getSize() {
    return sizeof(dSaveData_c);
}

// 8010E248
// The ctor is inlined here.
void dSaveData_c::create() {
    if (sSaveData == NULL) {
        sSaveData = (dSaveData_c *)dHeap::saveHeap_p->alloc(sizeof(dSaveData_c), 32);
        memset((void*)sSaveData, 0, sizeof(dSaveData_c));
        new (sSaveData) dSaveData_c();
        sSaveData->initialize();
    }
}

// 8010EA88
void dSaveData_c::clear() {
    memset((void *)this, 0, sizeof(dSaveData_c));
}

// 8010EA98
dPrivateData_c *dSaveData_c::setupPlayer(const wchar_t *name, u8 gender, u32 idx) {
    dPrivateData_c *player = dPlayerMgr_c::getPlayer(idx);
    u16 ids[PLAYER_NUM];
    u16 num = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *other = dPlayerMgr_c::getPlayer(i);
        if (i != idx && other->mPID.isValid() == 1U) {
            ids[num++] = other->mPID.player.mId;
        }
    }
    player->setup(name, dPersonalID_c::generateId(ids, num), gender);
    return player;
}

#include "d_save_data_dist.inc"
