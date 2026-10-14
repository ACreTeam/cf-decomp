// A dated town record with two scales (dSvDateEvent::Event_c; the snowmen). .text 80150778..80150A5C,
// .sdata2 80750CE0..80750CF8. See include/game/game/d_sv_date_event.hpp.
#include <game/game/d_sv_date_event.hpp>

namespace dSvDateEvent {

const f32 cScaleMin = 0.5f;
const f32 cScaleMax = 1.25f;

// 80150778
void Event_c::clear() {
    mValid = 0;
}

// 80150784
int Event_c::diffDays(const dTime_c *time) const {
    dTime_c t = *time;
    t.add(0, -TIME_DAY_START_HOUR, 0, 0);
    return mDate.diffDays(&t, FALSE);
}

// 8015082C
int Event_c::diffDaysAdjusted(const dTime_c *time) const {
    dTime_c t = *time;
    t.add(0, -TIME_DAY_START_HOUR, 0, 0);
    return mDate.diffDays(&t, TRUE);
}

// 801508D4
void Event_c::set(f32 scale1, f32 scale2) {
    dTime_c t = *dTime_c::getCurrent();
    t.add(0, -TIME_DAY_START_HOUR, 0, 0);
    mDate.set(&t);
    setScale1(scale1);
    setScale2(scale2);
    mValid = 1;
}

// 801509AC
void Event_c::setScale1(f32 scale) {
    f32 v = scale - 0.5f;
    f32 c = v < 0.0f ? 0.0f : (v > 0.75f ? 0.75f : v);
    mScale1 = 255.0f * c / 0.75f;
}

// 80150A04
void Event_c::setScale2(f32 scale) {
    f32 v = scale - 0.5f;
    f32 c = v < 0.0f ? 0.0f : (v > 0.75f ? 0.75f : v);
    mScale2 = 255.0f * c / 0.75f;
}

} // namespace dSvDateEvent
