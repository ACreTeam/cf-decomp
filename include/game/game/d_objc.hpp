#pragma once
#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_bgc.hpp>
#include <game/game/d_bgc_sphere.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

class dActor_c;

// Object collision (namespace dObjc): actors pushing each other apart, and what the Wii remote
// pointer (DPD) points at. Source: src/dol/game/d_objc.cpp (.text 801AD6E0..801B0230), run every
// frame by the dObjc_c process. See notes/d_objc.txt.
// Class names from the RTTI: dObjc::base_c, ac_c, pos_c, acpos_c, ins_c, dpdTri_c, dObjc_c. The
// other class, function and member names are inferred from behaviour.
//
// Every frame each user re-registers its objects (regist()); dObjc_c::execute then resolves them
// and empties the lists:
// - base_c (and ac_c / pos_c / acpos_c): vertical cylinders that push each other apart (mPush)
//   and record what they touched (mHitType, mHitID).
// - dpdTri_c (dpdQuad_c, dpdBox_c): triangles the pointer ray can hit (furniture, signs, ...).
// - ins_c: spheres (insects) the pointer ray can hit; also tested by ins_c::check.
// The pointer ray runs from the camera through the pointer; the nearest thing it hits is
// reported by getDpdHit(), where it meets the ground by getDpdPos().
//
// Class order matters: vtables come out in reverse class-completion order (d_objc's .data:
// dObjc_hostIO_c, dObjc_c, dpdTri_c, ins_c, ac_c, base_c), so base_c, ac_c, ins_c and dpdTri_c are
// declared in that order. All header inlines are weak copies after __sinit (-sym on).

namespace dObjc {

// A reference to a base by its unique id.
class baseID_c {
public:
    void set(fBase_c *base); // 801AD850
    fBase_c *get() const;    // 801AD870: NULL for no id

    /* 0x0 */ fBaseID_e mID;
}; // size 0x4

// Where the pointer ray meets the ground (the drum or the flat field).
class dpdPos_c {
public:
    dpdPos_c() { clear(); }

    void clear();                              // 801AD768
    void set(const mVec3_c &pos, BOOL water);  // 801AD784

    /* 0x0 */ mVec3_c mPos;
    /* 0xC */ u8 mWater; // the ground there is water
}; // size 0x10

// What the pointer ray hits first.
class dpdHit_c {
public:
    dpdHit_c() { clear(); }

    void clear();                                                      // 801AD7A4
    void set(const mVec3_c &pos, u32 type, fBase_c *base, u32 param);  // 801AD7F0

    /* 0x00 */ baseID_c mBase;
    /* 0x04 */ u32 mType;   // a HIT_* kind, or the base_c's mType
    /* 0x08 */ mVec3_c mPos;
    /* 0x14 */ u32 mParam;
}; // size 0x18

enum {
    HIT_FG = 0x80010000,     // a field object (tree, ...) on a column unit
    HIT_FTR = 0x80020000,    // furniture placed outdoors (indoor scenes)
    HIT_INSECT = 0x80080000, // an ins_c
    HIT_STR = 0x80100000,    // a town structure
};

// NULL while nothing was hit (a zero position).
const dpdPos_c *getDpdPos(); // 801AD6E0
const dpdHit_c *getDpdHit(); // 801AD724

// A vertical cylinder of radius mRadius and height mHeight standing on getPos(). RTTI dObjc::base_c
// (weak copy kept in d_a_npc). Two registered objects collide if each one's mType matches the
// other's mMask; a collision pushes them apart (mPush) by their weights and records the other's
// mType and id.
class base_c {
public:
    // Returns the weight to use against other (see mWeight) and may scale the push (rate).
    typedef u8 (*weightFunc)(base_c *self, base_c *other, f32 *rate);
    typedef void (*hitFunc)(base_c *other);

    enum {
        TYPE_DPD = 0x80000000,  // the pointer can hit it
        TYPE_ONCE = 0x40000000, // reported to only one object per frame
    };

    base_c(); // 801AE24C

    // getPos is not const: ac_c::getPos loads mActor after the prologue stores (non-const this).
    virtual const mVec3_c *getPos() = 0;
    virtual fBaseID_e getID() const = 0;
    virtual ~base_c() {} // 8002EB88 (d_a_npc)

    static void create();    // 801AE2A0
    static void clearList(); // 801AE2A4
    static void destroy();   // 801AE2CC

    static base_c *s_head; // 8074EAE8

    // Turns angle around and tells whether the push comes from within 45 degrees of it.
    BOOL checkPushAngle(mAng *angle) const; // 801AE4D4
    void set(f32 radius, f32 height, u32 type, u32 mask, u8 weight, u32 param); // 801AE580
    void init();                            // 801AE59C
    void regist();                          // 801AE5C4
    BOOL check(base_c *other);              // 801AE618
    const mVec3_c *getPush() const;         // 801AF830: Zero without a hit
    BOOL isHit(u32 type) const;             // 801AF850

    // calc reads the cylinder through these (its loads come before the getPos() call).
    f32 getRadius() const { return mRadius; }
    f32 getHeight() const { return mHeight; }

