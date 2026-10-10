#pragma once

// NPC / villager management. Source: src/dol/game/d_npc.cpp (.text 800ECD90..800F59C8,
// sinit 800F5768; built with -sym on). Class names under dNpc:: come from RTTI; the rest,
// and all method and field names, are inferred unless they came with symbols.txt.

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

class dAnimal_c;
class dPrivateData_c;

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

    msgMemory_c();                                                   // 800ECD90
    virtual ~msgMemory_c();                                          // 800ECDA0
    virtual void clear();                                            // 800ECDE0

    BOOL isValid();                                                  // 800ECE34
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
    msgMemorySecond_c();                                             // 800ECEF4
    virtual ~msgMemorySecond_c();                                    // 800ECF40
    virtual void clear();                                            // 800ECF98

    BOOL setA0(int value);                                           // 800ED01C
    BOOL setPersonal(const dPersonalID_c *pid);                      // 800ED06C
    BOOL setPlayer(const dPlayerID_c *player);                       // 800ED0C8
    BOOL setLand(const dLandID_c *land);                             // 800ED124
    BOOL setA4(u32 value);                                           // 800ED180: < 4
    BOOL setText(const wchar_t *text);                               // 800ED19C
    BOOL setAC(u32 value);                                           // 800ED224: < 50

    /* 0x46 */ dPersonalID_c mPersonal;
    /* 0x72 */ dPlayerID_c mPlayer;
    /* 0x88 */ dLandID_c mLand;
    /* 0x9E */ u8 _9E[2];
    /* 0xA0 */ s32 _A0;
    /* 0xA4 */ s32 _A4;
    /* 0xA8 */ s32 _A8;
    /* 0xAC */ u32 _AC; // 50 when cleared
    /* 0xB0 */ dItem::Item mItem0;
    /* 0xB2 */ dItem::Item mItem1;
    /* 0xB4 */ wchar_t mText[17];
}; // size 0xD8

} // namespace dNpc

// 0x11E at dNpcEntry_c+0x120. Counters per something, plus 6 indices.
class dNpcUnk120_c {
public:
    dNpcUnk120_c();  // 800EDB00
    ~dNpcUnk120_c(); // 800EDB04

    void clear();                                                // 800EDB44
    void set(u32 i, u16 value, u32 j);                           // 800EDBD0
    u16 get(u32 i, u32 j);                                       // 800EDBF4
    int getIdx(u32 i);                                           // 800EDC20
    void setIdx(u32 i, s8 value);                                // 800EDC40
    dAnimal_c *fn_800EDC54(u32 i, const dAnmPersonalID_c **exclude, u32 numExclude); // 800EDC54

    /* 0x000 */ u16 _000[7][20];
    /* 0x118 */ s8 _118[6]; // -1 when cleared; indexed up to 10
}; // size 0x11E

// 0x10 at dNpcEntry_c+0x250. A countdown: a mode, a minute count and the tick it expires at.
class dNpcTimer_c {
public:
    dNpcTimer_c();  // 800ED240
    ~dNpcTimer_c(); // 800ED244

    void clear();                                                    // 800ED284
    void reset();                                                    // 800ED29C: mode 5
    void addMinutes(u32 mins);                                       // 800ED2B8: saturates at 0xFFFF
    void set(int mode, u16 mins);                                     // 800ED2D8
    void copy(const dNpcTimer_c *other);                             // 800ED4C0
    void tick();                                                     // 800ED50C
    int getMode();                                                   // 800ED544
    void update();                                                   // 800ED590

    /* 0x0 */ s64 mEnd;
    /* 0x8 */ u32 mMode;
    /* 0xC */ u16 mMinutes;
}; // size 0x10

// 0x1 at dNpcEntry_c+0x288. A 0-32 value (friendship?).
class dNpcUnk288_c {
public:
    dNpcUnk288_c();  // 800ED690
    ~dNpcUnk288_c(); // 800ED694

    void clear();                                                    // 800ED6D4
    void fn_800ED6E0(dAnimal_c *animal, dPrivateData_c *player);     // 800ED6E0
    static int fn_800ED7D4(u32 max, u32 min, dAnimal_c *animal, dPrivateData_c *player); // 800ED7D4
    void set(u32 value);                                             // 800ED92C: clamped to 32
    void add(int delta);                                             // 800ED940
    void half();                                                     // 800ED964
    void twice();                                                    // 800ED970

