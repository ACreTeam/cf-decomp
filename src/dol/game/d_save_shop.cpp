// Nook's store (dSaveShop_c). .text 80143FE8..80146D80, .rodata 80476458..804764A8,
// .data 804EF318..804EF360, .bss 805F2720..805F2AC0, .sdata 8074B0F8..8074B130, .sbss 8074E7C8..8074E7D0,
// .sdata2 80750C40..80750C68.
#include <game/game/d_save_shop.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_shop_layout.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_npc_notice.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>
#include <game/game/d_post_office.hpp>

extern u8 lbl_8059FF80[]; // the random item filter (fn_800C60B4)

extern "C" {
int fn_800C60B4(dItem::Item *out, int num, const int *range, int rangeNum, const void *filter, const dItem::Item *exclude,
                int excludeNum, int); // 800C60B4: random items
BOOL fn_800DCEDC();                   // 800DCEDC: an online session is active
u32 fn_800DCF30();                    // 800DCF30: players in the session
void fn_800DD4C8();                   // 800DD4C8
void fn_800DD518(const void *data, u32 size); // 800DD518
void fn_800DD588(int type, int arg);  // 800DD588
void *fn_800A9058();                  // 800A9058
int fn_800A8F98(void *, int unitX, int unitZ, int); // 800A8F98: -1 when none
void fn_800ABAF4(int);                // 800ABAF4
u8 *fn_800AC320();                    // 800AC320
void fn_800CBD30(int slot, u16 msgId, const char *group); // 800CBD30: message word
void fn_800CBAF0(int slot, int month);                    // 800CBAF0: month name word
void fn_800CBB50(int slot, u8 day);                       // 800CBB50: day word
void fn_800CBDA0(int slot, const dItem::Item *item);      // 800CBDA0: item name word
void fn_800CBFE0(u8 hour, int slot, int);                 // 800CBFE0: hour word
void *fn_801683D8();                                      // 801683D8
void fn_80167BAC(void *);                                 // 80167BAC: reloads the structures' unit attributes
}

static const int sTools[8] = {
    dItem::ITEM_IDX_FISHING_ROD, dItem::ITEM_IDX_NET,       dItem::ITEM_IDX_WATERING_CAN, dItem::ITEM_IDX_SHOVEL,
    dItem::ITEM_IDX_AXE_00,      dItem::ITEM_IDX_SLINGSHOT, dItem::ITEM_IDX_TIMER,        0,
};

static const int sSeeds[12] = {
    dItem::ITEM_IDX_RED_TULIP_BAG,   dItem::ITEM_IDX_WHITE_TULIP_BAG,  dItem::ITEM_IDX_YELLOW_TULIP_BAG,
    dItem::ITEM_IDX_WHITE_PANSY_BAG, dItem::ITEM_IDX_YELLOW_PANSY_BAG, dItem::ITEM_IDX_RED_PANSY_BAG,
    dItem::ITEM_IDX_WHITE_COSMOS_BAG, dItem::ITEM_IDX_RED_COSMOS_BAG,  dItem::ITEM_IDX_SUN_COSMOS_BAG,
    dItem::ITEM_IDX_RED_ROSE_BAG,    dItem::ITEM_IDX_WHITE_ROSE_BAG,   dItem::ITEM_IDX_YELLOW_ROSE_BAG,
};

// How many of each the store sells, by stage.
static const u8 sToolNum[4] = {3, 3, 4, 6};
static const u8 sFtrNum[4] = {2, 3, 5, 8};
static const u8 sSeedNum[4] = {1, 2, 4, 7};
static const u8 sSaplingNum[4] = {0, 1, 2, 3};
static const u8 sWallNum[4] = {1, 1, 2, 3};
static const u8 sCarpetNum[4] = {1, 1, 2, 3};
static const u8 sPaperNum[4] = {1, 2, 3, 4};
static const u8 sPaintNum[4] = {0, 1, 1, 1};
static const int sSaplings[2] = {dItem::ITEM_IDX_SAPLING, dItem::ITEM_IDX_CEDAR_SAPLING};

// Chances: [0] a silver tool (of 100), [1] a sale today (of 14). Set by initRates.
static u8 sRates[2];

