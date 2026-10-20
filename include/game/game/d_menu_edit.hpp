#ifndef GAME_D_MENU_EDIT_HPP
#define GAME_D_MENU_EDIT_HPP

// Include order sets the RTTI / vtable order of the TU's data (reverse class
// completion): cLineNd_c, mTex::base_c, fBase_c, dLineNdPr_c,
// m2d::order3DMdl_c, dMenuBase_c, order_c, dMenuEdit_c.
#include <game/cLib/c_line.hpp>
#include <game/mLib/m_tex.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_btn.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_select.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/snd/snd_id.hpp>

class dDesign_c;

// The design editor (REL d_menu_editNP, profile MENU_EDIT; menus 0xF, 0x10 and
// 0x11): a 32x32 pixel editor for the player's original designs. The design
// (4 bits per pixel, a 16 color palette) is drawn into a CPU copy of the
// texture (mTex::edit4b_c over two work buffers, one of them the undo copy)
// that the "myDsgn_00" panes show. Tools (pens of three sizes, line, rectangle,
// oval, heart / star stamps, polka dots, stripes, fill, coat), a color window
// and a zoom / scroll / grid / undo button column; the mannequin (dOrgObj_c of
// REL d_org_objNP) previews the design. A pro design (menu 0x10) has four
// textures (front, back, two sleeves) that the parts window picks and swaps.
// RTTI confirms the global class names dMenuEdit_c and dMenuEdit_c::order_c;
// everything else is inferred.
class dMenuEdit_c : public dMenuBase_c {
  public:
    // Layout hit ids (mHover, mPushedBtn, mHeldPart): what the pointer is on.
    enum BTN_e {
        BTN_NONE = -1,
        BTN_GRID = 0,    // N_btnR_00: grid on / off
        BTN_ZOOM_IN = 1, // N_btnR_01
        BTN_ZOOM_OUT = 2,
        BTN_UNDO = 3,
        BTN_CANVAS = 4,  // the drawing area
        BTN_THUMB = 5,   // the thumbnail with the view frame (scroll)
        BTN_PREVIEW = 6, // the mannequin
        BTN_TOOL = 7,    // opens the tool window
        BTN_COLOR = 8,   // opens the color window
        BTN_QUIT = 9,    // the dMenuBtn_c buttons
        BTN_BTN_HIDDEN = 10,
        BTN_DONE = 11,
        BTN_PARTS = 12, // the "other parts" button of the pro design
        BTN_TOOL_BACK = 13,
        BTN_TOOL_FIRST = 14, // 14..25: the tools (TOOL_e + BTN_TOOL_FIRST)
        BTN_TOOL_LAST = 25,
        BTN_COLOR_BACK = 26,
        BTN_COLOR_FIRST = 27, // 27..41: the colors 1..15
        BTN_COLOR_LAST = 41,
        BTN_PALETTE_PREV = 42, // the palette arrows
        BTN_PALETTE_NEXT = 43,
        BTN_PART_FIRST = 44, // 44..47: the parts (front, back, sleeves)
        BTN_PART_LAST = 47,
        BTN_PARTS_BACK = 48,
        BTN_NUM
    };

    static bool isTool(int btn) {
        return btn >= BTN_TOOL_FIRST && btn <= BTN_TOOL_LAST;
    }
    static bool isMenuBtn(int btn) {
        return btn >= BTN_QUIT && btn <= BTN_DONE;
    }
    static bool isColor(int btn) {
        return btn >= BTN_COLOR_FIRST && btn <= BTN_COLOR_LAST;
    }
    static bool isPart(int btn) {
        return btn >= BTN_PART_FIRST && btn <= BTN_PARTS_BACK;
    }

    // The tools, in window order (the balloons "Fine Point" ... "Stripes").
    enum TOOL_e {
        TOOL_PEN_S, // Fine Point
        TOOL_RECT,  // Rectangle
        TOOL_HEART, // Heart Stamp
        TOOL_COAT,  // Coat: fills the whole texture
        TOOL_PEN_M, // Medium Point
        TOOL_OVAL,  // Oval
        TOOL_STAR,  // Star Stamp
        TOOL_DOTS,  // Polka Dots
        TOOL_PEN_L, // Broad Tip
        TOOL_LINE,  // Line
        TOOL_FILL,  // Fill
        TOOL_STRIPES,
        TOOL_NUM
    };

