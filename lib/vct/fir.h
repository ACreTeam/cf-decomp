#ifndef VCT_FIR_H
#define VCT_FIR_H

#include <math.h>
#include <revolution/os.h>
#include <revolution/types.h>

#include "vct.h"

// Integer helpers and filter entry points. libvct (DS) runs the divider and
// square root on the NITRO coprocessor; the Wii build goes through the compiler
// runtime and reports a bad result with OSPanic. OSPanic receives __FILE__ and
// __LINE__, so SQRT sits on line 31 and DIV64 on line 42 of this header (the
// original library embeds those numbers).

#define Q15_SHIFT 15

// 12-bit fixed point used by the high-pass filter
#define FX_SHIFT 12
#define FX_CONST(x) ((int)((x) * (1 << FX_SHIFT) + 0.5))

// Largest r with r * r <= n. The libm result is checked against its
// neighbours because the original code did not trust the conversion
// (u32 -> double -> u32); a mismatch reports and halts.
// Used for the VAD thresholds, the microphone gain and the FIR gain.
//
// Both products wrap in u32, which is what the checks below expect.
static inline u32 SQRT(u32 n) {
    u32 r = (u32)sqrt((double)n);
    if (r * r > n || (r + 1) * (r + 1) < n) {
        OSPanic(__FILE__, __LINE__, "WARNING!  Problematic SQRT().\n");
    }
    return r;
}


// Quotient of a 64-bit energy sum and a sample count. The result is expected to
// fit in 32 bits; an overflow reports and halts like SQRT does.
static inline u32 DIV64(u64 numer, u32 denom) {
    u64 quotient = numer / denom;
    if (quotient > 0xFFFFFFFF) {
        OSPanic(__FILE__, __LINE__, "WARNING!  Overflow in DIV64().\n");
    }
    return (u32)quotient;
}

u32 HPFFilter(void* audio_data, u32 length);
u32 HPFFilterPower(void* audio_data, u32 length);
void InitFIRFilter(void);
void vct_set_speaker_samples(void* spkBuffer, u32 length);
void DoFIRFilter(void* micSample, void* spkSample, u32 length, u32 micGain);
void vct_process_fir(void* micSample, u32 length, u32 gain);

#endif
