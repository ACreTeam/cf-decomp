#include <private/cx.h>
#include <revolution/cx.h>

u32 CXGetUncompressedSize(const void* src) {
    u32 size = CXiConvertEndian32_(IN_BUFFER_AT(u32, src, 0)) >> 8;
    if (!size) {
        size = CXiConvertEndian32_(IN_BUFFER_AT(u32, src, 4));
    }

    return size;
}

void CXUncompressLZ(const void* src, void* dst) {
    const u8* pSrc = src;
    u8* pDst = dst;

    u32 size = CXiConvertEndian32_(IN_BUFFER_AT(u32, pSrc, 0)) >> 8;
    BOOL stat = (IN_BUFFER_AT(u8, pSrc, 0) & 0x0F) ? TRUE : FALSE;
    pSrc += sizeof(u32);

    if (!size) {
        size = CXiConvertEndian32_(IN_BUFFER_AT(u32, pSrc, 0));
        pSrc += sizeof(u32);
    }

    while (size) {
        u32 i;
        u32 flags = *pSrc++;

        for (i = 0; i < 8; ++i) {
            if (!(flags & 0x80)) {
                *pDst++ = *pSrc++;
                size--;
            } else {
                s32 length = *pSrc >> 4;
                s32 offset;

                if (!stat) {
                    length += 3;
                } else {
                    if (length == 0x01) {
                        length = (*pSrc++ & 0x0F) << 12;
                        length |= *pSrc++ << 4;
                        length |= *pSrc >> 4;
                        length += 0x111;
                    } else if (length == 0x00) {
                        length = (*pSrc++ & 0x0F) << 4;
                        length |= *pSrc >> 4;
                        length += 0x11;
                    } else {
                        length += 0x01;
                    }
                }

                offset = (*pSrc++ & 0x0F) << 8;
                offset = (offset | *pSrc++) + 1;

                if (length > size) {
                    length = size;
                }

                size -= length;

                do {
#if defined(__MWERKS__)
                    // ERRATUM: Unsequenced modification and access to pDst
                    /* Here, the compiler has decided to evaluate the left hand
                     * side pDst first, then the right hand side pDst, then the
                     * side effect of the post-increment on the left hand side
                     * pDst.
                     */
                    *pDst++ = pDst[-offset];
#else
                    // equivalent defined version
                    *pDst = pDst[-offset];
                    ++pDst;
#endif
                } while (--length > 0);
            }

            if (!size) {
                break;
            }

            flags <<= 1;
        }
    }
}

void CXUncompressHuffman(const void *src, void *dst) {
    const u8 *pSrc = src;
    u8 *pDst = dst;

    s32 size = CXiConvertEndian32_(IN_BUFFER_AT(u32, pSrc, 0)) >> 8;

    const u8 *treep = size ? pSrc + 4 : pSrc + 8;
    const u8 *treeStartp = treep + 1;

    u32 dataBit = *pSrc & 0x0F;
    u32 destTmp = 0;
    u32 destTmpCount = 0;
    u32 destTmpDataNum = (dataBit & 0x07) + 4;

    if (!size) {
        size = CXiConvertEndian32_(IN_BUFFER_AT(u32, pSrc, 4));
    }

    pSrc = treep + ((*treep + 1) << 1);
    treep = treeStartp;

    while (size > 0) {
        s32 srcCount = 32;
        u32 srcTmp;
#if defined(__MWERKS__)
        srcTmp = CXiConvertEndian32_(*((u32 *)(pSrc))++);
#else
        srcTmp = CXiConvertEndian32_(IN_BUFFER_AT(u32, pSrc, 0));
        pSrc += sizeof(u32);
#endif

        while (--srcCount >= 0) {
            u32 treeShift = srcTmp >> 31;
            u32 treeCheck = *treep;
            treeCheck <<= treeShift;

            treep = (const u8 *)((((u32)treep >> 1) << 1) +
                                 (((*treep & 0x3F) + 1) << 1) + treeShift);

            if (treeCheck & 0x80) {
                destTmp >>= dataBit;
                destTmp |= *treep << (32 - dataBit);
                treep = treeStartp;

                if (++destTmpCount == destTmpDataNum) {
#if defined(__MWERKS__)
                    *((u32 *)(pDst))++ = CXiConvertEndian32_(destTmp);
#else
                    *(u32 *)pDst = CXiConvertEndian32_(destTmp);
                    pDst += sizeof(u32);
#endif
                    size -= sizeof(u32);
                    destTmpCount = 0;
                }
            }

            if (size <= 0) {
                break;
            }

            srcTmp <<= 1;
        }
    }
}
