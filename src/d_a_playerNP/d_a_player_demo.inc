// dAcPlayer_c: held-item / design / Mii mask changes, demo start / wait, doors and the net swing
// (.text 0x6E50..0x9614). Included by d_a_player.cpp.

// .text 0x6E50
void dAcPlayer_c::initChangeHeldItem() {
    mStep = 8;
    mTargetAngle = 0;
    if (mPlayerNo == dNet::getMyNo()) {
        int anmId = HMN_ANM_NML_WAIT1;
        if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) &&
            !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU)) {
            anmId = HMN_ANM_MENU_THINK1;
        }
        setAnm(anmId, m3d::FORWARD_LOOP);
        dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        setHeldItemParam((heldItemParam_s *)&mRemoteActionParams, mHeldItemChange, mStep, isHoldPose());
    } else {
        int anmId = HMN_ANM_NML_WAIT1;
        if (getHeldItemParam()->mHoldPose != 0) {
            anmId = HMN_ANM_MENU_THINK1;
        }
        setAnm(anmId, m3d::FORWARD_LOOP);
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
            mHeldItemChange = getHeldItemParam()->mChange;
            mHeldEquipStep = EQUIP_STEP_LOAD;
        }
        mStep = getHeldItemParam()->mStep;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
}

// .text 0x6F58
void dAcPlayer_c::executeChangeHeldItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        executeChangeHeldItemLocal();
    } else {
        executeChangeHeldItemRemote();
    }
}

// .text 0x6FA4
void dAcPlayer_c::endChangeHeldItem(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
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
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_MENU_ACTION;
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
}

// .text 0x7060
void dAcPlayer_c::executeChangeHeldItemLocal() {
    static const f32 l_endFrame = 60.0f;

    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
        dItem::Item held = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld;
        if (!dHmnToolBank_c::isHandTool(&held)) {
            mMdl.removePartAnm(1, 0.0f);
            removeTool();
            setState(dPlayerMgr::STATE_DEMO_START);
            return;
        }
    } else if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        setState(dPlayerMgr::STATE_DEMO_START);
        return;
    }
    if (mStep == 8) {
        if (mHeldEquipStep == EQUIP_STEP_LOADED) {
            setAnm(((mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) &&
                    !(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU))
                       ? HMN_ANM_MENU_EQUIP1
                       : HMN_ANM_NML_EQUIP1,
                   16.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            mStep = 9;
        }
    } else if (mStep == 9) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
        if (mAnm.checkFrame(21.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) {
                dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                if (priv != NULL && priv->mEquipment.mHeld.getId() == dItem::ITEM_ID_NONE) {
                    startSound(SE_EqOff);
                }
            }
            mToolScale.set(0.0f, 0.0f, 0.0f);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
        } else if (mAnm.checkFrame(30.0f)) {
            if (mHeldEquipStep == EQUIP_STEP_LOADED) {
                mHeldEquipStep = EQUIP_STEP_APPLIED;
                if ((int)mHeldItemChange == dPlayerMgr::HELD_ITEM_CHANGE_EQUIP) {
                    startSound(SE_item_soubi);
                    createHeldToolGlitter();
                    mToolScale.set(1.0f, 1.0f, 1.0f);
                    mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
                }
            }
        } else if (mAnm.checkFrame(31.0f)) {
            setToolAnm(mAnmId, TRUE, l_endFrame);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
        } else if (mAnm.isStop()) {
            int state = dPlayerMgr::STATE_IDLE;
            int anm = HMN_ANM_NML_WAIT1;
            if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) {
                state = dPlayerMgr::STATE_DEMO_WAIT;
            } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) {
                state = dPlayerMgr::STATE_DEMO_START;
                anm = HMN_ANM_MENU_THINK1;
            }
            setAnm(anm, 16.0f, 0.0f, 1.0f, l_endFrame, m3d::FORWARD_LOOP, FALSE);
            setState(state);
        }
    }
}

