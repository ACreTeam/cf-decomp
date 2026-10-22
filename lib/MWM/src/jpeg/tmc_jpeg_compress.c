// TMC JPEG encoder (RVL_MWM - TMC_JPEG070330).
// Function and type names are descriptive, not original.
#include <MWM/tmc_jpeg_internal.h>
#include <revolution/os.h>
#include <string.h>

int check_params(TMCJPEGParams *params, TMCJPEGWork *work) {
    int w = params->width;
    int h;
    int mode;

    if (w == 0) {
        return -1;
    }
    if (params->height == 0) {
        return -1;
    }
    work->width = w;
    h = params->height;
    work->height = h;
    work->restartInterval = params->restartInterval;
    mode = params->mode;
    work->mode = mode;
    switch (mode) {
    case 1: {
        u32 q = params->quality;
        if (q == 0 || q > 500) {
            return -1;
        }
        work->quality = q;
        break;
    }
    default:
        return -1;
    }

    switch (params->sampling) {
    case 0:
        work->sampling = 0;
        work->hShift = 2;
        work->vShift = 0;
        work->cbWidth = (work->width + 3) / 4;
        work->cbHeight = h;
        work->crWidth = (w + 3) / 4;
        work->crHeight = h;
        break;
    case 1:
        work->sampling = 1;
        work->hShift = 1;
        work->vShift = 0;
        work->cbWidth = (work->width + 1) / 2;
        work->cbHeight = h;
        work->crWidth = (w + 1) / 2;
        work->crHeight = h;
        break;
    case 2:
        work->sampling = 2;
        work->hShift = 1;
        work->vShift = 1;
        work->cbWidth = (work->width + 1) / 2;
        work->cbHeight = (work->height + 1) / 2;
        work->crWidth = (w + 1) / 2;
        work->crHeight = (h + 1) / 2;
        break;
    default:
        return -1;
    }

    if (params->format != GX_TF_RGB565 && params->format != GX_TF_RGBA8) {
        return -4;
    }
    work->src.ctx->format = params->format;
    return 0;
}

