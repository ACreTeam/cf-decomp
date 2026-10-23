#pragma once

// textinput::toolbar: the tool bar above the keyboard (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>

namespace textinput {

class Manager;

namespace toolbar {

class Base {
public:
    virtual void create(MEMAllocator *allocator);      // 0x08
    virtual void init();                               // 0x0C
    virtual bool isQwerty() const { return mQwerty; }  // 0x10
    virtual void setQwerty(bool qwerty);               // 0x14
    virtual void setLanguage(Language language);       // 0x18
    virtual Language getLanguage() const;              // 0x1C

protected:
    /* 0x04 */ bool mQwerty;
    /* 0x08 */ Language meLanguage;
    /* 0x0C */ MEMAllocator *mpAllocator;
    /* 0x10 */ Manager *mpManager;
}; // size 0x14

class LayoutByNW4R : public Base, public nw4rmanager::Layout {
public:
    virtual ~LayoutByNW4R();                                 // 0x94
    virtual void create(MEMAllocator *allocator);
    virtual void init();
    virtual void draw() { nw4rmanager::Layout::draw(); }     // 0x98
    virtual void vt_0x9C();
    virtual void vt_0xA0();
    virtual void setQwertyWithSE(bool qwerty);                // 0xA4
    virtual bool isHWKBMode() { return mbHWKBMode; }          // 0xA8

    /* 0xBC */ u8 _BC[0xC0 - 0xBC];
    /* 0xC0 */ bool mbHWKBMode;
};

} // namespace toolbar
} // namespace textinput
