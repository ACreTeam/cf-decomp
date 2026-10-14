#include <cstring>
#include <game/cLib/c_lib.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_c_uitag.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_clock.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_obj_chat.hpp>
#include <game/game/d_obj_telop.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_world.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <revolution/SC.h>

namespace dObjChat {
enum SoundId_e { SE_CHAT_MESSAGE = 0x16FE };
static Request_c l_requests[MEMBER_NUM];
}

// 801B0B20
CUSTOM_BASE_PROFILE(OBJ_CHAT, dObjChat_c, 0x125, 0x120);

namespace dObjChat {
static u32 l_textLength = 32;
}

// 801B0B94
dObjChat::Balloon_c::Balloon_c() {
}

// 801B0C00
int dObjChat_c::create() {
    if (mRes.load("Layout/chat_balloon.arc") == false) {
        return NOT_READY;
    }
    for (int i = 0; i < dObjChat::MEMBER_NUM; ++i) {
        mBalloons[i].init(&mRes, i);
    }
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        dObjChat::clear();
    }
    return SUCCEEDED;
}

// 801B0CA0
int dObjChat_c::doDelete() {
    return mRes.remove() != 0;
}

// 801B0CD0
int dObjChat_c::execute() {
    int k, j, i;
    static void (dObjChat_c::*l_stateFuncs[])() = {&dObjChat_c::stateMain};
    (this->*l_stateFuncs[mState])();
    for (i = 0; i < dObjChat::MEMBER_NUM; ++i) {
        dObjChat::l_requests[i].calc();
    }
    for (j = 0; j < dObjChat::MEMBER_NUM; ++j) {
        mBalloons[j].calc();
    }
    separateBalloons();
    for (k = 0; k < dObjChat::MEMBER_NUM; ++k) {
        mBalloons[k].move();
    }
    updateDepthOrder();
    return SUCCEEDED;
}

// 801B0DA4
void dObjChat_c::separateBalloons() {
    u32 changed;
    int count = 5;
    if (dObjChat::l_requests[0].mFlags & dObjChat::Request_c::FIXED) {
        return;
    }
    do {
        changed = 0;
        --count;
        for (int i = 0; i < dObjChat::MEMBER_NUM - 1; ++i) {
            for (int j = i + 1; j < dObjChat::MEMBER_NUM; ++j) {
                changed |= separatePair(i, j);
            }
        }
    } while (count > 0 && changed);
}

// 801B0E3C
BOOL dObjChat_c::separatePair(int first, int second) {
    dObjChat::Balloon_c *a = &mBalloons[first];
    if (!a->isActive()) {
        return FALSE;
    }
    dObjChat::Balloon_c *b = &mBalloons[second];
    if (!b->isActive()) {
        return FALSE;
    }
    f32 ay = a->getTargetY();
    f32 by = b->getTargetY();
    f32 extent = 20.0f * a->getScale() + 20.0f * b->getScale();
    f32 diff = ay - by;
    if (diff >= extent || diff <= -extent) {
        return FALSE;
    }
    f32 biased = diff;
    if (dObjChat::l_requests[first].mDepthOrder <
        dObjChat::l_requests[second].mDepthOrder) {
        biased -= 3.0f;
    }
    if (dObjChat::l_requests[first].mDepthOrder >
        dObjChat::l_requests[second].mDepthOrder) {
        biased += 3.0f;
    }
    f32 resultY;
    if ((biased <= 0.0f) != false) {
        f32 push = 0.5f * (extent + diff);
        by += push;
        resultY = ay - push;
    } else {
        f32 push = 0.5f * (extent - diff);
        by -= push;
        resultY = ay + push;
    }
    a->setTargetY(resultY);
    b->setTargetY(by);
    a->clampY();
    b->clampY();
    return TRUE;
}

// 801B1010
void dObjChat_c::updateDepthOrder() {
    for (int i = 0; i < dObjChat::MEMBER_NUM; ++i) {
        dObjChat::l_requests[i].mDepthOrder = 0;
        if (!mBalloons[i].isActive()) {
            dObjChat::l_requests[i].mDepthOrder = dObjChat::MEMBER_NUM;
        } else {
            for (int j = 0; j < dObjChat::MEMBER_NUM; ++j) {
                if (i != j && mBalloons[j].isActive() &&
                    mBalloons[i].getY() > mBalloons[j].getY()) {
                    ++dObjChat::l_requests[i].mDepthOrder;
                }
            }
        }
    }
}

