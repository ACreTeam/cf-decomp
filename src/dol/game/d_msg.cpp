// dMsg: message tag dispatch, windows and text (dMsg::Seq_c, Word_c, Ruby_c, RndJump_c, ...),
// the receipt Rcpt_c and the composer Comp_c (dState<Comp_c>). .text 8019C414..801A6468.
// From Word_c::Word_c on, the code lives in d_msg_comp.inc (included at the end; see notes/d_msg.txt).

// Include order sets the vtable / RTTI order (reverse class-definition order): dItem::name_c's
// weak vtable must come out last, and Rcpt_c's vtable first (Rcpt_c defined after the controller).
#include <game/game/d_item.hpp>
#include <game/game/d_script.hpp>
#include <cstring>
#include <game/game/d_msg_seq.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_scene.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <nw4r/lyt.h>
#include <game/game/d_msg_word.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_animal_id.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_tag_processor.hpp>
#include <cstdio>
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_font.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_save_building.hpp>

#include <game/game/d_msg_rcpt.hpp> // last: see above
#include <game/snd/snd_id.hpp>

typedef char dMsgRcptSizeCheck[sizeof(dMsg::Rcpt_c) == 0x64 ? 1 : -1];

// ---- 8019C414..8019EC88: RndJump_c, Seq_c (text output, tag dispatch) ----

