#ifndef VCT_UDP_BUFFER_H
#define VCT_UDP_BUFFER_H

#include <revolution/os.h>
#include <revolution/types.h>

typedef struct UDPPacket UDPPacket;

struct UDPPacket {
    /* 0x00 */ UDPPacket* free;
    /* 0x04 */ UDPPacket* prev;
    /* 0x08 */ UDPPacket* next;
    /* 0x0C */ int codec;
    /* 0x10 */ u8 remote;
    /* 0x11 */ u8 channel;
    /* 0x12 */ u8 _12;
    /* 0x13 */ u8 flags;
    /* 0x14 */ u32 length;
    /* 0x18 */ u32 sequence;
    /* 0x1C */ u8 buffer[0xE50];
    /* 0xE6C */ u8* samples;
    /* 0xE70 */ int marker;
    /* 0xE74 */ u32 timestamp;
    /* 0xE78 */ u64 receive_time;
    /* 0xE80 */ u64 performance_time;
}; // size 0xE88

BOOL vct_init_packet_buffer(void* buffer, u32 size);
void vct_reset_packet_buffer(void);
void vct_cleanup_packet_buffer(void);
UDPPacket* vct_alloc_packet_buffer(void);
void vct_free_packet_buffer(UDPPacket* buf);
int vct_count_free_packet_buffer(void);

#endif
