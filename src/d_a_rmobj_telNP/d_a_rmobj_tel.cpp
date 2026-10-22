#include <game/game/d_a_rmobj_tel.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_wifi.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/math/eggMath.h>
#include <revolution/sc.h>
#include <cstring>

// The house telephone. Functions are in target .text order.

namespace dARmobjTel {
// The label of the phone messages.
const char *l_msgLabel = "npc_SP/NPC_tel";

// .text 0x5C
BASE_PROFILE(A_RMOBJ_TEL, daRmobjTel_c);
} // namespace dARmobjTel

daRmobjTel_c *daRmobjTel_c::sInstance;

// .text 0x1B8
void daRmobjTel_c::pickUp() {
    if (sInstance != NULL) {
        sInstance->mModel.setAnmChr(ANM_PICK_UP, m3d::FORWARD_ONCE, 0.0f);
    }
}

// .text 0x1E4
void daRmobjTel_c::talk() {
    if (sInstance != NULL) {
        sInstance->mModel.setAnmChr(ANM_TALK, m3d::FORWARD_LOOP, 0.0f);
    }
}

// .text 0x210
void daRmobjTel_c::hangUp() {
    if (sInstance != NULL) {
        sInstance->mModel.setAnmChr(ANM_PICK_UP, m3d::REVERSE_ONCE, 0.0f);
    }
}

// .text 0x23C
int daRmobjTel_c::create() {
    if (mModel.create("obj_myroomR_tel", 0)) {
        static const dState::mode_c<daRmobjTel_c>::Entry l_modes[] = {
            {"待機", &daRmobjTel_c::initWait, &daRmobjTel_c::executeWait},
            {"電話アニメ開始要求", &daRmobjTel_c::initPhoneAnmStartReq,
             &daRmobjTel_c::executePhoneAnmStartReq},
            {"電話アニメ開始", &daRmobjTel_c::initPhoneAnmStart,
             &daRmobjTel_c::executePhoneAnmStart},
            {"メッセージ", &daRmobjTel_c::initMessage,
             &daRmobjTel_c::executeMessage},
            {"メッセージサスペンド", &daRmobjTel_c::initMessageSuspend,
             &daRmobjTel_c::executeMessageSuspend},
            {"メッセージメニュー開く", &daRmobjTel_c::initMessageMenuOpen,
             &daRmobjTel_c::executeMessageMenuOpen},
            {"メッセージメニュー閉じる", &daRmobjTel_c::initMessageMenuClose,
             &daRmobjTel_c::executeMessageMenuClose},
            {"メッセージ→着替え", &daRmobjTel_c::initMessageToChange,
             &daRmobjTel_c::executeMessageToChange},
            {"プレイヤ着替え：電話アニメ終了要求",
             &daRmobjTel_c::initChangePhoneAnmEndReq,
             &daRmobjTel_c::executeChangePhoneAnmEndReq},
            {"プレイヤ着替え：電話アニメ終了",
             &daRmobjTel_c::initChangePhoneAnmEnd,
             &daRmobjTel_c::executeChangePhoneAnmEnd},
            {"プレイヤ着替え開始", &daRmobjTel_c::initChangeStart,
             &daRmobjTel_c::executeChangeStart},
            {"プレイヤ着替え終了", &daRmobjTel_c::initChangeEnd,
             &daRmobjTel_c::executeChangeEnd},
            {"電話アニメ終了要求", &daRmobjTel_c::initPhoneAnmEndReq,
             &daRmobjTel_c::executePhoneAnmEndReq},
            {"電話アニメ終了", &daRmobjTel_c::initPhoneAnmEnd,
             &daRmobjTel_c::executePhoneAnmEnd},
            {"メッセージサスペンド", &daRmobjTel_c::initSpeakerSuspend,
             &daRmobjTel_c::executeSpeakerSuspend},
            {"メッセージ開く", &daRmobjTel_c::initMessageOpen,
             &daRmobjTel_c::executeMessageOpen},
        };
        dState::mode_c<daRmobjTel_c>::initialize("daRmobjTel_c", l_modes,
                                                 ARRAY_SIZE(l_modes));
        setDemoID(0);
        mModel.setAnmChr(ANM_IDLE, m3d::FORWARD_LOOP, 0.0f);
        sInstance = this;
        mVoiceMode = VOICE_MODE_NONE;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x338
int daRmobjTel_c::doDelete() {
    if (mModel.remove()) {
        sInstance = NULL;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x37C
int daRmobjTel_c::execute() {
    dState::base_c<daRmobjTel_c>::execute();
    mModel.play();
    return SUCCEEDED;
}

// .text 0x3D8
int daRmobjTel_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x400
void daRmobjTel_c::startPickUpSe() {
    mSeTimer = 20;
}

// .text 0x40C
BOOL daRmobjTel_c::isPickUpEnd() {
    if (mSeTimer != -1) {
        if (sLib::calcTimer(&mSeTimer) == 0) {
            playSe(SE_telephone_up);
            mSeTimer = -1;
        }
        return FALSE;
    }
    return mModel.isStopChr();
}

// .text 0x470
void daRmobjTel_c::startHangUpSe() {
    mSeTimer = 25;
}

// .text 0x47C
BOOL daRmobjTel_c::isHangUpEnd() {
    if (mSeTimer != -1) {
        if (sLib::calcTimer(&mSeTimer) == 0) {
            playSe(SE_telephone_down);
            mSeTimer = -1;
        }
        return FALSE;
    }
    return mModel.isStopChr();
}

// .text 0x4E0
void daRmobjTel_c::initWait() {
}

// .text 0x4E4
void daRmobjTel_c::executeWait() {
}

// .text 0x4E8
void daRmobjTel_c::initPhoneAnmStartReq() {
}

// .text 0x4EC
void daRmobjTel_c::executePhoneAnmStartReq() {
    if (dPlayerMgr::pickUpPhone()) {
        changeMode(&daRmobjTel_c::executePhoneAnmStart);
    }
}

// .text 0x654
void daRmobjTel_c::initPhoneAnmStart() {
    startPickUpSe();
}

// .text 0x658
void daRmobjTel_c::executePhoneAnmStart() {
    if (isPickUpEnd()) {
        if (dScene::isCurrentSceneMyHouse()) {
            if ((int)dNet::getMemberNum() > 1) {
                startMessage(MSG_CANT_USE);
            } else if (hasHouseFlag()) {
                startMessage(MSG_FLAG_HELLO);
            } else {
                startMessage(MSG_HELLO);
            }
        } else {
            startMessage(MSG_NOT_YOUR_PHONE);
        }
        changeMode(&daRmobjTel_c::executeMessage);
    }
}

// .text 0x824
void daRmobjTel_c::initMessage() {
}

// .text 0x828
void daRmobjTel_c::executeMessage() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (isMsgHidden(comp)) {
            changeMode(&daRmobjTel_c::executePhoneAnmEndReq);
        }
    }
}

// .text 0x9C4
void daRmobjTel_c::initMessageToChange() {
}

// .text 0x9C8
void daRmobjTel_c::executeMessageToChange() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (isMsgHidden(comp)) {
            changeMode(&daRmobjTel_c::executeChangePhoneAnmEndReq);
        }
    }
}

