#ifndef GAME_D_SHADOW_HPP
#define GAME_D_SHADOW_HPP
#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_color.hpp>
#include <revolution/GX.h>

// RTTI names are original; other names are inferred from behavior.
namespace dShadow {
enum { ROUND_CAPACITY = 68, TEXTURE_CAPACITY = 256, TEXTURE_COUNT = 9, VERTEX_CAPACITY = 32 };
enum Flags { DRAW_OPA = 1, DRAW_QUAD = 2, NO_DEPTH_TEST = 4, NO_HEIGHT_SCALE = 8 };
enum Texture { CIRCLE, DEFAULT, TREE_A, TREE_B, TREE_C, TREE_A_CUT, TREE_B_CUT, TREE_C_CUT, STONE };
class Round_c {
public:
    Round_c() {} // 801C8A9C
    ~Round_c() {} // 801C8AA0
    /* 0x00 */ mVec3_c mPos;
    /* 0x0C */ f32 mSizeX;
    /* 0x10 */ f32 mSizeZ;
    /* 0x14 */ u32 mFlags;
    /* 0x18 */ mAng mAngle;
    /* 0x1A */ u8 mAlpha;
}; // size 0x1C
class Texture_c {
public:
    Texture_c() {} // 801C8AE0
    ~Texture_c() {} // 801C8AE4
    /* 0x00 */ int mTexture;
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ f32 mWidth;
    /* 0x14 */ f32 mLength;
    /* 0x18 */ f32 mTexS0;
    /* 0x1C */ f32 mTexS1;
    /* 0x20 */ u8 mAlpha;
}; // size 0x24
struct Projection_s {
    /* 0x00 */ f32 mSizeX;
    /* 0x04 */ f32 mSizeZ;
    /* 0x08 */ nw4r::g3d::ResTex mTexture;
    /* 0x0C */ mColor mColor;
    /* 0x10 */ mAng mAngle;
    /* 0x12 */ bool mDepthTest;
}; // size 0x14
f32 getDistanceFade(const mVec3_c *pos); // 801C728C
f32 getAlpha(const mVec3_c *pos); // 801C72FC
BOOL requestRound(const mVec3_c *pos, u32 flags, const mAng *angle,
                  f32 sizeX, f32 scale, f32 alphaScale, f32 sizeZ, f32 heightScale); // 801C8518
BOOL requestTexture(const mVec3_c *pos, int texture, f32 width, f32 length,
                    f32 texS0, f32 texS1); // 801C8628
f32 getSunTime(); // 801C865C
// Debug hooks; both are constant FALSE and have no side effects.
BOOL fn_801C8664(); // 801C8664
BOOL fn_801C866C(); // 801C866C
}
class dShadow_c : public fBase_c, public m3d::proc_c {
public:
    dShadow_c(); // inlined in build, 801C6D54
    static fBase_c *build(); // 801C6D54
    virtual int create(); // 801C81D8
    virtual int execute(); // 801C8378
    virtual int draw(); // 801C846C
    virtual int doDelete(); // 801C84B8
    virtual ~dShadow_c() {} // 801C8990
    virtual void drawOpa() { drawPass(1); } // 801C8B64
    virtual void drawXlu() { drawPass(0); } // 801C8B6C
    BOOL addRound(const mVec3_c *pos, u32 flags, mAng angle,
                  f32 sizeX, f32 scale, f32 alphaScale, f32 sizeZ, f32 heightScale); // 801C7374
    BOOL addTexture(const mVec3_c *pos, int texture, f32 width, f32 length,
                    f32 texS0, f32 texS1); // 801C7528
    void drawPass(u32 pass); // 801C7638
    void drawQuad(const mVec3_c *pos, f32 size); // 801C7A14
    void drawTexture(const dShadow::Texture_c *shadow, mColor color); // 801C7AFC
    void updateHeight(); // 801C8674
    /* 0x006C */ dShadow::Round_c mRounds[dShadow::ROUND_CAPACITY];
    /* 0x07DC */ u32 mRoundCount;
    /* 0x07E0 */ dShadow::Texture_c mTextureShadows[dShadow::TEXTURE_CAPACITY];
    /* 0x2BE0 */ u32 mTextureCount;
    /* 0x2BE4 */ f32 mShear;
    /* 0x2BE8 */ f32 mHeight;
    /* 0x2BEC */ bool mRaiseWater;
    /* 0x2BED */ u8 _2BED[3];
    /* 0x2BF0 */ EGG::FrmHeap *mpProcHeap;
    /* 0x2BF4 */ mAllocator_c mAllocator;
    /* 0x2C10 */ bool mReady;
    /* 0x2C11 */ u8 _2C11[3];
    /* 0x2C14 */ nw4r::g3d::ResTex mTextures[dShadow::TEXTURE_COUNT];
}; // size 0x2C38
#endif
