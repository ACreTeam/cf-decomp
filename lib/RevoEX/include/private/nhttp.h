#ifndef PRIVATE_NHTTP_H
#define PRIVATE_NHTTP_H

#include <revolution/nhttp.h>
#include <revolution/ncd.h>
#include <revolution/os.h>
#include <revolution/ssl.h>

#define STR_POSTBOUND "--t9Sf4yfjf1RtvDu3AA"
#define LEN_POSTBOUND 18
#define LEN_URLBUF 256
#define MIN(AA, BB) (((AA) > (BB)) ? (BB) : (AA))

typedef struct NHTTPBgnEndInfo NHTTPBgnEndInfo;
typedef struct NHTTPRequestInfo NHTTPRequestInfo;
typedef struct NHTTPConnectionInfo NHTTPConnectionInfo;

struct NHTTPBgnEndInfo
{
    NCDIpConfig ipConfig;
    NHTTPAlloc alloc;
    NHTTPFree free;
    BOOL started;
    s32 socket;
    s32 sslError;
    NHTTPErr error;
    u32 _unk7DC;
    void* threadStack;
    void (*sslCallback)(SSLId sslId, void* param);
};

typedef enum NHTTPConnectionEvent
{
    NHTTP_EVENT_NONE,
    NHTTP_EVENT_POST_SEND,
    NHTTP_EVENT_BODY_RECV_FULL,
    NHTTP_EVENT_BODY_RECV_DONE,
    NHTTP_EVENT_COMPLETE,
    NHTTP_CALLBACK_EVENT_MAX,
} NHTTPConnectionEvent;

typedef s32 (*NHTTPConnectionCallback)(NHTTPConnectionInfo* connection,
    NHTTPConnectionEvent event, void* arg);

typedef struct NHTTPConnectionCallbackParam
{
    void* value;
    u32 _unk4;
    u32 _unk8;
    u32 _unkC;
} NHTTPConnectionCallbackParam;

typedef struct NHTTPPostSendArg
{
    const char* label;
    char* buf;
    u32 size;
    u32 offset;
} NHTTPPostSendArg;

typedef struct NHTTPBodyBufArg
{
    char* buf;
    u32 size;
    u32 offset;
} NHTTPBodyBufArg;

typedef struct NHTTPHeader
{
    struct NHTTPHeader* next;
    struct NHTTPHeader* prev;
    const char* name;
    const char* value;
    u32 length;
    u32 _unk14;
} NHTTPHeader;

typedef struct NHTTPReqQueue
{
    struct NHTTPReqQueue* next;
    struct NHTTPReqQueue* prev;
    s32 requestId;
    NHTTPRequestInfo* request;
    s32 _unk10;
} NHTTPReqQueue;

typedef struct NHTTPListInfo
{
    NHTTPReqQueue* reqQueue;
    s32 nextRequestId;
} NHTTPListInfo;

typedef struct NHTTPReqInfo
{
    NHTTPReqQueue* reqQueue;
} NHTTPReqInfo;

typedef struct NHTTPi_HDRBUFLIST
{
    struct NHTTPi_HDRBUFLIST* next_p;
    u8 block[NHTTP_HDRRECVBUF_BLOCKLEN];
} NHTTPi_HDRBUFLIST;

typedef struct NHTTPResponseInfo
{
    volatile s32 headerLen;
    volatile s32 bodyLen;
    volatile s32 totalBodyLen;
    volatile s32 contentLength;
    s32 isSuccess;
    s32 isHeaderParse;
    s32 httpStatus;
    u32 recvBufLen;
    char* allHeader_p;
    char* foundHeader_p;
    char* recvBuf_p;
    NHTTPResponseCallback bufFull;
    NHTTPResponseCleanup freeBuf;
    NHTTPi_HDRBUFLIST* hdrBufBlock_p;
    u8 hdrBufFirst[NHTTP_HDRRECVBUF_INILEN];
    void* param_p;
} NHTTPResponseInfo;

