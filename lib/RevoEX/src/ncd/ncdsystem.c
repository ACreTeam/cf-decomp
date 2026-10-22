#include <private/ios.h>
#include <private/ipc.h>
#include <revolution/ncd.h>
#include <revolution/os.h>

#include <string.h>

/*
 * Reconstructed from R4QE01. The translation unit name comes from the retail
 * assert literal at 0x80554DF0; no Wii decompilation project publishes an NCD
 * implementation, so the bodies here are recovered from the target rather than
 * adapted from a donor.
 */

#define NCD_IPC_HEAP_SIZE 0x1B60
#define NCD_MAX_PROFILE 3

static u32 ncdInitialized = 0;
static NCDConfig* ncdCommonBuffer;

OSMutex ncdMutex = {0};
s32 ncdCommonResult[8] ALIGN32;
IOSIoVector ncdCommonVector[4] ALIGN32;

const char* __NCDVersion = "<< RVL_SDK - NCD \trelease build: Dec 10 2007 10:02:22 (0x4199_60831) >>";

static void LockRight(void);
static NCDErr ExecConfigCommand(const char* name, NCDConfig* config, u32 command);

NCDErr NCDGetCurrentIfConfig(NCDIfConfig* ifConfig) {
    NCDErr err = 0;
    NCDConfig* config;
    NCDProfile* profile;
    s32 idx;

    if (ifConfig == (void*)NULL) {
        return -3;
    }

    LockRight();

    err = ExecConfigCommand("NCDGetCurrentIfConfig", 0, 3);
    if (err == 0) {
        config = ncdCommonBuffer;
        idx = ncdCommonResult[1];

        if (idx < 0 || idx >= NCD_MAX_PROFILE) {
            err = -7;
        } else {
            profile = &config->profiles[idx];

            if (profile->flags & 1) {
                ifConfig->selectedMedia = 2;
                memcpy(&ifConfig->netif, &profile->netif,
                       sizeof(NCDWiredProfile));
            } else {
                ifConfig->selectedMedia = 1;
                memcpy(&ifConfig->netif, &profile->netif,
                       sizeof(NCDWirelessProfile));
            }

            ifConfig->linkTimeout = config->linkTimeout;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDGetCurrentIpConfig(NCDIpConfig* ipConfig) {
    NCDErr err = 0;
    NCDConfig* config;
    s32 idx;

    if (ipConfig == (void*)NULL) {
        return -3;
    }

    LockRight();

    err = ExecConfigCommand("NCDGetCurrentIpConfig", 0, 3);
    if (err == 0) {
        config = ncdCommonBuffer;
        idx = ncdCommonResult[1];

        if (idx < 0 || idx >= NCD_MAX_PROFILE) {
            err = -7;
        } else {
            memcpy(&ipConfig->adjust, &config->profiles[idx].adjust,
                   sizeof(NCDIpAdjustProfile));

            if (config->profiles[idx].flags & 0x6) {
                ipConfig->useDhcp = TRUE;
                memcpy(&ipConfig->ip, &config->profiles[idx].ip,
                       sizeof(NCDIpProfile));
            } else {
                ipConfig->useDhcp = FALSE;
                memcpy(&ipConfig->ip, &config->profiles[idx].ip,
                       sizeof(NCDIpProfile));
            }

            if (config->profiles[idx].flags & 0x10) {
                ipConfig->useProxy = TRUE;
                memcpy(&ipConfig->proxy, &config->profiles[idx].proxy,
                       sizeof(NCDProxyProfile));
            } else {
                ipConfig->useProxy = FALSE;
                memset(&ipConfig->proxy, 0, sizeof(NCDProxyProfile));
            }
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDGetLinkStatus(void) {
    NCDErr err;
    s32 fd;

    if (OSGetCurrentThread() == NULL) {
        return -5;
    }

    LockRight();

    fd = IOS_Open("/dev/net/ncd/manage", IPC_ACCESS_NONE);
    if (fd < 0) {
        if (fd == IPC_RESULT_NOEXISTS) {
            err = -8;
        } else {
            err = -2;
        }
    } else {
        ncdCommonVector[0].base = (u8*)ncdCommonResult;
        ncdCommonVector[0].length = sizeof(ncdCommonResult);

        if (IOS_Ioctlv(fd, 7, 0, 1, ncdCommonVector) < 0) {
            err = -2;
        } else {
            err = ncdCommonResult[0];
            if (err == 0) {
                err = ncdCommonResult[1];
                err = err >= 0 ? err : -1;
            }
        }

        if (IOS_Close(fd) < 0) {
            err = -1;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDiGetWirelessMacAddress(u8* macAddr) {
    NCDErr err = 0;
    s32 fd;

    if (macAddr == (void*)NULL) {
        return -3;
    }
    if (OSGetCurrentThread() == NULL) {
        return -5;
    }

    LockRight();

    fd = IOS_Open("/dev/net/ncd/manage", IPC_ACCESS_NONE);
    if (fd < 0) {
        if (fd == IPC_RESULT_NOEXISTS) {
            err = -8;
        } else {
            err = -2;
        }
    } else {
        ncdCommonVector[0].base = (u8*)ncdCommonResult;
        ncdCommonVector[0].length = sizeof(ncdCommonResult);
        ncdCommonVector[1].base = (u8*)ncdCommonBuffer;
        ncdCommonVector[1].length = NCD_MAC_ADDRESS_LENGTH;

        if (IOS_Ioctlv(fd, 8, 0, 2, ncdCommonVector) < 0) {
            err = -2;
        } else {
            err = ncdCommonResult[0];
            if (err == 0) {
                memcpy(macAddr, ncdCommonBuffer, NCD_MAC_ADDRESS_LENGTH);
            }
        }

        if (IOS_Close(fd) < 0) {
            err = -1;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDLockWirelessDriver(void) {
    NCDErr err = 0;
    s32 fd;

    if (OSGetCurrentThread() == NULL) {
        return -5;
    }

    LockRight();

    fd = IOS_Open("/dev/net/ncd/manage", IPC_ACCESS_NONE);
    if (fd < 0) {
        if (fd == IPC_RESULT_NOEXISTS) {
            err = -8;
        } else {
            err = -2;
        }
    } else {
        ncdCommonVector[0].base = (u8*)ncdCommonResult;
        ncdCommonVector[0].length = sizeof(ncdCommonResult);

        if (IOS_Ioctlv(fd, 1, 0, 1, ncdCommonVector) < 0) {
            err = -2;
        } else {
            err = ncdCommonResult[0];
            if (err == 0) {
                err = ncdCommonResult[1];
                err = err > 0 ? err : -1;
            }
        }

        if (IOS_Close(fd) < 0) {
            err = -1;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDUnlockWirelessDriver(s32 id) {
    NCDErr err = 0;
    s32 fd;

    if (id <= 0) {
        return -3;
    }
    if (OSGetCurrentThread() == NULL) {
        return -5;
    }

    LockRight();

    fd = IOS_Open("/dev/net/ncd/manage", IPC_ACCESS_NONE);
    if (fd < 0) {
        if (fd == IPC_RESULT_NOEXISTS) {
            err = -8;
        } else {
            err = -2;
        }
    } else {
        ncdCommonVector[0].base = (u8*)ncdCommonBuffer;
        ncdCommonVector[0].length = 4;
        ncdCommonVector[1].base = (u8*)ncdCommonResult;
        ncdCommonVector[1].length = sizeof(ncdCommonResult);

        *(s32*)ncdCommonBuffer = id;

        if (IOS_Ioctlv(fd, 2, 1, 1, ncdCommonVector) < 0) {
            err = -2;
        } else {
            err = ncdCommonResult[0];
        }

        if (IOS_Close(fd) < 0) {
            err = -1;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

NCDErr NCDiGetEnabledConfigList(u32* list0, u32* list1, u32* list2) {
    NCDErr err;
    u32 mask0 = 0;
    u32 mask1 = 0;
    u32 mask2 = 0;
    u8 flags;
    u32 i;

    LockRight();

    err = ExecConfigCommand("NCDiGetEnabledConfigList", 0, 3);
    if (err == 0) {
        for (i = 0; i < NCD_MAX_PROFILE; i++) {
            NCDProfile* profile = &ncdCommonBuffer->profiles[i];

            flags = profile->flags;

            if (flags & 0x80) {
                if (flags & 1) {
                    mask0 |= 1 << i;
                } else {
                    if (profile->netif.wireless.configMethod != 1) {
                        mask1 |= 1 << i;
                    }
                    if (ncdCommonBuffer->profiles[i].netif.wireless.configMethod == 1) {
                        mask2 |= 1 << i;
                    }
                }
            }
        }
    }

    OSUnlockMutex(&ncdMutex);

    if (list0 != NULL) {
        *list0 = mask0;
    }
    if (list1 != NULL) {
        *list1 = mask1;
    }
    if (list2 != NULL) {
        *list2 = mask2;
    }

    return err;
}

static NCDErr ExecConfigCommand(const char* name, NCDConfig* config, u32 command) {
    NCDErr err = 0;
    s32 fd;

    if (OSGetCurrentThread() == NULL) {
        return -5;
    }

    LockRight();

    fd = IOS_Open("/dev/net/ncd/manage", IPC_ACCESS_NONE);
    if (fd < 0) {
        if (fd == IPC_RESULT_NOEXISTS) {
            err = -8;
        } else {
            err = -2;
        }
    } else {
        ncdCommonVector[0].base = (u8*)ncdCommonBuffer;
        ncdCommonVector[0].length = sizeof(NCDConfig);
        ncdCommonVector[1].base = (u8*)ncdCommonResult;
        ncdCommonVector[1].length = sizeof(ncdCommonResult);

        switch (command) {
        case 3:
        case 5:
            if (IOS_Ioctlv(fd, command, 0, 2, ncdCommonVector) < 0) {
                err = -2;
            } else {
                err = ncdCommonResult[0];
                if (err == 0 && config != (void*)NULL) {
                    memcpy(config, ncdCommonBuffer, sizeof(NCDConfig));
                }
            }
            break;
        case 4:
        case 6:
            if (config != (void*)NULL) {
                memcpy(ncdCommonBuffer, config, sizeof(NCDConfig));
            }
            if (IOS_Ioctlv(fd, command, 1, 1, ncdCommonVector) < 0) {
                err = -2;
            } else {
                err = ncdCommonResult[0];
            }
            break;
        }

        if (IOS_Close(fd) < 0) {
            err = -1;
        }
    }

    OSUnlockMutex(&ncdMutex);
    return err;
}

static void LockRight(void) {
    BOOL enabled;
    void* lo;

    enabled = OSDisableInterrupts();

    if (!(ncdInitialized & 1)) {
        OSRegisterVersion(__NCDVersion);
        OSInitMutex(&ncdMutex);

        lo = (void*)OSRoundUp32B(IPCGetBufferLo());

        if ((u32)IPCGetBufferHi() - (u32)lo < NCD_IPC_HEAP_SIZE) {
            OSPanic("ncdsystem.c", 1457,
                    "Could not reserve heap for NCD library from IPC arena");
        }

        IPCSetBufferLo((void*)((u32)lo + NCD_IPC_HEAP_SIZE));
        ncdCommonBuffer = (NCDConfig*)lo;

        memset(ncdCommonBuffer, 0, NCD_IPC_HEAP_SIZE);
        memset(ncdCommonResult, 0, sizeof(ncdCommonResult));
        memset(ncdCommonVector, 0, sizeof(ncdCommonVector));

        ncdInitialized |= 1;
    }

    OSRestoreInterrupts(enabled);
    OSLockMutex(&ncdMutex);
}