namespace dMsg {

void RndJumpData_c::clear() {
    mMsgCode = 0;
    std::memset(mLabel, 0, sizeof(mLabel));
    mChoice = 0xFF;
}

void RndJumpData_c::set(u16 msgCode, const char *label, u8 choice) {
    mMsgCode = msgCode;
    std::memcpy(mLabel, label, sizeof(mLabel));
    mChoice = choice;
}

int RndJumpData_c::get(u16 msgCode, const char *label) {
    if (mMsgCode == msgCode && std::strcmp(mLabel, label) == 0) {
        return mChoice;
    }
    return -1;
}

void RndJump_c::clear() {
    for (int i = 0; i < RND_JUMP_NUM; i++) {
        mData[i].clear();
    }
}

void RndJump_c::add(u16 msgCode, const char *label, u8 choice) {
    int idx = RND_JUMP_NUM - 1;
    for (int i = 0; i < RND_JUMP_NUM; i++) {
        if (mData[i].get(msgCode, label) != -1) {
            idx = i;
        }
    }
    if (idx != 0) {
        for (int i = idx; i > 0; i--) {
            mData[i].set(mData[i - 1].mMsgCode, mData[i - 1].mLabel, mData[i - 1].mChoice);
        }
    }
    mData[0].set(msgCode, label, choice);
}

int RndJump_c::get(u16 msgCode, const char *label) {
    for (int i = 0; i < RND_JUMP_NUM; i++) {
        int choice = mData[i].get(msgCode, label);
        if (choice != -1) {
            return choice;
        }
    }
    return -1;
}

BOOL Seq_c::create(Comp_c *comp, nw4r::lyt::TextBox *textBox) {
    mpTextBox = textBox;
    mpTextBox->SetTagProcessor(&g_tagProcessor);
    mpTextBox->AllocStringBuffer(ARRAY_SIZE(mText));
    mpComp = comp;
    mRndJump.clear();
    return TRUE;
}

BOOL Seq_c::remove() {
    if (mpTextBox != NULL) {
        mpTextBox->FreeStringBuffer();
    }
    return TRUE;
}

int Seq_c::calcTimer() {
    return mTimer == -1 ? mTimer : sLib::calcTimer(&mTimer);
}

void Seq_c::setState(int state) {
    Rcpt_c *rcpt = mpComp->mpRcpt;
    switch (state) {
    case SEQ_STATE_TYPE:
        rcpt->onTypeStart();
        break;
    case SEQ_STATE_PAGE_WAIT:
        rcpt->onPageWait();
        break;
    case SEQ_STATE_KEY_WAIT:
        rcpt->onKeyWait();
        break;
    case SEQ_STATE_END:
        rcpt->onTextEnd();
        break;
    case SEQ_STATE_SELECT:
        rcpt->onSelectStart();
        break;
    }
    mState = state;
}

void Seq_c::execute() {
    if (mState == SEQ_STATE_TYPE) {
        if (!mNoSkip && isSkipTrig()) {
            mFast = TRUE;
            mWait = 0;
        }
    } else if (mState == SEQ_STATE_PAGE_WAIT) {
        if ((isNextTrig() && !mTimerAuto) || mTimer == 0) {
            playSe(MSG_SE_NEXT);
            nextPage();
            setState(SEQ_STATE_TYPE);
        }
    } else if (mState == SEQ_STATE_KEY_WAIT) {
        if (isNextTrig()) {
            mFast = FALSE;
            playSe(MSG_SE_NEXT);
            setState(SEQ_STATE_TYPE);
        }
    }

    wchar_t tagStart = TAG_ESCAPE;
    while (true) {
        if (!isTypeReady()) {
            return;
        }
        if (!procChar()) {
            return;
        }
        wchar_t *text = mText;
        // Cut the drawing after the typed characters (the rest of the line keeps the layout).
        wchar_t cursor[3] = {tagStart, TAG_HEAD(6, dScript::TAG_GROUP_SYSTEM), dScript::TAG_SYSTEM_TYPE_END};
        u32 len = dScript::getStringLength(mWord.getBuffer(), ARRAY_SIZE(mText), 0);
        int rest = len - (mPageStart + mPos);
        if (len + 3 > ARRAY_SIZE(mText)) {
            return;
        }
        if (rest < 0) {
            return;
        }
        const wchar_t *buf = mWord.getBuffer();
        std::memcpy(text, buf + mPageStart, mPos * sizeof(wchar_t));
        text += mPos;
        std::memcpy(text, cursor, sizeof(cursor));
        int lineLen = dScript::getLineLength((const u16 *)(mWord.getBuffer() + mPageStart + mPos), ARRAY_SIZE(mText));
        buf = mWord.getBuffer();
        std::memcpy(text + 3, buf + mPageStart + mPos, lineLen * sizeof(wchar_t));
        int textLen = dScript::getStringLength(mText, ARRAY_SIZE(mText), 0);
        mpTextBox->SetString(mText, 0, textLen);
    }
}

void Seq_c::clearText() {
    std::memset(mText, 0, sizeof(mText));
    mpTextBox->SetString(mText, 0, 0);
}

void Seq_c::start(int speed, int align) {
    mRuby.clear();
    mPos = 0;
    mTyped = FALSE;
    mPageStart = 0;
    mPageLines = 0;
    mLine = 0;
    mLineTotal = 0;
    mTextLen = 0;
    mTimer = -1;
    mTimerAuto = FALSE;
    mWait = 0;
    mWord.start(mpComp);
    clearText();
    setState(SEQ_STATE_TYPE);
    mFast = FALSE;
    switch (speed) {
    case SEQ_SPEED_NORMAL:
        mNoSkip = FALSE;
        mNoWait = FALSE;
        break;
    case SEQ_SPEED_NO_SKIP:
        mNoWait = FALSE;
        mNoSkip = TRUE;
        break;
    case SEQ_SPEED_NO_WAIT:
        mNoSkip = FALSE;
        mNoWait = TRUE;
        break;
    }
    switch (align) {
    case SEQ_ALIGN_CENTER:
        mpTextBox->SetTextPositionH(nw4r::lyt::HORIZONTALPOSITION_CENTER);
        break;
    case SEQ_ALIGN_LEFT:
        mpTextBox->SetTextPositionH(nw4r::lyt::HORIZONTALPOSITION_LEFT);
        break;
    }
    mpTextBox->SetTextPositionV(nw4r::lyt::VERTICALPOSITION_TOP);
    mpComp->mVoice.mTagNoWait = FALSE;
    mpComp->mVoice.mPaused = FALSE;
}

void Seq_c::setText(const wchar_t *text) {
    mWord.clear();
    mWord.set(text, 0);
    mTextLen = mWord.getTextLength(mWord.getBuffer(), 0);
}

void Seq_c::nextPage() {
    mTimer = -1;
    mTimerAuto = FALSE;
    mPageStart += mPos;
    mPos = 0;
    mPageLines = 0;
    mLine = 0;
    mFast = FALSE;
    clearText();
    g_tagProcessor.mLine = 0;
    g_tagProcessor.mTypeLine = 0;
}

BOOL Seq_c::procChar() {
    mpChar = mWord.getBuffer() + mPos + mPageStart;
    wchar_t c = *mpChar;
    if (c == TAG_ESCAPE) {
        u8 len = dScript::getTagLength(mpChar);
        if (len + mPos > mTextLen) {
            setState(SEQ_STATE_END);
            return FALSE;
        }
        mWait = 0;
        procTag();
        if (isTypeReady()) {
            procChar();
        }
    } else if (mPos + mPageStart + 1 > mTextLen) {
        if (!mTyped) {
            mTimer = 0;
        }
        mpComp->setVoiceChar(L' ');
        setState(SEQ_STATE_END);
        return FALSE;
    } else if (c == L'\n') {
        mTyped = TRUE;
        mPos++;
        mPageLines++;
        mLine++;
        mLineTotal++;
        mpComp->setVoiceChar(*mpChar);
        mpComp->mVoice.mPaused = FALSE;
        g_tagProcessor.mTypeLine = mLine;
        if (mPageLines >= MSG_PAGE_LINES) {
            setState(SEQ_STATE_PAGE_WAIT);
            return FALSE;
        }
    } else {
        mTyped = TRUE;
        mPos++;
        if (mRuby.isEnd()) {
            wchar_t ch = getChar(mpChar);
            BOOL sep = dScript::isSeparator(getChar(mpChar + 1));
            if (ch == 0x306F && sep) { // the particle "ha" is voiced "wa" (0x308F)
                mpComp->setVoiceChar(0x308F);
            } else {
                mpComp->setVoiceChar(ch);
            }
        }
        mpComp->mVoice.mPaused = FALSE;
    }
    return TRUE;
}

wchar_t Seq_c::getChar(const wchar_t *p) {
    while (true) {
        if (*p != TAG_ESCAPE) {
            break;
        }
        p += dScript::getTagLength(p);
    }
    return *p;
}

BOOL Seq_c::isTypeReady() {
    if ((u32)(mState - SEQ_STATE_PAGE_WAIT) <= 4) {
        mWait = 0;
        return FALSE;
    }
    if (mNoWait) {
        while (!mRuby.isEnd()) {
            mRuby.procChar();
        }
        return TRUE;
    }
    if (mWait != 0) {
        mWait--;
        return FALSE;
    }
    if (mFast) {
        mWait = 1;
    } else {
        mWait = 3;
    }
    if (!mRuby.isEnd()) {
        mRuby.procChar();
        return mRuby.stepBaseChar();
    }
    return TRUE;
}

void Seq_c::procTag() {
    static TagFunc s_funcs[] = {
        NULL,
        NULL,
        &Seq_c::procFeel,
        NULL,
        NULL,
        NULL,
        &Seq_c::procBlockTag,
        &Seq_c::procSelectTag,
        &Seq_c::procJumpTag,
        &Seq_c::procMoodTag,
        &Seq_c::procSpeakerTag,
        &Seq_c::procSoundTag,
        NULL,
    };
    static TagFunc s_systemFunc = &Seq_c::procSystemTag;
    TagFunc func = NULL;
    u8 group = dScript::getTagGroup(mpChar);
    mPos += dScript::getTagLength(mpChar);
    if (group == dScript::TAG_GROUP_SYSTEM) {
        func = s_systemFunc;
    } else {
        func = s_funcs[group];
    }
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procFeel() {
    mpComp->mpRcpt->setSpeakerFeel(dScript::getTagId(mpChar));
}

void Seq_c::procBlockTag() {
    static TagFunc s_funcs[] = {
        &Seq_c::procWait,
        &Seq_c::procKeyWait,
        &Seq_c::procNoWaitOn,
        &Seq_c::procNoWaitOff,
        &Seq_c::procNoSkipOn,
        &Seq_c::procNoSkipOff,
        NULL,
        NULL,
        NULL,
        NULL,
        &Seq_c::procTimer,
        &Seq_c::procTimerAuto,
        &Seq_c::procVibration1,
        &Seq_c::procVibration2,
        &Seq_c::procVibration3,
        &Seq_c::procPay,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &Seq_c::procCustom0,
        &Seq_c::procCustom1,
        &Seq_c::procCustom2,
        &Seq_c::procCustom3,
        &Seq_c::procCustom4,
        &Seq_c::procPageEnd,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procSelectTag() {
    static const TagFunc s_funcs[] = {
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procSelect,
        &Seq_c::procAnalogSelect,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procJumpTag() {
    static const TagFunc s_funcs[] = {
        &Seq_c::procRndJump2,
        &Seq_c::procRndJump3,
        &Seq_c::procFriendshipJump,
        &Seq_c::procPlayerGenderJump,
        &Seq_c::procHouseSizeJump,
        &Seq_c::procInsectCatalogJump,
        &Seq_c::procFishCatalogJump,
        &Seq_c::procAnimalNumJump,
        &Seq_c::procTimeOfDayJump,
        &Seq_c::procWeatherJump,
        &Seq_c::procPlayerNumJump,
        &Seq_c::procWeekdayJump,
        &Seq_c::procVisitorJump,
        &Seq_c::procSpeciesJump,
        &Seq_c::procMoneyJump,
        &Seq_c::procPocketFullJump,
        &Seq_c::procTanJump,
        &Seq_c::procOnlineJump,
        &Seq_c::procMemberNumJump,
        &Seq_c::procRndJump4,
        &Seq_c::procRndJump5,
        &Seq_c::procRndJump6,
        &Seq_c::procRndJump7,
        &Seq_c::procRndJump8,
        &Seq_c::procSeasonJump,
        &Seq_c::procShopStageJump,
        &Seq_c::procMiiMaskJump,
        &Seq_c::procBeeStingJump,
        &Seq_c::procSceneJump,
        &Seq_c::procHourJump,
        &Seq_c::procHeldItemJump,
        &Seq_c::procHairJump,
        &Seq_c::procBuildingJump,
        &Seq_c::procKicksJump,
        &Seq_c::procPhineasJump,
        &Seq_c::procFlag0x28Jump,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procMoodTag() {
    static const TagFunc s_funcs[] = {
        &Seq_c::procMoodReset,
        &Seq_c::procMood1,
        &Seq_c::procMood2,
        &Seq_c::procMood3,
        &Seq_c::procMood4,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procSpeakerTag() {
    static const TagFunc s_funcs[] = {
        &Seq_c::procSpeakerPlayer,
        &Seq_c::procSpeakerNpc,
        &Seq_c::procSpeakerPartner,
        &Seq_c::procLookAtPlayer,
        &Seq_c::procTurnToPlayer,
        &Seq_c::procLookAtNpc,
        &Seq_c::procTurnToNpc,
        &Seq_c::procLookAtPartner,
        &Seq_c::procTurnToPartner,
        &Seq_c::procChangeSpeaker,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procSoundTag() {
    static const TagFunc s_funcs[] = {
        &Seq_c::procSe,
        &Seq_c::fn_8019F80C,
        &Seq_c::fn_8019F810,
        &Seq_c::procTownTune,
        &Seq_c::procVoiceType,
        &Seq_c::procVoiceTypeReset,
        &Seq_c::procVoiceMood,
        &Seq_c::procVoiceMoodReset,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procSystemTag() {
    static TagFunc s_funcs[] = {
        &Seq_c::procColor,
        &Seq_c::procScale,
        &Seq_c::procRuby,
        NULL,
    };
    TagFunc func = s_funcs[dScript::getTagId(mpChar)];
    if (func) {
        (this->*func)();
    }
}

void Seq_c::procWait() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 wait = 0;
    std::memcpy(&wait, params, sizeof(wait));
    if (!mFast && !mNoWait) {
        mWait = wait;
        mpComp->setVoiceChar(L' ');
        mpComp->mVoice.mPaused = TRUE;
    }
}

void Seq_c::procKeyWait() {
    mpComp->setVoiceChar(L' ');
    setState(SEQ_STATE_KEY_WAIT);
}

void Seq_c::procNoWaitOn() {
    mNoWait = TRUE;
    mpComp->mVoice.mTagNoWait = TRUE;
}

void Seq_c::procNoWaitOff() {
    mNoWait = FALSE;
    mpComp->mVoice.mTagNoWait = FALSE;
}

void Seq_c::procNoSkipOn() {
    mNoSkip = TRUE;
    mFast = FALSE;
}

void Seq_c::procNoSkipOff() {
    mNoSkip = FALSE;
}

void Seq_c::procTimer() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 timer = 0;
    std::memcpy(&timer, params, sizeof(timer));
    mTimer = timer;
    mTimerAuto = FALSE;
    mpComp->setVoiceChar(L' ');
}

void Seq_c::procTimerAuto() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 timer = 0;
    std::memcpy(&timer, params, sizeof(timer));
    mTimer = timer;
    mTimerAuto = TRUE;
    mpComp->setVoiceChar(L' ');
}

void Seq_c::procVibration1() {
    mpComp->setVibration(0);
}

void Seq_c::procVibration2() {
    mpComp->setVibration(1);
}

void Seq_c::procVibration3() {
    mpComp->setVibration(2);
}

void Seq_c::procPay() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 amount = 0;
    std::memcpy(&amount, params, sizeof(amount));
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int money = player->getPocketMoney();
    if (money >= amount) {
        player->payMoney(amount, FALSE);
    }
}

void Seq_c::procCustom0() {
    mpComp->mpRcpt->onCustomTag(0);
}

void Seq_c::procCustom1() {
    mpComp->mpRcpt->onCustomTag(1);
}

void Seq_c::procCustom2() {
    mpComp->mpRcpt->onCustomTag(2);
}

void Seq_c::procCustom3() {
    mpComp->mpRcpt->onCustomTag(3);
}

void Seq_c::procCustom4() {
    mpComp->mpRcpt->onCustomTag(4);
}

void Seq_c::procPageEnd() {
    mpComp->setVoiceChar(L' ');
    mPageLines = MSG_PAGE_LINES - 1; // the next new line ends the page
}

void Seq_c::procSelect() {
    dSelect::Comp_c *select = &mpComp->mSelect;
    u16 id = dScript::getTagId(mpChar);
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setVoiceChar(L' ');
    int num;
    if (id >= dScript::TAG_SELECT_2_CANCEL) {
        num = id - 2;
        select->mCancel = num - 1;
    } else {
        num = id + 2;
    }
    select->mNum = num;
    const wchar_t *p = mWord.getBuffer() + mPos + mPageStart + 1;
    for (int i = 0; i < num; i++) {
        const wchar_t *line = p;
        while (true) {
            if (*p == TAG_ESCAPE) {
                p += dScript::getTagLength(p);
            } else if (*p == L'\n' || *p == 0) {
                p++;
                break;
            } else {
                p++;
            }
        }
        u16 code = 0;
        std::memcpy(&code, params + i * sizeof(u16), sizeof(code));
        dString::Word_c word;
        word.set(line, 1);
        dSelect::Separate_c *item = select->getSeparate(i);
        item->mWord.copy(&word, 0);
        if (code == SEQ_ANSWER_CODE_CANCEL) {
            item->mIsCancel = TRUE;
            code = 0;
        }
        item->mMsgCode = code;
    }
    mpComp->openSelect();
}

void Seq_c::procAnalogSelect() {
    dMsgAnalogSelect_c *select = &mpComp->mAnalogSelect;
    u16 id = dScript::getTagId(mpChar);
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setVoiceChar(L' ');
    const wchar_t *p = mWord.getBuffer() + mPos + mPageStart + 1;
    for (int i = 0; i < 2; i++) {
        const wchar_t *line = p;
        while (true) {
            if (*p == TAG_ESCAPE) {
                p += dScript::getTagLength(p);
            } else if (*p == L'\n' || *p == 0) {
                p++;
                break;
            } else {
                p++;
            }
        }
        dString::Word_c word;
        word.set(line, 1);
        select->setText(i, &word);
    }
    for (int i = 0; i < 8; i++) {
        u16 code = 0;
        std::memcpy(&code, params + i * sizeof(u16), sizeof(code));
        select->setCode(i, code, 0);
    }
    mpComp->openAnalogSelect();
}

int Seq_c::selectRndJump(int choice, int num) {
    return choice;
}

void Seq_c::procRndJump(int num) {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 codes[SEQ_JUMP_CHOICE_MAX];
    Rcpt_c *rcpt = mpComp->mpRcpt;
    const char *label = rcpt->getMessageLabel();
    u16 msgCode = rcpt->getMessageCode();
    for (int i = 0; i < num; i++) {
        std::memcpy(&codes[i], params + i * sizeof(u16), sizeof(u16));
        if (!mpComp->isValidMsgCode(codes[i])) {
            mpComp->setNextMsgCode(0);
            return;
        }
    }
    u32 choice = cM::rndInt(num);
    if (choice == mRndJump.get(msgCode, label)) {
        u32 next = choice + 1;
        if (next == num) {
            next = choice - 1;
        }
        choice = next;
    }
    int sel = selectRndJump(choice, num);
    mRndJump.add(msgCode, label, sel);
    mpComp->setNextMsgCode(codes[sel]);
}

void Seq_c::procRndJump2() {
    procRndJump(2);
}

void Seq_c::procRndJump3() {
    procRndJump(3);
}

void Seq_c::procFriendshipJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 friendship = 0;
    int choice = 1;
    std::memcpy(&friendship, params, sizeof(friendship));
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dAnimal_c *animal = mpComp->mpRcpt->getSpeakerAnimal();
    if (player != NULL && animal != NULL) {
        dAnimalMemory_c *memory = animal->findMemory(&player->mPID);
        if (memory != NULL && memory->getFriendship() >= friendship) {
            choice = 0;
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params + 1, choice, 2));
}

void Seq_c::procPlayerGenderJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 gender = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        gender = player->mPID.player.getGender();
    }
    mpComp->setNextMsgCode(getJumpCode(params, gender, 2));
}

void Seq_c::procHouseSizeJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    int value = 0;
    if (player != NULL) {
        value = player->getHouseSize();
    }
    mpComp->setNextMsgCode(getJumpCode(params, value, 5));
}

void Seq_c::procInsectCatalogJump() {
    dCatalog_c *catalog;
    const u8 *params = dScript::getTagParams(mpChar);
    int choice = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        catalog = &player->mCatalog;
        if (catalog != NULL) {
            u32 registered = catalog->countRegistered(7);
            u32 all = catalog->countAll(7);
            f32 rate = (f32)registered / (f32)all;
            if (catalog->isComplete(7)) {
                choice = 2;
            } else if (rate >= 0.6f) {
                choice = 1;
            }
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 3));
}

void Seq_c::procFishCatalogJump() {
    dCatalog_c *catalog;
    const u8 *params = dScript::getTagParams(mpChar);
    int choice = 0;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        catalog = &player->mCatalog;
        if (catalog != NULL) {
            u32 registered = catalog->countRegistered(8);
            u32 all = catalog->countAll(8);
            f32 rate = (f32)registered / (f32)all;
            if (catalog->isComplete(8)) {
                choice = 2;
            } else if (rate >= 0.6f) {
                choice = 1;
            }
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 3));
}

void Seq_c::procAnimalNumJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    int num = dSaveData_c::getAnimalBlock()->getAnimalNum();
    int choice = CLAMP(num - 6, 0, 4);
    mpComp->setNextMsgCode(getJumpCode(params, choice, 5));
}

void Seq_c::procTimeOfDayJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dTime_c time = *dTime_c::getCurrent();
    mpComp->setNextMsgCode(getJumpCode(params, time.getTimeOfDay(), 4));
}

void Seq_c::procWeatherJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    int weather = 0;
    if (dWeather_c::sInstance != NULL) {
        weather = dWeather_c::sInstance->getMessageWeather();
    }
    mpComp->setNextMsgCode(getJumpCode(params, weather, 8));
}

void Seq_c::procPlayerNumJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    int num = dPrivateData_c::count(dSaveData_c::getRaw()->mPlayers);
    int choice = num;
    if (num > 0) {
        choice = num - 1;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 4));
}

