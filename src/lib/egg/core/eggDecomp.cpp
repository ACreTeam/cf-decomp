#include <lib/egg/core/eggDecomp.h>

#include <lib/egg/core/eggHeap.h>

namespace EGG {

void *Decomp::sWorkBuffer;
int Decomp::sBitCount[2];
int Decomp::sTreeBase[2];
int Decomp::sReadPos[2];
u32 Decomp::sBitBuf[2];

static inline u32 readBE32(const u8 *p) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

void Decomp::initWorkBuffer(Heap *heap) {
    if (sWorkBuffer == NULL) {
        if (heap == NULL) {
            heap = Heap::getCurrentHeap();
        }
        sWorkBuffer = Heap::alloc(0x9008, 32, heap);
    }
}

s32 Decomp::decodeASH(const u8 *src, u8 *dst) {
    u16 *work = (u16 *)sWorkBuffer;
    u16 *left9 = work;
    u16 *right9 = (u16 *)((u8 *)left9 + 0x7fe);
    u16 *left11 = (u16 *)((u8 *)right9 + 0x7fe);
    u16 *right11 = (u16 *)((u8 *)left11 + 0x1ffe);
    u16 *tmp = (u16 *)((u8 *)right11 + 0x1ffe);

    u32 size = (src[5] << 16) | (src[6] << 8) | src[7];
    u32 offset2 = readBE32(src + 8);

    sReadPos[0] = 12;
    sReadPos[1] = offset2;
    sBitCount[0] = 0;
    sBitCount[1] = 0;
    sTreeBase[0] = 0x200;
    sTreeBase[1] = 0x800;

    getBits(src, 32, 0);
    getBits(src, 32, 1);

    int root9 = readTree9(src, left9, right9, tmp);
    int root11 = readTree11(src, left11, right11, tmp);

    u32 out = 0;
    while (out < size) {
        int node = root9;
        while (node >= 0x200) {
            if (getBit(src, 0)) {
                node = right9[node];
            } else {
                node = left9[node];
            }
        }

        if (node < 0x100) {
            dst[out++] = node;
        } else {
            int dnode = root11;
            while (dnode >= 0x800) {
                if (getBit(src, 1)) {
                    dnode = right11[dnode];
                } else {
                    dnode = left11[dnode];
                }
            }

            int from = out - dnode - 1;
            int len = node - 0xfd;
            for (int i = 0; i < len; i++) {
                dst[out++] = dst[from++];
            }
        }
    }

    return out;
}

Decomp::CompressionType Decomp::checkCompressed(const u8 *src) {
    if (src[0] == 'Y' && src[1] == 'a' && src[2] == 'z') {
        return TYPE_SZS;
    }
    if (src[0] == 'A' && src[1] == 'S' && src[2] == 'H') {
        return TYPE_ASH;
    }
    if (src[0] == 'A' && src[1] == 'S' && src[2] == 'R') {
        return TYPE_ASR;
    }
    return TYPE_UNKNOWN;
}

s32 Decomp::getExpandSize(const u8 *src) {
    switch (checkCompressed(src)) {
    case TYPE_SZS:
        return (src[4] << 24) | (src[5] << 16) | (src[6] << 8) | src[7];
    case TYPE_ASH:
        return (src[5] << 16) | (src[6] << 8) | src[7];
    case TYPE_ASR:
        return (src[5] << 16) | (src[6] << 8) | src[7];
    default:
        return -1;
    }
}

u32 Decomp::getBits(const u8 *src, int count, int stream) {
    int bitCount = sBitCount[stream];
    int pos = sReadPos[stream];
    u32 buf = sBitBuf[stream];
    int total = bitCount + count;

    if (total > 32) {
        u32 next = readBE32(src + pos);
        sBitCount[stream] = total - 32;
        sReadPos[stream] = pos + 4;
        sBitBuf[stream] = next << (total - 32);
        return (buf >> (32 - count)) | (next >> (64 - count - bitCount));
    } else if (total == 32) {
        u32 next = readBE32(src + pos);
        sReadPos[stream] = pos + 4;
        sBitCount[stream] = 0;
        sBitBuf[stream] = next;
        return buf >> (32 - count);
    } else {
        sBitBuf[stream] = buf << count;
        sBitCount[stream] = total;
        return buf >> (32 - count);
    }
}

int Decomp::getBit(const u8 *src, int stream) {
    int bitCount = sBitCount[stream];
    int pos = sReadPos[stream];
    u32 buf = sBitBuf[stream];
    int bit = buf >> 31;

    if (bitCount == 31) {
        sReadPos[stream] = pos + 4;
        sBitCount[stream] = 0;
        sBitBuf[stream] = readBE32(src + pos);
    } else {
        sBitBuf[stream] = buf << 1;
        sBitCount[stream] = bitCount + 1;
    }
    return bit;
}

int Decomp::readTree9(const u8 *src, u16 *left, u16 *right, u16 *tmp) {
    int next = sTreeBase[0];
    int node = next;
    int sp = 0;

    while (true) {
        if (getBit(src, 0)) {
            tmp[sp] = node | 0x8000;
            next++;
            tmp[sp + 1] = node | 0x4000;
            node++;
            sp += 2;
            continue;
        }

        int leaf = getBits(src, 9, 0);
        int i = sp * 2;
        while (true) {
            i -= 2;
            sp--;
            u16 entry = tmp[i / 2];
            if (entry & 0x8000) {
                right[entry & 0x3fff] = leaf;
                leaf = entry & 0x3fff;
                if (sp == 0) {
                    return leaf;
                }
            } else {
                left[entry & 0x3fff] = leaf;
                node = next;
                break;
            }
        }
    }
}

int Decomp::readTree11(const u8 *src, u16 *left, u16 *right, u16 *tmp) {
    int next = sTreeBase[1];
    int node = next;
    int sp = 0;

    while (true) {
        if (getBit(src, 1)) {
            tmp[sp] = node | 0x8000;
            next++;
            tmp[sp + 1] = node | 0x4000;
            node++;
            sp += 2;
            continue;
        }

        int leaf = getBits(src, 11, 1);
        int i = sp * 2;
        while (true) {
            i -= 2;
            sp--;
            u16 entry = tmp[i / 2];
            if (entry & 0x8000) {
                right[entry & 0x3fff] = leaf;
                leaf = entry & 0x3fff;
                if (sp == 0) {
                    return leaf;
                }
            } else {
                left[entry & 0x3fff] = leaf;
                node = next;
                break;
            }
        }
    }
}

} // namespace EGG
