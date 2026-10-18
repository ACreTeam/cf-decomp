// dAcPlayer_c: remote-player sync and the pad / pointer input (.text 0x2C314..0x2DCD4).
// Included by d_a_player.cpp.

// .text 0x2C314
void dAcPlayer_c::syncRemoteState() {
    if (mPlayerNo == dNet::getMyNo()) {
        return;
    }
    if (!dNet::isSessionActive()) {
        return;
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE) {
        return;
    }
    if (dPlayerMngr_c::isRemotePresent(mPlayerNo)) {
        return;
    }
    u32 sync;
    u8 scene;
    u8 state;
    if (!dPlayerMgr::getRemoteStateInfo(&sync, &scene, &state, mPlayerNo)) {
        return;
    }
    if (state == dPlayerMgr::STATE_IDLE) {
        return;
    }
    if (scene != dScene::getCurrentScene()) {
        setState(dPlayerMgr::STATE_SCENE_START);
        return;
    }
    state--;
    if (state >= dPlayerMgr::STATE_COUNT) {
        return;
    }
    if (state == mRemoteSyncA && sync == mRemoteSyncB) {
        return;
    }
    switch (state) {
    case dPlayerMgr::STATE_IDLE:
    case dPlayerMgr::STATE_WALK:
    case dPlayerMgr::STATE_RUN:
        if (mState == dPlayerMgr::STATE_ENTER_ROOM) {
            if (mAnmId != HMN_ANM_NML_WAIT1) {
                return;
            }
            getRemoteActionParams(&mRemoteActionParams);
            setState(state);
            mSyncValue = sync;
            mRemoteSyncA = dPlayerMgr::STATE_NONE;
        } else if (mState != state) {
            getRemoteActionParams(&mRemoteActionParams);
            setState(state);
            mSyncValue = sync;
            mRemoteSyncA = dPlayerMgr::STATE_NONE;
        }
        break;
    default:
        if (mState == state) {
            if (mSyncValue != sync) {
                getRemoteActionParams(&mRemoteActionParams);
                setState(state);
                mSyncValue = sync;
                mRemoteSyncA = dPlayerMgr::STATE_NONE;
            }
        } else {
            getRemoteActionParams(&mRemoteActionParams);
            setState(state);
            mSyncValue = sync;
            mRemoteSyncA = dPlayerMgr::STATE_NONE;
        }
        break;
    }
}

// .text 0x2C4F0
void dAcPlayer_c::getRemoteActionParams(dPlayerMngr_c::actionParam_c *param) {
    if (mPlayerNo != dNet::getMyNo() && dNet::isSessionActive()) {
        cLib::memCpy(param, (u8 *)dNet::getRecord(mPlayerNo + NET_REC_MEMBER_C) + 6, 9);
    }
}

// .text 0x2C560
void dAcPlayer_c::updateNetRecords() {
    if (mPlayerNo == dNet::getMyNo() && dNet::isSessionActive() &&
        !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
        dNet::setRecord(mPlayerNo + NET_REC_MEMBER_A, &mPos, 0);
        dNet::setRecord(mPlayerNo + NET_REC_MEMBER_B, &mAngle.y, 0);
        struct {
            u32 mSync;
            u8 mScene;
            u8 mState;
        } info;
        u8 record[15];
        info.mScene = dScene::getCurrentScene();
        info.mState = mState + 1;
        if ((u32)mState < dPlayerMgr::STATE_COUNT && dPlayerMgr::isActionParamLocked(mState)) {
            info.mState = 0xAE;
        }
        info.mSync = mSyncValue;
        cLib::memCpy(record, &info, 6);
        cLib::memCpy(&record[6], &mRemoteActionParams, 9);
        dNet::setRecord(mPlayerNo + NET_REC_MEMBER_C, record, 0);
    }
}

