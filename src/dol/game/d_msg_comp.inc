// Included at the end of d_msg.cpp (one TU). With -sym on this file's functions get their own .text
// section after d_msg.cpp's, which ends at Word_c::setTagWord plus the tag classes' inline copies.

namespace dMsg {

Word_c::Word_c() {
    clear();
}

Word_c::~Word_c() {}

void Word_c::start(Comp_c *comp) {
    clear();
    mpComp = comp;
}

u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

int Word_c::getParseStart() {
    mScale = TEXT_SCALE_DEFAULT;
    mMaxScale = TEXT_SCALE_DEFAULT;
    mLineStart = 0;
    mPageLines = 0;
    mColor = mpComp->mWindowColor;
    mLineHasChar = 0;
    mHasSelect = 0;
    mAnimalID.clear();
    mHasAnimalID = 0;
    dScript::Word_c::getParseStart();
    int pos = 0;
    pos += insertColorTag(pos, mColor);
    pos += insertFeelTag(pos, mpComp->mFeel);
    return pos;
}

int Word_c::endLine(int pos) {
    int n = 0;
    if (pos != 0) {
        n = insertScaleTag(pos, mScale);
    }
    f32 scale = 0.01f * mMaxScale;
    f32 height = dFont::g_mainFont.mFont.GetHeight();
    f32 scaledHeight = scale * dFont::g_mainFont.mFont.GetHeight();
    f32 ascent = dFont::g_mainFont.mFont.GetAscent();
    f32 scaledAscent = scale * dFont::g_mainFont.mFont.GetAscent();
    n += insertLineOffsetTag(mLineStart, scaledAscent - ascent);
    mMaxScale = mScale;
    return n;
}

int Word_c::procNewLine(int pos) {
    if (mHasSelect) {
        return dScript::Word_c::procNewLine(pos);
    }
    int n = insertScaleTag(pos, TEXT_SCALE_DEFAULT) + 1;
    n += endLine(pos + n);
    mLineStart = pos + n;
    mLineHasChar = 0;
    mPageLines++;
    if (mPageLines >= MSG_PAGE_LINES) {
        mPageLines = 0;
        n += insertColorTag(pos + n, mColor);
    }
    return n;
}

int Word_c::procTag(int pos) {
    return dScript::Word_c::procTag(pos);
}

int Word_c::procEnd(int pos) {
    if (mHasSelect) {
        return dScript::Word_c::procEnd(pos);
    }
    endLine(0);
    return -1;
}

int Word_c::procChar(int pos) {
    getBuffer();
    if (!mLineHasChar) {
        mLineHasChar = 1;
    }
    return dScript::Word_c::procChar(pos);
}

int Word_c::procSelect(int pos) {
    u8 len = dScript::getTagLength(getBuffer() + pos);
    int n = endLine(0);
    mHasSelect = 1;
    return n + len;
}

// The tag builders copy a constant template (.sdata2 80751758..) and then set the escape and the parameter.
int Word_c::insertFeelTag(int pos, u32 id) {
    wchar_t tag[3] = {0, TAG_HEAD(6, 2), 0}; // feel tag (group 2, Seq_c::procFeel)
    tag[0] = TAG_ESCAPE;
    tag[2] = id;
    insert(tag, 3, pos);
    return 3;
}

int Word_c::insertColorTag(int pos, u8 color) {
    wchar_t tag[4] = {0, TAG_HEAD(8, dScript::TAG_GROUP_SYSTEM), 0x0000, 0}; // system 0: colour
    tag[0] = TAG_ESCAPE;
    tag[3] = color << 8;
    insert(tag, 4, pos);
    return 4;
}

int Word_c::insertLineOffsetTag(int pos, int offset) {
    wchar_t tag[4] = {0, TAG_HEAD(8, dScript::TAG_GROUP_SYSTEM), 0x0003, 0}; // system 3: line offset (y)
    tag[0] = TAG_ESCAPE;
    tag[3] = offset;
    insert(tag, 4, pos);
    return 4;
}

int Word_c::insertScaleTag(int pos, int scale) {
    wchar_t tag[4] = {0, TAG_HEAD(8, dScript::TAG_GROUP_SYSTEM), 0x0001, 0}; // system 1: scale
    tag[0] = TAG_ESCAPE;
    tag[3] = scale;
    insert(tag, 4, pos);
    return 4;
}

int Word_c::procNumber(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 idx = 0;
    std::memcpy(&idx, params, 1);
    if (idx >= 10) {
        return removeTag(pos);
    }
    dScript::Word_c *word = mpComp->getNumWord(idx);
    _18 = 1;
    return setTagWord(pos, word);
}

int Word_c::procRandomNumber(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 idx = 0;
    std::memcpy(&idx, params, 1);
    if (idx >= 10) {
        return removeTag(pos);
    }
    u16 min = 0;
    std::memcpy(&min, params + 1, 2);
    u16 max = 0;
    std::memcpy(&max, params + 3, 2);
    if (mpComp->getRandomNumber(idx) == MSG_RANDOM_NUMBER_NONE) {
        int value = min + cM::rndInt(max - min + 1);
        mpComp->setRandomNumber(idx, value);
        mpComp->formatNumber(idx, value, 12, dScript::NUM_FORMAT_REGION);
    }
    dScript::Word_c *word = mpComp->getNumWord(idx);
    _18 = 1;
    return setTagWord(pos, word);
}

int Word_c::procWord(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagId(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 idx = 0;
    std::memcpy(&idx, params, 1);
    if (idx >= 20) {
        return removeTag(pos);
    }
    if (mpComp->isWordCleared(idx)) {
        return removeTag(pos);
    }
    dScript::Word_c *word = mpComp->getWord(idx);
    _18 = 0;
    return setTagWord(pos, word);
}

} // namespace dMsg

// ---- 801A071C..801A2168: Word_c (word tags) ----

namespace dMsg {

int Word_c::procRandomWordTag(int pos, int kind) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    u8 useComp = 0;
    std::memcpy(&useComp, params + 1, 1);
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    dNpc::entry_c *entry = dNpc::getEntryByAnimal(&speaker->mID);
    if (!useComp) {
        entry = dNpc::getEntryByAnimal(&speaker->mID);
        if (entry == NULL) {
            useComp = 1;
        }
    }
    dAnmPersonalID_c *id = speaker->getID();
    const char *group = getGenderedGroup(kind, id);
    u16 msgId;
    u16 used[20];
    u16 *list = used;
    for (int i = 0; i < 20; i++) {
        if (useComp) {
            if ((list[i] = mpComp->getRandomWord(kind, i)) == 0 && entry != NULL) {
                list[i] = entry->mDialogueCache.getWord(kind, i);
            }
        } else {
            if (entry != NULL) {
                list[i] = entry->mDialogueCache.getWord(kind, i);
            }
            if (list[i] == 0) {
                list[i] = mpComp->getRandomWord(kind, i);
            }
        }
    }
    int n = procRandomWord(pos, kind, id, list);
    msgId = used[slot];
    if (useComp) {
        mpComp->setRandomWord(kind, slot, msgId);
    } else if (entry != NULL) {
        entry->mDialogueCache.setWord(kind, msgId, slot);
    }
    mpComp->setStringWord(slot, msgId, group);
    return n;
}

int Word_c::procDrink(int pos) { return procRandomWordTag(pos, 0); }
int Word_c::procObject(int pos) { return procRandomWordTag(pos, 1); }
int Word_c::procSweets(int pos) { return procRandomWordTag(pos, 2); }
int Word_c::procSports(int pos) { return procRandomWordTag(pos, 3); }
int Word_c::procMusic(int pos) { return procRandomWordTag(pos, 4); }
int Word_c::procFood(int pos) { return procRandomWordTag(pos, 5); }
int Word_c::procHobby(int pos) { return procRandomWordTag(pos, 6); }

int Word_c::procUnit(int pos) {
    int n = setTagWord(pos, &mpComp->mUnitWord);
    mpComp->updateUnitWord();
    return n;
}

int Word_c::procPlayerChar(int pos, u32 index) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    player->mPID.setWord(&name);
    wchar_t c = 0;
    if (!name.getChar(&c, index)) {
        return removeTag(pos);
    }
    removeTag(pos);
    int n = insertWord(pos, &c, 1, FALSE);
    int a = insertColorTag(pos, TEXT_COLOR_PLAYER);
    int b = insertColorTag(pos + a + n, mColor);
    mpComp->updateUnitWord();
    return n + a + b;
}

