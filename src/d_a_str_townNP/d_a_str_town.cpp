// The city's structures (REL d_a_str_townNP). See
// include/game/game/d_a_str_town.hpp.
// REL .text 0x5C..0x3C08, .ctors 0x0..0x4, .rodata 0x0..0x80, .data
// 0x0..0x2210, .bss 0x8..0x10.
#include <game/game/d_a_str_town.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop_gallery.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_weather.hpp>
#include <game/sLib/s_lib.hpp>

// The texture animation's speed change per frame (daStrTown_c::preExecute).
#define TOWN_ANM_RATE_STEP 0.05f

// ---------------------------------------------------------------------------
// The base (daStrTown_c, profile STR_TOWN)
// ---------------------------------------------------------------------------

// .text:0x5C
namespace dAStrTown {
BASE_PROFILE(STR_TOWN, daStrTown_c)
} // namespace dAStrTown

// .text:0xF0: the windows are lit from 18:00 to 23:00.
BOOL daStrTown_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= LIGHT_ON_HOUR && now.hour < LIGHT_OFF_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x198
void daStrTown_c::afterCreateMdl() {
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    mMdl.setPriorityDraw(2, 0x7F);
    if (mResFile.GetResAnmTexSrtNumEntries() != 0) {
        mAnmTexSrt.create(mdl, mResFile.GetResAnmTexSrt(0), allocator, NULL, 1);
        mAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmTexSrt);
    }
}

// .text:0x274: the animation eases to full speed while open, to a stop while
// closed.
int daStrTown_c::preExecute() {
    int ret = daSTR_c::preExecute();
    if (mAnmTexSrt.IsBound()) {
        f32 target = getCheckMsg() == DOOR_MSG_NONE ? 1.0f : 0.0f;
        if (target > mAnmRate) {
            mAnmRate += TOWN_ANM_RATE_STEP;
            if (mAnmRate >= target) {
                mAnmRate = target;
            }
        } else if (target < mAnmRate) {
            mAnmRate -= TOWN_ANM_RATE_STEP;
            if (mAnmRate <= target) {
                mAnmRate = target;
            }
        }
        mAnmTexSrt.setRate(mAnmRate, 0);
        mAnmTexSrt.play();
    }
    return ret;
}

// .text:0x368
int daStrTown_c::preDelete() {
    int ret = daSTR_c::preDelete();
    if (mAnmTexSrt.IsBound()) {
        mAnmTexSrt.remove();
    }
    return ret;
}

// ---------------------------------------------------------------------------
// Shampoodle (daStrBarber_c, profile STR_BARBER)
// ---------------------------------------------------------------------------

// .text:0x3C8
namespace dAStrTown {
BASE_PROFILE(STR_BARBER, daStrBarber_c)
} // namespace dAStrTown

// .text:0x55C: open from 8:00 to 21:00.
BOOL daStrBarber_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x604
BOOL daStrBarber_c::isEtcLightOn() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= CLOSE_HOUR && now.hour < ETC_LIGHT_OFF_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x6AC
int daStrBarber_c::getCheckMsg() {
    if (dSTR::isForceClosed()) {
        return DOOR_MSG_BARBER_CLOSED;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        return DOOR_MSG_NONE;
    }
    return DOOR_MSG_BARBER_CLOSED;
}

// ---------------------------------------------------------------------------
// The Happy Room Academy (daStrHappy_c, profile STR_HAPPY)
// ---------------------------------------------------------------------------

// .text:0x74C
namespace dAStrTown {
BASE_PROFILE(STR_HAPPY, daStrHappy_c)
} // namespace dAStrTown

// .text:0x7FC
BOOL daStrHappy_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x8A4
BOOL daStrHappy_c::isEtcLightOn() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= ETC_LIGHT_ON_HOUR && now.hour < CLOSE_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// ---------------------------------------------------------------------------
// Crazy Redd's (daStrBroker_c, profile STR_BROKER)
// ---------------------------------------------------------------------------

// .text:0x94C
namespace dAStrTown {
BASE_PROFILE(STR_BROKER, daStrBroker_c)
} // namespace dAStrTown

// .text:0x9FC: speaks with Redd's voice.
int daStrBroker_c::getVoiceMode() {
    dItem::Item redd = SP_NPC_BROKER;
    int voice = mVoiceType;
    bool same = dNpcInfo::getSpVoice(&redd) == voice;
    return !same;
}

