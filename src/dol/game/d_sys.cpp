#include <game/cLib/c_counter.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_manager.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_black_wait.hpp>
#include <game/game/d_caution.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_dylink.hpp>
#include <game/game/d_fader.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <game/game/d_hmn_head_item_mng.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_home_button.hpp>
#include <game/game/d_kbd.hpp>
#include <game/game/d_main.hpp>
#include <game/game/d_mii_icon.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_reset.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_scene_base.hpp>
#include <game/game/d_shutdown_fader.hpp>
#include <game/game/d_shutter_fader.hpp>
#include <game/game/d_side_wipe_fader.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_sys.hpp>
#include <game/game/d_sys_gfx.hpp>
#include <game/game/d_thunder.hpp>
#include <game/game/d_wifi_err.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_dvd.hpp>
#include <game/mLib/m_face.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/mLib/m_perf.hpp>
#include <game/mLib/m_video.hpp>
#include <lib/egg/core/eggDecomp.h>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/gfxe/eggDrawGX.h>

namespace dSys {
enum {
    ROOT_HEAP_ALIGNMENT = 4,
    GAME_HEAP_SIZE = 0x200000,
    ARCHIVE_HEAP_SIZE = 0x100,
    COMMAND_HEAP_SIZE = 0x40000,
    DYLINK_HEAP_SIZE = 0x318000,
    PROFILE_ROOT = fProf::S_STAGE,
    STARTUP_METHOD_NUM = 8,
};

EGG::TSystem<>::Configuration *sConfiguration;
EGG::ExpHeap *sRootHeapMem1;
EGG::ExpHeap *sRootHeapMem2;
EGG::ExpHeap *sRootHeapDebug;
bool sBootComplete;
u8 sFaceError;
extern sPhase_c sStartupPhase;
static EGG::TSystem<>::Configuration sSystem;

// 8016B894
void beginRender() {
    sConfiguration->mpDisplay->beginRender();
}
// 8016B8AC
void endRender() {
    fn_8016CEFC();
    sConfiguration->mpDisplay->endRender();
}
// 8016B8E4
void beginFrame() {
    sConfiguration->mpDisplay->beginFrame();
    sConfiguration->onBeginFrame();
    fn_8016CF00();
    fn_8016CF04();
}
// 8016B934
void endFrame() {
    sConfiguration->mpDisplay->endFrame();
    sConfiguration->onEndFrame();
}
// Unused, stripped from the DOL: its call queues getAudioMgr's weak copy right
// after onEndFrame.
EGG::SimpleAudioMgr *getAudioMgr() {
    return sConfiguration->getAudioMgr();
}
// 8016B97C
BOOL setBlack(u32 black) {
    EGG::AsyncDisplay *display = sConfiguration->mpDisplay;
    if (black != EGG::BaseSystem::mConfigData->getVideo()->isBlack() &&
        !(display->mScreenStateFlag & 1)) {
        display->mScreenStateFlag |= 1;
        return TRUE;
    }
    return FALSE;
}
// 8016B9FC
void setFrameRate(u8 rate) {
    EGG::AsyncDisplay *display = sConfiguration->mpDisplay;
    if (display != NULL) {
        display->mFrameRate = rate;
    }
}
// 8016BA14
void setClearColor(const nw4r::ut::Color &color) {
    EGG::AsyncDisplay *display = sConfiguration->mpDisplay;
    if (display != NULL) {
        display->setClearColor(color);
    }
}
// 8016BA6C
void reportHeaps() {
}
// 8016BA70
void reportHeapStep(int line) {
}
// 8016BA74
BOOL isFaceError() {
    return sFaceError;
}

// 8016BA7C
void init() {
    EGG::Heap *archive;
    EGG::Heap *command;

    sConfiguration = &sSystem;
    EGG::BaseSystem::mConfigData = sConfiguration;
    sConfiguration->initialize();
    sRootHeapMem1 = EGG::ExpHeap::create(
        -1, EGG::BaseSystem::mConfigData->mRootHeapMem1, ROOT_HEAP_ALIGNMENT);
    sRootHeapMem1->setName("dSys_c::RootHeapMEM1");
    sRootHeapMem2 = EGG::ExpHeap::create(
        -1, EGG::BaseSystem::mConfigData->mRootHeapMem2, ROOT_HEAP_ALIGNMENT);
    sRootHeapMem2->setName("dSys_c::RootHeapMEM2");
    if (EGG::BaseSystem::mConfigData->mRootHeapDebug != NULL) {
        sRootHeapDebug = EGG::ExpHeap::create(
            -1, EGG::BaseSystem::mConfigData->mRootHeapDebug,
            ROOT_HEAP_ALIGNMENT);
        sRootHeapDebug->setName("dSys_c::RootHeapDebug");
    }
    mVideo::m_video = (mVideo *)sConfiguration->mpVideo;
    dPad::init();
    mPad::create();
    mPad::beginPad();
    mPad::g_currentCore->setPosParam(0.1f, 1.0f);
    mPad::endPad();
    EGG::Heap *mem1 = sRootHeapMem1;
    EGG::Heap *mem2 = sRootHeapMem2;
    EGG::Heap *debug = sRootHeapDebug;
    reportHeaps();
    mHeap::createGameHeap(GAME_HEAP_SIZE, mem1);
    reportHeapStep(0x2d5);
    dHeap::createLocalHeap(mem2);
    reportHeapStep(0x2d8);
    mHeap::createArchiveHeap(ARCHIVE_HEAP_SIZE, mem2);
    reportHeapStep(0x2db);
    mHeap::createCommandHeap(COMMAND_HEAP_SIZE, mem2);
    reportHeapStep(0x2de);
    mHeap::createDylinkHeap(DYLINK_HEAP_SIZE, mem1);
    reportHeapStep(0x2e1);
    dHeap::createMenuDylHeap(mem1);
    reportHeapStep(0x2e4);
    EGG::Decomp::initWorkBuffer(mem2);
    reportHeapStep(0x2e7);
    dHeap::createHmnAnmHeap(mem2);
    reportHeapStep(0x2eb);
    dHeap::createHmnHeadHeap(mem2);
    reportHeapStep(0x2ee);
    dHeap::createHmnPaletteHeap(mem2);
    reportHeapStep(0x2f1);
    dHeap::createHmnFaceTexHeap(mem2);
    reportHeapStep(0x2f4);
    dHeap::createHmnBodyHeap(mem2);
    reportHeapStep(0x2f7);
    dHeap::createHmnFaceAnmHeap(mem2);
    reportHeapStep(0x2fa);
    dHmnClothMng_c::create(mem2);
    reportHeapStep(0x2fd);
    dHmnHeadItemMng_c::create(mem2);
    reportHeapStep(0x300);
    dHmnToolMng_c::create(mem2);
    reportHeapStep(0x303);
    mHeap::setCurrentHeap(mHeap::g_gameHeap);
    dHeap::createMsgHeap(mem2);
    reportHeapStep(0x30a);
    dHeap::createScriptHeap(mem2);
    reportHeapStep(0x30d);
    dHeap::createFontHeap(mem2);
    reportHeapStep(0x310);
    dHeap::createNgwordHeap(mem2);
    reportHeapStep(0x313);
    dHeap::createPrcWorkHeap(mem2);
    reportHeapStep(0x316);
    dHeap::createPrcHeap(mem2);
    reportHeapStep(0x319);
    dHeap::createSaveHeap(mem2);
    reportHeapStep(0x31c);
    dHeap::createBannerHeap(mem2);
    reportHeapStep(0x31f);
    dHeap::createBgAlwaysHeap(mem2);
    reportHeapStep(0x323);
    dHeap::createItemInfoHeap(mem2);
    reportHeapStep(0x326);
    dHeap::createFtrAlwaysHeap(mem1);
    reportHeapStep(0x329);
    dHeap::createFgDataHeap(mem2);
    reportHeapStep(0x32c);
    dHeap::createStrResHeap(mem2);
    reportHeapStep(0x332);
    dHeap::createDsnPlttHeap(mem1);
    reportHeapStep(0x335);
    dHeap::createFieldHeap(mem2);
    reportHeapStep(0x338);
    dHeap::createStrColHeap(mem2);
    reportHeapStep(0x33b);
    dHeap::createFgHeap(mem2);
    reportHeapStep(0x33e);
    dHeap::createNpcModelHeap(mem1);
    reportHeapStep(0x341);
    dHeap::createNpcNmlPackHeap(mem2);
    reportHeapStep(0x347);
    dHeap::createFgAlwaysHeap(mem2);
    reportHeapStep(0x34a);
    dHeap::createUkiHeap(mem2);
    reportHeapStep(0x34d);
    dHeap::createEnvLightHeap(mem2);
    reportHeapStep(0x350);
    dHeap::createNandHeap(mem1);
    reportHeapStep(0x353);
    dHeap::createNetSoHeap(mem2);
    reportHeapStep(0x358);
    dHeap::createNetDwcHeap(mem1);
    reportHeapStep(0x35a);
    dHeap::createNetWc24Heap(mem2);
    reportHeapStep(0x35c);
    dHeap::createSoundHeap(mem2);
    reportHeapStep(0x35f);
    dHeap::createNetGameHeap(mem1);
    reportHeapStep(0x362);
    dHeap::createPhotoHeap(mem1);
    reportHeapStep(0x365);
    dHeap::createFaceResHeap(mem2);
    reportHeapStep(0x36d);
    dHeap::createHbmHeap(mem2);
    reportHeapStep(0x370);
    dHeap::createBlackWaitHeap(mem2);
    reportHeapStep(0x373);
    dHeap::createDvdErrHeap(mem2);
    reportHeapStep(0x376);
    dHeap::createRumbleHeap(mem2);
    reportHeapStep(0x379);
    dHeap::createPlayerHeap(mem2);
    reportHeapStep(0x37c);
    dHeap::createHmnToolHeap(mem2);
    reportHeapStep(0x37f);
    dHeap::createEffectResHeap(mem2);
    reportHeapStep(0x382);
    dHeap::createEffectMngHeap(mem2);
    reportHeapStep(0x385);
    dHeap::createEffectCallBackHeap(mem2);
    reportHeapStep(0x388);
    dHeap::createGrowUpHeap(mem2);
    reportHeapStep(0x38c);
    dHeap::createNpcPermitMngHeap(mem2);
    mAllocator_c::init(mHeap::g_gameHeap);
    dFader::create();
    dFader::setFader(dFader::TYPE_BLACK);
    createShutterFader();
    dShutdownFader::create();
    OSCalendarTime time;
    OSTicksToCalendarTime(OSGetTime(), &time);
    OSGetTime();
    u32 subMinuteSeed = time.msec + time.sec;
    u32 subHourSeed = subMinuteSeed + time.min * TIME_SECONDS_PER_MINUTE;
    u32 subDaySeed = subHourSeed + time.hour * TIME_SECONDS_PER_HOUR;
    u32 seed = subDaySeed + time.yday * TIME_SECONDS_PER_DAY;
    cM::initRnd(seed);
    dScene_c::createPerf(debug);
    dRumble::create(1);
    dSysGfx::create(mem2);
    dKbd::initHKB();
    reportHeapStep(0x3BE);
    dCaution::g_caution.init();
    g_wifiErr.clear();
    cCounter_c::clear();
    fn_8016CEF8(debug);
    reportHeapStep(0x3E9);
    mPerf::setThreadColor(OSGetCurrentThread(), dPerf::getThreadColor(6));
    archive = mHeap::g_archiveHeap;
    command = mHeap::g_commandHeap;
    const GXColor *dvdColor = dPerf::getThreadColor(5);
    mDvd::create(15, command, archive, dvdColor);
    dPhoto::create(dHeap::photoHeap_p);
    reportHeapStep(0x3FD);
    dNetMgr_c::initDwc();
    dNet::createNet(dHeap::netGameHeap_p);
    fn_8000E06C();
    dReset::Manage_c::CreateInstance(mem2);
    dBlackWait::BlackWait_c::create(dHeap::blackWaitHeap_p);
    dHomeButton_c::create(dHeap::hbmHeap_p);
    dNpc::createPermitManagers(dHeap::npcPermitMngHeap_p);
    dScene_c::initNextScene();
    setFrameRate(1);
    setClearColor(nw4r::ut::Color(0x0000FF00UL));
}

} // namespace dSys

