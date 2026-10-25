// TMC JPEG encoder (RVL_MWM - TMC_JPEG070330).
// Function and type names are descriptive, not original.
#include <MWM/tmc_jpeg_internal.h>
#include <revolution/os.h>
#include <string.h>

// Version string registered with OSRegisterVersion
const char *__TMCJPEGVersion = "<< RVL_MWM - TMC_JPEG070330 \trelease build: "
                               "Mar 30 2007 10:15:00 (0x4199_60831) >>";

int TMCJPEG_EncodeFitSize(void *work, const void *image, int width, int height,
                          GXTexFmt format, u32 maxSize, u32 *size,
                          u32 *quality) {
    TMCJPEGParams params;
    TMCJPEGCtx ctx;
    int ret;
    int result;

    *size = 0;
    *quality = 0;

    if (width > 0xFFFF || width <= 0 || height > 0xFFFF || height <= 0) {
        ret = -1;
    } else if (format != GX_TF_RGB565 && format != GX_TF_RGBA8) {
        ret = -4;
    } else {
        ret = 0;
    }
    if (ret < 0) {
        return ret;
    }

    params.width = width;
    params.work = work;
    params.height = height;
    params.sampling = 2;
    params.mode = 1;
    params.quality = 0;
    params.restartInterval = 0;
    params.format = format;
    params.bufSize = 0;
    params.maxSize = maxSize;

    ret = search_quality(&ctx, image, &params);
    if (ret < 0) {
        return ret;
    }

    params.quality = ret;
    params.work = work;
    params.bufSize = 0x400;
    if (params.bufSize >= 0x5B4) {
        return -0x90;
    }

    result = start_compress(&ctx, (u8 *)work + 0x164C, image, &params);
    if (result < 0) {
        return result;
    }

    do {
        ret = encode_mcus(&ctx, result, image);
        if (ret < 0) {
            if (ret != -3) {
                return ret;
            }
            break;
        }
    } while (ret > 0);

    *quality = ctx.quality;
    *size = ctx.outSize;
    return 0;
}

int TMCJPEG_GetEncodeSize(void *work, const void *image, int width, int height,
                          GXTexFmt format, u32 *size, u32 quality) {
    TMCJPEGParams params;
    TMCJPEGCtx ctx;
    int ret;
    int result;

    *size = 0;

    if (width > 0xFFFF || width <= 0 || height > 0xFFFF || height <= 0) {
        ret = -1;
    } else if (format != GX_TF_RGB565 && format != GX_TF_RGBA8) {
        ret = -4;
    } else {
        ret = 0;
    }
    if (ret < 0) {
        return ret;
    }

    params.width = width;
    params.height = height;
    params.sampling = 2;
    params.mode = 1;
    params.restartInterval = 0;
    params.format = format;
    params.maxSize = 0;
    params.quality = quality;
    params.work = work;
    params.bufSize = 0x400;
    if (params.bufSize >= 0x5B4) {
        return -0x90;
    }

    result = start_compress(&ctx, (u8 *)work + 0x164C, image, &params);
    if (result < 0) {
        return result;
    }

    do {
        ret = encode_mcus(&ctx, result, image);
        if (ret < 0) {
            if (ret != -3) {
                return ret;
            }
            break;
        }
    } while (ret > 0);

    *size = ctx.outSize;
    return 0;
}

int TMCJPEG_Encode(void *work, const void *image, int width, int height,
                   GXTexFmt format, u32 quality, void *dst, u32 dstSize,
                   u32 *size) {
    TMCJPEGParams params;
    TMCJPEGCtx ctx;
    int ret;
    int result;

    OSRegisterVersion(__TMCJPEGVersion);

    *size = 0;
    params.mode = 1;

    if (width > 0xFFFF || width <= 0 || height > 0xFFFF || height <= 0) {
        ret = -1;
    } else if (dstSize < 0x25F) {
        ret = -3;
    } else if (format != GX_TF_RGB565 && format != GX_TF_RGBA8) {
        ret = -4;
    } else {
        ret = 0;
    }
    if (ret < 0) {
        return ret;
    }

    params.width = width;
    params.height = height;
    params.sampling = 2;
    params.mode = 1;
    params.quality = quality;
    params.restartInterval = 0;
    params.format = format;
    params.work = work;
    params.bufSize = dstSize;

    result = start_compress(&ctx, dst, image, &params);
    if (result < 0) {
        return result;
    }

    do {
        ret = encode_mcus(&ctx, result, image);
        if (ret < 0) {
            *size = ctx.outSize;
            return ret;
        }
    } while (ret > 0);

    *size = ctx.outSize;
    return 0;
}