// .text 0x731C
void dAcPlayer_c::executeChangeHeldItemRemote() {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        int anm = HMN_ANM_NML_WAIT1;
        if (((heldItemParam_s *)&mRemoteActionParams)->mHeldOut != 0) {
            anm = HMN_ANM_MENU_THINK1;
        }
        setAnm(anm, m3d::FORWARD_LOOP);
        return;
    }
    if (mStep == 8) {
        if ((mAnmId == HMN_ANM_MENU_EQUIP1 || mAnmId == HMN_ANM_NML_EQUIP1) && mAnm.isStop()) {
            setAnm(((heldItemParam_s *)&mRemoteActionParams)->mHeldOut != 0 ? HMN_ANM_MENU_THINK1 : HMN_ANM_NML_WAIT1,
                   m3d::FORWARD_LOOP);
        }
        if (mHeldEquipStep == EQUIP_STEP_LOADED) {
            if (mAnmId == HMN_ANM_MENU_EQUIP1 || mAnmId == HMN_ANM_NML_EQUIP1) {
                mAnm.setFrame(0.0f);
            } else {
                setAnm(((heldItemParam_s *)&mRemoteActionParams)->mHeldOut != 0 ? HMN_ANM_MENU_EQUIP1
                                                                                : HMN_ANM_NML_EQUIP1,
                       16.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            }
            mTargetAngle = 0;
            mStep = 9;
        }
    } else if (mStep == 9) {
        if (mAnmId == HMN_ANM_MENU_EQUIP1 || mAnmId == HMN_ANM_NML_EQUIP1) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
            if (mAnm.checkFrame(21.0f)) {
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) {
                    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
                    if (priv != NULL && priv->mEquipment.mHeld.getId() == dItem::ITEM_ID_NONE) {
                        startSound(SE_EqOff);
                    }
                }
                mToolScale.set(0.0f, 0.0f, 0.0f);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
            } else if (mAnm.checkFrame(30.0f)) {
                if (mHeldEquipStep == EQUIP_STEP_LOADED) {
                    mHeldEquipStep = EQUIP_STEP_APPLIED;
                    if ((int)mHeldItemChange == dPlayerMgr::HELD_ITEM_CHANGE_EQUIP) {
                        startSound(SE_item_soubi);
                        createHeldToolGlitter();
                        mToolScale.set(1.0f, 1.0f, 1.0f);
                        mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
                    }
                }
            } else if (mAnm.checkFrame(31.0f)) {
                setToolAnm(mAnmId, TRUE, 60.0f);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
            } else if (mAnm.isStop()) {
                int anm = HMN_ANM_NML_WAIT1;
                if (((heldItemParam_s *)&mRemoteActionParams)->mHeldOut != 0) {
                    anm = HMN_ANM_MENU_THINK1;
                }
                setAnm(anm, 16.0f, 0.0f, 1.0f, 60.0f, m3d::FORWARD_LOOP, FALSE);
            }
        }
    }
}

// .text 0x75CC
void dAcPlayer_c::setHeldItemParam(heldItemParam_s *param, u8 change, u8 step, u8 heldOut) {
    param->mChange = change;
    param->mStep = step;
    param->mHeldOut = heldOut;
}

// .text 0x75DC
void dAcPlayer_c::initChangeDesign() {
    setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    startChangeEquip();
}

// .text 0x7618
void dAcPlayer_c::executeChangeDesign() {
    if (mPlayerNo == dNet::getMyNo()) {
        executeChangeEquipMine(dPlayerMgr::STATE_DEMO_WAIT, HMN_ANM_NML_CHANGE1, HMN_ANM_NML_WAIT1);
    } else {
        executeChangeEquipRemote(HMN_ANM_NML_CHANGE1, HMN_ANM_NML_WAIT1);
    }
}

// .text 0x7678
void dAcPlayer_c::endChangeDesign(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOAD_INIT;
        }
    }
}

// .text 0x76D8
void dAcPlayer_c::initMiiMask() {
    mTargetAngle = 0;
    setAnm(HMN_ANM_MENU_CHANGE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x774C
void dAcPlayer_c::executeMiiMask() {
    if (mAnmId == HMN_ANM_MENU_CHANGE1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_wear);
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        } else if (mAnm.checkFrame(38.0f)) {
            createChangeEffect();
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_DEMO_START);
        } else if (mAnm.checkFrame(24.0f)) {
            updateMiiMask();
        }
    } else if (mAnmId != HMN_ANM_MENU_THINK1) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(24.0f)) {
            updateMiiMask();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
}