// 8016C084
template <> void EGG::TSystem<>::Configuration::initialize() {
    DVDInit();
    SCInit();

    initMemory();

    GraphicsFifo::create(mGraphicsFifoSize, nullptr);
    mHeap::createAssertHeap(mRootHeapMem1);

    mpVideo = new EGG::Video(mRenderMode, nullptr);
    mpXfbMgr = new EGG::XfbManager();
    for (int i = 0; i < 2; i++) {
        mpXfbMgr->attach(new Xfb(mRootHeapMem2));
    }

    mpDisplay = new EGG::AsyncDisplay(1);
    mpDisplay->resetBit(1);

    mpDisplay->setClearColor(nw4r::ut::Color::BLACK);
    mpDisplay->clearEFB();

    GfxEngine::Configuration cfg;
    u16 a[] = {cfg.m_14, cfg.m_16};
    u16 b[] = {cfg.m_02, cfg.m_02};
    StateGX::initialize(cfg.m_00, cfg.m_02, cfg.mColor, cfg.mPixelFmt);
    Screen::Initialize(a, b, nullptr);
    Screen::SetTVModeDefault();

    EGG::DrawGX::Initialize(NULL);

    Thread::initialize();
    mThread = new Thread(OSGetCurrentThread(), 4);

    DvdFile::initialize();

    CoreControllerMgr::createInstance();
    KPADSetFSStickClamp(30, 71); // Default controller pointer parameters.
    dScreenshot::init();
    mRootHeapMem1->becomeCurrentHeap();
}

