#ifndef REVOLUTION_OS_CACHE_H
#define REVOLUTION_OS_CACHE_H

#include <revolution/os/OSError.h>
#include <revolution/types.h>


// Locked cache (LC) address region.
#define LC_BASE_PREFIX 0xE000
#define LC_BASE (LC_BASE_PREFIX << 16)

#ifdef __cplusplus
extern "C" {
#endif

#define LC_MAX_DMA_BLOCKS (128)
#define LC_MAX_DMA_BYTES (LC_MAX_DMA_BLOCKS * 32)

void DCEnable();

void DCInvalidateRange(void* addr, u32 len);

void DCFlushRange(void* addr, u32 len);
void DCFlushRangeNoSync(void* addr, u32 len);

void DCStoreRange(void* addr, u32 len);
void DCStoreRangeNoSync(void* addr, u32 len);

void DCZeroRange(void* addr, u32 len);

void ICEnable();
void ICFlashInvalidate();
void ICInvalidateRange(void* addr, u32 len);

void LCEnable();
void LCDisable();

void LCLoadBlocks(void* dest, void* src, u32 numBlocks);
void LCStoreBlocks(void* dest, void* src, u32 numBlocks);
u32 LCStoreData(void* dest, void* src, u32 numBytes);

u32 LCQueueLength();
void LCQueueWait(u32 length);

void DMAErrorHandler(OSError error, OSContext* context, ...);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_CACHE_H