// .text:0xA4C
void daStrBroker_c::afterCreateMdl() {
    daStrTown_c::afterCreateMdl();
    dItem::Item redd = SP_NPC_BROKER;
    mVoiceType = dNpcInfo::getSpVoice(&redd);
}

// .text:0xA90
int daStrBroker_c::doDelete() {
    return SUCCEEDED;
}

// .text:0xA98
BOOL daStrBroker_c::hasStock() {
    return dSaveData_c::getTown()->mShops.mShopGallery.getUnsoldNum() > 0;
}

// .text:0xAD0
BOOL daStrBroker_c::isOpen() {
    return dSaveData_c::getTown()->mShops.mShopGallery.isOpen();
}

// .text:0xAFC
BOOL daStrBroker_c::isMember() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
        PRIVATE_FLAG0_REDD_MEMBER);
}

// .text:0xB24
BOOL daStrBroker_c::hasInvite() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer()->hasInvite();
}

// .text:0xB48
int daStrBroker_c::getCheckMsg() {
    if (!hasStock()) {
        return MSG_SOLD_OUT;
    }
    if (!isOpen()) {
        return MSG_CLOSED;
    }
    if (!isMember()) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (!hasInvite()) {
            return -(dSTR::getCount() == 0) + MSG_NOT_MEMBER_AGAIN;
        }
        if ((player != NULL ? player->findAnsweredInvite() : NULL) != NULL) {
            return DOOR_MSG_NONE;
        }
        if (dSTR::getCount() == 0) {
            return MSG_NO_INVITE;
        }
        return dSTR::getFlagA() ? MSG_NO_INVITE_REFUSED : MSG_NO_INVITE_AGAIN;
    }
    return DOOR_MSG_NONE;
}

// .text:0xC3C: an answered invitation lets the player knock.
int daStrBroker_c::getLockedMsg() {
    if (!isMember() && hasInvite()) {
        dMail_c *invite =
            dPlayerMgr::Mgr_c::getCurrentPlayer()->findAnsweredInvite();
        if (invite != NULL) {
            dPersonalID_c *from = invite->getFromPlayer();
            invite->getFromAnimal();
            if (dSTR::getCount() == 0) {
                return from != NULL ? MSG_INVITED : MSG_ANIMAL_INVITED;
            }
            if (dSTR::getFlagA()) {
                return from != NULL ? MSG_INVITED_REFUSED
                                    : MSG_ANIMAL_INVITED_REFUSED;
            }
            if (dSTR::getFlagB()) {
                return from != NULL ? MSG_INVITED_REFUSED_AGAIN
                                    : MSG_ANIMAL_INVITED_REFUSED_AGAIN;
            }
            return MSG_INVITED_AGAIN;
        }
    }
    return DOOR_MSG_NONE;
}

// .text:0xD18
void daStrBroker_c::onMessageStart(int arg) {
    setInviteWords();
    switch (mMessageCode) {
    case MSG_NO_INVITE_REFUSED:
        dSTR::setFlagA(FALSE);
        break;
    case MSG_INVITED_REFUSED:
    case MSG_ANIMAL_INVITED_REFUSED:
        dSTR::setFlagA(FALSE);
        break;
    case MSG_INVITED_REFUSED_AGAIN:
    case MSG_ANIMAL_INVITED_REFUSED_AGAIN:
        dSTR::setFlagB(FALSE);
        break;
    }
}

// .text:0xD8C
void daStrBroker_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_NOT_MEMBER:
        dSTR::setFlagA(TRUE);
        break;
    case MSG_NO_INVITE:
        dSTR::setFlagB(TRUE);
        break;
    case MSG_NO_INVITE_REFUSED:
        dSTR::setFlagB(TRUE);
        break;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mMessageCode) {
    case MSG_NOT_MEMBER:
    case MSG_NOT_MEMBER_AGAIN:
    case MSG_NO_INVITE:
    case MSG_NO_INVITE_AGAIN:
    case MSG_NO_INVITE_REFUSED:
        if (player != NULL) {
            player->setFlag0(PRIVATE_FLAG0_REDD_TURNED_AWAY);
        }
        break;
    }
}

// .text:0xE48: open from 10:00 to 1:00.
BOOL daStrBroker_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = TRUE;
    if (now.hour < OPEN_HOUR && now.hour >= CLOSE_HOUR) {
        lit = FALSE;
    }
    return lit;
}

