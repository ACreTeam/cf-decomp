#pragma once

// Town structures (namespace dSTR; src/dol/game/d_str.cpp, .text 80166990..8016AE68). The class names
// come from the RTTI.

#include <types.h>
// d_bgcf.hpp first: its flatChk_c / copyChk_c weak vtables come last in our .data (reverse class
// completion order), so its classes must complete before d_dvd.hpp's and ours.
#include <game/game/d_bgcf.hpp>
#include <game/game/d_dvd.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <game/mLib/m_vec.hpp>
#include <cstring>

namespace dScene { class dSceneChange_c; }
namespace dBGCF {
struct unitDat_c;
}
namespace dItem {
struct Item;
}
class dFdUnitAttr_c;
struct dBuildingInfo_c;
class daSTR_c; // d_a_str.hpp

namespace dSTR {

// The structures with a collision / model resource (cNameTbl): "strCol/<name>.bin" in
// "/Str/Col/strCol.arc", "strBrres/<name>.brres" in the season's StrBrres arc.
enum {
    STR_BALLOON,
    STR_BUS,
    STR_COMMON,
    STR_BALLOON_C,
    STR_BBS,
    STR_BBS_BIRD,
    STR_BUILDSITE,
    STR_BUSSTOP0,
    STR_CARNIVAL_BORD,
    STR_CDNUM,
    STR_CHECK0,
    STR_CHECK1,
    STR_CHECK2,
    STR_FOUNTAIN,
    STR_HARVEST0,
    STR_HARVEST1,
    STR_LIGHTHOUSE,
    STR_MUSEUM,
    STR_MYHOME0,
    STR_MYHOME1,
    STR_MYHOME2,
    STR_MYHOME3,
    STR_MYHOME4,
    STR_MYHOME_COMMON,
    STR_NPC_COMMON,
    STR_OFFICE,
    STR_POST,
    STR_TENT0,
    STR_TENT1,
    STR_WINDMILL,
    STR_UFO0,
    STR_UFO1,
    STR_CDBBS,
    STR_NC,
    STR_NG,
    STR_NK,
    STR_NW,
    STR_TAILOR,
    STR_TN_HOUSE0,
    STR_TN_HOUSE1,
    STR_TN_HOUSE2,
    STR_TN_HOUSE3,
    STR_TN_HOUSE4,
    STR_HOUSE_TEX,
    STR_BUSSTOP1,
    STR_TW_ATM,
    STR_TW_HRA,
    STR_TW_AUCTION,
    STR_TW_CUT,
    STR_TW_ETC00,
    STR_TW_ETC01,
    STR_TW_ETC02,
    STR_TW_ETC03,
    STR_TW_ETC04,
    STR_TW_FORTUNE,
    STR_TW_FOUNTAIN,
    STR_TW_GRACE,
    STR_TW_LAMP,
    STR_TW_ORGAN,
    STR_TW_RESET,
    STR_TW_SHOEBLACK,
    STR_TW_THEATER,
    STR_TW_TSUNEKICHI,

    STR_NUM, // 63
};

// 804F38DC: the structure resource names, by STR_*.
extern const char *cNameTbl[STR_NUM];

const char *getName(u32 idx);  // 80166AE0: cNameTbl[idx], the first one when out of range
int getIndex(const char *name); // 80166B08: the STR_* of a name, -1 if none

// A view rectangle around the camera target (dCamera_c::sCenter). Unused.
class viewArea_c {
public:
    viewArea_c(f32 front, f32 back, f32 halfWidth); // 80166B70
    void set(f32 front, f32 back, f32 halfWidth); // 80166BA0: the width scaled by 4/3
    BOOL isIn(const mVec3_c &pos) const;           // 80166BB8
    bool isOutside(f32 absX, f32 dz) const {
        return absX > mHalfWidth || dz > mFront || dz < mBack;
    }

    /* 0x0 */ f32 mFront;     // max z offset
    /* 0x4 */ f32 mBack;      // min z offset
    /* 0x8 */ f32 mHalfWidth; // max |x offset|
}; // size 0xC

// One flag per structure (805FB060 / 805FB0A0).
class strFlag_c {
public:
    strFlag_c() { clear(); }
    void clear() { memset(mFlags, 0, sizeof(mFlags)); }
    BOOL is(const char *name) const; // 80166C14
    void set(const char *name);      // 80166C64

