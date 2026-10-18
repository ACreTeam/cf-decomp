// dAcPlayer_c: idle / walk states and the remote-player move helpers (.text 0x2FF4..0x533C).
// Included by d_a_player.cpp.

// .text 0x2FF4
void dAcPlayer_c::initIdle() {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        mActorFlags2 &= ~(dPlayerMgr::ACTOR_FLAG2_SHOT_BALLOON | dPlayerMgr::ACTOR_FLAG2_SHOT_UFO);
        clearActorFlags101();
        mMsgStep = MSG_STEP_NONE;
        mTaskKind = FG_MNG_KIND_DROP;
        _2260 = dItem::ITEM_ID_NONE;
        if (dDemo::isActive9()) {
            dDemo::finish9();
        }
        _22A4[7] = 30;
        setIdleAnm();
        if (mState == dPlayerMgr::STATE_ENTER_ROOM) {
            _22DF[3] = 20;
        } else {
            _22DF[3] = 0;
        }
    } else {
        _21C0[3] = _21C0[2] = 1.0f;
        setIdleAnm();
        _22A4[1] = 0;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_PUSHED;
        _22A4[7] = dPlayerMgr::sRemoteActionDelayFrames;
        if (_21C0[17] <= 0.0f) {
            _21C0[17] = l_moveParam.mMaxSpeed;
        }
    }
}

// .text 0x3124
void dAcPlayer_c::endIdle(int nextState) {
    if (nextState != dPlayerMgr::STATE_WALK) {
        stopMove();
        _21C0[0] = 0.0f;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_LEFT_WATER;
    }
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_PUSHED;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    _22DF[3] = 0;
    _21C0[17] = 0.0f;
}

// .text 0x319C
void dAcPlayer_c::executeIdle() {
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        if (_22DF[3] != 0) {
            _22DF[3]--;
            mObjc.mWeight = 0xFF;
        } else {
            mObjc.mWeight = 0x80;
        }
        updateObjBalloon();
        setMoveInput();
        if (checkLeaveIdle()) {
            if (mTargetSpeed <= l_moveParam.mRunSpeed) {
                createFootprint(&mPos, 2);
            }
            dObjBal_c::hide();
        }
    } else if (!dPlayerMngr_c::isRemotePresent(mPlayerNo)) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_PUSHED) {
            if (_22A4[7] != 0) {
                _22A4[7]--;
            } else {
                mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_PUSHED;
            }
        } else {
            if (_22A4[1] != 0) {
                _22A4[1]--;
            }
            s16 angle;
            f32 distSq = getRemoteTargetDistSq(&angle, 1);
            if (distSq > 1024.0f) {
                f32 dist = nw4r::math::FSqrt(distSq);
                f32 maxSpeed = dist;
                if (dist > _21C0[17]) {
                    maxSpeed = _21C0[17];
                }
                f32 speed = maxSpeed;
                if (dist < 32.0f) {
                    clampApproachSpeed(dist, &speed);
                } else if (dist > 224.0f) {
                    mPos = mTargetPos;
                    mAngle.y = _2052;
                }
                mTargetSpeed = speed;
            } else {
                cLib::chasePosXZ(&mPos, mTargetPos, 0.3f);
                _2052 = angle;
                mTargetSpeed = 0.0f;
            }
            updateRemoteMove(angle, TRUE);
            calcRunFootEffects();
        }
    }
}

// .text 0x33A4
BOOL dAcPlayer_c::checkLeaveIdle() {
    if (checkActionInput(1)) {
        return TRUE;
    }
    if (checkDpadInput()) {
        return TRUE;
    }
    if (mTargetSpeed > 0.0f) {
        setState(dPlayerMgr::STATE_WALK);
        return TRUE;
    }
    return FALSE;
}

