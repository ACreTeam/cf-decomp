#pragma once

// NPC / villager management. Source: src/dol/game/d_npc.cpp (.text 800ECD90..800F59C8,
// sinit 800F5768; built with -sym on). RTTI supplies the message, list and manager
// class names. Other class, method and field names are inferred from their uses.

#include <game/game/d_field_info.hpp>
#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_quest.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_msg_def.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_net.hpp>

class dAnimal_c;
class dPrivateData_c;

class dActor_c;
class dPlayerActor_c;
namespace dScript { class Word_c; }
namespace EGG { class Heap; }

namespace dNpc {

// Inferred placement policy for foreground object 0x94 (a hole).
enum holeCheck_e {
    IGNORE_HOLES,
    CHECK_HOLES
};

// 0x48. A remembered message (group label + index).
class msgMemory_c {
public:
    // Who set the topic (talk_c::setTopic callers).
    enum kind_e {
        KIND_NONE,
        KIND_REACTION, // d_a_npc_nml msgReaction (group = reaction index)
        KIND_FREE,     // d_npc_talk_free
        KIND_ARBEIT,   // d_npc_talk_arbeit
        KIND_QUEST,    // d_a_npc_nml quest talk and the d_npc_talk_quest_* units (group = quest)
        KIND_ANY       // talk_c's reset value; matches every kind in getRememberedLabel
    };

    msgMemory_c();                                                              // 800ECD90
    virtual ~msgMemory_c();                                                     // 800ECDA0
    virtual void clear();                                                       // 800ECDE0

    BOOL isValid() const;                                                       // 800ECE34
    void set(const char *group, u16 msgId, u32 kind, u8 topicGroup, u8 topicIdx); // 800ECE80

    /* 0x04 */ u16 mMsgId;
    /* 0x06 */ char mGroup[0x3D];
    /* 0x43 */ u8 mKind; // kind_e
    /* 0x44 */ u8 mTopicGroup;
    /* 0x45 */ u8 mTopicIdx;
}; // size 0x48

// 0xD8. A message plus who it's about.
class msgMemorySecond_c : public msgMemory_c {
public:
    msgMemorySecond_c();                                                        // 800ECEF4
    virtual ~msgMemorySecond_c();                                               // 800ECF40
    virtual void clear();                                                       // 800ECF98

    BOOL setJinxTown(int value);                                                // 800ED01C
    BOOL setPersonal(const dPersonalID_c *pid);                                 // 800ED06C
    BOOL setPlayer(const dPlayerID_c *player);                                  // 800ED0C8
    BOOL setLand(const dLandID_c *land);                                        // 800ED124
    BOOL setRumorTown(u32 value);                                               // 800ED180: < 4
    BOOL setText(const wchar_t *text);                                          // 800ED19C
    BOOL setImpression(u32 value);                                              // 800ED224: < 50

    /* 0x46 */ dPersonalID_c mPersonal;
    /* 0x72 */ dPlayerID_c mPlayer;
    /* 0x88 */ dLandID_c mLand;
    /* 0x9E */ u8 mPadding9E[2];
    /* 0xA0 */ s32 mJinxTown;
    /* 0xA4 */ s32 mRumorTown;
    /* 0xA8 */ s32 mMessageYear;
    /* 0xAC */ u32 mImpression; // 50 when cleared
    /* 0xB0 */ dItem::Item mItem0;
    /* 0xB2 */ dItem::Item mItem1;
    /* 0xB4 */ wchar_t mText[17];
}; // size 0xD8

// 0x11E at entry_c+0x120. Cached word IDs for seven dialogue categories and six villager slots.
class dialogueCache_c {
public:
    dialogueCache_c();                                                          // 800EDB00
    ~dialogueCache_c();                                                         // 800EDB04

    void clear();                                                               // 800EDB44
    void clearAnimalIndices();                                                  // 800EDBB0
    void setWord(u32 i, u16 value, u32 j);                                      // 800EDBD0
    u16 getWord(u32 i, u32 j);                                                  // 800EDBF4
    int getAnimalIdx(u32 i);                                                    // 800EDC20
    void setAnimalIdx(u32 i, int value);                                        // 800EDC40
    dAnimal_c *pickAnimal(u32 i, const dAnmPersonalID_c **exclude, u32 numExclude); // 800EDC54

    /* 0x000 */ u16 mWordIds[MSG_RANDOM_WORD_KIND_NUM][MSG_WORD_NUM]; // drink, object, sweets, sports, music, food, hobby
    /* 0x118 */ s8 mAnimalIndices[6]; // -1 when cleared; indexed up to 10
}; // size 0x11E

// 0x10 at entry_c+0x250. A countdown: a mode, a frame count and its expiration tick.
class timer_c {
public:
    timer_c();                                                                  // 800ED240
    ~timer_c();                                                                 // 800ED244