// .text 0x78B0
void dAcPlayer_c::endMiiMask(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING)) {
        updateMiiMask();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x7908
void dAcPlayer_c::endDemoStart(int nextState) {}

// .text 0x5D3C
void dAcPlayer_c::initDemoStart() {
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        if (mState == dPlayerMgr::STATE_EAT_SPECIAL) {
            _22A4[7] = 150;
            setAnmNoMorph(HMN_ANM_MENU_THINK2, m3d::FORWARD_LOOP);
        } else if (mState == dPlayerMgr::STATE_SHOW_CATCH) {
            setAnm(HMN_ANM_MENU_THINK1, 30.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
        setDemoStartParam((demoStartParam_s *)&mRemoteActionParams, mState == dPlayerMgr::STATE_EAT_SPECIAL);
    } else {
        if (getDemoStartParam()->mEatSpecial != 0) {
            if (mState == dPlayerMgr::STATE_EAT_SPECIAL) {
                setAnmNoMorph(HMN_ANM_MENU_THINK2, m3d::FORWARD_LOOP);
            } else {
                setAnm(HMN_ANM_MENU_THINK2, m3d::FORWARD_LOOP);
            }
        } else if (mState == dPlayerMgr::STATE_ENTER_ROOM) {
            f32 x;
            f32 z;
            s16 angle;
            if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
                mTargetPos.set(x, mPos.y, z);
                mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
            }
            mVec3_c diff = mTargetPos - mPos;
            if (diff.x * diff.x + diff.z * diff.z < 100.0f) {
                setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
            } else {
                setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, 0.8f, 16.0f, m3d::FORWARD_LOOP, FALSE);
            }
        } else if (mState == dPlayerMgr::STATE_SHOW_CATCH) {
            setAnm(HMN_ANM_MENU_THINK1, 30.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
        if (getDemoStartParam()->mEatSpecial != 0) {
            _22A4[7] = 150;
        }
    }
    if (mAnmId == HMN_ANM_MENU_THINK2) {
        mLevelEffect3.create("af_mnp_delicious", &mMdlCallback.mFeelMtx, 1);
    }
}

// .text 0x790C
void dAcPlayer_c::executeDemoStart() {
    if (mAnmId == HMN_ANM_MENU_THINK2) {
        holdSound(SE_Happy);
        mLevelEffect3.create("af_mnp_delicious", &mMdlCallback.mFeelMtx, 0);
        if (_22A4[7] != 0) {
            _22A4[7]--;
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
    }
    if (mPlayerNo != dNet::getMyNo()) {
        if (mAnmId == HMN_ANM_NML_WALK1) {
            mVec3_c diff = mTargetPos - mPos;
            f32 distSq = diff.x * diff.x + diff.z * diff.z;
            if (distSq < 100.0f) {
                followRemoteTransform();
                setAnm(((demoStartParam_s *)&mRemoteActionParams)->mEating != 0 ? HMN_ANM_MENU_THINK2
                                                                                : HMN_ANM_MENU_THINK1,
                       m3d::FORWARD_LOOP);
            } else if (distSq > 25600.0f) {
                mPos.x = mTargetPos.x;
                mPos.z = mTargetPos.z;
            } else {
                sLib::chaseAngle(&mAngle.y.mAngle, dActor_c::targetAngleY(&mPos, &mTargetPos), 0xB61);
                cLib::chasePosXZ(&mPos, mTargetPos, 1.6f);
            }
        } else {
            followRemoteTransform();
        }
    }
}

// .text 0x7A6C
void dAcPlayer_c::setDemoStartParam(demoStartParam_s *param, BOOL eating) {
    param->mEating = eating;
}

// .text 0x7A74
void dAcPlayer_c::initDemoWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        l_talkActorID = (fBaseID_e)-1;
        if (_21B4 != NULL) {
            l_talkActorID = _21B4->mUniqueID;
        }
        dDemoActor_c *talker = dDemo::getTalkActor();
        if (talker != NULL) {
            l_talkActorID = talker->mUniqueID;
        }
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_KEEP_ANIMATION)) {
            setIdleAnm();
        }
        setDemoWaitParam((demoWaitParam_s *)&mRemoteActionParams, mAnmId, mLookYaw, mLookTilt,
                         !(mToolScale.x <= 0.0f));
    } else {
        BOOL sync = FALSE;
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE) && dNet::isSessionActive() &&
            !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
            sync = TRUE;
        }
        if (sync) {
            u16 anm = ((demoWaitParam_s *)&mRemoteActionParams)->mAnmId;
            if (anm == HMN_ANM_NML_WAIT1) {
                setIdleAnm();
            } else {
                setAnm(anm, m3d::FORWARD_LOOP);
            }
        } else {
            setIdleAnm();
        }
    }
}

// .text 0x7BA8
void dAcPlayer_c::endDemoWait(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        l_talkActorID = (fBaseID_e)-1;
    }
    if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_LOOK_AT) || mLookYaw != 0 || mLookTilt != 0 ||
        _22CA == 0 || _22CC == 0) {
        lookAt(NULL, 0x7F, 0x7F);
    }
}

// .text 0x7C3C
void dAcPlayer_c::executeDemoWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        faceTalkActor();
        BOOL toolOut = !(mToolScale.x <= 0.0f);
        u16 anm = mAnmId;
        if (anm == HMN_ANM_NML_WALK1) {
            anm = HMN_ANM_NML_WAIT1;
        }
        setDemoWaitParam((demoWaitParam_s *)&mRemoteActionParams, anm, mLookYaw, mLookTilt, toolOut);
        if (dDemo::isType(dDemo::TYPE_SCENE_START)) {
            if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_FROM_BUS)) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
            }
            dDemo::finishSceneStart();
        }
    } else if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE) && dNet::isSessionActive() &&
               !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
        getRemoteActionParams(&mRemoteActionParams);
        followRemoteTransform();
        demoWaitParam_s *param = (demoWaitParam_s *)&mRemoteActionParams;
        setLookAngles(param->mLookYaw, param->mLookTilt, 0x7F, 0x7F);
    }
}

