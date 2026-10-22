// Size estimation and GX texture sample loaders for TMC JPEG.
#include <MWM/tmc_jpeg_internal.h>
#include <revolution/os.h>
#include <string.h>

int start_compress_count(TMCJPEGCtx *ctx, const void *image,
                         TMCJPEGParams *params) {
    TMCJPEGWork *work;
    int ret;

    work = params->work;
    if (work == NULL) {
        return -1;
    }
    memset(ctx, 0, sizeof(*ctx));
    memset(work, 0, sizeof(*work));
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
    dest_init(&work->bw, NULL, params->bufSize);
    select_count_mcu(work, work->sampling);
    select_count_mcu_padded(work, work->sampling);
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
    if (work->restartInterval != 0) {
        ctx->outSize = 0x263;
    } else {
        ctx->outSize = 0x25D;
    }
    return ctx->totalMcus;
}

int estimate_size(TMCJPEGCtx *ctx, int count, const void *image, int batch) {
    int restartLeft;
    int ret;
    int done;
    int batchCount;
    int total;
    int mcusX;
    int mcusY;
    int x;
    TMCJPEGWork *work;
    int y;
    TMCJPEGSource *sampler;

    work = ctx->work;
    sampler = &work->src;
    ctx->plane[0] = image;
    ctx->pixels = image;
    ctx->plane[1] = (const u8 *)image + ctx->alignedWidth * ctx->alignedHeight;
    ctx->plane[2] = (const u8 *)image + ctx->planeW1 * ctx->planeH1;
    if (sampler->remY != 0 || sampler->remX != 0) {
        return count_mcus(work, ctx, count, batch);
    }
    mcusX = sampler->mcusX;
    mcusY = sampler->mcusY;
    total = mcusX * mcusY;
    x = ctx->curX;
    y = ctx->curY;
    batchCount = 0;
    done = 0;
    restartLeft = work->restartCounter;
    while (y < mcusY && done < count) {
        while (x < mcusX && done < count) {
            batchCount++;
            if (batchCount >= batch) {
                ret = work->encodeMcu(x, y, sampler);
                if (ret < 0) {
                    return ret;
                }
                batchCount = 0;
                count_get_size(&work->bw, &ctx->outSize);
            }
            restartLeft--;
            done++;
            x++;
            if (restartLeft == 0) {
                count_restart(work);
                restartLeft = work->restartInterval;
            }
        }
        if (x >= mcusX) {
            x = 0;
            y++;
        }
    }
    if (y == mcusY && x == 0) {
        count_eoi(work);
        if (work->bw.overflow == 1) {
            count_get_size(&work->bw, &ctx->outSize);
            return -3;
        }
    }
    sampler->curX = x;
    ctx->curX = x;
    sampler->curY = y;
    ctx->curY = y;
    count_get_size(&work->bw, &ctx->outSize);
    work->restartCounter = restartLeft;
    return total - x - y * mcusX;
}

