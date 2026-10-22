#include <MetroTRK/nubinit.h>
#include <MetroTRK/usr_put.h>
#include <MetroTRK/serpoll.h>

BOOL gTRKBigEndian;

DSError TRKInitializeNub(void)
{
    BOOL ret;
    DSError uartErr;

    ret = TRKInitializeEndian();

    if (ret == kNoError)
    {
        usr_put_initialize();
    }
    if (ret == kNoError)
    {
        ret = TRKInitializeEventQueue();
    }
    if (ret == kNoError)
    {
        ret = TRKInitializeMessageBuffers();
    }
    if (ret == kNoError)
    {
        ret = TRKInitializeDispatcher();
    }
    InitializeProgramEndTrap();
    if (ret == kNoError)
    {
        ret = TRKInitializeSerialHandler();
    }
    if (ret == kNoError)
    {
        ret = TRKInitializeTarget();
    }
    if (ret == kNoError)
    {
        uartErr = TRKInitializeIntDrivenUART(0x0000e100, 1, 0, &gTRKInputPendingPtr);
        TRKTargetSetInputPendingPtr(gTRKInputPendingPtr);
        if (uartErr != kNoError)
        {
            ret = uartErr;
        }
    }
    return ret;
}

DSError TRKTerminateNub(void)
{
    TRKTerminateSerialHandler();
    return kNoError;
}

void TRKNubWelcome(void)
{
    TRK_board_display("MetroTRK for Revolution v0.1");
    return;
}

BOOL TRKInitializeEndian(void)
{
    u8 bendian[4];
    BOOL result = FALSE;
    gTRKBigEndian = TRUE;

    bendian[0] = 0x12;
    bendian[1] = 0x34;
    bendian[2] = 0x56;
    bendian[3] = 0x78;

    if (*(u32*)bendian == 0x12345678)
    {
        gTRKBigEndian = TRUE;
    }
    else if (*(u32*)bendian == 0x78563412)
    {
        gTRKBigEndian = FALSE;
    }
    else
    {
        result = TRUE;
    }
    return result;
}
