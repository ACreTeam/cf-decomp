// The house rating (Happy Room Academy). .text 800AC260..800B4DA0. See include/game/game/d_hr.hpp.
#include <game/game/d_hr.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/cLib/c_math.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_date.hpp>

// Data. .bss order: l_mail (with its dtor chain record), l_rates, l_seriesScores, l_luckyItems, l_seriesParts,
// the three part tables, receiveModelRoom's static dSvMdlRm_c, then l_remarks (defined after updateRemarks).
// The dMail_c::setupSystem arguments of rate / sendRatingLetter / sendModelPresent are .sdata statics.
dMail_c l_mail;                           // 80596FB0
dHR::rate_c l_rates[4];                   // 80597340: by dHomeList_c index
dHR::seriesScores_c l_seriesScores;       // 80597350
dHR::ftrSet_c l_luckyItems;               // 80597560
dHR::seriesParts_c l_seriesParts;         // 80597760
u16 l_basicParts[dItem::SERIES_COUNT];    // 80597978
u16 l_themeParts[dItem::SERIES_COUNT];    // 80597A80
u16 l_setParts[dItem::SERIES_COUNT];      // 80597B88

// 800AC260: an added series (SERIES_EXTRA_BASIC_1 and up: no name in STR_Furniture)
BOOL dHR::isExtraSeries(int series) {
    return series >= dItem::SERIES_EXTRA_BASIC_1;
}

// 800AC278
BOOL fn_800AC278() {
    return TRUE;
}

// 800AC280
void fn_800AC280() {}

// 800AC284
BOOL fn_800AC284() {
    return TRUE;
}

// 800AC28C
const u8 *dHR::getRate(u32 home) {
    if (home == 4) {
        int cur = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
        if (cur != -1) {
            return l_rates[cur & 3].mScore;
        }
        static dHR::rate_c s_none;
        return s_none.mScore;
    }
    return l_rates[home & 3].mScore;
}

// 800AC320: the average rating of the rated houses
const u8 *dHR::getAverageRate() {
    static dHR::rate_c s_avg;
    s_avg.clear();
    u32 num = 0;
    for (u32 i = 0; i < 4; i++) {
        const u8 *rate = dHR::getRate(i);
        if (rate[3] != 0) {
            num++;
            s_avg.mScore[0] += rate[0];
            s_avg.mScore[1] += rate[1];
            s_avg.mScore[2] += rate[2];
            s_avg.mValid = TRUE;
        }
    }
    if (num != 0) {
        f32 n = num;
        u32 score2 = s_avg.mScore[2] / n;
        u32 score0 = s_avg.mScore[0] / n;
        u32 score1 = s_avg.mScore[1] / n;
        s_avg.mScore[0] = score0;
        s_avg.mScore[2] = score2;
        s_avg.mScore[1] = score1;
    }
    return s_avg.mScore;
}

// 800AC4A4
void dHR::updateRates() {
    for (u32 i = 0; i < 4; i++) {
        dHR::updateRate(i);
    }
}

// 800AC4E0: rates player house home (4: the current player's)
void dHR::updateRate(u32 home) {
    if (home == 4) {
        home = dSaveData_c::getRaw()->mHomes.findCurrentPlayer();
    }
    dHR::rate_c *rate = &l_rates[home];
    rate->clear();
    if (dSaveData_c::getRaw()->mHomes.getHomePlayerRaw(home) != NULL) {
        for (int room = 0; room < 3; room++) {
            dHR::fnShui_c shui;
            dFdBase_c *field = fn_80191178(home, room);
            if (field != NULL) {
                shui.calc(field);
                rate->mScore[0] += shui.mScore[0];
                rate->mScore[1] += shui.mScore[1];
                rate->mScore[2] += shui.mScore[2];
            }
        }
        rate->mValid = TRUE;
    } else {
        rate->mScore[0] = 0;
        rate->mScore[1] = 0;
        rate->mScore[2] = 0;
        rate->mValid = 0;
    }
}

// 800AC5F8
u32 dHR::getPictureRate() {
    u32 s0 = 0;
    u32 s1 = 0;
    u32 s2 = 0;
    u32 num = 0;
    for (u32 i = 0; i < 4; i++) {
        const u8 *rate = dHR::getRate(i);
        if (rate[3] != 0) {
            num++;
            s0 += rate[0];
            s1 += rate[1];
            s2 += rate[2];
        }
    }
    return (f32)(s0 + s1 + s2) / (10.0f * num);
}

// 800AC6B4
u32 dHR::getFtrRate() {
    u32 s1 = 0;
    u32 s2 = 0;
    u32 num = 0;
    for (u32 i = 0; i < 4; i++) {
        const u8 *rate = dHR::getRate(i);
        if (rate[3] != 0) {
            num++;
            s1 += rate[1];
            s2 += rate[2];
        }
    }
    return (f32)(s1 + s2) / (10.0f * num);
}

// 800AC770
dHR::fnShui_c::fnShui_c() {
    mScore[0] = mScore[1] = mScore[2] = 0;
    mItemL = mItemB = mItemR = dItem::ITEM_ID_NONE;
}

// 800AC798: furniture inside (x0..x1, z0..z1) with a colour (A or B) of color; *out = the last one
u8 dHR::fnShui_c::count(dFdBase_c *field, u32 x0, u32 x1, u32 z0, u32 z1, int color, dItem::Item *out) {
    u32 num = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = z0; z <= z1; z++) {
            for (u32 x = x0; x <= x1; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->hasFtrFunc()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    dNpcFtrShape_c shape;
                    fn_800A8B28(&shape, *item);
                    u32 tiles = fn_800A8BB8(&shape);
                    BOOL inside = TRUE;
                    for (u32 i = 0; i < tiles; i++) {
                        int ux = x + fn_800A8BE4(&shape, i)[0];
                        int uz = z + fn_800A8BE4(&shape, i)[1];
                        if (ux < (int)x0 || ux > (int)x1 || uz < (int)z0 || uz > (int)z1) {
                            inside = FALSE;
                            break;
                        }
                    }
                    if (inside) {
                        int colorA;
                        if (bitm->m_ftrColorA < 15) {
                            colorA = bitm->resolveColor(bitm->m_ftrColorA, FALSE);
                        } else {
                            colorA = bitm->resolveColor(0, TRUE);
                        }
                        if (color == colorA) {
                            num++;
                            if (out != NULL) {
                                *out = *item;
                            }
                        }
                        int colorB;
                        if ((u32)(s8)bitm->m_ftrColorB < 15) {
                            colorB = bitm->resolveColor((s8)bitm->m_ftrColorB, FALSE);
                        } else {
                            colorB = bitm->resolveColor(0, TRUE);
                        }
                        if (color == colorB) {
                            num++;
                            if (out != NULL) {
                                *out = *item;
                            }
                        }
                    }
                }
            }
        }
    }
    if (num >= 0x100) {
        return 0xFF;
    }
    return num;
}

// 800AC99C
void dHR::fnShui_c::calc(dFdBase_c *field) {
    int x0, x1, z0, z1;
    dHR::calcBounds(&x0, &x1, &z0, &z1, field);
    mScore[0] = count(field, x0, x0 + 1, z0, z1, 1, &mItemL);
    mScore[1] = count(field, x0, x1, z1 - 1, z1, 4, &mItemB);
    mScore[2] = count(field, x1 - 1, x1, z0, z1, 2, &mItemR);
}

// 800ACA60
void dHR::seriesParts_c::add(dItem::Item item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm != NULL) {
        int series = bitm->getSeries();
        if (series < dItem::SERIES_COUNT) {
            int part = bitm->getFtrPartA();
            u32 bit = 0;
            if (part == 1) {
                bit = 1;
            } else if (part == 2) {
                bit = 2;
            } else if (part == 3) {
                bit = 4;
            } else if (part == 4) {
                bit = 8;
            } else if (part == 5) {
                bit = 0x10;
            }
            mParts[series] = bit | mParts[series];
            mAll |= bit;
        }
    }
}

// 800ACB48
dHR::fdMdlCb_c::fdMdlCb_c(const dHomeRoom_c *room, dFdBase_c *field) {
    mWallpaper = room->getWallpaper();
    mCarpet = room->getCarpet();
    mField = field;
}

// 800ACBD0
dFdBase_c *dHR::fdMdlCb_c::getField(int room) {
    return mField;
}

// 800ACBD8
dItem::Item dHR::fdMdlCb_c::getWallpaper(int room) {
    return mWallpaper;
}

// 800ACBE4
dItem::Item dHR::fdMdlCb_c::getCarpet(int room) {
    return mCarpet;
}

// 800ACBF0
dFdBase_c *dHR::fdPlCb_c::getField(int room) {
    return fn_80190C44(mHome * 3 + room + 2);
}

// 800ACC08
dHomeRoom_c *dHR::fdPlCb_c::getRoom(int room) {
    u32 idx = mHome & 3;
    dSaveTown_c *town = dSaveData_c::getTown();
    dHome_c *home = town->mHomes.getHome(idx);
    if (home != NULL) {
        return home->getRoom(room);
    }
    return NULL;
}

// 800ACC6C
dItem::Item dHR::fdPlCb_c::getWallpaper(int room) {
    dHomeRoom_c *r = getRoom(room);
    if (r != NULL) {
        return r->getWallpaper();
    }
    return dItem::Item(dItem::ITEM_IDX_EXOTIC_WALL);
}

// 800ACCC4
dItem::Item dHR::fdPlCb_c::getCarpet(int room) {
    dHomeRoom_c *r = getRoom(room);
    if (r != NULL) {
        return r->getCarpet();
    }
    return dItem::Item(dItem::ITEM_IDX_EXOTIC_RUG);
}

static inline void fixItem(dItem::Item &item, int def) {
    if (dItem::isRealItemId(item.mId)) {
        if (dItem::getBITM(item.mId) == NULL) {
            item = dItem::Item(def);
        }
    }
}

