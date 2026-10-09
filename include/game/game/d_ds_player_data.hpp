#pragma once
#include <types.h>

// Animal Crossing: Wild World save data (src/dol/game/d_ds_player_data.cpp, .text 80085388..80085E8C;
// see notes/d_ds_player_data.txt). Used by the player select NPC (d_a_npc_sp_player_selectNP) to
// read a Wild World character sent from the DS: the save image holds two copies of the save, each
// with the town name and four players. Every offset depends on the region of the DS game, which is
// taken to be the console's (getRegion(): 0 Japan, 1 America, 2 Europe, 3 Korea).
// No RTTI or strings name any of this; all names are inferred from behaviour.

namespace dScript {
class Word_c;
}

namespace dDsPlayerData {

int getDsRegion();                                        // 80085388
u16 swap16(u16 value);                                    // 8008538C: DS data is little endian
u16 calcChecksum(const u16 *data, int size);              // 8008539C: sum of the u16s
u16 convertKrChar(u16 c);                                 // 80085400

// One player of a save (the pointer from getPlayer).
void getPlayerName(const u8 *player, dScript::Word_c *word); // 80085484
u8 getOption(const u8 *player);                           // 800855C8: option bit 0 (dSaveOption_c::mBit7)
u8 getHairColor(const u8 *player);                        // 80085610
u8 getHair(const u8 *player);                             // 80085658: converted to a City Folk hair style
u8 getFace(const u8 *player);                             // 80085734
u16 getTownId(const u8 *player);                          // 8008577C
u16 getPlayerId(const u8 *player);                        // 800857C4
u8 getGender(const u8 *player);                           // 8008580C
const u8 *getCatalog(const u8 *player);                   // 80085850: addCatalogItems() source
BOOL isValidPlayer(const u8 *player);                     // 80085894: town and player id set

// One copy of the save (the pointer from getSave / selectSave).
int countPlayers(const u8 *save);                         // 800858F0
int findPlayer(const u8 *save, u32 n);                    // 8008595C: index of the n-th valid player, or -1
const u8 *getPlayer(const u8 *save, int idx);             // 800859D8
void getTownName(const u8 *save, dScript::Word_c *word);  // 80085A3C
u8 getState(const u8 *save);                              // 80085B80
u8 getSaveCount(const u8 *save);                          // 80085BC4: compared to pick the newer copy
BOOL isChecksumOK(const u8 *save);                        // 80085C08
BOOL isState2(const u8 *save);                            // 80085C58
u32 isUsable(const u8 *save);                             // 80085C88: state is neither 2 nor 0x1C

// The whole save image (two copies).
void clearBuffer(u8 *buf);                                // 80085CE0: 0x40000 bytes
u8 *getSave(u8 *buf, int idx);                            // 80085CEC
u8 *selectSave(u8 *buf);                                  // 80085D3C: the copy to use, or NULL

} // namespace dDsPlayerData
