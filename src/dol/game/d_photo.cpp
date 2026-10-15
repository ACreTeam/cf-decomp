// .text 800FACC0..800FBB58, .ctors 804656F4..804656F8, .rodata 804759F0..80475A00, .bss 805CEB20..805D2420,
// .sbss 8074E5E0..8074E600, .sdata2 80750A60..80750A70.
#include <game/game/d_photo.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_thunder.hpp>
#include <game/mLib/m_3d/m_3d_capture.hpp>
#include <game/mLib/m_perf.hpp>
#include <game/mLib/m_tex.hpp>
#include <lib/revolution/OS/OSThread.h>
#include <revolution/SC.h>

// The TMC JPEG encoder (lib/MWM/jpeg/tmc_jpeg.c). Both return 0 on success.
extern "C" {
int fn_8036D514(void *work, const void *image, int width, int height, int format, u32 maxSize, u32 *size,
                u32 *quality); // the quality that fits maxSize
int fn_8036D698(void *work, const void *image, int width, int height, int format, u32 quality, void *dst,
                u32 dstSize, u32 *size); // encode
}

#define PHOTO_WIDTH 640
#define PHOTO_WIDTH_MAX 896
#define PHOTO_HEIGHT 480

static m3d::capture_c l_capture;
static OSThread l_thread;
static u8 l_stack[0x2000] ALIGN(32);
// The source position of each destination column / row, in quarter texels.
static int l_srcX[PHOTO_WIDTH_MAX + 1];
static int l_srcY[PHOTO_HEIGHT + 1];

static int l_mode;
static bool l_small;
static int l_step;
static void *l_encodeWork;
static u16 *l_image;
static u8 *l_jpeg;
static bool l_captured;
static bool l_encoded;
static u32 l_jpegSize;

using namespace dPhoto;

// 800FACC0
int dPhoto::getMode() {
    return l_mode;
}

// 800FACC8
void dPhoto::request(int mode) {
    if (l_mode != MODE_NONE) {
        return;
    }
    if (mode == MODE_CAPTURE) {
        l_step = STEP_CAPTURE;
        l_mode = mode;
        l_captured = false;
        l_encoded = false;
        l_jpegSize = 0;
    } else if (mode == MODE_ENCODE) {
        l_step = STEP_ENCODE;
        l_mode = mode;
        l_encoded = false;
        l_jpegSize = 0;
    }
}

// 800FAD20
void dPhoto::requestCapture() {
    request(MODE_CAPTURE);
}

// 800FAD28
void dPhoto::requestEncode(bool small) {
    request(MODE_ENCODE);
    l_small = small;
}

// 800FAD5C
void *dPhoto::getTexBuffer() {
    return l_capture.mTexBuffer;
}

// 800FAD6C
int dPhoto::getTexWidth() {
    return GXGetTexObjWidth(&l_capture.mTexObj);
}

// 800FAD9C
int dPhoto::getTexHeight() {
    return GXGetTexObjHeight(&l_capture.mTexObj);
}

// 800FADCC
u8 *dPhoto::getJpeg() {
    return l_jpeg;
}

// 800FADD4
u32 dPhoto::getJpegSize() {
    return l_jpegSize;
}

// 800FADDC
BOOL dPhoto::isCaptured() {
    return l_captured;
}

// 800FADE4
void dPhoto::clear() {
    l_captured = false;
    l_encoded = false;
}