// .text 0xB64
void daRmobjTel_c::initMessageSuspend() {
    getController()->mSuspendReq = dMsg::SUSPEND_REQ_MENU;
}

// .text 0xB74
void daRmobjTel_c::executeMessageSuspend() {
    if (isMsgSuspend(getController())) {
        changeMode(&daRmobjTel_c::executeMessageMenuOpen);
    }
}

// .text 0xD08
void daRmobjTel_c::initMessageMenuOpen() {
}

// .text 0xD0C
void daRmobjTel_c::executeMessageMenuOpen() {
    if (dMenuMng_c::openMenu(dMenuMng_c::MENU_FLAG_DESIGN)) {
        changeMode(&daRmobjTel_c::executeMessageMenuClose);
    }
}

// .text 0xE78
void daRmobjTel_c::initMessageMenuClose() {
}

// .text 0xE7C
void daRmobjTel_c::executeMessageMenuClose() {
    if (dMenuMng_c::isMenuEnd()) {
        if (lbl_8074EA7D == 1) {
            requestMessage(MSG_FLAG_DESIGN_DONE, dARmobjTel::l_msgLabel);
            if ((int)dMenuMng_c::getPickedDesign() != -1) {
                dPlayerMgr::refreshEquippedUmbrellaDesign();
                if (dPlayerMgr::isDesignEquipped((u8)dMenuMng_c::getPickedDesign())) {
                    mEquip.setFromPlayer();
                    mChangeClothes = TRUE;
                }
            }
        } else {
            requestMessage(MSG_FLAG_DESIGN_KEEP, dARmobjTel::l_msgLabel);
        }
        if (mChangeClothes) {
            changeMode(&daRmobjTel_c::executeMessageToChange);
        } else {
            changeMode(&daRmobjTel_c::executeMessage);
        }
        getController()->requestStart(0);
    }
}