// 801B10E8
int dObjChat_c::draw() {
    for (int order = dObjChat::MEMBER_NUM - 1; order >= 0; --order) {
        for (int i = 0; i < dObjChat::MEMBER_NUM; ++i) {
            if (order == dObjChat::l_requests[i].mOrder) {
                mBalloons[i].draw();
            }
        }
    }
    return SUCCEEDED;
}

// 801B1160
void dObjChat_c::stateMain() {
}

namespace dObjChat {

// 801B1164
void clear() {
    for (int i = 0; i < MEMBER_NUM; ++i) {
        l_requests[i].clear();
    }
}

// 801B11B4
void fixPositions() {
    for (int i = 0; i < MEMBER_NUM; ++i) {
        l_requests[i].mFlags |= Request_c::FIXED;
    }
}

// 801B11F0
void unfixPositions() {
    for (int i = 0; i < MEMBER_NUM; ++i) {
        l_requests[i].mFlags &= ~Request_c::FIXED;
    }
}

// 801B122C
void postText(const wchar_t *text, u8 kind) {
    setText(dNet::getMyNo(), text, l_textLength, kind);
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNet::beginPacket();
        dNet::addPacket(&kind, 1);
        dNet::addPacket(text, l_textLength * 2);
        dNet::sendPacket(NET_PACKET_CHAT, NET_MEMBER_NONE);
    }
}

// 801B12B0
void receiveText(const void *data, u32 size, int member) {
    int length = (size - 1) / 2;
    if (length < TEXT_BUFFER_SIZE) {
        wchar_t text[TEXT_BUFFER_SIZE];
        memset(text, 0, sizeof(text));
        u8 kind = *(const u8 *)data;
        cLib::memCpy(text, (const u8 *)data + 1, length * 2);
        setText(member, text, length, kind);
    }
}

// 801B133C
void postPreset(int group, int index) {
    int member = dNet::getMyNo();
    u8 groupByte = group;
    u8 indexByte = index;
    setPreset(member, group, index);
    if (dNet::isSessionActive() && dNet::getMemberNum() > 1) {
        dNet::beginPacket();
        dNet::addPacket(&groupByte, 1);
        dNet::addPacket(&indexByte, 1);
        dNet::sendPacket(NET_PACKET_CHAT_PRESET, NET_MEMBER_NONE);
    }
}

// 801B13C8
void receivePreset(const void *data, int member) {
    setPreset(member, ((const u8 *)data)[0], ((const u8 *)data)[1]);
}

// 801B13DC
void setText(int member, const wchar_t *text, u32 length, int kind) {
    updateOrder(member);
    l_requests[member].setText(text, length, kind);
}

// 801B144C
void setPreset(int member, int group, int index) {
    updateOrder(member);
    l_requests[member].setPreset(group, index);
}

// 801B14AC
void updateOrder(int member) {
    u16 order = l_requests[member].mOrder;
    for (int i = 0; i < MEMBER_NUM; ++i) {
        if (i != member && order > l_requests[i].mOrder) {
            ++l_requests[i].mOrder;
        }
    }
}

// 801B1544
void expireAll() {
    for (int i = 0; i < MEMBER_NUM; ++i) {
        l_requests[i].expire();
    }
}

// 801B1594
void playSound(f32 x) {
    f32 pan = x / 304.0f;
    if (pan < -1.0f) {
        pan = -1.0f;
    }
    if (pan > 1.0f) {
        pan = 1.0f;
    }
    playSePan(SE_CHAT_MESSAGE, pan);
}

// 801B15C4: release build disables regional preset substitutions.
BOOL useRegionalPresets() {
    return FALSE;
}

