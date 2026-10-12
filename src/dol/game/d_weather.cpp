// Weather manager, daily pattern generation and forecast cache.
#include <game/game/d_event.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_weather.hpp>
#include <game/sLib/s_lib.hpp>

CUSTOM_BASE_PROFILE(WEATHER, dWeather_c, 0xA9, 0x81);
dWeather_c *dWeather_c::sInstance;
namespace dWeather {
static dTimeStamp_c sCacheDate ALIGN(8);
static bool sNextCached;
static int sPattern;
static int sCityPattern;
static int sNextPattern;
static int sNextCityPattern;
static const u8 sPattern0[24] = {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const u8 sPattern1[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                                 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
static const u8 sPattern2[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1};
static const u8 sPattern3[24] = {1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
static const u8 sPattern4[24] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1,
                                 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};
static const u8 sPattern5[24] = {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern6[24] = {2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern7[24] = {2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 2,
                                 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2};
static const u8 sPattern8[24] = {2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2,
                                 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2};
static const u8 sPattern9[24] = {2, 2, 3, 3, 3, 2, 2, 2, 3, 3, 2, 2,
                                 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sPattern10[24] = {3, 2, 2, 1, 2, 3, 3, 3, 3, 3, 3, 2,
                                  2, 2, 3, 3, 3, 2, 2, 2, 2, 3, 3, 3};
static const u8 sPattern11[24] = {3, 3, 3, 2, 2, 3, 3, 2, 2, 2, 3, 3,
                                  3, 3, 3, 2, 2, 2, 2, 3, 3, 3, 3, 3};
static const u8 sPattern12[24] = {3, 3, 3, 3, 3, 2, 3, 3, 3, 2, 3, 3,
                                  3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3};
static const u8 sPattern13[24] = {3, 3, 3, 4, 4, 3, 3, 4, 4, 4, 4, 3,
                                  3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 3, 3};
static const u8 sPattern14[24] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 4,
                                  4, 4, 4, 4, 4, 3, 3, 3, 4, 4, 4, 3};
static const u8 sPattern15[24] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                  4, 4, 3, 3, 3, 2, 2, 3, 3, 3, 4, 4};
static const u8 sPattern16[24] = {2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3};
static const u8 sPattern17[24] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3,
                                  3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sPattern18[24] = {0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
                                  2, 2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2};
static const u8 sPattern19[24] = {3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1,
                                  1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 2};
static const u8 sPattern20[24] = {3, 2, 2, 2, 2, 2, 3, 2, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern21[24] = {2, 2, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0};
static const u8 sPattern22[24] = {1, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern23[24] = {0, 0, 0, 0, 1, 1, 3, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern24[24] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 4, 2, 1,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0};
static const u8 sPattern25[24] = {1, 1, 0, 0, 0, 1, 1, 1, 4, 2, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sPattern26[24] = {2, 2, 1, 2, 2, 2, 3, 3, 3, 4, 4, 3,
                                  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sPattern27[24] = {2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 3, 3,
                                  3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 2};
static const u8 sPattern28[24] = {2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
                                  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3};
static const u8 sPattern29[24] = {3, 3, 2, 2, 2, 3, 3, 2, 2, 2, 1, 1,
                                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sPattern30[24] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,
                                  2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2};
static const u8 sPattern31[24] = {4, 4, 3, 3, 3, 2, 2, 3, 2, 2, 2, 2,
                                  2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2};
static const u8 sWeights0[32] = {4, 10, 10, 10, 10, 10, 10, 2, 2, 2, 2,
                                 2, 2,  0,  0,  0,  2,  2,  2, 2, 2, 2,
                                 0, 0,  0,  0,  2,  2,  2,  2, 2, 2};
static const u8 sWeights1[32] = {0, 25, 25, 25, 25, 0, 0, 0, 0, 0, 0,
                                 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,
                                 0, 0,  0,  0,  0,  0, 0, 0, 0, 0};
static const u8 sWeights2[32] = {1, 10, 10, 10, 10, 10, 10, 6, 6, 6, 3,
                                 3, 3,  0,  0,  0,  0,  0,  0, 0, 0, 0,
                                 0, 0,  0,  0,  2,  2,  2,  2, 2, 2};
static const u8 sWeights3[32] = {0,  0,  0, 0, 0, 5, 5, 8, 8, 8, 10,
                                 10, 10, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                 0,  0,  0, 0, 3, 3, 3, 3, 3, 3};
static const u8 sWeights4[32] = {2, 12, 12, 12, 12, 5, 5, 2, 2, 2, 0,
                                 0, 0,  2,  2,  2,  2, 2, 2, 2, 2, 2,
                                 5, 5,  3,  3,  0,  0, 0, 0, 0, 0};
static const u8 sWeights5[32] = {2, 12, 12, 12, 12, 6, 6, 1, 1, 1, 0,
                                 0, 0,  0,  0,  0,  0, 0, 0, 2, 2, 2,
                                 8, 8,  5,  5,  0,  0, 0, 1, 1, 1};
static const u8 sWeights6[32] = {3, 6, 6,  6,  6, 3, 3, 0, 0, 0, 0,
                                 0, 0, 5,  5,  5, 0, 0, 0, 0, 0, 0,
                                 8, 8, 15, 15, 1, 1, 1, 1, 1, 1};
static const u8 sWeights7[32] = {3, 12, 12, 12, 12, 6, 6, 2, 2, 1, 1,
                                 1, 1,  2,  2,  2,  1, 1, 1, 2, 2, 2,
                                 4, 4,  0,  0,  1,  1, 1, 1, 1, 1};
static const u8 sWeights8[32] = {2, 15, 15, 15, 15, 8, 8, 2, 2, 0, 0,
                                 0, 0,  0,  0,  0,  2, 2, 2, 2, 2, 2,
                                 3, 3,  0,  0,  0,  0, 0, 0, 0, 0};
static const u8 sWeights9[32] = {2, 15, 15, 15, 15, 10, 10, 3, 3, 0, 0,
                                 0, 0,  0,  0,  0,  2,  2,  2, 2, 2, 2,
                                 0, 0,  0,  0,  0,  0,  0,  0, 0, 0};
static const u8 sWeights10[32] = {2, 12, 12, 12, 12, 10, 10, 0, 0,  0,  0,
                                  0, 0,  0,  0,  0,  0,  0,  0, 10, 10, 10,
                                  0, 0,  0,  0,  0,  0,  0,  0, 0,  0};
static const u8 sWeights11[32] = {2, 10, 10, 10, 10, 8, 8, 3, 3, 3, 3,
                                  3, 3,  0,  0,  0,  2, 2, 2, 2, 2, 2,
                                  0, 0,  0,  0,  2,  2, 2, 2, 2, 2};
static const u8 sWeights12[32] = {0, 0, 0,  0,  0,  0, 0, 0, 0, 0, 0,
                                  0, 0, 30, 40, 30, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0,  0,  0,  0, 0, 0, 0, 0};
static const u8 sWeights13[32] = {3, 6, 6, 6, 6, 5, 5, 3, 3, 3, 5,
                                  5, 5, 3, 3, 3, 2, 2, 2, 2, 2, 2,
                                  0, 0, 0, 0, 3, 3, 3, 3, 3, 3};
static const u8 sWeights14[32] = {0,  0,  0,  0, 0,  0,  0,  0,  0,  0, 10,
                                  10, 10, 10, 0, 0,  0,  0,  0,  0,  0, 0,
                                  0,  0,  0,  0, 10, 10, 10, 10, 10, 10};
static const u8 sWeights15[32] = {4, 20, 20, 20, 20, 8, 8, 0, 0, 0, 0,
                                  0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0,
                                  0, 0,  0,  0,  0,  0, 0, 0, 0, 0};
static const u8 sWeights16[32] = {4, 12, 12, 12, 12, 4, 4, 1, 1, 2, 2,
                                  2, 2,  2,  2,  2,  2, 2, 2, 2, 2, 2,
                                  0, 0,  0,  0,  2,  2, 2, 2, 2, 2};
static const u8 sWeights17[32] = {2, 12, 12, 12, 12, 10, 10, 0, 0,  0,  0,
                                  0, 0,  0,  0,  0,  0,  0,  0, 10, 10, 10,
                                  0, 0,  0,  0,  0,  0,  0,  0, 0,  0};
static const u8 sForecast[32] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2,
                                 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4,
                                 5, 5, 5, 5, 6, 6, 6, 7, 7, 7};
struct EventPattern {
    dQuestEvent_e event;
    u8 pattern;
};

static const EventPattern sEventPatterns[14] = {
    {(dQuestEvent_e)0xE, 4},  {(dQuestEvent_e)0xF, 4},
    {(dQuestEvent_e)0x10, 4}, {(dQuestEvent_e)0x17, 28},
    {(dQuestEvent_e)0x11, 4}, {(dQuestEvent_e)0x13, 4},
    {(dQuestEvent_e)0x14, 4}, {(dQuestEvent_e)0x18, 4},
    {(dQuestEvent_e)0x1D, 4}, {(dQuestEvent_e)0x2B, 4},
    {(dQuestEvent_e)0x15, 4}, {(dQuestEvent_e)0x16, 4},
    {(dQuestEvent_e)0x23, 4}, {(dQuestEvent_e)0x27, 4},
};

static char sResourcePath[32] = "/Weather/weather_obj.brres";

static const u8 *sPatterns[32] = {
    sPattern0,  sPattern1,  sPattern2,  sPattern3,  sPattern4,  sPattern5,
    sPattern6,  sPattern7,  sPattern8,  sPattern9,  sPattern10, sPattern11,
    sPattern12, sPattern13, sPattern14, sPattern15, sPattern16, sPattern17,
    sPattern18, sPattern19, sPattern20, sPattern21, sPattern22, sPattern23,
    sPattern24, sPattern25, sPattern26, sPattern27, sPattern28, sPattern29,
    sPattern30, sPattern31};
static const u8 *sWeightTables[18] = {
    sWeights0,  sWeights1,  sWeights2,  sWeights3,  sWeights4,  sWeights5,
    sWeights6,  sWeights7,  sWeights8,  sWeights9,  sWeights10, sWeights11,
    sWeights12, sWeights13, sWeights14, sWeights15, sWeights16, sWeights17};
struct SeasonTable {
    int table;
    s16 endDate;
};

static SeasonTable sSeasons[19] = {
    {15, 0x0004}, {16, 0x0117}, {17, 0x0118}, {0, 0x021F},  {1, 0x0308},
    {2, 0x050F},  {3, 0x0605},  {4, 0x061F},  {5, 0x071F},  {6, 0x080F},
    {7, 0x081E},  {8, 0x0A0E},  {9, 0x0A18},  {10, 0x0A19}, {11, 0x0B09},
    {12, 0x0B0A}, {13, 0x0B17}, {14, 0x0B1E}, {15, 0x0B1F},
};

static int sMessageWeather[7] = {0, 1, 2, 3, 4, 5, 6};

} // namespace dWeather

dWeather_c::dWeather_c()
    : mResourcesReady(0), mInitialized(0), mResetOnDelete(0) {
}

dWeather_c::~dWeather_c() {
}

int dWeather_c::create() {
    int result = SUCCEEDED;
    bool effects = false;
    if (!dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM) ||
        dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_CHKP)) {
        effects = true;
    }
    mDrawWeather = effects;
    if (dScene::getCurrentScene() == dScene::SCENE_DM_BUS_PL_CRT) {
        return FAILED;
    } else if (dScene::getCurrentScene() == dScene::SCENE_DM_PL_SEL ||
               dScene::getCurrentScene() == dScene::SCENE_DM_SAVE ||
               dScene::getCurrentScene() == dScene::SCENE_DM_LOAD) {
        return FAILED;
    } else {
        BOOL city = dWeather::isCityWeather();
        if (!mInitialized) {
            mSnowSeason = dScene::isLateSeason();
            mLandId = dSaveData_c::getRaw()->mLandID.mId;
            sInstance = this;
            mInitialized = true;
            dWeather::updateCache();
            dTime_c time = *dTime_c::getCurrent();
            if (city) {
                mWeather = dWeather::getCityWeather(&time);
                time.add(0, 1, 0, 0);
                mNextWeather = dWeather::getCityWeather(&time);
            } else {
                mWeather = dWeather::getWeather(&time);
                time.add(0, 1, 0, 0);
                mNextWeather = dWeather::getWeather(&time);
            }
        }
        if (mDrawWeather) {
            EGG::Heap *heap = dHeap::localHeap_p;
            if (mResources.load(dWeather::sResourcePath, heap, 0)) {
                mAllocator.attach(heap, 0x20);
                nw4r::g3d::ResFile snow(mResources.getData());
                mSnow.create(&snow, &mAllocator);
                nw4r::g3d::ResFile sakura(mResources.getData());
                mSakura.create(&sakura, &mAllocator);
                nw4r::g3d::ResFile confetti(mResources.getData());
                mConfetti.create(&confetti, &mAllocator);
                if (mWeather == dWeather::HEAVY_SNOW) {
                    mSnow.init(dWeatherSnow::getHeavyCount());
                } else if (mWeather == dWeather::SNOW) {
                    mSnow.init(dWeatherSnow::getLightCount());
                }
                int count = dWeatherSakura::getCount();
                if (count) {
                    mSakura.init(count);
                }
                mConfetti.init();
                mRain.init(mWeather);
                mResourcesReady = true;
            } else {
                result = NOT_READY;
            }
        }
        if (!result) {
            return result;
        }
        mRainLevel = getRainLevel();
    }
    return result;
}

int dWeather_c::execute() {
    sLib::chase(&mRainLevel, getRainLevel(), 1.0f / 120.0f);
    dWeather::updateCache();
    dTime_c time = *dTime_c::getCurrent();
    int weather;
    if (dWeather::isCityWeather()) {
        weather = dWeather::getCityWeather(&time);
        time.add(0, 1, 0, 0);
        mNextWeather = dWeather::getCityWeather(&time);
    } else {
        weather = dWeather::getWeather(&time);
        time.add(0, 1, 0, 0);
        mNextWeather = dWeather::getWeather(&time);
    }
    if (mWeather != weather) {
        mWeather = weather;
    }
    if (mDrawWeather) {
        if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_BUS)) {
            mRain.calcBus(mWeather);
            mSnow.calcBus(mWeather);
            mSakura.calc(dWeatherSakura::getCount());
        } else {
            mRain.calc(mWeather, mNextWeather);
            mSnow.calc(mWeather);
            mSakura.calc(dWeatherSakura::getCount());
        }
        mConfetti.calc();
    }
    if (mWeather == dWeather::HEAVY_RAIN || mRain.mThunder.mState) {
        mRain.mThunder.calc(!dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM));
    }
    if ((mWeather == dWeather::RAIN || mWeather == dWeather::HEAVY_RAIN ||
         mWeather == dWeather::SNOW || mWeather == dWeather::HEAVY_SNOW) &&
        !fgMngProc_isSaveFlag6() && !dSvMgr_c::isFullTransferComplete()) {
        if (!dScene::isCityScene(dScene::getCurrentScene()) &&
            (!dNet::isSessionActive() || (u32)dNet::isHost() == 1)) {
            fgMngProc_setSaveFlag6();
        }
    }
    return SUCCEEDED;
}

int dWeather_c::draw() {
    if (mWeather == dWeather::HEAVY_RAIN || mRain.mThunder.mState) {
        mRain.mThunder.draw(!dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM));
    }
    if (mDrawWeather) {
        mSnow.draw();
        mSakura.draw();
        mConfetti.draw();
    }
    return SUCCEEDED;
}

