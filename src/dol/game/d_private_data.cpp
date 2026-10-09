#include <game/game/d_field_info.hpp>
#include <game/game/d_private_data.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_script.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/cLib/c_math.hpp>
#include <game/sLib/s_lib.hpp>
#include <cstring>
#include <cstdio>
#include <revolution/OS/OSCache.h>
#include <revolution/OS/OSTime.h>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_npc_notice.hpp>
#include <game/game/d_post_office.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
// Save/town data roots.
BOOL fn_80101490();
void fn_801014A4();
void fn_801014D0();
BOOL fn_80101500();
void fn_80101514();

// dPlayerID_c / dPersonalID_c.
void fn_8013EE54(void *);

// dMail_c.
int fn_800CBE0C(int, int, int, int);

// dDesign_c.

// Other members.
void fn_80150AD4(dUnk5560_c *obj);

// Misc.
BOOL fn_800E593C(void *);
BOOL fn_800DCEDC();
void fn_800B0954(BOOL, int);
BOOL fn_8019B864();

// Letters and events.
u16 fn_800FABF4(int, int);
void fn_800C60B4(dItem::Item *item, int, s32 *, int, void *, int, int, int);
extern u8 lbl_8059FF80[];
void fn_80169C48();
void fn_80169F20();
void fn_80169F4C();
void fn_80169F78();
BOOL fn_80177C90();
BOOL fn_80013550();
void fn_800DD4C8();
void fn_800DD588(int, int);
}

struct SavingsLetter {
    s32 flag;
    s32 threshold;
    s32 kind;
    s32 item;
};

static inline BOOL isValidIndex(int idx) {
    BOOL valid = FALSE;
    if (idx >= 0 && idx < PLAYER_NUM) {
        valid = TRUE;
    }
    return valid;
}

// ---------------------------------------------------------------------------
// dPrivateData_c statics on the current player

// 80136244
BOOL dPrivateData_c::fn_80136244() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (player->isFlag0(0x28)) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }
    if (!player->mPID.isFromTown()) {
        return FALSE;
    }
    if (fn_800DCEDC()) {
        return FALSE;
    }
    if (player->mHost.mFlagA) {
        return FALSE;
    }
    return sLib::isInRange((int)player->mHost.mCount, 1, 7) != FALSE;
}

// 80136310
BOOL dPrivateData_c::fn_80136310() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (player->isFlag0(0x28)) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }
    if (!player->mPID.isFromTown()) {
        return FALSE;
    }
    if (fn_800DCEDC()) {
        return FALSE;
    }

    // TODO: this is likely a class body function thats inlined that returns an int/BOOL
    if ((int)player->mHost.mFlagA != 1) {
        return FALSE;
    }
    return sLib::isInRange((int)player->mHost.mCount, 1, 7) != FALSE;
}

// 801363E0
void dPrivateHost_c::decrease(int n) {
    if (n > 0) {
        s32 count = MAX(0, getCount() - n);

        if (count == 0) {
            mPID.clear();
            setCount(0);
            mFlagA = 0;
            mFlagB = 0;
        } else {
            setCount(count);
        }
    } else if (n < 0) {
        mPID.clear();
        setCount(0);
        mFlagA = 0;
        mFlagB = 0;
    }
}

// 80136460
void dPrivateData_c::saveHostToTown() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mTownHost = player->mHost;
}

// 8013654C
void dPrivateData_c::loadHostFromTown() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dSaveTown_c *town = dSaveData_c::getTown();
    player->mHost = town->mTownHost;
}

// ---------------------------------------------------------------------------
// dPrivateData_c

// 80136638
void dUnkDesignBoard_c::clear() {
    mDesign.initBlank();
    _880 = 2;
    _881 = 0;
    _883 = 0;
    _884 = 0;
    _885 = 0;
    _882 = 0;
    dPrivateData_c::clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x54);
}

// 80136694
BOOL dPrivateData_c::fn_80136694() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (dSaveData_c::getRaw()->mExtra.mNetEnabled == 0) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }
    if (!player->mPID.isFromTown()) {
        return FALSE;
    }
    if (fn_800DCEDC()) {
        return FALSE;
    }
    dSaveExtra_c *extra = dSaveData_c::getExtra();
    if (extra->mDesignBoard._883 == 0) {
        return FALSE;
    }
    return extra->mDesignBoard._882 != 0;
}

// 8013675C
void dUnkDesignBoard_c::decrease(int n) {
    int value = _882;
    if (n > 0) {
        value = MAX(0, value - n);
    }
    _882 = value;
}

// 8013677C
void dUnkDesignBoard_c::fn_8013677C() {
    if (dSaveData_c::getRaw()->mTownHost.mCount != 0) {
        return;
    }
    if (dSaveData_c::getRaw()->mExtra.mNetEnabled == 0) {
        return;
    }
    if (!fn_80177C90()) {
        return;
    }
    if (_882 != 0) {
        return;
    }
    if (_883 != 0) {
        clear();
        _883 = 0;
        _882 = 10;
    } else if (dSaveData_c::getExtra()->mDesignBoard.mDesign.mCreator.isValid()) {
        _882 = 7;
        _883 = 1;
    } else if (cM::rndInt(0x80) == 0) {
        _882 = 7;
        _883 = 1;
    } else {
        _882 = 1;
        _883 = 0;
    }
}

// 80136868
void dUnkDesignBoard_c::fn_80136868() {
    BOOL reset = FALSE;
    u8 keep = 0;
    if (_883 == 0) {
        keep = _882;
    }
    if (dSaveData_c::getRaw()->mExtra.mNetEnabled == 0) {
        reset = TRUE;
    }
    if (!fn_80177C90()) {
        reset = TRUE;
    }
    if (reset) {
        clear();
        _882 = keep;
    }
}

// 801368F4
BOOL dUnkDesignBoard_c::fn_801368F4() {
    return get885() >= 8;
}

// 80136914
dPrivateData_c::dPrivateData_c() {}

// 80136B08
dPrivateData_c::~dPrivateData_c() {}

// 80136C7C
void dPrivateData_c::copy(const dPrivateData_c *other) {
    memcpy((void*)this, other, sizeof(dPrivateData_c));
}

// 80136C88
void dPrivateData_c::updateChecksum() {
    mFriends.updateChecksum();
    mChecksum = calcChecksum();
}

// 80136CC0
BOOL dPrivateData_c::isChecksumValid(int) {
    if (!mFriends.isChecksumOK()) {
        return FALSE;
    }

    if (isChecksumOK()) {
        return TRUE;
    }

    return FALSE;
}

// 80136D1C
u32 dPrivateData_c::calcChecksum() const {
    return sCrc::calcCRC32(&mDates, sizeof(dPrivateData_c) - sizeof(mChecksum) - ((u8 *)&mChecksum - (u8 *)this), -1, -1);
}

// 80136D40
void dPrivateData_c::updateChecksumAll(dPrivateData_c *players) {
    dPrivateData_c *player = players;
    for (int i = 0; i < PLAYER_NUM; i++) {
        player->updateChecksum();
        player++;
    }
}

// 80136D90
BOOL dPrivateData_c::isChecksumValidAll(dPrivateData_c *players, int arg) {
    dPrivateData_c *player = players;
    BOOL valid = TRUE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (!player->isChecksumValid(arg)) {
            valid = FALSE;
            break;
        }
        player++;
    }
    return valid;
}

// 80136E10
BOOL dPrivateData_c::fn_80136E10(dPrivateData_c *players) {
    dPrivateData_c *player = players;
    BOOL result = TRUE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid() && !fn_800E593C(player->mFriends._0004)) {
            result = FALSE;
            break;
        }
        player++;
    }
    return result;
}

// 80136E90
void dPrivateData_c::setFlag0All(dPrivateData_c *players, u32 flag) {
    dPrivateData_c *player = players;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid()) {
            player->setFlag0(flag);
        }
        player++;
    }
}

// 80136F00
void dPrivateData_c::clearFlag0All(dPrivateData_c *players, u32 flag) {
    dPrivateData_c *player = players;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid()) {
            player->clearFlag0(flag);
        }
        player++;
    }
}

// 80136F70
BOOL dPrivateData_c::isFlag0Any(dPrivateData_c *players, u32 flag) {
    dPrivateData_c *player = players;
    BOOL result = FALSE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid() && player->isFlag0(flag)) {
            result = TRUE;
            break;
        }
        player++;
    }
    return result;
}

// 80137000
void dPrivateData_c::setup(const wchar_t *name, u16 id, u8 gender) {
    mPID.setPlayer(name, id, gender);
    fn_8010D7E8((dSaveOption_c *)&_83E8);
    mOrgDesigns.init(&mPID);
    mCatalog.clear();
    _85FA.clear();
    _8604.clear();
    mMotherMail.clear();
    mFriends.clear();
    mDates.init();
    _869A = -1;
    _869B = -1;
    _869C = 0;
    clearFlag0(0x13);
    _55F8 = 0;
    _8694 = 0;
    _8695 = 0;
    _8628 = 0;
    _862C = 0;
    fn_80150AD4(&_5560);
    _55CE.clear();
    _869D[0] = 0;
    _869D[1] = 0;
    _869D[2] = 0;
    _869D[3] = 0;
    _869D[4] = 0;
    memset(_83CC, 0, sizeof(_83CC));
    _83CA = dItem::Item();
}

