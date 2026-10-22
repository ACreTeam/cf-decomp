#include <limits.h>
#include <string.h>

#include "fir.h"
#include "g711.h"
#include "mixer.h"
#include "nitro_compat.h"
#include "vct.h"
#include "vct_main.h"

u32 VCTi_GetAudioDataSize(void);

#define MAX_DELAY_LINE 2048

static s16 sImpulseResponse[8] = {-5637, 16445, -14404, 11473, -12246, 892, 2288, -2669};

static int sFilterDelay;
static u32 sCounterIn;
static u32 sCounterOut;
static s16 sDelayLine[MAX_DELAY_LINE];
static int sTap[6];
static int sH[6];
static s16 sSpkBuffer[VCT_MAX_AUDIO_DATA_SIZE / sizeof(s16)];

void VCTi_MakeFilterParams(u32 frameLength) {
    sFilterDelay = frameLength * 24 + 3;
    VCTi_Report("VCTi_MakeFilterParams:%d\n", sFilterDelay);
}

// High-pass filter applied in place; returns the mean energy of the output.
u32 HPFFilter(void* audio_data, u32 length) {
    s16* sample = audio_data;
    u32 i;
    u64 sum;
    int in;
    static int prevIn = 0;
    static int prevOut = 0;

    length /= sizeof(s16);
    in = sample[0] * FX_CONST(0.9138);
    prevOut = (in + prevIn + prevOut * FX_CONST(0.8276)) >> FX_SHIFT;
    sum = prevOut * prevOut;
    prevIn = -in;

    for (i = 1; i < length; i++) {
        in = sample[i] * FX_CONST(0.9138);
        sample[i - 1] = prevOut;
        prevOut = (in + prevIn + prevOut * FX_CONST(0.8276)) >> FX_SHIFT;
        sum += prevOut * prevOut;
        prevIn = -in;
    }
    sample[length - 1] = prevOut;

    return DIV64(sum, length);
}

// Same filter, but only measures the energy and leaves the samples alone.
u32 HPFFilterPower(void* audio_data, u32 length) {
    s16* sample = audio_data;
    u32 i;
    u64 sum = 0;
    int in;
    static int prevOut = 0;
    static int prevIn = 0;

    length /= sizeof(s16);
    for (i = 0; i < length; i++) {
        in = sample[i] * FX_CONST(0.9138);
        prevOut = (in + prevIn + prevOut * FX_CONST(0.8276)) >> FX_SHIFT;
        sum += prevOut * prevOut;
        prevIn = -in;
    }

    return DIV64(sum, length);
}

void InitFIRFilter(void) {
    s16* imp = sImpulseResponse;
    int i;

    sCounterIn = 0;
    sCounterOut = MAX_DELAY_LINE - sFilterDelay;

    for (i = 0; i < MAX_DELAY_LINE; i++) {
        sDelayLine[i] = 0;
    }
    for (i = 0; i < 6; i++) {
        sTap[i] = 0;
        sH[i] = imp[i];
    }
}

void vct_set_speaker_samples(void* spkBuffer, u32 length) {
    if (length != VCTi_GetAudioDataSize()) {
        VCTi_Report("vct_set_speaker_samples:length != VCTi_GetAudioDataSize:%d != %d\n", length, VCTi_GetAudioDataSize());
        return;
    }

    MI_CpuCopyFast(spkBuffer, sSpkBuffer, length);
}

static inline s16 process_sample_unroll(s16 spkSample) {
    int acc = 0;

    sDelayLine[sCounterIn] = spkSample;
    sTap[0] = sDelayLine[sCounterOut];

    acc = sTap[5] * sH[5];
    acc += sTap[4] * sH[4];
    sTap[5] = sTap[4];
    acc += sTap[3] * sH[3];
    sTap[4] = sTap[3];
    acc += sTap[2] * sH[2];
    sTap[3] = sTap[2];
    acc += sTap[1] * sH[1];
    sTap[2] = sTap[1];
    acc += sTap[0] * sH[0];
    sTap[1] = sTap[0];

    acc = acc > SHRT_MAX << Q15_SHIFT ? SHRT_MAX << Q15_SHIFT : (acc < SHRT_MIN << Q15_SHIFT ? SHRT_MIN << Q15_SHIFT : acc);

    sCounterIn = (sCounterIn + 1) % MAX_DELAY_LINE;
    sCounterOut = (sCounterOut + 1) % MAX_DELAY_LINE;

    return acc >> Q15_SHIFT;
}

void DoFIRFilter(void* micSample, void* spkSample, u32 length, u32 micGain) {
    int i;
    u32 spkGain;
    u64 sum = 0;
    s16* mic = micSample;
    s16* spk = spkSample;
    static s16 shift = 0;

    length /= sizeof(s16);
    for (i = 0; i < length; i++) {
        spk[i] = process_sample_unroll(spk[i]);
        sum += spk[i] * spk[i];
        spk[i] = mic[i];
    }

    spkGain = DIV64(sum, length);
    spkGain = SQRT(spkGain);

    if (spkGain * 3 < micGain * 2) {
        shift = 0;
        return;
    }
    if (micGain < spkGain) {
        shift = 4;
    }

    if (shift < 4) {
        shift++;
    }

    for (i = 0; i < length; i++) {
        mic[i] >>= shift;
    }
}

