// TMC JPEG encoder (RVL_MWM - TMC_JPEG070330).
// Function and type names are descriptive, not original.
#include <MWM/tmc_jpeg_internal.h>
#include <revolution/os.h>
#include <string.h>

#define EMIT_BITS(code, size)                                                  \
    {                                                                          \
        free -= (size);                                                        \
        buf <<= (size);                                                        \
        buf += (code);                                                         \
        if (free < 16) {                                                       \
            put(bw, buf, 32 - free);                                           \
            buf = 0;                                                           \
            free = 32;                                                         \
        }                                                                      \
    }

int encode_one_block(int *block, TMCJPEGEntropy *ent) {
    u16 mask[17] = {0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
                    0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
                    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF};
    TMCJPEGHuffTable *huff;
    int temp;
    int nbits;
    int k;
    int run;
    int *recip;
    TMCJPEGBitWriter *bw = ent->bw;
    const int *order = ent->order + 1;
    TMCJPEGPutBitsFn put;
    int diff;
    int v;
    int buf;
    int free;
    int idx;
    int coef;
    int dc;
    int dcBits;
    int q;

    select_put_bits(ent);
    put = ent->put;
    recip = ent->recip;
    huff = ent->huff;

    dc = block[0];
    if (dc < 0) {
        dc = -dc;
        dc = -((recip[0] * dc + 0x8000) >> 16);
    } else {
        dc = (recip[0] * dc + 0x8000) >> 16;
    }

    diff = dc - ent->lastDc;
    ent->lastDc = dc;
    v = diff;
    if (diff < 0) {
        v = -diff;
        diff--;
    }
    dcBits = 0;
    while (v > 0) {
        v >>= 1;
        dcBits++;
    }
    if (dcBits >= huff[0].count) {
        return -2;
    }

    buf = huff[0].code[dcBits];
    buf <<= dcBits;
    buf |= diff & mask[dcBits];
    free = 32 - huff[0].size[dcBits] - dcBits;
    if (free < 16) {
        put(bw, buf, 32 - free);
        buf = 0;
        free = 32;
    }

    run = 0;
    for (k = 1; k < 64; k++) {
        idx = *order++;
        coef = block[idx];
        q = recip[idx];
        if (coef <= 0) {
            temp = -(((-coef) * q + 0x8000) >> 16);
        } else {
            temp = (coef * q + 0x8000) >> 16;
        }
        if (temp == 0) {
            run++;
            continue;
        }

        while (run > 15) {
            run -= 16;
            EMIT_BITS(huff[1].code[0xF0], huff[1].size[0xF0]);
        }

        v = temp;
        if (temp < 0) {
            v = -temp;
            temp--;
        }
        nbits = 0;
        do {
            nbits++;
        } while (v >> nbits);
        if (nbits >= huff[1].count) {
            return -2;
        }

        idx = (run << 4) + nbits;
        run = 0;
        EMIT_BITS(huff[1].code[idx], huff[1].size[idx]);
        free -= nbits;
        buf = (buf << nbits) | (temp & mask[nbits]);
        if (free < 16) {
            put(bw, buf, 32 - free);
            buf = 0;
            free = 32;
        }
    }

    if (run > 0) {
        free -= huff[1].size[0];
        buf <<= huff[1].size[0];
        buf += huff[1].code[0];
        put(bw, buf, 32 - free);
    } else if (free != 32) {
        put(bw, buf, 32 - free);
    }
    return 0;
}

void select_encode_mcu_padded(TMCJPEGWork *work, int sampling) {
    switch (sampling) {
    case 0:
        work->encodeMcuPadded = encode_mcu_padded_411;
        break;
    case 2:
        work->encodeMcuPadded = encode_mcu_padded_420;
        break;
    case 1:
        work->encodeMcuPadded = encode_mcu_padded_422;
        break;
    }
}

int encode_mcus_padded(TMCJPEGWork *work, TMCJPEGCtx *ctx, int count) {
    TMCJPEGSource *src = &work->src;
    int curX;
    int curY;
    int lastX;
    int lastY;
    int total;
    int done;
    int restart;
    int ret;
    int mcusX;
    int mcusY;

    done = 0;
    mcusX = src->mcusX;
    restart = work->restartCounter;
    mcusY = src->mcusY;
    lastX = mcusX - 1;
    curY = src->curY;
    total = mcusX * mcusY;
    curX = src->curX;
    lastY = mcusY - 1;

    while (curY < lastY && done < count) {
        while (curX < lastX && done < count) {
            ret = work->encodeMcu(curX, curY, src);
            if (ret < 0) {
                return ret;
            }
            done--;
            curX++;
            if (--restart == 0) {
                emit_restart(work);
                restart = work->restartInterval;
            }
        }
        if (done < count) {
            ret = work->encodeMcuPadded(curX, curY, 0, src);
            if (ret < 0) {
                return ret;
            }
            curX = 0;
            done--;
            curY++;
            if (--restart == 0) {
                emit_restart(work);
                restart = work->restartInterval;
            }
        }
    }

    while (curX < lastX && done < count) {
        ret = work->encodeMcuPadded(curX, curY, 1, src);
        if (ret < 0) {
            return ret;
        }
        done--;
        curX++;
        if (--restart == 0) {
            emit_restart(work);
            restart = work->restartInterval;
        }
    }

    if (done < count) {
        ret = work->encodeMcuPadded(curX, curY, 2, src);
        if (ret < 0) {
            return ret;
        }
        curX = 0;
        curY++;
        if (--restart == 0) {
            emit_restart(work);
            restart = work->restartInterval;
        }
        flush_bits_emit_eoi(&work->bw);
        if (work->bw.overflow != 0) {
            dest_get_size(&work->bw, &ctx->outSize);
            return -3;
        }
    }

    src->curX = curX;
    ctx->curX = curX;
    src->curY = curY;
    ctx->curY = curY;
    work->restartCounter = restart;
    dest_get_size(&work->bw, &ctx->outSize);
    return total - curX - curY * src->mcusX;
}

