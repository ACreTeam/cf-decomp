#ifndef D_A_RMOBJ_TEL_HPP
#define D_A_RMOBJ_TEL_HPP

// Include order: the weak RTTI / vtables in .data follow class completion (m3d
// animations, fBase_c, dActor_c, dDvd, dMsg::Rcpt_c, dDemoActor_c, then
// daRmobj_c::model_c), as d_a_rmobj_sample.hpp.
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_msg_comp.hpp>

// The telephone in the player's house (REL d_a_rmobj_telNP, profile
// A_RMOBJ_TEL, model "obj_myroomR_tel"). Talking to it picks it up (the
// player's STATE_PHONE_* states call pickUp / talk / hangUp) and Rover
// (SP_NPC_GUIDE) answers with the game settings (npc_SP/NPC_tel: voices, text,
// menu tutorials, WiiConnect24) and, at a house that flies a flag, the flag
// design. A new flag design that the player wears is put on after hanging up.
// Not a daRmobj_c (its RTTI has no daRmobj_c base); it only uses its model_c.
// Class name from RTTI ("daRmobjTel_c"); the modes are named from the mode
// table's names.
class daRmobjTel_c : public dDemoActor_c,
                     public dMsg::Rcpt_c,
                     public dMsgRcptExp_c<daRmobjTel_c> {
  public:
    typedef dState::base_c<daRmobjTel_c>::Method Method;

    // npc_SP/NPC_tel message codes (l_msgLabel), named from the English text.
    enum {
        MSG_CANT_USE =
            0x03, // "Seems like you can't use the phone right now" (net play)
        MSG_BYE = 0x04,   // "OK, bye-bye then!"
        MSG_HELLO = 0x05, // "What can I do for you?" (the settings menu)
        MSG_ANYTHING_ELSE = 0x06, // "Anything else?" (the settings menu)
        MSG_VOICE = 0x07,         // Animalese / Bebebese / Nothing
        MSG_VOICE_CONFIRM = 0x08, // "... sounds like this... Is that OK?"
        MSG_KANA = 0x0A,          // Kanji or Hiragana only (Japanese)
        MSG_KEY_INPUT = 0x0D,     // keyboard or cell-phone style
        MSG_TUTORIAL = 0x10,      // tutorial callouts for menu icons
        MSG_NOT_YOUR_PHONE =
            0x14, // "Using other people's phones ... is kind of rude"
        MSG_WC24_EXPLAIN = 0x15, // "This is very important info..."
        MSG_WC24_ON = 0x18, // "currently set to ON. What would you like to do?"
        MSG_WC24_OFF =
            0x19, // "currently set to OFF. What would you like to do?"
        MSG_WC24_ON_AGAIN = 0x1A, // after MSG_WC24_EXPLAIN
        MSG_WC24_OFF_AGAIN = 0x1B,
        MSG_FLAG_DESIGN_KEEP =
            0x1D, // "you're happy with what you've got?" (no design chosen)
        MSG_FLAG_HELLO = 0x1F,    // Game settings / Flag design / Never mind (a
                                  // house with a flag)
        MSG_GAME_SETTINGS = 0x20, // "What'll it be?" (the settings menu)
        MSG_FLAG_DESIGN = 0x21,   // ends with END_REQ_DESIGN_MENU
        MSG_FLAG_DESIGN_DONE =
            0x22, // "I'll make sure it turns out just the way you want it."
        MSG_FLAG_ANYTHING_ELSE =
            0x23 // MSG_ANYTHING_ELSE at a house with a flag
    };

    // onMessageEnd kinds (the BMG entries' end requests).
    enum { END_REQ_DESIGN_MENU = 0x12 };

    // sys_SELECT/SYS_Select choices of the settings menu
    // (dSelect::Comp_c::setSysMessage).
    enum {
        SEL_MSG_SOUND = 0x13,
        SEL_MSG_TEXT = 0x14,
        SEL_MSG_MENU = 0x15,
        SEL_MSG_WC24 = 0x16,
        SEL_MSG_NEVER_MIND = 0x17
    };

    // The modes (l_modes in create), in table order.
    enum {
        MODE_WAIT,                // "待機"
        MODE_PHONE_ANM_START_REQ, // "電話アニメ開始要求" (the player picks the
                                  // phone up)
        MODE_PHONE_ANM_START,    // "電話アニメ開始"
        MODE_MESSAGE,            // "メッセージ"
        MODE_MESSAGE_SUSPEND,    // "メッセージサスペンド" (for the design menu)
        MODE_MESSAGE_MENU_OPEN,  // "メッセージメニュー開く"
        MODE_MESSAGE_MENU_CLOSE, // "メッセージメニュー閉じる"
        MODE_MESSAGE_TO_CHANGE,  // "メッセージ→着替え"
        MODE_CHANGE_PHONE_ANM_END_REQ, // "プレイヤ着替え：電話アニメ終了要求"
        MODE_CHANGE_PHONE_ANM_END,     // "プレイヤ着替え：電話アニメ終了"
        MODE_CHANGE_START,             // "プレイヤ着替え開始"
        MODE_CHANGE_END,               // "プレイヤ着替え終了"
        MODE_PHONE_ANM_END_REQ,        // "電話アニメ終了要求"
        MODE_PHONE_ANM_END,            // "電話アニメ終了"
        MODE_SPEAKER_SUSPEND, // "メッセージサスペンド" (changing the speaker:
                              // mRequestEndMethod)
        MODE_MESSAGE_OPEN, // "メッセージ開く"
        MODE_NUM
    };

    // The phone model's chr animations.
    enum { ANM_IDLE, ANM_PICK_UP, ANM_TALK };

    daRmobjTel_c() {
    }
    virtual ~daRmobjTel_c() {
    }
    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual bool canInteract(dDemoActor_c *actor);
    virtual void onDemoCmd(int cmd, u8 member);

    virtual void onAnswer(int answer);
    struct MessageEndEntry {
        const char *label;
        int code;
        Method method;
    };
    void dispatchMessageEnd(MessageEndEntry *entry, int count);
    virtual void onMessageEnd(int kind);
    virtual void onMessageStart(int arg);
    virtual int getForcedVoiceMode();
    virtual int getVoiceMode();

    // The player's phone states (d_a_player STATE_PHONE_*): the phone's
    // animation.
    static void pickUp();
    static void talk();
    static void hangUp();

    // The phone sounds, mSeTimer frames after the animation starts.
    void startPickUpSe();
    BOOL isPickUpEnd();
    void startHangUpSe();
    BOOL isHangUpEnd();

    void initWait();
    void executeWait();
    void initPhoneAnmStartReq();
    void executePhoneAnmStartReq();
    void initPhoneAnmStart();
    void executePhoneAnmStart();
    void initMessage();
    void executeMessage();
    void initMessageSuspend();
    void executeMessageSuspend();
    void initMessageMenuOpen();
    void executeMessageMenuOpen();
    void initMessageMenuClose();
    void executeMessageMenuClose();
    void initMessageToChange();
    void executeMessageToChange();
    void initChangePhoneAnmEndReq();
    void executeChangePhoneAnmEndReq();
    void initChangePhoneAnmEnd();
    void executeChangePhoneAnmEnd();
    void initChangeStart();
    void executeChangeStart();
    void initChangeEnd();
    void executeChangeEnd();
    void initPhoneAnmEndReq();
    void executePhoneAnmEndReq();
    void initPhoneAnmEnd();
    void executePhoneAnmEnd();
    void initSpeakerSuspend();
    void executeSpeakerSuspend();
    void initMessageOpen();
    void executeMessageOpen();

    void startMessage(u16 code);

    // The message window's state.
    static bool isMsgHidden(dMsg::Comp_c *comp) {
        return comp->isState(&dMsg::Comp_c::executeHidden);
    }
    static bool isMsgSuspend(dMsg::Comp_c *comp) {
        return comp->isState(&dMsg::Comp_c::executeSuspend);
    }

    // Runs mRequestEndMethod once the window is suspended
    // (MODE_SPEAKER_SUSPEND); cleared unless it set a new one.
    void callRequestEndMethod() {
        if (mRequestEndMethod) {
            Method method = mRequestEndMethod;
            callMessageMethod(method);
            if (method == mRequestEndMethod) {
                mRequestEndMethod = NULL;
            }
        }
    }

    // onAnswer handlers.
    void selectVoiceMode0();
    void selectVoiceMode1();
    void selectVoiceMode2();
    void cancelVoiceMode();
    void decideVoiceMode();
    void setKanaOff();
    void setKanaOn();
    void setKeySeOff();
    void setKeySeOn();
    void setTutorialOn();
    void setTutorialOff();
    void enableNet();
    void disableNet();
    void selectNetSetting();

    // onMessageEnd handlers.
    void returnFromNetError();
    void returnFromNetError2();
    void holdNetError();
    void returnFromNetError3();
    void askNetSetting();
    void openSettingsMenu();

    BOOL hasHouseFlag();
    void requestMessage(u16 code, const char *label);
    void switchToSystemSpeaker();
    void switchToRoverSpeaker();
    void hideRoverSpeaker();
    void showRoverSpeaker();
    void startSpeakerSuspend();
    void startMessageOpen();

    static daRmobjTel_c *sInstance;

    /* 0x000 dDemoActor_c */
    /* 0x0BC dMsg::Rcpt_c */
    /* 0x120 dMsgRcptExp_c<daRmobjTel_c> */
    /* 0x15C */ daRmobj_c::model_c mModel;
    /* 0x308 */ int
        mVoiceMode; // VOICE_MODE_*: the voice tried in MSG_VOICE_CONFIRM
    /* 0x30C */ int mSeTimer; // frames to the phone sound, -1 once played
    /* 0x310 */ dEquip_c
        mEquip; // the clothes to put on after the call (the new flag design)
    /* 0x318 */ u8 mRoverSpeaks;   // the message has Rover's name
                                   // (onMessageStart; getVoiceMode)
    /* 0x319 */ u8 mChangeClothes; // put mEquip on after hanging up
}; // size 0x31C

#endif
