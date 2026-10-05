// Game-side random generator. See include/game/game/d_random.hpp and notes/d_random.txt.
// .text 80107D6C..80107F00; no data.
#include <game/game/d_random.hpp>
#include <game/game/d_save_data.hpp>

// 80107D6C
void dRandom_c::init(int a, int b, int c, int d, int e) {
    setSeed(a + b + c + d + e);
    int skip = ((c * e) & 7) + ((c + e) & 3) + 3;
    for (int i = 0; i < skip; i++) {
        rnd();
    }
}

// 80107DF4
void dRandom_c::initFromDateAndLandId(const dTime_c &time, int salt) {
    dSaveData_c *save = dSaveData_c::getRaw();
    init(time.year * 365, (time.month + 1) * 31, time.mday, save->mLandID.mId, salt);
}

// 80107E68
void dRandom_c::initFromDateWithSalt2(const dTime_c &time, int salt, int salt2) {
    init(time.year * 365, (time.month + 1) * 31, time.mday, salt2, salt);
}

// 80107E94
void dRandom_c::initFromDateWithSalt(const dTime_c &time, int salt) {
    init(time.year * 365, (time.month + 1) * 31, time.mday, 0x90C1, salt);
}

// 80107EC4
float dRandom_c::rnd() {
    return getRandomF();
}

// 80107EC8
float dRandom_c::rndF(float max) {
    return max * rnd();
}