// .text:0xEF0: Redd is in, unless sold out or closed.
BOOL daStrBroker_c::isReddIn(int msg) {
    if (msg != DOOR_MSG_NONE) {
        return (u32)(msg - MSG_SOLD_OUT) > 1;
    }
    return TRUE;
}

// .text:0xF18
BOOL daStrBroker_c::isWindowLit() {
    if (isNight()) {
        int msg = getCheckMsg();
        if (msg == DOOR_MSG_NONE) {
            return TRUE;
        }
        return isReddIn(msg);
    }
    return FALSE;
}

// .text:0xF88: words 0 / 1: the inviter's town and name.
void daStrBroker_c::setInviteWords() {
    dMsg::Comp_c *comp = mpController;
    dMail_c *invite =
        dPlayerMgr::Mgr_c::getCurrentPlayer()->findAnsweredInvite();
    if (invite != NULL) {
        dPersonalID_c *player = invite->getFromPlayer();
        dAnmPersonalID_c *animal = invite->getFromAnimal();
        dMsg::Comp_c *ctrl = mpController;
        if (player != NULL) {
            ctrl->setLandName(0, &player->land);
            ctrl->setPersonalName(1, player);
        } else if (animal != NULL) {
            ctrl->setLandName(0, &animal->mLand);
            dHmnName::Word_c name;
            animal->setWord(&name, LANGUAGE_NUM);
            ctrl->setWord(1, &name);
        }
    } else {
        dItem::name_c name(dItem::Item(0));
        comp->setWord(0, &name);
        comp->setWord(1, &name);
    }
}

// .text:0x10B0
void daStrBroker_c::onTalkStart(int kind, dMsg::Comp_c *comp) {
    BOOL noCount = FALSE;
    switch (kind) {
    case TALK_KIND_MSG: {
        setMessageLabel("npc_SP/NPC_tunekichi");
        setMessageCode(mMsgCode);
        if (isReddIn(mMsgCode)) {
            setReddName();
        } else {
            noCount = TRUE;
            dHmnName::Word_c name;
            setSpeakerName(
                (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
                GENDER_MALE);
        }
        dSaveTown_c *town = dSaveData_c::getTown();
        dTime_c next = town->mShops.mShopGallery.getNextWednesday();
        comp->formatNumber(0, next.month + 1, 2, dScript::NUM_FORMAT_PLAIN);
        comp->formatNumber(1, next.mday, 2, dScript::NUM_FORMAT_PLAIN);
        break;
    }
    default:
        setMessageLabel("npc_SP/NPC_tunekichi");
        setMessageCode(mMsgCode);
        setReddName();
        setInviteWords();
        break;
    }
    if (!noCount) {
        dSTR::incCount();
    }
}

// .text:0x1244: "Redd" once the player has been inside, else the unknown
// name.
void daStrBroker_c::setReddName() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    BOOL visited = TRUE;
    if (player != NULL) {
        visited = player->isFlag0(PRIVATE_FLAG0_REDD_VISITED);
    }
    if (!visited) {
        dHmnName::Word_c name;
        dString::loadString(&name, dNpcInfo::SP_NAME_UNKNOWN_REDD, "sys_STRING/STR_SPNpc_name");
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
            GENDER_MALE);
    } else {
        dHmnName::Word_c name;
        dItem::Item redd = SP_NPC_BROKER;
        dNpc::getNpcName(&name, &redd, LANGUAGE_NUM);
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(),
            GENDER_MALE);
    }
}

// ---------------------------------------------------------------------------
// Katrina's (daStrFortune_c, profile STR_FORTUNE)
// ---------------------------------------------------------------------------

// .text:0x1334
namespace dAStrTown {
BASE_PROFILE(STR_FORTUNE, daStrFortune_c)
} // namespace dAStrTown

// .text:0x13E4
BOOL daStrFortune_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= LIGHT_ON_HOUR && now.hour < LIGHT_OFF_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x148C: open from 10:00.
int daStrFortune_c::getCheckMsg() {
    if (dSTR::isForceClosed()) {
        return DOOR_MSG_FORTUNE_CLOSED;
    }
    dTime_c now = *dTime_c::getCurrent();
    return now.hour < OPEN_HOUR ? DOOR_MSG_FORTUNE_CLOSED : DOOR_MSG_NONE;
}

