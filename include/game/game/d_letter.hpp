#pragma once

// d_letter (DOL TU d_letter.cpp, .text 800CB0CC..800CCCD8): letter text building. The header / body / footer of a
// letter come from the sys_MAIL1 / sys_MAIL2 BMGs, with the word and number slots of a global dLetter::Comp_c
// (the counterpart of dMsg::Comp_c for letters) and its addressee / sender filled in by dLetter::Word_c's tag
// handlers. Also the keyboard key sound effects (d_kbd). Names from RTTI: dLetter::Comp_c, dLetter::Word_c; the
// free function names are inferred (after dMsg::Comp_c / dString where they do the same thing).

#include <types.h>
#include <game/game/d_script.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>

#define LETTER_WORD_NUM 20            // Comp_c::mWords
#define LETTER_NUM_WORD_NUM 10        // Comp_c::mNumWords
#define LETTER_RANDOM_WORD_KIND_NUM 7 // Comp_c::mRandomWords: drink, object, sweets, sports, music, food, hobby

namespace dLetter {

// A word with a buffer large enough for a letter part. Name from RTTI; vtable 804EB138. The tag handlers read
// the global Comp_c (the addressee is the "player", the sender the "animal").
class Word_c : public dScript::Word_c {
public:
    Word_c(); // 800CC2C4
    virtual ~Word_c(); // 800CC308
    virtual u32 getBufferSize(); // 800CC360
    virtual wchar_t *getBuffer(); // 800CC368
    virtual int procRubyTag(int pos); // 800CCAE0
    virtual int procNumber(int pos); // 800CC370
    virtual int procWord(int pos); // 800CC418
    virtual int procDrink(int pos); // 800CC5BC
    virtual int procObject(int pos); // 800CC5C4
    virtual int procSweets(int pos); // 800CC5CC
    virtual int procSports(int pos); // 800CC5D4
    virtual int procMusic(int pos); // 800CC5DC
    virtual int procFood(int pos); // 800CC5E4
    virtual int procHobby(int pos); // 800CC5EC
    virtual int procUnit(int pos); // 800CC5F4
    virtual int procPlayerChar0(int pos); // 800CC714
    virtual int procPlayerChar1(int pos); // 800CC71C
    virtual int procPlayerChar2(int pos); // 800CC724
    virtual int procPlayer(int pos); // 800CC72C
    virtual int procAnimal(int pos); // 800CC774
    virtual int procAnimalA(int pos); // 800CC7BC: (removed)
    virtual int procAnimalB(int pos); // 800CC7C0: (removed)
    virtual int procAnimalParam(int pos); // 800CC7C4: (removed)
    virtual int procCatchphrase(int pos); // 800CC7C8: (removed)
    virtual int procPlayerNickname(int pos); // 800CC7CC: (removed)
    virtual int procImpression(int pos); // 800CC7D0: (removed)
    virtual int procPlayerStarSign(int pos); // 800CC7D4: (removed)
    virtual int procAnimalStarSign(int pos); // 800CC7D8: (removed)
    virtual int procPlayerGender(int pos); // 800CC7DC
    virtual int procWordGender(int pos); // 800CC86C
    virtual int procAnimalGender(int pos); // 800CC824
    virtual int procPlayerElision(int pos); // 800CC8EC
    virtual int procAnimalElision(int pos); // 800CC934
    virtual int procAnimalAElision(int pos); // 800CC97C: (removed)
    virtual int procAnimalBElision(int pos); // 800CC980: (removed)
    virtual int procAnimalParamElision(int pos); // 800CC984: (removed)
    virtual int procPlayerNicknameElision(int pos); // 800CC988: (removed)
    virtual int procWordElision(int pos); // 800CC98C
    virtual int procNumberPlural(int pos); // 800CCA28

    int procRandomWordTag(int pos, int kind); // 800CC4BC: word 4-10: random word of kind, remembered per slot
    int procPlayerChar(int pos, u32 index); // 800CC650: character index of the addressee's name

    /* 0x024 */ wchar_t mBuffer[0x401];
}; // size 0x828

// The letter's word slots and people (global instance 805A02D0). Name from RTTI; vtable 804EB118.
class Comp_c {
public:
    virtual ~Comp_c(); // 800CCC54

    /* 0x0004 */ dString::Word_c mWords[LETTER_WORD_NUM];        // word slots (setWord)
    /* 0x12C4 */ dString::Word_c mNumWords[LETTER_NUM_WORD_NUM]; // number slots
    /* 0x1C24 */ dPersonalID_c mTo;                             // the addressee (setTo)
    /* 0x1C50 */ dAnmPersonalID_c mFrom;                        // the sender (setFrom)
    /* 0x1D10 */ u16 mRandomWords[LETTER_RANDOM_WORD_KIND_NUM][LETTER_WORD_NUM]; // random word msg ids per kind and slot
}; // size 0x1E28

} // namespace dLetter

