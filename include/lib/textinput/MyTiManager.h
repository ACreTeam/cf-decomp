#pragma once

// textinput::extend::memo::Manager: the manager with the memo extension (see tiTextInputBase.h).

#include <lib/textinput/tiManager.h>

namespace textinput {
namespace extend {
namespace memo {

// The keyboard settings, kept by the application between uses.
struct Setting {
    /* 0x00 */ u8 mKeyboardType;
    struct {
        /* 0x01 */ u8 m01;
        /* 0x02 */ u8 m02;
        /* 0x03 */ u8 m03;
    };
    struct {
        /* 0x04 */ u8 m04_7 : 1;
        /* 0x04 */ u8 m04_3 : 4;
        /* 0x04 */ u8 m04_0 : 3;
        /* 0x05 */ u8 m05_4 : 4;
        /* 0x05 */ u8 m05_0 : 4;
    };
    /* 0x06 */ u8 m06;
    /* 0x07 */ u8 m07;
}; // size 0x8

class Memo {
public:
    virtual void vt_0x08();
    virtual int vt_0x0C();
};

class Manager : public textinput::Manager {
public:
    Manager(MEMAllocator *allocator, nw4r::lyt::MultiArcResourceAccessor *multiArc, EventObserver *event)
        : textinput::Manager(allocator, multiArc, event), mpMemo(NULL), m40(1), m4C(NULL), m50(NULL), m54(NULL),
          m58(NULL), m5C(NULL), m60(NULL), m64(NULL), m68(NULL), m6C(NULL), m70(NULL), m74(NULL) {}
    virtual ~Manager();
    virtual void vt_0xC4();
    virtual void setSetting(const Setting &setting) { mSetting = setting; } // 0xC8
    virtual Setting getSetting() { return mSetting; }                        // 0xCC
    virtual void vt_0xD0(bool);
    virtual Memo *getMemo() { return mpMemo; }                               // 0xD4
    virtual void vt_0xD8(int);
    virtual int vt_0xDC() { return m40; }                                    // 0xDC
    virtual void vt_0xE0();
    virtual void vt_0xE4();
    virtual void vt_0xE8();
    virtual void vt_0xEC();
    virtual void vt_0xF0();
    virtual void vt_0xF4();
    virtual void vt_0xF8();
    virtual void vt_0xFC();
    virtual void vt_0x100();
    virtual void vt_0x104();
    virtual void vt_0x108();
    virtual void vt_0x10C();
    virtual void vt_0x110();
    virtual void vt_0x114();
    virtual void vt_0x118();
    virtual void vt_0x11C();
    virtual void vt_0x120();
    virtual void vt_0x124();
    virtual void vt_0x128();
    virtual void vt_0x12C();

protected:
    /* 0x3C */ Memo *mpMemo;
    /* 0x40 */ int m40;
    /* 0x44 */ Setting mSetting;
    /* 0x4C */ void *m4C;
    /* 0x50 */ void *m50;
    /* 0x54 */ void *m54;
    /* 0x58 */ void *m58;
    /* 0x5C */ void *m5C;
    /* 0x60 */ void *m60;
    /* 0x64 */ void *m64;
    /* 0x68 */ void *m68;
    /* 0x6C */ void *m6C;
    /* 0x70 */ void *m70;
    /* 0x74 */ void *m74;
}; // size 0x78

} // namespace memo
} // namespace extend
} // namespace textinput
