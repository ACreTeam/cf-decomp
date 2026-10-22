#include "audio.h"

#include <dwc/dwc_transport.h>
#include <revolution/os.h>
#include <string.h>

#include "adpcm.h"
#include "decoder.h"
#include "fir.h"
#include "g711.h"
#include "mixer.h"
#include "nitro_compat.h"
#include "queue.h"
#include "stream.h"
#include "udp_buffer.h"
#include "vad.h"
#include "vct.h"
#include "vct_main.h"
#include "vct_os.h"

#define VCT_SEND_BUFFER_SIZE 0xC20
#define VCT_SPARE_BUFFER_SIZE 0x250
#define VCT_DROP_BUFFER_SIZE 0xC00
#define VCT_AUDIO_HEADER_SIZE 0xC
#define VCT_MAX_SEND_PACKET_SIZE 0x4C0

typedef struct VCTSendInfo {
    u8 buffer[VCT_SEND_BUFFER_SIZE * 2];
    u32 read;
    u32 write;
    u8* ptr;
    u32 bitmap;
    u32 balance;
    int numClients;
} VCTSendInfo;

typedef struct VCTCodecInfo {
    u8 chunk_size;
    u8 header_size;
} VCTCodecInfo;

typedef enum {
    QUALITY_ALERT_STATUS_NONE = 0,
    QUALITY_ALERT_STATUS_ALERT,
    QUALITY_ALERT_STATUS_CLEARING
} QualityAlertStatus;

// Pink noise period used by the noise based drop recovery (generated table).
#define PINK_NOISE_LENGTH 0x600

static const VCTCodecInfo sCodecInfo[5] = {{8, 0}, {8, 0}, {2, 4}, {3, 4}, {4, 4}};

static const u8 sTransceiverBalance[VCT_MAX_TRANSCEIVER_CLIENT - 1][3] = {
    {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 1, 2}, {1, 2, 2}, {2, 2, 2}, {2, 2, 3},
};

static u16 sAudioSequence;
static u32 sCodec;
static u32 sSpareCodec;
static AdpcmState sAdpcmState;
static AdpcmState sSpareAdpcmState;
static BOOL sEnableEchoCancel;
static u8 sQualityAlertStatus;
static u64 sQualityAlertTime;
static BOOL sEnableQualityAlert;
static u64 sInitTime;
static u32 sLateTime;
static u32 sMaxPacketLength;
static u32 sAudioFrameTick;
static u32 sSparePacketSize;
static u32 sAudioPacketSize;
static BOOL sHpfMode;
static VCTVADStatus sVADStatus;
static BOOL sMarker;
static BOOL sChangeVAD;
static u32 sBitmap;
static u32 sNumStream;

static u8 sSpareBuffer[VCT_SPARE_BUFFER_SIZE];
static VCTSendInfo sSendInfo ATTRIBUTE_ALIGN(8);
static AudioStream sStream[VCT_MAX_AUDIO_STREAM];
static VCTSession* sAudioSession[VCT_MAX_AUDIO_STREAM];
static MATHRandContext16 sRandContext;
static s16 sMixBuffer[VCT_MAX_AUDIO_DATA_SIZE / sizeof(s16)];
static u8 sSendWork[VCT_SEND_BUFFER_SIZE];
static u8 sSpareWork[VCT_SEND_BUFFER_SIZE];
static s16 sDropBuffer[VCT_MAX_AUDIO_STREAM][VCT_DROP_BUFFER_SIZE / sizeof(s16)];

static u16 sQualityAlertFrameNum = 60;
static u8 sQualityAlertThreshold = 20;
static BOOL sEnableVAD = TRUE;
static int sAudioFrameLength = 0x44;
static int sAudioDataSize = 0x440;
static u8 sContinuousDropLimit = 4;
static u32 sFadeOutSampleNum = 0x480;
static u32 sMaxNoiseLevel = 0x960;
static u8 sDropRecoverMode = 4;
static u8 sDropRecoverCopyTailFrameLength = 0x20;
static s16 sNoiseLevelPercent = 100;
static u8 sInterpolateSampleNum = 16;
static u32 sQualityAlertClearTime = 0x708;

void VCTi_MakeFilterParams(u32 frameLength);

static inline void vct_reset_send_info(void) {
    sSendInfo.read = 0;
    sSendInfo.write = 0;
    sSendInfo.bitmap = 0;
    sSendInfo.ptr = 0;
    sSendInfo.balance = 0;
    sSendInfo.numClients = 0;
}

void VCTi_MakeAudioParams(u32 frameLength) {
    sAudioFrameLength = frameLength;
    sAudioDataSize = frameLength * 16;
    sAudioPacketSize = sCodecInfo[sCodec].chunk_size * frameLength + sCodecInfo[sCodec].header_size + VCT_AUDIO_HEADER_SIZE;
    sSparePacketSize = frameLength * sCodecInfo[sSpareCodec].chunk_size + sCodecInfo[sSpareCodec].header_size + VCT_AUDIO_HEADER_SIZE;
    sMaxPacketLength = frameLength * 8 + VCT_AUDIO_HEADER_SIZE;
    sAudioFrameTick = frameLength * (VCT_TIMER_CLOCK / 1000);
    sLateTime = VCT_USEC_TO_TICKS(100) + sAudioFrameTick;

    MI_CpuFill8(sSpareBuffer, 0, 0x250);
    VCTi_MakeStreamParams(sAudioFrameLength);
    VCTi_MakeFilterParams(sAudioFrameLength);
    vct_reset_send_info();
    vct_init_vad();

    {
        int i;

        for (i = 0; i < VCT_MAX_AUDIO_STREAM; i++) {
            vct_init_audio_queue(i);
            vct_init_stream_info(&sStream[i]);
        }
    }

    vct_init_decoder();
    InitFIRFilter();
    vct_reset_packet_buffer();
    VCTi_Report("VCTi_MakeAudioParams:codec:%d, frame:%d, data:%d, packet:%d, spare:%d, frame tick:%d, max length:%d, late time:%d, free buffer num:%d\n",
                sCodec, sAudioFrameLength, sAudioDataSize, sAudioPacketSize, sSparePacketSize, sAudioFrameTick, sMaxPacketLength, sLateTime,
                vct_count_free_packet_buffer());
}