    /* 0x00 */ u8 mFlags[STR_NUM];
}; // size 0x3F

// A structure's collision box (BG_ATTR_STR_COL units of its collision data), 805FB7DC.
class colArea_c {
public:
    colArea_c() : mValid(0) {}

    // 8016AC98 (weak; getEntranceSidePos)
    mVec3_c getCenter() const {
        mVec3_c c = mMin + mMax;
        c *= 0.5f;
        return c;
    }

    /* 0x00 */ mVec3_c mMin;
    /* 0x0C */ mVec3_c mMax;
    /* 0x18 */ u8 mValid;
}; // size 0x1C

// A structure's entrance (BG_ATTR_STR_ENT / STR_ENT_TO_RU / STR_ENT_TO_LU units), 805FB0EC.
// d_scene's dSceneBuilding_c.
class entArea_c {
public:
    entArea_c() : mValid(0), mAngle(0) {}

    int getDirType() const; // 80167FDC: 0 (angle 0), 2 (0x2000), 1 (0xE000), else 0
    // The point dist in front of the entrance of the structure standing at base / at a unit / where
    // building id stands (bg: the dBGCF slot for the ground height).
    mVec3_c calcFrontPos(const mVec3_c &base, int bg, f32 dist);  // 80168030
    mVec3_c calcFrontPos(int unitX, int unitZ, int bg, f32 dist); // 80168128
    mVec3_c calcFrontPos(u16 id, int bg, f32 dist);               // 801681B8 (d_scene)
    // The entrance box moved to base, at the ground height of its centre; returns mAngle << 16.
    int getBox(mVec3_c *min, mVec3_c *max, const mVec3_c &base, int bg); // 801682B4

    /* 0x00 */ mVec3_c mMin;
    /* 0x0C */ mVec3_c mMax;
    /* 0x18 */ u8 mValid;
    /* 0x1A */ s16 mAngle; // the facing of a STR_ENT_TO_RU (-0x2000) / LU (0x2000) entrance
}; // size 0x1C

// 805FB014: the function-local static of getHostIO (debug parameters; field names unknown).
// Kept before townTexBank_c: vtables come out in reverse class-completion order (colBank_c,
// brresBank_c, townTexBank_c, hostIO_c in the target's .data).
class hostIO_c {
public:
    hostIO_c(); // 8016699C
    virtual ~hostIO_c() {}

    /* 0x04 */ u8 _04;
    /* 0x05 */ u8 _05;
    /* 0x06 */ u8 _06;
    /* 0x08 */ s16 _08;
    /* 0x0A */ s16 _0A;
    /* 0x0C */ s16 _0C;
    /* 0x10 */ f32 _10;
    /* 0x14 */ s16 _14;
    /* 0x16 */ s16 _16;
    /* 0x18 */ u8 _18;
    /* 0x19 */ u8 _19;
    /* 0x1A */ u8 _1A;
    /* 0x1C */ s16 _1C;
    /* 0x1E */ s16 _1E;
    /* 0x20 */ s16 _20;
    /* 0x22 */ s16 _22;
    /* 0x24 */ u8 _24;
    /* 0x26 */ s16 _26;
    /* 0x28 */ s16 _28;
    /* 0x2A */ s16 _2A;
    /* 0x2C */ u8 _2C;
    /* 0x2E */ s16 _2E;
    /* 0x30 */ s16 _30;
    /* 0x32 */ s16 _32;
    /* 0x34 */ f32 _34;
    /* 0x38 */ f32 _38;
    /* 0x3C */ s16 _3C;
    /* 0x3E */ s16 _3E;
    /* 0x40 */ s16 _40;
    /* 0x44 */ f32 _44; // isAtFountainFront: how near the fountain counts
    /* 0x48 */ s16 _48;
}; // size 0x4C

// The town's season / language texture brres ("/Str/Brres/<region><lang>TownTex<season>.brres").
class townTexBank_c : public dDvd::brresBank_c {
public:
    townTexBank_c() {}
    virtual void onLoaded() {} // 8016ACF0 (weak); the implicit dtor is 8016AE08

    BOOL load(void *heap); // 80166CA4
}; // size 0x58

// The structures' brres archive ("/Str/Brres/<field|town><region><lang>StrBrres<season>.arc"), 805FBF10.
class brresBank_c : public dDvd::arcBank_c {
public:
    brresBank_c();            // 80166D4C
    virtual ~brresBank_c();   // 8016AA3C
    virtual void onLoaded();  // 80167094: inits and binds every structure brres