// .text 0x3428
void dAcPlayer_c::updateObjBalloon() {
    if (_22A4[7] != 0) {
        _22A4[7]--;
    }
    if (_22A4[7] == 0) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
                if (dScene::isCurrentSceneMyHouse()) {
                    dItem::Item item;
                    if (dSTR::isAlone()) {
                        mVec3_c pos;
                        getFrontUnit(&pos);
                        item = getFieldRealItem(&pos, 0, field);
                        if (!item.isSame(dItem::ITEM_ID_NONE)) {
                            if (item.isUsable()) {
                                item = dItem::ITEM_ID_NONE;
                            } else {
                                putUnit_c unit;
                                int res = dFtrm_c::findPutUnit(&unit, TRUE);
                                if (res >= 0 || res == dFtrm_c::PUT_UNIT_ITEM) {
                                    item = dItem::ITEM_ID_NONE;
                                }
                            }
                        }
                    }
                    dObjBal_c::show(item);
                }
            } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
                mVec3_c pos;
                getFrontUnit(&pos);
                dItem::Item item = getFieldPickableItem(&pos, 0, field);
                dObjBal_c::show(item);
            }
        }
    }
}

// .text 0x35A0
void dAcPlayer_c::initWalk() {
    mActorFlags = (mActorFlags & ~dPlayerMgr::ACTOR_FLAG_NOISY) | dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    _22A4[2] = 15;
    int no = mPlayerNo;
    _22B8[4] = 6;
    _22D8[1] = 0;
    _22D2[0] = 0;
    _22E5 = 0;
    _22E7[2] = 0;
    if (no == dNet::getMyNo()) {
        calcMoveInput();
        getMoveParam()->set(mSpeedF, 0);
        _22A4[7] = 0;
        _22C2 = mAngle.y;
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POINTER_WALK) {
            _22A4[7] = 15;
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mTargetPos.set(x, mPos.y, z);
            mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
            _2052 = angle;
            _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        }
        if (mState == dPlayerMgr::STATE_GET_OUT_OF_BED) {
            setAnm(HMN_ANM_NML_WALK1, 0.0f, 0.0f, l_moveParam.mWalkRateMin, 16.0f, m3d::FORWARD_LOOP, FALSE);
            setNetWalkAnm();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
        _21C0[3] = _21C0[2] = 1.0f;
    }
}

// .text 0x3704
void dAcPlayer_c::endWalk(int nextState) {
    switch (nextState) {
    case dPlayerMgr::STATE_IDLE:
    case dPlayerMgr::STATE_RUN:
        if (mAnmId != HMN_ANM_SNOWBALL_ROLL1) {
            _21C0[0] = mAnm.getFrame();
        } else {
            _21C0[0] = 0.0f;
        }
        break;
    case dPlayerMgr::STATE_EXIT_ROOM:
    case dPlayerMgr::STATE_TRIP:
        _2052 = mAngle.y;
        mAngle3D.y = _2052;
        _21C0[0] = 0.0f;
        _21C0[17] = 0.0f;
        break;
    default:
        _2050 = 0;
        mTargetSpeed = 0.0f;
        _21C0[0] = 0.0f;
        _21C0[17] = 0.0f;
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
            int type = mpToolBank->getToolType();
            if (type == HMN_TOOL_NET) {
                mpToolBank->setNetWait(m3d::FORWARD_LOOP, 90.0f);
            } else if (type == HMN_TOOL_ROD) {
                mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
            }
        }
        break;
    }
    _21B8 = l_moveParam.mMoveAccel;
    mAnm.setRate(1.0f);
    _2050 = 0;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_LEFT_WATER;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
}

// .text 0x384C
void dAcPlayer_c::executeWalk() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        f32 f4 = 16.0f;
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LEFT_WATER) {
            f4 = 70.0f;
        }
        setAnm(HMN_ANM_NML_WALK1, l_moveParam.mWalkMorph, _21C0[0], l_moveParam.mWalkRateMin, f4, m3d::FORWARD_LOOP,
               FALSE);
        setNetWalkAnm();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    if (_22A4[1] != 0) {
        _22A4[1]--;
    }
    if (_22A4[2] != 0) {
        _22A4[2]--;
    }
    if (_22B8[4] != 0) {
        _22B8[4]--;
    }
    if (mAnm.getRate() > 0.0f && _22B8[4] == 0) {
        checkFootSound();
    }
    if (mPlayerNo == dNet::getMyNo()) {
        executeWalkMine();
    } else {
        executeWalkRemote();
    }
}