int search_quality(TMCJPEGCtx *ctx, const void *image, TMCJPEGParams *params) {
    u32 s200;
    u32 s1;
    u32 s50;
    u32 s100;
    u32 target;
    u32 s500;
    int n;

    target = (u32)(0.9 * params->maxSize);
    params->quality = 100;
    n = start_compress_count(ctx, image, params);
    if (n >= 0) {
        if (estimate_size(ctx, n, image, 5) < 0) {
            return -2;
        }
        s100 = ctx->outSize * 5;
    } else {
        return -2;
    }
    if (s100 > target) {
        params->quality = 50;
        n = start_compress_count(ctx, image, params);
        if (n >= 0) {
            if (estimate_size(ctx, n, image, 5) < 0) {
                return -2;
            }
            s50 = ctx->outSize * 5;
        } else {
            return -2;
        }
        if (s50 > target) {
            params->quality = 1;
            n = start_compress_count(ctx, image, params);
            if (n >= 0) {
                if (estimate_size(ctx, n, image, 5) < 0) {
                    return -2;
                }
                s1 = ctx->outSize * 5;
            } else {
                return -2;
            }
            if (s1 >= target) {
                params->quality = 1;
            } else {
                params->quality = (target - s1) / ((s50 - s1 + 49) / 50);
                if (params->quality > 49) {
                    params->quality = 49;
                }
            }
        } else if (s50 == target) {
            params->quality = 50;
        } else {
            params->quality = 50 + (target - s50) / ((s100 - s50 + 49) / 50);
            if (params->quality > 99) {
                params->quality = 99;
            }
        }
    } else {
        params->quality = 200;
        n = start_compress_count(ctx, image, params);
        if (n >= 0) {
            if (estimate_size(ctx, n, image, 5) < 0) {
                return -2;
            }
            s200 = ctx->outSize * 5;
        } else {
            return -2;
        }
        if (s100 == target) {
            params->quality = 100;
        } else if (s200 > target) {
            params->quality =
                100 + (target - s100) / ((s200 - s100 + 99) / 100);
            if (params->quality > 199) {
                params->quality = 199;
            }
        } else {
            params->quality = 500;
            n = start_compress_count(ctx, image, params);
            if (n >= 0) {
                if (estimate_size(ctx, n, image, 5) < 0) {
                    return -2;
                }
                s500 = ctx->outSize * 5;
            } else {
                return -2;
            }
            if (s200 == target) {
                params->quality = 200;
            } else if (s500 > target) {
                params->quality =
                    200 + (target - s200) / ((s500 - s200 + 299) / 300);
                if (params->quality > 499) {
                    params->quality = 499;
                }
            } else {
                params->quality = 500;
            }
        }
    }
    if (params->quality < 1) {
        params->quality = 1;
    } else if (params->quality > 500) {
        params->quality = 500;
    }
    return params->quality;
}

static inline int abs_value(int v) {
    if (v < 0) {
        v = -v;
    }
    return v;
}

int count_one_block(int *block, TMCJPEGEntropy *comp) {
    TMCJPEGHuffTable *huff;
    int i;
    int run;
    int bits;
    int *quant;
    const int *order;
    int coef;
    int qt;
    int q;
    int diff;
    int nbits;

    order = comp->order + 1;
    quant = comp->recip;
    huff = comp->huff;
    coef = block[0];
    if (coef < 0) {
        coef = -coef;
        q = -((quant[0] * coef + 0x8000) >> 16);
    } else {
        q = (quant[0] * coef + 0x8000) >> 16;
    }
    diff = q - comp->lastDc;
    comp->lastDc = q;
    if (diff < 0) {
        diff = -diff;
    }
    nbits = 0;
    while (diff > 0) {
        diff >>= 1;
        nbits++;
    }
    if (nbits >= huff[0].count) {
        return -2;
    }

    bits = huff[0].size[nbits];
    bits += nbits;
    run = 0;
    for (i = 1; i < 64; i++) {
        int idx = *order++;
        coef = block[idx];
        qt = quant[idx];
        if (coef <= 0) {
            q = -((-coef * qt + 0x8000) >> 16);
        } else {
            q = (coef * qt + 0x8000) >> 16;
        }
        if (q == 0) {
            run++;
            continue;
        }
        while (run > 15) {
            bits += huff[1].size[0xF0];
            run -= 16;
        }
        q = abs_value(q);
        nbits = 0;
        do {
            nbits++;
        } while ((q >> nbits) != 0);
        if (nbits >= huff[1].count) {
            return -2;
        }
        bits += huff[1].size[(run << 4) + nbits] + nbits;
        run = 0;
    }
    if (run > 0) {
        bits += huff[1].size[0];
    }
    comp->bw->count += bits;
    return 0;
}

void select_count_mcu(TMCJPEGWork *work, int mode) {
    switch (work->sampling) {
    case 0:
        work->encodeMcu = count_mcu_411;
        break;
    case 2:
        work->encodeMcu = count_mcu_420;
        break;
    case 1:
        work->encodeMcu = count_mcu_422;
        break;
    }
}

