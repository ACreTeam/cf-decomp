#ifndef MSL_INTERNAL_MATH_NAN_H
#define MSL_INTERNAL_MATH_NAN_H

#ifdef __cplusplus
extern "C" {
#endif

extern int __float_nan[];

#ifdef __cplusplus
}
#endif

#define NAN (*(float*)__float_nan)

#endif // MSL_INTERNAL_MATH_NAN_H
