#include "stream.h"

#include <revolution/os.h>

#include "nitro_compat.h"
#include "queue.h"
#include "udp_buffer.h"
#include "vct.h"
#include "vct_main.h"

// Parameters derived from the frame length by VCTi_MakeStreamParams.
static int sMaxClockSkew;
static u32 sMaxBufferLatency;
static u32 sThrCountLimit;
static u32 sDefaultJitterBuffer;

void VCTi_MakeStreamParams(u32 frameLength) {
    u32 frameTick = VCTi_GetAudioFrameTick();
    u64 msTick = VCT_MSEC_TO_TICKS(1);

    sThrCountLimit = 1000 / frameLength * 5;
    sMaxBufferLatency = frameTick * 12;
    sMaxClockSkew = (frameLength - 2) * msTick;
    sDefaultJitterBuffer = msTick * 83 + frameTick;

    VCTi_Report("VCTi_MakeStreamParams:%d, %d, %d, %d\n", sMaxClockSkew, sMaxBufferLatency, sThrCountLimit, sDefaultJitterBuffer);
}

void vct_init_stream(AudioStream* stream, u32 channel, int aid) {
    vct_init_stream_info(stream);
    stream->channel = channel;
    stream->aid = aid;
}

void vct_init_stream_info(AudioStream* stream) {
    MI_CpuFill8(&stream->info, 0, sizeof(VCTAudioInfo));
    stream->info.bufferLatency = sDefaultJitterBuffer;
    stream->latency.prev = 0;
    stream->latency.sum = 0;
    stream->jitter = 0;
    stream->diff = 0;
    stream->spike = 0;
    stream->lowThr = 0;
    stream->highThr = sDefaultJitterBuffer;
    stream->thrCount = 0;
    stream->firstRemoteTime = 0;
    stream->firstLocalTime = 0;
    stream->resetSkew = 0;
    stream->skewCount = 0;
    stream->prevTimestamp = 0;
    stream->receiveSequence = 0;
    stream->playSequence = 0;
    stream->initSequence = 1;
    stream->prevBuffer = NULL;
    stream->continuousDrop = 0;
    stream->lossRate = 0;
    stream->frameCount = 0;
    stream->lateCount = 0;
    stream->comfortLevel = 0;
    stream->lastSample = 0;
    stream->interpolate = 0;
}

void vct_reset_stream(AudioStream* stream) {
    if (stream->prevBuffer != NULL) {
        vct_free_packet_buffer(stream->prevBuffer);
        stream->prevBuffer = NULL;
    }

    stream->aid = -1;
}

int vct_save_previous_buffer(AudioStream* stream, UDPPacket* packet) {
    if (stream->prevBuffer != NULL) {
        vct_free_packet_buffer(stream->prevBuffer);
    }

    stream->prevBuffer = packet;
    return vct_pop_audio_queue(stream->channel);
}

static inline void vct_init_sequence(AudioStream* stream, u16 sequence) {
    stream->maxSequence = sequence;
    stream->badSequence = 65537;
    stream->cycles = 0;
    stream->initSequence = 0;
}

BOOL vct_update_sequence(AudioStream* stream, UDPPacket* packet, u16 sequence) {
    u16 udelta = sequence - stream->maxSequence;

    if (udelta < 3000) {
        if (sequence < stream->maxSequence) {
            stream->cycles += 65536;
        }

        stream->maxSequence = sequence;
    } else if (udelta <= 65436) {
        if (sequence == stream->badSequence) {
            VCTi_Report("Jump sequence\n");
            vct_init_sequence(stream, sequence);
            stream->receiveSequence = (sequence - 1) & 0xFFFF;
        } else {
            VCTi_Report("Bad sequence %d\n", udelta);
            stream->badSequence = (sequence + 1) & 0xFFFF;
            return FALSE;
        }
    } else {
        u32 seq = sequence + stream->cycles;

        VCTi_Report("duplicate or mis order %d\n", seq);
        if (seq > stream->receiveSequence + 65436) {
            packet->sequence = seq - 65536;
            return TRUE;
        }
    }

    packet->sequence = sequence + stream->cycles;
    return TRUE;
}