// 80143FE8
void dSaveShop_c::clear() {
    memset(this, 0, sizeof(dSaveShop_c));
    mStageDate.reset();
    mSaleDate.reset();
    mOctoberDate.reset();
    mDecemberDate.reset();
    mStage = SHOP_STAGE_CRANNY;
    mNextStage = SHOP_STAGE_CRANNY;
    mUpgrading = FALSE;
    mBC_6 = FALSE;
    mVote = 0;
    mCountdown = 0x7F;
    mDaysLeft = 7;
    mPaintIdx = 0;
}

// 8014406C
void dSaveShop_c::setStage(int stage) {
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();
    mNextStage = stage;
    mStage = stage;
    if (mStageDate.isNone()) {
        mStageDate.setNow();
        mStageDate.toDayStart();
        mStageDate.addDays(-1);
    }

    switch (mStage) {
    case SHOP_STAGE_NOOK_N_GO:
        mDaysLeft = 14;
        break;
    case SHOP_STAGE_NOOKWAY:
        mDaysLeft = 21;
        break;
    case SHOP_STAGE_NOOKINGTONS:
        mCountdown = 37;
        break;
    }

    mUpgrading = FALSE;
    mSales.set(0);
    dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 9);
    if (mStageDate.diffDays(today.getTicks(), TRUE, FALSE) == 1) {
        dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 10);
    } else {
        int days = mStageDate.diffDays(today.getTicks(), TRUE, FALSE) - 1;
        if (days > 0) {
            int left = mDaysLeft - days;
            if (left < 0) {
                left = 0;
            }
            mDaysLeft = left;
        }
        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
    }

    fn_80167BAC(fn_801683D8());
    u16 msg = mStage + 10;
    dTime_c date = mStageDate.get();
    date.add(1, 0, 0, 0);
    fn_800EBB24(msg, "BBS_tanukichi", &date, NULL);
}

// 80144288
BOOL dSaveShop_c::isUpgradingToday() {
    BOOL today = FALSE;
    if (mUpgrading && mStageDate.diffDays(dTime_c::getCurrent(), TRUE, FALSE) == 0) {
        today = TRUE;
    }
    return today;
}

// 801442F0
BOOL dSaveShop_c::isUpgradingOnLastDay() {
    BOOL today = FALSE;
    if (mUpgrading && mStageDate.diffDays(&fgMngProc_getLastDayTime(), TRUE, FALSE) == 0) {
        today = TRUE;
    }
    return today;
}

// 8014435C
BOOL dSaveShop_c::isOpen() {
    const dTime_c &last = fgMngProc_getLastDayTime();
    dTimeStamp_c lastDay(&last);
    if (lastDay.diffDays(dTime_c::getCurrent(), TRUE, FALSE) != 0) {
        return FALSE;
    }

    dTime_c now = *dTime_c::getCurrent();
    switch (mStage) {
    case SHOP_STAGE_CRANNY:
        return now.hour >= 8 && now.hour < 22;
    case SHOP_STAGE_NOOK_N_GO:
        return !(now.hour >= 1 && now.hour < 7);
    case SHOP_STAGE_NOOKWAY:
        return now.hour >= 8 && now.hour < 22;
    case SHOP_STAGE_NOOKINGTONS:
        return now.hour >= 9 && now.hour < 21;
    }
    return FALSE;
}

// 801444B4
int dSaveShop_c::getSalesGoal(int stage) {
    switch (stage) {
    case SHOP_STAGE_CRANNY:
        return 30000;
    case SHOP_STAGE_NOOK_N_GO:
        return 80000;
    case SHOP_STAGE_NOOKWAY:
        return 150000;
    }
    return 150000;
}

// 80144504
void dSaveShop_c::processDays(int days) {
    updateStage(days);
    updateMail(days);
}