// .text 0x3960
void dAcPlayer_c::checkWalkTransition() {
    if (checkActionInput(1)) {
        return;
    }
    if (checkDpadInput()) {
        return;
    }
    if (0.0f == mTargetSpeed && mAnmId == HMN_ANM_NML_DASH1) {
        if (mAnm.checkFrame(l_moveParam.mDashStopFrame0) || mAnm.checkFrame(l_moveParam.mDashStopFrame1)) {
            setState(dPlayerMgr::STATE_IDLE);
            return;
        }
    }
    if (mSpeedF >= l_moveParam.mDashSpeedMin) {
        if (mTargetSpeed >= l_moveParam.m34 && _22A4[2] == 0 &&
            sLib::distanceAngle(mAngle3D.y, _2052) >= l_moveParam.mTurnBackAngle) {
            setState(dPlayerMgr::STATE_RUN);
            return;
        }
    } else if (mSpeedF <= 0.0f) {
        if (_22A4[7] == 0) {
            setState(dPlayerMgr::STATE_IDLE);
        }
        return;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        if (_22A4[6] != 0) {
            _22A4[6]--;
        }
        static const f32 sTripSpeed = l_moveParam.mDashSpeed - 0.01f;
        if (_22A4[6] == 0 && mSpeedF >= sTripSpeed) {
            dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
            if (pd != NULL) {
                bool unlucky = pd->mFortune == FORTUNE_BAD ||
                               pd->mEquipment.mHat.isSame(dItem::Item(dItem::ITEM_IDX_KING_TUT_MASK));
                if (unlucky && cM::rndInt(900) == 0) {
                    setState(dPlayerMgr::STATE_TRIP);
                }
            }
        }
    }
}

// .text 0x3B88
void dAcPlayer_c::setMoveAnmRate(f32 maxSpeed, f32 maxRate, f32 minRate) {
    f32 rate = l_moveParam.mAnmRateScale * maxRate * mAng(16384.0f * (mSpeedF / maxSpeed)).sin();
    if (rate < minRate) {
        rate = minRate;
    } else if (rate > maxRate) {
        rate = maxRate;
    }
    mAnm.setRate(rate);
    syncToolAnm();
}

// .text 0x3C54
void dAcPlayer_c::setPushAnmRate(f32 ratio) {
    static const f32 l_scale = 1.0f;

    if (ratio > l_scale) {
        ratio = l_scale;
    }
    f32 rate = 1.5f * mAng(16384.0f * ratio).sin();
    if (rate < 0.0f) {
        rate = 0.0f;
    } else if (rate > 0.9f) {
        rate = 0.9f;
    }
    mAnm.setRate(rate);
    if (mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setAnmRate(rate);
    }
}

// .text 0x3D28
void dAcPlayer_c::calcWalk() {
    BOOL pushed = FALSE;
    BOOL blocked = FALSE;
    if (mAcch.mHitFlags & dBGCF::HIT_WALL_FRONT) {
        blocked = TRUE;
    } else {
        mAng angle = (u16)mAngle3D.y.mAngle;
        if (mObjc.checkPushAngle(&angle)) {
            blocked = TRUE;
            if (mObjc.isHit(dObjc::TYPE_SNOWBALL)) {
                mVec3_c normal = mAcch.mGroundNormal;
                if (0.0f != normal.x || 0.0f != normal.z) {
                    blocked = FALSE;
                    pushed = TRUE;
                }
            }
        }
    }
    if (blocked) {
        if (mTargetSpeed > l_moveParam.mRunSpeed) {
            mTargetSpeed = l_moveParam.mBlockedDashSpeed;
        } else if (mTargetSpeed > l_moveParam.mWalkSpeed) {
            mTargetSpeed = l_moveParam.mBlockedRunSpeed;
        } else if (mTargetSpeed > 0.0f) {
            mTargetSpeed = l_moveParam.mBlockedWalkSpeed;
        }
    }
    calcTurn();
    updateMoveAnm(pushed);
}

