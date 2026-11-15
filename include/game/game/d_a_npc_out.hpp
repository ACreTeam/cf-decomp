#ifndef GAME_D_A_NPC_OUT_HPP
#define GAME_D_A_NPC_OUT_HPP

#include <types.h>
#include <game/game/d_effect.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_a_npc_nml.hpp>
#include <game/mLib/m_vec.hpp>

class dFdBase_c;
class dInsFldBase_c;

namespace dANpcOut {
// Inferred names. The route helper has no RTTI; the actor's RTTI is
// dAcNpcOut_c.
class route_c {
  public:
    enum { NODE_COUNT = 100, INVALID_UNIT_COORD = -1, DIRECTION_COUNT = 4 };
    enum Direction_e {
        DIR_NONE = 0,
        DIR_NORTH = 1,
        DIR_WEST = 2,
        DIR_SOUTH = 4,
        DIR_EAST = 8,
        DIR_ALL = DIR_NORTH | DIR_WEST | DIR_SOUTH | DIR_EAST
    };
    struct node_s {
        void clear();                   // .text 3DF0
        void copy(const node_s *other); // .text 3E04
        BOOL isValid() const;           // .text 3E2C
        /* 0x0 */ u8 mX;
        /* 0x1 */ u8 mZ;
        /* 0x2 */ u8 mTriedDirs;
    };

    route_c();
    ~route_c();
    void init();                                             // 3CE4
    BOOL isValid() const;                                    // 3D34
    BOOL isDestination(int x, int z) const;                  // 3D80
    static BOOL isValidUnit(int x, int z);                   // 3E0C
    void clearNodes(u32 count);                              // 3E3C
    void clearNodes();                                       // 3E98
    node_s *getNode(u32 index);                              // 3EA0
    const node_s *getNode(u32 index) const;                  // 3EC0
    BOOL hasSegment() const;                                 // 3EE0
    void setSegment(int x0, int z0, int x1, int z1);         // 3F50
    void clearSegment();                                     // 3F64
    BOOL isSegmentEnd(int x, int z) const;                   // 3F78
    void setNextUnit(int x, int z);                          // 4008
    BOOL hasNextUnit() const;                                // 4014
    
    static int getRouteDirs(int x, int z, dFdBase_c *field); // 404C
    static int findStartUnit(int *outX, int *outZ, u32 x, u32 z,
                             u32 radius); // 40B4
    static int findStartPos(int *outX, int *outZ, const mVec3_c *pos,
                            u32 radius);   // 4240
    BOOL pushFront(const node_s *node);    // 42DC
    BOOL popFront(node_s *out, u32 count); // 4378
    BOOL popBack(node_s *out, u32 count);  // 4470
    int findNode(int x, int z) const;      // 4558
    u32 countNodes() const;                // 468C
    static int chooseDir(int dirs, int excluded, int x, int z, int destX,
                         int destZ);                                     // 46FC
    static int oppositeDir(int dir);                                     // 48B4
    static BOOL isDeadEnd(int dirs, int incoming);                       // 4900
    static BOOL isJunction(int dirs, int incoming);                      // 4950
    static int getDirTo(int x, int z, int destX, int destZ);             // 49AC
    static BOOL offsetUnit(int *outX, int *outZ, int x, int z, u32 dir); // 4A10
    static int advanceUnit(int *x, int *z, int dir);                     // 4A50
    int backtrack(int *x, int *z, int destX, int destZ,
                  dFdBase_c *field);                               // 4AE8
    static BOOL isInsideBlock(int x, int z, int unitX, int unitZ); // 4BE8
    static BOOL isInsideBlock(int x, int z, const mVec3_c *pos);   // 4C48
    BOOL search(int destX, int destZ, const mVec3_c *pos);         // 4C90
    int findRouteIndex(int x, int z);                              // 4F50
    void trimToPos(const mVec3_c *pos);                            // 5108
    void trimToNearbyPos(const mVec3_c *pos);                      // 522C

    /* 0x000 */ node_s mNodes[NODE_COUNT];
    /* 0x12C */ int mDestX;
    /* 0x130 */ int mDestZ;
    /* 0x134 */ int mSegmentX1;
    /* 0x138 */ int mSegmentZ1;
    /* 0x13C */ int mSegmentX0;
    /* 0x140 */ int mSegmentZ0;
    /* 0x144 */ int mNextX;
    /* 0x148 */ int mNextZ;
}; // size 0x14C; actor member +0x2AD8
} // namespace dANpcOut

