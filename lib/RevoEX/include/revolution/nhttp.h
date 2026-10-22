#ifndef REVOLUTION_NHTTP_H
#define REVOLUTION_NHTTP_H

#include <revolution/types.h>

#include <revolution/nhttp/nhttp_err.h>

#define NHTTP_HDRRECVBUF_INILEN 1024
#define NHTTP_HDRRECVBUF_BLOCKSHIFT 9
#define NHTTP_HDRRECVBUF_BLOCKLEN (1 << NHTTP_HDRRECVBUF_BLOCKSHIFT)
#define NHTTP_HDRRECVBUF_BLOCKMASK (NHTTP_HDRRECVBUF_BLOCKLEN - 1)

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void* (*NHTTPAlloc)(u32 size, int align);
    typedef void (*NHTTPFree)(void* buf);
    typedef void (*NHTTPCleanupCallback)();

    typedef struct NHTTPRequestInfo NHTTPRequest;
    typedef struct NHTTPResponseInfo NHTTPResponse;
    typedef s32 NHTTPConnection;

    typedef enum NHTTPReqMethod
    {
        NHTTP_REQMETHOD_GET,
        NHTTP_REQMETHOD_POST,
        NHTTP_REQMETHOD_HEAD,
    } NHTTPReqMethod;

    typedef enum NHTTPEncodingType
    {
        NHTTP_ENCODING_TYPE_AUTO,
        NHTTP_ENCODING_TYPE_URL,
        NHTTP_ENCODING_TYPE_MULTIPART,
        NHTTP_ENCODING_TYPE_MAX,
    } NHTTPEncodingType;

    typedef void (*NHTTPReqCallback)(NHTTPErr error, NHTTPResponse* response,
        void* userParam);
    typedef void* (*NHTTPResponseCallback)(void** buffer, void* data,
        u32 contentLength, NHTTPAlloc alloc, NHTTPFree free, void* userParam);
    typedef void (*NHTTPResponseCleanup)(void* buffer, NHTTPFree free,
        void* userParam);

    s32 NHTTPStartup(NHTTPAlloc alloc, NHTTPFree free, u32 priority);
    void NHTTPCleanupAsync(NHTTPCleanupCallback callback);
    s32 NHTTPSSLGetError(void);
    NHTTPRequest* NHTTPCreateRequest(const char* url, NHTTPReqMethod method,
        void* buffer, u32 bufferSize, NHTTPReqCallback callback,
        void* userParam);
    NHTTPRequest* __NHTTPCreateRequestEx(const char* url, NHTTPReqMethod method,
        void* buffer, u32 bufferSize, NHTTPReqCallback callback,
        void* userParam, NHTTPResponseCallback responseCallback,
        NHTTPResponseCleanup cleanup);
    s32 NHTTPAddHeaderField(NHTTPRequest* request, const char* name,
        const char* value);
    s32 NHTTPAddPostDataAscii(NHTTPRequest* request, const char* name,
        const char* value);
    NHTTPConnection NHTTPSendRequestAsync(NHTTPRequest* request);
    s32 NHTTPCancelRequestAsync(NHTTPConnection connection);
    void NHTTPDestroyResponse(NHTTPResponse* response);
    s32 NHTTPGetBodyAll(NHTTPResponse* response, void** buffer);
    s32 NHTTPGetResultCode(NHTTPResponse* response);
    s32 NHTTPSetVerifyOption(NHTTPRequest* request, u32 option);
    s32 NHTTPSetProxy(NHTTPRequest* request, const char* server, u16 port,
        const char* username, const char* password);
    s32 NHTTPSetProxyDefault(NHTTPRequest* request);
    s32 NHTTPSetClientCertDefault(NHTTPRequest* request);
    s32 NHTTPSetRootCADefault(NHTTPRequest* request);
    s32 NHTTPAddPostDataBinary(NHTTPRequest* handle, const char* name, const char* value, u32 length);
    s32 NHTTPAddPostDataRaw(NHTTPRequest* handle, const void* data, u32 length);
    s32 NHTTPSetPostDataEncoding(NHTTPRequest* handle, NHTTPEncodingType encoding);
    s32 NHTTPGetHeaderField(NHTTPResponse* handle, const char* name, char** value);
    s32 NHTTPSetBuiltinRootCA(NHTTPRequest* handle, u32 index);
    s32 NHTTPSetBuiltinClientCert(NHTTPRequest* handle, u32 index);
    s32 NHTTPDisableVerifyOptionForDebug(NHTTPRequest* handle, u32 option);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_NHTTP_H