int Word_c::procPlayerChar0(int pos) { return procPlayerChar(pos, 0); }
int Word_c::procPlayerChar1(int pos) { return procPlayerChar(pos, 1); }
int Word_c::procPlayerChar2(int pos) { return procPlayerChar(pos, 2); }

int Word_c::procPlayer(int pos) {
    int n = procPlayerName(pos, &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID);
    int a = insertColorTag(pos, TEXT_COLOR_PLAYER);
    int b = insertColorTag(pos + a + n, mColor);
    mpComp->updateUnitWord();
    return n + a + b;
}

int Word_c::procAnimal(int pos) {
    int n = procAnimalName(pos, &mpComp->mpRcpt->getSpeakerAnimal()->mID);
    int a = insertColorTag(pos, TEXT_COLOR_ANIMAL);
    int b = insertColorTag(pos + a + n, mColor);
    return n + a + b;
}

void Word_c::pickAnimalA() {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    if (mpComp->mAnimalA == NULL) {
        mpComp->mAnimalA = getBestCompatAnimal(dSaveData_c::getTown()->mAnimals.mTown.mAnimals, speaker);
    }
}

int Word_c::procAnimalA(int pos) {
    pickAnimalA();
    dHmnName::Word_c name;
    setAnimalWord(&name, mpComp->mAnimalA, mpComp->mpRcpt->getSpeakerAnimal());
    int n = setTagWord(pos, &name);
    int a = insertColorTag(pos, TEXT_COLOR_ANIMAL);
    int b = insertColorTag(pos + a + n, mColor);
    _18 = 0;
    return n + a + b;
}

void Word_c::pickAnimalB() {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    if (mpComp->mAnimalB == NULL) {
        mpComp->mAnimalB = getWorstCompatAnimal(dSaveData_c::getTown()->mAnimals.mTown.mAnimals, speaker);
    }
}

int Word_c::procAnimalB(int pos) {
    pickAnimalB();
    dHmnName::Word_c name;
    setAnimalWord(&name, mpComp->mAnimalB, mpComp->mpRcpt->getSpeakerAnimal());
    int n = setTagWord(pos, &name);
    int a = insertColorTag(pos, TEXT_COLOR_ANIMAL);
    int b = insertColorTag(pos + a + n, mColor);
    _18 = 0;
    return n + a + b;
}

void Word_c::pickAnimalParam(u8 slot) {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    dAnimal_c *animal = mpComp->getAnimalParam(slot);
    dAnimal_c *listener = mpComp->mpRcpt->getListenerAnimal();
    if (animal == NULL) {
        dNpc::entry_c *entry = dNpc::getEntryByAnimal(&speaker->mID);
        if (entry != NULL) {
            const dAnmPersonalID_c *exclude[2] = {&speaker->mID, NULL};
            u32 num = 1;
            if (listener != NULL) {
                exclude[1] = &listener->mID;
                num = 2;
            }
            animal = entry->mDialogueCache.pickAnimal(slot, exclude, num);
        }
        mpComp->setAnimalParam(animal, slot);
    }
}

int Word_c::procAnimalParam(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagLength(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    pickAnimalParam(slot);
    dHmnName::Word_c name;
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    setAnimalWord(&name, mpComp->getAnimalParam(slot), speaker);
    int n = setTagWord(pos, &name);
    int a = insertColorTag(pos, TEXT_COLOR_ANIMAL);
    int b = insertColorTag(pos + a + n, mColor);
    _18 = 0;
    return n + a + b;
}

int Word_c::procTownName(int pos) {
    int n = dScript::Word_c::procTownName(pos);
    int a = insertColorTag(pos, TEXT_COLOR_TOWN);
    int b = insertColorTag(pos + a + n, mColor);
    return n + a + b;
}

int Word_c::procCatchphrase(int pos) {
    dHabitWord_c word;
    word.copy(&mpComp->mCatchphrase, 0);
    return setTagWord(pos, &word);
}

int Word_c::procPlayerNickname(int pos) {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    speaker->getPlayerCallName(&name, &player->mPID);
    int n = setTagWord(pos, &name);
    int a = insertColorTag(pos, TEXT_COLOR_PLAYER);
    int b = insertColorTag(pos + a + n, mColor);
    _18 = 0;
    mpComp->updateUnitWord();
    return n + a + b;
}

int Word_c::procImpression(int pos) {
    int index = getImpression(dPlayerMgr::Mgr_c::getCurrentPlayerRaw());
    dString::Word_c word(index + 1, "sys_STRING/STR_Impress");
    _18 = 0;
    return setTagWord(pos, &word);
}

int Word_c::procPlayerStarSign(int pos) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int sign = dTime_c::getStarSign(player->mBirthday.mMonth, player->mBirthday.mDay);
    dString::Word_c word(sign + 1, "sys_STRING/STR_Constellation");
    _18 = 1;
    return setTagWord(pos, &word);
}

int Word_c::procAnimalStarSign(int pos) {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    u32 month = speaker->getBirthMonth();
    u32 day = speaker->getBirthDay();
    int sign = dTime_c::getStarSign((u8)month, (u8)day);
    dString::Word_c word(sign + 1, "sys_STRING/STR_Constellation");
    _18 = 1;
    return setTagWord(pos, &word);
}

int Word_c::procGreeting(int pos) {
    const wchar_t *p = getBuffer() + pos;
    dScript::getTagLength(p);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    dGreetingWord_c word;
    if (!speaker->getGreeting(&word, &player->mPID)) {
        return removeTag(pos);
    }
    int len = getTextLength(p, 0);
    int i = 0;
    while (true) {
        int step;
        if (*p == TAG_ESCAPE) {
            step = dScript::getTagLength(p);
            u8 group = dScript::getTagGroup(p);
            u16 id = dScript::getTagId(p);
            if (group == dScript::TAG_GROUP_BLOCK && id == 7) { // block 7: end of the greeting
                break;
            }
        } else if (*p == 0) {
            return removeTag(pos);
        } else {
            step = 1;
        }
        if (i + step > len) {
            return removeTag(pos);
        }
        i += step;
        p += step;
    }
    erase(i, pos);
    return setTagWord(pos, &word);
}

int Word_c::procAnimalFlagBegin(int pos) {
    const wchar_t *p = getBuffer() + pos;
    dScript::getTagLength(p);
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    dNpc::entry_c *entry = dNpc::getEntryByAnimal(&mpComp->mpRcpt->getSpeakerAnimal()->mID);
    BOOL keep = TRUE;
    if (entry == NULL) {
        keep = FALSE;
    }
    if (!entry->mFlags.mTalked) {
        keep = FALSE;
    }
    int len = getTextLength(p, 0);
    int i = 0;
    while (true) {
        int step;
        if (*p == TAG_ESCAPE) {
            step = dScript::getTagLength(p);
            u8 group = dScript::getTagGroup(p);
            u16 id = dScript::getTagId(p);
            if (group == dScript::TAG_GROUP_BLOCK && id == 9) { // block 9: end of the flag block
                i += step;
                p += step;
                break;
            }
        } else if (*p == 0) {
            return removeTag(pos);
        } else if (!keep && _20 && dScript::toUpper((wchar_t *)p)) {
            clearCapitalize();
            break;
        } else {
            step = 1;
        }
        if (i + step > len) {
            return removeTag(pos);
        }
        i += step;
        p += step;
    }
    if (*p == L'\n') {
        i++;
    }
    if (!keep) {
        removeTag(pos);
    } else {
        erase(i, pos);
    }
    return 0;
}

int Word_c::procPageEnd(int pos) {
    u8 len = dScript::getTagLength(getBuffer() + pos);
    mPageLines = MSG_PAGE_LINES - 1;
    return len;
}

