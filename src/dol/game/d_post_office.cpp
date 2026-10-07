// The post office (see include/game/game/d_post_office.hpp).
// .text 8010263C..80104770.
#include <game/game/d_post_office.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_script.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <string.h>

// Not split yet (C linkage keeps the target names).
extern "C" {
// Visit (Wi-Fi) session.
BOOL fn_800DCEDC();                             // in a session
u32 fn_800DCF30();                              // members
int fn_800DCF58();                              // this console's member index (0 = host)
BOOL fn_800DCF90();
BOOL fn_800DCF2C(int player);                   // member present
void fn_800DD4C8();                             // start a packet
void fn_800DD518(const void *data, u32 size);   // add data
void fn_800DD588(int type, int player);         // send it
int fn_800092CC(const wchar_t *str);            // string length

int fn_8017BF90(const dPersonalID_c *pid, dPrivateData_c *player); // friend index

// A player's mail box (dPrivateData_c::fn_80139408).
int fn_8013EE8C(void *box);
void fn_8013EDFC(void *box, int idx, dMail_c *mail, int arg);
BOOL fn_8013EE94(void *box);
void fn_8013EF50(void *box);                    // deliver the letter to the future when due
BOOL fn_8013F06C(void *box);                    // has a letter to the future
void fn_8013F064(const dTime_c *time, void *box); // its delivery time

// WiiConnect24.
BOOL fn_80178048(dPostRequest_c *req);          // start sending
int fn_80178374();                              // 0 busy?, 1 sent, -1 error

void fn_8010F114(dSaveDistMail_c *mail, int player); // mark the downloaded letter as received
}

static dPostRequest_c l_request;
static u16 l_words[48];
static u16 l_recycleBuf[RECYCLE_BIN_ITEM_NUM];
static u8 l_melodyBuf[16];

static u8 l_syncState = 7;

static int l_sendState;
static int l_sendIdx;
static u8 l_recycleSync[PLAYER_NUM];
static u8 l_melodySync[PLAYER_NUM];

namespace dPostOffice {

// 8010263C
int getToPlayerIdx(dMail_c *mail) {
    dPersonalID_c *to = mail->getToPlayer();
    if (to == NULL) {
        return -1;
    }
    if (mail->isToOtherTown()) {
        return -2;
    }
    int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, to);
    return idx != -1 ? idx : -3;
}

// 801026C0
dPrivateData_c *getSender(dMail_c *mail) {
    if (!mail->isToOtherTown()) {
        return NULL;
    }
    dPersonalID_c *from = mail->getFromPlayer();
    if (from != NULL) {
        return dPlayerMgr_c::getPlayer(from);
    }
    dPersonalID_c *to = mail->getToPlayer();
    if (to == NULL) {
        return NULL;
    }
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
        if (player->mPID.isValid() && fn_8017BF90(to, player) >= 0) {
            return player;
        }
    }
    return NULL;
}

// 8010278C
int getFriendIdx(dMail_c *mail) {
    if (!mail->isToOtherTown()) {
        return -1;
    }
    dPersonalID_c *to = mail->getToPlayer();
    if (to == NULL) {
        return -2;
    }
    if (!to->isValid()) {
        return -3;
    }
    dPrivateData_c *sender = getSender(mail);
    if (sender == NULL) {
        return -3;
    }
    int idx = fn_8017BF90(to, sender);
    if (idx < 0) {
        idx = -4;
    }
    return idx;
}

// 80102830
int getToAnimalIdx(dMail_c *mail) {
    if (!mail->hasToAnimal()) {
        return -1;
    }
    if (mail->isToOtherTown()) {
        return -3;
    }
    for (int i = 0; i < ANIMAL_NUM; i++) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
        if (animal != NULL && animal->mID.isValid()) {
            if (animal->mID == *mail->getToAnimal()) {
                return i;
            }
        }
    }
    return -2;
}

// 80102974
BOOL deliverToAnimal(dMail_c *mail) {
    if (getToAnimalIdx(mail) >= 0) {
        dSaveData_c::getTown()->mAnimals.mTown.receiveLetter(mail);
    }
    return TRUE;
}

// 801029C0
BOOL deliverToPlayer(dMail_c *mail) {
    int idx = getToPlayerIdx(mail);
    if (idx < 0) {
        return FALSE;
    }
    void *box = dPlayerMgr_c::getPlayer(idx)->fn_80139408();
    int slot = fn_8013EE8C(box);
    if (slot < 0) {
        return FALSE;
    }
    fn_8013EDFC(box, slot, mail, 0);
    return TRUE;
}

