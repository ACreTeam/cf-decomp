// The player options (dSaveOption): each player's option bits (dPrivateData_c+0x83E8) and the static working
// copy with its changed flags (dSaveData_c::sOption). .text 8010D7E8..8010DC04, .sbss 8074E6D8..8074E6E0.
// See include/game/game/d_save_data.hpp (dSaveOption_c).
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <cstring>

// 8074E6D8
dSaveOption_c dSaveData_c::sOption;

static inline dSaveOption_c *getOption(dPrivateData_c *player) {
    return (dSaveOption_c *)&player->_83E8;
}

namespace dSaveOption {

// 8010D7E8
void setDefaults(dSaveOption_c *opt) {
    opt->mKana = 1;
    opt->mVoiceVolume = 6;
    opt->mVoiceMode = 0;
    opt->mKeySe = 0;
    opt->mNoTutorial = 0;
    opt->mBit2 = 0;
    opt->mVoiceChat = 0;
    opt->mVoiceSound = 1;
}

// 8010D808
void setVoiceVolume(dSaveOption_c *opt, u8 value) {
    opt->mVoiceVolume = value;
}

// 8010D810
void getBits(const dSaveOption_c *opt, void *dst) {
    memcpy(dst, opt, 1);
}

// 8010D824
void setBits(dSaveOption_c *opt, const void *src) {
    memcpy(opt, src, 1);
}

// 8010D82C
void clearChanged(dSaveOption_c *opt) {
    opt->mKanaChanged = 0;
    opt->mVoiceModeChanged = 0;
    opt->mKeySeChanged = 0;
    opt->mNoTutorialChanged = 0;
    opt->mBit2Changed = 0;
    opt->mVoiceChatChanged = 0;
    opt->mVoiceSoundChanged = 0;
}

// 8010D83C
void reset(dSaveOption_c *opt) {
    setDefaults(opt);
    clearChanged(opt);
}

// 8010D870
void setNewPlayerDefaults(dSaveOption_c *opt) {
    BOOL flag = TRUE;

    if (dSaveData_c::getTown()->isHeaderState2()) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(i);
            if (player->mPID.isValid() && getOption(player)->isKana() == TRUE) {
                flag = FALSE;
            }
        }
    } else {
        flag = FALSE;
    }

    if (flag) {
        opt->mKana = 0;
    } else {
        opt->mKana = 1;
    }
    opt->mVoiceMode = 0;
    opt->mKeySe = 0;
    opt->mNoTutorial = 0;
    opt->mBit2 = 0;
    opt->mVoiceChat = 0;
    opt->mVoiceSound = 1;
}

// 8010D944
int getKana() {
    return dSaveData_c::sOption.mKana;
}

// 8010D950
void setKana(BOOL v) {
    dSaveData_c::sOption.mKana = v;
    dSaveData_c::sOption.mKanaChanged = 1;
}

// 8010D970
int getVoiceMode() {
    return dSaveData_c::sOption.mVoiceMode;
}

// 8010D97C
void setVoiceMode(int v) {
    dSaveData_c::sOption.mVoiceMode = v;
    dSaveData_c::sOption.mVoiceModeChanged = 1;
}

// 8010D99C
BOOL getKeySe() {
    return dSaveData_c::sOption.mKeySe;
}

// 8010D9A8
void setKeySe(BOOL v) {
    dSaveData_c::sOption.mKeySe = v;
    dSaveData_c::sOption.mKeySeChanged = 1;
}

// 8010D9C8
BOOL isTutorialOff() {
    return dSaveData_c::sOption.mNoTutorial;
}

// 8010D9D4
void setTutorialOff(BOOL v) {
    dSaveData_c::sOption.mNoTutorial = v;
    dSaveData_c::sOption.mNoTutorialChanged = 1;
}

} // namespace dSaveOption

// 8010D9F4: probably mBit2's getter, stubbed
extern "C" int fn_8010D9F4() {
    return 0;
}

namespace dSaveOption {

// 8010D9FC
BOOL getVoiceChat() {
    return dSaveData_c::sOption.mVoiceChat;
}

// 8010DA08
void setVoiceChat(u8 v) {
    dSaveData_c::sOption.mVoiceChat = v;
    dSaveData_c::sOption.mVoiceChatChanged = 1;
}

// 8010DA28
BOOL getVoiceSound() {
    return dSaveData_c::sOption.mVoiceSound;
}

// 8010DA34
void setVoiceSound(u8 v) {
    dSaveData_c::sOption.mVoiceSound = v;
    dSaveData_c::sOption.mVoiceSoundChanged = 1;
}

// 8010DA54
u8 getPlayerVoiceVolume() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player == NULL) {
        return 0;
    }
    return getOption(player)->mVoiceVolume;
}

// 8010DA8C
void setPlayerVoiceVolume(u8 v) {
    setVoiceVolume(getOption(dPlayerMgr::Mgr_c::getCurrentPlayer()), v);
}

// 8010DAC8
void storeToCurrentPlayer() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        store(getOption(player));
    }
}

// 8010DAFC
void store(dSaveOption_c *opt) {
    dSaveOption_c *s = &dSaveData_c::sOption;
    u8 byte;

    if (s->mKanaChanged) {
        opt->mKana = getKana();
    }
    if (s->mVoiceModeChanged) {
        opt->mVoiceMode = getVoiceMode();
    }
    if (s->mKeySeChanged) {
        opt->mKeySe = getKeySe();
    }
    if (s->mNoTutorialChanged) {
        opt->mNoTutorial = isTutorialOff();
    }
    if (s->mVoiceChatChanged) {
        opt->mVoiceChat = getVoiceChat();
    }
    if (s->mVoiceSoundChanged) {
        opt->mVoiceSound = getVoiceSound();
    }

    getBits(opt, &byte);
    clearChanged(s);
    setBits(s, &byte);
}

} // namespace dSaveOption

// 8010DBFC
extern "C" int fn_8010DBFC() {
    return 0;
}
