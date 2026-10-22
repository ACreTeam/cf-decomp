#include "decoder.h"

#include <revolution/os.h>

#include "udp_buffer.h"
#include "vct.h"

static int sCount[VCT_MAX_AUDIO_STREAM] = {0};
static UDPPacket* sHead = NULL;
static UDPPacket* sTail = NULL;

void vct_init_decoder(void) {
    int sIntr = OSDisableInterrupts();
    int i;

    sTail = NULL;
    sHead = NULL;

    for (i = 0; i < VCT_MAX_AUDIO_STREAM; i++) {
        sCount[i] = 0;
    }

    OSRestoreInterrupts(sIntr);
}

void vct_flush_decoder(u32 ch) {
    int sIntr = OSDisableInterrupts();

    UDPPacket* p;
    while ((p = vct_get_packet_from_decoder(ch)) != NULL) {
        vct_remove_packet_from_decoder(p);
        vct_free_packet_buffer(p);
    }

    sCount[ch] = 0;
    OSRestoreInterrupts(sIntr);
}

void vct_flush_all_decoder(void) {
    int sIntr = OSDisableInterrupts();
    int i;

    UDPPacket* node = sHead;
    while (node != NULL) {
        UDPPacket* p = node;
        node = node->next;
        vct_free_packet_buffer(p);
    }

    sHead = NULL;
    sTail = NULL;

    for (i = 0; i < VCT_MAX_AUDIO_STREAM; i++) {
        sCount[i] = 0;
    }

    OSRestoreInterrupts(sIntr);
}

int vct_insert_decoder(UDPPacket* packet) {
    int sIntr = OSDisableInterrupts();

    if (sHead == NULL) {
        sHead = packet;
        packet->prev = NULL;
        sHead->next = NULL;
        sTail = sHead;
    } else {
        packet->prev = sTail;
        packet->next = NULL;
        sTail->next = packet;
        sTail = packet;
    }

    sCount[packet->channel]++;
    OSRestoreInterrupts(sIntr);
    return sCount[packet->channel];
}

UDPPacket* vct_top_decoder(void) {
    return sHead;
}

int vct_count_decoder_queue(u32 ch) {
    return sCount[ch];
}

inline UDPPacket* vct_get_packet_from_decoder(u32 ch) {
    UDPPacket* node = sHead;
    int sIntr = OSDisableInterrupts();

    while (node != NULL) {
        if (node->channel == ch) {
            OSRestoreInterrupts(sIntr);
            return node;
        } else {
            node = node->next;
        }
    }

    OSRestoreInterrupts(sIntr);
    return NULL;
}

int vct_remove_packet_from_decoder(UDPPacket* packet) {
    int sIntr = OSDisableInterrupts();

    if (packet->prev != NULL) {
        packet->prev->next = packet->next;
    } else {
        sHead = packet->next;
        if (sHead != NULL) {
            sHead->prev = NULL;
        }
    }

    if (packet->next != NULL) {
        packet->next->prev = packet->prev;
    } else {
        sTail = packet->prev;
        if (sTail != NULL) {
            sTail->next = NULL;
        }
    }

    sCount[packet->channel]--;
    OSRestoreInterrupts(sIntr);
    return sCount[packet->channel];
}
