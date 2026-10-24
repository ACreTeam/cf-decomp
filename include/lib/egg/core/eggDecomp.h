#pragma once

#include <types.h>

namespace EGG {
class Heap;

// ASH (Huffman + LZ) decompressor.
class Decomp {
public:
    enum CompressionType {
        TYPE_UNKNOWN = 0,
        TYPE_SZS = 1,
        TYPE_ASH = 2,
        TYPE_ASR = 3,
    };

    static void createWorkArea(Heap *heap);              // 8043EFD4
    static s32 decodeASH(const u8 *src, u8 *dst);        // 8043F020
    static CompressionType checkCompressed(const u8 *src); // 8043F258
    static s32 getExpandSize(const u8 *src);             // 8043F2DC

private:
    static u32 getBitsCode(const u8 *src, int count, int stream);      // 8043F38C
    static int getBit1c(const u8 *src, int stream);                  // 8043F494
    static int readTree9(const u8 *src, u16 *left, u16 *right, u16 *tmp);   // 8043F520
    static int readTree12(const u8 *src, u16 *left, u16 *right, u16 *tmp);  // 8043F5F4

    static void *sWorkBuffer;
    static int sBitCount[2];
    static int sTreeBase[2];
    static int sReadPos[2];
    static u32 sBitBuf[2];
};

} // namespace EGG
