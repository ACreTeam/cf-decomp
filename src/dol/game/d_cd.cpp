// The New Year's Eve countdown process (dCd_c). .text 8018D778..8018DAC8,
// .rodata 8047BEF8 (scene table), .data 804F9880 (vtable), .sdata 8074BF00
// (profile / RTTI). See include/game/game/d_cd.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_cd.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>

CUSTOM_BASE_PROFILE(CD, dCd_c, fProf::CD, fProf::DRAW_ORDER::CD);

// Scenes in which the countdown runs (by scene index).
static const u8 l_sceneEnabled[dScene::SCENE_NUM] = {
    1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
};

#define SECONDS_PER_DAY (24 * 60 * 60)
#define SECONDS_PER_HOUR (60 * 60)
#define SECONDS_PER_MINUTE 60

// 8018D7C4
int dCd_c::create() {
    int scene = dScene::getCurrentScene();
    mMuted = TRUE;
    mEnabled = l_sceneEnabled[scene] != 0;
    return 1;
}

// 8018D81C
int dCd_c::execute() {
    BOOL skip = FALSE;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        skip = TRUE;
    }
    if (!mEnabled || skip) {
        return 1;
    }

    dTime_c now = *dTime_c::getCurrent();
    if (now.mon == MONTH_DECEMBER && now.mday == 31) {
        u32 left = SECONDS_PER_DAY - (now.sec + now.min * SECONDS_PER_MINUTE +
                                      now.hour * SECONDS_PER_HOUR);
        u32 hours = left / SECONDS_PER_HOUR;
        left %= SECONDS_PER_HOUR;
        u32 mins = left / SECONDS_PER_MINUTE;
        u32 secs = left % SECONDS_PER_MINUTE;
        mDigit = secs % 10;
        if (mDigit != mPrevDigit && hours == 0) {
            if (mins == 1 && secs == 0) {
                if (!mMuted) {
                    playSe(SE_countdown_small);
                }
            } else if (mins == 0) {
                if (secs == 0) {
                    if (!mMuted) {
                        playSe(SE_countdown_hi);
                    }
                } else if (secs <= 10) {
                    if (!mMuted) {
                        playSe(SE_countdown_low);
                    }
                } else {
                    if (!mMuted) {
                        playSe(SE_countdown_small);
                    }
                }
            }
            mMuted = FALSE;
        }
        mPrevDigit = mDigit;
    } else if (now.mon == MONTH_JANUARY && now.mday == 1 && now.hour == 0 &&
               now.min == 0 && now.sec == 0) {
        if (!mNewYearDone) {
            playSe(SE_countdown_hi);
            mNewYearDone = TRUE;
        }
    }
    return 1;
}

// 8018DA60
dCd_c::~dCd_c() {
}

// 8018DABC
dScene::dSceneChange_c *dCd_c::getSceneChange() {
    return &dScene::gSceneChange;
}
