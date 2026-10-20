#pragma once

// The menu manager (RTTI dMenuMng_c). Names other than the class name are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

class dDesign_c;
class dMail_c;
class dMenuNd_c;
class dTime_c;

class dMenuMng_c : public fBase_c {
public:
    enum STATE_e {
        STATE_NONE,
        STATE_IDLE,
        STATE_OPEN,
        STATE_CREATING,
    };

    enum FLAG_e {
        FLAG_NO_BGM = 0x8,
        FLAG_CLOSE_REQUEST = 0x10, // a net message or the countdown wants the menu closed
        FLAG_PICKED_SHIRT = 0x20, // the design picked is the one worn as the shirt
        FLAG_PICKED_HAT = 0x40,   // the design picked is the one worn as the hat
    };

    enum { POCKET_NUM = 15 };

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();

    void createMenu();

    static void addMenu(dMenuNd_c *node);
    static void removeMenu(dMenuNd_c *node);
    static BOOL isFree();
    static BOOL open(u8 menu, BOOL noBgm);
    static BOOL openSub(u8 menu);
    static m2d::ResAccIf_c *getWinResAcc();
    static m2d::ResAccIf_c *getBtnResAcc();
    static m2d::ResAccLoader_c *getWiiMenuResAcc();
    static m2d::ResAccLoader_c *getWiiMenu2ResAcc();
    static BOOL isMenuEnd();
    static void onFlag(u32 flag);
    static void offFlag(u32 flag);
    static bool isFlag(u32 flag);
    static void setResult(u8 result);
    // The item argument of the item menus (s_C104); returns the previous one.
    static u16 setMenuItem(u16 item); // 8019AD38
    static void requestClose(); // 8019A7AC
    static void clearCloseRequest(); // 8019A7B4
    static bool isCloseRequested(); // 8019A7BC

    static BOOL openItemSel(u16 mask, u16 msgParam, u8 menu);
    static BOOL openItemSel2(u16 mask, u16 msgParam);
    static u16 makePocketMask(BOOL (*filter)(void *pocket, int flag));
    static u8 getSelSlot();
    static void setSelSlot(u8 slot);
    static u16 getSelMask();
    static void setSelMask(u16 mask);
    static void setPocketMask(u16 mask);
    static u16 getPocketMask();
    static u16 getItemMsgParam();
    static void clearSelItems();
    static void setSelItems();
    static u16 *getSelItems();
    static int getSelItemNum();
    static void removeSelItems();

    static wchar_t *getInputBuf();
    static void setInputBuf(const u16 *str, int len);
    static void clearInputBuf();
    static BOOL openInput(u8 menu, const u16 *str, int len);

    // openMenu / openItemSel menus (only the ones named so far).
    enum {
        MENU_DESIGN = 0x0D,        // the design menu from its tab (REL d_menu_desi00)
        MENU_DESIGN_SELECT = 0x0E, // the design menu for the design in getDesignIndex
        MENU_EDIT_DESIGN = 0x0F,   // the design editor (REL d_menu_editNP)
        MENU_EDIT_PRO = 0x10,      // ... for a pro design (four textures)
        MENU_EDIT_FACE = 0x11,     // ... for the face design (the cat model)
        MENU_ITEM_SELECT = 0x32,
        MENU_TURNIP_COUNT = 0x45, // how many turnips (REL d_menu_kabu)
        MENU_TURNIP_PRICE = 0x46, // the turnip price
        MENU_DONATION = 0x47,     // the public works donation
        MENU_AUCTION_MIN = 0x48,  // the auction's minimum price
        MENU_AUCTION_BID = 0x49,  // the auction bid (at least getNumberMin)
        MENU_BANK_DEPOSIT = 0x4A,
        MENU_BANK_WITHDRAW = 0x4B,
        MENU_BANK_PAY = 0x4C,
        MENU_MUSIC = 0x5D,
        MENU_STORAGE = 0x5E,
        MENU_DESIGN_TAKE = 0x65, // the slot for a design taken from the tailor (REL d_menu_desi01)
        MENU_TOWN_FLAG_DESIGN = 0x66, // the town flag's design (REL d_menu_desi03)
        MENU_DESIGN_GIFT = 0x67, // the slot for a design given by a villager (REL d_menu_desi01)
        MENU_DESIGN_DISPLAY = 0x68, // which design to display at the tailor (REL d_menu_desi01)
        MENU_DESIGN_SWAP = 0x69, // which design to swap with the tailor's (REL d_menu_desi01)
        MENU_DESIGN_PRO = 0x6A, // the slot for a pro design (REL d_menu_desi01)
        MENU_RECYCLE_BOX = 0x60, // the town hall's recycle box (daRmobjRB_c)
        MENU_MAILBOX = 0x61, // the mailbox's letters (daStrPost_c)
        MENU_FLAG_DESIGN = 0x6B, // the house flag's design, asked for on the phone (daRmobjTel_c)
        MENU_BBS = 0x6D,     // the bulletin board (daStrBbs_c)
        MENU_POINT_PRESENT = 0x75, // the presents of Nook's point machine (daRmobjPoint_c)
    };

