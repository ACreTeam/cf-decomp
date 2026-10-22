// REL .text 0xCC..0x19588, .ctors 0x0..0x4, .rodata 0x0..0xABC, .data 0x0..0x6728, .bss 0x8..0x108.
#include <game/game/d_m3d.hpp>
#include <game/game/d_a_ftr.hpp>
#include <game/game/d_a_ftr_act.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/framework/f_profile.hpp>
#include <game/snd/snd_id.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <revolution/mtx.h>
#include <string.h>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_demo.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_model_room.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/game/d_pattern_copy.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_select.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_a_player.hpp>

// Unused; referencing anmChrPart_c's vtable queues anmChr_c::getType before anmVis_c::getType among the weak
// functions. The original's equivalent is unknown; d_ftr_manager, with the same headers, must not have it.
inline void makePartAnm() {
    m3d::anmChrPart_c anm;
}

static const u32 l_homeScenes[] = {
    dScene::SCENE_ATTR_HOUSE0 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE1 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE2 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    dScene::SCENE_ATTR_HOUSE3 | dScene::SCENE_ATTR_PLAYER_HOUSE,
    0,
};

// 0xCC
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_LP, daFtrLp_c, fProf::FTR_LP, fProf::DRAW_ORDER::FTR_LP);
} // namespace dAFtrAct