// .text 0x7D48
void dAcPlayer_c::faceTalkActor() {
    dDemoActor_c *talker = dDemo::getTalkActor();
    if (talker != NULL && !talker->hasDemoFlag8()) {
        mVec3_c *pos = talker->getDemoPosition();
        if (pos != NULL && mAnmId != HMN_ANM_BED_SLEEP_WAIT1) {
            if (mAnmId != HMN_ANM_CHAIR_SITDOWN_WAIT1) {
                mTargetAngle = dActor_c::targetAngleY(&mPos, pos);
                bool reached = sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
                if (reached) {
                    setAnm(HMN_ANM_NML_WAIT1, l_moveParam.mWaitMorph, mAnm.getFrame(), 1.0f, 16.0f, m3d::FORWARD_LOOP,
                           FALSE);
                } else {
                    setAnm(HMN_ANM_NML_WALK1, l_moveParam.mWalkMorph, 0.0f, l_moveParam.mWalkRateMin, 16.0f,
                           m3d::FORWARD_LOOP, FALSE);
                }
            }
            mVec3_c *head = talker->demoHook58();
            if (head != NULL) {
                mVec3_c target(head->x, -5.0f + head->y, head->z);
                lookAt(&target, 0x7F, 0x7F);
            }
        }
    }
}

// .text 0x7EB4
void dAcPlayer_c::setDemoWaitParam(demoWaitParam_s *param, u16 anmId, s16 lookYaw, s16 lookTilt,
                                   BOOL toolOut) {
    param->mAnmId = anmId;
    param->mLookYaw = lookYaw;
    param->mLookTilt = lookTilt;
    param->mToolOut = toolOut;
}

// .text 0x7EC8
void dAcPlayer_c::initEnterDoor() {
    static const f32 l_startFrame = 20.0f;
    static const f32 l_endFrame = 60.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        _205C = mPos;
        _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y.mAngle) / l_startFrame;
        if (mTurnFrames <= 0) {
            mTurnFrames = 1000;
        }
        _22A4[0] = 0;
        mSpeedF = mTargetSpeed = 0.0f;
        daSTR_c *str = dSTR::getAcList()->searchNearest(mPos, FALSE);
        if (str != NULL) {
            _22A4[0] = fn_99_48DC(str);
            if (_22A4[0] == 0) {
                sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
                mSpeedF = mTargetSpeed = 0.2f;
                if (mDoorType == 2) {
                    setAnm(HMN_ANM_DOOR_IN1, m3d::FORWARD_ONCE);
                } else {
                    setAnm(HMN_ANM_NODOOR_IN1, m3d::FORWARD_ONCE);
                }
            }
            dAStr::requestNpcEnter(str);
        }
        moveToTarget();
        setEnterDoorParam((doorParam_s *)&mRemoteActionParams, mDoorType, _22A4[0]);
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, mPlayerNo)) {
            mPos.x = x;
            mPos.z = z;
            mAngle.y = angle;
        }
        doorParam_s *param = (doorParam_s *)&mRemoteActionParams;
        mDoorType = param->mDoorType;
        _22A4[0] = param->mWait;
        mSpeedF = mTargetSpeed = 0.0f;
        daSTR_c *str = dSTR::getAcList()->searchNearest(mPos, FALSE);
        if (str != NULL) {
            _22A4[0] = fn_99_48DC(str);
            if (_22A4[0] == 0) {
                setAnm(mDoorType == 2 ? HMN_ANM_DOOR_IN1 : HMN_ANM_NODOOR_IN1, l_endFrame, 0.0f, 1.0f, 16.0f,
                       m3d::FORWARD_ONCE, TRUE);
            }
            dAStr::requestNpcEnter(str);
        }
    }
}

// .text 0x8100
void dAcPlayer_c::executeEnterDoor() {
    if (mPlayerNo == dNet::getMyNo()) {
        _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        mAngle3D.x = 0;
        mAngle3D.y = _22C2;
        mAngle3D.z = 0;
        if (_22A4[0] != 0) {
            _22A4[0]--;
        }
        if (_22A4[0] == 0) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            mSpeedF = mTargetSpeed = 0.2f;
            if (mDoorType == 2) {
                setAnm(HMN_ANM_DOOR_IN1, m3d::FORWARD_ONCE);
            } else {
                setAnm(HMN_ANM_NODOOR_IN1, m3d::FORWARD_ONCE);
            }
        }
        moveToTarget();
    } else {
        followRemoteTransform();
        if (_22A4[0] != 0) {
            _22A4[0]--;
        }
        if (_22A4[0] == 0) {
            if (mDoorType == 2) {
                setAnm(HMN_ANM_DOOR_IN1, m3d::FORWARD_ONCE);
            } else {
                setAnm(HMN_ANM_NODOOR_IN1, m3d::FORWARD_ONCE);
            }
        }
        if (mAnmId == HMN_ANM_DOOR_IN1) {
            if (mAnm.checkFrame(80.0f)) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
            } else if (mAnm.getFrame() > 90.0f) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
            }
        } else if (mAnmId == HMN_ANM_NODOOR_IN1) {
            if (mAnm.checkFrame(20.0f)) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
            } else if (mAnm.isStop()) {
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
            }
        }
    }
    if (mAnmId == HMN_ANM_DOOR_IN1) {
        checkFootSound(20.0f, 32.0f, 61.0f, 73.0f);
    } else if (mAnmId == HMN_ANM_NODOOR_IN1) {
        checkFootSound(24.0f, 48.0f, 72.0f, 95.0f);
    }
}

// .text 0x831C
void dAcPlayer_c::endEnterDoor(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
}

