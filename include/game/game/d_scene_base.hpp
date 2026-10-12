#pragma once

// d_scene_base (DOL TU d_scene_base.cpp, .text 801553E0..8015593C): the scene base class dScene_c (its RTTI name)
// and the scene change it runs (the next scene's profile, its fades), plus the d_sys hooks of the frame's
// performance meter (mPerf). The NSMBW design (dScene_c::m_nextScene, its fade-in / fade-out frames); the names
// of the statics and of the d_sys hooks are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>

namespace EGG {
class Heap;
}

// Scene profile names used by the scene change.
enum {
    SCENE_PROF_BOOT = 0xA6,      // dScBoot_c (the boot scene)
    SCENE_PROF_ERROR = 0xA2,     // the scene of an error reset (d_reset)
    SCENE_PROF_NONE = 0x12C,     // m_nextScene: no scene change pending
};

// dScene_c::getKind results.
enum {
    SCENE_KIND_BOOT = 0, // the boot / title scenes
    SCENE_KIND_ONLINE = 2, // an online scene: postCreate clears the d_net busy state
};

class dScene_c : public fBase_c {
public:
    // Scenes are root-disabled (execute and draw) until their children are created (preExecute).
    dScene_c() { setProcControlFlag(ROOT_DISABLE_EXECUTE | ROOT_DISABLE_DRAW); }
    ~dScene_c() {}

    virtual int preCreate();                     // 801553E0
    virtual void postCreate(MAIN_STATE_e state); // 801553EC
    virtual int preDelete();                     // 80155464
    virtual void postDelete(MAIN_STATE_e state); // 80155490
    virtual int preExecute();                    // 801554F8
    virtual void postExecute(MAIN_STATE_e state); // 80155744
    virtual int preDraw();                       // 80155770
    virtual void postDraw(MAIN_STATE_e state);   // 8015579C

    // vtable 0x4C: the scene's kind (SCENE_KIND_*).
    virtual int getKind() = 0;

    static void initNextScene();         // 801557A0: the boot scene comes first (d_sys)
    static fBase_c *createNextScene();   // 801557B4: creates m_nextScene under the root (d_sys frame)

    // The d_sys hooks of the performance meter (mPerf; empty in this build).
    static void createPerf(EGG::Heap *heap); // 801558D0
    static void beginPerf();                 // 801558D4
    static void checkPerf();                 // 801558D8: sets / clears the d_main stop reason 8
    static void endPerf();                   // 80155938

    static ProfileName m_nextScene;  // 8074B1B0: SCENE_PROF_NONE when no scene change is pending
    static bool m_sceneCreated;      // 8074B1B2: the next scene was created (no new one until this one is deleted)
    static u16 m_fadeInFrame;        // 8074B1B4
    static u16 m_fadeOutFrame;       // 8074B1B6
    static dScene_c *m_nowScene;     // 8074E800: the current scene (set by its preCreate, cleared by its postDelete)
    static int m_fadeInType;         // 8074E804: the dFader::TYPE_e of the fade-in
    static int m_fadeOutType;        // 8074E808: the dFader::TYPE_e of the fade-out
};
