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
    // What setResult reports to the menu's opener (e.g. Pelly's bridge map).
    enum RESULT_e { RESULT_CANCEL, RESULT_OK };

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
    // The friend roster couldn't be fetched (the friend picker failed).
    static bool isNetError();
    static void onNetError();
    static void offNetError();
    static void onFlag(u32 flag);
    static void offFlag(u32 flag);
    static bool isFlag(u32 flag);
    static void setResult(u8 result);
    // The item argument of the item menus (s_C104); returns the previous one.
    static u16 setMenuItem(u16 item); // 8019AD38
    // The item a menu picked (shares s_C104 with the item argument): the song
    // the K.K. request menu (REL d_menu_hanNP) found.
    static u16 getPickedItem();       // 8019B240
    static void setPickedItem(u16 item); // 8019B248
    static void onNoBgm(); // 8019A7A4: the menu opened by dAcUitag keeps the BGM off
    // The pockets-full swap menus (REL d_menu_item03NP) for a picked-up, dug-up, caught insect or fish item.
    static BOOL openSwapPickUp(u16 item);                      // 8019AC04
    static BOOL openSwapDug(u16 item);                         // 8019AC44
    static BOOL openSwapInsect(u16 item);                      // 8019AC84
    static BOOL openSwapFish(u16 item, const mVec3_c *pos);    // 8019ACC4
    static u16 getMenuItem();                                  // 8019AD30
    // The text input menus (REL d_menu_hanNP), with a cleared input buffer.
    static BOOL openPlayerNameInput(); // 8019ADD4
    static BOOL openTownNameInput();   // 8019AE00
    static BOOL openSayInput();        // 8019AE2C: Resetti's "Say it!"
    static BOOL openMessageInput();    // 8019AE58
    static BOOL openPreviewMap(); // 8019AEE4: the town map Rover shows the new player on the bus (REL d_menu_map01NP)
    // What the player took in the lost and found menu (REL d_menu_item05NP): how many, and the item if one.
    static u32 getLostTakenNum();             // 8019B250
    static void setLostTakenNum(u32 num);     // 8019B258
    static u16 getLostTakenItem();            // 8019B260
    static void setLostTakenItem(u16 item);   // 8019B268
    // The block picked on the bridge map (REL d_menu_map02NP): Pelly takes it as the vote for a new bridge.
    static u32 getPickedBlockX();             // 8019B270
    static u32 getPickedBlockZ();             // 8019B278
    static void setPickedBlock(u32 x, u32 z); // 8019B280
    // The post office hand-in menus (REL d_menu_mslctNP), with a cleared selection mask.
    static BOOL openPostTown();      // 8019B29C
    static BOOL openPostOtherTown(); // 8019B2C4
    static BOOL openPostFuture();    // 8019B2EC
    static BOOL openFriendSelect();  // 8019B4EC
    static BOOL openWriteSelf();     // 8019B4F8
    static BOOL openPhoto01();       // 8019B504
    // The constellation slot the constellation menus work on (shares s_picked
    // with the picked design).
    static u32 getPickedStarSign();       // 8019B28C
    static void setPickedStarSign(u32 slot); // 8019B294
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
        MENU_MAP00 = 0x06,   // the town map from its tab (REL d_menu_map00NP)
        MENU_KEYTEST = 0x0C, // the keyboard test menu (REL d_menu_keytestNP)
        MENU_DESIGN = 0x0D,        // the design menu from its tab (REL d_menu_desi00)
        MENU_DESIGN_SELECT = 0x0E, // the design menu for the design in getDesignIndex
        MENU_EDIT_DESIGN = 0x0F,   // the design editor (REL d_menu_editNP)
        MENU_EDIT_PRO = 0x10,      // ... for a pro design (four textures)
        MENU_EDIT_FACE = 0x11,     // ... for the face design (the cat model)
        // The text entry menus (REL d_menu_hanNP, dMenuHan_c).
        MENU_INPUT_PLAYER_NAME = 0x12, // "What's your name?"
        MENU_INPUT_TOWN_NAME = 0x13,   // "What's your town's name?"
        MENU_INPUT_NEW_FRIEND_NAME = 0x14, // friend menu: name of the friend added by code; next the town
        MENU_INPUT_NEW_FRIEND_TOWN = 0x15, // ... his town; next the comment
        MENU_INPUT_FRIEND_NAME = 0x16,     // friend roster: rewrite the name of a friend; next the town
        MENU_INPUT_FRIEND_TOWN = 0x17,     // ... his town, then the entry is registered
        MENU_INPUT_MY_FRIEND_NAME = 0x18,  // the player's own friend name, typed in the frame's text box
        MENU_INPUT_NEW_FRIEND_COMMENT = 0x19, // the comment of the new friend, may be empty; registers the entry
        MENU_INPUT_FRIEND_COMMENT = 0x1A,     // friend roster: rewrite the comment of a friend
        MENU_INPUT_SAY = 0x1B,           // "Say it!" (Resetti)
        MENU_INPUT_DESIGN_NAME = 0x1C,   // the name of a design, back to the design editor
        MENU_INPUT_PRO_DESIGN_NAME = 0x1D, // ... of a pro design
        MENU_INPUT_MESSAGE = 0x1E,       // "What's the exact message?"
        MENU_INPUT_STAR_SIGN_NAME = 0x1F, // the name of a constellation
        MENU_INPUT_STAR_SIGN_NAME_2 = 0x20, // ... same menu
        MENU_INPUT_SONG = 0x21,          // the K.K. song request: the song named becomes getPickedItem
        MENU_INPUT_CATCHPHRASE = 0x22,   // opened by openInput with the current text
        MENU_INPUT_NICKNAME = 0x23,
        MENU_INPUT_GREETING = 0x24,
        MENU_SWAP_PICK_UP = 0x25, // pockets full: swap for a picked-up item (REL d_menu_item03NP)
        MENU_SWAP_DUG = 0x26,     // ... a dug-up item
        MENU_SWAP_INSECT = 0x27,  // ... a caught insect
        MENU_SWAP_FISH = 0x28,    // ... a caught fish
        MENU_ITEM_SELECT = 0x32,
        MENU_PREVIEW_MAP = 0x35, // the town map Rover shows the new player on the bus (REL d_menu_map01NP)
        MENU_BRIDGE_MAP = 0x36, // the town map to pick the new bridge's block, opened by Pelly (REL d_menu_map02NP)
        MENU_HOUSE_MAP = 0x37, // the players' houses on the town map, opened by Pelly (REL d_menu_map03NP)
        MENU_PHOTO01 = 0x39, // REL d_menu_photo01NP
        MENU_MUSEUM = 0x3C, // the museum's donation list (REL d_menu_museumNP)
        MENU_FRIEND_TOWN = 0x55, // "Go to which town?": the online friends' towns (REL d_menu_friend03NP)
        MENU_FRIEND_TOWN_B = 0x56, // ... same menu
        MENU_FRIEND04 = 0x57, // friend roster (REL d_menu_friend04), the first tab of dMenuFriend_c
        MENU_FRIEND05 = 0x58, // visiting friends
        MENU_FRIEND06 = 0x59, // friend code
        MENU_FRIEND07 = 0x5A, // mic chat
        MENU_FRIEND_CODE_ADD = 0x5B, // friend code entry for a new friend, asks his name next (REL d_menu_friend08)
        MENU_FRIEND_CODE_EDIT = 0x5C, // ... to change the key of the friend being edited
        MENU_WRITE_LETTER = 0x41,   // write a letter on the stationery of the pocket getPickedLetter (REL d_menu_writeNP)
        MENU_REWRITE_LETTER = 0x42, // ... edit the letter in slot getPickedLetter of the player
        MENU_WRITE_BOARD = 0x43,    // a message for the bulletin board of the friend (setPostType 1)
        MENU_WRITE_SELF = 0x44,     // a message sent to the player's own Wii (setPostType 0)
        MENU_TURNIP_COUNT = 0x45, // how many turnips (REL d_menu_kabu)
        MENU_TURNIP_PRICE = 0x46, // the turnip price
        MENU_DONATION = 0x47,     // the public works donation
        MENU_AUCTION_MIN = 0x48,  // the auction's minimum price
        MENU_AUCTION_BID = 0x49,  // the auction bid (at least getNumberMin)
        MENU_BANK_DEPOSIT = 0x4A,
        MENU_BANK_WITHDRAW = 0x4B,
        MENU_BANK_PAY = 0x4C,
        MENU_POST_TOWN = 0x4D,       // the letters to hand in at the post office, for the town (REL d_menu_mslct)
        MENU_POST_OTHER_TOWN = 0x4E, // ... letters to other towns
        MENU_POST_FUTURE = 0x4F,     // ... the letter to the future self
        MENU_ADDRESS = 0x53,        // the addressee of the menu manager's mail (REL d_menu_friend02)
        MENU_ADDRESS_LETTER = 0x54, // ... of the letter getPickedLetter of the player
        MENU_FRIEND_SELECT = 0x51,       // the Wii friend picker (REL d_menu_friend01NP); updates the roster first
        MENU_FRIEND_SELECT_KNOWN = 0x52, // ... for a roster that is already known
        MENU_MUSIC = 0x5D, // the K.K. music list (REL d_menu_musicNP)
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
        MENU_HANIWA = 0x76, // the gyroids to take out of the storage (REL d_menu_haniwaNP), opened by the cafe
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
    // The letter slot of the player the menus work on (shares s_picked with the picked design).
    static void setPickedLetter(u32 slot); // 8019AFBC
    static u32 getPickedLetter();          // 8019AFC4

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

    // The bits of the selection mask the post office menu (REL d_menu_mslct) reports to the clerk
    // (d_a_npc_sp_periko); the bits 1 << (slot + 1) are the players whose mailbox is full.
    enum SEL_MASK_e {
        SEL_NOT_RESIDENT = 0x01, // a letter to a villager or player that doesn't live here was returned
        SEL_POST_FULL = 0x20,    // the post office is full
        SEL_MAIL_TAKEN = 0x40,   // at least one letter was taken
        SEL_NOT_FRIEND = 0x80,   // a letter to another town was returned, the recipient isn't a friend
    };
    static BOOL isSelNotResident();
    static BOOL isSelNotFriend();
    static BOOL isSelPostFull();
    static BOOL isSelMailTaken();
    static void onSelNotResident();
    static void onSelNotFriend();
    static void onSelPostFull();
    static void onSelMailTaken();

    static void setTime(const dTime_c *time);
    static dTime_c getTime();

    static void setFriendIdx(int idx);
    static wchar_t *getFriendInfo();
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
extern u8 lbl_8074EA7D;
}