int Word_c::procPlayerGender(int pos) {
    return procByPlayerGender(pos, &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID);
}

int Word_c::procWordGender(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    dScript::getTagLength(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    return procByWordGender(pos, mpComp->getWord(slot));
}

int Word_c::procAnimalGender(int pos) {
    return procByAnimalGender(pos, &mAnimalID);
}

int Word_c::procPlayerElision(int pos) {
    return procElisionPlayer(pos, &dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID);
}

int Word_c::procAnimalElision(int pos) {
    return procElisionAnimal(pos, &mpComp->mpRcpt->getSpeakerAnimal()->mID);
}

int Word_c::procAnimalAElision(int pos) {
    pickAnimalA();
    dHmnName::Word_c name;
    mpComp->mAnimalA->mID.setWord(&name, LANGUAGE_NUM);
    return procElisionWord(pos, 0, &name);
}

int Word_c::procAnimalBElision(int pos) {
    pickAnimalB();
    dHmnName::Word_c name;
    mpComp->mAnimalB->mID.setWord(&name, LANGUAGE_NUM);
    return procElisionWord(pos, 0, &name);
}

int Word_c::procAnimalParamElision(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    pickAnimalParam(slot);
    dHmnName::Word_c name;
    mpComp->getAnimalParam(slot)->mID.setWord(&name, LANGUAGE_NUM);
    return procElisionWord(pos, 1, &name);
}

int Word_c::procPlayerNicknameElision(int pos) {
    dAnimal_c *speaker = mpComp->mpRcpt->getSpeakerAnimal();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dHmnName::Word_c name;
    speaker->getPlayerCallName(&name, &player->mPID);
    return procElisionWord(pos, 0, &name);
}

int Word_c::procWordElision(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    if (slot >= 20) {
        return removeTag(pos);
    }
    return procElisionWord(pos, 1, mpComp->getWord(slot));
}

int Word_c::procNumberPlural(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    const u8 *params = dScript::getTagParams(tag);
    dScript::getTagLength(tag);
    u8 slot = 0;
    std::memcpy(&slot, params, 1);
    if (slot >= 10) {
        return removeTag(pos);
    }
    return procPlural(pos, mpComp->getNumWord(slot));
}

int Word_c::procColor(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u8 len = dScript::getTagLength(tag);
    const u8 *params = dScript::getTagParams(tag);
    u8 color = 0;
    std::memcpy(&color, params, 1);
    mColor = color;
    return len;
}

int Word_c::procScale(int pos) {
    const wchar_t *tag = getBuffer() + pos;
    u8 len = dScript::getTagLength(tag);
    const u8 *params = dScript::getTagParams(tag);
    u16 scale = 0;
    std::memcpy(&scale, params, 2);
    if (!mLineHasChar) {
        mMaxScale = scale;
    }
    if (mMaxScale < scale) {
        mMaxScale = scale;
    }
    mScale = scale;
    return len;
}

int Word_c::procRubyTag(int pos) {
    if (getLanguage() == LANGUAGE_JP) {
        dScript::getTagLength(getBuffer() + pos);
        return procRubyJP(pos);
    }
    return removeTag(pos);
}

void Word_c::setAnimalWord(dScript::Word_c *word, dAnimal_c *animal, dAnimal_c *speaker) {
    animal->mID.setWord(word, LANGUAGE_NUM);
    mAnimalID.copy(&animal->mID);
    mHasAnimalID = 1;
    u8 looks = speaker->mID.getLooks(1);
    u16 index = dNpc::getPlayerUnitMessage(mAnimalID.getGender(0), looks);
    if (index != 0) {
        dString::Word_c unit(index, "sys_STRING/STR_Unit");
        mpComp->setUnitWord(&unit);
    } else {
        dString::Word_c unit;
        mpComp->setUnitWord(&unit);
    }
}

BOOL fn_801A20CC() { return dPad::isConfirmTriggered(); }
BOOL fn_801A20D0() { return dPad::isCancelTriggered(); }
BOOL fn_801A20D4() { return dPad::isCancelHeld(); }

BOOL isNextTrig() {
    BOOL ret = FALSE;
    if (fn_801A20CC() || fn_801A20D0()) {
        ret = TRUE;
    }
    return ret;
}

BOOL isSkipTrig() {
    BOOL ret = FALSE;
    if (fn_801A20D0() || fn_801A20D4()) {
        ret = TRUE;
    }
    return ret;
}

} // namespace dMsg

// ---- 801A2168..801A2384: Rcpt_c ----
namespace dMsg {
void Rcpt_c::init() {
    mpController = NULL;
    mpBmgData = NULL;
    std::memset(mMessageLabel, 0, sizeof(mMessageLabel));
    clearSpeakerName();
    mMessageCode = 0;
    mVoiceType = 1;
}

int Rcpt_c::getVoiceMode() { return VOICE_MODE_NONE; }
int Rcpt_c::getForcedVoiceMode() { return VOICE_MODE_NONE; }
int Rcpt_c::getVoiceMood() { return 0; }

void Rcpt_c::setSpeakerName(const u16 *name, u8 nameKind) {
    int length = dScript::getStringLength((const wchar_t *)name, 0, 0);
    std::memset(mSpeakerName, 0, sizeof(mSpeakerName));
    std::memcpy(mSpeakerName, name, length * sizeof(u16));
    mSpeakerNameKind = nameKind;
}

void Rcpt_c::clearSpeakerName() {
    std::memset(mSpeakerName, 0, sizeof(mSpeakerName));
    mSpeakerNameKind = 2;
}

void Rcpt_c::setMessageCode(u16 code) { mMessageCode = code; }

void Rcpt_c::setMessageLabel(const char *label) {
    if (std::strlen(label) != 0) {
        std::strncpy(mMessageLabel, label, sizeof(mMessageLabel) - 1);
        mpBmgData = NULL;
    }
}

void Rcpt_c::attachController(Comp_c *controller) { mpController = controller; }
void Rcpt_c::detachController() { mpController = NULL; }

void Rcpt_c::selectMessage(const EGG::MsgRes::MessageHeader *bmg) {}
void Rcpt_c::onMessageStart(int arg) {}
void Rcpt_c::onMessageEnd(int kind) {}
void Rcpt_c::onAnswer(int arg) {}
void Rcpt_c::onKeyWait() {}
void Rcpt_c::onPageWait() {}
void Rcpt_c::onTextEnd() {}
void Rcpt_c::onSelectStart() {}
void Rcpt_c::onTypeStart() {}
void Rcpt_c::onCustomTag(int no) {}
void Rcpt_c::setSpeakerFeel(u32 tag) {}
void Rcpt_c::resetMood() {}
void Rcpt_c::setMood1(int hours) {}
void Rcpt_c::setMood2(int hours) {}
void Rcpt_c::setMood3(int hours) {}
void Rcpt_c::setMood4(int hours) {}
void Rcpt_c::setSpeakerPlayer() {}
void Rcpt_c::setSpeakerNpc() {}
void Rcpt_c::setSpeakerPartner() {}
void Rcpt_c::speakerLookAtPlayer() {}
void Rcpt_c::speakerTurnToPlayer() {}
void Rcpt_c::speakerLookAtNpc() {}
void Rcpt_c::speakerTurnToNpc() {}
void Rcpt_c::speakerLookAtPartner() {}
void Rcpt_c::speakerTurnToPartner() {}
void Rcpt_c::changeSpeaker() {}
void Rcpt_c::playTownTune() {}
} // namespace dMsg

// ---- 801A2384..801A4518: Comp_c (profile, states) ----

void *dMsg::Comp_c::createMgr() {
    return new dMsg::Comp_c;
}

// 8074C128: the controller's profile (execute order 4, draw order 0xE3).
fProf::fBaseProfile_c g_profile_MSG = {&dMsg::Comp_c::createMgr, 4, 0xE3};

// 8074EAB4
dMsg::Comp_c *dMsg::Comp_c::mInstance;

dMsg::Comp_c::Comp_c() {}

dMsg::Comp_c::~Comp_c() {}