    // The stamps of TOOL_HEART / TOOL_STAR (drawStamp).
    enum STAMP_e {
        STAMP_HEART,
        STAMP_STAR,
    };

    // mZoom: the view shows 32 / 1, 32 / 2 or 32 / 4 pixels wide.
    enum ZOOM_e { ZOOM_1X, ZOOM_2X, ZOOM_4X, ZOOM_NUM };

    // mWinMode: the window opened over the editor.
    enum WIN_e {
        WIN_NONE,
        WIN_TOOL,
        WIN_COLOR,
        WIN_PARTS,
    };

    // mPart / the four textures of a design (dDesign_c::mTexture): the parts
    // buttons are in this order, the textures in ORG_DESIGN_TEX_* order.
    enum PART_e {
        PART_FRONT,
        PART_BACK,
        PART_SLEEVE_L,
        PART_SLEEVE_R,
        PART_NUM
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_DRAW,          // draw the editor
        FLAG_CALC,          // the main layout needs a calc
        FLAG_CALC_TOOL,     // the tool window needs a calc
        FLAG_CALC_COLOR,    // the color window needs a calc
        FLAG_CALC_PARTS,    // the parts window needs a calc
        FLAG_BTN_ANM,       // a button column animation is playing (mBtnAnm)
        FLAG_COLOR_BTN_ANM, // the color button animation is playing
        FLAG_CRSR_ANM,      // the palette arrow animation is playing
        FLAG_TOOL_BTN_ANM,  // the tool button animation is playing
        FLAG_ON_CANVAS,     // the pointer is on the canvas (drawing cursors)
        FLAG_PEN_FRAME,     // draw the pen frame (the brush outline)
        FLAG_ARROW,         // draw the line start arrow
        FLAG_ARROW_END,     // draw the line end arrow
        FLAG_RULER,         // draw the rectangle / oval ruler
        FLAG_RULER_END,     // draw the ruler's far corner
        FLAG_PRO,           // pro design: the parts window exists
        FLAG_GRID,          // the grid is on
        FLAG_DIALOG,        // draw the confirm dialog
        FLAG_SAVED,         // the design was saved (stateClose)
        FLAG_PREVIEW,       // draw the mannequin
        FLAG_FOLLOW,        // the held part follows the pointer
        FLAG_PLACED,        // the held part sits at a fixed position
        FLAG_MOVE,          // the held part flies to its target
        FLAG_FILTER,        // the smoothed pointer is valid
    };

    // mSubState.
    enum SUB_STATE_e {
        SUB_MAIN,         // hover and press the buttons / canvas
        SUB_PAINT,        // a stroke is being drawn (A held on the canvas)
        SUB_SCROLL,       // the thumbnail is dragged
        SUB_DRAG_IN,      // A is held outside, waiting for the canvas
        SUB_TOOL,         // the tool window
        SUB_COLOR,        // the color window
        SUB_PARTS,        // the parts window
        SUB_PARTS_PRESS,  // a part was pressed: click or drag
        SUB_PARTS_DRAG,   // the part follows the pointer
        SUB_PARTS_DROP,   // the part flies to its place
        SUB_PARTS_ANSWER, // the swap / copy answer menu
        SUB_DIALOG,       // the confirm dialog waits for a button
        SUB_COLOR_ANM,    // wait for the color window animation
        SUB_TOOL_ANM,     // wait for the tool window animation
        SUB_BUTTON,       // wait for the pushed dMenuBtn_c button
        SUB_DIALOG_OPEN,  // wait for the dialog's open animation
        SUB_DIALOG_PUSH,  // wait for the dialog's pushed button
        SUB_PARTS_ANM,    // wait for the parts window animation
        SUB_NUM
    };

    // The message codes (SYS_2D_menu.bmg).
    enum {
        MSG_QUIT = 0x1F,
        MSG_CLOSE_NO_SAVE = 0x36, // "Close without saving?"
        MSG_NEVER_MIND = 0xF,
        MSG_YES = 0x23,
        MSG_NO = 0x24,
        MSG_COPY = 0xD9, // "Copy?"
        MSG_SWAP = 0xDA, // "Swap?"
        MSG_DONE = 0x125,
        MSG_QUIT_FACE = 0x129, // "Quit without drawing a face?"
    };

