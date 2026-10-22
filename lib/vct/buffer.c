#include <revolution/os.h>

#include "udp_buffer.h"
#include "vct.h"
#include "vct_main.h"

void MI_CpuFill32(u32 data, void* dest, u32 size);

static UDPPacket* sFreeBuffer = NULL;
static int sBufferCount = 0;
static u32 sBufferSize = 0;
static UDPPacket* sBuffer = NULL;

BOOL vct_init_packet_buffer(void* buffer, u32 size) {
    int i;
    u32 count = size / sizeof(UDPPacket);

    sBuffer = buffer;

    if (count < VCT_MIN_AUDIO_BUFFER_COUNT || VCT_MAX_AUDIO_BUFFER_COUNT < count) {
        VCTi_Report("Audio buffer is too large or too small.(buffer count is %d)\n", count);
        return FALSE;
    }

    sBufferSize = size;
    sBufferCount = count;
    VCTi_Report("vct_init_packet_buffer:buffer count:%d, size:%d, sizeof(udp_packet):%d\n", count, size, sizeof(UDPPacket));

    if (sBuffer == NULL) {
        return FALSE;
    }

    if ((u32)sBuffer & 31) {
        VCTi_Report("Audio buffer address is not alignment with 32byte.\n");
        sBuffer = NULL;
        return FALSE;
    }

    MI_CpuFill32(0, sBuffer, size);

    for (i = 0; i < count - 1; i++) {
        sBuffer[i].free = &sBuffer[i + 1];
    }

    sBuffer[count - 1].free = NULL;
    sFreeBuffer = sBuffer;
    return TRUE;
}

void vct_reset_packet_buffer(void) {
    u32 i;

    MI_CpuFill32(0, sBuffer, sBufferSize);

    for (i = 0; i < sBufferCount - 1; i++) {
        sBuffer[i].free = &sBuffer[i + 1];
    }

    sBuffer[sBufferCount - 1].free = NULL;
    sFreeBuffer = sBuffer;
}

void vct_cleanup_packet_buffer(void) {
    sFreeBuffer = NULL;
    sBuffer = NULL;
}

UDPPacket* vct_alloc_packet_buffer(void) {
    UDPPacket* p = NULL;
    int sIntr = OSDisableInterrupts();

    if (sFreeBuffer != NULL) {
        p = sFreeBuffer;
        sFreeBuffer = sFreeBuffer->free;
    }

    OSRestoreInterrupts(sIntr);
    return p;
}

void vct_free_packet_buffer(UDPPacket* buf) {
    int sIntr = OSDisableInterrupts();

    buf->free = sFreeBuffer;
    sFreeBuffer = buf;
    buf->prev = NULL;
    buf->next = NULL;

    OSRestoreInterrupts(sIntr);
}

int vct_count_free_packet_buffer(void) {
    int n = 0;
    UDPPacket* p = sFreeBuffer;

    while (p != NULL) {
        n++;
        p = p->free;
    }

    return n;
}
