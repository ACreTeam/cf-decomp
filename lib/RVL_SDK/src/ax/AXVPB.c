#include <internal/fdlibm.h>
#include <private/ax.h>
#include <revolution/ax.h>
#include <revolution/os.h>
#include <string.h>


/**
 * It's really even worse than this: what appears to be manually unrolled
 * copies.
 *
 * I hope that it's just some compiler pattern that I'm unaware of, but in the
 * meantime I've managed to make a macro that still works.
 *
 * (If you compile for -O0 this will totally break functions)
 */
#define FUNNY_COPY(dst, src, type, n)                                          \
    do {                                                                       \
                                                                               \
        type* __dst = (type*)dst;                                              \
        type* __src = (type*)src;                                              \
        u16 i;                                                                 \
                                                                               \
        for (i = 0; i < n / sizeof(type); i++) {                               \
            *__dst = *__src;                                                   \
            __dst++;                                                           \
            __src++;                                                           \
        }                                                                      \
                                                                               \
    } while (0)

#define ADDRHI(x) (((u32)x) >> 16 & 0xFFFF)
#define ADDRLO(x) (((u32)x) & 0xFFFF)

typedef struct _AXITD {
    u8 UNK_0x0[0x40];
} AXITD;

static AXPB* __AXPB = NULL;
static AXPB __s_AXPB[AX_VOICE_MAX] ALIGN32;

static AXITD* __AXITD = NULL;
static AXITD __s_AXITD[AX_VOICE_MAX] ALIGN32;

// Did they forget the alignment here?
static AXVPB* __AXVPB = NULL;
static AXVPB __s_AXVPB[AX_VOICE_MAX];

s32 __AXMaxVoices = 0;
static s32 __AXNumVoices = 0;

static u32 __AXRecDspCycles = 0;
static u32 __AXMaxDspCycles = 0;

static u32 __AXMixCycles[] = {2,   408,  408,  810,  1404, 1404, 1404, 1404,
                              408, 816,  816,  1218, 1812, 1812, 1812, 1812,
                              707, 1115, 1115, 1517, 2111, 2111, 2111, 2111,
                              707, 1115, 1115, 1517, 2111, 2111, 2111, 2111};

static u32 __AXRmtMixCycles[] = {4, 86, 151, 151};

static void __AXVPBInitCommon(void);
static u32 __AXGetSrcCycles(u16 select, const AXPBSRC* src);

s32 __AXGetNumVoices(void) {
    return __AXNumVoices;
}

