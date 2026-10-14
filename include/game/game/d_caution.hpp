#pragma once

// d_caution (DOL TU d_caution.cpp, .text 80086558..80086C48): the disc error ("caution") window. It shows the
// "caution_00" layout with a message for the DVD drive status, in its own font, from data embedded in the DOL.
// No RTTI: the class and function names are inferred (namespace dCaution: see notes/game_tu_conventions.md).

#include <types.h>
#include <game/mLib/m_2d.hpp>
#include <lib/egg/core/eggMsgRes.h>
#include <nw4r/ut/ut_ResFont.h>

// Embedded resources (unsplit .data, outside d_caution's own data).
// 804A7780: the caution layout archive (U8, 0x89E bytes, "caution_00.brlyt" / ".brlan"), after d_fd_block_type's
// data. Used by d_caution and d_wifi_err.
extern u8 g_cautionArc[];
// 804D59A0..804DE6B8, after d_demo_actor's data: the window's font (RFNT) and its BMG messages per language.
extern u8 g_cautionFont[];   // 804D59A0
extern u8 g_cautionMsgUS[];  // 804DDB98: LANGUAGE_US
extern u8 g_cautionMsgQC[];  // 804DDF38: LANGUAGE_QC
extern u8 g_cautionMsgMX[];  // 804DE2F8: LANGUAGE_MX

namespace dCaution {

// The disc error window (0x3A8 bytes, g_caution). d_sys creates and runs it; d_home_button reads mActive.
class Caution_c {
public:
    bool loadFont(nw4r::ut::ResFont *font);       // 80086558: g_cautionFont into a dvdErrHeap_p buffer
    void removeFont(nw4r::ut::ResFont *font);     // 80086620
    wchar_t *getMessage(int msgId);               // 80086664: message msgId (tags removed) in mText
    void copyText(const wchar_t *text);           // 800866B0: text into mText without its tags
    void create();                                // 800866EC: builds the caution_00 layout
    void setMessage();                            // 800867E4: the message for mDriveStatus into T_dialg_00
    void init();                                  // 800868C4: selects the messages of the console language
    BOOL check();                                 // 80086990: opens / closes the window; returns mActive
    void draw();                                  // 80086AA8
    void execute();                               // 80086B00

    /* 0x000 */ u8 mActive;                       // the window is shown
    /* 0x004 */ s32 mDriveStatus;                 // DVDGetDriveStatus
    /* 0x008 */ nw4r::ut::ResFont mFont;
    /* 0x024 */ const EGG::MsgRes::MessageInfoDataBlock *mpInfo; // INF1
    /* 0x028 */ const u8 *mpData;                 // DAT1's strings
    /* 0x02C */ wchar_t mText[0x100];
    /* 0x22C */ m2d::ResAcc_c mResAcc;
    /* 0x2E8 */ m2d::Simple_c mLyt;
    /* 0x380 */ m2d::Anm_c mAnm;
    /* 0x3A4 */ u8 mNetError;                     // opened while networking: g_wifiErr is forced open too
}; // size 0x3A8

extern Caution_c g_caution; // 80584D90

} // namespace dCaution