    static void clearFlags(); // 80166DC4
    void clear();             // 80166E08
    BOOL isFlag0(const char *name); // 80166E28
    void setFlag0(const char *name); // 80166E34
    BOOL isFlag1(const char *name); // 80166E40
    void setFlag1(const char *name); // 80166E4C
    BOOL isSame(u32 isField, s8 season, int region, int lang); // 80166E58
    BOOL load(void *heap);    // 80166EA0
    BOOL isLoaded();          // 80166FF0
    BOOL unload();            // 80167040
    void *getBrres(const char *name); // 801671EC: "strBrres/<name>.brres"

    /* 0x74 */ u8 mRegion;   // getTownRegion when loaded
    /* 0x75 */ u8 mLang;     // getTownLanguage when loaded
    /* 0x76 */ s8 mSeason;   // dScene::getSeasonType when loaded
    /* 0x77 */ bool mIsField; // loaded outside the city
    /* 0x78 */ u8 _78;
    /* 0x7C */ townTexBank_c mTownTex;
    /* 0xD4 */ nw4r::g3d::ResFile mTownTexRes; // mTownTex's data, NULL if not loaded
}; // size 0xD8

// The structures' collision data ("/Str/Col/strCol.arc"), 805FBFF4.
class colBank_c : public dDvd::arcBank_c {
public:
    colBank_c() {}
    virtual ~colBank_c();    // 8016AABC
    virtual void onLoaded(); // 80167260: computes the collision boxes and entrances

    BOOL load(void *heap);                       // 80167250
    entArea_c *getEntArea(const char *name);     // 801676E4
    entArea_c *getEntArea(u16 building);         // 80167734
    colArea_c *getColArea(const char *name);     // 801677C8
    colArea_c *getColArea(u16 building);         // 80167818
    dBGCF::unitDat_c *getCol(int building);      // 801678AC: by building index
    dBGCF::unitDat_c *getCol(const char *name);  // 801678F8: "strCol/<name>.bin"
}; // size 0x74


// A light that fades or flickers on (d_environment's lbl_80624220; the town's night lights).
class light_c {
public:
    light_c() {
        init();
    }

    void init();                                           // 80167E08
    BOOL set(BOOL on);                                     // 80167E20: fades at 0.1 a frame
    BOOL update();                                         // 80167E30: TRUE when mValue changed
    BOOL set(BOOL on, BOOL fade, BOOL flicker, f32 speed); // 80167F04: TRUE when it starts to fade

    /* 0x00 */ f32 mValue;
    /* 0x04 */ f32 mTarget;      // 0 or 1
    /* 0x08 */ f32 mSpeed;
    /* 0x0C */ int mFlicker;     // 1: playing the flicker table
    /* 0x10 */ u16 mFlickerIdx;
    /* 0x12 */ u16 mFlickerWait; // frames before the flicker starts
}; // size 0x14

// The interface of the structure actors (RTTI dSTR::alwaysAc_c; second base of daSTR_c, d_a_str.hpp).
class alwaysAc_c {
public:
    virtual BOOL isActive() = 0;      // 0x08
    virtual int getUnitX() = 0;       // 0x0C
    virtual int getUnitZ() = 0;       // 0x10
    virtual int getBuildingIdx() = 0; // 0x14: the dBuildingInfo_c index, -1 if none
};

// The structure actors that exist (805FF230; daSTR_c adds / removes itself).
class acList_c {
public:
    acList_c() { init(); }
    void init();                                              // 801692AC
    BOOL add(daSTR_c *ac);                                    // 801692E4
    BOOL remove(daSTR_c *ac);                                 // 801693E0
    daSTR_c *search(int unitX, int unitZ, BOOL any);          // 80169424 (any: also inactive ones)
    daSTR_c *searchBuilding(u16 id, BOOL any);                // 801694F4
    daSTR_c *searchNearest(int unitX, int unitZ, BOOL any);   // 801695E8: not past unitZ
    daSTR_c *searchNearest(const mVec3_c &pos, BOOL any);     // 80169720

    /* 0x00 */ daSTR_c *mList[64];
}; // size 0x100

// The bus ride's destination (requestBus; the static of getBusInfo, cleared by the soft reset).
struct busInfo_c {
    busInfo_c() : mType(0) {}