// .text 0x2C64C
void dAcPlayer_c::sendStatePacket() {
    if (dNet::isSessionActive() && mPlayerNo == dNet::getMyNo()) {
        u8 packet[14] = {0};
        cLib::memCpy(packet, &mSyncValue, 4);
        u8 state = mState;
        cLib::memCpy(&packet[4], &state, 1);
        cLib::memCpy(&packet[5], &mRemoteActionParams, 9);
        dNet::beginPacket();
        dNet::addPacket(packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_PLAYER_STATE, 4);
    }
}

// .text 0x2C6F8
void dAcPlayer_c::followRemoteTransform() {
    static const f32 l_warpDistSq = 96.0f * 96.0f;

    f32 x;
    f32 z;
    s16 angle;
    if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, mPlayerNo)) {
        mTargetPos.set(x, mPos.y, z);
        mVec3_c diff = mPos - mTargetPos;
        if (diff.x * diff.x + diff.z * diff.z > l_warpDistSq) {
            mPos = mTargetPos;
            if (mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
                mMtx_c handMtx;
                getRHandMtx(&handMtx);
                mpToolBank->calcBalloonInit(&handMtx);
            }
        } else {
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 2.0f);
        }
        s16 cur = mAngle.y;
        sLib::addCalcAngle(&cur, angle, 5, 1000);
        mAngle.y = cur;
    }
}

// .text 0x2C810
void dAcPlayer_c::calcPadInput() {
    _22FE[0] = 0;
    _22FE[1] = 0;
    _22FE[5] = _22FE[4];
    _22FE[4] = 0;
    _22FE[3] = _22FE[2];
    _22FE[2] = 0;
    _2304[1] = _2304[0];
    _2304[0] = 0;
    _22F0 = 0.0f;
    _21B4 = NULL;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POINTER_WALK;
    if (!_22FE[5]) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_POINTER_NO_TALK;
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_POINTER_NO_USE;
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_POINTER;
    }
    if (!dPad::isAHeld()) {
        _22E7[0] = 0;
        _22E7[1] = 0;
    }
    mMoveSpeed = 0.0f;
    mMoveAngle = mAngle.y;
    _22F4 = l_moveParam.mMoveAccel;
    _22FA[2] = 0;
    _22FA[3] = 0;
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_DEMO) || !dBG::getMdlBank()->isReady()) {
        _22FE[5] = 0;
        _22FE[3] = 0;
        _2304[1] = 0;
        return;
    }
    if (mPad::getCore()->downTrigger(WPAD_BUTTON_RIGHT)) {
        _22FA[2] = 2;
    } else if (mPad::getCore()->downTrigger(WPAD_BUTTON_LEFT)) {
        _22FA[2] = 4;
    }
    if (mPad::getCore()->downTrigger(WPAD_BUTTON_UP)) {
        _22FA[3] = 3;
    } else if (mPad::getCore()->downTrigger(WPAD_BUTTON_DOWN)) {
        _22FA[3] = 1;
    }
    BOOL confirm = dPad::isConfirmTriggered();
    BOOL cancel = dPad::isCancelTriggered();
    BOOL confirmHeld = dPad::isConfirmHeld();
    BOOL cancelHeld = dPad::isCancelHeld();
    BOOL shake = dPad::isShakeTriggered();
    BOOL useStick = TRUE;
    _22FE[4] = confirm;
    _22FE[1] = confirmHeld;
    _22FE[2] = cancel;
    _22FE[0] = cancelHeld;
    _2304[0] = shake;
    f32 speed = 0.0f;
    f32 angle = mAngle.y;
    f32 accel = 0.22f;
    const dObjc::dpdPos_c *dpd = dObjc::getDpdPos();
    if (dpd != NULL) {
        _22FE[4] = 0;
        mVec3_c pos = dpd->mPos;
        if (checkPointerInput(&pos)) {
            if (dPad::isAHeld() && (_22B8[2] == 0 || _22B8[2] == 10)) {
                checkPointerOnScreen(1.05f, 1.075f, &_22E7[0]);
                if (_22E7[0] == 1) {
                    useStick = FALSE;
                    mActorFlags |= dPlayerMgr::ACTOR_FLAG_POINTER_WALK;
                    speed = calcPointerSpeed(&pos);
                    if (speed <= 0.0f) {
                        return;
                    }
                    accel = 0.22f;
                    mVec3_c from = mPos;
                    switch (mState) {
                    case dPlayerMgr::STATE_FTR_HOLD:
                    case dPlayerMgr::STATE_FTR_ROTATE:
                    case dPlayerMgr::STATE_FTR_PUSH_START:
                    case dPlayerMgr::STATE_FTR_PULL_START:
                        from = mPos + mPointerOffset;
                        break;
                    }
                    angle = dActor_c::targetAngleY(&from, &pos);
                }
            }
        } else {
            useStick = FALSE;
        }
    } else {
        if (_22FE[4]) {
            _22E7[0] = 2;
            _22E7[1] = 2;
        }
        _22B8[2] = 0;
    }
    if (useStick) {
        calcStickInput(&speed, &angle, &accel);
    }
    mMoveSpeed = speed;
    mMoveAngle = angle;
    _22F4 = accel;
}

