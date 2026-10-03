// Letters (dMail_c). .text 80117400..8011927C (ends with __sinit 801191E0).
// First pass: every function except the __sinit is written for equivalence.
#include <game/game/d_mail.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_letter.hpp>
#include <game/game/d_string.hpp>
#include <game/mLib/m_2d_string_word.hpp>
#include <cstring>
#include <game/game/d_player_mgr.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
dAnimal_c *fn_80129D90(dAnimalBlock_c *block, int idx); // villager, index clamped to the town's count

// dLetter text builders (fill the header/body/footer words).
void fn_800CB638(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 kind, const char *label);
void fn_800CB66C(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 a, u16 b, u16 c, int d,
                 int e, int f);
void fn_800CB760(dScript::Word_c *header, dScript::Word_c *body, dScript::Word_c *footer, u16 a, u16 b, u16 c, u16 d,
                 u16 e, int f, int g);
void fn_800CC0C8(const dPersonalID_c *to);
void fn_800CC1AC(const dAnmPersonalID_c *sender);
void fn_800CBC70(int slot, const dAnmPersonalID_c *sender);
void fn_800CBC10(int slot, const dAnmPersonalID_c *sender);

void fn_8016AE68(dScript::Word_c *word, u16 index, const char *group); // load a BMG string
void fn_8016B15C(int slot, dScript::Word_c *word); // script tag word
void fn_8016B050(int slot, const dPersonalID_c *pid); // script tag player
}

dItem::Item makeItemFromBaseId(u16 baseId);

// 80475D10: "from" message per mSenderKind value (getListWord).
static const u16 sFromMsgIds[MAIL_FROM_NUM] = {
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x70, 0x80, 0x7D, 0x7E,
    0x7F, 0x110, 0x111, 0x112, 0x10F, 0x114, 0x113, 0x115, 0x155,
};

// 805EF570..805F0F18: text scratch words, constructed by the __sinit (801191E0).
static dLetter::Word_c sHeaderWord;
static dLetter::Word_c sBodyWord;
static dLetter::Word_c sFooterWord;
static m2d::stringWord_c sMenuWord;

// 80117400
dMail_c::dMail_c() {}

// 80117410
dMail_c::~dMail_c() {}

// 80117450
dMail_c *dMail_c::copy(const dMail_c *other) {
    memcpy(static_cast<void*>(this), other, sizeof(dMail_c));
    return this;
}

// 80117484
u8 dMail_c::getStatus() {
    return m38C & MAIL_38C_STATUS_MASK;
}

// 80117490
void dMail_c::setStatus(u8 status) {
    m38C &= ~MAIL_38C_STATUS_MASK;
    m38C |= status;
}

// 801174A4
u16 dMail_c::getPresent() {
    return mPresent.mId;
}

// 801174AC
u8 dMail_c::get38C_hi() {
    return (m38C >> MAIL_38C_HI_SHIFT) & 3;
}

// 801174B8
u8 dMail_c::getSenderKind() {
    return mSenderKind;
}

// 801174C0
void dMail_c::clearFlags() {
    mSenderKind = 0;
}

// 801174CC
void dMail_c::clearFlags2() {
    mSenderKind = 0;
}

// 801174D8
void dMail_c::setFlag(int bit) {
    mSenderKind |= (u8)(1 << bit);
}

// 801174F4
void dMail_c::clearFlag(int bit) {
    mSenderKind &= ~(u8)(1 << bit);
}

// 80117510
u8 dMail_c::isFlag(int bit) {
    return mSenderKind & (u8)(1 << bit);
}

// 80117528
BOOL dMail_c::isReadFlaggedInvite() {
    if (getStatus() != MAIL_STATUS_READ) {
        return FALSE;
    }
    return isFlaggedInvite();
}

// 80117570
BOOL dMail_c::isFlaggedInvite() {
    int idx = dItem::Item_getIdxInKind(dItem::Item(MAIL_INVITE_CARD_IDX));

    if (idx != mPaper) {
        return FALSE;
    }
    return (m38C >> 5) & 1;
}

