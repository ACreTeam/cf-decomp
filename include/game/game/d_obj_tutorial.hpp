#pragma once

// The tutorial balloon process (DOL TU d_obj_tutorial.cpp, .text 801B6A88..801B700C; RTTI "dObjTutorial_c").
// Shows one speech balloon (dBalloon_c) with a word or a message at a screen position, on request
// (d_a_c_uitag). Member and function names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_balloon.hpp>
#include <game/mLib/m_vec.hpp>

namespace dScript {
class Word_c;
}

class dObjTutorial_c : public fBase_c {
public:
    // Message ids of setMsg with a fixed string ("sys_2D/SYS_2D_menu"); TOWN_MSG also sets the town name word.
    enum MSG_e {
        TOWN_MSG = 99,
        MENU_MSG_0 = 1000, // menu string 3
        MENU_MSG_1 = 1001, // menu string 0xEA
        MENU_MSG_2 = 1002, // menu string 5
    };

    // Message ids of setMsg / request in "sys_2D/SYS_2D_UIballoon".
    enum UIBALLOON_MSG_e {
        MSG_EMOTION_BASE = 0x32,    // + the emotion id: "Anger" (0x33), "Shock", "Laughter" ...
        MSG_GENRE_GREETINGS = 0x65, // "Greetings"; the next ones are "Questions", "Commands", "Places", "Comments"
    };

    virtual int create();   // 801B6ADC
    virtual int doDelete(); // 801B6B5C
    virtual int execute();  // 801B6B8C
    virtual int draw();     // 801B6BDC
    virtual ~dObjTutorial_c(); // 801B6F44

    void stateWait(); // 801B6C04: the only state (empty)
    void update();    // 801B6C08: opens / closes the balloon for the current request

    static void hide();      // 801B6D24: drops the request at the next update
    static void setWord(const mVec2_c *pos, const dScript::Word_c *word, int tail); // 801B6D30
    static void setMsg(const mVec2_c *pos, u16 msgId, int tail);                   // 801B6D9C: sys_2D/SYS_2D_UIballoon
    // 801B6E90: a new request (a, b); FALSE if it is the current one or (check) the tutorials are off.
    static BOOL request(int a, int b, BOOL check);
    static void setPriority(u8 priority); // 801B6F14: the balloon's draw priority, applied at the next update
    static BOOL isEnabled();              // 801B6F1C: the tutorial option (dSaveOption::isTutorialOff) is clear

    /* 0x064 */ dBalloon_c mBalloon;
    /* 0x334 */ int mShownA;
    /* 0x338 */ int mShownB;
    /* 0x33C */ u8 mStateIdx;
}; // size 0x340