    /* 0x0 */ int mType; // 1 or 2
    /* 0x4 */ u8 _4;
};

f32 calcLerp(f32 t, f32 a, f32 b);                // 80167FA4: a + t * (b - a)
const dBuildingInfo_c *getBuildingInfo(int idx);  // 80167FB4: the table entry (a zero one when out of range)
brresBank_c *getBrresBank();                      // 801683C0: 805FBF10
colBank_c *getColBank();                          // 801683CC: 805FBFF4
dFdUnitAttr_c *getTownUnitAttr();                 // 801683D8: 805FC068
dFdUnitAttr_c *getCityUnitAttr();                 // 801683E4: 805FD8EC
const char *getBuildingModel(int idx);            // 801683F0: dBuildingInfo_c::mModel
const char *getBuildingResName(int idx);          // 80168414: the STR_* name, with the gate / house / shop looks
void copyCol(int unitX, int unitZ, const char *name);    // 801685A4: dBGCF::copyUnits of its collision
void flattenCol(int unitX, int unitZ, const char *name); // 80168628: dBGCF::flattenUnits of its collision
int getCurBuilding(int *unitX, int *unitZ);       // 801686AC: the building the player is in, -1 if none
u32 getCurBuildingCode(); // 80168824: x | z << 8 | dBuildingInfo_c::mProfile <<
                          // 16
int getCurBuildingType(); // 80168888: dBuildingInfo_c::mType (the shop by its
                          // stage)
u32 countMembers(u8 mask);                        // 801688F0: set bits of the low 4
int getSlotIdx(const dItem::Item *item);          // 80168948: the building index, 0 if none

// Which net members are inside each building (the host keeps it; NET_PACKET_STR_LOCK / UNLOCK).
void recvLock(const u8 *data, int member);        // 801689A0: NET_PACKET_STR_LOCK
void recvUnlock(const u8 *data, int member);      // 80168A4C: NET_PACKET_STR_UNLOCK
void resetLocks();                                // 80168B90: (the soft reset)
void lock(const dItem::Item *item);               // 80168CAC: this member enters
int getLockState(const dItem::Item *item);        // 80168D60
void unlock(const dItem::Item *item, int member); // 80168DC0: the member leaves every building
BOOL isPartyFlag();                               // 80168F84 (8074E868)
void setPartyFlag();                              // 80168F8C
void clearPartyFlag();                            // 80168F98
BOOL isAlone();                                   // 80168FA4: countCurVisitors() <= 1
u8 getVisitors(u8 scene);                         // 80168FDC: the members in the scene's building
u32 countVisitors(u8 scene);                      // 80169070: +1 with the party flag in 0x450 scenes
u32 countCurVisitors();                           // 801690DC

int fn_8016911C();                                // 8016911C: 0 (meaning unknown)
int getDebugHouseType();                          // 80169124: -1 (8074B4A4; d_scene's season override)
BOOL getNpcHouseType(u32 *out, u32 no);           // 8016912C: villager no's TN_obj_house<out[0]> and look
u32 getNpcHouseVariantNum();                      // 80169290: 5
u32 getNpcHouseTypeNum();                         // 80169298: 6
acList_c *getAcList();                            // 801692A0: 805FF230
int getTownRegion();                              // 8016976C: getLanguageArea(getTownLanguage())
int getTownLanguage();                            // 80169790: dSaveData_c::getTownLanguage
int getCurRegion();                               // 801697BC: getTownRegion, 0 in a 9 scene (the city)
int getCurLanguage();                             // 801697F4: getTownLanguage, 0 in a 9 scene
void requestBus(busInfo_c *info, BOOL toTown, BOOL arg2, u8 arg3, BOOL arg4); // 8016982C

dScene::dSceneChange_c *getSceneChange(); // 80166990: &dScene::gSceneChange (this TU's copy)
hostIO_c *getHostIO();             // 80166A7C: the function-local hostIO_c
busInfo_c *getBusInfo();           // 801699CC: the function-local busInfo_c

// .bss / .sbss follow definition order (function-local statics where their function is compiled,
// each dtor'd object right after its 0xC destructor-chain node). The globals __sinit (8016AB1C)
// builds, in this order: l_flag0, l_flag1, l_entArea[63], l_colArea[63], (805FBEC0, a zero
// dBuildingInfo_c for getBuildingInfo, no ctor), brresBank_c 805FBF10, colBank_c 805FBFF4,
// dFdUnitAttr_c(1) 805FC068, dFdUnitAttr_c(2) 805FD8EC, (805FF170 0x34, 805FF1A4 0x8C: no ctor),
// 805FF230 (0x100, ctor 801692AC), the statics of getCityReturnPos / playRoomSe, then gArea.

// A box set by the structure actors (d_a_str_common: clear / set) and tested by the weather
// (confetti / sakura / snow: isInside). 805FF370.
class area_c {
public:
    area_c() { clear(); }
    void clear();                                     // 8016A704
    BOOL isSet() const;                               // 8016A724: not all zero
    void set(const mVec3_c &min, const mVec3_c &max); // 8016A790
    BOOL isInside(const mVec3_c &pos) const;          // 8016A7C4

