#ifdef __INTELLISENSE__
#include <game/game/d_script.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstring>
#include <cstdio>

namespace dScript {

// TODO: remove these when no longer needed
// Helpers defined in this TU. Names are inferred.
BOOL isVowelSound(int c);
u16 getSortKey(wchar_t c);
const char *getScriptDir();
BOOL isDigit(wchar_t c);
BOOL isHiragana(wchar_t c);
BOOL isKatakana(wchar_t c);
BOOL isUpperAlpha(wchar_t c);
BOOL isLowerAlpha(wchar_t c);
BOOL isAlpha(wchar_t c);
BOOL isFullwidthUpper(wchar_t c);
BOOL isFullwidthLower(wchar_t c);
BOOL isLatin1Upper(wchar_t c);
BOOL isLatin1Lower(wchar_t c);
BOOL isGreekLower(wchar_t c);
BOOL isGreekUpper(wchar_t c);
BOOL isHangul(wchar_t c);
BOOL isSeparator(wchar_t c);
bool toUpper(wchar_t *c);
bool toLower(wchar_t *c);
BOOL setNumber4(Word_c *word, int value);
BOOL setNumber2(Word_c *word, int value);
BOOL setMinute(Word_c *word, int value);
BOOL setSecond(Word_c *word, int value);
BOOL setNumber(Word_c *word, int value, int digits, NumberFormat_e format);
BOOL formatNumber(Word_c *word, int value, int digits, NumberFormat_e format, int sep);
BOOL setDecimal(Word_c *word, int decimals, f64 value);
int to12Hour(int hour);
BOOL setAmPm(Word_c *word, int hour);
BOOL setWeekdayName(Word_c *word, int day, BOOL abbrev);
BOOL setMonthName(Word_c *word, int month);
BOOL setDayName(Word_c *word, int day);
BOOL setCounter(Word_c *word, int type, int count);
BOOL setCounter2(Word_c *word, int type, int count);
BOOL makeNickname(Word_c *out, const Word_c *name, u16 index);
void appendMinus(wchar_t *buf);
void writeDigits(wchar_t *buf, int value, int digits);
void padSpacesFront(wchar_t *buf, int width);
void padSpacesBack(wchar_t *buf, int width);
void padZeros(wchar_t *buf, int width);
int numLength(const wchar_t *buf);
void shiftRight(wchar_t *buf, int pos, int n);
void reverse(wchar_t *buf);
void insertSeparators(wchar_t *buf, wchar_t sep);
BOOL isSpacePadFormat(NumberFormat_e format);
BOOL isSpacePadEndFormat(NumberFormat_e format);
BOOL isZeroPadFormat(NumberFormat_e format);
BOOL isSeparatorFormat(NumberFormat_e format);
int getLineLength(const u16 *text, u32 maxSize);
u8 getTagGroup(const wchar_t *tag);
u16 getTagId(const wchar_t *tag);
u8 getTagLength(const wchar_t *tag);
const u8 *getTagParams(const wchar_t *tag);
Bank_c *getBank();
BOOL loadBank(Bank_c *bank, void *heap);
void *getBmgFile(Bank_c *bank, const char *name);
u32 getBankHeapSize();
const char *getStringGroup(int kind, u8 gender);

// Dependencies whose owners are not recovered yet.
extern "C" {
BOOL fn_8016AE68(Word_c *word, u16 index, const char *group); // loads a BMG string
u16 fn_8016AF68(const char *group); // number of strings in a group
u16 fn_8016AFD4(const char *group); // random string index
extern u8 lbl_805FF398[];
}

}

#endif

// dScript::Word_c and the other functions after the static initializer
// (80157D3C..8015B680). Included at the end of d_script.cpp; see there.
// First pass: written for equivalence, matching has not started.