void dMsg::Comp_c::updateVoiceType() {
    u8 type = mpRcpt->getVoiceType();
    if (mVoiceType != MSG_VOICE_TYPE_NONE) {
        type = mVoiceType;
    }
    if (mCurVoiceType != type) {
        sndSetVoiceType(type);
    }
    mCurVoiceType = type;
}

int dMsg::Comp_c::create() {
    static const dState::mode_c<Comp_c>::Entry modes[] = {
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x94\xF1\x95\x5C\x8E\xA6", // hidden
         &dMsg::Comp_c::initializeHidden, &dMsg::Comp_c::executeHidden},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x8F\x6F\x8C\xBB\x82\xDC\x82\xC5\x82\xCC\x93\xC7\x82\xDD\x8D\x9E"
         "\x82\xDD", // load until shown
         &dMsg::Comp_c::initializeLoadAppear, &dMsg::Comp_c::executeLoadAppear},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x8E\x9F\x83\x79\x81\x5B\x83\x57\x82\xDC\x82\xC5\x82\xCC\x93\xC7"
         "\x82\xDD\x8D\x9E\x82\xDD", // load until the next page
         &dMsg::Comp_c::initializeLoadNextPage, &dMsg::Comp_c::executeLoadNextPage},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x8F\x6F\x8C\xBB", // appear
         &dMsg::Comp_c::initializeAppear, &dMsg::Comp_c::executeAppear},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x95\x5C\x8E\xA6\x91\x4F\x8F\x88\x97\x9D", // prepare
         &dMsg::Comp_c::initializePrepare, &dMsg::Comp_c::executePrepare},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x92\x75\x8A\xB7", // replace
         &dMsg::Comp_c::initializeReplace, &dMsg::Comp_c::executeReplace},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x95\x5C\x8E\xA6", // display
         &dMsg::Comp_c::initializeDisplay, &dMsg::Comp_c::executeDisplay},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x8A\xAE\x97\xB9", // complete
         &dMsg::Comp_c::initializeComplete, &dMsg::Comp_c::executeComplete},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x91\xDE\x8F\xEA", // exit
         &dMsg::Comp_c::initializeExit, &dMsg::Comp_c::executeExit},
        {"\x83\x81\x83\x62\x83\x5A\x81\x5B\x83\x57\x88\xEA\x8E\x9E\x91\xDE\x94\xF0", // suspend
         &dMsg::Comp_c::initializeSuspend, &dMsg::Comp_c::executeSuspend},
    };

    mInstance = this;
    if (!mResAcc.load("Layout/msg_win.arc")) {
        return NOT_READY;
    }
    if (!mSelect.create()) {
        return NOT_READY;
    }
    if (!mAnalogSelect.create()) {
        return NOT_READY;
    }
    dState::mode_c<Comp_c>::initialize("dMsg::Comp_c", modes, ARRAY_SIZE(modes), false);
    mLyt.mpResAcc = &mResAcc;
    mLyt.build("msg_win_00.brlyt", NULL);
    mLyt.createAnm(&mAnmWin, "msg_win_00_msgWinIn.brlan", NULL);
    mLyt.createAnm(&mAnmWaitIcon, "msg_win_00_waitIcn_00.brlan", NULL);
    mLyt.createAnm(&mAnmVib, "msg_win_00_msgWinVib1.brlan", NULL);
    mLyt.bind(&mAnmWin);
    mLyt.bind(&mAnmWaitIcon);
    mLyt.bind(&mAnmVib);
    mAnmWin.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmWaitIcon.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmVib.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmWin.setFrame(0.0f);
    mAnmWaitIcon.setFrame(mAnmWaitIcon.mFrameCtrl.mEndFrame);
    mAnmVib.setFrame(mAnmVib.mFrameCtrl.mEndFrame);
    mLyt.calc();
    nw4r::lyt::TextBox *text = mLyt.findTextBox("T_msg_00");
    mLyt.setFont(&dFont::g_mainFont.mFont);
    mSeq.create(this, text);
    mVoice.create(this);
    mpPageCursor = mLyt.findPane("pageCrsr_00");
    mpWaitIcon = mLyt.findPane("waitIcn_00");
    mpNamePane = mLyt.findPane("N_msgName");
    mpNameText = mLyt.findTextBox("T_msgName");
    mpNameFrame[0] = mLyt.findPane("msgName_frm_00");
    mpNameFrame[1] = mLyt.findPane("msgName_frm_01");
    mpNameFrame[2] = mLyt.findPane("msgName_frm_02");
    reset();
    return SUCCEEDED;
}

int dMsg::Comp_c::doDelete() {
    if (!mLoader.release()) {
        return NOT_READY;
    }
    if (!mResAcc.remove()) {
        return NOT_READY;
    }
    if (!dPrcMng_c::isDone()) {
        return NOT_READY;
    }
    if (!mSelect.remove()) {
        return NOT_READY;
    }
    if (!mAnalogSelect.remove()) {
        return NOT_READY;
    }
    if (!mSeq.remove()) {
        return NOT_READY;
    }
    mInstance = NULL;
    dBgm::VolStateTalk_c *talk = &dBgm::l_mgr.mVolCtrl.mTalk;
    if (talk->isState(1) || talk->isState(2)) {
        talk->end(0);
    }
    this->~Comp_c();
    return SUCCEEDED;
}

int dMsg::Comp_c::execute() {
    if (mNextStateFunc) {
        changeMode(mNextStateFunc);
        mNextStateFunc = NULL;
    }
    dState::base_c<Comp_c>::execute();
    mSelect.update();
    mAnalogSelect.update();
    return SUCCEEDED;
}

int dMsg::Comp_c::draw() {
    g_tagProcessor.mRectEnd = 0;
    g_tagProcessor.mDrawEnd = 0;
    g_tagProcessor.mLine = 0;
    mAnmWin.play();
    mAnmWaitIcon.play();
    mAnmVib.play();
    BOOL visible = !isState(&dMsg::Comp_c::executeHidden) && !isState(&dMsg::Comp_c::executeLoadAppear) &&
                   !isState(&dMsg::Comp_c::executeSuspend);
    if (visible) {
        mLyt.calc();
        mLyt.entry();
    }
    mSelect.draw();
    mAnalogSelect.draw();
    return SUCCEEDED;
}

void dMsg::Comp_c::reset() {
    getSelect()->reset();
    getAnalogSelect()->reset();
    mSuspendReq = SUSPEND_REQ_NONE;
    mNextStateFunc = NULL;
    mpCurSelect = NULL;
    mpBmg = NULL;
    mWaitIcon = 0;
    mWaitIconSe = 0;
    mPageDone = 0;
    mLock = 0;
    mForceNext = 0;
    mMouthOpen = 0;
    mNoCloseSe = 0;
    mBgmKeep = 0;
    mNextMsgCode = 0;
    mMemoryCode = 0;
    mStartReq = 0;
    mEndReq = 0;
    clearNumWords();
    clearWords();
    for (int i = 0; i < MSG_NICKNAME_NUM; i++) {
        mNicknames[i].clear();
    }
    mNicknameReq = 0;
    mNicknameSuffix = 0;
    mNicknameWord.clear();
    mNicknameCheckNum = 0;
    mCatchphrase.clear();
    mCatchphraseCheckNum = 0;
    std::memset(mBmgLabel, 0, sizeof(mBmgLabel));
    std::memset(mNextMsgLabel, 0, sizeof(mNextMsgLabel));
    mUnitWord.clear();
    mAnimalA = NULL;
    mAnimalB = NULL;
    for (int i = 0; i < ARRAY_SIZE(mAnimalParam); i++) {
        mAnimalParam[i] = NULL;
    }
    clearRandomWords();
    dHeap::msgHeap_p->free(3);
    mLoader.release();
    mVoiceType = MSG_VOICE_TYPE_NONE;
    mVoiceMood = MSG_VOICE_MOOD_NONE;
    mCurVoiceType = MSG_VOICE_TYPE_NONE;
    clearCheckWords();
    mStartMode = 0;
}

