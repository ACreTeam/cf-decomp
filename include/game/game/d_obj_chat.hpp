#ifndef GAME_D_OBJ_CHAT_HPP
#define GAME_D_OBJ_CHAT_HPP

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

// Helper, field and function names are inferred from the chat UI's behaviour.
namespace dObjChat {

enum TextKind_e {
    TEXT_KIND_PLAYER,
    TEXT_KIND_SYSTEM,
};

// Preset-menu categories and sys_2D/SYS_2D_Chat message IDs.
enum PresetGroup_e {
    PRESET_GROUP_GREETING,
    PRESET_GROUP_QUESTION,
    PRESET_GROUP_INVITATION,
    PRESET_GROUP_NAME_AND_DESTINATION,
    PRESET_GROUP_REACTION,
    PRESET_GROUP_REGISTERED_MEMBER,
    PRESET_GROUP_NUM,
    PRESET_GROUP_NONE = 0xFF,
};

enum PresetText_e {
    TEXT_GOOD_MORNING = 1,
    TEXT_GOOD_AFTERNOON,
    TEXT_GOOD_EVENING,
    TEXT_IM_HERE,
    TEXT_NICE_TO_MEET_YOU,
    TEXT_WELCOME,
    TEXT_IM_BACK,
    TEXT_WELCOME_BACK,
    TEXT_BYE_BYE,
    TEXT_GOOD_NIGHT,
    TEXT_WHAT,
    TEXT_WHATCHA_DOING,
    TEXT_WHERE_ARE_YOU,
    TEXT_WHERE_SHOULD_I_GO,
    TEXT_WHOSE_IS_THIS,
    TEXT_WHERE_TO,
    TEXT_WRAP_IT_UP,

    TEXT_HEY = 20,
    TEXT_LETS_GO,
    TEXT_COME_ON,
    TEXT_GATHER_ROUND,
    TEXT_OVER_HERE,
    TEXT_YOU_CAN_HAVE_THIS,
    TEXT_GIMME,
    TEXT_WANNA_TRADE,
    TEXT_LETS_GO_FISHING,
    TEXT_LETS_CATCH_BUGS,
    TEXT_SHOW_ME_YOUR_HOUSE,
    TEXT_TAKE_A_BREAK,
    TEXT_SAY_THAT_AGAIN,
    TEXT_GOTTA_GO,
    TEXT_WRAPPING_UP,

    TEXT_PLAYER_NAME_0 = 41,
    TEXT_PLAYER_NAME_1,
    TEXT_PLAYER_NAME_2,
    TEXT_PLAYER_NAME_3,
    TEXT_TO_TOWN_HALL,
    TEXT_TO_STORE,
    TEXT_TO_TAILOR,
    TEXT_TO_MUSEUM,
    TEXT_TO_CAFE,
    TEXT_TO_OBSERVATORY,
    TEXT_TO_GATE,
    TEXT_TO_BEACH,
    TEXT_TO_RIVER,
    TEXT_TO_HOUSE,

    TEXT_YES = 70,
    TEXT_OK,
    TEXT_THANKS,
    TEXT_I_DID_IT,
    TEXT_NO_WAY,
    TEXT_SORRY,
    TEXT_NO_THANKS,
    TEXT_CUTE,
    TEXT_COOL,
    TEXT_AWESOME,
    TEXT_KEEP_TRYING,
    TEXT_I_WON,
    TEXT_I_LOST,
    TEXT_I_CAUGHT_ONE,
    TEXT_IT_GOT_AWAY,

