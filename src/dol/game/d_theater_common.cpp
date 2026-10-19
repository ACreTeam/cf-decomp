// The theater's shared state (dTheater::common_c). .text 8016CCCC..8016CEF8.
// See include/game/game/d_theater_common.hpp.
#include <game/game/d_theater_common.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_random.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

namespace dTheater {

common_c g_common;

// 8016CCCC
common_c::common_c() : mProgram(0), mTicket(0), mOpen(0) {
    memset(_5, 0, sizeof(_5));
}

// 8016CD24
common_c::~common_c() {}

// 8016CD64
void common_c::clear() {
    mProgram = 0;
    mTicket = 0;
    mOpen = 0;
    memset(_5, 0, sizeof(_5));
}

// 8016CD84
void common_c::setProgram(u8 program) {
    mProgram = program;
}

// 8016CD8C
BOOL common_c::isKyosyoProgram() const {
    if (mProgram >= 1 && mProgram <= THEATER_PROGRAM_NUM) {
        return lbl_8047B130[mProgram - 1] == 0;
    }
    return FALSE;
}

// 8016CDC4
u8 common_c::drawSisyoProgram() {
    dRandom_c rnd(157);
    int seed[5];
    for (int i = 0; i < 5; i++) {
        seed[i] = cM::rndRange(0, 9999);
    }
    rnd.init(seed[0], seed[1], seed[2], seed[3], seed[4]);

    dSearchProgramSisyo_c cand;
    cand.clear();
    cand.search();
    return cand.getRandom(&rnd) + 1;
}

// 8016CE80
void common_c::setTicket(u8 on) {
    mTicket = on;
    if (on) {
        setOpen(0);
    }
}

// 8016CE98
void common_c::setOpen(u8 on) {
    mOpen = on;
    if (on) {
        setTicket(0);
    }
}

} // namespace dTheater