static u64 OS_MicroSecondsToTicks(u32 us) {
    return VCT_USEC_TO_TICKS(us);
}

BOOL vct_init_audio(u8 codec, u8 frameLength) {
    int i;

    sAudioSequence = OSGetTick();
    sNumStream = 0;
    sBitmap = 0;
    sEnableVAD = TRUE;
    sHpfMode = FALSE;
    sCodec = codec;
    sSpareCodec = 2;

    for (i = 0; i < VCT_MAX_AUDIO_STREAM; i++) {
        sAudioSession[i] = NULL;
        vct_init_stream(&sStream[i], i, -1);
    }

    MATH_InitRand16(&sRandContext, (u16)OSGetTick());

    VCTi_MakeAudioParams(frameLength);

    sStream[0].info.dropCount = 0;
    sStream[1].info.dropCount = 0;
    sStream[2].info.dropCount = 0;
    sInitTime = OSGetTime();
    return TRUE;
}

void vct_finish_audio(void) {
    sNumStream = 0;
}

BOOL VCT_StartStreaming(VCTSession* session) {
    u32 ch;

    if (session == NULL) {
        return FALSE;
    }
    if (sNumStream == VCT_MAX_AUDIO_STREAM) {
        VCTi_Report("VCT_StartStreaming: Conference clients are exceeding the upper bound.\n");
        return FALSE;
    }
    if (session->state != VCT_STATE_TALKING && session->state != VCT_STATE_CONNECTED) {
        return FALSE;
    }

    for (ch = 0; ch < VCT_MAX_AUDIO_STREAM; ch++) {
        if (sAudioSession[ch] == session) {
            VCTi_Report("This session already start straeming.\n");
            return TRUE;
        }
    }

    for (ch = 0; ch < VCT_MAX_AUDIO_STREAM; ch++) {
        if (sAudioSession[ch] == NULL) {
            sAudioSession[ch] = session;
            break;
        }
    }

    sNumStream++;
    sBitmap |= session->aidBitmap;
    vct_init_stream(&sStream[ch], ch, session->talking);
    vct_init_audio_queue(ch);

    if (session->state != VCT_STATE_TALKING) {
        return TRUE;
    }

    if (sNumStream == 1) {
        VCT_ResetVAD();
        sAudioSequence = OSGetTick();
        sAdpcmState.prevSample = 0;
        sAdpcmState.prevIndex = 0;
        sSpareAdpcmState.prevSample = 0;
        sSpareAdpcmState.prevIndex = 0;
        sMarker = TRUE;
        vct_reset_send_info();
    }

    return TRUE;
}

void VCT_StopStreaming(VCTSession* session) {
    u32 ch;

    for (ch = 0; ch < VCT_MAX_AUDIO_STREAM; ch++) {
        if (sAudioSession[ch] == session) {
            sAudioSession[ch] = NULL;
            sNumStream--;
            vct_reset_stream(&sStream[ch]);
            vct_flush_audio_queue(ch);
            vct_flush_decoder(ch);
            sBitmap &= ~session->aidBitmap;
            break;
        }
    }

    if (sNumStream != 0) {
        return;
    }

    sMarker = TRUE;
    VCT_ResetVAD();
    sBitmap = 0;
}

BOOL VCT_SendAudio(void* audio_data, u32 length) {
    AudioHeader* header;
    u32 ch;
    BOOL flag = FALSE;
    u8* ptr;

    if (!VCTi_IsInitialized()) {
        return FALSE;
    }
    if (length != sAudioDataSize) {
        VCTi_Report("VCT_SendAudio:Invalid audio data length:%d (length must be set %d)\n", length, sAudioDataSize);
        return FALSE;
    }
    if (sNumStream == 0) {
        return FALSE;
    }

    for (ch = 0; ch < VCT_MAX_AUDIO_STREAM; ch++) {
        if (sAudioSession[ch] != NULL && sAudioSession[ch]->state == VCT_STATE_TALKING) {
            flag = TRUE;
            break;
        }
    }

    if (!flag) {
        return FALSE;
    }

    ptr = &sSendInfo.buffer[(sSendInfo.write & 1) ? 0 : VCT_SEND_BUFFER_SIZE];

    header = (AudioHeader*)ptr;
    header->magic = VCT_MAGIC_PACKET_HEADER;

    if (sCodec >= VCT_CODEC_2BIT_ADPCM) {
        MI_CpuCopyFast(audio_data, ptr + sizeof(AudioHeader) + sizeof(AdpcmState), length);
    } else {
        MI_CpuCopyFast(audio_data, ptr + sizeof(AudioHeader), length);
    }

    header->header = VCT_HEADER_AUDIO;
    header->sequence = sAudioSequence++;
    header->timestamp = (u64)OSGetTime() / 64;
    sSendInfo.write++;
    return TRUE;
}

