// The dated daily town value (dSvDaily::Daily_c). .text 801503E4..80150524, .sbss 8074E7E0.
// See include/game/game/d_sv_daily.hpp.
#include <game/game/d_sv_daily.hpp>
#include <game/game/d_weather.hpp>

namespace dSvDaily {

u8 sRequest;

// 801503E4
void Daily_c::clear() {
    mValue = DAILY_NONE;
}

// 801503F0
void Daily_c::request() {
    sRequest = 1;
}

// 801503FC
void Daily_c::update() {
    if (sRequest) {
        set(dTime_c::getCurrent(), DAILY_INITIAL);
        sRequest = 0;
    }
    if (mValue != DAILY_NONE && !mDate.isToday(TRUE)) {
        clear();
    }
    fn_801C95CC();
}

// 80150478
void Daily_c::set(const dTime_c *time, u8 value) {
    dTime_c t = *time;
    t.add(0, -6, 0, 0);
    mDate.set(&t);
    mValue = value;
}

} // namespace dSvDaily
