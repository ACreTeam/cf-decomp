#ifndef REVOLUTION_NCD_H
#define REVOLUTION_NCD_H

#include <revolution/os/OSTime.h>

#include <revolution/ncd/NCDTypes.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

NCDErr NCDReadConfig(NCDConfig* config);
NCDErr NCDWriteConfig(const NCDConfig* config);

NCDErr NCDSetIpConfig(NCDIpConfig* ipConfig);
NCDErr NCDSetIfConfig(NCDIfConfig* ifConfig);

NCDErr NCDGetCurrentIpConfig(NCDIpConfig* ipConfig);
NCDErr NCDGetCurrentIfConfig(NCDIfConfig* ifConfig);

NCDErr NCDGetLinkStatus();
BOOL NCDIsInterfaceDecided();

NCDErr NCDGetWirelessMacAddress(u8* macAddr);
NCDErr NCDiGetWirelessMacAddress(u8* macAddr);
NCDErr NCDiGetEnabledConfigList(u32* list0, u32* list1, u32* list2);

NCDErr NCDLockWirelessDriver();
NCDErr NCDUnlockWirelessDriver(s32 id);

s32 NCDRestoreConfig();

void NCDSleep(OSTime tick);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // REVOLUTION_NCD_H
