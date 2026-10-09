// The BGM manager (namespace dBgm). See include/game/game/d_bgm.hpp and notes/d_bgm.txt.
// .text 80077754..800805BC, .ctors 80465648, .rodata 8046D2C8..8046D428,
// .data 804A6CF0..804A7648, .bss 80582FB8..80583648, .sdata 80749CC0..80749E08,
// .sdata2 807504C8..807504F0.
#include <game/game/d_bgm.hpp>
#include <game/framework/f_manager.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_weather.hpp>
#include <game/mLib/m_fader.hpp>
#include <lib/egg/math/eggMath.h>
#include <revolution/MTX.h>

// Not split yet (C linkage keeps the target names).
extern "C" {
// The sound layer (around 8000ED00).
void fn_8000EDD0(const mVec3_c *src, mVec3_c *dst); // 8000EDD0
void fn_8000EF30();                                 // 8000EF30
void fn_8000EF98();                                 // 8000EF98
void fn_8000F000();                                 // 8000F000
void fn_8000F068();                                 // 8000F068
void fn_8000F0D0(u32 id, int fadeIn);               // 8000F0D0: start the BGM
void fn_8000F158(int fade);                         // 8000F158: stop the BGM
void fn_8000F1D0(int weather);                      // 8000F1D0
void fn_8000F248();                                 // 8000F248
void fn_8000F2B0();                                 // 8000F2B0
u32 fn_8000F318(int song);                          // 8000F318: a music player song's BGM id
BOOL fn_8000F390();                                 // 8000F390
u32 fn_8000F3F8(int song);                          // 8000F3F8: a K.K. song's BGM id
const s8 *fn_8000F470();                            // 8000F470
int fn_8000F4D8();                                  // 8000F4D8
void fn_8000F540(f32 volume, int frames);           // 8000F540
void fn_8000F6C0(const mVec3_c *pos);               // 8000F6C0
void fn_8000F738(int arg);                          // 8000F738
void fn_8000F7B0(int arg);                          // 8000F7B0
void fn_8000FD24();                                 // 8000FD24: the hourly chime

BOOL fn_800DCEDC();                // 800DCEDC: an online session is active
u32 fn_800DCF30();                 // 800DCF30
BOOL fn_800F98E4(const u16 *item); // 800F98E4
BOOL fn_8018A978(dCamera_c *camera); // 8018A978
BOOL fn_8018EF30();                // 8018EF30
}

namespace dBgm {

// 80077754: counts a timer down unless the process manager stops execution.
static int &countDown(int &timer) {
    if (fManager_c::isExecuteStopped()) {
        return timer;
    }
    timer--;
    return timer;
}

// ---------------------------------------------------------------------------------------------
// FieldEvent_c
// ---------------------------------------------------------------------------------------------

// 80749CC0..80749CC8
static dQuestEvent_e l_toyDay = EVENT_TOY_DAY;
static dQuestEvent_e l_countdown = EVENT_COUNTDOWN;
static dQuestEvent_e l_countdown2 = EVENT_COUNTDOWN;

FieldEvent_c::FieldEvent_c() {
    mEventBgm[0] = -1;
    mEventBgm[1] = -1;
    mEventBgm[2] = -1;
    mEventBgm[3] = -1;
    mToyDayBgm = -1;
    mCountdownBgm = -1;
    mCountdownMute = 0;
    mNewYearBgm = -1;
    mNewYearBgm2 = -1;
}

FieldEvent_c::~FieldEvent_c() {}

void FieldEvent_c::init() {}

void FieldEvent_c::execute() {
    if (l_mgr.mTime.isSecChanged()) {
        checkEvent(&mEventBgm[0], EVENT_HALLOWEEN, 0x1A89, 0x26);
        checkEvent(&mEventBgm[1], EVENT_FIREWORKS, 0x1A99, 0x26);
        checkEvent(&mEventBgm[2], EVENT_FESTIVALE, 0x1AB1, 0x26);
        checkEvent(&mEventBgm[3], EVENT_HARVEST_FESTIVAL, 0x1AB2, 0x26);
        checkToyDay();
        checkCountdown();
        checkNewYear();
    }
}

void FieldEvent_c::stop() {
    stopEvent(&mEventBgm[0]);
    stopEvent(&mEventBgm[1]);
    stopEvent(&mEventBgm[2]);
    stopEvent(&mEventBgm[3]);
    stopToyDay();
    stopCountdown();
    stopNewYear();
}

void FieldEvent_c::checkEvent(u32 *bgm, int event, u32 id, int prio) {
    dQuestEvent_e ev = (dQuestEvent_e)event;
    bool on = dEvent::isOngoing(ev);
    if (*bgm == -1) {
        if (on) {
            *bgm = id;
            l_mgr.play(prio, id, 0x2D, 0);
        }
    } else if (!on) {
        *bgm = -1;
        l_mgr.stop(id);
    }
}

void FieldEvent_c::stopEvent(u32 *bgm) {
    if (*bgm != -1) {
        l_mgr.stop(*bgm);
        *bgm = -1;
    }
}

void FieldEvent_c::checkToyDay() {
    Time_c *const time = &l_mgr.mTime;
    BOOL date = time->isDate(MONTH_DECEMBER, 24);
    BOOL morning = time->isTime(6, 0, 0, 0, 0, 0);
    bool active = dEvent::isActive(l_toyDay) != FALSE;
    bool on = (date && morning) != FALSE && active;
    if (mToyDayBgm == -1) {
        if (on) {
            mToyDayBgm = 0x1AB3;
            l_mgr.play(0x26, 0x1AB3, 0x2D, 0);
        }
    } else if (!on) {
        l_mgr.stop(mToyDayBgm);
        mToyDayBgm = -1;
    }
}

void FieldEvent_c::stopToyDay() {
    if (mToyDayBgm != -1) {
        l_mgr.stop(mToyDayBgm);
        mToyDayBgm = -1;
    }
}

void FieldEvent_c::checkCountdown() {
    if (dEvent::isOngoing(l_countdown)) {
        Time_c *time = &l_mgr.mTime;
        BOOL t0 = time->isTime(23, 0, 0, 23, 30, 0);
        BOOL t1 = time->isTime(23, 30, 0, 23, 50, 0);
        BOOL t2 = time->isTime(23, 50, 0, 23, 55, 0);
        BOOL t3 = time->isTime(23, 55, 0, 0, 0, 0);
        u32 id = -1;
        if (t0) {
            id = 0x1A9A;
        } else if (t1) {
            id = 0x1A9B;
        } else if (t2) {
            id = 0x1A9C;
        } else if (t3) {
            id = 0x1A9D;
        }
        if (mCountdownBgm != -1 && mCountdownBgm != id) {
            l_mgr.stop(mCountdownBgm);
            mCountdownBgm = -1;
        }
        if (mCountdownBgm != id) {
            l_mgr.play(0x24, id, 0x2D, 0);
            mCountdownBgm = id;
        }

        BOOL m0 = time->isTime(23, 29, 50, 23, 30, 0);
        BOOL m1 = time->isTime(23, 49, 50, 23, 50, 0);
        BOOL m2 = time->isTime(23, 54, 50, 23, 55, 0);
        BOOL m3 = time->isTime(23, 58, 50, 0, 0, 0);
        if (m0 || m1 || m2 || m3) {
            if (!mCountdownMute) {
                l_mgr.mute(0x23, 600, 0);
                mCountdownMute = 1;
            }
        } else if (mCountdownMute) {
            l_mgr.unmute(0x23);
            mCountdownMute = 0;
        }
    } else {
        stopCountdown();
    }
}

void FieldEvent_c::stopCountdown() {
    if (mCountdownBgm != -1) {
        l_mgr.stop(mCountdownBgm);
        mCountdownBgm = -1;
    }
    if (mCountdownMute) {
        l_mgr.unmute(0x23);
        mCountdownMute = 0;
    }
}

void FieldEvent_c::checkNewYear() {
    Time_c *time = &l_mgr.mTime;
    if (time->isDate(0, 1)) {
        if (mNewYearBgm == -1) {
            mNewYearBgm = 0x1AA0;
            l_mgr.play(0x26, 0x1AA0, 0x2D, 0);
        }
    } else if (mNewYearBgm != -1) {
        l_mgr.stop(mNewYearBgm);
        mNewYearBgm = -1;
    }

    if (dEvent::isOngoing(l_countdown2) && time->isTime(0, 0, 0, 2, 0, 0)) {
        if (mNewYearBgm2 == -1) {
            if (time->isYearChanged() && time->isMinSec(0, 0, 0, 1)) {
                mNewYearBgm2 = 0x1A9E;
            } else {
                mNewYearBgm2 = 0x1A9F;
            }
            l_mgr.play(0x22, mNewYearBgm2, 0x2D, 0);
        } else if (mNewYearBgm2 == 0x1A9E && !l_mgr.mQueue.isCurrent(mNewYearBgm2)) {
            l_mgr.stop(mNewYearBgm2);
            mNewYearBgm2 = -1;
        }
    } else if (mNewYearBgm2 != -1) {
        l_mgr.stop(mNewYearBgm2);
        mNewYearBgm2 = -1;
    }
}

void FieldEvent_c::stopNewYear() {
    if (mNewYearBgm != -1) {
        l_mgr.stop(mNewYearBgm);
        mNewYearBgm = -1;
    }
    if (mNewYearBgm2 != -1) {
        l_mgr.stop(mNewYearBgm2);
        mNewYearBgm2 = -1;
    }
}

// ---------------------------------------------------------------------------------------------
// StgBase_c / StgNon_c
// ---------------------------------------------------------------------------------------------

StgBase_c::StgBase_c() : mParam(0) {}

StgBase_c::~StgBase_c() {}

void StgBase_c::start(u32 param) {
    mParam = param;
}

void StgBase_c::execute() {}

void StgBase_c::end() {}

bool StgBase_c::isKeep(u32 nextParam) const {
    return false;
}

void StgBase_c::reset() {}

StgNon_c::StgNon_c() {}

StgNon_c::~StgNon_c() {}

// ---------------------------------------------------------------------------------------------
// StgField_c
// ---------------------------------------------------------------------------------------------

// 8046D2C8: the hourly town BGM.
static const u32 sHourBgm[24] = {
    0x1A11, 0x1A12, 0x1A13, 0x1A14, 0x1A15, 0x1A16, 0x1A17, 0x1A18,
    0x1A19, 0x1A1A, 0x1A1B, 0x1A1C, 0x1A1D, 0x1A1E, 0x1A1F, 0x1A20,
    0x1A21, 0x1A22, 0x1A23, 0x1A24, 0x1A25, 0x1A26, 0x1A27, 0x1A28,
};

StgField_c::StgField_c()
    : mHour(24), mHourBgm(-1), mChimeMute(0), mNoChime(0), mHomeBgm(-1), mBgm18(-1), mBgm1C(-1), mBgm20(-1),
      mMute24(0), m25(0), mBgm28(-1), mMute2C(0), mMute2D(0), m30(0), mBgm34(-1), mMute38(0), mMute39(0), m3A(0) {}

StgField_c::~StgField_c() {}

void StgField_c::start(u32 param) {
    StgBase_c::start(param);
    if (!l_mgr.mChange.mKeep) {
        mEvent.init();
    }
    m3A = 0;
}

void StgField_c::execute() {
    checkHour();
    checkChime();
    checkHome();
    mEvent.execute();
}

void StgField_c::end() {
    if (l_mgr.mChange.mKeep) {
        return;
    }
    stopAll();
}

bool StgField_c::isKeep(u32 nextParam) const {
    Stage_e stage = getStage();
    Stage_e next = getStage(nextParam);
    bool keep = false;
    if (stage == next && m3A) {
        keep = true;
    }
    return keep;
}

void StgField_c::reset() {
    m3A = 0;
    stopAll();
}

void StgField_c::play18() {
    mBgm18 = 0x1A78;
    l_mgr.play(4, 0x1A78, 0x2D, 1);
    l_mgr.mute(3, 0, 0xF);
    l_mgr.mState.mFlags |= State_c::FLAG_4;
}

void StgField_c::stop18() {
    stop18Bgm();
    l_mgr.mState.mFlags &= ~State_c::FLAG_4;
}

void StgField_c::fn_80078408() {}

void StgField_c::fn_8007840C() {
    if (!m25) {
        l_mgr.mute(0x1A, 0xB4, 0xF0);
    }
    stop1C();
}

void StgField_c::mute1C() {
    l_mgr.mute(0x1C, 0x1E, 0);
    mMute24 = 1;
}

void StgField_c::set25() {
    m25 = 1;
}

void StgField_c::play20() {
    mBgm20 = 0x1AB8;
    l_mgr.play(0xE, 0x1AB8, 0x2D, 1);
}

void StgField_c::play1C() {
    if (mBgm20 != -1) {
        l_mgr.stop(mBgm20);
        mBgm20 = -1;
    }
    if (mMute24) {
        l_mgr.unmute(0x1C);
        mMute24 = 0;
    }
    mBgm1C = 0x1A7B;
    l_mgr.play(0x1B, 0x1A7B, 0x2D, 1);
}

void StgField_c::play28() {
    mBgm28 = 0x1A98;
    l_mgr.play(0x1B, 0x1A98, 0x2D, 1);
    l_mgr.mute(0x1A, 0, 0xF);
}

void StgField_c::fn_800785C8() {
    l_mgr.mute(0x1A, 0xB4, 0xF0);
    stop28();
}

void StgField_c::mute2C() {
    l_mgr.mute(0x1C, 0, 0);
    mMute2C = 1;
}

void StgField_c::fn_80078658() {
    l_mgr.mute(0x1A, 0, 0x1E);
    unmute2C();
}

void StgField_c::mute2D() {
    l_mgr.mute(0x1C, 0, 0);
    mMute2D = 1;
}

void StgField_c::fn_800786E8() {
    l_mgr.mute(0x1A, 0, 0x1E);
    unmute2D();
}

void StgField_c::set30(int value) {
    m30 = value;
}

void StgField_c::play34() {
    l_mgr.mute(0x1C, 0x2D, 0);
    mMute39 = 1;
    if (m30 == 0) {
        mBgm34 = 0x1A93;
    } else {
        mBgm34 = 0x1AB0;
    }
    l_mgr.play(0x1B, mBgm34, 0x2D, 1);
}

void StgField_c::mute38() {
    l_mgr.mute(0x1A, 0, 0);
    mMute38 = 1;
}

void StgField_c::unmute38() {
    if (mMute38) {
        l_mgr.unmute(0x1A);
        mMute38 = 0;
    }
}

void StgField_c::fn_80078850() {
    l_mgr.mute(0x1A, 0x38, 0x56);
    stop34();
}

void StgField_c::set3A() {
    m3A = 1;
}

void StgField_c::setNoChime(bool noChime) {
    mNoChime = noChime;
}

void StgField_c::stopAll() {
    stopHour();
    mHour = 24;
    mNoChime = 0;
    unmuteChime();
    setHomeBgm(-1);
    stop18Bgm();
    stop1C();
    stop28();
    unmute2C();
    unmute2D();
    stop34();
    mEvent.stop();
}

void StgField_c::playHour() {
    if (mHour < 24) {
        u32 id = sHourBgm[mHour];
        l_mgr.playField(id);
        mHourBgm = id;
    }
}

void StgField_c::stopHour() {
    if (mHourBgm != -1) {
        l_mgr.stop(mHourBgm);
        mHourBgm = -1;
    }
}

void StgField_c::checkHour() {
    int hour = dTime_c::getCurrent()->hour;
    if (hour != mHour && hour < 24) {
        stopHour();
        mHour = hour;
        playHour();
    }
}

void StgField_c::muteChime() {
    if (!mChimeMute) {
        l_mgr.mute(0x20, 600, 0);
        mChimeMute = 1;
    }
}

void StgField_c::unmuteChime() {
    if (mChimeMute) {
        l_mgr.unmute(0x20);
        mChimeMute = 0;
    }
}

void StgField_c::checkChime() {
    Time_c *time = &l_mgr.mTime;
    BOOL chime = time->isMinSec(59, 50, 0, 16);
    if (time->isDateTime(11, 31, 23, 59) || time->isDateTime(0, 1, 0, 0)) {
        chime = FALSE;
    }
    if (mNoChime) {
        if (chime) {
            if (time->isMinSec(0, 0, 0, 16)) {
                chime = FALSE;
            }
        } else {
            mNoChime = 0;
        }
    }
    if (chime) {
        muteChime();
    } else {
        unmuteChime();
    }
}

// The current player's house in the town (NULL: none yet).
static inline dHome_c *getPlayerHome() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer());
}