void __AXServiceVPB(AXVPB* vpb) {
    AXPB* dst;
    AXPB* src;
    u32 sync;

    __AXNumVoices++;

    dst = &__AXPB[vpb->index];
    src = &vpb->pb;
    sync = vpb->sync;

    if (sync == 0) {
        src->state = dst->state;
        src->ve.currentVolume = dst->ve.currentVolume;
        src->addr.currentAddressHi = dst->addr.currentAddressHi;
        src->addr.currentAddressLo = dst->addr.currentAddressLo;
        return;
    }

    if (sync & AX_PBSYNC_ALL) {
        memcpy(dst, src, sizeof(AXPB));
        return;
    }

    if (sync & AX_PBSYNC_SELECT) {
        dst->srcSelect = src->srcSelect;
        dst->coefSelect = src->coefSelect;
    }

    if (sync & AX_PBSYNC_MIXER_CTRL) {
        dst->mixerCtrl = src->mixerCtrl;
    }

    if (sync & AX_PBSYNC_STATE) {
        dst->state = src->state;
    } else {
        src->state = dst->state;
    }

    if (sync & AX_PBSYNC_TYPE) {
        dst->type = src->type;
    }

    if (sync & AX_PBSYNC_MIX) {
        memcpy(&dst->mix, &src->mix, sizeof(AXPBMIX));
    }

    if (sync & AX_PBSYNC_ITD_SHIFT) {
        dst->itd.targetShiftL = src->itd.targetShiftL;
        dst->itd.targetShiftR = src->itd.targetShiftR;
    } else if (sync & AX_PBSYNC_ITD) {
        FUNNY_COPY(&dst->itd, &src->itd, u16, sizeof(AXPBITD));
        __memclr(vpb->itdBuffer, sizeof(AXITD));
    }

    if (sync & AX_PBSYNC_DPOP) {
        memcpy(&dst->dpop, &src->dpop, sizeof(AXPBDPOP));
    }

    if (sync & AX_PBSYNC_VE_DELTA) {
        src->ve.currentVolume = dst->ve.currentVolume;
        dst->ve.currentDelta = src->ve.currentDelta;
    } else if (sync & AX_PBSYNC_VE) {
        dst->ve.currentVolume = src->ve.currentVolume;
        dst->ve.currentDelta = src->ve.currentDelta;
    }

    if (sync & (AX_PBSYNC_LOOP_FLAG | AX_PBSYNC_LOOP_ADDR | AX_PBSYNC_END_ADDR |
                AX_PBSYNC_CURR_ADDR)) {
        if (sync & AX_PBSYNC_LOOP_FLAG) {
            dst->addr.loopFlag = src->addr.loopFlag;
        }

        if (sync & AX_PBSYNC_LOOP_ADDR) {
            *(u32*)&dst->addr.loopAddressHi = *(u32*)&src->addr.loopAddressHi;
        }

        if (sync & AX_PBSYNC_END_ADDR) {
            *(u32*)&dst->addr.endAddressHi = *(u32*)&src->addr.endAddressHi;
        }

        if (sync & AX_PBSYNC_CURR_ADDR) {
            *(u32*)&dst->addr.currentAddressHi =
                *(u32*)&src->addr.currentAddressHi;
        } else {
            *(u32*)&src->addr.currentAddressHi =
                *(u32*)&dst->addr.currentAddressHi;
        }
    } else if (sync & AX_PBSYNC_ADDR) {
        FUNNY_COPY(&dst->addr, &src->addr, u32, sizeof(AXPBADDR));
    } else {
        src->addr.currentAddressHi = dst->addr.currentAddressHi;
        src->addr.currentAddressLo = dst->addr.currentAddressLo;
    }

    if (sync & AX_PBSYNC_ADPCM) {
        FUNNY_COPY(&dst->adpcm, &src->adpcm, u32, sizeof(AXPBADPCM));
    }

    if (sync & AX_PBSYNC_SRC_RATIO) {
        dst->src.ratioHi = src->src.ratioHi;
        dst->src.ratioLo = src->src.ratioLo;
    } else if (sync & AX_PBSYNC_SRC) {
        FUNNY_COPY(&dst->src, &src->src, u16, sizeof(AXPBSRC));
    }

    if (sync & AX_PBSYNC_ADPCM_LOOP) {
        FUNNY_COPY(&dst->adpcmLoop, &src->adpcmLoop, u16,
                   sizeof(AXPBADPCMLOOP));
    }

    if (sync & AX_PBSYNC_LPF_COEFS) {
        dst->lpf.a0 = src->lpf.a0;
        dst->lpf.b0 = src->lpf.b0;
    } else if (sync & AX_PBSYNC_LPF) {
        FUNNY_COPY(&dst->lpf, &src->lpf, u16, sizeof(AXPBLPF));
    }

    if (sync & AX_PBSYNC_BIQUAD_COEFS) {
        dst->biquad.b0 = src->biquad.b0;
        dst->biquad.b1 = src->biquad.b1;
        dst->biquad.b2 = src->biquad.b2;
        dst->biquad.a1 = src->biquad.a1;
        dst->biquad.a2 = src->biquad.a2;
    } else if (sync & AX_PBSYNC_BIQUAD) {
        FUNNY_COPY(&dst->biquad, &src->biquad, u16, sizeof(AXPBBIQUAD));
    }

    if (sync & AX_PBSYNC_REMOTE) {
        dst->remote = src->remote;
    }

    if (sync & AX_PBSYNC_RMT_MIXER_CTRL) {
        dst->rmtMixerCtrl = src->rmtMixerCtrl;
    }

    if (sync & AX_PBSYNC_RMTMIX) {
        memcpy(&dst->rmtMix, &src->rmtMix, sizeof(AXPBRMTMIX));
    }

    if (sync & AX_PBSYNC_RMTDPOP) {
        memcpy(&dst->rmtDpop, &src->rmtDpop, sizeof(AXPBRMTDPOP));
    }

    if (sync & AX_PBSYNC_RMTSRC) {
        memcpy(&dst->rmtSrc, &src->rmtSrc, sizeof(AXPBRMTSRC));
    }

    if (sync & AX_PBSYNC_RMTIIR_LPF_COEFS) {
        dst->rmtIIR.lpf.a0 = src->rmtIIR.lpf.a0;
        dst->rmtIIR.lpf.b0 = src->rmtIIR.lpf.b0;
    } else if (sync & AX_PBSYNC_RMTIIR_BIQUAD_COEFS) {
        dst->rmtIIR.biquad.b0 = src->rmtIIR.biquad.b0;
        dst->rmtIIR.biquad.b1 = src->rmtIIR.biquad.b1;
        dst->rmtIIR.biquad.b2 = src->rmtIIR.biquad.b2;
        dst->rmtIIR.biquad.a1 = src->rmtIIR.biquad.a1;
        dst->rmtIIR.biquad.a2 = src->rmtIIR.biquad.a2;
    } else if (sync & AX_PBSYNC_RMTIIR) {
        FUNNY_COPY(&dst->rmtIIR, &src->rmtIIR, u16, sizeof(AXPBRMTIIR));
    }
}