// .text 0x3E5C
void dAcPlayer_c::setMoveAnm(int anmId, BOOL force) {
    if (mAnmId == anmId) {
        return;
    }
    if (0.0f == mTargetSpeed && mAnmId == HMN_ANM_NML_DASH1) {
        return;
    }
    if (_22A4[1] != 0 && 0.0f != mTargetSpeed && !force) {
        return;
    }
    f32 frame = mAnm.getFrame();
    f32 morph = 60.0f;
    if (mAnmId == HMN_ANM_NML_DASH1) {
        f32 dashMorph = l_moveParam.mDashMorph;
        if (mAnm.checkFrame(l_moveParam.mDashStepFrame0) || mAnm.checkFrame(l_moveParam.mDashStepFrame1)) {
            startMoveAnm(dashMorph, frame, anmId);
        }
        return;
    }
    if (mAnmId == HMN_ANM_NML_WALK1) {
        if (anmId == HMN_ANM_NML_RUN1) {
            morph = l_moveParam.mWalkToRunMorph;
        } else if (anmId == HMN_ANM_NML_DASH1) {
            morph = l_moveParam.mWalkToDashMorph;
        }
    } else if (mAnmId == HMN_ANM_NML_RUN1) {
        if (anmId == HMN_ANM_NML_DASH1) {
            morph = l_moveParam.mRunToDashMorph;
        } else if (anmId == HMN_ANM_NML_WALK1) {
            morph = l_moveParam.mRunToWalkMorph;
        }
    }
    startMoveAnm(morph, frame, anmId);
}

// .text 0x3FDC
void dAcPlayer_c::calcTurn() {
    static const f32 l_rate = 1.0f / 6.0f;

    static const f32 sInvDashSpeed = 1.0f / l_moveParam.mDashSpeed;
    f32 ratio = mSpeedF * sInvDashSpeed;
    f32 turnMax = l_moveParam.mTurnMax;
    f32 turnMin = l_moveParam.mTurnMin;
    f32 bodyMax = l_moveParam.mBodyTurnMax;
    f32 bodyMin = l_moveParam.mBodyTurnMin;
    if (_22B8[4] != 0) {
        f32 t = (l_rate) * (6 - _22B8[4]);
        turnMin = l_angle3;
        bodyMin = l_angle3;
        turnMax *= t;
        bodyMax *= t;
    }
    sLib::chaseAngle(&_22C2, _2052, calcTurnStep(ratio, turnMax, turnMin));
    mAngle3D.set(0, _22C2, 0);
    sLib::chaseAngle(&mAngle.y.mAngle, _2052, calcTurnStep(ratio, bodyMax, bodyMin));
}

// .text 0x41BC
void dAcPlayer_c::calcTurnToTarget() {
    static const f32 sInvDashSpeed = 1.0f / l_moveParam.mDashSpeed;
    f32 ratio = mSpeedF * sInvDashSpeed;
    s16 step = calcTurnStep(ratio, l_moveParam.mTurnMax, l_moveParam.mTurnMin);
    sLib::chaseAngle(&_22C2, dActor_c::targetAngleY(&mPos, &mTargetPos), step);
    mAngle3D.set(0, _22C2, 0);
    static const f32 sBodyMax = 0.5f * l_moveParam.mBodyTurnMax;
    static const f32 sBodyMin = 0.5f * l_moveParam.mBodyTurnMin;
    sLib::chaseAngle(&mAngle.y.mAngle, _2052, calcTurnStep(ratio, sBodyMax, sBodyMin));
}