    // The tutorial balloons (SYS_2D_UIballoon.bmg).
    enum {
        BALLOON_TOOLS = 0xD,
        BALLOON_COLORS = 0xE,
        BALLOON_GRID = 0xF,
        BALLOON_ZOOM_IN = 0x10,
        BALLOON_ZOOM_OUT = 0x11,
        BALLOON_UNDO = 0x12,
        BALLOON_CLOSE = 0x13,
        BALLOON_CLOSE_2 = 0x14,
        BALLOON_OTHER_PARTS = 0x51,
        BALLOON_FRONT = 0x52,
        BALLOON_BACK = 0x53,
        BALLOON_SLEEVE_L = 0x54,
        BALLOON_SLEEVE_R = 0x55,
        BALLOON_PARTS_CLOSE = 0x56,
        BALLOON_TOOL_FIRST = 0x57, // + the tool: "Fine Point" ... "Stripes"
    };

    // The answers of the swap menu (mSelect's code).
    enum ANSWER_e {
        ANSWER_CANCEL,
        ANSWER_COPY,
        ANSWER_SWAP,
    };

    // The dialog buttons (mSlct's pushed button).
    enum DIALOG_e {
        DIALOG_NO,
        DIALOG_YES,
    };

    enum {
        TEX_BUF_NUM = 2,        // work buffers: the design and the undo copy
        TEX_SIZE = 0x200,       // one 32x32 4 bit texture
        TEX_BUF_SIZE = 0x800,   // the four textures of a design
        PIXEL_NUM = 32,         // the texture is 32 x 32
        PALETTE_COLOR_NUM = 16, // palette entries, 0 = transparent
        PALETTE_NUM = 16,       // the palettes of the game (getPaletteData)
        FILL_QUEUE_NUM = 64,    // the flood fill's span queue
        PICKUP_SPEED = 0x40,    // the mannequin turns by this each frame
        LYT_PRIORITY = 0x84,    // the priority of the windows' layouts
    };

    // The mannequin's node of the 2D callback list: draws the models of
    // dOrgObj_c between the layouts.
    class order_c : public m2d::order3DMdl_c {
      public:
        virtual void execute(); // .text:0x3EF0
    }; // size 0x10

    // The draw bits of dOrgObj_c::mDrawFlags (dOrgObj_c::DRAW_*) that the
    // preview shows: the shirt, the umbrella, the hat or the cat (the face).
    enum PREVIEW_e {
        PREVIEW_CLOTH,
        PREVIEW_UMB,
        PREVIEW_CAP,
        PREVIEW_CAT,
        PREVIEW_NUM,
    };

    dMenuEdit_c() : mTex(0, 0, NULL) {
    }

    virtual int create();     // .text:0x320
    virtual int doDelete();   // .text:0x7A4
    virtual int stateMain();  // .text:0x844
    virtual int stateAnm();   // .text:0x868
    virtual int stateClose(); // .text:0x8CC
    virtual int execute();    // .text:0x974
    virtual int draw();       // .text:0xA84

    // Sub states (the table in executeSubState).
    void subStateMain();        // .text:0xCAC
    void subStatePaint();       // .text:0xF08
    void subStateScroll();      // .text:0x1294
    void subStateDragIn();      // .text:0x1354
    void subStateTool();        // .text:0x1450
    void subStateColor();       // .text:0x1598
    void subStateParts();       // .text:0x1720
    void subStatePartsPress();  // .text:0x1870
    void subStatePartsDrag();   // .text:0x197C
    void subStatePartsDrop();   // .text:0x1B18
    void subStatePartsAnswer(); // .text:0x1B80
    void subStateDialog();      // .text:0x1C9C
    void subStateColorAnm();    // .text:0x1CEC
    void subStateToolAnm();     // .text:0x1D74
    void subStateButton();      // .text:0x1DFC
    void subStateDialogOpen();  // .text:0x1F14
    void subStateDialogPush();  // .text:0x1F6C
    void subStatePartsAnm();    // .text:0x2018
    void executeSubState();     // .text:0x2058