void StgField_c::checkHome() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    u32 id = -1;
    if (player != NULL) {
        if ((!fn_800DCEDC() || fn_800DCF30() < 2) && getPlayerHome() == NULL && !player->isFlag0(0x28)) {
            id = 0x1A7A;
        } else if (player->isFlag0(0xD)) {
            id = 0x1A7C;
        }
    }
    if (mHomeBgm != id) {
        setHomeBgm(id);
    }
}

void StgField_c::setHomeBgm(u32 id) {
    if (mHomeBgm != id) {
        if (mHomeBgm != -1) {
            l_mgr.stop(mHomeBgm);
            mHomeBgm = -1;
        }
        if (id != -1) {
            l_mgr.play(0x21, id, 0x2D, 0);
            mHomeBgm = id;
        }
    }
}

void StgField_c::stop18Bgm() {
    if (mBgm18 != -1) {
        l_mgr.stop(mBgm18);
        mBgm18 = -1;
    }
}

void StgField_c::stop1C() {
    m25 = 0;
    if (mBgm1C != -1) {
        l_mgr.stop(mBgm1C);
        mBgm1C = -1;
    }
    if (mBgm20 != -1) {
        l_mgr.stop(mBgm20);
        mBgm20 = -1;
    }
    if (mMute24) {
        l_mgr.unmute(0x1C);
        mMute24 = 0;
    }
}

void StgField_c::stop28() {
    if (mBgm28 != -1) {
        l_mgr.stop(mBgm28);
        mBgm28 = -1;
    }
}

void StgField_c::unmute2C() {
    if (mMute2C) {
        l_mgr.unmute(0x1C);
        mMute2C = 0;
    }
}

void StgField_c::unmute2D() {
    if (mMute2D) {
        l_mgr.unmute(0x1C);
        mMute2D = 0;
    }
}

