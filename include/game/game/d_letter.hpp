#pragma once

#include <types.h>
#include <game/game/d_script.hpp>

// Letter text building (TU around 800CB638, not recovered yet).
namespace dLetter {

// A word with a buffer large enough for a letter body. Name from RTTI; vtable 804EB138.
class Word_c : public dScript::Word_c {
public:
    Word_c(); // 800CC2C4
    virtual ~Word_c(); // 800CC308
    virtual u32 getBufferSize();
    virtual wchar_t *getBuffer();

    /* 0x024 */ wchar_t mBuffer[0x402];
}; // size 0x828

} // namespace dLetter
