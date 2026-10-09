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

    /* 0x04 */ u8 _4[4]; // m2d's static instance takes 8 bytes of .sbss
};
