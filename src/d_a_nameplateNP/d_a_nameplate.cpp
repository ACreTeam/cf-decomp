// The nameplate at a villager's house (REL d_a_nameplateNP). See
// include/game/game/d_a_nameplate.hpp.
#include <game/game/d_a_nameplate.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_def.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

namespace dANameplate {
// .text:0x5C
BASE_PROFILE(A_NAMEPLATE, daNameplate_c);
} // namespace dANameplate

// .text:0xF0
int daNameplate_c::create() {
    setDemoID((u16)mParam);
    setMode(MODE_WAIT);
    mObjc.set(this, 16.0f, 41.6f, dObjc::HIT_STR, 0, 0, 0);
    return SUCCEEDED;
}

// .text:0x160
int daNameplate_c::execute() {
    executeMode();
    mObjc.regist();
    return SUCCEEDED;
}

// .text:0x198
int daNameplate_c::draw() {
    return SUCCEEDED;
}

// .text:0x1A0
int daNameplate_c::doDelete() {
    return SUCCEEDED;
}

// .text:0x1A8
bool daNameplate_c::canInteract(dDemoActor_c *actor) {
    clearDemoState();
    if (actor != NULL) {
        mVec3_c dist = getPos() - actor->getPos();
        if (EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z) < 35.2f) {
            if (sLib::distanceAngle(-DEG_TO_ANGLE(180),
                                    actor->getAngle().y.mAngle) <= 0x1100) {
                return true;
            }
        }
    }
    return false;
}

// .text:0x240
void daNameplate_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setMode(MODE_TALK);
        break;
    case DEMO_CMD_END:
        setMode(MODE_WAIT);
        break;
    }
}

// .text:0x268
const daNameplate_c::InitFunc daNameplate_c::sInitFuncs[MODE_NUM] = {
    &daNameplate_c::initWait,
    &daNameplate_c::initTalk,
    &daNameplate_c::initTalkEnd,
};
BOOL daNameplate_c::setMode(int mode) {
    if (mode < MODE_NUM && (this->*sInitFuncs[mode])()) {
        mMode = mode;
        return TRUE;
    }
    return FALSE;
}

// .text:0x2D4
const daNameplate_c::ExecFunc daNameplate_c::sExecuteFuncs[MODE_NUM] = {
    &daNameplate_c::executeWait,
    &daNameplate_c::executeTalk,
    &daNameplate_c::executeTalkEnd,
};
void daNameplate_c::executeMode() {
    if (mMode < MODE_NUM) {
        (this->*sExecuteFuncs[mMode])();
    }
}

// .text:0x314
BOOL daNameplate_c::initWait() {
    return TRUE;
}

// .text:0x31C
void daNameplate_c::executeWait() {
}

// .text:0x320
BOOL daNameplate_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("sys_MESS/SYS_Board");
    setMessageCode(MSG_HOUSE);
    getController()->requestStart(0);
    int idx = mParam;
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *animal = town->mAnimals.mTown.getAnimal(idx);
    if (animal != NULL && animal->mID.isValid()) {
        getController()->setAnmPersonalName(0, &animal->mID);
    } else {
        dHmnName::Word_c name;
        dItem::Item item((u16)SP_NPC_SHOP_MASTER);
        dNpc::getNpcName(&name, &item, LANGUAGE_NUM);
        getController()->setWord(0, &name);
    }
    return TRUE;
}

// .text:0x41C
void daNameplate_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setMode(MODE_TALK_END);
        }
    }
}

// .text:0x490
BOOL daNameplate_c::initTalkEnd() {
    return TRUE;
}

// .text:0x498
void daNameplate_c::executeTalkEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}
