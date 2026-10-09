#pragma once

#include <types.h>
#include <lib/egg/core/eggMsgRes.h>
#include <game/game/d_dvd.hpp>

class dAnmPersonalID_c;
class dPersonalID_c;

// Script/word objects used to build localized names. Source: src/dol/game/d_script.cpp
// (.text 8015593C..8015B680; Word_c itself is in d_script_word.inc).
// Names come from RTTI: dScript::Word_c (vtable 804EFB90), with
// dScript::Inflect_c and dScript::Pacchim_c embedded as members (Word_c's
// RTTI lists no bases, so these are not base classes).
namespace dScript {

// Number formats for formatNumber. The odd values add thousands separators.
// Names are inferred.
enum NumberFormat_e {
    NUM_FORMAT_PLAIN,
    NUM_FORMAT_PLAIN_SEP,
    NUM_FORMAT_SPACE_PAD,
    NUM_FORMAT_SPACE_PAD_SEP,
    NUM_FORMAT_SPACE_PAD_END,
    NUM_FORMAT_SPACE_PAD_END_SEP,
    NUM_FORMAT_ZERO_PAD,
    NUM_FORMAT_ZERO_PAD_SEP,
    NUM_FORMAT_8,
    NUM_FORMAT_REGION, // plain, with separators in North America
};

// A BMG message resource. Name from RTTI; vtable 804EFCAC.
class Res_c : public EGG::MsgRes {
public:
    Res_c(const void *data); // 8015784C
    virtual ~Res_c(); // 80157888

    const wchar_t *getMessage(int id); // 801578E0
    void *getEntry(int id); // 801578E8
    u16 getCount(); // 801578F0: number of INF1 entries
};

// Grammatical info for a word: gender plus the definite/indefinite article
// indices used by the European languages.
class Inflect_c {
public:
    Inflect_c() : mGender(-1), mIndefArticle(0), mDefArticle(0) {}
    virtual ~Inflect_c() {} // 8015B598

    void setGender(u8 gender); // 80157BF8
    void setIndefArticle(u8 article); // 80157C00
    void setDefArticle(u8 article); // 80157C70

    s32 mGender; // 0x04: -1 when unset
    u16 mIndefArticle; // 0x08: 0 when unset or out of range
    u16 mDefArticle; // 0x0A
}; // sizeof = 0xC

// Korean particle helper: remembers the last Hangul syllable so the
// following particle can be chosen by its final consonant (batchim).
class Pacchim_c {
public:
    Pacchim_c() : mLastChar(0) {}
    virtual ~Pacchim_c() {} // 8015B5D8

    void check(wchar_t c); // 80157B18: keeps c if it is in U+AC00..U+D7A3

    u16 mLastChar; // 0x04
}; // sizeof = 0x8

// A word/name built from message data. Subclasses provide the buffer.
// The vtable has 55 slots (0xDC bytes). Text is parsed after every change
// (parse): BMG tags (0x1A) are dispatched through member-function
// tables to the tag handlers, which replace the tag with text.
// Names are inferred; the remaining vfXX (named by vtable offset) are default
// tag handlers that subclasses override.
class Word_c {
public:
    typedef int (Word_c::*TagFunc)(int pos);