// 80144548
void dSaveShop_c::updateStage(int days) {
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();

    if (!mStageDate.isNone()) {
        dTimeStamp_c next = mStageDate;
        next.addDays(1);
        if (next.get().year >= 2036) {
            dTimeStamp_c later = today;
            later.addDays(2);
            if (later.get().year < 2036) {
                mStageDate.setNow();
                mStageDate.toDayStart();
                mStageDate.addDays(1);
            }
        }
    }

    fn_800CBD30(0, mStage + 0x2E, "sys_STRING/STR_Unit");
    if ((u8)mCountdown != 0x7F) {
        if (days > 0) {
            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 9);
            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x6D);
            int left = mCountdown - days;
            if (mCountdown >= 8) {
                if (left > 0) {
                    mCountdown = left;
                    if (mCountdown <= 7) {
                        mVote = 0;
                    }
                } else {
                    mCountdown = 37;
                }
            } else {
                if (mCountdown >= 1 && left <= 0) {
                    decideNextStage();
                    if (mNextStage != mStage) {
                        mUpgrading = TRUE;
                        mBC_6 = TRUE;
                        mStageDate.setNow();
                        mStageDate.toDayStart();
                        mStageDate.addDays(left + 1);
                        dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 9);
                        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
                        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x6D);
                        fn_800CBAF0(2, mStageDate.get().month);
                        fn_800CBB50(3, mStageDate.get().mday);
                        dTime_c date = mStageDate.get();
                        date.add(-1, 0, 0, 0);
                        fn_800EBB24(10, "BBS_tanukichi", &date, NULL);
                    } else {
                        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 9);
                        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
                        dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 0x6D);
                    }
                }

                if (left >= 0) {
                    mCountdown = left;
                } else if (mUpgrading) {
                    if (left == -1) {
                        mCountdown = left;
                    } else {
                        mUpgrading = FALSE;
                        mStage = mNextStage;
                        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 9);
                        if (mStageDate.diffDays(today.getTicks(), TRUE, FALSE) == 1) {
                            dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 10);
                        } else {
                            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
                        }
                        fn_80167BAC(fn_801683D8());
                        u16 msg = mStage + 10;
                        dTime_c date = mStageDate.get();
                        date.add(1, 0, 0, 0);
                        fn_800EBB24(msg, "BBS_tanukichi", &date, NULL);
                        mCountdown = 37;
                    }
                } else {
                    mCountdown = 37;
                }
            }
        } else if (days < 0) {
            if (mCountdown == 0) {
                mStageDate.setNow();
                mStageDate.toDayStart();
                mStageDate.addDays(1);
                if (mUpgrading) {
                    dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 9);
                }
            } else if (mCountdown == -1) {
                mStageDate.setNow();
                mStageDate.toDayStart();
            }
        }

        if (mCountdown >= 8) {
            for (int i = 0; i < PLAYER_NUM; i++) {
                dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
                if (player != NULL && player->mPID.isValid()) {
                    player->_868F = 0;
                }
            }
        }
        return;
    }

    if (days > 0) {
        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 9);
        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x6D);
        int left = mDaysLeft - days;
        if (left < 0) {
            left = 0;
        }
        mDaysLeft = left;
        if (!mUpgrading && mCountdown == 0x7F && mDaysLeft <= 0 && mSales.get() >= getSalesGoal(mStage) &&
            mStage != SHOP_STAGE_NOOKINGTONS) {
            mUpgrading = TRUE;
            mStageDate.setNow();
            mStageDate.toDayStart();
            mStageDate.addDays(mDaysLeft + 1);
            mNextStage = mStage + 1;
            dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 9);
            dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
            u16 msg = mStage + 7;
            fn_800CBAF0(2, mStageDate.get().month);
            fn_800CBB50(3, mStageDate.get().mday);
            dTime_c date = mStageDate.get();
            date.add(-1, 0, 0, 0);
            fn_800EBB24(msg, "BBS_tanukichi", &date, NULL);
        }
    } else if (days < 0) {
        if (mUpgrading) {
            dTime_c last = fgMngProc_getLastDayTime();
            int diff = mStageDate.diffDays(&last, TRUE, FALSE);
            mStageDate.setNow();
            mStageDate.toDayStart();
            mStageDate.addDays(-diff);
            dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 9);
        }
    }

    if (mDaysLeft < 0) {
        mDaysLeft = 0;
    }
    if (mNextStage != mStage && mUpgrading && mStageDate.diffDays(today.getTicks(), TRUE, FALSE) > 0) {
        setStage(mNextStage);
    }
}

