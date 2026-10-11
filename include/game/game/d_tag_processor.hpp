#pragma once

#include <nw4r/ut.h>

// The game's text tag processor (TU d_tag_processor.cpp, 8016C720..8016CCCC). Name from the RTTI
// ("dTagProcessor_c"); m2d::tagProcessor_c (d_m2d) derives from it.
class dTagProcessor_c : public nw4r::ut::TagProcessorBase<wchar_t> {
public:
    dTagProcessor_c() {}
    virtual ~dTagProcessor_c() {}
    virtual Operation Process(u16 ch, ContextType *pCtx); // 8016C720
    virtual Operation CalcRect(nw4r::ut::Rect *pRect, u16 ch, ContextType *pCtx); // 8016C8EC

    void fn_8016C9FC(int, ContextType *pCtx, u8 size, void *data); // handles the 0xFF tags

    /* 0x04 */ u8 _4; // m2d's static instance takes 8 bytes of .sbss
    /* 0x05 */ u8 _5; // _4.._6 cleared by the message controller's draw (d_msg)
    /* 0x06 */ u8 _6;    // cleared by dMsg::Seq_c (new page)
    /* 0x07 */ u8 _7;    // dMsg::Seq_c: the line being typed
};

// The message window's tag processor (8074E8A0, .sbss; constructed in d_tag_processor's static init).
extern dTagProcessor_c g_tagProcessor; // 8074E8A0
