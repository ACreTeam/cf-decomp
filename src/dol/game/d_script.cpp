// dScript: BMG message resources, script words (Word_c) and the script bank.
// .text 8015593C..8015B680, compiled with -sym on. The main .text section ends
// with the static initializer at 80157CE0 (the global dScript::Bank_c at
// 805F2E94). Everything after it (80157D3C..) comes from
// include/game/game/d_script_word.inc: MWCC puts functions whose source is an
// included file in a second .text section that links after the main one. The
// data of both parts is interleaved, which is why this is one TU.
// Notes: notes/d_script.txt.
// First pass: every function is written for equivalence; matching has not started.
#include <game/game/d_script.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_nickname.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstring>
#include <cstdio>

namespace dScript {

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
u16 fn_800F3F84(void *obj, u8 looks, int);
extern u8 lbl_805FF398[];
}

// 8015593C: TRUE if c starts with a vowel sound (used to pick elided articles)
BOOL isVowelSound(int c) {
    switch (c) {
    case 0x40: case 0x41: case 0x45: case 0x48: case 0x49: case 0x4F: case 0x55:
    case 0x61: case 0x65: case 0x68: case 0x69: case 0x6F: case 0x75:
    case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6:
    case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
    case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:
    case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC:
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6:
    case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
    case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:
    case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC:
    case 0x152: case 0x153:
    case 0x386: case 0x388: case 0x389: case 0x38A: case 0x38C: case 0x38E: case 0x38F:
    case 0x390: case 0x391: case 0x395: case 0x397: case 0x399: case 0x39F: case 0x3A5:
    case 0x3A9: case 0x3AA: case 0x3AB: case 0x3AC: case 0x3AD: case 0x3AE: case 0x3AF:
    case 0x3B0: case 0x3B1: case 0x3B5: case 0x3B7: case 0x3B9: case 0x3BF: case 0x3C5:
    case 0x3C9: case 0x3CA: case 0x3CB: case 0x3CC: case 0x3CD: case 0x3CE:
    case 0x20AC:
        return TRUE;
    default:
        return FALSE;
    }
}

// 804767E8: sort keys for characters outside the ranges below
static const u16 lbl_804767E8[][2] = {
    {0x0386, 0x00BE}, {0x0388, 0x00BF}, {0x0389, 0x00C0}, {0x038A, 0x00C1}, {0x038C, 0x00C2},
    {0x038E, 0x00C3}, {0x038F, 0x00C4}, {0x03AA, 0x00C6}, {0x03AB, 0x00C7}, {0x03AC, 0x00C8},
    {0x03AD, 0x00C9}, {0x03AE, 0x00CA}, {0x03AF, 0x00CB}, {0x03CC, 0x00D0}, {0x03CD, 0x00D1},
    {0x03CE, 0x00D2}, {0x03CA, 0x00CE}, {0x03CB, 0x00CF}, {0x0390, 0x00C5}, {0x03B0, 0x00CC},
    {0x00FF, 0x009B}, {0x0178, 0x009B}, {0x00DF, 0x009A}, {0x30FC, 0x009C}, {0xFF5E, 0x009C},
    {0x00B7, 0x009D}, {0x30FB, 0x009D}, {0x003F, 0x009E}, {0xFF1F, 0x009E}, {0x0021, 0x009F},
    {0xFF01, 0x009F}, {0x3001, 0x00A0}, {0x3002, 0x00A1}, {0x0020, 0x00A2}, {0x3000, 0x00A2},
    {0x002E, 0x00A3}, {0x000A, 0x00A4}, {0x0000, 0x0000},
};