static void __AXDumpVPB(AXVPB* vpb) {
    AXPB* pb = &__AXPB[vpb->index];

    if (pb->state == AX_VOICE_RUN) {
        __AXDepopVoice(pb);
    }

    vpb->pb.state = AX_VOICE_STOP;
    pb->state = AX_VOICE_STOP;

    __AXPushCallbackStack(vpb);
}

void __AXSyncPBs(u32 baseCycles) {
    u32 cycles;
    u32 prio;
    AXVPB* head;

    __AXNumVoices = 0;

    DCInvalidateRange(__AXPB, __AXMaxVoices * sizeof(AXPB));
    DCInvalidateRange(__AXITD, __AXMaxVoices * sizeof(AXITD));

    cycles = 32 + __AXGetCommandListCycles() + __AXMaxVoices * 600 + baseCycles;

    for (prio = AX_PRIORITY_MAX; prio > AX_PRIORITY_FREE; prio--) {
        for (head = __AXGetStackHead(prio); head != NULL; head = head->next) {
            if (head->pb.itd.flag == 1) {
                cycles += 129;
            }

            if (head->depop) {
                __AXDepopVoice(&__AXPB[head->index]);
            }

            if (head->pb.state == AX_VOICE_RUN) {
                cycles += 387;

                if (head->pb.lpf.on) {
                    cycles += 309;
                }

                if (head->pb.biquad.on) {
                    cycles += 1024;
                }

                if (head->pb.itd.flag == 1) {
                    cycles += 27;
                }

                cycles += __AXGetSrcCycles(head->pb.srcSelect, &head->pb.src);
                cycles += __AXMixCycles[head->pb.mixerCtrl >> 0 & 31] +
                          __AXMixCycles[head->pb.mixerCtrl >> 16 & 31] +
                          __AXMixCycles[head->pb.mixerCtrl >> 21 & 31] +
                          __AXMixCycles[head->pb.mixerCtrl >> 26 & 31];

                if (head->pb.remote == TRUE) {
                    cycles += 613;

                    if (head->pb.rmtIIR.lpf.on == AX_PB_LPF_ON) {
                        cycles += 118;
                    } else if (head->pb.rmtIIR.biquad.on == AX_PB_BIQUAD_ON) {
                        cycles += 834;
                    }

                    cycles +=
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 0 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 2 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 4 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 6 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 8 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 10 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 12 & 3] +
                        __AXRmtMixCycles[head->pb.rmtMixerCtrl >> 14 & 3];
                }

                if (__AXMaxDspCycles > cycles) {
                    __AXServiceVPB(head);
                } else {
                    __AXDumpVPB(head);
                }

            } else {
                __AXServiceVPB(head);
            }

            head->sync = 0;
            head->depop = FALSE;
        }
    }

    __AXRecDspCycles = cycles;

    for (head = __AXGetStackHead(AX_PRIORITY_FREE); head != NULL;
         head = head->next) {
        if (head->depop) {
            __AXDepopVoice(&__AXPB[head->index]);
        }

        head->depop = FALSE;
        __AXPB[head->index].state = AX_VOICE_STOP;
    }

    DCFlushRange(__AXPB, __AXMaxVoices * sizeof(AXPB));
    DCFlushRange(__AXITD, __AXMaxVoices * sizeof(AXITD));
}

AXPB* __AXGetPBs(void) {
    return __AXPB;
}

