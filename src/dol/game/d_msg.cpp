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
    mField60 = 0;
    std::memset(mMessageLabel, 0, sizeof(mMessageLabel));
    clearSpeakerName();
    mMessageCode = 0;
    mField58 = 1;
}

int Rcpt_c::rcptHook88() { return 3; }
int Rcpt_c::rcptHook90() { return 3; }
int Rcpt_c::rcptHook8C() { return 0; }

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
        mField60 = 0;
    }
}

void Rcpt_c::attachController(Comp_c *controller) { mpController = controller; }
void Rcpt_c::detachController() { mpController = NULL; }

void Rcpt_c::rcptHook0C() {}
void Rcpt_c::rcptHook10() {}
void Rcpt_c::rcptHook14() {}
void Rcpt_c::rcptHook18() {}
void Rcpt_c::rcptHook1C() {}
void Rcpt_c::rcptHook20() {}
void Rcpt_c::rcptHook24() {}
void Rcpt_c::rcptHook28() {}
void Rcpt_c::rcptHook2C() {}
void Rcpt_c::rcptHook78() {}
void Rcpt_c::rcptHook34(u32 tag) {}
void Rcpt_c::rcptHook38() {}
void Rcpt_c::rcptHook3C() {}
void Rcpt_c::rcptHook40() {}
void Rcpt_c::rcptHook44() {}
void Rcpt_c::rcptHook48() {}
void Rcpt_c::rcptHook4C() {}
void Rcpt_c::rcptHook50() {}
void Rcpt_c::rcptHook54() {}
void Rcpt_c::rcptHook58() {}
void Rcpt_c::rcptHook5C() {}
void Rcpt_c::rcptHook60() {}
void Rcpt_c::rcptHook64() {}
void Rcpt_c::rcptHook68() {}
void Rcpt_c::rcptHook6C() {}
void Rcpt_c::rcptHook70() {}
void Rcpt_c::rcptHook74() {}
} // namespace dMsg