    /* 0x0 */ u8 mValue;
}; // size 0x1

// 0x290. Everything the game remembers about one villager.
class dNpcEntry_c {
public:
    dNpcEntry_c();  // 800ED97C
    ~dNpcEntry_c(); // 800ED9D4

    void clear(); // 800EDA6C

    /* 0x000 */ dNpc::msgMemory_c mMsg;
    /* 0x048 */ dNpc::msgMemorySecond_c mMsg2;
    /* 0x120 */ dNpcUnk120_c _120;
    /* 0x23E */ dQuestBase_c mQuest;
    /* 0x24C */ u8 _24C[4];
    /* 0x250 */ dNpcTimer_c mTimer;
    /* 0x260 */ dTime_c mTime;
    /* 0x288 */ dNpcUnk288_c _288;
    /* 0x289 */ u8 mStallBuyCount;    // the npc bought at the player's flea market stall
    /* 0x28A */ u8 mStallRefuseCount; // the player refused the npc's offer at the stall
    /* 0x28B */ u8 _28B;
    /* 0x28C */ struct {
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
    } _28C;              // cleared by memset in clear(); mTalked / mEventTalked also by dAcNpcNml_c create
    /* 0x28E */ u8 _28E[2];
}; // size 0x290

namespace dNpc {

// A list of dNpcEntry_c.
class list_c {
public:
    list_c(dNpcEntry_c *entries, u32 num) : mEntries(entries), mNum(num) {}
    virtual ~list_c() {}                                             // 800F5718
    virtual void clear();                                            // 800EDE0C
    virtual void init();                                             // 800EDE74: clear()

    dNpcEntry_c *get(u32 i);                                         // 800EDE84

    /* 0x4 */ dNpcEntry_c *mEntries;
    /* 0x8 */ u32 mNum;
    /* 0xC */ u8 _C[4];
}; // size 0x10

// The town's villager entries plus the time they were last updated (lbl_805BF610).
class listLand_c : public list_c {
public:
    listLand_c() : list_c(mList, 10) {}
    virtual ~listLand_c() {}                                         // 800F56B0
    virtual void clear();                                            // 800EDEA8

    void setTimeNow();                                               // 800EDEDC

    /* 0x0010 */ dNpcEntry_c mList[10];
    /* 0x19B0 */ dTime_c mTime;
}; // size 0x19D8

} // namespace dNpc

// Villager compatibility.
u8 fn_800EDF18(u32 a, u32 b);                                        // 800EDF18: personality x personality
u8 fn_800EDF48(u32 sign);                                            // 800EDF48: star sign -> element
u8 fn_800EDF68(u32 signA, u32 signB);                                // 800EDF68
u8 fn_800EDFD4(int monthA, int dayA, int monthB, int dayB);          // 800EDFD4
const u8 *fn_800EE030(u32 a, u32 b, const u8 *pairs, int num);       // 800EE030
u32 fn_800EE094(u32 a, u32 b);                                       // 800EE094: species x species
u32 fn_800EE138(dAnimal_c *a, dAnimal_c *b);                         // 800EE138
int fn_800EE244(dAnimal_c *a, dAnimal_c *b);                         // 800EE244: 0 good .. 2 bad

// 0x3040. A dAnimal_c held outside the town.
class dNpcAnimal_c {
public:
    dNpcAnimal_c();  // 800EE290
    ~dNpcAnimal_c(); // 800EE2C0

    void clear();    // 800EE318

    /* 0x0000 */ dAnimal_c mAnimal;
}; // size 0x3040

// 0xC100 (lbl_805C1000). Four spare villagers.
class dNpcAnimalBuf_c {
public:
    dNpcAnimalBuf_c();  // 800EE31C
    ~dNpcAnimalBuf_c(); // 800EE36C