void Seq_c::procWeekdayJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dTime_c time = *dTime_c::getCurrent();
    int choice = time.wday == 0 ? 6 : time.wday - 1;
    mpComp->setNextMsgCode(getJumpCode(params, choice, 7));
}

void Seq_c::procVisitorJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int choice = TRUE;
    if (player != NULL) {
        choice = player->mPID.land != dSaveData_c::getRaw()->mLandID;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procSpeciesJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 species = 0;
    std::memcpy(&species, params, sizeof(species));
    dAnimal_c *animal = mpComp->mpRcpt->getSpeakerAnimal();
    int choice = 1;
    if (animal != NULL) {
        choice = (u8)animal->getSpecies() != species;
    }
    mpComp->setNextMsgCode(getJumpCode(params + 2, choice, 2));
}

void Seq_c::procMoneyJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u32 price = 0;
    std::memcpy(&price, params, sizeof(price));
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int choice = 0;
    if (player != NULL) {
        int money = player->getPocketMoney();
        if (money < price) {
            choice = 1;
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params + 4, choice, 2));
}

void Seq_c::procPocketFullJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int choice = 0;
    if (player != NULL) {
        int idx = player->findEmptyPocket(0);
        if (idx < 0) {
            choice = 1;
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procTanJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int choice = 0;
    if (player != NULL) {
        choice = player->mTan >> 1;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 8));
}

