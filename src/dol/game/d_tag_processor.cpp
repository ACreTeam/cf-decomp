// The game's text tag processor (dTagProcessor_c). .text 8016C720..8016CCCC.
// See include/game/game/d_tag_processor.hpp.
#include <game/game/d_tag_processor.hpp>
#include <game/game/d_script.hpp>
#include <lib/egg/core/eggMsgRes.h>

// The tag code analyzeTag returns: group << 16 | id.
#define TAG_CODE(group, id) (((group) << 16) | (id))

dTagProcessor_c g_tagProcessor;

// The text colours of TAG_SYSTEM_COLOR (dMsg::TextColor_e).
static nw4r::ut::Color l_textColor[] = {
    nw4r::ut::Color(0x323C32FF), nw4r::ut::Color(0xD85000FF), nw4r::ut::Color(0x0080FFFF),
    nw4r::ut::Color(0xE00078FF), nw4r::ut::Color(0x808080FF), nw4r::ut::Color(0x2838FFFF),
    nw4r::ut::Color(0xA800C0FF), nw4r::ut::Color(0x6800FFFF), nw4r::ut::Color(0x008000FF),
    nw4r::ut::Color(0x0080FFFF), nw4r::ut::Color(0xFFFFFFFF),
};

#define TEXT_COLOR_NUM (sizeof(l_textColor) / sizeof(l_textColor[0]))

// 8016C720
dTagProcessor_c::Operation dTagProcessor_c::Process(u16 ch, ContextType *pCtx) {
    if (mDrawEnd) {
        return OPERATION_END_DRAW;
    }
    int c = ch;
    if (c == '\n') {
        mLine++;
    }
    if (ch == TAG_ESCAPE) {
        u8 size = 0;
        u32 cmd = 0;
        void *data = nullptr;
        EGG::MsgRes::analyzeTag(ch, pCtx->str, &size, &cmd, &data);
        switch (cmd) {
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_COLOR):
                procColor(0, pCtx, size, data);
                break;
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_SCALE):
                procScale(0, pCtx, size, data);
                break;
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_TYPE_END):
                mDrawEnd = 1;
                pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
                return OPERATION_END_DRAW;
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_LINE_OFFSET):
                procLineOffset(0, pCtx, size, data);
                break;
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_RUBY):
                pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
                return OPERATION_DEFAULT;
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_2):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_3):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_4):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_5):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_2_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_3_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_4_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_5_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_ANALOG):
                mDrawEnd = 1;
                pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
                return OPERATION_END_DRAW;
        }
        pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
        return OPERATION_DEFAULT;
    }
    return nw4r::ut::TagProcessorBase<wchar_t>::Process(ch, pCtx);
}

// 8016C8EC
dTagProcessor_c::Operation dTagProcessor_c::CalcRect(nw4r::ut::Rect *pRect, u16 ch, ContextType *pCtx) {
    if (ch == TAG_ESCAPE) {
        u8 size = 0;
        u32 cmd = 0;
        void *data = nullptr;
        EGG::MsgRes::analyzeTag(ch, pCtx->str, &size, &cmd, &data);
        switch (cmd) {
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_COLOR):
                pCtx->writer->ResetWidthLimit();
                break;
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_2):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_3):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_4):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_5):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_2_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_3_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_4_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_5_CANCEL):
            case TAG_CODE(dScript::TAG_GROUP_SELECT, dScript::TAG_SELECT_ANALOG):
                mRectEnd = 1;
                pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
                return OPERATION_END_DRAW;
            case TAG_CODE(dScript::TAG_GROUP_SYSTEM, dScript::TAG_SYSTEM_SCALE):
                procScale(0, pCtx, size, data);
                break;
        }
        pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
        return OPERATION_DEFAULT;
    }
    return nw4r::ut::TagProcessorBase<wchar_t>::CalcRect(pRect, ch, pCtx);
}

// 8016C9FC
void dTagProcessor_c::procColor(int, ContextType *pCtx, u8 size, void *data) {
    u8 idx = *(u8 *)data;
    if (idx >= TEXT_COLOR_NUM) {
        idx = 0;
    }
    nw4r::ut::Color color = l_textColor[idx];
    nw4r::ut::Color min = color;
    min.a = 0;
    pCtx->writer->SetColorMapping(min, color);
    pCtx->writer->SetupGX();
}

// 8016CA98
void dTagProcessor_c::procScale(int, ContextType *pCtx, u8 size, void *data) {
    u16 scale = *(u16 *)data;
    pCtx->writer->SetScale(0.85f * (scale / 100.0f), scale / 100.0f);
}

// 8016CAF0
void dTagProcessor_c::procLineOffset(int, ContextType *pCtx, u8 size, void *data) {
    s16 ofs = *(s16 *)data;
    pCtx->writer->MoveCursorY(ofs);
}
