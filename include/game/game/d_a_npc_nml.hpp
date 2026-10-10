#pragma once

// The normal (villager) npc actor. DOL TU d_a_npc_nml.cpp (.text 8002ED4C..80036324), not decompiled
// yet. Class names from the RTTI ("dAcNpcNml_c", "dAcNpcNml_c::talk_c", "dAcNpcNml_c::clothMng_c",
// "dAcNpcNml_c::resMng_c"). SCAFFOLD: only talk_c is sketched (its ctor 800308A8 plus the uses seen
// while scaffolding the d_npc_talk_* headers); the actor, clothMng_c and resMng_c are still to do.
// Field and helper names below are inferred; fn_ names are kept where the purpose isn't certain.
//
// The conversation code of talk_c is spread over the d_npc_talk_* TUs (d_npc_talk_quest_* for the
// quests). Each of those declares a non-polymorphic helper class deriving from talk_c with no data of
// its own (d_npc_talk_<name>.hpp); their member functions are stored in talk_c's member-function
// pointer slots and tables ({0, -1, fn} records, called through __ptmf_scall).
//
// Owner (dAcNpcNml_c) offsets seen from talk_c helpers: +0x1F94 dAnimal_c * (getAnimal),
// +0x1F98 a memory object (getEntry; u16 flags at +0x28C, dQuestBase_c at +0x23E, dNpcTimer_c at
// +0x250), +0x1FB0 the mood object (dNpcMood_c, size 0x1E0, d_npc_talk_mood), +0x19CC this talk_c,
// +0x16C0 action_c, +0xE8 anmSet_c.

#include <types.h>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>

class dAnimal_c;
class dPlayActor_c;
class dSceneChange_c;
class dAnimalMemory_c;
struct dSaveTownListEntry_c; // d_save_town.hpp
struct dSaveJinx_c;          // d_save_town.hpp
class dNpcEntry_c;
class dQuestVillager_c;
namespace dNpc {
class msgMemory_c;
class msgMemorySecond_c;
}

// The villager's mood: happy / angry / sad animations and effects on a timer (d_npc_talk_mood: ctor 800493EC,
// dtor 800494E8, non-polymorphic;
// name inferred). State int at +0x0, a member-function pointer at +0x4, dNpcTimer_c at +0x10, three
// dLevelEffect_c at +0x20 / +0xB4 / +0x148, flags at +0x1DC..+0x1DF. Only the dtor matters here (the
// weak dAcNpcNml_c dtor 8003602C calls it).
class dAcNpcNml_c;

class dNpcMood_c {
public:
    dNpcMood_c();  // 800493EC
    ~dNpcMood_c(); // 800494E8
    BOOL isEnabled();                           // 800495CC not mDisabled, and not online with more than 1 member
    void setAnm(dAcNpcNml_c *npc, u32 mood);    // 80049620 set the npc's wait / walk anm of the mood (0..4)
    void startTimer(int mood, int hours);       // 800496F0 start the timer: mood for hours * 3600 (called from nml startMood)
    static BOOL isMoodAnm(dAcNpcNml_c *npc);    // 80049754 current anm is a mood wait / walk anm (moods 1..4)
    static BOOL isMoodWaitAnm(dAcNpcNml_c *npc); // 80049830 current anm is a mood wait anm (moods 0..4)
    BOOL execute(dAcNpcNml_c *npc);             // 800498CC tick the npc entry's timer, update the anm, updateState
    BOOL enterState(dAcNpcNml_c *npc, u32 state); // 80049A64 enter state via the local static PTMF table 804A2F10 (5 entries)
    void updateState(dAcNpcNml_c *npc, int state); // 80049B30 state change by the current anm (0xC4..0xC9), run mProc
    BOOL enterHappy(dAcNpcNml_c *npc);          // 80049CA0 state 1 enter: mProc = procHappy
    void procHappy(dAcNpcNml_c *npc);           // 80049CCC state 1: sound 0x18CF, "afm_mnp_happy_walk_L/R"
    BOOL enterAngry(dAcNpcNml_c *npc);          // 80049D98 state 2 enter: mProc = procAngry
    void procAngry(dAcNpcNml_c *npc);           // 80049DC4 state 2: sound 0x18D1, "afm_mnp_pun_S"
    BOOL enterSad(dAcNpcNml_c *npc);            // 80049F00 state 3 enter: mProc = procSad
    void procSad(dAcNpcNml_c *npc);             // 80049F2C state 3: sound 0x18D0, "afm_mnp_sad_S"

    typedef BOOL (dNpcMood_c::*enterFunc)(dAcNpcNml_c *npc); // state enter (table of enterState)
    typedef void (dNpcMood_c::*stateFunc)(dAcNpcNml_c *npc); // per-frame state procedure (+0x4)

    // Three separate effects, not an array (the ctor / dtor handle each one inline).
    /* 0x000 */ int mState;          // 1..4 (updateState), 5 = none
    /* 0x004 */ stateFunc mProc;
    /* 0x010 */ dNpcTimer_c mTimer;  // pending mood timer: copied to the npc entry's timer by execute
    /* 0x020 */ dLevelEffect_c mEffectL;  // "afm_mnp_happy_walk_L", "afm_mnp_sad_S"
    /* 0x0B4 */ dLevelEffect_c mEffectR;  // "afm_mnp_happy_walk_R"
    /* 0x148 */ dLevelEffect_c mEffectPun; // "afm_mnp_pun_S" (followed while mPunOn)
    /* 0x1DC */ u8 mTimerPending;    // mTimer is to be copied
    /* 0x1DD */ u8 mShowEffects;     // init 1
    /* 0x1DE */ u8 mPunOn;
    /* 0x1DF */ u8 mDisabled;
}; // size 0x1E0

class dAcNpcNml_c : public dAcNpc_c {
public:
    // resMng_c / clothMng_c are defined after talk_c (see there): vtables are emitted in reverse
    // class-completion order, the target has dAcNpcNml_c, clothMng_c, resMng_c, talk_c.

    // The conversation handler of a villager (RTTI "dAcNpcNml_c::talk_c", vtable 804A14B8).
    class talk_c : public dAcNpc_c::recept_c {
    public:
        // Member-function pointer types of the procedure slots (by how d_a_npc_nml calls them).
        typedef int (talk_c::*msgFunc)(msgInfo_s *info); // +0xEC: picks the next message (label, code)
        typedef void (talk_c::*hookFunc)();              // +0x110..+0x134, choice answers
        // +0xF8: called once by onMessageStart (800313B0), which passes its own argument on (r4), so
        // the slot type takes it; the procedures ignore it.
        typedef void (talk_c::*endFunc)(int arg);
        // +0x104: per-frame step, cleared on TRUE. onMessageEnd (8003209C) passes its kind on (r4); the
        // procedures ignore it.
        typedef BOOL (talk_c::*stepFunc)(int kind);
        // Request handlers: the table of onMessageEnd (execReq, .data 804A0C70) and the slots
        // +0x140..+0x170; TRUE = handled.
        typedef BOOL (talk_c::*reqFunc)();

        // A procedure set: the 0x24-byte records of the nml tables 804A0784 / 804A0E48.
        struct procSet_s {
            msgFunc mMsg;   // -> +0xEC
            endFunc mHook;  // -> +0xF8
            stepFunc mStep; // -> +0x104
        };

        // Talk entries: the index into l_talkEntrySets (804A0E48) and the kind argument of getMsgLabel
        // (the label tables of the same TU).
        enum talkEntry_e {
            TALK_REACTION,       // 0  d_npc_talk_reaction (msgReaction)
            TALK_ARBEIT,         // 1  d_npc_talk_arbeit
            TALK_HALLOWEEN,      // 2  d_npc_talk_halloween
            TALK_COUNTDOWN,      // 3  d_npc_talk_countdown
            TALK_CARNIVAL,       // 4  d_npc_talk_carnival
            TALK_QUEST_DELIVERY, // 5  d_npc_talk_quest_delivery
            TALK_QUEST,          // 6  quest talk (msgQuestTalk; getMsgLabel idx = questTalk_e)
            TALK_ROLLAN,         // 7  d_npc_talk_rollan
            TALK_QUEST_OFFER,    // 8  msgQuestOffer
            TALK_RECALL,         // 9  msgRecall (no label table)
            TALK_HARVEST,        // 10 d_npc_talk_harvest
            TALK_FISHING,        // 11 d_npc_talk_fishing
            TALK_BUG,            // 12 d_npc_talk_bug
            TALK_FIREWORKS,      // 13 d_npc_talk_fireworks
            TALK_GREETING,       // 14 msgGreeting
            TALK_FREE,           // 15 d_npc_talk_free (the default entry)
            TALK_NUM
        };

        // Quest talk: the order of msgQuestTalk's table, the label tables of getMsgLabel(TALK_QUEST, idx, ..)
        // and the topic group of setTopic(KIND_QUEST, ..). Not dQuestKind_e order.
        enum questTalk_e {
            QUEST_TALK_ERRAND,        // 0  q06 QUEST_KIND_ERRAND_REQUEST
            QUEST_TALK_ERRAND_FINAL,  // 1  q07 QUEST_KIND_ERRAND_REQUEST_FINAL
            QUEST_TALK_INSECT,        // 2  q01 QUEST_KIND_REQUEST_INSECT
            QUEST_TALK_FISH,          // 3  q02 QUEST_KIND_REQUEST_FISH
            QUEST_TALK_FOSSIL,        // 4  q03 QUEST_KIND_REQUEST_FOSSIL
            QUEST_TALK_CLOTH,         // 5  q04 QUEST_KIND_REQUEST_CLOTH
            QUEST_TALK_FTR,           // 6  q05 QUEST_KIND_REQUEST_FTR
            QUEST_TALK_LOST_KEY,      // 7  q12 QUEST_KIND_REQUEST_6 (lost key)
            QUEST_TALK_SICK,          // 8  q11 QUEST_KIND_REQUEST_5 (sick villager)
            QUEST_TALK_STYLE,         // 9  q13 QUEST_KIND_STYLE
            QUEST_TALK_HOUSE_VISIT,   // 10 q08 QUEST_KIND_APPOINTMENT_1 (visits the player's house)
            QUEST_TALK_INVITATION,    // 11 q09 QUEST_KIND_APPOINTMENT_0 (invites the player over)
            QUEST_TALK_HIDE_AND_SEEK, // 12 q10 QUEST_KIND_HIDE_AND_SEEK
            QUEST_TALK_NUM
        };

        // Index of l_talkProcSets (setProcSet(&l_talkProcSets[i]), getProcSet(i)); named after each set's EC.
        enum talkProcSet_e {
            TALK_PROC_ENTRY,             // 0  msgEntry
            TALK_PROC_VISIT_CALL,        // 1  q08
            TALK_PROC_VISIT_FIRST,       // 2  q08
            TALK_PROC_VISIT_WAIT,        // 3  q08
            TALK_PROC_VISIT_BACK,        // 4  q08
            TALK_PROC_INVITE_WELCOME,    // 5  q09
            TALK_PROC_INVITE_FIRST,      // 6  q09
            TALK_PROC_INVITE_TRADE,      // 7  q09
            TALK_PROC_INVITE_WAIT,       // 8  q09
            TALK_PROC_INVITE_ANALOG,     // 9  q09
            TALK_PROC_HIDE_EXPLAIN,      // 10 q10
            TALK_PROC_HIDE_HIDER,        // 11 q10
            TALK_PROC_HIDE_REWARD,       // 12 q10
            TALK_PROC_HIDE_LOSE,         // 13 q10
            TALK_PROC_CANDY_ASK,         // 14 halloween
            TALK_PROC_HALLOWEEN_COSTUME, // 15 halloween
            TALK_PROC_APPROACH,          // 16 approach
            TALK_PROC_TOWN,              // 17 town
            TALK_PROC_TOWN_THEATER,      // 18 town
            TALK_PROC_TOWN_GRACE,        // 19 town
            TALK_PROC_TOWN_3P,           // 20 town
            TALK_PROC_FMARKET,           // 21 fmarket
            TALK_PROC_SALE,              // 22 fmarket
            TALK_PROC_STALL_CALL,        // 23 fmarket
            TALK_PROC_STALL_WAIT,        // 24 fmarket
            TALK_PROC_STALL_BACK,        // 25 fmarket
            TALK_PROC_STALL_NO_ITEM,     // 26 fmarket
            TALK_PROC_STALL_ITEM,        // 27 fmarket
            TALK_PROC_PLAYER_BIRTHDAY,   // 28 birthday
            TALK_PROC_NPC_BIRTHDAY,      // 29 birthday
            TALK_PROC_ETC_HIT,           // 30 free
            TALK_PROC_ETC_PUSH,          // 31 free
            TALK_PROC_ETC_FLEA,          // 32 free
            TALK_PROC_NUM
        };

        talk_c(); // 800308A8

        // d_a_npc_nml helpers used by the talk TUs (signatures provisional).
        // The PTMF setters take the pointer BY VALUE.
        void setActProc(hookFunc proc);           // +0x110 = proc (action)
        void clearActProc();                        // +0x110 = null
        void setProcSet(const procSet_s *set);    // +0xEC/+0xF8/+0x104 from set (null-checked)
        void setProcs(msgFunc msg, endFunc hook, stepFunc step); // all three
        void setMsgProc(msgFunc proc);            // +0xEC = proc
        void setHookProc(endFunc proc);            // +0xF8 = proc
        void setStepProc(stepFunc proc);           // +0x104 = proc
        dAnimal_c *getAnimal();                  // the villager's dAnimal_c (owner+0x1F94)
        dAnimal_c *getAnimal() const;            // same body (const overload)
        int getNpcIdx();                         // the villager index (< 10)
        dNpcEntry_c *getEntry();                // owner+0x1F98
        dNpcEntry_c *getEntry() const;          // same body (const overload)
        BOOL isHoldingTool(int toolType);            // the npc holds a tool of this type
        BOOL startMsg();                        // runs +0xEC with a stack msgInfo_s, starts the message
        void setTopic(u32 kind, u8 group, u8 idx); // +0x1F0/+0x327/+0x328 (msgMemory_c::set arguments; kind = msgMemory_c::kind_e)
        // (choice-table helpers clearChoice..showChoice and setLooksMsg: below)

        // ---- talk_c functions by range (each matching agent adds its own; keep address order) ----
        // 800309A0..80031C00
        virtual ~talk_c();                         // 800309A0
        virtual void init();                       // 80030A14 (+0x30)
        void recordMemoryTalk(dAnimalMemory_c **memory, u32 *memoryIdx, dAnimal_c *animal, int npcIdx,
                         const dLandID_c *land);   // 80030F18: finds/creates the player's memory, records the talk
        void recordTalk(const dLandID_c *land);   // 800310A0: recordMemoryTalk for mpMemory/mMemoryIdx unless mNoRecordTalk
        void fixMsgCode(u16 *code);               // 80031114: code 0 -> 1 (and mPickCode = 0)
        void setTopicFrom(const dNpc::msgMemory_c *mem); // 80031144: setTopic(mem->mKind, mTopicGroup, mTopicIdx)
        void start(dAcNpc_c *npc, dActor_c *actor, const procSet_s *set); // 80031160: start (RELs; npc unused)
        virtual void preExecute();                 // 80031254 (+0x94): runs mActProc
        virtual void onRequestEnd(int kind);       // 8003129C (+0xA0): runs mResultProc, then mNextResultProc
        virtual void getMsgInfo(msgInfo_s *info);  // 8003134C (+0x9C)
        virtual void onMessageStart(int arg);          // 800313B0 (+0x10): runs the one-shot mHookProc
        void resSell();                        // 80031448 result of reqSell
        BOOL reqSell();                        // 80031468 (request table 804A0C70 [1]): sell mItems[0] to the player
        void resBuy();                        // 800315A8 result of reqBuy
        BOOL reqBuy();                        // 800315D4 (804A0C70 [2]): buy mItems[0] from the player
        void resTrade();                        // 8003178C result of reqTrade
        BOOL reqTrade();                        // 800317B8 (804A0C70 [3]): trade mItems[0] for mItems[2]
        BOOL reqGive();                        // 80031960 (804A0C70 [4]): give mItems[0]
        BOOL reqTake();                        // 800319F4 (804A0C70 [5]): take mItems[0] from the pockets
        BOOL reqLetter();                        // 80031AE8 (804A0C70 [6]): mail menu on the animal's letter
        void resHabit();                        // 80031B3C result of reqHabit: stores the new habit
        BOOL reqHabit();                        // 80031BC0 (804A0C70 [12]): habit text input
        // 80031C00..80032D00
        // The answer table of the choice menu (+0x214), shown on the controller's dMsgSelect_c by
        // showChoice: answer i shows message code mEntry[i].mCode + rndF(mEntry[i].mRange); after the
        // menu onAnswer runs mProc[cursor].
        class choice_c {
        public:
            struct entry_s {
                /* 0x0 */ u16 mCode;
                /* 0x2 */ u8 mRange;
            }; // size 0x4

            void clear();                                            // 80032174 clear
            void setAnswer(int idx, u16 code, u8 range, hookFunc proc); // 80032188 answer idx (< 5)
            void setNum(u32 num);                                     // 8003224C answer count
            void setCancel(int cancel);                                  // 8003225C cancel answer
            BOOL hasAnswers() const;                                      // 80032354 any answers

            /* 0x00 */ entry_s mEntry[5];
            /* 0x14 */ hookFunc mProc[5];
            /* 0x50 */ u32 mNum;
            /* 0x54 */ int mCancel; // -1: none
        }; // size 0x58

        // The two-way npc select (+0x26C), shown on the controller's dMsgAnalogSelect_c by showNpcChoice
        // with "sys_SELECT/SYS_SelectNPC" codes; onAnswer runs mProc with the cursor in mAnswer.
        class npcChoice_c {
        public:
            void clear();                                            // 80032370 clear
            void set(u16 code0, u8 range0, u16 code1, u8 range1, hookFunc proc); // 80032384
            bool isSet() const;                                      // 800324A4 mProc set

            /* 0x00 */ choice_c::entry_s mEntry[2];
            /* 0x08 */ hookFunc mProc;
        }; // size 0x14

        // A message's attribute record in the script resource (dScript::Res_c::getEntry), read by
        // selectMessage to pick a message at random. Each condition: 0 = any, else value + 1 (mDay: value).
        struct msgAttr_s {
            /* 0x00 */ u8 _00[0xD];
            /* 0x0D */ u8 mTimeOfDay;  // dTime_c::getTimeOfDay
            /* 0x0E */ u8 mGender;     // the player's gender
            /* 0x0F */ u8 mSeason;     // dTime_c::getCurrentSeason
            /* 0x10 */ u8 mFriendship; // dAnimalMemory_c::getFriendshipLevel
            /* 0x11 */ u8 mMonth;      // 0-based month
            /* 0x12 */ u8 mDay;        // day of the month (not + 1)
            /* 0x13 */ u8 mFixed;      // never picked at random
        };