void Seq_c::procOnlineJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setNextMsgCode(getJumpCode(params, !dNet::isSessionActive(), 2));
}

void Seq_c::procMemberNumJump() {
    int num = dNet::getMemberNum();
    const u8 *params = dScript::getTagParams(mpChar);
    int choice = 0;
    if (num > 1) {
        choice = num - 1;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 4));
}

void Seq_c::procRndJump4() {
    procRndJump(4);
}

void Seq_c::procRndJump5() {
    procRndJump(5);
}

void Seq_c::procRndJump6() {
    procRndJump(6);
}

void Seq_c::procRndJump7() {
    procRndJump(7);
}

void Seq_c::procRndJump8() {
    procRndJump(8);
}

void Seq_c::procSeasonJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setNextMsgCode(getJumpCode(params, dTime_c::getCurrentSeason(), 4));
}

void Seq_c::procShopStageJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setNextMsgCode(getJumpCode(params, dSaveData_c::getTown()->mShops.mShop.mStage, 4));
}

void Seq_c::procMiiMaskJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    int choice = 1;
    if (player != NULL) {
        choice = player->mMiiMask == 0;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procBeeStingJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int choice = 1;
    if (player != NULL) {
        choice = !player->isFlag0(PRIVATE_FLAG0_BEE_STUNG);
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procSceneJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    mpComp->setNextMsgCode(getJumpCode(params, dScene::getCurrentScene() != 0, 2));
}

void Seq_c::procHourJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 from = 0;
    std::memcpy(&from, params, sizeof(from));
    u8 to = 0;
    std::memcpy(&to, params + 1, sizeof(to));
    dTime_c time = *dTime_c::getCurrent();
    u8 hour = time.hour;
    int choice = 1;
    if (from == to) {
        if (hour == from) {
            choice = 0;
        }
    } else if (from < to) {
        if (hour >= from && hour < to) {
            choice = 0;
        }
    } else if (hour >= from || hour < to) {
        choice = 0;
    }
    mpComp->setNextMsgCode(getJumpCode(params + 2, choice, 2));
}

} // namespace dMsg