// 80144D58
void dSaveShop_c::updateMail(int days) {
    static dMail_c sMail;

    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();

    if (today.get().month == MONTH_OCTOBER) {
        if (mOctoberDate.isNone() || mOctoberDate.get().year != today.get().year) {
            for (int i = 0; i < PLAYER_NUM; i++) {
                dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
                if (player != NULL && player->mPID.isValid()) {
                    player->setFlag0(0x56);
                }
            }
            mOctoberDate.setNow();
            mOctoberDate.toDayStart();
        }
    }
    if (today.get().month == MONTH_DECEMBER && today.get().mday >= 15 && today.get().mday <= 24) {
        if (mDecemberDate.isNone() || mDecemberDate.get().year != today.get().year) {
            for (int i = 0; i < PLAYER_NUM; i++) {
                dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
                if (player != NULL && player->mPID.isValid()) {
                    player->setFlag0(0x84);
                }
            }
            mDecemberDate.setNow();
            mDecemberDate.toDayStart();
        }
    }

    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
        if (player == NULL || !player->mPID.isValid()) {
            continue;
        }

        int rank = getRank(player);
        if (rank >= 1 && !player->isFlag0(0x33)) {
            static const u16 kind = 1;
            static const u8 sender[2] = {0x0F, 0x00};
            static const int paper = 0x154;
            sMail.setupSystem(&kind, "MAIL_ETC_TPS", sender, &player->mPID, &paper);
            dItem::Item gift(dItem::ITEM_IDX_NOOKS_CRANNY);
            fn_800CBDA0(0, &gift);
            sMail.setPresent(gift.mId, 0xFF);
            if (dPostOffice::add(&sMail)) {
                player->setFlag0(0x33);
            }
        }
        if (rank >= 2 && !player->isFlag0(0x34)) {
            static const u16 kind = 2;
            static const u8 sender[2] = {0x0F, 0x00};
            static const int paper = 0x154;
            sMail.setupSystem(&kind, "MAIL_ETC_TPS", sender, &player->mPID, &paper);
            dItem::Item gift(dItem::ITEM_IDX_NOOK_N_GO);
            fn_800CBDA0(0, &gift);
            sMail.setPresent(gift.mId, 0xFF);
            if (dPostOffice::add(&sMail)) {
                player->setFlag0(0x34);
            }
        }
        if (rank >= 3 && !player->isFlag0(0x35)) {
            static const u16 kind = 3;
            static const u8 sender[2] = {0x0F, 0x00};
            static const int paper = 0x154;
            sMail.setupSystem(&kind, "MAIL_ETC_TPS", sender, &player->mPID, &paper);
            dItem::Item gift(dItem::ITEM_IDX_NOOKWAY);
            fn_800CBDA0(0, &gift);
            sMail.setPresent(gift.mId, 0xFF);
            if (dPostOffice::add(&sMail)) {
                player->setFlag0(0x35);
            }
        }
        if (rank >= 4 && !player->isFlag0(0x36)) {
            static const u16 kind = 4;
            static const u8 sender[2] = {0x0F, 0x00};
            static const int paper = 0x154;
            sMail.setupSystem(&kind, "MAIL_ETC_TPS", sender, &player->mPID, &paper);
            dItem::Item gift(dItem::ITEM_IDX_NOOKINGTONS);
            fn_800CBDA0(0, &gift);
            sMail.setPresent(gift.mId, 0xFF);
            if (dPostOffice::add(&sMail)) {
                player->setFlag0(0x36);
            }
        }
        if (player->fn_801393AC()) {
            static const u16 kind = 5;
            static const u8 sender[2] = {0x10, 0x00};
            static const int paper = 0x157;
            sMail.setupSystem(&kind, "MAIL_ETC_TPS", sender, &player->mPID, &paper);
            dItem::Item gift(dItem::ITEM_IDX_SILVER_CAN);
            fn_800CBDA0(0, &gift);
            sMail.setPresent(gift.mId, 0xFF);
            if (dPostOffice::add(&sMail)) {
                player->setFlag0(0x18);
            }
        }

        if (days != 0 && player->isFlag0(0x56) && !player->isFlag0(0xD)) {
            if (today.get().month != MONTH_OCTOBER) {
                player->clearFlag0(0x56);
            } else {
                static const u16 kind = 1;
                static const u8 sender[2] = {0x0E, 0x00};
                static const int paper = 0x17C;
                sMail.setupSystem(&kind, "MAIL_NPC_Pumpking", sender, &player->mPID, &paper);
                if (dPostOffice::deliverToPlayer(&sMail)) {
                    player->clearFlag0(0x56);
                } else if (dPostOffice::add(&sMail)) {
                    player->clearFlag0(0x56);
                }
            }
        }
        if (days != 0 && player->isFlag0(0x84) && !player->isFlag0(0xD)) {
            if (today.get().month != MONTH_DECEMBER || today.get().mday < 15 || today.get().mday > 24) {
                player->clearFlag0(0x84);
            } else {
                static const u16 kind = 2;
                static const u8 sender[2] = {0x12, 0x00};
                static const int paper = 0x162;
                sMail.setupSystem(&kind, "MAIL_NPC_Pumpking", sender, &player->mPID, &paper);
                if (dPostOffice::deliverToPlayer(&sMail)) {
                    player->clearFlag0(0x84);
                } else if (dPostOffice::add(&sMail)) {
                    player->clearFlag0(0x84);
                }
            }
        }
    }
}

