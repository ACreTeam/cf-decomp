#pragma once

// The software keyboard (DOL TU d_kbd.cpp, not decompiled). Only what d_obj_keyboard uses; names other than
// the class names are inferred.

#include <types.h>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_vec.hpp>

namespace dKbd {

// The keyboard and its text input (a textinput::Manager at 0x1B4).
class Mng_c : public m2d::Base_c {
public:
    Mng_c();          // 800C87D0
    virtual ~Mng_c(); // 800C8890
    virtual void draw();

    void init(int mode);              // 800C8954
    BOOL loadRes();                   // 800C8B94
    BOOL create();                    // 800C8FB4
    BOOL remove(int mode);            // 800C90F8
    void calc();                      // 800C9288
    void open(int a, int b);          // 800C96EC
    BOOL reopen();                    // 800C97C4
    BOOL isTextChanged();             // 800C9868
    BOOL isSendReq();                 // 800C9938
    BOOL isClearReq();                // 800C994C
    int getEmotion();                 // 800C9968: the chosen emotion slot, -1 if none
    void clearEmotion();              // 800C9978
    void postCalc();                  // 800C99DC
    const wchar_t *getText();         // 800C9CD8
    void setText(const wchar_t *str); // 800C9CEC
    void setMaxLength(int len);       // 800C9D00
    int getInputPos();                // 800C9D14
    int getCursorPos();               // 800C9D48
    void setCursorPos(int pos);       // 800C9E0C
    void resetCursor();               // 800C9EB4
    const wchar_t *getInputText();    // 800CA1BC: the text being composed
    void fixInput();                  // 800CA29C
    BOOL isUpdated();                 // 800CA64C
    void updateInput();               // 800CA6A0

    /* 0x010 */ u8 _010[0x378 - 0x10];
}; // size 0x378

// The text cursor.
class Cur_c {
public:
    Cur_c();          // 800CA958
    virtual ~Cur_c(); // 800CA9C8

    BOOL load();                    // 800CAA4C
    BOOL remove();                  // 800CAA5C
    void build();                   // 800CAA64
    void draw();                    // 800CAAF8
    void setPos(const mVec3_c *pos); // 800CAB44

    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::Anm_c mBlinkAnm;
    /* 0x0C0 */ m2d::Anm_c mColorAnm;
    /* 0x0E4 */ m2d::ResAccLoader_c mRes;
}; // size 0x1B8

void playSeCanNotInput(); // 800CB0BC
void playSeSubmit();      // 800CB0C4

} // namespace dKbd