// 801175D4
BOOL dMail_c::isReadUnflaggedInvite() {
    if (getStatus() != MAIL_STATUS_READ) {
        return FALSE;
    }

    int idx = dItem::Item_getIdxInKind(dItem::Item(MAIL_INVITE_CARD_IDX));
    if (idx != mPaper) {
        return FALSE;
    }
    return ((m38C >> 5) & 1) ^ 1;
}

// 80117654
void dMail_c::flagInvite() {
    int idx = dItem::Item_getIdxInKind(dItem::Item(MAIL_INVITE_CARD_IDX));
    if (idx == mPaper) {
        m38C &= (u8)~MAIL_38C_INVITE_FLAG;
        m38C |= MAIL_38C_INVITE_FLAG;
    }
}

// 801176B8
void dMail_c::set38C_hi(u8 value) {
    m38C = (m38C & ~MAIL_38C_HI_MASK) | (value << MAIL_38C_HI_SHIFT);
}

// 801176CC
u16 dMail_c::setPresent(u16 item, u8 hi) {
    u16 old = mPresent.mId;
    if (hi == 0xFF) {
        hi = 0;
    }
    mPresent.mId = item;
    set38C_hi(hi);
    return old;
}

// 80117710
BOOL dMail_c::isWritten() {
    return getStatus() == MAIL_STATUS_WRITTEN;
}

// 80117740
BOOL dMail_c::isToNone() {
    return mTo.isType(MAIL_ADDR_NONE);
}

// 80117748
BOOL dMail_c::isEmpty() {
    return getStatus() == MAIL_STATUS_NONE;
}

// 80117774
BOOL dMail_c::isToFutureSelf() {
    return mTo.isType(MAIL_ADDR_FUTURE_SELF);
}

// 8011777C
BOOL dMail_c::isFromText() {
    return mFrom.isType(MAIL_ADDR_TEXT);
}

// 80117788
BOOL dMail_c::isToOtherTown() {
    dLandID_c town = dSaveData_c::getTown()->mLandID;
    dPersonalID_c *player = getToPlayer();
    if (player != NULL) {
        return town != player->land;
    }

    dAnmPersonalID_c *animal = getToAnimal();
    if (animal != NULL) {
        return town != animal->mLand;
    }
    return FALSE;
}

// 801178CC
BOOL dMail_c::hasToAnimal() {
    return getToAnimal() != NULL;
}

// 801178F8
void dMail_c::clear() {
    memset(static_cast<void*>(this), 0, sizeof(dMail_c));
    mPresent.mId = dItem::ITEM_ID_NONE;
}

// 80117938
BOOL dMail_c::isValid() {
#ifndef BUGFIXES
    return ((mNamePos <= MAIL_HEADER_LEN) & (mSenderKind <= 19)) != 0;
#else
    return ((mNamePos <= MAIL_HEADER_LEN) && (mSenderKind <= 19)) != 0;
#endif
}

// 80117984
void dMail_c::markSent() {
    switch (getStatus()) {
    case MAIL_STATUS_WRITTEN:
        setStatus(MAIL_STATUS_UNREAD);
        break;
    }

    set38C_hi(1);
}

// 801179D8
void dMail_c::markRead() {
    switch (getStatus()) {
    case MAIL_STATUS_UNREAD:
        setStatus(MAIL_STATUS_READ);
        break;
    }
}

// 80117A20
dPersonalID_c *dMail_c::getFromPlayer() {
    return mFrom.getPlayer();
}

// 80117A28
dAnmPersonalID_c *dMail_c::getFromAnimal() {
    return mFrom.getAnimal();
}

// 80117A30
dPersonalID_c *dMail_c::getToPlayer() {
    return mTo.getPlayer();
}

// 80117A34
dAnmPersonalID_c *dMail_c::getToAnimal() {
    return mTo.getAnimal();
}

// 80117A38
void dMail_c::setup(const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper) {
    clear();
    setPaper(paper);
    mSenderKind = MAIL_FROM_NAMED;
    mTo.setPlayer(to);
    mFrom.setAnimal(sender);
    setStatus(MAIL_STATUS_UNREAD);
    fn_800CC0C8(to);
    fn_800CC1AC(sender);
    fn_800CBC70(0, sender);
    fn_800CBC10(8, sender);
}

