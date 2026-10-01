#pragma once

#include <types.h>

// Script/word objects used to build localized names (TU around 80157D3C).
// Names come from RTTI: dScript::Word_c (vtable 804EFB90), with
// dScript::Inflect_c and dScript::Pacchim_c embedded as members (Word_c's
// RTTI lists no bases, so these are not base classes).
namespace dScript {

// Grammatical info for a word: gender plus the definite/indefinite article
// indices used by the European languages.
class Inflect_c {
public:
    Inflect_c() : mGender(-1), mIndefArticle(0), mDefArticle(0) {}
    virtual ~Inflect_c() {} // 8015B598

    void setGender(u8 gender); // 80157BF8
    void setIndefArticle(u8 article); // 80157C00
    void setDefArticle(u8 article); // 80157C70

    s32 mGender; // 0x04: -1 when unset
    u16 mIndefArticle; // 0x08: 0 when unset or out of range
    u16 mDefArticle; // 0x0A
}; // sizeof = 0xC

// Korean particle helper: remembers the last Hangul syllable so the
// following particle can be chosen by its final consonant (batchim).
class Pacchim_c {
public:
    Pacchim_c() : mLastChar(0) {}
    virtual ~Pacchim_c() {} // 8015B5D8

    void check(wchar_t c); // 80157B18: keeps c if it is in U+AC00..U+D7A3

    u16 mLastChar; // 0x04
}; // sizeof = 0x8

// A word/name built from message data. Subclasses provide the buffer.
// The vtable has 55 slots (0xDC bytes); only the ones used so far are
// declared here.
class Word_c {
public:
    Word_c(); // 80157D3C
    virtual ~Word_c(); // 80157D8C
    virtual u32 getBufferSize() = 0; // vtable +0x0C
    virtual wchar_t *getBuffer() = 0; // vtable +0x10

    void clear(); // 80157DCC: zeroes the buffer and resets the word state
    BOOL set(const wchar_t *str, int); // 801590F0: clear(), copy, then parse

    Inflect_c mInflect; // 0x04
    Pacchim_c mPacchim; // 0x10
    u32 _18; // 0x18
    u32 _1C; // 0x1C
    u8 _20; // 0x20
    u8 _21; // 0x21
    u8 _22; // 0x22 (cleared by clear(), not by the constructor)
    u8 _23; // 0x23
}; // sizeof = 0x24

} // namespace dScript
