#pragma once

// The m3d model / animation classes. In the original they were defined in ONE file: their weak inline
// functions share one weak .text section in every object, e.g. the fish REL's
// [anmChr_c dtor][bmdl_c::getType][anmChr_c::getType] and d_a_cockroachNP's interleaved anmChr_c /
// anmVis_c dtors and getTypes (see notes/mwcc_ghidra.md). The class order is the definition order the
// fish REL's weak RTTI order requires. The old per-class headers in m_3d/ just include this file.
#include <nw4r/g3d.h>
#include <nw4r/math.h>
#include <game/mLib/m_allocator.hpp>
#include <lib/egg/core/eggFrmHeap.h>

// ---- scn_leaf (was m_3d/scn_leaf.hpp)
namespace m3d {
    class scnLeaf_c {
    public:
        /// @unofficial
        enum leafType_e {
            TYPE_BMDL,
            TYPE_FACE, ///< m3d::faceEx_c (face_ex.cpp)
            TYPE_PROC
        };

        scnLeaf_c();
        virtual ~scnLeaf_c();

        virtual leafType_e getType() const = 0;
        virtual void remove();
        virtual void entry();

        void setOption(ulong option, ulong value);
        bool getOption(ulong option, ulong *value) const;
        void setScale(float x, float y, float z);
        void setScale(const nw4r::math::VEC3 &scale);
        void setLocalMtx(const nw4r::math::MTX34 *mtx);
        void getLocalMtx(nw4r::math::MTX34 *mtx) const;
        void getViewMtx(nw4r::math::MTX34 *mtx) const;
        void calc(bool keepEnabledAfter);
        void calcVtx(bool keepEnabledAfter);
        void setPriorityDraw(int prioOpa, int prioXlu);

        nw4r::g3d::ScnObj *getScn() const { return mpScn; }

    protected:
        nw4r::g3d::ScnLeaf *mpScn;
    };
}

// ---- calc_ratio (was m_3d/calc_ratio.hpp)
namespace m3d {
    /**
     * @brief Class to smoothly blend between two values.
     *
     * This interpolation is done using a custom interpolation.
     * The mWeight follows an ease-in/ease-out curve and the difference
     * between the previous and current value of this weight is used
     * to compute the actual interpolation.
     */
    class calcRatio_c {
    public:
        calcRatio_c(); ///< Constructs a ratio calculator.
        virtual ~calcRatio_c() {} ///< Destroys the ratio calculator.

        bool isEnd() const; ///< Returns whether the blend is complete.
        void calc(); ///< Advances the blend by one time step.
        void set(float duration); ///< Starts a blend with a given duration.
        void remove(); ///< Cancels and resets the blend.
        void reset(); ///< Resets the blend to its initial state.
        void offUpdate(); ///< Pauses the blend.

        float getScaleFrom() { return mScaleFrom; }
        float getScaleTo() { return mScaleTo; }
        float getSlerpParam() { return mInterpolateT; }

        bool isActive() const { return mIsActive; }
        bool isBlending() const { return mIsBlending; }

    private:
        float mWeight; ///< Current weight of the blend.
        float mT; ///< The current time of the blend, between 0 and 1.
        float mTimeStep; ///< How much to advance mT by each iteration.

        float mScaleFrom; ///< How much to multiply the old value by (between 0 and 1).
        float mScaleTo; ///< How much to multiply the new value by (between 0 and 1).
        float mInterpolateT; ///< If using an interpolation function, use this as the @p t parameter.

        bool mIsActive; ///< Whether the blend is paused.
        bool mIsBlending; ///< Whether the blend is active.
    };
}

// ---- bmdl (was m_3d/bmdl.hpp)
namespace m3d {
    class banm_c;
    class bmdl_c : public scnLeaf_c {
    public:
        bmdl_c() : mpAnm(nullptr) {}
        virtual ~bmdl_c();

        virtual scnLeaf_c::leafType_e getType() const { return TYPE_BMDL; }
        virtual void remove();
        virtual void setAnm(m3d::banm_c &anm);
        virtual void play();

        void setDrawMode(nw4r::g3d::ResMdlDrawMode mode);
        bool getNodeWorldMtx(ulong idx, nw4r::math::MTX34 *mtx) const;
        bool getNodeWorldMtxMultVecZero(ulong idx, nw4r::math::VEC3 &vec) const;
        bool getNodeWorldMtxMultVec(ulong idx, const nw4r::math::VEC3 &in, nw4r::math::VEC3 &out) const;