// .text 0x832C
void dAcPlayer_c::setEnterDoorParam(doorParam_s *param, u8 doorType, u8 wait) {
    param->mDoorType = doorType;
    param->mWait = wait;
}

// .text 0x8338
void dAcPlayer_c::initExitDoor() {
    static const f32 l_angleScale = 0.00024414063f;

    if (mPlayerNo == dNet::getMyNo()) {
        setExitDoorParam((doorParam_s *)&mRemoteActionParams, mDoorType);
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    } else {
        mDoorType = ((doorParam_s *)&mRemoteActionParams)->mDoorType;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        if (mDoorType == 2) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
        }
    }
    if (mDoorType == 2) {
        setAnm(HMN_ANM_DOOR_OUT1, 0.0f, 0.0f, 0.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    } else {
        setAnmNoMorph(HMN_ANM_NODOOR_OUT_WAIT1, m3d::FORWARD_LOOP);
    }
    _22A4[0] = 0;
    daSTR_c *str = dSTR::getAcList()->searchNearest(mPos, FALSE);
    if (str != NULL) {
        _22A4[0] = fn_99_490C(str);
        if (_22A4[0] == 0) {
            if (mDoorType == 2) {
                mAnm.setRate(1.0f);
            } else {
                setAnm(HMN_ANM_NODOOR_OUT1, m3d::FORWARD_ONCE);
            }
        }
        dAStr::requestNpcExit(str);
    }
    mVec3_c pos;
    getPosAtAngle(&pos, mAngle.y.mAngle, -64.0f);
    mObjcPos.set(this, pos, 20.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
    if (mDoorType == 3) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (dNet::isSessionActive()) {
                u8 prev = dScene::getPrevScene();
                if (prev == dScene::SCENE_RM_CHKP || prev == dScene::SCENE_RM_CHKP_NET ||
                    prev == dScene::SCENE_DM_CHKP_MISS) {
                    mPos.x += l_angleScale * mPlayerNo;
                }
            }
        } else if (dScene::getPrevScene() == dScene::SCENE_DM_CHKP_CNNCT) {
            mPos.x += l_angleScale * mPlayerNo;
        }
    }
    if (mDoorType == 2) {
        mPos.z += l_angleScale * mPlayerNo;
    }
}

// .text 0x85BC
void dAcPlayer_c::executeExitDoor() {
    if (mAnmId != HMN_ANM_NML_WAIT1) {
        if (_22A4[0] != 0) {
            _22A4[0]--;
        }
        if (_22A4[0] == 0) {
            if (mDoorType == 2) {
                mAnm.setRate(1.0f);
            } else {
                setAnm(HMN_ANM_NODOOR_OUT1, m3d::FORWARD_ONCE);
            }
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            dDemo::finishSceneStart();
        }
        mObjcPos.mPos = mMdlCallback.mBasePos;
    } else {
        if (mAnmId == HMN_ANM_DOOR_OUT1 && mAnm.getFrame() > 15.0f) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
        }
        if (mAnmId != HMN_ANM_NML_WAIT1) {
            if (mAnm.isStop()) {
                mObjcPos.mPos = mMdlCallback.mBasePos;
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            } else if (mAnm.getFrame() < 2.0f) {
                mVec3_c pos;
                getPosAtAngle(&pos, mAngle.y.mAngle, -64.0f);
                mObjcPos.mPos = pos;
            } else {
                mObjcPos.mPos = mMdlCallback.mBasePos;
            }
        }
    }
    mObjcPos.regist();
    if (mAnmId == HMN_ANM_NODOOR_OUT1) {
        checkFootSound(10.0f, 23.0f, 37.0f, 53.0f);
    } else if (mAnmId == HMN_ANM_DOOR_OUT1) {
        if (mAnm.checkFrame(18.0f) || mAnm.checkFrame(30.0f) || mAnm.checkFrame(50.0f) ||
            mAnm.checkFrame(80.0f) || mAnm.checkFrame(107.0f) || mAnm.checkFrame(115.0f)) {
            startFootSound(-1, 0.0f);
        }
    }
}

// .text 0x881C
void dAcPlayer_c::endExitDoor(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        if (nextState != dPlayerMgr::STATE_BED_HAIR && !dPlayerMgr::canHoldEquippedItem()) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
        }
    } else {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_DISABLED;
    }
    mObjcPos.mType &= ~0x20;
}

// .text 0x88AC
void dAcPlayer_c::setExitDoorParam(doorParam_s *param, u8 doorType) {
    param->mDoorType = doorType;
}

// .text 0x88B4
void dAcPlayer_c::initWalkThroughDoor() {
    if (mPlayerNo == dNet::getMyNo()) {
        _205C = mPos;
        _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        _22A4[0] = 0;
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        mSpeedF = mTargetSpeed = 0.5f;
        moveToTarget();
        setWalkThroughDoorParam((walkThroughDoorParam_s *)&mRemoteActionParams,
                                (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) != 0);
    }
    setAnm(HMN_ANM_DOOR_KNOCK1, m3d::FORWARD_ONCE);
}

