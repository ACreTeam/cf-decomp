#pragma once

// Game-side m2d extensions (City Folk only). Source: src/dol/game/d_m2d.cpp
// (.text 800075C0..8000AD84). See notes/d_m2d.txt.
// Class names from the RTTI: m2d::ResAccLoader_c, m2d::SimpleEx_c, m2d::tagProcessor_c, m2d::Word_c,
// m2d::itemWord_c, m2d::stringWord_c, m2d::stringWordL_c, m2d::stringWordLL_c, m2d::AnmEx_c,
// m2d::AnmEx2_c. Member, function and helper names are inferred from behaviour.
#include <game/mLib/m_2d.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_tag_processor.hpp>
#include <game/game/d_line.hpp>
#include <lib/egg/math/eggVector.h>
#include <nw4r/lyt.h>
#include <nw4r/ut.h>

class mAllocator_c;

namespace m2d {

// A resource accessor that loads its archive from the disc into the 2D resource heap.
class ResAccLoader_c : public ResAcc_c {
public:
    ResAccLoader_c(); // 800075C0
    virtual ~ResAccLoader_c(); // 8000760C

    bool load(const char *path); // 80007674: requests the archive and attaches it once loaded
    void remove(); // 800076EC: detaches and frees the archive, releases the loader

    // Creates the 2D resource heap ("2D resource heap (d2d::ResAccLoader_c::create)") and its allocator.
    static bool create(EGG::Heap *heap, size_t size); // 8000774C
    static mAllocator_c *getAllocator(); // 800077C4

    /* 0x0BC */ void *mpArc;
    /* 0x0C0 */ dDvd::loader_c mLoader;
}; // size 0xD4

// A layout with helpers for the game's menus.
class SimpleEx_c : public Simple_c {
public:
    virtual ~SimpleEx_c(); // 8000AA5C
    virtual bool build(const char *lytName, ResAccIf_c *resAcc); // 80007BE4

    void setVisible(const char *name, bool visible); // 80007BE8
    void setAlpha(const char *name, u8 alpha); // 80007C54
    // Loads (or reloads) an animation and binds it at frame 0.
    bool setAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc); // 80007CAC
    // Builds the layout from resAcc and applies the system font.
    bool build(ResAccIf_c *resAcc, const char *lytName); // 80007D30
    void setNumber(const char *name, int value, int digits, dScript::NumberFormat_e format); // 80007D9C
    void setNumberSep(const char *name, int value, int digits, dScript::NumberFormat_e format); // 80007E48
    void setMessage(const char *name, u16 index, const char *group); // 80007F24
    void setWord(const char *name, const dScript::Word_c *word); // 80007FC4
    void setPosZ(const char *name, f32 z); // 80008048
    void addPos(const char *name, f32 x, f32 y); // 800080A0
    const mVec3_c &getPos(const char *name); // 80008114
    void setPos(const char *name, const mVec3_c *pos); // 80008150
    // Moves pane dstName of other onto the screen position of pane srcName of this layout.
    void copyPos(SimpleEx_c *other, const char *srcName, const char *dstName); // 800085D4
    void copyPos(SimpleEx_c *other, const char *name); // 8000868C
    void setPosFrom(const char *srcName, const char *dstName, f32 x, f32 y); // 8000897C
};

// The tag processor of the m2d words: handles the color tag (group 0xFE) and the 0xFF tags.
class tagProcessor_c : public dTagProcessor_c {
public:
    tagProcessor_c(); // 80008C74
    virtual ~tagProcessor_c(); // 80008CB0

    virtual Operation Process(u16 ch, ContextType *pCtx); // 80008D0C
    virtual Operation CalcRect(nw4r::ut::Rect *pRect, u16 ch, ContextType *pCtx); // 80008DE0

    void setColor(ContextType *pCtx, u8 *data); // 80008E58
};

// A word that is shown in a text box (mpTextBox).
class Word_c : public dString::WordBase_c {
public:
    Word_c(); // 80008F80
    virtual ~Word_c(); // 80008FC4

