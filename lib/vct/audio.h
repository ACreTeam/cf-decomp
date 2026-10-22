#ifndef VCT_AUDIO_H
#define VCT_AUDIO_H

#include <revolution/os.h>
#include <revolution/types.h>

#include "stream.h"
#include "udp_buffer.h"
#include "vct.h"

#define VCT_HEADER_AUDIO 0x40
#define VCT_HEADER_AUDIO_END 0x41

typedef struct AudioHeader {
    u32 magic;
    u8 header;
    union {
        u8 codec;
        u8 marker;
    };
    u16 sequence;
    u32 timestamp;
} AudioHeader;

BOOL vct_init_audio(u8 codec, u8 frameLength);
void VCTi_MakeAudioParams(u32 frameLength);
void vct_finish_audio(void);
BOOL VCTi_IsAdpcmCodec(int codec);
void VCTi_SetHpfMode(BOOL enable);
u8* vct_prepare_send_buffer(void);
BOOL vct_handle_audio(u8 aid, u8* buffer, u32 size, u64 tick);
int vct_decode_audio_buffer(void);
BOOL vct_flush_send_buffer(void);
void vct_check_quality_alert_time(void);
BOOL vct_recover_drop_using_spare(AudioStream* stream, UDPPacket* node, s16* out, u32 length);
BOOL vct_recover_drop_stream(AudioStream* stream, s16* out, u32 length);
void vct_interpolate_samples(s16* src, s16* dst, u32 samples, u32 interpNum, s16 last);
void vct_recover_drop_copy_tail(s16* prev, s16* out, u32 length);
void vct_recover_drop_pink_noise(s16* prev, s16* out, u32 length);

#endif