    void calcAnm(BOOL anming); // .text:0x2090: the animations and layouts
    void startCloseAnm();      // .text:0x2438
    void checkPadMove();       // .text:0x2498: the d-pad moves the view
    BOOL checkShortcuts();     // .text:0x259C: the pad's palette / color keys

    // The design.
    void initDesign();            // .text:0x26B4: loads the design to edit
    void saveDesign();            // .text:0x27A8: writes it back
    void setPalette(u8 palette);  // .text:0x2940: the palette of the colors
    void setZoom(int zoom);       // .text:0x2AE8
    void setGrid(bool on);        // .text:0x2BE8
    void setScroll(int x, int y); // .text:0x2C94: clamps and shows the view
    void attachTexture(
        BOOL keepOrig);        // .text:0x2F58: the panes show the work texture
    void *getTexture();        // .text:0x3000: the texture being edited
    void *getTexture(int tex); // .text:0x3008: a texture of the work buffer
    void copyTexture(int from, int to); // .text:0x3028
    void swapTexture(int a, int b);     // .text:0x3098
    void selectTool(int btn);           // .text:0x313C
    dDesign_c *getDesign(int index); // .text:0x333C: the design the menu edits

    // Buttons, hover and tutorial.
    void setHover(int btn);              // .text:0x33AC
    void setBtnSelect(int btn, bool on); // .text:0x3420
    void selectBtn(int btn);             // .text:0x3728
    void deselectBtn(int btn);           // .text:0x3730
    void clearSelect();                  // .text:0x3738
    void setupBtns();                    // .text:0x37E8
    void showTutorial(int btn);          // .text:0x3848
    void hideTutorial();                 // .text:0x3A8C

    // The mannequin.
    void initPreview();            // .text:0x3A90
    void setPreviewKind(int kind); // .text:0x3B38
    void hidePreview();            // .text:0x3BE4
    void calcPreview();            // .text:0x3C00
    void updatePreviewTexture();   // .text:0x3DBC
    void nextPreviewKind();        // .text:0x3E90

    // Hit tests.
    int hitLayout(f32 scale, mVec2_c pos, m2d::Simple_c *lyt, int rectBase,
                  int rectMax, int circleBase,
                  int circleMax);              // .text:0x3F40
    int hitCheck(f32 scale, mVec2_c pos);      // .text:0x3FDC
    int hitCheckTool(f32 scale, mVec2_c pos);  // .text:0x40D4
    int hitCheckColor(f32 scale, mVec2_c pos); // .text:0x411C
    int hitCheckParts(mVec2_c pos, BOOL grab); // .text:0x41EC

    // The pointer and the canvas.
    void scrollToPointer();                   // .text:0x42C8
    mVec2_c getFilteredPointer();             // .text:0x4394
    void startFilter();                       // .text:0x4408
    void updateFilter();                      // .text:0x445C
    void setPixelFromPos(const mVec2_c *pos); // .text:0x44A8
    mVec2_c getPixelPos(int x, int y, int ofsX,
                        int ofsY); // .text:0x463C: pane position of a pixel

    // Drawing.
    void drawLine(int x0, int y0, int x1, int y1, int color,
                  int size);                         // .text:0x47F4
    void setPixel(int x, int y, int color);          // .text:0x4910
    void drawDot(int x, int y, int color, int size); // .text:0x4958
    BOOL beginTool();   // .text:0x4A4C: A pressed on the canvas
    void backupUndo();  // .text:0x4C64
    void restoreUndo(); // .text:0x4CC4
    void undo();        // .text:0x4D20
    void pickColor();   // .text:0x4D8C
    int findPixel(int x0, int x1, int y, int color);          // .text:0x4DE0
    int findSpanStart(int x, int y, int color);               // .text:0x4E58
    int findSpanEnd(int x, int y, int color);                 // .text:0x4EEC
    void floodFill(int x, int y, int color);                  // .text:0x4F80
    void fillAll(int color);                                  // .text:0x517C
    void drawRect(int x0, int y0, int x1, int y1, int color); // .text:0x51BC
    void drawOval(int x0, int y0, int x1, int y1, int color); // .text:0x52E8
    void drawDots(int color);                                 // .text:0x54A4
    void drawStripes(int color);                              // .text:0x55A8
    void drawStamp(int x, int y, int color, int stamp);       // .text:0x5654