// 0x478
BOOL daFtrLp_c::initModel() {
    if (getFtrFunc() != dItem::FTR_FUNC_PLAIN) {
        isAnmNum(1, ANM_ALL);
        u32 frameMax = getAnmFrameMax(0, ANM_ALL);
        u16 frame = getKind() == FTR_KIND_APPEAR ? 0 : (u32)cM::rndF(frameMax);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, frame, ANM_ALL);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else if (getSwitch()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x5C0
BOOL daFtrLp_c::executeMain() {
    playAnm(-1.0f, ANM_ALL);
    executeSwitch();
    updateSound();
    return TRUE;
}

// 0x60C
BOOL daFtrLp_c::deleteRes() {
    return TRUE;
}

// 0x614
int daFtrLp_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x634
BOOL daFtrLp_c::setSwitch(int sw, int param) {
    static BOOL (daFtrLp_c::*const init[])() = {&daFtrLp_c::initOff, &daFtrLp_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x6A8
void daFtrLp_c::executeSwitch() {
    static void (daFtrLp_c::*const execute[])() = {&daFtrLp_c::executeOff, &daFtrLp_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x6E8
BOOL daFtrLp_c::initOff() {
    changeSwitch(0, FALSE);
    turnOffSound();
    return TRUE;
}

// 0x734
void daFtrLp_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, (mSwitchState + 1) & 1, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x788
BOOL daFtrLp_c::initOn() {
    changeSwitch(1, FALSE);
    turnOnSound();
    return TRUE;
}

// 0x7D4
void daFtrLp_c::executeOn() {
    executeOff();
}

// 0x7D8
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_TRG, daFtrTrg_c, fProf::FTR_TRG, fProf::DRAW_ORDER::FTR_TRG);
} // namespace dAFtrAct

// 0xB8C
BOOL daFtrTrg_c::initModel() {
    isAnmNum(1, ANM_ALL);
    changeSwitch(0, FALSE);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    return TRUE;
}

// 0xC3C
BOOL daFtrTrg_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0xC60
BOOL daFtrTrg_c::deleteRes() {
    changeSwitch(0, FALSE);
    return TRUE;
}

// 0xC98
BOOL daFtrTrg_c::setSwitch(int sw, int param) {
    static BOOL (daFtrTrg_c::*const init[])() = {&daFtrTrg_c::initOff, &daFtrTrg_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xD0C
void daFtrTrg_c::executeSwitch() {
    static void (daFtrTrg_c::*const execute[])() = {&daFtrTrg_c::executeOff, &daFtrTrg_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xD4C
BOOL daFtrTrg_c::initOff() {
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    turnOffSound();
    if (getFtrFunc() == dItem::FTR_FUNC_PUNCHING_BAG) {
        mTimer.set(AUTO_TRIGGER_WAIT_BAG);
    } else {
        mTimer.set(AUTO_TRIGGER_WAIT);
    }
    return TRUE;
}

// 0xDD0
void daFtrTrg_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0xE68
BOOL daFtrTrg_c::initOn() {
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    mTimer.set(TRIGGER_TIME);
    return TRUE;
}

// 0xEC4
void daFtrTrg_c::executeOn() {
    if (getFtrFunc() != dItem::FTR_FUNC_PUNCHING_BAG) {
        toggleSwitch(FALSE);
    }
    if (playAnm(-1.0f, ANM_ALL)) {
        if (isSyncDisabled()) {
            setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
        } else {
            dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    } else {
        updateSound();
    }
    if (getFtrFunc() == dItem::FTR_FUNC_PUNCHING_BAG && isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0xFD0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_SW, daFtrSw_c, fProf::FTR_SW, fProf::DRAW_ORDER::FTR_SW);
} // namespace dAFtrAct

// 0x13A0
BOOL daFtrSw_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    if (!getSwitch() || dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP)) {
        mLamps.init(getResMdl(), FALSE);
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else {
        mLamps.init(getResMdl(), TRUE);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x14B0
BOOL daFtrSw_c::executeMain() {
    mLamps.update();
    executeSwitch();
    return TRUE;
}

// 0x14EC
BOOL daFtrSw_c::deleteRes() {
    return TRUE;
}

// 0x14F4
int daFtrSw_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x1514
BOOL daFtrSw_c::setSwitch(int sw, int param) {
    static BOOL (daFtrSw_c::*const init[])() = {&daFtrSw_c::initOff, &daFtrSw_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x1588
void daFtrSw_c::executeSwitch() {
    static void (daFtrSw_c::*const execute[])() = {&daFtrSw_c::executeOff, &daFtrSw_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x15C8
BOOL daFtrSw_c::initOff() {
    changeSwitch(0, FALSE);
    turnOffSound();
    mLamps.set(FALSE, TRUE, FALSE);
    return TRUE;
}

// 0x1628
void daFtrSw_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x1674
BOOL daFtrSw_c::initOn() {
    changeSwitch(1, FALSE);
    turnOnSound();
    mLamps.set(TRUE, TRUE, FALSE);
    return TRUE;
}

// 0x16D4
void daFtrSw_c::executeOn() {
    playAnm(-1.0f, ANM_ALL);
    updateSound();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x173C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_SW2, daFtrSw2_c, fProf::FTR_SW2, fProf::DRAW_ORDER::FTR_SW2);
} // namespace dAFtrAct

// 0x1B0C
BOOL daFtrSw2_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP)) {
        mLamps.init(getResMdl(), TRUE);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        mLamps.init(getResMdl(), FALSE);
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x1C1C
BOOL daFtrSw2_c::executeMain() {
    mLamps.update();
    executeSwitch();
    if (mAnmChr.IsBound()) {
        mAnmChr.setRate(mAnmRate);
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.setRate(mAnmRate, 0);
    }
    if (mAnmTexPat.IsBound()) {
        mAnmTexPat.setRate(mAnmRate, 0);
    }
    if (mAnmTexSrt.IsBound()) {
        mAnmTexSrt.setRate(mAnmRate, 0);
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.setRate(mAnmRate);
    }
    if (mAnmRate != 0.0f) {
        playAnm(-1.0f, ANM_ALL);
    }
    return TRUE;
}

// 0x1D18
BOOL daFtrSw2_c::deleteRes() {
    return TRUE;
}

// 0x1D20
int daFtrSw2_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x1D40
BOOL daFtrSw2_c::setSwitch(int sw, int param) {
    static BOOL (daFtrSw2_c::*const init[])() = {&daFtrSw2_c::initOff, &daFtrSw2_c::initOn, &daFtrSw2_c::initStop};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x1DB4
void daFtrSw2_c::executeSwitch() {
    static void (daFtrSw2_c::*const execute[])() = {&daFtrSw2_c::executeOff, &daFtrSw2_c::executeOn,
                                                   &daFtrSw2_c::executeStop};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x1DF4
BOOL daFtrSw2_c::initOff() {
    mAnmRate = 0.0f;
    changeSwitch(0, FALSE);
    mLamps.set(FALSE, TRUE, FALSE);
    return TRUE;
}

// 0x1E58
void daFtrSw2_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x1EA4
BOOL daFtrSw2_c::initOn() {
    mAnmRate = 1.0f;
    changeSwitch(1, FALSE);
    turnOnSound();
    mLamps.set(TRUE, TRUE, FALSE);
    return TRUE;
}

// 0x1F10
void daFtrSw2_c::executeOn() {
    updateSound();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_STOP, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x1F64
BOOL daFtrSw2_c::initStop() {
    changeSwitch(0, FALSE);
    turnOffSound();
    mLamps.set(TRUE, TRUE, FALSE);
    return TRUE;
}

// 0x1FC4
void daFtrSw2_c::executeStop() {
    sLib::chase(&mAnmRate, 0.0f, 1.0f / 60.0f);
    if (mAnmRate < 0.01f) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x2038
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_FLP, daFtrFlp_c, fProf::FTR_FLP, fProf::DRAW_ORDER::FTR_FLP);
} // namespace dAFtrAct

// 0x23EC
BOOL daFtrFlp_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x24AC
BOOL daFtrFlp_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0x24D0
BOOL daFtrFlp_c::deleteRes() {
    return TRUE;
}

// 0x24D8
int daFtrFlp_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x24F8
BOOL daFtrFlp_c::setSwitch(int sw, int param) {
    static BOOL (daFtrFlp_c::*const init[])() = {&daFtrFlp_c::initOff, &daFtrFlp_c::initTurnOn, &daFtrFlp_c::initOn,
                                                &daFtrFlp_c::initTurnOff};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x256C
void daFtrFlp_c::executeSwitch() {
    static void (daFtrFlp_c::*const execute[])() = {&daFtrFlp_c::executeOff, &daFtrFlp_c::executeTurnOn,
                                                   &daFtrFlp_c::executeOn, &daFtrFlp_c::executeTurnOff};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x25AC
BOOL daFtrFlp_c::initOff() {
    changeSwitch(0, FALSE);
    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    mTimer.set(AUTO_ON_WAIT);
    return TRUE;
}

// 0x261C
void daFtrFlp_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_ON, SWITCH_PARAM_NONE);
    }
}

// 0x26B4
BOOL daFtrFlp_c::initTurnOn() {
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0x2720
void daFtrFlp_c::executeTurnOn() {
    changeSwitch(1, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0x2798
BOOL daFtrFlp_c::initOn() {
    changeSwitch(1, FALSE);
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    mTimer.set(AUTO_OFF_WAIT);
    return TRUE;
}

// 0x2804
void daFtrFlp_c::executeOn() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x289C
BOOL daFtrFlp_c::initTurnOff() {
    changeSwitch(0, FALSE);
    setAnmEnd(1.0f, 0, m3d::REVERSE_ONCE, ANM_ALL);
    turnOffSound();
    return TRUE;
}

// 0x2904
void daFtrFlp_c::executeTurnOff() {
    changeSwitch(0, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x297C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR2_W, daFtr2W_c, fProf::FTR2_W, fProf::DRAW_ORDER::FTR2_W);
} // namespace dAFtrAct

// 0x2D30
BOOL daFtr2W_c::initModel() {
    isAnmNum(2, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    if (!getSwitch() || dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x2DF0
BOOL daFtr2W_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0x2E14
BOOL daFtr2W_c::deleteRes() {
    return TRUE;
}

// 0x2E1C
int daFtr2W_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x2E3C
BOOL daFtr2W_c::setSwitch(int sw, int param) {
    static BOOL (daFtr2W_c::*const init[])() = {&daFtr2W_c::initOff, &daFtr2W_c::initTurnOn, &daFtr2W_c::initOn,
                                               &daFtr2W_c::initTurnOff};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x2EB0
void daFtr2W_c::executeSwitch() {
    static void (daFtr2W_c::*const execute[])() = {&daFtr2W_c::executeOff, &daFtr2W_c::executeTurnOn,
                                                  &daFtr2W_c::executeOn, &daFtr2W_c::executeTurnOff};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x2EF0
BOOL daFtr2W_c::initOff() {
    changeSwitch(0, FALSE);
    setAnmEnd(1.0f, 1, m3d::FORWARD_ONCE, ANM_ALL);
    mTimer.set(AUTO_SWITCH_WAIT);
    return TRUE;
}

// 0x2F5C
void daFtr2W_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_ON, SWITCH_PARAM_NONE);
    }
}

// 0x2FF4
BOOL daFtr2W_c::initTurnOn() {
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0x3060
void daFtr2W_c::executeTurnOn() {
    changeSwitch(1, FALSE);
    updateSound();
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0x30E0
BOOL daFtr2W_c::initOn() {
    changeSwitch(1, FALSE);
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    mTimer.set(AUTO_SWITCH_WAIT);
    return TRUE;
}

// 0x314C
void daFtr2W_c::executeOn() {
    updateSound();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x31EC
BOOL daFtr2W_c::initTurnOff() {
    changeSwitch(0, FALSE);
    changeAnm(1.0f, 1, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOffSound();
    return TRUE;
}

// 0x3258
void daFtr2W_c::executeTurnOff() {
    updateSound();
    changeSwitch(0, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x32D8
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_FSW, daFtrFsw_c, fProf::FTR_FSW, fProf::DRAW_ORDER::FTR_FSW);
} // namespace dAFtrAct

// 0x368C
BOOL daFtrFsw_c::initModel() {
    isAnmNum(2, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP)) {
        turnOnSound();
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x3754
BOOL daFtrFsw_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0x3778
BOOL daFtrFsw_c::deleteRes() {
    return TRUE;
}

// 0x3780
int daFtrFsw_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x37A0
BOOL daFtrFsw_c::setSwitch(int sw, int param) {
    static BOOL (daFtrFsw_c::*const init[])() = {&daFtrFsw_c::initOff, &daFtrFsw_c::initTurnOn, &daFtrFsw_c::initOn,
                                                &daFtrFsw_c::initTurnOff};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x3814
void daFtrFsw_c::executeSwitch() {
    static void (daFtrFsw_c::*const execute[])() = {&daFtrFsw_c::executeOff, &daFtrFsw_c::executeTurnOn,
                                                   &daFtrFsw_c::executeOn, &daFtrFsw_c::executeTurnOff};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x3854
BOOL daFtrFsw_c::initOff() {
    changeSwitch(0, FALSE);
    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    mTimer.set(AUTO_SWITCH_WAIT);
    turnOffSound();
    return TRUE;
}

// 0x38CC
void daFtrFsw_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_ON, SWITCH_PARAM_NONE);
    }
}

// 0x3964
BOOL daFtrFsw_c::initTurnOn() {
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0x39D0
void daFtrFsw_c::executeTurnOn() {
    changeSwitch(1, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0x3A48
BOOL daFtrFsw_c::initOn() {
    changeSwitch(1, FALSE);
    setAnmEnd(1.0f, 1, m3d::FORWARD_LOOP, ANM_ALL);
    mTimer.set(AUTO_SWITCH_WAIT);
    return TRUE;
}

// 0x3AB4
void daFtrFsw_c::executeOn() {
    playAnm(-1.0f, ANM_ALL);
    updateSound();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_TURN_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_TURN_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x3B68
BOOL daFtrFsw_c::initTurnOff() {
    changeSwitch(0, FALSE);
    setAnmEnd(1.0f, 0, m3d::REVERSE_ONCE, ANM_ALL);
    return TRUE;
}

// 0x3BC8
void daFtrFsw_c::executeTurnOff() {
    changeSwitch(0, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x3C40
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_FAW, daFtrFaw_c, fProf::FTR_FAW, fProf::DRAW_ORDER::FTR_FAW);
} // namespace dAFtrAct

// 0x4020
BOOL daFtrFaw_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    if (!getSwitch() || dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_SHOP) ||
        isSyncDisabled()) {
        mLamps.init(getResMdl(), FALSE);
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else {
        mLamps.init(getResMdl(), TRUE);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x414C
BOOL daFtrFaw_c::executeMain() {
    if (mMdl.getScn() != NULL) {
        mLamps.update();
        executeSwitch();
        playAnm(mLamps.getValue(), ANM_ALL);
        updateSound();
    }
    return TRUE;
}

// 0x41B0
BOOL daFtrFaw_c::executeIdle() {
    executeMain();
    return TRUE;
}

// 0x41E0
BOOL daFtrFaw_c::deleteRes() {
    return TRUE;
}

// 0x41E8
int daFtrFaw_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x4208
BOOL daFtrFaw_c::setSwitch(int sw, int param) {
    static BOOL (daFtrFaw_c::*const init[])() = {&daFtrFaw_c::initOff, &daFtrFaw_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x427C
void daFtrFaw_c::executeSwitch() {
    static void (daFtrFaw_c::*const execute[])() = {&daFtrFaw_c::executeOff, &daFtrFaw_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x42BC
BOOL daFtrFaw_c::initOff() {
    changeSwitch(0, FALSE);
    turnOffSound();
    mLamps.set(FALSE, TRUE, FALSE);
    if (getFtrFunc() == dItem::FTR_FUNC_FIRE_WATER) {
        mTimer.set(AUTO_ON_WAIT);
    } else {
        mTimer.set(AUTO_ON_WAIT_TOILET);
    }
    return TRUE;
}

// 0x4354
void daFtrFaw_c::executeOff() {
    if (getFtrFunc() == dItem::FTR_FUNC_FIRE_WATER) {
        if (isSwitchChanging()) {
            dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
        } else if (isSyncDisabled() && mTimer.calc()) {
            setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
        }
    } else {
        if (isSwitchChanging()) {
            dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
        } else if (isSyncDisabled() && mTimer.calc()) {
            setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
        }
    }
}

// 0x4480
BOOL daFtrFaw_c::initOn() {
    changeSwitch(1, FALSE);
    turnOnSound();
    mLamps.set(TRUE, TRUE, FALSE);
    mOnTimer.set(TOILET_ON_TIME);
    mTimer.set(AUTO_OFF_WAIT);
    return TRUE;
}

// 0x44F8
void daFtrFaw_c::executeOn() {
    if (getFtrFunc() == dItem::FTR_FUNC_FIRE_WATER) {
        if (isSwitchChanging()) {
            dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
        } else if (isSyncDisabled() && mTimer.calc()) {
            setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
        }
    } else if (mOnTimer.calc()) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x45DC
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CHAIR, daFtrChair_c, fProf::FTR_CHAIR, fProf::DRAW_ORDER::FTR_CHAIR);
} // namespace dAFtrAct

// 0x4988
BOOL daFtrChair_c::initModel() {
    mSwitchNode.set(FALSE);
    switch (getFtrFunc()) {
    case dItem::FTR_FUNC_SOFA_ANIM:
    case dItem::FTR_FUNC_SOFA_SEAT_ANIM:
    case dItem::FTR_FUNC_CHAIR_ANIM:
    case dItem::FTR_FUNC_CHAIR_SEAT_ANIM:
        isAnmNum(1, ANM_ALL);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
        break;
    case dItem::FTR_FUNC_RIGHTY_DESK_ANIM:
    case dItem::FTR_FUNC_RIGHTY_DESK_SEAT_ANIM:
    case dItem::FTR_FUNC_LEFTY_DESK_ANIM:
    case dItem::FTR_FUNC_LEFTY_DESK_SEAT_ANIM:
        isAnmNum(1, ANM_ALL);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
        break;
    }
    return TRUE;
}

// 0x4A7C
BOOL daFtrChair_c::executeMain() {
    int func = getFtrFunc();
    if (isSyncDisabled()) {
        mSwitchNode.set(TRUE);
        playAnm(-1.0f, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_SOFA_SEAT_ANIM || func == dItem::FTR_FUNC_RIGHTY_DESK_SEAT_ANIM || func == dItem::FTR_FUNC_LEFTY_DESK_SEAT_ANIM || func == dItem::FTR_FUNC_CHAIR_SEAT_ANIM) {
        mSwitchNode.set(FALSE);
        for (u32 i = 0; i < dPlayerMgr::PLAYER_INDEX_COUNT; i++) {
            mVec3_c pos;
            if (dPlayerMgr::getChairPosition(&pos, i)) {
                int x = worldToUnit(pos.x);
                int z = worldToUnit(pos.z);
                dFTR::unitList_c units;
                getUnits(&units, NULL);
                for (u32 j = 0; j < units.mNum; j++) {
                    if (x == units.get(j)->x && z == units.get(j)->z) {
                        mSwitchNode.set(TRUE);
                        playAnm(-1.0f, ANM_ALL);
                        updateSound();
                        break;
                    }
                }
            }
        }
    } else if (func == dItem::FTR_FUNC_SOFA_ANIM || func == dItem::FTR_FUNC_RIGHTY_DESK_ANIM || func == dItem::FTR_FUNC_LEFTY_DESK_ANIM || func == dItem::FTR_FUNC_CHAIR_ANIM) {
        mSwitchNode.set(TRUE);
        playAnm(-1.0f, ANM_ALL);
        updateSound();
    }
    executeSit();
    return TRUE;
}

// 0x4C68
void daFtrChair_c::executeSit() {
    // Indexed by dPlayerMgr::SIT_FROM_*.
    static const u8 sides[] = {dFTR::SIDE_FRONT, dFTR::SIDE_RIGHT, dFTR::SIDE_LEFT};
    BOOL waiting = FALSE;
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    if (hold != NULL && dFTR::canHoldInScene()) {
        int type = -1;
        if (isRightyDesk()) {
            if (hold->mSide == dFTR::SIDE_LEFT) {
                type = dPlayerMgr::SIT_FROM_LEFT;
            }
        } else if (isLeftyDesk()) {
            if (hold->mSide == dFTR::SIDE_RIGHT) {
                type = dPlayerMgr::SIT_FROM_RIGHT;
            }
        } else if (!isChair()) {
            if (hold->mSide == dFTR::SIDE_FRONT) {
                type = dPlayerMgr::SIT_FROM_FRONT;
            }
        } else {
            for (int i = 0; i < ARRAY_SIZE(sides); i++) {
                if (sides[i] == hold->mSide) {
                    type = i;
                    break;
                }
            }
        }
        if (type != -1) {
            mVec3_c pos;
            getSeatPos(&pos, hold);
            if (hold->mPlayerLastPos.xzDistTo(hold->mPlayerPos) > 0.32f) {
                if (isFree(&pos, TRUE, sFreeRadius, sFreeRadius, sFreeRadius, 0.0f)) {
                    if (mSitTimer < dFtrm_c::getHoldWait()) {
                        mSitTimer++;
                        waiting = TRUE;
                    }
                    if (mSitTimer >= dFtrm_c::getHoldWait()) {
                        sit(hold, type);
                        mSitTimer = 0;
                    }
                }
            } else if (!dScene::isCurrentSceneModelRoom() &&
                       isFree(&pos, TRUE, sFreeRadius, sFreeRadius, sFreeRadius, 0.0f) && isSwitchChanging()) {
                sit(hold, type);
            }
        }
    }
    if (!waiting) {
        mSitTimer = 0;
    }
}

// 0x4EB4
BOOL daFtrChair_c::deleteRes() {
    return TRUE;
}

// 0x4EBC
void daFtrChair_c::getSeatPos(mVec3_c *out, const dFTR::hold_c *hold) {
    if (getFtrSize() == dItem::FTR_SIZE_1x1) {
        *out = mPos;
    } else {
        *out = hold->mPushPos;
        f32 dist = hold->mDist;
        dist = 16.0f + dist;
        mVec3_c ofs(0.0f, 0.0f, dist);
        ofs.rotY(mAng(hold->mPushAngle));
        *out += ofs;
    }
}

// 0x4FA4
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_BED, daFtrBed_c, fProf::FTR_BED, fProf::DRAW_ORDER::FTR_BED);
} // namespace dAFtrAct

// 0x5358
BOOL daFtrBed_c::initModel() {
    mSwitchNode.set(FALSE);
    if (daFTR_c::getFtrFunc() == dItem::FTR_FUNC_BED_ANIM || daFTR_c::getFtrFunc() == dItem::FTR_FUNC_BED_SEAT_ANIM) {
        isAnmNum(1, ANM_ALL);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    }
    int room;
    if (dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene()) != -1 && room == HOME_SCENE_LOFT) {
        setSwitch(BED_LOFT, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x5424
BOOL daFtrBed_c::canWalk(const unit_c &unit) {
    return dFTR::canNpcWalk(unit.x, unit.z);
}

// 0x5434
BOOL daFtrBed_c::executeMain() {
    int func = daFTR_c::getFtrFunc();
    if (isSyncDisabled()) {
        mSwitchNode.set(TRUE);
        playAnm(-1.0f, ANM_ALL);
    } else {
        switch (func) {
        case dItem::FTR_FUNC_BED_SEAT_ANIM: {
            mSwitchNode.set(FALSE);
            u8 wasOn = isPlayerOn();
            mPlayerOn = FALSE;
            for (u32 i = 0; i < dPlayerMgr::PLAYER_INDEX_COUNT; i++) {
                mVec3_c pos;
                if (dPlayerMgr::getBedAnimationPosition(&pos, i)) {
                    int x = worldToUnit(pos.x);
                    int z = worldToUnit(pos.z);
                    dFTR::unitList_c units;
                    getUnits(&units, NULL);
                    for (u32 j = 0; j < units.mNum; j++) {
                        if (x == units.get(j)->x && z == units.get(j)->z) {
                            mSwitchNode.set(TRUE);
                            playAnm(-1.0f, ANM_ALL);
                            updateSound();
                            mPlayerOn = TRUE;
                            break;
                        }
                    }
                }
            }
            if (wasOn && !mPlayerOn) {
                turnOffSound();
            }
            break;
        }
        case dItem::FTR_FUNC_BED_ANIM:
            mSwitchNode.set(TRUE);
            playAnm(-1.0f, ANM_ALL);
            updateSound();
            break;
        }
        executeSwitch();
        executeLie();
    }
    return TRUE;
}

// 0x5620
void daFtrBed_c::executeLie() {
    BOOL waiting = FALSE;
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!dScene::isCurrentSceneMyHouse()) {
        int house;
        if (dScene::getSceneAttrTableIndex(dScene::getCurrentScene(), l_homeScenes, &house) == HOME_SCENE_LOFT) {
            return;
        }
    }
    if (hold != NULL && player != NULL && dFTR::canHoldInScene() && (hold->mSide == dFTR::SIDE_BACK || hold->mSide == dFTR::SIDE_FRONT)) {
        if (canWalk(getNearUnit(&player->mPos))) {
            mVec3_c ofs(0.0f, 0.0f, 32.0f);
            ofs.rotY(mAng(hold->mPushAngle));
            mVec3_c pos(ofs.x + hold->mCenterPos.x, 0.0f, ofs.z + hold->mCenterPos.z);
            if (hold->mPlayerLastPos.xzDistTo(hold->mPlayerPos) > 0.32f) {
                if (isFree(&pos, TRUE, sFreeRadius, sFreeRadius, sFreeRadius, 0.0f)) {
                    if (mLieTimer < dFtrm_c::getHoldWaitNpc()) {
                        mLieTimer++;
                        waiting = TRUE;
                    }
                    if (mLieTimer >= dFtrm_c::getHoldWaitNpc()) {
                        lie(hold);
                        mLieTimer = 0;
                    }
                }
            } else if (!dScene::isCurrentSceneModelRoom() &&
                       isFree(&pos, TRUE, sFreeRadius, sFreeRadius, sFreeRadius, 0.0f) && isSwitchChanging()) {
                lie(hold);
            }
        }
    }
    if (!waiting) {
        mLieTimer = 0;
    }
}

// 0x5890
BOOL daFtrBed_c::deleteRes() {
    return TRUE;
}

// 0x5898
u32 daFtrBed_c::getHeadUnits(unitPair_c *units) {
    static mVec3_c l_ofs1x1[] = {mVec3_c(0.0f, 0.0f, 32.0f), mVec3_c(0.0f, 0.0f, -32.0f)};
    static mVec3_c l_ofs1x2[] = {mVec3_c(32.0f, 0.0f, 32.0f), mVec3_c(32.0f, 0.0f, -32.0f)};
    static mVec3_c l_ofs2x2[] = {mVec3_c(16.0f, 0.0f, 48.0f), mVec3_c(16.0f, 0.0f, -48.0f)};
    static mVec3_c *l_ofs[] = {l_ofs1x1, l_ofs1x2, l_ofs2x2};
    mVec3_c *ofs = l_ofs[getFtrSize()];
    if (ofs != NULL) {
        for (u32 i = 0; i < ARRAY_SIZE(units->mUnit); i++, ofs++) {
            mVec3_c pos;
            multMtx(&pos, ofs);
            units->get(i)->x = worldToUnit(pos.x);
            units->get(i)->z = worldToUnit(pos.z);
        }
        return ARRAY_SIZE(units->mUnit);
    }
    return 0;
}

// 0x5A9C
daFtrBed_c::unit_c daFtrBed_c::getNearUnit(const mVec3_c *pos) {
    unitPair_c units;
    getHeadUnits(&units);
    mVec3_c center0;
    dFdBase_c::getUnitCenterPos(center0, units.mUnit[0].x, units.mUnit[0].z);
    mVec3_c center1;
    dFdBase_c::getUnitCenterPos(center1, units.mUnit[1].x, units.mUnit[1].z);
    f32 dist0 = center0.xzDistTo(*pos);
    if (dist0 < center1.xzDistTo(*pos)) {
        return units.mUnit[0];
    }
    return units.mUnit[1];
}

static mVec3_c l_sideOfs1x2[] = {mVec3_c(0.0f, 0.0f, 0.0f)};
static mVec3_c l_sideOfs2x2[] = {mVec3_c(-16.0f, 0.0f, -16.0f), mVec3_c(-16.0f, 0.0f, 16.0f)};

// 0x5B6C
u32 daFtrBed_c::getSideUnits(unitPair_c *units) {
    static const struct {
        mVec3_c *mOfs;
        u32 mNum;
    } l_sideOfs[] = {
        {NULL, 0},
        {l_sideOfs1x2, ARRAY_SIZE(l_sideOfs1x2)},
        {l_sideOfs2x2, ARRAY_SIZE(l_sideOfs2x2)},
    };
    int size = mpBITM->getFtrSize();
    mVec3_c *ofs = l_sideOfs[size].mOfs;
    if (ofs != NULL) {
        for (u32 i = 0; i < l_sideOfs[size].mNum; i++, ofs++) {
            mVec3_c pos;
            multMtx(&pos, ofs);
            units->get(i)->x = worldToUnit(pos.x);
            units->get(i)->z = worldToUnit(pos.z);
        }
        return l_sideOfs[size].mNum;
    }
    return 0;
}

// 0x5C58
int daFtrBed_c::checkSeat(const mVec3_c *pos, mAng angle, const mVec3_c *ofs, mVec3_c *out) {
    mVec3_c seat;
    calcSeatPos(&seat, pos, angle, ofs);
    int x = worldToUnit(seat.x);
    int z = worldToUnit(seat.z);
    daFTR_c *ftr = dFTR::getUnitMap()->get(x, z, 0);
    if (out != NULL) {
        *out = seat;
    }
    if (!isFree(&seat, FALSE, sFreeRadius, sFreeRadius, sFreeRadius, 0.0f)) {
        return dFtrm_c::SEAT_BLOCKED;
    }
    if (ftr != NULL) {
        if (ftr == this) {
            return dFtrm_c::SEAT_BED;
        }
        if (ftr->canWalkOn()) {
            return dFtrm_c::SEAT_FLOOR;
        }
        if (mAngle.y == ftr->mAngle.y && ftr->mProfName == fProf::FTR_BED) {
            unitPair_c own;
            u32 ownNum = getSideUnits(&own);
            unitPair_c other;
            u32 otherNum = static_cast<daFtrBed_c *>(ftr)->getSideUnits(&other);
            for (u32 i = 0; i < ownNum; i++) {
                unit_c a = *own.get(i);
                for (u32 j = 0; j < otherNum; j++) {
                    unit_c b = *other.get(j);
                    if (labs(a.x - b.x) + labs(a.z - b.z) == 1) {
                        return dFtrm_c::SEAT_BED;
                    }
                }
            }
        }
    } else {
        f32 groundY = dBGCF::getGroundY(&seat, FALSE);
        if (groundY == dFTR::getGroundY(&seat) && groundY == dBGCF::getGroundY(pos, FALSE) &&
            dBGCF::getOpenNext(&seat) == -1) {
            return dFtrm_c::SEAT_FLOOR;
        }
    }
    return dFtrm_c::SEAT_BLOCKED;
}

// 0x5EA8
void daFtrBed_c::calcSeatPos(mVec3_c *out, const mVec3_c *pos, mAng angle, const mVec3_c *ofs) {
    *out = *pos;
    mVec3_c base(0.0f, 0.0f, 0.0f);
    if (ofs != NULL) {
        base = *ofs;
    }
    mVec3_c v(base.x, base.y, 16.0f + base.z);
    v.rotY(angle);
    *out += v;
}

// 0x5F84
static BOOL isBedFunc(int func) {
    return (u16)convFtrFunc(func) == fProf::FTR_BED;
}

// 0x5FB4
static BOOL isPocketDisabled(void *pocket, int flag) {
    if (flag == 0) {
        dItem::Item *item = static_cast<dItem::Item *>(pocket);
        if (dItem::isRealItemId(item->getId())) {
            const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
            if (bitm != NULL && isBedFunc(bitm->getFtrFunc())) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 0x6048
BOOL daFtrBed_c::setSwitch(int sw, int param) {
    static BOOL (daFtrBed_c::*const init[])() = {
        &daFtrBed_c::initOff,    &daFtrBed_c::initLoft,  &daFtrBed_c::initSpeak,
        &daFtrBed_c::initTalk,   &daFtrBed_c::initCancel, &daFtrBed_c::initSelect,
        &daFtrBed_c::initMenu,   &daFtrBed_c::initReplace, &daFtrBed_c::initFinish,
    };
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x60BC
void daFtrBed_c::executeSwitch() {
    static void (daFtrBed_c::*const execute[])() = {
        &daFtrBed_c::executeOff,    &daFtrBed_c::executeLoft,  &daFtrBed_c::executeSpeak,
        &daFtrBed_c::executeTalk,   &daFtrBed_c::executeCancel, &daFtrBed_c::executeSelect,
        &daFtrBed_c::executeMenu,   &daFtrBed_c::executeReplace, &daFtrBed_c::executeFinish,
    };
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x60FC
BOOL daFtrBed_c::initOff() {
    mLieTimer = 0;
    return TRUE;
}

// 0x610C
void daFtrBed_c::executeOff() {}

// 0x6110
BOOL daFtrBed_c::initLoft() {
    return TRUE;
}

// 0x6118
void daFtrBed_c::executeLoft() {
    BOOL pressed = FALSE;
    if (mPad::g_currentCore->getDpdValidFlag() > 0) {
        const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
        if (hit != NULL) {
            if (hit->mType == dObjc::HIT_FTR_ROOM && hit->mBase.get() == this) {
                if (dPlayerMgr::isATriggeredFacing(&hit->mPos, 0x2000, 64.0f)) {
                    pressed = TRUE;
                }
            } else if (hit->mType == 0x80000001 && dPad::isATriggered()) {
                pressed = TRUE;
            }
        }
        if (dPad::isConfirmTriggered() && !dPad::isATriggered()) {
            pressed = TRUE;
        }
    } else if (dPad::isConfirmTriggered()) {
        pressed = TRUE;
    }
    if (pressed && dScene::isCurrentSceneMyHouse()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL && hold->mSide == dFTR::SIDE_LEFT && dSTR::isAlone() &&
            (!dNet::isSessionActive() || dNet::getMemberNum() <= 1) && dPad::isConfirmTriggered()) {
            dDemo::requestSpeak(this, NULL);
        }
    }
}

// 0x6278
BOOL daFtrBed_c::initSpeak() {
    return TRUE;
}

// 0x6280
void daFtrBed_c::executeSpeak() {
    setSwitch(BED_TALK, SWITCH_PARAM_NONE);
}

// 0x6298
BOOL daFtrBed_c::initTalk() {
    attachDemoActor(this);
    mpController->requestStart(0);
    setMessageLabel("sys_MESS/SYS_Player");
    setMessageCode(MSG_SWAP_BED);
    return TRUE;
}

// 0x6304
void daFtrBed_c::executeTalk() {}

// 0x6308
BOOL daFtrBed_c::initCancel() {
    return TRUE;
}

// 0x6310
void daFtrBed_c::executeCancel() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// 0x6398
BOOL daFtrBed_c::initSelect() {
    return TRUE;
}

// 0x63A0
void daFtrBed_c::executeSelect() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            if (dMenuMng_c::openItemSel(dMenuMng_c::makePocketMask(isPocketDisabled), 0x22, dMenuMng_c::MENU_ITEM_SELECT)) {
                setSwitch(BED_MENU, SWITCH_PARAM_NONE);
            }
        }
    }
}

// 0x6448
BOOL daFtrBed_c::initMenu() {
    return TRUE;
}

static inline dHome_c *getPlayerHome() {
    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    return homes->getHome(homes->findCurrentPlayer());
}

static inline void putPocket(dPrivateData_c *player, const dItem::Item &item, int slot) {
    player->setPocket(&item, slot, FALSE);
}

// 0x6450
void daFtrBed_c::executeMenu() {
    if (!dMenuMng_c::isMenuEnd()) {
        return;
    }
    if (lbl_8074EA7D != 1) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
        return;
    }
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u32 slot = dMenuMng_c::getSelSlot();
    BOOL done = FALSE;
    if (slot < PLAYER_POCKETS_COUNT) {
        mNewItem = player->mPockets[slot];
        dHome_c *home = getPlayerHome();
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(mNewItem);
        if (home != NULL && bitm != NULL && isBedFunc(bitm->getFtrFunc())) {
            home->setLoftFtr(mNewItem);
            if (player != NULL) {
                putPocket(player, getItem(), slot);
            }
            setSwitch(BED_REPLACE, SWITCH_PARAM_NONE);
            done = TRUE;
        }
    }
    if (!done) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
    }
}

// 0x65E8
BOOL daFtrBed_c::initReplace() {
    getUnit(&mNewUnitX, &mNewUnitZ, NULL, mAng(0));
    startShrink();
    return TRUE;
}

// 0x6644
void daFtrBed_c::executeReplace() {
    if (mState == STATE_VANISHED) {
        static const int l_rot = 3;
        int x = mNewUnitX;
        int z = mNewUnitZ;
        if (dFtrm_c::createFtr(makeInfo(x, z, dItem::Item(mNewItem), l_rot, 0, FTR_KIND_APPEAR))) {
            setSwitch(BED_FINISH, SWITCH_PARAM_NONE);
        }
    }
}

// 0x66C4
BOOL daFtrBed_c::initFinish() {
    mFinishTimer = 0;
    return TRUE;
}

// 0x66D4
void daFtrBed_c::executeFinish() {
    static const int l_finishWait = 66;
    if (mFinishTimer < l_finishWait) {
        mFinishTimer++;
    }
    if (mFinishTimer == l_finishWait) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
        deleteRequest();
    }
}

// 0x6744
void daFtrBed_c::onDemoCmd(int cmd, u8 member) {
    if (!demoCmd(cmd)) {
        switch (cmd) {
        case DEMO_CMD_SPEAK:
            setSwitch(BED_SPEAK, SWITCH_PARAM_NONE);
            break;
        case DEMO_CMD_END:
            setSwitch(BED_LOFT, SWITCH_PARAM_NONE);
            break;
        }
    }
}

// 0x67D8
void daFtrBed_c::onAnswer(int arg) {
    dMsg::Comp_c *comp = mpController;
    int answer = comp != NULL && comp->getSelect() != NULL ? comp->getSelect()->mCursor : -1;
    switch (answer) {
    case ANSWER_SWAP:
        setSwitch(BED_SELECT, SWITCH_PARAM_NONE);
        break;
    case ANSWER_KEEP:
        setSwitch(BED_CANCEL, SWITCH_PARAM_NONE);
        break;
    }
}

// 0x6850
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CHEST, daFtrChest_c, fProf::FTR_CHEST, fProf::DRAW_ORDER::FTR_CHEST);
} // namespace dAFtrAct

// 0x6C04
BOOL daFtrChest_c::initModel() {
    if (getKind() == FTR_KIND_APPEAR) {
        changeSwitch(1, FALSE);
    }
    int func = getFtrFunc();
    if (func == dItem::FTR_FUNC_COOLER) {
        isAnmNum(2, ANM_ALL);
        createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_CLOSET) {
        isAnmNum(1, ANM_ALL);
        createAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    } else {
        nw4r::g3d::ResAnmChr anm = getAnmChr();
        if (anm.IsValid()) {
            createAnmChr(1.0f, anm, m3d::REVERSE_ONCE, 0);
        }
    }
    if (getSwitch() || dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        setSwitch(SWITCH_CLOSED, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OPEN, SWITCH_PARAM_NONE);
    }
    setMsgState(MSG_IDLE);
    return TRUE;
}

// 0x6D88
BOOL daFtrChest_c::executeMain() {
    executeMsgState();
    executeSwitch();
    return TRUE;
}

// 0x6DC0
BOOL daFtrChest_c::deleteRes() {
    return TRUE;
}

// 0x6DC8
BOOL daFtrChest_c::isWait() {
    if (daFTR_c::isWait() && mMsgState == MSG_IDLE) {
        return TRUE;
    }
    return FALSE;
}

// 0x6DF0
nw4r::g3d::ResAnmChr daFtrChest_c::getAnmChr() {
    nw4r::g3d::ResFile file(dFtrm_c::sLoader.getFile("int_anim.brres"));
    switch (getFtrFunc()) {
    case dItem::FTR_FUNC_DRESSER:
        return file.GetResAnmChr("int_anim_chestA");
    case dItem::FTR_FUNC_WARDROBE:
        return file.GetResAnmChr("int_anim_chestB");
    case dItem::FTR_FUNC_ARMOIRE:
        return file.GetResAnmChr("int_anim_chestC");
    }
    return nw4r::g3d::ResAnmChr(NULL);
}

// 0x6EA8
int daFtrChest_c::getSwitchBit(int sw) {
    static const u8 bits[] = {1, 0, 0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x6EC8
BOOL daFtrChest_c::setSwitch(int sw, int param) {
    static BOOL (daFtrChest_c::*const init[])() = {&daFtrChest_c::initClosed, &daFtrChest_c::initOpening,
                                                   &daFtrChest_c::initOpen, &daFtrChest_c::initClosing};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x6F3C
void daFtrChest_c::executeSwitch() {
    static void (daFtrChest_c::*const execute[])() = {&daFtrChest_c::executeClosed, &daFtrChest_c::executeOpening,
                                                      &daFtrChest_c::executeOpen, &daFtrChest_c::executeClosing};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x6F7C
BOOL daFtrChest_c::initClosed() {
    mTimer.set(AUTO_OPEN_WAIT);
    changeSwitch(1, FALSE);
    int func = getFtrFunc();
    if (func == dItem::FTR_FUNC_COOLER) {
        setAnmEnd(1.0f, 1, m3d::FORWARD_ONCE, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_CLOSET) {
        changeAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    } else {
        nw4r::g3d::ResAnmChr anm = getAnmChr();
        if (anm.IsValid()) {
            changeAnmChr(1.0f, anm, m3d::REVERSE_ONCE, 0);
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 0x7078
void daFtrChest_c::executeClosed() {
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    if (isSwitchChanging() && hold != NULL && hold->mSide == dFTR::SIDE_FRONT) {
        if (getFtrFunc() == dItem::FTR_FUNC_DRESSER) {
            dFTR::unitList_c units;
            getUnits(&units, NULL);
            mVec3_c ofs(0.0f, 0.0f, 33.0f);
            ofs.rotY(mAngle.y);
            int x, z;
            dBGCF::posToUnit(&x, &z, &ofs);
            if (x <= -2) {
                x = -1;
            }
            if (z <= -2) {
                z = -1;
            }
            for (u32 i = 0; i < units.mNum; i++) {
                int ux = x + units.get(i)->x;
                int uz = z + units.get(i)->z;
                mVec3_c pos;
                dBGCF::unitToPos(&pos, ux, uz);
                daFTR_c *ftr = dFTR::getUnitMap()->get(ux, uz, 0);
                if (dFTR::getGroundY(ux, uz) != dBGCF::getGroundY(&pos, FALSE) || dBGCF::getAttr(ux, uz) == BG_ATTR_WALL ||
                    (ftr != NULL && ftr != this && !ftr->canWalkOn()) || dBGCF::getOpenNext(ux, uz) != -1) {
                    changeSwitch(1, FALSE);
                    return;
                }
            }
        }
        dDemo::requestSpeak(this, NULL);
        dPlayerMgr::keepAnimation();
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_OPENING, SWITCH_PARAM_NONE);
    }
}

// 0x72F8
BOOL daFtrChest_c::initOpening() {
    changeSwitch(0, FALSE);
    turnOnSound();
    int func = getFtrFunc();
    if (func == dItem::FTR_FUNC_COOLER) {
        changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_CLOSET) {
        changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    } else {
        nw4r::g3d::ResAnmChr anm = getAnmChr();
        if (anm.IsValid()) {
            changeAnmChr(1.0f, anm, m3d::FORWARD_ONCE, 0);
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 0x73F0
void daFtrChest_c::executeOpening() {
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OPEN, SWITCH_PARAM_NONE);
    }
}

// 0x744C
BOOL daFtrChest_c::initOpen() {
    mTimer.set(AUTO_CLOSE_WAIT);
    changeSwitch(0, FALSE);
    int func = getFtrFunc();
    if (func == dItem::FTR_FUNC_COOLER) {
        setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_CLOSET) {
        setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    } else {
        nw4r::g3d::ResAnmChr anm = getAnmChr();
        if (anm.IsValid()) {
            setAnmChrEnd(1.0f, anm, m3d::FORWARD_ONCE);
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 0x7540
void daFtrChest_c::executeOpen() {
    if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_CLOSING, SWITCH_PARAM_NONE);
    }
}

// 0x75AC
BOOL daFtrChest_c::initClosing() {
    changeSwitch(1, FALSE);
    turnOffSound();
    int func = getFtrFunc();
    if (func == dItem::FTR_FUNC_COOLER) {
        changeAnm(1.0f, 1, m3d::FORWARD_ONCE, 0, ANM_ALL);
    } else if (func == dItem::FTR_FUNC_CLOSET) {
        setAnmEnd(1.0f, 0, m3d::REVERSE_ONCE, ANM_ALL);
    } else {
        nw4r::g3d::ResAnmChr anm = getAnmChr();
        if (anm.IsValid()) {
            setAnmChrEnd(1.0f, anm, m3d::REVERSE_ONCE);
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 0x769C
void daFtrChest_c::executeClosing() {
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_CLOSED, SWITCH_PARAM_NONE);
    }
}

// 0x76F8
void daFtrChest_c::onDemoCmd(int cmd, u8 member) {
    if (!demoCmd(cmd)) {
        switch (cmd) {
        case DEMO_CMD_SPEAK:
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE)) {
                if (dScene::isCurrentSceneMyHouse()) {
                    setMsgState(MSG_OPEN_START);
                } else {
                    setMsgState(MSG_TALK_START);
                }
            } else {
                setMsgState(MSG_TALK_START);
            }
            break;
        case DEMO_CMD_END:
            setMsgState(MSG_IDLE);
            break;
        }
    }
}

// 0x77A8
int daFtrChest_c::getChestType() {
    static const int funcs[] = {dItem::FTR_FUNC_DRESSER, dItem::FTR_FUNC_WARDROBE, dItem::FTR_FUNC_ARMOIRE};
    for (u32 i = 0; i < ARRAY_SIZE(funcs); i++) {
        if (funcs[i] == getFtrFunc()) {
            return i;
        }
    }
    return -1;
}

// 0x7828
BOOL daFtrChest_c::setMsgState(int state) {
    static BOOL (daFtrChest_c::*const init[])() = {
        &daFtrChest_c::initMsgIdle,      &daFtrChest_c::initMsgOpenStart, &daFtrChest_c::initMsgOpenWalk,
        &daFtrChest_c::initMsgOpen,      &daFtrChest_c::initMsgMenu,      &daFtrChest_c::initMsgClose,
        &daFtrChest_c::initMsgTalkStart, &daFtrChest_c::initMsgTalkWalk,  &daFtrChest_c::initMsgTalkOpen,
        &daFtrChest_c::initMsgTalk,      &daFtrChest_c::initMsgTalkEnd,   &daFtrChest_c::initMsgTalkClose,
    };
    if (state < MSG_NUM && (this->*init[state])()) {
        mMsgState = state;
        return TRUE;
    }
    return FALSE;
}

// 0x7894
void daFtrChest_c::executeMsgState() {
    static void (daFtrChest_c::*const execute[])() = {
        &daFtrChest_c::executeMsgIdle,      &daFtrChest_c::executeMsgOpenStart, &daFtrChest_c::executeMsgOpenWalk,
        &daFtrChest_c::executeMsgOpen,      &daFtrChest_c::executeMsgMenu,      &daFtrChest_c::executeMsgClose,
        &daFtrChest_c::executeMsgTalkStart, &daFtrChest_c::executeMsgTalkWalk,  &daFtrChest_c::executeMsgTalkOpen,
        &daFtrChest_c::executeMsgTalk,      &daFtrChest_c::executeMsgTalkEnd,   &daFtrChest_c::executeMsgTalkClose,
    };
    if (mMsgState < MSG_NUM) {
        (this->*execute[mMsgState])();
    }
}

// 0x78D4
BOOL daFtrChest_c::initMsgIdle() {
    return TRUE;
}

// 0x78DC
void daFtrChest_c::executeMsgIdle() {}

// 0x78E0
BOOL daFtrChest_c::initMsgOpenStart() {
    return TRUE;
}

// 0x78E8
void daFtrChest_c::executeMsgOpenStart() {
    setMsgState(MSG_OPEN_WALK);
}

// 0x78F0
BOOL daFtrChest_c::initMsgOpenWalk() {
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    int type = getChestType();
    if (hold != NULL && type != -1) {
        f32 x = hold->mPushPos.x;
        f32 z = hold->mPushPos.z;
        return dPlayerMgr::openChest(type, hold->mPushAngle, x, z);
    }
    return TRUE;
}

// 0x7960
void daFtrChest_c::executeMsgOpenWalk() {
    if (getChestType() == -1) {
        setMsgState(MSG_OPEN);
    } else if (dPlayerMgr::hasActionActor()) {
        setMsgState(MSG_OPEN);
    }
}

// 0x79BC
BOOL daFtrChest_c::initMsgOpen() {
    return dFTR::setSwitch(this, SWITCH_OPENING, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x79D0
void daFtrChest_c::executeMsgOpen() {
    if (mSwitchState == SWITCH_OPEN) {
        setMsgState(MSG_MENU);
    }
}

// 0x79E8
BOOL daFtrChest_c::initMsgMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_STORAGE) != FALSE;
}

// 0x7A18
void daFtrChest_c::executeMsgMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        setMsgState(MSG_CLOSE);
    }
}

// 0x7A58
BOOL daFtrChest_c::initMsgClose() {
    int type = getChestType();
    if (type != -1) {
        dPlayerMgr::closeChest(type);
        return dFTR::setSwitch(this, SWITCH_CLOSING, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
    return dFTR::setSwitch(this, SWITCH_CLOSING, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x7AC4
void daFtrChest_c::executeMsgClose() {
    if (mSwitchState == SWITCH_CLOSED) {
        dDemo::finishTalk(this);
    }
}

// 0x7AD8
BOOL daFtrChest_c::initMsgTalkStart() {
    return TRUE;
}

// 0x7AE0
void daFtrChest_c::executeMsgTalkStart() {
    setMsgState(MSG_TALK_WALK);
}

// 0x7AE8
BOOL daFtrChest_c::initMsgTalkWalk() {
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    int type = getChestType();
    if (hold != NULL && type != -1) {
        f32 x = hold->mPushPos.x;
        f32 z = hold->mPushPos.z;
        return dPlayerMgr::openChest(type, hold->mPushAngle, x, z);
    }
    return TRUE;
}

// 0x7B58
void daFtrChest_c::executeMsgTalkWalk() {
    if (getChestType() == -1) {
        setMsgState(MSG_TALK_OPEN);
    } else if (dPlayerMgr::hasActionActor()) {
        setMsgState(MSG_TALK_OPEN);
    }
}

// 0x7BB4
BOOL daFtrChest_c::initMsgTalkOpen() {
    return dFTR::setSwitch(this, SWITCH_OPENING, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x7BC8
void daFtrChest_c::executeMsgTalkOpen() {
    if (mSwitchState == SWITCH_OPEN) {
        setMsgState(MSG_TALK);
    }
}

// 0x7BE0
BOOL daFtrChest_c::initMsgTalk() {
    static const int debugMsgNum = SNOOP_MSG_NUM_VILLAGER;
    // Unreferenced; known only from .rodata. The owner is a guess that keeps the slot order.
    static const u16 _AAC = 1;
    static const int _AB0 = 3;
    attachDemoActor(this);
    mpController->requestStart(0);
    setMessageLabel("sys_MESS/SYS_Chest");
    int code;
    u32 num;
    int first;
    u32 scene = dScene::getCurrentScene();
    if (dFtrm_c::isDebug()) {
        code = mDebugMsgCount % debugMsgNum + SNOOP_MSG_FIRST;
        mDebugMsgCount++;
    } else {
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
            first = SNOOP_MSG_FIRST;
            num = SNOOP_MSG_NUM_VILLAGER;
        } else {
            first = SNOOP_MSG_FIRST;
            num = SNOOP_MSG_NUM_PLAYER;
        }
        dFTR::unitList_c units;
        getUnits(&units, NULL);
        int x, z;
        dBGCF::posToUnit(&x, &z, &mPos);
        scene += x + z * 4;
        code = first + scene % num;
    }
    setMessageCode(code);
    return TRUE;
}

// 0x7D20
void daFtrChest_c::executeMsgTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setMsgState(MSG_TALK_END);
        }
    }
}

// 0x7D94
BOOL daFtrChest_c::initMsgTalkEnd() {
    return TRUE;
}

// 0x7D9C
void daFtrChest_c::executeMsgTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setMsgState(MSG_TALK_CLOSE);
        }
    }
}

// 0x7E10
BOOL daFtrChest_c::initMsgTalkClose() {
    int type = getChestType();
    if (type != -1) {
        if (dPlayerMgr::closeChest(type)) {
            return dFTR::setSwitch(this, SWITCH_CLOSING, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
        return FALSE;
    }
    return dFTR::setSwitch(this, SWITCH_CLOSING, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x7E8C
void daFtrChest_c::executeMsgTalkClose() {
    if (mSwitchState == SWITCH_CLOSED) {
        detachDemoActor(this);
        dDemo::finishTalk(this);
    }
}

// 0x7EE0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CLOCK, daFtrClock_c, fProf::FTR_CLOCK, fProf::DRAW_ORDER::FTR_CLOCK);
} // namespace dAFtrAct

// 0x828C
BOOL daFtrClock_c::initModel() {
    isAnmNum(1, ANM_ALL);
    mTime = mPrevTime = *dTime_c::getCurrent();
    mChime = CHIME_NONE;
    if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_CHIME) {
        nw4r::g3d::ResFile res(mRes.getData());
        nw4r::g3d::ResMdl mdl = getResMdl();
        mAnmChr.create(mdl, res.GetResAnmChr(0), &mAllocator, NULL);
        mAnmChr.setFrame(0.0f);
        mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
        mAnmChr.setRate(1.0f);
        mMdl.setAnm(mAnmChr);
    } else if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_SYNC || getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) {
        nw4r::g3d::ResFile res(mRes.getData());
        nw4r::g3d::ResMdl mdl = getResMdl();
        if (res.GetResAnmChrNumEntries() != 0) {
            mAnmChr.create(mdl, res.GetResAnmChr(0), &mAllocator, NULL);
            mAnmChr.setFrame(0.0f);
            mAnmChr.mPlayMode = m3d::FORWARD_LOOP;
            mAnmChr.setRate(1.0f);
            mMdl.setAnm(mAnmChr);
        }
    } else if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_ANIM) {
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    }
    return TRUE;
}

// 0x8510
BOOL daFtrClock_c::executeIdle() {
    u16 timer = dFtrm_c::getTimer();
    if ((getFtrFunc() == dItem::FTR_FUNC_CLOCK_SYNC || getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) && mAnmChr.IsBound()) {
        mAnmChr.setFrame(timer);
    }
    return TRUE;
}

// 0x85BC
BOOL daFtrClock_c::executeMain() {
    mPrevTime = mTime;
    mTime = *dTime_c::getCurrent();
    if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_CHIME || getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) {
        if (!isSyncDisabled() && mChime >= 0 && mChime != mPrevChime) {
            turnOnSound();
        }
        mPrevChime = mChime;
        if (mChime == CHIME_NONE) {
            if (isSyncDisabled()) {
                mChime = 1;
            } else if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM) && mPrevTime.hour != mTime.hour) {
                mChime = mTime.hour % 12;
                if (mChime == 0) {
                    mChime = 12;
                }
                if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) {
                    mChimeTimer = CHIME_WAIT;
                } else if (mAnmChr.IsBound()) {
                    mAnmChr.setFrame(0.0f);
                    mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
                    mAnmChr.setRate(1.0f);
                }
            }
        } else if (mChime > 0) {
            if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) {
                if (mChimeTimer > 0) {
                    mChimeTimer--;
                }
                if (mChimeTimer == 0) {
                    if (!isSyncDisabled()) {
                        mChime--;
                    }
                    if (mChime == 0) {
                        mChime = CHIME_NONE;
                    }
                    mChimeTimer = CHIME_WAIT;
                }
            } else if (mAnmChr.IsBound()) {
                mAnmChr.play();
                if (mAnmChr.isStop()) {
                    mAnmChr.setFrame(0.0f);
                    mAnmChr.mPlayMode = m3d::FORWARD_ONCE;
                    mAnmChr.setRate(1.0f);
                    if (!isSyncDisabled()) {
                        mChime--;
                    }
                    if (mChime == 0) {
                        mChime = CHIME_NONE;
                    }
                }
            }
        }
    } else if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_ANIM) {
        playAnm(-1.0f, ANM_ALL);
    }
    u16 timer = dFtrm_c::getTimer();
    if ((getFtrFunc() == dItem::FTR_FUNC_CLOCK_SYNC || getFtrFunc() == dItem::FTR_FUNC_CLOCK_CUCKOO) && mAnmChr.IsBound()) {
        mAnmChr.setFrame(timer);
    }
    if (timer == TICK_FRAME || timer == TOCK_FRAME) {
        if (getFtrFunc() == dItem::FTR_FUNC_CLOCK_CHIME) {
            turnOffSound();
        } else if (getFtrFunc() != dItem::FTR_FUNC_CLOCK_CUCKOO) {
            turnOnSound();
        }
    }
    _920 = 0;
    _921 = 0;
    return TRUE;
}

// 0x8A38
BOOL daFtrClock_c::deleteRes() {
    return TRUE;
}

static dItem::Item l_villagerSong;

// 0x8A40
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_AUDIO, daFtrAudio_c, fProf::FTR_AUDIO, fProf::DRAW_ORDER::FTR_AUDIO);
} // namespace dAFtrAct

// 0x8E10
BOOL daFtrAudio_c::initModel() {
    if (getFtrFunc() == dItem::FTR_FUNC_AUDIO_ON_OFF_ANIM) {
        isAnmNum(1, ANM_CHR);
        isAnmNum(2, ANM_ALL & ~ANM_CHR);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    } else if (getFtrFunc() == dItem::FTR_FUNC_AUDIO) {
        isAnmNum(1, ANM_ALL);
        createAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    }
    mLamps.init(getResMdl(), getSwitch());
    if (getKind() == FTR_KIND_APPEAR) {
        dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled()) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        u8 scene = dScene::getCurrentScene();
        dSvMdlRm_c *modelRoom = &dSaveData_c::getTown()->mModelRoom;
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE) ||
            (dScene::isCurrentSceneModelRoom() && modelRoom->isVillagerOwner())) {
            int song = 0;
            if (!dScene::isCurrentSceneModelRoom()) {
                int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(), dScene::SCENE_ATTR_VILLAGER_HOUSE);
                dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
                if (animal != NULL && animal->mID.isValid()) {
                    dItem::Item music = animal->getMusic();
                    if (music.getKind() == dItem::KIND_MUSIC) {
                        song = dItem::seeker_c::get()->findLike(music);
                    }
                }
            } else {
                dItem::Item music = modelRoom->getSong();
                if (music.getKind() == dItem::KIND_MUSIC) {
                    song = dItem::seeker_c::get()->findLike(music);
                }
            }
            if (l_villagerSong.getKind() == dItem::KIND_MUSIC) {
                setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
            } else if (setSwitch(SWITCH_ON, (u8)song)) {
                l_villagerSong = dItem::Item(dItem::ITEM_IDX_KK_CHORALE, song, FALSE);
            }
        } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE) || dScene::isCurrentSceneModelRoom()) {
            int song = 0;
            if (!dScene::isCurrentSceneModelRoom()) {
                dItem::Item music = getCurrentRoomSong();
                if (music.getKind() == dItem::KIND_MUSIC) {
                    song = dItem::seeker_c::get()->findLike(music);
                }
            } else {
                dItem::Item music = modelRoom->getSong();
                if (music.getKind() == dItem::KIND_MUSIC) {
                    song = dItem::seeker_c::get()->findLike(music);
                }
            }
            setSwitch(SWITCH_ON, (u8)song);
        } else {
            clearCurrentRoomSong();
            setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
        }
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0x9294
BOOL daFtrAudio_c::executeMain() {
    if (mMdl.getScn() != NULL) {
        executeSwitch();
        mLamps.update();
    }
    executeDemo();
    return TRUE;
}

// 0x92E0
BOOL daFtrAudio_c::executeIdle() {
    return executeMain();
}

// 0x92F0
BOOL daFtrAudio_c::deleteRes() {
    stopMusic(mSong, mState == STATE_REMOVE);
    if (l_villagerSong.getKind() == dItem::KIND_MUSIC) {
        l_villagerSong = dItem::Item();
    }
    return TRUE;
}

// 0x9350
void daFtrAudio_c::playMusic(int song) {
    if (!mPlaying && (u32)song < SONG_NUM) {
        dBgm::l_mgr.mStgRoom.playMusic(song + 1);
        dBgm::l_mgr.mStgRoom.fn_800792AC(mSndFtr.getType());
        dItem::Item item(dItem::ITEM_IDX_KK_CHORALE, song, FALSE);
        setCurrentRoomSong(item);
        mPlaying = TRUE;
    }
}

// 0x93F4
void daFtrAudio_c::stopMusic(int song, BOOL clearRoomSong) {
    if (mPlaying && (u32)song < SONG_NUM) {
        dBgm::l_mgr.mStgRoom.stopMusic();
        if (clearRoomSong) {
            setCurrentRoomSong(dItem::Item());
        }
        mPlaying = FALSE;
    }
}

// 0x9470
void daFtrAudio_c::onDemoCmd(int cmd, u8 member) {
    if (!demoCmd(cmd)) {
        switch (cmd) {
        case DEMO_CMD_SPEAK:
            if (dScene::isCurrentSceneMyHouse()) {
                setDemo(DEMO_MENU_WAIT);
            } else {
                setDemo(DEMO_TALK_WAIT);
            }
            break;
        case DEMO_CMD_END:
            setDemo(DEMO_NONE);
            break;
        }
    }
}

// 0x9500
BOOL daFtrAudio_c::setDemo(int state) {
    static BOOL (daFtrAudio_c::*const init[])() = {
        &daFtrAudio_c::initDemoNone,     &daFtrAudio_c::initDemoMenuWait, &daFtrAudio_c::initDemoMenu,
        &daFtrAudio_c::initDemoTalkWait, &daFtrAudio_c::initDemoTalk,     &daFtrAudio_c::initDemoTalkEnd,
    };
    if (state < DEMO_NUM && (this->*init[state])()) {
        mDemoState = state;
        return TRUE;
    }
    return FALSE;
}

// 0x956C
void daFtrAudio_c::executeDemo() {
    static void (daFtrAudio_c::*const execute[])() = {
        &daFtrAudio_c::executeDemoNone,     &daFtrAudio_c::executeDemoMenuWait, &daFtrAudio_c::executeDemoMenu,
        &daFtrAudio_c::executeDemoTalkWait, &daFtrAudio_c::executeDemoTalk,     &daFtrAudio_c::executeDemoTalkEnd,
    };
    if (mDemoState < DEMO_NUM) {
        (this->*execute[mDemoState])();
    }
}

// 0x95AC
BOOL daFtrAudio_c::initDemoNone() {
    return TRUE;
}

// 0x95B4
void daFtrAudio_c::executeDemoNone() {}

// 0x95B8
BOOL daFtrAudio_c::initDemoMenuWait() {
    return TRUE;
}

// 0x95C0
void daFtrAudio_c::executeDemoMenuWait() {
    setDemo(DEMO_MENU);
}

// 0x95C8
BOOL daFtrAudio_c::initDemoMenu() {
    return dMenuMng_c::openMenu(dMenuMng_c::MENU_MUSIC) != FALSE;
}

// 0x95F8
void daFtrAudio_c::executeDemoMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        dDemo::finishTalk(this);
    }
}

// 0x9634
BOOL daFtrAudio_c::initDemoTalkWait() {
    return TRUE;
}

// 0x963C
void daFtrAudio_c::executeDemoTalkWait() {
    setDemo(DEMO_TALK);
}

// 0x9644
BOOL daFtrAudio_c::initDemoTalk() {
    attachDemoActor(this);
    mpController->requestStart(0);
    setMessageLabel("sys_MESS/SYS_Player");
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
        dItem::Item music(dItem::ITEM_IDX_KK_CHORALE);
        int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(), dScene::SCENE_ATTR_VILLAGER_HOUSE);
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
        if (animal != NULL && animal->mID.isValid()) {
            music = animal->getMusic();
        }
        setMessageCode(MSG_SONG_TITLE);
        mpController->setItemName(1, music);
    } else {
        dItem::Item music = getCurrentRoomSong();
        if (music.getKind() == dItem::KIND_MUSIC) {
            setMessageCode(MSG_SONG_TITLE);
            mpController->setItemName(1, music);
        } else {
            setMessageCode(MSG_NO_MUSIC);
        }
    }
    return TRUE;
}

// 0x9784
void daFtrAudio_c::executeDemoTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setDemo(DEMO_TALK_END);
        }
    }
}

// 0x97F8
BOOL daFtrAudio_c::initDemoTalkEnd() {
    return TRUE;
}

// 0x9800
void daFtrAudio_c::executeDemoTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}

// 0x9888
int daFtrAudio_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x98A8
BOOL daFtrAudio_c::setSwitch(int sw, int param) {
    static BOOL (daFtrAudio_c::*const init[])() = {&daFtrAudio_c::initOff, &daFtrAudio_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x991C
void daFtrAudio_c::executeSwitch() {
    static void (daFtrAudio_c::*const execute[])() = {&daFtrAudio_c::executeOff, &daFtrAudio_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x995C
BOOL daFtrAudio_c::initOff() {
    turnOffSound();
    changeSwitch(0, FALSE);
    if (mFlags.mCreated) {
        stopMusic(mSong, TRUE);
    }
    if (getFtrFunc() == dItem::FTR_FUNC_AUDIO_ON_OFF_ANIM) {
        changeAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL & ~ANM_CHR);
    }
    return TRUE;
}

// 0x9A00
void daFtrAudio_c::executeOff() {
    if (getFtrFunc() == dItem::FTR_FUNC_AUDIO_ON_OFF_ANIM) {
        playAnm(-1.0f, ANM_ALL & ~ANM_CHR);
    }
    mLamps.set(FALSE, TRUE, FALSE);
    if (isSwitchChanging()) {
        changeSwitch(0, FALSE);
        dDemo::requestSpeak(this, NULL);
        dPlayerMgr::keepAnimation();
    }
}

// 0x9AA4
BOOL daFtrAudio_c::initOn() {
    turnOnSound();
    dFTR::setSwitchAll(0, isMusicOn, 0);
    changeSwitch(1, FALSE);
    mSong = mSwitchParam;
    playMusic(mSong);
    if (getFtrFunc() == dItem::FTR_FUNC_AUDIO_ON_OFF_ANIM) {
        changeAnm(1.0f, 1, m3d::FORWARD_LOOP, 0, ANM_ALL & ~ANM_CHR);
    }
    return TRUE;
}

// 0x9B50
void daFtrAudio_c::executeOn() {
    playAnm(-1.0f, ANM_ALL);
    updateSound();
    mLamps.set(TRUE, TRUE, FALSE);
    dBgm::l_mgr.mStgRoom.setMusicPos(&mCenter);
    if (isSwitchChanging()) {
        changeSwitch(1, FALSE);
        dDemo::requestSpeak(this, NULL);
    }
}

// 0x9BF0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_TV, daFtrTv_c, fProf::FTR_TV, fProf::DRAW_ORDER::FTR_TV);
} // namespace dAFtrAct

// 0x9FC0
BOOL daFtrTv_c::initModel() {
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        mLamps.init(getResMdl(), TRUE);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        mLamps.init(getResMdl(), FALSE);
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0xA0A0
BOOL daFtrTv_c::executeMain() {
    mLamps.update();
    executeSwitch();
    mNearest = FALSE;
    return TRUE;
}

// 0xA0E4
BOOL daFtrTv_c::executeIdle() {
    mLamps.update();
    return TRUE;
}

// 0xA10C
BOOL daFtrTv_c::deleteRes() {
    return TRUE;
}

static inline void bindSelf(nw4r::g3d::ResFile &res) {
    res.Bind(res);
}

// 0xA114
BOOL daFtrTv_c::bindProgramRes(BOOL on) {
    nw4r::g3d::ResFile res(mRes.getData());
    void *common = dFtrm_c::sLoader.getFile("int_common.brres");
    if (on) {
        void *dummy = dFtrm_c::sLoader.getFile("int_kob_oth_dummytv.brres");
        res.Release();
        res.Bind(nw4r::g3d::ResFile(dummy));
        res.Bind(nw4r::g3d::ResFile(common));
        bindSelf(res);
    } else {
        res.Release();
        res.Bind(nw4r::g3d::ResFile(common));
        bindSelf(res);
    }
    return TRUE;
}

// 0xA1F0
void daFtrTv_c::setNearest() {
    mNearest = TRUE;
}

// 0xA1FC
int daFtrTv_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0xA21C
BOOL daFtrTv_c::setSwitch(int sw, int param) {
    static BOOL (daFtrTv_c::*const init[])() = {&daFtrTv_c::initOff, &daFtrTv_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xA290
void daFtrTv_c::executeSwitch() {
    static void (daFtrTv_c::*const execute[])() = {&daFtrTv_c::executeOff, &daFtrTv_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xA2D0
BOOL daFtrTv_c::initOff() {
    turnOffSound();
    mLamps.set(FALSE, TRUE, FALSE);
    changeSwitch(0, FALSE);
    bindProgramRes(FALSE);
    return TRUE;
}

// 0xA33C
void daFtrTv_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL && hold->mSide == dFTR::SIDE_FRONT) {
            dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    }
}

// 0xA3A8
BOOL daFtrTv_c::initOn() {
    turnOnSound();
    mLamps.set(TRUE, TRUE, FALSE);
    changeSwitch(1, FALSE);
    bindProgramRes(TRUE);
    return TRUE;
}

// 0xA414
void daFtrTv_c::executeOn() {
    if (isSwitchChanging()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL && hold->mSide == dFTR::SIDE_FRONT) {
            dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    }
}

// 0xA480
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_TV_F, daFtrTvF_c, fProf::FTR_TV_F, fProf::DRAW_ORDER::FTR_TV_F);
} // namespace dAFtrAct

// 0xA874
inline daFtrTv_c::~daFtrTv_c() {}

// 0xA9B4
BOOL daFtrTvF_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        mLamps.init(getResMdl(), TRUE);
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        mLamps.init(getResMdl(), FALSE);
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0xAAC4
BOOL daFtrTvF_c::executeMain() {
    mLamps.update();
    executeSwitch();
    return TRUE;
}

// 0xAB00
BOOL daFtrTvF_c::deleteRes() {
    return TRUE;
}

// 0xAB08
int daFtrTvF_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0xAB28
BOOL daFtrTvF_c::setSwitch(int sw, int param) {
    static BOOL (daFtrTvF_c::*const init[])() = {
        &daFtrTvF_c::initOff,
        &daFtrTvF_c::initOpen,
        &daFtrTvF_c::initOn,
        &daFtrTvF_c::initClose,
    };
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mDoorState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xAB9C
void daFtrTvF_c::executeSwitch() {
    static void (daFtrTvF_c::*const execute[])() = {
        &daFtrTvF_c::executeOff,
        &daFtrTvF_c::executeOpen,
        &daFtrTvF_c::executeOn,
        &daFtrTvF_c::executeClose,
    };
    if (mDoorState < ARRAY_SIZE(execute)) {
        (this->*execute[mDoorState])();
    }
}

// 0xABDC
BOOL daFtrTvF_c::initOff() {
    changeSwitch(0, FALSE);
    mLamps.set(FALSE, TRUE, FALSE);
    bindProgramRes(FALSE);
    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    return TRUE;
}

// 0xAC60
void daFtrTvF_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL && hold->mSide == dFTR::SIDE_FRONT) {
            dFTR::requestSwitch(this, SWITCH_OPEN, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    }
}

// 0xACCC
BOOL daFtrTvF_c::initOpen() {
    changeSwitch(1, FALSE);
    mLamps.set(TRUE, TRUE, FALSE);
    bindProgramRes(TRUE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0xAD58
void daFtrTvF_c::executeOpen() {
    changeSwitch(1, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0xADD0
BOOL daFtrTvF_c::initOn() {
    changeSwitch(1, FALSE);
    mLamps.set(TRUE, TRUE, FALSE);
    bindProgramRes(TRUE);
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    return TRUE;
}

// 0xAE50
void daFtrTvF_c::executeOn() {
    if (isSwitchChanging()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL && hold->mSide == dFTR::SIDE_FRONT) {
            dFTR::requestSwitch(this, SWITCH_CLOSE, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    }
}

// 0xAEBC
BOOL daFtrTvF_c::initClose() {
    changeSwitch(0, FALSE);
    mLamps.set(FALSE, TRUE, FALSE);
    bindProgramRes(FALSE);
    setAnmEnd(1.0f, 0, m3d::REVERSE_ONCE, ANM_ALL);
    turnOffSound();
    return TRUE;
}

// 0xAF44
void daFtrTvF_c::executeClose() {
    changeSwitch(0, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0xAFBC
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_PCART, daFtrPcart_c, fProf::FTR_PCART, fProf::DRAW_ORDER::FTR_PCART);
} // namespace dAFtrAct

// 0xB368
BOOL daFtrPcart_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(0.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    return TRUE;
}

// 0xB3C0
BOOL daFtrPcart_c::executeMain() {
    playAnm(-1.0f, ANM_ALL);
    return TRUE;
}

// 0xB3F0
BOOL daFtrPcart_c::deleteRes() {
    return TRUE;
}

// 0xB3F8
BOOL daFtrPcart_c::onMoveStart(BOOL push) {
    dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
    BOOL moved = FALSE;
    if (hold != NULL) {
        nw4r::g3d::ResFile res(mRes.getData());
        nw4r::g3d::ResAnmChr anm = res.GetResAnmChr(0);
        if (getFtrFunc() == dItem::FTR_FUNC_CART) {
            if (hold->mSide == dFTR::SIDE_BACK) {
                if (push) {
                    if (turnOnSound()) {
                        moved = TRUE;
                    }
                    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
                } else {
                    if (turnOnSound()) {
                        moved = TRUE;
                    }
                    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, (u16)(anm.GetNumFrame() - 1), ANM_ALL);
                }
            } else if (hold->mSide == dFTR::SIDE_FRONT) {
                if (push) {
                    if (turnOnSound()) {
                        moved = TRUE;
                    }
                    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, (u16)(anm.GetNumFrame() - 1), ANM_ALL);
                } else {
                    if (turnOnSound()) {
                        moved = TRUE;
                    }
                    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
                }
            }
        } else if (hold->mSide == dFTR::SIDE_RIGHT) {
            if (push) {
                if (!turnOnSound()) {
                    moved = TRUE;
                }
                changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
            } else {
                if (turnOnSound()) {
                    moved = TRUE;
                }
                changeAnm(1.0f, 0, m3d::REVERSE_ONCE, (u16)(anm.GetNumFrame() - 1), ANM_ALL);
            }
        } else if (hold->mSide == dFTR::SIDE_LEFT) {
            if (push) {
                if (turnOnSound()) {
                    moved = TRUE;
                }
                changeAnm(1.0f, 0, m3d::REVERSE_ONCE, (u16)(anm.GetNumFrame() - 1), ANM_ALL);
            } else {
                if (turnOnSound()) {
                    moved = TRUE;
                }
                changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
            }
        }
    }
    return moved;
}

// 0xB6A4
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_HNW, daFtrHnw_c, fProf::FTR_HNW, fProf::DRAW_ORDER::FTR_HNW);
} // namespace dAFtrAct

// 0xBA60
BOOL daFtrHnw_c::initModel() {
    u8 scene = dScene::getCurrentScene();
    mSwitchParam = SWITCH_PARAM_NONE;
    createAnmChr(1.0f, getIdleAnm(), m3d::FORWARD_ONCE, 0);
    if (getKind() == FTR_KIND_APPEAR) {
        if (scene != dScene::SCENE_RM_AUCTION) {
            changeSwitch(0, FALSE);
        }
        mAppeared = TRUE;
    } else if (isSyncDisabled()) {
        setSwitch(SWITCH_PLAY, SWITCH_PARAM_NONE);
    } else if (scene == dScene::SCENE_RM_AUCTION) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else if (getSwitch()) {
        setSwitch(SWITCH_PLAY, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0xBBD0
BOOL daFtrHnw_c::executeMain() {
    executeSwitch();
    if (!mOwnerSent && getKind() == FTR_KIND_APPEAR && isIdle() && !getSwitch() &&
        dScene::getCurrentScene() != dScene::SCENE_RM_AUCTION) {
        dFTR::sendOwnerSwitch(dScene::getCurrentScene(), &mPos, getLayer());
        mOwnerSent = TRUE;
    }
    return TRUE;
}

// 0xBCA0
BOOL daFtrHnw_c::executeIdle() {
    executeMain();
    return TRUE;
}

// 0xBCD0
BOOL daFtrHnw_c::deleteRes() {
    return TRUE;
}

// 0xBCD8
nw4r::g3d::ResAnmChr daFtrHnw_c::getDanceAnm() {
    return nw4r::g3d::ResFile(mRes.getData()).GetResAnmChr(0);
}

// 0xBD08
nw4r::g3d::ResAnmChr daFtrHnw_c::getIdleAnm() {
    return nw4r::g3d::ResFile(dFtrm_c::sLoader.getFile("int_hnw.brres")).GetResAnmChr(0);
}

// 0xBD48
mVec3_c daFtrHnw_c::getScale() const {
    return daFTR_c::getScale() * dFtrm_c::getPullRate();
}

// 0xBD88
int daFtrHnw_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0xBDA8
BOOL daFtrHnw_c::setSwitch(int sw, int param) {
    static BOOL (daFtrHnw_c::*const init[])() = {&daFtrHnw_c::initOff, &daFtrHnw_c::initPlay,
                                                 &daFtrHnw_c::initDance};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xBE1C
void daFtrHnw_c::executeSwitch() {
    static void (daFtrHnw_c::*const execute[])() = {&daFtrHnw_c::executeOff, &daFtrHnw_c::executePlay,
                                                    &daFtrHnw_c::executeDance};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xBE5C
BOOL daFtrHnw_c::initOff() {
    mSwitchParam = SWITCH_PARAM_NONE;
    int x = worldToUnit(mPos.x);
    int z = worldToUnit(mPos.z);
    removeRoomGyroid(x, z, dScene::getCurrentScene());
    turnOffSound();
    changeSwitch(0, FALSE);
    if (!mFlags.mCreated) {
        if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
            mAnmChr.setAnm(mMdl, getIdleAnm(), m3d::FORWARD_LOOP);
            mAnmChr.setRate(0.0f);
            mAnmChr.setFrame(0.0f);
            mMdl.setAnm(mAnmChr);
        }
        mSnd.setup((u8)dItem::seeker_c::get()->findLike(getItem()), -1);
        mSndReady = TRUE;
    } else {
        mSnd.stop();
        if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
            mAnmChr.setAnm(mMdl, getIdleAnm(), m3d::FORWARD_LOOP);
            mMdl.setAnm(mAnmChr, 32.0f);
        }
    }
    return TRUE;
}

// 0xC018
void daFtrHnw_c::executeOff() {
    mPhase = mSnd.calc(&mPos);
    if (mMdl.getScn() != NULL) {
        mMdl.play();
    }
    if (isSwitchChanging()) {
        sendSwitch(0);
    }
}

// 0xC0D0
BOOL daFtrHnw_c::initPlay() {
    turnOnSound();
    changeSwitch(1, FALSE);
    int x = worldToUnit(mPos.x);
    int z = worldToUnit(mPos.z);
    mWait = WAIT_NONE;
    nw4r::g3d::ResAnmChr idleAnm = getIdleAnm();
    u8 startBar;
    if (!mFlags.mCreated || mAppeared || isSyncDisabled()) {
        if (!isSyncDisabled() && (mAppeared || getKind() != FTR_KIND_PLACED)) {
            mAppeared = FALSE;
            s8 nextBar = SoundHaniwa::getStartBar();
            mSnd.setup((u8)dItem::seeker_c::get()->findLike(getItem()), nextBar);
            mSndReady = TRUE;
            startBar = mSnd.getStartBar() & START_BAR_MASK;
            addRoomGyroid(x, z, startBar, dScene::getCurrentScene());
            mSwitchParam = startBar;
            mAnmChr.setAnm(mMdl, idleAnm, m3d::FORWARD_LOOP);
            mMdl.setAnm(mAnmChr);
            mMdl.play();
            mMdl.calc(false);
            mWait = mSnd.getWaitFrames();
            mAnmChr.setAnm(mMdl, getDanceAnm(), m3d::FORWARD_LOOP);
            mAnmChr.setFrame(0.0f);
            mAnmChr.setRate(0.0f);
            mMdl.setAnm(mAnmChr, mWait);
            mMdl.play();
        } else {
            u8 scene = dScene::getCurrentScene();
            int roomBar = isSyncDisabled() ? 0 : getRoomGyroid(x, z, scene);
            if (roomBar < 0) {
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
                    roomBar = cM::rndInt(0x100) & START_BAR_MASK;
                    addRoomGyroid(x, z, roomBar, scene);
                } else {
                    roomBar = 0;
                }
            }
            mSnd.setup((u8)dItem::seeker_c::get()->findLike(getItem()), roomBar);
            mSndReady = TRUE;
            mSwitchParam = roomBar & START_BAR_MASK;
            if (isSyncDisabled()) {
                mAnmChr.setAnm(mMdl, idleAnm, m3d::FORWARD_LOOP);
                mMdl.setAnm(mAnmChr);
                mMdl.play();
                mMdl.calc(false);
                mWait = mSnd.getWaitFrames();
                mAnmChr.setAnm(mMdl, getDanceAnm(), m3d::FORWARD_LOOP);
                mAnmChr.setFrame(0.0f);
                mAnmChr.setRate(0.0f);
                mMdl.setAnm(mAnmChr, mWait);
                mMdl.play();
            } else {
                if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
                    mAnmChr.setAnm(mMdl, idleAnm, m3d::FORWARD_LOOP);
                    mAnmChr.setFrame(0.0f);
                    mAnmChr.setRate(0.0f);
                    mMdl.setAnm(mAnmChr);
                    mMdl.play();
                }
                mWait = 0;
            }
        }
    } else if (dNet::isSessionActive() && !dNet::isSceneOwner() && (u8)getSwitchParam() != SWITCH_PARAM_NONE) {
        mSnd.stop();
        u8 kind = dItem::seeker_c::get()->findLike(getItem());
        mSnd.setup(kind, getSwitchParam() & START_BAR_MASK);
        mSndReady = TRUE;
        mWait = mSnd.getWaitFrames();
        u8 scene = dScene::getCurrentScene();
        u32 param = getSwitchParam();
        addRoomGyroid(x, z, param & START_BAR_MASK, scene);
        if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
            mAnmChr.setAnm(mMdl, getDanceAnm(), m3d::FORWARD_LOOP);
            mAnmChr.setFrame(0.0f);
            mAnmChr.setRate(0.0f);
            mMdl.setAnm(mAnmChr, mWait);
            mMdl.play();
        }
    } else {
        mSnd.play();
        mWait = mSnd.getWaitFrames();
        startBar = mSnd.getStartBar() & START_BAR_MASK;
        addRoomGyroid(x, z, startBar, dScene::getCurrentScene());
        mSwitchParam = startBar;
        if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
            mAnmChr.setAnm(mMdl, getDanceAnm(), m3d::FORWARD_LOOP);
            mAnmChr.setFrame(0.0f);
            mAnmChr.setRate(0.0f);
            mMdl.setAnm(mAnmChr, mWait);
            mMdl.play();
        }
    }
    return TRUE;
}

// 0xC79C
void daFtrHnw_c::executePlay() {
    mPhase = mSnd.calc(&mPos);
    if (mWait != 0 && mMdl.getScn() != NULL) {
        mMdl.play();
        mWait--;
    }
    if (mWait == 0) {
        u32 param = getSwitchParam();
        setSwitch(SWITCH_DANCE, (u8)param);
    }
    if (isSwitchChanging()) {
        sendSwitch(1);
    }
}

// 0xC8AC
BOOL daFtrHnw_c::initDance() {
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_LOOP, 0, ANM_ALL);
    if (mAnmChr.IsBound() && mMdl.getScn() != NULL) {
        mAnmChr.setAnm(mMdl, getDanceAnm(), m3d::FORWARD_LOOP);
        mAnmChr.setRate(0.0f);
        mAnmChr.setFrame(0.0f);
        mMdl.setAnm(mAnmChr);
    }
    return TRUE;
}

// 0xC988
void daFtrHnw_c::executeDance() {
    f32 phase = mSnd.calc(&mPos);
    mPhase = phase;
    if (phase >= 0.0f && mAnmChr.IsBound() && mMdl.getScn() != NULL) {
        u16 frames = getDanceAnm().GetNumFrame();
        mAnmChr.setFrame(frames * phase);
    }
    if (isSwitchChanging()) {
        sendSwitch(1);
    }
}

// 0xCA98
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_KILLER, daFtrKiller_c, fProf::FTR_KILLER, fProf::DRAW_ORDER::FTR_KILLER);
} // namespace dAFtrAct

// 0xCE4C
BOOL daFtrKiller_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    return TRUE;
}

// 0xCEC0
BOOL daFtrKiller_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0xCEE4
BOOL daFtrKiller_c::deleteRes() {
    changeSwitch(0, FALSE);
    return TRUE;
}

// 0xCF1C
void daFtrKiller_c::onRemove() {
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
}

// 0xCF38
BOOL daFtrKiller_c::setSwitch(int sw, int param) {
    static BOOL (daFtrKiller_c::*const init[])() = {&daFtrKiller_c::initOff, &daFtrKiller_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xCFAC
void daFtrKiller_c::executeSwitch() {
    static void (daFtrKiller_c::*const execute[])() = {&daFtrKiller_c::executeOff, &daFtrKiller_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xCFEC
BOOL daFtrKiller_c::initOff() {
    mSwitchNode.set(FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    return TRUE;
}

// 0xD044
void daFtrKiller_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::setSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0xD090
BOOL daFtrKiller_c::initOn() {
    mSwitchNode.set(TRUE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0xD0F0
void daFtrKiller_c::executeOn() {
    int node = m3d::getNodeID(mMdl.getResMdl(), "v");
    if (node != -1) {
        mMdl.calc(false);
        mMtx_c mtx;
        mMdl.getNodeWorldMtx(node, &mtx);
        mVec3_c pos(0.0f, 0.0f, 0.0f);
        PSMTXMultVec(mtx, mVec3_c::Zero, pos);
        int x, z;
        dBGCF::posToUnit(&x, &z, &pos);
        if (!dBGCF::canPutItem(x, z)) {
            dFTR::setSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
        }
    }
    if (playAnm(-1.0f, ANM_ALL)) {
        dFTR::setSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0xD1FC
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CLOTH, daFtrCloth_c, fProf::FTR_CLOTH, fProf::DRAW_ORDER::FTR_CLOTH);
} // namespace dAFtrAct

// 0xD22C
daFtrCloth_c::daFtrCloth_c() {}

// 0xD5CC
BOOL daFtrCloth_c::createRes() {
    if (mMannequinRes.IsValid()) {
        return TRUE;
    }
    EGG::FrmHeap *heap = getHeap();
    void *res;
    if (dScene::getCurrentScene() == dScene::SCENE_RM_GRACE) {
        res = dFtrm_c::sLoader.loadFile("int_mannequin_grace.brres", heap);
    } else {
        res = dFtrm_c::sLoader.loadFile("int_mannequin.brres", heap);
    }
    if (res != NULL) {
        mMannequinRes = nw4r::g3d::ResFile(res);
        return TRUE;
    }
    return FALSE;
}

// 0xD674
nw4r::g3d::ResMdl daFtrCloth_c::getResMdl() {
    if (mMannequinRes.IsValid()) {
        return mMannequinRes.GetResMdl(0);
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0xD694
BOOL daFtrCloth_c::initModel() {
    if (getSwitch()) {
        setSwitch(SWITCH_DRESSED, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_BARE, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0xD714
BOOL daFtrCloth_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0xD738
BOOL daFtrCloth_c::deleteRes() {
    return TRUE;
}

// 0xD740
BOOL daFtrCloth_c::switchOn(daFTR_c *ftr) {
    return dFTR::requestSwitch(ftr, SWITCH_BARE, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0xD754
BOOL daFtrCloth_c::switchOff(daFTR_c *ftr) {
    return dFTR::requestSwitch(ftr, SWITCH_DRESSED, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0xD768
int daFtrCloth_c::getSwitchBit(int sw) {
    static const u8 bits[] = {1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0xD788
BOOL daFtrCloth_c::setSwitch(int sw, int param) {
    static BOOL (daFtrCloth_c::*const init[])() = {&daFtrCloth_c::initDressed, &daFtrCloth_c::initBare};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xD7FC
void daFtrCloth_c::executeSwitch() {
    static void (daFtrCloth_c::*const execute[])() = {&daFtrCloth_c::executeDressed, &daFtrCloth_c::executeBare};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xD83C
BOOL daFtrCloth_c::initDressed() {
    changeSwitch(1, FALSE);
    mMannequinRes.Release();
    mMannequinRes.Bind(nw4r::g3d::ResFile(mRes.getData()));
    mMannequinRes.Bind();
    return TRUE;
}

// 0xD8B0
void daFtrCloth_c::executeDressed() {
    changeSwitch(1, FALSE);
}

// 0xD8C8
BOOL daFtrCloth_c::initBare() {
    changeSwitch(0, FALSE);
    mMannequinRes.Release();
    mMannequinRes.Bind();
    return TRUE;
}

// 0xD928
void daFtrCloth_c::executeBare() {
    changeSwitch(0, FALSE);
}

// 0xD940
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_MYOBJ, daFtrMyobj_c, fProf::FTR_MYOBJ, fProf::DRAW_ORDER::FTR_MYOBJ);
} // namespace dAFtrAct

// 0xD970
daFtrMyobj_c::daFtrMyobj_c() {
    mMannequinRes = nw4r::g3d::ResFile(NULL);
}

// 0xDD28
BOOL daFtrMyobj_c::createRes() {
    if (mLoaded) {
        return TRUE;
    }
    void *res;
    EGG::FrmHeap *heap = getHeap();
    res = NULL;
    dFtrm_c::loader_c *loader = &dFtrm_c::sLoader;
    switch (mpBITM->getKind()) {
    case dItem::KIND_ORG_CLOTH:
        res = loader->loadFile("int_mannequin.brres", heap);
        break;
    case dItem::KIND_ORG_CAP:
        mStandRes = nw4r::g3d::ResFile(loader->getFile("standA.brres"));
        break;
    case dItem::KIND_ORG_EASEL:
        break;
    }
    mMannequinRes = nw4r::g3d::ResFile(res);
    int idx = dItem::seeker_c::get()->findLike(getItem());
    mPlayer = (idx >> DESIGN_SHIFT) & (PLAYER_NUM - 1);
    int designIdx = idx & (DESIGN_NUM - 1);
    mDesignIdx = designIdx;
    dPrivateData_c *player = dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, mPlayer);
    mDesignRes.loadDesignInPlace(&player->mOrgDesigns.mDesigns[designIdx], heap, 32, 128);
    mLoaded = TRUE;
    return TRUE;
}

// 0xDE70
nw4r::g3d::ResMdl daFtrMyobj_c::getResMdl() {
    if (mStandRes.IsValid()) {
        return mStandRes.GetResMdl(0);
    }
    if (mMannequinRes.IsValid()) {
        return mMannequinRes.GetResMdl(0);
    }
    return daFTR_c::getResMdl();
}

// 0xDEA4
BOOL daFtrMyobj_c::initModel() {
    static const char *capBoyNode = "cap_md_boy_mdl";
    static const char *capGirlNode = "cap_md_girl_mdl";
    if (mpBITM != NULL && mpBITM->getKind() == dItem::KIND_ORG_UMB) {
        createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    }
    if (mMannequinRes.IsValid()) {
        mMannequinRes.Release();
        mMannequinRes.Bind(nw4r::g3d::ResFile(mDesignRes.getData()));
        mMannequinRes.Bind();
    } else if (mStandRes.IsValid()) {
        nw4r::g3d::ResMdl mdl = daFTR_c::getResMdl();
        u8 male = isMale();
        nw4r::g3d::ResNode boy = mdl.GetResNode(capBoyNode);
        nw4r::g3d::ResNode girl = mdl.GetResNode(capGirlNode);
        if (!male) {
            if (boy.IsValid()) {
                boy.SetVisibility(false);
            }
            if (girl.IsValid()) {
                girl.SetVisibility(true);
            }
        } else {
            if (boy.IsValid()) {
                boy.SetVisibility(true);
            }
            if (girl.IsValid()) {
                girl.SetVisibility(false);
            }
        }
        nw4r::g3d::ResFile res(mRes.getData());
        res.Release();
        res.Bind(nw4r::g3d::ResFile(mDesignRes.getData()));
        res.Bind();
        mCapMdl.create(mdl, &mAllocator, 0, 1, NULL);
    } else {
        nw4r::g3d::ResFile res(mRes.getData());
        res.Release();
        res.Bind(nw4r::g3d::ResFile(mDesignRes.getData()));
        res.Bind();
    }
    return TRUE;
}

// 0xE0B8
BOOL daFtrMyobj_c::executeMain() {
    return TRUE;
}

// 0xE0C0
void daFtrMyobj_c::calcMdl(const mMtx_c *mtx) {
    daFTR_c::calcMdl(mtx);
    if (mStandRes.IsValid() && mCapMdl.getScn() != NULL) {
        int node = m3d::getNodeID(getResMdl(), "set");
        if (node != -1) {
            mMtx_c nodeMtx;
            mMdl.getNodeWorldMtx(node, &nodeMtx);
            mCapMdl.setLocalMtx(&nodeMtx);
            mCapMdl.calc(false);
        }
    }
}

// 0xE15C
void daFtrMyobj_c::entryMdl() {
    daFTR_c::entryMdl();
    if (mCapMdl.getScn() != NULL && mStandRes.IsValid()) {
        mCapMdl.entry();
    }
}

// 0xE1B4
BOOL daFtrMyobj_c::deleteRes() {
    mCapMdl.remove();
    return mDesignRes.release();
}

// 0xE1F8
u8 daFtrMyobj_c::isMale() const {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getPlayer(mPlayer);
    if (player != NULL) {
        return player->mPID.player.getGender() == GENDER_MALE;
    }
    return true;
}

// 0xE238
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_TAOBJ, daFtrTaobj_c, fProf::FTR_TAOBJ, fProf::DRAW_ORDER::FTR_TAOBJ);
} // namespace dAFtrAct

// 0xE268
daFtrTaobj_c::daFtrTaobj_c() {
    mMannequinRes = nw4r::g3d::ResFile(NULL);
}

// 0xE614
BOOL daFtrTaobj_c::createRes() {
    if (mLoaded) {
        return TRUE;
    }
    EGG::FrmHeap *heap = getHeap();
    int idx = dItem::seeker_c::get()->findLike(getItem());
    dFtrm_c::loader_c *loader = &dFtrm_c::sLoader;
    if (!mpBITM->m_hasRes) {
        mMannequinRes = nw4r::g3d::ResFile(loader->loadFile("int_mannequin.brres", heap));
    }
    mDesignRes.loadDesignInPlace(dPatternCopy::getDesign(dPatternCopy::PATTERN_TAILOR, idx & 7), heap, 32, 128);
    mLoaded = TRUE;
    return TRUE;
}

// 0xE6F0
nw4r::g3d::ResMdl daFtrTaobj_c::getResMdl() {
    if (mMannequinRes.IsValid()) {
        return mMannequinRes.GetResMdl(0);
    }
    return daFTR_c::getResMdl();
}

// 0xE70C
BOOL daFtrTaobj_c::initModel() {
    if (mMannequinRes.IsValid()) {
        mMannequinRes.Release();
        mMannequinRes.Bind(nw4r::g3d::ResFile(mDesignRes.getData()));
        mMannequinRes.Bind();
    } else {
        createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
        nw4r::g3d::ResFile res(mRes.getData());
        res.Release();
        res.Bind(nw4r::g3d::ResFile(mDesignRes.getData()));
        res.Bind();
    }
    return TRUE;
}

// 0xE7CC
BOOL daFtrTaobj_c::executeMain() {
    return TRUE;
}

// 0xE7D4
BOOL daFtrTaobj_c::deleteRes() {
    return mDesignRes.release();
}

// 0xE7DC
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CMPS, daFtrCmps_c, fProf::FTR_CMPS, fProf::DRAW_ORDER::FTR_CMPS);
} // namespace dAFtrAct

// 0xEB90
void daFtrCmps_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {
    int angle = getSwingAngle();
    if (nodeId == mNeedleNode) {
        daFTR_c *parent = dFTR::getList()->get(mParent);
        s16 rot;
        if (parent != NULL) {
            s16 parentAngle = parent->mAngle.y;
            rot = angle - (parentAngle + getParentAngle().mAngle);
        } else {
            rot = angle - mAngle.y;
        }
        mMtx_c mtx;
        anmRes->GetRotTrans(mtx);
        mtx.YrotM(mAng(rot));
        anmRes->SetRotTrans(mtx);
    }
}

// 0xEC74
BOOL daFtrCmps_c::initModel() {
    mNeedleNode = m3d::getNodeID(getResMdl(), "compass");
    mMdl.setCallback(this);
    return TRUE;
}

// 0xECE0
BOOL daFtrCmps_c::executeMain() {
    sLib::chase(&mSwingTarget, 0.0f, 1.875f);
    if (0.0f == mSwingTarget) {
        mSwingPhase = 0;
    } else {
        mSwingPhase += 800;
    }
    sLib::chase(&mSwing, mSwingTarget, 1.875f);
    return TRUE;
}

// 0xED68
BOOL daFtrCmps_c::deleteRes() {
    return TRUE;
}

// 0xED70
void daFtrCmps_c::onRotateStart(BOOL left) {
    mRotateCount = 0;
}

// 0xED7C
void daFtrCmps_c::onRotate(BOOL left) {
    static const u32 swingCount = 8;
    // Unreferenced; known only from .rodata. The owner is a guess that keeps the slot order.
    static const int _AA4 = 1;
    if (++mRotateCount == swingCount) {
        mSwingPhase = 0;
        if (left) {
            mSwingTarget = 45.0f + mSwingTarget * mSwingPhase.sin();
            if (mSwingTarget > 45.0f) {
                mSwingTarget = 45.0f;
            }
        } else {
            mSwingTarget = mSwingTarget * mSwingPhase.sin() - 45.0f;
            if (mSwingTarget < -45.0f) {
                mSwingTarget = -45.0f;
            }
        }
    }
}

// 0xEE4C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_INSTRUMENT, daFtrInstrument_c, fProf::FTR_INSTRUMENT, fProf::DRAW_ORDER::FTR_INSTRUMENT);
} // namespace dAFtrAct

// 0xF200
BOOL daFtrInstrument_c::initModel() {
    mSndInst.setup(mpBITM->m_ftrInstr);
    return TRUE;
}

// 0xF234
BOOL daFtrInstrument_c::executeMain() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, 0, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
    executeIdle();
    return TRUE;
}

// 0xF298
BOOL daFtrInstrument_c::executeIdle() {
    mSndInst.calc(&mCenter);
    return TRUE;
}

// 0xF2C8
BOOL daFtrInstrument_c::deleteRes() {
    mSndInst.stop();
    return TRUE;
}

// 0xF2F0
BOOL daFtrInstrument_c::setSwitch(int sw, int param) {
    daFTR_c::setSwitch(sw, param);
    mSndInst.play();
    return TRUE;
}

// 0xF328
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_BK, daFtrBk_c, fProf::FTR_BK, fProf::DRAW_ORDER::FTR_BK);
} // namespace dAFtrAct

// 0xF6D4
BOOL daFtrBk_c::initModel() {
    setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    return TRUE;
}

// 0xF70C
BOOL daFtrBk_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0xF730
BOOL daFtrBk_c::deleteRes() {
    return TRUE;
}

// 0xF738
BOOL daFtrBk_c::setSwitch(int sw, int param) {
    static BOOL (daFtrBk_c::*const init[])() = {&daFtrBk_c::initOff, &daFtrBk_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xF7AC
void daFtrBk_c::executeSwitch() {
    static void (daFtrBk_c::*const execute[])() = {&daFtrBk_c::executeOff, &daFtrBk_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xF7EC
BOOL daFtrBk_c::initOff() {
    return TRUE;
}

// 0xF7F4
void daFtrBk_c::executeOff() {
    if (isSwitchChanging()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (player != NULL && player->mBells > 0) {
            player->mBells--;
        }
        dFTR::setSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0xF8B8
BOOL daFtrBk_c::initOn() {
    turnOnSound();
    return TRUE;
}

// 0xF8DC
void daFtrBk_c::executeOn() {
    executeOff();
}

// 0xF8E0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_METRO, daFtrMetro_c, fProf::FTR_METRO, fProf::DRAW_ORDER::FTR_METRO);
} // namespace dAFtrAct

// 0xFC8C
BOOL daFtrMetro_c::initModel() {
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    return TRUE;
}

// 0xFD3C
BOOL daFtrMetro_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0xFD60
BOOL daFtrMetro_c::executeIdle() {
    return TRUE;
}

// 0xFD68
BOOL daFtrMetro_c::deleteRes() {
    return TRUE;
}

// 0xFD70
int daFtrMetro_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0xFD90
BOOL daFtrMetro_c::setSwitch(int sw, int param) {
    static BOOL (daFtrMetro_c::*const init[])() = {
        &daFtrMetro_c::initOff,  &daFtrMetro_c::initStart,  &daFtrMetro_c::initOn,
        &daFtrMetro_c::initStop, &daFtrMetro_c::initReturn,
    };
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0xFE04
void daFtrMetro_c::executeSwitch() {
    static void (daFtrMetro_c::*const execute[])() = {
        &daFtrMetro_c::executeOff,  &daFtrMetro_c::executeStart,  &daFtrMetro_c::executeOn,
        &daFtrMetro_c::executeStop, &daFtrMetro_c::executeReturn,
    };
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0xFE44
BOOL daFtrMetro_c::initOff() {
    changeSwitch(0, FALSE);
    changeAnm(1.0f, 0, m3d::REVERSE_ONCE, 0, ANM_ALL);
    return TRUE;
}

// 0xFEA8
void daFtrMetro_c::executeOff() {
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_START, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0xFEF4
BOOL daFtrMetro_c::initStart() {
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    return TRUE;
}

// 0xFF60
void daFtrMetro_c::executeStart() {
    isSyncDisabled();
    changeSwitch(1, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0xFFEC
BOOL daFtrMetro_c::initOn() {
    changeSwitch(1, FALSE);
    setAnmEnd(1.0f, 1, m3d::FORWARD_LOOP, ANM_ALL);
    return TRUE;
}

// 0x1004C
void daFtrMetro_c::executeOn() {
    isSyncDisabled();
    playAnm(-1.0f, ANM_ALL);
    countTick();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_STOP, SWITCH_PARAM_NONE, FALSE, TRUE);
    }
}

// 0x1011C
BOOL daFtrMetro_c::initStop() {
    changeSwitch(0, FALSE);
    turnOffSound();
    changeAnm(1.0f, 1, m3d::FORWARD_ONCE, ANM_FRAME_CURRENT, ANM_ALL);
    return TRUE;
}

// 0x10190
void daFtrMetro_c::executeStop() {
    isSyncDisabled();
    changeSwitch(0, FALSE);
    countTick();
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_RETURN, SWITCH_PARAM_NONE);
    }
}

// 0x10278
BOOL daFtrMetro_c::initReturn() {
    changeSwitch(0, FALSE);
    setAnmEnd(1.0f, 0, m3d::REVERSE_ONCE, ANM_ALL);
    mReturning = TRUE;
    return TRUE;
}

// 0x102E0
void daFtrMetro_c::executeReturn() {
    if (!isSyncDisabled() && mReturning) {
        mReturning = FALSE;
    }
    changeSwitch(0, FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x10388
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_FLOWER, daFtrFlower_c, fProf::FTR_FLOWER, fProf::DRAW_ORDER::FTR_FLOWER);
} // namespace dAFtrAct

// 0x10734
nw4r::g3d::ResMdl daFtrFlower_c::getResMdl() {
    nw4r::g3d::ResFile res(mRes.getData());
    if (res.IsValid()) {
        if (res.GetResMdlNumEntries() >= 2) {
            nw4r::g3d::ResMdl mdl = res.GetResMdl(1);
            if (mdl.IsValid()) {
                return mdl;
            }
        }
        return res.GetResMdl(0);
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0x1079C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CATALOG_CW, daFtrCatalogCW_c, fProf::FTR_CATALOG_CW, fProf::DRAW_ORDER::FTR_CATALOG_CW);
} // namespace dAFtrAct

// 0x10B54
BOOL daFtrCatalogCW_c::initModel() {
    nw4r::g3d::ResFile res(mLoader.getData());
    nw4r::g3d::ResTex tex = res.GetResTex(0);
    nw4r::g3d::ResPltt pltt = res.GetResPltt(0);
    if (mIsWall) {
        nw4r::g3d::ResMdl mdl = getResMdl();
        nw4r::g3d::ResMat mat = mdl.GetResMat("m_dummy_wall");
        mFile.Release();
        mat.BindTexByName(tex, "dummy_wall");
        mat.BindPlttByName(pltt, "pl_dummy_wall");
        mFile.Bind();
    } else {
        nw4r::g3d::ResMdl mdl = getResMdl();
        nw4r::g3d::ResMat mat = mdl.GetResMat("m_dummy_floor");
        mFile.Release();
        mat.BindTexByName(tex, "dummy_floor");
        mat.BindPlttByName(pltt, "pl_dummy_floor");
        mFile.Bind();
    }
    return TRUE;
}

static char l_catalogWallFile[] = "catalog_wall.brres";
static char l_catalogCarpetFile[] = "catalog_carpet.brres";

// 0x10CB0
BOOL daFtrCatalogCW_c::createRes() {
    if (mLoaded) {
        return TRUE;
    }
    dItem::Item item = mMgrItem;
    EGG::FrmHeap *heap = getHeap();
    if (!mFile.IsValid()) {
        switch (dItem::infoBank_c::get()->getBITM(item)->getKind()) {
        case dItem::KIND_WALL:
            mIsWall = TRUE;
            mFile = nw4r::g3d::ResFile(dFtrm_c::sLoader.getFile(l_catalogWallFile));
            break;
        case dItem::KIND_CARPET:
            mIsWall = FALSE;
            mFile = nw4r::g3d::ResFile(dFtrm_c::sLoader.getFile(l_catalogCarpetFile));
            break;
        }
    }
    if (mLoader.loadItem(item, heap)) {
        if (mFile.IsValid()) {
            mLoaded = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

// 0x10DD4
nw4r::g3d::ResMdl daFtrCatalogCW_c::getResMdl() {
    if (mFile.IsValid()) {
        nw4r::g3d::ResMdl mdl = mFile.GetResMdl(0);
        if (mdl.IsValid()) {
            return mdl;
        }
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0x10E18
BOOL daFtrCatalogCW_c::executeMain() {
    return TRUE;
}

// 0x10E20
BOOL daFtrCatalogCW_c::deleteRes() {
    return mLoader.release();
}

// 0x10E28
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_UMB, daFtrUmb_c, fProf::FTR_UMB, fProf::DRAW_ORDER::FTR_UMB);
} // namespace dAFtrAct

// 0x111D4
BOOL daFtrUmb_c::initModel() {
    isAnmNum(1, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    return TRUE;
}

// 0x1122C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_BALLOON, daFtrBalloon_c, fProf::FTR_BALLOON, fProf::DRAW_ORDER::FTR_BALLOON);
} // namespace dAFtrAct

// 0x115D8
nw4r::g3d::ResMdl daFtrBalloon_c::getResMdl() {
    nw4r::g3d::ResFile res(mRes.getData());
    if (res.IsValid()) {
        if (res.GetResMdlNumEntries() >= 2) {
            nw4r::g3d::ResMdl mdl = res.GetResMdl(1);
            if (mdl.IsValid()) {
                return mdl;
            }
        }
        return res.GetResMdl(0);
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0x11640
BOOL daFtrBalloon_c::initModel() {
    nw4r::g3d::ResFile res(mRes.getData());
    nw4r::g3d::ResMdl mdl = getResMdl();
    if (res.GetResAnmChrNumEntries() >= 1) {
        mAnmChr.create(mdl, res.GetResAnmChr(0), &mAllocator, NULL);
        mAnmChr.setFrame(0.0f);
        mAnmChr.mPlayMode = m3d::FORWARD_LOOP;
        mAnmChr.setRate(1.0f);
        mMdl.setAnm(mAnmChr);
    }
    if (res.GetResAnmClrNumEntries() >= 2) {
        f32 color = dItem::seeker_c::get()->findLike(getItem()) & 7;
        mAnmClr.create(mdl, res.GetResAnmClr(1), &mAllocator, NULL, 1);
        mAnmClr.setFrame(color, 0);
        mAnmClr.setPlayMode(m3d::FORWARD_ONCE, 0);
        mAnmClr.setRate(1.0f, 0);
        mMdl.setAnm(mAnmClr);
    }
    return TRUE;
}

// 0x117F0
BOOL daFtrBalloon_c::executeMain() {
    mAnmChr.play();
    return TRUE;
}

// 0x11820
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_WINDMILL, daFtrWindmill_c, fProf::FTR_WINDMILL, fProf::DRAW_ORDER::FTR_WINDMILL);
} // namespace dAFtrAct

// 0x11BCC
BOOL daFtrWindmill_c::initModel() {
    isAnmNum(1, ANM_ALL);
    int idx = dItem::seeker_c::get()->findLike(getItem());
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, (u16)idx, ANM_ALL);
    return TRUE;
}

// 0x11C4C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_ACC, daFtrAcc_c, fProf::FTR_ACC, fProf::DRAW_ORDER::FTR_ACC);
} // namespace dAFtrAct

// 0x12004
BOOL daFtrAcc_c::initModel() {
    nw4r::g3d::ResFile res(mRes.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    if (mdl.IsValid()) {
        if (mAccMdl.create(mdl, &mAllocator, 0, 1, NULL)) {
            if (isSyncDisabled()) {
                int lightObj = dBookLight_c::sLightObjIdx;
                mAccMdl.setLightSetIdxAll(1);
                mAccMdl.setEnvMapRefAll(1, lightObj);
            }
            if (getSwitch()) {
                setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
            } else {
                setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 0x1211C
BOOL daFtrAcc_c::createRes() {
    if (mStandFile.IsValid()) {
        return TRUE;
    }
    const dItem::BITM *bitm = getBITM();
    dFtrm_c::loader_c *loader = &dFtrm_c::sLoader;
    switch (bitm->getHideBone()) {
    case dItem::HIDE_BONE_CAP:
        mStandFile = nw4r::g3d::ResFile(loader->getFile("standA.brres"));
        break;
    case dItem::HIDE_BONE_FULL_MASK:
        mStandFile = nw4r::g3d::ResFile(loader->getFile("standC.brres"));
        break;
    case dItem::HIDE_BONE_HAIR_ACCESSORY:
    default:
        if (dScene::getCurrentScene() == dScene::SCENE_RM_GRACE) {
            mStandFile = nw4r::g3d::ResFile(loader->getFile("standD.brres"));
        } else {
            mStandFile = nw4r::g3d::ResFile(loader->getFile("standB.brres"));
        }
        break;
    }
    return mStandFile.IsValid();
}

// 0x12234
nw4r::g3d::ResMdl daFtrAcc_c::getResMdl() {
    if (mStandFile.IsValid()) {
        nw4r::g3d::ResMdl mdl = mStandFile.GetResMdl(0);
        if (mdl.IsValid()) {
            return mdl;
        }
    }
    return nw4r::g3d::ResMdl(NULL);
}

// 0x12278
BOOL daFtrAcc_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0x1229C
BOOL daFtrAcc_c::deleteRes() {
    mAccMdl.remove();
    return TRUE;
}

// 0x122CC
BOOL daFtrAcc_c::requestOff(daFTR_c *ftr) {
    return dFTR::requestSwitch(ftr, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x122E0
BOOL daFtrAcc_c::requestOn(daFTR_c *ftr) {
    return dFTR::requestSwitch(ftr, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
}

// 0x122F4
void daFtrAcc_c::calcMdl(const mMtx_c *mtx) {
    daFTR_c::calcMdl(mtx);
    if (mStandFile.IsValid() && mAccMdl.getScn() != NULL) {
        int node = m3d::getNodeID(getResMdl(), "set");
        if (node != -1) {
            mMtx_c nodeMtx;
            mMdl.getNodeWorldMtx(node, &nodeMtx);
            mAccMdl.setLocalMtx(&nodeMtx);
            mAccMdl.calc(false);
        }
    }
}

// 0x12390
void daFtrAcc_c::entryMdl() {
    daFTR_c::entryMdl();
    if (getSwitch() || isSyncDisabled()) {
        if (mStandFile.IsValid() && mAccMdl.getScn() != NULL) {
            mAccMdl.entry();
        }
    }
}

// 0x12420
mVec3_c daFtrAcc_c::getScale() const {
    return daFTR_c::getScale() * dFtrm_c::getPushRate();
}

// 0x12460
int daFtrAcc_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x12480
BOOL daFtrAcc_c::setSwitch(int sw, int param) {
    static BOOL (daFtrAcc_c::*const init[])() = {&daFtrAcc_c::initOff, &daFtrAcc_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x124F4
void daFtrAcc_c::executeSwitch() {
    static void (daFtrAcc_c::*const execute[])() = {&daFtrAcc_c::executeOff, &daFtrAcc_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x12534
BOOL daFtrAcc_c::initOff() {
    changeSwitch(0, FALSE);
    return TRUE;
}

// 0x1256C
void daFtrAcc_c::executeOff() {
    changeSwitch(0, FALSE);
}

// 0x12584
BOOL daFtrAcc_c::initOn() {
    changeSwitch(1, FALSE);
    return TRUE;
}

// 0x125BC
void daFtrAcc_c::executeOn() {
    changeSwitch(1, FALSE);
}

// 0x125D4
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_MONITOR, daFtrMonitor_c, fProf::FTR_MONITOR, fProf::DRAW_ORDER::FTR_MONITOR);
} // namespace dAFtrAct

// 0x129AC
BOOL daFtrMonitor_c::initModel() {
    isAnmNum(2, ANM_ALL);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    nw4r::g3d::ResMdl mdl = getResMdl();
    if (getKind() == FTR_KIND_APPEAR) {
        mLamps.init(mdl, FALSE);
        dFTR::setSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (getSwitch() && !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_SHOP_ROOM)) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
        mLamps.init(mdl, TRUE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
        mLamps.init(mdl, FALSE);
    }
    return TRUE;
}

// 0x12AF4
BOOL daFtrMonitor_c::executeMain() {
    executeSwitch();
    mLamps.update();
    return TRUE;
}

// 0x12B2C
BOOL daFtrMonitor_c::deleteRes() {
    return TRUE;
}

// 0x12B34
int daFtrMonitor_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x12B54
BOOL daFtrMonitor_c::setSwitch(int sw, int param) {
    static BOOL (daFtrMonitor_c::*const init[])() = {&daFtrMonitor_c::initOff, &daFtrMonitor_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x12BC8
void daFtrMonitor_c::executeSwitch() {
    static void (daFtrMonitor_c::*const execute[])() = {&daFtrMonitor_c::executeOff, &daFtrMonitor_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x12C08
BOOL daFtrMonitor_c::initOff() {
    turnOffSound();
    changeSwitch(0, FALSE);
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    mTimer.set(AUTO_ON_WAIT);
    return TRUE;
}

// 0x12C80
void daFtrMonitor_c::executeOff() {
    playAnm(-1.0f, ANM_ALL);
    mLamps.set(FALSE, TRUE, FALSE);
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_ON, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0x12D40
BOOL daFtrMonitor_c::initOn() {
    turnOnSound();
    changeSwitch(1, FALSE);
    changeAnm(1.0f, 1, m3d::FORWARD_LOOP, 0, ANM_ALL);
    mTimer.set(AUTO_OFF_WAIT);
    return TRUE;
}

// 0x12DB8
void daFtrMonitor_c::executeOn() {
    playAnm(-1.0f, ANM_ALL);
    mLamps.set(TRUE, TRUE, FALSE);
    updateSound();
    if (isSwitchChanging()) {
        dFTR::requestSwitch(this, SWITCH_OFF, SWITCH_PARAM_NONE, FALSE, TRUE);
    } else if (isSyncDisabled() && mTimer.calc()) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
}

// 0x12E80
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_CICADA, daFtrCicada_c, fProf::FTR_CICADA, fProf::DRAW_ORDER::FTR_CICADA);
} // namespace dAFtrAct

// 0x13234
BOOL daFtrCicada_c::initModel() {
    static const sound_c sounds[] = {
        {dItem::ITEM_IDX_CRICKET, 720, SE_room_koorogi},
        {dItem::ITEM_IDX_BELL_CRICKET, 480, SE_room_suzumushi},
        {dItem::ITEM_IDX_GRASSHOPPER, 300, SE_room_kirigirisu},
        {dItem::ITEM_IDX_FROG, 360, SE_room_kaeru},
    };
    isAnmNum(1, ANM_ALL);
    u32 frameMax = getAnmFrameMax(0, ANM_ALL);
    u16 frame = getKind() == FTR_KIND_APPEAR ? 0 : (u32)cM::rndF(frameMax);
    createAnm(1.0f, 0, m3d::FORWARD_LOOP, frame, ANM_ALL);
    mSoundId = -1;
    for (const sound_c *sound = sounds; sound < sounds + ARRAY_SIZE(sounds); sound++) {
        if (dItem::Item(sound->mItem).isSame(getItem())) {
            mInterval = sound->mInterval;
            mSoundId = sound->mSoundId;
            break;
        }
    }
    return TRUE;
}

// 0x13360
BOOL daFtrCicada_c::executeMain() {
    mSndInsect.setPos(&mCenter);
    playAnm(-1.0f, ANM_ALL);
    if (mInterval != 0) {
        if (mSoundId != -1 && mAnmChr.checkFrame(0.0f) && mTimer == 0) {
            if (mSoundId != -1) {
                mSndInsect.startSound(mSoundId);
            }
            mTimer = mInterval;
        }
    } else if (mSoundId != -1) {
        mSndInsect.holdSound(mSoundId);
    }
    if (mTimer != 0) {
        mTimer--;
    }
    return TRUE;
}

// 0x13430
BOOL daFtrCicada_c::deleteRes() {
    return TRUE;
}

// 0x13438
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_TEL, daFtrTel_c, fProf::FTR_TEL, fProf::DRAW_ORDER::FTR_TEL);
} // namespace dAFtrAct

// 0x137E4
BOOL daFtrTel_c::initModel() {
    if (getKind() == FTR_KIND_APPEAR) {
        changeSwitch(0, FALSE);
    }
    if (getSwitch()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    }
    setTalkState(TALK_STATE_WAIT);
    return TRUE;
}

// 0x1389C
BOOL daFtrTel_c::executeMain() {
    executeTalkState();
    executeSwitch();
    return TRUE;
}

// 0x138D4
BOOL daFtrTel_c::deleteRes() {
    return TRUE;
}

// 0x138DC
void daFtrTel_c::onDemoCmd(int cmd, u8 member) {
    if (!demoCmd(cmd)) {
        switch (cmd) {
        case DEMO_CMD_TALK:
        case DEMO_CMD_SPEAK:
            setTalkState(TALK_STATE_RING);
            break;
        case DEMO_CMD_END:
            setTalkState(TALK_STATE_WAIT);
            break;
        }
    }
}

// 0x13958
int daFtrTel_c::getSwitchBit(int sw) {
    static const u8 bits[] = {0, 1, 1, 0};
    if ((u32)sw < ARRAY_SIZE(bits)) {
        return bits[sw];
    }
    return 0;
}

// 0x13978
BOOL daFtrTel_c::setSwitch(int sw, int param) {
    static BOOL (daFtrTel_c::*const init[])() = {
        &daFtrTel_c::initOff,
        &daFtrTel_c::initPickUp,
        &daFtrTel_c::initOn,
        &daFtrTel_c::initPutDown,
    };
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x139EC
void daFtrTel_c::executeSwitch() {
    static void (daFtrTel_c::*const execute[])() = {
        &daFtrTel_c::executeOff,
        &daFtrTel_c::executePickUp,
        &daFtrTel_c::executeOn,
        &daFtrTel_c::executePutDown,
    };
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x13A2C
BOOL daFtrTel_c::initOff() {
    changeSwitch(0, FALSE);
    return TRUE;
}

// 0x13A64
void daFtrTel_c::executeOff() {}

// 0x13A68
BOOL daFtrTel_c::initPickUp() {
    changeSwitch(1, FALSE);
    turnOnSound();
    return TRUE;
}

// 0x13AB4
void daFtrTel_c::executePickUp() {
    setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
}

// 0x13ACC
BOOL daFtrTel_c::initOn() {
    changeSwitch(1, FALSE);
    return TRUE;
}

// 0x13B04
void daFtrTel_c::executeOn() {}

// 0x13B08
BOOL daFtrTel_c::initPutDown() {
    changeSwitch(0, FALSE);
    turnOffSound();
    return TRUE;
}

// 0x13B54
void daFtrTel_c::executePutDown() {
    setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
}

// 0x13B6C
BOOL daFtrTel_c::setTalkState(int state) {
    static BOOL (daFtrTel_c::*const init[])() = {
        &daFtrTel_c::initWait, &daFtrTel_c::initRing, &daFtrTel_c::initTalk,
        &daFtrTel_c::initTalkEnd, &daFtrTel_c::initHangUp,
    };
    if (state < TALK_STATE_NUM && (this->*init[state])()) {
        mTalkState = state;
        return TRUE;
    }
    return FALSE;
}

// 0x13BD8
void daFtrTel_c::executeTalkState() {
    static void (daFtrTel_c::*const execute[])() = {
        &daFtrTel_c::executeWait, &daFtrTel_c::executeRing, &daFtrTel_c::executeTalk,
        &daFtrTel_c::executeTalkEnd, &daFtrTel_c::executeHangUp,
    };
    if (mTalkState < TALK_STATE_NUM) {
        (this->*execute[mTalkState])();
    }
}

// 0x13C18
BOOL daFtrTel_c::initWait() {
    return TRUE;
}

// 0x13C20
void daFtrTel_c::executeWait() {
    if (!dScene::isCurrentSceneModelRoom() && isSwitchChanging()) {
        dFTR::hold_c *hold = dFTR::getHoldList()->find(this);
        if (hold != NULL) {
            mSpeakPos = hold->mPushPos;
            mSpeakAngle = hold->mPushAngle;
            dDemo::requestSpeak(this, NULL);
        }
    }
    changeSwitch(0, FALSE);
}

// 0x13CCC
BOOL daFtrTel_c::initRing() {
    setSwitch(SWITCH_PICK_UP, SWITCH_PARAM_NONE);
    mTalkTimer = 1;
    return TRUE;
}

// 0x13D18
void daFtrTel_c::executeRing() {
    if (mTalkTimer == 0) {
        setTalkState(TALK_STATE_TALK);
    }
    if (mTalkTimer != 0) {
        mTalkTimer--;
    }
}

// 0x13D68
BOOL daFtrTel_c::initTalk() {
    attachDemoActor(this);
    mpController->requestStart(0);
    if (getFtrFunc() == dItem::FTR_FUNC_PHONE) {
        setMessageLabel("sys_MESS/SYS_Fortune_tel");
        setMessageCode(MSG_FORTUNE_HELLO);
    } else {
        setMessageLabel("npc_SP/NPC_obj");
        setMessageCode(MSG_WISH_TAGS);
    }
    return TRUE;
}

// 0x13E10
void daFtrTel_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setTalkState(TALK_STATE_TALK_END);
        }
    }
}

// 0x13E84
BOOL daFtrTel_c::initTalkEnd() {
    return TRUE;
}

// 0x13E8C
void daFtrTel_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            setTalkState(TALK_STATE_HANG_UP);
        }
    }
}

// 0x13F18
BOOL daFtrTel_c::initHangUp() {
    setSwitch(SWITCH_PUT_DOWN, SWITCH_PARAM_NONE);
    mTalkTimer = 1;
    return TRUE;
}

// 0x13F64
void daFtrTel_c::executeHangUp() {
    if (mTalkTimer == 0) {
        dDemo::finishTalk(this);
    }
    if (mTalkTimer != 0) {
        mTalkTimer--;
    }
}

// 0x13FB0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_ATM, daFtrAtm_c, fProf::FTR_ATM, fProf::DRAW_ORDER::FTR_ATM);
} // namespace dAFtrAct

// 0x1435C
BOOL daFtrAtm_c::initModel() {
    setTalkState(TALK_STATE_WAIT);
    return TRUE;
}

// 0x14384
BOOL daFtrAtm_c::executeMain() {
    executeTalkState();
    return TRUE;
}

// 0x143A8
BOOL daFtrAtm_c::deleteRes() {
    return TRUE;
}

// 0x143B0
void daFtrAtm_c::onDemoCmd(int cmd, u8 member) {
    if (!demoCmd(cmd)) {
        switch (cmd) {
        case DEMO_CMD_SPEAK:
            setTalkState(TALK_STATE_TALK);
            break;
        case DEMO_CMD_END:
            setTalkState(TALK_STATE_WAIT);
            break;
        }
    }
}

// 0x14424
BOOL daFtrAtm_c::setTalkState(int state) {
    static BOOL (daFtrAtm_c::*const init[])() = {
        &daFtrAtm_c::initWait,    &daFtrAtm_c::initTalk,    &daFtrAtm_c::initTalkEnd, &daFtrAtm_c::initSuspend,
        &daFtrAtm_c::initMenu,    &daFtrAtm_c::initMenuEnd, &daFtrAtm_c::initPaidOff, &daFtrAtm_c::initPaidOffEnd,
    };
    if (state < TALK_STATE_NUM && (this->*init[state])()) {
        mTalkState = state;
        return TRUE;
    }
    return FALSE;
}

// 0x14490
void daFtrAtm_c::executeTalkState() {
    static void (daFtrAtm_c::*const execute[])() = {
        &daFtrAtm_c::executeWait,    &daFtrAtm_c::executeTalk,    &daFtrAtm_c::executeTalkEnd,
        &daFtrAtm_c::executeSuspend, &daFtrAtm_c::executeMenu,    &daFtrAtm_c::executeMenuEnd,
        &daFtrAtm_c::executePaidOff, &daFtrAtm_c::executePaidOffEnd,
    };
    if (mTalkState < TALK_STATE_NUM) {
        (this->*execute[mTalkState])();
    }
}

// 0x144D0
BOOL daFtrAtm_c::initWait() {
    return TRUE;
}

// 0x144D8
void daFtrAtm_c::executeWait() {
    if (!dScene::isCurrentSceneModelRoom() && isSwitchChanging()) {
        dDemo::requestSpeak(this, NULL);
    }
}

// 0x14534
BOOL daFtrAtm_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("npc_SP/NPC_obj");
    dScene::isCityScene(dScene::getCurrentScene());
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    mDebt = player->mDebt;
    u16 code = MSG_WELCOME;
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        code = MSG_NOT_A_CLIENT;
    }
    setMessageCode(code);
    mpController->requestStart(0);
    return TRUE;
}

// 0x145E0
void daFtrAtm_c::executeTalk() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setTalkState(TALK_STATE_TALK_END);
        }
    }
}

// 0x14654
BOOL daFtrAtm_c::initTalkEnd() {
    return TRUE;
}

// 0x1465C
void daFtrAtm_c::executeTalkEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            if (dPlayerMgr::Mgr_c::getCurrentPlayer()->mDebt == 0 && mDebt != 0) {
                mDebt = 0;
                setTalkState(TALK_STATE_PAID_OFF);
            } else {
                detachDemoActor(this);
                dDemo::finishTalk(this);
            }
        }
    }
}

// 0x14718
BOOL daFtrAtm_c::initSuspend() {
    mpController->mSuspendReq = 1;
    return TRUE;
}

// 0x1472C
void daFtrAtm_c::executeSuspend() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (comp->isCurrentMethod(&dMsg::Comp_c::executeSuspend)) {
            setTalkState(TALK_STATE_MENU);
        }
    }
}

// 0x147A0
BOOL daFtrAtm_c::initMenu() {
    dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mMenu) {
    case MENU_PAY:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_PAY) != FALSE;
    case MENU_DEPOSIT:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_DEPOSIT) != FALSE;
    case MENU_WITHDRAW:
        return dMenuMng_c::openMenu(dMenuMng_c::MENU_BANK_WITHDRAW) != FALSE;
    }
    return FALSE;
}

// 0x14840
void daFtrAtm_c::executeMenu() {
    if (dMenuMng_c::isMenuEnd()) {
        mpController->setNextMsgCode(mMenuMsgCode);
        mpController->requestStart(0);
        setTalkState(TALK_STATE_MENU_END);
    }
}

// 0x1489C
BOOL daFtrAtm_c::initMenuEnd() {
    return TRUE;
}

// 0x148A4
void daFtrAtm_c::executeMenuEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setTalkState(TALK_STATE_TALK_END);
        }
    }
}

// 0x14918
BOOL daFtrAtm_c::initPaidOff() {
    if (dPlayerMgr::reactDebtPaid()) {
        setMessageLabel("sys_MESS/SYS_Player");
        setMessageCode(MSG_PLAYER_DEBT_PAID);
        return TRUE;
    }
    return FALSE;
}

// 0x14974
void daFtrAtm_c::executePaidOff() {
    if (dPlayerMgr::isReactionFinished()) {
        mpController->requestStart(0);
        setTalkState(TALK_STATE_PAID_OFF_END);
    }
}

// 0x149C0
BOOL daFtrAtm_c::initPaidOffEnd() {
    return TRUE;
}

// 0x149C8
void daFtrAtm_c::executePaidOffEnd() {
    dMsg::Comp_c *comp = mpController;
    if (comp != NULL) {
        if (!comp->isCurrentMethod(&dMsg::Comp_c::executeHidden)) {
            setTalkState(TALK_STATE_TALK_END);
        }
    }
}

// 0x14A3C
void daFtrAtm_c::onMessageStart(int arg) {
    dSvMdlRm_c *modelRoom;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player != NULL) {
        switch (mMessageCode) {
        case MSG_DEBT:
            mpController->setNumber(0, player->mDebt, 12, 9);
            break;
        case MSG_SAVINGS:
            mpController->setNumber(1, player->mSavings, 12, 9);
            break;
        case MSG_WELCOME:
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_PLAYER_HOUSE)) {
                int room;
                int home = dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene());
                dPrivateData_c *owner = dSaveData_c::getRaw()->mHomes.getHomePlayerRaw(home);
                if (owner != NULL) {
                    mpController->setPersonalName(2, &owner->mPID);
                }
            } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_VILLAGER_HOUSE)) {
                int idx = dScene::getSceneAttrIndex(dScene::getCurrentScene(), dScene::SCENE_ATTR_VILLAGER_HOUSE);
                dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(idx);
                if (animal != NULL) {
                    mpController->setAnmPersonalName(2, &animal->mID);
                }
            } else if (dScene::isCurrentSceneModelRoom()) {
                modelRoom = &dSaveData_c::getTown()->mModelRoom;
                const dPersonalID_c *pid = modelRoom->getPlayerID();
                if (pid != NULL) {
                    mpController->setPersonalName(2, pid);
                }
                const dAnmPersonalID_c *aid = modelRoom->getAnimalID();
                if (aid != NULL) {
                    mpController->setAnmPersonalName(2, aid);
                }
            }
            break;
        }
    }
}

// 0x14CA8
void daFtrAtm_c::onMessageEnd(int kind) {
    dSelect::Comp_c *sel = mpController->getSelect();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u32 slot = dPlayerMgr::getCurrentPlayerSlot();
    switch (mMessageCode) {
    case MSG_WELCOME_MAIN_BRANCH:
    case MSG_WELCOME_BRANCH:
    case MSG_WELCOME:
        if (player->mDebt > 0) {
            sel->setSysMessage(0, SEL_MSG_PAY, 0, NULL, 0, 0);
            mAnswers[0] = ANSWER_PAY;
            sel->setSysMessage(1, SEL_MSG_DEPOSIT, 0, NULL, 0, 0);
            mAnswers[1] = ANSWER_DEPOSIT;
            sel->setSysMessage(2, SEL_MSG_WITHDRAW, 0, NULL, 0, 0);
            mAnswers[2] = ANSWER_WITHDRAW;
            if (player->isCardMissing() && slot < PLAYER_NUM) {
                sel->setSysMessage(3, SEL_MSG_REISSUE_CARD, 10, NULL, 0, 0);
                mAnswers[3] = ANSWER_REISSUE_CARD;
                sel->setSysMessage(4, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[4] = ANSWER_QUIT;
                sel->mCancel = 4;
                sel->mNum = 5;
            } else {
                sel->setSysMessage(3, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[3] = ANSWER_QUIT;
                sel->mCancel = 3;
                sel->mNum = 4;
            }
            mpController->openSelect();
        } else {
            sel->setSysMessage(0, SEL_MSG_DEPOSIT, 0, NULL, 0, 0);
            mAnswers[0] = ANSWER_DEPOSIT;
            sel->setSysMessage(1, SEL_MSG_WITHDRAW, 0, NULL, 0, 0);
            mAnswers[1] = ANSWER_WITHDRAW;
            if (player->isCardMissing() && slot < PLAYER_NUM) {
                sel->setSysMessage(2, SEL_MSG_REISSUE_CARD, 10, NULL, 0, 0);
                mAnswers[2] = ANSWER_REISSUE_CARD;
                sel->setSysMessage(3, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[3] = ANSWER_QUIT;
                sel->mCancel = 3;
                sel->mNum = 4;
            } else {
                sel->setSysMessage(2, SEL_MSG_QUIT, 3, NULL, 0, 0);
                mAnswers[2] = ANSWER_QUIT;
                sel->mCancel = 2;
                sel->mNum = 3;
            }
            mpController->openSelect();
        }
        break;
    }
}

// 0x14F64
void daFtrAtm_c::onAnswer(int arg) {
    dMsg::Comp_c *comp = mpController;
    int answer = comp != NULL && comp->getSelect() != NULL ? comp->getSelect()->mCursor : -1;
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    switch (mAnswers[answer]) {
    case ANSWER_PAY:
        if (player != NULL && player->mDebt != 0) {
            mMenuMsgCode = MSG_DEBT;
            mMenu = MENU_PAY;
            setTalkState(TALK_STATE_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_NO_DEBT);
        }
        break;
    case ANSWER_DEPOSIT:
        if (player != NULL && player->mSavings < PRIVATE_SAVINGS_MAX) {
            mMenuMsgCode = MSG_SAVINGS;
            mMenu = MENU_DEPOSIT;
            setTalkState(TALK_STATE_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_SAVINGS_FULL);
        }
        break;
    case ANSWER_WITHDRAW:
        if (player != NULL && player->mSavings > 0) {
            mMenuMsgCode = MSG_SAVINGS;
            mMenu = MENU_WITHDRAW;
            setTalkState(TALK_STATE_SUSPEND);
        } else {
            mpController->setNextMsgCode(MSG_NO_SAVINGS);
        }
        break;
    case ANSWER_REISSUE_CARD:
        if (player != NULL) {
            player->setFlag0(PRIVATE_FLAG0_CARD_REISSUE);
        }
        break;
    }
}

// 0x150E0
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_BARLEY, daFtrBarley_c, fProf::FTR_BARLEY, fProf::DRAW_ORDER::FTR_BARLEY);
} // namespace dAFtrAct

// 0x15494
BOOL daFtrBarley_c::initModel() {
    isAnmNum(1, ANM_ALL);
    changeSwitch(0, FALSE);
    createAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    return TRUE;
}

// 0x15544
BOOL daFtrBarley_c::executeMain() {
    executeSwitch();
    return TRUE;
}

// 0x15568
BOOL daFtrBarley_c::deleteRes() {
    changeSwitch(0, FALSE);
    return TRUE;
}

// 0x155A0
BOOL daFtrBarley_c::isMoving(dDemoActor_c *actor) {
    if (actor != NULL) {
        mVec3_c move = actor->getPos() - actor->getLastPos();
        return EGG::Mathf::sqrt(move.x * move.x + move.z * move.z) > 0.01f;
    }
    return FALSE;
}

// 0x1560C
BOOL daFtrBarley_c::isTrampled() {
    int x, z;
    int actorX, actorZ;
    dBGCF::posToUnit(&x, &z, &mPos);
    for (u32 i = 0; i < dPlayerMgr::PLAYER_INDEX_COUNT; i++) {
        dDemoActor_c *player = dPlayerMgr::getPlayerActor(i);
        if (player != NULL) {
            dBGCF::posToUnit(&actorX, &actorZ, player->getPosP());
            if (x == actorX && z == actorZ && isMoving(player)) {
                return TRUE;
            }
        }
    }
    for (int i = 0; i < dNpcList::countNml(); i++) {
        dDemoActor_c *npc = dNpcList::getNml(i);
        if (npc != NULL) {
            dBGCF::posToUnit(&actorX, &actorZ, npc->getPosP());
            if (x == actorX && z == actorZ && isMoving(npc)) {
                return TRUE;
            }
        }
    }
    for (int i = 0; i < dNpcList::countSp(); i++) {
        dDemoActor_c *npc = dNpcList::getSp(i);
        if (npc != NULL) {
            dBGCF::posToUnit(&actorX, &actorZ, npc->getPosP());
            if (x == actorX && z == actorZ && isMoving(npc)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 0x157A4
BOOL daFtrBarley_c::setSwitch(int sw, int param) {
    static BOOL (daFtrBarley_c::*const init[])() = {&daFtrBarley_c::initOff, &daFtrBarley_c::initOn};
    daFTR_c::setSwitch(sw, param);
    if ((u32)sw < ARRAY_SIZE(init) && (this->*init[sw])()) {
        mSwitchState = sw;
        return TRUE;
    }
    return FALSE;
}

// 0x15818
void daFtrBarley_c::executeSwitch() {
    static void (daFtrBarley_c::*const execute[])() = {&daFtrBarley_c::executeOff, &daFtrBarley_c::executeOn};
    if (mSwitchState < ARRAY_SIZE(execute)) {
        (this->*execute[mSwitchState])();
    }
}

// 0x15858
BOOL daFtrBarley_c::initOff() {
    setAnmEnd(1.0f, 0, m3d::FORWARD_ONCE, ANM_ALL);
    turnOffSound();
    mCounter.set(AUTO_SWAY_WAIT);
    return TRUE;
}

// 0x158B0
void daFtrBarley_c::executeOff() {
    if (isTrampled()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    } else if (isSyncDisabled() && mCounter.calc()) {
        setSwitch(SWITCH_ON, SWITCH_PARAM_NONE);
    }
}

// 0x1594C
BOOL daFtrBarley_c::initOn() {
    changeAnm(1.0f, 0, m3d::FORWARD_ONCE, 0, ANM_ALL);
    turnOnSound();
    mCounter.set(SWAY_TIME);
    return TRUE;
}

// 0x159A8
void daFtrBarley_c::executeOn() {
    toggleSwitch(FALSE);
    if (playAnm(-1.0f, ANM_ALL)) {
        setSwitch(SWITCH_OFF, SWITCH_PARAM_NONE);
    } else {
        updateSound();
    }
}

// 0x15A1C
namespace dAFtrAct {
CUSTOM_BASE_PROFILE(FTR_ROLLER, daFtrRoller_c, fProf::FTR_ROLLER, fProf::DRAW_ORDER::FTR_ROLLER);
} // namespace dAFtrAct

// 0x15DC8
BOOL daFtrRoller_c::onMoveStart(BOOL push) {
    // Unreferenced; known only from .rodata. The owner is a guess that keeps the slot order.
    static const int _A9C = 1;
    turnOnSound();
    return TRUE;
}
