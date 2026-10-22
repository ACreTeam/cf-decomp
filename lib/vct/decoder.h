#ifndef VCT_DECODER_H
#define VCT_DECODER_H

#include <revolution/types.h>

#include "udp_buffer.h"
#include "vct.h"

void vct_init_decoder(void);
void vct_flush_decoder(u32 ch);
void vct_flush_all_decoder(void);
int vct_insert_decoder(UDPPacket* packet);
UDPPacket* vct_top_decoder(void);
int vct_count_decoder_queue(u32 ch);
UDPPacket* vct_get_packet_from_decoder(u32 ch);
int vct_remove_packet_from_decoder(UDPPacket* packet);

#endif