// 8014538C
void dSaveShop_c::decideNextStage() {
    int votes[3];
    votes[0] = 0;
    votes[1] = 0;
    votes[2] = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        switch (dPlayerMgr_c::getPlayer(i)->_868F) {
        case 1:
            votes[0]++;
            break;
        case 2:
            votes[1]++;
            break;
        case 3:
            votes[2]++;
            break;
        }
    }

    int top = 0;
    for (int i = 0; i < 3; i++) {
        if (votes[i] > top) {
            top = votes[i];
        }
    }
    int ties = 0;
    for (int i = 0; i < 3; i++) {
        if (top == votes[i]) {
            ties++;
        }
    }

    if (top <= 0) {
        mVote = 4;
    } else {
        mVote = 4;
        int n = cM::rndInt(ties) + 1;
        int k = 0;
        for (int i = 0; i < 3; i++) {
            if (top == votes[i]) {
                k++;
                if (k == n) {
                    switch (i) {
                    case 0:
                        mVote = 1;
                        break;
                    case 1:
                        mVote = 2;
                        break;
                    case 2:
                        mVote = 3;
                        break;
                    }
                    break;
                }
            }
        }
    }

    switch (mVote) {
    case 1:
        mNextStage = 1;
        break;
    case 2:
        mNextStage = 2;
        break;
    case 3:
        mNextStage = 3;
        break;
    }
}

