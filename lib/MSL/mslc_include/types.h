#ifndef MSLC_TYPES_H
#define MSLC_TYPES_H

// Shim for the MSL C sources taken from mscharged-decomp: they expect the project's own <types.h>
#include <revolution/types.h>

#ifndef __cplusplus
typedef int bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#endif
