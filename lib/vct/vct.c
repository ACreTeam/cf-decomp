#include <revolution/os.h>

#include "audio.h"
#include "ssp.h"
#include "udp_buffer.h"
#include "vct.h"
#include "vct_main.h"

void* MI_CpuFill8(void* dest, int data, u32 size);

#define VCT_DEFAULT_FRAME_LENGTH 0x44
#define VCT_MIN_FRAME_LENGTH 0x44
#define VCT_MAX_FRAME_LENGTH 0xC0

static char id_string[] = "[SDK+Abiosso:DWC_VCT 2.0.2]";

enum HandleDataResult {
    HANDLE_DATA_RESULT_NO_CALLBACK = 0,
    HANDLE_DATA_RESULT_CALLBACK,
    HANDLE_DATA_RESULT_NOT_VCT_DATA,
};

static int initialized = FALSE;
static u32 sRecvPerFrame = 0;
static u32 sRecvStatA;
static u32 sRecvStatB;

VCTi_Globals vct;

BOOL VCTi_IsInitialized(void) {
    return initialized;
}

BOOL VCT_Init(VCTConfig* config) {
    OSRegisterVersion(id_string);

    if (config == NULL) {
        return FALSE;
    }
    if (initialized) {
        return TRUE;
    }
    if (config->mode != VCT_MODE_PHONE && config->mode != VCT_MODE_TRANSCEIVER && config->mode != VCT_MODE_CONFERENCE) {
        OSReport("Invalid VCT mode!!!");
        return FALSE;
    }
    if (config->callback == NULL) {
        return FALSE;
    }
    if (config->audioBuffer == NULL || config->audioBufferSize == 0) {
        return FALSE;
    }
    if ((int)config->audioBuffer % 32 != 0) {
        VCTi_Report("audioBuffer must alignment 32 byte\n");
        return FALSE;
    }

    MI_CpuFill8(&vct, 0, sizeof(VCTi_Globals));
    vct.callback = config->callback;
    vct.userData = config->userData;
    vct.SSPMode = config->mode;
    vct.listenMode = LISTEN_MODE_CLIENT;

    if (config->aid >= 32) {
        OSReport("Invalid aid");
    }
    if (config->aid >= 32) {
        return FALSE;
    }

    vct.myAID = config->aid;

    if (!vct_init_packet_buffer(config->audioBuffer, config->audioBufferSize)) {
        vct_cleanup_packet_buffer();
        return FALSE;
    }

    if (!vct_init_ssp(config)) {
        vct_cleanup_packet_buffer();
        return FALSE;
    }

    if (!VCTi_IsAdpcmCodec(config->codec)) {
        VCTi_Report("VCT_Init: invalid codec:%d\n", config->codec);
        vct_finish_ssp();
        vct_cleanup_packet_buffer();
        return FALSE;
    }

    if (config->frameLength == 0) {
        config->frameLength = VCT_DEFAULT_FRAME_LENGTH;
    }
    if (config->frameLength < VCT_MIN_FRAME_LENGTH || config->frameLength > VCT_MAX_FRAME_LENGTH) {
        VCTi_Report("VCT_Init: invalid frame length:%d\n", config->frameLength);
        vct_finish_ssp();
        vct_cleanup_packet_buffer();
        return FALSE;
    }

    if (!vct_init_audio(config->codec, config->frameLength)) {
        vct_finish_ssp();
        vct_cleanup_packet_buffer();
        return FALSE;
    }

    if (config->flags & VCT_CONFIG_FLAG_VAD) {
        VCT_EnableVAD(TRUE);
    } else {
        VCT_EnableVAD(FALSE);
    }
    if (config->flags & VCT_CONFIG_FLAG_ECHO_CANCEL) {
        VCT_EnableEchoCancel(TRUE);
    } else {
        VCT_EnableEchoCancel(FALSE);
    }
    if (config->flags & VCT_CONFIG_FLAG_HPF) {
        VCTi_SetHpfMode(TRUE);
    } else {
        VCTi_SetHpfMode(FALSE);
    }

    initialized = TRUE;
    sRecvStatB = 0;
    sRecvStatA = 0;
    return TRUE;
}

void VCT_Cleanup(void) {
    vct_finish_audio();
    vct_finish_ssp();
    vct_cleanup_packet_buffer();
    initialized = FALSE;
    vct.SSPMode = VCT_MODE_NULL;
}

void VCT_Main(void) {
    static u32 count = 0;

    if (!initialized) {
        return;
    }

    count++;

    if (!(count % 16)) {
        vct_check_timeout();
        vct_check_quality_alert_time();
    }

    if (!vct_flush_send_buffer()) {
        return;
    }

    while (vct_decode_audio_buffer() != 0) {
    }
}

BOOL VCT_HandleData(u8 aid, u8* buffer, int size) {
    VCTResult result;
    int ret = VCTi_HandleData(aid, buffer, size, &result);
    switch (ret) {
    case HANDLE_DATA_RESULT_CALLBACK:
        vct.callback(aid, result.event, result.session, vct.userData);
        return TRUE;
    case HANDLE_DATA_RESULT_NOT_VCT_DATA:
        return FALSE;
    case HANDLE_DATA_RESULT_NO_CALLBACK:
    default:
        return TRUE;
    }
}

int VCTi_HandleData(u8 aid, u8* buffer, int size, VCTResult* result) {
    u64 tick;

    if (buffer == NULL) {
        return HANDLE_DATA_RESULT_NO_CALLBACK;
    }
    if (((AudioHeader*)buffer)->magic != VCT_MAGIC_PACKET_HEADER) {
        return HANDLE_DATA_RESULT_NOT_VCT_DATA;
    }
    if (vct.SSPMode == VCT_MODE_NULL || !initialized) {
        return HANDLE_DATA_RESULT_NO_CALLBACK;
    }

    result->event = 0;
    result->session = NULL;
    tick = OSGetTime();

    if (!vct_check_hmac(buffer + size - 16, buffer, size - 16)) {
        VCTi_Report("HMAC authentication failure!\n");
        return HANDLE_DATA_RESULT_NO_CALLBACK;
    }
    size -= 16;

    if ((buffer[4] & 0xF0) == VCT_HEADER_AUDIO) {
        sRecvPerFrame++;
        vct_handle_audio(aid, buffer, size, tick);
        return HANDLE_DATA_RESULT_NO_CALLBACK;
    } else if (buffer[4] == 0 || buffer[4] == 0xFF) {
        if (vct_handle_ssp(aid, buffer, size, result)) {
            return HANDLE_DATA_RESULT_CALLBACK;
        } else {
            return HANDLE_DATA_RESULT_NO_CALLBACK;
        }
    } else {
        VCTi_Report("Unrecognized data received [%x]\n", buffer[4]);
        return HANDLE_DATA_RESULT_NO_CALLBACK;
    }
}