// 80145590
void dSaveShop_c::update() {
    for (int i = 0; i < SHOP_STOCK_NUM; i++) {
        mStock[i].mItem = dItem::ITEM_ID_NONE;
        mStock[i].mSold = FALSE;
    }

    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();
    if (dSaveData_c::getRaw()->isFlag(0x15) && !isUpgradingToday() && !isEventToday() &&
        cM::rndInt(14) < sRates[1]) {
        mSaleDate = dTimeStamp_c(dTime_c::getCurrent());
        mSaleDate.toDayStart();
        mSaleDate.add(0, (u8)(cM::rndInt(4) + 11), 0, 0);
    } else {
        mSaleDate.reset();
    }

    stockTools();
    mStock[6].mItem = dItem::Item(dItem::ITEM_IDX_MEDICINE).mId;
    stockSeeds();
    stockSaplings();

    dItem::Item ftrs[8];
    int num = getFtrNum();
    {
        int range[2];
        range[0] = dItem::KIND_FTR;
        range[1] = 0;
        fn_800C60B4(ftrs, num, range, 1, lbl_8059FF80, NULL, 0, 0);
    }

    u8 rate = fn_800AC320()[1];
    BOOL extra = FALSE;
    if (dSaveData_c::getRaw()->isFlag(0x15) && !isUpgradingToday()) {
        if (*(u16 *)((u8 *)dSaveData_c::getTown() + 0x5EC74) & 0x20) {
            extra = TRUE;
        } else if (!isEventToday() && !isSaleToday() && cM::rndInt(100) < rate / 20 + 20) {
            extra = TRUE;
        }
    }
    if (extra) {
        int range[2];
        range[0] = 0x29;
        range[1] = 6;
        int slot = 0;
        if (mStage == SHOP_STAGE_NOOKINGTONS) {
            slot = 2;
        }
        fn_800C60B4(&ftrs[slot], 1, range, 1, lbl_8059FF80, NULL, 0, 0);
    }

    int seasonal = 0;
    if (today.get().month == MONTH_DECEMBER) {
        if (today.get().mday >= 1 && today.get().mday <= 23) {
            if (getRegion() == 1) {
                if (cM::rndInt(2) == 0) {
                    seasonal = 0x2A;
                } else {
                    seasonal = 0x2B;
                }
            } else {
                seasonal = 0x2A;
            }
        } else if (today.get().mday == 24) {
            seasonal = 0x2A;
        } else if (today.get().mday >= 26 && today.get().mday <= 31) {
            if (getRegion() == 0) {
                seasonal = 0x52;
            } else if (getRegion() == 1) {
                seasonal = 0x2B;
            }
        }
    } else if (getRegion() == 0 && today.get().month == MONTH_MARCH && today.get().mday >= 1 && today.get().mday <= 3) {
        seasonal = 0x53;
    }
    if (seasonal != 0) {
        int slot = 1;
        if (mStage == SHOP_STAGE_NOOKINGTONS) {
            slot = 7;
        }
        int range[2];
        range[0] = dItem::KIND_FTR;
        range[1] = seasonal;
        fn_800C60B4(&ftrs[slot], 1, range, 1, lbl_8059FF80, NULL, 0, 0);
        dItem::infoBank_c::get()->getBITM(ftrs[1]);
    }
    for (int i = 0; i < num; i++) {
        mStock[17 + i].mItem = ftrs[i].mId;
    }

    dItem::Item walls[3];
    num = getWallNum();
    {
        int range[2];
        range[0] = dItem::KIND_WALL;
        range[1] = 0;
        fn_800C60B4(walls, num, range, 1, lbl_8059FF80, NULL, 0, 0);
    }
    for (int i = 0; i < num; i++) {
        mStock[25 + i].mItem = walls[i].mId;
    }

    dItem::Item carpets[3];
    num = getCarpetNum();
    {
        int range[2];
        range[0] = dItem::KIND_CARPET;
        range[1] = 0;
        fn_800C60B4(carpets, num, range, 1, lbl_8059FF80, NULL, 0, 0);
    }
    for (int i = 0; i < num; i++) {
        mStock[28 + i].mItem = carpets[i].mId;
    }

    dItem::Item papers[4];
    num = getPaperNum();
    {
        int range[2];
        range[0] = dItem::KIND_PAPER;
        range[1] = 0;
        fn_800C60B4(papers, num, range, 1, lbl_8059FF80, NULL, 0, 0);
    }
    for (int i = 0; i < num; i++) {
        mStock[31 + i].mItem = papers[i].mId;
    }

    for (u32 i = 0; i < getPaintNum(); i++) {
        mStock[35 + i].mItem = dItem::Item(dItem::ITEM_IDX_RED_PAINT, mPaintIdx, FALSE).mId;
        mPaintIdx++;
        if ((u32)mPaintIdx >= 12) {
            mPaintIdx = 0;
        }
    }

    postNotices();
}

// 80145FB4
int dSaveShop_c::getToolNum() {
    return sToolNum[mStage];
}

// 80145FC4
int dSaveShop_c::getFtrNum() {
    return sFtrNum[mStage];
}

// 80145FD4
int dSaveShop_c::getSeedNum() {
    return sSeedNum[mStage];
}

// 80145FE4
int dSaveShop_c::getSaplingNum() {
    return sSaplingNum[mStage];
}

// 80145FF4
int dSaveShop_c::getWallNum() {
    return sWallNum[mStage];
}

// 80146004
int dSaveShop_c::getCarpetNum() {
    return sCarpetNum[mStage];
}

// 80146014
int dSaveShop_c::getPaperNum() {
    return sPaperNum[mStage];
}

// 80146024
int dSaveShop_c::getPaintNum() {
    return sPaintNum[mStage];
}

// 80146034
u16 dSaveShop_c::getItem(int slot) {
    if (slot < 0 || slot >= SHOP_STOCK_NUM) {
        return dItem::ITEM_ID_NONE;
    }
    if (!isSold(slot)) {
        return mStock[slot].mItem;
    }
    return dItem::Item(dItem::ITEM_IDX_SOLD_OUT_SIGN_00).mId;
}

// 801460B0
void dSaveShop_c::sellAndNotify(int scene, int slot, int unitX, int unitZ, int price, int flag) {
    if ((u32)slot != 0xFF) {
        sell(scene, slot, unitX, unitZ);
    }
    addSales(price, flag);
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        dShopSale_c sale;
        sale.mSlot = slot;
        sale.mUnitX = unitX;
        sale.mUnitZ = unitZ;
        sale.mPrice = price;
        sale.mFlag = flag;
        sale.mScene = scene;
        fn_800DD4C8();
        fn_800DD518(&sale, sizeof(dShopSale_c));
        fn_800DD588(0x42, 4);
    }
}

