#pragma once
#include <game/game/d_effect.hpp>
#include <lib/egg/util/eggEffectCreator.hpp>
#include <lib/egg/util/eggEffectManager.hpp>
#include <game/mLib/m_3d.hpp>

// RTTI places these manager classes at global scope.
class dEffectGroupManager : public EGG::EffectCreator {
public:
    dEffectGroupManager(int group) : EGG::EffectCreator(group) {}
    virtual ~dEffectGroupManager() {} // 80088B74
    virtual u32 getGroupID(u32 flags); // 80087D84
};

class dEffectManager_c {
public:
    class DrawProc_c : public m3d::proc_c {
    public:
        DrawProc_c(int group) : mGroup(group) {}
        virtual ~DrawProc_c(); // 80088990
        virtual void drawXlu(); // 8008893C
        static DrawProc_c *create(int group, EGG::Heap *heap); // 800888A8
        /* 0x08 */ mAllocator_c mAllocator;
        /* 0x24 */ int mGroup;
    }; // size 0x28
};