// 80155B70: sort key for a character (name sorting)
u16 getSortKey(wchar_t c) {
    u16 key = 0xD3;
    if (isDigit(c)) {
        key = c + 0x27;
    } else if (isHiragana(c)) {
        key = c - 0x3040;
    } else if (isKatakana(c)) {
        key = c - 0x30A0;
    } else if (isLowerAlpha(c)) {
        key = c;
    } else if (isUpperAlpha(c)) {
        key = c + 0x20;
    } else if (isFullwidthLower(c)) {
        key = c - 0x10000 + 0x120;
    } else if (isFullwidthUpper(c)) {
        key = c - 0x10000 + 0x140;
    } else if (isLatin1Lower(c)) {
        key = c - 0x65;
    } else if (isLatin1Upper(c)) {
        key = c - 0x45;
    } else if (isGreekLower(c)) {
        key = c - 0x30C;
    } else if (isGreekUpper(c)) {
        key = c - 0x2EC;
    } else if (isHangul(c)) {
        key = c;
    } else {
        for (int i = 0; i < ARRAY_SIZE(lbl_804767E8); i++) {
            if (c == lbl_804767E8[i][0]) {
                key = lbl_804767E8[i][1];
                break;
            }
        }
    }
    return key;
}

// 80476880: per-language script directories
static const char lbl_80476880[LANGUAGE_NUM][13] = {
    "/Script/JPN/", "/Script/ENG/", "/Script/SPA/", "/Script/FRA/", "/Script/ENG/",
    "/Script/SPA/", "/Script/FRA/", "/Script/ITA/", "/Script/GER/", "/Script/JPN/",
};

// 80155D38
const char *getScriptDir() {
    return lbl_80476880[getLanguage()];
}

// 80155D68: 0-9
BOOL isDigit(wchar_t c) {
    return sLib::isInRange(c, 0x30, 0x39) != 0;
}

// 80155D9C: hiragana
BOOL isHiragana(wchar_t c) {
    return sLib::isInRange(c, 0x3041, 0x3093) != 0;
}

// 80155DD0: katakana
BOOL isKatakana(wchar_t c) {
    return sLib::isInRange(c, 0x30A1, 0x30F6) != 0;
}

// 80155E04: A-Z
BOOL isUpperAlpha(wchar_t c) {
    return sLib::isInRange(c, 0x41, 0x5A) != 0;
}

// 80155E38: a-z
BOOL isLowerAlpha(wchar_t c) {
    return sLib::isInRange(c, 0x61, 0x7A) != 0;
}

// 80155E6C: A-Z or a-z
BOOL isAlpha(wchar_t c) {
    BOOL ret = FALSE;
    if (isUpperAlpha(c) || isLowerAlpha(c)) {
        ret = TRUE;
    }
    return ret;
}

// 80155EC4: fullwidth A-Z
BOOL isFullwidthUpper(wchar_t c) {
    return sLib::isInRange(c, 0xFF21, 0xFF3A) != 0;
}

// 80155EFC: fullwidth a-z
BOOL isFullwidthLower(wchar_t c) {
    return sLib::isInRange(c, 0xFF41, 0xFF5A) != 0;
}

// 80155F34: Latin-1 uppercase
BOOL isLatin1Upper(wchar_t c) {
    BOOL ret = FALSE;
    if (c != 0xD7 && sLib::isInRange(c, 0xC0, 0xDE)) {
        ret = TRUE;
    }
    return ret;
}

// 80155F80: Latin-1 lowercase
BOOL isLatin1Lower(wchar_t c) {
    BOOL ret = FALSE;
    if (c != 0xF7 && sLib::isInRange(c, 0xE0, 0xFE)) {
        ret = TRUE;
    }
    return ret;
}

// 80155FCC: Greek lowercase
BOOL isGreekLower(wchar_t c) {
    return sLib::isInRange(c, 0x3B1, 0x3C9) != 0;
}

// 80156000: Greek uppercase
BOOL isGreekUpper(wchar_t c) {
    return sLib::isInRange(c, 0x391, 0x3A9) != 0;
}

// 80156034: Hangul
BOOL isHangul(wchar_t c) {
    BOOL ret = FALSE;
    if (sLib::isInRange(c, 0x1100, 0x11F9) || sLib::isInRange(c, 0x3131, 0x318E) ||
        sLib::isInRange(c, 0xAC00, 0xD7A3)) {
        ret = TRUE;
    }
    return ret;
}