static void vct_recover_drop(void* previous, void* missing, u32 length) {
    MI_CpuCopyFast(previous, missing, length);
}

static u64 vct_get_tick_audio(void) {
    return OSGetTime();
}



static u64 OS_MicroSecondsToTicks(u32 us);
static s64 OS_TicksToMilliSeconds(s64 ticks);
static u32 calc_equation(Equation* eq, u32 sample);

static BOOL VCTi_ReceiveAudioData(void* audio_data, u32 length, u32 channel, u32* outAID) {
    UDPPacket* node;
    AudioStream* stream = &sStream[channel];
    BOOL calc_skew = TRUE;
    u32 aid = stream->aid & 0xFF;
    int diff = vct_count_audio_queue(channel);

    while (diff != 0) {
        u64 tick = vct_get_tick_audio();
        u64 perf;

        node = vct_top_audio_queue(channel);
        if (node == NULL) {
            VCTi_Report("!!!!! Node is null (%d) !!!!!\n", diff);
            stream->info.bufferCount = diff;
            return FALSE;
        }

        stream->info.latencyDiff = (u32)tick - (u32)node->performance_time;
        if (stream->info.latencyDiff > stream->info.latencyMax) {
            stream->info.latencyMax = stream->info.latencyDiff;
        }
        stream->info.latencyAverage = (stream->info.latencyAverage * stream->info.latencyCount + stream->info.latencyDiff) / (stream->info.latencyCount + 1);
        stream->info.latencyCount++;

        if (stream->playSequence + 1 != node->sequence && stream->playSequence != 0 && node->marker == 0) {
            if (stream->playSequence == node->sequence) {
                VCTi_Report("Duplicated sequence:aid:%d, %d -> %d\n", aid, stream->playSequence, node->sequence);
                stream->info.duplicateCount++;
            } else if (stream->playSequence < node->sequence) {
                VCTi_Report("Drop sequence:aid:%d, %d -> %d\n", aid, stream->playSequence, node->sequence);
                stream->info.dropCount++;
            } else {
                VCTi_Report("Jam sequence:aid:%d, %d -> %d\n", aid, stream->playSequence, node->sequence);
                stream->info.jamCount++;
            }
        }

        perf = node->performance_time;
        if (perf > tick + OS_MicroSecondsToTicks(100)) {
            VCTi_Report(" **  Wait for buffer (%3dms) **\n", OS_TicksToMilliSeconds(perf - tick));
            if (node->marker == 0) {
                if (vct_recover_drop_using_spare(stream, node, audio_data, length)) {
                    if (outAID != NULL) {
                        *outAID |= 1 << aid;
                    }
                    if (stream->prevBuffer != NULL) {
                        vct_free_packet_buffer(stream->prevBuffer);
                        stream->prevBuffer = NULL;
                    }
                    stream->continuousDrop = 0;
                    return TRUE;
                }
                if (vct_recover_drop_stream(stream, audio_data, length)) {
                    if (outAID != NULL) {
                        *outAID |= 1 << aid;
                    }
                    return TRUE;
                }
                return FALSE;
            }
            return FALSE;
        }

        if (perf + sLateTime < tick) {
            VCTi_Report(" * Drop late buffer (%3dms)   *\n", OS_TicksToMilliSeconds(tick - perf));
            stream->info.lateCount++;
            stream->lateCount++;
            stream->playSequence = node->sequence;
            diff = vct_save_previous_buffer(stream, node);
            vct_top_audio_queue(channel);
            calc_skew = FALSE;
            continue;
        }

        stream->playSequence = node->sequence;

        if (calc_skew && stream->skewCount != 0) {
            VCTi_Report(" ****  Drop by clock skew  ****\n");
            diff = vct_save_previous_buffer(stream, node);
            vct_top_audio_queue(channel);
            stream->skewCount--;
            continue;
        }

        stream->info.internalLatency = calc_equation(&stream->latency, (u32)tick - (u32)node->receive_time);
        vct_recover_drop(node->samples, audio_data, length);
        vct_save_previous_buffer(stream, node);

        if (stream->interpolate != 0) {
            vct_interpolate_samples(audio_data, audio_data, length / sizeof(s16), sInterpolateSampleNum, stream->lastSample);
            stream->interpolate = 0;
        }

        if (outAID != NULL) {
            *outAID |= 1 << node->remote;
        }

        stream->continuousDrop = 0;
        return TRUE;
    }

    if (vct_recover_drop_stream(stream, audio_data, length)) {
        if (outAID != NULL) {
            *outAID |= 1 << aid;
        }
        return TRUE;
    }

    return FALSE;
}

static s64 OS_TicksToMilliSeconds(s64 ticks) {
    return VCT_TICKS_TO_MSEC(ticks);
}

static u32 calc_equation(Equation* eq, u32 sample) {
    u32 D = sample - eq->prev;
    u32 ret;

    eq->prev = sample;
    ret = eq->prev + ((int)(D - eq->prev) >> 4);
    eq->sum = ret;
    return ret;
}

