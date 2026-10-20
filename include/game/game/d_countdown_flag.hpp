#pragma once

// The New Year's countdown flags (REL d_countdown_flagNP, profile
// COUNTDOWN_FLAG, RTTI dCountDownFlag_c): the two town flags on the countdown
// board (d_a_str_cdbbs, which creates this actor as its child), one on each
// side of the board, waving in opposite directions. Each is a 5 x 5 cloth of
// points with the town flag's design as its texture, as in d_flag (whose cloth
// code this copies). Source: src/d_countdown_flagNP/d_countdown_flag.cpp.
// Names are inferred.

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

#define CDFLAG_POINT_X 5 // along the flag (column 0 is on the pole)
#define CDFLAG_POINT_Y 5
#define CDFLAG_POINT_NUM (CDFLAG_POINT_X * CDFLAG_POINT_Y)

// The flags' poles, relative to the board's pole top (l_poleTop): left at -x,
// right at +x.
#define CDFLAG_POLE_DIST_X 19.5f
#define CDFLAG_POLE_L_Z -4.5f
#define CDFLAG_POLE_R_Z -1.0f
// The cloths' rotation on their poles (left: +, right: -).
#define CDFLAG_ANGLE DEG_TO_ANGLE(30)
// The wind's angle step per frame (calc).
#define CDFLAG_WIND_ANGLE_STEP 0x40
// The steps init runs to settle the cloth.
#define CDFLAG_INIT_STEPS 30

class dCountDownFlag_c : public fBase_c {
  public:
    // A flag's cloth: two buffers of points / normals (the GPU draws one while
    // the other is calculated). The left and right flags differ in the
    // cloth's direction and wind.
    class drawProc_c : public m3d::proc_c {
      public:
        drawProc_c() : mResFile(NULL) {
        }
        virtual void drawOpa();
        // Lays the cloth out at mtx (its columns rotated by angle) and runs
        // calc a few times.
        virtual void init(const mMtx_c *mtx, mAng angle) = 0;
        // One step of the cloth at mtx.
        virtual void calc(const mMtx_c *mtx) = 0;

        void setupGX();
        void drawCloth();

        /* 0x008 */ mMtx_c mMtx;
        /* 0x038 */ int mBuf; // the buffer drawn
        /* 0x03C */ mVec3_c mWind;
        /* 0x048 */ mVec3_c mPos[2][CDFLAG_POINT_NUM];
        /* 0x2A0 */ mVec3_c mNrm[2][CDFLAG_POINT_NUM];
        /* 0x4F8 */ f32 mTexCoord[CDFLAG_POINT_NUM][2];
        /* 0x5C0 */ mVec3_c mVel[CDFLAG_POINT_NUM];
        /* 0x6EC */ nw4r::g3d::ResFile mResFile; // the design's texture
        /* 0x6F0 */ mAng mWindAngle;
    }; // size 0x6F4

    // The left flag (its cloth runs to -x).
    class drawProcCDL_c : public drawProc_c {
      public:
        virtual void init(const mMtx_c *mtx, mAng angle);
        virtual void calc(const mMtx_c *mtx);
    };

    // The right flag (its cloth runs to +x).
    class drawProcCDR_c : public drawProc_c {
      public:
        virtual void init(const mMtx_c *mtx, mAng angle);
        virtual void calc(const mMtx_c *mtx);
    };

    dCountDownFlag_c();
    virtual ~dCountDownFlag_c();
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    /* 0x064 */ drawProcCDL_c mProcL;
    /* 0x758 */ drawProcCDR_c mProcR;
    /* 0xE4C */ mAllocator_c mAllocator;
    /* 0xE68 */ dItem::resLoader_c mLoader; // the town flag's design
    /* 0xED8 */ dSTR::colArea_c *mColArea;  // the board's (culling)
    /* 0xEDC */ mVec3_c mPos;               // the board's position
}; // size 0xEE8
