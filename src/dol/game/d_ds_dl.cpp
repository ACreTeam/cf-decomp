#include <game/game/d_ds_dl.hpp>
#include <game/sLib/s_crc.hpp>
#include <string.h>
extern "C" void *fn_80412B98(void *, const void *, u32);
namespace dDsDl {
static const char s_noRom[] = "B2mRWeDsApNoRom";
static const char s_invalid[] = "B2mRWeDAInvalid";
static const char s_failed[] = "B2mRWeDsAFailed";
static Mutex_c s_mutex;
Manage_c g_manager;
static const char *s_appPaths[] = {"/DSApp/Exch_Sign.srl",
                                   "/DSApp/Vaga_Sign.srl"};
typedef void (Manage_c::*ModeFunc)();
static ModeFunc s_modeProc[] = {&Manage_c::executeReceive,
                                &Manage_c::executeSend,
                                &Manage_c::executeListen};
// 8008350C
Manage_c::Manage_c() {
    mMode = MODE_RECEIVE;
    mState = STATE_IDLE;
    mStep = STEP_DONE;
    mConnectTimeout = 0;
    mBuffer = NULL;
    mBufferSize = 0;
    mTransferState = TRANSFER_IDLE;
    mTransferSize = 0;
    mTransferred = 0;
    mChecksum = 0;
    mResult = RESULT_NONE;
    mRetries = 0;
    mCanSend = 0;
    mError = 0;
    mFinishing = 0;
    mWasConnected = 0;
}
// 8008358C
Manage_c::~Manage_c() {
}
// 800835F4
void execute() {
    if (g_manager.mFinishing) {
        return;
    }
    g_manager.execute();
}
// 80083610
void Manage_c::execute() {
    if (updateApplication() == FALSE) {
        mError = 1;
        return;
    }
    if (updateConnection() == FALSE) {
        mError = 1;
        return;
    }
    (this->*s_modeProc[mMode])();
    if (mTransferState == TRANSFER_ERROR) {
        mError = 1;
    } else {
        mError = 0;
    }
}
// 800836AC
BOOL Manage_c::updateApplication() {
    if ((u32)mState < STATE_START_APPLICATION ||
        (u32)mState > STATE_STOP_APPLICATION) {
        return TRUE;
    }
    u32 state = mApplication.execute();
    if (mApplication.isError()) {
        return FALSE;
    }
    switch (mState) {
    case STATE_START_APPLICATION:
        if (state == 4) {
            mState = STATE_DOWNLOAD_APPLICATION;
        }
        break;
    case STATE_DOWNLOAD_APPLICATION:
        if (state == 5) {
            mState = STATE_STOP_APPLICATION;
        }
        break;
    case STATE_STOP_APPLICATION:
        if (state == 0) {
            mState = STATE_IDLE;
        }
        break;
    }
    return TRUE;
}
// 80083784
BOOL Manage_c::updateConnection() {
    if ((u32)mState < STATE_CONNECT || (u32)mState > STATE_STOP_CONNECTION) {
        return TRUE;
    }
    mConnection.execute();
    if (mConnection.mState == STATE_CONNECTED) {
        return FALSE;
    }
    switch (mState) {
    case STATE_CONNECT:
        if (mConnection.mConnected) {
            mState = STATE_CONNECTED;
            mCanSend = 1;
            mWasConnected = 1;
        } else if (mConnectTimeout && --mConnectTimeout == 0) {
            return FALSE;
        }
        break;
    case STATE_CONNECTED:
        if (!mConnection.mConnected && mWasConnected) {
            return FALSE;
        }
        if (mCanSend) {
            executeTransfer();
        }
        break;
    case STATE_STOP_CONNECTION:
        if (mConnection.mState == STATE_IDLE) {
            mState = STATE_IDLE;
        }
        break;
    }
    return TRUE;
}
// 8008389C
void Manage_c::startApplication(int app, u32 id, int banner) {
    mApplication.start(id, s_appPaths[app], banner);
    mState = STATE_START_APPLICATION;
}
// 800838E0
void Manage_c::stopApplication() {
    if ((u32)mState >= STATE_START_APPLICATION &&
        (u32)mState < STATE_STOP_APPLICATION) {
        mApplication.stop();
        mState = STATE_STOP_APPLICATION;
    }
}
// 80083928
void Manage_c::startConnection(u32 id, u8 listen) {
    mConnection.start(id, listen, onReceive);
    mState = STATE_CONNECT;
    if (listen == 0) {
        mConnectTimeout = CONNECT_TIMEOUT;
    } else {
        mConnectTimeout = 0;
    }
}
// 80083990
void Manage_c::stopConnection() {
    if ((u32)mState >= STATE_CONNECT && (u32)mState < STATE_STOP_CONNECTION) {
        mConnection.stop();
        mState = STATE_STOP_CONNECTION;
        mCanSend = 0;
    }
}
// 800839E4
void Manage_c::clearTransfer() {
    mTransferState = TRANSFER_IDLE;
    mTransferSize = 0;
    mTransferred = 0;
}
// 800839F8
void Manage_c::startReceive() {
    mResult = RESULT_NONE;
    mRetries = 0;
    mTransferState = TRANSFER_REQUEST_RECEIVE;
}
// 80083A10
void Manage_c::startSend() {
    mChecksum = sCrc::calcCRC32(mBuffer, mBufferSize, -1, -1);
    mResult = RESULT_NONE;
    mRetries = 0;
    mTransferState = TRANSFER_REQUEST_SEND;
}
// 80083A64
void Manage_c::requestClose() {
    mTransferState = TRANSFER_CLOSE;
}
// 80083A70
void Manage_c::requestShutdown() {
    mTransferState = TRANSFER_SHUTDOWN;
}
// 80083A7C
void Manage_c::onReceive(u32, const void *data, u32 size) {
    u32 command = size >= 4 ? __lwbrx((void *)data, 0) : 0;
    OSLockMutex(&s_mutex.mMutex);
    switch (g_manager.mTransferState) {
    case TRANSFER_WAIT_RECEIVE_ACK:
        if (size == 4 && command == COMMAND_ACK) {
            g_manager.mTransferState = TRANSFER_RECEIVE_SIZE;
        } else {
            g_manager.mTransferState = TRANSFER_ERROR;
        }
        break;
    case TRANSFER_RECEIVE_SIZE:
        if (size == 4) {
            if (command <= g_manager.mBufferSize) {
                g_manager.mTransferSize = command;
                g_manager.mTransferState = TRANSFER_RECEIVE_DATA;
            } else {
                g_manager.mTransferSize = g_manager.mBufferSize;
                g_manager.mTransferState = TRANSFER_ERROR;
            }
        } else {
            g_manager.mTransferState = TRANSFER_ERROR;
        }
        break;
    case TRANSFER_RECEIVE_DATA:
        if (g_manager.mTransferred + size > g_manager.mTransferSize) {
            size = g_manager.mTransferSize - g_manager.mTransferred;
        }
        fn_80412B98(g_manager.mBuffer + g_manager.mTransferred, data, size);
        g_manager.mTransferred += size;
        if (g_manager.mTransferred >= g_manager.mTransferSize) {
            g_manager.mTransferState = TRANSFER_RECEIVE_CRC;
        }
        break;
    case TRANSFER_RECEIVE_CRC:
        g_manager.mChecksum = command;
        g_manager.mTransferState = TRANSFER_CHECK_CRC;
        break;
    case TRANSFER_WAIT_SEND_ACK:
        if (size == 4 && command == COMMAND_ACK) {
            g_manager.mTransferState = TRANSFER_SEND_SIZE;
            g_manager.mTransferSize = g_manager.mBufferSize;
        } else {
            g_manager.mTransferState = TRANSFER_ERROR;
        }
        break;
    case TRANSFER_WAIT_RESULT:
        if (size == 4) {
            if (command == COMMAND_ACK) {
                g_manager.mTransferState = TRANSFER_CONFIRM_RESULT;
                break;
            }
            if (command == COMMAND_NACK) {
                if (++g_manager.mRetries < MAX_RETRIES) {
                    g_manager.clearTransfer();
                    g_manager.mTransferState = TRANSFER_REQUEST_SEND;
                } else {
                    g_manager.mTransferState = TRANSFER_ERROR;
                }
                break;
            }
        }
        g_manager.mTransferState = TRANSFER_ERROR;
        break;
    }
    OSUnlockMutex(&s_mutex.mMutex);
}
// 80083CA8
void Manage_c::executeTransfer() {
    OSLockMutex(&s_mutex.mMutex);
    u32 command;
    switch (mTransferState) {
    case TRANSFER_REQUEST_RECEIVE:
        __stwbrx(COMMAND_REQUEST_RECEIVE, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_CHECK_CRC:
        u32 crc = sCrc::calcCRC32(mBuffer, mTransferSize, -1, -1);
        if (crc == mChecksum) {
            __stwbrx(COMMAND_ACK, &command, 0);
            if (mConnection.send(&command, 4, onSend) == FALSE) {
                mCanSend = 0;
                mTransferState = TRANSFER_SEND_ACK;
            }
        } else if (++mRetries < MAX_RETRIES) {
            __stwbrx(COMMAND_NACK, &command, 0);
            if (mConnection.send(&command, 4, onSend) == FALSE) {
                mCanSend = 0;
                mTransferState = TRANSFER_SEND_NACK;
            }
        } else {
            mTransferState = TRANSFER_ERROR;
        }
        break;
    case TRANSFER_REQUEST_SEND:
        __stwbrx(COMMAND_REQUEST_SEND, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_SEND_SIZE:
        __stwbrx(mTransferSize, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_SEND_DATA: {
        if (mConnection.send(mBuffer + mTransferred,
                             mTransferred + TRANSFER_CHUNK_SIZE < mTransferSize
                                 ? TRANSFER_CHUNK_SIZE
                                 : mTransferSize - mTransferred,
                             onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    }
    case TRANSFER_SEND_CRC:
        __stwbrx(mChecksum, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_CONFIRM_RESULT:
        __stwbrx(COMMAND_ACK, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_CLOSE:
        __stwbrx(COMMAND_CLOSE, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    case TRANSFER_SHUTDOWN:
        __stwbrx(COMMAND_SHUTDOWN, &command, 0);
        if (mConnection.send(&command, 4, onSend) == FALSE) {
            mCanSend = 0;
        }
        break;
    }
    OSUnlockMutex(&s_mutex.mMutex);
}
// 80083F94
void Manage_c::onSend(int result) {
    g_manager.mCanSend = 1;
    if (result == 0) {
        switch (g_manager.mTransferState) {
        case TRANSFER_REQUEST_RECEIVE:
            g_manager.mTransferState = TRANSFER_WAIT_RECEIVE_ACK;
            break;
        case TRANSFER_SEND_ACK:
            g_manager.readReceiveResult();
            if (g_manager.mMode == MODE_RECEIVE) {
                g_manager.mWasConnected = 0;
            }
            g_manager.clearTransfer();
            break;
        case TRANSFER_SEND_NACK:
            g_manager.clearTransfer();
            g_manager.mTransferState = TRANSFER_WAIT_RECEIVE_ACK;
            break;
        case TRANSFER_REQUEST_SEND:
            g_manager.mTransferState = TRANSFER_WAIT_SEND_ACK;
            break;
        case TRANSFER_SEND_SIZE:
            g_manager.mTransferState = TRANSFER_SEND_DATA;
            break;
        case TRANSFER_SEND_DATA:
            g_manager.mTransferred += TRANSFER_CHUNK_SIZE;
            if (g_manager.mTransferred >= g_manager.mTransferSize) {
                g_manager.mTransferred = g_manager.mTransferSize;
                g_manager.mTransferState = TRANSFER_SEND_CRC;
            }
            break;
        case TRANSFER_SEND_CRC:
            g_manager.mTransferState = TRANSFER_WAIT_RESULT;
            break;
        case TRANSFER_CONFIRM_RESULT:
            g_manager.setSendComplete();
            g_manager.clearTransfer();
            break;
        case TRANSFER_CLOSE:
        case TRANSFER_SHUTDOWN:
            g_manager.mWasConnected = 0;
            g_manager.mTransferState = TRANSFER_IDLE;
            break;
        }
    }
    g_manager.executeTransfer();
}
// 800840B8
void Manage_c::readReceiveResult() {
    if (mMode == MODE_RECEIVE && mTransferSize == 16) {
        void *data = mBuffer;
        if (memcmp(s_noRom, data, 16) == 0) {
            mResult = RESULT_NO_ROM;
            return;
        }
        if (memcmp(s_invalid, data, 16) == 0) {
            mResult = RESULT_INVALID;
            return;
        }
        if (memcmp(s_failed, data, 16) == 0) {
            mResult = RESULT_FAILED;
            return;
        }
    }
    mResult = RESULT_SUCCESS;
}
// 80084188
void Manage_c::setSendComplete() {
    mResult = RESULT_SUCCESS;
}
// 80084194
BOOL Manage_c::isConnected() {
    if (mApplication.mState == 4 || mConnection.mConnected != 0) {
        return TRUE;
    }
    return FALSE;
}
// 800841BC
void Manage_c::cancel() {
    stopApplication();
    stopConnection();
    mStep = STEP_STOP_CONNECTION;
}
// 800841F8
void Manage_c::receiveFromDs(void *buf, u32 size) {
    mMode = MODE_RECEIVE;
    mBuffer = (u8 *)buf;
    mBufferSize = size;
    clearTransfer();
    mWasConnected = 0;
    mError = 0;
    mFinishing = 0;
    startApplication(APP_TRAVEL, TRAVEL_ID, TRAVEL_BANNER);
    mStep = STEP_WAIT_APPLICATION;
}
// 80084264
void Manage_c::executeReceive() {
    switch (mStep) {
    case STEP_WAIT_APPLICATION:
        if (mState == STATE_IDLE) {
            startConnection(TRAVEL_ID, 0);
            startReceive();
            mStep = STEP_RECEIVE;
        }
        break;
    case STEP_RECEIVE:
        if (mTransferState == TRANSFER_IDLE) {
            mDisconnectDelay = DISCONNECT_DELAY;
            mStep = STEP_WAIT_DISCONNECT;
        }
        break;
    case STEP_WAIT_DISCONNECT:
        if (mDisconnectDelay) {
            --mDisconnectDelay;
        }
        if (!g_manager.isConnected() || !mDisconnectDelay) {
            stopConnection();
            mStep = STEP_STOP_CONNECTION;
        }
        break;
    case STEP_STOP_CONNECTION:
        if (mState == STATE_IDLE) {
            mStep = STEP_DONE;
        }
        break;
    }
}
// 8008436C
void Manage_c::sendToDs(void *buf, u32 size) {
    mMode = MODE_SEND;
    mBuffer = (u8 *)buf;
    mBufferSize = size;
    clearTransfer();
    mWasConnected = 0;
    mError = 0;
    mFinishing = 0;
    startApplication(APP_EXCHANGE, EXCHANGE_ID, EXCHANGE_BANNER);
    mStep = STEP_WAIT_APPLICATION;
}
// 800843DC
void Manage_c::executeSend() {
    switch (mStep) {
    case STEP_WAIT_APPLICATION:
        if (mState == STATE_IDLE) {
            startConnection(EXCHANGE_ID, 0);
            startSend();
            mStep = STEP_SEND;
        }
        break;
    case STEP_SEND:
        if (mTransferState == TRANSFER_IDLE) {
            mStep = STEP_WAIT_CLOSE;
        }
        break;
    case STEP_CLOSE:
        if (mTransferState == TRANSFER_IDLE) {
            mDisconnectDelay = DISCONNECT_DELAY;
            mStep = STEP_WAIT_DISCONNECT;
        }
        break;
    case STEP_WAIT_DISCONNECT:
        if (mDisconnectDelay) {
            --mDisconnectDelay;
        }
        if (!g_manager.isConnected() || !mDisconnectDelay) {
            stopConnection();
            mStep = STEP_STOP_CONNECTION;
        }
        break;
    case STEP_STOP_CONNECTION:
        if (mState == STATE_IDLE) {
            mStep = STEP_DONE;
        }
        break;
    }
}
// 800844EC
void Manage_c::closeSend() {
    if ((u32)mStep < STEP_DONE && (u32)mState == STATE_CONNECTED) {
        requestClose();
        mStep = STEP_CLOSE;
    }
}
// 80084538
void Manage_c::listenForDs(void *buf, u32 size) {
    mMode = MODE_LISTEN;
    mBuffer = (u8 *)buf;
    mBufferSize = size;
    clearTransfer();
    mWasConnected = 0;
    mError = 0;
    mFinishing = 0;
    startConnection(EXCHANGE_ID, 1);
    startReceive();
    mStep = STEP_RECEIVE;
}
// 800845AC
void Manage_c::executeListen() {
    switch (mStep) {
    case STEP_RECEIVE:
        if (mTransferState == TRANSFER_IDLE) {
            mStep = STEP_WAIT_CLOSE;
        }
        break;
    case STEP_SEND:
        break;
    case STEP_CLOSE:
        if (mTransferState == TRANSFER_IDLE) {
            mDisconnectDelay = DISCONNECT_DELAY;
            mStep = STEP_WAIT_DISCONNECT;
        }
        break;
    case STEP_WAIT_DISCONNECT:
        if (mDisconnectDelay) {
            --mDisconnectDelay;
        }
        if (!g_manager.isConnected() || !mDisconnectDelay) {
            stopConnection();
            mStep = STEP_STOP_CONNECTION;
        }
        break;
    case STEP_STOP_CONNECTION:
        if (mState == STATE_IDLE) {
            mStep = STEP_DONE;
        }
        break;
    }
}
// 800846A0
void Manage_c::shutdownListen() {
    if ((u32)mStep < STEP_DONE && (u32)mState == STATE_CONNECTED) {
        requestShutdown();
        mStep = STEP_CLOSE;
    }
}
// 800846EC
void Manage_c::closeListen() {
    if ((u32)mStep < STEP_DONE && (u32)mState == STATE_CONNECTED) {
        requestClose();
        mStep = STEP_CLOSE;
    }
}
// 80084738
BOOL Manage_c::finish() {
    if (mState == STATE_IDLE) {
        mStep = STEP_DONE;
        mFinishing = 0;
        return TRUE;
    }
    if ((u32)mStep < STEP_STOP_CONNECTION) {
        mFinishing = 1;
        cancel();
    } else {
        execute();
    }
    return FALSE;
}
typedef char CheckManagerSize[sizeof(Manage_c) == 0x1EC ? 1 : -1];
typedef char CheckApplicationSize[sizeof(dNetDsDownload::Application_c) == 0x19C
                                      ? 1
                                      : -1];
typedef char
    CheckConnectionSize[sizeof(dNetDsDownload::Connection_c) == 0x1C ? 1 : -1];
} // namespace dDsDl
