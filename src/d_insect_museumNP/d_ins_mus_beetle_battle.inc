// dInsMusBeetleBattle_c: the hercules beetle and golden stag fighting on the museum's tree stump.

namespace dInsectMuseum {

dInsMusBattleBeetleParam_c sBattleBeetleParam[dInsMusBeetleBattle_c::BEETLE_NUM] = {
    {{-7.0f, 14.0f, 5.0f}, 0.007f, 0.003f, 0.99f, 1.0f, 0.94f, 0.8f, 0.45f, 7.0f, 8.0f}, // hercules beetle
    {{10.0f, 14.0f, -6.0f}, 0.007f, 0.003f, 0.99f, 0.5f, 0.94f, 1.0f, 0.45f, 4.0f, 7.0f}, // golden stag
};

dInsMusBeetleBattle_c sBeetleBattle;

} // namespace dInsectMuseum

// 0x14EFC
dInsMusBeetleBattle_c::beetle_c::beetle_c(int idx)
    : mStateId(STATE_FACE), mState(NULL), mHomePos(0.0f, 0.0f, 0.0f), mPos(0.0f, 0.0f, 0.0f), mAngle(0, 0, 0),
      mTimer(0), mIdx(idx), mIsHeld(false), mIsFlipped(false), mSpeed(0.0f, 0.0f, 0.0f), mReturnPos(0.0f, 0.0f, 0.0f),
      mRadius(0.0f), mTurnAngle(0), mHitGround(false), mCrushed(false), mHoldDist(0.0f) {}

// 0x1502C
dInsMusBeetleBattle_c::beetle_c::~beetle_c() {}

// 0x150C0
void dInsMusBeetleBattle_c::beetle_c::init(const nw4r::g3d::ResFile &res, mAllocator_c *allocator) {
    mAxis.setIdentity();
    mAngle.set(0, 0, 0);
    PSMTXIdentity(mMtx);
    mIsHeld = false;
    mIsFlipped = false;
    mHitGround = false;
    createModel(res, allocator);
    dBgUtil::getNodePos(&mPos, "F2_kirikabu00", -1, -1);
    mPos.x += getParam().mOffset.x;
    mPos.y += getParam().mOffset.y;
    mPos.z += getParam().mOffset.z;
    mHomePos = mPos;
    if (mIdx == BEETLE_HERCULES) {
        mAxis.rotateY(DEG_TO_ANGLE(180));
    }
    mRadius = 8.0f;
}

// 0x151D8
void dInsMusBeetleBattle_c::beetle_c::execute() {
    (this->*mState)();
    mTimer++;
    mMdl.play();
    mSnd.setPos(&mPos);
}

// 0x15238
void dInsMusBeetleBattle_c::beetle_c::draw() {
    mMdl.calc(false);
    calcMtx();
    mMdl.entry();
    mVec3_c shadowPos;
    shadowPos.x = mMtx.m[0][3];
    shadowPos.y = mHomePos.y;
    shadowPos.z = mMtx.m[2][3];
    f32 size = getParam().mShadowSize;
    mAng angle(0);
    dShadow::requestRound(&shadowPos, dShadow::DRAW_QUAD, &angle, size, 1.0f, 0.5f, 0.0f, 1.0f);
}

// 0x152EC
void dInsMusBeetleBattle_c::beetle_c::remove() {
    mMdl.remove();
    mAnmChr.remove();
    mAnmVis.remove();
}

// 0x1534C
void dInsMusBeetleBattle_c::beetle_c::createModel(const nw4r::g3d::ResFile &res, mAllocator_c *allocator) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, allocator, nw4r::g3d::ScnMdl::ANM_VIS, 1, NULL);
    mAnmChrWait = res.GetResAnmChr("wait");
    mAnmChrStop = res.GetResAnmChr("stop");
    if (mAnmChrWait.IsValid()) {
        mAnmChr.create(mdl, mAnmChrWait, allocator, NULL);
    }
    mAnmVisWait = res.GetResAnmVis("wait");
    mAnmVisStop = res.GetResAnmVis("stop");
    if (mAnmVisWait.IsValid()) {
        mAnmVis.create(mdl, mAnmVisWait, allocator, NULL);
    }
    mMdl.setAnm(mAnmChr);
    mMdl.setAnm(mAnmVis);
}