// 80146170
void dSaveShop_c::addSales(int price, int flag) {
    mSales.set(price + mSales.get());
    if (mSales.get() >= getSalesGoal(mStage)) {
        mSales.set(getSalesGoal(mStage));
    }
    if (!mUpgrading && mCountdown == 0x7F && mDaysLeft <= 0 && mSales.get() >= getSalesGoal(mStage) &&
        mStage != SHOP_STAGE_NOOKINGTONS) {
        mUpgrading = TRUE;
        mStageDate.setNow();
        mStageDate.toDayStart();
        mStageDate.addDays(1);
        mNextStage = mStage + 1;
        dPrivateData_c::setFlag0All(dSaveData_c::getTown()->mPlayers, 9);
        dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 10);
        u16 msg = mStage + 7;
        fn_800CBAF0(2, mStageDate.get().month);
        fn_800CBB50(3, mStageDate.get().mday);
        fn_800EBB24(msg, "BBS_tanukichi", dTime_c::getCurrent(), NULL);
    }
}

// 801462F4
void dSaveShop_c::sell(int scene, int slot, int unitX, int unitZ) {
    if (slot >= 0 && slot < SHOP_STOCK_NUM) {
    } else {
        return;
    }
    if (!isSold(slot)) {
        dItem::Item item = *(dItem::Item *)&mStock[slot].mItem;
        switch (item.getKind()) {
        case dItem::KIND_PAPER:
            return;
        }
        mStock[slot].mSold = TRUE;
    } else {
        return;
    }

    if ((u32)scene != getCurrentScene()) {
        return;
    }
    int idx = fn_800A8F98(fn_800A9058(), unitX, unitZ, 0);
    if (idx != -1) {
        fn_800ABAF4(idx);
        return;
    }
    dItem::Item sold(dItem::ITEM_IDX_SOLD_OUT_SIGN_00);
    dFdBase_c *fd = fn_80190C44(0);
    if (fd != NULL) {
        fd->setItem(&sold, 0, 0, unitX, unitZ, 0);
    }
}

// 80146400
BOOL dSaveShop_c::isSold(int slot) {
    if (slot < 0 || slot >= SHOP_STOCK_NUM) {
        return TRUE;
    }
    return mStock[slot].mSold != 0;
}

// 80146434
void dSaveShop_c::stockTools() {
    u8 used[7];
    used[0] = FALSE;
    used[1] = FALSE;
    used[2] = FALSE;
    used[3] = FALSE;
    used[4] = FALSE;
    used[5] = FALSE;
    used[6] = FALSE;
    BOOL october = FALSE;
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();

    for (u32 i = 0; i < getToolNum(); i++) {
        if (i == 0 && today.get().month == MONTH_OCTOBER) {
            dItem::Item item;
            int range[2];
            range[0] = 0x30;
            range[1] = 0x2C;
            fn_800C60B4(&item, 1, range, 1, lbl_8059FF80, NULL, 0, 0);
            october = TRUE;
            mStock[i].mItem = item.mId;
            continue;
        }

        int idx = -1;
        if (i == 0 && mStage == SHOP_STAGE_CRANNY) {
            idx = 0;
        } else {
            int max = mStage == SHOP_STAGE_CRANNY ? 4 : 7;
            int n;
            if (october) {
                n = cM::rndInt(max - (i - 1));
            } else {
                n = cM::rndInt(max - i);
            }
            int k = -1;
            do {
                idx++;
                while (used[idx] == TRUE) {
                    idx++;
                }
                k++;
            } while (k < n);
        }
        dItem::Item item(sTools[idx]);
        mStock[i].mItem = item.mId;
        used[idx] = TRUE;
    }

    if (mStage != SHOP_STAGE_CRANNY && cM::rndInt(100) < sRates[0]) {
        u16 silver;
        switch (cM::rndInt(3)) {
        case 0:
            silver = dItem::Item(dItem::ITEM_IDX_SILVER_NET).mId;
            break;
        case 1:
            silver = dItem::Item(dItem::ITEM_IDX_SILVER_ROD).mId;
            break;
        case 2:
        default:
            silver = dItem::Item(dItem::ITEM_IDX_SILVER_SLINGSHOT).mId;
            break;
        }
        int slot;
        if (october) {
            slot = cM::rndInt(getToolNum() - 1) + 1;
        } else {
            slot = cM::rndInt(getToolNum());
        }
        mStock[slot].mItem = silver;
    }
}