// .text 0x1194
void daRmobjTel_c::initChangePhoneAnmEndReq() {
}

// .text 0x1198
void daRmobjTel_c::executeChangePhoneAnmEndReq() {
    if (dPlayerMgr::hangUpPhone()) {
        changeMode(&daRmobjTel_c::executeChangePhoneAnmEnd);
    }
}

// .text 0x1300
void daRmobjTel_c::initChangePhoneAnmEnd() {
    startHangUpSe();
}

// .text 0x1304
void daRmobjTel_c::executeChangePhoneAnmEnd() {
    if (isHangUpEnd()) {
        changeMode(&daRmobjTel_c::executeChangeStart);
    }
}

// .text 0x146C
void daRmobjTel_c::initChangeStart() {
}

// .text 0x1470
void daRmobjTel_c::executeChangeStart() {
    if (dPlayerMgr::changeEquipment(&mEquip, 5, 0)) {
        changeMode(&daRmobjTel_c::executeChangeEnd);
    }
}

// .text 0x15E4
void daRmobjTel_c::initChangeEnd() {
}

// .text 0x15E8
void daRmobjTel_c::executeChangeEnd() {
    if (!dPlayerMgr::isChangingEquipment()) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
    }
}

// .text 0x163C
void daRmobjTel_c::initPhoneAnmEndReq() {
}

// .text 0x1640
void daRmobjTel_c::executePhoneAnmEndReq() {
    if (dPlayerMgr::hangUpPhone()) {
        changeMode(&daRmobjTel_c::executePhoneAnmEnd);
    }
}

// .text 0x17A8
void daRmobjTel_c::initPhoneAnmEnd() {
    startHangUpSe();
}

// .text 0x17AC
void daRmobjTel_c::executePhoneAnmEnd() {
    if (isHangUpEnd()) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
    }
}

// .text 0x1800
void daRmobjTel_c::initSpeakerSuspend() {
    getController()->mSuspendReq = dMsg::SUSPEND_REQ_MENU;
}

// .text 0x1810
void daRmobjTel_c::executeSpeakerSuspend() {
    if (isMsgSuspend(getController())) {
        callRequestEndMethod();
    }
}

// .text 0x18E0
void daRmobjTel_c::initMessageOpen() {
    getController()->requestStart(0);
}

// .text 0x18EC
void daRmobjTel_c::executeMessageOpen() {
    changeMode(&daRmobjTel_c::executeMessage);
}

// .text 0x1A48
void daRmobjTel_c::startMessage(u16 code) {
    attachDemoActor(this);
    setMessageLabel(dARmobjTel::l_msgLabel);
    setMessageCode(code);
    mChangeClothes = FALSE;
    getController()->requestStart(0);
}

// .text 0x1AC4
bool daRmobjTel_c::canInteract(dDemoActor_c *actor) {
    if (actor != NULL) {
        mVec3_c diff = mPos - actor->getPos();
        if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) < 35.2f &&
            sLib::distanceAngle(-0x8000, actor->getAngle().y.mAngle) <=
                0x1100) {
            return true;
        }
    }
    return false;
}

// .text 0x1B50
void daRmobjTel_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        changeMode(&daRmobjTel_c::executePhoneAnmStartReq);
        break;
    case DEMO_CMD_END:
        changeMode(&daRmobjTel_c::executeWait);
        break;
    }
}

// .text 0x1DEC
int daRmobjTel_c::getForcedVoiceMode() {
    int mode = VOICE_MODE_NONE;
    switch (mVoiceMode) {
    case VOICE_MODE_ANIMALESE:
        mode = VOICE_MODE_ANIMALESE;
        break;
    case VOICE_MODE_BEBEBESE:
        mode = VOICE_MODE_BEBEBESE;
        break;
    case VOICE_MODE_SILENT:
        mode = VOICE_MODE_SILENT;
        break;
    }
    return mode;
}

