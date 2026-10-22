#pragma once

// d_line_draw (DOL TU d_line_draw.cpp, .text 800CD670..800CDD2C, .data 804EB320..804EB3A0, .sdata
// 8074AAA0..8074AAA8): dLineDrawProc_c (named by its RTTI, global scope), a debug m3d draw process that draws
// queued 3D lines (opaque lines in drawOpa, translucent ones in drawXlu) and clears the queue after each
// pass. Nothing references it in this build. Member names inferred.

#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/ut.h>
#include <revolution/gx.h>

class dLineDrawProc_c : public m3d::proc_c {
public:
    // A queued line (world positions, already curved by dWorld::curvePosition).
    struct line_c {
        line_c();  // 800CDC7C
        ~line_c(); // 800CDCEC

        void set(const mVec3_c &start, const mVec3_c &end, ::mColor color); // 800CDC88

        /* 0x00 */ mVec3_c mStart;
        /* 0x0C */ mVec3_c mEnd;
        /* 0x18 */ ::mColor mColor; // alpha 0xFF: drawn in drawOpa, else in drawXlu
    }; // size 0x1C

    dLineDrawProc_c();                  // 800CD670
    // No user-declared dtor: the implicit one (800CDB40) is inlined by its
    // users (d_a_balloonNP).
    virtual void drawOpa(); // 800CDCE4
    virtual void drawXlu(); // 800CDCDC

    BOOL destroy();                                                            // 800CD6C4
    BOOL addLine(const mVec3_c *start, const mVec3_c *end, ::mColor color);     // 800CD72C
    void draw(BOOL opa);                                                       // 800CD7F0
    BOOL create(EGG::Heap *heap, u32 lineMax);                                 // 800CDBB0

    /* 0x08 */ line_c *mLines;
    /* 0x0C */ u32 mLineMax;
    /* 0x10 */ u32 mLineNum;
    /* 0x14 */ EGG::FrmHeap *mHeap;
    /* 0x18 */ mAllocator_c mAllocator;
    /* 0x34 */ GXColor mMatColor; // the channel material color (never set here)
}; // size 0x38
