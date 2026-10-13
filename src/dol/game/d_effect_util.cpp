// Game effect convenience wrappers and footprints.
// Names other than RTTI class names are inferred from behaviour.
#include <game/game/d_effect.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_ground_attr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_sky.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_color.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <nw4r/ef/ef_resource.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/g3d/g3d_state.h>
#include <revolution/SC.h>

// 80087790
BOOL dEffect::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    dEffect_c effect;
    return effect.createEffect(name, 0, pos, angle, scale);
}

// 80087820
BOOL dEffect_c::createEffect(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    return createEffect(name, 0, pos, angle, scale);
}

// 80087844
BOOL dEffect::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale,
                     void (*callback)(nw4r::ef::Particle *, u32), u32 arg) {
    dEffect_c effect;
    return static_cast<mEf::effect_c *>(&effect)->createEffect(name, 0, pos, angle, scale, callback, arg);
}

// 800878DC
BOOL dEffect_c::createEffect(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale,
                            void (*callback)(nw4r::ef::Particle *, u32), u32 arg) {
    return static_cast<mEf::effect_c *>(this)->createEffect(name, 0, pos, angle, scale, callback, arg);
}

// 80087934
BOOL dLevelEffect_c::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (mHandle.IsValid()) cleanup();
    return createEffect(name, 0, pos, angle, scale);
}

// 800879B4
BOOL dLevelEffect_c::followEffect(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    return follow(pos, angle, scale);
}

// 800879C4
BOOL dLevelEffect_c::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale, int flag) {
    _90 = flag;
    return createEffect(name, 0, pos, angle, scale);
}

// 800879EC
BOOL dOneEmitterEffect_c::emit(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    create(NULL, pos, angle, scale);
    return TRUE;
}

// 80087A30
bool dEffect::create(const char *name, const mMtx_c *mtx) {
    dEffect_c effect;
    return effect.createEffect(name, 0, mtx);
}

// 80087AA0
bool dEffect_c::createEffect(const char *name, const mMtx_c *mtx) {
    return createEffect(name, 0, mtx);
}

// 80087AB8
bool dLevelEffect_c::create(const char *name, const mMtx_c *mtx) {
    if (mHandle.IsValid()) cleanup();
    return createEffect(name, 0, mtx);
}

// 80087B30
bool dLevelEffect_c::followEffect(const mMtx_c *mtx) { return follow(mtx); }

// 80087B40
bool dLevelEffect_c::create(const char *name, const mMtx_c *mtx, int flag) {
    _90 = flag;
    return createEffect(name, 0, mtx);
}

namespace dEffect {
extern dOneEmitterEffect_c l_playerLeft;
extern dOneEmitterEffect_c l_playerRight;
extern dOneEmitterEffect_c l_npcLeft;
extern dOneEmitterEffect_c l_npcRight;
}

// 80087B5C
BOOL dEffect::createFootprint(const mVec3_c *pos, const mAng3_c *angle, int player, int side) {
    int alpha = dGroundAttr::getFootprint(pos);
    if (!(u8)alpha) return FALSE;
    dBGCF::groundChk_c ground(pos, 0, 0, 0);
    mVec3_c normal = ground.mFloor.mNormal;
    mAng3_c rot;
    rot.x = cM::atan2s(normal.z, normal.y);
    rot.y = angle->y;
    rot.z = cM::atan2s(normal.x, normal.y);
    if (player) {
        if (side) return l_playerLeft.emitFootprint(pos, &rot, alpha);
        else return l_playerRight.emitFootprint(pos, &rot, alpha);
    } else {
        if (side) return l_npcLeft.emitFootprint(pos, &rot, alpha);
        else return l_npcRight.emitFootprint(pos, &rot, alpha);
    }
}

