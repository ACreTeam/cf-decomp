#pragma once
#include <revolution/gx.h>

// Virtual screen size the faders draw in (4:3 width; 16:9 width).
#define VI_VIRTUAL_WIDTH_STD (608.0f)
#define VI_VIRTUAL_WIDTH_WIDE (832.0f)
#define VI_VIRTUAL_HEIGHT (456.0f)
#define VI_VIRTUAL_HALF_WIDTH_STD (VI_VIRTUAL_WIDTH_STD / 2)
#define VI_VIRTUAL_HALF_WIDTH_WIDE (VI_VIRTUAL_WIDTH_WIDE / 2)
#define VI_VIRTUAL_HALF_HEIGHT (VI_VIRTUAL_HEIGHT / 2)

class mVideo {
public:
    static void create();

    float getWidth() const { return mRenderModeObj.fbWidth; }
    float getHeight() const { return mRenderModeObj.efbHeight; }

    static float getSmth(float offs) { return (m_video->mRenderModeObj.efbHeight - offs) * 0.5f; }

    GXRenderModeObj &mRenderModeObj;

    static mVideo *m_video;
};
