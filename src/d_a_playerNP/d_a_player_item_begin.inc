namespace {
extern const f32 l_pickUpOffsetX, l_pickUpOffsetY, l_pickUpOffsetZ;
static inline void getPickUpItemOffset(f32 &x, f32 &y, f32 &z) {
    x = l_pickUpOffsetX;
    y = l_pickUpOffsetY;
    z = l_pickUpOffsetZ;
}

}

// dAcPlayer_c: dig-up, pick-up, axe, eat, umbrella, trip, mail and stairs states (.text 0x129C4..0x1E398;
// the string-literal functions of event.inc up to its first data object are placed at the end of this
// section). Included by d_a_player.cpp.

// .text 0x129C4
void dAcPlayer_c::initShovelDigUpItem() {
    static const f32 l_approachFrames = 15.0f;
    static const u16 l_tiltFrames = 9;

    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    mTargetAngle = 0;
    setAnm(HMN_ANM_SCOOP_GET1, 10.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mStep = 10;
    if (mPlayerNo == dNet::getMyNo()) {
        setTargetUnitItem(0);
        initDigApproach(l_approachFrames, TRUE);
        BOOL special = TRUE;
        if (mTaskKind != FG_MNG_KIND_DIG_UP_FULL && mTaskKind != FG_MNG_KIND_PICK_UP_SPECIAL_FULL) {
            special = FALSE;
        }
        getDigUpParam()->set(mActionItem.mId, mTargetUnitX, mTargetUnitZ, special);
    } else {
        digUpItemParam_s *param = (digUpItemParam_s *)&mRemoteActionParams;
        if (param->mSpecial != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        } else {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        }
        mTargetUnitZ = param->mUnitZ;
        mTargetUnitX = param->mUnitX;
        mActionItem = param->mItem;
        _2260 = param->mItem;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    initDigTilt(l_tiltFrames);
}

// .text 0x11B0C
void dAcPlayer_c::executeShovelDigUpItem() {
    static const f32 l_tiltFrames = 10.0f;
    static const f32 l_tiltAngle = 30.0f;

    calcDigTilt(l_tiltFrames, l_tiltAngle);
    if (mAnmId == HMN_ANM_SCOOP_GET1) {
        if (mAnm.checkFrame(109.0f)) {
            createToolGlitter(&mActionItem);
        } else if (mAnm.checkFrame(32.0f)) {
            createDigEffect("af_ply_dig_b_sand02", "af_ply_dig_b_nml02");
        } else if (mAnm.checkFrame(15.0f)) {
            startWeatherSound(SE_Scoop, SE_Scoop_rain);
        } else if (mAnm.checkFrame(12.0f)) {
            createDigEffect("af_ply_dig_b_sand01", "af_ply_dig_b_nml01");
        } else if (mAnm.checkFrame(1.0f)) {
            setDigAnm(2);
        }
    }
    int playerNo = mPlayerNo;
    if (playerNo == dNet::getMyNo()) {
        f32 frame = mAnm.getFrame();
        calcDigApproach(l_tiltFrames);
        if (frame > 92.0f) {
            stepDigUpMsg();
        }
        if (frame > 70.0f) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mMdlCallback.mHandPos, &scale);
        } else if (mAnm.checkFrame(70.0f)) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mScriptedActionPos, &scale);
            if (dCamera_c::sInstance != NULL) {
                dCamera_c::sInstance->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
            }
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->mute0();
            }
        } else if (frame > l_tiltAngle) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mScriptedActionPos, &scale);
        } else if (mAnm.checkFrame(l_tiltAngle)) {
            spawnDugItem(mTaskKind == FG_MNG_KIND_DIG_UP_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            startSound(SE_HoriokoshitaHakken);
        } else if (mAnm.checkFrame(15.0f)) {
            startRumble(dRumble::PATTERN_DIG_UP_ITEM);
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, playerNo)) {
            mTargetPos.set(x, mPos.y, z);
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 2.0f);
        }
        if (mAnm.isStop()) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mMdlCallback.mHandPos, &scale);
        } else if (mAnm.getFrame() > 70.0f) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mMdlCallback.mHandPos, &scale);
        } else if (mAnm.getFrame() > l_tiltAngle) {
            mVec3_c scale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.mId, &mScriptedActionPos, &scale);
        } else if (mAnm.checkFrame(l_tiltAngle)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                spawnDugItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
            startSound(SE_HoriokoshitaHakken);
        }
    }
}