// ---------------------------------------------------------------------------
// GracieGrace (daStrGrace_c, profile STR_GRACE)
// ---------------------------------------------------------------------------

// .text:0x1524
namespace dAStrTown {
BASE_PROFILE(STR_GRACE, daStrGrace_c)
} // namespace dAStrTown

// .text:0x15D4
BOOL daStrGrace_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x167C
BOOL daStrGrace_c::isEtcLightOn() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= ETC_LIGHT_ON_HOUR && now.hour < CLOSE_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x1724: open from 9:00 to 21:00.
int daStrGrace_c::getCheckMsg() {
    if (dSTR::isForceClosed()) {
        return DOOR_MSG_GRACE_CLOSED;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= OPEN_HOUR && now.hour < CLOSE_HOUR) {
        return DOOR_MSG_NONE;
    }
    return DOOR_MSG_GRACE_CLOSED;
}

// ---------------------------------------------------------------------------
// The auction house (daStrAuction_c, profile STR_AUCTION)
// ---------------------------------------------------------------------------

// .text:0x17C4
namespace dAStrTown {
BASE_PROFILE(STR_AUCTION, daStrAuction_c)
} // namespace dAStrTown

// ---------------------------------------------------------------------------
// The theater (daStrTheater_c, profile STR_THEATER)
// ---------------------------------------------------------------------------

// .text:0x1874
namespace dAStrTown {
BASE_PROFILE(STR_THEATER, daStrTheater_c)
} // namespace dAStrTown

// .text:0x1924
BOOL daStrTheater_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= LIGHT_ON_HOUR && now.hour < LIGHT_OFF_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x19CC
BOOL daStrTheater_c::isNeonOn() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = FALSE;
    if (now.hour >= NEON_ON_HOUR && now.hour < LIGHT_OFF_HOUR) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x1A74
BOOL daStrTheater_c::isEtcLightOn() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = TRUE;
    if (now.hour < LIGHT_OFF_HOUR && now.hour >= ETC_LIGHT_OFF_HOUR) {
        lit = FALSE;
    }
    return lit;
}

// ---------------------------------------------------------------------------
// The city's fountain (daStrTwFountain_c, profile STR_TW_FOUNTAIN)
// ---------------------------------------------------------------------------

// .text:0x1B1C
namespace dAStrTown {
BASE_PROFILE(STR_TW_FOUNTAIN, daStrTwFountain_c)
} // namespace dAStrTown

// .text:0x1C48
void daStrTwFountain_c::afterCreateMdl() {
    daStrTown_c::afterCreateMdl();
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    if (mResFile.GetResAnmChrNumEntries() != 0) {
        mAnmChr.create(mdl, mResFile.GetResAnmChr(0), allocator, NULL);
        mAnmChr.mPlayMode = m3d::FORWARD_LOOP;
        mMdl.setAnm(mAnmChr);
    }
    if (mResFile.GetResAnmClrNumEntries() != 0) {
        mAnmClr.create(mdl, mResFile.GetResAnmClr(0), allocator, NULL, 1);
        mAnmClr.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmClr);
    }
    if (mResFile.GetResAnmTexPatNumEntries() != 0) {
        mAnmTexPat.create(mdl, mResFile.GetResAnmTexPat(0), allocator, NULL, 1);
        mAnmTexPat.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnmTexPat);
    }
    if (mResFile.GetResAnmTexSrtNumEntries() != 0) {
        mFountainAnmTexSrt.create(mdl, mResFile.GetResAnmTexSrt(0), allocator,
                                  NULL, 1);
        mFountainAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mFountainAnmTexSrt);
    }
}