    static BOOL openMenu(u8 menu);
    // Opens the auction bid prompt; minimum is its lowest bid (getNumberMin).
    static BOOL openAuctionBid(u32 minimum);
    // The number the amount prompts (REL d_menu_kabu) work with: the bid's
    // minimum going in, the confirmed amount coming out.
    static void setNumber(u32 number);
    static u32 getNumber();
    static u32 getNumberMin();
    static dMail_c *getMail();
    static void setMail(const dMail_c *mail);
    static BOOL openLetter(const dMail_c *mail);

    // The design slot the design menu (REL d_menu_desi00) hands to the menu it opens.
    static u8 getDesignIndex();           // 8019AD48
    static void setDesignIndex(u8 index); // 8019AD50
    static void setDesign(const dDesign_c *design);
    static dDesign_c *getDesign();
    // The design number (after the order) the design menus (REL d_menu_desi01..03) picked.
    static u32 getPickedDesign();                 // 8019AFE4
    static void setPickedDesign(u32 design);      // 8019AFEC
    // Whether the picked design is worn as the shirt / hat (the tailor refreshes the outfit).
    static bool isPickedShirt();                  // 8019AFF4
    static bool isPickedHat();                    // 8019AFFC
    static void setPickedShirt();                 // 8019B004
    static void setPickedHat();                   // 8019B00C
    static void clearPickedShirt();               // 8019B014
    static void clearPickedHat();                 // 8019B01C

    static void clearSelMask();
    static bool isSelMask(u16 bit);
    static void onSelMask(u16 bit);
    static int countSelSlots();
    static BOOL isSelSlot(int slot);
    static void onSelSlot(int slot);

    static void setTime(const dTime_c *time);
    static dTime_c getTime();

    static void setFriendIdx(int idx);
    static u8 *getFriendInfo();
    static void setPostType(int type);
    static wchar_t *getPostMsg();
    static wchar_t *getPostTitle();
    static void setupPost(BOOL withPhoto);
    static void clearPost();
    static BOOL sendPost();
    static bool isPhotoCaptured();
    static bool isFriendWithKey();

    static void startMenuBgm(u8 menu);
    static void endMenuBgm();

    static void resetToolPocket();
    static BOOL canPutAwayHeld();
    static void putAwayHeld();
    static BOOL isSwitchTool(int pocket);
    static u16 getNextTool();
    static u16 getPrevTool();
    static void swapHeld();

    /* 0x064 */ m2d::ResAccLoader_c mWinRes;
    /* 0x138 */ m2d::ResAccLoader_c mBtnRes;
    /* 0x20C */ m2d::ResAccLoader_c mWiiMenuRes;
    /* 0x2E0 */ m2d::ResAccLoader_c mWiiMenu2Res;
};

extern mVec3_c g_menuPos;

extern "C" {
void fn_8019A7A4();
BOOL fn_8019AC04(u16 arg);
BOOL fn_8019AC44(u16 arg);
BOOL fn_8019AC84(u16 arg);
BOOL fn_8019ACC4(u16 arg, const mVec3_c *pos);
u16 fn_8019AD30();
BOOL fn_8019ADD4();
BOOL fn_8019AE00();
BOOL fn_8019AE2C();
BOOL fn_8019AE58();
BOOL fn_8019AEE4();
void fn_8019AFBC(u32 arg);
u32 fn_8019AFC4();
u16 fn_8019B240();
void fn_8019B248(u16 arg);
u32 fn_8019B250();
void fn_8019B258(u32 arg);
u16 fn_8019B260();
void fn_8019B268(u16 arg);
u32 fn_8019B270();
u32 fn_8019B278();
void fn_8019B280(u32 a, u32 b);
u32 fn_8019B28C();
void fn_8019B294(u32 arg);
BOOL fn_8019B29C();
BOOL fn_8019B2C4();
BOOL fn_8019B2EC();
BOOL fn_8019B348();
BOOL fn_8019B350();
BOOL fn_8019B358();
BOOL fn_8019B3DC();
void fn_8019B3E4();
void fn_8019B3EC();
void fn_8019B3F4();
void fn_8019B418();
BOOL fn_8019B4EC();
BOOL fn_8019B4F8();
BOOL fn_8019B504();
bool fn_8019B518();
void fn_8019B52C();
void fn_8019B538();
extern u8 lbl_8074EA7D;
}
