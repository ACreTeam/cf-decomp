// dAcPlayer_c: net catch, fishing, furniture, bed / chair and shovel states (.text 0x9660..0x129C4; the
// string-literal functions of item.inc up to its first data object are placed at the end of this
// section). Included by d_a_player.cpp.

// .text 0x9660
void dAcPlayer_c::initNetCatch() {
    static const int l_turnFrames = 30;

    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
    setAnm(HMN_ANM_NET_GET1, l_turnFrames, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mpToolBank->setNetAnm1A9(m3d::FORWARD_ONCE, 8.0f);
    mTargetAngle = 0;
    mTurnFrames = sLib::distanceAngle(0, mAngle.y.mAngle) / l_turnFrames;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (dCamera_c::sInstance != NULL) {
            dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->mute0();
        }
        mStep = 0xA;
        if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mCatalog.isComplete(dItem::KIND_INSECT)) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE;
        } else {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE;
        }
        mActionItem = dItem::Item(dItem::ITEM_IDX_COMMON_BUTTERFLY).getId();
        if (mInsectLink.mpInsect != NULL) {
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->initHold();
            const int &type = (int)static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->getInsectType();
            dItem::Item item(dItem::ITEM_IDX_COMMON_BUTTERFLY, type, FALSE);
            mActionItem = item.getId();
            if (putInPocket(&item)) {
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
            } else {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
            }
            dClock::addBugCount();
            dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
            priv->mCatalog.registerItem(item.getId(), TRUE);
            dActivityLog::onCatchBug(type, mPlayerNo);
        }
        _2188 = mActionItem.mId;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_INSECT;
        setNetCatchParam((netCatchParam_s *)&mRemoteActionParams, mActionItem.mId);
    } else {
        mActionItem = ((netCatchParam_s *)&mRemoteActionParams)->mItem;
        if (g_playActorMng != NULL) {
            mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
            if (mInsectLink.mpInsect != NULL) {
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->initHold();
                mVec3_c scale(0.0f, 0.0f, 1.0f);
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
                    ->setHeldScale(&scale);
            }
        }
    }
}

// .text 0x98E0
void dAcPlayer_c::executeNetCatch() {
    static const f32 l_catchFrame = 20.0f;
    static const f32 l_showFrame = 67.0f;
    static const f32 l_showEndFrame = l_showFrame + l_catchFrame;

    f32 frame = mAnm.getFrame();
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    void *insect = mInsectLink.mpInsect;
    if (insect != NULL) {
        if (mAnm.isStop()) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        } else if (mAnm.checkFrame(l_showEndFrame)) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        } else if (frame <= l_showFrame) {
            mVec3_c scale(0.0f, 0.0f, 1.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        } else if (frame > l_showFrame && frame < l_showEndFrame) {
            f32 s = 0.056f * (mAnm.getFrame() - l_showFrame);
            if (s > 1.0f) {
                s = 1.0f;
            }
            mVec3_c scale(s, s, 1.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        }
        if (frame > 119.0f && mAnmId == HMN_ANM_NET_GET1) {
            createCatchEffect(static_cast<dInsFldBase_c *>(insect));
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (frame > 101.0f) {
            stepGetInsectMsg();
        }
    } else if (mInsectLink.mpInsect == NULL && g_playActorMng != NULL) {
        mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
        if (mInsectLink.mpInsect != NULL) {
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->initHold();
            mVec3_c scale(0.0f, 0.0f, 1.0f);
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
                ->setHeldScale(&scale);
        }
    }
}

// .text 0x6580
void dAcPlayer_c::createCatchEffect(dInsFldBase_c *actor) {
    if (actor == NULL) {
        return;
    }
    switch (actor->mType) {
    case 0x1A:
    case 0x38:
    case 0x3A:
    case 0x3B:
        mLevelEffect3.create("af_ply_kore", &mMdlCallback.mHandPos, NULL, NULL, 0);
        break;
    }
}

// .text 0x9B0C
void dAcPlayer_c::endNetCatch(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop0(0xB, 0x1A);
        }
    }
    if (nextState != dPlayerMgr::STATE_CATCH_POSE) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (dCamera_c::sInstance != NULL) {
                dCamera_c::sInstance->mRequestedMode = dCamera_c::sInstance->mDefaultMode;
            }
        } else if (nextState != dPlayerMgr::STATE_IDLE && nextState != dPlayerMgr::STATE_NET_POCKET) {
            mpToolBank->setNetWait(m3d::FORWARD_LOOP, 8.0f);
        }
    }
}

// .text 0x9BC4
void dAcPlayer_c::setNetCatchParam(netCatchParam_s *param, u16 item) {
    param->mItem = item;
}

// .text 0x9BCC
void dAcPlayer_c::initNetPocket() {
    if (mPlayerNo == dNet::getMyNo()) {
        setNetPocketParam((netPocketParam_s *)&mRemoteActionParams,
                          (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
    } else if (g_playActorMng != NULL) {
        mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
    }
    if (setAnm(HMN_ANM_NET_PUTAWAY1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE)) {
        mpToolBank->setNetAnm1B3(m3d::FORWARD_ONCE, 8.0f);
    }
}

// .text 0x9CB0
void dAcPlayer_c::executeNetPocket() {
    if (mAnmId != HMN_ANM_NET_PUTAWAY1) {
        return;
    }
    if (mInsectLink.mpInsect == NULL && g_playActorMng != NULL) {
        mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
    }
    void *insect = mInsectLink.mpInsect;
    if (insect != NULL) {
        if (mAnm.checkFrame(20.0f)) {
            mVec3_c scale(0.0f, 0.0f, 0.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        } else if (mAnm.getFrame() < 20.0f) {
            f32 s = 1.0f - 0.056f * mAnm.getFrame();
            if (s < 0.0f) {
                s = 0.0f;
            }
            mVec3_c scale(s, s, 1.0f);
            static_cast<dInsFldBase_c *>(insect)->setHeldScale(&scale);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
                setState(dPlayerMgr::STATE_POCKET_FULL);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        }
    } else if (mAnm.isStop()) {
        if (((netPocketParam_s *)&mRemoteActionParams)->mPocketFull != 0) {
            setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
    if (mAnm.checkFrame(15.0f)) {
        startSound(SE_gasagoso_return);
    }
}

// .text 0x9E90
void dAcPlayer_c::endNetPocket(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && nextState != dPlayerMgr::STATE_IDLE &&
        nextState != dPlayerMgr::STATE_POCKET_FULL) {
        mpToolBank->setNetWait(m3d::FORWARD_LOOP, 8.0f);
    }
    if (mInsectLink.mpInsect != NULL) {
        static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->cancelHold();
    }
    if (mInsectLink.mpInsect != NULL) {
        dInsectField::removeNpcLink(mInsectLink.mpInsect, &mInsectLink);
    }
}

// .text 0x9F1C
void dAcPlayer_c::setNetPocketParam(netPocketParam_s *param, u8 pocketFull) {
    param->mPocketFull = pocketFull;
}

// .text 0x9F24
void dAcPlayer_c::initFishingCast() {
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        setFishingCastParam((floatPosParam_s *)&mRemoteActionParams, mpToolBank->mFloat.mTarget.x,
                            mpToolBank->mFloat.mTarget.z);
    } else {
        floatPosParam_s *param = (floatPosParam_s *)&mRemoteActionParams;
        f32 x = param->mX;
        f32 z = param->mZ;
        mVec3_c target;
        mVec3_c pos(x, 0.0f, z);
        target.set(x, dBGCF::getGroundY(&pos, FALSE), z);
        mpToolBank->mFloat.setTarget(&target);
    }
    if (setAnm(HMN_ANM_POLE_SWING1, 3.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE)) {
        mpToolBank->setPoleSwing(m3d::FORWARD_ONCE, 8.0f);
    }
}

// .text 0xA02C
void dAcPlayer_c::executeFishingCast() {
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnmId == HMN_ANM_POLE_SWING1) {
            if (mAnm.isStop()) {
                if (setAnm(HMN_ANM_POLE_WAIT_NIBBLE1, m3d::FORWARD_LOOP)) {
                    mpToolBank->setPoleWaitNibble(m3d::FORWARD_LOOP, 8.0f);
                }
            } else if (mAnm.checkFrame(45.0f)) {
                startSound(SE_RodStroke);
                startRumble(dRumble::PATTERN_ROD_CAST);
            }
        } else if (mAnmId == HMN_ANM_POLE_WAIT_NIBBLE1) {
            if (mpToolBank->mFloat.isInWater()) {
                startRumble(dRumble::PATTERN_ROD_NIBBLE);
                setState(dPlayerMgr::STATE_FISHING_WAIT);
            }
        }
    } else {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_POLE_SWING1) {
            if (mAnm.isStop()) {
                if (setAnm(HMN_ANM_POLE_WAIT_NIBBLE1, m3d::FORWARD_LOOP)) {
                    mpToolBank->setPoleWaitNibble(m3d::FORWARD_LOOP, 8.0f);
                }
            } else if (mAnm.checkFrame(45.0f)) {
                startSound(SE_RodStroke);
            }
        }
        dUki_c *uki = &mpToolBank->mFloat;
        if (uki->mState == dUki_c::STATE_HOLD) {
            uki->setState(dUki_c::STATE_SWING);
        }
    }
}

// .text 0xA1B8
void dAcPlayer_c::endFishingCast(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        if (dCamera_c::sInstance != NULL) {
            dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_TOOL;
        }
        dBgm::VolStateFish_c *fish = &dBgm::l_mgr.mVolCtrl.mFish;
        if (fish != NULL) {
            fish->start();
        }
        mActorFlags2 &= ~0x20000;
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xA268
BOOL dAcPlayer_c::isFishingState(int state) {
    switch (state) {
    case dPlayerMgr::STATE_FISHING_CAST:
    case dPlayerMgr::STATE_FISHING_WAIT:
    case dPlayerMgr::STATE_FISHING_HOOKED:
    case dPlayerMgr::STATE_FISHING_FAIL:
    case dPlayerMgr::STATE_FISHING_PUT_BACK:
    case dPlayerMgr::STATE_FISHING_LAND:
    case dPlayerMgr::STATE_FISHING_GET:
    case dPlayerMgr::STATE_FISHING_PUT_AWAY:
    case dPlayerMgr::STATE_SHOW_CATCH_AT:
        return TRUE;
    }
    return FALSE;
}

// .text 0xA298
void dAcPlayer_c::setFishingCastParam(floatPosParam_s *param, f32 x, f32 z) {
    param->mX = x;
    param->mZ = z;
}

// .text 0xA2A4
void dAcPlayer_c::initFishingCastMiss() {
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        setFishingCastMissParam((floatPosParam_s *)&mRemoteActionParams,
                                mpToolBank->mFloat.mTarget.x, mpToolBank->mFloat.mTarget.z);
        _22A4[7] = 100;
    } else {
        floatPosParam_s *param = (floatPosParam_s *)&mRemoteActionParams;
        f32 x = param->mX;
        f32 z = param->mZ;
        mVec3_c target;
        mVec3_c pos(x, 0.0f, z);
        target.set(x, dBGCF::getGroundY(&pos, FALSE), z);
        mpToolBank->mFloat.setTarget(&target);
    }
    if (setAnm(HMN_ANM_POLE_SUKA1, m3d::FORWARD_ONCE)) {
        mpToolBank->setPoleSuka(m3d::FORWARD_ONCE, 8.0f);
    }
}

// .text 0xA3A0
void dAcPlayer_c::executeFishingCastMiss() {
    if (mPlayerNo == dNet::getMyNo()) {
        calcPointerTurn();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(40.0f)) {
            startSound(SE_RodStrokeSmall);
        }
        if (mAnmId == HMN_ANM_NML_WAIT1) {
            if (_22A4[7] != 0) {
                _22A4[7]--;
            }
            if (mpToolBank->mFloat.mState == dUki_c::STATE_HOLD || _22A4[7] == 0) {
                setState(dPlayerMgr::STATE_IDLE);
            }
        }
    } else {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_POLE_SUKA1) {
            if (mAnm.isStop()) {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            } else if (mAnm.checkFrame(40.0f)) {
                startSound(SE_RodStrokeSmall);
            }
        }
    }
}

// .text 0xA4D0
void dAcPlayer_c::endFishingCastMiss(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
    } else if (nextState != dPlayerMgr::STATE_IDLE) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xA53C
void dAcPlayer_c::setFishingCastMissParam(floatPosParam_s *param, f32 x, f32 z) {
    param->mX = x;
    param->mZ = z;
}