// 800ACD1C
dHR::fdNpcCb_c::fdNpcCb_c(dFdBase_c *field, dItem::Item carpet, dItem::Item wallpaper) {
    mField = field;
    mCarpet = carpet;
    mWallpaper = wallpaper;
    if (mField != NULL) {
        mField->clearUnknownItems();
    }
    fixItem(mCarpet, dItem::ITEM_IDX_EXOTIC_RUG);
    fixItem(mWallpaper, dItem::ITEM_IDX_EXOTIC_WALL);
}

// 800ACE30
dFdBase_c *dHR::fdNpcCb_c::getField(int room) {
    return mField;
}

// 800ACE38
dItem::Item dHR::fdNpcCb_c::getWallpaper(int room) {
    return mWallpaper;
}

// 800ACE44
dItem::Item dHR::fdNpcCb_c::getCarpet(int room) {
    return mCarpet;
}


// 800ACE50
int dHR::quickRate(dHR::area_c *area, dHR::fdCb_c *cb, u32 *rank) {
    int r = 3;
    int flags = 0;
    u8 facingWall = 0;
    u8 messy = 0;
    l_seriesParts.clear();
    l_seriesScores.clear();
    dFdBase_c *field = cb->getField(0);
    dHR::calcBounds(&area->mMinX, &area->mMaxX, &area->mMinZ, &area->mMaxZ, field);
    area->mWallpaper = cb->getWallpaper(0);
    area->mCarpet = cb->getCarpet(0);
    dHR::calcWallPoints(area, NULL, field, &facingWall, &messy);
    dHR::collectParts(area, field);
    u32 x = 0;
    int n = dHR::checkBasicSeries(area, NULL, field, &x);
    int p = dHR::checkThemeSeries(area, NULL, field);
    int q = dHR::checkSetSeries(area, NULL, field);
    if (messy) {
        flags |= HR_LAYOUT_MESSY;
        r = 1;
    }
    if (facingWall) {
        flags |= HR_LAYOUT_FACING_WALL;
        r -= 2;
    }
    if (l_seriesParts.hasAllParts()) {
        flags |= HR_LAYOUT_COMFY;
        r++;
    }
    if (p || q) {
        flags |= HR_LAYOUT_THEME_OR_SET;
        r++;
    }
    if (!p || !q) {
        flags |= HR_LAYOUT_NO_THEME_OR_SET;
    }
    if (n != dItem::SERIES_COUNT) {
        flags |= HR_LAYOUT_SERIES;
        r++;
    } else if (x >= 5) {
        flags |= HR_LAYOUT_SERIES_PARTS;
    }
    if (r <= 1) {
        r = 1;
    }
    if (r >= 5) {
        r = 5;
    }
    if (rank != NULL) {
        *rank = r;
    }
    return flags;
}

// 800AD118
int dHR::quickRateAnimal(dHR::area_c *area, int animalIdx, u32 *rank) {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(animalIdx);
    if (rank != NULL) {
        *rank = 3;
    }
    if (animal != NULL) {
        dFdInfoNpcHs_c *field = dFdInfoNpcHs_c::create(animalIdx, heap);
        if (field != NULL) {
            dHR::fdNpcCb_c cb(field, *animal->getCarpet(), *animal->getWall());
            int flags = dHR::quickRate(area, &cb, rank);
            dFdInfoNpcHs_c::remove(&field, heap);
            return flags;
        }
    }
    return 0;
}

// 800AD1FC
int dHR::quickRatePlayer(dHR::area_c *area, u32 home, u32 *rank) {
    dHR::fdPlCb_c cb(home);
    return dHR::quickRate(area, &cb, rank);
}

// 800AD234
int dHR::ratePlayer(dHR::area_c *area, u32 home, int mode, int type, int start, int count, u8 *sent, bool letter) {
    dHR::fdPlCb_c cb(home);
    dHR::result_c res;
    return dHR::rate(area, &res, &cb, start, count, mode, type, sent, letter);
}

// 800AD2E0
int dHR::rateAnimal(dHR::area_c *area, int animalIdx, dHR::result_c *res, int type) {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(animalIdx);
    if (animal != NULL) {
        dFdInfoNpcHs_c *field = dFdInfoNpcHs_c::create(animalIdx, heap);
        if (field != NULL) {
            dHR::fdNpcCb_c cb(field, *animal->getCarpet(), *animal->getWall());
            int score = dHR::rate(area, res, &cb, 0, 1, 0, type, NULL, false);
            dFdInfoNpcHs_c::remove(&field, heap);
            return score;
        }
    }
    return 0;
}

// 800AD3D4
int dHR::rateModelRoom(dHR::area_c *area, dHR::result_c *res, dSvMdlRm_c *mdlRoom, int type) {
    BOOL owned = FALSE;
    if (mdlRoom->isOwned() && (mdlRoom->mPlayerID.isValid() || mdlRoom->mAnimalID.isValid())) {
        owned = TRUE;
    }
    if (owned) {
        EGG::Heap *heap = EGG::Heap::getCurrentHeap();
        dFdInfoSvMdlRm_c *field = dFdInfoSvMdlRm_c::createWithBg15(mdlRoom, heap);
        if (field != NULL) {
            dHR::fdMdlCb_c cb(mdlRoom, field);
            dHR::result_c local;
            if (res == NULL) {
                res = &local;
            }
            int score = dHR::rate(area, res, &cb, 0, 1, 0, type, NULL, false);
            dFdInfoSvMdlRm_c::remove(&field, heap);
            return score;
        }
        return 0;
    }
    return 0;
}

