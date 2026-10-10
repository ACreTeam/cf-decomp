// Quest save data. .text 8013F458..80143E78 (sinit 80143DD4).
// See include/game/game/d_quest.hpp and notes/d_quest.txt.
#include <game/game/d_event.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_string.hpp>
#include <game/cLib/c_math.hpp>
#include <cstring>

// Event ids checked by checkEventSchedule / checkTodayEvents. The callee takes the id by const
// reference, so literal arguments become anonymous .sdata temporaries.

// First kind of each quest type (lbl_80476350).
static const int sKindBase[QUEST_TYPE_NUM] = {
    QUEST_KIND_REQUEST_INSECT, QUEST_KIND_ERRAND_REQUEST, QUEST_KIND_APPOINTMENT_0,
    QUEST_KIND_STYLE,          QUEST_KIND_HIDE_AND_SEEK,
};

// Errand start handlers, one per errand kind (lbl_805F2680, 0x10 each). All null; the sinit
// copies __ptmf_null into each entry.
struct dQuestErrandHandler_c {
    void (dQuestErrand_c::*mFunc)();
    u32 _0C;
};
static dQuestErrandHandler_c sErrandHandlers[QUEST_ERRAND_HANDLER_NUM] = {
    {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL},
};

// ---------------------------------------------------------------------------
// dQuestWish_c

// 8013F458
dQuestWish_c::dQuestWish_c() {}

// 8013F45C
dQuestWish_c::~dQuestWish_c() {}

// 8013F49C
void dQuestWish_c::clear() {
    mKind = 8;
    mValue = 0;
}

// 8013F4B0
BOOL dQuestWish_c::isValid() {
    return mKind < 8;
}

// 8013F4C0
void dQuestWish_c::set(int kind) {
    clear();
    mKind = kind;
    setValue(0);
}

// 8013F508
void dQuestWish_c::setRandom() {
    set(cM::rndInt(8));
}

// 8013F544
void dQuestWish_c::setValue(u8 value) {
    if (value > 100) {
        value = 100;
    }
    mValue = value;
}

// 8013F558
u8 dQuestWish_c::addValue(int delta) {
    if (isValid()) {
        int value = mValue + delta;
        if (value < 0) {
            value = 0;
        } else if (value >= 100) {
            value = 100;
        }
        setValue(value);
    }
    return mValue;
}

// 8013F5C8
int dQuestWish_c::getQuestKind() {
    return toQuestKind(mKind);
}

// 8013F5D0
BOOL dQuestWish_c::isWishItem(const dItem::Item *item, int kind) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }

    int itemKind = bitm->getKind();
    switch (kind) {
        case 0:
            if (itemKind == dItem::KIND_INSECT) {
                return TRUE;
            }
            break;
        case 1:
            if (itemKind == dItem::KIND_FISH) {
                return TRUE;
            }
            break;
        case 2:
            if (itemKind == dItem::KIND_FOSSIL) {
                return TRUE;
            }
            break;
        case 3:
            if (itemKind == dItem::KIND_CLOTH) {
                return TRUE;
            }
            break;
        case 4:
            if (itemKind == dItem::KIND_FTR) {
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// 8013F6C4
BOOL dQuestWish_c::isWishItem(const dItem::Item *item) {
    if (isValid() == FALSE) {
        return FALSE;
    }
    return isWishItem(item, mKind);
}

// ---------------------------------------------------------------------------
// dQuestBase_c

// 8013F718
dQuestBase_c::dQuestBase_c() {}

// 8013F760
dQuestBase_c::~dQuestBase_c() {}

// 8013F7A0
void dQuestBase_c::clear() {
    mKind = QUEST_KIND_NONE;
    mItem = dItem::ITEM_ID_NONE;
    mTimeLimit.reset();
    mDeadline = QUEST_DEADLINE_LIMIT;
}

// 8013F7E8
BOOL dQuestBase_c::isActive() const {
    return isValidKind(mKind);
}

// 8013F7F0
void dQuestBase_c::set(int kind, const dItem::Item *item, dTime_c *limit, u8 deadline, u8 state) {
    mKind = kind;
    mItem = *item;
    mState = state;
    if (limit != NULL) {
        setTimeLimit(*limit);
    }
    mDeadline = deadline;
}

// 8013F848
int dQuestBase_c::getType() const {
    return getKindType(mKind);
}

// 8013F850
int dQuestBase_c::getSubType() const {
    return getKindSubType(mKind);
}

// 8013F858
BOOL dQuestBase_c::isDaytime(const dTime_c &time) {
    if (time.hour >= 5 && time.hour < 22) {
        return TRUE;
    }
    return FALSE;
}

// 8013F87C
BOOL dQuestBase_c::isBeforeNight(const dTime_c &time) {
    return time.hour < 22;
}

// 8013F898: picks one of kinds at random among those allowed at this time.
int dQuestBase_c::pickDeadline(const int *deadlines, u32 num, dTime_c *time) {
    static BOOL (*const sChecks[7])(const dTime_c &) = {NULL, isDaytime, isBeforeNight, NULL, NULL, NULL, NULL};
    static const int sDefault[3] = {0, 1, 2};

    u32 count = 0;
    int result = 0;

    if (time == NULL) {
        time = dTime_c::getCurrent();
    }

    if (deadlines == NULL || num == 0) {
        deadlines = sDefault;
        num = 3;
    }

    if (deadlines != NULL && num != 0) {
        for (u32 i = 0; i < num; i++, deadlines++) {
            u32 deadline = *deadlines;
            if (deadline < QUEST_DEADLINE_NUM) {
                BOOL (*check)(const dTime_c &) = sChecks[deadline];
                if (check == NULL || check(*time)) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        result = *deadlines;
                    }
                    count++;
                }
            }
        }
    }

    return result;
}

// 8013F9D8
void dQuestBase_c::setTimeLimit(const dTime_c &time) {
    mTimeLimit.set(&time);
}

// 8013F9DC
dTime_c dQuestBase_c::getTimeLimit() const {
    return mTimeLimit.get();
}

// 8013F9E0: the top of the next hour (or the one after, from :50)
dTime_c dQuestBase_c::calcNextHour(const dTime_c &time) {
    dTime_c result;
    if (time.min >= 50) {
        result.set(time.year, time.month, time.mday, time.hour + 2, 0, 0);
    } else {
        result.set(time.year, time.month, time.mday, time.hour + 1, 0, 0);
    }
    result.normalize();
    return result;
}

// 8013FABC: noon, 6 PM or midnight
dTime_c dQuestBase_c::calcNextPeriod(const dTime_c &time) {
    dTime_c result;
    if (time.hour < 10) {
        result.set(time.year, time.month, time.mday, 12, 0, 0);
    } else if (time.hour < 16) {
        result.set(time.year, time.month, time.mday, 18, 0, 0);
    } else {
        result.set(time.year, time.month, time.mday + 1, 0, 0, 0);
    }
    result.normalize();
    return result;
}

