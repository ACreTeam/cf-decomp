#pragma once

// d_wifi_err (DOL TU d_wifi_err.cpp, .text 8017C4EC..8017CD3C): the network error window (dWifiErr_c, the
// "caution_00" layout). The namespace and dWifiErr::Word_c are named by RTTI; everything else is inferred.
// (The friend list functions before it are d_wifi's: see d_wifi.hpp.)

#include <types.h>
#include <game/game/d_script.hpp>
#include <game/mLib/m_2d.hpp>

namespace dWifiErr {

BOOL setSysMessage(dScript::Word_c *word, u16 msgId, const char *name); // 8017C9AC: a BMG message by id
void setErrorCodeWord(int digits);                       // 8017CA80: the error code, digits wide
dScript::Word_c *getErrorCodeWord();                     // 8017CAB8

// A message word with its own buffer; its number tags (time 6-12) show the error code.
class Word_c : public dScript::Word_c {
public:
    Word_c();                                            // 8017CAC4
    virtual ~Word_c();                                   // 8017CB08
    virtual u32 getBufferSize();                         // 8017CB60
    virtual wchar_t *getBuffer();                        // 8017CB68
    virtual int procNumber(int pos);                     // 8017CB70
    virtual int procRubyTag(int pos);                    // 8017CBEC

    /* 0x024 */ wchar_t mBuffer[0x101];
}; // size 0x228

} // namespace dWifiErr

// The network error window. 0x188 bytes.
class dWifiErr_c {
public:
    void create();               // 8017C4EC: builds the caution_00 layout
    void setMessage(int msgId);  // 8017C600: sys_2D/SYS_2D_menu message msgId into T_dialg_00
    void clear();                // 8017C69C
    u8 check();                  // 8017C6CC: opens the window on a reported error; returns mActive
    void draw();                 // 8017C7A8
    void execute();              // 8017C7DC

    /* 0x000 */ u8 mActive;        // the window is shown
    /* 0x001 */ u8 mNetError;      // the network manager reported an error
    /* 0x002 */ u8 mReported;      // dNet::getNetReportedError
    /* 0x003 */ u8 mFatal;         // error kind 3: A / C don't close the window
    /* 0x004 */ u8 mFatalMessage;  // the kind 3 message (0x15E) was set
    /* 0x005 */ u8 mCloseRequest;  // A / C pressed (or mResetRequest)
    /* 0x006 */ u8 mClosing;       // a reset was requested; waiting for its fade out
    /* 0x007 */ u8 mResetRequest;  // the reset button was pressed while it is shown (d_reset)
    /* 0x008 */ u8 mForce;         // shows the window without a net manager error; it can't be closed
    /* 0x00C */ int mErrorCode;    // dNetFriend::Error_c::mDetail
    /* 0x010 */ m2d::ResAcc_c mResAcc;
    /* 0x0CC */ m2d::Simple_c mLyt;
    /* 0x164 */ m2d::Anm_c mAnm;
}; // size 0x188

extern dWifiErr_c g_wifiErr; // 80621670
