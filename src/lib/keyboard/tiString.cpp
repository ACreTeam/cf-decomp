#include <lib/textinput/tiString.h>
#include <lib/textinput/tiUtil.h>
#include <wchar.h>

namespace textinput {
namespace tistring {

void StringBase::create(MEMAllocator *allocator) {
    mpAllocator = allocator;
    mpszString = (wchar_t *)MEMAllocFromAllocator(
        allocator, muMaxLength * sizeof(wchar_t));
    wmemset(mpszString, 0, muMaxLength);
    muLength = 0;
    mpszTmpString = (wchar_t *)MEMAllocFromAllocator(
        allocator, muMaxLength * sizeof(wchar_t));
    wmemset(mpszTmpString, 0, muMaxLength);
}

StringBase::~StringBase() {
    MEMFreeToAllocator(mpAllocator, mpszString);
    MEMFreeToAllocator(mpAllocator, mpszTmpString);
}

void StringBase::pushBack(wchar_t ch) {
    wchar_t str[16];
    str[0] = ch;
    str[1] = 0;
    wcsncat(mpszString, str, wcslen(str));
    muLength++;
}

void StringBase::popBack() {
    if (muLength != 0) {
        muLength--;
    }
    mpszString[muLength] = 0;
}

bool StringBase::append(const wchar_t *string) {
    u32 len = wcslen(string);
    if (len + getLength() > muMaxLength) {
        return false;
    }
    if (string == NULL) {
        return false;
    }
    while (*string != 0) {
        mpszString[muLength] = *string;
        string++;
        muLength++;
    }
    mpszString[muLength] = 0;
    return true;
}

bool StringBase::insert(u16 index, const wchar_t *string) {
    u32 len = wcslen(string);
    if ((u16)len + getLength() > muMaxLength) {
        return false;
    }
    if (string == NULL || index > getLength()) {
        return false;
    }
    wmemset(mpszTmpString, 0, muMaxLength);
    wcsncpy(mpszTmpString, &mpszString[index], getLength() - index);
    muLength = index;
    mpszString[index] = 0;
    append(string);
    append(mpszTmpString);
    return true;
}

void StringBase::remove(u16 index, u16 num) {
    if (getLength() != 0 && index <= getLength()) {
        if (index + num >= getLength()) {
            num = getLength() - index;
        }
        wmemset(mpszTmpString, 0, muMaxLength);
        wcsncpy(mpszTmpString, &mpszString[index] + num,
                getLength() - (index + num));
        muLength = index;
        mpszString[index] = 0;
        append(mpszTmpString);
    }
}

void StringBase::replace(u16 index, u16 num, const wchar_t *string) {
    if (index + num >= getLength()) {
        num = getLength() - index;
    }
    remove(index, num);
    insert(index, string);
}

void StringBase::set(const wchar_t *string) {
    clear();
    append(string);
}

void StringBase::setAt(u16 index, wchar_t ch) {
    if (muMaxLength > index) {
        mpszString[index] = ch;
    }
}

void StringBase::setLength(u16 length) {
    for (u16 i = length; i < muLength; i++) {
        setAt(i, 0);
    }
    muLength = length;
}

void StringBase::clear() {
    wmemset(mpszString, 0, muMaxLength);
    muLength = 0;
}

wchar_t StringBase::getLastWChar() {
    int last = muLength - 1;
    if (last >= 0) {
        return mpszString[last];
    }
    return 0;
}

void Decolated::inputChar(wchar_t ch) {
    if (ch == 0xFFFE) {
        return;
    }

    wchar_t kana[5];
    kana[0] = kana[1] = kana[2] = kana[3] = kana[4] = 0;
    u16 num;
    if (meTranslateMode != TM_Direct) {
        mwcKanaBuffer[0] = mwcKanaBuffer[1] = mwcKanaBuffer[2] =
            mwcKanaBuffer[3] = mwcKanaBuffer[4] = 0;
        u8 put;
        if (ch == L'\n' && KPRLookAhead(&mKanaQueue, NULL, 0) != 0) {
            put = KPRPutChar(&mKanaQueue, 0xFFFF);
        } else {
            put = KPRPutChar(&mKanaQueue, util::toWLower(ch));
        }
        for (u8 i = 0; i < put; i++) {
            kana[i] = KPRGetChar(&mKanaQueue);
        }
        KPRLookAhead(&mKanaQueue, mwcKanaBuffer, 5);
        num = put;
    } else {
        kana[0] = ch;
        num = 1;
        kana[1] = 0;
    }

    setCandidate(0);
    u32 start, end;
    getSelected(start, end);
    if (start == end) {
        insert(muCursorPos, kana);
        muCursorPos += num;
    } else {
        replace(start, end - start, kana);
        muCursorPos = start + num;
    }
    muAnchorPos = muCursorPos;
}

void Decolated::confirmKana() {
    inputChar('\n');
}

void Decolated::inputString(const wchar_t *string) {
    if (string != NULL) {
        for (const wchar_t *p = string; *p != 0; p++) {
            inputChar(*p);
        }
    }
}

void Decolated::inputString(const wchar_t *string, TranslateMode mode) {
    TranslateMode prevMode = meTranslateMode;
    setTranslateMode(mode);
    inputString(string);
    setTranslateMode(prevMode);
}

void Decolated::backSpace() {
    if (meTranslateMode != TM_Direct &&
        KPRLookAhead(&mKanaQueue, NULL, 0) != 0) {
        mKanaQueue.iCount--;
        u8 len = KPRLookAhead(&mKanaQueue, NULL, 0);
        mwcKanaBuffer[len] = 0;
        return;
    }

    u32 start, end;
    getSelected(start, end);
    if (start == end && start == 0) {
        return;
    }
    if (start == end) {
        if (hasCandidate()) {
            setCandidate(0);
        } else {
            if (muCursorPos != 0) {
                muCursorPos--;
            }
            remove(muCursorPos, 1);
        }
    } else {
        wchar_t empty = 0;
        replace(start, end - start, &empty);
        muCursorPos = start;
    }
    muAnchorPos = muCursorPos;
}

void Decolated::confirm(const wchar_t *string) {
    if (string != NULL) {
        insert(muCursorPos, string);
        muCursorPos += wcslen(string);
        muAnchorPos = muCursorPos;
    }
}

bool Decolated::moveCursorRight() {
    if (muCursorPos >= getLength()) {
        return !mbSustain;
    }
    muCursorPos++;
    if (!mbSustain) {
        muAnchorPos = muCursorPos;
    }
    return false;
}

bool Decolated::moveCursorLeft() {
    if (muCursorPos == 0) {
        return false;
    }
    muCursorPos--;
    if (!mbSustain) {
        muAnchorPos = muCursorPos;
    }
    return true;
}

void Decolated::setCursorPos(u32 pos) {
    muCursorPos = pos;
    if (!mbSustain) {
        muAnchorPos = pos;
    }
}

void Decolated::onSustain() {
    mbSustain = true;
    muAnchorPos = muCursorPos;
}

void Decolated::offSustain() {
    mbSustain = false;
}

void Decolated::getCursorPos(u32 *cursor, u32 *anchor) {
    *cursor = muCursorPos;
    *anchor = muAnchorPos;
}

bool Decolated::canBackSpace() {
    if (muCursorPos != 0 || (muCursorPos == 0 && muCursorPos != muAnchorPos)) {
        return true;
    }
    if (!isKanaFix()) {
        return true;
    }
    return false;
}

bool Decolated::deleteForward() {
    u32 pos = muCursorPos;
    if (pos >= getLength()) {
        return false;
    }
    remove(pos, 1);
    return true;
}

void Decolated::getSelected(u32 &start, u32 &end) {
    if (muCursorPos > muAnchorPos) {
        start = muAnchorPos;
        end = muCursorPos;
    } else {
        start = muCursorPos;
        end = muAnchorPos;
    }
}

wchar_t Decolated::getWCharAtCursor() {
    u32 start, end;
    getSelected(start, end);
    if (end == 0) {
        return 0;
    }
    return mpszString[end - 1];
}

void Decolated::replaceAtCursor(wchar_t ch) {
    u32 start, end;
    getSelected(start, end);
    if (end != 0) {
        mpszString[end - 1] = ch;
    }
}

bool Decolated::isDakuten() {
    return util::KBD_IsDakuten(getWCharAtCursor());
}

void Decolated::converDakuten() {
    replaceAtCursor(util::KBD_ConvertDakuten(getWCharAtCursor()));
}

bool Decolated::isHandaku() {
    return util::KBD_IsHandaku(getWCharAtCursor());
}

void Decolated::converHandaku() {
    replaceAtCursor(util::KBD_ConvertHandaku(getWCharAtCursor()));
}

void Decolated::convertAll() {
    replaceAtCursor(util::KBD_ConvertAll(getWCharAtCursor()));
}

bool Decolated::isSmall() {
    return util::KBD_IsSmall(getWCharAtCursor());
}

void Decolated::converSmall() {
    replaceAtCursor(util::KBD_ConvertSmall(getWCharAtCursor()));
}

bool Decolated::atTheBeginningOfASentence() {
    if (muCursorPos == 0) {
        return false;
    }
    if (getLength() < 1) {
        return false;
    }
    u32 start, end;
    getSelected(start, end);
    if (start != end) {
        return false;
    }
    switch (getWCharAtCursor()) {
    case '.':
    case '!':
    case '?':
        return true;
    }
    return false;
}

void Decolated::setTranslateMode(TranslateMode mode) {
    if (meTranslateMode == mode) {
        return;
    }
    if (!isKanaFix() && mode == TM_Direct) {
        confirmKana();
    }
    meTranslateMode = mode;
    switch (mode) {
    case TM_Hiragana:
        mKanaQueue.mode = KPR_MODE_JP_ROMAJI_HIRAGANA;
        break;
    case TM_Katakana:
        mKanaQueue.mode = KPR_MODE_JP_ROMAJI_KATAKANA;
        break;
    }
}

void Decolated::initKanaConverter() {
    KPRInitQueue(&mKanaQueue);
    KPRSetMode(&mKanaQueue, KPR_MODE_NONE);
    mwcKanaBuffer[0] = 0;
    switch (meTranslateMode) {
    case TM_Hiragana:
        mKanaQueue.mode = KPR_MODE_JP_ROMAJI_HIRAGANA;
        break;
    case TM_Katakana:
        mKanaQueue.mode = KPR_MODE_JP_ROMAJI_KATAKANA;
        break;
    }
}

void Decolated::setLength(u16 length) {
    StringBase::setLength(length);
    if (muCursorPos > length) {
        muCursorPos = muAnchorPos = length;
    }
}

} // namespace tistring
} // namespace textinput