// .text:0x1E44
int daStrTwFountain_c::execute() {
    mVec3_c sndPos = mPos;
    sndPos.x += 16.0f;
    sndPos.z += 16.0f;
    mSnd.setPos(&sndPos);
    mSnd.holdSound(SE_LEV_funsui_machi);
    if (mAnmChr.IsBound()) {
        mAnmChr.play();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
    if (mAnmTexPat.IsBound()) {
        mAnmTexPat.play();
    }
    if (mFountainAnmTexSrt.IsBound()) {
        mFountainAnmTexSrt.play();
    }
    return SUCCEEDED;
}

// .text:0x1F40
int daStrTwFountain_c::doDelete() {
    if (mAnmChr.IsBound()) {
        mAnmChr.remove();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.remove();
    }
    if (mAnmTexPat.IsBound()) {
        mAnmTexPat.remove();
    }
    if (mFountainAnmTexSrt.IsBound()) {
        mFountainAnmTexSrt.remove();
    }
    return SUCCEEDED;
}

// ---------------------------------------------------------------------------
// The reset building (daStrReset_c, profile STR_RESET)
// ---------------------------------------------------------------------------

// .text:0x1FFC
namespace dAStrTown {
BASE_PROFILE(STR_RESET, daStrReset_c)
} // namespace dAStrTown

// .text:0x20AC
void daStrReset_c::afterCreateMdl() {
    daStrTown_c::afterCreateMdl();
    mIsOpen = FALSE;
    mPos.y = 14.5f;
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_RESET_OPEN)) {
        dTime_c now = *dTime_c::getCurrent();
        mIsOpen = now.hour >= OPEN_HOUR;
    }
}

// .text:0x2170: the door's nodes ("open" / "close") show by the light.
int daStrReset_c::execute() {
    if (mIsOpen) {
        dBGCF::unlockNext(BGCF_NEXT_ALL);
    } else {
        dBGCF::lockNext(0);
    }
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    nw4r::g3d::ResNode open = mdl.GetResNode("open");
    nw4r::g3d::ResNode close = mdl.GetResNode("close");
    if (isNight()) {
        open.SetVisibility(true);
        close.SetVisibility(false);
    } else {
        open.SetVisibility(false);
        close.SetVisibility(true);
    }
    return SUCCEEDED;
}

// .text:0x2274
BOOL daStrReset_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    return mIsOpen;
}

// ---------------------------------------------------------------------------
// The ATM (daStrAtm_c, profile STR_ATM)
// ---------------------------------------------------------------------------

// .text:0x22B4
namespace dAStrTown {
BASE_PROFILE(STR_ATM, daStrAtm_c)
} // namespace dAStrTown

// .text:0x2364
BOOL daStrAtm_c::isNight() {
    return !dSTR::isForceClosed();
}

// ---------------------------------------------------------------------------
// A street lamp (daStrLamp_c, profile STR_LAMP)
// ---------------------------------------------------------------------------

// .text:0x238C
namespace dAStrTown {
BASE_PROFILE(STR_LAMP, daStrLamp_c)
} // namespace dAStrTown

// .text:0x245C
void daStrLamp_c::afterCreateMdl() {
    daStrTown_c::afterCreateMdl();
    if (dAStr::isFadeMode()) {
        mMdl.setPriorityDraw(0x7F, 9);
    } else {
        mMdl.setPriorityDraw(0x7F, 0x81);
    }
}

// .text:0x24B8
int daStrLamp_c::execute() {
    return SUCCEEDED;
}

// .text:0x24C0
void daStrLamp_c::drawMdl() {
    daSTR_c::drawMdl();
}

// .text:0x24C4
int daStrLamp_c::doDelete() {
    return SUCCEEDED;
}

// .text:0x24CC: lit from 18:00 to 6:00, and all day in bad weather.
BOOL daStrLamp_c::isNight() {
    if (dSTR::isForceClosed()) {
        return FALSE;
    }
    if (dWeather_c::sInstance != NULL &&
        !dWeather_c::isFineKind(dWeather_c::sInstance->mWeather)) {
        return TRUE;
    }
    dTime_c now = *dTime_c::getCurrent();
    BOOL lit = TRUE;
    if (now.hour < LIGHT_ON_HOUR && now.hour >= LIGHT_OFF_HOUR) {
        lit = FALSE;
    }
    return lit;
}

// ---------------------------------------------------------------------------
// The organ grinder (daStrOrgan_c, profile STR_ORGAN)
// ---------------------------------------------------------------------------

// .text:0x25B0
namespace dAStrTown {
BASE_PROFILE(STR_ORGAN, daStrOrgan_c)
} // namespace dAStrTown

// .text:0x2660
void daStrOrgan_c::afterCreateMdl() {
    dBase_c::createBase(fProf::ORGAN_BALLOON, this, 0, 0);
}

// .text:0x2674
int daStrOrgan_c::execute() {
    dBgm::l_mgr.mStgTown.setMusicPos(&mPos);
    return SUCCEEDED;
}

// .text:0x26AC
int daStrOrgan_c::doDelete() {
    return SUCCEEDED;
}