// 80102A3C
BOOL deliver(dMail_c *mail) {
    int idx = getToPlayerIdx(mail);
    if (idx == -1) {
        return deliverToAnimal(mail);
    } else if (idx == -2) {
        return FALSE;
    } else if (idx == -3) {
        return TRUE;
    } else {
        return deliverToPlayer(mail);
    }
}

// 80102AA4
void deliverAll() {
    dPostBox_c *box = dPostBox_c::get();
    int num = box->pack();
    for (int i = 0; i < num; i++) {
        if (deliver(&box->mMails[i])) {
            box->clear(i);
        }
    }
    box->pack();
}

// 80102B38
BOOL hasMail() {
    if (fn_800DCF58() != 0) {
        return FALSE;
    }
    void *box = dPlayerMgr_c::getCurrentPlayer()->fn_80139408();
    if (box == NULL) {
        return FALSE;
    }
    return fn_8013EE94(box);
}

// 80102B84
BOOL isFull() {
    return dPostBox_c::get()->findEmpty() == -1;
}

// 80102BBC
BOOL add(dMail_c *mail) {
    dPostBox_c *box = dPostBox_c::get();
    int idx = box->findEmpty();
    if (idx == -1) {
        return FALSE;
    }
    box->set(idx, mail);
    return TRUE;
}

// 80102C24
void holdMail(dMail_c *mail) {
    dPostBox_c::get()->setHeld(mail);
}

// 80102C60
void update() {
    if (!isSceneAttr(getCurrentScene(), 0x20) && (!fn_800DCEDC() || fn_800DCF30() <= 1)) {
        if (dPostBox_c::get()->checkDelivery(TRUE)) {
            deliverAll();
        }
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            void *box = player->fn_80139408();
            if (box != NULL) {
                fn_8013EF50(box);
            }
        }
    }
}

// 80102CE8
BOOL hasFutureLetter() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        void *box = player->fn_80139408();
        if (box != NULL) {
            return fn_8013F06C(box);
        }
    }
    return 0;
}

// 80102D28
void setFutureLetterTime(const dTime_c *time) {
    fn_8013F064(time, dPlayerMgr_c::getCurrentPlayer()->fn_80139408());
}

// 80102D64
BOOL checkFull() {
    if (fn_800DCEDC()) {
        if (fn_800DCF90()) {
            return isFull();
        }
        return TRUE;
    }
    if (!isFull()) {
        return FALSE;
    }
    deliverAll();
    return isFull();
}

// 80102DC4
void resetSync() {
    setSyncState(0);
    sendSyncState(0, 0);
}

// 80102DF4
int getSyncResult() {
    switch (getSyncState()) {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 3;
    default:
        return 2;
    }
}

// 80102E58
BOOL isOtherTownMailFull() {
    dPostBox_c *box;
    int num = 0;
    box = dPostBox_c::get();
    int count = box->pack();
    for (int i = 0; i < count; i++) {
        if (box->mMails[i].isToOtherTown()) {
            num++;
        }
    }
    return num > 30;
}

// 80102EF0
void removeSent() {
    dPostBox_c *box = dPostBox_c::get();
    if (l_sendIdx == -1) {
        box->clearHeld();
    } else if (l_sendIdx >= 0 && l_sendIdx < 32) {
        box->clear(l_sendIdx);
    }
}

// 80102F40
void findNextSend() {
    dPostBox_c *box = dPostBox_c::get();
    for (; l_sendIdx < 80; l_sendIdx++) {
        dMail_c *mail;
        if (l_sendIdx == -1) {
            mail = &box->mHeld;
        } else {
            mail = &box->mMails[l_sendIdx];
        }
        if (mail->isEmpty()) {
            continue;
        }
        int friendIdx = getFriendIdx(mail);
        if (friendIdx == -1) {
            continue;
        }
        if ((u32)(friendIdx + 4) <= 2) {
            removeSent();
        } else if (friendIdx >= 0 && friendIdx < 32) {
            dItem::Item present = mail->getPresent();
            dSaveDLItem_c *dl = NULL;
            u32 dlSize = 0;
            if (present.mId != 0xFFF1) {
                dl = dSaveDLItemList_c::getRaw()->find(present.mId);
            }
            if (dl != NULL) {
                dlSize = 0x2000;
            }
            dPrivateData_c *sender = getSender(mail);
            l_request.mData[2] = dl;
            l_request.mSize[2] = dlSize;
            l_request.mData[3] = mail;
            l_request.mSize[3] = sizeof(dMail_c);
            l_request.mFriend = friendIdx;
            l_request.mSender = sender;
            l_sendState = 1;
            return;
        }
    }
    l_sendState = 3;
}

