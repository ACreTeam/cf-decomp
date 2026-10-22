// dAcPlayer_c: water / phone / bus / room / coffee / slingshot / barber ... states, the input checks, the
// effects, the models and the Mii face (.text 0x1E398..0x2C2BC). Included by d_a_player.cpp.

// .text 0x1E398
void dAcPlayer_c::initWater() {
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    setAnm(HMN_ANM_JYORO_RAISE1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            mVec3_c pos;
            getNetFrontUnit(&pos);
            int blockX = mTargetUnitX >> 4;
            int blockZ = mTargetUnitZ >> 4;
            if (blockX > 0 && blockX < field->getBlockW() - 1 && blockZ > 0 && blockZ < field->getBlockH() - 1) {
                _229C = dFgobjManager::useTool(mPlayerNo, mTargetUnitX, mTargetUnitZ, dFgobjManager::USE_TOOL_WATERING,
                                               0, dItem::ITEM_ID_NONE);
                mTaskResult = dFgobjManager::getRequestResult(_229C);
                clearFieldRequest();
            }
        }
    }
    _2056 = mAngle.x;
    _2058 = mAngle.y;
    _205A = mAngle.z;
    startSound(SE_Jyouro);
}

// .text 0x1E4C8
void dAcPlayer_c::executeWater() {
    updateWaterEffect();
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.getFrame() > 96.0f && checkMoveOrAction()) {
            return;
        }
    } else {
        followRemoteTransform();
        if (mAnmId != HMN_ANM_NML_WAIT1 && mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        }
    }
}

// .text 0x1E598
void dAcPlayer_c::endWater(int nextState) {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) {
        mEffect.fade();
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
    }
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1E60C
void dAcPlayer_c::initBlowDandelion() {
    setAnm(HMN_ANM_TANPOPO_FLY1, 12.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mpToolBank->startFlower();
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x1D7D0
void dAcPlayer_c::executeBlowDandelion() {
    static const f32 l_blowFrame = 42.0f;

    f32 frame = mAnm.getFrame();
    if (mAnmId == HMN_ANM_TANPOPO_FLY1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_TanpopoWatage);
        } else if (mAnm.checkFrame(l_blowFrame)) {
            mLevelEffect2.create("af_itm_tanpopo", &mScriptedActionPos, &mAngle, NULL);
        } else if (frame > l_blowFrame) {
            mLevelEffect2.followEffect(&mScriptedActionPos, &mAngle, NULL);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
        if (mAnmId != HMN_ANM_NML_WAIT1 && mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
                removeTool();
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            }
        }
    }
}

// .text 0x1E6B0
void dAcPlayer_c::endBlowDandelion(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        removeTool();
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        removeTool();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x1E71C
void dAcPlayer_c::initPhonePickUp() {
    static const f32 l_moveFrames = 24.0f;
    static const f32 l_x = 304.0f;
    static const f32 l_y = 0.0f;
    static const f32 l_z = 399.0f;

    setAnm(HMN_ANM_TEL1, l_moveFrames, l_y, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    daRmobjTel_c::pickUp();
    mTargetPos.set(l_x, l_y, l_z);
    mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
    mVec3_c diff = mTargetPos - mPos;
    _21D0[0] = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / l_moveFrames;
    cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
    mTargetAngle = -0x8000;
    mTurnFrames = sLib::distanceAngle(-0x8000, mAngle.y) / l_moveFrames;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
}

// .text 0x1E848
void dAcPlayer_c::executePhonePickUp() {
    cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    if (mAnmId == HMN_ANM_TEL1 && mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            setState(dPlayerMgr::STATE_PHONE_TALK);
        } else {
            setAnmNoMorph(HMN_ANM_TEL_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x1E8DC
void dAcPlayer_c::endPhonePickUp(int nextState) {}

// .text 0x1E8E0
void dAcPlayer_c::initPhoneTalk() {
    setAnmNoMorph(HMN_ANM_TEL_WAIT1, m3d::FORWARD_LOOP);
    daRmobjTel_c::talk();
}

// .text 0x1E90C
void dAcPlayer_c::executePhoneTalk() {
    if (mPlayerNo != dNet::getMyNo()) {
        followRemoteTransform();
    }
}

// .text 0x1E94C
void dAcPlayer_c::endPhoneTalk(int nextState) {}

// .text 0x1E950
void dAcPlayer_c::initPhoneHangUp() {
    setAnm(HMN_ANM_TEL1, 12.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, FALSE);
    mAnm.setFrame(mAnm.mFrameMax - 1.0f);
    daRmobjTel_c::hangUp();
}

// .text 0x1E9C0
void dAcPlayer_c::executePhoneHangUp() {
    if (mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setState(dPlayerMgr::STATE_IDLE);
        } else {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x1EA38
void dAcPlayer_c::endPhoneHangUp(int nextState) {}

// .text 0x1EA3C
void dAcPlayer_c::initReact() {
    static const int l_turnFrames = 46;

    mTargetAngle = 0;
    mStep = 10;
    if (mPlayerNo == dNet::getMyNo()) {
        int type;
        switch (mReactionKind) {
        case dPlayerMgr::REACTION_DEBT_PAID:
            type = 2;
            break;
        case dPlayerMgr::REACTION_JOB_DONE:
            type = 3;
            break;
        default:
            type = 4;
            break;
        }
        if (&dBgm::l_mgr.mEvent != NULL) {
            dBgm::l_mgr.mEvent.muteC(type);
            dBgm::l_mgr.mEvent.playC(0);
        }
    }
    setAnm(HMN_ANM_LOAN_COMPLETE1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / l_turnFrames;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
}

// .text 0x1EB4C
void dAcPlayer_c::executeReact() {
    static const int l_changeFrame = 40;

    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    f32 frame = mAnm.getFrame();
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.checkFrame(l_changeFrame)) {
            if (dCamera_c::sInstance != NULL) {
                if (mReactionKind == dPlayerMgr::REACTION_DEBT_PAID && dScene::getCurrentScene() == dScene::SCENE_RM_OFFICE) {
                    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_26;
                } else {
                    dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
                }
            }
        } else if (frame > l_changeFrame) {
            switch (mReactionKind) {
            case dPlayerMgr::REACTION_JOB_DONE:
            case dPlayerMgr::REACTION_GOLD_NET:
            case dPlayerMgr::REACTION_GOLD_ROD:
            case dPlayerMgr::REACTION_GOLD_WATERING:
            case dPlayerMgr::REACTION_GOLD_AXE:
                stepReactMsg();
                break;
            }
        }
    }
}

// .text 0x1EC4C
void dAcPlayer_c::endReact(int nextState) {
    mReactionKind = dPlayerMgr::REACTION_NONE;
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stopC(0xB, 0x1A);
        }
    }
}

// .text 0x1ECC0
void dAcPlayer_c::stepReactMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            u16 code = MSG_JOB_DONE;
            switch (mReactionKind) {
            case dPlayerMgr::REACTION_JOB_DONE:
                code = MSG_JOB_DONE;
                break;
            case dPlayerMgr::REACTION_GOLD_NET:
                code = MSG_GOT_GOLD_NET;
                break;
            case dPlayerMgr::REACTION_GOLD_ROD:
                code = MSG_GOT_GOLD_ROD;
                break;
            case dPlayerMgr::REACTION_GOLD_WATERING:
                code = MSG_GOT_GOLD_WATERING;
                break;
            case dPlayerMgr::REACTION_GOLD_AXE:
                code = MSG_GOT_GOLD_AXE;
                break;
            }
            setMessageCode(code);
            getController()->requestStart(0);
            mStep = 11;
        }
        break;
    case 11: {
        dMsg::Comp_c *ctrl = getController();
        if (ctrl == NULL) {
            break;
        }
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            break;
        }
        mStep = 12;
        break;
    }
    case 12:
        if (mReactionKind == dPlayerMgr::REACTION_GOLD_AXE) {
            changeStateAfterMsg(dPlayerMgr::STATE_TAKE_OUT_TOOL);
        } else if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) &&
                   !(mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) &&
                   dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
            changeStateAfterMsg(mpToolBank->getToolType() == HMN_TOOL_UMBRELLA ? dPlayerMgr::STATE_OPEN_UMBRELLA
                                                                               : dPlayerMgr::STATE_TAKE_OUT_TOOL);
        } else {
            changeStateAfterMsg(dPlayerMgr::STATE_IDLE);
        }
        break;
    }
}

// .text 0x1EE74
void dAcPlayer_c::initEmotion() {
    if (mPlayerNo == dNet::getMyNo()) {
        getEmotionParam()->set(mEmotion);
    } else {
        mEmotion = getEmotionParam()->mEmotion;
    }
    startEmotion();
}

// .text 0x1EECC
void dAcPlayer_c::executeEmotion() {
    if (mPlayerNo == dNet::getMyNo()) {
        updateEmotion(dPlayerMgr::STATE_DEMO_START, TRUE);
    } else if (mAnmId != HMN_ANM_MENU_THINK1) {
        updateEmotion(dPlayerMgr::STATE_DEMO_START, TRUE);
    }
}

// .text 0x1EF34
void dAcPlayer_c::endEmotion(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        const emotionData_s *data = getEmotionData(0);
        if (data != NULL) {
            mManpu.start(&data->mStart, &mVec3_c::Zero, &mVec3_c::Zero, &mVec3_c::Zero);
        }
    }
}

// .text 0x1EF98
void dAcPlayer_c::startEmotion() {
    const emotionData_s *data = getEmotionData(mEmotion);
    if (data != NULL) {
        emotionData_s::Phase phase = emotionData_s::START;
        const dAcNpc_c::manpuMgr_c::startData_s *start = data->getPhaseData(phase);
        dAcNpc_c::manpuMgr_c::startData_s::AnmId anm = start->getAnmId();
        m3d::playMode_e mode = data->getMode();
        setAnm(anm, 8.0f, 0.0f, 1.0f, 16.0f, mode, FALSE);
        mManpu.start(start, &mVec3_c::Zero, &mVec3_c::Zero, &mVec3_c::Zero);
    }
    _22A4[7] = l_emotionFrames[mEmotion];
}

// .text 0x1F03C
void dAcPlayer_c::updateEmotion(int nextState, BOOL sound) {
    if (sound && _22A4[7] == l_emotionFrames[mEmotion]) {
        u32 se = mEmotion + 0x1776;
        if (se >= 0x1777 && se <= 0x1793) {
            startSound(se);
        }
    }
    if (mAnm.isStop()) {
        const emotionData_s *data = getEmotionData(mEmotion);
        if (data != NULL) {
            const dAcNpc_c::manpuMgr_c::startData_s *loop = &data->mLoop;
            if (setAnmNoMorph(loop->mAnmId, m3d::FORWARD_LOOP)) {
                mManpu.start(loop, &mVec3_c::Zero, &mVec3_c::Zero, &mVec3_c::Zero);
            }
        }
    }
    if (_22A4[7] == 0) {
        const emotionData_s *data = getEmotionData(0);
        if (data != NULL) {
            mManpu.start(&data->mStart, &mVec3_c::Zero, &mVec3_c::Zero, &mVec3_c::Zero);
        }
        int anmId = HMN_ANM_NML_WAIT1;
        if (nextState == dPlayerMgr::STATE_DEMO_START) {
            anmId = HMN_ANM_MENU_THINK1;
        }
        setAnm(anmId, l_emotionEndMorph[mEmotion], 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        if (mPlayerNo == dNet::getMyNo()) {
            setState(nextState);
        }
    } else {
        _22A4[7]--;
    }
}

// .text 0x1F1C8
void dAcPlayer_c::emotionParam_c::set(u8 emotion) {
    mEmotion = emotion;
}

// .text 0x1F1D0
void dAcPlayer_c::initMoveTo() {
    _21B8 = 0.22f;
    setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    setNetWalkAnm();
    startMoveToTarget();
    s16 angle = _2052;
    _22A4[1] = 0;
    _22C2 = angle;
}

// .text 0x1F248
void dAcPlayer_c::executeMoveTo() {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_ESCORTED) {
        mVec3_c dist = mPos - _205C;
        f32 scale = 1.0f - 0.001f * EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z);
        if (scale < 0.0f) {
            scale = 0.0f;
        } else if (scale > 1.0f) {
            scale = 1.0f;
        }
        mScale = scale;
    }
    _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
    mAngle3D.y = _22C2;
    mAngle3D.x = 0;
    mAngle3D.z = 0;
    calcTurn();
    updateMoveAnm(FALSE);
    if (mTargetSpeed) {
        _21B8 = 0.22f;
    } else {
        _21B8 = l_moveParam.mMoveAccel;
    }
    syncToolAnm();
    checkFootSound();
    if (moveToTarget()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x1F370
void dAcPlayer_c::endMoveTo(int nextState) {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_ESCORTED;
}

// .text 0x1F380
void dAcPlayer_c::initBusBoard() {
    setAnm(HMN_ANM_BUS_IN1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x1F3A8
void dAcPlayer_c::executeBusBoard() {
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_BUS_BOARDED);
    } else if (mAnm.checkFrame(155.0f)) {
        startSound(SE_jump);
    } else if (mAnm.checkFrame(175.0f)) {
        startSound(SE_CHAIR_SIT);
    }
    if (mAnm.checkFrame(17.0f) || mAnm.checkFrame(33.0f) || mAnm.checkFrame(52.0f) ||
        mAnm.checkFrame(71.0f) || mAnm.checkFrame(87.0f) || mAnm.checkFrame(103.0f) ||
        mAnm.checkFrame(122.0f) || mAnm.checkFrame(140.0f)) {
        startFootSound(SE_walk_yuka1, 1.5f);
    }
}

// .text 0x1F4F4
void dAcPlayer_c::endBusBoard(int nextState) {
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0x1F508
void dAcPlayer_c::initBusBoarded() {
    setAnmNoMorph(HMN_ANM_BUS_IN_WAIT1, m3d::FORWARD_LOOP);
}

// .text 0x1F514
void dAcPlayer_c::executeBusBoarded() {}

// .text 0x1F518
void dAcPlayer_c::endBusBoarded(int nextState) {}

// .text 0x1F51C
void dAcPlayer_c::initBusRide() {
    setAnm(HMN_ANM_BUS_IN2, m3d::FORWARD_ONCE);
}

// .text 0x1F528
void dAcPlayer_c::executeBusRide() {}

// .text 0x1F52C
void dAcPlayer_c::endBusRide(int nextState) {}

// .text 0x1F530
void dAcPlayer_c::initBusGetOff() {
    hideTool();
    setAnmNoMorph(HMN_ANM_BUS_OUT1, m3d::FORWARD_ONCE);
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0x1F57C
void dAcPlayer_c::executeBusGetOff() {
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_BUS_GOT_OFF);
    }
}

// .text 0x1F5C0
void dAcPlayer_c::endBusGetOff(int nextState) {}

// .text 0x1F5C4
void dAcPlayer_c::initBusGotOff() {
    setAnmNoMorph(HMN_ANM_BUS_OUT_WAIT1, m3d::FORWARD_LOOP);
}

// .text 0x1F5D0
void dAcPlayer_c::executeBusGotOff() {}

// .text 0x1F5D4
void dAcPlayer_c::endBusGotOff(int nextState) {}

// .text 0x1F5D8
void dAcPlayer_c::initBusLeave() {
    setAnm(HMN_ANM_BUS_OUT2, m3d::FORWARD_ONCE);
    mShadowScale = 1.0f;
    mShadowAlpha = 1.0f;
}

// .text 0x1F61C
void dAcPlayer_c::executeBusLeave() {
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    } else if (mAnm.checkFrame(1.0f)) {
        startSound(SE_CHAIR_STANDUP);
    } else if (mAnm.checkFrame(35.0f)) {
        startSound(SE_landing);
    }
    if (mAnm.checkFrame(68.0f) || mAnm.checkFrame(86.0f) || mAnm.checkFrame(106.0f) ||
        mAnm.checkFrame(125.0f) || mAnm.checkFrame(143.0f) || mAnm.checkFrame(161.0f)) {
        startFootSound(SE_walk_yuka1, 1.5f);
    } else if (mAnm.checkFrame(179.0f) || mAnm.checkFrame(195.0f)) {
        startFootSound(-1, 1.5f);
    }
}

// .text 0x1D934
void dAcPlayer_c::executeCracker() {
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(34.0f)) {
            startSound(SE_Cracker);
            startRumble(dRumble::PATTERN_CRACKER);
        } else if (mAnm.checkFrame(39.0f)) {
            removeTool();
            dEffect::create("af_itm_cracker", &mPos, &mAngle, NULL);
        }
    } else {
        followRemoteTransform();
        if (mAnmId != HMN_ANM_NML_WAIT1) {
            if (mAnm.isStop()) {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            } else if (mAnm.checkFrame(34.0f)) {
                startSound(SE_Cracker);
            } else if (mAnm.checkFrame(39.0f)) {
                removeTool();
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
                dEffect::create("af_itm_cracker", &mPos, &mAngle, NULL);
            }
        }
    }
}

// .text 0x1F77C
void dAcPlayer_c::endBusLeave(int nextState) {
    mShadowScale = mShadowAlpha = 1.0f;
}

// .text 0x1F790
void dAcPlayer_c::initCracker() {
    setAnm(HMN_ANM_CRACKER_SETOFF1, 32.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1F838
void dAcPlayer_c::endCracker(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        removeTool();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1F8A8
void dAcPlayer_c::initFireworks() {
    setAnm(HMN_ANM_FIRE_SETOFF1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, TRUE);
    mpToolBank->startHanabi();
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        dBgm::VolStateFirework_c *firework = &dBgm::l_mgr.mVolCtrl.mFirework;
        if (firework != NULL) {
            firework->start();
        }
        dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1DAB4
void dAcPlayer_c::executeFireworks() {
    static const f32 l_maxSize = 30.0f;
    static const f32 l_growFrames = 50.0f;
    static const f32 l_shrinkFrames = 100.0f;
    static const f32 l_growRate = 0.6f;
    static const f32 l_shrinkRate = 0.3f;

    if (mAnmId == HMN_ANM_FIRE_SETOFF1 && mpToolBank->getToolType() == HMN_TOOL_HANABI) {
        if (mpToolBank->getAnmRate() > 0.0f) {
            dEffectLight_c *light = dEffectLight_c::sInstance;
            if (light != NULL) {
                f32 frame = mpToolBank->getAnmFrame();
                f32 size;
                if (frame < l_growFrames) {
                    size = l_growRate * frame;
                } else if (frame > mpToolBank->getAnmFrameMax() - l_shrinkFrames) {
                    size = l_shrinkRate * (mpToolBank->getAnmFrameMax() - frame);
                } else {
                    size = l_maxSize;
                }
                if (size > l_maxSize) {
                    size = l_maxSize;
                }
                if (size < 0.0f) {
                    size = 0.0f;
                }
                nw4r::ut::Color color;
                color.Set(0x90, 0xE0, 0x60, 0x40);
                light->addGlow(&color, &mScriptedActionPos, size);
            }
            if (!mpToolBank->isAnmStop()) {
                if (mpToolBank->checkAnmFrame(1.0f)) {
                    mLevelEffect2.create(dItem::Item_getIdxInKind(mpToolBank->mItem) == 0 ? "af_itm_firework2"
                                                                                       : "af_itm_firework",
                                         &mModelMtx);
                } else if (mpToolBank->getAnmFrame() > 1.0f) {
                    if (dItem::Item_getIdxInKind(mpToolBank->mItem) == 0) {
                        holdSound(SE_SenkouHanabi);
                    } else {
                        holdSound(SE_TemochiHanabi);
                    }
                    mLevelEffect2.followEffect(&mModelMtx);
                }
            }
        } else {
            mpToolBank->startHanabi();
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mpToolBank->isAnmStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_FIRE_SETOFF1 &&
            (mpToolBank->isAnmStop() || mpToolBank->getToolType() != HMN_TOOL_HANABI)) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
                removeTool();
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
                hideTool();
            }
        }
    }
}

// .text 0x1F96C
void dAcPlayer_c::endFireworks(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        removeTool();
        dBgm::VolStateFirework_c *firework = &dBgm::l_mgr.mVolCtrl.mFirework;
        if (firework != NULL) {
            firework->end();
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        removeTool();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1F9F8
void dAcPlayer_c::initSceneStart() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    mShadowScale = 0.0f;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC | dPlayerMgr::ACTOR_FLAG_DISABLED;
}

// .text 0x1FA44
void dAcPlayer_c::executeSceneStart() {
    if (mPlayerNo == dNet::getMyNo()) {
        dDemo::finishSceneStart();
    }
}

// .text 0x1FA80
void dAcPlayer_c::endSceneStart(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    mShadowScale = 1.0f;
}

// .text 0x1FAA0
void dAcPlayer_c::initSceneStartHidden() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    mShadowScale = 0.0f;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC | dPlayerMgr::ACTOR_FLAG_DISABLED;
}

// .text 0x1FAEC
void dAcPlayer_c::executeSceneStartHidden() {
    if (mPlayerNo == dNet::getMyNo()) {
        dDemo::finishSceneStart();
    }
}

// .text 0x1FB28
void dAcPlayer_c::endSceneStartHidden(int nextState) {
    mShadowScale = 1.0f;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
}

// .text 0x1FB44
void dAcPlayer_c::initEnterRoom() {
    if (mPlayerNo == dNet::getMyNo()) {
        int kind = 0;
        if (dScene::getPrevScene() == dScene::SCENE_FIELD) {
            u8 other = 0;
            if (dSTR::getCurBuildingKind(&other) == 2) {
                kind = other != 0 ? 1 : 2;
            }
        }
        mTargetPos.x = mPos.x;
        mTargetPos.y = mPos.y;
        mTargetPos.z = mPos.z;
        mTargetSpeed = 1.0f;
        setEnterRoomParam(getEnterRoomParam(), mPos.x, mPos.z, kind);
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
        enterRoomParam_s *param = getEnterRoomParam();
        mTargetPos.set(param->mX, mPos.y, param->mZ);
        mPos.x = param->mX;
        mPos.z = param->mZ;
        mTargetSpeed = 0.0f;
        _22A4[7] = 0;
        if (param->mKind == 1) {
            _22A4[7] = dPlayerMgr::sRemoteActionDelayFrames;
            startSound(SE_room_DoorOpenCloseWithKnock);
        } else if (param->mKind == 2) {
            _22A4[7] = dPlayerMgr::sRemoteActionAlternateDelayFrames;
            startSound(SE_room_DoorOpenClose);
            dSTR::playRoomSe();
        }
        if (_22A4[7] == 0) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
            mSpeedF = mTargetSpeed = 1.2f;
        }
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    }
    _21B8 = 0.22f;
    startMoveToTarget();
    mVec3_c pos;
    getPosAtAngle(&pos, mAngle.y.mAngle - 0x8000, 32.0f);
    mPos = pos;
    _205C = pos;
    setAnmNoMorph(HMN_ANM_NML_WALK1, m3d::FORWARD_LOOP);
    mObjcPos.set(this, mMdlCallback.mBasePos, 20.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
}

// .text 0x1FD50
void dAcPlayer_c::executeEnterRoom() {
    if (mPlayerNo != dNet::getMyNo() && _22A4[7] != 0) {
        if (--_22A4[7] == 0) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
            mSpeedF = mTargetSpeed = 1.2f;
        } else if (_22A4[7] == l_roomSeFrame && getEnterRoomParam()->mKind == 1) {
            dSTR::playRoomSe();
        }
        return;
    }
    mObjcPos.mPos = mMdlCallback.mBasePos;
    mObjcPos.regist();
    if (mAnmId != HMN_ANM_NML_WAIT1) {
        updateMoveAnm(FALSE);
        checkFootSound();
        if (moveToTarget()) {
            if (mPlayerNo == dNet::getMyNo()) {
                if (dDemo::isType(dDemo::TYPE_SCENE_START)) {
                    if (!dPlayerMgr::isCockroachNoticed() && dScene::isCurrentSceneMyHouse() &&
                        isInHouseRoom() && dCockroachManager_c::sInstance != NULL &&
                        dCockroachManager_c::sInstance->isSpawned()) {
                        dPlayerMgr::markCockroachNoticed();
                        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_COCKROACH;
                    }
                    dDemo::finishSceneStart();
                    mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
                }
            } else {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            }
        }
    }
}

// .text 0x1FED0
void dAcPlayer_c::endEnterRoom(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
    }
    mObjcPos.mType &= ~0x20;
}

