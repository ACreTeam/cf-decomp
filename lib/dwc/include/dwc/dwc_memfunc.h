#pragma once

#include <revolution/types.h>
#include <dwc/dwc_memfunc_fwd.h>

#ifdef __cplusplus
extern "C" {
#endif


void* DWC_Alloc(DWCAllocType name, u32 size);
void* DWC_AllocEx(DWCAllocType name, u32 size, int align);
void DWC_Free(DWCAllocType name, void* ptr, u32 size);
void* DWC_Realloc(DWCAllocType name, void* ptr, u32 oldsize, u32 newsize);
void* DWC_ReallocEx(DWCAllocType name, void* ptr, u32 oldsize, u32 newsize, int align);

#ifdef __cplusplus
}
#endif