    void clear();                                                    // 800EE3D0
    static BOOL isValidIdx(u32 i);                                   // 800EE41C
    dNpcAnimal_c *get(int i);                                        // 800EE434
    BOOL fn_800EE440(int arg);                                       // 800EE440
    BOOL fn_800EE4AC();                                              // 800EE4AC
    void fn_800EE514();                                              // 800EE514
    BOOL fn_800EE5C4(const dAnimal_c *src, u32 idx);                 // 800EE5C4
    BOOL fn_800EE71C(const dAnimal_c *src);                          // 800EE71C
    u32 fn_800EE76C(dAnimal_c *dst, u32 idx);                        // 800EE76C
    dNpcAnimal_c *fn_800EE818(u32 idx);                              // 800EE818
    BOOL fn_800EE890(u32 idx);                                       // 800EE890

    /* 0x0000 */ dNpcAnimal_c mAnimals[4];
}; // size 0xC100

// 0x2F packed record, copied byte-wise (network data).
class dNpcDaub_c {
public:
    dNpcDaub_c();                                                    // 800EE8F0
    dNpcDaub_c(const dNpcDaub_c &other);                             // 800EE8F4
    ~dNpcDaub_c();                                                   // 800EE924

    void clear();                                                    // 800EE964
    void copy(const dNpcDaub_c *other);                              // 800EE9AC
    static void packXZ(u8 *dst, const mVec3_c *src);                 // 800EEA88
    static void unpackXZ(const u8 *src, mVec3_c *dst);               // 800EEAD4
    void set08(const u16 *value);                                    // 800EEB28
    mAng get08();                                                    // 800EEB34
    void set0D(mAng value);                                    // 800EEB6C
    mAng get0D();                                                    // 800EEB78
    void set0F(mAng value);                                    // 800EEBB0
    mAng get0F();                                                    // 800EEBBC
    void get11(void *dst, u32 size);                                 // 800EEBF4
    void set11(const void *src, u32 size);                           // 800EEC2C
    void get19(u32 *a, u32 *b);                                      // 800EEC5C
    void set19(u32 a, u32 b);                                        // 800EEC80
    void getMove(mVec3_c *from, mVec3_c *to, u16 *angle, f32 *speed); // 800EEC8C
    void setMove(const mVec3_c *from, const mVec3_c *to, const u16 *angle, f32 speed); // 800EED5C
    void getTalk(u16 *a, u16 *b, f32 *c);                            // 800EEE10
    void setTalk(const u16 *a, const u16 *b, f32 c);                 // 800EEEA8
    void getAct(u32 *a, u8 *b, f32 *c, f32 *d);                      // 800EEF14
    void setAct(u32 a, u8 b, f32 c, f32 d);                          // 800EEFDC

    /* 0x00 */ u8 _00[4];
    /* 0x04 */ u8 _04[4];
    /* 0x08 */ u8 _08[2];
    /* 0x0A */ u8 _0A; // 4 when cleared
    /* 0x0B */ u8 _0B; // 4 when cleared
    /* 0x0C */ u8 _0C; // 0x44 when cleared
    /* 0x0D */ u8 _0D[2];
    /* 0x0F */ u8 _0F[2];
    /* 0x11 */ u8 _11[8];
    /* 0x19 */ u8 _19[0x15];
    /* 0x2E */ u8 mFlag : 1;
    /* 0x2E */ u8 _2E : 7;
}; // size 0x2F

namespace dNpc {

// Base of the daub managers. Keys map to an index via getIdx; ids are getBase() + index.
class daubMng_c {
public:
    daubMng_c(dNpcDaub_c *daubs, u32 num);                           // 800EF060
    virtual ~daubMng_c();                                            // 800EF078
    virtual int getIdx(const u16 *key) = 0;
    virtual int getBase() = 0;

    void clear();                                                    // 800EF0C4
    void init();                                                     // 800EF124
    BOOL isValidIdx(u32 idx);                                        // 800EF128
    BOOL isValidKey(const u16 *key);                                 // 800EF140
    BOOL isValidId(int id);                                          // 800EF184
    int keyToId(const u16 *key);                                     // 800EF1BC: 0xBA when invalid
    int idToIdx(int id);                                             // 800EF234
    dNpcDaub_c *get(u32 idx);                                        // 800EF298
    dNpcDaub_c *getByKey(const u16 *key);                            // 800EF2EC
    dNpcDaub_c *getById(int id);                                     // 800EF330