void initial_setup(TMCJPEGWork *work) {
    TMCJPEGBitWriter *bw;
    int w;
    int h;
    int cbW;
    int cbH;
    int crW;
    int crH;
    int *recipLum;
    int *recipChrom;
    const int *order;
    int remX;
    int remY;
    int minRecip;
    int i;

    bw = &work->bw;
    recipLum = work->recipLum;
    recipChrom = work->recipChrom;
    order = work->order;

    work->restartNum = 0;
    work->restartCounter = work->restartInterval;
    w = work->width;
    h = work->height;
    cbW = work->cbWidth;
    cbH = work->cbHeight;
    crW = work->crWidth;
    crH = work->crHeight;

    switch (work->sampling) {
    case 0:
        work->src.mcusX = (w + 31) >> 5;
        work->src.mcusY = (h + 7) >> 3;
        work->src.curX = 0;
        work->src.curY = 0;
        remX = w % 32;
        remY = h % 8;
        work->src.remX = remX;
        work->src.remY = remY;
        work->src.lum.width = w;
        work->src.lum.height = h;
        work->src.lum.lastW = remX != 0 ? remX : 32;
        work->src.lum.lastH = remY != 0 ? remY : 8;
        work->src.lum.blkW = 32;
        work->src.lum.blkH = 8;
        work->src.lum.blocks = 4;
        work->src.lum.sampling = 0;
        work->src.lum.ent.stride = 32;
        work->src.lum.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipLum;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.lum.ent.minRecip = minRecip;
        work->src.lum.ent.huff = &work->huff[0];
        work->src.lum.ent.recip = recipLum;
        work->src.lum.ent.order = order;
        work->src.lum.ent.bw = bw;

        work->src.cb.width = cbW;
        work->src.cb.height = cbH;
        work->src.cb.lastW = cbW % 8 != 0 ? cbW % 8 : 8;
        work->src.cb.lastH = cbH % 8 != 0 ? cbH % 8 : 8;
        work->src.cb.blkW = 8;
        work->src.cb.blkH = 8;
        work->src.cb.blocks = 1;
        work->src.cb.sampling = 0;
        work->src.cb.ent.stride = 8;
        work->src.cb.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipChrom;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.cb.ent.minRecip = minRecip;
        work->src.cb.ent.huff = &work->huff[2];
        work->src.cb.ent.recip = recipChrom;
        work->src.cb.ent.order = order;
        work->src.cb.ent.bw = bw;

        work->src.cr.width = crW;
        work->src.cr.height = crH;
        work->src.cr.lastW = crW % 8 != 0 ? crW % 8 : 8;
        work->src.cr.lastH = crH % 8 != 0 ? crH % 8 : 8;
        work->src.cr.blkW = 8;
        work->src.cr.blkH = 8;
        work->src.cr.blocks = 1;
        work->src.cr.sampling = 0;
        work->src.cr.ent.stride = 8;
        work->src.cr.ent.lastDc = 0;
        work->src.cr.ent.minRecip = work->src.cb.ent.minRecip;
        work->src.cr.ent.huff = &work->huff[2];
        work->src.cr.ent.recip = recipChrom;
        work->src.cr.ent.order = order;
        work->src.cr.ent.bw = bw;
        break;
    case 2:
        work->src.mcusX = (w + 15) >> 4;
        work->src.mcusY = (h + 15) >> 4;
        work->src.curX = 0;
        work->src.curY = 0;
        remX = w % 16;
        remY = h % 16;
        work->src.remX = remX;
        work->src.remY = remY;
        work->src.lum.width = w;
        work->src.lum.height = h;
        work->src.lum.lastW = remX != 0 ? remX : 16;
        work->src.lum.lastH = remY != 0 ? remY : 16;
        work->src.lum.blkW = 16;
        work->src.lum.blkH = 16;
        work->src.lum.blocks = 4;
        work->src.lum.sampling = 2;
        work->src.lum.ent.stride = 16;
        work->src.lum.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipLum;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.lum.ent.minRecip = minRecip;
        work->src.lum.ent.huff = &work->huff[0];
        work->src.lum.ent.recip = recipLum;
        work->src.lum.ent.order = order;
        work->src.lum.ent.bw = bw;

        work->src.cb.width = cbW;
        work->src.cb.height = cbH;
        work->src.cb.lastW = cbW % 8 != 0 ? cbW % 8 : 8;
        work->src.cb.lastH = cbH % 8 != 0 ? cbH % 8 : 8;
        work->src.cb.blkW = 8;
        work->src.cb.blkH = 8;
        work->src.cb.blocks = 1;
        work->src.cb.sampling = 2;
        work->src.cb.ent.stride = 8;
        work->src.cb.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipChrom;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.cb.ent.minRecip = minRecip;
        work->src.cb.ent.huff = &work->huff[2];
        work->src.cb.ent.recip = recipChrom;
        work->src.cb.ent.order = order;
        work->src.cb.ent.bw = bw;

        work->src.cr.width = crW;
        work->src.cr.height = crH;
        work->src.cr.lastW = crW % 8 != 0 ? crW % 8 : 8;
        work->src.cr.lastH = crH % 8 != 0 ? crH % 8 : 8;
        work->src.cr.blkW = 8;
        work->src.cr.blkH = 8;
        work->src.cr.blocks = 1;
        work->src.cr.sampling = 2;
        work->src.cr.ent.stride = 8;
        work->src.cr.ent.lastDc = 0;
        work->src.cr.ent.minRecip = work->src.cb.ent.minRecip;
        work->src.cr.ent.huff = &work->huff[2];
        work->src.cr.ent.recip = recipChrom;
        work->src.cr.ent.order = order;
        work->src.cr.ent.bw = bw;
        break;
    case 1:
        work->src.mcusX = (w + 15) >> 4;
        work->src.mcusY = (h + 7) >> 3;
        work->src.curX = 0;
        work->src.curY = 0;
        remX = w % 16;
        remY = h % 8;
        work->src.remX = remX;
        work->src.remY = remY;
        work->src.lum.width = w;
        work->src.lum.height = h;
        work->src.lum.lastW = remX != 0 ? remX : 16;
        work->src.lum.lastH = remY != 0 ? remY : 8;
        work->src.lum.blkW = 16;
        work->src.lum.blkH = 8;
        work->src.lum.blocks = 2;
        work->src.lum.sampling = 1;
        work->src.lum.ent.stride = 16;
        work->src.lum.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipLum;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.lum.ent.minRecip = minRecip;
        work->src.lum.ent.huff = &work->huff[0];
        work->src.lum.ent.recip = recipLum;
        work->src.lum.ent.order = order;
        work->src.lum.ent.bw = bw;

        work->src.cb.width = cbW;
        work->src.cb.height = cbH;
        work->src.cb.lastW = cbW % 8 != 0 ? cbW % 8 : 8;
        work->src.cb.lastH = cbH % 8 != 0 ? cbH % 8 : 8;
        work->src.cb.blkW = 8;
        work->src.cb.blkH = 8;
        work->src.cb.blocks = 1;
        work->src.cb.sampling = 1;
        work->src.cb.ent.stride = 8;
        work->src.cb.ent.lastDc = 0;
        minRecip = 0x10000;
        {
            const int *p = recipChrom;
            for (i = 0; i < 64; p++, i++) {
                if (minRecip > 0x10000 / *p) {
                    minRecip = 0x10000 / *p;
                }
            }
        }
        work->src.cb.ent.minRecip = minRecip;
        work->src.cb.ent.huff = &work->huff[2];
        work->src.cb.ent.recip = recipChrom;
        work->src.cb.ent.order = order;
        work->src.cb.ent.bw = bw;

        work->src.cr.width = crW;
        work->src.cr.height = crH;
        work->src.cr.lastW = crW % 8 != 0 ? crW % 8 : 8;
        work->src.cr.lastH = crH % 8 != 0 ? crH % 8 : 8;
        work->src.cr.blkW = 8;
        work->src.cr.blkH = 8;
        work->src.cr.blocks = 1;
        work->src.cr.sampling = 1;
        work->src.cr.ent.stride = 8;
        work->src.cr.ent.lastDc = 0;
        work->src.cr.ent.minRecip = work->src.cb.ent.minRecip;
        work->src.cr.ent.huff = &work->huff[2];
        work->src.cr.ent.recip = recipChrom;
        work->src.cr.ent.order = order;
        work->src.cr.ent.bw = bw;
        break;
    }
}