void vct_process_fir(void* micSample, u32 length, u32 gain) {
    DoFIRFilter(micSample, sSpkBuffer, length, gain);
}

static int ulaw_segment[8] = {0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF};

#define BIAS 0x84
#define CLIP 8159
#define SIGN_BIT 0x80
#define QUANT_MASK 0xF
#define SEG_SHIFT 4
#define SEG_MASK 0x70

static inline int search(int val, int* table, int size) {
    int i;

    for (i = 0; i < size; i++) {
        if (val <= *table++) {
            return i;
        }
    }
    return size;
}

// Convert a linear PCM value to u-law (Sun Microsystems reference code).
static inline int linear2ulaw(int pcm_val) {
    int mask;
    int seg;
    int uval;

    pcm_val >>= 2;
    if (pcm_val < 0) {
        pcm_val = -pcm_val;
        mask = 0x7F;
    } else {
        mask = 0xFF;
    }
    if (pcm_val > CLIP) {
        pcm_val = CLIP;
    }
    pcm_val += (BIAS >> 2);

    seg = search(pcm_val, ulaw_segment, 8);

    if (seg >= 8) {
        return 0x7F ^ mask;
    } else {
        uval = (seg << SEG_SHIFT) | ((pcm_val >> (seg + 1)) & QUANT_MASK);
        return uval ^ mask;
    }
}

void vct_encode_g711_ulaw(u8* g711, s16* pcm, u32 pcm_samples) {
    int i;

    for (i = 0; i < pcm_samples; i++) {
        g711[i] = linear2ulaw(pcm[i]);
    }
}

// Convert a u-law value to 16-bit linear PCM.
static inline int ulaw2linear(int u_val) {
    int t;

    u_val = ~u_val;

    t = ((u_val & QUANT_MASK) << 3) + BIAS;
    t <<= (u_val & SEG_MASK) >> SEG_SHIFT;

    return (u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS);
}

void vct_decode_g711_ulaw(u8* g711, s16* pcm, u32 data_length) {
    int i;

    for (i = 0; i < data_length; i++) {
        pcm[i] = ulaw2linear(g711[i]);
    }
}

void vct_encode_8bit_raw(u8* encoded, s16* pcm, u32 pcm_samples) {
    s8* out = (s8*)encoded;
    int i;

    for (i = 0; i < pcm_samples; i++) {
        out[i] = pcm[i] >> 8;
    }
}

void vct_decode_8bit_raw(u8* encoded, s16* pcm, u32 data_length) {
    s8* in = (s8*)encoded;
    int i;

    for (i = 0; i < data_length; i++) {
        pcm[i] = in[i] << 8;
    }
}

u32 MATH_CountLeadingZeros(u32 x) {
    u32 y;
    u32 n = 32;

    y = x >> 16;
    if (y != 0) {
        n = n - 16;
        x = y;
    }
    y = x >> 8;
    if (y != 0) {
        n = n - 8;
        x = y;
    }
    y = x >> 4;
    if (y != 0) {
        n = n - 4;
        x = y;
    }
    y = x >> 2;
    if (y != 0) {
        n = n - 2;
        x = y;
    }
    y = x >> 1;
    return y != 0 ? n - 2 : n - x;
}

u32 MATH_CountPopulation(u32 x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0xFF;
}

void vct_mix_audio(s16* left, s16* right, s16* out, u32 length) {
    int i;
    s32 s;

    for (i = 0; i < length / sizeof(s16); i += 4) {
        s = left[i + 0] + right[i + 0];
        out[i + 0] = s > SHRT_MAX ? SHRT_MAX : (s < SHRT_MIN ? SHRT_MIN : s);
        s = left[i + 1] + right[i + 1];
        out[i + 1] = s > SHRT_MAX ? SHRT_MAX : (s < SHRT_MIN ? SHRT_MIN : s);
        s = left[i + 2] + right[i + 2];
        out[i + 2] = s > SHRT_MAX ? SHRT_MAX : (s < SHRT_MIN ? SHRT_MIN : s);
        s = left[i + 3] + right[i + 3];
        out[i + 3] = s > SHRT_MAX ? SHRT_MAX : (s < SHRT_MIN ? SHRT_MIN : s);
    }
}

void MI_CpuFill32(u32 data, void* dest, u32 size) {
    u32* d = dest;
    u32 i;

    for (i = 0; i < size / sizeof(u32); i++) {
        d[i] = data;
    }
}

void MI_CpuCopyFast(const void* src, void* dest, u32 size) {
    memcpy(dest, src, size);
}

void MI_CpuCopy8(const void* src, void* dest, u32 size) {
    memcpy(dest, src, size);
}
