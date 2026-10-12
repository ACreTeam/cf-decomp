#pragma once

// Scene data (dol/game/sc_data/d_sc_data_*.cpp): one dSceneData_c per scene (sScData*, see d_scene.hpp), built from
// a scene's own base process / resource lists plus the shared lists below (d_sc_data_common.cpp, a data-only TU:
// .data 804F4BB0..804F4D80, .sdata 8074B5F0..8074B5F8, .sdata2 807513A8..807513B8).
// The shared counts are extern const globals, so each scene's __sinit copies them into its dSceneDataEntry_c::mNum
// at run time. Names are inferred; the profile ids are raw (CF's profile name list isn't recovered).

#include <types.h>
#include <game/game/d_scene.hpp>
#include <revolution/GX.h>

// Scene descriptors consumed by d_scene.cpp. Each is stored in the scene's .sdata.
extern dSceneData_c *sScDataField;
extern dSceneData_c *sScDataRmHs0F1;
extern dSceneData_c *sScDataRmHs0F2;
extern dSceneData_c *sScDataRmHs0Bm;
extern dSceneData_c *sScDataRmHs0Loft;
extern dSceneData_c *sScDataRmNpc0;
extern dSceneData_c *sScDataRmOffice;
extern dSceneData_c *sScDataRmChkp;
extern dSceneData_c *sScDataRmMmEnt;
extern dSceneData_c *sScDataRmMmFish;
extern dSceneData_c *sScDataRmMmIns;
extern dSceneData_c *sScDataRmMmFossil0;
extern dSceneData_c *sScDataRmMmFossil1;
extern dSceneData_c *sScDataRmMmPic0;
extern dSceneData_c *sScDataRmMmPic1;
extern dSceneData_c *sScDataRmMmCafe;
extern dSceneData_c *sScDataRmMmAstro;
extern dSceneData_c *sScDataRmShop0;
extern dSceneData_c *sScDataRmShop1;
extern dSceneData_c *sScDataRmShop2;
extern dSceneData_c *sScDataRmShop3F1;
extern dSceneData_c *sScDataRmShop3F2;
extern dSceneData_c *sScDataRmTailor;
extern dSceneData_c *sScDataTown;
extern dSceneData_c *sScDataRmBroker;
extern dSceneData_c *sScDataRmBarber;
extern dSceneData_c *sScDataRmFortune;
extern dSceneData_c *sScDataRmGrace;
extern dSceneData_c *sScDataRmHappyEnt;
extern dSceneData_c *sScDataRmHappyMdl;
extern dSceneData_c *sScDataRmTheaterEnt;
extern dSceneData_c *sScDataRmTheaterShow;
extern dSceneData_c *sScDataRmAuction;
extern dSceneData_c *sScDataRmReset;
extern dSceneData_c *sScDataDmTitle;
extern dSceneData_c *sScDataDmSave;
extern dSceneData_c *sScDataDmPlSel;
extern dSceneData_c *sScDataDmBusPlCrt;
extern dSceneData_c *sScDataCheckField;
extern dSceneData_c *sScDataRmSample;

