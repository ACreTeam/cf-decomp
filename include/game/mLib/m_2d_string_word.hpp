#pragma once

#include <types.h>
#include <game/game/d_script.hpp>

namespace m2d {

// A script word used by 2D layouts (TU around 80009DA4). Name from RTTI; vtable 8049DA48.
class stringWord_c : public dScript::Word_c {
public:
    stringWord_c(); // 80009DA4
    virtual ~stringWord_c(); // 80009DE8
    virtual u32 getBufferSize();
    virtual wchar_t *getBuffer();

    /* 0x24 */ wchar_t mBuffer[0x68];
}; // size 0xF4

} // namespace m2d