namespace dInsectField {
void addNpcLink(void *, void *);
void removeNpcLink(void *, void *);
} // namespace dInsectField
class dAcNpcOut_c;
namespace dANpcOut {
// Behavior names and state IDs are inferred from the procedure table.
struct insectLink_s {
    ~insectLink_s() {
        if (mpInsect) {
            dInsectField::removeNpcLink(mpInsect, this);
        }
    }
    void *mpInsect;
    insectLink_s *mpNext;
};
class flea_c {
  public:
    enum State_e { STATE_ENABLED, STATE_DISABLED, STATE_UNINITIALIZED };
    static const u32 cEffectTimeBase;
    static const u32 cEffectTimeRange;
    flea_c();                         // FE8C
    ~flea_c();                        // FEE8
    void clear();                     // FF4C
    void init(dAcNpcOut_c *npc);      // FF60
    bool isEnabled(dAcNpcOut_c *npc); // 10090
    BOOL retire();                    // 100B4
    void execute(dAcNpcOut_c *npc);   // 100E4
    /* 0x00 */ int mState;
    /* 0x04 */ dEffect_c mEffect;
    /* 0x80 */ u32 mTimer;
}; // size 0x84
class action_c {
  public:
    static const float cObserveChance;
    static const u32 cObserveTimeRange;
    static const u32 cObserveTimeBase;
    static const float cInsectAcquireDistance;
    static const float cInsectTrackDistance;
    static const float cFishAcquireDistance;
    static const float cFishTrackDistance;
    static const float cLookAroundChance;
    static const u32 cLookAroundTimeBase;
    static const u32 cLookAroundTimeRange;
    static const u32 cLookFrontTimeBase;
    static const u32 cLookFrontTimeRange;
    static const u32 cVisitWaitTimeBase;
    static const u32 cVisitWaitTimeRange;
    static const float cApproachNoticeDistance;
    static const float cApproachHomeDistance;
    static const float cApproachPursueDistance;
    static const float cApproachTalkDistance;
    static const u32 cApproachWaitFrames;
    static const u32 cApproachLimitFrames;
    static const u32 cApproachTurnDistance;
    static const float cHalloweenAcquireDistance;
    static const float cHalloweenTakeDistance;
    static const float cHalloweenTrackDistance;
    static const float cHalloweenAskDistance;
    static const float cHalloweenTalkDistance;
    static const u32 cHalloweenWaitFrames;
    static const float cRemoteHalloweenTrackDistance;
    static const u32 cCooldownFrames;
    static const float cChatPartnerDistance;
    static const u32 cChatEmotionCount;

