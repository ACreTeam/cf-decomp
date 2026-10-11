#pragma once

// dMsg::Word_c (TU d_msg.cpp): the message text the controller (dMsg::Comp_c) lays out.
// Overrides the dScript::Word_c tag handlers with ones that read the controller's word slots, the speaker
// and the player. Name and base from RTTI ("dMsg::Word_c" : dScript::Word_c); vtable 804FB5A8.

#include <types.h>
#include <game/game/d_msg_voice.hpp> // defined before Word_c (vtable order)
#include <game/game/d_script.hpp>
#include <game/game/d_animal_id.hpp>

namespace dMsg {
class Comp_c;
}
class dAnimal_c;

namespace dMsg {

// Text colours: the u8 parameter of the system tag 0xFF:0 (procColor, insertColorTag), an index into the
// tag processor's palette (d_tag_processor static init, lbl_80600838; > 10 falls back to 0). Names from
// the use here (5 / 6 / 8) or from the palette's RGB.
enum TextColor_e {
    TEXT_COLOR_DEFAULT = 0, // 323C32 (dark grey; articles, window colours)
    TEXT_COLOR_RED = 1,     // D85000
    TEXT_COLOR_BLUE = 2,    // 0080FF
    TEXT_COLOR_PINK = 3,    // E00078
    TEXT_COLOR_GREY = 4,    // 808080 (Voice_c: not voiced in a window of type 0xD)
    TEXT_COLOR_PLAYER = 5,  // 2838FF: player names / nicknames
    TEXT_COLOR_ANIMAL = 6,  // A800C0: villager names
    TEXT_COLOR_VIOLET = 7,  // 6800FF
    TEXT_COLOR_TOWN = 8,    // 008000: the town name
    TEXT_COLOR_BLUE2 = 9,   // 0080FF (same as 2)
    TEXT_COLOR_WHITE = 10,  // FFFFFF
};

#define TEXT_SCALE_DEFAULT 100 // text scale in percent (system tag 0xFF:1)
#define MSG_PAGE_LINES 4       // lines per window page (Seq_c::mPageLines, Word_c::mPageLines)

} // namespace dMsg

namespace dMsg {

class Word_c : public dScript::Word_c {
public:
    Word_c(); // 8019FE6C
    virtual ~Word_c(); // 8019FEB0
    virtual u32 getBufferSize(); // 8019FF44
    virtual wchar_t *getBuffer(); // 8019FF4C
    virtual int setTagWord(int pos, dScript::Word_c *word); // 8019FB18
    virtual int getParseStart(); // 8019FF54
    virtual int procTownName(int pos); // 801A110C
    virtual int procRubyTag(int pos); // 801A1F74
    virtual int procTag(int pos); // 801A01DC
    virtual int procNewLine(int pos); // 801A0130
    virtual int procEnd(int pos); // 801A01E0
    virtual int procChar(int pos); // 801A021C
    virtual int procNumber(int pos); // 801A041C
    virtual int procRandomNumber(int pos); // 801A04E0
    virtual int procWord(int pos); // 801A0638
    virtual int procDrink(int pos); // 801A0928
    virtual int procObject(int pos); // 801A0930
    virtual int procSweets(int pos); // 801A0938
    virtual int procSports(int pos); // 801A0940
    virtual int procMusic(int pos); // 801A0948
    virtual int procFood(int pos); // 801A0950
    virtual int procHobby(int pos); // 801A0958
    virtual int procUnit(int pos); // 801A0960
    virtual int procPlayerChar0(int pos); // 801A0AD4
    virtual int procPlayerChar1(int pos); // 801A0ADC
    virtual int procPlayerChar2(int pos); // 801A0AE4
    virtual int procPlayer(int pos); // 801A0AEC
    virtual int procAnimal(int pos); // 801A0B88
    virtual int procAnimalA(int pos); // 801A0CAC
    virtual int procAnimalB(int pos); // 801A0E18
    virtual int procAnimalParam(int pos); // 801A0FE4
    virtual int procCatchphrase(int pos); // 801A1188
    virtual int procPlayerNickname(int pos); // 801A120C
    virtual int procImpression(int pos); // 801A12FC
    virtual int procPlayerStarSign(int pos); // 801A1388
    virtual int procAnimalStarSign(int pos); // 801A1420
    virtual int procGreeting(int pos); // 801A14F0
    virtual int procAnimalFlagBegin(int pos); // 801A16BC
    virtual int procPageEnd(int pos); // 801A1874: block 26 (page end): the next newline ends the page
    virtual int procSelect(int pos); // 801A0280: group 7 (select): ends the line; the layout stops here
    virtual int procPlayerGender(int pos); // 801A18D0
    virtual int procWordGender(int pos); // 801A191C
    virtual int procAnimalGender(int pos); // 801A19B4
    virtual int procPlayerElision(int pos); // 801A19BC
    virtual int procAnimalElision(int pos); // 801A1A08
    virtual int procAnimalAElision(int pos); // 801A1A68
    virtual int procAnimalBElision(int pos); // 801A1AEC
    virtual int procAnimalParamElision(int pos); // 801A1B70
    virtual int procPlayerNicknameElision(int pos); // 801A1C30
    virtual int procWordElision(int pos); // 801A1CD8
    virtual int procNumberPlural(int pos); // 801A1D78
    virtual int procColor(int pos); // 801A1E34
    virtual int procScale(int pos); // 801A1EC0

