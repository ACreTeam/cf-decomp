#pragma once

// Human/npc body animation tables and the shared body animation archive "/Anm/Anm.brcha".
// Source: src/dol/game/d_hmn_anm.cpp (.text 800B6350..800B6750). Body chr animation ids are
// < HMN_ANM_NUM (0x1BC = none). All names are inferred.

#include <types.h>
#include <game/game/d_dvd.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/mLib/m_mtx.hpp>
#include <nw4r/g3d.h>
#include <nw4r/math.h>

#define HMN_ANM_NUM 0x1BC  // also the "no animation" id
#define HMN_ANM_TOOL_NUM 14 // tool types (see fn_800BA890)

// The body animation archive (one static instance, 80598240, loaded into dHeap::hmnAnmHeap_p).
class dHmnAnm_c {
public:
    dHmnAnm_c();  // 800B6350
    ~dHmnAnm_c(); // 800B6384

    static u32 getHeapSize();                                     // 800B63E0: 0x133B00
    static nw4r::g3d::ResAnmChr getResAnmChr(int anmId);          // 800B63EC: null if none / not loaded
    static BOOL load();                                           // 800B643C: TRUE once loaded (or no heap)
    static u32 getType(u32 anmId);                                // 800B64B8: per-animation type (4 if out of range)
    static int getToolAnmId(int toolType);                        // 800B64DC: HMN_ANM_NUM for none / >= 14
    static int getItemToolAnmId(const dItem::Item *item);         // 800B6500
    static nw4r::g3d::ResAnmChr getItemToolResAnmChr(const dItem::Item *item); // 800B6524
    static BOOL isCurve(int anmId);                               // 800B6558: curve the root node
    static void curveMtx(mMtx_c *out, const mMtx_c *in, f32 baseY); // 800B6580: round-world curve
    static BOOL getFlag(int anmId);                               // 800B66A0
    static int getPlayMode(u32 anmId);                             // 800B66C8: default play mode (4 if out of range)
    static u8 getSubIdx(u32 anmId);                               // 800B66E8: dense index of some animations (0x76 = none)

    /* 0x00 */ void *mData;
    /* 0x04 */ dDvd::loader_c mLoader;
}; // size 0x18