    /* 0x00 */ mVec3_c mMin;
    /* 0x0C */ mVec3_c mMax;
}; // size 0x18 (its implicit dtor is 8016ADC8)

extern area_c gArea; // 805FF370

// Event structures: put up by the town hall (16 units per acre) / removed if they stand.
void putCountdownBoard();    // 80169AC4: the New Year's countdown board
void removeCountdownBoard(); // 80169ADC
void putCarnival();          // 80169AEC: the carnival board
void removeCarnival();       // 80169B04
void putFishTent();          // 80169B14: the fishing tourney tent
void removeFishTent();       // 80169B2C
void putInsectTent();        // 80169B3C: the Bug-Off tent
void removeInsectTent();     // 80169B54
void putHarvest();           // 80169B64: the Harvest Festival structures
void removeHarvest();        // 80169BB4
void putUfo();               // 80169BF4: moves the UFO to its spot
void removeUfo();            // 80169C38

void buildPublicWorks();  // 80169C48: builds the public works whose SAVE_FLAG_BUILD_* is set
void updateFlag14();      // 80169ED8: town flag 14 with a 1 in 10 chance (meaning unknown)
BOOL requestWindmill();   // 80169F20: sets SAVE_FLAG_BUILD_WINDMILL
BOOL requestLighthouse(); // 80169F4C: sets SAVE_FLAG_BUILD_LIGHTHOUSE
BOOL requestFountain();   // 80169F78: sets SAVE_FLAG_BUILD_FOUNTAIN
// 80169FA4: the spot beside a building's entrance (side 0: left, else right), FALSE if none.
BOOL getEntranceSidePos(mVec3_c *out, const dItem::Item &item, int side);

// A counter and two flags (meaning unknown; d_sv_proc calls resetCount).
void resetCount();      // 8016A208: clears the counter and both flags
void clearCount();      // 8016A230 (8074E878)
u32 getCount();         // 8016A23C
void incCount();        // 8016A244: counts up to 0x100
void clearFlagA();      // 8016A25C (8074E87C)
void setFlagA(u8 flag); // 8016A264
u8 getFlagA();          // 8016A26C
void clearFlagB();      // 8016A274 (8074E87D)
void setFlagB(u8 flag); // 8016A27C
u8 getFlagB();          // 8016A284

mVec3_c *getCityReturnPos(); // 8016A28C: where the player arrives back in the town from the city
// 8016A2F8: whether pos / angle are at the fountain's front (in the town); its spots to front / base.
BOOL isAtFountainFront(const mVec3_c &pos, const s16 &angle, mVec3_c *front, mVec3_c *base, BOOL draw);
void playRoomSe();                            // 8016A548: the room's ambient sound (houses, shop, office, tailor)
BOOL getBusStopBox(mVec3_c *min, mVec3_c *max); // 8016A614: the box around the bus stop
BOOL fn_8016A87C();                           // 8016A87C: FALSE (meaning unknown)
int getCurBuildingKind(u8 *other);            // 8016A884: the current scene's building type (0: none)

} // namespace dSTR

// The flattenUnits / copyUnits callbacks (RTTI "copyChk_c" / "flatChk_c": no namespace). The
// structures' collision (STR_COL) and entrance-to (STR_ENT_TO_RU / LU) units. Declared after the
// dSTR classes: their vtables come first in .data (flatChk_c, then copyChk_c).
class copyChk_c : public dBGCF::copyChk_c {
public:
    virtual void check(int attr, u8 *copyTri, u8 *copyAttr, u8 *copyRoute); // 8016AA0C
};

class flatChk_c : public dBGCF::flatChk_c {
public:
    virtual BOOL check(int attr); // 8016A9D8
};