void StgField_c::stop34() {
    if (mBgm34 != -1) {
        l_mgr.stop(mBgm34);
        mBgm34 = -1;
    }
    if (mMute39) {
        l_mgr.unmute(0x1C);
        mMute39 = 0;
    }
    if (mMute38) {
        l_mgr.unmute(0x1A);
        mMute38 = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// StgRoom_c
// ---------------------------------------------------------------------------------------------

StgRoom_c::StgRoom_c() : mMusicBgm(-1), mHomeBgm(-1), mRoomBgm(-1), mMusicPos(mVec3_c::Zero), mMusicPosSet(0) {}

StgRoom_c::~StgRoom_c() {}

void StgRoom_c::start(u32 param) {
    StgBase_c::start(param);
    u32 type = getType();
    if (type == 1) {
        int room;
        int home = dHomeList_c::getHomeFromScene(&room, getCurrentScene());
        if (home != -1 && room == 3) {
            if (dSaveData_c::getRaw()->mHomes.getPlayerOfHome(home) != -1) {
                mHomeBgm = 0x1A7D;
                l_mgr.play(0x18, 0x1A7D, 0x2D, 0);
            }
        }
    }
    if (type == 0 || type - 2 <= 1) {
        u32 id = fn_8000F318(0);
        mRoomBgm = id;
        l_mgr.play(0x19, id, 0x2D, 0);
    }
}

void StgRoom_c::execute() {
    updateMusicPos();
}

void StgRoom_c::end() {
    if (mHomeBgm != -1) {
        l_mgr.stop(mHomeBgm);
        mHomeBgm = -1;
    }
    if (mRoomBgm != -1) {
        l_mgr.stop(mRoomBgm);
        mRoomBgm = -1;
    }
}

void StgRoom_c::playMusic(int song) {
    if (mMusicBgm != -1) {
        stopMusic();
    }
    u32 id = fn_8000F318(song);
    mMusicBgm = id;
    l_mgr.playRoom(id);
}

void StgRoom_c::stopMusic() {
    if (mMusicBgm != -1) {
        l_mgr.stop(mMusicBgm);
        mMusicBgm = -1;
    }
}

void StgRoom_c::setMusicPos(const mVec3_c *pos) {
    mMusicPosSet = 1;
    fn_8000EDD0(pos, &mMusicPos);
}

void StgRoom_c::fn_800792AC(int arg) {
    fn_8000F738(arg);
}

void StgRoom_c::updateMusicPos() {
    if (mMusicPosSet) {
        l_mgr.mPlayer.setPos(&mMusicPos);
        mMusicPosSet = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// StgOffice_c / StgChkp_c
// ---------------------------------------------------------------------------------------------

StgOffice_c::StgOffice_c() {
    mBgm = -1;
}

StgOffice_c::~StgOffice_c() {}

void StgOffice_c::start(u32 param) {
    StgBase_c::start(param);
    u16 item = 0x800A;
    if (fn_800F98E4(&item)) {
        mBgm = 0x1A79;
    } else {
        mBgm = 0x1A8C;
    }
    l_mgr.play(0x18, mBgm, 0x2D, 0);
}

void StgOffice_c::execute() {}

void StgOffice_c::end() {
    l_mgr.stop(mBgm);
    mBgm = -1;
}

StgChkp_c::StgChkp_c() {
    mBgm = -1;
}

StgChkp_c::~StgChkp_c() {}

void StgChkp_c::start(u32 param) {
    StgBase_c::start(param);
    if (!l_mgr.mChange.mKeep) {
        mBgm = 0x1A88;
        l_mgr.play(0x18, 0x1A88, 0x2D, 0);
    }
}

void StgChkp_c::execute() {}

void StgChkp_c::end() {
    if (!l_mgr.mChange.mKeep) {
        stopBgm();
    }
    l_mgr.mState.mFlags &= ~State_c::FLAG_10;
}

bool StgChkp_c::isKeep(u32 nextParam) const {
    Stage_e stage = getStage();
    Stage_e next = getStage(nextParam);
    bool keep = false;
    if (stage == next) {
        keep = true;
    }
    return keep;
}

void StgChkp_c::reset() {
    stopBgm();
}

void StgChkp_c::stopBgm() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
}

// ---------------------------------------------------------------------------------------------
// StgMuseum_c
// ---------------------------------------------------------------------------------------------

StgMuseum_c::StgMuseum_c() {
    mBgm = -1;
    mPrevParam = 0;
    mRoomChange = 0;
}

StgMuseum_c::~StgMuseum_c() {}

void StgMuseum_c::start(u32 param) {
    StgBase_c::start(param);
    if (l_mgr.mChange.mKeep) {
        checkRoomChange();
    } else {
        mBgm = 0x1A86;
        l_mgr.play(0x18, 0x1A86, 0x2D, 0);
    }
    if (getType() == 2) {
        l_mgr.mVolCtrl.mStage.start();
    }
    mPrevParam = param;
}

void StgMuseum_c::execute() {
    updateRoomChange();
}

void StgMuseum_c::end() {
    if (getType() == 2) {
        l_mgr.mVolCtrl.mStage.end();
    }
    if (!l_mgr.mChange.mKeep) {
        stopBgm();
    }
}

bool StgMuseum_c::isKeep(u32 nextParam) const {
    Stage_e stage = getStage();
    Stage_e next = getStage(nextParam);
    bool keep = false;
    if (stage == next) {
        keep = true;
        changeRoom(nextParam);
    }
    return keep;
}

void StgMuseum_c::reset() {
    stopBgm();
}

void StgMuseum_c::stopBgm() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
    mPrevParam = 0;
    mRoomChange = 0;
}

BOOL StgMuseum_c::isSameMusic(int type, int otherType) {
    BOOL same = FALSE;
    if (type == 0) {
        same = TRUE;
    } else if (type == 1) {
        same = TRUE;
    } else if (type == 2) {
        same = TRUE;
    } else if (type == 3) {
        if (otherType == 0) {
            same = TRUE;
        }
    } else if (type == 4) {
        if (otherType == 0) {
            same = TRUE;
        }
    } else if (type == 5) {
        same = TRUE;
    }
    return same;
}

void StgMuseum_c::checkRoomChange() {
    if (isSameMusic((mPrevParam >> 16) & 0xF, getType())) {
        mRoomChange = 1;
    }
}

void StgMuseum_c::updateRoomChange() {
    if (mRoomChange) {
        if (mFader_c::isStatus(mFaderBase_c::FADE_IN) || mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
            fn_8000F2B0();
            mRoomChange = 0;
        }
    }
}

void StgMuseum_c::changeRoom(u32 nextParam) const {
    if (isSameMusic(getType(), (nextParam >> 16) & 0xF)) {
        fn_8000F248();
    }
}

// ---------------------------------------------------------------------------------------------
// StgCafe_c
// ---------------------------------------------------------------------------------------------

StgCafe_c::StgCafe_c() {
    mKKBgm = -1;
    mCafeMute = 0;
    mBgm10 = -1;
    mKKMute = 0;
}

StgCafe_c::~StgCafe_c() {}

void StgCafe_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1A87, 0x2D, 0);
}

void StgCafe_c::execute() {}

void StgCafe_c::end() {
    l_mgr.stop(0x1A87);
    if (mBgm10 != -1) {
        l_mgr.stop(mBgm10);
        mBgm10 = -1;
    }
    if (mKKMute) {
        l_mgr.unmute(0x14);
        mKKMute = 0;
    }
}

void StgCafe_c::playKK(int song, int fadeIn, u8 arg) {
    if (mKKBgm != -1) {
        stopKK();
    }
    u32 id = fn_8000F3F8(song);
    mKKBgm = id;
    l_mgr.playKK(id, fadeIn, arg);
}

void StgCafe_c::stopKK() {
    if (mKKBgm != -1) {
        l_mgr.stop(mKKBgm);
        mKKBgm = -1;
    }
}

BOOL StgCafe_c::isKKStopped() {
    return fn_8000F390() == 0;
}

void StgCafe_c::muteCafe() {
    if (mCafeMute) {
        unmuteCafe();
    }
    l_mgr.mute(0x17, 0x2D, 0);
    mCafeMute = 1;
}

void StgCafe_c::unmuteCafe() {
    if (mCafeMute) {
        l_mgr.unmute(0x17);
        mCafeMute = 0;
    }
}

BOOL StgCafe_c::isKKPlaying() const {
    BOOL playing = FALSE;
    if (mKKBgm != -1 && l_mgr.mQueue.isCurrent(mKKBgm)) {
        playing = TRUE;
    }
    return playing;
}

int StgCafe_c::getKKInfo0() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[0] : -1;
}

int StgCafe_c::getKKInfo1() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[1] : -1;
}

int StgCafe_c::getKKInfo2() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[2] : -1;
}

int StgCafe_c::getKKInfo3() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[3] : -1;
}

int StgCafe_c::getKKInfo4() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[4] : -1;
}

f32 StgCafe_c::getKKInfo8() {
    const s8 *info = fn_8000F470();
    return info != NULL ? *(const f32 *)(info + 8) : 0.0f;
}

f32 StgCafe_c::getKKInfoC() {
    const s8 *info = fn_8000F470();
    return info != NULL ? *(const f32 *)(info + 0xC) : 0.0f;
}

f32 StgCafe_c::getKKInfo10() {
    const s8 *info = fn_8000F470();
    return info != NULL ? *(const f32 *)(info + 0x10) : 0.0f;
}

int StgCafe_c::getKKInfo5() {
    const s8 *info = fn_8000F470();
    return info != NULL ? info[5] : -1;
}

int StgCafe_c::fn_80079F30() {
    int result = 0;
    if (isKKPlaying()) {
        result = fn_8000F4D8();
    }
    return result;
}

void StgCafe_c::muteKK() {
    l_mgr.mute(0x14, 0x3C, 0);
    mKKMute = 1;
}

void StgCafe_c::play1A92(int fade) {
    if (!isKKPlaying()) {
        mBgm10 = 0x1A92;
        l_mgr.play(0x13, 0x1A92, 0x2D, 1);
    }
    if (fade > 0) {
        l_mgr.mute(0x12, 0, fade);
    }
}

void StgCafe_c::stop1A92(int fadeArg, int fade) {
    if (mBgm10 != -1) {
        l_mgr.stop(mBgm10);
        mBgm10 = -1;
    }
    if (mKKMute) {
        l_mgr.unmute(0x14);
        mKKMute = 0;
    }
    if (fade > 0) {
        l_mgr.mute(0x12, fadeArg, fade);
    }
}

// ---------------------------------------------------------------------------------------------
// StgShop_c
// ---------------------------------------------------------------------------------------------

// 8046D328: per store type, the closing music window (from 10 s before the closing chime to
// just after closing time) and the chime itself.
static const ShopTime_s sShopTime[4] = {
    {21, 49, 50, 22, 0, 1, 21, 50, 0},
    {0, 49, 50, 1, 0, 1, 0, 50, 0},
    {21, 49, 50, 22, 0, 1, 21, 50, 0},
    {20, 49, 50, 21, 0, 1, 20, 50, 0},
};

// 8046D3B8
static const u32 sShopBgm[4] = {0x1A7F, 0x1A80, 0x1A81, 0x1A82};

StgShop_c::StgShop_c() {
    mType = 0;
    mBgm = -1;
    mClosingBgm = -1;
    mChimeMute = 0;
}

StgShop_c::~StgShop_c() {}

void StgShop_c::start(u32 param) {
    StgBase_c::start(param);
    mType = getType();
    const ShopTime_s *time = &sShopTime[mType];
    mBellTime = l_mgr.mTime.isTime(time->mStartH, time->mStartM, time->mStartS, time->mChimeH, time->mChimeM,
                                   time->mChimeS);
    if (!l_mgr.mChange.mKeep) {
        mBgm = sShopBgm[mType];
        l_mgr.play(0x18, mBgm, 0x2D, 0);
    }
}

void StgShop_c::execute() {
    Time_c *now = &l_mgr.mTime;
    const ShopTime_s *time = &sShopTime[mType];
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL flag = FALSE;
    if (player != NULL && player->isFlag0(0xD)) {
        flag = TRUE;
    }
    if (!flag && (now->isTime(time->mStartH, time->mStartM, time->mStartS, time->mCloseH, time->mCloseM,
                                     time->mCloseS) ||
                  !dSaveData_c::getTown()->mShops.mShop.isOpen())) {
        playClosing();
    } else {
        stopClosing();
    }
    if (!mBellTime && now->isTime(time->mStartH, time->mStartM, time->mStartS, time->mChimeH, time->mChimeM,
                                         time->mChimeS)) {
        muteBell();
    } else {
        unmuteBell();
    }
}

void StgShop_c::end() {
    if (l_mgr.mChange.mKeep) {
        return;
    }
    stopAll();
}

bool StgShop_c::isKeep(u32 nextParam) const {
    Stage_e stage = getStage();
    Stage_e next = getStage(nextParam);
    bool keep = false;
    if (stage == next) {
        u32 type = getType();
        u32 nextType = getType(nextParam);
        if (type == 3 && nextType == 3) {
            keep = true;
        }
    }
    return keep;
}

void StgShop_c::reset() {
    stopAll();
}

void StgShop_c::stopAll() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
    stopClosing();
    unmuteBell();
}

void StgShop_c::playClosing() {
    if (mClosingBgm != -1) {
        return;
    }
    mClosingBgm = 0x1A83;
    l_mgr.play(0x11, 0x1A83, 0x2D, 0);
}

void StgShop_c::stopClosing() {
    if (mClosingBgm != -1) {
        l_mgr.stop(mClosingBgm);
        mClosingBgm = -1;
    }
}

void StgShop_c::muteBell() {
    if (mChimeMute) {
        return;
    }
    mChimeMute = 1;
    l_mgr.mute(0x10, 600, 0);
}

void StgShop_c::unmuteBell() {
    if (!mChimeMute) {
        return;
    }
    mChimeMute = 0;
    l_mgr.unmute(0x10);
}

// ---------------------------------------------------------------------------------------------
// StgTailor_c
// ---------------------------------------------------------------------------------------------

