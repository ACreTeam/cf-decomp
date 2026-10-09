#pragma once
#include <types.h>
#include <nw4r/ut.h>
#include <nw4r/math.h>
#include <revolution/RFL.h>

class mAllocator_c;

// mFace: Mii (RFL) face models (City Folk only). m_face.cpp is at 0x802B39E4..0x802B3E40 (no source yet).
// All names are @unofficial; offsets/behaviour come from the target code.
namespace mFace {
    /**
     * An object whose RFL data is initialised later: entry() queues it in a global nw4r::ut::List
     * (0x806A5DD0); a per-frame pass (0x802B3B10) calls init() for each queued object, sets mIsReady and
     * unlinks it (0x802B3BC0).
     */
    class obj_c : public nw4r::ut::Link {
    public:
        obj_c() : mIsReady(false) {}

        virtual void init() = 0;

        bool isReady() const { return mIsReady; }

    protected:
        void entry(); // 0x802B3BA8
        void leave(); // 0x802B3BC0

        bool mIsReady; // 0xC
    };

    /// A Mii face model: an RFLCharModel (base at 0x10) and the parameters to build it with.
    class model_c : public obj_c, public RFLCharModel {
    public:
        model_c() : mpBuffer(nullptr) {}
        ~model_c() { remove(); }

        virtual void init(); // 0x802B3C40, RFLInitCharModel with the stored parameters

        bool create(RFLDataSource source, u16 index, RFLResolution resolution, u32 expressionFlag,
                    mAllocator_c *allocator, RFLMiddleDB *middleDB); // 0x802B3C74
        void remove(); // 0x802B3D38, frees mpBuffer through mpAllocator
        void setMtx(const nw4r::math::MTX34 *mtx); // 0x802B3E20, RFLSetMtx

    private:
        RFLDataSource mSource;      // 0x98
        u16 mIndex;                 // 0x9C
        RFLResolution mResolution;  // 0xA0
        u32 mExpressionFlag;        // 0xA4
        RFLMiddleDB *mpMiddleDB;    // 0xA8
        void *mpBuffer;             // 0xAC
        mAllocator_c *mpAllocator;  // 0xB0
    };
}