// ---- 8019EC88..801A071C: Seq_c tag handlers, Ruby_c, Word_c (start) ----

namespace dMsg {

// held is item's kind: item is the given firework.
static inline bool isHanabi(const dItem::Item *item, int held, int idx) {
    return held == dItem::KIND_HANABI && item->isSame(dItem::Item(idx));
}

void Seq_c::procHeldItemJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 kind = 0;
    std::memcpy(&kind, params, 1);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    BOOL choice = TRUE;
    if (player != NULL) {
        if (dScene::getCurrentScene() == dScene::SCENE_TOWN) {
            int held = player->mEquipment.mHeld.getKind();
            switch (kind) {
            case JUMP_HELD_UMBRELLA:
                if (held == dItem::KIND_UMBRELLA) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_WINDMILL:
                if (held == dItem::KIND_WINDMILL) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_BALLOON:
                if (held == dItem::KIND_BALLOON) {
                    choice = FALSE;
                }
                break;
            }
        } else if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
            const dItem::Item *item = player->getHeldItem();
            int held = item->getKind();
            switch (kind) {
            case JUMP_HELD_UMBRELLA:
                if (held == dItem::KIND_UMBRELLA) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_FISHINGROD:
                if (held == dItem::KIND_FISHINGROD || held == dItem::KIND_SILVER_FISHINGROD || held == dItem::KIND_GOLD_FISHINGROD) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_NET:
                if (held == dItem::KIND_NET || held == dItem::KIND_SILVER_NET || held == dItem::KIND_GOLD_NET) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_SCOOP:
                if (held == dItem::KIND_SCOOP || held == dItem::KIND_SILVER_SCOOP || held == dItem::KIND_GOLD_SCOOP) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_WATERING:
                if (held == dItem::KIND_WATERING || held == dItem::KIND_SILVER_WATERING || held == dItem::KIND_GOLD_WATERING) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_AXE:
                if (held == dItem::KIND_AXE || held == dItem::KIND_SILVER_AXE || held == dItem::KIND_GOLD_AXE) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_PACHINKO:
                if (held == dItem::KIND_PACHINKO || held == dItem::KIND_GOLD_PACHINKO || held == dItem::KIND_SILVER_PACHINKO) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_CRACKER:
                if (held == dItem::KIND_CRACKER) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_SYABON:
                if (held == dItem::KIND_SYABON) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_WINDMILL:
                if (held == dItem::KIND_WINDMILL) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_SPARKLER:
                if (isHanabi(item, held, dItem::ITEM_IDX_SPARKLER)) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_ROMAN_CANDLE:
                if (isHanabi(item, held, dItem::ITEM_IDX_ROMAN_CANDLE)) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_BALLOON:
                if (held == dItem::KIND_BALLOON) {
                    choice = FALSE;
                }
                break;
            case JUMP_HELD_FLOWER:
                if (held == dItem::KIND_FLOWER) {
                    choice = FALSE;
                }
                break;
            }
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params + 1, choice, 2));
}