// 801B15CC
void loadPreset(dScript::Word_c *word, u16 index) {
    if (useRegionalPresets()) {
        switch (getLanguage()) {
        case LANGUAGE_MX:
            switch (index) {
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            }
            break;
        case LANGUAGE_QC:
            switch (index) {
            case TEXT_NICE_TO_MEET_YOU:
                index = TEXT_REGIONAL_NICE_TO_MEET_YOU;
                break;
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            }
            break;
        case LANGUAGE_ES:
            switch (index) {
            case TEXT_NICE_TO_MEET_YOU:
                index = TEXT_REGIONAL_NICE_TO_MEET_YOU;
                break;
            case TEXT_WELCOME:
                index = TEXT_REGIONAL_WELCOME;
                break;
            case TEXT_WELCOME_BACK:
                index = TEXT_REGIONAL_WELCOME_BACK;
                break;
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            case TEXT_WRAP_IT_UP:
                index = TEXT_REGIONAL_WRAP_IT_UP;
                break;
            case TEXT_GATHER_ROUND:
                index = TEXT_REGIONAL_GATHER_ROUND;
                break;
            case TEXT_YOU_CAN_HAVE_THIS:
                index = TEXT_REGIONAL_YOU_CAN_HAVE_THIS;
                break;
            case TEXT_WANNA_TRADE:
                index = TEXT_REGIONAL_WANNA_TRADE;
                break;
            case TEXT_SHOW_ME_YOUR_HOUSE:
                index = TEXT_REGIONAL_SHOW_ME_YOUR_HOUSE;
                break;
            case TEXT_SAY_THAT_AGAIN:
                index = TEXT_REGIONAL_SAY_THAT_AGAIN;
                break;
            case TEXT_SORRY:
                index = TEXT_REGIONAL_SORRY;
                break;
            }
            break;
        case LANGUAGE_FR:
            switch (index) {
            case TEXT_NICE_TO_MEET_YOU:
                index = TEXT_REGIONAL_NICE_TO_MEET_YOU;
                break;
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            case TEXT_YOU_CAN_HAVE_THIS:
                index = TEXT_REGIONAL_YOU_CAN_HAVE_THIS;
                break;
            case TEXT_GIMME:
                index = TEXT_REGIONAL_GIMME;
                break;
            case TEXT_SHOW_ME_YOUR_HOUSE:
                index = TEXT_REGIONAL_SHOW_ME_YOUR_HOUSE;
                break;
            case TEXT_KEEP_TRYING:
                index = TEXT_REGIONAL_KEEP_TRYING;
                break;
            }
            break;
        case LANGUAGE_IT:
            switch (index) {
            case TEXT_NICE_TO_MEET_YOU:
                index = TEXT_REGIONAL_NICE_TO_MEET_YOU;
                break;
            case TEXT_WHAT:
                index = TEXT_REGIONAL_WHAT;
                break;
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            case TEXT_WHOSE_IS_THIS:
                index = TEXT_REGIONAL_WHOSE_IS_THIS;
                break;
            }
            break;
        case LANGUAGE_DE:
            switch (index) {
            case TEXT_WHATCHA_DOING:
                index = TEXT_REGIONAL_WHATCHA_DOING;
                break;
            case TEXT_WHERE_ARE_YOU:
                index = TEXT_REGIONAL_WHERE_ARE_YOU;
                break;
            case TEXT_COME_ON:
                index = TEXT_REGIONAL_COME_ON;
                break;
            case TEXT_GATHER_ROUND:
                index = TEXT_REGIONAL_GATHER_ROUND;
                break;
            case TEXT_YOU_CAN_HAVE_THIS:
                index = TEXT_REGIONAL_YOU_CAN_HAVE_THIS;
                break;
            case TEXT_WANNA_TRADE:
                index = TEXT_REGIONAL_WANNA_TRADE;
                break;
            }
            break;
        }
    }
    dString::loadString(word, index, "sys_2D/SYS_2D_Chat");
}

// 801B1854
void loadPlayerPreset(dScript::Word_c *word, int index) {
    dPrivateData_c *player = NULL;
    int slot = 0;
    if (!dNet::isSessionActive()) {
        player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        slot = 0;
    } else {
        int i;
        int remaining = index;
        for (i = 0; i < MEMBER_NUM; ++i) {
            if (dNet::isMember(i)) {
                if (remaining > 0) {
                    --remaining;
                    ++slot;
                } else {
                    player = dPlayerMgr::Mgr_c::getNetPlayer(i);
                    break;
                }
            }
        }
    }
    dString::setPlayerWord(slot, &player->mPID);
    loadPreset(word, (u16)(index + TEXT_PLAYER_NAME_0));
}

// 801B1910
void loadHomePreset(dScript::Word_c *word, int index) {
    dPrivateData_c *player;
    for (int i = 0; i < MEMBER_NUM; ++i) {
        player = dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (player && player->mPID.isValid()) {
            if (index <= 0) {
                break;
            }
            player = NULL;
            --index;
        }
    }
    dString::setPlayerWord(PRESET_NAME_WORD_SLOT, &player->mPID);
    loadPreset(word, TEXT_TO_HOUSE);
}

// 801B19BC
void loadAnimalPreset(dScript::Word_c *word, int index) {
    dAnimal_c *animal;
    for (int i = 0; i < ANIMAL_NUM; ++i) {
        animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
        if (animal && animal->mID.isValid()) {
            if (index <= 0) {
                break;
            }
            animal = NULL;
            --index;
        }
    }
    dHmnName::Word_c name;
    animal->mID.setWord(&name, LANGUAGE_NUM);
    dString::setWord(PRESET_NAME_WORD_SLOT, &name);
    loadPreset(word, TEXT_TO_HOUSE);
}