    void setTextBox(nw4r::lyt::TextBox *textBox); // 8000901C
    void toFixedSpaces(); // 80009084
    void fromFixedSpaces(); // 80009134
    void update(); // 800091E0
    f32 calcWidth(const wchar_t *str, int len); // 8000924C

    static int getLength(const wchar_t *str); // 800092CC: characters, tags not counted
    static int getSize(const wchar_t *str); // 80009338: characters, tags counted
    static int getSizeAt(const wchar_t *str, int pos); // 800093AC: size of the first pos characters

    virtual int procColor(int pos); // 80009444
    virtual int procRubyTag(int pos); // 8000948C

    int getPosAt(f32 x); // 80009490
    int getPosAtScreenX(f32 x); // 800095BC
    f32 getScreenXLeft(int pos); // 800096E0
    f32 getScreenXRight(int pos); // 80009790
    f32 getScreenX(int pos); // 8000986C
    BOOL insertColored(const wchar_t *str, int pos, const u8 *color0, const u8 *color1); // 800098A8
    BOOL insertColor(int pos, const u8 *color0, const u8 *color1, BOOL before); // 80009AA0
    int getLineNum(); // 80009C34

    /* 0x24 */ nw4r::lyt::TextBox *mpTextBox;
}; // size 0x28

class itemWord_c : public Word_c {
public:
    itemWord_c(); // 80009CF8
    virtual ~itemWord_c(); // 80009D3C
    virtual u32 getBufferSize(); // 80009D94
    virtual wchar_t *getBuffer(); // 80009D9C

    /* 0x28 */ wchar_t mBuffer[0x11];
};

class stringWord_c : public Word_c {
public:
    stringWord_c(); // 80009DA4
    virtual ~stringWord_c(); // 80009DE8
    virtual u32 getBufferSize(); // 80009E40
    virtual wchar_t *getBuffer(); // 80009E48

    /* 0x28 */ wchar_t mBuffer[0x65];
};

class stringWordL_c : public Word_c {
public:
    stringWordL_c(); // 80009E50
    virtual ~stringWordL_c(); // 80009E94
    virtual u32 getBufferSize(); // 80009EEC
    virtual wchar_t *getBuffer(); // 80009EF4

    /* 0x28 */ wchar_t mBuffer[0xC9];
};

class stringWordLL_c : public Word_c {
public:
    stringWordLL_c(); // 80009EFC
    virtual ~stringWordLL_c(); // 80009F40
    virtual u32 getBufferSize(); // 80009F98
    virtual wchar_t *getBuffer(); // 80009FA0

    /* 0x28 */ wchar_t mBuffer[0x190];
};

// An animation that switches between a table of animation files.
class AnmEx_c : public Anm_c {
public:
    // Inferred stop-status type; enumerator names are unofficial.
    enum StopState_e {
        RUNNING,
        STOPPED,
    };

    AnmEx_c(); // 80009FA8
    virtual ~AnmEx_c(); // 8000A004

    virtual void stop(); // 8000A228: unbinds the current animation
    virtual void change(int idx); // 8000A27C: loads and binds animation idx

    void init(SimpleEx_c *lyt, ResAccIf_c *resAcc, const char **names, int num); // 8000A05C
    bool play(); // 8000A07C
    bool setNext(int idx); // 8000A130
    BOOL isEnd(); // 8000A160
    void start(int idx); // 8000A1D0
    void startEnd(int idx); // 8000A1E4

    /* 0x24 */ SimpleEx_c *mpLyt;
    /* 0x28 */ const char **mpNames;
    /* 0x2C */ ResAccIf_c *mpResAcc;
    /* 0x30 */ int mNum;
    /* 0x34 */ int mCurIdx;
    /* 0x38 */ int mNextIdx;
}; // size 0x3C

// An AnmEx_c that can retarget its animations to numbered panes.
class AnmEx2_c : public AnmEx_c {
public:
    enum FLAG_e {
        FLAG_NUMBER = 1, // rename the animation targets to mNumber
        FLAG_END = 2, // start at the last frame
    };

    AnmEx2_c(); // 8000A2D8
    virtual ~AnmEx2_c(); // 8000A31C

    virtual void stop(); // 8000A380
    virtual void change(int idx); // 8000A3F4

