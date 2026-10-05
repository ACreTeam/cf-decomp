#pragma once

// Game-side random generator: a cRandom_c seeded from a few values (often a date plus the town
// id), then advanced a seed-dependent number of steps. Source: src/dol/game/d_random.cpp
// (.text 80107D6C..80107F00). The class and function names are inferred (no RTTI).
// See notes/d_random.txt.

#include <types.h>
#include <game/cLib/c_random.hpp>
#include <game/game/d_date.hpp>

class dRandom_c : public cRandom_c {
public:
    dRandom_c(u32 seed) : cRandom_c(seed) {}

    void init(int a, int b, int c, int d, int e);                // 80107D6C: seed a + .. + e, then skip
    void initFromDateAndLandId(const dTime_c &time, int salt);   // 80107DF4: date + this town's id
    void initFromDateWithSalt2(const dTime_c &time, int salt, int salt2); // 80107E68: date + salt2 + salt
    void initFromDateWithSalt(const dTime_c &time, int salt);    // 80107E94: date + 0x90C1 + salt
    float rnd();                                                 // 80107EC4: 0..1
    float rndF(float max);                                       // 80107EC8: 0..max
};
