#include <lib/egg/util/eggEffect.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <lib/egg/util/eggExEffect.hpp>
#include <nw4r/ef.h>
#include <math.h>
#include <nw4r/math.h>

namespace EGG {

static inline f32 absf(f32 x) {
    return x > 0.0f ? x : -x;
}

static const nw4r::ef::Handle<nw4r::ef::Effect> cInvalidHandle;

Effect::Effect() : mFlags(0) {
    mName[0] = '\0';
    mGroupID = 0;
    reset();
}

Effect::~Effect() {}

BOOL Effect::create() {
    if (mHandle.IsValid()) {
        if (!mHandle.IsAlive()) {
            mHandle = cInvalidHandle;
        }
    }

    if (!mHandle.IsValid()) {
        reset();
        mHandle = EffectManager::sEffectManager->createEffect(mName, mGroupID);
        return mHandle.IsValid();
    }

    return FALSE;
}

void Effect::fade() {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->RetireEmitterAll();
        mHandle = cInvalidHandle;
    }
}

void Effect::followFade() {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->RetireEmitterAll();
    }
}

void Effect::kill() {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->RetireEmitterAll();
        mHandle.GetPtr()->RetireParticleAll();
        mHandle = cInvalidHandle;
    }
}

void Effect::setDisableCalc(bool disable) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->SetFlagDisableCalc(disable);
    }
}

void Effect::setDisableDraw(bool disable) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->SetFlagDisableDraw(disable);
    }
}

void Effect::setDisableCalcDraw(bool disable) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->SetFlagDisableCalc(disable);
        mHandle.GetPtr()->SetFlagDisableDraw(disable);
    }
}

void Effect::setLife(u16 life) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                int numManager = emitter->GetNumParticleManager();
                for (int j = 0; j < numManager; j++) {
                    ExParticleManager *manager =
                        static_cast<ExParticleManager *>(emitter->GetParticleManager(j));
                    if (manager != NULL) {
                        manager->setLife(life);
                    }
                }
            }
        }
    }
}

void Effect::setEmitRatio(f32 ratio) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mEmitRatio = ratio;
            }
        }
    }
}

void Effect::setEmitInterval(u16 interval) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mEmitInterval = interval;
            }
        }
    }
}

void Effect::setEmitEmitDiv(u16 div) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mEmitEmitDiv = div;
            }
        }
    }
}

void Effect::setPowerRadiationDir(f32 power) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mVelPowerRadiationDir = power;
            }
        }
    }
}

void Effect::setPowerSpecDir(f32 power) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mVelPowerSpecDir = power;
            }
        }
    }
}

void Effect::setPowerSpecDirAdd(f32 power) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL && emitter->mResource != NULL) {
                nw4r::ef::EmitterDesc *desc = emitter->mResource->GetEmitterDesc();
                if (desc != NULL) {
                    emitter->mParameter.mVelPowerSpecDir = desc->velPowerSpecDir + power;
                }
            }
        }
    }
}

void Effect::setSpecDir(const nw4r::math::VEC3 *dir) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                nw4r::math::MTX34 inv;
                nw4r::math::MTX34 mtx;
                emitter->CalcGlobalMtx(&mtx);
                mtx._03 = 0.0f;
                mtx._13 = 0.0f;
                mtx._23 = 0.0f;
                if (nw4r::math::MTX34Inv(&inv, &mtx)) {
                    nw4r::math::VEC3 local;
                    nw4r::math::VEC3Transform(&local, &inv, dir);
                    emitter->mParameter.mVelSpecDir.x = acosf(local.y);
                    emitter->mParameter.mVelSpecDir.y = atan2f(local.x, local.z);
                    emitter->mParameter.mVelSpecDir.z = 0.0f;
                }
            }
        }
    }
}

