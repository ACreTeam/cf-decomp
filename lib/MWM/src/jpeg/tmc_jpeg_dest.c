// TMC JPEG encoder (RVL_MWM - TMC_JPEG070330).
// Function and type names are descriptive, not original.
#include <MWM/tmc_jpeg_internal.h>
#include <revolution/os.h>
#include <string.h>

// Number of bits left in the current output byte -> mask table
static const u8 s_padBits[9] = {0x00, 0x01, 0x03, 0x07, 0x0F,
                                0x1F, 0x3F, 0x7F, 0xFF};

void dest_init(TMCJPEGBitWriter *bw, u8 *buf, u32 size) {
    bw->start = buf;
    bw->cur = buf;
    bw->freeBits = 8;
    bw->size = size;
    bw->count = 0;
    bw->overflow = 0;
}

void dest_get_size(TMCJPEGBitWriter *bw, u32 *size) {
    *size = bw->count + (bw->cur - bw->start);
}

static inline void wrap_check(TMCJPEGBitWriter *bw) {
    if (bw->cur - bw->start >= bw->size) {
        bw->overflow = 1;
        bw->count += bw->cur - bw->start;
        bw->cur = bw->start;
    }
}

void emit_byte(TMCJPEGBitWriter *bw, u8 value) {
    *bw->cur++ = value;
    wrap_check(bw);
    *bw->cur = 0;
}

void emit_2bytes(TMCJPEGBitWriter *bw, u16 value) {
    *bw->cur++ = value >> 8;
    wrap_check(bw);
    *bw->cur++ = value;
    wrap_check(bw);
    *bw->cur = 0;
}

void emit_bytes(TMCJPEGBitWriter *bw, const u8 *data, int count) {
    int i;
    for (i = 0; i < count; i++) {
        wrap_check(bw);
        *bw->cur = *data++;
        bw->cur++;
    }
    *bw->cur = 0;
}

void flush_bits_emit_marker(TMCJPEGBitWriter *bw, u16 marker) {
    if (bw->freeBits != 8) {
        put_bits(bw, s_padBits[bw->freeBits], bw->freeBits);
    }
    wrap_check(bw);
    *bw->cur++ = marker >> 8;
    wrap_check(bw);
    *bw->cur++ = marker;
    wrap_check(bw);
    *bw->cur = 0;
}

void flush_bits_emit_eoi(TMCJPEGBitWriter *bw) {
    if (bw->freeBits != 8) {
        put_bits(bw, s_padBits[bw->freeBits], bw->freeBits);
    }
    wrap_check(bw);
    *bw->cur++ = 0xFF;
    wrap_check(bw);
    *bw->cur++ = 0xD9;
}

void select_put_bits(TMCJPEGEntropy *ent) {
    TMCJPEGBitWriter *bw = ent->bw;
    if ((u32)(bw->cur - bw->start) >= bw->size - 0x100) {
        ent->put = put_bits;
    } else {
        ent->put = put_bits_fast;
    }
}

void put_bits_fast(TMCJPEGBitWriter *bw, int bits, int nbits) {
    int acc = *bw->cur;
    int free = bw->freeBits;
    while (nbits > free) {
        int shifted = bits >> (nbits - free);
        acc |= shifted & s_padBits[free];
        *bw->cur++ = acc;
        if (acc == 0xFF) {
            *bw->cur++ = 0;
        }
        nbits -= free;
        free = 8;
        acc = 0;
    }
    if (nbits == free) {
        acc |= bits & s_padBits[nbits];
        bw->freeBits = 8;
        *bw->cur++ = acc;
        if (acc == 0xFF) {
            *bw->cur++ = 0;
        }
        *bw->cur = 0;
    } else {
        free -= nbits;
        acc |= (bits & s_padBits[nbits]) << free;
        *bw->cur = acc;
        bw->freeBits = free;
    }
}

void put_bits(TMCJPEGBitWriter *bw, int bits, int nbits) {
    int acc = *bw->cur;
    int free = bw->freeBits;
    while (nbits > free) {
        int shifted = bits >> (nbits - free);
        acc |= shifted & s_padBits[free];
        *bw->cur++ = acc;
        wrap_check(bw);
        if (acc == 0xFF) {
            *bw->cur++ = 0;
            wrap_check(bw);
        }
        nbits -= free;
        free = 8;
        acc = 0;
    }
    if (nbits == free) {
        acc |= bits & s_padBits[nbits];
        bw->freeBits = 8;
        *bw->cur++ = acc;
        wrap_check(bw);
        if (acc == 0xFF) {
            *bw->cur++ = 0;
            wrap_check(bw);
        }
        *bw->cur = 0;
    } else {
        free -= nbits;
        acc |= (bits & s_padBits[nbits]) << free;
        *bw->cur = acc;
        bw->freeBits = free;
    }
}
