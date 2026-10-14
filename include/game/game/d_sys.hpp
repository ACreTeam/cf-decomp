#ifndef GAME_D_SYS_HPP
#define GAME_D_SYS_HPP

#include <types.h>
#include <lib/egg/core/eggSystem.h>
#include <game/sLib/s_phase.hpp>

// System startup and frame processing. Names are inferred from behavior.
namespace dSys {
extern EGG::TSystem<>::Configuration *sConfiguration; // 8074E880
extern EGG::ExpHeap *sRootHeapMem1; // 8074E884
extern EGG::ExpHeap *sRootHeapMem2; // 8074E888
extern EGG::ExpHeap *sRootHeapDebug; // 8074E88C
extern bool sBootComplete; // 8074E890: set by the boot sequence, checked by d_reset
extern u8 sUnknownState; // 8074E891: also written by the boot sequence

void beginRender(); // 8016B894
void endRender(); // 8016B8AC
void beginFrame(); // 8016B8E4
void endFrame(); // 8016B934
EGG::SimpleAudioMgr *getAudioMgr(); // unused (stripped); sets the weak getAudioMgr order
BOOL setBlack(u32 black); // 8016B97C: request a video black-state change
void setFrameRate(u8 rate); // 8016B9FC
void setClearColor(const nw4r::ut::Color &color); // 8016BA14
void reportHeaps(); // 8016BA6C: stripped debug hook
void reportHeapStep(int line); // 8016BA70: stripped debug hook, source line
u8 fn_8016BA74(); // 8016BA74: returns a byte that is never set here
void init(); // 8016BA7C
sPhase_c::METHOD_RESULT_e initDyl(void *); // 8016C26C
sPhase_c::METHOD_RESULT_e waitDyl(void *); // 8016C290
sPhase_c::METHOD_RESULT_e linkRoot(void *); // 8016C2BC
sPhase_c::METHOD_RESULT_e linkBootScene(void *); // 8016C2EC
sPhase_c::METHOD_RESULT_e linkErrorScene(void *); // 8016C31C
sPhase_c::METHOD_RESULT_e initLoader(void *); // 8016C34C
sPhase_c::METHOD_RESULT_e initSound(void *); // 8016C370
sPhase_c::METHOD_RESULT_e waitSound(void *); // 8016C394
void runFrame(); // 8016C3C0
} // namespace dSys

#endif