// 80137110
int dPrivateData_c::findInSave() const {
    dPrivateData_c *player;
    BOOL same;
    dPersonalID_c *pid;

    for (int i = 0; i < PLAYER_NUM; i++) {
        player = getChecked(dSaveData_c::getTown()->mPlayers, i);
        same = FALSE;
        pid = &player->mPID;
        if (mPID.land.mId == pid->land.mId && mPID.land.mRegion == pid->land.mRegion &&
            memcmp(mPID.land.mName, pid->land.mName, sizeof(pid->land.mName)) == 0 &&
            mPID.isSamePlayer(pid)) {
            same = TRUE;
        }
        if (same) {
            return i;
        }
    }
    return -1;
}

// 801371DC
void dPrivateData_c::fn_801371DC() {
    if (isFlag0(0x28)) {
        return;
    }
    dTime_c stamp;
    stamp = _8618.get();
    dTime_c prev = stamp;
    prev.year--;
    dTime_c next = stamp;
    next.year++;
    dTime_c now = *dTime_c::getCurrent();
    next.normalize();
    prev.normalize();
    if (dTime_c::isSameOrBeforeDay(now, prev) == 0) {
        stamp.year = now.year;
        _8618.set(OSCalendarTimeToTicks((OSCalendarTime *)&stamp));
        return;
    }
    if (dTime_c::isSameOrBeforeDay(now, next) != 1) {
        return;
    }
    if (isFlag0(0xD)) {
        return;
    }
    if (isFlag0(0x6C)) {
        return;
    }
    if (_8628 <= 0) {
        return;
    }
    static dMail_c mail;
    static u16 lbl_8074B090 = 0xF;
    static u16 lbl_8074B092 = 0x1100;
    static u32 lbl_8074B094 = 0x159;
    mail.clear();
    mail.setupSystem(&lbl_8074B090, "MAIL_ETC_ATM", (const u8 *)&lbl_8074B092, &mPID, (const int *)&lbl_8074B094);
    mail.setPresent(dItem::Item(dItem::ITEM_IDX_TOWN_HALL_MODEL).mId, 0xFF);
    if (dPostOffice::deliverToPlayer(&mail)) {
        setFlag0(0x6C);
    } else if (dPostOffice::add(&mail)) {
        setFlag0(0x6C);
    }
}

// 8013760C
void dPrivateData_c::fn_8013760C() {
    if (!isFlag0(0x28) && isFlag0(0x4A) && _8628 > 0) {        
        static dMail_c mail;
        static u8 lbl_8074B098 = MAIL_FROM_THANK_YOU;
        s32 range[4];
        u16 b;
        u16 a;

        mail.clear();
        b = fn_800FABF4(3, dTime_c::getCurrentSeason());
        a = cM::rndInt(3) + 1;
        mail.setupSystem(&a, "MAIL_NPC_maigo", &lbl_8074B098, &mPID, (const dItem::Item *)&b);
        dItem::Item present;
        range[0] = 3;
        range[1] = 0x19;
        fn_800C60B4(&present, 1, range, 1, lbl_8059FF80, 0, 0, 0);
        mail.setPresent(present.mId, 0xFF);
        if (dPostOffice::deliverToPlayer(&mail)) {
            clearFlag0(0x4A);
        } else if (dPostOffice::add(&mail)) {
            clearFlag0(0x4A);
        }
    }
}

// 80137788
BOOL dPrivateData_c::sendLetter(u16 kind, const dItem::Item *present, dPrivateData_c *player, int amount) {
    static dMail_c mail;
    static u8 lbl_8074B099 = MAIL_FROM_ABD;
    static u32 lbl_8074B09C = 0x17D;
    mail.clear();
    fn_800CBE0C(0, amount, 5, 9);
    u16 msg = kind;
    mail.setupSystem(&msg, "MAIL_ETC_ATM", &lbl_8074B099, &player->mPID, (const int *)&lbl_8074B09C);
    dItem::Item item = *present;
    if (dItem::isRealItemId(item.mId)) {
        mail.setPresent(item.mId, 0xFF);
    }
    if (dPostOffice::add(&mail)) {
        return TRUE;
    }
    return FALSE;
}

// 80137898
void dPrivateData_c::fn_80137898(int days) {
    if (isFlag0(0x28)) {
        return;
    }
    BOOL closed = dSaveData_c::getTown()->mTimeOffset.isChanged() != 0;
    if (!closed || _862C > 0) {
        int months;
        int savings;
        int pending;

        pending = _862C;
        savings = mSavings;
        if (!closed) {
            dTime_c now = *dTime_c::getCurrent();
            dTime_c start = *dTime_c::getCurrent();
            start.add(-_8628, 0, 0, 0);
            months = now.month - start.month;
            months += (now.year - start.year) * 12;
            if (months > 0) {
                pending += (savings / 200) * months;
            }
        }
        int pay = pending > PRIVATE_BELLS_MAX ? PRIVATE_BELLS_MAX : pending;
        if (pay > 0 && mSavings < PRIVATE_SAVINGS_MAX) {
            int total = savings + pay > PRIVATE_SAVINGS_MAX ? PRIVATE_SAVINGS_MAX : savings + pay;
            BOOL sent = FALSE;
            if (days != 0) {
                dItem::Item present;
                if (sendLetter(1, &present, this, pay)) {
                    sent = TRUE;
                }
            }
            if (sent) {
                setSavings(total);
                _862C = 0;
            } else {
                _862C = pay;
            }
        }
    }

    // @BUG: missing static qualifier
    SavingsLetter letters[13] = {
        {0x3A, 10000, 0xC, 0xE0},       {0x3B, 100000, 2, 0x8EC},      {0x3C, 1000000, 0xD, 0xE1},
        {0x3D, 10000000, 3, 0x8ED},     {0x3E, 100000000, 4, 0x533},   {0x3F, 200000000, 5, 0x534},
        {0x40, 300000000, 6, 0x535},    {0x41, 400000000, 7, 0x536},   {0x42, 500000000, 8, 0x537},
        {0x43, 600000000, 9, 0x538},    {0x44, 700000000, 0xA, 0x539}, {0x45, 999999999, 0xB, 0x8EE},
        {0, 0, -1, 0x533},
    };
    // memcpy(letters, lbl_80476150, sizeof(letters));
    if (days != 0) {
        for (SavingsLetter *letter = letters; letter->threshold != 0; letter++) {
            if (!isFlag0(letter->flag) && mSavings >= letter->threshold) {
                dItem::Item present((int)letter->item);
                if (sendLetter(letter->kind, &present, this, 0)) {
                    setFlag0(letter->flag);
                }
                break;
            }
        }
    }
    if (days > 0 && isFlag0(0x46)) {
        dItem::Item present(dItem::ITEM_IDX_SHOPPING_CARD);
        if (sendLetter(0xE, &present, this, 0)) {
            clearFlag0(0x46);
        }
    }
}

// 80137BCC
void dPrivateData_c::updateLooks(int days) {
    updateTan(days);
    updateHair(days);
}

// 80137C10
void dPrivateData_c::updateTan(int days) {
    if (days != 0) {
        int tan = mTan;
        if (days < 0) {
            if (isFlag0(0x50)) {
                tan += 2;
            }
        } else if (days > 0) {
            if (isFlag0(0x50)) {
                tan += 2 - (days - 1);
            } else {
                tan -= days;
            }
        }
        if (tan >= 16) {
            tan = 15;
        } else if (tan < 0) {
            tan = 0;
        }
        mTan = tan;
        clearFlag0(0x50);
    }
}

// 80137CD0
void dPrivateData_c::fn_80137CD0(int days) {
    if (!fn_80101500()) {
        fn_80101514();
        if (isFlag0(0x28)) {
            return;
        }
    }
    updateTan(days);
}

// 80137D34
void dPrivateData_c::updateHair(int days) {
    if (!fn_80101490()) {
        fn_801014A4();
        if (days >= 15) {
            u8 hair = 0x19;
            if (mHair < 13) {
                hair = 0xC;
            }
            mHair = hair;
            fn_801014D0();
        }
    }
}

// 80137DA4
void dPrivateData_c::dailyUpdate(int days) {
    if ((u8)getCurrentScene() == SCENE_DM_BUS_PL_CRT && dPlayerMgr_c::getCurrentPlayer() == this) {
        return;
    }
    if (days != 0) {
        memset(mFlags1, 0, sizeof(mFlags1));
        _7FA6 = 0;
        if (isFlag0(0x17)) {
            clearFlag0(0x17);
            if (_8692 < 11) {
                _8692++;
            }
        }
        if (isFlag0(0x1A)) {
            clearFlag0(0x1A);
            if (_8691 < 15) {
                _8691++;
            }
        }
        fn_8013C878();
    }
    if (isFlag0(0x72) && dPostOffice::add(&mFutureSelfLetter)) {
        clearFlag0(0x72);
    }
    _83CA = dItem::Item();
    _8628 += days;
    _8630 += days;
    if (dPlayerMgr_c::getCurrentPlayer() == this) {
        fn_800B0954(_8628 != 0, 1);
        updateLooks(_8628);
        fn_80137898(_8628);
        fn_801371DC();
        fn_8013760C();
    }
    if (dPlayerMgr_c::getCurrentPlayer() == this) {
        _8628 = 0;
    }
}

