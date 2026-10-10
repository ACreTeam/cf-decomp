#pragma once

#include <types.h>

class dAnimal_c;

namespace dMsg {
class Comp_c;

// RTTI confirms that this is an independent polymorphic base, size 0x64.
// Hook names/signatures are provisional where the target contains only blr.
class Rcpt_c {
public:
    Rcpt_c() { init(); }
    virtual ~Rcpt_c() {} // 8002ECF4, shared weak definition
    virtual void selectMessage(const void *bmg); // vtable +0xC
    virtual void onMessageStart(int arg); // vtable +0x10
    virtual void onMessageEnd(int kind); // vtable +0x14
    virtual void onAnswer(int arg); // vtable +0x18
    virtual void onKeyWait(); // vtable +0x1C
    virtual void onPageWait(); // vtable +0x20
    virtual void onTextEnd(); // vtable +0x24
    virtual void onSelectStart(); // vtable +0x28
    virtual void onTypeStart(); // vtable +0x2C
    virtual void init(); // +0x30, 801A2168
    virtual void setSpeakerFeel(u32 tag); // called with a u16 message tag value (dAcNpc_c::recept_c: feel)
    virtual void resetMood();
    virtual void setMood1(int hours);
    virtual void setMood2(int hours);
    virtual void setMood3(int hours);
    virtual void setMood4(int hours);
    virtual void setSpeakerPlayer();
    virtual void setSpeakerNpc();
    virtual void setSpeakerPartner();
    virtual void speakerLookAtPlayer();
    virtual void speakerTurnToPlayer();
    virtual void speakerLookAtNpc();
    virtual void speakerTurnToNpc();
    virtual void speakerLookAtPartner();
    virtual void speakerTurnToPartner();
    virtual void changeSpeaker();
    virtual void playTownTune();
    virtual void onCustomTag(int no);
    virtual dAnimal_c *getSpeakerAnimal() { return NULL; } // 8002ED44
    virtual dAnimal_c *getListenerAnimal() { return NULL; } // 8002ED3C
    virtual int isSpeakerItchy() { return 0; } // 8002ED34
    virtual int getVoiceMode(); // 801A21C8, returns 3
    virtual int getVoiceMood(); // 801A21D8, returns 0
    virtual int getForcedVoiceMode(); // 801A21D0, returns 3

    void setSpeakerName(const u16 *name, u8 nameKind); // 801A21E0
    void clearSpeakerName(); // 801A2260
    void setMessageCode(u16 code); // 801A22A0
    void setMessageLabel(const char *label); // 801A22A8
    void attachController(Comp_c *controller); // 801A2304
    void detachController(); // 801A230C

    Comp_c *getController() const { return mpController; }

protected:
    char mMessageLabel[61]; // 0x04; copy limit 60, extra terminator byte
    u16 mMessageCode; // 0x42
    u16 mSpeakerName[9]; // 0x44
    u8 mSpeakerNameKind; // 0x56; clear initializes to 2
    int mVoiceType; // 0x58; initialized to 1, meaning not yet established
    Comp_c *mpController; // 0x5C; controller keeps the reciprocal link
    const void *mpBmgData; // 0x60; cleared on init and nonempty message-label changes
};
} // namespace dMsg
