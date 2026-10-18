#pragma once

// d_rumble (DOL TU d_rumble.cpp, .text 800FAB0C..800FABF4): Wii Remote rumble. Plays rumble patterns from a table
// on controller 0 through EGG's rumble manager. No RTTI: the names are inferred (the TU was called d_pad_speaker
// by the first split pass).

#include <types.h>

namespace dRumble {

// The patterns of sPatterns, named by their users (d_a_player, d_uki).
enum Pattern_e {
    PATTERN_ROD_CAST = 0x00,         // the cast lands (SE_RodStroke)
    PATTERN_ROD_NIBBLE = 0x01,       // the float twitches in the water
    PATTERN_FLOAT_NIBBLE_S = 0x02,   // d_uki: a fish nibbles, by its size (S / M / L)
    PATTERN_FLOAT_NIBBLE_M = 0x03,
    PATTERN_FLOAT_NIBBLE_L = 0x04,
    PATTERN_FLOAT_SINK_S = 0x05,     // d_uki: the float sinks (the bite), by the fish size
    PATTERN_FLOAT_SINK_M = 0x06,
    PATTERN_FLOAT_SINK_L = 0x07,
    PATTERN_HOOKED_S = 0x08,         // the fish is hooked, by its size
    PATTERN_HOOKED_M = 0x09,
    PATTERN_HOOKED_L = 0x0A,
    PATTERN_AXE_CHOP = 0x0B,
    PATTERN_AXE_FELL = 0x0C,         // the tree falls
    PATTERN_AXE_HIT = 0x0D,
    PATTERN_AXE_BOUNCE = 0x0E,       // the axe bounces off an object
    PATTERN_DIG = 0x0F,
    PATTERN_DIG_UP_ITEM = 0x10,
    PATTERN_DIG_STUMP = 0x11,
    PATTERN_DIG_STUMP_OUT = 0x12,
    PATTERN_SHOVEL_HIT = 0x13,
    PATTERN_SHOVEL_BOUNCE = 0x14,    // the shovel bounces off an object / a rafflesia
    PATTERN_BURY = 0x15,
    PATTERN_BURY_END = 0x16,
    PATTERN_NET_MISS = 0x17,
    PATTERN_NET_CATCH = 0x18,
    PATTERN_SLINGSHOT_AIM = 0x19,
    PATTERN_SLINGSHOT_SHOOT = 0x1A,
    PATTERN_SLINGSHOT_SHOOT_SILVER = 0x1B,
    PATTERN_SLINGSHOT_SHOOT_GOLD = 0x1C,
    PATTERN_SHAKE_TREE = 0x1D,
    PATTERN_PULL_WEED = 0x1E,
    PATTERN_DOOR = 0x1F,
    PATTERN_CRACKER = 0x20,
    PATTERN_TRIP = 0x21,
    PATTERN_PITFALL_FALL = 0x22,
    PATTERN_PITFALL_STRUGGLE = 0x23,
    PATTERN_PITFALL_EXIT = 0x24,
    PATTERN_TRAMPLE_FLOWER = 0x25,
    PATTERN_TOOL_SWING = 0x26,
};

// A rumble pattern: '*' rumbles for a frame, '-' pauses.
struct Pattern_s {
    /* 0x00 */ char mPattern[0x24];
    /* 0x24 */ int mFrames; // EGG::CoreController::startPatternRumble: 0 plays it once, -1 repeats it
}; // size 0x28

void create(u8 num);              // 800FAB0C: creates controller 0's rumble manager (in dHeap::rumbleHeap_p)
void stop();                      // 800FAB60: stops controller 0's rumble (d_wifi_err / d_caution opening)
void start(int pattern, bool force); // 800FAB6C: plays sPatterns[pattern] when rumble is on and not disabled
void disable();                   // 800FABDC: (d_reset)
void enable();                    // 800FABE8: (d_reset)

} // namespace dRumble
