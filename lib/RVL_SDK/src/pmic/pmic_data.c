#include <private/pmic.h>

PMICRings __iinfo;
PMICScratch __sinfo;
static s32 __init;

/* 0x80228EB0 (0xE8): copies up to `count` captured samples out of the capture
 * ring. */
s32 PMICRead(s16 *dst, s32 count) {
    u32 level;
    s32 n;
    s32 read;
    s32 capacity;
    s16 *buffer;
    s32 i;
    if (__init == 0) {
        return PMIC_ERR_INVALID;
    }
    if (dst == NULL || count < 0) {
        return PMIC_ERR_INVALID;
    }
    level = OSDisableInterrupts();
    n = __iinfo.captureCount;
    if (n >= count) {
        n = count;
    }
    capacity = __iinfo.captureCapacity;
    read = __iinfo.captureRead;
    buffer = __iinfo.captureBuffer;
    for (i = n; i > 0; i--) {
        *dst++ = buffer[read++];
        if (read >= capacity) {
            read = 0;
        }
    }
    __iinfo.captureRead = read;
    __iinfo.captureCount -= n;
    OSRestoreInterrupts(level);
    return n;
}

/* 0x80228F98 (0x13C): appends `count` samples to the playback ring. */
s32 PMICWrite(s16 *src, s32 count) {
    u32 level;
    s32 overflow = 0;
    s32 capacity;
    s32 write;
    s16 *buffer;
    s32 i;
    if (__init == 0) {
        return PMIC_ERR_INVALID;
    }
    if (src == NULL || count < 0 || count > __iinfo.playCapacity) {
        return PMIC_ERR_INVALID;
    }
    if (__PMICGetState() < PMIC_STATE_READY) {
        return 0;
    }
    level = OSDisableInterrupts();
    capacity = __iinfo.playCapacity;
    write = __iinfo.playWrite;
    buffer = __iinfo.playBuffer;
    if (count > capacity - __iinfo.playCount) {
        overflow = 1;
    }
    for (i = count; i > 0; i--) {
        buffer[write++] = *src++;
        if (write >= capacity) {
            write = 0;
        }
    }
    if (overflow != 0) {
        __iinfo.playCount = capacity;
        __iinfo.playWrite = write;
        __iinfo.playRead = write;
        __iinfo.playUnderrun = 1;
        __iinfo.playFault = 1;
    } else {
        __iinfo.playWrite = write;
        __iinfo.playCount += count;
    }
    OSRestoreInterrupts(level);
    return count;
}

/* filter coefficients (symmetric, 64 taps) used by PMICConvertOutput. */
static s16 __decimation_taps[64] = {
    -2,   1,    8,     5,    -13,  -17,  15,    41,   0,    -68,   -37,
    86,   108,  -68,   -198, -10,  282,  172,   -302, -409, 199,   682,
    98,   -903, -643,  945,  1499, -585, -2859, -800, 6267, 12906, 12906,
    6267, -800, -2859, -585, 1499, 945,  -643,  -903, 98,   682,   199,
    -409, -302, 172,   282,  -10,  -198, -68,   108,  86,   -37,   -68,
    0,    41,   15,    -17,  -13,  5,    8,     1,    -2,
};

/* 0x802290D4 (0x590): converts the AI output buffer into the echo-cancel
 * reference samples. */
s32 PMICConvertOutput(s16 *dmaBuffer, s32 samples, u32 mode, s16 *out,
                      BOOL reset) {
    s32 keep;
    s16 *tailLeft;
    s16 *tailRight;
    s32 n;
    s16 *taps;
    s16 *histRight;
    s16 *inLeft;
    s16 *inRight;
    s32 produced;
    s32 i;
    s16 *histLeft;
    s32 step;
    s32 frames;
    s32 k;
    s32 j;
    s32 pos;
    s32 sumL;
    s32 sumR;

    if (__init == 0) {
        return PMIC_ERR_INVALID;
    }
    if (dmaBuffer == NULL || samples < 0 || samples > 0x1200 ||
        (mode != 0 && mode != 1) || out == NULL) {
        return PMIC_ERR_INVALID;
    }
    if (reset == 0) {
        if (mode != __sinfo.mode) {
            return PMIC_ERR_INVALID;
        }
    }
    if (mode == 1) {
        return 0;
    }
    if (reset != 0) {
        __sinfo.mode = mode;
        __sinfo.histLen = 0x3F;
        memset(__sinfo.histRight, 0, 0x80);
        memset(__sinfo.histLeft, 0, 0x80);
    }
    frames = samples >> 1;
    if (frames == 0) {
        return 0;
    }
    pos = __sinfo.histLen;
    histLeft = (s16 *)__sinfo.histLeft;
    histRight = (s16 *)__sinfo.histRight;
    for (i = 0; i < frames; i++) {
        histLeft[pos] = dmaBuffer[i * 2];
        histRight[pos++] = dmaBuffer[i * 2 + 1];
    }
    if (mode == 0) {
        taps = __decimation_taps;
        produced = (pos - 0x40) / 2 + 1;
        __sinfo.histLen = (pos & 1) + 0x3E;
        step = 2;
    }
    for (i = 0; i < produced; i++) {
        sumL = 0;
        sumR = 0;
        inRight = histRight + i * 2;
        inLeft = histLeft + i * 2;
        for (k = 0; k < 64; k++) {
            sumL += taps[k] * inLeft[k];
            sumR += taps[k] * inRight[k];
        }
        sumL = (sumL + 0x4000) >> 15;
        sumR = (sumR + 0x4000) >> 15;
        if (sumL > 0x7FFF) {
            sumL = 0x7FFF;
        } else if (sumL < -0x8000) {
            sumL = -0x8000;
        }
        if (sumR > 0x7FFF) {
            sumR = 0x7FFF;
        } else if (sumR < -0x8000) {
            sumR = -0x8000;
        }
        out[i * 2] = sumL;
        out[i * 2 + 1] = sumR;
    }
    keep = __sinfo.histLen;
    tailLeft = (s16 *)__sinfo.histLeft;
    tailRight = (s16 *)__sinfo.histRight;
    j = produced * step;
    for (n = 0; n < keep; n++) {
        tailLeft[n] = tailLeft[j];
        tailRight[n] = tailRight[j++];
    }
    return produced * 2;
}