static inline bool isHelmet(int bone) {
    return bone == 3 || bone == 4 || bone == 5 || bone == 6;
}

// Headwear whose hide-bone kind (2..6) hides the hair.
static inline bool hidesHair(const dItem::Item &item) {
    int bone = item.getHideBone();
    return bone == 2 || isHelmet(bone);
}

void Seq_c::procHairJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 mode = 0;
    std::memcpy(&mode, params, 1);
    u8 hair = 0;
    std::memcpy(&hair, params + 1, 1);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    BOOL choice = FALSE;
    if (player != NULL) {
        if (player->mMiiMask != 0 || hidesHair(player->mEquipment.mHat)) {
            choice = TRUE;
        } else {
            u8 alt = hair + 13;
            choice = TRUE;
            u8 cur = player->mHair;
            if (mode == 0) {
                if (cur == hair) {
                    choice = FALSE;
                }
            } else if (mode == 1) {
                if (cur == alt) {
                    choice = FALSE;
                }
            } else if (cur == hair || cur == alt) {
                choice = FALSE;
            }
        }
    }
    mpComp->setNextMsgCode(getJumpCode(params + 2, choice, 2));
}

void Seq_c::procBuildingJump() {
    BOOL built;
    const u8 *params = dScript::getTagParams(mpChar);
    built = FALSE;
    u8 kind = 0;
    std::memcpy(&kind, params, 1);
    int x, z;
    BOOL choice = FALSE;
    switch (kind) {
    case JUMP_BUILDING_BRIDGE: {
        dSaveMainField_c *field = &dSaveData_c::getRaw()->mMainField;
        if (field->mBridgeBlockX == -1 && field->mBridgeBlockZ == -1) {
            built = TRUE;
        }
        if (built) {
            choice = TRUE;
        }
        break;
    }
    case JUMP_BUILDING_FOUNTAIN: {
        dItem::Item item((u16)BUILDING_FOUNTAIN);
        if (dSaveBuildingList_c::get()->getPos(&x, &z, &item, 1)) {
            choice = TRUE;
        }
        break;
    }
    case JUMP_BUILDING_WINDMILL: {
        dItem::Item item((u16)BUILDING_WINDMILL);
        if (dSaveBuildingList_c::get()->getPos(&x, &z, &item, 1)) {
            choice = TRUE;
        }
        break;
    }
    case JUMP_BUILDING_LIGHTHOUSE: {
        dItem::Item item((u16)BUILDING_LIGHTHOUSE);
        if (dSaveBuildingList_c::get()->getPos(&x, &z, &item, 1)) {
            choice = TRUE;
        }
        break;
    }
    }
    mpComp->setNextMsgCode(getJumpCode(params + 1, choice, 2));
}