// .text 0x1E30
void daRmobjTel_c::selectVoiceMode0() {
    getController()->setStringWord(1, 5, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_ANIMALESE;
}

// .text 0x1E78
void daRmobjTel_c::selectVoiceMode1() {
    getController()->setStringWord(1, 6, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_BEBEBESE;
}

// .text 0x1EC0
void daRmobjTel_c::selectVoiceMode2() {
    getController()->setStringWord(1, 7, "sys_STRING/STR_Unit");
    mVoiceMode = VOICE_MODE_SILENT;
}

// .text 0x1F08
void daRmobjTel_c::cancelVoiceMode() {
    mVoiceMode = VOICE_MODE_NONE;
}

// .text 0x1F14
void daRmobjTel_c::decideVoiceMode() {
    dSaveOption::setVoiceMode(mVoiceMode);
    mVoiceMode = VOICE_MODE_NONE;
}

// .text 0x1F4C
void daRmobjTel_c::setKanaOff() {
    dSaveOption::setKana(FALSE);
}

// .text 0x1F54
void daRmobjTel_c::setKanaOn() {
    dSaveOption::setKana(TRUE);
}

// .text 0x1F5C
void daRmobjTel_c::setKeySeOff() {
    dSaveOption::setKeySe(FALSE);
}

// .text 0x1F64
void daRmobjTel_c::setKeySeOn() {
    dSaveOption::setKeySe(TRUE);
}

// .text 0x1F6C
void daRmobjTel_c::setTutorialOn() {
    dSaveOption::setTutorialOff(FALSE);
}

// .text 0x1F74
void daRmobjTel_c::setTutorialOff() {
    dSaveOption::setTutorialOff(TRUE);
}

// .text 0x1F7C
void daRmobjTel_c::enableNet() {
    if (SCCheckPCMessageRestriction()) {
        switchToSystemSpeaker();
        requestMessage(NET_ERROR_MSG_RESTRICTED, sNetErrorMessageLabel);
    } else if (dWifi::getWc24State() != dWifi::WC24_STATE_ON) {
        int message = dWifi::getErrorMessage();
        int error = dWifi::getErrorCode();
        u16 code = NET_ERROR_MSG_WC24_OTHER;
        if (message <= 9) {
            code = message + NET_ERROR_MSG_WC24_NAND;
        }
        switchToSystemSpeaker();
        getController()->formatNumber(0, error, 6, 6);
        requestMessage(code, sNetErrorMessageLabel);
    } else {
        dWifi::enableNet();
    }
}

// .text 0x2054
void daRmobjTel_c::disableNet() {
    dWifi::disableNet();
}

// .text 0x2058
void daRmobjTel_c::selectNetSetting() {
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        if (!dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled) {
            requestMessage(MSG_WC24_OFF, dARmobjTel::l_msgLabel);
        } else {
            requestMessage(MSG_WC24_ON, dARmobjTel::l_msgLabel);
        }
    }
}

// .text 0x20D4
void daRmobjTel_c::onAnswer(int answer) {
    struct Entry {
        const char *label;
        int code;
        int answer;
        Method method;
    };
    static Entry messages[] = {
        {dARmobjTel::l_msgLabel, MSG_ANYTHING_ELSE, 3,
         &daRmobjTel_c::selectNetSetting},
        {dARmobjTel::l_msgLabel, MSG_HELLO, 3, &daRmobjTel_c::selectNetSetting},
        {dARmobjTel::l_msgLabel, MSG_GAME_SETTINGS, 3,
         &daRmobjTel_c::selectNetSetting},
        {dARmobjTel::l_msgLabel, MSG_VOICE, 0, &daRmobjTel_c::selectVoiceMode0},
        {dARmobjTel::l_msgLabel, MSG_VOICE, 1, &daRmobjTel_c::selectVoiceMode1},
        {dARmobjTel::l_msgLabel, MSG_VOICE, 2, &daRmobjTel_c::selectVoiceMode2},
        {dARmobjTel::l_msgLabel, MSG_VOICE_CONFIRM, 0,
         &daRmobjTel_c::decideVoiceMode},
        {dARmobjTel::l_msgLabel, MSG_VOICE_CONFIRM, 1,
         &daRmobjTel_c::cancelVoiceMode},
        {dARmobjTel::l_msgLabel, MSG_KANA, 1, &daRmobjTel_c::setKanaOff},
        {dARmobjTel::l_msgLabel, MSG_KANA, 0, &daRmobjTel_c::setKanaOn},
        {dARmobjTel::l_msgLabel, MSG_KEY_INPUT, 0, &daRmobjTel_c::setKeySeOff},
        {dARmobjTel::l_msgLabel, MSG_KEY_INPUT, 1, &daRmobjTel_c::setKeySeOn},
        {dARmobjTel::l_msgLabel, MSG_TUTORIAL, 0, &daRmobjTel_c::setTutorialOn},
        {dARmobjTel::l_msgLabel, MSG_TUTORIAL, 1,
         &daRmobjTel_c::setTutorialOff},
        {dARmobjTel::l_msgLabel, MSG_WC24_ON, 0, &daRmobjTel_c::enableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_ON, 1, &daRmobjTel_c::disableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_OFF, 0, &daRmobjTel_c::enableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_OFF, 1, &daRmobjTel_c::disableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_ON_AGAIN, 0,
         &daRmobjTel_c::enableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_ON_AGAIN, 1,
         &daRmobjTel_c::disableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_OFF_AGAIN, 0,
         &daRmobjTel_c::enableNet},
        {dARmobjTel::l_msgLabel, MSG_WC24_OFF_AGAIN, 1,
         &daRmobjTel_c::disableNet},
    };
    u16 code = getMessageCode();
    const char *label;
    int selected = getController()->mSelect.mCursor;
    Entry *entry = messages;
    label = getMessageLabel();
    for (int i = 0; i < 22; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code &&
            selected == entry->answer) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x21F4
void daRmobjTel_c::onMessageStart(int arg) {
    if (std::strcmp(getMessageLabel(), sNetErrorMessageLabel) == 0) {
        mRoverSpeaks = FALSE;
    } else {
        switch (getMessageCode()) {
        case MSG_NOT_YOUR_PHONE:
        case MSG_CANT_USE:
            mRoverSpeaks = FALSE;
            break;
        default:
            mRoverSpeaks = TRUE;
            break;
        }
    }
    if (mRoverSpeaks) {
        dHmnName::Word_c name;
        dItem::Item item((u16)SP_NPC_GUIDE);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
        setSpeakerName(
            (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
        dItem::Item voice((u16)SP_NPC_GUIDE);
        mVoiceType = dNpcInfo::getSpVoice(&voice);
    }
}

inline void daRmobjTel_c::dispatchMessageEnd(MessageEndEntry *messages,
                                             int count) {
    int i;
    MessageEndEntry *entry;
    const char *label;
    u16 code;
    code = getMessageCode();
    entry = messages;
    label = getMessageLabel();
    for (i = 0; i < count; i++, entry++) {
        if (std::strcmp(label, entry->label) == 0 && code == entry->code) {
            callMessageMethod(entry->method);
        }
    }
}

// .text 0x22EC
void daRmobjTel_c::onMessageEnd(int kind) {
    dMsg::Comp_c *comp = getController();
    static MessageEndEntry messages[] = {
        {dARmobjTel::l_msgLabel, MSG_WC24_EXPLAIN,
         &daRmobjTel_c::askNetSetting},
        {dARmobjTel::l_msgLabel, MSG_ANYTHING_ELSE,
         &daRmobjTel_c::openSettingsMenu},
        {dARmobjTel::l_msgLabel, MSG_HELLO, &daRmobjTel_c::openSettingsMenu},
        {dARmobjTel::l_msgLabel, MSG_GAME_SETTINGS,
         &daRmobjTel_c::openSettingsMenu},
        {sNetErrorMessageLabel, NET_ERROR_MSG_RESTRICTED,
         &daRmobjTel_c::returnFromNetError3},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_OFF,
         &daRmobjTel_c::returnFromNetError3},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NAND,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_UPDATE,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NO_INTERNET,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY2,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_NAND2,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_OTHER,
         &daRmobjTel_c::returnFromNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_RESET,
         &daRmobjTel_c::holdNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_RESET2,
         &daRmobjTel_c::holdNetError},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY,
         &daRmobjTel_c::returnFromNetError2},
        {sNetErrorMessageLabel, NET_ERROR_MSG_WC24_BUSY3,
         &daRmobjTel_c::returnFromNetError2},
    };
    dispatchMessageEnd(messages, 16);
    switch (kind) {
    case END_REQ_DESIGN_MENU:
        changeMode(&daRmobjTel_c::executeMessageSuspend);
        break;
    }
    if (comp->mNextMsgCode == MSG_ANYTHING_ELSE && hasHouseFlag()) {
        comp->setNextMsgCode(MSG_FLAG_ANYTHING_ELSE);
    }
}

// .text 0x2544
void daRmobjTel_c::returnFromNetError() {
    switchToRoverSpeaker();
    requestMessage(MSG_ANYTHING_ELSE, dARmobjTel::l_msgLabel);
}

// .text 0x2584
void daRmobjTel_c::returnFromNetError2() {
    switchToRoverSpeaker();
    requestMessage(MSG_ANYTHING_ELSE, dARmobjTel::l_msgLabel);
}

// .text 0x25C4
void daRmobjTel_c::holdNetError() {
    getController()->mLock = 1;
}

// .text 0x25D4
void daRmobjTel_c::returnFromNetError3() {
    switchToRoverSpeaker();
    requestMessage(MSG_ANYTHING_ELSE, dARmobjTel::l_msgLabel);
}

// .text 0x2614
void daRmobjTel_c::askNetSetting() {
    if (!dSaveData_c::getRaw()->mExtra.mHeader.mNetEnabled) {
        requestMessage(MSG_WC24_OFF_AGAIN, dARmobjTel::l_msgLabel);
    } else {
        requestMessage(MSG_WC24_ON_AGAIN, dARmobjTel::l_msgLabel);
    }
}

// .text 0x267C
void daRmobjTel_c::openSettingsMenu() {
    dMsg::Comp_c *controller = getController();
    dSelect::Comp_c *select = &controller->mSelect;
    select->setSysMessage(0, SEL_MSG_SOUND, MSG_VOICE, NULL, 0, 0);
    if (getLanguage() == LANGUAGE_JP) {
        select->setSysMessage(1, SEL_MSG_TEXT, MSG_KANA, NULL, 0, 0);
    } else {
        select->setSysMessage(1, SEL_MSG_TEXT, MSG_KEY_INPUT, NULL, 0, 0);
    }
    select->setSysMessage(2, SEL_MSG_MENU, MSG_TUTORIAL, NULL, 0, 0);
    int num = 3;
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUS_NET_EVENT_DONE)) {
        select->setSysMessage(3, SEL_MSG_WC24, dMsg::MSG_CODE_NONE, NULL, 0, 0);
        num = 4;
    }
    controller->mSelect.mCancel = num;
    select->setSysMessage(num, SEL_MSG_NEVER_MIND, MSG_BYE, NULL, 0, 0);
    controller->mSelect.mNum = num + 1;
    controller->openSelect();
}