int encode_mcu_padded_411(int x, int y, int kind, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *buf = mcu;
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->loadPadded(buf, src->ctx, x * 32, y * 8);
    if (ret < 0) {
        return ret;
    }
    luma = buf + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, 0x20);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, 0x20);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 16, block, 0x20);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 24, block, 0x20);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0x104, block, 8);
    ret = encode_one_block(block, &src->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0x144, block, 8);
    ret = encode_one_block(block, &src->cr.ent);
    return ret < 0 ? ret : 0;
}

int encode_mcu_padded_420(int x, int y, int kind, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *buf = mcu;
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->loadPadded(buf, src->ctx, x * 16, y * 16);
    if (ret < 0) {
        return ret;
    }
    luma = buf + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 128, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 136, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0x104, block, 8);
    ret = encode_one_block(block, &src->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0x144, block, 8);
    ret = encode_one_block(block, &src->cr.ent);
    return ret < 0 ? ret : 0;
}

int encode_mcu_padded_422(int x, int y, int kind, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *buf = mcu;
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->loadPadded(buf, src->ctx, x * 16, y * 8);
    if (ret < 0) {
        return ret;
    }
    luma = buf + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, 0x10);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0x84, block, 8);
    ret = encode_one_block(block, &src->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)buf + 0xC4, block, 8);
    ret = encode_one_block(block, &src->cr.ent);
    return ret < 0 ? ret : 0;
}

int write_frame_header(TMCJPEGWork *work) {
    TMCJPEGBitWriter *bw = &work->bw;
    u32 height;
    u32 width;
    int vSamp;
    int hSamp;

    vSamp = 1 << work->vShift;
    hSamp = 1 << work->hShift;
    height = work->height;
    width = work->width;

    emit_2bytes(bw, 0xFFC0);
    emit_2bytes(bw, 0x11);
    emit_byte(bw, 8);
    emit_2bytes(bw, height);
    emit_2bytes(bw, width);
    emit_byte(bw, 3);
    emit_byte(bw, 0);
    put_bits(bw, hSamp, 4);
    put_bits(bw, vSamp, 4);
    emit_byte(bw, 0);
    emit_byte(bw, 1);
    emit_byte(bw, 0x11);
    emit_byte(bw, 1);
    emit_byte(bw, 2);
    emit_byte(bw, 0x11);
    emit_byte(bw, 1);
    return write_scan_header(work) < 0 ? -2 : 0;
}

int write_file_header(TMCJPEGWork *work) {
    TMCJPEGBitWriter *bw = &work->bw;
    int i;
    const int *qLum;
    const int *qChrom = work->quantChrom;
    const int *order = work->order;

    emit_2bytes(bw, 0xFFD8);
    emit_2bytes(bw, 0xFFDB);
    emit_2bytes(bw, 0x43);
    emit_byte(bw, 0);
    qLum = work->quantLum;
    for (i = 0; i < 64; i++) {
        emit_byte(bw, qLum[order[i]]);
    }
    emit_2bytes(bw, 0xFFDB);
    emit_2bytes(bw, 0x43);
    emit_byte(bw, 1);
    for (i = 0; i < 64; i++) {
        emit_byte(bw, qChrom[order[i]]);
    }
    emit_2bytes(bw, 0xFFC4);
    emit_2bytes(bw, 0x1F);
    emit_byte(bw, 0x00);
    emit_bytes(bw, work->stdTable[0], 16);
    emit_bytes(bw, work->stdTable[1], 12);
    emit_2bytes(bw, 0xFFC4);
    emit_2bytes(bw, 0x1F);
    emit_byte(bw, 0x01);
    emit_bytes(bw, work->stdTable[2], 16);
    emit_bytes(bw, work->stdTable[3], 12);
    emit_2bytes(bw, 0xFFC4);
    emit_2bytes(bw, 0xB5);
    emit_byte(bw, 0x10);
    emit_bytes(bw, work->stdTable[4], 16);
    emit_bytes(bw, work->stdTable[5], 162);
    emit_2bytes(bw, 0xFFC4);
    emit_2bytes(bw, 0xB5);
    emit_byte(bw, 0x11);
    emit_bytes(bw, work->stdTable[6], 16);
    emit_bytes(bw, work->stdTable[7], 162);
    if (work->restartInterval != 0) {
        emit_2bytes(bw, 0xFFDD);
        emit_2bytes(bw, 4);
        emit_2bytes(bw, work->restartInterval);
    }
    if (write_frame_header(work) < 0) {
        return -2;
    }
    return 0;
}
