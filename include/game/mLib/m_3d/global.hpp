#pragma once

#include <game/mLib/m_allocator.hpp>
#include <lib/egg/gfxe/eggLightManager.h>
#include <lib/egg/gfxe/eggFogManager.h>
#include <lib/egg/gfxe/eggScreen.h>
#include <nw4r/g3d.h>

/// @addtogroup mlib
/// @{

/// @brief mLib 3D library
namespace m3d {
    class bmdl_c;

    bool create(EGG::Heap *heap, ulong maxChildren, ulong maxScnObj, ulong numLightObj, ulong numLightSet);

    bool createFogMgr(EGG::Heap *heap, int numFog);
    void removeFogMgr();

    nw4r::g3d::ScnRoot *getScnRoot();
    nw4r::g3d::Camera getCamera(int idx);
    nw4r::g3d::Camera getCurrentCamera();
    int getCurrentCameraID();
    void setCurrentCamera(int idx);

    nw4r::g3d::LightSetting *getLightSettingP();
    EGG::LightManager *getLightMgr();
    EGG::FogManager *getFogMgr();

    void calcWorld();
    void calcMaterial();
    void calcView();

    void drawOpa();
    void drawXlu();

    bool pushBack(nw4r::g3d::ScnObj *obj);

    void clear();
    void reset();

    int getMatID(nw4r::g3d::ResMdl mdl, char const *name);
    int getNodeID(nw4r::g3d::ResMdl mdl, char const *name);

    void resetMaterial(); ///< Turns off all indirect texture processing.

    void screenEffectReset(int cameraID, EGG::Screen &screen);
}
/// @}