    /* 0x4 */ dNpcDaub_c *mDaubs;
    /* 0x8 */ u32 mNum;
}; // size 0xC

// 10 daubs, ids 0x2B.. (lbl_805CD10C).
class daubMngNormal_c : public daubMng_c {
public:
    daubMngNormal_c();                                               // 800EF368
    virtual ~daubMngNormal_c();                                      // 800EF3CC
    virtual int getIdx(const u16 *key);                              // 800EF440
    virtual int getBase() { return 0x2B; }                           // 800F5760

    void fn_800EF4B8();                                              // 800EF4B8

    /* 0x00C */ dNpcDaub_c mList[10];
}; // size 0x1E4

// 97 daubs, ids 0x35.. (lbl_805CD2FC).
class daubMngSpecial_c : public daubMng_c {
public:
    daubMngSpecial_c();                                              // 800EF57C
    virtual ~daubMngSpecial_c();                                     // 800EF5E0
    virtual int getIdx(const u16 *key);                              // 800EF654
    virtual int getBase() { return 0x35; }                           // 800F5758

    void fn_800EF768();                                              // 800EF768

    /* 0x00C */ dNpcDaub_c mList[97];
}; // size 0x11F8

} // namespace dNpc

// A scene's actor placements (getSceneData; dSceneActorData_c in d_scene.hpp), as used by fn_800EF670.
struct dNpcLayoutEntry_c {
    /* 0x00 */ u8 _00[4];
    /* 0x04 */ mVec3_c mPos;
    /* 0x10 */ u8 _10[2];
    /* 0x12 */ s16 mAngle;
    /* 0x14 */ u8 _14[4];
    /* 0x18 */ u32 mKey;
}; // size 0x1C

struct dNpcLayoutGroup_c {
    /* 0x0 */ u16 mType;
    /* 0x2 */ u16 mCount;
    /* 0x4 */ dNpcLayoutEntry_c *mEntries;
}; // size 0x8

struct dNpcLayout_c {
    /* 0x0 */ u16 mNum;
    /* 0x4 */ dNpcLayoutGroup_c *mGroups;
};

BOOL fn_800EF670(mVec3_c *pos, s16 *angle, const dItem::Item &key, u8 layout); // 800EF670

// Daub manager access (lbl_8074AE88: the normal and special managers).
dNpc::daubMng_c *fn_800EFBBC(u32 i);                                 // 800EFBBC
dNpc::daubMng_c *fn_800EFBDC(const u16 *key);                        // 800EFBDC
dNpc::daubMng_c *fn_800EFC54(int id);                                // 800EFC54
void fn_800EFCCC();                                                  // 800EFCCC
dNpcDaub_c *fn_800EFD04(const u16 *key);                             // 800EFD04
void fn_800EFD48();                                                  // 800EFD48
void fn_800EFDD0(void *dst, int id);                                 // 800EFDD0
void fn_800EFDD4();                                                  // 800EFDD4
void fn_800EFE68(void *dst, int id);                                 // 800EFE68
const dNpcDaub_c *fn_800EFE6C(const u16 *key);                      // 800EFE6C: the received daub copy
void fn_800EFEDC(void *dst, int id);                                 // 800EFEDC
void fn_800EFF60(int *idx, void *dst, const u8 *src);                // 800EFF60
void fn_800EFF8C(u8 *dst, int idx, const void *src);                  // 800EFF8C
void fn_800EFFA0(u32 idx, const void *data);                         // 800EFFA0

