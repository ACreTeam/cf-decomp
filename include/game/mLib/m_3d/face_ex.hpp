#pragma once
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_face.hpp>
#include <revolution/RFL.h>

// m3d face procs (City Folk only): a Mii face (mFace::model_c) drawn as an m3d proc.
// face_ex.cpp is at 0x802B363C..0x802B3998. Virtual names past proc_c's are @unofficial.
namespace m3d {
    class bface_c : public proc_c {
    public:
        virtual ~bface_c() {}

        virtual void remove();
        virtual void drawOpa();
        virtual void drawXlu();
        virtual void drawOpaCore() = 0;
        virtual void drawXluCore() = 0;

        bool create(RFLDataSource source, u16 index, RFLResolution resolution, u32 expressionFlag,
                    mAllocator_c *allocator, RFLMiddleDB *middleDB);

    protected:
        mFace::model_c mModel; // 0x08
    };

    class faceEx_c : public bface_c {
    public:
        faceEx_c();
        virtual ~faceEx_c() {}

        virtual leafType_e getType() const { return TYPE_FACE; }
        virtual void drawOpaCore();
        virtual void drawXluCore();
        /// Called right before RFLDrawOpaCore / RFLDrawXluCore (extra GX state in derived classes).
        virtual void setupDrawOpa() {}
        virtual void setupDrawXlu() {}

    protected:
        RFLDrawCoreSetting mDrawSetting; // 0xBC
    };
}