    enum Action_e {
        ACTION_WANDER = 0,
        ACTION_OFFSCREEN = 1,
        ACTION_REMOTE_HOUSE_WAIT = 2,
        ACTION_AFTER_TALK = 3,
        ACTION_FACE_PLAYER = 4,
        ACTION_HIDE_SEEK_INVITE = 5,
        ACTION_HIDE_SEEK_STANDBY = 6,
        ACTION_HIDE_SEEK_WAIT = 7,
        ACTION_HIDE_SEEK_PEEK = 8,
        ACTION_HIDE_SEEK_PLAY = 9,
        ACTION_HIDE_SEEK_WIN = 10,
        ACTION_HIDE_SEEK_LOSE = 11,
        ACTION_HIDE_SEEK_FINISH = 12,
        ACTION_REMOTE_HIDE_WAIT = 13,
        ACTION_REMOTE_HIDE_PEEK = 14,
        ACTION_HIDE_SEEK_FACE_PLAYER = 15,
        ACTION_HIDE_SEEK_AFTER_TALK = 16,
        ACTION_REMOTE_IDLE = 17,
        ACTION_REMOTE_CLOTH_WAIT = 18,
        ACTION_TOOL_CHANGE = 19,
        ACTION_REMOTE_TOOL_CHANGE = 20,
        ACTION_MASK_CHANGE = 21,
        ACTION_REMOTE_MASK_CHANGE = 22,
        ACTION_HALLOWEEN_CHASE = 23,
        ACTION_REMOTE_HALLOWEEN_WAIT = 24,
        ACTION_NPC_CHAT = 25,
        ACTION_NPC_CHAT_PARTNER = 26,
        ACTION_NPC_CHAT_AFTER_TALK = 27,
        ACTION_NPC_CHAT_BUSY = 28,
        ACTION_NPC_CHAT_RESERVED = 29,
        ACTION_ENTER_HOUSE = 30,
        ACTION_IN_HOUSE = 31,
        ACTION_LEAVE_HOUSE = 32,
        ACTION_BIRTHDAY_VISIT = 33,
        ACTION_AFTER_BIRTHDAY = 34,
        ACTION_VISIT_WAIT = 35,
        ACTION_PITFALL = 36,
        ACTION_PITFALL_TALK_RESERVED = 37,
        ACTION_AFTER_PITFALL_TALK = 38,
        ACTION_NET_HIT = 39,
        ACTION_NET_HIT_TALK_RESERVED = 40,
        ACTION_AFTER_NET_HIT_TALK = 41,
        ACTION_ITCHY = 42,
        ACTION_PUSHED = 43,
        ACTION_AFTER_PUSHED_TALK = 44,
        ACTION_APPROACH = 45,
        ACTION_AFTER_APPROACH_TALK = 46,
        ACTION_GREETING = 47,
        ACTION_WATCH_FIREWORKS = 48,
        ACTION_REMOTE_WATCH_FIREWORKS = 49,
        ACTION_REQUEST_CATCH = 50,
        ACTION_REMOTE_CLOTH_CHANGE = 51,
        ACTION_COUNT = 52
    };
    typedef void (action_c::*procFunc)(dAcNpcOut_c *);
    struct procPair_s {
        procFunc enter;
        procFunc execute;
    };
    action_c();  // 5318
    ~action_c(); // 535C
    void request(u32 action, u32 afterTalk,
                 const dAcNpcNml_c::talk_c::procSet_s *talk);  // 539C
    void setAction(dAcNpcOut_c *npc, u32 action);              // 53AC
    void execute(dAcNpcOut_c *npc);                            // 53FC
    u32 getMaskedIdleAction(dAcNpcOut_c *npc, BOOL allowJoin); // 5550
    u32 getMaskedWaitAction(dAcNpcOut_c *npc);                 // 5790
    void startCooldown();                                      // 5934
    void restoreAction(dAcNpcOut_c *npc);                      // 5940
    void enterWander(dAcNpcOut_c *npc);                        // .text 5B4C
    void executeWander(dAcNpcOut_c *npc);                      // .text 70B0
    void enterOffscreen(dAcNpcOut_c *npc);                     // .text 76E0
    void executeOffscreen(dAcNpcOut_c *npc);                   // .text 8268
    void enterRemoteHouseWait(dAcNpcOut_c *npc);               // .text 84F0
    void executeRemoteHouseWait(dAcNpcOut_c *npc);             // .text 85A0
    void enterAfterTalk(dAcNpcOut_c *npc);                     // .text 870C
    void stepAfterTalk00(dAcNpcOut_c *npc);                    // .text 87E0
    void executeAfterTalk(dAcNpcOut_c *npc);                   // .text 8868
    void enterFacePlayer(dAcNpcOut_c *npc);                    // .text 88A8
    void stepFacePlayer00(dAcNpcOut_c *npc);                   // .text 8958
    void executeFacePlayer(dAcNpcOut_c *npc);                  // .text 89BC
    void enterHideSeekInvite(dAcNpcOut_c *npc);                // .text 89FC
    void stepHideSeekInvite00(dAcNpcOut_c *npc);               // .text 8AD4
    void executeHideSeekInvite(dAcNpcOut_c *npc);              // .text 8B2C
    void enterHideSeekStandby(dAcNpcOut_c *npc);               // .text 8B6C
    void executeHideSeekStandby(dAcNpcOut_c *npc);             // .text 8C44
    void enterHideSeekWait(dAcNpcOut_c *npc);                  // .text 8CE4
    void stepHideSeekWait00(dAcNpcOut_c *npc);                 // .text 8E6C
    void stepHideSeekWait01(dAcNpcOut_c *npc);                 // .text 8EC0
    void executeHideSeekWait(dAcNpcOut_c *npc);                // .text 8FA8
    void enterHideSeekPeek(dAcNpcOut_c *npc);                  // .text 8FE8
    void executeHideSeekPeek(dAcNpcOut_c *npc);                // .text 9128
    void enterHideSeekPlay(dAcNpcOut_c *npc);                  // .text 91AC
    void executeHideSeekPlay(dAcNpcOut_c *npc);                // .text 9308
    void enterHideSeekWin(dAcNpcOut_c *npc);                   // .text 93C8
    void executeHideSeekWin(dAcNpcOut_c *npc);                 // .text 94DC
    void enterHideSeekLose(dAcNpcOut_c *npc);                  // .text 9534
    void executeHideSeekLose(dAcNpcOut_c *npc);                // .text 964C
    void enterHideSeekFinish(dAcNpcOut_c *npc);                // .text 96A4
    void executeHideSeekFinish(dAcNpcOut_c *npc);              // .text 97B8
    void enterRemoteHideWait(dAcNpcOut_c *npc);                // .text 9834
    void executeRemoteHideWait(dAcNpcOut_c *npc);              // .text 997C
    void enterRemoteHidePeek(dAcNpcOut_c *npc);                // .text 9AC8
    void executeRemoteHidePeek(dAcNpcOut_c *npc);              // .text 9BF8
    void enterHideSeekFacePlayer(dAcNpcOut_c *npc);            // .text 9CDC
    void stepHideSeekFacePlayer00(dAcNpcOut_c *npc);           // .text 9DF0
    void stepHideSeekFacePlayer01(dAcNpcOut_c *npc);           // .text 9E70
    void executeHideSeekFacePlayer(dAcNpcOut_c *npc);          // .text 9ED4
    void enterHideSeekAfterTalk(dAcNpcOut_c *npc);             // .text 9F14
    void stepHideSeekAfterTalk00(dAcNpcOut_c *npc);            // .text A02C
    void stepHideSeekAfterTalk01(dAcNpcOut_c *npc);            // .text A0B4
    void executeHideSeekAfterTalk(dAcNpcOut_c *npc);           // .text A184
    void enterRemoteClothWait(dAcNpcOut_c *npc);               // .text A1C4
    void executeRemoteClothWait(dAcNpcOut_c *npc);             // .text A284
    void enterToolChange(dAcNpcOut_c *npc);                    // .text A544
    void stepToolChange00(dAcNpcOut_c *npc);                   // .text A68C
    void stepToolChange01(dAcNpcOut_c *npc);                   // .text A764
    void stepToolChange02(dAcNpcOut_c *npc);                   // .text A898
    void stepToolChange03(dAcNpcOut_c *npc);                   // .text A964
    void stepToolChange04(dAcNpcOut_c *npc);                   // .text AA90
    void stepToolChange05(dAcNpcOut_c *npc);                   // .text AC48
    void stepToolChange06(dAcNpcOut_c *npc);                   // .text AD5C
    void stepToolChange07(dAcNpcOut_c *npc);                   // .text AE30
    void executeToolChange(dAcNpcOut_c *npc);                  // .text AEAC
    void enterRemoteToolChange(dAcNpcOut_c *npc);              // .text AEEC
    void stepRemoteToolChange00(dAcNpcOut_c *npc);             // .text AF74
    void stepRemoteToolChange01(dAcNpcOut_c *npc);             // .text B028
    void stepRemoteToolChange02(dAcNpcOut_c *npc);             // .text B108
    void stepRemoteToolChange03(dAcNpcOut_c *npc);             // .text B1BC
    void stepRemoteToolChange04(dAcNpcOut_c *npc);             // .text B29C
    void stepRemoteToolChange05(dAcNpcOut_c *npc);             // .text B350
    void stepRemoteToolChange06(dAcNpcOut_c *npc);             // .text B46C
    void stepRemoteToolChange07(dAcNpcOut_c *npc);             // .text B520
    void stepRemoteToolChange08(dAcNpcOut_c *npc);             // .text B664
    void stepRemoteToolChange09(dAcNpcOut_c *npc);             // .text B6A0
    void stepRemoteToolChange10(dAcNpcOut_c *npc);             // .text B70C
    void stepRemoteToolChange11(dAcNpcOut_c *npc);             // .text B748
    void stepRemoteToolChange12(dAcNpcOut_c *npc);             // .text B7B4
    void stepRemoteToolChange13(dAcNpcOut_c *npc);             // .text B7F0
    void stepRemoteToolChange14(dAcNpcOut_c *npc);             // .text B864
    void stepRemoteToolChange15(dAcNpcOut_c *npc);             // .text B8A0
    void executeRemoteToolChange(dAcNpcOut_c *npc);            // .text B91C
    void enterMaskChange(dAcNpcOut_c *npc);                    // .text BA64
    void stepMaskChange00(dAcNpcOut_c *npc);                   // .text BB1C
    void stepMaskChange01(dAcNpcOut_c *npc);                   // .text BBB0
    void executeMaskChange(dAcNpcOut_c *npc);                  // .text BC14
    void enterRemoteMaskChange(dAcNpcOut_c *npc);              // .text BC54
    void stepRemoteMaskChange00(dAcNpcOut_c *npc);             // .text BC8C
    void stepRemoteMaskChange01(dAcNpcOut_c *npc);             // .text BD40
    void stepRemoteMaskChange02(dAcNpcOut_c *npc);             // .text BE20
    void stepRemoteMaskChange03(dAcNpcOut_c *npc);             // .text BE5C
    void executeRemoteMaskChange(dAcNpcOut_c *npc);            // .text BEC0
    void enterHalloweenChase(dAcNpcOut_c *npc);                // .text BFFC
    void stepHalloweenChase00(dAcNpcOut_c *npc);               // .text C0CC
    void stepHalloweenChase01(dAcNpcOut_c *npc);               // .text C6E8
    void executeHalloweenChase(dAcNpcOut_c *npc);              // .text C7D8
    void enterRemoteHalloweenWait(dAcNpcOut_c *npc);           // .text C818
    void stepRemoteHalloweenWait00(dAcNpcOut_c *npc);          // .text C8FC
    void stepRemoteHalloweenWait01(dAcNpcOut_c *npc);          // .text CAFC
    void executeRemoteHalloweenWait(dAcNpcOut_c *npc);         // .text CC64
    void enterNpcChat(dAcNpcOut_c *npc);                       // .text CCA4
    void stepNpcChat00(dAcNpcOut_c *npc);                      // .text CE24
    void stepNpcChat01(dAcNpcOut_c *npc);                      // .text CF60
    void stepChatOutcome00(dAcNpcOut_c *npc);                  // .text D3A4
    void stepChatOutcome01(dAcNpcOut_c *npc);                  // .text D518
    void stepChatOutcome02(dAcNpcOut_c *npc);                  // .text D57C
    void stepChatOutcome03(dAcNpcOut_c *npc);                  // .text D9B4
    void stepChatOutcome04(dAcNpcOut_c *npc);                  // .text DC08
    void stepChatOutcome05(dAcNpcOut_c *npc);                  // .text DDD4
    void stepChatOutcome06(dAcNpcOut_c *npc);                  // .text E044
    void stepNpcChat02(dAcNpcOut_c *npc);                      // .text E190
    void stepNpcChat03(dAcNpcOut_c *npc);                      // .text E524
    void stepNpcChat04(dAcNpcOut_c *npc);                      // .text E578
    void executeNpcChat(dAcNpcOut_c *npc);                     // .text E648
    void enterNpcChatPartner(dAcNpcOut_c *npc);                // .text E688
    void stepNpcChatPartner00(dAcNpcOut_c *npc);               // .text E7FC
    void stepNpcChatPartner01(dAcNpcOut_c *npc);               // .text E91C
    void executeNpcChatPartner(dAcNpcOut_c *npc);              // .text EA90
    void enterNpcChatReserved(dAcNpcOut_c *npc);               // .text EAD0
    void executeNpcChatReserved(dAcNpcOut_c *npc);             // .text EAD4
    void enterNpcChatAfterTalk(dAcNpcOut_c *npc);              // .text EAD8
    void stepNpcChatAfterTalk00(dAcNpcOut_c *npc);             // .text EB34
    void executeNpcChatAfterTalk(dAcNpcOut_c *npc);            // .text EBA4
    void enterNpcChatBusy(dAcNpcOut_c *npc);                   // .text EBE4
    void executeNpcChatBusy(dAcNpcOut_c *npc);                 // .text ECAC
    void enterEnterHouse(dAcNpcOut_c *npc);                    // .text ECB0
    void stepEnterHouse00(dAcNpcOut_c *npc);                   // .text EE28
    void stepEnterHouse01(dAcNpcOut_c *npc);                   // .text EF3C
    void stepEnterHouse02(dAcNpcOut_c *npc);                   // .text EF78
    void stepEnterHouse03(dAcNpcOut_c *npc);                   // .text EFB4
    void stepEnterHouse04(dAcNpcOut_c *npc);                   // .text F088
    void executeEnterHouse(dAcNpcOut_c *npc);                  // .text F18C
    void enterInHouse(dAcNpcOut_c *npc);                       // .text F1CC
    void executeInHouse(dAcNpcOut_c *npc);                     // .text F314
    void enterLeaveHouse(dAcNpcOut_c *npc);                    // .text F528
    void stepLeaveHouse00(dAcNpcOut_c *npc);                   // .text F6EC
    void executeLeaveHouse(dAcNpcOut_c *npc);                  // .text F7B8
    void enterRemoteIdle(dAcNpcOut_c *npc);                    // .text F7F8
    void executeRemoteIdle(dAcNpcOut_c *npc);                  // .text F9A8
    void enterBirthdayVisit(dAcNpcOut_c *npc);                 // .text 10284
    void stepBirthdayVisit00(dAcNpcOut_c *npc);                // .text 10440
    void executeBirthdayVisit(dAcNpcOut_c *npc);               // .text 1044C
    void enterAfterBirthday(dAcNpcOut_c *npc);                 // .text 1048C
    void stepAfterBirthday00(dAcNpcOut_c *npc);                // .text 1058C
    void stepAfterBirthday01(dAcNpcOut_c *npc);                // .text 10620
    void executeAfterBirthday(dAcNpcOut_c *npc);               // .text 10694
    void enterVisitWait(dAcNpcOut_c *npc);                     // .text 106D4
    void executeVisitWait(dAcNpcOut_c *npc);                   // .text 10894
    void enterPitfall(dAcNpcOut_c *npc);                       // .text 10AA0
    void stepPitfall00(dAcNpcOut_c *npc);                      // .text 10C48
    void stepPitfall01(dAcNpcOut_c *npc);                      // .text 10D4C
    void stepPitfall02(dAcNpcOut_c *npc);                      // .text 10FDC
    void stepPitfall03(dAcNpcOut_c *npc);                      // .text 1120C
    void executePitfall(dAcNpcOut_c *npc);                     // .text 1127C
    void enterPitfallTalkReserved(dAcNpcOut_c *npc);           // .text 112BC
    void executePitfallTalkReserved(dAcNpcOut_c *npc);         // .text 112C0
    void enterAfterPitfallTalk(dAcNpcOut_c *npc);              // .text 1136C
    void stepAfterPitfallTalk00(dAcNpcOut_c *npc);             // .text 113C8
    void stepAfterPitfallTalk01(dAcNpcOut_c *npc);             // .text 114B0
    void executeAfterPitfallTalk(dAcNpcOut_c *npc);            // .text 11518
    void enterNetHit(dAcNpcOut_c *npc);                        // .text 11558
    void stepNetHit00(dAcNpcOut_c *npc);                       // .text 116D8
    void stepNetHit01(dAcNpcOut_c *npc);                       // .text 1182C
    void stepNetHit02(dAcNpcOut_c *npc);                       // .text 118AC
    void executeNetHit(dAcNpcOut_c *npc);                      // .text 11928
    void enterNetHitTalkReserved(dAcNpcOut_c *npc);            // .text 11968
    void executeNetHitTalkReserved(dAcNpcOut_c *npc);          // .text 1196C
    void enterAfterNetHitTalk(dAcNpcOut_c *npc);               // .text 11A44
    void stepAfterNetHitTalk00(dAcNpcOut_c *npc);              // .text 11B00
    void stepAfterNetHitTalk01(dAcNpcOut_c *npc);              // .text 11B9C
    void executeAfterNetHitTalk(dAcNpcOut_c *npc);             // .text 11C14
    void enterItchy(dAcNpcOut_c *npc);                         // .text 11C54
    void stepItchy00(dAcNpcOut_c *npc);                        // .text 11DA0
    void stepItchy01(dAcNpcOut_c *npc);                        // .text 11E34
    void executeItchy(dAcNpcOut_c *npc);                       // .text 11EA8
    void enterPushed(dAcNpcOut_c *npc);                        // .text 11EE8
    void stepPushed00(dAcNpcOut_c *npc);                       // .text 12030
    void stepPushed01(dAcNpcOut_c *npc);                       // .text 120C4
    void executePushed(dAcNpcOut_c *npc);                      // .text 12140
    void enterAfterPushedTalk(dAcNpcOut_c *npc);               // .text 12180
    void stepAfterPushedTalk00(dAcNpcOut_c *npc);              // .text 12210
    void executeAfterPushedTalk(dAcNpcOut_c *npc);             // .text 12280
    void enterApproach(dAcNpcOut_c *npc);                      // .text 122C0
    void stepApproach00(dAcNpcOut_c *npc);                     // .text 12604
    void stepApproach01(dAcNpcOut_c *npc);                     // .text 128E0
    void stepApproach02(dAcNpcOut_c *npc);                     // .text 12B90
    void executeApproach(dAcNpcOut_c *npc);                    // .text 12E9C
    void enterAfterApproachTalk(dAcNpcOut_c *npc);             // .text 12EDC
    void stepAfterApproachTalk00(dAcNpcOut_c *npc);            // .text 12F6C
    void executeAfterApproachTalk(dAcNpcOut_c *npc);           // .text 12FDC
    void enterGreeting(dAcNpcOut_c *npc);                      // .text 1301C
    void stepGreeting00(dAcNpcOut_c *npc);                     // .text 13160
    void stepGreeting01(dAcNpcOut_c *npc);                     // .text 13210
    void stepGreeting02(dAcNpcOut_c *npc);                     // .text 13274
    void executeGreeting(dAcNpcOut_c *npc);                    // .text 132E4
    void enterWatchFireworks(dAcNpcOut_c *npc);                // .text 13324
    void stepWatchFireworks00(dAcNpcOut_c *npc);               // .text 13460
    void stepWatchFireworks01(dAcNpcOut_c *npc);               // .text 13564
    void stepWatchFireworks02(dAcNpcOut_c *npc);               // .text 135F0
    void executeWatchFireworks(dAcNpcOut_c *npc);              // .text 13668
    void enterRemoteWatchFireworks(dAcNpcOut_c *npc);          // .text 136A8
    void executeRemoteWatchFireworks(dAcNpcOut_c *npc);        // .text 137F8
    void enterRequestCatch(dAcNpcOut_c *npc);                  // .text 13B1C
    void stepRequestCatch00(dAcNpcOut_c *npc);                 // .text 13C64
    void stepRequestCatch01(dAcNpcOut_c *npc);                 // .text 13CDC
    void executeRequestCatch(dAcNpcOut_c *npc);                // .text 13D58
    void enterRemoteClothChange(dAcNpcOut_c *npc);             // .text 13D98
    void stepRemoteClothChange00(dAcNpcOut_c *npc);            // .text 13E84
    void executeRemoteClothChange(dAcNpcOut_c *npc);           // .text 13EE0
    void startTalk(dAcNpcOut_c *npc);                          // .text 5480
    dAcNpcNml_c *findChatPartner(dAcNpcOut_c *npc);            // .text 5948
    void setMood(dAcNpcOut_c *npc, int mood, int hours,
                 BOOL pending);                               // .text 5AF0
    BOOL avoidObstacle(dAcNpcOut_c *npc);                     // .text 5CC0
    BOOL shouldChangeTool(dAcNpcOut_c *npc);                  // .text 5D80
    BOOL tryStartNpcChat(dAcNpcOut_c *npc);                   // .text 5EB4
    BOOL stopIfPushed(dAcNpcOut_c *npc);                      // .text 5F74
    bool tryReturnLostItem(dAcNpcOut_c *npc);                 // .text 6014
    u32 getHideSeekAction(dAcNpcOut_c *npc, BOOL allowJoin);  // .text 5568
    void saveDefaultAction(dAcNpcOut_c *npc, BOOL allowJoin); // .text 56D0
    u32 getHideSeekReturnAction(dAcNpcOut_c *npc);            // .text 57A8
    void saveHouseAction(dAcNpcOut_c *npc);                   // .text 58A0
    void initToolChange(dAcNpcOut_c *npc, u8 putTool, u8 closeUmbrella,
                        u8 takeTool, u8 openUmbrella); // .text A3D0
    BOOL takeOutTool(dAcNpcOut_c *npc);                // .text A5A4
    bool scaleOutTool(dAcNpcOut_c *npc);               // .text A6C8
    BOOL openUmbrella(dAcNpcOut_c *npc);               // .text A7D0
    BOOL finishOpeningUmbrella(dAcNpcOut_c *npc);      // .text A8D4
    BOOL putAwayTool(dAcNpcOut_c *npc);                // .text A9C0
    BOOL takeNextTool(dAcNpcOut_c *npc, u8 toolStep,
                      u8 umbrellaStep);   // .text AACC
    BOOL closeUmbrella(dAcNpcOut_c *npc); // .text ACBC
    BOOL finishToolChange(dAcNpcOut_c *npc, u8 putTool, u8 closeUmbrella,
                          u8 takeTool, u8 openUmbrella); // .text AD98
    void initMaskChange(dAcNpcOut_c *npc);               // .text B95C
    BOOL changeMaskAtFrame(dAcNpcOut_c *npc);            // .text BAA8
    BOOL isMaskChangeDone(dAcNpcOut_c *npc);             // .text BB58
    static int calcHideWait(dAnimal_c *animal);          // .text 8C48
    BOOL isApproachAnimationDone(dAcNpcOut_c *npc);      // .text 110D8
    void setApproachMood(dAcNpcOut_c *npc);              // .text 11164
    BOOL shouldChangeEventMode(dAcNpcOut_c *npc);        // .text 5E18
    BOOL isRemoteToolReady(dAcNpcOut_c *npc);            // .text F910
    BOOL escapePitfall(dAcNpcOut_c *npc);                // .text 10E84
    void initIdleChat(dAcNpcOut_c *npc);                 // .text BF00
    BOOL stopWander(dAcNpcOut_c *npc);                   // .text 10818
    BOOL canSeePlayer(dAcNpcOut_c *npc, float distance); // .text 1257C
    int getApproachAnm(dAcNpcOut_c *npc);                // .text 12838
    BOOL isNearStartPos(dAcNpcOut_c *npc);               // .text 12500
    int getApproachLoopAnm(dAcNpcOut_c *npc);            // .text 1288C
    static BOOL chooseChatLearner(dAcNpcOut_c **teacher, dAcNpcOut_c **learner,
                                  dAcNpcOut_c *npc); // .text D280
    BOOL canExchangeDesign(dAcNpcOut_c *npc);        // .text D92C
    BOOL canExchangeCloth(dAcNpcOut_c *npc);         // .text DB80
    static const dDesign_c *
    chooseTailorDesign(const dDesign_c *current);          // .text DE70
    BOOL canLearnTailorDesign(dAcNpcOut_c *npc);           // .text DF9C
    BOOL isChatPartnerNear(dAcNpcOut_c *npc);              // .text CED4
    BOOL shouldChangeLook(dAcNpcOut_c *npc);               // .text 6FF4
    static BOOL isPlayerShowingCatch();                    // .text 6074
    BOOL canRequestCatch(dAcNpcOut_c *npc);                // .text 6124
    BOOL tryIdleAction(dAcNpcOut_c *npc);                  // .text 622C
    BOOL findHomeWalkPos(mVec3_c *pos, dAcNpcOut_c *npc);  // .text 6420
    BOOL findRouteWalkPos(mVec3_c *pos, dAcNpcOut_c *npc); // .text 6604
    BOOL findBlockWalkPos(mVec3_c *pos, int x, int z,
                          dAcNpcOut_c *npc);               // .text 6CDC
    BOOL findHouseWalkPos(mVec3_c *pos, dAcNpcOut_c *npc); // .text 6D7C
    BOOL findWalkPos(mVec3_c *pos, dAcNpcOut_c *npc);      // .text 6E64
    BOOL tryGoHome(dAcNpcOut_c *npc);                      // .text 6F94
    static BOOL canStepUnit(int x, int z, int fromX, int fromZ,
                            u32 dir); // .text 77A8
    static BOOL findStepUnit(int *x, int *z, int destX, int destZ, int fromX,
                             int fromZ, int lastX, int lastZ); // .text 78B8
    BOOL stepAlongRoute(dAcNpcOut_c *npc);                     // .text 7B10
    BOOL stepTowardBlock(int x, int z, dAcNpcOut_c *npc);      // .text 7D0C
    BOOL stepNearby(dAcNpcOut_c *npc);                         // .text 7EB4
    BOOL stepOutsideView(dAcNpcOut_c *npc);                    // .text 818C
    /* 0x00 */ const procPair_s *mpProcs;
    /* 0x04 */ const dAcNpcNml_c::talk_c::procSet_s *mpTalkSet;
    /* 0x08 */ u32 mAction;
    /* 0x0C */ u32 mRequestedAction;
    /* 0x10 */ u32 mAfterTalkAction;
    /* 0x14 */ u32 mSavedAction;
    /* 0x18 */ u32 mChasePlayer;
    mVec3_c mStartPos;
    /* 0x28 */ u32 mTimer0;
    /* 0x2C */ u32 mTimer1;
    /* 0x30 */ u32 mCooldown;
    /* 0x34 */ u32 mBlockedFrames;
    /* 0x38 */ u32 mTimer2;
    /* 0x3C */ u32 mLookTimer;
    /* 0x40 */ u8 mStep;
    /* 0x41 */ u8 mInPitfall;
    /* 0x42 */ u8 mChatFlags[3];
    /* 0x45 */ u8 mWasShowingCatch;
    /* 0x46 */ u8 _46[2];
}; // size 0x48
extern const action_c::procPair_s sActionPairs[action_c::ACTION_COUNT];
} // namespace dANpcOut

