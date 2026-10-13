// The town's jinx ("charm"). .text 8014F8FC..80150140, .rodata 80476668..804766C0, .data 804EF5C0..804EF5E8 (the
// value switch's jump table). See include/game/game/d_sv_activity.hpp.
#include <game/game/d_sv_activity.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/cLib/c_math.hpp>

// The chance of each condition and outcome, in percent.
static const int sConditionRates[JINX_COND_NUM + 1] = {7, 7, 12, 12, 15, 7, 0, 5, 15, 15, 5, 0};
static const int sOutcomeRates[JINX_OUTCOME_NUM] = {20, 7, 10, 10, 7, 10, 7, 15, 7, 7};

// 8014F8FC
void dSavePostSend_c::set(const dPersonalID_c *to) {
    mTo.copy(to);
    mSender.copy(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID);
    mPending = TRUE;
}

// 8014F948
void dSaveJinx_c::clear() {
    mCondition = JINX_COND_NUM;
    mOutcome = JINX_OUTCOME_NUM;
    mValue = 0;
}

// 8014F96C
void dSvActivity_c::clear() {
    mYear = -1;
    mToday.clear();
    mPending.clear();
    mActive = 0;
}

// 8014F998
void dSvActivity_c::updateMonth(const dTime_c *now) {
    if (mYear == now->year && mMonth == now->month) {
        return;
    }
    dSaveData_c::getTown()->mTownList.clear();
    mYear = now->year;
    mMonth = now->month;

    int condition = 0;
    int r = cM::rndInt(100);
    for (int i = 0; i < JINX_COND_NUM; i++) {
        if ((r -= sConditionRates[i]) < 0) {
            condition = i;
            break;
        }
    }
    mJinx.mCondition = condition;
    switch (condition) {
    case JINX_COND_CUT_TREE:
        mJinx.mValue = 1;
        break;
    case JINX_COND_PULL_WEEDS:
        mJinx.mValue = cM::rndInt(9) + 1;
        break;
    case JINX_COND_PLANT_TREES:
        mJinx.mValue = cM::rndInt(3) + 1;
        break;
    case JINX_COND_PLANT_FLOWERS:
        mJinx.mValue = cM::rndInt(5) + 1;
        break;
    case JINX_COND_CATCH_BUG:
        mJinx.mValue = dInsectInfo::getRandomOfTerm();
        break;
    case JINX_COND_CATCH_FISH:
        mJinx.mValue = dFishInfo::getRandomOfTerm();
        break;
    }

    int outcome = 0;
    r = cM::rndInt(100);
    for (int i = 0; i < JINX_OUTCOME_NUM; i++) {
        if ((r -= sOutcomeRates[i]) < 0) {
            outcome = i;
            break;
        }
    }
    mJinx.mOutcome = outcome;

    // Friend towns hear about it.
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mVillagerMove.mJinx = mJinx;
    town->mVillagerMove.mFlags |= dSvVillagerMove::VILLAGER_MOVE_FLAG_JINX;
    dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x82);
}

// 8014FBF0: the outcomes a visitor can bring about
void dSvActivity_c::setVisitorDone(BOOL done) {
    if (done) {
        int outcome = mJinx.mOutcome;
        switch (outcome) {
        case JINX_OUTCOME_NO_BEES:
        case JINX_OUTCOME_TOWN_IMPROVES:
        case JINX_OUTCOME_FRIENDSHIP:
        case JINX_OUTCOME_MONEY:
        case JINX_OUTCOME_DROPPED_ITEMS:
            return;
        default:
            if (fgMngProc_isDayUpToDate()) {
                mToday.mOutcomes |= 1 << outcome;
            } else {
                mPending.mOutcomes |= 1 << outcome;
            }
            break;
        }
    }
}

// 8014FCA0
void dSvActivity_c::setDone(int outcome, BOOL done, int player) {
    if (done) {
        dSvActivityResult_c *result = fgMngProc_isDayUpToDate() ? &mToday : &mPending;
        result->mOutcomes |= 1 << outcome;
        if (outcome == JINX_OUTCOME_FRIENDSHIP) {
            result->mFriendshipPlayers |= 1 << player;
        } else if (outcome == JINX_OUTCOME_MONEY) {
            result->mMoneyPlayers |= 1 << player;
        }
    }
}

