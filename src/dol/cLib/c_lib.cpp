#include <game/cLib/c_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/revolution/MTX/vec.h>
#include <string.h>

inline float calcDistance(const mVec3_c &a, const mVec3_c &b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return EGG::Mathf::sqrt(dx * dx + dy * dy + dz * dz);
}

inline float calcDistanceXZ(const mVec3_c &a, const mVec3_c &b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;

    return EGG::Mathf::sqrt(dx * dx + dz * dz);
}

namespace cLib {

void *memCpy(void *dst, const void *src, size_t size) {
    return memcpy(dst, src, size);
}

void *memSet(void *dst, int val, ulong size) {
    return memset(dst, val, size);
}

int memCmp(const void *p0, const void *p1, size_t size) {
    return memcmp(p0, p1, size);
}

float addCalcPos(mVec3_c *currPos, const mVec3_c &targetPos, float ratio,
                 float maxStep, float minStep) {
    if (*currPos != targetPos) {
        mVec3_c stepVector;
        stepVector =
            *currPos -
            targetPos; // need mVec3_c::operator- to not be inlined here
        float distance = PSVECMag(stepVector);
        if (distance < minStep) {
            *currPos = targetPos;
        } else {
            stepVector *= ratio;
            float step = distance * ratio;
            if (!isZero(step)) {
                if (step > maxStep) {
                    stepVector *= maxStep / step;
                } else if (step < minStep) {
                    stepVector *= minStep / step;
                }
                *currPos -= stepVector;
            } else {
                *currPos = targetPos;
            }
        }
    }

    return calcDistance(*currPos, targetPos);
}

float addCalcPosXZ(mVec3_c *currentPos, const mVec3_c &targetPos, float ratio,
                   float maxStep, float minStep) {
    if (currentPos->x != targetPos.x || currentPos->z != targetPos.z) {
        mVec3_c stepVector;
        stepVector.x = currentPos->x - targetPos.x;
        stepVector.z = currentPos->z - targetPos.z;
        float distance = stepVector.xzLen();
        if (distance < minStep) {
            currentPos->x = targetPos.x;
            currentPos->z = targetPos.z;
        } else {
            stepVector.x *= ratio;
            stepVector.z *= ratio;
            float step = distance * ratio;
            if (!isZero(step)) {
                if (step > maxStep) {
                    float scale = maxStep / step;
                    stepVector.x *= scale;
                    stepVector.z *= scale;
                } else if (step < minStep) {
                    float scale = minStep / step;
                    stepVector.x *= scale;
                    stepVector.z *= scale;
                }
                currentPos->x -= stepVector.x;
                currentPos->z -= stepVector.z;
            } else {
                currentPos->x = targetPos.x;
                currentPos->z = targetPos.z;
            }
        }
    }

    mVec3_c offset;
    offset.x = currentPos->x - targetPos.x;
    offset.z = currentPos->z - targetPos.z;
    return offset.xzLen();
}

void addCalcPos2(mVec3_c *currentPos, const mVec3_c &targetPos, float ratio,
                 float maxStep) {
    if (*currentPos != targetPos) {
        mVec3_c stepVector;

        stepVector = ratio * (*currentPos - targetPos);
        float distance = PSVECMag(stepVector);

        if (distance > maxStep) {
            stepVector.normalize();
            stepVector *= maxStep;
        }
        *currentPos -= stepVector;
    }
}

void addCalcPosXZ2(mVec3_c *currentPos, const mVec3_c &targetPos, float ratio,
                   float maxStep) {
    if (currentPos->x != targetPos.x || currentPos->z !=targetPos.z) {
        mVec3_c stepVector;
        float distance;

        stepVector.x = ratio * (currentPos->x - targetPos.x);
        stepVector.z = ratio * (currentPos->z - targetPos.z);
        distance = stepVector.xzLen();
        if (!cLib::isZero(distance)) {
            if (distance > maxStep) {
                float n = maxStep / distance;
                stepVector.x *= n;
                stepVector.z *= n;
            }
            currentPos->x -= stepVector.x;
            currentPos->z -= stepVector.z;
        }
    }
}

bool chasePos(mVec3_c *currentPos, const mVec3_c &targetPos, float step) {
    if (step) {
        mVec3_c offset = *currentPos - targetPos;
        float distance = PSVECMag(offset);
        if (isZero(distance) || distance <= step) {
            *currentPos = targetPos;
            return true;
        }
        *currentPos -= (step / distance) * offset;
    } else if (*currentPos == targetPos) {
        return true;
    }
    return false;
}

bool chasePosXZ(mVec3_c *currentPos, const mVec3_c &targetPos, float step) {
    mVec3_c offset;
    float distance;
    
    offset.x = currentPos->x - targetPos.x;
    offset.z = currentPos->z - targetPos.z;
    distance = offset.xzLen();

    if (step) {
        if (isZero(distance) || distance <= step) {
            currentPos->x = targetPos.x;
            currentPos->z = targetPos.z;
            return true;
        }

        currentPos->x -= (step / distance) * offset.x;
        currentPos->z -= (step / distance) * offset.z;
    } else if (isZero(distance)) {
        return true;
    }
    return false;
}

s16 targetAngleY(const mVec3_c &vec1, const mVec3_c &vec2) {
    return cM::atan2s(vec2.x - vec1.x, vec2.z - vec1.z);
}

s16 targetAngleX(const mVec3_c &vec1, const mVec3_c &vec2) {
    mVec3_c diff = vec2 - vec1;
    return cM::atan2s(diff.y, diff.xzLen());
}

void offsetPos(mVec3_c* result, const mVec3_c &origin, s16 angle, const mVec3_c &offset) {
    float sinForZ;
    float cos = nw4r::math::CosIdx(angle);
    float sin = nw4r::math::SinIdx(angle);

    result->x = origin.x + (offset.x * cos + offset.z * sin);
    sinForZ = sin; // required for the match
    result->y = origin.y + offset.y;
    result->z = origin.z + (offset.z * cos - offset.x * sinForZ);
}

float easeOut(float t, float exponent) {
    return 1.0f - std::powf(1.0f - t, exponent);
}

} // namespace cLib
