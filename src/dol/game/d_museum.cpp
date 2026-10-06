// Museum donation record (AC m_museum_display.c counterpart). See notes/d_museum.txt.
// .text 80119478..8011A6C4, .ctors 80465724, .data 804EE4B0..804EE4C0 (sender label),
// .bss 805F0F18..805F1420, .sdata 8074B048..8074B050, .sbss 8074E700..8074E708.
#include <game/game/d_museum.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_item.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
BOOL fn_801029C0(dMail_c *mail); // 801029C0: deliver to the mailbox
BOOL fn_80102BBC(dMail_c *mail); // 80102BBC: hold at the post office
}

// 805F0F18: lists the items of a kind for countDonated / getNthDonated.
static dItem::seeker_c sSeeker;

static inline dPrivateData_c *getTownPlayer(int idx) {
    return dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, idx);
}

// The nibble of item idx in a donor array.
static inline int getNibble(const u8 *donors, int idx) {
    return (donors[idx >> 1] >> ((idx & 1) << 2)) & 0xF;
}

// 80119478
void dMuseum_c::init() {
    clear();
}

// 8011947C
void dMuseum_c::sendCompleteMail() {
    if (!isComplete()) {
        return;
    }
    static dMail_c sMail;
    static u16 sKind = 1;
    static u16 sSender = 0x400;
    static u32 sPaper = 0x14C;
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (!getTownPlayer(i)->mPID.isValid() || getTownPlayer(i)->isFlag0(0xD) ||
            getTownPlayer(i)->isFlag0(0x2F)) {
            continue;
        }
        sMail.setupSystem(&sKind, "MAIL_NPC_hu-ta", (const u8 *)&sSender, &getTownPlayer(i)->mPID,
                          (const int *)&sPaper);
        sMail.setPresent(dItem::Item(dItem::ITEM_IDX_MUSEUM_MODEL).mId, 0xFF);
        if (fn_801029C0(&sMail)) {
            getTownPlayer(i)->setFlag0(0x2F);
        } else if (fn_80102BBC(&sMail)) {
            getTownPlayer(i)->setFlag0(0x2F);
        }
    }
}

// 8011960C
dMuseum_c::dMuseum_c() {
    clear();
}

// 80119654
void dMuseum_c::clear() {
    int i;
    for (i = 0; i < (int)sizeof(mFossil); i++) {
        mFossil[i] = 0;
    }
    for (i = 0; i < (int)sizeof(mFish); i++) {
        mFish[i] = 0;
    }
    for (i = 0; i < (int)sizeof(mInsect); i++) {
        mInsect[i] = 0;
    }
    for (i = 0; i < (int)sizeof(mPicture); i++) {
        mPicture[i] = 0;
    }
}

// 8011980C
void dMuseum_c::clearDonor(const dItem::Item &item) {
    int idx;
    u8 *donors = getDonors(item, &idx);
    if (donors != NULL) {
        donors[idx >> 1] &= ~(0xF << ((idx & 1) << 2));
    }
}

// 80119858
void dMuseum_c::setDeleted(const dItem::Item &item) {
    int idx;
    u8 *donors = getDonors(item, &idx);
    if (donors != NULL) {
        u8 cleared = donors[idx >> 1] & ~(0xF << ((idx & 1) << 2));
        donors[idx >> 1] = cleared | (MUSEUM_DONOR_DELETED << ((idx & 1) << 2));
    }
}

// 801198B4
u8 *dMuseum_c::getDonors(const dItem::Item &item, int *idx) {
    switch (item.getKind()) {
    case dItem::KIND_FOSSIL:
        if (idx != NULL) {
            *idx = dItem::seeker_c::get()->findLike(item);
        }
        return mFossil;
    case dItem::KIND_PICTURE:
        if (idx != NULL) {
            *idx = dItem::seeker_c::get()->findLike(item);
        }
        return mPicture;
    case dItem::KIND_INSECT:
        if (idx != NULL) {
            *idx = dItem::seeker_c::get()->findLike(item);
        }
        return mInsect;
    case dItem::KIND_FISH:
        if (idx != NULL) {
            *idx = dItem::seeker_c::get()->findLike(item);
        }
        return mFish;
    }
    return NULL;
}

// 801199D0
int dMuseum_c::getDonor(const dItem::Item &item) {
    int idx;
    u8 *donors = getDonors(item, &idx);
    if (donors != NULL) {
        return getNibble(donors, idx);
    }
    return MUSEUM_DONOR_NONE;
}

