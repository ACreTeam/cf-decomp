#pragma once
#include <types.h>
#include <game/game/d_bgc.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/mLib/m_angle.hpp>
#include <nw4r/ut/ut_algorithm.h>

// The game's bg check (namespace dBGCF). Source: src/dol/game/d_bgcf.cpp (.text 8006C620..800767A8).
// See notes/d_bgcf.txt. The namespace and these class names come from the RTTI: dBGCF::floor_c,
// wall_c, column_c, clmcb_c, dtcb_c, mvbg_c, addDat_c, copyChk_c, flatChk_c (and the file-local
// callbacks dtcbCorrect_c / dtcbLineChk_c). Every other name (classes without RTTI, members,
// functions) is inferred.
//
// World layout: the field is a grid of units (32 x 32 world units). 16 x 16 units make a block (an
// acre); a bg slot holds up to 7 x 7 blocks, each with a pointer to its 16 x 16 unit collision entries
// (unitDat_c) and a base height. dBGCF keeps 16 slots (setCurrentBg selects one). A check (check())
// builds the collision geometry around the mover from the unit data: a 7 x 7 grid of cells (4
// triangles each), floor_c triangles, wall_c walls (height steps between units), column_c cylinders
// from the slot's callback (clmcb_c::getAttr), and walls/floors from the moving bg objects (mvbg_c).
// A dtcb_c callback then runs over the walls, floors and columns.
//
// The class order matters: vtables come out in reverse class-completion order (mvbg_c, addDat_c,
// floor_c, wall_c, column_c, clmcb_c, dtcb_c), and the header inlines (-sym on) come after __sinit
// in the order they are first needed.

namespace dBGCF {

// What a check (check(), acch_c::check, checkLine) builds and does.
enum checkFlags_e {
    CHECK_NONE = 0,
    CHECK_FLOOR = 0x1,           // floors: the ground, column tops, moving bg floors
    CHECK_WALL = 0x2,            // walls: steps, columns, moving bg walls
    CHECK_STEP_EXT_48 = 0x4,     // steps get a wall 48 higher (WALL_TYPE_STEP_EXT), walls around
                                 // columns, and acch_c::check does not go from water onto land
    CHECK_STEP_EXT_96 = 0x8,     // steps get a wall 96 higher
    CHECK_STEP_EXT = 0xC,        // either: walls block at any height, ATTRW / unit animation columns
    CHECK_WALLS = 0xE,
    CHECK_SET_POS = 0x10,        // write the corrected position (the line's end) back
    CHECK_NO_COLUMN_WALLS = 0x20,
    CHECK_NO_COLUMNS = 0x40,
    CHECK_NO_MVBG = 0x80,        // no moving bg objects
    CHECK_NO_WAVE_WATER = 0x100, // acch_c::check: BG_ATTR_WAVE is not water (ground check mode 1)
    CHECK_EDGE_WALLS = 0x200,    // wall edge hits count in acch_c::mHitFlags
    CHECK_NO_ANM = 0x400,        // no ATTRW / unit animation columns
    CHECK_ALL = 0xFFFFFFFF
};

// Which triangle attributes a check uses (attribute table mTriAttr0 / mTriAttr1, the cell type).
enum layer_e {
    LAYER_TOP = 0,  // bridges are ground
    LAYER_WATER = 1 // what is under bridges; river triangles are flat at the water level
};

// The 4 triangles of a unit, by the side of the unit they touch (getUnitQuarter, calcTri0..3).
enum unitQuarter_e {
    QUARTER_NEG_Z = 0,
    QUARTER_NEG_X = 1,
    QUARTER_POS_Z = 2,
    QUARTER_POS_X = 3
};

// unitDat_c::mFlags
enum unitFlags_e {
    UNIT_CONTINUOUS = 0x1, // the 4 triangles meet at one height (no step inside the unit)
    UNIT_FLAT = 0x2,
    UNIT_HEIGHT = 0x7C, // base height (steps of 7)
    UNIT_ROUTE = 0x80      // part of the NPC walking routes
};

// wallDat_s::mType / wall_c::mType / acchWall_c::mType
enum wallType_e {
    WALL_TYPE_NONE = 0,     // an empty acchWall_c entry
    WALL_TYPE_NORMAL = 1,   // a step between units, a moving bg wall
    WALL_TYPE_STEP_EXT = 2, // the part of a step wall above the step (CHECK_STEP_EXT)
    WALL_TYPE_COLUMN = 3    // walls between and around columns: they push, but are no hit (isValid)
};

// column_c::mType. acchWall_c::add gets it as the wall type of a column's side.
enum columnType_e {
    COLUMN_TYPE_STAND = 1, // can be stood on
    COLUMN_TYPE_ANM = 2    // an ATTRW column or a unit animation (CHECK_STEP_EXT only)
};

// mvbg_c::mPointNum
enum mvbgPoints_e {
    MVBG_POINTS_LINE = 2, // sizeX or sizeZ is 0
    MVBG_POINTS_BOX = 4
};

// acch_c::mHitFlags. The wall directions are relative to the angle passed to the check.
enum hitFlags_e {
    HIT_GROUND = 0x1,
    HIT_UNDER_WATER = 0x2, // below the water surface
    HIT_WATER = 0x4,       // over water, above its surface
    HIT_WALL_FRONT = 0x8,
    HIT_WALL_RIGHT = 0x10,
    HIT_WALL_LEFT = 0x20,
    HIT_WALL_BACK = 0x40,
    HIT_WALL = 0x78,
    HIT_STEP_EXT_FRONT = 0x80, // WALL_TYPE_STEP_EXT walls
    HIT_STEP_EXT_RIGHT = 0x100,
    HIT_STEP_EXT_LEFT = 0x200,
    HIT_STEP_EXT_BACK = 0x400,
    HIT_STEP_EXT = 0x780,
    HIT_CORNER = 0x800,      // two walls, facing between them
    HIT_FLAG12 = 0x1000,     // two walls whose angles add up to 0
    HIT_WATER_SLOPE = 0x2000 // water on a wave slope (groundChk_c::mSlope)
};

class mvbg_c;
class wall_c;
class acch_c;

// Packed corner heights for one triangle (5 bits per corner, in steps of 7).
struct triDat_c {
    u32 getH0() const { return mValue & 0x1F; }
    u32 getH1() const { return (mValue >> 5) & 0x1F; }
    u32 getH2() const { return (mValue >> 10) & 0x1F; }
    f32 getY0() const { return 7.0f * getH0(); }
    f32 getY1() const { return 7.0f * getH1(); }
    f32 getY2() const { return 7.0f * getH2(); }
    f32 getWaterY() const { return 0.5f * (getY1() + getY2()); }
    void clear() { mValue = 0; }