void Seq_c::procKicksJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dItem::Item key((u16)0x804B);
    dDemoActor_c *present = dNpcList::getSpByKey(key);
    BOOL choice = TRUE;
    if (present) {
        choice = FALSE;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procPhineasJump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dItem::Item key((u16)0x805F);
    dDemoActor_c *present = dNpcList::getSpByKey(key);
    BOOL choice = TRUE;
    if (present) {
        choice = FALSE;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

void Seq_c::procFlag0x28Jump() {
    const u8 *params = dScript::getTagParams(mpChar);
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    BOOL choice = TRUE;
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        choice = FALSE;
    }
    mpComp->setNextMsgCode(getJumpCode(params, choice, 2));
}

u16 Seq_c::getJumpCode(const u8 *params, int choice, int num) {
    u16 codes[SEQ_JUMP_CHOICE_MAX];
    for (int i = 0; i < num; i++) {
        std::memcpy(&codes[i], params + i * sizeof(u16), sizeof(u16));
        if (!mpComp->isValidMsgCode(codes[i])) {
            return 0;
        }
    }
    return codes[selectRndJump(choice, num)];
}

void Seq_c::procMoodReset() {
    mpComp->mpRcpt->resetMood();
}

void Seq_c::procMood1() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 hours = 0;
    std::memcpy(&hours, params, 1);
    mpComp->mpRcpt->setMood1(hours);
}

void Seq_c::procMood2() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 hours = 0;
    std::memcpy(&hours, params, 1);
    mpComp->mpRcpt->setMood2(hours);
}

void Seq_c::procMood3() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 hours = 0;
    std::memcpy(&hours, params, 1);
    mpComp->mpRcpt->setMood3(hours);
}

void Seq_c::procMood4() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 hours = 0;
    std::memcpy(&hours, params, 1);
    mpComp->mpRcpt->setMood4(hours);
}

void Seq_c::procSpeakerPlayer() {
    mpComp->mpRcpt->setSpeakerPlayer();
}

void Seq_c::procSpeakerNpc() {
    mpComp->mpRcpt->setSpeakerNpc();
}

void Seq_c::procSpeakerPartner() {
    mpComp->mpRcpt->setSpeakerPartner();
}

void Seq_c::procLookAtPlayer() {
    mpComp->mpRcpt->speakerLookAtPlayer();
}

void Seq_c::procTurnToPlayer() {
    mpComp->mpRcpt->speakerTurnToPlayer();
}

void Seq_c::procLookAtNpc() {
    mpComp->mpRcpt->speakerLookAtNpc();
}

void Seq_c::procTurnToNpc() {
    mpComp->mpRcpt->speakerTurnToNpc();
}

void Seq_c::procLookAtPartner() {
    mpComp->mpRcpt->speakerLookAtPartner();
}

void Seq_c::procTurnToPartner() {
    mpComp->mpRcpt->speakerTurnToPartner();
}

void Seq_c::procChangeSpeaker() {
    mpComp->mpRcpt->changeSpeaker();
}

void Seq_c::procColor() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 value = 0;
    std::memcpy(&value, params, 1);
    mpComp->mVoice.mColor = value;
}