void dMsg::Comp_c::clearCheckWords() {
    mCheckNum = 0;
    mCheckHits = 0;
    for (int i = 0; i < MSG_CHECK_WORD_NUM; i++) {
        mCheckResults[i] = 0;
        mCheckWords[i] = NULL;
        mCheckStrs[i] = NULL;
    }
}

void dMsg::Comp_c::fn_801A3084() {
    mForceNext = 0;
}

void dMsg::Comp_c::fn_801A3090() {
    mForceNext = 0;
}

void dMsg::Comp_c::offerNickname(dScript::Word_c *word, u8 looks) {
    static const u16 suffixes[6] = {0x1F, 0x29, 0x33, 0x01, 0x0B, 0x15};
    u16 suffix = 1;
    if (looks < 6) {
        suffix = suffixes[looks];
    }
    if (isState(&dMsg::Comp_c::executeReplace)) {
        return; // the words are being checked
    }
    if (!mNicknameReq) {
        mNicknameWord.copy(word, 0);
        mNicknameReq = 1;
        mNicknameSuffix = suffix;
    }
}

dHmnName::Word_c *dMsg::Comp_c::getNickname(int idx) {
    return &mNicknames[idx];
}

void dMsg::Comp_c::requestStart(int mode) {
    if (!isState(&dMsg::Comp_c::executeSuspend)) {
        if (!isState(&dMsg::Comp_c::executeHidden)) {
            return;
        }
    }
    mStartMode = mode;
    mNextStateFunc = &dMsg::Comp_c::executeLoadAppear;
}

// The window colour's text colour (.sbss2 807539F0).
static const u8 l_windowColorTbl[7] = {0, 0, 0, 0, 0, 0, 0};

#ifdef MUST_MATCH
// 804FB15C / 804FB168: two member pointers (executeSuspend, executeHidden) nothing in the DOL uses: their
// function (an isState test of both, between requestStart and setWindowColor) was dead-stripped, they stayed.
#pragma force_active on
dMsg::Comp_c::StateFunc lbl_804FB15C = &dMsg::Comp_c::executeSuspend;
dMsg::Comp_c::StateFunc lbl_804FB168 = &dMsg::Comp_c::executeHidden;
#pragma force_active reset
#endif

void dMsg::Comp_c::setWindowColor(int color) {
    static const char *names[] = {
        "msg_win_00_msgWinGry.brlan", "msg_win_00_msgWinBrwn.brlan", "msg_win_00_msgWinYlw.brlan",
        "msg_win_00_msgWinPnk.brlan", "msg_win_00_msgWinOrng.brlan", "msg_win_00_msgWinGrn.brlan",
        "msg_win_00_msgWinRed.brlan",
    };
    mWindowColor = l_windowColorTbl[color];
    mLyt.changeAnm(&mAnmWin, names[color], NULL);
    mAnmWin.setToEnd();
    mLyt.calc();
}

void dMsg::Comp_c::setVibration(int level) {
    static const char *names[] = {
        "msg_win_00_msgWinVib1.brlan",
        "msg_win_00_msgWinVib2.brlan",
        "msg_win_00_msgWinVib3.brlan",
    };
    mLyt.changeAnm(&mAnmVib, names[level], NULL);
    mAnmVib.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmVib.setFrame(0.0f);
}

void dMsg::Comp_c::setVoiceChar(wchar_t c) {
    if (!mSeq.mNoWait) {
        mVoice.setChar(c);
    }
}

void dMsg::Comp_c::setMouthOpen(BOOL open) {
    if (open) {
        mMouthOpen = 1;
    } else {
        mMouthOpen = 0;
    }
}

void dMsg::Comp_c::closeMouth() {
    mMouthOpen = 0;
}

void dMsg::Comp_c::initializeHidden() {
    reset();
}

void dMsg::Comp_c::executeHidden() {}

void dMsg::Comp_c::applyNextMessage() {
    mpRcpt->setMessageCode(mNextMsgCode);
    mpRcpt->setMessageLabel(mNextMsgLabel);
    mNextMsgCode = 0;
    std::memset(mNextMsgLabel, 0, sizeof(mNextMsgLabel));
}

void dMsg::Comp_c::initializeLoadAppear() {
    if (mSuspendReq != SUSPEND_REQ_NONE) {
        applyNextMessage();
        mSuspendReq = SUSPEND_REQ_NONE;
    } else {
        dBgm::l_mgr.mVolCtrl.mTalk.start();
    }
}

void dMsg::Comp_c::executeLoadAppear() {
    if (loadBmg()) {
        changeMode(&dMsg::Comp_c::executeAppear);
    }
}

void dMsg::Comp_c::initializeLoadNextPage() {
    applyNextMessage();
}

void dMsg::Comp_c::executeLoadNextPage() {
    if (loadBmg()) {
        changeMode(&dMsg::Comp_c::executePrepare);
    }
}

void dMsg::Comp_c::initializeAppear() {
    mSeq.clearText();
    mpPageCursor->SetVisible(false);
    mpWaitIcon->SetVisible(false);
    updateSpeakerName();
    updateUnitWord();
    dScript::Res_c res(mpBmg);
    if (!res.isExistMsg(mpRcpt->getMessageCode(), 0)) {
        mpRcpt->setMessageCode(0);
    }
    if (mpRcpt->getMessageCode() >= res.getCount()) {
        mpRcpt->setMessageCode(0);
    }
    MsgInfo_s *info = (MsgInfo_s *)res.getEntry(mpRcpt->getMessageCode());
    setWindowColor(getWindowColor(info->mWindow));
    mLyt.changeAnm(&mAnmWin, "msg_win_00_msgWinIn.brlan", NULL);
    mAnmWin.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmWin.setFrame(0.0f);
    playSe(SE_text_window_open);
}

void dMsg::Comp_c::executeAppear() {
    if (mAnmWin.mFrameCtrl.isStop()) {
        changeMode(&dMsg::Comp_c::executePrepare);
    }
}

BOOL dMsg::Comp_c::isValidMsgCode(u16 code) {
    if (mpRcpt != NULL) {
        switch (mpRcpt->getMessageCode()) {
        case MSG_CODE_NONE:
        case MSG_CODE_HOLD:
        case MSG_CODE_INVALID:
            break;
        default:
            if (code == mpRcpt->getMessageCode()) {
                return FALSE;
            }
            break;
        }
    }
    return TRUE;
}

void dMsg::Comp_c::initializePrepare() {
    dScript::Res_c res(mpBmg);
    if (!res.isExistMsg(mpRcpt->getMessageCode(), 0)) {
        mpRcpt->setMessageCode(0);
    }
    if (mpRcpt->getMessageCode() >= res.getCount()) {
        mpRcpt->setMessageCode(0);
    }
    MsgInfo_s *info = (MsgInfo_s *)res.getEntry(mpRcpt->getMessageCode());
    u16 next = info->mNextCode;
    if (!isValidMsgCode(next)) {
        next = MSG_CODE_NONE;
    }
    if (mNextMsgCode == MSG_CODE_NONE) {
        setNextMsgCode(next);
    }
    if (std::strlen(mNextMsgLabel) == 0) {
        setNextMsgLabel(mpRcpt->getMessageLabel());
    }
    mMemoryCode = info->mMemoryCode;
    mFeel = info->mFeel;
    addFriendship(getFriendshipDelta(info->mFriendship));
    int color = getWindowColor(info->mWindow);
    int speed = getTextSpeed(info->mWindow);
    int align = getTextAlign(info->mWindow);
    mStartReq = info->mStartReq;
    mEndReq = info->mEndReq;
    mSeq.start(speed, align);
    mVoice.reset(0);
    mVoice.mMsgNoWait = speed == 1;
    setWindowColor(color);
    notifyMessageStart();
    clearCheckWords();
    dString::Word_c *word = mWords;
    for (int i = 0; i < MSG_WORD_NUM; i++, word++) {
        int len = word->getLength(0);
        if (word->_22 && len) {
            mCheckWords[mCheckNum] = word;
            mCheckStrs[mCheckNum] = word->getBuffer();
            mCheckNum++;
        }
    }
    if (mNicknameReq) {
        mNicknameCheckNum = mCheckNum;
        dHmnName::Word_c *nickname = mNicknames;
        for (int i = 0; i < MSG_NICKNAME_NUM; i++, nickname++) {
            if (dScript::makeNickname(nickname, &mNicknameWord, mNicknameSuffix + i)) {
                nickname->getLength(0);
                mCheckWords[mCheckNum] = nickname;
                mCheckStrs[mCheckNum] = nickname->getBuffer();
                mCheckNum++;
            } else {
                nickname->clear();
            }
        }
    }
    dAnimal_c *animal = mpRcpt->getSpeakerAnimal();
    if (animal != NULL) {
        mCatchphraseCheckNum = mCheckNum;
        BOOL itchy = mpRcpt->isSpeakerItchy();
        animal->getHabitWord(&mCatchphrase, getLanguage(), itchy);
        mCatchphrase._22 = 1;
        mCatchphrase._23 = 1;
        mCheckWords[mCheckNum] = &mCatchphrase;
        mCheckStrs[mCheckNum] = mCatchphrase.getBuffer();
        mCheckNum++;
    }
    updateSpeakerName();
}

