#pragma once

// The rainbow in the sky (RTTI dSkyRainbow_c, profile 0xAC), created by dSky_c outside late season.

#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_mtx.hpp>

class dSkyRainbow_c : public fBase_c {
public:
    dSkyRainbow_c();
    virtual ~dSkyRainbow_c();

    virtual int create();
    virtual int preCreate() { return SUCCEEDED; }
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(MAIN_STATE_e state) {}
    virtual int draw();
    virtual void postDraw(MAIN_STATE_e state) {}
    virtual void deleteReady() {}
    virtual bool createHeap() { return true; }

    void calcMtx();

    /* 0x000 fBase_c (vtable at 0x060) */
    /* 0x064 */ dDvd::brresBank_c mRes;
    /* 0x0BC */ m3d::smdl_c mMdl;
    /* 0x0C8 */ m3d::anmTexSrt_c mAnm; // never created
    /* 0x0F4 */ mAllocator_c mAllocator;
    /* 0x110 */ mMtx_c mMtx;
    /* 0x140 */ u8 mAlpha;
}; // size 0x144