    void clear();                                                               // 800ED284
    void reset();                                                               // 800ED29C: mode 5
    void addFrames(u32 frameCount);                                             // 800ED2B8: saturates at 0xFFFF
    void set(int mode, u16 frameCount);                                         // 800ED2D8
    void copy(const timer_c *other);                                            // 800ED4C0
    void tick();                                                                // 800ED50C
    int getMode();                                                              // 800ED544
    void update();                                                              // 800ED590

    /* 0x0 */ s64 mEndTick;
    /* 0x8 */ u32 mMode;
    /* 0xC */ u16 mFrames;
}; // size 0x10

// 0x1 at entry_c+0x288. A 0-32 talk weight scaled by friendship and reduced after talking.
class talkWeight_c {
public:
    talkWeight_c();                                                             // 800ED690
    ~talkWeight_c();                                                            // 800ED694

    void clear();                                                               // 800ED6D4
    void initFromFriendship(dAnimal_c *animal, dPrivateData_c *player);         // 800ED6E0
    static int calcFriendshipValue(u32 max, u32 min, dAnimal_c *animal, dPrivateData_c *player); // 800ED7D4
    void set(u32 value);                                                        // 800ED92C: clamped to 32
    void add(int delta);                                                        // 800ED940
    void half();                                                                // 800ED964
    void twice();                                                               // 800ED970

    /* 0x0 */ u8 mWeight;
}; // size 0x1

// 0x290. Everything the game remembers about one villager.
class entry_c {
public:
    entry_c();                                                                  // 800ED97C
    ~entry_c();                                                                 // 800ED9D4

    void clear();                                                               // 800EDA6C

    /* 0x000 */ msgMemory_c mTopicMemory;
    /* 0x048 */ msgMemorySecond_c mRememberedMessage;
    /* 0x120 */ dialogueCache_c mDialogueCache;
    /* 0x23E */ dQuestBase_c mQuest;
    /* 0x24C */ u8 mPadding24C[4];
    /* 0x250 */ timer_c mMoodTimer;
    /* 0x260 */ dTime_c mTime;
    /* 0x288 */ talkWeight_c mTalkWeight;
    /* 0x289 */ u8 mStallBuyCount;    // the npc bought at the player's flea market stall
    /* 0x28A */ u8 mStallRefuseCount; // the player refused the npc's offer at the stall
    /* 0x28B */ u8 mPadding28B;
    /* 0x28C */ struct flags_s {
        u16 mTalked : 1;      // 0x8000: a talk ended since the actor was created (talk_c::onTalkEnd; cleared by
                              //         dAcNpcNml_c::create). Read by msgGreeting (first-talk
                              //         greeting), the quest offer talk and q13; quest offers are rarer.
        u16 mEventTalked : 1; // 0x4000: the event's first message was given since the actor was created
                              //         (bug, fishing, carnival, fmarket; cleared by dAcNpcNml_c::create)
        u16 mFmarketSale : 1; // 0x2000: a flea-market sale was agreed (selFmarketAsk) and not finished yet
        u16 mBirthdayDone : 1; // 0x1000: this player congratulated the npc on its birthday (d_npc_talk_birthday)
        u16 mFlag11 : 1; // 0x0800
        u16 mPoisonDone : 1;  // 0x0400: the poison reaction was given (d_npc_talk_reaction stepPoison)
        u16 mFlag9 : 1;  // 0x0200
        u16 mFlag8 : 1;  // 0x0100
        u16 mFlag7 : 1;  // 0x0080
        u16 mFlag6 : 1;  // 0x0040
        u16 mFlag5 : 1;  // 0x0020
        u16 mFlag4 : 1;  // 0x0010
        u16 mFlag3 : 1;  // 0x0008
        u16 mFlag2 : 1;  // 0x0004
        u16 mFlag1 : 1;  // 0x0002
        u16 mFlag0 : 1;  // 0x0001
    } mFlags;              // cleared by memset in clear(); mTalked / mEventTalked also by dAcNpcNml_c create
    /* 0x28E */ u8 mPadding28E[2];
}; // size 0x290

// Villager compatibility.

u32 getPersonalityCompatibility(u8 a, u8 b);                                    // 800EDF18: personality x personality
u32 getStarSignElement(u32 sign);                                               // 800EDF48: star sign -> element
u32 getStarSignCompatibility(u32 signA, u32 signB);                             // 800EDF68
u32 getBirthdayCompatibility(int monthA, int dayA, int monthB, int dayB);       // 800EDFD4
const u8 *findSpeciesPair(u32 a, u32 b, const u8 *pairs, int num);              // 800EE030
u32 getSpeciesCompatibility(u32 a, u32 b);                                      // 800EE094: species x species
u32 calcAnimalCompatibilityScore(dAnimal_c *a, dAnimal_c *b);                   // 800EE138
int calcAnimalCompatibilityRank(dAnimal_c *a, dAnimal_c *b);                    // 800EE244: 0 good .. 2 bad

// 0x3040. A dAnimal_c held outside the town.
class bufferedAnimal_c {
public:
    bufferedAnimal_c();                                                         // 800EE290
    ~bufferedAnimal_c();                                                        // 800EE2C0