int count_mcu_411(int mcuX, int mcuY, TMCJPEGSource *sampler) {
    int block[64];
    u8 buf[0x184];
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->load(buf, sampler->ctx, mcuX << 5, mcuY << 3);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 16, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 24, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cb.ent;
    fdct_chroma((s8 *)(buf + 0x104), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cr.ent;
    fdct_chroma((s8 *)(buf + 0x144), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int count_mcu_420(int mcuX, int mcuY, TMCJPEGSource *sampler) {
    int block[64];
    u8 buf[0x184];
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->load(buf, sampler->ctx, mcuX << 4, mcuY << 4);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 0x80, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 0x88, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cb.ent;
    fdct_chroma((s8 *)(buf + 0x104), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cr.ent;
    fdct_chroma((s8 *)(buf + 0x144), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int count_mcu_422(int mcuX, int mcuY, TMCJPEGSource *sampler) {
    int block[64];
    u8 buf[0x184];
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->load(buf, sampler->ctx, mcuX << 4, mcuY << 3);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cb.ent;
    fdct_chroma((s8 *)(buf + 0x84), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    comp = &sampler->cr.ent;
    fdct_chroma((s8 *)(buf + 0xC4), block, comp->stride);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int count_eoi(TMCJPEGWork *work) {
    work->bw.count = ((work->bw.count + 7) / 8 + 2) * 8;
    return 0;
}

void count_restart(TMCJPEGWork *work) {
    work->bw.count = ((work->bw.count + 7) / 8 + 2) * 8;
    work->restartNum++;
    work->src.lum.ent.lastDc = 0;
    work->src.cb.ent.lastDc = 0;
    work->src.cr.ent.lastDc = 0;
}

void count_get_size(TMCJPEGBitWriter *writer, u32 *size) {
    u32 bytes = writer->count >> 3;

    *size += bytes;
    writer->count -= bytes << 3;
}

int count_mcus(TMCJPEGWork *work, TMCJPEGCtx *ctx, int limit, int batch) {
    TMCJPEGSource *sampler = &work->src;
    int x;
    int y;
    int lastX;
    int lastY;
    int total;
    int n;
    int restartLeft;
    int batchCount;
    int ret;

    n = 0;
    batchCount = 0;
    restartLeft = work->restartCounter;
    lastX = sampler->mcusX - 1;
    y = sampler->curY;
    total = sampler->mcusX * sampler->mcusY;
    x = sampler->curX;
    lastY = sampler->mcusY - 1;

    while (y < lastY && n < limit) {
        while (x < lastX && n < limit) {
            batchCount++;
            if (batchCount >= batch) {
                ret = work->encodeMcu(x, y, sampler);
                if (ret < 0) {
                    return ret;
                }
                batchCount = 0;
                count_get_size(&work->bw, &ctx->outSize);
            }
            restartLeft--;
            n--;
            x++;
            if (restartLeft == 0) {
                count_restart(work);
                restartLeft = work->restartInterval;
            }
        }
        if (n < limit) {
            batchCount++;
            if (batchCount >= batch) {
                ret = work->encodeMcuPadded(x, y, 0, sampler);
                if (ret < 0) {
                    return ret;
                }
                batchCount = 0;
                count_get_size(&work->bw, &ctx->outSize);
            }
            restartLeft--;
            x = 0;
            n--;
            y++;
            if (restartLeft == 0) {
                count_restart(work);
                restartLeft = work->restartInterval;
            }
        }
    }
    while (x < lastX && n < limit) {
        batchCount++;
        if (batchCount >= batch) {
            ret = work->encodeMcuPadded(x, y, 1, sampler);
            if (ret < 0) {
                return ret;
            }
            batchCount = 0;
            count_get_size(&work->bw, &ctx->outSize);
        }
        restartLeft--;
        n--;
        x++;
        if (restartLeft == 0) {
            count_restart(work);
            restartLeft = work->restartInterval;
        }
    }
    if (n < limit) {
        batchCount++;
        if (batchCount >= batch) {
            ret = work->encodeMcuPadded(x, y, 2, sampler);
            if (ret < 0) {
                return ret;
            }
            batchCount = 0;
            count_get_size(&work->bw, &ctx->outSize);
        }
        restartLeft--;
        x = 0;
        y++;
        if (restartLeft == 0) {
            count_restart(work);
            restartLeft = work->restartInterval;
        }
        count_eoi(work);
        if (work->bw.overflow) {
            count_get_size(&work->bw, &ctx->outSize);
            return -3;
        }
    }
    sampler->curX = x;
    ctx->curX = x;
    sampler->curY = y;
    ctx->curY = y;
    work->restartCounter = restartLeft;
    count_get_size(&work->bw, &ctx->outSize);
    return total - x - y * sampler->mcusX;
}

void select_count_mcu_padded(TMCJPEGWork *work, int mode) {
    switch (mode) {
    case 0:
        work->encodeMcuPadded = count_mcu_padded_411;
        break;
    case 2:
        work->encodeMcuPadded = count_mcu_padded_420;
        break;
    case 1:
        work->encodeMcuPadded = count_mcu_padded_422;
        break;
    }
}

int count_mcu_padded_411(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler) {
    int block[64];
    u8 samples[0x184];
    u8 *buf = samples;
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->loadPadded(buf, sampler->ctx, mcuX << 5, mcuY << 3);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, 0x20);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, 0x20);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 16, block, 0x20);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 24, block, 0x20);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0x104), block, 8);
    ret = count_one_block(block, &sampler->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0x144), block, 8);
    ret = count_one_block(block, &sampler->cr.ent);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int count_mcu_padded_420(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler) {
    int block[64];
    u8 samples[0x184];
    u8 *buf = samples;
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->loadPadded(buf, sampler->ctx, mcuX << 4, mcuY << 4);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 0x80, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 0x88, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0x104), block, 8);
    ret = count_one_block(block, &sampler->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0x144), block, 8);
    ret = count_one_block(block, &sampler->cr.ent);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