// .text 0x1FF24
void dAcPlayer_c::setEnterRoomParam(enterRoomParam_s *param, f32 x, f32 z, u8 kind) {
    param->mX = x;
    param->mZ = z;
    param->mKind = kind;
}

// .text 0x1FF34
void dAcPlayer_c::initExitRoom() {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
        if (mTargetSpeed < l_moveParam.mNearDist) {
            mTargetSpeed = l_moveParam.mNearDist;
        }
        _22C2 = mTargetAngle;
        mVec3_c pos = mTargetPos;
        int unitX = (int)pos.x >> 5;
        int unitZ = (int)pos.z >> 5;
        calcPosAtAngle(&mTargetPos, &pos, mTargetAngle, 96.0f);
        dFdBase_c::snapToUnitCenter(&mTargetPos, &mTargetPos);
        setExitRoomParam(getExitRoomParam(), mTargetSpeed, _22C2, unitX, unitZ);
    } else {
        exitRoomParam_s *param = getExitRoomParam();
        _22C2 = param->mAngle;
        mTargetSpeed = param->mSpeed;
        mTargetAngle = param->mAngle;
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, param->mUnitX, param->mUnitZ);
        calcPosAtAngle(&mTargetPos, &pos, mTargetAngle, 96.0f);
        dFdBase_c::snapToUnitCenter(&mTargetPos, &mTargetPos);
        mTargetAngle = _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
    }
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_DOOR_SE;
    _21B8 = 0.22f;
    mAngle3D.set(0, _22C2, 0);
    startMoveToTarget();
    _22A4[5] = 0;
    setAnm(HMN_ANM_NML_WALK1, m3d::FORWARD_LOOP);
    _22DF[4] = 0xFF;
}

// .text 0x200D8
void dAcPlayer_c::executeExitRoom() {
    static const int l_walkFrames = 100;
    static const int l_requestFrame = 30;

    mVec3_c dist = mPos - mTargetPos;
    f32 distSq = dist.x * dist.x + dist.z * dist.z;
    if (mPlayerNo == dNet::getMyNo()) {
        if (_22A4[5] >= l_requestFrame && !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED)) {
            if (dAPlayer::getSceneChange()->requestExit(mExitNo, 2, 2)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
            }
        }
    } else if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_DOOR_SE) && _22DF[4] < 20 && isRoomSeScene()) {
        startSound(SE_room_DoorOpenClose);
        dSTR::playRoomSe();
        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_DOOR_SE;
    }
    if (_22A4[5] < l_walkFrames) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
        mAngle3D.set(0, _22C2, 0);
        updateMoveAnm(FALSE);
        if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_DOOR_SE)) {
            checkFootSound();
        }
        _22A4[5]++;
    }
    if (distSq < 9216.0f) {
        u32 alpha = (u8)(255.0f * (distSq / 9216.0f));
        if (alpha > 0xFF) {
            alpha = 0xFF;
        }
        if (alpha < _22DF[4]) {
            setModelColor(alpha);
        }
        _22DF[4] = alpha;
        mShadowAlpha -= 0.05f;
        if (mShadowAlpha < 0.0f) {
            mShadowAlpha = 0.0f;
        }
    }
}

// .text 0x202B0
void dAcPlayer_c::endExitRoom(int nextState) {
    _22DF[4] = 0xFF;
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
}

// .text 0x202C8
BOOL dAcPlayer_c::isRoomSeScene() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        switch (dScene::getCurrentScene()) {
        case dScene::SCENE_RM_HS0_F1:
        case dScene::SCENE_RM_HS1_F1:
        case dScene::SCENE_RM_HS2_F1:
        case dScene::SCENE_RM_HS3_F1:
        case dScene::SCENE_RM_NPC0:
        case dScene::SCENE_RM_NPC1:
        case dScene::SCENE_RM_NPC2:
        case dScene::SCENE_RM_NPC3:
        case dScene::SCENE_RM_NPC4:
        case dScene::SCENE_RM_NPC5:
        case dScene::SCENE_RM_NPC6:
        case dScene::SCENE_RM_NPC7:
        case dScene::SCENE_RM_NPC8:
        case dScene::SCENE_RM_NPC9:
        case dScene::SCENE_RM_OFFICE:
        case dScene::SCENE_RM_SHOP0:
        case dScene::SCENE_RM_TAILOR:
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x20328
void dAcPlayer_c::setExitRoomParam(exitRoomParam_s *param, f32 speed, s16 angle, u8 unitX, u8 unitZ) {
    param->mSpeed = speed;
    param->mAngle = angle;
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
}

// .text 0x2033C
void dAcPlayer_c::initChangeEquipmentQuick() {}

// .text 0x20340
void dAcPlayer_c::executeChangeEquipmentQuick() {
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo && mEquipStep == EQUIP_STEP_LOADED) {
        dPrivateData_c *data = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (data->mEquipment.mHat.mId != dItem::ITEM_ID_NONE ||
            data->mEquipment.mAcc.mId != dItem::ITEM_ID_NONE) {
            startSound(SE_AutoCapOn);
        } else {
            startSound(SE_AutoCapOff);
        }
        mEquipStep = EQUIP_STEP_APPLIED;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x203E0
void dAcPlayer_c::endChangeEquipmentQuick(int nextState) {}

// .text 0x203E4
void dAcPlayer_c::initGoddessItem() {
    setAnm(HMN_ANM_AXE_SWING1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    _22A4[7] = 0;
    dCamera_c *camera = dCamera_c::sInstance;
    if (camera != NULL) {
        camera->mRequestedMode = dCamera_c::MODE_SCRIPTED_ACTION;
    }
    mStep = 0x20;
}

// .text 0x2045C
void dAcPlayer_c::executeGoddessItem() {
    if (mStep == 0x20) {
        if (mAnmId == HMN_ANM_AXE_SWING1) {
            getRHandMtx(&mToolMtx);
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_AXE_SUKA1, m3d::FORWARD_ONCE);
            }
        } else if (mAnmId == HMN_ANM_AXE_SUKA1) {
            f32 frame = mAnm.getFrame();
            if (mAnm.isStop()) {
                if (updateThrownAxe()) {
                    mStep = 0x21;
                } else if (setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP)) {
                    mMdl.removePartAnm(1, 110.0f);
                }
            } else if (mAnm.checkFrame(18.0f)) {
                startToolSound(SE_AxeThrowSpring);
                mVec3_c dist = mShowPos - mScriptedActionPos;
                _21D0[0] = EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z) / 85.0f;
                if (updateThrownAxe()) {
                    mStep = 0x21;
                }
            } else if (frame < 18.0f) {
                getRHandMtx(&mToolMtx);
                _22D2[1] = mAngle.y;
            } else if (updateThrownAxe()) {
                mStep = 0x21;
            }
        } else if (updateThrownAxe()) {
            mStep = 0x21;
        }
    } else if (sinkThrownAxe()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x1DD6C
BOOL dAcPlayer_c::updateThrownAxe() {
    static const int l_throwFrames = 79;

    _22D2[1] += 0xA8C;
    f32 t = _22A4[7];
    f32 speed = 0.22f * t - t * (0.0046f * t);
    if (speed < -2.4f) {
        speed = -2.4f;
    }
    mScriptedActionPos.y += speed;
    bool reached = cLib::chasePosXZ(&mScriptedActionPos, mShowPos, _21D0[0]);
    makeMtx(&mToolMtx, &mScriptedActionPos, _22D2[1]);
    if (reached || _22A4[7] > 1000) {
        mVec3_c pos(mShowPos.x - 16.0f, mShowPos.y, mShowPos.z - 16.0f);
        dEffect::create("af_itm_ono_wc", &pos, NULL, NULL);
        startToolSound(SE_AxeHitSpring);
        int rest = _22A4[7] - l_throwFrames;
        _22A4[7] = 0;
        if (rest > 0 && _22A4[7] < 10) {
            _22A4[7] = rest;
        }
        return TRUE;
    }
    _22A4[7]++;
    return FALSE;
}



// .text 0x20634
void dAcPlayer_c::endGoddessItem(int nextState) {
    dNpcInfo::goddess_c::get()->setItem(mpToolBank->mItem);
    removeTool();
    dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
}

// .text 0x2068C
BOOL dAcPlayer_c::sinkThrownAxe() {
    int spin = 0xA8C - _22A4[7] * 0x50;
    if (spin < 400) {
        spin = 400;
    }
    _22D2[1] += spin;
    f32 fall = -2.4f + 0.35f * _22A4[7];
    if (fall > -0.15f) {
        fall = -0.15f;
    }
    mScriptedActionPos.y += fall;
    f32 bottom = 12.5f + mShowPos.y;
    dActor_c::makeMtx(&mToolMtx, &mScriptedActionPos, _22D2[1]);
    if (mScriptedActionPos.y < bottom || _22A4[7] > 1000) {
        return TRUE;
    }
    _22A4[7]++;
    return FALSE;
}

// .text 0x20788
void dAcPlayer_c::initDrinkCoffee() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnm(HMN_ANM_CAFE_DRINK1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    } else {
        setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x20808
void dAcPlayer_c::executeDrinkCoffee() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.checkFrame(25.0f)) {
            dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
            if (cafe != NULL) {
                cafe->muteKK();
            }
            startSound(SE_cafe_CupSlide_player);
        } else if (mAnm.checkFrame(60.0f)) {
            startSound(SE_cafe_CupPickUp_player);
        } else if (mAnm.checkFrame(140.0f)) {
            startSound(SE_cafe_Nomu_player);
        } else if (mAnm.checkFrame(200.0f)) {
            startSound(SE_cafe_Nomiowaru_player);
        } else if (mAnm.checkFrame(mAnm.mFrameMax - 2.0f)) {
            dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
            if (cafe != NULL) {
                cafe->play1A92(0);
            }
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x20934
void dAcPlayer_c::endDrinkCoffee(int nextState) {}

// .text 0x20938
void dAcPlayer_c::initFinishCoffee() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnmNoMorph(HMN_ANM_CAFE_DRINK2, m3d::FORWARD_ONCE);
        dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
        if (cafe != NULL) {
            cafe->stop1A92(0xB, 0x47);
        }
    } else {
        setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x209B0
void dAcPlayer_c::executeFinishCoffee() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
            setState(dPlayerMgr::STATE_DEMO_WAIT);
        } else if (mAnm.checkFrame(45.0f)) {
            startSound(SE_cafe_CupOku_player);
        } else if (mAnm.checkFrame(70.0f)) {
            startSound(SE_cafe_CupSlideReturn_player);
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x20A6C
void dAcPlayer_c::endFinishCoffee(int nextState) {
    setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
}

// .text 0x20A78
void dAcPlayer_c::initTurnFront() {
    static const f32 l_x = 264.0f;
    static const f32 l_y = 0.0f;
    static const f32 l_z = 369.0f;

    setAnm(HMN_ANM_CONST_WAIT1, 15.0f, l_y, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    mTargetPos.set(l_x, l_y, l_z);
    mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
    mVec3_c dist = mTargetPos - mPos;
    _21D0[0] = EGG::Math<f32>::sqrt(dist.x * dist.x + dist.z * dist.z) / 15.0f;
    cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
    mTargetAngle = -0x8000;
    mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / 15.0f;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    mStep = 0x16;
}

// .text 0x20BA8
void dAcPlayer_c::executeTurnFront() {
    if (mStep == 0x16) {
        bool posDone = cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
        bool angleDone = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
        if (posDone && angleDone) {
            mStep = 0x17;
        }
    }
}

// .text 0x20C34
void dAcPlayer_c::endTurnFront(int nextState) {}

// .text 0x20C38
void dAcPlayer_c::initSlingshotAim() {
    setAnm(HMN_ANM_SLING_SHOOT1, m3d::FORWARD_ONCE);
    mpToolBank->setSlingShoot1(m3d::FORWARD_ONCE, 8.0f);
    mTargetAngle = -0x8000;
    mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / 11;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = dCamera_c::MODE_SOUND_TARGET_23;
        }
        startRumble(dRumble::PATTERN_SLINGSHOT_AIM);
    }
    _22A4[7] = 0;
}

// .text 0x20CFC
void dAcPlayer_c::executeSlingshotAim() {
    if (mAnmId == HMN_ANM_SLING_SHOOT1) {
        holdSound(SE_LEV_SlingShot_Pull);
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE) {
            setState(dPlayerMgr::STATE_IDLE);
            return;
        }
        if (_22FE[1] == 0) {
            _22A4[7] = 1;
        }
        if (mAnmId == HMN_ANM_SLING_SHOOT1) {
            if (mAnm.isStop()) {
                dBgm::VolCtrl_c *volCtrl = &dBgm::l_mgr.mVolCtrl;
                if (volCtrl != NULL) {
                    dBgm::VolStateCatapult_c *catapult = &dBgm::l_mgr.mVolCtrl.mCatapult;
                    if (catapult != NULL) {
                        catapult->start();
                    }
                }
                if (_22A4[7] != 0) {
                    setState(dPlayerMgr::STATE_SLINGSHOT_SHOOT);
                } else {
                    setAnm(HMN_ANM_SLING_SHOOT2, m3d::FORWARD_LOOP);
                    mpToolBank->setSlingShoot2(m3d::FORWARD_LOOP, 8.0f);
                }
            }
        } else if (_22A4[7] != 0) {
            setState(dPlayerMgr::STATE_SLINGSHOT_SHOOT);
        }
    } else if (mAnmId == HMN_ANM_SLING_SHOOT1 && mAnm.isStop()) {
        setAnm(HMN_ANM_SLING_SHOOT2, m3d::FORWARD_LOOP);
        mpToolBank->setSlingShoot2(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0x20E60
void dAcPlayer_c::endSlingshotAim(int nextState) {
    if (mPlayerNo == dNet::getMyNo() && nextState != dPlayerMgr::STATE_SLINGSHOT_SHOOT) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::VolCtrl_c *volCtrl = &dBgm::l_mgr.mVolCtrl;
        if (volCtrl != NULL) {
            dBgm::VolStateCatapult_c *catapult = &dBgm::l_mgr.mVolCtrl.mCatapult;
            if (catapult != NULL) {
                catapult->end();
            }
        }
    }
}

// .text 0x20EE0
void dAcPlayer_c::initSlingshotShoot() {
    if (mPlayerNo == dNet::getMyNo()) {
        dDemo::request9();
        int kind = dRumble::PATTERN_SLINGSHOT_SHOOT;
        int itemKind = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld.getKind();
        if (itemKind == dItem::KIND_SILVER_PACHINKO) {
            kind = dRumble::PATTERN_SLINGSHOT_SHOOT_SILVER;
        } else if (itemKind == dItem::KIND_GOLD_PACHINKO) {
            kind = dRumble::PATTERN_SLINGSHOT_SHOOT_GOLD;
        }
        startRumble(kind);
    }
    setAnm(HMN_ANM_SLING_SHOOT3, m3d::FORWARD_ONCE);
    mpToolBank->setSlingShoot3(m3d::FORWARD_ONCE, 8.0f);
}

// .text 0x20F84
void dAcPlayer_c::executeSlingshotShoot() {
    if (mAnmId == HMN_ANM_SLING_SHOOT3) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_SlingShot_Shot);
        } else if (mAnm.checkFrame(4.0f)) {
            mpToolBank->mFloat.shoot(&mPos);
        } else if (mAnm.isStop()) {
            if (setAnm(HMN_ANM_SLING_SHOOT_WAIT1, m3d::FORWARD_LOOP)) {
                mpToolBank->setSlingWait(m3d::FORWARD_LOOP, 8.0f);
            }
        }
    }
    if (mPlayerNo == dNet::getMyNo() && mAnmId != HMN_ANM_SLING_SHOOT3 &&
        mpToolBank->mFloat.mState == dUki_c::STATE_SHOT_HOLD) {
        setState(dPlayerMgr::STATE_IDLE);
    }
}

// .text 0x21084
void dAcPlayer_c::endSlingshotShoot(int nextState) {
    mpToolBank->mFloat.setState(dUki_c::STATE_SHOT_HOLD);
    if (mPlayerNo == dNet::getMyNo() && nextState != dPlayerMgr::STATE_SLINGSHOT_WATCH) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::VolCtrl_c *volCtrl = &dBgm::l_mgr.mVolCtrl;
        if (volCtrl != NULL) {
            dBgm::VolStateCatapult_c *catapult = &dBgm::l_mgr.mVolCtrl.mCatapult;
            if (catapult != NULL) {
                catapult->end();
            }
        }
    }
}

// .text 0x21114
void dAcPlayer_c::initSlingshotWatch() {
    if (setAnm(HMN_ANM_SLING_SHOOT_WAIT1, m3d::FORWARD_LOOP)) {
        mpToolBank->setSlingWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0x21164
void dAcPlayer_c::executeSlingshotWatch() {}

// .text 0x21168
void dAcPlayer_c::endSlingshotWatch(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::VolCtrl_c *volCtrl = &dBgm::l_mgr.mVolCtrl;
        if (volCtrl != NULL) {
            dBgm::VolStateCatapult_c *catapult = &dBgm::l_mgr.mVolCtrl.mCatapult;
            if (catapult != NULL) {
                catapult->end();
            }
        }
    }
}

// .text 0x211D4
void dAcPlayer_c::initBedHair() {
    if (mPlayerNo == dNet::getMyNo()) {
        mEmotion = 0x12;
        startEmotion();
        startSound(SE_Akubi);
    }
}

// .text 0x21228
void dAcPlayer_c::executeBedHair() {
    if (mPlayerNo == dNet::getMyNo()) {
        int nextState = dPlayerMgr::STATE_IDLE;
        if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && !(mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) &&
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
            nextState = mpToolBank->getToolType() == HMN_TOOL_UMBRELLA ? dPlayerMgr::STATE_OPEN_UMBRELLA
                                                          : dPlayerMgr::STATE_TAKE_OUT_TOOL;
        }
        updateEmotion(nextState, FALSE);
    }
}

// .text 0x212B8
void dAcPlayer_c::endBedHair(int nextState) {
    if (nextState != dPlayerMgr::STATE_TAKE_OUT_TOOL && nextState != dPlayerMgr::STATE_OPEN_UMBRELLA) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
    }
}

// .text 0x212D8
void dAcPlayer_c::initListenKK() {
    f32 frame = 0.0f;
    mStep = 0x18;
    if (mAnmId == HMN_ANM_CHAIR_SITDOWN_WAIT1) {
        frame = mAnm.getFrame();
        dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
        if (cafe != NULL && cafe->isKKPlaying()) {
            mStep = 0x19;
            if (cafe->getKKInfo3() <= 0) {
                frame = cafe->getKKInfoC() - 1.0f;
                if (frame < 0.0f) {
                    frame += mAnm.mFrameMax;
                }
            }
        }
    }
    setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, 20.0f, frame, 1.0f, 16.0f, m3d::FORWARD_LOOP, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        setListenKKParam(getListenKKParam(), mLookYaw, mLookTilt);
    }
}

// .text 0x213EC
void dAcPlayer_c::executeListenKK() {
    if (mPlayerNo == dNet::getMyNo()) {
        dDemoActor_c *talker = dDemo::getTalkActor();
        if (talker != NULL) {
            mVec3_c *pos = talker->demoHook58();
            if (pos != NULL) {
                mVec3_c target(pos->x, -5.0f + pos->y, pos->z);
                lookAt(&target, 0x7F, 0x7F);
                setListenKKParam(getListenKKParam(), mLookYaw, mLookTilt);
            }
        }
    } else {
        getRemoteActionParams(&mRemoteActionParams);
        setLookAngles(getListenKKParam()->mLookYaw, getListenKKParam()->mLookTilt, 0x7F, 0x7F);
    }
    dBgm::StgCafe_c *cafe = &dBgm::l_mgr.mStgCafe;
    if (cafe != NULL && cafe->isKKPlaying()) {
        mStep = 0x19;
        if (cafe->getKKInfo3() <= 0) {
            f32 frame = cafe->getKKInfoC() - 1.0f;
            if (frame < 0.0f) {
                frame += mAnm.mFrameMax;
            }
            mAnm.setFrame(frame);
        }
    } else if (mPlayerNo == dNet::getMyNo() && mStep == 0x19) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x2155C
void dAcPlayer_c::endListenKK(int nextState) {
    lookAt(NULL, 0x7F, 0x7F);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
    mStep = 0x22;
}

// .text 0x215A8
void dAcPlayer_c::setListenKKParam(listenKKParam_s *param, s16 lookYaw, s16 lookTilt) {
    param->mLookYaw = lookYaw;
    param->mLookTilt = lookTilt;
}

// .text 0x215B4
void dAcPlayer_c::initBlowBubble() {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
    setAnm(HMN_ANM_TANPOPO_FLY1, 12.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
    }
}

// .text 0x21634
void dAcPlayer_c::endBlowBubble(int nextState) {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) {
        mEffect.fade();
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
    }
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
    }
}

// .text 0x2169C
void dAcPlayer_c::initPutAwayToolFull() {
    static const int l_putAwayAnm = HMN_ANM_TOOL_PUTAWAY1;

    setAnm(l_putAwayAnm, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        mStep = 0x22;
        setPutAwayToolFullParam(getPutAwayToolFullParam(), 0x22);
    } else {
        mStep = getPutAwayToolFullParam()->mStep;
    }
}

// .text 0x1DECC
void dAcPlayer_c::executeBlowBubbles() {
    static const f32 l_blowFrame = 42.0f;

    if (mAnmId == HMN_ANM_TANPOPO_FLY1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_Shabondama);
        } else if (mAnm.checkFrame(l_blowFrame)) {
            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EFFECT;
            mEffect.createEffect("af_itm_syabon", &mScriptedActionPos, &mAngle, NULL, wateringParticleCallback,
                                 dPlayerMngr_c::makeActorParam(mPlayerNo, 2));
        } else if (mAnm.getFrame() > l_blowFrame && (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT)) {
            mEffect.follow(&mScriptedActionPos, &mAngle, NULL);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
        if (mAnmId != HMN_ANM_NML_WAIT1 && mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x2171C
void dAcPlayer_c::executePutAwayToolFull() {
    static const f32 l_scaleRate = 0.08f;

    if (mPlayerNo == dNet::getMyNo()) {
        setPutAwayToolFullParam(getPutAwayToolFullParam(), mStep);
    } else {
        getRemoteActionParams(&mRemoteActionParams);
        mStep = getPutAwayToolFullParam()->mStep;
    }
    if (mStep == 0x1F) {
        int anmId = mAnmId;
        if (anmId == HMN_ANM_TOOL_PUTAWAY1 && mAnm.getPlayMode() == m3d::REVERSE_ONCE) {
            if (mAnm.checkFrame(18.0f)) {
                f32 scale = l_scaleRate;
                if (scale > 1.0f) {
                    scale = 1.0f;
                }
                mToolScale.set(scale, scale, scale);
                setAnm(HMN_ANM_NML_WAIT1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
            } else if (mAnm.checkFrame(36.0f)) {
                startSound(SE_gasagoso);
            }
        } else if (anmId == HMN_ANM_NML_WAIT1) {
            if (mToolScale.x < 1.0f) {
                f32 scale = l_scaleRate * mAnm.getFrame();
                if (scale > 1.0f) {
                    scale = 1.0f;
                }
                mToolScale.set(scale, scale, scale);
            }
            if (mAnm.checkFrame(18.0f)) {
                mToolScale.set(1.0f, 1.0f, 1.0f);
                if (mPlayerNo == dNet::getMyNo()) {
                    setState(dPlayerMgr::STATE_IDLE);
                }
            }
        } else {
            mToolScale.set(0.0f, 0.0f, 0.0f);
            setAnm(HMN_ANM_TOOL_PUTAWAY1, 60.0f, 24.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, TRUE);
            mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        }
    } else if (mAnmId == HMN_ANM_TOOL_PUTAWAY1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_gasagoso_return);
        } else if (mAnm.checkFrame(18.0f)) {
            mToolScale.set(0.0f, 0.0f, 0.0f);
        } else if (mAnm.getFrame() >= 35.0f) {
            mToolScale.set(0.0f, 0.0f, 0.0f);
            setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
            if (mPlayerNo == dNet::getMyNo()) {
                mStep = 10;
            }
        } else if (mAnm.getFrame() < 18.0f && mToolScale.x > 0.0f) {
            f32 scale = 1.0f - 0.065f * mAnm.getFrame();
            if (scale < 0.0f) {
                scale = 0.0f;
            }
            mToolScale.set(scale, scale, scale);
        }
    } else if (mAnmId == HMN_ANM_PUTAWAY_WAIT1) {
        if (mPlayerNo == dNet::getMyNo()) {
            stepPocketFullMsg();
        }
    }
}

// .text 0x21A2C
void dAcPlayer_c::endPutAwayToolFull(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        mToolScale.set(1.0f, 1.0f, 1.0f);
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x21A88
void dAcPlayer_c::stepPocketFullMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(MSG_EQUIP_POCKETS_FULL);
            getController()->requestStart(0);
            mStep = 11;
        }
        break;
    case 11: {
        dMsg::Comp_c *ctrl = getController();
        if (ctrl == NULL) {
            break;
        }
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            break;
        }
        mStep = 12;
        break;
    }
    case 12:
        if (setStepAfterMsg(0x1F)) {
            setAnm(HMN_ANM_TOOL_PUTAWAY1, 60.0f, 24.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, TRUE);
            mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        }
        break;
    }
}