    void clear();                                                               // 800EE318

    /* 0x0000 */ dAnimal_c mAnimal;
}; // size 0x3040

// 0xC100 (lbl_805C1000). Four spare villagers.
class animalBuffer_c {
public:
    animalBuffer_c();                                                           // 800EE31C
    ~animalBuffer_c();                                                          // 800EE36C

    void clear();                                                               // 800EE3D0
    static BOOL isValidIdx(u32 i);                                              // 800EE41C
    bufferedAnimal_c *get(int i);                                               // 800EE434
    BOOL takeMovedAnimal(int arg);                                              // 800EE440
    BOOL storeMovedAnimal();                                                    // 800EE4AC
    void exchangeMovedAnimal();                                                 // 800EE514
    BOOL exchangeWithRandomMember(const dAnimal_c *src, u32 idx);               // 800EE5C4
    BOOL setFirst(const dAnimal_c *src);                                        // 800EE71C
    u32 take(dAnimal_c *dst, u32 idx);                                          // 800EE76C
    bufferedAnimal_c *getValid(u32 idx);                                        // 800EE818
    BOOL clearAt(u32 idx);                                                      // 800EE890

    /* 0x0000 */ bufferedAnimal_c mAnimals[NET_MEMBER_MAX];
}; // size 0xC100

// 0x2F packed record, copied byte-wise (network data).
class daub_c {
public:
    daub_c();                                                                   // 800EE8F0
    daub_c(const daub_c &other);                                                // 800EE8F4
    ~daub_c();                                                                  // 800EE924

    void clear();                                                               // 800EE964
    void copy(const daub_c *other);                                             // 800EE9AC
    static void packXZ(u8 *dst, const mVec3_c *src);                            // 800EEA88
    static void unpackXZ(const u8 *src, mVec3_c *dst);                          // 800EEAD4
    void setAngleY(const u16 *value);                                           // 800EEB28
    mAng getAngleY();                                                           // 800EEB34
    void setHeadPitch(mAng value);                                              // 800EEB6C
    mAng getHeadPitch();                                                        // 800EEB78
    void setHeadYaw(mAng value);                                                // 800EEBB0
    mAng getHeadYaw();                                                          // 800EEBBC
    void getNpcData(void *dst, u32 size);                                       // 800EEBF4
    void setNpcData(const void *src, u32 size);                                 // 800EEC2C
    void getAction(u32 *action, u32 *priority);                                 // 800EEC5C
    void setAction(u32 action, u32 priority);                                   // 800EEC80
    void getMove(mVec3_c *target, mVec3_c *goal, u16 *turnSpeed, f32 *morph);   // 800EEC8C
    void setMove(const mVec3_c *target, const mVec3_c *goal, const u16 *turnSpeed, f32 morph); // 800EED5C
    void getTalk(u16 *angle, u16 *turnSpeed, f32 *morph);                       // 800EEE10
    void setTalk(const u16 *angle, const u16 *turnSpeed, f32 morph);            // 800EEEA8
    void getAct(u32 *animation, u8 *playMode, f32 *morph, f32 *frame);          // 800EEF14
    void setAct(u32 animation, u8 playMode, f32 morph, f32 frame);              // 800EEFDC

    /* 0x00 */ u8 mPosX[4];
    /* 0x04 */ u8 mPosZ[4];
    /* 0x08 */ u8 mAngleY[2];
    /* 0x0A */ u8 _0A; // Unidentified byte, initialized to 4; only clear/copy uses recovered.
    /* 0x0B */ u8 _0B; // Unidentified byte, initialized to 4; only clear/copy uses recovered.
    /* 0x0C */ u8 mScene; // dScene::SCENE_NUM when cleared
    /* 0x0D */ u8 mHeadPitch[2];
    /* 0x0F */ u8 mHeadYaw[2];
    /* 0x11 */ u8 mNpcData[8];
    /* 0x19 */ u8 mActionData[0x15]; // action, priority, then action-specific parameters
    /* 0x2E */ u8 mValid : 1;
    /* 0x2E */ u8 _2E : 7; // Unidentified packed flag bits.
}; // size 0x2F

// Base of the daub managers. Keys map to an index via getIdx; ids are getBase() + index.
class daubMng_c {
public:
    daubMng_c(daub_c *daubs, u32 num);                                          // 800EF060
    virtual ~daubMng_c();                                                       // 800EF078
    virtual int getIdx(const u16 *key) = 0;
    virtual int getBase() = 0;

