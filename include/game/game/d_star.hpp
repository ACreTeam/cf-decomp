#pragma once

// Shooting stars, part of the sky actor (dSky_c + 0x1E8). Sources: src/dol/game/d_star_mgr.cpp,
// src/dol/game/d_star.cpp. Names are inferred (no RTTI).

#include <types.h>

#define STAR_NUM 16

class dStar_c {
public:
    dStar_c() : mWait(0), mActive(false) {}

    void start(int wait); // 80090D20
    void execute();       // 80090D70
    int countDown();      // 80090E10

    /* 0x0 */ int mWait;
    /* 0x4 */ bool mActive;
    /* 0x8 */ f32 mX;
    /* 0xC */ f32 mY;
}; // size 0x10

class dStarMgr_c {
public:
    void init();    // 80090964
    void execute(); // 80090A40

    /* 0x000 */ dStar_c mStars[STAR_NUM];
    /* 0x100 */ int mLastSec;
    /* 0x104 */ bool mEnabled;
}; // size 0x108
