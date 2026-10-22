#include "adpcm.h"

#include <limits.h>
#include <revolution/types.h>

#include "vct.h"
#include "vct_main.h"

#define STEP_SIZE_TABLE_LENGTH 89

static const s16 cAdpcmStepSizeTable[STEP_SIZE_TABLE_LENGTH] = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,    19,    21,    23,    25,    28,    31,    34,    37,
    41,    45,    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,   130,   143,   157,   173,   190,   209,
    230,   253,   279,   307,   337,   371,   408,   449,   494,   544,   598,   658,   724,   796,   876,   963,   1060,  1166,
    1282,  1411,  1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
    7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};

static const s8 cAdpcmIndexTable4[] = {-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};

static inline s16 DecodeAdpcm4bit(int code, AdpcmState* state) {
    int index = state->prevIndex;
    int step = cAdpcmStepSizeTable[index];
    int sample = state->prevSample;
    int d = step >> 3;

    if (code & 4) {
        d += step;
    }
    if (code & 2) {
        d += step >> 1;
    }
    if (code & 1) {
        d += step >> 2;
    }

    if (code & 8) {
        sample -= d;
        if (sample < SHRT_MIN) {
            sample = SHRT_MIN;
        }
    } else {
        sample += d;
        if (sample > SHRT_MAX) {
            sample = SHRT_MAX;
        }
    }

    index = index + cAdpcmIndexTable4[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevIndex = index;
    state->prevSample = sample;
    return sample;
}

static inline void vct_decode_adpcm_32(u8* adpcm, s16* pcm, u32 data_length, AdpcmState* state) {
    int i;

    for (i = 0; i < data_length; i++) {
        *pcm++ = DecodeAdpcm4bit(*adpcm & 0xF, state);
        *pcm++ = DecodeAdpcm4bit((*adpcm >> 4) & 0xF, state);
        adpcm++;
    }
}

static inline u8 EncodeAdpcm4bit(int sample, AdpcmState* state) {
    int step;
    int index;
    int d = sample - state->prevSample;
    int code;

    index = state->prevIndex;

    if (d < 0) {
        code = 8;
        d = -d;
    } else {
        code = 0;
    }

    step = cAdpcmStepSizeTable[index];
    if (d >= step) {
        code |= 4;
        d -= step;
    }
    if (d >= step >> 1) {
        code |= 2;
        d -= step >> 1;
    }
    if (d >= step >> 2) {
        code |= 1;
    }

    d = step >> 3;
    if (code & 4) {
        d += step;
    }
    if (code & 2) {
        d += step >> 1;
    }
    if (code & 1) {
        d += step >> 2;
    }

    if (code & 8) {
        d = -d;
    }

    sample = state->prevSample + d;
    if (sample < SHRT_MIN) {
        sample = SHRT_MIN;
    }
    if (sample > SHRT_MAX) {
        sample = SHRT_MAX;
    }

    index += cAdpcmIndexTable4[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevSample = sample;
    state->prevIndex = index;
    return code;
}

static inline void vct_encode_adpcm_32(u8* adpcm, s16* pcm, u32 pcm_samples, AdpcmState* state) {
    u32 i;
    u8 code;

    for (i = 0; i < pcm_samples >> 1; i++) {
        code = EncodeAdpcm4bit(*pcm++, state);
        code |= EncodeAdpcm4bit(*pcm++, state) << 4;
        *adpcm++ = code;
    }
}

static const s8 cAdpcmIndexTable3[] = {-1, -1, 1, 2, -1, -1, 1, 2};

static inline s16 DecodeAdpcm3bit(int code, AdpcmState* state) {
    int index = state->prevIndex;
    int step = cAdpcmStepSizeTable[index];
    int sample = state->prevSample;
    int d = step >> 2;

    if (code & 2) {
        d += step;
    }
    if (code & 1) {
        d += step >> 1;
    }

    if (code & 4) {
        sample -= d;
        if (sample < SHRT_MIN) {
            sample = SHRT_MIN;
        }
    } else {
        sample = sample + d;
        if (sample > SHRT_MAX) {
            sample = SHRT_MAX;
        }
    }

    index = index + cAdpcmIndexTable3[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevIndex = index;
    state->prevSample = sample;
    return sample;
}

static void vct_decode_adpcm_24(u8* adpcm, s16* pcm, u32 data_length, AdpcmState* state) {
    u32 i;

    for (i = 0; i < data_length / 3; i++) {
        u8 d0 = adpcm[0];
        u8 d1 = adpcm[1];
        u8 d2 = adpcm[2];

        pcm[0] = DecodeAdpcm3bit(d0 >> 5, state);
        pcm[1] = DecodeAdpcm3bit((d0 >> 2) & 7, state);
        pcm[2] = DecodeAdpcm3bit(((d0 << 1) & 7) | (d1 >> 7), state);

        pcm[3] = DecodeAdpcm3bit((d1 >> 4) & 7, state);
        pcm[4] = DecodeAdpcm3bit((d1 >> 1) & 7, state);
        pcm[5] = DecodeAdpcm3bit(((d1 << 2) & 7) | (d2 >> 6), state);

        pcm[6] = DecodeAdpcm3bit((d2 >> 3) & 7, state);
        pcm[7] = DecodeAdpcm3bit(d2 & 7, state);

        adpcm += 3;
        pcm += 8;
    }
}

static inline u8 EncodeAdpcm3Bit(int sample, AdpcmState* state) {
    int step;
    int index;
    int d = sample - state->prevSample;
    int code;

    index = state->prevIndex;

    if (d < 0) {
        code = 4;
        d = -d;
    } else {
        code = 0;
    }

    step = cAdpcmStepSizeTable[index];
    if (d >= step) {
        code |= 2;
        d -= step;
    }
    if (d >= step >> 1) {
        code |= 1;
    }

    d = step >> 2;
    if (code & 2) {
        d += step;
    }
    if (code & 1) {
        d += step >> 1;
    }

    if (code & 4) {
        d = -d;
    }

    sample = state->prevSample + d;
    if (sample < SHRT_MIN) {
        sample = SHRT_MIN;
    }
    if (sample > SHRT_MAX) {
        sample = SHRT_MAX;
    }

    index += cAdpcmIndexTable3[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevSample = sample;
    state->prevIndex = index;
    return code;
}

static void vct_encode_adpcm_24(u8* adpcm, s16* pcm, u32 pcm_samples, AdpcmState* state) {
    u32 i;

    for (i = 0; i < pcm_samples >> 3; i++) {
        u8 tmp0;
        u8 tmp1;
        u8 code = EncodeAdpcm3Bit(*pcm++, state) << 5;

        code |= EncodeAdpcm3Bit(*pcm++, state) << 2;
        tmp0 = EncodeAdpcm3Bit(*pcm++, state);
        *adpcm++ = code | (tmp0 >> 1);

        code = EncodeAdpcm3Bit(*pcm++, state) << 4;
        code |= EncodeAdpcm3Bit(*pcm++, state) << 1;
        tmp1 = EncodeAdpcm3Bit(*pcm++, state);
        *adpcm++ = code | (tmp0 << 7) | (tmp1 >> 2);

        code = EncodeAdpcm3Bit(*pcm++, state) << 3;
        code |= EncodeAdpcm3Bit(*pcm++, state);
        *adpcm++ = code | (tmp1 << 6);
    }
}

static const s8 cAdpcmIndexTable2[] = {-1, 1, -1, 1};

static inline s16 DecodeAdpcm2bit(int code, AdpcmState* state) {
    int sample = state->prevSample;
    int index = state->prevIndex;
    int step = cAdpcmStepSizeTable[index];
    int d = step & -(code & 1);

    if (code & 2) {
        sample -= d;
        if (sample < SHRT_MIN) {
            sample = SHRT_MIN;
        }
    } else {
        sample += d;
        if (sample > SHRT_MAX) {
            sample = SHRT_MAX;
        }
    }

    index = index + cAdpcmIndexTable2[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevIndex = index;
    state->prevSample = sample;
    return sample;
}

static inline void vct_decode_adpcm_16(u8* adpcm, s16* pcm, u32 data_length, AdpcmState* state) {
    u32 i;

    for (i = 0; i < data_length; i++) {
        *pcm++ = DecodeAdpcm2bit(*adpcm & 3, state);
        *pcm++ = DecodeAdpcm2bit((*adpcm >> 2) & 3, state);
        *pcm++ = DecodeAdpcm2bit((*adpcm >> 4) & 3, state);
        *pcm++ = DecodeAdpcm2bit((*adpcm >> 6) & 3, state);
        adpcm++;
    }
}

static inline u8 EncodeAdpcm2bit(int sample, AdpcmState* state) {
    int step;
    int index;
    int d = sample - state->prevSample;
    int code;

    index = state->prevIndex;

    if (d < 0) {
        code = 2;
        d = -d;
    } else {
        code = 0;
    }

    step = cAdpcmStepSizeTable[index];
    if (d >= step) {
        code |= 1;
    }

    d = step & -(code & 1);
    if (code & 2) {
        d = -d;
    }

    sample = state->prevSample + d;
    if (sample < SHRT_MIN) {
        sample = SHRT_MIN;
    }
    if (sample > SHRT_MAX) {
        sample = SHRT_MAX;
    }

    index += cAdpcmIndexTable2[code];
    if (index < 0) {
        index = 0;
    } else if (index > STEP_SIZE_TABLE_LENGTH - 1) {
        index = STEP_SIZE_TABLE_LENGTH - 1;
    }

    state->prevSample = sample;
    state->prevIndex = index;
    return code;
}

static inline void vct_encode_adpcm_16(u8* adpcm, s16* pcm, u32 pcm_samples, AdpcmState* state) {
    u32 i;
    u8 code;

    for (i = 0; i < pcm_samples >> 2; i++) {
        code = EncodeAdpcm2bit(*pcm++, state);
        code |= EncodeAdpcm2bit(*pcm++, state) << 2;
        code |= EncodeAdpcm2bit(*pcm++, state) << 4;
        code |= EncodeAdpcm2bit(*pcm++, state) << 6;
        *adpcm++ = code;
    }
}

void vct_decode_adpcm(u8* adpcm, s16* pcm, u32 data_length, VCTCodec codec) {
    AdpcmState state;

    state.prevSample = ((s16*)adpcm)[0];
    state.prevIndex = adpcm[2];

    switch (codec) {
    case VCT_CODEC_2BIT_ADPCM:
        vct_decode_adpcm_16(&adpcm[4], pcm, data_length - 4, &state);
        break;
    case VCT_CODEC_3BIT_ADPCM:
        vct_decode_adpcm_24(&adpcm[4], pcm, data_length - 4, &state);
        break;
    case VCT_CODEC_4BIT_ADPCM:
        vct_decode_adpcm_32(&adpcm[4], pcm, data_length - 4, &state);
        break;
    default:
        VCTi_Report("vct_decode_adpcm:not supported codec:%d\n", codec);
        break;
    }
}

void vct_encode_adpcm(u8* adpcm, s16* pcm, u32 pcm_samples, AdpcmState* state, VCTCodec codec) {
    ((s16*)adpcm)[0] = state->prevSample;
    adpcm[2] = state->prevIndex;
    adpcm[3] = 0;

    switch (codec) {
    case VCT_CODEC_2BIT_ADPCM:
        vct_encode_adpcm_16(&adpcm[4], pcm, pcm_samples, state);
        break;
    case VCT_CODEC_3BIT_ADPCM:
        vct_encode_adpcm_24(&adpcm[4], pcm, pcm_samples, state);
        break;
    case VCT_CODEC_4BIT_ADPCM:
        vct_encode_adpcm_32(&adpcm[4], pcm, pcm_samples, state);
        break;
    default:
        VCTi_Report("vct_encode_adpcm:not supported codec:%d\n", codec);
        break;
    }
}
