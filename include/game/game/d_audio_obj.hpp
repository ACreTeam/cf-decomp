#pragma once

// Game-side sound objects attached to actors (DOL TU d_audio_obj.cpp, .text 800107AC..800120A8, .data
// 8049E630..8049E7C8, .sdata 80749818..80749848, not decompiled yet). Class names from the RTTI
// (dAudioObjCharBase_c, dAudioObjPlayer_c, dAudioObjNpc_c).
//
// The TU holds, in order: dAudioObjCharBase_c, dAudioObjPlayer_c (d_a_player), dAudioObjNpc_c, the
// dSndObj* holders of d_snd_obj.hpp (insects, furniture actions), the furniture sound objects
// (d_a_ftr_act, d_ftr_manager; an 11-case factory of SoundTv_* objects), more player functions, and a
// weak tail (SoundHandle ctor, SoundObj<1>/<2> dtors and inline virtuals). One TU: the factory's
// jumptable is the first .data object, before the vtables MWCC writes at the end of the TU.

#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/snd/snd_obj.hpp>

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

    void setMaxSpeed(f32 speed);    // 80010804
    f32 getSpeedRate(f32 speed);    // 8001080C: speed / mMaxSpeed, at most 1 (for SoundFoot::start)

    /* 0x00 */ // vtable
    /* 0x04 */ f32 mMaxSpeed; // the actor's run speed
}; // size 0x8

// The player's sound object (vtable 8049E69C).
class dAudioObjPlayer_c : public dAudioObjCharBase_c {
public:
    dAudioObjPlayer_c();  // 80010828
    ~dAudioObjPlayer_c(); // 8001086C

    virtual void setPos(const mVec3_c *pos);                                 // 800108D4
    virtual void startSound(u32 id);                                         // 80010928
    virtual void startSound(u32 id, const mVec3_c *pos);                     // 8001093C
    virtual void holdSound(u32 id);                                          // 800109B0
    virtual void holdSound(u32 id, const mVec3_c *pos);                      // 800109C0
    virtual void startFootSound(int id, u8 attr, f32 speed);                 // 80010A30
    virtual void startFootSound(int id, u8 attr, const mVec3_c *pos, f32 speed); // 80010A8C

    /* 0x08 */ SoundObjPlayer mObj;
}; // size 0x5C

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

    /* 0x08 */ SoundObjNPC mObj;
}; // size 0x50
