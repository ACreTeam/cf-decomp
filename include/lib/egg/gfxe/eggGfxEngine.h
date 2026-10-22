#pragma once

#include <types.h>
#include <revolution/GX.h>

namespace EGG {

namespace GfxEngine {
    class Configuration {
    public:
        Configuration();

        u16 m_00;
        u16 m_02;
        GXColor mColor;
        GXPixelFmt mPixelFmt;
        u16 m_0C;
        u16 m_0E;
        u32 mBufferSize;
        u16 m_14;
        u16 m_16;
        u8 m_18;
        // MWCC places the vptr where the first virtual member is declared.
        virtual ~Configuration() {}
    };
}

} // namespace EGG
