#pragma once

// The chat text box and its software keyboard (RTTI dObjKeyboard_c). Names other than the class names
// are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_m2d.hpp>

class dObjKeyboard_c : public fBase_c {
public:
    enum { TEXT_LEN = 0x20 };

    enum STATE_e {
        STATE_WAIT,
        STATE_OPEN,
        STATE_INPUT,
        STATE_CLOSE,
        STATE_EMOTION_START,
        STATE_EMOTION,
        STATE_EMOTION_END,
    };

    enum ANM_e {
        ANM_IN,
        ANM_OUT,
    };

#if VERSION == VERSION_RUUK01_01
    enum { MODE_NUM = 2 };
#elif VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02
    enum { MODE_NUM = 3 };
#endif

    enum FLAG_e {
        FLAG_OPEN = 0x1,
        FLAG_CALC = 0x2,
        FLAG_KBD_UPDATED = 0x4,
    };

    class Word_c : public m2d::Word_c {
    public:
        Word_c();
        virtual ~Word_c();
        virtual u32 getBufferSize();
        virtual wchar_t *getBuffer();

        /* 0x28 */ wchar_t mBuffer[0x2F];
    }; // size 0x88

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual ~dObjKeyboard_c();

    void stateWait();
    void stateOpen();
    void stateInput();
    void stateClose();
    void stateEmotionStart();
    void stateEmotion();
    void stateEmotionEnd();
    BOOL canOpen();
    BOOL checkLength();
    void updateText();
    void restoreText();
    void clearText();
    BOOL send();
    void initAnm();
    void resetTimer();
    void playAnm();
    void setCursor();
    u8 getEmotion(int slot);

    static bool isOpen();

    /* 0x064 */ m2d::ResAccLoader_c mRes;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mBoxAnm;
    /* 0x20C */ m2d::Anm_c mDownAnm;
#if VERSION == VERSION_RUUJ01_01 || VERSION == VERSION_RUUJ01_02 || VERSION == VERSION_RUUK01_01
    /* 0x230 */ m2d::SimpleEx_c mModeLyt;
    /* 0x2C8 */ m2d::AnmEx_c mModeAnm;
    /* 0x304 */ m2d::AnmEx2_c mModeKeyAnm[MODE_NUM];
#endif
    /* 0x230 */ m2d::SimpleEx_c mCapsLyt;
    /* 0x2C8 */ m2d::Anm_c mRegionAnm;
    /* 0x2EC */ m2d::AnmEx_c mCapsAnm;
    /* 0x328 */ u8 mState;
    /* 0x32C */ int mTimer;
    /* 0x330 */ Word_c mWord;
    /* 0x3B8 */ wchar_t mText[TEXT_LEN + 1];
    /* 0x3FC */ dKbd::Mng_c mKbd;
    /* 0x774 */ dKbd::Cur_c mCur;
    /* 0x92C */ int mEmotionSlot;
    /* 0x930 */ u8 mEmotion;
}; // size 0x934
