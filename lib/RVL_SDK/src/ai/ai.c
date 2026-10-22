#include <private/ai.h>
#include <revolution/ai.h>

#include <private/dsp.h>
#include <revolution/dsp.h>

#include <private/hollywood.h>

#include <private/os.h>
#include <revolution/os.h>

const char* __AIVersion = "<< RVL_SDK - AI \trelease build: Aug  8 2007 01:58:12 (0x4199_60831) >>";

static AIDCallback __AID_Callback;

static void* __CallbackStack;
static void* __OldStack;

static s64 bound_32KHz;
static s64 bound_48KHz;

static s64 min_wait;
static s64 max_wait;
static s64 buffer;

static BOOL __AID_Active;
static BOOL __AI_init_flag;

static void __AICallbackStackSwitch(AIDCallback callback);
static void __AI_SRC_INIT(void);

AIDCallback AIRegisterDMACallback(AIDCallback callback) {
    BOOL enabled;
    AIDCallback old;

    old = __AID_Callback;
    enabled = OSDisableInterrupts();

    __AID_Callback = callback;

    OSRestoreInterrupts(enabled);
    return old;
}

void AIInitDMA(u32 startAddr, u32 length) {
    BOOL enabled;

    enabled = OSDisableInterrupts();

    DSP_WRITE_REG(DSP_DMA_START_ADDR_H, (DSP_READ_REG(DSP_DMA_START_ADDR_H) & ~0x1FFF) | (startAddr >> 16 & 0xFFFF));

    DSP_WRITE_REG(DSP_DMA_START_ADDR_L, (DSP_READ_REG(DSP_DMA_START_ADDR_L) & ~0xFFE0) | (startAddr & 0xFFFF));

    DSP_WRITE_REG(DSP_DMA_CONTROL_LENGTH, (DSP_READ_REG(DSP_DMA_CONTROL_LENGTH) & ~0x7FFF) | ((length / 32) & 0xFFFF));

    OSRestoreInterrupts(enabled);
}

void AIStartDMA(void) {
    DSP_SET_REG_F(DSP_DMA_CONTROL_LENGTH, (1 << DSP_DMA_CONTROL_LENGTH_CTRL));
}

u32 AIGetDMABytesLeft(void) {
    return (DSP_READ_REG(DSP_DMA_BYTES_LEFT) & 0x7FFF) * 32;
}

u32 AIGetDMAStartAddr(void) {
    return ((DSP_READ_REG(DSP_DMA_START_ADDR_H) & 0x1FFF) << 16) | (DSP_READ_REG(DSP_DMA_START_ADDR_L) & 0xFFE0);
}

u32 AIGetDMALength(void) {
    return (DSP_READ_REG(DSP_DMA_CONTROL_LENGTH) & 0x7FFF) << 5;
}

BOOL AICheckInit(void) {
    return __AI_init_flag;
}

static void AISetDSPSampleRate(u32 rate) {
    BOOL enabled;

    if (rate != AIGetDSPSampleRate()) {
        AI_DEL_REG_F(AI_CONTROL, (1 << AI_CONTROL_SAMPLERATE));

        if (rate == AI_DSP_SAMPLE_RATE_32KHZ) {
            enabled = OSDisableInterrupts();

            __AI_SRC_INIT();
            AI_SET_REG_F(AI_CONTROL, (1 << AI_CONTROL_SAMPLERATE));

            OSRestoreInterrupts(enabled);
        }
    }
}

static u32 AIGetDSPSampleRate(void) {
    return ((AI_READ_REG(AI_CONTROL) & (1 << AI_CONTROL_SAMPLERATE)) >> 6) ^ 1;
}

void AIInit(void* stack) {
    if (__AI_init_flag != TRUE) {
        OSRegisterVersion(__AIVersion);

        bound_32KHz = OSNanosecondsToTicks(31524);
        bound_48KHz = OSNanosecondsToTicks(42024);
        min_wait = OSNanosecondsToTicks(42000);
        max_wait = OSNanosecondsToTicks(63000);
        buffer = OSNanosecondsToTicks(3000);

        AI_DEL_REG_F(AI_CONTROL, (1 << AI_CONTROL_PSTAT) | (1 << AI_CONTROL_AIINTMSK) | (1 << AI_CONTROL_AIINTVLD));

        AI_WRITE_REG(AI_VOLUME, 0);
        AI_WRITE_REG(AI_AIIT, 0);

        AI_WRITE_REG(AI_CONTROL, (AI_READ_REG(AI_CONTROL) & ~(1 << AI_CONTROL_SCRESET)) | (1 << AI_CONTROL_SCRESET));

        AISetDSPSampleRate(AI_DSP_SAMPLE_RATE_32KHZ);

        __AID_Callback = NULL;
        __CallbackStack = stack;

        __OSSetInterruptHandler(__OS_INTERRUPT_DSP_AI, __AIDHandler);
        __OSUnmaskInterrupts(OS_INTERRUPTMASK_DSP_AI);

        __AI_init_flag = TRUE;
    }
}