// 801B1A8C
void loadNamePreset(dScript::Word_c *word, int index) {
    int i;
    int base;
    int count;
    int members = dNet::isSessionActive() ? (int)dNet::getMemberNum() : 1;
    if (index < members) {
        loadPlayerPreset(word, index);
        return;
    }
    base = members + DESTINATION_PRESET_NUM;
    if (index < base) {
        loadPreset(word, (u16)(index - members + TEXT_TO_TOWN_HALL));
        return;
    }
    count = 0;
    for (i = 0; i < MEMBER_NUM; ++i) {
        dPrivateData_c *player =
            dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (player && player->mPID.isValid()) {
            ++count;
        }
    }
    i = base + count;
    if (index < i) {
        loadHomePreset(word, index - base);
        return;
    }
    // The original retains the house count when adding the animals here.
    for (int j = 0; j < ANIMAL_NUM; ++j) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(j);
        if (animal && animal->mID.isValid()) {
            ++count;
        }
    }
    if (index < i + count) {
        loadAnimalPreset(word, index - i);
    }
}

// 801B1BCC
void loadMemberPreset(dScript::Word_c *word, int member) {
    dString::setPlayerWord(0, &dPlayerMgr::Mgr_c::getNetPlayer(member)->mPID);
    loadPreset(word, TEXT_REGISTERED_MEMBER);
}

// 801B1C18
int getNamePresetNum() {
    int i;
    int count = dNet::isSessionActive()
        ? dNet::getMemberNum() + DESTINATION_PRESET_NUM
        : 1 + DESTINATION_PRESET_NUM;
    for (i = 0; i < MEMBER_NUM; ++i) {
        dPrivateData_c *player =
            dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (player && player->mPID.isValid()) {
            ++count;
        }
    }
    for (i = 0; i < ANIMAL_NUM; ++i) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
        if (animal && animal->mID.isValid()) {
            ++count;
        }
    }
    return count;
}

// 801B1CE4
void getPreset(dScript::Word_c *word, int group, int index) {
    switch (group) {
    case PRESET_GROUP_GREETING:
        loadPreset(word, (u16)(index + TEXT_GOOD_MORNING));
        break;
    case PRESET_GROUP_QUESTION:
        loadPreset(word, (u16)(index + TEXT_WHAT));
        break;
    case PRESET_GROUP_INVITATION:
        loadPreset(word, (u16)(index + TEXT_HEY));
        break;
    case PRESET_GROUP_NAME_AND_DESTINATION:
        loadNamePreset(word, index);
        break;
    case PRESET_GROUP_REACTION:
        loadPreset(word, (u16)(index + TEXT_YES));
        break;
    case PRESET_GROUP_REGISTERED_MEMBER:
        loadMemberPreset(word, index);
        break;
    }
}

// 801B1D5C
int getPresetNum(int group) {
    switch (group) {
    case PRESET_GROUP_GREETING:
        return GREETING_PRESET_NUM;
    case PRESET_GROUP_QUESTION:
        return QUESTION_PRESET_NUM;
    case PRESET_GROUP_INVITATION:
        return INVITATION_PRESET_NUM;
    case PRESET_GROUP_NAME_AND_DESTINATION:
        return getNamePresetNum();
    case PRESET_GROUP_REACTION:
        return REACTION_PRESET_NUM;
    case PRESET_GROUP_REGISTERED_MEMBER:
        return MEMBER_NUM;
    }
    return 0;
}

// 801B1DC4
void Request_c::clear() {
    mTimer = 0;
    mFlags = 0;
    mOrder = MEMBER_NUM;
    memset(mText, 0, sizeof(mText));
    mDepthOrder = MEMBER_NUM;
    mGroup = PRESET_GROUP_NONE;
    mIndex = PRESET_INDEX_NONE;
}

// 801B1E28
void Request_c::calc() {
    if (mTimer > 0 && dObjTelop_c::isLocked()) {
        mTimer = 0;
    }
    if (mTimer > 0) {
        --mTimer;
    }
}

// 801B1E84
BOOL Request_c::isPending() {
    BOOL result = FALSE;
    if ((mFlags & PENDING) || mTimer > 0) {
        result = TRUE;
    }
    return result;
}

// 801B1EAC
void Request_c::expire() {
    mTimer = 0;
}

// 801B1EB8
bool Request_c::isExpired() {
    return mTimer <= 0;
}

