#include <game/game/d_ds_dl.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net_wus.hpp>
#include <revolution/NHTTP.h>
#include <revolution/OS.h>
#include <revolution/SO.h>
#include <revolution/OS/OSTime.h>
#include <stdio.h>
#include <string.h>


// Narrow interfaces to the unscaffolded socket and HTTP SDK libraries.
extern "C" {
void *__NHTTPCreateConnection(const char *, int, void *, u32, void *, void *);
int fn_8041B7BC(void *, int);
int fn_8041B824(void *, int);
int NHTTPDeleteConnection(void *);
int NHTTPStartConnection(void *);
int NHTTPGetConnectionError(void *);
int NHTTPGetHeaderField(void *, const char *, char **);
u32 NHTTPGetBodyBuffer(void *, char **, u32 *);
}

namespace dNetWus {
static const char *sNotifyUrl = "https://wus.wapp.wii.com/notify";
static const char *sInquiryUrl = "https://wus.wapp.wii.com/inquiry";
static const char *sAppVersionHeader = "X-APPVER";
static const char *sAppVersion = "001000";
static const char *sGameCodeHeader = "X-GAMECD";
static const char *sUserField = "uid";
static const char *sUser = "RVL-RUDJ";
static const char *sPasswordField = "pwd";
static const char *sPassword = "2C5NHqCv";
static const char *sWiiIdField = "wiino";
static const char *sActionField = "action";
static const char *sActions[] = {"U", "D"};
static const char *sCheckIdsField = "chkno";
static const char *sValidTermField = "vterm";
static const char *sResultHeader = "X-RESULT";
static const char *sNotifyResult = "001";
static const char *sInquiryResult = "011";
static dDsDl::Mutex_c sHeapMutex;
static void *sResponseBuffer;
static int sInitState;

// 800EB06C
void *alloc(u32 size, int align) {
    OSLockMutex(&sHeapMutex.mMutex);
    void *p = dHeap::netSoHeap_p->alloc(size, align);
    OSUnlockMutex(&sHeapMutex.mMutex);
    return p;
}

// 800EB0E0
void free(void *p) {
    if (p) {
        OSLockMutex(&sHeapMutex.mMutex);
        dHeap::netSoHeap_p->free(p);
        OSUnlockMutex(&sHeapMutex.mMutex);
    }
}

// 800EB144
void *allocSocket(int, u32 size) {
    return alloc(size, 32);
}

// 800EB150
void freeSocket(int, void *p) {
    free(p);
}

// 800EB158
dNetWus_c::dNetWus_c() : mRequest(NULL), mError(0), mStatus(HTTP_STATUS_OK) {
}

// 800EB170
dNetWus_c::~dNetWus_c() {
}

// 800EB1B0
BOOL dNetWus_c::open() {
    if (sInitState < INIT_SOCKET) {
        SOLibraryConfig callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.alloc = allocSocket;
        callbacks.free = freeSocket;
        int error = SOInit(&callbacks);
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_SOCKET;
    }
    if (sInitState < INIT_SOCKET_STARTED) {
        int error = SOStartup();
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_SOCKET_STARTED;
    }
    if (!sResponseBuffer) {
        sResponseBuffer = dHeap::netWc24Heap_p->alloc(RESPONSE_BUFFER_SIZE, 32);
        if (sResponseBuffer == NULL) {
            return FALSE;
        }
    }
    if (sInitState < INIT_HTTP) {
        if (NHTTPStartup(alloc, free, HTTP_THREAD_PRIORITY) != 0) {
            return FALSE;
        }
        sInitState = INIT_HTTP;
    }
    return TRUE;
}

// 800EB2D4
BOOL dNetWus_c::close() {
    if (sInitState >= INIT_HTTP) {
        NHTTPCleanupAsync(0);
        sInitState = INIT_SOCKET_STARTED;
    }
    if (sResponseBuffer) {
        dHeap::netWc24Heap_p->free(sResponseBuffer);
        sResponseBuffer = NULL;
    }
    if (sInitState >= INIT_SOCKET_STARTED) {
        int error = SOCleanup();
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_SOCKET;
    }
    if (sInitState >= INIT_SOCKET) {
        int error = SOFinish();
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_NONE;
    }
    return TRUE;
}

// 800EB398
BOOL dNetWus_c::createRequest(const char *url) {
    mRequest = __NHTTPCreateConnection(url, HTTP_POST, sResponseBuffer,
                           RESPONSE_BUFFER_SIZE, NULL, NULL);
    if (mRequest == NULL) {
        return FALSE;
    }
    if (fn_8041B7BC(mRequest, 0) == 0 && fn_8041B824(mRequest, 0) == 0 &&
        NHTTPSetVerifyOption(mRequest, 0) == 0) {
        return TRUE;
    }
    destroyRequest();
    return FALSE;
}

// 800EB43C
BOOL dNetWus_c::destroyRequest() {
    if (mRequest == NULL) {
        return TRUE;
    }
    mError = NHTTPDeleteConnection(mRequest);
    if (mError == 0) {
        mRequest = NULL;
        return TRUE;
    }
    return FALSE;
}

// 800EB4A0
BOOL dNetWus_c::sendRequest() {
    mError = NHTTPStartConnection(mRequest);
    if (mError == 0) {
        u32 ticksPerMsec;
        BOOL timeout = FALSE;
        u32 start = OSGetTick();
        while (NHTTPGetConnectionError(mRequest) == HTTP_REQUEST_PENDING) {
            ticksPerMsec = OS_TIME_SPEED / 1000;
            if ((OSGetTick() - start) / ticksPerMsec >= REQUEST_TIMEOUT_MSEC) {
                timeout = TRUE;
                break;
            }
            OSSleepTicks((s64)ticksPerMsec * POLL_INTERVAL_MSEC);
        }
        if (!timeout) {
            mStatus = NHTTPGetResultCode(mRequest);
            if (mStatus >= 0 && mStatus == HTTP_STATUS_OK) {
                return TRUE;
            }
        }
    }
    destroyRequest();
    return FALSE;
}

// 800EB584
BOOL dNetWus_c::notify(u64 userId, BOOL remove) {
    if (createRequest(sNotifyUrl) == FALSE) {
        return FALSE;
    }
    char id[WII_ID_LENGTH + 1];
    snprintf(id, sizeof(id), "%016llu", userId);
    int error = NHTTPAddHeaderField(mRequest, sAppVersionHeader, sAppVersion);
    error |= NHTTPAddHeaderField(mRequest, sGameCodeHeader, OSGetAppGamename());
    error |= NHTTPAddPostDataAscii(mRequest, sUserField, sUser);
    error |= NHTTPAddPostDataAscii(mRequest, sPasswordField, sPassword);
    error |= NHTTPAddPostDataAscii(mRequest, sWiiIdField, id);
    error |= NHTTPAddPostDataAscii(mRequest, sActionField, sActions[remove]);
    if (error != 0) {
        destroyRequest();
        return FALSE;
    }
    if (sendRequest() == FALSE) {
        return FALSE;
    }
    char *result = NULL;
    BOOL success;
    if (NHTTPGetHeaderField(mRequest, sResultHeader, &result) == RESULT_CODE_LENGTH &&
        strcmp(sNotifyResult, result) == 0) {
        success = TRUE;
    } else {
        success = FALSE;
    }
    destroyRequest();
    return success;
}

// 800EB700
BOOL dNetWus_c::inquire(const u64 *keys, int num, int validTerm, u8 *results) {
    if (num == 0) {
        return TRUE;
    }
    if (createRequest(sInquiryUrl) == FALSE) {
        return FALSE;
    }
    char *ids = new (dHeap::netWc24Heap_p,
                     4) char[MAX_INQUIRY_IDS * (WII_ID_LENGTH + 1)];
    if (ids == NULL) {
        destroyRequest();
        delete[] ids;
        return FALSE;
    }
    ids[0] = 0;
    int remaining = num;
    for (;;) {
        char id[WII_ID_LENGTH + 1];
        sprintf(id, "%016llu", *keys);
        strncat(ids, id, WII_ID_LENGTH);
        if (--remaining == 0) {
            break;
        }
        strcat(ids, ",");
        keys++;
    }
    char term[RESULT_CODE_LENGTH + 1];
    snprintf(term, sizeof(term), "%d", validTerm);
    int error = NHTTPAddHeaderField(mRequest, sAppVersionHeader, sAppVersion);
    error |= NHTTPAddHeaderField(mRequest, sGameCodeHeader, OSGetAppGamename());
    error |= NHTTPAddPostDataAscii(mRequest, sUserField, sUser);
    error |= NHTTPAddPostDataAscii(mRequest, sPasswordField, sPassword);
    error |= NHTTPAddPostDataAscii(mRequest, sCheckIdsField, ids);
    error |= NHTTPAddPostDataAscii(mRequest, sValidTermField, term);
    if (error != 0) {
        destroyRequest();
        delete[] ids;
        return FALSE;
    }
    if (sendRequest() == FALSE) {
        delete[] ids;
        return FALSE;
    }
    BOOL success = FALSE;
    char *result = NULL;
    if (NHTTPGetHeaderField(mRequest, sResultHeader, &result) == RESULT_CODE_LENGTH &&
        strcmp(sInquiryResult, result) == 0) {
        char *body = NULL;
        u32 size = 0;
        u32 bodyLength = NHTTPGetBodyBuffer(mRequest, &body, &size);
        if (bodyLength == (u32)num) {
            for (u32 i = 0; i < (u32)num; i++) {
                if (*body == '1') {
                    *results = 1;
                } else {
                    *results = 0;
                }
                results++;
                body++;
            }
            success = TRUE;
        }
    }
    destroyRequest();
    delete[] ids;
    return success;
}

// 800EB96C: compiler-generated initialization of sHeapMutex.
} // namespace dNetWus

typedef char WusSizeCheck[sizeof(dNetWus::dNetWus_c) == 0xC ? 1 : -1];