// 80137F58
void dPrivateData_c::setPocket(const dItem::Item *item, int idx, BOOL flag) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm == NULL || !bitm->getKindFlag4()) {
            return;
        }
    }
    dItem::Item pocket = *item;
    if (dItem::isRealItemId(item->mId) && pocket.getKind() != 9) {
        pocket = pocket.withVariant(0);
    }
    mPockets[idx] = pocket;
    if (!flag && pocket.getKind() == 0x24) {
        flag = TRUE;
    }
    setPocketFlag(idx, flag);
    if (!flag && dItem::isRealItemId(item->mId)) {
        mCatalog.registerItem(item->mId, FALSE);
    }
    if (item->mId != dItem::ITEM_ID_NONE) {
        fn_8019B864();
    }
    int kind = pocket.getKind();
    if (kind == 0x3F) {
        setFlag0(0x51);
    } else if (kind == 0x4A) {
        setFlag0(0x52);
    }
}

// 801380EC
int dPrivateData_c::findEmptyPocket(int start) {
    for (int i = start; i < PLAYER_POCKETS_COUNT; i++) {
        if (mPockets[i].mId == dItem::ITEM_ID_NONE) {
            return i;
        }
    }
    return -1;
}

// 8013812C
BOOL dPrivateData_c::pickUp(const dItem::Item *item, BOOL flag) {
    int idx = findEmptyPocket(0);
    if (idx == -1) {
        return FALSE;
    }
    dItem::Item pick = item->getPickItem();
    setPocket(&pick, idx, flag);
    return TRUE;
}

// 801381B4
int dPrivateData_c::countPockets(BOOL (*fn)(const dItem::Item *), u16 *mask) {
    int i;
    int count = 0;
    u16 localMask = 0;
    if (mask == NULL) {
        mask = &localMask;
    }
    *mask = 0;
    for (i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (getPocketFlag(i) == 0) {
            if ((fn != NULL && fn(&mPockets[i]) == TRUE) ||
                (fn == NULL && mPockets[i].mId == dItem::ITEM_ID_NONE)) {
                *mask |= (1 << i);
                count++;
            }
        }
    }
    return count;
}

// 8013828C
int dPrivateData_c::countPocketsFlag(BOOL (*fn)(const dItem::Item *, int), u16 *mask) {
    int i;
    int count = 0;
    u16 localMask = 0;
    if (mask == NULL) {
        mask = &localMask;
    }
    *mask = 0;
    if (fn == NULL) {
        return 0;
    }
    for (i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (fn(&mPockets[i], getPocketFlag(i))) {
            *mask |= (1 << i);
            count++;
        }
    }
    return count;
}

// 8013834C
void dPrivateData_c::setPocketFlag(int idx, int flag) {
    mPocketFlags[idx] = flag;
}

// 8013835C
void dPrivateData_c::fn_8013835C(void *arg, int idx) {
    mLetters[idx].copy((const dMail_c *)arg);
}

// 8013836C
void dPrivateData_c::clearLetter(int idx) {
    mLetters[idx].clear();
}

// 8013837C
int dPrivateData_c::findLetter() {
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        if (mLetters[i].isEmpty()) {
            return i;
        }
    }
    return -1;
}

// 801383DC
int dPrivateData_c::getPocketFlag(int idx) const {
    return mPocketFlags[idx];
}

// 801383EC
void dPrivateData_c::clear() {
    memset(this, 0, sizeof(dPrivateData_c));
    mPID.clear();
    mBells = 0;
    mCatalog.init();
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        mPockets[i] = dItem::Item();
    }
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        mLetters[i].clear();
    }
    mLetterStyle.clear();
    mEquipment.clear();
    _8614.clear();
    _868F = 0;
    _7EEE.clear();
    mDebt = 0;
    _83BE.clear();
    _83C2.clear();
    mSavings = 0;
    mErrand.clear();
    _8620.reset();
    mMotherMail.clear();
    mHost.mPID.clear();
    mHost.mCount = 0;
    mHost.mFlagA = 0;
    mHost.mFlagB = 0;
    _8697 = 0;
    _8698 = 0;
    _8699 = 0;
    _83C6.clear();
    mBirthdayHost.clear();
    mVisitorLetter.clear();
    mValentineYear.clear();
    mNewYearYear.clear();
    _86A5 = 0;
}

// 80138580
BOOL dPrivateData_c::isFlag0(u32 flag) const {
    if (flag < PRIVATE_FLAGS0_NUM) {
        return (mFlags0[(int)flag >> 3] & (1 << (flag & 7))) != 0;
    }
    return FALSE;
}

// 801385C0
void dPrivateData_c::setFlag0(u32 flag) {
    if (flag < PRIVATE_FLAGS0_NUM) {
        mFlags0[(int)flag >> 3] |= (1 << (flag & 7));
    }
}

// 801385F0
void dPrivateData_c::clearFlag0(u32 flag) {
    if (flag < PRIVATE_FLAGS0_NUM) {
        mFlags0[(int)flag >> 3] &= ~(1 << (flag & 7));
    }
}

// 80138620
BOOL dPrivateData_c::isFlag1(u32 flag) {
    if (flag < PRIVATE_FLAGS1_NUM) {
        return (mFlags1[(int)flag >> 3] & (1 << (flag & 7))) != 0;
    }
    return FALSE;
}

// 80138660
void dPrivateData_c::setFlag1(u32 flag) {
    if (flag < PRIVATE_FLAGS1_NUM) {
        mFlags1[(int)flag >> 3] |= (1 << (flag & 7));
    }
}

// 80138690
void dPrivateData_c::clearFlag1(u32 flag) {
    if (flag < PRIVATE_FLAGS1_NUM) {
        mFlags1[(int)flag >> 3] &= ~(1 << (flag & 7));
    }
}

// 801386C0
void dPrivateData_c::setFlag2(u32 flag) {
    if (flag < PRIVATE_FLAGS2_NUM) {
        mFlags2[(int)flag >> 3] |= (1 << (flag & 7));
    }
}

// 801386F0
BOOL dPrivateData_c::isFlag3(u32 flag) {
    if (flag < PRIVATE_FLAGS3_NUM) {
        return (mFlags3[(int)flag >> 3] & (1 << (flag & 7))) != 0;
    }
    return FALSE;
}

// 80138730
void dPrivateData_c::setFlag3(u32 flag) {
    if (flag < PRIVATE_FLAGS3_NUM) {
        mFlags3[(int)flag >> 3] |= (1 << (flag & 7));
    }
}

// 80138760
BOOL dPrivateData_c::fn_80138760() {
    if (isFlag0(0x3A) && !isFlag0(0x46)) {
        BOOL found = FALSE;
        dItem::Item a(dItem::ITEM_IDX_SHOPPING_CARD);
        dItem::Item b(dItem::ITEM_IDX_GOLD_CARD);

        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (mPockets[i] == a || mPockets[i] == b) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80138890
int dPrivateData_c::fn_80138890() const {
    int savings = mSavings;
    if (savings > 0 && isFlag0(0x3A)) {
        const dItem::Item a(dItem::ITEM_IDX_SHOPPING_CARD);
        const dItem::Item b(dItem::ITEM_IDX_GOLD_CARD);
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (mPockets[i] == a || mPockets[i] == b) {
                return savings;
            }
        }
    }
    return 0;
}

// 801389A4
void dPrivateData_c::setSavings(int amount) {
    if (amount > PRIVATE_SAVINGS_MAX) {
        mSavings = PRIVATE_SAVINGS_MAX;
    } else {
        mSavings = amount;
    }
}

// 801389C4
void dPrivateData_c::addSavings(int amount) {
    int savings = mSavings + amount;
    if (savings >= PRIVATE_SAVINGS_MAX) {
        mSavings = PRIVATE_SAVINGS_MAX;
    } else {
        mSavings = savings;
    }
}

// 801389EC
void dPrivateData_c::subSavings(int amount) {
    int savings = mSavings - amount;
    if (savings < 0) {
        mSavings = 0;
    } else {
        mSavings = savings;
    }
}

// 80138A0C
void dPrivateData_c::payDebt(int amount) {
    if (amount != 0) {
        if (mDebt < amount) {
            mDebt = 0;
        } else {
            mDebt -= amount;
        }
    }
}

// 80138A38
void dPrivateData_c::setDebtFromHouse() {
    // 80476220
    static const s32 lbl_80476220[6] = {19800, 120000, 248000, 368000, 598000, 0};

    int house = dSaveData_c::getTown()->mHomes.findOwner(this);
    if (house != -1) {
        u32 size = dSaveData_c::getTown()->mHomes.getHome(house)->mNextSize;
        if (size < 5) {
            mDebt = lbl_80476220[size];
        }
    } else {
        mDebt = 19800;
    }
}

// 80138AC8
int dPrivateData_c::getPocketMoney() const {
    int money = mBells;
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (getPocketFlag(i) == 0 && mPockets[i].isMoney()) {
            money += mPockets[i].getPrice();
        }
    }
    return money;
}

