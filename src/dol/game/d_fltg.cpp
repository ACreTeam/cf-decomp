// The flying targets list (dFltg). Proposed TU: .text 800A7368..800A75B0, .bss 8058B6D8..8058B6F0, .sdata2 80750728..80750730.
// See include/game/game/d_fltg.hpp.
#include <game/game/d_fltg.hpp>

namespace dFltg {

const f32 cFlyHeight = 112.0f;

static actor_c *l_actor[FLTG_MAX];

// 800A7368
void clear() {
    for (int i = 0; i < FLTG_MAX; i++) {
        l_actor[i] = nullptr;
    }
}

// 800A738C
BOOL add(actor_c *actor) {
    if (actor != nullptr) {
        for (int i = 0; i < FLTG_MAX; i++) {
            if (l_actor[i] == actor) {
                return TRUE;
            }
        }
    }
    for (int i = 0; i < FLTG_MAX; i++) {
        if (l_actor[i] == nullptr) {
            l_actor[i] = actor;
            return TRUE;
        }
    }
    return FALSE;
}

// 800A7448
BOOL remove(actor_c *actor) {
    if (actor != nullptr) {
        for (int i = 0; i < FLTG_MAX; i++) {
            if (l_actor[i] == actor) {
                l_actor[i] = nullptr;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 800A74A0
u32 getCount() {
    u32 count = 0;
    for (int i = 0; i < FLTG_MAX; i++) {
        if (l_actor[i] != nullptr) {
            count++;
        }
    }
    return count;
}

// 800A74FC
actor_c *getNth(u32 n) {
    u32 count = 0;
    for (int i = 0; i < FLTG_MAX; i++) {
        if (l_actor[i] != nullptr) {
            if (count == n) {
                return l_actor[i];
            }
            count++;
        }
    }
    return nullptr;
}

// 800A7554
int getOtherRaccoIdx(actor_c *self) {
    for (int i = 0; i < FLTG_MAX; i++) {
        if (l_actor[i] != nullptr && l_actor[i] != self) {
            return l_actor[i]->getRaccoIdx();
        }
    }
    return -1;
}

} // namespace dFltg
