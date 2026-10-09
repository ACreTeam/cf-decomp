#pragma once
#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>

// Bg helpers (src/dol/game/d_bg_util.cpp, .text 800767A8..80077754; see notes/d_bg_util.txt).
// No RTTI or strings name them except "sign.bin"; the names are inferred from what they do.
//
// - Signs: the bg model brres of a block carries "sign.bin" in its External folder; createSigns
//   spawns one sign actor (profile 0xF6, d_a_sign_tagNP) per entry. The actor reads its message,
//   size and flag back through getSignMsg / getSignSize / getSignFlag while it is created.
// - The room bg (dBgiDraw_c, d_bgi_drawNP) while it exists: its wallpaper / carpet and model
//   queries, with defaults when there is none. Wallpaper / carpet changes can be sent to the other
//   players (sendRoomItem, packet 0x3D; recvRoomItem is the handler).
// - The wallpaper / carpet of each scene: the player house rooms keep theirs in the save data
//   (dHomeRoom_c), every other scene in a table here.

class mVec3_c;
class dBgiDraw_c;

namespace dBgUtil {

// The net packet of a wallpaper / carpet change (4 bytes).
struct roomItemMsg_c {
    /* 0x0 */ u16 mItem; // dItem::Item id
    /* 0x2 */ u16 mScene : 7;
    /* 0x2 */ u16 mIsWallpaper : 1;
    /* 0x2 */ u16 mType : 2;
    /* 0x2 */ u16 mFlag1 : 1;
    /* 0x2 */ u16 mFlag0 : 1;
    /* 0x2 */ u16 : 4;
}; // size 0x4

u16 getSignMsg();                                                                      // 800767A8
const mVec3_c *getSignSize();                                                          // 800767B0
u8 getSignFlag();                                                                      // 800767BC
void createSign(const mVec3_c *pos, const mVec3_c *size, s16 angleY, int msg, int flag); // 800767C4
void createSigns(nw4r::g3d::ResFile res, int blockX, int blockZ);                      // 80076840

void setBgiDraw(dBgiDraw_c *draw);                                                     // 80076A64
void sendRoomItem(dItem::Item item, int type, int flag0, BOOL isWallpaper, int flag1);  // 80076A6C
dItem::Item getWallpaper();                                                            // 80076AFC
BOOL setWallpaper(dItem::Item item, int type, int flag0, int flag1, BOOL send);         // 80076B30
BOOL setDesignWallpaper(int slot, BOOL type2, int flag1, BOOL send);                     // 80076BE8
BOOL isWallpaperDone();                                                                // 80076C84
dItem::Item getCarpet();                                                               // 80076CB0
dItem::Item getCarpetRaw();                                                            // 80076CE4
BOOL setCarpet(dItem::Item item, int type, int flag0, int flag1, BOOL send);            // 80076D18
BOOL setDesignCarpet(int slot, BOOL type2, int flag1, BOOL send);                        // 80076DD0
BOOL isCarpetDone();                                                                   // 80076E6C
void recvRoomItem(const roomItemMsg_c *msg);                                           // 80076E98
BOOL getNodePos(mVec3_c *pos, const char *name, int blockX, int blockZ);                // 80076FB8
BOOL isBgiReady();                                                                     // 800770B0

void initSceneItems();                                                                 // 800770DC
BOOL setSceneWallpaper(u8 scene, dItem::Item item, int type);                          // 800771D8
dItem::Item getSceneWallpaper(u8 scene, int *type);                                    // 800772D8
BOOL setSceneCarpet(u8 scene, dItem::Item item, int type);                             // 800773E4
dItem::Item getSceneCarpet(u8 scene, int *type);                                       // 800774DC

} // namespace dBgUtil
