// Theater weekly program schedule. See include/game/game/d_theater.hpp and notes/d_theater.txt.
// .text 8014BD88..8014C2C4, .data 804EF518..804EF5C0, .sdata 8074B168..8074B180,
// .sdata2 80750CB8..80750CC8.
#include <game/game/d_theater.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_search_cand.hpp>
#include <game/game/d_random.hpp>
#include <string.h>

// Not split yet (C linkage keeps the target names).
extern "C" {
int fn_80090850(int min, int max);                                    // 80090850: random min..max-1
BOOL fn_8014D07C(dSaveTimeOffset_c *offset);                          // 8014D07C: clock was changed
}

namespace dTheater {

// Dr. Shrunk's programs.
class dSearchProgramSisyo_c : public dSearchCand_c<THEATER_PROGRAM_NUM> {
public:
    virtual BOOL check(int idx); // 8014C2B4
};

// Frillard's programs.
class dSearchProgramKyosyo_c : public dSearchCand_c<THEATER_PROGRAM_NUM> {
public:
    virtual BOOL check(int idx); // 8014C29C
};

// 8014BD88
void dSchedule_c::init() {
    mWeek = 0xFFFF;
    memset(mPrograms, 0, sizeof(mPrograms));
}

// 8014BDA4
void dSchedule_c::setup() {
    dTime_c now = *dTime_c::getCurrent();
    build(getWeek(now));
}

// 8014BE60
u16 getWeek(dTime_c time) {
    dTime_c start;
    start.init();
    int days = dTime_c::diffDays(&time, &start, FALSE) - 1;
    if (days < 0) {
        return 0;
    }
    u16 week = days / 7 + 2;
    return week - 1;
}

// 8014BEDC
void dSchedule_c::update() {
    if (!fn_8014D07C(&dSaveData_c::getTown()->mTimeOffset) || !isValid()) {
        dTime_c now = *dTime_c::getCurrent();
        u16 week = getWeek(now);
        if (week != mWeek || !isValid()) {
            build(week);
        }
    } else {
        // The clock was changed: keep the programs, just take the new week.
        dTime_c now = *dTime_c::getCurrent();
        mWeek = getWeek(now);
    }
}

// 8014C074
void dSchedule_c::build(u16 week) {
    dRandom_c rnd(157);
    int seed[5];
    for (int i = 0; i < 5; i++) {
        seed[i] = fn_80090850(0, 9999);
    }
    rnd.init(seed[0], seed[1], seed[2], seed[3], seed[4]);

    dSearchProgramSisyo_c cand;
    cand.clear();
    cand.search();
    for (int i = 0; i < THEATER_WEEK_PROGRAM_NUM; i++) {
        int program = cand.getRandom(&rnd);
        mPrograms[i] = program + 1;
        cand.remove(program);
    }
    if (rnd.rndF(100.0f) < 5.0f) {
        addKyosyoProgram(&rnd);
    }
    mWeek = week;
}

// 8014C190
void dSchedule_c::addKyosyoProgram(dRandom_c *rnd) {
    dSearchProgramKyosyo_c cand;
    cand.clear();
    cand.search();
    int slot = rnd->rndF(THEATER_WEEK_PROGRAM_NUM);
    mPrograms[(u8)slot] = cand.getRandom(rnd) + 1;
}

// 8014C224
BOOL dSchedule_c::isValid() {
    for (int i = 0; i < THEATER_WEEK_PROGRAM_NUM; i++) {
        if (mPrograms[i] < 1 || mPrograms[i] > THEATER_PROGRAM_NUM) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8014C29C
BOOL dSearchProgramKyosyo_c::check(int idx) {
    return lbl_8047B130[idx] == 0;
}

// 8014C2B4
BOOL dSearchProgramSisyo_c::check(int idx) {
    return lbl_8047B130[idx];
}

} // namespace dTheater