namespace dSys {
// 8016C26C
sPhase_c::METHOD_RESULT_e initDyl(void *) {
    dDyl::initAsync();
    return sPhase_c::OK;
}
// 8016C290
sPhase_c::METHOD_RESULT_e waitDyl(void *) {
    return (sPhase_c::METHOD_RESULT_e)(dDyl::initAsyncIsDone() != 0);
}
// 8016C2BC
sPhase_c::METHOD_RESULT_e linkRoot(void *) {
    return (sPhase_c::METHOD_RESULT_e)(
        dDyl::linkASync((ProfileName)PROFILE_ROOT) == fBase_c::SUCCEEDED);
}
// 8016C2EC
sPhase_c::METHOD_RESULT_e linkBootScene(void *) {
    return (sPhase_c::METHOD_RESULT_e)(
        dDyl::linkASync((ProfileName)SCENE_PROF_BOOT) == fBase_c::SUCCEEDED);
}
// 8016C31C
sPhase_c::METHOD_RESULT_e linkErrorScene(void *) {
    return (sPhase_c::METHOD_RESULT_e)(
        dDyl::linkASync((ProfileName)SCENE_PROF_ERROR) == fBase_c::SUCCEEDED);
}
// 8016C34C
sPhase_c::METHOD_RESULT_e initLoader(void *) {
    dBase_c::initLoader();
    return sPhase_c::OK;
}
// 8016C370
sPhase_c::METHOD_RESULT_e initSound(void *) {
    startSoundInit();
    return sPhase_c::OK;
}
// 8016C394
sPhase_c::METHOD_RESULT_e waitSound(void *) {
    return (sPhase_c::METHOD_RESULT_e)(isSoundInitDone() != 0);
}

// 8016C3C0
void runFrame() {
    EGG::Heap *mem1 = sRootHeapMem1;
    EGG::Heap *mem2 = sRootHeapMem2;
    BOOL caution = dCaution::g_caution.check();
    BOOL wifiError = g_wifiErr.check();
    static_cast<EGG::ExpHeap *>(mem1)->checkHeap(1);
    static_cast<EGG::ExpHeap *>(mem2)->checkHeap(1);
    dHomeButton_c::getInstance()->execute();
    dReset::Manage_c::GetInstance()->Calculate();
    dResetSave::execute();
    m3d::calcWorld();
    beginFrame();
    m3d::calcMaterial();
    mFace::execute();
    dPad::begin();
    m3d::reset();
    beginRender();
    dSysGfx::draw();
    dPhoto::capture();
    drawShutterFader();
    mFader_c::draw();
    dShutdownFader::draw();
    m3d::clear();
    dHomeButton_c::getInstance()->draw();
    if (wifiError) {
        g_wifiErr.draw();
    }
    dReset::Manage_c::GetInstance()->Draw();
    if (caution) {
        dCaution::g_caution.draw();
    }
    if (!caution && !wifiError) {
        dBlackWait::BlackWait_c::getInstance()->draw();
    }
    dHomeButton_c::getInstance()->drawIcon();
    endRender();
    dScene_c::endPerf();
    dScene_c::beginPerf();
    dScene_c::checkPerf();
    dNet::receiveNet();
    if (!caution && !wifiError) {
        dPhoto::execute();
    }
    if (sStartupPhase.callMethod(NULL) == sPhase_c::DONE) {
        if (!caution && !wifiError) {
            if (dHomeButton_c::getInstance()->mState ==
                dHomeButton_c::STATE_OPEN) {
                if (!dMain::isStopped(dMain::STOP_REASON_HOME_MENU)) {
                    dMain::addStop(dMain::STOP_REASON_HOME_MENU);
                }
            } else {
                if (dMain::isStopped(dMain::STOP_REASON_HOME_MENU)) {
                    dMain::removeStop(dMain::STOP_REASON_HOME_MENU);
                }
            }
        }
        if ((fManager_c::m_StopProcInf & fManager_c::PROC_FLAG_CREATE) !=
            fManager_c::PROC_FLAG_CREATE) {
            dScene_c::createNextScene();
        }
        fManager_c::mainLoop();
    }
    if (!caution && !wifiError) {
        if (dHomeButton_c::getInstance()->mState != dHomeButton_c::STATE_OPEN) {
            calcShutterFader();
        }
        dShutdownFader::calc();
        mFader_c::mFader->calc();
        dBlackWait::BlackWait_c::getInstance()->execute();
    }
    if (caution) {
        dCaution::g_caution.execute();
    } else if (wifiError) {
        g_wifiErr.execute();
    }
    dNet::updateNet();
    dMiiIcon::execute();
    executeSound();
    dPad::end();
    endFrame();
    static_cast<EGG::ExpHeap *>(mem1)->checkHeap(1);
    static_cast<EGG::ExpHeap *>(mem2)->checkHeap(1);
    cCounter_c::m_gameFrame++;
}

static sPhase_c::phaseMethod sStartupMethods[] = {
    initDyl,        waitDyl,    linkRoot,  linkBootScene,
    linkErrorScene, initLoader, initSound, waitSound};
sPhase_c sStartupPhase(sStartupMethods, STARTUP_METHOD_NUM);
} // namespace dSys