void emit_restart(TMCJPEGWork *work) {
    flush_bits_emit_marker(&work->bw, 0xFFD0 + (work->restartNum & 7));
    work->restartNum = work->restartNum + 1;
    work->src.lum.ent.lastDc = 0;
    work->src.cb.ent.lastDc = 0;
    work->src.cr.ent.lastDc = 0;
}

void setup_source(TMCJPEGWork *work, TMCJPEGCtx *ctx) {
    TMCJPEGSource *src = &work->src;

    ctx->curX = 0;
    ctx->curY = 0;
    ctx->outSize = 0;
    ctx->quality = work->quality;
    ctx->mcusX = src->mcusX;
    ctx->mcusY = src->mcusY;
    ctx->totalMcus = ctx->mcusX * ctx->mcusY;
    ctx->hSamp = src->lum.blkW;
    ctx->vSamp = src->lum.blkH;
    ctx->edgeWidth = src->remX;
    ctx->edgeHeight = src->remY;
    ctx->lastMcuX = src->remX == 0 ? (ctx->mcusX + 1) * ctx->hSamp
                                   : (ctx->mcusX - 1) * ctx->hSamp;
    ctx->lastMcuY = src->remY == 0 ? (ctx->mcusY + 1) * ctx->vSamp
                                   : (ctx->mcusY - 1) * ctx->vSamp;
    ctx->scale = 1;
    ctx->sampling = work->sampling;
    ctx->width = work->width;
    ctx->height = work->height;
    ctx->cbWidth = work->cbWidth;
    ctx->cbHeight = work->cbHeight;
    ctx->lumRowBytes = ctx->hSamp * ctx->mcusX;
    ctx->lumRows = ctx->vSamp * ctx->mcusY;
    ctx->cbRowBytes = src->cb.blkW * ctx->mcusX;
    ctx->cbRows = src->cb.blkH * ctx->mcusY;
}