// 80119A1C
int dMuseum_c::getDonor(int kind, int idx) {
    u8 *donors = NULL;
    switch (kind) {
    case dItem::KIND_FOSSIL:
        donors = mFossil;
        break;
    case dItem::KIND_PICTURE:
        donors = mPicture;
        break;
    case dItem::KIND_INSECT:
        donors = mInsect;
        break;
    case dItem::KIND_FISH:
        donors = mFish;
        break;
    }
    if (donors != NULL) {
        return getNibble(donors, idx);
    }
    return MUSEUM_DONOR_NONE;
}

// 80119A94
int dMuseum_c::getDonorKind(const dItem::Item &item) {
    int donor = getDonor(item);
    if (donor == MUSEUM_DONOR_NONE) {
        return MUSEUM_NOT_DONATED;
    }
    if (donor == MUSEUM_DONOR_DELETED) {
        return MUSEUM_DONATED_BY_DELETED;
    }
    return donor != fn_801017B8() + 1;
}

// 80119AF8
BOOL dMuseum_c::isDonated(const dItem::Item &item) {
    return getDonor(item) != MUSEUM_DONOR_NONE;
}

// 80119B24
BOOL dMuseum_c::isDonated(int kind, int idx) {
    return getDonor(kind, idx) != MUSEUM_DONOR_NONE;
}

// 80119B50
void dMuseum_c::donate(const dItem::Item &item) {
    clearDonor(item);
    int idx;
    u8 *donors = getDonors(item, &idx);
    if (donors != NULL) {
        u32 player = fn_801017B8() & 3;
        donors[idx >> 1] |= (player + 1) << ((idx & 1) << 2);
    }
    if (isComplete()) {
        mCompleteTime.setNow();
    }
}

// 80119BE4
void dMuseum_c::deletePlayer(int player) {
    u32 i;
    for (i = 0; i < MUSEUM_FOSSIL_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_AMBER, i, FALSE);
        if ((player & 3) + 1 == getDonor(item)) {
            setDeleted(item);
        }
    }
    for (i = 0; i < MUSEUM_FISH_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, i, FALSE);
        if ((player & 3) + 1 == getDonor(item)) {
            setDeleted(item);
        }
    }
    for (i = 0; i < MUSEUM_INSECT_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, i, FALSE);
        if ((player & 3) + 1 == getDonor(item)) {
            setDeleted(item);
        }
    }
    for (i = 0; i < MUSEUM_PICTURE_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_DYNAMIC_PAINTING, i, FALSE);
        if ((player & 3) + 1 == getDonor(item)) {
            setDeleted(item);
        }
    }
}

// 80119D48
BOOL dMuseum_c::setDonorWord(dScript::Word_c *word, const dItem::Item &item) {
    dPersonalID_c pid;
    if (!getDonorID(&pid, item)) {
        return FALSE;
    }
    if (pid.isValid()) {
        pid.setWord(word);
        return TRUE;
    }
    return FALSE;
}

// 80119DB0
BOOL dMuseum_c::getDonorID(dPersonalID_c *out, const dItem::Item &item) {
    if ((u32)getDonorKind(item) <= MUSEUM_DONATED_BY_OTHER) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer((getDonor(item) - 1) & 3);
        if (player->mPID.isValid()) {
            *out = player->mPID;
            return TRUE;
        }
    }
    return FALSE;
}

// 80119EE8
BOOL dMuseum_c::isComplete() {
    if (!isPictureComplete()) {
        return FALSE;
    }
    if (!isFishComplete()) {
        return FALSE;
    }
    if (!isInsectComplete()) {
        return FALSE;
    }
    if (!isFossilComplete()) {
        return FALSE;
    }
    return TRUE;
}