StgTailor_c::StgTailor_c() {}

StgTailor_c::~StgTailor_c() {}

void StgTailor_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1A85, 0x2D, 0);
}

void StgTailor_c::execute() {}

void StgTailor_c::end() {
    l_mgr.stop(0x1A85);
}

// ---------------------------------------------------------------------------------------------
// StgTown_c
// ---------------------------------------------------------------------------------------------

StgTown_c::StgTown_c()
    : mBgm(-1), mChimeMute(0), mNoChime(0), mMusicPos(mVec3_c::Zero), mMusicPosSet(0), mBgm20(-1) {}

StgTown_c::~StgTown_c() {}

void StgTown_c::start(u32 param) {
    StgBase_c::start(param);
}

void StgTown_c::execute() {
    checkHour();
    checkChime();
    checkBgm20();
    updateMusicPos();
}

void StgTown_c::end() {
    stopTown();
    mNoChime = 0;
    unmuteChime();
    stopBgm20();
    mMusicPosSet = 0;
}

void StgTown_c::play1AB7() {
    playBgm20();
    fn_8007A7DC();
}

void StgTown_c::setMusicPos(const mVec3_c *pos) {
    mMusicPosSet = 1;
    fn_8000EDD0(pos, &mMusicPos);
}

void StgTown_c::fn_8007A790() {
    if (getSceneParent() != NULL && getCurrentScene() == 0x2D) {
        l_mgr.mState.mFlags &= ~State_c::FLAG_8;
    }
}

void StgTown_c::fn_8007A7DC() {
    l_mgr.mState.mFlags |= State_c::FLAG_8;
}

void StgTown_c::setNoChime(bool noChime) {
    mNoChime = noChime;
}

void StgTown_c::playTown(u32 id) {
    l_mgr.playTown(id);
    mBgm = id;
}

void StgTown_c::stopTown() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
}

void StgTown_c::checkHour() {
    int hour = dTime_c::getCurrent()->hour;
    u32 id;
    if (hour < 4) {
        id = 0x1AA6;
    } else if (hour < 9) {
        id = 0x1AA3;
    } else if (hour < 19) {
        id = 0x1AA4;
    } else {
        id = 0x1AA5;
    }
    if (id != mBgm) {
        stopTown();
        playTown(id);
    }
}

void StgTown_c::muteChime() {
    if (!mChimeMute) {
        l_mgr.mute(0x20, 600, 0);
        mChimeMute = 1;
    }
}

void StgTown_c::unmuteChime() {
    if (mChimeMute) {
        l_mgr.unmute(0x20);
        mChimeMute = 0;
    }
}

void StgTown_c::checkChime() {
    Time_c *time = &l_mgr.mTime;
    BOOL chime = time->isMinSec(59, 50, 0, 16);
    if (mNoChime) {
        if (chime) {
            if (time->isMinSec(0, 0, 0, 16)) {
                chime = FALSE;
            }
        } else {
            mNoChime = 0;
        }
    }
    if (chime) {
        muteChime();
    } else {
        unmuteChime();
    }
}

void StgTown_c::playBgm20() {
    mBgm20 = 0x1AB7;
    l_mgr.play(4, 0x1AB7, 0x2D, 0);
    l_mgr.mute(3, 0, 0x3C);
}

void StgTown_c::stopBgm20() {
    if (mBgm20 != -1) {
        l_mgr.stop(mBgm20);
        mBgm20 = -1;
    }
}

void StgTown_c::checkBgm20() {
    if (mBgm20 != -1 && l_mgr.mQueue.isCurrent(mBgm20) && !fn_8000F390()) {
        stopBgm20();
    }
}

void StgTown_c::updateMusicPos() {
    if (mMusicPosSet) {
        l_mgr.mPlayer.setPos(&mMusicPos);
        mMusicPosSet = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// StgBroker_c / StgBarber_c / StgFortune_c / StgGrace_c / StgHappy_c
// ---------------------------------------------------------------------------------------------

StgBroker_c::StgBroker_c() {}

StgBroker_c::~StgBroker_c() {}

void StgBroker_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1A8A, 0x2D, 0);
}

void StgBroker_c::execute() {}

void StgBroker_c::end() {
    l_mgr.stop(0x1A8A);
}

StgBarber_c::StgBarber_c() {
    mMute = 0;
}

StgBarber_c::~StgBarber_c() {}

void StgBarber_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1A84, 0x2D, 0);
}

void StgBarber_c::execute() {}

void StgBarber_c::end() {
    l_mgr.stop(0x1A84);
    unmute();
}

void StgBarber_c::mute() {
    mMute = 1;
    l_mgr.mute(0xF, 0xD2, 0);
}

void StgBarber_c::fn_8007ADF4() {
    l_mgr.mute(0xD, 0, 0x3C);
    unmute();
}

void StgBarber_c::unmute() {
    if (mMute) {
        l_mgr.unmute(0xF);
        mMute = 0;
    }
}

StgFortune_c::StgFortune_c() {
    mMute = 0;
}

StgFortune_c::~StgFortune_c() {}

void StgFortune_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1A8B, 0x2D, 1);
}

void StgFortune_c::execute() {}

void StgFortune_c::end() {
    l_mgr.stop(0x1A8B);
    unmute();
}

void StgFortune_c::mute() {
    mMute = 1;
    l_mgr.mute(0xF, 0, 0);
}

void StgFortune_c::unmute2() {
    unmute();
}

void StgFortune_c::unmute() {
    if (mMute) {
        l_mgr.unmute(0xF);
        mMute = 0;
    }
}

StgGrace_c::StgGrace_c() {}

StgGrace_c::~StgGrace_c() {}

void StgGrace_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1AAC, 0x2D, 0);
}

void StgGrace_c::execute() {}

void StgGrace_c::end() {
    l_mgr.stop(0x1AAC);
}

StgHappy_c::StgHappy_c() {}

StgHappy_c::~StgHappy_c() {}

void StgHappy_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1AAB, 0x2D, 0);
}

void StgHappy_c::execute() {}

void StgHappy_c::end() {
    l_mgr.stop(0x1AAB);
}

// ---------------------------------------------------------------------------------------------
// StgTheater_c
// ---------------------------------------------------------------------------------------------

StgTheater_c::StgTheater_c() {
    mBgm = -1;
    mShowType = 0;
    mShowBgm = -1;
    mShowEndBgm = -1;
    mMute = 0;
}

StgTheater_c::~StgTheater_c() {}

void StgTheater_c::start(u32 param) {
    StgBase_c::start(param);
    if (!l_mgr.mChange.mKeep) {
        mBgm = 0x1AAD;
        l_mgr.play(0x18, 0x1AAD, 0x2D, 0);
    }
}

void StgTheater_c::execute() {
    checkShowEnd();
}

void StgTheater_c::end() {
    if (!l_mgr.mChange.mKeep) {
        stopBgm();
    }
    if (mShowBgm != -1) {
        l_mgr.stop(mShowBgm);
        mShowBgm = -1;
    }
    if (mShowEndBgm != -1) {
        l_mgr.stop(mShowEndBgm);
        mShowEndBgm = -1;
    }
    if (mMute) {
        l_mgr.unmute(0x17);
        mMute = 0;
    }
}

bool StgTheater_c::isKeep(u32 nextParam) const {
    Stage_e stage = getStage();
    Stage_e next = getStage(nextParam);
    VolStateStage_c *show = &l_mgr.mVolCtrl.mStage;
    bool keep = stage == next;
    if (keep) {
        if (getType() == 0) {
            show->startShow();
        } else {
            show->endShow();
        }
    } else {
        show->cancelShow();
    }
    return keep;
}

void StgTheater_c::reset() {
    stopBgm();
}

void StgTheater_c::mute() {
    l_mgr.mute(0x17, 0x3C, 0);
    mMute = 1;
}

void StgTheater_c::unmute() {
    if (mMute) {
        l_mgr.unmute(0x17);
        mMute = 0;
    }
}

void StgTheater_c::playShow(int type) {
    mShowType = type;
    if (type == 0) {
        mShowBgm = 0x1AA7;
    } else {
        mShowBgm = 0x1AAE;
    }
    l_mgr.play(0x11, mShowBgm, 10, 1);
}

void StgTheater_c::stopShow() {
    l_mgr.stop(mShowBgm);
    mShowBgm = -1;
}

void StgTheater_c::playShowEnd() {
    if (mShowType == 0) {
        mShowEndBgm = 0x1AA8;
    } else {
        mShowEndBgm = 0x1AAF;
    }
    l_mgr.play(0x11, mShowEndBgm, 0, 1);
}

void StgTheater_c::playEncore() {
    l_mgr.mute(0x15, 0, 0x61);
    mShowBgm = 0x1AA9;
    l_mgr.play(0x16, 0x1AA9, 10, 1);
}

void StgTheater_c::playEncoreEnd() {
    l_mgr.stop(mShowBgm);
    mShowBgm = -1;
    mShowEndBgm = 0x1AAA;
    l_mgr.play(0x11, 0x1AAA, 0, 1);
}

void StgTheater_c::checkShowEnd() {
    if (mShowEndBgm != -1 && l_mgr.mQueue.isCurrent(mShowEndBgm) && !fn_8000F390()) {
        l_mgr.stop(mShowEndBgm);
        mShowEndBgm = -1;
    }
}

void StgTheater_c::stopBgm() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
}

// ---------------------------------------------------------------------------------------------
// StgAuction_c / StgReset_c
// ---------------------------------------------------------------------------------------------

StgAuction_c::StgAuction_c() {}

StgAuction_c::~StgAuction_c() {}

void StgAuction_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1AA1, 0x2D, 0);
}

void StgAuction_c::execute() {}

void StgAuction_c::end() {
    l_mgr.stop(0x1AA1);
}

StgReset_c::StgReset_c() {
    mBgm = -1;
}

StgReset_c::~StgReset_c() {}

void StgReset_c::start(u32 param) {
    StgBase_c::start(param);
}

void StgReset_c::execute() {}

void StgReset_c::end() {
    stop();
}

void StgReset_c::play() {
    mBgm = 0x1AB4;
    l_mgr.play(0x11, 0x1AB4, 0x14, 1);
}

void StgReset_c::stop2() {
    stop();
}

void StgReset_c::stop() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
}

// ---------------------------------------------------------------------------------------------
// StgTitle_c
// ---------------------------------------------------------------------------------------------

StgTitle_c::StgTitle_c() {
    mBgm = -1;
    mMute = 0;
}

StgTitle_c::~StgTitle_c() {}

void StgTitle_c::start(u32 param) {
    StgBase_c::start(param);
}

void StgTitle_c::execute() {
    updatePlay();
}

void StgTitle_c::end() {}

void StgTitle_c::requestPlay() {
    mPlayReq = 1;
}

void StgTitle_c::stop() {
    if (mBgm != -1) {
        l_mgr.stop(mBgm);
        mBgm = -1;
    }
    mPlayReq = 0;
}