BOOL vct_calc_jitter_and_skew(AudioStream* stream, UDPPacket* packet) {
    u64 remote;
    u64 local;
    int RmS;
    u32 frameTick;
    u64 tick;

    if (stream->initSequence != 0) {
        vct_init_sequence(stream, packet->sequence);
    } else {
        if (!vct_update_sequence(stream, packet, packet->sequence)) {
            stream->info.dropCount++;
            return FALSE;
        }

        stream->info.sequence = packet->sequence;

        if (stream->receiveSequence == packet->sequence) {
            VCTi_Report(" !!!! Duplicate packet (%d) !!!!\n", stream->aid);
            return FALSE;
        }

        if (stream->receiveSequence + 1 != packet->sequence && packet->marker == 0) {
            int d = (int)(packet->sequence - stream->receiveSequence);

            if (d < 0) {
                d = -d;
            }
            if (d > 100) {
                VCTi_Report(" !!! Jump sequence (%5d) !!!\n", stream->receiveSequence);
                stream->receiveSequence = packet->sequence;
                return FALSE;
            }

            VCTi_Report("Skip sequence %d -> %d\n", stream->receiveSequence, packet->sequence);
            if (stream->receiveSequence > packet->sequence) {
                VCTi_Report(" !!!!!! Jam sequence (%d) !!!!!!\n", stream->aid);
                stream->info.jamCount++;
            }
        }
    }

    stream->receiveSequence = packet->sequence;

    if (stream->firstRemoteTime == 0) {
        stream->firstRemoteTime = packet->timestamp;
        stream->firstLocalTime = packet->receive_time;
        stream->resetSkew = 0;
    }

    remote = (u64)(packet->timestamp - stream->firstRemoteTime) << 6;
    local = packet->receive_time - stream->firstLocalTime;
    RmS = (int)(remote - local);
    frameTick = VCTi_GetAudioFrameTick();

    if (stream->diff == 0 || packet->marker != 0) {
        stream->diff = RmS;
    } else {
        int Dn = RmS - stream->diff;

        if (Dn < 0) {
            Dn = -Dn;
        }
        stream->diff = RmS;

        if (Dn > (stream->jitter + frameTick) * 2 + VCT_MSEC_TO_TICKS(100)) {
            VCTi_Report(" !!!! Detect spike jitter  !!!!\n");
            stream->spike = Dn / frameTick + 2;
        }

        if (stream->spike <= 0) {
            stream->jitter += (Dn - stream->jitter) >> 4;
            stream->info.jitter = stream->jitter;
        }
    }

    packet->performance_time = remote + stream->firstLocalTime + stream->info.bufferLatency;

    tick = OSGetTime();
    if (packet->performance_time + frameTick < tick) {
        for (;;) {
            if (!(packet->performance_time + frameTick + 0x3C00000000LL < tick)) {
                break;
            }
            packet->performance_time += 0x4000000000LL;
        }
    }

    if (packet->performance_time + frameTick < tick) {
        VCTi_Report(" !!!! Late buffer (%d:%3lldms) !!!\n", stream->aid, VCT_TICKS_TO_MSEC(tick - (packet->performance_time + frameTick)));
        stream->info.lateCount++;
        stream->lateCount++;
        return FALSE;
    }

    if (packet->performance_time > tick + VCT_TIMER_CLOCK) {
        VCTi_Report(" !! Future buffer (%d:%3dms) !!\n", stream->aid);
        return FALSE;
    }

    if (stream->spike <= 0) {
        int stdev = stream->jitter * 3;
        int div = stdev / (int)frameTick;

        if (stdev > stream->highThr) {
            stream->highThr = sDefaultJitterBuffer + (div + 1) * frameTick;
            stream->lowThr = stream->highThr - frameTick - (int)frameTick / 2;
            stream->info.bufferLatency = sDefaultJitterBuffer + div * frameTick;
            VCTi_Report(" !!! recalc jitter (%3dms) !!!!\n", VCT_TICKS_TO_MSEC(stream->info.bufferLatency));
            VCTi_Report("  (stdev:%3d/Low:%3d/High:%3d)\n", VCT_TICKS_TO_MSEC(stdev), VCT_TICKS_TO_MSEC(stream->lowThr),
                        VCT_TICKS_TO_MSEC(stream->highThr));
            stream->thrCount = 0;
        } else if (stdev < stream->lowThr) {
            if (++stream->thrCount > sThrCountLimit) {
                stream->lowThr = div * frameTick - (int)frameTick / 2;
                stream->highThr = sDefaultJitterBuffer + (div + 1) * frameTick;
                if (stream->lowThr < 0) {
                    stream->lowThr = 0;
                }
                stream->info.bufferLatency = sDefaultJitterBuffer + div * frameTick;
                VCTi_Report(" !!! recalc jitter (%3dms) !!!!\n", VCT_TICKS_TO_MSEC(stream->info.bufferLatency));
                VCTi_Report("  (stdev:%3d/Low:%3d/High:%3d)\n", VCT_TICKS_TO_MSEC(stdev), VCT_TICKS_TO_MSEC(stream->lowThr),
                            VCT_TICKS_TO_MSEC(stream->highThr));
                stream->thrCount = 0;
            }
        }

        if (stream->info.bufferLatency > sMaxBufferLatency) {
            stream->info.bufferLatency = sMaxBufferLatency;
        }

        if (stream->resetSkew < 16) {
            stream->resetSkew++;
            stream->delayEstimate = RmS;
            stream->activeDelay = RmS;
        } else {
            stream->delayEstimate = (RmS + stream->delayEstimate * 31) / 32;
        }

        stream->info.clockSkew = stream->activeDelay - stream->delayEstimate;
        if (stream->info.clockSkew > sMaxClockSkew) {
            stream->info.clockSkew = sMaxClockSkew;
            stream->activeDelay = stream->delayEstimate;
        }
        if (stream->info.clockSkew < -sMaxClockSkew) {
            stream->skewCount++;
            stream->info.clockSkew = -sMaxClockSkew;
            stream->activeDelay = stream->delayEstimate;
        }
    }

    stream->spike--;
    return TRUE;
}
