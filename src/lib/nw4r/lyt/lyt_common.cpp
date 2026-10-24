#include <nw4r/lyt.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>

#include <revolution/gx.h>

#include <cstring>

namespace nw4r {
namespace lyt {
namespace detail {

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
bool EqualsResName(const char* name1, const char* name2) {
    return std::strncmp(name1, name2, NW4R_LYT_RES_NAME_LEN) == 0;
}

bool EqualsMaterialName(const char* name1, const char* name2) {
    return std::strncmp(name1, name2, NW4R_LYT_MATERIAL_NAME_LEN) == 0;
}

bool TestFileHeader(const res::BinaryFileHeader& fileHeader, ulong testSig) {
    return testSig == GetSignatureInt(fileHeader.signature) &&
           fileHeader.byteOrder == NW4R_BYTEORDER_BIG;
}

/******************************************************************************
 *
 * TexCoordAry
 *
 ******************************************************************************/
TexCoordAry::TexCoordAry() : mCap(0), mNum(0), mpData(NULL) {}

void TexCoordAry::Free() {
    if (mpData != NULL) {
        Layout::DeleteArray(&mpData[0][0], mNum * VERTEXCOLOR_MAX);
        mpData = NULL;

        mCap = 0;
        mNum = 0;
    }
}

void TexCoordAry::Reserve(u8 num) {
    if (mCap < num) {
        Free();

        const ulong coordNum = num;
        math::VEC2* const pVecAry =
            Layout::NewArray<math::VEC2>(VERTEXCOLOR_MAX * coordNum);
        mpData = reinterpret_cast<TexCoord*>(pVecAry);

        if (mpData != NULL) {
            mCap = num;
        }
    }
}

void TexCoordAry::SetSize(u8 num) {
    if (mpData != NULL && num <= mCap) {
        // clang-format off
        static TexCoord texCoords = {
            math::VEC2(0.0f, 0.0f),
            math::VEC2(1.0f, 0.0f),
            math::VEC2(0.0f, 1.0f),
            math::VEC2(1.0f, 1.0f)
        };
        // clang-format on

        for (int j = mNum; j < num; j++) {
            for (int i = 0; i < VERTEXCOLOR_MAX; i++) {
                mpData[j][i] = texCoords[i];
            }
        }

        mNum = num;
    }
}

void TexCoordAry::Copy(const void* pResTexCoord, u8 texCoordNum) {
    mNum = ut::Max<u8>(mNum, texCoordNum);
    const TexCoord* src = static_cast<const TexCoord*>(pResTexCoord);

    for (int j = 0; j < texCoordNum; j++) {
        for (int i = 0; i < VERTEXCOLOR_MAX; i++) {
            mpData[j][i] = src[j][i];
        }
    }
}

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
bool IsModulateVertexColor(ut::Color* vtxColors, u8 glbAlpha) {
    if (glbAlpha != 255) {
        return true;
    }

    if (vtxColors != NULL && (vtxColors[VERTEXCOLOR_LT] != ut::Color::WHITE ||
                            vtxColors[VERTEXCOLOR_RT] != ut::Color::WHITE ||
                            vtxColors[VERTEXCOLOR_LB] != ut::Color::WHITE ||
                            vtxColors[VERTEXCOLOR_RB] != ut::Color::WHITE)) {
        return true;
    }

    return false;
}

ut::Color MultipleAlpha(ut::Color color, u8 alpha) {
    ut::Color result = color;

    if (alpha != 255) {
        result.a = color.a * alpha / 255;
    }

    return result;
}

void MultipleAlpha(ut::Color* dst, const ut::Color* src, u8 alpha) {
    for (int i = 0; i < VERTEXCOLOR_MAX; i++) {
        dst[i] = MultipleAlpha(src[i], alpha);
    }
}

void SetVertexFormat(bool bModulate, u8 texCoordNum) {
    GXClearVtxDesc();

    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    if (bModulate) {
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    }

    for (int i = 0; i < texCoordNum; i++) {
        GXSetVtxDesc(static_cast<GXAttr>(GX_VA_TEX0 + i), GX_DIRECT);
    }

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);

    if (bModulate) {
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    }

    for (int i = 0; i < texCoordNum; i++) {
        GXSetVtxAttrFmt(GX_VTXFMT0, static_cast<GXAttr>(GX_VA_TEX0 + i),
                        GX_TEX_ST, GX_F32, 0);
    }
}

void DrawQuad(const math::VEC2& basePt, const Size& size, u8 texCoordNum,
              const TexCoord* texCoords, const ut::Color* vtxColors) {

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    {
        GXPosition2f32(basePt.x, basePt.y);
        if (vtxColors != NULL) {
            GXColor1u32(vtxColors[VERTEXCOLOR_LT]);
        }
        for (int i = 0; i < texCoordNum; i++) {
            GXTexCoord2f32(texCoords[i][VERTEXCOLOR_LT].x,
                           texCoords[i][VERTEXCOLOR_LT].y);
        }

        GXPosition2f32(basePt.x + size.width, basePt.y);
        if (vtxColors != NULL) {
            GXColor1u32(vtxColors[VERTEXCOLOR_RT]);
        }
        for (int i = 0; i < texCoordNum; i++) {
            GXTexCoord2f32(texCoords[i][VERTEXCOLOR_RT].x,
                           texCoords[i][VERTEXCOLOR_RT].y);
        }

        GXPosition2f32(basePt.x + size.width, basePt.y - size.height);
        if (vtxColors != NULL) {
            GXColor1u32(vtxColors[VERTEXCOLOR_RB]);
        }
        for (int i = 0; i < texCoordNum; i++) {
            GXTexCoord2f32(texCoords[i][VERTEXCOLOR_RB].x,
                           texCoords[i][VERTEXCOLOR_RB].y);
        }

        GXPosition2f32(basePt.x, basePt.y - size.height);
        if (vtxColors != NULL) {
            GXColor1u32(vtxColors[VERTEXCOLOR_LB]);
        }
        for (int i = 0; i < texCoordNum; i++) {
            GXTexCoord2f32(texCoords[i][VERTEXCOLOR_LB].x,
                           texCoords[i][VERTEXCOLOR_LB].y);
        }
    }
    GXEnd();
}

void DrawQuad(const math::VEC2& basePt, const Size& size, u8 texCoordNum,
              const TexCoord* texCoords, const ut::Color* vtxColors, u8 alpha) {

    ut::Color wkVtxColors[VERTEXCOLOR_MAX];

    if (vtxColors != NULL) {
        MultipleAlpha(wkVtxColors, vtxColors, alpha);
    }

    DrawQuad(basePt, size, texCoordNum, texCoords, vtxColors ? wkVtxColors : NULL);
}

} // namespace detail
} // namespace lyt
} // namespace nw4r