void Effect::setSpecDirAdd(const nw4r::math::VEC3 *dir) {
    if (mHandle.IsValid()) {
        int numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL && emitter->mResource != NULL) {
                nw4r::ef::EmitterDesc *desc = emitter->mResource->GetEmitterDesc();
                if (desc != NULL) {
                    nw4r::math::MTX34 rot;
                    nw4r::math::MTX34 inv;
                    nw4r::math::MTX34 mtx;
                    nw4r::math::VEC3 vec(0.0f, 1.0f, 0.0f);
                    emitter->CalcGlobalMtx(&mtx);
                    mtx._03 = 0.0f;
                    mtx._13 = 0.0f;
                    mtx._23 = 0.0f;
                    nw4r::math::MTX34RotXYZRad(&rot, desc->velSpecDir.x, desc->velSpecDir.y,
                                               desc->velSpecDir.z);
                    nw4r::math::VEC3Transform(&vec, &rot, &vec);
                    nw4r::math::VEC3Transform(&vec, &mtx, &vec);
                    vec += *dir;
                    nw4r::math::VEC3Normalize(&vec, &vec);
                    if (nw4r::math::MTX34Inv(&inv, &mtx)) {
                        nw4r::math::VEC3 local;
                        nw4r::math::VEC3Transform(&local, &inv, &vec);
                        emitter->mParameter.mVelSpecDir.x = acosf(local.y);
                        emitter->mParameter.mVelSpecDir.y = atan2f(local.x, local.z);
                        emitter->mParameter.mVelSpecDir.z = 0.0f;
                    }
                }
            }
        }
    }
}

void Effect::setVelocity(const nw4r::math::VEC3 *vel) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->SetVelocity(*vel);
    }
}

void Effect::setColor(u8 r, u8 g, u8 b, u8 a, bool recursive) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->GetNumEmitter();
        GXColor color = {r, g, b, a};
        mHandle.GetPtr()->Modifier_SetSimpleLightType(1, recursive);
        mHandle.GetPtr()->Modifier_SetSimpleLightAmbient(color, recursive);
    }
}

void Effect::setRegisterColor(const _GXColor &color0, const _GXColor &color1, u8 index) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                int numManager = emitter->GetNumParticleManager();
                for (int j = 0; j < numManager; j++) {
                    ExParticleManager *manager =
                        static_cast<ExParticleManager *>(emitter->GetParticleManager(j));
                    if (manager != NULL) {
                        manager->setRegisterColor(color0, color1, index);
                    }
                }
            }
        }
    }
}

void Effect::setRegisterAlpha(u8 alpha0, u8 alpha1, u8 index) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                int numManager = emitter->GetNumParticleManager();
                for (int j = 0; j < numManager; j++) {
                    ExParticleManager *manager =
                        static_cast<ExParticleManager *>(emitter->GetParticleManager(j));
                    if (manager != NULL) {
                        manager->setRegisterAlpha(alpha0, alpha1, index);
                    }
                }
            }
        }
    }
}

void Effect::setDefaultParticleSize(const nw4r::math::VEC2 *size) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                int numManager = emitter->GetNumParticleManager();
                for (int j = 0; j < numManager; j++) {
                    ExParticleManager *manager =
                        static_cast<ExParticleManager *>(emitter->GetParticleManager(j));
                    if (manager != NULL) {
                        manager->setScale(const_cast<nw4r::math::VEC2 *>(size));
                    }
                }
            }
        }
    }
}

void Effect::setParticleScale(const nw4r::math::VEC2 *scale, bool recursive) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->Modifier_SetScale(*const_cast<nw4r::math::VEC2 *>(scale), recursive);
    }
}

void Effect::setDefaultParticleRotate(const nw4r::math::VEC3 *rotate) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                int numManager = emitter->GetNumParticleManager();
                for (int j = 0; j < numManager; j++) {
                    ExParticleManager *manager =
                        static_cast<ExParticleManager *>(emitter->GetParticleManager(j));
                    if (manager != NULL) {
                        manager->setRotate(rotate);
                    }
                }
            }
        }
    }
}

