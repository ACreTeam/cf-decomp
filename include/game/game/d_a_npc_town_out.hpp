#ifndef D_A_NPC_TOWN_OUT_HPP
#define D_A_NPC_TOWN_OUT_HPP

#include <game/game/d_a_npc_town.hpp>

// A moved-out villager strolling around the city square (REL
// d_a_npc_town_outNP, profile A_NPC_TOWN_OUT, spawned by
// dNpcMng_c::createCityAnimals). It wanders inside the square, talks, turns to
// the player and takes out or puts away its umbrella with the weather. Class
// names from RTTI ("dAcNpcTownOut_c", "dAcNpcTownOut_c::toolMng_c"); the rest
// is named from behaviour (the action table follows dANpcOut::action_c of
// d_a_npc_out).

class dAcNpcTownOut_c;
namespace dANpcTownOut {
class action_c {
  public:
    enum Action_e {
        ACTION_WANDER,      // stroll inside the square
        ACTION_TALK,        // talk to the player (DEMO_CMD_TALK)
        ACTION_FACE_PLAYER, // turn to the player (DEMO_CMD_START)
        ACTION_TOOL_CHANGE, // take out / put away the held tool (umbrella)
        ACTION_NUM,
        ACTION_NONE = ACTION_NUM
    };
    // mStep of ACTION_TOOL_CHANGE (l_toolChangeSteps).
    enum ToolChangeStep_e {
        TOOL_TAKE_OUT,
        TOOL_SCALE_OUT,
        TOOL_OPEN_UMBRELLA,
        TOOL_FINISH_OPEN_UMBRELLA,
        TOOL_PUT_AWAY,
        TOOL_TAKE_NEXT,
        TOOL_CLOSE_UMBRELLA,
        TOOL_FINISH_CHANGE
    };

    typedef void (action_c::*procFunc)(dAcNpcTownOut_c *);
    struct procPair_s {
        procFunc enter;
        procFunc execute;
    };

    static const u32 cWaitTimeBase;
    static const u32 cWaitTimeRange;

    action_c();                                       // 7E4
    ~action_c();                                      // 808
    void setAction(dAcNpcTownOut_c *npc, u32 action); // 848
    void execute(dAcNpcTownOut_c *npc);               // 898

    void enterWander(dAcNpcTownOut_c *npc);      // 8E0
    BOOL checkDetour(dAcNpcTownOut_c *npc);      // 970
    BOOL checkBlocked(dAcNpcTownOut_c *npc);     // A30
    BOOL shouldChangeTool(dAcNpcTownOut_c *npc); // AD0
    BOOL tryIdleAction(dAcNpcTownOut_c *npc);    // B68
    void executeWander(dAcNpcTownOut_c *npc);    // C10

    void enterTalk(dAcNpcTownOut_c *npc);   // F6C
    void stepTalk00(dAcNpcTownOut_c *npc);  // 1000
    void executeTalk(dAcNpcTownOut_c *npc); // 1058

    void enterFacePlayer(dAcNpcTownOut_c *npc);   // 1098
    void stepFacePlayer00(dAcNpcTownOut_c *npc);  // 10F8
    void executeFacePlayer(dAcNpcTownOut_c *npc); // 115C

    void initToolChange(dAcNpcTownOut_c *npc, u8 putTool, u8 closeUmbrellaStep,
                        u8 takeTool,
                        u8 openUmbrellaStep);         // 119C
    void enterToolChange(dAcNpcTownOut_c *npc);       // 12E0
    BOOL takeOutTool(dAcNpcTownOut_c *npc);           // 1328
    void stepToolChange00(dAcNpcTownOut_c *npc);      // 1410
    BOOL scaleOutTool(dAcNpcTownOut_c *npc);          // 144C
    void stepToolChange01(dAcNpcTownOut_c *npc);      // 14E8
    BOOL openUmbrella(dAcNpcTownOut_c *npc);          // 1538
    void stepToolChange02(dAcNpcTownOut_c *npc);      // 1600
    BOOL finishOpeningUmbrella(dAcNpcTownOut_c *npc); // 163C
    void stepToolChange03(dAcNpcTownOut_c *npc);      // 16CC
    BOOL putAwayTool(dAcNpcTownOut_c *npc);           // 171C
    void stepToolChange04(dAcNpcTownOut_c *npc);      // 17EC
    BOOL takeNextTool(dAcNpcTownOut_c *npc, u8 toolStep,
                      u8 umbrellaStep);          // 1828
    void stepToolChange05(dAcNpcTownOut_c *npc); // 19A4
    BOOL closeUmbrella(dAcNpcTownOut_c *npc);    // 19FC
    void stepToolChange06(dAcNpcTownOut_c *npc); // 1A9C
    BOOL finishToolChange(dAcNpcTownOut_c *npc, u8 putTool,
                          u8 closeUmbrellaStep, u8 takeTool,
                          u8 openUmbrellaStep);   // 1AD8
    void stepToolChange07(dAcNpcTownOut_c *npc);  // 1B70
    void executeToolChange(dAcNpcTownOut_c *npc); // 1BD0

    /* 0x00 */ const procPair_s *mpProcs;
    /* 0x04 */ u32 mAction;
    /* 0x08 */ u32 mWaitTimer; // frames left standing before the next walk
    /* 0x0C */ u32 mLookTimer; // frames left before the next look-around change
    /* 0x10 */ u8 mStep;
    /* 0x14 */ u32 mBlockedFrames; // frames walking into a wall
}; // size 0x18
} // namespace dANpcTownOut

class dAcNpcTownOut_c : public dAcNpcTown_c {
    friend class dANpcTownOut::action_c;

  public:
    class toolMng_c : public dAcNpc_c::toolBase_c {
      public:
        toolMng_c();                       // 678
        virtual ~toolMng_c();              // 6CC
        void create(dAcNpcTownOut_c *npc); // 724
        bool remove();                     // 780
    };

    virtual bool canInteract(dDemoActor_c *actor); // 1E0
    virtual void onDemoCmd(int cmd, u8 member);    // 248
    virtual int preCreate();                       // 320
    virtual int create();                          // 36C
    virtual int doDelete();                        // 408
    virtual int vtB0();                            // 45C
    virtual const dItem::Item *getHoldItem();      // 488
    // Unreferenced inline virtuals are emitted in reverse declaration order.
    virtual ~dAcNpcTownOut_c() {
    } // 1C5C
    virtual toolBase_c *getTool() {
        return &mTool;
    } // 1C54

    BOOL takeInitialTool(); // 4A4
    BOOL updateUmbrella(); // 560: the held item follows the rain (now and then)

    /* 0x219C */ dANpcTownOut::action_c mTownAction;
    /* 0x21B4 */ toolMng_c mTool;
}; // size 0x28E4

#endif
