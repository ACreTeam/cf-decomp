#pragma once

// The room object actor base daRmobj_c (RTTI "daRmobj_c", "daRmobj_c::model_c"). DOL TU d_a_rmobj.cpp
// (.text 800636C4..80064FB4) and d_a_rmobj_model.cpp (model_c::create, 80064FB4..80065230). The room objects (RELs d_a_rmobj_*NP: atm, bus, cafe, checkp, fossil, grace,
// theater, ...) derive from it, and some special npcs (d_a_npc_sp_cafe, _fortune, _kappei, _kutsumigaki,
// _resetsan) use its model_c. Its constructor and model_c's are inlined into the RELs (not in the DOL).
// Everything below class level is named from behaviour.

#include <types.h>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>

namespace EGG {
class FrmHeap;
}

class daRmobj_c : public dDemoActor_c, public dMsg::Rcpt_c {
public:
    // A model with its animations, from /Rmobj/<name>.brres (index 0 of each kind). Every animation is
    // optional: the calls on an unbound one do nothing.
    class model_c : public dDvd::brresBank_c {
    public:
        bool remove();                                        // 80063778: unloads, removes everything
        bool entry();                                         // 800638A8: the model's entry (draw)
        nw4r::math::MTX34 *getNodeMtx(const char *name);      // 800638CC: a static copy, NULL if none
        mVec3_c *getNodePos(const char *name);                // 80063960: a static copy (zero if none)
        nw4r::math::MTX34 *getNodeMtx(int node);              // 800639FC
        mVec3_c *getNodePos(int node);                        // 80063A64
        void setRate(f32 rate);                               // 80063B00: all the animations
        BOOL setAnm(u32 idx, m3d::playMode_e mode, f32 blend); // 80063B68: animation idx of every kind
        BOOL setAnm(const char *name, m3d::playMode_e mode, f32 blend); // 80063CAC: every kind named so
        void play();                                          // 80063E00
        BOOL isStop();                                        // 80063E4C: every once-animation stopped

        void setRateChr(f32 rate);                            // 80063EE4
        void setFrameChr(f32 frame);                          // 80063F34
        BOOL setAnmChr(u32 idx, m3d::playMode_e mode, f32 blend); // 80063F84
        BOOL setAnmChr(const char *name, m3d::playMode_e mode, f32 blend); // 80064090
        void playMdl();                                       // 80064194: the model (and its chr animation)
        bool isStopChr();                                     // 800641A4

        void setRateVis(f32 rate);                            // 80064200
        BOOL setAnmVis(u32 idx, m3d::playMode_e mode);        // 80064250
        BOOL setAnmVis(const char *name, m3d::playMode_e mode); // 80064350
        void playVis();                                       // 80064448
        bool isStopVis();                                     // 80064494

        void setRateClr(f32 rate);                            // 800644E8
        BOOL setAnmClr(u32 idx, m3d::playMode_e mode);        // 8006453C
        BOOL setAnmClr(const char *name, m3d::playMode_e mode); // 80064648
        void playClr();                                       // 8006474C
        bool isStopClr();                                     // 80064798

        void setRatePat(f32 rate);                            // 800647F8
        BOOL setAnmPat(u32 idx, m3d::playMode_e mode);        // 8006484C
        BOOL setAnmPat(const char *name, m3d::playMode_e mode); // 80064958
        void playPat();                                       // 80064A5C
        bool isStopPat();                                     // 80064AA8

        void setRateSrt(f32 rate);                            // 80064B08
        BOOL setAnmSrt(u32 idx, m3d::playMode_e mode);        // 80064B5C
        BOOL setAnmSrt(const char *name, m3d::playMode_e mode); // 80064C68
        void playSrt();                                       // 80064D6C
        bool isStopSrt();                                     // 80064DB8

        bool create(const char *name, u32 bufferOption);      // 80064FB4 (d_a_rmobj_model.cpp): loads and builds it

        bool isCreated() const { return mMdl.getScn() != NULL; }

        static char m_frmHeapName[];   // 804A4E60: "daRmobj_c::m_frmHeap_p : <general OBJ heap>"
        static const f32 cCreateRate;  // 80750318: 0 (the animations start stopped)

        /* 0x058 */ m3d::anmChr_c mAnmChr;
        /* 0x090 */ m3d::anmMatClr_c mAnmClr;
        /* 0x0BC */ m3d::anmTexPat_c mAnmPat;
        /* 0x0E8 */ m3d::anmTexSrt_c mAnmSrt;
        /* 0x114 */ m3d::anmVis_c mAnmVis;
        /* 0x14C */ m3d::mdl_c mMdl;
        /* 0x18C */ EGG::FrmHeap *m_frmHeap_p; // "daRmobj_c::m_frmHeap_p" (0x7800, dHeap::localHeap_p)
        /* 0x190 */ mAllocator_c mAllocator;
    };

    virtual ~daRmobj_c();                  // 80064F40
    virtual int preCreate();               // 800636C4
    virtual int preDelete();               // 8006374C
    virtual int preExecute();              // 800636F0
    virtual void postExecute(MAIN_STATE_e state); // 8006371C
    virtual int preDraw();                 // 80063720
    virtual mVec3_c *getDemoPosition();    // 80064EF8: mLookPos, or the position while it is zero

    // 80064E18: mLookPos = the position with x and z moved toward the player, inside min..max.
    void setLookPos(const mVec3_c *min, const mVec3_c *max);

    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 */ mVec3_c mLookPos;
    /* 0x12C */ dSndObjSimple_c mSnd;
}; // size 0x15C