BOOL VCT_ReceiveAudio(void* audio_data, u32 length, u32* outAID) {
    u32 i;
    BOOL flag = FALSE;

    if (!VCTi_IsInitialized()) {
        return FALSE;
    }

    if (outAID != NULL) {
        *outAID = 0;
    }

    if (length != sAudioDataSize) {
        VCTi_Report("vct_receive_mixed_audio:Invalid audio data length:%d (length must be set %d)\n", length, sAudioDataSize);
        MI_CpuFill32(0, audio_data, length);
    } else if (sNumStream != 0) {
        for (i = 0; i < VCT_MAX_AUDIO_STREAM; i++) {
            if (VCTi_ReceiveAudioData(audio_data, length, i, outAID) == TRUE) {
                flag = TRUE;
                break;
            }
        }

        if (flag) {
            i++;
            if (i == VCT_MAX_AUDIO_STREAM) {
                flag = TRUE;
            } else {
                u32 j;

                for (j = i; j < VCT_MAX_AUDIO_STREAM; j++) {
                    if (VCTi_ReceiveAudioData(sMixBuffer, length, j, outAID) == TRUE) {
                        vct_mix_audio(audio_data, sMixBuffer, audio_data, length);
                    }
                }
            }
        } else {
            MI_CpuFill32(0, audio_data, length);
        }
    } else {
        MI_CpuFill32(0, audio_data, length);
        flag = FALSE;
    }

    if (sEnableEchoCancel) {
        vct_set_speaker_samples(audio_data, length);
    }

    return flag;
}

u8* vct_prepare_send_buffer(void) {
    u8* ptr;
    u8* audio_data;
    AudioHeader* header;
    u32 micScale;
    u32 micGain;
    u32 codec = sCodec;
    int intr;
    u32 diff = sSendInfo.write - sSendInfo.read;

    if (diff > 1) {
        sSendInfo.read = sSendInfo.write - 1;
        diff = 1;
    }

    if (diff == 0) {
        return NULL;
    }

    ptr = &sSendInfo.buffer[(sSendInfo.read & 1) ? 0 : VCT_SEND_BUFFER_SIZE];

    intr = OSDisableInterrupts();
    MI_CpuCopyFast(ptr, sSendWork, VCT_SEND_BUFFER_SIZE);
    header = (AudioHeader*)sSendWork;
    OSRestoreInterrupts(intr);

    audio_data = (u8*)header + sizeof(AudioHeader);
    if (codec >= VCT_CODEC_2BIT_ADPCM) {
        audio_data += sizeof(AdpcmState);
    }

    if (sHpfMode) {
        micScale = HPFFilter(audio_data, sAudioDataSize);
    } else {
        micScale = HPFFilterPower(audio_data, sAudioDataSize);
    }
    micGain = SQRT(micScale);

    if (sEnableEchoCancel) {
        vct_process_fir(audio_data, sAudioDataSize, micGain);
    }

    header->codec = codec;

    if (sEnableVAD) {
        sVADStatus = VCTi_GetVADStatus(micScale, micGain, 0);
        switch (sVADStatus) {
        case VCT_VAD_LOST:
            header->header = VCT_HEADER_AUDIO_END;
            break;
        case VCT_VAD_NONE:
            sSendInfo.read++;
            return NULL;
        case VCT_VAD_DETECT:
            header->marker |= 0x80;
            break;
        }
    }

    if (!sEnableVAD && sChangeVAD) {
        sChangeVAD = FALSE;
        header->marker |= 0x80;
    }

    if (sMarker) {
        sMarker = FALSE;
        header->marker |= 0x80;
    }

    if (codec >= VCT_CODEC_2BIT_ADPCM) {
        if (header->marker & 0x80) {
            sAdpcmState.prevSample = 0;
            sAdpcmState.prevIndex = 0;
            sSpareAdpcmState.prevSample = 0;
            sSpareAdpcmState.prevIndex = 0;
        }

        if (sEnableQualityAlert && sQualityAlertStatus == QUALITY_ALERT_STATUS_NONE) {
            AudioHeader* spareHeader;

            header->marker |= 0x40;
            MI_CpuCopyFast(header, sSpareWork, VCT_SEND_BUFFER_SIZE);
            vct_encode_adpcm(audio_data - sizeof(AdpcmState), (s16*)audio_data, sAudioDataSize / sizeof(s16), &sAdpcmState, codec);
            MI_CpuCopy8(sSpareBuffer, (u8*)header + sAudioPacketSize, sSparePacketSize);

            spareHeader = (AudioHeader*)sSpareWork;
            spareHeader->marker = (spareHeader->marker & 0xF0) | (sSpareCodec & 0xF);
            vct_encode_adpcm(sSpareWork + sizeof(AudioHeader), (s16*)(sSpareWork + sizeof(AudioHeader) + sizeof(AdpcmState)),
                             sAudioDataSize / sizeof(s16), &sSpareAdpcmState, sSpareCodec);
            MI_CpuCopy8(sSpareWork, sSpareBuffer, sSparePacketSize);
        } else {
            vct_encode_adpcm(audio_data - sizeof(AdpcmState), (s16*)audio_data, sAudioDataSize / sizeof(s16), &sAdpcmState, codec);
        }
    } else if (codec == VCT_CODEC_G711_ULAW) {
        vct_encode_g711_ulaw(audio_data, (s16*)audio_data, sAudioDataSize / sizeof(s16));
    } else if (codec == VCT_CODEC_8BIT_RAW) {
        vct_encode_8bit_raw(audio_data, (s16*)audio_data, sAudioDataSize / sizeof(s16));
    }

    sSendInfo.bitmap = sBitmap & ~(1 << vct.myAID);
    sSendInfo.numClients = (u8)MATH_CountPopulation(sSendInfo.bitmap) - 1;

    if (sSendInfo.numClients > VCT_MAX_TRANSCEIVER_CLIENT - 1) {
        VCTi_Report("Too many transceiver clients\n");
        return NULL;
    }

    sSendInfo.balance = 0;
    return (u8*)header;
}

