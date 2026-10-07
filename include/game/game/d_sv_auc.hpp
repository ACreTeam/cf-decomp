#pragma once

// The city auction house (SCENE_RM_AUCTION). Source: src/dol/game/d_sv_auc.cpp
// (.text 8010A120..8010D7E8). The file and the dSvAuc prefix come from the RTTI string
// "dSvAuc_hostIO_c"; the other names are inferred.
//
// Nine items are up for auction (dSvAuc_c, in the town save): slots 0..2 for large furniture,
// 3..8 for the rest. Items are listed by players of this town or arrive from other towns
// (WiiConnect24); bids, sales and new listings go out again through three lists in dSaveExtra_c
// (dSvAucList_c). An auction day starts at 6:00; the day type (getDayType) decides whether bids
// are taken or the auction closes, and the results go out as letters (MAIL_ETC_Auction).

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_private_data.hpp>
#include <lib/egg/core/eggBitFlag.h>

#include <game/game/d_date.hpp>
struct dLandID_c;

// One item up for auction (0x78). Its constructor (8010EB50, weak, emitted in d_save_data) sets
// mItem to none and calls clear().
class dSvAucItem_c {
public:
    void clear();                                            // 8010A758
    void deletePlayer(u32 player);                           // 8010A7E8: drops the player's bid
    void receive(const dSvAucItem_c *other);                 // 8010A978: a listing from another town, without its bids
    void receiveBid(const dSvAucItem_c *other);              // 8010AC50: a higher bid from another town
    BOOL isValid() const;                                          // 8010ADAC: a seller and a known item
    BOOL hasNoBids() const;                                        // 8010AE64
    BOOL isFromTown(const dLandID_c *land) const;                  // 8010AEBC: NULL = this town
    BOOL isFromOtherTown(const dLandID_c *land) const;             // 8010AF6C
    void updateDay(const dLandID_c *land, u32 day);          // 8010B01C: clears it once it is over
    BOOL isSellerOffline(const dLandID_c *land) const;             // 8010B0A0
    BOOL isHighBidderFrom(const dLandID_c *land) const;            // 8010B100
    void sendResults(const dLandID_c *land, u32 day, BOOL online); // 8010B1AC
    void close(const dLandID_c *land, int dayType);          // 8010B5CC: sold to the high bidder
    int getMyBid() const;                                          // 8010B65C: the current player's bid
    void placeBid(int price, dPrivateData_c *player);        // 8010B6D8: NULL = the current player
    void bid(int price);                                     // 8010B8B4
    BOOL isMyUnsold() const;                                       // 8010B8BC
    BOOL isMyWon() const;                                          // 8010B9C0
    u32 getDay() const { return mDay; }
    BOOL isOpenToday() const;                                      // 8010BAB4
    BOOL isNotToday() const;                                       // 8010BB58
    void take();                                             // 8010BBD8: the current player received it
    BOOL hasMyBid() const;                                         // 8010BC64
    BOOL isOver(u32 day, const dLandID_c *land) const;             // 8010BCF0
    BOOL sendResult(u32 player, u16 kind, int money, const dItem::Item *present); // 8010BDF8
    const dPersonalID_c *getSeller() const;                              // 8010C038: NULL when empty4

    /* 0x00 */ dItem::Item mItem;
    /* 0x02 */ s8 mSold;
    /* 0x03 */ s8 mSentResults;                              // a bit per player
    /* 0x04 */ dPersonalID_c mSeller;
    /* 0x30 */ int mMinPrice;
    /* 0x34 */ int mBids[PLAYER_NUM];
    /* 0x44 */ int mHighBid;
    /* 0x48 */ dPersonalID_c mHighBidder;
    /* 0x74 */ u16 mDay;                                     // getToday() when listed
}; // size 0x78

// Records to send to other towns, one slot per player (dSaveExtra_c). Its methods are in the unsplit
// code at 8014F2C0.
struct dSvAucList_c {
    void clear(u32 player);                                  // 8014F2C0: frees the slot
    int addMine(const dSvAucItem_c *item);                   // 8014F344: into the current player's slot
    int addSeller(const dSvAucItem_c *item);                 // 8014F57C: into the seller's slot
    int getDay(u32 player);                                  // 8014F79C: the slot's mDay, -1 when free

    /* 0x00 */ u8 _00;
    /* 0x01 */ u8 mMask;                                     // the slots in use (a bit per player)
    /* 0x02 */ u8 _02[2];
    /* 0x04 */ dSvAucItem_c mItems[PLAYER_NUM];
}; // size 0x1E4

