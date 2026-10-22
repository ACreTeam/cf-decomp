#ifndef VCT_G711_H
#define VCT_G711_H

#include <revolution/types.h>

#include "vct.h"

void vct_encode_g711_ulaw(u8* g711, s16* pcm, u32 pcm_samples);
void vct_decode_g711_ulaw(u8* g711, s16* pcm, u32 data_length);
void vct_encode_8bit_raw(u8* encoded, s16* pcm, u32 pcm_samples);
void vct_decode_8bit_raw(u8* encoded, s16* pcm, u32 data_length);

#endif