// 8014FD48
void dSvActivity_c::advanceDays(dTime_c last, dTime_c *now, int *days) {
    int num = *days;
    if (num >= 14) {
        num = 14;
    } else if (num < 0) {
        mActive = 0;
        mToday.clear();
        mPending.clear();
        return;
    }
    while (num-- != 0) {
        last.add(1, 0, 0, 0);
        if (last.year >= 2036) {
            last.year -= 36;
            last.normalize();
        }
        mMoneyPlayers = 0;
        u16 keep = 0;
        // The town keeps improving, except on Sundays.
        if (last.wday != 0) {
            keep |= 1 << JINX_OUTCOME_TOWN_IMPROVES;
        }
        // Dropped items for a week.
        if (mActive & (1 << JINX_OUTCOME_DROPPED_ITEMS)) {
            dTime_c date;
            date.set(mDropDate.year, mDropDate.month, mDropDate.day, 0, 0, 0);
            int diff = dTime_c::diffDays(now, &date, TRUE);
            if (diff < 7 && diff >= 0) {
                keep |= 1 << JINX_OUTCOME_DROPPED_ITEMS;
            }
        }
        mActive &= keep;
        for (int i = 0; i < JINX_OUTCOME_NUM; i++) {
            if (mToday.mOutcomes & (1 << i)) {
                applyOutcome(i, last, mToday.mFriendshipPlayers, mToday.mMoneyPlayers);
            }
        }
        BOOL improves = mToday.mOutcomes & (1 << JINX_OUTCOME_TOWN_IMPROVES);
        mToday = mPending;
        if (improves) {
            mToday.mOutcomes |= 1 << JINX_OUTCOME_TOWN_IMPROVES;
        }
        mPending.clear();
    }
}

// 8014FF14
void dSvActivity_c::applyOutcome(const int &outcome, const dTime_c &time, const int &friendshipPlayers,
                                 const int &moneyPlayers) {
    mActive |= 1 << outcome;
    switch (outcome) {
    case JINX_OUTCOME_DROPPED_ITEMS:
        mDropDate.set(&time);
        break;
    case JINX_OUTCOME_FRIENDSHIP:
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (friendshipPlayers & (1 << i)) {
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
                dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
                if (pid != NULL) {
                    dAnimalBlock_c *animals = &dSaveData_c::getTown()->mAnimals.mTown;
                    for (int j = 0; j < ANIMAL_NUM; j++) {
                        dAnimal_c *animal = animals->getAnimal(j);
                        if (animal != NULL && animal->mID.isValid()) {
                            dAnimalMemory_c *memory = animal->findMemory(pid);
                            if (memory != NULL) {
                                memory->addFriendship(3);
                            }
                        }
                    }
                }
            }
        }
        break;
    case JINX_OUTCOME_MONEY:
        mMoneyPlayers |= moneyPlayers;
        break;
    case JINX_OUTCOME_TOWN_IMPROVES:
        if (time.wday == 0) {
            mToday.mOutcomes &= ~(1 << JINX_OUTCOME_TOWN_IMPROVES);
        } else {
            mActive &= ~(1 << JINX_OUTCOME_TOWN_IMPROVES);
        }
        break;
    }
}

// 80150080
void dSvActivity_c::setTodayOutcomes(u16 outcomes) {
    mToday.mOutcomes = outcomes;
}

// 80150088
void dSvActivity_c::removePlayer(int player) {
    mToday.removePlayer(player);
    mPending.removePlayer(player);
    mMoneyPlayers &= ~(1 << player);
    if (mMoneyPlayers == 0) {
        mActive &= ~(1 << JINX_OUTCOME_MONEY);
    }
}

// 801500F8
void dSvActivityResult_c::removePlayer(int player) {
    int mask = ~(1 << player);
    if ((mFriendshipPlayers &= mask) == 0) {
        mOutcomes &= ~(1 << JINX_OUTCOME_FRIENDSHIP);
    }
    if ((mMoneyPlayers &= mask) == 0) {
        mOutcomes &= ~(1 << JINX_OUTCOME_MONEY);
    }
}