// The auction (dSaveTown_c::mAuction).
class dSvAuc_c {
public:
    static BOOL create();                                    // 8010A120
    static BOOL resetChanged();                              // 8010A128
    static u32 getDay(dTime_c time);                         // 8010A154: weeks since 2000, days from 6:00
    static u32 getToday();                                   // 8010A260
    static int getDayType(dTime_c time);                     // 8010A300
    static int getTodayType();                               // 8010A418
    static void getNextOpenDate(int *year, int *month, int *day);  // 8010A4B8
    static void getNextCloseDate(int *year, int *month, int *day); // 8010A608

    void clear();                                            // 8010C0A4
    u64 *getSender(const dSvAucItem_c *item);           // 8010C104
    static u64 *findSender(const dSvAucItem_c *item);   // 8010C1F8
    void setSender(u32 idx, const u64 *sender);            // 8010C22C
    dSvAucItem_c *getItem(u32 idx);                          // 8010C250
    const dSvAucItem_c *getItemConst(u32 idx) const;                     // 8010C264
    int countEmptyLarge() const;                                   // 8010C278: slots 0..2
    int countEmptySmall() const;                                   // 8010C300: slots 3..8
    int countEmpty() const;                                        // 8010C388
    int findSlot(const dItem::Item *item) const;                   // 8010C3CC: -1 when there is no room
    BOOL list(u32 idx, const dItem::Item *item, int price, const dPersonalID_c *seller); // 8010C500
    static void cleanLists(int day, BOOL unused);            // 8010C688
    void update(int dayType, u32 day, const dLandID_c *land); // 8010C808
    static void updateOnline();                              // 8010C940
    void receiveListings(dSvAucList_c *list, const u64 *sender, BOOL online); // 8010C9C0
    BOOL receiveBids(dSvAucList_c *list, BOOL online);       // 8010CAF4
    void receiveSales(const dSvAucList_c *list, BOOL online);      // 8010CC1C
    void deletePlayer(int player);                           // 8010CDCC
    BOOL addListing(const dSvAucItem_c *item, const u64 *sender); // 8010CE2C
    int find(const dSvAucItem_c *item) const;                      // 8010CFC8
    BOOL has(const dSvAucItem_c *item) const;                      // 8010D0D8
    BOOL hasOpenToday() const;                                     // 8010D108
    BOOL hasMyUnsold() const;                                      // 8010D16C
    BOOL hasMyWon() const;                                         // 8010D1D0
    BOOL hasListedToday(const dPersonalID_c *pid) const;           // 8010D234
    BOOL hasMyBidToday() const;                                    // 8010D36C
    static void sendListing(const dSvAucItem_c *item);       // 8010D3F8
    static void sendBid(const dSvAucItem_c *item);           // 8010D43C
    static void sendSale(const dSvAucItem_c *item);          // 8010D478
    void pickDLItem();                                       // 8010D4B4
    static dSvAuc_c *get();                                  // 8010D6BC
    static EGG::TBitFlag<u32> *getChanged();                 // 8010D6E4: bits 0..2 = listings / bids / sales received
    static BOOL isBusy();                                    // 8010D708

    // *probably* exists
    static inline BOOL isToday(u16 day) {
        return day == getToday() && getTodayType() == 2;
    }

    static int sToday;                                       // 8074AFF8: -1, "today" for update()

    /* 0x000 */ dSvAucItem_c mItems[9];
    /* 0x438 */ u64 mSenders[9];                            // the sender of a listing from another town (WiiConnect24 id)
    /* 0x480 */ int mDay;
    /* 0x484 */ u8 mDayType;
}; // size 0x488

// Debug tuning (RTTI "dSvAuc_hostIO_c", 805EC1B8). MWCC puts the vtable pointer where the first
// virtual is declared, here after the fields.
class dSvAuc_hostIO_c {
public:
    dSvAuc_hostIO_c() {
        _00 = 0;
        _04 = 0;
        _06.clear();
        _38 = 0;
        _3C = 1000;
        _34 = 0;
        _40 = 5;
        _42 = -1;
        _44 = 5;
        _48 = dSvAuc_c::sToday;
    }

    /* 0x00 */ int _00;
    /* 0x04 */ u8 _04;
    /* 0x06 */ dPersonalID_c _06;
    /* 0x34 */ int _34;
    /* 0x38 */ int _38;
    /* 0x3C */ int _3C;
    /* 0x40 */ s16 _40;
    /* 0x42 */ s16 _42;
    /* 0x44 */ s16 _44;
    /* 0x48 */ int _48;

    virtual ~dSvAuc_hostIO_c() {}                            // 8010D710
    /* 0x4C vtable */
}; // size 0x50
