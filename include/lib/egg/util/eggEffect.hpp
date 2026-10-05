#pragma once
#include <nw4r/math.h>
#include <nw4r/ef/ef_handle.h>
#include <revolution/GX/GXTypes.h>

// City Folk's EGG::Effect (vtable 80561BE0, ctor 8044A11C, dtor 8044A188). Its vtable has two
// fewer virtuals than NSMBW's before setRegisterColor; which two is unknown, so 0x0C..0x50 are
// placeholders. From 0x54 on the slots line up with NSMBW's shifted by -8 bytes: the ctor calls
// reset() at 0x90, d_fireworks calls setRegisterColor (0x54), setScale(f32) (0x78) and update
// (0x8C). Size 0x7C (a stack instance spans 0x7C bytes in d_fireworks).

namespace EGG {

class Effect {
public:
    enum ERecursive {
        RECURSIVE_3 = 3
    };

    Effect();          // 8044A11C
    virtual ~Effect(); // 8044A188
    virtual void vf0C();
    virtual void vf10();
    virtual void vf14();
    virtual void vf18();
    virtual void vf1C();
    virtual void vf20();
    virtual void vf24();
    virtual void vf28();
    virtual void vf2C();
    virtual void vf30();
    virtual void vf34();
    virtual void vf38();
    virtual void vf3C();
    virtual void vf40();
    virtual void vf44();
    virtual void vf48();
    virtual void vf4C();
    virtual void vf50();
    virtual void setRegisterColor(const _GXColor &color0, const _GXColor &color1, u8 index); // 0x54
    virtual void vf58();
    virtual void vf5C();
    virtual void vf60();
    virtual void vf64();
    virtual void vf68();
    virtual void vf6C();
    virtual void vf70();
    virtual void vf74();
    virtual void setScale(float scale); // 0x78
    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual void vf88();
    virtual void update(); // 0x8C (8044B6C4)
    virtual void reset();  // 0x90

    /* 0x04 */ u8 _04;
    /* 0x05 */ u8 _05[0x1F];
    /* 0x24 */ u32 _24;
    /* 0x28 */ u32 _28;
    /* 0x2C */ u8 _2C[0x48];
    /* 0x74 */ nw4r::ef::HandleBase mHandle;
}; // size 0x7C

} // namespace EGG
