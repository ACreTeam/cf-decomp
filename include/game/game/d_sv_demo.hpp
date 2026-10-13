#ifndef D_SV_DEMO_HPP
#define D_SV_DEMO_HPP

#include <game/framework/f_base.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/mLib/m_vec.hpp>

// RTTI confirms global scope. State and callback names are inferred from the
// original state-table labels and message handlers.
class dSvDemo_c : public fBase_c, public dMsg::Rcpt_c,
                  public dMsgRcptExp_c<dSvDemo_c> {
public:
    enum SaveKind_e { SAVE_CONTINUE, SAVE_INTERRUPT };
    enum BedSide_e { BED_SIDE_POSITIVE_X = 1, BED_SIDE_OTHER = 2, BED_SIDE_NONE = 3 };
    enum MessageCode_e {
        MESSAGE_NORMAL_SAVE = 0x0B,
        MESSAGE_SAVE_UNAVAILABLE = 0x14,
        MESSAGE_NETWORK_SAVE_CHOICE = 0x15,
        MESSAGE_NETWORK_SAVE_WAIT = 0x16,
        MESSAGE_NETWORK_SAVE_REFUSED = 0x19,
        MESSAGE_EXIT_BLOCKED = 0x1C
    };

    class stepNetSaveContinue_c : public dState::step_c<dSvDemo_c, stepNetSaveContinue_c> {
    public:
        stepNetSaveContinue_c() {}
        virtual ~stepNetSaveContinue_c() {}
        void requestSave(); // 801BADA4
        void waitRequest(); // 801BADF4
        void waitMessageEnd(); // 801BAEB8
        void waitRefusedMessageEnd(); // 801BAEEC
    };

    class stepNetSaveInterrupt_c : public dState::step_c<dSvDemo_c, stepNetSaveInterrupt_c> {
    public:
        stepNetSaveInterrupt_c() {}
        virtual ~stepNetSaveInterrupt_c() {}
        void requestSave(); // 801BB050
        void waitRequest(); // 801BB0A0
        void waitMessageEnd(); // 801BB164
        void waitRefusedMessageEnd(); // 801BB198
    };

    // Implicit virtual destructor: 801BB67C.
    virtual int create(); // 801B9848
    virtual int doDelete(); // 801B98F4
    virtual int execute(); // 801B9938
    virtual void onMessageEnd(int kind); // 801B998C
    virtual void onAnswer(int answer); // 801B9CDC

    void startNetworkSave(); // 801B9A28
    void startNormalSave(); // 801B9DA8
    void chooseContinueSave(); // 801B9F04
    void chooseInterruptSave(); // 801B9F18
    void requestMessage(u16 code); // 801B9F2C
    BOOL finishMessage(); // 801B9FA8
    BOOL finishMessageAndDemo(); // 801BA150
    void initializeWait(); // 801BA2FC
    void executeWait(); // 801BA300
    void initializeMessage(); // 801BABAC
    void executeMessage(); // 801BABB0
    void initializeSaveScene(); // 801BAC04
    void executeSaveScene(); // 801BAC14
    void initializeIdle(); // 801BAC5C
    void executeIdle(); // 801BAC60
    void initializeNetSaveContinue(); // 801BAC64
    void executeNetSaveContinue(); // 801BAD5C
    void initializeNetSaveInterrupt(); // 801BAF10
    void executeNetSaveInterrupt(); // 801BB008

    /* 0x0104 */ stepNetSaveContinue_c mContinueStep;
    /* 0x0134 */ stepNetSaveInterrupt_c mInterruptStep;
    /* 0x0164 */ int mSaveKind; // SaveKind_e
    /* 0x0168 */ mVec3_c mPreviousPosition;
    /* 0x0174 */ int mBedSide; // BedSide_e
}; // sizeof = 0x178

#endif