void dMsg::Comp_c::executePrepare() {
    if (mCheckNum == 0) {
        changeMode(&dMsg::Comp_c::executeReplace);
    } else if (dPrcMng_c::startCheck((const u16 **)mCheckStrs, mCheckNum, (u8 *)mCheckResults, &mCheckHits)) {
        changeMode(&dMsg::Comp_c::executeReplace);
    }
}

void dMsg::Comp_c::initializeReplace() {}

void dMsg::Comp_c::executeReplace() {
    if (mCheckNum != 0) {
        if (!dPrcMng_c::isDone()) {
            return;
        }
        dAnimal_c *animal = mpRcpt->getSpeakerAnimal();
        for (int i = 0; i < mCheckNum; i++) {
            if (animal != NULL && i >= mCatchphraseCheckNum) {
                if (mCheckResults[i] == 1 && mCheckWords[i]->_23) {
                    mCheckWords[i]->censor();
                }
            } else if (mNicknameReq == 1 && i >= mNicknameCheckNum) {
                if (mCheckResults[i] == 1) {
                    mCheckWords[i]->clear();
                }
            } else if (mCheckResults[i] == 1 && mCheckWords[i]->_23) {
                mCheckWords[i]->censor();
            }
        }
        if (mNicknameReq) {
            mNicknameReq = 0;
        }
    }
    dScript::Res_c res(mpBmg);
    mSeq.setText(res.getMessage(mpRcpt->getMessageCode()));
    changeMode(&dMsg::Comp_c::executeDisplay);
}

void dMsg::Comp_c::initializeDisplay() {}

void dMsg::Comp_c::executeDisplay() {
    updateCursor();
    mSeq.execute();
    mVoice.calc();
    mSeq.calcTimer();
    if (mSeq.mState == dMsg::SEQ_STATE_SELECT || mSeq.mState == dMsg::SEQ_STATE_END) {
        if ((dMsg::isNextTrig() && !mSeq.isTimerAuto()) || mSeq.isTimer(0) || mSeq.isTimer(-1)) {
            if (mSeq.mTimer > 0) {
                mSeq.mTimer = 0;
            }
            changeMode(&dMsg::Comp_c::executeComplete);
        }
    }
    if (mSeq.mState != dMsg::SEQ_STATE_TYPE) {
        closeMouth();
    }
}

void dMsg::Comp_c::initializeComplete() {
    notifyMessageEnd();
    closeMouth();
    mPageDone = 0;
}

// ---- 801A4518..801A6468: Comp_c (words, window), Voice_c; weak functions at the end ----

// 801A4518
void dMsg::Comp_c::executeComplete() {
    updateCursor();
    if (mLock) {
        return;
    }
    if (!mPageDone) {
        if (mSeq.mState == dMsg::SEQ_STATE_SELECTING && mpCurSelect != NULL) {
            if (mpCurSelect->isClosed()) {
                setNextMsgCode(mpCurSelect->mCode);
                setNextMsgLabel(mpCurSelect->mLabel);
                notifyAnswer();
                mPageDone = 1;
                mpCurSelect->reset();
                mpCurSelect = NULL;
            }
            return;
        }
        if (!dMsg::isNextTrig() && mSeq.mTimer != 0 && !mForceNext) {
            return;
        }
        if (mSeq.mState == dMsg::SEQ_STATE_SELECT && mpCurSelect != NULL) {
            mpCurSelect->open();
            mSeq.setState(dMsg::SEQ_STATE_SELECTING);
            return;
        }
        mPageDone = 1;
        if (mNextMsgCode != MSG_CODE_NONE && mSuspendReq == SUSPEND_REQ_NONE && mSeq.mTyped) {
            playSe(SE_page_okuri);
        }
        return;
    }
    if (mNextMsgCode == MSG_CODE_NONE) {
        changeMode(&dMsg::Comp_c::executeExit);
    } else if (mSuspendReq != SUSPEND_REQ_NONE) {
        fn_801A3084();
        changeMode(&dMsg::Comp_c::executeExit);
    } else {
        fn_801A3090();
        changeMode(&dMsg::Comp_c::executeLoadNextPage);
    }
}

// 801A4A24
void dMsg::Comp_c::initializeExit() {
    mLyt.changeAnm(&mAnmWin, "msg_win_00_msgWinOut.brlan", NULL);
    mAnmWin.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mAnmWin.setFrame(0.0f);
    mpPageCursor->SetVisible(false);
    mpWaitIcon->SetVisible(false);
    if (!mNoCloseSe) {
        playSe(SE_text_window_close);
    }
    if (mSuspendReq == SUSPEND_REQ_NONE) {
        dBgm::l_mgr.mVolCtrl.mTalk.end(mBgmKeep);
    }
}

// 801A4AD0
void dMsg::Comp_c::executeExit() {
    if (mAnmWin.mFrameCtrl.isStop()) {
        if (mSuspendReq != SUSPEND_REQ_NONE) {
            changeMode(&dMsg::Comp_c::executeSuspend);
        } else {
            changeMode(&dMsg::Comp_c::executeHidden);
        }
    }
}

// 801A4D70
void dMsg::Comp_c::initializeSuspend() {}

// 801A4D74
void dMsg::Comp_c::executeSuspend() {}

// 801A4D78
void dMsg::Comp_c::notifyMessageStart() {
    mpRcpt->onMessageStart(mStartReq);
}

// 801A4D94
void dMsg::Comp_c::notifyMessageEnd() {
    if (mNextMsgCode == MSG_CODE_HOLD) {
        mLock = 1;
    }
    mpRcpt->onMessageEnd(mEndReq);
}

// 801A4DC4
void dMsg::Comp_c::notifyAnswer() {
    mpRcpt->onAnswer(0);
}

// 801A4DDC
void dMsg::Comp_c::attachActor(dMsg::Rcpt_c *rcpt) {
    mpRcpt = rcpt;
    rcpt->attachController(this);
}

// 801A4DF0
void dMsg::Comp_c::detachActor() {
    if (mpRcpt != NULL) {
        mpRcpt->detachController();
        mpRcpt = NULL;
    }
}

// 801A4E34
void dMsg::Comp_c::setNextMsgLabel(const char *label) {
    std::strncpy(mNextMsgLabel, label, 60);
}

// 801A4E44
void dMsg::Comp_c::setNextMsgCode(u16 code) {
    mNextMsgCode = code;
}

// 801A4E4C
BOOL dMsg::Comp_c::loadBmg() {
    dMsg::Rcpt_c *rcpt = mpRcpt;
    if (rcpt->getBmgData() != NULL) {
        mpBmg = rcpt->getBmgData();
    } else {
        if (mpBmg != NULL) {
            if (std::strlen(mBmgLabel) == 0 || std::strcmp(mBmgLabel, rcpt->getMessageLabel()) != 0) {
                mpBmg = NULL;
                mLoader.release();
                dHeap::msgHeap_p->free(3);
            }
        }
        if (mpBmg == NULL) {
            char path[0x50];
            std::sprintf(path, "%s%s.bmg", dScript::getScriptDir(), mpRcpt->getMessageLabel());
            mpBmg = static_cast<const EGG::MsgRes::MessageHeader *>(mLoader.request(path, 0, dHeap::msgHeap_p));
            std::memcpy(mBmgLabel, mpRcpt->getMessageLabel(), sizeof(mBmgLabel));
        }
    }
    if (mpBmg != NULL) {
        mpRcpt->selectMessage(mpBmg);
        return TRUE;
    }
    return FALSE;
}

