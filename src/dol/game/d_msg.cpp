// dMsg: message tag dispatch, windows and text (dMsg::Seq_c, Word_c, Ruby_c, RndJump_c, ...),
// the receipt Rcpt_c and the composer Comp_c (dState<Comp_c>). .text 8019C414..801A6468.
// TODO: only dMsg::Rcpt_c (801A2168..801A2384) is decompiled. The rest of the TU, the tag/window
// code before it and Comp_c after it plus their data, still has to be done.

// TODO: remove this once we've got the RTTI at the right location.
// Keep the duplicate RTTI name string at 804FB42C even when the linker
// selects the earlier weak RTTI record, which references 804A0640.
#pragma force_active on

#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_script.hpp>
#include <cstring>

typedef char dMsgRcptSizeCheck[sizeof(dMsg::Rcpt_c) == 0x64 ? 1 : -1];

namespace dMsg {
void Rcpt_c::init() {
    mpController = NULL;
    mpBmgData = NULL;
    std::memset(mMessageLabel, 0, sizeof(mMessageLabel));
    clearSpeakerName();
    mMessageCode = 0;
    mVoiceType = 1;
}

int Rcpt_c::getVoiceMode() { return 3; }
int Rcpt_c::getForcedVoiceMode() { return 3; }
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
        std::strncpy(mMessageLabel, label, 60);
        mpBmgData = NULL;
    }
}

void Rcpt_c::attachController(Comp_c *controller) { mpController = controller; }
void Rcpt_c::detachController() { mpController = NULL; }

void Rcpt_c::selectMessage(const void *bmg) {}
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
