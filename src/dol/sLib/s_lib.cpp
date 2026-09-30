#include <game/sLib/s_lib.hpp>
#include <MSL/cstdio>
#include <MSL/stdlib.h>

vprintfFunc sLib::p_VPrintfFuncPtr = vprintf;

float sLib::addCalc(float *value, float target, float smoothing, float maxStep, float minStep) {
    if (*value != target) {
        float dist = target - *value;
        float step = smoothing * dist;

        if (step >= minStep || step <= -minStep) {
            if (step > maxStep) {
                step = maxStep;
            }
            if (step < -maxStep) {
                step = -maxStep;
            }

            *value += step;

        } else if (step > 0.0f) {
            if (step < minStep) {
                *value += minStep;
                if (*value > target) {
                    *value = target;
                }
            }

        } else {
            if (step > -minStep) {
                *value += -minStep;
                if (*value < target) {
                    *value = target;
                }
            }
        }
    }

    float dist = target - *value;
    return (dist > 0.0f) ? dist : -dist;
}

void sLib::addCalc2(float *value, float target, float smoothing, float maxStep) {
    if (*value != target) {
        float step = smoothing * (target - *value);
        if (step > maxStep) {
            step = maxStep;
        } else if (step < -maxStep) {
            step = -maxStep;
        }
        *value += step;
    }
}

void sLib::addCalc0(float *value, float smoothing, float maxStep) {
    float step = *value * smoothing;
    if (step > maxStep) {
        step = maxStep;
    } else if (step < -maxStep) {
        step = -maxStep;
    }
    *value -= step;
}

int sLib::distanceAngle(s16 angle1, s16 angle2) {
    return abs((s16)(angle1 - angle2));
}

s16 sLib::addCalcAngle(s16 *value, s16 target, s16 smoothing, s16 maxStep, s16 minStep) {
    return addCalcAngleT<s16>(value, target, smoothing, maxStep, minStep);
}

void sLib::addCalcAngle(s16 *value, s16 target, s16 smoothing, s16 maxStep) {
    addCalcAngleT<s16>(value, target, smoothing, maxStep);
}

BOOL sLib::chase(u8 *value, u8 target, s16 step) {
    if (step) {
        s16 current = *value;
        s16 signedTarget = target;
        s16 signedStep = step;
        if (current > signedTarget) {
            signedStep = -signedStep;
        }

        current += signedStep;
        if (signedStep * (current - signedTarget) >= 0) {
            *value = target;
            return TRUE;
        }
        *value = current;
    } else if (*value == target) {
        return TRUE;
    }
    return FALSE;
}

BOOL sLib::chase(s16 *value, s16 target, s16 step) {
    return sLib::chaseT<s16>(value, target, step);
}

BOOL sLib::chase(int *value, int target, int step) {
    return sLib::chaseT<int>(value, target, step);
}

BOOL sLib::chase(long *value, long target, long step) {
    return sLib::chaseT<long>(value, target, step);
}

BOOL sLib::chase(float *value, float target, float step) {
    return sLib::chaseT<float>(value, target, step);
}

BOOL sLib::isInRange(int value, int bound1, int bound2) {
    return isInRangeT<int>(value, bound1, bound2);
}

BOOL sLib::isInRange(s16 value, s16 bound1, s16 bound2) {
    return isInRangeT<s16>(value, bound1, bound2);
}

BOOL sLib::isInRange(float value, float bound1, float bound2) {
    return isInRangeT<float>(value, bound1, bound2);
}

BOOL sLib::chaseAngle(s16 *value, s16 target, s16 step) {
    if (*value == target) {
        return TRUE;
    }

    if (step != 0) {
        s16 dist = *value - target;
        if (dist > 0) {
            step = -step;
        }

        *value += step;
        dist = *value - target;

        if (step * dist >= 0) {
            *value = target;
            return TRUE;
        }
    }

    return FALSE;
}

// Define the templates after their callers so MWCC emits their instances last.
template <typename T>
T sLib::addCalcAngleT(T *value, T target, T smoothing, T maxStep, T minStep) {
    T dist = target - *value;
    if (*value != target) {
        T step = dist / smoothing;

        if (step > minStep || step < -minStep) {
            if (step > maxStep) {
                step = maxStep;
            }
            else if (step < -maxStep) {
                step = -maxStep;
            }

            *value += step;

        } else if (dist >= 0) {
            *value += minStep;
            dist = target - *value;
            if (dist <= 0) {
                *value = target;
            }

        } else {
            *value -= minStep;
            dist = target - *value;
            if (dist >= 0) {
                *value = target;
            }
        }
    }

    return target - *value;
}

template <typename T>
void sLib::addCalcAngleT(T *value, T target, T smoothing, T maxStep) {
    T dist = target - *value;
    T step = dist / smoothing;

    if (step > maxStep) {
        *value += maxStep;
    } else if (step < -maxStep) {
        *value -= maxStep;
    } else {
        *value += step;
    }
}

template <typename T>
BOOL sLib::chaseT(T *value, T target, T step) {
    if (*value == target) {
        return TRUE;
    }

    if (step) {
        if (*value > target) {
            step = -step;
        }

        *value += step;
        if (step * (*value - target) >= 0) {
            *value = target;
            return TRUE;
        }
    }

    return FALSE;
}

template <typename T>
BOOL sLib::isInRangeT(T value, T bound1, T bound2) {
    if (bound1 < bound2) {
        return value >= bound1 && value <= bound2;
    } else {
        return value >= bound2 && value <= bound1;
    }
}

// Emit the template instances in the order of their public wrappers.
template s16 sLib::addCalcAngleT<s16>(s16 *, s16, s16, s16, s16);
template void sLib::addCalcAngleT<s16>(s16 *, s16, s16, s16);
template BOOL sLib::chaseT<s16>(s16 *, s16, s16);
template BOOL sLib::chaseT<int>(int *, int, int);
template BOOL sLib::chaseT<long>(long *, long, long);
template BOOL sLib::chaseT<float>(float *, float, float);
template BOOL sLib::isInRangeT<int>(int, int, int);
template BOOL sLib::isInRangeT<s16>(s16, s16, s16);
template BOOL sLib::isInRangeT<float>(float, float, float);