// 0x154A4
void dInsMusBeetleBattle_c::beetle_c::setAnmChr(const nw4r::g3d::ResAnmChr &anm, f32 blend) {
    if (anm.IsValid()) {
        mAnmChr.setAnm(mMdl, anm, m3d::FORWARD_LOOP);
        mMdl.setAnm(mAnmChr, blend);
    }
}

// 0x1550C
void dInsMusBeetleBattle_c::beetle_c::setAnmVis(const nw4r::g3d::ResAnmVis &anm) {
    if (anm.IsValid()) {
        mAnmVis.setAnm(mMdl, anm, m3d::FORWARD_LOOP);
        mMdl.setAnm(mAnmVis);
    }
}

// 0x15570
void dInsMusBeetleBattle_c::beetle_c::setAnmRate(f32 rate) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setRate(rate);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setRate(rate);
    }
}

// 0x155DC
void dInsMusBeetleBattle_c::beetle_c::setAnmFrame(f32 frame) {
    if (mAnmChr.IsBound() == true) {
        mAnmChr.setFrame(frame);
    }
    if (mAnmVis.IsBound() == true) {
        mAnmVis.setFrame(frame);
    }
}

// 0x15648
void dInsMusBeetleBattle_c::beetle_c::calcMtx() {
    mMtx_c mtx;
    mMtx_c rot;
    mMtx_c trans;
    PSMTXIdentity(mtx);
    PSMTXIdentity(rot);
    PSMTXIdentity(trans);
    if (isAttacker() == true) {
        rot = getAxisMtx();
        PSMTXTrans(trans, mPos.x, mPos.y, mPos.z);
        PSMTXConcat(mtx, trans, mtx);
        PSMTXConcat(mtx, rot, mtx);
    } else {
        mVec3_c pos = mPos;
        if (mIsHeld == true) {
            mMtx_c head = getInstance()->getAttackerHeadMtx();
            mAxis.set(head);
            mAxis.rotateY(DEG_TO_ANGLE(180));
            f32 frame = getOpponentAnmFrame();
            // identical branches here and below, as in the target
            if (isHercules() == true) {
                if (frame <= 108.0f) {
                    mVec3_c up(0.0f, 1.0f, 0.0f);
                    mAxis.setUp(up);
                }
            } else {
                if (frame <= 108.0f) {
                    mVec3_c up(0.0f, 1.0f, 0.0f);
                    mAxis.setUp(up);
                }
            }
            f32 x = head.m[0][3];
            f32 y = head.m[1][3];
            f32 z = head.m[2][3];
            pos.set(x, y, z);
            mVec3_c offset(0.0f, 0.0f, 0.0f);
            if (isHercules() == true) {
                offset.set(0.0f, -3.0f, -mHoldDist);
            } else {
                offset.set(0.0f, -3.0f, -mHoldDist);
            }
            mMtx_c axisMtx = getAxisMtx();
            PSMTXMultVec(axisMtx, offset, offset);
            pos += offset;
        }
        rot = getAxisMtx();
        PSMTXTrans(trans, pos.x, pos.y, pos.z);
        PSMTXConcat(mtx, trans, mtx);
        PSMTXConcat(mtx, rot, mtx);
    }
    mMtx = mtx;
    mMdl.setLocalMtx(&mtx);
}

// 0x159FC
f32 dInsMusBeetleBattle_c::beetle_c::getAnmFrame() {
    return mAnmChr.getFrame();
}