void Seq_c::procScale() {
    const u8 *params = dScript::getTagParams(mpChar);
    u16 value = 0;
    std::memcpy(&value, params, 2);
    mpComp->mVoice.mScale = 0.01f * value;
}

void Seq_c::procRuby() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 len = dScript::getTagLength(mpChar);
    u8 count = 0;
    std::memcpy(&count, params, 1);
    mRuby.set(mpComp, count, len - 4, (const wchar_t *)(params + 1));
}

void Seq_c::procSe() {
    playSe(SE_Eat_Gokun);
}

void Seq_c::fn_8019F80C() {}

void Seq_c::fn_8019F810() {}

void Seq_c::procTownTune() {
    mpComp->mpRcpt->playTownTune();
}

void Seq_c::procVoiceType() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 kind = 0;
    std::memcpy(&kind, params, 1);
    int value = MSG_VOICE_TYPE_NONE;
    switch (kind) {
    case 0:
        value = 5;
        break;
    case 1:
        value = 6;
        break;
    case 2:
        value = 7;
        break;
    case 3:
        value = 8;
        break;
    case 4:
        value = 9;
        break;
    case 5:
        value = 10;
        break;
    case 6:
        value = 11;
        break;
    case 7:
        value = 12;
        break;
    }
    if (value != MSG_VOICE_TYPE_NONE) {
        mpComp->mVoiceType = value;
    }
}

void Seq_c::procVoiceTypeReset() {
    mpComp->mVoiceType = MSG_VOICE_TYPE_NONE;
}

void Seq_c::procVoiceMood() {
    const u8 *params = dScript::getTagParams(mpChar);
    u8 kind = 0;
    int value = 0;
    std::memcpy(&kind, params, 1);
    switch (kind) {
    case 0:
        value = 0;
        break;
    case 1:
        value = 4;
        break;
    case 2:
        value = 1;
        break;
    case 3:
        value = 2;
        break;
    case 4:
        value = 3;
        break;
    }
    mpComp->mVoiceMood = value;
}

void Seq_c::procVoiceMoodReset() {
    mpComp->mVoiceMood = MSG_VOICE_MOOD_NONE;
}

void Ruby_c::clear() {
    mpComp = NULL;
    mpText = NULL;
    mPos = 0;
    mLen = 0;
    mBasePos = 0;
    mBaseLen = 0;
    mStep = 1;
}

void Ruby_c::set(Comp_c *comp, int baseLen, int len, const wchar_t *text) {
    mpComp = comp;
    mpText = text;
    mLen = len;
    mBaseLen = baseLen;
    mPos = 0;
    mBasePos = 0;
    mStep = 1;
    if (baseLen != 0) {
        mStep = len / baseLen;
    }
    if (mStep == 0) {
        mStep = 1;
    }
}

BOOL Ruby_c::stepBaseChar() {
    int n = mBasePos * mStep;
    if (mBasePos == 0) {
        mBasePos = nw4r::ut::Min(mBasePos + 1, mBaseLen);
        return TRUE;
    }
    if (mPos > n) {
        mBasePos = nw4r::ut::Min(mBasePos + 1, mBaseLen);
        return TRUE;
    }
    return FALSE;
}

BOOL Ruby_c::isEnd() {
    return mPos >= mLen;
}

void Ruby_c::procChar() {
    if (mpComp != NULL) {
        mpComp->setVoiceChar(mpText[mPos]);
    }
    mPos = nw4r::ut::Min(mPos + 1, mLen);
}

int Word_c::setTagWord(int pos, dScript::Word_c *word) {
    mPacchim.check(word->mPacchim.mLastChar);
    dScript::getTagLength(getBuffer() + pos);
    BOOL hasArticle = FALSE;
    int n = 0;
    dString::Word_c article;
    if (_1C == 0) {
        if (word->mInflect.mIndefArticle != 0) {
            hasArticle = dString::loadString(&article, word->mInflect.mIndefArticle, "sys_STRING/STR_Article");
        }
    } else if (_1C == 1) {
        if (word->mInflect.mDefArticle != 0) {
            hasArticle = dString::loadString(&article, word->mInflect.mDefArticle, "sys_STRING/STR_Article");
        }
    }
    removeTag(pos);
    if (hasArticle) {
        n = insertColorTag(pos, TEXT_COLOR_DEFAULT);
        n += insertWordOf(pos + n, &article, TRUE);
        n += insertColorTag(pos + n, mColor);
    }
    n += insertWordOf(pos + n, word, FALSE);
    _1C = 0;
    return n;
}

// The tag classes' inline ctor / dtor bodies. They are inline (the controller's ctor / dtor in
// d_msg_comp.inc inline them), but their bodies are in this file, so with -sym on their out-of-line copies
// (8019FCB8..8019FE6C) go to the end of this file's code, before the included file's. They come out in
// the order Comp_c's ctor references them; that is this order because Seq_c has an explicit ctor.
inline RndJumpData_c::RndJumpData_c() {}
inline RndJumpData_c::~RndJumpData_c() {}
inline RndJump_c::~RndJump_c() {}
inline Ruby_c::~Ruby_c() {}
inline Seq_c::~Seq_c() {}
inline Voice_c::~Voice_c() {}

} // namespace dMsg

// Everything from dMsg::Word_c's ctor (8019FE6C) to the end of the code.
#include "d_msg_comp.inc"
