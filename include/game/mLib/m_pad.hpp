#pragma once

// Based on the Skyward Sword decompilation (zeldaret/ss), m/m_pad.h
#include <types.h>
#include <lib/egg/core/eggController.h>

namespace mPad {

extern EGG::CoreControllerMgr *g_padMg;
extern int g_currentCoreId;
extern EGG::CoreController *g_currentCore;
extern EGG::CoreController *g_core[4];

inline EGG::CoreController *getCore(const int i) {
    return g_core[i];
}
inline EGG::CoreController *getCore() {
    return g_currentCore;
}
inline EGG::CoreControllerMgr *getMgr() {
    return g_padMg;
}
inline int getCurrentCoreID() {
    return g_currentCoreId;
}

void create();
void beginPad();
void endPad();

} // namespace mPad