// .text 0x8964
void dAcPlayer_c::executeWalkThroughDoor() {
    if (mAnmId == HMN_ANM_DOOR_KNOCK1) {
        if (mAnm.checkFrame(20.0f) || mAnm.checkFrame(35.0f)) {
            if (mDoorTarget >= 0) {
                startSound(mDoorTarget);
            }
            if (mAnm.checkFrame(20.0f)) {
                startRumble(dRumble::PATTERN_DOOR);
            }
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        _22C2 = dActor_c::targetAngleY(&mPos, &mTargetPos);
        mAngle3D.y = _22C2;
        mAngle3D.x = 0;
        mAngle3D.z = 0;
        moveToTarget();
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        if (mAnm.isStop()) {
            if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU) {
                setState(dPlayerMgr::STATE_DEMO_WAIT);
            } else {
                setState(dPlayerMgr::STATE_ENTER_DOOR);
            }
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x8AB8
void dAcPlayer_c::endWalkThroughDoor(int nextState) {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EQUIP_FROM_MENU;
}

// .text 0x8AC8
void dAcPlayer_c::setWalkThroughDoorParam(walkThroughDoorParam_s *param, u8 demoWaitNext) {
    param->mDemoWaitNext = demoWaitNext;
}

// .text 0x8AD0
void dAcPlayer_c::initNetSwing() {
    _22A4[1] = 0;
    if (setAnm(HMN_ANM_NET_SWING1, 4.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE)) {
        mpToolBank->setNetAnm1B6(m3d::FORWARD_ONCE, 8.0f);
    }
    mVec3_c pos;
    getPosAtAngle(&pos, getActionAngle(), 27.0f);
    mObjcPos.set(this, pos, 1.0f, 32.0f, 4, 0x8000003B, 0, mPlayerNo);
    getPosAtAngle(&pos, getActionAngle(), 37.0f);
    mObjcPos2.set(this, pos, 1.0f, 32.0f, 4, 0x8000003B, 0, mPlayerNo);
    if (mPlayerNo == dNet::getMyNo()) {
        startPointerTurn(10.0f);
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
    }
    _2200 = 0.0f;
    mObjcPos.mWeightFunc = objcWeightHeight;
    mObjcPos2.mWeightFunc = objcWeightHeight;
    mObjcPos.mHitFunc = objcHit;
    mObjcPos2.mHitFunc = objcHit;
}

// .text 0x8C40
void dAcPlayer_c::executeNetSwing() {
    if (mAnm.checkFrame(5.0f)) {
        startSound(SE_AmiFuri);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        switch (checkNetSwingHit()) {
        case 0:
            calcPointerTurn();
            checkNetInsect();
            break;
        case 1:
            checkNetInsect();
            if (mInsectLink.mpInsect != NULL) {
                startRumble(dRumble::PATTERN_NET_CATCH);
            } else {
                startRumble(dRumble::PATTERN_NET_MISS);
            }
            break;
        case 2:
            if (_2266 != dItem::ITEM_ID_NONE && _2200 <= 0.0f && mInsectLink.mpInsect == NULL &&
                getNpcToolReaction() == TRUE && g_playActorMng != NULL &&
                fn_116_1CDC(g_playActorMng, &mScriptedActionPos)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_NET_SPAWNED_INSECT;
            }
            if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NET_SPAWNED_INSECT) {
                startRumble(dRumble::PATTERN_NET_CATCH);
                startToolSound(SE_AmiGet);
            } else {
                startRumble(dRumble::PATTERN_NET_MISS);
                startWeatherSound(SE_AmiHitEtc, SE_AmiHitEtc_rain);
            }
            break;
        case 3:
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
            break;
        }
        int result = 1;
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NET_SPAWNED_INSECT) {
            void *insect = fn_116_1D34(g_playActorMng);
            if (insect != NULL) {
                mInsectLink.set(insect);
            } else {
                result = 0;
            }
        }
        if (mInsectLink.mpInsect != NULL) {
            result = fn_116_7DC4(mInsectLink.mpInsect, mPlayerNo);
        }
        if (mAnm.getRate() == 0.0f || mAnm.isStop()) {
            switch (result) {
            case 0:
                return;
            case 1:
                if (mInsectLink.mpInsect != NULL || mpToolBank->getAnmRate() == 0.0f ||
                    mpToolBank->isAnmStop()) {
                    if (mInsectLink.mpInsect != NULL) {
                        dInsectField::removeNpcLink(mInsectLink.mpInsect, &mInsectLink);
                    }
                    setState(dPlayerMgr::STATE_IDLE);
                }
                break;
            case 2:
                setState(dPlayerMgr::STATE_NET_CATCH);
                break;
            }
        }
    } else {
        switch (checkNetSwingHit()) {
        case 0:
            followRemoteTransform();
            break;
        case 2:
            startWeatherSound(SE_AmiHitEtc, SE_AmiHitEtc_rain);
            break;
        case 3:
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
            break;
        }
    }
    mObjcPos.regist();
    mObjcPos2.regist();
}

// .text 0x8F74
void dAcPlayer_c::endNetSwing(int nextState) {
    mObjcPos.mWeightFunc = NULL;
    _2200 = 0.0f;
    mObjcPos2.mWeightFunc = NULL;
    mObjcPos.mHitFunc = NULL;
    mObjcPos2.mHitFunc = NULL;
    mObjcPos.init();
    mObjcPos2.init();
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_NET_SPAWNED_INSECT;
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        _2278 = 0;
        _2266 = dItem::ITEM_ID_NONE;
        _227C = 0;
    } else if (nextState != dPlayerMgr::STATE_IDLE && nextState != dPlayerMgr::STATE_NET_CATCH) {
        mpToolBank->setNetWait(m3d::FORWARD_LOOP, 8.0f);
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x904C
void dAcPlayer_c::getModelOffsetPos(mVec3_c *pos, f32 x, f32 y, f32 z) {
    mMtx_c mtx = mModelMtx;
    mtx.YrotM(mAng(0x238E));
    mMtx_c trans;
    PSMTXTrans(trans, x, y, z);
    PSMTXConcat(mtx, trans, mtx);
    mVec3_c local(mtx._03, mtx._13, mtx._23);
    dWorld::toFieldPosition(pos, &local);
}

// .text 0x916C
void dAcPlayer_c::checkNetInsect() {
    if (mInsectLink.mpInsect != NULL) {
        return;
    }
    if (mAnm.getFrame() < 9.0f) {
        return;
    }
    mVec3_c start;
    getModelOffsetPos(&start, -14.0f, 0.0f, 0.0f);
    mVec3_c end;
    getModelOffsetPos(&end, -15.0f, 0.0f, 0.0f);
    f32 radius;
    int kind = mpToolBank->mItem.getKind();
    radius = 9.0f;
    if (kind == dItem::KIND_SILVER_NET) {
        radius = 9.45f;
    } else if (kind == dItem::KIND_GOLD_NET) {
        radius = 9.9f;
    }
    radius -= fn_116_5B1C();
    if (radius <= 0.0f) {
        radius = 0.1f;
    }
    u32 hit;
    if (dObjc::ins_c::check(&start, &end, radius, &hit) && hit != 0) {
        mInsectLink.set((void *)hit);
        if (mInsectLink.mpInsect != NULL &&
            ((dInsectFieldActor_c *)mInsectLink.mpInsect)->mType == 0xC) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->stop14();
            }
        }
        startToolSound(SE_AmiGet);
    }
}

