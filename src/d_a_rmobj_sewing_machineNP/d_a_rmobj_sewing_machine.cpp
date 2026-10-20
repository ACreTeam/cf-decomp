#include <game/game/d_a_rmobj_sewing_machine.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_item_sel.hpp>
#include <game/game/d_net.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>

// The sewing machine of the Able Sisters. Functions are in target .text order.

namespace dARmobjSewingMachine {
// .text 0x5C
BASE_PROFILE(A_RMOBJ_SEWING_MACHINE, daRmobjSewingMachine_c);
} // namespace dARmobjSewingMachine

daRmobjSewingMachine_c *daRmobjSewingMachine_c::s_instance;

// .text 0x8C
daRmobjSewingMachine_c::daRmobjSewingMachine_c() : mItem((u16)0) {
}

// .text 0x284
daRmobjSewingMachine_c::~daRmobjSewingMachine_c() {
}

// .text 0x3F8
int daRmobjSewingMachine_c::create() {
    setDemoID(0);
    if (!loadItem()) {
        return NOT_READY;
    }
    bool ok = true;
    ok &= mMachineMdl.create("obj_tailor_saw", 0);
    ok &= mClothMdl.create("obj_tailor_cloth", 0);
    if (ok) {
        nw4r::g3d::ResFile file(mClothMdl.getData());
        file.Release();
        file.Bind(nw4r::g3d::ResFile(mResLoader.getData()));
        file.Bind();
        mMachineMdl.setAnm("obj_tailor_saw", m3d::FORWARD_LOOP, 0.0f);
        mMachineMdl.setRate(2.0f);
        mClothMdl.setAnmChr("obj_tailor_cloth", m3d::FORWARD_LOOP, 0.0f);
        int node = m3d::getNodeID(file.GetResMdl("obj_tailor_cloth"), "needle");
        if (node != -1) {
            mClothMdl.mMdl.calc(false);
            mMtx_c mtx;
            mClothMdl.mMdl.getNodeWorldMtx(node, &mtx);
            mNeedlePos.set(mtx._03, mtx._13, mtx._23);
        }
        s_instance = this;
        initRate();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x57C
int daRmobjSewingMachine_c::execute() {
    if (isNeedleMoving()) {
        mSnd.setPos(&mNeedlePos);
    }
    executeState();
    mMachineMdl.play();
    if (mState != STATE_STOP) {
        mClothMdl.playMdl();
    }
    mClothMdl.mMdl.calc(false);
    return SUCCEEDED;
}

// .text 0x5F0
int daRmobjSewingMachine_c::draw() {
    mMachineMdl.entry();
    mClothMdl.entry();
    return SUCCEEDED;
}

// .text 0x62C
int daRmobjSewingMachine_c::doDelete() {
    if (mMachineMdl.remove() && mClothMdl.remove() && mResLoader.release()) {
        s_instance = NULL;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x69C
void daRmobjSewingMachine_c::initRate() {
    mRate = 2.0f;
}

namespace dARmobjSewingMachine {
static const daRmobjSewingMachine_c::InitFunc l_stateInit[] = {
    &daRmobjSewingMachine_c::initSew,
    &daRmobjSewingMachine_c::initStart,
    &daRmobjSewingMachine_c::initStop,
};
static const daRmobjSewingMachine_c::ExecFunc l_stateExec[] = {
    &daRmobjSewingMachine_c::executeSew,
    &daRmobjSewingMachine_c::executeStart,
    &daRmobjSewingMachine_c::executeStop,
};
} // namespace dARmobjSewingMachine

// .text 0x6AC
BOOL daRmobjSewingMachine_c::changeState(int state) {
    if (state < STATE_NUM &&
        (this->*dARmobjSewingMachine::l_stateInit[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text 0x718
void daRmobjSewingMachine_c::executeState() {
    if (mState < STATE_NUM) {
        (this->*dARmobjSewingMachine::l_stateExec[mState])();
    }
}

// .text 0x758
BOOL daRmobjSewingMachine_c::initSew() {
    return TRUE;
}

// .text 0x760
void daRmobjSewingMachine_c::executeSew() {
    if (isNeedleMoving()) {
        mSnd.holdSound(SE_LEV_mishin);
    }
}

// .text 0x7A0
BOOL daRmobjSewingMachine_c::initStart() {
    return TRUE;
}

// .text 0x7A8
void daRmobjSewingMachine_c::executeStart() {
    if (speedUp()) {
        changeState(STATE_SEW);
    }
    if (isNeedleMoving()) {
        mSnd.holdSound(SE_LEV_mishin);
    }
}

// .text 0x804
BOOL daRmobjSewingMachine_c::initStop() {
    mFrame = mClothMdl.mAnmChr.getFrame();
    return TRUE;
}

// .text 0x83C
void daRmobjSewingMachine_c::executeStop() {
    slowDown();
    mClothMdl.setFrameChr(mFrame);
}

// .text 0x874
BOOL daRmobjSewingMachine_c::loadItem() {
    dItemSel::Range_c range(dItem::KIND_CLOTH, 0);
    if (dNet::isSceneOwner() && mItem.getKind() != dItem::KIND_CLOTH) {
        if (!dItemSel::select(&mItem, 1, &range, 1, &dItemSel::sDefaultFilter,
                              NULL, 0, NULL)) {
            mItem = dItem::Item(dItem::ITEM_IDX_ONE_BALL_SHIRT);
        }
        setRecord(mItem);
    } else if (mItem.getKind() != dItem::KIND_CLOTH) {
        u16 id = getRecord();
        if (dItem::Item(id).getKind() == dItem::KIND_CLOTH) {
            mItem = id;
        } else if (!dItemSel::select(&mItem, 1, &range, 1,
                                     &dItemSel::sDefaultFilter, NULL, 0,
                                     NULL)) {
            mItem = dItem::Item(dItem::ITEM_IDX_ONE_BALL_SHIRT);
        }
    }
    return mResLoader.loadItem(mItem, dHeap::localHeap_p);
}

// .text 0x9CC
void daRmobjSewingMachine_c::slowDown() {
    sLib::addCalc0(&mRate, 0.98f, 0.01f);
    mMachineMdl.setRate(mRate);
}

// .text 0xA18
bool daRmobjSewingMachine_c::speedUp() {
    bool done = sLib::chase(&mRate, 2.0f, 0.01f);
    mMachineMdl.setRate(mRate);
    return done;
}

// .text 0xA74
BOOL daRmobjSewingMachine_c::isNeedleMoving() {
    mFrame = mClothMdl.mAnmChr.getFrame();
    BOOL ret = FALSE;
    if ((47.0f <= mFrame && mFrame <= 150.0f) ||
        (215.0f <= mFrame && mFrame <= 318.0f) ||
        (383.0f <= mFrame && mFrame <= 486.0f) ||
        (551.0f <= mFrame && mFrame <= 654.0f)) {
        ret = TRUE;
    }
    return ret;
}

// .text 0xB50
void daRmobjSewingMachine_c::setRecord(dItem::Item item) {
    dItem::Item record = item;
    dNet::setRecord(NET_REC_SEWING_CLOTH, &record, 0);
}

// .text 0xB84
u16 daRmobjSewingMachine_c::getRecord() {
    u16 id = 0;
    cLib::memCpy(&id, dNet::getRecord(NET_REC_SEWING_CLOTH), sizeof(id));
    return id;
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
