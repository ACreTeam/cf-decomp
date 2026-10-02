#pragma once

#include <types.h>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_script.hpp>

#define MAIL_ADDRESS_RAW_LEN 95
#define MAIL_HEADER_LEN 32
#define MAIL_BODY_LEN 192
#define MAIL_FOOTER_LEN 32

// Item index of the invite card stationery (dItem::Item(int)).
#define MAIL_INVITE_CARD_IDX 0x155

// dMail_c::m38C layout.
#define MAIL_38C_STATUS_MASK 0x1F // getStatus / setStatus
#define MAIL_38C_INVITE_FLAG 0x20 // isFlaggedInvite / flagInvite (invite cards only)
#define MAIL_38C_HI_SHIFT 6       // get38C_hi / set38C_hi
#define MAIL_38C_HI_MASK 0xC0

// dMail_c status (m38C bits 0-4).
enum {
    MAIL_STATUS_NONE,    // empty slot
    MAIL_STATUS_UNREAD,  // delivered, not opened
    MAIL_STATUS_READ,    // opened (markRead)
    MAIL_STATUS_WRITTEN, // written by the player, not sent yet (markSent -> UNREAD)
};

// dMailAddress_c::mType.
enum {
    MAIL_ADDR_NONE,
    MAIL_ADDR_PLAYER,
    MAIL_ADDR_ANIMAL,
    MAIL_ADDR_ANIMAL_NONAME, // } hideName: the name isn't shown,
    MAIL_ADDR_PLAYER_NONAME, // } setWord(skipNoName) leaves the word empty
    MAIL_ADDR_OTHER_NONAME,  // }
    MAIL_ADDR_FUTURE_SELF,   // a letter to the player's future self
    MAIL_ADDR_TEXT,          // free text (setFromBMG sender)
};

// dMail_c::mSenderKind: picks the "from" line in mail lists (sFromMsgIds, sys_2D/SYS_2D_menu).
enum {
    MAIL_FROM_NAMED,        // "From <name>", name filled in
    MAIL_FROM_MOM,
    MAIL_FROM_HRA,          // Happy Room Academy
    MAIL_FROM_NOOK,
    MAIL_FROM_MUSEUM,
    MAIL_FROM_SNOWMAN,
    MAIL_FROM_NAMED2,       // "From <name>", no name filled in
    MAIL_FROM_ABD,          // ABD notice
    MAIL_FROM_THANK_YOU,    // thank-you card
    MAIL_FROM_DAD,
    MAIL_FROM_MOM_AND_DAD,
    MAIL_FROM_CHIP,
    MAIL_FROM_BUG_OFF,
    MAIL_FROM_AUCTION,
    MAIL_FROM_JACK,
    MAIL_FROM_TNPS,
    MAIL_FROM_POSY_FARM,
    MAIL_FROM_TORTIMER,
    MAIL_FROM_JINGLE,

    MAIL_FROM_NUM
};

// 0xC2. The first 0xBF bytes hold one of three forms, picked by mType (MAIL_ADDR_*).
struct dMailAddress_c {                                    // 0xC2
    void setWord(dScript::Word_c *word, BOOL skipNoName);  // 801189C4: name word for this address
    void getName(wchar_t *out, BOOL skipNoName);           // 80118AD8: name into a 9-wchar buffer
    void setPlayer(const dPersonalID_c *pid);              // 80118B70
    void setAnimal(const dAnmPersonalID_c *animal);        // 80118C54
    void setText(const wchar_t *text);                     // 80118D44: up to 95 chars
    void hideName();                                       // 80118DF8: switch to the *_NONAME type
    void setFutureSelf();                                  // 80118E3C
    dPersonalID_c *getPlayer();                            // 80118E48: player types, else NULL
    dAnmPersonalID_c *getAnimal();                         // 80118E7C: animal types, else NULL
    BOOL isType(u8 type);                                  // 80118EA0

    union {
        dPersonalID_c mPlayer;                 // PLAYER, PLAYER_NONAME, FUTURE_SELF
        dAnmPersonalID_c mAnimal;              // ANIMAL, ANIMAL_NONAME
        wchar_t mText[MAIL_ADDRESS_RAW_LEN+1]; // TEXT
    };
    u8 mType;                        // 0xC0
};

// A letter. Source: src/dol/game/d_mail.cpp (.text 80117400..8011927C).
// Notes on the names: notes/d_mail.txt.
class dMail_c {                    // 0x390
public:
    dMail_c(); // 80117400
    ~dMail_c(); // 80117410

