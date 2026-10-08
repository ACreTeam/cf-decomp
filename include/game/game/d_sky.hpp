#pragma once

// The sky actor (RTTI dSky_c, profile 0xAA): the sky, cloud and moon models, the star field and the
// shooting stars. Source: src/d_skyNP/d_sky.cpp. Names other than the classes' are inferred.

#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_star.hpp>
#include <game/game/d_star_draw.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_mtx.hpp>
#include <nw4r/math.h>

class dSky_c : public fBase_c {
public:
    dSky_c();                         // 0x8C
    virtual ~dSky_c();                // 0x370

    virtual int create();             // 0x4BC
    virtual int preCreate() { return SUCCEEDED; } // 0x1AB0 (weak)
    virtual int doDelete();           // 0xE78
    virtual int execute();            // 0x7E8
    virtual void postExecute(MAIN_STATE_e state) {} // 0x1AAC (weak)
    virtual int draw();               // 0x868
    virtual void postDraw(MAIN_STATE_e state) {}    // 0x1AA8 (weak)
    virtual void deleteReady() {}     // 0x1AA4 (weak)
    virtual bool createHeap() { return true; }      // 0x1A9C (weak)
    virtual void setFireworksBrightness(f32 brightness); // 0xF88: vtable 0x4C

    void calc();                      // 0x1170

    /* 0x000 fBase_c (vtable at 0x060) */
    /* 0x064 */ m3d::smdl_c mMdlCloud;
    /* 0x070 */ m3d::smdl_c mMdlSky;
    /* 0x07C */ m3d::smdl_c mMdlMoon;
    /* 0x088 */ mAllocator_c mAllocator;
    /* 0x0A4 */ dStarDraw_c mStarDraw;
    /* 0x1E8 */ dStarMgr_c mStarMgr;
    /* 0x2F0 */ m3d::anmTexSrt_c mAnmCloud;
    /* 0x31C */ m3d::anmTexSrt_c mAnmMoon;
    /* 0x348 */ dDvd::brresBank_c mResCloud; // /Sky/bg_cloud.brres
    /* 0x3A0 */ dDvd::brresBank_c mResSky;   // /Sky/bg_sky.brres
    /* 0x3F8 */ dDvd::brresBank_c mResUnused;
    /* 0x450 */ dDvd::brresBank_c mResMoon;  // /Sky/bg_moon.brres
    /* 0x4A8 */ mMtx_c mSkyMtx;
    /* 0x4D8 */ mMtx_c mCloudMtx;
    /* 0x508 */ mMtx_c mMoonMtx;
    /* 0x538 */ mMtx_c mMtx538;
    /* 0x568 */ mVec3_c mPos;
    /* 0x574 */ mColor mSkyColor0;
    /* 0x578 */ mColor mSkyColor1;
    /* 0x57C */ u8 _57C[0x584 - 0x57C];
    /* 0x584 */ f32 mBrightness;     // fireworks flashes dim the stars
    /* 0x588 */ u8 _588[0x58C - 0x588];
    /* 0x58C */ bool mCreated;
    /* 0x58D */ bool mSeasonActorCreated;
    /* 0x58E */ bool mIsCity;
}; // size 0x590

extern dSky_c *lbl_8074E830; // 8074E830 (defined in d_play_util.cpp)
