#pragma once

// Theater weekly program schedule: Dr. Shrunk ("Sisyo", シショー) performs 4 of his 23 programs
// a week; rarely one is swapped for one of Frillard's ("Kyosyo", キョショー) 6. The schedule is
// saved at dSaveExtra_c+0x1C8. Source: src/dol/game/d_theater.cpp (.text 8014BD88..8014C2C4).
// The namespace is from the RTTI (dTheater::dSearchProgram*_c); dSchedule_c and the function
// names are inferred. See notes/d_theater.txt.

#include <types.h>
#include <game/game/d_date.hpp>

class dRandom_c;

#define THEATER_PROGRAM_NUM 29   // Dr. Shrunk's and Frillard's programs together
#define THEATER_WEEK_PROGRAM_NUM 4

namespace dTheater {

// Program flags (8047B130, in another TU; also read by fn_8016CD8C for d_a_npc_sp_sisyoNP):
// nonzero for Dr. Shrunk's programs, 0 for Frillard's.
extern "C" const u8 lbl_8047B130[];

u16 getWeek(dTime_c time); // 8014BE60: weeks since 2000-01-01 (2000-01-01 itself is week 0)

// No ctor: dSaveData_c::create does not construct it; the save init (80116900) calls init().
class dSchedule_c {
public:
    void init();                       // 8014BD88: no week, no programs
    void setup();                      // 8014BDA4: build for the current week
    void update();                     // 8014BEDC: daily; rebuild when the week changes
    void build(u16 week);              // 8014C074
    void addKyosyoProgram(dRandom_c *rnd); // 8014C190: Frillard replaces one slot
    BOOL isValid();                    // 8014C224: every slot 1..THEATER_PROGRAM_NUM

    /* 0x0 */ u16 mWeek;                                 // 0xFFFF: none
    /* 0x2 */ u8 mPrograms[THEATER_WEEK_PROGRAM_NUM];    // program index + 1
}; // size 0x6

} // namespace dTheater