// .text 0xA548
void dAcPlayer_c::initFishingWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        mTargetAngle = mAngle.y.mAngle;
        dUki_c *uki = &mpToolBank->mFloat;
        setFishingWaitParam((fishingWaitParam_s *)&mRemoteActionParams, uki->mPos[0].x,
                            uki->mPos[0].z, uki->mState);
    } else if (mState != dPlayerMgr::STATE_FISHING_CAST || mpToolBank->getToolType() != HMN_TOOL_ROD) {
        fishingWaitParam_s *param = (fishingWaitParam_s *)&mRemoteActionParams;
        mVec3_c pos(param->mX, mPos.y, param->mZ);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        mpToolBank->mFloat.setPos(pos.x, chk.getWaterY(), pos.z);
        mpToolBank->mFloat.setState(param->mFloatState);
    }
    if (setAnm(HMN_ANM_POLE_WAIT_NIBBLE1, m3d::FORWARD_LOOP)) {
        mpToolBank->setPoleWaitNibble(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xA644
void dAcPlayer_c::executeFishingWait() {
    sLib::chaseAngle(&mAngle.y.mAngle, dActor_c::targetAngleY(&mPos, &mpToolBank->mFloat.mPos[0]),
                     0xB61);
    if (mPlayerNo == dNet::getMyNo()) {
        dUki_c *uki = &mpToolBank->mFloat;
        setFishingWaitParam((fishingWaitParam_s *)&mRemoteActionParams, uki->mPos[0].x,
                            uki->mPos[0].z, uki->mState);
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE) {
            dUki_c::setFish(mPlayerNo, NULL);
            setState(dPlayerMgr::STATE_FISHING_PUT_BACK);
        } else if (_22FE[5] != 0 || _2304[1] != 0) {
            dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
            if (fish == NULL) {
                setState(dPlayerMgr::STATE_FISHING_PUT_BACK);
            } else if (fish->pull()) {
                setState(dPlayerMgr::STATE_FISHING_HOOKED);
            } else {
                setState(dPlayerMgr::STATE_FISHING_PUT_BACK);
            }
        }
    } else {
        getRemoteActionParams(&mRemoteActionParams);
        fishingWaitParam_s *param = (fishingWaitParam_s *)&mRemoteActionParams;
        dUki_c *uki = &mpToolBank->mFloat;
        u8 cur = uki->mState;
        u8 state = param->mFloatState;
        if (cur == dUki_c::STATE_HOLD) {
            mVec3_c pos(param->mX, mPos.y, param->mZ);
            dBGCF::groundChk_c chk(&pos, 0, 0, 0);
            uki->setPos(pos.x, chk.getWaterY(), pos.z);
            uki->setState(dUki_c::STATE_FLOAT);
        }
        if (state == dUki_c::STATE_FLOAT) {
            if (cur == dUki_c::STATE_BITE) {
                uki->setState(state);
            }
        } else if (state == dUki_c::STATE_BITE) {
            if (cur == dUki_c::STATE_FLOAT) {
                uki->setState(state);
            } else if (cur == dUki_c::STATE_DROP) {
                mVec3_c *floatPos = &uki->mPos[0];
                dBGCF::groundChk_c chk(floatPos, 0, 0, 0);
                uki->setPos(floatPos->x, chk.getWaterY(), floatPos->z);
                uki->setState(state);
            }
        }
        if (uki->isInWater()) {
            mVec3_c pos = uki->mPos[0];
            mVec3_c target(param->mX, pos.y, param->mZ);
            mVec3_c diff = target - pos;
            if (diff.x * diff.x + diff.z * diff.z > 256.0f) {
                cLib::addCalcPosXZ2(&pos, target, 0.1f, 5.0f);
                uki->setPos(&pos);
            }
        }
    }
}

// .text 0xA8DC
void dAcPlayer_c::endFishingWait(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && !isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xA948
void dAcPlayer_c::setFishingWaitParam(fishingWaitParam_s *param, f32 x, f32 z, u8 floatState) {
    param->mX = x;
    param->mZ = z;
    param->mFloatState = floatState;
}

// .text 0xA958
void dAcPlayer_c::initFishingHooked() {
    setAnm(HMN_ANM_POLE_HIT1, 20.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    mpToolBank->setPoleHit(m3d::FORWARD_LOOP, 8.0f);
    dUki_c *uki = &mpToolBank->mFloat;
    if (mPlayerNo == dNet::getMyNo()) {
        setFishingHookedParam((floatPosParam_s *)&mRemoteActionParams, uki->mPos[0].x,
                              uki->mPos[0].z);
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->mute0();
        }
        dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
        if (fish != NULL) {
            switch (fish->mParam.mSize) {
            case FISH_SIZE_TINY:
                startRumble(dRumble::PATTERN_HOOKED_S);
                break;
            case FISH_SIZE_SMALL:
                startRumble(dRumble::PATTERN_HOOKED_S);
                break;
            case FISH_SIZE_MEDIUM:
                startRumble(dRumble::PATTERN_HOOKED_M);
                break;
            case FISH_SIZE_NARROW:
                startRumble(dRumble::PATTERN_HOOKED_M);
                break;
            case FISH_SIZE_LARGE:
                startRumble(dRumble::PATTERN_HOOKED_M);
                break;
            case FISH_SIZE_XLARGE:
                startRumble(dRumble::PATTERN_HOOKED_M);
                break;
            case FISH_SIZE_HUGE:
                startRumble(dRumble::PATTERN_HOOKED_L);
                break;
            case FISH_SIZE_FIN:
                startRumble(dRumble::PATTERN_HOOKED_L);
                break;
            }
        }
    } else {
        floatPosParam_s *param = (floatPosParam_s *)&mRemoteActionParams;
        mVec3_c pos(param->mX, mPos.y, param->mZ);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        pos.y = chk.getWaterY();
        if (mState != dPlayerMgr::STATE_FISHING_WAIT || uki->mPos[0].y != pos.y) {
            uki->setPos(&pos);
        }
    }
}

// .text 0x65D8
void dAcPlayer_c::stepGetFishMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play0(10);
            }
            attachDemoActor(this);
            int kind = 0;
            dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
            if (fish != NULL) {
                kind = fish->getCatchKind();
            }
            setMessageLabel("sys_MESS/SYS_Get_fish");
            setMessageCode(kind + 1);
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
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE) && pd->mCatalog.isComplete(dItem::KIND_FISH)) {
            mMsgStep = MSG_STEP_QUEUE_COMPLETE;
        } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
            int x;
            int z;
            if (dFgobjManager::findPutSpot(0x10, &x, &z, NULL)) {
                mMsgStep = MSG_STEP_QUEUE_ASK_RELEASE;
                dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
                if (fish != NULL) {
                    BOOL trash = FALSE;
                    int kind = fish->mKind;
                    if (kind >= 0x40) {
                        BOOL key = FALSE;
                        if (kind >= 0x43 && kind < 0x4B) {
                            key = TRUE;
                        }
                        if (!key) {
                            trash = TRUE;
                        }
                    }
                    if (trash) {
                        mMsgStep = MSG_STEP_QUEUE_ASK_RELEASE_FISH;
                    }
                }
            } else {
                mMsgStep = MSG_STEP_QUEUE_LET_GO;
            }
        }
        break;
    }
    case 12:
        switch (mMsgStep) {
        case MSG_STEP_SWAP_FISH:
            changeStateAfterMsg(dPlayerMgr::STATE_FISHING_PUT_AWAY);
            break;
        case MSG_STEP_RELEASE:
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_INSECT;
            changeStateAfterMsg(dPlayerMgr::STATE_SHOW_CATCH_AT);
            break;
        case MSG_STEP_RELEASE_FISH:
            if (!mActionItem.isFish()) {
                if (changeStateAfterMsg(dPlayerMgr::STATE_PUT_HELD_ITEM)) {
                    deleteHookedFish();
                }
            } else {
                changeStateAfterMsg(dPlayerMgr::STATE_IDLE);
            }
            break;
        case MSG_STEP_COMPLETE:
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_INSECT;
            setState(dPlayerMgr::STATE_CATCH_POSE);
            break;
        default:
            changeStateAfterMsg(dPlayerMgr::STATE_FISHING_PUT_AWAY);
            break;
        }
        break;
    }
}

// .text 0x688C
void dAcPlayer_c::stepPlayerMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(MSG_NO_MOVE_WITH_GUESTS);
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
        changeStateAfterMsg(dPlayerMgr::STATE_IDLE);
        break;
    }
}

// .text 0x6988
void dAcPlayer_c::checkDigUnit(u32 mode, BOOL alt) {
    if (mObjcPos.mHitType != 0) {
        if (mode == 1) {
            startToolSound(SE_Boyon);
            startRumble(dRumble::PATTERN_SHOVEL_BOUNCE);
            breakHitSnowmen();
        }
        return;
    }
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(pos, mTargetUnitX, mTargetUnitZ);
    u16 item = getFieldRealItem(&pos, 0, dField::getInfo(0));
    if (mode == 0) {
        if (item != dItem::ITEM_ID_NONE) {
            return;
        }
        if (isRafflesiaAt(&pos)) {
            dFgobjManager::stepOnUnit(mPlayerNo, &pos);
        } else {
            dEffect::create("af_itm_scoop_hitstar", &mPos, &mAngle, NULL);
        }
    } else if (mode == 1) {
        if (item == dItem::ITEM_ID_NONE) {
            if (isRafflesiaAt(&pos)) {
                startToolSound(SE_Boyon);
                startRumble(dRumble::PATTERN_SHOVEL_BOUNCE);
            } else {
                if (alt) {
                    startSound(SE_ScoopHit_Extra);
                } else {
                    startSound(SE_ScoopHit);
                }
                startRumble(dRumble::PATTERN_SHOVEL_HIT);
            }
        } else {
            startToolSound(SE_Boyon);
            startRumble(dRumble::PATTERN_SHOVEL_BOUNCE);
        }
    }
}

// .text 0xAB10
void dAcPlayer_c::executeFishingHooked() {
    if (mPlayerNo == dNet::getMyNo()) {
        dUki_c *uki = &mpToolBank->mFloat;
        dFishFldShadow_c *fish = uki->getFish();
        if (fish == NULL) {
            setState(dPlayerMgr::STATE_FISHING_FAIL);
            return;
        }
        mVec3_c mouth;
        if (fish->getMouthPos(&mouth)) {
            uki->setFishPos(&mouth);
        }
        if (fish->isCatching()) {
            setState(dPlayerMgr::STATE_FISHING_LAND);
        }
    } else if (mAnmId != HMN_ANM_POLE_GET1) {
        dFishFldShadow_c *fish;
        dUki_c *uki = &mpToolBank->mFloat;
        fish = uki->getFish();
        if (fish != NULL && fish->isCatching()) {
            setAnm(HMN_ANM_POLE_GET1, m3d::FORWARD_ONCE);
            mpToolBank->setPoleGet1(m3d::FORWARD_ONCE, 8.0f);
            startSound(SE_RodBack);
        } else {
            if (uki->mState == dUki_c::STATE_HOLD) {
                floatPosParam_s *param = (floatPosParam_s *)&mRemoteActionParams;
                mVec3_c pos(param->mX, mPos.y, param->mZ);
                dBGCF::groundChk_c chk(&pos, 0, 0, 0);
                uki->setPos(pos.x, chk.getWaterY(), pos.z);
                uki->setState(dUki_c::STATE_HOOKED);
            }
            if (fish != NULL) {
                mVec3_c mouth;
                if (fish->getMouthPos(&mouth)) {
                    uki->setFishPos(&mouth);
                }
            }
        }
    }
}

// .text 0xACAC
void dAcPlayer_c::endFishingHooked(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dRumble::stop();
        dBgm::VolStateFish_c *fish = &dBgm::l_mgr.mVolCtrl.mFish;
        if (fish != NULL) {
            fish->end();
        }
        if (nextState != dPlayerMgr::STATE_FISHING_LAND) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->stop0(0xB, 0x1A);
            }
        }
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xAD58
void dAcPlayer_c::setFishingHookedParam(floatPosParam_s *param, f32 x, f32 z) {
    param->mX = x;
    param->mZ = z;
}

// .text 0xAD64
void dAcPlayer_c::initFishingFail() {
    if (mPlayerNo == dNet::getMyNo()) {
        getFishingFailParam()->set(mpToolBank->mFloat.mPos[0].x, mpToolBank->mFloat.mPos[0].z);
    } else {
        floatPosParam_s *param = (floatPosParam_s *)&mRemoteActionParams;
        mVec3_c pos(param->mX, mPos.y, param->mZ);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        mpToolBank->mFloat.setPos(pos.x, chk.getWaterY(), pos.z);
    }
    if (setAnm(HMN_ANM_POLE_FAIL1, m3d::FORWARD_ONCE)) {
        mpToolBank->setPoleFail(m3d::FORWARD_ONCE, 8.0f);
    }
    startSound(SE_RodBack);
}

// .text 0xAE34
void dAcPlayer_c::executeFishingFail() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        if (mAnmId == HMN_ANM_NML_WAIT1 && mpToolBank->mFloat.mState == dUki_c::STATE_HOLD) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        dUki_c *uki = &mpToolBank->mFloat;
        if (uki->mState == dUki_c::STATE_HOLD) {
            uki->setState(dUki_c::STATE_REEL_EMPTY);
        }
    }
}

// .text 0xAF00
void dAcPlayer_c::endFishingFail(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mPreviousMode;
        }
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xAF88
void dAcPlayer_c::fishingFailParam_c::set(f32 x, f32 z) {
    mFloatX = x;
    mFloatZ = z;
}

// .text 0xAF94
void dAcPlayer_c::initFishingPutBack() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (dCamera_c::sInstance != NULL) {
            dCamera_c::sInstance->restoreModeFast();
        }
        getFishingPutBackParam()->set(mpToolBank->mFloat.mPos[0].x, mpToolBank->mFloat.mPos[0].z);
    } else {
        mVec3_c pos(getFishingPutBackParam()->mFloatX, mPos.y, getFishingPutBackParam()->mFloatZ);
        dBGCF::groundChk_c ground(&pos, 0, 0, 0);
        mpToolBank->mFloat.setPos(pos.x, ground.getWaterY(), pos.z);
    }
    if (setAnm(HMN_ANM_POLE_PUTBACK1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE)) {
        mpToolBank->setPolePutback(m3d::FORWARD_ONCE, 8.0f);
    }
    startSound(SE_RodBack);
}

// .text 0xB098
void dAcPlayer_c::executeFishingPutBack() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        if (mAnmId == HMN_ANM_NML_WAIT1 && mpToolBank->mFloat.mState != dUki_c::STATE_REEL) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
        dUki_c *uki = &mpToolBank->mFloat;
        if (uki->mState == dUki_c::STATE_HOLD) {
            uki->setState(dUki_c::STATE_REEL);
        }
    }
}