// 800AD514
BOOL dHR::hasOrgDesign(dHR::area_c *area, dHR::fdCb_c *cb, int start, int count) {
    for (int i = start; i < start + count; i++) {
        dFdBase_c *field = cb->getField(i);
        area->mWallpaper = cb->getWallpaper(i);
        area->mCarpet = cb->getCarpet(i);
        if (area->mWallpaper.isOrgDesign()) {
            return TRUE;
        }
        if (area->mCarpet.isOrgDesign()) {
            return TRUE;
        }
        for (int layer = 0; layer < 2; layer++) {
            for (u32 z = 0; z < 16; z++) {
                for (u32 x = 0; x < 16; x++) {
                    dItem::Item *item = field->getItem(x, z, layer);
                    if (item != NULL && dItem::isRealItemId(item->mId) && item->isOrgDesign()) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

// A player of this town (the land ID of the save).
static inline bool isTownPlayer(const dPrivateData_c *player) {
    return dSaveData_c::getRaw()->mLandID == player->mPID.land;
}

// 800AD680
int dHR::rate(dHR::area_c *area, dHR::result_c *res, dHR::fdCb_c *cb, int start, int count, int mode, int type,
                u8 *sent, bool letter) {
    if (res != NULL) {
        res->mFlags = 0;
    }
    if (type != 6 && dHR::hasOrgDesign(area, cb, start, count)) {
        return -1;
    }
    l_seriesScores.clear();
    l_luckyItems.clear();
    l_seriesParts.clear();

    int score;
    int end = start + count;
    int fromPoints = 0;
    int fengShuiPoints = 0;
    int colorPoints = 0;
    int genrePoints = 0;
    int categoryPoints = 0;
    int wallPoints = 0;
    int setScore = 0;
    u8 facingWall = FALSE;
    BOOL messy = FALSE;
    for (; start < end; start++) {
        dFdBase_c *field = cb->getField(start);
        area->mWallpaper = cb->getWallpaper(start);
        area->mCarpet = cb->getCarpet(start);
        dHR::calcBounds(&area->mMinX, &area->mMaxX, &area->mMinZ, &area->mMaxZ, field);
        dHR::fnShui_c obj;
        obj.calc(field);
        dHR::checkBasicSeries(area, res, field, NULL);
        dHR::checkThemeSeries(area, res, field);
        dHR::checkSetSeries(area, res, field);
        dHR::collectLucky(area, field);
        dHR::collectParts(area, field);
        fromPoints += dHR::calcFromPoints(area, field);
        fengShuiPoints += dHR::calcFengShuiPoints(area, &obj);
        colorPoints += dHR::calcColorPoints(area, field);
        genrePoints += dHR::calcGenrePoints(area, field, &res->mNewOld, &res->mAdultKiddy, type);
        categoryPoints += dHR::calcCategoryPoints(area, res, field, &res->mCategory);
        u8 a = 0;
        u8 b = 0;
        wallPoints -= dHR::calcWallPoints(area, res, field, &a, &b);
        if (a) {
            facingWall = TRUE;
        }
        if (b) {
            messy = TRUE;
        }
    }

    if (type == 6 || type == 0) {
        score = fromPoints;
        score += fengShuiPoints;
        score += colorPoints;
        score += genrePoints;
        score += categoryPoints;
        score += wallPoints;
    } else if (type == 1) {
        score = fengShuiPoints;
    } else {
        score = genrePoints;
    }

    int basicNum = 0;
    int themeNum = 0;
    u32 basicSeries = 0;
    u32 themeSeries = 0;
    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
        u32 value = l_seriesScores.get(i);
        if (series != NULL && value != 0) {
            switch (series->getGroup()) {
            case 0:
                basicNum++;
                break;
            case 1:
                if (value > 3000) {
                    themeNum++;
                }
                break;
            case 2:
                setScore += value;
                break;
            }
        }
        if (type == 6 || type == 0) {
            score += value;
        }
    }

    if (basicNum != 0) {
        int pick = cM::rndInt(basicNum);
        int n = 0;
        for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
            if (l_seriesScores.get(i) != 0) {
                dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
                if (series != NULL && series->getGroup() == 0) {
                    if (n == pick) {
                        basicSeries = i;
                        break;
                    }
                    n++;
                }
            }
        }
    }
    if (themeNum != 0) {
        int pick = cM::rndInt(themeNum);
        int n = 0;
        for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
            if (l_seriesScores.get(i) != 0) {
                dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
                if (series != NULL && series->getGroup() == 1) {
                    if (n == pick) {
                        themeSeries = i;
                        break;
                    }
                    n++;
                }
            }
        }
    }

    int partsBonus = l_seriesParts.getBonus();
    if (type == 6 || type == 0) {
        score += partsBonus;
    }

    u32 luckyCount = 0;
    for (u32 i = 0; i < 0x1000; i++) {
        if (l_luckyItems.test(i)) {
            luckyCount++;
        }
    }
    u32 luckyScore = luckyCount * 7777;
    if (type == 6 || type == 0) {
        score += luckyScore;
    }
    if (score < 0) {
        score = 0;
    }

    if (res != NULL) {
        if (basicNum != 0) {
            u32 value = l_seriesScores.get(basicSeries);
            if (value == 30000) {
                res->mFlags |= HR_RESULT_SERIES_COMPLETE;
            } else if (value == 25000) {
                res->mFlags |= HR_RESULT_SERIES_ALMOST;
            } else {
                res->mFlags |= HR_RESULT_SERIES_NEED_WALL_FLOOR;
            }
        }
        if (themeNum != 0) {
            res->mFlags |= HR_RESULT_THEME;
        }
        if (setScore >= 3000) {
            res->mFlags |= HR_RESULT_SET;
        }
        if (setScore > 0) {
            res->mFlags |= HR_RESULT_SET_SERIES;
        }
        if (partsBonus > 0) {
            res->mFlags |= HR_RESULT_COMFY;
        }
        if (fengShuiPoints >= 500) {
            res->mFlags |= HR_RESULT_FENG_SHUI;
        }
        if (colorPoints >= 2000) {
            res->mFlags |= HR_RESULT_COLOR;
        }
        if (colorPoints > 0) {
            res->mFlags |= HR_RESULT_COLOR_MATCH;
        }
        if (genrePoints >= 2000) {
            res->mFlags |= HR_RESULT_COHESIVE;
        }
        if (categoryPoints >= 3000) {
            res->mFlags |= HR_RESULT_COLLECTION;
        }
        if (luckyScore >= 7000) {
            res->mFlags |= HR_RESULT_LUCKY;
        }
        if (messy) {
            res->mFlags |= HR_RESULT_MESSY;
        }
        if (facingWall) {
            res->mFlags |= HR_RESULT_FACING_WALL;
        }
    }

    if (sent != NULL) {
        *sent = FALSE;
    }
    dHomeList_c *homes;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (mode == 1 && player != NULL && letter) {
        if (isTownPlayer(player) && !player->isFlag0(0xD) && !player->isFlag0(PRIVATE_FLAG0_HRA_JOINED)) {
            static u16 kind = HR_MAIL_WELCOME;
            static u16 sender = MAIL_FROM_HRA << 8; // the bytes {MAIL_FROM_HRA, 0}
            static int paper = dItem::ITEM_IDX_ACADEMY_PAPER;
            l_mail.setupSystem(&kind, "MAIL_ETC_Happyroom", (const u8 *)&sender, &player->mPID, &paper);
            if (dPostOffice::deliverToPlayer(&l_mail)) {
                player->setFlag0(PRIVATE_FLAG0_HRA_JOINED);
            }
            dTime_c now = *dTime_c::getCurrent();
            if (now.hour < 6) {
                now.add(-1, 0, 0, 0);
            }
            dYMD_c date;
            date.set(&now);
            player->mHRAInfo.mRatingDate = date;
        }
    }

    if (mode == 2 || (mode == 1 && player != NULL && isTownPlayer(player) && player->isFlag0(PRIVATE_FLAG0_HRA_JOINED) &&
                      player->mHRAInfo.isNewRatingWeek(NULL))) {
        if (sent != NULL) {
            *sent = TRUE;
        }
        u8 send = TRUE;
        BOOL weekly = TRUE;
        if (mode == 1) {
            send = FALSE;
            if (player != NULL && player->isFlag0(PRIVATE_FLAG0_HRA_RATED) && player->isFlag0(PRIVATE_FLAG0_HRA_JOINED) && !player->isFlag0(PRIVATE_FLAG0_HRA_NO_LETTER)) {
                send = TRUE;
            }
            weekly = FALSE;
            if (player != NULL && player->isFlag0(PRIVATE_FLAG0_HRA_RATED) && player->isFlag0(PRIVATE_FLAG0_HRA_JOINED)) {
                weekly = TRUE;
            }
        }
        homes = &dSaveData_c::getTown()->mHomes;
        u32 size = homes->getHome(homes->findCurrentPlayer())->mSize;
        int msg = 0;
        u32 points = 0;
        if (basicNum != 0) {
            u32 value = l_seriesScores.get(basicSeries);
            if (value == 30000) {
                points |= HR_RESULT_SERIES_COMPLETE;
            } else if (value == 25000) {
                points |= HR_RESULT_SERIES_ALMOST;
            } else {
                points |= HR_RESULT_SERIES_NEED_WALL_FLOOR;
            }
        }
        if (themeNum != 0) {
            points |= HR_RESULT_THEME;
        }
        if (setScore >= 5000) {
            points |= HR_RESULT_SET;
        }
        if (partsBonus > 0) {
            points |= HR_RESULT_COMFY;
        }
        if (fengShuiPoints >= 2000) {
            points |= HR_RESULT_FENG_SHUI;
        }
        if (colorPoints >= 6000) {
            points |= HR_RESULT_COLOR;
        }
        if (genrePoints >= 6000) {
            points |= HR_RESULT_COHESIVE;
        }
        if (categoryPoints >= 3200) {
            points |= HR_RESULT_COLLECTION;
        }
        if (luckyScore >= 7000) {
            points |= HR_RESULT_LUCKY;
        }
        if (facingWall) {
            points |= HR_RESULT_FACING_WALL;
        }
        if (messy) {
            points |= HR_RESULT_MESSY;
        }

        int numPoints = 0;
        for (int k = 0; k < HR_MAIL_POINT_NUM; k++) {
            if (points & (1 << k)) {
                numPoints++;
            }
        }
        BOOL general;
        if (numPoints != 0) {
            general = cM::rndInt(numPoints + 1) == 0;
        } else {
            general = TRUE;
        }
        if (general) {
            if (score == 0) {
                msg = HR_MAIL_SCORE_ZERO;
            } else if (score <= 19999) {
                msg = HR_MAIL_SIZE_0 + size;
            } else if (score <= 69999) {
                msg = HR_MAIL_SCORE_LOW;
            } else if (score <= 99999) {
                msg = HR_MAIL_SCORE_MID;
            } else {
                msg = HR_MAIL_SCORE_HIGH;
            }
        } else {
            u32 pick = cM::rndInt(numPoints);
            u32 n = 0;
            for (int k = 0; k < HR_MAIL_POINT_NUM; k++) {
                if (points & (1 << k)) {
                    if (pick == n) {
                        msg = HR_MAIL_SERIES_COMPLETE + k;
                        break;
                    }
                    n++;
                }
            }
        }

        int remark = msg;
        u32 series;
        if ((u32)(msg - HR_MAIL_SERIES_COMPLETE) <= HR_MAIL_SERIES_NEED_WALL_FLOOR - HR_MAIL_SERIES_COMPLETE) {
            series = basicSeries;
            if (dHR::isExtraSeries(basicSeries)) {
                msg -= HR_MAIL_SERIES_COMPLETE - HR_MAIL_EXTRA_SERIES_COMPLETE;
            }
        } else {
            series = themeSeries;
            if (msg == HR_MAIL_THEME_COMPLETE && dHR::isExtraSeries(themeSeries)) {
                msg = HR_MAIL_EXTRA_THEME_COMPLETE;
            }
        }
        if (dHR::isExtraSeries(series)) {
            series = 0;
        }

        dYMD_c newDate;
        BOOL isNew = FALSE;
        if (player != NULL && player->mHRAInfo.isNewModelRoomMonth(&newDate)) {
            isNew = TRUE;
        }
        BOOL mailed = FALSE;
        if (weekly) {
            if (isNew && dHR::sendRatingLetter(area, HR_MAIL_MODEL_ROOM_THEME, score, series, res->mCategory, 0) && player != NULL) {
                mailed = TRUE;
                player->mHRAInfo.mModelRoomDate = newDate;
            }
            if (dHR::sendModelPresent(area, score)) {
                mailed = TRUE;
            }
        }
        if (send && !mailed) {
            dHR::sendRatingLetter(area, msg, score, series, res->mCategory, 0);
        }

        dTime_c now = *dTime_c::getCurrent();
        if (now.hour < 6) {
            now.add(-1, 0, 0, 0);
        }
        dYMD_c date;
        date.set(&now);
        if (player != NULL) {
            player->mHRAInfo.mRatingDate = date;
        }
        if (player != NULL && player->isFlag0(PRIVATE_FLAG0_HRA_RATED)) {
            player->mHRAInfo.mLastScore = score;
            player->mHRAInfo.mLastRemark = remark - HR_MAIL_SCORE_ZERO;
            player->mHRAInfo.mLastSeries = series;
            player->setFlag0(PRIVATE_FLAG0_HRA_HAS_RESULT);
        }
    }
    return score;
}




// 800AE70C
u32 dHR::checkBasicSeries(dHR::area_c *area, dHR::result_c *result, dFdBase_c *field, u32 *maxParts) {
    memset(l_basicParts, 0, sizeof(l_basicParts));
    u32 max = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->getKind() == dItem::KIND_FTR &&
                    item->getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
                    int series = item->getSeries();
                    int idx = dItem::seeker_c::get()->findInSeries(*item);
                    l_basicParts[series] |= 1 << idx;
                }
            }
        }
    }
    if (area->mWallpaper.getKind() == dItem::KIND_WALL && area->mWallpaper.getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
        l_basicParts[area->mWallpaper.getSeries()] |= 0x400;
    }
    if (area->mCarpet.getKind() == dItem::KIND_CARPET && area->mCarpet.getSeriesGroup() == dItem::SERIES_GROUP_BASIC) {
        l_basicParts[area->mCarpet.getSeries()] |= 0x800;
    }

    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
        if (series != NULL && series->getGroup() == dItem::SERIES_GROUP_BASIC) {
            int parts = l_basicParts[i];
            u32 n = 0;
            for (int b = 0; b < 12; b++) {
                if ((parts >> b) & 1) {
                    n++;
                }
            }
            if (n > max) {
                max = n;
            }
        }
    }
    if (maxParts != NULL) {
        *maxParts = max;
    }

    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
        if (series != NULL && series->getGroup() == dItem::SERIES_GROUP_BASIC && l_basicParts[i] == 0xFFF &&
            l_seriesScores.raise(i, 30000)) {
            if (result != NULL) {
                result->mBasicSeries = i;
            }
            return i;
        }
    }
    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
        if (series != NULL && series->getGroup() == dItem::SERIES_GROUP_BASIC) {
            u16 parts = l_basicParts[i];
            if ((parts & 0x3FF) == 0x3FF && ((parts & 0x400) || (parts & 0x800)) && l_seriesScores.raise(i, 25000)) {
                if (result != NULL) {
                    result->mBasicSeries2 = i;
                }
                return dItem::SERIES_COUNT;
            }
        }
    }
    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(i);
        if (series != NULL && series->getGroup() == dItem::SERIES_GROUP_BASIC && l_basicParts[i] == 0x3FF &&
            l_seriesScores.raise(i, 20000)) {
            if (result != NULL) {
                result->mBasicSeries3 = i;
            }
            return dItem::SERIES_COUNT;
        }
    }
    return dItem::SERIES_COUNT;
}

