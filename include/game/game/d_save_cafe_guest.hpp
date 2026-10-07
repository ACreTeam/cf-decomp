#pragma once

#include <types.h>

// Who sits at the cafe (d_a_npc_sp_cafe_guest). The actor turns 0..4 into special NPC ids, whose low byte
// is the model in Npc/Special/Model; each has a message sheet npc_SP/NPC_cafe_*.
enum {
    CAFE_GUEST_PELLY,    // 0: 0x800A, model "pga", NPC_cafe_periko (mornings)
    CAFE_GUEST_PHYLLIS,  // 1: 0x800B, model "pgb", NPC_cafe_perimi (nights but Saturday's)
    CAFE_GUEST_KAPPN,    // 2: 0x8014, model "wip", NPC_cafe_kappei
    CAFE_GUEST_RESETTI,  // 3: 0x805C, model "mof", NPC_cafe_reset (Sundays)
    CAFE_GUEST_DON,      // 4: 0x805D, model "mod", NPC_cafe_racket (Saturdays, once player flag0 0x61 is set)
    CAFE_GUEST_NONE,     // 5
    CAFE_GUEST_TOTAKEKE, // 6: Saturday night; K.K. Slider plays (not spawned by the cafe guest actor)
};

// The cafe's guest schedule (d_a_npc_sp_cafe_guest): dSaveShops_c::mCafeGuest. Rolled each day; slot n
// is the guest of one time of day (see getCurrentGuest). Source: src/dol/game/d_save_cafe_guest.cpp.
class dSaveCafeGuest_c { // 0x4
public:
    void clear();              // 80149608
    void update();             // 80149650: daily, rolls the slots
    u8 getCurrentGuest();      // 801498DC: by the time of day; CAFE_GUEST_NONE if not rolled today
    u8 getGuest(int slot);     // 80149A8C: CAFE_GUEST_NONE if not rolled today

    /* 0x0 */ u8 mGuests[4]; // CAFE_GUEST_*: morning, noon, afternoon, night
};