        nw4r::g3d::ResMdl getResMdl() const;
        nw4r::g3d::ResMat getResMat(size_t idx) const;
        void removeAnm(nw4r::g3d::ScnMdlSimple::AnmObjType objType);

        void setTevColor(ulong idx, _GXTevRegID regID, _GXColor color, bool markDirty);
        void setTevColorAll(_GXTevRegID regID, _GXColor color, bool markDirty);
        void setTevKColor(ulong idx, _GXTevKColorID colID, _GXColor color, bool markDirty);
        void setTevKColorAll(_GXTevKColorID colID, _GXColor color, bool markDirty);
        void setLightSetIdxAll(int idx); ///< @unofficial
        void setEnvMapRefAll(int camRef, int lightRef); ///< @unofficial

    protected:
        banm_c *mpAnm;
    };
}

// ---- smdl (was m_3d/smdl.hpp)
namespace m3d {
    class smdl_c : public bmdl_c {
    public:
        smdl_c();
        virtual ~smdl_c();

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *objSize);

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount) {
            return create(resMdl, allocator, bufferOption, viewCount, nullptr);
        }

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption) {
            return create(resMdl, allocator, bufferOption, 1);
        }
    };
}

// ---- mdl (was m_3d/mdl.hpp)
namespace m3d {
    class mdl_c : public smdl_c {
    public:
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

            bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, size_t *pSize);
            void remove();
            void setBlendFrame(float blendFrame);
            void calcBlend();

            calcRatio_c mCalcRatio;
            int mNodeCount;
            nw4r::g3d::ChrAnmResult *mpNodeResults;
            callback_c *mpCallback;
            mAllocator_c *mpAllocator;
        };

    public:
        mdl_c();
        virtual ~mdl_c();

        virtual void remove();

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount, size_t *pSize);
        void setCallback(callback_c *callback);

        void setAnm(m3d::banm_c &anm);
        void setAnm(m3d::banm_c &anm, float blendFrame);

        void play();

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption, int viewCount) {
            return create(resMdl, allocator, bufferOption, viewCount, nullptr);
        }

        bool create(nw4r::g3d::ResMdl resMdl, mAllocator_c *allocator, ulong bufferOption) {
            return create(resMdl, allocator, bufferOption, 1, nullptr);
        }

    private:
        mdlCallback_c mCallback;
    };
}

// ---- proc (was m_3d/proc.hpp)
namespace m3d {
    class proc_c : public scnLeaf_c {
    public:
        virtual ~proc_c() {}
        virtual scnLeaf_c::leafType_e getType() const { return scnLeaf_c::TYPE_PROC; };

        virtual void drawOpa() {}
        virtual void drawXlu() {}

        bool create(mAllocator_c *allocator, size_t *size);
    };

    void proc_c_drawProc(nw4r::g3d::ScnProc *proc, bool drawOpa);
}

// ---- banm (was m_3d/banm.hpp)
namespace m3d {
    enum playMode_e {
        FORWARD_LOOP, ///< Play the animation forward in a loop.
        FORWARD_ONCE, ///< Play the animation forward once.
        REVERSE_LOOP, ///< Play the animation in reverse in a loop.
        REVERSE_ONCE, ///< Play the animation in reverse once.
        PLAYMODE_INHERIT, ///< Use the play mode of the parent.

        MASK_LOOP = 1, ///< Mask for loop play mode.
        MASK_FORWARD = 2, ///< Mask for forward play mode.
    };

    class banm_c {
    public:
        /// @unofficial
        enum anmType_e {
            TYPE_ANM_CHR,
            TYPE_ANM_VIS,
            TYPE_ANM_MAT_CLR,
            TYPE_ANM_TEX_PAT,
            TYPE_ANM_TEX_SRT,
            TYPE_ANM_SHP
        };

        banm_c() : mpObj(nullptr), mpHeap(nullptr) {}
        virtual ~banm_c();
        virtual anmType_e getType() const = 0;
        virtual void remove();
        virtual void play() {}

        bool createAllocator(mAllocator_c *allocator, size_t *size);
        bool IsBound() const;
        float getFrame() const;
        void setFrameOnly(float frame);
        float getRate() const;
        void setRate(float rate);

        nw4r::g3d::AnmObj *getObj() { return mpObj; }

    protected:
        nw4r::g3d::AnmObj *mpObj;
        EGG::FrmHeap *mpHeap;
        mAllocator_c mAllocator;
    };
}