    TEXT_REGISTERED_MEMBER = 90,
    TEXT_REGIONAL_NICE_TO_MEET_YOU,
    TEXT_REGIONAL_WELCOME,
    TEXT_REGIONAL_WELCOME_BACK,
    TEXT_REGIONAL_WHAT,
    TEXT_REGIONAL_WHATCHA_DOING,
    TEXT_REGIONAL_WHERE_ARE_YOU,
    TEXT_REGIONAL_WHOSE_IS_THIS,
    TEXT_REGIONAL_WRAP_IT_UP,
    TEXT_REGIONAL_COME_ON,
    TEXT_REGIONAL_GATHER_ROUND,
    TEXT_REGIONAL_YOU_CAN_HAVE_THIS,
    TEXT_REGIONAL_GIMME,
    TEXT_REGIONAL_WANNA_TRADE,
    TEXT_REGIONAL_SHOW_ME_YOUR_HOUSE,
    TEXT_REGIONAL_SAY_THAT_AGAIN,
    TEXT_REGIONAL_SORRY,
    TEXT_REGIONAL_KEEP_TRYING,
};

enum {
    MEMBER_NUM = 4,
    TEXT_BUFFER_SIZE = 101,
    PRESET_INDEX_NONE = 0xFF,
    GREETING_PRESET_NUM = 10,
    QUESTION_PRESET_NUM = 7,
    INVITATION_PRESET_NUM = 15,
    DESTINATION_PRESET_NUM = 9,
    REACTION_PRESET_NUM = 15,
    PRESET_NAME_WORD_SLOT = 4,
    DISPLAY_FRAMES = 1800,
};

class Request_c {
public:
    enum Flag_e { PENDING = 1, FIXED = 2, SYSTEM = 4, PRESET = 8 };
    void clear(); // 801B1DC4
    void calc(); // 801B1E28
    BOOL isPending(); // 801B1E84
    void expire(); // 801B1EAC
    bool isExpired(); // 801B1EB8
    BOOL isSystem(); // 801B1ECC
    void startTimer(); // 801B1EF4
    void setText(const wchar_t *text, u32 length, u32 kind); // 801B1F14: TextKind_e
    void setPreset(int group, int index); // 801B1FC0: PresetGroup_e
    bool hasSound(); // 801B1FE4
    void getWord(dScript::Word_c *word); // 801B1FF8

    /* 0x00 */ int mTimer;
    /* 0x04 */ int mDepthOrder;
    /* 0x08 */ wchar_t mText[TEXT_BUFFER_SIZE];
    /* 0xD2 */ u16 mFlags;
    /* 0xD4 */ u16 mOrder;
    /* 0xD6 */ u8 mGroup; // PresetGroup_e
    /* 0xD7 */ u8 mIndex;
}; // size 0xD8

class Balloon_c {
public:
    enum State_e {
        STATE_INIT,
        STATE_DISABLED,
        STATE_MISSING,
        STATE_REMOTE,
        STATE_PLAYER,
        STATE_FIXED,
    };
    enum Anm_e { ANM_IN, ANM_OUT, ANM_NUM };
    enum Color_e { COLOR_BLUE, COLOR_YELLOW, COLOR_ORANGE, COLOR_PURPLE, COLOR_NUM };
    enum TextStyle_e { TEXT_STYLE_FREE, TEXT_STYLE_PRESET, TEXT_STYLE_NUM };