    void start(Comp_c *comp); // 8019FF08: clear() and set mpComp (called by Seq_c::start)
    int insertColorTag(int pos, u8 color); // 801A0334: inserts a text colour tag (4 chars), returns 4
    // 8019FB18..801A071C
    int endLine(int pos); // 8019FFF0: ends a line: scale tag (at pos != 0), then a line offset tag at the line's
                              // start (mLineStart) that moves it down by the ascent the line's largest scale adds
    int insertFeelTag(int pos, u32 id); // 801A02F0: inserts a feel tag 2:id (3 chars; Seq_c::procFeel), returns 3
    int insertLineOffsetTag(int pos, int offset); // 801A0384: inserts tag 0xFF:3 (adds offset to the cursor y, 4 chars), returns 4
    int insertScaleTag(int pos, int scale); // 801A03D0: inserts a scale tag 0xFF:1 (4 chars), returns 4
    // Inserts word's text at pos (setTagWord; the virtual calls are not devirtualized).
    u32 insertWordOf(u32 pos, dScript::Word_c *word, BOOL isArticle) {
        const wchar_t *text = word->getBuffer();
        int len = word->getLength(0);
        return insertWord(pos, text, len, isArticle);
    }

    // 801A071C..801A2168
    int procRandomWordTag(int pos, int kind); // 801A071C: word 4-10: random word of kind, remembered per slot
    int procPlayerChar(int pos, u32 index); // 801A09BC: character index of the player's name
    void pickAnimalA(); // 801A0C2C: best-compatibility villager of the speaker (once)
    void pickAnimalB(); // 801A0D98: worst-compatibility villager of the speaker (once)
    void pickAnimalParam(u8 slot); // 801A0F04: a villager for slot (once)
    void setAnimalWord(dScript::Word_c *word, dAnimal_c *animal, dAnimal_c *speaker); // 801A1FF0

    /* 0x0024 */ wchar_t mBuffer[0x1401];
    /* 0x2828 */ s32 mLineStart; // the current line's start position (its line offset tag goes there)
    /* 0x282C */ u16 mPageLines; // lines in the current page; a new page re-inserts the colour tag
    /* 0x282E */ u16 mScale;    // procScale: the current text scale
    /* 0x2830 */ u16 mMaxScale; // procScale: the largest scale so far (reset to the first unless mLineHasChar)
    /* 0x2834 */ Comp_c *mpComp;
    /* 0x2838 */ u8 mColor;     // procColor: the current text colour
    /* 0x2839 */ u8 mLineHasChar; // the current line has a character (procChar); cleared by procNewLine
    /* 0x283A */ u8 mHasSelect; // a select tag was laid out: the rest uses the base newline / end handlers
    /* 0x283C */ dAnmPersonalID_c mAnimalID; // the last villager named by procAnimalA/B/Param
    /* 0x28FC */ u8 mHasAnimalID;
}; // size 0x2900

} // namespace dMsg
