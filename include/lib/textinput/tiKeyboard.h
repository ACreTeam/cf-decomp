#pragma once

// textinput::keyboard: the base of the keyboards (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
namespace keyboard {

enum VKeyCode {
    VK_LineFeed = 0x00,
    VK_BackSpace = 0x01,
    VK_Space = 0x02,
    VK_SignWindow = 0x09,
    VK_PredictLanguage = 0x0A,
    VK_Alphabet = 0x0B,
    VK_Numeric = 0x0D,
    VK_Hiragana = 0x12,
    VK_Katakana = 0x13,
    VK_CapsChange = 0x17,
    VK_SignClose = 0x18,
    VK_SignNext = 0x19,
    VK_SignPrev = 0x1A,
    VK_None = 0x1B,
};

class KeyboardBase : public CommandSender {
public:
    virtual ~KeyboardBase() {}
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void onKey(u32 key, void *data);      // 0x20
    virtual int getType();                        // 0x24
    virtual void setLanguage(Language language) { meLanguage = language; } // 0x28
    virtual Language getLanguage() const { return meLanguage; } // 0x2C
    virtual void update() {}                      // 0x30
    virtual void onActive() {}                    // 0x34

protected:
    /* 0x10 */ Language meLanguage;
}; // size 0x14

} // namespace keyboard
} // namespace textinput
