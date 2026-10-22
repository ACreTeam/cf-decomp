#include <private/gx.h>
#include <revolution/gx.h>

#include <private/os.h>
#include <revolution/os.h>

#include <string.h>


static __GXFifoObj CPUFifo;
static __GXFifoObj GPFifo;
static GXBool CPUFifoReady = FALSE;
static GXBool GPFifoReady = FALSE;
static GXBool CPGPLinked;
static OSThread* __GXCurrentThread;
static volatile BOOL GXOverflowSuspendInProgress;
static GXBreakPtCallback BreakPointCB;
volatile void* __GXCurrentBP;
static volatile u32 __GXOverflowCount;

static void __GXWriteFifoIntEnable(GXBool , GXBool);
static void __GXWriteFifoIntReset(GXBool, GXBool);

#define CP_READ_2U16(dest, regH, regL, type)                                                                                                             {                                                                                                                                                        u32 temp;                                                                                                                                            temp = (u32)GX_GET_CP_REG(regH) << 16;                                                                                                               temp |= (u32)GX_GET_CP_REG(regL);                                                                                                                    dest = (type)OSPhysicalToCached(temp);                                                                                                           }

#define CP_READ_2U16_INT(dest, regH, regL, type)                                                                                                         {                                                                                                                                                        u32 temp;                                                                                                                                            temp = (u32)GX_GET_CP_REG(regH) << 16;                                                                                                               temp |= (u32)GX_GET_CP_REG(regL);                                                                                                                    dest = (type)temp;                                                                                                                               }

static void GXOverflowHandler(__OSInterrupt interrupt, OSContext *context) {
    __GXOverflowCount++;
    __GXWriteFifoIntEnable(GX_FALSE, GX_TRUE);
    __GXWriteFifoIntReset(GX_TRUE, GX_FALSE);
    GXOverflowSuspendInProgress = TRUE;
    OSSuspendThread(__GXCurrentThread);
}

static void GXUnderflowHandler(__OSInterrupt interrupt, OSContext *context) {
    OSResumeThread(__GXCurrentThread);
    GXOverflowSuspendInProgress = FALSE;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
}

static void __GXFifoReadEnable(void) {
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 0, 1); 
    GX_SET_CP_REG(1, (u16)__GXData->cpEnable);
}

static void __GXFifoReadDisable(void) {
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 0, 0); 
    GX_SET_CP_REG(1, (u16)__GXData->cpEnable);
}

static void __GXFifoLink(GXBool en) {
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 4, (en ? 0x1 : 0)); 
    GX_SET_CP_REG(1, (u16)__GXData->cpEnable);
}

static void GXBreakPointHandler(__OSInterrupt interrupt, OSContext *context) {
    OSContext exceptionContext;
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);
    GX_SET_CP_REG(1, __GXData->cpEnable);

    if (BreakPointCB) {
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        (*BreakPointCB)();
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

void GXCPInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
    __GXData->cpStatus = GX_GET_CP_REG(0);
    if (GX_GET_REG_FIELD(__GXData->cpEnable, 1, 3) && GX_GET_REG_FIELD(__GXData->cpStatus, 1, 1)) {
        GXUnderflowHandler(interrupt, context);
    }

    if (GX_GET_REG_FIELD(__GXData->cpEnable, 1, 2) && GX_GET_REG_FIELD(__GXData->cpStatus, 1, 0)) {
        GXOverflowHandler(interrupt, context);
    }

    if (GX_GET_REG_FIELD(__GXData->cpEnable, 1, 5) && GX_GET_REG_FIELD(__GXData->cpStatus, 1, 4)) {
        GXBreakPointHandler(interrupt, context);
    }
}

static void GXInitFifoLimits(GXFifoObj *fifo, u32 hi, u32 lo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    realFifo->hiWatermark = hi;
    realFifo->loWatermark = lo;
}

void GXInitFifoBase(GXFifoObj *fifo, void *base, u32 size) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    realFifo->base = base;
    realFifo->top = (void*)((u32)base + size - 4);
    realFifo->size = size;
    realFifo->count = 0;
    GXInitFifoLimits(fifo, size - (16 * 1024), OSRoundDown32B(size/2));
    GXInitFifoPtrs(fifo, base, base);
}

static void GXInitFifoPtrs(GXFifoObj* fifo, void *readPtr, void* writePtr) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    BOOL en = OSDisableInterrupts();
    realFifo->rdPtr = readPtr;
    realFifo->wrPtr = writePtr;
    realFifo->count = (u8*)writePtr - (u8*)readPtr;
    if (realFifo->count < 0) {
        realFifo->count += realFifo->size;
    }

    OSRestoreInterrupts(en);
}

GXBool CPGPLinkCheck(void) {
    u32 check = 0;
    s32 range1, range2, overlap;

    if (!CPUFifoReady || !GPFifoReady) {
        return GX_FALSE;
    }

    if (CPUFifo.base == GPFifo.base) {
        ++check;
    }

    if (CPUFifo.top == GPFifo.top) {
        ++check;
    }

    if (check == 2) {
        if (CPUFifo.hiWatermark == GPFifo.hiWatermark) {
            ++check;
        }

        if (CPUFifo.loWatermark == GPFifo.loWatermark) {
            ++check;
        }

        return GX_TRUE;
    }
    
    range1 = (s32)((u8*)CPUFifo.top - (u8*)GPFifo.base);
    range2 = (s32)((u8*)GPFifo.top - (u8*)CPUFifo.base);
    overlap = ((range1 > 0) && (range2 > 0) || (range1 < 0 && (range2 < 0)));

    if (overlap) {
        OSReport("CPUFifo: %08X - %08X\n", (u32)CPUFifo.base, (u32)CPUFifo.top);
        OSReport("GP Fifo: %08X - %08X\n", (u32)GPFifo.base, (u32)GPFifo.top);
    }

    return GX_FALSE;
}

void GXSetCPUFifo(GXFifoObj* fifo) {
    const __GXFifoObj* realFifo = (const __GXFifoObj*)fifo;
    BOOL en = OSDisableInterrupts();
    u32 reg;

    if (fifo == NULL) {
        CPUFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        CPUFifo.bind_cpu = CPUFifo.bind_gp = GX_FALSE;
        OSRestoreInterrupts(en);
        return;
    }

    CPUFifo = *realFifo;
    CPUFifoReady = GX_TRUE;
    CPUFifo.bind_cpu = GX_TRUE;

    if (CPGPLinkCheck()) {
        CPGPLinked = GX_TRUE;
        CPUFifo.bind_gp = GX_TRUE;

        reg = 0;
        
        GX_SET_PI_REG(3, GX_PHY_ADDR(CPUFifo.base));
        GX_SET_PI_REG(4, GX_PHY_ADDR(CPUFifo.top));

        GX_SET_REG_FIELD(reg, 24, 5, (GX_PHY_ADDR(CPUFifo.wrPtr) >> 5));
        GX_SET_REG_FIELD(reg, 1, 29, 0);
        GX_SET_PI_REG(5, reg);

        __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    }
    else {
        CPUFifo.bind_gp = GX_FALSE;

        if (CPGPLinked) {
            __GXFifoLink(GX_FALSE);
            CPGPLinked = GX_FALSE;
        }

        __GXWriteFifoIntEnable (GX_FALSE, GX_FALSE);

        reg = 0;

        GX_SET_PI_REG(3, GX_PHY_ADDR(CPUFifo.base));
        GX_SET_PI_REG(4, GX_PHY_ADDR(CPUFifo.top));  
        GX_SET_REG_FIELD(reg, 24, 5, (GX_PHY_ADDR(CPUFifo.wrPtr) >> 5));
        GX_SET_REG_FIELD(reg, 1, 29, 0);
        GX_SET_PI_REG(5, reg);
    }

    PPCSync();
    OSRestoreInterrupts(en);
}