    void clear();                                                               // 800EF0C4
    void init();                                                                // 800EF124
    BOOL isValidIdx(u32 idx);                                                   // 800EF128
    BOOL isValidKey(const u16 *key);                                            // 800EF140
    BOOL isValidId(int id);                                                     // 800EF184
    int keyToId(const u16 *key);                                                // 800EF1BC: NET_REC_NUM when invalid
    int idToIdx(int id);                                                        // 800EF234
    daub_c *get(u32 idx);                                                       // 800EF298
    daub_c *getByKey(const u16 *key);                                           // 800EF2EC
    daub_c *getById(int id);                                                    // 800EF330

    /* 0x4 */ daub_c *mDaubs;
    /* 0x8 */ u32 mDaubCount;
}; // size 0xC

// 10 daubs, ids 0x2B.. (lbl_805CD10C).
class daubMngNormal_c : public daubMng_c {
public:
    daubMngNormal_c();                                                          // 800EF368
    virtual ~daubMngNormal_c();                                                 // 800EF3CC
    virtual int getIdx(const u16 *key);                                         // 800EF440
    virtual int getBase();                                      // 800F5760

    void initHousePositions();                                                  // 800EF4B8

    /* 0x00C */ daub_c mStorage[ANIMAL_NUM];
}; // size 0x1E4

// 97 daubs, ids 0x35.. (lbl_805CD2FC).
class daubMngSpecial_c : public daubMng_c {
public:
    daubMngSpecial_c();                                                         // 800EF57C
    virtual ~daubMngSpecial_c();                                                // 800EF5E0
    virtual int getIdx(const u16 *key);                                         // 800EF654
    virtual int getBase();                                      // 800F5758

    void initSpecialPositions();                                                // 800EF768

    /* 0x00C */ daub_c mStorage[NPC_SPECIAL_NUM];
}; // size 0x11DC

// Keep the list classes after the daub managers for weak-method emission order.
// A list of entry_c.
class list_c {
public:
    list_c(entry_c *entries, u32 num) : mEntries(entries), mEntryCount(num) {}
    virtual ~list_c();                                                        // 800F5718
    virtual void clear();                                                       // 800EDE0C
    virtual void init();                                                        // 800EDE74: clear()

    entry_c *get(u32 i);                                                        // 800EDE84

    /* 0x4 */ entry_c *mEntries;
    /* 0x8 */ u32 mEntryCount;
    /* 0xC */ u8 _C[4]; // Unidentified storage; no recovered accesses.
}; // size 0x10

// The town's villager entries plus the time they were last updated (lbl_805BF610).
class listLand_c : public list_c {
public:
    listLand_c() : list_c(mStorage, ANIMAL_NUM) {}
    virtual ~listLand_c();                                                    // 800F56B0
    virtual void clear();                                                       // 800EDEA8

    void setTimeNow();                                                          // 800EDEDC