// ---- fanm (was m_3d/fanm.hpp)
namespace m3d {

    /// @brief Animation object.
    class fanm_c : public banm_c {
    public:
        fanm_c(); ///< Constructs an animation object.
        virtual ~fanm_c(); ///< Destroys the animation object.

        /// @brief Updates the animation.
        /// Call this function every frame to update the animation.
        virtual void play();

        /**
         * @brief Starts the animation with the given parameters.
         *
         * @param duration The number of frames in the animation.
         * @param playMode The play mode of the animation.
         * @param updateRate The speed of the animation.
         * @param startFrame The starting frame of the animation. Set to -1 to start from the beginning.
         */
        void set(float duration, m3d::playMode_e playMode, float updateRate, float startFrame);

        float getFrame() const;

        /// @brief Jumps to the specified frame in the animation.
        /// @param frame The frame to jump to.
        void setFrame(float frame);

        float getRate() const;
        void setRate(float rate);

        /// @brief Checks whether the animation is stopped.
        bool isStop() const;

        /// @brief Checks whether the animation has reached the specified frame.
        /// @param frame The frame to check.
        bool checkFrame(float frame) const;

        float mFrameMax; ///< The last frame number of the animation.
        float mFrameStart; ///< The first frame number of the animation.
        float mCurrFrame; ///< The frame the animation is currently on.
        u8 mPlayMode; ///< The play mode of the animation.
    };
}

// ---- anm_chr (was m_3d/anm_chr.hpp)
namespace m3d {
    class anmChr_c : public fanm_c {
    public:
        virtual ~anmChr_c() {}
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_CHR; };

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr anmChr, mAllocator_c *allocator, size_t *objSize);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, m3d::playMode_e playMode);
        void setAnmAfter(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmChr anmChr, m3d::playMode_e playMode);
        void setFrmCtrlDefault(nw4r::g3d::ResAnmChr &anmChr, m3d::playMode_e playMode);

        bool create2(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr anmChr, mAllocator_c *allocator) {
            return create(mdl, anmChr, allocator, nullptr);
        }

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmChr anmChr, mAllocator_c *allocator) {
            return create2(mdl, anmChr, allocator);
        }
    };
}

// ---- anm_chr_blend (was m_3d/anm_chr_blend.hpp)
namespace m3d {
    class anmChrBlend_c : public banm_c {
    public:
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_CHR; };

        bool create(nw4r::g3d::ResMdl mdl, int count, mAllocator_c *allocator, size_t *objSize);
        void attach(int idx, nw4r::g3d::AnmObjChrRes *chrRes, float weight);
        void attach(int idx, m3d::anmChr_c *anmChrRes, float weight);
        void detach(int idx);
    };
}

// ---- anm_vis (was m_3d/anm_vis.hpp)
namespace m3d {
    class anmVis_c : public fanm_c {
    public:
        virtual ~anmVis_c() {}
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_VIS; };

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmVis anmVis, mAllocator_c *allocator, size_t *objSize);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmVis anmVis, m3d::playMode_e playMode);
        void setFrmCtrlDefault(nw4r::g3d::ResAnmVis &anmVis, m3d::playMode_e playMode);
    };
}

// ---- anm_shp
namespace m3d {
    class anmShp_c : public fanm_c {
    public:
        virtual ~anmShp_c() {}
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_SHP; };

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmShp anmShp, mAllocator_c *allocator, size_t *objSize);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmShp anmShp, m3d::playMode_e playMode);
        void setFrmCtrlDefault(nw4r::g3d::ResAnmShp &anmShp, m3d::playMode_e playMode);
    };
}

// ---- anm_mat_clr (was m_3d/anm_mat_clr.hpp)
namespace m3d {
    class anmMatClr_c : public banm_c {
    public:
        anmMatClr_c() : mpChildren(nullptr) {}
        virtual ~anmMatClr_c();
        virtual void remove();
        virtual void play();
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_MAT_CLR; };