// 0x15A04
void dInsMusBeetleBattle_c::beetle_c::initFace() {
    mTimer = 0;
    f32 frame = getAnmFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.7f);
    setAnmFrame(frame);
    mState = &beetle_c::stateFace;
    mCrushed = false;
    mStateId = STATE_FACE;
}

// 0x15AB4
void dInsMusBeetleBattle_c::beetle_c::initCharge() {
    mTimer = 0;
    f32 frame = getAnmFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(frame);
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mState = &beetle_c::stateCharge;
    mStateId = STATE_CHARGE;
}

// 0x15B74
void dInsMusBeetleBattle_c::beetle_c::initBack() {
    mTimer = 0;
    f32 frame = getAnmFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(1.0f);
    setAnmFrame(frame);
    mState = &beetle_c::stateBack;
    mStateId = STATE_BACK;
}

// 0x15C1C
void dInsMusBeetleBattle_c::beetle_c::initPush() {
    mTimer = 0;
    f32 frame = getAnmFrame();
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.5f);
    setAnmFrame(frame);
    mState = &beetle_c::statePush;
    mStateId = STATE_PUSH;
}

// 0x15CC4
void dInsMusBeetleBattle_c::beetle_c::initLift() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    mTimer = 0;
    f32 frame = getAnmFrame();
    if (isAttacker() == true) {
        setAnmChr(mAnmChrStop, 30.0f);
        setAnmVis(mAnmVisStop);
        setAnmRate(param.mLiftAnmRate);
    } else {
        setAnmRate(4.0f);
    }
    setAnmFrame(frame);
    mState = &beetle_c::stateLift;
    if (!isAttacker()) {
        dInsMusBeetleBattle_c *battle = getInstance();
        mMtx_c mtx;
        battle->getAttackerMtx(&mtx);
        PSMTXInverse(mtx, mtx);
        PSMTXConcat(mtx, mMtx, mtx);
        battle->setHeldMtx(mtx);
        mAxis.setIdentity();
    }
    mIsHeld = true;
    mStateId = STATE_LIFT;
    mMtx_c head = getInstance()->getAttackerHeadMtx();
    f32 x = head.m[0][3];
    f32 y = head.m[1][3];
    f32 z = head.m[2][3];
    mVec3_c headPos(x, y, z);
    mHoldDist = headPos.xzDistTo(mPos);
}

// 0x15EC4
void dInsMusBeetleBattle_c::beetle_c::initThrow() {
    mTimer = 0;
    f32 frame = getAnmFrame();
    if (isAttacker() == true) {
        setAnmChr(mAnmChrStop, 0.0f);
        setAnmVis(mAnmVisStop);
        setAnmRate(1.0f);
    } else {
        setAnmRate(2.0f);
    }
    setAnmFrame(frame);
    mState = &beetle_c::stateThrow;
    mIsHeld = false;
    mHitGround = false;
    mStateId = STATE_THROW;
    if (!isAttacker()) {
        mMtx_c head = getInstance()->getAttackerHeadMtx();
        f32 x = head.m[0][3];
        mPos.x = x;
        f32 y = head.m[1][3];
        mPos.y = y;
        f32 z = head.m[2][3];
        mPos.z = z;
        mVec3_c offset(0.0f, 0.0f, -mHoldDist);
        mMtx_c axisMtx = getAxisMtx();
        PSMTXMultVec(axisMtx, offset, offset);
        mPos += offset;
        mVec3_c vel = getOpponentAxis().getSide();
        vel *= -0.6f;
        vel.y = 2.0f;
        mSpeed = vel;
    }
    mSnd.startSound(SE_InsectBattleRollDown);
}

