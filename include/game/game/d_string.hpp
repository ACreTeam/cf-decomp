#pragma once

#include <game/game/d_script.hpp>

// String words built on dScript::Word_c (TU around 8016B20C). Names and the
// hierarchy come from RTTI: dString::Word_c : dString::WordBase_c :
// dScript::Word_c.
namespace dString {

// Abstract: still leaves the buffer accessors pure. Overrides Word_c's
// vtable slot +0x24 (8016B3B0), which reads gender/article bytes from an
// inflection tag into mInflect.
class WordBase_c : public dScript::Word_c {
public:
    virtual ~WordBase_c(); // 8016B20C
}; // sizeof = 0x24

// A word with a fixed 101-character buffer.
class Word_c : public WordBase_c {
public:
    Word_c(); // 8016B630
    virtual ~Word_c(); // 8016B740
    virtual u32 getBufferSize(); // 8016B798: returns sizeof(mBuffer)
    virtual wchar_t *getBuffer(); // 8016B7A0

    wchar_t mBuffer[0x65]; // 0x24
}; // sizeof = 0xF0

} // namespace dString
