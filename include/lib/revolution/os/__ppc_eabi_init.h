#ifndef REVOLUTION_OS_PPC_EABI_INIT_H
#define REVOLUTION_OS_PPC_EABI_INIT_H

#include <decomp.h>
#include <revolution/types.h>
#include <size_t.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Linker-generated symbols.

// Debugger stack
extern u8 _db_stack_addr[];
extern u8 _db_stack_end[];

// Program arena
extern u8 __ArenaLo[];
extern u8 __ArenaHi[];

// Program stack
extern u8 _stack_addr[];
extern u8 _stack_end[];

// Section bounds emitted by the linker
extern u8 _f_init[];
extern u8 _e_init[];

// Small data bases
extern u8 _SDA_BASE_[];
extern u8 _SDA2_BASE_[];

#pragma section ".init"
__declspec(section ".init") void __init_hardware(void);
__declspec(section ".init") void __flush_cache(void* addr, size_t size);

typedef struct RomSection {
    void* phys;
    void* virt;
    size_t size;
} RomSection;

typedef struct BssSection {
    void* virt;
    size_t size;
} BssSection;

__declspec(section ".init") extern const RomSection _rom_copy_info[];
__declspec(section ".init") extern const BssSection _bss_init_info[];

void __init_user(void);
void __init_cpp(void);
void __fini_cpp(void);
void _ExitProcess(void);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_PPC_EABI_INIT_H