void GXSetGPFifo(GXFifoObj* fifo) {
    const __GXFifoObj* realFifo = (const __GXFifoObj*)fifo;
    BOOL en;
    u32 stbtmp;

    en = OSDisableInterrupts();
    __GXFifoReadDisable();
    __GXWriteFifoIntEnable (GX_FALSE, GX_FALSE);

    if (fifo == NULL) {
        GPFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        __GXFifoLink(GX_FALSE);
        GPFifo.bind_gp = GPFifo.bind_cpu = GX_FALSE;
        OSRestoreInterrupts(en);
        return;
    }

    GPFifo = *realFifo;
    GPFifoReady = GX_TRUE;
    GPFifo.bind_gp = GX_TRUE;
    GX_SET_CP_REG(0x10, GX_PHY_ADDR(GPFifo.base));
    GX_SET_CP_REG(0x12, GX_PHY_ADDR(GPFifo.top));
    GX_SET_CP_REG(0x18, GPFifo.count);
    GX_SET_CP_REG(0x1a, GX_PHY_ADDR(GPFifo.wrPtr));
    GX_SET_CP_REG(0x1c, GX_PHY_ADDR(GPFifo.rdPtr));
    GX_SET_CP_REG(0x14, GPFifo.hiWatermark);
    GX_SET_CP_REG(0x16, GPFifo.loWatermark);
    GX_SET_CP_REG(0x11, GX_PHY_ADDR(GPFifo.base) >> 16);
    GX_SET_CP_REG(0x13, GX_PHY_ADDR(GPFifo.top) >> 16);
    GX_SET_CP_REG(0x19, GPFifo.count >> 16);
    GX_SET_CP_REG(0x1b, GX_PHY_ADDR(GPFifo.wrPtr) >> 16);
    GX_SET_CP_REG(0x1d, GX_PHY_ADDR(GPFifo.rdPtr) >> 16);
    GX_SET_CP_REG(0x15, GPFifo.hiWatermark >> 16);
    GX_SET_CP_REG(0x17, GPFifo.loWatermark >> 16);

    PPCSync();

    if (CPGPLinkCheck()) {
        CPGPLinked = GX_TRUE;
        GPFifo.bind_cpu = GX_TRUE;
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    }
    else {
        CPGPLinked = GX_FALSE;
        GPFifo.bind_cpu = GX_FALSE;
        __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);
        __GXFifoLink(GX_FALSE);
    }

    stbtmp = __GXData->cpEnable;
    GX_SET_REG_FIELD(stbtmp, 1, 1, 0);
    GX_SET_REG_FIELD(stbtmp, 1, 5, 0);
    GX_SET_CP_REG(1, stbtmp);
    GX_SET_CP_REG(1, __GXData->cpEnable);
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(en);
}

static void __GXSaveFifoCPStat(void) {
    CP_READ_2U16(GPFifo.rdPtr, 29, 28, void*);
    CP_READ_2U16_INT(GPFifo.count, 25, 24, s32);
}

static void __GXSaveFifoPIStat(void) {
    u32 reg = GX_GET_PI_REG(5);
    CPUFifo.wrPtr = OSPhysicalToCached(reg & 0x1FFFFFE0);
    CPUFifo.wrap = (GXBool)((reg & 0x20000000) ? GX_TRUE : GX_FALSE);
}

void __GXSaveFifo(void) {
    BOOL en = OSDisableInterrupts();

    if (CPUFifoReady) {
        __GXSaveFifoPIStat();
    }

    if (GPFifoReady) {
        __GXSaveFifoCPStat();
    }

    if (CPGPLinked) {
        CPUFifo.rdPtr = GPFifo.rdPtr;
        CPUFifo.count = GPFifo.count;
        GPFifo.wrPtr = CPUFifo.wrPtr;
        GPFifo.wrap = CPUFifo.wrap;
    }
    else if (CPUFifoReady) {
        CPUFifo.count  = (s32)CPUFifo.wrPtr - (s32)CPUFifo.rdPtr;
        if (CPUFifo.count < 0) {
            CPUFifo.count += CPUFifo.size;
        }
    }

    OSRestoreInterrupts(en);
}

GXBool __GXIsGPFifoReady(void) {
    return GPFifoReady;
}

void GXGetGPStatus(GXBool* overhi, GXBool* underlow, GXBool* readIdle, GXBool* cmdIdle, GXBool* brkpt) {
    __GXData->cpStatus = GX_GET_CP_REG(0);
    *overhi = (u8)GX_GET_REG_FIELD(__GXData->cpStatus, 1, 0);
    *underlow = (u8)GX_GET_REG_FIELD(__GXData->cpStatus, 1, 1);
    *readIdle = (u8)GX_GET_REG_FIELD(__GXData->cpStatus, 1, 2);
    *cmdIdle = (u8)GX_GET_REG_FIELD(__GXData->cpStatus, 1, 3);
    *brkpt = (u8)GX_GET_REG_FIELD(__GXData->cpStatus, 1, 4);
}

GXBool GXGetCPUFifo(GXFifoObj *fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    if (!CPUFifoReady) {
        return GX_FALSE;
    }

    __GXSaveFifo();
    *realFifo = CPUFifo;
    return GX_TRUE;
}

#if 0 // not linked in City Folk
void GXGetFifoPtrs(const GXFifoObj* fifo, void** readPtr, void** writePtr) {
    __GXFifoObj* realFifo = (__GXFifoObj*) fifo;
    *readPtr  = realFifo->rdPtr;
    *writePtr = realFifo->wrPtr;
}
#endif