    // The tool and color windows.
    BOOL playColorBtnAnm();         // .text:0x56F8
    void closeColorWin();           // .text:0x574C
    void setColorBtnAnm(int back);  // .text:0x57CC
    void changePalette(int delta);  // .text:0x5838
    void changePaletteAnm(int dir); // .text:0x58A0
    BOOL playToolBtnAnm();          // .text:0x5934
    void closeToolWin();            // .text:0x5988
    void setToolBtnAnm(int back);   // .text:0x5A08
    void setColor(int color);       // .text:0x5A28

    // The parts window of a pro design.
    void setPartsBtnAnm(int back);             // .text:0x5B60
    void initPartsWin();                       // .text:0x5B80
    void initPartsTexture();                   // .text:0x5C24
    void setPartsPalette();                    // .text:0x5C9C
    void selectPart(int part);                 // .text:0x5D08
    int getTexIndex(int part);                 // .text:0x5DE4
    mVec2_c getPartPos(int btn, f32 x, f32 y); // .text:0x5DF8
    void showPart();                           // .text:0x5EB8
    void pickUpPart();                         // .text:0x5EEC
    nw4r::lyt::Pane *getPartPane(int btn);     // .text:0x5F90
    void setupSwapMenu();                      // .text:0x5FE0
    void openSwapMenu(int btn);                // .text:0x6074

    void pushBtn(int btn);   // .text:0x6230
    void initExtra();        // .text:0x6584 (empty)
    void updatePenFrame();   // .text:0x6588: which penFrm_Nx pane
    void clampPixelToView(); // .text:0x6674
    void placePenFrame();    // .text:0x673C
    void placeArrowStart();  // .text:0x67EC
    void placeArrowEnd();    // .text:0x6854
    void placeRulerStart();  // .text:0x68BC
    void placeRuler();       // .text:0x694C

    void playSeUndo();        // .text:0x6AFC
    void playSeRedo();        // .text:0x6B04
    void playSePaintDecide(); // .text:0x6B0C
    void playSePen();         // .text:0x6B14
    void playSePen2();        // .text:0x6B1C
    void playSePen3();        // .text:0x6B24
    void playSeSetStart();    // .text:0x6B2C
    void playSeSetMove();     // .text:0x6B34
    void playSeSetEnd();      // .text:0x6B3C
    void playSeModeChange();  // .text:0x6B44
    void playSeSpoit();       // .text:0x6B4C
    void playSeFrameSlide();  // .text:0x6B54
    void playSeFrameGet();    // .text:0x6B5C
    void playSeFramePlace();  // .text:0x6B64

    // Implicit destructor: .text:0x6B6C.

    virtual int state2() {
        return SUCCEEDED;
    } // .text:0x6E1C
    virtual int state3() {
        return SUCCEEDED;
    } // .text:0x6E14
    virtual void postDraw(MAIN_STATE_e) {
    } // .text:0x6E10
    virtual void deleteReady() {
    } // .text:0x6E0C
    virtual bool createHeap() {
        return true;
    } // .text:0x6E04

