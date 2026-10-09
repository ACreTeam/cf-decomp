#pragma once

// Game-side sound objects attached to actors (DOL TU d_audio_obj.cpp, .text 800107AC..800120A8, not
// decompiled yet). Class names from the RTTI (dAudioObjCharBase_c, dAudioObjNpc_c); only what d_a_npc
// uses is declared.

#include <types.h>
#include <game/mLib/m_vec.hpp>

// Abstract base (vtable 8049E6E0: 7 pure slots, no virtual dtor). ctor 800107AC, dtor 800107C4.
class dAudioObjCharBase_c {
public:
    dAudioObjCharBase_c();  // 800107AC
    ~dAudioObjCharBase_c(); // 800107C4

    virtual void setPos(const mVec3_c *pos) = 0;                                 // +0x08
    virtual void startSound(u32 id) = 0;                                         // +0x0C
    virtual void startSound(u32 id, const mVec3_c *pos) = 0;                     // +0x10
    virtual void holdSound(u32 id) = 0;                                          // +0x14
    virtual void holdSound(u32 id, const mVec3_c *pos) = 0;                      // +0x18
    virtual void startFootSound(int id, u8 attr, f32 speed) = 0;                 // +0x1C
    virtual void startFootSound(int id, u8 attr, const mVec3_c *pos, f32 speed) = 0; // +0x20

    void setMaxDist(f32 dist); // 80010804: stores +0x04

    /* 0x00 */ // vtable
    /* 0x04 */ f32 mMaxDist;
}; // size 0x8

// The NPC's sound object (dAcNpc_c::mAudioObj at 0x1B88). Vtable 8049E65C = {80010BCC setPos,
// 80010C20, 80010C34, 80010CA8, 80010CB8, 80010D28, 80010D84} (no dtor slot).
class dAudioObjNpc_c : public dAudioObjCharBase_c {
public:
    dAudioObjNpc_c();  // 80010B20
    ~dAudioObjNpc_c(); // 80010B64

    virtual void setPos(const mVec3_c *pos);                                 // 80010BCC
    virtual void startSound(u32 id);                                         // 80010C20
    virtual void startSound(u32 id, const mVec3_c *pos);                     // 80010C34
    virtual void holdSound(u32 id);                                          // 80010CA8
    virtual void holdSound(u32 id, const mVec3_c *pos);                      // 80010CB8
    virtual void startFootSound(int id, u8 attr, f32 speed);                 // 80010D28
    virtual void startFootSound(int id, u8 attr, const mVec3_c *pos, f32 speed); // 80010D84

    /* 0x08 */ u8 _08[0x50 - 0x08]; // sound handles (+0x08 object with a vtable, +0x3C)
}; // size 0x50