// 80138B58
int dPrivateData_c::getMoneyRoom(int slots) {
    int room = PRIVATE_BELLS_MAX - mBells;
    int bagPrice = dItem::Item(dItem::ITEM_IDX_99000_BELLS).getPrice();
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (getPocketFlag(i) == 0 && mPockets[i].isMoney()) {
            room += bagPrice - mPockets[i].getPrice();
        } else if (mPockets[i].mId == dItem::ITEM_ID_NONE) {
            room += bagPrice;
        }
    }
    return room + slots * bagPrice;
}

// 80138C24
int dPrivateData_c::findMoneyPocketMostRoom() {
    int idx = findEmptyPocket(0);
    int bagPrice = dItem::Item(dItem::ITEM_IDX_99000_BELLS).getPrice();
    if (idx == -1) {
        int i, best;
        for (best = 0, i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (getPocketFlag(i) == 0 && mPockets[i].isMoney()) {
                int room = bagPrice - mPockets[i].getPrice();
                if (room > best) {
                    best = room;
                    idx = i;
                }
            }
        }
    }
    return idx;
}

// 80138CE0
int dPrivateData_c::findMoneyPocketSmallest() {
    int idx = -1;
    int i;
    int smallest = dItem::Item(dItem::ITEM_IDX_99000_BELLS).getPrice() + 1;
    for (i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (getPocketFlag(i) == 0 && mPockets[i].isMoney()) {
            int price = mPockets[i].getPrice();
            if (price < smallest) {
                smallest = price;
                idx = i;
            }
        }
    }
    return idx;
}

// 80138D84
BOOL dPrivateData_c::addMoney(int amount) {
    if (getMoneyRoom(0) < amount) {
        return FALSE;
    }
    int bagPrice = dItem::Item(dItem::ITEM_IDX_99000_BELLS).getPrice();
    int bells = mBells + amount;
    while (bells > PRIVATE_BELLS_MAX) {
        int idx = findMoneyPocketMostRoom();
        dItem::Item old = mPockets[idx];
        dItem::Item money = dItem::Item::getMoneyItem(bagPrice, TRUE, NULL);
        int added = money.getPrice();
        if (old.mId != dItem::ITEM_ID_NONE) {
            added -= old.getPrice();
        }
        setPocket(&money, idx, FALSE);
        bells -= added;
    }
    mBells = bells;
    return TRUE;
}

// 80138E78
BOOL dPrivateData_c::payMoney(int amount, BOOL allowItems) {
    if (getPocketMoney() < amount) {
        return FALSE;
    }
    if (allowItems && dPlayerMgr_c::getCurrentPlayer()->countPockets(NULL, NULL) <= 0) {
        int idx = findMoneyPocketSmallest();
        if (idx == -1) {
            return FALSE;
        }
        dItem::Item money = mPockets[idx];
        if (amount < money.getPrice() && mBells + money.getPrice() - amount > PRIVATE_BELLS_MAX) {
            return FALSE;
        }
        mBells += money.getPrice();
        money = dItem::Item();
        setPocket(&money, idx, FALSE);
    }
    
    while (amount > mBells) {
        int idx = findMoneyPocketSmallest();
        dItem::Item money = mPockets[idx];
        mBells += money.getPrice();
        money = dItem::Item();
        setPocket(&money, idx, FALSE);
    }

    mBells -= amount;
    return TRUE;
}

// 80138FFC
u8 dPrivateData_c::get_86A5() {
    return _86A5 < 5 ? _86A5 : 0;
}

// 80139018
void dPrivateData_c::set_86A5(u8 value) {
    if (value < 5) {
        _86A5 = value;
    }
}

// 8013902C
void dPrivateData_c::fn_8013902C() {
    dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
    const dHome_c *house = static_cast<const dHomeList_c *>(homes)->getHome(homes->findCurrentPlayer());
    if (house != NULL) {
        set_86A5(house->mSize);
    }
}

// 80139090
void dPrivateData_c::addNookPoints(int points) {
    int current = mNookPoints + points;
    if (current > PRIVATE_NOOK_POINTS_MAX) {
        current = PRIVATE_NOOK_POINTS_MAX;
    }
    mNookPoints = current;
    int lifetime = mNookPointsLifetime + points;
    if (lifetime > PRIVATE_NOOK_POINTS_MAX) {
        lifetime = PRIVATE_NOOK_POINTS_MAX;
    }
    mNookPointsLifetime = lifetime;
    if (mNookPointsMax < mNookPoints) {
        mNookPointsMax = mNookPoints;
    }
}

// 801390E8
void dPrivateData_c::subNookPoints(int points) {
    mNookPoints -= points;
}

// 801390F8
void dPrivateData_c::addNookPointsForShop() {
    dLandID_c *town = &dSaveData_c::getTown()->mLandID;
    BOOL same = FALSE;
    if (mPID.land.mId == town->mId && mPID.land.mRegion == town->mRegion &&
        memcmp(mPID.land.mName, town->mName, sizeof(town->mName)) == 0) {
        same = TRUE;
    }
    if (!same) {
        addNookPoints(5);
    } else {
        addNookPoints(3);
    }
    setFlag1(0xA);
}

// 801391A0
void dPrivateData_c::fn_801391A0() {
    _8620.setNow();
    _8620.toDayStart();
}

// 801391E0
BOOL dPrivateData_c::fn_801391E0() {
    if (_8620.isNone()) {
        return TRUE;
    }
    dTimeStamp_c week(_8620.getTicks());
    dTime_c cal;
    cal = _8620.get();
    week.addDays(-cal.wday);
    dTimeStamp_c now(dTime_c::getCurrent());
    dTime_c nowCal;
    nowCal = now.get();
    now.addDays(-nowCal.wday);
    return week.diffDays(now.getTicks(), 1, 0) != 0;
}

// 801392B8
void dPrivateData_c::inc_8694() {
    set_8694(get_8694() + 1);
}

// 801392F4
void dPrivateData_c::set_8694(u8 value) {
    _8694 = value;
    if (_8694 > 10) {
        _8694 = 9;
    }
}

// 80139314
void dPrivateData_c::fn_80139314() {
    if (_8695 > _8694) {
        setFlag0(0x58);
    }
    _8695 = _8694;
}

// 80139364
u8 dPrivateData_c::get_8694() {
    return _8694;
}

// 80139370
u8 dPrivateData_c::get_8695() {
    return _8695;
}

// 8013937C
BOOL dPrivateData_c::fn_8013937C() {
    return isFlag0(0x61);
}

// 80139384
int dPrivateData_c::get_8693() {
    return _8693;
}

// 80139390
void dPrivateData_c::inc_8693() {
    if (_8693 < 50) {
        _8693++;
    }
}

// 801393AC
BOOL dPrivateData_c::fn_801393AC() {
    BOOL result = FALSE;
    if (get_8693() >= 50 && !isFlag0(0x18)) {
        result = TRUE;
    }
    return result;
}

// 80139408
void *dPrivateData_c::fn_80139408() {
    int idx = find(dSaveData_c::getTown()->mPlayers, &mPID);
    if (idx == -1) {
        return NULL;
    }
    return dSaveData_c::getExtra()->_17FF58[idx];
}

// 80139468
void *dPrivateData_c::fn_80139468() {
    int idx = find(dSaveData_c::getTown()->mPlayers, &mPID);
    if (idx == -1) {
        return NULL;
    }
    return &dSaveData_c::getExtra()->mSavedLetters[idx];
}

// 801394D0
void *dPrivateData_c::fn_801394D0() {
    int idx = find(dSaveData_c::getTown()->mPlayers, &mPID);
    if (idx == -1) {
        return NULL;
    }
    return &dSaveData_c::getExtra()->mSavedLetterItems[idx];
}

// 80139530
void *dPrivateData_c::fn_80139530() {
    int idx = find(dSaveData_c::getTown()->mPlayers, &mPID);
    if (idx == -1) {
        return NULL;
    }
    return &dSaveData_c::getExtra()->mSavedPatterns[idx];
}

// 80139594
dMail_c *dPrivateData_c::fn_80139594() {
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        dMail_c* mail = &mLetters[i];
        if (mail != NULL && mail->isValid() && mail->isReadFlaggedInvite() && mail->getFromPlayer()) {
            return mail;
        }
    }

    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        dMail_c* mail = &mLetters[i];
        if (mail != NULL && mail->isValid() && mail->isReadFlaggedInvite() && mail->getFromAnimal()) {
            return mail;
        }
    }
    return NULL;
}

// 8013967C
dMail_c *dPrivateData_c::fn_8013967C() {
    for (int i = 0; i < PLAYER_MAIL_COUNT; i++) {
        dMail_c* mail = &mLetters[i];
        if (mail != NULL && mail->isValid() && mail->isReadUnflaggedInvite()) {
            return mail;
        }
    }
    return NULL;
}

