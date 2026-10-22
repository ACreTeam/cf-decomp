#ifndef METROTRK_DOLPHIN_TRK_GLUE_H
#define METROTRK_DOLPHIN_TRK_GLUE_H


#include <private/os/OSInterrupt.h>
#include <MetroTRK/dserror.h>
#include <revolution/os/OSContext.h>
#include <revolution/os/OSInterrupt.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum HardwareType
    {
        HARDWARE_AMC_DDH = 0,
        HARDWARE_GDEV = 1,
        HARDWARE_BBA = 2,
    } HardwareType;

    typedef int UARTError;
    typedef int (*DBCommFunc)(void);
    typedef int (*DBCommInitFunc)(void*, __OSInterruptHandler);
    typedef int (*DBCommReadFunc)(u8*, int);
    typedef int (*DBCommWriteFunc)(const u8*, int);

    typedef struct DBCommTable
    {
        DBCommInitFunc initialize_func;
        DBCommFunc init_interrupts_func;
        DBCommFunc shutdown_func;
        DBCommFunc peek_func;
        DBCommReadFunc read_func;
        DBCommWriteFunc write_func;
        DBCommFunc open_func;
        DBCommFunc close_func;
        DBCommFunc pre_continue_func;
        DBCommFunc post_stop_func;
    } DBCommTable;

    extern BOOL _MetroTRK_Has_Framing;
    extern u8 TRK_Use_BBA;
    extern DBCommTable gDBCommTable;

    void TRKLoadContext(OSContext* ctx, u32 r4);
    void TRKEXICallBack(s16 interrupt, OSContext* ctx);
    int InitMetroTRKCommTable(int hwId);
    DSError TRKInitializeIntDrivenUART(u32, u32, u32, void*);
    void EnableEXI2Interrupts(void);
    int TRKPollUART(void);
    UARTError TRKReadUARTN(void* bytes, u32 length);
    UARTError TRKWriteUARTN(const void* bytes, u32 length);
    void ReserveEXI2Port(void);
    void UnreserveEXI2Port(void);
    void TRK_board_display(char* str);
    DSError InitializeProgramEndTrap(void);
    void TRKUARTInterruptHandler(void);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_DOLPHIN_TRK_GLUE_H