// 80103084
void sendNext() {
    l_sendIdx++;
    findNextSend();
}

// 80103094
void startSend() {
    l_sendState = 0;
    l_sendIdx = -1;
    dPostBox_c::get()->pack();
}

// 801030D0
BOOL updateSend() {
    if (l_sendState == 0) {
        findNextSend();
    }
    if (l_sendState == 1) {
        l_sendState = fn_80178048(&l_request) ? 2 : 3;
    }
    if (l_sendState == 2) {
        int result = fn_80178374();
        if (result == 1) {
            removeSent();
        } else if (result != 0) {
            if (result == -1) {
                return FALSE;
            }
            l_sendState = 3;
            return TRUE;
        }
        sendNext();
    }
    return l_sendState == 3;
}

// 80103184
void setSyncWait() {
    l_syncState = 7;
}

// 80103190
void setSyncState(u8 state) {
    l_syncState = state;
}

// 80103198
u8 getSyncState() {
    return l_syncState;
}

// 801031A0
void onSyncPacket(int player, const u8 *data) {
    u8 state = *data;
    switch (state) {
    case 0:
        if (isFull()) {
            replySyncState(1, player);
        } else {
            replySyncState(2, player);
        }
        break;
    case 1:
    case 2:
        setSyncState(state);
        break;
    case 3:
        break;
    case 4:
    case 5:
    case 6:
        setSyncState(state);
        break;
    }
}

// 80103234
void sendMail(dMail_c *mail) {
    setSyncState(3);
    fn_800DD4C8();
    fn_800DD518(mail, sizeof(dMail_c));
    fn_800DD588(0x2E, 0);
}

// 80103280
void onMailPacket(int player, const void *data) {
    dMail_c mail;
    cLib::memCpy(&mail, data, sizeof(dMail_c));
    int state = 6;
    if (add(&mail)) {
        state = isFull() ? 5 : 4;
    }
    replySyncState(state, player);
}

// 8010330C
void sendSyncState(u8 state, int player) {
    u8 data = state;
    fn_800DD4C8();
    fn_800DD518(&data, 1);
    fn_800DD588(0x2D, player);
}

// 80103354
void replySyncState(u8 state, int player) {
    sendSyncState(state, player);
}