// 801B1ECC
BOOL Request_c::isSystem() {
    if (mFlags & SYSTEM) {
        return true;
    }
    return (u16)(mFlags & PRESET) != 0;
}

// 801B1EF4
void Request_c::startTimer() {
    if (!(mFlags & PENDING)) {
        return;
    }
    mTimer = DISPLAY_FRAMES;
    mFlags &= ~PENDING;
}

// 801B1F14
void Request_c::setText(const wchar_t *text, u32 length, u32 kind) {
    memset(mText, 0, sizeof(mText));
    memcpy(mText, text, length * sizeof(*text));
    mTimer = 0;
    mFlags |= PENDING;
    mOrder = 0;
    if (kind == TEXT_KIND_SYSTEM) {
        mFlags |= SYSTEM;
    } else {
        mFlags &= ~SYSTEM;
    }
    mFlags &= ~PRESET;
}

// 801B1FC0
void Request_c::setPreset(int group, int index) {
    mGroup = group;
    mIndex = index;
    mTimer = 0;
    mOrder = 0;
    mFlags |= PENDING | PRESET;
}

// 801B1FE4
bool Request_c::hasSound() {
    return !(mFlags & SYSTEM);
}

// 801B1FF8
void Request_c::getWord(dScript::Word_c *word) {
    word->clear();
    if (mFlags & PRESET) {
        getPreset(word, mGroup, mIndex);
    } else {
        word->set(mText, FALSE);
    }
}

// 801B2064
void Balloon_c::init(m2d::ResAccLoader_c *res, int member) {
    mpRes = res;
    mLyt.build(res, "chat_balloon_00.brlyt");
    mMember = member;
    mLyt.mPriority = 2;
    mRequest = member;
    setColor(member);
    setTextStyle(TEXT_STYLE_FREE);
    mWord.setTextBox(mLyt.findTextBox("T_msg_00"));
    nw4r::lyt::Pane *pane = mLyt.findPane("bln_00");
    mBaseWidth = pane->GetScale().x * pane->GetSize().width;
    pane = mLyt.findPane("nameBln_00");
    mNameWidth = pane->GetScale().x * pane->GetSize().width;
    initAnm();
    mFlags |= DIRTY;
    mState = STATE_INIT;
    updateEnabled();
    if (isEnabled()) {
        setName(dPlayerMgr::Mgr_c::getNetPlayer(mMember)->mPID.player.mName);
    } else {
        mState = STATE_DISABLED;
    }
    mTargetAlpha = 1.0f;
    mAlpha = 1.0f;
    mTargetScale = 1.0f;
    mScale = 1.0f;
}

// 801B219C
void Balloon_c::calc() {
    if (!isEnabled()) {
        return;
    }
    updateText();
    u8 state = getState();
    if (state != mState) {
        mState = state;
    }
    switch (mState) {
    case STATE_PLAYER:
        calcPlayerPos();
        break;
    case STATE_FIXED:
        calcFixedPos();
        break;
    case STATE_REMOTE:
        calcRemotePos();
        break;
    default:
        mTargetPos.x = 0.0f;
        mTargetPos.y = 0.0f;
        break;
    }
    switch (mState) {
    case STATE_PLAYER:
        calcAlpha();
        mTargetScale = 1.0f;
        break;
    case STATE_FIXED:
        mTargetAlpha = 1.0f;
        mTargetScale = 1.0f;
        break;
    default:
        mTargetScale = 0.6f;
        mTargetAlpha = 0.0f;
        break;
    }
    clampPos();
    playAnm();
}

// 801B22AC
void Balloon_c::move() {
    if (mPos.x > 2.0f + mTargetPos.x) {
        sLib::addCalc(&mPos.x, 2.0f + mTargetPos.x, 0.5f, 10.0f, 0.1f);
    } else if (mPos.x < -2.0f + mTargetPos.x) {
        sLib::addCalc(&mPos.x, -2.0f + mTargetPos.x, 0.5f, 10.0f, 0.1f);
    }
    if (mPos.y > 2.0f + mTargetPos.y) {
        sLib::addCalc(&mPos.y, 2.0f + mTargetPos.y, 0.5f, 10.0f, 0.1f);
    }
    if (mPos.y < -2.0f + mTargetPos.y) {
        sLib::addCalc(&mPos.y, -2.0f + mTargetPos.y, 0.5f, 10.0f, 0.1f);
    }
    sLib::addCalc(&mAlpha, mTargetAlpha, 1.0f, 0.02f, 0.02f);
    sLib::addCalc(&mScale, mTargetScale, 1.0f, 0.02f, 0.02f);
    mLyt.mPos.x = mPos.x;
    mLyt.mPos.y = mPos.y;
    mLyt.getRootPane()->SetScale(mVec2_c(mScale, mScale));
    mFlags |= DIRTY;
}

