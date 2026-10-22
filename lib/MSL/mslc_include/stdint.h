#ifndef _STDINT_H_
#define _STDINT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Minimum-width integer types (C99 7.18.1.2). */
typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
typedef short int_least16_t;
typedef unsigned short uint_least16_t;
typedef long int_least32_t;
typedef unsigned long uint_least32_t;

typedef unsigned long int uintptr_t;

#ifdef __cplusplus
}
#endif

#endif