void __AXSetPBDefault(AXVPB* vpb) {
    vpb->pb.state = AX_VOICE_STOP;
    vpb->pb.itd.flag = 0;
    vpb->sync = AX_PBSYNC_STATE | AX_PBSYNC_ITD | AX_PBSYNC_LPF |
                AX_PBSYNC_BIQUAD | AX_PBSYNC_REMOTE | AX_PBSYNC_RMTSRC |
                AX_PBSYNC_RMTIIR;
    vpb->pb.lpf.on = 0;
    vpb->pb.biquad.on = 0;
    vpb->pb.remote = 0;
    vpb->pb.rmtIIR.lpf.on = 0;
    vpb->pb.rmtSrc.currentAddressFrac = 0;
    vpb->pb.rmtSrc.last_samples[0] = 0;
    vpb->pb.rmtSrc.last_samples[1] = 0;
    vpb->pb.rmtSrc.last_samples[2] = 0;
    vpb->pb.rmtSrc.last_samples[3] = 0;
}

void __AXVPBInit(void) {
    __AXMaxVoices = AX_VOICE_MAX;

    __AXPB = __s_AXPB;
    __AXITD = __s_AXITD;
    __AXVPB = __s_AXVPB;

    __AXVPBInitCommon();
}

static void __AXVPBInitCommon(void) {
    u32 i;
    u32* dst;

    __AXMaxDspCycles = OS_BUS_CLOCK / 667;
    __AXRecDspCycles = 0;

    for (dst = (u32*)__AXPB, i = __AXMaxVoices * (sizeof(AXPB) / sizeof(u32));
         i > 0; i--) {
        *dst++ = 0;
    }

    for (dst = (u32*)__AXITD, i = __AXMaxVoices * (sizeof(AXITD) / sizeof(u32));
         i > 0; i--) {
        *dst++ = 0;
    }

    for (dst = (u32*)__AXVPB, i = __AXMaxVoices * (sizeof(AXVPB) / sizeof(u32));
         i > 0; i--) {
        *dst++ = 0;
    }

    for (i = 0; i < __AXMaxVoices; i++) {
        AXPB* pb = &__AXPB[i];
        AXITD* itd = &__AXITD[i];
        AXVPB* vpb = &__AXVPB[i];

        vpb->index = i;
        vpb->itdBuffer = itd;
        __AXSetPBDefault(vpb);

        if (i == __AXMaxVoices - 1) {
            pb->nextHi = pb->nextLo = 0;
            vpb->pb.nextHi = vpb->pb.nextLo = 0;
        } else {
            vpb->pb.nextHi = ADDRHI((u32)pb + 320);
            vpb->pb.nextLo = ADDRLO((u32)pb + 320);

            pb->nextHi = ADDRHI((u32)pb + 320);
            pb->nextLo = ADDRLO((u32)pb + 320);
        }

        vpb->pb.currHi = ADDRHI(pb);
        vpb->pb.currLo = ADDRLO(pb);

        pb->currHi = ADDRHI(pb);
        pb->currLo = ADDRLO(pb);

        vpb->pb.itd.bufferHi = ADDRHI(itd);
        vpb->pb.itd.bufferLo = ADDRLO(itd);

        pb->itd.bufferHi = ADDRHI(itd);
        pb->itd.bufferLo = ADDRLO(itd);

        vpb->priority = AX_PRIORITY_MIN;
        __AXPushFreeStack(vpb);
    }

    DCFlushRange(__AXPB, __AXMaxVoices * sizeof(AXPB));
}

void AXSetVoiceSrcType(AXVPB* vpb, u32 type) {
    BOOL old;
    AXPB* ppb;

    old = OSDisableInterrupts();
    ppb = &vpb->pb;
    switch (type) {
    case AX_SRC_TYPE_NONE:
        ppb->srcSelect = 2;
        break;
    case AX_SRC_TYPE_LINEAR:
        ppb->srcSelect = 1;
        break;
    case AX_SRC_TYPE_4TAP_8K:
        ppb->srcSelect = 0;
        ppb->coefSelect = 0;
        break;
    case AX_SRC_TYPE_4TAP_12K:
        ppb->srcSelect = 0;
        ppb->coefSelect = 1;
        break;
    case AX_SRC_TYPE_4TAP_16K:
        ppb->srcSelect = 0;
        ppb->coefSelect = 2;
        break;
    }

    vpb->sync |= AX_PBSYNC_SELECT;
    OSRestoreInterrupts(old);
}

void AXSetVoiceState(AXVPB* vpb, u16 state) {
    BOOL enabled = OSDisableInterrupts();

    if (vpb->pb.state == state) {
        OSRestoreInterrupts(enabled);
        return;
    }

    vpb->pb.state = state;
    vpb->sync |= AX_PBSYNC_STATE;

    if (state == AX_VOICE_STOP) {
        vpb->depop = TRUE;
    }

    OSRestoreInterrupts(enabled);
}