// .text 0x1E038
void dAcPlayer_c::initFortuneHit() {
    setAnm(HMN_ANM_FORTUNE_HIT1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    dEffect::create("af_ply_fortune_hit", &mMdlCallback.mFeelMtx);
}

// .text 0x1E098
void dAcPlayer_c::executeTrickChange() {
    switch (mStep) {
    case 8:
        if (mAnmId == HMN_ANM_HALLOWEEN_CHANGE1 && mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        if (mEquipStep == EQUIP_STEP_LOADED) {
            if (mAnmId == HMN_ANM_HALLOWEEN_CHANGE1) {
                mAnm.setFrame(0.0f);
            } else {
                setAnm(HMN_ANM_HALLOWEEN_CHANGE1, 4.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            }
            startSound(SE_HalloweenItazura);
            mStep = 9;
        }
        break;
    case 9:
        if (mAnmId == HMN_ANM_NML_WAIT1) {
            break;
        }
        if (mAnm.checkFrame(36.0f)) {
            if (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK) {
                dEffect::create("af_ply_trick_b", &mPos, &mAngle, NULL);
            } else {
                dEffect::create("af_ply_trick_a", &mPos, &mAngle, NULL);
            }
            startSound(SE_HalloweenSmoke);
        }
        if (mAnm.isStop()) {
            if (mPlayerNo == dNet::getMyNo()) {
                setState(dPlayerMgr::STATE_DEMO_WAIT);
            } else {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            }
        } else if (mAnm.checkFrame(36.0f)) {
            if (mEquipStep == EQUIP_STEP_LOADED) {
                mEquipStep = EQUIP_STEP_APPLIED;
            }
        } else if (mAnm.getFrame() > 36.0f && mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        }
        break;
    default:
        if (mPlayerNo == dNet::getMyNo()) {
            setState(dPlayerMgr::STATE_DEMO_WAIT);
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        break;
    }
}

// .text 0x21BD4
void dAcPlayer_c::setPutAwayToolFullParam(putAwayToolFullParam_s *param, u8 step) {
    param->mStep = step;
}

// .text 0x21BDC
void dAcPlayer_c::initShoeShineStart() {
    _21B8 = 0.22f;
    setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    startMoveToTarget();
    s16 angle = _2052;
    _22A4[1] = 0;
    _22C2 = angle;
    mStep = 0x1A;
}

// .text 0x21C54
void dAcPlayer_c::executeShoeShineStart() {
    switch (mStep) {
    case 0x1A:
        _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        mAngle3D.y = _22C2;
        mAngle3D.x = 0;
        mAngle3D.z = 0;
        calcTurn();
        updateMoveAnm(FALSE);
        if (mTargetSpeed) {
            _21B8 = 0.22f;
        } else {
            _21B8 = l_moveParam.mMoveAccel;
        }
        checkFootSound();
        if (moveToTarget()) {
            f32 rate = (mTurnFrames / 500.0f) * l_moveParam.mWalkRateMin;
            if (rate > l_moveParam.mDashRateMin) {
                rate = l_moveParam.mDashRateMin;
            } else if (rate < l_moveParam.mWalkRateMin) {
                rate = l_moveParam.mWalkRateMin;
            }
            mAnm.setRate(rate);
            mTargetAngle = -0x2000;
            mStep = 0x1B;
        }
        break;
    case 0x1B: {
        bool done = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
        if (done) {
            mStep = 0x1C;
            setAnm(HMN_ANM_SHOE_SITDOWN1, 5.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        }
        break;
    }
    case 0x1C:
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_SHOE_SHINE);
        }
        break;
    }
}

// .text 0x21E04
void dAcPlayer_c::endShoeShineStart(int nextState) {}

// .text 0x21E08
void dAcPlayer_c::initShoeShine() {
    setAnmNoMorph(HMN_ANM_SHOE_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    mStep = 0x1D;
}

// .text 0x21E44
void dAcPlayer_c::executeShoeShine() {
    if (mStep == 0x1E) {
        rebindShoePalette();
        mStep = 0x1D;
    }
}

// .text 0x21E84
void dAcPlayer_c::endShoeShine(int nextState) {
    if (mStep == 0x1E) {
        rebindShoePalette();
    }
    mStep = 0x22;
}

// .text 0x21EC4
void dAcPlayer_c::initShoeShineEnd() {
    setAnm(HMN_ANM_SHOE_STANDUP1, 5.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x21EEC
void dAcPlayer_c::executeShoeShineEnd() {
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x21F30
void dAcPlayer_c::endShoeShineEnd(int nextState) {}

// .text 0x21F34
void dAcPlayer_c::executeFortuneHit() {
    updateFortuneHitEffect();
    if (mAnmId == HMN_ANM_FORTUNE_HIT1 && mAnm.isStop()) {
        setAnmNoMorph(HMN_ANM_FORTUNE_HIT_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x21F8C
void dAcPlayer_c::endFortuneHit(int nextState) {}

// .text 0x21F90
void dAcPlayer_c::updateFortuneHitEffect() {
    holdSound(SE_LEV_Piyoru);
    mMtx_c mtx = mMdlCallback.mFeelMtx;
    mVec3_c pos(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
    dWorld::toFieldPosition(&pos, &pos);
    mLevelEffect3.create("af_ply_kizetsu", &pos, NULL, NULL, 0);
}

// .text 0x22064
void dAcPlayer_c::initFortuneFaint() {
    setAnm(HMN_ANM_FORTUNE_HIT2, 9.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x2208C
void dAcPlayer_c::executeFortuneFaint() {
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    } else if (mAnm.getFrame() < 20.0f) {
        mMtx_c mtx = mMdlCallback.mFeelMtx;
        mVec3_c pos(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
        dWorld::toFieldPosition(&pos, &pos);
        if (mLevelEffect3.create("af_ply_kizetsu", &pos, NULL, NULL, 1)) {
            mLevelEffect3.setColor(0xFF, 0xFF, 0xFF, 0xFF - (u32)mAnm.getFrame() * 12, EGG::Effect::RECURSIVE_0);
        }
    }
}

// .text 0x221D4
void dAcPlayer_c::endFortuneFaint(int nextState) {}

// .text 0x221D8
void dAcPlayer_c::initFlinch() {
    setAnm(HMN_ANM_STARTLE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x22200
void dAcPlayer_c::executeFlinch() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_DEMO_WAIT);
        }
    } else if (mAnm.isStop()) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x22278
void dAcPlayer_c::endFlinch(int nextState) {}

// .text 0x2227C
void dAcPlayer_c::initSurprised() {
    s16 angle = mAngle.y;
    mTurnFrames = 1000;
    mTargetAngle = angle;
    mEmotion = 2;
    startEmotion();
}

// .text 0x22298
void dAcPlayer_c::executeSurprised() {
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
        updateEmotion(dPlayerMgr::STATE_IDLE, TRUE);
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        updateEmotion(dPlayerMgr::STATE_IDLE, TRUE);
    }
}

// .text 0x22310
void dAcPlayer_c::endSurprised(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        const emotionData_s *data = getEmotionData(0);
        if (data != NULL) {
            mManpu.start(&data->mStart, &mVec3_c::Zero, &mVec3_c::Zero, &mVec3_c::Zero);
        }
    }
}

// .text 0x22374
void dAcPlayer_c::initTrickChange() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    startChangeEquip();
}

// .text 0x223B0
void dAcPlayer_c::endTrickChange(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOAD_INIT;
        }
    }
}

// .text 0x22410
void dAcPlayer_c::initTurnToTalker() {
    setAnm(HMN_ANM_NML_WALK1, l_moveParam.mWalkMorph, 0.0f, l_moveParam.mWalkRateMin, 16.0f, m3d::FORWARD_LOOP, FALSE);
    _22A4[7] = 0;
}

// .text 0x2246C
void dAcPlayer_c::executeTurnToTalker() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnmId == HMN_ANM_NPC_GRE_FRIENDLY1) {
            if (mAnm.isStop()) {
                setState(dPlayerMgr::STATE_DEMO_WAIT);
            }
            return;
        }
        dDemoActor_c *talker = dDemo::getTalkActor();
        if (talker != NULL) {
            mVec3_c *pos = talker->getDemoPosition();
            if (pos != NULL) {
                mTargetAngle = dActor_c::targetAngleY(&mPos, pos);
                bool done = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
                if (done) {
                    setAnm(HMN_ANM_NPC_GRE_FRIENDLY1, m3d::FORWARD_ONCE);
                }
            }
        }
        if (++_22A4[7] >= 500) {
            setAnm(HMN_ANM_NPC_GRE_FRIENDLY1, m3d::FORWARD_ONCE);
        }
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x22574
void dAcPlayer_c::endTurnToTalker(int nextState) {}

// .text 0x22578
void dAcPlayer_c::initTurnTo() {
    setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, l_moveParam.mWalkRateMin, 16.0f, m3d::FORWARD_LOOP, FALSE);
    f32 rate = (mTurnFrames / 500.0f) * l_moveParam.mWalkRateMin;
    if (rate > l_moveParam.mDashRateMin) {
        rate = l_moveParam.mDashRateMin;
    } else if (rate < l_moveParam.mWalkRateMin) {
        rate = l_moveParam.mWalkRateMin;
    }
    mAnm.setRate(rate);
    setNetWalkAnm();
}

// .text 0x2263C
void dAcPlayer_c::executeTurnTo() {
    checkFootSound();
    bool done = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    if (done) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x22694
void dAcPlayer_c::endTurnTo(int nextState) {}

// .text 0x22698
void dAcPlayer_c::initChangeHeldItemDemo() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
}

// .text 0x226A4
void dAcPlayer_c::executeChangeHeldItemDemo() {
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo && !(mActorFlags & dPlayerMgr::ACTOR_FLAG_DEMO)) {
        dItem::Item held = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld;
        if (held.mId == dItem::ITEM_ID_NONE) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
            if (held.getKind() == dItem::KIND_GOLD_AXE) {
                mReactionKind = dPlayerMgr::REACTION_GOLD_AXE;
                setState(dPlayerMgr::STATE_REACT);
            } else {
                setState(dPlayerMgr::STATE_TAKE_OUT_TOOL);
            }
        } else if (mHeldEquipStep == EQUIP_STEP_LOADED) {
            mHeldEquipStep = EQUIP_STEP_APPLIED;
            mToolScale.set(0.0f, 0.0f, 0.0f);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
        }
    }
}

// .text 0x22784
void dAcPlayer_c::endChangeHeldItemDemo(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
    }
}

// .text 0x227D4
void dAcPlayer_c::initAxeHitObject() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mObjcPos.mHitType != 0) {
            setAnmNoMorph(HMN_ANM_AXE_HANE1, m3d::FORWARD_ONCE);
        } else {
            setAnmNoMorph(HMN_ANM_AXE_SUKA1, m3d::FORWARD_ONCE);
        }
        getPosAtAngle(&mTargetPos, mAngle.y.mAngle, -8.0f);
        setHitObjectParam(getHitObjectParam(), mObjcPos.mHitType != 0);
    } else if (getHitObjectParam()->mHit) {
        setAnmNoMorph(HMN_ANM_AXE_HANE1, m3d::FORWARD_ONCE);
    } else {
        setAnmNoMorph(HMN_ANM_AXE_SUKA1, m3d::FORWARD_ONCE);
    }
}

// .text 0x228A0
void dAcPlayer_c::executeAxeHitObject() {
    if (mAnmId == HMN_ANM_AXE_HANE1) {
        if (mAnm.checkFrame(18.0f)) {
            startToolSound(SE_Boyon);
            startRumble(dRumble::PATTERN_AXE_BOUNCE);
            breakHitSnowmen();
            if (_2266 != dItem::ITEM_ID_NONE && _2278 == 0 && _227C == 0) {
                getNpcToolReaction();
            }
        }
    } else if (mAnmId == HMN_ANM_AXE_SUKA1) {
        if (mAnm.checkFrame(15.0f)) {
            startToolSound(SE_AxeKaraburi);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.getFrame() > 18.0f && mAnm.getFrame() < 38.0f) {
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.3f, 2.0f);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x22A08
void dAcPlayer_c::endAxeHitObject(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mObjcPos.init();
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
    }
}

// .text 0x22A60
void dAcPlayer_c::setHitObjectParam(hitObjectParam_s *param, u8 hit) {
    param->mHit = hit;
}

// .text 0x22A68
void dAcPlayer_c::initShovelHitObject() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mObjcPos.mHitType != 0 || mObjcPos2.mHitType != 0) {
            setAnmNoMorph(HMN_ANM_SCOOP_HANE1, m3d::FORWARD_ONCE);
            if (_2266 != dItem::ITEM_ID_NONE) {
                dDemoActor_c *npc = dNpcList::getNmlByKey(dItem::Item(_2266));
                if (npc != NULL) {
                    dPrivateData_c *data = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                    if (data != NULL) {
                        static_cast<dAcNpc_c *>(npc)->vt68(&data->mEquipment.mHeld);
                    }
                }
            }
        }
        getPosAtAngle(&mTargetPos, mAngle.y.mAngle, -8.0f);
        getShovelHitActorParam()->set(mObjcPos.mHitType != 0 || mObjcPos2.mHitType != 0);
    } else if (getHitObjectParam()->mHit) {
        setAnmNoMorph(HMN_ANM_SCOOP_HANE1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x22B94
void dAcPlayer_c::executeShovelHitObject() {
    if (mAnmId == HMN_ANM_SCOOP_HANE1 && mAnm.checkFrame(12.0f)) {
        startToolSound(SE_Boyon);
        startRumble(dRumble::PATTERN_SHOVEL_BOUNCE);
        breakHitSnowmen();
        if (_2266 != dItem::ITEM_ID_NONE && _2278 == 0 && _227C == 0) {
            getNpcToolReaction();
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        f32 frame = mAnm.getFrame();
        if (mAnmId != HMN_ANM_SCOOP_HANE1 || mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (frame > 12.0f && frame < 32.0f) {
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.3f, 2.0f);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x22CE8
void dAcPlayer_c::endShovelHitActor(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mObjcPos.init();
        mObjcPos2.init();
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
    }
}

// .text 0x22D48
void dAcPlayer_c::shovelHitActorParam_c::set(u8 hit) {
    mHit = hit;
}

// .text 0x22D50
void dAcPlayer_c::initChangeHair() {
    if (mPlayerNo == dNet::getMyNo()) {
        mStep = 0x11;
        setAnm(HMN_ANM_BARBER_CUT1, 9.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    }
}

// .text 0x22DC4
void dAcPlayer_c::executeChangeHair() {
    if (mPlayerNo == dNet::getMyNo()) {
        updateBarber(FALSE);
    }
}

// .text 0x22E08
void dAcPlayer_c::endChangeHair(int nextState) {
    mStep = 0x22;
    setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
}

// .text 0x22E50
void dAcPlayer_c::updateBarber(BOOL miiMask) {
    switch (mStep) {
    case 0x11:
        if (mAnm.checkFrame(12.0f)) {
            _2214 = 0;
            loadHead();
            takeOffMiiMask();
            setFaceAnm(mAnmId, m3d::FORWARD_ONCE, TRUE);
            mTexPatAnm.setFrame(12.0f, 0);
        } else if (mAnm.isStop()) {
            mStep = 0x12;
            setAnmNoMorph(miiMask ? HMN_ANM_BARBER_MII2 : HMN_ANM_BARBER_CUT2, m3d::FORWARD_ONCE);
        }
        break;
    case 0x12:
        if (mAnm.isStop()) {
            mStep = 0x13;
            setAnmNoMorph(miiMask ? HMN_ANM_BARBER_MII3 : HMN_ANM_BARBER_CUT3, m3d::FORWARD_ONCE);
        }
        break;
    case 0x13:
        if (mAnm.checkFrame(23.0f)) {
            if (miiMask) {
                putOnMiiMask();
                _2214 = getHairNo();
            } else {
                _2214 = getHairNo();
                _2218 = getSkinColorNo();
                loadHead();
                setFaceAnm(mAnmId, m3d::FORWARD_ONCE, TRUE);
                mTexPatAnm.setFrame(23.0f, 0);
            }
        } else if (mAnm.isStop()) {
            mStep = 0x14;
        }
        break;
    case 0x14:
        break;
    default:
        setState(dPlayerMgr::STATE_DEMO_WAIT);
        break;
    }
}

// .text 0x23028
void dAcPlayer_c::initChangeMiiMask() {
    mStep = 0x11;
    setAnm(HMN_ANM_BARBER_MII1, 9.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x23058
void dAcPlayer_c::executeChangeMiiMask() {
    if (mPlayerNo == dNet::getMyNo()) {
        updateBarber(TRUE);
    }
}

// .text 0x2309C
void dAcPlayer_c::endChangeMiiMask(int nextState) {
    mStep = 0x22;
    setAnm(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
}

// .text 0x230E4
void dAcPlayer_c::initEatSpecial() {
    setAnm(HMN_ANM_MENU_EAT2, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        getEatSpecialParam()->set(mActionItem.mId);
    } else {
        mActionItem = getEatSpecialParam()->mItem;
    }
    mTargetAngle = 0;
}

// .text 0x23164
void dAcPlayer_c::executeEatSpecial() {
    if (mAnmId == HMN_ANM_MENU_EAT2) {
        calcEatEffect();
        if (mAnm.getFrame() > 200.0f) {
            holdSound(SE_Happy);
            mLevelEffect3.create("af_mnp_delicious", &mMdlCallback.mFeelMtx, 0);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        updateEat(TRUE);
    } else if (mAnmId != HMN_ANM_MENU_THINK2) {
        updateEat(FALSE);
    }
}

// .text 0x23210
void dAcPlayer_c::endEatSpecial(int nextState) {
    _21D0[9] = 0.0f;
    _21D0[10] = 0.0f;
}

// .text 0x23224
void dAcPlayer_c::eatSpecialParam_c::set(u16 item) {
    mItem = item;
}

// .text 0x2322C
void dAcPlayer_c::initFillHoleWait() {
    setAnm(HMN_ANM_HOLE_BURY_WAIT1, 9.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
        mTargetAngle = dActor_c::targetAngleY(&mPos, &pos);
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        _22A4[7] = 9;
    }
}

// .text 0x232C8
void dAcPlayer_c::executeFillHoleWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (_22A4[7] != 0) {
            _22A4[7]--;
        }
        if (_229C != -1) {
            bool ready = false;
            bool turned = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
            if (_22A4[7] == 0 && turned) {
                ready = true;
            }
            waitFieldTask(0, ready, 0);
        } else {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x23390
void dAcPlayer_c::endFillHoleWait(int nextState) {}

// .text 0x23394
void dAcPlayer_c::initMiiMaskImmediate() {
    mTargetAngle = 0;
    if (mPlayerNo == dNet::getMyNo()) {
        if (mEquipChangeMode != 0) {
            setAnm(HMN_ANM_NML_CHANGE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        }
    } else {
        setAnm(HMN_ANM_NML_CHANGE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x23448
void dAcPlayer_c::executeMiiMaskImmediate() {
    if (mAnmId == HMN_ANM_NML_CHANGE1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_wear);
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        } else if (mAnm.checkFrame(38.0f)) {
            createChangeEffect();
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mEquipChangeMode == 0) {
            if (dScene::getCurrentScene() == dScene::SCENE_RM_BARBER) {
                if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mMiiMask == 0) {
                    startSound(SE_EqOff);
                } else {
                    startSound(SE_EqOn);
                }
            }
            updateMiiMask();
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
            setState(dPlayerMgr::STATE_DEMO_WAIT);
        } else {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x7D0);
            if (mAnm.isStop()) {
                setState(dPlayerMgr::STATE_DEMO_WAIT);
            } else if (mAnm.checkFrame(24.0f)) {
                updateMiiMask();
            }
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x7D0);
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(24.0f)) {
            updateMiiMask();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
}

// .text 0x23620
void dAcPlayer_c::endMiiMaskImmediate(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING)) {
        updateMiiMask();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x23678
void dAcPlayer_c::updateMiiMask() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) && priv->mMiiMask != 0) {
        putOnMiiMask();
    } else {
        freeAccessoryHeap();
        loadHead();
        takeOffMiiMask();
    }
}

// .text 0x236E8
void dAcPlayer_c::initChangeOutfitAndHeld() {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    } else {
        setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
    }
    mTargetAngle = 0;
    mStep = 8;
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        getOutfitAndHeldParam()->set(mEquipFlags, mHeldItemChange, mStep);
    } else {
        dPlayerMgr::Mgr_c::getNetPlayer(playerNo);
        if (!dPlayerMngr_c::isEquipChecked(mPlayerNo)) {
            u16 flags = getOutfitAndHeldParam()->mEquipFlags;
            if (flags & dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY) {
                mEquipFlags &= ~dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY;
            } else if (flags & dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY) {
                mEquipFlags &= ~dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY;
            } else if (flags & dPlayerMgr::EQUIPMENT_HAT_NONE) {
                mEquipFlags &= ~dPlayerMgr::EQUIPMENT_HAT_ANY;
            } else if ((flags & dPlayerMgr::EQUIPMENT_HAT_CAP) || (flags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK) ||
                       (flags & dPlayerMgr::EQUIPMENT_HAT_MASK) || (flags & dPlayerMgr::EQUIPMENT_HAT_MASK_2) ||
                       (flags & dPlayerMgr::EQUIPMENT_HAT_OVER_HEAD)) {
                mEquipFlags &= ~dPlayerMgr::EQUIPMENT_HAT_STYLE;
            }
            mEquipFlags |= flags;
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
            mHeldItemChange = getOutfitAndHeldParam()->mHeldItemChange;
        }
        mEquipStep = EQUIP_STEP_LOAD;
        mHeldEquipStep = EQUIP_STEP_LOAD;
        mStep = getOutfitAndHeldParam()->mStep;
    }
}

// .text 0x23844
void dAcPlayer_c::executeChangeOutfitAndHeld() {
    switch (mStep) {
    case 8:
        if ((mAnmId == HMN_ANM_MENU_CHANGE1 || mAnmId == HMN_ANM_NML_CHANGE1) && mAnm.isStop()) {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
        if (mEquipStep == EQUIP_STEP_LOADED && mHeldEquipStep == EQUIP_STEP_LOADED) {
            if (mAnmId == HMN_ANM_MENU_CHANGE1 || mAnmId == HMN_ANM_NML_CHANGE1) {
                mAnm.setFrame(0.0f);
            } else {
                BOOL mask = FALSE;
                if (mPlayerNo == dNet::getMyNo() && (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU)) {
                    mask = TRUE;
                }
                setAnm(mask ? HMN_ANM_NML_CHANGE1 : HMN_ANM_MENU_CHANGE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE,
                       FALSE);
            }
            mTargetAngle = 0;
            mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / 6.0f;
            if (mTurnFrames <= 0) {
                mTurnFrames = 1000;
            }
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            mStep = 9;
            startSound(SE_wear);
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        }
        break;
    case 9:
        if (mAnmId == HMN_ANM_MENU_CHANGE1 || mAnmId == HMN_ANM_NML_CHANGE1) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            if (mAnm.getFrame() > 24.0f) {
                if (mEquipStep == EQUIP_STEP_LOADED) {
                    mEquipStep = EQUIP_STEP_APPLIED;
                }
                if (mHeldEquipStep == EQUIP_STEP_LOADED) {
                    mHeldEquipStep = EQUIP_STEP_APPLIED;
                }
            }
            if (mAnm.isStop()) {
                if (mPlayerNo == dNet::getMyNo()) {
                    setState((mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) ? dPlayerMgr::STATE_DEMO_WAIT
                                                                                      : dPlayerMgr::STATE_DEMO_START);
                } else {
                    setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
                }
            } else if (mAnm.checkFrame(24.0f)) {
                if (mEquipStep == EQUIP_STEP_LOADED) {
                    mEquipStep = EQUIP_STEP_APPLIED;
                }
                if (mHeldEquipStep == EQUIP_STEP_LOADED) {
                    mHeldEquipStep = EQUIP_STEP_APPLIED;
                }
            } else if (mAnm.checkFrame(38.0f)) {
                createChangeEffect();
            }
        }
        break;
    default:
        if (mPlayerNo == dNet::getMyNo()) {
            setState((mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) ? dPlayerMgr::STATE_DEMO_WAIT
                                                                              : dPlayerMgr::STATE_DEMO_START);
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
        break;
    }
}

// .text 0x23B50
void dAcPlayer_c::endChangeOutfitAndHeld(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOAD_INIT;
        }
        if (mHeldEquipStep == EQUIP_STEP_LOADED) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            mHeldEquipStep = EQUIP_STEP_INIT_LOADED;
            mToolScale.set(1.0f, 1.0f, 1.0f);
        } else if (mHeldEquipStep == EQUIP_STEP_LOAD) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            mHeldEquipStep = EQUIP_STEP_LOAD_INIT;
            mToolScale.set(1.0f, 1.0f, 1.0f);
        }
    }
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU;
}

// .text 0x23C24
void dAcPlayer_c::outfitAndHeldParam_c::set(u16 equipFlags, u8 heldItemChange, u8 step) {
    mEquipFlags = equipFlags;
    mHeldItemChange = heldItemChange;
    mStep = step;
}

// .text 0x23C34
void dAcPlayer_c::initSwapItem() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo && _22A0 == -1) {
        _22A0 = dFgobjManager::swapItem(playerNo, _218C);
    }
}

// .text 0x23C94
void dAcPlayer_c::executeSwapItem() {
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        if (_22A0 == -1) {
            _22A0 = dFgobjManager::swapItem(playerNo, _218C);
        } else if (checkPutEffectDone()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    }
}

// .text 0x23D08
void dAcPlayer_c::endSwapItem(int nextState) {}

// .text 0x23D0C
void dAcPlayer_c::initPutHeldItem() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo && _22A0 == -1) {
        _22A0 = dFgobjManager::putHeldItem(playerNo, mActionItem.mId);
    }
}

