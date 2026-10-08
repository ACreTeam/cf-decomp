// .text 80090964..80090D70, .sdata2 80750608..80750628.
#include <game/game/d_star.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_weather.hpp>

// 80090964
void dStarMgr_c::init() {
    dTime_c now = *dTime_c::getCurrent();
    mEnabled = !isCityScene(getCurrentScene()) && !isSceneAttr(getCurrentScene(), 0x110) &&
               !isSceneAttr(getCurrentScene(), 0x4000);
    mLastSec = now.sec;
}

// 80090A40
void dStarMgr_c::execute() {
    if (!mEnabled) {
        return;
    }
    dTime_c now = *dTime_c::getCurrent();
    dUnk8074EBE8_c *weather = lbl_8074EBE8;
    if (weather != NULL && fn_801C98CC(weather) == 0 && (now.hour >= 19 || now.hour <= 4) && now.sec < mLastSec) {
        dRandom_c rnd(0x9D);
        dTime_c time = *dTime_c::getCurrent();
        dSaveData_c *save = dSaveData_c::getRaw();
        rnd.init(time.year, time.yday, time.min + time.hour * 60, save->mLandID.mId, 0x77201243);
        f32 r = rnd.rndF(1.0f);
        int num = 0;
        if (r < 0.25f) {
            num = 5;
        } else if (r < 0.5f) {
            num = 8;
        }
        for (int i = 0; i < num; i++) {
            for (int j = 0; j < STAR_NUM; j++) {
                if (!mStars[j].mActive) {
                    mStars[j].start(rnd.rndF(3600.0f));
                    f32 minX = 210.0f;
                    f32 maxX = 710.0f;
                    f32 minY = 20.0f;
                    f32 maxY = 120.0f;
                    f32 x = rnd.rndF(1.0f);
                    f32 y = rnd.rndF(1.0f);
                    mStars[j].mX = minX * (1.0f - x) + maxX * x;
                    mStars[j].mY = minY * (1.0f - y) + maxY * y;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < STAR_NUM; i++) {
        mStars[i].execute();
    }
    mLastSec = now.sec;
}

// 80090D20: here rather than d_star.cpp; the weak ~cRandom_c that ends this TU follows it.
void dStar_c::start(int wait) {
    mWait = wait;
    mActive = true;
}