// 801A4F74
void dMsg::Comp_c::setNumWord(int idx, const dScript::Word_c *word) {
    if (idx < MSG_NUM_WORD_NUM) {
        mNumWords[idx].clear();
        mNumWords[idx].copy(word, 0);
    }
}

// 801A4FD0
dScript::Word_c *dMsg::Comp_c::getNumWord(u8 idx) {
    return &mNumWords[idx];
}

// 801A4FE0
void dMsg::Comp_c::setRandomNumber(int idx, u16 id) {
    mRandomNumbers[idx] = id;
}

// 801A4FF4
u16 dMsg::Comp_c::getRandomNumber(int idx) {
    return mRandomNumbers[idx];
}

// 801A5008
void dMsg::Comp_c::setAnimalParam(dAnimal_c *animal, int idx) {
    mAnimalParam[idx] = animal;
}

// 801A501C
dAnimal_c *dMsg::Comp_c::getAnimalParam(int idx) {
    return mAnimalParam[idx];
}

// 801A5030
void dMsg::Comp_c::setNumber(int idx, int value, int width, int format) {
    dString::Word_c word;
    dScript::setNumber(&word, value, width, (dScript::NumberFormat_e)format);
    setNumWord(idx, &word);
}

// 801A50A8
void dMsg::Comp_c::setNumber4(int idx, int value) {
    dString::Word_c word;
    dScript::setNumber4(&word, value);
    setNumWord(idx, &word);
}

// 801A5118
void dMsg::Comp_c::setNumber2(int idx, int value) {
    dString::Word_c word;
    dScript::setNumber2(&word, value);
    setNumWord(idx, &word);
}

// 801A5188
void dMsg::Comp_c::setMinute(int idx, int value) {
    dString::Word_c word;
    dScript::setMinute(&word, value);
    setNumWord(idx, &word);
}

// 801A51F8
void dMsg::Comp_c::formatNumber(int idx, int value, int width, int format) {
    dString::Word_c word;
    dScript::formatNumber(&word, value, width, (dScript::NumberFormat_e)format, 0);
    setNumWord(idx, &word);
}

// 801A5274
void dMsg::Comp_c::setDecimal(int idx, int precision, double value) {
    dString::Word_c word;
    dScript::setDecimal(&word, precision, value);
    setNumWord(idx, &word);
}

// 801A52F4
void dMsg::Comp_c::setMonthName(int idx, int month) {
    dString::Word_c word;
    dScript::setMonthName(&word, month);
    setWord(idx, &word);
}

// 801A5364
void dMsg::Comp_c::setDayName(int idx, int day) {
    dString::Word_c word;
    dScript::setDayName(&word, day);
    setWord(idx, &word);
}

// 801A53D4
void dMsg::Comp_c::setPersonalName(int idx, const dPersonalID_c *id) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    dHmnName::Word_c word;
    id->setWord(&word);
    if (player != NULL) {
        if (player->mPID.fn_8013E740(id)) {
            word._22 = 1;
            word._23 = 1;
        } else {
            word._22 = 0;
            word._23 = 0;
        }
    }
    setWord(idx, &word);
}

// 801A548C
void dMsg::Comp_c::setPlayerName(int idx, const dPlayerID_c *id) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    dHmnName::Word_c word;
    id->setWord(&word);
    if (player != NULL) {
        if (player->mPID.player.fn_8013EB28(id)) {
            word._22 = 1;
            word._23 = 1;
        } else {
            word._22 = 0;
            word._23 = 0;
        }
    }
    setWord(idx, &word);
}

// 801A5544
void dMsg::Comp_c::setLandName(int idx, const dLandID_c *id) {
    dSaveData_c *save = dSaveData_c::getRaw();
    dLandNameWord_c word;
    // dLandID_c::setWord is not const (symbol setWord__9dLandID_cFPQ27dScript6Word_c).
    const_cast<dLandID_c *>(id)->setWord(&word);
    if (save->mLandID.isNewTown(id)) {
        word._22 = 1;
        word._23 = 1;
    } else {
        word._22 = 0;
        word._23 = 0;
    }
    setWord(idx, &word);
}

// 801A55F8
void dMsg::Comp_c::setAnmPersonalName(int idx, const dAnmPersonalID_c *id) {
    dHmnName::Word_c word;
    const_cast<dAnmPersonalID_c *>(id)->setWord(&word, 10);
    setWord(idx, &word);
}

// 801A566C
void dMsg::Comp_c::setItemName(int idx, const dItem::Item &item) {
    dItem::name_c name(item);
    setWord(idx, &name);
}

// 801A56D0
void dMsg::Comp_c::clearWord(int idx) {
    if (idx < MSG_WORD_NUM) {
        mWords[idx].clear();
        mWordCleared[idx] = 1;
    }
}

// 801A5728
void dMsg::Comp_c::setPersonalWord(int idx, const dScript::Word_c *src, const dPersonalID_c *pid) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    dString::Word_c word;
    word.copy(src, 0);
    if (player != NULL) {
        if (player->mPID.fn_8013E740(pid)) {
            word._22 = 1;
            word._23 = 1;
        } else {
            word._22 = 0;
            word._23 = 0;
        }
    }
    setWord(idx, &word);
}

// 801A57D8
void dMsg::Comp_c::setWord(int idx, const dScript::Word_c *word) {
    if (idx < MSG_WORD_NUM) {
        mWords[idx].clear();
        mWords[idx].copy(word, 0);
        mWordCleared[idx] = 0;
    }
}

// 801A5858
dScript::Word_c *dMsg::Comp_c::getWord(u8 idx) {
    return &mWords[idx];
}

// 801A5868
BOOL dMsg::Comp_c::isWordCleared(int idx) {
    return mWordCleared[idx];
}

// 801A5874
void dMsg::Comp_c::setStringWord(int idx, u16 msgId, const char *group) {
    dString::Word_c word(msgId, group);
    setWord(idx, &word);
}

// 801A58D4
u16 dMsg::Comp_c::getRandomWord(int kind, int idx) {
    return mRandomWords[kind][idx];
}

// 801A58F0
void dMsg::Comp_c::setRandomWord(int kind, int idx, u16 msgId) {
    mRandomWords[kind][idx] = msgId;
}

// 801A590C
void dMsg::Comp_c::clearWords() {
    for (int i = 0; i < MSG_WORD_NUM; i++) {
        mWords[i].clear();
        mWordCleared[i] = 0;
    }
}

// 801A5978
void dMsg::Comp_c::clearRandomWords() {
    std::memset(mRandomWords, 0, sizeof(mRandomWords));
}

// 801A598C
void dMsg::Comp_c::clearNumWords() {
    for (int i = 0; i < MSG_NUM_WORD_NUM; i++) {
        mNumWords[i].clear();
        mRandomNumbers[i] = MSG_RANDOM_NUMBER_NONE;
    }
}

// 801A5A00
void dMsg::Comp_c::openSelect() {
    if (mpCurSelect != &mAnalogSelect) {
        mSeq.setState(dMsg::SEQ_STATE_SELECT);
        mpCurSelect = &mSelect;
    }
}

// 801A5A4C
void dMsg::Comp_c::openAnalogSelect() {
    if (mpCurSelect != &mSelect) {
        mSeq.setState(dMsg::SEQ_STATE_SELECT);
        mpCurSelect = &mAnalogSelect;
    }
}