// .text 0x4364
void dAcPlayer_c::updateMoveAnm(BOOL noLean) {
    if (mSpeedF > l_moveParam.mRunSpeed || mTargetSpeed == l_moveParam.mDashSpeed) {
        setMoveAnm(HMN_ANM_NML_DASH1, FALSE);
        setMoveAnmRate(l_moveParam.mDashSpeed, l_moveParam.mDashRateMin, l_moveParam.mRunRateMin);
    } else if (mSpeedF > l_moveParam.mWalkSpeed || mTargetSpeed == l_moveParam.mRunSpeed) {
        setMoveAnm(HMN_ANM_NML_RUN1, FALSE);
        setMoveAnmRate(l_moveParam.mRunSpeed, l_moveParam.mRunRateMin, l_moveParam.mWalkRateMin);
    } else {
        setMoveAnm(HMN_ANM_NML_WALK1, FALSE);
        setMoveAnmRate(l_moveParam.mRunSpeed, l_moveParam.mRunRateMin, l_moveParam.mWalkRateMin);
    }
    _2050 = 0;
    if (mSpeedF > l_moveParam.mWalkSpeed && !noLean) {
        static const f32 sInvLeanRange = 1.0f / (l_moveParam.mMaxSpeed - l_moveParam.mWalkSpeed);
        f32 t = sInvLeanRange * (mSpeedF - l_moveParam.mWalkSpeed);
        int lean = _22FA[4] ? l_moveParam.mLeanMaxAlt : l_moveParam.mLeanMax;
        _2050 = lean * t;
    }
}

