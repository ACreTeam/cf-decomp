#ifndef VCT_VOICECHAT_H_
#define VCT_VOICECHAT_H_

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Voice chat library (DWC_VCT 2.0.2 by Abiosso, Wii revision of libVCT).
// Interface follows https://github.com/ntrtwl/libvct (DS revision).

#define VCT_ERROR_NONE 0
#define VCT_ERROR_BAD_PARAM -1
#define VCT_ERROR_BAD_MODE -2
#define VCT_ERROR_BAD_REQUEST -3
#define VCT_ERROR_SEND_FAIL -4
#define VCT_ERROR_TRANSCEIVER_BUSY -5
#define VCT_ERROR_EXHAUST_CLIENTS -6

#define VCT_MAX_SESSION 8
#define VCT_MAX_TRANSCEIVER_CLIENT 8
#define VCT_MAX_CONFERENCE_CLIENT 4

#define VCT_MAX_AUDIO_DATA_SIZE 3072

#define VCT_MAX_AUDIO_STREAM (VCT_MAX_CONFERENCE_CLIENT - 1)

#define VCT_MIN_AUDIO_BUFFER_COUNT 4
#define VCT_MAX_AUDIO_BUFFER_COUNT (24 * VCT_MAX_AUDIO_STREAM)

#define VCT_MAGIC_PACKET_HEADER '_VCT'

typedef struct _VCTSession VCTSession;

typedef enum {
    VCT_CODEC_8BIT_RAW,
    VCT_CODEC_G711_ULAW,
    VCT_CODEC_2BIT_ADPCM,
    VCT_CODEC_3BIT_ADPCM,
    VCT_CODEC_4BIT_ADPCM,
    VCT_CODEC_END
} VCTCodec;

typedef enum {
    VCT_MODE_NULL,
    VCT_MODE_PHONE,
    VCT_MODE_TRANSCEIVER,
    VCT_MODE_CONFERENCE
} VCTMode;

typedef enum {
    VCT_EVENT_NONE,
    VCT_EVENT_INCOMING,
    VCT_EVENT_REJECT,
    VCT_EVENT_BUSY,
    VCT_EVENT_CANCEL,
    VCT_EVENT_NOTIFY_FREE,
    VCT_EVENT_NOTIFY_BUSY,
    VCT_EVENT_CONNECTED,
    VCT_EVENT_RESPONDBYE,
    VCT_EVENT_DISCONNECTED,
    VCT_EVENT_CONTACT,
    VCT_EVENT_TIMEOUT,
    VCT_EVENT_ABORT
} VCTEvent;

typedef enum {
    VCT_STATE_INIT,
    VCT_STATE_OUTGOING,
    VCT_STATE_TALKING,
    VCT_STATE_CONNECTED,
    VCT_STATE_INCOMING,
    VCT_STATE_DISCONNECTING
} VCTState;

struct _VCTSession {
    /* 0x00 */ VCTMode mode;
    /* 0x04 */ u8 aid;
    /* 0x05 */ u8 talking;
    /* 0x08 */ u32 aidBitmap;
    /* 0x0C */ VCTState state;
    /* 0x10 */ void* userData;
    /* 0x14 */ VCTSession* next;
}; // size 0x18

typedef void (*VCTEventCallback)(u8 aid, VCTEvent result, VCTSession* session, void* param);

typedef struct {
    /* 0x00 */ VCTSession* session;
    /* 0x04 */ u32 numSession;
    /* 0x08 */ VCTMode mode;
    /* 0x0C */ u8 aid;
    /* 0x0D */ u8 codec;
    /* 0x0E */ u8 frameLength;
    /* 0x0F */ u8 flags;
    /* 0x10 */ void* audioBuffer;
    /* 0x14 */ u32 audioBufferSize;
    /* 0x18 */ VCTEventCallback callback;
    /* 0x1C */ void* userData;
} VCTConfig;

#define VCT_CONFIG_FLAG_VAD 0x1
#define VCT_CONFIG_FLAG_ECHO_CANCEL 0x2
#define VCT_CONFIG_FLAG_HPF 0x4

BOOL VCT_Init(VCTConfig* config);
void VCT_Cleanup(void);
void VCT_Main(void);
BOOL VCT_HandleData(u8 aid, u8* buffer, int size);

BOOL VCT_StartStreaming(VCTSession* session);
void VCT_StopStreaming(VCTSession* session);
BOOL VCT_SendAudio(void* audio_data, u32 length);
BOOL VCT_ReceiveAudio(void* audio_data, u32 length, u32* outAIDBitmap);
BOOL VCT_SetCodec(VCTCodec codec);
BOOL VCT_SetSpareCodec(VCTCodec codec, VCTCodec spareCodec);
void VCT_SetDropRecoverCopyTailFrameLength(u32 length);
void VCT_SetInterpolateSampleNum(u32 num);
void VCTi_SetHpfMode(BOOL enable);
void VCTi_SetContinuousDropLimit(u32 limit);
void VCTi_SetQualityAlertFrameNum(u32 num);
u32 VCTi_GetAudioFrameLength(void);
u32 VCTi_GetAudioDataSize(void);
void VCT_EnableVAD(BOOL flag);
void VCT_SetVADReleaseTime(int count);
void VCT_ResetVAD(void);
void VCT_EnableEchoCancel(BOOL flag);
void VCT_EnableQualityAlert(BOOL enable);
void VCT_SetDropRecoverMode(int mode);
void VCT_SetQualityAlertThreshold(u32 threshold);

typedef enum {
    VCT_REPORTLEVEL_NONE,
    VCT_REPORTLEVEL_ALL
} VCTReportLevel;

void VCT_SetReportLevel(VCTReportLevel level);
u32 VCT_GetAudioDataSize(void);

#ifdef __cplusplus
}
#endif

#endif
