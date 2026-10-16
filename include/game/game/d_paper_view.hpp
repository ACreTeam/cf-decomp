#ifndef GAME_D_PAPER_VIEW_HPP
#define GAME_D_PAPER_VIEW_HPP

#include <types.h>
#include <game/game/d_m2d.hpp>

class dMail_c;

// Narrow interface to d_paperNP. RTTI establishes the global class name;
// method names and the unexposed storage below are inferred.
class dPaper_c {
public:
    dPaper_c();                         // d_paperNP .text:0x5C
    virtual ~dPaper_c();                // 0x108
    int remove();                      // 0x1A4
    void draw();                       // 0x264
    void execute();                    // 0x2AC
    void setMail(dMail_c *mail, BOOL);   // 0x3F0
    int load();                        // 0x1700
    void create();                     // 0x178C
    void calcAnm();                     // 0x1978

    /* 0x004 */ u8 _0004[0xD4];
    /* 0x0D8 */ m2d::SimpleEx_c *mpLayout;
    /* 0x0DC */ u8 _00DC[0x918];
}; // size 0x9F4

#endif
