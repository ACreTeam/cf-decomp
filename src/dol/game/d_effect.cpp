// Game effect classes: curved-world placement and emitter overrides.
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

// 80087064
bool dEffect_c::createEffect(const char *name, ulong group, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (dEffect::isActive() == FALSE) return FALSE;
    mVec3_c curved(*pos);
    mAng3_c rot;
    if (angle == NULL) rot.set(0, 0, 0);
    else rot = *angle;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        rot.x += dWorld::curvePosition(&curved, pos);
    }
    return mEf::effect_c::createEffect(name, group, &curved, &rot, scale);
}

// 80087158
bool dEffect_c::follow(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (dEffect::isActive() == FALSE) return FALSE;
    mVec3_c curved(*pos);
    mAng3_c rot;
    if (angle == NULL) rot.set(0, 0, 0);
    else rot = *angle;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        rot.x += dWorld::curvePosition(&curved, pos);
    }
    return mEf::effect_c::follow(&curved, &rot, scale);
}

// 8008724C
bool dEffect_c::follow(const mMtx_c *mtx) {
    if (dEffect::isActive()) {
        return mEf::effect_c::follow(mtx);
    }
    return FALSE;
}

// 800872A0
bool dLevelEffect_c::follow(const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (dEffect::isActive() == FALSE) return FALSE;
    mVec3_c curved(*pos);
    mAng3_c rot;
    if (angle == NULL) rot.set(0, 0, 0);
    else rot = *angle;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        rot.x += dWorld::curvePosition(&curved, pos);
    }
    return mEf::levelEffect_c::follow(&curved, &rot, scale);
}

// 80087394
bool dLevelEffect_c::createEffect(const char *name, ulong group, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (dEffect::isActive() == FALSE) return FALSE;
    mVec3_c curved(*pos);
    mAng3_c rot;
    if (angle == NULL) rot.set(0, 0, 0);
    else rot = *angle;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        rot.x += dWorld::curvePosition(&curved, pos);
    }
    mHandle.IsValid();
    return mEf::levelEffect_c::createEffect(name, group, &curved, &rot, scale);
}

// 80087490
BOOL dOneEmitterEffect_c::create(const char *name, const mVec3_c *pos, const mAng3_c *angle, const mVec3_c *scale) {
    if (dEffect::isActive() == FALSE) return FALSE;
    mVec3_c curved(*pos);
    mAng3_c rot;
    if (angle == NULL) rot.set(0, 0, 0);
    else rot = *angle;
    if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
        rot.x += dWorld::curvePosition(&curved, pos);
    }
    return mEf::oneEmitter_c::create(name, &curved, &rot, scale);
}

// 8008757C
bool dEffect_c::createEffect(const char *name, ulong group, const mMtx_c *mtx) {
    if (dEffect::isActive()) {
        return mEf::effect_c::createEffect(name, group, mtx);
    }
    return FALSE;
}

// 800875F0
bool dLevelEffect_c::follow(const mMtx_c *mtx) {
    if (dEffect::isActive()) {
        return mEf::levelEffect_c::follow(mtx);
    }
    return FALSE;
}

// 80087644
bool dLevelEffect_c::createEffect(const char *name, ulong group, const mMtx_c *mtx) {
    if (dEffect::isActive() == FALSE) return FALSE;
    return mEf::levelEffect_c::createEffect(name, group, mtx);
}

// 800876B8
BOOL dOneEmitterEffect_c::create(const char *name, const mMtx_c *mtx) {
    if (dEffect::isActive()) {
        return mEf::oneEmitter_c::create(name, mtx);
    }
    return FALSE;
}

// 8008771C
void dLevelEffect_c::fade() {
    if (_90) mEf::levelEffect_c::kill();
    else mEf::levelEffect_c::fade();
}

// 80087730
void dOneEmitterEffect_c::create(const char *name) { mEf::oneEmitter_c::create(name); }

// 80087734: inline dOneEmitterEffect_c destructor (header).