int count_mcu_padded_422(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler) {
    int block[64];
    u8 samples[0x184];
    u8 *buf = samples;
    u8 *y;
    TMCJPEGEntropy *comp;
    int ret;

    ret = sampler->loadPadded(buf, sampler->ctx, mcuX << 4, mcuY << 3);
    if (ret < 0) {
        return ret;
    }
    y = buf + 4;
    comp = &sampler->lum.ent;
    fdct_luma(y, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_luma(y + 8, block, 0x10);
    ret = count_one_block(block, comp);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0x84), block, 8);
    ret = count_one_block(block, &sampler->cb.ent);
    if (ret < 0) {
        return ret;
    }
    fdct_chroma((s8 *)(buf + 0xC4), block, 8);
    ret = count_one_block(block, &sampler->cr.ent);
    if (ret < 0) {
        return ret;
    }
    return 0;
}

typedef struct TMCJPEGTileRounding {
    int extra;
} TMCJPEGTileRounding;

// @HACK - A by-value result keeps the remainder flag separate during MWCC propagation.
static inline TMCJPEGTileRounding get_tile_rounding(u16 size) {
    TMCJPEGTileRounding rounding;

    rounding.extra = size % 4 != 0;
    return rounding;
}

int select_load_mcu_rgba8(TMCJPEGSource *sampler, TMCJPEGCtx *ctx) {
    switch (ctx->sampling) {
    case 0:
        sampler->load = load_mcu_rgba8_411;
        sampler->loadPadded = load_mcu_rgba8_padded_411;
        break;
    case 1:
        sampler->load = load_mcu_rgba8_422;
        sampler->loadPadded = load_mcu_rgba8_padded_422;
        break;
    case 2:
        sampler->load = load_mcu_rgba8_420;
        sampler->loadPadded = load_mcu_rgba8_padded_420;
        break;
    default:
        return -0x70;
    }
    ctx->alignedWidth = (ctx->width / 4 + (ctx->width % 4 != 0)) * 4;
    ctx->alignedHeight =
        (ctx->height / 4 + get_tile_rounding(ctx->height).extra) * 4;
    return 0;
}

static inline int rgb_luma(int r, int g, int b) {
    return (r * 77 + g * 150 + b * 29 + 128) >> 8;
}

// @HACK - These casted reads and separate int/long conversions delay MWCC's
// IRO propagation. They fold away in the back end; preserve their statement order.
static inline int rgba8_luma(u16 ar, u16 gb) {
    return ((*(const u16 *)&ar & 0xFF) * 77 +
            (*(const u16 *)&gb >> 8) * 150 +
            (*(const u16 *)&gb & 0xFF) * 29 + 128) >> 8;
}

