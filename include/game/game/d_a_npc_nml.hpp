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
// +0x250), +0x1FB0 the birthday mood object (size 0x1E0, d_npc_talk_birthday), +0x19CC this talk_c,
// +0x16C0 action_c, +0xE8 anmSet_c.

#include <types.h>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>

class dAnimal_c;
class dPlayActor_c;
class dSceneChange_c;
class dAnimalMemory_c;
struct dSaveTownJinx_c; // d_save_town.hpp
class dNpcEntry_c;
class dQuestVillager_c;
namespace dNpc {
class msgMemory_c;
class msgMemorySecond_c;
}

// The villager's birthday mood (d_npc_talk_birthday: ctor 800493EC, dtor 800494E8, non-polymorphic;
// name inferred). State int at +0x0, a member-function pointer at +0x4, dNpcTimer_c at +0x10, three
// dLevelEffect_c at +0x20 / +0xB4 / +0x148, flags at +0x1DC..+0x1DF. Only the dtor matters here (the
// weak dAcNpcNml_c dtor 8003602C calls it).
class dAcNpcNml_c;

class dNpcBirthdayMood_c {
public:
    dNpcBirthdayMood_c();  // 800493EC
    ~dNpcBirthdayMood_c(); // 800494E8
    BOOL fn_800495CC();                         // 800495CC enabled: not +0x1DF, and fn_800DCF30() <= 1
    void fn_80049620(dAcNpcNml_c *npc, u32 mood); // 80049620 set wait / walk anm of the mood (0..4)
    void fn_800496F0(int mood, int hours);      // 800496F0 start the timer: mood for hours * 3600 (called from nml 80032DE0)
    static BOOL fn_80049754(dAcNpcNml_c *npc);  // 80049754 current anm is a mood walk / wait anm (entries 1..4)
    static BOOL fn_80049830(dAcNpcNml_c *npc);  // 80049830 current anm is a mood wait anm (entries 0..4)
    BOOL fn_800498CC(dAcNpcNml_c *npc);         // 800498CC execute: tick the timer, update anm, fn_80049B30
    BOOL fn_80049A64(dAcNpcNml_c *npc, u32 state); // 80049A64 enter state via local static PTMF table 804A2F10 (5 entries)
    void fn_80049B30(dAcNpcNml_c *npc, int state); // 80049B30 state change by current anm (0xC4..0xC9), run the ptmf at +0x4
    BOOL fn_80049CA0(dAcNpcNml_c *npc);         // 80049CA0 state 1 enter: ptmf = fn_80049CCC
    void fn_80049CCC(dAcNpcNml_c *npc);         // 80049CCC state 1: sound 0x18CF, "afm_mnp_happy_walk_L/R"
    BOOL fn_80049D98(dAcNpcNml_c *npc);         // 80049D98 state 2 enter: ptmf = fn_80049DC4
    void fn_80049DC4(dAcNpcNml_c *npc);         // 80049DC4 state 2: sound 0x18D1, "afm_mnp_pun_S"
    BOOL fn_80049F00(dAcNpcNml_c *npc);         // 80049F00 state 3 enter: ptmf = fn_80049F2C
    void fn_80049F2C(dAcNpcNml_c *npc);         // 80049F2C state 3: sound 0x18D0, "afm_mnp_sad_S"

    /* 0x000 */ u8 _0[0x1E0];
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
        virtual int getVoiceMood();                  // 80032D1C (+0x8C): birthday mood (0..4) by the entry's timer mode
        BOOL startBirthdayMood(int mood, int hours);     // 80032D7C starts the speaker's birthday mood timer
        virtual void resetMood();                 // 80032E04 (+0x38): startBirthdayMood(0, 0)
        virtual void setMood1(int hours);        // 80032E10 (+0x3C): startBirthdayMood(1, hours)
        virtual void setMood2(int hours);        // 80032E1C (+0x40): startBirthdayMood(2, hours)
        virtual void setMood3(int hours);        // 80032E28 (+0x44): startBirthdayMood(3, hours)
        virtual void setMood4(int hours);        // 80032E34 (+0x48): startBirthdayMood(4, hours)
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
        int msgYes(msgInfo_s *info);          // 80034ACC EC "Q_Yes" (code 4..6 by mQuestKind)
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
        inline dSaveTownJinx_c *getJinxTown();     // inline (defined in the .cpp): other town mJinxTown's jinx entry, NULL = this town

        // ---- Procedures of the d_npc_talk_* TUs. They are talk_c members: the nml tables store
        // them as constant {0,-1,fn} records, which MWCC only emits for members of talk_c itself
        // (a derived-class member pointer cast to talk_c is initialised at run time). ----
        // ---- d_npc_talk_approach (.text 80036324..80038134) ----
        // Label of an approach message: group 0 = 804A1C08 (4: "ApD_Moving", "ApD_Fortune" x3),
        // 1 = 804A1C70 (7: ApB_Habit/Hello/Nickname, ApC_Present/Sell/Trade/Want), 3 = .sdata 80749970
        // (2: "ApA_Letter", "ApA_Always"); NULL otherwise (group 2 has none). Also called by nml
        // getLabelByTable. The third argument is unused.
        static const char *fn_80036324(int group, u32 idx, int unused); // 80036324

        int fn_800363A4();                          // 800363A4 140 (after ApD_Moving): resets the town's moving-out villager index, clears dAnimal_c memory flag 23
        int fn_8003643C(msgInfo_s *info);           // 8003643C EC ApD_Moving: villager is moving out (talk_c+0x17C bit 23 clear); sets the bit, 140 = fn_800363A4
        int fn_80036564(msgInfo_s *info);           // 80036564 EC ApD_Fortune (codes 1..3): player fortune state 1, gender differs; 104 = fn_8003762C
        int fn_80036698(msgInfo_s *info);           // 80036698 EC ApD_Fortune (codes 4..6): player fortune state 2, same gender; 104 = fn_8003762C
        int fn_800367CC(msgInfo_s *info);           // 800367CC EC ApD_Fortune (codes 7..9): player fortune state 4; F8 = fn_8003766C
        int fn_800368DC(msgInfo_s *info);           // 800368DC EC: first of the ApD checks 8046C088 that picks a message
        int fn_80036974(msgInfo_s *info);           // 80036974 EC ApB_Habit (dAnimal_c+0x3037 == 0)
        int fn_80036A28(msgInfo_s *info);           // 80036A28 EC ApB_Hello (talk_c+0x17C bits 13..15 == 0)
        int fn_80036ADC(msgInfo_s *info);           // 80036ADC EC ApB_Nickname: friendship > 0, no nickname yet (fn_80038078); 104 = fn_80037888
        int fn_80036C28(msgInfo_s *info);           // 80036C28 EC ApC_Present: player has an empty pocket; F8 = fn_80037B58
        int fn_80036D08(msgInfo_s *info);           // 80036D08 EC ApC_Sell: empty pocket and >= 2000 bells; F8 = fn_80037BE0

        // Picks a random sellable item from the player's pockets (player NULL: current player); prefers
        // the item kind of 8046C0B8 indexed by the villager's byte at dAnimal_c+0x2BE4. 0xFFF1 if none.
        static void fn_80036E04(dItem::Item *out, dAcNpcNml_c::talk_c *talk, dPrivateData_c *player); // 80036E04

        int fn_80037038(msgInfo_s *info);           // 80037038 EC ApC_Trade: player has a sellable item; F8 = fn_80037E10
        int fn_80037108(msgInfo_s *info);           // 80037108 EC ApC_Want: player has a sellable item; F8 = fn_80037F1C
        int fn_800371D8(msgInfo_s *info);           // 800371D8 EC: up to 3 random tries over the 7 ApB/ApC checks 8046C0D8
        int fn_800372CC(msgInfo_s *info);           // 800372CC EC: nml msgPendingQuest(info, 25.0f) (25% chance)
        int fn_800372D4(msgInfo_s *info);           // 800372D4 EC ApA_Letter: villager holds letters, none from the player
        int fn_800373BC(msgInfo_s *info);           // 800373BC EC ApA_Always (fallback)
        int fn_80037448(msgInfo_s *info);           // 80037448 EC: random one of 8046C130 (Letter / Always), else ApA_Always
        int fn_800374DC(msgInfo_s *info);           // 800374DC EC root (nml 804A0784+0x240): isSpeakerItchy() ? ApA_Always : first of 8046C148
        void fn_8003759C(int arg);                  // 8003759C F8 (nml 804A0784+0x240): recordTalk(0), talk count 0x44, dNpc::msgMemory_c::set
        BOOL fn_8003762C();                         // 8003762C 104: once idle, sets talk_c+0x17C bit 20
        void fn_8003766C();                         // 8003766C F8 (fortune): lucky item into mItems[0] (fn_800F4510), EC = fn_800377A8 or fn_8003781C (pockets full)
        int fn_800377A8(msgInfo_s *info);           // 800377A8 EC ApD_Fortune, code = mMessageCode + 3
        int fn_8003781C(msgInfo_s *info);           // 8003781C EC ApD_Fortune, code 13
        BOOL fn_80037888();                         // 80037888 104: choice 0 = fn_8003791C
        void fn_8003791C();                         // 8003791C choice: saves the old nickname to mName, makes a new one, 104 = fn_80037A08
        BOOL fn_80037A08();                         // 80037A08 104: choices fn_80037AC8 / fn_80037AE8
        void fn_80037AC8();                         // 80037AC8 choice: talk_c+0x17C bits 16..19 = 0xE
        void fn_80037AE8();                         // 80037AE8 choice: friendship < 64: restores the nickname from mName
        void fn_80037B58();                         // 80037B58 F8 ApC_Present: gift item into mItems[0], word 0
        void fn_80037BE0();                         // 80037BE0 F8 ApC_Sell: villager's item (dAnimal_c::pickNewItem) and price (talk_c+0x28C)
        void fn_80037E10();                         // 80037E10 F8 ApC_Trade: villager's item mItems[0], player's item mItems[2] (fn_80036E04)
        void fn_80037F1C();                         // 80037F1C F8 ApC_Want: player's item (fn_80036E04) and offered price (talk_c+0x28C)
        static BOOL fn_80038078();                  // 80038078 any villager's memory of the current player has bits 16..19 set (nickname given)
        // ---- d_npc_talk_arbeit (.text 80038134..80039B38) ----
        // Check procedures of the table 8046C178 (tried in order by fn_800386A0); each returns TRUE when it
        // picked the message.
        int fn_80038134(msgInfo_s *info);           // 80038134 moving-in remark (dAnimal_c::isMovingIn, private flag 0xD)
        int fn_80038238(msgInfo_s *info);           // 80038238 moving-out remark (dAnimal_c::isMovingOut)
        BOOL fn_80038344(msgInfo_s *info, int errandKind, u8 errandState, int labelIdx); // 80038344 errand of this npc of the kind/state, item in pockets
        int fn_80038570(msgInfo_s *info);           // 80038570 fn_80038344(info, 0xC, 0, 2)
        int fn_80038580(msgInfo_s *info);           // 80038580 fn_80038344(info, 0xE, 0, 3)
        int fn_80038590(msgInfo_s *info);           // 80038590 fn_80038344(info, 0xF, 0, 4)
        int fn_800385A0(msgInfo_s *info);           // 800385A0 private flag 0xD set
        int fn_800386A0(msgInfo_s *info);           // 800386A0 EC: tries the 6 checks of 8046C178 (entry in nml table 804A0E48+0x24)
        void fn_80038738(int arg);                  // 80038738 F8: msgMemory set, talk count 0x44 when talk_c+0x32C
        BOOL fn_800387BC();                         // 800387BC 104: two choices (fn_80038894 / fn_800396EC)
        void fn_80038894();                         // 80038894 choice: select the errand item (reqSelectItem), 128 = fn_80038928
        void fn_80038928();                         // 80038928 128: take the selected pocket item, requestItemActEx
        void fn_80038A94();                         // 80038A94 134: EC = fn_80038AF0, close the message
        int fn_80038AF0(msgInfo_s *info);           // 80038AF0 EC: errand result message by errand kind (0xC..0xF)
        BOOL fn_80038D4C();                         // 80038D4C 104: requestHandActD, EC = fn_80038DCC
        int fn_80038DCC(msgInfo_s *info);           // 80038DCC EC: reward item (fn_800C60B4, default 0x640) to talk_c+0x280, pickUp
        BOOL fn_80038EB4();                         // 80038EB4 104: requestItemAct of the reward
        int fn_80038F40(msgInfo_s *info);           // 80038F40 EC
        BOOL fn_80038FA4();                         // 80038FA4 104: reqMenu1C, 128 = fn_80039014
        void fn_80039014();                         // 80039014 128: EC = fn_80039068
        int fn_80039068(msgInfo_s *info);           // 80039068 EC: player's birthday month/day as words 2/3
        BOOL fn_80039134();                         // 80039134 104: two choices (fn_8003920C / fn_80039364)
        void fn_8003920C();                         // 8003920C choice: EC = fn_80039278, errand state 2
        int fn_80039278(msgInfo_s *info);           // 80039278 EC: today is the player's / the npc's birthday
        void fn_80039364();                         // 80039364 choice: EC = fn_800393B8
        int fn_800393B8(msgInfo_s *info);           // 800393B8 EC
        BOOL fn_8003941C();                         // 8003941C 104: requestItemAct of talk_c+0x280
        int fn_800394A8(msgInfo_s *info);           // 800394A8 EC
        BOOL fn_8003950C();                         // 8003950C 104: errand state 2
        BOOL fn_80039564();                         // 80039564 104: errand state 1
        BOOL fn_800395BC();                         // 800395BC 104: reqMailMenu (letter errand, dAnimal_c+0x2C66)
        int fn_80039658(msgInfo_s *info);           // 80039658 EC
        int fn_800396BC(msgInfo_s *info);           // 800396BC EC: "Q_Cancel"
        void fn_800396EC();                         // 800396EC choice: EC = fn_80039740
        int fn_80039740(msgInfo_s *info);           // 80039740 EC: code from fn_800398B4
        BOOL fn_800397DC();                         // 800397DC 104: two choices (fn_800398F4 / fn_80039A94)
        static u16 fn_800398B4();                   // 800398B4 6 + player byte at dPrivateData_c+0x8696
        void fn_800398F4();                         // 800398F4 choice: EC = fn_80039948
        int fn_80039948(msgInfo_s *info);           // 80039948 EC: dPrivateData_c::inc_8696 when it reaches 0xB with 4 players
        BOOL fn_80039A44();                         // 80039A44 104: dPrivateData_c::inc_8696
        void fn_80039A94();                         // 80039A94 choice: EC = fn_80039AE8
        int fn_80039AE8(msgInfo_s *info);           // 80039AE8 EC
        // ---- d_npc_talk_free (.text 80039B38..8003F354) ----
        // Special-day conditions (PTMF table 8046C1C0, fn_80039CF4); this is unused.
        BOOL fn_80039B38();                         // 80039B38 today is Feb 29 (dTime_c::getCurrent: month 1, day 29)
        BOOL fn_80039B7C();                         // 80039B7C dEvent::isOngoing(EVENT_APRIL_FOOLS_DAY)
        BOOL fn_80039BAC();                         // 80039BAC weather topic: by hour (fn_801C9C08) or getWeatherPhaseA/B
        BOOL fn_80039C34();                         // 80039C34 dEvent::isActive(EVENT_JP_AUTUMN_MOON)
        BOOL fn_80039C64();                         // 80039C64 dEvent::isActive(EVENT_KR_DAEBOREUM)
        BOOL fn_80039C94();                         // 80039C94 dEvent::isActive(EVENT_NA_AUTUMN_MOON)
        BOOL fn_80039CC4();                         // 80039CC4 dEvent::isActive(EVENT_EU_AUTUMN_MOON)
        int fn_80039CF4();                          // 80039CF4 index of the first true condition of 8046C1C0, 7 if none

        static BOOL fn_80039D7C(BOOL withMoon);     // 80039D7C one of the 30 events of 8046C218 is active (withMoon: or one of the 5 of 8046C290)
        const u8 *fn_80039E38();                    // 80039E38 group weights (10 x u8, rows in 8046C290) for 8046C5A0: by town / visitor / special day / event
        static int fn_80039F74();                   // 80039F74 random other town (0..7) the current player is registered in, -1 if none
        dAnimalMemory_c *fn_8003A0FC();             // 8003A0FC random memory of this villager about another player (not the current one)
        const u8 *fn_8003A284();                    // 8003A284 FreeA weights (7 x u8, .sdata2 8074FFC0..80750010) for 8046C350

        // FreeA (8046C350)
        int fn_8003A3DC(msgInfo_s *info);           // 8003A3DC EC FreeA_Clothes: the player's shirt / hat / face item; 104 = fn_8003F184
        int fn_8003A814(msgInfo_s *info);           // 8003A814 EC FreeA_Always (fallback of fn_8003BA84)
        void fn_8003A894(const dPersonalID_c *creator, const wchar_t *designName, dAnimal_c *animal); // 8003A894 words 1..3: design creator name and land, design name (also nml msgRecall)
        int fn_8003A998(msgInfo_s *info);           // 8003A998 EC FreeA_Dress: the villager's shirt design and its creator
        int fn_8003AB40(msgInfo_s *info);           // 8003AB40 EC FreeA_Friend: two random villagers and their relation