// .text 0xB164
void dAcPlayer_c::endFishingPutBack(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::VolStateFish_c *fish = &dBgm::l_mgr.mVolCtrl.mFish;
        if (fish != NULL) {
            fish->end();
        }
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xB1E8
void dAcPlayer_c::fishingPutBackParam_c::set(f32 x, f32 z) {
    mFloatX = x;
    mFloatZ = z;
}

// .text 0xB1F4
void dAcPlayer_c::initFishingLand() {
    if (mPlayerNo == dNet::getMyNo()) {
        getFishingLandParam()->set(mpToolBank->mFloat.mPos[0].x, mpToolBank->mFloat.mPos[0].z);
    } else {
        if (mAnmId == HMN_ANM_POLE_GET1) {
            return;
        }
        mVec3_c pos(getFishingLandParam()->mFloatX, mPos.y, getFishingLandParam()->mFloatZ);
        dBGCF::groundChk_c ground(&pos, 0, 0, 0);
        mpToolBank->mFloat.setPos(pos.x, ground.getWaterY(), pos.z);
    }
    if (setAnm(HMN_ANM_POLE_GET1, m3d::FORWARD_ONCE)) {
        mpToolBank->setPoleGet1(m3d::FORWARD_ONCE, 8.0f);
    }
    startSound(SE_RodBack);
}

// .text 0xB2D0
void dAcPlayer_c::executeFishingLand() {
    if (mPlayerNo == dNet::getMyNo()) {
        dUki_c *uki = &mpToolBank->mFloat;
        if (uki->mState == dUki_c::STATE_HOLD) {
            dFishFldShadow_c *fish = uki->getFish();
            if (fish != NULL) {
                if (fish->isCaught()) {
                    setState(dPlayerMgr::STATE_FISHING_GET);
                }
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        }
    }
}

// .text 0xB350
void dAcPlayer_c::endFishingLand(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mPreviousMode;
        }
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xB3D8
void dAcPlayer_c::fishingLandParam_c::set(f32 x, f32 z) {
    mFloatX = x;
    mFloatZ = z;
}

// .text 0xB3E4
void dAcPlayer_c::initFishingGet() {
    static const int l_turnFrames = 30;

    if (setAnm(HMN_ANM_POLE_GET2, m3d::FORWARD_ONCE)) {
        mpToolBank->setPoleGet2(m3d::FORWARD_ONCE, 8.0f);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (dCamera_c::sInstance != NULL) {
            dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
        }
        mStep = 10;
        if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mCatalog.isComplete(dItem::KIND_FISH)) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE;
        } else {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE;
        }
        mActionItem = dItem::Item(dItem::ITEM_IDX_BITTERLING);
        dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
        if (fish != NULL) {
            dItem::Item item = fish->getItem();
            mActionItem = item;
            if (putInPocket(&item)) {
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
            } else {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
            }
            BOOL isTrash = FALSE;
            int kind = fish->mKind;
            if (kind >= FISH_NUM) {
                BOOL isKey = FALSE;
                if (kind >= 0x43 && kind < FISH_TYPE_NUM) {
                    isKey = TRUE;
                }
                if (!isKey) {
                    isTrash = TRUE;
                }
            }
            if (!isTrash) {
                dClock::addFishCount();
                dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->registerCatalogItem(item, TRUE);
                dActivityLog::onCatchFish(fish->mKind, mPlayerNo);
            }
        }
        _2188 = mActionItem.getId();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_INSECT;
    } else if (mState != dPlayerMgr::STATE_FISHING_LAND) {
        mpToolBank->mFloat.setState(dUki_c::STATE_HOLD);
    }
    mTargetAngle = 0;
    mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / l_turnFrames;
    if (mTurnFrames <= 0) {
        mTurnFrames = 1000;
    }
}

// .text 0xB5F8
void dAcPlayer_c::executeFishingGet() {
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.getFrame() > 41.0f) {
            stepGetFishMsg();
        }
    }
}

// .text 0xB660
void dAcPlayer_c::endFishingGet(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mShowPos = mpToolBank->mFloat.mTarget;
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop0(11, 0x1A);
        }
    }
    if (nextState != dPlayerMgr::STATE_CATCH_POSE) {
        if (mPlayerNo == dNet::getMyNo()) {
            dCamera_c *camera = dCamera_c::sInstance;
            if (camera != NULL) {
                camera->mRequestedMode = camera->mDefaultMode;
            }
        } else if (!isFishingState(nextState)) {
            mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
        }
    }
}

// .text 0xB738
void dAcPlayer_c::initFishingPutAway() {
    if (mPlayerNo == dNet::getMyNo()) {
        getFishingPutAwayParam()->set((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
        setAnm(HMN_ANM_POLE_PUTAWAY1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_POLE_PUTAWAY1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        if (mState != dPlayerMgr::STATE_FISHING_GET) {
            mpToolBank->mFloat.setState(dUki_c::STATE_HOLD);
        }
    }
    mpToolBank->setPolePutaway(m3d::FORWARD_ONCE, 8.0f);
}

// .text 0xB7F4
void dAcPlayer_c::executeFishingPutAway() {
    if (mAnmId != HMN_ANM_POLE_PUTAWAY1) {
        return;
    }
    dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
    if (fish != NULL) {
        if (mAnm.checkFrame(18.0f)) {
            mVec3_c scale(0.0f, 0.0f, 0.0f);
            fish->setHeldScale(&scale);
        } else if (mAnm.getFrame() < 18.0f) {
            f32 s = 1.0f - 0.065f * mAnm.getFrame();
            if (s < 0.0f) {
                s = 0.0f;
            }
            mVec3_c scale(s, s, 1.0f);
            fish->setHeldScale(&scale);
        }
    }
    if (mAnm.checkFrame(15.0f)) {
        startSound(SE_gasagoso_return);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
                setState(dPlayerMgr::STATE_POCKET_FULL);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        }
    } else if (mAnm.isStop()) {
        if (getFishingPutAwayParam()->mFull) {
            setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0xB990
void dAcPlayer_c::endFishingPutAway(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        deleteHookedFish();
    } else if (!isFishingState(nextState)) {
        mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
    }
}

// .text 0xBA08
void dAcPlayer_c::fishingPutAwayParam_c::set(u8 full) {
    mFull = full;
}

// .text 0xBA10
void dAcPlayer_c::initReturnHome() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (dSTR::getCurBuildingCode() == 0 ||
            (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) && dBGCF::isSlopeStep(&mPos))) {
            int anm = HMN_ANM_STING_DOWNA1;
            switch (mpToolBank->getToolType()) {
            case HMN_TOOL_NET:
                anm = HMN_ANM_STING_DOWNB1;
                mpToolBank->setNetAnm1B5(m3d::FORWARD_ONCE, 40.0f);
                break;
            case HMN_TOOL_ROD:
                anm = HMN_ANM_STING_DOWNB1;
                mpToolBank->setPoleStingDown(m3d::FORWARD_ONCE, 8.0f);
                break;
            case HMN_TOOL_UMBRELLA:
                anm = HMN_ANM_STING_DOWNB1;
                break;
            }
            setAnm(anm, m3d::FORWARD_ONCE);
            getReturnHomeParam()->set(2);
        } else {
            int type = dSTR::getCurBuildingType();
            if (type == 2) {
                setAnm(HMN_ANM_DOOR_OUT1, 0.0f, 0.0f, 0.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            } else {
                setAnmNoMorph(HMN_ANM_NODOOR_OUT_WAIT1, m3d::FORWARD_LOOP);
            }
            getReturnHomeParam()->set(type);
            mDoorType = type;
        }
    } else {
        mDoorType = getReturnHomeParam()->mDoorType;
        if (mDoorType == 2) {
            setAnm(HMN_ANM_DOOR_OUT1, 0.0f, 0.0f, 0.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        } else {
            setAnmNoMorph(HMN_ANM_NODOOR_OUT_WAIT1, m3d::FORWARD_LOOP);
        }
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC | dPlayerMgr::ACTOR_FLAG_DISABLED;
    }
}

// .text 0xBBC0
void dAcPlayer_c::executeReturnHome() {
    static const f32 l_exitFrame = 141.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        if ((mAnmId == HMN_ANM_STING_DOWNA1 || mAnmId == HMN_ANM_STING_DOWNB1) && mAnm.checkFrame(l_exitFrame)) {
            requestHomeScene();
        }
    }
}

// .text 0xBC28
void dAcPlayer_c::endReturnHome(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
        if (nextState != dPlayerMgr::STATE_EXIT_DOOR) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        }
    }
}

// .text 0xBC88
void dAcPlayer_c::returnHomeParam_c::set(u8 doorType) {
    mDoorType = doorType;
}

// .text 0xBC90
void dAcPlayer_c::initFtrGrab() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER) {
            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_FTR;
        }
        dFTR::hold_c *hold = dFTR::getHoldList()->get(0);
        if (hold != NULL && dFTR::holdList_c::isHoldScene()) {
            mTargetPos = hold->mPushPos;
            mTargetAngle = hold->mPushAngle;
        } else {
            mTargetPos = mPos;
            _2052 = mAngle.y;
        }
        setAnm(HMN_ANM_KAGU_HOLD1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        int pose = encodeSeatPose(mTargetAngle, 0);
        getFtrGrabParam()->set(mTargetPos.x, mTargetPos.z, pose);
    } else {
        mTargetPos.x = getFtrGrabParam()->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = getFtrGrabParam()->mZ;
        int side;
        mTargetAngle = decodeSeatPose(&side, getFtrGrabParam()->mPose);
        mVec3_c diff = mTargetPos - mPos;
        if (diff.x * diff.x + diff.z * diff.z < 100.0f) {
            setAnm(HMN_ANM_KAGU_HOLD1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        } else {
            setAnm(HMN_ANM_NML_WALK1, 10.0f, 0.0f, 0.8f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        }
    }
    sLib::chaseAngle(&mAngle.y.mAngle,
                     mAnmId == HMN_ANM_KAGU_HOLD1 ? mTargetAngle : dActor_c::targetAngleY(&mPos, &mTargetPos), 1000);
    cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    _22A4[4] = 0;
}

// .text 0xBEA0
void dAcPlayer_c::executeFtrGrab() {
    static const f32 l_grabFrame = 5.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_FTR_HOLD);
        } else if (mAnm.getFrame() >= l_grabFrame) {
            if (_22A4[4] != 0) {
                setAnm(HMN_ANM_NML_WAIT1, 12.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
                setState(dPlayerMgr::STATE_IDLE);
            } else if (_22FE[1] == 0) {
                dFTR::getHoldList()->startHold();
                _22A4[4] = 1;
            }
        }
    } else if (mAnmId == HMN_ANM_KAGU_HOLD1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            mAnm.mPlayMode = m3d::FORWARD_LOOP;
        }
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, dActor_c::targetAngleY(&mPos, &mTargetPos), 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 1.6f);
        mVec3_c diff = mTargetPos - mPos;
        f32 distSq = diff.x * diff.x + diff.z * diff.z;
        if (distSq < 100.0f) {
            setAnm(HMN_ANM_KAGU_HOLD1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        } else {
            if (distSq > 25600.0f) {
                mPos.x = mTargetPos.x;
                mPos.z = mTargetPos.z;
            }
            setAnm(HMN_ANM_NML_WALK1, 10.0f, 0.0f, 0.8f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        }
    }
}

// .text 0xC0B0
void dAcPlayer_c::endFtrGrab(int nextState) {
    _22A4[4] = 0;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
    _22DB = 0;
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_POINTER_FTR;
}

// .text 0xC0D8
void dAcPlayer_c::ftrGrabParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0xC0E8
void dAcPlayer_c::initFtrHold() {
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        if (mState == dPlayerMgr::STATE_FTR_PUSH || mState == dPlayerMgr::STATE_FTR_PULL) {
            if (!setAnm(HMN_ANM_KAGU_HOLD1, 45.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, TRUE)) {
                mAnm.mPlayMode = m3d::FORWARD_LOOP;
            }
        } else if (!setAnm(HMN_ANM_KAGU_HOLD1, m3d::FORWARD_LOOP)) {
            mAnm.mPlayMode = m3d::FORWARD_LOOP;
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mTargetPos.set(x, mPos.y, z);
            mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
        }
        mVec3_c diff = mTargetPos - mPos;
        if (diff.x * diff.x + diff.z * diff.z < 100.0f) {
            setAnm(HMN_ANM_KAGU_HOLD1, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, 0.8f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        }
    }
    mActorFlags2 &= ~(dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED | dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE);
    mStep = 0x15;
}