// 80117AE8
BOOL dMail_c::findNamePos(wchar_t *header, u8 *pos) {
    int idx = -1;
    for (int i = 0; i < MAIL_HEADER_LEN && header[i] != 0 && idx == -1; i++) {
        if (header[i] == L'\n') {
            idx = i;
        }
    }

    if (idx != -1) {
        *pos = idx;
        for (int i = idx; i < MAIL_HEADER_LEN; i++) {
            header[i] = header[i + 1];
        }
        return TRUE;
    }

    return FALSE;
}

// 80117C5C
void dMail_c::prepareHeader() {
    mNamePos = 0;
    if (!findNamePos(mHeader, &mNamePos)) {
        mTo.hideName();
    }
}

// 80117CA8
void dMail_c::setTextFromLabel(const u16 *kind, const char *label) {
    sHeaderWord.clear();
    sBodyWord.clear();
    sFooterWord.clear();
    fn_800CB638(&sHeaderWord, &sBodyWord, &sFooterWord, *kind, label);
    memcpy(mHeader, static_cast<dScript::Word_c &>(sHeaderWord).getBuffer(), sizeof(mHeader));
    memcpy(mBody, static_cast<dScript::Word_c &>(sBodyWord).getBuffer(), sizeof(mBody));
    memcpy(mFooter, static_cast<dScript::Word_c &>(sFooterWord).getBuffer(), sizeof(mFooter));
}

// 80117D94
void dMail_c::setTextFromParts(const u16 *a, const u16 *b, const u16 *c, int d, int e, int f) {
    sHeaderWord.clear();
    sBodyWord.clear();
    sFooterWord.clear();
    fn_800CB66C(&sHeaderWord, &sBodyWord, &sFooterWord, *a, *b, *c, d, e, f);
    memcpy(mHeader, static_cast<dScript::Word_c &>(sHeaderWord).getBuffer(), sizeof(mHeader));
    memcpy(mBody, static_cast<dScript::Word_c &>(sBodyWord).getBuffer(), sizeof(mBody));
    memcpy(mFooter, static_cast<dScript::Word_c &>(sFooterWord).getBuffer(), sizeof(mFooter));
}

// 80117E90
void dMail_c::setTextFromParts(u16 a, u16 b, u16 c, u16 d, u16 e, int f, int g) {
    sHeaderWord.clear();
    sBodyWord.clear();
    sFooterWord.clear();
    fn_800CB760(&sHeaderWord, &sBodyWord, &sFooterWord, a, b, c, d, e, f, g);
    memcpy(mHeader, static_cast<dScript::Word_c &>(sHeaderWord).getBuffer(), sizeof(mHeader));
    memcpy(mBody, static_cast<dScript::Word_c &>(sBodyWord).getBuffer(), sizeof(mBody));
    memcpy(mFooter, static_cast<dScript::Word_c &>(sFooterWord).getBuffer(), sizeof(mFooter));
}

// 80117F94
void dMail_c::setPaper(const dItem::Item *paper) {
    mPaper = dItem::seeker_c::get()->findLike(*paper);
}

// 80117FD4
void dMail_c::setupFromPlayer(const dItem::Item *paper) {
    clear();
    setPaper(paper);
    mSenderKind = MAIL_FROM_NAMED;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    mFrom.setPlayer(&player->mPID);
    setStatus(MAIL_STATUS_WRITTEN);
    player->mLetterStyle.apply(this);
}

// Up to 10 characters of decimal digits; anything else is skipped.
static inline u16 parseIndex(const wchar_t *str) {
    u16 idx = 0;
    for (int i = 0; i < 10; i++) {
        wchar_t c = str[i];
        if (c == 0) {
            break;
        }
        if (c >= L'0' && c <= L'9') {
            idx *= 10;
            idx += c - L'0';
        }
    }
    return idx;
}

