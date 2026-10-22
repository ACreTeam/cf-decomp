#pragma once

#include <types.h>
#include <lib/egg/gfxe/eggFrustum.h>
#include <nw4r/math.h>

namespace EGG {

class Screen : public Frustum {
public:
    virtual ~Screen() {}

    enum TVMode {
        TV_MODE_4_3,
        TV_MODE_16_9,
        TV_MODE_EFB, // the EFB-sized entry (selected by FLAG_USE_EFB_RATIO)
        TV_MODE_MAX
    };

    struct DataEfb {
        struct Viewport {
            f32 x;      // 0x00
            f32 y;      // 0x04
            f32 width;  // 0x08
            f32 height; // 0x0C
            f32 near;   // 0x10
            f32 far;    // 0x14
        } vp;

        s32 ox; // 0x18
        s32 oy; // 0x1C
    };

    struct TVModeInfo {
        u16 width;
        u16 height;
        nw4r::math::VEC2 ratios; // EFB size / canvas size
    };

    Screen();
    virtual void SetProjectionGX() const;
    virtual void CopyToG3D(nw4r::g3d::Camera camera) const;
    // 80448F1C: the matrix that maps the unit quad onto (x, y, width, height) of this screen (name from SS's EGG).
    void CalcMatrixForDrawQuad(nw4r::math::MTX34 *mtx, f32 x, f32 y, f32 width, f32 height) const;

    void CopyFromAnother(const Screen &); // not in CF's DOL (only dead-stripped users)

    void GetPosSizeInEfb() const;        // 80448BF0
    const DataEfb &GetDataEfb() const;   // 80448E94
    bool IsChangeEfb() const;            // 80448EF0
    void GetGlobalPos(f32 *pX, f32 *pY) const; // 80448FC4
    void GetPosInEfb(f32 x, f32 y, f32 *pX, f32 *pY) const; // 80449190

    void SetParent(const Screen *pParent) {
        if (pParent == NULL) {
            pParent = spRoot;
        }
        if (mParent != pParent) {
            mFlags |= FLAG_DIRTY;
            mParent = pParent;
        }
    }

    static void Initialize(const unsigned short *maxX, const unsigned short *maxY, Screen *pUserRoot);
    static void SetTVMode(TVMode tvMode);
    static void SetTVModeDefault();

    static u16 GetSizeXMax() { return sTVModeInfo[sTVMode].width; }
    static u16 GetSizeYMax() { return sTVModeInfo[sTVMode].height; }

    // CF's (older) EGG::Screen is 0x68 bytes.
    /* 0x3C */ const Screen *mParent;
    /* 0x40 */ nw4r::math::VEC2 mPosition;
    /* 0x48 */ mutable DataEfb mDataEfb;

    static TVMode sTVMode;
    static TVModeInfo sTVModeInfo[Screen::TV_MODE_MAX];

private:
    typedef void (*ChangeTVModeFunc)(void *pArg);

    static Screen *spRoot;                    // 8074FCA4
    static ChangeTVModeFunc sChangeTVModeFunc; // 8074FCA8
    static void *spChangeTVModeFuncInfo;      // 8074FCAC
    static nw4r::math::VEC2 sCanvasScale;     // 8074FCB0
    static nw4r::math::VEC2 sCanvasOffset;    // 8074FCB8
};

} // namespace EGG