// 800AEBC8
BOOL dHR::checkThemeSeries(dHR::area_c *area, dHR::result_c *result, dFdBase_c *field) {
    memset(l_themeParts, 0, sizeof(l_themeParts));
    int wallSeries = area->mWallpaper.getSeries();
    int carpetSeries = area->mCarpet.getSeries();
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->getKind() == dItem::KIND_FTR &&
                    item->getSeriesGroup() == dItem::SERIES_GROUP_THEME) {
                    int series = item->getSeries();
                    int idx = dItem::seeker_c::get()->findInSeries(*item);
                    l_themeParts[series] |= 1 << idx;
                }
            }
        }
    }

    BOOL complete = FALSE;
    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        int id = i;
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(id);
        if (series == NULL || series->getGroup() != dItem::SERIES_GROUP_THEME) {
            continue;
        }
        if (!dHR::isExtraSeries(id)) {
            u16 all = 0;
            u8 n = dItem::seeker_c::get()->searchSeries(id, dItem::KIND_FTR);
            if (n == 0) {
                continue;
            }
            for (u32 b = 0; b < n; b++) {
                all |= 1 << b;
            }
            if (l_themeParts[id] >= all) {
                if (id == wallSeries && id == carpetSeries) {
                    l_seriesScores.raise(id, (n + 1) * 3000);
                    complete = TRUE;
                    if (result != NULL) {
                        result->mThemeSeries = id;
                    }
                }
            } else if (id == wallSeries && id == carpetSeries) {
                l_seriesScores.raise(id, 3000);
                if (result != NULL) {
                    result->mThemePartSeries = id;
                    result->mFlags |= HR_RESULT_THEME_WALL_FLOOR;
                }
            }
        } else {
            int parts = l_themeParts[id];
            u32 n = 0;
            for (int b = 0; b < 16; b++) {
                if ((parts >> b) & 1) {
                    n++;
                }
            }
            if (n >= 6) {
                if (id == wallSeries && id == carpetSeries) {
                    l_seriesScores.raise(id, (n + 1) * 3000);
                    complete = TRUE;
                    if (result != NULL) {
                        result->mThemeSeries = id;
                    }
                }
            } else if (id == wallSeries && id == carpetSeries) {
                l_seriesScores.raise(id, 3000);
                if (result != NULL) {
                    result->mThemePartSeries = id;
                    result->mFlags |= HR_RESULT_THEME_WALL_FLOOR;
                }
            }
        }
    }
    return complete;
}

// 800AF038
BOOL dHR::checkSetSeries(dHR::area_c *area, dHR::result_c *result, dFdBase_c *field) {
    memset(l_setParts, 0, sizeof(l_setParts));
    BOOL complete = FALSE;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && isFurnitureKind(item->getKind()) &&
                    item->getSeriesGroup() == dItem::SERIES_GROUP_SET) {
                    int series = item->getSeries();
                    int idx = dItem::seeker_c::get()->findInSeries(*item);
                    l_setParts[series] |= 1 << idx;
                }
            }
        }
    }

    for (u32 i = 0; i < dItem::SERIES_COUNT; i++) {
        int id = i;
        dItem::Series *series = dItem::infoBank_c::get()->getSeries(id);
        if (series == NULL || series->getGroup() != dItem::SERIES_GROUP_SET) {
            continue;
        }
        if (!dHR::isExtraSeries(id)) {
            u16 all = 0;
            u8 n = dItem::seeker_c::get()->searchSeries(id, dItem::KIND_FTR);
            if (n == 0) {
                n = dItem::seeker_c::get()->searchSeries(id, dItem::KIND_FOSSIL);
            }
            if (n == 0) {
                continue;
            }
            for (u32 b = 0; b < n; b++) {
                all |= 1 << b;
            }
            if (l_setParts[id] >= all) {
                complete = TRUE;
                l_seriesScores.raise(id, n * 1000);
                if (result != NULL) {
                    result->mSetSeries = id;
                }
            }
        } else {
            int parts = l_setParts[id];
            u32 n = 0;
            for (int b = 0; b < 16; b++) {
                if ((parts >> b) & 1) {
                    n++;
                }
            }
            if (n >= 3) {
                complete = TRUE;
                l_seriesScores.raise(id, n * 1000);
                if (result != NULL) {
                    result->mSetSeries = id;
                }
            }
        }
    }
    return complete;
}

// 800AF3F4
void dHR::collectParts(const dHR::area_c *area, dFdBase_c *field) {
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->getKind() == dItem::KIND_FTR) {
                    l_seriesParts.add(*item);
                }
            }
        }
    }
}

// 800AF4AC
int dHR::calcFromPoints(const dHR::area_c *area, dFdBase_c *field) {
    int points = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && dItem::clampField(bitm->m_ftrFunc, 0x41, 1) != 0) {
                        points += bitm->getFromNameIndex();
                    }
                }
            }
        }
    }
    return points;
}

// 800AF58C
int dHR::calcFengShuiPoints(const dHR::area_c *area, const dHR::fnShui_c *fnShui) {
    return (fnShui->mScore[0] + fnShui->mScore[1] + fnShui->mScore[2]) * 100;
}

// 800AF5A8
u32 dHR::calcColorPoints(const dHR::area_c *area, dFdBase_c *field) {
    u8 colors[15];
    memset(colors, 0, sizeof(colors));
    dHR::ftrSet_c set;
    u32 count = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && isFurnitureKind(item->getKind())) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL) {
                        dHR::addFtr(&set, *item);
                        u32 a = bitm->m_ftrColorA;
                        int colorA = a < 15 ? bitm->resolveColor(a, FALSE) : bitm->resolveColor(0, TRUE);
                        int b = (s8)bitm->m_ftrColorB;
                        int colorB = static_cast<u32>(b) < 15 ? bitm->resolveColor(b, FALSE) : bitm->resolveColor(0, TRUE);
                        colors[colorA]++;
                        count++;
                        colors[colorB]++;
                    }
                }
            }
        }
    }
    if (count >= 8) {
        u32 n = set.count();
        for (u32 i = 0; i < 15; i++) {
            if (i != 0) {
                f32 ratio = (f32)colors[i] / (f32)(count * 2);
                if (ratio >= 0.9f) {
                    return n * 600;
                }
                if (ratio >= 0.7f) {
                    return n * 200;
                }
            }
        }
    }
    return 0;
}

// 800AF9A0
void dHR::addFtr(dHR::ftrSet_c *set, dItem::Item item) {
    if (item.hasFtrFunc()) {
        u16 base = dItem::infoBank_c::get()->getBaseIdFromItemId(item.mId);
        if (base < 0x1000) {
            set->mBits[base >> 3] |= 1 << (base & 7);
        }
    }
}

// 800AFA18
u32 dHR::calcGenrePoints(const dHR::area_c *area, dFdBase_c *field, u32 *newOld, u32 *adultKiddy, int theme) {
    u8 newOldCount[3];
    memset(newOldCount, 0, sizeof(newOldCount));
    u8 adultKiddyCount[3];
    memset(adultKiddyCount, 0, sizeof(adultKiddyCount));
    dHR::ftrSet_c set;
    u32 points = 0;
    set.clear();

    u32 count = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && isFurnitureKind(bitm->getKind())) {
                        dHR::addFtr(&set, *item);
                        newOldCount[bitm->getNewOld()]++;
                        count++;
                    }
                }
            }
        }
    }
    if (count >= 8) {
        u32 n = set.count();
        for (u32 i = 0; i < 3; i++) {
            if (i != 0 && (theme == 6 || theme == 0 || (i == 1 && theme == 4) || (i == 2 && theme == 5))) {
                f32 ratio = (f32)newOldCount[i] / (f32)count;
                if (ratio >= 0.9f) {
                    *newOld = i;
                    points += n * 300;
                } else if (ratio >= 0.7f) {
                    *newOld = i;
                    points += n * 100;
                }
            }
        }
    }

    set.clear();
    count = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && isFurnitureKind(bitm->getKind())) {
                        dHR::addFtr(&set, *item);
                        adultKiddyCount[bitm->getAdultKiddy()]++;
                        count++;
                    }
                }
            }
        }
    }
    if (count >= 8) {
        u32 n = set.count();
        for (u32 i = 0; i < 3; i++) {
            if (i != 0 && (theme == 6 || theme == 0 || (i == 1 && theme == 2) || (i == 2 && theme == 3))) {
                f32 ratio = (f32)adultKiddyCount[i] / (f32)count;
                if (ratio >= 0.9f) {
                    *adultKiddy = i;
                    points += n * 300;
                } else if (ratio >= 0.7f) {
                    *adultKiddy = i;
                    points += n * 100;
                }
            }
        }
    }
    return points;
}

