#pragma once

// The flags (REL d_flagNP, profile FLAG, RTTI dFlag_c / dFlag_c::drawProc_c):
// the town flag on its pole (kind 0, the town's flag design) and the player
// houses' flags (kind 1 + the home index, the home's design), which copy the
// town flag's cloth instead of simulating their own. A 5 x 5 cloth of points
// drawn as a triangle strip with the design as its texture. Source:
// src/d_flagNP/d_flag.cpp. Names are inferred.

// The include order sets the vtable / RTTI order in .data (reverse class
// completion): m3d's classes before fBase_c.
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/g3d.h>

namespace dSTR {
class colArea_c;
}

#define FLAG_POINT_X 5 // along the flag (column 0 is on the pole)
#define FLAG_POINT_Y 5
#define FLAG_POINT_NUM (FLAG_POINT_X * FLAG_POINT_Y)

class dFlag_c : public fBase_c {
  public:
    enum {
        KIND_TOWN, // the town flag; KIND_HOME + n: player home n's flag
        KIND_HOME,
    };

    // The cloth: two buffers of points / normals (the GPU draws one while the
    // other is calculated).
    class drawProc_c : public m3d::proc_c {
      public:
        drawProc_c() : mResFile(NULL) {
        }
        virtual void drawOpa();

        void init(const mMtx_c *mtx, const mAng &angle);
        void calc(const mMtx_c *mtx, BOOL copy);
        void setupGX();
        void drawCloth();

        /* 0x008 */ mMtx_c mMtx;
        /* 0x038 */ int mBuf; // the buffer drawn
        /* 0x03C */ mVec3_c mWind;
        /* 0x048 */ mVec3_c mPos[2][FLAG_POINT_NUM];
        /* 0x2A0 */ mVec3_c mNrm[2][FLAG_POINT_NUM];
        /* 0x4F8 */ f32 mTexCoord[FLAG_POINT_NUM][2];
        /* 0x5C0 */ mVec3_c mVel[FLAG_POINT_NUM];
        /* 0x6EC */ nw4r::g3d::ResFile mResFile; // the design's texture
        /* 0x6F0 */ mAng mWindAngle;
    }; // size 0x6F4

    dFlag_c();
    virtual ~dFlag_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    // Creates a flag of kind at pos (the house flags: parent is the house).
    static fBase_c *construct(const mVec3_c *pos, int kind, fBase_c *parent);

    static dFlag_c *s_townFlag;

    /* 0x064 */ drawProc_c mProc;
    /* 0x758 */ mAllocator_c mAllocator;
    /* 0x774 */ dItem::resLoader_c mLoader; // the flag design
    /* 0x7E4 */ dSTR::colArea_c *mColArea;  // the house's (culling)
    /* 0x7E8 */ mVec3_c mPos;
    /* 0x7F4 */ int mKind;
}; // size 0x7F8