// .text 0xC268
void dAcPlayer_c::executeFtrHold() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mStep == 0x15) {
            if (_22FE[1] == 0 || (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE)) {
                setState(dPlayerMgr::STATE_IDLE);
                return;
            }
            if (dScene::getCurrentScene() == dScene::SCENE_RM_MM_CAFE && mObjc.isHit(dObjc::TYPE_ACTION)) {
                setState(dPlayerMgr::STATE_IDLE);
                return;
            }
            if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL) && isNoStickInput()) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE;
            }
            int dir = DIR_NONE;
            if (mSeatPose != DIR_NONE) {
                dir = mSeatPose;
            }
            if (!isNoStickInput()) {
                dir = getInputDirection();
            }
            if (dir == DIR_NONE) {
                return;
            }
            if (!dSTR::isAlone() && dScene::isCurrentSceneMyHouse()) {
                mStep = 10;
                return;
            }
            switch (dir) {
            case DIR_BACK:
                setState(dPlayerMgr::STATE_FTR_PULL_START);
                break;
            case DIR_FRONT:
                setState(dPlayerMgr::STATE_FTR_PUSH_START);
                break;
            case DIR_LEFT:
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL) {
                    dFTR::holdList_c *list = dFTR::getHoldList();
                    if (list->isIdle()) {
                        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                        mSeatPose = DIR_NONE;
                        if (list->canRotateLeft()) {
                            mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT;
                            setState(dPlayerMgr::STATE_FTR_ROTATE);
                        } else {
                            playSe(SE_CanNot);
                        }
                    } else {
                        if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED) && mAnm.getFrame() >= 2.0f &&
                            list->getFtr(0) == NULL) {
                            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED;
                        }
                        if ((mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED) &&
                            !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE)) {
                            playSe(SE_CanNot);
                            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE;
                            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                            mSeatPose = DIR_NONE;
                        }
                    }
                }
                break;
            case DIR_RIGHT:
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL) {
                    dFTR::holdList_c *list = dFTR::getHoldList();
                    if (list->isIdle()) {
                        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                        mSeatPose = DIR_NONE;
                        if (list->canRotateRight()) {
                            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT;
                            setState(dPlayerMgr::STATE_FTR_ROTATE);
                        } else {
                            playSe(SE_CanNot);
                        }
                    } else {
                        if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED) && mAnm.getFrame() >= 2.0f &&
                            list->getFtr(0) == NULL) {
                            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED;
                        }
                        if ((mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED) &&
                            !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE)) {
                            playSe(SE_CanNot);
                            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_FTR_BLOCKED_SE;
                            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                            mSeatPose = DIR_NONE;
                        }
                    }
                }
                break;
            }
        } else {
            stepPlayerMsg();
        }
    } else {
        mVec3_c diff = mTargetPos - mPos;
        f32 distSq = diff.x * diff.x + diff.z * diff.z;
        if (distSq < 100.0f) {
            followRemoteTransform();
            setAnm(HMN_ANM_KAGU_HOLD1, m3d::FORWARD_LOOP);
        } else {
            if (distSq > 25600.0f) {
                mPos.x = mTargetPos.x;
                mPos.z = mTargetPos.z;
            } else {
                sLib::chaseAngle(&mAngle.y.mAngle, dActor_c::targetAngleY(&mPos, &mTargetPos), 0xB61);
                cLib::chasePosXZ(&mPos, mTargetPos, 1.6f);
            }
            setAnm(HMN_ANM_NML_WALK1, 10.0f, 0.0f, 0.8f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        }
    }
}

// .text 0xC6A4
void dAcPlayer_c::endFtrHold(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
        mSeatPose = DIR_NONE;
        if (nextState != dPlayerMgr::STATE_FTR_ROTATE) {
            _21C0[1] = 0.0f;
        }
    }
    mStep = 0x22;
}

// .text 0xC718
BOOL dAcPlayer_c::isNoStickInput() {
    return isNoMoveInput();
}

// .text 0xC71C
void dAcPlayer_c::updateHoldDir() {
    static const f32 l_endFrame = 48.0f;

    if (mAnm.getFrame() >= l_endFrame && !isNoStickInput()) {
        mSeatPose = getInputDirection();
    }
}

// .text 0xC77C
BOOL dAcPlayer_c::isHoldDirChanged(int dir) {
    int cur = getInputDirection();
    if (isNoStickInput() || cur != dir) {
        return TRUE;
    }
    return FALSE;
}

// .text 0xC7E4
void dAcPlayer_c::initFtrRotate() {
    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::holdList_c *list = dFTR::getHoldList();
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT) {
            list->startRotateLeft();
            setAnm(HMN_ANM_KAGU_TURNR1, 16.0f, _21C0[1], 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        } else {
            list->startRotateRight();
            setAnm(HMN_ANM_KAGU_TURNL1, 16.0f, _21C0[1], 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        }
        getFtrRotateParam()->set((mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT) != 0);
    } else if (getFtrRotateParam()->mLeft) {
        setAnm(HMN_ANM_KAGU_TURNR1, 16.0f, _21C0[1], 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    } else {
        setAnm(HMN_ANM_KAGU_TURNL1, 16.0f, _21C0[1], 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    }
    _22DB = 0;
    _21C0[1] = 0.0f;
}

// .text 0xC90C
void dAcPlayer_c::executeFtrRotate() {
    static const f32 l_turnFrame = 8.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.getPlayMode() == m3d::REVERSE_ONCE) {
            if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL)) {
                if (isNoStickInput()) {
                    mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
                }
            } else if (!isNoStickInput()) {
                int dir = getInputDirection();
                dFTR::holdList_c *list = dFTR::getHoldList();
                if (dir == DIR_LEFT) {
                    _22DB = 1;
                } else if (dir == DIR_RIGHT) {
                    _22DB = 2;
                }
            }
            if (_22DB != 0 && mAnm.getFrame() <= l_turnFrame) {
                if (_22DB == 1 && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT)) {
                    _21C0[1] = mAnm.getFrame();
                } else if (_22DB == 2 && !(mActorFlags & dPlayerMgr::ACTOR_FLAG_FTR_ROTATE_LEFT)) {
                    _21C0[1] = mAnm.getFrame();
                }
                setState(dPlayerMgr::STATE_FTR_HOLD);
            } else if (mAnm.isStop()) {
                _22DB = 0;
                setState(dPlayerMgr::STATE_FTR_HOLD);
            }
        } else if (mAnm.getPlayMode() == m3d::FORWARD_ONCE) {
            if (mAnm.isStop()) {
                mAnm.mPlayMode = m3d::REVERSE_ONCE;
            }
        }
    } else if (mAnm.getPlayMode() == m3d::REVERSE_ONCE) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_KAGU_HOLD1, m3d::FORWARD_LOOP);
        }
    } else if (mAnm.getPlayMode() == m3d::FORWARD_ONCE) {
        if (mAnm.isStop()) {
            mAnm.mPlayMode = m3d::REVERSE_ONCE;
        }
    }
}

// .text 0xCAE4
void dAcPlayer_c::endFtrRotate(int nextState) {}

// .text 0xCAE8
void dAcPlayer_c::ftrRotateParam_c::set(u8 left) {
    mLeft = left;
}

// .text 0xCAF0
void dAcPlayer_c::initFtrPushStart() {
    setAnm(HMN_ANM_KAGU_PUSH1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0xCB18
void dAcPlayer_c::executeFtrPushStart() {
    static const f32 l_startFrame = 11.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::holdList_c *list = dFTR::getHoldList();
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.isStop()) {
            if (isHoldDirChanged(DIR_FRONT)) {
                setState(dPlayerMgr::STATE_FTR_HOLD);
            }
        } else if (mAnm.checkFrame(l_startFrame)) {
            if (list->canPush()) {
                setState(dPlayerMgr::STATE_FTR_PUSH);
            } else {
                playSe(SE_CanNot);
            }
        } else if (isHoldDirChanged(DIR_FRONT)) {
            setState(dPlayerMgr::STATE_FTR_HOLD);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
        }
    }
}

// .text 0xCC28
void dAcPlayer_c::endFtrPushStart(int nextState) {}

// .text 0xCC2C
void dAcPlayer_c::initFtrPush() {
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        getFrontPos(&mTargetPos);
        _22A4[4] = 0;
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mVec3_c pos(x, mPos.y, z);
            calcPosAtAngle(&mTargetPos, &pos, angle, 32.0f);
        } else {
            getFrontPos(&mTargetPos);
        }
    }
    setAnm(HMN_ANM_KAGU_PUSH2, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0xCD14
void dAcPlayer_c::executeFtrPush() {
    if (mPlayerNo == dNet::getMyNo()) {
        updateHoldDir();
        if (_22A4[4] == 0) {
            dFTR::getHoldList()->startPush();
            _22A4[4] = 1;
        }
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_FTR_HOLD);
        }
    } else if (mAnm.isStop()) {
        setAnm(HMN_ANM_KAGU_HOLD1, 45.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        mPos.set(mTargetPos.x, mTargetPos.y, mTargetPos.z);
    }
}

// .text 0xCDEC
void dAcPlayer_c::endFtrPush(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mPos.set(mTargetPos.x, mTargetPos.y, mTargetPos.z);
        _22A4[4] = 0;
        if (mSeatPose != DIR_NONE) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
        }
    } else {
        mPos.set(mTargetPos.x, mTargetPos.y, mTargetPos.z);
    }
}

// .text 0xCE78
void dAcPlayer_c::initFtrPullStart() {
    setAnm(HMN_ANM_KAGU_PULL1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0xCEA0
void dAcPlayer_c::executeFtrPullStart() {
    static const f32 l_startFrame = 11.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::holdList_c *list = dFTR::getHoldList();
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_STOP_FOR_SAVE) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.isStop()) {
            if (isHoldDirChanged(DIR_BACK)) {
                setState(dPlayerMgr::STATE_FTR_HOLD);
            }
        } else if (mAnm.checkFrame(l_startFrame)) {
            if (list->canPull()) {
                setState(dPlayerMgr::STATE_FTR_PULL);
            } else {
                playSe(SE_CanNot);
            }
        } else if (isHoldDirChanged(DIR_BACK)) {
            setState(dPlayerMgr::STATE_FTR_HOLD);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
        }
    }
}

// .text 0xCFB0
void dAcPlayer_c::endFtrPullStart(int nextState) {}

// .text 0xCFB4
void dAcPlayer_c::initFtrPull() {
    if (mPlayerNo == dNet::getMyNo()) {
        _2098 = mPos;
        getPosAtAngle(&mTargetPos, mAngle.y, -32.0f);
        _22A4[4] = 0;
    }
    setAnm(HMN_ANM_KAGU_PULL2, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0xD054
void dAcPlayer_c::executeFtrPull() {
    if (mPlayerNo == dNet::getMyNo()) {
        updateHoldDir();
        if (_22A4[4] == 0) {
            dFTR::getHoldList()->startPull();
            _22A4[4] = 1;
        }
        _2098 = mPos;
        mPos.x = mMdlCallback.mSoundPos.x;
        mPos.z = mMdlCallback.mSoundPos.z;
        mLastPos = mPos;
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_FTR_HOLD);
        }
    } else if (mAnmId != HMN_ANM_KAGU_HOLD1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_KAGU_HOLD1, 45.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
            mPos = mMdlCallback.mSoundPos;
        }
    }
}

// .text 0xD174
void dAcPlayer_c::endFtrPull(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        if ((u32)((int)mTargetPos.z >> 5) <= 13) {
            mPos.set(mTargetPos);
        }
        _22A4[4] = 0;
        if (mSeatPose != DIR_NONE) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_FTR_STICK_NEUTRAL;
        }
        mLastPos = mPos;
    } else if (mAnmId != HMN_ANM_KAGU_HOLD1) {
        mPos = mMdlCallback.mSoundPos;
    }
}

// .text 0xD240
void dAcPlayer_c::initLieDown() {
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos;
        calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
        dFTR::requestPull(&pos);
        mVec3_c base = mTargetPos;
        calcPosAtAngle(&mTargetPos, &base, mTargetAngle + 0x8000, 1.0f);
    }
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
}

// .text 0xD2EC
void dAcPlayer_c::executeLieDown() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (dFTR::getPullState()) {
        case dFTR::HOLD_STATE_PENDING:
            return;
        case dFTR::HOLD_STATE_REJECTED:
            setState(dPlayerMgr::STATE_IDLE);
            break;
        case dFTR::HOLD_STATE_ACCEPTED:
        default:
            switch (mSeatSide) {
            case 1:
            case 2:
                setState(dPlayerMgr::STATE_GET_INTO_BED);
                break;
            }
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xD390
void dAcPlayer_c::endLieDown(int nextState) {}

// .text 0xD394
void dAcPlayer_c::initGetIntoBed() {
    if (mPlayerNo == dNet::getMyNo()) {
        int pose = encodeSeatPose(mTargetAngle, mSeatSide);
        getGetIntoBedParam()->set(mTargetPos.x, mTargetPos.z, pose);
    } else {
        mTargetPos.x = getGetIntoBedParam()->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = getGetIntoBedParam()->mZ;
        mTargetAngle = decodeSeatPose(&mSeatSide, getGetIntoBedParam()->mPose);
    }
    if (mSeatSide == 2) {
        setAnm(HMN_ANM_BED_SLEEPL1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    } else {
        setAnm(HMN_ANM_BED_SLEEPR1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    }
    mVec3_c diff = mTargetPos - mPos;
    _21D0[0] = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / 36.0f;
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
    cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
    startSound(SE_jump);
}

// .text 0xD4EC
void dAcPlayer_c::executeGetIntoBed() {
    sLib::addCalc2(&mShadowAlpha, 0.0f, 0.06f, 0.1f);
    if (mAnmId != HMN_ANM_BED_SLEEP_WAIT1 && mAnm.checkFrame(mAnm.mFrameMax - 38.0f)) {
        mVec3_c pos;
        calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
        dFtrm_c::turnOnSeatSound(&pos);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_BED_WAIT);
        }
    } else if (mAnmId != HMN_ANM_BED_SLEEP_WAIT1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP);
            setBedPos();
        }
    }
}

// .text 0xD624
void dAcPlayer_c::endGetIntoBed(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        setBedPos();
    } else if (setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP)) {
        setBedPos();
    }
}

// .text 0xD688
void dAcPlayer_c::setBedPos() {
    mVec3_c pos;
    calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
    mPos.set(pos.x, pos.y, pos.z);
    mAngle.y = mTargetAngle;
    if (mSeatSide == 2) {
        mAngle.y = mTargetAngle - 0x4000;
    } else if (mSeatSide == 1) {
        mAngle.y = mTargetAngle + 0x4000;
    }
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0xD720
void dAcPlayer_c::getIntoBedParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0xD730
void dAcPlayer_c::initGetUp() {
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos(mPos.x, mPos.y, mPos.z);
        switch (mSeatPose) {
        case DIR_LEFT:
            calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle + 0x4000, 32.0f);
            break;
        case DIR_RIGHT:
            calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle - 0x4000, 32.0f);
            break;
        }
        dFTR::requestRotate(&pos);
    }
}

