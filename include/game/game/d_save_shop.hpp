#pragma once

#include <types.h>
#include <game/game/d_save_money.hpp>
#include <game/game/d_time_stamp.hpp>

class dPrivateData_c;

// A shop's stock slot (Nook's and the tailor's).
struct dSaveShopStock_c { // 0x4
    /* 0x0 */ u16 mItem;
    /* 0x2 */ u8 mSold;
    /* 0x3 */ u8 _3;
};

enum {
    SHOP_STAGE_CRANNY,      // Nook's Cranny
    SHOP_STAGE_NOOK_N_GO,   // Nook 'n' Go
    SHOP_STAGE_NOOKWAY,     // Nookway
    SHOP_STAGE_NOOKINGTONS, // Nookington's
};

#define SHOP_STOCK_NUM 36

// Nook's store (NH SaveShop): dSaveShops_c::mShop. Stock slots: 0..5 tools, 6 medicine, 7..13 seed bags,
// 14..16 saplings, 17..24 furniture, 25..27 wallpaper, 28..30 carpet, 31..34 stationery, 35 paint; how many
// of each depends on the stage. The store is upgraded once its sales reach getSalesGoal() and mDaysLeft
// has run out; at Nookington's the players' votes (dPrivateData_c::_868F) decide what comes next.
// Source: src/dol/game/d_save_shop.cpp.
class dSaveShop_c { // 0xC4
public:
    void clear();                                         // 80143FE8
    void setStage(int stage);                             // 8014406C: also posts the opening notice
    BOOL isUpgradingToday();                              // 80144288
    BOOL isUpgradingOnLastDay();                          // 801442F0
    BOOL isOpen();                                        // 8014435C: by the stage's hours
    int getSalesGoal(int stage);                          // 801444B4
    void processDays(int days);                           // 80144504: daily, days since the last time
    void updateStage(int days);                           // 80144548
    void updateMail(int days);                            // 80144D58: the point rank gifts and event mail
    void decideNextStage();                               // 8014538C: by the players' votes
    void update();                                        // 80145590: daily, restocks
    int getToolNum();                                     // 80145FB4
    int getFtrNum();                                      // 80145FC4
    int getSeedNum();                                     // 80145FD4
    int getSaplingNum();                                  // 80145FE4
    int getWallNum();                                     // 80145FF4
    int getCarpetNum();                                   // 80146004
    int getPaperNum();                                    // 80146014
    int getPaintNum();                                    // 80146024
    u16 getItem(int slot);                                // 80146034: ITEM_IDX_SOLD_OUT_SIGN_00 once sold
    void sellAndNotify(int scene, int slot, int unitX, int unitZ, int price, int flag); // 801460B0
    void addSales(int price, int flag);                   // 80146170
    void sell(int scene, int slot, int unitX, int unitZ); // 801462F4: marks the slot sold, clears its unit
    BOOL isSold(int slot);                                // 80146400: TRUE for slots out of range
    void stockTools();                                    // 80146434
    void stockSeeds();                                    // 801466A0
    void stockSaplings();                                 // 80146790
    BOOL isEventToday();                                  // 80146864
    BOOL isSaleToday();                                   // 80146890
    BOOL isSaleNow();                                     // 801468F0
    BOOL hasFrom6();                                      // 80146A1C: an unsold item of getFrom() 6
    int getRank(dPrivateData_c *player);                  // 80146AA4: Nook point rank, 0 = none
    int getRank(u16 points);                              // 80146AC0
    u16 getPointsToNextRank(u16 points);                  // 80146B04
    void postNotices();                                   // 80146B90
    static void initRates();                              // 80146D68

    /* 0x00 */ int mStage;     // SHOP_STAGE_*
    /* 0x04 */ int mNextStage;
    /* 0x08 */ dSaveShopStock_c mStock[SHOP_STOCK_NUM];
    /* 0x98 */ dSaveMoney_c mSales;
    /* 0x9C */ dTimeStamp_c mStageDate;    // when the upgrade is done
    /* 0xA4 */ dTimeStamp_c mSaleDate;     // today's sale, from its hour
    /* 0xAC */ dTimeStamp_c mOctoberDate;  // the last October mail
    /* 0xB4 */ dTimeStamp_c mDecemberDate; // the last December mail
    /* 0xBC */ u8 mUpgrading : 1;
    /* 0xBC */ u8 mBC_6 : 1;
    /* 0xBC */ u8 mBC_0 : 6;
    /* 0xBD */ u8 mVote;       // 1..3, 4 = none
    /* 0xBE */ s8 mDaysLeft;   // before the next upgrade
    /* 0xBF */ s8 mCountdown;  // Nookington's vote countdown, 0x7F = none
    /* 0xC0 */ s8 mPaintIdx;   // the next paint color
    /* 0xC1 */ u8 _C1[3];
};