// .text 0x4520
BOOL dAcPlayer_c::isMovingToTarget(f32 distSq, s16 angle) {
    if (distSq < l_moveParam.mNearDist * l_moveParam.mNearDist) {
        _2052 = angle;
        return FALSE;
    }
    if (distSq < 256.0f) {
        _2052 = angle;
        if (sLib::distanceAngle(_22C2, dActor_c::targetAngleY(&mPos, &mTargetPos)) < 0x4000 &&
            mAnmId != HMN_ANM_NML_WAIT1) {
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// .text 0x45C4
void dAcPlayer_c::startMoveAnm(f32 morph, f32 frame, int anmId) {
    f32 f4 = 16.0f;
    if (_22A4[2] == 0) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_LEFT_WATER;
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LEFT_WATER) {
        f4 = 70.0f;
    }
    if (setAnm(anmId, morph, frame, mAnm.getRate(), f4, m3d::FORWARD_LOOP, FALSE)) {
        _22A4[1] = 20;
        if (mpToolBank->getToolType() == HMN_TOOL_NET) {
            switch (anmId) {
            case HMN_ANM_NML_WALK1:
                mpToolBank->setNetAnm1B9(m3d::FORWARD_LOOP, morph);
                mpToolBank->setAnmFrame(mAnm.getFrame());
                break;
            case HMN_ANM_NML_RUN1:
                mpToolBank->setNetAnm1B4(m3d::FORWARD_LOOP, morph);
                mpToolBank->setAnmFrame(mAnm.getFrame());
                break;
            case HMN_ANM_NML_DASH1:
                mpToolBank->setNetAnm1A8(m3d::FORWARD_LOOP, morph);
                mpToolBank->setAnmFrame(mAnm.getFrame());
                break;
            }
        }
    }
}

// .text 0x473C
s16 dAcPlayer_c::calcTurnStep(f32 ratio, s16 maxStep, s16 minStep) const {
    s16 step = maxStep * ratio;
    if (step < minStep) {
        return minStep;
    } else if (step > maxStep) {
        return maxStep;
    }
    return step;
}

// .text 0x4798
void dAcPlayer_c::executeWalkMine() {
    mVec3_c pos(mPos);
    s16 angle = mAngle.y;
    f32 speed = mSpeedF;
    calcMoveInput();
    BOOL pushing = daSnowMan_c::pushAll(mMoveSpeed, &pos, &angle, &speed, mAng(getInputAngle()));
    if (pushing) {
        mPos = pos;
        _2052 = angle;
        mAngle.y = angle;
        mAngle3D.y = angle;
        mSpeedF = 0.0f;
        mTargetSpeed = 0.0f;
    }
    if (pushing) {
        calcTurn();
        f32 frame = mAnm.getFrame();
        if (setAnm(HMN_ANM_SNOWBALL_ROLL1, 60.0f, frame, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE)) {
            if (mpToolBank->getToolType() == HMN_TOOL_NET) {
                mpToolBank->setNetAnm1B9(m3d::FORWARD_LOOP, 60.0f);
                mpToolBank->setAnmFrame(frame);
            }
        }
        setPushAnmRate(speed);
        _2050 = 0;
    } else {
        calcWalk();
    }
    if (_22A4[7] != 0) {
        _22A4[7]--;
        if (mSpeedF > l_moveParam.mNearDist) {
            _22A4[7] = 0;
        }
    }
    calcRunFootEffects();
    if (dNet::isSessionActive()) {
        if (_22E5 != 0) {
            _22E5--;
        } else {
            _22E5 = 10;
            _22B8[3]++;
        }
        getMoveParam()->set(mSpeedF, _22B8[3]);
    }
    if (!pushing) {
        checkWalkTransition();
    } else {
        startPitfallTrigger();
    }
}

// .text 0x49B4
void dAcPlayer_c::executeWalkRemote() {
    getRemoteActionParams(&mRemoteActionParams);
    s16 angle;
    f32 distSq = getRemoteTargetDistSq(&angle, 0);
    if (isMovingToTarget(distSq, angle)) {
        f32 dist = nw4r::math::FSqrt(distSq);
        f32 speed = dist;
        _21C0[17] = l_moveParam.mMaxSpeed;
        if (getMoveParam()->mSpeed <= l_moveParam.mRunSpeed) {
            _21C0[17] = l_moveParam.mRunSpeed;
        }
        if (dist > _21C0[17]) {
            speed = _21C0[17];
        } else {
            _2052 = angle;
        }
        f32 c = mAng(dActor_c::targetAngleY(&mPos, &mTargetPos) - _22C2).cos();
        if (c < 0.0f) {
            c = 0.0f;
        }
        f32 v = speed * c;
        if (dist < 16.0f) {
            clampApproachSpeed(dist, &v);
        } else if (dist > 224.0f) {
            mPos = mTargetPos;
            mAngle.y = _2052;
        }
        mTargetSpeed = v;
    } else {
        mTargetSpeed = 0.0f;
    }
    if (_22E5 != 0) {
        _22E5--;
    } else {
        _22E5 = 40;
        _22E7[2] = 0;
        if (_22B8[3] == getMoveParam()->mCount) {
            _22E7[2] = 1;
        }
        _22B8[3] = getMoveParam()->mCount;
    }
    updateRemoteMove(angle, _22E7[2]);
    calcRunFootEffects();
}

// .text 0x4BA0
f32 dAcPlayer_c::getRemoteTargetDistSq(s16 *angleOut, int mode) {
    f32 x;
    f32 z;
    s16 angle;
    if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, mPlayerNo)) {
        mTargetPos.set(x, mPos.y, z);
        mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
        *angleOut = angle;
        mVec3_c diff = mPos - mTargetPos;
        f32 distSq = diff.x * diff.x + diff.z * diff.z;
        if (mode == 0) {
            if (distSq < 1024.0f) {
                _2052 = angle;
            } else {
                s16 target = dActor_c::targetAngleY(&mPos, &mTargetPos);
                if (sLib::distanceAngle(target, angle) <= 0x2000) {
                    _2052 = angle;
                } else {
                    _2052 = target;
                }
            }
        } else if (mode == 1) {
            _2052 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        }
        return distSq;
    }
    mTargetPos = mPos;
    _2052 = mAngle.y;
    return 0.0f;
}