// .text 0x23D6C
void dAcPlayer_c::executePutHeldItem() {
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        if (_22A0 == -1) {
            _22A0 = dFgobjManager::putHeldItem(playerNo, mActionItem.mId);
        } else if (checkPutEffectDone()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    }
}

// .text 0x23DE0
void dAcPlayer_c::endPutHeldItem(int nextState) {}

// .text 0x23DE4
void dAcPlayer_c::setState(int state) {
    if ((u32)state < dPlayerMgr::STATE_COUNT) {
        endState(state);
        mpToolBank->stopWind();
        setStateObjc(state);
        const stateProc_s *proc = &sStateProcs[state];
        if (proc->mInit) {
            (this->*proc->mInit)();
        }
        mState = state;
        if (mPlayerNo == dNet::getMyNo()) {
            mSyncValue = dNet::getNetFrame();
            if (dPlayerMgr::isActionParamLocked(state)) {
                sendStatePacket();
            }
        }
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE;
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_UNIT_ACTION;
        int remote = dPlayerMngr_c::getRemoteState(mPlayerNo);
        if (remote != dPlayerMgr::STATE_NONE && remote != state) {
            dPlayerMgr::cacheRemoteState(mPlayerNo, dPlayerMgr::STATE_NONE);
        }
    }
}

// .text 0x23EE8
void dAcPlayer_c::endState(int nextState) {
    if ((u32)mState < dPlayerMgr::STATE_COUNT && sStateProcs[mState].mEnd) {
        (this->*sStateProcs[mState].mEnd)(nextState);
    }
}

// .text 0x23F70
void dAcPlayer_c::changeStateCallback(int playerNo, int state) {
    dAcPlayer_c *player = dPlayerMgr::getActorRaw(playerNo);
    if (player != NULL) {
        player->setState(state);
    }
}

// .text 0x23FAC
void dAcPlayer_c::clearFrameRequests() {
    checkEquipReady();
    checkToolReady();
    if (mPlayerNo == dNet::getMyNo()) {
        dPlayerMgr::clearAxeInFountain();
    }
    mFgObjItem = dItem::ITEM_ID_NONE;
}

// .text 0x24000
void dAcPlayer_c::calcMove() {
    mManpu.execute(&mMdlCallback.mFeelMtx, mAnmId, &mAngle.y, NULL, mAnm.getFrame());
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LOOK_AT) {
        bool yawDone = sLib::chaseAngle(&mMdlCallback.mHeadRotY, mLookTilt, _22CC);
        bool tiltDone = sLib::chaseAngle(&mMdlCallback.mHeadRotX, mLookYaw, _22CA);
        if (yawDone && tiltDone) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_LOOK_AT;
        }
    }
    sLib::chaseAngle(&mAngle.x.mAngle, _2050, 0x15E);
    calcSlopeSpeed();
    sLib::chase(&mSpeedF, mTargetSpeed, _21B8);
    if (mSpeedF > l_moveParam.mMaxSpeed) {
        mSpeedF = l_moveParam.mMaxSpeed;
    }
    calcSpeed();
    mPos.x += mSpeed.x;
    mPos.y += mSpeed.y;
    mPos.z += mSpeed.z;
    mMdlCallback.mCurveBaseY = mPos.y;
    mAngle3D = mAngle;
    if (mState == dPlayerMgr::STATE_WALK &&
        dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        checkUnitChange();
        checkWalkIntoTree();
    }
    calcFaceExpression();
    applyEquipment();
    applyTool();
    updateSunTimer();
    updateSlipEffect();
    calcSound();
}

// .text 0x241A4
void dAcPlayer_c::calcSlopeSpeed() {
    if (mTargetSpeed == 0.0f) {
        return;
    }
    mVec3_c normal = mAcch.mGroundNormal;
    if (normal.x == 0.0f && normal.z == 0.0f) {
        return;
    }
    f32 min;
    f32 max = l_moveParam.mRunSpeed;
    f32 rate;
    if (mTargetSpeed > max) {
        min = l_moveParam.m4C;
        if (mTargetSpeed <= min) {
            return;
        }
        max = l_moveParam.mMaxSpeed;
        rate = l_moveParam.m40;
    } else if (mTargetSpeed > l_moveParam.mWalkSpeed) {
        min = l_moveParam.m48;
        if (mTargetSpeed <= min) {
            return;
        }
        rate = l_moveParam.m3C;
    } else {
        min = l_moveParam.m44;
        if (mTargetSpeed <= min) {
            return;
        }
        rate = l_moveParam.m38;
        max = l_moveParam.mWalkSpeed;
    }
    mTargetSpeed += rate * getSpeedF(&normal);
    if (mTargetSpeed < min) {
        mTargetSpeed = min;
    } else if (mTargetSpeed > max) {
        mTargetSpeed = max;
    }
}

// .text 0x242E0
void dAcPlayer_c::setMoveTarget(const mVec3_c *pos, f32 speed) {
    mTargetPos = *pos;
    mTargetSpeed = speed;
}

// .text 0x24300
void dAcPlayer_c::startMoveToTarget() {
    if (mTargetSpeed) {
        _21B8 = 0.22f;
    } else {
        _21B8 = l_moveParam.mMoveAccel;
    }
    _205C = mPos;
    mSpeedF = mTargetSpeed;
    _2052 = dActor_c::targetAngleY(&mPos, &mTargetPos);
}

// .text 0x24388
BOOL dAcPlayer_c::moveToTarget() {
    mVec3_c total = mTargetPos - _205C;
    f32 totalDistSq = total.x * total.x + total.z * total.z;
    mVec3_c moved = mPos - _205C;
    if (totalDistSq <= moved.x * moved.x + moved.z * moved.z) {
        goto arrived;
    }
    {
        mVec3_c rest = mTargetPos - mPos;
        f32 restDistSq = rest.x * rest.x + rest.z * rest.z;
        if (mSpeedF * mSpeedF > restDistSq) {
        arrived:
            mPos = mTargetPos;
            mTargetSpeed = 0.0f;
            mSpeedF = 0.0f;
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x2447C
void dAcPlayer_c::setStateObjc(int state) {
    if (mPlayerNo == dNet::getMyNo()) {
        mObjc.mWeight = l_stateObjcFixed[state] ? 0xFF : 0x80;
        switch (state) {
        case dPlayerMgr::STATE_DEMO_WAIT:
            mObjc.mWeightFunc = objcWeightTalk;
            break;
        case dPlayerMgr::STATE_PUT_AWAY_TOOL:
            mObjc.mWeightFunc = objcWeightTalkOnly;
            break;
        case dPlayerMgr::STATE_IDLE:
            mObjc.mWeightFunc = objcWeightRemote;
            break;
        case dPlayerMgr::STATE_AXE_HIT:
        case dPlayerMgr::STATE_AXE_HIT_OBJECT:
        case dPlayerMgr::STATE_SHOVEL_HIT:
        case dPlayerMgr::STATE_SHOVEL_HIT_OBJECT:
            mObjc.mWeightFunc = objcWeightFixed;
            break;
        default:
            mObjc.mWeightFunc = NULL;
            break;
        }
        if (l_stateObjcFlag[state]) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_DEMO;
        }
    } else if (state == dPlayerMgr::STATE_IDLE) {
        mObjc.mWeightFunc = objcWeightEnter;
    } else {
        mObjc.mWeightFunc = NULL;
    }
    switch (l_stateMoveReset[state]) {
    case 0:
        stopMove();
        break;
    case 1:
        stopMoveAll();
        break;
    case 2:
        if (mPlayerNo != dNet::getMyNo()) {
            stopMoveAll();
        }
        break;
    }
}

// .text 0x24628
BOOL dAcPlayer_c::setAnm(int anmId, int mode) {
    return setAnm(anmId, 60.0f, 0.0f, 1.0f, 16.0f, mode, FALSE);
}

// .text 0x24648
BOOL dAcPlayer_c::setAnmNoMorph(int anmId, int mode) {
    return setAnm(anmId, 0.0f, 0.0f, 1.0f, 16.0f, mode, FALSE);
}

// .text 0x24668
BOOL dAcPlayer_c::setAnm(int anmId, f32 morph, f32 frame, f32 rate, f32 blend, int mode, BOOL force) {
    if ((u32)anmId >= HMN_ANM_NUM) {
        return FALSE;
    }
    if (anmId == mAnmId && !force) {
        return FALSE;
    }
    mAnm.setAnm(mMdl, dHmnAnm_c::getResAnmChr(anmId), (m3d::playMode_e)mode);
    mAnm.setFrame(frame);
    mAnm.setRate(rate);
    mMdl.setAnm(mAnm, morph);
    setToolAnm(anmId, FALSE, blend);
    setFaceAnm(anmId, mode, force);
    setFaceExprAnm(anmId);
    mAnmId = anmId;
    mMdlCallback.mAnmId = anmId;
    return TRUE;
}

// .text 0x24788
BOOL dAcPlayer_c::setToolAnm(int anmId, BOOL force, f32 blend) {
    if (mToolScale.x <= 0.0f && !force) {
        return FALSE;
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
        switch (dHmnAnm_c::getType(mAnmId)) {
        case 0:
        case 1:
        case 2:
        case 3:
            mMdl.removePartAnm(1, blend);
            break;
        }
    }
    switch (dHmnAnm_c::getType(anmId)) {
    case 0:
        setToolPartAnm(blend, 1);
        break;
    case 1:
        if (force) {
            setToolPartAnm(blend, 1);
        }
        break;
    case 3:
        setToolPartAnm(blend, 0);
        break;
    case 2: {
        int type = mpToolBank->getToolType();
        if (type == HMN_TOOL_UMBRELLA || type == HMN_TOOL_BALLOON || type == HMN_TOOL_WINDMILL) {
            setToolPartAnm(blend, 1);
        }
        break;
    }
    }
    return TRUE;
}

// .text 0x1E2B4
void dAcPlayer_c::setToolPartAnm(f32 blend, BOOL bothArms) {
    dItem::Item item = mpToolBank->mItem;
    nw4r::g3d::ResAnmChr anmChr = dHmnAnm_c::getItemToolResAnmChr(&item);
    if (anmChr.IsValid()) {
        mPartAnm.setAnm(mMdl, anmChr, m3d::FORWARD_ONCE);
        mMdl.setPartAnm(1, &mPartAnm, blend);
        mMdl.setPartNode(1, "Rarm1");
        if (bothArms) {
            int type = dHmnToolBank_c::getItemToolType(&item);
            if (type == HMN_TOOL_SCOOP || type == HMN_TOOL_AXE) {
                mMdl.setPartNode(1, "Larm1");
            } else {
                mMdl.setPartNode(0, "Larm1");
            }
        }
    }
}

// .text 0x248D8
void dAcPlayer_c::getFrontPos(mVec3_c *out) {
    getPosAtAngle(out, mAngle.y, 32.0f);
}

// .text 0x248E8
void dAcPlayer_c::getPosAtAngle(mVec3_c *out, s16 angle, f32 dist) {
    calcPosAtAngle(out, &mPos, angle, dist);
}

// .text 0x248F4
void dAcPlayer_c::calcPosAtAngle(mVec3_c *out, const mVec3_c *pos, s16 angle, f32 dist) {
    out->x = pos->x + dist * nw4r::math::SinIdx(angle);
    out->z = pos->z + dist * nw4r::math::CosIdx(angle);
    mVec3_c ground(out->x, pos->y, out->z);
    out->y = dBGCF::getGroundY(&ground, FALSE);
}

// .text 0x249C0
ulong dAcPlayer_c::getToolNodeIdx() {
    switch (mpToolBank->getToolType()) {
    case HMN_TOOL_NET:
    case HMN_TOOL_UMBRELLA:
        return 3;
    case HMN_TOOL_SCOOP:
    case HMN_TOOL_WATERING:
    case HMN_TOOL_FLOWER:
    case HMN_TOOL_CRACKER:
    case HMN_TOOL_HANABI:
    case HMN_TOOL_SYABON:
        return 1;
    default:
        return 0;
    }
}

// .text 0x24A18
void dAcPlayer_c::stopMove() {
    _2050 = 0;
    _21B8 = l_moveParam.mMoveAccel;
    mTargetSpeed = 0.0f;
    mAngle3D.y = _2052 = mAngle.y;
}

// .text 0x24A4C
void dAcPlayer_c::stopMoveAll() {
    stopMove();
    mSpeedF = 0.0f;
}

// .text 0x24A84
BOOL dAcPlayer_c::checkActionInput(BOOL other) {
    if (mPlayerNo != dNet::getMyNo()) {
        return FALSE;
    }
    if (checkTalkInput()) {
        return TRUE;
    }
    if (checkPickUpInput()) {
        return TRUE;
    }
    if (checkToolSwingInput()) {
        startRumble(dRumble::PATTERN_TOOL_SWING);
        return TRUE;
    }
    if (other && startPitfallTrigger()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x24B48
BOOL dAcPlayer_c::checkDpadInput() {
    if (_22FA[2] == 0 && _22FA[3] == 0) {
        return FALSE;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        if (_22FA[2] == 2) {
            dItem::Item next = dMenuMng_c::getNextTool();
            if (canHoldTool(&next)) {
                dMenuMng_c::swapHeld();
                changeHeldItem(&next);
                return TRUE;
            }
        } else if (_22FA[2] == 4) {
            dItem::Item prev = dMenuMng_c::getPrevTool();
            if (canHoldTool(&prev)) {
                dMenuMng_c::swapHeld();
                changeHeldItem(&prev);
                return TRUE;
            }
        } else if (_22FA[3] == 1) {
            if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld.mId != dItem::ITEM_ID_NONE &&
                (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT)) {
                if (dMenuMng_c::canPutAwayHeld()) {
                    dMenuMng_c::putAwayHeld();
                    dItem::Item none;
                    changeHeldItem(&none);
                    return TRUE;
                }
                setState(dPlayerMgr::STATE_PUT_AWAY_TOOL_FULL);
                return TRUE;
            }
        } else if (_22FA[3] == 3) {
            if (mState == dPlayerMgr::STATE_IDLE && dCamera_c::sInstance != NULL) {
                dCamera_c::sInstance->resetLookUpTimer();
            }
        }
    } else if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL && dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_FTR_ROOM)) {
            int yaw = camera->mRoomYaw;
            if (_22FA[2] == 2) {
                yaw++;
            } else if (_22FA[2] == 4) {
                yaw--;
            }
            if (yaw < 0) {
                yaw = 0;
            } else if (yaw > 2) {
                yaw = 2;
            }
            int pitch = camera->mRoomPitch;
            if (_22FA[3] == 3) {
                pitch++;
            } else if (_22FA[3] == 1) {
                pitch--;
            }
            if (pitch < 0) {
                pitch = 0;
            } else if (pitch > 2) {
                pitch = 2;
            }
            camera->setRoomView(yaw, pitch);
        }
    }
    return FALSE;
}

// .text 0x24D84
BOOL dAcPlayer_c::canHoldTool(const dItem::Item *item) {
    if (item->getHideBone() != 9) {
        return FALSE;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) &&
        !dHmnToolBank_c::isHandTool(item)) {
        return FALSE;
    }
    return TRUE;
}

// .text 0x24DF0
void dAcPlayer_c::changeHeldItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        mHeldItemChange = dPlayerMgr::HELD_ITEM_CHANGE_REMOVE;
        mHeldEquipStep = EQUIP_STEP_LOAD;
    } else {
        mHeldItemChange = dPlayerMgr::HELD_ITEM_CHANGE_EQUIP;
        mHeldEquipStep = EQUIP_STEP_LOAD;
    }
    dPlayerMngr_c::setLocalPlayerEquip(dNet::getMyNo(), dEquip_c::PART_HELD, item->mId);
    dPlayerMngr_c::setLocalEquip(dNet::getMyNo(), 3, item->mId);
    mStep = 8;
    setState(dPlayerMgr::STATE_CHANGE_HELD_ITEM);
}

// .text 0x24E84
BOOL dAcPlayer_c::checkTalkInput() {
    if (_22FE[5] == 0) {
        return FALSE;
    }
    dDemoActor_c *actor = dDemoActor_c::findInteraction(this);
    if (actor != NULL && !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_NO_TALK)) {
        dDemo::requestTalk(this, actor);
        _21B4 = actor;
        return TRUE;
    }
    if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_NO_USE) && checkUseInput()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x24F24
BOOL dAcPlayer_c::checkToolSwingInput() {
    if (_2304[1] == 0 || !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        return FALSE;
    }
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT)) {
        return FALSE;
    }
    switch (mpToolBank->getToolType()) {
    case HMN_TOOL_ROD:
        startFishingCast();
        return TRUE;
    case HMN_TOOL_AXE:
        if (checkAxeIntoFountain()) {
            return TRUE;
        }
        setState(dPlayerMgr::STATE_AXE_SWING);
        return TRUE;
    case HMN_TOOL_NET:
        setState(dPlayerMgr::STATE_NET_SWING);
        return TRUE;
    }
    return FALSE;
}

// .text 0x25004
BOOL dAcPlayer_c::checkUseInput() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        return checkToolUseInput();
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        return checkFtrGrabInput();
    }
    return FALSE;
}

// .text 0x25068
BOOL dAcPlayer_c::checkFtrGrabInput() {
    if (_22FE[5] == 0) {
        return FALSE;
    }
    if (dFTR::holdList_c::isHoldScene() && dFTR::getHoldList()->get(0) != NULL) {
        setState(dPlayerMgr::STATE_FTR_GRAB);
        return TRUE;
    }
    return FALSE;
}

// .text 0x250DC
BOOL dAcPlayer_c::checkToolUseInput() {
    if (_22FE[5] == 0) {
        return FALSE;
    }
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT)) {
        return startShakeTree();
    }
    int type = mpToolBank->getToolType();
    switch (type) {
    case HMN_TOOL_UMBRELLA:
        if (startShakeTree()) {
            return TRUE;
        }
        break;
    case HMN_TOOL_BALLOON:
    case HMN_TOOL_WINDMILL:
        return startShakeTree();
    case HMN_TOOL_ROD:
        startFishingCast();
        return TRUE;
    case HMN_TOOL_AXE:
        if (checkAxeIntoFountain()) {
            return TRUE;
        }
        break;
    }
    setState(l_toolStates[type]);
    return TRUE;
}

// .text 0x251D8
void dAcPlayer_c::startFishingCast() {
    mVec3_c pos;
    getPosAtAngle(&pos, getActionAngle(), 94.0f);
    if (dBGCF::getGroundY(&pos, FALSE) <= mPos.y && dBGCF::isWaterArea(&pos, 24.0f)) {
        mpToolBank->mFloat.setTarget(&pos);
        setState(dPlayerMgr::STATE_FISHING_CAST);
    } else {
        mpToolBank->mFloat.setTarget(&pos);
        setState(dPlayerMgr::STATE_FISHING_CAST_MISS);
    }
}