void StgTitle_c::mute() {
    mMute = 1;
    l_mgr.mute(1, 0x1E, 0);
}

void StgTitle_c::unmute() {
    if (mMute) {
        l_mgr.unmute(1);
        mMute = 0;
    }
}

void StgTitle_c::updatePlay() {
    if (mPlayReq) {
        if (mFader_c::isStatus(mFaderBase_c::FADE_IN) || mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
            mBgm = 0x19C3;
            l_mgr.play(2, 0x19C3, 0x2D, 0);
            mPlayReq = 0;
        }
    }
}

BOOL StgTitle_c::isEnd() const {
    BOOL end = FALSE;
    if (mBgm != -1 && l_mgr.mQueue.isCurrent(mBgm) && !fn_8000F390()) {
        end = TRUE;
    }
    return end;
}

// ---------------------------------------------------------------------------------------------
// StgSave_c / StgLoad_c / StgPlSel_c / StgBus_c
// ---------------------------------------------------------------------------------------------

StgSave_c::StgSave_c() {}

StgSave_c::~StgSave_c() {}

void StgSave_c::start(u32 param) {
    StgBase_c::start(param);
}

void StgSave_c::execute() {}

void StgSave_c::end() {}

StgLoad_c::StgLoad_c() {}

StgLoad_c::~StgLoad_c() {}

void StgLoad_c::start(u32 param) {
    StgBase_c::start(param);
}

void StgLoad_c::execute() {}

void StgLoad_c::end() {}

StgPlSel_c::StgPlSel_c() {
    mMute = 0;
}

StgPlSel_c::~StgPlSel_c() {}

void StgPlSel_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1AB5, 0x2D, 1);
}

void StgPlSel_c::execute() {}

void StgPlSel_c::end() {
    l_mgr.stop(0x1AB5);
    unmute();
}

void StgPlSel_c::unmute() {
    if (mMute) {
        l_mgr.unmute(0x17);
        mMute = 0;
    }
}

StgBus_c::StgBus_c() {
    mMute = 0;
    mMuteReq = 0;
}

StgBus_c::~StgBus_c() {}

void StgBus_c::start(u32 param) {
    StgBase_c::start(param);
    l_mgr.play(0x18, 0x1AB6, 0x2D, 1);
    mMuteReq = 0;
    mMute = 0;
}

void StgBus_c::execute() {
    if (mMuteReq == mMute) {
        return;
    }
    if (mMuteReq) {
        mute();
    } else {
        unmute();
    }
}

void StgBus_c::end() {
    l_mgr.stop(0x1AB6);
    mMuteReq = 0;
    unmute();
}

void StgBus_c::setMute(BOOL on) {
    mMuteReq = on == FALSE;
}

void StgBus_c::fn_8007BF90(int arg) {
    fn_8000F7B0(arg);
}

void StgBus_c::mute() {
    l_mgr.mute(0x17, 5, 0);
    mMute = 1;
}

void StgBus_c::unmute() {
    if (mMute) {
        l_mgr.unmute(0x17);
        mMute = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// Time_c
// ---------------------------------------------------------------------------------------------

Time_c::Time_c() : mNow(*dTime_c::getCurrent()), mPrev(*dTime_c::getCurrent()) {}

Time_c::~Time_c() {}

void Time_c::init() {
    mNow = *dTime_c::getCurrent();
    mPrev = *dTime_c::getCurrent();
}

void Time_c::update() {
    mPrev = mNow;
    mNow = *dTime_c::getCurrent();
}

BOOL Time_c::isDateTime(int month, u32 day, int hour, u32 min) const {
    BOOL result = FALSE;
    if (month == mNow.month && day == mNow.mday && hour == mNow.hour && min == mNow.min) {
        result = TRUE;
    }
    return result;
}

BOOL Time_c::isDate(int month, u32 day) const {
    BOOL result = FALSE;
    if (month == mNow.month && day == mNow.mday) {
        result = TRUE;
    }
    return result;
}

BOOL Time_c::isMinSec(int min0, int sec0, int min1, int sec1) const {
    BOOL result = FALSE;
    u32 now = mNow.min * 60 + mNow.sec;
    u32 start = min0 * 60 + sec0;
    u32 end = min1 * 60 + sec1;
    if (start <= end) {
        if (now >= start && now < end) {
            result = TRUE;
        }
    } else if (now >= start || now < end) {
        result = TRUE;
    }
    return result;
}

BOOL Time_c::isTime(int h0, int m0, int s0, int h1, int m1, int s1) const {
    BOOL result = FALSE;
    u32 now = mNow.hour * 3600 + mNow.min * 60 + mNow.sec;
    u32 start = h0 * 3600 + m0 * 60 + s0;
    u32 end = h1 * 3600 + m1 * 60 + s1;
    if (start <= end) {
        if (now >= start && now < end) {
            result = TRUE;
        }
    } else if (now >= start || now < end) {
        result = TRUE;
    }
    return result;
}

BOOL Time_c::isSecChanged() const {
    return mPrev.sec != mNow.sec;
}

BOOL Time_c::isHourChanged() const {
    return mPrev.hour != mNow.hour;
}

BOOL Time_c::isYearChanged() const {
    return mPrev.year != mNow.year;
}

void Time_c::print() {}

// ---------------------------------------------------------------------------------------------
// State_c
// ---------------------------------------------------------------------------------------------

State_c::State_c() {
    mFlags = 0;
    mMode = 0;
    mState = 0;
    mTimer = 0;
}

State_c::~State_c() {}

void State_c::init() {
    mFlags = 0;
    mMode = 0;
    mState = 0;
    mTimer = 0;
}

void State_c::start() {
    setMode();
}

void State_c::execute() {
    executeState();
    checkChime();
}

typedef void (State_c::*StateFunc)();

// 8046D3C8
static const StateFunc sStateFuncs[5] = {
    &State_c::stateWaitFader,
    &State_c::stateCheckScene,
    &State_c::stateWaitFader2,
    &State_c::stateWait,
    &State_c::stateCheckLeave,
};

void State_c::executeState() {
    (this->*sStateFuncs[mState])();
}

void State_c::stateWaitFader() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mState = 1;
    }
}

void State_c::stateCheckScene() {
    u8 scene = getCurrentScene();
    u8 prev = getPrevScene();
    bool enter = false;
    if (scene == 0) {
        if (prev == 0x3C || prev == 0x3E) {
            enter = true;
        }
    } else if (scene == 0x2D && prev == 0x3D) {
        enter = true;
    }
    if (enter) {
        mFlags |= FLAG_ENTER;
        mState = 2;
    }
}

void State_c::stateWaitFader2() {
    if (mFader_c::isStatus(mFaderBase_c::HIDDEN)) {
        mState = 3;
        mTimer = 2;
    }
}

void State_c::stateWait() {
    if (countDown(mTimer) <= 0) {
        mFlags &= ~FLAG_ENTER;
        mState = 4;
    }
}

void State_c::stateCheckLeave() {
    u8 scene = getCurrentScene();
    if (scene != 0 && scene != 0x2D) {
        mState = 0;
    }
}

void State_c::setMode() {
    if (l_mgr.mStage == NULL) {
        return;
    }
    mMode = (l_mgr.mStage->mParam >> 8) & 3;
}

void State_c::checkChime() {
    Time_c *time = &l_mgr.mTime;
    if (time->isHourChanged() && time->isMinSec(0, 0, 0, 1)) {
        BOOL chime = TRUE;
        if (mMode == 0) {
            chime = FALSE;
        }
        if (mFlags != 0) {
            chime = FALSE;
        }
        if (chime) {
            fn_8000FD24();
        }
        bool noChime = !chime;
        l_mgr.mStgField.setNoChime(noChime);
        l_mgr.mStgTown.setNoChime(noChime);
    }
}

// ---------------------------------------------------------------------------------------------
// Change_c / Silence_c
// ---------------------------------------------------------------------------------------------

Change_c::Change_c() {
    mState = 0;
    mTimer = 0;
    mKeep = 0;
}

Change_c::~Change_c() {}

void Change_c::init() {
    mState = 0;
    mTimer = 0;
    mKeep = 0;
}

void Change_c::execute() {
    if (mState == 2) {
        if (mFader_c::isStatus(mFaderBase_c::HIDDEN) && fn_800FF840()) {
            mTimer = 5;
            mState = 3;
        }
    } else if (mState == 3 && countDown(mTimer) <= 0) {
        mState = 0;
        l_mgr.unmute(8);
        l_mgr.mStgTown.fn_8007A790();
    }
}

void Change_c::start(int fade) {
    StgBase_c *stage = l_mgr.mStage;
    u8 next = getNextScene();
    if (stage != NULL && next < 0x44) {
        mKeep = stage->isKeep(getSceneParamC(next));
    }
    if (!mKeep) {
        if (mState != 0) {
            l_mgr.unmute(8);
        }
        l_mgr.mute(8, fade, 0);
        mState = 1;
        mTimer = 0;
    }
    l_mgr.mVolCtrl.mTalk.mKeep = 0;
}

void Change_c::end() {
    if (mState == 1) {
        mState = 2;
    }
    mKeep = 0;
}

Silence_c::Silence_c() {
    mOn = 0;
}

Silence_c::~Silence_c() {}

void Silence_c::init() {
    mOn = 0;
}

void Silence_c::execute() {
    if (fn_8018EF30()) {
        if (!mOn) {
            mOn = 1;
            l_mgr.mute(9, 0x2D, 0);
        }
    } else if (mOn) {
        mOn = 0;
        l_mgr.unmute(9);
    }
}

// ---------------------------------------------------------------------------------------------
// Event_c
// ---------------------------------------------------------------------------------------------

// 8046D404
static const u32 sEventBgm[5] = {0x1A8E, 0x1A8F, 0x1A90, 0x1A91, 0x1A8F};

Event_c::Event_c() {
    mBgm0 = -1;
    mMute4 = 0;
    mType = 0;
    mBgmC = -1;
    mMute10 = 0;
    mBgm14 = -1;
    mMute18 = 0;
    mBgm1C = -1;
    mMute20 = 0;
    mBgm24 = -1;
    mMute28 = 0;
    mMute29 = 0;
    mBgm2C = -1;
    mMute30 = 0;
}

Event_c::~Event_c() {}

void Event_c::fn_8007CBB0() {}

void Event_c::stopAll() {
    stopBgm0();
    stopBgmC();
    stopBgm14();
    unmute29();
    stopBgm2C();
}

void Event_c::init() {
    mBgm0 = -1;
    mMute4 = 0;
    mType = 0;
    mBgmC = -1;
    mMute10 = 0;
    mBgm14 = -1;
    mMute18 = 0;
    mBgm1C = -1;
    mMute20 = 0;
    mBgm24 = -1;
    mMute28 = 0;
    mMute29 = 0;
    mBgm2C = -1;
    mMute30 = 0;
}

