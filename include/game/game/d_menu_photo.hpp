#ifndef GAME_D_MENU_PHOTO_HPP
#define GAME_D_MENU_PHOTO_HPP

#include <game/game/d_m2d.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_slct_b.hpp>
#include <game/game/d_select.hpp>
#include <game/mLib/m_vec.hpp>

// The photo menu of the bottom UI panel (REL d_menu_photoNP, profile
// MENU_PHOTO, dMenuMng_c::MENU_PHOTO, opened by dAcUitag::TAB_PHOTO): the
// window of Layout/photo.arc shows the photo that was taken in town (dPhoto's
// capture texture, copied into the picture pane photo_00), or the NoPhoto
// picture and the text "You have no photos.", and the "SD Card" button. The
// button opens a small answer window (dSelect::MComp_c): "Save Photo", "Erase
// All" and "Quit". Saving encodes the photo to a JPEG (dPhoto::requestEncode)
// and writes it to the SD card (dScreenshot: mount, make the folders, write);
// erasing deletes all the photos of the game on the card. Both ask for a
// confirmation first, and every step reports its failure with a message of
// SYS_SDcard.bmg in a dialog (dMenuSlctB_c). The tabs of the bottom panel
// (dAcUitag) stay reachable. RTTI confirms the global class name dMenuPhoto_c;
// everything else is inferred.
class dMenuPhoto_c : public dMenuBase_c {
  public:
    // Sub states (setSubState, the tables in executeSubState and
    // executeCloseRequest).
    enum SUB_STATE_e {
        SUB_MAIN,
        SUB_PUSH,        // the SD card button's push animation
        SUB_MOUNT,       // waits for the SD card to be mounted
        SUB_DIALOG,      // a dialog opens, waits for its button
        SUB_DIALOG_PUSH, // the dialog button's push animation
        SUB_DIALOG_END,  // the dialog closes: the confirmation or the end
        SUB_SAVE_ENCODE, // waits for the JPEG encode
        SUB_SAVE,        // the SD card steps of the save
        SUB_SAVE_WAIT,   // the "saving" message stays a minimum time
        SUB_SAVE_DONE,   // the message closes, "complete" opens
        SUB_ERROR,       // the message closes, the error opens
        SUB_ERASE,       // the SD card steps of the erase
        SUB_ERASE_WAIT,  // the "erasing" message stays a minimum time
        SUB_ERASE_DONE,  // the message closes, "erased" opens
        SUB_ANSWER,      // waits for the answer window
        SUB_NUM,
    };

    // The answers of the answer window (dSelect::BaseComp_c::mCode).
    enum ANSWER_e {
        ANSWER_SAVE,
        ANSWER_ERASE,
        ANSWER_QUIT,
    };

    // The animations of the SD card button (mBtnAnm).
    enum BTN_ANM_e {
        BTN_ANM_OFF, // not selected (_out)
        BTN_ANM_ON,  // selected (_in)
        BTN_ANM_PUSH,
        BTN_ANM_NUM,
    };

    // The buttons of the dialog (mSlct): a question has two, a message one.
    enum DIALOG_BTN_e {
        DIALOG_BTN_NO,
        DIALOG_BTN_YES,
        DIALOG_BTN_OK,
    };

    // Bit indexes for dMenuBase_c::onFlag / isFlag.
    enum FLAG_BIT_e {
        FLAG_BTN_ANM_PLAYED, // set while the button animation plays (never
                             // read)
        FLAG_DRAW,           // the layout is drawn
        FLAG_BTN_ANM,        // the button animation is playing
        FLAG_DIALOG,         // the dialog is drawn
        FLAG_NO_PHOTO,       // there is no photo to save
    };

    // Minimum frames of the "saving" / "erasing" messages.
    enum { MESSAGE_FRAMES = 120 };

    // The tab of the bottom panel under the pointer / chosen by the pad: none.
    enum { TAB_NONE = -1 };

    // The result of m2d::hitButton: nothing under the pointer.
    enum { BTN_NONE = -1 };

    // mStep before the first step of an SD card job.
    enum { STEP_NONE = 0 };

    // Messages of SYS_2D_menu.bmg (the code is the INF1 index).
    enum {
        MSG_SD_CARD = 0x31,   // the button
        MSG_NO_PHOTOS = 0x32, // in place of the photo
        MSG_OK = 0xEC,
        MSG_SAVE_PHOTO = 0x15B, // the answers
        MSG_ERASE_ALL = 0x15C,
        MSG_QUIT = 0x15D,
    };

    // Messages of sys_SELECT/SYS_Select.bmg (the dialog buttons).
    enum {
        SEL_MSG_SAVE = 0xD,
        SEL_MSG_NEVER_MIND = 0xE,
        SEL_MSG_ERASE = 0x11,
        SEL_MSG_CANCEL = 0x12,
    };