BOOL vct_handle_audio(u8 aid, u8* buffer, u32 size, u64 tick) {
    UDPPacket* p;
    AudioStream* stream = NULL;
    u8* spare = NULL;
    u32 mainDataSize = 0;
    u32 spareSize = 0;
    u32 codec;
    u32 packet_size;
    u32 aidBit;
    u8* tmp_buffer;
    u32 ch;

    if (sNumStream == 0) {
        VCTi_Report("vct_handle_audio: Discard packet from aid %d (Not streaming now)\n", aid);
        return FALSE;
    }

    if (size > sMaxPacketLength) {
        VCTi_Report("vct_handle_audio: Discard packet by bad length (%dbyte)\n", size);
    }

    for (ch = 0; ch < VCT_MAX_AUDIO_STREAM; ch++) {
        if (aid == sStream[ch].aid) {
            stream = &sStream[ch];
            break;
        }
    }

    if (stream == NULL) {
        VCTi_Report("vct_handle_audio: Discard packet from aid %d (This aid doesn't streaming)\n", aid);
        return FALSE;
    }

    if (buffer[4] == VCT_HEADER_AUDIO_END) {
        VCTi_Report("vct_handle_audio: Reset clock skew and set comfort noise level\n");
        stream->comfortLevel = *(u16*)(buffer + 6);
        stream->firstRemoteTime = 0;
        stream->resetSkew = 1;
        return FALSE;
    } else if (buffer[4] != VCT_HEADER_AUDIO) {
        return FALSE;
    }

    codec = buffer[5] & 0xF;
    if (!VCTi_IsAdpcmCodec(codec)) {
        VCTi_Report("vct_handle_audio: Discard packet by invalid codec %x\n", buffer[5]);
        return FALSE;
    }

    stream->info.codec = codec;
    packet_size = sAudioFrameLength * sCodecInfo[codec].chunk_size + sCodecInfo[codec].header_size + VCT_AUDIO_HEADER_SIZE;

    if (buffer[5] & 0x40) {
        u32 spareCodec;

        spare = buffer + packet_size;
        spareCodec = spare[5] & 0xF;
        spare[5] = spareCodec;
        if (!(spareCodec >= VCT_CODEC_2BIT_ADPCM && spareCodec <= VCT_CODEC_3BIT_ADPCM)) {
            VCTi_Report("vct_handle_audio: Discard packet by invalid spare codec %x\n", spareCodec);
            return FALSE;
        }

        mainDataSize = packet_size - VCT_AUDIO_HEADER_SIZE;
        spareSize = sAudioFrameLength * sCodecInfo[spareCodec].chunk_size + sCodecInfo[spareCodec].header_size + VCT_AUDIO_HEADER_SIZE;
        packet_size += spareSize;
    }

    if (size != packet_size) {
        VCTi_Report("vct_handle_audio: Discard packet by invalid length (%d != %d)\n", size, packet_size);
        return FALSE;
    }

    if (buffer[5] & 0x20) {
        sQualityAlertStatus = QUALITY_ALERT_STATUS_ALERT;
    } else if (sQualityAlertStatus == QUALITY_ALERT_STATUS_ALERT) {
        sQualityAlertTime = OSGetTime();
        sQualityAlertStatus = QUALITY_ALERT_STATUS_CLEARING;
        VCTi_Report("vct_handle_audio:QUALITY_ALERT_STATUS_CLEARING_ALERT\n");
    }

    aidBit = aid != 0 ? 1 << aid : 1;
    if (!(sBitmap & aidBit)) {
        VCTi_Report("vct_handle_audio: Discard packet by invalid remote aid: %d\n", aid);
        return FALSE;
    }

    p = vct_alloc_packet_buffer();
    if (p == NULL) {
        VCTi_Report("vct_handle_audio: Can't allocate audio buffer; flush buffer\n");
        vct_flush_audio_queue(stream->channel);
        vct_flush_decoder(stream->channel);
        p = vct_alloc_packet_buffer();
        if (p == NULL) {
            vct_flush_all_audio_queue();
            vct_flush_all_decoder();
            p = vct_alloc_packet_buffer();
        }
        if (p == NULL) {
            VCTi_Report("vct_handle_audio: Can't allocate audio buffer!!!\n");
            return FALSE;
        }
    }

    if (buffer[5] & 0x40) {
        tmp_buffer = &p->buffer[VCT_DROP_BUFFER_SIZE] - mainDataSize;
    } else {
        tmp_buffer = &p->buffer[VCT_DROP_BUFFER_SIZE] - (size - VCT_AUDIO_HEADER_SIZE);
    }

    MI_CpuCopy8(buffer + VCT_AUDIO_HEADER_SIZE, tmp_buffer, size - VCT_AUDIO_HEADER_SIZE);

    p->channel = stream->channel;
    p->codec = codec;
    p->samples = tmp_buffer;
    p->flags = buffer[5];
    if (buffer[5] & 0x40) {
        p->length = mainDataSize;
        MI_CpuCopy8(spare, &p->buffer[VCT_DROP_BUFFER_SIZE], spareSize);
    } else {
        p->length = size - VCT_AUDIO_HEADER_SIZE;
    }
    p->receive_time = tick;
    p->remote = aid;
    p->marker = buffer[5] & 0x80;
    p->timestamp = *(u32*)(buffer + 8);
    p->sequence = *(u16*)(buffer + 6);

    stream->frameCount++;
    if (sQualityAlertFrameNum != 0 && stream->frameCount >= sQualityAlertFrameNum) {
        stream->lossRate = stream->lateCount * 100 / sQualityAlertFrameNum;
        stream->frameCount = 0;
        stream->lateCount = 0;
    }

    if (!vct_calc_jitter_and_skew(stream, p)) {
        vct_free_packet_buffer(p);
        return FALSE;
    }

    vct_insert_decoder(p);
    return TRUE;
}

