#ifndef D_A_NPC_SP_MISSING_CHILD_HPP
#define D_A_NPC_SP_MISSING_CHILD_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_msg_rcpt_exp.hpp>
#include <game/game/d_state.hpp>

// Katie, the lost kitten (REL d_a_npc_sp_missing_childNP, profile
// A_NPC_SP_MISSING_CHILD, item key SP_NPC_MISSING_CHILD). She cries in town,
// follows the player who agrees to take her home, falls over now and then,
// comes out of the gate and is reunited with her mother (SP_NPC_MISSING_MOTHER)
// in the checkpoint scene, alone or over the net.
//
// Class names from RTTI ("dAcNpcSpMissingChild_c", its talk_c, the step
// classes and "dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>"); mode and
// step names from the original CP932 state labels; everything else is named
// from behaviour.
//
// Declaration order matters:
// - .data holds the vtables in reverse class-completion order: the actor,
//   talk_c, stepDemoReunionNet_c, stepDemoReunion_c, stepFall_c, stepGate_c,
//   then mode_c<dAcNpcSpMissingChild_c>. So the nested classes are declared
//   stepGate_c, stepFall_c, stepDemoReunion_c, stepDemoReunionNet_c, talk_c.
// - stepDemoBase_c is instantiated for stepDemoReunionNet_c first, then for
//   stepDemoReunion_c; within an instantiation its members come out in
//   declaration order.
class dAcNpcSpMissingChild_c : public dAcNpcSp_c,
                               public dState::mode_c<dAcNpcSpMissingChild_c> {
  public:
    // dAcNpc_c::action_c request priorities.
    enum Priority_e { PRIORITY_IDLE = 0, PRIORITY_DEMO = 1 };

    enum ReunionTiming_e {
        REUNION_EXIT_WALK_FRAMES = 60,
        REUNION_SMILE_DELAY_FRAMES = 20,
        REUNION_SMILE_HOLD_FRAMES = 120,
        REUNION_SCENE_CHANGE_DELAY_FRAMES = 20,
    };

    typedef dState::base_c<dAcNpcSpMissingChild_c>::Method Method;

    class talk_c;

    // The modes (the table in create), in table order.
    enum {
        MODE_WAIT,             // "待機"
        MODE_WAIT_PLAYER,      // "プレイヤ生成待機" (wait for the player)
        MODE_TURN,             // "回転"
        MODE_DEMO_WAIT,        // "デモ待機"
        MODE_TALK,             // "会話"
        MODE_FOLLOW_WALK,      // "プレーヤー追従歩き"
        MODE_FOLLOW_RUN,       // "プレーヤー追従走り"
        MODE_CRY,              // "泣き"
        MODE_SOB,              // "べそかき"
        MODE_FALL,             // "転ぶ"
        MODE_COME_OUT_OF_GATE, // "門から出てくる"
        MODE_NONE,             // "処理なし"
        MODE_REUNION,          // "再会デモ"
        MODE_REUNION_NET,      // "再会デモ（通信）"
    };

    // Mode MODE_COME_OUT_OF_GATE: walk out of the gate in two legs.
    class stepGate_c
        : public dState::step_c<dAcNpcSpMissingChild_c, stepGate_c> {
      public:
        stepGate_c() {
        }
        virtual ~stepGate_c() {
        }
        void startWalk1(); // "歩き1開始"
        void waitWalk1();  // "歩き1終了"
        void startWalk2(); // "歩き2開始"
        void waitWalk2();  // "歩き2終了"
    }; // size 0x30

    // Mode MODE_FALL: fall over, then get up.
    class stepFall_c
        : public dState::step_c<dAcNpcSpMissingChild_c, stepFall_c> {
      public:
        stepFall_c() {
        }
        virtual ~stepFall_c() {
        }
        void initFallAnm();  // "転びアニメ初期化"
        void waitFallAnm();  // "転びアニメ終了待ち"
        void initGetUpAnm(); // "起き上がりアニメ初期化"
        void waitGetUpAnm(); // "起き上がりアニメ終了待ち"

        /* 0x30 */ mVec3_c mFallPos; // where the fall ends
        /* 0x3C */ f32 mFallSpeed;   // per frame, to reach mFallPos
    }; // size 0x40

    // The reunion demo steps shared by stepDemoReunion_c and
    // stepDemoReunionNet_c (RTTI
    // "dAcNpcSpMissingChild_c::stepDemoBase_c<Owner, Step>"). The step
    // methods are named from the labels of the step tables that use them.
    template <class Owner, class Step>
    class stepDemoBase_c : public dState::step_c<Owner, Step> {
      public:
        void startWalk();            // "歩き開始"
        void waitWalk();             // "歩き終了"
        void waitPlayerWalk();       // "プレイヤ歩き終了"
        void waitFadeIn();           // "フェードイン終了待ち"
        void finish();               // "終了"
        void startTalk();            // "会話開始"
        void waitTalkSuspend();      // "会話サスペンドまで待つ"
        void restartTalk();          // "会話再開"
        void startSideMove();        // "再会サイド移動開始"
        void waitSideMove();         // "再会サイド移動終了"
        void startReunion();         // "再会開始"
        void turnChildToMother();    // "子ネコが母ネコの方向へ回転"
        void turnMotherToChild();    // "母ネコが子ネコの方向へ回転"
        void turnPlayer();           // "プレイヤ回転"
        void finishReunion();        // "再会終了"
        void finishTalk();           // "会話終了"
        void turnToMotherSide();     // "母ネコの側方向へ回転"
        void waitTurnToMotherSide(); // "母ネコの側方向へ回転終了"
        void moveToMotherSide();     // "母ネコの側方向へ移動"
        void waitMotherTurn();       // "母ネコ回転終了"
        void waitMoveToMotherSide(); // "母ネコの側方向へ移動終了"
        void turnToExit();           // "出口方向へ回転"
        void waitTurnToExit();       // "出口方向へ回転終了"
        void moveToExit();           // "出口方向へ移動"
        void smileChild();           // "子ネコ笑顔"
        void smileMother();          // "母ネコ笑顔"
        void waitSmile();            // "笑顔終了"
        void waitSceneChange();      // "シーン切り替え待ち終了"
        void changeScene();          // "元のシーンへ" / "門の外のシーンへ"
        static int getPlayerNo();    // the net member of the player

        stepDemoBase_c() : mTimer(0) {
        }
        virtual ~stepDemoBase_c() {
        }

        /* 0x30 */ u32 mTimer;
    }; // size 0x34

    // Mode MODE_REUNION: the reunion with her mother.
    class stepDemoReunion_c
        : public stepDemoBase_c<dAcNpcSpMissingChild_c, stepDemoReunion_c> {
      public:
        // changeScene: the exit to the scene outside the gate.
        enum { EXIT_IDX = 0 };

        stepDemoReunion_c() {
        }
        virtual ~stepDemoReunion_c() {
        }
    };

    // Mode MODE_REUNION_NET: the reunion over the net; the gatekeeper saves.
    class stepDemoReunionNet_c
        : public stepDemoBase_c<dAcNpcSpMissingChild_c, stepDemoReunionNet_c> {
      public:
        // changeScene: the exit back to the original scene.
        enum { EXIT_IDX = 1 };

        stepDemoReunionNet_c() {
        }
        virtual ~stepDemoReunionNet_c() {
        }
        void startGatekeeperSave(); // "もんばんにセーブを任せる"
        void waitGatekeeperSave();  // "もんばんにセーブおわった？"
    };

    // The message receptor (RTTI "dAcNpcSpMissingChild_c::talk_c").
    class talk_c : public dAcNpcSp_c::receptSp_c, public dMsgRcptExp_c<talk_c> {
      public:
        typedef dState::base_c<talk_c>::Method Method;

        // What getMsgInfo starts (mMsgKind).
        enum {
            MSG_KIND_LOST,    // she is lost in town
            MSG_KIND_FELL,    // she fell over
            MSG_KIND_FOLLOW,  // she follows the player
            MSG_KIND_REUNION, // the reunion with her mother
        };

        // npc_SP/NPC_maigo_girl message codes (l_msgLabel), named from the
        // English text.
        enum {
            MSG_FIRST_MEETING = 0x01,       // "I can't find my mommy..."
            MSG_FIRST_MEETING_AGAIN = 0x02, // the same, after being brought
                                            // home once
            MSG_TAKE_HOME = 0x03,           // the player takes her home
            MSG_REFUSE = 0x04,              // the player leaves her
            MSG_MEET = 0x05,                // later meetings
            MSG_MEET_AGAIN = 0x06,
            MSG_FELL = 0x07,    // after falling over
            MSG_FOLLOW = 0x08,  // while following
            MSG_REUNION = 0x29, // "M-Mommy!"
        };

        talk_c(dAcNpcSpMissingChild_c *npc);      // 0x450C
        virtual ~talk_c();                        // 0x4574
        virtual void onMessageStart(int arg);     // 0x478C
        virtual void onMessageEnd(int kind);      // 0x47D0
        virtual void onAnswer(int answer);        // 0x48E8
        virtual void init();                      // 0x45CC
        virtual void preExecute();                // 0x48EC
        virtual void onTalkEnd();                 // 0x48F0
        virtual void getMsgInfo(msgInfo_s *info); // 0x465C
        virtual void onRequestEnd(int kind);      // 0x48F4
        void start(dActor_c *target);             // 0x4614

        // ---- message handlers (onMessageEnd table) ----
        void startEscort(); // 0x4890: she follows the player
        void stopEscort();  // 0x48B8
        void endReunion();  // 0x48E0

        /* 0x0EC dMsgRcptExp_c<talk_c> */
        /* 0x128 */ dAcNpcSpMissingChild_c *mpOwner;
        /* 0x12C */ int mMsgKind; // MSG_KIND_*
    }; // size 0x130

    dAcNpcSpMissingChild_c() : mChildTalk(this) {
    }
    virtual ~dAcNpcSpMissingChild_c() {
    } // weak
    virtual int create();                          // 0x280
    virtual int preCreate();                       // 0x218
    virtual int doDelete();                        // 0xD78
    virtual void onDemoCmd(int cmd, u8 member);    // 0xE14
    virtual bool canInteract(dDemoActor_c *actor); // 0x4A6C
    virtual int getEarType();                      // 0x4CB8
    virtual int vtB0();                            // 0xDC0

    // ---- modes ----
    void initWait();             // 0x1524
    void executeWait();          // 0x15A0
    void initWaitPlayer();       // 0x134C
    void executeWaitPlayer();    // 0x1358
    void initTurn();             // 0x1868
    void executeTurn();          // 0x18FC
    void initDemoWait();         // 0x1BD8
    void executeDemoWait();      // 0x1C74
    void initTalk();             // 0x1CA4
    void executeTalk();          // 0x1D58
    void initFollowWalk();       // 0x1D98
    void executeFollowWalk();    // 0x1E40
    void initFollowRun();        // 0x2524
    void executeFollowRun();     // 0x25F0 "プレーヤー追従走り"
    void initCry();              // 0x2CF4
    void executeCry();           // 0x2D5C
    void initSob();              // 0x2D60
    void executeSob();           // 0x2DC8
    void initFall();             // 0x2DCC
    void executeFall();          // 0x2EE8 "転ぶ"
    void initComeOutOfGate();    // 0x34D0
    void executeComeOutOfGate(); // 0x360C
    void initNone();             // 0x3A9C
    void executeNone();          // 0x3AA0
    void initReunion();          // 0x3AA4
    void executeReunion();       // 0x3F30
    void initReunionNet();       // 0x3F78
    void executeReunionNet();    // 0x443C

    void walkToGate(); // 0x4484: walk to (240, 0, 256)
    void calcFrontPos(mVec3_c *pos, const mAng &angle,
                      f32 dist); // 0x498C: on the ground, dist ahead
    void runToMotherSide();      // 0x4CC0: run to (224, 0, 288)
    void runToMother();          // 0x4D6C: run to (224, 0, 336)

    // stepFall_c moves her (dActor_c's position is protected).
    mVec3_c *getPosPtr() {
        return &mPos;
    }
    void setLastPos() {
        mLastPos = mPos;
    }

    // initTalk: the angle to the talk partner and the turn speed.
    mAng getTalkAngle() const {
        dActor_c *target = mChildTalk.mpActActor;
        mAng angle(0);
        if (target) {
            angle = getAngleYTo(target);
        }
        return angle;
    }
    mAng getTalkTurnSpeed() const {
        return l_turnSpeed;
    }

    // The end of the reunion demos (stepDemoBase_c::finish).
    void endDemo() {
        changeMode(&dAcNpcSpMissingChild_c::executeNone);
    }

    // Used by the gatekeeper when the player leaves with Katie.
    void startFollowRun() {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowRun);
    }
    void startFall() {
        changeMode(&dAcNpcSpMissingChild_c::executeFall);
    }

    /* 0x1C7C dState::mode_c<dAcNpcSpMissingChild_c> */
    /* 0x1CA0 */ stepGate_c mStepGate;
    /* 0x1CD0 */ stepFall_c mStepFall;
    /* 0x1D10 */ stepDemoReunion_c mStepDemoReunion;
    /* 0x1D44 */ stepDemoReunionNet_c mStepDemoReunionNet;
    /* 0x1D78 */ talk_c mChildTalk;
}; // size 0x1EA8

#endif
