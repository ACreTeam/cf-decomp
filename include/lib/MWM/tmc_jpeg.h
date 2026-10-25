#ifndef RVL_MWM_TMC_JPEG_H
#define RVL_MWM_TMC_JPEG_H
#include <revolution/gx.h>
#ifdef __cplusplus
extern "C" {
#endif

// TMC JPEG encoder (RVL_MWM - TMC_JPEG070330). Function names are not known; these are descriptive.

// format: GX_TF_RGB565 or GX_TF_RGBA8 (GX-tiled source image)
// Finds the quality whose output fits maxSize. Returns 0 on success.
int TMCJPEG_EncodeFitSize(void* work, const void* image, int width, int height, GXTexFmt format, u32 maxSize, u32* size,
                          u32* quality);
int TMCJPEG_GetEncodeSize(void* work, const void* image, int width, int height, GXTexFmt format, u32* size, u32 quality);
// Encodes image into dst at the given quality. Returns 0 on success.
int TMCJPEG_Encode(void* work, const void* image, int width, int height, GXTexFmt format, u32 quality, void* dst,
                   u32 dstSize, u32* size);

#ifdef __cplusplus
}
#endif
#endif
