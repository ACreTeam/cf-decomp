#pragma once

#include <dwc/dwc_error_fwd.h>

// Matching declarations that do not require SDK integer typedefs.
#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*DWCMatchedCallback)(DWCError error, int cancel,
        void* param);
    typedef void (*DWCMatchedSCCallback)(DWCError error, int cancel,
        int self, int isServer, int index, void* param);
    typedef void (*DWCNewClientCallback)(int index, void* param);

    typedef int (*DWCEvalPlayerCallback)(int index, void* param);
    typedef void (*DWCStopSCCallback)(void* param);

    typedef enum DWCMatchOptType
    {
        DWC_MATCH_OPTION_MIN_COMPLETE,
        DWC_MATCH_OPTION_SC_CONNECT_BLOCK,
        DWC_MATCH_OPTION_NUM
    } DWCMatchOptType;

    typedef struct DWCMatchOptMinComplete
    {
        unsigned char valid;
        unsigned char minEntry;
        unsigned char pad[2];
        unsigned long timeout;
    } DWCMatchOptMinComplete;

    int DWC_CancelMatching(void);
    int DWC_CancelMatchingAsync(void);
    int DWC_IsValidCancelMatching(void);
    unsigned char DWC_AddMatchKeyInt(unsigned char keyID,
        const char* keyString, const int* valueSrc);

    int DWC_StopSCMatchingAsync(DWCStopSCCallback callback, void* param);
    int DWC_GetMatchIntValue(int index, const char* keyString, int idefault);
    int DWC_SetMatchingOption(DWCMatchOptType opttype, const void* optval, int optlen);

#ifdef __cplusplus
}
#endif