// 801396F4
BOOL dPrivateData_c::fn_801396F4() {
    BOOL result = FALSE;
    if (fn_80139594() != NULL || fn_8013967C() != NULL) {
        result = TRUE;
    }
    return result;
}

// 8013974C
void dPrivateData_c::inc_8696() {
    u32 value = _8696;
    value++;
    if (value >= 9) {
        value = 0;
    }
    _8696 = value;
}

// 80139770
dQuestErrand_c *dPrivateData_c::findErrand(dAnmPersonalID_c *animal, int which, u32 idx) {
    if (!animal->isValid()) {
        return NULL;
    }
    if (!mPID.isValid()) {
        return NULL;
    }
    const dQuestErrandList_c &errands = mErrand;
    const dQuestErrand_c *errand = errands.get(idx);
    if (errand != NULL && errand->mBase.isActive()) {
        for (int i = 0; i < 2; i++) {
            if (which == 2 || which == i) {
                BOOL same;
                BOOL ok;
                const dAnmPersonalID_c *other = errand->getAnimal(i);

                // My suspicion is that this is something like *animal == *other
                // Perhaps this is a const function and it takes const dAnmPersonalID_c& instead of a pointer.
                same = FALSE;
                if (other->mLand.mId == animal->mLand.mId && other->mLand.mRegion == animal->mLand.mRegion &&
                    memcmp(other->mLand.mName, animal->mLand.mName, sizeof(animal->mLand.mName)) == 0 &&
                    other->mLand2.mId == animal->mLand2.mId && other->mLand2.mRegion == animal->mLand2.mRegion &&
                    memcmp(other->mLand2.mName, animal->mLand2.mName, sizeof(animal->mLand2.mName)) == 0 &&
                    other->mNpcIdx == animal->mNpcIdx) {
                    same = TRUE;
                }
                if (same) {
                    ok = TRUE;
                    switch (errand->mBase.mKind) {
                    case 0xC:
                    case 0xE:
                    case 0xF:
                        if (countErrandPockets(NULL) == 0) {
                            ok = FALSE;
                        }
                        break;
                    }
                    if (ok) {
                        return (dQuestErrand_c *)errand;
                    }
                }
            }
        }
    }
    return NULL;
}

// 80139914
BOOL dPrivateData_c::isPocketErrandItem(const dItem::Item *item, int flag) {
    if (item->mId != dItem::ITEM_ID_NONE && flag == 3) {
        return TRUE;
    }
    return FALSE;
}

// 80139938
int dPrivateData_c::countErrandPockets(u16 *mask) {
    return countPocketsFlag(isPocketErrandItem, mask);
}

// 80139948
BOOL dPrivateData_c::fn_80139948(int arg) {
    if (!mPID.isValid()) {
        return FALSE;
    }
    if (!mErrand.isExpired(*(dTime_c *)arg)) {
        return FALSE;
    }
    u16 mask = 0;
    if (countErrandPockets(&mask) == 0 || mask == 0) {
        return FALSE;
    }
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if ((mask >> i) & 1) {
            setPocketFlag(i, 5);
        }
    }
    return TRUE;
}

// 80139A14
void dPrivateDates_c::init() {
    mDate0.set(2000, 0, 1);
    mDate1.set(2000, 0, 1);
    _08 = 0;
    _0C = 0;
}

// 80139A6C
BOOL dPrivateDates_c::fn_80139A6C() {
    if (mDate0.year == 0) {
        mDate0.set(2000, 0, 1);
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour < 6) {
        now.add(-1, 0, 0, 0);
    }
    u16 today = dTheater::getWeek(now);
    dTime_c last = mDate0.get();
    u16 last_day = dTheater::getWeek(last);
    if (today != last_day) {
        return TRUE;
    }
    return FALSE;
}

// 80139C30
BOOL dPrivateDates_c::fn_80139C30(dYMD_c *out) {
    if (mDate1.year == 0) {
        mDate1.set(2000, 0, 1);
    }
    dTime_c cal = getModelRoomDate();
    if (cal.year != mDate1.year || cal.month != mDate1.month || cal.mday != mDate1.day) {
        if (out != NULL) {
            out->set(cal.year, cal.month, cal.mday);
        }
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// dCatalog_c

// 805F2518
static dItem::seeker_c lbl_805F2518;

// 80139D2C
void dCatalog_c::clear() {
    memset(this, 0, sizeof(dCatalog_c));
}

// 80139D38
void dCatalog_c::init() {
    clear();
}

// 80139D3C
void dCatalog_c::registerDefaults(int set) {
    registerItem(dItem::Item(dItem::ITEM_IDX_TAPE_DECK).mId, FALSE);
    registerItem(dItem::Item(dItem::ITEM_IDX_CARDBOARD_BOX).mId, FALSE);
    if (set != -1) {
        u16 items[3];
        items[0] = dItem::ITEM_ID_NONE;
        items[1] = dItem::ITEM_ID_NONE;
        items[2] = dItem::ITEM_ID_NONE;
        dHomeRoom_c::getDefaultItems(items, set, 0);
        registerItem(items[0], FALSE);
        registerItem(items[1], FALSE);
        registerItem(items[2], FALSE);
    }
}

// 80139E04
u16 dCatalog_c::getNth(int kind, u32 n, dItem::seeker_c::candCB_c *cb) {
    if (kind == dItem::KIND_CAP) {
        static const s32 lbl_80476238[4][2] = {{5, 3}, {6, 3}, {5, 5}, {6, 5}};

        u32 total = 0;
        for (const s32(*entry)[2] = lbl_80476238; entry < lbl_80476238 + 4; entry++) {
            lbl_805F2518.search((*entry)[0], (*entry)[1], cb);
            u32 start = total;
            total += lbl_805F2518.mCount;
            if (n < total) {
                return lbl_805F2518.getNth(n - start).mId;
            }
        }
    }
    lbl_805F2518.search(kind, 7, cb);
    return lbl_805F2518.getNth(n).mId;
}

// 80139ED8
u16 dCatalog_c::getNthRegistered(int kind, u32 n) {
    catalogCandCB_c cb(this, TRUE);
    return getNth(kind, n, &cb);
}

// 80139F20
u16 dCatalog_c::getNthAny(int kind, u32 n) {
    catalogCandCB_c cb(this, FALSE);
    return getNth(kind, n, &cb);
}

// 80139F68
BOOL dCatalog_c::isNthRegistered(int kind, u32 n) {
    return isRegistered(getNthAny(kind, n));
}

// 80139FA4
bool dCatalog_c::isRegistered(u16 item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(item));
    if (bitm != NULL && bitm->m_catalogStore) {
        u16 baseId = dItem::infoBank_c::get()->getBaseIdFromItemId(item);
        if (baseId < CATALOG_BIT_NUM) {
            return getBit(baseId) != 0;
        }
    }
    return FALSE;
}

// 8013A03C
BOOL dCatalog_c::isRegistered(const dItem::Item &item) {
    return isRegistered(item.mId);
}

// 8013A044
BOOL dCatalog_c::registerItem(u16 item, BOOL force) {
    if (!force) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(item));
        int kind = bitm != NULL ? bitm->getKind() : dItem::KIND_NONE;
        if (kind == dItem::KIND_INSECT || kind == dItem::KIND_FISH) {
            return FALSE;
        }
    }
    u16 baseId = dItem::infoBank_c::get()->getBaseIdFromItemId(item);
    if (baseId < CATALOG_BIT_NUM) {
        setBit(baseId);
        return TRUE;
    }
    return FALSE;
}

// 8013A10C
u32 dCatalog_c::countRegistered(int kind) {
    catalogCandCB_c cb(this, TRUE);
    if (kind == dItem::KIND_CAP) {
        lbl_805F2518.search(dItem::KIND_CAP, 7, &cb);
        u32 count = lbl_805F2518.mCount;
        lbl_805F2518.search(dItem::KIND_ACC, 7, &cb);
        return count + lbl_805F2518.mCount;
    }
    lbl_805F2518.search(kind, 7, &cb);
    return lbl_805F2518.mCount;
}

// 8013A1B4
u32 dCatalog_c::countAll(int kind) {
    catalogCandCB_c cb(this, FALSE);
    if (kind == dItem::KIND_CAP) {
        lbl_805F2518.search(dItem::KIND_CAP, 7, &cb);
        u32 count = lbl_805F2518.mCount;
        lbl_805F2518.search(dItem::KIND_ACC, 7, &cb);
        return count + lbl_805F2518.mCount;
    }
    lbl_805F2518.search(kind, 7, &cb);
    return lbl_805F2518.mCount;
}

// 8013A25C
BOOL dCatalog_c::isComplete(int kind) {
    return countAll(kind) == countRegistered(kind);
}

// ---------------------------------------------------------------------------
// dPrivateBits85FA_c

// 8013A2B8
void dPrivateBits85FA_c::clear() {
    for (int i = 0; i < 10; i++) {
        mBits[i] = 0;
    }
}

