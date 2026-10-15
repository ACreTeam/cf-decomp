#include <game/game/d_net_ds_download.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_thunder.hpp>
#include <game/mLib/m_perf.hpp>
#include <revolution/OS.h>
#include <string.h>

// Narrow interfaces to the unscaffolded DS download SDK.
extern "C" {
int fn_803FA7FC(void *);
int fn_803FA9E4();
void fn_803FB9E4(const u8 *, int);
void fn_803FE694(void (*)(), void *);
int fn_803FC7A4();
void *fn_80412B98(void *, const void *, u32);
BOOL fn_803FBB50(const void *, u32, u16, int, int *, void (*)(), void *);
int fn_804011B4(void *);
int fn_804014B4();
u32 fn_804017C4();
u32 fn_80401560();
u32 fn_8040189C();
BOOL fn_804016D4(int, void *);
}

namespace dNetDsDownload {
enum {
    RESULT_OK = 0, RESULT_PENDING = -237,
    PACKET_SIZE = 512, MAC_ADDRESS_SIZE = 6,
    WORKER_STACK_SIZE = 0x2000, WORKER_PRIORITY = 20,
    SOCKET_HEAP_SIZE = 0x19000, DWC_HEAP_SIZE = 0x7D000,
    DOWNLOAD_STATE_READY = 2, COMMON_NAME_BANNER = 11
};
// SDK configuration layouts; unknown options retain offset names.
struct ConnectionConfig {
    void *(*alloc)(u32);
    void (*free)(void *);
    u32 _0008, _000C, mId, _0014, _0018, _001C, _0020, _0024;
    u32 mSendSize, mRecvSize, _0030;
    u8 _0034[0x160];
    void (*mCallback)(int, const void *);
    u32 _0198, _019C;
    u8 _01A0[0x38];
};
struct ApplicationConfig {
    void *(*alloc)(u32);
    void (*free)(void *);
    u32 _0008, mId, _0010, _0014, _0018;
    const wchar_t *mTitle, *mName, *mDescription;
    u32 _0028;
    void *mRom;
    u32 _0030, _0034;
    u8 mLanguage, mKana;
    u8 _003A[0x1F2];
};
static ConnectionConfig sConnectionConfig = {
    alloc, free, 8, 1, 0x003FFFAA, 0xFFFFFFFF, 0, 4000, 0, 1, PACKET_SIZE, PACKET_SIZE, 1
};
char sProgramRegistration[] =
    "<NINTENDO-DS-PROGRAM-REGISTRATION-LIST>\n"
    "F658:NTRJ01(00)'NINTENDO    'SDK4.02RELEASEplus1 [NINTENDO:DWC-DL3.1.30003.080909.2241_DWC-DL_3_1_PLUS3,WiFi2.1.30003.0709200229][UBIQUITOUS:CPS,SSL]\n"
    "1E2E:NTRJ01(00)'NINTENDO    'SDK4.02RELEASEplus1 [NINTENDO:BACKUP]\n"
    "</NINTENDO-DS-PROGRAM-REGISTRATION-LIST>\n";
struct DefaultBanner {
    wchar_t mTitle[10];
    wchar_t mName[12];
    wchar_t mDescription[52];
};
static DefaultBanner sDefaultBanner = {
    L"RVFOREST1", L"DS Appli", L"This is DS Download Appli for RVFOREST1"
};
static ApplicationConfig sApplicationConfig = {
    alloc, free, 8, 0x003FFFAA, 0xFFFFFFFF, 0, 0,
    sDefaultBanner.mTitle, sDefaultBanner.mName, sDefaultBanner.mDescription, 2
};
static u8 sSendBuffer[PACKET_SIZE];
static OSThread sWorker;
const char *sRegistrationList[] = {sProgramRegistration, NULL};
static int sConnectionResult;
static Connection_c *sConnection;
static int sApplicationResult;
u32 _8074E524; // Unreferenced four-byte storage; original purpose is unknown.
static void *sWorkerStack;
static u8 sDownloadedAddress[MAC_ADDRESS_SIZE];

typedef char CheckApplicationSize[sizeof(Application_c) == 0x19C ? 1 : -1];
typedef char CheckConnectionSize[sizeof(Connection_c) == 0x1C ? 1 : -1];
typedef char CheckConnectionConfig[sizeof(ConnectionConfig) == 0x1D8 ? 1 : -1];
typedef char CheckApplicationConfig[sizeof(ApplicationConfig) == 0x22C ? 1 : -1];

// 800E3E20
void *openConnection(void *) {
    sConnectionResult = fn_803FA7FC(&sConnectionConfig);
    return NULL;
}
// 800E3E50
void *closeConnection(void *) {
    sConnectionResult = fn_803FA9E4();
    return NULL;
}
// 800E3E78
Connection_c::Connection_c() {
    mRecv = NULL;
    mSend = NULL;
    mState = STATE_IDLE;
    mSendResult = 0;
    mPeerBits = 0;
    mConnected = 0;
    sConnection = this;
}
// 800E3E9C
Connection_c::~Connection_c() { sConnection = NULL; }
// 800E3EE4
u32 Connection_c::execute() {
    switch (mState) {
    case STATE_OPEN:
        if (pollWorker()) {
            if (sConnectionResult == RESULT_OK) {
                mState = STATE_READY;
                fn_803FB9E4(&mListen, 1);
                fn_803FE694(onConnect, NULL);
            } else { mState = STATE_ERROR; }
        }
        break;
    case STATE_STOP:
        if (pollWorker()) {
            if (sConnectionResult == RESULT_OK && fn_803FC7A4() != 0) break;
            sConnectionResult = RESULT_PENDING;
            startWorker(closeConnection);
            mPeerBits = 0;
            mState = STATE_CLOSE;
        }
        break;
    case STATE_CLOSE:
        if (pollWorker()) {
            if (sConnectionResult == RESULT_OK) {
                mState = STATE_IDLE;
                mRecv = NULL;
                mSend = NULL;
            } else {
                sConnectionResult = RESULT_PENDING;
                startWorker(closeConnection);
            }
        }
        break;
    }
    mConnected = mPeerBits;
    return mState;
}
// 800E4018
void Connection_c::start(u32 id, u8 listen, RecvFunc recv) {
    sConnectionConfig.mId = id;
    mListen = listen;
    sConnectionConfig.mCallback = onEvent;
    sConnectionConfig._0198 = 2;
    sConnectionConfig._019C = 0;
    mPeerBits = 0;
    mConnected = 0;
    mRecv = recv;
    memset(mPeerAddress, 0, MAC_ADDRESS_SIZE);
    sConnectionResult = RESULT_PENDING;
    startWorker(openConnection);
    mState = STATE_OPEN;
}
// 800E40A4
void Connection_c::stop() {
    if ((int)mState == STATE_IDLE) return;
    if (mState != STATE_CLOSE) mState = STATE_STOP;
}
// 800E40C4
BOOL Connection_c::send(const void *data, u32 size, SendFunc done) {
    fn_80412B98(sSendBuffer, data, size);
    mSend = done;
    return fn_803FBB50(sSendBuffer, size, mConnected, 14, &mSendResult, onSend, NULL);
}
// 800E4140
void Connection_c::onSend() {
    if (sConnection->mPeerBits != 0 && sConnection->mSend != NULL)
        sConnection->mSend(sConnection->mSendResult);
}
// 800E416C
void Connection_c::onEvent(int event, const void *info) {
    const u32 *words = (const u32 *)info;
    Connection_c *connection = sConnection;
    switch (event) {
    case EVENT_NONE: break;
    case EVENT_RESET_PEERS: connection->mPeerBits = 0; break;
    case EVENT_PEER_JOIN:
        memcpy(connection->mPeerAddress, (const u8 *)info + 0x18, MAC_ADDRESS_SIZE);
        connection->mPeerBits |= 1 << words[4];
        break;
    case EVENT_PEER_LEAVE:
        memset(connection->mPeerAddress, 0, MAC_ADDRESS_SIZE);
        connection->mPeerBits &= ~(1 << words[4]);
        break;
    case EVENT_RECEIVE:
        if (connection->mRecv != NULL)
            connection->mRecv(words[4], (const void *)words[5], words[6]);
        break;
    }
}
// 800E4250
void Connection_c::onConnect() {}
// 800E4254
void *openApplication(void *) {
    sApplicationResult = fn_804011B4(&sApplicationConfig);
    return NULL;
}
// 800E4284
void *closeApplication(void *) {
    sApplicationResult = fn_804014B4();
    return NULL;
}
// 800E42AC
Application_c::Application_c() {
    mState = STATE_IDLE;
    mPath = NULL;
}
// 800E42E8
Application_c::~Application_c() {}

// HACK: The const-pointer read delays propagation until late PCode folding.
// This preserves the reset Boolean and the unreachable branch at 800E452C.
// TODO: Replace this workaround once the original source pattern is known.
static inline BOOL keepExecuteResult(BOOL value) {
    return *static_cast<const BOOL *>(&value);
}

// 800E4344
u32 Application_c::execute() {
    switch (mState) {
    case STATE_IDLE: break;
    case STATE_LOAD_ROM: {
        // Keep the heap snapshot ahead of the path load in the generated code.
        void *heap = dHeap::localHeap_p;
        if (mBank.load(mPath, *static_cast<void *const *>(&heap), 0)) {
            sApplicationConfig.mRom = mBank.getData();
            mState = STATE_OPEN;
            sApplicationResult = RESULT_PENDING;
            startWorker(openApplication);
        }
        break;
    }
    case STATE_OPEN:
        if (pollWorker()) {
            if (sApplicationResult == RESULT_OK) mState = STATE_START_DOWNLOAD;
            else mState = STATE_OPEN_ERROR;
        }
        break;
    case STATE_START_DOWNLOAD:
        if (fn_804017C4() == DOWNLOAD_STATE_READY) {
            if (fn_80401560() == DOWNLOAD_STATE_READY) mState = STATE_DOWNLOAD;
            else mState = STATE_DOWNLOAD_ERROR;
        }
        break;
    case STATE_DOWNLOAD:
        if (fn_8040189C() == DOWNLOAD_STATE_READY) {
            u8 info[0x30];
            if (fn_804016D4(1, info)) setDownloadedAddress(0, info + 0x1E);
            stop();
        } else if (fn_804017C4() != DOWNLOAD_STATE_READY) { mState = STATE_DOWNLOAD_ERROR; }
        break;
    case STATE_STOP:
        if (pollWorker()) {
            if (sApplicationResult == RESULT_OK) {
                mState = STATE_CLOSE;
                sApplicationResult = RESULT_PENDING;
                startWorker(closeApplication);
            } else { mState = STATE_UNLOAD_ROM; }
        }
        break;
    case STATE_CLOSE:
        if (pollWorker()) {
            if (sApplicationResult == RESULT_OK) mState = STATE_UNLOAD_ROM;
            else {
                sApplicationResult = RESULT_PENDING;
                startWorker(closeApplication);
            }
        }
        break;
    case STATE_UNLOAD_ROM:
        if (!keepExecuteResult(dReset::Manage_c::GetInstance()->isSafetyWait())) {
            if (mBank.unload(FALSE)) {
                mState = STATE_IDLE;
                break;
            }

            // HACK: Always false; retain the target's unreachable branch.
            while (!keepExecuteResult(TRUE)) {
                if (mState) break;
                break;
            }
            break;
        } else {
            mState = STATE_IDLE;
        }
        break;
    }
    return mState;
}
// 800E4550
void Application_c::start(u32 id, const char *path, int banner) {
    sApplicationConfig.mId = id;
    dString::Word_c title;
    dString::loadString(&title, banner, "sys_STRING/STR_Banner");
    copyBannerText(title.getText(), mTitle);
    sApplicationConfig.mTitle = mTitle;
    dString::Word_c name;
    dString::loadString(&name, COMMON_NAME_BANNER, "sys_STRING/STR_Banner");
    copyBannerText(name.getText(), mName);
    sApplicationConfig.mName = mName;
    dString::Word_c description;
    dString::loadString(&description, banner + 1, "sys_STRING/STR_Banner");
    copyBannerText(description.getText(), mDescription);
    sApplicationConfig.mDescription = mDescription;
    sApplicationConfig.mLanguage = getLanguage();
    sApplicationConfig.mKana = dSaveOption::getKana();
    clearDownloadedAddress();
    mPath = path;
    mState = STATE_LOAD_ROM;
}
// 800E46AC
void Application_c::stop() {
    if ((mState >= STATE_LOAD_ROM && mState < STATE_OPEN) || mState == STATE_OPEN_ERROR) mState = STATE_UNLOAD_ROM;
    else if ((mState >= STATE_OPEN && mState < STATE_STOP) || mState == STATE_DOWNLOAD_ERROR) mState = STATE_STOP;
}
// 800E46F8
void Application_c::copyBannerText(const wchar_t *src, wchar_t *dst) {
    wchar_t ch;
    while ((ch = *src) != 0) { *dst = ch; src++; dst++; }
    *dst = ch;
}
// 800E471C
void *alloc(u32 size) { return dHeap::netSoHeap_p->alloc(size, 32); }
// 800E4738
void free(void *ptr) { if (ptr != NULL) dHeap::netSoHeap_p->free(ptr); }
// 800E475C
void startWorker(void *(*entry)(void *)) {
    sWorkerStack = dHeap::netWc24Heap_p->alloc(WORKER_STACK_SIZE, 32);
    OSCreateThread(&sWorker, entry, NULL, (u8 *)sWorkerStack + WORKER_STACK_SIZE, WORKER_STACK_SIZE, WORKER_PRIORITY, OS_THREAD_DETACHED);
    mPerf::setThreadColor(&sWorker, dPerf::getThreadColor(9));
    OSResumeThread(&sWorker);
}
// 800E47F0
BOOL pollWorker() {
    if (OSIsThreadTerminated(&sWorker)) {
        if (sWorkerStack != NULL) {
            dHeap::netWc24Heap_p->free(sWorkerStack);
            sWorkerStack = NULL;
            mPerf::resetThreadColor(&sWorker);
        }
        return TRUE;
    }
    return FALSE;
}
// 800E4864
void setDownloadedAddress(int index, const void *address) { memcpy(sDownloadedAddress + index * MAC_ADDRESS_SIZE, address, MAC_ADDRESS_SIZE); }
// 800E4878
void clearDownloadedAddress() { memset(sDownloadedAddress, 0, MAC_ADDRESS_SIZE); }
// 800E4888
u32 getSocketHeapSize() { return SOCKET_HEAP_SIZE; }
// 800E4894
u32 getDwcHeapSize() { return DWC_HEAP_SIZE; }
} // namespace dNetDsDownload