struct NHTTPRequestInfo
{
    BOOL cancel;
    s32 state;
    BOOL secure;
    BOOL proxyEnabled;
    BOOL isRawData;
    s32 hostEnd;
    s32 pathStart;
    s32 method;
    u32 port;
    char* url;
    char* host;
    NHTTPResponseInfo* response;
    NHTTPHeader* headers;
    NHTTPHeader* postData;
    char multipartBoundary[LEN_POSTBOUND + 2];
    char authorization[0x5C];
    s32 authorizationLength;
    SSLId sslId;
    const char* clientCertData;
    u32 clientCertSize;
    const char* privateKeyData;
    u32 privateKeySize;
    const char* rootCAData;
    u32 rootCASize;
    BOOL clientCertDefault;
    u32 verifyOption;
    NHTTPEncodingType encodingType;
    void* sslCallbackParam;
    u32 builtinRootCA;
    u32 builtinClientCert;
    char proxyServer[0x100];
    u32 proxyPort;
    char proxyAuthorization[0x5C];
    s32 proxyAuthorizationLength;
    u32 recvBufferSize;
    const void* postBuffer;
    u32 postBufferSize;
    void* _unk244;
};

struct NHTTPConnectionInfo
{
    BOOL started;
    s32 state;
    s32 sslError;
    BOOL inProgress;
    NHTTPRequestInfo* request;
    NHTTPResponseInfo* response;
    s32 requestId;
    NHTTPConnectionCallback callback;
    NHTTPConnectionInfo* next;
    u32 postSendBuf;
    u32 postSendSize;
    u32 virtualContentLength;
    NHTTPReqCallback requestCallback;
    char recvBuffer[0x8000] ALIGN32;
    u32 recvBufferOffset;
    u32 recvBufferRemaining;
};

typedef struct NHTTPSysInfo NHTTPSysInfo;

typedef struct NHTTPMutexInfo
{
    OSMutex mutex;
    BOOL initialized;
} NHTTPMutexInfo;

typedef struct NHTTPThreadInfo
{
    OSMessageQueue messageQueue;
    OSMessage messages[3];
    OSThread thread;
    BOOL isCreateCommThreadMessageQueue;
    u8 _unk34C[0x14];
    char commBuf[0x100];
} NHTTPThreadInfo;

typedef struct NHTTPThreadData
{
    /* 0x000 */ s32 id;
    /* 0x004 */ char preHostname[0x100];
    /* 0x104 */ char tmpBodyBuf[0x200];
    /* 0x304 */ char tmpHeaderBuf[0xE];
    /* 0x312 */ u16 padding;
    /* 0x314 */ u32 hostaddr;
    /* 0x318 */ u32 preHostaddr;
    /* 0x31C */ s32 port;
    /* 0x320 */ s32 prePort;
    /* 0x324 */ s32 sendBufwp;
    /* 0x328 */ s32 recvBufwp;
    /* 0x32C */ s32 contentLength;
    /* 0x330 */ NHTTPErr result;
    /* 0x334 */ BOOL restartConnect;
    /* 0x338 */ BOOL isKeepAlive;
    /* 0x33C */ BOOL isChunked;
} NHTTPThreadData;

void* NHTTPi_alloc(u32 size, int align);
void NHTTPi_SetError(NHTTPBgnEndInfo* info, NHTTPErr error);
s32 NHTTPi_strlen(const char* string);
s32 NHTTPi_compareToken(const char* string, const char* token);
int NHTTPi_memfind(const char* data, int size,
    const char* pattern, int patternSize);
extern const char NHTTPi_strMultipartBound[];
NHTTPHeader* NHTTPi_getHdrFromList(NHTTPHeader** list);
BOOL NHTTP_AddHeaderField(NHTTPRequestInfo* request, NHTTPBgnEndInfo* info,
    const char* name, const char* value);
BOOL NHTTP_AddPostDataAscii(NHTTPRequestInfo* request, NHTTPBgnEndInfo* info,
    const char* label, const char* value);
BOOL NHTTP_AddPostDataBinary(NHTTPRequestInfo* request, NHTTPBgnEndInfo* info,
    const char* label, const char* value, u32 length);
BOOL NHTTP_AddPostDataRaw(NHTTPRequestInfo* request, const void* data,
    u32 length);