void Event_c::execute() {}

void Event_c::mute0() {
    l_mgr.mute(0xF, 0, 0);
    mMute4 = 1;
}

void Event_c::play0(int fade) {
    mBgm0 = 0x1A8D;
    l_mgr.play(0xE, 0x1A8D, 0x2D, 1);
    if (fade > 0) {
        l_mgr.mute(0xD, 0, fade);
    }
}

void Event_c::stop0(int fadeArg, int fade) {
    stopBgm0();
    if (fade > 0) {
        l_mgr.mute(0xD, fadeArg, fade);
    }
}

void Event_c::muteC(int type) {
    mType = type;
    l_mgr.mute(0xC, 0, 0);
    mMute10 = 1;
}

void Event_c::playC(int fade) {
    u32 id = sEventBgm[mType];
    mBgmC = id;
    l_mgr.play(0xB, id, 0x2D, 1);
    if (fade > 0) {
        l_mgr.mute(10, 0, fade);
    }
}

void Event_c::stopC(int fadeArg, int fade) {
    stopBgmC();
    if (fade > 0) {
        l_mgr.mute(10, fadeArg, fade);
    }
}

void Event_c::mute18() {
    l_mgr.mute(0x1F, 0, 0);
    mMute18 = 1;
}

void Event_c::play14() {
    mBgm14 = 0x1A94;
    l_mgr.play(0x1E, 0x1A94, 0x2D, 0);
}

void Event_c::stop14() {
    if (mBgm14 != -1) {
        l_mgr.stop(mBgm14);
        mBgm14 = -1;
    }
    if (mMute18) {
        l_mgr.unmute(0x1F);
        mMute18 = 0;
    }
    l_mgr.mute(0x1D, 0, 0x82);
}

void Event_c::mute20() {
    if (mBgm14 != -1) {
        l_mgr.stop(mBgm14);
        mBgm14 = -1;
    }
    if (mMute18) {
        l_mgr.unmute(0x1F);
        mMute18 = 0;
    }
    l_mgr.mute(0xF, 0, 0);
    mMute20 = 1;
}

void Event_c::play1C() {
    mBgm1C = 0x1A95;
    l_mgr.play(0xE, 0x1A95, 0x2D, 1);
}

void Event_c::stop1C() {
    stopBgm14();
    l_mgr.mute(0xD, 0xB, 0x1A);
}

void Event_c::mute28() {
    mMute28 = 1;
    l_mgr.mute(7, 0, 0);
}

void Event_c::play24() {
    mBgm24 = 0x1A96;
    l_mgr.play(6, 0x1A96, 0x2D, 1);
}

void Event_c::stop24() {
    if (mBgm24 != -1) {
        l_mgr.stop(mBgm24);
        mBgm24 = -1;
    }
    if (mMute28) {
        l_mgr.unmute(7);
        mMute28 = 0;
    }
}

void Event_c::mute29() {
    mMute29 = 1;
    l_mgr.mute(0xF, 0, 0);
}

void Event_c::unmute29Fade() {
    unmute29();
    l_mgr.mute(0xD, 0xB, 0x1A);
}

void Event_c::mute30() {
    mMute30 = 1;
    l_mgr.mute(0xF, 0, 0);
}

void Event_c::play2C() {
    mBgm2C = 0x1A97;
    l_mgr.play(0xE, 0x1A97, 0x2D, 1);
}

void Event_c::stop2CFade() {
    stopBgm2C();
    l_mgr.mute(0xD, 0xB, 0x1A);
}

void Event_c::stopBgm0() {
    if (mBgm0 != -1) {
        l_mgr.stop(mBgm0);
        mBgm0 = -1;
    }
    if (mMute4) {
        l_mgr.unmute(0xF);
        mMute4 = 0;
    }
}

void Event_c::stopBgmC() {
    if (mBgmC != -1) {
        l_mgr.stop(mBgmC);
        mBgmC = -1;
    }
    if (mMute10) {
        l_mgr.unmute(0xC);
        mMute10 = 0;
    }
}

void Event_c::stopBgm14() {
    if (mBgm14 != -1) {
        l_mgr.stop(mBgm14);
        mBgm14 = -1;
    }
    if (mMute18) {
        l_mgr.unmute(0x1F);
        mMute18 = 0;
    }
    if (mBgm1C != -1) {
        l_mgr.stop(mBgm1C);
        mBgm1C = -1;
    }
    if (mMute20) {
        l_mgr.unmute(0xF);
        mMute20 = 0;
    }
}

void Event_c::unmute29() {
    if (mMute29) {
        l_mgr.unmute(0xF);
        mMute29 = 0;
    }
}

void Event_c::stopBgm2C() {
    if (mBgm2C != -1) {
        l_mgr.stop(mBgm2C);
        mBgm2C = -1;
    }
    if (mMute30) {
        l_mgr.unmute(0xF);
        mMute30 = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// Player_c
// ---------------------------------------------------------------------------------------------

Player_c::Player_c() {
    mWeatherReq = 0;
    mStartReq = 0;
    mStopReq = 0;
    mVolumeReq = 0;
    mWeatherSet = 0;
    mId = -1;
    mFadeIn = 0;
    mStopFade = 0;
    mVolume = 0.0f;
    mVolumeFrames = 0;
    mWeather = 0;
    mPos = NULL;
}

Player_c::~Player_c() {}

void Player_c::start(u32 id, int fadeIn, u8 weather) {
    mId = id;
    mStartReq = 1;
    mFadeIn = fadeIn;
    mWeatherReq = weather;
}

void Player_c::stop(int fade) {
    mStopFade = fade;
    mStopReq = 1;
}

void Player_c::setVolume(f32 volume, int frames) {
    mVolume = volume;
    mVolumeReq = 1;
    mVolumeFrames = frames;
}

void Player_c::setWeather(int weather) {
    mWeather = weather;
    mWeatherSet = 1;
}

void Player_c::setPos(const mVec3_c *pos) {
    mPos = pos;
}

void Player_c::init() {
    mWeatherReq = 0;
    mStartReq = 0;
    mStopReq = 0;
    mVolumeReq = 0;
    mWeatherSet = 0;
    mId = -1;
    mFadeIn = 0;
    mStopFade = 0;
    mVolume = 0.0f;
    mVolumeFrames = 0;
    mWeather = 0;
    mPos = NULL;
}

void Player_c::execute() {
    if (mStopReq) {
        mStopReq = 0;
        fn_8000F158(mStopFade);
    }
    if (mStartReq) {
        mStartReq = 0;
        fn_8000F0D0(mId, mFadeIn);
        if (mWeatherReq) {
            mWeatherReq = 0;
            setWeather(l_mgr.mWeather.getKind());
        }
    }
    if (mWeatherSet) {
        mWeatherSet = 0;
        fn_8000F1D0(mWeather);
    }
    if (mVolumeReq) {
        mVolumeReq = 0;
        fn_8000F540(mVolume, mVolumeFrames);
    }
    if (mPos != NULL) {
        if (l_mgr.mQueue.isCurrentPos()) {
            fn_8000F6C0(mPos);
        }
        mPos = NULL;
    }
}

// ---------------------------------------------------------------------------------------------
// Request_c / Queue_c
// ---------------------------------------------------------------------------------------------

Request_c::Request_c() {
    mTimer = 0;
    clear();
    mLink.prevObject = NULL;
    mLink.nextObject = NULL;
}

Request_c::~Request_c() {}

void Request_c::clear() {
    mPrio = REQ_FREE;
    mId = -1;
    mFadeOut = 0;
    mFadeIn = 0;
    mTimer = 0;
    mDelete = 0;
    mWeather = 0;
    mDuck = 0;
    mPos = 0;
}

void Request_c::set(int prio, u32 id, int fadeOut, int fadeIn, int timer, u8 weather, u8 duck, u8 pos) {
    mPrio = prio;
    mId = id;
    mFadeOut = fadeOut;
    mFadeIn = fadeIn;
    mTimer = timer;
    mDelete = 0;
    mWeather = weather;
    mDuck = duck;
    mPos = pos;
}

BOOL Request_c::isTimeUp() {
    BOOL up = FALSE;
    if (mTimer > 0 && countDown(mTimer) <= 0) {
        up = TRUE;
    }
    return up;
}

void Request_c::print(const char *msg) {}

Queue_c::Queue_c() : mCurrent(NULL) {
    nw4r::ut::List_Init(&mList, offsetof(Request_c, mLink));
}

Queue_c::~Queue_c() {}

void Queue_c::entry(int prio, u32 id, int fadeOut, int fadeIn, int timer, u8 weather, u8 duck, u8 pos) {
    Request_c *req = alloc();
    if (req != NULL) {
        Request_c *next = NULL;
        Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
        while (r != NULL) {
            if (prio < r->mPrio) {
                next = r;
                break;
            }
            r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
        }
        req->set(prio, id, fadeOut, fadeIn, timer, weather, duck, pos);
        nw4r::ut::List_Insert(&mList, next, req);
        if (id == -1) {
            req->print("▽▽▽　登録　▽▽▽");
        } else {
            req->print("▼▼▼　登録　▼▼▼");
        }
        l_mgr.mTime.print();
        print("Entry");
    }
}

void Queue_c::removePrio(int prio) {
    Request_c *req = findPrio(prio);
    if (req != NULL) {
        req->print("△△△削除予約△△△");
        l_mgr.mTime.print();
        req->mDelete = 1;
        print("Delete");
    }
}

void Queue_c::remove(u32 id) {
    Request_c *req = findId(id);
    if (req != NULL) {
        req->print("▲▲▲削除予約▲▲▲");
        req->mDelete = 1;
    }
}

void Queue_c::init() {
    Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    while (r != NULL) {
        Request_c *cur = r;
        r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
        nw4r::ut::List_Remove(&mList, cur);
        cur->clear();
    }
    mCurrent = NULL;
}

void Queue_c::execute() {
    checkStop();
    removeDeleted();
    checkStart();
    checkTimers();
}

BOOL Queue_c::isCurrent(u32 id) {
    BOOL result = FALSE;
    if (id == getCurrentId() && id != -1) {
        result = TRUE;
    }
    return result;
}

u32 Queue_c::getCurrentId() {
    u32 id = -1;
    if (mCurrent != NULL) {
        id = mCurrent->mId;
    }
    return id;
}

f32 Queue_c::getCurrentVolume() {
    f32 volume = 0.0f;
    if (mCurrent != NULL) {
        volume = 1.0f;
    }
    return volume;
}

BOOL Queue_c::isCurrentDuck() {
    BOOL duck = FALSE;
    if (mCurrent != NULL) {
        duck = mCurrent->mDuck;
    }
    return duck;
}

BOOL Queue_c::isCurrentPos() {
    BOOL pos = FALSE;
    if (mCurrent != NULL) {
        pos = mCurrent->mPos;
    }
    return pos;
}

void Queue_c::print(const char *msg) {}

Request_c *Queue_c::findPrio(int prio) {
    Request_c *found = NULL;
    Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    while (r != NULL) {
        if (prio == r->mPrio) {
            found = r;
            break;
        }
        r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
    }
    return found;
}

Request_c *Queue_c::findId(u32 id) {
    Request_c *found = NULL;
    Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    while (r != NULL) {
        if (id == r->mId) {
            found = r;
            break;
        }
        r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
    }
    return found;
}

Request_c *Queue_c::findNext(Request_c *req) {
    Request_c *found = NULL;
    Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    while (r != NULL) {
        if (r == req) {
            break;
        }
        if (!r->mDelete) {
            found = r;
            break;
        }
        r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
    }
    return found;
}

Request_c *Queue_c::alloc() {
    Request_c *found = NULL;
    for (Request_c *r = mRequests; r < &mRequests[16]; r++) {
        if (r->mPrio == Request_c::REQ_FREE) {
            found = r;
            break;
        }
    }
    return found;
}

void Queue_c::checkStop() {
    if (mCurrent != NULL && mCurrent->mId != -1) {
        int fade = -1;
        Request_c *next = findNext(mCurrent);
        if (next != NULL) {
            fade = next->mFadeOut;
        } else if (mCurrent->mDelete) {
            fade = mCurrent->mFadeOut;
        }
        if (fade >= 0) {
            l_mgr.mPlayer.stop(fade);
            mCurrent = NULL;
        }
    }
}

void Queue_c::removeDeleted() {
    Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    while (r != NULL) {
        Request_c *cur = r;
        r = (Request_c *)nw4r::ut::List_GetNext(&mList, r);
        if (cur->mDelete) {
            nw4r::ut::List_Remove(&mList, cur);
            cur->clear();
        }
    }
}

void Queue_c::checkStart() {
    Request_c *first = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL);
    if (first != NULL && mCurrent == NULL && first->mId != -1) {
        l_mgr.mPlayer.start(first->mId, first->mFadeIn, first->mWeather);
        l_mgr.mVolCtrl.mRefresh = 1;
        mCurrent = first;
    }
}

void Queue_c::checkTimers() {
    for (Request_c *r = (Request_c *)nw4r::ut::List_GetNext(&mList, NULL); r != NULL;
         r = (Request_c *)nw4r::ut::List_GetNext(&mList, r)) {
        if (r->isTimeUp()) {
            r->print("▲▲▲削除予約（タイマー）▲▲▲");
            r->mDelete = 1;
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Weather_c
// ---------------------------------------------------------------------------------------------

Weather_c::Weather_c() {
    mWeather = 0;
    mPrevWeather = 0;
}

Weather_c::~Weather_c() {}

static inline BOOL isSnow(int weather) {
    BOOL snow = TRUE;
    if (weather != 6 && weather != 5) {
        snow = FALSE;
    }
    return snow;
}

static inline BOOL isRain(int weather) {
    BOOL rain = TRUE;
    if (weather != 4 && weather != 3) {
        rain = FALSE;
    }
    return rain;
}

int Weather_c::getKind() {
    int weather = mWeather;
    BOOL snow = isSnow(weather);
    BOOL rain = isRain(weather);
    if (snow) {
        return 3;
    }
    return rain ? 2 : 1;
}

void Weather_c::init() {
    mWeather = 0;
    mPrevWeather = 0;
}

void Weather_c::set() {
    if (lbl_8074EBE8 == NULL) {
        return;
    }
    int weather = lbl_8074EBE8->_5884;
    mWeather = weather;
    mPrevWeather = weather;
}

void Weather_c::update() {
    if (l_mgr.mTime.isHourChanged()) {
        dUnk8074EBE8_c *weather = lbl_8074EBE8;
        if (weather != NULL) {
            mPrevWeather = mWeather;
            mWeather = weather->_5884;
        }
    }
}

// ---------------------------------------------------------------------------------------------
// VolState_c and the states
// ---------------------------------------------------------------------------------------------

VolState_c::VolState_c(int id) : mId(id) {
    clear();
    mLink.prevObject = NULL;
    mLink.nextObject = NULL;
    mNomineeLink.prevObject = NULL;
    mNomineeLink.nextObject = NULL;
}

VolState_c::~VolState_c() {}

void VolState_c::clear() {
    mFrames = 0;
    mVolume = 0.0f;
    mChanged = 0;
    mDelete = 0;
}

void VolState_c::init() {
    clear();
}

VolStateAdjust_c::VolStateAdjust_c() : VolState_c(0) {
    mOn = 0;
}

VolStateAdjust_c::~VolStateAdjust_c() {}

void VolStateAdjust_c::init() {
    VolState_c::init();
    mOn = 0;
}

void VolStateAdjust_c::execute() {
    if (mOn) {
        l_mgr.mVolCtrl.denominate(this, 0);
        mOn = 0;
    }
}

VolStateWait_c::VolStateWait_c() : VolState_c(1) {
    mState = 0;
}

VolStateWait_c::~VolStateWait_c() {}

void VolStateWait_c::init() {
    VolState_c::init();
    mState = 0;
}

void VolStateWait_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0xF);
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0xF);
    }
}

