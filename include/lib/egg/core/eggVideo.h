#pragma once

#include <lib/egg/core/eggBitFlag.h>
#include <revolution/gx.h>
#include <revolution/vi.h>

namespace EGG {

class Video {
public:
    // Render modes picked by getStandardRenderModeObj():
    // [0] NTSC int 4:3, [1] NTSC prog 4:3, [2] PAL50 4:3, [3] PAL60 int 4:3, [4] PAL60 prog 4:3,
    // [5..9] the same for 16:9.
    struct RenderModeObjSet {
        const GXRenderModeObj *pObj[10];
    };

    Video(GXRenderModeObj *renderMode, const RenderModeObjSet *rmSet) : pRenderMode(nullptr), mFlag(0), m_08(0) {
        initialize(renderMode, rmSet);
    }

    void initialize(GXRenderModeObj *renderMode, const RenderModeObjSet *rmSet);
    GXRenderModeObj *configure(GXRenderModeObj *renderMode, const RenderModeObjSet *rmSet);

    bool isBlack() { return mFlag & 1; }
    void changeBlack() {
        VISetBlack(mFlag.offBit(0));
        mFlag.toggleBit(0);
    }

    static u32 getTickPerVRetrace();
    static u32 getTickPerVRetrace(ulong);
    static const GXRenderModeObj *getStandardRenderModeObj(const RenderModeObjSet *rmSet);

    const GXRenderModeObj *pRenderMode; // 0x00
    TBitFlag<u8> mFlag;                 // 0x04
    u32 m_08;                           // 0x08 (tick of the last configure)
};

} // namespace EGG
