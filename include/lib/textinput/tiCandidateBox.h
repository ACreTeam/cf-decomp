#pragma once

// textinput::candidatebox: the conversion candidate box (see tiTextInputBase.h).

#include <lib/textinput/tiNw4rManager.h>
#include <wchar.h>

namespace textinput {
namespace candidatebox {

class Base;
class LayoutByNW4R;

// The side that opens the candidate box (the input form).
class CandidateBoxCaller {
public:
    struct Candidates {
        /* 0x000 */ s32 mNumCandidate;
        /* 0x004 */ wchar_t mszCandidate[50][61];
    }; // size 0x17D8

    CandidateBoxCaller() : mpCandidateBox(NULL) { mCandidates.mNumCandidate = 0; }

    virtual void resetCandidate() { // 0x08
        mCandidates.mNumCandidate = 0;
        updateCandidate();
    }
    virtual void addCandidate(const wchar_t *wcString) { // 0x0C
        wcsncpy(mCandidates.mszCandidate[mCandidates.mNumCandidate], wcString, 61);
        mCandidates.mNumCandidate++;
    }
    virtual void setCandidateBox(Base *candidateBox) { mpCandidateBox = candidateBox; } // 0x10
    virtual void updateCandidate();    // 0x14
    virtual void makeEmptyCandidate(); // 0x18

    /* 0x04 */ Base *mpCandidateBox;
    /* 0x08 */ Candidates mCandidates;
}; // size 0x17E0

class Base : public CommandSender {
public:
    virtual void vt_0x20();
    virtual void draw();     // 0x24
    virtual void turnOn();   // 0x28
    virtual void turnOff();  // 0x2C
    virtual void makeEmpty(); // 0x30
    virtual void vt_0x34();
    virtual void vt_0x38();
    virtual void vt_0x3C();
    virtual void vt_0x40();
    virtual void vt_0x44();
    virtual void vt_0x48();

    /* 0x10 */ u8 _10[0x17F4 - 0x10];
}; // size 0x17F4

class UITextArea {
public:
    bool IsScrolling();
    void ChangeSelectedText(s32 index);
    void ScrollToSelectedText();

    /* 0x000 */ u8 _000[0x2D0];
}; // size 0x2D0

class LayoutByNW4R : public Base, public nw4rmanager::Layout, public nw4rmanager::AnmObserver {
public:
    virtual ~LayoutByNW4R();
    virtual void draw();
    virtual void vt_0xCC();
    virtual void vt_0xD0();
    virtual void onLeftScroll();          // 0xD4
    virtual void vt_0xD8();
    virtual void vt_0xDC();
    virtual void vt_0xE0();
    virtual void changeSelectEmpty_compatible(); // 0xE4
    virtual void changeSelect();          // 0xE8
    virtual void vt_0xEC();
    virtual void vt_0xF0();
    virtual bool isInScroll() { return mTextArea.IsScrolling(); } // 0xF4
    virtual void setActive(bool active) { mbActive = active; }    // 0xF8
    virtual bool isActive() const { return mbActive; }            // 0xFC

    /* 0x18A0 */ u8 _18A0[0x18B8 - 0x18A0];
    /* 0x18B8 */ UITextArea mTextArea;
    /* 0x1B88 */ bool mbActive;
};

} // namespace candidatebox
} // namespace textinput