// 80119F68
BOOL dMuseum_c::isInsectComplete() {
    for (u32 i = 0; i < MUSEUM_INSECT_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, i, FALSE);
        if (!isDonated(item)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 80119FDC
BOOL dMuseum_c::isFossilComplete() {
    for (u32 i = 0; i < MUSEUM_FOSSIL_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_AMBER, i, FALSE);
        if (!isDonated(item)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8011A050
BOOL dMuseum_c::isPictureComplete() {
    for (u32 i = 0; i < MUSEUM_PICTURE_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_DYNAMIC_PAINTING, i, FALSE);
        if (!isDonated(item)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8011A0C4
BOOL dMuseum_c::isFishComplete() {
    for (u32 i = 0; i < MUSEUM_FISH_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, i, FALSE);
        if (!isDonated(item)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8011A138
BOOL dMuseum_c::hasAnyDonation() {
    u32 i;
    for (i = 0; i < MUSEUM_FOSSIL_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_AMBER, i, FALSE);
        if ((u32)isDonated(item) == TRUE) {
            return TRUE;
        }
    }
    for (i = 0; i < MUSEUM_PICTURE_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_DYNAMIC_PAINTING, i, FALSE);
        if ((u32)isDonated(item) == TRUE) {
            return TRUE;
        }
    }
    for (i = 0; i < MUSEUM_INSECT_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, i, FALSE);
        if ((u32)isDonated(item) == TRUE) {
            return TRUE;
        }
    }
    for (i = 0; i < MUSEUM_FISH_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, i, FALSE);
        if ((u32)isDonated(item) == TRUE) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8011A26C
int dMuseum_c::getIndex(const dItem::Item &item) {
    return dItem::seeker_c::get()->findLike(item);
}

// 8011A29C
BOOL dMuseum_c::isFossilSetComplete(const dItem::Item &item) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return FALSE;
    }
    int set = 0;
    u32 raw = bitm->m_fossil;
    if (raw < 0x1C) {
        set = raw;
    }
    if (set == 0) {
        return FALSE;
    }
    for (u32 i = 0; i < MUSEUM_FOSSIL_NUM; i++) {
        dItem::Item fossil(dItem::ITEM_IDX_AMBER, i, FALSE);
        const dItem::BITM *b = dItem::infoBank_c::get()->getBITM(fossil);
        int other;
        if (b != NULL) {
            other = 0;
            u32 r = b->m_fossil;
            if (r < 0x1C) {
                other = r;
            }
        } else {
            other = 0;
        }
        if (set == other && !isDonated(fossil)) {
            return FALSE;
        }
    }
    return TRUE;
}

// 8011A3A8
int dMuseum_c::getProgress() {
    u32 num = 0;
    for (u32 i = 0; i < MUSEUM_PICTURE_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_DYNAMIC_PAINTING, i, FALSE);
        if (isDonated(item)) {
            num++;
        }
    }
    for (u32 i = 0; i < MUSEUM_FISH_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_BITTERLING, i, FALSE);
        if (isDonated(item)) {
            num++;
        }
    }
    for (u32 i = 0; i < MUSEUM_INSECT_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, i, FALSE);
        if (isDonated(item)) {
            num++;
        }
    }
    for (u32 i = 0; i < MUSEUM_FOSSIL_NUM; i++) {
        dItem::Item item(dItem::ITEM_IDX_AMBER, i, FALSE);
        if (isDonated(item)) {
            num++;
        }
    }
    int percent = num * 100 / MUSEUM_ITEM_NUM;
    if (percent <= 0 && (int)num > 0) {
        percent = 1;
    }
    return percent;
}

// 8011A4F8
int dMuseum_c::getKindNum(int kind) {
    switch (kind) {
    case dItem::KIND_FOSSIL:
        return MUSEUM_FOSSIL_NUM;
    case dItem::KIND_INSECT:
        return MUSEUM_INSECT_NUM;
    case dItem::KIND_FISH:
        return MUSEUM_FISH_NUM;
    case dItem::KIND_PICTURE:
        return MUSEUM_PICTURE_NUM;
    }
    return 0;
}

// 8011A550
int dMuseum_c::countDonated(int kind) {
    int count = 0;
    u32 num = getKindNum(kind);
    sSeeker.search(kind, 6, NULL);
    for (u32 i = 0; i < num; i++) {
        dItem::Item item = sSeeker.getNth(i);
        if (isDonated(item)) {
            count++;
        }
    }
    return count;
}

// 8011A5E8
u16 dMuseum_c::getNthDonated(int kind, int n) {
    int found = 0;
    u32 num = getKindNum(kind);
    if (n < 0) {
        return dItem::ITEM_ID_NONE;
    }
    for (u32 i = 0; i < num; i++) {
        if (isDonated(kind, i)) {
            if (found == n) {
                sSeeker.search(kind, 6, NULL);
                return sSeeker.getNth(i).mId;
            }
            found++;
        }
    }
    return dItem::ITEM_ID_NONE;
}
