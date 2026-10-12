#ifndef D_WEATHER_H
#define D_WEATHER_H

#include <types.h>

class dTime_c;

// The weather manager dWeather_c (fBase_c profile; RTTI "dWeather_c", vtable 80500454, create 801C8EEC sets
// sInstance). Its TU (around 801C8EEC) is not split yet; only what other TUs use.
class dWeather_c {
public:
    static dWeather_c *sInstance; // 8074EBE8

    u8 _0000[0x5884];
    int _5884; // the weather: 3/4 rain, 5/6 snow

    static BOOL isRainKind(int weather) { return weather == 4 || weather == 3; } // _5884 is rain
};

extern "C" {
int fn_801C98CC(dWeather_c *weather); // the current weather (_5884)
int fn_801C987C(dWeather_c *weather); // 801C987C: weather kind for message jumps (dMsg::Seq_c tag 8:9)
// Weather check by time (d_npc_talk_free isWeatherTopic: 0 allows the weather topic by the hour).
int fn_801C9C08(const dTime_c *time); // 801C9C08
int fn_801C9CAC(const dTime_c *time); // 801C9CAC: a day's kind (cached for today and tomorrow)
int fn_801C9D9C(const dTime_c *time); // 801C9D9C: a day's weather (3/4 rain, 5/6 snow)
void fn_801C95CC(); // 801C95CC: resets the weather dTimeStamp_c (8074EBF0) and the byte 8074EBF8
void fn_801C95F8(); // 801C95F8: sets dWeather_c::sInstance->_5899, or fn_801C95CC without one
}

#endif