// .text 0x25290
BOOL dAcPlayer_c::checkAxeIntoFountain() {
    if (!dNet::isSessionActive()) {
        dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayerRaw(mPlayerNo);
        if (priv != NULL && !priv->isFlag0(PRIVATE_FLAG0_TRAVELLING) && !priv->isFlag1(PRIVATE_FLAG1_FOUNTAIN_AXE)) {
            int kind = mpToolBank->mItem.getKind();
            if (kind != dItem::KIND_GOLD_AXE && kind != dItem::KIND_SILVER_AXE) {
                if (dSTR::isAtFountainFront(mPos, (u16)mAngle.y.mAngle, &mTargetPos, &mShowPos, FALSE)) {
                    BOOL ok = TRUE;
                    int num = dNpcList::countSp();
                    for (int i = 0; i < num; i++) {
                        dDemoActor_c *sp = dNpcList::getSp(i);
                        if (sp != NULL && sp->mProfName == fProf::A_NPC_SP_CEREMONY_SONCHO &&
                            (int)dSaveData_c::getTown()->mNewConstruction == NEW_CONSTRUCTION_FOUNTAIN) {
                            ok = FALSE;
                        }
                    }
                    if (ok) {
                        dPlayerMgr::setAxeInFountain();
                        setState(dPlayerMgr::STATE_IDLE);
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

// .text 0x253D0
BOOL dAcPlayer_c::checkPickUpInput() {
    if (_22FE[3] == 0) {
        return FALSE;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        return checkPickUpOutdoor();
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        return checkPickUpRoom();
    }
    return FALSE;
}

// .text 0x25448
BOOL dAcPlayer_c::checkPickUpRoom() {
    if (!dScene::isCurrentSceneMyHouse()) {
        return FALSE;
    }
    int room;
    if (dHomeList_c::getHomeFromScene(&room, dScene::getCurrentScene()) == -1) {
        return FALSE;
    }
    if (room == 3) {
        return FALSE;
    }
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    dFdBase_c *field;
    if (!dSTR::isAlone()) {
        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_NOT_ALONE;
    } else {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_NOT_ALONE;
    }
    if (priv->findEmptyPocket(0) == -1) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
    } else {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
    }
    putUnit_c unit;
    int ftr = dFtrm_c::findPutUnit(&unit, TRUE);
    if (pickUpFromUnit(ftr, &unit)) {
        return TRUE;
    }
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) || (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NOT_ALONE)) {
        field = dField::getInfo(0);
        if (field != NULL) {
            if (ftr == dFtrm_c::PUT_UNIT_ITEM) {
                setState(dPlayerMgr::STATE_PICK_UP_UNIT_ITEM);
                return TRUE;
            }
            mVec3_c pos;
            getFrontUnit(&pos);
            if (getFieldRealItem(&pos, 0, field) != dItem::ITEM_ID_NONE) {
                setState(dPlayerMgr::STATE_PICK_UP_UNIT_ITEM);
                return TRUE;
            }
        }
    } else if (ftr == dFtrm_c::PUT_UNIT_ITEM) {
        int unitZ = unit.mZ;
        int unitX = unit.mX;
        mTargetUnitX = unitX;
        mTargetUnitZ = unitZ;
        _229C = dFgobjManager::pickUp(mPlayerNo, unitX, unitZ);
        if (_229C != -1) {
            setState(dPlayerMgr::STATE_PICK_UP_REACH);
            return TRUE;
        }
    } else {
        mVec3_c pos;
        getFrontUnit(&pos);
        _229C = dFgobjManager::pickUp(mPlayerNo, mTargetUnitX, mTargetUnitZ);
        if (_229C != -1) {
            setState(dPlayerMgr::STATE_PICK_UP_REACH);
            return TRUE;
        }
    }
    ftr = dFtrm_c::findPutUnit(&unit, &mPos, mAngle.y, FALSE);
    if (pickUpFromUnit(ftr, &unit)) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x256B4
BOOL dAcPlayer_c::checkPickUpOutdoor() {
    mVec3_c pos;
    if (mpToolBank->getToolType() != HMN_TOOL_SCOOP) {
        getNearFrontUnit(&pos);
        int x = mTargetUnitX;
        int z = mTargetUnitZ;
        if (!daSnowMan_c::isAtUnit(x, z) && canWalkToUnit(x, z)) {
            _229C = dFgobjManager::fillHole(mPlayerNo, x, z, 0);
            if (_229C != -1) {
                setState(dPlayerMgr::STATE_FILL_HOLE_WAIT);
                return TRUE;
            }
        }
    }
    getFrontUnit(&pos);
    int x = mTargetUnitX;
    int z = mTargetUnitZ;
    if (canWalkToUnit(x, z)) {
        _229C = dFgobjManager::pickUp(mPlayerNo, x, z);
        if (_229C != -1) {
            setState(dPlayerMgr::STATE_PICK_UP_REACH);
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x257D0
BOOL dAcPlayer_c::checkMoveOrAction() {
    if (mMoveSpeed) {
        if (mState == dPlayerMgr::STATE_WATER) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_LEFT_WATER;
            setAnm(HMN_ANM_NML_WAIT1, 60.0f, 0.0f, 1.0f, 70.0f, m3d::FORWARD_LOOP, FALSE);
        }
        setState(dPlayerMgr::STATE_IDLE);
        return TRUE;
    }
    return checkActionInput(FALSE);
}

// .text 0x25860
BOOL dAcPlayer_c::pickUpFromUnit(int ftr, const putUnit_c *unit) {
    if (ftr < 0) {
        return FALSE;
    }
    _2268 = ftr;
    setTargetUnit(unit->mX, unit->mZ);
    mPickUpKind = unit->mLayer;
    _2188 = unit->mItem;
    mActionItem = unit->mDirItem;
    if (mPickUpKind == 0) {
        setState(dPlayerMgr::STATE_PICK_UP);
        return TRUE;
    }
    if (mPickUpKind == 1) {
        setState(dPlayerMgr::STATE_PICK_UP);
        return TRUE;
    }
    return FALSE;
}

// .text 0x258EC
BOOL dAcPlayer_c::isRafflesiaAt(const mVec3_c *pos) {
    if (nw4r::math::FAbs(mPos.y - dBGCF::getGroundY(pos, FALSE)) > 16.0f) {
        return FALSE;
    }
    dFdBase_c *field = dField::getInfo(0);
    if (field == NULL) {
        return FALSE;
    }
    dItem::Item *item = field->getItem(pos, 0);
    if (item != NULL && (item->isWiltedRafflesia() || item->isRafflesia())) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x259C4
void dAcPlayer_c::breakHitSnowmen() {
    if (_2278 != 0) {
        daSnowMan_c *snowman = (daSnowMan_c *)_2278;
        if (snowman != NULL) {
            snowman->breakBall();
        }
    }
    if (_227C != 0) {
        daSnowMan_c *snowman = (daSnowMan_c *)_227C;
        if (snowman != NULL) {
            snowman->breakBall();
        }
    }
}

// .text 0x25A18
int dAcPlayer_c::getNpcToolReaction() {
    int result = 2;
    dAcNpc_c *npc = (dAcNpc_c *)dNpcList::getNmlByKey(dItem::Item(_2266));
    if (npc != NULL) {
        dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (priv != NULL) {
            result = npc->vt68(&priv->mEquipment.mHeld);
        }
    }
    return result;
}

// Unreferenced folded statics retained in the original REL; owner stripped.
void dAPlayer_retainEffectConstants() {
    static const f32 l_unknown2650 = 100.0f;
    static const u8 l_unknown2654 = 9;
    static const f32 l_unknown2658 = 20.0f;
    static const u8 l_unknown265C = 12;
}

// .text 0x25AA4
void dAcPlayer_c::createHeldToolGlitter() {
    static const int l_glitterCount = 7;

    static const f32 l_glitterOfsX[l_glitterCount] = {0.0f, l_glitterCount, l_glitterCount, 0.0f, 1.0f, 3.0f, 3.0f};
    static const f32 l_glitterOfsY[l_glitterCount] = {0.0f, -3.0f, -3.0f, -2.0f, -2.0f, -3.0f, -3.0f};
    static const f32 l_glitterOfsZ[l_glitterCount] = {0.0f, 20.0f, 20.0f, 14.0f, 16.0f, 10.0f, 10.0f};
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return;
    }
    dItem::Item held = priv->mEquipment.mHeld;
    BOOL gold;
    switch (held.getKind()) {
    case dItem::KIND_GOLD_FISHINGROD:
    case dItem::KIND_GOLD_SCOOP:
    case dItem::KIND_GOLD_AXE:
    case dItem::KIND_GOLD_WATERING:
    case dItem::KIND_GOLD_NET:
    case dItem::KIND_GOLD_PACHINKO:
        gold = TRUE;
        break;
    case dItem::KIND_SILVER_FISHINGROD:
    case dItem::KIND_SILVER_SCOOP:
    case dItem::KIND_SILVER_AXE:
    case dItem::KIND_SILVER_WATERING:
    case dItem::KIND_SILVER_NET:
    case dItem::KIND_SILVER_PACHINKO:
        gold = FALSE;
        break;
    default:
        return;
    }
    u32 type = dHmnToolBank_c::getItemToolType(&held);
    if (type >= l_glitterCount) {
        type = 0;
    }
    mMtx_c mtx;
    getRHandMtx(&mtx);
    mMtx_c trans;
    f32 z = l_glitterOfsZ[type];
    f32 y = l_glitterOfsY[type];
    f32 x = l_glitterOfsX[type];
    PSMTXTrans(trans, x, y, z);
    PSMTXConcat(mtx, trans, mtx);
    dEffect::create(gold ? "af_itm_gold" : "af_itm_silver", &mtx);
}

// .text 0x25BBC
void dAcPlayer_c::createToolGlitter(const dItem::Item *item) {
    BOOL gold;
    switch (item->getKind()) {
    case dItem::KIND_GOLD_FISHINGROD:
    case dItem::KIND_GOLD_SCOOP:
    case dItem::KIND_GOLD_AXE:
    case dItem::KIND_GOLD_WATERING:
    case dItem::KIND_GOLD_NET:
    case dItem::KIND_GOLD_PACHINKO:
        gold = TRUE;
        break;
    case dItem::KIND_SILVER_FISHINGROD:
    case dItem::KIND_SILVER_SCOOP:
    case dItem::KIND_SILVER_AXE:
    case dItem::KIND_SILVER_WATERING:
    case dItem::KIND_SILVER_NET:
    case dItem::KIND_SILVER_PACHINKO:
        gold = FALSE;
        break;
    default:
        return;
    }
    mMtx_c mtx = mMdlCallback.mHandMtx;
    mMtx_c trans;
    PSMTXTrans(trans, 10.0f, 3.0f, -8.0f);
    PSMTXConcat(mtx, trans, mtx);
    dEffect::create(gold ? "af_itm_gold" : "af_itm_silver", &mtx);
}

// .text 0x25CE0
void dAcPlayer_c::calcRunFootEffects() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        if (mAnm.checkFrame(16.0f)) {
            createRunEffect(&mMdlCallback.mRFootPos, 0);
        } else if (mAnm.checkFrame(0.0f)) {
            createRunEffect(&mMdlCallback.mLFootPos, 1);
        }
    }
}

// .text 0x25D6C
void dAcPlayer_c::createRunEffect(mVec3_c *foot, int side) {
    static const char *l_runEffects[] = {
        "afi_ply_run_smoke", "af_ply_run_sea", "af_ply_run_rain", "afi_ply_run_snow", "afi_ply_run_sand",
    };
    mVec3_c pos(foot->x, mPos.y, foot->z);
    createFootprint(foot, side);
    if (mAnmId == HMN_ANM_NML_DASH1) {
        u32 kind = dPlayerMgr::getSurfaceEffectKind(&pos);
        if (kind < 5) {
            dEffect::create(l_runEffects[kind], &pos, &mAngle, NULL);
        }
    }
}

// .text 0x25DEC
void dAcPlayer_c::createFootprint(const mVec3_c *pos, int side) {
    switch (side) {
    case 0:
        dEffect::createFootprint(pos, &mAngle, 1, 0);
        break;
    case 1:
        dEffect::createFootprint(pos, &mAngle, 1, 1);
        break;
    case 2:
        dEffect::createFootprint(&mMdlCallback.mRFootPos, &mAngle, 1, 0);
        dEffect::createFootprint(&mMdlCallback.mLFootPos, &mAngle, 1, 1);
        break;
    }
}

// .text 0x25E90
void dAcPlayer_c::wateringParticleCallback(nw4r::ef::Particle *particle, u32 param) {
    static const f32 l_waterDistSq = 225.0f;

    if (particle != NULL) {
        u32 kind = param & 0xF;
        mVec3_c pos;
        if (dScene::isOutdoorScene(dScene::getCurrentScene())) {
            mVec3_c ptclPos;
            ptclPos = particle->mParameter.mPosition;
            dWorld::toFieldPosition(&pos, &ptclPos);
        } else {
            pos = particle->mParameter.mPosition;
        }
        if (kind == dItem::KIND_CARPET) {
            int playerNo = param >> 30;
            for (int i = 0; i < 4; i++) {
                if (i == playerNo && dPlayerMgr::isState(dPlayerMgr::STATE_BLOW_BUBBLES, playerNo)) {
                    continue;
                }
                mVec3_c *soundPos = dPlayerMgr::getSoundPosition(i);
                if (soundPos != NULL && nw4r::math::FAbs(20.0f + soundPos->y - pos.y) < 20.0f) {
                    f32 distSq = (pos - *soundPos).xzLenSq();
                    if (distSq < l_waterDistSq && particle->mParticleManager != NULL) {
                        particle->mParticleManager->RetireParticle(particle);
                    }
                }
            }
        } else {
            dBGCF::groundChk_c chk(&pos, 0, 0, 0);
            if (chk.mWater != BG_WATER_NONE) {
                if (chk.isUnderWater(pos.y)) {
                    pos.y = chk.mWaterY;
                    dEffect::create(kind == dItem::KIND_MONEY ? "af_hny_hamon_S01" : "af_itm_jyoro_col_b00", &pos, NULL,
                                    NULL);
                    if (particle->mParticleManager != NULL) {
                        particle->mParticleManager->RetireParticle(particle);
                    }
                }
            } else if (kind == dItem::KIND_WALL && pos.y <= chk.getHeight(FALSE)) {
                f32 height = chk.getHeight(FALSE);
                if (nw4r::math::FAbs(height - pos.y) <= 5.0f) {
                    pos.y = height;
                    mVec3_c normal = chk.mFloor.mNormal;
                    mAng3_c angle(cM::atan2s(normal.z, normal.y), 0, cM::atan2s(normal.x, normal.y));
                    dEffect::create("af_itm_jyoro_col_a00", &pos, &angle, NULL);
                    if (particle->mParticleManager != NULL) {
                        particle->mParticleManager->RetireParticle(particle);
                    }
                }
            }
        }
    }
}

// .text 0x26134
void dAcPlayer_c::updateSlipEffect() {
    switch (_22D8[0]) {
    case 0:
        break;
    case 1:
        startSlipEffect();
        break;
    case 2:
        if (!mLevelEffect0.followEffect(&mMdlCallback.mRFootPos, reinterpret_cast<mAng3_c *>(&_2056), NULL)) {
            _22D8[0] = 0;
            mLevelEffect0.setParticleCallback(NULL, 0);
        }
        break;
    }
}

// .text 0x261B8
void dAcPlayer_c::createGroundEffect(const char *sandEffect, const char *effect) {
    dBGCF::groundChk_c chk(&mPos, 0, 0, 0);
    BOOL sand = FALSE;
    if (chk.mAttr == BG_ATTR_SAND || chk.mWater == BG_WATER_SEA) {
        sand = TRUE;
    }
    dEffect::create(sand ? sandEffect : effect, &mPos, &mAngle, NULL);
}

// .text 0x26254
void dAcPlayer_c::createFootEffect(const char *effect, const mVec3_c *pos) {
    dBGCF::groundChk_c chk(pos, 0, 0, 0);
    if (chk.mAttr == BG_ATTR_SAND || chk.mWater == BG_WATER_SEA ||
        (u8)dGroundAttr::getSnowWear(pos) > dPlayerMgr::sSnowEffectWearThreshold) {
        dEffect::create(effect, pos, &mAngle, NULL);
    }
}

// .text 0x262F4
void dAcPlayer_c::createSurfaceEffect(const char *const *names, const mVec3_c *pos, const mAng3_c *angle) {
    int kind = dPlayerMgr::getSurfaceEffectKind(pos);
    if (kind == dItem::KIND_WALL || kind == dItem::KIND_CARPET || kind == dItem::KIND_FTR) {
        dEffect::create(names[kind], pos, angle, NULL, dPlayerMgr::onParticleCollision, kind);
    } else {
        dEffect::create(names[kind], pos, angle, NULL, NULL, 0);
    }
}

// .text 0x26390
void dAcPlayer_c::startSlipEffect() {
    static const char *l_slipEffects[] = {
        "afi_ply_slip_smoke", "af_ply_slip_water", "af_ply_slip_water", "af_ply_slip_snow", "afi_ply_slip_sand",
    };
    _2056 = 0;
    _2058 = _22C2;
    _205A = 0;
    int kind = dPlayerMgr::getSurfaceEffectKind(&mMdlCallback.mRFootPos);
    if (kind == dItem::KIND_WALL || kind == dItem::KIND_CARPET || kind == dItem::KIND_FTR) {
        createSurfaceEffect(l_slipEffects, &mMdlCallback.mRFootPos, reinterpret_cast<mAng3_c *>(&_2056));
        _22D8[0] = 0;
    } else {
        mLevelEffect0.create(l_slipEffects[kind], &mMdlCallback.mRFootPos, reinterpret_cast<mAng3_c *>(&_2056), NULL);
        mLevelEffect0.setParticleCallback(NULL, 0);
        _22D8[0] = 2;
    }
}

// .text 0x26444
BOOL dAcPlayer_c::isInHouseRoom() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        switch (dScene::getCurrentScene()) {
        case dScene::SCENE_RM_HS0_F1:
        case dScene::SCENE_RM_HS0_F2:
        case dScene::SCENE_RM_HS0_BM:
        case dScene::SCENE_RM_HS1_F1:
        case dScene::SCENE_RM_HS1_F2:
        case dScene::SCENE_RM_HS1_BM:
        case dScene::SCENE_RM_HS2_F1:
        case dScene::SCENE_RM_HS2_F2:
        case dScene::SCENE_RM_HS2_BM:
        case dScene::SCENE_RM_HS3_F1:
        case dScene::SCENE_RM_HS3_F2:
        case dScene::SCENE_RM_HS3_BM:
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x264A4
void dAcPlayer_c::setFaceAnm(int anmId, int mode, BOOL force) {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL)) {
        return;
    }
    BOOL changed = FALSE;
    int eyeTex = dHmnFaceAnmMng_c::getEyeTexId(anmId);
    if (eyeTex < 0x1D3 && (_2224 != eyeTex || force)) {
        nw4r::g3d::ResAnmTexPat old = mFaceAnmMng.getEyeResAnmTexPat();
        old.Release();
        if (!mFaceAnmMng.setEyeTexByAnm(anmId)) {
            return;
        }
        nw4r::g3d::ResAnmTexPat anm = mFaceAnmMng.getEyeResAnmTexPat();
        nw4r::g3d::ResFile palette = mPaletteMng.getResFile(0);
        nw4r::g3d::ResFile tex = dHmnFaceTexMng_c::getResFile(getFaceNo());
        anm.Bind(palette);
        anm.Bind(tex);
        int eyeMode = mode;
        if (eyeTex == 0) {
            eyeMode = m3d::FORWARD_ONCE;
        }
        mTexPatAnm.setAnm(mHeadMdl, anm, 0, (m3d::playMode_e)eyeMode);
        _2224 = eyeTex;
        changed = TRUE;
        if (eyeTex == 0) {
            mTexPatAnm.setFrame(15.0f, 0);
        }
    }
    int mouthTex = dHmnFaceAnmMng_c::getMouthTexId(anmId);
    if (mouthTex < 0x1D3 && (_2228 != mouthTex || force)) {
        nw4r::g3d::ResAnmTexPat old = mFaceAnmMng.getMouthResAnmTexPat();
        old.Release();
        if (!mFaceAnmMng.setMouthTexByAnm(anmId)) {
            return;
        }
        nw4r::g3d::ResAnmTexPat anm = mFaceAnmMng.getMouthResAnmTexPat();
        nw4r::g3d::ResFile tex = dHmnFaceTexMng_c::getResFile(getGenderBodyNo());
        anm.Bind(mPaletteMng.getResFile(0));
        anm.Bind(tex);
        mTexPatAnm.setAnm(mHeadMdl, anm, 1, (m3d::playMode_e)mode);
        _2228 = mouthTex;
        changed = TRUE;
    }
    if (changed) {
        mHeadMdl.setAnm(mTexPatAnm);
    }
}

// .text 0x266BC
void dAcPlayer_c::updateFaceAnm() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
        if (dHmnFaceAnmMng_c::getEyeTexId(mAnmId) == 0) {
            if (mTexPatAnm.isStop(0) && mFaceAnmMng.calcBlink()) {
                mTexPatAnm.setFrame(0.0f, 0);
            }
            mTexPatAnm.play();
        } else {
            mTexPatAnm.play();
        }
    }
}

// .text 0x26764
void dAcPlayer_c::snapToGround() {
    mPos.y -= 5.0f;
    mAcch.check(15.0f, &mPos, &mLastPos, mAngle.y, (int)this, 0x17, 1);
}

// .text 0x267C4
void dAcPlayer_c::setNetObjc(f32 radius, f32 height, f32 dist, f32 y) {
    mVec3_c pos;
    getPosAtAngle(&pos, getActionAngle(), dist);
    pos.y = y;
    mObjcPos.set(this, pos, radius, height, 4, 0x8000003B, 0, mPlayerNo);
}

// .text 0x26858
void dAcPlayer_c::clearFieldRequest() {
    if (_229C != -1) {
        dFgobjManager::clearRequest(_229C);
        _229C = -1;
        mTaskResult = dFgobjManager::REQUEST_RESULT_WAIT;
    }
}

// .text 0x268A4
void dAcPlayer_c::setTargetUnit(const mVec3_c *pos) {
    mTargetUnitX = worldToUnit(pos->x);
    mTargetUnitZ = worldToUnit(pos->z);
}

// .text 0x268E0
void dAcPlayer_c::setTargetUnitHere(mVec3_c *pos) {
    *pos = mPos;
    setTargetUnit(pos);
}

// .text 0x268FC
void dAcPlayer_c::getFrontUnit(mVec3_c *pos) {
    calcPosAtAngle(pos, &mPos, mAngle.y, l_frontCheckDist);
    setTargetUnit(pos);
}

// .text 0x26950
void dAcPlayer_c::getNearFrontUnit(mVec3_c *pos) {
    calcPosAtAngle(pos, &mPos, mAngle.y, 20.8f);
    setTargetUnit(pos);
}

// .text 0x269A4
void dAcPlayer_c::getFrontUnit26(mVec3_c *pos) {
    getPosAtAngle(pos, mAngle.y, 26.0f);
    setTargetUnit(pos);
}

// .text 0x269F4
void dAcPlayer_c::getNetFrontUnit(mVec3_c *pos) {
    getPosAtAngle(pos, getActionAngle(), 32.0f);
    setTargetUnit(pos);
}

// .text 0x26A54
void dAcPlayer_c::getFreeFrontUnit(mVec3_c *pos) {
    if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_TURN) {
        getPosAtAngle(pos, _22D2[2], 32.0f);
    } else {
        getPosAtAngle(pos, mAngle.y, 32.0f);
        if (!isBlockedGround(pos)) {
            s16 step = l_angle10;
            mVec3_c side;
            getPosAtAngle(&side, step + mAngle.y.mAngle, 32.0f);
            if (isBlockedGround(&side)) {
                *pos = side;
            } else {
                getPosAtAngle(&side, mAngle.y.mAngle - step, 32.0f);
                if (isBlockedGround(&side)) {
                    *pos = side;
                }
            }
        }
    }
    setTargetUnit(pos);
}

// .text 0x26B84
BOOL dAcPlayer_c::setToolTargetUnit(mVec3_c *pos, int unitX, int unitZ) {
    BOOL result = TRUE;
    mVec3_c target;
    getPosAtAngle(&target, getActionAngle(), 32.0f);
    int x = worldToUnit(target.x);
    int z = worldToUnit(target.z);
    if (x == unitX && z == unitZ) {
        calcPosAtAngle(&target, &mPos, getActionAngle(), 48.0f);
        result = FALSE;
        x = worldToUnit(target.x);
        z = worldToUnit(target.z);
    }
    *pos = target;
    mTargetUnitX = x;
    mTargetUnitZ = z;
    return result;
}

// .text 0x26C94
BOOL dAcPlayer_c::isBlockedGround(mVec3_c *pos) {
    int attr;
    dBGCF::getGroundY(pos, &attr, 0x31);
    switch (attr) {
    case BG_ATTR_FLOOR:
        break;
    case BG_ATTR_WOOD: {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(pos, 0);
            if (item != NULL && item->isStump()) {
                return FALSE;
            }
        }
        return TRUE;
    }
    case BG_ATTR_STONE:
        return TRUE;
    }
    return FALSE;
}

// .text 0x26D54
BOOL dAcPlayer_c::canWalkToUnit(int unitX, int unitZ) {
    int x = worldToUnit(mPos.x);
    int z = worldToUnit(mPos.z);
    return dBGCF::checkWalkable(x, z, dBGCF::getUnitQuarter(&mPos), unitX, unitZ);
}

// .text 0x26DE0
BOOL dAcPlayer_c::waitFieldTask(int failState, bool wait, u32 mode) {
    if (_229C == -1) {
        setState(dPlayerMgr::STATE_IDLE);
        return TRUE;
    }
    if (mTaskResult == dFgobjManager::REQUEST_RESULT_WAIT) {
        mTaskResult = dFgobjManager::getRequestResult(_229C);
    }
    if (!wait && (mode != 1 || mTaskResult != dFgobjManager::REQUEST_RESULT_REJECTED)) {
        return FALSE;
    }
    switch (mTaskResult) {
    case dFgobjManager::REQUEST_RESULT_ACCEPTED:
        if (!applyFieldTask()) {
            setState(failState);
        }
        clearFieldRequest();
        break;
    case dFgobjManager::REQUEST_RESULT_REJECTED:
        setState(failState);
        clearFieldRequest();
        break;
    case dFgobjManager::REQUEST_RESULT_WAIT:
        return FALSE;
    default:
        setState(failState);
        break;
    }
    return TRUE;
}

// .text 0x26F04
BOOL dAcPlayer_c::applyFieldTask() {
    if (_229C == -1) {
        return FALSE;
    }
    dFgMngPlayerState_c *state = fgMngState_getPlayer(_229C);
    int x;
    int z;
    if (state != NULL) {
        dFdAsPos_c pos = state->getPos();
        x = pos.mX;
        z = pos.mZ;
        mPickUpKind = state->mLayer;
        _2260 = state->mItem;
        _2262 = state->mBaseItem;
    } else {
        x = 0;
        z = 0;
    }
    mTargetUnitX = x;
    mTargetUnitZ = z;
    mTaskKind = state->mKind;
    return changeStateByTaskKind();
}