// 800FADF4
static void *photoThread(void *) {
    while (true) {
        if (l_step == STEP_ENCODING) {
            int width;
            int height;
            u32 maxSize;
            if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
                if (l_small) {
                    width = PHOTO_WIDTH_MAX;
                    height = PHOTO_HEIGHT;
                    maxSize = 70 * 1024;
                } else {
                    width = PHOTO_WIDTH_MAX;
                    height = PHOTO_HEIGHT;
                    maxSize = 100 * 1024;
                }
            } else {
                if (l_small) {
                    width = PHOTO_WIDTH;
                    height = PHOTO_HEIGHT;
                    maxSize = 70 * 1024;
                } else {
                    width = PHOTO_WIDTH;
                    height = PHOTO_HEIGHT;
                    maxSize = 100 * 1024;
                }
            }

            int srcWidth = GXGetTexObjWidth(&l_capture.mTexObj);
            int srcHeight = GXGetTexObjHeight(&l_capture.mTexObj);
            int srcWidth4 = srcWidth * 4;
            int srcHeight4 = srcHeight * 4;
            for (int i = width; i >= 0; i--) {
                int position = (f32)srcWidth4 * i / width;
                l_srcX[i] = position;
            }
            for (int i = height; i >= 0; i--) {
                int position = (f32)srcHeight4 * i / height;
                l_srcY[i] = position;
            }

            mTex::edit16b_c src(0, 0, NULL);
            mTex::edit16b_c dst(0, 0, NULL);
            src.init(srcWidth, srcHeight, l_capture.mTexBuffer);
            dst.init(width, height, l_image);

            // Each destination texel is the average of the source texels it covers.
            for (int x = width - 1; x >= 0; x--) {
                for (int y = height - 1; y >= 0; y--) {
                    int x0 = l_srcX[x] >> 2;
                    int x1 = l_srcX[x + 1] >> 2;
                    int y0 = l_srcY[y] >> 2;
                    int y1 = l_srcY[y + 1] >> 2;
                    u16 sampleCount = 0;
                    u32 b = 0;
                    u32 g = 0;
                    u32 r = 0;
                    int sx = x0;
                    do {
                        int sy = y0;
                        do {
                            u16 color = src.get(sx, sy);
                            b += color & 0x1F;
                            g += (color >> 5) & 0x3F;
                            r += (color >> 11) & 0x1F;
                            sampleCount++;
                        } while (++sy < y1);
                    } while (++sx < x1);
                    if (sampleCount > 1) {
                        b = (f32)b / sampleCount;
                        g = (f32)g / sampleCount;
                        r = (f32)r / sampleCount;
                    }
                    if (b > 0x1F) {
                        b = 0x1F;
                    }
                    if (g > 0x3F) {
                        g = 0x3F;
                    }
                    if (r > 0x1F) {
                        r = 0x1F;
                    }
                    dst.set(x, y, ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F), FALSE);
                }
            }

            if (l_encodeWork != NULL) {
                u32 size = 0;
                u32 quality = 0;
                if (fn_8036D514(l_encodeWork, l_image, width, height, 4, maxSize, &size, &quality) == 0 &&
                    quality >= 1 && quality <= 500 &&
                    fn_8036D698(l_encodeWork, l_image, width, height, 4, quality, l_jpeg, maxSize, &size) == 0 &&
                    size <= maxSize)
                {
                    l_jpegSize = size;
                    l_encoded = true;
                }
            }
            l_step = STEP_NONE;
        }
        OSSuspendThread(&l_thread);
    }
}

// 800FB868
void dPhoto::create(EGG::Heap *heap) {
    l_mode = MODE_NONE;
    l_step = STEP_NONE;
    l_captured = false;
    l_encoded = false;
    l_jpegSize = 0;
    EGG::Video *video = dSys::sConfiguration->mpVideo;
    if (video == NULL) {
        return;
    }
    // Preserve the height-before-width loads; MWCC otherwise folds them into argument order.
    const volatile GXRenderModeObj *mode = video->pRenderMode;
    u16 height = mode->efbHeight;
    if (!l_capture.create(heap, mode->fbWidth, height, GX_TF_RGB565, GX_NEAR, false)) {
        return;
    }
    l_encodeWork = heap->alloc(0x1C00, 32);
    l_image = (u16 *)heap->alloc(PHOTO_WIDTH_MAX * PHOTO_HEIGHT * sizeof(u16), 32);
    const u32 jpegSizeKB[3] = {100, 70, 100}; // This line pushed it to 99.65, but only [2] is used, and feels a bit off to me. I think it's a good candidate to keep in mind while going for 100%.
    l_jpeg = (u8 *)heap->alloc(jpegSizeKB[2] * 1024, 32);
    OSCreateThread(&l_thread, photoThread, NULL, l_stack + sizeof(l_stack), sizeof(l_stack), 29, OS_THREAD_DETACHED);
    mPerf::setThreadColor(&l_thread, dPerf::getThreadColor(14));
}

// 800FB9A0
void dPhoto::capture() {
    if (l_step == STEP_CAPTURE) {
        if (dSys::sConfiguration->mpVideo != NULL) {
            l_capture.capture(0, 0, false);
            l_step = STEP_CAPTURED;
        }
    } else if (l_step == STEP_CAPTURED) {
        l_step = STEP_NONE;
        l_mode = MODE_NONE;
        l_captured = true;
    }
}

// 800FBA18
void dPhoto::execute() {
    if (l_mode != MODE_ENCODE) {
        return;
    }
    if (l_step == STEP_ENCODE) {
        if (OSIsThreadSuspended(&l_thread)) {
            l_step = STEP_ENCODING;
            OSResumeThread(&l_thread);
        }
    } else if (l_step == STEP_NONE) {
        if (OSIsThreadSuspended(&l_thread)) {
            l_mode = MODE_NONE;
        }
    }
}

// 800FBAA0
BOOL dPhoto::tryClear() {
    if (l_mode == MODE_NONE) {
        if (l_captured) {
            l_captured = false;
        }
        if (l_encoded) {
            l_encoded = false;
        }
        return TRUE;
    }
    return FALSE;
}
