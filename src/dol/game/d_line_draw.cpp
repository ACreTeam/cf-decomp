// d_line_draw: the debug 3D line drawer dLineDrawProc_c. .text 800CD670..800CDD2C, .data 804EB320..804EB3A0,
// .sdata 8074AAA0..8074AAA8. See include/game/game/d_line_draw.hpp.
#include <game/game/d_line_draw.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_light.hpp>
#include <game/mLib/m_heap.hpp>
#include <nw4r/g3d.h>
#include <revolution/MTX.h>

// 800CD670
dLineDrawProc_c::dLineDrawProc_c() {
    mLines = NULL;
    mLineMax = 0;
    mLineNum = 0;
}

// 800CD6C4
BOOL dLineDrawProc_c::destroy() {
    m3d::scnLeaf_c::remove();
    if (mHeap != NULL) {
        mHeap::destroyFrmHeap(mHeap);
        mHeap = NULL;
    }
    if (mLines != NULL) {
        delete[] mLines;
        mLines = NULL;
    }
    return TRUE;
}

// 800CD72C
BOOL dLineDrawProc_c::addLine(const mVec3_c *start, const mVec3_c *end, ::mColor color) {
    if (mLineNum < mLineMax) {
        line_c *line = &mLines[mLineNum];
        mVec3_c curvedStart;
        mVec3_c curvedEnd;
        dWorld::curvePosition(&curvedStart, start);
        dWorld::curvePosition(&curvedEnd, end);
        line->set(curvedStart, curvedEnd, color);
        mLineNum++;
        return TRUE;
    }
    return FALSE;
}

// 800CD7F0
void dLineDrawProc_c::draw(BOOL opa) {
    m3d::resetMaterial();
    nw4r::g3d::G3DState::Invalidate(0x7FF);
    for (line_c *line = mLines; line < &mLines[mLineNum]; line++) {
        BOOL doDraw = FALSE;
        if (opa) {
            if (line->mColor.a == 0xFF) {
                doDraw = TRUE;
            }
        } else {
            if (line->mColor.a != 0xFF) {
                doDraw = TRUE;
            }
        }
        if (doDraw) {
            nw4r::math::MTX34 viewMtx;
            getViewMtx(&viewMtx);
            GXLoadPosMtxImm(viewMtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);
            GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
            GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            GXSetZCompLoc(GX_TRUE);
            GXSetNumChans(1);
            GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, fn_800CD250(), GX_DF_NONE, GX_AF_SPOT);
            GXSetChanAmbColor(GX_COLOR0A0, fn_800CD168(0));
            GXSetChanMatColor(GX_COLOR0A0, mMatColor);
            GXSetNumTexGens(0);
            GXSetNumIndStages(0);
            GXSetNumTevStages(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GXSetCullMode(GX_CULL_NONE);
            nw4r::math::MTX34 posMtx;
            nw4r::math::MTX34 nrmMtx;
            getViewMtx(&posMtx);
            getViewMtx(&nrmMtx);
            PSMTXInvXpose(nrmMtx, nrmMtx);
            GXLoadPosMtxImm(posMtx, GX_PNMTX0);
            GXLoadNrmMtxImm(nrmMtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);
            GXClearVtxDesc();
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
            GXSetTevColor(GX_TEVREG0, line->mColor);
            if (opa) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
            GXSetLineWidth(10, 0);
            GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);
            GXPosition3f32(line->mStart.x, line->mStart.y, line->mStart.z);
            GXPosition3f32(line->mEnd.x, line->mEnd.y, line->mEnd.z);
            GXEnd();
        }
    }
    mLineNum = 0;
}

// 800CDB40
dLineDrawProc_c::~dLineDrawProc_c() {}

// 800CDBB0
BOOL dLineDrawProc_c::create(EGG::Heap *heap, u32 lineMax) {
    mLines = new (heap, 4) line_c[lineMax];
    mLineMax = lineMax;
    if (mHeap == NULL) {
        mHeap = mHeap::createFrmHeap(0x1400, heap, "dLineDrawProc_c::::m_frmHeap_p : ライン表示ヒープ", 0x20, mHeap::OPT_NONE);
        if (mHeap != NULL) {
            mAllocator.attach(mHeap, 0x20);
            m3d::proc_c::create(&mAllocator, NULL);
        }
    }
    return TRUE;
}

// 800CDC7C
dLineDrawProc_c::line_c::line_c() {}

// 800CDC88
void dLineDrawProc_c::line_c::set(const mVec3_c &start, const mVec3_c &end, ::mColor color) {
    mStart = start;
    mEnd = end;
    mColor = color;
}

// 800CDCDC
void dLineDrawProc_c::drawXlu() {
    draw(FALSE);
}

// 800CDCE4
void dLineDrawProc_c::drawOpa() {
    draw(TRUE);
}

// 800CDCEC
dLineDrawProc_c::line_c::~line_c() {}