// 8013A2E8
BOOL dPrivateBits85FA_c::isSet(const dItem::Item &item) {
    return isSetIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013A330
void dPrivateBits85FA_c::set(const dItem::Item &item) {
    setIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013A378
void dPrivateBits85FA_c::reset(const dItem::Item &item) {
    resetIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013A3C0
BOOL dPrivateBits85FA_c::isSetIdx(u32 idx) {
    return (mBits[idx >> 3] >> (idx & 7)) & 1;
}

// 8013A3D8
void dPrivateBits85FA_c::setIdx(u32 idx) {
    mBits[idx >> 3] |= (1 << (idx & 7));
}

// 8013A3F8
void dPrivateBits85FA_c::resetIdx(u32 idx) {
    mBits[idx >> 3] &= ~(1 << (idx & 7));
}

// ---------------------------------------------------------------------------
// dDesignOrder_c / dDesignList_c

// 8013A418
dDesignOrder_c::dDesignOrder_c() {}

// 8013A41C
dDesignOrder_c::~dDesignOrder_c() {}

// 8013A45C
void dDesignOrder_c::init() {
    u8 v = 0;
    for (s32 i = 0; i < PLAYER_ORG_DESIGN_COUNT; i++) {
        mOrder[i] = v++;
    }
}

// 8013A4A0
void dDesignOrder_c::swap(u32 a, u32 b) {
    u8 tmp = mOrder[a & 7];
    mOrder[a & 7] = mOrder[b & 7];
    mOrder[b & 7] = tmp;
}

// 8013A4BC
u8 dDesignOrder_c::get(u32 i) {
    return mOrder[i & 7] & 7;
}

// 8013A4CC
void dDesignList_c::init(dPersonalID_c *creator) {
    dDesign_c *design;
    for (u32 i = 0; i < PLAYER_ORG_DESIGN_COUNT; i++) {
        design = &mDesigns[i & 7];
        mDesigns[i & 7].clear();
        mDesigns[i & 7].setFromItem((i & 7) + 0x9CC);
        design->mCreator = *creator;
        design->loadTextureA(i);
    }
    mOrder.init();
}

// 8013A5F4
dDesign_c *dDesignList_c::getDesign(u32 i) {
    return &mDesigns[mOrder.get(i)];
}

// 8013A630
void dDesignList_c::setDesign(u32 i, dDesign_c *design) {
    u32 idx = mOrder.get(i) & 7;
    mDesigns[idx] = *design;
    DCStoreRangeNoSync(&mDesigns[idx], sizeof(dDesign_c));
}

// 8013A880
u8 dDesignList_c::getOrder(u32 i) {
    return mOrder.get(i);
}

// 8013A888
u16 dDesignList_c::fn_8013A888(u32 i, int kind) {
    int order = getOrder(i);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dItem::seeker_c::get()->search(kind, 6, NULL);
    if (player != NULL) {
        int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, &player->mPID);
        if (idx != -1) {
            return dItem::seeker_c::get()->getNth(order + idx * 8).mId;
        }
    }
    return dItem::seeker_c::get()->getNth(order).mId;
}

// ---------------------------------------------------------------------------
// dEquip_c

// 8013A940
dEquip_c::dEquip_c() {
    clear();
}

// 8013A988
dEquip_c::~dEquip_c() {}

// 8013A9C8
BOOL dEquip_c::fn_8013A9C8() {
    if (mAcc.mId == dItem::ITEM_ID_NONE) {
        return TRUE;
    }
    int bone = mHat.getHideBone();
    if (bone == 2 || bone == 4 || (u32)(bone - 6) <= 1) {
        return FALSE;
    }
    return TRUE;
}

// 8013AA28
void dEquip_c::setFromPlayer() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    mHeld = player->mEquipment.mHeld;
    mShirt = player->mEquipment.mShirt;
    mHat = player->mEquipment.mHat;
    mAcc = player->mEquipment.mAcc;
    if (mShirt.mId == dItem::ITEM_ID_NONE) {
        mShirt = dItem::Item(dItem::ITEM_IDX_WORK_UNIFORM);
    }
}

// 8013AA90
void dEquip_c::clear() {
    mHeld = dItem::Item();
    mShirt = dItem::Item();
    mHat = dItem::Item();
    mAcc = dItem::Item();
}

// ---------------------------------------------------------------------------
// dPrivateSlots_c

// 8013AAAC
BOOL dPrivateSlots_c::isValid(int i) {
    u8 slot = mSlots[i];
    return slot != 0xFF && slot < 0x1E;
}

// 8013AACC
u8 dPrivateSlots_c::get(int i) {
    return mSlots[i];
}

// 8013AAD4
void dPrivateSlots_c::set(int i, u8 value) {
    mSlots[i] = value;
}

// 8013AADC
void dPrivateSlots_c::clear() {
    mSlots[0] = 0xFF;
    mSlots[1] = 0xFF;
    mSlots[2] = 0xFF;
    mSlots[3] = 0xFF;
}

// 8013AAF4
int dPrivateSlots_c::findEmpty() {
    for (int i = 0; i < 4; i++) {
        if (!isValid(i)) {
            return i;
        }
    }
    return -1;
}

// 8013AB54
dUnk83ED_c::dUnk83ED_c() {}

// 8013AB58
dUnk83ED_c::~dUnk83ED_c() {}

// ---------------------------------------------------------------------------
// dOutfit_c

// 8013AB98
dOutfit_c::dOutfit_c() {
    clear();
}

// 8013ABD0
dOutfit_c::~dOutfit_c() {}

// 8013AC28
void dOutfit_c::clear() {
    mEquip.clear();
    mHairColor = 0;
    mHair = 0xD;
    mShoeColor = 4;
    mFlagByte &= ~7;
}

// 8013AC78
void dOutfit_c::setDefault() {
    clear();
    mEquip.mShirt = dItem::Item(dItem::ITEM_IDX_GRACIES_TOP);
    mEquip.mHat = dItem::Item(dItem::ITEM_IDX_GRACIE_HAT);
    mEquip.mHeld = dItem::Item();
    mEquip.mAcc = dItem::Item();
    mFlags.valid = true;
}

// 8013ACE8
void dOutfit_c::setFromPlayer() {
    clear();
    mEquip.setFromPlayer();
    mHairColor = dPlayerMgr_c::getCurrentPlayer()->mHairColor;
    mHair = dPlayerMgr_c::getCurrentPlayer()->mHair;
    mShoeColor = dPlayerMgr_c::getCurrentPlayer()->mShoeColor;
    mFlags.valid = true;
    mFlags.male = false;
    if (dPlayerMgr_c::getCurrentPlayer()->mPID.player.mGender == GENDER_MALE) {
        mFlags.male = true;
    }
    mFlags.noShoes = false;
    if (dPlayerMgr_c::getCurrentPlayer()->_83F7 == 0) {
        mFlags.noShoes = true;
    }
}

// 8013ADA4
void dOutfit_c::copy(const dOutfit_c *other) {
    memcpy(this, other, sizeof(dOutfit_c));
}

// 8013ADAC
BOOL dOutfit_c::isSame(const dOutfit_c *other) const {
    BOOL same = mFlags.valid;
    if (same) {
        same = other->mFlags.valid;
    }
    if (same) {
        same = mEquip.mAcc.isSame(other->mEquip.mAcc);
    }
    if (same) {
        same = mEquip.mHat.isSame(other->mEquip.mHat);
    }
    if (same) {
        same = mEquip.mShirt.isSame(other->mEquip.mShirt);
    }
    if (same) {
        same = mShoeColor == other->mShoeColor;
    }
    if (same) {
        same = hasShoes() == other->hasShoes();
    }
    return same;
}

// ---------------------------------------------------------------------------
// dPrivateData_c statics over the save players

// 8013AEC0
void dPrivateData_c::clearAll(dPrivateData_c *players) {
    for (int i = 0; i < PLAYER_NUM; i++) {
        clearPlayer(players, i);
    }
}

// 8013AF0C
void dPrivateData_c::clearPlayer(dPrivateData_c *players, int idx) {
    players[idx].clear();
    dSaveData_c::getExtra()->mSavedLetters[idx].clear();
    dSaveData_c::getExtra()->mSavedLetterItems[idx].clear();
    dSaveData_c::getExtra()->mSavedPatterns[idx].init();
    fn_8013EE54(dSaveData_c::getExtra()->_17FF58[idx]);
}

// 8013AFB8
int dPrivateData_c::find(dPrivateData_c *players, const dPersonalID_c *pid) {
    if ((u32)pid->isValid() == TRUE) {
        BOOL same;
        const dPersonalID_c* other;
        for (int i = 0; i < PLAYER_NUM; i++) {
            other = &players->mPID;
            same = FALSE;
            if (pid->land.mId == other->land.mId && pid->land.mRegion == other->land.mRegion &&
                memcmp(pid->land.mName, other->land.mName, sizeof(other->land.mName)) == 0 &&
                pid->isSamePlayer(other)) {
                same = TRUE;
            }
            if (same) {
                return i;
            }
            players++;
        }
    }
    return -1;
}

// 8013B080
int dPrivateData_c::findByPlayerID(dPrivateData_c *players, const dPlayerID_c *id) {
    if ((u32)id->isValid() == TRUE) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (id->isSame(&players->mPID.player)) {
                return i;
            }
            players++;
        }
    }
    return -1;
}

