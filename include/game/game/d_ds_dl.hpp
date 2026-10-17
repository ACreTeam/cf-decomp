#pragma once
#include <game/game/d_net_ds_download.hpp>
#include <revolution/OS.h>
// DS download play; names inferred from protocol behavior and callers.
namespace dDsDl {
enum Mode_e { MODE_RECEIVE, MODE_SEND, MODE_LISTEN };
enum State_e {
    STATE_IDLE,
    STATE_START_APPLICATION,
    STATE_DOWNLOAD_APPLICATION,
    STATE_STOP_APPLICATION,
    STATE_CONNECT,
    STATE_CONNECTED,
    STATE_STOP_CONNECTION
};
enum Step_e {
    STEP_WAIT_APPLICATION = 0,
    STEP_RECEIVE = 2,
    STEP_SEND = 3,
    STEP_WAIT_CLOSE = 4,
    STEP_CLOSE = 5,
    STEP_WAIT_DISCONNECT = 6,
    STEP_STOP_CONNECTION = 7,
    STEP_DONE = 8
};
enum TransferState_e {
    TRANSFER_IDLE,
    TRANSFER_ERROR,
    TRANSFER_CLOSE,
    TRANSFER_SHUTDOWN,
    TRANSFER_REQUEST_RECEIVE,
    TRANSFER_WAIT_RECEIVE_ACK,
    TRANSFER_RECEIVE_SIZE,
    TRANSFER_RECEIVE_DATA,
    TRANSFER_RECEIVE_CRC,
    TRANSFER_CHECK_CRC,
    TRANSFER_SEND_NACK,
    TRANSFER_SEND_ACK,
    TRANSFER_REQUEST_SEND,
    TRANSFER_WAIT_SEND_ACK,
    TRANSFER_SEND_SIZE,
    TRANSFER_SEND_DATA,
    TRANSFER_SEND_CRC,
    TRANSFER_WAIT_RESULT,
    TRANSFER_CONFIRM_RESULT
};
enum Result_e {
    RESULT_NONE,
    RESULT_SUCCESS,
    RESULT_NO_ROM,
    RESULT_INVALID,
    RESULT_FAILED
};
enum {
    APP_EXCHANGE,
    APP_TRAVEL,
    TRANSFER_CHUNK_SIZE = 508,
    MAX_RETRIES = 5,
    CONNECT_TIMEOUT = 600,
    DISCONNECT_DELAY = 60,
    EXCHANGE_ID = 0x400805,
    TRAVEL_ID = 0x400806,
    EXCHANGE_BANNER = 14,
    TRAVEL_BANNER = 12
};
enum Command_e {
    COMMAND_REQUEST_RECEIVE = 0x52455153, // REQS
    COMMAND_REQUEST_SEND = 0x52455152,    // REQR
    COMMAND_ACK = 0x41434B53,             // ACKS
    COMMAND_NACK = 0x4E41434B,            // NACK
    COMMAND_CLOSE = 0x434C4F53,           // CLOS
    COMMAND_SHUTDOWN = 0x5348444E         // SHDN
};
class Mutex_c {
  public:
    Mutex_c() {
        OSInitMutex(&mMutex);
    }
    ~Mutex_c() {
    } // 80084800, weak header copy
    // The static mutex starts on an 8-byte boundary after its destructor node.
    OSMutex mMutex ALIGN(8);
};
class Manage_c {
  public:
    Manage_c();                                                  // 8008350C
    ~Manage_c();                                                 // 8008358C
    void execute();                                              // 80083610
    BOOL updateApplication();                                    // 800836AC
    BOOL updateConnection();                                     // 80083784
    void startApplication(int app, u32 id, int banner);          // 8008389C
    void stopApplication();                                      // 800838E0
    void startConnection(u32 id, u8 listen);                     // 80083928
    void stopConnection();                                       // 80083990
    void clearTransfer();                                        // 800839E4
    void startReceive();                                         // 800839F8
    void startSend();                                            // 80083A10
    void requestClose();                                         // 80083A64
    void requestShutdown();                                      // 80083A70
    static void onReceive(u32 peer, const void *data, u32 size); // 80083A7C
    void executeTransfer();                                      // 80083CA8
    static void onSend(int result);                              // 80083F94
    void readReceiveResult();                                    // 800840B8
    void setSendComplete();                                      // 80084188
    BOOL isConnected();                                          // 80084194
    void cancel();                                               // 800841BC
    void receiveFromDs(void *buf, u32 size);                     // 800841F8
    void executeReceive();                                       // 80084264
    void sendToDs(void *buf, u32 size);                          // 8008436C
    void executeSend();                                          // 800843DC
    void closeSend();                                            // 800844EC
    void listenForDs(void *buf, u32 size);                       // 80084538
    void executeListen();                                        // 800845AC
    void shutdownListen();                                       // 800846A0
    void closeListen();                                          // 800846EC
    BOOL finish(); // 80084738: polled by d_reset until idle
    // The connection is being stopped (or is already).
    BOOL isStopping() const {
        return (u32)mStep >= STEP_STOP_CONNECTION;
    }
    // The transfer ended with a result (the connection is being stopped).
    BOOL isTransferDone() const {
        return (u32)mStep >= STEP_STOP_CONNECTION && mResult != RESULT_NONE;
    }
    // A listen received the data successfully.
    BOOL isReceived() const {
        return (u32)mStep >= STEP_WAIT_CLOSE && mResult == RESULT_SUCCESS;
    }
    // Back to idle (after cancel / the end of a transfer).
    BOOL isDone() const {
        return (u32)mStep >= STEP_DONE;
    }

    /* 0x000 */ dNetDsDownload::Application_c mApplication;
    /* 0x19C */ dNetDsDownload::Connection_c mConnection;
    /* 0x1B8 */ int mMode;
    /* 0x1BC */ int mState;
    /* 0x1C0 */ int mStep;
    /* 0x1C4 */ int mConnectTimeout;
    /* 0x1C8 */ u8 *mBuffer;
    /* 0x1CC */ u32 mBufferSize;
    /* 0x1D0 */ int mTransferState;
    /* 0x1D4 */ u32 mTransferSize;
    /* 0x1D8 */ u32 mTransferred;
    /* 0x1DC */ u32 mChecksum;
    /* 0x1E0 */ int mResult;
    /* 0x1E4 */ u16 mDisconnectDelay;
    /* 0x1E6 */ u8 mRetries;
    /* 0x1E7 */ u8 mCanSend;
    /* 0x1E8 */ u8 mError;
    /* 0x1E9 */ u8 mFinishing;
    /* 0x1EA */ volatile u8 mWasConnected;
    /* 0x1EB */ u8 _01EB;
}; // 0x1EC
extern Manage_c g_manager; // 80584B94
void execute();            // 800835F4
} // namespace dDsDl
