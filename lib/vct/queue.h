#ifndef VCT_QUEUE_H
#define VCT_QUEUE_H

#include <revolution/types.h>

#include "udp_buffer.h"

void vct_init_audio_queue(u32 ch);
void vct_flush_audio_queue(u32 ch);
void vct_flush_all_audio_queue(void);
int vct_insert_audio_queue(UDPPacket* packet, u32 ch);
UDPPacket* vct_top_audio_queue(u32 ch);
int vct_pop_audio_queue(u32 ch);
int vct_count_audio_queue(u32 ch);

#endif
