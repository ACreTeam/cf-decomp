#pragma once
#include <types.h>

/// @brief The signature of the function used to print formatted output.
typedef int (*vprintfFunc)(const char *format, va_list *args);

/**
 * @brief A collection of motion and interpolation utilities.
 * @details Provides utilities for timer management, proportional smoothing and fixed step movement
 * for both scalar and angular data types.
 *
 * All functions operate directly on pointers to allow for in-place updates.
 * @ingroup slib
 */
namespace sLib {

/**
 * @brief Decrements a timer value.
 * @details The timer is only decreased if it's non-zero.
 * @param value The timer value to be decremented.
 * @return The updated timer value.
 */
template <typename T>
T calcTimer(T *value) {
    if (*value != 0) {
        (*value)--;
    }
    return *value;
}

/**
 * @brief Smoothly moves @p value towards @p target using proportional scaling.
 * @details If the applied step overshoots the target, the value is snapped exactly to the target.
 * @param value The value to be updated.
 * @param target The target value.
 * @param smoothing The smoothing factor (between @p 0.0 and @p 1.0). Higher value means snappier movement.
 * @param maxStep The maximum step for a single update.
 * @param minStep The minimum step to prevent jittering.
 * @return The absolute remaining distance to the target.
 */
float addCalc(float *value, float target, float smoothing, float maxStep, float minStep);

/// @brief Smooths towards a target with a maximum step and no minimum step.
void addCalc2(float *value, float target, float smoothing, float maxStep);

/// @brief Smooths towards zero with a maximum step.
void addCalc0(float *value, float smoothing, float maxStep);

/// @brief Returns the absolute wrapped distance between two binary angles.
int distanceAngle(s16 angle1, s16 angle2);

/**
 * @brief Smoothly moves @p value towards @p target using proportional scaling.
 * @details If the applied step overshoots the target, the value is snapped exactly to the target.
 * @param value The value to be updated.
 * @param target The target value.
 * @param smoothing The smoothing factor. Higher value means smoother movement.
 * @param maxStep The maximum step for a single update.
 * @param minStep The minimum step to prevent jittering.
 * @return The absolute remaining distance to the target.
 */
template <typename T>
T addCalcAngleT(T *value, T target, T smoothing, T maxStep, T minStep);
s16 addCalcAngle(s16 *value, s16 target, s16 smoothing, s16 maxStep, s16 minStep); ///< @copydoc sLib::addCalcAngleT(T*, T, T, T, T)

/**
 * @brief Smoothly moves @p value towards @p target using proportional scaling.
 * @details If the applied step overshoots the target, the value is snapped exactly to the target.
 * @param value The value to be updated.
 * @param target The target value.
 * @param smoothing The smoothing factor. Higher value means smoother movement.
 * @param maxStep The maximum step for a single update.
 */
template <typename T>
void addCalcAngleT(T *value, T target, T smoothing, T maxStep);
void addCalcAngle(s16 *value, s16 target, s16 smoothing, s16 maxStep); ///< @copydoc sLib::addCalcAngleT(T*, T, T, T)

/** @brief Moves @p value towards @p target by a fixed @p step amount.
 * @details The step direction is automatically adjusted to move toward the target.
 * If the applied step overshoots the target, the value is snapped exactly to the target.
 * @param value The value to be updated.
 * @param target The target value.
 * @param step The step value.
 * @return @p TRUE if the value reached the target, @p FALSE otherwise.
 */
template <typename T>
BOOL chaseT(T *value, T target, T step);
BOOL chase(u8 *value, u8 target, s16 step);
BOOL chase(s16 *value, s16 target, s16 step); ///< @copydoc sLib::chaseT
BOOL chase(int *value, int target, int step); ///< @copydoc sLib::chaseT
BOOL chase(long *value, long target, long step); ///< @copydoc sLib::chaseT
BOOL chase(float *value, float target, float step); ///< @copydoc sLib::chaseT

/** @brief Moves @p value towards @p target by a fixed @p step amount.
 * @details The step direction is automatically adjusted to move toward the target.
 * If the applied step overshoots the target, the value is snapped exactly to the target.
 * @param value The value to be updated.
 * @param target The target value.
 * @param step The step value.
 * @return @p TRUE if the value reached the target, @p FALSE otherwise.
 */
BOOL chaseAngle(s16 *value, s16 target, s16 step);

/// @brief Tests an inclusive range, allowing either order for the bounds.
template <typename T>
BOOL isInRangeT(T value, T bound1, T bound2);
BOOL isInRange(int value, int bound1, int bound2);
BOOL isInRange(s16 value, s16 bound1, s16 bound2);
BOOL isInRange(float value, float bound1, float bound2);

extern vprintfFunc p_VPrintfFuncPtr; ///< Pointer to std::printf. @unofficial

} // namespace sLib