// .text 0x2CBF8
void dAcPlayer_c::setMoveInput() {
    _2052 = getInputAngle();
    f32 speed = mMoveSpeed;
    f32 accel = _22F4;
    mTargetSpeed = speed;
    _21B8 = accel;
}

// .text 0x2CC38
void dAcPlayer_c::calcMoveInput() {
    s16 angle = getInputAngle();
    switch (_22D8[1]) {
    case 0:
        _22D2[0] = angle;
        _2052 = angle;
        _22D8[1] = 1;
        break;
    case 1:
        checkInputTurn(angle, _22D2[0]);
        _22D2[0] = angle;
        _2052 = angle;
        break;
    case 2:
        if (sLib::distanceAngle(angle, _22D2[0]) > 0x2000) {
            _2052 = angle;
        } else {
            checkInputTurn(angle, _22D2[0]);
            if (_22D8[1] != 3) {
                _2052 = angle;
            }
        }
        _22D2[0] = angle;
        break;
    case 3:
        if (sLib::distanceAngle(angle, _22D2[0]) > 0x2000) {
            _2052 = angle;
        } else {
            checkInputTurn(angle, _22D2[0]);
            if (_22D8[1] != 2) {
                _2052 = angle;
            }
        }
        _22D2[0] = getInputAngle();
        break;
    }
    f32 speed = mMoveSpeed;
    f32 accel = _22F4;
    mTargetSpeed = speed;
    _21B8 = accel;
}

// .text 0x2CD68
void dAcPlayer_c::checkInputTurn(s16 angle, s16 prevAngle) {
    s16 diff = angle - prevAngle;
    if (diff > 0) {
        _22D8[1] = 2;
    } else if (diff < 0) {
        _22D8[1] = 3;
    } else if (diff != 0 && diff != -0x8000) {
        _2052 = angle;
    }
}

// .text 0x2CDA4
f32 dAcPlayer_c::calcPointerSpeed(const mVec3_c *target) {
    static const f32 l_stopSpeed = 0.0f;

    mVec3_c from = mPos;
    switch (mState) {
    case dPlayerMgr::STATE_FTR_HOLD:
    case dPlayerMgr::STATE_FTR_ROTATE:
    case dPlayerMgr::STATE_FTR_PUSH_START:
    case dPlayerMgr::STATE_FTR_PULL_START:
        from = mPos + mPointerOffset;
        break;
    }
    f32 distSq = (from - *target).xzLenSq();
    if (distSq < 256.0f) {
        return l_stopSpeed;
    }
    f32 maxSpeed = _22FE[0] ? l_moveParam.mDashSpeed : l_moveParam.mRunSpeed;
    f32 speed = distSq * maxSpeed / 2048.0f;
    f32 minSpeed = l_moveParam.mNearDist;
    if (speed > maxSpeed) {
        speed = maxSpeed;
    } else if (speed < minSpeed) {
        speed = minSpeed;
    }
    return speed;
}

