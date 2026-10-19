#include <egg/core/eggHeap.h>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_footmark.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_hr.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_sv_proc.hpp>
#include <game/game/d_wifi.hpp>

// Unrecovered dependency interfaces; the implementations are outside this TU.
BOOL sendVisitorLetter(dPrivateData_c *player);

namespace dSvProc {
dSvProc_c sProc;
typedef BOOL (dSvProc_c::*Step)();
static const Step sSteps[dSvProc_c::MODE_COUNT][dSvProc_c::STEP_COUNT] = {
    {&dSvProc_c::createTown, &dSvProc_c::startTown, &dSvProc_c::finishTown},
    {&dSvProc_c::createPlayer, &dSvProc_c::startPlayer,
     &dSvProc_c::finishPlayer},
    {&dSvProc_c::createVisitor, &dSvProc_c::startVisitor,
     &dSvProc_c::finishVisitor},
    {&dSvProc_c::createVisitorPlayer, &dSvProc_c::startVisitorPlayer,
     &dSvProc_c::finishVisitorPlayer},
    {&dSvProc_c::prepareLoad, &dSvProc_c::startLoad, &dSvProc_c::finishLoad},
    {&dSvProc_c::createTitleTown, &dSvProc_c::startTitleTown,
     &dSvProc_c::finishTitleTown},
    {&dSvProc_c::prepareGrowth, &dSvProc_c::startGrowth,
     &dSvProc_c::finishGrowth}};
} // namespace dSvProc

// 80154358
void dSvProc_c::reset() {
    mPreviousStep = STEP_COUNT;
    mStep = 0;
}

// 8015436C
BOOL dSvProc_c::process(int mode) {
    if (mStep < STEP_COUNT) {
        const dSvProc::Step *steps = dSvProc::sSteps[mode];
        mPreviousStep = mStep;
        if (!steps[mStep]) {
            return FALSE;
        }
        if ((this->*steps[mStep])()) {
            ++mStep;
        }
        if (mStep < STEP_COUNT) {
            return FALSE;
        }
        return TRUE;
    }
    return TRUE;
}