// 800B003C
u32 dHR::calcCategoryPoints(const dHR::area_c *area, dHR::result_c *result, dFdBase_c *field, u32 *category) {
    u16 counts[dItem::FTR_PART_B_COUNT];
    memset(counts, 0, sizeof(counts));
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->hasFtrFunc()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL) {
                        counts[dItem::clampField((s8)bitm->m_ftrPartB, dItem::FTR_PART_B_COUNT, dItem::FTR_PART_B_ART)]++;
                    }
                }
            }
        }
    }
    u32 points = 0;
    for (u32 i = 0; i < dItem::FTR_PART_B_COUNT; i++) {
        if (i != dItem::FTR_PART_B_NONE && counts[i] >= 8) {
            *category = i;
            points += counts[i] * 400;
            if (result != NULL) {
                switch (i) {
                case dItem::FTR_PART_B_INSTRUMENT:
                    result->mFlags |= HR_RESULT_MANY_INSTRUMENTS;
                    break;
                case dItem::FTR_PART_B_ART:
                    result->mFlags |= HR_RESULT_MANY_ART;
                    break;
                case dItem::FTR_PART_B_MODEL:
                    result->mFlags |= HR_RESULT_MANY_MODELS;
                    break;
                case dItem::FTR_PART_B_DOLL:
                    result->mFlags |= HR_RESULT_MANY_DOLLS;
                    break;
                case dItem::FTR_PART_B_PLANT:
                    result->mFlags |= HR_RESULT_MANY_PLANTS;
                    break;
                }
            }
        }
    }
    return points;
}

// 800B0204
int dHR::calcWallPoints(const dHR::area_c *area, dHR::result_c *result, dFdBase_c *field, u8 *wall, u8 *real) {
    u32 dir;
    int points = 0;
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL && item->hasFtrFunc()) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && bitm->m_hraWallCheck) {
                        dir = item->mId & 3;
                        if (x == area->mMinX && dir == 3) {
                            *wall = 1;
                            points += 100;
                            if (result != NULL) {
                                result->setWallItem(*item);
                            }
                        }
                        if (x == area->mMaxX && dir == 1) {
                            *wall = 1;
                            points += 100;
                            if (result != NULL) {
                                result->setWallItem(*item);
                            }
                        }
                        if (z == area->mMinZ && dir == 2) {
                            *wall = 1;
                            points += 100;
                            if (result != NULL) {
                                result->setWallItem(*item);
                            }
                        }
                        if (z == area->mMaxZ && dir == 0) {
                            *wall = 1;
                            points += 100;
                            if (result != NULL) {
                                result->setWallItem(*item);
                            }
                        }
                    }
                } else if (item != NULL && dItem::isRealItemId(item->mId)) {
                    *real = 1;
                    points += 1;
                    if (result != NULL) {
                        result->setRealItem(*item);
                    }
                }
            }
        }
    }
    return points;
}

// 800B0424
void dHR::collectLucky(const dHR::area_c *area, dFdBase_c *field) {
    for (int layer = 0; layer < 2; layer++) {
        for (u32 z = area->mMinZ; z <= area->mMaxZ; z++) {
            for (u32 x = area->mMinX; x <= area->mMaxX; x++) {
                dItem::Item *item = field->getItem(x, z, layer);
                if (item != NULL) {
                    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                    if (bitm != NULL && bitm->m_hraLuckyBonus) {
                        dHR::addFtr(&l_luckyItems, *item);
                    }
                }
            }
        }
    }
}

// The literals become anonymous .sdata temporaries.
static inline void setupMail(const u16 &kind, const u8 &sender, const dPersonalID_c *to, const int &paper) {
    l_mail.setupSystem(&kind, "MAIL_ETC_Happyroom", &sender, to, &paper);
}

// 800B04F8
BOOL dHR::sendRatingLetter(const dHR::area_c *area, int kind, int points, u32 series, u32 category, BOOL hold) {
    fn_800CBF34(0, points, 7, 9);
    dItem::nameSeries_c name;
    if (!dHR::isExtraSeries(series)) {
        setFurnitureName(&name, series);
    }
    if (kind == HR_MAIL_MODEL_ROOM_THEME) {
        fn_800CBD30(0, getModelRoomTheme() + HR_THEME_UNIT_MSG, "sys_STRING/STR_Unit");
    } else {
        fn_800CBCD4(0, &name);
    }
    fn_800CBCD4(1, &name);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL && player->mPID.isValid()) {
        l_mail.clear();
        setupMail(kind, 2, &player->mPID, 0x15B);
        if (hold) {
            if (dPostOffice::add(&l_mail)) {
                return TRUE;
            }
        } else {
            if (dPostOffice::deliverToPlayer(&l_mail)) {
                return TRUE;
            }
            if (dPostOffice::add(&l_mail)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 800B0698
BOOL dHR::sendModelPresent(const dHR::area_c *area, int points) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        int kind = -1;
        u32 flag = 0x2A;
        u16 present = dItem::ITEM_ID_NONE;
        if (!player->isFlag0(PRIVATE_FLAG0_HRA_HOUSE_MODEL)) {
            if (points >= 70000) {
                kind = HR_MAIL_PRESENT_70000;
                flag = 0x2A;
                present = dItem::Item(dItem::ITEM_IDX_HOUSE_MODEL).mId;
            }
        } else if (!player->isFlag0(PRIVATE_FLAG0_HRA_WIDE_HOUSE_MODEL)) {
            if (points >= 100000) {
                kind = HR_MAIL_PRESENT_100000;
                flag = 0x2B;
                present = dItem::Item(dItem::ITEM_IDX_WIDE_HOUSE_MODEL).mId;
            }
        } else if (!player->isFlag0(PRIVATE_FLAG0_HRA_TWO_STORY_MODEL)) {
            if (points >= 150000) {
                kind = HR_MAIL_PRESENT_150000;
                flag = 0x2C;
                present = dItem::Item(dItem::ITEM_IDX_TWO_STORY_MODEL).mId;
            }
        }
        if (kind != -1) {
            fn_800CBF34(0, points, 7, 9);
            l_mail.clear();
            setupMail(kind, 2, &player->mPID, 0x15B);
            l_mail.setPresent(present, 0xFF);
            if (dPostOffice::deliverToPlayer(&l_mail)) {
                player->setFlag0(flag);
                return TRUE;
            }
            if (dPostOffice::add(&l_mail)) {
                player->setFlag0(flag);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 800B0844
BOOL dHR::calcBounds(int *minX, int *maxX, int *minZ, int *maxZ, dFdBase_c *field) {
    *minZ = 16;
    *minX = 16;
    *maxZ = -16;
    *maxX = -16;
    BOOL hasMinX = FALSE;
    BOOL hasMaxX = FALSE;
    BOOL hasMinZ = FALSE;
    BOOL hasMaxZ = FALSE;
    for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
            if (field->canPutItem(x, z)) {
                if (x <= *minX) {
                    *minX = x;
                    hasMinX = TRUE;
                }
                if (x >= *maxX) {
                    *maxX = x;
                    hasMaxX = TRUE;
                }
                if (z <= *minZ) {
                    *minZ = z;
                    hasMinZ = TRUE;
                }
                if (z >= *maxZ) {
                    *maxZ = z;
                    hasMaxZ = TRUE;
                }
            }
        }
    }
    return (hasMaxZ & hasMaxX & (hasMinZ & hasMinX)) != 0;
}

// 800B094C
BOOL fn_800B094C() {
    return FALSE;
}

// 800B0954
void dHR::rateCurrentPlayer(bool letter, int mode) {
    int home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
    if (home != -1) {
        dHR::area_c area;
        u8 done;
        dHR::ratePlayer(&area, home, mode, 6, 0, 3, &done, letter);
        if (done) {
            dHR::selectAllModelRooms();
        }
    }
}

// 800B09E4
int dHR::rateAnimalHome(int animalIdx, dHR::result_c *result, int type) {
    dHR::area_c area;
    return dHR::rateAnimal(&area, animalIdx, result, type);
}

// 800B0A28
int dHR::rateModelRoomResult(dHR::result_c *result, dSvMdlRm_c *room, int type) {
    dHR::area_c area;
    dHR::result_c def;
    if (result == NULL) {
        result = &def;
    }
    return dHR::rateModelRoom(&area, result, room, type);
}

// 800B0AB0
void dHR::selectAllModelRooms() {
    dHR::selectModelRooms(1, 1);
}



// 800B0ABC: picks this month's model rooms. room (kept, shown at the HRA) is re-rated for a new theme
// (or dropped if it is from this town), other (the candidate) starts empty; then every room of the
// players' houses and every villager's house is rated and kept where it scores better. No owner and
// no score: an empty concrete room.
void dHR::selectModelRooms(BOOL players, BOOL animals) {
    dSvMdlRm_c *room;
    dSvMdlRm_c *other;
    int theme = getModelRoomTheme();
    room = &dSaveData_c::getTown()->mModelRoom;
    other = &dSaveData_c::getTown()->_0640C8.mModelRoomCandidate;
    other->clear();
    if (room->isFromThisTown()) {
        room->clear();
    } else if (room->hasOwner() && theme != room->mTheme) {
        room->mScore = dHR::rateModelRoomResult(NULL, room, theme);
        room->mTheme = theme;
    }
    if (players) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player == NULL || !player->mPID.isValid() || !player->isFlag0(PRIVATE_FLAG0_HRA_RATED)) {
                continue;
            }
            int home = dSaveData_c::getTown()->mHomes.findOwner(player);
            if (home == -1) {
                continue;
            }
            for (u32 r = 0; r < HOME_ROOM_NUM; r++) {
                BOOL thisTown = room->isFromThisTown();
                dHR::area_c area;
                int score = dHR::ratePlayer(&area, home, 0, theme, r, 1, NULL, FALSE);
                if (!room->hasOwner() || (thisTown && score >= room->mScore) || (!thisTown && score > room->mScore)) {
                    if (room->setFromHome(home, r)) {
                        room->mScore = score;
                        room->mTheme = theme;
                    }
                }
                if (!other->hasOwner() || score >= other->mScore) {
                    if (other->setFromHome(home, r)) {
                        other->mScore = score;
                        other->mTheme = theme;
                    }
                }
            }
        }
    }
    if (animals) {
        for (u32 i = 0; i < ANIMAL_NUM; i++) {
            dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
            if (animal == NULL || !animal->mID.isValid()) {
                continue;
            }
            dHR::result_c result;
            int score = dHR::rateAnimalHome(i, &result, theme);
            if (score > room->mScore && room->setFromAnimal(i)) {
                room->mScore = score;
                room->mTheme = theme;
            }
            if (!other->hasOwner() || score > other->mScore) {
                if (other->setFromAnimal(i)) {
                    other->mScore = score;
                    other->mTheme = theme;
                }
            }
        }
    }
    if (room->mScore == 0) {
        room->clear();
        room->mFlags |= MODEL_ROOM_OWNED;
        room->setWallpaper(dItem::Item(dItem::ITEM_IDX_CONCRETE_WALL));
        room->setCarpet(dItem::Item(dItem::ITEM_IDX_CONCRETE_FLOOR));
        room->mRoomType = 0;
        room->mTheme = theme;
    }
    if (other->hasOwner()) {
        dSaveData_c::getTown()->_0640C8.setModelRoom(*other);
    }
}