    u16 mValue;
}; // size 0x2

// One unit's collision data (10 bytes; 16 x 16 per block).
struct unitDat_c {
    const triDat_c &getTri(int i) const { return mTri[i]; }
    f32 getY0(int i) const { return mTri[i].getY0(); }
    f32 getY1(int i) const { return mTri[i].getY1(); }
    f32 getY2(int i) const { return mTri[i].getY2(); }
    f32 getBaseY() const {
        f32 h = getHeight();
        return 7.0f * h;
    }
    int getHeight() const { return (mFlags >> 2) & 0x1F; }
    int isRoute() const { return (mFlags >> 7) & 1; }
    int isFlat() const { return (mFlags >> 1) & 1; }
    int isContinuous() const { return mFlags & 1; }

    // The 3 corners of triangle 0..3 of a unit whose corner is at base (this is not used).
    void calcTri0(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const; // 8006C864
    void calcTri1(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const; // 8006C95C
    void calcTri2(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const; // 8006CA54
    void calcTri3(mVec3_c *out, unitDat_c *dat, const mVec3_c &base) const; // 8006CB4C

    /* 0x0 */ triDat_c mTri[4];
    /* 0x8 */ u8 mFlags; // unitFlags_e
    /* 0x9 */ u8 mAttr;  // BG_ATTR_*
}; // size 0xA

// A moving bg object (an actor's collision box, made into walls and floors by each check). RTTI
// dBGCF::mvbg_c, vtable 804A6C88. Linked into the mvbgList_c (add / remove).
class mvbg_c {
public:
    virtual ~mvbg_c() {}                                 // 80076700
    virtual void onHit(wall_c *wall, int arg, f32 dist); // 80072068

    void init(); // 80071EC8
    mAng getAngle() const { return mAngle; }
    BOOL isAngle(const mAng &ang) const {
        mAng cur = getAngle();
        return cur.mAngle == ang.mAngle;
    }
    // Corner i of the box, wrapping around (mPointNum is MVBG_POINTS_LINE or MVBG_POINTS_BOX).
    const mVec3_c &getPoint(int i) const { return mPoint[i & (mPointNum - 1)]; }
    const dBGC::vecXZ_c &getNormal(int i) const { return mNormal[i & (mPointNum - 1)]; }
    BOOL set(f32 sizeX, f32 sizeZ, f32 height, const mVec3_c &pos, mAng ang, const mVec3_c &scale); // 80071FA0

    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ f32 mSizeX;
    /* 0x14 */ f32 mSizeZ;
    /* 0x18 */ f32 mHeight;
    /* 0x1C */ mVec3_c mScale;
    /* 0x28 */ mAng mAngle;
    /* 0x2A */ s16 mPointNum; // mvbgPoints_e
    /* 0x2C */ mvbg_c *mNext;
    /* 0x30 */ mVec3_c mPoint[4];
    /* 0x60 */ dBGC::vecXZ_c mNormal[4];
    /* 0x80 */ mVec3_c mCenter;
    /* 0x8C */ mVec3_c mHalf;
    /* 0x98 */ u8 mActive;
}; // size 0x9C

// Pieces of geometry remember where they came from (attribute and moving bg). RTTI dBGCF::addDat_c,
// a non-polymorphic second base of floor_c, wall_c and column_c.
class addDat_c {
public:
    addDat_c(); // 8006E18C

    void set(int attr, mvbg_c *mvbg); // 8006E19C
    void set(const addDat_c &other); // 8006E1A8
    int getAttr() const { return mAttr; }

    /* 0x0 */ u8 mAttr;
    /* 0x4 */ mvbg_c *mMvbg;
}; // size 0x8

// A floor triangle. RTTI dBGCF::floor_c, vtable 804A6C34.
class floor_c : public dBGC::poly_c, public addDat_c {
public:
    floor_c() {}          // 8007656C
    virtual ~floor_c() {} // 800765B0

    BOOL set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, int attr, mvbg_c *mvbg); // 8006EFA4

    /* 0x38 */ // addDat_c
}; // size 0x40

// What a wall is made from.
struct wallDat_s {
    /* 0x00 */ dBGC::vecXZ_c mStart;
    /* 0x08 */ f32 mY0;
    /* 0x0C */ dBGC::vecXZ_c mEnd;
    /* 0x14 */ f32 mY1;
    /* 0x18 */ int mType; // wallType_e
    /* 0x1C */ int mAttr;
    /* 0x20 */ u8 mNoFace;
}; // size 0x24

// A vertical wall: a lineXZ_c with a bottom and top height. RTTI dBGCF::wall_c, vtable 804A6C04.
class wall_c : public dBGC::lineXZ_c, public addDat_c {
public:
    wall_c() : mNoFace(0) {}
    wall_c(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg); // 8006F424

    virtual BOOL isValid() const { return mType != WALL_TYPE_COLUMN; } // 800766E8

    dBGC::vecXZ_c getCenter() const;                                           // 8006F4CC
    BOOL set(const wall_c &other);                                             // 8006F50C
    BOOL set(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg); // 8006F5A0
    f32 calcCorrectR(const mVec3_c &pos, BOOL noCheck, f32 r) const;           // 8006F638
    int getType() const { return mType; }
    f32 getY0() const { return mY0; }
    f32 getY1() const { return mY1; }

    /* 0x20 */ // addDat_c
    /* 0x28 */ int mType; // wallType_e
    /* 0x2C */ f32 mY0;
    /* 0x30 */ f32 mY1;
    /* 0x34 */ u8 mY1Low; // mY1 < mY0
    /* 0x35 */ u8 mNoFace;
}; // size 0x38

// A vertical cylinder with its attribute. RTTI dBGCF::column_c, vtable 804A6BC8.
class column_c : public dBGC::column_c, public addDat_c {
public:
    column_c() {}          // 800765F0
    virtual ~column_c() {} // 80076648

    void set(const mVec3_c &pos, int type, int attr, f32 r, f32 h); // 80070AEC
    BOOL makeWall(class wallList_c *walls, const column_c &other) const; // 80070BB4

    /* 0x18 */ // addDat_c
    /* 0x20 */ int mType; // columnType_e
    /* 0x24 */ s16 mUnitX;
    /* 0x26 */ s16 mUnitZ;
}; // size 0x28

// Collision callback of a bg slot (RTTI dBGCF::clmcb_c, vtable 804A6BA8): getAttr gives the column
// (radius, height, attribute) standing on a unit, if any. dFdBase_c (d_field_info) derives from it.
class clmcb_c {
public:
    virtual ~clmcb_c() {}                                                                     // 80076690
    virtual BOOL getAttr(f32 *radius, f32 *height, int *attr, int x, int z) { return FALSE; } // 80076688
};

// Callback of copyUnits (RTTI dBGCF::copyChk_c; its vtable is in the users' TUs).
class copyChk_c {
public:
    virtual void check(int attr, u8 *copyTri, u8 *copyAttr, u8 *copyRoute) { // 800766D0
        *copyTri = TRUE;
        *copyAttr = FALSE;
        *copyRoute = FALSE;
    }
};

// Callback of flattenUnits (RTTI dBGCF::flatChk_c; its vtable is in the users' TUs).
class flatChk_c {
public:
    virtual BOOL check(int attr) = 0;
};

class grid_c;

// The walls of a check.
class wallList_c {
public:
    u32 getNum() const { return mNum; }

    BOOL correct(mVec3_c *pos, const mVec3_c &old, f32 r, acch_c *acch, u32 flags, int arg,
                 BOOL noCheck);                                                    // 8006F7D0
    BOOL add(const wall_c &wall);                                                  // 8006FC0C
    BOOL add(const wallDat_s &dat, const dBGC::vecXZ_c &normal, mvbg_c *mvbg);     // 8006FC58
    void makeStepWalls(grid_c *grid, const int *xz, f32 h);                        // 8006FC94
    void makeSteps(grid_c *grid, const int *min, const int *max, u32 flags);       // 80070A0C

    /* 0x0000 */ wall_c mWalls[0x60];
    /* 0x1500 */ u32 mNum;
}; // size 0x1504

// The floor triangles of a check.
class floorList_c {
public:
    u32 getNum() const { return mNum; }

    void make(grid_c *grid, const int *min, const int *max); // 8006F018
    BOOL add(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, int attr, mvbg_c *mvbg); // 8006F3F8

    /* 0x0000 */ floor_c mFloors[0x5A];
    /* 0x1680 */ u32 mNum;
}; // size 0x1684

// The columns of a check.
class columnList_c {
public:
    u32 getNum() const { return mNum; }

    BOOL correctSide(mVec3_c *pos, f32 r, acch_c *acch, int arg, clmcb_c *cb);         // 80070D94
    BOOL correctTop(mVec3_c *pos, const mVec3_c &old, int *attr, int arg, clmcb_c *cb); // 80070E80
    BOOL add(f32 r, f32 h, const mVec3_c &pos, int type, int attr);                     // 80070F30
    void make(clmcb_c *cb, const int *min, const int *max, BOOL anm);                   // 80070F90
    void makeWalls(wallList_c *walls, u32 flags, clmcb_c *cb);                          // 80071160

    /* 0x000 */ u32 mNum;
    /* 0x004 */ column_c mColumns[0x18];
}; // size 0x3C4

// The moving bg objects.
class mvbgList_c {
public:
    mvbgList_c() { mHead = NULL; }
    ~mvbgList_c() { mHead = NULL; }

    void clear();                                                                   // 80072080
    BOOL calc(mvbg_c *mvbg, const mVec3_c &pos, mAng ang, const mVec3_c &scale);    // 8007208C
    BOOL add(f32 sizeX, f32 sizeZ, f32 height, mvbg_c *mvbg, const mVec3_c &pos, mAng ang,
             const mVec3_c *scale);                                                 // 80072478
    BOOL remove(mvbg_c *mvbg);                                                      // 80072518
    BOOL isOutside(const mVec3_c &center, const mVec3_c &half, const mVec3_c *a,
                   const mVec3_c *b);                                               // 80072588
    void makeGeometry(const mVec3_c &center, const mVec3_c &half, wallList_c *walls, floorList_c *floors,
                      u32 flags, BOOL below);                                       // 800726E0

    /* 0x0 */ mvbg_c *mHead;
}; // size 0x4

// The result of a ground check at a position. Used as a local by many actors.
class groundChk_c {
public:
    // type: layer_e. mode: as in isWaterAttr.
    groundChk_c(const mVec3_c *pos, int type, int mode, u8 arg);         // 8006E1BC
    groundChk_c(int unitX, int unitZ, int type, int mode, u8 arg);       // 8006E238

    BOOL isContinuous();                         // 8006E2C8
    f32 getHeight(BOOL withColumn);         // 8006E31C
    BOOL isUnderWater(f32 y);               // 8006E400
    void setSlope(f32 prevY, f32 y, mAng ang); // 8006E42C
    void check(const mVec3_c *pos, int type, int mode); // 8006E4AC

    /* 0x00 */ u8 mSlope;
    /* 0x04 */ mVec3_c mSlopeCenter;
    /* 0x10 */ mVec3_c mPos;
    /* 0x1C */ int mUnitX;
    /* 0x20 */ int mUnitZ;
    /* 0x24 */ mVec3_c mDir;   // (0, 0, 1) turned to the slope or flow direction
    /* 0x30 */ int mWater;     // the attribute's water kind (bgWater_e)
    /* 0x34 */ int mAttr;      // the ground triangle's attribute
    /* 0x38 */ int mUnitAttr;
    /* 0x3C */ f32 mWaterY;
    /* 0x40 */ floor_c mFloor; // the ground triangle
    /* 0x80 */ u8 mArg;
}; // size 0x84

// The wall hits of one acch_c check. No constructor of its own: clear() (800714C8) is called in place by
// acch_c's constructor and by acch_c::clear().
class acchWall_c {
public:
    void clear(); // 800714C8

    BOOL add(mAng ang, int type, int attr, BOOL ignore); // 800714F0

    int getNum() const { return mNum; }
    mAng getAngle(int i) const {
        const int stride = sizeof(mAngle[0]);
        return *static_cast<const mAng *>(nw4r::ut::AddOffsetToPtr(mAngle, i * stride));
    }
    int getType(int i) const {
        const int stride = sizeof(mType[0]);
        return *static_cast<const int *>(nw4r::ut::AddOffsetToPtr(mType, i * stride));
    }

    /* 0x00 */ mAng mAngle[2];
    /* 0x04 */ u8 mNum;
    /* 0x08 */ int mAttr[2];
    /* 0x10 */ int mType[2]; // wallType_e (columnType_e for a column's side)
    /* 0x18 */ u8 mFlags;    // bit n: hit n is not counted in acch_c::mHitFlags (an edge hit)
}; // size 0x1C

// An actor's bg collision state ("actor collision check"). check(radius, &pos, &prevPos, angle, ...)
// corrects the position against the bg: a dtcbCorrect_c pass and the floor check, then fills the
// results below.
class acch_c {
public:
    acch_c() {
        mWall.clear();
        init();
    }

    u32 isWallHit() const { return mHitFlags & HIT_WALL; }         // a wall in any direction
    u32 isStepWallHit() const { return mHitFlags & HIT_STEP_EXT; } // a step's extended wall

    void clear();                 // 800715A8
    void init();                  // 8007162C
    void calcWallFlags(mAng ang); // 80071690
    void checkSteps(f32 r, mVec3_c *pos, const mVec3_c *old, mAng ang, int arg, u32 flags); // 8007405C
    void check(f32 r, mVec3_c *pos, const mVec3_c *old, mAng ang, int arg, u32 flags,
               BOOL doClear); // 80074234
    f32 getStepHeight() const { return mStepHeight; }

    /* 0x00 */ u8 _00;            // cleared at the end of each check
    /* 0x04 */ u32 mPrevHitFlags; // mHitFlags of the previous check (HIT_UNDER_WATER: LAYER_WATER)
    /* 0x08 */ u32 mHitFlags;     // hitFlags_e
    /* 0x0C */ int mGroundAttr;
    /* 0x10 */ acchWall_c mWall;
    /* 0x2C */ mVec3_c mPushOut;     // how far the check moved the position
    /* 0x38 */ mVec3_c mGroundNormal;
    /* 0x44 */ f32 mStepHeight;      // still counts as ground this far above the floor
}; // size 0x48

// A check callback, run over the walls, floors and columns by check(). RTTI dBGCF::dtcb_c.
class dtcb_c {
public:
    virtual ~dtcb_c() {}
    virtual void checkWall(wallList_c *walls) = 0;
    virtual void checkFloor(floorList_c *floors) = 0;
    virtual void checkColumn(columnList_c *columns) = 0;
};

// The result of checkLine.
struct lineChk_s {
    /* 0x0 */ addDat_c mDat;
    /* 0x8 */ mVec3_c mNormal;
}; // size 0x14

extern const f32 cWaterY0; // 80750418: 14
extern const f32 cWaterY1; // 8075041C: 14
extern const f32 cWaterY2; // 80750420: 70

// Positions.
unitDat_c *getUnitDat(unitDat_c *data, int idx);                  // 8006CC44
unitDat_c *getUnitDat(unitDat_c *data, int x, int z);             // 8006CC54
void posToUnit(int *unitX, int *unitZ, const mVec3_c *pos);       // 8006CC64
void posToBlock(int *blockX, int *blockZ, const mVec3_c *pos);    // 8006CCD8
void unitToBlock(int *blockX, int *blockZ, int unitX, int unitZ); // 8006CD4C
void unitToPos(mVec3_c *pos, int unitX, int unitZ);               // 8006CD90
void snapToUnit(mVec3_c *out, const mVec3_c *pos);                // 8006CDF4

int getTriAttr1(int attr, int idx); // 8006D414
int getTriAttr0(int attr, int idx); // 8006D4CC

// Moving bg objects.
mvbgList_c *getMvbgList(); // 8007206C

// Unit queries (current slot).
BOOL isLoaded();                                         // 80072A00
unitDat_c *getBlockData(int blockX, int blockZ);         // 80072AA8
unitDat_c *getUnitDat(int unitX, int unitZ);             // 80072B00
unitDat_c *getUnitDat(const mVec3_c *pos);               // 80072B7C
int getUnitAttr(int unitX, int unitZ);                   // 80072BB4
clmcb_c *getClmcb();                                     // 80072BE8
BOOL isWaterAttr(int attr, int mode);                    // 80072C60
BOOL isWaterToLand(const mVec3_c *from, const mVec3_c *to, acch_c *acch, int arg); // 80072CD8
int getRouteDirs(int unitX, int unitZ);                  // 80072D54: 1 << unitQuarter_e per route neighbour
void setRoute(int unitX, int unitZ, int on);             // 80072E50
int getUnitQuarter(const mVec3_c *pos);                  // 80072E94: unitQuarter_e
int isFlat(const mVec3_c *pos);                          // 80072F48
int getAttr(int unitX, int unitZ);                       // 80072F80
int getGroundAttr(const mVec3_c *pos, int type);         // 80072F84
int getWaterKind(int unitX, int unitZ);                  // 80073054
int isRiver(int unitX, int unitZ);                       // 800730D0
int isSea(int unitX, int unitZ);                         // 80073114
int canPutItem(int unitX, int unitZ);                    // 80073158
int canPutItemNoBridge(int unitX, int unitZ);           // 800731B0
int isGrassGround(int unitX, int unitZ);                 // 80073208
int canNpcPutItem(int unitX, int unitZ);                 // 80073260
int getDigType(int unitX, int unitZ);                    // 800732B8
int getPlantType(int unitX, int unitZ);                  // 80073314
int getGrassMin(int unitX, int unitZ);                   // 80073370: grass wear limits
int getGrassMax(int unitX, int unitZ);                   // 800733B0
int isBeachGround(int unitX, int unitZ);                 // 800733F0
BOOL getMapColors(u8 *out, int unitX, int unitZ);        // 80073434: 3 x 3 town map pixels
f32 getUnitY(const mVec3_c *pos);                        // 80073510
int getFootstepSe(int attr);                             // 80073798: sound id
int getFtrSe(int attr);                                  // 800737BC: furniture move sound id
BOOL checkWalkable(int unitX, int unitZ, int dir, int toX, int toZ);                // 800737E4
BOOL checkPut(int unitX, int unitZ, int dir, int toX, int toZ, BOOL noBridge); // 80073C94
BOOL checkPut(const mVec3_c *pos, int toX, int toZ, BOOL noBridge);                  // 80073D2C
BOOL isWaterArea(const mVec3_c *pos, f32 *waterY, int mode, f32 r);                 // 80073D9C
BOOL findWaterPos(mVec3_c *out, const mVec3_c *pos, mAng ang, u32 num, int mode, f32 dist,
                  f32 r);                                                           // 80073EE4
BOOL isWaterArea(const mVec3_c *pos, f32 r);                                        // 80074050

// The bg check.
BOOL checkLineSteps(lineChk_s *out, mVec3_c *end, const mVec3_c *start, u32 flags); // 80074630
BOOL checkLine(lineChk_s *out, mVec3_c *end, const mVec3_c *start, u32 flags);      // 8007481C
f32 getGroundY(const mVec3_c *pos, BOOL withColumn);                                // 80074974
f32 getGroundY(const mVec3_c *pos, int *attr, u32 flags);                           // 80074C8C
f32 getUnitBaseY(int unitX, int unitZ);                                             // 80074D64
f32 getUnitBaseY(const mVec3_c *pos);                                               // 80074E5C
f32 getEdgeGroundY(const mVec3_c *pos);                                             // 80074E94
f32 getUnitMinY(int unitX, int unitZ);                                              // 80074F94
BOOL getColumnAttr(int x, int z, f32 *radius, f32 *height, int *attr);              // 80075114
BOOL checkColumn(const mVec3_c *pos, f32 *radius, f32 *height, int *attr);          // 800751D8
void fn_800752E0();                                                              // 800752E0
void fn_800752E4();                                                              // 800752E4
void check(const mVec3_c *min, const mVec3_c *max, dtcb_c *cb, u32 flags, int mode, int type); // 800752E8

// Slots.
void setCurrentBg(int idx);                                          // 800755A0
BOOL entryBg(u32 blockW, u32 blockH, clmcb_c *cb, int idx);          // 80075658
BOOL setBlock(int blockX, int blockZ, unitDat_c *data, u32 y, int idx); // 800756F4
void resetBg(int idx);                                               // 80075780
void clearBlocks(int idx);                                           // 80075808
void releaseBg(int idx);                                             // 8007589C
BOOL setUnitAnm(int x, int z, u32 time, f32 to, f32 from);           // 800758E0
void setBgY(f32 y);                                                  // 80075908
int getBlockUnitAttr(unitDat_c *data, int x, int z);                  // 80075930
void copyUnits(unitDat_c *src, int unitX, int unitZ, copyChk_c *cb); // 80075954
void flattenUnits(unitDat_c *src, int unitX, int unitZ, flatChk_c *cb); // 80075AB0
BOOL isSlopeStep(const mVec3_c *pos);                                // 80075B9C
int getNext(int unitX, int unitZ);                                   // 80075C98
int getOpenNext(int unitX, int unitZ);                               // 80075CD8
int getOpenNext(const mVec3_c *pos);                                 // 80075D3C
BOOL lockNext(int idx);                                              // 80075D74
BOOL isNextLocked(const mVec3_c *pos);                               // 80075DC8
int getLockedNext(int unitX, int unitZ);                             // 80075E30
int getLockedNext(const mVec3_c *pos);                               // 80075E98
BOOL unlockNext(int idx);                                            // 80075ED0
mVec3_c getNextCenter(int idx);                                      // 80075F24
BOOL getArea(f32 *minX, f32 *maxX, f32 *minZ, f32 *maxZ);            // 80076104
BOOL setSoilX(int unitX, int unitZ);                                 // 80076260
void fn_800762C8();                                               // 800762C8
void updateUnitAnm();                                                // 800762CC
void fn_800762E0();                                               // 800762E0
BOOL isNearFall(int unitX, int unitZ);                               // 800762E4
BOOL isNearFall(const mVec3_c *pos);                                 // 80076370

} // namespace dBGCF
