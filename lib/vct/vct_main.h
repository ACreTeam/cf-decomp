#ifndef VCT_MAIN_H
#define VCT_MAIN_H

#include <revolution/os.h>
#include <revolution/types.h>

#include "vct.h"

typedef enum {
    LISTEN_MODE_CLIENT = 0,
    LISTEN_MODE_SERVER,
} VCTListenMode;

typedef struct VCTi_Globals {
    u8 myAID;
    u8 serverAID;
    VCTEventCallback callback;
    void* userData;
    VCTListenMode listenMode;
    VCTMode SSPMode;
    int autoResponse;
} VCTi_Globals;

typedef struct VCTResult {
    VCTEvent event;
    VCTSession* session;
} VCTResult;

extern VCTi_Globals vct;

int VCTi_HandleData(u8 aid, u8* buffer, int size, VCTResult* result);
BOOL VCTi_IsInitialized(void);

// debug.c
void VCTi_Report(const char* fmt, ...);

// hmac.c
void vct_add_hmac(void* digest, void* data, u32 size);
BOOL vct_check_hmac(const void* digest, void* data, u32 size);

#endif