#pragma dont_inline on

/* 0x80229664 (0xF4): lays the capture ring, the playback ring and the scratch
 * areas out in `area`. */
u8 *__PMICDataInit(u8 *area) {
    area = (u8 *)(((u32)area + 0x1F) & ~0x1F);
    __iinfo.captureCapacity = 0x900;
    __iinfo.captureCount = 0;
    __iinfo.captureWrite = 0;
    __iinfo.captureRead = 0;
    __iinfo.captureOverflow = 0;
    __iinfo.captureBuffer = (s16 *)area;
    memset(area, 0, 0x1200);
    __iinfo.playCapacity = 0x600;
    __iinfo.playCount = 0;
    __iinfo.playWrite = 0;
    __iinfo.playRead = 0;
    __iinfo.playPrimed = 0;
    __iinfo.playUnderrun = 0;
    __iinfo.playOverrun = 0;
    __iinfo.playFault = 0;
    __iinfo.playBuffer = (s16 *)(area + 0x1200);
    memset(area + 0x1200, 0, 0xC00);
    __sinfo.mode = 0;
    __sinfo.histLen = 0x3F;
    __sinfo.histRight = area + 0x1E00;
    memset(area + 0x1E00, 0, 0x80);
    __sinfo.histLeft = area + 0x3080;
    memset(area + 0x3080, 0, 0x80);
    __init = 1;
    return area + 0x4300;
}

/* 0x80229758 (0xC): marks the rings as unusable. */
void __PMICDataQuit(void) {
    __init = 0;
}

/* 0x80229764 (0x2C): empties the capture ring. */
void __PMICDataReadReset(void) {
    __iinfo.captureCount = 0;
    __iinfo.captureWrite = 0;
    __iinfo.captureRead = 0;
    __iinfo.captureOverflow = 0;
    memset(__iinfo.captureBuffer, 0, 0x1200);
}

/* 0x80229790 (0x30): empties the playback ring. */
void __PMICDataWriteReset(void) {
    __iinfo.playCount = 0;
    __iinfo.playWrite = 0;
    __iinfo.playRead = 0;
    __iinfo.playPrimed = 0;
    __iinfo.playFault = 0;
    memset(__iinfo.playBuffer, 0, 0xC00);
}

/* 0x802297C0 (0x64): empties the playback ring and clears its error counters.
 */
void __PMICDataWriteResetAll(void) {
    __iinfo.playCount = 0;
    __iinfo.playWrite = 0;
    __iinfo.playRead = 0;
    __iinfo.playPrimed = 0;
    __iinfo.playFault = 0;
    memset(__iinfo.playBuffer, 0, 0xC00);
    __iinfo.playUnderrun = 0;
    __iinfo.playOverrun = 0;
}

#pragma dont_inline reset

/* 0x80229824 (0xFC): appends captured samples to the capture ring while
 * recording. */
s32 __PMICDataRecvData(s16 *src, s32 bytes) {
    s32 overflow = 0;
    s32 n;
    s32 write;
    s32 capacity;
    s16 *buffer;
    s32 i;
    if (__PMICGetState() != PMIC_STATE_RECORDING) {
        return 1;
    }
    n = bytes / 2;
    capacity = __iinfo.captureCapacity;
    write = __iinfo.captureWrite;
    buffer = __iinfo.captureBuffer;
    if (n > capacity - __iinfo.captureCount) {
        overflow = 1;
    }
    for (i = n; i > 0; i--) {
        buffer[write++] = *src++;
        if (write >= capacity) {
            write = 0;
        }
    }
    if (overflow != 0) {
        __iinfo.captureCount = capacity;
        __iinfo.captureWrite = write;
        __iinfo.captureRead = write;
        __iinfo.captureOverflow = 1;
    } else {
        __iinfo.captureWrite = write;
        __iinfo.captureCount += n;
    }
    return 1;
}

/* 0x80229920 (0x10C): moves playback samples into an isochronous packet,
 * swapping each pair. */
s32 __PMICDataSendData(s16 *dst, s32 bytes) {
    s32 n = bytes / 2;
    s16 *first;
    s32 read;
    s32 capacity;
    s16 *second;
    s16 *buffer;
    s32 i;
    if (__iinfo.playCount >= n) {
        second = dst + 1;
        first = dst;
        read = __iinfo.playRead;
        capacity = __iinfo.playCapacity;
        buffer = __iinfo.playBuffer;
        for (i = n / 2; i > 0; i--) {
            *second = buffer[read++];
            *first = buffer[read++];
            second += 2;
            first += 2;
            if (read >= capacity) {
                read = 0;
            }
        }
        __iinfo.playRead = read;
        __iinfo.playCount -= n;
        if (__iinfo.playPrimed < 0x10) {
            __iinfo.playPrimed++;
        }
    } else {
        if (__iinfo.playPrimed < 0x10) {
            __iinfo.playPrimed = 0;
        } else {
            __iinfo.playOverrun = 1;
            __iinfo.playFault = 1;
        }
        memset(dst, 0, bytes);
    }
    return __iinfo.playFault == 0;
}