// 80476904: word separators
static const u16 lbl_80476904[] = {
    0x002C, 0x00B7, 0x30FB, 0x003F, 0xFF1F, 0x0021, 0xFF01,
    0x3001, 0x3002, 0x0020, 0x3000, 0x002E, 0x000A, 0x0000,
};

// 801560B8
BOOL isSeparator(wchar_t c) {
    for (int i = 0; i < ARRAY_SIZE(lbl_80476904); i++) {
        if (c == lbl_80476904[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80476920: {lowercase, uppercase} pairs not covered by the ranges
static const u16 lbl_80476920[][2] = {
    {0x03AC, 0x0386}, {0x03AD, 0x0388}, {0x03AE, 0x0389}, {0x03AF, 0x038A}, {0x03CC, 0x038C},
    {0x03CD, 0x038E}, {0x03CE, 0x038F}, {0x03CA, 0x03AA}, {0x03CB, 0x03AB}, {0x00FF, 0x0178},
};

// 8015616C: to uppercase; FALSE if c is not a letter
bool toUpper(wchar_t *c) {
    if (isFullwidthLower(*c) || isLowerAlpha(*c) || isLatin1Lower(*c) || (isGreekLower(*c) && *c != 0x3C2)) {
        *c -= 0x20;
        return TRUE;
    }
    if (*c >= 0x101 && *c <= 0x177 && *c % 2 == 1) {
        *c -= 1;
        return TRUE;
    }
    for (int i = 0; i < ARRAY_SIZE(lbl_80476920); i++) {
        if (*c == lbl_80476920[i][0]) {
            *c = lbl_80476920[i][1];
            return TRUE;
        }
        if (*c == lbl_80476920[i][1]) {
            return TRUE;
        }
    }
    if (*c >= 0x17A && *c <= 0x17E && *c % 2 == 0) {
        *c -= 1;
        return TRUE;
    }
    if (isFullwidthUpper(*c) || isUpperAlpha(*c) || isLatin1Upper(*c) || isGreekUpper(*c) ||
        (*c >= 0x100 && *c <= 0x176 && *c % 2 == 0) || *c == 0x178 ||
        (*c >= 0x179 && *c <= 0x17D && *c % 2 == 1)) {
        return TRUE;
    }
    return FALSE;
}

// 80156368: to lowercase; FALSE if c is not a letter
bool toLower(wchar_t *c) {
    if (isFullwidthUpper(*c) || isUpperAlpha(*c) || isLatin1Upper(*c) || isGreekUpper(*c)) {
        *c += 0x20;
        return TRUE;
    }
    if (*c >= 0x100 && *c <= 0x176 && *c % 2 == 0) {
        *c += 1;
        return TRUE;
    }
    for (int i = 0; i < ARRAY_SIZE(lbl_80476920); i++) {
        if (*c == lbl_80476920[i][1]) {
            *c = lbl_80476920[i][0];
            return TRUE;
        }
        if (*c == lbl_80476920[i][0]) {
            return TRUE;
        }
    }
    if (*c >= 0x179 && *c <= 0x17D && *c % 2 == 1) {
        *c += 1;
        return TRUE;
    }
    if (isFullwidthLower(*c) || isLowerAlpha(*c) || isLatin1Lower(*c) || isGreekLower(*c) ||
        (*c >= 0x101 && *c <= 0x177 && *c % 2 == 1) || *c == 0xFF ||
        (*c >= 0x17A && *c <= 0x17E && *c % 2 == 0)) {
        return TRUE;
    }
    return FALSE;
}

// 80156554
BOOL setNumber4(Word_c *word, int value) {
    return formatNumber(word, value, 4, NUM_FORMAT_PLAIN, 0);
}

// 80156564
BOOL setNumber2(Word_c *word, int value) {
    return formatNumber(word, value, 2, NUM_FORMAT_PLAIN, 0);
}

// 80156574
BOOL setMinute(Word_c *word, int value) {
    NumberFormat_e format = NUM_FORMAT_PLAIN;
    switch (getRegion()) {
    case 1:
    case 2:
        format = NUM_FORMAT_ZERO_PAD;
        break;
    }
    return formatNumber(word, value, 2, format, 0);
}

// 801565E8
BOOL setSecond(Word_c *word, int value) {
    NumberFormat_e format = NUM_FORMAT_PLAIN;
    switch (getRegion()) {
    case 1:
    case 2:
        format = NUM_FORMAT_ZERO_PAD;
        break;
    }
    return formatNumber(word, value, 2, format, 0);
}

// 8015665C
BOOL setNumber(Word_c *word, int value, int digits, NumberFormat_e format) {
    if (getLanguage() == LANGUAGE_DE) {
        return formatNumber(word, value, digits, format, '.');
    }
    return formatNumber(word, value, digits, format, 0);
}

// 801566E8: formats a number into word
// format: 0/1 plain, 2/3 space padded, 4/5 space padded at the end, 6/7 zero
// padded; odd formats add thousands separators, 9 picks by region.
BOOL formatNumber(Word_c *word, int value, int digits, NumberFormat_e format, int sepArg) {
    int sep = sepArg;
    if (sep == 0) {
        switch (getLanguage()) {
        case LANGUAGE_QC:
        case LANGUAGE_ES:
        case LANGUAGE_FR:
        case LANGUAGE_DE:
            sep = ' ';
            break;
        case LANGUAGE_IT:
            sep = '.';
            break;
        default:
            sep = ',';
            break;
        }
    }

    if (format == NUM_FORMAT_REGION) {
        switch (getRegion()) {
        case CONSOLE_REGION_NA:
        case CONSOLE_REGION_EU:
            format = NUM_FORMAT_PLAIN_SEP;
            break;
        case CONSOLE_REGION_JP:
        case CONSOLE_REGION_KR:
            format = NUM_FORMAT_PLAIN;
            break;
        default:
            format = NUM_FORMAT_PLAIN;
            break;
        }
    }

    char tmp[0x12];
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%d", value);
    if ((int)strlen(tmp) <= 4 || digits <= 4) {
        switch (getLanguage()) {
        case LANGUAGE_QC:
        case LANGUAGE_ES:
        case LANGUAGE_FR:
            if (format == NUM_FORMAT_PLAIN_SEP) {
                format = NUM_FORMAT_PLAIN;
            } else if (format == NUM_FORMAT_SPACE_PAD_SEP) {
                format = NUM_FORMAT_SPACE_PAD;
            } else if (format == NUM_FORMAT_SPACE_PAD_END_SEP) {
                format = NUM_FORMAT_SPACE_PAD_END;
            } else if (format == NUM_FORMAT_ZERO_PAD_SEP) {
                format = NUM_FORMAT_ZERO_PAD;
            }
            break;
        }
    }

    int absValue = value;
    if (value < 0) {
        absValue = -value;
    }

    wchar_t buf[0x12];
    memset(buf, 0, sizeof(buf));
    writeDigits(buf, absValue, digits);
    if (value < 0) {
        appendMinus(buf);
    }
    if (isSpacePadFormat(format)) {
        padSpacesFront(buf, digits);
    }
    if (isSpacePadEndFormat(format)) {
        padSpacesBack(buf, digits);
    }
    if (isZeroPadFormat(format)) {
        padZeros(buf, digits);
    }
    if (isSeparatorFormat(format)) {
        insertSeparators(buf, sep);
    }
    reverse(buf);
    if (!word->set(buf, 0)) {
        return FALSE;
    }
    return TRUE;
}

// 80156928: formats a decimal number into word
BOOL setDecimal(Word_c *word, int decimals, f64 value) {
    BOOL ret = TRUE;
    const char *p;
    wchar_t point;
    switch (getLanguage()) {
    case LANGUAGE_QC:
    case LANGUAGE_ES:
    case LANGUAGE_FR:
    case LANGUAGE_IT:
    case LANGUAGE_DE:
        point = ',';
        break;
    default:
        point = '.';
        break;
    }

    char tmp[0xD];
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%0.10f", value);
    int len = strlen(tmp);
    p = tmp;
    int afterPoint = 0;
    BOOL seenPoint = FALSE;
    for (int i = 0; i < len; p++, i++) {
        char ch = *p;
        if (ch == 0) {
            break;
        }
        if (ch == '-') {
            ret = (ret & word->appendChar('-')) != 0;
        } else if (ch == '.') {
            if (decimals == 0) {
                break;
            }
            ret = (ret & word->appendChar(point)) != 0;
            seenPoint = TRUE;
        } else {
            ret = (ret & word->appendChar((u16)ch)) != 0;
            if (seenPoint) {
                if (++afterPoint == decimals) {
                    break;
                }
            }
        }
    }
    return ret;
}

// 80156A8C: 24-hour to 12-hour clock where the language uses it
int to12Hour(int hour) {
    switch (getLanguage()) {
    case LANGUAGE_JP:
    case LANGUAGE_US:
    case LANGUAGE_EN:
    case LANGUAGE_KR:
        if (hour >= 12) {
            hour -= 12;
        }
        if (hour == 0) {
            hour = 12;
        }
        break;
    }
    return hour;
}

// 80156B00: AM/PM word
BOOL setAmPm(Word_c *word, int hour) {
    BOOL ret = TRUE;
    int idx;

    switch (getLanguage()) {
    case LANGUAGE_JP:
    case LANGUAGE_US:
    case LANGUAGE_MX:
    case LANGUAGE_QC:
    case LANGUAGE_EN:
    case LANGUAGE_KR:
        idx = 3;
        if (hour >= 12) {
            idx = 4;
        }

        ret = fn_8016AE68(word, idx, "sys_STRING/STR_Unit");
        break;
    default:
        word->clear();
        break;
    }
    return ret;
}

// 80156B98: weekday name (short form if abbrev)
BOOL setWeekdayName(Word_c *word, int day, BOOL abbrev) {
    return fn_8016AE68(word, day + (abbrev ? 7 : 0) + 1, "sys_STRING/STR_Week");
}

// 80156BC0: month name
BOOL setMonthName(Word_c *word, int month) {
    return fn_8016AE68(word, month + 1, "sys_STRING/STR_Month");
}

// 80156BD4: day of the month
BOOL setDayName(Word_c *word, int day) {
    return fn_8016AE68(word, day, "sys_STRING/STR_Day");
}

// 80476948
static const u16 lbl_80476948[] = {2, 2, 1, 2, 1, 1, 2, 1, 2, 1};

// 80156BE4: JP counter word (singular for 1)
BOOL setCounter(Word_c *word, int type, int count) {
    if (count == 1 && type == 0) {
        return fn_8016AE68(word, 1, "sys_STRING/STR_Unit");
    }
    return fn_8016AE68(word, lbl_80476948[type], "sys_STRING/STR_Unit");
}

// 8047695C
static const u16 lbl_8047695C[] = {0x10, 0x10, 0x11, 0x12, 0x11, 0x11, 0x10, 0x11, 0x11, 0x11};

// 80156C20
BOOL setCounter2(Word_c *word, int type, int count) {
    if (count == 1 && type == 0) {
        return fn_8016AE68(word, 0x11, "sys_STRING/STR_Unit");
    }
    return fn_8016AE68(word, lbl_8047695C[type], "sys_STRING/STR_Unit");
}

// 80156C5C: builds a nickname word; FALSE if the nickname is unusable
BOOL makeNickname(Word_c *out, const Word_c *name, u16 index) {
    dNicknameWord_c word;
    dString::Word_c nickname;
    // TODO: owner of this global (a Word_c at +0x12C4, a flag at +0x13B4)
    ((Word_c *)(lbl_805FF398 + 0x12C4))->copy(name, 0);
    lbl_805FF398[0x13B4] = 0;
    if (!fn_8016AE68(&nickname, index, "sys_STRING/STR_Nickname")) {
        return FALSE;
    }
    if (lbl_805FF398[0x13B4]) {
        return FALSE;
    }
    if (nickname.getLength(0) >= 9) {
        return FALSE;
    }
    word.set(static_cast<Word_c &>(nickname).getBuffer(), 0);
    if (word.isSame((Word_c *)(lbl_805FF398 + 0x12C4))) {
        return FALSE;
    }
    if (word.hasNoLetters()) {
        return FALSE;
    }
    if (out != NULL && !out->set(static_cast<Word_c &>(word).getBuffer(), 0)) {
        return FALSE;
    }
    return TRUE;
}

} // namespace dScript

// Word_c's code. It sits here in the original source: its data (PTMF tables,
// strings) comes between makeNickname's and loadBank's.
#include <game/game/d_script_word.inc>

namespace dScript {

// 80156E68: appends '-'
void appendMinus(wchar_t *buf) {
    int len = numLength(buf);
    buf[len] = '-';
}

// 80156EA0: writes value's digits in reverse order
void writeDigits(wchar_t *buf, int value, int digits) {
    if (value == 0) {
        *buf = '0';
        return;
    }

    wchar_t *p = buf;
    int n = value;
    for (int i = 0; i < digits && n != 0 && value != 0; i++) {
        int q = n / 10;
        *p++ = n - q * 10 + '0';
        n = q;
    }
}

// 80156F10: pads with spaces at the start
void padSpacesFront(wchar_t *buf, int width) {
    int pad = width - numLength(buf);
    shiftRight(buf, 0, pad);
    for (int i = pad - 1; i >= 0; i--) {
        buf[i] = ' ';
    }
}

// 80157038: pads with spaces at the end
void padSpacesBack(wchar_t *buf, int width) {
    int len = numLength(buf);
    for (int i = len; i < width; i++) {
        buf[i] = ' ';
    }
}

// 801571A0: pads with zeros at the end (the start, once reversed)
void padZeros(wchar_t *buf, int width) {
    int len = numLength(buf);
    for (int i = len; i < width; i++) {
        buf[i] = '0';
    }
}

// 80157308: length within an 18-character buffer
int numLength(const wchar_t *buf) {
    int i;
    for (i = 0; i < 18; i++) {
        if (buf[i] == 0) {
            break;
        }
    }
    return i;
}

// 801573B4: moves buf[pos..] right by n characters
void shiftRight(wchar_t *buf, int pos, int n) {
    if (n <= 0) {
        return;
    }
    for (int i = 17 - n; i >= pos; i--) {
        buf[i + n] = buf[i];
    }
}

// 80157580: reverses buf
void reverse(wchar_t *buf) {
    int len = numLength(buf);
    for (int i = 0, j = len - 1; i < len >> 1; i++, j--) {
        wchar_t tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
    }
}

// 8015771C: inserts sep after every third digit
void insertSeparators(wchar_t *buf, wchar_t sep) {
    wchar_t *p = buf;
    int i = 0;
    int count = 0;
    for (; *p != 0; p++, i++) {
        if (isDigit(*p)) {
            if (++count >= 3) {
                int next = i + 1;
                count = 0;
                if (next < 18 && isDigit(buf[next])) {
                    shiftRight(buf, i, 1);
                    buf[next] = sep;
                }
            }
        }
    }
}

// 801577C4
BOOL isSpacePadFormat(NumberFormat_e format) {
    return format == NUM_FORMAT_SPACE_PAD || format == NUM_FORMAT_SPACE_PAD_SEP;
}

// 801577E4
BOOL isSpacePadEndFormat(NumberFormat_e format) {
    return format == NUM_FORMAT_SPACE_PAD_END || format == NUM_FORMAT_SPACE_PAD_END_SEP;
}

// 80157804
BOOL isZeroPadFormat(NumberFormat_e format) {
    return format == NUM_FORMAT_ZERO_PAD || format == NUM_FORMAT_ZERO_PAD_SEP;
}

// 80157824: formats with thousands separators
BOOL isSeparatorFormat(NumberFormat_e format) {
    return format == NUM_FORMAT_PLAIN_SEP || format == NUM_FORMAT_SPACE_PAD_SEP || format == NUM_FORMAT_SPACE_PAD_END_SEP || format == NUM_FORMAT_ZERO_PAD_SEP;
}

// 8015784C
Res_c::Res_c(const void *data) : EGG::MsgRes(data) {}

// 80157888
Res_c::~Res_c() {}

// 801578E0
const wchar_t *Res_c::getMessage(int id) {
    return getMsg(id, 0);
}

// 801578E8
void *Res_c::getEntry(int id) {
    return getMsgEntry(id, 0);
}

// 801578F0
u16 Res_c::getCount() {
    if (mpInf1 == NULL) {
        return 0;
    }
    return *(const u16 *)((const u8 *)mpInf1 + 8);
}

// 8015790C: length in characters (a tag counts as its full size); 0 if it exceeds maxSize
int getStringLength(const wchar_t *text, u32 maxSize, int breakOnNewLine) {
    u32 len = 0;
    while (true) {
        u32 n;
        if (*text == 0x1A) {
            n = getTagLength(text);
        } else {
            if (*text == 0 || (breakOnNewLine && *text == L'\n')) {
                break;
            }
            n = 1;
        }
        if (maxSize != 0 && len + n > maxSize) {
            len = 0;
            break;
        }
        len += n;
        text += n;
    }
    return len;
}

// 801579C0
int getLineLength(const u16 *text, u32 maxSize) {
    return getStringLength((const wchar_t *)text, maxSize, 1);
}

// 801579C8: tag group
u8 getTagGroup(const wchar_t *tag) {
    u16 esc = 0;
    u8 group = 0;
    memcpy(&esc, tag, 2);
    memcpy(&group, (const u8 *)tag + 3, 1);
    return group;
}

// 80157A20: tag id
u16 getTagId(const wchar_t *tag) {
    u16 esc = 0;
    u16 id = 0;
    memcpy(&esc, tag, 2);
    memcpy(&id, (const u8 *)tag + 4, 2);
    return id;
}

// 80157A78: tag length in characters
u8 getTagLength(const wchar_t *tag) {
    u16 esc = 0;
    u8 size = 0;
    memcpy(&esc, tag, 2);
    memcpy(&size, (const u8 *)tag + 2, 1);
    return size / 2;
}

// 80157AD4: tag parameters
const u8 *getTagParams(const wchar_t *tag) {
    u16 esc = 0;
    memcpy(&esc, tag, 2);
    return (const u8 *)tag + 6;
}

// 80157B18
void Pacchim_c::check(wchar_t c) {
    if (c >= 0xAC00 && c <= 0xD7A3) {
        mLastChar = c;
    }
}

// 805F2E94
static Bank_c lbl_805F2E94;

// 80157B30
Bank_c *getBank() {
    return &lbl_805F2E94;
}

// 80157B3C: loads the language's script.arc
BOOL loadBank(Bank_c *bank, void *heap) {
    char path[0x50];
    sprintf(path, "%sscript.arc", getScriptDir());
    return bank->load(path, heap, 0);
}

// 80157BA0
void *getBmgFile(Bank_c *bank, const char *name) {
    char path[0x50];
    sprintf(path, "%s.bmg", name);
    return bank->getFile(path, NULL);
}

// 80157BEC: heap size for script.arc
u32 getBankHeapSize() {
    return 0x96000;
}

// 80157BF8
void Inflect_c::setGender(u8 gender) {
    mGender = gender;
}

// 80157C00
void Inflect_c::setIndefArticle(u8 article) {
    u16 count = fn_8016AF68("sys_STRING/STR_Article");
    if (article == 0xFF) {
        mIndefArticle = 0;
    } else if ((u32)article > count) {
        mIndefArticle = 0;
    } else {
        mIndefArticle = article;
    }
}

// 80157C70
void Inflect_c::setDefArticle(u8 article) {
    u16 count = fn_8016AF68("sys_STRING/STR_Article");
    if (article == 0xFF) {
        mDefArticle = 0;
    } else if ((u32)article > count) {
        mDefArticle = 0;
    } else {
        mDefArticle = article;
    }
}

// 80157CE0: __sinit (constructs lbl_805F2E94)

} // namespace dScript