    dMail_c *copy(const dMail_c *other); // 80117450
    u8 getStatus();                      // 80117484
    void setStatus(u8 status);           // 80117490
    u16 getPresent();                    // 801174A4
    u8 get38C_hi();                      // 801174AC
    u8 getSenderKind();                  // 801174B8
    void clearFlags();                   // 801174C0
    void clearFlags2();                  // 801174CC (same code as clearFlags)
    void setFlag(int bit);               // 801174D8
    void clearFlag(int bit);             // 801174F4
    u8 isFlag(int bit);                  // 80117510
    BOOL isReadFlaggedInvite();          // 80117528: read && isFlaggedInvite
    BOOL isFlaggedInvite();              // 80117570: invite card with MAIL_38C_INVITE_FLAG
    BOOL isReadUnflaggedInvite();        // 801175D4: read invite card without MAIL_38C_INVITE_FLAG
    void flagInvite();                   // 80117654: sets MAIL_38C_INVITE_FLAG on an invite card
    void set38C_hi(u8 value);            // 801176B8
    u16 setPresent(u16 item, u8 hi);     // 801176CC: returns the old present
    BOOL isWritten();                    // 80117710
    BOOL isToNone();                     // 80117740
    BOOL isEmpty();                      // 80117748
    BOOL isToFutureSelf();               // 80117774
    BOOL isFromText();                   // 8011777C
    BOOL isToOtherTown();                // 80117788: recipient's town differs from this save's
    BOOL hasToAnimal();                  // 801178CC
    void clear();                        // 801178F8
    BOOL isValid();                      // 80117938: mNamePos <= 32 && mSenderKind <= 19
    void markSent();                     // 80117984: WRITTEN -> UNREAD, 38C_hi = 1
    void markRead();                     // 801179D8: UNREAD -> READ
    dPersonalID_c *getFromPlayer();      // 80117A20
    dAnmPersonalID_c *getFromAnimal();   // 80117A28
    dPersonalID_c *getToPlayer();        // 80117A30
    dAnmPersonalID_c *getToAnimal();     // 80117A34
    void setup(const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper); // 80117A38
    static BOOL findNamePos(wchar_t *header, u8 *pos); // 80117AE8: removes the first newline, stores its index
    void prepareHeader();                               // 80117C5C
    void setTextFromLabel(const u16 *kind, const char *label); // 80117CA8: dLetter text (fn_800CB638)
    void setTextFromParts(const u16 *a, const u16 *b, const u16 *c, int d, int e, int f); // 80117D94: fn_800CB66C
    void setTextFromParts(u16 a, u16 b, u16 c, u16 d, u16 e, int f, int g);              // 80117E90: fn_800CB760
    void setPaper(const dItem::Item *paper);            // 80117F94
    void setupFromPlayer(const dItem::Item *paper);     // 80117FD4: from the current player, with their letter style
    BOOL setFromBMG(const void *bmg);                   // 8011804C: header/body/footer/sender/paper/present
    void setupFromAnimal(const u16 *kind, const char *label, const dAnmPersonalID_c *sender,
                         const dPersonalID_c *to, const dItem::Item *paper); // 801183B8
    void setupFromAnimal(const u16 *kind, const char *label, const dAnmPersonalID_c *sender,
                         const dPersonalID_c *to, const int *paperIdx); // 80118420
    void setupFromAnimal(const u16 *a, const u16 *b, const u16 *c, int d, int e, int f,
                         const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper); // 80118488
    void setupFromAnimal(const u16 *a, const u16 *b, const u16 *c, const u16 *d, const u16 *e, int f, int g,
                         const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper); // 80118508
    void setupSystem(const u16 *kind, const char *label, const u8 *senderKind, const dPersonalID_c *to,
                     const dItem::Item *paper); // 80118590
    void setupSystem(const u16 *kind, const char *label, const u8 *senderKind, const dPersonalID_c *to,
                     const int *paperIdx); // 8011862C
    void setToVillager(int idx);                        // 80118694
    void setToPlayer(int idx, BOOL keepHeader);         // 80118708
    void setToSelf();                                   // 80118784
    void setToFriend(int idx);                          // 801187DC
    void getToName(wchar_t *out);                       // 80118838
    void setFromWord(dScript::Word_c *word);            // 80118840
    void getListWord(dScript::Word_c *word);            // 8011884C: "To <name>" / "From <name>" line for mail lists

    dMailAddress_c mTo;                   // 0x000
    dMailAddress_c mFrom;                 // 0x0C2
    wchar_t mHeader[MAIL_HEADER_LEN + 1]; // 0x184  0x42 bytes
    wchar_t mBody[MAIL_BODY_LEN + 1];     // 0x1C6  0x182 bytes
    wchar_t mFooter[MAIL_FOOTER_LEN + 1]; // 0x348  0x42 bytes
    u8 mNamePos;                     // 0x38A  where the recipient name goes in the header (newline index, findNamePos)
    u8 mPaper;                       // 0x38B  stationery index (seeker_c::findLike on the paper item)
    u8 m38C;                         // 0x38C  MAIL_38C_*: status (bits 4-0), invite flag (bit 5), value (bits 7-6).
                                     //        Written with masks, not a bitfield (see notes/matching_tips.md).
    u8 mSenderKind;                  // 0x38D  MAIL_FROM_*. On the letter shared by all players
                                     //        (dSaveExtra_c) it's a bitmask of who got it (setFlag / isFlag).
    dItem::Item mPresent;            // 0x38E  0xFFF1 when there's no present
};

// A player's saved letter greeting and signature (dPrivateData_c+0x7DFA). 0xC8.
// apply / applyHeader copy it into a dMail_c, store copies it back.
class dLetterStyle_c {
public:
    dLetterStyle_c(); // 80118EB4
    ~dLetterStyle_c(); // 80118EE4
    void clear(); // 80118F24
    void init();                            // 80118F64: defaults from sys_2D/SYS_2D_menu
    void apply(dMail_c *mail);              // 80119098: footer + header
    void applyHeader(dMail_c *mail);        // 801190F8
    void store(dMail_c *mail);              // 80119164

    /* 0x00 */ wchar_t mHeader[MAIL_HEADER_LEN + 1];       // "Dear \n,"
    /* 0x42 */ wchar_t mFutureHeader[MAIL_HEADER_LEN + 1]; // "Dear future \n," (letters to the future self)
    /* 0x84 */ wchar_t mFooter[MAIL_FOOTER_LEN + 1];       // "From <player>"
    /* 0xC6 */ u8 mNamePos;       // 0xFF when cleared
    /* 0xC7 */ u8 mFutureNamePos; // 0xFF when cleared; >= 0x20 makes apply call init
}; // size 0xC8