        static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, long count, bool calcAligned);
        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, mAllocator_c *allocator, size_t *objSize, long count);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmClr anmClr, long idx, m3d::playMode_e playMode);
        void releaseAnm(long idx);
        void play(long idx);
        float getFrame(long idx) const;
        void setFrame(float frame, long idx);
        float getRate(long idx) const;
        void setRate(float rate, long idx);
        bool isStop(long idx) const;
        bool checkFrame(float frame, long idx) const;
        void setPlayMode(m3d::playMode_e playMode, long idx);
        m3d::playMode_e getPlayMode(long idx) const;
        float getFrameMax(long idx) const;
        float getFrameStart(long idx) const;

        class child_c : public fanm_c {
        public:
            virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_MAT_CLR; };
            virtual ~child_c() {}

            static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, bool calcAligned);
            bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmClr anmClr, mAllocator_c *allocator, size_t *objSize);
            void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmClr anmClr, m3d::playMode_e playMode);
            void releaseAnm();
            void setFrmCtrlDefault(nw4r::g3d::ResAnmClr &anmClr, m3d::playMode_e playMode);
        };

        child_c *mpChildren;
    };
}

// ---- anm_tex_pat (was m_3d/anm_tex_pat.hpp)
namespace m3d {
    class anmTexPat_c : public banm_c {
    public:
        anmTexPat_c() : mpChildren(nullptr) {}
        virtual ~anmTexPat_c();
        virtual void remove();
        virtual void play();
        virtual banm_c::anmType_e getType() const { return banm_c::TYPE_ANM_TEX_PAT; };

        static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, long count, bool calcAligned);
        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator, size_t *objSize, long count);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexPat anmTexPat, long idx, m3d::playMode_e playMode);
        void releaseAnm(long idx);
        void play(long idx);
        float getFrame(long idx) const;
        void setFrame(float frame, long idx);
        float getRate(long idx) const;
        void setRate(float rate, long idx);
        bool isStop(long idx) const;
        bool checkFrame(float frame, long idx) const;
        void setPlayMode(m3d::playMode_e playMode, long idx);
        m3d::playMode_e getPlayMode(long idx) const;
        float getFrameMax(long idx) const;
        float getFrameStart(long idx) const;

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator, int count) {
            return create(mdl, anmTexPat, allocator, nullptr, count);
        }

        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator) {
            return create(mdl, anmTexPat, allocator, 1);
        }

        class child_c : public fanm_c {
        public:
            virtual banm_c::anmType_e getType( void ) const { return banm_c::TYPE_ANM_TEX_PAT; };
            virtual ~child_c() {}

            static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, bool calcAligned);
            bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexPat anmTexPat, mAllocator_c *allocator, size_t *objSize);
            void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexPat anmTexPat, m3d::playMode_e playMode);
            void releaseAnm();
            void setFrmCtrlDefault(nw4r::g3d::ResAnmTexPat &anmTexPat, m3d::playMode_e playMode);
        };

        child_c *mpChildren;
    };
}

// ---- anm_tex_srt (was m_3d/anm_tex_srt.hpp)
namespace m3d {
    class anmTexSrt_c : public banm_c {
    public:
        anmTexSrt_c() : mpChildren(nullptr) {}
        virtual ~anmTexSrt_c();
        virtual void remove();
        virtual void play();
        virtual banm_c::anmType_e getType( void ) const { return TYPE_ANM_TEX_SRT; };

        static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, long count, bool calcAligned);
        bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, mAllocator_c *allocator, size_t *objSize, long count);
        void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, long idx, m3d::playMode_e playMode);
        void releaseAnm(long idx);
        void play(long idx);
        float getFrame(long idx) const;
        void setFrame(float frame, long idx);
        float getRate(long idx) const;
        void setRate(float rate, long idx);
        bool isStop(long idx) const;
        bool checkFrame(float frame, long idx) const;
        void setPlayMode(m3d::playMode_e playMode, long idx);
        m3d::playMode_e getPlayMode(long idx) const;
        float getFrameMax(long idx) const;
        float getFrameStart(long idx) const;

        class child_c : public fanm_c {
        public:
            virtual banm_c::anmType_e getType( void ) const { return TYPE_ANM_TEX_SRT; };
            virtual ~child_c() {}

            static size_t heapCost(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, bool calcAligned);
            bool create(nw4r::g3d::ResMdl mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, mAllocator_c *allocator, size_t *objSize);
            void setAnm(m3d::bmdl_c &mdl, nw4r::g3d::ResAnmTexSrt anmTexSrt, m3d::playMode_e playMode);
            void releaseAnm();
            void setFrmCtrlDefault(nw4r::g3d::ResAnmTexSrt &anmTexSrt, m3d::playMode_e playMode);
        };

        child_c *mpChildren;
    };
}

#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_3d/internal.hpp>
#include <game/mLib/m_3d/m_3d_capture.hpp>
