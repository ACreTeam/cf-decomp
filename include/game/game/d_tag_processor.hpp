#pragma once

#include <nw4r/ut.h>

// The game's text tag processor (TU d_tag_processor.cpp, 8016C720..8016CCCC). Name from the RTTI
// ("dTagProcessor_c"); m2d::tagProcessor_c (d_m2d) derives from it. Handles the BMG system tags
// (group 0xFF, dScript::TagSystem_e) and stops drawing at an answer list (group 7, TagSelect_e).
// Method and member names are inferred.
class dTagProcessor_c : public nw4r::ut::TagProcessorBase<wchar_t> {
public:
    dTagProcessor_c() {}
    virtual ~dTagProcessor_c() {}
    virtual Operation Process(u16 ch, ContextType *pCtx); // 8016C720
    virtual Operation CalcRect(nw4r::ut::Rect *pRect, u16 ch, ContextType *pCtx); // 8016C8EC

    // The system tag handlers: (unused, context, tag size, tag parameters).
    void procColor(int, ContextType *pCtx, u8 size, void *data);      // 8016C9FC: [u8 color] text colour (dMsg::TextColor_e)
    void procScale(int, ContextType *pCtx, u8 size, void *data);      // 8016CA98: [u16 percent] text scale
    void procLineOffset(int, ContextType *pCtx, u8 size, void *data); // 8016CAF0: [s16] moves the cursor's y

    /* 0x04 */ u8 mRectEnd;  // CalcRect reached an answer list (cleared by the message controller's draw, d_msg)
    /* 0x05 */ u8 mDrawEnd;  // Process reached an answer list / TAG_SYSTEM_TYPE_END: draw nothing more
    /* 0x06 */ u8 mLine;     // lines drawn ('\n' count); cleared by dMsg::Seq_c (new page)
    /* 0x07 */ u8 mTypeLine; // dMsg::Seq_c: the line being typed
};

// The message window's tag processor (8074E8A0, .sbss; constructed in d_tag_processor's static init).
extern dTagProcessor_c g_tagProcessor; // 8074E8A0
