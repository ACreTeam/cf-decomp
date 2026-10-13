#pragma once

namespace EGG {
class EffectCreator;

class EffectManager {
public:
    static EffectManager *sEffectManager;
    void setCreator(EffectCreator *, int); // 8044C2FC
};

} // namespace EGG