// .text 0x26F9C
BOOL dAcPlayer_c::changeStateByTaskKind() {
    mpToolBank->getToolType();
    switch (mTaskKind) {
    case FG_MNG_KIND_SHAKE_TREE:
        setState(dPlayerMgr::STATE_SHAKE_TREE_END);
        return TRUE;
    case FG_MNG_KIND_SHAKE_TREE_SPECIAL:
        setState(dPlayerMgr::STATE_SHAKE_TREE_END);
        return TRUE;
    case FG_MNG_KIND_PICK_UP:
    case FG_MNG_KIND_PICK_UP_FULL:
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
            dItem::Item item = _2262;
            BOOL special = FALSE;
            int kind = item.getKind();
            if (kind == dItem::KIND_GOLD_SCOOP) {
                dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                if (pd != NULL && !pd->isFlag0(PRIVATE_FLAG0_DUG_GOLD_SCOOP)) {
                    pd->setFlag0(PRIVATE_FLAG0_DUG_GOLD_SCOOP);
                    special = TRUE;
                }
            } else if (kind == dItem::KIND_GOLD_PACHINKO) {
                dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                if (pd != NULL && !pd->isFlag0(PRIVATE_FLAG0_DUG_GOLD_PACHINKO)) {
                    pd->setFlag0(PRIVATE_FLAG0_DUG_GOLD_PACHINKO);
                    special = TRUE;
                }
            }
            if (special) {
                setState(dPlayerMgr::STATE_PICK_UP_ITEM_MSG);
            } else {
                setState(dPlayerMgr::STATE_PICK_UP_ITEM);
            }
        } else {
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM);
            setState(dPlayerMgr::STATE_PICK_UP_UNIT_ITEM);
        }
        return TRUE;
    case FG_MNG_KIND_PULL:
        setState(dPlayerMgr::STATE_PULL_WEED);
        return TRUE;
    case FG_MNG_KIND_CHOP_TREE:
        setState(dPlayerMgr::STATE_AXE_CHOP);
        return TRUE;
    case FG_MNG_KIND_CHOP_TREE_SPECIAL:
        setState(dPlayerMgr::STATE_AXE_CHOP);
        return TRUE;
    case FG_MNG_KIND_DIG_HOLE:
        setState(dPlayerMgr::STATE_SHOVEL_DIG);
        return TRUE;
    case FG_MNG_KIND_DIG_UP:
        setState(dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM);
        return TRUE;
    case FG_MNG_KIND_DIG_UP_FULL:
        setState(dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM);
        return TRUE;
    case FG_MNG_KIND_DIG_OUT:
        setState(dPlayerMgr::STATE_SHOVEL_DIG);
        return TRUE;
    case FG_MNG_KIND_BLOCKED:
        if (mState == dPlayerMgr::STATE_SHOVEL_WAIT) {
            setState(dPlayerMgr::STATE_SHOVEL_HIT);
            return TRUE;
        }
        if (mState == dPlayerMgr::STATE_AXE_STRIKE) {
            setState(dPlayerMgr::STATE_AXE_HIT);
            return TRUE;
        }
        return TRUE;
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_HIT_MONEY_ROCK:
        if (mState == dPlayerMgr::STATE_SHOVEL_WAIT) {
            setState(dPlayerMgr::STATE_SHOVEL_HIT);
            return TRUE;
        }
        if (mState == dPlayerMgr::STATE_AXE_STRIKE) {
            setState(dPlayerMgr::STATE_AXE_HIT);
            return TRUE;
        }
        break;
    case FG_MNG_KIND_BURY:
        setState(dPlayerMgr::STATE_SHOVEL_BURY);
        return TRUE;
    case FG_MNG_KIND_FILL_HOLE:
        setState(dPlayerMgr::STATE_STAMP_HOLE);
        return TRUE;
    case FG_MNG_KIND_PICK_UP_SPECIAL:
        if (mState == dPlayerMgr::STATE_SHOVEL_WAIT) {
            setState(dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM);
            return TRUE;
        }
        if (mState == dPlayerMgr::STATE_PICK_UP_REACH) {
            setState(dPlayerMgr::STATE_PICK_UP_ITEM_MSG);
            return TRUE;
        }
        break;
    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
        if (mState == dPlayerMgr::STATE_SHOVEL_WAIT) {
            setState(dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM);
            return TRUE;
        }
        if (mState == dPlayerMgr::STATE_PICK_UP_REACH) {
            setState(dPlayerMgr::STATE_PICK_UP_ITEM_MSG);
            return TRUE;
        }
        break;
    case FG_MNG_KIND_MISS:
        if (mState == dPlayerMgr::STATE_SHOVEL_WAIT) {
            setState(dPlayerMgr::STATE_SHOVEL_SWING_LOW);
            return TRUE;
        }
        if (mState == dPlayerMgr::STATE_AXE_STRIKE) {
            setState(dPlayerMgr::STATE_AXE_MISS);
            return TRUE;
        }
        break;
    case FG_MNG_KIND_PITFALL:
        setState(dPlayerMgr::STATE_PITFALL_FALL);
        return TRUE;
    default:
        return FALSE;
    }
    return FALSE;
}

// .text 0x27314
void dAcPlayer_c::setTargetUnitItem(int layer) {
    int x = mTargetUnitX;
    int z = mTargetUnitZ;
    dFdBase_c *field = dField::getInfo(0);
    if (field != NULL) {
        dItem::Item *item = field->getItem(x, z, layer);
        if (item != NULL) {
            _218C = item->mId;
            mActionItem = item->mId;
            if (item->mId == dItem::FG_LUCKY_CLOVER) {
                dItem::Item shown(dItem::ITEM_IDX_LUCKY_CLOVER);
                _2188 = shown;
                mActionItem = shown;
            } else {
                _2188 = item->getPickItem();
            }
        } else {
            _2188 = dItem::ITEM_ID_NONE;
            mActionItem = dItem::ITEM_ID_NONE;
            _218C = dItem::ITEM_ID_NONE;
        }
    }
}

// .text 0x273F0
BOOL dAcPlayer_c::checkPutEffectDone() {
    if (_22A0 == -1) {
        return FALSE;
    }
    switch (dFgobjManager::playPutEffect(_22A0)) {
    case 1:
    case 2:
        dFgobjManager::clearRequest(_22A0);
        _22A0 = -1;
        return TRUE;
    }
    return FALSE;
}

// .text 0x27464
void dAcPlayer_c::checkWalkIntoTree() {
    mVec3_c pos;
    getPosAtAngle(&pos, mAngle.y, 14.0f);
    int x = worldToUnit(pos.x);
    int z = worldToUnit(pos.z);
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_UNIT_ACTION;
    if (x != mActionUnitX || z != mActionUnitZ) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(x, z, 0);
            if (item != NULL && item->isTree()) {
                int myNo = dNet::getMyNo();
                if (mPlayerNo == myNo) {
                    dFgobjManager::startShakeTree(mPlayerNo, x, z, 1);
                }
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_UNIT_ACTION;
            }
        }
        mActionUnitX = x;
        mActionUnitZ = z;
    }
}

// .text 0x27568
BOOL dAcPlayer_c::startPitfallTrigger() {
    if (mPlayerNo != dNet::getMyNo()) {
        return FALSE;
    }
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    if (_229C != -1) {
        return FALSE;
    }
    mVec3_c pos;
    setTargetUnitHere(&pos);
    _229C = dFgobjManager::requestPitfall(mPlayerNo, mTargetUnitX, mTargetUnitZ);
    if (_229C != -1) {
        setState(dPlayerMgr::STATE_PITFALL_TRIGGER);
        return TRUE;
    }
    return FALSE;
}

// .text 0x27614
BOOL dAcPlayer_c::startShakeTree() {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    mVec3_c pos;
    getFrontUnit26(&pos);
    int attr;
    dBGCF::getGroundY(&pos, &attr, 0x31);
    if (attr == BG_ATTR_WOOD) {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(&pos, 0);
            if (item != NULL && item->isStump()) {
                return FALSE;
            }
        }
        int x = mTargetUnitX;
        int z = mTargetUnitZ;
        if (canWalkToUnit(x, z)) {
            _229C = dFgobjManager::useTool(mPlayerNo, x, z, dFgobjManager::USE_TOOL_SHAKE, 0, dItem::ITEM_ID_NONE);
            if (_229C != -1) {
                setState(dPlayerMgr::STATE_SHAKE_TREE);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// .text 0x27744
BOOL dAcPlayer_c::changeStateAfterMsg(int state) {
    dMsg::Comp_c *ctrl = getController();
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            setState(state);
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x277E8
BOOL dAcPlayer_c::setStepAfterMsg(int step) {
    dMsg::Comp_c *ctrl = getController();
    if (ctrl != NULL) {
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            mStep = step;
            return TRUE;
        }
    }
    return FALSE;
}

// .text 0x27884
void dAcPlayer_c::clearActorFlags101() {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DEMO;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
}

// .text 0x27898
BOOL dAcPlayer_c::putInPocket(dItem::Item *item) {
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (pd == NULL) {
        return FALSE;
    }
    int idx = pd->findEmptyPocket(0);
    if (idx == -1) {
        return FALSE;
    }
    if (item->mId == dItem::ITEM_ID_NONE) {
        return TRUE;
    }
    if (item->isFg74()) {
        return TRUE;
    }
    dItem::Item pick = item->getPickItem();
    pd->setPocket(&pick, idx, FALSE);
    return TRUE;
}

// .text 0x27958
void dAcPlayer_c::deleteHookedFish() {
    dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
    if (fish != NULL) {
        fish->requestDelete();
    }
}

// .text 0x2798C
int dAcPlayer_c::getStartState() {
    int state = mParam & 0xFF;
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        if (state != dPlayerMgr::STATE_RETURN_HOME && state != dPlayerMgr::STATE_SCENE_START &&
            state != dPlayerMgr::STATE_SCENE_START_HIDDEN && mPlayerNo == dNet::getMyNo() &&
            dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) &&
            dBGCF::isSlopeStep(&mPos)) {
            return dPlayerMgr::STATE_FAINT;
        }
        switch (state) {
        case dPlayerMgr::STATE_CHAIR_WAIT:
            setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
            state = dPlayerMgr::STATE_DEMO_WAIT;
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
            break;
        case dPlayerMgr::STATE_BED_WAIT:
            setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP);
            state = dPlayerMgr::STATE_DEMO_WAIT;
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
            break;
        }
    } else {
        if (state == dPlayerMgr::STATE_FROM_REMOTE) {
            state = dPlayerMngr_c::getRemoteState(mPlayerNo);
            if (!dPlayerMngr_c::getRemoteParam(mPlayerNo, &mRemoteActionParams)) {
                state = dPlayerMgr::STATE_IDLE;
            }
        }
        if (state >= dPlayerMgr::STATE_COUNT) {
            state = dPlayerMgr::STATE_IDLE;
        }
    }
    return state;
}

// .text 0x27AC0
void dAcPlayer_c::setStartState() {
    int state = getStartState();
    if (mPlayerNo == dNet::getMyNo()) {
        if (state == dPlayerMgr::STATE_DEMO_WAIT && (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) &&
            !dPlayerMgr::isToolActionHeld()) {
            showTool();
        }
        setState(state);
    } else if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE) || !dNet::isSessionActive()) {
        setState(state);
    } else if (dPlayerMngr_c::isRemotePresent(mPlayerNo)) {
        getRemoteActionParams(&mRemoteActionParams);
        setState(state);
    } else if (!dPlayerMgr::isActionParamLocked(state)) {
        getRemoteActionParams(&mRemoteActionParams);
        setState(state);
        int value;
        u8 scene;
        u8 remoteState;
        if (dPlayerMgr::getRemoteStateInfo(&value, &scene, &remoteState, mPlayerNo)) {
            mSyncValue = value;
            mRemoteSyncA = dPlayerMgr::STATE_NONE;
        }
    } else {
        int value;
        u8 scene;
        u8 remoteState;
        if (dPlayerMgr::getRemoteStateInfo(&value, &scene, &remoteState, mPlayerNo)) {
            remoteState = remoteState - 1;
        } else {
            remoteState = dPlayerMgr::STATE_NONE;
        }
        dPlayerMgr::setRemoteActionSync(mPlayerNo, remoteState, value);
        setState(state);
    }
}

// .text 0x27C38
void dAcPlayer_c::setStartAnm() {
    if (dPlayerMgr::isActionParamLocked(mState)) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        return;
    }
    if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld.mId != dItem::ITEM_ID_NONE &&
        dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        switch (mState) {
        case dPlayerMgr::STATE_DEMO_WAIT:
            if (((demoWaitParam_s *)&mRemoteActionParams)->mToolOut) {
                showTool();
            }
            break;
        case dPlayerMgr::STATE_EXIT_DOOR:
        case dPlayerMgr::STATE_RETURN_HOME:
        case dPlayerMgr::STATE_TAKE_OUT_TOOL:
        case dPlayerMgr::STATE_RECEIVE_ITEM:
        case dPlayerMgr::STATE_PASS_ITEM:
        case dPlayerMgr::STATE_PASS_ITEM_WAIT:
        case dPlayerMgr::STATE_HAND_ITEM_REACH:
        case dPlayerMgr::STATE_HAND_ITEM_TAKE:
        case dPlayerMgr::STATE_HAND_ITEM_HOLD:
        case dPlayerMgr::STATE_HAND_ITEM_PUT_AWAY:
            break;
        default:
            showTool();
            break;
        }
    }
    switch (mState) {
    case dPlayerMgr::STATE_SHOW_CATCH:
        if (mStep == 5) {
            mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        }
        break;
    case dPlayerMgr::STATE_NET_CATCH:
    case dPlayerMgr::STATE_FISHING_GET:
    case dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM:
    case dPlayerMgr::STATE_PICK_UP_ITEM_MSG:
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        break;
    case dPlayerMgr::STATE_REACT:
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        if (_2224 == 0x55) {
            mTexPatAnm.setFrame(mTexPatAnm.getFrameMax(0) - 1.0f, 0);
        }
        if (_2228 == 0x56) {
            mTexPatAnm.setFrame(mTexPatAnm.getFrameMax(0) - 1.0f, 1);
        }
        break;
    case dPlayerMgr::STATE_CATCH_POSE:
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
        if (_2224 == 0x23 || _2224 == 0x13) {
            mTexPatAnm.setFrame(mTexPatAnm.getFrameMax(0) - 1.0f, 0);
        }
        if (_2228 == 0x24 || _2228 == 0x14) {
            mTexPatAnm.setFrame(mTexPatAnm.getFrameMax(0) - 1.0f, 1);
        }
        break;
    case dPlayerMgr::STATE_PUT_AWAY_TOOL_FULL:
        if (mStep != 0x1F) {
            mAnm.setFrame(34.0f);
        }
        break;
    }
}

// .text 0x27EE8
u16 dAcPlayer_c::getFieldRealItem(const mVec3_c *pos, int layer, dFdBase_c *field) {
    if (field == NULL) {
        return dItem::ITEM_ID_NONE;
    }
    dItem::Item *item = field->getItem(pos, layer);
    if (item != NULL && dItem::isRealItemId(item->mId)) {
        return item->mId;
    }
    return dItem::ITEM_ID_NONE;
}

// .text 0x27F5C
u16 dAcPlayer_c::getFieldPickableItem(const mVec3_c *pos, int layer, dFdBase_c *field) {
    u16 id = getFieldRealItem(pos, layer, field);
    if (id != dItem::ITEM_ID_NONE) {
        dItem::Item item = id;
        int kind = item.getKind();
        if (kind == dItem::KIND_MUSHROOM || kind == dItem::KIND_MUSH_FTR) {
            return dItem::ITEM_ID_NONE;
        }
        if (!field->isBuried(pos)) {
            return id;
        }
    }
    return dItem::ITEM_ID_NONE;
}

// .text 0x27FF8
void dAcPlayer_c::addModelOffset(mVec3_c *pos, f32 x, f32 y, f32 z) {
    mMtx_c mtx;
    getRootMtx(&mtx);
    mMtx_c trans;
    trans.trans(x, y, z);
    mtx.concat(trans);
    mVec3_c offset(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
    dWorld::toFieldPosition(&offset, &offset);
    f32 dx = offset.x - mPos.x;
    f32 dy = offset.y - mPos.y;
    f32 dz = offset.z - mPos.z;
    pos->x += dx;
    pos->y += dy;
    pos->z += dz;
}

// .text 0x28100
void dAcPlayer_c::setIdleAnm() {
    if (mState == dPlayerMgr::STATE_WALK) {
        setAnm(HMN_ANM_NML_WAIT1, l_moveParam.mWaitMorph, mAnm.getFrame(), 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
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
}

// .text 0x281E8
void dAcPlayer_c::setNetWalkAnm() {
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1B9(m3d::FORWARD_LOOP, 90.0f);
        if (mAnmId == HMN_ANM_NML_WALK1) {
            mpToolBank->setAnmFrame(mAnm.getFrame());
            mpToolBank->setAnmRate(mAnm.getRate());
        }
    }
}

// .text 0x2826C
void dAcPlayer_c::showTool() {
    mToolScale.set(1.0f, 1.0f, 1.0f);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
    setToolAnm(mAnmId, TRUE, 16.0f);
}

// .text 0x282A0
void dAcPlayer_c::hideTool() {
    mToolScale.set(0.0f, 0.0f, 0.0f);
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
    mMdl.removePartAnm(1, 16.0f);
}

// .text 0x282D4
void dAcPlayer_c::updateSunTimer() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    if (_22B8[1] != 0) {
        _22B8[1]--;
        return;
    }
    _22B8[1] = 300;
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        return;
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED) {
        return;
    }
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (pd == NULL) {
        return;
    }
    if (pd->isFlag0(PRIVATE_FLAG0_SUNTAN)) {
        return;
    }
    if (mpToolBank->getToolType() == HMN_TOOL_UMBRELLA) {
        return;
    }
    dWeather_c *weather = dWeather_c::sInstance;
    if (weather == NULL) {
        return;
    }
    int kind = weather->mWeather;
    if (dWeather_c::isRainKind(kind) || dWeather_c::isSnowKind(kind)) {
        return;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour < 10 || now.hour >= 17) {
        return;
    }
    if (!(now.month == 7 || (now.month == 6 && now.mday >= 16) || (now.month == 8 && now.mday <= 15))) {
        return;
    }
    u32 *timer = dPlayerMgr::getTimer();
    if ((int)(*timer -= 300) <= 0) {
        pd->setFlag0(PRIVATE_FLAG0_SUNTAN);
        dPlayerMgr::resetTimer();
    }
}

// .text 0x28498
void dAcPlayer_c::setFgObjItem(u16 item, const mVec3_c *pos, const mVec3_c *scale) {
    mFgObjItem = item;
    _20C8 = *pos;
    _20D4 = *scale;
}

// .text 0x284D0
void dAcPlayer_c::startRumble(int kind) {
    if (mPlayerNo == dNet::getMyNo()) {
        dRumble::start(kind, FALSE);
    }
}

// .text 0x28520
void dAcPlayer_c::setDigAnm(int anm) {
    static const int sNeighbors[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int unitX = mTargetUnitX;
    int unitZ = mTargetUnitZ;
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, unitX, unitZ);
    pos.y = dBGCF::getGroundY(&pos, FALSE);
    dBGCF::groundChk_c chk(&pos, 0, 0, 0);
    mVec3_c normal = chk.mFloor.mNormal;
    mAng3_c angle(cM::atan2s(normal.z, normal.y), mAngle.y.mAngle, (s16)-cM::atan2s(normal.x, normal.y));
    BOOL sand = TRUE;
    if (chk.mAttr != BG_ATTR_SAND && chk.mWater != BG_WATER_SEA) {
        sand = FALSE;
    }
    dFdBase_c *field;
    const int (*neighbor)[2];
    u8 wear;
    wear = 0xFF;
    field = dField::getInfo(0);
    if (field != NULL) {
        u32 total = field->getGrassWear(&pos, TRUE) * 400;
        neighbor = sNeighbors;
        for (int i = 0; i < 4; i++, neighbor++) {
            mVec3_c side;
            dFdBase_c::getUnitCenterPos(&side, unitX + (*neighbor)[0], unitZ + (*neighbor)[1]);
            dBGCF::groundChk_c sideChk(&side, 0, 0, 0);
            if (sideChk.mAttr == BG_ATTR_SOIL) {
                total += field->getGrassWear(&side, TRUE) * 100;
            }
        }
        wear = total / 800;
    }
    mpToolBank->setDigAnm(&pos, &angle, anm, !sand, wear);
}

// .text 0x286BC
void dAcPlayer_c::onMessageStart(int arg) {
    if (mMsgStep == MSG_STEP_COMPLETE_QUEUED) {
        mMsgStep = MSG_STEP_COMPLETE;
    }
}

// .text 0x286D4
void dAcPlayer_c::onMessageEnd(int kind) {
    switch (mMsgStep) {
    case MSG_STEP_QUEUE_ASK_DROP:
        getController()->setNextMsgCode(MSG_POCKETS_FULL);
        mMsgStep = MSG_STEP_ASK_DROP_QUEUED;
        break;
    case MSG_STEP_QUEUE_ASK_BURY:
        getController()->setNextMsgCode(MSG_DUG_UP_POCKETS_FULL);
        mMsgStep = MSG_STEP_ASK_BURY;
        break;
    case MSG_STEP_QUEUE_LET_GO:
        getController()->setNextMsgCode(MSG_CATCH_LET_GO);
        mMsgStep = MSG_STEP_RELEASE;
        break;
    case MSG_STEP_QUEUE_ASK_RELEASE:
        getController()->setNextMsgCode(MSG_CATCH_SWAP_OR_RELEASE);
        mMsgStep = MSG_STEP_ASK_RELEASE;
        break;
    case MSG_STEP_QUEUE_ASK_RELEASE_FISH:
        getController()->setNextMsgCode(MSG_CATCH_SWAP_OR_RELEASE_FISH);
        mMsgStep = MSG_STEP_ASK_RELEASE_FISH;
        break;
    case MSG_STEP_QUEUE_COMPLETE:
        getController()->setNextMsgCode(MSG_CATCH_SO_THAT_MEANS);
        mMsgStep = MSG_STEP_QUEUE_COMPLETE_2;
        break;
    case MSG_STEP_QUEUE_COMPLETE_2:
        getController()->setNextMsgCode(MSG_CATCH_COMPLETE);
        mMsgStep = MSG_STEP_COMPLETE_QUEUED;
        break;
    }
}

// .text 0x287C8
void dAcPlayer_c::onAnswer(int arg) {
    switch (mMsgStep) {
    case MSG_STEP_ASK_DROP:
    case MSG_STEP_ASK_DROP_QUEUED:
    case MSG_STEP_ASK_BURY:
    case MSG_STEP_ASK_RELEASE:
    case MSG_STEP_ASK_RELEASE_FISH: {
        dMsg::Comp_c *ctrl = getController();
        int answer = (ctrl != NULL && ctrl->getSelect() != NULL) ? ctrl->getSelect()->mCursor : -1;
        if (answer == 0) {
            switch (mMsgStep) {
            case MSG_STEP_ASK_BURY:
                mMsgStep = MSG_STEP_SWAP_DUG;
                break;
            case MSG_STEP_ASK_RELEASE:
            case MSG_STEP_ASK_RELEASE_FISH:
                if (_2188.isInsect()) {
                    mMsgStep = MSG_STEP_SWAP_INSECT;
                } else {
                    mMsgStep = MSG_STEP_SWAP_FISH;
                }
                break;
            default:
                mMsgStep = MSG_STEP_SWAP;
                break;
            }
        } else {
            switch (mMsgStep) {
            case MSG_STEP_ASK_BURY:
                mMsgStep = MSG_STEP_NONE;
                break;
            case MSG_STEP_ASK_RELEASE:
                mMsgStep = MSG_STEP_RELEASE;
                break;
            case MSG_STEP_ASK_RELEASE_FISH:
                mMsgStep = MSG_STEP_RELEASE_FISH;
                break;
            default:
                mMsgStep = MSG_STEP_NONE;
                break;
            }
        }
        break;
    }
    }
}