// .text 0x27B8
BOOL daRmobjTel_c::hasHouseFlag() {
    int room;
    int home = dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
    if (home != -1) {
        dHome_c *house = dSaveData_c::getTown()->mHomes.getHome(home);
        if (house != NULL && house->_15BB.mBits.mHasFlag) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x2830
int daRmobjTel_c::getVoiceMode() {
    return mRoverSpeaks == 0;
}

// .text 0x2840
void daRmobjTel_c::requestMessage(u16 code, const char *label) {
    getController()->setNextMsgCode(code);
    getController()->setNextMsgLabel(label);
}

// .text 0x2888
void daRmobjTel_c::switchToSystemSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (name.isSame(&speaker)) {
        startSpeakerSuspend();
        mRequestEndMethod = &daRmobjTel_c::hideRoverSpeaker;
    }
}

// .text 0x2930
void daRmobjTel_c::switchToRoverSpeaker() {
    dString::Word_c speaker((const wchar_t *)getSpeakerName());
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    if (!name.isSame(&speaker)) {
        startSpeakerSuspend();
        mRequestEndMethod = &daRmobjTel_c::showRoverSpeaker;
    }
}

// .text 0x29D8
void daRmobjTel_c::hideRoverSpeaker() {
    clearSpeakerName();
    startMessageOpen();
}

// .text 0x2A10
void daRmobjTel_c::showRoverSpeaker() {
    dHmnName::Word_c name;
    dItem::Item item((u16)SP_NPC_GUIDE);
    dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
    setSpeakerName(
        (const u16 *)static_cast<dScript::Word_c &>(name).getBuffer(), 2);
    startMessageOpen();
}

// .text 0x2A94
void daRmobjTel_c::startSpeakerSuspend() {
    changeMode(&daRmobjTel_c::executeSpeakerSuspend);
}

// .text 0x2BF0
void daRmobjTel_c::startMessageOpen() {
    changeMode(&daRmobjTel_c::executeMessageOpen);
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