// 0x160A4
void dInsMusBeetleBattle_c::beetle_c::initRecover() {
    mIsHeld = false;
    mTimer = 0;
    setAnmChr(mAnmChrWait, 0.0f);
    setAnmVis(mAnmVisWait);
    setAnmRate(0.5f);
    setAnmFrame(0.0f);
    mState = &beetle_c::stateRecover;
    if (isAttacker() == true) {
        mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
    }
    mStateId = STATE_RECOVER;
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

// 0x1617C
void dInsMusBeetleBattle_c::beetle_c::initReturn() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    mIsHeld = false;
    mTimer = 0;
    f32 frame = getAnmFrame();
    setAnmChr(mAnmChrWait, 240.0f);
    setAnmVis(mAnmVisWait);
    if (isAttacker() == true) {
        setAnmRate(-param.mWalkAnmRate);
    } else {
        setAnmRate(param.mWalkAnmRate);
    }
    setAnmFrame(frame);
    mState = &beetle_c::stateReturn;
    mVec3_c stumpPos;
    getInstance()->getStumpPos(&stumpPos);
    mReturnPos = mHomePos;
    if (isAttacker() == true) {
        mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
    }
    mStateId = STATE_RETURN;
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

// 0x162D4
void dInsMusBeetleBattle_c::beetle_c::stateFace() {
    mVec3_c target;
    getOpponentPos(&target);
    mVec3_c dir = target - mPos;
    dir.y = 0.0f;
    dir.normalize();
    mVec3_c front = mAxis.getFront();
    front += (dir - front) * getParam().mTurnRate;
    front.normalize();
    if (0.9999f <= dir.dot(front)) {
        mAxis.setFront(dir);
        setAnmRate(0.0f);
        getInstance()->setReady(mIdx);
    } else {
        mAxis.setFront(front);
    }
}

// 0x1641C
void dInsMusBeetleBattle_c::beetle_c::stateCharge() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    mSpeed *= param.mFriction;
    mVec3_c front = mAxis.getFront();
    front *= param.mAccel;
    mSpeed += front;
    mPos += mSpeed;
    mVec3_c target;
    getOpponentPos(&target);
    if (target.xzDistTo(mPos) <= 10.0f) {
        mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
        getInstance()->setReady(mIdx);
        if (isAttacker() == true) {
            if (!mCrushed) {
                mSnd.startSound(SE_InsectBattle_InsectCrush);
                mCrushed = true;
            } else {
                mSnd.startSound(SE_InsectBattle_InsectCrushMini);
            }
        }
    }
}

// 0x165CC
void dInsMusBeetleBattle_c::beetle_c::stateBack() {
    mVec3_c front = mAxis.getFront();
    mPos -= front * 0.02f;
    if (mTimer >= BACK_TIME) {
        getInstance()->setReady(mIdx);
    }
}

// 0x1665C
void dInsMusBeetleBattle_c::beetle_c::statePush() {
    if (mTimer >= PUSH_TIME) {
        getInstance()->setReady(mIdx);
    }
}

// 0x1669C
void dInsMusBeetleBattle_c::beetle_c::stateLift() {
    if (!isAttacker()) {
        mSnd.holdSound(SE_LEV_InsectBattleMogaku);
    }
    if (isAttacker() == true) {
        if (LIFT_END_FRAME <= mAnmChr.getFrame()) {
            getInstance()->setReady(mIdx);
        }
    } else {
        getInstance()->setReady(mIdx);
    }
}

// 0x16724
void dInsMusBeetleBattle_c::beetle_c::stateThrow() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    if (isAttacker() != true) {
        mSpeed.x *= param.mThrowFriction;
        mSpeed.z *= param.mThrowFriction;
        mSpeed.y -= 0.185f;
        mPos += mSpeed;
        if (mTimer > 5 && mPos.y < mHomePos.y + param.mFallHeight) {
            mPos.y = mHomePos.y + param.mFallHeight;
            if (mHitGround == true) {
                mSpeed.y = 0.0f;
            } else {
                mSpeed.y = param.mBounceSpeed;
                mHitGround = true;
                mTimer = 0;
                mSnd.startSound(SE_InsectBattleHitDown);
            }
        }
        if (mTimer <= 20 && !mHitGround) {
            mAxis.rotateZ(mAng(DEG_TO_ANGLE(9)));
            mAxis.rotateX(mAng(DEG_TO_ANGLE(1)));
            if (mTimer >= 18) {
                mAxis.setUp(mVec3_c(0.0f, -1.0f, 0.0f));
            }
        }
    }
    if (mTimer > THROW_TIME) {
        getInstance()->setReady(mIdx);
    }
    if (529.0f <= mAnmChr.getFrame()) { // empty in the target
    }
}