namespace dScript {

// 80157D3C
Word_c::Word_c() : _18(0), _1C(0), _20(0), _21(0) {}

// 80157D8C
Word_c::~Word_c() {}

// 80157DCC
void Word_c::clear() {
    memset(getBuffer(), 0, getBufferSize());
    mInflect.mGender = -1;
    mInflect.mIndefArticle = 0;
    mInflect.mDefArticle = 0;
    mPacchim.mLastChar = 0;
    _18 = 0;
    _1C = 0;
    _21 = 0;
    _20 = 0;
    _22 = 0;
    _23 = 0;
}

// 80157E60: runs the group 4 tags before the real parse
void Word_c::preParse() {
    u32 max = getCapacity();
    const wchar_t *p = getBuffer();
    u32 pos = 0;
    while (true) {
        u32 n;
        if (*p == 0x1A) {
            u8 group = getTagGroup(p);
            u8 len = getTagLength(p);
            if (group == 4) {
                n = preParseWordTag(pos);
            } else {
                n = len;
            }
        } else if (*p == 0) {
            n = -1;
        } else {
            n = 1;
        }
        if (n == -1 || pos + n > max) {
            break;
        }
        pos += n;
        p += n;
    }
}

// 80157F30: replaces the tags in the buffer
void Word_c::parse() {
    preParse();
    u32 max = getCapacity();
    getTextLength(getBuffer(), 0);
    u32 pos = getParseStart();
    const wchar_t *p = getBuffer() + pos;
    while (true) {
        u32 n;
        if (*p == 0x1A) {
            n = procTag(pos);
        } else if (*p == 0) {
            n = procEnd(pos);
        } else if (*p == L'\n') {
            n = procNewLine(pos);
        } else {
            n = procChar(pos);
        }
        if (n == -1 || pos + n > max) {
            break;
        }
        pos += n;
        p += n;
    }
}

// 80158088
int Word_c::procTag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u8 group = getTagGroup(tag);
    getTagId(tag);
    u8 len = getTagLength(tag);
    static TagFunc sGroups[] = {
        &Word_c::procIconTag, &Word_c::procIconTag2, NULL, &Word_c::procTimeTag, &Word_c::procWordTag,
        &Word_c::procNameTag, &Word_c::procBlockTag, &Word_c::procGroup7Tag, NULL, NULL,
        NULL, NULL, &Word_c::procGrammarTag,
    };
    static TagFunc sGroupFF = &Word_c::procSystemTag;
    TagFunc func = NULL;
    func = group == 0xFF ? sGroupFF : sGroups[group];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 8015822C
int Word_c::getParseStart() {
    _1C = 0;
    clearCapitalize();
    return 0;
}

// 80158258
int Word_c::procNewLine(int pos) {
    return 1;
}

// 80158260
int Word_c::procEnd(int pos) {
    return -1;
}

// 80158268
int Word_c::procChar(int pos) {
    wchar_t *p = getBuffer() + pos;
    mPacchim.check(*p);
    if (_20 && toUpper(p) == TRUE) {
        _20 = 0;
    }
    return 1;
}

// 80476970
static const Word_c::TagFunc lbl_80476970[] = {
    &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon,
    &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon,
    &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon,
    &Word_c::insertIcon, &Word_c::insertIcon, &Word_c::insertIcon,
};

// 801582E8
int Word_c::procIconTag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476970[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476A24
static const Word_c::TagFunc lbl_80476A24[] = {
    &Word_c::insertIcon2, &Word_c::insertIcon2, &Word_c::insertIcon2, &Word_c::insertIcon2,
    &Word_c::insertIcon2, &Word_c::insertIcon2, &Word_c::insertIcon2, &Word_c::insertIcon2,
    &Word_c::insertIcon2, &Word_c::insertIcon2, &Word_c::insertIcon2,
};

// 801583B4
int Word_c::procIconTag2(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476A24[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476AA8
static const Word_c::TagFunc lbl_80476AA8[] = {
    &Word_c::procYear, &Word_c::procMonth, &Word_c::procDay, &Word_c::procHour,
    &Word_c::procMinute, &Word_c::procSecond, &Word_c::procNumber,        &Word_c::procNumber,
    &Word_c::procNumber,        &Word_c::procNumber,        &Word_c::procNumber,        &Word_c::procNumber,
    &Word_c::procNumber,        &Word_c::procRandomNumber,
};

// 80158480
int Word_c::procTimeTag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476AA8[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 8015854C
int Word_c::preParseWordTag(int pos) {
    static TagFunc sFuncs[] = {
        NULL,          NULL,          NULL,          NULL,          &Word_c::procDrink,
        &Word_c::procObject, &Word_c::procSweets, &Word_c::procSports, &Word_c::procMusic, &Word_c::procFood,
        &Word_c::procHobby, NULL,          NULL,          NULL,          NULL,
    };
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = sFuncs[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476B50
static const Word_c::TagFunc lbl_80476B50[] = {
    &Word_c::procWord, &Word_c::procWord, &Word_c::procWord, &Word_c::procWord, &Word_c::procWord,
    &Word_c::procWord, &Word_c::procWord, &Word_c::procWord, &Word_c::procWord, &Word_c::procWord,
    &Word_c::procWord, &Word_c::procUnit, &Word_c::procPlayerChar0, &Word_c::procPlayerChar1, &Word_c::procPlayerChar2,
};

// 801586B0
int Word_c::procWordTag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476B50[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 8015877C
int Word_c::procNameTag(int pos) {
    static TagFunc sFuncs[] = {
        &Word_c::procPlayer, &Word_c::procAnimal, &Word_c::procAnimalA, &Word_c::procAnimalB,         &Word_c::procAnimalParam,
        &Word_c::procTownName, &Word_c::procCatchphrase, &Word_c::procPlayerNickname, NULL,                  &Word_c::procWeekday,
        &Word_c::procImpression, NULL,          &Word_c::procPlayerStarSign, &Word_c::procAnimalStarSign,
    };
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = sFuncs[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 8015889C
int Word_c::procBlockTag(int pos) {
    static TagFunc sFuncs[] = {
        NULL, NULL, NULL, NULL, NULL, NULL, &Word_c::procGreeting, &Word_c::procGreetingEnd, &Word_c::procAnimalFlagBegin,
        &Word_c::procAnimalFlagEnd, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &Word_c::vfA0,
    };
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = sFuncs[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476C04
static const Word_c::TagFunc lbl_80476C04[] = {
    &Word_c::vfA4, &Word_c::vfA4, &Word_c::vfA4, &Word_c::vfA4, &Word_c::vfA4,
    &Word_c::vfA4, &Word_c::vfA4, &Word_c::vfA4, &Word_c::vfA4,
};

// 80158AA8
int Word_c::procGroup7Tag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476C04[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476C70
static const Word_c::TagFunc lbl_80476C70[] = {
    &Word_c::setInflect,        &Word_c::setDefiniteArticle, &Word_c::setNoArticle, &Word_c::setCapitalize,
    &Word_c::procPlayerGender,        &Word_c::procWordGender,        &Word_c::procCounterJP, &Word_c::procParticleKR,
    &Word_c::procAnimalGender,        &Word_c::setCapitalizeWords, &Word_c::procPlayerElision,        &Word_c::procAnimalElision,
    &Word_c::procAnimalAElision,        &Word_c::procAnimalBElision,        &Word_c::procAnimalParamElision,        &Word_c::procTownElision,
    &Word_c::procPlayerNicknameElision,        &Word_c::procWordElision,        &Word_c::procNumberPlural,        &Word_c::procCounterJP2,
};

// 80158B74
int Word_c::procGrammarTag(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = lbl_80476C70[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80158C40
int Word_c::procSystemTag(int pos) {
    static TagFunc sFuncs[] = {&Word_c::procColor, &Word_c::procScale, &Word_c::procRubyTag, NULL};
    const wchar_t *tag = getBuffer() + pos;
    u16 id = getTagId(tag);
    u8 len = getTagLength(tag);
    TagFunc func = sFuncs[id];
    if (func) {
        return (this->*func)(pos);
    }
    return len;
}

// 80476D60: button icon characters for tag group 0
static const wchar_t lbl_80476D60[] = {
    0xE06C, 0xE06D, 0xE06E, 0xE06F, 0xE037, 0xE038, 0xE039, 0xE03A,
    0xE03B, 0xE021, 0xE03D, 0xE01D, 0xE01E, 0xE01F, 0xE036, 0x0000,
};

// 80158D48
int Word_c::insertIcon(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u8 len = getTagLength(tag);
    u16 id = getTagId(tag);
    insert(&lbl_80476D60[id], 1, pos + len);
    removeTag(pos);
    return 1;
}

// 80476D80: button icon characters for tag group 1
static const wchar_t lbl_80476D80[] = {
    0xE041, 0xE042, 0xE043, 0xE046, 0xE044, 0xE045, 0xE047, 0xE048, 0xE04A, 0xE04B, 0xE049, 0x0000,
};

// 80158DF0
int Word_c::insertIcon2(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u8 len = getTagLength(tag);
    u16 id = getTagId(tag);
    insert(&lbl_80476D80[id], 1, pos + len);
    removeTag(pos);
    return 1;
}

// 80158E98
BOOL Word_c::capitalize() {
    return toUpper(getBuffer());
}

// 80158EC8
u32 Word_c::getCapacity() {
    return getBufferSize() / 2;
}

// 80158EF8
u32 Word_c::getTextLength(const wchar_t *str, int breakOnNewLine) {
    u32 max = getCapacity() - 1;
    return getStringLength(str, max, breakOnNewLine);
}

// 80158F40
u32 Word_c::getLength(int breakOnNewLine) {
    u32 max = getCapacity() - 1;
    return getStringLength(getBuffer(), max, breakOnNewLine);
}

// 80158FA4
BOOL Word_c::isSame(Word_c *other) {
    u32 len = getLength(0);
    u32 otherLen = other->getTextLength(other->getBuffer(), 0);
    if (len != otherLen) {
        return FALSE;
    }
    const wchar_t *buf = getBuffer();
    const wchar_t *otherBuf = other->getBuffer();
    return memcmp(otherBuf, buf, len * 2) == 0;
}

// 80159060
BOOL Word_c::getChar(wchar_t *out, u32 index) {
    const wchar_t *buf = getBuffer();
    if (index >= getLength(0)) {
        return FALSE;
    }
    memcpy(out, buf + index, 2);
    return TRUE;
}

// 801590F0
BOOL Word_c::set(const wchar_t *str, int arg) {
    clear();
    if (!writeText(str, 0, arg)) {
        return FALSE;
    }
    parse();
    return TRUE;
}

// 80159164
BOOL Word_c::copy(const Word_c *src, int breakOnNewLine) {
    clear();
    mInflect = src->mInflect;
    mPacchim = src->mPacchim;
    _18 = src->_18;
    _1C = src->_1C;
    _20 = src->_20 != 0;
    _21 = src->_21 != 0;
    _22 = src->_22 != 0;
    _23 = src->_23 != 0;
    if (!writeText(const_cast<Word_c *>(src)->getBuffer(), 0, breakOnNewLine)) {
        return FALSE;
    }
    parse();
    return TRUE;
}

// 8015926C
BOOL Word_c::appendText(const wchar_t *str, int breakOnNewLine) {
    if (!appendRaw(str, breakOnNewLine)) {
        return FALSE;
    }
    parse();
    return TRUE;
}

// 801592B4
BOOL Word_c::appendChar(wchar_t c) {
    wchar_t buf = c;
    if (!append(&buf, 1)) {
        return FALSE;
    }
    parse();
    return TRUE;
}

// 80159308
BOOL Word_c::appendWord(Word_c *other, int breakOnNewLine) {
    return appendText(other->getBuffer(), breakOnNewLine);
}

// 80159360
BOOL Word_c::writeText(const wchar_t *str, u32 pos, int breakOnNewLine) {
    return write(str, getTextLength(str, breakOnNewLine), pos);
}

// 801593BC
BOOL Word_c::appendRaw(const wchar_t *str, int breakOnNewLine) {
    return append(str, getTextLength(str, breakOnNewLine));
}

// 80159404
BOOL Word_c::write(const wchar_t *str, u32 len, u32 pos) {
    u32 max = getCapacity() - 1;
    wchar_t *buf = getBuffer();
    wchar_t *dst = buf + pos;
    wchar_t *end = buf + max;
    if (pos + len > max) {
        return FALSE;
    }
    memcpy(dst, str, len * 2);
    *end = 0;
    return TRUE;
}

// 80159498
BOOL Word_c::insert(const wchar_t *str, u32 len, u32 pos) {
    wchar_t *buf = getBuffer();
    u32 cur = getLength(0);
    u32 end = pos + len;
    u32 max = getCapacity() - 1;
    if (cur + len > max) {
        return FALSE;
    }
    if (end > max) {
        return FALSE;
    }
    if (pos > cur) {
        return FALSE;
    }
    wchar_t *src = buf + cur;
    wchar_t *dst = src + len;
    for (int i = 0; i <= cur - pos; i++) {
        *dst-- = *src--;
    }
    write(str, len, pos);
    return TRUE;
}

// 801595EC
BOOL Word_c::append(const wchar_t *str, u32 len) {
    getBuffer();
    u32 cur = getLength(0);
    if (cur + len > getCapacity() - 1) {
        return FALSE;
    }
    write(str, len, cur);
    return TRUE;
}

// 8015968C
BOOL Word_c::erase(u32 len, u32 pos) {
    wchar_t *buf = getBuffer();
    u32 max = getCapacity() - 1;
    u32 end = len + pos;
    u32 cur = getLength(0);
    if (end > cur) {
        return FALSE;
    }
    int rest = cur - end;
    wchar_t *src = buf + end;
    wchar_t *dst = buf + pos;
    if (end > max) {
        return FALSE;
    }
    for (int i = 0; i < rest; i++) {
        *dst++ = *src++;
    }
    for (int i = 0; i < len; i++) {
        *dst++ = 0;
    }
    return TRUE;
}

// 80159848
int Word_c::removeTag(int pos) {
    const wchar_t *p = getBuffer() + pos;
    if (*p == 0x1A) {
        u8 len = getTagLength(p);
        erase(len, pos);
    }
    return 0;
}

// 801598B8
int Word_c::setTagWord(int pos, Word_c *word) {
    const wchar_t* str;

    mPacchim.check(word->mPacchim.mLastChar);
    getTagLength(getBuffer() + pos);
    BOOL hasArticle = FALSE;
    u32 n = 0;
    dString::Word_c article;
    if (_1C == 0) {
        if (word->mInflect.mIndefArticle != 0) {
            hasArticle = fn_8016AE68(&article, word->mInflect.mIndefArticle, "sys_STRING/STR_Article");
        }
    } else if (_1C == 1) {
        if (word->mInflect.mDefArticle != 0) {
            hasArticle = fn_8016AE68(&article, word->mInflect.mDefArticle, "sys_STRING/STR_Article");
        }
    }
    removeTag(pos);
    if (hasArticle) {
        str = static_cast<Word_c &>(article).getBuffer();
        n = insertWord(pos, str, article.getLength(0), TRUE);
    }
    str = word->getBuffer();
    n += insertWord(pos + n, str, word->getLength(0), FALSE);
    _1C = 0;
    return n;
}

// 80159A30
u32 Word_c::insertWord(u32 pos, const wchar_t *str, int len, BOOL isArticle) {
    wchar_t *p = getBuffer() + pos;
    insert(str, len, pos);
    if (_21) {
        BOOL capNext = TRUE;
        for (int i = 0; i < len; i++, p++) {
            if (capNext) {
                if (toUpper(p)) {
                    capNext = FALSE;
                }
            } else if (*p == L' ') {
                capNext = TRUE;
            }
        }
    } else if (_20) {
        toUpper(p);
    }
    if (isArticle) {
        _20 = 0;
    } else {
        clearCapitalize();
    }
    return len;
}

// 80159B2C
int Word_c::procYear(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setNumber4(&word, time.year);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 80159C00
int Word_c::procMonth(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setMonthName(&word, time.month);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 80159CD4
int Word_c::procDay(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setDayName(&word, time.mday);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 80159DA8
int Word_c::procHour(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    int hour = to12Hour(time.hour);
    dString::Word_c word;
    setNumber2(&word, hour);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 80159E90
int Word_c::procMinute(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setMinute(&word, time.min);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 80159F64
int Word_c::procSecond(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setSecond(&word, time.sec);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 8015A038
int Word_c::procNumber(int pos) {
    return removeTag(pos);
}

// 8015A03C
int Word_c::procRandomNumber(int pos) {
    return removeTag(pos);
}

// 804EFB48: {male, female} string groups
static const char *lbl_804EFB48[][2] = {
    {"sys_STRING/STR_Drink", "sys_STRING/STR_Drink"},
    {"sys_STRING/STR_ObjectM", "sys_STRING/STR_ObjectF"},
    {"sys_STRING/STR_Sweets", "sys_STRING/STR_Sweets"},
    {"sys_STRING/STR_Sports", "sys_STRING/STR_Sports"},
    {"sys_STRING/STR_Music", "sys_STRING/STR_Music"},
    {"sys_STRING/STR_Food", "sys_STRING/STR_Food"},
    {"sys_STRING/STR_HobbyM", "sys_STRING/STR_HobbyF"},
};

// 8015A040
const char *getStringGroup(int kind, u8 gender) {
    return lbl_804EFB48[kind][gender];
}

// 8015A05C
const char *Word_c::getGenderedGroup(int kind, dAnmPersonalID_c *id) {
    u8 gender = id->getGender(1);
    return getStringGroup(kind, gender);
}

// 8015A0A0
int Word_c::procWord(int pos) {
    return removeTag(pos);
}

// 8015A0A4
int Word_c::procDrink(int pos) {
    return removeTag(pos);
}

// 8015A0A8
int Word_c::procObject(int pos) {
    return removeTag(pos);
}

// 8015A0AC
int Word_c::procSweets(int pos) {
    return removeTag(pos);
}

// 8015A0B0
int Word_c::procSports(int pos) {
    return removeTag(pos);
}

// 8015A0B4
int Word_c::procMusic(int pos) {
    return removeTag(pos);
}

// 8015A0B8
int Word_c::procFood(int pos) {
    return removeTag(pos);
}

// 8015A0BC
int Word_c::procHobby(int pos) {
    return removeTag(pos);
}

// 8015A0C0
int Word_c::procUnit(int pos) {
    return removeTag(pos);
}

// 8015A0C4
int Word_c::procAnimalUnit(int pos, void *obj, dAnmPersonalID_c *id) {
    getTagLength(getBuffer() + pos);
    u16 index = fn_800F3F84((const dPersonalID_c *)obj, id->getLooks(1), NULL);
    if (index != 0) {
        dString::Word_c word(index, "sys_STRING/STR_Unit");
        _18 = 0;
        return setTagWord(pos, &word);
    }
    return removeTag(pos);
}

// 8015A1AC
int Word_c::procPlayerChar0(int pos) {
    return removeTag(pos);
}

// 8015A1B0
int Word_c::procPlayerChar1(int pos) {
    return removeTag(pos);
}

// 8015A1B4
int Word_c::procPlayerChar2(int pos) {
    return removeTag(pos);
}

// 8015A1B8
int Word_c::procPlayer(int pos) {
    return removeTag(pos);
}

// 8015A1BC
int Word_c::procPlayerName(int pos, const dPersonalID_c *pid) {
    dHmnName::Word_c name;
    pid->setWord(&name);
    _18 = 0;
    return setTagWord(pos, &name);
}

// 8015A248
int Word_c::procAnimal(int pos) {
    return removeTag(pos);
}

// 8015A24C
int Word_c::procAnimalName(int pos, dAnmPersonalID_c *id) {
    dHmnName::Word_c name;
    id->setWord(&name, LANGUAGE_NUM);
    _18 = 0;
    return setTagWord(pos, &name);
}

// 8015A2DC
int Word_c::procAnimalA(int pos) {
    return removeTag(pos);
}

// 8015A2E0
int Word_c::procAnimalB(int pos) {
    return removeTag(pos);
}

// 8015A2E4
int Word_c::procAnimalParam(int pos) {
    return removeTag(pos);
}

// 8015A2E8
int Word_c::procTownName(int pos) {
    dLandNameWord_c name;
    dSaveData_c::getTown()->mLandID.setWord(&name);
    _18 = 0;
    return setTagWord(pos, &name);
}

// 8015A370
int Word_c::procCatchphrase(int pos) {
    return removeTag(pos);
}

// 8015A374
int Word_c::procPlayerNickname(int pos) {
    return removeTag(pos);
}

// 8015A378
int Word_c::procWeekday(int pos) {
    dTime_c time = *dTime_c::getCurrent();
    dString::Word_c word;
    setWeekdayName(&word, time.wday, FALSE);
    _18 = 1;
    return setTagWord(pos, &word);
}

// 8015A450
int Word_c::procImpression(int pos) {
    return removeTag(pos);
}

// 8015A454
int Word_c::procPlayerStarSign(int pos) {
    return removeTag(pos);
}

// 8015A458
int Word_c::procAnimalStarSign(int pos) {
    return removeTag(pos);
}

// 8015A45C
int Word_c::procGreeting(int pos) {
    return removeTag(pos);
}

// 8015A460
int Word_c::procGreetingEnd(int pos) {
    return removeTag(pos);
}

// 8015A464
int Word_c::procAnimalFlagBegin(int pos) {
    return removeTag(pos);
}

// 8015A468
int Word_c::procAnimalFlagEnd(int pos) {
    return removeTag(pos);
}

// 8015A46C
int Word_c::vfA0(int pos) {
    return removeTag(pos);
}

// 8015A470
int Word_c::vfA4(int pos) {
    return removeTag(pos);
}

// 8015A474
int Word_c::setInflect(int pos) {
    return removeTag(pos);
}

// 8015A478
int Word_c::setDefiniteArticle(int pos) {
    _1C = 1;
    return removeTag(pos);
}

// 8015A484
int Word_c::setNoArticle(int pos) {
    _1C = 2;
    return removeTag(pos);
}

// 8015A490
int Word_c::setCapitalize(int pos) {
    _20 = 1;
    return removeTag(pos);
}

// 8015A49C
int Word_c::procPlayerGender(int pos) {
    return removeTag(pos);
}

// 8015A4A0
int Word_c::procByPlayerGender(int pos, const dPersonalID_c *pid) {
    return procSelect2(pos, pid->player.mGender);
}

// 8015A4A8
int Word_c::procSelect2(int pos, int select) {
    const wchar_t *tag = getBuffer() + pos;
    const wchar_t *str0 = (const wchar_t *)getTagParams(tag);
    u8 len = getTagLength(tag);
    u32 n0 = getTextLength(str0, 0);
    const wchar_t *str1 = str0 + (n0 + 1);
    u32 n1 = (len - 3) - (n0 + 1);
    u32 n;
    if (select == 0) {
        insert(str0, n0, pos + len);
        n = n0;
    } else {
        insert(str1, n1, pos + len);
        n = n1;
    }
    removeTag(pos);
    return n;
}

// 8015A580
int Word_c::procWordGender(int pos) {
    return removeTag(pos);
}

// 8015A584
int Word_c::procByWordGender(int pos, const Word_c *word) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    u32 skip;
    const wchar_t *str0;
    u32 n0;
    const wchar_t *str = (const wchar_t *)(params + 1);
    str0 = str;
    n0 = getTextLength(str, 0);
    skip = n0 + 1;
    str += skip;
    const wchar_t *str1 = str;
    u32 n1 = getTextLength(str, 0);
    skip += n1 + 1;
    const wchar_t *str2 = str + (n1 + 1);
    u32 n2 = (len - 4) - skip;
    u32 n = 0;
    if (word->mInflect.mGender == GENDER_MALE && str0 != NULL) {
        n = insertWord(pos + len, str0, n0, FALSE);
    } else if (word->mInflect.mGender == GENDER_FEMALE && str1 != NULL) {
        n = insertWord(pos + len, str1, n1, FALSE);
    } else if (word->mInflect.mGender == GENDER_OTHER && str2 != NULL) {
        n = insertWord(pos + len, str2, n2, FALSE);
    }
    removeTag(pos);
    return n;
}

// 8015A6EC
int Word_c::procCounterJP(int pos) {
    if (getLanguage() != LANGUAGE_JP) {
        return removeTag(pos);
    }
    const wchar_t *p = getBuffer() + pos;
    int last = 0;
    int count = 0;
    for (u32 i = 0; i < pos; i++, p--) {
        if (isDigit(*p)) {
            if (count == 0) {
                last = *p - '0';
            }
            count++;
        } else if (count != 0) {
            break;
        }
    }
    if (count != 0) {
        dString::Word_c word;
        setCounter(&word, last, count);
        _18 = 1;
        return setTagWord(pos, &word);
    }
    return removeTag(pos);
}

// 80476D98: Korean particles after a final consonant
static const wchar_t lbl_80476D98[][4] = {
    {0xC740, 0x0000}, {0xC744, 0x0000}, {0xC774, 0x0000}, {0xC544, 0x0000}, {0xC774, 0x0000},
    {0xACFC, 0x0000}, {0xC73C, 0xB85C}, {0xC758, 0x0000}, {0xC774, 0xAD70}, {0xC5D0, 0xAC8C},
    {0xB3C4, 0x0000}, {0xC774, 0xB77C, 0xB294}, {0xC774, 0xB77C, 0xACE0}, {0xC774, 0xB77C, 0xB124},
    {0xC774, 0xB791},
};

// 80476E10: Korean particles after a vowel
static const wchar_t lbl_80476E10[][4] = {
    {0xB294, 0x0000}, {0xB97C, 0x0000}, {0xAC00, 0x0000}, {0xC57C, 0x0000}, {0x0000, 0x0000},
    {0xC640, 0x0000}, {0xB85C, 0x0000}, {0xC758, 0x0000}, {0xAD70, 0x0000}, {0xC5D0, 0xAC8C},
    {0xB3C4, 0x0000}, {0xB77C, 0xB294}, {0xB77C, 0xACE0}, {0xB77C, 0xB124}, {0xB791, 0x0000},
};

// 8015A810
int Word_c::procParticleKR(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    u8 type = 0;
    memcpy(&type, params, 1);
    const wchar_t *particle;
    u16 last = mPacchim.mLastChar;
    s32 numberWord = _18;
    // (last - 0xAC00) % 28 is the syllable's final consonant (0 = none)
    if (last != 0 && (last - 0xAC00) % 28 != 0) {
        if (numberWord == 1) {
            // a final rieul (8) takes the vowel form of the particle
            if (type == 6 && (last - 0xAC00) % 28 == 8) {
                particle = lbl_80476E10[type];
            } else {
                particle = lbl_80476D98[type];
            }
        } else {
            particle = lbl_80476D98[type];
        }
    } else {
        particle = lbl_80476E10[type];
    }
    u32 n = insertWord(pos + len, particle, getTextLength(particle, 0), FALSE);
    removeTag(pos);
    return n;
}

// 8015A98C
int Word_c::procAnimalGender(int pos) {
    return removeTag(pos);
}

// 8015A990
int Word_c::procByAnimalGender(int pos, dAnmPersonalID_c *id) {
    u8 gender = id->getGender(0);
    return procSelect2(pos, gender);
}

// 8015A9E4
int Word_c::setCapitalizeWords(int pos) {
    _21 = 1;
    return removeTag(pos);
}

// 8015A9F0
int Word_c::procElisionPlayer(int pos, const dPersonalID_c *pid) {
    dHmnName::Word_c name;
    pid->setWord(&name);
    return procElisionWord(pos, 0, &name);
}

// 8015AA6C
int Word_c::procElisionAnimal(int pos, dAnmPersonalID_c *id) {
    dHmnName::Word_c name;
    id->setWord(&name, LANGUAGE_NUM);
    return procElisionWord(pos, 0, &name);
}

// 8015AAEC
int Word_c::procElisionWord(int pos, int offset, Word_c *word) {
    u8 len = getTagLength(getBuffer() + pos);
    const wchar_t *next = getBuffer() + pos + len;
    if (*next == 0x1A) {
        u8 group = getTagGroup(next);
        u16 id = getTagId(next);
        if (group == 12 && id == 2) {
            _1C = 2;
        }
    }
    dString::Word_c article;
    const wchar_t *first = word->getBuffer();
    if (_1C == 0) {
        if (word->mInflect.mIndefArticle != 0) {
            fn_8016AE68(&article, word->mInflect.mIndefArticle, "sys_STRING/STR_Article");
            first = static_cast<Word_c &>(article).getBuffer();
        }
    } else if (_1C == 1) {
        if (word->mInflect.mDefArticle != 0) {
            fn_8016AE68(&article, word->mInflect.mDefArticle, "sys_STRING/STR_Article");
            first = static_cast<Word_c &>(article).getBuffer();
        }
    }
    return procElision(pos, offset, *first);
}

// 8015AC64
int Word_c::procElision(int pos, int offset, wchar_t first) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    const wchar_t *str0 = (const wchar_t *)(params + offset);
    u32 n0 = getTextLength(str0, 0);
    const wchar_t *str1 = str0 + (n0 + 1);
    u32 n1 = (len - (offset + 3)) - (n0 + 1);
    u32 n = !isVowelSound(first) ? insertWord(pos + len, str0, n0, FALSE) : insertWord(pos + len, str1, n1, FALSE);

    removeTag(pos);
    return n;
}

// 8015AD68
int Word_c::procPlayerElision(int pos) {
    return removeTag(pos);
}

// 8015AD6C
int Word_c::procAnimalElision(int pos) {
    return removeTag(pos);
}

// 8015AD70
int Word_c::procAnimalAElision(int pos) {
    return removeTag(pos);
}

// 8015AD74
int Word_c::procAnimalBElision(int pos) {
    return removeTag(pos);
}

// 8015AD78
int Word_c::procAnimalParamElision(int pos) {
    return removeTag(pos);
}

// 8015AD7C
int Word_c::procTownElision(int pos) {
    dLandNameWord_c name;
    dSaveData_c::getTown()->mLandID.setWord(&name);
    return procElision(pos, 0, *static_cast<Word_c &>(name).getBuffer());
}

// 8015AE0C
int Word_c::procPlayerNicknameElision(int pos) {
    return removeTag(pos);
}

// 8015AE10
int Word_c::procWordElision(int pos) {
    return removeTag(pos);
}

// 8015AE14
int Word_c::procNumberPlural(int pos) {
    return removeTag(pos);
}

// 8015AE18
int Word_c::procPlural(int pos, Word_c *word) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    u8 type = 0;
    memcpy(&type, params, 1);
    if (type >= 10) {
        return removeTag(pos);
    }
    const wchar_t *str;
    u32 skip;
    const wchar_t *str0;
    u32 n0;
    const wchar_t *str1;
    u32 n1;
    const wchar_t *str2;
    u32 n2;
    str = (const wchar_t *)(params + 1);
    str0 = str;
    n0 = getTextLength(str, 0);
    skip = n0 + 1;
    str += skip;
    str1 = str;
    n1 = getTextLength(str, 0);
    skip += n1 + 1;
    str2 = str + (n1 + 1);
    n2 = (len - 4) - skip;
    u32 n;
    if (word->getTextLength(word->getBuffer(), 0) == 1) {
        switch (*word->getBuffer()) {
        case '0':
            n = insertWord(pos + len, str0, n0, FALSE);
            break;
        case '1':
            n = insertWord(pos + len, str1, n1, FALSE);
            break;
        default:
            n = insertWord(pos + len, str2, n2, FALSE);
            break;
        }
    } else {
        n = insertWord(pos + len, str2, n2, FALSE);
    }
    removeTag(pos);
    return n;
}

// 8015B008
int Word_c::procCounterJP2(int pos) {
    if (getLanguage() != LANGUAGE_JP) {
        return removeTag(pos);
    }
    const wchar_t *p = getBuffer() + pos;
    int last = 0;
    int count = 0;
    for (u32 i = 0; i < pos; i++, p--) {
        if (isDigit(*p)) {
            if (count == 0) {
                last = *p - '0';
            }
            count++;
        } else if (count != 0) {
            break;
        }
    }
    if (count != 0) {
        dString::Word_c word;
        setCounter2(&word, last, count);
        _18 = 1;
        return setTagWord(pos, &word);
    }
    return removeTag(pos);
}

// 8015B12C
int Word_c::procColor(int pos) {
    return removeTag(pos);
}

// 8015B130
int Word_c::procScale(int pos) {
    return removeTag(pos);
}

// 8015B134
int Word_c::procRubyTag(int pos) {
    return procRubyJP(pos);
}

// 8015B138: furigana: the reading replaces the base text when kana mode is on
int Word_c::procRubyJP(int pos) {
    if (getLanguage() == LANGUAGE_JP) {
        const wchar_t *tag = getBuffer() + pos;
        const u8 *params = getTagParams(tag);
        u8 len = getTagLength(tag);
        u8 baseLen = 0;
        memcpy(&baseLen, params, sizeof(baseLen));
        const wchar_t *reading = (const wchar_t *)(params + 1);
        int n = len - 4;
        if (fn_8010D944() == 1) {
            if (n != 0) {
                insert(reading, n, len + pos + baseLen);
                erase(len + baseLen, pos);
            } else {
                n = len;
            }
        } else {
            n = len;
        }
        return n;
    }
    return removeTag(pos);
}

// 8015B240
int Word_c::procRuby(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    u8 baseLen = 0;
    memcpy(&baseLen, params, 1);
    const wchar_t *reading = (const wchar_t *)(params + 1);
    int n = len - 4;
    if (fn_8010D944() == 1) {
        insert(reading, n, len + pos + baseLen);
        erase(len + baseLen, pos);
    } else {
        removeTag(pos);
        n = baseLen;
    }
    return n;
}

// 8015B324
BOOL Word_c::fill(wchar_t c, u32 len, u32 pos) {
    wchar_t ch = c;
    u32 max = getCapacity() - 1;
    if (pos > max) {
        return FALSE;
    }
    if (len + pos > max) {
        return FALSE;
    }
    wchar_t *p = getBuffer() + pos;
    for (u32 i = 0; i < len; p++, i++) {
        memcpy(p, &ch, sizeof(wchar_t)); // cursed way to do operator=, huh Nintendo?
    }
    return TRUE;
}

// 8015B3DC
BOOL Word_c::censor() {
    return fill('!', getLength(0), 0);
}

// 8015B420: picks a random string from a group, avoiding the ones in used
int Word_c::procRandomWord(int pos, int kind, dAnmPersonalID_c *id, u16 *used) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = getTagParams(tag);
    u8 len = getTagLength(tag);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    const char *group = getGenderedGroup(kind, id);
    u16 count = fn_8016AF68(group) - 1;
    u16 index = used[slot];
    if (index == 0) {
        int num = count >= 20 ? 20 : count;
        BOOL hasFree = FALSE;
        for (int i = 0; i < num; i++) {
            if (used[i] == 0) {
                hasFree = TRUE;
                break;
            }
        }
        if (hasFree) {
            index = fn_8016AFD4(group);
            for (int i = 0; i < 20; i++) {
                if (index == used[i]) {
                    index++;
                    if (index > count) {
                        index = 1;
                    }
                    i = 0;
                }
            }
        } else {
            index = 1;
        }
    }
    used[slot] = index;
    return len;
}

// 8015B588
void Word_c::clearCapitalize() {
    _21 = 0;
    _20 = 0;
}

// 8015B598: Inflect_c::~Inflect_c (d_script.hpp)
// 8015B5D8: Pacchim_c::~Pacchim_c (d_script.hpp)



} // namespace dScript
