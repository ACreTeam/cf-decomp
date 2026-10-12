#pragma once

// d_sv_lang_flag (DOL TU d_sv_lang_flag.cpp, .text 80117118..80117400): per player, which of the downloaded
// per-language villager messages (dSaveExtra_c::mDistContent, one slot per personality) are still to be told
// (dSaveExtra_c::mLangFlags, +0x1CE). update() sets the flags of the slots that have content; a villager clears
// its personality's flag once it told the message (d_a_npc; dAcNpcNml_c::talk_c::msgReDownload gets it).
// No RTTI: names inferred.

#include <types.h>

struct dSaveDistContent_c;
struct dSaveDistContentList_c;

namespace dSvLangFlag {

enum {
    LANG_FLAG_NUM = 6,          // one per villager personality (looks)
    LANG_FLAG_LANGUAGE_CURRENT = 10, // LANGUAGE_NUM: the console's language (getLanguage())
};

struct Flag_c {
    Flag_c();                                       // 80117118
    void clear(u32 player);                         // 8011711C
    void clearAll();                                // 80117130
    // Sets player's flags of list's slots with content in lang (list NULL: the save's mDistContent).
    void update(u32 player, const dSaveDistContentList_c *list, int lang); // 8011717C
    void updateAll();                               // 80117244: every player, the save's list, the console's language
    void set(u32 player, u8 looks);                 // 80117298
    void reset(u32 player, u8 looks);               // 801172C0
    BOOL isSet(u32 player, u8 looks) const;         // 801172E8
    // The downloaded message of personality looks in lang when player's flag is set, else NULL.
    const dSaveDistContent_c *get(u32 player, u8 looks, int lang) const; // 80117318

    /* 0x0 */ u8 mFlags[4]; // per player: 1 << looks
}; // size 0x4

} // namespace dSvLangFlag