        void resNickname();                        // 80031C44 result of reqNickname: stores the nickname
        BOOL reqNickname();                        // 80031CB8 (804A0C70 [13]): nickname text input
        void resGreeting();                        // 80031D2C result of reqGreeting: stores the greeting
        BOOL reqGreeting();                        // 80031DA0 (804A0C70 [15]): greeting text input
        BOOL reqProc0();                        // 80031E14 (804A0C70 [16]): runs mReqProc
        BOOL reqProc1();                        // 80031E64 (804A0C70 [17]): runs _14C
        BOOL reqProc2();                        // 80031EB4 (804A0C70 [18]): runs _158
        BOOL reqProc3();                        // 80031F04 (804A0C70 [19]): runs _164
        BOOL reqProc4();                        // 80031F54 (804A0C70 [20]): runs _170
        BOOL execReq(int kind);                // 80031FA4 runs the request handler of kind (< 21)
        virtual void onMessageEnd(int kind);         // 8003209C (+0x14): handler, else mStepProc
        void clearChoice();                        // 80032180 mChoice.clear
        void setChoice(int idx, u16 code, u8 range, hookFunc proc); // 800321C4 answer idx
        void setChoiceProc(int idx, hookFunc proc);  // 80032204 answer idx without a message code
        void setChoiceNum(u32 num);                 // 80032254 answer count
        void setChoiceCancel(int cancel);              // 80032264 cancel answer (-1: none)
        void showChoice() const;                  // 8003226C shows the choice menu
        BOOL hasChoice() const;                  // 80032368 mChoice has answers
        void clearNpcChoice();                        // 8003237C mNpcChoice.clear
        void setNpcChoice(u16 code0, u8 range0, u16 code1, u8 range1, hookFunc proc); // 800323B0
        void showNpcChoice() const;                  // 800323F0 shows the npc select
        bool hasNpcChoice() const;                  // 800324D4 mNpcChoice is set
        virtual void onAnswer(int arg);          // 800324DC (+0x18): runs the chosen answer
        virtual void onTalkEnd();                       // 800325FC (+0x98): runs mTalkEndProc once, flags the entry
        static const procSet_s *getProcSet(int idx); // 8003267C record idx (< 33) of the table 804A0784
        dNpc::msgMemorySecond_c *getRememberedMsg();    // 800326A0 the entry's mMsg2
        dNpc::msgMemorySecond_c *getRememberedMsg() const; // 800326D4 same (const)
        const char *getRememberedLabel(u16 *code, int kind) const; // 80032708 remembered label (KIND_ANY: any)
        BOOL rememberMsg(const char *label, u16 code); // 80032798 remembers the message in mMsg2
        void setLooksMsg(msgInfo_s *info, const char *label, u16 code); // 800328EC label + looks into mLabel
        virtual void selectMessage(const void *bmg);  // 80032988 (+0x0C): picks a message code at random
        virtual dAnimal_c *getSpeakerAnimal();           // 80032BD8 (+0x7C): the speaker's dAnimal_c
        virtual dAnimal_c *getListenerAnimal();           // 80032C50 (+0x80): the listener's dAnimal_c
        virtual int isSpeakerItchy();                  // 80032CCC (+0x84): the speaker's isItchy
        // The speaker / the partner (these call getNpc const: const helpers).
        int getTalkIdx() const { return mTalkIdx; }
        // The controller's choice menus (NULL without a controller).
        dMsgSelect_c *getSelect() const {
            return getController() != NULL ? getController()->getSelect() : NULL;
        }
        dMsgAnalogSelect_c *getAnalogSelect() const {
            return getController() != NULL ? getController()->getAnalogSelect() : NULL;
        }
        dAcNpc_c *getTalkNpc() const { return getNpc(getTalkIdx()); }
        dAcNpc_c *getPartnerNpc() const { return getNpc(1); }
        // 80032D00..80033E00
        typedef const procSet_s *(talk_c::*setFunc)(); // event procedure-set pickers (table 8046BDBC)
        virtual int getVoiceMood();                  // 80032D1C (+0x8C): mood (0..4) by the entry's timer mode
        BOOL startMood(int mood, int hours);             // 80032D7C starts the speaker's mood timer
        virtual void resetMood();                 // 80032E04 (+0x38): startMood(0, 0)
        virtual void setMood1(int hours);        // 80032E10 (+0x3C): startMood(1, hours)
        virtual void setMood2(int hours);        // 80032E1C (+0x40): startMood(2, hours)
        virtual void setMood3(int hours);        // 80032E28 (+0x44): startMood(3, hours)
        virtual void setMood4(int hours);        // 80032E34 (+0x48): startMood(4, hours)
        virtual void playTownTune();                 // 80032E40 (+0x74): action: lock, then play the town tune (actPlayTune)
        void actPlayTune();                        // 80032E94 action: start the town tune
        void actWaitTuneStart();                        // 80032F7C action: wait until the tune plays
        void actWaitTuneEnd();                        // 80032FD8 action: wait for its end, unlock
        static const char *getLabelByTable(int table, u32 kind, u32 idx, u32 sub); // 80033028 label by table (0, 0x10, 0x14)
        static const char *getMsgLabel(u32 kind, u32 idx, u32 sub);  // 800330AC message label from per-TU tables
        static BOOL filterItem(const dItem::Item *item, int flag);   // 800334CC pocket filter (countPocketsItem)
        static int countPocketsItem(const dItem::Item *item, int flag, u16 *mask, dPrivateData_c *player); // 80033510
        static int countPocketsKind(u16 *mask, int kind, dPrivateData_c *player, BOOL noSaleToo); // 800335B4 pockets of a BITM kind
        static BOOL filterUsed(const dItem::Item *item, int flag);   // 800336C8 pocket filter (countPocketsUsed)
        static int countPocketsUsed(u16 *mask, dPrivateData_c *player);    // 800336EC occupied pockets
        BOOL setMemoryText(const wchar_t *name);     // 80033770 copies a name (16 chars max) to mMemoryText
        int msgEntry(msgInfo_s *info);          // 800337F8 first msgFunc of the 804A0E48 sets that answers
        const procSet_s *getEventProcSet();            // 80033880 first event set (fishing, bug, fireworks)
        const msgFunc getReactionMsg(int idx);        // 80033904 reaction msgFunc idx (< 28; table 8046BDE0)
        int msgReaction(msgInfo_s *info);          // 80033958 EC of 804A0E48[0]: tries the 28 reactions
        void endReaction(int arg);                 // 80033A60 end hook (F8 slot of l_talkEntrySets[0]): memory, talk count, clear msgMemorySecond
        BOOL stepReaction(int kind);                // 80033ADC 104 (reactions): clear the entry's quest
        int countPocketsFossil(u16 *mask, dQuestVillager_c *quest, dPrivateData_c *player); // 80033B5C fossil pockets (q03)
        void setQ4Word(int mode, int idx);       // 80033C54 word idx = the villager's liked look (q04)
        void setQ5Word(int mode, u32 value, int idx); // 80033CF0 word idx = part/color/image/series (q05)
        // 80033E00..80035C08
        int msgQuestTalk(msgInfo_s *info);          // 80033E68 EC (804A0E48 [6]): first quest talk of 8046BF30 that picks a message
        void endQuestTalk(int arg);                 // 80033F00 F8 (804A0E48 [6]): memory, msgMemory_c::set, talk count 0x44
        BOOL stepErrandOver(int kind);                // 80033FA4 104: errand over: takes the item back (Q_Payback) or Q_Lost
        int msgPayback(msgInfo_s *info);          // 80034164 EC "Q_Payback"
        int msgLost(msgInfo_s *info);          // 80034194 EC "Q_Lost"
        int msgTimeover2(msgInfo_s *info);          // 800341C0 EC "Q_Timeover2", 104 = stepClearErrand
        BOOL stepClearErrand(int kind);                // 80034224 104: clears the player's errand
        void selResumeTalk();                        // 80034290 choice: getEventProcSet() set (default 804A0E48 [15]), run EC
        int msgCancel(msgInfo_s *info);          // 800342E4 EC "Q_Cancel"
        int startQuestOffer(msgInfo_s *info, int labelIdx, endFunc hook, stepFunc step, BOOL checkTalk); // 80034314 quest offer
        static BOOL isEventOngoing();                 // 8003451C one of the 3 events of 8046BFCC is ongoing
        int startErrandOffer(msgInfo_s *info, int questKind, int labelIdx, stepFunc step); // 80034588 errand offer
        int startRequestOffer(msgInfo_s *info, int labelIdx, int questKind, stepFunc step); // 80034750 villager request offer
        int msgPendingQuest(msgInfo_s *info, f32 chance); // 800348C0 by chance %: the offer check (8046BFD8) of the npc entry's pending quest kind
        int msgQuestOffer(msgInfo_s *info);          // 800349C0 EC (804A0E48 [8]): msgPendingQuest 10% / 20%
        void endQuestOffer(int arg);                 // 80034A3C F8: memory, msgMemory_c::set, talk count 0x44
        int msgYes(msgInfo_s *info);          // 80034ACC EC "Q_Yes" (code 4..6 by mErrandDeadline)
        void selNo();                        // 80034B20 choice: EC = msgNo, run EC
        int msgNo(msgInfo_s *info);          // 80034B74 EC "Q_No"
        void selRequestNo();                        // 80034BA0 choice: request refused (clear / clearPlayerFlag), EC = msgRequestNo
        int msgRequestNo(msgInfo_s *info);          // 80034C4C EC "Q_No"
        BOOL stepRequestChoice(int kind);                // 80034C78 104: yes/no choice (selRequestYes / selRequestNo)
        void selRequestYes();                        // 80034D78 choice: request accepted, EC = msgRequestYes
        int msgRequestYes(msgInfo_s *info);          // 80034E18 EC "Q_Yes1/2/34" by player count, 104 = stepRequestYes
        BOOL stepRequestYes(int kind);                // 80034ED0 104: sound 0x171E
        int msgRecall(msgInfo_s *info);          // 80034F08 EC (804A0E48 [9]): repeats the remembered message (msgMemorySecond_c)
        void endRecall(int arg);                 // 800357DC F8: memory, msgMemory_c::set
        int msgGreeting(msgInfo_s *info);          // 80035834 EC (804A0E48 [14]): greeting by talk count / weather (getMsgLabel(TALK_GREETING, ..))
        void endGreeting(int arg);                 // 80035A58 F8: memory, talk count 0x44
        BOOL stepGreeting(int kind);                // 80035AA0 104: once idle, procedures 804A0E48 [15]
        void addFriendship(int delta);               // 80035B04 friendship += delta (current player's memory)
        inline dSaveTownListEntry_c *getJinxTown(); // inline (defined in the .cpp): town list entry mJinxTown (land and jinx), NULL = this town

        // ---- Procedures of the d_npc_talk_* TUs. They are talk_c members: the nml tables store
        // them as constant {0,-1,fn} records, which MWCC only emits for members of talk_c itself
        // (a derived-class member pointer cast to talk_c is initialised at run time). ----
        // ---- d_npc_talk_approach (.text 80036324..80038134) ----
        // Label of an approach message: group 0 = 804A1C08 (4: "ApD_Moving", "ApD_Fortune" x3),
        // 1 = 804A1C70 (7: ApB_Habit/Hello/Nickname, ApC_Present/Sell/Trade/Want), 3 = .sdata 80749970
        // (2: "ApA_Letter", "ApA_Always"); NULL otherwise (group 2 has none). Also called by nml
        // getLabelByTable. The third argument is unused.
        static const char *getApproachLabel(int group, u32 idx, int unused); // 80036324

        // Ap* names: the message label without its group letter. Fortune: the player's fortune state
        // (dPrivateData_c+0x83F9: 1 love, 2 friendship, 4 item luck).
        int reqApMoveOut();                         // 800363A4 140 (after ApD_Moving): the moving-out villager becomes the moving-in index (mMoveOutIdx -> mMoveInIdx), clears its memory flag 23
        int msgApMoving(msgInfo_s *info);           // 8003643C EC ApD_Moving: villager is moving out (memory flag _23 clear); sets the flag, 140 = reqApMoveOut
        int msgApFortuneLove(msgInfo_s *info);      // 80036564 EC ApD_Fortune (codes 1..3): player fortune state 1, gender differs; 104 = stepApFortune
        int msgApFortuneFriend(msgInfo_s *info);    // 80036698 EC ApD_Fortune (codes 4..6): player fortune state 2, same gender; 104 = stepApFortune
        int msgApFortuneItem(msgInfo_s *info);      // 800367CC EC ApD_Fortune (codes 7..9): player fortune state 4; F8 = endApFortuneItem
        int msgApD(msgInfo_s *info);                // 800368DC EC: first of the ApD checks 8046C088 that picks a message
        int msgApHabit(msgInfo_s *info);            // 80036974 EC ApB_Habit (dAnimal_c::mHabitCooldown == 0)
        int msgApHello(msgInfo_s *info);            // 80036A28 EC ApB_Hello (memory mGreetingWait == 0)
        int msgApNickname(msgInfo_s *info);         // 80036ADC EC ApB_Nickname: friendship > 0, no nickname given lately (isNicknameWaiting); 104 = stepApNickname
        int msgApPresent(msgInfo_s *info);          // 80036C28 EC ApC_Present: player has an empty pocket; F8 = endApPresent
        int msgApSell(msgInfo_s *info);             // 80036D08 EC ApC_Sell: empty pocket and >= 2000 bells; F8 = endApSell

        // Picks a random sellable item from the player's pockets (player NULL: current player); prefers
        // the item kind the villager's wish asks for (8046C0B8 by dQuestWish_c::mKind). Invalid if none.
        static dItem::Item pickWantedPocketItem(const dAcNpcNml_c::talk_c *talk, dPrivateData_c *player); // 80036E04

        int msgApTrade(msgInfo_s *info);            // 80037038 EC ApC_Trade: player has a sellable item; F8 = endApTrade
        int msgApWant(msgInfo_s *info);             // 80037108 EC ApC_Want: player has a sellable item; F8 = endApWant
        int msgApBC(msgInfo_s *info);               // 800371D8 EC: up to 3 random tries over the 7 ApB/ApC checks 8046C0D8
        int msgApPendingQuest(msgInfo_s *info);     // 800372CC EC: nml msgPendingQuest(info, 25.0f) (25% chance)
        int msgApLetter(msgInfo_s *info);           // 800372D4 EC ApA_Letter: villager holds letters, none from the player
        int msgApAlways(msgInfo_s *info);           // 800373BC EC ApA_Always (fallback)
        int msgApA(msgInfo_s *info);                // 80037448 EC: random one of 8046C130 (Letter / Always), else ApA_Always
        int msgApproach(msgInfo_s *info);           // 800374DC EC root (nml 804A0784+0x240): isSpeakerItchy() ? ApA_Always : first of 8046C148
        void endApproach(int arg);                  // 8003759C F8 (nml 804A0784+0x240): recordTalk(0), talk count 0x44, dNpc::msgMemory_c::set
        BOOL stepApFortune(int kind);               // 8003762C 104: once idle, sets memory flag _20 (fortune remark made)
        void endApFortuneItem(int arg);             // 8003766C F8 (fortune): lucky item into mItem0 (fn_800F4510), EC = msgApFortuneGift or msgApFortuneFull (pockets full)
        int msgApFortuneGift(msgInfo_s *info);      // 800377A8 EC ApD_Fortune, code = mMessageCode + 3
        int msgApFortuneFull(msgInfo_s *info);      // 8003781C EC ApD_Fortune, code 13
        BOOL stepApNickname(int kind);              // 80037888 104: choice 0 = selApNicknameNew
        void selApNicknameNew();                    // 8003791C choice: saves the old nickname to mName, makes a new one, 104 = stepApNicknameChoice
        BOOL stepApNicknameChoice(int kind);        // 80037A08 104: choices selApNicknameKeep / selApNicknameUndo
        void selApNicknameKeep();                   // 80037AC8 choice: memory mNicknameWait = 14
        void selApNicknameUndo();                   // 80037AE8 choice: friendship < 64: restores the nickname from mName
        void endApPresent(int arg);                 // 80037B58 F8 ApC_Present: gift item into mItem0, word 0
        void endApSell(int arg);                    // 80037BE0 F8 ApC_Sell: villager's item (dAnimal_c::pickNewItem) and price (mPrice)
        void endApTrade(int arg);                   // 80037E10 F8 ApC_Trade: villager's item mItem0, player's item mItem2 (pickWantedPocketItem)
        void endApWant(int arg);                    // 80037F1C F8 ApC_Want: player's item (pickWantedPocketItem) and offered price (mPrice)
        static BOOL isNicknameWaiting();            // 80038078 any villager's memory of the current player has mNicknameWait set (nickname given lately)
        // ---- d_npc_talk_arbeit (.text 80038134..80039B38) ----
        // The newcomer's first jobs (errands QUEST_KIND_FIRSTJOB_*, dPrivateData_c::mErrand; private flag
        // 0xD set while the player is a newcomer). Check procedures of the table 8046C178 (tried in order
        // by msgArbeit); each returns TRUE when it picked the message.
        int msgArbeitMoveIn(msgInfo_s *info);       // 80038134 moving-in remark (dAnimal_c::isMovingIn, private flag 0xD)
        int msgArbeitMoveOut(msgInfo_s *info);      // 80038238 moving-out remark (dAnimal_c::isMovingOut)
        BOOL msgArbeitErrand(msgInfo_s *info, u8 errandKind, u8 errandState, u8 labelIdx); // 80038344 errand of this npc of the kind/state, item in pockets; 104 = stepArbeitErrandChoice
        int msgArbeitDeliverFtr(msgInfo_s *info);   // 80038570 msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_FTR, 0, 2)
        int msgArbeitDeliverCarpet(msgInfo_s *info); // 80038580 msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_CARPET, 0, 3)
        int msgArbeitDeliverCan(msgInfo_s *info);   // 80038590 msgArbeitErrand(info, QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN, 0, 4)
        int msgArbeitNewcomer(msgInfo_s *info);     // 800385A0 private flag 0xD set; 104 = stepArbeitNewcomerChoice
        int msgArbeit(msgInfo_s *info);             // 800386A0 EC: tries the 6 checks of 8046C178 (entry in nml table 804A0E48+0x24)
        void endArbeit(int arg);                    // 80038738 F8: msgMemory set, talk count 0x44 when mCountTalk
        BOOL stepArbeitErrandChoice(int kind);      // 800387BC 104: two choices (selArbeitGive / selArbeitOther)
        void selArbeitGive();                       // 80038894 choice: select the errand item (reqSelectItem), 128 = resArbeitGive
        void resArbeitGive();                       // 80038928 128: take the selected pocket item, requestItemActEx; 134 = resArbeitGiven (else EC = msgArbeitCancel)
        void resArbeitGiven();                      // 80038A94 134: EC = msgArbeitGiven, close the message
        int msgArbeitGiven(msgInfo_s *info);        // 80038AF0 EC: errand result message by errand kind (FIRSTJOB_DELIVER_FTR..WATERING_CAN)
        BOOL stepArbeitFtrDone(int kind);           // 80038D4C 104: requestHandActD, EC = msgArbeitFtrReward
        int msgArbeitFtrReward(msgInfo_s *info);    // 80038DCC EC: reward item (fn_800C60B4, default ITEM_IDX_EXOTIC_BED) to mItem0, pickUp
        BOOL stepArbeitFtrReward(int kind);         // 80038EB4 104: requestItemAct of the reward, EC = msgArbeitBirthdayAsk
        int msgArbeitBirthdayAsk(msgInfo_s *info);  // 80038F40 EC code 0x16, 104 = stepArbeitBirthdayMenu
        BOOL stepArbeitBirthdayMenu(int kind);      // 80038FA4 104: reqMenu1C (birthday entry), 128 = resArbeitBirthday
        void resArbeitBirthday();                   // 80039014 128: EC = msgArbeitBirthdayCheck
        int msgArbeitBirthdayCheck(msgInfo_s *info); // 80039068 EC: player's birthday month/day as words 2/3
        BOOL stepArbeitBirthdayChoice(int kind);    // 80039134 104: two choices (selArbeitBirthdayYes / selArbeitBirthdayNo)
        void selArbeitBirthdayYes();                // 8003920C choice: EC = msgArbeitBirthdayYes, errand state 2
        int msgArbeitBirthdayYes(msgInfo_s *info);  // 80039278 EC: today is the player's / the npc's birthday
        void selArbeitBirthdayNo();                 // 80039364 choice: EC = msgArbeitBirthdayNo
        int msgArbeitBirthdayNo(msgInfo_s *info);   // 800393B8 EC code 0x19, 104 = stepArbeitBirthdayMenu (asks again)
        BOOL stepArbeitCarpetReward(int kind);      // 8003941C 104: requestItemAct of mItem0, EC = msgArbeitCarpetThanks
        int msgArbeitCarpetThanks(msgInfo_s *info); // 800394A8 EC code 0x1F, 104 = stepArbeitErrandDone
        BOOL stepArbeitErrandDone(int kind);        // 8003950C 104: errand state 2
        BOOL stepArbeitErrandNext(int kind);        // 80039564 104: errand state 1
        BOOL stepArbeitLetterMenu(int kind);        // 800395BC 104: reqMailMenu (the npc's letter, dAnimal_c::mLetter), EC = msgArbeitLetter
        int msgArbeitLetter(msgInfo_s *info);       // 80039658 EC code 0x21, 104 = stepArbeitErrandNext
        int msgArbeitCancel(msgInfo_s *info);       // 800396BC EC: "Q_Cancel"
        void selArbeitOther();                      // 800396EC choice: EC = msgArbeitProgress
        int msgArbeitProgress(msgInfo_s *info);     // 80039740 EC: code from getArbeitProgressCode, 104 = stepArbeitProgressNext
        BOOL stepArbeitNewcomerChoice(int kind);    // 800397DC 104: two choices (selArbeitProgress / selArbeitNo)
        static u16 getArbeitProgressCode();         // 800398B4 6 + player byte at dPrivateData_c+0x8696
        void selArbeitProgress();                   // 800398F4 choice: EC = msgArbeitProgressCheck
        int msgArbeitProgressCheck(msgInfo_s *info); // 80039948 EC: dPrivateData_c::incNewcomerTip when it reaches 0xB with 4 players
        BOOL stepArbeitProgressNext(int kind);      // 80039A44 104: dPrivateData_c::incNewcomerTip
        void selArbeitNo();                         // 80039A94 choice: EC = msgArbeitNo
        int msgArbeitNo(msgInfo_s *info);           // 80039AE8 EC code 4
        // ---- d_npc_talk_free (.text 80039B38..8003F354) ----
        typedef BOOL (talk_c::*condFunc)(); // condition procedures (tables 8046C1C0, 8046C4F8)
        // Special-day conditions (PTMF table 8046C1C0, getSpecialDay); this is unused.
        BOOL isLeapDay();                           // 80039B38 today is Feb 29 (dTime_c::getCurrent: month 1, day 29)
        BOOL isAprilFools();                        // 80039B7C dEvent::isOngoing(EVENT_APRIL_FOOLS_DAY)
        BOOL isWeatherTopic();                      // 80039BAC weather topic: by hour (fn_801C9C08) or getWeatherPhaseA/B
        BOOL isMoonJPN();                           // 80039C34 dEvent::isActive(EVENT_JP_AUTUMN_MOON)
        BOOL isMoonKOR();                           // 80039C64 dEvent::isActive(EVENT_KR_DAEBOREUM)
        BOOL isMoonUSA();                           // 80039C94 dEvent::isActive(EVENT_NA_AUTUMN_MOON)
        BOOL isMoonEUR();                           // 80039CC4 dEvent::isActive(EVENT_EU_AUTUMN_MOON)
        int getSpecialDay();                        // 80039CF4 index of the first true condition of 8046C1C0, 7 if none

        static BOOL isEventDay(BOOL withMoon);      // 80039D7C one of the 30 events of 8046C218 is active (withMoon: or one of the 5 of 8046C290)
        const u8 *getFreeWeights();                 // 80039E38 group weights (10 x u8, rows in 8046C290) for 8046C5A0: by town / visitor / special day / event
        static int pickFriendTown();                // 80039F74 random other town (0..7) the current player is registered in, -1 if none
        dAnimalMemory_c *pickOtherMemory();         // 8003A0FC random memory of this villager about another player (not the current one)
        const u8 *getGeneralWeights();              // 8003A284 FreeA weights (7 x u8, .sdata2 8074FFC0..80750010) for 8046C350

        // FreeA general topics (8046C350)
        int msgFreeClothes(msgInfo_s *info);        // 8003A3DC EC FreeA_Clothes: the player's shirt / hat / face item; 104 = stepFreeClothes
        int msgFreeAlways(msgInfo_s *info);         // 8003A814 EC FreeA_Always (fallback of msgFreeGeneral)
        void setDesignWords(const dPersonalID_c *creator, const wchar_t *designName, dAnimal_c *animal); // 8003A894 words 1..3: design creator name and land, design name (also nml msgRecall)
        int msgFreeDress(msgInfo_s *info);          // 8003A998 EC FreeA_Dress: the villager's shirt design and its creator
        int msgFreeFriend(msgInfo_s *info);         // 8003AB40 EC FreeA_Friend: two random villagers and their relation