// RTTI confirms the outdoor villager actor is at global scope.
class dAcNpcOut_c : public dAcNpcNml_c {
    friend class dANpcOut::action_c;
    friend class dANpcOut::flea_c;

  public:
    enum Mode_e { MODE_NORMAL, MODE_MASKED };
    enum { PENDING_PLACE_NONE = 5, TALK_WEIGHT_INTERVAL = 2700 };

    dANpcOut::route_c *getRoute() {
        return &mRoute;
    }

    class toolMng_c : public dAcNpc_c::toolBase_c {
      public:
        void create(dAcNpcOut_c *npc); // FDCC
        bool remove();                 // FE28
        toolMng_c();                   // FD20
        virtual ~toolMng_c();          // FD74
    };
    dAcNpcOut_c()
        : mPendingPlace(PENDING_PLACE_NONE), mPendingUnitX(-1),
          mPendingUnitZ(-1), mAlternateModel(this), mApproachCooldown(0),
          mTalkWeightTimer(0) {
        mInsectLink.mpInsect = NULL;
        mInsectLink.mpNext = NULL;
        mFishLink.mFish = NULL;
        mFishLink.mNext = NULL;
    }

    virtual void deleteReady();
    virtual bool createHeap();
    // Unreferenced inline virtuals are emitted in reverse declaration order.
    virtual ~dAcNpcOut_c() {
    }
    virtual toolBase_c *getTool() {
        return &mTool;
    } // 13FB4
    virtual int vtCC() {
        return mApproachActive;
    } // 13FAC: purpose not yet established, returns +2C4D
    virtual clothBase_c *getCloth() {
        return &mCloth;
    } // 13FA4
    BOOL syncTool();                               // E4C
    void stopEmotionManpu();                       // 14D8
    bool canChangeTalkWeight(int delta) const;     // 2E18
    void addTalkWeight(int delta);                 // 2F5C
    void halveTalkWeight();                        // 2FEC
    void clearTalkWeight();                        // 345C
    BOOL changeMask();                             // 1088
    BOOL canStartNpcChat();                        // 11FC
    BOOL checkLostItemPickup();                    // 2918
    virtual bool canInteract(dDemoActor_c *actor); // 2A8
    virtual int getEarType();                      // 1010
    virtual model_c *getModel();                   // 1038
    virtual const dItem::Item *getHoldItem();      // D74
    virtual int isItchy();                         // 2BF0
    virtual int vtC8(); // 2C00: purpose not yet established, returns +2C4C
    virtual int isInPitfall(); // 29CC
    model_c *getAlternateModel();                                        // 1060
    void setPendingPlace(int place);                                     // 2778
    void clearPendingPlace();                                            // 2780
    void sendPendingPlace();                                             // 2788
    void setPendingUnit(int x, int z);                                   // 27F0
    void clearPendingUnit();                                             // 285C
    BOOL isValidPendingUnit(int x, int z) const;                         // 2868
    bool hasPendingUnit() const;                                         // 2888
    void addNetHitCount();                                               // 2A74
    void clearNetHitCount();                                             // 2A68
    u8 getNetHitCount();                                                 // 2B50
    BOOL isNetHitLimit();                                                // 2B58
    void updateIdleTimer();                                              // 2D54
    int getGreetingAnm();                                                // 34CC
    static int getGreetingSound(int anm);                                // 35F0
    static mAng randomTurnAngle(const mAng *angle);                      // 3890
    BOOL canLeaveHouse() const;                                          // 1770
    BOOL getHousePos(mVec3_c *pos);                                      // 2218
    BOOL isPlayerNearHouse();                                            // 2684
    BOOL isInsectVisible(void *insect, const mAng *fov, float distance); // 3910
    BOOL isFishVisible(dFishFldShadow_c *fish, const mAng *fov,
                       float distance);              // 3AD8
    BOOL getHouseUnit(int *x, int *z);               // 21A0
    BOOL shouldGoHome();                             // 227C
    BOOL wantsToGoHome() const;                      // 1564
    BOOL needsMaskChange();                          // 1164
    BOOL wantsToApproach();                          // 3200
    void clearIdleTimer();                           // 2B94
    void addIdleTimer();                             // 2BA0
    void subtractIdleTimer();                        // 2BB8
    BOOL isIdleTimerExpired();                       // 2BD0
    dNpc::talkWeight_c *getTalkWeight();             // 2DE0
    const dNpc::talkWeight_c *getTalkWeight() const; // 2DFC