// 80154424
BOOL dSvProc_c::createTown() {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dSaveTown_c *fieldTown = dSaveData_c::getTown();
    if (!fieldTown->mMainField.createField(heap)) {
        return FALSE;
    }
    loadResources();
    initializePlayer();
    initializeTown();
    return TRUE;
}
// 80154498
BOOL dSvProc_c::startTown() {
    if (!(dNpc::initTownAnimals() & 1)) {
        return FALSE;
    }
    dSaveData_c::getTown()->mTownInfo.init();
    prepareTown();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 801544FC
BOOL dSvProc_c::finishTown() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    dHR::selectModelRooms(0, 1);
    updateTown();
    dSaveData_c::getTown()->mHeader.setState(2);
    return TRUE;
}
// 8015455C
BOOL dSvProc_c::createPlayer() {
    loadResources();
    initializePlayer();
    return TRUE;
}
// 80154594
BOOL dSvProc_c::startPlayer() {
    prepareTown();
    dEvent::resetDailyId();
    fgMngProc_clearFg94();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 801545CC
BOOL dSvProc_c::finishPlayer() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    updateTown();
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    extra->mLangFlags.update(dPlayerMgr::getCurrentPlayerSlot(), NULL,
                             dSvLangFlag::LANG_FLAG_LANGUAGE_CURRENT);
    return TRUE;
}
// 80154634
BOOL dSvProc_c::createVisitor() {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dSaveTown_c *fieldTown = dSaveData_c::getTown();
    if (!fieldTown->mMainField.createField(heap)) {
        return FALSE;
    }
    loadResources();
    initializeVisitor();
    initializeTown();
    return TRUE;
}
// 801546A8
BOOL dSvProc_c::startVisitor() {
    if (!(dNpc::initTownAnimals() & 1)) {
        return FALSE;
    }
    dSaveData_c::getTown()->mTownInfo.init();
    prepareTown();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 8015470C
BOOL dSvProc_c::finishVisitor() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    dHR::selectModelRooms(0, 1);
    updateTown();
    dSaveData_c::getTown()->mHeader.setState(2);
    return TRUE;
}
// 8015476C
BOOL dSvProc_c::createVisitorPlayer() {
    loadResources();
    initializeVisitor();
    return TRUE;
}
// 801547A4
BOOL dSvProc_c::startVisitorPlayer() {
    prepareTown();
    fgMngProc_clearFg94();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 801547D8
BOOL dSvProc_c::finishVisitorPlayer() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    updateTown();
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    extra->mLangFlags.update(dPlayerMgr::getCurrentPlayerSlot(), NULL,
                             dSvLangFlag::LANG_FLAG_LANGUAGE_CURRENT);
    return TRUE;
}
// 80154840
BOOL dSvProc_c::prepareLoad() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mWasVisitor = player ? player->isFlag0(PRIVATE_FLAG0_TRAVELLING) : FALSE;
    if (player->mPID.land == dSaveData_c::getTown()->mLandID) {
        player->clearFlag0(PRIVATE_FLAG0_TRAVELLING);
    }
    loadResources();
    return TRUE;
}
// 80154904
BOOL dSvProc_c::startLoad() {
    prepareTown();
    dEvent::resetDailyId();
    fgMngProc_clearFg94();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 8015493C
BOOL dSvProc_c::finishLoad() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (mWasVisitor) {
        dNpcMng_c::_8074EACF = 1;
    }
    if (player->mPID.land == dSaveData_c::getTown()->mLandID) {
        player->clearFlag0(0x4B);
        if (dPrivateData_c::isFlag0Any(dSaveData_c::getTown()->mPlayers,
                                       0x4B)) {
            player->setFlag0(0x4C);
        } else {
            player->clearFlag0(0x4C);
        }
        dSaveData_c::getTown()->mTownHost.clearFlagB();
        if (dSaveData_c::getTown()->mTownHost.mCount == 0) {
            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers,
                                          0x47);
        }
        if (dSaveData_c::getTown()->mTownHost.getFlagC() == 1) {
            dPrivateHost_c *host = &dSaveData_c::getTown()->mTownHost;
            BOOL clear = TRUE;
            for (int i = 0; i < PLAYER_NUM; ++i) {
                dPrivateData_c *p = dPlayerMgr::Mgr_c::getPlayer(i);
                if (p->mPID.isValid() &&
                    dWifi::findFriend(&host->mPID, p) >= 0) {
                    clear = FALSE;
                }
            }
            if (clear) {
                dSaveData_c::getTown()->mTownHost.clear();
            }
        }
        dSaveData_c::getExtra()->mDesignBoard.update();
        dNpcInfo::resetsan_c::get()->init();
        if (!mWasVisitor && !dNpcMng_c::isPlayerSelectVisitor()) {
            if (player->getResetCount() == 0) {
                dNpcInfo::resetsan_c::get()->set(1);
            } else if (player->isFlag0(PRIVATE_FLAG0_PLAYING_UNSAVED)) {
                dNpcInfo::resetsan_c::get()->set(1);
            }
            if (dNpcInfo::resetsan_c::get()->getVisit() == 1) {
                player->syncResetCount();
            }
        }
        player->setFlag0(PRIVATE_FLAG0_PLAYING_UNSAVED);
        dPrivateData_c::loadHostFromTown();
        dSvStarSign::List_c::registerPlayerSign();
        checkSummerMorningStart();
        if (dPostOffice::canGetDistMail()) {
            player->setFlag0(PRIVATE_FLAG0_DIST_MAIL_PENDING);
        }
        sendVisitorLetter(player);
        if (player) {
            player->mVisitorLetter.clear();
        }
    }
    updateTown();
    return TRUE;
}
// 80154BD0
BOOL dSvProc_c::createTitleTown() {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dSaveTown_c *fieldTown = dSaveData_c::getTown();
    if (!fieldTown->mMainField.createField(heap)) {
        return FALSE;
    }
    loadResources();
    wchar_t name[9] = L"タイトルデモ"; // Title Demo
    dSaveData_c::getTown()->mHeader.setState(0);
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mLandID.setRandomId(name, getLanguage());
    dSaveData_c::getTown()->mDaily.request();
    initializeTown();
    return TRUE;
}
// 80154CC4
BOOL dSvProc_c::startTitleTown() {
    if (!(dNpc::initTownAnimals() & 1)) {
        return FALSE;
    }
    dSaveData_c::getTown()->mTownInfo.init();
    prepareTown();
    dEvent::resetDailyId();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 80154D2C
BOOL dSvProc_c::finishTitleTown() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    updateTown();
    return TRUE;
}
// 80154D74
BOOL dSvProc_c::prepareGrowth() {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dSaveTown_c *fieldTown = dSaveData_c::getTown();
    if (!fieldTown->mMainField.createFinish(heap)) {
        return FALSE;
    }
    loadResources();
    dTime_c::loadOffset();
    return TRUE;
}
// 80154DDC
BOOL dSvProc_c::startGrowth() {
    prepareTown();
    dEvent::resetDailyId();
    fgMngProc_clearFg94();
    startFgMngProcThread(fgMngProcThreadFunc);
    return TRUE;
}
// 80154E14
BOOL dSvProc_c::finishGrowth() {
    if (!isFgMngProcThreadDone()) {
        return FALSE;
    }
    updateTown();
    return TRUE;
}
// 80154E5C
void dSvProc_c::loadResources() {
    dSaveTown_c::RegionInfo_c &region = dSaveData_c::getTown()->mRegionInfo;
    int language = getLanguage();
    region.language = language;
    region.region = getLanguageArea(language);
    dItem::infoBank_c::get()->loadDlItems();
    dSTR::getTownUnitAttr()->setAll();
    dFootmark::getEditor()->loadSave();
    dSTR::resetCount();
}
// 80154EC4
void dSvProc_c::prepareTown() {
    dField::reloadHomes();
    dSaveData_c::getTown()->mMainField.resetCreate();
    dBgUtil::initSceneItems();
    clearAllRoomMaps();
    dHR::updateRates();
    dSaveData_c::getTown()->mAnimals.mTown.expireLostItemQuests();
    dSaveData_c::getTown()->mAnimals.mTown.checkStylePlayers();
    dSaveData_c::getTown()->mAnimals.mTown.updateAppointmentFlags();
    dSaveData_c::getTown()->mAnimals.mTown.checkHideAndSeek();
    dSaveData_c::getTown()->mVisitorNpc.buildTodayEvents();
}
// 80154F50
void dSvProc_c::updateTown() {
    dSaveData_c::getTown()->mShops.update();
    dSaveData_c::getTown()->mDaily.update();
    dNpc::initEntries();
    dNpcInfo::init();
    daSnowMan_c::clearBalls();
    dSaveData_c::getTown()->mAnimals.mTown.resetDailyTalkCounts();
    dSaveData_c::getTown()->mAnimals.mTown.assignQuestCandidates();
    dSTR::getCityUnitAttr()->setAll();
    dSaveData_c::getTown()->setFlag(0x15);
    dKbd::resetSetting();
    dSaveData_c::getTown()->mAnimals.mTown.clearSpots();
    dSaveData_c::getTown()->mAnimals.mTown.clearAllQuestStarted();
    dSaveData_c::getTown()->mAnimals.mTown.clearAllQuestStarted2();
    dSaveData_c::getTown()->mAnimals.mTown.updateTalkFlags(NULL);
    if (dScene::getCurrentScene() == dScene::SCENE_DM_PL_SEL) {
        dSaveData_c::getTown()->mAnimals.mTown.clearAllMemoryFlag9();
    }
}
// 80155034
void dSvProc_c::initializePlayer() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    player->mPID.land.copy(dSaveData_c::getTown()->getLandID());
    player->mOrgDesigns.init(&player->mPID);
    dTime_c now = *dTime_c::getCurrent();
    player->mBankStamp.set(OSCalendarTimeToTicks(&now));
    dTime_c day = now;
    day.add(0, -6, 0, 0);
    player->mMotherMail.mLastDate.set(&day);
    player->setFlag0(PRIVATE_FLAG0_HOUSE_TOUR);
    player->setFlag0(PRIVATE_FLAG0_PART_TIME_JOB);
    player->setFlag0(PRIVATE_FLAG0_PLAYING_UNSAVED);
    if (now.month == MONTH_OCTOBER) {
        player->setFlag0(0x56);
    }
    if (now.month == MONTH_DECEMBER && now.mday >= 15 && now.mday <= 24) {
        player->setFlag0(0x84);
    }
    dSaveData_c::getTown()->mDaily.request();
}
// 801551E8
void dSvProc_c::initializeVisitor() {
    initializePlayer();
    dPlayerMgr::Mgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_JOINED_AS_VISITOR);
}
// 80155214
void dSvProc_c::initializeTown() {
    dSaveData_c::getTown()->mHeader.init();
    dSaveData_c::getExtra()->setupNew();
    dSaveData_c::getTown()->mTimeOffset.init();
    dSaveData_c::getTown()->mVisitorNpc.clear();
    dSaveData_c::getTown()->mHomes.initAll();
    dSaveData_c::getTown()->resetSaveTime();
    dSaveData_c::getTown()->mItemRarity.randomize();
    dSaveData_c::getTown()->setPublicWorksDays(0xFF);
    dSaveData_c::getTown()->mPoliceBox.init();
    dSaveData_c::getTown()->mRecycleBin.clear();
    dSaveData_c::getTown()->mShops.loadTailorDesigns();
    dSaveData_c::getTown()->mNpcSpotA.init();
    dSaveData_c::getTown()->mNpcSpotB.clear();
    dSaveData_c::getTown()->mHarvestSpot.clear();
    dSaveData_c::getTown()->mWeek.init();
    dSaveData_c::getTown()->mPlayerFlag.init();
    dSaveData_c::getTown()->mFishing.reset();
    dSaveData_c::getTown()->mBugOff.reset();
    dSaveData_c::getTown()->mDailyRand.init();
    dSaveData_c::getTown()->mBuilding.initTownFlag();
    dSaveData_c::getTown()->mNoticeBoard.init();
    dSaveData_c::getExtra()->mDesignBoard.clear();
    dFootmark::getEditor()->clearMarks();
}