// .text 0x5F84
int dAcPlayer_c::checkNetSwingHit() {
    if (0.0f == mAnm.getRate()) {
        return 3;
    }
    if (checkNetHit()) {
        return 2;
    }
    mVec3_c tip;
    getModelOffsetPos(&tip, -6.0f, 0.0f, 6.0f);
    mVec3_c start1(mMdlCallback.mRHandPos);
    mVec3_c end1(tip);
    mVec3_c start2;
    getModelOffsetPos(&start2, -15.0f, 0.0f, 15.0f);
    mVec3_c end2(tip);
    dBGCF::lineChk_s chk1;
    dBGCF::lineChk_s chk2;
    BOOL hit = FALSE;
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c start3;
        getModelOffsetPos(&start3, -30.0f, 0.0f, 0.0f);
        mVec3_c end3(tip);
        if (dBGCF::checkLine(&chk1, &end3, &start3, 0x33)) {
            hit = TRUE;
        }
    }
    if (hit || dBGCF::checkLine(&chk1, &end1, &start1, 0x33) || dBGCF::checkLine(&chk2, &end2, &start2, 0x33)) {
        f32 frame = mAnm.getFrame();
        if (frame > 7.0f && 0.0f != mAnm.getRate()) {
            BOOL water = FALSE;
            mVec3_c pos(tip.x, mPos.y, tip.z);
            if (frame >= 16.0f) {
                dBGCF::groundChk_c ground(&pos, 0, 0, 0);
                f32 waterY = ground.mWaterY;
                f32 depth = mPos.y - waterY;
                if (depth <= 16.0f && ground.mWater != BG_WATER_NONE) {
                    if (depth <= 8.0f && ground.mWater == BG_WATER_SEA) {
                        pos.y = waterY;
                        water = TRUE;
                    } else if (frame >= 17.0f) {
                        pos.y = ground.mWaterY;
                        water = TRUE;
                    }
                }
            }
            if (water) {
                startToolSound(SE_AmiHitWater);
                dEffect::create("af_hny_watercolumn_L", &pos, NULL, NULL);
                dEffect::create("af_hny_mizutama_L", &pos, NULL, NULL, wateringParticleCallback, 0);
            } else {
                startWeatherSound(SE_AmiHitEtc, SE_AmiHitEtc_rain);
                if (frame >= 14.0f) {
                    mVec3_c front;
                    calcPosAtAngle(&front, &pos, mAngle.y.mAngle + 0x8000, 10.0f);
                    if (nw4r::math::FAbs(mPos.y - dBGCF::getGroundY(&front, FALSE)) <= 16.0f) {
                        dFgobjManager::stepOnUnit(mPlayerNo, &front);
                    }
                }
            }
            mpToolBank->setNetAnm1AA(m3d::FORWARD_ONCE, 8.0f);
            mAnm.setRate(0.0f);
            return 1;
        }
    }
    if (!mAnm.isStop() && mAnm.checkFrame(mAnm.mFrameMax - 2.0f)) {
        startWeatherSound(SE_AmiHitEtc, SE_AmiHitEtc_rain);
        mpToolBank->setNetAnm1AA(m3d::FORWARD_ONCE, 8.0f);
        mAnm.setRate(0.0f);
        if (mInsectLink.mpInsect == NULL) {
            startRumble(dRumble::PATTERN_NET_MISS);
        }
    }
    return 0;
}

