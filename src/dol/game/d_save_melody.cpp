// The town tune (dSaveMelody_c). .text 8011927C..80119444.
#include <game/game/d_save_melody.hpp>
#include <cstring>

// Not split yet (C linkage keeps the target names).
extern "C" {
const u8 *fn_8000F9A0(); // 8000F9A0: the default tune
}

// 8011927C
dSaveMelody_c::dSaveMelody_c() {}

// 80119280
dSaveMelody_c::~dSaveMelody_c() {}

// 801192C0
dSaveMelody_c *dSaveMelody_c::copy(const dSaveMelody_c *other) {
    memcpy(this, other, sizeof(dSaveMelody_c));
    return this;
}

// 801192F4
void dSaveMelody_c::clear() {
    memset(this, 0, sizeof(dSaveMelody_c));
    set(fn_8000F9A0());
}

// 80119338
void dSaveMelody_c::set(const u8 *notes) {
    for (int i = 0; i < SAVE_MELODY_NOTE_NUM; i++) {
        mNotes[i] = notes[i];
    }
}

// 801193BC
void dSaveMelody_c::get(u8 *notes) {
    for (int i = 0; i < SAVE_MELODY_NOTE_NUM; i++) {
        notes[i] = mNotes[i];
    }
}

// 80119440
u8 *dSaveMelody_c::getNotes() {
    return mNotes;
}
