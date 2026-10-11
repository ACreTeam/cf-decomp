// Wii Remote rumble. .text 800FAB0C..800FABF4, .data 804ED1F0..804ED830, .sbss 8074E5D8..8074E5E0.
// See include/game/game/d_rumble.hpp.
#include <game/game/d_rumble.hpp>
#include <game/game/d_heap.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_pad.hpp>
#include <lib/revolution/WPAD/WPAD.h>

namespace dRumble {

#define PATTERN_NUM 40

static Pattern_s sPatterns[PATTERN_NUM] = {
    {"***-*-*--*--*---*---*", 0}, // 0
    {"****-*", 0}, // 1
    {"****", 0}, // 2
    {"****", 0}, // 3
    {"*****", 0}, // 4
    {"*******-*", 0}, // 5
    {"********-*", 0}, // 6
    {"**********-*", 0}, // 7
    {"*--*--", -1}, // 8
    {"*-*-*-", -1}, // 9
    {"**-*--", -1}, // 10
    {"*******", 0}, // 11
    {"******-*-*--*", 0}, // 12
    {"********-*", 0}, // 13
    {"*****-*--*---*", 0}, // 14
    {"*****-*--*", 0}, // 15
    {"******-*-*", 0}, // 16
    {"*******-*", 0}, // 17
    {"******-*--*", 0}, // 18
    {"******-*", 0}, // 19
    {"*****-*--*---*", 0}, // 20
    {"***-*-------**-*", 0}, // 21
    {"******", 0}, // 22
    {"****-*", 0}, // 23
    {"*****-*-*--*", 0}, // 24
    {"***-*--*---*", 0}, // 25
    {"****", 0}, // 26
    {"*****", 0}, // 27
    {"*****", 0}, // 28
    {"***-*-------------", -1}, // 29
    {"****-*", 0}, // 30
    {"****-------***", 0}, // 31
    {"******", 0}, // 32
    {"********-*--*", 0}, // 33
    {"*****-*-*--*", 0}, // 34
    {"***-*", 0}, // 35
    {"******-*-*--*--*", 0}, // 36
    {"***-*", 0}, // 37
    {"*****-*", 0}, // 38
    {"", 0}, // 39
};

static u8 sDisabled; // 8074E5D8

// 800FAB0C
void create(u8 num) {
    mHeap::saveCurrentHeap();
    mHeap::setCurrentHeap(dHeap::rumbleHeap_p);
    mPad::g_padMg->getNthController(0)->createRumbleMgr(num);
    mHeap::restoreCurrentHeap();
    sDisabled = FALSE;
}

// 800FAB60
void stop() {
    mPad::g_core[0]->stopRumbleMgr();
}

// 800FAB6C
void start(int pattern, bool force) {
    if (WPADIsMotorEnabled() && sDisabled != TRUE) {
        mPad::g_core[0]->startPatternRumble(sPatterns[pattern].mPattern, sPatterns[pattern].mFrames, force);
    }
}

// 800FABDC
void disable() {
    sDisabled = TRUE;
}

// 800FABE8
void enable() {
    sDisabled = FALSE;
}

} // namespace dRumble
