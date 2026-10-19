#ifndef D_A_NPC_SP_GATEKEEPER_A_HPP
#define D_A_NPC_SP_GATEKEEPER_A_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>
#include <game/game/d_sv_step.hpp>

// Copper, the gatekeeper at the town gate (REL d_a_npc_sp_gatekeeper_aNP,
// profile A_NPC_SP_GATEKEEPER_A). He opens and closes the gate for net play
// (host and visitor), sees players off and welcomes them, connects to
// Nintendo WFC and runs the DS "go out" application.
//
// Class names from RTTI ("dAcNpcSpGkA_c", "dAcNpcSpGkA_c::talk_c", the step
// classes, "dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>"); mode and step names
// from the original CP932 state labels; everything else is named from
// behaviour. Names marked "provisional" were picked during scaffolding and may
// be refined (see the plan of the TU).
//
// Declaration order matters:
// - .data holds the weak vtables in reverse class-completion order: the talk
//   steps (stepDemoGoOutNetVst_c first), dMsgRcptExp_c<talk_c>,
//   mode_c<talk_c>, then the main steps (stepDemoBringBack_c first ...
//   stepDemoOpenGateNetHst_c last), then mode_c<dAcNpcSpGkA_c>. So the main
//   steps are declared before talk_c, in member order, and talk_c's steps in
//   member order.
// - stepDemoBase_c members are instantiated per step class in declaration
//   order; the order below was merged from all the instantiations.
class dAcNpcSpGkA_c : public dAcNpcSp_c, public dState::mode_c<dAcNpcSpGkA_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    typedef dState::base_c<dAcNpcSpGkA_c>::Method Method;

    class talk_c;

    // The modes (l_modes in create, .rodata 0x2C), in table order.
    enum {
        MODE_WAIT,               // "待機"
        MODE_DEMO_WAIT,          // "デモ待機"
        MODE_TALK,               // "会話"
        MODE_TURN_BACK,          // "初期角度に復帰" (back to the initial angle)
        MODE_NONE,               // "処理無し"
        MODE_OPEN_GATE_NET_HST,  // "デモ：ネットホストの門を開く"
        MODE_CLOSE_GATE_NET_HST, // "デモ：ネットホストの門を閉じる"
        MODE_GO_OUT_NET_VST, // "デモ：ネットビジターとして通信開始シーケンス"
        MODE_COME_IN_NET,    // "デモ：新規参入受け入れシーケンス"
        MODE_REUNION_NET_HST, // "デモ：迷子ネコ：ネットホストの新規参入受け入れシーケンス"
        MODE_REUNION_NET_VST, // "デモ：迷子ネコ：ネットビジターの新規参入受け入れシーケンス"
        MODE_RETIRE_NET_VST,   // "デモ：退場（本人）"
        MODE_SEE_OFF_NET_HST,  // "デモ：退場（見送り）"
        MODE_SEE_OFF_NET_VST,  // "デモ：退場（見送り）"
        MODE_GO_OUT_DS_APPLI,  // "デモ：DSアプリでおでかけ"
        MODE_COME_IN_DS_APPLI, // "デモ：DSアプリで受け入れ"
        MODE_REMOTE,           // "通信：リモート"
        MODE_REMOTE_TALK,      // "通信：リモート会話"
        MODE_BRING_BACK,       // "デモ：プレイヤ引き戻し"
    };

    // The common demo steps of the gate demos (RTTI
    // "dAcNpcSpGkA_c::stepDemoBase_c<Owner, Step>"). Owner is dAcNpcSpGkA_c or
    // talk_c; the gatekeeper is the owner itself or talk_c::mpOwner. Adds 8
    // bytes to dSvStep_c and no virtuals. The step methods are named from the
    // labels of the step tables that use them.
    template <class Owner, class Step>
    class stepDemoBase_c : public dSvStep_c<Owner, Step> {
      public:
        // ---- helpers (instantiated in the second pass: called from the
        // methods below) ----
        void initSaluteAnm(); // the other gatekeeper's salute (mTimer)
        BOOL updateSaluteAnm();
        void initFaceSideAnm(); // turn sideways (mTimer)
        BOOL updateFaceSideAnm();
        BOOL isFaceSideAnmDone();
        BOOL isWalkTimeUp();             // sLib::calcTimer(&mWalkTimer) == 0
        void startExitMove(int member);  // camera close-up, escorted walk out
                                         // to (256, 0, 48); mWalkTimer = 60
        void startEnterMove(int member); // walk to (256, 0, 304);
                                         // mWalkTimer = 60
        void startGateMove(int member);  // walk to (256, 0, 336)
        BOOL isPlayerMoveDone(int member);

        // ---- step methods ----
        void faceFront();          // "もんばん前を向く"
        void initFaceSide();       // "もんばん横を向く初期化"
        void faceSide();           // "もんばん横を向く"
        void waitFaceSide();       // "もんばん横を向く完了"
        void initSalute();         // "もんばん敬礼初期化"
        void salute();             // "もんばん敬礼"
        void waitFadeIn();         // "フェードイン終了待ち"
        void keepBgmVolume();      // "BGM音量維持"
        void waitLockStart();      // "ロック開始まで待つ"
        void releaseIconAndLock(); // "アイコンとロック解除"
        void startSeeOffMsg();     // "見送りメッセージ開始"
        void startWelcomeMsg();    // "受け入れメッセージ開始"
        void waitTalkEnd();        // "会話終了まで待つ" / "会話終了待ち"
        void closeChat();          // "チャット閉じる"
        void waitTalkSuspend();    // "会話サスペンドまで待つ"
        void openGate();           // "門を開く"
        void waitGateOpen();       // "門が開くまで待つ"
        void waitPlayerWalkTime(); // "プレイヤが指定時間歩いたか？"
        void toPrevScene();        // "元のシーンへ"
        void toTitleScene();       // "タイトルシーンへ"
        void finish();             // "終了"
        void sendReleaseRequest(); // "デモのロック受付キャンセルクリアを送信"
        void notifySaveDone(); // "セーブ終了を連絡" / "セーブとデモが終了なら"
        void waitDisconnectReady(); // "デモの終了条件待ち"
        void
        waitItemsReceived(); // "「相手が持っている全追加データ」の受け取りが終わっているか？"
        void waitJoinSent();    // "「全データ」の送信が終わっているか？"
        void waitJoinDoneAll(); // "全マシンネットデモが終了可能か？"
        void sendFinishJoin();  // "「お出かけデモの終了を送信したい」をセット"
        void waitJoinAllDone(); // "「ネットデモの終了命令」が来ていたら"
        void
        waitJoinFinished(); // "「お出かけデモの終了命令」を送信し終わったか？"
        void waitDemoComplete(); // "デモ自体が完了しているか？" (mDemoDone)
        void startPlayerMoveToGate(); // "門の手前までプレイヤ移動開始"
        void waitPlayerMoveToGate();  // "門の手前までプレイヤ移動完了"
        void turnPlayerToGate();      // "門の方向へプレイヤ回転"
        void waitPlayerTurnToGate();  // "門の方向へプレイヤ回転完了"
        void demoDone();              // "デモ完了！！" (sets mDemoDone)

        stepDemoBase_c() {
        }
        virtual ~stepDemoBase_c() {
        }

        /* 0x30 */ u32 mTimer;     // the animation helpers
        /* 0x34 */ u32 mWalkTimer; // isWalkTimeUp
    }; // size 0x38

    // ---- the main steps, in member order ----

    // Mode MODE_OPEN_GATE_NET_HST.
    class stepDemoOpenGateNetHst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoOpenGateNetHst_c> {
      public:
        stepDemoOpenGateNetHst_c() {
        }
        virtual ~stepDemoOpenGateNetHst_c() {
        }
        void startParent();    // 0x2078 "親の開始"
        void finishTalkDemo(); // 0x2128 "会話デモ終了"
    };

    // Mode MODE_CLOSE_GATE_NET_HST.
    class stepDemoCloseGateNetHst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoCloseGateNetHst_c> {
      public:
        stepDemoCloseGateNetHst_c() {
        }
        virtual ~stepDemoCloseGateNetHst_c() {
        }
        void closeGate();      // 0x2274 "門を閉じる"
        void waitGateClose();  // 0x2310 "門が閉じるまで待つ"
        void finishTalkDemo(); // 0x23B4 "会話デモ終了"
    };

    // Mode MODE_GO_OUT_NET_VST: the player leaves to visit another town.
    class stepDemoGoOutNetVst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutNetVst_c> {
      public:
        stepDemoGoOutNetVst_c() {
        }
        virtual ~stepDemoGoOutNetVst_c() {
        }
        void startPlayerExit();   // 0x272C "プレイヤが門の外へ出る開始"
        void toOutsideScene();    // 0x2A9C "門の外のシーンへ"
        void startProcedureMsg(); // 0x2C28 "手続きメッセージ開始"
    };

    // Mode MODE_RETIRE_NET_VST: the visitor leaves (himself).
    class stepDemoRetireNetVst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoRetireNetVst_c> {
      public:
        stepDemoRetireNetVst_c() {
        }
        virtual ~stepDemoRetireNetVst_c() {
        }
        void startPlayerMoveToGate(); // 0x3A50 "門の手前までプレイヤ移動開始"
        void waitPlayerMoveToGate();  // 0x3AF0 "門の手前までプレイヤ移動完了"
        void turnPlayerToGate();      // 0x3B98 "門の方向へプレイヤ回転"
        void waitPlayerTurnToGate();  // 0x3C3C "門の方向へプレイヤ回転完了"
        void startPlayerExit();       // 0x3CE4 "プレイヤが門の外へ出る開始"
        void toSaveScene();           // 0x3D84 "セーブシーンへ"
    };

    // Mode MODE_GO_OUT_DS_APPLI: the player leaves with the DS application.
    class stepDemoGoOutDsAppli_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoGoOutDsAppli_c> {
      public:
        stepDemoGoOutDsAppli_c() {
        }
        virtual ~stepDemoGoOutDsAppli_c() {
        }
        void startPlayerExit(); // 0x4EFC "プレイヤが門の外へ出る開始"
    };

    // Mode MODE_REUNION_NET_HST ("lost cat": a visitor joins, host side).
    class stepDemoReunionNetHst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetHst_c> {
      public:
        stepDemoReunionNetHst_c() {
        }
        virtual ~stepDemoReunionNetHst_c() {
        }
        void setupVisitor(); // 0x2FC8 "初期化処理"
    };

    // Mode MODE_REUNION_NET_VST ("lost cat": visitor side).
    class stepDemoReunionNetVst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoReunionNetVst_c> {
      public:
        stepDemoReunionNetVst_c() {
        }
        virtual ~stepDemoReunionNetVst_c() {
        }
    };

    // Mode MODE_COME_IN_NET: a new visitor walks in.
    class stepDemoComeInNet_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInNet_c> {
      public:
        stepDemoComeInNet_c() {
        }
        virtual ~stepDemoComeInNet_c() {
        }
        void startPlayerEnter(); // 0x3350 "プレイヤが門の中に入る開始"
        void playerMoveDone();   // 0x33F8 "プレイヤ移動完了"
    };

    // Mode MODE_COME_IN_DS_APPLI: the player comes back with the DS
    // application.
    class stepDemoComeInDsAppli_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoComeInDsAppli_c> {
      public:
        stepDemoComeInDsAppli_c() {
        }
        virtual ~stepDemoComeInDsAppli_c() {
        }
        void startPlayerEnter(); // 0x5264 "プレイヤが門の中に入る開始"
        void toOutsideScene();   // 0x5304 "門の外のシーンへ"
        void playerMoveDone();   // 0x53B0 "プレイヤ移動完了"
    };

    // Mode MODE_SEE_OFF_NET_HST: the host sees a visitor off.
    class stepDemoSeeOffNetHst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetHst_c> {
      public:
        stepDemoSeeOffNetHst_c() {
        }
        virtual ~stepDemoSeeOffNetHst_c() {
        }
        void waitSaveStart();   // 0x423C "セーブ開始まで待つ"
        void startPlayerExit(); // 0x42EC "プレイヤが門の外へ出る開始"
    };

    // Mode MODE_SEE_OFF_NET_VST: a visitor sees another visitor off.
    class stepDemoSeeOffNetVst_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoSeeOffNetVst_c> {
      public:
        stepDemoSeeOffNetVst_c() {
        }
        virtual ~stepDemoSeeOffNetVst_c() {
        }
        void waitSaveStart();   // 0x472C "セーブ開始まで待つ"
        void startPlayerExit(); // 0x47D0 "プレイヤが門の外へ出る開始"
    };

    // Mode MODE_BRING_BACK: the player is walked back in front of the gate.
    class stepDemoBringBack_c
        : public stepDemoBase_c<dAcNpcSpGkA_c, stepDemoBringBack_c> {
      public:
        stepDemoBringBack_c() {
        }
        virtual ~stepDemoBringBack_c() {
        }
        void startPlayerMoveToGate(); // 0x49BC "門の手前までプレイヤ移動開始"
        void waitPlayerMoveToGate();  // 0x4A5C "門の手前までプレイヤ移動完了"
        void finishTalk();            // 0x4B04 "会話終了"
    };

    // The message receptor (RTTI "dAcNpcSpGkA_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // The modes (l_modes in the constructor, .rodata 0x248).
        enum {
            MODE_WAIT,                     // "待機"
            MODE_SAVE_BEFORE_WIFI_CONNECT, // "WiFiコネクション接続前・セーブ処理"
            MODE_WIFI_CONNECT,             // "WiFiコネクション接続"
            MODE_WIFI_DISCONNECT,          // "WiFiコネクション切断"
            MODE_GO_OUT_START,             // "おでかけ開始シーケンス"
            MODE_GO_OUT_END,               // "おでかけ終了シーケンス"
            MODE_DS_APPLI,                 // "DSアプリ開始"
            MODE_COME_IN_NET_HST, // "ネットホストの新規参入受け入れシーケンス"
            MODE_COME_IN_NET_VST, // "ネットビジターの新規参入受け入れシーケンス"
            MODE_GO_OUT_NET_VST,  // "ネットビジターとして通信開始シーケンス"
        };

        // npc_SP/NPC_gatekeeperA message codes (l_msgLabel), named from the
        // English text.
        enum {
            MSG_FIRST_MEETING = 0x01, // "Hello, and welcome to the checkpoint"
            MSG_HELLO_PART_TIME_JOB =
                0x02,                 // "unfinished business" (Nook's job)
            MSG_HELLO_MORNING = 0x03, // + dTime_c::getTimeOfDay() (0x03..0x06)
            MSG_RESIDENT_PROCESSING = 0x07, // "your resident information is
                                            // currently being processed"
            MSG_MENU_GATE_OPEN = 0x08,      // "Is there something I can do for
                                            // you?" go out / close the gate
            MSG_MENU = 0x09,             // go out / invite guests / friend code
            MSG_ASK_GO_HOME = 0x0C,      // "halt the Nintendo WFC session and
                                         // return home?" (net visitor)
            MSG_GO_HOME_REMINDER = 0x0D, // "When you go home, ... alert us"
            MSG_GO_HOME_START = 0x0E,    // "Understood! I will take care of
                                         // that"
            MSG_BUSY_IN_CONVERSATION =
                0x0F,                    // "unable to make preparations, as
                                         // there is a conversation in progress"
            MSG_GO_HOME_BYE = 0x10,      // "We wish you a good day!"
            MSG_NO_FRIENDS = 0x11,       // "you do not have anyone on your
                                         // friend roster" (invite guests)
            MSG_OPEN_GATE_START = 0x12,  // "I shall start the gate-opening
                                         // process"
            MSG_OPEN_GATE = 0x13,        // "I will open the gate!"
            MSG_FRIEND_GATE_OPEN = 0x14, // "a friend of yours already has his
                                         // or her gate open ... still want to
                                         // open your gate?"
            MSG_OPEN_GATE_HOORAH = 0x15, // "... I will open the gate! Hoo-rah!"
            MSG_SEARCH_GATE_START = 0x16, // "I will now search for a town with
                                          // an open gate..."
            MSG_FRIEND_CODE_SAVED = 0x1D, // "Nintendo WFC User Information ...
                                          // has been saved!"
            MSG_MENU_AGAIN = 0x1F,      // "Is there anything else I can do for
                                        // you?" (the MSG_MENU choices)
            MSG_CANT_CLOSE_GATE = 0x20, // "against policy to close the gate
                                        // when there is a visitor"
            MSG_CLOSE_GATE = 0x21,      // "I will close the gate at once!"
            MSG_NO_FRIEND_CODE = 0x23,  // "you have not yet been issued a
                                        // friend code"
            MSG_FRIEND_CODE = 0x25,     // "your friend code is as follows"
            MSG_ISSUE_FRIEND_CODE = 0x26, // "I will now make preparations to
                                          // issue you a friend code!"
            MSG_CANT_LEAVE = 0x2D,        // "While there is a visitor in town,
                                   // it is against regulations to leave!"
            MSG_GO_OUT_NO_FRIENDS = 0x2E, // "You need to register some
                                          // friends" (go out)
            MSG_GO_OUT_INSTEAD = 0x2F,    // "you would rather venture out than
                                          // have friends visit you?"
            MSG_SEARCH_GATE = 0x30, // "I will now search for a town with an
                                    // open gate..."
            MSG_FRIEND_CODE_SAVED2 = 0x31, // "... has been saved! I present
                                           // you ... with your friend code"
            MSG_TOWN_LIST = 0x33,          // "present the list of towns you can
                                           // visit!"
            MSG_SAFE_JOURNEY = 0x34,       // "Please have a safe and enjoyable
                                           // journey!"
            MSG_GO_OUT_CANCELLED = 0x35, // "you have decided against venturing
                                         // out?"
            MSG_TOWN_GATE_CLOSED = 0x36, // "the town ... did not have its gate
                                         // open!"
            MSG_TOWN_FULL = 0x37,        // "That town already has three
                                         // visitors."
            MSG_TOWN_BUSY = 0x38,        // "someone else must be trying to
                                         // enter at the exact same time"
            MSG_TOWN_NO_RESPONSE = 0x39, // "there is no response"
            MSG_TOWN_IN_CONVERSATION = 0x3A, // "there seems to be a
                                             // conversation in progress"
            MSG_TOWN_UNAVAILABLE = 0x3B, // "you cannot visit that town right
                                         // now"
            MSG_MENU_END = 0x3C,         // "Is that so? Well, if you need
                                         // anything, do not hesitate to ask!"
            MSG_MENU_GATE_OPEN2 = 0x3D,  // "Is there something I can do for
                                         // you at this time?" (gate open)
            MSG_MENU2 = 0x3E,            // "Is there anything else I can do
                                         // for you?"
            MSG_GO_OUT_NO_FRIEND_CODE = 0x3F, // "In order to venture out over
                                              // Nintendo WFC, you need ... a
                                              // friend code"
            MSG_ISSUE_FRIEND_CODE2 = 0x40,    // "Affirmative! I shall now make
                                              // preparations to issue you a
                                              // friend code!"
            MSG_FILL_OUT_FORMS = 0x41,        // "So, you want to go somewhere?"
                                              // (dAcNpcSpGkA_c::mAutoTalk)
            MSG_READY_TO_GO_HOME = 0x47,      // "Are you ready to go home?"
            MSG_GO_OUT_HOW = 0x51,     // "via Nintendo Wi-Fi Connection or via
                                       // the DS Suitcase?"
            MSG_DS_TURN_ON = 0x53,     // "turn the Nintendo DS on and then
                                       // activate DS Download Play!"
            MSG_DS_DOWNLOAD = 0x54,    // "Please download the DS Suitcase"
            MSG_DS_NOT_FOUND = 0x55,   // "I was unable to find a DS"
            MSG_DS_ERROR = 0x56,       // "an error seems to have occurred"
            MSG_DS_JOURNEY = 0x57,     // "have a pleasant journey!"
            MSG_DS_JOURNEY2 = 0x58,    // "Enjoy your trip!"
            MSG_DS_RETURN_HINT = 0x59, // "When you are ready to return,
                                       // please visit the checkpoint!"
            MSG_DS_COMMUNICATING = 0x5A,   // "Now communicating!"
            MSG_DS_ASK_QUIT = 0x5B,        // "Do you want to quit?"
            MSG_DS_CANCEL_HINT = 0x5C,     // "If you want to quit, ... cancel!"
            MSG_DS_TURN_ON_VISITOR = 0x5D, // "please turn on the Nintendo DS's
                                           // power and tap DS Download Play!"
            MSG_CONNECTING = 0x66,         // "Now connecting to Nintendo WFC."
            MSG_SEARCHING = 0x67, // "I will now search for a town with an
                                  // open gate."
            MSG_INQUIRING = 0x68, // "Submitting an inquiry to <town>..."
        };

        // What the Wi-Fi connection is made for (mConnectPurpose).
        enum {
            CONNECT_FRIEND_CODE,           // issue a friend code
            CONNECT_OPEN_GATE,             // open the gate
            CONNECT_FRIEND_CODE_OPEN_GATE, // issue a friend code, then open
                                           // the gate
            CONNECT_GO_OUT,                // search for a town to visit
        };

        // ---- talk_c's steps, in member order ----

        // Mode MODE_SAVE_BEFORE_WIFI_CONNECT: saves before connecting.
        class stepSaveBeforeWiFiConnect_c
            : public dSvStep_c<talk_c, stepSaveBeforeWiFiConnect_c> {
          public:
            stepSaveBeforeWiFiConnect_c() {
            }
            virtual ~stepSaveBeforeWiFiConnect_c() {
            }
            void finish(); // 0x86F8 "終了"
        }; // size 0x30

        // Mode MODE_WIFI_CONNECT.
        class stepWiFiConnect_c : public dSvStep_c<talk_c, stepWiFiConnect_c> {
          public:
            stepWiFiConnect_c() {
            }
            virtual ~stepWiFiConnect_c() {
            }
            void connect();     // 0x8A30 "WiFiコネクションに接続"
            void waitConnect(); // 0x8ACC "WiFiコネクションに接続確立"
            void finish();      // 0x8C1C "終了"
        }; // size 0x30

        // Mode MODE_GO_OUT_START: connects to the host as a visitor.
        class stepConnectNetVst_c
            : public dSvStep_c<talk_c, stepConnectNetVst_c> {
          public:
            stepConnectNetVst_c() {
            }
            virtual ~stepConnectNetVst_c() {
            }
            void startGuest();     // 0x9D44 "「子機としてマッチング」を要求"
            void requestConnect(); // 0x9E7C "親機に「デモのロック要求」を送信"
            void
            getConnectResult(); // 0x9F34
                                // "親機への「デモのロック要求」の結果を取得"
            void
            setupJoinSave(); // 0xA1C4
                             // "「新規参入時セーブデータ送受信処理」開始前の設定"
            void
            waitJoinPlayerSent(); // 0xA284
                                  // "「新規参入プレイヤー表示のためのデータ」の送信が終わっているか？"
            void
            requestTown(); // 0xA328
                           // "「新規参入マシンがセーブデータ要求」を送信要求セット"
        }; // size 0x30

        // Mode MODE_GO_OUT_END: the visitor goes home.
        class stepDisconnectNetVst_c
            : public dState::step_c<talk_c, stepDisconnectNetVst_c> {
          public:
            stepDisconnectNetVst_c() {
            }
            virtual ~stepDisconnectNetVst_c() {
            }
            void requestDisconnect(); // 0xA614 "デモのロック"
            void getRequestResult();  // 0xA6B8 "要求の結果"
            void checkSaveReady();    // 0xA8E8 "セーブ可能か調べる"
            void waitMsgEnd();        // 0xA98C "メッセージ終了待ち→デモへ"

            /* 0x30 */ u32 _30;
        }; // size 0x34

        // Mode MODE_DS_APPLI: sends the "go out" DS application and saves.
        class stepGoOutDsAppli_c
            : public dSvStep_c<talk_c, stepGoOutDsAppli_c> {
          public:
            stepGoOutDsAppli_c() : mTimer(0) {
            }
            virtual ~stepGoOutDsAppli_c() {
            }
            void sendApp();        // 0xAE90 "おでかけアプリ配信"
            void connect();        // 0xAF84 "接続チェック"
            void waitSendDone();   // 0xB124 "送信完了チェック"
            void waitLockStart();  // 0xB264 "ロック開始まで待つ"
            void notifySaveDone(); // 0xB310 "セーブ完了をDSに通知"
            void waitSent();       // 0xB428 "送信しきったか？"
            void setErrorMsg();    // 0xB568 "エラーメッセージをセット"
            void abortSave();  // 0xB5AC: mCurrentMethod = enableHomeAndReset
                               // (provisional)
            void startEndDs(); // 0xB5CC "DS通信終了開始"
            void endDs();      // 0xB670 "DS通信終了"

            /* 0x30 */ u32 mTimer;
        }; // size 0x34

        // Mode MODE_COME_IN_NET_HST.
        class stepDemoComeInNetHst_c
            : public stepDemoBase_c<talk_c, stepDemoComeInNetHst_c> {
          public:
            stepDemoComeInNetHst_c() {
            }
            virtual ~stepDemoComeInNetHst_c() {
            }
            void setupVisitor(); // 0xBAEC "初期化処理"
        }; // size 0x38

        // Mode MODE_COME_IN_NET_VST.
        class stepDemoComeInNetVst_c
            : public stepDemoBase_c<talk_c, stepDemoComeInNetVst_c> {
          public:
            stepDemoComeInNetVst_c() {
            }
            virtual ~stepDemoComeInNetVst_c() {
            }
        }; // size 0x38

        // Mode MODE_GO_OUT_NET_VST: the visitor joins the host's town.
        class stepDemoGoOutNetVst_c
            : public stepDemoBase_c<talk_c, stepDemoGoOutNetVst_c> {
          public:
            stepDemoGoOutNetVst_c() {
            }
            virtual ~stepDemoGoOutNetVst_c() {
            }
            BOOL searchFriend(int member); // 0xC1B8
            void searchFriend0();          // 0xC2A8 "ともだち情報で検索0"
            void searchFriend1();          // 0xC350 "ともだち情報で検索1"
            void searchFriend2();          // 0xC3F8 "ともだち情報で検索2"
            void searchFriend3();          // 0xC4A0 "ともだち情報で検索3"
            void
            waitJoinPlayersReady(); // 0xC548
                                    // "「村データ・プレイヤーデータ」の受け取りが終わっているか？"
            void
            callJoinCallback(); // 0xC5F4
                                // "「新規参入時に設定されたコールバック」を呼び出す（参入側）"
        }; // size 0x38

        talk_c(dAcNpcSpGkA_c *npc); // 0x5458
        virtual ~talk_c() {
        } // weak
        virtual void onMessageStart(int arg);     // 0x5B98
        virtual void onMessageEnd(int kind);      // 0x6070
        virtual void onAnswer(int answer);        // 0x7B70
        virtual void init();                      // 0x559C
        virtual void preExecute();                // 0x5950
        virtual void onTalkEnd();                 // 0x59A0
        virtual void getMsgInfo(msgInfo_s *info); // 0x562C
        virtual void onRequestEnd(int kind);      // 0x82B0

        void start(dActor_c *target);                     // 0x55E4
        void setNextMessage(u16 code, const char *label); // 0x59A4
        int selectNetMsg(); // 0x59F0: 8 while the net is active, else 9
                            // (provisional)
        static int
        getNetErrorMsg(); // 0x5A20: the NetworkError message of the error code
        int selectNetMsg2(); // 0x5B68: 0x3D while the net is active, else 0x3E
                             // (provisional)

        // ---- message handlers (onMessageStart / onMessageEnd / onAnswer
        // tables), named from the messages they follow ----
        void startDsAppli();        // 0x5C80: changeMode(MODE_DS_APPLI)
        void startWiFiDisconnect(); // 0x5DDC: changeMode(MODE_WIFI_DISCONNECT)
        void setFriendCodeWord();   // 0x5F38
        void setNetPlayerName();    // 0x5F74
        void setLeaveMemberName();  // 0x5FF4
        void switchToSystemSpeaker();     // 0x61D0 (request-end hideSpeaker)
        void switchToGatekeeperSpeaker(); // 0x627C (request-end showSpeaker)
        void startComeInNet();    // 0x6328: the talk mode of the owner's
                                  // MODE_COME_IN_NET demo
        void endVisitorLeftMsg(); // 0x65F8 (empty)
        void startBringBack();    // 0x65FC: owner changeMode(MODE_BRING_BACK)
        void endFirstMeeting();   // 0x6760
        void showMenu();          // 0x67D4: selectNetMsg()
        void checkDsConnected();  // 0x6818
        void checkDsAppliEnd();   // 0x68D4
        void startFriendCodeConnect(); // 0x6978: CONNECT_FRIEND_CODE
        void startConnect();           // 0x6984: save, then connect
        void startGoOutEnd();          // 0x69E8: changeMode(MODE_GO_OUT_END)
        void startRetire(); // 0x6B58: owner changeMode(MODE_RETIRE_NET_VST)
        void startGoOutConnect(); // 0x6CE4: CONNECT_GO_OUT
        void showTownList();      // 0x6CF0
        void openFriendMenu();    // 0x6D00: reqMenu19, end onFriendMenuEnd
        void cancelGoOut();       // 0x6D4C
        void startGoOutStart();   // 0x6D98: changeMode(MODE_GO_OUT_START)
        void
        startGoOutNetVstDemo(); // 0x6F08: owner changeMode(MODE_GO_OUT_NET_VST)
        void endTownBusy();     // 0x70C8
        void endTownRefused();  // 0x70CC
        void setDisconnectWaitIcon(); // 0x7118
        void startWiFiConnect();      // 0x7188: changeMode(MODE_WIFI_CONNECT)
        void endGoOutNoFriends();     // 0x72F8
        void startSaveBeforeWiFiConnect(); // 0x733C
        void startGoOutNetVst();           // 0x74B0
        void endNetRestricted();           // 0x765C
        void startOpenGateConnect();       // 0x76DC
        void
        startOpenGate(); // 0x772C: owner changeMode(MODE_OPEN_GATE_NET_HST)
        void
        startCloseGate(); // 0x7890: owner changeMode(MODE_CLOSE_GATE_NET_HST)
        void startGoOutDsAppliDemo();   // 0x7A00: owner
                                        // changeMode(MODE_GO_OUT_DS_APPLI)
        void answerSystemSpeaker();     // 0x7CA8
        void answerBackToMenu();        // 0x7CAC
        void answerGoOutInstead();      // 0x7CF0
        void answerGoOutWiFi();         // 0x7E68
        void answerGoOut();             // 0x7F18
        void answerInviteGuests();      // 0x7F9C
        void answerFriendCode();        // 0x8028
        void answerCloseGate();         // 0x80B4
        void answerDsRetry();           // 0x8114
        void answerRemindGoHome();      // 0x8174
        void formatFriendCode(u64 key); // 0x81BC
        void
        hideSpeaker(); // 0x8348: request-end method of switchToSystemSpeaker
        void showSpeaker();       // 0x8388: request-end method of
                                  // switchToGatekeeperSpeaker
        void onFriendMenuEnd();   // 0x8418: request-end method of
                                  // openFriendMenu
        void resetSpeakerFeel();  // 0x84E4: setSpeakerFeel(0)
        void resetSpeakerFeel2(); // 0x84F8: setSpeakerFeel(0)

        // ---- modes ----
        void initWait();                     // 0x850C
        void executeWait();                  // 0x8510
        void initSaveBeforeWiFiConnect();    // 0x8514
        void executeSaveBeforeWiFiConnect(); // 0x86B0
        void initWiFiConnect();              // 0x8898
        void executeWiFiConnect();           // 0x89E8
        void initWiFiDisconnect();           // 0x91F8
        void executeWiFiDisconnect();        // 0x91FC
        BOOL checkNetError(int arg);         // 0x938C
        void initGoOutStart();               // 0x9B24
        void executeGoOutStart();            // 0x9CFC
        void initGoOutEnd();                 // 0xA4D0
        void executeGoOutEnd();              // 0xA5CC
        void initDsAppli();                  // 0xAB20
        void executeDsAppli();               // 0xAE48
        void initComeInNetHst();             // 0xB824
        void executeComeInNetHst();          // 0xBAA4
        void initComeInNetVst();             // 0xBB90
        void executeComeInNetVst();          // 0xBDD8
        void initGoOutNetVst();              // 0xBE20
        void executeGoOutNetVst();           // 0xC170

        void endDemo(); // 0xC7EC: changeMode(MODE_WAIT)

        // The gatekeeper (stepDemoBase_c reaches it through its owner).
        dAcNpcSpGkA_c *getGk() {
            return mpOwner;
        }
        void setNetErrorMsg(); // 0xCDF0

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ stepSaveBeforeWiFiConnect_c mStepSaveBeforeWiFiConnect;
        /* 0x158 */ stepWiFiConnect_c mStepWiFiConnect;
        /* 0x188 */ stepConnectNetVst_c mStepConnectNetVst;
        /* 0x1B8 */ stepDisconnectNetVst_c mStepDisconnectNetVst;
        /* 0x1EC */ stepGoOutDsAppli_c mStepGoOutDsAppli;
        /* 0x220 */ stepDemoComeInNetHst_c mStepDemoComeInNetHst;
        /* 0x258 */ stepDemoComeInNetVst_c mStepDemoComeInNetVst;
        /* 0x290 */ stepDemoGoOutNetVst_c mStepDemoGoOutNetVst;
        /* 0x2C8 */ dAcNpcSpGkA_c *mpOwner;
        /* 0x2CC */ u8
            mSavedBeforeConnect;         // stepSaveBeforeWiFiConnect_c::finish
        /* 0x2CD */ u8 mFriendSlot;      // passed to startGuest
        /* 0x2CE */ u8 mLockRefused;     // the host refused the demo lock
        /* 0x2D0 */ int mConnectPurpose; // CONNECT_*
    }; // size 0x2D4

    dAcNpcSpGkA_c() : mTalkGk(this) {
    }
    virtual ~dAcNpcSpGkA_c() {
    } // weak
    virtual int create();                                  // 0x358
    virtual int preCreate();                               // 0x2AC
    virtual int doDelete();                                // 0xB90
    virtual void onDemoCmd(int cmd, u8 member);            // 0xCB8
    virtual int canSpeak(dDemoActor_c *actor);             // 0x15A0
    virtual void getName(dHmnName::Word_c *name, int len); // 0xCCBC
    virtual int getSoundId();                              // 0x15FC
    virtual int vtB0();                                    // 0xC54

    // ---- modes ----
    void initWait();               // 0x17D0
    void executeWait();            // 0x17E4
    void initDemoWait();           // 0x183C
    void executeDemoWait();        // 0x1840
    void initTalk();               // 0x1844
    void executeTalk();            // 0x18BC
    void initTurnBack();           // 0x18FC
    void executeTurnBack();        // 0x1944
    void initNone();               // 0x1AB8
    void executeNone();            // 0x1ABC
    void initRemote();             // 0x1AC0
    void executeRemote();          // 0x1AC4
    void initRemoteTalk();         // 0x1D98
    void executeRemoteTalk();      // 0x1D9C
    void initOpenGateNetHst();     // 0x1F18
    void executeOpenGateNetHst();  // 0x2030
    void initCloseGateNetHst();    // 0x2130
    void executeCloseGateNetHst(); // 0x222C
    void initGoOutNetVst();        // 0x23BC
    void executeGoOutNetVst();     // 0x26E4
    void initReunionNetHst();      // 0x2CE4
    void executeReunionNetHst();   // 0x2F80
    void initComeInNet();          // 0x306C
    void executeComeInNet();       // 0x3308
    void initReunionNetVst();      // 0x34A4
    void executeReunionNetVst();   // 0x3724
    void initRetireNetVst();       // 0x376C
    void executeRetireNetVst();    // 0x3A08
    void initSeeOffNetHst();       // 0x3E34
    void executeSeeOffNetHst();    // 0x41F4
    void initSeeOffNetVst();       // 0x4394
    void executeSeeOffNetVst();    // 0x46E4
    void initBringBack();          // 0x4878
    void executeBringBack();       // 0x4974
    BOOL isPlayerInGate();      // 0x4B48: net member 4 at z < 256 (provisional)
    void initGoOutDsAppli();    // 0x4BE0
    void executeGoOutDsAppli(); // 0x4EB4
    void initComeInDsAppli();   // 0x4F9C
    void executeComeInDsAppli(); // 0x521C

    void endDemo();            // 0xC690: changeMode(executeNone)
    void startReunionNetHst(); // 0xC948: changeMode(executeReunionNetHst),
                               // unreferenced
    void startReunionNetVst(); // 0xCAA4: changeMode(executeReunionNetVst),
                               // unreferenced
    BOOL isModeNone();         // 0xCC00, unreferenced
    static BOOL isAnyFriendInSession(); // 0xCC4C (provisional owner)
    bool isMode(Method method) const {
        return isCurrentMethod(method);
    }

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mTalkGk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    // The gatekeeper (stepDemoBase_c reaches it through its owner).
    dAcNpcSpGkA_c *getGk() {
        return this;
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpGkA_c> */
    /* 0x1CA0 */ stepDemoOpenGateNetHst_c mStepDemoOpenGateNetHst;
    /* 0x1CD8 */ stepDemoCloseGateNetHst_c mStepDemoCloseGateNetHst;
    /* 0x1D10 */ stepDemoGoOutNetVst_c mStepDemoGoOutNetVst;
    /* 0x1D48 */ stepDemoRetireNetVst_c mStepDemoRetireNetVst;
    /* 0x1D80 */ stepDemoGoOutDsAppli_c mStepDemoGoOutDsAppli;
    /* 0x1DB8 */ stepDemoReunionNetHst_c mStepDemoReunionNetHst;
    /* 0x1DF0 */ stepDemoReunionNetVst_c mStepDemoReunionNetVst;
    /* 0x1E28 */ stepDemoComeInNet_c mStepDemoComeInNet;
    /* 0x1E60 */ stepDemoComeInDsAppli_c mStepDemoComeInDsAppli;
    /* 0x1E98 */ stepDemoSeeOffNetHst_c mStepDemoSeeOffNetHst;
    /* 0x1ED0 */ stepDemoSeeOffNetVst_c mStepDemoSeeOffNetVst;
    /* 0x1F08 */ stepDemoBringBack_c mStepDemoBringBack;
    /* 0x1F40 */ u8 mDemoDone; // stepDemoBase_c::demoDone / waitDemoComplete
    /* 0x1F44 */ talk_c mTalkGk;
    /* 0x2218 */ u8 mDsDlActive; // vtB0 runs dDsDl::execute
    /* 0x2219 */ u8
        mAutoTalk; // executeWait: the talk starts by itself (getMsgInfo)
}; // size 0x221C

#endif
