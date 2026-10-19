// A museum exhibit's plaque (REL d_a_nameplate_museumNP). See
// include/game/game/d_a_nameplate_museum.hpp.
#include <game/game/d_a_nameplate_museum.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>

namespace dANameplateMuseum {
// .text:0x5C
BASE_PROFILE(A_NAMEPLATE_MUSEUM, daNameplateMuseum_c);
} // namespace dANameplateMuseum

u16 daNameplateMuseum_c::sNum;

// The demo ID: the kind and the plaque's number.
union DemoID_u {
    struct {
        u8 mKind : 8;
        u8 mNo : 8;
    } mBits;
    u16 mValue;
};

// .text:0xF8
int daNameplateMuseum_c::create() {
    if (getPanel()->mName != NULL) {
        if (dBgUtil::isBgiReady()) {
            mVec3_c ofs = mPos;
            dBgUtil::getNodePos(&mPos, getPanel()->mName, -1, -1);
            mPos += ofs;
            mLastPos = mPos;
        } else {
            return NOT_READY;
        }
    }
    mCreated = TRUE;
    mKind = getPanel()->mKind;
    mSolid = getPanel()->mSolid;
    mCount = getPanel()->mCount;
    mIds = getPanel()->mIds;
    mMsgCode = getPanel()->mMsgCode;
    DemoID_u id;
    id.mBits.mKind = mKind;
    id.mBits.mNo = sNum;
    setDemoID(id.mValue);
    setMode(MODE_WAIT);
    if (mSolid) {
        mObjc.set(this, 12.8f, 25.6f, dObjc::HIT_RMOBJ, dObjc::MASK_DEFAULT,
                  0xFF, 0);
    } else {
        mObjc.set(this, 12.8f, 25.6f, dObjc::HIT_RMOBJ, 0, 0, 0);
    }
    sNum++;
    return SUCCEEDED;
}

// .text:0x29C
int daNameplateMuseum_c::execute() {
    executeMode();
    mObjc.regist();
    return SUCCEEDED;
}

// .text:0x2D4
int daNameplateMuseum_c::draw() {
    return SUCCEEDED;
}

// .text:0x2DC
int daNameplateMuseum_c::doDelete() {
    if (mCreated) {
        sNum--;
    }
    return SUCCEEDED;
}

static inline f32 getDistXZ(const mVec3_c &a, const mVec3_c &b) {
    mVec3_c dist = a - b;
    return EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z);
}
// .text:0x300
bool daNameplateMuseum_c::canInteract(dDemoActor_c *actor) {
    static const f32 cTalkRange = 35.2f;
    clearDemoState();
    if (actor != NULL) {
        if (getDistXZ(getPos(), actor->getPos()) < cTalkRange) {
            mVec3_c pos = actor->getPos();
            mVec3_c toActor = pos - getPos();
            mVec3_c front(0.0f, 0.0f, 1.0f);
            front.rotY(mAngle.y);
            if (sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                                    actor->getAngle().y.mAngle) <=
                DEG_TO_ANGLE(45)) {
                if (toActor.dot(front) > 0.0f) {
                    return true;
                }
            }
        }
    }
    return false;
}

// .text:0x438
void daNameplateMuseum_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_TALK:
        setMode(MODE_TALK);
        break;
    case DEMO_CMD_END:
        setMode(MODE_WAIT);
        break;
    }
}

// .text:0x460
int daNameplateMuseum_c::getItemBase() const {
    switch (mKind) {
    case KIND_INSECT:
        return dItem::ITEM_IDX_COMMON_BUTTERFLY;
    case KIND_FISH:
        return dItem::ITEM_IDX_BITTERLING;
    case KIND_PICTURE:
        return dItem::ITEM_IDX_DYNAMIC_PAINTING;
    case KIND_FOSSIL:
        return dItem::ITEM_IDX_AMBER;
    }
    return dItem::ITEM_IDX_DUMMY;
}

