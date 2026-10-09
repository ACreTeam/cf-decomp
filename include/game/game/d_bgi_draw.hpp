#pragma once
#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>

// The interior (room) bg drawer, d_bgi_drawNP (REL). Only what the DOL needs is declared: the RTTI
// base list of "dBgiDraw_c" is fBase_c at 0x00, dDvd::brresBank_c at 0x64 and dBGI::alwaysAc_c at
// 0xBC. dBgiDraw_c registers itself with dBgUtil::setBgiDraw (d_bg_util.cpp) while it exists.

class mVec3_c;

namespace dBGI {

// The room's wallpaper / carpet and model queries that the DOL reaches through dBgUtil (RTTI
// "dBGI::alwaysAc_c"). Slot order from dBgiDraw_c's vtable (the REL functions in the comments).
class alwaysAc_c {
public:
    // 0x08 (fn_107_A68): the wallpaper; NONE while it is an item of kind 0x16
    virtual dItem::Item getWallpaper() const;
    // 0x0C (fn_107_8B8): request a new wallpaper; type is replaced by the item's kind 1/2
    virtual BOOL setWallpaper(dItem::Item item, int type, int flag0, int flag1);
    // 0x10 (fn_107_53C4): no wallpaper change pending
    virtual BOOL isWallpaperDone() const;
    // 0x14 (fn_107_AF0): the carpet; NONE while it is an item of kind 0x16
    virtual dItem::Item getCarpet() const;
    // 0x18 (fn_107_B78): the carpet as set
    virtual dItem::Item getCarpetRaw() const;
    // 0x1C: request a new carpet (as setWallpaper)
    virtual BOOL setCarpet(dItem::Item item, int type, int flag0, int flag1);
    // 0x20 (fn_107_543C): no carpet change pending
    virtual BOOL isCarpetDone() const;
    // 0x24 (fn_107_1E64): world position of a model node of a block
    virtual BOOL getNodePos(mVec3_c *pos, int blockX, int blockZ, const char *name);
    virtual int vf28(int blockX, int blockZ);                          // 0x28 (fn_107_1F24)
    virtual int vf2C(int arg, int blockX, int blockZ, int arg2);      // 0x2C (fn_107_1F8C)
    virtual void *vf30();                                              // 0x30 (fn_107_5484)
    // 0x34 (fn_107_2080): the room model is ready
    virtual BOOL isReady();
};

} // namespace dBGI

class dBgiDraw_c : public fBase_c, public dDvd::brresBank_c, public dBGI::alwaysAc_c {};
