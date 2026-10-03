#pragma once

#include <game/game/d_script.hpp>

// Script word holding a villager nickname. Name from RTTI ("dNicknameWord_c");
// vtable 804ECD38. Its TU (around 800EB96C) is not split; names are inferred.
class dNicknameWord_c : public dScript::Word_c {
public:
    dNicknameWord_c(); // 800EB9B4
    virtual ~dNicknameWord_c(); // 800EB9F8
    virtual u32 getBufferSize(); // 800EBA50: returns sizeof(mBuffer)
    virtual wchar_t *getBuffer(); // 800EBA58

    BOOL hasNoLetters(); // 800EBA60: TRUE if there is no kana or Latin letter

    /* 0x24 */ wchar_t mBuffer[9];
}; // size 0x38
