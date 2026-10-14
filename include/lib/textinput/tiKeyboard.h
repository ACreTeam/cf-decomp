#pragma once

// textinput::keyboard: the base of the keyboards (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>

namespace textinput {
namespace keyboard {

class KeyboardBase : public CommandSender {
public:
    virtual ~KeyboardBase();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void setCommandReceiver(CommandReceiver *receiver);
    virtual void updateFromReceiver(u32, void *);
    virtual void onKey(u32 key);                  // 0x20
    virtual int getType();                        // 0x24
    virtual void setLanguage(Language language);  // 0x28
    virtual Language getLanguage() const;         // 0x2C
    virtual void update();                        // 0x30
    virtual void onActive();                      // 0x34

protected:
    /* 0x10 */ Language meLanguage;
}; // size 0x14

} // namespace keyboard
} // namespace textinput