// 801B23F0
void Balloon_c::draw() {
    if (isEnabled()) {
        drawLayout();
    }
}

// 801B242C
BOOL Balloon_c::isActive() {
    BOOL result = FALSE;
    if (isEnabled() == true && l_requests[mRequest].isExpired() == FALSE) {
        result = TRUE;
    }
    return result;
}

// 801B2494
f32 Balloon_c::getTargetY() {
    return mTargetPos.y;
}
// 801B249C
f32 Balloon_c::getY() {
    return mPos.y;
}
// 801B24A4
f32 Balloon_c::getScale() {
    return mTargetScale;
}
// 801B24AC
void Balloon_c::setTargetY(f32 y) {
    mTargetPos.y = y;
}

// 801B24B4
Balloon_c::State_e Balloon_c::getState() {
    if (!isEnabled()) {
        return STATE_DISABLED;
    }
    int member = mMember;
    BOOL otherScene = FALSE;
    if (member != dNet::getMyNo()) {
        u32 pos;
        u8 scene, flags;
        if (!dPlayerMgr::getRemoteStateInfo(&pos, &scene, &flags, mMember)) {
            l_requests[mRequest].clear();
            return STATE_MISSING;
        }
        otherScene = scene != dScene::getCurrentScene();
    }
    if (l_requests[mRequest].mFlags & Request_c::FIXED) {
        return STATE_FIXED;
    }
    if (otherScene) {
        return STATE_REMOTE;
    }
    // Keep the selection signed before converting to the state enum.
    return (State_e)(dPlayerMgr::getPlayerActor(member) == NULL
        ? (int)STATE_REMOTE : (int)STATE_PLAYER);
}

// 801B25BC
void Balloon_c::drawLayout() {
    if (mFlags & DIRTY) {
        mFlags &= ~DIRTY;
        mLyt.calc();
    }
    if (mFlags & SHOW) {
        mLyt.entry();
    }
}

// 801B2620
void Balloon_c::calcPlayerPos() {
    int member = mMember;
    dPlayerActor_c *player = dPlayerMgr::getNpcTargetActor(member);
    if (!player) {
        return;
    }
    mVec3_c *position = player->demoHook58();
    if (!position) {
        position = &player->mPos;
    }
    if (position) {
        mVec3_c pos = *position;
        mVec3_c curved;
        if (member != dNet::getMyNo() &&
            dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN)) {
            mVec3_c horizon;
            dWorld::getHorizonPosition(&horizon, &pos);
            if (horizon.z > pos.z) {
                pos = horizon;
            }
        }
        pos.y = player->mPos.y + 43.0f;
        dWorld::curvePosition(&curved, &pos);
        f32 x, y;
        dCamera::worldToScreen(lbl_8074E9B0, &x, &y, &curved);
        mTargetPos.x = -304.0f + 608.0f * x;
        mTargetPos.y = -228.0f + 456.0f * (1.0f - y);
    }
}

// 801B276C
void Balloon_c::calcFixedPos() {
    static const f32 l_fixedX[] = {-95.0f, -120.0f, -115.0f, -140.0f};
    int order = l_requests[mRequest].mOrder;
    mTargetPos.x = l_fixedX[order];
    mTargetPos.y = 95.0f + 33.0f * order;
}

// 801B27D4
void Balloon_c::calcRemotePos() {
    static const f32 l_remoteX[] = {300.0f, -300.0f, 300.0f};
    static const f32 l_remoteY[] = {300.0f, -300.0f, -300.0f};
    int index = mRequest;
    if (index > dNet::getMyNo()) {
        --index;
    }
    mTargetPos.x = l_remoteX[index];
    mTargetPos.y = l_remoteY[index];
}

// 801B283C
void Balloon_c::calcAlpha() {
    mTargetAlpha = 1.0f;
    int net_no = dNet::getMyNo();
    if (mMember != net_no) {
        dPlayerActor_c *player = dPlayerMgr::getPlayerActor(mMember);
        if (player) {
            dPlayerActor_c *local = dPlayerMgr::getPlayerActor(4);
            if (local) {
                mVec3_c diff = local->mPos - player->mPos;
                f32 distance =
                    EGG::Mathf::sqrt(diff.x * diff.x + diff.z * diff.z);
                if (!(distance <= 100.0f)) {
                    if ((distance >= 600.0f)) {
                        mTargetAlpha = 0.0f;
                    } else {
                        mTargetAlpha = (600.0f - distance) / 500.0f;
                    }
                }
            }
        }
    }
}

