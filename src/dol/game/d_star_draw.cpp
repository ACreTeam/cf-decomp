// .text 80163AA0..80166990, .data 804F0C50..804F35B0, .sdata 8074B340..8074B348, .sdata2 80750EE0..80750FA0.
#include <game/game/d_star_draw.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_save_data.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_color.hpp>
#include <game/sLib/s_lib.hpp>
#include <nw4r/g3d.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>
#include <cmath>
#include <string.h>

// Not decompiled yet (C linkage keeps the target names).
extern "C" {
BOOL fn_80150D3C(dSaveUnk72E0A_c *signs, int idx);         // 80150D3C: the constellation has an owner
dStarSign_c *fn_80150D24(dSaveUnk72E0A_c *signs, int idx); // 80150D24: a constellation
void fn_802466E8(u32 mask);                                 // 802466E8: invalidate the cached GX state
}

// 80163AA0
BOOL dStarDraw_c::isStarFree(int star, const dStarDraw_c *draw) {
    dSaveUnk72E0A_c *signs = &dSaveData_c::getRaw()->_072E0A;
    for (int i = 0; i < 16; i++) {
        if (fn_80150D3C(signs, i) && i != draw->mSignIdx) {
            u16 *lines = fn_80150D24(signs, i)->mLines;
            for (int j = 0; j < 16; j++, lines++) {
                int line = *lines;
                if (line < LINE_NUM) {
                    line_c *l = &sLines[*lines];
                    if (star == l->mStar1 || star == l->mStar2) {
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;
}

// 80163C14
BOOL dStarDraw_c::isStarSelectable(int star, const dStarDraw_c *draw) {
    if (!isStarFree(star, draw)) {
        return FALSE;
    }
    BOOL isEmpty = TRUE;
    u16 *lines = draw->mSign->mLines;
    for (int j = 0; j < 16; j++, lines++) {
        if (*lines != 0xFFFF) {
            line_c *l = &sLines[*lines];
            if (l->mStar1 == star || l->mStar2 == star) {
                return TRUE;
            }
            isEmpty = FALSE;
        }
    }
    return isEmpty;
}

// 80163D68
BOOL dStarDraw_c::isLineFree(u16 line, const dStarDraw_c *draw) {
    dStarSign_c *sign = draw->mSign;
    if (sign != NULL) {
        u16 *lines = sign->mLines;
        for (int j = 0; j < 16; j++, lines++) {
            if (*lines != 0xFFFF && line == *lines) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 80163E78
BOOL dStarDraw_c::isLineSelectable(u16 line, const dStarDraw_c *draw) {
    if (isLineFree(line, draw)) {
        line_c *l = &sLines[line];
        if (isStarFree(l->mStar1, draw) && isStarFree(l->mStar2, draw)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80163EFC
void dStarDraw_c::traceSign(const dStarSign_c *sign, int star, u16 skipLine, u8 *visited) {
    for (int i = 0; i < 16; i++) {
        const u16 *line = &sign->mLines[i];
        if (*line != 0xFFFF && skipLine != *line && !visited[i]) {
            line_c *l = &sLines[*line];
            if (l->mStar1 == star) {
                visited[i] = TRUE;
                traceSign(sign, l->mStar2, skipLine, visited);
            } else if (l->mStar2 == star) {
                visited[i] = TRUE;
                traceSign(sign, l->mStar1, skipLine, visited);
            }
        }
    }
}

// 80163FD8
BOOL dStarDraw_c::isLineRemovable(u16 line, const dStarDraw_c *draw) {
    const dStarSign_c *sign = draw->mSign;
    if (sign != NULL) {
        u8 visited1[16];
        u8 visited2[16];
        u8 emptyVisited[16];
        memset(visited1, 0, sizeof(visited1));
        memset(visited2, 0, sizeof(visited2));
        memset(emptyVisited, 0, sizeof(emptyVisited));
        line_c *l = &sLines[line];
        s16 end = l->mStar2;
        traceSign(sign, l->mStar1, line, visited1);
        traceSign(sign, end, line, visited2);
        if (memcmp(visited1, visited2, sizeof(visited1)) == 0 ||
            memcmp(visited1, emptyVisited, sizeof(visited1)) == 0 ||
            memcmp(visited2, emptyVisited, sizeof(visited2)) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801640DC
bool dStarDraw_c::create(EGG::Heap *heap) {
    if (mCreated) {
        return true;
    }
    if (mRes.load("/Sky/bg_star.brres", heap, 0)) {
        mAllocator.attach(heap, 0x20);
        proc_c::create(&mAllocator, NULL);
        setup(nw4r::g3d::ResFile(mRes.getData()));
        mCreated = true;
        return true;
    }
    return false;
}

// 8016418C
bool dStarDraw_c::unload() {
    if (mRes.unload(FALSE)) {
        if (mCreated) {
            scnLeaf_c::remove();
            mCreated = false;
        }
        return true;
    }
    return false;
}

// 801641F0
void dStarDraw_c::setup(nw4r::g3d::ResFile file) {
    mResFile = file;
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    mdl.GetResMat(0).GetResMatMisc().SetFogIdx(-1);
    for (int i = 0; i < 8; i++) {
        mStars[i].init(i);
    }
}

static inline bool isZero(f32 val) {
    return std::fabs(val) <= FLT_EPSILON;
}

static inline f32 tanAng(const mAng &ang) {
    f32 c = ang.cos();
    f32 s = ang.sin();
    if (isZero(c)) {
        return s > 0.0f ? 1.0f / 0.0f : -1.0f / 0.0f;
    }
    return s / c;
}

// 80164278
void dStarDraw_c::calc(const mVec3_c *pos, const mAng &angle) {
    mVec3_c trans(0.0f, -330.0f, 0.0f);
    mPos = *pos;
    setAngle(angle);
    trans += *pos;

    f32 t = tanAng(mAng(11.25f * mAng::DegreeToAngleCoefficient));
    f32 half = 0.5f; // really? I couldn't match this otherwise though.
    f32 slope = t * (trans.z * half);
    f32 scale = 1.0f - m0AC * half;
    f32 offset = slope * (m0B0 * m0AC);
    trans.y += offset;
    trans.z *= scale;

    mMtx.trans(trans);
    mMtx.ZrotM(angle);
    for (int i = 0; i < 8; i++) {
        mStars[i].calc();
    }
}

// 80164414
void dStarDraw_c::getStarScreenPos(int star, mVec2_c *out) {
    nw4r::math::MTX44 proj;
    lbl_8074E9B0->getProjectionMtx(&proj);
    mVec3_c pos(sStars[star].x, sStars[star].y, 0.0f);
    mVec3_c view;
    fn_803911A4(mMtx, pos, view);
    f32 x = proj._00 * view.x + proj._01 * view.y + proj._02 * view.z + proj._03;
    f32 y = proj._10 * view.x + proj._11 * view.y + proj._12 * view.z + proj._13;
    f32 w = proj._30 * view.x + proj._31 * view.y + proj._32 * view.z + proj._33;
    f32 scale = 1.0f / (2.0f * w);
    out->x = 608.0f * (x * scale);
    out->y = 456.0f * (y * scale);
}

// Placeholder for a stripped function that pooled the tan(11.25 degrees) double here.
f32 dStarDraw_pooledTan() {
    return tan(NW4R_MATH_DEG_TO_RAD(11.25f));
}

// 8016456C
int dStarDraw_c::findStar(const mVec2_c *pos, StarFilter filter, f32 radius) {
    f32 best = radius * radius;
    int found = -1;
    for (int i = 0; i < 400; i++) {
        mVec2_c screen;
        getStarScreenPos(i, &screen);
        mVec2_c d = *pos - screen;
        f32 dist = d.x * d.x + d.y * d.y;
        if (dist < best && (filter == NULL || filter(i, this))) {
            best = dist;
            found = i;
        }
    }
    return found;
}

// 80164648
int dStarDraw_c::findLinkedStar(int star, const mVec2_c *pos, u16 *line, LineFilter filter, f32 radius) {
    f32 best = radius * radius;
    int foundLine = 0xFFFF;
    int found = -1;
    line_c *l = sLines;
    for (int i = 0; i < LINE_NUM; i++, l++) {
        int other;
        if (l->mStar1 == star) {
            other = l->mStar2;
        } else if (l->mStar2 == star) {
            other = l->mStar1;
        } else {
            continue;
        }
        if (filter != NULL && !filter(i, this)) {
            continue;
        }
        mVec2_c screen;
        getStarScreenPos(other, &screen);
        mVec2_c d = *pos - screen;
        f32 dist = d.x * d.x + d.y * d.y;
        if (dist < best) {
            best = dist;
            foundLine = i;
            found = other;
        }
    }
    if (line != NULL) {
        *line = foundLine;
    }
    return found;
}

// 80164760
int dStarDraw_c::findLine(int star, const mVec2_c *pos, LineFilter filter, f32 radius) {
    f32 best = radius * radius;
    int found = 0xFFFF;
    line_c *l = sLines;
    for (int i = 0; i < LINE_NUM; i++, l++) {
        if (l->mStar1 != star && l->mStar2 != star) {
            continue;
        }
        if (filter != NULL && !filter(i, this)) {
            continue;
        }
        mVec2_c start;
        getStarScreenPos(l->mStar1, &start);
        mVec2_c end;
        getStarScreenPos(l->mStar2, &end);
        mVec2_c dir = end - start;
        mVec2_c rel = *pos - start;
        f32 dot = dir.x * rel.x + dir.y * rel.y;
        f32 len = dir.x * dir.x + dir.y * dir.y;
        EGG::Vector2f nearest;
        if (dot < 0.0f) {
            nearest = start;
        } else if (dot > len) {
            nearest = end;
        } else {
            nearest = start + dir * (dot / len);
        }
        nearest -= *pos;
        f32 dist = nearest.x * nearest.x + nearest.y * nearest.y;
        if (dist < best) {
            best = dist;
            found = i;
        }
    }
    return found;
}

// 80164934
u16 dStarDraw_c::findSignLine(const mVec2_c *pos, LineFilter filter, u16 *slot, f32 radius) {
    dStarSign_c *sign = mSign;
    u16 found = 0xFFFF;
    int foundSlot = 16;
    if (mSignIdx >= 0 && sign != NULL) {
        f32 best = radius * radius;
        u16 *lines = sign->mLines;
        for (u32 j = 0; j < 16; j++, lines++) {
            u16 l = *lines;
            if (l == 0xFFFF) {
                continue;
            }
            line_c *line = &sLines[l];
            if (filter != NULL && !filter(l, this)) {
                continue;
            }
            mVec2_c start;
            getStarScreenPos(line->mStar1, &start);
            mVec2_c end;
            getStarScreenPos(line->mStar2, &end);
            mVec2_c dir = end - start;
            mVec2_c rel = *pos - start;
            f32 dot = dir.x * rel.x + dir.y * rel.y;
            f32 len = dir.x * dir.x + dir.y * dir.y;
            EGG::Vector2f nearest;
            if (dot < 0.0f) {
                nearest = start;
            } else if (dot > len) {
                nearest = end;
            } else {
                nearest = start + dir * (dot / len);
            }
            nearest -= *pos;
            f32 dist = nearest.x * nearest.x + nearest.y * nearest.y;
            if (dist < best) {
                best = dist;
                found = *lines;
                foundSlot = j;
            }
        }
    }
    if (slot != NULL) {
        *slot = foundSlot;
    }
    return found;
}

// 80164B30
int dStarDraw_c::findSign(const mVec2_c *pos, f32 radius) {
    dSaveUnk72E0A_c *signs = &dSaveData_c::getRaw()->_072E0A;
    f32 best = radius * radius;
    int found = -1;
    for (int i = 0; i < 16; i++) {
        if (fn_80150D3C(signs, i)) {
            dStarSign_c *sign = fn_80150D24(signs, i);
            u16 *lines = sign->mLines;
            for (u32 j = 0; j < 16; j++, lines++) {
                u16 l = *lines;
                if (l != 0xFFFF) {
                    line_c *line = &sLines[l];
                    mVec2_c start;
                    getStarScreenPos(line->mStar1, &start);
                    mVec2_c end;
                    getStarScreenPos(line->mStar2, &end);
                    mVec2_c dir = end - start;
                    mVec2_c rel = *pos - start;
                    f32 dot = dir.x * rel.x + dir.y * rel.y;
                    f32 len = dir.x * dir.x + dir.y * dir.y;
                    EGG::Vector2f nearest;
                    if (dot < 0.0f) {
                        nearest = start;
                    } else if (dot > len) {
                        nearest = end;
                    } else {
                        nearest = start + dir * (dot / len);
                    }
                    nearest -= *pos;
                    f32 dist = nearest.x * nearest.x + nearest.y * nearest.y;
                    if (dist < best) {
                        best = dist;
                        found = i;
                    }
                }
            }
        }
    }
    return found;
}

// 80164D14
void dStarDraw_c::drawXlu() {
    if (mMode == 2) {
        m3d::resetMaterial();
        fn_802466E8(0x7FF);
        drawBackground();
    }

    m3d::resetMaterial();
    nw4r::g3d::ResMat mat = mResFile.GetResMdl(0).GetResMat(0);
    nw4r::g3d::Draw1Mat1ShpDirectly(mat, nw4r::g3d::ResShp(NULL), NULL, NULL, 0, NULL, NULL);

    nw4r::ut::Color selColor;
    selColor.Set(0xFF, 0x00, 0x00, 0xFF);
    pos_c *star = sStars;
    for (int i = 0; i < 400; i++, star++) {
        const nw4r::ut::Color *color = NULL;
        f32 scale = 1.0f;
        if (i == mSelStar) {
            color = &selColor;
            scale = 1.2f;
        }
        if (drawStar(star, scale, color)) {
            star->mVisible = true;
        } else {
            star->mVisible = false;
        }
    }

    u8 alpha = mAlpha;
    nw4r::ut::Color bgColor;
    bgColor.Set(0x5C, 0x5C, 0xC0, alpha);
    f32 bgScale = 0.75f;
    star = sBgStars;
    for (int i = 0; i < 120; i++, star++) {
        if (drawStar(star, bgScale, &bgColor)) {
            star->mVisible = true;
        } else {
            star->mVisible = false;
        }
    }

    m3d::resetMaterial();
    fn_802466E8(0x7FF);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    void *data;
    u16 width;
    u16 height;
    GXTexFmt format;
    f32 minLod;
    f32 maxLod;
    GXBool mipmap;
    nw4r::g3d::ResTex tex = mResFile.GetResTex("tex_star_line");
    tex.GetTexObjParam(&data, &width, &height, &format, &minLod, &maxLod, &mipmap);
    GXTexObj texObj;
    GXInitTexObj(&texObj, data, width, height, format, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, minLod, maxLod, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP0);

    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    if (mMode != 1) {
        drawSigns();
    }
}

// 801650E8
void dStarDraw_c::drawSigns() {
    dSaveUnk72E0A_c *signs = &dSaveData_c::getRaw()->_072E0A;
    for (int i = 0; i < 16; i++) {
        if (i == mSignIdx && mSign != NULL) {
            u16 *line = mSign->mLines;
            for (int j = 0; j < 16; j++, line++) {
                u16 l = *line;
                if (l == 0xFFFF) {
                    continue;
                }
                mColor color;
                pos_c *star1 = &sStars[sLines[l].mStar1];
                pos_c *star2 = &sStars[sLines[l].mStar2];
                if (mWhiteLines) {
                    color = nw4r::ut::Color();
                } else if (*line == mSelLine) {
                    color.Set(0x15, 0xFF, 0x00, 0xFF);
                } else {
                    color.Set(0xFF, 0x00, 0x00, 0xFF);
                }
                if (star1->mVisible || star2->mVisible) {
                    drawLine(star1, star2, -1.0f, color);
                }
            }
            if (mSelStar >= 0) {
                drawStarLines(mSelStar, mColor(0x80, 0x80, 0xFF, 0x80));
            }
        } else if (fn_80150D3C(signs, i)) {
            u16 *line = fn_80150D24(signs, i)->mLines;
            for (int j = 0; j < 16; j++, line++) {
                u16 l = *line;
                if (l == 0xFFFF) {
                    continue;
                }
                pos_c *star1 = &sStars[sLines[l].mStar1];
                pos_c *star2 = &sStars[sLines[l].mStar2];
                if (star1->mVisible || star2->mVisible) {
                    drawLine(star1, star2, -1.0f, nw4r::ut::Color());
                }
            }
        }
    }
}

// 80165300
void dStarDraw_c::drawBackground() {
    nw4r::ut::Color top;
    nw4r::ut::Color bottom;
    top.Set(0x00, 0x00, 0x40, 0xFF);
    bottom.Set(0x00, 0x00, 0x20, 0xFF);

    m3d::resetMaterial();
    fn_802466E8(0x7FF);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetTevColor(GX_TEVREG0, top);
    GXSetTevColor(GX_TEVREG1, bottom);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXColor white = {0xFF, 0xFF, 0xFF, 0xFF};
    GXSetChanAmbColor(GX_COLOR0A0, white);
    GXSetChanMatColor(GX_COLOR0A0, white);
    GXSetNumTexGens(0);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXLoadPosMtxImm(mMtx_c::Identity, GX_PNMTX0);
    GXLoadNrmMtxImm(mMtx_c::Identity, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    f32 z = -200.0f;
    f32 deg = 0.5f * dCamera_c::getFovy();
    mAng halfFovy = deg * mAng::DegreeToAngleCoefficient;
    f32 t = 0.004f + halfFovy.sin();
    f32 aspect = lbl_8074E9B0->getAspect();
    f32 h = z * t;
    f32 w = h * aspect;
    mVec3_c topLeft(-w, h, z);
    mVec3_c topRight(w, h, z);
    mVec3_c bottomLeft(-w, -h, z);
    mVec3_c bottomRight(w, -h, z);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(topRight.x, topRight.y, topRight.z);
    GXColor4u8(top.r, top.g, top.b, top.a);
    GXPosition3f32(topLeft.x, topLeft.y, topLeft.z);
    GXColor4u8(top.r, top.g, top.b, top.a);
    GXPosition3f32(bottomRight.x, bottomRight.y, bottomRight.z);
    GXColor4u8(bottom.r, bottom.g, bottom.b, bottom.a);
    GXPosition3f32(bottomLeft.x, bottomLeft.y, bottomLeft.z);
    GXColor4u8(bottom.r, bottom.g, bottom.b, bottom.a);
    GXEnd();
}

// 80165700
u8 dStarDraw_c::calcAlpha(const nw4r::math::VEC3 *pos) {
    if (mMode == 2) {
        return 0xFF;
    }
    f32 y = pos->y - mPos.y;
    f32 top = 400.0f;
    f32 fade = 100.0f;
    f32 bottom = top - fade;
    u8 alpha = mAlpha;
    if (y < top) {
        f32 t = (y - bottom) / (top - bottom);
        alpha = mAlpha * (t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t));
    }
    return alpha;
}

// 8016579C
bool dStarDraw_c::drawStar(pos_c *star, f32 scale, const nw4r::ut::Color *color) {
    star_c *twinkle = &mStars[star->mTwinkle];
    mVec3_c pos(star->x, star->y, 0.0f);
    mVec3_c view;
    fn_803911A4(mMtx, pos, view);
    f32 h = -view.z * (f32)tan(NW4R_MATH_DEG_TO_RAD(11.25f));
    f32 w = h * lbl_8074E9B0->getAspect();
    h += 2.0f;
    w += 2.0f;
    if (view.x < -w || view.x > w || view.y < -h || view.y > h) {
        if (view.y > -h) {
            star->mAlpha = calcAlpha(&view);
        }
        return false;
    }

    mMtx_c mtx = mMtx;
    mtx.trans(view);
    f32 size;
    switch (star->mSize) {
    case 0:
        size = 0.625f;
        break;
    case 1:
        size = 1.0f;
        break;
    case 2:
    default:
        size = 1.375f;
        break;
    }
    star->mAlpha = calcAlpha(&view);
    size *= 1.0f + 0.25f * twinkle->getScale();
    size *= scale;
    mMtx_c scaleMtx;
    PSMTXScale(scaleMtx, size, size, size);
    PSMTXConcat(mtx, scaleMtx, mtx);

    if (color != NULL) {
        GXColor c = {color->r, color->g, color->b, color->a};
        GXSetTevColor(GX_TEVREG0, c);
    } else {
        GXColor c = {0xFF, 0xFF, 0x80, star->mAlpha};
        GXSetTevColor(GX_TEVREG0, c);
    }

    nw4r::g3d::Draw1Mat1ShpDirectly(nw4r::g3d::ResMat(NULL), mResFile.GetResMdl(0).GetResShp(0), &mtx, &mtx, 0, NULL,
                                    NULL);
    return true;
}

// 80165A84
void dStarDraw_c::drawLine(const pos_c *star1, const pos_c *star2, f32 width, const nw4r::ut::Color &color) {
    if (width < 0.0f) {
        width = 1.6f;
    }
    mVec3_c start(star1->x, star1->y, 0.0f);
    mVec3_c end(star2->x, star2->y, 0.0f);
    mMtx_c mtx = mMtx;
    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXLoadNrmMtxImm(mMtx_c::Identity, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXColor c = {color.r, color.g, color.b, mAlpha};
    GXSetTevColor(GX_TEVREG0, c);

    mVec3_c dir = end - start;
    if (dir.normalizeRS()) {
        mVec3_c side = dir;
        side.rotZ(0x4000);
        dir *= 10.0f;
        side *= width;
        mVec3_c v0, v1, v2, v3;
        v3 = end;
        v3 -= dir;
        v3 -= side;
        v2 = end;
        v2 -= dir;
        v2 += side;
        v0 = start;
        v0 += dir;
        v0 += side;
        v1 = start;
        v1 += dir;
        v1 -= side;

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
        u8 alpha1 = star1->mAlpha;
        u8 alpha2 = star2->mAlpha;
        GXPosition3f32(v1.x, v1.y, v1.z);
        GXColor4u8(color.r, color.g, color.b, alpha1);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition3f32(v0.x, v0.y, v0.z);
        GXColor4u8(color.r, color.g, color.b, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(v3.x, v3.y, v3.z);
        GXColor4u8(color.r, color.g, color.b, alpha2);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(v2.x, v2.y, v2.z);
        GXColor4u8(color.r, color.g, color.b, alpha2);
        GXTexCoord2f32(0.0f, 1.0f);
        GXEnd();
    }
}

// 80165E18
void dStarDraw_c::drawStarLines(int star, const nw4r::ut::Color &) {
    pos_c *pos = &sStars[star];
    mVec3_c start(pos->x, pos->y, 0.0f);
    line_c *l = sLines;
    for (int i = 0; i < LINE_NUM; i++, l++) {
        nw4r::ut::Color color;
        color.Set(0x80, 0x80, 0xFF, 0x80);
        f32 width = 1.6f;
        int other;
        if (l->mStar1 == star) {
            other = l->mStar2;
        } else if (l->mStar2 == star) {
            other = l->mStar1;
        } else {
            continue;
        }
        if (!isLineFree(i, this) || !isStarFree(other, this)) {
            continue;
        }
        if (i == mSelLine) {
            color.Set(0x15, 0xFF, 0x00, 0xFF);
            width = 3.2f;
        }
        drawLine(pos, &sStars[other], width, color);
    }
}

// 80165F50
void dStarDraw_c::star_c::init(int idx) {
    f32 t = idx / 8.0f;
    mTimer = 24.0f * t;
    mPhase = 360.0f * t;
    s16 angle = mPhase * mAng::DegreeToAngleCoefficient;
    mAngle = angle;
}

// 80165FBC
void dStarDraw_c::star_c::calc() {
    if (sLib::calcTimer(&mTimer) != 0) {
        f32 phase = mPhase + (360.0f - mPhase) * (1.0f / mTimer);
        s16 angle = phase * mAng::DegreeToAngleCoefficient;
        mPhase = phase;
        mAngle = angle;
    } else {
        mTimer = cM::rndRange<s32>(24, 24);
        mPhase = 0.0f;
        mAngle = 0;
    }
}

// 80166070
f32 dStarDraw_c::star_c::getScale() const {
    return 0.5f - 0.5f * mAng(mAngle).cos();
}

// 801660B0
mAng dStarDraw_c::getSkyAngle(const dTime_c *time) {
    dTime_c t = *time;
    t.add(0, -18, 0, 0);
    f32 days = 365.0f;
    if (dTime_c::isLeapYear(t.year)) {
        days = 366.0f;
    }
    int secs = t.sec + t.min * 60 + t.hour * 3600;
    f32 rate = t.yday / days;
    mAng year = (360.0f * rate - 90.0f) * mAng::DegreeToAngleCoefficient;
    mAng day = 360.0f * (secs / 43200.0f) * mAng::DegreeToAngleCoefficient;
    return day + year;
}

// 80166218
mAng dStarDraw_c::getSignAngle(const dStarSign_c *sign) {
    mVec2_c min(NW4R_MATH_FLT_MAX, NW4R_MATH_FLT_MAX);
    mVec2_c max(-NW4R_MATH_FLT_MAX, -NW4R_MATH_FLT_MAX);
    const u16 *line = sign->mLines;
    for (int i = 0; i < 16; i++, line++) {
        u16 l = *line;
        if (l != 0xFFFF) {
            const line_c *ln = &sLines[l];
            pos_c *star1 = &sStars[ln->mStar1];
            pos_c *star2 = &sStars[ln->mStar2];
            if (min.x > star1->x) {
                min.x = star1->x;
            }
            if (max.x < star1->x) {
                max.x = star1->x;
            }
            if (min.y > star1->y) {
                min.y = star1->y;
            }
            if (max.y < star1->y) {
                max.y = star1->y;
            }
            if (min.x > star2->x) {
                min.x = star2->x;
            }
            if (max.x < star2->x) {
                max.x = star2->x;
            }
            if (min.y > star2->y) {
                min.y = star2->y;
            }
            if (max.y < star2->y) {
                max.y = star2->y;
            }
        }
    }
    EGG::Vector2f center = (min + max) * 0.5f;
    return mAng(0x4000 - cM::atan2s(center.y, center.x));
}

// 8016636C
void dStarDraw_c::getSignDate(const dStarSign_c *sign, dTime_c *date) {
    mAng base = 60.0f * mAng::DegreeToAngleCoefficient;
    int diff = (s16)(getSignAngle(sign) - base);
    f32 days = 365.0f;
    if (dTime_c::isLeapYear(date->year)) {
        days = 366.0f;
    }
    f32 deg = 90.0f + diff * mAng::AngleToDegreeCoefficient;
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    date->mday = 1;
    date->month = 0;
    date->add(deg * days / 360.0f, 0, 0, 0);
}

// 8016646C
void dStarDraw_c::getSignTime(const dStarSign_c *sign, dTime_c *time) {
    dTime_c t = *time;
    t.add(0, -6, 0, 0);
    t.hour = 18;
    t.min = 0;
    t.sec = 0;
    mAng sky = getSkyAngle(&t);
    s16 diff = getSignAngle(sign) - sky;
    f32 deg = diff * mAng::AngleToDegreeCoefficient;
    if (deg < 0.0f) {
        deg += 360.0f;
    }
    int secs = 43200.0 * (deg / 360.0f);
    secs += 30;
    int hours = secs / 3600;
    secs %= 3600;
    t.add(0, hours, secs / 60, 0);
    *time = t;
}

// 80166650
bool dStarDraw_c::isSignVisible(const dStarSign_c *sign, const dTime_c *time) {
    if (time->hour >= 6 && time->hour < 18) {
        return false;
    }
    mAng sky = getSkyAngle(time);
    mAng angle = getSignAngle(sign);
    s16 max = 20.0f * mAng::DegreeToAngleCoefficient;
    s16 min = -20.0f * mAng::DegreeToAngleCoefficient;
    return sLib::isInRange((s16)(sky - angle), min, max);
}

// 80166708
BOOL dStarDraw_c::isSignLinked(const dStarSign_c *sign1, const dStarSign_c *sign2) {
    const u16 *line1 = sign1->mLines;
    for (u32 i = 0; i < 16; i++, line1++) {
        if (*line1 != 0xFFFF) {
            const u16 *line2 = sign2->mLines;
            for (u16 j = 0; j < 16; j++, line2++) {
                if (*line2 != 0xFFFF) {
                    const line_c *a = &sLines[*line1];
                    const line_c *b = &sLines[*line2];
                    if (a->mStar1 == b->mStar1 || a->mStar1 == b->mStar2 || a->mStar2 == b->mStar1 || a->mStar2 == b->mStar2) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

dStarDraw_c::pos_c dStarDraw_c::sStars[400] = {
    {-893.909f, -2.807f, 0, 0, 0, 0},
    {-830.375f, -30.997f, 0, 6, 0, 0},
    {-720.789f, -28.427f, 1, 5, 0, 0},
    {-960.527f, -45.333f, 1, 3, 0, 0},
    {-657.071f, -50.188f, 1, 4, 0, 0},
    {-762.025f, -65.381f, 1, 7, 0, 0},
    {-888.049f, -80.533f, 1, 3, 0, 0},
    {-811.146f, -96.141f, 0, 1, 0, 0},
    {-702.584f, -91.607f, 0, 6, 0, 0},
    {-932.317f, -128.216f, 1, 0, 0, 0},
    {-868.34f, -141.974f, 0, 0, 0, 0},
    {-651.294f, -115.984f, 0, 1, 0, 0},
    {-776.27f, -151.25f, 0, 1, 0, 0},
    {-936.494f, -188.388f, 1, 4, 0, 0},
    {-704.784f, -155.585f, 1, 5, 0, 0},
    {-825.915f, -198.977f, 2, 6, 0, 0},
    {-906.762f, -232.71f, 1, 6, 0, 0},
    {-629.433f, -167.686f, 1, 2, 0, 0},
    {-853.35f, -255.444f, 0, 6, 0, 0},
    {-740.521f, -227.595f, 0, 4, 0, 0},
    {-671.14f, -211.174f, 2, 1, 0, 0},
    {-909.905f, -305.041f, 0, 5, 0, 0},
    {-785.771f, -267.526f, 0, 3, 0, 0},
    {-850.137f, -313.9f, 0, 1, 0, 0},
    {-616.646f, -256.89f, 0, 7, 0, 0},
    {-744.157f, -311.197f, 1, 1, 0, 0},
    {-678.337f, -291.756f, 1, 3, 0, 0},
    {-797.947f, -354.326f, 1, 5, 0, 0},
    {-868.697f, -391.79f, 1, 6, 0, 0},
    {-717.304f, -369.528f, 0, 5, 0, 0},
    {-812.376f, -436.949f, 1, 0, 0, 0},
    {-604.889f, -327.473f, 1, 0, 0, 0},
    {-647.066f, -365.152f, 1, 7, 0, 0},
    {-741.017f, -430.781f, 2, 6, 0, 0},
    {-673.18f, -413.639f, 2, 3, 0, 0},
    {-803.856f, -526.733f, 1, 3, 0, 0},
    {-739.543f, -507.234f, 2, 2, 0, 0},
    {-597.501f, -417.735f, 1, 1, 0, 0},
    {-667.845f, -483.88f, 0, 7, 0, 0},
    {-529.143f, -400.201f, 1, 6, 0, 0},
    {-730.887f, -577.744f, 0, 2, 0, 0},
    {-681.462f, -551.454f, 1, 1, 0, 0},
    {-594.165f, -493.669f, 0, 2, 0, 0},
    {-528.063f, -461.444f, 1, 6, 0, 0},
    {-723.072f, -638.521f, 1, 5, 0, 0},
    {-619.439f, -565.648f, 0, 1, 0, 0},
    {-653.93f, -621.338f, 1, 6, 0, 0},
    {-462.599f, -474.176f, 2, 7, 0, 0},
    {-660.178f, -696.677f, 2, 6, 0, 0},
    {-500.948f, -529.545f, 0, 3, 0, 0},
    {-543.765f, -576.456f, 1, 1, 0, 0},
    {-582.005f, -647.798f, 0, 7, 0, 0},
    {-613.036f, -725.933f, 0, 2, 0, 0},
    {-507.476f, -653.56f, 2, 1, 0, 0},
    {-465.099f, -606.199f, 0, 3, 0, 0},
    {-419.186f, -546.83f, 0, 2, 0, 0},
    {-541.785f, -715.653f, 1, 2, 0, 0},
    {-572.836f, -774.808f, 0, 1, 0, 0},
    {-439.093f, -666.61f, 1, 2, 0, 0},
    {-504.499f, -767.828f, 2, 2, 0, 0},
    {-389.517f, -614.776f, 0, 5, 0, 0},
    {-459.516f, -727.047f, 0, 0, 0, 0},
    {-512.874f, -819.417f, 1, 3, 0, 0},
    {-342.939f, -576.906f, 1, 2, 0, 0},
    {-451.057f, -801.783f, 1, 6, 0, 0},
    {-374.967f, -689.335f, 0, 5, 0, 0},
    {-456.887f, -848.291f, 0, 7, 0, 0},
    {-330.28f, -630.314f, 1, 6, 0, 0},
    {-398.493f, -768.865f, 2, 7, 0, 0},
    {-398.346f, -861.479f, 0, 4, 0, 0},
    {-344.501f, -746.858f, 1, 1, 0, 0},
    {-269.114f, -621.063f, 1, 5, 0, 0},
    {-298.62f, -692.219f, 0, 1, 0, 0},
    {-339.619f, -816.847f, 0, 2, 0, 0},
    {-340.581f, -892.951f, 1, 7, 0, 0},
    {-277.5f, -771.285f, 0, 1, 0, 0},
    {-235.795f, -723.699f, 1, 4, 0, 0},
    {-283.561f, -896.895f, 0, 2, 0, 0},
    {-258.206f, -825.613f, 1, 5, 0, 0},
    {-204.623f, -661.349f, 1, 1, 0, 0},
    {-195.336f, -783.948f, 2, 1, 0, 0},
    {-174.428f, -728.845f, 0, 5, 0, 0},
    {-208.783f, -871.8f, 1, 5, 0, 0},
    {-217.536f, -941.748f, 1, 4, 0, 0},
    {-133.952f, -654.06f, 1, 7, 0, 0},
    {-145.311f, -840.814f, 0, 2, 0, 0},
    {-152.186f, -904.0f, 1, 2, 0, 0},
    {-124.042f, -783.215f, 1, 6, 0, 0},
    {-109.383f, -717.65f, 2, 0, 0, 0},
    {-120.626f, -956.704f, 0, 3, 0, 0},
    {-89.321f, -888.362f, 1, 5, 0, 0},
    {-61.068f, -669.007f, 1, 6, 0, 0},
    {-71.912f, -825.752f, 0, 2, 0, 0},
    {-46.263f, -745.343f, 0, 2, 0, 0},
    {-48.676f, -950.635f, 2, 2, 0, 0},
    {-5.968f, -815.001f, 1, 1, 0, 0},
    {-4.2f, -657.841f, 1, 0, 0, 0},
    {-4.633f, -886.563f, 0, 1, 0, 0},
    {29.369f, -939.041f, 1, 3, 0, 0},
    {27.768f, -713.364f, 1, 6, 0, 0},
    {31.567f, -772.354f, 0, 0, 0, 0},
    {69.181f, -825.624f, 2, 0, 0, 0},
    {82.401f, -878.524f, 2, 3, 0, 0},
    {99.315f, -950.882f, 0, 7, 0, 0},
    {75.129f, -656.42f, 1, 5, 0, 0},
    {102.008f, -766.086f, 0, 1, 0, 0},
    {105.296f, -704.019f, 1, 3, 0, 0},
    {145.487f, -892.999f, 1, 3, 0, 0},
    {170.69f, -952.074f, 1, 4, 0, 0},
    {157.712f, -836.545f, 0, 0, 0, 0},
    {170.929f, -776.61f, 0, 2, 0, 0},
    {163.723f, -710.15f, 0, 2, 0, 0},
    {154.374f, -644.708f, 1, 7, 0, 0},
    {227.583f, -907.023f, 1, 1, 0, 0},
    {221.689f, -827.232f, 0, 0, 0, 0},
    {225.646f, -735.936f, 1, 0, 0, 0},
    {211.919f, -678.576f, 0, 0, 0, 0},
    {298.507f, -906.344f, 2, 7, 0, 0},
    {276.322f, -793.012f, 0, 2, 0, 0},
    {296.861f, -845.507f, 1, 0, 0, 0},
    {292.543f, -736.94f, 0, 4, 0, 0},
    {275.009f, -676.15f, 0, 6, 0, 0},
    {254.441f, -623.632f, 0, 7, 0, 0},
    {364.466f, -866.576f, 0, 7, 0, 0},
    {349.498f, -785.956f, 0, 7, 0, 0},
    {341.146f, -708.331f, 0, 3, 0, 0},
    {427.636f, -866.184f, 1, 1, 0, 0},
    {322.512f, -637.077f, 0, 7, 0, 0},
    {419.617f, -810.329f, 0, 6, 0, 0},
    {413.282f, -734.552f, 1, 0, 0, 0},
    {337.673f, -575.487f, 1, 5, 0, 0},
    {482.479f, -820.566f, 1, 2, 0, 0},
    {393.36f, -666.842f, 2, 6, 0, 0},
    {392.623f, -598.513f, 2, 5, 0, 0},
    {492.937f, -736.954f, 0, 3, 0, 0},
    {542.543f, -795.546f, 0, 1, 0, 0},
    {463.731f, -656.06f, 1, 2, 0, 0},
    {397.875f, -528.078f, 1, 7, 0, 0},
    {555.238f, -725.136f, 0, 0, 0, 0},
    {449.402f, -579.515f, 0, 4, 0, 0},
    {594.988f, -766.544f, 1, 3, 0, 0},
    {542.28f, -666.908f, 1, 5, 0, 0},
    {526.837f, -604.053f, 0, 4, 0, 0},
    {624.988f, -699.469f, 1, 2, 0, 0},
    {467.445f, -507.958f, 0, 1, 0, 0},
    {513.287f, -544.592f, 1, 0, 0, 0},
    {617.573f, -625.852f, 1, 6, 0, 0},
    {580.917f, -564.211f, 0, 7, 0, 0},
    {687.033f, -658.869f, 1, 3, 0, 0},
    {512.211f, -465.698f, 1, 5, 0, 0},
    {570.168f, -503.812f, 0, 5, 0, 0},
    {640.602f, -546.25f, 2, 2, 0, 0},
    {692.518f, -578.533f, 0, 5, 0, 0},
    {747.007f, -613.268f, 0, 6, 0, 0},
    {571.949f, -437.464f, 1, 6, 0, 0},
    {629.263f, -478.223f, 1, 1, 0, 0},
    {525.359f, -396.357f, 1, 1, 0, 0},
    {691.629f, -504.142f, 0, 6, 0, 0},
    {782.711f, -561.932f, 1, 1, 0, 0},
    {748.913f, -528.068f, 2, 3, 0, 0},
    {570.057f, -365.473f, 1, 7, 0, 0},
    {624.6f, -398.545f, 0, 0, 0, 0},
    {684.855f, -432.304f, 1, 4, 0, 0},
    {823.232f, -496.262f, 1, 0, 0, 0},
    {759.201f, -454.743f, 0, 4, 0, 0},
    {604.583f, -323.467f, 1, 3, 0, 0},
    {666.23f, -348.444f, 0, 6, 0, 0},
    {727.102f, -373.89f, 0, 4, 0, 0},
    {784.648f, -389.224f, 1, 5, 0, 0},
    {839.582f, -416.045f, 0, 6, 0, 0},
    {653.396f, -274.569f, 1, 1, 0, 0},
    {777.761f, -326.93f, 0, 1, 0, 0},
    {831.703f, -348.961f, 0, 1, 0, 0},
    {709.813f, -283.325f, 0, 1, 0, 0},
    {618.17f, -223.753f, 1, 4, 0, 0},
    {888.504f, -318.997f, 1, 6, 0, 0},
    {773.994f, -267.536f, 1, 0, 0, 0},
    {836.17f, -285.615f, 2, 7, 0, 0},
    {687.807f, -201.297f, 0, 2, 0, 0},
    {822.405f, -222.954f, 0, 3, 0, 0},
    {758.099f, -197.754f, 1, 5, 0, 0},
    {902.073f, -233.378f, 1, 7, 0, 0},
    {638.022f, -146.645f, 1, 3, 0, 0},
    {831.24f, -161.739f, 2, 1, 0, 0},
    {704.944f, -134.191f, 1, 0, 0, 0},
    {942.847f, -171.712f, 1, 1, 0, 0},
    {774.831f, -122.894f, 0, 0, 0, 0},
    {903.639f, -128.627f, 1, 7, 0, 0},
    {954.741f, -103.334f, 1, 1, 0, 0},
    {654.707f, -68.337f, 2, 0, 0, 0},
    {844.323f, -87.242f, 0, 7, 0, 0},
    {774.228f, -60.054f, 2, 0, 0, 0},
    {706.957f, -51.583f, 0, 0, 0, 0},
    {897.937f, -38.685f, 0, 3, 0, 0},
    {954.748f, -14.996f, 0, 1, 0, 0},
    {793.743f, 1.557f, 0, 3, 0, 0},
    {739.715f, 2.845f, 1, 5, 0, 0},
    {675.996f, 13.861f, 1, 6, 0, 0},
    {849.507f, 19.742f, 0, 1, 0, 0},
    {965.529f, 38.996f, 0, 0, 0, 0},
    {906.778f, 37.79f, 0, 1, 0, 0},
    {779.956f, 66.089f, 1, 4, 0, 0},
    {708.517f, 76.426f, 1, 4, 0, 0},
    {945.782f, 105.983f, 0, 1, 0, 0},
    {648.203f, 75.651f, 1, 4, 0, 0},
    {883.615f, 109.305f, 1, 5, 0, 0},
    {824.487f, 114.158f, 0, 4, 0, 0},
    {939.501f, 177.464f, 1, 3, 0, 0},
    {737.565f, 142.577f, 0, 6, 0, 0},
    {864.49f, 173.096f, 2, 1, 0, 0},
    {652.436f, 137.653f, 1, 1, 0, 0},
    {782.18f, 189.932f, 1, 7, 0, 0},
    {906.147f, 249.926f, 0, 2, 0, 0},
    {834.14f, 236.377f, 0, 3, 0, 0},
    {701.188f, 201.987f, 1, 0, 0, 0},
    {630.767f, 188.214f, 1, 0, 0, 0},
    {771.315f, 260.674f, 0, 4, 0, 0},
    {907.11f, 315.005f, 2, 0, 0, 0},
    {842.896f, 297.401f, 0, 2, 0, 0},
    {658.097f, 247.589f, 0, 1, 0, 0},
    {720.613f, 274.621f, 0, 7, 0, 0},
    {867.641f, 358.731f, 0, 6, 0, 0},
    {795.96f, 338.479f, 1, 3, 0, 0},
    {732.531f, 337.714f, 1, 7, 0, 0},
    {666.009f, 312.906f, 0, 0, 0, 0},
    {862.302f, 413.785f, 0, 1, 0, 0},
    {594.381f, 289.799f, 1, 3, 0, 0},
    {822.733f, 411.012f, 1, 3, 0, 0},
    {749.698f, 399.976f, 0, 1, 0, 0},
    {674.157f, 376.483f, 0, 3, 0, 0},
    {606.839f, 344.879f, 1, 3, 0, 0},
    {833.075f, 474.313f, 0, 0, 0, 0},
    {783.271f, 451.492f, 0, 0, 0, 0},
    {739.434f, 467.322f, 0, 6, 0, 0},
    {686.079f, 439.148f, 0, 2, 0, 0},
    {544.654f, 364.458f, 2, 6, 0, 0},
    {783.008f, 527.754f, 2, 1, 0, 0},
    {621.951f, 430.637f, 0, 6, 0, 0},
    {715.216f, 513.651f, 0, 4, 0, 0},
    {563.168f, 413.099f, 2, 0, 0, 0},
    {672.127f, 498.034f, 1, 4, 0, 0},
    {726.955f, 582.352f, 0, 7, 0, 0},
    {683.862f, 571.802f, 0, 1, 0, 0},
    {641.456f, 541.78f, 2, 3, 0, 0},
    {586.747f, 500.611f, 0, 4, 0, 0},
    {488.489f, 439.709f, 1, 0, 0, 0},
    {525.225f, 486.217f, 0, 4, 0, 0},
    {696.913f, 656.205f, 2, 1, 0, 0},
    {569.041f, 552.58f, 0, 6, 0, 0},
    {607.405f, 592.769f, 2, 3, 0, 0},
    {629.336f, 648.182f, 0, 4, 0, 0},
    {656.1f, 699.287f, 0, 7, 0, 0},
    {489.307f, 554.439f, 2, 0, 0, 0},
    {549.641f, 623.773f, 0, 0, 0, 0},
    {443.881f, 504.72f, 1, 7, 0, 0},
    {568.049f, 676.25f, 0, 4, 0, 0},
    {609.131f, 745.829f, 1, 1, 0, 0},
    {486.73f, 619.14f, 0, 6, 0, 0},
    {423.763f, 573.995f, 0, 3, 0, 0},
    {490.156f, 679.209f, 1, 6, 0, 0},
    {514.15f, 730.554f, 0, 4, 0, 0},
    {525.883f, 788.383f, 1, 7, 0, 0},
    {424.689f, 651.553f, 0, 4, 0, 0},
    {350.86f, 551.264f, 1, 3, 0, 0},
    {453.336f, 716.406f, 1, 7, 0, 0},
    {361.491f, 612.253f, 1, 0, 0, 0},
    {453.183f, 773.368f, 0, 3, 0, 0},
    {478.543f, 831.172f, 1, 7, 0, 0},
    {389.562f, 729.562f, 0, 6, 0, 0},
    {341.576f, 672.967f, 0, 5, 0, 0},
    {396.215f, 808.175f, 0, 5, 0, 0},
    {291.749f, 598.208f, 2, 1, 0, 0},
    {423.071f, 869.399f, 0, 3, 0, 0},
    {329.492f, 757.606f, 1, 2, 0, 0},
    {362.627f, 882.547f, 2, 0, 0, 0},
    {288.965f, 709.53f, 0, 1, 0, 0},
    {322.345f, 822.635f, 0, 5, 0, 0},
    {249.177f, 650.057f, 0, 7, 0, 0},
    {265.005f, 787.48f, 0, 2, 0, 0},
    {283.166f, 855.143f, 0, 4, 0, 0},
    {233.806f, 709.887f, 0, 6, 0, 0},
    {296.466f, 916.358f, 0, 2, 0, 0},
    {224.729f, 850.909f, 0, 7, 0, 0},
    {174.508f, 670.811f, 2, 3, 0, 0},
    {188.387f, 737.118f, 0, 2, 0, 0},
    {229.993f, 907.137f, 0, 3, 0, 0},
    {192.596f, 804.675f, 2, 2, 0, 0},
    {181.215f, 943.925f, 1, 0, 0, 0},
    {154.263f, 873.97f, 0, 0, 0, 0},
    {123.095f, 723.485f, 0, 2, 0, 0},
    {134.632f, 796.657f, 0, 6, 0, 0},
    {101.794f, 656.41f, 1, 4, 0, 0},
    {118.996f, 957.175f, 0, 6, 0, 0},
    {80.128f, 769.02f, 2, 7, 0, 0},
    {83.644f, 833.257f, 0, 5, 0, 0},
    {91.218f, 912.297f, 1, 2, 0, 0},
    {52.304f, 686.566f, 1, 7, 0, 0},
    {35.582f, 948.854f, 2, 0, 0, 0},
    {30.369f, 860.043f, 0, 3, 0, 0},
    {20.875f, 738.639f, 0, 4, 0, 0},
    {11.469f, 796.752f, 0, 2, 0, 0},
    {-26.724f, 905.288f, 0, 7, 0, 0},
    {-21.825f, 681.416f, 0, 2, 0, 0},
    {-43.498f, 745.902f, 1, 0, 0, 0},
    {-48.508f, 837.221f, 0, 5, 0, 0},
    {-62.278f, 963.261f, 1, 4, 0, 0},
    {-95.372f, 903.27f, 1, 3, 0, 0},
    {-82.227f, 710.373f, 0, 5, 0, 0},
    {-101.412f, 775.75f, 1, 0, 0, 0},
    {-116.796f, 836.16f, 0, 2, 0, 0},
    {-149.851f, 941.981f, 0, 6, 0, 0},
    {-127.453f, 669.784f, 0, 3, 0, 0},
    {-139.328f, 724.497f, 0, 4, 0, 0},
    {-173.481f, 875.221f, 0, 3, 0, 0},
    {-176.31f, 799.173f, 2, 7, 0, 0},
    {-210.773f, 918.616f, 2, 7, 0, 0},
    {-229.009f, 857.126f, 0, 1, 0, 0},
    {-194.824f, 690.569f, 0, 7, 0, 0},
    {-216.553f, 749.274f, 0, 1, 0, 0},
    {-295.759f, 920.069f, 1, 7, 0, 0},
    {-206.283f, 626.924f, 2, 6, 0, 0},
    {-271.615f, 801.303f, 1, 3, 0, 0},
    {-303.676f, 867.959f, 0, 7, 0, 0},
    {-252.324f, 675.596f, 0, 4, 0, 0},
    {-296.436f, 734.265f, 2, 7, 0, 0},
    {-327.908f, 791.632f, 0, 7, 0, 0},
    {-376.458f, 863.718f, 1, 6, 0, 0},
    {-322.795f, 674.077f, 1, 2, 0, 0},
    {-391.257f, 793.814f, 1, 7, 0, 0},
    {-294.902f, 596.968f, 1, 3, 0, 0},
    {-439.566f, 852.157f, 1, 2, 0, 0},
    {-374.18f, 721.012f, 0, 0, 0, 0},
    {-463.688f, 777.975f, 0, 7, 0, 0},
    {-432.672f, 724.619f, 0, 6, 0, 0},
    {-373.725f, 602.247f, 0, 7, 0, 0},
    {-516.32f, 807.527f, 1, 2, 0, 0},
    {-433.534f, 662.258f, 0, 6, 0, 0},
    {-503.792f, 730.524f, 0, 7, 0, 0},
    {-379.597f, 528.541f, 1, 6, 0, 0},
    {-561.479f, 756.769f, 1, 1, 0, 0},
    {-510.072f, 673.518f, 1, 5, 0, 0},
    {-448.963f, 585.975f, 1, 2, 0, 0},
    {-484.864f, 631.26f, 0, 3, 0, 0},
    {-565.896f, 694.633f, 0, 4, 0, 0},
    {-621.088f, 739.145f, 1, 7, 0, 0},
    {-563.673f, 628.275f, 1, 0, 0, 0},
    {-519.029f, 573.089f, 0, 3, 0, 0},
    {-457.316f, 502.305f, 1, 7, 0, 0},
    {-643.219f, 686.601f, 2, 3, 0, 0},
    {-628.974f, 627.446f, 0, 3, 0, 0},
    {-543.481f, 517.447f, 0, 3, 0, 0},
    {-475.554f, 450.832f, 1, 0, 0, 0},
    {-702.15f, 647.772f, 0, 6, 0, 0},
    {-599.754f, 548.908f, 1, 4, 0, 0},
    {-547.908f, 457.044f, 0, 5, 0, 0},
    {-690.787f, 566.899f, 0, 6, 0, 0},
    {-615.949f, 476.6f, 0, 5, 0, 0},
    {-664.871f, 514.014f, 0, 0, 0, 0},
    {-769.582f, 572.947f, 1, 6, 0, 0},
    {-546.119f, 400.237f, 1, 0, 0, 0},
    {-730.131f, 511.065f, 1, 1, 0, 0},
    {-616.932f, 419.753f, 1, 4, 0, 0},
    {-798.492f, 520.742f, 1, 0, 0, 0},
    {-691.138f, 436.018f, 0, 2, 0, 0},
    {-597.407f, 346.911f, 0, 5, 0, 0},
    {-765.436f, 442.563f, 0, 5, 0, 0},
    {-659.35f, 372.591f, 1, 5, 0, 0},
    {-843.185f, 459.754f, 0, 0, 0, 0},
    {-736.891f, 373.213f, 2, 0, 0, 0},
    {-815.332f, 397.696f, 1, 2, 0, 0},
    {-648.702f, 304.546f, 0, 0, 0, 0},
    {-870.986f, 403.742f, 1, 5, 0, 0},
    {-714.136f, 315.947f, 0, 3, 0, 0},
    {-597.447f, 257.175f, 1, 0, 0, 0},
    {-806.091f, 341.6f, 0, 3, 0, 0},
    {-869.035f, 348.832f, 0, 3, 0, 0},
    {-776.212f, 296.78f, 1, 0, 0, 0},
    {-666.881f, 244.813f, 1, 1, 0, 0},
    {-852.516f, 286.367f, 0, 1, 0, 0},
    {-910.246f, 298.815f, 2, 5, 0, 0},
    {-734.083f, 227.72f, 0, 3, 0, 0},
    {-631.106f, 186.903f, 1, 7, 0, 0},
    {-824.418f, 235.803f, 0, 5, 0, 0},
    {-910.813f, 238.352f, 2, 0, 0, 0},
    {-704.903f, 180.965f, 0, 5, 0, 0},
    {-792.657f, 170.506f, 2, 2, 0, 0},
    {-855.987f, 174.293f, 1, 0, 0, 0},
    {-936.747f, 191.002f, 0, 5, 0, 0},
    {-665.723f, 131.181f, 0, 3, 0, 0},
    {-714.602f, 105.997f, 1, 5, 0, 0},
    {-904.819f, 126.467f, 0, 0, 0, 0},
    {-833.705f, 108.457f, 0, 0, 0, 0},
    {-777.37f, 95.532f, 0, 3, 0, 0},
    {-960.48f, 109.467f, 0, 0, 0, 0},
    {-673.137f, 59.635f, 1, 5, 0, 0},
    {-889.705f, 65.047f, 1, 5, 0, 0},
    {-818.993f, 36.815f, 1, 3, 0, 0},
    {-947.135f, 34.633f, 2, 1, 0, 0},
    {-752.03f, 25.965f, 0, 0, 0, 0},
    {-679.913f, 6.892f, 0, 2, 0, 0},
};

dStarDraw_c::pos_c dStarDraw_c::sBgStars[120] = {
    {-981.241f, 0.047f, 1, 7, 0, 0},
    {-1036.3f, -5.608f, 0, 5, 0, 0},
    {-1038.271f, -72.695f, 0, 2, 0, 0},
    {-997.106f, -107.256f, 0, 3, 0, 0},
    {-973.624f, -161.827f, 0, 6, 0, 0},
    {-1024.319f, -180.475f, 0, 3, 0, 0},
    {-968.178f, -221.313f, 2, 3, 0, 0},
    {-1014.794f, -260.98f, 0, 4, 0, 0},
    {-942.331f, -266.31f, 1, 2, 0, 0},
    {-989.98f, -346.844f, 1, 3, 0, 0},
    {-947.485f, -378.391f, 1, 0, 0, 0},
    {-918.634f, -464.969f, 1, 1, 0, 0},
    {-868.13f, -462.956f, 2, 2, 0, 0},
    {-875.8f, -552.731f, 1, 5, 0, 0},
    {-799.231f, -577.953f, 0, 7, 0, 0},
    {-837.722f, -630.2f, 1, 2, 0, 0},
    {-790.474f, -648.617f, 0, 3, 0, 0},
    {-750.168f, -712.215f, 2, 0, 0, 0},
    {-683.967f, -772.517f, 1, 0, 0, 0},
    {-644.453f, -816.854f, 0, 1, 0, 0},
    {-602.95f, -850.85f, 2, 4, 0, 0},
    {-554.963f, -865.923f, 0, 5, 0, 0},
    {-487.01f, -909.243f, 1, 0, 0, 0},
    {-447.4f, -940.263f, 2, 1, 0, 0},
    {-371.949f, -961.475f, 1, 0, 0, 0},
    {-316.07f, -986.24f, 0, 3, 0, 0},
    {-291.353f, -942.428f, 2, 6, 0, 0},
    {-211.351f, -991.768f, 1, 4, 0, 0},
    {-149.606f, -1017.41f, 2, 2, 0, 0},
    {-100.262f, -1040.192f, 1, 4, 0, 0},
    {-76.808f, -986.747f, 0, 1, 0, 0},
    {-48.976f, -1031.015f, 2, 0, 0, 0},
    {-13.437f, -983.448f, 2, 4, 0, 0},
    {18.317f, -1023.294f, 0, 3, 0, 0},
    {60.663f, -982.911f, 1, 2, 0, 0},
    {83.85f, -1038.859f, 1, 7, 0, 0},
    {148.677f, -1032.395f, 0, 3, 0, 0},
    {194.705f, -1002.28f, 1, 0, 0, 0},
    {236.449f, -964.089f, 1, 5, 0, 0},
    {253.352f, -1018.633f, 0, 6, 0, 0},
    {325.132f, -998.074f, 1, 1, 0, 0},
    {355.113f, -953.13f, 1, 6, 0, 0},
    {437.727f, -937.747f, 1, 1, 0, 0},
    {468.701f, -885.768f, 1, 2, 0, 0},
    {522.554f, -879.311f, 0, 2, 0, 0},
    {571.01f, -831.511f, 1, 6, 0, 0},
    {625.161f, -836.161f, 2, 5, 0, 0},
    {642.763f, -746.292f, 2, 3, 0, 0},
    {684.115f, -780.266f, 1, 2, 0, 0},
    {731.472f, -737.752f, 1, 3, 0, 0},
    {788.472f, -653.796f, 0, 0, 0, 0},
    {823.157f, -605.429f, 1, 0, 0, 0},
    {854.959f, -561.627f, 0, 3, 0, 0},
    {867.558f, -512.724f, 0, 2, 0, 0},
    {917.635f, -503.069f, 0, 0, 0, 0},
    {873.06f, -462.413f, 0, 6, 0, 0},
    {939.783f, -444.768f, 1, 2, 0, 0},
    {955.198f, -354.449f, 2, 0, 0, 0},
    {977.159f, -285.421f, 2, 4, 0, 0},
    {1010.663f, -232.461f, 0, 1, 0, 0},
    {963.19f, -211.972f, 0, 0, 0, 0},
    {998.917f, -141.773f, 1, 2, 0, 0},
    {1029.792f, -80.692f, 0, 2, 0, 0},
    {1031.366f, -11.718f, 1, 4, 0, 0},
    {1027.584f, 82.068f, 1, 2, 0, 0},
    {1007.84f, 135.344f, 1, 1, 0, 0},
    {1010.027f, 215.496f, 1, 4, 0, 0},
    {1000.711f, 278.82f, 0, 0, 0, 0},
    {951.133f, 288.302f, 1, 1, 0, 0},
    {932.54f, 362.871f, 1, 1, 0, 0},
    {937.035f, 460.419f, 2, 5, 0, 0},
    {880.838f, 464.0f, 0, 2, 0, 0},
    {891.721f, 554.152f, 0, 1, 0, 0},
    {831.935f, 554.329f, 1, 0, 0, 0},
    {844.162f, 613.718f, 1, 0, 0, 0},
    {771.213f, 616.466f, 1, 7, 0, 0},
    {798.15f, 680.947f, 1, 7, 0, 0},
    {747.258f, 664.586f, 0, 0, 0, 0},
    {720.5f, 732.573f, 0, 0, 0, 0},
    {664.6f, 774.664f, 0, 2, 0, 0},
    {577.3f, 784.711f, 1, 5, 0, 0},
    {609.434f, 829.414f, 1, 2, 0, 0},
    {554.011f, 866.077f, 0, 5, 0, 0},
    {516.45f, 908.082f, 0, 0, 0, 0},
    {460.862f, 900.197f, 0, 5, 0, 0},
    {412.199f, 937.472f, 1, 7, 0, 0},
    {348.738f, 966.736f, 1, 5, 0, 0},
    {249.569f, 959.469f, 0, 6, 0, 0},
    {203.865f, 1013.157f, 0, 5, 0, 0},
    {123.436f, 1006.13f, 0, 4, 0, 0},
    {76.616f, 978.674f, 1, 3, 0, 0},
    {82.043f, 1039.19f, 0, 2, 0, 0},
    {0.224f, 1004.926f, 0, 6, 0, 0},
    {-71.444f, 1025.745f, 0, 4, 0, 0},
    {-121.287f, 979.94f, 0, 1, 0, 0},
    {-143.138f, 1031.819f, 1, 3, 0, 0},
    {-179.202f, 990.648f, 1, 2, 0, 0},
    {-232.478f, 979.499f, 2, 1, 0, 0},
    {-340.351f, 984.481f, 1, 2, 0, 0},
    {-365.265f, 918.284f, 0, 1, 0, 0},
    {-410.367f, 896.483f, 2, 0, 0, 0},
    {-458.223f, 924.219f, 0, 2, 0, 0},
    {-493.985f, 887.952f, 1, 4, 0, 0},
    {-545.663f, 893.512f, 1, 0, 0, 0},
    {-582.434f, 838.601f, 1, 5, 0, 0},
    {-644.149f, 809.763f, 0, 5, 0, 0},
    {-693.554f, 734.55f, 0, 3, 0, 0},
    {-731.003f, 694.012f, 1, 2, 0, 0},
    {-749.245f, 634.989f, 1, 1, 0, 0},
    {-801.556f, 650.264f, 1, 2, 0, 0},
    {-837.103f, 605.128f, 0, 1, 0, 0},
    {-846.247f, 544.66f, 1, 6, 0, 0},
    {-900.64f, 477.355f, 1, 3, 0, 0},
    {-962.205f, 415.513f, 1, 4, 0, 0},
    {-927.467f, 377.719f, 1, 0, 0, 0},
    {-980.371f, 339.453f, 0, 4, 0, 0},
    {-972.912f, 272.805f, 1, 0, 0, 0},
    {-998.375f, 222.812f, 0, 1, 0, 0},
    {-1027.091f, 173.625f, 1, 4, 0, 0},
    {-1014.754f, 68.007f, 0, 7, 0, 0},
};

dStarDraw_c::line_c dStarDraw_c::sLines[LINE_NUM + 1] = {
    {0, 1}, {0, 3}, {0, 6}, {0, 395}, {0, 396}, {0, 397}, {1, 5}, {1, 6},
    {1, 7}, {1, 396}, {1, 398}, {2, 4}, {2, 5}, {2, 8}, {2, 398}, {2, 399},
    {3, 6}, {3, 9}, {3, 397}, {4, 8}, {4, 11}, {4, 399}, {5, 7}, {5, 8},
    {5, 12}, {5, 398}, {6, 7}, {6, 9}, {6, 10}, {7, 10}, {7, 12}, {8, 11},
    {8, 12}, {8, 14}, {9, 10}, {9, 13}, {10, 12}, {10, 13}, {10, 15}, {10, 16},
    {11, 14}, {11, 17}, {12, 14}, {12, 15}, {12, 19}, {13, 16}, {14, 17}, {14, 19},
    {14, 20}, {15, 16}, {15, 18}, {15, 19}, {15, 22}, {16, 18}, {16, 21}, {17, 20},
    {17, 24}, {18, 21}, {18, 22}, {18, 23}, {19, 20}, {19, 22}, {19, 25}, {19, 26},
    {20, 24}, {20, 26}, {21, 23}, {21, 28}, {22, 23}, {22, 25}, {22, 27}, {23, 27},
    {23, 28}, {24, 26}, {24, 31}, {25, 26}, {25, 27}, {25, 29}, {26, 29}, {26, 31},
    {26, 32}, {27, 28}, {27, 29}, {27, 30}, {27, 33}, {28, 30}, {29, 32}, {29, 33},
    {29, 34}, {30, 33}, {30, 35}, {30, 36}, {31, 32}, {31, 37}, {31, 39}, {32, 34},
    {32, 37}, {33, 34}, {33, 36}, {33, 38}, {34, 37}, {34, 38}, {35, 36}, {35, 40},
    {36, 38}, {36, 40}, {36, 41}, {37, 38}, {37, 39}, {37, 42}, {37, 43}, {38, 41},
    {38, 42}, {38, 45}, {39, 43}, {39, 47}, {40, 41}, {40, 44}, {40, 46}, {41, 45},
    {41, 46}, {42, 43}, {42, 45}, {42, 49}, {42, 50}, {43, 47}, {43, 49}, {44, 46},
    {44, 48}, {45, 46}, {45, 50}, {45, 51}, {46, 48}, {46, 51}, {47, 49}, {47, 55},
    {48, 51}, {48, 52}, {49, 50}, {49, 54}, {49, 55}, {50, 51}, {50, 53}, {50, 54},
    {51, 52}, {51, 53}, {51, 56}, {52, 56}, {52, 57}, {53, 54}, {53, 56}, {53, 58},
    {53, 61}, {54, 55}, {54, 58}, {54, 60}, {55, 60}, {55, 63}, {56, 57}, {56, 59},
    {56, 61}, {57, 59}, {57, 62}, {58, 60}, {58, 61}, {58, 65}, {59, 61}, {59, 62},
    {59, 64}, {60, 63}, {60, 65}, {60, 67}, {61, 64}, {61, 65}, {61, 68}, {62, 64},
    {62, 66}, {63, 67}, {63, 71}, {64, 66}, {64, 68}, {64, 69}, {65, 67}, {65, 68},
    {65, 70}, {65, 72}, {66, 69}, {67, 71}, {67, 72}, {68, 69}, {68, 70}, {68, 73},
    {69, 73}, {69, 74}, {70, 72}, {70, 73}, {70, 75}, {71, 72}, {71, 79}, {72, 75},
    {72, 76}, {72, 79}, {73, 74}, {73, 75}, {73, 77}, {73, 78}, {74, 77}, {75, 76},
    {75, 78}, {75, 80}, {76, 79}, {76, 80}, {76, 81}, {77, 78}, {77, 82}, {77, 83},
    {78, 80}, {78, 82}, {79, 81}, {79, 84}, {80, 81}, {80, 82}, {80, 85}, {80, 87},
    {81, 84}, {81, 87}, {81, 88}, {82, 83}, {82, 85}, {82, 86}, {83, 86}, {83, 89},
    {84, 88}, {84, 91}, {85, 86}, {85, 87}, {85, 90}, {85, 92}, {86, 89}, {86, 90},
    {87, 88}, {87, 92}, {87, 93}, {88, 91}, {88, 93}, {89, 90}, {89, 94}, {90, 92},
    {90, 94}, {90, 97}, {91, 93}, {91, 96}, {92, 93}, {92, 95}, {92, 97}, {93, 95},
    {93, 96}, {93, 99}, {93, 100}, {94, 97}, {94, 98}, {95, 97}, {95, 100}, {95, 101},
    {96, 99}, {96, 104}, {97, 98}, {97, 101}, {97, 102}, {98, 102}, {98, 103}, {99, 100},
    {99, 104}, {99, 105}, {99, 106}, {100, 101}, {100, 105}, {101, 102}, {101, 105}, {101, 109},
    {102, 103}, {102, 107}, {102, 109}, {103, 107}, {103, 108}, {104, 106}, {104, 112}, {105, 106},
    {105, 109}, {105, 110}, {105, 111}, {106, 111}, {106, 112}, {107, 108}, {107, 109}, {107, 113},
    {108, 113}, {109, 110}, {109, 113}, {109, 114}, {110, 111}, {110, 114}, {110, 115}, {111, 112},
    {111, 115}, {111, 116}, {112, 116}, {112, 122}, {113, 114}, {113, 117}, {113, 119}, {114, 115},
    {114, 118}, {114, 119}, {115, 116}, {115, 118}, {115, 120}, {115, 121}, {116, 121}, {116, 122},
    {117, 119}, {117, 123}, {118, 119}, {118, 120}, {118, 124}, {119, 123}, {119, 124}, {120, 121},
    {120, 124}, {120, 125}, {121, 122}, {121, 125}, {121, 127}, {122, 127}, {122, 130}, {123, 124},
    {123, 126}, {123, 128}, {124, 125}, {124, 128}, {124, 129}, {125, 127}, {125, 129}, {125, 132},
    {126, 128}, {126, 131}, {127, 130}, {127, 132}, {127, 133}, {128, 129}, {128, 131}, {128, 134},
    {129, 132}, {129, 134}, {129, 136}, {130, 133}, {130, 137}, {131, 134}, {131, 135}, {132, 133},
    {132, 136}, {133, 136}, {133, 137}, {133, 139}, {134, 135}, {134, 136}, {134, 138}, {134, 141},
    {135, 138}, {135, 140}, {136, 139}, {136, 141}, {136, 142}, {137, 139}, {137, 144}, {138, 140},
    {138, 141}, {138, 143}, {139, 142}, {139, 144}, {139, 145}, {140, 143}, {141, 142}, {141, 143},
    {141, 146}, {142, 145}, {142, 146}, {142, 147}, {143, 146}, {143, 148}, {144, 145}, {144, 149},
    {145, 147}, {145, 149}, {145, 150}, {146, 147}, {146, 148}, {146, 151}, {146, 152}, {147, 150},
    {147, 151}, {148, 152}, {148, 153}, {149, 150}, {149, 154}, {149, 156}, {150, 151}, {150, 154},
    {150, 155}, {151, 152}, {151, 155}, {151, 157}, {152, 153}, {152, 157}, {152, 159}, {153, 158},
    {153, 159}, {154, 155}, {154, 156}, {154, 160}, {154, 161}, {155, 157}, {155, 161}, {155, 162},
    {156, 160}, {157, 159}, {157, 162}, {157, 164}, {158, 159}, {158, 163}, {159, 163}, {159, 164},
    {160, 161}, {160, 165}, {161, 162}, {161, 165}, {161, 166}, {162, 164}, {162, 166}, {162, 167},
    {163, 164}, {163, 169}, {164, 167}, {164, 168}, {164, 169}, {165, 166}, {165, 170}, {165, 174},
    {166, 167}, {166, 170}, {166, 173}, {167, 168}, {167, 171}, {167, 173}, {168, 169}, {168, 171},
    {168, 172}, {169, 172}, {169, 175}, {170, 173}, {170, 174}, {170, 178}, {171, 172}, {171, 173},
    {171, 176}, {171, 177}, {172, 175}, {172, 177}, {173, 176}, {173, 178}, {173, 180}, {174, 178},
    {174, 182}, {175, 177}, {175, 181}, {176, 177}, {176, 179}, {176, 180}, {177, 179}, {177, 181},
    {178, 180}, {178, 182}, {178, 184}, {179, 180}, {179, 181}, {179, 183}, {180, 183}, {180, 184},
    {180, 186}, {181, 183}, {181, 185}, {181, 187}, {182, 184}, {182, 189}, {183, 186}, {183, 187},
    {183, 190}, {184, 186}, {184, 189}, {184, 192}, {185, 187}, {185, 188}, {186, 190}, {186, 191},
    {186, 192}, {187, 188}, {187, 190}, {187, 193}, {188, 193}, {188, 194}, {189, 192}, {189, 197},
    {190, 191}, {190, 193}, {190, 195}, {190, 198}, {191, 192}, {191, 195}, {191, 196}, {192, 196},
    {192, 197}, {193, 194}, {193, 198}, {193, 200}, {194, 199}, {194, 200}, {195, 196}, {195, 198},
    {195, 201}, {196, 197}, {196, 201}, {196, 202}, {197, 202}, {197, 204}, {198, 200}, {198, 201},
    {198, 205}, {198, 206}, {199, 200}, {199, 203}, {200, 203}, {200, 205}, {201, 202}, {201, 206},
    {201, 208}, {202, 204}, {202, 208}, {202, 210}, {203, 205}, {203, 207}, {204, 210}, {205, 206},
    {205, 207}, {205, 209}, {206, 208}, {206, 209}, {206, 211}, {207, 209}, {207, 212}, {208, 210},
    {208, 211}, {208, 214}, {209, 211}, {209, 212}, {209, 213}, {210, 214}, {210, 215}, {211, 213},
    {211, 214}, {211, 216}, {212, 213}, {212, 217}, {212, 218}, {213, 216}, {213, 218}, {214, 215},
    {214, 216}, {214, 219}, {214, 220}, {215, 219}, {215, 226}, {216, 218}, {216, 220}, {216, 222},
    {216, 223}, {217, 218}, {217, 221}, {218, 221}, {218, 222}, {219, 220}, {219, 224}, {219, 226},
    {220, 223}, {220, 224}, {221, 222}, {221, 225}, {221, 227}, {222, 223}, {222, 227}, {222, 228},
    {223, 224}, {223, 228}, {223, 229}, {224, 226}, {224, 229}, {224, 230}, {225, 227}, {225, 231},
    {226, 230}, {226, 235}, {227, 228}, {227, 231}, {227, 232}, {228, 229}, {228, 232}, {228, 233},
    {228, 234}, {229, 230}, {229, 234}, {229, 237}, {230, 235}, {230, 237}, {230, 239}, {231, 232},
    {231, 236}, {232, 233}, {232, 236}, {233, 234}, {233, 236}, {233, 238}, {233, 240}, {234, 237},
    {234, 240}, {235, 239}, {235, 245}, {236, 238}, {236, 241}, {237, 239}, {237, 240}, {237, 244},
    {238, 240}, {238, 241}, {238, 242}, {239, 244}, {239, 245}, {239, 246}, {240, 242}, {240, 243},
    {240, 244}, {241, 242}, {241, 247}, {242, 243}, {242, 247}, {242, 249}, {242, 250}, {243, 244},
    {243, 248}, {243, 249}, {244, 246}, {244, 248}, {245, 246}, {245, 254}, {246, 248}, {246, 252},
    {246, 254}, {247, 250}, {247, 251}, {248, 249}, {248, 252}, {248, 253}, {249, 250}, {249, 253},
    {250, 251}, {250, 253}, {250, 255}, {251, 255}, {251, 256}, {252, 253}, {252, 254}, {252, 257},
    {252, 258}, {253, 255}, {253, 257}, {253, 259}, {254, 258}, {254, 263}, {255, 256}, {255, 259},
    {255, 260}, {256, 260}, {256, 261}, {257, 258}, {257, 259}, {257, 262}, {258, 262}, {258, 263},
    {258, 265}, {259, 260}, {259, 262}, {259, 264}, {260, 261}, {260, 264}, {260, 266}, {261, 266},
    {261, 267}, {262, 264}, {262, 265}, {262, 268}, {262, 269}, {263, 265}, {263, 271}, {264, 266},
    {264, 268}, {265, 269}, {265, 271}, {266, 267}, {266, 268}, {266, 270}, {267, 270}, {267, 272},
    {268, 269}, {268, 270}, {268, 273}, {269, 271}, {269, 273}, {269, 275}, {269, 277}, {270, 272},
    {270, 273}, {270, 274}, {270, 276}, {271, 277}, {272, 274}, {273, 275}, {273, 276}, {273, 278},
    {274, 276}, {274, 279}, {274, 281}, {275, 277}, {275, 278}, {275, 280}, {276, 278}, {276, 279},
    {277, 280}, {277, 283}, {278, 279}, {278, 280}, {278, 282}, {278, 284}, {278, 286}, {279, 281},
    {279, 282}, {279, 285}, {280, 283}, {280, 284}, {281, 285}, {282, 285}, {282, 286}, {282, 288},
    {283, 284}, {283, 289}, {283, 291}, {284, 286}, {284, 289}, {284, 290}, {285, 287}, {285, 288},
    {286, 288}, {286, 290}, {287, 288}, {287, 292}, {288, 290}, {288, 292}, {288, 294}, {288, 295},
    {289, 290}, {289, 291}, {289, 293}, {289, 296}, {290, 293}, {290, 294}, {291, 296}, {292, 295},
    {292, 297}, {293, 294}, {293, 296}, {293, 299}, {293, 300}, {294, 295}, {294, 298}, {294, 300},
    {295, 297}, {295, 298}, {296, 299}, {296, 302}, {297, 298}, {297, 301}, {297, 305}, {298, 300},
    {298, 301}, {298, 304}, {299, 300}, {299, 302}, {299, 303}, {300, 303}, {300, 304}, {301, 304},
    {301, 305}, {301, 306}, {302, 303}, {302, 307}, {302, 311}, {303, 304}, {303, 307}, {303, 308},
    {304, 306}, {304, 308}, {304, 309}, {305, 306}, {305, 310}, {306, 309}, {306, 310}, {306, 313},
    {307, 308}, {307, 311}, {307, 312}, {308, 309}, {308, 312}, {308, 314}, {309, 313}, {309, 314},
    {310, 313}, {310, 315}, {311, 312}, {311, 317}, {311, 320}, {312, 314}, {312, 317}, {312, 318},
    {313, 314}, {313, 315}, {313, 316}, {314, 316}, {314, 318}, {314, 321}, {315, 316}, {315, 319},
    {316, 319}, {316, 321}, {316, 322}, {317, 318}, {317, 320}, {317, 323}, {318, 321}, {318, 323},
    {318, 324}, {319, 322}, {319, 326}, {320, 323}, {320, 329}, {321, 322}, {321, 324}, {321, 325},
    {322, 325}, {322, 326}, {323, 324}, {323, 327}, {323, 329}, {324, 325}, {324, 327}, {324, 331},
    {325, 326}, {325, 328}, {325, 331}, {326, 328}, {326, 330}, {327, 329}, {327, 331}, {327, 334},
    {327, 336}, {328, 330}, {328, 331}, {328, 332}, {328, 333}, {329, 334}, {329, 338}, {330, 332},
    {330, 335}, {331, 333}, {331, 336}, {332, 333}, {332, 335}, {332, 337}, {333, 336}, {333, 337},
    {333, 340}, {334, 336}, {334, 338}, {334, 341}, {335, 337}, {335, 339}, {336, 340}, {336, 341},
    {336, 342}, {337, 339}, {337, 340}, {337, 343}, {338, 341}, {338, 347}, {338, 351}, {339, 343},
    {339, 344}, {340, 342}, {340, 343}, {340, 345}, {341, 342}, {341, 346}, {341, 347}, {342, 345},
    {342, 346}, {343, 344}, {343, 345}, {343, 348}, {343, 349}, {344, 348}, {345, 346}, {345, 349},
    {345, 353}, {346, 347}, {346, 350}, {346, 353}, {347, 350}, {347, 351}, {348, 349}, {348, 352},
    {349, 352}, {349, 353}, {349, 355}, {350, 351}, {350, 353}, {350, 354}, {350, 356}, {351, 354},
    {351, 359}, {352, 355}, {352, 358}, {353, 355}, {353, 356}, {353, 357}, {354, 356}, {354, 359},
    {354, 361}, {355, 357}, {355, 358}, {355, 360}, {356, 357}, {356, 361}, {356, 363}, {357, 360},
    {357, 363}, {358, 360}, {358, 362}, {359, 361}, {359, 364}, {360, 362}, {360, 363}, {360, 365},
    {361, 363}, {361, 364}, {361, 366}, {362, 365}, {362, 367}, {363, 365}, {363, 366}, {363, 368},
    {364, 366}, {364, 370}, {364, 373}, {365, 367}, {365, 368}, {365, 369}, {366, 368}, {366, 370},
    {366, 372}, {367, 369}, {367, 371}, {368, 369}, {368, 372}, {368, 374}, {368, 376}, {369, 371},
    {369, 374}, {369, 375}, {370, 372}, {370, 373}, {370, 377}, {371, 375}, {372, 376}, {372, 377},
    {372, 380}, {373, 377}, {373, 381}, {374, 375}, {374, 376}, {374, 378}, {375, 378}, {375, 379},
    {376, 378}, {376, 380}, {376, 382}, {377, 380}, {377, 381}, {377, 384}, {378, 379}, {378, 382},
    {378, 383}, {379, 383}, {380, 382}, {380, 384}, {380, 385}, {381, 384}, {381, 388}, {382, 383},
    {382, 385}, {382, 386}, {383, 386}, {383, 387}, {384, 385}, {384, 388}, {384, 389}, {385, 386},
    {385, 389}, {385, 391}, {385, 392}, {386, 387}, {386, 390}, {386, 391}, {387, 390}, {387, 393},
    {388, 389}, {388, 394}, {389, 392}, {389, 394}, {389, 398}, {390, 391}, {390, 393}, {390, 395},
    {391, 392}, {391, 395}, {391, 396}, {392, 396}, {392, 398}, {393, 395}, {393, 397}, {394, 398},
    {394, 399}, {395, 396}, {395, 397}, {396, 398}, {398, 399}, {0, 0},
};