// 800B12F4: a model room received over WiiConnect24 (d_wifi): rated for this month's theme, it replaces
// the kept one if it scores better (from this town: at least as well; ties also win against a villager's room).
void dHR::receiveModelRoom(dSvMdlRm_c *src) {
    dSvMdlRm_c *room;
    int theme = getModelRoomTheme();
    fn_800B4690(src);
    static dSvMdlRm_c sRoom;
    sRoom = *src;
    room = &dSaveData_c::getTown()->mModelRoom;
    if (room->hasOwner() && theme != room->mTheme) {
        room->mScore = dHR::rateModelRoomResult(NULL, room, theme);
        room->mTheme = theme;
    }
    if (sRoom.hasOwner() && !sRoom.hasInvalidItem()) {
        sRoom.mScore = dHR::rateModelRoomResult(NULL, &sRoom, theme);
        sRoom.mTheme = theme;
        if (room->hasOwner()) {
            if (room->isFromThisTown()) {
                if (sRoom.mScore >= room->mScore) {
                    *room = sRoom;
                }
            } else {
                int newScore = sRoom.mScore;
                int score = room->mScore;
                if ((room->isVillagerOwner() && newScore >= score) || newScore > score) {
                    *room = sRoom;
                }
            }
        } else {
            *room = sRoom;
        }
    }
    if (room->mScore == 0) {
        room->clear();
        room->mFlags |= MODEL_ROOM_OWNED;
        room->setWallpaper(dItem::Item(dItem::ITEM_IDX_CONCRETE_WALL));
        room->setCarpet(dItem::Item(dItem::ITEM_IDX_CONCRETE_FLOOR));
        room->mRoomType = 0;
        room->mTheme = theme;
    }
}


// 800B2480: rates player house home
int dHR::getPlayerRank(int home, u32 *rank) {
    dHR::area_c area;
    return dHR::quickRatePlayer(&area, home, rank);
}

// 800B24BC: rates villager animalIdx's house
int dHR::getAnimalRank(int animalIdx, u32 *rank) {
    dHR::area_c area;
    return dHR::quickRateAnimal(&area, animalIdx, rank);
}

// 800B24F8
dHR::remark_c::remark_c() {
    mSeries = 0;
    mMsg = 0;
}

// 800B2514
void dHR::remark_c::clear() {
    mItem = dItem::Item(dItem::ITEM_IDX_APPLE);
    mSeries = 0;
    mMsg = 0;
}

// 800B255C
dItem::Item dHR::remark_c::getItem() const {
    return !dItem::isRealItemId(mItem.mId) ? dItem::Item(dItem::ITEM_IDX_APPLE) : mItem;
}

// 800B25C4
void dHR::remark_c::setWords(dDemo_c *demo) const {
    if (demo != NULL) {
        if (!dHR::isExtraSeries(mSeries)) {
            u32 series = mSeries;
            dItem::nameSeries_c name;
            setFurnitureName(&name, series);
            demo->setWord(0, &name);
        }
        dItem::Item item = getItem();
        demo->setItemName(1, &item);
    }
}

extern dHR::remarks_c l_remarks;

// 800B2674
dHR::remarks_c *dHR::getRemarks() {
    return &l_remarks;
}

// 800B2680
void dHR::updateRemarks() {
    dHR::makeRemarks(&l_remarks);
}

// 800B268C
void dHR::makeRemarks(dHR::remarks_c *remarks) {
    dHR::remarks_c tmp;
    if (remarks == NULL) {
        remarks = &tmp;
    }
    dHR::makeModelRoomRemarks(&remarks->mTheme, &remarks->mSeries, &remarks->mParts, &remarks->mNotable, &remarks->mExtra);
}

// 805981F4 (its implicit dtor, 800B2700, is generated here)
dHR::remarks_c l_remarks;

// The lucky series (0x12..0x15) among the basic series of a rating, or 0.
static inline int findLuckySeries(const dHR::result_c &info) {
    static const int sLuckySeries[4] = {0x12, 0x15, 0x14, 0x13};

    int series = 0;
    const int *lucky = sLuckySeries;
    for (int i = 0; i < 4; i++, lucky++) {
        if (*lucky == info.mBasicSeries || *lucky == info.mBasicSeries2 || *lucky == info.mBasicSeries3) {
            series = *lucky;
        }
    }
    return series;
}

