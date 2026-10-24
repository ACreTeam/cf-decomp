#include <nw4r/ef.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>

#include <cstring>
#include <cmath>
#include <nw4r/ef/ef_field.h>
#include <nw4r/ef/ef_postfield.h>

namespace nw4r {
namespace ef {

math::MTX34 ParticleManager::smDrawMtxPMtoEM;
math::MTX34 ParticleManager::smMtxInv;

s32 ParticleManager::smMtxInvId = 0;

ParticleManager::ParticleManager()
    : mActivityList(offsetof(Particle, mActivityLink)) {
}

void ParticleManager::SendClosing() {
    mManagerEM->Closing(this);
}

void ParticleManager::DestroyFunc() {
}

bool ParticleManager::Closing(Particle *pParticle) {
    pParticle->mParticleManager->UnRef();

    mActivityList.ToClosing(pParticle);
    pParticle->mLifeStatus = NW4R_EF_LS_CLOSING;

    return true;
}

int ParticleManager::RetireParticle(Particle *pParticle) {
    if (pParticle->mLifeStatus != NW4R_EF_LS_ACTIVE) {
        return 0;
    }

    mActivityList.ToWait(pParticle);
    pParticle->Destroy();
    return 1;
}

int ParticleManager::RetireParticleAll() {
    int num = 0;

    Particle *pIt;
    Particle *pNext;

    for (pIt = static_cast<Particle *>(mActivityList.mActiveList.headObject);
         pIt != NULL; pIt = pNext) {

        pNext = static_cast<Particle *>(
            NW4R_UT_LIST_GET_LINK(mActivityList.mActiveList, pIt)->nextObject);

        if (pIt->mLifeStatus == NW4R_EF_LS_ACTIVE) {
            num += RetireParticle(pIt);
        }
    }

    return num;
}

bool ParticleManager::Initialize(Emitter *pParent, EmitterResource *pResource) {
    ReferencedObject::Initialize();

    mActivityList.Initialize();

    mModifier.mScale.x = 1.0f;
    mModifier.mScale.y = 1.0f;

    mModifier.mRotate.x = 0.0f;
    mModifier.mRotate.y = 0.0f;
    mModifier.mRotate.z = 0.0f;

    mManagerEM = pParent;
    pParent->Ref();

    mResource = pResource;
    mFlag = 0;
    SetMtxDirty();

    EmitterDesc *pDesc = mResource->GetEmitterDesc();

    mDrawStrategy =
        mManagerEM->mManagerEF->mManagerES->mDrawStrategyBuilder->Create(
            pDesc->drawSetting.ptcltype);

    mLastCalced = NULL;

    Modifier_SetSimpleLightParameter(pDesc->drawSetting);

    return true;
}

ParticleManager::~ParticleManager() {
}

Particle *
ParticleManager::CreateParticle(u16 life, math::VEC3 pos, math::VEC3 vel,
                                const math::MTX34 *pSpace, f32 momentum,
                                const EmitterInheritSetting *pSetting,
                                Particle *pReferencePtcl, u16 calcRemain) {

    Particle *pParticle =
        mManagerEM->mManagerEF->mManagerES->GetMemoryManager()->AllocParticle();

    if (pParticle == NULL) {
        return NULL;
    }

    if (!pParticle->Initialize(life, pos, vel, this, pSpace, momentum, pSetting,
                               pReferencePtcl)) {
        return NULL;
    }

    pParticle->mCalcRemain += calcRemain;
    mActivityList.ToActive(pParticle);
    pParticle->mLifeStatus = NW4R_EF_LS_ACTIVE;

    return pParticle;
}

static math::VEC3 TransformParticlePosition(bool isLocal,
                                            const math::MTX34 *localToEmitter,
                                            const math::MTX34 *localToGlobal,
                                            const math::MTX34 *emitterToGlobal,
                                            const math::VEC3 *prevPosition) {
    math::VEC3 ptclPos;
    if (isLocal) {
        math::VEC3Transform(&ptclPos, localToEmitter, prevPosition);
    } else {
        math::VEC3Transform(&ptclPos, localToGlobal, prevPosition);
        math::VEC3 emitPos(0.0f, 0.0f, 0.0f);
        math::VEC3Transform(&emitPos, emitterToGlobal, &emitPos);
        math::VEC3Sub(&ptclPos, &ptclPos, &emitPos);
    }
    return ptclPos;
}

inline bool CalcFieldGravity(math::VEC3 &affect, InfoField &info) {
    Rotation2VecY(math::VEC3(info.u.gravity.rotate), &affect);
    affect *= info.u.gravity.power;
    return true;
}

inline bool CalcFieldSpeed(math::VEC3 &affect, InfoField &info,
                           const math::VEC3 &velocity) {
    math::VEC3Scale(&affect, &velocity, info.u.speed.speed - 1.0f);
    return true;
}

inline bool CalcFieldMagnet(math::VEC3 &affect, InfoField &info,
                            const math::VEC3 &ptclPos) {
    math::VEC3Sub(&affect,
                  reinterpret_cast<math::VEC3 *>(&info.u.magnet.translate),
                  &ptclPos);
    Normalize(&affect, &affect);
    math::VEC3Scale(&affect, &affect, info.u.magnet.power);
    return true;
}

inline bool CalcFieldNewton(math::VEC3 &affect, InfoField &info,
                            const math::VEC3 &ptclPos) {
    math::VEC3Sub(&affect,
                  reinterpret_cast<math::VEC3 *>(&info.u.newton.translate),
                  &ptclPos);
    f32 len = math::VEC3LenSq(&affect);
    Normalize(&affect, &affect);
    math::VEC3Scale(&affect, &affect, info.u.newton.power);
    if (len > info.u.newton.refDistance * info.u.newton.refDistance) {
        math::VEC3Scale(&affect, &affect,
                        info.u.newton.refDistance * info.u.newton.refDistance /
                            len);
    }
    return true;
}

inline bool CalcFieldVortex(math::VEC3 &affect, InfoField &info,
                            const math::VEC3 &ptclPos) {
    math::VEC3 forcedir;
    Rotation2VecY(math::VEC3(info.u.vortex.rotate), &forcedir);
    info.u.vortex.distance *= info.u.vortex.distance;
    math::VEC3 fvec;
    math::VEC3Scale(&fvec, &forcedir, math::VEC3Dot(&forcedir, &ptclPos));
    math::VEC3Sub(&fvec, &ptclPos, &fvec);
    f32 len = math::VEC3LenSq(&fvec);
    if (len == 0.0f) {
        return false;
    }
    f32 power;
    if (len >= info.u.vortex.distance) {
        power = info.u.vortex.outerSpeed;
    } else {
        len /= info.u.vortex.distance;
        power = (1.0f - len) * info.u.vortex.innerSpeed +
                len * info.u.vortex.outerSpeed;
    }
    Normalize(&fvec, &fvec);
    math::VEC3Cross(&affect, &fvec, &forcedir);
    math::VEC3Scale(&affect, &affect, power);
    return true;
}

inline bool CalcFieldSpin(math::VEC3 &affect, InfoField &info,
                          const math::VEC3 &ptclPos) {
    math::VEC3 forcedir;
    Rotation2VecY(math::VEC3(info.u.spin.rotate), &forcedir);
    math::MTX34 mtx;
    math::MTX34RotAxisRad(&mtx, &forcedir, info.u.spin.speed);
    math::VEC3Transform(&affect, &mtx, &ptclPos);
    math::VEC3Sub(&affect, &affect, &ptclPos);
    return true;
}

inline bool CalcFieldRandom(math::VEC3 &affect, InfoField &info,
                            const AnimCurveHeader *header, const u8 *ptr,
                            const Particle *pp, u32 tick, u16 seed,
                            const math::VEC3 &prevVel,
                            const math::VEC3 &prevDir,
                            const math::MTX34 &localToEmitterNoTrans) {
    u32 rand = RANDOM_INIT(seed, header->randomSeed, tick, 0);
    if (!(reinterpret_cast<const AnimCurveHeader *>(ptr)->processFlag &
          AnimCurveHeader::PROC_FLAG_TIMING)) {
        if (tick == 0 || tick % (info.u.random.interval + 1)) {
            return false;
        }
    } else {
        if (pp->mTick != 0 && tick % (info.u.random.interval + 1)) {
            return false;
        }
    }
    if (info.option & 1) {
        math::VEC3 speed;
        switch (info.addTarget) {
        case 0:
            speed = prevVel;
            break;
        case 1:
            if (pp->mTick == 0) {
                speed = prevVel;
            } else {
                speed = prevDir / pp->mParameter.mMomentum;
            }
            break;
        }
        math::VEC3Transform(&speed, &localToEmitterNoTrans, &speed);
        info.u.random.power = math::VEC3Len(&speed);
    }
    if (info.option & 2) {
        affect.x = static_cast<s16>(rand >> 16) / 32768.0f;
        rand = RANDOM_NEXT(rand);
        affect.y = static_cast<s16>(rand >> 16) / 32768.0f;
        rand = RANDOM_NEXT(rand);
        affect.z = static_cast<s16>(rand >> 16) / 32768.0f;
        rand = RANDOM_NEXT(rand);
        if ((info.option & 1) && !Normalize(&affect, &affect)) {
            affect.y = 1.0f;
        }
        math::VEC3Scale(&affect, &affect, info.u.random.power);
    } else {
        math::VEC3 forward;
        if (pp->mTick == 0) {
            forward = prevVel;
        } else {
            forward = prevDir;
        }
        math::VEC3Transform(&forward, &localToEmitterNoTrans, &forward);
        if (!Normalize(&forward, &forward)) {
            forward.y = 1.0f;
        }
        math::MTX34 mtx;
        GetDirMtxY(&mtx, forward);
        if (info.u.random.diffusion != 0.0f) {
            f32 rx = (rand >> 16) / 65535.0f * info.u.random.diffusion;
            rand = RANDOM_NEXT(rand);
            f32 ry = 2.0f * (NW4R_MATH_PI * ((rand >> 16) / 65535.0f));
            rand = RANDOM_NEXT(rand);
            _PSSinCosRad(&affect.x, rx);
            affect.z = affect.x;
            f32 sin, cos;
            PSSinCosRad(&sin, &cos, ry);
            affect.x *= sin;
            affect.z *= cos;
            if (info.option & 1) {
                math::VEC3Scale(&affect, &affect, info.u.random.power);
            } else {
                math::VEC3Scale(&affect, &affect,
                                (rand >> 16) / 65535.0f * info.u.random.power);
            }
        } else {
            affect.x = 0.0f;
            if (info.option & 1) {
                affect.y = info.u.random.power;
            } else {
                affect.y = (static_cast<s16>(rand >> 16) / 32768.0f) *
                           info.u.random.power;
            }
            affect.z = 0.0f;
        }
        math::VEC3Transform(&affect, &mtx, &affect);
    }
    if (!(info.option & 4)) {
        affect.x = 0.0f;
    }
    if (!(info.option & 8)) {
        affect.y = 0.0f;
    }
    if (!(info.option & 16)) {
        affect.z = 0.0f;
    }
    return true;
}

void ParticleManager::Calc() {
    Particle *pFirst = static_cast<Particle *>(
        ut::List_GetNext(&mActivityList.mActiveList, mLastCalced));

    if (pFirst == NULL) {
        return;
    }

    Particle *pIt = pFirst;

    if (mManagerEM->mManagerEF->mCallBack.mPrevPtclCalc != NULL) {
        mManagerEM->mManagerEF->mCallBack.mPrevPtclCalc(
            this, &mActivityList.mActiveList, pFirst);
    }

    math::MTX34 mtxLocToGlb;
    CalcGlobalMtx(&mtxLocToGlb);

    math::MTX34 mtxGlbToLoc;
    math::MTX34Inv(&mtxGlbToLoc, &mtxLocToGlb);

    math::MTX34 mtxEmToGlb;
    mManagerEM->CalcGlobalMtx(&mtxEmToGlb);

    math::MTX34 mtxEmToGlbNoTrans = mtxEmToGlb;
    mtxEmToGlbNoTrans._03 = 0.0f;
    mtxEmToGlbNoTrans._13 = 0.0f;
    mtxEmToGlbNoTrans._23 = 0.0f;
    math::VEC3 v(0.57735026f, 0.57735026f, 0.57735026f);
    math::VEC3Transform(&v, &mtxEmToGlbNoTrans, &v);
    f32 emitterScale = math::VEC3Len(&v);

    math::MTX34 mtxEmToLoc;
    math::MTX34Mult(&mtxEmToLoc, &mtxGlbToLoc, &mtxEmToGlb);

    math::MTX34 mtxLocToEm;
    math::MTX34Inv(&mtxLocToEm, &mtxEmToLoc);

    math::MTX34 mtxGlbToLocNoTrans = mtxGlbToLoc;
    mtxGlbToLocNoTrans._03 = 0.0f;
    mtxGlbToLocNoTrans._13 = 0.0f;
    mtxGlbToLocNoTrans._23 = 0.0f;

    math::MTX34 mtxLocToGlbNoTrans;
    math::MTX34Inv(&mtxLocToGlbNoTrans, &mtxGlbToLocNoTrans);

    math::MTX34 mtxLocToEmNoTrans = mtxLocToEm;
    mtxLocToEmNoTrans._03 = 0.0f;
    mtxLocToEmNoTrans._13 = 0.0f;
    mtxLocToEmNoTrans._23 = 0.0f;

    math::MTX34 mtxEmToLocNoTrans = mtxEmToLoc;
    mtxEmToLocNoTrans._03 = 0.0f;
    mtxEmToLocNoTrans._13 = 0.0f;
    mtxEmToLocNoTrans._23 = 0.0f;

    Particle *pNext;

    for (; pIt != NULL; pIt = pNext) {
        pNext = static_cast<Particle *>(
            NW4R_UT_LIST_GET_LINK(mActivityList.mActiveList, pIt)->nextObject);

        if (pIt->mLifeStatus != NW4R_EF_LS_ACTIVE) {
            continue;
        }

        if (pIt->mEvalStatus != NW4R_EF_ES_WAIT) {
            continue;
        }

        pIt->mEvalStatus = NW4R_EF_ES_DONE;

        if (pIt->mCalcRemain != 0) {
            mManagerEM->mManagerEF->SetFlagExistCalcRemain(true);
        }

        math::VEC3 prevPos = pIt->mParameter.mPosition;
        math::VEC3 prevVel = pIt->mParameter.mVelocity;

        math::VEC3 prevDir;
        pIt->GetMoveDir(&prevDir);

        pIt->mParameter.mPrevPosition = pIt->mParameter.mPosition;

        if (pIt->mLife <= pIt->mTick) {
            RetireParticle(pIt);
            continue;
        }

        math::VEC3 addVel(0.0f, 0.0f, 0.0f);
        math::VEC3 addPos(0.0f, 0.0f, 0.0f);
        u16 i;
        bool postfieldEnable = false;
        PostFieldInfo::AnimatableParams postfieldAnimParams;
        PostFieldInfo *postfieldResource;
        EmitterResource *postfieldChildRes = NULL;

        bool findEmitterTiming = false;

        for (i = pIt->mTick == 0 ? 0 : mResource->NumPtclInitTrack();
             i < mResource->NumPtclTrack(); i++) {

            u8 *pPtclTrack = mResource->GetPtclTrack(i);

#define pTrackAsHeader reinterpret_cast<AnimCurveHeader *>(pPtclTrack)

            if (pTrackAsHeader->processFlag & AnimCurveHeader::PROC_FLAG_STOP) {
                continue;
            }

            u32 tick;
            u32 life;
            u16 seed;

            if (pTrackAsHeader->processFlag &
                AnimCurveHeader::PROC_FLAG_TIMING) {

                findEmitterTiming = true;

                tick = pIt->mParticleManager->mManagerEM->mTick;

                if (pIt->mParticleManager->mManagerEM->mParameter.mComFlags &
                    EmitterDesc::CMN_FLAG_MAX_LIFE) {
                    life = 0xFFFFFFFF;
                } else {
                    life =
                        pIt->mParticleManager->mManagerEM->mParameter.mEmitSpan;
                }

                seed = pIt->mParticleManager->mManagerEM->mRandSeed;
            } else {
                tick = pIt->mTick;
                life = pIt->mLife;
                seed = pIt->mRandSeed;
            }

            if (static_cast<u8>(pTrackAsHeader->magic - static_cast<s8>(0xAB)) >
                1) {
                continue;
            }

            const u8 kind = pTrackAsHeader->kindType;
            switch (pTrackAsHeader->curveFlag) {
            case 4:
                AnimCurveExecuteTexture(pPtclTrack, pIt, tick, seed, life);
                break;
            case 5:
                AnimCurveExecuteChild(pPtclTrack, pIt, tick, seed, life);
                break;
            case 6:
                AnimCurveExecuteRotate(pPtclTrack, &pIt->mParameter.mRotate.x,
                                       tick, seed, life);
                break;
            case 0: {
                u8 *target = reinterpret_cast<u8 *>(&pIt->mParameter) + kind;
                AnimCurveExecuteU8(pPtclTrack, target, tick, seed, life);
                break;
            }
            case 3: {
                f32 *target = reinterpret_cast<f32 *>(
                    reinterpret_cast<u8 *>(&pIt->mParameter) + kind);
                AnimCurveExecuteF32(pPtclTrack, target, tick, seed, life);
                break;
            }
            case 7: {
                AnimCurveHeader *header = pTrackAsHeader;
                u8 *infoTable = pPtclTrack + sizeof(AnimCurveHeader) +
                                header->keyTable + header->rangeTable +
                                header->randomTable + header->nameTable;
                InfoField infoCopy;
                memcpy(&infoCopy, infoTable, sizeof(infoCopy));
                if (header->keyTable != 0) {
                    AnimCurveExecuteF32(pPtclTrack, &infoCopy.u.animParam, tick,
                                        seed, life);
                }
                math::VEC3 ptclPos;
                if (kind == 6 || kind == 2 || kind == 3 || kind == 4) {
                    ptclPos = TransformParticlePosition(
                        infoCopy.space == 1, &mtxLocToEm, &mtxLocToGlb,
                        &mtxEmToGlb, &prevPos);
                }
                math::VEC3 affect(0.0f, 0.0f, 0.0f);
                switch (kind) {
                case 1:
                    CalcFieldSpeed(affect, infoCopy, prevVel);
                    break;
                case 0:
                    CalcFieldGravity(affect, infoCopy);
                    break;
                case 7:
                    CalcFieldRandom(affect, infoCopy, header, pPtclTrack, pIt,
                                    tick, seed, prevVel, prevDir,
                                    mtxLocToEmNoTrans);
                    break;
                case 6:
                    CalcFieldSpin(affect, infoCopy, ptclPos);
                    break;
                case 2:
                    CalcFieldMagnet(affect, infoCopy, ptclPos);
                    break;
                case 3:
                    CalcFieldNewton(affect, infoCopy, ptclPos);
                    break;
                case 4:
                    CalcFieldVortex(affect, infoCopy, ptclPos);
                    break;
                }
                switch (infoCopy.space) {
                case 1:
                    math::VEC3Transform(&affect, &mtxEmToLocNoTrans, &affect);
                    break;
                case 0:
                    math::VEC3Transform(&affect, &mtxGlbToLocNoTrans, &affect);
                    break;
                case 2:
                    break;
                case 3:
                    affect *= emitterScale;
                    math::VEC3Transform(&affect, &mtxGlbToLocNoTrans, &affect);
                    break;
                }
                switch (infoCopy.addTarget) {
                case 0:
                    addVel += affect;
                    break;
                case 1:
                    addPos += affect;
                    break;
                }
                break;
            }
            case 2: {
                AnimCurveHeader *header = pTrackAsHeader;
                if (header->infoTable != 0) {
                    AnimCurveNameTable *nameTable =
                        reinterpret_cast<AnimCurveNameTable *>(
                            pPtclTrack + sizeof(AnimCurveHeader) +
                            header->keyTable + header->rangeTable +
                            header->randomTable);
                    postfieldResource = reinterpret_cast<PostFieldInfo *>(
                        reinterpret_cast<u8 *>(nameTable) + header->nameTable);
                    memcpy(&postfieldAnimParams, postfieldResource,
                           sizeof(postfieldAnimParams));
                    if (header->nameTable > 4) {
                        postfieldChildRes = reinterpret_cast<EmitterResource *>(
                            nameTable
                                ->datas[postfieldResource->mChildOption
                                            .mNameIdx]
                                .work);
                    }
                }
                if (header->keyTable != 0) {
                    f32 *target = reinterpret_cast<f32 *>(
                        reinterpret_cast<u8 *>(&postfieldAnimParams) + kind);
                    AnimCurveExecuteF32(pPtclTrack, target, tick, seed, life);
                }
                postfieldEnable = true;
                break;
            }
            }

#undef pTrackAsHeader
        }

        if (postfieldEnable) {
            math::VEC3 nextVelocity(pIt->mParameter.mVelocity);
            nextVelocity += addVel;
            bool requestKilling = false;
            bool normalMove =
                calcPostField(pIt, postfieldAnimParams, postfieldResource,
                              postfieldChildRes, mtxEmToGlb, mtxGlbToLoc,
                              prevPos, addPos, &nextVelocity, &requestKilling);
            if (requestKilling) {
                RetireParticle(pIt);
                continue;
            }
            if (normalMove) {
                pIt->mParameter.mVelocity = nextVelocity;
                pIt->AddPosition(&addPos);
                pIt->AddPosition(&pIt->mParameter.mVelocity);
            }
            if (postfieldResource->mWrapOption & 1) {
                math::MTX34 mtxFormToLocal;
                math::MTX34Identity(&mtxFormToLocal);
                MTX34ScaleNonZero(&mtxFormToLocal,
                                  &postfieldResource->mWrapScale,
                                  &mtxFormToLocal);
                math::MTX34 rot;
                PSMTX34RotXYZRad(&rot, postfieldResource->mWrapRotate.x,
                                 postfieldResource->mWrapRotate.y,
                                 postfieldResource->mWrapRotate.z);
                math::MTX34Mult(&mtxFormToLocal, &rot, &mtxFormToLocal);
                math::MTX34Trans(&mtxFormToLocal,
                                 &postfieldResource->mWrapTranslate,
                                 &mtxFormToLocal);
                if (postfieldResource->mWrapOption & 2) {
                    math::VEC3 vec(mtxEmToGlb._03, mtxEmToGlb._13,
                                   mtxEmToGlb._23);
                    math::MTX34Trans(&mtxFormToLocal, &vec, &mtxFormToLocal);
                }
                math::MTX34Mult(&mtxFormToLocal, &mtxGlbToLoc, &mtxFormToLocal);
                math::MTX34 mtxLocalToForm;
                math::MTX34Inv(&mtxLocalToForm, &mtxFormToLocal);
                math::VEC3 posInForm;
                math::VEC3Transform(&posInForm, &mtxLocalToForm,
                                    &pIt->mParameter.mPosition);
                bool wrap = false;
                if (posInForm.x > 1.0f) {
                    posInForm.x = std::fmodf(posInForm.x + 1.0f, 2.0f) - 1.0f;
                    wrap = true;
                } else if (posInForm.x < -1.0f) {
                    posInForm.x = std::fmodf(posInForm.x - 1.0f, 2.0f) + 1.0f;
                    wrap = true;
                }
                if (posInForm.y > 1.0f) {
                    posInForm.y = std::fmodf(posInForm.y + 1.0f, 2.0f) - 1.0f;
                    wrap = true;
                } else if (posInForm.y < -1.0f) {
                    posInForm.y = std::fmodf(posInForm.y - 1.0f, 2.0f) + 1.0f;
                    wrap = true;
                }
                if (posInForm.z > 1.0f) {
                    posInForm.z = std::fmodf(posInForm.z + 1.0f, 2.0f) - 1.0f;
                    wrap = true;
                } else if (posInForm.z < -1.0f) {
                    posInForm.z = std::fmodf(posInForm.z - 1.0f, 2.0f) + 1.0f;
                    wrap = true;
                }
                if (wrap) {
                    math::VEC3Transform(&pIt->mParameter.mPosition,
                                        &mtxFormToLocal, &posInForm);
                }
            }
        } else {
            pIt->mParameter.mVelocity += addVel;
            pIt->AddPosition(&addPos);
            pIt->AddPosition(&pIt->mParameter.mVelocity);
        }

        pIt->mTick++;
    }

    mLastCalced =
        static_cast<Particle *>(ut::List_GetLast(&mActivityList.mActiveList));

    if (mManagerEM->mManagerEF->mCallBack.mPostPtclCalc != NULL) {
        mManagerEM->mManagerEF->mCallBack.mPostPtclCalc(
            this, &mActivityList.mActiveList, pFirst);
    }
}

void ParticleManager::Draw(const DrawInfo &rInfo) {
    const EmitterDesc *pDesc = mResource->GetEmitterDesc();

    if ((pDesc->drawSetting.mFlags & EmitterDrawSetting::FLAG_HIDDEN) ||
        (mManagerEM->mParameter.mComFlags &
         EmitterDesc::CMN_FLAG_DISABLE_DRAW)) {
        return;
    }

    DrawInfo drawInfoWithZ(rInfo);
    f32 zOffset = mResource->GetEmitterDrawSetting()->zOffset;
    if (zOffset != 0.0f) {
        math::MTX34 emMtx;
        mManagerEM->CalcGlobalMtx(&emMtx);
        math::VEC3 orig(emMtx._03, emMtx._13, emMtx._23);
        drawInfoWithZ.SetZOffset(zOffset, orig);
    }
    mDrawStrategy->Draw(drawInfoWithZ, this);
}

math::MTX34 *ParticleManager::CalcGlobalMtx(math::MTX34 *pResult) {
    if (mMtxDirty) {
        math::MTX34 orig;
        mManagerEM->CalcGlobalMtx(&orig);

        mManagerEM->RestructMatrix(&mMtx, &orig, mFlag & FLAG_MTX_INHERIT_SCALE,
                                   mFlag & FLAG_MTX_INHERIT_ROT,
                                   mInheritTranslate,
                                   mFlag & FLAG_MTX_INHERIT_PIVOT);

        mMtxDirty = false;
    }

    *pResult = mMtx;
    return pResult;
}

void ParticleManager::BeginCalc(bool onlyIfRemain) {
    mLastCalced = NULL;

    Particle *pIt =
        static_cast<Particle *>(mActivityList.mActiveList.headObject);

    // clang-format off
    for (; pIt != NULL; pIt = static_cast<Particle*>(
            NW4R_UT_LIST_GET_LINK(mActivityList.mActiveList, pIt)->nextObject))
    // clang-format on
    {
        if (!onlyIfRemain || pIt->mCalcRemain != 0) {
            if (pIt->mCalcRemain != 0) {
                pIt->mCalcRemain--;
            }

            if (pIt->GetLifeStatus() == NW4R_EF_LS_ACTIVE &&
                pIt->mEvalStatus == NW4R_EF_ES_DONE) {

                pIt->mEvalStatus = NW4R_EF_ES_WAIT;
            }
        }
    }
}

void ParticleManager::EndCalc() {
    Particle *pIt =
        static_cast<Particle *>(mActivityList.mActiveList.headObject);

    // clang-format off
    for (; pIt != NULL; pIt = static_cast<Particle*>(
            NW4R_UT_LIST_GET_LINK(mActivityList.mActiveList, pIt)->nextObject))
    // clang-format on
    {
        if (pIt->GetLifeStatus() == NW4R_EF_LS_ACTIVE &&
            pIt->mEvalStatus == NW4R_EF_ES_SKIP) {

            pIt->mEvalStatus = NW4R_EF_ES_DONE;
        }
    }
}

void ParticleManager::BeginDraw() {
    math::MTX34 emMtx;
    math::MTX34 pmMtx;

    mManagerEM->CalcGlobalMtx(&emMtx);
    CalcGlobalMtx(&pmMtx);

    math::MTX34Inv(&emMtx, &emMtx);
    math::MTX34Mult(&smDrawMtxPMtoEM, &emMtx, &pmMtx);
}

const math::MTX34 *ParticleManager::Draw_GetMtxPMtoEM() const {
    return &smDrawMtxPMtoEM;
}

void ParticleManager::EndDraw() {
}

void ParticleManager::Draw_ModifyColor(Particle *pParticle, GXColor *pColorPri,
                                       GXColor *pColorSec) {
    switch (mModifier.mLight.mType) {
    case ParticleModifier::SIMPLELIGHT_AMBIENT: {
        pColorPri->r = (pColorPri->r * mModifier.mLight.mAmbient.r + 128) >> 8;
        pColorPri->g = (pColorPri->g * mModifier.mLight.mAmbient.g + 128) >> 8;
        pColorPri->b = (pColorPri->b * mModifier.mLight.mAmbient.b + 128) >> 8;
        pColorPri->a = (pColorPri->a * mModifier.mLight.mAmbient.a + 128) >> 8;

        pColorSec->r = (pColorSec->r * mModifier.mLight.mAmbient.r + 128) >> 8;
        pColorSec->g = (pColorSec->g * mModifier.mLight.mAmbient.g + 128) >> 8;
        pColorSec->b = (pColorSec->b * mModifier.mLight.mAmbient.b + 128) >> 8;
        pColorSec->a = (pColorSec->a * mModifier.mLight.mAmbient.a + 128) >> 8;
        break;
    }

    case ParticleModifier::SIMPLELIGHT_DIFFUSE: {
        if (mModifier.mLight.mRadius < NW4R_MATH_FLT_EPSILON) {
            // clang-format off
            pColorPri->r = (pColorPri->r * mModifier.mLight.mAmbient.r + 128) >> 8;
            pColorPri->g = (pColorPri->g * mModifier.mLight.mAmbient.g + 128) >> 8;
            pColorPri->b = (pColorPri->b * mModifier.mLight.mAmbient.b + 128) >> 8;
            pColorPri->a = (pColorPri->a * mModifier.mLight.mAmbient.a + 128) >> 8;

            pColorSec->r = (pColorSec->r * mModifier.mLight.mAmbient.r + 128) >> 8;
            pColorSec->g = (pColorSec->g * mModifier.mLight.mAmbient.g + 128) >> 8;
            pColorSec->b = (pColorSec->b * mModifier.mLight.mAmbient.b + 128) >> 8;
            pColorSec->a = (pColorSec->a * mModifier.mLight.mAmbient.a + 128) >> 8;
            // clang-format on
        } else {
            const math::MTX34 *pMtxPMtoEM = Draw_GetMtxPMtoEM();

            math::VEC3 pos;
            math::VEC3Transform(&pos, pMtxPMtoEM,
                                &pParticle->mParameter.mPosition);

            math::VEC3Sub(&pos, &pos, &mModifier.mLight.mPosition);
            f32 dist = math::VEC3Len(&pos);

            if (dist > mModifier.mLight.mRadius) {
                // clang-format off
                pColorPri->r = (pColorPri->r * mModifier.mLight.mAmbient.r + 128) >> 8;
                pColorPri->g = (pColorPri->g * mModifier.mLight.mAmbient.g + 128) >> 8;
                pColorPri->b = (pColorPri->b * mModifier.mLight.mAmbient.b + 128) >> 8;
                pColorPri->a = (pColorPri->a * mModifier.mLight.mAmbient.a + 128) >> 8;

                pColorSec->r = (pColorSec->r * mModifier.mLight.mAmbient.r + 128) >> 8;
                pColorSec->g = (pColorSec->g * mModifier.mLight.mAmbient.g + 128) >> 8;
                pColorSec->b = (pColorSec->b * mModifier.mLight.mAmbient.b + 128) >> 8;
                pColorSec->a = (pColorSec->a * mModifier.mLight.mAmbient.a + 128) >> 8;
                // clang-format on
            } else {
                s32 attn = (256 * dist) / mModifier.mLight.mRadius;

                u16 lr = mModifier.mLight.mDiffuse.r * 256 +
                         attn * (mModifier.mLight.mAmbient.r -
                                 mModifier.mLight.mDiffuse.r);

                u16 lg = mModifier.mLight.mDiffuse.g * 256 +
                         attn * (mModifier.mLight.mAmbient.g -
                                 mModifier.mLight.mDiffuse.g);

                u16 lb = mModifier.mLight.mDiffuse.b * 256 +
                         attn * (mModifier.mLight.mAmbient.b -
                                 mModifier.mLight.mDiffuse.b);

                u16 la = mModifier.mLight.mDiffuse.a * 256 +
                         attn * (mModifier.mLight.mAmbient.a -
                                 mModifier.mLight.mDiffuse.a);

                pColorPri->r = (pColorPri->r * lr + 128) >> 16;
                pColorPri->g = (pColorPri->g * lg + 128) >> 16;
                pColorPri->b = (pColorPri->b * lb + 128) >> 16;
                pColorPri->a = (pColorPri->a * la + 128) >> 16;

                pColorSec->r = (pColorSec->r * lr + 128) >> 16;
                pColorSec->g = (pColorSec->g * lg + 128) >> 16;
                pColorSec->b = (pColorSec->b * lb + 128) >> 16;
                pColorSec->a = (pColorSec->a * la + 128) >> 16;
            }
        }
        break;
    }

    default: {
        break;
    }
    }
}

} // namespace ef
} // namespace nw4r
