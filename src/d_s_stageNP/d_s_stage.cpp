// The game scene (dScStage_c). See include/game/game/d_s_stage.hpp.
#include <game/game/d_s_stage.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_effect_manager.hpp>
#include <game/game/d_fader.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_item_debug.hpp>
#include <game/game/d_m2d.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_root.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_sys_gfx.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_2d.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <revolution/gx.h>

// drawCallback: the 2D layouts with a priority below this one are drawn
// between the two 3D passes (the world, then the camera 1 overlay), the rest
// after them.
#define STAGE_2D_SPLIT_PRIORITY 0x83

namespace dSStage {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text 0x68
BASE_PROFILE(S_STAGE, dScStage_c);
} // namespace dSStage

// .text 0xC0
void dScStage_c::drawCallback() {
    GXInvalidateTexAll();
    m2d::executeCaptures();
    m3d::calcView();
    m3d::drawOpa();
    m3d::drawXlu();
    m2d::defaultSet();
    m2d::drawBefore(STAGE_2D_SPLIT_PRIORITY);
    m3d::clear();
    m3d::reset();
    m3d::setCurrentCamera(1);
    m2d::executeCallbacks();
    m3d::calcMaterial();
    m3d::calcView();
    m3d::drawOpa();
    m3d::drawXlu();
    m2d::defaultSet();
    m2d::drawAfter(STAGE_2D_SPLIT_PRIORITY);
    dEffect::draw(0, 3);
    dEffect::draw(0, 2);
    m2d::reset();
    m3d::setCurrentCamera(0);
}

// .text 0x150
int dScStage_c::create() {
    dFader::fn_8008BA48();
    if (!fgMngProc_isDayUpToDate() && dDemo::isDayChangeWarp()) {
        fgMngProc_procDayChange(FALSE);
    }
    dTime_c::update();
    if (!dScene::isSceneResLoaded()) {
        return NOT_READY;
    }
    if (!mSceneUpdated) {
        dScene::updateCurrentScene();
        mSceneUpdated = true;
    }
    if (!createField()) {
        return NOT_READY;
    }
    if (!sndSetupStage(dScene::getCurrentScene())) {
        return NOT_READY;
    }
    dSys::setFrameRate(1);
    nw4r::ut::Color clear(0x000000FF);
    dSys::setClearColor(clear);
    if (dReset::Manage_c::GetInstance()->isSoftResetFade()) {
        return FAILED;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        player->syncHouseSize();
    }
    dSysGfx::sDrawCallback = drawCallback;
    dDemoActor_c::initActorList();
    dScene::createScene(this);
    dBGCF::fn_800762C8();
    dDemo::startScene();
    dEffect::start();
    m2d::initCaptureList();
    dItemDebug::create();
    return SUCCEEDED;
}

// .text 0x290
int dScStage_c::doDelete() {
    if (!dResetSave::isIdle()) {
        return NOT_READY;
    }
    BOOL ready = TRUE;
    if (dReset::Manage_c::GetInstance()->isResetting() ||
        dSStage::getSceneChange()->mScene == dScene::SCENE_DM_TITLE) {
        if (!dPhoto::tryClear()) {
            ready = FALSE;
        }
    }
    if (!dNet::isNetReadyToPost()) {
        ready = FALSE;
    }
    if (!ready) {
        return NOT_READY;
    }
    if (!dScene::releaseSceneRes()) {
        return NOT_READY;
    }
    if (!deleteField()) {
        return NOT_READY;
    }
    dEffect::shutdown();
    dSysGfx::sDrawCallback = NULL;
    dScene::destroyScene();
    dBGCF::fn_800762E0();
    if (!dItemDebug::draw()) {
        return NOT_READY;
    }
    dDemo::offSilence();
    dReset::Manage_c::GetInstance()->SetSoftResetScene();
    return SUCCEEDED;
}

// .text 0x3AC
int dScStage_c::execute() {
    dFgMngProc_c::execute();
    dTime_c::update();
    fgMngProc_updateFrame();
    dWorld::getEdge()->set(dCamera_c::sCenter);
    dBGCF::updateUnitAnm();
    dSceneCheck::fn_80081854();
    dItemDebug::execute();
    return SUCCEEDED;
}

// .text 0x3F4
int dScStage_c::draw() {
    dEffect::l_drawWorld->entry();
    dEffect::l_drawOverlay->entry();
    dEffect::l_drawSky->entry();
    dEffect::l_drawTinted->entry();
    dEffect::l_drawBackground->entry();
    return SUCCEEDED;
}

// .text 0x48C
void dScStage_c::deleteReady() {
}

// .text 0x490
BOOL dScStage_c::createField() {
    if (mField == NULL) {
        mField = dBase_c::createBase(fProf::FIELD, dRoot_c::sInstance, 0, 0);
    }
    if (mField != NULL && !dField::sSceneReady) {
        return FALSE;
    }
    return TRUE;
}

// .text 0x508
BOOL dScStage_c::deleteField() {
    if (dField::sSceneReady) {
        if (mField != NULL) {
            mField->deleteRequest();
            mField = NULL;
        }
        return FALSE;
    }
    mField = NULL;
    return TRUE;
}
