#pragma once
#include <nw4r/math.h>
#include <nw4r/ef/ef_drawinfo.h>

namespace EGG {
class EffectCreator;
class Heap;

class EffectManager {
public:
    struct Configuration {
        Heap *heap;
        int effects;
        int emitters;
        int managers;
        int particles;
    };

    static EffectManager *createInstance(int creators, int groups, Heap *heap); // 8044BD68
    static EffectManager *sEffectManager;
    int initialize(Configuration &config); // 8044BDF4
    void setCreator(EffectCreator *, int); // 8044C2FC
    void shutdown(); // 8044C320
    void setCamera(const nw4r::math::VEC3 *, const nw4r::math::MTX34 *, f32, f32, int, bool); // 8044C390
    void draw(nw4r::ef::DrawInfo *, int, int); // 8044C448
};

} // namespace EGG