// 801B2910
void Balloon_c::clampPos() {
    mVec3_c namePos, bodyPos, nameTemp, bodyTemp;
    mVec2_c scale;
    mVec2_c offset;
    mLyt.mPos.x = mTargetPos.x;
    mLyt.mPos.y = mTargetPos.y;
    scale.set(mTargetScale, mTargetScale);
    mLyt.getRootPane()->SetScale(scale);
    mLyt.calc();
    offset.set(0.0f, 0.0f);
    m2d::getScreenLeftTop(&nameTemp, mLyt.findPane("nameBln_00"));
    namePos = nameTemp;
    if (!(l_requests[mRequest].mFlags & Request_c::FIXED)) {
        f32 top = 194.0f;
        if (isTopObstructed()) {
            top -= 45.0f;
        }
        if (namePos.y > top) {
            offset.y = top - namePos.y;
        }
    }
    m2d::getScreenRightBottom(&bodyTemp, mLyt.findPane("bln_00"));
    bodyPos = bodyTemp;
    if (bodyPos.x > 294.0f) {
        offset.x = 294.0f - bodyPos.x;
    }
    f32 left = -294.0f;
    if (namePos.y > 32.0f && dClock::isShown()) {
        if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
            left = -163.0f;
        } else {
            left = -143.0f;
        }
    }
    if (namePos.x < left) {
        offset.x = left - namePos.x;
    }
    f32 bottom = -194.0f;
    if (isBottomObstructed()) {
        bottom += 30.0f;
    }
    if (bodyPos.y < bottom) {
        offset.y = bottom - bodyPos.y;
    }
    if (offset.x != 0.0f || offset.y != 0.0f) {
        mTargetPos.x += offset.x;
        mTargetPos.y += offset.y;
    }
    mFlags |= DIRTY;
}

// 801B2B2C
void Balloon_c::clampY() {
    mVec3_c namePos, bodyPos, nameTemp, bodyTemp;
    mVec2_c scale;
    mVec2_c offset;
    mLyt.mPos.x = mTargetPos.x;
    mLyt.mPos.y = mTargetPos.y;
    scale.set(mTargetScale, mTargetScale);
    mLyt.getRootPane()->SetScale(scale);
    mLyt.calc();
    offset.set(0.0f, 0.0f);
    m2d::getScreenLeftTop(&nameTemp, mLyt.findPane("nameBln_00"));
    namePos = nameTemp;
    if (!(l_requests[mRequest].mFlags & Request_c::FIXED)) {
        f32 top = 194.0f;
        if (isTopObstructed()) {
            top -= 45.0f;
        }
        if (namePos.y > top) {
            offset.y = top - namePos.y;
        }
    }
    m2d::getScreenRightBottom(&bodyTemp, mLyt.findPane("bln_00"));
    bodyPos = bodyTemp;
    f32 bottom = -194.0f;
    if (isBottomObstructed()) {
        bottom += 30.0f;
    }
    if (bodyPos.y < bottom) {
        offset.y = bottom - bodyPos.y;
    }
    if (offset.y != 0.0f) {
        mTargetPos.y += offset.y;
    }
    mFlags |= DIRTY;
}

// 801B2CC4
bool Balloon_c::isTopObstructed() {
    if (dDemo::isType(dDemo::TYPE_MENU)) {
        return false;
    }
    if (dMoney_c::isShow()) {
        return true;
    }
    return dAcUitag::isBottomPanelShown();
}

// 801B2D10
bool Balloon_c::isBottomObstructed() {
    return dAcUitag::isBottomPanelShown();
}

// 801B2D14
BOOL Balloon_c::isEnabled() {
    return (u16)(mFlags & ENABLED) != 0;
}

// 801B2D2C
void Balloon_c::updateEnabled() {
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_DEMO)) {
        return;
    }
    int member = mMember;
    if (dNet::isSessionActive()) {
        if (dNet::isMember(member)) {
            mFlags |= ENABLED;
        } else {
            mFlags &= ~ENABLED;
        }
    } else {
        if (mRequest == 0) {
            mFlags |= ENABLED;
        } else {
            mFlags &= ~ENABLED;
        }
    }
}