// .text 0x2CED4
int dAcPlayer_c::getInputDirection() {
    return getAngleDirection((s16)(getInputAngle() - mAngle.y.mAngle));
}

// .text 0x2CF14
int dAcPlayer_c::getInputDirectionOrNone() {
    if (isNoMoveInput()) {
        return DIR_NONE;
    }
    return getAngleDirection((s16)(getInputAngle() - mAngle.y.mAngle));
}

// .text 0x2CF6C
int dAcPlayer_c::getAngleDirection(s16 angle) {
    if (angle <= -0x6000) {
        return DIR_BACK;
    }
    if (angle <= -0x2000) {
        return DIR_RIGHT;
    }
    if (angle <= 0x2000) {
        return DIR_FRONT;
    }
    int dir = DIR_BACK;
    if (angle <= 0x6000) {
        dir = DIR_LEFT;
    }
    return dir;
}

// .text 0x2CFB0
s16 dAcPlayer_c::decodeSeatPose(int *side, int pose) {
    switch (pose) {
    case 0:
        *side = 0;
        return 0;
    case 1:
        *side = 0;
        return 0x4000;
    case 2:
        *side = 0;
        return -0x8000;
    case 3:
        *side = 0;
        return -0x4000;
    case 4:
        *side = 2;
        return 0;
    case 5:
        *side = 2;
        return 0x4000;
    case 6:
        *side = 2;
        return -0x8000;
    case 7:
        *side = 2;
        return -0x4000;
    case 8:
        *side = 1;
        return 0;
    case 9:
        *side = 1;
        return 0x4000;
    case 10:
        *side = 1;
        return -0x8000;
    case 11:
        *side = 1;
        return -0x4000;
    default:
        *side = 0;
        return 0;
    }
}

// .text 0x2D0A0
int dAcPlayer_c::encodeSeatPose(s16 angle, int side) {
    switch (getAngleDirection(angle)) {
    case 0:
        if (side == 0) {
            return 0;
        }
        if (side == 2) {
            return 4;
        }
        if (side == 1) {
            return 8;
        }
        break;
    case 1:
        if (side == 0) {
            return 1;
        }
        if (side == 2) {
            return 5;
        }
        if (side == 1) {
            return 9;
        }
        break;
    case 2:
        if (side == 0) {
            return 2;
        }
        if (side == 2) {
            return 6;
        }
        if (side == 1) {
            return 10;
        }
        break;
    case 3:
        if (side == 0) {
            return 3;
        }
        if (side == 2) {
            return 7;
        }
        if (side == 1) {
            return 11;
        }
        break;
    }
    return 0;
}

// .text 0x2D1BC
s16 dAcPlayer_c::getInputAngle() {
    if (dCamera_c::sInstance == NULL) {
        return 0;
    }
    s16 angle = mMoveAngle;
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_POINTER_WALK) && mMoveSpeed) {
        angle += dCamera_c::sInstance->mCurrent.mYaw;
    }
    return angle;
}

// .text 0x2D20C
BOOL dAcPlayer_c::isNoMoveInput() {
    return mMoveSpeed <= 0.0f;
}

