#include <lib/egg/core/eggDecomp.h>

#include <lib/egg/core/eggHeap.h>

namespace EGG {

void *Decomp::sWorkBuffer;
int Decomp::sBitCount[2];
int Decomp::sTreeBase[2];
int Decomp::sReadPos[2];
u32 Decomp::sBitBuf[2];

static inline u32 readBE32(u8 *p) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

void Decomp::createWorkArea(Heap *heap) {
    if (sWorkBuffer == NULL) {
        if (heap == NULL) {
            heap = Heap::getCurrentHeap();
        }
        sWorkBuffer = Heap::alloc(0x9008, 32, heap);
    }
}

s32 Decomp::decodeASH(const u8 *src, u8 *dst) {
    int node9, dstIdx, root9, root11, node11;

    u16 *left9, *right9;
    u16 *left11, *right11;
    u16 *tmp;

    u32 expandSize = src[5] << 16 | src[6] << 8 | src[7];
    u32 backRefOfs = src[8] << 24 | src[9] << 16 | src[10] << 8 | src[11];

    left9 = reinterpret_cast<u16 *>(sWorkBuffer);
    right9 = left9 + 0x3FF;
    left11 = right9 + 0x3FF;
    right11 = left11 + 0xFFF;
    tmp = right11 + 0xFFF;

    sReadPos[0] = 0xC;
    sReadPos[1] = backRefOfs;

    sBitCount[0] = sBitCount[1] = 0;

    sTreeBase[0] = 0x200;
    sTreeBase[1] = 0x800;

    getBitsCode(src, 32, 0);
    getBitsCode(src, 32, 1);

    root9 = readTree9(src, left9, right9, tmp);
    root11 = readTree12(src, left11, right11, tmp);

    dstIdx = 0;
    while (dstIdx < expandSize) {
        for (node9 = root9; node9 >= 0x200;) {
            if (getBit1c(src, 0) != 0) {
                node9 = right9[node9];
            } else {
                node9 = left9[node9];
            }
        }

        if (node9 < 0x100) {
            dst[dstIdx++] = static_cast<u8>(node9);
            continue;
        }

        for (node11 = root11; node11 >= 0x800;) {
            if (getBit1c(src, 1) != 0) {
                node11 = right11[node11];
            } else {
                node11 = left11[node11];
            }
        }

        int runIdx = dstIdx - node11 - 1;
        node9 -= 253;

        for (; node9 > 0; node9--, dstIdx++, runIdx++) {
            dst[dstIdx] = dst[runIdx];
        }
    }

    return dstIdx;
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

u32 Decomp::getBitsCode(const u8 *src, int count, int stream) {
    int byteIdx = sReadPos[stream];
    int bitIdx = sBitCount[stream];
    u32 bits = sBitBuf[stream];

    int code = 0;

    if (bitIdx + count > 32) {
        u32 newBits = src[byteIdx + 0] << 24 | src[byteIdx + 1] << 16 | src[byteIdx + 2] << 8 | src[byteIdx + 3];

        int oldPos = 32 - count;
        int newPos = 64 - count - bitIdx;
        code = bits >> oldPos | newBits >> newPos;

        bitIdx += count - 32;

        sBitBuf[stream] = newBits << bitIdx;
        sBitCount[stream] = bitIdx;
        sReadPos[stream] = byteIdx + sizeof(u32);
    } else if (bitIdx + count == 32) {
        code = bits >> (32 - count);
        sBitBuf[stream] = src[byteIdx + 0] << 24 | src[byteIdx + 1] << 16 | src[byteIdx + 2] << 8 | src[byteIdx + 3];
        sReadPos[stream] = byteIdx + sizeof(u32);
        sBitCount[stream] = 0;
    } else {
        code = bits >> (32 - count);
        sBitBuf[stream] = bits << count;
        sBitCount[stream] = bitIdx + count;
    }

    return code;
}

int Decomp::getBit1c(const u8 *src, int stream) {
    int byteIdx = sReadPos[stream];
    int bitIdx = sBitCount[stream];
    u32 bits = sBitBuf[stream];

    int code = bits >> 31;

    if (bitIdx == 31) {
        sBitBuf[stream] = src[byteIdx + 0] << 24 | src[byteIdx + 1] << 16 | src[byteIdx + 2] << 8 | src[byteIdx + 3];
        sReadPos[stream] = byteIdx + sizeof(u32);
        sBitCount[stream] = 0;
    } else {
        sBitBuf[stream] = bits << 1;
        sBitCount[stream] = bitIdx + 1;
    }

    return code;
}

int Decomp::readTree9(const u8 *src, u16 *left, u16 *right, u16 *tmp) {
    u32 index, index2;
    index2 = sTreeBase[0];
    index = index2;

    int sp = 0;

    while (true) {
        if (getBit1c(src, 0) != 0) {
            tmp[sp] = 0x8000 | index;
            tmp[sp + 1] = 0x4000 | index;
            index++;
            index2++;
            sp += 2;
            continue;
        }

        int code = getBitsCode(src, 9, 0);

        while (true) {
            u16 node = tmp[--sp];

            if (node & 0x8000) {
                right[node & 0x3fff] = code;
                code = node & 0x3fff;

                if (sp == 0) {
                    return code;
                }
            } else {
                left[node & 0x3fff] = code;
                index = index2;
                break;
            }
        }
    }
}

int Decomp::readTree12(const u8 *src, u16 *left, u16 *right, u16 *tmp) {
    u32 index, index2;
    index2 = sTreeBase[1];
    index = index2;

    int sp = 0;

    while (true) {
        if (getBit1c(src, 1) != 0) {
            tmp[sp] = 0x8000 | index;
            tmp[sp + 1] = 0x4000 | index;
            index++;
            index2++;
            sp += 2;
            continue;
        }

        int code = getBitsCode(src, 11, 1);

        while (true) {
            u16 node = tmp[--sp];

            if (node & 0x8000) {
                right[node & 0x3fff] = code;
                code = node & 0x3fff;

                if (sp == 0) {
                    return code;
                }
            } else {
                left[node & 0x3fff] = code;
                index = index2;
                break;
            }
        }
    }
}

} // namespace EGG
