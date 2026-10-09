#pragma once

// Game-side m3d extensions (City Folk only; not in Skyward Sword). Source: src/dol/game/d_m3d.cpp
// (.text 8000AD84..8000BCA8). See notes/d_m3d.txt.
// Class names from the RTTI: m3d::mdlEx_c, m3d::mdlEx_c::mdlCallback_c, m3d::mdlEx_c::callback_c,
// m3d::anmChrPart_c. Member and function names are inferred from behaviour.
//
// mdlEx_c is a model that can play several character animations at once, each on its own part of
// the skeleton: every node has a slot index (0 = the model's main animation set with setAnm(),
// 1..n = animations attached with setPartAnm()), and a calcRatio_c per slot blends the change.
// The npc actor (d_a_npc) uses it with anmChrPart_c animations for e.g. the head or the arms.
#include <game/mLib/m_3d.hpp>

namespace m3d {
    class mdlEx_c : public smdl_c {
    public:
        /// The user hook called from the calcWorld callbacks. All inline: the npc actor's
        /// dAcNpc_c::mdlCallback_c derives from it (the vtable is a weak copy in d_a_npc; the three
        /// empty timing functions are kept in d_m3d).
        class callback_c {
        public:
            virtual ~callback_c() {}
            virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {}
            virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl) {}
            virtual void timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl) {}
        };

        class mdlCallback_c : public nw4r::g3d::ICalcWorldCallback {
        public:
            mdlCallback_c();
            virtual ~mdlCallback_c();

            virtual void ExecCallbackA(nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw);
            virtual void ExecCallbackB(nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw);
            virtual void ExecCallbackC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl, nw4r::g3d::FuncObjCalcWorld *cw);

            bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong anmNum, size_t *pSize);
            void setAnm(int idx, anmChr_c *anm, float blendFrame);
            void calcBlend();
            void setPartAnm(int idx, anmChr_c *anm, float blendFrame);
            void removePartAnm(int idx, float blendFrame);
            void playAnm();
            calcRatio_c *getCalcRatio(ulong nodeId);
            void getAnmResult(nw4r::g3d::ChrAnmResult *anmRes, ulong nodeId);
            void changeNodeAnmIdx(u8 from, u8 to);

            ulong getNodeAnmIdx(ulong nodeId) const { return mpNodeAnmIdx[nodeId] & 0x7F; }

            ulong mNodeCount; ///< 0x04
            ulong mAnmNum; ///< 0x08 number of animation slots
            nw4r::g3d::ChrAnmResult *mpNodeResults; ///< 0x0C last result per node (flags 0 = none yet)
            anmChr_c **mpAnm; ///< 0x10 animation per slot
            calcRatio_c *mpCalcRatio; ///< 0x14 blend per slot
            u8 *mpNodeAnmIdx; ///< 0x18 slot per node; bit 0x80 = the part animation is blending out
            callback_c *mpCallback; ///< 0x1C
        };

        mdlEx_c();
        virtual ~mdlEx_c();

        virtual void setAnm(banm_c &anm);
        virtual void play();
        virtual bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *pSize);
        virtual bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong anmNum, ulong bufferOption, int viewCount, size_t *pSize);

        void calcBlend();
        void setAnm(banm_c &anm, float blendFrame);
        bool setPartAnm(int idx, anmChr_c *anm, float blendFrame);
        void setPartNode(u8 idx, const char *nodeName);
        void setPartNode(u8 idx, ulong nodeId);
        void setNodeAnmIdx(u8 idx, ulong nodeId);
        void removePartAnm(int idx, float blendFrame);
        void setCallback(callback_c *callback);

    private:
        mdlCallback_c mCallback; ///< 0x0C
    };

    /// A character animation played on a part of an mdlEx_c (applied with setAnmAfter, so it does
    /// not replace the model's main animation). Known from d_a_npc's dAcNpc_c::chrPartAnm_c, which
    /// derives from it; its vtable and dtor are weak copies in d_a_npc. No data members of its own.
    class anmChrPart_c : public anmChr_c {
    public:
        virtual ~anmChrPart_c() {}

        void setAnm(bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, playMode_e playMode);
    };
}