void Effect::setParticleRotate(const nw4r::math::VEC3 *rotate, bool recursive) {
    if (mHandle.IsValid()) {
        mHandle.GetPtr()->Modifier_SetRotate(*rotate, recursive);
    }
}

void Effect::setEmitterSize(const nw4r::math::VEC3 *size, int recursive) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                nw4r::ef::EmitterParameter &param = emitter->mParameter;
                u32 type = param.mEmitFlags & 0xFF;
                if (type == nw4r::ef::EMITFORMTYPE_DISC) {
                    param.mParams[0] = size->x;
                    param.mParams[4] = size->z;
                    if (recursive != 0) {
                        param.mEmitFlags &= ~nw4r::ef::EmitterDesc::EMIT_FLAG_XYZ_SAME_SIZE;
                    }
                } else if (type == nw4r::ef::EMITFORMTYPE_LINE) {
                    param.mParams[0] = size->x;
                } else if (type == nw4r::ef::EMITFORMTYPE_CUBE) {
                    param.mParams[0] = size->x;
                    param.mParams[1] = size->y;
                    param.mParams[2] = size->z;
                } else if (type == nw4r::ef::EMITFORMTYPE_CYLINDER) {
                    param.mParams[0] = size->x;
                    param.mParams[4] = size->y;
                    param.mParams[5] = size->z;
                    if (recursive != 0) {
                        param.mEmitFlags &= ~nw4r::ef::EmitterDesc::EMIT_FLAG_XYZ_SAME_SIZE;
                    }
                } else if (type == nw4r::ef::EMITFORMTYPE_SPHERE) {
                    param.mParams[0] = size->x;
                    param.mParams[4] = size->y;
                    param.mParams[5] = size->z;
                    if (recursive != 0) {
                        param.mEmitFlags &= ~nw4r::ef::EmitterDesc::EMIT_FLAG_XYZ_SAME_SIZE;
                    }
                } else if (type == nw4r::ef::EMITFORMTYPE_POINT) {
                } else if (type == nw4r::ef::EMITFORMTYPE_TORUS) {
                    param.mParams[0] = size->x;
                    param.mParams[4] = size->y;
                    param.mParams[5] = size->z;
                    if (recursive != 0) {
                        param.mEmitFlags &= ~nw4r::ef::EmitterDesc::EMIT_FLAG_XYZ_SAME_SIZE;
                    }
                }
            }
        }
    }
}

void Effect::setLocalScale(const nw4r::math::VEC3 *scale) {
    if (mHandle.IsValid()) {
        u16 numEmitter = mHandle.GetPtr()->GetNumEmitter();
        for (int i = 0; i < numEmitter; i++) {
            nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(i);
            if (emitter != NULL) {
                emitter->mParameter.mScale = *scale;
                emitter->SetMtxDirty();
            }
        }
    }
}

void Effect::setDynamicsScale(const nw4r::math::VEC3 *scale, const nw4r::math::VEC2 *ptclScale) {
    nw4r::math::VEC2 ptcl;
    ptcl.x = 1.0f;
    ptcl.y = 1.0f;
    if (ptclScale != NULL) {
        ptcl.x = ptclScale->x;
        ptcl.y = ptclScale->y;
    } else {
        if (absf(scale->x) < NW4R_MATH_FLT_EPSILON) {
            ptcl.x = 0.0f;
        } else {
            ptcl.x /= scale->x;
        }
        if (absf(scale->y) < NW4R_MATH_FLT_EPSILON) {
            ptcl.y = 0.0f;
        } else {
            ptcl.y /= scale->y;
        }
    }
    setScale(scale);
    setParticleScale(&ptcl, false);
}

void Effect::setScale(f32 scale) {
    mFlags |= FLAG_SCALE;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
}

void Effect::setScale(const nw4r::math::VEC3 *scale) {
    mFlags |= FLAG_SCALE;
    mScale = *scale;
}