// 801A5A98
void dMsg::Comp_c::updateSpeakerName() {
    int len = dScript::getStringLength((const wchar_t *)mpRcpt->getSpeakerName(), 0, 0);
    if (len == 0) {
        mpNamePane->SetVisible(false);
        return;
    }
    mpNameText->SetString((const wchar_t *)mpRcpt->getSpeakerName(), 0, len);
    mpNamePane->SetVisible(true);
    switch (mpRcpt->getSpeakerNameKind()) {
    case 0:
        mpNameFrame[0]->SetVisible(true);
        mpNameFrame[1]->SetVisible(false);
        mpNameFrame[2]->SetVisible(false);
        break;
    case 1:
        mpNameFrame[0]->SetVisible(false);
        mpNameFrame[1]->SetVisible(true);
        mpNameFrame[2]->SetVisible(false);
        break;
    default:
        mpNameFrame[0]->SetVisible(false);
        mpNameFrame[1]->SetVisible(false);
        mpNameFrame[2]->SetVisible(true);
        break;
    }
}

// 801A5C00
void dMsg::Comp_c::addFriendship(s8 delta) {
    if (delta != 0) {
        dAnimal_c *animal = mpRcpt->getSpeakerAnimal();
        if (animal != NULL) {
            u32 memIdx = animal->getMemoryIdx(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mPID);
            dAnimalMemory_c *memory = animal->getMemory(memIdx);
            if (memory != NULL) {
                dNpc::sendAnimalFriendshipById(&animal->mID, memIdx, memory->addFriendship(delta));
            }
        }
    }
}

// 801A5CA8
void dMsg::Comp_c::updateCursor() {
    int cur = g_tagProcessor.mLine;
    int prev = g_tagProcessor.mTypeLine;
    if (cur != 0 && cur != prev) {
        g_tagProcessor.mTypeLine = cur;
    }
    updateVoiceType();
    int state = mSeq.mState;
    if (((state == dMsg::SEQ_STATE_PAGE_WAIT || state == dMsg::SEQ_STATE_END || state == dMsg::SEQ_STATE_SELECT) &&
         ((mSeq.isTimerAuto() && mSeq.mTimer <= 0) || !mSeq.isTimerAuto())) ||
        state == dMsg::SEQ_STATE_KEY_WAIT) {
        if (isState(&dMsg::Comp_c::executeComplete)) {
            if (mWaitIcon) {
                if (mWaitIconSe) {
                    holdSe(SE_PLEASE_WAIT_TSUUSHIN);
                } else {
                    holdSe(SE_PLEASE_WAIT);
                }
                showWaitIcon();
            } else if (mLock) {
                hideCursor();
            } else {
                showPageCursor();
            }
        } else {
            showPageCursor();
        }
    } else {
        hideCursor();
    }
}

// 801A5DE8
void dMsg::Comp_c::showWaitIcon() {
    if (!mpWaitIcon->IsVisible()) {
        mLyt.changeAnm(&mAnmWaitIcon, "msg_win_00_waitIcn_00.brlan", NULL);
        mAnmWaitIcon.mFrameCtrl.mFlags = 0;
        mAnmWaitIcon.setFrame(0.0f);
    }
    mpWaitIcon->SetVisible(true);
    mpPageCursor->SetVisible(false);
}

// 801A5E78
void dMsg::Comp_c::showPageCursor() {
    if (!mpPageCursor->IsVisible()) {
        mLyt.changeAnm(&mAnmWaitIcon, "msg_win_00_pageCrsr_00.brlan", NULL);
        mAnmWaitIcon.mFrameCtrl.mFlags = 0;
        mAnmWaitIcon.setFrame(0.0f);
    }
    mpWaitIcon->SetVisible(false);
    mpPageCursor->SetVisible(true);
}

// 801A5F08
void dMsg::Comp_c::hideCursor() {
    mpWaitIcon->SetVisible(false);
    mpPageCursor->SetVisible(false);
}

// 801A5F30
s8 dMsg::Comp_c::getFriendshipDelta(int idx) {
    static const s8 table[] = {0,  40, 35, 30, 25, 20, 15, 10, 9,   8,   7,   6,   5,   4,   3,   2,   1,
                               -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -15, -20, -25, -30, -35, -40};
    return table[idx];
}

// 801A5F44
int dMsg::Comp_c::getWindowColor(u8 idx) {
    static const int table[] = {0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6};
    return table[idx];
}

// 801A5F58
int dMsg::Comp_c::getTextSpeed(u8 idx) {
    static const int table[] = {0, 0, 1, 1, 2, 2, 1, 0, 1, 2, 0, 2, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1};
    return table[idx];
}

// 801A5F6C
int dMsg::Comp_c::getTextAlign(u8 idx) {
    static const int table[] = {0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    return table[idx];
}

// 801A5F80
void dMsg::Comp_c::setUnitWord(const dScript::Word_c *word) {
    mUnitWord.clear();
    mUnitWord.copy(word, 0);
}

// 801A5FD4
void dMsg::Comp_c::updateUnitWord() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dAnimal_c *animal = mpRcpt->getSpeakerAnimal();
    if (player != NULL && animal != NULL) {
        u16 msgId = dNpc::getPlayerUnitMessageForAnimal(&player->mPID, animal->mID.getLooks(1), animal);
        if (msgId != 0) {
            dString::Word_c word(msgId, "sys_STRING/STR_Unit");
            setUnitWord(&word);
        } else {
            dString::Word_c word;
            setUnitWord(&word);
        }
    }
}

namespace dMsg {

// 801A60B0
void Voice_c::create(Comp_c *comp) {
    mpOwner = comp;
}

// 801A60B8
void Voice_c::setChar(wchar_t c) {
    mPrevChar = mChar;
    mChar = c;
    mCharSet = 1;
}

// 801A60D0
void Voice_c::reset(int arg) {
    mPrevChar = 0;
    mChar = 0;
    mColor = 0;
    mMsgNoWait = 0;
    mTagNoWait = 0;
    mPaused = 0;
    mScale = 1.0f;
}

// 801A60F8
void Voice_c::calc() {
    if (mpOwner == NULL) {
        return;
    }
    BOOL isNone = FALSE;
    if (mColor == TEXT_COLOR_GREY && mpOwner->mVoiceType == MSG_VOICE_TYPE_NONE) {
        isNone = TRUE;
    }
    if (!mMsgNoWait && !mTagNoWait) {
        dMsg::Rcpt_c *rcpt = mpOwner->mpRcpt;
        int mode = 1;
        int mood = 0;
        if (rcpt != NULL) {
            mood = rcpt->getVoiceMood();
            if (mpOwner->mVoiceMood != MSG_VOICE_MOOD_NONE) {
                mood = mpOwner->mVoiceMood;
            }
            switch (dSaveOption::getVoiceMode()) {
            case 0:
                if (rcpt->getVoiceMode() != VOICE_MODE_NONE) {
                    mode = rcpt->getVoiceMode();
                    if (isNone) {
                        mode = 1;
                    }
                }
                break;
            case 1:
                mode = 1;
                break;
            case 2:
                mode = 2;
                break;
            }
            if (rcpt->getForcedVoiceMode() != VOICE_MODE_NONE) {
                mode = rcpt->getForcedVoiceMode();
            }
        }
        if (mode != 1 || (mPrevChar != 0 && mPrevChar != 0x20 && mPrevChar != 0x0A && mPrevChar != 0x30FC &&
                          mPrevChar != 0xFF5E && mPrevChar != 0x3F && mPrevChar != 0xFF1F && mPrevChar != 0x21 &&
                          mPrevChar != 0xFF01 && mPrevChar != 0x3000 && mPrevChar != 0x3001 &&
                          mPrevChar != 0x3002 && mPrevChar != 0x2C && mPrevChar != 0x2E)) {
            if (mPrevChar != 0 && mCharSet) {
                sndSpeakVoice(mode, mood, mPrevChar, mChar, mScale);
                mCharSet = 0;
            }
        }
        BOOL open = TRUE;
        if (isNone) {
            open = FALSE;
        } else if (dScript::isSeparator(mPrevChar)) {
            open = FALSE;
        }
        mpOwner->setMouthOpen(open);
    }
    if (mPaused) {
        mpOwner->closeMouth();
    }
}

} // namespace dMsg