BOOL NHTTP_SetPostDataEncoding(NHTTPRequestInfo* request,
    NHTTPEncodingType encoding);
s32 NHTTP_GetHeaderField(NHTTPResponseInfo* response, const char* label,
    char** value);

void NHTTPi_InitBgnEndInfo(NHTTPBgnEndInfo* info);
void NHTTPi_InitListInfo(NHTTPListInfo* info);
void NHTTPi_InitRequestInfo(NHTTPReqInfo* info);
void NHTTPi_InitThreadInfo(NHTTPThreadInfo* info);

void NHTTPi_InitMutexInfo(NHTTPMutexInfo* info);
void NHTTPi_lockReqList(NHTTPMutexInfo* info);
void NHTTPi_unlockReqList(NHTTPMutexInfo* info);

NHTTPConnectionInfo* NHTTPi_ControlConnectionList(void* mutexInfo,
    void* handle, u32 mode);
s32 NHTTPi_CommitConnectionList(void* mutexInfo,
    NHTTPConnectionInfo* connection);
s32 NHTTPi_OmitConnectionList(void* mutexInfo,
    NHTTPConnectionInfo* connection);
NHTTPRequestInfo* NHTTPi_Connection2Request(void* mutexInfo,
    NHTTPConnectionInfo* connection);
NHTTPResponseInfo* NHTTPi_Connection2Response(void* mutexInfo,
    NHTTPConnectionInfo* connection);
NHTTPConnectionInfo* NHTTPi_Request2Connection(void* mutexInfo,
    NHTTPRequestInfo* request);
NHTTPConnectionInfo* NHTTPi_Response2Connection(void* mutexInfo,
    NHTTPResponseInfo* response);
NHTTPConnectionInfo* NHTTPi_GetConnection(void* mutexInfo,
    NHTTPConnectionInfo* connection);
NHTTPRequestInfo* NHTTPi_GetRequest(void* mutexInfo,
    NHTTPRequestInfo* request);
NHTTPResponseInfo* NHTTPi_GetResponse(void* mutexInfo,
    NHTTPResponseInfo* response);
s32 NHTTPi_GetConnectionListLength(void);
s32 NHTTPi_PostSendCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection, const char* label, u32 arg);
void NHTTPi_BufferFullCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection);
void NHTTPi_ReceivedCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection);
void NHTTPi_CompleteCallback(void* mutexInfo,
    NHTTPConnectionInfo* connection);
NHTTPSysInfo* NHTTPi_GetSystemInfoP(void);
NHTTPBgnEndInfo* NHTTPi_GetBgnEndInfoP(NHTTPSysInfo* sysInfo_p);
NHTTPListInfo* NHTTPi_GetListInfoP(NHTTPSysInfo* sysInfo_p);
NHTTPReqInfo* NHTTPi_GetReqInfoP(NHTTPSysInfo* sysInfo_p);
void* NHTTPi_GetThreadInfoP(NHTTPSysInfo* sysInfo_p);
void* NHTTPi_GetMutexInfoP(NHTTPSysInfo* sysInfo_p);
void NHTTPi_SetVirtualContentLength(NHTTPConnectionInfo* connection,
    u32 length);
u32 NHTTPi_GetVirtualContentLength(NHTTPConnectionInfo* connection);

void NHTTPi_InitConnectionList(void);
s32 NHTTPi_checkKeepAlive(s32 socket);
int NHTTPi_SocRecv(void* mutexInfo, NHTTPRequestInfo* request, int socket,
    char* buffer, int length, int flags);
s32 NHTTPi_RecvBuf(void* mutexInfo, NHTTPRequestInfo* request, s32 socket,
    s32 offset, s32 flags);
int NHTTPi_RecvBufN(void* mutexInfo, NHTTPRequestInfo* request, int socket,
    int offset, int length, int flags);
void NHTTPi_WaitForCompletion(NHTTPConnectionInfo* connection);
void NHTTPi_NotifyCompletion(NHTTPConnectionInfo* connection);