// ---------------------------------------------------------------------------
// The city's bulletin board (daStrChest_c, profile STR_CHEST)
// ---------------------------------------------------------------------------

// .text:0x26B4
namespace dAStrTown {
BASE_PROFILE(STR_CHEST, daStrChest_c)
} // namespace dAStrTown

// .text:0x2764
void daStrChest_c::afterCreateMdl() {
    static const f32 cBoardHeight = 57.6f;
    setState(STATE_WAIT);
    mVec3_c bottomL(mPos.x - 48.0f, mPos.y, 16.0f + mPos.z);
    mVec3_c bottomR(16.0f + mPos.x, mPos.y, 16.0f + mPos.z);
    mVec3_c topL = bottomL;
    mVec3_c topR = bottomR;
    topL.y += cBoardHeight;
    topR.y += cBoardHeight;
    mVec3_c center = bottomL + bottomR;
    center *= 0.5f;
    mDpdQuad.set(topL, bottomL, bottomR, topR, dObjc::HIT_STR, this, &center,
                 0);
    mMdl.setPriorityDraw(2, 0x7F);
}

// .text:0x2884
int daStrChest_c::execute() {
    executeState();
    mDpdQuad.regist();
    return SUCCEEDED;
}

// .text:0x28BC
int daStrChest_c::doDelete() {
    return SUCCEEDED;
}

// .text:0x28C4
bool daStrChest_c::canInteract(dDemoActor_c *actor) {
    if (actor != NULL &&
        sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                            actor->getAngle().y) < DEG_TO_ANGLE(45)) {
        clearDemoState();
        return mInFront;
    }
    return false;
}

// .text:0x2928
void daStrChest_c::onDemoCmd(int cmd, u8) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setState(STATE_MENU);
        break;
    case DEMO_CMD_END:
        setState(STATE_WAIT);
        break;
    }
}

static BOOL (
    daStrChest_c::*const l_chestStateInit[daStrChest_c::STATE_NUM])() = {
    &daStrChest_c::initWait,
    &daStrChest_c::initMenu,
    &daStrChest_c::initMenuWait,
};

static void (
    daStrChest_c::*const l_chestStateExecute[daStrChest_c::STATE_NUM])() = {
    &daStrChest_c::executeWait,
    &daStrChest_c::executeMenu,
    &daStrChest_c::executeMenuWait,
};

// .text:0x2950
BOOL daStrChest_c::setState(int state) {
    if (state < STATE_NUM && (this->*l_chestStateInit[state])()) {
        mChestState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x29BC
void daStrChest_c::executeState() {
    if (mChestState < STATE_NUM) {
        (this->*l_chestStateExecute[mChestState])();
    }
}

// .text:0x29FC
BOOL daStrChest_c::initWait() {
    return TRUE;
}

// .text:0x2A04: the talk spot follows the player along the board.
void daStrChest_c::executeWait() {
    static const f32 cRightEnd = 6.4f;
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        f32 left = mPos.x - 38.4f;
        f32 right = cRightEnd + mPos.x;
        const f32 &x = player->getPos().x;
        mVec3_c pos = mPos;
        if (x < left) {
            pos.x = left;
            mInFront = FALSE;
        } else if (x > right) {
            pos.x = right;
            mInFront = FALSE;
        } else {
            pos.x = x;
            mInFront = TRUE;
        }
        mDemoPos = pos;
    }
}

// .text:0x2AC8
BOOL daStrChest_c::initMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_BBS) != 0;
}

// .text:0x2AF8
void daStrChest_c::executeMenu() {
    setState(STATE_MENU_WAIT);
}

// .text:0x2B00
BOOL daStrChest_c::initMenuWait() {
    return TRUE;
}

// .text:0x2B08
void daStrChest_c::executeMenuWait() {
    if (dMenuMng_c::isMenuEnd()) {
        dDemo::finishTalk(this);
    }
}

// Unreferenced: four halfwords, zeroed by __sinit (a debug parameter block
// with no vtable; inferred).
class daStrTown_hostIO_c {
  public:
    daStrTown_hostIO_c() {
        _0 = 0;
        _2 = 0;
        _4 = 0;
        _6 = 0;
    }

    /* 0x0 */ s16 _0;
    /* 0x2 */ s16 _2;
    /* 0x4 */ s16 _4;
    /* 0x6 */ s16 _6;
}; // size 0x8

static daStrTown_hostIO_c l_hostIO;
