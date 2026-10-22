#include <lib/egg/core/eggVideo.h>

#include <revolution/os.h>
#include <revolution/sc.h>

namespace EGG {
namespace {

#define RMO_SAMPLE_PATTERN                                                                         \
    {{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}}
#define RMO_VFILTER_INT {7, 7, 12, 12, 12, 7, 7}
#define RMO_VFILTER_PROG {0, 0, 21, 22, 21, 0, 0}

const GXRenderModeObj gRMO_Ntsc_640x456IntDf_4x3 = {
    VI_TVMODE_NTSC_INT, 640, 456, 456, 25, 12, 670, 456, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Ntsc_640x456Prog_4x3 = {
    VI_TVMODE_NTSC_PROG, 640, 456, 456, 25, 12, 670, 456, VI_XFBMODE_SF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_PROG};

const GXRenderModeObj gRMO_Pal50_640x456IntDf_4x3 = {
    VI_TVMODE_PAL_INT, 640, 456, 542, 27, 16, 666, 542, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Pal60_640x456IntDf_4x3 = {
    VI_TVMODE_EURGB60_INT, 640, 456, 456, 25, 12, 670, 456, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Pal60_640x456Prog_4x3 = {
    VI_TVMODE_EURGB60_PROG, 640, 456, 456, 25, 12, 670, 456, VI_XFBMODE_SF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_PROG};

const GXRenderModeObj gRMO_Ntsc_640x456IntDf_16x9 = {
    VI_TVMODE_NTSC_INT, 640, 456, 456, 17, 12, 686, 456, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Ntsc_640x456Prog_16x9 = {
    VI_TVMODE_NTSC_PROG, 640, 456, 456, 17, 12, 686, 456, VI_XFBMODE_SF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_PROG};

const GXRenderModeObj gRMO_Pal50_640x456IntDf_16x9 = {
    VI_TVMODE_PAL_INT, 640, 456, 542, 19, 16, 682, 542, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Pal60_640x456IntDf_16x9 = {
    VI_TVMODE_EURGB60_INT, 640, 456, 456, 17, 12, 686, 456, VI_XFBMODE_DF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_INT};

const GXRenderModeObj gRMO_Pal60_640x456Prog_16x9 = {
    VI_TVMODE_EURGB60_PROG, 640, 456, 456, 17, 12, 686, 456, VI_XFBMODE_SF, GX_FALSE, GX_FALSE,
    RMO_SAMPLE_PATTERN, RMO_VFILTER_PROG};

const Video::RenderModeObjSet gDefaultRenderModeObjSet = {{
    &gRMO_Ntsc_640x456IntDf_4x3,
    &gRMO_Ntsc_640x456Prog_4x3,
    &gRMO_Pal50_640x456IntDf_4x3,
    &gRMO_Pal60_640x456IntDf_4x3,
    &gRMO_Pal60_640x456Prog_4x3,
    &gRMO_Ntsc_640x456IntDf_16x9,
    &gRMO_Ntsc_640x456Prog_16x9,
    &gRMO_Pal50_640x456IntDf_16x9,
    &gRMO_Pal60_640x456IntDf_16x9,
    &gRMO_Pal60_640x456Prog_16x9,
}};

} // namespace

void Video::initialize(GXRenderModeObj *renderMode, const RenderModeObjSet *rmSet) {
    VIInit();
    configure(renderMode, rmSet);
}

GXRenderModeObj *Video::configure(GXRenderModeObj *renderMode, const RenderModeObjSet *rmSet) {
    GXRenderModeObj *oldRenderMode = (GXRenderModeObj *)pRenderMode;

    if (rmSet == NULL) {
        rmSet = &gDefaultRenderModeObjSet;
    }
    if (renderMode == NULL) {
        renderMode = (GXRenderModeObj *)getStandardRenderModeObj(rmSet);
    }

    if (pRenderMode != renderMode) {
        pRenderMode = renderMode;

        VISetBlack(TRUE);
        VIConfigure(renderMode);
        VIFlush();
        m_08 = OSGetTick();
        mFlag.setBit(0);

        u16 fbWidth = renderMode->fbWidth;
        u16 efbHeight = renderMode->efbHeight;

        f32 scaleY = GXGetYScaleFactor(efbHeight, renderMode->xfbHeight);
        u32 numXfbLines = GXGetNumXfbLines(efbHeight, scaleY);

        GXSetDispCopySrc(0, 0, fbWidth, efbHeight);
        GXSetDispCopyDst(fbWidth, numXfbLines);
        GXSetDispCopyYScale(scaleY);

        VIWaitForRetrace();
        VIWaitForRetrace();
#ifdef MUST_MATCH
        // Layout only: keeps the u32 -> float conversion double first in .sdata2 (its user was stripped).
        4503599627370496.0;
#endif
    }

    return oldRenderMode;
}

u32 Video::getTickPerVRetrace(ulong tvFormat) {
    f32 retraceRate;

    switch (tvFormat) {
    case VI_NTSC:
    case VI_EURGB60:
    default: {
        retraceRate = 59.94f;
        break;
    }

    case VI_PAL:
    case VI_MPAL: {
        retraceRate = 50.0f;
        break;
    }
    }

    return static_cast<u32>(OSSecondsToTicks(1)) / retraceRate;
}

u32 Video::getTickPerVRetrace() {
    return getTickPerVRetrace(VIGetTvFormat());
}

const GXRenderModeObj *Video::getStandardRenderModeObj(const RenderModeObjSet *rmSet) {
    bool progressive = SCGetProgressiveMode() == SC_PROGRESSIVE_MODE_ON;
    bool pal60 = SCGetEuRgb60Mode() == SC_EURGB60_MODE_ON;
    bool stdAspect = SCGetAspectRatio() == SC_ASPECT_RATIO_4x3;
    bool componentCable = VIGetDTVStatus() == VI_VISEL_COMPONENT;

    bool ntsc = false;
    switch (VIGetTvFormat()) {
    case VI_NTSC: {
        ntsc = true;
        break;
    }

    case VI_PAL:
    case VI_EURGB60: {
        ntsc = false;
        break;
    }

    default: {
        ntsc = true;
        break;
    }
    }

    if (componentCable && progressive) {
        if (ntsc) {
            return stdAspect ? rmSet->pObj[1] : rmSet->pObj[6];
        } else {
            return stdAspect ? rmSet->pObj[4] : rmSet->pObj[9];
        }
    }

    if (ntsc) {
        return stdAspect ? rmSet->pObj[0] : rmSet->pObj[5];
    }

    if (pal60) {
        return stdAspect ? rmSet->pObj[3] : rmSet->pObj[8];
    }

    return stdAspect ? rmSet->pObj[2] : rmSet->pObj[7];
}

} // namespace EGG