// Network messages (types 0x55..0x82): parse / build / send triples.
void fn_800F0020(int *idx, u8 *value, const u8 *src);                 // 800F0020
void fn_800F0048(u8 *dst, int idx, int value);                          // 800F0048
void fn_800F0054(u32 idx, u8 value);                                  // 800F0054
void fn_800F00D4(int *idx, u8 *value, const u8 *src);                 // 800F00D4
void fn_800F00FC(u8 *dst, int idx, int value);                          // 800F00FC
void fn_800F0108(u32 idx, u8 value);                                  // 800F0108
void fn_800F0188(int *idx, int *kind, u8 *count, u16 *msg, u8 *flag, const u8 *src); // 800F0188
void fn_800F01F0(u8 *dst, int idx, int kind, int count, int msg, int flag); // 800F01F0
void fn_800F0208(u32 idx, u32 kind, u8 count, u8 msg, s8 flag);       // 800F0208
void fn_800F02A8(u32 idx, u32 kind, u8 count, u8 msg, s8 flag);       // 800F02A8
void fn_800F0348(int *idx, int *kind, u8 *value, const u8 *src);      // 800F0348
void fn_800F038C(u8 *dst, int idx, int kind, int value);                 // 800F038C
void fn_800F039C(u32 idx, u32 kind, s8 value);                        // 800F039C
void fn_800F0434(const dAnmPersonalID_c *animal, u32 kind, s8 value); // 800F0434
void fn_800F0494(int *idx, int *kind, int *value, const u8 *src);     // 800F0494
void fn_800F04D8(u8 *dst, int idx, int kind, int value);                 // 800F04D8
void fn_800F04E8(u32 idx, u32 kind, u8 value);                        // 800F04E8
void fn_800F0580(int *idx, int *kind, const u8 *src);                 // 800F0580
void fn_800F05BC(u8 *dst, int idx, int kind);                           // 800F05BC
void fn_800F05C8(u32 idx, u32 kind);                                  // 800F05C8
void fn_800F0650();                                                   // 800F0650
void fn_800F06A8(u8 value);                                           // 800F06A8
void fn_800F0708(int value);                                          // 800F0708
void fn_800F0770(u8 *kind, int *x, int *z, const u8 *src);            // 800F0770
void fn_800F080C(u8 *dst, int kind, int x, int z);                       // 800F080C
void fn_800F0864(int kind, int x, int z);                                // 800F0864
void fn_800F08EC(int *idx, void *item, int *count, const u8 *src);    // 800F08EC
void fn_800F0968(u8 *dst, int idx, const void *item, int count);        // 800F0968
void fn_800F09B4(int idx, const void *item, int count);                 // 800F09B4
void fn_800F0A3C(int *idx, int *mode, const u8 *src);                 // 800F0A3C
void fn_800F0A78(u8 *dst, int idx, int mode);                           // 800F0A78
void fn_800F0A84(int idx, int mode);                                    // 800F0A84
void fn_800F0AFC(int *idx, int *a, int *b, u32 *c, const u8 *src);    // 800F0AFC
void fn_800F0B50(u8 *dst, int idx, int a, int b, int c);                  // 800F0B50
void fn_800F0B64(int idx, int a, int b, int c);                       // 800F0B64: int params (d_animal callers pass ints untruncated)
void fn_800F0BFC(int *idx, u32 *kind, void *data, const u8 *src);     // 800F0BFC
void fn_800F0C44(u8 *dst, int idx, int kind, const void *data);         // 800F0C44
void fn_800F0C5C(int idx, int kind, const void *data);                  // 800F0C5C
void fn_800F0CE4(int *idx, const u8 *src);                            // 800F0CE4
void fn_800F0D04(u8 *dst, int idx);                                    // 800F0D04
void fn_800F0D0C(int idx);                                             // 800F0D0C
void fn_800F0D74(int *idx, int *count, void *item, const u8 *src);    // 800F0D74
void fn_800F0DBC(u8 *dst, int idx, int count, const void *item);        // 800F0DBC
void fn_800F0DD4(int idx, int count, const void *item);                 // 800F0DD4
void fn_800F0E5C(int *idx, void *item, const u8 *src);                // 800F0E5C
void fn_800F0E88(u8 *dst, int idx, const void *item);                  // 800F0E88
void fn_800F0E9C(int idx, const void *item);                           // 800F0E9C
void fn_800F0F14(int *idx, const u8 *src);                            // 800F0F14
void fn_800F0F34(u8 *dst, int idx);                                    // 800F0F34
void fn_800F0F3C(int idx);                                             // 800F0F3C
void fn_800F0FA4(int *idx, void *item, const u8 *src);                // 800F0FA4
void fn_800F0FD0(u8 *dst, int idx, const void *item);                  // 800F0FD0
void fn_800F0FE4(int idx, const void *item);                           // 800F0FE4
void fn_800F105C(int *idx, void *item, const u8 *src);                // 800F105C
void fn_800F1088(u8 *dst, int idx, const void *item);                  // 800F1088
void fn_800F109C(int idx, const void *item);                           // 800F109C
void fn_800F1114(int *idx, void *item, const u8 *src);                // 800F1114
void fn_800F1140(u8 *dst, int idx, const void *item);                  // 800F1140
void fn_800F1154(int idx, const void *item);                           // 800F1154
void fn_800F11CC(int *idx, void *item, const u8 *src);                // 800F11CC
void fn_800F11F8(u8 *dst, int idx, const void *item);                  // 800F11F8
void fn_800F120C(int idx, const void *item);                           // 800F120C
void fn_800F1284(int *idx, int *kind, u8 *value, const u8 *src);      // 800F1284
void fn_800F12C8(u8 *dst, int idx, int kind, int value);                 // 800F12C8
void fn_800F12D8(int idx, int kind, int value);                          // 800F12D8
void fn_800F1360(int *idx, const u8 *src);                            // 800F1360
void fn_800F1380(u8 *dst, int idx);                                    // 800F1380
void fn_800F1388(int idx);                                             // 800F1388
void fn_800F13F0(int *idx, const u8 *src);                            // 800F13F0
void fn_800F1410(u8 *dst, int idx);                                    // 800F1410
void fn_800F1418(int idx);                                             // 800F1418
void fn_800F1480(int *idx, int *kind, void *item, const u8 *src);     // 800F1480
void fn_800F14FC(u8 *dst, int idx, int kind, const void *item);         // 800F14FC
void fn_800F1514(int idx, int kind, const void *item);                  // 800F1514
void fn_800F159C(void *item, int *mode, const u8 *src);               // 800F159C
void fn_800F15F8(u8 *dst, const void *item, int mode);                 // 800F15F8
void fn_800F1638(const void *item, int mode);                          // 800F1638
void fn_800F16B0(const void *item, int mode);                          // 800F16B0
void fn_800F1728(void *item, const void *src);                        // 800F1728
void fn_800F1730(void *dst, const void *item);                        // 800F1730
void fn_800F1738(const void *item, u32 player);                       // 800F1738
void fn_800F17D0(const void *item);                                   // 800F17D0
void fn_800F1838(const void *item, u32 player);                       // 800F1838
void fn_800F18D0(const void *item, int mode);                          // 800F18D0
void fn_800F1948(const void *item, int mode);                          // 800F1948
void fn_800F19C0(const void *item);                                   // 800F19C0
void fn_800F1A28(int *idx, void *item, const u8 *src);                // 800F1A28
void fn_800F1A54(u8 *dst, int idx, const void *item);                  // 800F1A54
void fn_800F1A68(int idx, const void *item);                           // 800F1A68