// 80103358
BOOL isSeparator(wchar_t c) {
    const wchar_t separators[11] = {0x3092, 0xFF01, 0xFF1F, 0x3002, 0x3001, 0x0020,
                                    0x3000, 0x000A, 0x002C, 0x002E, 0x0000};
    const wchar_t *p = separators;
    for (int i = 0; i < 11; i++, p++) {
        if (c == *p) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80103404
BOOL isWord(const wchar_t *str) {
    for (int i = 0; i < 3; i++) {
        if (isSeparator(str[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

// 80103464
int countWords(dMail_c *mail) {
    const wchar_t *str = mail->mBody;
    BOOL start = TRUE;
    int num = 0;
    for (int i = 0; i < MAIL_BODY_LEN - 3; i++, str++) {
        if (*str == 0) {
            break;
        }
        if (start && isWord(str)) {
            num++;
        }
        start = isSeparator(*str);
    }
    return num;
}

// 801034EC
void replaceNewlines(wchar_t *str, int len) {
    for (int i = 0; i < len; i++, str++) {
        if (*str == L'\n') {
            *str = L' ';
        }
    }
}

// 80103518
void toHiragana(wchar_t *str, int len) {
    for (int i = 0; i < len; i++) {
        if (dScript::isKatakana(str[i]) && str[i] != 0x30F6) {
            str[i] -= 0x60;
        }
    }
}

// 80103590
void toLower(wchar_t *str, int len) {
    for (int i = 0; i < len; i++) {
        dScript::toLower(&str[i]);
    }
}

// 801035EC
void normalize(wchar_t *str, int len) {
    switch (getRegion()) {
    case 0:
        toHiragana(str, len);
        break;
    case 1:
    case 2:
        toLower(str, len);
        break;
    }
}

// 80103658
int getLineLength(const wchar_t *str, int max) {
    for (int i = 0; i < max; i++) {
        if (str[i] == 0) {
            return i;
        }
        if (str[i] == L'\n') {
            return i;
        }
    }
    return max;
}

// 801036A0
const wchar_t *nextLine(const wchar_t *str, int max) {
    str += getLineLength(str, max);
    if (*str == L'\n') {
        str++;
    }
    return str;
}

// 801036E4
int findWord(const wchar_t *word, const wchar_t *list) {
    int i = 0;
    while (list != NULL && *list != 0) {
        int len = getLineLength(list, 3);
        if (memcmp(list, word, len * sizeof(wchar_t)) == 0) {
            return i;
        }
        list = nextLine(list, 4);
        i++;
    }
    return -1;
}

// 80103784
int getWordId(const wchar_t *word) {
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), "sys_STRING/STR_Mailcheck"));
    u32 id;
    u16 c = *word;
    if (getRegion() == 0) {
        // Fold voiced kana onto the unvoiced one.
        if (c >= 0x304B && c <= 0x3062 && (c & 1) == 0) {
            c = c - 1;
        }
        if (c >= 0x3064 && c <= 0x3069 && (c & 1) == 1) {
            c = c - 1;
        }
        if (c >= 0x306F && c <= 0x307D) {
            int rem = (c - 0x306F) % 3;
            if (rem > 0) {
                c = c - rem;
            }
        }
    }
    for (id = 1; id < res.getCount(); id++) {
        const wchar_t *list = res.getMessage(id);
        if (*list == c) {
            int idx = findWord(word, list);
            if (idx == -1) {
                return -1;
            }
            return idx + id * 200;
        }
    }
    return -1;
}

// 801038F4
BOOL bodyContains(dMail_c *mail, const wchar_t *str) {
    int len = dScript::getLineLength((const u16 *)str, 0);
    const wchar_t *body = mail->mBody;
    for (int i = 0; i < MAIL_BODY_LEN - len; body++, i++) {
        if (memcmp(body, str, len * sizeof(wchar_t)) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8010397C
int scoreWords(dMail_c *mail) {
    memset(l_words, 0, sizeof(l_words));
    u16 *out = l_words;
    int total = 0;
    int unique = 0;
    const wchar_t *str = mail->mBody;
    BOOL start = TRUE;
    for (int i = 0; i < MAIL_BODY_LEN - 3; i++, str++) {
        if (*str == 0) {
            break;
        }
        if (start && isWord(str)) {
            int id = getWordId(str);
            if (id != -1) {
                u16 *p = l_words;
                int seen = 0;
                for (int j = 0; j < unique; j++, p++) {
                    if (id == *p) {
                        seen++;
                    }
                }
                if (seen < 2) {
                    *out++ = id;
                    unique++;
                }
            }
            total++;
        }
        start = isSeparator(*str);
    }
    if (total == 0) {
        return 0;
    }
    return unique * 100 / total;
}

// 80103A80
int scoreNames(dMail_c *mail) {
    dAnimalBlock_c *animals;
    int score = 0;
    dPersonalID_c *from = mail->getFromPlayer();
    if (from == NULL) {
        return 0;
    }
    if (bodyContains(mail, from->player.mName)) {
        score = 3;
    }
    dAnmPersonalID_c *to = mail->getToAnimal();
    if (to == NULL) {
        return score;
    }
    if (bodyContains(mail, to->getName(LANGUAGE_NUM))) {
        score += 3;
    }
    animals = &dSaveData_c::getRaw()->mAnimals.mTown;
    dItem::Item key = animals->getAnimalKey(to);
    dAnimal_c *animal = animals->getAnimalByKeyConst(&key);
    if (animal == NULL) {
        return score;
    }
    if (!animal->usesNickname(from)) {
        return score;
    }
    dHmnName::Word_c name;
    animal->getPlayerCallName(&name, from);
    dScript::Word_c *word = &name;
    if (bodyContains(mail, word->getBuffer())) {
        score += 3;
    }
    return score;
}

// 80103BC8
int getLengthType(dMail_c *mail) {
    int len = fn_800092CC(mail->mBody);
    if (len > 40) {
        return 2;
    }
    return len > 20;
}

// 80103C10
int getReplyType(dMail_c *mail) {
    dMail_c copy;
    copy.copy(mail);
    normalize(copy.mBody, MAIL_BODY_LEN);
    BOOL rare = cM::rndInt(100) < 10;
    if (rare && findSpecial(&copy)) {
        return 3;
    }
    int words = countWords(mail);
    if (words < 2) {
        return 0;
    }
    if (words == 2) {
        return 1;
    }
    int score = scoreWords(&copy);
    if (score + scoreNames(mail) >= 25) {
        return 2;
    }
    return 1;
}

// 80103D24
int findSpecial(dMail_c *mail) {
    dMail_c copy;
    copy.copy(mail);
    normalize(copy.mBody, MAIL_BODY_LEN);
    replaceNewlines(copy.mBody, MAIL_BODY_LEN);
    dScript::Res_c res(dScript::getBmgFile(dScript::getBank(), "sys_STRING/STR_LetterSP"));
    for (u32 id = 1; id < res.getCount(); id++) {
        for (const wchar_t *str = res.getMessage(id); *str != 0; str = nextLine(str, 100)) {
            if (bodyContains(&copy, str)) {
                return id;
            }
        }
    }
    return 0;
}

// 80103E3C
void clearRecycleSync() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        l_recycleSync[i] = 0;
    }
}

// 80103E58
BOOL isRecycleSyncDone() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (l_recycleSync[i] == 1) {
            return FALSE;
        }
    }
    return TRUE;
}

// 80103EB4
void sendRecycleBin(int player) {
    fn_800DD4C8();
    fn_800DD518(dSaveData_c::getTown()->mRecycleBin.getItems(), sizeof(dRecycleBin_c));
    fn_800DD588(0x62, player);
}

// 80103F04
void broadcastRecycleBin() {
    BOOL online = FALSE;
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        online = TRUE;
    }
    if (online) {
        int self = fn_800DCF58();
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (i != self && fn_800DCF2C(i)) {
                l_recycleSync[i] = 1;
                sendRecycleBin(i);
            }
        }
    }
}

// 80103FB0
void onRecycleBin(int player, const void *data) {
    cLib::memCpy(l_recycleBuf, data, sizeof(l_recycleBuf));
    dRecycleBin_c *bin = &dSaveData_c::getTown()->mRecycleBin;
    u16 *item = l_recycleBuf;
    for (int i = 0; i < RECYCLE_BIN_ITEM_NUM; i++, item++) {
        bin->set(i, *item);
    }
    fn_800DD4C8();
    fn_800DD588(0x63, player);
}

// 80104044
void onRecycleAck(int player) {
    l_recycleSync[player] = 2;
}

// 80104054
void clearMelodySync() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        l_melodySync[i] = 0;
    }
}

