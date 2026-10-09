// Wii Speak voice chat (namespace dVoiceChat).
// .text 801711EC..801732D4, .rodata 8047B1B0, .data 804F45B0, .bss 80600940, .sdata 8074B510,
// .sbss 8074E8D0, .sdata2 80751118. See include/game/game/d_voice_chat.hpp and
// notes/d_voice_chat.txt.
#include <game/game/d_voice_chat.hpp>
#include <game/snd/snd_manager.hpp>
#include <lib/dwc/vct.h>
#include <revolution/AI.h>
#include <revolution/AX.h>
#include <revolution/MIX.h>
#include <revolution/OS.h>
#include <revolution/PMIC.h>
#include <cstring>

namespace dVoiceChat {

// The microphone state machine (updateMic). Even states issue a PMIC request, odd ones wait
// for its completion (l_pmicResult).
enum MicState_e {
    MIC_OPEN,          // 0
    MIC_WAIT_OPEN,     // 1
    MIC_SET_2,         // 2: fn_80229A2C
    MIC_WAIT_2,        // 3
    MIC_SET_4,         // 4: fn_80229B04
    MIC_WAIT_4,        // 5
    MIC_SET_6,         // 6: fn_8022A038
    MIC_WAIT_6,        // 7
    MIC_START,         // 8
    MIC_WAIT_START,    // 9
    MIC_STOP,          // 10
    MIC_WAIT_STOP,     // 11
    MIC_STOPPED,       // 12: open, not running
    MIC_CLOSE,         // 13
    MIC_WAIT_CLOSE,    // 14
    MIC_CLOSED,        // 15
    MIC_ACTIVE,        // 16: running; also where a failed request ends up
};

// The speaker voice (initSpeaker / updateSpeaker).
struct SpeakerVoice {
    /* 0x00 */ AXVPB *mpVpb;
    /* 0x04 */ u32 mState;
};

enum SpeakerState_e {
    SPEAKER_NONE,       // 0
    SPEAKER_CLEAR,      // 1: clear the buffer
    SPEAKER_START,      // 2
    SPEAKER_RUN,        // 3: refill the half the voice has left
    SPEAKER_ACQUIRE,    // 4
    SPEAKER_PLAY,       // 5
    SPEAKER_FREE,       // 6
    SPEAKER_FREED,      // 7
};

// Alarm period in ms by l_frameIdx, and the value for fn_8036CB50 (VCT).
static const u8 l_frameTimes[10] = {0x44, 0x60, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0xA8, 0xC0};
static const u8 l_vadParams[10] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

// Speaker gain by (l_targetGain - l_curGain): +6 dB to +26 dB in 0.1 dB steps (10^(0.3 + i / 200)).
static const f32 l_gainTable[201] = {
    1.99526203f, 2.0183661f, 2.04173803f, 2.0653801f, 2.0892961f, 2.11348891f,
    2.1379621f, 2.16271901f, 2.18776202f, 2.21309495f, 2.23872089f, 2.26464391f,
    2.29086804f, 2.31739497f, 2.34422898f, 2.37137389f, 2.39883304f, 2.42660999f,
    2.45470905f, 2.48313308f, 2.51188588f, 2.54097295f, 2.57039595f, 2.60015988f,
    2.6302681f, 2.66072512f, 2.691535f, 2.72270107f, 2.75422907f, 2.78612089f,
    2.81838298f, 2.85101795f, 2.88403201f, 2.91742706f, 2.95120907f, 2.98538303f,
    3.01995206f, 3.05492091f, 3.09029508f, 3.12607908f, 3.16227794f, 3.19889498f,
    3.23593712f, 3.27340698f, 3.31131101f, 3.34965396f, 3.38844204f, 3.42767811f,
    3.46736908f, 3.50751901f, 3.54813409f, 3.58921909f, 3.63078094f, 3.67282295f,
    3.71535206f, 3.75837398f, 3.80189395f, 3.84591794f, 3.89045095f, 3.9355011f,
    3.98107195f, 4.02717018f, 4.07380295f, 4.12097502f, 4.16869402f, 4.2169652f,
    4.26579523f, 4.31519079f, 4.36515808f, 4.41570377f, 4.46683598f, 4.51855898f,
    4.57088184f, 4.62380981f, 4.677351f, 4.73151302f, 4.78630114f, 4.84172392f,
    4.89778805f, 4.95450211f, 5.01187181f, 5.06990719f, 5.12861395f, 5.1880002f,
    5.24807501f, 5.30884409f, 5.37031794f, 5.43250322f, 5.49540901f, 5.55904293f,
    5.62341309f, 5.68852901f, 5.75439882f, 5.82103205f, 5.88843679f, 5.95662117f,
    6.02559614f, 6.09536886f, 6.16594982f, 6.23734808f, 6.30957317f, 6.38263512f,
    6.45654202f, 6.53130579f, 6.60693407f, 6.68343878f, 6.76082993f, 6.8391161f,
    6.91831017f, 6.99841976f, 7.07945824f, 7.16143417f, 7.24435997f, 7.32824516f,
    7.41310215f, 7.4989419f, 7.58577585f, 7.67361498f, 7.7624712f, 7.85235596f,
    7.94328213f, 8.03526115f, 8.12830544f, 8.22242641f, 8.3176384f, 8.41395092f,
    8.5113802f, 8.60993767f, 8.70963573f, 8.8104887f, 8.91250896f, 9.01571083f,
    9.12010765f, 9.22571373f, 9.33254337f, 9.44060898f, 9.5499258f, 9.66050911f,
    9.77237225f, 9.88553143f, 10.0f, 10.1157951f, 10.2329302f, 10.3514223f,
    10.4712849f, 10.5925369f, 10.7151928f, 10.8392687f, 10.9647818f, 11.0917482f,
    11.2201853f, 11.3501081f, 11.4815359f, 11.6144857f, 11.7489758f, 11.8850222f,
    12.022644f, 12.1618605f, 12.3026876f, 12.4451456f, 12.5892544f, 12.7350311f,
    12.8824959f, 13.0316677f, 13.1825666f, 13.3352137f, 13.4896288f, 13.6458311f,
    13.8038425f, 13.9636841f, 14.1253748f, 14.2889404f, 14.4543982f, 14.6217718f,
    14.7910843f, 14.9623566f, 15.1356115f, 15.3108749f, 15.4881659f, 15.667511f,
    15.8489323f, 16.0324535f, 16.2181015f, 16.4058971f, 16.5958691f, 16.7880402f,
    16.9824371f, 17.1790848f, 17.3780079f, 17.5792351f, 17.782795f, 17.9887085f,
    18.1970081f, 18.4077206f, 18.6208706f, 18.8364906f, 19.0546074f, 19.2752495f,
    19.4984455f, 19.7242279f, 19.9526234f,
};

// 4x upsampling FIR: 4 phases of 32 taps.
static s16 l_firCoef[4][32] = {
    {
        3, -5, 14, -24, 42, -64, 93, -123,
        156, -182, 199, -191, 144, -8, -409, 7525,
        1562, -903, 659, -503, 385, -286, 207, -141,
        92, -55, 31, -14, 6, 0, 0, 0,
    },
    {
        3, -6, 13, -20, 31, -40, 48, -48,
        37, -5, -55, 163, -346, 684, -1472, 6209,
        3996, -1547, 908, -585, 384, -244, 148, -81,
        38, -11, -1, 8, -7, 7, -3, 3,
    },
    {
        3, -3, 7, -7, 8, -1, -11, 38,
        -81, 148, -244, 384, -585, 908, -1547, 3996,
        6209, -1472, 684, -346, 163, -55, -5, 37,
        -48, 48, -40, 31, -20, 13, -6, 3,
    },
    {
        0, 0, 0, 6, -14, 31, -55, 92,
        -141, 207, -286, 385, -503, 659, -903, 1562,
        7525, -409, -8, 144, -191, 199, -182, 156,
        -123, 93, -64, 42, -24, 14, -5, 3,
    },
};

// Microphone samples (sent) and received samples (played), one audio frame each.
static s16 l_micBuf[0x600] ALIGN(32);
static s16 l_spkBuf[0x600] ALIGN(32);
static u8 l_vctWork[0x15CC0] ALIGN(32); // VCT work memory (VCTConfig::audioBuffer)
static VCTSession l_sessions[3];
static VCTConfig l_config;
static OSAlarm l_alarm;
static OSMessageQueue l_msgQueue;
static u8 l_aiBufData[0x6000] ALIGN(32); // the AX voice's looping buffer (32 kHz)
static u8 l_dmaBuf[2][0x180] ALIGN(32);  // copies of the AI output blocks
static s16 l_refBuf[0x60] ALIGN(32);     // echo-cancel reference (speaker) samples
static s16 l_history[31];                // upsample's last 31 input samples

static u32 l_micState = MIC_CLOSED;
static int l_vctParamD = 4;
static u16 l_pmicParam6 = 3;
static u8 l_frameIdx = 7;
static u8 l_vctParamAC8 = 4;
static u8 l_vctParamA60 = 1;
static u8 l_idle = TRUE;    // no client connected
static u8 l_stopped = TRUE; // the audio alarm is not running
static u8 l_memberMode = 2; // the last PMIC preset chosen (selectPreset)
static BOOL l_echoOk = TRUE;
static u8 *l_aiBuf = l_aiBufData;
static BOOL l_refReset = TRUE;

static u16 l_clientMask;
static u16 l_pmicParam4;
static u8 l_initialized;
static u8 l_started;
static u8 l_muted;
static u8 l_micConnected;
static u8 l_8074E8D8;
static u16 l_dropCount;
static OSMessage l_msg;
static int l_targetGain;
static int l_curGain;
static int l_maxGain;
static int l_frameCount;
static u8 l_clipped;
static u8 l_echoCancel;
static u8 l_prevEchoCancel;
static s16 *l_spkSrc;
static SpeakerVoice l_voice;
static AIDMACallback l_oldDmaCallback;
static void *l_lastDmaBuf;
static u8 l_dmaIdx;
static u32 l_lastPos;
static volatile s32 l_pmicResult;

static void initSpeaker(s16 *src);
static void shutdownSpeaker();
static s32 updateMic();
static void pmicCallback(s32 result);
static void pmicStartCallback(s32 result);
static u8 selectPreset(u8 count);

// Members in the chat: ourselves plus one per bit of the client mask.
static u8 getMemberCount() {
    u8 count = 1;
    for (int i = 0; i < 16; i++) {
        if (l_clientMask & (1 << i)) {
            count++;
        }
    }
    return count;
}

static void eventCallback(u8 aid, int event, VCTSession *session, void *data) {
    switch (event) {
    case 9:
        fn_803662F0(session);
        fn_8036A554(session);
        l_clientMask &= ~(1 << aid);
        if (l_clientMask == 0) {
            l_idle = TRUE;
        }
        setMemberCount(getMemberCount());
        l_dropCount = 0;
        break;
    case 7:
        fn_80366120(session);
        l_dropCount = 0;
        break;
    case 12:
        fn_803662F0(session);
        fn_8036A554(session);
        l_clientMask &= ~(1 << aid);
        if (l_clientMask == 0) {
            l_idle = TRUE;
        }
        setMemberCount(getMemberCount());
        l_dropCount = 0;
        break;
    }
}

static void setGain(int gain) {
    BOOL enabled = OSDisableInterrupts();
    l_targetGain = gain;
    if (l_targetGain < 0) {
        l_targetGain = 0;
    } else if (l_targetGain > 200) {
        l_targetGain = 200;
    }
    l_maxGain = l_targetGain;
    if (l_curGain > l_targetGain) {
        l_curGain = l_targetGain;
    }
    OSRestoreInterrupts(enabled);
}

// Applies the speaker gain to the received audio, backing off one step per 16 samples while it
// clips and recovering one step per 16 clean samples.
static void applyGain() {
    s16 *buf = l_spkBuf;
    u32 samples = fn_8036D50C() / 2;
    for (u32 i = 0; i < samples; i++) {
        f32 value = buf[i];
        value *= l_gainTable[l_targetGain - l_curGain];
        if (value > 23170.0f) {
            value = 23170.0f;
            l_clipped = TRUE;
        } else if (value < -23170.0f) {
            value = -23170.0f;
            l_clipped = TRUE;
        }
        buf[i] = value + 0.5f;
        if (++l_frameCount >= 16) {
            l_frameCount = 0;
            if (l_clipped) {
                if (++l_curGain > l_maxGain) {
                    l_curGain = l_maxGain;
                }
                l_clipped = FALSE;
            } else {
                if (--l_curGain < 0) {
                    l_curGain = 0;
                }
            }
        }
    }
}

static void frameAlarm(OSAlarm *alarm, OSContext *context) {
    u32 size = fn_8036D50C();
    u32 samples = size / 2;
    if (fn_80366A08(l_spkBuf, size, NULL)) {
        applyGain();
    } else {
        memset(l_spkBuf, 0, sizeof(l_spkBuf));
    }

    s32 read;
    if (l_micConnected) {
        read = fn_80228EB0(l_micBuf, samples);
        if (read <= 0) {
            memset(l_micBuf, 0, size);
        }
    } else {
        read = -1;
        memset(l_micBuf, 0, size);
    }

    if (l_echoCancel) {
        BOOL ok;
        if (l_prevEchoCancel) {
            ok = fn_80229BE4(l_micBuf, l_spkBuf, samples, 0);
        } else {
            ok = fn_80229BE4(l_micBuf, l_spkBuf, samples, 1);
        }
        if (ok && !l_echoOk) {
            fn_8022A114(1);
            fn_8022A114(0);
        }
        l_echoOk = ok;
    } else {
        l_echoOk = TRUE;
    }
    l_prevEchoCancel = l_echoCancel;

    if (read >= samples) {
        fn_803663C4(l_micBuf, size);
    } else {
        l_dropCount++;
    }
}

static void startAudio() {
    initSpeaker(l_spkBuf);
    l_echoCancel = TRUE;
    l_prevEchoCancel = FALSE;
    l_echoOk = TRUE;
    OSCreateAlarm(&l_alarm);
    OSSetPeriodicAlarm(&l_alarm, OSGetTime(), OS_MSEC_TO_TICKS(getFrameTime()), frameAlarm);
    l_stopped = FALSE;
}

static void stopAudio() {
    OSCancelAlarm(&l_alarm);
    l_stopped = TRUE;
    shutdownSpeaker();
}

BOOL init(u8 aid) {
    if (!l_initialized) {
        l_config.session = l_sessions;
        l_config.numSession = 3;
        l_config.mode = 3;
        l_config.aid = aid;
        l_config.audioBuffer = l_vctWork;
        l_config.audioBufferSize = sizeof(l_vctWork);
        l_config.callback = eventCallback;
        l_config.userData = NULL;
        l_config._0E = getFrameTime();
        l_config._0D = l_vctParamD;
        l_config._0F = 0;
        if (!fn_8036CF1C(&l_config)) {
            return FALSE;
        }
        fn_80368A60(l_vctParamA60);
        fn_8036CB50(l_vadParams[l_frameIdx]);
        fn_80368AC8(l_vctParamAC8);
        fn_80368A40(TRUE);
        fn_80368DE0(40);
        fn_80368DE8(16);
        fn_80368AD0(2);
        fn_8036D458(0);
        OSInitMessageQueue(&l_msgQueue, &l_msg, 1);
        l_clientMask = 0;
        l_memberMode = 2;
        l_micConnected = FALSE;
        l_idle = TRUE;
        l_stopped = TRUE;
        l_initialized = TRUE;
    }
    return TRUE;
}

void cleanup() {
    if (l_initialized) {
        stop();
        fn_8036D1DC();
        l_clientMask = 0;
        l_initialized = FALSE;
    }
}

BOOL start() {
    if (!l_started) {
        if (!l_initialized) {
            return FALSE;
        }
        memset(l_micBuf, 0, sizeof(l_micBuf));
        memset(l_spkBuf, 0, sizeof(l_spkBuf));
        l_clipped = FALSE;
        l_frameCount = 0;
        startAudio();
        l_dropCount = 0;
        l_started = TRUE;
        l_8074E8D8 = 0;
    }
    return TRUE;
}

void stop() {
    if (l_started) {
        stopAudio();
        l_micConnected = FALSE;
        l_idle = TRUE;
        l_started = FALSE;
    }
}

s32 update() {
    s32 result = updateMic();
    if (result == -100) {
        l_micConnected = FALSE;
    }
    return result;
}

void calc() {
    if (l_started) {
        fn_8036D218();
    }
}

void connect(u8 aid) {
    if (l_started) {
        int bit = 1 << aid;
        if (!(l_clientMask & bit)) {
            if (fn_8036A854(aid) == 0) {
                l_clientMask |= bit;
                l_idle = FALSE;
                setMemberCount(getMemberCount());
            }
        }
    }
}

void disconnect(u8 aid) {
    if (l_started && (l_clientMask & (1 << aid))) {
        fn_8036AA20(aid);
    }
}

BOOL isStarted() {
    return l_started;
}

u16 getClientMask() {
    return l_clientMask;
}

u32 getFrameTime() {
    return l_frameTimes[l_frameIdx];
}

void setParam(u8 value) {
    if (value != l_vctParamA60) {
        l_vctParamA60 = value;
        if (l_initialized) {
            fn_80368A60(value);
        }
    }
}

BOOL isMicConnected() {
    return l_micConnected;
}

BOOL isMicActive() {
    return l_started ? l_micConnected : 0;
}

static inline BOOL isStoppedActive() {
    return l_stopped && l_micState == MIC_ACTIVE;
}

BOOL isMicShutDown() {
    return l_micState == MIC_CLOSED || isStoppedActive();
}

void setVolume(int volume) {
    setGain(volume * 20);
}

void setMute(u8 mute) {
    l_muted = mute;
}

void setMemberCount(u8 count) {
    if (l_started && count >= 2 && count <= 9) {
        int mode = selectPreset(count);
        if (mode) {
            l_memberMode = mode;
        }
    }
}

BOOL fn_80171C08(int arg) {
    return TRUE;
}

BOOL fn_80171C10(u16 arg) {
    return FALSE;
}

static void updateSpeaker();
static void dmaCallback();

static void initSpeaker(s16 *src) {
    if (l_voice.mState == SPEAKER_NONE) {
        l_spkSrc = src;
        SoundManager::getInstance()->setFrameCallback(updateSpeaker);
        l_voice.mState = SPEAKER_CLEAR;
        l_lastDmaBuf = NULL;
        l_dmaIdx = 0;
        l_refReset = TRUE;
        BOOL enabled = OSDisableInterrupts();
        l_oldDmaCallback = AIRegisterDMACallback(dmaCallback);
        OSRestoreInterrupts(enabled);
    }
}

static void shutdownSpeaker() {
    if (l_voice.mState != SPEAKER_NONE) {
        SoundManager::getInstance()->clearFrameCallback();
        if (l_voice.mState != SPEAKER_FREED) {
            AXSetVoiceState(l_voice.mpVpb, AX_VOICE_STOP);
            AXFreeVoice(l_voice.mpVpb);
            l_voice.mpVpb = NULL;
        }
        l_voice.mState = SPEAKER_NONE;
        BOOL enabled = OSDisableInterrupts();
        AIRegisterDMACallback(l_oldDmaCallback);
        OSRestoreInterrupts(enabled);
        l_oldDmaCallback = NULL;
    }
}

static void setupVoice(AXVPB *vpb, void *buf, u32 size) {
    AXPBADDR addr;
    u32 start = (u32)OSCachedToPhysical(buf) / 2;
    u32 end = start + size / 2 - 1;
    addr.loopFlag = TRUE;
    addr.format = AX_SAMPLE_FORMAT_PCM_S16;
    addr.loopAddressHi = start >> 16;
    addr.loopAddressLo = start & 0xFFFF;
    addr.endAddressHi = end >> 16;
    addr.endAddressLo = end & 0xFFFF;
    addr.currentAddressHi = start >> 16;
    addr.currentAddressLo = start & 0xFFFF;
    AXSetVoiceAddr(vpb, &addr);
    AXSetVoiceSrcRatio(vpb, 1.0f);
    AXSetVoiceSrcType(vpb, AX_SRC_TYPE_NONE);
}

static void acquireVoice(SpeakerVoice *voice, void *buf, u32 size) {
    if (buf) {
        AXVPB *vpb = AXAcquireVoice(15, NULL, 0);
        if (vpb) {
            MIXInitChannel(vpb, 0, 0, -904, -904, -904, 64, 127, 30);
            setupVoice(vpb, buf, size);
            voice->mpVpb = vpb;
        }
    }
}

static BOOL fillSpeaker(void *dst, s16 *src, u32 size);

// The voice's play position as a physical byte address.
static inline u32 getPlayPos(const AXVPB *vpb) {
    return ((vpb->pb.addr.currentAddressHi << 16) | vpb->pb.addr.currentAddressLo) * 2;
}

static void updateSpeaker() {
    u32 size = fn_8036D50C() * 8;
    switch (l_voice.mState) {
    case SPEAKER_CLEAR:
        memset(l_aiBuf, 0, size);
        DCFlushRange(l_aiBuf, size);
        l_voice.mState = SPEAKER_ACQUIRE;
        break;
    case SPEAKER_ACQUIRE:
        acquireVoice(&l_voice, l_aiBuf, size);
        l_voice.mState = SPEAKER_PLAY;
        break;
    case SPEAKER_PLAY:
        AXSetVoiceState(l_voice.mpVpb, AX_VOICE_RUN);
        l_voice.mState = SPEAKER_START;
        break;
    case SPEAKER_START:
        l_lastPos = 0;
        l_voice.mState = SPEAKER_RUN;
        break;
    case SPEAKER_RUN: {
        AXVPB *vpb = l_voice.mpVpb;
        if (vpb) {
            u32 pos = getPlayPos(vpb);
            u32 half = size / 2;
            u32 mid = (u32)((u8 *)OSCachedToPhysical(l_aiBuf) + half);
            if (pos < l_lastPos) {
                fillSpeaker(l_aiBuf + half, l_spkSrc, half);
            } else if (l_lastPos < mid && pos >= mid) {
                fillSpeaker(l_aiBuf, l_spkSrc, half);
            }
            l_lastPos = pos;
        }
        break;
    }
    case SPEAKER_FREE:
        memset(l_aiBuf, 0, size);
        DCFlushRange(l_aiBuf, size);
        AXSetVoiceState(l_voice.mpVpb, AX_VOICE_STOP);
        AXFreeVoice(l_voice.mpVpb);
        l_voice.mpVpb = NULL;
        l_voice.mState = SPEAKER_FREED;
        break;
    }
}

static void upsample(s16 *in, int count, s16 *out);

static BOOL fillSpeaker(void *dst, s16 *src, u32 size) {
    upsample(src, size / 8, (s16 *)dst);
    DCFlushRange(dst, size);
    return TRUE;
}

// AI DMA callback: keeps a copy of each output block (the echo-cancel reference).
static void dmaCallback() {
    void *last = l_lastDmaBuf;
    if (l_oldDmaCallback) {
        l_oldDmaCallback();
    }
    l_lastDmaBuf = OSPhysicalToCached(AIGetDMAStartAddr());
    AIInitDMA(l_dmaBuf[l_dmaIdx], sizeof(l_dmaBuf[0]));
    if (last) {
        DCInvalidateRange(last, sizeof(l_dmaBuf[0]));
        memcpy(l_dmaBuf[l_dmaIdx], last, sizeof(l_dmaBuf[0]));
    } else {
        memset(l_dmaBuf[l_dmaIdx], 0, sizeof(l_dmaBuf[0]));
    }
    s32 n = fn_802290D4(l_dmaBuf[l_dmaIdx], 0xC0, 0, l_refBuf, l_refReset);
    if (n > 0) {
        if (l_muted) {
            memset(l_refBuf, 0, n * 2);
        }
        fn_80228F98(l_refBuf, n);
        l_refReset = FALSE;
    }
    DCFlushRange(l_dmaBuf[l_dmaIdx], sizeof(l_dmaBuf[0]));
    l_dmaIdx ^= 1;
}

static inline s16 clamp16(int value) {
    if (value > 0x7FFF) {
        return 0x7FFF;
    }
    if (value < -0x8000) {
        return -0x8000;
    }
    return value;
}

// 4x upsampling: each input sample gives 4 output samples, one per FIR phase. The last 31
// input samples are kept for the next call.
static void upsample(s16 *in, int count, s16 *out) {
    int n;
    int i;
    const s16 *src;
    const s16 *hist;
    int j;
    const s16 *h;
    int acc0;
    int acc1;
    int acc2;
    int acc3;
    s16 v0;
    s16 v1;
    s16 v2;
    s16 v3;
    hist = l_history;
    src = in;
    for (i = 0; i < 31; i++) {
        h = hist++;
        acc0 = 0;
        acc1 = 0;
        acc2 = 0;
        acc3 = 0;
        n = 31 - i;
        for (j = 0; j < n; j++) {
            acc0 += *h * (l_firCoef[0][j] << 2);
            acc1 += *h * (l_firCoef[1][j] << 2);
            acc2 += *h * (l_firCoef[2][j] << 2);
            acc3 += *h++ * (l_firCoef[3][j] << 2);
        }
        h = in;
        for (; j < 32; j++) {
            acc0 += *h * (l_firCoef[0][j] << 2);
            acc1 += *h * (l_firCoef[1][j] << 2);
            acc2 += *h * (l_firCoef[2][j] << 2);
            acc3 += *h++ * (l_firCoef[3][j] << 2);
        }
        acc0 = (acc0 + 0x4000) >> 15;
        acc1 = (acc1 + 0x4000) >> 15;
        acc2 = (acc2 + 0x4000) >> 15;
        acc3 = (acc3 + 0x4000) >> 15;
        v0 = clamp16(acc0);
        v1 = clamp16(acc1);
        v2 = clamp16(acc2);
        v3 = clamp16(acc3);
        out[0] = v0;
        out[1] = v1;
        out[2] = v2;
        out[3] = v3;
        out += 4;
    }
    for (; i < count; i++) {
        h = src++;
        acc0 = 0;
        acc1 = 0;
        acc2 = 0;
        acc3 = 0;
        for (j = 0; j < 32; j++) {
            acc0 += *h * (l_firCoef[0][j] << 2);
            acc1 += *h * (l_firCoef[1][j] << 2);
            acc2 += *h * (l_firCoef[2][j] << 2);
            acc3 += *h++ * (l_firCoef[3][j] << 2);
        }
        acc0 = (acc0 + 0x4000) >> 15;
        acc1 = (acc1 + 0x4000) >> 15;
        acc2 = (acc2 + 0x4000) >> 15;
        acc3 = (acc3 + 0x4000) >> 15;
        v0 = clamp16(acc0);
        v1 = clamp16(acc1);
        v2 = clamp16(acc2);
        v3 = clamp16(acc3);
        out[0] = v0;
        out[1] = v1;
        out[2] = v2;
        out[3] = v3;
        out += 4;
    }
    for (int k = 0; k < 31; k++) {
        l_history[k] = in[count - 31 + k];
    }
}

static s32 updateMic() {
    s32 status = fn_80225FA8();
    if (status == PMIC_ERR_NO_DEVICE) {
        l_micState = MIC_CLOSED;
        l_micConnected = FALSE;
        return status;
    }
    if (status == PMIC_ERR_NONE) {
        l_micConnected = TRUE;
    } else {
        l_micState = MIC_ACTIVE;
        l_micConnected = FALSE;
    }

    if (l_idle || l_stopped) {
        if (l_micState == MIC_ACTIVE) {
            l_micState = MIC_STOP;
        } else if (l_micState == MIC_START) {
            l_micState = MIC_STOPPED;
        }
        if (l_stopped) {
            if (l_micState == MIC_STOPPED) {
                l_micState = MIC_CLOSE;
            } else if (l_micState == MIC_OPEN) {
                l_micState = MIC_CLOSED;
            }
        }
    } else {
        if (l_micState == MIC_STOPPED) {
            l_micState = MIC_START;
        } else if (l_micState == MIC_CLOSED) {
            l_micState = MIC_OPEN;
        }
    }

    s32 result;
    switch (l_micState) {
    case MIC_OPEN:
        l_pmicResult = 1;
        result = fn_8022607C(pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_OPEN;
        } else if (result != PMIC_ERR_BUSY && result != PMIC_ERR_RETRY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_OPEN:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_SET_2;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_OPEN;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_SET_2:
        l_pmicResult = 1;
        result = fn_80229A2C(0, pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_2;
        } else if (result != PMIC_ERR_BUSY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_2:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_SET_6;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_SET_2;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_SET_4:
        l_pmicResult = 1;
        result = fn_80229B04(l_pmicParam4, pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_4;
        } else if (result != PMIC_ERR_BUSY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_4:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_SET_6;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_SET_4;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_SET_6:
        l_pmicResult = 1;
        result = fn_8022A038(l_pmicParam6, pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_6;
        } else if (result != PMIC_ERR_BUSY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_6:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_START;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_SET_6;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_START:
        l_pmicResult = 1;
        result = fn_80226234(pmicStartCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_START;
        } else if (result != PMIC_ERR_BUSY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_START:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_ACTIVE;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_START;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_STOP:
        l_pmicResult = 1;
        result = fn_80226348(pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_STOP;
        } else if (result != PMIC_ERR_BUSY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_STOP:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_STOPPED;
        } else if (l_pmicResult == PMIC_ERR_RETRY) {
            l_micState = MIC_STOP;
        } else {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_CLOSE:
        l_pmicResult = 1;
        result = fn_802261A4(pmicCallback, NULL);
        if (result == PMIC_ERR_NONE) {
            l_micState = MIC_WAIT_CLOSE;
        } else if (result != PMIC_ERR_BUSY && result != PMIC_ERR_RETRY) {
            l_micState = MIC_ACTIVE;
        }
        break;
    case MIC_WAIT_CLOSE:
        if (l_pmicResult > 0) {
            break;
        }
        if (l_pmicResult == 0) {
            l_micState = MIC_CLOSED;
        } else {
            l_micState = l_pmicResult == PMIC_ERR_RETRY ? MIC_CLOSE : MIC_ACTIVE;
        }
        break;
    case MIC_STOPPED:
    case MIC_CLOSED:
    case MIC_ACTIVE:
        break;
    }
}

static void pmicCallback(s32 result) {
    l_pmicResult = result;
}

static void pmicStartCallback(s32 result) {
    l_pmicResult = result;
    fn_8036D510(result);
}

// The PMIC preset for the number of members (3 uses the 6-member preset and reports 6).
static u8 selectPreset(u8 count) {
    switch (count) {
    case 2:
        fn_80229EF0(lbl_80521120);
        break;
    case 3:
        fn_80229EF0(lbl_80521160);
        count = 6;
        break;
    case 4:
        fn_80229EF0(lbl_805211E0);
        break;
    case 5:
        fn_80229EF0(lbl_80521140);
        break;
    case 6:
        fn_80229EF0(lbl_80521160);
        break;
    case 7:
        fn_80229EF0(lbl_80521180);
        break;
    case 8:
        fn_80229EF0(lbl_805211A0);
        break;
    case 9:
        fn_80229EF0(lbl_805211C0);
        break;
    default:
        return 0;
    }
    return count;
}

} // namespace dVoiceChat