// 8011804C
BOOL dMail_c::setFromBMG(const void *bmg) {
    clear();
    dScript::Res_c res(bmg);
    if (res.getCount() < 6) {
        return FALSE;
    }

    sHeaderWord.clear();
    sBodyWord.clear();
    sFooterWord.clear();
    if (!sHeaderWord.set(res.getMessage(1), 0)) {
        return FALSE;
    }
    if (!sBodyWord.set(res.getMessage(2), 0)) {
        return FALSE;
    }
    if (!sFooterWord.set(res.getMessage(3), 0)) {
        return FALSE;
    }

    memcpy(mHeader, static_cast<dScript::Word_c &>(sHeaderWord).getBuffer(), sizeof(mHeader));
    memcpy(mBody, static_cast<dScript::Word_c &>(sBodyWord).getBuffer(), sizeof(mBody));
    memcpy(mFooter, static_cast<dScript::Word_c &>(sFooterWord).getBuffer(), sizeof(mFooter));
    mNamePos = 0;
    findNamePos(mHeader, &mNamePos);

    sFooterWord.clear();
    sFooterWord.set(res.getMessage(4), 0);
    mFrom.setText(static_cast<dScript::Word_c &>(sFooterWord).getBuffer());

    dItem::Item paper = makeItemFromBaseId(parseIndex(res.getMessage(5)));
    setPaper(&paper);

    if (res.getCount() >= 7) {
        dItem::Item present = makeItemFromBaseId(parseIndex(res.getMessage(6)));
        setPresent(present.mId, 1);
    }

    setStatus(MAIL_STATUS_UNREAD);
    return TRUE;
}

// 801183B8
void dMail_c::setupFromAnimal(const u16 *kind, const char *label, const dAnmPersonalID_c *sender, const dPersonalID_c *to,
                          const dItem::Item *paper) {
    setup(sender, to, paper);
    setTextFromLabel(kind, label);
    prepareHeader();
}

// 80118420
void dMail_c::setupFromAnimal(const u16 *kind, const char *label, const dAnmPersonalID_c *sender, const dPersonalID_c *to,
                          const int *paperIdx) {
    dItem::Item paper(*paperIdx);
    setupFromAnimal(kind, label, sender, to, &paper);
}

// 80118488
void dMail_c::setupFromAnimal(const u16 *a, const u16 *b, const u16 *c, int d, int e, int f,
                          const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper) {
    setup(sender, to, paper);
    setTextFromParts(a, b, c, d, e, f);
    prepareHeader();
}

// 80118508
void dMail_c::setupFromAnimal(const u16 *a, const u16 *b, const u16 *c, const u16 *d, const u16 *e, int f, int g,
                          const dAnmPersonalID_c *sender, const dPersonalID_c *to, const dItem::Item *paper) {
    setup(sender, to, paper);
    setTextFromParts(*a, *b, *c, *d, *e, f, g);
    prepareHeader();
}

// 80118590
void dMail_c::setupSystem(const u16 *kind, const char *label, const u8 *senderKind, const dPersonalID_c *to,
                          const dItem::Item *paper) {
    clear();
    setPaper(paper);
    mSenderKind = *senderKind;
    mTo.setPlayer(to);
    mFrom.hideName();
    setStatus(MAIL_STATUS_UNREAD);
    fn_800CC0C8(to);
    setTextFromLabel(kind, label);
    prepareHeader();
}

// 8011862C
void dMail_c::setupSystem(const u16 *kind, const char *label, const u8 *senderKind, const dPersonalID_c *to,
                          const int *paperIdx) {
    dItem::Item paper(*paperIdx);
    setupSystem(kind, label, senderKind, to, &paper);
}

// 80118694
void dMail_c::setToVillager(int idx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dAnimal_c *animal = fn_80129D90(&dSaveData_c::getTown()->mAnimals.mBlock, idx);
    mTo.setAnimal(&animal->mID);
    player->mLetterStyle.applyHeader(this);
}

// 80118708
void dMail_c::setToPlayer(int idx, BOOL keepHeader) {
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    dPrivateData_c *player = dPlayerMgr_c::getPlayer(idx);
    mTo.setPlayer(&player->mPID);
    if (!keepHeader) {
        current->mLetterStyle.applyHeader(this);
    }
}