int start_compress(TMCJPEGCtx *ctx, u8 *dst, const void *image,
                   TMCJPEGParams *params) {
    TMCJPEGWork *work = params->work;
    int ret;

    if (work == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(TMCJPEGCtx));
    memset(work, 0, 0x164C);
    ctx->work = work;
    work->src.ctx = ctx;
    ret = check_params(params, work);
    if (ret < 0) {
        return ret;
    }
    std_huff_tables(work);
    make_derived_huff_tables(work);
    set_quant_tables(work);
    initial_setup(work);
    dest_init(&work->bw, dst, params->bufSize);
    select_encode_mcu(work, work->sampling);
    select_encode_mcu_padded(work, work->sampling);
    if (write_file_header(work) < 0) {
        return -2;
    }
    setup_source(work, ctx);
    switch (ctx->format) {
    case GX_TF_RGB565:
        ret = select_load_mcu_rgb565(&work->src, ctx);
        if (ret < 0) {
            return ret;
        }
        break;
    case GX_TF_RGB5A3:
        break;
    case GX_TF_RGBA8:
        ret = select_load_mcu_rgba8(&work->src, ctx);
        if (ret < 0) {
            return ret;
        }
        break;
    }
    dest_get_size(&work->bw, &ctx->outSize);
    return ctx->totalMcus;
}

int encode_mcus(TMCJPEGCtx *ctx, int count, const void *image) {
    int restart;
    int done;
    int total;
    int mcusX;
    int mcusY;
    int curX;
    int curY;
    TMCJPEGWork *work;
    TMCJPEGSource *src;
    int ret;

    work = ctx->work;
    src = &work->src;
    ctx->plane[0] = image;
    ctx->pixels = image;
    ctx->plane[1] = (const u8 *)image + ctx->alignedWidth * ctx->alignedHeight;
    ctx->plane[2] = (const u8 *)image + ctx->planeW1 * ctx->planeH1;

    if (src->remY != 0 || src->remX != 0) {
        return encode_mcus_padded(work, ctx, count);
    }

    mcusX = src->mcusX;
    done = 0;
    mcusY = src->mcusY;
    curX = ctx->curX;
    total = mcusX * mcusY;
    curY = ctx->curY;
    restart = work->restartCounter;

    while (curY < mcusY && done < count) {
        while (curX < mcusX && done < count) {
            ret = work->encodeMcu(curX, curY, src);
            if (ret < 0) {
                return ret;
            }
            done++;
            curX++;
            if (--restart == 0) {
                flush_bits_emit_marker(&work->bw,
                                       0xFFD0 + (work->restartNum & 7));
                work->restartNum++;
                work->src.lum.ent.lastDc = 0;
                work->src.cb.ent.lastDc = 0;
                work->src.cr.ent.lastDc = 0;
                restart = work->restartInterval;
            }
        }
        if (curX >= mcusX) {
            curX = 0;
            curY++;
        }
    }

    if (curY == mcusY && curX == 0) {
        flush_bits_emit_eoi(&work->bw);
        if (work->bw.overflow == 1) {
            dest_get_size(&work->bw, &ctx->outSize);
            return -3;
        }
    }

    src->curX = curX;
    ctx->curX = curX;
    src->curY = curY;
    ctx->curY = curY;
    dest_get_size(&work->bw, &ctx->outSize);
    work->restartCounter = restart;
    return total - curX - curY * mcusX;
}

