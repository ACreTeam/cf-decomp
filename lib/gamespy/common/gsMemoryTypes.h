#pragma once

#include <stddef.h>

typedef void* (*gsMallocCB)(size_t size);
typedef void (*gsFreeCB)(void* ptr);
typedef void* (*gsReallocCB)(void* ptr, size_t size);
typedef void* (*gsMemalignCB)(size_t boundary, size_t size);

// installed with gsiMemoryCallbacksSet
typedef struct {
  gsMallocCB mMalloc;
  gsFreeCB mFree;
  gsReallocCB mRealloc;
  gsMemalignCB mMemalign;
} GSIMemoryCallbacks;

extern GSIMemoryCallbacks memmanagercallbacks;