// 80118784
void dMail_c::setToSelf() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    mTo.setPlayer(&player->mPID);
    mTo.setFutureSelf();
    player->mLetterStyle.applyHeader(this);
}

// 801187DC
void dMail_c::setToFriend(int idx) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    // TODO: 0x78-byte records at dPrivateData_c+0x1D0 (still inside _0000), each starting with a dPersonalID_c.
    mTo.setPlayer((const dPersonalID_c *)(player->_0000 + 0x1D0 + idx * 0x78));
    player->mLetterStyle.applyHeader(this);
}

// 80118838
void dMail_c::getToName(wchar_t *out) {
    mTo.getName(out, TRUE);
}

// 80118840
void dMail_c::setFromWord(dScript::Word_c *word) {
    mFrom.setWord(word, TRUE);
}

// 8011884C
void dMail_c::getListWord(dScript::Word_c *word) {
    u8 status = getStatus();
    u8 sender = getSenderKind();
    dHmnName::Word_c name;
    dString::Word_c text;
    word->clear();

    if (status == MAIL_STATUS_WRITTEN) {
        if (isToFutureSelf()) {
            fn_8016AE68(word, 0x79, "sys_2D/SYS_2D_menu");
        } else {
            mTo.setWord(&name, FALSE);
            fn_8016B15C(0, &name);
            fn_8016AE68(word, 0x6E, "sys_2D/SYS_2D_menu");
        }
    } else if (isFromText()) {
        mFrom.setWord(&text, FALSE);
        fn_8016B15C(0, &text);
        fn_8016AE68(word, 0x6F, "sys_2D/SYS_2D_menu");
    } else {
        if (sender == MAIL_FROM_NAMED) {
            mFrom.setWord(&name, FALSE);
            fn_8016B15C(0, &name);
        }
        fn_8016AE68(word, sFromMsgIds[sender], "sys_2D/SYS_2D_menu");
    }
}

// 801189C4
void dMailAddress_c::setWord(dScript::Word_c *word, BOOL skipNoName) {
    dHmnName::Word_c name;

    if (skipNoName) {
        switch (mType) {
        case MAIL_ADDR_ANIMAL_NONAME:
        case MAIL_ADDR_PLAYER_NONAME:
        case MAIL_ADDR_OTHER_NONAME:
            word->clear();
            return;
        }
    }

    switch (mType) {
    case MAIL_ADDR_PLAYER:
    case MAIL_ADDR_FUTURE_SELF:
        mPlayer.setWord(&name);
        break;
    case MAIL_ADDR_ANIMAL:
        mAnimal.setWord(&name, 10);
        break;
    case MAIL_ADDR_TEXT:
        word->set(mText, 0);
        return;
    }
    word->copy(&name, 0);
}

// 80118AD8
void dMailAddress_c::getName(wchar_t *out, BOOL skipNoName) {
    dHmnName::Word_c name;
    memset(out, 0, (PLAYER_NAME_LEN + 1) * sizeof(wchar_t));
    setWord(&name, skipNoName);
    memcpy(out, static_cast<dScript::Word_c &>(name).getBuffer(), (PLAYER_NAME_LEN + 1) * sizeof(wchar_t));
}

// 80118B70
void dMailAddress_c::setPlayer(const dPersonalID_c *pid) {
    mType = MAIL_ADDR_PLAYER;
    mPlayer = *pid;
}

// 80118C54
void dMailAddress_c::setAnimal(const dAnmPersonalID_c *animal) {
    mType = MAIL_ADDR_ANIMAL;
    mAnimal = *animal;
}

// 80118D44
void dMailAddress_c::setText(const wchar_t *text) {
    mType = MAIL_ADDR_TEXT;
    memset(this, 0, sizeof(mText));
    for (int i = 0; i < MAIL_ADDRESS_RAW_LEN; i++) {
        if (text[i] != 0) {
            mText[i] = text[i];
        }
    }
}