void select_encode_mcu(TMCJPEGWork *work, int sampling) {
    switch (sampling) {
    case 0:
        work->encodeMcu = encode_mcu_411;
        break;
    case 2:
        work->encodeMcu = encode_mcu_420;
        break;
    case 1:
        work->encodeMcu = encode_mcu_422;
        break;
    }
}

int encode_mcu_411(int x, int y, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->load(mcu, src->ctx, x * 32, y * 8);
    if (ret < 0) {
        return ret;
    }
    luma = mcu + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 16, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 24, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cb.ent;
    fdct_chroma((s8 *)mcu + 0x104, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cr.ent;
    fdct_chroma((s8 *)mcu + 0x144, block, ent->stride);
    ret = encode_one_block(block, ent);
    return ret < 0 ? ret : 0;
}

int encode_mcu_420(int x, int y, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->load(mcu, src->ctx, x * 16, y * 16);
    if (ret < 0) {
        return ret;
    }
    luma = mcu + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 0x80, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 0x88, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cb.ent;
    fdct_chroma((s8 *)mcu + 0x104, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cr.ent;
    fdct_chroma((s8 *)mcu + 0x144, block, ent->stride);
    ret = encode_one_block(block, ent);
    return ret < 0 ? ret : 0;
}

int encode_mcu_422(int x, int y, TMCJPEGSource *src) {
    int block[64];
    u8 mcu[0x184];
    u8 *luma;
    TMCJPEGEntropy *ent;
    int ret;

    ret = src->load(mcu, src->ctx, x * 16, y * 8);
    if (ret < 0) {
        return ret;
    }
    luma = mcu + 4;
    ent = &src->lum.ent;
    fdct_luma(luma, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(luma + 8, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cb.ent;
    fdct_chroma((s8 *)mcu + 0x84, block, ent->stride);
    ret = encode_one_block(block, ent);
    if (ret < 0) {
        return ret;
    }
    ent = &src->cr.ent;
    fdct_chroma((s8 *)mcu + 0xC4, block, ent->stride);
    ret = encode_one_block(block, ent);
    return ret < 0 ? ret : 0;
}

int write_scan_header(TMCJPEGWork *work) {
    TMCJPEGBitWriter *bw = &work->bw;

    emit_2bytes(bw, 0xFFDA);
    emit_2bytes(bw, 0xC);
    emit_byte(bw, 3);
    emit_byte(bw, 0);
    emit_byte(bw, 0);
    emit_byte(bw, 1);
    emit_byte(bw, 0x11);
    emit_byte(bw, 2);
    emit_byte(bw, 0x11);
    emit_byte(bw, 0);
    emit_byte(bw, 0x3F);
    emit_byte(bw, 0);
    return 0;
}

void fdct_luma(const u8 *in, int *out, int stride) {
    int i;
    int ws[64];
    int colEven13;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int odd12;
    int *p;
    int z1, z2, z3, z4, z11, z13;

    p = ws;
    for (i = 0; i < 8; i++) {
        tmp0 = in[0] + in[7];
        tmp7 = in[0] - in[7];
        tmp1 = in[1] + in[6];
        tmp6 = in[1] - in[6];
        tmp2 = in[2] + in[5];
        tmp5 = in[2] - in[5];
        tmp3 = in[3] + in[4];
        tmp4 = in[3] - in[4];
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        z1 = ((tmp13 + tmp12) * 0x2D41) >> 14;
        p[0] = tmp10 + tmp11 - 0x400;
        p[32] = tmp10 - tmp11;
        p[16] = tmp13 + z1;
        p[48] = tmp13 - z1;

        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        odd12 = tmp6 + tmp7;
        z2 = (tmp10 * 0x3B21 - odd12 * 0x187E) >> 14;
        z4 = (odd12 * 0x3B21 + tmp10 * 0x187E) >> 14;
        z3 = (tmp11 * 0x2D41) >> 14;
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;
        p[8] = z11 + z4;
        p[56] = z11 - z4;
        p[24] = z13 - z2;
        p[40] = z13 + z2;
        in += stride;
        p++;
    }

    p = ws;
    for (i = 0; i < 8; i++) {
        tmp0 = p[0] + p[7];
        tmp7 = p[0] - p[7];
        tmp1 = p[1] + p[6];
        tmp6 = p[1] - p[6];
        tmp2 = p[2] + p[5];
        tmp5 = p[2] - p[5];
        tmp3 = p[3] + p[4];
        tmp4 = p[3] - p[4];
        tmp10 = tmp0 + tmp3;
        colEven13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        z1 = ((colEven13 + tmp12) * 0x2D41) >> 14;
        out[0] = tmp10 + tmp11;
        out[32] = tmp10 - tmp11;
        out[16] = colEven13 + z1;
        out[48] = colEven13 - z1;

        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        odd12 = tmp6 + tmp7;
        z1 = (tmp10 - odd12) * 0x187E;
        /* Keep these subtractions to preserve the target operand order. */
        z2 = (tmp10 * 0x22A3 - (-z1)) >> 14;
        z4 = (odd12 * 0x539F - (-z1)) >> 14;
        z3 = (tmp11 * 0x2D41) >> 14;
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;
        out[40] = z13 + z2;
        out[24] = z13 - z2;
        out[8] = z11 + z4;
        out[56] = z11 - z4;
        p += 8;
        out++;
    }
}

void fdct_chroma(const s8 *in, int *out, int stride) {
    int i;
    int ws[64];
    int colEven13;
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13;
    int odd12;
    int *p;
    int z1, z2, z3, z4, z11, z13;

    p = ws;
    for (i = 0; i < 8; i++) {
        tmp0 = in[0] + in[7];
        tmp7 = in[0] - in[7];
        tmp1 = in[1] + in[6];
        tmp6 = in[1] - in[6];
        tmp2 = in[2] + in[5];
        tmp5 = in[2] - in[5];
        tmp3 = in[3] + in[4];
        tmp4 = in[3] - in[4];
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        z1 = ((tmp13 + tmp12) * 0x2D41) >> 14;
        p[0] = tmp10 + tmp11;
        p[32] = tmp10 - tmp11;
        p[16] = tmp13 + z1;
        p[48] = tmp13 - z1;

        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        odd12 = tmp6 + tmp7;
        z2 = (tmp10 * 0x3B21 - odd12 * 0x187E) >> 14;
        z4 = (odd12 * 0x3B21 + tmp10 * 0x187E) >> 14;
        z3 = (tmp11 * 0x2D41) >> 14;
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;
        p[8] = z11 + z4;
        p[56] = z11 - z4;
        p[24] = z13 - z2;
        p[40] = z13 + z2;
        in += stride;
        p++;
    }

    p = ws;
    for (i = 0; i < 8; i++) {
        tmp0 = p[0] + p[7];
        tmp7 = p[0] - p[7];
        tmp1 = p[1] + p[6];
        tmp6 = p[1] - p[6];
        tmp2 = p[2] + p[5];
        tmp5 = p[2] - p[5];
        tmp3 = p[3] + p[4];
        tmp4 = p[3] - p[4];
        tmp10 = tmp0 + tmp3;
        colEven13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        z1 = ((colEven13 + tmp12) * 0x2D41) >> 14;
        out[0] = tmp10 + tmp11;
        out[32] = tmp10 - tmp11;
        out[16] = colEven13 + z1;
        out[48] = colEven13 - z1;

        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        odd12 = tmp6 + tmp7;
        z1 = (tmp10 - odd12) * 0x187E;
        /* Keep these subtractions to preserve the target operand order. */
        z2 = (tmp10 * 0x22A3 - (-z1)) >> 14;
        z4 = (odd12 * 0x539F - (-z1)) >> 14;
        z3 = (tmp11 * 0x2D41) >> 14;
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;
        out[40] = z13 + z2;
        out[24] = z13 - z2;
        out[8] = z11 + z4;
        out[56] = z11 - z4;
        p += 8;
        out++;
    }
}

// Standard Huffman tables (Annex K), in target order.
static const u8 s_dcLumBits[17] = {
    0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const u8 s_dcLumVals[12] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
};

static const u8 s_dcChromBits[17] = {
    0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const u8 s_dcChromVals[12] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
};

static const u8 s_acLumBits[17] = {
    0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
    0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
};

static const u8 s_acLumVals[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06,
    0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
    0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72,
    0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
    0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
};

static const u8 s_acChromBits[17] = {
    0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
    0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
};

static const u8 s_acChromVals[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41,
    0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1,
    0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
    0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,
    0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
    0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
    0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
};

void std_huff_tables(TMCJPEGWork *work) {
    work->stdTable[0] = s_dcLumBits;
    work->stdTable[1] = s_dcLumVals;
    work->stdTable[2] = s_dcChromBits;
    work->stdTable[3] = s_dcChromVals;
    work->stdTable[4] = s_acLumBits;
    work->stdTable[5] = s_acLumVals;
    work->stdTable[6] = s_acChromBits;
    work->stdTable[7] = s_acChromVals;
    work->huff[0].count = 0;
    work->huff[0].code = work->dcLumCode;
    work->huff[0].size = work->dcLumSize;
    work->huff[1].count = 0;
    work->huff[1].code = work->acLumCode;
    work->huff[1].size = work->acLumSize;
    work->huff[2].count = 0;
    work->huff[2].code = work->dcChromCode;
    work->huff[2].size = work->dcChromSize;
    work->huff[3].count = 0;
    work->huff[3].code = work->acChromCode;
    work->huff[3].size = work->acChromSize;
}

void make_derived_huff_tables(TMCJPEGWork *work) {
    u32 huffsize[256];
    u32 huffcode[256];
    int i;
    u32 *hs;
    int p;
    int l;
    int j;
    const u8 *bits;
    u32 *hc;
    u32 *sizes;
    u16 code;
    u32 x;
    u8 si;
    const u8 *vals;
    TMCJPEGHuffTable *tbl;
    u8 lastp;
    int symbol;

    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            bits = s_dcLumBits;
            vals = s_dcLumVals;
            tbl = &work->huff[0];
            break;
        case 1:
            bits = s_dcChromBits;
            vals = s_dcChromVals;
            tbl = &work->huff[2];
            break;
        case 2:
            bits = s_acLumBits;
            vals = s_acLumVals;
            tbl = &work->huff[1];
            break;
        case 3:
            bits = s_acChromBits;
            vals = s_acChromVals;
            tbl = &work->huff[3];
            break;
        }

        p = 0;
        l = 1;
        j = 1;
        hs = huffsize;
        while (1) {
            if (j > bits[l]) {
                l++;
                j = 1;
                if (l > 16) {
                    huffsize[p] = 0;
                    lastp = p;
                    break;
                }
            } else {
                *hs++ = l + 1;
                p++;
                j++;
            }
        }

        code = 0;
        si = huffsize[0];
        hc = huffcode;
        sizes = huffsize;
        for (;;) {
            do {
                *hc = code;
                code++;
                hc++;
                x = *++sizes;
            } while (si == x);
            if (x == 0) {
                break;
            }
            do {
                code <<= 1;
                si++;
            } while (si != x);
        }

        p = 0;
        do {
            symbol = vals[p];
            tbl->code[symbol] = huffcode[p];
            tbl->size[symbol] = huffsize[p];
            p++;
        } while (p < lastp);
        tbl->count = lastp;
    }
}
