#pragma once

// The game's sound system (DOL TU dol/sound/SoundSystem.cpp, 801CA0F4..801CE58C, not decompiled;
// see snd_manager.hpp). The class has no RTTI; its name comes from the split holding its methods.
// The instance is the function-local static of the inline getInstance (80564964, guard 8074E149;
// the accessor is inlined everywhere, also in SoundBgm/SoundConductor/SoundObj/SoundTv/..., so the
// static and its guard are weak like SoundManager's). The dtor (800105AC) is an empty inline one: its
// first copy is in d_snd_util, with the member dtors SystemLevHandle (800106C4) and SoundHandle
// (8001071C). Only what is used so far; member names are inferred, unknown members keep their
// offsets. Methods of the unsplit TU keep their fn_ names.

#include <types.h>
#include <lib/nw4r/snd.h>
#include <lib/nw4r/ut/ut_list.h>
#include <game/snd/snd_conductor.hpp>
#include <game/snd/snd_fader.hpp>
#include <game/snd/snd_voice.hpp>

class mVec3_c;

extern "C" {
u32 fn_801CE584(); // 801CE584: size of the sound heap (0xE00000)
}

// RTTI "SystemLevHandle" (vtable 80500594; base nw4r::snd::SoundHandle). A sound handle with a
// level; the ctor is inline (its kept copy 801CE564 is in SoundSystem.cpp).
class SystemLevHandle : public nw4r::snd::SoundHandle {
public:
    SystemLevHandle() : _08(0), _0C(0) {}
    virtual ~SystemLevHandle() {} // 800106C4 (weak copy in d_snd_util)

    /* 0x04 vtable */
    /* 0x08 */ u32 _08;
    /* 0x0C */ u32 _0C;
}; // size 0x10

class SoundSystem {
public:
    // Two fade handles (0x00, 0x28) and their state.
    struct FadePair_c {
        // methods in SoundBgm.cpp (the struct may be the TU's SoundBgm)
        const u8 *fn_801D74E0();                         // 801D74E0: the default town tune notes
        void fn_801D74EC(const u8 *notes);               // 801D74EC: copies the town tune notes to _50
        void fn_801D7578(u32 soundId, const u8 *notes);  // 801D7578: plays a melody (NULL: the town tune)

        // two members, not an array: FadePair_c's implicit dtor (inlined in 800105AC) destroys them one by one
        /* 0x00 */ SoundFadeHandle mHandle0;
        /* 0x28 */ SoundFadeHandle mHandle1;
        /* 0x50 */ u8 _50[0x74 - 0x50];
    }; // size 0x74

    // (the ctor is inlined into SoundSystem's: the handles, then _0C = 0 and _10 = a float)
    struct Unk1AC_c {
        /* 0x00 */ nw4r::snd::SoundHandle mHandles[2];
        /* 0x08 */ nw4r::snd::SoundHandle mHandle;
        /* 0x0C */ u32 _0C;
        /* 0x10 */ f32 _10;
    }; // size 0x14

    SoundSystem(); // 801CA0F4
    // declared inline (800105AC, first copy in d_snd_util): an implicit dtor would join the open weak
    // section (snd_obj.hpp's, after ~SoundManager) and ~SystemLevHandle would then follow
    // ~SoundHandle; the target tail has ~SoundSystem, ~SystemLevHandle, ~SoundHandle.
    ~SoundSystem() {}

    static SoundSystem *getInstance() {
        static SoundSystem instance;
        return &instance;
    }

    void fn_801CA264(); // 801CA264: init (after SoundManager::fn_801CE58C)
    void fn_801CB31C(u32 id, int fadeIn); // 801CB31C: start the BGM
    void fn_801CBC90(int fade);           // 801CBC90: stop the BGM
    BOOL fn_801CBDE8();                   // 801CBDE8
    int fn_801CBE58();                    // 801CBE58
    u32 fn_801CC1E8(int song);            // 801CC1E8: a music player song's BGM id
    u32 fn_801CC220(int song);            // 801CC220: a K.K. song's BGM id
    void fn_801CC23C(int weather);        // 801CC23C
    const s8 *fn_801CC874();              // 801CC874
    void fn_801CD38C();                   // 801CD38C
    void fn_801CD4EC();                   // 801CD4EC
    void fn_801CD780();                   // 801CD780
    void fn_801CD8EC();                   // 801CD8EC
    void fn_801CDA78();                   // 801CDA78
    void fn_801CDBF0();                   // 801CDBF0
    void fn_801CE1B4();                   // 801CE1B4
    void fn_801CE394();                   // 801CE394
    // [B] used by d_snd_util 8000F540..800107AC
    void fn_801CBEC8(const mVec3_c *pos);       // 801CBEC8: updates a positional sound (handle 0x44) from pos
    void fn_801CC160(int type);                 // 801CC160: sets _90/_94/_98 from a table
    void fn_801CD710(int state);                // 801CD710: fades the fader at 0xAC (1: out, else in)
    void fn_801CAE08(int se);                   // 801CAE08: plays a sound effect (fn_801CAE10 with pan 0)
    void fn_801CAE10(int se, f32 pan);          // 801CAE10: plays a sound effect with a pan
    void fn_801CB134(int se);                   // 801CB134
    void fn_801CC8D4(const u8 *notes);          // 801CC8D4: plays melody 0xEC5 with notes
    void fn_801CC8E4(u32 idx);                  // 801CC8E4: plays sound 0xED9 + idx (idx < 14)
    void fn_801CC8EC(u32 id);                   // 801CC8EC: mFades.fn_801D7578(id, NULL)
    void fn_801CC8F8();                         // 801CC8F8: the hourly chime (0xEB1)
    void fn_801CC900(const mVec3_c *pos, int arg); // 801CC900
    void fn_801CC998(int a, int b, u16 key1, u16 key2, f32 f); // 801CC998
    void fn_801CCA30(int idx);                  // 801CCA30: sets _198 (0..12), clears _18C/_18D
    void fn_801CD374(f32 value);                // 801CD374: sets _1AC._10
    void fn_801CDD5C(f32 value);                // 801CDD5C: updates handles 0x1F4/0x1F8 (sounds 0x1978/0x1979)

    /* 0x000 */ nw4r::snd::SoundHandle mHandle;
    /* 0x004 */ SystemLevHandle mLevHandles[4];
    /* 0x044 */ nw4r::snd::SoundHandle mHandle44;
    /* 0x048 */ u8 _048[0x0C0 - 0x048];
    /* 0x0C0 */ FadePair_c mFades;
    /* 0x134 */ SoundConductorKK mConductorKK;
    /* 0x16C */ SoundConductor mConductor;
    /* 0x188 */ SoundVoice mVoice;
    /* 0x1AC */ Unk1AC_c _1AC;
    /* 0x1C0 */ u32 _1C0;
    /* 0x1C4 */ u32 _1C4;
    /* 0x1C8 */ u32 _1C8;
    /* 0x1CC */ nw4r::ut::List mLists[3];
    /* 0x1F0 */ nw4r::snd::SoundHandle mHandle1F0;
    /* 0x1F4 */ nw4r::snd::SoundHandle mHandle1F4;
    /* 0x1F8 */ nw4r::snd::SoundHandle mHandle1F8;
}; // size 0x1FC