// .text 0xD7FC
void dAcPlayer_c::executeGetUp() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (dFTR::getRotateState()) {
        case dFTR::HOLD_STATE_PENDING:
            return;
        case dFTR::HOLD_STATE_REJECTED:
            setState(dPlayerMgr::STATE_BED_WAIT);
            break;
        case dFTR::HOLD_STATE_ACCEPTED:
        default:
            switch (mSeatPose) {
            case DIR_LEFT:
            case DIR_RIGHT:
                setState(dPlayerMgr::STATE_GET_OUT_OF_BED);
                break;
            }
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xD8AC
void dAcPlayer_c::endGetUp(int nextState) {}

// .text 0xD8B0
void dAcPlayer_c::initGetOutOfBed() {
    dFtrm_c::turnOffSeatSound(&mPos);
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        getGetOutOfBedParam()->set(mSeatPose == DIR_RIGHT);
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mTargetPos.set(x, mPos.y, z);
            mTargetAngle = angle;
        }
        if (getGetOutOfBedParam()->mRight) {
            mSeatPose = DIR_RIGHT;
        } else {
            mSeatPose = DIR_LEFT;
        }
    }
    if (mSeatPose == DIR_LEFT) {
        setAnmNoMorph(HMN_ANM_BED_WAKEUPR1, m3d::FORWARD_ONCE);
    } else {
        setAnmNoMorph(HMN_ANM_BED_WAKEUPL1, m3d::FORWARD_ONCE);
    }
    mObjcPos.set(this, mMdlCallback.mBasePos, 16.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
}

// .text 0xD9D4
void dAcPlayer_c::executeGetOutOfBed() {
    sLib::addCalc2(&mShadowScale, 1.0f, 0.12f, 0.1f);
    sLib::addCalc2(&mShadowAlpha, 1.0f, 0.08f, 0.1f);
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(mAnm.mFrameMax - 20.0f)) {
            startSound(SE_landing);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setGetOutOfBedPos();
        } else if (mAnm.checkFrame(mAnm.mFrameMax - 20.0f)) {
            startSound(SE_landing);
        }
    }
    mObjcPos.mPos = mMdlCallback.mBasePos;
    mObjcPos.regist();
}

// .text 0xDB40
void dAcPlayer_c::endGetOutOfBed(int nextState) {
    mObjcPos.mType &= ~0x20;
    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::releaseRotate();
        setGetOutOfBedPos();
    } else if (setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP)) {
        setGetOutOfBedPos();
    }
}

// .text 0xDBB4
void dAcPlayer_c::setGetOutOfBedPos() {
    if (mSeatPose == DIR_LEFT) {
        mAngle.y += 0x4000;
    } else if (mSeatPose == DIR_RIGHT) {
        mAngle.y -= 0x4000;
    }
    mVec3_c *pos = &mMdlCallback.mSoundPos;
    if (pos != NULL) {
        mPos.set(pos->x, pos->y, pos->z);
    }
    mShadowScale = 1.0f;
    mShadowAlpha = 1.0f;
}

// .text 0xDC18
void dAcPlayer_c::getOutOfBedParam_c::set(BOOL right) {
    mRight = right;
}

// .text 0xDC20
void dAcPlayer_c::initBedWait() {
    setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP);
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0xDC64
void dAcPlayer_c::executeBedWait() {
    dFtrm_c::updateSeatSound(&mPos);
    if (mPlayerNo == dNet::getMyNo()) {
        int result = 0;
        int dir = getInputDirectionOrNone();
        switch (dir) {
        case DIR_LEFT:
            result = dFtrm_c::getSeatPosRight(&mPos, mAngle.y, NULL);
            break;
        case DIR_RIGHT:
            result = dFtrm_c::getSeatPosLeft(&mPos, mAngle.y, NULL);
            break;
        }
        if (result == 1) {
            mSeatPose = dir;
            setState(dPlayerMgr::STATE_BED_ROLL);
        } else if (result == 2) {
            mSeatPose = dir;
            setState(dPlayerMgr::STATE_GET_UP);
        }
    } else if (dNet::isSessionActive()) {
        followRemoteTransform();
    } else if (mAnm.checkFrame(65.0f)) {
        startSound(SE_neiki);
    }
}

// .text 0xDD98
void dAcPlayer_c::endBedWait(int nextState) {
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
}

// .text 0xDDA8
void dAcPlayer_c::initBedRoll() {
    if (mPlayerNo == dNet::getMyNo()) {
        getBedRollParam()->set(mSeatPose == DIR_RIGHT);
    } else if (getBedRollParam()->mRight) {
        mSeatPose = DIR_RIGHT;
    } else {
        mSeatPose = DIR_LEFT;
    }
    mVec3_c pos;
    if (mSeatPose == DIR_LEFT) {
        setAnm(HMN_ANM_BED_TURNLR1, m3d::FORWARD_LOOP);
        calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle + 0x4000, 32.0f);
    } else {
        setAnm(HMN_ANM_BED_TURNRL1, m3d::FORWARD_LOOP);
        calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle - 0x4000, 32.0f);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::requestMove(&pos);
    }
}

// .text 0xDEAC
void dAcPlayer_c::executeBedRoll() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (dFTR::getMoveState()) {
        case dFTR::HOLD_STATE_PENDING:
            return;
        case dFTR::HOLD_STATE_ACCEPTED:
            setState(dPlayerMgr::STATE_BED_ROLL_OVER);
            break;
        case dFTR::HOLD_STATE_REJECTED:
            setState(dPlayerMgr::STATE_BED_ROLL_BLOCKED);
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xDF38
void dAcPlayer_c::endBedRoll(int nextState) {}

// .text 0xDF3C
void dAcPlayer_c::bedRollParam_c::set(BOOL right) {
    mRight = right;
}

// .text 0xDF44
void dAcPlayer_c::initBedRollOver() {
    startSound(SE_bed_negaeri);
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        getBedRollOverParam()->set(mSeatPose == DIR_RIGHT);
        if (mSeatPose == DIR_LEFT) {
            setAnm(HMN_ANM_BED_TURNLR2, m3d::FORWARD_ONCE);
        } else {
            setAnm(HMN_ANM_BED_TURNRL2, m3d::FORWARD_ONCE);
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            _205C.set(x, mPos.y, z);
            mTargetAngle = angle;
        } else {
            _205C = mPos;
            mTargetAngle = mAngle.y;
        }
        if (getBedRollOverParam()->mRight) {
            mSeatPose = DIR_RIGHT;
        } else {
            mSeatPose = DIR_LEFT;
        }
        if (mSeatPose == DIR_LEFT) {
            setAnmNoMorph(HMN_ANM_BED_TURNLR2, m3d::FORWARD_ONCE);
        } else {
            setAnmNoMorph(HMN_ANM_BED_TURNRL2, m3d::FORWARD_ONCE);
        }
    }
}

// .text 0xE088
void dAcPlayer_c::executeBedRollOver() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_BED_WAIT);
        }
    } else if (mAnmId != HMN_ANM_BED_SLEEP_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop() && setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP)) {
            setBedRollPos();
        }
    }
}

// .text 0xE124
void dAcPlayer_c::endBedRollOver(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        setBedRollPos();
    } else if (setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP)) {
        setBedRollPos();
    }
}

// .text 0xE188
void dAcPlayer_c::setBedRollPos() {
    s16 rollAngle = 0;
    s16 angle = mAngle.y;
    mVec3_c base = mPos;
    if (mPlayerNo != dNet::getMyNo()) {
        angle = mTargetAngle;
        base = _205C;
    }
    if (mSeatPose == DIR_LEFT) {
        rollAngle = angle + 0x4000;
    } else if (mSeatPose == DIR_RIGHT) {
        rollAngle = angle - 0x4000;
    }
    mVec3_c pos;
    calcPosAtAngle(&pos, &base, rollAngle, 32.0f);
    mPos.set(pos.x, pos.y, pos.z);
}

// .text 0xE268
void dAcPlayer_c::bedRollOverParam_c::set(BOOL right) {
    mRight = right;
}

// .text 0xE270
void dAcPlayer_c::initBedRollBlocked() {
    BOOL playSound = TRUE;
    f32 frame = 0.0f;
    for (int i = 0; i < 4; i++) {
        if (mPlayerNo != i && dPlayerMgr::getActionAnimationFrame(&frame, i) && frame < 15.0f) {
            playSound = FALSE;
            break;
        }
    }
    if (playSound) {
        startSound(SE_bed_hit);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        getBedRollBlockedParam()->set(mSeatPose == DIR_RIGHT);
    } else if (getBedRollBlockedParam()->mRight) {
        mSeatPose = DIR_RIGHT;
    } else {
        mSeatPose = DIR_LEFT;
    }
    if (mSeatPose == DIR_LEFT) {
        setAnm(HMN_ANM_BED_TURNLR3, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_BED_TURNRL3, m3d::FORWARD_ONCE);
    }
}

// .text 0xE3A4
void dAcPlayer_c::executeBedRollBlocked() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_BED_WAIT);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_BED_SLEEP_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0xE424
void dAcPlayer_c::endBedRollBlocked(int nextState) {}

// .text 0xE428
void dAcPlayer_c::bedRollBlockedParam_c::set(BOOL right) {
    mRight = right;
}

// .text 0xE430
void dAcPlayer_c::initSitDown() {
    if (mAnmId != HMN_ANM_NML_WALK1 && mAnmId != HMN_ANM_NML_RUN1) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c base = mTargetPos;
        calcPosAtAngle(&mTargetPos, &base, mTargetAngle + 0x8000, 1.0f);
        mVec3_c pos;
        calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
        dFTR::requestHold(&pos);
    }
}

// .text 0xE4EC
void dAcPlayer_c::executeSitDown() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (dFTR::getHoldState()) {
        case dFTR::HOLD_STATE_PENDING:
            return;
        case dFTR::HOLD_STATE_REJECTED:
            setState(dPlayerMgr::STATE_IDLE);
            break;
        case dFTR::HOLD_STATE_ACCEPTED:
        default:
            switch (mSeatSide) {
            case 0:
                setState(dPlayerMgr::STATE_SIT_DOWN_FRONT);
                break;
            case 2:
                setState(dPlayerMgr::STATE_SIT_DOWN_SIDE);
                break;
            case 1:
                setState(dPlayerMgr::STATE_SIT_DOWN_SIDE);
                break;
            }
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xE5C0
void dAcPlayer_c::endSitDown(int nextState) {}

// .text 0xE5C4
void dAcPlayer_c::initSitDownFront() {
    if (mPlayerNo == dNet::getMyNo()) {
        int pose = encodeSeatPose(mTargetAngle, mSeatSide);
        getSitDownFrontParam()->set(mTargetPos.x, mTargetPos.z, pose);
    } else {
        mTargetPos.x = getSitDownFrontParam()->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = getSitDownFrontParam()->mZ;
        mTargetAngle = decodeSeatPose(&mSeatSide, getSitDownFrontParam()->mPose);
    }
    cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
    setAnm(HMN_ANM_CHAIR_SITDOWNF1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    startSound(SE_jump);
}

// .text 0xE6B4
void dAcPlayer_c::executeSitDownFront() {
    sLib::addCalc2(&mShadowAlpha, 0.0f, 0.06f, 0.1f);
    if (mAnmId != HMN_ANM_CHAIR_SITDOWN_WAIT1 && mAnm.checkFrame(mAnm.mFrameMax - 18.0f)) {
        mVec3_c pos;
        calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
        dFtrm_c::turnOnSeatSound(&pos);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_CHAIR_WAIT);
        }
    } else if (mAnmId != HMN_ANM_CHAIR_SITDOWN_WAIT1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
            setChairFrontPos();
        }
    }
}

// .text 0xE7EC
void dAcPlayer_c::endSitDownFront(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        setChairFrontPos();
    } else if (setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP)) {
        setChairFrontPos();
    }
}

// .text 0xE850
void dAcPlayer_c::setChairFrontPos() {
    mVec3_c pos;
    calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
    mPos.set(pos.x, pos.y, pos.z);
    mAngle.y = mTargetAngle + 0x8000;
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0xE8C8
void dAcPlayer_c::sitDownFrontParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0xE8D8
void dAcPlayer_c::initSitDownSide() {
    if (mPlayerNo == dNet::getMyNo()) {
        int pose = encodeSeatPose(mTargetAngle, mSeatSide);
        getSitDownSideParam()->set(mTargetPos.x, mTargetPos.z, pose);
    } else {
        mTargetPos.x = getSitDownSideParam()->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = getSitDownSideParam()->mZ;
        mTargetAngle = decodeSeatPose(&mSeatSide, getSitDownSideParam()->mPose);
    }
    if (mSeatSide == 2) {
        setAnm(HMN_ANM_CHAIR_SITDOWNR1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    } else {
        setAnm(HMN_ANM_CHAIR_SITDOWNL1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
    cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    startSound(SE_jump);
}

// .text 0xE9FC
void dAcPlayer_c::executeSitDownSide() {
    sLib::addCalc2(&mShadowAlpha, 0.0f, 0.06f, 0.1f);
    if (mAnmId != HMN_ANM_CHAIR_SITDOWN_WAIT1 && mAnm.checkFrame(mAnm.mFrameMax - 18.0f)) {
        mVec3_c pos;
        calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
        dFtrm_c::turnOnSeatSound(&pos);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_CHAIR_WAIT);
        }
    } else if (mAnmId != HMN_ANM_CHAIR_SITDOWN_WAIT1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
            setChairSidePos();
        }
    }
}

// .text 0xEB34
void dAcPlayer_c::endSitDownSide(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        setChairSidePos();
    } else if (setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP)) {
        setChairSidePos();
    }
}

