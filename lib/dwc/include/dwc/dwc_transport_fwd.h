#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*DWCUserRecvCallback)(unsigned char aid,
        unsigned char* buffer, int size);

    int DWC_SendUnreliable(unsigned char aid, const void* buffer, int size);
    int DWC_SetRecvBuffer(unsigned char aid, void* buffer, int size);
    int DWC_SetUserRecvCallback(DWCUserRecvCallback callback);

#ifdef __cplusplus
}
#endif
