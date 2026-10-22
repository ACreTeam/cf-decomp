#ifndef PRIVATE_AX_DSP_CODE_H
#define PRIVATE_AX_DSP_CODE_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern u16 axDspInitVector;
extern u16 axDspResumeVector;

extern u8 axDspSlave[];
extern u16 axDspSlaveLength;

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_AX_DSP_CODE_H