// .text 0x12AFC
void dAcPlayer_c::endShovelDigUpItem(int nextState) {
    resetDigTilt();
    if (mPlayerNo == dNet::getMyNo()) {
        if (dCamera_c::sInstance != NULL) {
            dCamera_c::sInstance->mRequestedMode = dCamera_c::sInstance->mDefaultMode;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop0(0xB, 0x1A);
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        spawnDugItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x12BA0
void dAcPlayer_c::stepDigUpMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play0(10);
            }
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            int kind = _2188.getKind();
            int code = MSG_DUG_UP;
            if (kind == dItem::KIND_GOLD_SCOOP) {
                dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                if (pd != NULL && !pd->isFlag0(PRIVATE_FLAG0_DUG_GOLD_SCOOP)) {
                    pd->setFlag0(PRIVATE_FLAG0_DUG_GOLD_SCOOP);
                    code = MSG_GOT_GOLD_SCOOP;
                }
            } else if (kind == dItem::KIND_GOLD_PACHINKO) {
                dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                if (pd != NULL && !pd->isFlag0(PRIVATE_FLAG0_DUG_GOLD_PACHINKO)) {
                    pd->setFlag0(PRIVATE_FLAG0_DUG_GOLD_PACHINKO);
                    code = MSG_GOT_GOLD_PACHINKO;
                }
            }
            setMessageCode(code);
            getController()->requestStart(0);
            if (code == 0xB) {
                getController()->setItemName(0, _2188);
            }
            mStep = 11;
            putInPocket(&_2188);
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
        if (mTaskKind == FG_MNG_KIND_DIG_UP_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL) {
            mMsgStep = MSG_STEP_QUEUE_ASK_BURY;
        }
        break;
    }
    case 12:
        if (mTaskKind == FG_MNG_KIND_DIG_UP_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL) {
            if (mMsgStep == MSG_STEP_SWAP_DUG) {
                changeStateAfterMsg(dPlayerMgr::STATE_SHOW_DUG_ITEM);
            } else if (mMsgStep == MSG_STEP_NONE) {
                changeStateAfterMsg(dPlayerMgr::STATE_PUT_ITEM_IN_HOLE);
            }
        } else if (mTaskKind == FG_MNG_KIND_DIG_UP || mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL) {
            changeStateAfterMsg(dPlayerMgr::STATE_SHOW_DUG_ITEM);
        } else {
            changeStateAfterMsg(dPlayerMgr::STATE_IDLE);
        }
        break;
    }
}

// .text 0x11F1C
void dAcPlayer_c::executeStampHole() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.getFrame() > 66.0f) {
            if (checkMoveOrAction()) {
                return;
            }
        } else if (mAnm.checkFrame(15.0f)) {
            createGroundEffect("af_ply_footbury_sand", "af_ply_footbury_nml");
        } else if (mAnm.checkFrame(20.0f)) {
            dFgobjManager::spawnFillHole(mPlayerNo, mTargetUnitX, mTargetUnitZ);
            startWeatherSound(SE_AnaUmeByFoot, SE_AnaUmeByFoot_rain);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.checkFrame(15.0f)) {
            createGroundEffect("af_ply_footbury_sand", "af_ply_footbury_nml");
        } else if (mAnm.checkFrame(20.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                dFgobjManager::spawnFillHole(mPlayerNo, mTargetUnitX, mTargetUnitZ);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            startWeatherSound(SE_AnaUmeByFoot, SE_AnaUmeByFoot_rain);
        } else if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        }
    }
}

