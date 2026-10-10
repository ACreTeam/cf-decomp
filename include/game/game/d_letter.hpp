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

// Letter text and message word slots (this TU, unsplit: C linkage keeps the target names).
class dPersonalID_c;
class dAnmPersonalID_c;
struct dLandID_c;
namespace dItem {
struct Item;
}
extern "C" {
// 800CB638: header / body / footer words of the mail text label (kind: the entry).
void fn_800CB638(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 kind, const char *label);
// 800CB66C / 800CB760: header / body / footer words from explicit entries.
void fn_800CB66C(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 a, u16 b, u16 c, int d,
                 int e, int f);
void fn_800CB720(dScript::Word_c *word, u16 msgId, const char *group); // 800CB720: word = BMG string group[msgId]
void fn_800CB760(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 a, u16 b, u16 c, u16 d,
                 u16 e, int f, int g);
u16 fn_800CBA14(const char *label); // 800CBA14: header variants of a mail label
u16 fn_800CBA40(const char *label); // 800CBA40: body variants
u16 fn_800CBA6C(const char *label); // 800CBA6C: footer variants
u16 fn_800CBA98(const char *label); // 800CBA98
u16 fn_800CBAC4(const char *label); // 800CBAC4
void fn_800CBAF0(int slot, int month);                         // 800CBAF0: month name word
void fn_800CBB50(int slot, u8 day);                            // 800CBB50: day word
void fn_800CBBB0(int slot, const dPersonalID_c *pid);          // 800CBBB0: player name word
void fn_800CBC10(int slot, const dLandID_c *land);             // 800CBC10: town name word
void fn_800CBC70(int slot, const dAnmPersonalID_c *animal);    // 800CBC70: villager name word
void fn_800CBCD4(int slot, dScript::Word_c *word);             // 800CBCD4: slot = a copy of word
void fn_800CBD30(int slot, u16 index, const char *group);      // 800CBD30: slot = BMG string group[index]
void fn_800CBDA0(int slot, const dItem::Item *item);           // 800CBDA0: item name word
void fn_800CBE0C(int slot, int value, int digits, int format); // 800CBE0C: number word (nothing when value < 0)
void fn_800CBEB4(int slot, int value);                         // 800CBEB4
void fn_800CBF34(int slot, int value, int digits, int format); // 800CBF34: number message word
void fn_800CBFE0(u8 hour, int slot, int);                      // 800CBFE0
void fn_800CC0C8(const dPersonalID_c *to);                     // 800CC0C8
void fn_800CC1AC(const dAnmPersonalID_c *sender);              // 800CC1AC
}
