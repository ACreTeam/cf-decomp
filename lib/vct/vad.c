#include "vad.h"

#include <revolution/os.h>

#include "fir.h"
#include "vct.h"
#include "vct_main.h"

#define DEFAULT_RELEASE_COUNT 5

u32 VCTi_GetAudioFrameLength(void);

static VCTVADInfo sVADInfo = {FALSE, 0, DEFAULT_RELEASE_COUNT, DEFAULT_RELEASE_COUNT * 68};

static u32 sNoiseThreshold;
static u32 sIndex;
static int sLostCount;

static u32 E2scales[4];

// Frames quieter than this are never voice.
static u32 sMinScale = 0x31;
// Activity starts at this percentage of the averaged background level, and
// stops (after the release count) at the second percentage.
static u16 sStartRatio = 0x208;
static u16 sStopRatio = 0x118;

void VCT_SetVADReleaseTime(int count) {
    sVADInfo.releaseCount = count;
    sVADInfo.releaseTime = sVADInfo.releaseCount * VCTi_GetAudioFrameLength();
}

static inline void reset_E2(void) {
    int i;

    for (i = 0; i < 4; i++) {
        E2scales[i] = 0x01000000;
    }

    sIndex = 0;
}

void VCT_ResetVAD(void) {
    reset_E2();
    sNoiseThreshold = 0;
    sVADInfo.activity = FALSE;
    sVADInfo.scale = 0;
    sLostCount = 0;
}

static inline u32 calc_avg_E2(void) {
    int i;
    u32 E2 = 0;

    for (i = 0; i < 4; i++) {
        E2 += E2scales[i];
    }

    return SQRT(E2 / 4);
}

static inline u32 calc_threshold(void) {
    int i;
    u32 E2 = 0;

    for (i = 0; i < 4; i++) {
        E2 += E2scales[i];
    }

    E2 = SQRT(E2 / 4);
    return E2 * sStopRatio / 100;
}

static inline void store_E2(u32 scale) {
    E2scales[sIndex] = scale;
    sIndex = (sIndex + 1) % 4;
}

VCTVADStatus VCTi_GetVADStatus(u32 scale, u32 E1, u8* outScale) {
    int result;

    sVADInfo.scale = E1 >> 8;
    if (outScale != NULL) {
        *outScale = E1 >> 8;
    }

    if (scale < sMinScale) {
        return VCT_VAD_NONE;
    }

    if (!sVADInfo.activity) {
        u32 E2 = calc_avg_E2();
        if (E1 != 0 && E1 >= E2 * sStartRatio / 100) {
            sNoiseThreshold = calc_threshold();
            sIndex = 0;
            result = VCT_VAD_DETECT;
            sVADInfo.activity = TRUE;
        } else {
            result = VCT_VAD_NONE;
        }
        store_E2(scale);
    } else {
        u32 E2;

        store_E2(scale);

        E2 = calc_avg_E2();
        if (E2 <= sNoiseThreshold) {
            if (++sLostCount > sVADInfo.releaseCount) {
                sVADInfo.activity = FALSE;
                sLostCount = 0;
                sIndex = 0;
                return VCT_VAD_LOST;
            }
        } else {
            sLostCount = 0;
        }
        result = VCT_VAD_ACTIVE;
    }

    return result;
}

void vct_init_vad(void) {
    sVADInfo.releaseCount = DEFAULT_RELEASE_COUNT;
    sVADInfo.releaseTime = sVADInfo.releaseCount * VCTi_GetAudioFrameLength();
    reset_E2();
    sNoiseThreshold = 0;
    sVADInfo.activity = FALSE;
    sVADInfo.scale = 0;
    sLostCount = 0;
}