// 0x168C4
void dInsMusBeetleBattle_c::beetle_c::stateRecover() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    mVec3_c up = mAxis.getUp();
    if (mTimer <= 30 && up.y < 0.98f) {
        mAxis.rotateZ(mAng(DEG_TO_ANGLE(6)));
        if (mTimer >= 15) {
            mPos.y -= param.mFallHeight / 15.0f;
        } else {
            mPos.y = mHomePos.y + param.mFallHeight;
        }
    } else {
        mAxis.setUp(mVec3_c(0.0f, 1.0f, 0.0f));
        mPos.y = mHomePos.y;
        if (mTimer >= RECOVER_TIME) {
            initReturn();
        }
    }
}

// 0x169CC
void dInsMusBeetleBattle_c::beetle_c::stateReturn() {
    const dInsMusBattleBeetleParam_c &param = getParam();
    mPos.y = mHomePos.y;
    if (mPos.xzDistTo(mHomePos) <= 0.2f) {
        mVec3_c target;
        getOpponentPos(&target);
        mVec3_c dir = target - mPos;
        dir.y = 0.0f;
        dir.normalize();
        mVec3_c front = mAxis.getFront();
        mVec3_c turn = dir - front;
        if (PSVECMag(turn) < 0.1f) {
            setAnmRate(0.0f);
        } else {
            setAnmRate(param.mWalkAnmRate);
        }
        front += turn * 0.005f;
        front.normalize();
        mAxis.setFront(front);
        getInstance()->setReady(mIdx);
    } else {
        mVec3_c dir = mHomePos - mPos;
        dir.y = 0.0f;
        dir.normalize();
        mVec3_c front = mAxis.getFront();
        front.y = 0.0f;
        front.normalize();
        bool facing;
        if (isAttacker() == true) {
            facing = true;
        } else {
            facing = 0.999f <= dir.dot(front);
        }
        if (!facing) {
            mVec3_c turn = dir - front;
            front += turn * 0.01f;
            front.y = 0.0f;
            front.normalize();
            mAxis.setFront(front);
        } else {
            mSpeed = front;
            if (isAttacker() == true) {
                mSpeed *= 0.1f * 0.1f; // folded product, not the 0.01f literal
                mPos -= mSpeed;
            } else {
                static const f32 walkSpeed = 0.05f;
                mSpeed *= walkSpeed;
                mPos += mSpeed;
            }
        }
    }
}

// 0x16CFC
void dInsMusBeetleBattle_c::beetle_c::getOpponentPos(mVec3_c *out) {
    dInsMusBeetleBattle_c *battle = getInstance();
    if (mIdx == BEETLE_HERCULES) {
        battle->getBeetlePos(out, BEETLE_STAG);
    } else {
        battle->getBeetlePos(out, BEETLE_HERCULES);
    }
}

// 0x16D5C
dInsectUtil::axis_c dInsMusBeetleBattle_c::beetle_c::getOpponentAxis() {
    dInsMusBeetleBattle_c *battle = getInstance();
    if (mIdx == BEETLE_HERCULES) {
        return battle->getBeetleAxis(BEETLE_STAG);
    } else {
        return battle->getBeetleAxis(BEETLE_HERCULES);
    }
}

// 0x16DC0
f32 dInsMusBeetleBattle_c::beetle_c::getOpponentAnmFrame() {
    dInsMusBeetleBattle_c *battle = getInstance();
    if (mIdx == BEETLE_HERCULES) {
        return battle->getBeetleAnmFrame(BEETLE_STAG);
    } else {
        return battle->getBeetleAnmFrame(BEETLE_HERCULES);
    }
}

