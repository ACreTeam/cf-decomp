#pragma once

#include <types.h>

namespace dMsg {
class Comp_c;

// RTTI confirms that this is an independent polymorphic base, size 0x64.
// Hook names/signatures are provisional where the target contains only blr.
class Rcpt_c {
public:
    Rcpt_c() { init(); }
    virtual ~Rcpt_c() {} // 8002ECF4, shared weak definition
    virtual void rcptHook0C(); // vtable +0xC
    virtual void rcptHook10(); // vtable +0x10
    virtual void rcptHook14(); // vtable +0x14
    virtual void rcptHook18(); // vtable +0x18
    virtual void rcptHook1C(); // vtable +0x1C
    virtual void rcptHook20(); // vtable +0x20
    virtual void rcptHook24(); // vtable +0x24
    virtual void rcptHook28(); // vtable +0x28
    virtual void rcptHook2C(); // vtable +0x2C
    virtual void init(); // +0x30, 801A2168
    virtual void rcptHook34(u32 tag); // called with a u16 message tag value (dAcNpc_c::recept_c: feel)
    virtual void rcptHook38();
    virtual void rcptHook3C();
    virtual void rcptHook40();
    virtual void rcptHook44();
    virtual void rcptHook48();
    virtual void rcptHook4C();
    virtual void rcptHook50();
    virtual void rcptHook54();
    virtual void rcptHook58();
    virtual void rcptHook5C();
    virtual void rcptHook60();
    virtual void rcptHook64();
    virtual void rcptHook68();
    virtual void rcptHook6C();
    virtual void rcptHook70();
    virtual void rcptHook74();
    virtual void rcptHook78();
    virtual int rcptHook7C() { return 0; } // 8002ED44
    virtual int rcptHook80() { return 0; } // 8002ED3C
    virtual int rcptHook84() { return 0; } // 8002ED34
    virtual int rcptHook88(); // 801A21C8, returns 3
    virtual int rcptHook8C(); // 801A21D8, returns 0
    virtual int rcptHook90(); // 801A21D0, returns 3

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
    int mField58; // 0x58; initialized to 1, meaning not yet established
    Comp_c *mpController; // 0x5C; controller keeps the reciprocal link
    u32 mField60; // 0x60; cleared on init and nonempty message-label changes
};
} // namespace dMsg
