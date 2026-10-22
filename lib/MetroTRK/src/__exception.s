.include "macros.inc"

.section .init, "ax"

.global gTRKInterruptVectorTable
gTRKInterruptVectorTable:
    .asciz "Metrowerks Target Resident Kernel for PowerPC"
    .balign 4
    .fill 0xD0

#############################################
# Interrupt vector slot 0x0000 is reserved. #
#############################################

# Slot 0x0100: System Reset Exception
    b __TRK_reset
    .fill 0xFC

# Slot 0x0200: Machine Check Exception
    mtspr 0x111, r2
    mfspr r2, 0x1a
    icbi 0, r2
    mfdar r2
    dcbi 0, r2
    mfspr r2, 0x111
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x200
    rfi
    .fill 0xB4

# Slot 0x0300: DSI Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x300
    rfi
    .fill 0xCC

# Slot 0x0400: ISI Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x400
    rfi
    .fill 0xCC

# Slot 0x0500: External Interrupt Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x500
    rfi
    .fill 0xCC

# Slot 0x0600: Alignment Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x600
    rfi
    .fill 0xCC

# Slot 0x0700: Program Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x700
    rfi
    .fill 0xCC

# Slot 0x0800: Floating Point Unavailable Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x800
    rfi
    .fill 0xCC

# Slot 0x0900: Decrementer Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x900
    rfi
    .fill 0xCC

######################################################
# Interrupt vector slots 0x0A00 & 0x0B00 are reserved.
    .fill 0x100
    .fill 0x100
######################################################

# Slot 0x0C00: System Call Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0xc00
    rfi
    .fill 0xCC

# Slot 0x0D00: Trace Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0xd00
    rfi
    .fill 0xCC

############################################################################
# Slot 0x0E00 is usually for the Floating Point Assist Exception Handler,  #
# however that exception is not implemented in the PPC 750CL architecture. #
############################################################################

# Slot 0x0F00: Performance Monitor Exception
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0xe00
    rfi
    .fill 0xCC

##################################################################################
# Interrupt vector slots 0x1000 through 0x1200 are not implemented in the 750CL. #
##################################################################################

# Slot 0x1300: Instruction Address Breakpoint Exception
    b 1f
    .fill 0x1C
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0xf20
    rfi
1:
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0xf00
    rfi
    .fill 0x78

# Slot 0x1400: System Management Interrupt Exception
    mtspr 0x111, r2
    mfcr r2
    mtspr 0x112, r2
    mfmsr r2
    andis. r2, r2, 2
    beq 1f
    mfmsr r2
    xoris r2, r2, 2
    sync 0
    mtmsr r2
    sync 0
    mtspr 0x111, r2
1:
    mfspr r2, 0x112
    mtcrf 0xff, r2
    mfspr r2, 0x111
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1000
    rfi
    .fill 0x90


##############################################################################
# Interrupt vector slots 0x1500 and 0x1600 are not implemented in the 750CL. #
##############################################################################

# Slot 0x1700: Thermal-Management Interrupt Exception
    mtspr 0x111, r2
    mfcr r2
    mtspr 0x112, r2
    mfmsr r2
    andis. r2, r2, 2
    beq 1f
    mfmsr r2
    xoris r2, r2, 2
    sync 0
    mtmsr r2
    sync 0
    mtspr 0x111, r2
1:
    mfspr r2, 0x112
    mtcrf 0xff, r2
    mfspr r2, 0x111
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1100
    rfi
    .fill 0x90

# Slot 0x1800(?)
    mtspr 0x111, r2
    mfcr r2
    mtspr 0x112, r2
    mfmsr r2
    andis. r2, r2, 2
    beq 1f
    mfmsr r2
    xoris r2, r2, 2
    sync 0
    mtmsr r2
    sync 0
    mtspr 0x111, r2
1:
    mfspr r2, 0x112
    mtcrf 0xff, r2
    mfspr r2, 0x111
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1200
    rfi
    .fill 0x90

# Slot 0x1900(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1300
    rfi
    .fill 0xCC

# Slot 0x1A00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1400
    rfi
    .fill 0x1CC

# Slot 0x1B00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1600
    rfi
    .fill 0xCC

# Slot 0x1C00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1700
    rfi
    .fill 0x4CC

# Slot 0x1D00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1c00
    rfi
    .fill 0xCC

# Slot 0x1E00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1d00
    rfi
    .fill 0xCC

# Slot 0x1F00(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1e00
    rfi
    .fill 0xCC

# Slot 0x2000(?)
    mtspr 0x111, r2
    mtspr 0x112, r3
    mtspr 0x113, r4
    mfspr r2, 0x1a
    mfspr r4, 0x1b
    mfmsr r3
    ori r3, r3, 0x30
    mtspr 0x1b, r3
    lis r3, TRKInterruptHandler@h
    ori r3, r3, TRKInterruptHandler@l
    mtspr 0x1a, r3
    li r3, 0x1f00
    rfi
.global gTRKInterruptVectorTableEnd
gTRKInterruptVectorTableEnd:
