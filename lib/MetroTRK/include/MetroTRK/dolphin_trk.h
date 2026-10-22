#ifndef METROTRK_DOLPHIN_TRK_H
#define METROTRK_DOLPHIN_TRK_H

#include <MetroTRK/dserror.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum TRKExceptionVector
    {
        PPC_SystemReset = 0x100,
        PPC_MachineCheck = 0x200,
        PPC_DataStorage = 0x300,
        PPC_InstructionStorage = 0x400,
        PPC_ExternalInterrupt = 0x500,
        PPC_Alignment = 0x600,
        PPC_Program = 0x700,
        PPC_FloatingPointUnavaiable = 0x800,
        PPC_Decrementer = 0x900,
        PPC_SystemCall = 0xC00,
        PPC_Trace = 0xD00,
        PPC_PerformanceMonitor = 0xF00,
        PPC_InstructionAddressBreakpoint = 0x1300,
        PPC_SystemManagementInterrupt = 0x1400,
        PPC_ThermalManagementInterrupt = 0x1700,
    } TRKExceptionVector;

    void InitMetroTRK(void);
    void InitMetroTRK_BBA(void);
    void EnableMetroTRKInterrupts(void);
    DSError TRKInitializeTarget(void);
    u32 TRKTargetTranslate(u32 address);
    void __TRK_copy_vectors(void);
    void __TRK_reset(void);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_DOLPHIN_TRK_H
