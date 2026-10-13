#ifndef GAME_D_SHADOW_INTERNAL_HPP
#define GAME_D_SHADOW_INTERNAL_HPP
#include <game/game/d_shadow.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/mLib/m_mtx.hpp>

class dShadow_hostIO_c {
public:
    /* 0x00 */ f32 mHeight;
    /* 0x04 */ GXColor mColor;
    dShadow_hostIO_c() : mHeight(2.0f) {
        mColor.r = 38; mColor.g = 42; mColor.b = 72;
    }
    virtual ~dShadow_hostIO_c() {} // 801C8950
};
class shadowDtcb_c : public dBGCF::dtcb_c {
public:
    shadowDtcb_c(const mVec3_c &pos, const mMtx_c *view, const dShadow::Projection_s &projection,
                 f32 height, bool raiseWater) {
        mPos = pos; mpView = view; mProjection = projection;
        mHeight = height; mRaiseWater = raiseWater;
    }
    virtual ~shadowDtcb_c() {} // 801C8B24
    virtual void checkWall(dBGCF::wallList_c *) {} // existing weak copy 800752E0
    virtual void checkFloor(dBGCF::floorList_c *floors); // 801C8704
    virtual void checkColumn(dBGCF::columnList_c *) {} // existing weak copy 800752E4
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ const mMtx_c *mpView;
    /* 0x14 */ dShadow::Projection_s mProjection;
    /* 0x28 */ f32 mHeight;
    /* 0x2C */ bool mRaiseWater;
};

namespace dShadow {
struct Bounds_c {
    Bounds_c(f32 front, f32 back, f32 side) : x(front), y(back), z(side) {}
    f32 x, y, z;
};
}
#endif
