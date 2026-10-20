#ifndef DWC_SUPPORT_REVOLUTION_NHTTP_H
#define DWC_SUPPORT_REVOLUTION_NHTTP_H

#include <revolution/types.h>
#include <revolution/nhttp/nhttp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*NHTTPAlloc)(u32 size, int align);
typedef void (*NHTTPFree)(void *buf);
typedef void (*NHTTPCleanupCallback)();

typedef struct NHTTPRequestInfo NHTTPRequest;
typedef struct NHTTPResponseInfo NHTTPResponse;
typedef s32 NHTTPConnection;

typedef enum NHTTPReqMethod {
    NHTTP_REQMETHOD_GET,
    NHTTP_REQMETHOD_POST,
    NHTTP_REQMETHOD_HEAD,
} NHTTPReqMethod;

typedef void (*NHTTPReqCallback)(NHTTPErr error, NHTTPResponse *response, void *userParam);

s32 NHTTPStartup(NHTTPAlloc alloc, NHTTPFree free, u32 priority);
void NHTTPCleanupAsync(NHTTPCleanupCallback callback);
s32 NHTTPSSLGetError(void);
NHTTPRequest *NHTTPCreateRequest(const char *url, NHTTPReqMethod method, void *buffer, u32 bufferSize,
                                 NHTTPReqCallback callback, void *userParam);
s32 NHTTPAddHeaderField(NHTTPRequest *request, const char *name, const char *value);
s32 NHTTPAddPostDataAscii(NHTTPRequest *request, const char *name, const char *value);
NHTTPConnection NHTTPSendRequestAsync(NHTTPRequest *request);
s32 NHTTPCancelRequestAsync(NHTTPConnection connection);
void NHTTPDestroyResponse(NHTTPResponse *response);
s32 NHTTPGetBodyAll(NHTTPResponse *response, void **buffer);
s32 NHTTPGetResultCode(NHTTPResponse *response);
s32 NHTTPSetVerifyOption(NHTTPRequest *request, u32 option);
s32 NHTTPSetProxyDefault(NHTTPRequest *request);
s32 NHTTPSetClientCertDefault(NHTTPRequest *request);
s32 NHTTPSetRootCADefault(NHTTPRequest *request);

#ifdef __cplusplus
}
#endif

#endif
