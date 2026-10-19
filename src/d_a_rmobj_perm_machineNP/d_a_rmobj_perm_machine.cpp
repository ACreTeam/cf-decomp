#include <game/game/d_a_rmobj_perm_machine.hpp>
#include <game/framework/f_profile.hpp>
#include <game/snd/snd_id.hpp>

// The perm machine at Shampoodle. Functions are in target .text order.

daRmobjPermMachine_c *daRmobjPermMachine_c::m_instance;

// The machine's debug parameters (RTTI "daPermMachineHIO_c"; nothing reads
// them in this build).
class daPermMachineHIO_c {
  public:
    daPermMachineHIO_c() {
        _04 = 0;
        _05 = 0;
    }
    virtual ~daPermMachineHIO_c() {
    }

    /* 0x4 */ u8 _04;
    /* 0x5 */ u8 _05;
}; // size 0x8

namespace dARmobjPermMachine {
static daPermMachineHIO_c l_HIO;

// .text 0xCC
BASE_PROFILE(A_RMOBJ_PERM_MACHINE, daRmobjPermMachine_c);
} // namespace dARmobjPermMachine

// .text 0xFC
daRmobjPermMachine_c::daRmobjPermMachine_c()
    : mPermDone(false), mSpecialMakeDone(false) {
}

// .text 0x410
daRmobjPermMachine_c::~daRmobjPermMachine_c() {
}

// .text 0x54C
int daRmobjPermMachine_c::create() {
    setDemoID(0);
    if (mModel.create("obj_tw_b_machine", 0)) {
        nw4r::g3d::ResFile res(mModel.getData());
        setStep(STEP_WAIT);
        mModel.setAnmChr("obj_b_machine00", m3d::FORWARD_ONCE, 0.0f);
        mModel.setFrameChr(0.0f);
        int node = m3d::getNodeID(res.GetResMdl("obj_b_machine"), "body");
        if (node != -1) {
            mModel.mMdl.calc(false);
            mMtx_c mtx;
            mModel.mMdl.getNodeWorldMtx(node, &mtx);
            mSoundPos.set(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
        }
        static mVec3_c pos(304.0f, 0.0f, 374.4f);
        dBGCF::getMvbgList()->add(25.6f, 16.0f, 32.0f, &mMvbg, pos);
        m_instance = this;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text 0x6D0
int daRmobjPermMachine_c::execute() {
    executeStep();
    mSnd.setPos(&mSoundPos);
    return SUCCEEDED;
}

// .text 0x70C
int daRmobjPermMachine_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// .text 0x734
int daRmobjPermMachine_c::doDelete() {
    if (mModel.remove()) {
        dBGCF::getMvbgList()->remove(&mMvbg);
        m_instance = NULL;
        return SUCCEEDED;
    }
    return NOT_READY;
}

typedef BOOL (daRmobjPermMachine_c::*initStep_t)();
typedef void (daRmobjPermMachine_c::*executeStep_t)();

namespace dARmobjPermMachine {
static const initStep_t l_initSteps[daRmobjPermMachine_c::STEP_NUM] = {
    &daRmobjPermMachine_c::initWait,
    &daRmobjPermMachine_c::initPerm,
    &daRmobjPermMachine_c::initSpecialMake,
};
static const executeStep_t l_executeSteps[daRmobjPermMachine_c::STEP_NUM] = {
    &daRmobjPermMachine_c::executeWait,
    &daRmobjPermMachine_c::executePerm,
    &daRmobjPermMachine_c::executeSpecialMake,
};
} // namespace dARmobjPermMachine

// .text 0x790
BOOL daRmobjPermMachine_c::setStep(int step) {
    if (step < STEP_NUM && (this->*dARmobjPermMachine::l_initSteps[step])()) {
        mStep = step;
        return TRUE;
    }
    return FALSE;
}

// .text 0x7FC
void daRmobjPermMachine_c::executeStep() {
    if (mStep < STEP_NUM) {
        (this->*dARmobjPermMachine::l_executeSteps[mStep])();
    }
}

// .text 0x83C
BOOL daRmobjPermMachine_c::initWait() {
    return TRUE;
}

// .text 0x844
void daRmobjPermMachine_c::executeWait() {
}

// .text 0x848
BOOL daRmobjPermMachine_c::initPerm() {
    mPermDone = false;
    if (mModel.setAnmChr(ANM_PERM, m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_PermaMachine);
    }
    return TRUE;
}

// .text 0x8A8
void daRmobjPermMachine_c::executePerm() {
    if (mModel.mAnmChr.checkFrame(135.0f)) {
        mSnd.startSound(SE_PermaMachineEnd);
    } else if (mModel.mAnmChr.checkFrame(155.0f)) {
        mVec3_c *pos = mModel.getNodePos("head");
        dEffect::create("af_obj_pmachine_a_smk", pos, NULL, NULL);
    }
    mModel.playMdl();
    mModel.mMdl.calc(false);
    if (mModel.isStopChr()) {
        mPermDone = true;
        setStep(STEP_WAIT);
    }
}

// .text 0x970
BOOL daRmobjPermMachine_c::initSpecialMake() {
    mSpecialMakeDone = false;
    if (mModel.setAnmChr(ANM_SPECIAL_MAKE, m3d::FORWARD_ONCE, 0.0f)) {
        mSnd.startSound(SE_SpecialMakeMachine);
    }
    mMtx_c *mtx = (mMtx_c *)mModel.getNodeMtx("tuno");
    mEffect.create("af_obj_pmachine_b_elec", mtx);
    return TRUE;
}

// .text 0x9F4
void daRmobjPermMachine_c::executeSpecialMake() {
    mEffect.followEffect((mMtx_c *)mModel.getNodeMtx("tuno"));
    if (mModel.mAnmChr.checkFrame(155.0f)) {
        mSnd.startSound(SE_SpecialMakeMachineEnd);
    } else if (mModel.mAnmChr.checkFrame(175.0f)) {
        mVec3_c *pos = mModel.getNodePos("head");
        dEffect::create("af_obj_pmachine_b_smk", pos, NULL, NULL);
    }
    mModel.playMdl();
    mModel.mMdl.calc(false);
    if (mModel.isStopChr()) {
        mSpecialMakeDone = true;
        setStep(STEP_WAIT);
    }
}

// .text 0xADC
u8 daRmobjPermMachine_c::isPermDone() {
    return mPermDone;
}

// .text 0xAE4
u8 daRmobjPermMachine_c::isSpecialMakeDone() {
    return mSpecialMakeDone;
}

// .text 0xAEC
void daRmobjPermMachine_c::startSwitchSound() {
    mSnd.startSound(SE_SwitchOn);
}

#ifdef MUST_MATCH
// Matching-only emission-order workaround; no evidence of an original helper.
// The qualified call queues anmChr::getType before anmVis::getType.
// MWCC omits the unused body. See notes/TUs/d_a_rmobj_audit.txt.
static void orderRmobjWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
