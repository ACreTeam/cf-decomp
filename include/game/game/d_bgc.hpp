#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>

// Bg collision geometry primitives (namespace dBGC; class names from the RTTI: dBGC::sphere_c,
// poly_c, lineXZ_c, line_c, circle_c, column_c). Used by the bg check (d_bgcf.cpp, dBGCF) and by
// actors directly (d_objc, d_insect_util).
// Source: src/dol/game/d_bgc.cpp (.text 80069D2C..8006C620). See notes/d_bgc.txt.
// Member function names, vecXZ_c and the field names are inferred.
//
// The "correct" functions push a position (pos) that moved from old out of the shape, given the
// mover's radius r, and return whether they hit.
//
// Every vtable pointer is at the end of its class: MWCC places it after the data members declared
// before the first virtual function, so the members come first here.
// The class order matters: vtables come out in reverse class-completion order (sphere_c, poly_c,
// lineXZ_c, line_c). sphere_c is in d_bgc_sphere.hpp (see there).
// The inline getters (getHeight, getDir, getD, ...) are not cosmetic: reading a member through one
// changes register colouring and scheduling, and the target needs them where they are used.

namespace dBGC {

// Near-zero test used by every primitive (|v| < 0.001).
inline bool isZero(f32 v) {
    return EGG::Mathf::abs(v) < 0.001f;
}

// A vertical circle (XZ plane) at mCenter. RTTI dBGC::circle_c (80749CB8). Its only virtual is the
// destructor (kept pure: no circle_c vtable exists in the DOL; 804A6CC4 is column_c's RTTI base list).
class circle_c {
public:
    /* 0x00 */ mVec3_c mCenter;
    /* 0x0C */ f32 mRadius;
    /* 0x10 */ // vtable

    virtual ~circle_c() = 0;

    bool checkInsideXZ(const mVec3_c &p) const; // 80069D2C
}; // size 0x14

inline circle_c::~circle_c() {}

// A vertical cylinder standing on mCenter. RTTI dBGC::column_c, vtable 804A6CA8 (weak, kept in
// d_bgcf.cpp).
class column_c : public circle_c {
public:
    /* 0x14 */ f32 mHeight;

    column_c() {
        mRadius = 0.0f;
        mHeight = 0.0f;
    }
    virtual ~column_c() {}

    f32 getHeight() const { return mHeight; }

    BOOL correctSide(mVec3_c *pos, f32 r) const;                 // 80069DC0
    BOOL correctTop(mVec3_c *pos, const mVec3_c &old) const;     // 80069F2C
    BOOL crossTop(mVec3_c *pos, const mVec3_c &old) const;       // 80069FA8
    BOOL crossSide(mVec3_c *pos, const mVec3_c &old) const;      // 8006A0FC
    BOOL crossTopBottom(mVec3_c *pos, const mVec3_c &old) const; // 8006A410
    BOOL crossSideInf(mVec3_c *pos, const mVec3_c &old) const;   // 8006A5E8
}; // size 0x18

// A 3D segment. RTTI dBGC::line_c, vtable 804A556C.
class line_c {
public:
    /* 0x00 */ mVec3_c mStart;
    /* 0x0C */ mVec3_c mEnd;
    /* 0x18 */ mVec3_c mDir; // normalized mEnd - mStart
    /* 0x24 */ // vtable

    line_c(const mVec3_c &start, const mVec3_c &end) { set(start, end); }
    virtual ~line_c() {} // 8006C558

    const mVec3_c &getStart() const { return mStart; }
    const mVec3_c &getDir() const { return mDir; }

    void calcDir(mVec3_c *dir) const;                              // 8006A904
    f32 calcDistance(const mVec3_c &p) const;                      // 8006A95C
    void set(const mVec3_c &start, const mVec3_c &end);            // 8006A9A0
    f32 calcNearestDistance(mVec3_c *out, const mVec3_c &p) const; // 8006A9D8
    void calcNearest(mVec3_c *out, const mVec3_c &p) const;        // 8006AA3C
    bool checkBetween(const mVec3_c &p) const;                     // 8006AABC
}; // size 0x28

// A 2D vector in the XZ plane (no RTTI; name inferred).
class vecXZ_c {
public:
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 z;

    vecXZ_c() : x(0.0f), z(0.0f) {}
    vecXZ_c(f32 fx, f32 fz) : x(fx), z(fz) {}

    void set(f32 fx, f32 fz) {
        x = fx;
        z = fz;
    }