// .text 0xEB98
void dAcPlayer_c::setChairSidePos() {
    mVec3_c pos;
    calcPosAtAngle(&pos, &mTargetPos, mTargetAngle, 32.0f);
    mPos.set(pos.x, pos.y, pos.z);
    mAngle.y = mTargetAngle;
    if (mSeatSide == 2) {
        mAngle.y = mTargetAngle + 0x4000;
    } else if (mSeatSide == 1) {
        mAngle.y = mTargetAngle - 0x4000;
    }
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0xEC30
void dAcPlayer_c::sitDownSideParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0xEC40
void dAcPlayer_c::initChairWait() {
    setAnmNoMorph(HMN_ANM_CHAIR_SITDOWN_WAIT1, m3d::FORWARD_LOOP);
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0xEC84
void dAcPlayer_c::executeChairWait() {
    dFtrm_c::updateSeatSound(&mPos);
    if (mPlayerNo == dNet::getMyNo()) {
        BOOL ok = FALSE;
        int dir = getInputDirectionOrNone();
        switch (dir) {
        case DIR_FRONT:
            ok = dFtrm_c::canSit(&mPos, mAngle.y);
            break;
        case DIR_LEFT:
            ok = dFtrm_c::canStandLeft(&mPos, mAngle.y);
            break;
        case DIR_RIGHT:
            ok = dFtrm_c::canStandRight(&mPos, mAngle.y);
            break;
        }
        if (ok) {
            mSeatPose = dir;
            setState(dPlayerMgr::STATE_CHAIR_POSE);
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xED80
void dAcPlayer_c::endChairWait(int nextState) {
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION;
}

// .text 0xED90
void dAcPlayer_c::initChairPose() {
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos(mPos.x, mPos.y, mPos.z);
        switch (mSeatPose) {
        case DIR_FRONT:
            getFrontPos(&pos);
            break;
        case DIR_LEFT:
            calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle + 0x4000, 32.0f);
            break;
        case DIR_BACK:
            break;
        case DIR_RIGHT:
            calcPosAtAngle(&pos, &mPos, mAngle.y.mAngle - 0x4000, 32.0f);
            break;
        }
        dFTR::requestPush(&pos);
    }
}

// .text 0xEE70
void dAcPlayer_c::executeChairPose() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (dFTR::getPushState()) {
        case 0:
            return;
        case 2:
            setState(dPlayerMgr::STATE_CHAIR_WAIT);
            break;
        case 1:
        default:
            switch (mSeatPose) {
            case DIR_FRONT:
                setState(dPlayerMgr::STATE_CHAIR_STAND_UP);
                break;
            case DIR_LEFT:
                setState(dPlayerMgr::STATE_CHAIR_FACE_FRONT);
                break;
            case DIR_BACK:
                break;
            case DIR_RIGHT:
                setState(dPlayerMgr::STATE_CHAIR_FACE_FRONT);
                break;
            }
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0xEF48
void dAcPlayer_c::endChairPose(int nextState) {}

// .text 0xEF4C
void dAcPlayer_c::initChairStandUp() {
    int playerNo = mPlayerNo;
    if (playerNo == dNet::getMyNo()) {
        getFrontPos(&_205C);
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, playerNo)) {
            mVec3_c pos(x, mPos.y, z);
            calcPosAtAngle(&_205C, &pos, angle, 32.0f);
        } else {
            getFrontPos(&_205C);
        }
    }
    setAnmNoMorph(HMN_ANM_CHAIR_STANDUPF1, m3d::FORWARD_ONCE);
    mObjcPos.set(this, mMdlCallback.mBasePos, 16.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
}

// .text 0xF040
void dAcPlayer_c::executeChairStandUp() {
    sLib::addCalc2(&mShadowScale, 1.0f, 0.12f, 0.1f);
    sLib::addCalc2(&mShadowAlpha, 1.0f, 0.08f, 0.1f);
    if (mAnmId == HMN_ANM_CHAIR_STANDUPF1) {
        if (mAnm.checkFrame(1.0f)) {
            dFtrm_c::turnOffSeatSound(&mPos);
        } else if (mAnm.checkFrame(mAnm.mFrameMax - 20.0f)) {
            startSound(SE_landing);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setStandUpPos();
        }
    }
    mObjcPos.mPos = mMdlCallback.mBasePos;
    mObjcPos.regist();
}

// .text 0xF190
void dAcPlayer_c::endChairStandUp(int nextState) {
    mObjcPos.mType &= ~0x20;
    if (mPlayerNo == dNet::getMyNo()) {
        dFTR::releaseHold();
        setStandUpPos();
    } else if (setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP)) {
        setStandUpPos();
    }
}

// .text 0xF204
void dAcPlayer_c::setStandUpPos() {
    mPos = _205C;
    mShadowScale = 1.0f;
    mShadowAlpha = 1.0f;
}

// .text 0xF230
void dAcPlayer_c::initChairFaceFront() {
    int playerNo = mPlayerNo;
    if (playerNo == dNet::getMyNo()) {
        dFdBase_c::snapToUnitCenter(&mTargetPos, &mPos);
        setChairFaceFrontParam((chairFaceFrontParam_s *)&mRemoteActionParams, mSeatPose == DIR_RIGHT);
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, playerNo)) {
            mVec3_c pos(x, mPos.y, z);
            dFdBase_c::snapToUnitCenter(&mTargetPos, &pos);
            mTargetAngle = angle;
        }
        if (((chairFaceFrontParam_s *)&mRemoteActionParams)->mFromLeft != 0) {
            mSeatPose = DIR_RIGHT;
        } else {
            mSeatPose = DIR_LEFT;
        }
    }
    if (mSeatPose == DIR_LEFT) {
        setAnmNoMorph(HMN_ANM_CHAIR_STANDUPR1, m3d::FORWARD_ONCE);
    } else {
        setAnmNoMorph(HMN_ANM_CHAIR_STANDUPL1, m3d::FORWARD_ONCE);
    }
    cLib::chasePosXZ(&mPos, mTargetPos, 0.6f);
}

// .text 0xF344
void dAcPlayer_c::executeChairFaceFront() {
    if (mAnm.checkFrame(1.0f)) {
        startSound(SE_Yokomuku);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        cLib::chasePosXZ(&mPos, mTargetPos, 0.6f);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_CHAIR_STAND_UP);
        }
    } else {
        cLib::chasePosXZ(&mPos, mTargetPos, 0.6f);
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        mAnm.isStop();
    }
}

// .text 0xF400
void dAcPlayer_c::endChairFaceFront(int nextState) {
    if (mSeatPose == DIR_LEFT) {
        mAngle.y.mAngle += 0x4000;
    } else if (mSeatPose == DIR_RIGHT) {
        mAngle.y.mAngle -= 0x4000;
    }
}

// .text 0xF434
void dAcPlayer_c::setChairFaceFrontParam(chairFaceFrontParam_s *param, BOOL fromLeft) {
    param->mFromLeft = fromLeft;
}

// Unreferenced folded static retained in the original REL; owner stripped.
void dAPlayer_retainToolConstant() {
    static const int l_unknown2798 = 13;
}

// .text 0xF43C
void dAcPlayer_c::initTakeOutTool() {
    setAnm(HMN_ANM_TOOL_PUTAWAY1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, TRUE);
    mAnm.setFrame(mAnm.mFrameMax - 1.0f);
}

// .text 0xF4A8
void dAcPlayer_c::executeTakeOutTool() {
    static const f32 l_scaleRate = 0.08f;
    static const f32 l_soundFrame = 36.0f;

    if (mAnmId == HMN_ANM_TOOL_PUTAWAY1) {
        if (mAnm.checkFrame(18.0f)) {
            f32 scale = l_scaleRate;
            if (scale > 1.0f) {
                scale = 1.0f;
            }
            mToolScale.set(scale, scale, scale);
            setAnm(HMN_ANM_NML_WAIT1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        } else if (mAnm.checkFrame(l_soundFrame)) {
            startSound(SE_gasagoso);
        }
    } else if (mAnmId == HMN_ANM_NML_WAIT1) {
        f32 scale = l_scaleRate * mAnm.getFrame();
        if (scale > 1.0f) {
            scale = 1.0f;
        }
        if (mToolScale.x < 1.0f) {
            mToolScale.set(scale, scale, scale);
        }
        if (mAnm.checkFrame(12.0f) && scale < 1.0f) {
            createHeldToolGlitter();
        }
        if (mAnm.checkFrame(18.0f)) {
            if (mPlayerNo == dNet::getMyNo()) {
                setState(dPlayerMgr::STATE_IDLE);
            } else {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            }
        }
    }
}

// .text 0xF62C
void dAcPlayer_c::endTakeOutTool(int nextState) {
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
    mToolScale.set(1.0f, 1.0f, 1.0f);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
}

// .text 0xF654
void dAcPlayer_c::initPutAwayTool() {
    setAnm(HMN_ANM_TOOL_PUTAWAY1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        l_talkActorID = (fBaseID_e)-1;
        dDemoActor_c *talker = dDemo::getTalkActor();
        if (talker != NULL) {
            l_talkActorID = talker->mUniqueID;
        }
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0xF6E4
void dAcPlayer_c::executePutAwayTool() {
    if (mAnmId == HMN_ANM_TOOL_PUTAWAY1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_gasagoso_return);
        } else if (mAnm.checkFrame(18.0f)) {
            mToolScale.set(0.0f, 0.0f, 0.0f);
        } else if (mAnm.getFrame() < 18.0f) {
            if (mToolScale.x > 0.0f) {
                f32 scale = 1.0f - 0.065f * mAnm.getFrame();
                if (scale < 0.0f) {
                    scale = 0.0f;
                }
                mToolScale.set(scale, scale, scale);
            }
        } else if (mAnm.isStop()) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            setAnm(HMN_ANM_NML_WAIT1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
            if (mPlayerNo == dNet::getMyNo()) {
                if (dDemo::isType(dDemo::TYPE_TALK) || dDemo::isType(dDemo::TYPE_SPEAK)) {
                    setState(dPlayerMgr::STATE_DEMO_WAIT);
                }
            } else {
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            }
        }
    }
}

// .text 0xF85C
void dAcPlayer_c::endPutAwayTool(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mToolScale.set(0.0f, 0.0f, 0.0f);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
        l_talkActorID = (fBaseID_e)-1;
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        mToolScale.set(0.0f, 0.0f, 0.0f);
    }
}