    // Messages of sys_2D/SYS_SDcard.bmg.
    enum {
        SD_MSG_NONE = 0,
        SD_MSG_SAVE_ASK = 0x1,
        SD_MSG_SAVING = 0x2,
        SD_MSG_SAVED = 0x3,
        SD_MSG_UNABLE_TO_SAVE = 0x4,
        SD_MSG_ERASE_ASK = 0x5,
        SD_MSG_ERASING = 0x6,
        SD_MSG_ERASED = 0x7,
        SD_MSG_ERASE_ERROR = 0x8,
        SD_MSG_NO_CARD = 0xB,
        SD_MSG_UNKNOWN_DEVICE = 0xC,
        SD_MSG_WRITE_PROTECTED = 0xD,
        SD_MSG_NO_SPACE = 0xF,
        SD_MSG_TOO_MANY_FOLDERS = 0x10,
        SD_MSG_TOO_MANY_FILES = 0x11,
        SD_MSG_NO_PHOTO_DATA = 0x12,
        SD_MSG_NO_PHOTO_TAKEN = 0x13,
    };

    // The steps of the SD card work (mStep, after the mount): save.
    enum SAVE_STEP_e {
        SAVE_STEP_MOUNT = 1,
        SAVE_STEP_MAKE_ROOT,
        SAVE_STEP_MAKE_PATH,
        SAVE_STEP_WRITE,
        SAVE_STEP_NUM,
    };
    // ... and erase.
    enum ERASE_STEP_e {
        ERASE_STEP_MOUNT = 1,
        ERASE_STEP_CHECK,
        ERASE_STEP_DELETE,
        ERASE_STEP_NUM,
    };

    virtual int create();    // .text:0x110
    virtual int doDelete();  // .text:0x2BC
    virtual int stateMain(); // .text:0x350
    virtual int execute();   // .text:0x3A0
    virtual int draw();      // .text:0x41C

    void subStateMain();                      // .text:0x484
    void subStatePush();                      // .text:0x5E0
    void subStateMount();                     // .text:0x628
    void subStateDialog();                    // .text:0x6E8
    void subStateDialogPush();                // .text:0x748
    void subStateDialogEnd();                 // .text:0x7AC
    void subStateSaveEncode();                // .text:0x894
    void subStateSave();                      // .text:0x900
    void subStateSaveWait();                  // .text:0x9CC
    void subStateSaveDone();                  // .text:0xA30
    void subStateError();                     // .text:0xA78
    void subStateErase();                     // .text:0xB40
    void subStateEraseWait();                 // .text:0xC38
    void subStateEraseDone();                 // .text:0xC9C
    void subStateAnswer();                    // .text:0xAC0
    BOOL subStateCloseRequest();              // .text:0xCE4
    BOOL subStateCloseDialog();               // .text:0xD1C
    BOOL subStateCloseAnswer();               // .text:0xD78
    BOOL executeCloseRequest();               // .text:0xDC8
    void executeSubState();                   // .text:0xF2C
    void updateBtnAnm();                      // .text:0xF64
    int hitUitagTab(const mVec2_c *pos);      // .text:0xFC4
    BOOL selectUitagTab(int tab, BOOL delay); // .text:0x1028
    void startCloseAnm();                     // .text:0x1078
    void setTexts();                          // .text:0x1094
    void initAnms();                          // .text:0x10EC
    BOOL initPhoto();                         // .text:0x1144
    void showMessage(u16 msg, BOOL error);    // .text:0x123C
    void openSaveConfirm();                   // .text:0x1348
    void openEraseConfirm();                  // .text:0x1420
    void startSave();                         // .text:0x14F8
    void startErase();                        // .text:0x15BC
    u16 getErrorMessage(int result) const;    // .text:0x1688
    BOOL stepSave();                          // .text:0x1700
    BOOL stepErase();                         // .text:0x177C
    void openAnswers();                       // .text:0x17EC
    void onSaveAnswer();                      // .text:0x19A8
    void onEraseAnswer();                     // .text:0x1A10
    void onQuitAnswer();                      // .text:0x1A54
    void setAnswers();                        // .text:0x1A68

    virtual int state2() {
        return SUCCEEDED;
    }
    virtual int state3() {
        return SUCCEEDED;
    }
    virtual void postDraw(MAIN_STATE_e) {
    }
    virtual void deleteReady() {
    }
    virtual bool createHeap() {
        return true;
    }

    // Implicit destructor: .text:0x1AFC.

    /* 0x0D4 */ m2d::ResAccLoader_c mRes;
    /* 0x1A8 */ m2d::SimpleEx_c mLyt;
    /* 0x240 */ m2d::Anm_c mNoPhotoAnm;
    /* 0x264 */ m2d::Anm_c mScrlIconAnm;
    /* 0x288 */ m2d::AnmEx_c mBtnAnm;
    /* 0x2C4 */ dMenuSlctB_c mSlct;
    /* 0x9B4 */ int mStep;   // the SD card step (SAVE_STEP_e / ERASE_STEP_e)
    /* 0x9B8 */ int mTimer;  // frames left of the minimum message time
    /* 0x9BC */ u16 mErrMsg; // the SYS_SDcard message of the failure, 0: none
    /* 0x9C0 */ dSelect::MComp_c mComp;
    /* 0x136C */ int mMode; // the answer taken (ANSWER_e)
}; // size 0x1370

#endif