// .text 0x6370
void dAcPlayer_c::stepGetInsectMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play0(10);
            }
            attachDemoActor(this);
            int kind = 0;
            if (mInsectLink.mpInsect != NULL) {
                kind = ((dInsectFieldActor_c *)mInsectLink.mpInsect)->mType;
            }
            setMessageLabel("sys_MESS/SYS_Get_bug");
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
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_CATALOG_COMPLETE) && pd->mCatalog.isComplete(dItem::KIND_INSECT)) {
            mMsgStep = MSG_STEP_QUEUE_COMPLETE;
        } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
            int x;
            int z;
            if (dFgobjManager::findPutSpot(0x10, &x, &z, NULL)) {
                mMsgStep = MSG_STEP_QUEUE_ASK_RELEASE;
            } else {
                mMsgStep = MSG_STEP_QUEUE_LET_GO;
            }
        }
        break;
    }
    case 12:
        switch (mMsgStep) {
        case MSG_STEP_SWAP_INSECT:
            changeStateAfterMsg(dPlayerMgr::STATE_NET_POCKET);
            break;
        case MSG_STEP_RELEASE:
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_INSECT;
            changeStateAfterMsg(dPlayerMgr::STATE_SHOW_CATCH_AT);
            break;
        case MSG_STEP_COMPLETE:
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_INSECT;
            setState(dPlayerMgr::STATE_CATCH_POSE);
            break;
        default:
            changeStateAfterMsg(dPlayerMgr::STATE_NET_POCKET);
            break;
        }
        break;
    }
}

// .text 0x92D4
BOOL dAcPlayer_c::checkNetHit() {
    if (mObjcPos.mHitType != 0 && mAnm.getRate() != 0.0f) {
        if (mObjcPos.isHit(dObjc::TYPE_SNOWMAN) && mAnm.checkFrame(10.0f)) {
            mpToolBank->setNetAnm1AA(m3d::FORWARD_ONCE, 8.0f);
            mAnm.setRate(0.0f);
            return TRUE;
        }
        if (mAnm.getFrame() >= 11.0f) {
            dObjc::acpos_c *objc = &mObjcPos;
            f32 frame = 0.0f;
            f32 dy = mPos.y - objc->getPos()->y;
            if (dy < 7.5f) {
                frame = 11.0f;
            } else if (dy >= 7.5f && dy < 15.0f) {
                frame = 12.0f;
            } else if (dy >= 15.0f && dy < 25.0f) {
                frame = 13.0f;
            } else if (dy >= 25.0f && dy < 35.0f) {
                frame = 14.0f;
            } else if (dy >= 35.0f && dy < 40.0f) {
                frame = 15.0f;
            }
            if (_2200 > 0.0f && _2200 < 25.0f) {
                frame += 1.0f;
                if (frame > 15.0f) {
                    frame = 15.0f;
                }
            }
            if (mAnm.checkFrame(frame)) {
                mpToolBank->setNetAnm1AA(m3d::FORWARD_ONCE, 8.0f);
                mAnm.setRate(0.0f);
                return TRUE;
            }
        }
    }
    if (mObjcPos2.mHitType != 0 && mAnm.getRate() != 0.0f && mAnm.getFrame() >= 12.0f) {
        dObjc::acpos_c *objc = &mObjcPos2;
        f32 frame = 0.0f;
        f32 dy = mPos.y - objc->getPos()->y;
        if (dy < 7.5f) {
            frame = 12.0f;
        } else if (dy >= 7.5f && dy < 15.0f) {
            frame = 13.0f;
        } else if (dy >= 15.0f && dy < 35.0f) {
            frame = 14.0f;
        } else if (dy >= 35.0f && dy < 40.0f) {
            frame = 15.0f;
        }
        if (_2200 > 0.0f && _2200 < 25.0f) {
            frame += 1.0f;
            if (frame > 15.0f) {
                frame = 15.0f;
            }
        }
        if (mAnm.checkFrame(frame)) {
            mpToolBank->setNetAnm1AA(m3d::FORWARD_ONCE, 8.0f);
            mAnm.setRate(0.0f);
            return TRUE;
        }
    }
    _2200 = 0.0f;
    return FALSE;
}
