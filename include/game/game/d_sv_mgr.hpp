#pragma once

#include <game/framework/f_base.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_state_mode.hpp>
#include <game/game/d_sv_step.hpp>

// Save manager. Method names are inferred from the original state-table labels.
class dSvMgr_c : public fBase_c, public dMsg::Rcpt_c,
                 public dState::mode_c<dSvMgr_c> {
public:
    template <class Owner, class Step>
    class stepBase_c : public dSvStep_c<Owner, Step> {
    public:
        void requestSaveMessage();
        void requestNetworkSaveMessage();
        void waitMessageLock();
        void releaseMessageLock();
        void waitMessageEnd();
        void gotoOutsideGate();
        void gotoPreviousScene();
        void gotoWelcomeDemo();
        void gotoTitle();
        void finish();
        void finishFinal();
        void waitCanSave();
        void waitNewPlayer();
        void findFriend();
        void waitSavePermission();
        void processLoadedSave();
        void initializeGrowth();
        void processGrowth();
        stepBase_c() {}
        inline virtual ~stepBase_c();
    };

    class stepSaveNormal_c : public stepBase_c<dSvMgr_c, stepSaveNormal_c> {
    public:
        stepSaveNormal_c() {}
        inline virtual ~stepSaveNormal_c();
        void closeGate(); // 801BC58C
        void disconnectWiFi(); // 801BC634
        void startWC24Send(); // 801BC6DC
        void waitWC24Send(); // 801BC780
        void startVillageMail(); // 801BC824
        void waitVillageMail(); // 801BC8C0
        void finish(); // 801BC96C
    };

    class stepSaveRetireNetVst_c : public stepBase_c<dSvMgr_c, stepSaveRetireNetVst_c> {
    public:
        stepSaveRetireNetVst_c() {}
        inline virtual ~stepSaveRetireNetVst_c();
    };

    class stepSaveConnectNetHst_c : public stepBase_c<dSvMgr_c, stepSaveConnectNetHst_c> {
    public:
        stepSaveConnectNetHst_c() {}
        inline virtual ~stepSaveConnectNetHst_c();
        void initializeSave(); // 801BD098
    };

    class stepSaveConnectNetVst_c : public stepBase_c<dSvMgr_c, stepSaveConnectNetVst_c> {
    public:
        stepSaveConnectNetVst_c() {}
        inline virtual ~stepSaveConnectNetVst_c();
    };

    class stepSaveContinueNetHst_c : public stepBase_c<dSvMgr_c, stepSaveContinueNetHst_c> {
    public:
        stepSaveContinueNetHst_c() {}
        inline virtual ~stepSaveContinueNetHst_c();
        void waitAllMachinesSaved(); // 801BD6C8
    };

    class stepSaveContinueNetVst_c : public stepBase_c<dSvMgr_c, stepSaveContinueNetVst_c> {
    public:
        stepSaveContinueNetVst_c() {}
        inline virtual ~stepSaveContinueNetVst_c();
        void notifyHostSaveComplete(); // 801BDA48
        void waitSaveDemoEnd(); // 801BDAF0
    };

    class stepSaveInterruptNetHst_c : public stepBase_c<dSvMgr_c, stepSaveInterruptNetHst_c> {
    public:
        stepSaveInterruptNetHst_c() {}
        inline virtual ~stepSaveInterruptNetHst_c();
        void initializeSave(); // 801BDED0
    };

    class stepSaveInterruptNetVst_c : public stepBase_c<dSvMgr_c, stepSaveInterruptNetVst_c> {
    public:
        stepSaveInterruptNetVst_c() {}
        inline virtual ~stepSaveInterruptNetVst_c();
        void initializeSave(); // 801BE388
    };

    // MWCC emits vtables in reverse class-completion order; keep load last.
    class stepLoad_c : public stepBase_c<dSvMgr_c, stepLoad_c> {
    public:
        stepLoad_c() {}
        inline virtual ~stepLoad_c();
        void processLoadedSave(); // 801BE938
        void gotoTitle(); // 801BEC2C
        void finish(); // 801BECD4
    };

    dSvMgr_c() {}
    inline virtual ~dSvMgr_c(); // 801C6CF8; game-heap deletion, not ordinary delete
    virtual int create(); // 801BBBFC
    virtual int doDelete(); // 801BBDFC
    virtual int execute(); // 801BBE94
    static dSvMgr_c *createMgr(); // 801BB7C4
    typedef dState::base_c<dSvMgr_c>::Method Method;
    virtual void onMessageEnd(int kind); // 801BBF64; receiver completion callback
    void requestMessage(u16 code);
    void lockMessage();
    dMsg::Comp_c *getMessageController() { return mpController; }
    void setLoadFailed() { mLoadFailed = true; }
    static void setLoadErrorMessage(u16 code);
    void finishLoad();
    static void requestSaveNormal();
    static void requestSaveRetireNetVst();
    static void requestSaveConnectNetHst();
    static void requestSaveConnectNetVst();
    static void requestSaveContinueNetHst();
    static void requestSaveContinueNetVst();
    static void requestSaveInterruptNetHst();
    static void requestSaveInterruptNetVst();
    static void cancelRequest();
    static int isTransferComplete(int offset, int size);
    static int isFullTransferComplete();
    static int isTownTransferComplete();
    static int isDLDataTransferComplete();
    static int isPlayerTransferComplete(int player);
    static int getDLDataOffset();
    static int getVisitorDataOffset();
    static int getPlayerDataOffset(int player);
    static void addVisitor(int player);
    static void removeVisitor(int player);
    static void clearVisitors();
    static int getLastVisitor();
    static int getRequiredBlocks();
    static int getRequiredInodes();
    static int getRequiredFiles();
    static unsigned int blocksForSize(unsigned int size);
    static unsigned int inodesForSize(unsigned int size);
    static void closeGateForSave();
    static void clearRequestedMode();
    static Method mRequestedMode;
    static int mVisitors[3];
    static int mCurrentVisitor;
    static int mPlayerIndex;
    static const char *mMessageLabel;

    void initializeWait(); // 801BC03C
    void executeWait(); // 801BC040
    void initializeIdle(); // 801BC1F4
    void executeIdle(); // 801BC1F8
    void initializeSaveNormal(); // 801BC1FC
    void executeSaveNormal(); // 801BC544
    void initializeSaveRetireNetVst(); // 801BCA1C
    void executeSaveRetireNetVst(); // 801BCD3C
    void initializeSaveConnectNetHst(); // 801BCD84
    void executeSaveConnectNetHst(); // 801BD050
    void initializeSaveConnectNetVst(); // 801BD148
    void executeSaveConnectNetVst(); // 801BD3C0
    void initializeSaveContinueNetHst(); // 801BD408
    void executeSaveContinueNetHst(); // 801BD680
    void initializeSaveContinueNetVst(); // 801BD788
    void executeSaveContinueNetVst(); // 801BDA00
    void initializeSaveInterruptNetHst(); // 801BDBA0
    void executeSaveInterruptNetHst(); // 801BDE88
    void initializeSaveInterruptNetVst(); // 801BDFC0
    void executeSaveInterruptNetVst(); // 801BE340
    void initializeLoad(); // 801BE7C0
    void executeLoad(); // 801BE8F0

protected:
    stepLoad_c mLoad; // 0x0EC
    bool mLoadFailed; // 0x11C; provisional semantic name
    dState::base_c<dSvMgr_c>::Method mMessageCallback; // 0x120
    stepSaveNormal_c mSaveNormal; // 0x12C
    stepSaveRetireNetVst_c mSaveRetireNetVst; // 0x15C
    stepSaveConnectNetHst_c mSaveConnectNetHst; // 0x18C
    stepSaveConnectNetVst_c mSaveConnectNetVst; // 0x1BC
    stepSaveContinueNetHst_c mSaveContinueNetHst; // 0x1EC
    stepSaveContinueNetVst_c mSaveContinueNetVst; // 0x21C
    stepSaveInterruptNetHst_c mSaveInterruptNetHst; // 0x24C
    stepSaveInterruptNetVst_c mSaveInterruptNetVst; // 0x27C
}; // sizeof = 0x2AC

