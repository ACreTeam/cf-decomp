#ifndef PRIVATE_OS_REBOOT_H
#define PRIVATE_OS_REBOOT_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void __OSReboot(u32 resetCode, u32 bootDol);
void OSGetSaveRegion(void** start, void** end);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_OS_REBOOT_H