// NHTTP_bgnend.c
void NHTTPi_free(void* buf);
void NHTTPi_SetSSLError(NHTTPBgnEndInfo* info, s32 error);
s32 NHTTPi_GetSSLError(NHTTPBgnEndInfo* info);
BOOL NHTTPi_Startup(void* systemInfo, NHTTPAlloc alloc, NHTTPFree free, u32 priority);
void NHTTPi_CleanupAsync(void* systemInfo, NHTTPCleanupCallback callback);

// NHTTP_list.c
s32 NHTTPi_setReqQueue(NHTTPListInfo* info, NHTTPRequestInfo* request);
BOOL NHTTPi_freeReqQueue(NHTTPListInfo* info, void* mutexInfo, s32 requestId);
void NHTTPi_allFreeReqQueue(NHTTPListInfo* info, void* mutexInfo);
NHTTPReqQueue* NHTTPi_getReqFromReqQueue(NHTTPReqQueue** queue);

// NHTTP_os_RVL.c
void NHTTPi_initLockReqList(NHTTPMutexInfo* info);
void NHTTPi_exitLockReqList(void);
BOOL NHTTPi_createCommThread(NHTTPThreadInfo* info, s32 priority, void* stack);
void NHTTPi_destroyCommThread(NHTTPThreadInfo* info, NHTTPBgnEndInfo* bgnEndInfo);
void NHTTPi_idleCommThread(NHTTPThreadInfo* info);
void NHTTPi_kickCommThread(NHTTPThreadInfo* info);
void NHTTPi_CheckCurrentThread(NHTTPThreadInfo* info, BOOL notCommThread);
void* NHTTPi_CommThreadProc(void* argument);

// NHTTP_recvbuf.c
s32 NHTTPi_findNextLineHdrRecvBuf(const NHTTPResponseInfo* recvBuf, s32 start, s32 end, s32* separator, s32* lineBreakLength);
s32 NHTTPi_skipSpaceHdrRecvBuf(const NHTTPResponseInfo* recvBuf, s32 start, s32 end);
s32 NHTTPi_compareTokenN_HdrRecvBuf(const NHTTPResponseInfo* recvBuf, s32 start, s32 end, const char* token, s8 terminal);
int NHTTPi_loadFromHdrRecvBuf(const NHTTPResponseInfo* res_p, char* dst_p, int pos, int len);
int NHTTPi_isRecvBufFull(const NHTTPResponseInfo* res_p, const int pos);

// NHTTP_request.c
NHTTPRequestInfo* NHTTP_CreateRequest(NHTTPBgnEndInfo* bgnEndInfo_p, const char* url_p, s32 method, void* buf_p, u32 len, void* param_p, NHTTPResponseCallback bufFull, NHTTPResponseCleanup freeBuf);
void NHTTP_DestroyRequest(void* systemInfo, NHTTPRequestInfo* request);
BOOL NHTTPi_destroyRequestObject(void* mutexInfo, NHTTPRequestInfo* request);
s32 NHTTP_SendRequestAsync(void* systemInfo, NHTTPRequestInfo* request);
BOOL NHTTP_CancelRequestAsync(void* sysInfo_p, s32 id);
void NHTTPi_cancelAllRequests(void* sysInfo_p);

// NHTTP_response.c
void NHTTP_DestroyResponse(void* mutexInfo_p, NHTTPResponseInfo* res_p);
s32 NHTTPi_getHeaderValue(const NHTTPResponseInfo* res_p, const char* label_p, s32* pos_p);

// NHTTP_socket_RVL.c
s32 NHTTPi_SocOpen(NHTTPRequestInfo* request);
s32 NHTTPi_SocClose(void* mutexInfo, NHTTPRequestInfo* request, s32 socket);
s32 NHTTPi_SocConnect(NHTTPBgnEndInfo* info, void* mutexInfo, NHTTPRequestInfo* request, s32 socket, u32 address, u32 port);
s32 NHTTPi_SocSSLConnect(NHTTPBgnEndInfo* bgnEndInfo_p, void* mutexInfo_p, NHTTPRequestInfo* req_p, const s32 socket);
s32 NHTTPi_SocRecv_sub(NHTTPConnectionInfo* connection, s32 socket, char* buffer, s32 length, s32 flags);
s32 NHTTPi_SocSend_sub(s32 socket, const char* i_buf, u32 i_bufSize, s32 flags);
int NHTTPi_SocSend(const NHTTPRequestInfo* req_p, const int socket, const char* buf_p, const int len, const int flags);
void NHTTPi_SocCancel(void* mutexInfo, NHTTPRequestInfo* request, s32 socket);
u32 NHTTPi_resolveHostname(NHTTPRequestInfo* request, const char* hostname);

