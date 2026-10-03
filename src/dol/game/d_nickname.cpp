// Script word holding a villager nickname. .text 800EB9B4..800EBB24.
// Starts right after the static initializer of the previous TU (800EB96C).
#include <game/game/d_nickname.hpp>

// 800EB9B4
dNicknameWord_c::dNicknameWord_c() {
    clear();
}

// 800EB9F8
dNicknameWord_c::~dNicknameWord_c() {}

// 800EBA50
u32 dNicknameWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 800EBA58
wchar_t *dNicknameWord_c::getBuffer() {
    return mBuffer;
}

// 800EBA60
BOOL dNicknameWord_c::hasNoLetters() {
    const wchar_t *p = getBuffer();
    int len = getTextLength(p, 0);
    for (int i = 0; i <= len; i++, p++) {
        if (dScript::isHiragana(*p)) {
            return FALSE;
        }
        if (dScript::isKatakana(*p)) {
            return FALSE;
        }
        if (dScript::isAlpha(*p)) {
            return FALSE;
        }
    }
    return TRUE;
}