// .text 0x2D22C
void dAcPlayer_c::checkPointerTarget(const mVec3_c *pos) {
    static const f32 l_holdDistSq = 24.0f * 24.0f;

    if (mState == dPlayerMgr::STATE_FISHING_WAIT) {
        _22FE[4] = 1;
        return;
    }
    checkPointerOnScreen(1.05f, 1.075f, &_22E7[1]);
    if (_22E7[1] == 2) {
        _22FE[4] = 1;
        return;
    }
    const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
    mVec3_c target = *pos;
    if (hit != NULL) {
        u32 type = hit->mType;
        mVec3_c hitPos = hit->mPos;
        switch (type) {
        case dObjc::TYPE_PLAYER:
            if (this == hit->mBase.get()) {
                _22FE[4] = 1;
                mPointerOffset = target - mPos;
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
                    mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER;
                }
            }
            break;
        case dObjc::HIT_STR:
        case 0x80400000:
            if (dPlayerMgr::isFacingPosition(40.0f, &hitPos, l_angle45)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_NO_USE;
                _22FE[4] = 1;
            }
            break;
        case dObjc::HIT_FG: {
            int tool = mpToolBank->getToolType();
            if (tool == 0) {
                if (dPlayerMgr::isFacingPosition(40.0f, &hitPos, l_angle45)) {
                    _22FE[4] = 1;
                }
            } else if (tool == 2) {
                target = hitPos;
            }
            break;
        }
        case dObjc::TYPE_NPC: {
            f32 dist = 48.0f;
            s16 maxAngle = l_angle75;
            fBase_c *base = hit->mBase.get();
            if (base != NULL) {
                u16 prof = base->mProfName;
                if (prof == fProf::A_NPC_SP_PERIKO_05A || prof == fProf::A_NPC_SP_PERIKO_05B ||
                    prof == fProf::A_NPC_SP_FORTUNE || prof == fProf::A_NPC_SP_CAFE) {
                    dist = 80.0f;
                    maxAngle = l_angle22_5;
                } else if (prof == fProf::A_NPC_SP_TOTAKEKE) {
                    dist = 80.0f;
                    maxAngle = dDemoActor_c::mInteractionAngle;
                }
            }
            if (dPlayerMgr::isFacingPosition(dist, &hitPos, maxAngle)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_NO_USE;
                _22FE[4] = 1;
            } else if (isInSpeechRange(128.0f, 224.0f, &hitPos)) {
                dItem::Item key = hit->mParam;
                dDemoActor_c *npc = dNpcList::getNmlByKey(key);
                if (npc != NULL) {
                    dDemo::requestSpeech(this, npc);
                }
            }
            break;
        }
        case dObjc::HIT_INSECT:
            if (mpToolBank->getToolType() == HMN_TOOL_NET) {
                target = hitPos;
            }
            break;
        case dObjc::HIT_FTR_ROOM: {
            mVec3_c unitPos;
            getFrontPos(&unitPos);
            dFdBase_c::snapToUnitCenter(unitPos, unitPos);
            if (dFgobjManager::getDistXZSq(hitPos, unitPos) < l_holdDistSq && dFTR::holdList_c::isHoldScene()) {
                dFTR::hold_c *hold = dFTR::getHoldList()->get(0);
                if (hold != NULL) {
                    dFdBase_c::snapToUnitCenter(unitPos, mPos);
                    s16 hitAngle = dActor_c::targetAngleY(&unitPos, &hitPos);
                    mAng holdAngle = hold->mPushAngle;
                    if (getAngleDirection(holdAngle) == getAngleDirection(hitAngle)) {
                        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_NO_TALK | dPlayerMgr::ACTOR_FLAG2_POINTER;
                        mPointerOffset = target - mPos;
                        _22FE[4] = 1;
                    }
                }
            }
            break;
        }
        case dObjc::TYPE_SNOWMAN:
            if (dPlayerMgr::isFacingPosition(40.0f, &hitPos, l_angle45)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_NO_USE;
                _22FE[4] = 1;
            }
            break;
        }
    }
    if (!_22FE[4]) {
        checkPointerTurn(&target);
    }
}

// .text 0x2D698
void dAcPlayer_c::checkPointerTurn(const mVec3_c *pos) {
    _22B8[2] = 0;
    switch (mpToolBank->getToolType()) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
    case 8:
    case 9:
    case 0xD:
        if (sLib::distanceAngle(dActor_c::targetAngleY(&mPos, pos), mAngle.y) < l_angle75b) {
            _22B8[2] = 1;
            _22D2[2] = dActor_c::targetAngleY(&mPos, pos);
        }
        break;
    }
}