int vct_decode_audio_buffer(void) {
    UDPPacket* p;
    int i;
    int n;
    u32 ch;
    u32 remote;
    int intr;

    p = vct_top_decoder();
    if (p == NULL) {
        return 0;
    }

    intr = OSDisableInterrupts();
    ch = p->channel;
    n = vct_count_decoder_queue(ch);
    for (i = 0; i < n; i++) {
        p = vct_get_packet_from_decoder(ch);
        remote = p->remote;
        if (p->codec >= VCT_CODEC_2BIT_ADPCM) {
            vct_decode_adpcm(p->samples, (s16*)p->buffer, p->length, p->codec);
        } else if (p->codec == VCT_CODEC_G711_ULAW) {
            vct_decode_g711_ulaw(p->samples, (s16*)p->buffer, p->length);
        } else {
            vct_decode_8bit_raw(p->samples, (s16*)p->buffer, p->length);
        }

        p->samples = p->buffer;
        vct_remove_packet_from_decoder(p);
        if (vct_insert_audio_queue(p, p->channel) < 0) {
            VCTi_Report("!!!!! Audio buffer is full !!!!!\n");
            vct_free_packet_buffer(p);
        }
    }

    OSRestoreInterrupts(intr);
    if (n > 1) {
        VCTi_Report("Decode %d buffer in stream %d\n", n, remote);
    }
    return n;
}