// .text:0x4B4
BOOL daNameplateMuseum_c::isAnyDonated() const {
    for (u32 i = 0; i < mCount; i++) {
        if (isDonated(i)) {
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x554
BOOL daNameplateMuseum_c::isAllDonated() const {
    for (u32 i = 0; i < mCount; i++) {
        if (!isDonated(i)) {
            return FALSE;
        }
    }
    return TRUE;
}

// .text:0x5F4
void daNameplateMuseum_c::findFirstDonated() {
    mIndex = 0;
    for (u32 i = 0; i < mCount; i++) {
        if (isDonated(i)) {
            mIndex = i;
            return;
        }
    }
    mIndex = mCount;
}

// .text:0x69C
void daNameplateMuseum_c::findNextDonated() {
    for (u32 i = mIndex + 1; i < mCount; i++) {
        if (isDonated(i)) {
            mIndex = i;
            return;
        }
    }
    mIndex = mCount;
}

// .text:0x740
u32 daNameplateMuseum_c::countDonated() const {
    u32 num = 0;
    for (u32 i = mIndex; i < mCount; i++) {
        if (isDonated(i)) {
            num++;
        }
    }
    return num;
}

// .text:0x7E8
void daNameplateMuseum_c::onMessageStart(int arg) {
    if (mKind == KIND_INSECT || mKind == KIND_FISH) {
        u32 num = 0;
        switch (getMessageCode()) {
        case MSG_ITEMS_1:
            num = 1;
            break;
        case MSG_ITEMS_2:
            num = 2;
            break;
        case MSG_ITEMS_3:
            num = 3;
            break;
        case MSG_ITEMS_4:
            num = 4;
            break;
        }
        for (u32 i = 0; i < num; i++) {
            dItem::Item item(getItemBase(), mIds[mIndex], FALSE);
            getController()->setItemName(i + 1, item);
            findNextDonated();
        }
    } else if (mIndex < mCount) {
        dItem::Item item(getItemBase(), mIds[mIndex], FALSE);
        dHmnName::Word_c name;
        getMuseum()->setDonorWord(&name, item);
        getController()->setWord(0, &name);
        getController()->setItemName(1, item);
        if (mKind != KIND_FOSSIL) {
            findNextDonated();
        }
    }
}

// .text:0x960
void daNameplateMuseum_c::onMessageEnd(int kind) {
    if (mKind == KIND_FOSSIL) {
        if (getMessageCode() != MSG_COCKROACH_WARD && isAllDonated() &&
            mMsgCode == getMessageCode()) {
            getController()->setNextMsgCode(getMessageCode() + 1);
        }
    } else if (countDonated() != 0) {
        getController()->getSelect()->mNum = 2;
        getController()->getSelect()->setSysMessage(0, SEL_MSG_READ_MORE, 0,
                                                    NULL, 0, 0);
        getController()->getSelect()->setSysMessage(1, SEL_MSG_ENOUGH, 0, NULL,
                                                    0, 0);
        getController()->getSelect()->mCancel = 1;
        getController()->openSelect();
    }
}

// .text:0xA44
void daNameplateMuseum_c::onAnswer(int arg) {
    if (getController()->getAnswer() == 0) {
        dItem::Item item(getItemBase(), mIds[mIndex], FALSE);
        if (mKind == KIND_INSECT || mKind == KIND_FISH) {
            u32 num = countDonated();
            if (num > ITEMS_PER_PAGE) {
                num = ITEMS_PER_PAGE;
            }
            getController()->setNextMsgCode(MSG_ITEMS_1 + ((num - 1) & 3));
        } else if (getMuseum()->getDonorKind(item) !=
                   MUSEUM_DONATED_BY_DELETED) {
            getController()->setNextMsgCode(MSG_DONATED_BY);
        } else {
            getController()->setNextMsgCode(MSG_DONATED_LONG_AGO);
        }
    }
}

// .text:0xB14
const daNameplateMuseum_c::InitFunc daNameplateMuseum_c::sInitFuncs[MODE_NUM] =
    {
        &daNameplateMuseum_c::initWait,
        &daNameplateMuseum_c::initTalk,
        &daNameplateMuseum_c::initTalkEnd,
};
BOOL daNameplateMuseum_c::setMode(int mode) {
    if (mode < MODE_NUM && (this->*sInitFuncs[mode])()) {
        mMode = mode;
        return TRUE;
    }
    return FALSE;
}

// .text:0xB80
const daNameplateMuseum_c::ExecFunc
    daNameplateMuseum_c::sExecuteFuncs[MODE_NUM] = {
        &daNameplateMuseum_c::executeWait,
        &daNameplateMuseum_c::executeTalk,
        &daNameplateMuseum_c::executeTalkEnd,
};
void daNameplateMuseum_c::executeMode() {
    if (mMode < MODE_NUM) {
        (this->*sExecuteFuncs[mMode])();
    }
}

// .text:0xBC0
BOOL daNameplateMuseum_c::initWait() {
    return TRUE;
}

// .text:0xBC8
void daNameplateMuseum_c::executeWait() {
}

// .text:0xBCC
BOOL daNameplateMuseum_c::initTalk() {
    attachDemoActor(this);
    setMessageLabel("sys_MESS/SYS_Museum");
    getController()->requestStart(0);
    findFirstDonated();
    if (!isAnyDonated()) {
        setMessageCode(MSG_THANK_YOU);
    } else if (mKind == KIND_FOSSIL) {
        if (mCount == 1) {
            setMessageCode(MSG_ITEMS_1);
        } else {
            setMessageCode(mMsgCode);
        }
    } else if (mKind == KIND_INSECT || mKind == KIND_FISH) {
        u32 num = countDonated();
        if (num > ITEMS_PER_PAGE) {
            num = ITEMS_PER_PAGE;
        }
        setMessageCode(MSG_ITEMS_1 + ((num - 1) & 3));
    } else {
        dItem::Item item(getItemBase(), mIds[mIndex], FALSE);
        if (getMuseum()->getDonorKind(item) != MUSEUM_DONATED_BY_DELETED) {
            setMessageCode(MSG_DONATED_BY);
        } else {
            setMessageCode(MSG_DONATED_LONG_AGO);
        }
    }
    return TRUE;
}

// .text:0xD28
void daNameplateMuseum_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setMode(MODE_TALK_END);
        }
    }
}

// .text:0xD9C
BOOL daNameplateMuseum_c::initTalkEnd() {
    return TRUE;
}

// .text:0xDA4
void daNameplateMuseum_c::executeTalkEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            dDemo::finishTalk(this);
        }
    }
}