// .text 0x2D748
BOOL dAcPlayer_c::calcStickInput(f32 *speed, f32 *angle, f32 *accel) {
    f32 x = dPad::getStickX();
    f32 y = dPad::getStickY();
    if (0.0f == x && 0.0f == y) {
        return FALSE;
    }
    f32 minSpeed = l_moveParam.mNearDist;
    f32 maxSpeed = l_moveParam.mRunSpeed;
    f32 speedScale = l_moveParam.m00;
    f32 accelScale = l_moveParam.m24;
    if (_22FE[0]) {
        minSpeed = l_moveParam.m0C;
        maxSpeed = l_moveParam.mDashSpeed;
        speedScale = l_moveParam.m04;
        accelScale = l_moveParam.m28;
    }
    f32 lenSq = x * x + -y * -y;
    f32 len = lenSq < 1.0f ? nw4r::math::FSqrt(lenSq) : 1.0f;
    if (len > 1.0f) {
        len = 1.0f;
    }
    f32 s = len * speedScale;
    if (s > maxSpeed) {
        s = maxSpeed;
    } else if (s < minSpeed) {
        s = minSpeed;
    }
    *speed = s;
    *angle = cM::atan2s(x, -y);
    *accel = len * accelScale;
    _22F0 = len;
    return TRUE;
}

// .text 0x2D918
void dAcPlayer_c::startPointerTurn(f32 speed) {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER) {
        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_TURN;
        mTurnFrames = sLib::distanceAngle(_22D2[2], mAngle.y) / speed;
        if (mTurnFrames <= 0) {
            mTurnFrames = 1000;
        }
        mTargetAngle = _22D2[2];
    }
}

// .text 0x2D9C0 (+ a dead blr at 0x2D9DC)
void dAcPlayer_c::calcPointerTurn() {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_TURN) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    }
}

// .text 0x2D9E0
void dAcPlayer_c::endPointerTurn() {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_POINTER_TURN;
}

// .text 0x2D9F0
BOOL dAcPlayer_c::checkPointerInput(const mVec3_c *pos) {
    BOOL cPressed = FALSE;
    if (dPad::isCTriggered()) {
        _22FE[4] = 1;
        cPressed = TRUE;
    } else if (_22B8[2] != 0) {
        if (!dPad::isAHeld()) {
            if (_22B8[2] < 10) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_NO_TALK | dPlayerMgr::ACTOR_FLAG2_POINTER;
                _22FE[4] = 1;
            }
            _22B8[2] = 0;
        } else if (++_22B8[2] >= 10) {
            _22B8[2] = 10;
        }
    } else if (dPad::isATriggered()) {
        checkPointerTarget(pos);
    }
    if (_22FE[4]) {
        _22B8[2] = 0;
        if (!cPressed) {
            return FALSE;
        }
    }
    return TRUE;
}

// .text 0x2DAF4
s16 dAcPlayer_c::getActionAngle() {
    BOOL pointer =
        (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER) || (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_TURN);
    if (pointer) {
        return _22D2[2];
    }
    return mAngle.y;
}

// .text 0x2DB28
void dAcPlayer_c::checkPointerOnScreen(f32 width, f32 height, u8 *result) {
    if (*result == 0) {
        mVec2_c pointer = dPad::getRawPointer();
        if (pointer.x > -width && pointer.x < width && pointer.y > -height && pointer.y < height) {
            *result = 1;
        } else {
            *result = 2;
        }
    }
}

// .text 0x2DBD0
BOOL dAcPlayer_c::isInSpeechRange(f32 minDist, f32 maxDist, const mVec3_c *pos) {
    if (mState != dPlayerMgr::STATE_IDLE && mState != dPlayerMgr::STATE_WALK) {
        return FALSE;
    }
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        return FALSE;
    }
    f32 minSq = minDist * minDist;
    f32 maxSq = maxDist * maxDist;
    f32 distSq = (mPos - *pos).xzLenSq();
    if (distSq < minSq || distSq > maxSq) {
        return FALSE;
    }
    return sLib::distanceAngle(dActor_c::targetAngleY(&mPos, pos), mAngle.y) <= l_angle75;
}
