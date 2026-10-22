#include <MetroTRK/UDP_Stubs.h>
#include <MetroTRK/cc_gdev.h>
#include <MetroTRK/dolphin_trk_glue.h>
#include <MetroTRK/mem_TRK.h>
#include <MetroTRK/targimpl.h>
#include <revolution/base/PPCArch.h>
#include <revolution/os/OSCache.h>
#include <revolution/os/OSError.h>
#include <revolution/os/OSThread.h>

BOOL _MetroTRK_Has_Framing;

u8 TRK_Use_BBA;

DBCommTable gDBCommTable = { };

void TRKEXICallBack(s16 param_0, OSContext* ctx)
{
    OSEnableScheduler();
    TRKLoadContext(ctx, 0x500);
}

int InitMetroTRKCommTable(int hwId)
{
    int result = 1;
    OSReport("Devkit set to : %ld\n", hwId);
    OSReport("MetroTRK : Sizeof Reply - %ld bytes\n", 64);
    TRK_Use_BBA = 0;

    if (hwId == HARDWARE_BBA)
    {
        OSReport("MetroTRK : Set to BBA\n");
        TRK_Use_BBA = 1;
        gDBCommTable.initialize_func = udp_cc_initialize;
        gDBCommTable.open_func = udp_cc_open;
        gDBCommTable.close_func = udp_cc_close;
        gDBCommTable.read_func = udp_cc_read;
        gDBCommTable.write_func = udp_cc_write;
        gDBCommTable.shutdown_func = udp_cc_shutdown;
        gDBCommTable.peek_func = udp_cc_peek;
        gDBCommTable.pre_continue_func = udp_cc_pre_continue;
        gDBCommTable.post_stop_func = udp_cc_post_stop;
        gDBCommTable.init_interrupts_func = NULL;
        return 0;
    }
    else if (hwId == HARDWARE_GDEV)
    {
        OSReport("MetroTRK : Set to NDEV hardware\n");
        gDBCommTable.initialize_func = (DBCommInitFunc)gdev_cc_initialize;
        gDBCommTable.open_func = gdev_cc_open;
        gDBCommTable.close_func = gdev_cc_close;
        gDBCommTable.read_func = gdev_cc_read;
        gDBCommTable.write_func = gdev_cc_write;
        gDBCommTable.shutdown_func = gdev_cc_shutdown;
        gDBCommTable.peek_func = gdev_cc_peek;
        gDBCommTable.pre_continue_func = gdev_cc_pre_continue;
        gDBCommTable.post_stop_func = gdev_cc_post_stop;
        gDBCommTable.init_interrupts_func = gdev_cc_initinterrupts;
        return 0;
    }
    else
    {
        OSReport("MetroTRK : Set to UNKNOWN hardware. (%ld)\n", hwId);
        OSReport("MetroTRK : Invalid hardware ID passed from OS\n");
        OSReport("MetroTRK : Defaulting to GDEV Hardware\n");
        return 1;
    }
}

DSError TRKInitializeIntDrivenUART(u32 param_0, u32 param_1, u32 param_2, void* param_3)
{
    gDBCommTable.initialize_func(param_3, TRKEXICallBack);
    gDBCommTable.open_func();
    return kNoError;
}

void EnableEXI2Interrupts(void)
{
    if (!TRK_Use_BBA)
    {
        if (gDBCommTable.init_interrupts_func != NULL)
        {
            gDBCommTable.init_interrupts_func();
        }
    }
}

int TRKPollUART(void)
{
    return gDBCommTable.peek_func();
}

UARTError TRKReadUARTN(void* bytes, u32 length)
{
    int readErr = gDBCommTable.read_func(bytes, length);
    return (-readErr | readErr) >> 31;
}

UARTError TRKWriteUARTN(const void* bytes, u32 length)
{
    int writeErr = gDBCommTable.write_func(bytes, length);
    return (-writeErr | writeErr) >> 31;
}

void ReserveEXI2Port(void)
{
    gDBCommTable.post_stop_func();
}

void UnreserveEXI2Port(void)
{
    gDBCommTable.pre_continue_func();
}

void TRK_board_display(char* str)
{
    OSReport("%s\n", str);
}

DSError InitializeProgramEndTrap(void)
{
    static const u32 EndofProgramInstruction = 'END';

    u8* endOfProgramInstructionBytes = (u8*)&EndofProgramInstruction;
    u8* ppcHaltPtr = (u8*)PPCHalt;
    TRK_memcpy(ppcHaltPtr + 4, endOfProgramInstructionBytes, 4);
    ICInvalidateRange(ppcHaltPtr + 4, 4);
    DCFlushRange(ppcHaltPtr + 4, 4);
}

void TRKUARTInterruptHandler(void)
{
}

// clang-format off
asm void TRKLoadContext(OSContext* ctx, u32)
{
    nofralloc
    lwz r0, OSContext.gpr[0](r3)
    lwz r1, OSContext.gpr[1](r3)
    lwz r2, OSContext.gpr[2](r3)
    lhz r5, OSContext.state(r3)
    rlwinm. r6, r5, 0, 0x1e, 0x1e
    beq loadVolatileContext
    rlwinm r5, r5, 0, 0x1f, 0x1d
    sth r5, OSContext.state(r3)
    lmw r5, OSContext.gpr[5](r3)
    b loadControlContext
loadVolatileContext:
    lmw r13, OSContext.gpr[13](r3)
loadControlContext:
    mr r31, r3
    mr r3, r4
    lwz r4, OSContext.cr(r31)
    mtcrf 0xff, r4
    lwz r4, OSContext.lr(r31)
    mtlr r4
    lwz r4, OSContext.ctr(r31)
    mtctr r4
    lwz r4, OSContext.xer(r31)
    mtxer r4
    mfmsr r4
    rlwinm r4, r4, 0, 0x11, 0xf
    rlwinm r4, r4, 0, 0x1f, 0x1d
    mtmsr r4
    mtsprg 1, r2
    lwz r4, OSContext.gpr[3](r31)
    mtsprg 2, r4
    lwz r4, OSContext.gpr[4](r31)
    mtsprg 3, r4
    lwz r2, OSContext.srr0(r31)
    lwz r4, OSContext.srr1(r31)
    lwz r31, OSContext.gpr[31](r31)
    b TRKInterruptHandler
}
// clang-format on
