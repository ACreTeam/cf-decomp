// dAcPlayer_c: run and change-equipment states (.text 0x53A4..0x6E50; the string-literal functions
// of demo.inc and tool.inc up to createStampEffect are placed at the end of this section).
// Included by d_a_player.cpp.

// .text 0x53A4
void dAcPlayer_c::initRun() {
    if (mPlayerNo == dNet::getMyNo()) {
        getRunParam()->set(_2052);
        setAnm(HMN_ANM_NML_SLIP1, l_moveParam.mRunStartMorph, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        _2050 = 0;
        _22A4[3] = mSpeedF * l_moveParam.mRunStopFrames;
        _21B8 = l_moveParam.mRunStopAccel;
        mTargetSpeed = 0.0f;
        _22C2 = _2052 + 0x8000;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    } else {
        if (_21C0[17] <= 0.0f) {
            _21C0[17] = l_moveParam.mMaxSpeed;
        }
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, mPlayerNo)) {
            mTargetPos.set(x, mPos.y, z);
            mTargetPos.y = dBGCF::getGroundY(&mTargetPos, FALSE);
            _22C2 = _2052 = dActor_c::targetAngleY(&mPos, &mTargetPos);
            mVec3_c diff = mPos - mTargetPos;
            f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
            if (dist > l_moveParam.mMaxSpeed) {
                dist = l_moveParam.mMaxSpeed;
            }
            mTargetSpeed = dist;
            setAnm(HMN_ANM_NML_DASH1, l_moveParam.mWalkMorph, _21C0[0], l_moveParam.mWalkRateMin, 16.0f,
                   m3d::FORWARD_LOOP, FALSE);
            setNetWalkAnm();
        }
    }
}

// .text 0x5564
void dAcPlayer_c::executeRun() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
                _22D8[0] = 1;
                startWeatherSound(SE_SlipTurn, SE_SlipTurn_rain);
            } else {
                startSound(SE_SlipTurn);
            }
        }
        mAngle3D.set(0, _22C2, 0);
        bool reached = sLib::chaseAngle(&mAngle.y.mAngle, _2052, l_moveParam.m30);
        if (reached && countDown(&_22A4[3]) == 0) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        if (_22A4[1] != 0) {
            _22A4[1]--;
        }
        if (_22A4[2] != 0) {
            _22A4[2]--;
        }
        s16 angle;
        f32 distSq = getRemoteTargetDistSq(&angle, 2);
        if (isMovingToTarget(distSq, _2052)) {
            f32 dist = nw4r::math::FSqrt(distSq);
            f32 speed = dist;
            if (dist > _21C0[17]) {
                speed = _21C0[17];
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
        updateRemoteMove(angle, TRUE);
        calcRunFootEffects();
    }
}

// .text 0x57A8
void dAcPlayer_c::runParam_c::set(s16 angle) {
    mAngle = angle;
}

// .text 0x57B0
u16 dAcPlayer_c::countDown(u16 *timer) {
    if (*timer != 0) {
        (*timer)--;
    }
    return *timer;
}

// .text 0x57CC
void dAcPlayer_c::initChangeEquipment() {
    setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
    mTargetAngle = 0;
    startChangeEquip();
}

// .text 0x5810
void dAcPlayer_c::executeChangeEquipment() {
    if (mPlayerNo == dNet::getMyNo()) {
        executeChangeEquipMine(dPlayerMgr::STATE_DEMO_START, HMN_ANM_MENU_CHANGE1, HMN_ANM_MENU_THINK1);
    } else {
        executeChangeEquipRemote(HMN_ANM_MENU_CHANGE1, HMN_ANM_MENU_THINK1);
    }
}