// 80104070
BOOL isMelodySyncDone() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        if (l_melodySync[i] == 1) {
            return FALSE;
        }
    }
    return TRUE;
}

// 801040CC
void sendMelody(int player) {
    fn_800DD4C8();
    fn_800DD518(dSaveData_c::getTown()->mVillageMelody.getNotes(), sizeof(dSaveMelody_c));
    fn_800DD588(0x67, player);
}

// 8010411C
void broadcastMelody() {
    BOOL online = FALSE;
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        online = TRUE;
    }
    if (online) {
        int self = fn_800DCF58();
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (i != self && fn_800DCF2C(i)) {
                l_melodySync[i] = 1;
                sendMelody(i);
            }
        }
    }
}

// 801041C8
void onMelody(int player, const void *data) {
    cLib::memCpy(l_melodyBuf, data, sizeof(l_melodyBuf));
    dSaveData_c::getTown()->mVillageMelody.set(l_melodyBuf);
    fn_800DD4C8();
    fn_800DD588(0x68, player);
}

// 8010422C
void onMelodyAck(int player) {
    l_melodySync[player] = 2;
}

// 8010423C
BOOL canGetDistMail() {
    dMail_c mail = dSaveData_c::getExtra()->mDistMail.mMail;
    if (mail.isEmpty()) {
        return FALSE;
    }
    int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, &dPlayerMgr_c::getCurrentPlayer()->mPID);
    if (idx == -1) {
        return FALSE;
    }
    if (mail.isFlag(idx)) {
        return FALSE;
    }
    return TRUE;
}

// 801043FC
BOOL getDistMail() {
    dSaveDistMail_c *dist;
    if (!canGetDistMail()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    int slot = player->findLetter();
    if (slot == -1) {
        return FALSE;
    }
    int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, &player->mPID);
    if (idx == -1) {
        return FALSE;
    }
    dist = &dSaveData_c::getExtra()->mDistMail;
    dMail_c *src = &dist->mMail;
    dMail_c mail = *src;
    mail.setToPlayer(idx, TRUE);
    mail.clearFlags();
    player->fn_8013835C(&mail, slot);
    fn_8010F114(dist, idx);
    return TRUE;
}

// 801045E8
void getDistMailSender(dScript::Word_c *word) {
    dMail_c mail = dSaveData_c::getExtra()->mDistMail.mMail;
    mail.setFromWord(word);
}

} // namespace dPostOffice