void Effect::setPos(const nw4r::math::VEC3 *pos) {
    mFlags |= FLAG_POS;
    mPos = *pos;
}

void Effect::setMtx(const nw4r::math::MTX34 *mtx) {
    mFlags |= FLAG_MTX;
    nw4r::math::MTX34Copy(&mMtx, mtx);
}

void Effect::setPtclAnim(int index, bool play) {
    bool stop = !play;
    nw4r::ef::Emitter *emitter;
    if (mHandle.IsValid() && mHandle.GetPtr()->GetNumEmitter() != 0 &&
        (emitter = mHandle.GetPtr()->GetEmitter(0)) != NULL && emitter->mResource != NULL) {
        if (index < 0) {
            for (int i = 0; i < emitter->mResource->NumPtclTrack(); i++) {
                nw4r::ef::ResAnimCurve curve(emitter->mResource->GetPtclTrack(i));
                curve.SetStop(stop);
            }
        } else {
            nw4r::ef::ResAnimCurve curve(emitter->mResource->GetPtclTrack(index));
            curve.SetStop(stop);
        }
    } else {
        nw4r::ef::ResEmitter resEmitter =
            nw4r::ef::Resource::GetInstance()->FindEmitter(mName, NULL);
        if (index < 0) {
            for (int i = 0; i < resEmitter.ptr()->NumPtclTrack(); i++) {
                nw4r::ef::ResAnimCurve curve = resEmitter.GetPtclTrack(i);
                curve.SetStop(stop);
            }
        } else {
            nw4r::ef::ResAnimCurve curve = resEmitter.GetPtclTrack(index);
            curve.SetStop(stop);
        }
    }
}

void Effect::update() {
    if (mHandle.IsValid()) {
        if (mHandle.IsAlive()) {
            nw4r::math::MTX34 mtx;
            if (mFlags & FLAG_MTX) {
                nw4r::math::MTX34Copy(&mtx, &mMtx);
            } else {
                nw4r::math::MTX34Identity(&mtx);
            }
            if (mFlags & FLAG_SCALE) {
                mtx._00 *= mScale.x;
                mtx._10 *= mScale.x;
                mtx._20 *= mScale.x;
                mtx._01 *= mScale.y;
                mtx._11 *= mScale.y;
                mtx._21 *= mScale.y;
                mtx._02 *= mScale.z;
                mtx._12 *= mScale.z;
                mtx._22 *= mScale.z;
            }
            if (mFlags & FLAG_POS) {
                mtx._03 = mPos.x;
                mtx._13 = mPos.y;
                mtx._23 = mPos.z;
            }
            mHandle.GetPtr()->SetRootMtx(mtx);
        } else {
            mHandle = cInvalidHandle;
        }
    }
}

f32 Effect::getBaseEmitRatio() {
    if (mHandle.IsValid() && mHandle.GetPtr()->GetNumEmitter() != 0) {
        nw4r::ef::Emitter *emitter = mHandle.GetPtr()->GetEmitter(0);
        if (emitter != NULL && emitter->mResource != NULL) {
            nw4r::ef::EmitterDesc *desc = emitter->mResource->GetEmitterDesc();
            if (desc != NULL) {
                return desc->emitEmit;
            }
        }
    }
    nw4r::ef::ResEmitter resEmitter = nw4r::ef::Resource::GetInstance()->FindEmitter(mName, NULL);
    return resEmitter.ptr()->GetEmitterDesc()->emitEmit;
}

nw4r::ef::Emitter *Effect::getRootEmitter() {
    if (mHandle.IsValid()) {
        return mHandle.GetPtr()->GetRootEmitter();
    }
    return NULL;
}

void Effect::reset() {
    mFlags = 0;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    nw4r::math::MTX34Identity(&mMtx);
    mHandle = cInvalidHandle;
}

} // namespace EGG