// 801B2DDC
void Balloon_c::updateText() {
    if (!mAnm.isEnd()) {
        return;
    }
    switch (mAnm.mCurIdx) {
    case ANM_IN:
        if (l_requests[mRequest].isExpired()) {
            mAnm.setNext(ANM_OUT);
        }
        break;
    case ANM_OUT:
        mFlags &= ~SHOW;
        if (!dObjTelop_c::isLocked() && l_requests[mRequest].isPending()) {
            l_requests[mRequest].getWord(&mWord);
            mWord.update();
            setTextStyle(l_requests[mRequest].isSystem()
                ? TEXT_STYLE_PRESET : TEXT_STYLE_FREE);
            setWidth();
            l_requests[mRequest].startTimer();
            mAnm.setNext(ANM_IN);
            mFlags |= SHOW;
            if (l_requests[mRequest].hasSound() &&
                mRequest != dNet::getMyNo() && !dDemo::isType(dDemo::TYPE_SCENE_START) &&
                !dDemo::isType(dDemo::TYPE_ENTER)) {
                playSound(mPos.x);
            }
        }
        break;
    }
}

// 801B2F60
void Balloon_c::setWidth() {
    f32 width = mWord.calcWidth(NULL, 0);
    width += 25.0f;
    width *= 1.18f;
    f32 scaleX = width / mBaseWidth;
    mVec2_c scale(scaleX, 1.0f);
    nw4r::lyt::Pane *pane = mLyt.findPane("N_blnBdy");
    pane->SetScale(scale);
    mVec3_c pos = pane->GetTranslate();
    pos.x = 10.0f * scaleX;
    pane->SetTranslate(pos);
}

// 801B3014
void Balloon_c::setName(const wchar_t *name) {
    nw4r::lyt::TextBox *box = mLyt.findTextBox("T_name_00");
    m2d::stringWord_c word;
    word.setTextBox(box);
    word.clear();
    word.set(name, 0);
    word.update();
    f32 width = word.calcWidth(NULL, 0);
    width += 25.0f;
    width *= 1.18f;
    f32 scaleX = width / mNameWidth;
    mVec2_c scale(scaleX, 1.0f);
    nw4r::lyt::Pane *pane = mLyt.findPane("N_nameBln");
    pane->SetScale(scale);
    mVec3_c pos = pane->GetTranslate();
    pos.x = 10.0f * (1.0f - scaleX);
    pane->SetTranslate(pos);
}

// 801B3138
void Balloon_c::setColor(int color) {
    static const char *l_colorAnmNames[] = {
        "chat_balloon_00_blnBlue.brlan", "chat_balloon_00_blnYlow.brlan",
        "chat_balloon_00_blnOrange.brlan", "chat_balloon_00_blnPurple.brlan"};
    mLyt.setAnm(&mColorAnm, l_colorAnmNames[color], NULL);
    mColorAnm.setToEnd();
}

// 801B3188
void Balloon_c::setTextStyle(int style) {
    static const char *l_textAnmNames[] = {"chat_balloon_00_T_msg_free.brlan",
                                    "chat_balloon_00_T_msg_teikei.brlan"};
    mLyt.setAnm(&mTextAnm, l_textAnmNames[style], NULL);
    mTextAnm.setToEnd();
}

// 801B31D4
void Balloon_c::initAnm() {
    static const char *l_balloonAnmNames[] = {"chat_balloon_00_blnIn.brlan",
                                    "chat_balloon_00_blnOut.brlan"};
    mAnm.init(&mLyt, mpRes, l_balloonAnmNames, ANM_NUM);
    mAnm.start(ANM_OUT);
    mAnm.setToEnd();
}

// 801B3228
void Balloon_c::playAnm() {
    if (mAnm.play()) {
        mFlags |= DIRTY;
    }
}

} // namespace dObjChat

// 801B326C
dObjChat_c::~dObjChat_c() {
}

// 801B32F0
dObjChat::Balloon_c::~Balloon_c() {
}

typedef char RequestSizeCheck[sizeof(dObjChat::Request_c) == 0xD8 ? 1 : -1];
typedef char BalloonSizeCheck[sizeof(dObjChat::Balloon_c) == 0x24C ? 1 : -1];
typedef char ChatSizeCheck[sizeof(dObjChat_c) == 0xA6C ? 1 : -1];
typedef char
    RequestFlagsCheck[offsetof(dObjChat::Request_c, mFlags) == 0xD2 ? 1 : -1];
typedef char BalloonPosCheck[offsetof(dObjChat::Balloon_c, mTargetPos) == 0x214
                                 ? 1
                                 : -1];
typedef char
    BalloonFlagsCheck[offsetof(dObjChat::Balloon_c, mFlags) == 0x24A ? 1 : -1];
typedef char
    ChatBalloonsCheck[offsetof(dObjChat_c, mBalloons) == 0x138 ? 1 : -1];