    Word_c(); // 80157D3C
    virtual ~Word_c(); // 80157D8C
    virtual u32 getBufferSize() = 0; // vtable +0x0C
    virtual wchar_t *getBuffer() = 0; // vtable +0x10
    virtual int setTagWord(int pos, Word_c *word); // 801598B8: insert word at a tag (with article)
    virtual int getParseStart(); // 8015822C: start position for parse()
    virtual int procTownName(int pos); // 8015A2E8: town name
    virtual int procRubyTag(int pos); // 8015B134: furigana (JP only)
    virtual int setInflect(int pos); // 8015A474: grammar 0: sets gender/articles of the next word from the params
    virtual int procTag(int pos); // 80158088: tag: dispatch by group
    virtual int procNewLine(int pos); // 80158258: newline
    virtual int procEnd(int pos); // 80158260: end of string
    virtual int procChar(int pos); // 80158268: plain character
    virtual int procNumber(int pos); // 8015A038: time 6-12: number slot (param)
    virtual int procRandomNumber(int pos); // 8015A03C: time 13: number slot, rolled in [min, max] if unset
    virtual int procWord(int pos); // 8015A0A0: word 0-10: word slot (param)
    virtual int procDrink(int pos); // 8015A0A4: word 4 (pre-scan): random drink
    virtual int procObject(int pos); // 8015A0A8: word 5 (pre-scan): random object (gendered)
    virtual int procSweets(int pos); // 8015A0AC: word 6 (pre-scan): random sweets
    virtual int procSports(int pos); // 8015A0B0: word 7 (pre-scan): random sport
    virtual int procMusic(int pos); // 8015A0B4: word 8 (pre-scan): random music
    virtual int procFood(int pos); // 8015A0B8: word 9 (pre-scan): random food
    virtual int procHobby(int pos); // 8015A0BC: word 10 (pre-scan): random hobby (gendered)
    virtual int procUnit(int pos); // 8015A0C0: word 11: counter unit (see procAnimalUnit)
    virtual int procPlayerChar0(int pos); // 8015A1AC: word 12: 1st character of the player's name
    virtual int procPlayerChar1(int pos); // 8015A1B0: word 13: 2nd character of the player's name
    virtual int procPlayerChar2(int pos); // 8015A1B4: word 14: 3rd character of the player's name
    virtual int procPlayer(int pos); // 8015A1B8: name 0: player name (see procPlayerName)
    virtual int procAnimal(int pos); // 8015A248: name 1: villager name (see procAnimalName)
    virtual int procAnimalA(int pos); // 8015A2DC: name 2: another villager picked from the town
    virtual int procAnimalB(int pos); // 8015A2E0: name 3: another villager picked from the town
    virtual int procAnimalParam(int pos); // 8015A2E4: name 4: villager from a slot (param)
    virtual int procCatchphrase(int pos); // 8015A370: name 6: villager's catchphrase (dHabitWord_c)
    virtual int procPlayerNickname(int pos); // 8015A374: name 7: what the villager calls the player
    virtual int procImpression(int pos); // 8015A450: name 10: player impression (STR_Impress)
    virtual int procPlayerStarSign(int pos); // 8015A454: name 12: player's star sign
    virtual int procAnimalStarSign(int pos); // 8015A458: name 13: villager's star sign
    virtual int procGreeting(int pos); // 8015A45C: block 6: villager's custom greeting, replaces text up to block 7
    virtual int procAnimalFlagBegin(int pos); // 8015A464: block 8: keeps or drops text up to block 9 by a villager flag
    virtual int vfA0(int pos); // 8015A46C: block 26
    virtual int vfA4(int pos); // 8015A470: group 7 (all ids)
    virtual int procPlayerGender(int pos); // 8015A49C: grammar 4: one of two strings by the player's gender
    virtual int procWordGender(int pos); // 8015A580: grammar 5: one of three strings by a word slot's gender
    virtual int procAnimalGender(int pos); // 8015A98C: grammar 8: one of two strings by the villager's gender
    virtual int procPlayerElision(int pos); // 8015AD68: grammar 10: elision by the player name
    virtual int procAnimalElision(int pos); // 8015AD6C: grammar 11: elision by the villager name
    virtual int procAnimalAElision(int pos); // 8015AD70: grammar 12: elision by procAnimalA
    virtual int procAnimalBElision(int pos); // 8015AD74: grammar 13: elision by procAnimalB
    virtual int procAnimalParamElision(int pos); // 8015AD78: grammar 14: elision by procAnimalParam
    virtual int procPlayerNicknameElision(int pos); // 8015AE0C: grammar 16: elision by procPlayerNickname
    virtual int procWordElision(int pos); // 8015AE10: grammar 17: elision by a word slot
    virtual int procNumberPlural(int pos); // 8015AE14: grammar 18: plural form by a number slot
    virtual int procColor(int pos); // 8015B12C: system 0: text color (u8 param)
    virtual int procScale(int pos); // 8015B130: system 1: text scale (u16 param)