// 800B2740
void dHR::makeModelRoomRemarks(dHR::remark_c *outTheme, dHR::remark_c *outSeries, dHR::remark_c *outParts, dHR::remark_c *outNotable, dHR::remark_c *outExtra) {
    dHR::remark_c tmp0;
    dHR::remark_c tmp1;
    dHR::remark_c tmp2;
    dHR::remark_c tmp3;
    dHR::remark_c tmp4;
    if (outTheme == NULL) {
        outTheme = &tmp0;
    }
    if (outSeries == NULL) {
        outSeries = &tmp1;
    }
    if (outParts == NULL) {
        outParts = &tmp2;
    }
    if (outNotable == NULL) {
        outNotable = &tmp3;
    }
    if (outExtra == NULL) {
        outExtra = &tmp4;
    }
    outTheme->clear();
    outSeries->clear();
    outParts->clear();
    outNotable->clear();
    outExtra->clear();

    dHR::result_c info;
    dSvMdlRm_c *mdlRm = &dSaveData_c::getTown()->mModelRoom;
    dHomeRoom_c room(*mdlRm);
    int theme = getModelRoomTheme();
    dHR::rateModelRoomResult(&info, mdlRm, 0);

    // outTheme: the theme (getModelRoomTheme: 1 feng shui, 2/3 adult/kiddy, 4/5 new/old); 0x51 none.
    if (theme == 0) {
        outTheme->mMsg = HR_REMARK_THEME_NONE;
    } else if (theme == 1) {
        EGG::Heap *heap = lbl_8074E440;
        dFdInfoSvMdlRm_c *fd = dFdInfoSvMdlRm_c::createWithBg15(mdlRm, heap);
        if (fd != NULL) {
            dHR::fnShui_c fs;
            fs.calc(fd);
            u8 best = fs.mScore[0];
            if (best != 0) {
                outTheme->mMsg = HR_REMARK_FENG_SHUI_YELLOW_WEST;
                outTheme->mItem = fs.mItemL;
            }
            if (fs.mScore[2] > best) {
                outTheme->mMsg = HR_REMARK_FENG_SHUI_RED_EAST;
                best = fs.mScore[2];
                outTheme->mItem = fs.mItemR;
            }
            if (fs.mScore[1] > best) {
                outTheme->mMsg = HR_REMARK_FENG_SHUI_GREEN_SOUTH;
                outTheme->mItem = fs.mItemB;
            }
            if (outTheme->mMsg == 0) {
                outTheme->mMsg = HR_REMARK_THEME_NONE;
            }
            dFdInfoSvMdlRm_c::remove(&fd, heap);
        }
    } else {
        dHR::rateModelRoomResult(NULL, mdlRm, theme);
        dItem::Item item;
        if (theme == 4) {
            searchNew_c search;
            outTheme->mMsg = HR_REMARK_THEME_NEW;
            item = mdlRm->getRandomRoomItem(search);
            outTheme->mItem = item;
        } else if (theme == 5) {
            searchOld_c search;
            outTheme->mMsg = HR_REMARK_THEME_OLD;
            item = mdlRm->getRandomRoomItem(search);
            outTheme->mItem = item;
        } else if (theme == 2) {
            searchAdult_c search;
            outTheme->mMsg = HR_REMARK_THEME_ADULT;
            item = mdlRm->getRandomRoomItem(search);
            outTheme->mItem = item;
        } else {
            searchKiddy_c search;
            outTheme->mMsg = HR_REMARK_THEME_KIDDY;
            item = mdlRm->getRandomRoomItem(search);
            outTheme->mItem = item;
        }
        if (item == dItem::ITEM_ID_NONE) {
            outTheme->mMsg = HR_REMARK_THEME_NONE;
        }
    }

    // outSeries: the series (basic series messages 0x5F..0x68, lucky series 0x6B, series item 0x6C..0x71).
    dItem::Item mainItem = outTheme->getItem();
    BOOL done = FALSE;
    if (info.isFlag(HR_RESULT_SERIES_COMPLETE) || info.isFlag(HR_RESULT_SERIES_ALMOST) || info.isFlag(HR_RESULT_SERIES_NEED_WALL_FLOOR)) {
        switch (info.mBasicSeries) {
        case 0x10:
            done = TRUE;
            outSeries->mMsg = HR_REMARK_SERIES_HALLOWEEN;
            break;
        case 0xE:
            done = TRUE;
            outSeries->mMsg = HR_REMARK_SERIES_JINGLE;
            break;
        case 0xF:
            done = TRUE;
            outSeries->mMsg = HR_REMARK_SERIES_HARVEST;
            break;
        case 0x11:
            done = TRUE;
            outSeries->mMsg = HR_REMARK_SERIES_BUNNY_DAY;
            break;
        }
        if (!done) {
            switch (info.mBasicSeries2) {
            case 0x10:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_HALLOWEEN;
                break;
            case 0xE:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_JINGLE;
                break;
            case 0xF:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_HARVEST;
                break;
            case 0x11:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_BUNNY_DAY;
                break;
            }
        }
        if (!done) {
            switch (info.mBasicSeries3) {
            case 0x10:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_HALLOWEEN;
                break;
            case 0xE:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_JINGLE;
                break;
            case 0xF:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_HARVEST;
                break;
            case 0x11:
                done = TRUE;
                outSeries->mMsg = HR_REMARK_SERIES_BUNNY_DAY;
                break;
            }
        }
        if (!done) {
            int series = findLuckySeries(info);
            if (series != 0) {
                dHR::searchSeries_c search(&mainItem, 1, series);
                dItem::Item item = mdlRm->getRandomRoomItem(search);
                if (item != dItem::ITEM_ID_NONE) {
                    outSeries->mMsg = HR_REMARK_SERIES_GRACIE;
                    outSeries->mItem = item;
                }
            } else if (info.isFlag(HR_RESULT_SERIES_COMPLETE)) {
                if (!dHR::isExtraSeries(info.mBasicSeries)) {
                    dHR::searchSeries_c search(&mainItem, 1, info.mBasicSeries);
                    dItem::Item item = mdlRm->getRandomRoomItem(search);
                    if (item != dItem::ITEM_ID_NONE) {
                        outSeries->mMsg = HR_REMARK_SERIES_COMPLETE;
                        outSeries->mItem = item;
                        outSeries->mSeries = info.mBasicSeries;
                    }
                }
            } else if (info.isFlag(HR_RESULT_SERIES_ALMOST)) {
                if (!dHR::isExtraSeries(info.mBasicSeries2)) {
                    dHR::searchSeries_c search(&mainItem, 1, info.mBasicSeries2);
                    dItem::Item item = mdlRm->getRandomRoomItem(search);
                    if (item != dItem::ITEM_ID_NONE) {
                        outSeries->mMsg = HR_REMARK_SERIES_ALMOST;
                        outSeries->mItem = item;
                        outSeries->mSeries = info.mBasicSeries2;
                    }
                }
            } else if (info.isFlag(HR_RESULT_SERIES_NEED_WALL_FLOOR)) {
                if (!dHR::isExtraSeries(info.mBasicSeries3)) {
                    dHR::searchSeries_c search(&mainItem, 1, info.mBasicSeries3);
                    dItem::Item item = mdlRm->getRandomRoomItem(search);
                    if (item != dItem::ITEM_ID_NONE) {
                        outSeries->mMsg = HR_REMARK_SERIES_NEED_WALL_FLOOR;
                        outSeries->mItem = item;
                        outSeries->mSeries = info.mBasicSeries3;
                    }
                }
            }
        }
    } else if (info.isFlag(HR_RESULT_THEME)) {
        if (!dHR::isExtraSeries(info.mThemeSeries)) {
            outSeries->mMsg = HR_REMARK_THEME_SERIES;
            outSeries->mSeries = info.mThemeSeries;
        }
    } else if (info.isFlag(HR_RESULT_LUCKY)) {
        dHR::searchLucky_c search(&mainItem, 1);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            outSeries->mMsg = HR_REMARK_LUCKY_ITEM;
            outSeries->mItem = item;
        }
    } else if (info.isFlag(HR_RESULT_SET_SERIES)) {
        dItem::Item exclude = outTheme->getItem();
        dHR::searchSeries_c search(&exclude, 1, info.mSetSeries);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            outSeries->mMsg = HR_REMARK_SET_SERIES;
            outSeries->mItem = item;
        }
    }

    // outParts: a random one of the furniture part remarks (0x79..0x80).
    u16 msgs2[64];
    u16 items2[64];
    int num = 0;
    if (info.isFlag(HR_RESULT_MANY_INSTRUMENTS)) {
        dHR::searchCat_c search(dItem::FTR_PART_B_INSTRUMENT);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        msgs2[num] = HR_REMARK_MANY_INSTRUMENTS;
        items2[num] = item.mId;
        num++;
    }
    if (info.isFlag(HR_RESULT_MANY_PLANTS)) {
        dHR::searchCat_c search(dItem::FTR_PART_B_PLANT);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        msgs2[num] = HR_REMARK_MANY_PLANTS;
        items2[num] = item.mId;
        num++;
    }
    if (info.isFlag(HR_RESULT_MANY_ART)) {
        dHR::searchCat_c search(dItem::FTR_PART_B_ART);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        msgs2[num] = HR_REMARK_MANY_ART;
        items2[num] = item.mId;
        num++;
    }
    if (info.isFlag(HR_RESULT_MANY_MODELS)) {
        dHR::searchCat_c search(dItem::FTR_PART_B_MODEL);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        msgs2[num] = HR_REMARK_MANY_MODELS;
        items2[num] = item.mId;
        num++;
    }
    if (info.isFlag(HR_RESULT_MANY_DOLLS)) {
        dHR::searchCat_c search(dItem::FTR_PART_B_DOLL);
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        msgs2[num] = HR_REMARK_MANY_DOLLS;
        items2[num] = item.mId;
        num++;
    }
    if (info.isFlag(HR_RESULT_COLOR_MATCH)) {
        msgs2[num] = HR_REMARK_COLOR_MATCH;
        items2[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (info.isFlag(HR_RESULT_THEME_WALL_FLOOR) && !dHR::isExtraSeries(info.mThemePartSeries)) {
        outParts->mSeries = info.mThemePartSeries;
        msgs2[num] = HR_REMARK_THEME_WALL_FLOOR;
        items2[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (info.isFlag(HR_RESULT_COMFY)) {
        msgs2[num] = HR_REMARK_COMFY;
        items2[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (num != 0) {
        int i = cM::rndInt(num);
        outParts->mMsg = msgs2[i];
        outParts->mItem = items2[i];
    }

    // outNotable: a random one of the notable items / collections in the room (0x91..0xC0).
    u16 msgs3[64];
    u16 items3[64];
    num = 0;
    bool found0 = false;
    bool found1 = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    bool found5 = false;
    bool found6 = false;
    bool found7 = false;
    bool found8 = false;
    bool found9 = false;
    bool found10 = false;
    bool found11 = false;
    u32 numGyroid = 0;
    u32 numFossil = 0;
    u32 numCat5 = 0;
    u32 numPartB1 = 0;
    u32 numCat6 = 0;
    u32 numCat4 = 0;
    u32 numCat7 = 0;
    u32 numCat8 = 0;
    u32 numCat1 = 0;
    u32 numCat9 = 0;
    u32 numPartA2 = 0;
    u32 numPartA1 = 0;
    u32 numCreature = 0;
    u16 gyroid = dItem::ITEM_ID_NONE;
    dItem::Item birthdayCake(dItem::ITEM_IDX_BIRTHDAY_CAKE);
    dItem::Item chocolateHeart(dItem::ITEM_IDX_CHOCOLATE_HEART);
    dItem::Item festiveTree(dItem::ITEM_IDX_FESTIVE_TREE);
    dItem::Item bigFestiveTree(dItem::ITEM_IDX_BIG_FESTIVE_TREE);
    dItem::Item snowman(dItem::ITEM_IDX_SNOWMAN);
    dItem::Item portrait(dItem::ITEM_IDX_PORTRAIT);
    for (u32 layer = 0; layer < 2; layer++) {
        dItem::Item *item = room.getLayer(layer)->mItems[0];
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++, item++) {
                if (!dItem::isRealItemId(item->getId())) {
                    continue;
                }
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                if (bitm == NULL) {
                    continue;
                }
                int kind = bitm->getKind();
                int from = dItem::clampField(bitm->m_from, dItem::FROM_COUNT, dItem::FROM_NONE);
                int partB = dItem::clampField(static_cast<s8>(bitm->m_ftrPartB), dItem::FTR_PART_B_COUNT, dItem::FTR_PART_B_ART);
                int cat = getCategoryQ5(*item);
                if (!found0 && item->isSame(snowman) && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_SNOWMAN;
                    items3[num] = item->mId;
                    num++;
                    found0 = true;
                } else if (!found1 && item->isSame(birthdayCake) && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_BIRTHDAY_CAKE;
                    items3[num] = item->mId;
                    num++;
                    found1 = true;
                } else if (!found2 && item->isSame(portrait) && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_PORTRAIT;
                    items3[num] = item->mId;
                    num++;
                    found2 = true;
                } else if (!found3 && (item->isSame(festiveTree) || item->isSame(bigFestiveTree)) && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_FESTIVE_TREE;
                    items3[num] = item->mId;
                    num++;
                    found3 = true;
                } else if (!found4 && item->isSame(chocolateHeart) && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_CHOCOLATE;
                    items3[num] = item->mId;
                    num++;
                    found4 = true;
                } else if (!found5 && from == dItem::FROM_HALLOWEEN && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_HALLOWEEN_ITEM;
                    items3[num] = item->mId;
                    num++;
                    found5 = true;
                } else if (!found6 && from == dItem::FROM_JINGLE && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_JINGLE_ITEM;
                    items3[num] = item->mId;
                    num++;
                    found6 = true;
                } else if (!found7 && from == dItem::FROM_HARVESTFESTIVAL && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_HARVEST_ITEM;
                    items3[num] = item->mId;
                    num++;
                    found7 = true;
                } else if (!found8 && from == dItem::FROM_EASTER && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_BUNNY_DAY_ITEM;
                    items3[num] = item->mId;
                    num++;
                    found8 = true;
                } else if (!found9 && kind == dItem::KIND_CLOTH && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_CLOTHES;
                    items3[num] = item->mId;
                    num++;
                    found9 = true;
                } else if (!found10 && kind == dItem::KIND_FISH && bitm->getFtrSize() == 2 &&
                           item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_AQUARIUM;
                    items3[num] = item->mId;
                    num++;
                    found10 = true;
                } else if (!found11 && item->getSeriesGroup() == 1 && item->isNotSame(mainItem)) {
                    msgs3[num] = HR_REMARK_ODD_ITEM;
                    items3[num] = item->mId;
                    num++;
                    found11 = true;
                }

                if (kind == dItem::KIND_HANIWA) {
                    gyroid = item->mId;
                    numGyroid++;
                } else if (kind == dItem::KIND_FOSSIL && from == dItem::FROM_NICE_FOSSIL) {
                    numFossil++;
                } else if (cat == 6) {
                    numCat6++;
                } else if (cat == 5) {
                    numCat5++;
                } else if (partB == dItem::FTR_PART_B_ART) {
                    numPartB1++;
                } else if (cat == 4) {
                    numCat4++;
                } else if (cat == 1) {
                    numCat1++;
                } else if (bitm->getFtrPartA() == 1) {
                    numPartA1++;
                } else if (bitm->getFtrPartA() == 2) {
                    numPartA2++;
                } else if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH) {
                    numCreature++;
                } else if (cat == 8) {
                    numCat8++;
                } else if (cat == 7) {
                    numCat7++;
                } else if (cat == 9) {
                    numCat9++;
                }
            }
        }
    }
    if (numGyroid == 1) {
        msgs3[num] = HR_REMARK_GYROID;
        items3[num] = gyroid;
        num++;
    } else if (numGyroid > 1) {
        msgs3[num] = HR_REMARK_GYROIDS;
        items3[num] = gyroid;
        num++;
    }
    if (numFossil == 1) {
        msgs3[num] = HR_REMARK_FOSSIL;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    } else if (numFossil > 1) {
        msgs3[num] = HR_REMARK_FOSSILS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat5 >= 3 && numCat5 < 8) {
        msgs3[num] = HR_REMARK_SOME_INSTRUMENTS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat6 >= 3 && numCat6 < 8) {
        msgs3[num] = HR_REMARK_SOME_PLANTS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numPartB1 >= 3 && numPartB1 < 8) {
        msgs3[num] = HR_REMARK_SOME_PAINTINGS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat4 >= 3) {
        msgs3[num] = HR_REMARK_LAMPS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat7 >= 3) {
        msgs3[num] = HR_REMARK_CLOCKS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numPartA1 >= 3) {
        msgs3[num] = HR_REMARK_CHAIRS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numPartA2 >= 3) {
        msgs3[num] = HR_REMARK_DRESSERS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat8 >= 2) {
        msgs3[num] = HR_REMARK_STEREOS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat9 >= 2) {
        msgs3[num] = HR_REMARK_TVS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCat1 >= 2) {
        msgs3[num] = HR_REMARK_BEDS;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (numCreature >= 4) {
        msgs3[num] = HR_REMARK_CREATURES;
        items3[num] = dItem::ITEM_ID_NONE;
        num++;
    }
    if (num != 0) {
        int i = cM::rndInt(num);
        outNotable->mMsg = msgs3[i];
        outNotable->mItem = items3[i];
    }

    // outExtra: a random one of the wall/real item, the theme's opposite item and the room size remark (0x8A..0x8C).
    u16 msgs4[64];
    u16 items4[64];
    num = 0;
    if (info.isFlag(HR_RESULT_MESSY)) {
        bool ok = false;
        bool is97 = outNotable->mMsg == HR_REMARK_CHOCOLATE;
        bool other = !is97;
        if (is97 && info.mRealItem.isNotSame(chocolateHeart)) {
            ok = true;
        }
        if (other || ok) {
            msgs4[num] = HR_REMARK_MESSY_ITEM;
            items4[num] = info.mRealItem.mId;
            num++;
        }
    }
    if (info.isFlag(HR_RESULT_FACING_WALL)) {
        BOOL same = outTheme->getItem().isSame(info.mWallItem.mId);
        if (!same) {
            same = outSeries->getItem().isSame(info.mWallItem.mId);
        }
        if (!same) {
            same = outParts->getItem().isSame(info.mWallItem.mId);
        }
        if (!same) {
            same = outNotable->getItem().isSame(info.mWallItem.mId);
        }
        if (!same) {
            msgs4[num] = HR_REMARK_FACING_WALL;
            items4[num] = info.mWallItem.mId;
            num++;
        }
    }
    if (theme == 4) {
        searchOld_c search;
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            BOOL used = outTheme->getItem().isSame(item) || outSeries->getItem().isSame(item) || outParts->getItem().isSame(item) ||
                        outNotable->getItem().isSame(item);
            if (!used) {
                msgs4[num] = HR_REMARK_MISMATCH;
                items4[num] = item.mId;
                num++;
            }
        }
    } else if (theme == 5) {
        searchNew_c search;
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            BOOL used = outTheme->getItem().isSame(item) || outSeries->getItem().isSame(item) || outParts->getItem().isSame(item) ||
                        outNotable->getItem().isSame(item);
            if (!used) {
                msgs4[num] = HR_REMARK_MISMATCH;
                items4[num] = item.mId;
                num++;
            }
        }
    } else if (theme == 2) {
        searchKiddy_c search;
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            BOOL used = outTheme->getItem().isSame(item) || outSeries->getItem().isSame(item) || outParts->getItem().isSame(item) ||
                        outNotable->getItem().isSame(item);
            if (!used) {
                msgs4[num] = HR_REMARK_MISMATCH;
                items4[num] = item.mId;
                num++;
            }
        }
    } else if (theme == 3) {
        searchAdult_c search;
        dItem::Item item = mdlRm->getRandomRoomItem(search);
        if (item != dItem::ITEM_ID_NONE) {
            BOOL used = outTheme->getItem().isSame(item) || outSeries->getItem().isSame(item) || outParts->getItem().isSame(item) ||
                        outNotable->getItem().isSame(item);
            if (!used) {
                msgs4[num] = HR_REMARK_MISMATCH;
                items4[num] = item.mId;
                num++;
            }
        }
    }

    u32 tiles = mdlRm->countRoomFtrTiles();
    int size;
    switch (mdlRm->mRoomType) {
    case 0:
        if (tiles <= 5) {
            size = 0;
        } else if (tiles < 10) {
            size = 1;
        } else {
            size = 2;
        }
        break;
    case 1:
        if (tiles <= 6) {
            size = 0;
        } else if (tiles < 20) {
            size = 1;
        } else {
            size = 2;
        }
        break;
    default:
        if (tiles <= 8) {
            size = 0;
        } else if (tiles < 36) {
            size = 1;
        } else {
            size = 2;
        }
        break;
    }
    msgs4[num] = HR_REMARK_LAYOUT_SPARSE + size;
    items4[num] = dItem::ITEM_ID_NONE;
    num++;
    if (num != 0) {
        int i = cM::rndInt(num);
        outExtra->mMsg = msgs4[i];
        outExtra->mItem = items4[i];
    }
}