// 801466A0
void dSaveShop_c::stockSeeds() {
    u8 used[12];
    int i = 0;
    while (i < 12) {
        used[i++] = FALSE;
    }
    for (u32 i = 0; i < getSeedNum(); i++) {
        int n = cM::rndInt(12 - i);
        int k = -1;
        int idx = -1;
        do {
            idx++;
            while (used[idx] == TRUE) {
                idx++;
            }
            k++;
        } while (k < n);
        dItem::Item item(sSeeds[idx]);
        used[idx] = TRUE;
        mStock[7 + i].mItem = item.mId;
    }
}

// 80146790
void dSaveShop_c::stockSaplings() {
    u32 i;
    for (i = 0; i < getSaplingNum() - (getSaplingNum() & 1); i++) {
        mStock[14 + i].mItem = dItem::Item(sSaplings[(int)i % 2]).mId;
    }
    if (getSaplingNum() & 1) {
        int slot = i + 14;
        mStock[slot].mItem = dItem::Item(sSaplings[cM::rndInt(2)]).mId;
    }
}

// 80146864
BOOL dSaveShop_c::isEventToday() {
    return dEvent::isSavedDateToday() != 0;
}

// 80146890
BOOL dSaveShop_c::isSaleToday() {
    BOOL today = FALSE;
    if (!mSaleDate.isNone() && mSaleDate.isToday(TRUE)) {
        today = TRUE;
    }
    return today;
}

// 801468F0
BOOL dSaveShop_c::isSaleNow() {
    dTime_c now = *dTime_c::getCurrent();
    BOOL started = FALSE;
    BOOL today = FALSE;
    if (!mSaleDate.isNone() && isSaleToday()) {
        today = TRUE;
    }
    if (today) {
        dTime_c time = now;
        if (mSaleDate.compare(&time) >= 0) {
            started = TRUE;
        }
    }
    return started;
}

// 80146A1C
BOOL dSaveShop_c::hasFrom6() {
    for (int i = 0; i < SHOP_STOCK_NUM; i++) {
        if (!isSold(i)) {
            dItem::Item item = *(dItem::Item *)&mStock[i];
            if (item.getFrom() == 6) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80146AA4
int dSaveShop_c::getRank(dPrivateData_c *player) {
    if (player == NULL) {
        return 0;
    }
    return getRank(player->mNookPointsLifetime);
}

// 80146AC0
int dSaveShop_c::getRank(u16 points) {
    if (points < 300) {
        return 0;
    }
    if (points < 5000) {
        return 1;
    }
    if (points < 10000) {
        return 2;
    }
    int rank = 4;
    if (points < 20000) {
        rank = 3;
    }
    return rank;
}

// 80146B04
u16 dSaveShop_c::getPointsToNextRank(u16 points) {
    switch (getRank(points)) {
    case 0:
        return 300 - points;
    case 1:
        return 5000 - points;
    case 2:
        return 10000 - points;
    case 3:
        return 20000 - points;
    case 4:
        break;
    }
    return 0;
}

// 80146B90
void dSaveShop_c::postNotices() {
    if (!mStageDate.isNone() && mStageDate.isToday(TRUE)) {
        return;
    }

    fn_800CBD30(0, mStage + 0x2E, "sys_STRING/STR_Unit");
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();
    dTime_c date = today.get();

    if (isSaleToday()) {
        u16 msg = cM::rndInt(2) + 3;
        fn_800CBFE0(mSaleDate.get().hour, 4, 0);
        fn_800EBB24(msg, "BBS_tanukichi", &date, NULL);
    }

    if (hasFrom6()) {
        u16 msg = cM::rndInt(2) + 5;
        for (int i = 0; i < SHOP_STOCK_NUM; i++) {
            if (!isSold(i)) {
                dItem::Item item = *(dItem::Item *)&mStock[i].mItem;
                if (item.getFrom() == 6) {
                    fn_800CBDA0(1, &item);
                    break;
                }
            }
        }
        fn_800EBB24(msg, "BBS_tanukichi", &date, NULL);
    }
}

// 80146D68
void dSaveShop_c::initRates() {
    sRates[0] = 2;
    sRates[1] = 1;
}