// NHTTP_stdlib_RVL.c
void* NHTTPi_memcpy(void* destination, const void* source, u32 size);
s32 NHTTPi_strcmp(const char* left, const char* right);
void* NHTTPi_memclr(void* destination, u32 size);
s32 NHTTPi_strnicmp(const char* left, const char* right, s32 size);
s32 NHTTPi_getUrlEncodedSize(const char* string);
s32 NHTTPi_getUrlEncodedSize2(const char* string, s32 length);
s32 NHTTPi_encodeUrlChar(char* destination, char c);
s32 NHTTPi_strToHex(const char* string, s32 length);
s32 NHTTPi_strToInt(const char* string, s32 length);
s32 NHTTPi_intToStr(char* p_p, u32 n);
s32 NHTTPi_strtonum(const char* string, s32 length);
s32 NHTTPi_Base64Encode(char* destination, const char* source);

// NHTTP_thread.c
void NHTTPi_IsCreateCommThreadMessageQueueOn(NHTTPThreadInfo* info);
BOOL NHTTPi_IsCreateCommThreadMessageQueue(NHTTPThreadInfo* info);
s32 NHTTPi_CheckHeaderEnd(char* buffer, s32 position);
s32 NHTTPi_SaveBuf(NHTTPRequestInfo* request, char* buffer, s32 socket, s32* bufferedBytes, const char* data, s32 length);
BOOL NHTTPi_GetPostContentlength(void* mutexInfo, NHTTPRequestInfo* request, const char* label, s32* contentLength, s32 format);
s32 NHTTPi_SendPostData(void* mutexInfo, NHTTPRequestInfo* request, char* buffer, const char* label, s32 socket, s32* bufferedBytes, s32 format);
BOOL NHTTPi_BufFull(void* mutexInfo, NHTTPResponseInfo* response);
s32 NHTTPi_SendProxyConnectMethod(NHTTPThreadData* threadData_p);
BOOL NHTTPi_RecvProxyConnectHeader(NHTTPThreadData* threadData_p);
s32 NHTTPi_SendProcPostDataRaw(NHTTPThreadData* threadData_p);
s32 NHTTPi_SendProcPostDataBinary(NHTTPThreadData* threadData_p);
s32 NHTTPi_SendProcPostDataAscii(NHTTPThreadData* threadData_p);
void NHTTPi_ThreadReqEnd(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadExecReqQueue(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadHostAddrProc(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadConnectProc(NHTTPThreadData* threadData_p);
s32 NHTTPi_ThreadProxyProc(NHTTPThreadData* threadData_p);
s32 NHTTPi_ThreadSendProc(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadRecvHeaderProc(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadParseHeaderProc(NHTTPThreadData* threadData_p);
BOOL NHTTPi_ThreadRecvBodyProc(NHTTPThreadData* threadData_p);
void NHTTPi_CommThreadProcMain(void* argument);

// d_nhttp.c
s32 NHTTPi_TemplateConnectionCallback(NHTTPConnectionInfo* handle, NHTTPConnectionEvent event, void* arg_p);

// d_nhttp_private.c
NHTTPConnectionInfo* __NHTTPCreateConnection(const char* url_p, s32 method, void* buf_p, u32 len, NHTTPConnectionCallback callback, void* userParam_p);
s32 NHTTPDeleteConnection(NHTTPConnectionInfo* handle);
s32 NHTTPStartConnection(NHTTPConnectionInfo* handle);
s32 NHTTPGetBodyBuffer(NHTTPConnectionInfo* handle, void** buffer, u32* bufferSize);
void* NHTTPGetUserParam(NHTTPConnectionInfo* handle);
s32 NHTTPGetConnectionError(NHTTPConnectionInfo* handle);

#endif // PRIVATE_NHTTP_H