        // Memory filters of msgFreeMemory (plain function-pointer table 8046C320, slots 1..11, code = slot + 1);
        // mem = the villager's memory picked by dAnimal_c::getRandomMemory, sameTown = its player is of this town.
        // "Away": the player is of another town but was met here (mem->mFlags.mSameTown); "Visitor": never was.
        static BOOL isMemLevel0(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);     // 8003AD10 sameTown, friendship level 0
        static BOOL isMemLevel2(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);     // 8003AD4C sameTown, friendship level 2
        static BOOL isMemLevel1(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);     // 8003AD88 sameTown, friendship level 1
        static BOOL isMemLetter(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);     // 8003ADC4 sameTown, has a letter from that player
        static BOOL isMemPresent(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);    // 8003AE0C sameTown, memory present (mPresent) set
        static BOOL isMemVisitor(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown);    // 8003AE30 !sameTown, mFlags.mSameTown clear
        static BOOL isMemAwayLetter(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE54 !sameTown, has a letter from that player
        static BOOL isMemAwayLevel0(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE9C !sameTown, mFlags.mSameTown, friendship level 0
        static BOOL isMemAwayLevel2(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AEE4 !sameTown, mFlags.mSameTown, friendship level 2
        static BOOL isMemAwayLevel1(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AF2C !sameTown, mFlags.mSameTown, friendship level 1
        static BOOL isFromOtherTown(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AF74 the villager's previous town (dAnimal_c::mPrevLand) is another town

        void setMemoryWords(const dPersonalID_c *pid, const dLandID_c *land, u32 impress, const dItem::Item *item, dAnimal_c *animal); // 8003B018 words 0..4 of the memory topic, impression (mImpression), item (mItem4) (also nml msgRecall)
        int msgFreeMemory(msgInfo_s *info);         // 8003B174 EC FreeA_Memory: code 1..12 by the filters of 8046C320
        int msgFreeRumor(msgInfo_s *info);          // 8003B3B4 EC FreeA_Rumor: news of a visited town (mRumorTown = its index)
        void setJinxWords(const dSaveJinx_c *jinx, const dLandID_c *land); // 8003B858 jinx words: bug / fish or count, the town (also nml msgRecall)
        int msgFreeJinx(msgInfo_s *info);           // 8003B960 EC FreeA_Jinx: jinx of another town (mJinxTown = its index); 104 = stepJinx
        int msgFreeGeneral(msgInfo_s *info);        // 8003BA84 EC: FreeA topic by the weights of getGeneralWeights, else FreeA_Always

        // A2 special days (8046C3A4)
        int msgFree0229(msgInfo_s *info);           // 8003BBC8 EC FreeA_0229
        int msgFree0401(msgInfo_s *info);           // 8003BC48 EC FreeA_0401: land of a random remembered player of another town
        int msgFreeWeather(msgInfo_s *info);        // 8003BE24 EC FreeA_Weather: code by hour / weather (50%)
        int msgFreeMoonJPN(msgInfo_s *info);        // 8003BF70 EC FreeA_MoonJPN (code 1 day, 2 night)
        int msgFreeMoonKOR(msgInfo_s *info);        // 8003C01C EC FreeA_MoonKOR
        int msgFreeMoonUSA(msgInfo_s *info);        // 8003C0C8 EC FreeA_MoonUSA
        int msgFreeMoonEUR(msgInfo_s *info);        // 8003C174 EC FreeA_MoonEUR
        int msgFreeSpecialDay(msgInfo_s *info);     // 8003C220 EC: special-day topic of getSpecialDay (in the own town outdoors)

        // FreeB hobby topics
        int msgFreeBug(msgInfo_s *info);            // 8003C2BC EC FreeB_Bug: two bugs of the season (dInsectInfo::getRandomByRarity) in mItem4/mItem5
        int msgFreeFashion(msgInfo_s *info);        // 8003C458 EC FreeB_Fassion
        int msgFreeFish(msgInfo_s *info);           // 8003C4D8 EC FreeB_Fish: two fish of the season (dFishInfo::getRandomByRarity) in mItem4/mItem5
        int msgFreeFossil(msgInfo_s *info);         // 8003C674 EC FreeB_Fossil: a fossil (dItem::seeker_c::getRandomFossil) in mItem4
        int msgFreeGardening(msgInfo_s *info);      // 8003C770 EC FreeB_Gardening
        int msgFreeInterior(msgInfo_s *info);       // 8003C820 EC FreeB_Interior
        int msgFreeParty(msgInfo_s *info);          // 8003C8A0 EC FreeB_Party
        int msgFreeHint(msgInfo_s *info);           // 8003C920 EC FreeB_Hint (fallback of msgFreeHobby)
        int msgFreeHobby(msgInfo_s *info);          // 8003C9A0 EC: hobby topic by the villager's wish kind (mQuest.mWish.mKind, 70%), else FreeB_Hint

        // ApC item offers (8046C3F8; the F8 hooks are d_npc_talk_approach's)
        int msgApcPresent(msgInfo_s *info);         // 8003CAB8 EC ApC_Present, F8 = endApPresent
        int msgApcSell(msgInfo_s *info);            // 8003CB58 EC ApC_Sell, F8 = endApSell
        int msgApcTrade(msgInfo_s *info);           // 8003CBF8 EC ApC_Trade, F8 = endApTrade
        int msgApcWant(msgInfo_s *info);            // 8003CC98 EC ApC_Want, F8 = endApWant
        const u8 *getApcWeights();                  // 8003CD38 ApC weights (4 x u8, .sdata2 80750024..80750038) by money, empty pocket, sellable item
        int msgApc(msgInfo_s *info);                // 8003CE58 EC: ApC topic by the weights of getApcWeights

        int msgFreeMoving(msgInfo_s *info);         // 8003CF40 EC FreeD_Moving2: villager moving out (memory flag bit 23 set); 140 = reqApMoveOut

        // FreeE town news
        const u8 *getTownWeights();                 // 8003D04C FreeE weights (2 x u8, .sdata2 8075003C..80750044)
        static int findEventDay(int event, u32 from, u32 to, const dTime_c &time); // 8003D0DC last day d in [to, from] with dEvent::isEventWithin(event, time, d, d), -1 if none (event and days are converted to const-ref temporaries per call, so not dQuestEvent_e / int)
        int msgFreeEvent(msgInfo_s *info);          // 8003D180 EC FreeE_Event: code by the current / next town event
        int msgFreeSnpc(msgInfo_s *info);           // 8003D5C0 EC FreeE_Snpc: code by the special npc in town (8046C428 / 8046C440)
        int msgFreeTown(msgInfo_s *info);           // 8003D710 EC: FreeE topic (8046C468, or 8046C480 when isEventDay(TRUE)) by getTownWeights

        // FreeF place
        static BOOL isBuildingNear(const dItem::Item *building, int xMin, int xMax, int zMin, int zMax); // 8003D7D0 the building's block position (dSaveBuildingList_c::getPos) lies in the range
        int msgFreePlace(msgInfo_s *info);          // 8003D894 EC FreeF_Building / House1 / House2 / Inside: what the villager stands next to

        // FreeG online play
        static const u8 *getOnlineWeights();        // 8003DC3C FreeG weights (3 x u8, .sdata2 80750048 own town / 8075004C visitor)
        int msgFreeHost(msgInfo_s *info);           // 8003DCDC EC FreeG_Host
        void setHostWords();                        // 8003DD5C words: host player's name (dPlayerMgr_c::getNetPlayer(0)) and the current player's land (also nml msgRecall)
        int msgFreeVisitor(msgInfo_s *info);        // 8003DE3C EC FreeG_Visitor (code 2 when memory mFlags.mSameTown)
        int msgFreeJinxV(msgInfo_s *info);          // 8003DEF8 EC FreeG_JinxV: jinx of this town (mJinxTown = -1); 104 = stepJinx
        int msgFreeOnline(msgInfo_s *info);         // 8003DFE8 EC: FreeG topic by the weights of getOnlineWeights

        // FreeH coming events (conditions 8046C4F8, topics 8046C540)
        BOOL isInTownEvent(dQuestEvent_e event);    // 8003E0B4 villager in the event state, the town's current event is event and it is active
        BOOL isHarvestTopic();                      // 8003E13C EVENT_HARVEST_FESTIVAL and not over
        BOOL isHalloweenTopic();                    // 8003E184 EVENT_HALLOWEEN not started yet
        BOOL isCountdownTopic();                    // 8003E1CC EVENT_COUNTDOWN not started yet
        BOOL isFmarketTopic();                      // 8003E214 in the own town outdoors and EVENT_FLEA_MARKET
        BOOL isXmasTopic();                         // 8003E26C EVENT_TOY_DAY and not over
        BOOL isEasterTopic();                       // 8003E2B4 villager valid and EVENT_BUNNY_DAY ongoing
        int pickComingEvent();                      // 8003E308 random index of a true condition of 8046C4F8, 6 if none
        int msgFreeHarvest(msgInfo_s *info);        // 8003E3F0 EC FreeH_Harvest (code 2 while ongoing)
        int msgFreeCountdown(msgInfo_s *info);      // 8003E49C EC FreeH_Countdown: next year as word 0 (mMsgYear)
        int msgFreeFmarket(msgInfo_s *info);        // 8003E544 EC FreeH_Fmarket
        int msgFreeXmas(msgInfo_s *info);           // 8003E5C4 EC FreeH_Xmas
        int msgFreeEaster(msgInfo_s *info);         // 8003E688 EC FreeH_Easter
        int msgFreeHalloween(msgInfo_s *info);      // 8003E708 EC FreeH_Halloween
        int msgFreeComingEvent(msgInfo_s *info);    // 8003E7B4 EC: FreeH topic 8046C540 of pickComingEvent

        // FreeI K.K. Slider's letter
        int msgFreeTunekichi(msgInfo_s *info);      // 8003E838 EC FreeI_Tunekichi (code 1), memory flag bit 21
        int reqTunekichiLetter();                   // 8003E8E4 140: dAnimal_c::sendTunekichiLetter, else memory mFlags.mTunekichiInvite
        int msgFreeTunekichi2(msgInfo_s *info);     // 8003E994 EC FreeI_Tunekichi (code 2), 140 = reqTunekichiLetter
        int msgFreeKK(msgInfo_s *info);             // 8003EA5C EC: FreeI topic (8046C588) by the player's flags / player count ("PF<2" weights), else msgFreeGeneral

        int msgFree(msgInfo_s *info);               // 8003EC4C EC root (nml 804A0E48+0x21C record): group by the weights of getFreeWeights, else msgFreeGeneral
        void endFree(int arg);                      // 8003EDEC F8 (nml 804A0E48+0x21C record): recordTalk(0), dNpc::msgMemory_c::set

        // Jinx (a town's "charm" in the English game; two messages: its condition, then its outcome)
        BOOL stepJinx(int kind);                    // 8003EE44 104: once idle, procedures {msgJinx, null, stepJinx2} (setProcs) (also nml 804A1180)
        int msgJinx(msgInfo_s *info);               // 8003EF14 EC: FreeA_Jinx / FreeG_JinxV, code 11 + the jinx condition
        BOOL stepJinx2(int kind);                   // 8003EFF8 104: once idle, procedures {msgJinx2, null, null}
        int msgJinx2(msgInfo_s *info);              // 8003F0A0 EC: FreeA_Jinx / FreeG_JinxV, code 31 + the jinx outcome

        BOOL stepFreeClothes(int kind);             // 8003F184 104 (Clothes): once idle, memory flag bit 10, remembers mItem0 at memory+0x86

        // Etc lines (records of nml's table 804A0784)
        int msgEtcHit(msgInfo_s *info);             // 8003F1D8 EC "Etc_Hit", record 804A0784+0x438
        void endEtc(int arg);                       // 8003F224 F8 of the Etc records: like d_npc_talk_approach endApproach
        int msgEtcPush(msgInfo_s *info);            // 8003F2B4 EC "Etc_Push", record 804A0784+0x45C
        int msgEtcFlea(msgInfo_s *info);            // 8003F304 EC "Etc_Flea", record 804A0784+0x480
        // ---- d_npc_talk_carnival (.text 8003F354..80043C34) ----
        int msgCarnival(msgInfo_s *info);           // 8003F354 EC: Festivale start, {msgCarnival, endCarnival, null} at nml 804A0E48+0x90; carnivalState_e ("Ev_Carnival"), 104 = a game start by state
        void endCarnival(int arg);                  // 8003F6A0 F8: msgMemory set, talk count 0x44, dAnimalMemory_c event flag 0 (met at the Festivale), fn_800F04E8
        BOOL stepCarnivalGift(int kind);            // 8003F758 104: give a random candy (requestItemAct), EC = msgCarnivalGift
        int msgCarnivalGift(msgInfo_s *info);       // 8003F83C EC: "Ev_Carnival" 0xD
        BOOL startCarnivalMsg(msgFunc next);        // 8003F86C once the controller is idle: EC = next, start the message
        static int countStakeItems(u16 *slotMask, dPrivateData_c *player); // 8003F8E0 count the sellable pocket items (price > 0, purchasable), mask of their slots
        BOOL setStakeMsg(msgInfo_s *info, const char *label, stepFunc next); // 8003F9D4 pick the stake (a candy from the pockets, 500 bells, or an item via countStakeItems), message code 3..6, 104 = next
        BOOL setStakeChoice(hookFunc yes);          // 8003FDD8 once idle: two answers, 0 = yes (handler), 1 = cancel
        // Game 1 ("Ev_Carnival1"): win two rounds in a row (70%, then 85%)
        BOOL stepCarnival1Start(int kind);          // 8003FE74 104: startCarnivalMsg(msgCarnival1Stake)
        int msgCarnival1Stake(msgInfo_s *info);     // 8003FEB4 EC: setStakeMsg(info, "Ev_Carnival1", stepCarnival1Stake)
        BOOL stepCarnival1Stake(int kind);          // 8003FEFC 104: setStakeChoice(selCarnival1Accept)
        void selCarnival1Accept();                  // 8003FF3C choice: 104 = stepCarnival1Accept
        BOOL stepCarnival1Accept(int kind);         // 8003FF7C 104: EC = msgCarnival1Play
        int msgCarnival1Play(msgInfo_s *info);      // 8003FFF4 EC: 0xB, 104 = stepCarnival1Choice
        BOOL stepCarnival1Choice(int kind);         // 80040060 104: two answers, both selCarnival1Answer
        void selCarnival1Answer();                  // 80040120 choice: 104 = stepCarnival1Answer
        BOOL stepCarnival1Answer(int kind);         // 80040160 104: EC = msgCarnival1Result
        int msgCarnival1Result(msgInfo_s *info);    // 800401D8 EC: 70% / 85% chance (+0x32F), 0xE..0x11; second win: candy prize; F8 / 104 by result
        void endCarnivalWin(int arg);               // 800403E4 F8: mpNpc->mAudioObj.startSound(0x17A1) (the player won)
        void endCarnivalLose(int arg);              // 80040408 F8: mpNpc->mAudioObj.startSound(0x17A2) (the player lost)
        BOOL stepCarnival1Prize(int kind);          // 8004042C 104: hand over the prize (+0x280) into the pockets (requestItemAct), EC = msgCarnival1Prize
        int msgCarnival1Prize(msgInfo_s *info);     // 800404E0 EC: 0x16
        BOOL stepCarnival1Lost(int kind);           // 80040510 104: EC = msgCarnival1Pay
        int msgCarnival1Pay(msgInfo_s *info);       // 80040588 EC: take the stake (500 bells or the pocket item), 0x13..0x15, 104 = stepCarnival1Pay
        BOOL stepCarnival1Pay(int kind);            // 80040708 104: requestItemActEx(6, stake), EC = msgCarnival1Paid
        int msgCarnival1Paid(msgInfo_s *info);      // 800407AC EC: 0x17
        // Game 2 ("Ev_Carnival2", rock-paper-scissors): two wins = prize, two losses = payment
        BOOL stepCarnival2Start(int kind);          // 800407DC 104: startCarnivalMsg(msgCarnival2Stake)
        int msgCarnival2Stake(msgInfo_s *info);     // 8004081C EC: setStakeMsg(info, "Ev_Carnival2", stepCarnival2Stake)
        BOOL stepCarnival2Stake(int kind);          // 80040864 104: setStakeChoice(selCarnival2Accept)
        void selCarnival2Accept();                  // 800408A4 choice: clear the loss / win counts (+0x32F / +0x330), 104 = stepCarnival2Choice
        BOOL stepCarnival2Next(int kind);           // 800408F0 104: EC = msgCarnival2Next
        int msgCarnival2Next(msgInfo_s *info);      // 80040968 EC: 0xB (next round), 104 = stepCarnival2Choice
        BOOL stepCarnival2Choice(int kind);         // 800409CC 104: three answers (hands), all selCarnival2Hand
        void selCarnival2Hand();                    // 80040AC8 choice: EC = msgCarnival2Hand, start the message
        int msgCarnival2Hand(msgInfo_s *info);      // 80040B1C EC: the npc's random hand vs. +0x32B, 0xC..0xE, counts losses (+0x32F) / wins (+0x330)
        BOOL stepCarnival2Lose(int kind);           // 80040CC0 104: EC = msgCarnival2Lose
        int msgCarnival2Lose(msgInfo_s *info);      // 80040D38 EC: 0x14..0x16 by +0x32B, F8 = endCarnivalLose, 104 = stepCarnival2Score
        BOOL stepCarnival2Win(int kind);            // 80040DF0 104: EC = msgCarnival2Win
        int msgCarnival2Win(msgInfo_s *info);       // 80040E68 EC: 0x11..0x13 by +0x32B, F8 = endCarnivalWin, 104 = stepCarnival2Score
        BOOL stepCarnival2Draw(int kind);           // 80040F20 104: EC = msgCarnival2Draw
        int msgCarnival2Draw(msgInfo_s *info);      // 80040F98 EC: 0xF (draw), 104 = stepCarnival2Choice
        BOOL stepCarnival2Score(int kind);          // 80040FFC 104: EC = msgCarnival2Score
        int msgCarnival2Score(msgInfo_s *info);     // 80041074 EC: 0x17..0x1B by the loss / win counts; 104 = next round / prize (2 wins) / payment (2 losses)
        BOOL stepCarnival2Won(int kind);            // 800411B8 104: EC = msgCarnival2Won
        int msgCarnival2Won(msgInfo_s *info);       // 80041230 EC: 0x1C, random candy prize, 104 = stepCarnival2Prize
        BOOL stepCarnival2Prize(int kind);          // 800412CC 104: hand over the prize (requestItemAct), EC = msgCarnival2Prize
        int msgCarnival2Prize(msgInfo_s *info);     // 80041368 EC: 0x20
        BOOL stepCarnival2Lost(int kind);           // 80041398 104: EC = msgCarnival2Pay
        int msgCarnival2Pay(msgInfo_s *info);       // 80041410 EC: take the stake, 104 = stepCarnival2Pay
        BOOL stepCarnival2Pay(int kind);            // 80041590 104: requestItemActEx(6, stake), EC = msgCarnival2Paid
        int msgCarnival2Paid(msgInfo_s *info);      // 80041614 EC: 0x21
        // Game 3 ("Ev_Carnival3"): two rounds, the player loses each with 20%; a draw is replayed
        BOOL stepCarnival3Start(int kind);          // 80041644 104: startCarnivalMsg(msgCarnival3Stake)
        int msgCarnival3Stake(msgInfo_s *info);     // 80041684 EC: setStakeMsg(info, "Ev_Carnival3", stepCarnival3Stake)
        BOOL stepCarnival3Stake(int kind);          // 800416CC 104: setStakeChoice(selCarnival3Accept)
        void selCarnival3Accept();                  // 8004170C choice: clear the counters, 104 = stepCarnival3Choice
        BOOL stepCarnival3Choice(int kind);         // 80041758 104: two answers, both selCarnival3Answer
        void selCarnival3Answer();                  // 80041818 choice: 104 = stepCarnival3Answer
        BOOL stepCarnival3Answer(int kind);         // 80041858 104: EC = msgCarnival3Round1
        int msgCarnival3Round1(msgInfo_s *info);    // 800418D0 EC: round 1, 20% loss, 0xF..0x12, 104 = stepCarnival3Choice2
        BOOL stepCarnival3Choice2(int kind);        // 80041A24 104: two answers, both selCarnival3Answer2
        void selCarnival3Answer2();                 // 80041AE4 choice: 104 = stepCarnival3Answer2
        BOOL stepCarnival3Answer2(int kind);        // 80041B24 104: EC = msgCarnival3Round2
        int msgCarnival3Round2(msgInfo_s *info);    // 80041B9C EC: round 2, 20% loss, 0x17..0x1A, 104 = stepCarnival3Score
        BOOL stepCarnival3Score(int kind);          // 80041CF0 104: EC = msgCarnival3Score
        int msgCarnival3Score(msgInfo_s *info);     // 80041D68 EC: result 0x1B / 0x1D / 0x1E by the loss / win counts; 104 = retry / prize / payment
        BOOL stepCarnival3Draw(int kind);           // 80041E48 104: EC = msgCarnival3Draw
        int msgCarnival3Draw(msgInfo_s *info);      // 80041EC0 EC: 0xC (draw), 104 = stepCarnival3Choice
        BOOL stepCarnival3Won(int kind);            // 80041F30 104: random candy prize, 104 = stepCarnival3Prize
        BOOL stepCarnival3Prize(int kind);          // 80041FBC 104: hand over the prize (requestItemAct), EC = msgCarnival3Prize
        int msgCarnival3Prize(msgInfo_s *info);     // 80042064 EC: 0x23
        BOOL stepCarnival3Lost(int kind);           // 80042094 104: EC = msgCarnival3Pay
        int msgCarnival3Pay(msgInfo_s *info);       // 8004210C EC: take the stake, 104 = stepCarnival3Pay
        BOOL stepCarnival3Pay(int kind);            // 8004228C 104: requestItemActEx(6, stake), EC = msgCarnival3Paid
        int msgCarnival3Paid(msgInfo_s *info);      // 80042330 EC: 0x24
        // Game 4 ("Ev_Carnival4"): win three rounds (80%, 85%, 90%), then pick the candy colour
        BOOL stepCarnival4Start(int kind);          // 80042360 104: startCarnivalMsg(msgCarnival4Stake)
        int msgCarnival4Stake(msgInfo_s *info);     // 800423A0 EC: setStakeMsg(info, "Ev_Carnival4", stepCarnival4Stake)
        BOOL stepCarnival4Stake(int kind);          // 800423E8 104: setStakeChoice(selCarnival4Accept)
        void selCarnival4Accept();                  // 80042428 choice: clear the round (+0x32F), 104 = stepCarnival4Choice
        BOOL stepCarnival4Choice(int kind);         // 80042470 104: two answers, both selCarnival4Answer
        void selCarnival4Answer();                  // 80042530 choice: 104 = stepCarnival4Answer
        BOOL stepCarnival4Answer(int kind);         // 80042570 104: EC = msgCarnival4Result
        int msgCarnival4Result(msgInfo_s *info);    // 800425E8 EC: round +0x32F (chance from 8046C6B0), 0xE..0x13; 104 = next round / prize choice / payment
        BOOL stepCarnival4Next(int kind);           // 800427C8 104: EC = msgCarnival4Next
        int msgCarnival4Next(msgInfo_s *info);      // 80042840 EC: 0xB (next round), 104 = stepCarnival4Choice
        BOOL stepCarnival4Color(int kind);          // 800428A4 104: four answers (candy colours), all selCarnival4Color
        void selCarnival4Color();                   // 800429CC choice: prize = candy +0x32B (local static dItem::Item[4] red/blue/yellow/green), 104 = stepCarnival4Prize
        BOOL stepCarnival4Prize(int kind);          // 80042B08 104: hand over the prize (requestItemAct), EC = msgCarnival4Prize
        int msgCarnival4Prize(msgInfo_s *info);     // 80042BBC EC: 0x19
        BOOL stepCarnival4Lost(int kind);           // 80042BEC 104: EC = msgCarnival4Pay
        int msgCarnival4Pay(msgInfo_s *info);       // 80042C64 EC: take the stake, 104 = stepCarnival4Pay
        BOOL stepCarnival4Pay(int kind);            // 80042DE4 104: requestItemActEx(6, stake), EC = msgCarnival4Paid
        int msgCarnival4Paid(msgInfo_s *info);      // 80042E68 EC: 0x1A
        // Game 5 ("Ev_Carnival5"): guess one of three (85%), then one of five (70%), then pick the candy colour
        BOOL stepCarnival5Start(int kind);          // 80042E98 104: startCarnivalMsg(msgCarnival5Stake)
        int msgCarnival5Stake(msgInfo_s *info);     // 80042ED8 EC: setStakeMsg(info, "Ev_Carnival5", stepCarnival5Stake)
        BOOL stepCarnival5Stake(int kind);          // 80042F20 104: setStakeChoice(selCarnival5Accept)
        void selCarnival5Accept();                  // 80042F60 choice: clear +0x32F, 104 = stepCarnival5Choice
        BOOL stepCarnival5Choice(int kind);         // 80042FA8 104: three answers, all selCarnival5Answer
        void selCarnival5Answer();                  // 800430A4 choice: 104 = stepCarnival5Answer
        BOOL stepCarnival5Answer(int kind);         // 800430E4 104: EC = msgCarnival5Result
        int msgCarnival5Result(msgInfo_s *info);    // 8004315C EC: guess one of three (85%), 0xF..0x12; 104 = stepCarnival5Choice2 / payment
        BOOL stepCarnival5Lost(int kind);           // 800432C0 104: EC = msgCarnival5Pay
        int msgCarnival5Pay(msgInfo_s *info);       // 80043338 EC: take the stake, 104 = stepCarnival5Pay
        BOOL stepCarnival5Pay(int kind);            // 800434B8 104: requestItemActEx(6, stake), EC = msgCarnival5Paid
        int msgCarnival5Paid(msgInfo_s *info);      // 8004355C EC: 0x25
        BOOL stepCarnival5Choice2(int kind);        // 8004358C 104: five answers, all selCarnival5Answer2
        void selCarnival5Answer2();                 // 800436E0 choice: 104 = stepCarnival5Answer2
        BOOL stepCarnival5Answer2(int kind);        // 80043720 104: EC = msgCarnival5Result2
        int msgCarnival5Result2(msgInfo_s *info);   // 80043798 EC: guess one of five (70%), 0x19..0x1E; 104 = stepCarnival5Color / payment
        BOOL stepCarnival5Color(int kind);          // 800438EC 104: four answers (candy colours), all selCarnival5Color
        void selCarnival5Color();                   // 80043A14 choice: prize = candy +0x32B (local static dItem::Item[4]), 104 = stepCarnival5Prize
        BOOL stepCarnival5Prize(int kind);          // 80043B50 104: hand over the prize (requestItemAct), EC = msgCarnival5Prize
        int msgCarnival5Prize(msgInfo_s *info);     // 80043C04 EC: 0x24
        // ---- d_npc_talk_countdown (.text 80043C34..80043EBC) ----
        int msgCountdown(msgInfo_s *info);          // 80043C34 EC (l_talkEntrySets [TALK_COUNTDOWN]): countdownState_e by the time, code = state + 1 (getMsgLabel kind 3); 0 when not in the event
        void endCountdown(int arg);                 // 80043E2C F8: talk count 0x44, msgMemory set
        // ---- d_npc_talk_halloween (.text 80043EBC..80045154) ----
        int msgCandyAsk(msgInfo_s *info);           // 80043EBC EC (l_talkProcSets [14]): "Ev_Halloween" 0x15 (asks for candy), F8 = endCandyAsk, 104 = stepCandyChoice
        void endCandyAsk(int arg);                  // 80043F48 F8: msgMemory set
        BOOL stepCandyChoice(int kind);             // 80043FDC 104: two choices (selCandyGive / selCandyNone)
        void selCandyGive();                        // 8004409C choice: select a pocket item (reqSelectItem), 128 = resCandyGive
        void resCandyGive();                        // 80044120 128: take the selected pocket item; 134 = resCandyGiven (candy) / resCandyWrong; EC = msgCandyNone if cancelled
        void resCandyGiven();                       // 800442A8 134: EC = msgCandyThanks, close the message
        int msgCandyThanks(msgInfo_s *info);        // 80044304 EC: 0x17, 104 = stepCandyThanks
        BOOL stepCandyThanks(int kind);             // 80044368 104: requestHandAct10(2), EC = msgCandyCostume
        int msgCandyCostume(msgInfo_s *info);       // 800443EC EC: 0x18/0x19 by the player's costume (0x1A without memory)
        void resCandyWrong();                       // 80044480 134 (not candy): EC = msgCandyWrong, close the message
        int msgCandyWrong(msgInfo_s *info);         // 800444DC EC: 0x1B, 104 = stepCandyWrong
        BOOL stepCandyWrong(int kind);              // 80044540 104: requestHandActD, 128 = resCandyTrick, EC = msgCandyTrick
        static BOOL filterTrickItem(const dItem::Item *item, int arg); // 800445DC pocket filter for countPocketsFlag: a valid, purchasable item
        static int countTrickPockets(u16 *slotMask, dPrivateData_c *player); // 80044640 count pockets matching filterTrickItem
        BOOL trickPockets();                        // 800446C4 trick: replace a random matching pocket item with a jack-in-the-box (this unused)
        static BOOL isMatchingOutfit(const dItem::Item *hat, const dItem::Item *shirt, const dItem::Item *acc); // 8004478C hat or accessory has the shirt's fashion theme (fn_800F8948)
        static BOOL isInCostume(const dEquip_c *equip); // 800447EC isMatchingOutfit(&mHat, &mShirt, &mAcc): wearing a costume
        void trickClothes();                        // 80044800 trick: pumpkin head, or a moldy / patched shirt (trickPockets when in costume or neither fits), requestEquip
        void resCandyTrick();                       // 80044A00 128: trickClothes()
        int msgCandyTrick(msgInfo_s *info);         // 80044A04 EC: 0x1C
        int msgCandyNone(msgInfo_s *info);          // 80044A34 EC: 0x1D (no candy), 104 = stepCandyNone
        BOOL stepCandyNone(int kind);               // 80044A98 104: EC = msgCandyNoneTrick, trickClothes
        int msgCandyNoneTrick(msgInfo_s *info);     // 80044B18 EC: 0x1E
        void selCandyNone();                        // 80044B48 choice: EC = msgCandyNone
        int msgHalloweenCostume(msgInfo_s *info);   // 80044B9C EC (l_talkProcSets [15]): 0x1F/0x20 by the costume, F8 = endHalloweenCostume
        void endHalloweenCostume(int arg);          // 80044C50 F8: msgMemory set
        int msgHalloween(msgInfo_s *info);          // 80044CC8 EC (l_talkEntrySets [TALK_HALLOWEEN]): EVENT_HALLOWEEN ongoing, npc in the event, in its house; 104 = stepHalloweenFirst or stepHalloweenPresent
        void endHalloween(int arg);                 // 80044F2C F8 of msgHalloween: msgMemory set, talk count 0x44
        BOOL stepHalloweenFirst(int kind);          // 80044FBC 104: dAnimalMemory_c event flag 0 (met on Halloween), fn_800F04E8
        BOOL stepHalloweenPresent(int kind);        // 80045038 104: present (fn_800C60B4) into an empty pocket on code 3, session flag 0
        // ---- d_npc_talk_harvest (.text 80045154..8004664C) ----
        // Returns the knife-and-fork choice (selForkGive) when the festival is on for this npc (event flag 1,
        // fork returned, not set yet) and the player has a knife and fork, else null; *code = 0x13, *flag = 1
        // then. The quest talk units' choice steps offer it too. Returned through the hidden
        // struct pointer (r3, `this` in r4).
        const hookFunc getHarvestChoice(u16 *code, u8 *flag, BOOL checkEvent); // 80045154
        int msgHarvest(msgInfo_s *info);            // 800452B0 EC (l_talkEntrySets [TALK_HARVEST]): festival request (getMsgLabel(TALK_HARVEST, 0) label, code 0)
        void endHarvest(int arg);                   // 80045390 F8: talk count 0x44, msgMemory set
        BOOL stepHarvest(int kind);                 // 80045420 104: two choices (getHarvestChoice's / selHarvestNo)
        void selForkGive();                         // 80045544 choice: select the knife and fork (reqSelectItem), 128 = resForkGive
        void resForkGive();                         // 800455D0 128: take the selected pocket item, 134 = resForkGiven; EC = msgForkCancel if cancelled
        int msgForkCancel(msgInfo_s *info);         // 800456E0 EC: item select cancelled (getMsgLabel(TALK_HARVEST, 1) label "Q_Cancel"), code 0
        void resForkGiven();                        // 8004573C 134: EC = msgForkThanks, close the message
        int msgForkThanks(msgInfo_s *info);         // 80045798 EC: "Ev_Harvest" code 1; 104 = stepForkThanks
        BOOL stepForkThanks(int kind);              // 8004581C 104: requestHandActD, 104 = stepForkThanksWait
        BOOL stepForkThanksWait(int kind);          // 80045874 104: EC = msgSpotIntro
        int msgSpotIntro(msgInfo_s *info);          // 800458EC EC: "Ev_Harvest" code 7, 104 = stepSpotIntro
        BOOL stepSpotIntro(int kind);               // 80045970 104: EC = msgHarvestSpot, dAnimalEventState_c flag 1 (fork returned), fn_800F0108

        // Location checks of the table 8046C708, called by msgHarvestSpot with the festival spot (save town
        // +0x66745) as field block / unit (dFdBase_c::posToBlockUnit; argument names inferred). TRUE and
        // *code set on a match.
        BOOL checkSpotNpcBlock(u16 *code, int blockX, int blockZ, int unitX, int unitZ);    // 80045A20 0xB: the villager's acre (its house's when in its house)
        BOOL checkSpotPlayerHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045B38 0xC: the current player's house acre
        BOOL checkSpotOtherHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ);  // 80045C00 0xD: another player's house acre (name word 0, unit word 1)
        BOOL checkSpotNpcHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ);    // 80045D8C 0xE: another villager's house acre (name word 2, unit word 3)
        BOOL checkSpotLandmark(u16 *code, int blockX, int blockZ, int unitX, int unitZ);    // 80045F14 0xF..0x13: shop, tailor, town hall, museum, gate acre (8046C6F0)
        BOOL checkSpotWater(u16 *code, int blockX, int blockZ, int unitX, int unitZ);       // 80045FC8 0x14/0x15/0x20: waterfall, Pascal's (racco) acre, pond
        BOOL checkSpotEmptyHouse(u16 *code, int blockX, int blockZ, int unitX, int unitZ);  // 80046098 0x16: an unowned player house acre
        BOOL checkSpotBuildSite(u16 *code, int blockX, int blockZ, int unitX, int unitZ);   // 8004617C 0x17: empty build site in the acre
        BOOL checkSpotBeach(u16 *code, int blockX, int blockZ, int unitX, int unitZ);       // 8004627C 0x18: a beach acre
        BOOL checkSpotGateLine(u16 *code, int blockX, int blockZ, int unitX, int unitZ);    // 800462F8 0x19..0x1B: west / east of the gate acre in its row, below it (larger z) in its column
        BOOL checkSpotEdge(u16 *code, int blockX, int blockZ, int unitX, int unitZ);        // 800463F0 0x1C/0x1D: west / east edge acres of the map
        BOOL checkSpotDefault(u16 *code, int blockX, int blockZ, int unitX, int unitZ);     // 800464A8 0x1E/0x1F: EVENT_HARVEST_FESTIVAL ongoing or not (always matches)

        int msgHarvestSpot(msgInfo_s *info);        // 800464F8 EC: where the festival spot is (code 0x1F when no check matches)
        void selHarvestNo();                        // 8004660C choice "no": back to l_talkEntrySets [TALK_FREE], startMsg
        // ---- d_npc_talk_bug (.text 8004664C..80046BD0) ----
        // State bugOffState_e (d_npc_talk_bug.hpp), BUGOFF_STATE_NONE = no Bug-Off talk: npc's event flag 0
        // not set yet, this npc leads (getEntry object flag 0x4000 clear / set), another villager leads, the
        // current player leads, another player leads.
        int getBugOffTalkState();                   // 8004664C Bug-Off state (EVENT_BUG_OFF ongoing, outdoors in the town, npc in the event)
        const procSet_s *getBugOffProcSet();        // 800468B4 l_talkEntrySets [TALK_BUG] while getBugOffTalkState() < BUGOFF_STATE_NONE, else NULL (getEventProcSet table 8046BDBC)
        int msgBugOff(msgInfo_s *info);             // 800468EC EC: code by state (getMsgLabel kind 0xC), names the bug / score / leader, may set getEntry object flag 0x4000
        void endBugOff(int arg);                    // 80046B18 F8: dAnimalMemory_c event flag 0, fn_800F04E8, talk count 0x44, msgMemory set
        // ---- d_npc_talk_fireworks (.text 80046BD0..800470E4) ----
        // State fireworksState_e (d_npc_talk_fireworks.hpp), FIREWORKS_STATE_NONE = no fireworks talk: not
        // started yet (within 30 min of the start), first hour, middle, last hour before the end
        // (dEvent::getStartTime / getEndTime, dTime_c::isSameOrAfter).
        int getFireworksTalkState();                // 80046BD0 fireworks state (EVENT_FIREWORKS active and not over, outdoors in the town, npc in the event)
        const procSet_s *getFireworksProcSet();     // 80046F60 l_talkEntrySets [TALK_FIREWORKS] while getFireworksTalkState() < FIREWORKS_STATE_NONE, else NULL (table 8046BDBC)
        int msgFireworks(msgInfo_s *info);          // 80046F98 EC: code = state + 1 (getMsgLabel kind 0xD)
        void endFireworks(int arg);                 // 80047054 F8: talk count 0x44, msgMemory set
        // ---- d_npc_talk_fishing (.text 800470E4..800476E0) ----
        // State fishingState_e (d_npc_talk_fishing.hpp), FISHING_STATE_NONE = no tourney talk: npc's event
        // flag 0 not set yet (no fish / a fish recorded), this npc leads (getEntry object flag 0x4000 clear /
        // set), the current player leads, another player leads, another villager leads.
        int getFishingTalkState();                  // 800470E4 tourney state (EVENT_FISHING_TOURNEY ongoing, outdoors in the town, npc in the event)
        const procSet_s *getFishingProcSet();       // 8004736C l_talkEntrySets [TALK_FISHING] while getFishingTalkState() < FISHING_STATE_NONE, else NULL (table 8046BDBC)
        int msgFishing(msgInfo_s *info);            // 800473A4 EC: code = state + 1 (getMsgLabel kind 0xB), names the fish / leader / size (setDecimal), may set getEntry object flag 0x4000
        void endFishing(int arg);                   // 80047628 F8: dAnimalMemory_c event flag 0, fn_800F04E8, talk count 0x44, msgMemory set
        // ---- d_npc_talk_fmarket (.text 800476E0..8004902C) ----
        // EC (l_talkProcSets [21]), the npc's own stall: "Ev_FmarketNPC*" (label table 804A2D00) by the npc's
        // boxed furniture count, the memory of the player (talk_c+0x17C, its land) and the entry flags
        // mEventTalked (greeted) / mFmarketSale (asked); land name 1; code random or fixed; 104 = stepFmarketAsk for
        // case 6; setTopic(KIND_ANY, 0, 0).
        int msgFmarket(msgInfo_s *info);            // 800476E0
        void endFmarket(int arg);                   // 800479C4 F8 ([21], [22]): msgMemory set, entry flag mEventTalked, talk count 0x44
        BOOL stepFmarketAsk(int kind);              // 80047A6C 104: one choice (selFmarketAsk) when the controller is idle
        void selFmarketAsk();                       // 80047B00 choice: entry flag mFmarketSale, fn_8019C34C
        // EC (l_talkProcSets [22]), selling: the boxed furniture at mFtrPosX / mFtrPosZ (getRoomFtrIdx),
        // price getSellPrice (d_npc_talk_quest_q09) into mPrice; "Ev_FmarketNPC3" when the player has a free
        // pocket and the money (104 = stepSaleChoice), else "Ev_FmarketNPC4"; item name 0, number 3.
        int msgSale(msgInfo_s *info);               // 80047B38
        BOOL stepSaleChoice(int kind);              // 80047CD4 104: two choices (selSaleYes code 0x42, selSaleNo code 0x45)
        void selSaleYes();                          // 80047DAC choice: EC = msgSaleYes
        int msgSaleYes(msgInfo_s *info);            // 80047E00 EC: "Q09_TradeYes"; F8 = endSaleYes, 104 = stepSaleYes; returns 0
        void endSaleYes(int arg);                   // 80047E8C F8: the sale: fn_800A8F98 -> mFtrHandle, removeRoomFtr, payMoney / pickUp, clears mFmarketSale; 110 = actSaleWait
        BOOL stepSaleYes(int kind);                 // 80047FA0 104: fn_8019C35C when the controller is idle
        void actSaleWait();                         // 80047FE4 110: waits for fn_800A9354(mFtrHandle), unlocks, clears 110
        void selSaleNo();                           // 8004805C choice: EC = msgSaleNo
        int msgSaleNo(msgInfo_s *info);             // 800480B0 EC: "Q09_TradeNo"; returns 0
        // The npc at the player's stall (l_talkProcSets [23]..[27]).
        int msgStallCall(msgInfo_s *info);          // 800480E0 EC ([23]): "Q08_Call"; F8 = endStallTalk, 104 = stepStallCall
        void endStallTalk(int arg);                 // 8004816C F8 ([23]..[25]): msgMemory set, talk count 0x44
        BOOL stepStallCall(int kind);               // 800481FC 104: EC = msgStallComing, setRequest1(0), 128 = resStallCall
        void resStallCall();                        // 8004829C 128: 110 = actStallWalk
        void actStallWalk();                        // 800482DC 110: npc requestWalk to l_walkTargetPos; 110 = actStallArrive
        void actStallArrive();                      // 8004838C 110: at the goal: requestWait, camera fn_8018B5B8, reqMsgClose, clear 110, memory event flag 0
        int msgStallComing(msgInfo_s *info);        // 80048438 EC: "Ev_FmarketPC" code 1
        int msgStallWait(msgInfo_s *info);          // 80048468 EC ([24]): "Q08_Wait"; F8 = endStallTalk, 104 = stepStallWait
        BOOL stepStallWait(int kind);               // 800484F4 104: EC = msgStallLook when the controller is idle
        int msgStallLook(msgInfo_s *info);          // 8004856C EC: "Ev_FmarketPC" code 2; F8 = endStallLook
        void endStallLook(int arg);                 // 800485D0 F8: memory (mpMemory) event flag 1
        int msgStallBack(msgInfo_s *info);          // 800485E8 EC ([25]): "Q08_Back"; F8 = endStallTalk, 104 = stepStallBack
        BOOL stepStallBack(int kind);               // 80048674 104: EC = msgStallLook when the controller is idle
        int msgStallNoItem(msgInfo_s *info);        // 800486EC EC ([26]): sets [26], "Ev_FmarketPC" code 3
        void endStallBuy(int arg);                  // 8004874C F8 ([26], [27]): msgMemory set, talk count 0x44
        int msgStallItem(msgInfo_s *info);          // 800487DC EC ([27]): item of the player's stall (mItem0, furniture handle mFtrHandle); "Ev_FmarketPC" code 4, else msgStallNoItem
        BOOL stepStallItem(int kind);               // 800488B0 104 ([27]): two choices (selStallPrice code 0x6D, selStallNo code 0x6E)
        void selStallPrice();                       // 80048988 choice: reqMenu21(1) (price input), 128 = resStallPrice
        // 128: the entered price (recept_c::fn_80029B20) to mPrice, number 0; EC = msgStallPriceOK when it
        // is <= getBuyPrice (d_npc_talk_quest_q09) of the item, msgStallTooHigh when higher,
        // msgStallRefused when cancelled; reqMsgClose.
        void resStallPrice();                       // 800489D4
        int msgStallPriceOK(msgInfo_s *info);       // 80048B0C EC: "Ev_FmarketPC" code 6 (price accepted); 104 = stepStallBuy
        // 104: the purchase: the player has room for the money (getMoneyRoom), fn_800A8F98, removeRoomFtr,
        // addMoney, dAnimal_c::addNewItem + fn_800F0E9C, entry byte +0x289 += 1; 110 = actStallBuyWait;
        // otherwise EC = msgStallNoRoom.
        BOOL stepStallBuy(int kind);                // 80048B70
        void actStallBuyWait();                     // 80048D3C 110: waits for fn_800A9354(mFtrHandle), unlocks, clears 110, EC = msgStallBought
        int msgStallBought(msgInfo_s *info);        // 80048DE4 EC: "Ev_FmarketPC" code 9
        int msgStallNoRoom(msgInfo_s *info);        // 80048E14 EC: "Ev_FmarketPC" code 8 (no room for the money)
        void selStallNo();                          // 80048E44 choice: EC = msgStallRefused
        int msgStallRefused(msgInfo_s *info);       // 80048E98 EC: "Ev_FmarketPC" code 5 (refused), entry byte +0x28A += 1
        int msgStallTooHigh(msgInfo_s *info);       // 80048EF0 EC: "Ev_FmarketPC" code 7 (price too high); 104 = stepStallTooHigh
        BOOL stepStallTooHigh(int kind);            // 80048F54 104: two choices (selStallPrice code 0x6F, selStallNo code 0x70)
        // ---- d_npc_talk_birthday (.text 8004902C..80049FDC) ----
        // Talk procedures. Sets in nml l_talkProcSets: [28] {msgPlayerBirthday, endPlayerBirthday,
        // stepPlayerBirthday}, [29] {msgNpcBirthday, endNpcBirthday, null}.
        int msgPlayerBirthday(msgInfo_s *info);     // 8004902C EC: player's birthday; code 0xE / 0xB for a Feb 29 birthday (on / not on Feb 29), 4 / 1 by friendship (>= 0x40)
        void endPlayerBirthday(int arg);            // 80049128 F8: msgMemory set, talk count 0x44
        BOOL stepPlayerBirthday(int kind);          // 800491B8 104: present ITEM_IDX_BIRTHDAY_CAKE to talk_c+0x280, pickUp, requestItemAct; EC = msgPlayerBirthdayCake
        int msgPlayerBirthdayCake(msgInfo_s *info); // 8004926C EC: code = mMessageCode + 1; player mBirthdayHost._08 = -1
        int msgNpcBirthday(msgInfo_s *info);        // 800492B8 EC: npc's birthday; code 7, 8 when already congratulated (getEntry() _28C.mBirthdayDone)
        void endNpcBirthday(int arg);               // 80049344 F8: msgMemory set, sets _28C.mBirthdayDone (congratulated), talk count 0x44
        // ---- d_npc_talk_quest_q04 (.text 80049FDC..8004B75C) ----
        int msgClothOffer(msgInfo_s *info);         // 80049FDC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 5, kind 3, stepClothAccept)
        int msgClothReq(msgInfo_s *info);           // 8004A024 EC: "Q04_Req" code (mMatchMode == 0 ? 4 : 1) + rndF(3); 104 = nml stepRequestChoice
        int msgClothQuest(msgInfo_s *info);         // 8004A0D8 talk check while the quest runs (nml .rodata 8046BF30): state 7 won by
        //          another player, 6 past deadline, 8 has a shirt (countPocketsKind(4)), 14 not
        //          in the quest (startQuestOffer offer: accept stepClothAccept); label
        //          getMsgLabel(6, id 5, state), 104 = descriptor proc by state, names,
        //          setQ4Word (look name of the request)
        BOOL stepClothGiveChoice(int kind);         // 8004A5F8 104 (state 8): choices getRollanChoice / getHarvestChoice procs, give
        //          (selClothGive, 0x40) and no (nml selResumeTalk, 4)
        void selClothGive();                        // 8004A7CC choice: item select (reqSelectItem, countPocketsKind(4) mask), 128 = resClothSelect
        void resClothSelect();                      // 8004A858 128: selected pocket item to talk_c+0x280 / +0x282 (name word 7),
        //          requestItemActEx; isClothRequestMatch: pocket cleared, 134 = resClothMatch,
        //          else 134 = resClothMismatch (EC = nml msgCancel if cancelled)
        void resClothMatch();                       // 8004A9E8 134: EC = msgClothWin, close the message
        int msgClothWin(msgInfo_s *info);           // 8004AA44 EC: "Q04_Win" code 0; F8 = endClothReward, 104 = stepClothWin
        void endClothReward(int arg);               // 8004AACC F8: reward (pickClothReward) to talk_c+0x280: pick up item or add bells
        BOOL stepClothWin(int kind);                // 8004AC68 104: quest state 1, requester = player, wish +5; EC = msgClothWin2; the npc
        //          wears the shirt (setCloth(talk_c+0x282), fn_800F1A68, npc onDaubClothChange /
        //          offDaubClothChanged), requestHandActC, 128 = resClothWear
        void resClothWear();                        // 8004ADBC 128: npc (talk_c+0x68) offDaubClothChange / onDaubClothChanged
        int msgClothWin2(msgInfo_s *info);          // 8004ADF4 EC: "Q04_Win" code 4; 104 = stepClothWin2
        BOOL stepClothWin2(int kind);               // 8004AE54 104: EC = msgClothWin3, requestItemAct of the reward, sound 0x171F
        int msgClothWin3(msgInfo_s *info);          // 8004AF04 EC: "Q04_Win" code 8
        void resClothMismatch();                    // 8004AF30 134 (no match): EC = msgClothNG, close the message
        int msgClothNG(msgInfo_s *info);            // 8004AF8C EC: "Q04_NG" code 1..4 by mMatchMode and dItem::Item::isSame with
        //          dAnimal_c+0x2FFA; word 6 = setLookName (dItem::nameLook_c); 104 = stepClothNG
        BOOL stepClothNG(int kind);                 // 8004B0D8 104: requestHandActE (hand the shirt back)
        BOOL stepClothOver(int kind);               // 8004B11C 104 (states 6/7): remove the player from the quest, clear it when empty
        BOOL stepClothTalkChoice(int kind);         // 8004B1B0 104 (other states): choices getRollanChoice / getHarvestChoice procs, selClothCon (1)
        //          and no (nml selResumeTalk, 4)
        void selClothCon();                         // 8004B384 choice: EC = msgClothCon
        int msgClothCon(msgInfo_s *info);           // 8004B3D8 EC: "Q04_ConA" (mMatchMode != 0) / "Q04_ConB" code 0; 104 = stepClothCon
        BOOL stepClothCon(int kind);                // 8004B47C 104: EC = msgClothCon2
        int msgClothCon2(msgInfo_s *info);          // 8004B4F4 EC: "Q04_ConA" / "Q04_ConB" code 7 + min(players - 1, 2)
        BOOL stepClothAccept(int kind);             // 8004B5A8 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names, setQ4Word; EC = msgClothReq
        // ---- d_npc_talk_quest_q02 (.text 8004B75C..8004CBCC) ----
        int msgFishOffer(msgInfo_s *info);          // 8004B75C offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 3, kind 1, stepFishAccept)
        int msgFishReq(msgInfo_s *info);            // 8004B7A4 EC: "Q02_Req" code 0; 104 = nml stepRequestChoice
        int msgFishQuest(msgInfo_s *info);          // 8004B804 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching item (countPocketsItem),
        //          6 not in the quest (startQuestOffer offer: accept stepFishAccept); label
        //          getMsgLabel(6, id 3, state) (l_q02Labels; the PTMF constants after it are this
        //          function's), 104 by state: stepFishLose (3/4), stepFishGiveChoice (5), else
        //          stepFishChoice; names
        BOOL stepFishGiveChoice(int kind);          // 8004BD14 104 (state 5): choices getRollanChoice / getHarvestChoice procs, give
        //          (selFishGive, 0x3E) and no (nml selResumeTalk, 4)
        void selFishGive();                         // 8004BEE8 choice: item select (reqSelectItem, countPocketsItem mask), 128 = resFishGive
        void resFishGive();                         // 8004BF98 128: take the selected pocket item to talk_c+0x286, requestItemActEx,
        //          134 = resFishGiven (EC = nml msgCancel if cancelled)
        void resFishGiven();                        // 8004C0C8 134: EC = msgFishWin, close the message
        int msgFishWin(msgInfo_s *info);            // 8004C124 EC: "Q02_Win" code 0; F8 = endFishWin, 104 = stepFishWin
        void endFishWin(int arg);                   // 8004C1AC F8: reward (pickInsectFishReward) to talk_c+0x280: pick up item or add bells
        BOOL stepFishWin(int kind);                 // 8004C350 104: quest state 1, requester = player, wish +5, the npc keeps the
        //          fish (addNewItem); requestHandActD, EC = msgFishReward
        int msgFishReward(msgInfo_s *info);         // 8004C4A8 EC: "Q02_Win" code 4; 104 = stepFishReward
        BOOL stepFishReward(int kind);              // 8004C508 104: EC = msgFishRewardEnd, requestItemAct of the reward, sound 0x171F
        int msgFishRewardEnd(msgInfo_s *info);      // 8004C5B8 EC: "Q02_Win" code 8
        BOOL stepFishLose(int kind);                // 8004C5E4 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL stepFishChoice(int kind);              // 8004C678 104 (other states): choices getRollanChoice / getHarvestChoice procs, selFishCon (1)
        //          and no (nml selResumeTalk, 4)
        void selFishCon();                          // 8004C84C choice: EC = msgFishCon
        int msgFishCon(msgInfo_s *info);            // 8004C8A0 EC: "Q02_Con" code 0; 104 = stepFishCon
        BOOL stepFishCon(int kind);                 // 8004C900 104: EC = msgFishConPlayers
        int msgFishConPlayers(msgInfo_s *info);     // 8004C978 EC: "Q02_Con" code 7 + min(players - 1, 2)
        BOOL stepFishAccept(int kind);              // 8004CA18 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = msgFishReq
        // ---- d_npc_talk_quest_q03 (.text 8004CBCC..8004E31C) ----
        int msgFossilOffer(msgInfo_s *info);        // 8004CBCC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 4, kind 2, stepFossilAccept)
        int msgFossilReq(msgInfo_s *info);          // 8004CC14 EC: "Q03_Req" code (mMatchMode == 0 ? 4 : 1) + rndF(3); 104 = nml stepRequestChoice
        int msgFossilQuest(msgInfo_s *info);        // 8004CCC8 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching fossil (countPocketsFossil),
        //          6 not in the quest (startQuestOffer offer: accept stepFossilAccept), 8 / 9 the
        //          winner (quest state 1 / other); label getMsgLabel(6, id 4, state) (l_q03Labels; the
        //          PTMF constants after it are this function's), 104 by state: stepFossilLose (3/4),
        //          stepFossilGiveChoice (5), stepFossilReturn (8), stepFossilComp (9), else
        //          stepFossilChoice; names, word 5 = item or "any fossil" (STR_Unit 0x63)
        BOOL stepFossilGiveChoice(int kind);        // 8004D2D0 104 (state 5): choices getRollanChoice / getHarvestChoice procs, give
        //          (selFossilGive, 0x3F) and no (nml selResumeTalk, 4)
        void selFossilGive();                       // 8004D4A4 choice: item select (reqSelectItem, countPocketsFossil mask), 128 = resFossilGive
        void resFossilGive();                       // 8004D548 128: take the selected pocket item to talk_c+0x286 (name word 6),
        //          requestItemActEx, 134 = resFossilGiven (EC = nml msgCancel if cancelled)
        void resFossilGiven();                      // 8004D688 134: EC = msgFossilWin, close the message
        int msgFossilWin(msgInfo_s *info);          // 8004D6E4 EC: "Q03_Win" code 0; F8 = endFossilWin, 104 = stepFossilWin
        void endFossilWin(int arg);                 // 8004D76C F8: reward (pickFossilReward) to talk_c+0x280: pick up item or add bells
        BOOL stepFossilWin(int kind);               // 8004D910 104: quest state 1, fn_800F12D8(idx, 2, 1), requester = player, wish +5,
        //          dAnimal_c::setQuestStarted; the npc keeps the fossil (addNewItem,
        //          dAnimalMemory_c::setPresent on talk_c+0x17C / +0x180); EC = msgFossilReward,
        //          requestHandActD
        int msgFossilReward(msgInfo_s *info);       // 8004DAB8 EC: "Q03_Win" code 4; 104 = stepFossilReward
        BOOL stepFossilReward(int kind);            // 8004DB18 104: EC = msgFossilRewardEnd, requestItemAct of the reward, sound 0x171F
        int msgFossilRewardEnd(msgInfo_s *info);    // 8004DBC8 EC: "Q03_Win" code 8
        BOOL stepFossilLose(int kind);              // 8004DBF4 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL stepFossilChoice(int kind);            // 8004DC88 104 (other states): choices getRollanChoice / getHarvestChoice procs, selFossilCon (1)
        //          and no (nml selResumeTalk, 4)
        void selFossilCon();                        // 8004DE5C choice: EC = msgFossilCon
        int msgFossilCon(msgInfo_s *info);          // 8004DEB0 EC: "Q03_Con" code (mMatchMode == 0 and an item set ? 4 : 1) + rndF(3);
        //          104 = stepFossilCon
        BOOL stepFossilCon(int kind);               // 8004DF70 104: EC = msgFossilConPlayers
        int msgFossilConPlayers(msgInfo_s *info);   // 8004DFE8 EC: "Q03_Con" code 7 + min(players - 1, 2)
        BOOL stepFossilReturn(int kind);            // 8004E088 104 (state 8, "Q03_Return"): quest state 2, fn_800F12D8(idx, 2, 2)
        BOOL stepFossilComp(int kind);              // 8004E0FC 104 (state 9, "Q03_Comp"): quest state 4
        BOOL stepFossilAccept(int kind);            // 8004E150 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = msgFossilReq
        // ---- d_npc_talk_quest_q05 (.text 8004E31C..8004FADC) ----
        int msgFtrOffer(msgInfo_s *info);           // 8004E31C offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 6, kind 4, stepFtrAccept)
        int msgFtrReq(msgInfo_s *info);             // 8004E364 EC: "Q05_Req" code 0; 104 = nml stepRequestChoice
        int msgFtrQuest(msgInfo_s *info);           // 8004E3C4 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has furniture (countPocketsKind(3)), 9 not
        //          in the quest (startQuestOffer offer: accept stepFtrAccept), 11 / 12 the winner
        //          (quest state 1 / other); label getMsgLabel(6, id 6, state), 104 =
        //          descriptor proc by state, names, setQ5Word (part name of the request)
        BOOL stepFtrGiveChoice(int kind);           // 8004E990 104 (state 5): choices getRollanChoice / getHarvestChoice procs, give
        //          (selFtrGive, 0x41) and no (nml selResumeTalk, 4)
        void selFtrGive();                          // 8004EB64 choice: item select (reqSelectItem, countPocketsKind(3) mask), 128 = resFtrSelect
        void resFtrSelect();                        // 8004EBF0 128: selected pocket item to talk_c+0x280 (name word 6), requestItemActEx;
        //          checkFtrRequest == 0: pocket cleared, item to talk_c+0x286, 134 =
        //          resFtrMatch, else 134 = resFtrMismatch (EC = nml msgCancel if cancelled)
        void resFtrMatch();                         // 8004ED80 134: EC = msgFtrWin, close the message
        int msgFtrWin(msgInfo_s *info);             // 8004EDDC EC: "Q05_Win" code 0; F8 = endFtrReward, 104 = stepFtrWin
        void endFtrReward(int arg);                 // 8004EE64 F8: reward (pickFtrReward) to talk_c+0x280: pick up item or add bells
        BOOL stepFtrWin(int kind);                  // 8004F000 104: quest state 1, fn_800F12D8(idx, 4, 1), requester = player, wish +5,
        //          dAnimal_c::setQuestStarted; the npc keeps the furniture (addNewItem,
        //          dAnimalMemory_c::setPresent on talk_c+0x17C / +0x180); EC = msgFtrWin2,
        //          requestHandActD
        int msgFtrWin2(msgInfo_s *info);            // 8004F1A8 EC: "Q05_Win" code 4; 104 = stepFtrWin2
        BOOL stepFtrWin2(int kind);                 // 8004F208 104: EC = msgFtrWin3, requestItemAct of the reward, sound 0x171F
        int msgFtrWin3(msgInfo_s *info);            // 8004F2B8 EC: "Q05_Win" code 8
        void resFtrMismatch();                      // 8004F2E4 134 (no match): EC = msgFtrNG, close the message
        int msgFtrNG(msgInfo_s *info);              // 8004F340 EC: "Q05_NG" code 1..3 by checkFtrRequest; 104 = stepFtrNG
        BOOL stepFtrNG(int kind);                   // 8004F400 104: requestHandActE (hand the item back)
        BOOL stepFtrOver(int kind);                 // 8004F444 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL stepFtrTalkChoice(int kind);           // 8004F4D8 104 (other states): choices getRollanChoice / getHarvestChoice procs, selFtrCon (1)
        //          and no (nml selResumeTalk, 4)
        void selFtrCon();                           // 8004F6AC choice: EC = msgFtrCon
        int msgFtrCon(msgInfo_s *info);             // 8004F700 EC: "Q05_Con" code 0; 104 = stepFtrCon
        BOOL stepFtrCon(int kind);                  // 8004F760 104: EC = msgFtrCon2
        int msgFtrCon2(msgInfo_s *info);            // 8004F7D8 EC: "Q05_Con" code 7 + min(players - 1, 2)
        BOOL stepFtrReturn(int kind);               // 8004F868 104 (state 11, "Q05_Return"): quest state 2, fn_800F12D8(idx, 4, 2)
        BOOL stepFtrComp(int kind);                 // 8004F8DC 104 (state 12, "Q05_Comp"): quest state 4
        BOOL stepFtrAccept(int kind);               // 8004F930 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names, setQ5Word; EC = msgFtrReq
        // ---- d_npc_talk_quest_q01 (.text 8004FADC..80050F4C) ----
        int msgInsectOffer(msgInfo_s *info);        // 8004FADC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 2, kind 0, stepInsectAccept)
        int msgInsectReq(msgInfo_s *info);          // 8004FB24 EC: "Q01_Req" code 0; 104 = nml stepRequestChoice
        int msgInsectQuest(msgInfo_s *info);        // 8004FB84 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching item (countPocketsItem),
        //          6 not in the quest (startQuestOffer offer: accept stepInsectAccept); label
        //          getMsgLabel(6, id 2, state) (l_q01Labels; the PTMF constants after it are this
        //          function's), 104 by state: stepInsectLose (3/4), stepInsectGiveChoice (5), else
        //          stepInsectChoice; names
        BOOL stepInsectGiveChoice(int kind);        // 80050094 104 (state 5): choices getRollanChoice / getHarvestChoice procs, give
        //          (selInsectGive, 0x3D) and no (nml selResumeTalk, 4)
        void selInsectGive();                       // 80050268 choice: item select (reqSelectItem, countPocketsItem mask), 128 = resInsectGive
        void resInsectGive();                       // 80050318 128: take the selected pocket item to talk_c+0x286, requestItemActEx,
        //          134 = resInsectGiven (EC = nml msgCancel if cancelled)
        void resInsectGiven();                      // 80050448 134: EC = msgInsectWin, close the message
        int msgInsectWin(msgInfo_s *info);          // 800504A4 EC: "Q01_Win" code 0; F8 = endInsectWin, 104 = stepInsectWin
        void endInsectWin(int arg);                 // 8005052C F8: reward (pickInsectFishReward) to talk_c+0x280: pick up item or add bells
        BOOL stepInsectWin(int kind);               // 800506D0 104: quest state 1, requester = player, wish +5, the npc keeps the
        //          insect (addNewItem); EC = msgInsectReward, requestHandActD
        int msgInsectReward(msgInfo_s *info);       // 80050828 EC: "Q01_Win" code 4; 104 = stepInsectReward
        BOOL stepInsectReward(int kind);            // 80050888 104: EC = msgInsectRewardEnd, requestItemAct of the reward, sound 0x171F
        int msgInsectRewardEnd(msgInfo_s *info);    // 80050938 EC: "Q01_Win" code 8
        BOOL stepInsectLose(int kind);              // 80050964 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL stepInsectChoice(int kind);            // 800509F8 104 (other states): choices getRollanChoice / getHarvestChoice procs, selInsectCon (1)
        //          and no (nml selResumeTalk, 4)
        void selInsectCon();                        // 80050BCC choice: EC = msgInsectCon
        int msgInsectCon(msgInfo_s *info);          // 80050C20 EC: "Q01_Con" code 0; 104 = stepInsectCon
        BOOL stepInsectCon(int kind);               // 80050C80 104: EC = msgInsectConPlayers
        int msgInsectConPlayers(msgInfo_s *info);   // 80050CF8 EC: "Q01_Con" code 7 + min(players - 1, 2)
        BOOL stepInsectAccept(int kind);            // 80050D98 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = msgInsectReq
        // ---- d_npc_talk_quest_q12 (.text 80050F4C..80051C68) ----
        int msgLostKeyQuest(msgInfo_s *info);       // 80050F4C quest check (nml table 8046BF30 entry 7): kind 6 active, label getMsgLabel(6, 7, idx);
        //          104 by state: stepLostKeyReport (1/2), stepLostKeyChoice (3/4), stepLostKeyVisit (5)
        BOOL stepLostKeyChoice(int kind);           // 8005125C 104: choice 0 = selLostKeyGive (2 entries, cancel 1)
        void selLostKeyGive();                      // 800512F0 choice: reqSelectItem(key pockets, 0x22, 0x78, 1), 128 = resLostKeyGive
        void resLostKeyGive();                      // 80051380 128: selected pocket == quest item -> remove it, 134 = resLostKeyOK (else resLostKeyNG), requestItemActEx(7); cancel: EC = msgLostKeyCancel
        void resLostKeyOK();                        // 800514E0 134: EC = msgLostKeyOK, close the message
        int msgLostKeyOK(msgInfo_s *info);          // 8005153C EC "Q12_KeyOK", 104 = stepLostKeyOK
        BOOL stepLostKeyOK(int kind);               // 800515A0 104: EC = msgLostKeyItem, requestHandActD, friendship +5 (nml addFriendship)
        int msgLostKeyItem(msgInfo_s *info);        // 8005162C EC "Q12_Item", F8 = endLostKeyItem, 104 = stepLostKeyItem
        void endLostKeyItem(int arg);               // 800516B8 F8: reward (dAnimal_c::pickLostItemReward) to talk_c+0x280 and pickUp, or bells (word 2)
        BOOL stepLostKeyItem(int kind);             // 80051810 104: quest state 2, requester, removePlayer, held-item change minute, clear when empty; EC = msgLostKeyItemEnd, requestItemAct(+0x280)
        int msgLostKeyItemEnd(msgInfo_s *info);     // 800519D4 EC "Q12_Item" code 4
        void resLostKeyNG();                        // 80051A04 134: EC = msgLostKeyNG, close the message
        int msgLostKeyNG(msgInfo_s *info);          // 80051A60 EC "Q12_KeyNG", 104 = stepLostKeyNG
        BOOL stepLostKeyNG(int kind);               // 80051AC4 104: requestHandActE
        int msgLostKeyCancel(msgInfo_s *info);      // 80051B08 EC "Q12_Cancel"
        BOOL stepLostKeyReport(int kind);           // 80051B38 104 (states 1/2): removePlayer; clear the quest + dLostQuest_c::setTime when empty
        BOOL stepLostKeyVisit(int kind);            // 80051BF0 104 (state 5): dQuestVillager_c::addPlayer(player, 1)
        // ---- d_npc_talk_quest_q11 (.text 80051C68..80052EB0) ----
        int msgSickQuest(msgInfo_s *info);          // 80051C68 quest check (nml table 8046BF30 entry 8): kind 5 active, label getMsgLabel(6, 8, idx); F8/104 from 804A3A68
        BOOL stepSickChoice(int kind);              // 80052190 104: two choices (msg 0x48 -> selSickMedicine, 0x49 -> selSickTalk)
        void selSickMedicine();                     // 80052268 choice: EC = msgSickMedicine
        int msgSickMedicine(msgInfo_s *info);       // 800522BC EC "Q11_Medicine", 104 = stepSickMedicine
        BOOL stepSickMedicine(int kind);            // 8005237C 104: reqSelectItem(medicine pockets, 0x22, 1), 128 = resSickMedicine
        void resSickMedicine();                     // 80052440 128: take the selected item, 134 = resSickCure, requestItemActEx(9), friendship +7; cancel: EC = msgSickCancel
        void resSickCure();                         // 80052578 134: EC = msgSickCure, close the message
        int msgSickCure(msgInfo_s *info);           // 800525D4 EC "Q11_Cure", 104 = stepSickCure
        BOOL stepSickCure(int kind);                // 80052694 104: dQuestSick_c::setPlayer, dQuestVillager_c::addPlayer(player, 1)
        int msgSickCancel(msgInfo_s *info);         // 8005274C EC "Q11_Cancel", 104 = stepSickVisit, friendship -1
        BOOL stepSickVisit(int kind);               // 80052818 104: dQuestVillager_c::addPlayer(player, 1) (also the default 104 of msgSickQuest)
        void selSickTalk();                         // 8005289C choice: EC = msgSickTalk
        int msgSickTalk(msgInfo_s *info);           // 800528F0 EC "Q11_Talk", 104 = stepSickTalk
        BOOL stepSickTalk(int kind);                // 800529B0 104: dQuestVillager_c::addPlayer(player, 1)
        BOOL stepSickGreet(int kind);               // 80052A34 104 (msgSickQuest): EC = msgSickTalk
        void endSickReward(int arg);                // 80052AAC F8 (msgSickQuest): reward (dAnimal_c::pickSickReward) to talk_c+0x280, pickUp, item word 4
        BOOL stepSickReward(int kind);              // 80052B48 104 (msgSickQuest): removePlayer, clear when empty; EC = msgSickEnd, requestItemAct(+0x280)
        int msgSickEnd(msgInfo_s *info);            // 80052C5C EC "Q11_End"
        void endSickRewardFull(int arg);            // 80052C88 F8 (msgSickQuest): dAnimal_c::sendSickReward (pockets full); quest state 2 on failure
        BOOL stepSickRewardFull(int kind);          // 80052D04 104 (msgSickQuest): removePlayer, clear when state 1 and empty; EC = msgSickEnd
        BOOL stepSickReport(int kind);              // 80052E04 104 (msgSickQuest): removePlayer, clear when state 1 and empty
        // ---- d_npc_talk_quest_delivery (.text 80052EB0..800546F4) ----
        int msgDelivery(msgInfo_s *info);           // 80052EB0 EC (l_talkEntrySets [TALK_QUEST_DELIVERY]): errand of state 0 whose animal 1 is this npc, item in pockets; label getMsgLabel(5, kind == 8, 0), 104 = stepPackageChoice / stepClothesChoice by kind
        void endQuestCommon(int arg);               // 80053224 F8 (shared end-of-talk hook of the quest talks and offers): nml recordTalk / getRememberedMsg, dNpc::msgMemory_c::set, dAnimalTalkCount_c::inc(0x44)
        void startDeliverySelect(hookFunc next);    // 800532B4 reqSelectItem(errand item pockets, 0x22, 1), 128 = next (by value: callers pass a stack copy)
        int msgDeliveryCancel(msgInfo_s *info);     // 80053390 EC "Q_Cancel"
        void selDeliveryResume();                   // 800533C0 choice: EC/F8/104 = getEventProcSet() (default 804A1064), run EC
        BOOL stepPackageChoice(int kind);           // 80053414 104 (kind 7): common choices (getRollanChoice / getHarvestChoice), 0xD -> selPackageGive, 4 -> selDeliveryResume
        void selPackageGive();                      // 800535E8 choice: startDeliverySelect(resPackageSelect)
        void resPackageSelect();                    // 80053628 128: take the selected pocket item, requestItemActEx(7), 134 = resPackageWrapped (pocket flag 2) / resPackageOpened; cancel: EC = msgDeliveryCancel
        void resPackageWrapped();                   // 80053774 134: EC = msgPackageGet, close the message
        int msgPackageGet(msgInfo_s *info);         // 800537D0 EC "Q06_Get0", 104 = stepPackageGet, friendship +5
        BOOL stepPackageGet(int kind);              // 80053840 104: EC = msgPackageGet2, requestHandActF
        int msgPackageGet2(msgInfo_s *info);        // 800538C0 EC "Q06_Get1", 104 = stepPackageGet2
        BOOL stepPackageGet2(int kind);             // 80053924 104: errand state 1 (2 when past the deadline); EC = msgPackageFin, requestHandActD
        int msgPackageFin(msgInfo_s *info);         // 80053A04 EC "Q06_Fin"
        void resPackageOpened();                    // 80053A30 134: EC = msgPackageOpened, close the message
        int msgPackageOpened(msgInfo_s *info);      // 80053A8C EC "Q06_Open", 104 = stepPackageOpened, friendship +3
        BOOL stepPackageOpened(int kind);           // 80053AFC 104: same as stepPackageGet2
        BOOL stepClothesChoice(int kind);           // 80053BDC 104 (kind 8): common choices, 0xD -> selClothesGive, 4 -> selDeliveryResume
        void selClothesGive();                      // 80053DB0 choice: startDeliverySelect(resClothesSelect)
        void resClothesSelect();                    // 80053DF0 128: as resPackageSelect, item also kept at talk_c+0x282; 134 = resClothesWrapped (pocket flag 2) / resClothesOpened
        void resClothesWrapped();                   // 80053F44 134: EC = msgClothesGet, close the message
        int msgClothesGet(msgInfo_s *info);         // 80053FA0 EC "Q07_Get0", 104 = stepClothesGet
        int updateClothesMatch();                   // 80054004 dAnimal_c::getStyleMatch of the errand item (2 when none); sets errand state match + 1 (4 past the deadline)
        BOOL stepClothesGet(int kind);              // 800540C8 104: EC by updateClothesMatch(): 0 msgClothesGet1, 1 msgClothesGet3, else msgClothesGet2; requestHandActF
        int msgClothesGet1(msgInfo_s *info);        // 800541D0 EC "Q07_Get1", 104 = stepClothesWear, friendship +5
        BOOL stepClothesWear(int kind);             // 80054240 104: EC = msgClothesFin, npc puts on talk_c+0x282 (dAnimal_c::setCloth), requestHandActC, 128 = resClothesWear
        void resClothesWear();                      // 80054338 128: nml offDaubClothChange / onDaubClothChanged on the npc (cloth change end)
        int msgClothesFin(msgInfo_s *info);         // 80054370 EC "Q07_Fin"
        int msgClothesGet2(msgInfo_s *info);        // 8005439C EC "Q07_Get2", 104 = stepClothesWear, friendship +5
        int msgClothesGet3(msgInfo_s *info);        // 8005440C EC "Q07_Get3", 104 = stepClothesKeep, friendship +5
        BOOL stepClothesKeep(int kind);             // 8005447C 104: requestHandActD
        void resClothesOpened();                    // 800544C0 134: EC by updateClothesMatch(): 0 msgClothesOpen1, 1 msgClothesOpen3, else msgClothesOpen2; close the message
        int msgClothesOpen1(msgInfo_s *info);       // 800545A4 EC "Q07_Open1", 104 = stepClothesWear, friendship +3
        int msgClothesOpen2(msgInfo_s *info);       // 80054614 EC "Q07_Open2", 104 = stepClothesWear, friendship +3
        int msgClothesOpen3(msgInfo_s *info);       // 80054684 EC "Q07_Open3", 104 = stepClothesKeep, friendship +3
        // ---- d_npc_talk_quest_q07 (.text 800546F4..80056024) ----
        int msgErrandFinalOffer(msgInfo_s *info);   // 800546F4 offer check of QUEST_KIND_ERRAND_REQUEST_FINAL (nml l_questOffer [1]):
        //          startErrandOffer(info, kind 8, QUEST_TALK_ERRAND_FINAL, stepErrandFinalOffer)
        BOOL stepErrandFinalOffer(int kind);        // 8005473C 104 (offer accepted): once idle, EC = msgErrandFinalReq
        int msgErrandFinalReq(msgInfo_s *info);     // 800547B4 EC "Q07_Req", 104 = stepErrandFinalReq
        BOOL stepErrandFinalReq(int kind);          // 80054814 104: yes/no choice (0x15 -> selErrandFinalYes, 0x1F -> nml selNo); clears the entry's
        //          quest
        void selErrandFinalYes();                   // 80054914 choice (yes): EC = msgErrandFinalYes
        int msgErrandFinalYes(msgInfo_s *info);     // 80054968 EC "Q_Yes", F8 = endErrandFinalYes, 104 = stepErrandFinalYes
        void endErrandFinalYes(int arg);            // 800549F4 F8: starts the errand: deadline (pickDeadline), recipient (pickRandomAvailableAnimal),
        //          clothing (fn_800C60B4, not the recipient's own) to talk_c+0x280, dQuestErrand_c::start(8, ...), deadline words
        BOOL stepErrandFinalYes(int kind);          // 80054C48 104: once idle, EC = nml msgYes, hands talk_c+0x280 to the player (requestItemAct),
        //          sound 0x171E
        int msgErrandFinalQuest(msgInfo_s *info);   // 80054CEC quest check (nml msgQuestTalk [1]): active errand of kind 8 that this villager gave;
        //          label getMsgLabel(6, 1, state) (l_q07Labels), 104 by state: stepErrandFinalChoice (open), nml stepErrandOver
        //          (past deadline), stepErrandFinalReportChoice (delivered); names
        BOOL stepErrandFinalReportChoice(int kind); // 800550B0 104 (delivered): choices getRollanChoice / getHarvestChoice procs, report
        //          (selErrandFinalReport, 0x10) and selErrandFinalResume (4)
        void selErrandFinalReport();                // 80055284 choice (report): errand state 4 (late) -> EC = nml msgTimeover2, friendship -1; else
        //          EC = msgErrandFinalReport
        int msgErrandFinalReport(msgInfo_s *info);  // 80055348 EC "Q07_Report", 104 = stepErrandFinalReport
        BOOL stepErrandFinalReport(int kind);       // 800553AC 104: how the recipient liked it: 0x33 selErrandFinalGood, 0x34 selErrandFinalNormal,
        //          0x35 selErrandFinalBad (no cancel)
        void selErrandFinalGood();                  // 800554C8 choice: EC = msgErrandFinalGood
        void selErrandFinalNormal();                // 8005551C choice: EC = msgErrandFinalNormal
        void selErrandFinalBad();                   // 80055570 choice: EC = msgErrandFinalBad
        int msgErrandFinalGood(msgInfo_s *info);    // 800555C4 EC "Q07_Good", 104 = stepErrandFinalAnswer, addAnswerFriendship(1)
        int msgErrandFinalNormal(msgInfo_s *info);  // 80055634 EC "Q07_Normal", 104 = stepErrandFinalAnswer, addAnswerFriendship(3)
        int msgErrandFinalBad(msgInfo_s *info);     // 800556A4 EC "Q07_Bad", 104 = stepErrandFinalAnswer, addAnswerFriendship(2)
        BOOL stepErrandFinalAnswer(int kind);       // 80055710 104: once idle, EC = msgErrandFinalRewardFull (pockets full) / msgErrandFinalReward
        int msgErrandFinalReward(msgInfo_s *info);  // 800557E8 EC "Q_Item", F8 = endErrandFinalReward, 104 = stepErrandFinalReward
        void endErrandFinalReward(int arg);         // 80055874 F8: reward (dAnimal_c::pickErrandFinalReward by errand state and answer) to
        //          talk_c+0x280: pick up item or add bells
        BOOL stepErrandFinalReward(int kind);       // 80055A5C 104: clears the errand; EC = msgErrandFinalEnd, requestItemAct of the reward, sound
        //          0x171F
        int msgErrandFinalEnd(msgInfo_s *info);     // 80055B3C EC "Q07_End"
        int msgErrandFinalRewardFull(msgInfo_s *info); // 80055B68 EC "Q_ItemFull", 104 = stepErrandFinalRewardFull
        BOOL stepErrandFinalRewardFull(int kind);   // 80055BCC 104: errand _18E = answer kind + 1, dAnimal_c::completeErrandRequestFinal; EC =
        //          msgErrandFinalEnd, sound 0x171F
        void selErrandFinalResume();                // 80055CF8 choice: EC/F8/104 = getEventProcSet() (default 804A0E48 [TALK_FREE]), run EC
        BOOL stepErrandFinalChoice(int kind);       // 80055D4C 104 (errand open): choices getRollanChoice / getHarvestChoice procs, selErrandFinalCon
        //          (1) and selErrandFinalResume (4)
        void selErrandFinalCon();                   // 80055F20 choice: EC = msgErrandFinalCon
        int msgErrandFinalCon(msgInfo_s *info);     // 80055F74 EC "Q07_Con"
        void addAnswerFriendship(int answer);       // 80055FA0 friendship +5 if the answer matches the errand state (how the recipient liked it),
        //          else +3
        // ---- d_npc_talk_quest_q06 (.text 80056024..800573D0) ----
        int msgErrandOffer(msgInfo_s *info);        // 80056024 offer check of QUEST_KIND_ERRAND_REQUEST (nml l_questOffer [0]):
        //          startErrandOffer(info, kind 7, QUEST_TALK_ERRAND, stepErrandOffer)
        BOOL stepErrandOffer(int kind);             // 8005606C 104 (offer accepted): once idle, EC = msgErrandReq
        int msgErrandReq(msgInfo_s *info);          // 800560E4 EC "Q06_Req", 104 = stepErrandReq
        BOOL stepErrandReq(int kind);               // 80056144 104: yes/no choice (0x15 -> selErrandYes, 0x1F -> nml selNo); clears the entry's quest
        void selErrandYes();                        // 80056244 choice (yes): EC = msgErrandYes
        int msgErrandYes(msgInfo_s *info);          // 80056298 EC "Q_Yes", F8 = endErrandYes, 104 = stepErrandYes
        void endErrandYes(int arg);                 // 80056324 F8: starts the errand: deadline (pickDeadline), recipient (pickRandomAvailableAnimal),
        //          item (fn_800F4608) to talk_c+0x280, dQuestErrand_c::start(7, ...), deadline words
        BOOL stepErrandYes(int kind);               // 80056534 104: once idle, EC = nml msgYes, hands talk_c+0x280 to the player (requestItemAct),
        //          sound 0x171E
        int msgErrandQuest(msgInfo_s *info);        // 800565D8 quest check (nml msgQuestTalk [0]): active errand of kind 7 that this villager gave;
        //          label getMsgLabel(6, 0, state) (l_q06Labels), 104 by state: stepErrandChoice (open), nml stepErrandOver (past
        //          deadline), stepErrandReportChoice (delivered); names
        BOOL stepErrandReportChoice(int kind);      // 80056928 104 (delivered): choices getRollanChoice / getHarvestChoice procs, report
        //          (selErrandReport, 0x10) and no (nml selResumeTalk, 4)
        void selErrandReport();                     // 80056AFC choice (report): errand state 1 (in time) -> EC = msgErrandReport, friendship +5; else
        //          EC = nml msgTimeover2, friendship -1
        int msgErrandReport(msgInfo_s *info);       // 80056BCC EC "Q06_Report", 104 = stepErrandReport
        BOOL stepErrandReport(int kind);            // 80056C30 104: once idle, EC = msgErrandRewardFull (pockets full) / msgErrandReward
        int msgErrandReward(msgInfo_s *info);       // 80056D14 EC "Q_Item", F8 = endErrandReward, 104 = stepErrandReward
        void endErrandReward(int arg);              // 80056DA0 F8: reward (dAnimal_c::pickErrandReward) to talk_c+0x280: pick up item or add bells
        BOOL stepErrandReward(int kind);            // 80056EF4 104: clears the errand; EC = msgErrandEnd, requestItemAct of the reward, sound 0x171F
        int msgErrandEnd(msgInfo_s *info);          // 80056FD4 EC "Q06_End"
        int msgErrandRewardFull(msgInfo_s *info);   // 80057000 EC "Q_ItemFull", 104 = stepErrandRewardFull
        BOOL stepErrandRewardFull(int kind);        // 80057064 104: errand _18E = 1, dAnimal_c::completeErrandRequest; EC = msgErrandEnd, sound
        //          0x171F
        BOOL stepErrandChoice(int kind);            // 80057170 104 (errand open): choices getRollanChoice / getHarvestChoice procs, selErrandCon (1)
        //          and no (nml selResumeTalk, 4)
        void selErrandCon();                        // 80057344 choice: EC = msgErrandCon
        int msgErrandCon(msgInfo_s *info);          // 80057398 EC "Q06_Con"
        static dSceneChange_c *getSceneChange();    // 800573C4 &gSceneChange (like ::getSceneChange 80161CC4); called from d_npc_talk_quest_q10
        // ---- d_npc_talk_quest_q10 (.text 800573D0..80058990) ----
        int msgHideOffer(msgInfo_s *info);          // 800573D0 offer check (nml l_questOffer, kind 0x14): no rain/snow, 6:00..21:59, town checks; day
        //          to mStartDay; nml startQuestOffer(info, 10, F8 endQuestCommon, 104 stepHideOffer, 1)
        BOOL stepHideOffer(int kind);               // 800575AC 104: once idle, EC = msgHideReq
        int msgHideReq(msgInfo_s *info);            // 80057624 EC "Q10_Req", 104 = stepHideReq
        BOOL stepHideReq(int kind);                 // 80057684 104: choices 0 = selHideYes, 1 = selHideNo; clears the entry's quest
        void selHideYes();                          // 8005776C choice (yes): EC = msgHideOK
        int msgHideOK(msgInfo_s *info);             // 800577C0 EC "Q10_OK" (code 4 when mStartDay is not today), F8 = endHideOK, 104 = stepHideOK
        void endHideOK(int arg);                    // 800578A0 F8: starts the game (dAnimalBlock_c::startHideAndSeek(player, npc, now))
        BOOL stepHideOK(int kind);                  // 80057918 104: setRequest1(0), 128 = resHideSetup
        void resHideSetup();                        // 80057988 128: scene exit to set up the game (fn_801A6748(0)), day change blocked
        void selHideNo();                           // 80057A10 choice (no): EC = msgHideNo
        int msgHideNo(msgInfo_s *info);             // 80057A64 EC "Q10_No"
        int msgHideQuest(msgInfo_s *info);          // 80057A90 quest check (nml msgQuestTalk [12]): game of this player; msgHideHider unless time is
        //          up, else label getMsgLabel(6, 0xC, 0) ("Q10_Badend"), 104 = stepHideBadend, addHiderFriendship(-5)
        BOOL stepHideBadend(int kind);              // 80057C64 104: dQuestPlayerAnimal_c::clearInfo, fn_800F0650
        int msgHideExplain(msgInfo_s *info);        // 80057CB8 EC (nml l_talkProcSets [10]) "Q10_Explain" (minutes / hiders left as words 0 / 1), F8
        //          = endQuestCommon, 104 = stepHideExplain
        BOOL stepHideExplain(int kind);             // 80057DB0 104: setRequest1(0), 128 = resHideStart
        void resHideStart();                        // 80057E20 128: scene exit, game running: state 1, time limit, fn_801A6748(1), day change
        //          unblocked
        int msgHideHider(msgInfo_s *info);          // 80057EA8 EC (nml l_talkProcSets [11]): hider talk "Q10_Visit" / "Q10_Find" (104 stepHideFind) /
        //          "Q10_Over" / "Q10_Wait" (104 stepHideOver); F8 = endQuestCommon
        BOOL stepHideFind(int kind);                // 80058114 104: hider found (setFlag, fn_800F0708); all found -> l_talkProcSets [12] or
        //          setRequest1(0), 128 = resHideEnd; else EC = msgHideContinue; friendship +5
        void resHideEnd();                          // 80058270 128: scene exit at the game end (fn_801A6748(2)), day change blocked
        int msgHideContinue(msgInfo_s *info);       // 800582F8 EC "Q10_Continue" (hiders left as word 1)
        BOOL stepHideOver(int kind);                // 80058388 104: time over: state 2, l_talkProcSets [13] or setRequest1(0), 128 = resHideEnd;
        //          addHiderFriendship(-2)
        int msgHideReward(msgInfo_s *info);         // 8005844C EC (nml l_talkProcSets [12]) "Q10_Item" / "Q10_ItemFull", F8 = endHideReward, 104 =
        //          stepHideReward / stepHideRewardFull
        void endHideReward(int arg);                // 80058574 F8: reward (dAnimalBlock_c::pickHideAndSeekPresent) to talk_c+0x280, into the pockets
        //          or sent by letter (pockets full); state 3
        BOOL stepHideReward(int kind);              // 800586BC 104: requestItemAct(+0x280), EC = msgHideEnd
        int msgHideEnd(msgInfo_s *info);            // 80058754 EC "Q10_End"
        BOOL stepHideRewardFull(int kind);          // 80058780 104: EC = msgHideEnd
        int msgHideLose(msgInfo_s *info);           // 800587F8 EC (nml l_talkProcSets [13]) "Q10_Lose", F8 = endHideLose
        void endHideLose(int arg);                  // 8005885C F8: state 3 (or clearInfo), fgMngProc_unblockDayChange
        void addHiderFriendship(s8 delta);          // 800588BC friendship delta for the player with each unfound hider (`this` unused)
        // ---- d_npc_talk_quest_q13 (.text 80058990..8005A3C4) ----
        int msgStyleOffer(msgInfo_s *info);         // 80058990 offer check (nml table 8046BFD8, kind 0x13): canStartStyle; nml startQuestOffer(info, 7, F8 endQuestCommon, 104 stepStyleOffer, 1)
        BOOL stepStyleOffer(int kind);                   // 80058A98 104: EC = msgStyleReq
        int msgStyleReq(msgInfo_s *info);           // 80058B10 EC "Q13_Req", F8 = endStyleReq, 104 = stepStyleReq
        void endStyleReq(int arg);                    // 80058B98 F8: pickStyleOtherPlayer -> talk_c+0x1EC, its name as word 0
        BOOL stepStyleReq(int kind);                   // 80058C98 104: choices (0x15 -> selStyleYes, 0x1F -> nml selNo); dQuestBase_c::clear at getEntry()+0x23E
        void selStyleYes();                         // 80058D98 choice (accept): EC = msgStyleYes
        int msgStyleYes(msgInfo_s *info);           // 80058DEC EC "Q13_Yes", F8 = endStyleYes
        void endStyleYes(int arg);                    // 80058E4C F8: startStyle(npc, player, talk_c+0x1EC, now), topic text as word 4
        int msgStyleQuest(msgInfo_s *info);         // 80058F18 quest check (nml table 8046BF30 entry 9): label getMsgLabel(6, 9, idx), F8/104 from 804A431C; or msgStyleAsk / nml startQuestOffer(info, 9, ...)
        BOOL stepStyleAsk(int kind);                   // 80059504 104 (804A431C): EC = msgStyleAsk
        int msgStyleAsk(msgInfo_s *info);           // 8005957C EC "Q13_Ask" ("Q13_Ask2" in state 1), 104 = stepStyleAskChoice
        BOOL stepStyleAskChoice(int kind);                   // 80059618 104: common choices (getRollanChoice / getHarvestChoice), 0x36 -> selStyleAnswer, 0x39 -> selStyleForget
        void selStyleAnswer();                         // 800597EC choice (answer): reqMenu22(1), 128 = resStyleAnswer
        void resStyleAnswer();                         // 80059838 128: typed word == topic text -> EC = msgStyleAnswerOK, else msgStyleAnswerNG; close the message
        int msgStyleAnswerOK(msgInfo_s *info);           // 8005995C EC "Q13_AnswerOK", 104 = stepStyleAnswerOK
        BOOL stepStyleAnswerOK(int kind);                   // 800599C0 104: EC = msgStyleReward
        int msgStyleReward(msgInfo_s *info);           // 80059A38 EC "Q13_AnswerOK" code 9 (10: pockets full), F8/104 = endStyleReward/stepStyleReward (full: endStyleRewardFull/stepStyleRewardFull), friendship +3
        void endStyleReward(int arg);                    // 80059B88 F8: pickStylePresent to talk_c+0x280, pickUp, item word 5; state 2
        BOOL stepStyleReward(int kind);                   // 80059C20 104: EC = msgStyleThank, requestItemAct(+0x280)
        int msgStyleThank(msgInfo_s *info);           // 80059CB8 EC "Q13_Thank"
        void endStyleRewardFull(int arg);                    // 80059CE8 F8 (pockets full): sendStyleLetterTo(1, 0), flag bit 1 on failure; state 2
        BOOL stepStyleRewardFull(int kind);                   // 80059D44 104: EC = msgStyleThank
        int msgStyleAnswerNG(msgInfo_s *info);           // 80059DBC EC "Q13_AnswerNG", 104 = stepStyleAnswerNG
        BOOL stepStyleAnswerNG(int kind);                   // 80059E20 104: quest state 1
        void selStyleForget();                         // 80059E70 choice (forgot): EC = msgStyleForget
        int msgStyleForget(msgInfo_s *info);           // 80059EC4 EC "Q13_Forget"
        BOOL stepStyleTalk(int kind);                   // 80059EF4 104 (804A431C): common choices, 1 -> selStyleCon, 4 -> selStyleResume
        void selStyleCon();                         // 8005A0C8 choice: EC = msgStyleCon
        int msgStyleCon(msgInfo_s *info);           // 8005A11C EC "Q13_Con"
        void selStyleResume();                         // 8005A148 choice: EC/F8/104 = getEventProcSet() (default 804A1064), run EC
        BOOL stepStyleOver(int kind);                   // 8005A19C 104 (804A431C): dQuestPlayerPair_c::clearInfo
        void endStyleItem(int arg);                    // 8005A1EC F8 (804A431C): pickStylePresent to talk_c+0x280, pickUp; clearInfo unless flag bit 1 (then state 3)
        BOOL stepStyleItem(int kind);                   // 8005A2A0 104 (804A431C): EC = msgStyleEnd, requestItemAct(+0x280)
        int msgStyleEnd(msgInfo_s *info);           // 8005A338 EC "Q13_End"
        void endStyleItemFull(int arg);                    // 8005A364 F8 (804A431C): sendStyleLetterTo(0, 0), flag bit 0 on failure; state 3
        // ---- d_npc_talk_quest_q09 (.text 8005A3C4..8005C82C) ----
        int msgInviteOffer(msgInfo_s *info);        // 8005A3C4: offer check of QUEST_KIND_APPOINTMENT_0 (nml l_questOffer): outdoors in the town, canStartAppointment, then startQuestOffer(info, 9, F8 endQuestCommon, 104 stepInviteOffer, TRUE)
        BOOL stepInviteOffer(int kind);             // 8005A4E8 104: once idle, EC = msgInviteReq
        int msgInviteReq(msgInfo_s *info);          // 8005A560 EC "Q09_Req", 104 = stepInviteChoice
        BOOL stepInviteChoice(int kind);            // 8005A5C0 104: yes/no choice (selInviteYes / selInviteNo); clears the entry's quest
        void selInviteYes();                        // 8005A6C0 choice: EC = msgInviteReserve
        int msgInviteReserve(msgInfo_s *info);      // 8005A714 EC "Q09_Reserve", 104 = stepInviteTimeMenu
        BOOL stepInviteTimeMenu(int kind);          // 8005A778 104: once idle, the time menu (reqMenu27), result proc resInviteTime
        void resInviteTime();                       // 8005A7E8 result of the time menu: checks the entered time; EC = msgInviteReserved or an error
        int msgInviteReserved(msgInfo_s *info);     // 8005AC64 EC "Q09_Reserved", F8 = endInviteReserved
        void endInviteReserved(int arg);            // 8005ACC8 F8: dAnimalBlock_c::startAppointment, meet-time words, friendship +2
        int msgInviteTooSoon(msgInfo_s *info);      // 8005AE48 EC "Q09_Error1" (less than 30 minutes ahead), 104 = stepInviteTimeMenu
        int msgInviteTooLate(msgInfo_s *info);      // 8005AEAC EC "Q09_Error2" (more than 12 hours ahead or before 6:00), 104 = stepInviteTimeMenu
        int msgInviteSleeping(msgInfo_s *info);     // 8005AF10 EC "Q09_Error3" (the villager's sleep time), 104 = stepInviteTimeMenu
        void selInviteNo();                         // 8005AF74 choice: EC = msgInviteNo
        int msgInviteNo(msgInfo_s *info);           // 8005AFC8 EC "Q09_No"
        int msgInviteQuest(msgInfo_s *info);        // 8005AFF4 EC (nml msgQuestTalk [11]): talk with the villager of the appointment: meet-time words; 104 = stepInviteTalkMenu, stepInviteClear once the time has passed
        BOOL stepInviteTalkMenu(int kind);          // 8005B2E4 104: talk menu (getRollanChoice, getHarvestChoice) with "Q09_Con" (selInviteCon) and selInviteResume
        void selInviteCon();                        // 8005B4B8 choice: EC = msgInviteCon
        int msgInviteCon(msgInfo_s *info);          // 8005B50C EC "Q09_Con"
        void selInviteResume();                     // 8005B538 choice: getEventProcSet() set (default free talk), run EC
        BOOL stepInviteClear(int kind);             // 8005B58C 104: once idle, clears the appointment
        int msgInviteWelcome(msgInfo_s *info);      // 8005B5DC EC (nml l_talkProcSets [5]): "Q09_Welcome", F8 = endQuestCommon, 104 = stepInviteWelcome, friendship +10
        BOOL stepInviteWelcome(int kind);           // 8005B674 104: once idle, appointment flag 0
        int msgInviteRoomtalk(msgInfo_s *info);     // 8005B6C8 EC (room talk [0]): "Q_Roomtalk"
        static u16 pickHouseFtrMsg(dItem::Item *item, u8 looks); // 8005B6F8: random item of the villager's house (scene attr 0x850) with an npc message; returns the message code (0: none)
        int msgInviteFurniture(msgInfo_s *info);    // 8005B858 EC (room talk [1]): "Q09_Furniture" about a house item, the villager's music
        int msgInviteTradeOffer(msgInfo_s *info);   // 8005B940 EC (room talk [2]): "Q09_Trade1" when the trade is still open, 104 = stepInviteTradeOffer
        int msgInviteFirst(msgInfo_s *info);        // 8005BA28 EC (nml l_talkProcSets [6]): "Q09_First" once (flag 1), "Q09_Trade2" (flag 2 set, 3 clear), else a random room talk
        void endInviteFirst(int arg);               // 8005BBD0 F8: endQuestCommon, appointment flag 1
        BOOL stepInviteTradeOffer(int kind);        // 8005BC04 104: once idle, a single choice selInviteTradeOffer
        void selInviteTradeOffer();                 // 8005BC98 choice: appointment flag 2, fn_8019C34C
        u32 getSellPrice(const dItem::Item *item);  // 8005BCCC: the villager's price for selling its item: getPrice * (255 - friendship of _17C) / 512, at least 10; also called from d_npc_talk_fmarket
        u32 getBuyPrice(const dItem::Item *item);   // 8005BD4C: the most the villager pays for the item: getPrice * (255 + friendship) / 512 rounded to 10, at least 10 (0 without item, price or memory); called from d_npc_talk_fmarket
        int msgInviteTrade(msgInfo_s *info);        // 8005BE04 EC (nml l_talkProcSets [7]): "Q09_Trade3" (affordable, 104 = stepInviteTradeChoice) / "Q09_Trade4" for mItem0 (price getSellPrice), F8 = endQuestCommon
        BOOL stepInviteTradeChoice(int kind);       // 8005BFC0 104: yes/no choice (selInviteTradeYes / selInviteTradeNo)
        void selInviteTradeYes();                   // 8005C098 choice: EC = msgInviteTradeYes
        int msgInviteTradeYes(msgInfo_s *info);     // 8005C0EC EC "Q09_TradeYes" (returns 0), F8 = endInviteTradeYes, 104 = stepInviteTradeYes
        void endInviteTradeYes(int arg);            // 8005C178 F8: removes the room furniture, pays, picks up mItem0, appointment flag 3; 110 = actInviteTradeWait
        BOOL stepInviteTradeYes(int kind);          // 8005C284 104: once idle, fn_8019C35C
        void actInviteTradeWait();                  // 8005C2C8 110: waits for the furniture (fn_800A9354), unlocks, clears 110
        void selInviteTradeNo();                    // 8005C340 choice: EC = msgInviteTradeNo
        int msgInviteTradeNo(msgInfo_s *info);      // 8005C394 EC "Q09_TradeNo" (returns 0)
        int msgInviteWait(msgInfo_s *info);         // 8005C3C4 EC (nml l_talkProcSets [8]): "Q09_Wait", F8 = endQuestCommon
        int msgInviteAnalog(msgInfo_s *info);       // 8005C428 EC (nml l_talkProcSets [9]): "Q09_Analog", F8 = endQuestCommon, 104 = stepInvitePresentChoice
        BOOL stepInvitePresentChoice(int kind);     // 8005C4B4 104: once idle, the npc select (selInvitePresent)
        void selInvitePresent();                    // 8005C544 npc select: pickAppointmentPresent2 by house size and answer, EC = msgInvitePresent
        int msgInvitePresent(msgInfo_s *info);      // 8005C698 EC "Q09_Analog", code by house size and answer, 104 = stepInviteBye
        BOOL stepInviteBye(int kind);               // 8005C788 104: once idle, EC = msgInviteBye
        int msgInviteBye(msgInfo_s *info);          // 8005C800 EC "Q09_Bye" (returns 0)
        // ---- d_npc_talk_quest_q08 (.text 8005C82C..8005E584) ----
        int msgVisitOffer(msgInfo_s *info);         // 8005C82C: offer check of QUEST_KIND_APPOINTMENT_1 (nml l_questOffer): outdoors in the town, canStartAppointment, then startQuestOffer(info, 8, F8 endQuestCommon, 104 stepVisitOffer, TRUE)
        BOOL stepVisitOffer(int kind);              // 8005C950 104: once idle, EC = msgVisitReq
        int msgVisitReq(msgInfo_s *info);           // 8005C9C8 EC "Q08_Req", 104 = stepVisitChoice
        BOOL stepVisitChoice(int kind);             // 8005CA28 104: yes/no choice (selVisitYes / selVisitNo); clears the entry's quest
        void selVisitYes();                         // 8005CB28 choice: EC = msgVisitReserve
        int msgVisitReserve(msgInfo_s *info);       // 8005CB7C EC "Q08_Reserve", 104 = stepVisitTimeMenu
        BOOL stepVisitTimeMenu(int kind);           // 8005CBE0 104: once idle, the time menu (reqMenu27), result proc resVisitTime
        void resVisitTime();                        // 8005CC50 result of the time menu: checks the entered time; EC = msgVisitReserved or an error
        int msgVisitReserved(msgInfo_s *info);      // 8005D0CC EC "Q08_Reserved", F8 = endVisitReserved
        void endVisitReserved(int arg);             // 8005D130 F8: dAnimalBlock_c::startAppointment, meet-time words, friendship +2
        int msgVisitTooSoon(msgInfo_s *info);       // 8005D2B0 EC "Q08_Error1" (less than 30 minutes ahead), 104 = stepVisitTimeMenu
        int msgVisitTooLate(msgInfo_s *info);       // 8005D314 EC "Q08_Error2" (more than 12 hours ahead or before 6:00), 104 = stepVisitTimeMenu
        int msgVisitSleeping(msgInfo_s *info);      // 8005D378 EC "Q08_Error3" (the villager's sleep time), 104 = stepVisitTimeMenu
        void selVisitNo();                          // 8005D3DC choice: EC = msgVisitNo
        int msgVisitNo(msgInfo_s *info);            // 8005D430 EC "Q08_No"
        int msgVisitQuest(msgInfo_s *info);         // 8005D45C EC (nml msgQuestTalk [10]): talk with the villager of the appointment: meet-time words; 104 = stepVisitTalkMenu, stepVisitClear once the time has passed
        BOOL stepVisitTalkMenu(int kind);           // 8005D724 104: talk menu (getRollanChoice, getHarvestChoice) with "Q08_Con" (selVisitCon) and selVisitResume
        void selVisitCon();                         // 8005D8F8 choice: EC = msgVisitCon
        int msgVisitCon(msgInfo_s *info);           // 8005D94C EC "Q08_Con"
        void selVisitResume();                      // 8005D978 choice: getEventProcSet() set (default free talk), run EC
        BOOL stepVisitClear(int kind);              // 8005D9CC 104: once idle, clears the appointment
        int msgVisitCall(msgInfo_s *info);          // 8005DA1C EC (nml l_talkProcSets [1]): "Q08_Call", F8 = endQuestCommon, 104 = stepVisitCall
        BOOL stepVisitCall(int kind);               // 8005DAA8 104: once idle, EC = msgVisitDoor, setRequest1, result proc resVisitCall
        void resVisitCall();                        // 8005DB48 request result: 110 = actVisitWalk
        void actVisitWalk();                        // 8005DB88 110: the npc walks to l_walkTargetPos (the door); 110 = actVisitArrive
        void actVisitArrive();                      // 8005DC38 110: once arrived: wait, close the message, camera on the npc, clear 110, appointment flag 0
        int msgVisitDoor(msgInfo_s *info);          // 8005DCE4 EC "Q08_Door", friendship +10
        int msgVisitRoomtalk(msgInfo_s *info);      // 8005DD2C EC (room talk [0]): "Q_Roomtalk"
        static u32 pickRoomFtrMsg(u16 *msg, dItem::Item *item, u32 count, int layer, u8 looks); // 8005DD5C: random item of layer of the player's main room with an npc message into msg / item; returns the running count
        int msgVisitFurniture(msgInfo_s *info);     // 8005DE9C EC (room talk [1]): "Q08_Furniture" about an item of the player's room
        int msgVisitLayout(msgInfo_s *info);        // 8005DFA0 EC (room talk [2]): "Q08_Layout" by the house rating, star word, picks the present (pickAppointmentPresent)
        int msgVisitFirst(msgInfo_s *info);         // 8005E178 EC (nml l_talkProcSets [2]): "Q08_First" once (flag 1), else a random room talk
        void endVisitFirst(int arg);                // 8005E2BC F8: endQuestCommon, appointment flag 1
        int msgVisitWait(msgInfo_s *info);          // 8005E2F0 EC (nml l_talkProcSets [3]): "Q08_Wait", F8 = endQuestCommon, 104 = stepVisitWait
        BOOL stepVisitWait(int kind);               // 8005E37C 104: once idle, EC = msgVisitBye
        int msgVisitBye(msgInfo_s *info);           // 8005E3F4 EC "Q08_Bye", F8 = endVisitBye
        void endVisitBye(int arg);                  // 8005E454 F8: appointment state 2 (done)
        int msgVisitBack(msgInfo_s *info);          // 8005E480 EC (nml l_talkProcSets [4]): "Q08_Back", F8 = endQuestCommon, 104 = stepVisitBack
        BOOL stepVisitBack(int kind);               // 8005E50C 104: once idle, EC = msgVisitBye
        // ---- d_npc_talk_reaction (.text 8005E584..80060980) ----
        // Index n below = reaction_e n (d_npc_talk_reaction.hpp) = entry n of getReactionMsg's table.
        void resetActiveMood();                     // 8005E584 when the npc timer (getEntry()+0x250) mode > 1 (a mood runs): resetMood (virtual +0x38)
        int msgReMoveout(msgInfo_s *info);          // 8005E5DC  0 "Re_Moveout": dAnimal_c::isMovingOut; code 1..5 by the memory flags (talk_c+0x17C); 104 = stepMoveout
        int msgReCafe(msgInfo_s *info);             // 8005E740  1 "Re_Cafe": current scene SCENE_RM_MM_CAFE
        int msgReFishing(msgInfo_s *info);          // 8005E7CC  2 "Re_Fishing": always FALSE
        int msgReFall(msgInfo_s *info);             // 8005E7D4  3 "Re_Fall": the npc is in a pitfall
        int msgReRun(msgInfo_s *info);              // 8005E888  4 "Re_Run" (offline, fgMngProc_isBusy)
        int msgEvFirst(msgInfo_s *info);            // 8005E944  5 "Ev_First" (private flag 0xD, no memory; code by dAnimal_c::isMovingIn / isMovingOut)
        // First-meeting check shared by the Re_First* reactions: compares (a, b, c, movingIn) with "the
        // player has a memory entry with this npc" (talk_c+0x17C, per labelIdx 8..0xC), "the player is from
        // this town", "the npc came from this town" and dAnimal_c::isMovingIn; on a match fills info with
        // label labelIdx and code 0 / 0x15 (checkSick and sick) / 0x16 (checkLostItem and lost item request
        // not done); 104 = stepBeeFaceSeen with private flag 2. Inferred.
        BOOL msgFirstMeeting(msgInfo_s *info, u32 known, u32 fromTown, u32 npcFromTown, u32 movingIn, int labelIdx, BOOL checkSick, u8 checkLostItem); // 8005EA54
        void setPrevLandWord(int idx);                  // 8005EDA0 setLandName(idx) of the npc's original land (dAnimal_c+0x230C) and copy it to talk_c+0x1D2
        int msgReFirstA1(msgInfo_s *info);          // 8005EE18  6 "Re_FirstA1": msgFirstMeeting(info, 0, 1, 1, 1, 6, 0, 0)
        int msgReFirstA2(msgInfo_s *info);          // 8005EE58  7 "Re_FirstA2": msgFirstMeeting(info, 0, 1, 1, 0, 7, 1, 1); 104 = nml stepReaction
        int msgReFirstB1(msgInfo_s *info);          // 8005EEF4  8 "Re_FirstB1": msgFirstMeeting(info, 0, 1, 0, 1, 8, 0, 0), setPrevLandWord(0)
        int msgReFirstB2(msgInfo_s *info);          // 8005EF78  9 "Re_FirstB2": msgFirstMeeting + setPrevLandWord; 104 = nml stepReaction
        int msgReFirstC1(msgInfo_s *info);          // 8005F024 10 "Re_FirstC1": msgFirstMeeting + setPrevLandWord
        int msgReFirstC2(msgInfo_s *info);          // 8005F0A8 11 "Re_FirstC2": msgFirstMeeting + setPrevLandWord; 104 = nml stepReaction
        void setPlayerLandWord();                   // 8005F154 setLandName(0) of the current player's land (dPrivateData_c+0x7EC2)
        int msgReFirstV(msgInfo_s *info);           // 8005F1C4 12 "Re_FirstV": msgFirstMeeting(.., 0xC, ..) for four variants, setPlayerLandWord
        int msgReMovein(msgInfo_s *info);           // 8005F320 13 "Re_Movein": dAnimal_c::isMovingIn
        BOOL isReactionBlocked() const;                   // 8005F3F8 TRUE = no reaction now: no npc, player has private flag 0xD, lost item request open or the npc is sick
        int msgReBirthday(msgInfo_s *info);         // 8005F4B4 14 "Re_Birthday" (EVENT_PLAYER_BIRTHDAY_0 + player, leap year); 104 = stepBirthday
        // Not-seen-for-a-while reaction: days since the last talk (dTimeStamp_c at talk_c+0x17C +4) >=
        // minDays; code (days / divisor >= 12 for label 0xF); 104 = stepBeeFaceSeen with private flag 2.
        BOOL msgNotSeen(msgInfo_s *info, int minDays, int divisor, int labelIdx); // 8005F63C
        int msgRe30days(msgInfo_s *info);           // 8005F7C4 15 "Re_30days": msgNotSeen(info, 30 (LANGUAGE_JP) / 60, 30, 0xF)
        int msgRe7days(msgInfo_s *info);            // 8005F83C 16 "Re_7days": msgNotSeen(info, 7 (LANGUAGE_JP) / 14, 7, 0x10)
        int msgReTire(msgInfo_s *info);             // 8005F8B4 17 "Re_Tire": npc timer mode 4
        int msgReAnger(msgInfo_s *info);            // 8005F980 18 "Re_Anger": npc timer mode 2
        int msgReSad(msgInfo_s *info);              // 8005FA4C 19 "Re_Sad": npc timer mode 3
        int msgReBeeFace(msgInfo_s *info);          // 8005FB18 20 "Re_BeeFace": private flag 2 (stung face), not yet seen (memory flags 0x200); 104 = stepBeeFaceSeen
        // dPlayActorMng_c::forEachActiveActor callback of findNearItem (arg = the npc): keeps the nearest
        // item actor that is a scorpion or a tarantula (getActorItem) within 48 height and 96 distance in
        // npc mNearItem / mNearDist.
        static int findNearItemCb(dPlayActor_c *actor, void *npc); // 8005FC5C
        dItem::Item findNearItem();                  // 8005FDD8 the scorpion / tarantula nearest to the npc (0xFFF1 = none); struct return (r3 = result, r4 = this)
        int msgRePoison(msgInfo_s *info);           // 8005FE7C 21 "Re_Poison": npc timer flag 0x400 clear, outdoors in the town; item name of findNearItem; 104 = stepPoison
        int msgReXmas(msgInfo_s *info);             // 8005FFE4 22 "Re_Xmas": EVENT_TOY_DAY or Dec 25, fn_800F4250(looks, time); 104 = stepEventTalked
        int msgReNewyear(msgInfo_s *info);          // 800601F8 23 "Re_Newyear": EVENT_NEW_YEARS_DAY ongoing; 104 = stepEventTalked
        int msgReHarvest(msgInfo_s *info);          // 80060304 24 "Re_Harvest": EVENT_HARVEST_FESTIVAL over, dAnimalEventState_c::isInEvent; 104 = stepEventTalked
        int msgReHalloween(msgInfo_s *info);        // 80060450 25 "Re_Halloween": EVENT_HALLOWEEN over; 104 = stepEventTalked
        int msgReFireworks(msgInfo_s *info);        // 800605A8 26 "Re_Fireworks": EVENT_FIREWORKS over, before 2:00; 104 = stepEventTalked
        int msgReDownload(msgInfo_s *info);         // 8006071C 27 "download": a downloaded message (fn_80117318) when offline or alone
        BOOL stepMoveout(int kind);                 // 80060878 104: memory flags (talk_c+0x17C) |= 0x100
        BOOL stepBeeFaceSeen(int kind);             // 80060898 104: memory flags |= 0x200 (the bee face was seen; arg: the 104 slot's, unused; nml stepReaction passes it on)
        BOOL stepPoison(int kind);                 // 800608B8 104: npc timer (getEntry()) flags +0x28C |= 0x400
        BOOL stepEventTalked(int kind);                 // 800608F0 104: memory flags |= 0x1000000; fn_800F05C8(getNpcIdx(npc), talk_c+0x180)
        BOOL stepBirthday(int kind);                 // 8006094C 104: memory flags &= ~0x800
        // Item of an insect actor: dItem::Item(ITEM_IDX_COMMON_BUTTERFLY (the first insect), actor+0x2F0, 0);
        // struct return. Maybe an inline of the actor class. Inferred.
        static dItem::Item getActorItem(dPlayActor_c *actor); // 8006096C
        // ---- d_npc_talk_rollan (.text 80060980..800613BC) ----
        // The Rollan answer for this npc: one of the 4 choices of 8046CD30 (selRollanFloor /
        // selRollanFloorFull / selRollanWall / selRollanWallFull) by the npc's state (fn_8015112C: 1 old
        // flooring, 2 old wallpaper) and a free pocket, with its menu code (807502D8: 7, 7, 10, 10) in *code
        // and 3 in *flag (both may be NULL). Null when the player is not from this town, has private flag
        // 0xD, a visitor is here (dEvent::isVisitorHere(9)), the npc is moving / sick / has a lost item, ...
        // Struct return (r3 = result, r4 = this). Also adds the Rollan answer to the quest talk menus.
        const hookFunc getRollanChoice(u16 *code, u8 *flag); // 80060980
        int msgRollan(msgInfo_s *info);             // 80060C44 EC: getMsgLabel(TALK_ROLLAN, 0, 0) when getRollanChoice has an answer; setTopic(KIND_ANY, 0, 0)
        void endRollan(int arg);                    // 80060D28 F8: msgMemory set, talk count 0x44
        BOOL stepRollan(int kind);                  // 80060DB8 104: two choices: getRollanChoice's answer and nml selResumeTalk (804A4B60)
        // Present roll: needs a free pocket and no private flag 0xD; percent chance 20 + (sum of two bytes
        // of the player's home entry, fn_800AC28C) / 10, +5 when the player's fortune (+0x83F9) is 4, max 100.
        static BOOL rollRollanGift();               // 80060ED8
        void clearRollanFlag();                     // 80060FCC fn_801510EC(getTown() +0x72CC0, npc's dAnimalBlock_c index): clears this npc's bit
        void selRollanFloor();                      // 80061034 choice: EC = msgRollanFloorGift (present) or msgRollanFloorNone by rollRollanGift
        int msgRollanFloorGift(msgInfo_s *info);    // 800610C4 EC: present ITEM_IDX_OLD_FLOORING to mItem0, "Ev_Rollan" code 4..6
        int msgRollanFloorNone(msgInfo_s *info);    // 80061140 EC: "Ev_Rollan" code 1..3 (no present)
        void selRollanFloorFull();                  // 800611A4 choice (pockets full): EC = msgRollanFloorNone
        void selRollanWall();                       // 800611F8 choice: EC = msgRollanWallGift (present) or msgRollanWallNone by rollRollanGift
        int msgRollanWallGift(msgInfo_s *info);     // 80061288 EC: present ITEM_IDX_OLD_WALLPAPER to mItem0, "Ev_Rollan" code 0xE..0x10
        int msgRollanWallNone(msgInfo_s *info);     // 80061304 EC: "Ev_Rollan" code 0xB..0xD (no present)
        void selRollanWallFull();                   // 80061368 choice (pockets full): EC = msgRollanWallNone
        // ---- d_npc_talk_town (.text 800613BC..80061ADC) ----
        // EC (via 8046CD78): "Town_Rumor": a memory of a player from another town (save list at dSaveData_c::getRaw()
        // +0x735E0, fn_80116A74) that is not the current player; land / personal name as words 0 / 1,
        // code from the memory kind (jumptable 804A4BCC, dAnimal_c::findMemory2), unit word 2
        // ("sys_STRING/STR_Unit", fn_801A5874) or clearWord(2). FALSE when there is no such memory.
        int msgTownRumor(msgInfo_s *info);          // 800613BC
        int msgTownAlways(msgInfo_s *info);         // 80061670 EC: (via 8046CD78): "Town_Always", code 0; reloads record 17 (setProcSet)
        int msgTown(msgInfo_s *info);               // 800616D0 EC (record 17): rumor / always by the odds 807502E0 ({20, 80}), table 8046CD78; falls back to msgTownAlways
        int msgTownTheater(msgInfo_s *info);        // 80061790 EC (record 18): "Town_Theater", code 0x1F / 0x1E / sbss 8074EAC8 (theater state 80600874, fn_8016CD8C)
        int msgTownGrace(msgInfo_s *info);          // 80061844 EC (record 19): "Town_Grace", code by dSaveShopGrace_c sale stage / sold out / town flag 0x17
        static const char *get3PLabel(u32 looks);   // 80061934 "3P_*" label of a personality (table 804A4BF8; NULL for >= 6); nml getLabelByTable kind 0x14
        int msgTown3P(msgInfo_s *info);             // 80061958 EC (record 20): 3P label of the npc's chat partner (mpChatPartner's mpAnimal), anm-personal names 0 / 1

        /* 0x0EC */ msgFunc mMsgProc;
        /* 0x0F8 */ endFunc mHookProc;        // one-shot: cleared before the call (onMessageStart, 800313B0)
        /* 0x104 */ stepFunc mStepProc;       // onMessageEnd (8003209C)
        /* 0x110 */ hookFunc mActProc;        // action (setActProc / clearActProc; called by 80031254)
        /* 0x11C */ hookFunc mTalkEndProc;            // one-shot (called and cleared by 800325FC)
        /* 0x128 */ hookFunc mResultProc;     // menu result (8003129C: calls it, then moves +0x134 here)
        /* 0x134 */ hookFunc mNextResultProc;
        /* 0x140 */ reqFunc mReqProc[5];          // request handler slots (not initialized by the ctor;
                                              // init 80030A14 clears all 5 with one hoisted null)
        /* 0x17C */ dAnimalMemory_c *mpMemory; // the villager's memory of the current player; init NULL
        /* 0x180 */ u32 mMemoryIdx;                 // init -1; the memory index (getMemoryIdx); always tested
                                              // unsigned (cmplwi 0x10): < 16 = valid
        /* 0x184 */ int mJinxTown;                 // dSaveTownList_c index for the jinx topic (-1: this town)
        /* 0x188 */ int mRumorTown;                 // rumor town index
        /* 0x18C */ u32 mImpression;                 // impression level; init 50 = none; tested unsigned (< 50)
        /* 0x190 */ dPersonalID_c mMsgPersonal;
        /* 0x1BC */ dPlayerID_c mMsgPlayer;         // q13
        /* 0x1D2 */ dLandID_c mMsgLand;
        /* 0x1E8 */ u8 _1E8[0x1EC - 0x1E8];
        /* 0x1EC */ dPlayerID_c *mStylePlayer;      // the other player of the style quest (q13, pickStyleOtherPlayer)
        /* 0x1F0 */ u32 mTopicKind;                 // setTopic
        /* 0x1F4 */ int mMsgYear;                 // countdown year
        /* 0x1F8 */ int mFtrPosX;                 // fmarket deal
        /* 0x1FC */ int mFtrPosZ;                 // fmarket deal
        /* 0x200 */ int mFtrHandle;               // fg handle of the furniture at mFtrPosX/Z (fn_800A8F98; -1 none)
        /* 0x204 */ int mGiveFlag;                 // 1 when an item was given
        /* 0x208 */ dEquip_c mEquip;
        /* 0x210 */ u16 mPickCode;                 // init 0xFFFF; 0 when a message without a code starts (fixMsgCode)
        /* 0x212 */ u8 _212[0x214 - 0x212];
        /* 0x214 */ choice_c mChoice;         //
        /* 0x26C */ npcChoice_c mNpcChoice;   //
        // Six separate items, not an array: an Item array member makes the dtor call __destroy_arr
        // (dtor 800309A0 has none). (was mItems[6]; mItems[n] -> mItemN)
        /* 0x280 */ dItem::Item mItem0;       // init 0xFFF1; given/checked/sold (reqSell..)
        /* 0x282 */ dItem::Item mItem1;       // delivered
        /* 0x284 */ dItem::Item mItem2;       // the player's item of a trade (reqTrade)
        /* 0x286 */ dItem::Item mItem3;       // kept
        /* 0x288 */ dItem::Item mItem4;
        /* 0x28A */ dItem::Item mItem5;
        /* 0x28C */ int mPrice;                 // price / stake (fmarket, carnival)
        /* 0x290 */ dHmnName::Word_c mName;
        /* 0x2C8 */ wchar_t mMemoryText[17];    // a name (16 chars + terminator), setMemoryText
        /* 0x2EA */ char mLabel[0x3D];        // message label built by setLooksMsg
        /* 0x327 */ u8 mTopicGroup;                  // setTopic
        /* 0x328 */ u8 mTopicIdx;                  // setTopic
        /* 0x329 */ u8 mNoRecordTalk;                  // gates recordTalk
        /* 0x32A */ u8 mErrandDeadline;             // dQuestDeadline_e of the offered errand (q06/q07, dQuestBase_c::pickDeadline)
        /* 0x32B */ u8 mAnswer;                  // chosen answer index
        /* 0x32C */ u8 mCountTalk;                  // count this talk
        /* 0x32D */ u8 mHadNickname;                  // nickname flag
        /* 0x32E */ u8 mIsAnimalItem;                  // the item came from the villager
        /* 0x32F */ u8 mGameCount[4];         // carnival game counters: round, or losses [0] / wins [1]; variant flags [2] [3]
        /* 0x333 */ u8 mGameShown[2];         // carnival game 3: second win / loss variant shown
        /* 0x335 */ u8 mStartDay;                  // day stored by q10
    }; // size >= 0x336 (end not checked)

    // The villager's model resources (RTTI "dAcNpcNml_c::resMng_c", vtable 804A1478; code
    // 80030148..800303C0). Must stay defined after talk_c and before clothMng_c (vtable order).
    class resMng_c : public dAcNpc_c::resBase_c {
    public:
        resMng_c();                           // 80030148
        virtual ~resMng_c();                  // 80030194
        virtual BOOL create(dAcNpc_c *npc);   // 800301EC: alloc + load model / setup / texture
        virtual void *getMdlRes();            // 800303B0
        virtual void *getTexRes();            // 800303B8

        /* 0x4 */ void *mpMdl;   // 0x7200 bytes (fn_800F9D8C), species model (fn_800F9D94)
        /* 0x8 */ void *mpTex;   // 0x8480 bytes, LZ-uncompressed from the animal's setup data
        /* 0xC */ void *mpSetup; // "/Npc/Normal/Setup/%d.bin" (loadRes)
    }; // size 0x10

    // The villager's shirt (RTTI "dAcNpcNml_c::clothMng_c", vtable 804A1428; code
    // 800303C0..800308A8). Wraps a dHmnClothMng_c slot. Must stay defined after resMng_c.
    class clothMng_c : public dAcNpc_c::clothBase_c {
    public:
        clothMng_c();                                          // 800303C0
        virtual ~clothMng_c();                                 // 8003041C
        void init(dAcNpcNml_c *npc);                           // 80030484 (create): slot, mCloth
        BOOL isSlotValid();                                    // 800304FC
        virtual BOOL requestCloth(const dItem::Item *item, dDesign_c *design); // 80030518: request a shirt
        virtual BOOL requestNpcCloth(dAcNpc_c *npc);           // 800305F0: request the animal's shirt
        virtual BOOL create(dAcNpc_c *npc);                      // 8003061C: create (request mCloth)
        virtual void bindCloth(nw4r::g3d::ResMdl mdl);              // 80030690: bind + lock the loaded shirt
        virtual void swapCloth(nw4r::g3d::ResMdl mdl);              // 80030708: release "cloth", rebind
        BOOL syncLoad();                                       // 80030780
        virtual BOOL execute(dAcNpc_c *npc);                      // 80030788: execute
        virtual BOOL isLoaded(const dAcNpc_c *npc) const;          // 80030848: the loaded shirt is the animal's
        void clearLoaded();                                    // 80030898

        /* 0x4 */ dHmnCloth_c mHmnCloth;
        /* 0x6 */ dItem::Item mCurCloth;    // shown (set from mLoadedCloth by bindCloth)
        /* 0x8 */ dItem::Item mReqCloth;    // being requested
        /* 0xA */ dItem::Item mLoadedCloth; // request done, not bound yet
        /* 0xC */ dItem::Item mCloth;       // initial shirt (init: the animal's, else 0x373)
    }; // size 0x10

    // the actor (vtable 804A1310; code 8002ED4C..80030194, inline virtuals in the weak tail
    // 80036004..). Its constructor is not in the DOL.

    // The 8-byte net daub data of the npc (dAcNpc_c::getDaubData / setDaubData, dNpcDaub_c::get11).
    struct daubData_s {
        u8 mToolChange : 1; // setDaubToolChange group
        u8 mToolChanged : 1; // setDaubToolChanged group
        u8 mMaskChange : 1; // setDaubMaskChange group
        u8 mInHouse : 1; // setDaubInHouse group
        u8 mWatchFireworks : 1; // setDaubWatchFireworks group
        u8 mClothChange : 1; // setDaubClothChange group
        u8 mClothChanged : 1; // setDaubClothChanged group (cloth change: d_npc_talk_quest_q04 / delivery)
        u8 mFlag01 : 1;
        u8 _1[7];
    }; // size 0x8

    // vtable 804A1310 overrides
    // The weak tail (d_a_npc_nml.hpp section after __sinit) is isItchy, vtCC (needed first by the
    // virtual calls in talk_c), then getCloth, isInPitfall, vtC8, ~dAcNpcNml_c: inline virtuals not needed
    // earlier come out in REVERSE declaration order at the vtable, so the dtor is declared first (and
    // inline: an implicit dtor lands in the main .text) and getCloth after isInPitfall.
    virtual ~dAcNpcNml_c() {}                      // weak 8003602C (+0x48)
    virtual int create();                          // 8002F8A0
    virtual int preCreate();                       // 8002F814
    virtual int doDelete();                        // 8002F914
    virtual bool canInteract(dDemoActor_c *actor); // 8002ED4C (+0x4C)
    virtual int demoHook60(dDemoActor_c *actor);   // 8002EDE4 (+0x60)
    virtual void getName(dHmnName::Word_c *name, int len); // 8002EF5C (+0x88)
    virtual u8 getNameKind();                      // 8002EF74 (+0x8C): gender, 2 = no animal
    virtual f32 vt90() const;                          // 8002EF94 (+0x90)
    virtual f32 vt94() const;                          // 8002EFD4 (+0x94)
    virtual int getEarType();                      // 8002F038 (+0x98)
    virtual int getSoundId();                          // 8002F604 (+0xA0)
    virtual int getVoiceType() const;                          // 8002F090 (+0xA8)
    virtual void getManpuOfs(mVec3_c *ofs, mVec3_c *ofsL, mVec3_c *ofsR, u8 type); // 8002F6E4 (+0xAC)
    virtual u32 getHeapSize();                     // 8002EE44 (+0xB4)
    virtual u32 addToNpcList();                            // 8002EE4C (+0xB8)
    virtual void removeFromNpcList();                           // 8002EE54 (+0xBC)
    virtual int getFaceType();                     // 8002EE58 (+0xC0)
    // new virtuals (+0xC4..+0xE0)
    virtual int isItchy() { return 0; }               // weak 80036004
    virtual int vtC8() { return 0; }               // weak 80036024
    virtual int vtCC() { return 0; }               // weak 8003600C
    virtual int startNpcChat(dAcNpc_c *partner);           // 8002F804
    virtual int endNpcChat();                            // 8002F80C
    virtual int isInPitfall() { return 0; }               // weak 8003601C
    virtual void setAnimal();                      // 8002EE64 (+0xDC): mpAnimal / mpEntry (preCreate)
    virtual int removeRoomFtr(int obj, int x, int z);       // 8002F5BC
    virtual clothBase_c *getCloth() { return &mCloth; } // weak 80036014 (+0x78) (keep after isInPitfall: weak tail order)

    // Npc ids (dAcNpc_c::mNpcItem): kind 0xE; 0xE800.. special npcs, 0xE400.. (bit 0x400) excluded.
    static bool isNpcItem(const dItem::Item &item) { return ITEM_NAME_TYPE(item.mId) == 0xE; }
    static bool isSpNpcItem(const dItem::Item &item) { return ITEM_NAME_TYPE(item.mId) == 0xE && (item.mId & 0x800); }
    static bool isNpc400Item(const dItem::Item &item) { return ITEM_NAME_TYPE(item.mId) == 0xE && (item.mId & 0x400); }
    static bool isAnimalItem(const dItem::Item &item) { return isNpcItem(item) && !isSpNpcItem(item); }
    static bool isVillagerItem(const dItem::Item &item) { return isAnimalItem(item) && !isNpc400Item(item); }

    int getNpcIdx() const;                       // 8002EEB8: villager index (0..), sp npc 0x800.. - 0xE800, -1
    int getSpeciesEarType(u8 species);                   // 8002F014: ear type of a species (8046BCD0), 60 = none
    BOOL isTradeFtr(u32 idx);                     // 8002F0F8: furniture flag idx (< 10)
    void clearTradeFtr(u32 idx);                     // 8002F118: clear furniture flag
    int getTradeFtrNum();                             // 8002F138: number of furniture flags set
    static BOOL isSellableFtr(const dItem::Item *item); // 8002F1A0: furniture/fossil with a price 1..9999
    void selectTradeFtrs();                            // 8002F254: picks the furniture flags (3.. random)
    int getRoomFtrIdx(u32 a, u32 b);                 // 8002F3D0: room furniture slot (10 = none)
    static dItem::Item getRoomFtrAt(int *outX, int *outZ, int x, int z); // 8002F568
    static dItem::Item getRoomFtrAtPos(int *outX, int *outZ, const mVec3_c *pos); // 8002F574
    void onSessionFlag(u32 bit);                     // 8002F5C4: set mSessionFlags bit (< 8)
    BOOL isSessionFlag(u32 bit);                     // 8002F5E4
    static const Vec *searchPosTable(const posTable33_s *table, int num, int key, u8 idx); // 8002F684
    BOOL setDaubToolChange(BOOL on);                     // 8002F968: daub mToolChange
    void onDaubToolChange();                            // 8002FA0C
    void offDaubToolChange();                            // 8002FA14
    BOOL isDaubToolChange();                            // 8002FA1C
    BOOL setDaubToolChanged(BOOL on);                     // 8002FA64: daub mToolChanged
    void onDaubToolChanged();                            // 8002FB08
    void offDaubToolChanged();                            // 8002FB10
    BOOL isDaubToolChanged();                            // 8002FB18
    BOOL setDaubMaskChange(BOOL on);                     // 8002FB60: daub mMaskChange
    void onDaubMaskChange();                            // 8002FC04
    void offDaubMaskChange();                            // 8002FC0C
    BOOL isDaubMaskChange();                            // 8002FC14
    BOOL setDaubInHouse(BOOL on);                     // 8002FC5C: daub mInHouse
    void onDaubInHouse();                            // 8002FD00
    void offDaubInHouse();                            // 8002FD08
    BOOL isDaubInHouse();                            // 8002FD10
    BOOL setDaubWatchFireworks(BOOL on);                     // 8002FD58: daub mWatchFireworks
    void onDaubWatchFireworks();                            // 8002FDFC
    void offDaubWatchFireworks();                            // 8002FE04
    BOOL isDaubWatchFireworks();                            // 8002FE0C
    BOOL setDaubClothChange(BOOL on);                     // 8002FE54: daub mClothChange
    void onDaubClothChange();                            // 8002FEF8
    void offDaubClothChange();                            // 8002FF00
    BOOL isDaubClothChange();                            // 8002FF08
    BOOL setDaubClothChanged(BOOL on);                     // 8002FF50: daub mClothChanged
    void onDaubClothChanged();                            // 8002FFF4
    void offDaubClothChanged();                            // 8002FFFC
    BOOL isDaubClothChanged();                            // 80030004
    BOOL isClothChangeReady() const;                            // 8003004C: cloth differs from the animal's and mCloth is ready
    void changeClothModel();                            // 800300D4: swaps the cloth model
    static u32 calcHeapSize(BOOL arg);              // 80035B88: heap size

    /* 0x1C48 */ dAcNpc_c *mpChatPartner;
    /* 0x1C4C */ resMng_c mRes;
    /* 0x1C5C */ talk_c mNmlTalk;
    /* 0x1F94 */ dAnimal_c *mpAnimal;
    /* 0x1F98 */ dNpcEntry_c *mpEntry;
    /* 0x1F9C */ clothMng_c mCloth;
    /* 0x1FAC */ u8 _1FAC[4];          // clothMng_c is 0x10; unknown 4 bytes (or alignment of 0x1FB0)
    /* 0x1FB0 */ dNpcMood_c mMood; // (d_npc_talk_mood; dtor 800494E8)
    /* 0x2190 */ f32 mNearDist;           // d_npc_talk_reaction fn_8005FDD8: distance of mNearItem
    /* 0x2194 */ dItem::Item mNearItem;   // d_npc_talk_reaction fn_8005FDD8: nearest item actor's item
    /* 0x2196 */ u16 mTradeFtrFlags;        // furniture slots (bit per dAnimal_c::getFtr idx)
    /* 0x2198 */ u8 mBusy;
    /* 0x2199 */ u8 mSessionFlags;
}; // size 0x219C

// ---- d_a_npc_nml globals used by the talk TUs (placeholder names; defined in d_a_npc_nml.cpp) ----
// Message labels: const char arrays (.rodata 8046BD54.., .sdata2 8074FF30..), defined at the top of the
// TU, right after the ear-type table 8046BCD0 and before the first function (literal order).
extern const char l_Ai_Quest[];    // 8046BD54
extern const char l_Q_Cancel[];    // 8046BD60
extern const char l_Q_ItemFull[];  // 8046BD6C
extern const char l_Q_Timeover[];  // 8046BD78
extern const char l_Q_Timeover2[]; // 8046BD84
extern const char l_Q_Payback[];   // 8046BD90
extern const char l_Ev_Arbeit[];   // 8046BD9C
extern const char l_Q_Yes[];       // 8074FF30
extern const char l_Q_No[];        // 8074FF38
extern const char l_Q_Item[];      // 8074FF40
extern const char l_Q_Lost[];      // 8074FF48

// Procedure sets stored into talk_c by setProcSet. Their NULL slots are initialized from __ptmf_null
// in __sinit, including the const entry table. Records are addressed by index from the talk TUs.
extern dAcNpcNml_c::talk_c::procSet_s l_talkProcSets[dAcNpcNml_c::talk_c::TALK_PROC_NUM];  // 804A0784
extern const dAcNpcNml_c::talk_c::procSet_s l_talkEntrySets[dAcNpcNml_c::talk_c::TALK_NUM]; // 804A0E48 ([15] = free talk, the default)

extern mVec3_c l_walkTargetPos; // .bss: (256, 0, 464) walk target (d_npc_talk_fmarket, d_npc_talk_quest_q08)

// label tables read by talk_c::getMsgLabel (kinds 8 and 0xE). In .data they sit among later data
// (after 804A10D8 / before the vtables), so they are defined later in the file (next to the
// code that uses those kinds) and must be globals declared here (placeholder names).
extern const char *l_aiqLabels[3];       // 804A110C {"AiQ_Approach", "AiQ_First", "AiQ_Again"}
extern const char *l_aiGreetLabels[16];  // 804A12D0 {"Ai_Snow1", "Ai_Rain1", .., "AiV_Fine2"}