static inline int rgba8_luma_channels(u16 ar, u16 gb) {
    int b;
    int g;
    int r;

    r = (int)(*(const u16 *)&ar & 0xFF);
    r = (int)(long)r;
    g = (int)(*(const u16 *)&gb >> 8);
    g = (int)(long)g;
    b = (int)(gb & 0xFF);
    b = (int)(long)b;
    return (r * 77 + g * 150 + b * 29 + 128) >> 8;
}

int load_mcu_rgba8_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    int r;
    int b;
    u8 *cb;
    u8 *cr;
    int xx;
    int endX;
    int endY;
    int stride;
    int scale;
    u32 width;
    int w;
    int h;
    int yy;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = (width >> 2) << 1;
    pixels = (const u16 *)ctx->pixels;
    w = 32 / scale;
    h = 8 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 4) {
            {
                r = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
                b = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)] & 0xFF;
                y[0] = rgba8_luma(
                    (pixels + ((yy & 3) << 2))[
                        (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                    (pixels + ((yy & 3) << 2))[
                        (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)]);
                b -= y[0];
                *cb++ = (b * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            {
                int arValue;

                arValue = (int)(pixels + ((yy & 3) << 2))[
                    ((xx + 1) & 3) + (((int)(((xx + 1) >> 2) << 1) + ((yy >> 2) * stride)) << 4)];
                arValue = (int)(long)arValue;
                y[1] = rgba8_luma_channels(
                    arValue,
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 1) & 3) + ((((((xx + 1) >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)]);
            }
            {
                int gbValue;

                gbValue = (int)(pixels + ((yy & 3) << 2))[
                    ((xx + 2) & 3) + ((((((xx + 2) >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
                gbValue = (int)(long)gbValue;
                y[2] = rgba8_luma(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 2) & 3) + (((int)(((xx + 2) >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                    gbValue);
            }
            {
                int gbValue;

                gbValue = (int)(pixels + ((yy & 3) << 2))[
                    ((xx + 3) & 3) + ((((((xx + 3) >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
                gbValue = (int)(long)gbValue;
                y[3] = rgba8_luma(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 3) & 3) + (((int)(((xx + 3) >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                    gbValue);
            }
            y += 4;
        }
        y += 32 - w;
        cb += (32 - w) >> 2;
        cr += (32 - w) >> 2;
    }
    return 0;
}

int load_mcu_rgba8_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    int r;
    int scale;
    u8 *cb;
    u8 *cr;
    int w;
    int h;
    int xx;
    int endX;
    int endY;
    int yy;
    u32 width;
    int stride;

    y = dst + 4;
    cb = dst + 0x84;
    cr = dst + 0xC4;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = (width >> 2) << 1;
    pixels = (const u16 *)ctx->pixels;
    w = 16 / scale;
    h = 8 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 2) {
            int gb;

            {
                int g;
                int b;

                gb = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
                r = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
                g = gb >> 8;
                b = gb & 0xFF;
                y[0] = rgb_luma(r, g, b);
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            {
                int r;
                int g;
                int b;

                gb = (pixels + ((yy & 3) << 2))[
                    ((xx + 1) & 3) + ((((((xx + 1) >> 2) << 1) + 1) + ((yy >> 2) * stride)) << 4)];
                r = (pixels + ((yy & 3) << 2))[
                    ((xx + 1) & 3) + (((((xx + 1) >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
                g = gb >> 8;
                b = gb & 0xFF;
                y[1] = rgb_luma(r, g, b);
            }
            y += 2;
        }
        y += 16 - w;
        cb += (16 - w) >> 1;
        cr += (16 - w) >> 1;
    }
    return 0;
}

int load_mcu_rgba8_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    int r;
    int scale;
    u8 *cb;
    u8 *cr;
    int w;
    int h;
    int xx;
    int endX;
    int endY;
    int yy;
    u32 width;
    int stride;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = (width >> 2) << 1;
    pixels = (const u16 *)ctx->pixels;
    w = 16 / scale;
    h = 16 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 2) {
            int gb;

            {
                int b;

                gb = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
                r = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
                b = gb & 0xFF;
                y[0] = rgba8_luma(
                    (pixels + ((yy & 3) << 2))[
                        (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                    gb);
                if ((yy & 1) == 0) {
                    *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                    *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
                }
            }
            {
                gb = (pixels + ((yy & 3) << 2))[
                    ((xx + 1) & 3) + ((((((xx + 1) >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
                y[1] = rgba8_luma(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 1) & 3) + (((((xx + 1) >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                    gb);
            }
            y += 2;
        }
        y += 16 - w;
        if ((yy & 1) == 0) {
            cb += (16 - w) >> 1;
            cr += (16 - w) >> 1;
        }
    }
    return 0;
}

int load_mcu_rgba8_padded_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    int r;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = (width >> 2) << 1;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 32 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 8 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            int gb;
            int b;

            gb = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
            r = (pixels + ((yy & 3) << 2))[
                (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
            b = gb & 0xFF;
            y[0] = rgba8_luma(
                (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                gb);
            if ((xx & 3) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 32; i++) {
            *y = y[-1];
            y++;
            if ((i & 3) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 8; h++) {
        memcpy(y, y - 32, 32);
        y += 32;
        memcpy(cb, cb - 8, 8);
        memcpy(cr, cr - 8, 8);
        cb += 8;
        cr += 8;
    }
    return 0;
}

int load_mcu_rgba8_padded_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    int r;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x84;
    cr = dst + 0xC4;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = (width >> 2) << 1;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 16 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 8 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            int gb;
            int b;

            gb = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
            r = (pixels + ((yy & 3) << 2))[
                (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
            b = gb & 0xFF;
            y[0] = rgba8_luma(
                (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                gb);
            if ((xx & 1) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 16; i++) {
            *y = y[-1];
            y++;
            if ((i & 1) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 8; h++) {
        memcpy(y, y - 16, 16);
        y += 16;
        memcpy(cb, cb - 8, 8);
        memcpy(cr, cr - 8, 8);
        cb += 8;
        cr += 8;
    }
    return 0;
}

int load_mcu_rgba8_padded_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    int r;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = (width >> 2) << 1;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 16 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 16 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            int gb;
            int b;

            gb = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((((xx >> 2) << 1) + ((yy >> 2) * stride)) + 1) << 4)];
            r = (pixels + ((yy & 3) << 2))[
                (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)] & 0xFF;
            b = gb & 0xFF;
            y[0] = rgba8_luma(
                (pixels + ((yy & 3) << 2))[
                    (xx & 3) + ((((xx >> 2) << 1) + ((yy >> 2) * stride)) << 4)],
                gb);
            if ((xx & 1) == 0 && (yy & 1) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 16; i++) {
            *y = y[-1];
            y++;
            if ((i & 1) == 0 && (yy & 1) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 16; h++) {
        memcpy(y, y - 16, 16);
        y += 16;
        if ((h & 1) == 0) {
            memcpy(cb, cb - 8, 8);
            memcpy(cr, cr - 8, 8);
            cb += 8;
            cr += 8;
        }
    }
    return 0;
}

int select_load_mcu_rgb565(TMCJPEGSource *sampler, TMCJPEGCtx *ctx) {
    switch (ctx->sampling) {
    case 0:
        sampler->load = load_mcu_rgb565_411;
        sampler->loadPadded = load_mcu_rgb565_padded_411;
        break;
    case 1:
        sampler->load = load_mcu_rgb565_422;
        sampler->loadPadded = load_mcu_rgb565_padded_422;
        break;
    case 2:
        sampler->load = load_mcu_rgb565_420;
        sampler->loadPadded = load_mcu_rgb565_padded_420;
        break;
    default:
        return -0x70;
    }
    ctx->alignedWidth = (ctx->width / 4 + (ctx->width % 4 != 0)) * 4;
    ctx->alignedHeight =
        (ctx->height / 4 + get_tile_rounding(ctx->height).extra) * 4;
    return 0;
}

static inline int rgb565_luma(int p) {
    u8 r;
    u8 g;
    u8 b;

    r = p >> 8;
    g = p >> 3;
    b = p << 3;
    r &= 0xF8;
    g &= 0xFC;
    b &= 0xF8;
    return rgb_luma(r, g, b);
}

// Reuse the blue channel already extracted for the chroma calculation.
static inline int rgb565_luma_with_blue(int p, u8 b) {
    u8 r;
    u8 g;

    r = p >> 8;
    g = p >> 3;
    r &= 0xF8;
    g &= 0xFC;
    return rgb_luma(r, g, b);
}

// Preserve these explicit mask assignments. MWCC handles compound assignments
// differently here, changing the channel register lifetimes.
static inline int rgb565_luma_channels(int p) {
    u8 r;
    u8 g;
    u8 b;

    r = p >> 8;
    g = p >> 3;
    b = p << 3;
    r = r & 0xF8;
    g = g & 0xFC;
    b = b & 0xF8;
    return rgb_luma(r, g, b);
}

int load_mcu_rgb565_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    u8 b;
    u8 *cb;
    u8 *cr;
    int xx;
    int endX;
    int endY;
    int stride;
    int scale;
    u32 width;
    int w;
    int h;
    int yy;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = width >> 2;
    pixels = (const u16 *)ctx->pixels;
    w = 32 / scale;
    h = 8 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 4) {
            {
                int p;
                u8 r;

                p = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
                r = p >> 8;
                b = p << 3;
                r &= 0xF8;
                b &= 0xF8;
                y[0] = rgb565_luma_with_blue(p, b);
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            {
                y[1] = rgb565_luma_channels(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 1) & 3) + ((((xx + 1) >> 2) + ((yy >> 2) * stride)) << 4)]);
            }
            {
                int p;

                p = (pixels + ((yy & 3) << 2))[
                    ((xx + 2) & 3) + ((((xx + 2) >> 2) + ((yy >> 2) * stride)) << 4)];
                y[2] = rgb565_luma(p);
            }
            {
                int p;

                p = (pixels + ((yy & 3) << 2))[
                    ((xx + 3) & 3) + ((((xx + 3) >> 2) + ((yy >> 2) * stride)) << 4)];
                y[3] = rgb565_luma(p);
            }
            y += 4;
        }
        y += 32 - w;
        cb += (32 - w) >> 2;
        cr += (32 - w) >> 2;
    }
    return 0;
}

int load_mcu_rgb565_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    u8 b;
    u8 *cb;
    u8 *cr;
    int xx;
    int endX;
    int endY;
    int stride;
    int scale;
    u32 width;
    int w;
    int h;
    int yy;

    y = dst + 4;
    cb = dst + 0x84;
    cr = dst + 0xC4;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = width >> 2;
    pixels = (const u16 *)ctx->pixels;
    w = 16 / scale;
    h = 8 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 2) {
            {
                u16 p;
                u8 r;

                p = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
                r = p >> 8;
                b = p << 3;
                r &= 0xF8;
                b &= 0xF8;
                y[0] = rgb565_luma_with_blue(p, b);
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            {
                y[1] = rgb565_luma(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 1) & 3) + ((((xx + 1) >> 2) + ((yy >> 2) * stride)) << 4)]);
            }
            y += 2;
        }
        y += 16 - w;
        cb += (16 - w) >> 1;
        cr += (16 - w) >> 1;
    }
    return 0;
}

int load_mcu_rgb565_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    const u16 *pixels;
    u8 *y;
    u8 r;
    u8 b;
    u8 *cb;
    u8 *cr;
    int xx;
    int endX;
    int endY;
    int stride;
    int scale;
    u32 width;
    int w;
    int h;
    int yy;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    scale = ctx->scale;
    width = ctx->alignedWidth;
    stride = width >> 2;
    pixels = (const u16 *)ctx->pixels;
    w = 16 / scale;
    h = 16 / scale;
    endX = px + w;
    endY = py + h;

    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx += 2) {
            {
                u16 p;
                u8 g;

                p = (pixels + ((yy & 3) << 2))[
                    (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
                r = p >> 8;
                g = p >> 3;
                b = p << 3;
                r &= 0xF8;
                g &= 0xFC;
                b &= 0xF8;
                y[0] = rgb_luma(r, g, b);
                if ((yy & 1) == 0) {
                    *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                    *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
                }
            }
            {
                y[1] = rgb565_luma(
                    (pixels + ((yy & 3) << 2))[
                        ((xx + 1) & 3) + ((((xx + 1) >> 2) + ((yy >> 2) * stride)) << 4)]);
            }
            y += 2;
        }
        y += 16 - w;
        if ((yy & 1) == 0) {
            cb += (16 - w) >> 1;
            cr += (16 - w) >> 1;
        }
    }
    return 0;
}

int load_mcu_rgb565_padded_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    u8 r;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    u8 b;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = width >> 2;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 32 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 8 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            u16 p;
            u8 g;

            p = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
            r = p >> 8;
            g = p >> 3;
            b = p << 3;
            r &= 0xF8;
            g &= 0xFC;
            b &= 0xF8;
            y[0] = rgb_luma(r, g, b);
            if ((xx & 3) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 32; i++) {
            *y = y[-1];
            y++;
            if ((i & 3) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 8; h++) {
        memcpy(y, y - 32, 32);
        y += 32;
        memcpy(cb, cb - 8, 8);
        memcpy(cr, cr - 8, 8);
        cb += 8;
        cr += 8;
    }
    return 0;
}

int load_mcu_rgb565_padded_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    u8 r;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    u8 b;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x84;
    cr = dst + 0xC4;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = width >> 2;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 16 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 8 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            u16 p;
            u8 g;

            p = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
            r = p >> 8;
            g = p >> 3;
            b = p << 3;
            r &= 0xF8;
            g &= 0xFC;
            b &= 0xF8;
            y[0] = rgb_luma(r, g, b);
            if ((xx & 1) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 16; i++) {
            *y = y[-1];
            y++;
            if ((i & 1) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 8; h++) {
        memcpy(y, y - 16, 16);
        y += 16;
        memcpy(cb, cb - 8, 8);
        memcpy(cr, cr - 8, 8);
        cb += 8;
        cr += 8;
    }
    return 0;
}

int load_mcu_rgb565_padded_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py) {
    u8 *y;
    u8 *cb;
    u8 *cr;
    u32 width;
    const u16 *pixels;
    u8 r;
    u8 b;
    int h;
    int i;
    int xx;
    int yy;
    int w;
    int endX;
    int endY;
    int stride;

    y = dst + 4;
    cb = dst + 0x104;
    cr = dst + 0x144;
    width = ctx->alignedWidth;
    pixels = (const u16 *)ctx->pixels;
    stride = width >> 2;
    if (ctx->lastMcuX == px) {
        w = ctx->edgeWidth;
    } else {
        w = 16 / ctx->scale;
    }
    if (ctx->lastMcuY == py) {
        h = ctx->edgeHeight;
    } else {
        h = 16 / ctx->scale;
    }
    endX = px + w;
    endY = py + h;
    for (yy = py; yy < endY; yy++) {
        for (xx = px; xx < endX; xx++) {
            u16 p;
            u8 g;

            p = (pixels + ((yy & 3) << 2))[
                (xx & 3) + (((xx >> 2) + ((yy >> 2) * stride)) << 4)];
            r = p >> 8;
            g = p >> 3;
            b = p << 3;
            r &= 0xF8;
            g &= 0xFC;
            b &= 0xF8;
            y[0] = rgb_luma(r, g, b);
            if ((xx & 1) == 0 && (yy & 1) == 0) {
                *cb++ = ((b - y[0]) * 0x90 + 0x80) >> 8;
                *cr++ = ((r - y[0]) * 0xB7 + 0x80) >> 8;
            }
            y++;
        }
        for (i = w; i < 16; i++) {
            *y = y[-1];
            y++;
            if ((i & 1) == 0 && (yy & 1) == 0) {
                *cb = cb[-1];
                cb++;
                *cr = cr[-1];
                cr++;
            }
        }
    }
    for (; h < 16; h++) {
        memcpy(y, y - 16, 16);
        y += 16;
        if ((h & 1) == 0) {
            memcpy(cb, cb - 8, 8);
            memcpy(cr, cr - 8, 8);
            cb += 8;
            cr += 8;
        }
    }
    return 0;
}
