#pragma once

#include <dwc/dwc_memfunc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DWCi_MEMFUNC_SIGNATURE 'DWCM'

typedef struct {
  u32 magic;
  u32 size;
  u32 _[6];
} DWCi_AllocateHeader;

DWCi_AllocateHeader* DWCi_GetAllocateHeader(void* block);
void* DWCi_SetAllocateHeader(DWCi_AllocateHeader* blockHeader, u32 size);
u32 DWCi_GetAllocateSize(void* block);

void DWCi_SetMemFunc(DWCAllocEx allocator, DWCFreeEx freer);

void* DWCi_GsMalloc(u32 size);
void* DWCi_GsRealloc(void* block, u32 size);
void DWCi_GsFree(void* block);
void* DWCi_GsMemalign(u32 align, u32 size);

#ifdef __cplusplus
}
#endif