    void clear(); // 80157DCC: zeroes the buffer and resets the word state
    void preParse(); // pre-scan: group 4 tags
    void parse(); // parse
    int procIconTag(int pos); // tag group 0 (button icons)
    int procIconTag2(int pos); // tag group 1 (button icons)
    int procTimeTag(int pos); // tag group 3 (date/time, number slots)
    int preParseWordTag(int pos); // tag group 4 (pre-scan: random words into word slots)
    int procWordTag(int pos); // tag group 4 (word slots)
    int procNameTag(int pos); // tag group 5
    int procBlockTag(int pos); // tag group 6 (text blocks)
    int procGroup7Tag(int pos); // tag group 7
    int procGrammarTag(int pos); // tag group 12 (grammar)
    int procSystemTag(int pos); // tag group 0xFF
    int insertIcon(int pos);
    int insertIcon2(int pos);
    BOOL capitalize(); // capitalizes the first character
    u32 getCapacity(); // capacity in characters
    u32 getTextLength(const wchar_t *str, int breakOnNewLine); // length of str (tags count as their size)
    u32 getLength(int breakOnNewLine); // length of this word
    BOOL isSame(Word_c *other); // same text
    BOOL getChar(wchar_t *out, u32 index); // character at index
    BOOL set(const wchar_t *str, int); // 801590F0: clear(), copy, then parse
    BOOL copy(const Word_c *src, int breakOnNewLine); // copy
    BOOL appendText(const wchar_t *str, int breakOnNewLine); // append and parse
    BOOL appendChar(wchar_t c); // append a character and parse
    BOOL appendWord(Word_c *other, int breakOnNewLine); // append a word and parse
    BOOL writeText(const wchar_t *str, u32 pos, int breakOnNewLine); // write at pos
    BOOL appendRaw(const wchar_t *str, int breakOnNewLine); // append
    BOOL write(const wchar_t *str, u32 len, u32 pos); // write len characters at pos
    BOOL insert(const wchar_t *str, u32 len, u32 pos); // insert at pos
    BOOL append(const wchar_t *str, u32 len); // append len characters
    BOOL erase(u32 len, u32 pos); // delete len characters at pos
    int removeTag(int pos); // default tag handler: deletes the tag
    u32 insertWord(u32 pos, const wchar_t *str, int len, BOOL isArticle); // insert with capitalization
    int procYear(int pos); // year
    int procMonth(int pos); // month name
    int procDay(int pos); // day
    int procHour(int pos); // hour
    int procMinute(int pos); // minute
    int procSecond(int pos); // second
    const char *getGenderedGroup(int kind, dAnmPersonalID_c *id); // gendered string group
    int procAnimalUnit(int pos, void *obj, dAnmPersonalID_c *id);
    int procPlayerName(int pos, const dPersonalID_c *pid); // player name
    int procAnimalName(int pos, dAnmPersonalID_c *id); // villager name
    int procWeekday(int pos); // weekday
    int procGreetingEnd(int pos); // block 7: end of procGreeting
    int procAnimalFlagEnd(int pos); // block 9: end of procAnimalFlagBegin
    int setDefiniteArticle(int pos); // definite article for the next word
    int setNoArticle(int pos); // no article for the next word
    int setCapitalize(int pos); // capitalize the next word
    int procByPlayerGender(int pos, const dPersonalID_c *pid); // procSelect2 by the player's gender
    int procSelect2(int pos, int select); // picks one of two strings
    int procByWordGender(int pos, const Word_c *word); // picks one of three strings by word's gender
    int procCounterJP(int pos); // JP counter after a number
    int procParticleKR(int pos); // KR particle
    int procByAnimalGender(int pos, dAnmPersonalID_c *id); // by villager gender
    int setCapitalizeWords(int pos); // capitalize every word of the next word
    int procElisionPlayer(int pos, const dPersonalID_c *pid);
    int procElisionAnimal(int pos, dAnmPersonalID_c *id);
    int procElisionWord(int pos, int offset, Word_c *word); // elision by the word's first letter
    int procElision(int pos, int offset, wchar_t first);
    int procTownElision(int pos); // elision by the town name
    int procPlural(int pos, Word_c *word); // by "0"/"1"/other
    int procCounterJP2(int pos); // JP counter after a number
    int procRubyJP(int pos); // furigana (JP only)
    int procRuby(int pos); // furigana
    BOOL fill(wchar_t c, u32 len, u32 pos); // fill
    BOOL censor(); // replaces every character with '!'
    int procRandomWord(int pos, int kind, dAnmPersonalID_c *id, u16 *used); // random word
    void clearCapitalize(); // clears the capitalization flags

    Inflect_c mInflect; // 0x04
    Pacchim_c mPacchim; // 0x10
    s32 _18; // 0x18: set to 1 by the date/number handlers before setTagWord
    s32 _1C; // 0x1C: article for the next word (0 indefinite, 1 definite, 2 none)
    u8 _20; // 0x20: capitalize the next word
    u8 _21; // 0x21: capitalize every word of the next word
    u8 _22; // 0x22 (cleared by clear(), not by the constructor)
    u8 _23; // 0x23
}; // sizeof = 0x24


// The script.arc bank (global at 805F2E94). Name from RTTI; vtable 804EFCE0.
// Its destructor, vtable and RTTI are not in d_script: they follow it, right
// after fn_8015B618 (8015B618..8015B680, data 804EFCE0..804EFD30).
class Bank_c : public dDvd::arcBank_c {
public:
    virtual ~Bank_c(); // 8015B620

    /* 0x74 */ u8 _74[0x18];
}; // size 0x8C

int getStringLength(const wchar_t *text, u32 maxSize, int breakOnNewLine); // 8015790C
BOOL setNumber(Word_c *word, int value, int digits, NumberFormat_e format); // 8015665C
BOOL formatNumber(Word_c *word, int value, int digits, NumberFormat_e format, int sep); // 801566E8
u8 getTagLength(const wchar_t *tag);
int to12Hour(int hour); // 80156A8C
BOOL setAmPm(Word_c *word, int hour); // 80156B00
BOOL isHiragana(wchar_t c); // 80155D9C
BOOL isKatakana(wchar_t c); // 80155DD0
BOOL isAlpha(wchar_t c); // 80155E6C
bool toLower(wchar_t *c); // 80156368
int getLineLength(const u16 *text, u32 maxSize); // 801579C0
Bank_c *getBank(); // 80157B30
void *getBmgFile(Bank_c *bank, const char *name); // 80157BA0

} // namespace dScript