    /* 0x0D4 */ m2d::ResAccLoader_c mRes; // Layout/edit_win.arc
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;     // edit_design_win_00
    /* 0x240 */ m2d::Anm_c mBtnAnm;       // btnR_psh
    /* 0x264 */ m2d::SimpleEx_c mToolLyt; // slct_tool_00
    /* 0x2FC */ m2d::Anm_c mToolWinBtnAnm;
    /* 0x320 */ m2d::Anm_c mToolIcnAnm;
    /* 0x344 */ m2d::Anm_c mToolBtnAnm;
    /* 0x368 */ m2d::SimpleEx_c mColorLyt; // slct_clr_00
    /* 0x400 */ m2d::Anm_c mPltBtnAnm;
    /* 0x424 */ m2d::Anm_c mClrBtnAnm;
    /* 0x448 */ m2d::Anm_c mClrCrsrAnm;
    /* 0x46C */ m2d::AnmEx2_c mPreviewAnm; // mnqnIn / mnqnOut (mannequin)
    /* 0x4B0 */ m2d::Anm_c mScrlIconAnm;
    /* 0x4D4 */ m2d::Anm_c mIconPtrnAnm;
    /* 0x4F8 */ m2d::Anm_c mFrmPtrnAnm;
    /* 0x51C */ m2d::SimpleEx_c mToolCrsr;   // tool_crsr_00
    /* 0x5B4 */ m2d::SimpleEx_c mPenFrame;   // penFrm_crsr_00
    /* 0x64C */ m2d::SimpleEx_c mArrowStart; // arrow_crsr_00
    /* 0x6E4 */ m2d::SimpleEx_c mArrowEnd;
    /* 0x77C */ m2d::SimpleEx_c mRulerStart; // ruler_crsr_00
    /* 0x814 */ m2d::SimpleEx_c mRulerEnd;
    /* 0x8AC */ m2d::SimpleEx_c mPartsLyt; // pro_design_win_00
    /* 0x944 */ m2d::Anm_c mPartsBtnAnm;
    /* 0x968 */ m2d::AnmEx_c mChngBtnAnm; // chngBtn_in / chngBtn_out
    /* 0x9A4 */ m2d::Anm_c mPartBtnAnm;   // partsBtnPsh
    /* 0x9C8 */ m2d::Anm_c mClothPtrnAnm;
    /* 0x9EC */ int mHeldPart; // the part pressed / dragged (BTN_PART_*)
    /* 0x9F0 */ dMenuBtn_c mBtn;
    /* 0xF34 */ dMenuSlctB_c mSlct;
    /* 0x1624 */ mTex::edit4b_c mTex; // the work texture in the panes
    /* 0x164C */ int mZoom;           // ZOOM_e
    /* 0x1650 */ int mScrollX;        // the view's offset in pixels
    /* 0x1654 */ int mScrollY;
    /* 0x1658 */ int mHover;     // BTN_e under the pointer
    /* 0x165C */ int mPushedBtn; // the dMenuBtn_c button being pushed
    /* 0x1660 */ int mPixelX;    // the pixel under the pointer
    /* 0x1664 */ int mPixelY;
    /* 0x1668 */ int mPrevX; // the previous pixel (stroke direction)
    /* 0x166C */ int mPrevY;
    /* 0x1670 */ int mStartX; // where the stroke / shape started
    /* 0x1674 */ int mStartY;
    /* 0x1678 */ int mBuf;         // the work buffer being edited
    /* 0x167C */ int mUndoBuf;     // the buffer that holds the undo copy
    /* 0x1680 */ int mTexNo;       // the texture of the design being edited
    /* 0x1684 */ int mPart;        // the part chosen in the parts window
    /* 0x1688 */ int mTool;        // BTN_TOOL_FIRST + TOOL_e
    /* 0x168C */ int mColor;       // the palette index being drawn with
    /* 0x1690 */ int mPreviewKind; // PREVIEW_e
    /* 0x1694 */ void *mOrigTex0;  // the panes' own texture images
    /* 0x1698 */ void *mOrigTex1;
    /* 0x169C */ order_c mOrder;
    /* 0x16AC */ m2d::zBuffClear_c mZClear;
    /* 0x16D4 */ mAng mTurn;                 // the mannequin's turn
    /* 0x16D8 */ int mWinMode;               // WIN_e
    /* 0x16DC */ m2d::ResAccLoader_c mRes2;  // Layout/desi_win.arc
    /* 0x17B0 */ m2d::SimpleEx_c mPickupLyt; // pickUp_design_00
    /* 0x1848 */ dMenuMove_c mMove;
    /* 0x1868 */ int mHoldTimer;
    /* 0x186C */ dSelect::MComp_c mSelect; // the swap / copy menu
    /* 0x2218 */ int mSwapPart;
    /* 0x221C */ int mAnswer;
    /* 0x2220 */ int mPadRepeat;
    /* 0x2224 */ f32 mFilterX; // the smoothed pointer
    /* 0x2228 */ f32 mFilterY;
}; // size 0x222C

#endif