namespace dItem {
struct Item;
}
namespace dLetter {

// The mail BMGs (the kind argument of getEntryNum / loadEntry).
enum MailKind_e {
    MAIL_KIND_TO,     // sys_MAIL1/TO/: header
    MAIL_KIND_MSG,    // sys_MAIL1/MSG/: body
    MAIL_KIND_FROM,   // sys_MAIL1/FROM/: footer
    MAIL_KIND_BBS,    // sys_BBS/: notice board text
    MAIL_KIND_TO2,    // sys_MAIL2/TO2/
    MAIL_KIND_MSG2_A, // sys_MAIL2/MSG2_A/: the three body parts
    MAIL_KIND_MSG2_B,
    MAIL_KIND_MSG2_C,
    MAIL_KIND_FROM2,  // sys_MAIL2/FROM2/
    MAIL_KIND_NUM
};

// Keyboard key sound effects (d_kbd). The set depends on the language and the player's gender.
BOOL isMalePlayer();                       // 800CB0CC: the player is not female (TRUE without a player)
int getKeySeSet();                        // 800CB110: SE set: 0 US, 1 QC, 2 MX (+3 for a female player)
void playDigitKeySe(int idx, BOOL a, BOOL b); // 800CB188: digit key SE (b: third, a: second set)
void playUpperKeySe(int idx);                // 800CB1EC: upper case letter key SE
void playAltKeySe(int idx);                // 800CB234: letter key SE (b)
void playLowerKeySe(int idx);                // 800CB27C: lower case letter key SE
void playKeySe(wchar_t c, BOOL a, BOOL b, BOOL pcKeyboard); // 800CB2DC: the key SE of c (0x16DA for others); pcKeyboard (d_kbd) is unused

int getEntryNum(int kind, const char *label); // 800CB34C: the number of entries of the mail BMG kind/label
// 800CB3C8: word = entry idx of the mail BMG kind/label (it has to fit)
BOOL loadEntry(dScript::Word_c *word, int kind, u16 idx, const char *label);
void reset();                       // 800CB594: clears the global Comp_c
// 800CB638: header / body / footer words of the mail text label (kind: the entry).
BOOL loadText(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 kind, const char *label);
// 800CB66C / 800CB760: header / body / footer words from explicit entries.
BOOL loadTextParts(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx,
                 u16 bodyIdx, u16 footerIdx, const char *headerLabel, const char *bodyLabel, const char *footerLabel);
BOOL loadBbsText(dScript::Word_c *word, u16 msgId, const char *group); // 800CB720: word = BMG string group[msgId]
BOOL loadText2(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 headerIdx, u16 bodyA,
                 u16 bodyB, u16 bodyC, u16 footerIdx, const char *bodyLabel, const char *label);
u16 getToNum(const char *label); // 800CBA14: header variants of a mail label
u16 getMsgNum(const char *label); // 800CBA40: body variants
u16 getFromNum(const char *label); // 800CBA6C: footer variants
u16 getMsg2ANum(const char *label); // 800CBA98
u16 getTo2Num(const char *label); // 800CBAC4
void setMonthName(int slot, int month);                         // 800CBAF0: month name word
void setDayName(int slot, u8 day);                            // 800CBB50: day word
void setPersonalName(int slot, const dPersonalID_c *pid);          // 800CBBB0: player name word
void setLandName(int slot, const dLandID_c *land);             // 800CBC10: town name word
void setAnmPersonalName(int slot, const dAnmPersonalID_c *animal);    // 800CBC70: villager name word
void setWord(int slot, dScript::Word_c *word);             // 800CBCD4: slot = a copy of word
void setStringWord(int slot, u16 index, const char *group);      // 800CBD30: slot = BMG string group[index]
void setItemName(int slot, const dItem::Item *item);           // 800CBDA0: item name word
dScript::Word_c *getWord(u8 slot);                         // 800CBDF4: word slot
void setNumber(int slot, int value, int digits, dScript::NumberFormat_e format); // 800CBE0C: number word (nothing when value < 0)
void setNumber4(int slot, int value);                         // 800CBEB4
void formatNumber(int slot, int value, int digits, dScript::NumberFormat_e format); // 800CBF34: number message word
void setHour(u8 hour, int slot, int numSlot);              // 800CBFE0: am/pm word and the 12-hour number
dScript::Word_c *getNumWord(u8 slot);                         // 800CC070: number slot
u16 getRandomWord(int kind, int slot);                           // 800CC088: random word of kind used for slot
void setRandomWord(int kind, int slot, u16 msgId);               // 800CC0A8
void setTo(const dPersonalID_c *to);                     // 800CC0C8
void setFrom(const dAnmPersonalID_c *sender);              // 800CC1AC
dPersonalID_c *getTo();                                  // 800CC2A4: the addressee
dAnmPersonalID_c *getFrom();                               // 800CC2B4: the sender
} // namespace dLetter