// 0x16E0C
bool dInsMusBeetleBattle_c::beetle_c::isAttacker() {
    dInsMusBeetleBattle_c *battle = getInstance();
    return battle->mAttacker == mIdx;
}

// 0x16E4C
bool dInsMusBeetleBattle_c::beetle_c::isHercules() {
    return mIdx == BEETLE_HERCULES;
}

// 0x16E5C
mMtx_c dInsMusBeetleBattle_c::beetle_c::getHeadMtx() {
    mMtx_c mtx;
    PSMTXIdentity(mtx);
    mMdl.getNodeWorldMtx(m3d::getNodeID(mMdl.getResMdl(), "head"), &mtx);
    return mtx;
}

// 0x16EC0
dInsMusBeetleBattle_c::dInsMusBeetleBattle_c()
    : mState(NULL), mHercules(BEETLE_HERCULES), mStag(BEETLE_STAG), mStumpPos(0.0f, 0.0f, 0.0f), m_heap_p(NULL), mTimer(0),
      mAttacker(BEETLE_STAG), mHerculesReady(false), mStagReady(false), mIsActive(false) {}

// 0x16F58
dInsMusBeetleBattle_c::~dInsMusBeetleBattle_c() {}

// 0x16FCC
BOOL dInsMusBeetleBattle_c::create(const nw4r::g3d::ResFile &herculesRes, const nw4r::g3d::ResFile &stagRes) {
    dBgUtil::getNodePos(&mStumpPos, "F2_kirikabu00", -1, -1);
    mHercules.init(herculesRes, &mAllocator);
    mStag.init(stagRes, &mAllocator);
    initFace();
    mIsActive = true;
    return TRUE;
}

// 0x17058
BOOL dInsMusBeetleBattle_c::execute() {
    mHercules.execute();
    mStag.execute();
    (this->*mState)();
    return TRUE;
}

// 0x170A4
BOOL dInsMusBeetleBattle_c::draw() {
    mHercules.draw();
    mStag.draw();
    return TRUE;
}

// 0x170E0
BOOL dInsMusBeetleBattle_c::remove() {
    mHercules.remove();
    mStag.remove();
    freeHeap();
    if (m_heap_p != NULL) {
        mHeap::destroyFrmHeap(m_heap_p);
        m_heap_p = NULL;
    }
    mIsActive = false;
    return TRUE;
}

// 0x17144
void dInsMusBeetleBattle_c::setHeap(EGG::FrmHeap *heap) {
    m_heap_p = heap;
    mAllocator.attach(heap, 0x20);
}

// 0x17150
void dInsMusBeetleBattle_c::freeHeap() {
    if (m_heap_p != NULL) {
        m_heap_p->free(MEM_FRM_HEAP_FREE_ALL);
    }
}

// 0x17168
dInsMusBeetleBattle_c *dInsMusBeetleBattle_c::getInstance() {
    return &sBeetleBattle;
}

// 0x17174
void dInsMusBeetleBattle_c::getBeetlePos(mVec3_c *out, int idx) const {
    if (idx == BEETLE_HERCULES) {
        *out = mHercules.mPos;
    } else {
        *out = mStag.mPos;
    }
}

// 0x171B4
dInsectUtil::axis_c dInsMusBeetleBattle_c::getBeetleAxis(int idx) const {
    if (idx == BEETLE_HERCULES) {
        return mHercules.mAxis;
    } else {
        return mStag.mAxis;
    }
}

// 0x17254
f32 dInsMusBeetleBattle_c::getBeetleAnmFrame(int idx) {
    if (idx == BEETLE_HERCULES) {
        return mHercules.mAnmChr.getFrame();
    } else {
        return mStag.mAnmChr.getFrame();
    }
}

// 0x1726C
void dInsMusBeetleBattle_c::getAttackerMtx(mMtx_c *out) const {
    if (mAttacker == BEETLE_HERCULES) {
        *out = mHercules.mMtx;
    } else {
        *out = mStag.mMtx;
    }
}