BOOL vct_flush_send_buffer(void) {
    int loop;
    int i;
    u32 packetSize;
    u8 packet[VCT_MAX_SEND_PACKET_SIZE];

    if (sNumStream == 0) {
        return TRUE;
    }

    if (sSendInfo.ptr == NULL) {
        sSendInfo.ptr = vct_prepare_send_buffer();
        if (sSendInfo.ptr != NULL && sEnableEchoCancel) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    loop = 1;
    if (vct.SSPMode == VCT_MODE_TRANSCEIVER) {
        loop = sTransceiverBalance[sSendInfo.numClients][sSendInfo.balance++];
    }

    if (((AudioHeader*)sSendInfo.ptr)->marker & 0x40) {
        packetSize = sAudioPacketSize + sSparePacketSize;
    } else {
        packetSize = sAudioPacketSize;
    }

    for (i = 0; i < loop; i++) {
        AudioStream* stream;
        u32 aid = MATH_CountLeadingZeros(sSendInfo.bitmap);

        if (aid == 32) {
            break;
        }
        sSendInfo.bitmap &= ~(0x80000000 >> aid);
        aid = 31 - aid;

        if (aid == sStream[0].aid) {
            stream = &sStream[0];
        } else if (aid == sStream[1].aid) {
            stream = &sStream[1];
        } else if (aid == sStream[2].aid) {
            stream = &sStream[2];
        } else {
            stream = NULL;
        }

        if (stream != NULL && sQualityAlertThreshold != 0 && stream->lossRate >= sQualityAlertThreshold) {
            ((AudioHeader*)sSendInfo.ptr)->marker |= 0x20;
        } else {
            ((AudioHeader*)sSendInfo.ptr)->marker &= ~0x20;
        }

        if (packetSize + 0x10 > VCT_MAX_SEND_PACKET_SIZE) {
            OSPanic("audio.c", 1274, "buffer size is too small!");
        }

        memcpy(packet, sSendInfo.ptr, packetSize);
        vct_add_hmac(packet + packetSize, packet, packetSize);

        if (!DWC_SendUnreliable(aid, packet, packetSize + 0x10)) {
            VCTi_Report("Failed to send audio packet to aid%d\n", aid);
        }

        if (sSendInfo.bitmap == 0) {
            sSendInfo.read++;
            sSendInfo.ptr = NULL;
            sSendInfo.bitmap = 0;
            break;
        }
    }

    return TRUE;
}

BOOL vct_recover_drop_using_spare(AudioStream* stream, UDPPacket* node, s16* out, u32 length) {
    u8* samples;
    u32 dataSize;

    if (!(node->flags & 0x40)) {
        return FALSE;
    }

    if (stream->playSequence + 2 != node->sequence) {
        return FALSE;
    }

    samples = node->samples;
    dataSize = sAudioFrameLength * sCodecInfo[samples[VCT_DROP_BUFFER_SIZE + 5]].chunk_size + sCodecInfo[samples[VCT_DROP_BUFFER_SIZE + 5]].header_size;
    VCTi_Report("vct_recover_drop_using_spare: spare codec:%x, sequecne:%d -> %d(%d)\n", samples[VCT_DROP_BUFFER_SIZE + 5], stream->playSequence,
                node->sequence, *(u16*)(samples + VCT_DROP_BUFFER_SIZE + 6));

    stream->playSequence = node->sequence - 1;
    vct_decode_adpcm(samples + VCT_DROP_BUFFER_SIZE + VCT_AUDIO_HEADER_SIZE, out, dataSize, samples[VCT_DROP_BUFFER_SIZE + 5]);
    stream->info.spareRecoverCount++;
    return TRUE;
}

// Rebuilds a missing frame: the previous frame's samples are continued according
// to the drop recovery mode, then faded out the longer the drop goes on.
static inline BOOL vct_recover_drop_impl(AudioStream* stream, s16* prev, s16* out, u32 length) {
    int ch = stream->channel;
    s16* work = sDropBuffer[ch];
    u32 samples;
    BOOL result;

    if (stream->continuousDrop == 0) {
        if (prev == NULL) {
            return FALSE;
        }

        switch (sDropRecoverMode) {
        case 0:
            VCTi_Report("vct_recover_drop:VCT_DROP_RECOVER_MODE_NONE\n");
            MI_CpuFill32(0, work, length);
            break;
        case 1:
            VCTi_Report("vct_recover_drop:VCT_DROP_RECOVER_MODE_COPY_ALL\n");
            MI_CpuCopyFast(prev, work, length);
            break;
        case 2:
            VCTi_Report("vct_recover_drop:VCT_DROP_RECOVER_MODE_COPY_TAIL\n");
            vct_recover_drop_copy_tail(prev, work, length);
            break;
        case 3: {
            u32 i;
            u32 sum = 0;
            u32 level;

            VCTi_Report("vct_recover_drop:VCT_DROP_RECOVER_MODE_WHITE_NOISE\n");
            samples = length / sizeof(s16);
            for (i = 0; i < samples; i++) {
                sum += prev[i] * prev[i];
            }
            level = SQRT(sum / samples);
            if (level > sMaxNoiseLevel) {
                level = sMaxNoiseLevel;
            }
            if (sNoiseLevelPercent >= 0 && sNoiseLevelPercent <= 800) {
                level = level * sNoiseLevelPercent / 100;
            }
            for (i = 0; i < samples; i++) {
                sRandContext.x = sRandContext.mul * sRandContext.x + sRandContext.add;
                work[i] = (level * (0x7FFF - (sRandContext.x >> 16))) >> 15;
            }
            break;
        }
        case 4:
            VCTi_Report("vct_recover_drop:VCT_DROP_RECOVER_MODE_PINK_NOISE\n");
            vct_recover_drop_pink_noise(prev, work, length);
            break;
        }

        MI_CpuCopyFast(work, out, length);
        vct_interpolate_samples(out, out, length / sizeof(s16), sInterpolateSampleNum, prev[length / sizeof(s16) - 1]);
        result = TRUE;
    } else if (sContinuousDropLimit == 1 && stream->continuousDrop == 1) {
        u32 i;
        u32 n = sFadeOutSampleNum;

        if (n != 0) {
            if (n > length / sizeof(s16)) {
                n = length / sizeof(s16);
            }
            for (i = 0; i < n; i++) {
                out[i] = work[i] * (n - (i + 1)) / n;
            }
        }
        VCTi_Report("vct_filter_fade_out:channel:%d, continuous drop:%d, [%d]:%d->%d\n", ch, stream->continuousDrop, length / 4, work[length / 4], out[length / 4]);
        result = TRUE;
    } else if (stream->continuousDrop < sContinuousDropLimit) {
        u32 i;

        samples = length / sizeof(s16);
        for (i = 0; i < samples; i++) {
            out[i] = work[i] - work[i] * (int)stream->continuousDrop / sContinuousDropLimit;
        }
        VCTi_Report("vct_recover_drop_fade:channel:%d, continuous drop:%d, [%d]:%d->%d\n", ch, stream->continuousDrop, length / 4, work[length / 4], out[length / 4]);
        result = TRUE;
    } else {
        MI_CpuFill32(0, out, length);
        if (++stream->continuousDrop == 0) {
            stream->continuousDrop = sContinuousDropLimit;
        }
        return FALSE;
    }

    stream->lastSample = out[length / sizeof(s16) - 1];
    stream->interpolate = TRUE;
    stream->continuousDrop++;
    return result;
}

BOOL vct_recover_drop_stream(AudioStream* stream, s16* out, u32 length) {
    UDPPacket* prevBuffer = stream->prevBuffer;
    BOOL result;

    if (prevBuffer == NULL) {
        result = vct_recover_drop_impl(stream, NULL, out, length);
        if (!result) {
            return FALSE;
        }
        stream->info.recoverCount++;
        return TRUE;
    }

    result = vct_recover_drop_impl(stream, (s16*)prevBuffer->samples, out, length);
    vct_free_packet_buffer(stream->prevBuffer);
    stream->prevBuffer = NULL;

    if (!result) {
        VCTi_Report("vct_recover_drop_stream: continuous late\n");
        return FALSE;
    }

    stream->info.recoverCount++;
    return TRUE;
}

void vct_recover_drop_copy_tail(s16* prev, s16* out, u32 length) {
    u32 tailBytes = sDropRecoverCopyTailFrameLength * 16;
    int tailSamples;
    s16 last;
    s16* tail;
    int interp;
    int i;

    if (length < tailBytes) {
        VCTi_Report("vct_recover_drop_copy_tail:invalid tailDataSize:%d\n", tailBytes);
        return;
    }

    tailSamples = (int)tailBytes / 2;
    last = prev[length / sizeof(s16) - 1];
    tail = prev + (length / sizeof(s16) - tailSamples);
    interp = sInterpolateSampleNum;

    for (i = 0; i < length / sizeof(s16); i++) {
        int j = i % tailSamples;

        if (j < interp) {
            int w = j + 1;

            out[i] = (last * (interp - w) + w * tail[j]) / interp;
        } else {
            out[i] = tail[j];
        }
    }
}

void vct_interpolate_samples(s16* src, s16* dst, u32 samples, u32 interpNum, s16 last) {
    int i;

    if (interpNum == 0) {
        return;
    }

    if (interpNum > samples) {
        interpNum = samples;
    }

    for (i = 0; i < interpNum; i++) {
        int w = i + 1;

        dst[i] = (last * ((int)interpNum - w) + w * src[i]) / (int)interpNum;
    }
}

BOOL VCTi_IsAdpcmCodec(int codec) {
    return codec >= VCT_CODEC_2BIT_ADPCM && codec <= VCT_CODEC_4BIT_ADPCM;
}

BOOL VCT_SetCodec(VCTCodec codec) {
    if (!VCTi_IsAdpcmCodec(codec)) {
        VCTi_Report("VCT_SetCodec: invalid codec:%d\n", codec);
        return FALSE;
    }

    sCodec = codec;
    return TRUE;
}

BOOL VCT_SetSpareCodec(VCTCodec codec, VCTCodec spareCodec) {
    if (!VCTi_IsAdpcmCodec(codec)) {
        VCTi_Report("VCT_SetSpareCodec: invalid codec:%d\n", codec);
        return FALSE;
    }
    if (!(spareCodec >= VCT_CODEC_2BIT_ADPCM && spareCodec <= VCT_CODEC_3BIT_ADPCM)) {
        VCTi_Report("VCT_SetSpareCodec: invalid spare codec:%d\n", spareCodec);
        return FALSE;
    }
    if (spareCodec > codec) {
        VCTi_Report("VCT_SetSpareCodec: invalid codec combination:%d, %d\n", codec, spareCodec);
        return FALSE;
    }

    sCodec = codec;
    sSpareCodec = spareCodec;
    return TRUE;
}

void VCT_EnableQualityAlert(BOOL enable) {
    sEnableQualityAlert = enable;
    if (enable) {
        sSpareAdpcmState.prevSample = 0;
        sSpareAdpcmState.prevIndex = 0;
    }
}

void VCT_EnableVAD(BOOL flag) {
    sEnableVAD = flag;
    VCT_ResetVAD();

    if (!flag) {
        sChangeVAD = TRUE;
    }
}

void VCT_EnableEchoCancel(BOOL flag) {
    if (flag) {
        VCTi_Report("VCT_EnableEchoCancel: ignored because echo cancel of VCT should not be used for Wii\n");
        return;
    }

    sEnableEchoCancel = flag;
}

void VCT_SetDropRecoverCopyTailFrameLength(u32 length) {
    if (length > 0x20) {
        VCTi_Report("VCT_SetDropRecoverCopyTailFrameLength:invalid param:%d\n", length);
        return;
    }

    sDropRecoverCopyTailFrameLength = length;
}

void VCT_SetInterpolateSampleNum(u32 num) {
    if (num > 0x20) {
        VCTi_Report("VCT_SetInterpolateSampleNum:too large number:%d\n", num);
        return;
    }

    sInterpolateSampleNum = num;
}

void VCTi_SetHpfMode(BOOL enable) {
    sHpfMode = enable;
}

void VCT_SetDropRecoverMode(int mode) {
    sDropRecoverMode = mode;
}

void VCTi_SetContinuousDropLimit(u32 limit) {
    if (limit == 0) {
        return;
    }

    sContinuousDropLimit = limit;
}

u32 VCTi_GetAudioFrameLength(void) {
    return sAudioFrameLength;
}

u32 VCTi_GetAudioDataSize(void) {
    return sAudioDataSize;
}

u32 VCTi_GetAudioFrameTick(void) {
    return sAudioFrameTick;
}

void vct_recover_drop_pink_noise(s16* prev, s16* out, u32 length) {
    static s16 sPinkNoise[PINK_NOISE_LENGTH] = {
#include "pink_noise.inc"
    };
    u32 samples = length / sizeof(s16);
    u32 sum = 0;
    u32 level;
    u32 offset;
    int i;

    for (i = 0; i < samples; i++) {
        sum += prev[i] * prev[i];
    }

    level = SQRT(sum / samples);
    VCTi_Report("vct_recover_drop_pink_noise:%d\n", level);

    if (level > sMaxNoiseLevel) {
        level = sMaxNoiseLevel;
    }
    if (sNoiseLevelPercent >= 0 && sNoiseLevelPercent <= 800) {
        level = level * sNoiseLevelPercent / 100;
    }

    sRandContext.x = sRandContext.mul * sRandContext.x + sRandContext.add;
    offset = ((sRandContext.x >> 16) * 0x300) >> 16;

    for (i = 0; i < samples; i++) {
        out[i] = (level * sPinkNoise[((int)i + (int)offset) % PINK_NOISE_LENGTH]) >> 15;
    }
}

void VCTi_SetQualityAlertFrameNum(u32 num) {
    sQualityAlertFrameNum = num;
}

void VCT_SetQualityAlertThreshold(u32 threshold) {
    if (threshold > 100) {
        VCTi_Report("VCT_SetQualityAlertThreshold:invalid threshold :%d\n", threshold);
        return;
    }

    sQualityAlertThreshold = threshold;
}

void vct_check_quality_alert_time(void) {
    if (sQualityAlertStatus == QUALITY_ALERT_STATUS_CLEARING) {
        s64 elapsed = (s64)(OSGetTime() - sQualityAlertTime) / (VCT_TIMER_CLOCK / 1000);

        if (elapsed >= (s64)(sQualityAlertClearTime * 1000)) {
            VCTi_Report("vct_check_quality_alert_time:QUALITY_ALERT_STATUS_CLEARED\n");
            sQualityAlertStatus = QUALITY_ALERT_STATUS_NONE;
            sQualityAlertTime = 0;
            sSpareAdpcmState.prevSample = 0;
            sSpareAdpcmState.prevIndex = 0;
        }
    }
}