// 8013B104
int dPrivateData_c::findEmpty(dPrivateData_c *players) {
    dPrivateData_c *player = players;
    int result = -1;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (!player->mPID.isValid()) {
            result = i;
            break;
        }
        player++;
    }
    return result;
}

// 8013B174
int dPrivateData_c::count(dPrivateData_c *players) {
    dPrivateData_c *player = players;
    int count = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid()) {
            count++;
        }
        player++;
    }
    return count;
}

// 8013B1E0
dPrivateData_c *dPrivateData_c::get(dPrivateData_c *players, const dPersonalID_c *pid) {
    return getChecked(players, find(players, pid));
}

// 8013B218
dPrivateData_c *dPrivateData_c::getChecked(dPrivateData_c *players, int idx) {
    if (isValidIndex(idx)) {
        dSvMgr_c::isPlayerTransferComplete(idx);
        return &players[idx];
    }
    return NULL;
}

// 8013B28C
dPrivateData_c *dPrivateData_c::getRaw(dPrivateData_c *players, int idx) {
    if (isValidIndex(idx)) {
        return &players[idx];
    }
    return NULL;
}

// 8013B2C8
int dPrivateData_c::getNthValid(dPrivateData_c *players, u32 n) {
    dPrivateData_c *player = players;
    int result = -1;
    u32 count = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid()) {
            if (n == count) {
                result = i;
                break;
            }
            count++;
        }
        player++;
    }
    return result;
}

// 8013B344
BOOL dPrivateData_c::contains(dPrivateData_c *players, const dPersonalID_c *pid) {
    dPrivateData_c *player = players;
    BOOL result = FALSE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (player->mPID.isValid() && player->mPID.isSamePlayer(pid)) {
            result = TRUE;
            break;
        }
        player++;
    }
    return result;
}

// 8013B3D4
dQuestErrand_c *dPrivateData_c::findErrandAll(dPrivateData_c *players, dAnmPersonalID_c *animal, int which,
                                              u32 idx) {
    if (!animal->isValid()) {
        return NULL;
    }
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = getRaw(players, i);
        if (player != NULL) {
            dQuestErrand_c *errand = player->findErrand(animal, which, idx);
            if (errand != NULL) {
                return errand;
            }
        }
    }
    return NULL;
}

// 8013B474
BOOL dPrivateData_c::fn_8013B474(dPrivateData_c *players, int arg) {
    BOOL result = FALSE;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = getChecked(players, i);
        if (player != NULL && player->fn_80139948(arg)) {
            result = TRUE;
        }
    }
    return result;
}

// 8013B4F8
int dPrivateData_c::fn_8013B4F8() {
    int counts[3];
    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player->mPID.isValid()) {
            counts[player->_869C]++;
        }
    }
    if (counts[1] > counts[2]) {
        return 1;
    }
    if (counts[2] > counts[1]) {
        return 2;
    }
    return cM::rndInt(2) == 0 ? 1 : 2;
}

// 8013B5CC
int dPrivateData_c::fn_8013B5CC() {
    int ties[PLAYER_NUM];
    u8 votes[7 * 7];
    memset(votes, 0, sizeof(votes));
    int best = 0;
    int bestIdx = -1;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player->mPID.isValid()) {
            s8 x = player->_869A;
            s8 y = player->_869B;
            if (x >= 0 && y >= 0) {
                int idx = x * 7 + y;
                u8 count = ++votes[idx];
                if (count > best) {
                    bestIdx = idx;
                    best = count;
                }
            }
        }
    }
    if (best == 0) {
        return -1;
    }
    int num = 0;
    for (int i = 0; i < 7 * 7; i++) {
        if (votes[i] == best) {
            ties[num++] = i;
        }
    }
    if (num > 1) {
        bestIdx = ties[cM::rndInt(num)];
    }
    return bestIdx;
}

// 8013B7A4
void dPrivateData_c::fn_8013B7A4() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player->mPID.isValid()) {
            player->_869A = -1;
            player->_869B = -1;
            player->_869C = 0;
            player->clearFlag0(0x13);
        }
    }
    dSaveData_c::getTown()->mPublicWorksDays = 0xFF;
}

// 8013B848
void dPrivateData_c::fn_8013B848() {
    int x = -1;
    int y = -1;
    int idx = fn_8013B5CC();
    if (idx != -1) {
        x = idx / 7;
        y = idx % 7;
    } else {
        dFdBase_c *map = fn_80190C44(1);
        int count = 0;
        for (int j = 1; j < map->mBlockH - 1; j++) {
            for (int i = 1; i < map->mBlockW - 1; i++) {
                if (map->isBlockVariant(i, j)) {
                    count++;
                }
            }
        }
        int pick = cM::rndInt(count);
        int n = 0;
        for (int j = 1; j < map->mBlockH - 1; j++) {
            for (int i = 1; i < map->mBlockW - 1; i++) {
                if (map->isBlockVariant(i, j)) {
                    if (n == pick) {
                        x = i;
                        y = j;
                    } else {
                        n++;
                    }
                }
            }
        }
    }
    if (x >= 0 && y >= 0) {
        fn_800EBB24(2, "BBS_office", 0, 0);
        dSaveData_c::getTown()->mMainField.setBridgeBlock(x, y);
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE);
        fn_8013B7A4();
        setFlag0All(dSaveData_c::getTown()->mPlayers, 0x7F);
    }
}

// 8013B9F8
void dPrivateData_c::fn_8013B9F8() {
    fn_800EBB24(3, "BBS_office", 0, 0);
    fn_80169F78();
    dSaveData_c::getTown()->clearFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE);
    fn_8013B7A4();
    setFlag0All(dSaveData_c::getTown()->mPlayers, 0x7F);
}

// 8013BA50
void dPrivateData_c::fn_8013BA50() {
    int x, y;
    dItem::Item id1((u16)BUILDING_WINDMILL);
    BOOL a = dSaveBuildingList_c::get()->getPos(&x, &y, &id1, 1);
    dItem::Item id2((u16)BUILDING_LIGHTHOUSE);
    BOOL b = dSaveBuildingList_c::get()->getPos(&x, &y, &id2, 1);
    if ((u8)fn_8013B4F8() == 1) {
        if (!a) {
            if (b) {
                fn_800EBB24(9, "BBS_office", 0, 0);
            } else {
                fn_800EBB24(6, "BBS_office", 0, 0);
            }
            clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x6B);
            fn_80169F20();
        }
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE);
        fn_8013B7A4();
        setFlag0All(dSaveData_c::getTown()->mPlayers, 0x7F);
    } else {
        if (!b) {
            if (a) {
                fn_800EBB24(0xA, "BBS_office", 0, 0);
            } else {
                fn_800EBB24(5, "BBS_office", 0, 0);
            }
            clearFlag0All(dSaveData_c::getTown()->mPlayers, 0x6A);
            fn_80169F4C();
        }
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE);
        fn_8013B7A4();
        setFlag0All(dSaveData_c::getTown()->mPlayers, 0x7F);
    }
}

// 8013BBCC
void dPrivateData_c::fn_8013BBCC() {
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE)) {
        switch (dSaveData_c::getPublicWorks()) {
        case 0:
            fn_8013B848();
            break;
        case 1:
            fn_8013B9F8();
            break;
        case 2:
        case 3:
            fn_8013BA50();
            break;
        }
    }
    fn_80169C48();
}

// 8013BC38
void dPrivateData_c::fn_8013BC38(int days) {
    if (days < 1) {
        return;
    }
    if (dSaveData_c::getRaw()->mPublicWorksDays != 0xFF) {
        int left = dSaveData_c::getRaw()->mPublicWorksDays - days;
        left = left < 0 ? 0 : left;
        dSaveData_c::getTown()->mPublicWorksDays = left;
        if (left == 0) {
            fn_8013BBCC();
        }
    }
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED)) {
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_PUBLIC_WORKS_FUNDED);
        dSaveData_c::getTown()->setFlag(SAVE_FLAG_PUBLIC_WORKS_VOTE);
        switch (dSaveData_c::getPublicWorks()) {
        case 0:
            fn_800EBB24(1, "BBS_office", 0, 0);
            break;
        case 2:
            fn_800EBB24(4, "BBS_office", 0, 0);
            break;
        case 3: {
            int x, y;
            dItem::Item id((u16)BUILDING_WINDMILL);
            if (dSaveData_c::getRaw()->mBuilding.mList.getPos(&x, &y, &id, 1)) {
                fn_800EBB24(8, "BBS_office", 0, 0);
            } else {
                fn_800EBB24(7, "BBS_office", 0, 0);
            }
            break;
        }
        }
    }
}

