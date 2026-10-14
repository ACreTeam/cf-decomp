#pragma once

// textinput::string: the edited strings (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <revolution/KPR.h>

namespace textinput {
namespace string {

class StringBase {
public:
    virtual ~StringBase();                                 // 0x08
    virtual void create(MEMAllocator *allocator);          // 0x0C
    virtual void pushBack(wchar_t ch);                     // 0x10
    virtual void popBack();                                // 0x14
    virtual void clear();                                  // 0x18
    virtual u16 getLength() const;                         // 0x1C
    virtual bool append(const wchar_t *string);            // 0x20
    virtual bool insert(u16 index, const wchar_t *string); // 0x24
    virtual void remove(u16 index, u16 num);               // 0x28
    virtual void replace(u16 index, u16 num, const wchar_t *string); // 0x2C
    virtual void set(const wchar_t *string);               // 0x30
    virtual void setAt(u16 index, wchar_t ch);             // 0x34
    virtual void setLength(u16 length);                    // 0x38
    virtual wchar_t *getWCString() const;                  // 0x3C
    virtual void setCandidate(wchar_t candidate);          // 0x40
    virtual wchar_t getCandidate();                        // 0x44
    virtual bool hasCandidate() const;                     // 0x48
    virtual wchar_t getLastWChar();                        // 0x4C

protected:
    /* 0x04 */ u8 _04[0x18 - 0x04];
}; // size 0x18

// A string edited with a cursor and kana composition.
class Decolated : public StringBase {
public:
    virtual void inputChar();                              // 0x50
    virtual void inputString();                            // 0x54
    virtual void deleteChar();                             // 0x58
    virtual void backSpace();                              // 0x5C
    virtual void confirm();                                // 0x60
    virtual void moveCursorRight();                        // 0x64
    virtual void moveCursorLeft();                         // 0x68
    virtual void setCursorPos(u32 pos);                    // 0x6C
    virtual void onSustain();                              // 0x70
    virtual void offSustain();                             // 0x74
    virtual void isOnSustain();                            // 0x78
    virtual u32 getCursorPos() { return mCursorPos; }      // 0x7C
    virtual void getCursorPos(u32 *start, u32 *end);       // 0x80
    virtual void canBackSpace();                           // 0x84
    virtual void deleteForward();                          // 0x88
    virtual void getSelected();                            // 0x8C
    virtual void getWCharAtCursor();                       // 0x90
    virtual void replaceAtCursor();                        // 0x94
    virtual void isDakuten();                              // 0x98
    virtual void converDakuten();                          // 0x9C
    virtual void isHandaku();                              // 0xA0
    virtual void converHandaku();                          // 0xA4
    virtual void convertAll();                             // 0xA8
    virtual void isSmall();                                // 0xAC
    virtual void converSmall();                            // 0xB0
    virtual bool atTheBeginningOfASentence();              // 0xB4
    virtual void initKanaConverter();                      // 0xB8
    virtual wchar_t *getKanaBuffer() { return mKanaBuffer; } // 0xBC
    virtual bool isKanaFix() { return KPRLookAhead(&mKanaQueue, NULL, 0) == 0; } // 0xC0
    virtual void confirmKana();                            // 0xC4
    virtual void clearKana();                              // 0xC8

protected:
    /* 0x18 */ u32 mCursorPos;
    /* 0x1C */ u8 _1C[0x28 - 0x1C];
    /* 0x28 */ KPRQueue mKanaQueue;
    /* 0x40 */ wchar_t mKanaBuffer[1];
};

} // namespace string
} // namespace textinput