int dWeather_c::doDelete() {
    int result = SUCCEEDED;
    if (mResourcesReady) {
        mSnow.remove();
        mSakura.remove();
        mConfetti.remove();
        mRain.remove();
        mResourcesReady = false;
    }
    if (mResetOnDelete) {
        mResetOnDelete = false;
        dWeather::resetCache();
    }
    if (mDrawWeather && !mResources.unload(FALSE)) {
        result = NOT_READY;
    }
    if (!result) {
        return result;
    }
    if (result == SUCCEEDED && sInstance) {
        sInstance = NULL;
    }
    return result;
}

namespace dWeather {
BOOL isCityWeather() {
    BOOL city = FALSE;
    if (dScene::isCityScene(dScene::getCurrentScene()) ||
        dScene::getCurrentScene() == dScene::SCENE_DM_BUS_TO_TOWN) {
        city = TRUE;
    }
    return city;
}

void resetCache() {
    sCacheDate.reset();
    sNextCached = false;
}

void requestReset() {
    if (dWeather_c::sInstance) {
        dWeather_c::sInstance->mResetOnDelete = true;
    } else {
        resetCache();
    }
}

int getSeasonTable(u8 month, u8 day) {
    int result = 18;
    s16 date = (month << 8) | day;
    for (int i = 0; i < 19; ++i) {
        if (date <= sSeasons[i].endDate) {
            result = sSeasons[i].table;
            break;
        }
    }
    return result;
}

void updateCache() {
    dTime_c now = *dTime_c::getCurrent();
    bool changed = false;
    if (sCacheDate.isNone()) {
        changed = true;
    } else {
        if (sCacheDate.diffDays(now, TRUE, FALSE)) {
            changed = true;
        }
    }
    if (changed) {
        if (sNextCached) {
            sPattern = sNextPattern;
            sCityPattern = sNextCityPattern;
        } else {
            sPattern = (u8)calcPattern(&now, 0);
            sCityPattern = (u8)calcPattern(&now, 1);
        }
        sNextCached = false;
        sCacheDate.set(now);
    }
    if (!sNextCached) {
        dTime_c next = now;
        next.add(0, 1, 0, 0);
        if (sCacheDate.diffDays(next, TRUE, FALSE)) {
            sNextPattern = (u8)calcPattern(&next, 0);
            sNextCityPattern = (u8)calcPattern(&next, 1);
            sNextCached = true;
        }
    }
}

} // namespace dWeather

