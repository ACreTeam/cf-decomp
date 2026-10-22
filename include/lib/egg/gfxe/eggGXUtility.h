#pragma once

#include <nw4r/g3d.h>
#include <revolution/gx.h>

namespace EGG {

namespace GXUtility {
    // 80448504: scales/offsets the perspective texture-matrix terms (names follow Skyward Sword's EGG).
    void setScaleOffsetPerspective(f32 *pOut, f32 f1, f32 f2, f32 f3, f32 f4);
    // 80448544
    void getTexObj(GXTexObj *obj, nw4r::g3d::ResTex resTex, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                   GXTexFilter minFilter, GXTexFilter magFilter);
}

} // namespace EGG