// Furniture footprint iterator (fn_800A8B28 / fn_800A8BB8 / fn_800A8BE4, declared in d_ftr.hpp).
// fn_800A8B28 fills it from an item (and returns it), fn_800A8BB8 is the tile count.
struct dNpcFtrShape_c {
    /* 0x0 */ s32 mSize; // BITM::m_ftrSize (0..2)
    /* 0x4 */ s32 mRot; // the item id's low 2 bits
};

// Extra filter for the spot pickers.
typedef BOOL (*dNpcSpotFunc)(int x, int z, int arg);

// Spot pickers.
class dActor_c;
class dPlayerActor_c;
dPlayerActor_c *getPlayerOnUnit(int x, int z);                           // 800F1AE0
dActor_c *getActorOnUnit(int x, int z);                                 // 800F1B7C
dItem::Item fn_800F1BE4(int *outX, int *outZ, int x, int z, dFdBase_c *map, u8 kind); // 800F1BE4
BOOL canPutItemOnUnit(int x, int z, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA); // 800F1DF0
BOOL canPutItemAt(const mVec3_c *pos, const dItem::Item *item, dFdBase_c *map, u8 kind, BOOL allowFg94, BOOL checkA); // 800F20D0
BOOL fn_800F2138(mVec3_c *out, const mVec3_c *pos, u32 radius);      // 800F2138
BOOL fn_800F22FC(const mVec3_c *pos, int x, int z, f32 dist);        // 800F22FC
BOOL fn_800F23C0(int *outX, int *outZ, dFdBase_c *map, u8 kind, int x0, int x1, int z0, int z1,
                 dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F23C0
BOOL fn_800F25A4(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F25A4
BOOL fn_800F2644(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F2644
BOOL fn_800F26C8(int *outX, int *outZ);                              // 800F26C8
BOOL fn_800F28AC(mVec3_c *out);                                      // 800F28AC
BOOL fn_800F2920(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F2920
BOOL fn_800F29C4(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, f32 dist); // 800F29C4
BOOL fn_800F2A48(const dItem::Item *item);                           // 800F2A48
u32 fn_800F2C94(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist); // 800F2C94
u32 fn_800F2F8C(int *outX, int *outZ, u32 num, u32 bx, u32 bz, dFdBase_c *map, const mVec3_c *exclude,
                BOOL allowFg94, f32 dist); // 800F2F8C
BOOL fn_800F3178(int *outX, int *outZ, dNpcSpotFunc func, int arg, const mVec3_c *exclude, u32 type,
                 BOOL skipTrees, BOOL allowFg94, f32 dist); // 800F3178
BOOL fn_800F3830(mVec3_c *out, dNpcSpotFunc func, int arg, const mVec3_c *exclude, u32 type, BOOL skipTrees,
                 BOOL allowFg94, f32 dist); // 800F3830
BOOL fn_800F3C0C(int x, int z);                                      // 800F3C0C
BOOL fn_800F3D20(const mVec3_c *pos);                                // 800F3D20

// Names, words and hours.
namespace dScript {
class Word_c;
}
BOOL getNpcName(dScript::Word_c *word, const dItem::Item *key, int language); // 800F3D68
void fn_800F3E24();                                                  // 800F3E24
dTime_c *fn_800F3E38();                                              // 800F3E38
void fn_800F3E48();                                                  // 800F3E48
dNpcEntry_c *fn_800F3E54(u32 i);                                     // 800F3E54
dNpcEntry_c *fn_800F3E64(const dItem::Item *key);                    // 800F3E64
dNpcEntry_c *fn_800F3EE8(const dAnmPersonalID_c *animal);             // 800F3EE8
u32 fn_800F3F30(dAnimal_c *a, dAnimal_c *b);                         // 800F3F30
int fn_800F3F34(dAnimal_c *a, dAnimal_c *b);                         // 800F3F34
int fn_800F3F38(u8 gender, u8 looks);                                // 800F3F38
int fn_800F3F84(const dPersonalID_c *pid, u8 looks, dAnimal_c *animal); // 800F3F84
int fn_800F3FFC(u8 idx);                                             // 800F3FFC
BOOL fn_800F4020(u8 idx, const dTime_c *now);                        // 800F4020
BOOL fn_800F4250(u8 idx, const dTime_c *now);                        // 800F4250
BOOL fn_800F42E0(u8 idx, const dTime_c *now);                        // 800F42E0
int fn_800F4510(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4510: picks num items from the table, filtered
int fn_800F45A4(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F45A4: picks num items from the table, filtered
int fn_800F4608(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4608: picks num items from the table, filtered
int fn_800F4668(dItem::Item *out, int num, const void *filter, const dItem::Item *exclude, int excludeNum); // 800F4668: picks num items from the table, filtered
void fn_800F46CC(const dAnmPersonalID_c *animal, int slot);          // 800F46CC
void fn_800F4718(const dPersonalID_c *pid, int slot);                // 800F4718
void fn_800F4764(const dAnmPersonalID_c *animal, int slot);          // 800F4764
void fn_800F4774(const dItem::Item *item, int slot);                 // 800F4774
void fn_800F4794(u8 gender, u8 looks, int slot);                      // 800F4794

// Spare villager buffer access (lbl_805C1000).
void fn_800F47D8();                                                  // 800F47D8
BOOL fn_800F47E4(int arg);                                           // 800F47E4
BOOL fn_800F47F4();                                                  // 800F47F4
void fn_800F4800();                                                  // 800F4800
BOOL fn_800F4840(const dAnimal_c *src, u32 idx);                     // 800F4840
dNpcAnimal_c *fn_800F4858();                                         // 800F4858
BOOL fn_800F4868();                                                  // 800F4868
u32 fn_800F4878(dAnimal_c *dst, u32 idx);                            // 800F4878
BOOL fn_800F4890(const dAnimal_c *src);                              // 800F4890

// Villager moves.
BOOL fn_800F48A0(dAnimal_c *animal);                                 // 800F48A0
BOOL fn_800F4904(const dAnmPersonalID_c *id);                        // 800F4904
BOOL fn_800F4984(dAnimal_c *animal);                                 // 800F4984
dAnimal_c *fn_800F4A08();                                            // 800F4A08
BOOL fn_800F4A34(dAnimal_c *animal);                                 // 800F4A34
BOOL fn_800F4AB8(dAnimal_c **animals);                               // 800F4AB8
BOOL fn_800F4C08(dAnimal_c *animal, int arg);                        // 800F4C08
BOOL fn_800F4D08();                                                  // 800F4D08
f32 getAnimalHandItemOfs(u8 idx, u32 axis);                                   // 800F4D84
f32 getSpHandItemOfs(const dItem::Item *key, u32 axis);                   // 800F4DB8

// Per-NPC communication permit. 3 bytes.
class dNpcPermit_c {
public:
    dNpcPermit_c();                                                  // 800F4E00
    ~dNpcPermit_c();                                                 // 800F4E40

    void clear();                                                    // 800F4E80
    void setState(u32 state);                                        // 800F4E94: clamped to 4
    BOOL hasState();                                                 // 800F4EF4
    BOOL isFlag(u32 i);                                              // 800F4F54
    void setFlag(u32 i);                                             // 800F4F74
    void set2(u32 value);                                            // 800F4F94: clamped to 4
    void reset2();                                                   // 800F4FF4
    BOOL fn_800F5000();                                              // 800F5000
    BOOL fn_800F5094();                                              // 800F5094

    /* 0x0 */ u8 mState; // 4: none
    /* 0x1 */ u8 mFlags; // one bit per player
    /* 0x2 */ u8 _2;     // 4: none
}; // size 0x3

namespace dNpc {

class permitMng_c {
public:
    permitMng_c(dNpcPermit_c *entries, u32 num) : mEntries(entries), mNum(num) {}
    static void *operator new(size_t, void *p) { return p; }
    virtual ~permitMng_c() {}
    virtual int getIdx(const dItem::Item *key) = 0;
    virtual void init();                                             // 800F513C

    dNpcPermit_c *get(u32 i);                                        // 800F51AC
    dNpcPermit_c *getByKey(const dItem::Item *key);                  // 800F51E4

    /* 0x4 */ dNpcPermit_c *mEntries;
    /* 0x8 */ u32 mNum;
}; // size 0xC

// 10 villagers (lbl_8074E568).
class normalPermitMng_c : public permitMng_c {
public:
    normalPermitMng_c() : permitMng_c(mList, 10) {}
    virtual ~normalPermitMng_c() {}                                  // 800F5648
    virtual int getIdx(const dItem::Item *key);                      // 800F5228

    /* 0xC */ dNpcPermit_c mList[10];
}; // size 0x2C

// 97 special NPCs (lbl_8074E56C).
class specialPermitMng_c : public permitMng_c {
public:
    specialPermitMng_c() : permitMng_c(mList, 97) {}
    virtual ~specialPermitMng_c() {}                                 // 800F55E0
    virtual int getIdx(const dItem::Item *key);                      // 800F52AC

    /* 0xC */ dNpcPermit_c mList[97];
}; // size 0x130

} // namespace dNpc

namespace EGG {
class Heap;
}
dNpc::permitMng_c *fn_800F52D4(const dItem::Item *key);              // 800F52D4
u32 fn_800F5370();                                                   // 800F5370: heap size for fn_800F5378
void fn_800F5378(EGG::Heap *heap);                                   // 800F5378
void fn_800F548C();                                                  // 800F548C
dNpcPermit_c *fn_800F54E0(const dItem::Item *key);                   // 800F54E0
void fn_800F5524(u8 player);                                         // 800F5524
BOOL fn_800F552C();                                                  // 800F552C
void fn_800F5548();                                                  // 800F5548
BOOL fn_800F5550();                                                  // 800F5550
void fn_800F55B0();                                                  // 800F55B0
void fn_800F55B4(int, int, int, u8 player);                          // 800F55B4
int fn_800F55BC();                                                   // 800F55BC
