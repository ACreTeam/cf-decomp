#pragma once

#include <game/game/d_script.hpp>

// String words built on dScript::Word_c. Defined in src/dol/game/d_string.cpp.
// Names and the hierarchy come from RTTI: dString::Word_c :
// dString::WordBase_c : dScript::Word_c.
namespace dString {

// Abstract: still leaves the buffer accessors pure. Resolves the player and
// word-slot tags from the current player and the global word slots (Comp_c).
class WordBase_c : public dScript::Word_c {
public:
    WordBase_c(); // 8016B1D0
    virtual ~WordBase_c(); // 8016B20C

    virtual int setInflect(int pos); // 8016B3B0
    virtual int procWord(int pos); // 8016B264
    virtual int procPlayerChar0(int pos); // 8016B398
    virtual int procPlayerChar1(int pos); // 8016B3A0
    virtual int procPlayerChar2(int pos); // 8016B3A8
    virtual int procPlayer(int pos); // 8016B110
    virtual int procPlayerGender(int pos); // 8016B480
    virtual int procWordGender(int pos); // 8016B4CC
    virtual int procPlayerElision(int pos); // 8016B54C
    virtual int procWordElision(int pos); // 8016B598

    int procPlayerChar(int pos, int index); // 8016B2FC: character of the player name
}; // sizeof = 0x24

// A word with a fixed 101-character buffer.
class Word_c : public WordBase_c {
public:
    Word_c(); // 8016B630
    Word_c(const wchar_t *str); // 8016B674
    Word_c(u16 index, const char *group); // 8016B6D4: loads a BMG string (fn_8016AE68)
    virtual ~Word_c(); // 8016B740
    virtual u32 getBufferSize(); // 8016B798: returns sizeof(mBuffer)
    virtual wchar_t *getBuffer(); // 8016B7A0

    wchar_t mBuffer[0x65]; // 0x24
}; // sizeof = 0xF0

// The global word slots used by the tag handlers (lbl_805FF398). Name from RTTI.
class Comp_c {
public:
    virtual ~Comp_c(); // 8016B820 (include/game/game/d_string_comp.inc)

    /* 0x0004 */ Word_c mWords[20]; // word slots (fn_8016B15C / fn_8016B1B8)
    /* 0x12C4 */ Word_c mPlayerName; // source for procPlayerChar
    /* 0x13B4 */ u8 mNoChar; // set when procPlayerChar runs past the name
}; // sizeof = 0x13B8

} // namespace dString