f32 dWeather_c::getRainLevel() {
    if (mDrawWeather) {
        return mRain.getLevel();
    }
    return dWeatherRain::Rain_c::getLevel(mWeather, mNextWeather);
}

int dWeather_c::getMessageWeather() {
    return dWeatherSakura::isSeason() ? 7 : dWeather::sMessageWeather[mWeather];
}

int dWeather_c::getCurrentPattern() {
    return dWeather::sPattern;
}

namespace dWeather {
BOOL getEventPattern(const dTime_c *time, u8 *pattern) {
    for (u32 i = 0; i < 14; ++i) {
        dQuestEvent_e event = sEventPatterns[i].event;
        if (dEvent::isEventOnDate(event, (const int &)time->year,
                                  (const int &)time->month,
                                  (const int &)time->mday)) {
            *pattern = sEventPatterns[i].pattern;
            return TRUE;
        }
    }
    return FALSE;
}

int calcPattern(const dTime_c *time, int city) {
    dSvDaily::Daily_c *daily = &dSaveData_c::getRaw()->mDaily;
    dTime_c date = *time;
    date.add(0, -6, 0, 0);
    if (!city) {
        if (daily->mValue != dSvDaily::DAILY_NONE &&
            daily->mDate.isDate(&date)) {
            return daily->mValue;
        }
        u8 pattern;
        if (getEventPattern(&date, &pattern)) {
            return pattern;
        }
    }
    u8 table = getSeasonTable((u8)date.month, (u8)date.mday);
    f32 total = 0.0f;
    const u8 *weights = sWeightTables[table];
    dRandom_c random(0x9D);
    if (city) {
        random.initFromDateWithSalt(date, 0x841037AC);
    } else if (dWeather_c::sInstance) {
        random.initFromDateWithSalt2(date, 0x841037AC,
                                     dWeather_c::sInstance->mLandId);
    } else {
        random.initFromDateAndLandId(date, 0x841037AC);
    }
    f32 value = random.rndF(100.0f);
    for (u32 i = 0; i < 32; ++i) {
        total += weights[i];
        if (total > value) {
            return i;
        }
    }
    return 0;
}

int getPattern(const dTime_c *time) {
    dTimeStamp_c *stamp = &sCacheDate;
    int days = 2;
    if (!sCacheDate.isNone()) {
        days = stamp->diffDays(*time, TRUE, FALSE);
    }
    if (days == 0) {
        return sPattern;
    }
    if (days == 1 && sNextCached) {
        return sNextPattern;
    }
    return (u8)calcPattern(time, 0);
}

int getCityPattern(const dTime_c *time) {
    int days = sCacheDate.diffDays(*time, TRUE, FALSE);
    if (days == 0) {
        return sCityPattern;
    }
    if (days == 1 && sNextCached) {
        return sNextCityPattern;
    }
    return (u8)calcPattern(time, 1);
}

int getWeather(const dTime_c *time) {
    int hour = time->hour - 6;
    if (hour < 0) {
        hour += 24;
    }
    int weather = sPatterns[getPattern(time)][hour];
    if (isSnowSeason()) {
        if (weather == RAIN) {
            weather = SNOW;
        } else if (weather == HEAVY_RAIN) {
            weather = HEAVY_SNOW;
        }
    }
    return weather;
}

int getCityWeather(const dTime_c *time) {
    int hour = time->hour - 6;
    if (hour < 0) {
        hour += 24;
    }
    int weather = sPatterns[getCityPattern(time)][hour];
    if (isSnowSeason()) {
        if (weather == RAIN) {
            weather = SNOW;
        } else if (weather == HEAVY_RAIN) {
            weather = HEAVY_SNOW;
        }
    }
    return weather;
}

u8 getForecast(const dTime_c *time) {
    u8 forecast = sForecast[getPattern(time)];
    if (dScene::isLateSeason(time)) {
        switch (forecast) {
        case 2:
            forecast = 8;
            break;
        case 6:
            forecast = 9;
            break;
        case 7:
            forecast = 10;
            break;
        }
    }
    return forecast;
}

BOOL isSnowSeason() {
    if (dWeather_c::sInstance) {
        return dWeather_c::sInstance->mSnowSeason;
    }
    return dScene::isLateSeason();
}

} // namespace dWeather
