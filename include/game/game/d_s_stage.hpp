#pragma once

// d_s_stage (REL d_s_stageNP, profile S_STAGE): the game scene dScStage_c (its
// RTTI name). It loads the scene's resources and sound, creates the field
// (fProf::FIELD) under the root, sets the scene up (dScene::createScene) and
// draws the 3D world through dSysGfx::sDrawCallback; deleting it waits for the
// photo, the network post and the field to finish. Member names other than
// the fBase_c overrides are inferred.

#include <types.h>
#include <game/game/d_scene_base.hpp>

class dScStage_c : public dScene_c {
  public:
    virtual int create();       // 0x150
    virtual int doDelete();     // 0x290
    virtual int execute();      // 0x3AC
    virtual int draw();         // 0x3F4
    virtual void deleteReady(); // 0x48C
    virtual int getKind() {
        return SCENE_KIND_ONLINE;
    } // 0x5D8

    static void drawCallback(); // 0xC0: the scene draw (dSysGfx::sDrawCallback)

    BOOL createField(); // 0x490: FALSE until the field is ready
    BOOL deleteField(); // 0x508: FALSE while the field is still up

    /* 0x64 */ fBase_c *mField;    // the field (fProf::FIELD)
    /* 0x68 */ bool mSceneUpdated; // dScene::updateCurrentScene ran
}; // size 0x6C
