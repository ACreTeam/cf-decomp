#include <nw4r/ef.h>

namespace nw4r {
namespace ef {

CreationQueue::CreationQueue() : mNumItem(0) {
}

void CreationQueue::AddParticleCreation(const EmitterInheritSetting *pSetting,
                                        Particle *pParticle,
                                        EmitterResource *pResource,
                                        u16 calcRemain,
                                        const math::VEC3 *position,
                                        const math::VEC3 *velocity) {

    if (mNumItem >= QUEUE_SIZE) {
        return;
    }

    mQueueData[mNumItem].mType = CreationQueueData::TYPE_PARTICLE;
    mQueueData[mNumItem].mFlags = 0;
    mQueueData[mNumItem].mCalcRemain = calcRemain;
    mQueueData[mNumItem].mInheritSetting = *pSetting;
    mQueueData[mNumItem].mReferenceParticle = pParticle;

    pParticle->Ref();

    mQueueData[mNumItem].mEmitterResource = pResource;

    if (position) {
        mQueueData[mNumItem].mFlags |= 1;
        mQueueData[mNumItem].mPosition = *position;
    }
    if (velocity) {
        mQueueData[mNumItem].mFlags |= 2;
        mQueueData[mNumItem].mVelocity = *velocity;
    }
    mNumItem++;
}

void CreationQueue::AddEmitterCreation(const EmitterInheritSetting *pSetting,
                                       Particle *pParticle,
                                       EmitterResource *pResource,
                                       u16 calcRemain,
                                       const math::VEC3 *position,
                                       const math::VEC3 *velocity) {

    if (mNumItem >= QUEUE_SIZE) {
        return;
    }

    mQueueData[mNumItem].mType = CreationQueueData::TYPE_EMITTER;
    mQueueData[mNumItem].mFlags = 0;
    mQueueData[mNumItem].mCalcRemain = calcRemain;
    mQueueData[mNumItem].mInheritSetting = *pSetting;
    mQueueData[mNumItem].mReferenceParticle = pParticle;

    pParticle->Ref();

    mQueueData[mNumItem].mEmitterResource = pResource;

    if (position) {
        mQueueData[mNumItem].mFlags |= 1;
        mQueueData[mNumItem].mPosition = *position;
    }
    if (velocity) {
        mQueueData[mNumItem].mFlags |= 2;
        mQueueData[mNumItem].mVelocity = *velocity;
    }
    mNumItem++;
}

void CreationQueue::Execute() {
    for (int i = 0; i < mNumItem; i++) {
        math::VEC3 *position = NULL;
        CreationQueueData &rData = mQueueData[i];
        if (rData.mFlags & 1) {
            position = &rData.mPosition;
        }

        switch (rData.mType) {
        case CreationQueueData::TYPE_PARTICLE: {
            rData.mReferenceParticle->mParticleManager->mManagerEM
                ->CreateEmitterTmp(
                    rData.mEmitterResource, &rData.mInheritSetting,
                    rData.mReferenceParticle, rData.mCalcRemain, position);

            rData.mReferenceParticle->UnRef();
            break;
        }

        case CreationQueueData::TYPE_EMITTER: {
            rData.mReferenceParticle->mParticleManager->mManagerEM
                ->CreateEmitter(rData.mEmitterResource, &rData.mInheritSetting,
                                rData.mReferenceParticle, rData.mCalcRemain,
                                position);

            rData.mReferenceParticle->UnRef();
            break;
        }
        }
    }

    mNumItem = 0;
}

} // namespace ef
} // namespace nw4r