// .text 0x28900
void dAcPlayer_c::createBodyModel() {
    mBodyMng.setBufIdx(mPlayerNo);
    mBodyMng.setBody(dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mPID.player.getGender() != 0);
    nw4r::g3d::ResFile file(mBodyMng.getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 2, 0, 1, NULL);
    rebindShoePalette();
    setupBodyPalette();
    mAnm.create(mdl, dHmnAnm_c::getResAnmChr(0), &mAllocator, NULL);
    mAnm.mPlayMode = m3d::FORWARD_LOOP;
    mMdl.setAnm(mAnm);
    mPartAnm.create(mdl, dHmnAnm_c::getResAnmChr(0x14), &mAllocator, NULL);
    mMdlCallback.mBaseNode = mdl.GetResNode("base").GetID();
    mMdlCallback.mRootNode = mdl.GetResNode("root").GetID();
    mMdlCallback.mHandNode = mdl.GetResNode("Lhand").GetID();
    mMdlCallback.mRHandNode = mdl.GetResNode("Rhand").GetID();
    mMdlCallback.mRFootNode = mdl.GetResNode("Rfoot3").GetID();
    mMdlCallback.mLFootNode = mdl.GetResNode("Lfoot3").GetID();
    mMdlCallback.mFeelNode = mdl.GetResNode("feel").GetID();
    mMdlCallback.mHeadNode = mdl.GetResNode("head").GetID();
    mMdl.setCallback(&mMdlCallback);
    mFace.loadTevSetting();
}

// .text 0x28B70
void dAcPlayer_c::releaseClothTex() {
    mMdl.getResMdl().ReleasePlttByName("cloth");
    mMdl.getResMdl().ReleaseTexByName("cloth");
    bindCloth();
}

// .text 0x28BDC
void dAcPlayer_c::setupHeadModel(nw4r::g3d::ResFile file) {
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    createHeadMdl(file);
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mPaletteMng.setPltt(_221C, 0);
    mPaletteMng.setPltt(_2218, 1);
    nw4r::g3d::ResFile pltt = mPaletteMng.getResFile(0);
    mdl.Bind(pltt);
    int region = pd->mPID.land.mRegion;
    if (region != 0 && region != 9) {
        u16 *data = pltt.GetResPltt("skin").GetPlttData();
        const u8 *color = l_faceColors[pd->mFace];
        data[0] = ((color[0] & 0xF8) << 7) | ((color[1] & 0xF8) << 2) | ((color[2] & 0xF8) >> 3) | 0x8000;
        data[1] = ((color[3] & 0xF8) << 7) | ((color[4] & 0xF8) << 2) | ((color[5] & 0xF8) >> 3) | 0x8000;
        DCStoreRangeNoSync(data, 0x20);
    }
    mdl.Bind(mPaletteMng.getResFile(1));
    nw4r::g3d::ResFile eyeFile = dHmnFaceTexMng_c::getResFile(getFaceNo());
    mdl.Bind(eyeFile);
    nw4r::g3d::ResFile mouthFile = dHmnFaceTexMng_c::getResFile(getGenderBodyNo());
    mdl.Bind(mouthFile);
    nw4r::g3d::ResMdl headMdl = mHeadMdl.getResMdl();
    nw4r::g3d::ResMatTevColor tev = headMdl.GetResMat("h").GetResMatTevColor();
    GXColor hairColor;
    if (tev.GXGetTevColor(GX_TEVPREV, &hairColor)) {
        u8 colorNo = pd->getHairColor();
        hairColor.a = l_hairColorAlpha[(u8)colorNo];
        tev.GXSetTevColor(GX_TEVPREV, hairColor);
        tev.DCStore(false);
    }
    mFaceAnmMng.setEyeTex(0);
    nw4r::g3d::ResAnmTexPat anm = mFaceAnmMng.getEyeResAnmTexPat();
    anm.Bind(pltt);
    anm.Bind(eyeFile);
    mTexPatAnm.create(mHeadMdl.getResMdl(), anm, &mHeadAllocator, NULL, 2);
    mTexPatAnm.setAnm(mHeadMdl, anm, 0, m3d::FORWARD_ONCE);
    mTexPatAnm.setFrame(15.0f, 0);
    mFaceAnmMng.setMouthTex(1);
    anm = mFaceAnmMng.getMouthResAnmTexPat();
    anm.Bind(pltt);
    anm.Bind(mouthFile);
    mTexPatAnm.setAnm(mHeadMdl, anm, 1, m3d::FORWARD_LOOP);
    mHeadMdl.setAnm(mTexPatAnm);
    int myNo = dNet::getMyNo();
    if (mPlayerNo != myNo && dPlayerMngr_c::isEquipChecked(mPlayerNo)) {
        int mode = dHmnAnm_c::getPlayMode(mAnmId);
        if (mode != 4) {
            setFaceAnm(mAnmId, mode, TRUE);
        }
    }
}

// .text 0x28F74
void dAcPlayer_c::releaseHeadModel() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
        nw4r::g3d::ResAnmTexPat anm = mFaceAnmMng.getEyeResAnmTexPat();
        anm.Release();
        anm = mFaceAnmMng.getMouthResAnmTexPat();
        anm.Release();
        mHeadMdl.getResMdl().ReleasePlttByName("skin");
        mHeadMdl.getResMdl().ReleasePlttByName("h");
        mHeadMdl.getResMdl().ReleaseTexByName("e.0");
        mHeadMdl.getResMdl().ReleaseTexByName("m.0");
        mTexPatAnm.remove();
        mHeadMdl.remove();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HEAD_MDL;
    }
}

// .text 0x29074
void dAcPlayer_c::setBodyKColor(const GXColor &color) {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("b0");
    if (mat.IsValid()) {
        nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
        tev.GXSetTevKColor(GX_KCOLOR1, color);
        tev.DCStore(false);
    }
}

// .text 0x2911C
void dAcPlayer_c::createMiiHeap() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
        if (mpMiiHeap == NULL) {
            mpMiiHeap = mHeap::createFrmHeap(dPlayerMgr::getMiiHeapSize(), dHeap::playerHeap_p,
                                             "dAcPlayer_c::m_heap_Mii_p", 0x20, mHeap::OPT_NONE);
        }
        mMiiAllocator.attach(mpMiiHeap, 0x20);
        createMiiFace();
    }
}

// .text 0x29198
void dAcPlayer_c::createModels() {
    mHeadMng.setBufIdx(mPlayerNo);
    mPaletteMng.setType(mPlayerNo);
    mFaceAnmMng.setType(mPlayerNo);
    createBodyModel();
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) && pd->mMiiMask) {
        if (!putOnMiiMask()) {
            loadHead();
        }
    } else if (mEquipStep != EQUIP_STEP_APPLIED) {
        loadHead();
    } else if (pd->mMiiMask) {
        loadHead();
    }
    applyEquipment();
    applyTool();
}

// .text 0x2926C
void dAcPlayer_c::setupBodyPalette() {
    u32 no = _221C;
    if (no >= 8) {
        no = 0;
    }
    GXColor color = {l_skinColors[no][0], l_skinColors[no][1], l_skinColors[no][2]};
    setBodyKColor(color);
}

// .text 0x292D4
void dAcPlayer_c::bindCloth() {
    void *file = mpCloth->getResFile();
    if (file != NULL) {
        mMdl.getResMdl().Bind(nw4r::g3d::ResFile(file));
        mpCloth->lock();
    }
}

// .text 0x29330
void *dAcPlayer_c::loadHeadData(int kind) {
    if (mHeadMng.setHead(_2214, kind) == FALSE) {
        return NULL;
    }
    return mHeadMng.getData();
}

// .text 0x29380
void *dAcPlayer_c::loadMaskHeadData(int kind) {
    if (mHeadMng.setHead(0x1A, kind) == FALSE) {
        return NULL;
    }
    return mHeadMng.getData();
}

// .text 0x293D0
void dAcPlayer_c::createHeadMdl(nw4r::g3d::ResFile file) {
    mHeadMdl.create(file.GetResMdl(0), &mHeadAllocator, 0, 1, NULL);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_HEAD_MDL;
}

// .text 0x29430
void dAcPlayer_c::createHairMdl(nw4r::g3d::ResFile file) {
    mHairMdl.create(file.GetResMdl(0), &mHeadAllocator, 0, 1, NULL);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_HAIR_MDL;
}

// .text 0x29490
void dAcPlayer_c::releaseHairModel() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL) {
        mHairMdl.getResMdl().ReleasePlttByName("h");
        mHairMdl.remove();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HAIR_MDL;
    }
    releaseHeadModel();
}

// .text 0x29508
void dAcPlayer_c::releaseAccessoryModel() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
        mAccessoryMdl.remove();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL;
    }
}

// .text 0x29558
void dAcPlayer_c::freeHeadHeap() {
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) || (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL)) {
        releaseHairModel();
        if (mpHeadHeap != NULL) {
            mpHeadHeap->free(3);
        }
        mHeadAllocator.attach(mpHeadHeap, 0x20);
    }
}

// .text 0x295C0
void dAcPlayer_c::freeAccessoryHeap() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
        releaseAccessoryModel();
        if (mpAccessoryHeap != NULL) {
            mpAccessoryHeap->free(3);
        }
        mAccessoryAllocator.attach(mpAccessoryHeap, 0x20);
    }
}

// .text 0x2961C
void dAcPlayer_c::initEquipment() {
    int accBone;
    if (mEquipStep != EQUIP_STEP_NONE) {
        return;
    }
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (pd->mEquipment.mShirt.mId == dItem::ITEM_ID_NONE) {
        pd->mEquipment.mShirt = dItem::Item(dItem::ITEM_IDX_MOLDY_SHIRT);
        // The original also constructs a second, unused fallback item.
        (void)dItem::Item(dItem::ITEM_IDX_MOLDY_SHIRT);
    }
    mEquipFlags = 0;
    mEquipStep = EQUIP_STEP_LOAD_INIT;
    checkMiiFace();
    BOOL miiMask = FALSE;
    if (mPlayerNo == dNet::getMyNo() || !dNet::isSessionActive()) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (player->isFlag0(PRIVATE_FLAG0_TRAVELLING) && mPlayerNo == dNet::getMyNo()) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
                mMiiMask = 0;
                if (player->mMiiMask) {
                    miiMask = TRUE;
                }
            }
        } else if (player->mMiiMask) {
            u16 index = 0;
            if (RFLSearchOfficialData(&player->mMiiId, &index)) {
                miiMask = TRUE;
                mMiiMask = index;
            } else {
                player->mMiiMask = 0;
            }
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
        mMiiMask = 0;
        if (pd->mMiiMask) {
            miiMask = TRUE;
        }
    }
    if (miiMask) {
        if (pd->mEquipment.mHat.mId != dItem::ITEM_ID_NONE) {
            pd->mEquipment.mHat = dItem::ITEM_ID_NONE;
        }
        if (pd->mEquipment.mAcc.mId != dItem::ITEM_ID_NONE) {
            pd->mEquipment.mAcc = dItem::ITEM_ID_NONE;
        }
    } else {
        int bone = dItem::HIDE_BONE_NONE;
        pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(pd->mEquipment.mHat);
        if (bitm != NULL) {
            bone = bitm->getHideBone();
        }
        if (bone == dItem::HIDE_BONE_CAP || bone == dItem::HIDE_BONE_HAIR_ACCESSORY) {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_CAP;
        } else if (bone == dItem::HIDE_BONE_FULL_MASK) {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_FULL_MASK;
        } else if (bone == 3 || bone == 4) {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_MASK;
        } else if (bone == 5 || bone == 6) {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_MASK_2;
        } else if (bone == 7) {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_OVER_HEAD;
        } else {
            mEquipFlags |= dPlayerMgr::EQUIPMENT_HAT_NONE;
        }
        if (bone != 2 && bone != 4 && bone != 6 && bone != 7) {
            accBone = dItem::HIDE_BONE_NONE;
            const dItem::BITM *accBitm = dItem::infoBank_c::get()->getBITM(pd->mEquipment.mAcc);
            if (accBitm != NULL) {
                accBone = accBitm->getHideBone();
            }
            if (accBone == dItem::HIDE_BONE_ACCESSORY) {
                mEquipFlags |= dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY;
            }
        } else {
            if (pd->mEquipment.mAcc.mId != dItem::ITEM_ID_NONE) {
                pd->mEquipment.mAcc = dItem::ITEM_ID_NONE;
            }
        }
    }
    mEquipFlags |= dPlayerMgr::EQUIPMENT_CHANGE_CLOTH;
}

// .text 0x29934
void dAcPlayer_c::initHeldTool() {
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (pd == NULL) {
        return;
    }
    dItem::Item held = pd->mEquipment.mHeld;
    if (held.mId == dItem::ITEM_ID_NONE) {
        return;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        if (!dHmnToolBank_c::isHandTool(&held)) {
            return;
        }
    } else if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return;
    }
    if (mHeldEquipStep == EQUIP_STEP_NONE) {
        mHeldItemChange = 0;
        int bone = dItem::HIDE_BONE_NONE;
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(held);
        if (bitm != NULL) {
            bone = bitm->getHideBone();
        }
        if (bone == dItem::HIDE_BONE_HAND) {
            mHeldItemChange = 1;
            mHeldEquipStep = EQUIP_STEP_LOAD_INIT;
        }
    }
}

// .text 0x29A20
void dAcPlayer_c::loadCapHead() {
    freeHeadHeap();
    BOOL cap = FALSE;
    const dItem::BITM *bitm =
        dItem::infoBank_c::get()->getBITM(dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHat);
    if (bitm != NULL && bitm->getHideBone() == dItem::HIDE_BONE_CAP) {
        cap = TRUE;
    }
    void *head = loadHeadData(cap);
    if (head == NULL) {
        return;
    }
    setupHeadModel(nw4r::g3d::ResFile(head));
    nw4r::g3d::ResFile capFile(mpHeadItem->getCapResFile());
    if (capFile.ptr() == NULL || !capFile.HasResMdl()) {
        return;
    }
    createHairMdl(capFile);
    mpHeadItem->lockCap();
}

// .text 0x29B0C
void dAcPlayer_c::loadFullMask() {
    freeHeadHeap();
    nw4r::g3d::ResFile capFile(mpHeadItem->getCapResFile());
    if (capFile.ptr() == NULL || !capFile.HasResMdl()) {
        return;
    }
    createHairMdl(capFile);
    mpHeadItem->lockCap();
}

// .text 0x29B7C
void dAcPlayer_c::loadMaskHead() {
    freeHeadHeap();
    BOOL kind = FALSE;
    const dItem::BITM *bitm =
        dItem::infoBank_c::get()->getBITM(dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHat);
    if (bitm != NULL) {
        int bone = bitm->getHideBone();
        if (bone == 5 || bone == 6) {
            kind = TRUE;
        }
    }
    void *head = loadMaskHeadData(kind);
    if (head == NULL) {
        return;
    }
    setupHeadModel(nw4r::g3d::ResFile(head));
    nw4r::g3d::ResFile capFile(mpHeadItem->getCapResFile());
    if (capFile.ptr() == NULL || !capFile.HasResMdl()) {
        return;
    }
    createHairMdl(capFile);
    mpHeadItem->lockCap();
    nw4r::g3d::ResMdl mdl = mHairMdl.getResMdl();
    nw4r::g3d::ResMat mat = mdl.GetResMat("h");
    if (!mat.IsValid()) {
        return;
    }
    mHairMdl.getResMdl().ReleasePlttByName("h");
    mdl.Bind(mPaletteMng.getResFile(0));
    mdl.Bind(mPaletteMng.getResFile(1));
    nw4r::g3d::ResMatTevColor tev = mat.GetResMatTevColor();
    GXColor hairColor;
    if (tev.GXGetTevColor(GX_TEVPREV, &hairColor)) {
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        u8 colorNo = pd->getHairColor();
        hairColor.a = l_hairColorAlpha[(u8)colorNo];
        tev.GXSetTevColor(GX_TEVPREV, hairColor);
        tev.DCStore(false);
    }
}

// .text 0x29D78
void dAcPlayer_c::loadHeadAndCap() {
    freeHeadHeap();
    void *head = loadHeadData(FALSE);
    if (head == NULL) {
        return;
    }
    setupHeadModel(nw4r::g3d::ResFile(head));
    nw4r::g3d::ResFile capFile(mpHeadItem->getCapResFile());
    if (capFile.ptr() == NULL || !capFile.HasResMdl()) {
        return;
    }
    createHairMdl(capFile);
    mpHeadItem->lockCap();
}

// .text 0x29E0C
void dAcPlayer_c::loadHead() {
    freeHeadHeap();
    void *head = loadHeadData(FALSE);
    if (head != NULL) {
        setupHeadModel(nw4r::g3d::ResFile(head));
    }
}

// .text 0x29E5C
void dAcPlayer_c::loadAccessory() {
    freeAccessoryHeap();
    nw4r::g3d::ResFile file(mpHeadItem->getAccessoryResFile());
    if (file.ptr() == NULL || !file.HasResMdl()) {
        return;
    }
    mAccessoryMdl.create(file.GetResMdl(0), &mAccessoryAllocator, 0, 1, NULL);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL;
    mpHeadItem->lockAccessory();
}

// .text 0x29EF0
void dAcPlayer_c::removeAccessory() {
    freeAccessoryHeap();
}

// .text 0x29EF4
BOOL dAcPlayer_c::checkEquipLoaded(BOOL load) {
    if (mEquipStep != EQUIP_STEP_LOAD && mEquipStep != EQUIP_STEP_LOAD_INIT) {
        return TRUE;
    }
    BOOL head = TRUE;
    BOOL acc = TRUE;
    BOOL cloth = TRUE;
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_CAP) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK) ||
        (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_MASK) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_MASK_2) ||
        (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_OVER_HEAD) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_NONE)) {
        if (!dPlayerMngr_c::checkEquip(mPlayerNo, dEquip_c::PART_HAT)) {
            head = FALSE;
            if (load && dPlayerMngr_c::isEquipLoaded(mPlayerNo, dEquip_c::PART_HAT)) {
                head = TRUE;
            }
        }
    }
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY)) {
        if (!dPlayerMngr_c::checkEquip(mPlayerNo, dEquip_c::PART_ACCESSORY)) {
            acc = FALSE;
            if (load && dPlayerMngr_c::isEquipLoaded(mPlayerNo, dEquip_c::PART_ACCESSORY)) {
                acc = TRUE;
            }
        }
    }
    if (mEquipFlags & dPlayerMgr::EQUIPMENT_CHANGE_CLOTH) {
        if (!dPlayerMngr_c::checkEquip(mPlayerNo, dEquip_c::PART_SHIRT)) {
            cloth = FALSE;
            if (load && dPlayerMngr_c::isEquipLoaded(mPlayerNo, dEquip_c::PART_SHIRT)) {
                cloth = TRUE;
            }
        }
    }
    if (head && acc && cloth) {
        if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOADED;
        } else if (mEquipStep == EQUIP_STEP_LOAD_INIT) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else {
            mEquipStep = EQUIP_STEP_LOADED;
        }
        return TRUE;
    }
    return FALSE;
}

// .text 0x2A0A4
BOOL dAcPlayer_c::checkToolLoaded(BOOL load) {
    if (mHeldEquipStep != EQUIP_STEP_LOAD && mHeldEquipStep != EQUIP_STEP_LOAD_INIT) {
        return TRUE;
    }
    BOOL ready = TRUE;
    if (mHeldItemChange) {
        if (!dPlayerMngr_c::checkEquip(mPlayerNo, dEquip_c::PART_HELD)) {
            ready = FALSE;
            if (load && dPlayerMngr_c::isEquipLoaded(mPlayerNo, dEquip_c::PART_HELD)) {
                ready = TRUE;
            }
        }
    }
    if (ready) {
        if (mHeldEquipStep == EQUIP_STEP_LOAD) {
            mHeldEquipStep = EQUIP_STEP_LOADED;
        } else if (mHeldEquipStep == EQUIP_STEP_LOAD_INIT) {
            mHeldEquipStep = EQUIP_STEP_INIT_LOADED;
        } else {
            mHeldEquipStep = EQUIP_STEP_LOADED;
        }
    }
    return ready;
}

// .text 0x2A184
BOOL dAcPlayer_c::checkEquipReady() {
    if (mEquipStep != EQUIP_STEP_LOAD && mEquipStep != EQUIP_STEP_LOAD_INIT) {
        return TRUE;
    }
    BOOL ready = TRUE;
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_CAP) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK) ||
        (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_MASK) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_MASK_2) ||
        (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_OVER_HEAD) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_NONE)) {
        if (!dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT)) {
            ready = FALSE;
        }
    }
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY)) {
        if (!dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_ACCESSORY)) {
            ready = FALSE;
        }
    }
    if (mEquipFlags & dPlayerMgr::EQUIPMENT_CHANGE_CLOTH) {
        if (!dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_SHIRT)) {
            ready = FALSE;
        }
    }
    if (ready) {
        if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOADED;
        } else if (mEquipStep == EQUIP_STEP_LOAD_INIT) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else {
            mEquipStep = EQUIP_STEP_LOADED;
        }
    }
    return ready;
}

// .text 0x2A2B0
BOOL dAcPlayer_c::checkToolReady() {
    if (mHeldEquipStep != EQUIP_STEP_LOAD && mHeldEquipStep != EQUIP_STEP_LOAD_INIT) {
        return TRUE;
    }
    BOOL ready = TRUE;
    if (mHeldItemChange) {
        dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (!dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HELD)) {
            ready = FALSE;
        }
    }
    if (ready) {
        if (mHeldEquipStep == EQUIP_STEP_LOAD) {
            mHeldEquipStep = EQUIP_STEP_LOADED;
        } else if (mHeldEquipStep == EQUIP_STEP_LOAD_INIT) {
            mHeldEquipStep = EQUIP_STEP_INIT_LOADED;
        } else {
            mHeldEquipStep = EQUIP_STEP_LOADED;
        }
    }
    return ready;
}

// .text 0x2A36C
void dAcPlayer_c::applyEquipment() {
    if (mEquipStep != EQUIP_STEP_APPLIED) {
        return;
    }
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_CHANGE_CLOTH) &&
        dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_SHIRT)) {
        releaseClothTex();
    }
    dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    u16 flags = mEquipFlags;
    dItem::Item hat = pd->mEquipment.mHat;
    if (flags & dPlayerMgr::EQUIPMENT_HAT_NONE) {
        if (dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) && hat.mId == dItem::ITEM_ID_NONE) {
            loadHead();
            takeOffMiiMask();
        }
    } else if (flags & dPlayerMgr::EQUIPMENT_HAT_CAP) {
        if (!pd->mMiiMask && dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) &&
            hat.mId != dItem::ITEM_ID_NONE) {
            loadCapHead();
            takeOffMiiMask();
        }
    } else if (flags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK) {
        if (!pd->mMiiMask && dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) &&
            hat.mId != dItem::ITEM_ID_NONE) {
            loadFullMask();
            takeOffMiiMask();
        }
    } else if (flags & dPlayerMgr::EQUIPMENT_HAT_MASK) {
        if (!pd->mMiiMask && dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) &&
            hat.mId != dItem::ITEM_ID_NONE) {
            loadMaskHead();
            takeOffMiiMask();
        }
    } else if (flags & dPlayerMgr::EQUIPMENT_HAT_MASK_2) {
        if (!pd->mMiiMask && dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) &&
            hat.mId != dItem::ITEM_ID_NONE) {
            loadMaskHead();
            takeOffMiiMask();
        }
    } else if (flags & dPlayerMgr::EQUIPMENT_HAT_OVER_HEAD) {
        if (!pd->mMiiMask && dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HAT) &&
            hat.mId != dItem::ITEM_ID_NONE) {
            loadHeadAndCap();
            takeOffMiiMask();
        }
    }
    if (mEquipFlags & dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY) {
        if (dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_ACCESSORY) &&
            pd->mEquipment.mAcc.mId == dItem::ITEM_ID_NONE) {
            removeAccessory();
        }
    } else if ((mEquipFlags & dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY) && !pd->mMiiMask &&
               dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_ACCESSORY) &&
               pd->mEquipment.mAcc.mId != dItem::ITEM_ID_NONE && !pd->mMiiMask) {
        loadAccessory();
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK) {
            loadHead();
            takeOffMiiMask();
        }
    }
    setModelColor(0xFF);
    mEquipFlags = 0;
    mEquipStep = EQUIP_STEP_NONE;
    if (dPlayerMngr_c::isEquipChecked(mPlayerNo)) {
        int myNo = dNet::getMyNo();
        if (mPlayerNo == myNo) {
            dPlayerMngr_c::clearEquipChecked(mPlayerNo);
        } else {
            if (dPlayerMngr_c::updateEquip(mPlayerNo, dEquip_c::PART_SHIRT) == dPlayerMngr_c::EQUIP_STATE_CHANGE) {
                mEquipStep = EQUIP_STEP_LOAD_INIT;
                mEquipFlags |= dPlayerMgr::EQUIPMENT_CHANGE_CLOTH;
            }
            int hat = dPlayerMngr_c::updateEquip(mPlayerNo, dEquip_c::PART_HAT);
            if (hat == dPlayerMngr_c::EQUIP_STATE_LOAD || hat == dPlayerMngr_c::EQUIP_STATE_CHANGE) {
                int bone = dItem::HIDE_BONE_NONE;
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(pd->mEquipment.mHat);
                if (bitm != NULL) {
                    bone = bitm->getHideBone();
                }
                if (dPlayerMgr::setHatHideBone(bone, this)) {
                    mEquipStep = EQUIP_STEP_LOAD_INIT;
                }
            }
            int acc = dPlayerMngr_c::updateEquip(mPlayerNo, dEquip_c::PART_ACCESSORY);
            if (acc == dPlayerMngr_c::EQUIP_STATE_CHANGE) {
                mEquipStep = EQUIP_STEP_LOAD_INIT;
                mEquipFlags |= dPlayerMgr::EQUIPMENT_SHOW_ACCESSORY;
            } else if (acc == dPlayerMngr_c::EQUIP_STATE_LOAD) {
                mEquipStep = EQUIP_STEP_LOAD_INIT;
                mEquipFlags |= dPlayerMgr::EQUIPMENT_HIDE_ACCESSORY;
            }
            dPlayerMngr_c::clearEquipChecked(mPlayerNo);
        }
    }
}

