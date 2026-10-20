#pragma once
#include <types.h>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/math.h>
#include <egg/math/eggMatrix.h>

/// @brief A 3x4 matrix.
/// @ingroup mlib
class mMtx_c : public EGG::Matrix34f {
public:
    /// @brief Constructs an empty matrix.
    mMtx_c() {}

    /// @brief Constructs a matrix from an MTX34.
    mMtx_c(const nw4r::math::MTX34 &mtx) { *this = mtx; }

    /// @brief Constructs a matrix with the given components.
    mMtx_c(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23);

    /// @brief Mtx cast operator.
    operator Mtx*() { return &mtx; }

    /// @brief Const Mtx cast operator.
    operator const Mtx*() const { return &mtx; }

    // Keep mMtx_c copy assignment implicit so it uses Matrix34f::copyFrom.
    mMtx_c &operator=(const nw4r::math::MTX34 &other) {
        for (int i = 0; i < 12; i++) {
            f32 v = other.a[i];
            a[i] = v;
        }
        return *this;
    }

    void XrotS(mAng angle); ///< Generates a rotation matrix for the X axis with the given angle.
    void XrotM(mAng angle); ///< Rotates the matrix on the X axis by the given angle.
    void YrotS(mAng angle); ///< Generates a rotation matrix for the Y axis with the given angle.
    void YrotM(mAng angle); ///< Rotates the matrix on the Y axis by the given angle.
    void ZrotS(mAng angle); ///< Generates a rotation matrix for the Z axis with the given angle.
    void ZrotM(mAng angle); ///< Rotates the matrix on the Z axis by the given angle.

    void ZXYrotM(mAng xRot, mAng yRot, mAng zRot); ///< Rotates the matrix on the Y, X and Z axes by the given angles.
    void XYZrotM(mAng xRot, mAng yRot, mAng zRot); ///< Rotates the matrix on the Z, Y and X axes by the given angles.

    void toRot(mAng3_c &out) const; ///< Extracts the rotation vector from the matrix.
    void multVecZero(nw4r::math::VEC3 &out) const; ///< Extracts the translation vector from the matrix.
    void zero(); ///< Zeroes out the matrix.

    mVec3_c getTranslation() const {
        float x = m[0][3];
        float y = m[1][3];
        float z = m[2][3];
        return mVec3_c(x, y, z);
    }

    static mMtx_c createTrans(const mVec3_c &v) { mMtx_c mtx; mtx.trans(v); return mtx; }
    static mMtx_c createTrans(float x, float y, float z) { mMtx_c mtx; PSMTXTrans(mtx, x, y, z); return mtx; }
    static mMtx_c createScale(const mVec3_c &v) { mMtx_c mtx; PSMTXScale(mtx, v.x, v.y, v.z); return mtx; }
    static mMtx_c createScale(float x, float y, float z) { mMtx_c mtx; PSMTXScale(mtx, x, y, z); return mtx; }

    mMtx_c &concat(const mMtx_c &other) { PSMTXConcat(*this, other, *this); return *this; }
    mMtx_c &trans(const mVec3_c &v) { PSMTXTrans(*this, v.x, v.y, v.z); return *this; }
    mMtx_c &trans(float x, float y, float z) { PSMTXTrans(*this, x, y, z); return *this; }

    /// @brief Multiplies the matrix by a scale matrix (this = this * scale).
    void scaleM(float x, float y, float z) { mMtx_c m; PSMTXScale(m, x, y, z); PSMTXConcat(*this, m, *this); }
    /// @brief Multiplies the matrix by a translation matrix (this = this * trans).
    void transM(float x, float y, float z) { mMtx_c m; PSMTXTrans(m, x, y, z); PSMTXConcat(*this, m, *this); }
    mMtx_c &ZXYrotM(const mAng3_c &ang) { ZXYrotM(ang.x, ang.y, ang.z); return *this; }

    float transX() const { return getTranslation().x; }
    float transY() const { return getTranslation().y; }
    float transZ() const { return getTranslation().z; }

    static mMtx_c Identity; ///< The identity matrix.
};

STATIC_ASSERT(sizeof(EGG::Matrix34f) == sizeof(Mtx));
STATIC_ASSERT(sizeof(mMtx_c) == sizeof(nw4r::math::MTX34));
