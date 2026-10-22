#ifndef MSL_ANSI_FP_H
#define MSL_ANSI_FP_H

#include <types.h>

#include <float.h>
#include <math.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIGDIGLEN 36

typedef struct decimal {
    char sign;
    char unk1;
    s16 exp;
    struct {
        u8 length;
        u8 text[SIGDIGLEN];
        u8 unk41;
    } sig;
} decimal;

typedef struct decform {
    char style;
    char unk1;
    s16 digits;
} decform;

void __ull2dec(decimal* result, u64 value);
void __timesdec(decimal* result, const decimal* x, const decimal* y);
void __str2dec(decimal* d, const char* s, short exp);
void __two_exp(decimal* result, long exp);
bool __equals_dec(const decimal* x, const decimal* y);
bool __less_dec(const decimal* x, const decimal* y);
void __minus_dec(decimal* result, const decimal* x, const decimal* y);
void __num2dec_internal(decimal* d, double value);
double __dec2num(const decimal* d);
void __num2dec(const decform* form, double value, decimal* d);

#ifdef __cplusplus
}
#endif

#endif // MSL_ANSI_FP_H
