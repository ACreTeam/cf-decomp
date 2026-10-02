#pragma once

#include <types.h>
#include <lib/egg/core/eggMsgRes.h>

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
// The vtable has 55 slots (0xDC bytes).
class Word_c {
public:
    Word_c(); // 80157D3C
    virtual ~Word_c(); // 80157D8C
    virtual u32 getBufferSize() = 0; // vtable +0x0C
    virtual wchar_t *getBuffer() = 0; // vtable +0x10
    // Vtable +0x14..+0xD8. Names and signatures are placeholders (vfXX =
    // vtable offset); most of these are empty or tail-call another function.
    virtual void vf14(); // 801598B8
    virtual void vf18(); // 8015822C
    virtual void vf1C(); // 8015A2E8
    virtual void vf20(); // 8015B134
    virtual void vf24(); // 8015A474
    virtual void vf28(); // 80158088
    virtual void vf2C(); // 80158258
    virtual void vf30(); // 80158260
    virtual void vf34(); // 80158268
    virtual void vf38(); // 8015A038
    virtual void vf3C(); // 8015A03C
    virtual void vf40(); // 8015A0A0
    virtual void vf44(); // 8015A0A4
    virtual void vf48(); // 8015A0A8
    virtual void vf4C(); // 8015A0AC
    virtual void vf50(); // 8015A0B0
    virtual void vf54(); // 8015A0B4
    virtual void vf58(); // 8015A0B8
    virtual void vf5C(); // 8015A0BC
    virtual void vf60(); // 8015A0C0
    virtual void vf64(); // 8015A1AC
    virtual void vf68(); // 8015A1B0
    virtual void vf6C(); // 8015A1B4
    virtual void vf70(); // 8015A1B8
    virtual void vf74(); // 8015A248
    virtual void vf78(); // 8015A2DC
    virtual void vf7C(); // 8015A2E0
    virtual void vf80(); // 8015A2E4
    virtual void vf84(); // 8015A370
    virtual void vf88(); // 8015A374
    virtual void vf8C(); // 8015A450
    virtual void vf90(); // 8015A454
    virtual void vf94(); // 8015A458
    virtual void vf98(); // 8015A45C
    virtual void vf9C(); // 8015A464
    virtual void vfA0(); // 8015A46C
    virtual void vfA4(); // 8015A470
    virtual void vfA8(); // 8015A49C
    virtual void vfAC(); // 8015A580
    virtual void vfB0(); // 8015A98C
    virtual void vfB4(); // 8015AD68
    virtual void vfB8(); // 8015AD6C
    virtual void vfBC(); // 8015AD70
    virtual void vfC0(); // 8015AD74
    virtual void vfC4(); // 8015AD78
    virtual void vfC8(); // 8015AE0C
    virtual void vfCC(); // 8015AE10
    virtual void vfD0(); // 8015AE14
    virtual void vfD4(); // 8015B12C
    virtual void vfD8(); // 8015B130

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

// A BMG message resource. Name from RTTI; vtable 804EFCAC.
class Res_c : public EGG::MsgRes {
public:
    Res_c(const void *data); // 8015784C
    virtual ~Res_c(); // 80157888

    const wchar_t *getMessage(int id); // 801578E0
    u16 getCount(); // 801578F0: number of INF1 entries
};

} // namespace dScript
