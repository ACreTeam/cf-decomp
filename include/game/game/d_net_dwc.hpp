#ifndef GAME_D_NET_DWC_HPP
#define GAME_D_NET_DWC_HPP
#include <game/game/d_net_friend.hpp>
#include <revolution/os.h>
namespace EGG {
class Heap;
}
// DWC session and worker thread; names inferred from transport behavior.
namespace dNetDwc {
enum State_e {
    STATE_NONE,
    STATE_INITIALIZED,
    STATE_SCHEDULER_SUSPENDED,
    STATE_MIC_OPEN,
    STATE_SOCKET_INITIALIZED,
    STATE_SOCKET_STARTED,
    STATE_DWC_READY,
    STATE_LOGIN,
    STATE_ONLINE,
    STATE_MATCHING,
    STATE_CONNECTED
};
// Error stage and priority published to the network manager.
enum ErrorStage_e {
    ERROR_SOCKET_INIT = 1,
    ERROR_SOCKET_START,
    ERROR_SOCKET_STOP,
    ERROR_SOCKET_CLEANUP,
    ERROR_DWC_PROCESS,
    ERROR_LOGIN,
    ERROR_HOST_MATCH,
    ERROR_FRIEND_MATCH,
    ERROR_MATCH_TIMEOUT,
    ERROR_ANYBODY_MATCH,
    ERROR_SCHEDULER_SUSPEND = 13,
    ERROR_SCHEDULER_RESUME,
    ERROR_FRIEND_OFFLINE,
    ERROR_FRIEND_FULL
};
enum ErrorKind_e {
    ERROR_RECOVERABLE = 1,
    ERROR_RESTART_REQUIRED,
    ERROR_PERMANENT
};
enum FriendCheck_e { FRIEND_AVAILABLE, FRIEND_OFFLINE, FRIEND_FULL };
enum {
    MEMBER_COUNT = 4,
    THREAD_STACK_SIZE = 0x8000,
    THREAD_PRIORITY = 14,
    MIC_WORK_SIZE = 0x6000,
    MATCH_TIMEOUT_MS = 180000,
    KEEPALIVE_FRAMES = 180,
    KEEPALIVE_LIMIT = 40,
    MIC_SHUTDOWN_RETRIES = 5000,
    MIC_RETRY_DELAY_MS = 2,
    MIC_UNAVAILABLE = -100,
    SCHEDULER_RETRIES = 20,
    SCHEDULER_RETRY_DELAY_MS = 500,
    GAME_PRODUCT_ID = 0x2CC0,
    TRANSPORT_BUFFER_SIZE = 0x4000,
    TRANSPORT_PACKET_SIZE = 0x531,
    GAME_CODE = 0x5255554A
};
struct SendNode_c {
    SendNode_c() : mNext(0) {
    }
    /* 0x0 */ SendNode_c *mNext;
    /* 0x4 */ u8 mAid;
};
class Session_c {
  public:
    Session_c();                                             // 800E77F4
    ~Session_c();                                            // 800E799C
    void init(EGG::Heap *, EGG::Heap *, dNetFriend::Data_c *); // 800E79E4
    void cleanup();                                          // 800E7A6C
    void setAccount(DWCUserData *, DWCFriendData *, u8,
                    dNetFriend::SendBuf_c *); // 800E7ABC
    void reset();                     // 800E7AD8
    void start(DWCUserData *, DWCFriendData *, u8, dNetFriend::SendBuf_c *,
               u8);                                                 // 800E7AF8
    void requestStop();                                             // 800E7BE4
    static void *threadMain(void *);                                // 800E7C20
    void setState(int);                                             // 800E7E54
    BOOL openMic(void *);                                           // 800E7E90
    BOOL closeMic();                                                // 800E7EDC
    BOOL suspendScheduler();                                        // 800E7FA8
    BOOL resumeScheduler();                                         // 800E8058
    BOOL startSockets();                                            // 800E80B0
    BOOL stopSockets();                                             // 800E8198
    void execute();                                                 // 800E8264
    BOOL matchAnybody(u8);                                          // 800E84C0
    BOOL hostMatch(u8, u8);                                         // 800E85CC
    BOOL matchFriend(u8, u32);                                      // 800E86BC
    void cancelMatch();                                             // 800E8820
    void closeConnections();                                        // 800E8864
    void updateFriends();                                           // 800E88A8
    void setBusy();                                                 // 800E8910
    void clearBusy();                                               // 800E8918
    BOOL setMatchClosed(BOOL);                                      // 800E8920
    void sendPending();                                             // 800E89C0
    void initSendOrder(u8);                                         // 800E8A84
    void rotateSendOrder(SendNode_c *);                             // 800E8AF4
    void setError(u8, u8, int, int);                                // 800E8B50
    BOOL clearError();                                              // 800E8B5C
    void clearConnections();                                        // 800E8BB4
    void publishStatus();                                           // 800E8C44
    int checkFriend(u32);                                           // 800E8CB0
    void initVoice(u8);                                             // 800E8D18
    void cleanupVoice();                                            // 800E8D5C
    void startVoice();                                              // 800E8D60
    void stopVoice();                                               // 800E8D64
    void connectVoice(u8);                                          // 800E8DA4
    void disconnectVoice(u8);                                       // 800E8DF0
    void setVoiceEnabled(BOOL);                                     // 800E8DF8
    static void onLogin(int, int, void *);                          // 800E8E1C
    static void onFriendsUpdated(int, BOOL, void *);                // 800E8ED4
    static void onBuddyFriend(int, void *);                  // 800E8EE4
    static void onBuddyFriendOnline(int, void *);                   // 800E8EE8
    static void onFriendDeleted(int, int, void *);                    // 800E8EEC
    static int canMatch(int, void *);                               // 800E8EF0
    static void onMatchClosed(int, void *);                         // 800E8F1C
    static void onMatchProgress(int, int, void *);                  // 800E8F2C
    static void onMatched(int, BOOL, BOOL, BOOL, int, int, void *); // 800E8F30
    static void onClosed(int, BOOL, BOOL, u8, int, void *);         // 800E907C
    static void onSend(u32, u8, void *);                            // 800E9140
    static void onReceive(u8, const void *, u32, void *);           // 800E9164
    static void onPing(int, u8, void *);                            // 800E91D4
    static Session_c *sInstance;
    /* 0x000 */ OSThread mThread;
    /* 0x318 */ s64 mMatchStart;
    /* 0x320 */ DWCUserData *mUserData;
    /* 0x324 */ DWCFriendData *mFriends;
    /* 0x328 */ dNetFriend::Data_c *mData;
    /* 0x32C */ dNetFriend::Error_c mError;
    /* 0x338 */ dNetFriend::SendBuf_c *mSendBufs;
    /* 0x33C */ SendNode_c *mSendHead;
    /* 0x340 */ SendNode_c mSendOrder[MEMBER_COUNT - 1];
    /* 0x358 */ BOOL mInVoiceCalc;
    /* 0x35C */ u16 _35C[MEMBER_COUNT];
    /* 0x364 */ u8 mKeepaliveCount[MEMBER_COUNT];
    /* 0x368 */ int mState;
    /* 0x36C */ u32 _36C;
    /* 0x370 */ u32 mKeepaliveTimer;
    /* 0x374 */ u16 mMemberBits;
    /* 0x376 */ u8 mFriendNum;
    /* 0x377 */ u8 mUpdatingFriends;
    /* 0x378 */ u8 mStopRequested;
    /* 0x379 */ u8 mKeepaliveEnabled;
    /* 0x37A */ u8 mVoiceEnabled;
    /* 0x37B */ u8 mMatchClosed;
    /* 0x37C */ u8 mChangingMatchClosed;
}; // size 0x380
} // namespace dNetDwc
#endif
