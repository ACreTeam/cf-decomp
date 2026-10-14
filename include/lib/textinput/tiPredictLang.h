#pragma once

// textinput::predictlang: the prediction language dialog (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <lib/textinput/tiKeyboard.h>

namespace textinput {
namespace predictlang {

class Base : public CommandSender {
public:
    virtual ~Base();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void setPredictMode(int predictMode); // 0x20

protected:
    /* 0x10 */ u8 _10[0x1C - 0x10];
}; // size 0x1C

class LayoutByNW4R : public Base, public nw4rmanager::Layout, public nw4rmanager::AnmObserver {
public:
    virtual ~LayoutByNW4R();
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void draw() { nw4rmanager::Layout::draw(); }                       // 0xA4
    virtual void open(int predictMode, keyboard::KeyboardBase *keyboard);      // 0xA8
    virtual void endToOpen();                                                  // 0xAC
    virtual void startToClose();                                               // 0xB0
    virtual void endToClose();                                                 // 0xB4
    virtual bool isActive() { return mbActive; }                               // 0xB8

protected:
    /* 0xC8 */ bool mbActive;
};

} // namespace predictlang
} // namespace textinput
