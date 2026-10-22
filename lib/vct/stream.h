#ifndef VCT_STREAM_H
#define VCT_STREAM_H

#include <revolution/types.h>

#include "udp_buffer.h"
#include "vct.h"
#include "vct_os.h"

typedef struct VCTAudioInfo {
    /* 0x00 */ u32 internalLatency;
    /* 0x04 */ u32 bufferLatency;
    /* 0x08 */ u32 bufferCount;
    /* 0x0C */ int clockSkew;
    /* 0x10 */ u32 sequence;
    /* 0x14 */ u32 jitter;
    /* 0x18 */ u32 codec;
    /* 0x1C */ u32 dropCount;
    /* 0x20 */ u32 jamCount;
    /* 0x24 */ u32 duplicateCount;
    /* 0x28 */ u32 recoverCount;
    /* 0x2C */ u32 spareRecoverCount;
    /* 0x30 */ u32 lateCount;
    /* 0x34 */ int latencyDiff;
    /* 0x38 */ int latencyMax;
    /* 0x3C */ u32 latencyAverage;
    /* 0x40 */ u32 latencyCount;
} VCTAudioInfo; // size 0x44

typedef struct Equation {
    u32 prev;
    u32 sum;
} Equation;

typedef struct AudioStream {
    /* 0x00 */ VCTAudioInfo info;
    /* 0x44 */ Equation latency;
    /* 0x4C */ u32 comfortLevel;
    /* 0x50 */ u32 channel;
    /* 0x54 */ int aid;
    /* 0x58 */ UDPPacket* prevBuffer;
    /* 0x5C */ u32 continuousDrop;
    /* 0x60 */ u32 lossRate;
    /* 0x64 */ u32 frameCount;
    /* 0x68 */ u32 lateCount;
    /* 0x6C */ u32 _6C;
    /* 0x70 */ u64 firstLocalTime;
    /* 0x78 */ u32 firstRemoteTime;
    /* 0x7C */ u32 prevTimestamp;
    /* 0x80 */ u32 receiveSequence;
    /* 0x84 */ u32 playSequence;
    /* 0x88 */ int initSequence;
    /* 0x8C */ u32 cycles;
    /* 0x90 */ u32 badSequence;
    /* 0x94 */ u16 maxSequence;
    /* 0x98 */ u32 resetSkew;
    /* 0x9C */ int skewCount;
    /* 0xA0 */ int delayEstimate;
    /* 0xA4 */ int activeDelay;
    /* 0xA8 */ int jitter;
    /* 0xAC */ int diff;
    /* 0xB0 */ int spike;
    /* 0xB4 */ int lowThr;
    /* 0xB8 */ s16 lastSample;
    /* 0xBC */ int interpolate;
    /* 0xC0 */ u32 _C0;
    /* 0xC4 */ u32 highThr;
    /* 0xC8 */ u32 thrCount;
    /* 0xCC */ u32 _CC;
} AudioStream; // size 0xD0

void VCTi_MakeStreamParams(u32 frameLength);
void vct_init_stream(AudioStream* stream, u32 channel, int aid);
void vct_init_stream_info(AudioStream* stream);
void vct_reset_stream(AudioStream* stream);
int vct_save_previous_buffer(AudioStream* stream, UDPPacket* packet);
BOOL vct_update_sequence(AudioStream* stream, UDPPacket* packet, u16 sequence);
BOOL vct_calc_jitter_and_skew(AudioStream* stream, UDPPacket* packet);

u32 VCTi_GetAudioFrameTick(void);

#endif
