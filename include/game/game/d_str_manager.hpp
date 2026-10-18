#pragma once

// The structure manager (REL d_str_managerNP, profile STR_MANAGER, RTTI
// dStrm_c). It loads the structure resources (dSTR::brresBank_c) into the str
// heap, creates the building actors of the field and marks the units around
// them. Source: src/d_str_managerNP/d_str_manager.cpp. Names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_allocator.hpp>
#include <nw4r/g3d.h>

class dStrm_c : public fBase_c {
  public:
    virtual int create();   // 0x9A8
    virtual int doDelete(); // 0xA3C
    virtual int execute();  // 0xA2C
    virtual int draw();     // 0xA34

    // 0x120: the actors of the placed buildings (and the city's ATM).
    void createBuildings();
    // 0x2A8: the construction site actors.
    void createBuildSites();
    // 0x3AC: frees the str heaps (and the resources when they don't fit the
    // scene).
    void setupHeap();
    // 0x46C: once; the str heap and the unit attributes of the town.
    void init();
    void createActors();     // 0x6D4
    void createBalloonMng(); // 0x784: the town's balloons
    // 0x7D0: no grass and items on the structure units.
    void clearStrUnits();

    // Debug parameters (the HIO tool); constant in the release build.
    static BOOL isDrawDebug(); // 0xAC0
    static u8 isToyDay();      // 0xAC8: sToyDay
    static BOOL isUseHostIO(); // 0xAD4
    static f32 getOffsetY();   // 0xADC: added to the structures' height
    static f32 getScale();     // 0xAEC: the structures' model scale
    static BOOL isEntryMdl();  // 0xAFC

    static BOOL isLoaded() {
        return sInstance != NULL ? sInstance->mLoaded : FALSE;
    }
    static mAllocator_c *getAllocator() {
        return sInstance != NULL ? &sInstance->mAllocator : NULL;
    }

    // .bss 0x8: the next construction site's number (d_a_str_field).
    static u16 sBuildSiteNo;
    // .bss 0xA: the player entered a structure's door (d_a_str,
    // d_a_str_common; dInsFldBase_c::isSceneChanging). Cleared in doDelete.
    static u8 sDoorEntered;
    // .bss 0xB: dEvent::isActive(EVENT_TOY_DAY) at create.
    static u8 sToyDay;
    // .bss 0x48: set in init, cleared in doDelete.
    static dStrm_c *sInstance;
    // .bss 0x4C: strBrres/obj_str_common.brres (d_a_str).
    static nw4r::g3d::ResFile sCommonBrres;

    /* 0x64 */ u8 _64_0 : 5;
    // The street organ was placed in the city on a visit day (removed in
    // doDelete).
    /* 0x64 */ u8 mOrganPlaced : 1;
    /* 0x64 */ u8 mLoaded : 1;  // the structure resources are loaded (create)
    /* 0x64 */ u8 mCreated : 1; // init ran
    /* 0x68 */ mAllocator_c mAllocator; // the structure models (d_a_bbs_bird)
}; // size 0x84