// 8013FBC0: midnight
dTime_c dQuestBase_c::calcMidnight(const dTime_c &time) {
    dTime_c result;
    result.set(time.year, time.month, time.mday + 1, 0, 0, 0);
    result.normalize();
    return result;
}

// 8013FC68: 6 AM, three days on
dTime_c dQuestBase_c::calc3Days(const dTime_c &time) {
    dTime_c result = time;
    result.add(0, -6, 0, 0);
    result.hour = 6;
    result.min = 0;
    result.sec = 0;
    result.msec = 0;
    result.usec = 0;
    result.add(3, 0, 0, 0);
    return result;
}

// 8013FD84: twelve hours on
dTime_c dQuestBase_c::calc12Hours(const dTime_c &time) {
    dTime_c result = time;
    result.set(result.year, result.month, result.mday, result.hour + 12, result.min, 0);
    result.normalize();
    return result;
}

// 8013FE80: a week on
dTime_c dQuestBase_c::calc1Week(const dTime_c &time) {
    dTime_c result = time;
    result.set(result.year, result.month, result.mday + 7, result.hour, result.min, 0);
    result.normalize();
    return result;
}

// 8013FF7C
dTime_c dQuestBase_c::getDeadline(const dTime_c &now) const {
    static dTime_c (*const sDeadlines[QUEST_DEADLINE_NUM])(const dTime_c &) = {
        calcNextHour, calcNextPeriod, calcMidnight, calc3Days, calc12Hours, NULL, calc1Week,
    };

    u32 deadline = mDeadline;
    dTime_c limit = getTimeLimit();
    if (deadline < QUEST_DEADLINE_NUM) {
        dTime_c (*calc)(const dTime_c &) = sDeadlines[deadline];
        if (calc != NULL) {
            return calc(limit);
        }
        return now;
    }
    return limit;
}

// 801400E4
BOOL dQuestBase_c::isPastDeadline(const dTime_c *now) const {
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    dTime_c limit = getDeadline(*now);
    if (!dTime_c::isSameOrAfter(limit, *now)) {
        return TRUE;
    }
    return FALSE;
}

