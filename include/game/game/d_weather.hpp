#pragma once
#include <game/framework/f_base.hpp>
#include <game/game/d_weather_draw.hpp>
#include <game/game/d_dvd.hpp>

class dTime_c;

namespace dWeather {
enum Kind_e { CLEAR, SUNNY, CLOUDY, RAIN, HEAVY_RAIN, SNOW, HEAVY_SNOW };
BOOL isCityWeather();
void resetCache();
void requestReset();
int getSeasonTable(u8 month, u8 day);
void updateCache();
BOOL getEventPattern(const dTime_c *time, u8 *pattern);
int calcPattern(const dTime_c *time, int city);
int getPattern(const dTime_c *time);
int getCityPattern(const dTime_c *time);
int getWeather(const dTime_c *time);
int getCityWeather(const dTime_c *time);
int getForecast(const dTime_c *time);
BOOL isSnowSeason();
} // namespace dWeather

class dWeather_c : public fBase_c {
  public:
    dWeather_c();
    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual ~dWeather_c();
    f32 getRainLevel();
    int getMessageWeather();
    int getCurrentPattern();
    static BOOL isFineKind(int weather) {
        return weather == dWeather::CLEAR || weather == dWeather::SUNNY;
    }
    static BOOL isRainKind(int weather) {
        return weather == dWeather::HEAVY_RAIN || weather == dWeather::RAIN;
    }
    static dWeather_c *sInstance;
    dWeatherRain::Rain_c mRain;
    dWeatherSnowDraw_c mSnow;
    dWeatherSakuraDraw_c mSakura;
    dWeatherConfettiDraw_c mConfetti;
    mAllocator_c mAllocator;
    dDvd::brresBank_c mResources;
    int mWeather;
    int mNextWeather;
    u32 mLandId;
    f32 mRainLevel;
    u8 mDrawWeather;
    u8 mSnowSeason;
    u8 mResourcesReady;
    u8 mInitialized;
    u8 _5898;
    u8 mResetOnDelete;
};

STATIC_ASSERT(sizeof(dWeather_c) == 0x589C);