// .text 0x4D04
void dAcPlayer_c::clampApproachSpeed(f32 dist, f32 *speed) const {
    static const f32 sInvRange = 1.0f / (32.0f - l_moveParam.mNearDist);
    f32 max = l_moveParam.mNearDist +
              sInvRange * (dist - l_moveParam.mNearDist) * (l_moveParam.mDashSpeed - l_moveParam.mNearDist);
    if (max < l_moveParam.mNearDist) {
        max = l_moveParam.mNearDist;
    }
    if (*speed > max) {
        *speed = max;
    }
}

// .text 0x4D8C
void dAcPlayer_c::syncToolAnm() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
        int type = mpToolBank->getToolType();
        if (type == HMN_TOOL_NET) {
            mpToolBank->setAnmRate(mAnm.getRate());
        } else if (type == HMN_TOOL_WINDMILL) {
            mpToolBank->calcWind(&mPos);
        }
    }
}

// .text 0x4DF8
void dAcPlayer_c::updateRemoteMove(s16 angle, BOOL animate) {
    if (mTargetSpeed) {
        _21B8 = 0.1f;
    } else {
        _21B8 = l_moveParam.mMoveAccel;
    }
    if (mTargetSpeed <= 0.0f && animate) {
        if (_2050 != 0) {
            calcTurnToTarget();
            updateMoveAnm(FALSE);
        } else {
            _2052 = angle;
            calcTurnToTarget();
            if (setAnm(HMN_ANM_NML_WAIT1, l_moveParam.mWaitMorph, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE)) {
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
                    int type = mpToolBank->getToolType();
                    if (type == HMN_TOOL_NET) {
                        mpToolBank->setNetWait(m3d::FORWARD_LOOP, 90.0f);
                        if (mAnmId == HMN_ANM_NML_WAIT1) {
                            mpToolBank->setAnmFrame(mAnm.getFrame());
                        }
                    } else if (type == HMN_TOOL_ROD) {
                        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
                    }
                }
                _22A4[1] = 20;
            }
            mpToolBank->stopWind();
        }
    } else {
        switch (mState) {
        case dPlayerMgr::STATE_WALK: {
            f32 speed = mSpeedF;
            f32 target = mTargetSpeed;
            mSpeedF = getMoveParam()->mSpeed;
            mTargetSpeed = getMoveParam()->mSpeed;
            if (speed <= 0.0f) {
                _2052 = angle;
            }
            calcTurnToTarget();
            updateMoveAnm(FALSE);
            mSpeedF = speed;
            mTargetSpeed = target;
            break;
        }
        case dPlayerMgr::STATE_RUN: {
            f32 target = mTargetSpeed;
            mTargetSpeed = mSpeedF;
            calcTurnToTarget();
            updateMoveAnm(FALSE);
            mTargetSpeed = target;
            break;
        }
        case dPlayerMgr::STATE_IDLE:
            calcTurnToTarget();
            updateMoveAnm(FALSE);
            break;
        }
    }
}

// .text 0x5018
void dAcPlayer_c::moveParam_c::set(f32 speed, u8 count) {
    mSpeed = speed;
    mCount = count;
}

// .text 0x5024
void dAcPlayer_c::endRun(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mSpeedF = 0.0f;
        mVec3_c pos(mPos.x - 4.0f, mPos.y, mPos.z - 1.0f);
        createFootEffect("af_ply_footslide_yoko", &pos);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    } else {
        _22C2 = mAngle.y;
        _21C0[17] = 0.0f;
    }
    mAngle3D = mAngle;
    mTargetSpeed = 0.0f;
    if (nextState != dPlayerMgr::STATE_IDLE && (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT)) {
        int type = mpToolBank->getToolType();
        if (type == HMN_TOOL_NET) {
            mpToolBank->setNetWait(m3d::FORWARD_LOOP, 90.0f);
        } else if (type == HMN_TOOL_ROD) {
            mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
        }
    }
}