// 80140230
BOOL dQuestBase_c::isExpired(const dTime_c *now) const {
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    dTime_c limit = getTimeLimit();
    if (!dTime_c::isSameOrAfter(*now, limit)) {
        return TRUE;
    }

    int type = getType();
    int kind = mKind;
    if (type == QUEST_TYPE_REQUEST || kind == QUEST_KIND_STYLE) {
        dTime_c end = getDeadline(*now);
        if (kind != QUEST_KIND_REQUEST_6) {
            end.add(1, 0, 0, 0);
        }
        if (!dTime_c::isSameOrAfter(end, *now)) {
            return TRUE;
        }
    } else if (type == QUEST_TYPE_ERRAND && getSubType() == QUEST_ERRAND_TYPE_FIRSTJOB) {
        dTime_c end = getDeadline(*now);
        end.add(0, -6, 0, 0);
        end.hour = 6;
        end.min = 0;
        end.sec = 0;
        end.msec = 0;
        end.usec = 0;
        end.add(1, 0, 0, 0);
        if (!dTime_c::isSameOrAfter(end, *now)) {
            return TRUE;
        }
    } else {
        return isPastDeadline(now);
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// dQuestErrand_c

// 80140650
dQuestErrand_c::dQuestErrand_c() {}

// 80140680
dQuestErrand_c::~dQuestErrand_c() {}

// 801406D8
void dQuestErrand_c::clear() {
    mBase.clear();
    for (int i = 0; i < 2; i++) {
        mAnimals[i].clear();
    }
    _18E = 0;
}

// 8014073C
BOOL dQuestErrand_c::start(int kind, const dAnmPersonalID_c *animal0, const dAnmPersonalID_c *animal1,
                           const dItem::Item *item, dTime_c *limit, u8 deadline, u8 state) {
    int index = -1;
    clear();
    if (dQuestBase_c::getKindType(kind) == QUEST_TYPE_ERRAND && dQuestBase_c::getKindIndex(&index, kind)) {
        mBase.set(kind, item, limit, deadline, state);
        if (sErrandHandlers[index].mFunc) {
            (this->*sErrandHandlers[index].mFunc)();
        }
        if (animal0 != NULL) {
            mAnimals[0].copy(animal0);
        }
        if (animal1 != NULL) {
            mAnimals[1].copy(animal1);
        }
        return TRUE;
    }
    return FALSE;
}

// 80140840
dAnmPersonalID_c *dQuestErrand_c::getAnimal(int i) {
    return &mAnimals[i];
}

// 80140850
dAnmPersonalID_c *dQuestErrand_c::getAnimal(int i) const {
    return (dAnmPersonalID_c *)&mAnimals[i];
}

// ---------------------------------------------------------------------------
// dQuestErrandList_c

// 80140860
dQuestErrandList_c::dQuestErrandList_c() {}

// 801408A8
dQuestErrandList_c::~dQuestErrandList_c() {}

// 8014090C
void dQuestErrandList_c::clearAnimals() {
    for (int i = 0; i < 3; i++) {
        mAnimals[i].clear();
    }
}

// 80140958
dAnmPersonalID_c *dQuestErrandList_c::getAnimal(int i) {
    return &mAnimals[i];
}

// 80140968
dAnmPersonalID_c *dQuestErrandList_c::getAnimal(int i) const {
    return (dAnmPersonalID_c *)&mAnimals[i];
}

// 80140978
void dQuestErrandList_c::clear() {
    mErrands[0].clear();
    clearAnimals();
}

// 801409AC
dQuestErrand_c *dQuestErrandList_c::getErrand(int i) {
    return &mErrands[i];
}

// 801409B8
dQuestErrand_c *dQuestErrandList_c::getErrand(int i) const {
    return (dQuestErrand_c *)&mErrands[i];
}

// 801409C4
dQuestErrand_c *dQuestErrandList_c::get(u32 i) {
    if (i < 1) {
        return &mErrands[i];
    }
    return NULL;
}

// 801409E0
dQuestErrand_c *dQuestErrandList_c::get(u32 i) const {
    if (i < 1) {
        return (dQuestErrand_c *)&mErrands[i];
    }
    return NULL;
}

// 801409FC
BOOL dQuestErrandList_c::isActive() const {
    dQuestErrand_c *errand = getErrand(0);
    if (errand->mBase.isActive() && errand->mBase.getSubType() == QUEST_ERRAND_TYPE_FIRSTJOB) {
        return TRUE;
    }
    return FALSE;
}

// 80140A54
int dQuestErrandList_c::getKind() const {
    if (isActive()) {
        return getErrand(0)->mBase.mKind;
    }
    return QUEST_KIND_NONE;
}

// 80140AA0
BOOL dQuestErrandList_c::isState(u8 state) const {
    if (isActive() && state == getErrand(0)->mBase.mState) {
        return TRUE;
    }
    return FALSE;
}

// 80140B04
BOOL dQuestErrandList_c::setState(u8 state) {
    if (isActive()) {
        getErrand(0)->mBase.mState = state;
        return TRUE;
    }
    return FALSE;
}

// 80140B60
void dQuestErrandList_c::resetState(u8 state) {
    if (isActive()) {
        dQuestErrand_c *errand = getErrand(0);
        if (state == errand->mBase.mState) {
            errand->mBase.mState = 0;
        }
    }
}

// 80140BC0
dItem::Item *dQuestErrandList_c::getItem() const {
    return &getErrand(0)->mBase.mItem;
}

// 80140BE8
const dAnmPersonalID_c *dQuestErrandList_c::getSender() const {
    if (isActive()) {
        const dQuestErrand_c *errand = getErrand(0);
        const dAnmPersonalID_c *animal = errand->getAnimal(1);
        if (animal->isValid()) {
            return animal;
        }
    }
    return NULL;
}

// 80140C4C
void dQuestErrandList_c::startChangeCloth() {
    dQuestErrand_c *errand = getErrand(0);
    dItem::Item item;
    errand->start(QUEST_KIND_FIRSTJOB_CHANGE_CLOTH, NULL, NULL, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
    clearAnimals();
}

// 80140CB0
void dQuestErrandList_c::startPlantFlower() {
    dQuestErrand_c *errand = getErrand(0);
    dItem::Item item;
    errand->start(QUEST_KIND_FIRSTJOB_PLANT_FLOWER, NULL, NULL, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
}

// 80140D00
void dQuestErrandList_c::start11() {
    dQuestErrand_c *errand = getErrand(0);
    dItem::Item item;
    errand->start(QUEST_KIND_FIRSTJOB_11, NULL, NULL, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
}

// 80140D50
void dQuestErrandList_c::startDeliverFtr() {
    dQuestErrand_c *errand = getErrand(0);
    dSaveTown_c *save = dSaveData_c::getTown();
    dAnimal_c *animal = save->mAnimals.mTown.pickRandomAnimalNotMoving(NULL, 0);
    if (animal != NULL) {
        dItem::Item item(dItem::ITEM_IDX_EXOTIC_BED);
        errand->start(QUEST_KIND_FIRSTJOB_DELIVER_FTR, NULL, &animal->mID, &item, dTime_c::getCurrent(),
                      QUEST_DEADLINE_12_HOURS, 0);
        getAnimal(0)->copy(&animal->mID);
    }
}

// 80140E00
void dQuestErrandList_c::startDeliverFtrAgain() {
    dAnimalBlock_c *block;
    dQuestErrand_c *errand = getErrand(0);
    const dAnmPersonalID_c *exclude[1] = {NULL};
    int num = 0;

    const dAnmPersonalID_c *prev = getSender();
    if (prev != NULL && prev->isValid()) {
        exclude[0] = prev;
        num = 1;
    }

    block = &dSaveData_c::getTown()->mAnimals.mTown;
    dAnimal_c *animal = block->pickRandomAnimalNotMoving(exclude, num);
    const dAnmPersonalID_c *target = NULL;
    if (animal != NULL) {
        target = &animal->mID;
    } else if (prev != NULL && prev->isValid()) {
        dItem::Item key = block->getAnimalKey(prev);
        if (key.mId != dItem::ITEM_ID_NONE) {
            target = prev;
        }
    }

    if (target != NULL) {
        dItem::Item item(dItem::ITEM_IDX_EXOTIC_BED);
        errand->start(QUEST_KIND_FIRSTJOB_DELIVER_FTR, NULL, target, &item, dTime_c::getCurrent(),
                      QUEST_DEADLINE_12_HOURS, 0);
        getAnimal(0)->copy(target);
    }
}

// 80140F28
void dQuestErrandList_c::startSendLetter() {
    dQuestErrand_c *errand = getErrand(0);
    const dAnmPersonalID_c *exclude[1] = {NULL};
    int num = 0;

    dAnmPersonalID_c *prev = getAnimal(0);
    if (prev->isValid()) {
        exclude[0] = prev;
        num = 1;
    }

    dSaveTown_c *save = dSaveData_c::getTown();
    dAnimal_c *animal = save->mAnimals.mTown.pickRandomAnimalNotMoving(exclude, num);
    if (animal != NULL) {
        dItem::Item item;
        errand->start(QUEST_KIND_FIRSTJOB_SEND_LETTER, NULL, &animal->mID, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        getAnimal(1)->copy(&animal->mID);
    }
}

// 8014100C
void dQuestErrandList_c::startSendLetter2() {
    startSendLetter();
}

// 80141010: a letter was sent during QUEST_KIND_FIRSTJOB_SEND_LETTER
BOOL dQuestErrandList_c::checkLetter(dMail_c *mail) {
    if (mail->isEmpty()) {
        return FALSE;
    }

    dQuestErrand_c *errand = getErrand(0);
    if ((int)errand->mBase.mKind != QUEST_KIND_FIRSTJOB_SEND_LETTER) {
        return FALSE;
    }

    dAnmPersonalID_c *to = mail->getToAnimal();
    if (to == NULL) {
        if (errand->mBase.mState == 0) {
            errand->mBase.mState = 2;
            return TRUE;
        }
        return FALSE;
    }

    if (to->isValid() && *to == *errand->getAnimal(1)) {
        if (errand->mBase.mState == 0 || errand->mBase.mState == 2) {
            errand->mBase.mState = 3;
            return TRUE;
        }
    } else if (errand->mBase.mState == 0) {
        errand->mBase.mState = 2;
        return TRUE;
    }
    return FALSE;
}

// 801411B8
void dQuestErrandList_c::startDeliverCarpet() {
    dQuestErrand_c *errand = getErrand(0);
    const dAnmPersonalID_c *exclude[2] = {NULL, NULL};
    const dAnmPersonalID_c **p = exclude;
    int num = 0;

    for (int i = 0; i < 2; i++) {
        dAnmPersonalID_c *animal = getAnimal(i);
        if (animal->isValid()) {
            *p++ = animal;
            num++;
        }
    }

    dSaveTown_c *save = dSaveData_c::getTown();
    dAnimal_c *animal = save->mAnimals.mTown.pickRandomAnimalNotMoving(exclude, num);
    if (animal != NULL) {
        dItem::Item item(dItem::ITEM_IDX_EXOTIC_RUG);
        errand->start(QUEST_KIND_FIRSTJOB_DELIVER_CARPET, NULL, &animal->mID, &item, dTime_c::getCurrent(),
                      QUEST_DEADLINE_12_HOURS, 0);
        getAnimal(2)->copy(&animal->mID);
    }
}

// 801412AC
void dQuestErrandList_c::startDeliverCarpetAgain() {
    dAnimalBlock_c *block;
    dQuestErrand_c *errand = getErrand(0);
    const dAnmPersonalID_c *exclude[3] = {NULL, NULL, NULL};
    const dAnmPersonalID_c **p = exclude;
    int num = 0;

    for (int i = 0; i < 2; i++) {
        dAnmPersonalID_c *animal = getAnimal(i);
        if (animal->isValid()) {
            *p++ = animal;
            num++;
        }
    }

    const dAnmPersonalID_c *prev = getSender();
    if (prev != NULL && prev->isValid()) {
        exclude[num] = prev;
        num++;
    }

    block = &dSaveData_c::getTown()->mAnimals.mTown;
    dAnimal_c *animal = block->pickRandomAnimalNotMoving(exclude, num);
    const dAnmPersonalID_c *target = NULL;
    if (animal != NULL) {
        target = &animal->mID;
    } else if (prev != NULL && prev->isValid()) {
        dItem::Item key = block->getAnimalKey(prev);
        if (key.mId != dItem::ITEM_ID_NONE) {
            target = prev;
        }
    }

    if (target != NULL) {
        dItem::Item item(dItem::ITEM_IDX_EXOTIC_RUG);
        errand->start(QUEST_KIND_FIRSTJOB_DELIVER_CARPET, NULL, target, &item, dTime_c::getCurrent(),
                      QUEST_DEADLINE_12_HOURS, 0);
        getAnimal(2)->copy(target);
    }
}

// 80141420
void dQuestErrandList_c::startDeliverWateringCan() {
    dAnimalBlock_c *block;
    dQuestErrand_c *errand = getErrand(0);
    dAnmPersonalID_c *prev = getAnimal(1);
    block = &dSaveData_c::getTown()->mAnimals.mTown;
    const dAnmPersonalID_c *target = NULL;

    dItem::Item key = block->getAnimalKey(prev);
    dAnimal_c *animal = block->getAnimalByKey(&key);
    if (animal != NULL && !animal->isMoving()) {
        target = &animal->mID;
    } else {
        const dAnmPersonalID_c *exclude[1] = {NULL};
        exclude[0] = getAnimal(2);
        animal = block->pickRandomAnimalNotMoving(exclude, 1);
        if (animal != NULL) {
            target = &animal->mID;
        } else {
            animal = block->pickRandomAnimalNotMoving(NULL, 0);
            if (animal != NULL) {
                target = &animal->mID;
            }
        }
        prev->clear();
    }

    if (target != NULL) {
        dItem::Item item(dItem::ITEM_IDX_WATERING_CAN);
        errand->start(QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN, NULL, target, &item, dTime_c::getCurrent(),
                      QUEST_DEADLINE_12_HOURS, 0);
    }
}

// 80141550
void dQuestErrandList_c::startPostNotice() {
    dQuestErrand_c *errand = getErrand(0);
    dItem::Item item;
    errand->start(QUEST_KIND_FIRSTJOB_POST_NOTICE, NULL, NULL, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
}

// 801415A0
void dQuestErrandList_c::notifyNoticePosted() {
    dQuestErrand_c *errand = getErrand(0);
    if (errand->mBase.isActive() && (int)errand->mBase.mKind == QUEST_KIND_FIRSTJOB_POST_NOTICE &&
        errand->mBase.mState == 0) {
        errand->mBase.mState = 1;
    }
}

// 801415FC
void dQuestErrandList_c::cancel() {
    if (isActive()) {
        getErrand(0)->clear();
    }
}

// 80141640
BOOL dQuestErrandList_c::isSender(const dAnmPersonalID_c *animal) const {
    if (animal->isValid() && isActive()) {
        dAnmPersonalID_c *other = getAnimal(1);
        if (*(dAnmPersonalID_c *)animal == *other) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80141748
BOOL dQuestErrandList_c::isExpired(const dTime_c &now) const {
    if (!isActive()) {
        return FALSE;
    }

    const dQuestErrand_c *errand = getErrand(0);
    dQuestKind_e kind = (dQuestKind_e)errand->mBase.mKind;
    u8 state = errand->mBase.mState;
    BOOL result = FALSE;
    switch ((int)kind) {
        case QUEST_KIND_FIRSTJOB_DELIVER_FTR:
            if (state < 2) {
                result = errand->mBase.isExpired(&now);
            }
            break;
        case QUEST_KIND_FIRSTJOB_DELIVER_CARPET:
            if (state < 2) {
                result = errand->mBase.isExpired(&now);
            }
            break;
        case QUEST_KIND_FIRSTJOB_DELIVER_WATERING_CAN:
            if (state < 1) {
                result = errand->mBase.isExpired(&now);
            }
            break;
    }
    return result;
}

// ---------------------------------------------------------------------------
// dQuestVillager_c

// 80141814
dQuestVillager_c::dQuestVillager_c() {}

// 80141844
dQuestVillager_c::~dQuestVillager_c() {}

// 8014189C
void dQuestVillager_c::clear() {
    mBase.clear();
    clearRequester();
    for (u32 i = 0; i < 4; i++) {
        mPlayers[i].clear();
    }
    mPlayerFlags = 0;
    mMatchMode = 0;
    mMatchParam = 0;
}

// 80141910
BOOL dQuestVillager_c::start(int kind, const dPlayerID_c *player, const dItem::Item *item, dTime_c *limit,
                             u8 deadline, u8 state) {
    int index = -1;
    clear();
    if (dQuestBase_c::getKindType(kind) == QUEST_TYPE_REQUEST && dQuestBase_c::getKindIndex(&index, kind)) {
        mBase.set(kind, item, limit, deadline, state);
        if (player != NULL && player->isValid()) {
            addPlayer(player, TRUE);
        }
    }
    return FALSE;
}

// 801419D0
BOOL dQuestVillager_c::isValidPlayerIndex(u32 i) {
    return i < 4;
}

// 801419E8
void dQuestVillager_c::setPlayerFlag(int i) {
    if (isValidPlayerIndex(i)) {
        mPlayerFlags |= (1 << i);
    }
}

// 80141A3C
void dQuestVillager_c::clearPlayerFlag(int i) {
    if (isValidPlayerIndex(i)) {
        mPlayerFlags &= ~(1 << i);
    }
}

// 80141A90
BOOL dQuestVillager_c::getPlayerFlag(int i) {
    if (isValidPlayerIndex(i)) {
        return (mPlayerFlags >> i) & 1;
    }
    return FALSE;
}

// 80141AE4
dPlayerID_c *dQuestVillager_c::getPlayer(int i) {
    if (isValidPlayerIndex(i)) {
        return &mPlayers[i];
    }
    return NULL;
}

// 80141B38
int dQuestVillager_c::findPlayer(const dPlayerID_c *player) {
    if (player->isValid()) {
        dPlayerID_c *other = getPlayer(0);
        for (u32 i = 0; i < 4; i++, other++) {
            if (player->isSame(other)) {
                return i;
            }
        }
    }
    return -1;
}

// 80141BC8: forgets players that are no longer in the save
void dQuestVillager_c::removeMissingPlayers() {
    dPrivateData_c *players = dSaveData_c::getRaw()->mPlayers;
    dPlayerID_c *player = mPlayers;
    for (u32 i = 0; i < 4; i++, player++) {
        if (player->isValid()) {
            BOOL found = FALSE;
            for (int j = 0; j < 4; j++) {
                dPrivateData_c *data = dPrivateData_c::getRaw(players, j);
                if (data != NULL && data->mPID.player.isSame(player)) {
                    found = TRUE;
                    break;
                }
            }
            if (!found) {
                player->clear();
                clearPlayerFlag(i);
            }
        }
    }
}

// 80141C88
int dQuestVillager_c::findEmptyPlayer() {
    dPlayerID_c *player = mPlayers;
    for (u32 i = 0; i < 4; i++, player++) {
        if (!player->isValid()) {
            return i;
        }
    }
    return -1;
}

// 80141CE8
BOOL dQuestVillager_c::addPlayer(const dPlayerID_c *player, BOOL flag) {
    if (player->isValid()) {
        int i = findPlayer(player);
        if (!isValidPlayerIndex(i)) {
            i = findEmptyPlayer();
            if (!isValidPlayerIndex(i)) {
                removeMissingPlayers();
                i = findEmptyPlayer();
            }
            if (isValidPlayerIndex(i)) {
                mPlayers[i].copy(player);
                if (flag) {
                    setPlayerFlag(i);
                }
                return TRUE;
            }
        } else if ((u32)flag != (u32)getPlayerFlag(i)) {
            if (flag) {
                setPlayerFlag(i);
            } else {
                clearPlayerFlag(i);
            }
        }
    }
    return FALSE;
}

// 80141E18
int dQuestVillager_c::countPlayers(BOOL all) {
    dPlayerID_c *player = getPlayer(0);
    int num = 0;
    for (u32 i = 0; i < 4; i++, player++) {
        if (player->isValid() && (all || getPlayerFlag(i))) {
            num++;
        }
    }
    return num;
}

// 80141EA4: picks a flagged player other than exclude. count is never incremented in the
// target, so the last candidate always wins.
dPlayerID_c *dQuestVillager_c::pickOtherPlayer(const dPlayerID_c *exclude) {
    if (!exclude->isValid()) {
        return NULL;
    }

    dPlayerID_c *player = getPlayer(0);
    dPlayerID_c *result = NULL;
    u32 count = 0;
    for (u32 i = 0; i < 4; i++, player++) {
        if (getPlayerFlag(i) && player->isValid() && !exclude->isSame(player)) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                result = player;
            }
        }
    }
    return result;
}

// 80141FC0
void dQuestVillager_c::removePlayer(const dPlayerID_c *player) {
    int i = findPlayer(player);
    if (isValidPlayerIndex(i)) {
        mPlayers[i].clear();
        clearPlayerFlag(i);
    }
}

// 80142028
BOOL dQuestVillager_c::getPlayerFlag(const dPlayerID_c *player) {
    if (player->isValid()) {
        int i = findPlayer(player);
        if (isValidPlayerIndex(i)) {
            return getPlayerFlag(i);
        }
    }
    return FALSE;
}

// 801420A4
void dQuestVillager_c::clearPlayerFlag(const dPlayerID_c *player) {
    if (player->isValid()) {
        int i = findPlayer(player);
        if (isValidPlayerIndex(i)) {
            clearPlayerFlag(i);
        }
    }
}

// 80142118
void dQuestVillager_c::setRequester(const dPlayerID_c *player) {
    mRequester.copy(player);
}

// 80142120
void dQuestVillager_c::clearRequester() {
    mRequester.clear();
}

// 80142128: insect price band (0-2), or 3 if not an insect
int dQuestVillager_c::getInsectPriceRank(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    int result = 3;
    if (bitm != NULL && bitm->getKind() == dItem::KIND_INSECT) {
        int price = item->getPrice();
        static const int sPrices[2] = {2000, 10000};
        result = 2;
        for (int i = 0; i < 2; i++) {
            if (price < sPrices[i]) {
                result = i;
                break;
            }
        }
    }
    return result;
}

// 801421C8: fish price band (0-2), or 3 if not a fish
int dQuestVillager_c::getFishPriceRank(const dItem::Item *item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    int result = 3;
    if (bitm != NULL && bitm->getKind() == dItem::KIND_FISH) {
        int price = item->getPrice();
        static const int sPrices[2] = {2000, 10000};
        result = 2;
        for (int i = 0; i < 2; i++) {
            if (price < sPrices[i]) {
                result = i;
                break;
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// dLostQuest_c

// 80142268
dLostQuest_c::dLostQuest_c() {}

// 801422A4
void dLostQuest_c::clear() {
    mTime.reset();
    _08 = -1;
    mKeyIdx = -1;
}

// 801422DC
void dLostQuest_c::set(const dTime_c &time, int value, const dItem::Item *item) {
    setTime(time);
    _08 = value;
    fn_80142338(item);
}

// 80142330
void dLostQuest_c::setTime(const dTime_c &time) {
    mTime.set(&time);
}

// 80142334
dTime_c dLostQuest_c::getTime() {
    return mTime.get();
}

// 80142338
void dLostQuest_c::fn_80142338(const dItem::Item *item) {
    s8 group = -1;
    if (item->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm != NULL && bitm->getKind() == 0x35) {
            group = dItem::seeker_c::get()->findLike(*item);
        }
    }
    mKeyIdx = group;
}

// 801423D8: random item from index group 0xBF, other than mKeyIdx
dItem::Item dLostQuest_c::fn_801423D8() {
    dItem::Item item;
    u32 count = 0;
    for (u32 i = 0; i < 8; i++) {
        if ((int)i != mKeyIdx) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                item.setFromIndex(dItem::ITEM_IDX_KEY_00, i, FALSE);
            }
            count++;
        }
    }
    return item;
}

// 801424BC
BOOL dLostQuest_c::fn_801424BC(dTime_c *now) {
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }

    dTime_c time = getTime();
    if (!dTime_c::isSameOrAfter(*now, time)) {
        setTime(*now);
        return FALSE;
    }

    dTime_c a;
    a.set(now->year, now->month, now->mday, now->hour - 6, 0, 0);
    a.hour = 6;
    a.normalize();

    dTime_c b;
    b.set(time.year, time.month, time.mday, time.hour - 6, 0, 0);
    b.hour = 6;
    b.normalize();

    int days = dTime_c::diffDays(&a, &b, FALSE);
    if (days >= 4) {
        f32 chance;
        if (days < 13) {
            chance = 10.0f * (days - 3);
        } else {
            chance = 100.0f;
        }
        if (cM::rndF(100.0f) < chance) {
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// dQuestSick_c

// 801426DC
dQuestSick_c::dQuestSick_c() {}

// 801426E0
void dQuestSick_c::clear() {
    mAnimalIdx = -1;
    _43 = 0;
    mSickness = -4;
    _45 = 0;
    for (int i = 0; i < 3; i++) {
        mPlayers[i].clear();
    }
}

// 80142748
dItem::Item dQuestSick_c::getMedicine() {
    return dItem::Item((int)11);
}

// 80142750
int dQuestSick_c::getMaxSickness(u32 i) {
    switch (i) {
        case 2:
        case 5:
        case 8:
        case 10:
            return 60;
        case 0:
        case 1:
        case 3:
        case 4:
        case 6:
        case 7:
        case 9:
        default:
            return 40;
    }
}

// 80142780
void dQuestSick_c::addSickness(int delta, u32 i) {
    int value = mSickness;
    value += delta;
    if (value < -4) {
        value = -4;
    } else {
        int max = getMaxSickness(i);
        if (value >= max) {
            value = max;
        }
    }
    mSickness = value;
}

// 801427E4
void dQuestSick_c::start(int value) {
    mAnimalIdx = value;
    mSickness = 0;
    _43 = 0;
    _45 = 0;
    for (int i = 0; i < 3; i++) {
        mPlayers[i].clear();
    }
}

// 80142844
void dQuestSick_c::addVisit() {
    _45++;
    if (_45 >= 3) {
        _43 = 1;
        _45 = 3;
    }
}

// 80142870
dPlayerID_c *dQuestSick_c::getPlayer(u32 i) {
    if (i < 3) {
        return &mPlayers[i];
    }
    return NULL;
}

// 8014288C
void dQuestSick_c::setPlayer(u32 i, const dPlayerID_c *player) {
    if (i < 3) {
        mPlayers[i].copy(player);
    }
}

// 801428A8
void dQuestSick_c::clearPlayer(u32 i) {
    if (i < 3) {
        mPlayers[i].clear();
    }
}

// 801428C0
BOOL dQuestSick_c::hasPlayer(const dPlayerID_c *player) {
    if (!player->isValid()) {
        return FALSE;
    }

    dPlayerID_c *other = getPlayer(0);
    for (int i = 0; i < 3; i++, other++) {
        if (player->isSame(other)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80142958
int dQuestSick_c::countPlayers() {
    int num = 0;
    dPlayerID_c *player = getPlayer(0);
    for (int i = 0; i < 3; i++, player++) {
        if (player->isValid()) {
            num++;
        }
    }
    return num;
}

// ---------------------------------------------------------------------------
// dQuestPlayerItem_c

// 801429C8
dQuestPlayerItem_c::dQuestPlayerItem_c() {}

// 80142A08
void dQuestPlayerItem_c::clear() {
    mBase.clear();
    mPlayer.clear();
    mHour = 0;
    mAnimalIdx = -1;
    mMinute = 0;
    mItem = dItem::ITEM_ID_NONE;
    mFlags = 0;
}

// 80142A64: the limit's date at mHour:mMinute, pushed a day on if that is not after the limit
dTime_c dQuestPlayerItem_c::getMeetTime() {
    dTime_c limit = mBase.getTimeLimit();
    dTime_c result;
    result.set(limit.year, limit.month, limit.mday, mHour, mMinute, 0);
    result.normalize();
    if (dTime_c::isSameOrAfter(limit, result)) {
        result.add(1, 0, 0, 0);
    }
    return result;
}

// 80142C2C
BOOL dQuestPlayerItem_c::isPastMeetTime(const dTime_c &now, int mins) {
    dTime_c start = getMeetTime();
    dTime_c end;
    memcpy(&end, &start, sizeof(dTime_c));
    end.add(0, 0, mins, 0);
    return dTime_c::isSameOrAfter(now, end) != FALSE;
}

// 80142DA0
void dQuestPlayerItem_c::start(u8 kind, const dPlayerID_c *player, int value, u8 hour, u8 min, dTime_c *limit) {
    clear();
    dItem::Item item;
    mBase.set(kind, &item, limit, QUEST_DEADLINE_1_WEEK, 0);
    mPlayer.copy(player);
    mAnimalIdx = value;
    mHour = hour;
    mMinute = min;
}

// 80142E2C
void dQuestPlayerItem_c::setFlag(int bit) {
    mFlags |= (u8)(1 << bit);
}

// 80142E48
BOOL dQuestPlayerItem_c::isFlag(u32 bit) {
    if (bit < 8) {
        return (mFlags >> bit) & 1;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// dQuestPlayerAnimal_c

// 80142E68
dQuestPlayerAnimal_c::dQuestPlayerAnimal_c() {}

// 80142EA8
void dQuestPlayerAnimal_c::clear() {
    clearInfo();
    _E9 = -2;
}

// 80142EDC
void dQuestPlayerAnimal_c::clearInfo() {
    mPlayer.clear();
    mAnimal.clear();
    mBase.clear();
    for (int i = 0; i < 3; i++) {
        setHider(i, -1);
    }
    mMinutes = 0;
    _E9 = 0;
    mFoundFlags = 0;
    mItem = dItem::ITEM_ID_NONE;
}

// 80142F5C
int dQuestPlayerAnimal_c::getHider(u32 i) {
    if (i < 3) {
        return mHiders[i];
    }
    return -1;
}

// 80142F7C
void dQuestPlayerAnimal_c::setHider(u32 i, int value) {
    if (i < 3) {
        mHiders[i] = value;
    }
}

// 80142F90
void dQuestPlayerAnimal_c::fn_80142F90(int delta) {
    int value = _E9 + delta;
    if (value < -2) {
        value = -2;
    } else if (value > 10) {
        value = 10;
    }
    _E9 = value;
}

// 80142FC0
int dQuestPlayerAnimal_c::countHiders() {
    int num = 0;
    for (int i = 0; i < 3; i++) {
        if ((u32)getHider(i) < 10) {
            num++;
        }
    }
    return num;
}

// 80143028
void dQuestPlayerAnimal_c::start(int value0, int value1, int value2, const dPlayerID_c *player, dTime_c *limit) {
    static const u8 sBase[3] = {5, 10, 10};
    static const u8 sRange[3] = {2, 2, 2};

    clear();
    dItem::Item item;
    mBase.set(QUEST_KIND_HIDE_AND_SEEK, &item, limit, QUEST_DEADLINE_LIMIT, 0);
    setHider(0, value0);
    setHider(1, value1);
    setHider(2, value2);
    mPlayer.copy(player);

    u32 num = countHiders();
    if (num < 1) {
        num = 1;
    } else if (num > 3) {
        num = 3;
    }
    u32 idx = num - 1;
    mMinutes = sBase[idx] + cM::rndInt(sRange[idx]) * 5;
}

// 80143120
dTime_c dQuestPlayerAnimal_c::getEndTime(BOOL half) {
    dTime_c limit = mBase.getTimeLimit();
    dTime_c result;
    int mins = half ? mMinutes : mMinutes * 2;
    result.set(limit.year, limit.month, limit.mday, limit.hour, limit.min + mins, limit.sec);
    result.normalize();
    return result;
}

// 8014323C
BOOL dQuestPlayerAnimal_c::isTimeUp(const dTime_c &now, BOOL half) {
    if (!mBase.isActive()) {
        return TRUE;
    }

    dTime_c end;
    end = getEndTime(half);
    return dTime_c::isSameOrAfter(now, end) != FALSE;
}

// 8014337C
void dQuestPlayerAnimal_c::setFlag(u32 i) {
    if (i < 3) {
        mFoundFlags |= (1 << i);
    }
}

// 8014339C
BOOL dQuestPlayerAnimal_c::isFlag(u32 i) {
    if (i < 3) {
        return (mFoundFlags >> i) & 1;
    }
    return FALSE;
}

// 801433BC
int dQuestPlayerAnimal_c::countUnfound() {
    u32 num = countHiders();
    int count = 0;
    for (u32 i = 0; i < num; i++) {
        if (!isFlag(i)) {
            count++;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// dQuestPlayerPair_c

// 80143438
dQuestPlayerPair_c::dQuestPlayerPair_c() {}

// 8014346C
void dQuestPlayerPair_c::clear() {
    clearInfo();
    mAnimalIdx = -1;
}

// 801434A0
void dQuestPlayerPair_c::clearInfo() {
    mBase.clear();
    for (int i = 0; i < 2; i++) {
        mPlayers[i].clear();
    }
    memset(mTopicText, 0, sizeof(mTopicText));
    mTopic = QUEST_STYLE_TOPIC_NONE;
    _5E = 0;
}

// 80143520
void dQuestPlayerPair_c::start(int value, const dPlayerID_c *player0, const dPlayerID_c *player1, dTime_c *limit) {
    static const u8 sTopics[15] = {
        QUEST_STYLE_TOPIC_CLOTHES,     QUEST_STYLE_TOPIC_CLOTHES,     QUEST_STYLE_TOPIC_CLOTHES,
        QUEST_STYLE_TOPIC_ACCESSORIES, QUEST_STYLE_TOPIC_ACCESSORIES, QUEST_STYLE_TOPIC_ACCESSORIES,
        QUEST_STYLE_TOPIC_FURNITURE,   QUEST_STYLE_TOPIC_FURNITURE,   QUEST_STYLE_TOPIC_FURNITURE,
        QUEST_STYLE_TOPIC_WALLPAPER,   QUEST_STYLE_TOPIC_WALLPAPER,   QUEST_STYLE_TOPIC_WALLPAPER,
        QUEST_STYLE_TOPIC_CARPET,      QUEST_STYLE_TOPIC_CARPET,      QUEST_STYLE_TOPIC_CARPET,
    };

    dItem::Item item;
    mBase.set(QUEST_KIND_STYLE, &item, limit, QUEST_DEADLINE_3_DAYS, 0);
    mAnimalIdx = value;
    mPlayers[0].copy(player0);
    mPlayers[1].copy(player1);

    u32 index = cM::rndF(15.0f);
    mTopic = sTopics[index];
    setTopicText(index + 1);
}

// 801435DC
dPlayerID_c *dQuestPlayerPair_c::getPlayer(int i) {
    return &mPlayers[i];
}

// 801435E8
void dQuestPlayerPair_c::setTopicText(u16 index) {
    dString::Word_c word(index, "sys_STRING/STR_Q13");
    memset(mTopicText, 0, sizeof(mTopicText));
    dString::WordBase_c *base = &word;
    memcpy(mTopicText, base->getBuffer(), sizeof(mTopicText) - sizeof(wchar_t));
}

// ---------------------------------------------------------------------------
// dQuestVillagerWish_c

// 80143660
dQuestVillagerWish_c::dQuestVillagerWish_c() {}

// 80143698
dQuestVillagerWish_c::~dQuestVillagerWish_c() {}

// 80143700
void dQuestVillagerWish_c::clear() {
    mWish.clear();
    mQuest.clear();
}

// ---------------------------------------------------------------------------
// dQuestBase_c kind helpers

// 80143734: kind is a real quest (not QUEST_KIND_NONE or above)
BOOL dQuestBase_c::isValidKind(u32 kind) {
    return kind < QUEST_KIND_NONE;
}

// 8014374C
BOOL dQuestBase_c::isValidType(u32 type) {
    return type < QUEST_TYPE_NONE;
}

// 80143764
int dQuestBase_c::getKindType(int kind) {
    int type = QUEST_TYPE_NONE;
    if (kind < QUEST_KIND_ERRAND_REQUEST) {
        type = QUEST_TYPE_REQUEST;
    } else if (kind < QUEST_KIND_APPOINTMENT_0) {
        type = QUEST_TYPE_ERRAND;
    } else if (kind < QUEST_KIND_STYLE) {
        type = QUEST_TYPE_APPOINTMENT;
    } else if (kind < QUEST_KIND_HIDE_AND_SEEK) {
        type = QUEST_TYPE_STYLE;
    } else if (kind < QUEST_KIND_NONE) {
        type = QUEST_TYPE_HIDE_AND_SEEK;
    }
    return type;
}

// 801437BC
int dQuestBase_c::getKindSubType(int kind) {
    int result = QUEST_ERRAND_TYPE_NONE;
    if (getKindType(kind) == QUEST_TYPE_ERRAND) {
        if (kind < QUEST_KIND_FIRSTJOB_CHANGE_CLOTH) {
            result = QUEST_ERRAND_TYPE_REQUEST;
        } else if (kind < QUEST_KIND_APPOINTMENT_0) {
            result = QUEST_ERRAND_TYPE_FIRSTJOB;
        }
    }
    return result;
}

// 8014381C
BOOL dQuestBase_c::getKindIndex(int *index, int kind) {
    int type = getKindType(kind);
    if (isValidType(type)) {
        *index = kind - sKindBase[type];
        return TRUE;
    }
    return FALSE;
}

// 80143894: wish kind -> villager quest kind (7 picks one at random)
int dQuestWish_c::toQuestKind(int kind) {
    static const u8 sKinds[5] = {
        QUEST_KIND_REQUEST_INSECT, QUEST_KIND_REQUEST_FISH, QUEST_KIND_REQUEST_FOSSIL, QUEST_KIND_REQUEST_CLOTH,
        QUEST_KIND_REQUEST_FTR,
    };

    int questKind = -1;
    if ((u32)kind < 5) {
        questKind = kind;
    } else if (kind == 7) {
        questKind = cM::rndF(5.0f);
    }

    if ((u32)questKind < 5) {
        return sKinds[questKind];
    }
    return QUEST_KIND_NONE;
}

// 801438F8: is quest kind allowed around the scheduled events at time?
BOOL dQuestBase_c::checkEventSchedule(int kind, const dTime_c *time) {
    static const int sEvents0[10] = {EVENT_TOY_DAY, EVENT_FLEA_MARKET, EVENT_HARVEST_FESTIVAL, EVENT_FISHING_TOURNEY, EVENT_BUG_OFF, EVENT_FIREWORKS, EVENT_HALLOWEEN, EVENT_COUNTDOWN, EVENT_FESTIVALE, 0};
    static const int sEvents1[4] = {EVENT_PLAYER_BIRTHDAY_0, EVENT_PLAYER_BIRTHDAY_1, EVENT_PLAYER_BIRTHDAY_2, EVENT_PLAYER_BIRTHDAY_3};

    dTime_c::getCurrent(); // result unused in the original
    BOOL result = TRUE;
    if (time == NULL) {
        time = dTime_c::getCurrent();
    }

    dTime_c day = *time;
    day.add(0, -6, 0, 0);

    switch (kind) {
        case QUEST_KIND_ERRAND_REQUEST:
        case QUEST_KIND_ERRAND_REQUEST_FINAL: {
            const int *events = sEvents0;
            for (u32 i = 0; i < 9; i++, events++) {
                if (dEvent::isEventWithin((dQuestEvent_e)*events, day, 1, 1)) {
                    result = FALSE;
                    break;
                }
            }
            break;
        }
        case QUEST_KIND_STYLE: {
            const int *events = sEvents0;
            for (u32 i = 0; i < 9; i++, events++) {
                BOOL hit = *events != EVENT_TOY_DAY && dEvent::isEventWithin((dQuestEvent_e)*events, day, 1, 1);
                if (hit) {
                    result = FALSE;
                    break;
                }
            }
            break;
        }
        case QUEST_KIND_APPOINTMENT_0:
        case QUEST_KIND_APPOINTMENT_1: {
            const int *events = sEvents0;
            for (u32 i = 0; i < 9; i++, events++) {
                if (dEvent::isEventWithin((dQuestEvent_e)*events, day, 1, 1)) {
                    result = FALSE;
                    break;
                }
            }
            if (result) {
                events = sEvents1;
                for (u32 i = 0; i < 4; i++, events++) {
                    if (dEvent::isEventWithin((dQuestEvent_e)*events, day, 1, 1)) {
                        result = FALSE;
                        break;
                    }
                }
            }
            break;
        }
        case QUEST_KIND_REQUEST_6: {
            if (dEvent::isEventWithin(EVENT_HALLOWEEN, day, 1, 1)) {
                result = FALSE;
            }
            const int *events = sEvents1;
            for (u32 i = 0; i < 4; i++, events++) {
                if (dEvent::isEventWithin((dQuestEvent_e)*events, day, 1, 1)) {
                    result = FALSE;
                    break;
                }
            }
            break;
        }
    }

    return result;
}

// 80143BA4: is quest kind allowed during today's events?
BOOL dQuestBase_c::checkTodayEvents(int kind) {
    static const int sEvents0[4] = {EVENT_FLEA_MARKET, EVENT_HALLOWEEN, EVENT_COUNTDOWN, EVENT_FESTIVALE};
    static const int sEvents1[22] = {
        EVENT_BUNNY_DAY, EVENT_NEW_YEARS_DAY, EVENT_JP_SETSUBUN, EVENT_JP_GIRLS_DAY, EVENT_JP_CHILDRENS_DAY, EVENT_JP_AUTUMN_MOON, EVENT_JP_TANABATA, EVENT_NA_GROUNDHOG_DAY, EVENT_NA_NATURE_DAY, EVENT_NA_LABOR_DAY, EVENT_NA_EXPLORERS_DAY,
        EVENT_NA_AUTUMN_MOON, EVENT_EU_MIDSUMMERS_DAY, EVENT_EU_NAUGHTY_OR_NICE_DAY, EVENT_EU_MIDWINTERS_DAY, EVENT_EU_AUTUMN_MOON, EVENT_KR_LUNAR_NEW_YEAR, EVENT_KR_ARBOR_DAY, EVENT_KR_TEACHERS_DAY, EVENT_KR_DAEBOREUM, EVENT_APRIL_FOOLS_DAY, 0,
    };

    for (u32 i = 0; i < 4; i++) {
        if (dEvent::isActive((dQuestEvent_e)sEvents0[i])) {
            return FALSE;
        }
    }

    if (kind == QUEST_KIND_HIDE_AND_SEEK) {
        for (u32 i = 0; i < 21; i++) {
            if (dEvent::isActive((dQuestEvent_e)sEvents1[i])) {
                return FALSE;
            }
        }
    }

    if (dEvent::isActive(EVENT_TOY_DAY)) {
        switch (kind) {
            case QUEST_KIND_REQUEST_6:
            case QUEST_KIND_ERRAND_REQUEST:
            case QUEST_KIND_ERRAND_REQUEST_FINAL:
            case QUEST_KIND_APPOINTMENT_0:
            case QUEST_KIND_APPOINTMENT_1:
            case QUEST_KIND_HIDE_AND_SEEK:
                return FALSE;
        }
    }

    if (dEvent::isActive(EVENT_HARVEST_FESTIVAL) || dEvent::isActive(EVENT_FIREWORKS)) {
        switch (kind) {
            case QUEST_KIND_REQUEST_5:
            case QUEST_KIND_REQUEST_6:
            case QUEST_KIND_APPOINTMENT_0:
            case QUEST_KIND_APPOINTMENT_1:
            case QUEST_KIND_STYLE:
            case QUEST_KIND_HIDE_AND_SEEK:
                return FALSE;
        }
    }

    if (dEvent::isActive(EVENT_FISHING_TOURNEY)) {
        switch (kind) {
            case QUEST_KIND_REQUEST_6:
            case QUEST_KIND_HIDE_AND_SEEK:
                return FALSE;
        }
        if (!dEvent::isOver(EVENT_FISHING_TOURNEY)) {
            return FALSE;
        }
    }

    if (dEvent::isActive(EVENT_BUG_OFF)) {
        switch (kind) {
            case QUEST_KIND_REQUEST_6:
            case QUEST_KIND_HIDE_AND_SEEK:
                return FALSE;
        }
        if (!dEvent::isOver(EVENT_BUG_OFF)) {
            return FALSE;
        }
    }

    switch (kind) {
        case QUEST_KIND_REQUEST_6:
        case QUEST_KIND_APPOINTMENT_0:
        case QUEST_KIND_APPOINTMENT_1:
            for (int i = 0; i < 4; i++) {
                if (dEvent::isActive((dQuestEvent_e)(i + EVENT_PLAYER_BIRTHDAY_0))) {
                    return FALSE;
                }
            }
            break;
    }

    return TRUE;
}