void VolStateWait_c::start() {
    mState = 1;
}

void VolStateWait_c::end() {
    mState = 3;
}

void VolStateWait_c::cancel() {
    if (mState == 1) {
        mState = 0;
    } else if (mState == 2) {
        end();
    }
}

// 8046D418: the menu volumes.
static const f32 sMenuVolume[4] = {0.3125f, 1.0f, 0.0f, 1.0f};

VolStateMenu_c::VolStateMenu_c() : VolState_c(2) {
    mKind = 4;
    mState = 0;
}

VolStateMenu_c::~VolStateMenu_c() {}

void VolStateMenu_c::init() {
    VolState_c::init();
    mKind = 4;
    mState = 0;
}

void VolStateMenu_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, sMenuVolume[mKind], 0xF);
        fn_8000EF30();
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0xF);
        fn_8000EF98();
    }
}

void VolStateMenu_c::start(int kind) {
    mState = 1;
    mKind = kind;
    if (kind == 1) {
        l_mgr.mPlayer.setWeather(0xF);
    }
    l_mgr.mVolCtrl.mTalk.mKeep = 0;
}

void VolStateMenu_c::end() {
    if (mKind == 1) {
        l_mgr.mPlayer.setWeather(0xE);
    }
    mState = 3;
    mKind = 4;
}

VolStateTalk_c::VolStateTalk_c() : VolState_c(3) {
    mState = 0;
    mKeep = 0;
}

VolStateTalk_c::~VolStateTalk_c() {}

void VolStateTalk_c::init() {
    VolState_c::init();
    mState = 0;
    mKeep = 0;
}

void VolStateTalk_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0xF);
        fn_8000F000();
    } else if (mState == 3) {
        if (!mKeep) {
            mState = 0;
            l_mgr.mVolCtrl.denominate(this, 0xF);
        }
        fn_8000F068();
    }
}

void VolStateTalk_c::start() {
    mState = 1;
}

void VolStateTalk_c::end(u8 keep) {
    mKeep = keep;
    mState = 3;
}

VolStateCatapult_c::VolStateCatapult_c() : VolState_c(4) {
    mState = 0;
}

VolStateCatapult_c::~VolStateCatapult_c() {}

void VolStateCatapult_c::init() {
    VolState_c::init();
    mState = 0;
}

void VolStateCatapult_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0x2D);
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0x2D);
    }
}

void VolStateCatapult_c::start() {
    mState = 1;
}

void VolStateCatapult_c::end() {
    mState = 3;
}

VolStateFish_c::VolStateFish_c() : VolState_c(5) {
    mState = 0;
}

VolStateFish_c::~VolStateFish_c() {}

void VolStateFish_c::init() {
    VolState_c::init();
    mState = 0;
}

void VolStateFish_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0x2D);
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0x2D);
    }
}

void VolStateFish_c::start() {
    mState = 1;
}

void VolStateFish_c::end() {
    mState = 3;
}

VolStateFirework_c::VolStateFirework_c() : VolState_c(6) {
    mState = 0;
}

VolStateFirework_c::~VolStateFirework_c() {}

void VolStateFirework_c::init() {
    VolState_c::init();
    mState = 0;
}

void VolStateFirework_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0x2D);
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0x2D);
    }
}

void VolStateFirework_c::start() {
    mState = 1;
}

void VolStateFirework_c::end() {
    mState = 3;
}

VolStateSky_c::VolStateSky_c() : VolState_c(7) {
    mOn = 0;
}

VolStateSky_c::~VolStateSky_c() {}

void VolStateSky_c::init() {
    VolState_c::init();
    mOn = 0;
}

void VolStateSky_c::execute() {
    bool on = false;
    if (lbl_8074E9B0 != NULL && fn_8018A978(lbl_8074E9B0)) {
        on = true;
    }
    if (mOn) {
        if (!on) {
            l_mgr.mVolCtrl.denominate(this, 0x2D);
            mOn = 0;
        }
    } else if (on) {
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0xB4);
        mOn = 1;
    }
}

VolStateBeach_c::VolStateBeach_c() : VolState_c(8) {
    mOn = 0;
}

VolStateBeach_c::~VolStateBeach_c() {}

void VolStateBeach_c::init() {
    VolState_c::init();
    mOn = 0;
}

void VolStateBeach_c::execute() {
    const mVec3_c *pos = (const mVec3_c *)fn_800FBC88(4);
    bool on = false;
    if (pos != NULL) {
        dBGCF::groundChk_c ground(pos, 0, 0, 0);
        mVec3_c above(pos->x, pos->y, pos->z + 32.0f);
        int attr = ground.mAttr;
        dBGCF::groundChk_c ground2(&above, 0, 0, 0);
        int water = ground2.mWater;
        if (attr == 0x17 || attr == 0x1A || water == 2) {
            on = true;
        }
    }
    if (mOn) {
        if (!on) {
            l_mgr.mVolCtrl.denominate(this, 0xB4);
        }
    } else if (on) {
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0xB4);
    }
    mOn = on;
}

VolStateFocus_c::VolStateFocus_c() : VolState_c(9), mPos(mVec3_c::Zero) {
    mType = 2;
    mOn = 0;
}

VolStateFocus_c::~VolStateFocus_c() {}

void VolStateFocus_c::init() {
    VolState_c::init();
    mType = 2;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mOn = 0;
}

