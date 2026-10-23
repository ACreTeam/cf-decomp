#pragma once

// textinput::tistring::WithAtok: the string with ATOK (Japanese) prediction (see tiTextInputBase.h).

#include <lib/textinput/tiString.h>

namespace textinput {
namespace tistring {

class WithAtok : public Decolated {
public:
    struct DrawInfo {};

    WithAtok(u16 maxLength) : Decolated(maxLength) {}

    virtual ~WithAtok() {}                                                // 0x08
    virtual bool isFix() { return true; }                                 // 0xCC
    virtual void setFix(bool fix) {}                                      // 0xD0
    virtual void initConverting() {}                                      // 0xD4
    virtual void pushBack(wchar_t ch) {}                                  // 0x10
    virtual void popBack() {}                                             // 0x14
    virtual void inputChar(wchar_t ch) {}                                 // 0x50
    virtual void backSpace() {}                                           // 0x5C
    virtual void confirm(const wchar_t *string) {}                        // 0x60
    virtual bool moveCursorRight() { return false; }                      // 0x64
    virtual bool moveCursorLeft() { return false; }                       // 0x68
    virtual void getCursorPos(u32 *cursor, u32 *anchor) {}                // 0x80
    virtual bool isConverting() { return false; }                         // 0xD8
    virtual const wchar_t *getConfirmedWCString() const { return NULL; }  // 0xDC
    virtual int getCurrentNumPredicted() { return 0; }                    // 0xE0
    virtual void getPredicted(int index, wchar_t *predicted) {}           // 0xE4
    virtual void commitPredicted(int index) {}                            // 0xE8
    virtual void setSelectedCandidate(u32 index) {}                       // 0xEC
    virtual bool isCandidateSelected() { return false; }                  // 0xF0
    virtual int getSelectedCandidate() { return 0; }                      // 0xF4
    virtual void init() {}                                                // 0xF8
    virtual void setInputing(wchar_t ch) {}                               // 0xFC
    virtual void getDrawString(DrawInfo &drawInfo) {}                     // 0x100
    virtual void openDictionary(void *atokDict, int atokDictSize, void *apotDict, int apotDictSize, void *nintendoDict,
                                int nintendoDictSize) {}                  // 0x104
    virtual void closeDictionary() {}                                     // 0x108
    virtual bool hasConfirmedString() { return false; }                   // 0x10C
    virtual void enableConfirmedString(bool enable) {}                    // 0x110
    virtual void startConverting() {}                                     // 0x114
    virtual bool isDictionaryOpen() { return false; }                     // 0x118
    virtual void changeKanaMode(bool kana) {}                             // 0x11C
    virtual u32 getInputStringLength() { return 0; }                      // 0x120
    virtual void resetRelation() {}                                       // 0x124
    virtual void setFixMode(bool fix) {}                                  // 0x128
    virtual void setFixedPredictionJP(int num, const char **predicts) {}   // 0x12C
    virtual void setDefaultPredictionJP(int num, const char **predicts) {} // 0x130
    virtual int getSelectedConverting() { return 0; }                     // 0x134
    virtual int getFixedPredictionNum() { return 0; }                     // 0x138

    bool isFixMode() { return false; }
    bool isKanaMode() { return false; }
};

} // namespace tistring
} // namespace textinput
