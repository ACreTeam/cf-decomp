// The new day's announcement (REL d_a_growup_msgNP). See
// include/game/game/d_a_growup_msg.hpp.
// REL .text 0x5C..0x3E4, .data 0x0..0x178.
#include <game/game/d_a_growup_msg.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_scene.hpp>
#include <game/mLib/m_fader.hpp>

namespace dAGrowupMsg {

// .text:0x5C: like dScene::getSceneChange.
dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text:0x68
BASE_PROFILE(A_GROWUP_MSG, daGrowupMsg_c)

} // namespace dAGrowupMsg

using namespace dAGrowupMsg;

// .text:0xDC
int daGrowupMsg_c::create() {
    mStep = STEP_WAIT_FADE;
    if (!dDemo::isDayChangeWarp()) {
        return FAILED;
    }
    dDemo::onHoldStart();
    return SUCCEEDED;
}

// .text:0x11C
int daGrowupMsg_c::execute() {
    switch (mStep) {
    case STEP_WAIT_FADE: {
        bool fading = true;
        if (!mFader_c::isStatus(mFaderBase_c::FADE_IN) &&
            !mFader_c::isStatus(mFaderBase_c::FADE_OUT)) {
            fading = false;
        }
        if (fading) {
            break;
        }
        dMsg::Comp_c *comp = dMsg::Comp_c::mInstance;
        comp->attachActor(this);
        setMessageLabel("sys_MESS/SYS_Announcement");
        setMessageCode(MSG_NEW_DAY);
        comp->requestStart(0);
        mStep = STEP_OPEN;
        break;
    }
    case STEP_OPEN: {
        dMsg::Comp_c *comp = getController();
        if (comp != NULL) {
            if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
                mStep = STEP_SHOW;
            }
        }
        break;
    }
    case STEP_SHOW: {
        dMsg::Comp_c *comp = getController();
        if (comp != NULL) {
            if (comp->isState(&dMsg::Comp_c::executeHidden)) {
                // The exit to take after the day change (1 in the city).
                if (dScene::isCityScene(dScene::getCurrentScene())) {
                    getSceneChange()->requestExit(1, 0, 0);
                } else {
                    getSceneChange()->requestExit(0, 0, 0);
                }
                dDemo::offDayChangeWarp();
                mStep = STEP_DONE;
            }
        }
        break;
    }
    case STEP_DONE:
        break;
    }
    return SUCCEEDED;
}

// .text:0x310
int daGrowupMsg_c::doDelete() {
    if (mStep != STEP_WAIT_FADE) {
        dDemo::offHoldStart();
    }
    return SUCCEEDED;
}