void VolStateFocus_c::execute() {
    const mVec3_c *pos = (const mVec3_c *)fn_800FBC88(4);
    bool on = false;
    f32 volume = 0.0f;
    if (mType != 2 && pos != NULL) {
        f32 dist = EGG::Mathf::sqrt(PSVECSquareDistance(*pos, mPos));
        f32 near;
        f32 far;
        if (mType == 0) {
            near = 200.0f;
            far = 400.0f;
        } else {
            near = 150.0f;
            far = 350.0f;
        }
        if (dist < far + 10.0f) {
            f32 rate;
            if (dist <= near) {
                rate = 1.0f;
            } else if (dist < far) {
                rate = (dist - far) / (near - far);
            } else {
                rate = 0.0f;
            }
            on = true;
            volume = (1.0f - rate) + rate * 0.3125f;
        }
        mType = 2;
    }
    if (mOn != on) {
        if (on) {
            l_mgr.mVolCtrl.nominate(this, volume, 0xF);
        } else {
            l_mgr.mVolCtrl.denominate(this, 0xF);
        }
        mOn = on;
    } else if (on) {
        l_mgr.mVolCtrl.setVolume(this, volume);
    }
}

void VolStateFocus_c::set(int type, const mVec3_c &pos) {
    mType = type;
    mPos = pos;
}

VolStateStage_c::VolStateStage_c() : VolState_c(10) {
    mState = 0;
    mShowState = 0;
}

VolStateStage_c::~VolStateStage_c() {}

void VolStateStage_c::init() {
    VolState_c::init();
    mState = 0;
    mShowState = 0;
}

void VolStateStage_c::execute() {
    if (mState == 1) {
        mState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.5f, 0x78);
    } else if (mState == 3) {
        mState = 0;
        l_mgr.mVolCtrl.denominate(this, 0x78);
    }
    if (mShowState == 1) {
        mShowState = 2;
        l_mgr.mVolCtrl.nominate(this, 0.3125f, 0xB4);
    } else if (mShowState == 3) {
        mShowState = 0;
        l_mgr.mVolCtrl.denominate(this, 0xB4);
    }
}

void VolStateStage_c::start() {
    mState = 1;
}

void VolStateStage_c::end() {
    mState = 3;
}

void VolStateStage_c::startShow() {
    mShowState = 1;
}

void VolStateStage_c::endShow() {
    mShowState = 3;
}

void VolStateStage_c::cancelShow() {
    if (mShowState == 2) {
        mShowState = 3;
    } else {
        mShowState = 0;
    }
}

// ---------------------------------------------------------------------------------------------
// VolCtrl_c
// ---------------------------------------------------------------------------------------------

VolCtrl_c::VolCtrl_c() {
    mRefresh = 0;
    nw4r::ut::List_Init(&mStates, offsetof(VolState_c, mLink));
    nw4r::ut::List_Init(&mNominees, offsetof(VolState_c, mNomineeLink));
    nw4r::ut::List_Append(&mStates, &mAdjust);
    nw4r::ut::List_Append(&mStates, &mWait);
    nw4r::ut::List_Append(&mStates, &mMenu);
    nw4r::ut::List_Append(&mStates, &mTalk);
    nw4r::ut::List_Append(&mStates, &mCatapult);
    nw4r::ut::List_Append(&mStates, &mFish);
    nw4r::ut::List_Append(&mStates, &mFirework);
    nw4r::ut::List_Append(&mStates, &mSky);
    nw4r::ut::List_Append(&mStates, &mBeach);
    nw4r::ut::List_Append(&mStates, &mFocus);
    nw4r::ut::List_Append(&mStates, &mStage);
}

VolCtrl_c::~VolCtrl_c() {}

void VolCtrl_c::execute() {
    executeStates();
    calcVolume();
    removeDeleted();
}

void VolCtrl_c::init() {
    VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL);
    while (s != NULL) {
        VolState_c *cur = s;
        s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s);
        nw4r::ut::List_Remove(&mNominees, cur);
    }
    for (VolState_c *t = (VolState_c *)nw4r::ut::List_GetNext(&mStates, NULL); t != NULL;
         t = (VolState_c *)nw4r::ut::List_GetNext(&mStates, t)) {
        t->init();
    }
    mRefresh = 0;
}

void VolCtrl_c::nominate(VolState_c *state, f32 volume, int frames) {
    VolState_c *next = NULL;
    bool found = false;
    VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL);
    while (s != NULL) {
        if (s == state) {
            found = true;
            break;
        }
        if (state->mId < s->mId) {
            next = s;
            break;
        }
        s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s);
    }
    state->mVolume = volume;
    state->mFrames = frames;
    state->mChanged = 1;
    if (found) {
        print("Refresh Only");
    } else {
        nw4r::ut::List_Insert(&mNominees, next, state);
        print("EntryNominate");
    }
}

void VolCtrl_c::denominate(VolState_c *state, int frames) {
    state->mFrames = frames;
    state->mVolume = 0.0f;
    state->mChanged = 1;
    state->mDelete = 1;
    print("DeleteNominate");
}

void VolCtrl_c::setVolume(VolState_c *state, f32 volume) {
    if (fManager_c::isExecuteStopped()) {
        return;
    }
    state->mVolume = volume;
    state->mChanged = 1;
}

void VolCtrl_c::print(const char *msg) {}

void VolCtrl_c::executeStates() {
    for (VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mStates, NULL); s != NULL;
         s = (VolState_c *)nw4r::ut::List_GetNext(&mStates, s)) {
        s->execute();
    }
}

void VolCtrl_c::calcVolume() {
    if (l_mgr.mQueue.getCurrentId() != -1) {
        if (mRefresh) {
            refreshVolume();
        } else {
            changeVolume();
        }
    }
    mRefresh = 0;
    for (VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL); s != NULL;
         s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s)) {
        s->mChanged = 0;
    }
}

void VolCtrl_c::refreshVolume() {
    Queue_c *queue = &l_mgr.mQueue;
    BOOL duck = queue->isCurrentDuck();
    f32 volume = queue->getCurrentVolume();
    for (VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL); s != NULL;
         s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s)) {
        if (duck && s->mId >= 3) {
            break;
        }
        if (!s->mDelete) {
            volume = s->mVolume;
            break;
        }
    }
    l_mgr.mPlayer.setVolume(volume, 0);
}

void VolCtrl_c::changeVolume() {
    Queue_c *queue = &l_mgr.mQueue;
    BOOL duck = queue->isCurrentDuck();
    VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL);
    int frames;
    f32 volume;
    bool changed = false;
    bool found = false;
    if (s != NULL && (!duck || s->mId < 3) && s->mChanged) {
        frames = s->mFrames;
        changed = true;
    }
    if (changed) {
        while (s != NULL) {
            if (duck && s->mId >= 3) {
                break;
            }
            if (!s->mDelete) {
                volume = s->mVolume;
                found = true;
                break;
            }
            s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s);
        }
        if (!found) {
            volume = queue->getCurrentVolume();
        }
        l_mgr.mPlayer.setVolume(volume, frames);
    }
}

void VolCtrl_c::removeDeleted() {
    VolState_c *s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, NULL);
    while (s != NULL) {
        VolState_c *cur = s;
        s = (VolState_c *)nw4r::ut::List_GetNext(&mNominees, s);
        if (cur->mDelete) {
            nw4r::ut::List_Remove(&mNominees, cur);
            cur->clear();
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Mgr_c
// ---------------------------------------------------------------------------------------------

Mgr_c::Mgr_c() {
    mStage = NULL;
}

Mgr_c::~Mgr_c() {}

void Mgr_c::play(int prio, u32 id, int fadeOut, u8 duck) {
    mQueue.entry(prio, id, fadeOut, 0, 0, 0, duck, 0);
}

void Mgr_c::playField(u32 id) {
    mQueue.entry(0x28, id, 0x2D, 0, 0, 1, 0, 0);
}

void Mgr_c::playTown(u32 id) {
    mQueue.entry(0x27, id, 0x2D, 0, 0, 1, 0, 1);
}

void Mgr_c::playRoom(u32 id) {
    mQueue.entry(0x18, id, 0x2D, 0, 0, 0, 0, 1);
}

void Mgr_c::playKK(u32 id, int fadeIn, u8 duck) {
    mQueue.entry(0x11, id, 0, fadeIn, 0, 0, duck, 0);
}

void Mgr_c::mute(int prio, int fadeOut, int timer) {
    mQueue.entry(prio, -1, fadeOut, 0, timer, 0, 0, 0);
}

void Mgr_c::stop(u32 id) {
    mQueue.remove(id);
}

void Mgr_c::unmute(int prio) {
    mQueue.removePrio(prio);
}

void Mgr_c::fn_8008001C() {}

void Mgr_c::init() {
    mChange.init();
    mSilence.init();
    mEvent.init();
    resetStages();
    mQueue.init();
    mVolCtrl.init();
    mPlayer.init();
    mTime.init();
    mState.init();
    mWeather.init();
}

void Mgr_c::execute() {
    mTime.update();
    mWeather.update();
    mChange.execute();
    mSilence.execute();
    mEvent.execute();
    executeStage();
    mQueue.execute();
    mVolCtrl.execute();
    mPlayer.execute();
    mState.execute();
}

void Mgr_c::sceneStart() {
    mWeather.set();
    mEvent.fn_8007CBB0();
    startStage();
    mChange.end();
    mState.start();
}

void Mgr_c::sceneEnd() {
    endStage();
    mEvent.stopAll();
}

void Mgr_c::sceneChange(int fade) {
    mChange.start(fade);
    fn_80080354();
}

void Mgr_c::startStage() {
    StgBase_c *stages[23] = {
        &mStgNon, &mStgField, &mStgRoom, &mStgOffice, &mStgChkp, &mStgMuseum,
        &mStgCafe, &mStgShop, &mStgTailor, &mStgTown, &mStgBroker, &mStgBarber,
        &mStgFortune, &mStgGrace, &mStgHappy, &mStgTheater, &mStgAuction, &mStgReset,
        &mStgTitle, &mStgSave, &mStgLoad, &mStgPlSel, &mStgBus,
    };
    if (getSceneParent() != NULL) {
        u32 param = getSceneParamC(getCurrentScene());
        mStage = stages[(u8)param];
        if (mStage != NULL) {
            mStage->start(param);
        }
    } else {
        mStage = NULL;
    }
}

void Mgr_c::endStage() {
    if (mStage != NULL) {
        mStage->end();
        mStage = NULL;
    }
}

void Mgr_c::fn_80080354() {}

void Mgr_c::resetStages() {
    mStgNon.reset();
    mStgField.reset();
    mStgRoom.reset();
    mStgOffice.reset();
    mStgChkp.reset();
    mStgMuseum.reset();
    mStgCafe.reset();
    mStgShop.reset();
    mStgTailor.reset();
    mStgTown.reset();
    mStgBroker.reset();
    mStgBarber.reset();
    mStgFortune.reset();
    mStgGrace.reset();
    mStgHappy.reset();
    mStgTheater.reset();
    mStgAuction.reset();
    mStgReset.reset();
    mStgTitle.reset();
    mStgSave.reset();
    mStgLoad.reset();
    mStgPlSel.reset();
    mStgBus.reset();
}

void Mgr_c::executeStage() {
    if (mStage == NULL) {
        return;
    }
    mStage->execute();
}

Mgr_c l_mgr;

} // namespace dBgm
