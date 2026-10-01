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
        stepBase_c() {}
        virtual ~stepBase_c() {}
    };

    class stepLoad_c : public stepBase_c<dSvMgr_c, stepLoad_c> {
    public:
        stepLoad_c() {}
        virtual ~stepLoad_c() {}
        void processLoadedSave(); // 801BE938
        void gotoTitle(); // 801BEC2C
        void finish(); // 801BECD4
        void initializeLoad(); // 801C30C0
        void startLoad(); // 801C3158
        void waitLoad(); // 801C31FC
    };

    class stepSaveNormal_c : public stepBase_c<dSvMgr_c, stepSaveNormal_c> {
    public:
        stepSaveNormal_c() {}
        virtual ~stepSaveNormal_c() {}
        void checkSaveResult();
        void closeGate(); // 801BC58C
        void disconnectWiFi(); // 801BC634
        void startWC24Send(); // 801BC6DC
        void waitWC24Send(); // 801BC780
        void startVillageMail(); // 801BC824
        void waitVillageMail(); // 801BC8C0
        void finish(); // 801BC96C
        void requestSaveMessage(); // 801C18D8
        void waitMessageLock(); // 801C197C
        void releaseMessageLock(); // 801C1A64
        void waitMessageEnd(); // 801C1B14
        void gotoTitle(); // 801C1BEC
        void finishFinal(); // 801C1C94
        void initializeFullSave(); // 801C5080
        void startFullSave(); // 801C5128
        void waitFullSave(); // 801C51CC
        void initializeDownloadBox(); // 801C51D0
        void createDownloadBox(); // 801C52E8
        void disableHomeAndReset(); // 801C54B4
        void enableHomeAndReset(); // 801C5564
        void saveFailed(); // 801C5614
    };

    class stepSaveRetireNetVst_c : public stepBase_c<dSvMgr_c, stepSaveRetireNetVst_c> {
    public:
        stepSaveRetireNetVst_c() {}
        virtual ~stepSaveRetireNetVst_c() {}
        void checkSaveResult();
        void requestNetworkSaveMessage(); // 801C1170
        void waitMessageLock(); // 801C1214
        void releaseMessageLock(); // 801C12FC
        void waitMessageEnd(); // 801C13AC
        void gotoOutsideGate(); // 801C1484
        void finish(); // 801C1530
        void processLoadedSave(); // 801C1538
        void initializeGrowth(); // 801C178C
        void processGrowth(); // 801C182C
        void initializeLoad(); // 801C4908
        void startLoad(); // 801C49A0
        void waitLoad(); // 801C4A44
        void initializeFullSave(); // 801C4AE8
        void startFullSave(); // 801C4B90
        void waitFullSave(); // 801C4C34
        void initializeDownloadBox(); // 801C4C38
        void createDownloadBox(); // 801C4D50
        void disableHomeAndReset(); // 801C4F1C
        void enableHomeAndReset(); // 801C4FCC
        void saveFailed(); // 801C507C
    };

    class stepSaveConnectNetHst_c : public stepBase_c<dSvMgr_c, stepSaveConnectNetHst_c> {
    public:
        stepSaveConnectNetHst_c() {}
        virtual ~stepSaveConnectNetHst_c() {}
        void checkSaveResult();
        void initializeSave(); // 801BD098
        void requestNetworkSaveMessage(); // 801C0A74
        void waitMessageLock(); // 801C0B18
        void releaseMessageLock(); // 801C0C00
        void waitMessageEnd(); // 801C0CB0
        void gotoWelcomeDemo(); // 801C0D88
        void finish(); // 801C0F20
        void waitNewPlayer(); // 801C0F28
        void findFriend(); // 801C0FD0
        void waitSavePermission(); // 801C10CC
        void initializeHostSave(); // 801C4678
        void startHostSave(); // 801C4714
        void waitHostSave(); // 801C47B8
        void initializeTownSave(); // 801C47BC
        void startTownSave(); // 801C485C
        void waitTownSave(); // 801C4900
        void saveFailed(); // 801C4904
    };

    class stepSaveConnectNetVst_c : public stepBase_c<dSvMgr_c, stepSaveConnectNetVst_c> {
    public:
        stepSaveConnectNetVst_c() {}
        virtual ~stepSaveConnectNetVst_c() {}
        void checkSaveResult();
        void requestNetworkSaveMessage(); // 801C0378
        void waitMessageLock(); // 801C041C
        void releaseMessageLock(); // 801C0504
        void waitMessageEnd(); // 801C05B4
        void gotoWelcomeDemo(); // 801C068C
        void finish(); // 801C0824
        void waitNewPlayer(); // 801C082C
        void findFriend(); // 801C08D4
        void waitSavePermission(); // 801C09D0
        void initializeVisitorSave(); // 801C4530
        void startVisitorSave(); // 801C45CC
        void waitVisitorSave(); // 801C4670
        void saveFailed(); // 801C4674
    };

    class stepSaveContinueNetHst_c : public stepBase_c<dSvMgr_c, stepSaveContinueNetHst_c> {
    public:
        stepSaveContinueNetHst_c() {}
        virtual ~stepSaveContinueNetHst_c() {}
        void checkSaveResult();
        void waitAllMachinesSaved(); // 801BD6C8
        void requestNetworkSaveMessage(); // 801BFF0C
        void waitMessageLock(); // 801BFFB0
        void releaseMessageLock(); // 801C0098
        void waitMessageEnd(); // 801C0148
        void gotoPreviousScene(); // 801C0220
        void finish(); // 801C02CC
        void waitCanSave(); // 801C02D4
        void initializeFullSave(); // 801C40F8
        void startFullSave(); // 801C41A0
        void waitFullSave(); // 801C4244
        void initializeDownloadBox(); // 801C4248
        void createDownloadBox(); // 801C4360
        void saveFailed(); // 801C452C
    };

    class stepSaveContinueNetVst_c : public stepBase_c<dSvMgr_c, stepSaveContinueNetVst_c> {
    public:
        stepSaveContinueNetVst_c() {}
        virtual ~stepSaveContinueNetVst_c() {}
        void checkSaveResult();
        void notifyHostSaveComplete(); // 801BDA48
        void waitSaveDemoEnd(); // 801BDAF0
        void requestNetworkSaveMessage(); // 801BFAA0
        void waitMessageLock(); // 801BFB44
        void releaseMessageLock(); // 801BFC2C
        void waitMessageEnd(); // 801BFCDC
        void gotoPreviousScene(); // 801BFDB4
        void finish(); // 801BFE60
        void waitCanSave(); // 801BFE68
        void initializeVisitorSave(); // 801C3FB0
        void startVisitorSave(); // 801C404C
        void waitVisitorSave(); // 801C40F0
        void saveFailed(); // 801C40F4
    };

    class stepSaveInterruptNetHst_c : public stepBase_c<dSvMgr_c, stepSaveInterruptNetHst_c> {
    public:
        stepSaveInterruptNetHst_c() {}
        virtual ~stepSaveInterruptNetHst_c() {}
        void checkSaveResult();
        void initializeSave(); // 801BDED0
        void requestNetworkSaveMessage(); // 801BF4E8
        void waitMessageLock(); // 801BF58C
        void releaseMessageLock(); // 801BF674
        void waitMessageEnd(); // 801BF724
        void gotoPreviousScene(); // 801BF7FC
        void finish(); // 801BF8A8
        void waitCanSave(); // 801BF8B0
        void initializeGrowth(); // 801BF954
        void processGrowth(); // 801BF9F4
        void initializeFullSave(); // 801C3A18
        void startFullSave(); // 801C3AC0
        void waitFullSave(); // 801C3B64
        void initializeDownloadBox(); // 801C3B68
        void createDownloadBox(); // 801C3C80
        void disableHomeAndReset(); // 801C3E4C
        void enableHomeAndReset(); // 801C3EFC
        void saveFailed(); // 801C3FAC
    };

    class stepSaveInterruptNetVst_c : public stepBase_c<dSvMgr_c, stepSaveInterruptNetVst_c> {
    public:
        stepSaveInterruptNetVst_c() {}
        virtual ~stepSaveInterruptNetVst_c() {}
        void checkSaveResult();
        void initializeSave(); // 801BE388
        void requestSaveMessage(); // 801BECDC
        void waitMessageLock(); // 801BED80
        void releaseMessageLock(); // 801BEE68
        void waitMessageEnd(); // 801BEF18
        void gotoOutsideGate(); // 801BEFF0
        void finish(); // 801BF09C
        void waitCanSave(); // 801BF0A4
        void processLoadedSave(); // 801BF148
        void initializeGrowth(); // 801BF39C
        void processGrowth(); // 801BF43C
        void initializeLoad(); // 801C32A0
        void startLoad(); // 801C3338
        void waitLoad(); // 801C33DC
        void initializeFullSave(); // 801C3480
        void startFullSave(); // 801C3528
        void waitFullSave(); // 801C35CC
        void initializeDownloadBox(); // 801C35D0
        void createDownloadBox(); // 801C36E8
        void disableHomeAndReset(); // 801C38B4
        void enableHomeAndReset(); // 801C3964
        void saveFailed(); // 801C3A14
    };

    dSvMgr_c() {}
    virtual ~dSvMgr_c(); // 801C6CF8; game-heap deletion, not ordinary delete
    virtual int create(); // 801BBBFC
    virtual int doDelete(); // 801BBDFC
    virtual int execute(); // 801BBE94
    static dSvMgr_c *createMgr(); // 801BB7C4
    typedef dState::base_c<dSvMgr_c>::Method Method;
    virtual void rcptHook14(); // 801BBF64; receiver completion callback
    void requestMessage(u16 code);
    void lockMessage();
    void *getMessageController() { return mpController; }
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
    static int isHostTransferComplete();
    static int isPlayerTransferComplete(int player);
    static int getHostDataOffset();
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
