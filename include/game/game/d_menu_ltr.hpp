#ifndef GAME_D_MENU_LTR_HPP
#define GAME_D_MENU_LTR_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>

// The letter window of the item menus (REL d_menu_ltrNP): the player's ten
// letters as icons, the letter/back button and a held letter. No RTTI survives:
// the namespace follows the file name, and all class, method and member names
// are inferred (the item window in d_menu_item is the model).
namespace dMenuLtr {

// Not derived from m2d::SimpleEx_c: the layout is the first member. A derived class
// would get a vtable of its own, and the menus that construct the icons (d_menu_mslct)
// store the SimpleEx_c vtable.
class Icon_c {
  public:
    enum BASE_ANM_e { BASE_ANM_NORMAL, BASE_ANM_SELECTED, BASE_ANM_COUNT };

    void create(m2d::ResAccIf_c *res);       // .text:0x9F8
    void setPos(const mVec2_c *pos);         // .text:0xAA0
    void calcLayout();                       // .text:0xAB4
    void drawLayout();                       // .text:0xAC4
    void setRootPos(const mVec3_c *pos);     // .text:0xAC8
    void setScale(const mVec2_c &scale);     // .text:0xB0C
    void setAlpha(u8 alpha);                 // .text:0xB48
    const mVec3_c &getRootPos();             // .text:0xB78
    void setSelected(bool selected);         // .text:0xBB4
    void setNoChoice(bool visible);          // .text:0xC34
    void setMark(bool visible);              // .text:0xC44
    void setMail(dMail_c *mail, int unused); // .text:0xCA4
    int getTexFrame(dMail_c *mail);          // .text:0xD80
    bool calcVanish();                       // .text:0xE3C
    void calcRotate();                       // .text:0xE9C

    /* 0x000 */ m2d::SimpleEx_c mLyt;
    /* 0x098 */ m2d::Anm_c mTexAnm; // the letter picture (getTexFrame)
    /* 0x0BC */ m2d::Anm_c mVanishAnm;
    /* 0x0E0 */ m2d::Anm_c mBaseAnm; // BASE_ANM_e
    /* 0x104 */ m2d::Anm_c mRotateAnm;
    /* 0x128 */ u16 mFlags; // only cleared here
}; // size 0x12C

class Window_c {
  public:
    enum { LETTER_COUNT = 10 };
    enum WIN_ANM_e {
        WIN_ANM_OPEN,
        WIN_ANM_CLOSE,
        WIN_ANM_OUT,
        WIN_ANM_IN,
        WIN_ANM_POST_IN,
        WIN_ANM_POST_OUT,
        WIN_ANM_COUNT
    };
    enum BTN_ANM_e {
        BTN_ANM_LETTER_PUSH, // plays the decide SE
        BTN_ANM_LETTER_TO_BACK,
        BTN_ANM_BACK_PUSH, // plays the cancel SE
        BTN_ANM_BACK_TO_LETTER,
        BTN_ANM_COUNT
    };

    void calcLayouts();                          // .text:0x5C
    void create();                               // .text:0x130
    bool load();                                 // .text:0x238
    void drawWindow();                           // .text:0x244
    void drawIcons();                            // .text:0x24C
    void drawButton();                           // .text:0x298
    bool remove();                               // .text:0x2A0
    void setMark(int letter, bool visible);      // .text:0x2CC
    void setNoChoice(int letter, bool visible);  // .text:0x2E4
    void setSelected(int letter, bool selected); // .text:0x2FC
    void startWindowAnm(int anm, bool toEnd);    // .text:0x314
    void startButtonAnm(int anm, bool toEnd);    // .text:0x378
    int hitLetter(const mVec2_c *pos, bool setOffset,
                  float scale);                                // .text:0x434
    int hitButton(const mVec2_c *pos, float scale);            // .text:0x53C
    void setPos(m2d::SimpleEx_c *base);                        // .text:0x5F0
    static int positionIcon(nw4r::lyt::Pane *pane, void *arg); // .text:0x64C
    void setFaceButton(bool visible);                          // .text:0x6E4
    bool calcButtonAnm();                                      // .text:0x6F8
    bool calcWindowAnm();                                      // .text:0x740
    void playWindowAnm();                                      // .text:0x788
    bool calcVanish(int letter);                               // .text:0x7E8
    void calcRotate();                                         // .text:0x7F8
    void setMail(int letter, dMail_c *mail, int unused);       // .text:0x844
    mVec2_c getLetterPos(int letter, float dx, float dy);      // .text:0x860
    void drawHeldLetter();                                     // .text:0x8E8
    void calcHeldLetter();                                     // .text:0x8F0
    void setHeldMail(const dMail_c *mail);                     // .text:0x934
    dMail_c *getHeldMail();                                    // .text:0x974
    void clearHeldMail();                                      // .text:0x97C
    m2d::ResAccIf_c *getRes();              // .text:0x9BC
    bool isMoveEnd();                       // .text:0x9C0
    bool calcMove();                        // .text:0x9C8
    void startMove(const mVec2_c &pos);     // .text:0x9D0
    void calcFollow();                      // .text:0x9D8
    void startFollow();                     // .text:0x9E0
    void setMoveOffset(const mVec2_c &ofs); // .text:0x9E8
    void setMovePos(const mVec2_c &pos);    // .text:0x9F0

    /* 0x0000 */ m2d::ResAccLoader_c mRes;
    /* 0x00D4 */ m2d::SimpleEx_c mWinLyt;
    /* 0x016C */ m2d::SimpleEx_c mBtnLyt;
    /* 0x0204 */ Icon_c mIcons[LETTER_COUNT];
    /* 0x0DBC */ m2d::Anm_c mWinAnm;
    /* 0x0DE0 */ m2d::Anm_c mBtnAnm;
    /* 0x0E04 */ Icon_c mHeldIcon;
    /* 0x0F30 */ dMail_c mHeldMail;
    /* 0x12C0 */ dMenuMove_c mMove;
}; // size 0x12E0

} // namespace dMenuLtr

#endif