    void setNumber(int number); // 8000A374
    void onFlag(u32 flag); // 8000A3CC
    BOOL isFlag(u32 flag); // 8000A3DC

    /* 0x3C */ int mNumber;
    /* 0x40 */ u32 mFlags;
}; // size 0x44

// A node of the post-draw callback list.
class callbackNode_c : public dPriLineNd_c {
public:
    virtual void execute() = 0;
};

// A node of the capture list: copies the frame buffer into a texture after drawing.
class captureNode_c : public dPriLineNd_c {
public:
    void execute(); // 8000A5D0
    void clear(); // 8000A6B8
    void set(u16 id, int width, int height, void *buffer); // 8000A6D0
    void entry(); // 8000A724
    void remove(); // 8000A728

    /* 0x0C */ void *mpBuffer;
    /* 0x10 */ int mWidth;
    /* 0x14 */ int mHeight;
    /* 0x18 */ u16 mID;
};

void initCallbackList(); // 8000A4E8
void addCallback(callbackNode_c *node); // 8000A4F0
void executeCallbacks(); // 8000A4FC
void initCaptureList(); // 8000A55C
void addCapture(captureNode_c *node); // 8000A564
void removeCapture(captureNode_c *node); // 8000A570
void executeCaptures(); // 8000A57C

// Moves a position to a target in a number of steps, slowing down over the last mDecel steps.
class moveCalc_c {
public:
    void reset(); // 8000A72C
    bool calc(); // 8000A738: true when done
    void set(const mVec2_c *from, const mVec2_c *to, int maxSteps, int minSteps, int decel, f32 speed); // 8000A7EC

    /* 0x00 */ mVec2_c mPos;
    /* 0x08 */ mVec2_c mStep;
    /* 0x10 */ int mDecel;
    /* 0x14 */ int mCount;
};

// Free helpers.
f32 getDefaultScale(); // inferred stripped helper; see notes/d_m2d.txt
int getNameNumber(const char *name, int digits); // 800077CC
int isInPane(nw4r::lyt::Pane *pane, void *arg); // 80007878
bool isCircleButton(nw4r::lyt::Pane *pane); // 80007A44
bool isRectButton(nw4r::lyt::Pane *pane); // 80007B14
f32 calcTextWidth(nw4r::lyt::TextBox *textBox, const wchar_t *str, int len); // 80008194
mVec3_c getTotalPos(nw4r::lyt::Pane *pane); // 80008308
void setNameNumber(char *name, void *arg); // 8000837C
void setAnmNumber(Anm_c *anm, int number); // 800083F8
nw4r::lyt::Material *getMaterial(nw4r::lyt::Pane *pane); // 80008468
void fn_80008478(nw4r::lyt::Pane *pane); // 80008478
void fn_8000849C(nw4r::lyt::Pane *pane); // 8000849C
void getTevColors(nw4r::lyt::Pane *pane, u8 *white, u8 *black); // 800084C0
void setTevColors(nw4r::lyt::Pane *pane, const u8 *white, const u8 *black); // 8000855C
void getScreenPos(mVec3_c *pos, nw4r::lyt::Pane *pane); // 80008694
void getScreenLeftTop(mVec3_c *pos, nw4r::lyt::Pane *pane); // 80008724
void getScreenRightBottom(mVec3_c *pos, nw4r::lyt::Pane *pane); // 800087F4
bool isInPaneRect(nw4r::lyt::Pane *pane, f32 margin, const mVec2_c *pos); // 800088C4
void rgb565ToColor(u16 color, GXColorS10 *out); // 80008A20
u8 clampColor(int value); // 80008A50
void toColor(const GXColorS10 *in, u8 *out); // 80008A78
int hitButton(const mVec2_c *pos, Simple_c *lyt, int rectBase, int rectMax, int circleBase, int circleMax, f32 scale); // 80008ADC
void setVisible(nw4r::lyt::Pane *root, const char **names, int num, bool visible); // 80008BF4
int writeColorTag(wchar_t *buf, const u8 *color0, const u8 *color1); // 80008EF4

} // namespace m2d