// 80118DF8
void dMailAddress_c::hideName() {
    switch (mType) {
    case MAIL_ADDR_PLAYER:
    case MAIL_ADDR_PLAYER_NONAME:
        mType = MAIL_ADDR_PLAYER_NONAME;
        break;
    case MAIL_ADDR_ANIMAL:
    case MAIL_ADDR_ANIMAL_NONAME:
        mType = MAIL_ADDR_ANIMAL_NONAME;
        break;
    default:
        mType = MAIL_ADDR_OTHER_NONAME;
        break;
    }
}

// 80118E3C
void dMailAddress_c::setFutureSelf() {
    mType = MAIL_ADDR_FUTURE_SELF;
}

// 80118E48
dPersonalID_c *dMailAddress_c::getPlayer() {
    switch (mType) {
    case MAIL_ADDR_PLAYER:
    case MAIL_ADDR_PLAYER_NONAME:
    case MAIL_ADDR_FUTURE_SELF:
        return &mPlayer;
    }
    return NULL;
}

// 80118E7C
dAnmPersonalID_c *dMailAddress_c::getAnimal() {
    switch (mType) {
    case MAIL_ADDR_ANIMAL:
    case MAIL_ADDR_ANIMAL_NONAME:
        return &mAnimal;
    }
    return NULL;
}

// 80118EA0
BOOL dMailAddress_c::isType(u8 type) {
    return mType == type;
}

// 80118EB4
dLetterStyle_c::dLetterStyle_c() {
    clear();
}

// 80118EE4
dLetterStyle_c::~dLetterStyle_c() {}

// 80118F24
void dLetterStyle_c::clear() {
    memset(static_cast<void*>(this), 0, sizeof(dLetterStyle_c));
    mFutureNamePos = 0xFF;
    mNamePos = 0xFF;
}

// 80118F64
void dLetterStyle_c::init() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    mFutureNamePos = 0;
    mNamePos = 0;

    sMenuWord.clear();
    fn_8016AE68(&sMenuWord, 0x7A, "sys_2D/SYS_2D_menu");
    memcpy(mHeader, static_cast<dScript::Word_c &>(sMenuWord).getBuffer(), sizeof(mHeader));
    dMail_c::findNamePos(mHeader, &mNamePos);

    sMenuWord.clear();
    fn_8016AE68(&sMenuWord, 0x7B, "sys_2D/SYS_2D_menu");
    memcpy(mFutureHeader, static_cast<dScript::Word_c &>(sMenuWord).getBuffer(), sizeof(mFutureHeader));
    dMail_c::findNamePos(mFutureHeader, &mFutureNamePos);

    fn_8016B050(0, &player->mPID);
    sMenuWord.clear();
    fn_8016AE68(&sMenuWord, 0x7C, "sys_2D/SYS_2D_menu");
    memcpy(mFooter, static_cast<dScript::Word_c &>(sMenuWord).getBuffer(), sizeof(mFooter));
}

// 80119098
void dLetterStyle_c::apply(dMail_c *mail) {
    if (mFutureNamePos >= 0x20) {
        init();
    }
    memcpy(mail->mFooter, mFooter, sizeof(mFooter));
    applyHeader(mail);
}

// 801190F8
void dLetterStyle_c::applyHeader(dMail_c *mail) {
    const wchar_t *header;
    if (mail->isToFutureSelf()) {
        mail->mNamePos = mFutureNamePos;
        header = mFutureHeader;
    } else {
        mail->mNamePos = mNamePos;
        header = mHeader;
    }
    memcpy(mail->mHeader, header, sizeof(mHeader));
}

// 80119164
void dLetterStyle_c::store(dMail_c *mail) {
    memcpy(mFooter, mail->mFooter, sizeof(mFooter));
    wchar_t *header;
    if (mail->isToFutureSelf()) {
        mFutureNamePos = mail->mNamePos;
        header = mFutureHeader;
    } else {
        mNamePos = mail->mNamePos;
        header = mHeader;
    }
    memcpy(header, mail->mHeader, sizeof(mHeader));
}

// 801191E0: __sinit, generated for the static words above.