u32 GXGetFifoCount(const GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    return (u32)realFifo->count;
}

GXBool GXGetFifoWrap(const GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*) fifo;
    return realFifo->wrap;
}

GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb) {
    GXBreakPtCallback oldcb = BreakPointCB;
    BOOL enabled = OSDisableInterrupts();
    BreakPointCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

#if 0 // not linked in City Folk
void GXEnableBreakPt(void* break_pt) {
    BOOL enabled = OSDisableInterrupts();
    __GXFifoReadDisable();
    GX_SET_CP_REG(0x1e, GX_PHY_ADDR(break_pt));
    GX_SET_CP_REG(0x1f, GX_PHY_ADDR(break_pt) >> 16);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 1, 0);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);
    GX_SET_CP_REG(1, __GXData->cpEnable);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 1, 1);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 5, 1);
    GX_SET_CP_REG(1, __GXData->cpEnable);
    __GXCurrentBP = break_pt;
    __GXFifoReadEnable();
    OSRestoreInterrupts(enabled);
}
#endif

#if 0 // not linked in City Folk
void GXDisableBreakPt(void) {
    BOOL enabled = OSDisableInterrupts();
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 1, 0);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);
    GX_SET_CP_REG(1, __GXData->cpEnable);
    __GXCurrentBP = 0;
    OSRestoreInterrupts(enabled);
}
#endif

void __GXFifoInit() {
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_CP, GXCPInterruptHandler);
    __OSUnmaskInterrupts(OS_INTERRUPTMASK_PI_CP);
    __GXCurrentThread = OSGetCurrentThread();
    GXOverflowSuspendInProgress = FALSE;
    memset(&CPUFifo, 0, sizeof(__GXFifoObj));
    memset(&GPFifo, 0, sizeof(__GXFifoObj));
    CPUFifoReady = GX_FALSE;
    GPFifoReady  = GX_FALSE;
}

void __GXCleanGPFifo (void)
{
    BOOL enabled;

    if ( !GPFifoReady) {
        return;
    }

    enabled = OSDisableInterrupts();
    __GXFifoReadDisable();
    __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);
    GPFifo.rdPtr = GPFifo.wrPtr;
    GPFifo.count = 0;
    GX_SET_CP_REG(0x18, GPFifo.count);
    GX_SET_CP_REG(0x1a, GX_PHY_ADDR(GPFifo.wrPtr));
    GX_SET_CP_REG(0x1c, GX_PHY_ADDR(GPFifo.rdPtr));
    GX_SET_CP_REG(0x19, GPFifo.count>>16);
    GX_SET_CP_REG(0x1b, GX_PHY_ADDR(GPFifo.wrPtr)>>16);
    GX_SET_CP_REG(0x1d, GX_PHY_ADDR(GPFifo.rdPtr)>>16);

    PPCSync();

    if (CPGPLinked) {
        u32 reg = 0;
        CPUFifo.rdPtr = GPFifo.rdPtr;
        CPUFifo.wrPtr = GPFifo.wrPtr;
        CPUFifo.count = GPFifo.count;
        GX_SET_REG_FIELD(reg, 24, 5, (GX_PHY_ADDR(CPUFifo.wrPtr) >> 5));
        GX_SET_REG_FIELD(reg, 1, 29, 0);
        GX_SET_PI_REG(5, reg);
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    }

    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 1, 0);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);
    GX_SET_CP_REG(1, __GXData->cpEnable);
    __GXCurrentBP = 0;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(enabled);
}

static void __GXWriteFifoIntEnable(GXBool hi, GXBool lo) {
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 2, hi);
    GX_SET_REG_FIELD(__GXData->cpEnable, 1, 3, lo);
    GX_SET_CP_REG(1, (u16)__GXData->cpEnable);
}

static void __GXWriteFifoIntReset(GXBool hi, GXBool lo) {
    GX_SET_REG_FIELD(__GXData->cpClr, 1, 0, hi);
    GX_SET_REG_FIELD(__GXData->cpClr, 1, 1, lo);
    GX_SET_CP_REG(2, (u16)__GXData->cpClr);
}

OSThread* GXSetCurrentGXThread(void) {
    BOOL enabled;
    OSThread* prev;

    enabled = OSDisableInterrupts();
    prev = __GXCurrentThread;
    __GXCurrentThread = OSGetCurrentThread();
    OSRestoreInterrupts(enabled);
    return prev;
}

OSThread* GXGetCurrentGXThread(void) {
    return __GXCurrentThread;
}