// .text 0x514C
void dAcPlayer_c::executeChangeEquipMine(int nextState, int changeAnm, int waitAnm) {
    switch (mStep) {
    case 8:
        if (mAnmId == changeAnm && mAnm.isStop()) {
            setAnm(waitAnm, m3d::FORWARD_LOOP);
        }
        if (mEquipStep == EQUIP_STEP_LOADED) {
            if (mAnmId == changeAnm) {
                mAnm.setFrame(0.0f);
            } else {
                setAnm(changeAnm, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            }
            mTargetAngle = 0;
            mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / 6.0f;
            if (mTurnFrames <= 0) {
                mTurnFrames = 1000;
            }
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            mStep = 9;
            playWearSound();
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        }
        break;
    case 9:
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
        if (mAnm.checkFrame(38.0f)) {
            createChangeEffect();
        }
        if (mAnm.isStop()) {
            setState(nextState);
        } else if (mAnm.checkFrame(24.0f) && mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        }
        break;
    default:
        setState(nextState);
        break;
    }
}

// .text 0x5870
void dAcPlayer_c::endChangeEquipment(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mEquipStep == EQUIP_STEP_LOADED) {
            mEquipStep = EQUIP_STEP_APPLIED;
        } else if (mEquipStep == EQUIP_STEP_LOAD) {
            mEquipStep = EQUIP_STEP_LOAD_INIT;
        }
    }
}

// .text 0x58D0
void dAcPlayer_c::executeChangeEquipRemote(int changeAnm, int waitAnm) {
    switch (mStep) {
    case 8:
        if (mAnmId == changeAnm && mAnm.isStop()) {
            setAnm(waitAnm, m3d::FORWARD_LOOP);
        }
        if (mEquipStep == EQUIP_STEP_LOADED) {
            if (mAnmId == changeAnm) {
                mAnm.setFrame(0.0f);
            } else {
                setAnm(changeAnm, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            }
            mTargetAngle = 0;
            mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y) / 6.0f;
            if (mTurnFrames <= 0) {
                mTurnFrames = 1000;
            }
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            mStep = 9;
            playWearSound();
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        }
        break;
    case 9:
        if (mAnmId == waitAnm) {
            break;
        }
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
        if (mAnm.checkFrame(38.0f)) {
            createChangeEffect();
        }
        if (mAnm.isStop()) {
            setAnm(waitAnm, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(24.0f)) {
            if (mEquipStep == EQUIP_STEP_LOADED) {
                mEquipStep = EQUIP_STEP_APPLIED;
            }
        } else if (mAnm.getFrame() > 24.0f) {
            if (mEquipStep == EQUIP_STEP_LOADED) {
                mEquipStep = EQUIP_STEP_APPLIED;
            }
        }
        break;
    default:
        setAnm(waitAnm, m3d::FORWARD_LOOP);
        break;
    }
}

// .text 0x5B0C
void dAcPlayer_c::startChangeEquip() {
    mStep = 8;
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        getEquipParam()->set(mEquipFlags, mStep);
    } else {
        dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (!dPlayerMngr_c::isEquipChecked(mPlayerNo)) {
            u16 flags = getEquipParam()->mEquipFlags;
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
        mEquipStep = EQUIP_STEP_LOAD;
        mStep = getEquipParam()->mStep;
    }
}

// .text 0x5C20
void dAcPlayer_c::createChangeEffect() {
    static const char *l_changeEffects[] = {
        "af_ply_change_nml", "af_ply_change_water00", "af_ply_change_water00", "af_ply_change_snow00",
        "af_ply_change_sand",
    };
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_OUTDOOR)) {
        mAng3_c angle(0, 0, 0);
        dPlayerMgr::createSurfaceEffects(l_changeEffects, &mPos, &angle, "af_ply_change_water01",
                                         "af_ply_change_snow01");
    }
}

// .text 0x5C90
void dAcPlayer_c::playWearSound() {
    dItem::Item shirt;
    dItem::Item hat;
    if (mEquipFlags & dPlayerMgr::EQUIPMENT_CHANGE_CLOTH) {
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (pd != NULL) {
            shirt = pd->mEquipment.mShirt;
        }
    }
    if ((mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_MASK_2) || (mEquipFlags & dPlayerMgr::EQUIPMENT_HAT_FULL_MASK)) {
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
        if (pd != NULL) {
            hat = pd->mEquipment.mHat;
        }
    }
    startSound(dPlayerMgr::getWearSound(&shirt, &hat));
}

// .text 0x5D30
void dAcPlayer_c::equipParam_c::set(u16 equipFlags, u8 step) {
    mEquipFlags = equipFlags;
    mStep = step;
}
