#include "gsMemory.h"

GSIMemoryCallbacks memmanagercallbacks;

void gsiMemoryCallbacksSet(gsMallocCB p_malloc, gsFreeCB p_free,
                           gsReallocCB p_realloc, gsMemalignCB p_memalign) {
  memmanagercallbacks.mMalloc = p_malloc;
  memmanagercallbacks.mFree = p_free;
  memmanagercallbacks.mRealloc = p_realloc;
  memmanagercallbacks.mMemalign = p_memalign;
}

void* gsimalloc(size_t size) { return memmanagercallbacks.mMalloc(size); }

void* gsirealloc(void* ptr, size_t size) {
  return memmanagercallbacks.mRealloc(ptr, size);
}

void gsifree(void* ptr) {
  if (ptr == NULL)
    return;
  memmanagercallbacks.mFree(ptr);
}