void __AIDHandler(__OSInterrupt intr, OSContext* ctx) {
#pragma unused(intr)

    OSContext tempCtx;

    DSP_WRITE_REG(DSP_CONTROL_STATUS,
                  (DSP_READ_REG(DSP_CONTROL_STATUS) & ~((1 << DSP_CONTROL_STATUS_ARINT) | (1 << DSP_CONTROL_STATUS_DSPINT))) |
                      (1 << DSP_CONTROL_STATUS_AIDINT));

    OSClearContext(&tempCtx);
    OSSetCurrentContext(&tempCtx);

    if (__AID_Callback != NULL && !__AID_Active) {
        __AID_Active = TRUE;

        if (__CallbackStack != NULL) {
            __AICallbackStackSwitch(__AID_Callback);
        } else {
            __AID_Callback();
        }

        __AID_Active = FALSE;
    }

    OSClearContext(&tempCtx);
    OSSetCurrentContext(ctx);
}

// clang-format off
static asm void __AICallbackStackSwitch(register AIDCallback callback) {
    mr r31, callback

    lis r5, __OldStack@ha
    addi r5, r5, __OldStack@l
    stw r1, 0(r5)

    lis r5, __CallbackStack@ha
    addi r5, r5, __CallbackStack@l
    lwz r1, 0(r5)

    subi r1, r1, 0x8
    mtlr r31
    blrl

    lis r5, __OldStack@ha
    addi r5, r5, __OldStack@l
    lwz r1, 0(r5)

    frfree
    blr
}
// clang-format on

static void __AI_SRC_INIT(void) {
    s64 start = 0;
    s64 end = 0;
    BOOL exit = FALSE;
    s64 diff = 0;
    s64 wait = 0;
    u32 samples = 0;

    while (!exit) {
        AI_WRITE_REG(AI_CONTROL, (AI_READ_REG(AI_CONTROL) & ~(1 << AI_CONTROL_SCRESET)) | (1 << AI_CONTROL_SCRESET));

        AI_DEL_REG_F(AI_CONTROL, (1 << AI_CONTROL_AFR));

        AI_WRITE_REG(AI_CONTROL, (AI_READ_REG(AI_CONTROL) & ~(1 << AI_CONTROL_PSTAT)) | (1 << AI_CONTROL_PSTAT));

        samples = AI_READ_REG(AI_AISCNT) & ~0x80000000;
        while (samples == (AI_READ_REG(AI_AISCNT) & ~0x80000000)) {
            ;
        }

        start = OSGetTime();

        AI_WRITE_REG(AI_CONTROL, (AI_READ_REG(AI_CONTROL) & ~(1 << AI_CONTROL_AFR)) | (1 << AI_CONTROL_AFR));

        AI_WRITE_REG(AI_CONTROL, (AI_READ_REG(AI_CONTROL) & ~(1 << AI_CONTROL_PSTAT)) | (1 << AI_CONTROL_PSTAT));

        samples = AI_READ_REG(AI_AISCNT) & ~0x80000000;
        while (samples == (AI_READ_REG(AI_AISCNT) & ~0x80000000)) {
            ;
        }

        end = OSGetTime();
        diff = end - start;

        AI_DEL_REG_F(AI_CONTROL, (1 << AI_CONTROL_AFR));
        AI_DEL_REG_F(AI_CONTROL, (1 << AI_CONTROL_PSTAT));

        if (diff < bound_32KHz - buffer) {
            wait = min_wait;
            exit = TRUE;
        } else if (diff >= bound_32KHz + buffer &&
                   diff < bound_48KHz - buffer) {
            wait = max_wait;
            exit = TRUE;
        } else {
            exit = FALSE;
        }
    }

    while (end + wait > OSGetTime()) {
        ;
    }
}
