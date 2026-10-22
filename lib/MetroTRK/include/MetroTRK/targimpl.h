#ifndef METROTRK_TARGIMPL_H
#define METROTRK_TARGIMPL_H


#include <MetroTRK/trk_bool.h>
#include <MetroTRK/dserror.h>
#include <MetroTRK/nubevent.h>
#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Default_PPC
    {
        u32 GPR[32];
        u32 PC;
        u32 LR;
        u32 CR;
        u32 CTR;
        u32 XER;
    } Default_PPC;

    typedef struct Float_PPC
    {
        u64 FPR[32];
        u64 FPSCR;
        u64 FPECR;
    } Float_PPC;

    typedef struct Extended1_PPC_6xx_7xx
    {
        u32 SR[16];
        u32 TBL;
        u32 TBU;
        u32 HID0;
        u32 HID1;
        u32 MSR;
        u32 PVR;
        u32 IBAT0U;
        u32 IBAT0L;
        u32 IBAT1U;
        u32 IBAT1L;
        u32 IBAT2U;
        u32 IBAT2L;
        u32 IBAT3U;
        u32 IBAT3L;
        u32 DBAT0U;
        u32 DBAT0L;
        u32 DBAT1U;
        u32 DBAT1L;
        u32 DBAT2U;
        u32 DBAT2L;
        u32 DBAT3U;
        u32 DBAT3L;
        u32 DMISS;
        u32 DCMP;
        u32 HASH1;
        u32 HASH2;
        u32 IMISS;
        u32 ICMP;
        u32 RPA;
        u32 SDR1;
        u32 DAR;
        u32 DSISR;
        u32 SPRG0;
        u32 SPRG1;
        u32 SPRG2;
        u32 SPRG3;
        u32 DEC;
        u32 IABR;
        u32 EAR;
        u32 DABR;
        u32 PMC1;
        u32 PMC2;
        u32 PMC3;
        u32 PMC4;
        u32 SIA;
        u32 MMCR0;
        u32 MMCR1;
        u32 THRM1;
        u32 THRM2;
        u32 THRM3;
        u32 ICTC;
        u32 L2CR;
        u32 UMMCR2;
        u32 UBAMR;
        u32 UMMCR0;
        u32 UPMC1;
        u32 UPMC2;
        u32 USIA;
        u32 UMMCR1;
        u32 UPMC3;
        u32 UPMC4;
        u32 USDA;
        u32 MMCR2;
        u32 BAMR;
        u32 SDA;
        u32 MSSCR0;
        u32 MSSCR1;
        u32 PIR;
        u32 exceptionID;
        u32 GQR[8];
        u32 HID_G;
        u32 WPAR;
        u32 DMA_U;
        u32 DMA_L;
    } Extended1_PPC_6xx_7xx;

    typedef struct Extended2_PPC_6xx_7xx
    {
        u32 PSR[32][2];
    } Extended2_PPC_6xx_7xx;

    typedef struct ProcessorState_PPC_6xx_7xx
    {
        Default_PPC Default;
        Float_PPC Float;
        Extended1_PPC_6xx_7xx Extended1;
        Extended2_PPC_6xx_7xx Extended2;
        u32 transport_handler_saved_ra;
    } ProcessorState_PPC_6xx_7xx;

    typedef ProcessorState_PPC_6xx_7xx ProcessorState_PPC;
    extern ProcessorState_PPC gTRKCPUState;
    extern ProcessorRestoreFlags_PPC gTRKRestoreFlags;

    typedef struct TRKState
    {
        u32 gpr[32];
        u32 lr;
        u32 ctr;
        u32 xer;
        u32 msr;
        u32 dar;
        u32 dsisr;
        BOOL isStopped;
        BOOL inputActivated;
        void* inputPendingPtr;
    } TRKState;

    extern TRKState gTRKState;

    u32 __TRK_get_MSR(void);
    void __TRK_set_MSR(u32 msr);
    void TRKInterruptHandler(u16 exceptionID);
    void TRKPostInterruptEvent(void);
    void TRKSwapAndGo(void);
    void TRKTargetSetInputPendingPtr(void* ptr);
    void TRKTargetSetStopped(bool stopped);
    bool TRKTargetStopped(void);
    DSError TRKTargetStop(void);
    DSError TRKTargetSingleStep(u32 count, bool stepOver);
    DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, bool stepOver);
    u32 TRKTargetGetPC(void);
    DSError TRKTargetSupportRequest(void);
    DSError TRKTargetAddStopInfo(MessageBuffer* buffer);
    void TRKTargetAddExceptionInfo(MessageBuffer* buffer);
    DSError TRKTargetInterrupt(NubEvent* event);
    DSError TRKTargetAccessMemory(void* data, u32 start, size_t* length, MemoryAccessOptions accessOptions, BOOL read);
    DSError TRKTargetReadInstruction(void* data, u32 start);
    DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister, MessageBuffer* buffer, size_t* registersLength, bool read);
    DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer* buffer, size_t* registersLength, s32 read);
    DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer* buffer, size_t* registersLength, s32 read);
    DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister, MessageBuffer* buffer, size_t* registersLength, bool read);
    DSError TRKPPCAccessSPR(void* srcDestPtr, u32 spr, bool read);
    DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, bool read);
    DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, bool read);
    DSError TRKPPCAccessSpecialReg(void* srcDestPtr, u32* instructionData, bool read);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_TARGIMPL_H