    /* 0x0010 */ entry_c mStorage[ANIMAL_NUM];
    /* 0x19B0 */ dTime_c mTime;
}; // size 0x19D8

// A scene's actor placements (dScene::getSceneData; dScene::dSceneActorData_c in d_scene.hpp), as used by getLayoutPlacement.
struct layoutActor_s {
    /* 0x00 */ u8 mProfileData[4]; // u16 actor profile followed by alignment bytes
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ u8 mAngleX[2];
    /* 0x12 */ s16 mAngleY;
    /* 0x14 */ u8 mAngleZData[4]; // s16 Z angle followed by alignment bytes
    /* 0x18 */ u32 mNpcKey;
}; // size 0x1C

struct layoutGroup_s {
    /* 0x0 */ u16 mType;
    /* 0x2 */ u16 mEntryCount;
    /* 0x4 */ layoutActor_s *mEntries;
}; // size 0x8

struct layout_s {
    /* 0x0 */ u16 mGroupCount;
    /* 0x4 */ layoutGroup_s *mGroups;
};

BOOL getLayoutPlacement(mVec3_c *pos, s16 *angle, const dItem::Item &key, u8 layout); // 800EF670

// Daub manager access (lbl_8074AE88: the normal and special managers).
daubMng_c *getDaubManager(u32 i);                                               // 800EFBBC
daubMng_c *getDaubManagerByKey(const u16 *key);                                 // 800EFBDC
daubMng_c *getDaubManagerById(int id);                                          // 800EFC54
void initDaubManagers();                                                        // 800EFCCC
daub_c *getDaubByKey(const u16 *key);                                           // 800EFD04
void initNormalDaubs();                                                         // 800EFD48
void copyNormalDaubRecord(void *dst, int id);                                   // 800EFDD0
void initSpecialDaubs();                                                        // 800EFDD4
void copySpecialDaubRecord(void *dst, int id);                                  // 800EFE68
const daub_c *getReceivedDaub(const u16 *key);                                  // 800EFE6C: the received daub copy
void copyDaubRecord(void *dst, int id);                                         // 800EFEDC
void parseAnimalHeldItemPacket(int *idx, void *dst, const u8 *src);             // 800EFF60
void buildAnimalHeldItemPacket(u8 *dst, int idx, const void *src);              // 800EFF8C
void sendAnimalHeldItem(u32 idx, const void *data);                             // 800EFFA0

// Network messages (types 0x55..0x82): parse / build / send triples.
void parseAnimalHeldItemTimePacket(int *idx, u8 *value, const u8 *src);         // 800F0020
void buildAnimalHeldItemTimePacket(u8 *dst, int idx, int value);                // 800F0048
void sendAnimalHeldItemTime(u32 idx, u8 value);                                 // 800F0054
void parseAnimalEventFlagsPacket(int *idx, u8 *value, const u8 *src);           // 800F00D4
void buildAnimalEventFlagsPacket(u8 *dst, int idx, int value);                  // 800F00FC
void sendAnimalEventFlags(u32 idx, u8 value);                                   // 800F0108
void parseAnimalMemoryPacket(int *idx, int *kind, u8 *count, u16 *msg, u8 *flag, const u8 *src); // 800F0188
void buildAnimalMemoryPacket(u8 *dst, int idx, int kind, int count, int msg, int flag); // 800F01F0
void sendAnimalMemoryInit(u32 idx, u32 kind, u8 count, u8 msg, s8 flag);        // 800F0208
void sendAnimalMemorySet(u32 idx, u32 kind, u8 count, u8 msg, s8 flag);         // 800F02A8
void parseAnimalFriendshipPacket(int *idx, int *kind, u8 *value, const u8 *src); // 800F0348
void buildAnimalFriendshipPacket(u8 *dst, int idx, int kind, int value);        // 800F038C
void sendAnimalFriendship(u32 idx, u32 kind, s8 value);                         // 800F039C
void sendAnimalFriendshipById(const dAnmPersonalID_c *animal, u32 kind, s8 value); // 800F0434
void parseAnimalMemoryFlagOnPacket(int *idx, int *kind, int *value, const u8 *src); // 800F0494
void buildAnimalMemoryFlagOnPacket(u8 *dst, int idx, int kind, int value);      // 800F04D8
void sendAnimalMemoryFlagOn(u32 idx, u32 kind, u8 value);                       // 800F04E8
void parseAnimalEventTalkedPacket(int *idx, int *kind, const u8 *src);          // 800F0580
void buildAnimalEventTalkedPacket(u8 *dst, int idx, int kind);                  // 800F05BC
void sendAnimalEventTalked(u32 idx, u32 kind);                                  // 800F05C8
void sendHideAndSeekClear();                                                    // 800F0650
void sendHideAndSeekState(u8 value);                                            // 800F06A8
void sendHideAndSeekFlag(int value);                                            // 800F0708
void parseAnimalFtrTouchPacket(u8 *kind, int *x, int *z, const u8 *src);        // 800F0770
void buildAnimalFtrTouchPacket(u8 *dst, int kind, int x, int z);                // 800F080C
void sendAnimalFtrTouch(int kind, int x, int z);                                // 800F0864
void parseAnimalFtrPacket(int *idx, void *item, int *count, const u8 *src);     // 800F08EC
void buildAnimalFtrPacket(u8 *dst, int idx, const void *item, int count);       // 800F0968
void sendAnimalFtr(int idx, const void *item, int count);                       // 800F09B4
void parseAnimalPlacePacket(int *idx, int *mode, const u8 *src);                // 800F0A3C
void buildAnimalPlacePacket(u8 *dst, int idx, int mode);                        // 800F0A78
void sendAnimalPlace(int idx, int mode);                                        // 800F0A84
void parseAnimalSpotPacket(int *idx, int *a, int *b, u32 *c, const u8 *src);    // 800F0AFC
void buildAnimalSpotPacket(u8 *dst, int idx, int a, int b, int c);              // 800F0B50
void sendAnimalSpot(int idx, int a, int b, int c);                              // 800F0B64: int params (d_animal callers pass ints untruncated)
void parseAnimalSpotTimePacket(int *idx, u32 *kind, void *data, const u8 *src); // 800F0BFC
void buildAnimalSpotTimePacket(u8 *dst, int idx, int kind, const void *data);   // 800F0C44
void sendAnimalSpotTime(int idx, int kind, const void *data);                   // 800F0C5C
void parseAnimalSpotClearPacket(int *idx, const u8 *src);                       // 800F0CE4
void buildAnimalSpotClearPacket(u8 *dst, int idx);                              // 800F0D04
void sendAnimalSpotClear(int idx);                                              // 800F0D0C
void parseAnimalFtrSetPacket(int *idx, int *count, void *item, const u8 *src);  // 800F0D74
void buildAnimalFtrSetPacket(u8 *dst, int idx, int count, const void *item);    // 800F0DBC
void sendAnimalFtrSet(int idx, int count, const void *item);                    // 800F0DD4
void parseAnimalNewItemAddPacket(int *idx, void *item, const u8 *src);          // 800F0E5C
void buildAnimalNewItemAddPacket(u8 *dst, int idx, const void *item);           // 800F0E88
void sendAnimalNewItemAdd(int idx, const void *item);                           // 800F0E9C
void parseAnimalNewItemClearPacket(int *idx, const u8 *src);                    // 800F0F14
void buildAnimalNewItemClearPacket(u8 *dst, int idx);                           // 800F0F34
void sendAnimalNewItemClear(int idx);                                           // 800F0F3C
void parseAnimalNewItemRemovePacket(int *idx, void *item, const u8 *src);       // 800F0FA4
void buildAnimalNewItemRemovePacket(u8 *dst, int idx, const void *item);        // 800F0FD0
void sendAnimalNewItemRemove(int idx, const void *item);                        // 800F0FE4
void parseAnimalWallPacket(int *idx, void *item, const u8 *src);                // 800F105C
void buildAnimalWallPacket(u8 *dst, int idx, const void *item);                 // 800F1088
void sendAnimalWall(int idx, const void *item);                                 // 800F109C
void parseAnimalCarpetPacket(int *idx, void *item, const u8 *src);              // 800F1114
void buildAnimalCarpetPacket(u8 *dst, int idx, const void *item);               // 800F1140
void sendAnimalCarpet(int idx, const void *item);                               // 800F1154
void parseAnimalMusicPacket(int *idx, void *item, const u8 *src);               // 800F11CC
void buildAnimalMusicPacket(u8 *dst, int idx, const void *item);                // 800F11F8
void sendAnimalMusic(int idx, const void *item);                                // 800F120C
void parseAnimalQuestStatePacket(int *idx, int *kind, u8 *value, const u8 *src); // 800F1284
void buildAnimalQuestStatePacket(u8 *dst, int idx, int kind, int value);        // 800F12C8
void sendAnimalQuestState(int idx, int kind, int value);                        // 800F12D8
void parseAnimalQuestClearPacket(int *idx, const u8 *src);                      // 800F1360
void buildAnimalQuestClearPacket(u8 *dst, int idx);                             // 800F1380
void sendAnimalQuestClear(int idx);                                             // 800F1388
void parseAnimalPlaceTimeClearPacket(int *idx, const u8 *src);                  // 800F13F0
void buildAnimalPlaceTimeClearPacket(u8 *dst, int idx);                         // 800F1410
void sendAnimalPlaceTimeClear(int idx);                                         // 800F1418
void parseAnimalPresentPacket(int *idx, int *kind, void *item, const u8 *src);  // 800F1480
void buildAnimalPresentPacket(u8 *dst, int idx, int kind, const void *item);    // 800F14FC
void sendAnimalPresent(int idx, int kind, const void *item);                    // 800F1514
void parsePermitStatePacket(void *item, int *mode, const u8 *src);              // 800F159C
void buildPermitStatePacket(u8 *dst, const void *item, int mode);               // 800F15F8
void sendPermitState(const void *item, int mode);                               // 800F1638
void sendPermitSetState(const void *item, int mode);                            // 800F16B0
void parsePermitKeyPacket(void *item, const void *src);                         // 800F1728
void buildPermitKeyPacket(void *dst, const void *item);                         // 800F1730
void sendPermitFlag(const void *item, u32 player);                              // 800F1738
void sendPermitQuery(const void *item);                                         // 800F17D0
void sendPermitAnswer(const void *item, u32 player);                            // 800F1838
void sendPermitClearState(const void *item, int mode);                          // 800F18D0
void sendPermitFallbackPlayer(const void *item, int mode);                      // 800F1948
void sendPermitClear(const void *item);                                         // 800F19C0
void parseAnimalClothPacket(int *idx, void *item, const u8 *src);               // 800F1A28
void buildAnimalClothPacket(u8 *dst, int idx, const void *item);                // 800F1A54
void sendAnimalCloth(int idx, const void *item);                                // 800F1A68

// Extra filter for the spot pickers.
typedef BOOL (*spotFunc)(int x, int z, int arg);

// Spot pickers.
dPlayerActor_c *getPlayerOnUnit(int x, int z);                                  // 800F1AE0
dActor_c *getActorOnUnit(int x, int z);                                         // 800F1B7C

dItem::Item getFurnitureCoveringUnit(int *outX, int *outZ, int x, int z, dFdBase_c *map, u8 kind); // 800F1BE4

BOOL canPutItemOnUnit(int x, int z, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA); // 800F1DF0
inline bool canPlaceEmptyItemOnUnit(int x, int z, dFdBase_c *map) {
    dItem::Item none;
    return canPutItemOnUnit(x, z, &none, map, dScene::SCENE_FIELD, TRUE, TRUE);
}

BOOL canPutItemAt(const mVec3_c *pos, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA); // 800F20D0

BOOL pickNearbyFreePos(mVec3_c *out, const mVec3_c *pos, u32 radius);           // 800F2138
BOOL isUnitWithinDistance(const mVec3_c *pos, int x, int z, f32 dist);          // 800F22FC
BOOL pickFreeUnitInRect(int *outX, int *outZ, dFdBase_c *map, u8 kind, int x0, int x1, int z0, int z1,
                 spotFunc func, int arg, const mVec3_c *exclude, f32 dist);     // 800F23C0
BOOL pickOutdoorFreeUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F25A4
BOOL pickOutdoorFreePos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F2644
BOOL pickUnitWithNorthClearance(int *outX, int *outZ);                          // 800F26C8
BOOL pickPosWithNorthClearance(mVec3_c *out);                                   // 800F28AC
BOOL pickSceneFreeUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F2920
BOOL pickSceneFreePos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F29C4
BOOL hasEntranceSideSpots(const dItem::Item *item);                             // 800F2A48
u32 pickBuildingSideUnitInBlock(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist);                                      // 800F2C94
u32 pickTreeSideUnitInBlock(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist);                                      // 800F2F8C
BOOL pickOutdoorPlacementUnit(int *outX, int *outZ, spotFunc func, int arg, const mVec3_c *exclude, u32 type,
                 BOOL skipTrees, BOOL allowFg94, f32 dist);                     // 800F3178
BOOL pickOutdoorPlacementPos(mVec3_c *out, spotFunc func, int arg, const mVec3_c *exclude, u32 type, BOOL skipTrees,
                 BOOL allowFg94, f32 dist);                                     // 800F3830
BOOL isUnitBlockedToSouth(int x, int z);                                        // 800F3C0C
BOOL isPosBlockedToSouth(const mVec3_c *pos);                                   // 800F3D20

// Names, words and hours.
BOOL getNpcName(dScript::Word_c *word, const dItem::Item *key, int language);   // 800F3D68

void initEntries();                                                             // 800F3E24
dTime_c *getEntryTime();                                                        // 800F3E38
void setEntryTimeNow();                                                         // 800F3E48
entry_c *getEntry(u32 i);                                                       // 800F3E54
entry_c *getEntryByKey(const dItem::Item *key);                                 // 800F3E64
entry_c *getEntryByAnimal(const dAnmPersonalID_c *animal);                      // 800F3EE8
u32 getAnimalCompatibilityScore(dAnimal_c *a, dAnimal_c *b);                    // 800F3F30
int getAnimalCompatibilityRank(dAnimal_c *a, dAnimal_c *b);                     // 800F3F34
int getPlayerUnitMessage(u8 gender, u8 looks);                                  // 800F3F38
int getPlayerUnitMessageForAnimal(const dPersonalID_c *pid, u8 looks, dAnimal_c *animal); // 800F3F84
int getPersonalityVoiceType(u8 idx);                                            // 800F3FFC
BOOL isAnimalSleepTime(u8 idx, const dTime_c *now);                             // 800F4020
BOOL isPastAnimalWakeTime(u8 idx, const dTime_c *now);                          // 800F4250
BOOL isAnimalDaytime(u8 idx, const dTime_c *now);                               // 800F42E0
int selectPresentItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4510: picks num items from the table, filtered
int selectRoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F45A4: picks num items from the table, filtered
int selectGroupARoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4608: picks num items from the table, filtered
int selectGroupCRoomItems(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4668: picks num items from the table, filtered
void setAnimalNameWord(const dAnmPersonalID_c *animal, int slot);               // 800F46CC
void setPlayerNameWord(const dPersonalID_c *pid, int slot);                     // 800F4718
void setAnimalLandNameWord(const dAnmPersonalID_c *animal, int slot);           // 800F4764
void setItemNameWord(const dItem::Item *item, int slot);                        // 800F4774
void setPlayerUnitWord(u8 gender, u8 looks, int slot);                          // 800F4794

// Spare villager buffer access (lbl_805C1000).
void clearAnimalBuffer();                                                       // 800F47D8
BOOL takeMovedAnimalToBuffer(int arg);                                          // 800F47E4
BOOL storeBufferedMovedAnimal();                                                // 800F47F4
void exchangeBufferedMovedAnimal();                                             // 800F4800
BOOL exchangeBufferedAnimalWithMember(const dAnimal_c *src, u32 idx);           // 800F4840
bufferedAnimal_c *getBufferedAnimal();                                          // 800F4858
BOOL clearBufferedAnimal();                                                     // 800F4868
u32 takeBufferedAnimal(dAnimal_c *dst, u32 idx);                                // 800F4878
BOOL setBufferedAnimal(const dAnimal_c *src);                                   // 800F4890

// Villager moves.
BOOL takeMovedAnimal(dAnimal_c *animal);                                        // 800F48A0
BOOL removeMovedAnimal(const dAnmPersonalID_c *id);                             // 800F4904
BOOL addMovedAnimal(dAnimal_c *animal);                                         // 800F4984
dAnimal_c *pickMovedAnimal();                                                   // 800F4A08
BOOL receiveMovedAnimal(dAnimal_c *animal);                                     // 800F4A34
BOOL addRandomMovedAnimal(dAnimal_c **animals);                                 // 800F4AB8
BOOL isAnimalOnAllowedSpot(dAnimal_c *animal, int arg);                         // 800F4C08
BOOL initTownAnimals();                                                         // 800F4D08

f32 getAnimalHandItemOfs(u8 idx, u32 axis);                                     // 800F4D84
f32 getSpHandItemOfs(const dItem::Item *key, u32 axis);                         // 800F4DB8

// Per-NPC communication permit. 3 bytes.
class permit_c {
public:
    permit_c();                                                                 // 800F4E00
    ~permit_c();                                                                // 800F4E40

    void clear();                                                               // 800F4E80
    void setState(u32 state);                                                   // 800F4E94: clamped to 4
    BOOL hasState();                                                            // 800F4EF4
    BOOL isFlag(u32 i);                                                         // 800F4F54
    void setFlag(u32 i);                                                        // 800F4F74
    void setFallbackPlayer(u32 value);                                          // 800F4F94: clamped to 4
    void resetFallbackPlayer();                                                 // 800F4FF4
    BOOL isOwnedByMe();                                                         // 800F5000
    BOOL isReady();                                                             // 800F5094

    /* 0x0 */ u8 mOwnerPlayer; // NET_MEMBER_NONE: none
    /* 0x1 */ u8 mAckPlayers; // one bit per player
    /* 0x2 */ u8 mFallbackPlayer;     // NET_MEMBER_NONE: none
}; // size 0x3

class permitMng_c {
public:
    permitMng_c(permit_c *entries, u32 num) : mEntries(entries), mPermitCount(num) {}
    static void *operator new(size_t, void *p) { return p; }
    virtual ~permitMng_c() {}
    virtual int getIdx(const dItem::Item *key) = 0;
    virtual void init();                                                        // 800F513C

    permit_c *get(u32 i);                                                       // 800F51AC
    permit_c *getByKey(const dItem::Item *key);                                 // 800F51E4

    /* 0x4 */ permit_c *mEntries;
    /* 0x8 */ u32 mPermitCount;
}; // size 0xC

// 10 villagers (lbl_8074E568).
class normalPermitMng_c : public permitMng_c {
public:
    normalPermitMng_c() : permitMng_c(mStorage, ANIMAL_NUM) {}
    virtual ~normalPermitMng_c();                                             // 800F5648
    virtual int getIdx(const dItem::Item *key);                                 // 800F5228

    /* 0xC */ permit_c mStorage[ANIMAL_NUM];
}; // size 0x2C

// 97 special NPCs (lbl_8074E56C).
class specialPermitMng_c : public permitMng_c {
public:
    specialPermitMng_c() : permitMng_c(mStorage, NPC_SPECIAL_NUM) {}
    virtual ~specialPermitMng_c();                                            // 800F55E0
    virtual int getIdx(const dItem::Item *key);                                 // 800F52AC

    /* 0xC */ permit_c mStorage[NPC_SPECIAL_NUM];
}; // size 0x130

permitMng_c *getPermitManager(const dItem::Item *key);                          // 800F52D4
u32 getPermitHeapSize();                                                        // 800F5370: heap size for createPermitManagers
void createPermitManagers(EGG::Heap *heap);                                     // 800F5378
void initPermitManagers();                                                      // 800F548C
permit_c *getPermitByKey(const dItem::Item *key);                               // 800F54E0
void setPermitScene(u8 scene);                                                  // 800F5524
BOOL hasPermitScene();                                                          // 800F552C
void clearPermitScene();                                                        // 800F5548
BOOL isPermitSceneOwner();                                                      // 800F5550
void resetPermits();                                                            // 800F55B0
void onNetSceneOpen(int, int, int, u8 scene);                                   // 800F55B4
int onNetSceneClose();                                                          // 800F55BC

} // namespace dNpc
