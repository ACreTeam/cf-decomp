#ifndef D_NET_DS_DOWNLOAD_HPP
#define D_NET_DS_DOWNLOAD_HPP
#include <types.h>
#include <game/game/d_dvd.hpp>
// Names inferred from download application and connection operations.
namespace dNetDsDownload {
typedef void (*RecvFunc)(u32, const void *, u32);
typedef void (*SendFunc)(int);
class Application_c {
  public:
    enum State_e {
        STATE_IDLE, STATE_LOAD_ROM, STATE_OPEN, STATE_START_DOWNLOAD,
        STATE_DOWNLOAD, STATE_STOP, STATE_CLOSE, STATE_UNLOAD_ROM,
        STATE_OPEN_ERROR = 9, STATE_DOWNLOAD_ERROR = 10
    };
    Application_c();  // 800E42AC
    ~Application_c(); // 800E42E8
    bool isError() const {
        return mState == STATE_OPEN_ERROR || mState == STATE_DOWNLOAD_ERROR;
    }
    u32 execute();                                    // 800E4344
    void start(u32 id, const char *path, int banner); // 800E4550
    void stop();                                      // 800E46AC
    void copyBannerText(const wchar_t *src, wchar_t *dst); // 800E46F8
    /* 0x000 */ dDvd::bank_c mBank;
    /* 0x058 */ const char *mPath;
    /* 0x05C */ u32 mState;
    /* 0x060 */ wchar_t mTitle[11];
    /* 0x076 */ wchar_t mName[49];
    /* 0x0D8 */ wchar_t mDescription[98];
}; // 0x19C
class Connection_c {
  public:
    enum State_e {
        STATE_IDLE, STATE_OPEN, STATE_READY, STATE_STOP, STATE_CLOSE, STATE_ERROR
    };
    enum Event_e {
        EVENT_NONE = 1, EVENT_RESET_PEERS, EVENT_RECEIVE, EVENT_PEER_JOIN, EVENT_PEER_LEAVE
    };
    Connection_c();                                       // 800E3E78
    ~Connection_c();                                      // 800E3E9C
    u32 execute();                                       // 800E3EE4
    void start(u32 id, u8 listen, RecvFunc recv);         // 800E4018
    void stop();                                          // 800E40A4
    BOOL send(const void *data, u32 size, SendFunc done); // 800E40C4
    static void onSend(); // 800E4140
    static void onEvent(int event, const void *info); // 800E416C
    static void onConnect(); // 800E4250: empty SDK callback
    /* 0x00 */ RecvFunc mRecv;
    /* 0x04 */ SendFunc mSend;
    /* 0x08 */ u32 mState;
    /* 0x0C */ int mSendResult;
    /* 0x10 */ u16 mPeerBits;
    /* 0x12 */ u16 mConnected;
    /* 0x14 */ u8 mPeerAddress[6];
    /* 0x1A */ u8 mListen;
}; // 0x1C
void *openConnection(void *arg); // 800E3E20
void *closeConnection(void *arg); // 800E3E50
void *openApplication(void *arg); // 800E4254
void *closeApplication(void *arg); // 800E4284
void *alloc(u32 size); // 800E471C
void free(void *ptr); // 800E4738
void startWorker(void *(*entry)(void *)); // 800E475C
BOOL pollWorker(); // 800E47F0
void setDownloadedAddress(int index, const void *address); // 800E4864
void clearDownloadedAddress(); // 800E4878
u32 getSocketHeapSize(); // 800E4888
u32 getDwcHeapSize(); // 800E4894
} // namespace dNetDsDownload
#endif