// .text 0xF8F0
void dAcPlayer_c::initShakeTreeEnd() {
    if (mPlayerNo == dNet::getMyNo()) {
        setShakeTreeEndParam((unitParam_s *)&mRemoteActionParams, mTargetUnitX, mTargetUnitZ);
    } else {
        unitParam_s *param = (unitParam_s *)&mRemoteActionParams;
        mTargetUnitZ = param->mUnitZ;
        mTargetUnitX = param->mUnitX;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    if (!setAnm(HMN_ANM_TREE_SHAKE_WAIT1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE)) {
        mAnm.mPlayMode = m3d::FORWARD_ONCE;
    }
}

// .text 0xF9B0
void dAcPlayer_c::executeShakeTreeEnd() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnmId == HMN_ANM_TREE_SHAKE_WAIT1) {
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_TREE_SHAKE1, m3d::FORWARD_ONCE);
                dFgobjManager::startShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ, 3);
            }
        } else if (mAnmId == HMN_ANM_TREE_SHAKE1) {
            if (mAnm.isStop()) {
                if (mTaskKind == FG_MNG_KIND_SHAKE_TREE_SPECIAL) {
                    setState(dPlayerMgr::STATE_SURPRISED);
                } else {
                    setState(dPlayerMgr::STATE_IDLE);
                }
            }
        } else {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_TREE_SHAKE_WAIT1) {
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_TREE_SHAKE1, m3d::FORWARD_ONCE);
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                    dFgobjManager::startShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ, 3);
                    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
                }
            }
        } else if (mAnmId == HMN_ANM_TREE_SHAKE1) {
            if (mAnm.isStop()) {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            }
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0xFB2C
void dAcPlayer_c::endShakeTreeEnd(int nextState) {
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        dRumble::stop();
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        dFgobjManager::startShakeTree(playerNo, mTargetUnitX, mTargetUnitZ, 3);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0xFBA4
void dAcPlayer_c::setShakeTreeEndParam(unitParam_s *param, u8 unitX, u8 unitZ) {
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
}

// .text 0xFBB0
void dAcPlayer_c::initShakeTree() {
    if (mPlayerNo == dNet::getMyNo()) {
        setShakeTreeParam((unitParam_s *)&mRemoteActionParams, mTargetUnitX, mTargetUnitZ);
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
        mTargetAngle = dActor_c::targetAngleY(&mPos, &pos);
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        startRumble(dRumble::PATTERN_SHAKE_TREE);
    } else {
        unitParam_s *param = (unitParam_s *)&mRemoteActionParams;
        mTargetUnitZ = param->mUnitZ;
        mTargetUnitX = param->mUnitX;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING | dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0xFC64
void dAcPlayer_c::executeShakeTree() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (_229C != -1) {
            waitFieldTask(0, sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61), 1);
            if (mState == dPlayerMgr::STATE_SHAKE_TREE && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
                if (setAnm(HMN_ANM_TREE_SHAKE_WAIT1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE)) {
                    mAnm.setFrame(0.0f);
                }
                dFgobjManager::startShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ, 2);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
        } else {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
            if (setAnm(HMN_ANM_TREE_SHAKE_WAIT1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, TRUE)) {
                mAnm.setFrame(0.0f);
            }
            dFgobjManager::startShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ, 2);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        followRemoteTransform();
    }
}

// .text 0xFDD4
void dAcPlayer_c::endShakeTree(int nextState) {
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        if (nextState == dPlayerMgr::STATE_SHAKE_TREE_END) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                if (setAnm(HMN_ANM_TREE_SHAKE_WAIT1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE)) {
                    mAnm.setFrame(0.0f);
                }
                dFgobjManager::startShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ, 2);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
        } else {
            dRumble::stop();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        dFgobjManager::startShakeTree(playerNo, mTargetUnitX, mTargetUnitZ, 2);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    if (nextState != dPlayerMgr::STATE_SHAKE_TREE_END) {
        dFgobjManager::stopShakeTree(mPlayerNo, mTargetUnitX, mTargetUnitZ);
    }
    if (mAnmId == HMN_ANM_TREE_SHAKE_WAIT1) {
        mAnm.mPlayMode = m3d::FORWARD_ONCE;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0xFF10
void dAcPlayer_c::setShakeTreeParam(unitParam_s *param, u8 unitX, u8 unitZ) {
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
}

// .text 0xFF1C
void dAcPlayer_c::initShovelStart() {
    setAnm(HMN_ANM_SCOOP_SWING1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mToolTarget = TOOL_TARGET_NONE;
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER) {
            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_TURN;
        }
        setNetObjc(10.0f, 32.0f, 32.0f, mPos.y);
        mVec3_c pos;
        getPosAtAngle(&pos, getActionAngle(), 48.0f);
        pos.y = mPos.y;
        mObjcPos2.set(this, pos, 10.0f, 32.0f, 4, 0x8000003B, 0, mPlayerNo);
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
        mObjcPos.mHitFunc = objcHit;
        mObjcPos2.mHitFunc = objcHit;
        mObjcPos.regist();
        mObjcPos2.regist();
    }
}

// .text 0x10044
void dAcPlayer_c::executeShovelStart() {
    if (mPlayerNo == dNet::getMyNo()) {
        checkDigTarget();
        if (mToolTarget == TOOL_TARGET_SHOVEL_UNIT) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 2000);
        } else if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_TURN) {
            sLib::chaseAngle(&mAngle.y.mAngle, getActionAngle(), 2000);
        }
        if (mAnm.isStop()) {
            if (mObjcPos.mHitType != 0) {
                mToolTarget = TOOL_TARGET_SHOVEL_OBJECT;
            }
            setState(dPlayerMgr::STATE_SHOVEL_WAIT);
        } else {
            mObjcPos.regist();
            mObjcPos2.regist();
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_SCOOP_SWING_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x1013C
void dAcPlayer_c::endShovelStart(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        mObjcPos.mHitFunc = NULL;
        mObjcPos2.mHitFunc = NULL;
        if (mToolTarget != TOOL_TARGET_SHOVEL_OBJECT) {
            mObjcPos.init();
            mObjcPos2.init();
            _2278 = 0;
            _2266 = dItem::ITEM_ID_NONE;
            _227C = 0;
        }
    }
}

// .text 0x101C0
void dAcPlayer_c::initDigTilt(int frames) {
    mVec3_c center;
    dFdBase_c::getUnitCenterPos(&center, mTargetUnitX, mTargetUnitZ);
    _22CE = 0;
    mTurnFrames = 0;
    mMdlCallback.mRootRotX = 0;
    f32 dy = dBGCF::getGroundY(&center, FALSE) - mPos.y;
    if (dy) {
        mVec3_c normal;
        mVec3_c diff = mPos - center;
        f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
        s16 tilt = cM::atan2s(dy, dist) * -1;
        normal.set((const nw4r::math::VEC3 &)mAcch.mGroundNormal);
        if (0.0f == normal.x && 0.0f == normal.z) {
            _22CE = 0.7f * tilt;
        } else {
            _22CE = tilt;
        }
        if (_22CE > 0x1194) {
            _22CE = 0x1194;
        } else if (_22CE < -0x1194) {
            _22CE = -0x1194;
        }
        mTurnFrames = _22CE / frames;
        if (mTurnFrames < 0) {
            mTurnFrames -= mTurnFrames * 2;
        }
    }
}

// .text 0x10348
void dAcPlayer_c::calcDigTilt(f32 tiltEnd, f32 untiltStart) {
    if (_22CE != 0) {
        if (mAnm.getFrame() < tiltEnd) {
            sLib::chaseAngle(&mMdlCallback.mRootRotX, _22CE, mTurnFrames);
        } else if (mAnm.getFrame() >= untiltStart) {
            sLib::chaseAngle(&mMdlCallback.mRootRotX, 0, mTurnFrames);
        }
    }
}

// .text 0x103DC
void dAcPlayer_c::resetDigTilt() {
    _22CE = 0;
    mTurnFrames = 0;
    mMdlCallback.mRootRotX = 0;
}

// .text 0x103F0
void dAcPlayer_c::initDigApproach(f32 frames, BOOL backOff) {
    static const f32 l_farDistSq = 961.0f;
    static const f32 l_nearDistSq = 576.0f;

    mVec3_c center;
    dFdBase_c::getUnitCenterPos(&center, mTargetUnitX, mTargetUnitZ);
    _21D0[0] = 0.0f;
    f32 dx = center.x - mPos.x;
    f32 dx2 = dx * dx;
    f32 dz = center.z - mPos.z;
    f32 dz2 = dz * dz;
    if (dx2 > l_farDistSq && dz2 > l_farDistSq) {
        dFdBase_c::snapToUnitCenter(&mTargetPos, &mPos);
        mVec3_c diff = mTargetPos - mPos;
        f32 step = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / frames;
        _21D0[0] = step;
        cLib::chasePosXZ(&mPos, mTargetPos, step);
    } else if (backOff) {
        mVec3_c diff = mPos - center;
        if (diff.x * diff.x + diff.z * diff.z < l_nearDistSq) {
            getPosAtAngle(&mTargetPos, mAngle.y.mAngle, -16.0f);
            mVec3_c diff2 = mTargetPos - mPos;
            _21D0[0] = EGG::Math<f32>::sqrt(diff2.x * diff2.x + diff2.z * diff2.z) / frames;
        }
    }
}

// .text 0x1055C
void dAcPlayer_c::calcDigApproach(f32 endFrame) {
    if (_21D0[0] && mAnm.getFrame() < endFrame) {
        cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
    }
}

// .text 0x105CC
void dAcPlayer_c::checkDigTarget() {
    if (mToolTarget == TOOL_TARGET_NONE) {
        if (mObjcPos.mHitType != 0) {
            mToolTarget = TOOL_TARGET_SHOVEL_OBJECT;
        } else {
            mVec3_c pos;
            int unitX = worldToUnit(mPos.x);
            int unitZ = worldToUnit(mPos.z);
            if (!setToolTargetUnit(&pos, unitX, unitZ) && mObjcPos2.mHitType != 0) {
                mToolTarget = TOOL_TARGET_SHOVEL_OBJECT;
            }
            if (mToolTarget == TOOL_TARGET_NONE) {
                int targetX = mTargetUnitX;
                int targetZ = mTargetUnitZ;
                int distX = cM::absI(unitX - targetX);
                int distZ = cM::absI(unitZ - targetZ);
                if ((distX == 0 && distZ == 0) ||
                    !dBGCF::checkWalkable(unitX, unitZ, dBGCF::getUnitQuarter(&mPos), targetX, targetZ)) {
                    f32 groundY = dBGCF::getGroundY(&pos, FALSE);
                    if (groundY > mPos.y) {
                        mToolTarget = TOOL_TARGET_SHOVEL_HIT;
                    } else if (groundY < mPos.y) {
                        mToolTarget = TOOL_TARGET_SHOVEL_LOW;
                    }
                    if (mToolTarget == TOOL_TARGET_NONE) {
                        dFdBase_c *field = dField::getInfo(0);
                        if (field != NULL) {
                            int digType = field->getDigType(targetX, targetZ);
                            if (digType == 2) {
                                mToolTarget = TOOL_TARGET_SHOVEL_HIT;
                            } else if (digType == 3) {
                                mToolTarget = TOOL_TARGET_SHOVEL_LOW;
                            } else {
                                mToolTarget = TOOL_TARGET_SHOVEL_HIT;
                            }
                        } else {
                            mToolTarget = TOOL_TARGET_SHOVEL_HIT;
                        }
                    }
                } else {
                    mToolTarget = TOOL_TARGET_SHOVEL_UNIT;
                    dFdBase_c::getUnitCenterPos(&pos, targetX, targetZ);
                    mTargetAngle = dActor_c::targetAngleY(&mPos, &pos);
                }
            }
        }
    }
}

// .text 0x1079C
void dAcPlayer_c::initShovelWait() {
    setAnm(HMN_ANM_SCOOP_SWING_WAIT1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo && mToolTarget == TOOL_TARGET_SHOVEL_UNIT) {
        _229C = dFgobjManager::useTool(playerNo, mTargetUnitX, mTargetUnitZ, dFgobjManager::USE_TOOL_SHOVEL, 0,
                                       dItem::ITEM_ID_NONE);
    }
}

// .text 0x10830
void dAcPlayer_c::executeShovelWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        switch (mToolTarget) {
        case TOOL_TARGET_SHOVEL_LOW:
            setState(dPlayerMgr::STATE_SHOVEL_SWING_LOW);
            break;
        case TOOL_TARGET_SHOVEL_HIT:
            setState(dPlayerMgr::STATE_SHOVEL_HIT);
            break;
        case TOOL_TARGET_SHOVEL_OBJECT:
            setState(dPlayerMgr::STATE_SHOVEL_HIT_OBJECT);
            break;
        case TOOL_TARGET_SHOVEL_UNIT:
            if (_229C == -1) {
                setState(dPlayerMgr::STATE_SHOVEL_MISS);
            } else {
                bool start = false;
                if (mAnm.getFrame() <= 1.0f) {
                    start = true;
                }
                waitFieldTask(0, start, 0);
            }
            break;
        default:
            setState(dPlayerMgr::STATE_SHOVEL_MISS);
            break;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x1093C
void dAcPlayer_c::endShovelWait(int nextState) {}

// .text 0x10940
void dAcPlayer_c::initShovelHit() {
    setAnm(HMN_ANM_SCOOP_HANE1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
        pos.y = mPos.y;
        mObjcPos.set(this, pos, 15.0f, 15.0f, 4, dObjc::TYPE_SNOWBALL, 0, mPlayerNo);
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
        mObjcPos.mHitFunc = objcHit;
        mObjcPos.regist();
        mShovelTaskStage = 0;
        dFgMngTask_c *task = fgMngTask_get(fgMngTask_findGroup((u8)mPlayerNo, 3, TRUE));
        if (task != NULL) {
            mShovelTaskStage = task->mStage;
        }
        BOOL chop = FALSE;
        if (mTaskKind == FG_MNG_KIND_HIT_MONEY_ROCK) {
            chop = TRUE;
        }
        getPosAtAngle(&mTargetPos, mAngle.y.mAngle, -8.0f);
        setShovelHitParam((shovelHitParam_s *)&mRemoteActionParams, mAngle.y.mAngle, mTargetUnitX, mTargetUnitZ, 0,
                          chop, mShovelTaskStage == 1);
    } else {
        shovelHitParam_s *param = (shovelHitParam_s *)&mRemoteActionParams;
        mTargetUnitZ = param->mUnitZ;
        mTargetUnitX = param->mUnitX;
        if (param->mChop != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
    }
}

// .text 0x10AE4
void dAcPlayer_c::executeShovelHit() {
    static const f32 l_hitFrame = 11.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        f32 frame = mAnm.getFrame();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else {
            if (mAnm.checkFrame(l_hitFrame)) {
                checkDigUnit(0, mShovelTaskStage == 1);
            } else if (mAnm.checkFrame(12.0f)) {
                if (mTaskKind == FG_MNG_KIND_HIT_MONEY_ROCK) {
                    dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ, mAngle.y.mAngle);
                }
                checkDigUnit(1, mShovelTaskStage == 1);
            } else if (frame > 12.0f && frame < 32.0f) {
                cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.3f, 2.0f);
            }
            mObjcPos.regist();
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(l_hitFrame)) {
            checkDigUnit(0, ((shovelHitParam_s *)&mRemoteActionParams)->mTaskStage1);
        } else if (mAnm.checkFrame(12.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ,
                                              ((shovelHitParam_s *)&mRemoteActionParams)->mAngle);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            checkDigUnit(1, ((shovelHitParam_s *)&mRemoteActionParams)->mTaskStage1);
        }
    }
}

// .text 0x10CDC
void dAcPlayer_c::endShovelHit(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mObjcPos.init();
        mObjcPos.mHitFunc = NULL;
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ, mAngle.y.mAngle);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x10D70
void dAcPlayer_c::setShovelHitParam(shovelHitParam_s *param, s16 angle, u8 unitX, u8 unitZ, u8 arg4, BOOL chop,
                                    BOOL taskStage1) {
    param->mAngle = angle;
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
    param->_4 = arg4;
    param->mChop = chop;
    param->mTaskStage1 = taskStage1;
}