namespace dScData {

// Plain-data mirrors of the scene records (d_scene.hpp's types hold mVec3_c / mAng3_c, which have constructors, so the
// tables are written with these and cast where d_scene's types are expected).

// SCENE_DATA_PLAYER: the player's start (dScenePlayerData_c).
struct PlayerData_c {
    /* 0x00 */ f32 mPos[3];
    /* 0x0C */ s16 mAngle[3];
    /* 0x12 */ u16 _12;
    /* 0x14 */ u32 mType;
}; // size 0x18

// SCENE_DATA_ACTOR: an actor placed in the scene. mParam is the actor's parameter: a value, or a pointer (a
// PanelData_c for the museum / room panels, an EntParam_c for the entrance actors).
struct ActorData_c {
    /* 0x00 */ u16 mProfile;
    /* 0x02 */ u16 _02;
    /* 0x04 */ f32 mPos[3];
    /* 0x10 */ s16 mAngle[3];
    /* 0x16 */ u16 _16;
    /* 0x18 */ void *mParam;
}; // size 0x1C

// A panel actor's parameter: its item ids and the layout pane it shows them on.
struct PanelData_c {
    /* 0x00 */ u32 mKind;
    /* 0x04 */ s32 mRoom; // -1: none
    /* 0x08 */ u32 *mIds;
    /* 0x0C */ u32 mCount;
    /* 0x10 */ char *mName;
    /* 0x14 */ u32 mFlags;
}; // size 0x18

// SCENE_DATA_3 (scene settings): a kind and its data (a pointer to a record, or a value for some kinds).
struct Scene3Entry_c {
    /* 0x00 */ u32 mKind;
    /* 0x04 */ void *mData;
};
// Scene setting kind 5: a colour and a byte.
struct ColorByte_c {
    /* 0x0 */ GXColor mColor;
    /* 0x4 */ u8 mValue;
}; // size 0x5

// An exit (dSceneExitData_c).
struct ExitData_c {
    /* 0x00 */ u8 mScenes[4];
    /* 0x04 */ f32 mPos[3];
    /* 0x10 */ s16 mAngle;
    /* 0x12 */ u16 _12;
    /* 0x14 */ u32 mType;
    /* 0x18 */ u8 mPosType;
    /* 0x19 */ u8 _19;
    /* 0x1A */ s16 mPosAngle;
}; // size 0x1C

// The scene's exit info (dSceneData_c's extra word): a list of ids and flags.
struct ExitInfo_c {
    /* 0x0 */ u16 *mList;
    /* 0x4 */ u32 mFlags;
};

// A scene's dSceneData_c and its exit info.
struct SceneData_c {
    /* 0x00 */ dSceneData_c mData;
    /* 0x0C */ ExitInfo_c *mExitInfo;
}; // size 0x10

// Base processes and resources of every scene.
extern dSceneBaseData_c g_commonBase[]; // 804F4BB0
extern const u16 g_commonBaseNum;       // 807513A8: 21
extern u16 g_commonRes[];               // 8074B5F0
extern const u16 g_commonResNum;        // 807513AA: 2

// The field scenes (d_sc_data_field, d_sc_data_dm_title).
extern dSceneBaseData_c g_fieldBase[]; // 804F4C58
extern const u16 g_fieldBaseNum;       // 807513AC: 10
extern u16 g_fieldRes[];               // 804F4CA8
extern const u16 g_fieldResNum;        // 807513AE: 21

// The room scenes (d_sc_data_rm_*, d_sc_data_dm_bus_pl_crt / dm_pl_sel / dm_save).
extern dSceneBaseData_c g_roomBase[]; // 804F4CD8
extern const u16 g_roomBaseNum;       // 807513B0: 7
extern u16 g_roomRes[];               // 804F4D10
extern const u16 g_roomResNum;        // 807513B2: 8

// The parameter of actor 0x0E in the happy room / theater entrances (d_sc_data_rm_happy_ent, d_sc_data_rm_theater_ent),
// built by their __sinit. Field meanings not recovered.
struct EntParam_c {
    EntParam_c(u32 scene, u32 a, u32 b, u32 c, u32 d) {
        mScene = scene;
        _04 = a;
        _08 = b;
        _0C = c;
        _10 = d;
        _14 = 64.0f;
        _18 = 8.0f;
    }

    /* 0x00 */ u32 mScene;
    /* 0x04 */ u32 _04;
    /* 0x08 */ u32 _08;
    /* 0x0C */ u32 _0C;
    /* 0x10 */ u32 _10;
    /* 0x14 */ f32 _14;
    /* 0x18 */ f32 _18;
}; // size 0x1C

// The town scene (d_sc_data_town).
extern dSceneBaseData_c g_townBase[]; // 804F4D20
extern const u16 g_townBaseNum;       // 807513B4: 8
extern u16 g_townRes[];               // 804F4D60
extern const u16 g_townResNum;        // 807513B6: 13

} // namespace dScData
