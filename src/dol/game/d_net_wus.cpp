#include <game/game/d_ds_dl.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net_wus.hpp>
#include <revolution/OS.h>
#include <revolution/OS/OSTime.h>
#include <stdio.h>
#include <string.h>


// Narrow interfaces to the unscaffolded socket and HTTP SDK libraries.
extern "C" {
struct SocketAllocators {
    void *(*alloc)(int, u32);
    void (*free)(int, void *);
};
int fn_8043BE3C(SocketAllocators *);
int fn_8043C100();
int fn_8043C534();
int fn_8043C004();
int fn_8041AD48(void *(*)(u32, int), void (*)(void *), int);
void fn_8041ADD4(int);
void *fn_8041A87C(const char *, int, void *, u32, void *, void *);
int fn_8041B7BC(void *, int);
int fn_8041B824(void *, int);
int fn_8041B4C8(void *, int);
int fn_8041A9C4(void *);
int fn_8041AB60(void *);
int fn_8041ACFC(void *);
int fn_8041B468(void *);
int fn_8041B0EC(void *, const char *, const char *);
int fn_8041B1B0(void *, const char *, const char *);
int fn_8041B394(void *, const char *, char **);
u32 fn_8041ABEC(void *, char **, u32 *);
char *fn_804571A8(char *, const char *, size_t);
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
        SocketAllocators callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.alloc = allocSocket;
        callbacks.free = freeSocket;
        int error = fn_8043BE3C(&callbacks);
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_SOCKET;
    }
    if (sInitState < INIT_SOCKET_STARTED) {
        int error = fn_8043C100();
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
        if (fn_8041AD48(alloc, free, HTTP_THREAD_PRIORITY) != 0) {
            return FALSE;
        }
        sInitState = INIT_HTTP;
    }
    return TRUE;
}

// 800EB2D4
BOOL dNetWus_c::close() {
    if (sInitState >= INIT_HTTP) {
        fn_8041ADD4(0);
        sInitState = INIT_SOCKET_STARTED;
    }
    if (sResponseBuffer) {
        dHeap::netWc24Heap_p->free(sResponseBuffer);
        sResponseBuffer = NULL;
    }
    if (sInitState >= INIT_SOCKET_STARTED) {
        int error = fn_8043C534();
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_SOCKET;
    }
    if (sInitState >= INIT_SOCKET) {
        int error = fn_8043C004();
        if (error != 0 && error != SOCKET_ALREADY_INITIALIZED) {
            return FALSE;
        }
        sInitState = INIT_NONE;
    }
    return TRUE;
}

// 800EB398
BOOL dNetWus_c::createRequest(const char *url) {
    mRequest = fn_8041A87C(url, HTTP_POST, sResponseBuffer,
                           RESPONSE_BUFFER_SIZE, NULL, NULL);
    if (mRequest == NULL) {
        return FALSE;
    }
    if (fn_8041B7BC(mRequest, 0) == 0 && fn_8041B824(mRequest, 0) == 0 &&
        fn_8041B4C8(mRequest, 0) == 0) {
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
    mError = fn_8041A9C4(mRequest);
    if (mError == 0) {
        mRequest = NULL;
        return TRUE;
    }
    return FALSE;
}

// 800EB4A0
BOOL dNetWus_c::sendRequest() {
    mError = fn_8041AB60(mRequest);
    if (mError == 0) {
        u32 ticksPerMsec;
        BOOL timeout = FALSE;
        u32 start = OSGetTick();
        while (fn_8041ACFC(mRequest) == HTTP_REQUEST_PENDING) {
            ticksPerMsec = OS_TIME_SPEED / 1000;
            if ((OSGetTick() - start) / ticksPerMsec >= REQUEST_TIMEOUT_MSEC) {
                timeout = TRUE;
                break;
            }
            OSSleepTicks((s64)ticksPerMsec * POLL_INTERVAL_MSEC);
        }
        if (!timeout) {
            mStatus = fn_8041B468(mRequest);
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
    int error = fn_8041B0EC(mRequest, sAppVersionHeader, sAppVersion);
    error |= fn_8041B0EC(mRequest, sGameCodeHeader, OSGetAppGamename());
    error |= fn_8041B1B0(mRequest, sUserField, sUser);
    error |= fn_8041B1B0(mRequest, sPasswordField, sPassword);
    error |= fn_8041B1B0(mRequest, sWiiIdField, id);
    error |= fn_8041B1B0(mRequest, sActionField, sActions[remove]);
    if (error != 0) {
        destroyRequest();
        return FALSE;
    }
    if (sendRequest() == FALSE) {
        return FALSE;
    }
    char *result = NULL;
    BOOL success;
    if (fn_8041B394(mRequest, sResultHeader, &result) == RESULT_CODE_LENGTH &&
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
        fn_804571A8(ids, id, WII_ID_LENGTH);
        if (--remaining == 0) {
            break;
        }
        strcat(ids, ",");
        keys++;
    }
    char term[RESULT_CODE_LENGTH + 1];
    snprintf(term, sizeof(term), "%d", validTerm);
    int error = fn_8041B0EC(mRequest, sAppVersionHeader, sAppVersion);
    error |= fn_8041B0EC(mRequest, sGameCodeHeader, OSGetAppGamename());
    error |= fn_8041B1B0(mRequest, sUserField, sUser);
    error |= fn_8041B1B0(mRequest, sPasswordField, sPassword);
    error |= fn_8041B1B0(mRequest, sCheckIdsField, ids);
    error |= fn_8041B1B0(mRequest, sValidTermField, term);
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
    if (fn_8041B394(mRequest, sResultHeader, &result) == RESULT_CODE_LENGTH &&
        strcmp(sInquiryResult, result) == 0) {
        char *body = NULL;
        u32 size = 0;
        u32 bodyLength = fn_8041ABEC(mRequest, &body, &size);
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