    void set(const vecXZ_c &v);                         // 8006B870
    vecXZ_c operator+(const vecXZ_c &v) const;          // 8006B884
    void operator+=(const vecXZ_c &v);                  // 8006B8B8
    vecXZ_c operator-(const vecXZ_c &v) const;          // 8006B8DC
    void operator*=(f32 f);                             // 8006B910
    f32 getSquareDistance(const vecXZ_c &v) const;      // 8006B92C
    BOOL normalize();                                   // 8006B964
    void rotY(s16 angle);                               // 8006B9E4
    void setNormal(const vecXZ_c &a, const vecXZ_c &b); // 8006BA44

    static vecXZ_c Zero; // 8074E1E0
}; // size 0x8

// A 2D line (wall seen from above) through mStart and mEnd with its normal (mNormal . p + mD == 0).
// RTTI dBGC::lineXZ_c, vtable 804A5550.
class lineXZ_c {
public:
    /* 0x00 */ vecXZ_c mStart;
    /* 0x08 */ vecXZ_c mEnd;
    /* 0x10 */ vecXZ_c mNormal;
    /* 0x18 */ f32 mD;
    /* 0x1C */ // vtable

    lineXZ_c() {}
    lineXZ_c(const vecXZ_c &start, const vecXZ_c &end, const vecXZ_c &normal) {
        set(start, end, normal);
    }

    void set(const vecXZ_c &start, const vecXZ_c &end, const vecXZ_c &normal); // 8006BAA0
    f32 calcD() const;                                                          // 8006BB08
    f32 calcDist(const vecXZ_c &p) const;                                       // 8006BB2C
    BOOL cross(vecXZ_c *out, const vecXZ_c &a, const vecXZ_c &b) const;         // 8006BB54
    BOOL crossSeg(vecXZ_c *out, const vecXZ_c &a, const vecXZ_c &b) const;      // 8006BD08
    BOOL checkInside(const vecXZ_c &p) const;                                   // 8006BE38
    BOOL correctFace(vecXZ_c *pos, const vecXZ_c &old, f32 r) const;            // 8006BFD4
    BOOL correctEdge(vecXZ_c *pos, const vecXZ_c &old, f32 r) const;            // 8006C0E8
    BOOL correctCross(vecXZ_c *pos, const vecXZ_c &old, f32 r, BOOL flag = FALSE) const; // 8006C2D4

    virtual BOOL isValid() const { return TRUE; } // 8006C598

    f32 getNormalX() const { return mNormal.x; }
    f32 getNormalZ() const { return mNormal.z; }
    f32 getD() const { return mD; }
}; // size 0x20

// A triangle with its plane (mNormal . p + mD == 0). RTTI dBGC::poly_c, vtable 804A5524.
class poly_c {
public:
    /* 0x00 */ mVec3_c mPos0;
    /* 0x0C */ mVec3_c mPos1;
    /* 0x18 */ mVec3_c mPos2;
    /* 0x24 */ mVec3_c mNormal;
    /* 0x30 */ f32 mD;
    /* 0x34 */ // vtable

    poly_c(); // 8006AB54
    poly_c(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2) { set(p0, p1, p2); }
    poly_c(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2, const mVec3_c &normal) {
        set(p0, p1, p2, normal);
    }

    f32 calcY(const mVec3_c &p) const; // 8006AB78

    virtual BOOL correctFace(mVec3_c *pos, const mVec3_c &old, f32 r) const;  // 8006ADF0
    virtual BOOL correctCross(mVec3_c *pos, const mVec3_c &old, f32 r) const; // 8006AD04
    virtual BOOL correctEdge(mVec3_c *pos, const mVec3_c &old, f32 r) const;  // 8006AF0C
    virtual BOOL correct(mVec3_c *pos, const mVec3_c &old, f32 r) const;      // 8006AC34
    virtual ~poly_c() {}                                                // 8006C5A0

    static void calcNormal(mVec3_c *normal, const mVec3_c &p0, const mVec3_c &p1,
                           const mVec3_c &p2); // 8006B0E4
    BOOL set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2); // 8006B174
    BOOL set(const mVec3_c &p0, const mVec3_c &p1, const mVec3_c &p2,
             const mVec3_c &normal);                                    // 8006B208
    BOOL checkInsideXZ(const mVec3_c &p) const;                         // 8006B29C
    f32 calcD() const;                                                  // 8006B398
    f32 calcDist(const mVec3_c &p) const;                               // 8006B3CC
    BOOL checkInsideXY(const mVec3_c &p) const;                         // 8006B404
    BOOL checkInsideYZ(const mVec3_c &p) const;                         // 8006B500
    BOOL crossSeg(mVec3_c *out, const mVec3_c &a, const mVec3_c &b) const;  // 8006B5FC
    BOOL crossLine(mVec3_c *out, const mVec3_c &a, const mVec3_c &b) const; // 8006B69C
}; // size 0x38

} // namespace dBGC