// 0x17340
void dInsMusBeetleBattle_c::setHeldMtx(mMtx_c mtx) {
    mHeldMtx = mtx;
}

// 0x173A4
void dInsMusBeetleBattle_c::getStumpPos(mVec3_c *out) const {
    *out = mStumpPos;
}

// 0x173C0
void dInsMusBeetleBattle_c::setReady(int idx) {
    if (idx == BEETLE_HERCULES) {
        mHerculesReady = true;
    }
    if (idx == BEETLE_STAG) {
        mStagReady = true;
    }
}

// 0x173E4
mMtx_c dInsMusBeetleBattle_c::getAttackerHeadMtx() {
    if (mAttacker == BEETLE_HERCULES) {
        return mHercules.getHeadMtx();
    } else {
        return mStag.getHeadMtx();
    }
}

// 0x17400
void dInsMusBeetleBattle_c::clearReady() {
    mHerculesReady = false;
    mStagReady = false;
}

// 0x17410
void dInsMusBeetleBattle_c::initFace() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateFace;
    mHercules.initFace();
    mStag.initFace();
    if (0.5f < cM::rnd()) {
        mAttacker = BEETLE_HERCULES;
    } else {
        mAttacker = BEETLE_STAG;
    }
}

// 0x17498
void dInsMusBeetleBattle_c::initCharge() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateCharge;
    mHercules.initCharge();
    mStag.initCharge();
}

// 0x174F4
void dInsMusBeetleBattle_c::initBack() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateBack;
    mHercules.initBack();
    mStag.initBack();
}

// 0x17550
void dInsMusBeetleBattle_c::initPush() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::statePush;
    mHercules.initPush();
    mStag.initPush();
}

// 0x175AC
void dInsMusBeetleBattle_c::initLift() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateLift;
    mHercules.initLift();
    mStag.initLift();
}

// 0x17608
void dInsMusBeetleBattle_c::initThrow() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateThrow;
    mHercules.initThrow();
    mStag.initThrow();
}

// 0x17664
void dInsMusBeetleBattle_c::initFinish() {
    mTimer = 0;
    mState = &dInsMusBeetleBattle_c::stateFinish;
    if (mHercules.isAttacker() == true) {
        mHercules.initReturn();
        mStag.initRecover();
    } else {
        mHercules.initRecover();
        mStag.initReturn();
    }
}

// 0x176E4
void dInsMusBeetleBattle_c::stateFace() {
    if (mHerculesReady == true && mStagReady == true) {
        initCharge();
        clearReady();
    }
}

// 0x17730
void dInsMusBeetleBattle_c::stateCharge() {
    if (mHerculesReady == true && mStagReady == true) {
        if (0.5f < cM::rnd()) {
            initPush();
        } else {
            initBack();
        }
        clearReady();
    }
}

// 0x177A0
void dInsMusBeetleBattle_c::stateBack() {
    if (mHerculesReady == true && mStagReady == true) {
        initCharge();
        clearReady();
    }
}

// 0x177EC
void dInsMusBeetleBattle_c::statePush() {
    if (mHerculesReady == true && mStagReady == true) {
        if (0.5f < cM::rnd()) {
            initBack();
        } else {
            initLift();
        }
        clearReady();
    }
}

// 0x1785C
void dInsMusBeetleBattle_c::stateLift() {
    if (mHerculesReady == true && mStagReady == true) {
        initThrow();
        clearReady();
    }
}

// 0x178A8
void dInsMusBeetleBattle_c::stateThrow() {
    if (mHerculesReady == true && mStagReady == true) {
        initFinish();
        clearReady();
    }
}

// 0x178F4
void dInsMusBeetleBattle_c::stateFinish() {
    if (mHerculesReady == true && mStagReady == true) {
        initFace();
        clearReady();
    }
}