// .text 0x2A758
void dAcPlayer_c::applyTool() {
    if (mHeldEquipStep != EQUIP_STEP_APPLIED && mHeldEquipStep != EQUIP_STEP_INIT_LOADED) {
        return;
    }
    if (!dPlayerMngr_c::isEquipReady(mPlayerNo, dEquip_c::PART_HELD)) {
        if (mHeldEquipStep == EQUIP_STEP_APPLIED) {
            mHeldEquipStep = EQUIP_STEP_INIT_LOADED;
        }
        return;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        int change = mHeldItemChange;
        if (change == dPlayerMgr::HELD_ITEM_CHANGE_REMOVE) {
            if (pd->mEquipment.mHeld.mId == dItem::ITEM_ID_NONE) {
                removeTool();
                if (mHeldEquipStep == EQUIP_STEP_INIT_LOADED) {
                    mMdl.removePartAnm(1, 0.0f);
                }
            }
        } else if (change == dPlayerMgr::HELD_ITEM_CHANGE_EQUIP && pd->mEquipment.mHeld.mId != dItem::ITEM_ID_NONE) {
            mpToolBank->change();
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_OUT;
            if (mHeldEquipStep == EQUIP_STEP_INIT_LOADED) {
                setToolAnm(mAnmId, FALSE, 60.0f);
            }
        }
    }
    mHeldItemChange = 0;
    mHeldEquipStep = EQUIP_STEP_NONE;
}

// .text 0x2A874
void dAcPlayer_c::removeTool() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
        mpToolBank->putAway();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_OUT;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
    mToolScale.set(0.0f, 0.0f, 0.0f);
}

// .text 0x2A8DC
void dAcPlayer_c::resetFaceTex() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
        nw4r::g3d::ResMdl mdl = mHeadMdl.getResMdl();
        nw4r::g3d::ResAnmTexPat eye = mFaceAnmMng.getEyeResAnmTexPat();
        eye.fn_8023A41C("e.0");
        nw4r::g3d::ResAnmTexPat mouth = mFaceAnmMng.getMouthResAnmTexPat();
        mouth.fn_8023A41C("m.0");
        mHeadMdl.getResMdl().ReleaseTexByName("e.0");
        mHeadMdl.getResMdl().ReleaseTexByName("m.0");
        nw4r::g3d::ResFile eyeFile = dHmnFaceTexMng_c::getResFile(getFaceNo());
        mdl.Bind(eyeFile);
        nw4r::g3d::ResFile mouthFile = dHmnFaceTexMng_c::getResFile(getGenderBodyNo());
        mdl.Bind(mouthFile);
        eye.Bind(eyeFile);
        mTexPatAnm.setAnm(mHeadMdl, eye, 0, m3d::FORWARD_ONCE);
        mTexPatAnm.setFrame(15.0f, 0);
        mouth.Bind(mouthFile);
        mTexPatAnm.setAnm(mHeadMdl, mouth, 1, m3d::FORWARD_LOOP);
        mHeadMdl.setAnm(mTexPatAnm);
    }
}

// .text 0x2AA60
void dAcPlayer_c::setModelColor(u8 brightness) {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    setMatColor(brightness, &mdl);
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
        nw4r::g3d::ResMdl mdl304 = mHeadMdl.getResMdl();
        setMatColor(brightness, &mdl304);
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL) {
        nw4r::g3d::ResMdl mdl344 = mHairMdl.getResMdl();
        setMatColor(brightness, &mdl344);
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
        nw4r::g3d::ResMdl mdl384 = mAccessoryMdl.getResMdl();
        setMatColor(brightness, &mdl384);
    }
    mFace.mBrightness = brightness;
}

// .text 0x2AB2C
void dAcPlayer_c::setMatColor(u8 brightness, nw4r::g3d::ResMdl *mdl) {
    if (mdl != NULL) {
        ulong num = mdl->GetResMatNumEntries();
        for (ulong i = 0; i < num; i++) {
            nw4r::g3d::ResMatChan chan = mdl->GetResMat(i).GetResMatChan();
            nw4r::ut::Color color;
            color.Set(brightness, brightness, brightness, 0xFF);
            chan.GXSetChanAmbColor(GX_COLOR0, color);
            color.Set(brightness, brightness, brightness, 0xFF);
            chan.GXSetChanMatColor(GX_COLOR0, color);
        }
    }
}

// .text 0x2ABE8
void dAcPlayer_c::rebindShoePalette() {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    mdl.ReleasePlttByName("b0");
    mPaletteMng.setPltt(getShoeNo(), 2);
    mdl.Bind(mPaletteMng.getResFile(2));
    mdl.ReleaseTexByName("b0");
    mdl.Bind(dHmnFaceTexMng_c::getResFile(getShoeNo2()));
}

// .text 0x2AC90
void dAcPlayer_c::createMiiFace() {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE)) {
        return;
    }
    BOOL found = FALSE;
    u32 index = 0;
    BOOL local = FALSE;
    if (mPlayerNo == dNet::getMyNo() || !dNet::isSessionActive()) {
        local = TRUE;
    }
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv->isFlag0(PRIVATE_FLAG0_TRAVELLING) && mPlayerNo == dNet::getMyNo()) {
        local = FALSE;
    }
    if (local) {
        if (mFace::isValidData(RFLDataSource_Official, mMiiMask, NULL)) {
            index = mMiiMask;
            found = TRUE;
        } else {
            for (u32 i = 0; i < 100; i++) {
                if (mFace::isValidData(RFLDataSource_Official, i, NULL)) {
                    index = i;
                    found = TRUE;
                    break;
                }
            }
        }
        if (!found) {
            mActorFlags &= ~(dPlayerMgr::ACTOR_FLAG_MII_MASK_OK | dPlayerMgr::ACTOR_FLAG_MII_FACE);
            dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->setMiiMask(0);
            return;
        }
    } else {
        found = TRUE;
    }
    if (found) {
        RFLDataSource source = RFLDataSource_Official;
        RFLMiddleDB *db = NULL;
        if (!local) {
            source = RFLDataSource_MiddleDB;
            if (priv->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
                db = dMiiIcon::getTravelDB();
            } else {
                db = dMiiIcon::getMemberDB(mPlayerNo);
            }
        }
        if (!mFace.create(source, index, RFLResolution_128, 0x3F, &mMiiAllocator, db)) {
            if (local) {
                mActorFlags &= ~(dPlayerMgr::ACTOR_FLAG_MII_MASK_OK | dPlayerMgr::ACTOR_FLAG_MII_FACE);
                dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->setMiiMask(0);
            } else {
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_MII_FACE;
                if (priv->isFlag0(PRIVATE_FLAG0_TRAVELLING) && mPlayerNo == dNet::getMyNo()) {
                    priv->setMiiMask(0);
                }
            }
        }
    }
}

// .text 0x2AE88
void dAcPlayer_c::setFaceMtxFromHead() {
    mMtx_c mtx;
    getHeadMtx(&mtx);
    setFaceMtx(&mtx);
}

// .text 0x2AEC4
void dAcPlayer_c::setFaceMtx(mMtx_c *headMtx) {
    headMtx->ZrotM(mAng(-0x4000));
    headMtx->XrotM(mAng(-0x514));
    headMtx->scaleM(0.34f, 0.34f, 0.34f);
    headMtx->transM(0.0f, 0.0f, 6.0f);
    mFace.setLocalMtx(headMtx);
}

// .text 0x2AF84
void dAcPlayer_c::calcFaceExpression() {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK)) {
        return;
    }
    if (mFaceExprKey != NULL) {
        if (mFaceExprKey->mFrame != -1) {
            if (mAnm.checkFrame(mFaceExprKey->mFrame)) {
                setFaceExpression(mFaceExprKey->mExpression);
                mFaceExprKey++;
            }
        } else if (mAnm.getPlayMode() == m3d::FORWARD_LOOP) {
            setFaceExprAnm(mAnmId);
        }
    } else {
        calcFaceBlink();
    }
}

// .text 0x2B03C
void dAcPlayer_c::setFaceExpression(u8 expression) {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK)) {
        return;
    }
    if (expression == _22DF[0]) {
        return;
    }
    switch (mState) {
    case dPlayerMgr::STATE_MII_MASK_IMMEDIATE:
    case dPlayerMgr::STATE_MII_MASK:
    case dPlayerMgr::STATE_CHANGE_MII_MASK:
        return;
    }
    switch (expression) {
    case RFLExp_Normal:
        mFace.getModel()->setExpression(RFLExp_Normal);
        break;
    case RFLExp_Smile:
        mFace.getModel()->setExpression(RFLExp_Smile);
        break;
    case RFLExp_Anger:
        mFace.getModel()->setExpression(RFLExp_Anger);
        break;
    case RFLExp_Sorrow:
        mFace.getModel()->setExpression(RFLExp_Sorrow);
        break;
    case RFLExp_Surprise:
        mFace.getModel()->setExpression(RFLExp_Surprise);
        break;
    case RFLExp_Blink:
        mFace.getModel()->setExpression(RFLExp_Blink);
        break;
    case RFLExp_Max:
        break;
    }
    _22DF[0] = expression;
}

// .text 0x2B130
void dAcPlayer_c::setFaceExprAnm(int anmId) {
    mFaceExprKey = NULL;
    u8 idx = dHmnAnm_c::getSubIdx(anmId);
    if (idx < ARRAY_SIZE(l_faceExprTracks)) {
        mFaceExprKey = l_faceExprTracks[idx];
    } else {
        setFaceExpression(RFLExp_Normal);
    }
}

// .text 0x2B198
void dAcPlayer_c::calcFaceBlink() {
    if (_22DF[2] == 0) {
        if (_22DF[1] == 0) {
            if (mFaceAnmMng.calcBlink()) {
                setFaceExpression(RFLExp_Blink);
                _22DF[2] = 15;
                if (mFaceAnmMng.mBlinkCount == 1) {
                    _22DF[1] = 8;
                }
            }
        } else {
            _22DF[1]--;
        }
    } else if (--_22DF[2] == 0) {
        setFaceExpression(RFLExp_Normal);
    }
}

// .text 0x2B238
BOOL dAcPlayer_c::putOnMiiMask() {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE)) {
        return FALSE;
    }
    BOOL local = FALSE;
    if (mPlayerNo == dNet::getMyNo() || !dNet::isSessionActive()) {
        local = TRUE;
    }
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv->isFlag0(PRIVATE_FLAG0_TRAVELLING) && mPlayerNo == dNet::getMyNo()) {
        local = FALSE;
    }
    RFLDataSource source = RFLDataSource_Official;
    RFLMiddleDB *db = NULL;
    if (!local) {
        source = RFLDataSource_MiddleDB;
        if (priv->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
            db = dMiiIcon::getTravelDB();
        } else {
            db = dMiiIcon::getMemberDB(mPlayerNo);
        }
    }
    if (!mFace::isValidData(source, mMiiMask, db)) {
        return FALSE;
    }
    if (local && !mFace.getModel()->change(source, mMiiMask, NULL)) {
        return FALSE;
    }
    dPlayerMgr::MiiData_c info;
    if (RFLGetAdditionalInfo((RFLCharInfo *)&info, source, db, mMiiMask) == RFLErrcode_Success) {
        GXColor color = info.mFavoriteColor;
        setBodyKColor(color);
        freeHeadHeap();
        freeAccessoryHeap();
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_MII_MASK;
    } else {
        return FALSE;
    }
    return TRUE;
}

// .text 0x2B3B4 (+ a dead blr at 0x2B3D4)
void dAcPlayer_c::takeOffMiiMask() {
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK)) {
        return;
    }
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE)) {
        return;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_MII_MASK;
    setupBodyPalette();
}

// .text 0x2B3D8
void dAcPlayer_c::checkMiiFace() {
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
            if (dMiiIcon::getTravelDB() != NULL) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_MII_FACE;
            }
        } else if (RFLGetAvailableOfficialDataNum() != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_MII_MASK_OK | dPlayerMgr::ACTOR_FLAG_MII_FACE;
        }
    } else if (dNet::isSessionActive()) {
        if (!fn_8010D9F4() && dMiiIcon::getMemberDB(mPlayerNo) != NULL) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_MII_FACE;
        }
    } else if (RFLGetAvailableOfficialDataNum() != 0) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_MII_FACE;
    }
}

// .text 0x2B4B0
dAcPlayer_c::myFace_c::myFace_c() {
    mBrightness = 0xFF;
    mDrawSetting.tevStageNum = 3;
    mDrawSetting.txcGenNum = 2;
}

// .text 0x2B504
dAcPlayer_c::myFace_c::~myFace_c() {}

// .text 0x2B58C
void dAcPlayer_c::myFace_c::loadTevSetting() {
    nw4r::g3d::ResFile file(dUki_c::getResData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl("mii_head_mat");
    nw4r::g3d::ResMat mat = mdl.GetResMat("mii_head_mat");
    nw4r::g3d::ResTev tev = mat.GetResTev();
    const GXTevStageID stages[2] = {GX_TEVSTAGE1, GX_TEVSTAGE2};
    for (int i = 0; i < 2; i++) {
        if (!tev.GXGetTevOrder(stages[i], &mTexCoord[i], &mTexMap[i], &mChannel[i])) {
            mTexCoord[i] = GX_TEXCOORD_NULL;
            mTexMap[i] = GX_TEXMAP_NULL;
            mChannel[i] = GX_COLOR0A0;
        }
        if (!tev.GXGetTevColorIn(stages[i], &mColorA[i], &mColorB[i], &mColorC[i], &mColorD[i])) {
            mColorA[i] = GX_CC_ZERO;
            mColorB[i] = GX_CC_CPREV;
            mColorC[i] = GX_CC_RASC;
            mColorD[i] = GX_CC_ZERO;
        }
        if (!tev.GXGetTevSwapMode(stages[i], &mRasSwap[i], &mTexSwap[i])) {
            mRasSwap[i] = GX_TEV_SWAP2;
            mTexSwap[i] = GX_TEV_SWAP2;
        }
        if (!tev.GXGetTevColorOp(stages[i], &mColorOp[i], &mColorBias[i], &mColorScale[i], &mColorClamp[i],
                                 &mColorOut[i])) {
            mColorOp[i] = GX_TEV_ADD;
            mColorBias[i] = GX_TB_ZERO;
            mColorScale[i] = GX_CS_SCALE_1;
            mColorClamp[i] = GX_TRUE;
            mColorOut[i] = GX_TEVPREV;
        }
        if (!tev.GXGetTevAlphaIn(stages[i], &mAlphaA[i], &mAlphaB[i], &mAlphaC[i], &mAlphaD[i])) {
            mAlphaA[i] = GX_CA_ZERO;
            mAlphaB[i] = GX_CA_ZERO;
            mAlphaC[i] = GX_CA_ZERO;
            mAlphaD[i] = GX_CA_APREV;
        }
        if (!tev.GXGetTevAlphaOp(stages[i], &mAlphaOp[i], &mAlphaBias[i], &mAlphaScale[i], &mAlphaClamp[i],
                                 &mAlphaOut[i])) {
            mAlphaOp[i] = GX_TEV_ADD;
            mAlphaBias[i] = GX_TB_ZERO;
            mAlphaScale[i] = GX_CS_SCALE_1;
            mAlphaClamp[i] = GX_FALSE;
            mAlphaOut[i] = GX_TEVPREV;
        }
        if (!tev.GXGetTevIndirect(stages[i], &mIndStage[i], &mIndFormat[i], &mIndBias[i], &mIndMtx[i],
                                  &mIndWrapS[i], &mIndWrapT[i], &mIndAddPrev[i], &mIndUtcLod[i], &mIndAlphaSel[i])) {
            mIndStage[i] = GX_INDTEXSTAGE0;
            mIndFormat[i] = GX_ITF_8;
            mIndBias[i] = GX_ITB_NONE;
            mIndMtx[i] = GX_ITM_OFF;
            mIndWrapS[i] = GX_ITW_OFF;
            mIndWrapT[i] = GX_ITW_OFF;
            mIndAddPrev[i] = GX_FALSE;
            mIndUtcLod[i] = GX_FALSE;
            mIndAlphaSel[i] = GX_ITBA_OFF;
        }
    }
    if (!tev.GXGetTevSwapModeTable(GX_TEV_SWAP2, &mSwapTable[0], &mSwapTable[1], &mSwapTable[2], &mSwapTable[3])) {
        mSwapTable[0] = GX_CH_RED;
        mSwapTable[1] = GX_CH_GREEN;
        mSwapTable[2] = GX_CH_BLUE;
        mSwapTable[3] = GX_CH_ALPHA;
    }
}

// .text 0x2B9F4
void dAcPlayer_c::myFace_c::setupDrawOpa() {
    static const f32 l_envScale = 1.0f / 0.34f;

    void *image;
    u16 width;
    u16 height;
    GXCITexFmt format;
    f32 minLod;
    f32 maxLod;
    GXBool mipmap;
    mMtx_c mtx;
    mMtx_c envMtx;
    mMtx_c envScale;
    m3d::resetMaterial();
    nw4r::g3d::ResFile file(dUki_c::getResData());
    nw4r::g3d::ResTex tex = file.GetResTex("mii_head_env");
    nw4r::g3d::ResPltt pltt = file.GetResPltt("mii_head_env");
    tex.GetTexObjCIParam(&image, &width, &height, &format, &minLod, &maxLod, &mipmap);
    GXTexObj texObj;
    GXInitTexObjCI(&texObj, image, width, height, (GXTexFmt)format, GX_MIRROR, GX_CLAMP, mipmap, GX_TLUT0);
    GXTlutObj tlutObj;
    GXInitTlutObj(&tlutObj, pltt.GetPlttData(), pltt.GetFmt(), pltt.GetNumEntries());
    GXLoadTlut(&tlutObj, GX_TLUT0);
    GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, minLod, maxLod, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX_TEXMAP1);

    PSMTXIdentity(mtx);
    getLocalMtx(&mtx);
    mtx.scaleM(l_envScale, l_envScale, l_envScale);
    if (dCamera_c::sInstance != NULL) {
        nw4r::math::MTX34 *view = &dCamera_c::sInstance->mCamera.getViewMatrix();
        if (view != NULL) {
            PSMTXConcat(*view, mtx, mtx);
        }
    }
    mtx._03 = 0.0f;
    mtx._13 = 0.0f;
    mtx._23 = 0.0f;
    const nw4r::math::MTX34 *env = nw4r::g3d::G3DState::GetEnvironmentTexMtxPtr();
    if (env != NULL) {
        EGG::Matrix34f envCopy(env->_00, env->_01, env->_02, env->_03, env->_10, env->_11, env->_12, env->_13,
                               env->_20, env->_21, env->_22, env->_23);
        envMtx = *(nw4r::math::MTX34 *)&envCopy;
        PSMTXScale(envScale, 2.0f, 1.0f, 1.0f);
        envScale.transM(0.5f, 0.0f, 0.0f);
        PSMTXConcat(envMtx, envScale, envMtx);
        PSMTXConcat(envMtx, mtx, mtx);
    }
    GXLoadTexMtxImm(mtx, GX_TEXMTX1, GX_MTX_3x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_NRM, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTevSwapModeTable(GX_TEV_SWAP2, mSwapTable[0], mSwapTable[1], mSwapTable[2], mSwapTable[3]);
    const GXTevStageID stages[2] = {GX_TEVSTAGE1, GX_TEVSTAGE2};
    for (int i = 0; i < 2; i++) {
        GXSetTevIndirect(stages[i], mIndStage[i], mIndFormat[i], mIndBias[i], mIndMtx[i], mIndWrapS[i],
                         mIndWrapT[i], mIndAddPrev[i], mIndUtcLod[i], mIndAlphaSel[i]);
        GXSetTevSwapMode(stages[i], mRasSwap[i], mTexSwap[i]);
        GXSetTevOrder(stages[i], GX_TEXCOORD1, GX_TEXMAP1, mChannel[i]);
        GXSetTevColorIn(stages[i], mColorA[i], mColorB[i], mColorC[i], mColorD[i]);
        GXSetTevColorOp(stages[i], mColorOp[i], mColorBias[i], mColorScale[i], mColorClamp[i], mColorOut[i]);
        GXSetTevAlphaIn(stages[i], mAlphaA[i], mAlphaB[i], mAlphaC[i], mAlphaD[i]);
        GXSetTevAlphaOp(stages[i], mAlphaOp[i], mAlphaBias[i], mAlphaScale[i], mAlphaClamp[i], mAlphaOut[i]);
    }
    GXColor amb = {0xFF, 0xFF, 0xFF, 0xFF};
    nw4r::g3d::LightSet lightSet = dLight::getLightSet(0);
    nw4r::g3d::AmbLightObj *ambObj = lightSet.GetAmbLightObj();
    if (ambObj != NULL) {
        amb.r = ambObj->r;
        amb.g = ambObj->g;
        amb.b = ambObj->b;
        amb.a = ambObj->a;
    }
    GXColor matColor = {0, 0, 0, 0xFF};
    matColor.r = mBrightness;
    matColor.g = mBrightness;
    matColor.b = mBrightness;
    GXLightID lightMask = dLight::getLightMask();
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, lightMask, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanAmbColor(GX_COLOR0, amb);
    GXSetChanMatColor(GX_COLOR0, matColor);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_COPY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

// .text 0x2BEDC
void dAcPlayer_c::myFace_c::drawXluCore() {
    RFLLoadVertexSetting(&mDrawSetting);
    RFLLoadMaterialSetting(&mDrawSetting);
    GXSetZCompLoc(GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_OR, GX_NEVER, 0);
    EGG::Fog *fog = m3d::getFogMgr()->mFog[0];
    if (fog && (fog->mFlags & EGG::Fog::FLAG_ENABLE)) {
        GXSetFog(fog->mFogType, fog->GetStartZ(), fog->GetEndZ(), fog->GetNearZ(), fog->GetFarZ(), fog->mColor);
    }
    setupDrawXlu();
    RFLDrawXluCore(&mModel, &mDrawSetting);
}

// Stripped-function witness. Names/types of these folded statics are provisional.
void dAPlayer_retainFaceConstants() {
    static const u32 l_unknown263C = 0x050E060F;
    static const u8 l_unknown2640 = 6;
}
