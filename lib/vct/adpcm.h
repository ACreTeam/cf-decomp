#ifndef VCT_ADPCM_H
#define VCT_ADPCM_H

#include <revolution/types.h>

#include "vct.h"

typedef struct AdpcmState {
    s16 prevSample;
    u8 prevIndex;
} AdpcmState;

void vct_decode_adpcm(u8* adpcm, s16* pcm, u32 data_length, VCTCodec codec);
void vct_encode_adpcm(u8* adpcm, s16* pcm, u32 pcm_samples, AdpcmState* state, VCTCodec codec);

#endif