    virtual int canSpeech(dDemoActor_c *actor);                     // 2D4
    virtual void onDemoCmd(int cmd, u8 member);                     // 484
    virtual int create();                                           // 6BC
    virtual int preCreate();                                        // 65C
    virtual int doDelete();                                         // A18
    virtual int vtB0();                                             // AC0
    BOOL isAppointmentTime(int from, int to) const;                 // C14
    BOOL isHomeAppointmentTime() const;                             // D0C
    BOOL isOutdoorAppointmentTime() const;                          // D40
    BOOL takeInitialTool();                                         // D90
    BOOL updateHeldItem();                                          // F70
    virtual int startNpcChat(dAcNpc_c *partner);                    // 13E0
    virtual int endNpcChat();                                       // 1468
    virtual void vt80();                                            // 14F8
    virtual void vt84();                                            // 1508
    BOOL hasHouseVisitor() const;                                   // 1688
    virtual int vt68(const dItem::Item *item);                      // 29E0
    BOOL shouldChangeSpot() const;                                  // 183C
    BOOL wantsTownHallSpot();                                       // 1AB0
    BOOL chooseSpot();                                              // 1DD4
    BOOL searchSpotRoute();                                         // 1F1C
    BOOL isInSpotBlock();                                           // 1F9C
    BOOL finishSpotRoute();                                         // 202C
    BOOL isAtHomeSpot();                                            // 2128
    BOOL updateSpotRoute(BOOL checkDistance);                       // 2378
    void releasePendingUnit();                                      // 28BC
    BOOL updateTalkWeight();                                        // 3070
    BOOL canWatchFireworks();                                       // 3640
    BOOL isPlayerPushing();                                         // 2C08
    static BOOL isTurnAngleWide(const mAng *angle);                 // 3858
    static BOOL findInsectCallback(dInsFldBase_c *actor, void *arg); // 397C
    BOOL findInsect();                                              // 3A5C
    static BOOL findFishCallback(dFishFldShadow_c *actor, void *arg);   // 3B44
    BOOL findFish();                                                // 3C24
    /* 0x219A */ u8 _219A[2];
    /* 0x219C */ int mMode;
    /* 0x21A0 */ int mPendingPlace;
    /* 0x21A4 */ int mPendingUnitX;
    /* 0x21A8 */ int mPendingUnitZ;
    /* 0x21AC */ u32 mIdleTimer;
    /* 0x21B0 */ dANpcOut::action_c mOutAction;
    /* 0x21F8 */ viewArea_c mViewArea0;
    /* 0x2210 */ viewArea_c mViewArea1;
    /* 0x2228 */ toolMng_c mTool;
    /* 0x2958 */ model_c mAlternateModel;
    /* 0x2A54 */ dANpcOut::flea_c mFlea;
    /* 0x2AD8 */ dANpcOut::route_c mRoute;
    /* 0x2C24 */ u32 mApproachCooldown;
    /* 0x2C28 */ mVec3_c mTargetPos;
    /* 0x2C34 */ u32 mTalkWeightTimer;
    /* 0x2C38 */ dANpcOut::insectLink_s mInsectLink;
    /* 0x2C40 */ dFishLink_c mFishLink;
    /* 0x2C48 */ u8 mPlayerMoved;
    /* 0x2C49 */ u8 mBirthdayHost;
    /* 0x2C4A */ u8 mNetHitCount;
    /* 0x2C4B */ u8 mHitByNet;
    /* 0x2C4C */ u8 mChatOutcomePending;
    /* 0x2C4D */ u8 mApproachActive;
    /* 0x2C4E */ u8 mLostItemComplete;
    /* 0x2C4F */ u8 _2C4F;
}; // size 0x2C50

// Keep the leaf defaults in the final weak text section.
#include <game/game/d_a_npc_out_inline.hpp>

#endif