#if 0 // not linked in City Folk
void AXSetVoiceType(AXVPB* vpb, u16 type) {
    BOOL old;

    old = OSDisableInterrupts();
    vpb->pb.type = type;
    vpb->sync |= AX_PBSYNC_TYPE;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceMix(AXVPB* vpb, AXPBMIX* mix) {
    BOOL old;
    s32 mixerCtrl;
    u16* dst;
    u16* src;

    src = (u16*)mix;
    dst = (u16*)&vpb->pb.mix;
    mixerCtrl = 0;

    old = OSDisableInterrupts();

    if ((*dst++ = *src++))
        mixerCtrl |= 0x1;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x5;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x2;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x6;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x10000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x50000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x20000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x60000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x200000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0xA00000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x400000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0xC00000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x4000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x14000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x8000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x18000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x8;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x18;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x80000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x180000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x1000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x3000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x20000000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x60000000;

    vpb->pb.mixerCtrl = mixerCtrl;
    vpb->sync |= AX_PBSYNC_MIX | AX_PBSYNC_MIXER_CTRL;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceItdTarget(AXVPB* p, u16 lShift, u16 rShift) {
    BOOL old;

    old = OSDisableInterrupts();
    p->pb.itd.targetShiftL = lShift;
    p->pb.itd.targetShiftR = rShift;
    p->sync |= AX_PBSYNC_ITD_SHIFT;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceVe(AXVPB* vpb, AXPBVE* ve) {
    BOOL old;

    old = OSDisableInterrupts();
    vpb->pb.ve.currentVolume = ve->currentVolume;
    vpb->pb.ve.currentDelta = ve->currentDelta;
    vpb->sync |= AX_PBSYNC_VE;
    OSRestoreInterrupts(old);
}
#endif

void AXSetVoiceAddr(AXVPB* vpb, AXPBADDR* addr) {
    BOOL enabled = OSDisableInterrupts();

    u32* dst = (u32*)&vpb->pb.addr;
    const u32* src = (u32*)addr;

    // :(
    *dst = *src;
    dst++;
    src++;

    *dst = *src;
    dst++;
    src++;

    *dst = *src;
    dst++;
    src++;

    *dst = *src;

    switch (addr->format) {
    case AX_SAMPLE_FORMAT_PCM_S16:
        dst++;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0x08000000;
        *dst = 0;
        break;
    case AX_SAMPLE_FORMAT_PCM_S8:
        dst++;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0;
        *dst++ = 0x01000000;
        *dst = 0;
        break;
    case AX_SAMPLE_FORMAT_DSP_ADPCM:
    default:
        break;
    }

    vpb->sync &= ~(AX_PBSYNC_LOOP_FLAG | AX_PBSYNC_LOOP_ADDR |
                   AX_PBSYNC_END_ADDR | AX_PBSYNC_CURR_ADDR);
    vpb->sync |= AX_PBSYNC_ADDR | AX_PBSYNC_ADPCM;

    OSRestoreInterrupts(enabled);
}

#if 0 // not linked in City Folk
void AXSetVoiceLoop(AXVPB* p, u16 loop) {
    BOOL old;

    old = OSDisableInterrupts();
    p->pb.addr.loopFlag = loop;
    p->sync |= AX_PBSYNC_LOOP_FLAG;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceLoopAddr(AXVPB* p, u32 addr) {
    BOOL old;

    old = OSDisableInterrupts();
    p->pb.addr.loopAddressHi = addr >> 16;
    p->pb.addr.loopAddressLo = addr;
    p->sync |= AX_PBSYNC_LOOP_ADDR;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceEndAddr(AXVPB* p, u32 addr) {
    BOOL old;

    old = OSDisableInterrupts();
    p->pb.addr.endAddressHi = addr >> 16;
    p->pb.addr.endAddressLo = addr;
    p->sync |= AX_PBSYNC_END_ADDR;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceCurrentAddr(AXVPB* p, u32 addr) {
    BOOL old;

    old = OSDisableInterrupts();
    p->pb.addr.currentAddressHi = addr >> 16;
    p->pb.addr.currentAddressLo = addr;
    p->sync |= AX_PBSYNC_CURR_ADDR;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceAdpcm(AXVPB* vpb, AXPBADPCM* adpcm) {
    BOOL old;
    u32* dst;
    u32* src;

    dst = (void*)&vpb->pb.adpcm;
    src = (void*)adpcm;

    old = OSDisableInterrupts();

    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;

    vpb->sync |= AX_PBSYNC_ADPCM;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceSrc(AXVPB* vpb, AXPBSRC* src_) {
    BOOL old;
    u16* dst;
    u16* src;

    dst = (void*)&vpb->pb.src;
    src = (void*)src_;

    old = OSDisableInterrupts();

    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;

    vpb->sync &= ~AX_PBSYNC_SRC_RATIO;
    vpb->sync |= AX_PBSYNC_SRC;
    OSRestoreInterrupts(old);
}
#endif

void AXSetVoiceSrcRatio(AXVPB* vpb, float ratio) {
    u32 r;
    BOOL old;

    old = OSDisableInterrupts();
    r = 65536.0f * ratio;
    vpb->pb.src.ratioHi = r >> 16;
    vpb->pb.src.ratioLo = r;
    vpb->sync |= AX_PBSYNC_SRC_RATIO;
    OSRestoreInterrupts(old);
}

#if 0 // not linked in City Folk
void AXSetVoiceAdpcmLoop(AXVPB* vpb, AXPBADPCMLOOP* adpcmloop) {
    BOOL old;
    u16* dst;
    u16* src;

    dst = (void*)&vpb->pb.adpcmLoop;
    src = (void*)adpcmloop;
    old = OSDisableInterrupts();

    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;

    vpb->sync |= AX_PBSYNC_ADPCM_LOOP;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceLpf(AXVPB* vpb, AXPBLPF* lpf) {
    BOOL old;
    u16* dst;
    u16* src;

    dst = (u16*)&vpb->pb.lpf;
    src = (u16*)lpf;

    old = OSDisableInterrupts();

    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;
    vpb->sync |= AX_PBSYNC_LPF;

    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceLpfCoefs(AXVPB* vpb, u16 a0, u16 b0) {
    BOOL old;

    old = OSDisableInterrupts();

    vpb->pb.lpf.a0 = a0;
    vpb->pb.lpf.b0 = b0;
    vpb->sync |= AX_PBSYNC_LPF_COEFS;

    OSRestoreInterrupts(old);
}
#endif

#define fM_PI 3.14159265358979323846f

void AXGetLpfCoefs(u16 freq, u16* a, u16* b) {
    f32 rf31 = 2.0f - (f32)cos(2 * fM_PI * freq / AX_SAMPLE_RATE);
    f32 rf30 = (f32)sqrt(rf31 * rf31 - 1.0f) - rf31;

    *b = 32768 * -rf30;
    *a = 32767 - *b;
}

#if 0 // not linked in City Folk
void AXSetVoiceRmtOn(AXVPB* vpb, u16 on) {
    BOOL old = OSDisableInterrupts();

    vpb->pb.remote = on;
    vpb->sync |= AX_PBSYNC_REMOTE;
    OSRestoreInterrupts(old);
}
#endif

#if 0 // not linked in City Folk
void AXSetVoiceRmtMix(AXVPB* vpb, AXPBRMTMIX* mix) {
    BOOL old;
    u16 mixerCtrl;
    u16* dst;
    u16* src;

    src = (u16*)mix;
    dst = (u16*)&vpb->pb.rmtMix;
    mixerCtrl = 0;

    old = OSDisableInterrupts();

    if ((*dst++ = *src++))
        mixerCtrl |= 0x1;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x2;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x4;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x8;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x10;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x20;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x40;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x80;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x100;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x200;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x400;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x800;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x1000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x2000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x4000;
    if ((*dst++ = *src++))
        mixerCtrl |= 0x8000;

    vpb->pb.rmtMixerCtrl = mixerCtrl;
    vpb->sync |= AX_PBSYNC_RMTMIX | AX_PBSYNC_RMT_MIXER_CTRL;
    OSRestoreInterrupts(old);
}
#endif

void AXSetMaxDspCycles(u32 num) {
    __AXMaxDspCycles = num;
}

static u32 __AXGetSrcCycles(u16 select, const AXPBSRC* src) {
    u32 ratio = src->ratioHi << 16 | src->ratioLo;

    if (select == 0) {
        return (ratio * 512 + 32768) / 65536 + 1561;
    }

    if (select == 1) {
        return (ratio * 512 + 32768) / 65536 + 1466;
    }

    return 605;
}

s32 AXGetMaxVoices(void) {
    return __AXMaxVoices;
}