// .text 0x12DF4
void dAcPlayer_c::spawnDugItem(int full) {
    dFgobjManager::spawnDigUp(mPlayerNo, _2260, mTargetUnitX, mTargetUnitZ, full);
}

// .text 0x12E10
void dAcPlayer_c::digUpParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 full) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mFull = full;
}

// .text 0x12E24
void dAcPlayer_c::initShowDugItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        getShowDugItemParam()->set(mActionItem.getId(), mTaskKind == FG_MNG_KIND_DIG_UP_FULL ||
                                                            mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL);
    } else {
        mActionItem = getShowDugItemParam()->mItem;
    }
    setAnm(HMN_ANM_SCOOP_PUTAWAY1, 18.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x12EC4
void dAcPlayer_c::executeShowDugItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        executeShowItem(HMN_ANM_SCOOP_PUTAWAY1,
                        mTaskKind == FG_MNG_KIND_DIG_UP_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL, FALSE);
    } else {
        executeShowItem(HMN_ANM_SCOOP_PUTAWAY1, getShowDugItemParam()->mFull, FALSE);
    }
}

// .text 0x12F40
void dAcPlayer_c::endShowDugItem(int nextState) {}

// .text 0x12F44
void dAcPlayer_c::showDugItemParam_c::set(u16 item, u8 full) {
    mItem = item;
    mFull = full;
}

// .text 0x12F50
void dAcPlayer_c::initFillHole() {
    if (mPlayerNo == dNet::getMyNo()) {
        getFillHoleParam()->set(mTargetUnitX, mTargetUnitZ);
    } else {
        mTargetUnitZ = getFillHoleParam()->mUnitZ;
        mTargetUnitX = getFillHoleParam()->mUnitX;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    setAnm(HMN_ANM_HOLE_BURY1, 3.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x13000
void dAcPlayer_c::endFillHole(int nextState) {
    int myNo = dNet::getMyNo();
    if (mPlayerNo != myNo && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        dFgobjManager::spawnFillHole(mPlayerNo, mTargetUnitX, mTargetUnitZ);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1306C
void dAcPlayer_c::fillHoleParam_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x13078
void dAcPlayer_c::initPickUpReach() {
    setAnm(HMN_ANM_PICKUP_WAIT1, m3d::FORWARD_LOOP);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1AD(m3d::FORWARD_LOOP, 8.0f);
        mpToolBank->setAnmFrame(mAnm.getFrame());
    }
}

// .text 0x130EC
void dAcPlayer_c::executePickUpReach() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (_229C != -1) {
            waitFieldTask(0, 1, 0);
        } else {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x13160
void dAcPlayer_c::endPickUpReach(int nextState) {}

// .text 0x13164
void dAcPlayer_c::initPickUpItem() {
    setAnm(HMN_ANM_PICKUP1, 11.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1AB(m3d::FORWARD_ONCE, 8.0f);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        BOOL full = FALSE;
        setTargetUnitItem(0);
        if (mTaskKind == FG_MNG_KIND_PICK_UP) {
            putInPocket(&_2188);
        } else if (mTaskKind == FG_MNG_KIND_PICK_UP_FULL) {
            full = TRUE;
        }
        initPickUpItemPos();
        getPickUpParam()->set(mActionItem.getId(), mTargetUnitX, mTargetUnitZ, full);
    } else {
        mTargetUnitZ = getPickUpParam()->mUnitZ;
        mTargetUnitX = getPickUpParam()->mUnitX;
        if (getPickUpParam()->mFull != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        } else {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        }
        mActionItem = getPickUpParam()->mItem;
        _218C = getPickUpParam()->mItem;
        mPickUpKind = 0;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        initPickUpItemPos();
    }
}