// 8013BDA0
BOOL dPrivateData_c::fn_8013BDA0(int chance, int count, int flag) {
    dSaveTown_c *town = dSaveData_c::getTown();
    if (isFlag0Any(dSaveData_c::getTown()->mPlayers, 0x4B)) {
        return FALSE;
    }
    if (isFlag0(0x4C)) {
        return FALSE;
    }
    dPrivateHost_c *townHost = &town->mTownHost;
    if (townHost->mCount != 0) {
        return FALSE;
    }
    if (mHost.mCount != 0) {
        return FALSE;
    }
    if (cM::rndInt(chance) != 0) {
        return FALSE;
    }
    int coin = cM::rndInt(2);
    townHost->mFlagA = coin == 0;
    townHost->mCount = count;
    townHost->mFlagC = flag;
    townHost->mPID = mPID;
    mHost.mFlagA = coin - 1 == 0;
    dPrivateData_c *host = NULL;
    if (flag == 1) {
        host = dPlayerMgr_c::getCurrentPlayer();
    } else {
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player->mPID.isValid()) {
                host = player;
            }
        }
    }
    mHost.mPID = host->mPID;
    mHost.mCount = count;
    mHost.mFlagC = flag;
    return TRUE;
}

// ---------------------------------------------------------------------------
// dItemPairRing_c

// 8013C054
void dItemPairRing_c::clear() {
    for (int i = 0; i < 8; i++) {
        mA[i] = 0xFFFF;
        mB[i] = 0xFFFF;
    }
    mCount = 0;
    mIndex = -1;
    _22_7 = 0;
    _22_6 = 0;
    _22_5 = 0;
    _21_5 = 0;
    _21_4 = 0;
    _21_3 = 0;
    _21_2 = 0;
    _21_1 = 0;
}

// 8013C0C0
void dItemPairRing_c::fn_8013C0C0() {
    _21_5 = 1;
    _21_2 = 1;
}

// 8013C0D0
void dItemPairRing_c::push(const dItem::Item *a, const dItem::Item *b) {
    mIndex++;
    if (mIndex >= 8) {
        mIndex = 0;
    }
    mA[mIndex] = *a;
    mB[mIndex] = *b;
    _21_4 = 1;
    _21_1 = 1;
    if (!_22_6) {
        _22_7 = 1;
    }
    _22_6 = 1;
    mCount = 0;
    dSaveTown_c *town = dSaveData_c::getTown();
    town->_06673F = 1;
    if (fn_80013550()) {
        fn_800DD4C8();
        fn_800DD588(0x4B, 4);
    }
}

// 8013C1A0
void dItemPairRing_c::fn_8013C1A0() {
    if (mCount < 3) {
        mCount++;
    }
}

// 8013C1C0
BOOL dItemPairRing_c::contains(const dItem::Item *a, const dItem::Item *b) {
    if (mIndex < 0) {
        return FALSE;
    }
    for (int i = 0; i < 8; i++) {
        if (mA[i].isSame(*a) && mB[i].isSame(*b)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8013C25C
void dItemPairRing_c::fn_8013C25C() {
    _21_3 = 1;
    _21_1 = 0;
    _22_7 = 0;
    fn_8013C27C();
}

// 8013C27C
void dItemPairRing_c::fn_8013C27C() {
    _21_2 = 0;
    _21_0 = 0;
    _22_5 = 0;
    mCount = 0;
}

// 8013C2A0
u8 dPrivateData_c::get_869D(int i) {
    return _869D[i];
}

// 8013C2B0
void dPrivateData_c::inc_869D(int i) {
    if (_869D[i] < 0xFF) {
        _869D[i]++;
    }
}

// 8013C2D0
BOOL dPrivateData_c::isBirthday(const dTime_c& cal) const {
    if (mBirthday.isSame(1, 29) && !dTime_c::isLeapYear(cal.year) && cal.month == 1 && cal.mday == 28) {
        return TRUE;
    }
    
    if (mBirthday.isSame(cal.month, cal.mday)) {
        return TRUE;
    }

    return FALSE;
}

// ---------------------------------------------------------------------------
// dUnk55FC_c / dAnimalItem_c / dUnk8634_c

// 8013C3A0
dUnk55FC_c::dUnk55FC_c() {}

// 8013C3A4
dUnk55FC_c::~dUnk55FC_c() {}

// 8013C3E4
void dUnk55FC_c::clear() {
    _00 = -1;
    _04 = -1;
    _08 = -1;
}

// 8013C3F8
dAnimalItem_c::dAnimalItem_c() {}

// 8013C408
dAnimalItem_c::~dAnimalItem_c() {}

// 8013C448
void dAnimalItem_c::clear() {
    mAnimal.clear();
    mItem = dItem::Item();
}

// 8013C480
BOOL dAnimalItem_c::isValid() {
    return mAnimal.isValid() != FALSE;
}

// 8013C4AC
void dAnimalItem_c::set(const dAnmPersonalID_c *animal, const dItem::Item *item) {
    mAnimal.copy(animal);
    mItem = *item;
}

// 8013C4EC
dUnk8634_c::dUnk8634_c() {}

// 8013C4F0
dUnk8634_c::~dUnk8634_c() {}

// 8013C530
void dUnk8634_c::clear() {
    mValue = -1;
}

// ---------------------------------------------------------------------------
// dPrivateBits8604_c

// 8013C53C
void dPrivateBits8604_c::clear() {
    for (int i = 0; i < 16; i++) {
        mBits[i] = 0;
    }
}

// 8013C584
BOOL dPrivateBits8604_c::isSet(const dItem::Item &item) {
    return isSetIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013C5CC
void dPrivateBits8604_c::set(const dItem::Item &item) {
    setIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013C614
void dPrivateBits8604_c::reset(const dItem::Item &item) {
    resetIdx(dItem::seeker_c::get()->findLike(item));
}

// 8013C65C
BOOL dPrivateBits8604_c::isSetIdx(u32 idx) const {
    return (mBits[idx >> 3] >> (idx & 7)) & 1;
}

// 8013C674
void dPrivateBits8604_c::setIdx(u32 idx) {
    mBits[idx >> 3] |= (1 << (idx & 7));
}

// 8013C694
void dPrivateBits8604_c::resetIdx(u32 idx) {
    mBits[idx >> 3] &= ~(1 << (idx & 7));
}

// 8013C6B4
int dPrivateBits8604_c::count() const {
    u32 i;
    int count = 0;
    for (i = 0; i < 0x7F; i++) {
        if (isSetIdx(i)) {
            count++;
        }
    }
    return count;
}

// 8013C71C
BOOL dPrivateData_c::fn_8013C71C() const {
    BOOL result = FALSE;
    if (((mHair == 0xC || mHair == 0x19) != 0 && _83F5 == 0) != 0) {
        int bone = mEquipment.mHat.getHideBone();
        if ((bone == 2 || (u32)(bone - 3) <= 3) == FALSE) {
            result = TRUE;
        }
    }
    return result;
}

// 8013C7C0
u8 dPrivateData_c::get_86A3() {
    return _86A3;
}

// 8013C7CC
BOOL dPrivateData_c::fn_8013C7CC() const {
    if (_86A4 < 3) {
        return FALSE;
    }
    if (!isFlag0(0x73)) {
        return FALSE;
    }
    if (!isFlag0(0x74)) {
        return FALSE;
    }
    if (_8692 < 11) {
        return FALSE;
    }
    return dSaveData_c::getTown()->mShops.mShop.mCountdown != 0x7F;
}

// 8013C878
void dPrivateData_c::fn_8013C878() {
    u8 step = _86A3;
    switch (step) {
    case 0:
        if (fn_8013C7CC()) {
            _86A3++;
        }
        break;
    case 1:
        if (isFlag0(0x79) && isFlag0(0x7A)) {
            _86A3++;
            clearFlag0(0x79);
            clearFlag0(0x7A);
        }
        break;
    case 2:
        if (isFlag0(0x7D) && isFlag0(0x7B) && isFlag0(0x7C)) {
            _86A3++;
            clearFlag0(0x7D);
            clearFlag0(0x7B);
            clearFlag0(0x7C);
        }
        break;
    case 3:
    case 5:
        if (isFlag0(0x78)) {
            _86A3++;
            clearFlag0(0x78);
        }
        break;
    case 4:
    case 7:
        if (isFlag0(0x79)) {
            _86A3++;
            clearFlag0(0x79);
        }
        break;
    case 6:
        if (isFlag0(0x7B) && isFlag0(0x7C)) {
            _86A3++;
            clearFlag0(0x7B);
            clearFlag0(0x7C);
        }
        break;
    case 8:
        if (isFlag0(0x7A)) {
            _86A3++;
            clearFlag0(0x7A);
        }
        break;
    case 9:
        if (isFlag0(0x7B)) {
            _86A3++;
            clearFlag0(0x7B);
        }
        break;
    }
    if (step != _86A3) {
        clearFlag0(0x80);
    }
}

// 8013CAD8
dOutfit_c *dPrivateData_c::getOutfit(int i) {
    return &mOutfits[i];
}

// 8013CAE8
BOOL catalogCandCB_c::check(const dItem::BITM *, dItem::Item *item) const {
    if (mpCatalog != NULL) {
        if (mOnlyRegistered == TRUE) {
            return mpCatalog->isRegistered(item->mId);
        }
        return TRUE;
    }
    return FALSE;
}
