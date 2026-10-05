#pragma once

// Bulletin board notices. Source: src/dol/game/d_npc_notice.cpp (.text 800EBB24..800ECD90).
// Automatic notice posting for the town board (see d_notice for the board itself). The file
// name is a working name; function names are inferred.

#include <types.h>
#include <game/game/d_date.hpp>

class dNotice_c;

// ---------------------------------------------------------------------------
// Bulletin board posting (800EBB24..800ECD90). Free functions.

// Posts a BBS notice: message `msgId` from group `group` dated `date` (NULL = now).
void fn_800EBB24(u16 msgId, const char *group, const dTime_c *date, const void *sender); // 800EBB24
void fn_800EBCF4(const dNotice_c *note);                              // 800EBCF4: add a note to the town board
void fn_800EBD3C(u8 idx);                                            // 800EBD3C
dNotice_c *fn_800EBD78(int idx);                                     // 800EBD78
int fn_800EBDB4();                                                   // 800EBDB4: note count
BOOL fn_800EBDE0();                                                  // 800EBDE0
void fn_800EBEA4(int idx);                                           // 800EBEA4
BOOL fn_800EBF14(int idx);                                           // 800EBF14
void fn_800EBF94(const dTime_c *day);                                // 800EBF94: fixed-date notices
void fn_800EC060(const dTime_c *day);                                // 800EC060: birthday notices
void fn_800EC19C(const dTime_c *day);                                // 800EC19C: event notices
void fn_800EC4B0(const dTime_c *day);                                // 800EC4B0
void fn_800EC4F8(const dTime_c *day);                                // 800EC4F8: Tom Nook notice
void fn_800EC72C(const dTime_c *day);                                // 800EC72C
void fn_800EC77C(const dTime_c *day);                                // 800EC77C: all notices for a day
void fn_800EC7C8(const dTime_c *now);                                // 800EC7C8: catch up since the last visit
void fn_800ECC78();                                                  // 800ECC78
void fn_800ECCB4(int unused, const void *data);                      // 800ECCB4: net receive (0x19A bytes)
void fn_800ECD08(const void *data);                                  // 800ECD08: net send
void fn_800ECD4C(int unused, const u8 *data);                        // 800ECD4C
void fn_800ECD54(u8 idx);                                            // 800ECD54