    Balloon_c(); // 801B0B94
    ~Balloon_c(); // 801B32F0
    void init(m2d::ResAccLoader_c *res, int member); // 801B2064
    void calc(); // 801B219C
    void move(); // 801B22AC
    void draw(); // 801B23F0
    BOOL isActive(); // 801B242C
    f32 getTargetY(); // 801B2494
    f32 getY(); // 801B249C
    f32 getScale(); // 801B24A4
    void setTargetY(f32 y); // 801B24AC
    State_e getState(); // 801B24B4
    void drawLayout(); // 801B25BC
    void calcPlayerPos(); // 801B2620
    void calcFixedPos(); // 801B276C
    void calcRemotePos(); // 801B27D4
    void calcAlpha(); // 801B283C
    void clampPos(); // 801B2910
    void clampY(); // 801B2B2C
    bool isTopObstructed(); // 801B2CC4
    bool isBottomObstructed(); // 801B2D10
    BOOL isEnabled(); // 801B2D14
    void updateEnabled(); // 801B2D2C
    void updateText(); // 801B2DDC
    void setWidth(); // 801B2F60
    void setName(const wchar_t *name); // 801B3014
    void setColor(int color); // 801B3138: Color_e, same order as network members
    void setTextStyle(int style); // 801B3188: TextStyle_e
    void initAnm(); // 801B31D4
    void playAnm(); // 801B3228
    enum Flag_e { DIRTY = 1, ENABLED = 2, SHOW = 8 };
    /* 0x000 */ m2d::ResAccLoader_c *mpRes;
    /* 0x004 */ m2d::SimpleEx_c mLyt;
    /* 0x09C */ m2d::Anm_c mColorAnm;
    /* 0x0C0 */ m2d::stringWord_c mWord;
    /* 0x1B4 */ m2d::AnmEx_c mAnm;
    /* 0x1F0 */ m2d::Anm_c mTextAnm;
    /* 0x214 */ mVec2_c mTargetPos;
    /* 0x21C */ mVec2_c mPos;
    /* 0x224 */ f32 mTargetScale;
    /* 0x228 */ f32 mScale;
    /* 0x22C */ f32 mTargetAlpha;
    /* 0x230 */ f32 mAlpha;
    /* 0x234 */ u8 _234[4];
    /* 0x238 */ int mMember;
    /* 0x23C */ int mRequest;
    /* 0x240 */ f32 mBaseWidth;
    /* 0x244 */ f32 mNameWidth;
    /* 0x248 */ u16 mState; // State_e
    /* 0x24A */ u16 mFlags;
}; // size 0x24C

void clear(); // 801B1164
void fixPositions(); // 801B11B4
void unfixPositions(); // 801B11F0
void postText(const wchar_t *text, u8 kind); // 801B122C: TextKind_e, one-byte packet field
void receiveText(const void *data, u32 size, int member); // 801B12B0
void postPreset(int group, int index); // 801B133C: PresetGroup_e
void receivePreset(const void *data, int member); // 801B13C8
void setText(int member, const wchar_t *text, u32 length, int kind); // 801B13DC
void setPreset(int member, int group, int index); // 801B144C
void updateOrder(int member); // 801B14AC
void expireAll(); // 801B1544
void playSound(f32 x); // 801B1594
BOOL useRegionalPresets(); // 801B15C4: release stub, always false
void loadPreset(dScript::Word_c *word, u16 index); // 801B15CC
void loadPlayerPreset(dScript::Word_c *word, int index); // 801B1854
void loadHomePreset(dScript::Word_c *word, int index); // 801B1910
void loadAnimalPreset(dScript::Word_c *word, int index); // 801B19BC
void loadNamePreset(dScript::Word_c *word, int index); // 801B1A8C
void loadMemberPreset(dScript::Word_c *word, int member); // 801B1BCC
int getNamePresetNum(); // 801B1C18
void getPreset(dScript::Word_c *word, int group, int index); // 801B1CE4
int getPresetNum(int group); // 801B1D5C

} // namespace dObjChat

// Global scope established by RTTI "dObjChat_c".
class dObjChat_c : public fBase_c {
public:
    enum State_e { STATE_MAIN };
    virtual int create(); // 801B0C00
    virtual int doDelete(); // 801B0CA0
    virtual int execute(); // 801B0CD0
    virtual int draw(); // 801B10E8
    virtual ~dObjChat_c(); // 801B326C
    void separateBalloons(); // 801B0DA4
    BOOL separatePair(int first, int second); // 801B0E3C
    void updateDepthOrder(); // 801B1010
    void stateMain(); // 801B1160: empty state handler

    /* 0x064 */ m2d::ResAccLoader_c mRes;
    /* 0x138 */ dObjChat::Balloon_c mBalloons[dObjChat::MEMBER_NUM];
    /* 0xA68 */ u8 mState; // State_e
}; // size 0xA6C

#endif