// .text 0x10D8C
void dAcPlayer_c::initShovelDig() {
    static const f32 l_approachFrames = 15.0f;
    static const u16 l_tiltFrames = 9;

    int anm = HMN_ANM_SCOOP_DIG1;
    if (mPlayerNo == dNet::getMyNo()) {
        BOOL tree = FALSE;
        if (mTaskKind == FG_MNG_KIND_DIG_OUT) {
            anm = HMN_ANM_SCOOP_STUMP1;
            tree = TRUE;
        }
        setAnm(anm, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        initDigApproach(l_approachFrames, TRUE);
        setShovelDigParam((shovelDigParam_s *)&mRemoteActionParams, _2262, _2260, mTargetUnitX, mTargetUnitZ,
                          tree);
    } else {
        shovelDigParam_s *param = (shovelDigParam_s *)&mRemoteActionParams;
        if (param->mTree != 0) {
            mTaskKind = FG_MNG_KIND_DIG_OUT;
            anm = HMN_ANM_SCOOP_STUMP1;
        } else {
            mTaskKind = FG_MNG_KIND_DIG_HOLE;
            anm = HMN_ANM_SCOOP_DIG1;
        }
        int x = param->mUnitX;
        int z = param->mUnitZ;
        mTargetUnitX = x;
        mTargetUnitZ = z;
        _2262 = param->mTreeItem;
        _2260 = param->mItem;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    setAnm(anm, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    initDigTilt(l_tiltFrames);
}

// .text 0x10EEC
void dAcPlayer_c::executeShovelDig() {
    static const f32 l_tiltFrames = 10.0f;
    static const f32 l_tiltAngle = 30.0f;
    static const f32 l_startFrame = 20.0f;
    static const f32 l_endFrame = 60.0f;

    calcDigTilt(l_tiltFrames, l_tiltAngle);
    if (mPlayerNo == dNet::getMyNo()) {
        calcDigEffects();
        calcDigApproach(l_tiltFrames);
        f32 frame;
        if (mTaskKind == FG_MNG_KIND_DIG_HOLE) {
            frame = l_startFrame;
        } else {
            frame = l_endFrame;
        }
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(frame)) {
            spawnDigResult(mTaskKind == FG_MNG_KIND_DIG_OUT);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        calcDigEffects();
        followRemoteTransform();
        f32 frame;
        if (((shovelDigParam_s *)&mRemoteActionParams)->mTree != 0) {
            frame = l_endFrame;
        } else {
            frame = l_startFrame;
        }
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(frame) && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
            spawnDigResult(((shovelDigParam_s *)&mRemoteActionParams)->mTree);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
    }
}

// .text 0x6B0C
void dAcPlayer_c::calcDigEffects() {
    if (mAnmId == HMN_ANM_SCOOP_STUMP1) {
        if (mAnm.checkFrame(1.0f)) {
            setDigAnm(1);
        } else if (mAnm.checkFrame(9.0f)) {
            startWeatherSound(SE_kirikabu_scoop, SE_kirikabu_scoop_rain);
            createDigEffect("af_ply_dig_c_sand01", "af_ply_dig_c_nml01");
            startRumble(dRumble::PATTERN_DIG_STUMP);
        } else if (mAnm.checkFrame(60.0f)) {
            startWeatherSound(SE_kirikabu_out, SE_kirikabu_out_rain);
            createDigEffect("af_ply_dig_c_sand02", "af_ply_dig_c_nml02");
            startRumble(dRumble::PATTERN_DIG_STUMP_OUT);
        }
    } else if (mAnmId == HMN_ANM_SCOOP_DIG1) {
        if (mAnm.checkFrame(1.0f)) {
            setDigAnm(0);
        } else if (mAnm.checkFrame(9.0f)) {
            createDigEffect("af_ply_dig_a_sand01", "af_ply_dig_a_nml01");
        } else if (mAnm.checkFrame(15.0f)) {
            startWeatherSound(SE_Scoop, SE_Scoop_rain);
            startRumble(dRumble::PATTERN_DIG);
        } else if (mAnm.checkFrame(28.0f)) {
            createDigEffect("af_ply_dig_a_sand02", "af_ply_dig_a_nml02");
            dItem::Item item(_2262);
            if (item.isWeed() || item.isClover() || item.isWiltedFlower() || item.isFlower() ||
                item.isDandelion()) {
                dFgobjManager::playGrassEffect(mTargetUnitX, mTargetUnitZ, item.getId(), &mAngle, TRUE);
            }
        }
    }
}

// .text 0x11068
void dAcPlayer_c::endShovelDig(int nextState) {
    resetDigTilt();
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        spawnDigResult(mTaskKind == FG_MNG_KIND_DIG_OUT);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x110E0
void dAcPlayer_c::spawnDigResult(BOOL tree) {
    int unitX = mTargetUnitX;
    int unitZ = mTargetUnitZ;
    if (tree) {
        dFgobjManager::spawnDigUp(mPlayerNo, _2260, unitX, unitZ, 0);
        dFgobjManager::spawnFellTree(mPlayerNo, _2262, unitX, unitZ, mAngle.y.mAngle);
    } else {
        dFgobjManager::spawnDigUp(mPlayerNo, _2260, unitX, unitZ, 0);
    }
}

// .text 0x11174
void dAcPlayer_c::createDigEffect(const char *sandEffect, const char *effect) {
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
    pos.y = dBGCF::getGroundY(&pos, FALSE);
    dBGCF::groundChk_c chk(&pos, 0, 0, 0);
    BOOL sand = FALSE;
    if (chk.mAttr == BG_ATTR_SAND || chk.mWater == BG_WATER_SEA) {
        sand = TRUE;
    }
    dEffect::create(sand ? sandEffect : effect, &pos, &mAngle, NULL);
}

// .text 0x11230
void dAcPlayer_c::setShovelDigParam(shovelDigParam_s *param, u16 treeItem, u16 item, u8 unitX, u8 unitZ, u8 tree) {
    param->mTreeItem = treeItem;
    param->mItem = item;
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
    param->mTree = tree;
}

// .text 0x11248
void dAcPlayer_c::initShovelMiss() {
    setAnm(HMN_ANM_SCOOP_CANCEL1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x11270
void dAcPlayer_c::executeShovelMiss() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnm.isStop()) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x112E8
void dAcPlayer_c::endShovelMiss(int nextState) {}

// .text 0x112EC
void dAcPlayer_c::initShovelSwingLow() {
    setAnm(HMN_ANM_SCOOP_SUKA1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x11314
void dAcPlayer_c::executeShovelSwingLow() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(10.0f)) {
            startSound(SE_ScoopKaraburi);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(10.0f)) {
            startSound(SE_ScoopKaraburi);
        }
    }
}

// .text 0x113F0
void dAcPlayer_c::endShovelSwingLow(int nextState) {}

// .text 0x113F4
void dAcPlayer_c::initPutItemInHole() {
    static const int l_turnFrames = 12;

    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo == myNo) {
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION)) {
            if (_229C == -1) {
                _229C = dFgobjManager::useToolAtTask(playerNo, dFgobjManager::USE_TOOL_SHOVEL, 0, _218C);
            }
            setAnm(HMN_ANM_SCOOP_SWING1, m3d::FORWARD_ONCE);
        }
        mVec3_c pos;
        dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
        mTargetAngle = dActor_c::targetAngleY(&mPos, &pos);
        mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y.mAngle) / l_turnFrames;
        if (mTurnFrames <= 0) {
            mTurnFrames = 1000;
        }
    } else {
        setAnm(HMN_ANM_SCOOP_SWING1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    }
    _22A4[7] = 0;
}

// .text 0x11504
void dAcPlayer_c::executePutItemInHole() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION)) {
            bool ready = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            if (mAnmId == HMN_ANM_SCOOP_SWING1) {
                if (mAnm.isStop()) {
                    setAnm(HMN_ANM_SCOOP_SWING_WAIT1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
                } else {
                    ready = false;
                }
            } else if (ready && !(mAnm.getFrame() <= 1.0f)) {
                ready = false;
            }
            if (_229C != -1 ||
                (_229C = dFgobjManager::useToolAtTask(mPlayerNo, dFgobjManager::USE_TOOL_SHOVEL, 0, _218C)) != -1) {
                waitFieldTask(0, ready, 0);
            }
        } else {
            if (2.0f == _22A4[7]) {
                setAnm(HMN_ANM_SCOOP_SWING1, m3d::FORWARD_ONCE);
            } else {
                _22A4[7]++;
            }
            if (mAnmId == HMN_ANM_SCOOP_SWING1) {
                sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
                if (mAnm.isStop()) {
                    setState(dPlayerMgr::STATE_SHOVEL_BURY);
                }
            }
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_SCOOP_SWING_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x116E0
void dAcPlayer_c::endPutItemInHole(int nextState) {}

// .text 0x116E4
void dAcPlayer_c::initShovelBury() {
    static const f32 l_approachFrames = 19.0f;
    static const u16 l_tiltFrames = 11;

    if (mPlayerNo == dNet::getMyNo()) {
        u8 putItem = TRUE;
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) ||
            (mActorFlags & dPlayerMgr::ACTOR_FLAG_HOLE_FROM_TASK)) {
            putItem = FALSE;
        }
        setShovelBuryParam((shovelBuryParam_s *)&mRemoteActionParams, _218C, mTargetUnitX, mTargetUnitZ, putItem);
        initDigApproach(l_approachFrames, FALSE);
    } else {
        shovelBuryParam_s *param = (shovelBuryParam_s *)&mRemoteActionParams;
        mTargetUnitZ = param->mUnitZ;
        mTargetUnitX = param->mUnitX;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    initDigTilt(l_tiltFrames);
    setAnm(HMN_ANM_SCOOP_BURY1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x117D0
void dAcPlayer_c::executeShovelBury() {
    static const f32 l_hitFrame = 12.0f;
    static const f32 l_buryFrame = 69.0f;
    static const f32 l_endFrame = 64.0f;
    static const f32 l_stampFrame = 20.0f;

    calcDigTilt(l_hitFrame, l_buryFrame);
    if (mPlayerNo == dNet::getMyNo()) {
        calcBuryEffects();
        calcDigApproach(l_hitFrame);
        if (mAnm.checkFrame(15.0f)) {
            dFgobjManager::spawnDigHole(mPlayerNo, mTargetUnitX, mTargetUnitZ);
        } else if (mAnm.checkFrame(l_stampFrame)) {
            startWeatherSound(SE_ScoopUmeru, SE_ScoopUmeru_rain);
        } else if (mAnm.checkFrame(l_endFrame)) {
            startWeatherSound(SE_ScoopUmeru_pashin, SE_ScoopUmeru_pashin_rain);
        } else if (mAnm.isStop()) {
            if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) ||
                (mActorFlags & dPlayerMgr::ACTOR_FLAG_HOLE_FROM_TASK)) {
                setState(dPlayerMgr::STATE_IDLE);
            } else {
                setState(dPlayerMgr::STATE_DEMO_START);
            }
        }
    } else {
        u8 putItem = ((shovelBuryParam_s *)&mRemoteActionParams)->mPutItem;
        if ((putItem != 0 && mAnmId != HMN_ANM_MENU_THINK1) || (putItem == 0 && mAnmId != HMN_ANM_NML_WAIT1)) {
            calcBuryEffects();
            followRemoteTransform();
            if (mAnm.checkFrame(15.0f)) {
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                    dFgobjManager::spawnDigHole(mPlayerNo, mTargetUnitX, mTargetUnitZ);
                    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
                }
            } else if (mAnm.checkFrame(l_stampFrame)) {
                startWeatherSound(SE_ScoopUmeru, SE_ScoopUmeru_rain);
            } else if (mAnm.checkFrame(l_endFrame)) {
                startWeatherSound(SE_ScoopUmeru_pashin, SE_ScoopUmeru_pashin_rain);
            } else if (mAnm.isStop()) {
                if (putItem != 0) {
                    setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
                } else {
                    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
                }
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            }
        }
    }
}

// .text 0x6D60
void dAcPlayer_c::calcBuryEffects() {
    if (mAnm.checkFrame(1.0f)) {
        setDigAnm(3);
    } else if (mAnm.checkFrame(10.0f)) {
        createDigEffect("af_ply_bury_sand01", "af_ply_bury_nml01");
        startRumble(dRumble::PATTERN_BURY);
    } else if (mAnm.checkFrame(25.0f)) {
        createDigEffect("af_ply_bury_sand02", "af_ply_bury_nml02");
    } else if (mAnm.checkFrame(64.0f)) {
        createStampEffect();
        startRumble(dRumble::PATTERN_BURY_END);
    }
}

// .text 0x11A10
void dAcPlayer_c::endShovelBury(int nextState) {
    resetDigTilt();
    int myNo = dNet::getMyNo();
    int playerNo = mPlayerNo;
    if (playerNo != myNo && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        dFgobjManager::spawnDigHole(playerNo, mTargetUnitX, mTargetUnitZ);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_MENU_ACTION;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HOLE_FROM_TASK;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x11A88
void dAcPlayer_c::createStampEffect() {
    static const char *effects[] = {
        "afi_ply_stamp_nml", "af_ply_stamp_water02", "af_ply_stamp_water02", "af_ply_stamp_snow02",
        "afi_ply_stamp_sand",
    };
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
    pos.y = dBGCF::getGroundY(&pos, FALSE);
    dPlayerMgr::createSurfaceEffects(effects, &pos, &mAngle, "af_ply_stamp_water", "af_ply_stamp_snow");
}

// .text 0x11AF8
void dAcPlayer_c::setShovelBuryParam(shovelBuryParam_s *param, u16 item, u8 unitX, u8 unitZ, u8 putItem) {
    param->mItem = item;
    param->mUnitX = unitX;
    param->mUnitZ = unitZ;
    param->mPutItem = putItem;
}

#include "d_a_player_item_begin.inc"

// .text 0x132BC
namespace {
extern const f32 l_pickUpOffsetX = -4.10376f;
extern const f32 l_pickUpOffsetY = 3.173462f;
extern const f32 l_pickUpOffsetZ = 9.193726f;
}