// 800B4684
int dHR::isBusy() {
    return 0;
}

// 800B468C
void fn_800B468C() {}

// 800B4690
void fn_800B4690(dSvMdlRm_c *room) {}

// 800B4694
BOOL dHR::searchCat_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL ok = bitm != NULL;
    if (ok) {
        int part = (s8)bitm->m_ftrPartB;
        ok = ((u32)part < dItem::FTR_PART_B_COUNT ? part : dItem::FTR_PART_B_ART) == mPart;
    }
    return ok;
}

// 800B4710
BOOL dHR::searchLucky_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm != NULL && bitm->m_hraLuckyBonus) {
        const dItem::Item *p = mExclude;
        if (p != NULL && mExcludeNum != 0) {
            for (; p < mExclude + mExcludeNum; p++) {
                if (p->isSame(*item)) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 800B47D4
BOOL dHR::searchSeries_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm != NULL && mSeries == bitm->getSeries()) {
        const dItem::Item *p = mExclude;
        if (p != NULL && mExcludeNum != 0) {
            for (; p < mExclude + mExcludeNum; p++) {
                if (p->isSame(*item)) {
                    return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 800B48B0
BOOL searchKiddy_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL ok = FALSE;
    if (bitm != NULL && bitm->getAdultKiddy() == 2) {
        ok = TRUE;
    }
    return ok;
}

// 800B4908
BOOL searchAdult_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL ok = FALSE;
    if (bitm != NULL && bitm->getAdultKiddy() == 1) {
        ok = TRUE;
    }
    return ok;
}

// 800B4960
BOOL searchOld_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL ok = FALSE;
    if (bitm != NULL && bitm->getNewOld() == 2) {
        ok = TRUE;
    }
    return ok;
}

// 800B49B8
BOOL searchNew_c::check(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    BOOL ok = FALSE;
    if (bitm != NULL && bitm->getNewOld() == 1) {
        ok = TRUE;
    }
    return ok;
}
