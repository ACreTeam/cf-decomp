#pragma once

// textinput::tistring: the edited strings (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <revolution/kpr.h>

namespace textinput {
namespace tistring {

class StringBase {
public:
    StringBase(u16 maxLength)
        : muMaxLength(maxLength), muLength(0), mpszString(NULL), mpszTmpString(NULL), mwcCandidate(0),
          mpAllocator(NULL) {}

    virtual ~StringBase();                                           // 0x08
    virtual void create(MEMAllocator *allocator);                    // 0x0C
    virtual void pushBack(wchar_t ch);                               // 0x10
    virtual void popBack();                                          // 0x14
    virtual void clear();                                            // 0x18
    virtual u16 getLength() const { return muLength; }               // 0x1C
    virtual bool append(const wchar_t *string);                      // 0x20
    virtual bool insert(u16 index, const wchar_t *string);           // 0x24
    virtual void remove(u16 index, u16 num);                         // 0x28
    virtual void replace(u16 index, u16 num, const wchar_t *string); // 0x2C
    virtual void set(const wchar_t *string);                         // 0x30
    virtual void setAt(u16 index, wchar_t ch);                       // 0x34
    virtual void setLength(u16 length);                              // 0x38
    virtual wchar_t *getWCString() const { return mpszString; }      // 0x3C
    virtual void setCandidate(wchar_t candidate) { mwcCandidate = candidate; } // 0x40
    virtual wchar_t getCandidate() const { return mwcCandidate; }    // 0x44
    virtual bool hasCandidate() const { return mwcCandidate != 0; }  // 0x48
    virtual wchar_t getLastWChar();                                  // 0x4C

protected:
    /* 0x04 */ u16 muMaxLength;
    /* 0x06 */ u16 muLength;
    /* 0x08 */ wchar_t *mpszString;
    /* 0x0C */ wchar_t *mpszTmpString;
    /* 0x10 */ wchar_t mwcCandidate;
    /* 0x14 */ MEMAllocator *mpAllocator;
}; // size 0x18

// A string edited with a cursor and kana composition.
class Decolated : public StringBase {
public:
    enum TranslateMode {
        TM_Direct,
        TM_Hiragana,
        TM_Katakana,
#if VERSION == VERSION_RUUK01_01
        TM_Hangul,
#endif
    };

    Decolated(u16 maxLength)
        : StringBase(maxLength), muCursorPos(0), muAnchorPos(0), mbSustain(false), meTranslateMode(TM_Direct) {
        initKanaConverter();
    }

    virtual ~Decolated() {} // 0x08

    virtual void clear() { // 0x18
        StringBase::clear();
        muCursorPos = 0;
        muAnchorPos = 0;
        mbSustain = false;
        initKanaConverter();
    }
    virtual void set(const wchar_t *string) { // 0x30
        StringBase::set(string);
        muAnchorPos = muCursorPos = getLength();
    }
    virtual void setLength(u16 length); // 0x38

    virtual void inputChar(wchar_t ch);                       // 0x50
    virtual void inputString(const wchar_t *string);          // 0x54
    virtual void deleteChar() {}                              // 0x58
    virtual void backSpace();                                 // 0x5C
    virtual void confirm(const wchar_t *string);              // 0x60
    virtual bool moveCursorRight();                           // 0x64
    virtual bool moveCursorLeft();                            // 0x68
    virtual void setCursorPos(u32 pos);                       // 0x6C
    virtual void onSustain();                                 // 0x70
    virtual void offSustain();                                // 0x74
    virtual bool isOnSustain() { return mbSustain; }          // 0x78
    virtual u32 getCursorPos() const { return muCursorPos; }  // 0x7C
    virtual void getCursorPos(u32 *cursor, u32 *anchor);      // 0x80
    virtual bool canBackSpace();                              // 0x84
    virtual bool deleteForward();                             // 0x88
    virtual void getSelected(u32 &start, u32 &end);           // 0x8C
    virtual wchar_t getWCharAtCursor();                       // 0x90
    virtual void replaceAtCursor(wchar_t ch);                 // 0x94
    virtual bool isDakuten();                                 // 0x98
    virtual void converDakuten();                             // 0x9C
    virtual bool isHandaku();                                 // 0xA0
    virtual void converHandaku();                             // 0xA4
    virtual void convertAll();                                // 0xA8
    virtual bool isSmall();                                   // 0xAC
    virtual void converSmall();                               // 0xB0
    virtual bool atTheBeginningOfASentence();                 // 0xB4
    virtual void initKanaConverter();                         // 0xB8
    virtual wchar_t *getKanaBuffer() { return mwcKanaBuffer; } // 0xBC
#if VERSION == VERSION_RUUK01_01
    virtual bool isKanaFix() const {
        if (meTranslateMode == TM_Hangul) {
            return muHangulLength == 0;
        }
        return KPRLookAhead(const_cast<KPRQueue *>(&mKanaQueue), NULL, 0) == 0;
    }
    virtual void confirmKana();
    virtual bool clearKana();
    virtual void EnableKSXFilter(bool enable);
#else
    virtual bool isKanaFix() const { return KPRLookAhead(const_cast<KPRQueue *>(&mKanaQueue), NULL, 0) == 0; } // 0xC0
    virtual bool clearKana() { return KPRLookAhead(&mKanaQueue, NULL, 0) == 0; } // 0xC4
    virtual void confirmKana();                               // 0xC8
#endif

    void inputString(const wchar_t *string, TranslateMode mode);
    void setTranslateMode(TranslateMode mode);
    TranslateMode getTranslateMode() const { return meTranslateMode; }

protected:
    /* 0x18 */ u32 muCursorPos;
    /* 0x1C */ u32 muAnchorPos;
    /* 0x20 */ bool mbSustain;
    /* 0x24 */ TranslateMode meTranslateMode;
    /* 0x28 */ KPRQueue mKanaQueue;
#if VERSION == VERSION_RUUK01_01
    /* 0x40 */ u8 mHangul[0x50 - 0x40];
    /* 0x50 */ u32 muHangulLength;
    /* 0x54 */ u8 _54[0x5C - 0x54];
#endif
    /* 0x40 */ wchar_t mwcKanaBuffer[5];
}; // size 0x4C

} // namespace tistring
} // namespace textinput