    /* 0x04 */ u32 mParam;
    /* 0x08 */ f32 mRadius;
    /* 0x0C */ f32 mHeight;
    /* 0x10 */ mVec3_c mPush;    // the push out of the others (xz)
    /* 0x1C */ u32 mType;
    /* 0x20 */ u32 mMask;
    /* 0x24 */ u32 mHitType;     // the mType of everything touched
    /* 0x28 */ fBaseID_e mHitID; // the last one touched
    /* 0x2C */ f32 mDepth;       // the deepest overlap this frame (weight 0)
    /* 0x30 */ base_c *mNext;
    /* 0x34 */ u8 mWeight;       // 0: not pushed, 0xFF: does not move, else relative weight + 1
    /* 0x35 */ u8 _35;
    /* 0x36 */ u8 mRegistered;
    /* 0x37 */ u8 mOnceHit;      // TYPE_ONCE: already reported this frame
    /* 0x38 */ weightFunc mWeightFunc;
    /* 0x3C */ hitFunc mHitFunc;
}; // size 0x40

// A base_c following an actor's position. RTTI dObjc::ac_c (weak copy kept in d_a_npc; vtable
// 804FB974 is d_objc's).
class ac_c : public base_c {
public:
    ac_c() {}

    virtual const mVec3_c *getPos(); // 801AF868
    virtual fBaseID_e getID() const; // 801AF8D0
    virtual ~ac_c() {}               // 8002EBC8 (d_a_npc)

    void set(dActor_c *actor, f32 radius, f32 height, u32 type, u32 mask, u8 weight,
             u32 param); // 801AF8DC

    /* 0x40 */ dActor_c *mActor;
}; // size 0x44

// A base_c at a fixed position. RTTI dObjc::pos_c (vtable and inlines in its users, e.g. d_uki).
class pos_c : public base_c {
public:
    pos_c() {}

    virtual const mVec3_c *getPos() { return &mPos; }
    virtual fBaseID_e getID() const { return (fBaseID_e)0; }
    virtual ~pos_c() {}

    void set(const mVec3_c &pos, f32 radius, f32 height, u32 type, u32 mask, u8 weight,
             u32 param); // 801AF8F8

    /* 0x40 */ mVec3_c mPos;
}; // size 0x4C

// An actor's base_c at a position of its own. RTTI dObjc::acpos_c (vtable and inlines in its
// users, e.g. d_a_balloon).
class acpos_c : public ac_c {
public:
    acpos_c() {}

    virtual const mVec3_c *getPos() { return &mPos; }
    virtual fBaseID_e getID() const { return ac_c::getID(); }
    virtual ~acpos_c() {}

    void set(dActor_c *actor, const mVec3_c &pos, f32 radius, f32 height, u32 type, u32 mask,
             u8 weight, u32 param); // 801AF928

    /* 0x44 */ mVec3_c mPos;
}; // size 0x50

// An insect: a sphere the pointer can hit. RTTI dObjc::ins_c.
class ins_c : public dBGC::sphere_c {
public:
    ins_c();             // 801AF988
    virtual ~ins_c() {} // 801B01F0

    void regist(const mVec3_c *pos, f32 radius, u32 param, BOOL enable); // 801AF9DC
    // Whether the segment start..end, swept with radius, touches an enabled insect (its mParam
    // goes to param).
    static BOOL check(const mVec3_c *start, const mVec3_c *end, f32 radius, u32 *param); // 801AFA84
    static void clearList(); // 801AFED0

    static ins_c *s_head; // 8074EAFC

    /* 0x14 */ ins_c *mNext;
    /* 0x18 */ u8 _18[4];
    /* 0x1C */ u32 mParam;
    /* 0x20 */ u8 mHit;
    /* 0x21 */ u8 mRegistered;
    /* 0x22 */ u8 mEnabled;
}; // size 0x24

// A triangle the pointer can hit. RTTI dObjc::dpdTri_c.
class dpdTri_c : public dBGC::poly_c {
public:
    dpdTri_c();             // 801AD88C
    virtual ~dpdTri_c() {} // 801B01B0

    // The center defaults to the triangle's centroid.
    void set(const dBGC::poly_c &poly, u32 type, fBase_c *owner, const mVec3_c *center,
             u32 param); // 801AD8E0
    void regist();       // 801ADA8C
    void init();         // 801ADAEC

    static void create();    // 801ADAFC
    static void clearList(); // 801ADB00
    static void destroy();   // 801ADB2C

    static dpdTri_c *s_head; // 8074EAE0
    static int s_num;        // 8074EAE4

    /* 0x38 */ dpdTri_c *mNext;
    /* 0x3C */ fBaseID_e mOwnerID;
    /* 0x40 */ u8 _40;
    /* 0x41 */ u8 mRegistered;
    /* 0x44 */ u32 mType;
    /* 0x48 */ mVec3_c mCenter;
    /* 0x54 */ u32 mParam;
}; // size 0x58

// A quad (p0, p1, p2, p3) as two dpdTri_c (no RTTI).
class dpdQuad_c {
public:
    void set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, const mVec3_c &p3, u32 type,
             fBase_c *owner, const mVec3_c *center, u32 param); // 801ADB30
    void regist();                                              // 801ADCB8

    /* 0x00 */ dpdTri_c mTri[2];
}; // size 0xB0

// A box (four sides and the top) as five dpdQuad_c (no RTTI).
class dpdBox_c {
public:
    void set(const mVec3_c &pos, f32 sizeX, f32 sizeZ, f32 height, s16 angle, u32 type,
             fBase_c *owner, u32 param); // 801ADD04
    void regist();                       // 801AE200

    /* 0x000 */ dpdQuad_c mQuad[5];
}; // size 0x370

// Resolves the frame's objects: pushes between base_c, then the pointer ray (dObjc_c::execute).
void calc(); // 801AE6EC

} // namespace dObjc
