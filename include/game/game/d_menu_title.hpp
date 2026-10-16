#ifndef GAME_D_MENU_TITLE_HPP
#define GAME_D_MENU_TITLE_HPP

#include <game/game/d_m2d.hpp>

namespace dMenuTitle {

// Names inferred from the title layout and animation usage. No class RTTI survives.
class Title_c {
public:
    enum FLAG_e { FLAG_DRAW = 1, FLAG_CALC = 2 };

    bool load();                // .text:0x5C
    void create();              // .text:0x68
    void setMessage(u16 msgNo);  // .text:0xDC
    bool remove();              // .text:0xF4
    void calcAnm();             // .text:0x120
    void draw();                // .text:0x16C
    void startAnm(int anm);      // .text:0x1D0
    void setIcon(int icon);      // .text:0x230

    /* 0x000 */ m2d::ResAccLoader_c mRes;
    /* 0x0D4 */ m2d::SimpleEx_c mLyt;
    /* 0x16C */ m2d::Anm_c mWinAnm;
    /* 0x190 */ m2d::Anm_c mIconAnm;
    /* 0x1B4 */ u16 mFlags;
}; // size 0x1B8

} // namespace dMenuTitle

#endif
