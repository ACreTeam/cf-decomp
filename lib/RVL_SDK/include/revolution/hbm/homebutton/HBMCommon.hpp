#ifndef HOMEBUTTON_COMMON_HPP
#define HOMEBUTTON_COMMON_HPP

#include "revolution/hbm/HBMCommon.h"

#include "revolution/mem/allocator.h"
#include "revolution/types.h"

extern "C" MEMAllocator* spAllocator;

void* HBMAllocMem(u32 length);
void HBMFreeMem(void* ptr);

#endif
