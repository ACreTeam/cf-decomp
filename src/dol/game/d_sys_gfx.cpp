// The graphics system creation and the reset draw callback (dSysGfx). .text 800AC1CC..800AC260, .sbss 8074E398.
// See include/game/game/d_sys_gfx.hpp.
#include <game/game/d_sys_gfx.hpp>
#include <game/game/d_light.hpp>
#include <game/game/d_m2d.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_face.hpp>
#include <lib/egg/core/eggHeap.h>

namespace dSysGfx {

void (*sDrawCallback)();

// 800AC1CC
void create(EGG::Heap *heap) {
    sDrawCallback = NULL;
    m3d::create(EGG::Heap::getCurrentHeap(), 0x100, 0x100, 0x10, 2);
    m3d::createFogMgr(EGG::Heap::getCurrentHeap(), 1);
    dLight::init();
    m2d::create(heap, 0xC8000);
    m2d::ResAccLoader_c::create(heap, 0x41A000);
    mFace::create();
}

// 800AC248
void draw() {
    if (sDrawCallback != NULL) {
        sDrawCallback();
    }
}

} // namespace dSysGfx