        // Memory filters of fn_8003B174 (plain function-pointer table 8046C320, slots 1..11);
        // mem = the villager's memory picked by dAnimal_c::getRandomMemory, sameTown = it is of this town.
        static BOOL fn_8003AD10(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AD10 sameTown, friendship level 0
        static BOOL fn_8003AD4C(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AD4C sameTown, friendship level 2
        static BOOL fn_8003AD88(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AD88 sameTown, friendship level 1
        static BOOL fn_8003ADC4(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003ADC4 sameTown, has a letter from that player
        static BOOL fn_8003AE0C(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE0C sameTown, memory item (mem+0x84) set
        static BOOL fn_8003AE30(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE30 !sameTown, memory bit 25 clear
        static BOOL fn_8003AE54(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE54 !sameTown, has a letter from that player
        static BOOL fn_8003AE9C(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AE9C !sameTown, bit 25, friendship level 0
        static BOOL fn_8003AEE4(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AEE4 !sameTown, bit 25, friendship level 2
        static BOOL fn_8003AF2C(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AF2C !sameTown, bit 25, friendship level 1
        static BOOL fn_8003AF74(dAnimalMemory_c *mem, dAnimal_c *animal, BOOL sameTown); // 8003AF74 the villager's previous town (dAnimal_c+0x230C) is another town

        void fn_8003B018(const dPersonalID_c *pid, const dLandID_c *land, u32 impress, const dItem::Item *item, dAnimal_c *animal); // 8003B018 words 0..4 of the memory topic, impression (talk_c+0x18C), item (talk_c+0x288) (also nml msgRecall)
        int fn_8003B174(msgInfo_s *info);           // 8003B174 EC FreeA_Memory: code 1..12 by the filters of 8046C320
        int fn_8003B3B4(msgInfo_s *info);           // 8003B3B4 EC FreeA_Rumor: news of a visited town (talk_c+0x188 = its index)
        void fn_8003B858(const u8 *jinx, const dLandID_c *land); // 8003B858 words of a jinx entry: item by kind nibble, land, number (also nml msgRecall)
        int fn_8003B960(msgInfo_s *info);           // 8003B960 EC FreeA_Jinx: jinx of another town (talk_c+0x184 = its index); 104 = fn_8003EE44
        int fn_8003BA84(msgInfo_s *info);           // 8003BA84 EC: FreeA topic by the weights of fn_8003A284, else FreeA_Always

        // A2 special days (8046C3A4)
        int fn_8003BBC8(msgInfo_s *info);           // 8003BBC8 EC FreeA_0229
        int fn_8003BC48(msgInfo_s *info);           // 8003BC48 EC FreeA_0401: land of a random remembered player of another town
        int fn_8003BE24(msgInfo_s *info);           // 8003BE24 EC FreeA_Weather: code by hour / weather (50%)
        int fn_8003BF70(msgInfo_s *info);           // 8003BF70 EC FreeA_MoonJPN (code 1 day, 2 night)
        int fn_8003C01C(msgInfo_s *info);           // 8003C01C EC FreeA_MoonKOR
        int fn_8003C0C8(msgInfo_s *info);           // 8003C0C8 EC FreeA_MoonUSA
        int fn_8003C174(msgInfo_s *info);           // 8003C174 EC FreeA_MoonEUR
        int fn_8003C220(msgInfo_s *info);           // 8003C220 EC: special-day topic of fn_80039CF4 (if isCurrentSceneAttr(5))

        // FreeB hobby topics
        int fn_8003C2BC(msgInfo_s *info);           // 8003C2BC EC FreeB_Bug: two bugs of the season (dInsectInfo::getRandomByRarity) in mItems[4]/[5]
        int fn_8003C458(msgInfo_s *info);           // 8003C458 EC FreeB_Fassion
        int fn_8003C4D8(msgInfo_s *info);           // 8003C4D8 EC FreeB_Fish: two fish of the season (dFishInfo::getRandomByRarity) in mItems[4]/[5]
        int fn_8003C674(msgInfo_s *info);           // 8003C674 EC FreeB_Fossil: a fossil (dItem::seeker_c::getRandomFossil) in mItems[4]
        int fn_8003C770(msgInfo_s *info);           // 8003C770 EC FreeB_Gardening
        int fn_8003C820(msgInfo_s *info);           // 8003C820 EC FreeB_Interior
        int fn_8003C8A0(msgInfo_s *info);           // 8003C8A0 EC FreeB_Party
        int fn_8003C920(msgInfo_s *info);           // 8003C920 EC FreeB_Hint (fallback of fn_8003C9A0)
        int fn_8003C9A0(msgInfo_s *info);           // 8003C9A0 EC: hobby topic by dAnimal_c+0x2BE4 (70%), else FreeB_Hint

        // ApC item offers (8046C3F8; the F8 hooks are d_npc_talk_approach's)
        int fn_8003CAB8(msgInfo_s *info);           // 8003CAB8 EC ApC_Present, F8 = fn_80037B58
        int fn_8003CB58(msgInfo_s *info);           // 8003CB58 EC ApC_Sell, F8 = fn_80037BE0
        int fn_8003CBF8(msgInfo_s *info);           // 8003CBF8 EC ApC_Trade, F8 = fn_80037E10
        int fn_8003CC98(msgInfo_s *info);           // 8003CC98 EC ApC_Want, F8 = fn_80037F1C
        const u8 *fn_8003CD38();                    // 8003CD38 ApC weights (4 x u8, .sdata2 80750024..80750038) by money, empty pocket, sellable item
        int fn_8003CE58(msgInfo_s *info);           // 8003CE58 EC: ApC topic by the weights of fn_8003CD38

        int fn_8003CF40(msgInfo_s *info);           // 8003CF40 EC FreeD_Moving2: villager moving out (talk_c+0x17C bit 23 set); 140 = fn_800363A4

        // FreeE
        const u8 *fn_8003D04C();                    // 8003D04C FreeE weights (2 x u8, .sdata2 8075003C..80750044)
        static int fn_8003D0DC(dQuestEvent_e event, int from, int to, const dTime_c &time); // 8003D0DC last day d in [to, from] with dEvent::isEventWithin(event, time, d, d), -1 if none
        int fn_8003D180(msgInfo_s *info);           // 8003D180 EC FreeE_Event: code by the current / next town event
        int fn_8003D5C0(msgInfo_s *info);           // 8003D5C0 EC FreeE_Snpc: code by the special npc in town (8046C428 / 8046C440)
        int fn_8003D710(msgInfo_s *info);           // 8003D710 EC: FreeE topic (8046C468, or 8046C480 when fn_80039D7C(1)) by fn_8003D04C

        // FreeF
        static BOOL fn_8003D7D0(const dItem::Item *building, int xMin, int xMax, int zMin, int zMax); // 8003D7D0 the building's block position (dSaveBuildingList_c::getPos) lies in the range
        int fn_8003D894(msgInfo_s *info);           // 8003D894 EC FreeF_Building / House1 / House2 / Inside: what the villager stands next to

        // FreeG
        static const u8 *fn_8003DC3C();             // 8003DC3C FreeG weights (3 x u8, .sdata2 80750048 own town / 8075004C visitor)
        int fn_8003DCDC(msgInfo_s *info);           // 8003DCDC EC FreeG_Host
        void fn_8003DD5C();                         // 8003DD5C words: host player's name (dPlayerMgr_c::getNetPlayer(0)) and the current player's land (also nml msgRecall)
        int fn_8003DE3C(msgInfo_s *info);           // 8003DE3C EC FreeG_Visitor (code 2 when talk_c+0x17C bit 25)
        int fn_8003DEF8(msgInfo_s *info);           // 8003DEF8 EC FreeG_JinxV: jinx of this town (talk_c+0x184 = -1); 104 = fn_8003EE44
        int fn_8003DFE8(msgInfo_s *info);           // 8003DFE8 EC: FreeG topic by the weights of fn_8003DC3C

        // FreeH
        BOOL fn_8003E0B4(dQuestEvent_e event);      // 8003E0B4 villager in the event state, the town's current event is event and it is active
        BOOL fn_8003E13C();                         // 8003E13C EVENT_HARVEST_FESTIVAL and not over
        BOOL fn_8003E184();                         // 8003E184 EVENT_HALLOWEEN not started yet
        BOOL fn_8003E1CC();                         // 8003E1CC EVENT_COUNTDOWN not started yet
        BOOL fn_8003E214();                         // 8003E214 isCurrentSceneAttr(5) and EVENT_FLEA_MARKET
        BOOL fn_8003E26C();                         // 8003E26C EVENT_TOY_DAY and not over
        BOOL fn_8003E2B4();                         // 8003E2B4 villager valid and EVENT_BUNNY_DAY ongoing
        int fn_8003E308();                          // 8003E308 random index of a true condition of 8046C4F8, 6 if none
        int fn_8003E3F0(msgInfo_s *info);           // 8003E3F0 EC FreeH_Harvest (code 2 while ongoing)
        int fn_8003E49C(msgInfo_s *info);           // 8003E49C EC FreeH_Countdown: next year as word 0 (talk_c+0x1F4)
        int fn_8003E544(msgInfo_s *info);           // 8003E544 EC FreeH_Fmarket
        int fn_8003E5C4(msgInfo_s *info);           // 8003E5C4 EC FreeH_Xmas
        int fn_8003E688(msgInfo_s *info);           // 8003E688 EC FreeH_Easter
        int fn_8003E708(msgInfo_s *info);           // 8003E708 EC FreeH_Halloween
        int fn_8003E7B4(msgInfo_s *info);           // 8003E7B4 EC: FreeH topic 8046C540 of fn_8003E308

        // FreeI
        int fn_8003E838(msgInfo_s *info);           // 8003E838 EC FreeI_Tunekichi (code 1), talk_c+0x17C bit 21
        int fn_8003E8E4();                          // 8003E8E4 140: dAnimal_c::sendTunekichiLetter, talk_c+0x17C bit 22
        int fn_8003E994(msgInfo_s *info);           // 8003E994 EC FreeI_Tunekichi (code 2), 140 = fn_8003E8E4
        int fn_8003EA5C(msgInfo_s *info);           // 8003EA5C EC: FreeI topic (8046C588) by the player's flags / player count ("PF<2" weights), else fn_8003BA84

        int fn_8003EC4C(msgInfo_s *info);           // 8003EC4C EC root (nml 804A0E48+0x21C record): group by the weights of fn_80039E38, else fn_8003BA84
        void fn_8003EDEC(int arg);                  // 8003EDEC F8 (nml 804A0E48+0x21C record): recordTalk(0), dNpc::msgMemory_c::set

        // Jinx (two messages from the jinx byte: high nibble, then low nibble)
        BOOL fn_8003EE44(int kind);                 // 8003EE44 104: once idle, procedures {fn_8003EF14, null, fn_8003EFF8} (setProcs) (also nml 804A1180)
        int fn_8003EF14(msgInfo_s *info);           // 8003EF14 EC: FreeA_Jinx / FreeG_JinxV, code 11 + high nibble
        BOOL fn_8003EFF8();                         // 8003EFF8 104: once idle, procedures {fn_8003F0A0, null, null}
        int fn_8003F0A0(msgInfo_s *info);           // 8003F0A0 EC: FreeA_Jinx / FreeG_JinxV, code 31 + low nibble

        BOOL fn_8003F184();                         // 8003F184 104 (Clothes): once idle, talk_c+0x17C bit 10, remembers mItems[0] at memory+0x86

        // Etc lines (records of nml's table 804A0784)
        int fn_8003F1D8(msgInfo_s *info);           // 8003F1D8 EC "Etc_Hit", record 804A0784+0x438
        void fn_8003F224(int arg);                  // 8003F224 F8 of the Etc records: like d_npc_talk_approach fn_8003759C
        int fn_8003F2B4(msgInfo_s *info);           // 8003F2B4 EC "Etc_Push", record 804A0784+0x45C
        int fn_8003F304(msgInfo_s *info);           // 8003F304 EC "Etc_Flea", record 804A0784+0x480
        // ---- d_npc_talk_carnival (.text 8003F354..80043C34) ----
        int fn_8003F354(msgInfo_s *info);           // 8003F354 EC: Festivale start, {fn_8003F354, fn_8003F6A0, null} at nml 804A0E48+0x90; state 0..0x18 ("Ev_Carnival"), 104 = a game start by state
        void fn_8003F6A0(int arg);                  // 8003F6A0 F8: msgMemory set, talk count 0x44, dAnimalMemory_c event flag 0, fn_800F04E8
        BOOL fn_8003F758();                         // 8003F758 104: give a random feather (requestItemAct), EC = fn_8003F83C
        int fn_8003F83C(msgInfo_s *info);           // 8003F83C EC: "Ev_Carnival" 0xD
        BOOL fn_8003F86C(msgFunc next);             // 8003F86C once the controller is idle: EC = next, start the message
        static int fn_8003F8E0(u16 *slotMask, dPrivateData_c *player); // 8003F8E0 count the sellable pocket items (price > 0, purchasable), mask of their slots
        BOOL fn_8003F9D4(msgInfo_s *info, const char *label, stepFunc next); // 8003F9D4 pick the stake (a feather from the pockets, 500 bells, or an item via fn_8003F8E0), message code 3..6, 104 = next
        BOOL fn_8003FDD8(hookFunc yes);             // 8003FDD8 once idle: two answers, 0 = yes (handler), 1 = cancel
        // Game 1 ("Ev_Carnival1")
        BOOL fn_8003FE74();                         // 8003FE74 104: fn_8003F86C(fn_8003FEB4)
        int fn_8003FEB4(msgInfo_s *info);           // 8003FEB4 EC: fn_8003F9D4(info, "Ev_Carnival1", fn_8003FEFC)
        BOOL fn_8003FEFC();                         // 8003FEFC 104: fn_8003FDD8(fn_8003FF3C)
        void fn_8003FF3C();                         // 8003FF3C choice: 104 = fn_8003FF7C
        BOOL fn_8003FF7C();                         // 8003FF7C 104: EC = fn_8003FFF4
        int fn_8003FFF4(msgInfo_s *info);           // 8003FFF4 EC: 0xB, 104 = fn_80040060
        BOOL fn_80040060();                         // 80040060 104: two answers, both fn_80040120
        void fn_80040120();                         // 80040120 choice: 104 = fn_80040160
        BOOL fn_80040160();                         // 80040160 104: EC = fn_800401D8
        int fn_800401D8(msgInfo_s *info);           // 800401D8 EC: 70% / 85% chance (+0x32F), 0xE..0x11; win: feather prize; F8 / 104 by result
        void fn_800403E4();                         // 800403E4 F8: mpNpc->mAudioObj virtual +0xC (sound 0x17A1)
        void fn_80040408();                         // 80040408 F8: mpNpc->mAudioObj virtual +0xC (sound 0x17A2)
        BOOL fn_8004042C();                         // 8004042C 104: hand over the prize (+0x280) into the pockets (requestItemAct), EC = fn_800404E0
        int fn_800404E0(msgInfo_s *info);           // 800404E0 EC: 0x16
        BOOL fn_80040510();                         // 80040510 104: EC = fn_80040588
        int fn_80040588(msgInfo_s *info);           // 80040588 EC: take the stake (500 bells or the pocket item), 0x13..0x15, 104 = fn_80040708
        BOOL fn_80040708();                         // 80040708 104: requestItemActEx(6, stake), EC = fn_800407AC
        int fn_800407AC(msgInfo_s *info);           // 800407AC EC: 0x17
        // Game 2 ("Ev_Carnival2", rock-paper-scissors)
        BOOL fn_800407DC();                         // 800407DC 104: fn_8003F86C(fn_8004081C)
        int fn_8004081C(msgInfo_s *info);           // 8004081C EC: fn_8003F9D4(info, "Ev_Carnival2", fn_80040864)
        BOOL fn_80040864();                         // 80040864 104: fn_8003FDD8(fn_800408A4)
        void fn_800408A4();                         // 800408A4 choice: clear +0x32F / +0x330, 104 = fn_800409CC
        BOOL fn_800408F0();                         // 800408F0 104: EC = fn_80040968
        int fn_80040968(msgInfo_s *info);           // 80040968 EC: 0xB, 104 = fn_800409CC
        BOOL fn_800409CC();                         // 800409CC 104: three answers, all fn_80040AC8
        void fn_80040AC8();                         // 80040AC8 choice: EC = fn_80040B1C, start the message
        int fn_80040B1C(msgInfo_s *info);           // 80040B1C EC: the npc's random hand vs. +0x32B, 0xC..0xE, counts wins / losses
        BOOL fn_80040CC0();                         // 80040CC0 104: EC = fn_80040D38
        int fn_80040D38(msgInfo_s *info);           // 80040D38 EC: 0x14..0x16 by +0x32B, F8 = fn_80040408, 104 = fn_80040FFC
        BOOL fn_80040DF0();                         // 80040DF0 104: EC = fn_80040E68
        int fn_80040E68(msgInfo_s *info);           // 80040E68 EC: by +0x32B, F8 = fn_800403E4, 104 = fn_80040FFC
        BOOL fn_80040F20();                         // 80040F20 104: EC = fn_80040F98
        int fn_80040F98(msgInfo_s *info);           // 80040F98 EC: 0xF (draw), 104 = fn_800409CC
        BOOL fn_80040FFC();                         // 80040FFC 104: EC = fn_80041074
        int fn_80041074(msgInfo_s *info);           // 80041074 EC: 0x17..0x1B by the win / loss counts; 104 = next round / prize / payment
        BOOL fn_800411B8();                         // 800411B8 104: EC = fn_80041230
        int fn_80041230(msgInfo_s *info);           // 80041230 EC: 0x1C, random feather prize, 104 = fn_800412CC
        BOOL fn_800412CC();                         // 800412CC 104: hand over the prize (requestItemAct), EC = fn_80041368
        int fn_80041368(msgInfo_s *info);           // 80041368 EC: 0x20
        BOOL fn_80041398();                         // 80041398 104: EC = fn_80041410
        int fn_80041410(msgInfo_s *info);           // 80041410 EC: take the stake, 104 = fn_80041590
        BOOL fn_80041590();                         // 80041590 104: requestItemActEx(6, stake), EC = fn_80041614
        int fn_80041614(msgInfo_s *info);           // 80041614 EC: 0x21
        // Game 3 ("Ev_Carnival3")
        BOOL fn_80041644();                         // 80041644 104: fn_8003F86C(fn_80041684)
        int fn_80041684(msgInfo_s *info);           // 80041684 EC: fn_8003F9D4(info, "Ev_Carnival3", fn_800416CC)
        BOOL fn_800416CC();                         // 800416CC 104: fn_8003FDD8(fn_8004170C)
        void fn_8004170C();                         // 8004170C choice: clear the counters, 104 = fn_80041758
        BOOL fn_80041758();                         // 80041758 104: two answers, both fn_80041818
        void fn_80041818();                         // 80041818 choice: 104 = fn_80041858
        BOOL fn_80041858();                         // 80041858 104: EC = fn_800418D0
        int fn_800418D0(msgInfo_s *info);           // 800418D0 EC: round 1, 20% chance, 0xF..0x12, 104 = fn_80041A24
        BOOL fn_80041A24();                         // 80041A24 104: two answers, both fn_80041AE4
        void fn_80041AE4();                         // 80041AE4 choice: 104 = fn_80041B24
        BOOL fn_80041B24();                         // 80041B24 104: EC = fn_80041B9C
        int fn_80041B9C(msgInfo_s *info);           // 80041B9C EC: round 2, 20% chance, 104 = fn_80041CF0
        BOOL fn_80041CF0();                         // 80041CF0 104: EC = fn_80041D68
        int fn_80041D68(msgInfo_s *info);           // 80041D68 EC: result 0x1B / 0x1D / 0x1E by the win / loss counts; 104 = retry / prize / payment
        BOOL fn_80041E48();                         // 80041E48 104: EC = fn_80041EC0
        int fn_80041EC0(msgInfo_s *info);           // 80041EC0 EC: 0xC (draw), 104 = fn_80041758
        BOOL fn_80041F30();                         // 80041F30 104: random feather prize, 104 = fn_80041FBC
        BOOL fn_80041FBC();                         // 80041FBC 104: hand over the prize (requestItemAct), EC = fn_80042064
        int fn_80042064(msgInfo_s *info);           // 80042064 EC: 0x23
        BOOL fn_80042094();                         // 80042094 104: EC = fn_8004210C
        int fn_8004210C(msgInfo_s *info);           // 8004210C EC: take the stake, 104 = fn_8004228C
        BOOL fn_8004228C();                         // 8004228C 104: requestItemActEx(6, stake), EC = fn_80042330
        int fn_80042330(msgInfo_s *info);           // 80042330 EC: 0x24
        // Game 4 ("Ev_Carnival4")
        BOOL fn_80042360();                         // 80042360 104: fn_8003F86C(fn_800423A0)
        int fn_800423A0(msgInfo_s *info);           // 800423A0 EC: fn_8003F9D4(info, "Ev_Carnival4", fn_800423E8)
        BOOL fn_800423E8();                         // 800423E8 104: fn_8003FDD8(fn_80042428)
        void fn_80042428();                         // 80042428 choice: clear the counters, 104 = fn_80042470
        BOOL fn_80042470();                         // 80042470 104: two answers, both fn_80042530
        void fn_80042530();                         // 80042530 choice: 104 = fn_80042570
        BOOL fn_80042570();                         // 80042570 104: EC = fn_800425E8
        int fn_800425E8(msgInfo_s *info);           // 800425E8 EC: round +0x32F (chance from 8046C6B0), 0xE..0x13; 104 = next round / prize choice / payment
        BOOL fn_800427C8();                         // 800427C8 104: EC = fn_80042840
        int fn_80042840(msgInfo_s *info);           // 80042840 EC: 0xB, 104 = fn_80042470
        BOOL fn_800428A4();                         // 800428A4 104: four answers (feather colours), all fn_800429CC
        void fn_800429CC();                         // 800429CC choice: prize = feather +0x32B (local static dItem::Item[4] 0x46/0x45/0x47/0x48), 104 = fn_80042B08
        BOOL fn_80042B08();                         // 80042B08 104: hand over the prize (requestItemAct), EC = fn_80042BBC
        int fn_80042BBC(msgInfo_s *info);           // 80042BBC EC: 0x19
        BOOL fn_80042BEC();                         // 80042BEC 104: EC = fn_80042C64
        int fn_80042C64(msgInfo_s *info);           // 80042C64 EC: take the stake, 104 = fn_80042DE4
        BOOL fn_80042DE4();                         // 80042DE4 104: requestItemActEx(6, stake), EC = fn_80042E68
        int fn_80042E68(msgInfo_s *info);           // 80042E68 EC: 0x1A
        // Game 5 ("Ev_Carnival5")
        BOOL fn_80042E98();                         // 80042E98 104: fn_8003F86C(fn_80042ED8)
        int fn_80042ED8(msgInfo_s *info);           // 80042ED8 EC: fn_8003F9D4(info, "Ev_Carnival5", fn_80042F20)
        BOOL fn_80042F20();                         // 80042F20 104: fn_8003FDD8(fn_80042F60)
        void fn_80042F60();                         // 80042F60 choice: clear the counters, 104 = fn_80042FA8
        BOOL fn_80042FA8();                         // 80042FA8 104: three answers, all fn_800430A4
        void fn_800430A4();                         // 800430A4 choice: 104 = fn_800430E4
        BOOL fn_800430E4();                         // 800430E4 104: EC = fn_8004315C
        int fn_8004315C(msgInfo_s *info);           // 8004315C EC: guess one of three (85%), 0xF..0x12; 104 = fn_8004358C / payment
        BOOL fn_800432C0();                         // 800432C0 104: EC = fn_80043338
        int fn_80043338(msgInfo_s *info);           // 80043338 EC: take the stake, 104 = fn_800434B8
        BOOL fn_800434B8();                         // 800434B8 104: requestItemActEx(6, stake), EC = fn_8004355C
        int fn_8004355C(msgInfo_s *info);           // 8004355C EC: 0x25
        BOOL fn_8004358C();                         // 8004358C 104: five answers, all fn_800436E0
        void fn_800436E0();                         // 800436E0 choice: 104 = fn_80043720
        BOOL fn_80043720();                         // 80043720 104: EC = fn_80043798
        int fn_80043798(msgInfo_s *info);           // 80043798 EC: guess one of five (70%), 0x19..0x1E; 104 = fn_800438EC / payment
        BOOL fn_800438EC();                         // 800438EC 104: four answers (feather colours), all fn_80043A14
        void fn_80043A14();                         // 80043A14 choice: prize = feather +0x32B (local static dItem::Item[4]), 104 = fn_80043B50
        BOOL fn_80043B50();                         // 80043B50 104: hand over the prize (requestItemAct), EC = fn_80043C04
        int fn_80043C04(msgInfo_s *info);           // 80043C04 EC: 0x24
        // ---- d_npc_talk_countdown (.text 80043C34..80043EBC) ----
        int fn_80043C34(msgInfo_s *info);           // 80043C34 EC: countdown start, {fn_80043C34, fn_80043E2C, null} at nml 804A0E48+0x6C; state 0..6 by the time, code = state + 1 (getMsgLabel kind 3); 0 when not in the event
        void fn_80043E2C(int arg);                  // 80043E2C F8: talk count 0x44, msgMemory set
        // ---- d_npc_talk_halloween (.text 80043EBC..80045154) ----
        int fn_80043EBC(msgInfo_s *info);           // 80043EBC EC: "Ev_Halloween" 0x15, F8 = fn_80043F48, 104 = fn_80043FDC
        void fn_80043F48();                         // 80043F48 F8: msgMemory set
        BOOL fn_80043FDC();                         // 80043FDC 104: two choices (fn_8004409C / fn_80044B48)
        void fn_8004409C();                         // 8004409C choice: select candy (reqSelectItem), 128 = fn_80044120
        void fn_80044120();                         // 80044120 128: take the selected pocket item; 134 by its kind
        void fn_800442A8();                         // 800442A8 134: EC = fn_80044304, close the message
        int fn_80044304(msgInfo_s *info);           // 80044304 EC: 0x17, 104 = fn_80044368
        BOOL fn_80044368();                         // 80044368 104: requestHandAct10(2), EC = fn_800443EC
        int fn_800443EC(msgInfo_s *info);           // 800443EC EC: 0x18/0x19 by the player's costume (0x1A without memory)
        void fn_80044480();                         // 80044480 134: EC = fn_800444DC, close the message
        int fn_800444DC(msgInfo_s *info);           // 800444DC EC: 0x1B, 104 = fn_80044540
        BOOL fn_80044540();                         // 80044540 104: requestHandActD, 128 = fn_80044A00, EC = fn_80044A04
        static BOOL fn_800445DC(const dItem::Item *item, int arg); // 800445DC pocket filter for countPocketsFlag
        static int fn_80044640(u16 *slotMask, dPrivateData_c *player); // 80044640 count pockets matching fn_800445DC
        BOOL fn_800446C4();                         // 800446C4 trick: replace a random matching pocket item with 0x8E1 (this unused)
        static BOOL fn_8004478C(const dItem::Item *a, const dItem::Item *b, const dItem::Item *c); // 8004478C fn_800F8948(a, b) || fn_800F8948(c, b)
        static BOOL fn_800447EC(const dEquip_c *equip); // 800447EC fn_8004478C(&mHat, &mShirt, &mAcc): wearing a costume
        void fn_80044800();                         // 80044800 trick: change the player's clothes (items 0x56E / 0x4A4 / 0x4A5), requestEquip
        void fn_80044A00();                         // 80044A00 128: fn_80044800()
        int fn_80044A04(msgInfo_s *info);           // 80044A04 EC: 0x1C
        int fn_80044A34(msgInfo_s *info);           // 80044A34 EC: 0x1D, 104 = fn_80044A98
        BOOL fn_80044A98();                         // 80044A98 104: EC = fn_80044B18, fn_80044800
        int fn_80044B18(msgInfo_s *info);           // 80044B18 EC: 0x1E
        void fn_80044B48();                         // 80044B48 choice: EC = fn_80044A34
        int fn_80044B9C(msgInfo_s *info);           // 80044B9C EC: 0x1F/0x20 by the costume, F8 = fn_80044C50 (entry in nml table)
        void fn_80044C50();                         // 80044C50 F8: msgMemory set
        int fn_80044CC8(msgInfo_s *info);           // 80044CC8 EC: Halloween start (EVENT_HALLOWEEN ongoing, npc in the event), {fn_80044CC8, fn_80044F2C, null} at nml 804A0E48+0x48; 104 = fn_80044FBC or fn_80045038
        void fn_80044F2C(int arg);                  // 80044F2C F8 of fn_80044CC8: msgMemory set, talk count 0x44
        BOOL fn_80044FBC();                         // 80044FBC 104: dAnimalMemory_c event flag 0, fn_800F04E8
        BOOL fn_80045038();                         // 80045038 104: present (fn_800C60B4) into an empty pocket on code 3
        // ---- d_npc_talk_harvest (.text 80045154..8004664C) ----
        // Returns the ingredient choice (fn_80045544) when the festival is on for this npc and the player
        // has item 0x44, else null; *code = 0x13, *flag = 1 then. Returned through the hidden struct pointer
        // (r3, `this` in r4). The member-pointer type is provisional.
        hookFunc fn_80045154(u16 *code, u8 *flag, BOOL checkEvent); // 80045154
        int fn_800452B0(msgInfo_s *info);           // 800452B0 EC: festival request (getMsgLabel(0xA, 0) label, code 0); triple {fn_800452B0, fn_80045390, fn_80045420} at nml 804A0E48+0x168
        void fn_80045390(int arg);                  // 80045390 F8: talk count 0x44, msgMemory set
        BOOL fn_80045420(int kind);                 // 80045420 104: two choices (fn_80045154's / fn_8004660C)
        void fn_80045544();                         // 80045544 choice: select item 0x44 (reqSelectItem), 128 = fn_800455D0
        void fn_800455D0();                         // 800455D0 128: take the selected pocket item, 134 = fn_8004573C
        int fn_800456E0(msgInfo_s *info);           // 800456E0 EC: item select cancelled (getMsgLabel(0xA, 1) label, probably "Q_Cancel"), code 0
        void fn_8004573C();                         // 8004573C 134: EC = fn_80045798, close the message
        int fn_80045798(msgInfo_s *info);           // 80045798 EC: getMsgLabel(0xA, 2) label, code 1; 104 = fn_8004581C
        BOOL fn_8004581C();                         // 8004581C 104: requestHandActD, 104 = fn_80045874
        BOOL fn_80045874();                         // 80045874 104: EC = fn_800458EC
        int fn_800458EC(msgInfo_s *info);           // 800458EC EC: code 7, 104 = fn_80045970
        BOOL fn_80045970();                         // 80045970 104: EC = fn_800464F8, dAnimalEventState_c flag 1, fn_800F0108

        // Location checks of the table 8046C708, called by fn_800464F8 with the npc's position as field
        // block / unit (dFdBase_c::posToBlockUnit; argument names inferred). TRUE and *code set on a match.
        BOOL fn_80045A20(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045A20 0xB: at its own house
        BOOL fn_80045B38(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045B38 0xC: at the current player's house
        BOOL fn_80045C00(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045C00 0xD: at another player's house (name word 1)
        BOOL fn_80045D8C(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045D8C 0xE: at another villager's house (name word 2)
        BOOL fn_80045F14(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045F14 0xF..0x13: block flag of 8046C6F0
        BOOL fn_80045FC8(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80045FC8 0x14/0x15/0x20: block flags 0x100000 / 0x200
        BOOL fn_80046098(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 80046098 0x16: at an empty player house
        BOOL fn_8004617C(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 8004617C 0x17: empty build site in the block
        BOOL fn_8004627C(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 8004627C 0x18: block flag 0x10
        BOOL fn_800462F8(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 800462F8 0x19..0x1B: relative to the block with flag 0x800
        BOOL fn_800463F0(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 800463F0 0x1C/0x1D: edge blocks of the map
        BOOL fn_800464A8(u16 *code, int blockX, int blockZ, int unitX, int unitZ); // 800464A8 0x1E/0x1F: EVENT_HARVEST_FESTIVAL ongoing or not

        int fn_800464F8(msgInfo_s *info);           // 800464F8 EC: location remark (code 0x1F when no check matches)
        void fn_8004660C();                         // 8004660C choice "no": back to the triple at nml 804A0E48+0x21C {fn_8003EC4C, fn_8003EDEC, null}, startMsg
        // ---- d_npc_talk_bug (.text 8004664C..80046BD0) ----
        // State 0..5, 6 = no Bug-Off talk: 0 npc's event flag 0 not set yet, 1 / 2 this npc leads (getEntry
        // object flag 0x4000 clear / set), 5 another villager leads, 3 the current player leads, 4 another
        // player leads.
        int fn_8004664C();                          // 8004664C Bug-Off state (EVENT_BUG_OFF ongoing, scene attr 5, npc in the event)
        const procSet_s *fn_800468B4();             // 800468B4 {fn_800468EC, fn_80046B18, null} at nml 804A0E48+0x1B0 while fn_8004664C() < 6, else NULL (table 8046BDBC)
        int fn_800468EC(msgInfo_s *info);           // 800468EC EC: code by state (getMsgLabel kind 0xC), names the bug / score / leader, may set getEntry object flag 0x4000
        void fn_80046B18(int arg);                  // 80046B18 F8: dAnimalMemory_c event flag 0, fn_800F04E8, talk count 0x44, msgMemory set
        // ---- d_npc_talk_fireworks (.text 80046BD0..800470E4) ----
        // State 0..3, 4 = no fireworks talk: 0 not started yet (within 30 min of the start), 1 first hour,
        // 2 middle, 3 last hour before the end (dEvent::getStartTime / getEndTime, dTime_c::isSameOrAfter).
        int fn_80046BD0();                          // 80046BD0 fireworks state (EVENT_FIREWORKS active and not over, scene attr 5, npc in the event)
        const procSet_s *fn_80046F60();             // 80046F60 {fn_80046F98, fn_80047054, null} at nml 804A0E48+0x1D4 while fn_80046BD0() < 4, else NULL (table 8046BDBC)
        int fn_80046F98(msgInfo_s *info);           // 80046F98 EC: code = state + 1 (getMsgLabel kind 0xD)
        void fn_80047054(int arg);                  // 80047054 F8: talk count 0x44, msgMemory set
        // ---- d_npc_talk_fishing (.text 800470E4..800476E0) ----
        // State 0..6, 7 = no tourney talk: 0 / 1 npc's event flag 0 not set yet (no fish / a fish recorded),
        // 2 / 3 this npc leads (getEntry object flag 0x4000 clear / set), 6 another villager leads,
        // 4 the current player leads, 5 another player leads.
        int fn_800470E4();                          // 800470E4 tourney state (EVENT_FISHING_TOURNEY ongoing, scene attr 5, npc in the event)
        const procSet_s *fn_8004736C();             // 8004736C {fn_800473A4, fn_80047628, null} at nml 804A0E48+0x18C while fn_800470E4() < 7, else NULL (table 8046BDBC)
        int fn_800473A4(msgInfo_s *info);           // 800473A4 EC: code = state + 1 (getMsgLabel kind 0xB), names the fish / leader / size (setDecimal), may set getEntry object flag 0x4000
        void fn_80047628(int arg);                  // 80047628 F8: dAnimalMemory_c event flag 0, fn_800F04E8, talk count 0x44, msgMemory set
        // ---- d_npc_talk_fmarket (.text 800476E0..8004902C) ----
        // EC (record 21): "Ev_FmarketNPC*" (label table 804A2D00) by the npc's boxed furniture count, the
        // memory of the player (talk_c+0x17C) and npc timer flags 0x4000 / 0x2000; land name 1; code random
        // or fixed; 104 = fn_80047A6C for case 6; setTopic(5, 0, 0).
        int fn_800476E0(msgInfo_s *info);           // 800476E0
        void fn_800479C4(int arg);                  // 800479C4 F8 (records 21, 22): msgMemory set, npc timer flag 0x4000, talk count 0x44
        BOOL fn_80047A6C();                         // 80047A6C 104: one choice (fn_80047B00) when the controller is idle
        void fn_80047B00();                         // 80047B00 choice: npc timer flag 0x2000, fn_8019C34C
        // EC (record 22): picks a boxed furniture (getRoomFtrIdx on talk_c+0x1F8 / +0x1FC), price
        // fn_8005BCCC (d_npc_talk_quest_q09) into talk_c+0x28C; "Ev_FmarketNPC3" when the player has a free
        // pocket and the money (104 = fn_80047CD4), else "Ev_FmarketNPC4"; item name 0, number 3.
        int fn_80047B38(msgInfo_s *info);           // 80047B38
        BOOL fn_80047CD4();                         // 80047CD4 104: two choices (fn_80047DAC code 0x42, fn_8004805C code 0x45)
        void fn_80047DAC();                         // 80047DAC choice: EC = fn_80047E00
        int fn_80047E00(msgInfo_s *info);           // 80047E00 EC: "Q09_TradeYes"; F8 = fn_80047E8C, 104 = fn_80047FA0; returns 0
        void fn_80047E8C();                         // 80047E8C F8: the sale: fn_800A8F98 -> talk_c+0x200, npc virtual +0xE0, payMoney / pickUp; 110 = fn_80047FE4
        BOOL fn_80047FA0();                         // 80047FA0 104: fn_8019C35C when the controller is idle
        void fn_80047FE4();                         // 80047FE4 110: waits for fn_800A9354(talk_c+0x200), then clears 110
        void fn_8004805C();                         // 8004805C choice: EC = fn_800480B0
        int fn_800480B0(msgInfo_s *info);           // 800480B0 EC: "Q09_TradeNo"; returns 0
        int fn_800480E0(msgInfo_s *info);           // 800480E0 EC (record 23): "Q08_Call"; F8 = fn_8004816C, 104 = fn_800481FC
        void fn_8004816C();                         // 8004816C F8: msgMemory set, talk count 0x44
        BOOL fn_800481FC();                         // 800481FC 104: EC = fn_80048438, setRequest1(0), 128 = fn_8004829C
        void fn_8004829C();                         // 8004829C 128: 110 = fn_800482DC
        void fn_800482DC();                         // 800482DC 110: npc requestWalk to l_walkTargetPos; 110 = fn_8004838C
        void fn_8004838C();                         // 8004838C 110: at the goal: requestWait, camera fn_8018B5B8, reqMsgClose, clear 110, memory event flag 0
        int fn_80048438(msgInfo_s *info);           // 80048438 EC: "Ev_FmarketPC" code 1
        int fn_80048468(msgInfo_s *info);           // 80048468 EC (record 24): "Q08_Wait"; F8 = fn_8004816C, 104 = fn_800484F4
        BOOL fn_800484F4();                         // 800484F4 104: EC = fn_8004856C when the controller is idle
        int fn_8004856C(msgInfo_s *info);           // 8004856C EC: "Ev_FmarketPC" code 2; F8 = fn_800485D0
        void fn_800485D0();                         // 800485D0 F8: memory (dAnimalMemory_c at talk_c+0x17C) event flag 1
        int fn_800485E8(msgInfo_s *info);           // 800485E8 EC (record 25): "Q08_Back"; F8 = fn_8004816C, 104 = fn_80048674
        BOOL fn_80048674();                         // 80048674 104: EC = fn_8004856C when the controller is idle
        int fn_800486EC(msgInfo_s *info);           // 800486EC EC (record 26): reloads record 26, "Ev_FmarketPC" code 3
        void fn_8004874C(int arg);                  // 8004874C F8 (records 26, 27): msgMemory set, talk count 0x44
        int fn_800487DC(msgInfo_s *info);           // 800487DC EC (record 27): item of the player's stall (talk_c+0x280 / +0x200); "Ev_FmarketPC" code 4, else fn_800486EC
        BOOL fn_800488B0(int kind);                 // 800488B0 104 (record 27): two choices (fn_80048988 code 0x6D, fn_80048E44 code 0x6E)
        void fn_80048988();                         // 80048988 choice: reqMenu21(1) (price input), 128 = fn_800489D4
        // 128: the entered price (recept_c::fn_80029B20) to talk_c+0x28C, number 0; EC = fn_80048B0C when
        // it is <= fn_8005BD4C (d_npc_talk_quest_q09) of the item, fn_80048EF0 when higher, fn_80048E98 when
        // cancelled (table 804A2E6C); reqMsgClose.
        void fn_800489D4();                         // 800489D4
        int fn_80048B0C(msgInfo_s *info);           // 80048B0C EC: "Ev_FmarketPC" code 6 (price accepted); 104 = fn_80048B70
        // 104: the purchase: the player has room for the money (getMoneyRoom), fn_800A8F98, npc virtual
        // +0xE0, addMoney, dAnimal_c::addNewItem + fn_800F0E9C, npc timer byte +0x289 += 1; 110 =
        // fn_80048D3C; otherwise EC = fn_80048E14.
        BOOL fn_80048B70();                         // 80048B70
        void fn_80048D3C();                         // 80048D3C 110: waits for fn_800A9354(talk_c+0x200), then clears 110, EC = fn_80048DE4
        int fn_80048DE4(msgInfo_s *info);           // 80048DE4 EC: "Ev_FmarketPC" code 9
        int fn_80048E14(msgInfo_s *info);           // 80048E14 EC: "Ev_FmarketPC" code 8 (no room for the money)
        void fn_80048E44();                         // 80048E44 choice: EC = fn_80048E98
        int fn_80048E98(msgInfo_s *info);           // 80048E98 EC: "Ev_FmarketPC" code 5 (refused), npc timer byte +0x28A += 1
        int fn_80048EF0(msgInfo_s *info);           // 80048EF0 EC: "Ev_FmarketPC" code 7 (price too high); 104 = fn_80048F54
        BOOL fn_80048F54();                         // 80048F54 104: two choices (fn_80048988 code 0x6F, fn_80048E44 code 0x70)
        // ---- d_npc_talk_birthday (.text 8004902C..80049FDC) ----
        // Talk procedures. Triples in nml 804A0784: +0x3F0 {fn_8004902C, fn_80049128, fn_800491B8},
        // +0x414 {fn_800492B8, fn_80049344, null}.
        int fn_8004902C(msgInfo_s *info);           // 8004902C EC: player's birthday; code 0xE / 0xB for a Feb 29 birthday, 4 / 1 by friendship (>= 0x40)
        void fn_80049128(int arg);                  // 80049128 F8: msgMemory set, talk count 0x44
        BOOL fn_800491B8(int kind);                 // 800491B8 104: present item 0x8E0 to talk_c+0x280, pickUp, requestItemAct; EC = fn_8004926C
        int fn_8004926C(msgInfo_s *info);           // 8004926C EC: code = mMessageCode + 1; player byte +0x5604 = -1
        int fn_800492B8(msgInfo_s *info);           // 800492B8 EC: npc's birthday; code 7, 8 when already congratulated (flag 0x1000 at npc+0x1F98 object +0x28C)
        void fn_80049344(int arg);                  // 80049344 F8: msgMemory set, sets that 0x1000 flag, talk count 0x44
        // ---- d_npc_talk_quest_q04 (.text 80049FDC..8004B75C) ----
        int fn_80049FDC(msgInfo_s *info);           // 80049FDC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 5, kind 3, fn_8004B5A8)
        int fn_8004A024(msgInfo_s *info);           // 8004A024 EC: "Q04_Req" code (mMatchMode == 0 ? 4 : 1) + rndF(3); 104 = nml stepRequestChoice
        int fn_8004A0D8(msgInfo_s *info);           // 8004A0D8 talk check while the quest runs (nml .rodata 8046BF30): state 7 won by
        //          another player, 6 past deadline, 8 has a shirt (countPocketsKind(4)), 14 not
        //          in the quest (startQuestOffer offer: accept fn_8004B5A8); label
        //          getMsgLabel(6, id 5, state), 104 = descriptor proc by state, names,
        //          setQ4Word (look name of the request)
        BOOL fn_8004A5F8();                         // 8004A5F8 104 (state 8): choices fn_80060980 / fn_80045154 procs, give
        //          (fn_8004A7CC, 0x40) and no (nml selResumeTalk, 4)
        void fn_8004A7CC();                         // 8004A7CC choice: item select (reqSelectItem, countPocketsKind(4) mask), 128 = fn_8004A858
        void fn_8004A858();                         // 8004A858 128: selected pocket item to talk_c+0x280 / +0x282 (name word 7),
        //          requestItemActEx; isClothRequestMatch: pocket cleared, 134 = fn_8004A9E8,
        //          else 134 = fn_8004AF30 (EC = nml msgCancel if cancelled)
        void fn_8004A9E8();                         // 8004A9E8 134: EC = fn_8004AA44, close the message
        int fn_8004AA44(msgInfo_s *info);           // 8004AA44 EC: "Q04_Win" code 0; F8 = fn_8004AACC, 104 = fn_8004AC68
        void fn_8004AACC();                         // 8004AACC F8: reward (pickClothReward) to talk_c+0x280: pick up item or add bells
        BOOL fn_8004AC68();                         // 8004AC68 104: quest state 1, requester = player, wish +5; EC = fn_8004ADF4; the npc
        //          wears the shirt (setCloth(talk_c+0x282), fn_800F1A68, npc onDaubClothChange /
        //          offDaubClothChanged), requestHandActC, 128 = fn_8004ADBC
        void fn_8004ADBC();                         // 8004ADBC 128: npc (talk_c+0x68) offDaubClothChange / onDaubClothChanged
        int fn_8004ADF4(msgInfo_s *info);           // 8004ADF4 EC: "Q04_Win" code 4; 104 = fn_8004AE54
        BOOL fn_8004AE54();                         // 8004AE54 104: EC = fn_8004AF04, requestItemAct of the reward, sound 0x171F
        int fn_8004AF04(msgInfo_s *info);           // 8004AF04 EC: "Q04_Win" code 8
        void fn_8004AF30();                         // 8004AF30 134 (no match): EC = fn_8004AF8C, close the message
        int fn_8004AF8C(msgInfo_s *info);           // 8004AF8C EC: "Q04_NG" code 1..4 by mMatchMode and dItem::Item::isSame with
        //          dAnimal_c+0x2FFA; word 6 = setLookName (dItem::nameLook_c); 104 = fn_8004B0D8
        BOOL fn_8004B0D8();                         // 8004B0D8 104: requestHandActE (hand the shirt back)
        BOOL fn_8004B11C();                         // 8004B11C 104 (states 6/7): remove the player from the quest, clear it when empty
        BOOL fn_8004B1B0();                         // 8004B1B0 104 (other states): choices fn_80060980 / fn_80045154 procs, fn_8004B384 (1)
        //          and no (nml selResumeTalk, 4)
        void fn_8004B384();                         // 8004B384 choice: EC = fn_8004B3D8
        int fn_8004B3D8(msgInfo_s *info);           // 8004B3D8 EC: "Q04_ConA" (mMatchMode != 0) / "Q04_ConB" code 0; 104 = fn_8004B47C
        BOOL fn_8004B47C();                         // 8004B47C 104: EC = fn_8004B4F4
        int fn_8004B4F4(msgInfo_s *info);           // 8004B4F4 EC: "Q04_ConA" / "Q04_ConB" code 7 + min(players - 1, 2)
        BOOL fn_8004B5A8();                         // 8004B5A8 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names, setQ4Word; EC = fn_8004A024
        // ---- d_npc_talk_quest_q02 (.text 8004B75C..8004CBCC) ----
        int fn_8004B75C(msgInfo_s *info);           // 8004B75C offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 3, kind 1, fn_8004CA18)
        int fn_8004B7A4(msgInfo_s *info);           // 8004B7A4 EC: "Q02_Req" code 0; 104 = nml stepRequestChoice
        int fn_8004B804(msgInfo_s *info);           // 8004B804 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching item (countPocketsItem),
        //          6 not in the quest (startQuestOffer offer: accept fn_8004CA18); label
        //          getMsgLabel(6, id 3, state), 104 = descriptor proc by state, names
        BOOL fn_8004BD14();                         // 8004BD14 104 (state 5): choices fn_80060980 / fn_80045154 procs, give
        //          (fn_8004BEE8, 0x3E) and no (nml selResumeTalk, 4)
        void fn_8004BEE8();                         // 8004BEE8 choice: item select (reqSelectItem, countPocketsItem mask), 128 = fn_8004BF98
        void fn_8004BF98();                         // 8004BF98 128: take the selected pocket item to talk_c+0x286, requestItemActEx,
        //          134 = fn_8004C0C8 (EC = nml msgCancel if cancelled)
        void fn_8004C0C8();                         // 8004C0C8 134: EC = fn_8004C124, close the message
        int fn_8004C124(msgInfo_s *info);           // 8004C124 EC: "Q02_Win" code 0; F8 = fn_8004C1AC, 104 = fn_8004C350
        void fn_8004C1AC();                         // 8004C1AC F8: reward (pickInsectFishReward) to talk_c+0x280: pick up item or add bells
        BOOL fn_8004C350();                         // 8004C350 104: quest state 1, requester = player, wish +5, the npc keeps the
        //          fish (addNewItem); EC = fn_8004C4A8, requestHandActD
        int fn_8004C4A8(msgInfo_s *info);           // 8004C4A8 EC: "Q02_Win" code 4; 104 = fn_8004C508
        BOOL fn_8004C508();                         // 8004C508 104: EC = fn_8004C5B8, requestItemAct of the reward, sound 0x171F
        int fn_8004C5B8(msgInfo_s *info);           // 8004C5B8 EC: "Q02_Win" code 8
        BOOL fn_8004C5E4();                         // 8004C5E4 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL fn_8004C678();                         // 8004C678 104 (other states): choices fn_80060980 / fn_80045154 procs, fn_8004C84C (1)
        //          and no (nml selResumeTalk, 4)
        void fn_8004C84C();                         // 8004C84C choice: EC = fn_8004C8A0
        int fn_8004C8A0(msgInfo_s *info);           // 8004C8A0 EC: "Q02_Con" code 0; 104 = fn_8004C900
        BOOL fn_8004C900();                         // 8004C900 104: EC = fn_8004C978
        int fn_8004C978(msgInfo_s *info);           // 8004C978 EC: "Q02_Con" code 7 + min(players - 1, 2)
        BOOL fn_8004CA18();                         // 8004CA18 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = fn_8004B7A4
        // ---- d_npc_talk_quest_q03 (.text 8004CBCC..8004E31C) ----
        int fn_8004CBCC(msgInfo_s *info);           // 8004CBCC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 4, kind 2, fn_8004E150)
        int fn_8004CC14(msgInfo_s *info);           // 8004CC14 EC: "Q03_Req" code (mMatchMode == 0 ? 4 : 1) + rndF(3); 104 = nml stepRequestChoice
        int fn_8004CCC8(msgInfo_s *info);           // 8004CCC8 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching fossil (countPocketsFossil),
        //          6 not in the quest (startQuestOffer offer: accept fn_8004E150), 8 / 9 the
        //          winner (quest state 1 / other); label getMsgLabel(6, id 4, state),
        //          104 = descriptor proc by state, names
        BOOL fn_8004D2D0();                         // 8004D2D0 104 (state 5): choices fn_80060980 / fn_80045154 procs, give
        //          (fn_8004D4A4, 0x3F) and no (nml selResumeTalk, 4)
        void fn_8004D4A4();                         // 8004D4A4 choice: item select (reqSelectItem, countPocketsFossil mask), 128 = fn_8004D548
        void fn_8004D548();                         // 8004D548 128: take the selected pocket item to talk_c+0x286 (name word 6),
        //          requestItemActEx, 134 = fn_8004D688 (EC = nml msgCancel if cancelled)
        void fn_8004D688();                         // 8004D688 134: EC = fn_8004D6E4, close the message
        int fn_8004D6E4(msgInfo_s *info);           // 8004D6E4 EC: "Q03_Win" code 0; F8 = fn_8004D76C, 104 = fn_8004D910
        void fn_8004D76C();                         // 8004D76C F8: reward (pickFossilReward) to talk_c+0x280: pick up item or add bells
        BOOL fn_8004D910();                         // 8004D910 104: quest state 1, fn_800F12D8(idx, 2, 1), requester = player, wish +5,
        //          dAnimal_c::setQuestStarted; the npc keeps the fossil (addNewItem,
        //          dAnimalMemory_c::setPresent on talk_c+0x17C / +0x180); EC = fn_8004DAB8,
        //          requestHandActD
        int fn_8004DAB8(msgInfo_s *info);           // 8004DAB8 EC: "Q03_Win" code 4; 104 = fn_8004DB18
        BOOL fn_8004DB18();                         // 8004DB18 104: EC = fn_8004DBC8, requestItemAct of the reward, sound 0x171F
        int fn_8004DBC8(msgInfo_s *info);           // 8004DBC8 EC: "Q03_Win" code 8
        BOOL fn_8004DBF4();                         // 8004DBF4 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL fn_8004DC88();                         // 8004DC88 104 (other states): choices fn_80060980 / fn_80045154 procs, fn_8004DE5C (1)
        //          and no (nml selResumeTalk, 4)
        void fn_8004DE5C();                         // 8004DE5C choice: EC = fn_8004DEB0
        int fn_8004DEB0(msgInfo_s *info);           // 8004DEB0 EC: "Q03_Con" code (mMatchMode == 0 and an item set ? 4 : 1) + rndF(3);
        //          104 = fn_8004DF70
        BOOL fn_8004DF70();                         // 8004DF70 104: EC = fn_8004DFE8
        int fn_8004DFE8(msgInfo_s *info);           // 8004DFE8 EC: "Q03_Con" code 7 + min(players - 1, 2)
        BOOL fn_8004E088();                         // 8004E088 104 (state 8, "Q03_Return"): quest state 2, fn_800F12D8(idx, 2, 2)
        BOOL fn_8004E0FC();                         // 8004E0FC 104 (state 9, "Q03_Comp"): quest state 4
        BOOL fn_8004E150();                         // 8004E150 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = fn_8004CC14
        // ---- d_npc_talk_quest_q05 (.text 8004E31C..8004FADC) ----
        int fn_8004E31C(msgInfo_s *info);           // 8004E31C offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 6, kind 4, fn_8004F930)
        int fn_8004E364(msgInfo_s *info);           // 8004E364 EC: "Q05_Req" code 0; 104 = nml stepRequestChoice
        int fn_8004E3C4(msgInfo_s *info);           // 8004E3C4 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has furniture (countPocketsKind(3)), 9 not
        //          in the quest (startQuestOffer offer: accept fn_8004F930), 11 / 12 the winner
        //          (quest state 1 / other); label getMsgLabel(6, id 6, state), 104 =
        //          descriptor proc by state, names, setQ5Word (part name of the request)
        BOOL fn_8004E990();                         // 8004E990 104 (state 5): choices fn_80060980 / fn_80045154 procs, give
        //          (fn_8004EB64, 0x41) and no (nml selResumeTalk, 4)
        void fn_8004EB64();                         // 8004EB64 choice: item select (reqSelectItem, countPocketsKind(3) mask), 128 = fn_8004EBF0
        void fn_8004EBF0();                         // 8004EBF0 128: selected pocket item to talk_c+0x280 (name word 6), requestItemActEx;
        //          checkFtrRequest == 0: pocket cleared, item to talk_c+0x286, 134 =
        //          fn_8004ED80, else 134 = fn_8004F2E4 (EC = nml msgCancel if cancelled)
        void fn_8004ED80();                         // 8004ED80 134: EC = fn_8004EDDC, close the message
        int fn_8004EDDC(msgInfo_s *info);           // 8004EDDC EC: "Q05_Win" code 0; F8 = fn_8004EE64, 104 = fn_8004F000
        void fn_8004EE64();                         // 8004EE64 F8: reward (pickFtrReward) to talk_c+0x280: pick up item or add bells
        BOOL fn_8004F000();                         // 8004F000 104: quest state 1, fn_800F12D8(idx, 4, 1), requester = player, wish +5,
        //          dAnimal_c::setQuestStarted; the npc keeps the furniture (addNewItem,
        //          dAnimalMemory_c::setPresent on talk_c+0x17C / +0x180); EC = fn_8004F1A8,
        //          requestHandActD
        int fn_8004F1A8(msgInfo_s *info);           // 8004F1A8 EC: "Q05_Win" code 4; 104 = fn_8004F208
        BOOL fn_8004F208();                         // 8004F208 104: EC = fn_8004F2B8, requestItemAct of the reward, sound 0x171F
        int fn_8004F2B8(msgInfo_s *info);           // 8004F2B8 EC: "Q05_Win" code 8
        void fn_8004F2E4();                         // 8004F2E4 134 (no match): EC = fn_8004F340, close the message
        int fn_8004F340(msgInfo_s *info);           // 8004F340 EC: "Q05_NG" code 1..3 by checkFtrRequest; 104 = fn_8004F400
        BOOL fn_8004F400();                         // 8004F400 104: requestHandActE (hand the item back)
        BOOL fn_8004F444();                         // 8004F444 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL fn_8004F4D8();                         // 8004F4D8 104 (other states): choices fn_80060980 / fn_80045154 procs, fn_8004F6AC (1)
        //          and no (nml selResumeTalk, 4)
        void fn_8004F6AC();                         // 8004F6AC choice: EC = fn_8004F700
        int fn_8004F700(msgInfo_s *info);           // 8004F700 EC: "Q05_Con" code 0; 104 = fn_8004F760
        BOOL fn_8004F760();                         // 8004F760 104: EC = fn_8004F7D8
        int fn_8004F7D8(msgInfo_s *info);           // 8004F7D8 EC: "Q05_Con" code 7 + min(players - 1, 2)
        BOOL fn_8004F868();                         // 8004F868 104 (state 11, "Q05_Return"): quest state 2, fn_800F12D8(idx, 4, 2)
        BOOL fn_8004F8DC();                         // 8004F8DC 104 (state 12, "Q05_Comp"): quest state 4
        BOOL fn_8004F930();                         // 8004F930 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names, setQ5Word; EC = fn_8004E364
        // ---- d_npc_talk_quest_q01 (.text 8004FADC..80050F4C) ----
        int fn_8004FADC(msgInfo_s *info);           // 8004FADC offer check (nml .rodata 8046BFD8): startRequestOffer(info, id 2, kind 0, fn_80050D98)
        int fn_8004FB24(msgInfo_s *info);           // 8004FB24 EC: "Q01_Req" code 0; 104 = nml stepRequestChoice
        int fn_8004FB84(msgInfo_s *info);           // 8004FB84 talk check while the quest runs (nml .rodata 8046BF30): state 4 won by
        //          another player, 3 past deadline, 5 has a matching item (countPocketsItem),
        //          6 not in the quest (startQuestOffer offer: accept fn_80050D98); label
        //          getMsgLabel(6, id 2, state), 104 = descriptor proc by state, names
        BOOL fn_80050094();                         // 80050094 104 (state 5): choices fn_80060980 / fn_80045154 procs, give
        //          (fn_80050268, 0x3D) and no (nml selResumeTalk, 4)
        void fn_80050268();                         // 80050268 choice: item select (reqSelectItem, countPocketsItem mask), 128 = fn_80050318
        void fn_80050318();                         // 80050318 128: take the selected pocket item to talk_c+0x286, requestItemActEx,
        //          134 = fn_80050448 (EC = nml msgCancel if cancelled)
        void fn_80050448();                         // 80050448 134: EC = fn_800504A4, close the message
        int fn_800504A4(msgInfo_s *info);           // 800504A4 EC: "Q01_Win" code 0; F8 = fn_8005052C, 104 = fn_800506D0
        void fn_8005052C();                         // 8005052C F8: reward (pickInsectFishReward) to talk_c+0x280: pick up item or add bells
        BOOL fn_800506D0();                         // 800506D0 104: quest state 1, requester = player, wish +5, the npc keeps the
        //          insect (addNewItem); EC = fn_80050828, requestHandActD
        int fn_80050828(msgInfo_s *info);           // 80050828 EC: "Q01_Win" code 4; 104 = fn_80050888
        BOOL fn_80050888();                         // 80050888 104: EC = fn_80050938, requestItemAct of the reward, sound 0x171F
        int fn_80050938(msgInfo_s *info);           // 80050938 EC: "Q01_Win" code 8
        BOOL fn_80050964();                         // 80050964 104 (states 3/4): remove the player from the quest, clear it when empty
        BOOL fn_800509F8();                         // 800509F8 104 (other states): choices fn_80060980 / fn_80045154 procs, fn_80050BCC (1)
        //          and no (nml selResumeTalk, 4)
        void fn_80050BCC();                         // 80050BCC choice: EC = fn_80050C20
        int fn_80050C20(msgInfo_s *info);           // 80050C20 EC: "Q01_Con" code 0; 104 = fn_80050C80
        BOOL fn_80050C80();                         // 80050C80 104: EC = fn_80050CF8
        int fn_80050CF8(msgInfo_s *info);           // 80050CF8 EC: "Q01_Con" code 7 + min(players - 1, 2)
        BOOL fn_80050D98();                         // 80050D98 104 (accept): start the quest (pickRequestItem,
        //          dQuestVillager_c::start) or join it; names; EC = fn_8004FB24
        // ---- d_npc_talk_quest_q12 (.text 80050F4C..80051C68) ----
        int fn_80050F4C(msgInfo_s *info);           // 80050F4C quest check (nml table 8046BF30 entry 7): kind 6 active, label getMsgLabel(6, 7, idx); 104 from 804A392C
        BOOL fn_8005125C();                         // 8005125C 104: choice 0 = fn_800512F0 (2 entries, cancel 1)
        void fn_800512F0();                         // 800512F0 choice: reqSelectItem(key pockets, 0x22, 0x78, 1), 128 = fn_80051380
        void fn_80051380();                         // 80051380 128: selected pocket == quest item -> remove it, 134 = fn_800514E0 (else fn_80051A04), requestItemActEx(7); cancel: EC = fn_80051B08
        void fn_800514E0();                         // 800514E0 134: EC = fn_8005153C, close the message
        int fn_8005153C(msgInfo_s *info);           // 8005153C EC "Q12_KeyOK", 104 = fn_800515A0
        BOOL fn_800515A0();                         // 800515A0 104: EC = fn_8005162C, requestHandActD, friendship +5 (nml addFriendship)
        int fn_8005162C(msgInfo_s *info);           // 8005162C EC "Q12_Item", F8 = fn_800516B8, 104 = fn_80051810
        void fn_800516B8();                         // 800516B8 F8: reward (dAnimal_c::pickLostItemReward) to talk_c+0x280 and pickUp, or bells (word 2)
        BOOL fn_80051810();                         // 80051810 104: quest state 2, requester, removePlayer, clear when empty; EC = fn_800519D4, requestItemAct(+0x280)
        int fn_800519D4(msgInfo_s *info);           // 800519D4 EC "Q12_Item" code 4
        void fn_80051A04();                         // 80051A04 134: EC = fn_80051A60, close the message
        int fn_80051A60(msgInfo_s *info);           // 80051A60 EC "Q12_KeyNG", 104 = fn_80051AC4
        BOOL fn_80051AC4();                         // 80051AC4 104: requestHandActE
        int fn_80051B08(msgInfo_s *info);           // 80051B08 EC "Q12_Cancel"
        BOOL fn_80051B38();                         // 80051B38 104 (804A392C): removePlayer; clear the quest + dLostQuest_c::setTime when empty
        BOOL fn_80051BF0();                         // 80051BF0 104 (804A392C): dQuestVillager_c::addPlayer(player, 1)
        // ---- d_npc_talk_quest_q11 (.text 80051C68..80052EB0) ----
        int fn_80051C68(msgInfo_s *info);           // 80051C68 quest check (nml table 8046BF30 entry 8): kind 5 active, label getMsgLabel(6, 8, idx); F8/104 from 804A3A68
        BOOL fn_80052190();                         // 80052190 104: two choices (msg 0x48 -> fn_80052268, 0x49 -> fn_8005289C)
        void fn_80052268();                         // 80052268 choice: EC = fn_800522BC
        int fn_800522BC(msgInfo_s *info);           // 800522BC EC "Q11_Medicine", 104 = fn_8005237C
        BOOL fn_8005237C();                         // 8005237C 104: reqSelectItem(medicine pockets, 0x22, 1), 128 = fn_80052440
        void fn_80052440();                         // 80052440 128: take the selected item, 134 = fn_80052578, requestItemActEx(9), friendship +7; cancel: EC = fn_8005274C
        void fn_80052578();                         // 80052578 134: EC = fn_800525D4, close the message
        int fn_800525D4(msgInfo_s *info);           // 800525D4 EC "Q11_Cure", 104 = fn_80052694
        BOOL fn_80052694();                         // 80052694 104: dQuestSick_c::setPlayer, dQuestVillager_c::addPlayer(player, 1)
        int fn_8005274C(msgInfo_s *info);           // 8005274C EC "Q11_Cancel", 104 = fn_80052818, friendship -1
        BOOL fn_80052818();                         // 80052818 104: dQuestVillager_c::addPlayer(player, 1) (also the default 104 of fn_80051C68)
        void fn_8005289C();                         // 8005289C choice: EC = fn_800528F0
        int fn_800528F0(msgInfo_s *info);           // 800528F0 EC "Q11_Talk", 104 = fn_800529B0
        BOOL fn_800529B0();                         // 800529B0 104: dQuestVillager_c::addPlayer(player, 1)
        BOOL fn_80052A34();                         // 80052A34 104 (804A3A68): EC = fn_800528F0
        void fn_80052AAC();                         // 80052AAC F8 (804A3A68): reward (dAnimal_c::pickSickReward) to talk_c+0x280, pickUp, item word 4
        BOOL fn_80052B48();                         // 80052B48 104 (804A3A68): removePlayer, clear when empty; EC = fn_80052C5C, requestItemAct(+0x280)
        int fn_80052C5C(msgInfo_s *info);           // 80052C5C EC "Q11_End"
        void fn_80052C88();                         // 80052C88 F8 (804A3A68): dAnimal_c::sendSickReward (pockets full); quest state 2 on failure
        BOOL fn_80052D04();                         // 80052D04 104 (804A3A68): removePlayer, clear when state 1 and empty; EC = fn_80052C5C
        BOOL fn_80052E04();                         // 80052E04 104 (804A3A68): removePlayer, clear when state 1 and empty
        // ---- d_npc_talk_quest_delivery (.text 80052EB0..800546F4) ----
        int fn_80052EB0(msgInfo_s *info);           // 80052EB0 EC (nml table 804A0E48): errand of state 0 whose animal 1 is this npc, item in pockets; label getMsgLabel(5, kind == 8, 0), 104 = 804A3BC8[kind == 8]
        void fn_80053224(int arg);                  // 80053224 F8 (shared end-of-talk hook): nml recordTalk / getRememberedMsg, dNpc::msgMemory_c::set, dAnimalTalkCount_c::inc(0x44)
        void fn_800532B4(hookFunc next);            // 800532B4 reqSelectItem(errand item pockets, 0x22, 1), 128 = next (by value: callers pass a stack copy)
        int fn_80053390(msgInfo_s *info);           // 80053390 EC "Q_Cancel"
        void fn_800533C0();                         // 800533C0 choice: EC/F8/104 = getEventProcSet() (default 804A1064), run EC
        BOOL fn_80053414();                         // 80053414 104 (kind 7): common choices (fn_80060980 / fn_80045154), 0xD -> fn_800535E8, 4 -> fn_800533C0
        void fn_800535E8();                         // 800535E8 choice: fn_800532B4(fn_80053628)
        void fn_80053628();                         // 80053628 128: take the selected pocket item, requestItemActEx(7), 134 = fn_80053774 (pocket flag 2) / fn_80053A30; cancel: EC = fn_80053390
        void fn_80053774();                         // 80053774 134: EC = fn_800537D0, close the message
        int fn_800537D0(msgInfo_s *info);           // 800537D0 EC "Q06_Get0", 104 = fn_80053840, friendship +5
        BOOL fn_80053840();                         // 80053840 104: EC = fn_800538C0, requestHandActF
        int fn_800538C0(msgInfo_s *info);           // 800538C0 EC "Q06_Get1", 104 = fn_80053924
        BOOL fn_80053924();                         // 80053924 104: errand state 1 (2 when past the deadline); EC = fn_80053A04, requestHandActD
        int fn_80053A04(msgInfo_s *info);           // 80053A04 EC "Q06_Fin"
        void fn_80053A30();                         // 80053A30 134: EC = fn_80053A8C, close the message
        int fn_80053A8C(msgInfo_s *info);           // 80053A8C EC "Q06_Open", 104 = fn_80053AFC, friendship +3
        BOOL fn_80053AFC();                         // 80053AFC 104: same as fn_80053924
        BOOL fn_80053BDC();                         // 80053BDC 104 (kind 8): common choices, 0xD -> fn_80053DB0, 4 -> fn_800533C0
        void fn_80053DB0();                         // 80053DB0 choice: fn_800532B4(fn_80053DF0)
        void fn_80053DF0();                         // 80053DF0 128: as fn_80053628, item also kept at talk_c+0x282; 134 = fn_80053F44 (pocket flag 2) / fn_800544C0
        void fn_80053F44();                         // 80053F44 134: EC = fn_80053FA0, close the message
        int fn_80053FA0(msgInfo_s *info);           // 80053FA0 EC "Q07_Get0", 104 = fn_800540C8
        int fn_80054004();                          // 80054004 dAnimal_c::getStyleMatch of the errand item (2 when none); sets errand state match + 1 (4 past the deadline)
        BOOL fn_800540C8();                         // 800540C8 104: EC by fn_80054004(): 0 fn_800541D0, 1 fn_8005440C, else fn_8005439C; requestHandActF
        int fn_800541D0(msgInfo_s *info);           // 800541D0 EC "Q07_Get1", 104 = fn_80054240, friendship +5
        BOOL fn_80054240();                         // 80054240 104: EC = fn_80054370, npc puts on talk_c+0x282 (dAnimal_c::setCloth), requestHandActC, 128 = fn_80054338
        void fn_80054338();                         // 80054338 128: nml offDaubClothChange / onDaubClothChanged on the npc (cloth change end)
        int fn_80054370(msgInfo_s *info);           // 80054370 EC "Q07_Fin"
        int fn_8005439C(msgInfo_s *info);           // 8005439C EC "Q07_Get2", 104 = fn_80054240, friendship +5
        int fn_8005440C(msgInfo_s *info);           // 8005440C EC "Q07_Get3", 104 = fn_8005447C, friendship +5
        BOOL fn_8005447C();                         // 8005447C 104: requestHandActD
        void fn_800544C0();                         // 800544C0 134: EC by fn_80054004(): 0 fn_800545A4, 1 fn_80054684, else fn_80054614; close the message
        int fn_800545A4(msgInfo_s *info);           // 800545A4 EC "Q07_Open1", 104 = fn_80054240, friendship +3
        int fn_80054614(msgInfo_s *info);           // 80054614 EC "Q07_Open2", 104 = fn_80054240, friendship +3
        int fn_80054684(msgInfo_s *info);           // 80054684 EC "Q07_Open3", 104 = fn_8005447C, friendship +3
        // ---- d_npc_talk_quest_q07 (.text 800546F4..80056024) ----
        int fn_800546F4(msgInfo_s *info);           // 800546F4: request entry for QUEST_KIND_ERRAND_REQUEST_FINAL (lbl_8046BFD8 [1]): returns startErrandOffer(msg, 8, 0, fn_8005473C)
        BOOL fn_8005473C();                         // 8005473C: request accepted (r7 of startErrandOffer): starts the request talk, only while mpController->_6C84 == 0; sets _0EC=fn_800547B4
        int fn_800547B4(msgInfo_s *info);           // 800547B4: "Q07_Req"; sets _104=fn_80054814
        BOOL fn_80054814();                         // 80054814: yes/no choice; clears the villager quest (getEntry() + 0x23E); sets choice=fn_80054914, choice=selNo
        void fn_80054914();                         // 80054914: choice "yes"; sets _0EC=fn_80054968
        int fn_80054968(msgInfo_s *info);           // 80054968: "Q_Yes"; sets _0F8=fn_800549F4, _104=fn_80054C48
        void fn_800549F4();                         // 800549F4: starts the errand: pickKind (lbl_8046C9D0), recipient, item (fn_800C60B4) into mItems[0], dQuestErrand_c::start(8, ...), deadline words
        BOOL fn_80054C48();                         // 80054C48: hands mItems[0] to the player (requestItemAct), only while mpController->_6C84 == 0; sets _0EC=msgYes
        int fn_80054CEC(msgInfo_s *info);           // 80054CEC: quest-talk entry (lbl_8046BF30 [1]): active errand of kind 8 whose recipient is this villager; label and _104 from l_q07Labels + 0x18 by state
        BOOL fn_800550B0();                         // 800550B0: step for state 3 (errand done): talk menu with the report choice; sets choice=fn_80055284, choice=fn_80055CF8
        void fn_80055284();                         // 80055284: choice "report": errand mState 1 -> Q07_Report + addFriendship(5), else msgTimeover2 + addFriendship(-1)
        int fn_80055348(msgInfo_s *info);           // 80055348: "Q07_Report"; sets _104=fn_800553AC
        BOOL fn_800553AC();                         // 800553AC: 3-way choice (texts 0x33-0x35) -> fn_800554C8 / fn_8005551C / fn_80055570
        void fn_800554C8();                         // 800554C8: choice 0 -> Q07_Good; sets _0EC=fn_800555C4
        void fn_8005551C();                         // 8005551C: choice 1 -> Q07_Normal; sets _0EC=fn_80055634
        void fn_80055570();                         // 80055570: choice 2 -> Q07_Bad; sets _0EC=fn_800556A4
        int fn_800555C4(msgInfo_s *info);           // 800555C4: "Q07_Good"; fn_80055FA0(1); sets _104=fn_80055710
        int fn_80055634(msgInfo_s *info);           // 80055634: "Q07_Normal"; fn_80055FA0(3); sets _104=fn_80055710
        int fn_800556A4(msgInfo_s *info);           // 800556A4: "Q07_Bad"; fn_80055FA0(2); sets _104=fn_80055710
        BOOL fn_80055710();                         // 80055710: pockets full -> Q_ItemFull, else Q_Item; sets _0EC=fn_80055B68, _0EC=fn_800557E8
        int fn_800557E8(msgInfo_s *info);           // 800557E8: "Q_Item"; sets _0F8=fn_80055874, _104=fn_80055A5C
        void fn_80055874();                         // 80055874: final reward: dAnimal_c::pickErrandFinalReward (lbl_807501E8), pickUp or addMoney, setBells
        BOOL fn_80055A5C();                         // 80055A5C: clears the errand, takes the delivered item (requestItemAct); sets _0EC=fn_80055B3C
        int fn_80055B3C(msgInfo_s *info);           // 80055B3C: "Q07_End"
        int fn_80055B68(msgInfo_s *info);           // 80055B68: "Q_ItemFull"; sets _104=fn_80055BCC
        BOOL fn_80055BCC();                         // 80055BCC: dAnimal_c::completeErrandRequestFinal (lbl_807501F8); sets _0EC=fn_80055B3C
        void fn_80055CF8();                         // 80055CF8: choice: generic talk (getEventProcSet, setProcSet(lbl_804A0E48 entry))
        BOOL fn_80055D4C();                         // 80055D4C: step for state 0 (errand open): talk menu with the Q07_Con choice; sets choice=fn_80055F20, choice=fn_80055CF8
        void fn_80055F20();                         // 80055F20: choice "conversation"; sets _0EC=fn_80055F74
        int fn_80055F74(msgInfo_s *info);           // 80055F74: "Q07_Con"
        void fn_80055FA0(int answer);               // 80055FA0: addFriendship(5) if the errand mState == answer, else addFriendship(3)
        // ---- d_npc_talk_quest_q06 (.text 80056024..800573D0) ----
        int fn_80056024(msgInfo_s *info);           // 80056024: request entry for QUEST_KIND_ERRAND_REQUEST (lbl_8046BFD8 [0]): returns startErrandOffer(msg, 7, 0, fn_8005606C)
        BOOL fn_8005606C();                         // 8005606C: request accepted (r7 of startErrandOffer): starts the request talk, only while mpController->_6C84 == 0; sets _0EC=fn_800560E4
        int fn_800560E4(msgInfo_s *info);           // 800560E4: "Q06_Req"; sets _104=fn_80056144
        BOOL fn_80056144();                         // 80056144: yes/no choice (texts 0x15/0x1F); clears the villager quest (getEntry() + 0x23E); sets choice=fn_80056244, choice=selNo
        void fn_80056244();                         // 80056244: choice "yes"; sets _0EC=fn_80056298
        int fn_80056298(msgInfo_s *info);           // 80056298: "Q_Yes"; sets _0F8=fn_80056324, _104=fn_80056534
        void fn_80056324();                         // 80056324: starts the errand: pickKind (lbl_8046CA00), recipient (pickRandomAvailableAnimal), item into mItems[0], dQuestErrand_c::start(7, ...), deadline words
        BOOL fn_80056534();                         // 80056534: hands mItems[0] to the player (requestItemAct), only while mpController->_6C84 == 0; sets _0EC=msgYes
        int fn_800565D8(msgInfo_s *info);           // 800565D8: quest-talk entry (lbl_8046BF30 [0]): active errand of kind 7 whose recipient is this villager; label getMsgLabel(6, 0, state), _104 from l_q06Labels + 0x18 by state
        BOOL fn_80056928();                         // 80056928: step for state 3 (errand done): talk menu (fn_80060980, fn_80045154) with the report choice; sets choice=fn_80056AFC, choice=selResumeTalk
        void fn_80056AFC();                         // 80056AFC: choice "report": errand mState 1 -> Q06_Report + addFriendship(5), else msgTimeover2 + addFriendship(-1)
        int fn_80056BCC(msgInfo_s *info);           // 80056BCC: "Q06_Report"; sets _104=fn_80056C30
        BOOL fn_80056C30();                         // 80056C30: pockets full -> Q_ItemFull, else Q_Item; sets _0EC=fn_80057000, _0EC=fn_80056D14
        int fn_80056D14(msgInfo_s *info);           // 80056D14: "Q_Item"; sets _0F8=fn_80056DA0, _104=fn_80056EF4
        void fn_80056DA0();                         // 80056DA0: reward: dAnimal_c::pickErrandReward, pickUp or addMoney, setBells
        BOOL fn_80056EF4();                         // 80056EF4: clears the errand, takes the delivered item (requestItemAct); sets _0EC=fn_80056FD4
        int fn_80056FD4(msgInfo_s *info);           // 80056FD4: "Q06_End"
        int fn_80057000(msgInfo_s *info);           // 80057000: "Q_ItemFull"; sets _104=fn_80057064
        BOOL fn_80057064();                         // 80057064: dAnimal_c::completeErrandRequest; sets _0EC=fn_80056FD4
        BOOL fn_80057170();                         // 80057170: step for state 0 (errand open): talk menu (fn_80060980, fn_80045154) with the Q06_Con choice; sets choice=fn_80057344, choice=selResumeTalk
        void fn_80057344();                         // 80057344: choice "conversation"; sets _0EC=fn_80057398
        int fn_80057398(msgInfo_s *info);           // 80057398: "Q06_Con"
        static dSceneChange_c *fn_800573C4();       // 800573C4: returns &gSceneChange (like getSceneChange 80161CC4); called from d_npc_talk_quest_q10
        // ---- d_npc_talk_quest_q10 (.text 800573D0..80058990) ----
        int fn_800573D0(msgInfo_s *info);           // 800573D0 offer check (nml table 8046BFD8, kind 0x14): weather, hour, town checks; day to talk_c+0x335; nml startQuestOffer(info, 10, F8 fn_80053224, 104 fn_800575AC, 1)
        BOOL fn_800575AC();                         // 800575AC 104: EC = fn_80057624
        int fn_80057624(msgInfo_s *info);           // 80057624 EC "Q10_Req", 104 = fn_80057684
        BOOL fn_80057684();                         // 80057684 104: choices 0 = fn_8005776C, 1 = fn_80057A10; dQuestBase_c::clear at getEntry()+0x23E
        void fn_8005776C();                         // 8005776C choice (yes): EC = fn_800577C0
        int fn_800577C0(msgInfo_s *info);           // 800577C0 EC "Q10_OK" (code 4 when talk_c+0x335 is not today), F8 = fn_800578A0, 104 = fn_80057918
        void fn_800578A0();                         // 800578A0 F8: starts the game (dAnimalBlock_c fn_801305E0(player, npc, now))
        BOOL fn_80057918();                         // 80057918 104: setRequest1(0), 128 = fn_80057988
        void fn_80057988();                         // 80057988 128: game start sequence (fn_801A6748(0), fn_80078898)
        void fn_80057A10();                         // 80057A10 choice (no): EC = fn_80057A64
        int fn_80057A64(msgInfo_s *info);           // 80057A64 EC "Q10_No"
        int fn_80057A90(msgInfo_s *info);           // 80057A90 quest check (nml table 8046BF30 entry 12): game of this player; fn_80057EA8 unless timed out, else label getMsgLabel(6, 0xC, 0), 104 = fn_80057C64, fn_800588BC(-5)
        BOOL fn_80057C64();                         // 80057C64 104: dQuestPlayerAnimal_c::clearInfo, fn_800F0650
        int fn_80057CB8(msgInfo_s *info);           // 80057CB8 EC (nml table 804A0784) "Q10_Explain" (minutes / hiders left as words 0 / 1), F8 = fn_80053224, 104 = fn_80057DB0
        BOOL fn_80057DB0();                         // 80057DB0 104: setRequest1(0), 128 = fn_80057E20
        void fn_80057E20();                         // 80057E20 128: game running: state 1, time stamp, fn_801A6748(1)
        int fn_80057EA8(msgInfo_s *info);           // 80057EA8 EC (nml table 804A0784): hider talk "Q10_Visit" / "Q10_Find" (104 fn_80058114) / "Q10_Over" / "Q10_Wait" (104 fn_80058388); F8 = fn_80053224
        BOOL fn_80058114();                         // 80058114 104: hider found (setFlag, fn_800F0708); all found -> setRequest1(0), 128 = fn_80058270; else EC = fn_800582F8; friendship +5
        void fn_80058270();                         // 80058270 128: game end sequence (fn_801A6748(2), fn_80078898)
        int fn_800582F8(msgInfo_s *info);           // 800582F8 EC "Q10_Continue" (hiders left as word 1)
        BOOL fn_80058388();                         // 80058388 104: time over: state 2, setRequest1(0), 128 = fn_80058270; fn_800588BC(-2)
        int fn_8005844C(msgInfo_s *info);           // 8005844C EC (nml table 804A0784) "Q10_Item" / "Q10_ItemFull", F8 = fn_80058574, 104 = fn_800586BC / fn_80058780
        void fn_80058574();                         // 80058574 F8: reward (dAnimalBlock_c fn_80130778) to talk_c+0x280, into the pockets or kept (pockets full); state 3
        BOOL fn_800586BC();                         // 800586BC 104: requestItemAct(+0x280), EC = fn_80058754
        int fn_80058754(msgInfo_s *info);           // 80058754 EC "Q10_End"
        BOOL fn_80058780();                         // 80058780 104: EC = fn_80058754
        int fn_800587F8(msgInfo_s *info);           // 800587F8 EC (nml table 804A0784) "Q10_Lose", F8 = fn_8005885C
        void fn_8005885C();                         // 8005885C F8: state 3 (or clearInfo), fn_8009DFE4
        void fn_800588BC(s8 delta);                 // 800588BC friendship delta for the player on each unfound hider (`this` unused)
        // ---- d_npc_talk_quest_q13 (.text 80058990..8005A3C4) ----
        int fn_80058990(msgInfo_s *info);           // 80058990 offer check (nml table 8046BFD8, kind 0x13): canStartStyle; nml startQuestOffer(info, 7, F8 fn_80053224, 104 fn_80058A98, 1)
        BOOL fn_80058A98();                         // 80058A98 104: EC = fn_80058B10
        int fn_80058B10(msgInfo_s *info);           // 80058B10 EC "Q13_Req", F8 = fn_80058B98, 104 = fn_80058C98
        void fn_80058B98();                         // 80058B98 F8: pickStyleOtherPlayer -> talk_c+0x1EC, its name as word 0
        BOOL fn_80058C98();                         // 80058C98 104: choices (0x15 -> fn_80058D98, 0x1F -> nml selNo); dQuestBase_c::clear at getEntry()+0x23E
        void fn_80058D98();                         // 80058D98 choice (accept): EC = fn_80058DEC
        int fn_80058DEC(msgInfo_s *info);           // 80058DEC EC "Q13_Yes", F8 = fn_80058E4C
        void fn_80058E4C();                         // 80058E4C F8: startStyle(npc, player, talk_c+0x1EC, now), topic text as word 4
        int fn_80058F18(msgInfo_s *info);           // 80058F18 quest check (nml table 8046BF30 entry 9): label getMsgLabel(6, 9, idx), F8/104 from 804A431C; or fn_8005957C / nml startQuestOffer(info, 9, ...)
        BOOL fn_80059504();                         // 80059504 104 (804A431C): EC = fn_8005957C
        int fn_8005957C(msgInfo_s *info);           // 8005957C EC "Q13_Ask" ("Q13_Ask2" in state 1), 104 = fn_80059618
        BOOL fn_80059618();                         // 80059618 104: common choices (fn_80060980 / fn_80045154), 0x36 -> fn_800597EC, 0x39 -> fn_80059E70
        void fn_800597EC();                         // 800597EC choice (answer): reqMenu22(1), 128 = fn_80059838
        void fn_80059838();                         // 80059838 128: typed word == topic text -> EC = fn_8005995C, else fn_80059DBC; close the message
        int fn_8005995C(msgInfo_s *info);           // 8005995C EC "Q13_AnswerOK", 104 = fn_800599C0
        BOOL fn_800599C0();                         // 800599C0 104: EC = fn_80059A38
        int fn_80059A38(msgInfo_s *info);           // 80059A38 EC "Q13_AnswerOK" code 9 (10: pockets full), F8/104 = fn_80059B88/fn_80059C20 (full: fn_80059CE8/fn_80059D44), friendship +3
        void fn_80059B88();                         // 80059B88 F8: pickStylePresent to talk_c+0x280, pickUp, item word 5; state 2
        BOOL fn_80059C20();                         // 80059C20 104: EC = fn_80059CB8, requestItemAct(+0x280)
        int fn_80059CB8(msgInfo_s *info);           // 80059CB8 EC "Q13_Thank"
        void fn_80059CE8();                         // 80059CE8 F8 (pockets full): sendStyleLetterTo(1, 0), flag bit 1 on failure; state 2
        BOOL fn_80059D44();                         // 80059D44 104: EC = fn_80059CB8
        int fn_80059DBC(msgInfo_s *info);           // 80059DBC EC "Q13_AnswerNG", 104 = fn_80059E20
        BOOL fn_80059E20();                         // 80059E20 104: quest state 1
        void fn_80059E70();                         // 80059E70 choice (forgot): EC = fn_80059EC4
        int fn_80059EC4(msgInfo_s *info);           // 80059EC4 EC "Q13_Forget"
        BOOL fn_80059EF4();                         // 80059EF4 104 (804A431C): common choices, 1 -> fn_8005A0C8, 4 -> fn_8005A148
        void fn_8005A0C8();                         // 8005A0C8 choice: EC = fn_8005A11C
        int fn_8005A11C(msgInfo_s *info);           // 8005A11C EC "Q13_Con"
        void fn_8005A148();                         // 8005A148 choice: EC/F8/104 = getEventProcSet() (default 804A1064), run EC
        BOOL fn_8005A19C();                         // 8005A19C 104 (804A431C): dQuestPlayerPair_c::clearInfo
        void fn_8005A1EC();                         // 8005A1EC F8 (804A431C): pickStylePresent to talk_c+0x280, pickUp; clearInfo unless flag bit 1 (then state 3)
        BOOL fn_8005A2A0();                         // 8005A2A0 104 (804A431C): EC = fn_8005A338, requestItemAct(+0x280)
        int fn_8005A338(msgInfo_s *info);           // 8005A338 EC "Q13_End"
        void fn_8005A364();                         // 8005A364 F8 (804A431C): sendStyleLetterTo(0, 0), flag bit 0 on failure; state 3
        // ---- d_npc_talk_quest_q09 (.text 8005A3C4..8005C82C) ----
        int fn_8005A3C4(msgInfo_s *info);           // 8005A3C4: request entry for QUEST_KIND_APPOINTMENT_0 (lbl_8046BFD8 [9]): scene attr 5, canStartAppointment(.., 0x11), then startQuestOffer(msg, 9, fn_80053224, fn_8005A4E8, 1)
        BOOL fn_8005A4E8();                         // 8005A4E8: request accepted: starts the request talk, only while mpController->_6C84 == 0; sets _0EC=fn_8005A560
        int fn_8005A560(msgInfo_s *info);           // 8005A560: "Q09_Req"; sets _104=fn_8005A5C0
        BOOL fn_8005A5C0();                         // 8005A5C0: yes/no choice; clears the villager quest; sets choice=fn_8005A6C0, choice=fn_8005AF74
        void fn_8005A6C0();                         // 8005A6C0: choice "yes"; sets _0EC=fn_8005A714
        int fn_8005A714(msgInfo_s *info);           // 8005A714: "Q09_Reserve"; sets _104=fn_8005A778
        BOOL fn_8005A778();                         // 8005A778: opens the time menu (reqMenu27(1)), only while mpController->_6C84 == 0; sets _128=fn_8005A7E8
        void fn_8005A7E8();                         // 8005A7E8: time menu result: checks the entered time (dTime_c, isSleepTime), sets _0EC from lbl_804A44F8 (Reserved / Error1-3)
        int fn_8005AC64(msgInfo_s *info);           // 8005AC64: "Q09_Reserved"; sets _0F8=fn_8005ACC8
        void fn_8005ACC8();                         // 8005ACC8: dAnimalBlock_c::startAppointment, day/time words, addFriendship(2)
        int fn_8005AE48(msgInfo_s *info);           // 8005AE48: "Q09_Error1"; sets _104=fn_8005A778
        int fn_8005AEAC(msgInfo_s *info);           // 8005AEAC: "Q09_Error2"; sets _104=fn_8005A778
        int fn_8005AF10(msgInfo_s *info);           // 8005AF10: "Q09_Error3"; sets _104=fn_8005A778
        void fn_8005AF74();                         // 8005AF74: choice "no"; sets _0EC=fn_8005AFC8
        int fn_8005AFC8(msgInfo_s *info);           // 8005AFC8: "Q09_No"
        int fn_8005AFF4(msgInfo_s *info);           // 8005AFF4: quest-talk entry (lbl_8046BF30 [11]): this villager has the appointment; meet-time words; _104 = fn_8005B2E4, or fn_8005B58C once the time has passed
        BOOL fn_8005B2E4();                         // 8005B2E4: talk menu (fn_80060980, fn_80045154) with the Q09_Con choice; sets choice=fn_8005B4B8, choice=fn_8005B538
        void fn_8005B4B8();                         // 8005B4B8: choice "conversation"; sets _0EC=fn_8005B50C
        int fn_8005B50C(msgInfo_s *info);           // 8005B50C: "Q09_Con"
        void fn_8005B538();                         // 8005B538: choice: generic talk (getEventProcSet, setProcSet(lbl_804A0E48 entry))
        BOOL fn_8005B58C();                         // 8005B58C: clears the appointment (dQuestPlayerItem_c::clear), only while mpController->_6C84 == 0
        int fn_8005B5DC(msgInfo_s *info);           // 8005B5DC: talk state (lbl_804A0784 [5]); addFriendship(10); "Q09_Welcome"; sets _0F8=fn_80053224, _104=fn_8005B674
        BOOL fn_8005B674();                         // 8005B674: appointment flag 0, only while mpController->_6C84 == 0
        int fn_8005B6C8(msgInfo_s *info);           // 8005B6C8: random room talk (lbl_8046CB44 + 0x24 [0]); "Q_Roomtalk"
        static u16 fn_8005B6F8(dItem::Item *item, u8 looks); // 8005B6F8: random item of the villager's house (dFdBlock_c, scene attr 0x850) flagged for an npc message; returns the message flag
        int fn_8005B858(msgInfo_s *info);           // 8005B858: random room talk [1]: "Q09_Furniture", item from fn_8005B6F8 and the villager's music (dAnimal_c::getMusic)
        int fn_8005B940(msgInfo_s *info);           // 8005B940: random room talk [2]: "Q09_Trade1" (lbl_8046CB44) when appointment flag 3 is clear and a pocket is free; sets _104 = lbl_804A4608 [0] (fn_8005BC04)
        int fn_8005BA28(msgInfo_s *info);           // 8005BA28: talk state (lbl_804A0784 [6]): "Q09_First" (flag 1 clear), "Q09_Trade2" (flag 2 set, 3 clear), else a random room talk (lbl_8046CB44 + 0x24)
        void fn_8005BBD0();                         // 8005BBD0: lbl_804A4608 [1]: fn_80053224(); appointment flag 1
        BOOL fn_8005BC04();                         // 8005BC04: single choice (setChoiceProc) -> fn_8005BC98; sets choice=fn_8005BC98
        void fn_8005BC98();                         // 8005BC98: appointment flag 2; fn_8019C34C()
        u32 fn_8005BCCC(const dItem::Item *item);   // 8005BCCC: trade price: getPrice * (255 - friendship of _17C) / 512, at least 10; also called from d_npc_talk_fmarket
        u32 fn_8005BD4C(const dItem::Item *item);   // 8005BD4C: price scaled by friendship of _17C, rounded to 10, at least 10 (0 without item or memory); called from d_npc_talk_fmarket
        int fn_8005BE04(msgInfo_s *info);           // 8005BE04: talk state (lbl_804A0784 [7]): "Q09_Trade3" / "Q09_Trade4" offer for mItems[0] (price fn_8005BCCC); sets _0F8=fn_80053224, _104=fn_8005BFC0
        BOOL fn_8005BFC0();                         // 8005BFC0: yes/no choice -> fn_8005C098 / fn_8005C340; sets choice=fn_8005C098, choice=fn_8005C340
        void fn_8005C098();                         // 8005C098: choice "yes"; sets _0EC=fn_8005C0EC
        int fn_8005C0EC(msgInfo_s *info);           // 8005C0EC: "Q09_TradeYes" (returns 0); sets _0F8=fn_8005C178, _104=fn_8005C284
        void fn_8005C178();                         // 8005C178: pays (payMoney), picks up mItems[0], appointment flag; sets _110[0]=fn_8005C2C8
        BOOL fn_8005C284();                         // 8005C284: fn_8019C35C(), only while mpController->_6C84 == 0
        void fn_8005C2C8();                         // 8005C2C8: action (fn_800A93BC, fn_800A9354)
        void fn_8005C340();                         // 8005C340: choice "no"; sets _0EC=fn_8005C394
        int fn_8005C394(msgInfo_s *info);           // 8005C394: "Q09_TradeNo" (returns 0)
        int fn_8005C3C4(msgInfo_s *info);           // 8005C3C4: talk state (lbl_804A0784 [8]); "Q09_Wait"; sets _0F8=fn_80053224
        int fn_8005C428(msgInfo_s *info);           // 8005C428: talk state (lbl_804A0784 [9]); "Q09_Analog"; sets _0F8=fn_80053224, _104=fn_8005C4B4
        BOOL fn_8005C4B4();                         // 8005C4B4: fills the +0x26C list (clearNpcChoice, setNpcChoice), then showNpcChoice; sets +0x26C list=fn_8005C544
        void fn_8005C544();                         // 8005C544: +0x26C list proc: pickAppointmentPresent2 (lbl_8046CBD8), removeNewItem; sets _0EC=fn_8005C698
        int fn_8005C698(msgInfo_s *info);           // 8005C698: "Q09_Analog" with the table lbl_8046CC10; sets _104=fn_8005C788
        BOOL fn_8005C788();                         // 8005C788: only while mpController->_6C84 == 0; sets _0EC=fn_8005C800
        int fn_8005C800(msgInfo_s *info);           // 8005C800: "Q09_Bye" (returns 0)
        // ---- d_npc_talk_quest_q08 (.text 8005C82C..8005E584) ----
        int fn_8005C82C(msgInfo_s *info);           // 8005C82C: request entry for QUEST_KIND_APPOINTMENT_1 (lbl_8046BFD8 [8]): scene attr 5, canStartAppointment(.., 0x12), then startQuestOffer(msg, 9, fn_80053224, fn_8005C950, 1)
        BOOL fn_8005C950();                         // 8005C950: request accepted: starts the request talk, only while mpController->_6C84 == 0; sets _0EC=fn_8005C9C8
        int fn_8005C9C8(msgInfo_s *info);           // 8005C9C8: "Q08_Req"; sets _104=fn_8005CA28
        BOOL fn_8005CA28();                         // 8005CA28: yes/no choice; clears the villager quest; sets choice=fn_8005CB28, choice=fn_8005D3DC
        void fn_8005CB28();                         // 8005CB28: choice "yes"; sets _0EC=fn_8005CB7C
        int fn_8005CB7C(msgInfo_s *info);           // 8005CB7C: "Q08_Reserve"; sets _104=fn_8005CBE0
        BOOL fn_8005CBE0();                         // 8005CBE0: opens the time menu (reqMenu27(1)), only while mpController->_6C84 == 0; sets _128=fn_8005CC50
        void fn_8005CC50();                         // 8005CC50: time menu result: checks the entered time (dTime_c, isSleepTime), sets _0EC from lbl_804A4768 (Reserved / Error1-3)
        int fn_8005D0CC(msgInfo_s *info);           // 8005D0CC: "Q08_Reserved"; sets _0F8=fn_8005D130
        void fn_8005D130();                         // 8005D130: dAnimalBlock_c::startAppointment, day/time words, addFriendship(2)
        int fn_8005D2B0(msgInfo_s *info);           // 8005D2B0: "Q08_Error1"; sets _104=fn_8005CBE0
        int fn_8005D314(msgInfo_s *info);           // 8005D314: "Q08_Error2"; sets _104=fn_8005CBE0
        int fn_8005D378(msgInfo_s *info);           // 8005D378: "Q08_Error3"; sets _104=fn_8005CBE0
        void fn_8005D3DC();                         // 8005D3DC: choice "no"; sets _0EC=fn_8005D430
        int fn_8005D430(msgInfo_s *info);           // 8005D430: "Q08_No"
        int fn_8005D45C(msgInfo_s *info);           // 8005D45C: quest-talk entry (lbl_8046BF30 [10]): this villager has the appointment; meet-time words; _104 = fn_8005D724, or fn_8005D9CC once the time has passed
        BOOL fn_8005D724();                         // 8005D724: talk menu (fn_80060980, fn_80045154) with the Q08_Con choice; sets choice=fn_8005D8F8, choice=fn_8005D978
        void fn_8005D8F8();                         // 8005D8F8: choice "conversation"; sets _0EC=fn_8005D94C
        int fn_8005D94C(msgInfo_s *info);           // 8005D94C: "Q08_Con"
        void fn_8005D978();                         // 8005D978: choice: generic talk (getEventProcSet, setProcSet(lbl_804A0E48 entry))
        BOOL fn_8005D9CC();                         // 8005D9CC: clears the appointment (dQuestPlayerItem_c::clear), only while mpController->_6C84 == 0
        int fn_8005DA1C(msgInfo_s *info);           // 8005DA1C: talk state (lbl_804A0784 [1]); "Q08_Call"; sets _0F8=fn_80053224, _104=fn_8005DAA8
        BOOL fn_8005DAA8();                         // 8005DAA8: setRequest1, only while mpController->_6C84 == 0; sets _0EC=fn_8005DCE4, _128=fn_8005DB48
        void fn_8005DB48();                         // 8005DB48: menu result; sets _110[0]=fn_8005DB88
        void fn_8005DB88();                         // 8005DB88: action: walk to l_walkTargetPos (requestWalk); sets _110[0]=fn_8005DC38
        void fn_8005DC38();                         // 8005DC38: action: wait, close the message, camera, clear _110[0] (clearActProc); appointment flag 0
        int fn_8005DCE4(msgInfo_s *info);           // 8005DCE4: "Q08_Door"; addFriendship(10)
        int fn_8005DD2C(msgInfo_s *info);           // 8005DD2C: random room talk (lbl_8046CCE0 [0]); "Q_Roomtalk"
        static int fn_8005DD5C(u16 *msg, dItem::Item *item, int count, int layer, u8 looks); // 8005DD5C: random item of the player's home room layer flagged for an npc message (getNpcMsgFlagged); returns the running count
        int fn_8005DE9C(msgInfo_s *info);           // 8005DE9C: random room talk (lbl_8046CCE0 [1]): furniture remark via fn_8005DD5C; "Q08_Furniture"
        int fn_8005DFA0(msgInfo_s *info);           // 8005DFA0: random room talk (lbl_8046CCE0 [2]): layout remark (word table lbl_804A48B8), pickAppointmentPresent; "Q08_Layout"
        int fn_8005E178(msgInfo_s *info);           // 8005E178: talk state (lbl_804A0784 [2]): "Q08_First" once (flag 1), else a random lbl_8046CCE0 talk; sets _0F8=fn_8005E2BC, _0F8=fn_80053224
        void fn_8005E2BC();                         // 8005E2BC: fn_80053224(); appointment flag 1
        int fn_8005E2F0(msgInfo_s *info);           // 8005E2F0: talk state (lbl_804A0784 [3]); "Q08_Wait"; sets _0F8=fn_80053224, _104=fn_8005E37C
        BOOL fn_8005E37C();                         // 8005E37C: only while mpController->_6C84 == 0; sets _0EC=fn_8005E3F4
        int fn_8005E3F4(msgInfo_s *info);           // 8005E3F4: "Q08_Bye"; sets _0F8=fn_8005E454
        void fn_8005E454();                         // 8005E454: appointment mState = 2
        int fn_8005E480(msgInfo_s *info);           // 8005E480: talk state (lbl_804A0784 [4]); "Q08_Back"; sets _0F8=fn_80053224, _104=fn_8005E50C
        BOOL fn_8005E50C();                         // 8005E50C: only while mpController->_6C84 == 0; sets _0EC=fn_8005E3F4
        // ---- d_npc_talk_reaction (.text 8005E584..80060980) ----
        void fn_8005E584();                         // 8005E584 when the npc timer (getEntry()+0x250) mode > 1: virtual call at vtable +0x38 (resetMood) on this
        int fn_8005E5DC(msgInfo_s *info);           // 8005E5DC  0 "Re_Moveout": dAnimal_c::isMovingOut; code 1..5 by the memory flags (talk_c+0x17C); 104 = fn_80060878
        int fn_8005E740(msgInfo_s *info);           // 8005E740  1 "Re_Cafe": current scene 0x25
        int fn_8005E7CC(msgInfo_s *info);           // 8005E7CC  2 "Re_Fishing": always FALSE
        int fn_8005E7D4(msgInfo_s *info);           // 8005E7D4  3 "Re_Fall"
        int fn_8005E888(msgInfo_s *info);           // 8005E888  4 "Re_Run" (fn_800DCEDC, fgMngProc_isBusy)
        int fn_8005E944(msgInfo_s *info);           // 8005E944  5 "Ev_First" (dAnimal_c::isMovingIn / isMovingOut, private flag 0xD)
        // First-meeting check shared by the Re_First* reactions: compares (a, b, c, movingIn) with "the
        // player has a memory entry with this npc" (talk_c+0x17C, per labelIdx 8..0xC), "the player is from
        // this town", "the npc came from this town" and dAnimal_c::isMovingIn; on a match fills info with
        // label labelIdx and code 0 / 0x15 (checkSick and sick) / 0x16 (checkLostItem and lost item request
        // not done); 104 = fn_80060898 with private flag 2. Inferred.
        BOOL fn_8005EA54(msgInfo_s *info, BOOL a, BOOL b, BOOL c, BOOL movingIn, int labelIdx, BOOL checkSick, u8 checkLostItem); // 8005EA54
        void fn_8005EDA0(int idx);                  // 8005EDA0 setLandName(idx) of the npc's original land (dAnimal_c+0x230C) and copy it to talk_c+0x1D2
        int fn_8005EE18(msgInfo_s *info);           // 8005EE18  6 "Re_FirstA1": fn_8005EA54(info, 0, 1, 1, 1, 6, 0, 0)
        int fn_8005EE58(msgInfo_s *info);           // 8005EE58  7 "Re_FirstA2": fn_8005EA54(info, 0, 1, 1, 0, 7, 1, 1); 104 = nml stepReaction
        int fn_8005EEF4(msgInfo_s *info);           // 8005EEF4  8 "Re_FirstB1": fn_8005EA54(info, 0, 1, 0, 1, 8, 0, 0), fn_8005EDA0(0)
        int fn_8005EF78(msgInfo_s *info);           // 8005EF78  9 "Re_FirstB2": fn_8005EA54 + fn_8005EDA0; 104 = nml stepReaction
        int fn_8005F024(msgInfo_s *info);           // 8005F024 10 "Re_FirstC1": fn_8005EA54 + fn_8005EDA0
        int fn_8005F0A8(msgInfo_s *info);           // 8005F0A8 11 "Re_FirstC2": fn_8005EA54 + fn_8005EDA0; 104 = nml stepReaction
        void fn_8005F154();                         // 8005F154 setLandName(0) of the current player (dPrivateData_c+0x7EC2)
        int fn_8005F1C4(msgInfo_s *info);           // 8005F1C4 12 "Re_FirstV": fn_8005EA54(.., 0xC, ..) for three variants, fn_8005F154
        int fn_8005F320(msgInfo_s *info);           // 8005F320 13 "Re_Movein": dAnimal_c::isMovingIn
        BOOL fn_8005F3F8();                         // 8005F3F8 TRUE = no reaction now: no npc, player has private flag 0xD, lost item request open or the npc is sick
        int fn_8005F4B4(msgInfo_s *info);           // 8005F4B4 14 "Re_Birthday" (dTime_c, leap year; fn_801017B8); 104 = fn_8006094C
        // Not-seen-for-a-while reaction: days since the last talk (dTimeStamp_c at talk_c+0x17C +4) >=
        // minDays; code (days / divisor >= 12 for label 0xF); 104 = fn_80060898 with private flag 2.
        BOOL fn_8005F63C(msgInfo_s *info, int minDays, int divisor, int labelIdx); // 8005F63C
        int fn_8005F7C4(msgInfo_s *info);           // 8005F7C4 15 "Re_30days": fn_8005F63C(info, 30 (language 0) / 60, 30, 0xF)
        int fn_8005F83C(msgInfo_s *info);           // 8005F83C 16 "Re_7days": fn_8005F63C(info, 7 (language 0) / 14, 7, 0x10)
        int fn_8005F8B4(msgInfo_s *info);           // 8005F8B4 17 "Re_Tire": npc timer mode 4
        int fn_8005F980(msgInfo_s *info);           // 8005F980 18 "Re_Anger": npc timer mode
        int fn_8005FA4C(msgInfo_s *info);           // 8005FA4C 19 "Re_Sad": npc timer mode
        int fn_8005FB18(msgInfo_s *info);           // 8005FB18 20 "Re_BeeFace": player flag 2; 104 = fn_80060898
        // dPlayActorMng_c::forEachActiveActor callback of fn_8005FDD8 (arg = the npc): keeps the nearest
        // item actor (fn_8006096C, not items 0x1D1 / 0x1D0) within 48 height and 96 distance in
        // npc+0x2194 / +0x2190.
        static int fn_8005FC5C(dPlayActor_c *actor, void *npc); // 8005FC5C
        dItem::Item fn_8005FDD8();                  // 8005FDD8 the item nearest to the npc (0xFFF1 = none); struct return (r3 = result, r4 = this)
        int fn_8005FE7C(msgInfo_s *info);           // 8005FE7C 21 "Re_Poison": npc timer flag 0x400 clear, scene attr 5; item name of fn_8005FDD8; 104 = fn_800608B8
        int fn_8005FFE4(msgInfo_s *info);           // 8005FFE4 22 "Re_Xmas": dEvent::isOngoing(23), fn_800F4250(looks, time); 104 = fn_800608F0
        int fn_800601F8(msgInfo_s *info);           // 800601F8 23 "Re_Newyear": dEvent::isOngoing(20); 104 = fn_800608F0
        int fn_80060304(msgInfo_s *info);           // 80060304 24 "Re_Harvest": dEvent::isOver(22), dAnimalEventState_c::isInEvent; 104 = fn_800608F0
        int fn_80060450(msgInfo_s *info);           // 80060450 25 "Re_Halloween": dEvent::isOver(21); 104 = fn_800608F0
        int fn_800605A8(msgInfo_s *info);           // 800605A8 26 "Re_Fireworks": dEvent::isOver(16), dTime_c; 104 = fn_800608F0
        int fn_8006071C(msgInfo_s *info);           // 8006071C 27 "download" (fn_800DCEDC / fn_800DCF30, fn_80117318)
        BOOL fn_80060878();                         // 80060878 104: memory flags (talk_c+0x17C) |= 0x100
        BOOL fn_80060898(int kind);                 // 80060898 104: memory flags |= 0x200 (arg: the 104 slot's, unused; nml stepReaction passes it on)
        BOOL fn_800608B8();                         // 800608B8 104: npc timer (getEntry()) flags +0x28C |= 0x400
        BOOL fn_800608F0();                         // 800608F0 104: memory flags |= 0x1000000; fn_800F05C8(getNpcIdx(npc), talk_c+0x180)
        BOOL fn_8006094C();                         // 8006094C 104: memory flags &= ~0x800
        // Item of an item actor: dItem::Item(0x192, actor+0x2F0, 0); struct return. Maybe an inline of the
        // actor class. Inferred.
        static dItem::Item fn_8006096C(dPlayActor_c *actor); // 8006096C
        // ---- d_npc_talk_rollan (.text 80060980..800613BC) ----
        // Answer procedure of a choice menu (setChoice / setChoiceProc, talk_c+0x214 table); called with
        // the talk object only. Provisional.

        // The Rollan answer for this npc: one of the 4 choices of 8046CD30 (fn_80061034 / fn_800611A4 /
        // fn_800611F8 / fn_80061368) by the npc's state (fn_8015112C) and a free pocket, with its
        // menu code (807502D8: 7, 7, 10, 10) in *code and 3 in *flag (both may be NULL). Null when the
        // player is not from this town, has private flag 0xD, a visitor is here (dEvent::isVisitorHere(9)),
        // the npc is moving / sick / has a lost item, ... Struct return (r3 = result, r4 = this).
        hookFunc fn_80060980(u16 *code, u8 *flag);  // 80060980
        int fn_80060C44(msgInfo_s *info);           // 80060C44 EC: nml label kind 7 (getMsgLabel(7, 0, 0)) when fn_80060980 has an answer; setTopic(5, 0, 0)
        void fn_80060D28(int arg);                  // 80060D28 F8: msgMemory set, talk count 0x44
        BOOL fn_80060DB8(int kind);                 // 80060DB8 104: two choices: fn_80060980's answer and nml selResumeTalk (804A4B60)
        // Present roll: needs a free pocket and no private flag 0xD; percent chance 20 + (sum of two bytes
        // of the player's home entry, fn_800AC28C) / 10, +5 when the player byte +0x83F9 is 4, max 100.
        static BOOL fn_80060ED8();                  // 80060ED8
        void fn_80060FCC();                         // 80060FCC fn_801510EC(getTown() +0x72CC0, npc's dAnimalBlock_c index): marks this npc
        void fn_80061034();                         // 80061034 choice: EC = fn_800610C4 (present) or fn_80061140 by fn_80060ED8
        int fn_800610C4(msgInfo_s *info);           // 800610C4 EC: present item 0x36C to talk_c+0x280, "Ev_Rollan" code 4..6
        int fn_80061140(msgInfo_s *info);           // 80061140 EC: "Ev_Rollan" code 1..3 (no present)
        void fn_800611A4();                         // 800611A4 choice: EC = fn_80061140
        void fn_800611F8();                         // 800611F8 choice: EC = fn_80061288 (present) or fn_80061304 by fn_80060ED8
        int fn_80061288(msgInfo_s *info);           // 80061288 EC: present item 0x313 to talk_c+0x280, "Ev_Rollan" code 0xE..0x10
        int fn_80061304(msgInfo_s *info);           // 80061304 EC: "Ev_Rollan" code 0xB..0xD (no present)
        void fn_80061368();                         // 80061368 choice: EC = fn_80061304
        // ---- d_npc_talk_town (.text 800613BC..80061ADC) ----
        // EC (via 8046CD78): "Town_Rumor": a memory of a player from another town (save list at dSaveData_c::getRaw()
        // +0x735E0, fn_80116A74) that is not the current player; land / personal name as words 0 / 1,
        // code from the memory kind (jumptable 804A4BCC, dAnimal_c::findMemory2), unit word 2
        // ("sys_STRING/STR_Unit", fn_801A5874) or clearWord(2). FALSE when there is no such memory.
        int fn_800613BC(msgInfo_s *info);           // 800613BC
        int fn_80061670(msgInfo_s *info);           // 80061670 EC: (via 8046CD78): "Town_Always", code 0; reloads record 17 (setProcSet)
        int fn_800616D0(msgInfo_s *info);           // 800616D0 EC (record 17): rumor / always by the odds 807502E0 ({20, 80}), table 8046CD78; falls back to fn_80061670
        int fn_80061790(msgInfo_s *info);           // 80061790 EC (record 18): "Town_Theater", code 0x1F / 0x1E / sbss 8074EAC8 (theater state 80600874, fn_8016CD8C)
        int fn_80061844(msgInfo_s *info);           // 80061844 EC (record 19): "Town_Grace", code by dSaveShopGrace_c sale stage / sold out / town flag 0x17
        static const char *fn_80061934(u32 looks);  // 80061934 "3P_*" label of a personality (table 804A4BF8; NULL for >= 6); nml getLabelByTable kind 0x14
        int fn_80061958(msgInfo_s *info);           // 80061958 EC (record 20): 3P label of the npc's partner villager (npc+0x1C48 -> +0x1F94), anm-personal names 0 / 1

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
        /* 0x184 */ int mJinxTown;                 // other-town index for the jinx topic (-1: this town)
        /* 0x188 */ int mRumorTown;                 // rumor town index
        /* 0x18C */ u32 mImpression;                 // impression level; init 50 = none; tested unsigned (< 50)
        /* 0x190 */ dPersonalID_c mMsgPersonal;
        /* 0x1BC */ dPlayerID_c mMsgPlayer;         // q13
        /* 0x1D2 */ dLandID_c mMsgLand;
        /* 0x1E8 */ u8 _1E8[0x1EC - 0x1E8];
        /* 0x1EC */ dPlayerID_c *_1EC;        // the other player of the style quest (q13)
        /* 0x1F0 */ u32 mTopicKind;                 // setTopic
        /* 0x1F4 */ int mMsgYear;                 // countdown year
        /* 0x1F8 */ int mFtrPosX;                 // fmarket deal
        /* 0x1FC */ int mFtrPosZ;                 // fmarket deal
        /* 0x200 */ int _200;                 // fmarket handle (fn_800A8F98)
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
        /* 0x32A */ u8 mQuestKind;                  // picked quest kind
        /* 0x32B */ u8 mAnswer;                  // chosen answer index
        /* 0x32C */ u8 mCountTalk;                  // count this talk
        /* 0x32D */ u8 mHadNickname;                  // nickname flag
        /* 0x32E */ u8 mIsAnimalItem;                  // the item came from the villager
        /* 0x32F */ u8 _32F[4];               // carnival game counters
        /* 0x333 */ u8 _333[2];
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
    /* 0x1FB0 */ dNpcBirthdayMood_c mBirthdayMood; // (d_npc_talk_birthday; dtor 800494E8)
    /* 0x2190 */ u8 _2190[6];
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
extern dAcNpcNml_c::talk_c::procSet_s l_talkProcSets[33];  // 804A0784
extern const dAcNpcNml_c::talk_c::procSet_s l_talkEntrySets[dAcNpcNml_c::talk_c::TALK_NUM]; // 804A0E48 ([15] = free talk, the default)

extern mVec3_c l_walkTargetPos; // .bss: (256, 0, 464) walk target (d_npc_talk_fmarket, d_npc_talk_quest_q08)

// label tables read by talk_c::getMsgLabel (kinds 8 and 0xE). In .data they sit among later data
// (after 804A10D8 / before the vtables), so they are defined later in the file (next to the
// code that uses those kinds) and must be globals declared here (placeholder names).
extern const char *l_aiqLabels[3];       // 804A110C {"AiQ_Approach", "AiQ_First", "AiQ_Again"}
extern const char *l_aiGreetLabels[16];  // 804A12D0 {"Ai_Snow1", "Ai_Rain1", .., "AiV_Fine2"}