// sys_MESS/SYS_Save_and_Load message codes (dSvMgr_c::mMessageLabel, sLoadErrorMessage).
enum {
    SV_MSG_NAND_DAMAGED = 0x0C, // "The Wii system memory has been damaged."
    SV_MSG_NAND_ERROR = 0x12,   // "Could not access Wii system memory."
    SV_MSG_SAVING_NET = 0x0D,   // "Saving... Please do not touch RESET or the Power Button."
    SV_MSG_SAVING_VISIT = 0x1B, // "Saving... Please do not touch the Power Button or RESET."
    SV_MSG_VISITOR_CAME = 0x1E, // "<name> has come over to play."
    SV_MSG_VISITOR_LEFT = 0x20, // "<name> is going home."
};

// sys_MESS/SYS_NetworkError message codes (sNetErrorMessageLabel).
enum {
    NET_ERROR_MSG_WFC_ERROR = 0x01,        // "Unable to connect to Nintendo Wi-Fi Connection."
    NET_ERROR_MSG_WFC_BUSY = 0x02,         // "high traffic volumes, or the service is down"
    NET_ERROR_MSG_WFC_DISCONTINUED = 0x03, // the service for this software has been discontinued
    NET_ERROR_MSG_NAND_FULL = 0x04,        // "not enough available space in Wii system memory"
    NET_ERROR_MSG_NAND_CORRUPTED = 0x05,   // "The Wii system memory has been corrupted."
    NET_ERROR_MSG_NO_INTERNET = 0x06,      // "Unable to connect to the Internet."
    NET_ERROR_MSG_NO_RESPONSE = 0x07,      // "There is no response." (friend roster)
    NET_ERROR_MSG_DISCONNECTED = 0x08,     // "You have been disconnected from Nintendo Wi-Fi Connection."
    NET_ERROR_MSG_COMM_ERROR = 0x09,       // "Communication error. Please try again."
    NET_ERROR_MSG_DOWNLOAD_FAILED = 0x0A,  // "Download failed."
    NET_ERROR_MSG_WFC_RESET = 0x0B,        // "an error has occurred. Reset and try again."
    NET_ERROR_MSG_DISCONNECTED2 = 0x0C,    // same text as NET_ERROR_MSG_DISCONNECTED
    NET_ERROR_MSG_RESTRICTED = 0x13,   // Parental Controls restrict network features
    NET_ERROR_MSG_WC24_NAND = 0x15,    // WC24 errors: 0x15 + dWifi::getErrorMessage() (0..9)
    NET_ERROR_MSG_WC24_UPDATE = 0x16,  // a system update is required
    NET_ERROR_MSG_WC24_RESET = 0x17,   // "Reset and try again."
    NET_ERROR_MSG_WC24_BUSY = 0x18,    // "temporarily unavailable"
    NET_ERROR_MSG_WC24_OFF = 0x19,     // WiiConnect24 is off in Wii Options
    NET_ERROR_MSG_WC24_NO_INTERNET = 0x1A,
    NET_ERROR_MSG_WC24_BUSY2 = 0x1B,
    NET_ERROR_MSG_WC24_BUSY3 = 0x1C,
    NET_ERROR_MSG_WC24_NAND2 = 0x1D,
    NET_ERROR_MSG_WC24_RESET2 = 0x1E,
    NET_ERROR_MSG_WC24_OTHER = 0x1F,   // any other error (refer to the manual)
};

extern const char *sNetErrorMessageLabel; // 8074C3CC: "SYS_MESS/SYS_NetworkError"
extern u16 sLoadErrorMessage; // 8074EB98 (d_nand clears it after deleting the save)
