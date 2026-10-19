// dAcPlayer_c: item actions (continued).
void dAcPlayer_c::executePickUpItem() {
    if (mAnmId != HMN_ANM_PICKUP1) {
        return;
    }
    f32 frame = mAnm.getFrame();
    f32 scale = 1.0f;
    if (frame >= 23.0f && frame < 51.0f) {
        scale = 1.0f - 0.05f * (frame - 23.0f);
        if (scale < 0.0f) {
            scale = 0.0f;
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        updatePickUpItemMine(scale, frame);
    } else {
        updatePickUpItemRemote(scale, frame);
    }
    if (mAnm.checkFrame(40.0f)) {
        startSound(SE_gasagoso_return);
    }
}

// .text 0x133C0
void dAcPlayer_c::endPickUpItem(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        takeFieldItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
}

// .text 0x1341C
void dAcPlayer_c::initPickUpItemPos() {
    static const f32 l_itemY = 8.0f;

    dFgDraw::getItemPosition(&mItemPos, _218C, mTargetUnitX, mTargetUnitZ, FALSE);
    f32 x, y, z;
    getPickUpItemOffset(x, y, z);
    mMtx_c mtx;
    getRootMtx(&mtx);
    mMtx_c trans;
    trans.trans(x, y, z);
    mtx.concat(trans);
    mShowPos.x = mtx.m[0][3];
    mShowPos.y = mtx.m[1][3];
    mShowPos.z = mtx.m[2][3];
    dWorld::toFieldPosition(&mShowPos, &mShowPos);
    mVec3_c diff = mShowPos - mItemPos;
    _21D0[0] = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / l_itemY;
    _21D0[1] = nw4r::math::FAbs(mItemPos.y - mShowPos.y) / l_itemY;
}

// .text 0x13548
void dAcPlayer_c::setItemPosToHand() {
    mItemPos.set(mMdlCallback.mHandPos.x, mMdlCallback.mHandPos.y, mMdlCallback.mHandPos.z);
    addModelOffset(&mItemPos, -3.0f, -2.0f, -2.0f);
}

// .text 0x1357C
void dAcPlayer_c::updatePickUpItemMine(f32 scale, f32 frame) {
    if (mAnm.isStop()) {
        if (mTaskKind == FG_MNG_KIND_PICK_UP_FULL) {
            setState(dPlayerMgr::STATE_POCKET_FULL);
        } else {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnm.checkFrame(12.0f)) {
        takeFieldItem(mTaskKind == FG_MNG_KIND_PICK_UP_FULL);
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame > 12.0f && frame < 20.0f) {
        cLib::chasePosXZ(&mItemPos, mShowPos, _21D0[0]);
        sLib::chase(&mItemPos.y, mShowPos.y, _21D0[1]);
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame >= 20.0f && frame < 51.0f) {
        setItemPosToHand();
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    }
}

// .text 0x13708
void dAcPlayer_c::updatePickUpItemRemote(f32 scale, f32 frame) {
    static const f32 l_warpDistSq = 1024.0f;

    followRemoteTransform();
    if (mAnm.isStop()) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
            setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    } else if (mAnm.checkFrame(12.0f)) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
            takeFieldItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame > 12.0f && frame < 20.0f) {
        if (mShowPos.xzDistSqTo(mMdlCallback.mHandPos) > l_warpDistSq) {
            cLib::chasePosXZ(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
            sLib::chase(&mItemPos.y, mMdlCallback.mHandPos.y, _21D0[1]);
        } else {
            cLib::chasePosXZ(&mItemPos, mShowPos, _21D0[0]);
            sLib::chase(&mItemPos.y, mShowPos.y, _21D0[1]);
        }
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame >= 20.0f && frame < 51.0f) {
        setItemPosToHand();
        mVec3_c itemScale(scale, scale, scale);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    }
}

// .text 0x13900
void dAcPlayer_c::takeFieldItem(BOOL release) {
    if (mState == dPlayerMgr::STATE_PULL_WEED) {
        BOOL notClover = TRUE;
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(mTargetUnitX, mTargetUnitZ, 0);
            if (item != NULL) {
                if (*item == dItem::FG_CLOVER) {
                    notClover = FALSE;
                }
                dFgobjManager::playGrassEffect(mTargetUnitX, mTargetUnitZ, item->getId(), &mAngle, 0);
            }
        }
        if (notClover) {
            startSound(SE_zassou_nuku);
        } else {
            startSound(SE_PullOutGrass);
        }
        startRumble(dRumble::PATTERN_PULL_WEED);
    } else {
        int kind = _2188.getKind();
        if (kind == dItem::KIND_FLOWER) {
            if (mState == dPlayerMgr::STATE_PICK_UP_ITEM_MSG) {
                startSound(SE_PullOutGrass);
                dFgobjManager::playGrassEffect(mTargetUnitX, mTargetUnitZ, _218C, NULL, 0);
            } else {
                startSound(SE_PullOutFlower);
            }
        } else if (kind == dItem::KIND_MUSHROOM) {
            startSound(SE_PullOutKinoko);
        } else {
            startSound(SE_PickUpGoods);
        }
    }
    if (release) {
        fgMngTask_releaseAt(mTargetUnitX, mTargetUnitZ, mPickUpKind);
    } else {
        fgMngTask_procAt(mTargetUnitX, mTargetUnitZ, mPickUpKind);
    }
}

// .text 0x13A70
void dAcPlayer_c::pickUpParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 full) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mFull = full;
}

// .text 0x13A84
void dAcPlayer_c::initPickUpItemMsg() {
    mTargetAngle = 0;
    setAnm(HMN_ANM_PICKUP_GET1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1AC(m3d::FORWARD_ONCE, 8.0f);
        mpToolBank->setAnmFrame(mAnm.getFrame());
    }
    if (mPlayerNo == dNet::getMyNo()) {
        BOOL full = FALSE;
        mStep = 10;
        mMsgStep = MSG_STEP_NONE;
        setTargetUnitItem(0);
        if (mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL || mTaskKind == FG_MNG_KIND_PICK_UP) {
            putInPocket(&_2188);
        } else if (mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_FULL) {
            full = TRUE;
        }
        getPickUpMsgParam()->set(mActionItem.getId(), mTargetUnitX, mTargetUnitZ, full);
    } else {
        if (getPickUpMsgParam()->mFull != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        } else {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_POCKETS_FULL;
        }
        mTargetUnitZ = getPickUpMsgParam()->mUnitZ;
        mTargetUnitX = getPickUpMsgParam()->mUnitX;
        mActionItem = getPickUpMsgParam()->mItem;
        _218C = getPickUpMsgParam()->mItem;
        mPickUpKind = 0;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    dFgDraw::getItemPosition(&mItemPos, _218C, mTargetUnitX, mTargetUnitZ, FALSE);
}

// .text 0x13C1C
void dAcPlayer_c::executePickUpItemMsg() {
    f32 frame = mAnm.getFrame();
    if (mAnmId == HMN_ANM_PICKUP_GET1 && mAnm.checkFrame(65.0f)) {
        createToolGlitter(&mActionItem);
    }
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        if (mAnm.checkFrame(14.0f)) {
            takeFieldItem(mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_FULL);
            dCamera_c *camera = dCamera_c::sInstance;
            if (camera != NULL) {
                camera->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
            }
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->mute0();
            }
        }
        updateLiftItem(frame);
        if (frame > 47.0f) {
            stepPickUpMsg();
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mTargetPos.set(x, mPos.y, z);
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 2.0f);
        }
        if (mAnm.checkFrame(14.0f) && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
            takeFieldItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        updateLiftItem(frame);
    }
}

// .text 0x13DC8
void dAcPlayer_c::endPickUpItemMsg(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop0(0xB, 0x1A);
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        takeFieldItem((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) != 0);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mStep = 0x22;
}

// .text 0x13E64
void dAcPlayer_c::setLiftItemPos(BOOL chase) {
    int kind = mActionItem.getKind();
    BOOL lifted = kind == dItem::KIND_MUSH_FTR || mActionItem.isSame(dItem::Item(dItem::ITEM_IDX_LUCKY_CLOVER));
    if (lifted) {
        mMtx_c mtx = mMdlCallback.mHandMtx;
        mMtx_c trans;
        trans.trans(5.0f, 0.0f, 0.0f);
        mtx.concat(trans);
        mVec3_c pos(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
        dWorld::toFieldPosition(&pos, &pos);
        if (chase) {
            cLib::chasePos(&mItemPos, pos, 3.5f);
            mVec3_c itemScale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
        } else {
            mVec3_c itemScale(1.0f, 1.0f, 1.0f);
            setFgObjItem(mActionItem.getId(), &pos, &itemScale);
        }
    } else {
        mVec3_c itemScale(1.0f, 1.0f, 1.0f);
        setFgObjItem(mActionItem.getId(), &mMdlCallback.mHandPos, &itemScale);
    }
}

// .text 0x14024
void dAcPlayer_c::updateLiftItem(f32 frame) {
    if (mAnm.isStop()) {
        setLiftItemPos(FALSE);
    } else if (mAnm.checkFrame(14.0f)) {
        mVec3_c itemScale(1.0f, 1.0f, 1.0f);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame > 14.0f && frame < 22.0f) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 2000);
        cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, 2.0f);
        mVec3_c itemScale(1.0f, 1.0f, 1.0f);
        setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
    } else if (frame >= 22.0f && frame < 50.0f) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 2000);
        setLiftItemPos(TRUE);
    } else if (frame >= 50.0f) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 2000);
        setLiftItemPos(FALSE);
    }
}

// .text 0x14194
void dAcPlayer_c::stepPickUpMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play0(10);
            }
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            int kind = mActionItem.getKind();
            int code = MSG_FOUND_CLOVER;
            if (kind == dItem::KIND_GOLD_SCOOP) {
                code = MSG_GOT_GOLD_SCOOP;
            } else if (kind == dItem::KIND_GOLD_PACHINKO) {
                code = MSG_GOT_GOLD_PACHINKO;
            } else if (kind == dItem::KIND_MUSH_FTR) {
                code = MSG_GOT_MUSHROOM_FTR;
            }
            setMessageCode(code);
            getController()->requestStart(0);
            if (code == 0x2F) {
                getController()->setItemName(0, _2188);
            }
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
        if (mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_FULL) {
            mMsgStep = MSG_STEP_QUEUE_ASK_DROP;
        }
        break;
    }
    case 12:
        if (mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_FULL) {
            if (mMsgStep == MSG_STEP_SWAP) {
                changeStateAfterMsg(dPlayerMgr::STATE_SHOW_PICKED_ITEM);
            } else if (mMsgStep == MSG_STEP_NONE) {
                changeStateAfterMsg(dPlayerMgr::STATE_SWAP_ITEM);
            }
        } else if (mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL || mTaskKind == FG_MNG_KIND_PICK_UP) {
            changeStateAfterMsg(dPlayerMgr::STATE_SHOW_PICKED_ITEM);
        } else {
            changeStateAfterMsg(dPlayerMgr::STATE_IDLE);
        }
        break;
    }
}

// .text 0x120D4
void dAcPlayer_c::executePitfallFall() {
    sLib::addCalc2(&mShadowAlpha, 0.0f, 0.05f, 0.05f);
    sLib::chase(&mPos.y, mTargetPos.y, 1.5f);
    if (mPlayerNo == dNet::getMyNo()) {
        switch (mAnmId) {
        case HMN_ANM_PITFALL_FALL1:
            if (_22A4[7] != 0) {
                _22A4[7]--;
            }
            bool atTarget = cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
            if (atTarget && _22A4[7] == 0) {
                setAnmNoMorph(HMN_ANM_PITFALL_FALL2, m3d::FORWARD_ONCE);
                startSound(SE_otoshiana_hamaru);
                if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
                    mpToolBank->setNetAnm1B0(m3d::FORWARD_ONCE, 8.0f);
                    mpToolBank->setAnmFrame(mAnm.getFrame());
                }
                dFgobjManager::requestPitfallEnter(mPlayerNo, mTargetUnitX, mTargetUnitZ);
            }
            break;
        case HMN_ANM_PITFALL_FALL2:
            if (mAnm.isStop()) {
                setState(dPlayerMgr::STATE_PITFALL_STRUGGLE);
            } else if (mAnm.checkFrame(20.0f)) {
                createGroundEffect("af_ply_pitfall_sand", "af_ply_pitfall_nml");
            }
            break;
        default:
            setState(dPlayerMgr::STATE_IDLE);
            break;
        }
    } else {
        switch (mAnmId) {
        case HMN_ANM_PITFALL_FALL3:
            break;
        default:
            break;
        case HMN_ANM_PITFALL_FALL1:
            if (_22A4[7] != 0) {
                _22A4[7]--;
            }
            bool atTarget = cLib::chasePosXZ(&mPos, mTargetPos, _21D0[0]);
            if (atTarget && _22A4[7] == 0) {
                setAnmNoMorph(HMN_ANM_PITFALL_FALL2, m3d::FORWARD_ONCE);
                startSound(SE_otoshiana_hamaru);
                if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
                    mpToolBank->setNetAnm1B0(m3d::FORWARD_ONCE, 8.0f);
                    mpToolBank->setAnmFrame(mAnm.getFrame());
                }
                dFgobjManager::requestPitfallEnter(mPlayerNo, mTargetUnitX, mTargetUnitZ);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            break;
        case HMN_ANM_PITFALL_FALL2:
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_PITFALL_FALL3, m3d::FORWARD_LOOP);
            } else if (mAnm.checkFrame(20.0f)) {
                createGroundEffect("af_ply_pitfall_sand", "af_ply_pitfall_nml");
            }
            break;
        }
    }
}

// .text 0x12390
void dAcPlayer_c::executePitfallClimbOut() {
    sLib::addCalc2(&mShadowScale, 1.0f, 0.12f, 0.1f);
    sLib::addCalc2(&mShadowAlpha, 1.0f, 0.08f, 0.1f);
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.checkFrame(1.0f)) {
            dFgobjManager::requestPitfallExit(mPlayerNo, mTargetUnitX, mTargetUnitZ);
            createGroundEffect("af_ply_pitout_sand", "af_ply_pitout_nml");
        } else if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.checkFrame(1.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                dFgobjManager::requestPitfallExit(mPlayerNo, mTargetUnitX, mTargetUnitZ);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
                createGroundEffect("af_ply_pitout_sand", "af_ply_pitout_nml");
            }
        } else if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x14384
void dAcPlayer_c::pickUpMsgParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 full) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mFull = full;
}

// .text 0x14398
void dAcPlayer_c::initShowPickedItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        getShowPickedItemParam()->set(mActionItem.getId(), mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL ||
                                                               mTaskKind == FG_MNG_KIND_PICK_UP_FULL);
    } else {
        mActionItem = getShowPickedItemParam()->mItem;
    }
    setAnm(HMN_ANM_PICKUP_GET2, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x14438
void dAcPlayer_c::executeShowPickedItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        executeShowItem(HMN_ANM_PICKUP_GET2,
                        mTaskKind == FG_MNG_KIND_PICK_UP_SPECIAL_FULL || mTaskKind == FG_MNG_KIND_PICK_UP_FULL, FALSE);
    } else {
        executeShowItem(HMN_ANM_PICKUP_GET2, getShowPickedItemParam()->mFull, FALSE);
    }
}

// .text 0x144B4
void dAcPlayer_c::endShowPickedItem(int nextState) {}

// .text 0x144B8
void dAcPlayer_c::executeShowItem(int anmId, BOOL full, BOOL toolAfter) {
    if (mAnmId != anmId) {
        return;
    }
    mVec3_c scale(1.0f, 1.0f, 1.0f);
    if (mAnm.getFrame() < 18.0f) {
        f32 s = 1.0f - 0.065f * mAnm.getFrame();
        if (s < 0.0f) {
            s = 0.0f;
        }
        scale.set(s, s, s);
    } else {
        scale.set(0.0f, 0.0f, 0.0f);
    }
    setFgObjItem(mActionItem.getId(), &mMdlCallback.mHandPos, &scale);
    if (mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (full) {
                setState(dPlayerMgr::STATE_POCKET_FULL);
            } else if (toolAfter) {
                setState(mpToolBank->getToolType() == HMN_TOOL_UMBRELLA ? dPlayerMgr::STATE_OPEN_UMBRELLA
                                                                        : dPlayerMgr::STATE_TAKE_OUT_TOOL);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        } else if (full) {
            setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    } else if (mAnm.checkFrame(15.0f)) {
        startSound(SE_gasagoso_return);
    }
}

// .text 0x14658
void dAcPlayer_c::showPickedItemParam_c::set(u16 item, u8 full) {
    mItem = item;
    mFull = full;
}

// .text 0x14664
void dAcPlayer_c::initPullWeed() {
    setAnm(HMN_ANM_PICKUP_WEEDS1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1AE(m3d::FORWARD_ONCE, 8.0f);
        mpToolBank->setAnmFrame(mAnm.getFrame());
    }
    if (mPlayerNo == dNet::getMyNo()) {
        getPullWeedParam()->set(mTargetUnitX, mTargetUnitZ);
    } else {
        mTargetUnitZ = getPullWeedParam()->mUnitZ;
        mTargetUnitX = getPullWeedParam()->mUnitX;
        mPickUpKind = 0;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
}

// .text 0x14748
void dAcPlayer_c::executePullWeed() {
    static const f32 l_pullFrame = 28.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(l_pullFrame)) {
            if (mTaskKind == FG_MNG_KIND_PULL) {
                takeFieldItem(FALSE);
            }
        } else if (mAnm.getFrame() > 42.0f) {
            if (checkMoveOrAction()) {
                return;
            }
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(l_pullFrame) && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
            takeFieldItem(FALSE);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
    }
}

// .text 0x14884
void dAcPlayer_c::endPullWeed(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        takeFieldItem(FALSE);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
}

// .text 0x148E0
void dAcPlayer_c::pullWeedParam_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x148EC
void dAcPlayer_c::initPitfallTrigger() {}

// .text 0x148F0
void dAcPlayer_c::executePitfallTrigger() {
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

// .text 0x14964
void dAcPlayer_c::endPitfallTrigger(int nextState) {}

// .text 0x14968
void dAcPlayer_c::initPitfallFall() {
    static const f32 l_startFrame = 10.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        getPitfallFallParam()->set(mTargetUnitX, mTargetUnitZ);
        dFdBase_c::getUnitCenterPos(&mTargetPos, mTargetUnitX, mTargetUnitZ);
        mTargetPos.y = dBGCF::getUnitBaseY(&mTargetPos);
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_18;
        }
        startRumble(dRumble::PATTERN_PITFALL_FALL);
    } else {
        mTargetUnitX = getPitfallFallParam()->mUnitX;
        mTargetUnitZ = getPitfallFallParam()->mUnitZ;
        dFdBase_c::getUnitCenterPos(&mTargetPos, mTargetUnitX, mTargetUnitZ);
        mTargetPos.y = dBGCF::getUnitBaseY(&mTargetPos);
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    _21D0[0] = 0.0f;
    mVec3_c diff = mTargetPos - mPos;
    f32 speed = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / l_startFrame;
    _21D0[0] = speed;
    cLib::chasePosXZ(&mPos, mTargetPos, speed);
    sLib::chase(&mPos.y, mTargetPos.y, 1.5f);
    setAnm(HMN_ANM_PITFALL_FALL1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1AF(m3d::FORWARD_LOOP, 8.0f);
        mpToolBank->setAnmFrame(mAnm.getFrame());
    }
    startSound(SE_OtoshianaGuratsuku);
    u32 flags = mActorFlags;
    _22A4[7] = 24;
    mActorFlags = flags | dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x14B14
void dAcPlayer_c::endPitfallFall(int nextState) {
    int myNo = dNet::getMyNo();
    if (mPlayerNo != myNo && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        dFgobjManager::requestPitfallEnter(mPlayerNo, mTargetUnitX, mTargetUnitZ);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
    mShadowScale = 0.0f;
    mShadowAlpha = 0.0f;
}

// .text 0x14B90
void dAcPlayer_c::pitfallFallParam_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x14B9C
void dAcPlayer_c::initPitfallStruggle() {
    setAnmNoMorph(HMN_ANM_PITFALL_FALL3, m3d::FORWARD_LOOP);
    startSound(SE_OtoshianaMogaku);
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mpToolBank->getToolType() == HMN_TOOL_NET) {
        mpToolBank->setNetAnm1B1(m3d::FORWARD_LOOP, 8.0f);
        mpToolBank->setAnmFrame(mAnm.getFrame());
    }
    if (mPlayerNo == dNet::getMyNo()) {
        _21D0[7] = 600.0f;
        _21D0[4] = 0.0f;
        _21D0[5] = 0.0f;
        _21D0[2] = 0.0f;
        _21D0[3] = 0.0f;
        _21D0[6] = 1.0f;
        _22A4[7] = 0;
        _21D0[8] = 0.0f;
        startRumble(dRumble::PATTERN_PITFALL_STRUGGLE);
        getPitfallStruggleParam()->set(_21D0[6]);
    } else {
        mAnm.setRate(getPitfallStruggleParam()->mRate);
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x14C9C
void dAcPlayer_c::executePitfallStruggle() {
    syncToolAnm();
    if (mAnm.checkFrame(0.0f)) {
        startSound(SE_OtoshianaMogaku);
        startRumble(dRumble::PATTERN_PITFALL_STRUGGLE);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (_22A4[7] != 0) {
            _22A4[7]--;
        }
        f32 power = 0.0f;
        int mode = dPad::getInputMode();
        f32 stick = _22F0;
        if (stick) {
            s16 angle = mMoveAngle;
            f32 x = stick * mAng(angle).sin();
            f32 z = stick * mAng(angle).cos();
            if (mode == 1) {
                _21D0[4] = 0.0f;
                _21D0[5] = 0.0f;
                f32 dx = _21D0[2] - x;
                f32 dz = _21D0[3] - z;
                if (dx != 0.0f || dz != 0.0f) {
                    power += 24.0f * nw4r::math::FSqrt(dx * dx + dz * dz);
                }
                _21D0[2] = x;
                _21D0[3] = z;
            } else if (mode == 0) {
                _21D0[2] = 0.0f;
                _21D0[3] = 0.0f;
                f32 dx = _21D0[4] - x;
                f32 dz = _21D0[5] - z;
                if (dx != 0.0f || dz != 0.0f) {
                    power += 32.0f * nw4r::math::FSqrt(dx * dx + dz * dz);
                }
                _21D0[4] = x;
                _21D0[5] = z;
            }
        } else {
            _21D0[4] = 0.0f;
            _21D0[5] = 0.0f;
            _21D0[2] = 0.0f;
            _21D0[3] = 0.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_1)) {
            power += 24.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_2)) {
            power += 24.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_A)) {
            power += 24.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_B)) {
            power += 24.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_FS_C)) {
            power += 24.0f;
        }
        if (mPad::getCore()->downTrigger(WPAD_BUTTON_FS_Z)) {
            power += 24.0f;
        }
        if (dPad::isShakeHeld()) {
            power += 10.0f;
        }
        _21D0[8] += power;
        if (power > 0.0f && _22A4[7] == 0) {
            _22A4[7] = 3;
            _21D0[7] -= power;
        } else {
            _21D0[7] -= 1.0f;
        }
        f32 rate = 1.0f;
        f32 step = 0.02f;
        if (_21D0[8] > 23.0f) {
            _21D0[8] = 0.0f;
            rate = 2.4f;
            step = 1.0f;
        }
        if (_21D0[7] <= 120.0f) {
            rate = 2.4f;
            step = 0.01f;
        }
        sLib::chase(&_21D0[6], rate, step);
        mAnm.setRate(_21D0[6]);
        getPitfallStruggleParam()->set(_21D0[6]);
        if (_21D0[7] <= 0.0f || (mActorFlags2 & 4)) {
            setState(dPlayerMgr::STATE_PITFALL_CLIMB_OUT);
        }
    } else {
        followRemoteTransform();
        getRemoteActionParams(&mRemoteActionParams);
        mAnm.setRate(getPitfallStruggleParam()->mRate);
    }
}

// .text 0x15104
void dAcPlayer_c::endPitfallStruggle(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x15114
void dAcPlayer_c::pitfallStruggleParam_c::set(f32 rate) {
    mRate = rate;
}

// .text 0x1511C
void dAcPlayer_c::initPitfallExit() {
    if (mPlayerNo == dNet::getMyNo()) {
        getPitfallExitParam()->set(mTargetUnitX, mTargetUnitZ);
        dRumble::stop();
        startRumble(dRumble::PATTERN_PITFALL_EXIT);
    } else {
        mTargetUnitZ = getPitfallExitParam()->mUnitZ;
        mTargetUnitX = getPitfallExitParam()->mUnitX;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    int anmId = HMN_ANM_PITFALL_OUTA1;
    switch (mpToolBank->getToolType()) {
    case HMN_TOOL_NET:
        anmId = HMN_ANM_PITFALL_OUTB1;
        mpToolBank->setNetAnm1B2(m3d::FORWARD_ONCE, 8.0f);
        break;
    case HMN_TOOL_ROD:
    case HMN_TOOL_UMBRELLA:
        anmId = HMN_ANM_PITFALL_OUTB1;
        break;
    }
    setAnm(anmId, 0.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    startSound(SE_otoshiana_nukeru);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1523C
void dAcPlayer_c::endPitfallExit(int nextState) {
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
        dFgobjManager::requestPitfallExit(mPlayerNo, mTargetUnitX, mTargetUnitZ);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
    mShadowScale = 1.0f;
    mShadowAlpha = 1.0f;
}

// .text 0x152D4
void dAcPlayer_c::pitfallExitParam_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x152E0
void dAcPlayer_c::initPocketFull() {
    setAnm(HMN_ANM_PUTAWAY_WAIT1, m3d::FORWARD_LOOP);
    mStep = 0;
    if (mPlayerNo == dNet::getMyNo() && dDemo::isActive9()) {
        mStep = 3;
    }
}

// .text 0x15340
void dAcPlayer_c::executePocketFull() {
    if (mPlayerNo == dNet::getMyNo()) {
        stepPocketFull();
    }
}

// .text 0x15380
void dAcPlayer_c::endPocketFull(int nextState) {
    mStep = 0x22;
}

// .text 0x1538C
void dAcPlayer_c::stepPocketFull() {
    switch (mTaskKind) {
    case FG_MNG_KIND_PICK_UP_FULL:
    case FG_MNG_KIND_DIG_UP_FULL:
    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
        break;
    default:
        if (mMsgStep != MSG_STEP_SWAP_INSECT && mMsgStep != MSG_STEP_SWAP_FISH) {
            setState(dPlayerMgr::STATE_IDLE);
            return;
        }
        break;
    }
    switch (mStep) {
    case 0:
        if (dDemo::request9()) {
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(MSG_POCKETS_FULL);
            getController()->requestStart(0);
            mStep = 1;
            mMsgStep = MSG_STEP_ASK_DROP;
        }
        break;
    case 1: {
        dMsg::Comp_c *ctrl = getController();
        if (ctrl == NULL) {
            break;
        }
        if (ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            break;
        }
        mStep = 2;
        break;
    }
    case 2: {
        dMsg::Comp_c *ctrl = getController();
        if (ctrl == NULL) {
            break;
        }
        if (!ctrl->isState(&dMsg::Comp_c::executeHidden)) {
            break;
        }
        detachDemoActor(this);
        mStep = 3;
        break;
    }
    case 3:
        switch (mMsgStep) {
        case MSG_STEP_SWAP:
            if (_2188.isValid() && fn_8019AC04(_2188.getId())) {
                mStep = 4;
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
            break;
        case MSG_STEP_SWAP_DUG:
            if (_2188.isValid() && fn_8019AC44(_2188.getId())) {
                mStep = 4;
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
            break;
        case MSG_STEP_SWAP_INSECT:
            if (_2188.isValid() && fn_8019AC84(_2188.getId())) {
                mStep = 4;
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
            break;
        case MSG_STEP_SWAP_FISH:
            mShowPos = mpToolBank->mFloat.mTarget;
            if (_2188.isValid() && fn_8019ACC4(_2188.getId(), &mShowPos)) {
                mStep = 4;
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
            break;
        default:
            setState(dPlayerMgr::STATE_SWAP_ITEM);
            break;
        }
        break;
    case 4:
        if (dMenuMng_c::isMenuEnd()) {
            if (lbl_8074EA7D == 1) {
                setState(dPlayerMgr::STATE_IDLE);
            } else {
                switch (mMsgStep) {
                case MSG_STEP_SWAP:
                    setState(dPlayerMgr::STATE_SWAP_ITEM);
                    break;
                case MSG_STEP_SWAP_DUG:
                    setState(dPlayerMgr::STATE_PUT_ITEM_IN_HOLE);
                    break;
                case MSG_STEP_SWAP_INSECT:
                    mActorFlags |= dPlayerMgr::ACTOR_FLAG_MENU_ACTION;
                    setState(dPlayerMgr::STATE_SHOW_CATCH_AT);
                    break;
                case MSG_STEP_SWAP_FISH:
                    if (mActionItem.isFish()) {
                        mActorFlags |= dPlayerMgr::ACTOR_FLAG_MENU_ACTION;
                        setState(dPlayerMgr::STATE_SHOW_CATCH_AT);
                    } else {
                        setState(dPlayerMgr::STATE_PUT_HELD_ITEM);
                    }
                    break;
                default:
                    setState(dPlayerMgr::STATE_SWAP_ITEM);
                    break;
                }
            }
        }
        break;
    default:
        setState(dPlayerMgr::STATE_IDLE);
        break;
    }
}

// .text 0x124EC
void dAcPlayer_c::executeAxeChop() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            if (mTaskKind == FG_MNG_KIND_CHOP_TREE_SPECIAL) {
                setState(dPlayerMgr::STATE_SURPRISED);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        } else if (mAnm.checkFrame(18.0f)) {
            startChopTree();
            updateAxeModel();
            startToolSound(SE_AxeCutWood);
            dEffect::create("af_itm_axe_woodchip", &mPos, &mAngle, NULL);
            u32 flag = fgMngState_getUnitFlag(mTargetUnitX, mTargetUnitZ);
            startRumble(flag == 3 ? dRumble::PATTERN_AXE_FELL : dRumble::PATTERN_AXE_CHOP);
        } else if (mAnm.getFrame() > 18.0f && mTaskKind != FG_MNG_KIND_CHOP_TREE_SPECIAL) {
            if (checkMoveOrAction()) {
                return;
            }
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(18.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                startChopTree();
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            updateAxeModel();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            startToolSound(SE_AxeCutWood);
            dEffect::create("af_itm_axe_woodchip", &mPos, &mAngle, NULL);
        }
    }
}

// .text 0x126C8
void dAcPlayer_c::createAxeHitEffect() {
    startToolSound(SE_AxeHitEtc);
    dEffect::create("af_itm_axe_hitstar", &mPos, &mAngle, NULL);
    startRumble(dRumble::PATTERN_AXE_HIT);
}

// .text 0x15764
void dAcPlayer_c::initAxeSwing() {
    setAnm(HMN_ANM_AXE_SWING1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mToolTarget = TOOL_TARGET_NONE;
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER) {
            mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_POINTER_TURN;
        }
        setNetObjc(10.0f, 32.0f, 32.0f, 16.0f + mPos.y);
        _2266 = dItem::ITEM_ID_NONE;
        _2278 = 0;
        _227C = 0;
        mObjcPos.mHitFunc = objcHit;
        mObjcPos.regist();
    }
}

// .text 0x15834
void dAcPlayer_c::executeAxeSwing() {
    if (mPlayerNo == dNet::getMyNo()) {
        checkAxeTarget();
        if (mToolTarget == TOOL_TARGET_AXE_UNIT) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 2000);
        } else if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_POINTER_TURN) {
            sLib::chaseAngle(&mAngle.y.mAngle, getActionAngle(), 2000);
        }
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_AXE_STRIKE);
        } else {
            mObjcPos.regist();
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnmNoMorph(HMN_ANM_AXE_SWING_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x15910
void dAcPlayer_c::endAxeSwing(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        endPointerTurn();
        mObjcPos.mHitFunc = NULL;
        if (mToolTarget != TOOL_TARGET_AXE_OBJECT) {
            mObjcPos.init();
            _2278 = 0;
            _2266 = dItem::ITEM_ID_NONE;
            _227C = 0;
        }
    }
}

// .text 0x15988
int dAcPlayer_c::checkAxeHit(mVec3_c *pos) {
    if (mObjcPos.mHitType != 0) {
        return TOOL_TARGET_AXE_OBJECT;
    }
    s16 angle = getActionAngle();
    if (dSTR::isAtFountainFront(mPos, angle, NULL, NULL, 0)) {
        return TOOL_TARGET_AXE_MISS;
    }
    int kind;
    f32 groundY = dBGCF::getGroundY(pos, &kind, 0x31);
    switch (kind) {
    case 0x15: {
        dFdBase_c *field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(pos, 0);
            if (item != NULL && item->isStump()) {
                return TOOL_TARGET_AXE_MISS;
            }
        }
        s8 result = checkAxeBreak(mpToolBank->mItem.getKind());
        if (result == 0) {
            return TOOL_TARGET_AXE_UNIT;
        } else if (result == 1) {
            return TOOL_TARGET_AXE_BREAK;
        }
        break;
    }
    case 0x13: {
        if (groundY - mPos.y < 10.0f) {
            return TOOL_TARGET_AXE_MISS;
        }
        s8 result = checkAxeBreak(mpToolBank->mItem.getKind());
        if (result == 0) {
            return TOOL_TARGET_AXE_UNIT;
        } else if (result == 1) {
            return TOOL_TARGET_AXE_BREAK;
        }
        break;
    }
    default:
        if (groundY - mPos.y >= 16.0f) {
            s8 result = checkAxeBreak(mpToolBank->mItem.getKind());
            if (result == 0) {
                return TOOL_TARGET_AXE_HIT;
            } else if (result == 1) {
                return TOOL_TARGET_AXE_BREAK;
            }
        }
        break;
    }
    return TOOL_TARGET_AXE_MISS;
}

// .text 0x15B6C
s8 dAcPlayer_c::checkAxeBreak(int kind) {
    if (kind == dItem::KIND_SILVER_AXE || kind == dItem::KIND_GOLD_AXE) {
        return 0;
    }
    if (kind == dItem::KIND_AXE) {
        if (cM::rndInt(80) < 10) {
            u16 next = mpToolBank->getNextAxe();
            dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, next);
            return next == dItem::ITEM_ID_NONE;
        }
        return 0;
    }
    return -1;
}

// .text 0x15C14
BOOL dAcPlayer_c::isAxeReplaced() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mEquipment.mHeld.getKind() == dItem::KIND_AXE && mpToolBank->mItem.getKind() == dItem::KIND_AXE &&
        mpToolBank->mItem.getId() != player->mEquipment.mHeld.getId()) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x15CA4
void dAcPlayer_c::updateAxeModel() {
    if (isAxeReplaced()) {
        dItem::Item axe = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld;
        mpToolBank->setAxeItem(axe);
        int wear = dItem::seeker_c::get()->findLike(axe);
        if (wear == 2) {
            startToolSound(SE_AxeBroken1);
        } else if (wear == 5) {
            startToolSound(SE_AxeBroken2);
        }
    }
}

// .text 0x15D34
void dAcPlayer_c::checkAxeTarget() {
    if (mToolTarget == TOOL_TARGET_NONE) {
        mVec3_c pos;
        getFreeFrontUnit(&pos);
        int hit = checkAxeHit(&pos);
        mToolTarget = hit;
        if (hit == 5) {
            dFdBase_c::getUnitCenterPos(&pos, mTargetUnitX, mTargetUnitZ);
            mTargetAngle = dActor_c::targetAngleY(&mPos, &pos);
        }
    }
}

// .text 0x15DA8
void dAcPlayer_c::initAxeStrike() {
    setAnmNoMorph(HMN_ANM_AXE_SWING_WAIT1, m3d::FORWARD_LOOP);
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo && mToolTarget == TOOL_TARGET_AXE_UNIT) {
        _229C = dFgobjManager::useTool(mPlayerNo, mTargetUnitX, mTargetUnitZ, dFgobjManager::USE_TOOL_AXE, 0,
                                       dItem::ITEM_ID_NONE);
    }
}

// .text 0x15E20
void dAcPlayer_c::executeAxeStrike() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mToolTarget == TOOL_TARGET_AXE_UNIT) {
            if (_229C != -1) {
                bool start = false;
                if (mAnm.getFrame() <= 1.0f) {
                    start = true;
                }
                waitFieldTask(0, start, 0);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        } else {
            switch (mToolTarget) {
            case TOOL_TARGET_AXE_MISS:
                setState(dPlayerMgr::STATE_AXE_MISS);
                break;
            case TOOL_TARGET_AXE_HIT:
                setState(dPlayerMgr::STATE_AXE_HIT);
                break;
            case TOOL_TARGET_AXE_BREAK:
                setState(dPlayerMgr::STATE_AXE_BREAK);
                break;
            case TOOL_TARGET_AXE_OBJECT:
                setState(dPlayerMgr::STATE_AXE_HIT_OBJECT);
                break;
            default:
                setState(dPlayerMgr::STATE_AXE_MISS);
                break;
            }
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x15F40
void dAcPlayer_c::endAxeStrike(int nextState) {}

// .text 0x15F44
void dAcPlayer_c::initAxeChop() {
    if (mPlayerNo == dNet::getMyNo()) {
        getAxeChopParam()->set(mTargetUnitX, mTargetUnitZ);
        setAnmNoMorph(HMN_ANM_AXE_HIT1, m3d::FORWARD_ONCE);
    } else {
        mTargetUnitZ = getAxeChopParam()->mUnitZ;
        mTargetUnitX = getAxeChopParam()->mUnitX;
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING | dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        if (!setAnmNoMorph(HMN_ANM_AXE_HIT1, m3d::FORWARD_ONCE)) {
            mAnm.setFrame(0.0f);
        }
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x15FF8
void dAcPlayer_c::endAxeChop(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
            startChopTree();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
            updateAxeModel();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1607C
BOOL dAcPlayer_c::startChopTree() {
    enum { CHOP_RIGHT = 4, CHOP_LEFT = 5 };
    return dFgobjManager::startChopTree(mPlayerNo, mTargetUnitX, mTargetUnitZ,
                                      mAngle.y >= 0 ? CHOP_LEFT : CHOP_RIGHT);
}

// .text 0x160A4
void dAcPlayer_c::axeChopParam_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x160B0
void dAcPlayer_c::initAxeMiss() {
    setAnmNoMorph(HMN_ANM_AXE_SUKA1, m3d::FORWARD_ONCE);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x160F0
void dAcPlayer_c::executeAxeMiss() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(15.0f)) {
            startToolSound(SE_AxeKaraburi);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(15.0f)) {
            startToolSound(SE_AxeKaraburi);
        }
    }
}

// .text 0x161D8
void dAcPlayer_c::endAxeMiss(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x161E8
void dAcPlayer_c::initAxeHit() {
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    if (mPlayerNo == dNet::getMyNo()) {
        BOOL fell = FALSE;
        if (mTaskKind == FG_MNG_KIND_HIT_MONEY_ROCK) {
            fell = TRUE;
        }
        getPosAtAngle(&mTargetPos, mAngle.y, -8.0f);
        getAxeHitParam()->set(mAngle.y, mTargetUnitX, mTargetUnitZ, 0, fell);
        setAnmNoMorph(HMN_ANM_AXE_HANE1, m3d::FORWARD_ONCE);
    } else {
        mTargetUnitZ = getAxeHitParam()->mUnitZ;
        mTargetUnitX = getAxeHitParam()->mUnitX;
        if (getAxeHitParam()->mFell != 0) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        if (!setAnmNoMorph(HMN_ANM_AXE_HANE1, m3d::FORWARD_ONCE)) {
            mAnm.setFrame(0.0f);
        }
    }
}

// .text 0x162F4
void dAcPlayer_c::executeAxeHit() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(18.0f)) {
            if (mTaskKind == FG_MNG_KIND_HIT_MONEY_ROCK) {
                dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ, mAngle.y);
            }
            updateAxeModel();
            createAxeHitEffect();
        } else if (mAnm.getFrame() > 18.0f && mAnm.getFrame() < 38.0f) {
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.3f, 2.0f);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(18.0f)) {
            if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ, getAxeHitParam()->mAngle);
                mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
            }
            updateAxeModel();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            createAxeHitEffect();
        }
    }
}

// .text 0x1647C
void dAcPlayer_c::endAxeHit(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
            dFgobjManager::procChopResult(mTargetUnitX, mTargetUnitZ, mAngle.y);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
            updateAxeModel();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x16508
void dAcPlayer_c::axeHitParam_c::set(s16 angle, u8 unitX, u8 unitZ, u8 arg, u8 fell) {
    mAngle = angle;
    mUnitX = unitX;
    mUnitZ = unitZ;
    _4 = arg;
    mFell = fell;
}

// .text 0x16520
void dAcPlayer_c::initAxeBreak() {
    mTargetAngle = 0;
    if (mPlayerNo == dNet::getMyNo()) {
        setAnmNoMorph(HMN_ANM_AXE_BROKE1, m3d::FORWARD_ONCE);
    } else {
        if (!setAnmNoMorph(HMN_ANM_AXE_BROKE1, m3d::FORWARD_ONCE)) {
            mAnm.setFrame(0.0f);
        }
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x1271C
void dAcPlayer_c::executeAxeBreak() {
    int playerNo = mPlayerNo;
    if (playerNo == dNet::getMyNo()) {
        f32 frame = mAnm.getFrame();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_AXE_BREAK_MSG);
        } else if (mAnm.checkFrame(20.0f)) {
            startToolSound(SE_AxeBroken3);
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
            dEffect::create("af_itm_axe_break", &mPos, &mAngle, NULL);
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->mute30();
            }
        } else if (frame >= 20.0f && frame <= 48.0f) {
            sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 0xD, 0x1194);
            if (frame > 40.0f) {
                sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 200);
            }
        } else if (frame > 48.0f) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 200);
        }
    } else if (mAnmId == HMN_ANM_AXE_BROKE1) {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, playerNo)) {
            mTargetPos.set(x, mPos.y, z);
            cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 2.0f);
        }
        f32 frame = mAnm.getFrame();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_AXE_BROKE2, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        } else if (mAnm.checkFrame(20.0f)) {
            startToolSound(SE_AxeBroken3);
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
            dEffect::create("af_itm_axe_break", &mPos, &mAngle, NULL);
        } else if (frame >= 20.0f && frame <= 48.0f) {
            sLib::addCalcAngle(&mAngle.y.mAngle, mTargetAngle, 0xD, 0x1194);
            if (frame > 40.0f) {
                sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 200);
            }
        } else if (frame > 48.0f) {
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 200);
        }
    }
}

// .text 0x165B4
void dAcPlayer_c::endAxeBreak(int nextState) {
    removeTool();
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_HIDE_TOOL;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

// .text 0x165F4
void dAcPlayer_c::initAxeBreakMsg() {
    setAnm(HMN_ANM_AXE_BROKE2, m3d::FORWARD_LOOP);
    mStep = 10;
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
        }
    }
}

// .text 0x16658
void dAcPlayer_c::executeAxeBreakMsg() {
    if (mPlayerNo == dNet::getMyNo()) {
        stepAxeBreakMsg();
    } else {
        followRemoteTransform();
    }
}

// .text 0x166A4
void dAcPlayer_c::endAxeBreakMsg(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop2CFade();
        }
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
    }
}

// .text 0x16708
void dAcPlayer_c::stepAxeBreakMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play2C();
            }
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(MSG_AXE_BROKE);
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

// .text 0x16818
void dAcPlayer_c::initShowCatch() {
    mTargetAngle = 0;
    mStep = 5;
    if (mPlayerNo == dNet::getMyNo()) {
        (getShowCatchParam())->set(mActionItem.getId(), mStep);
    } else {
        mActionItem = getShowCatchParam()->mItem;
        mStep = getShowCatchParam()->mStep;
    }
    BOOL insect = mActionItem.isInsect();
    createShowCatch(insect);
    enum { SHOW_INSECT = 0x53, SHOW_FISH = 0x54 };
    setAnm(insect ? SHOW_INSECT : SHOW_FISH, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x168E8
void dAcPlayer_c::executeShowCatch() {
    if (mPlayerNo == dNet::getMyNo()) {
        (getShowCatchParam())->set(mActionItem.getId(), mStep);
        if (mStep == 5 || mStep == 6) {
            updateShowCatch((mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0);
        } else if (mStep == 7) {
            putAwayShowCatch((mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0);
            if (mAnm.isStop()) {
                setState(dPlayerMgr::STATE_DEMO_START);
            }
        } else {
            setState(dPlayerMgr::STATE_DEMO_START);
        }
    } else {
        getRemoteActionParams(&mRemoteActionParams);
        mStep = getShowCatchParam()->mStep;
        if (mStep == 6) {
            mStep = 7;
        }
        if (mStep == 5) {
            if (mAnmId == HMN_ANM_MENU_SHOW1 || mAnmId == HMN_ANM_MENU_SHOW2) {
                updateShowCatch(mActionItem.isInsect());
            }
        } else if (mStep == 7) {
            if (mAnmId == HMN_ANM_MENU_SHOW1 || mAnmId == HMN_ANM_MENU_SHOW2) {
                setAnm(HMN_ANM_MENU_PUTAWAY1, m3d::FORWARD_ONCE);
            }
            if (mAnmId == HMN_ANM_MENU_PUTAWAY1) {
                putAwayShowCatch(mActionItem.isInsect());
                if (mAnm.isStop()) {
                    setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
                }
            }
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x16A74
void dAcPlayer_c::endShowCatch(int nextState) {
    removeShowCatch(mActionItem.isInsect());
}

// .text 0x16AB0
void dAcPlayer_c::createShowCatch(BOOL insect) {
    int type = dItem::seeker_c::get()->findLike(mActionItem);
    if (insect) {
        if (g_playActorMng != NULL) {
            mShowInsect =
                reinterpret_cast<dInsectField_c *>(g_playActorMng)
                    ->addPlayerInsect(type, dInsectField_c::PLAYER_INSECT_SHOW,
                                      &mMdlCallback.mHandPos, mPlayerNo);
        }
    } else if (g_fishField != NULL) {
        mShowFish = g_fishField->addPlayerFish(type, 0, &mMdlCallback.mHandPos, mPlayerNo);
    }
}

// .text 0x16B50
void dAcPlayer_c::removeShowCatch(BOOL insect) {
    if (insect) {
        if (mShowInsect != NULL) {
            static_cast<dInsFldBase_c *>(mShowInsect)->cancelHold();
        }
    } else if (mShowFish != NULL) {
        mShowFish->requestDelete();
    }
    mShowInsect = NULL;
    mShowFish = NULL;
}

// .text 0x16BB0
void dAcPlayer_c::updateShowCatch(BOOL insect) {
    static const f32 l_showFrame = 30.0f;
    static const f32 l_showEndFrame = 48.0f;
    static const f32 l_releaseFrame = 10.0f;
    static const f32 l_releaseEndFrame = 40.0f;
    static const f32 l_pocketFrame = 75.0f;
    static const f32 l_fullScale = 1.0f;

    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
    if (insect) {
        if (mShowInsect != NULL) {
            static_cast<dInsFldBase_c *>(mShowInsect)
                ->setHeldPos(&mMdlCallback.mHandPos);
            f32 frame = mAnm.getFrame();
            if (mAnm.isStop()) {
                mVec3_c scale(l_fullScale, l_fullScale, l_fullScale);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            } else if (mAnm.checkFrame(l_showFrame)) {
                mVec3_c scale(0.0f, 0.0f, l_fullScale);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            } else if (mAnm.checkFrame(l_showEndFrame)) {
                mVec3_c scale(l_fullScale, l_fullScale, l_fullScale);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            } else if (frame > l_showFrame && frame < l_showEndFrame) {
                f32 s = 0.056 * (mAnm.getFrame() - l_showFrame);
                if (s > l_fullScale) {
                    s = l_fullScale;
                }
                mVec3_c scale(s, s, l_fullScale);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            }
            if (frame > 76.0f) {
                createCatchEffect(static_cast<dPlayActor_c *>(mShowInsect));
            }
        } else {
            int type = dItem::seeker_c::get()->findLike(mActionItem);
            if (g_playActorMng != NULL) {
                mShowInsect = reinterpret_cast<dInsectField_c *>(g_playActorMng)
                                  ->addPlayerInsect(
                                      type, dInsectField_c::PLAYER_INSECT_SHOW,
                                      &mMdlCallback.mHandPos, mPlayerNo);
            }
        }
    } else if (mShowFish != NULL) {
        dFishInfo::getHoldParam(mShowFish->mKind);
        f32 frame = mAnm.getFrame();
        if (mAnm.isStop()) {
            mVec3_c scale(l_fullScale, l_fullScale, l_fullScale);
            mShowFish->setHeldScale(&scale);
        } else if (mAnm.checkFrame(l_showFrame)) {
            mVec3_c scale(0.0f, 0.0f, l_fullScale);
            mShowFish->setHeldScale(&scale);
        } else if (mAnm.checkFrame(l_showEndFrame)) {
            mVec3_c scale(l_fullScale, l_fullScale, l_fullScale);
            mShowFish->setHeldScale(&scale);
        } else if (frame > l_showFrame && frame < l_showEndFrame) {
            f32 s = 0.056 * (mAnm.getFrame() - l_showFrame);
            if (s > l_fullScale) {
                s = l_fullScale;
            }
            mVec3_c scale(s, s, l_fullScale);
            mShowFish->setHeldScale(&scale);
        }
    } else {
        int type = dItem::seeker_c::get()->findLike(mActionItem);
        if (g_fishField != NULL) {
            mShowFish = g_fishField->addPlayerFish(type, 0, &mMdlCallback.mHandPos, mPlayerNo);
        }
    }
    if (mAnm.isStop()) {
        if (mStep == 6) {
            if (mPlayerNo == dNet::getMyNo()) {
                mStep = 7;
            }
            setAnm(HMN_ANM_MENU_PUTAWAY1, m3d::FORWARD_ONCE);
        }
    } else if (mAnm.checkFrame(l_releaseFrame)) {
        startSound(SE_gasagoso_misebirakashi);
    } else if (mAnm.checkFrame(l_releaseEndFrame)) {
        startSound(SE_toridashi_misebirakashi);
    } else if (mAnm.checkFrame(l_pocketFrame)) {
        startSound(SE_Misebirakashi);
    }
}

// .text 0x16FA0
void dAcPlayer_c::putAwayShowCatch(BOOL insect) {
    static const f32 l_putAwayFrame = 22.0f;

    if (insect) {
        if (mShowInsect != NULL) {
            static_cast<dInsFldBase_c *>(mShowInsect)
                ->setHeldPos(&mMdlCallback.mHandPos);
            if (mAnm.checkFrame(18.0f)) {
                mVec3_c scale(0.0f, 0.0f, 0.0f);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            } else if (mAnm.checkFrame(l_putAwayFrame)) {
                startSound(SE_MisebirakashiKakunou);
            } else if (mAnm.getFrame() < 18.0f) {
                f32 s = 1.0f - 0.065f * mAnm.getFrame();
                if (s < 0.0f) {
                    s = 0.0f;
                }
                mVec3_c scale(s, s, 1.0f);
                static_cast<dInsFldBase_c *>(mShowInsect)->setHeldScale(&scale);
            }
        }
    } else if (mShowFish != NULL) {
        mShowFish->setHeldPos(&mMdlCallback.mHandPos);
        if (mAnm.checkFrame(18.0f)) {
            mVec3_c scale(0.0f, 0.0f, 0.0f);
            mShowFish->setHeldScale(&scale);
        } else if (mAnm.checkFrame(l_putAwayFrame)) {
            startSound(SE_MisebirakashiKakunou);
        } else if (mAnm.getFrame() < 18.0f) {
            f32 s = 1.0f - 0.065f * mAnm.getFrame();
            if (s < 0.0f) {
                s = 0.0f;
            }
            mVec3_c scale(s, s, 1.0f);
            mShowFish->setHeldScale(&scale);
        }
    }
}

// .text 0x1716C
void dAcPlayer_c::showCatchParam_c::set(u16 item, u8 step) {
    mItem = item;
    mStep = step;
}

// .text 0x17178
void dAcPlayer_c::initShowCatchAt() {
    if (mPlayerNo == dNet::getMyNo()) {
        int x = worldToUnit(mShowPos.x);
        int z = worldToUnit(mShowPos.z);
        getShowCatchAtParam()->set(mActionItem.mId, x, z,
                                   (mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) != 0);
        releaseCatch(mActionItem.isInsect(), (mActorFlags & dPlayerMgr::ACTOR_FLAG_MENU_ACTION) != 0);
    } else {
        showCatchAtParam_c *param = getShowCatchAtParam();
        dFdBase_c::getUnitCenterPos(&mShowPos, param->mUnitX, param->mUnitZ);
        dBGCF::groundChk_c chk(&mShowPos, 0, 0, 0);
        mShowPos.y = chk.getWaterY();
        mActionItem = param->mItem;
        if (param->mFromPocket == 0 && mInsectLink.mpInsect == NULL && g_playActorMng != NULL) {
            mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
        }
        releaseCatch(mActionItem.isInsect(), param->mFromPocket);
    }
    setIdleAnm();
}

// .text 0x172C4
void dAcPlayer_c::executeShowCatchAt() {
    if (mPlayerNo == dNet::getMyNo()) {
        watchReleasedCatch((mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0);
    } else {
        if (mActionItem.isInsect() && mInsectLink.mpInsect == NULL && g_playActorMng != NULL) {
            mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
        }
        watchReleasedCatch(mActionItem.isInsect());
    }
}

// .text 0x17388
void dAcPlayer_c::endShowCatchAt(int nextState) {
    if (mPlayerNo != dNet::getMyNo()) {
        if (mInsectLink.mpInsect != NULL) {
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->cancelHold();
        }
        if (mFishLink.mFish != NULL) {
            mFishLink.mFish->requestDelete();
        }
    }
    if (mInsectLink.mpInsect != NULL) {
        dInsectField::removeNpcLink(mInsectLink.mpInsect, &mInsectLink);
    }
    if (mFishLink.mFish != NULL) {
        mFishLink.mFish->removeLink(&mFishLink);
    }
    lookAt(NULL, 0x7F, 0x7F);
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_MENU_ACTION;
}

// .text 0x17428
void dAcPlayer_c::releaseCatch(BOOL insect, BOOL fromPocket) {
    if (insect) {
        if (fromPocket) {
            dPlayActorMng_c *mng = g_playActorMng;
            if (mng != NULL) {
                int type = dItem::seeker_c::get()->findLike(mActionItem);
                mInsectLink.set(
                    reinterpret_cast<dInsectField_c *>(mng)->addPlayerInsect(
                        type, dInsectField_c::PLAYER_INSECT_RELEASE,
                        &mMdlCallback.mHandPos, mPlayerNo));
            }
        }
        if (mInsectLink.mpInsect != NULL) {
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->release();
        }
    } else {
        if (fromPocket) {
            dFishField_c *field = g_fishField;
            if (field != NULL) {
                int type = dItem::seeker_c::get()->findLike(mActionItem);
                dFishFldShadow_c *fish = field->addPlayerFish(type, 1, &mMdlCallback.mHandPos, mPlayerNo);
                if (mFishLink.mFish != NULL) {
                    mFishLink.mFish->removeLink(&mFishLink);
                }
                if (fish != NULL) {
                    fish->addLink(&mFishLink);
                }
            }
        } else {
            dFishFldShadow_c *fish = mpToolBank->mFloat.getFish();
            if (mFishLink.mFish != NULL) {
                mFishLink.mFish->removeLink(&mFishLink);
            }
            if (fish != NULL) {
                fish->addLink(&mFishLink);
            }
        }
        if (mFishLink.mFish != NULL) {
            mFishLink.mFish->throwBack(&mShowPos);
        }
    }
}

// .text 0x175B0
void dAcPlayer_c::watchReleasedCatch(BOOL insect) {
    if (insect) {
        dInsectFieldActor_c *actor = (dInsectFieldActor_c *)mInsectLink.mpInsect;
        if (actor != NULL) {
            mVec3_c pos(actor->mPos);
            lookAt(&pos, 0x7F, 0x7F);
            static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
                ->setHeldScale(&mVec3_c(1.0f, 1.0f, 1.0f));
        } else if (mPlayerNo == dNet::getMyNo()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        dFishFldShadow_c *fish = mFishLink.mFish;
        if (fish != NULL) {
            mVec3_c pos(fish->mPos);
            lookAt(&pos, 0x7F, 0x7F);
            mFishLink.mFish->setHeldScale(&mVec3_c(1.0f, 1.0f, 1.0f));
        } else if (mPlayerNo == dNet::getMyNo()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    }
}

// .text 0x176CC
void dAcPlayer_c::showCatchAtParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 fromPocket) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mFromPocket = fromPocket;
}

// .text 0x176E0
void dAcPlayer_c::initPickUp() {
    mStep = 10;
    if (mPlayerNo == dNet::getMyNo()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NOT_ALONE) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) && _2188.mId != dItem::ITEM_ID_NONE) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else {
            putInPocket(&_2188);
            if (mPickUpKind == 0) {
                setAnm(HMN_ANM_PICKUP1, m3d::FORWARD_ONCE);
                dFtrm_c::sendEvent(_2268);
            } else {
                setAnm(HMN_ANM_KAGU_PUTAWAY1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
            }
        }
        getPickUpFtrParam()->set(mActionItem.mId, mTargetUnitX, mTargetUnitZ, mPickUpKind, mAnmId == HMN_ANM_NML_WAIT1);
    } else {
        pickUpFtrParam_c *param = getPickUpFtrParam();
        u8 unitX = param->mUnitX;
        u8 unitZ = param->mUnitZ;
        mTargetUnitX = unitX;
        mTargetUnitZ = unitZ;
        mActionItem = param->mItem;
        mPickUpKind = param->mKind;
        if (param->mFull) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        } else {
            setAnm(mPickUpKind == 0 ? HMN_ANM_PICKUP1 : HMN_ANM_KAGU_PUTAWAY1, 60.0f, 0.0f, 1.0f, 16.0f,
                   m3d::FORWARD_ONCE, TRUE);
        }
    }
}

// .text 0x17874
void dAcPlayer_c::executePickUp() {
    static const f32 l_takeFrame = 11.0f;
    static const f32 l_putFrame = 14.0f;
    static const f32 l_putDownFrame = 32.0f;
    static const f32 l_secondTakeFrame = 6.0f;
    static const f32 l_secondPutFrame = 17.0f;
    static const f32 l_secondPutDownFrame = 24.0f;
    static const f32 l_secondEndFrame = 45.0f;

    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnmId == HMN_ANM_NML_WAIT1) {
            stepPickUpFullMsg();
            return;
        }
        if ((mAnmId == HMN_ANM_PICKUP1 || mAnmId == HMN_ANM_KAGU_PUTAWAY1) && mAnm.checkFrame(40.0f)) {
            startSound(SE_gasagoso_return);
        }
        f32 holdFrame = mPickUpKind == 0 ? l_putFrame : l_secondPutDownFrame;
        f32 endFrame = mPickUpKind == 0 ? l_putDownFrame : l_secondEndFrame;
        f32 grabFrame = mPickUpKind == 0 ? l_takeFrame : l_secondPutFrame;
        f32 frame = mAnm.getFrame();
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mPickUpKind == 1 && mAnm.checkFrame(l_secondTakeFrame)) {
            dFtrm_c::sendEvent(_2268);
        } else if (mAnm.checkFrame(grabFrame)) {
            mVec3_c *effectPos = dFtrm_c::getEffectPos(_2268);
            if (effectPos != NULL) {
                mItemPos = *effectPos;
            } else {
                mItemPos = mMdlCallback.mHandPos;
            }
            f32 frames = 13.0f;
            if (frames < 1.0f) {
                frames = 1.0f;
            }
            _21D0[0] = mItemPos.xzDistTo(mMdlCallback.mHandPos) / frames;
        } else if (frame > grabFrame && frame <= holdFrame) {
            cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(1.0f, 1.0f, 1.0f));
        } else if (frame > holdFrame && frame < endFrame) {
            f32 s = 1.0f - 0.056f * (frame - holdFrame);
            if (s < 0.0f) {
                s = 0.0f;
            }
            mVec3_c scale;
            scale.x = s;
            scale.y = s;
            scale.z = s;
            cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
            setFgObjItem(mActionItem.mId, &mItemPos, &scale);
        }
    } else if (mAnm.isStop()) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x17B50
void dAcPlayer_c::endPickUp(int nextState) {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_NOT_ALONE;
}

// .text 0x17B60
void dAcPlayer_c::stepPickUpFullMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            attachDemoActor(this);
            if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NOT_ALONE) {
                setMessageLabel("sys_MESS/SYS_Player");
                setMessageCode(MSG_NO_PICK_UP_WITH_GUESTS);
            } else {
                setMessageLabel("sys_MESS/SYS_Player");
                setMessageCode(MSG_POCKETS_FULL_LEAVE);
            }
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

// .text 0x17C88
void dAcPlayer_c::pickUpFtrParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 kind, BOOL full) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mKind = kind;
    mFull = full;
}

// .text 0x17CA0
void dAcPlayer_c::initInsectBite() {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->mute29();
        }
        setAnm(HMN_ANM_STING_MOSQUITO1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        mTargetAngle = 0;
        mStep = 10;
        startSound(SE_mosqueto_sasareru);
    }
}

// .text 0x17D3C
void dAcPlayer_c::executeInsectBite() {
    static const int l_biteFrame = 20;

    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnmId == HMN_ANM_STING_MOSQUITO1) {
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_STING_MOSQUITO2, m3d::FORWARD_LOOP);
                mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y.mAngle) / l_biteFrame;
                if (mTurnFrames <= 0) {
                    mTurnFrames = 1000;
                }
                if (mPlayerNo == dNet::getMyNo()) {
                    dCamera_c *camera = dCamera_c::sInstance;
                    if (camera != NULL) {
                        camera->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
                    }
                }
            }
        } else {
            if (mAnm.getFrame() <= l_biteFrame) {
                sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            }
            stepInsectBiteMsg();
        }
    }
}

// .text 0x17E38
void dAcPlayer_c::endInsectBite(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->unmute29Fade();
        }
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
    }
}

// .text 0x17E9C
void dAcPlayer_c::stepInsectBiteMsg() {
    switch (mStep) {
    case 10:
        if (dDemo::request9()) {
            attachDemoActor(this);
            setMessageLabel("sys_MESS/SYS_Player");
            setMessageCode(MSG_MOSQUITO_BITE);
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

// .text 0x17F98
void dAcPlayer_c::initBeeSting() {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->mute20();
        }
        setAnm(HMN_ANM_STING_BEE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
        mStep = 10;
        startSound(SE_hachi_sasareru);
        if (mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
            dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
        }
        _22A4[7] = 60;
    }
}

// .text 0x1805C
void dAcPlayer_c::executeBeeSting() {
    static const int l_turnFrames = 100;

    if (mPlayerNo == dNet::getMyNo()) {
        int toolType = mpToolBank->getToolType();
        if (_22A4[7] == 1 && toolType == HMN_TOOL_BALLOON) {
            mAng angle = getAngleY();
            mpToolBank->releaseBalloon(&angle);
            mMdl.removePartAnm(1, 0.0f);
        }
        if (_22A4[7] != 0) {
            _22A4[7]--;
        }
        switch (mAnmId) {
        case HMN_ANM_STING_BEE1:
            if (mAnm.isStop()) {
                mTargetAngle = 0x7FFF;
                setAnmNoMorph(HMN_ANM_STING_BEE2, m3d::FORWARD_ONCE);
                if (toolType == HMN_TOOL_BALLOON && mpToolBank->isBalloonFly()) {
                    mMdl.removePartAnm(1, 0.0f);
                }
                mTurnFrames = sLib::distanceAngle(mTargetAngle, mAngle.y.mAngle) / l_turnFrames;
                if (mTurnFrames <= 0) {
                    mTurnFrames = 1000;
                }
            }
            break;
        case HMN_ANM_STING_BEE2:
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, mTurnFrames);
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_STING_BEE3, m3d::FORWARD_ONCE);
                if (toolType == HMN_TOOL_BALLOON && mpToolBank->isBalloonFly()) {
                    mMdl.removePartAnm(1, 0.0f);
                }
                mActorFlags |= dPlayerMgr::ACTOR_FLAG_BEE_STUNG;
                resetFaceTex();
                mAngle.x = 0;
                mAngle.y = 0x7FFF;
                mAngle.z = 0;
            }
            break;
        case HMN_ANM_STING_BEE3:
            if (mAnm.isStop()) {
                setAnmNoMorph(HMN_ANM_STING_BEE4, m3d::FORWARD_ONCE);
                if (toolType == HMN_TOOL_BALLOON && mpToolBank->isBalloonFly()) {
                    mMdl.removePartAnm(1, 0.0f);
                }
            }
            break;
        case HMN_ANM_STING_BEE4:
            if (mAnm.isStop()) {
                mTargetAngle = 0;
                setAnmNoMorph(HMN_ANM_STING_BEE5, m3d::FORWARD_ONCE);
                if (toolType == HMN_TOOL_BALLOON && mpToolBank->isBalloonFly()) {
                    mMdl.removePartAnm(1, 0.0f);
                }
                dCamera_c *camera = dCamera_c::sInstance;
                if (camera != NULL) {
                    camera->mRequestedMode = dCamera_c::MODE_PLAYER_VIEW_13;
                }
            } else if (mAnm.checkFrame(35.0f)) {
                startSound(SE_Kyorokyoro);
            }
            break;
        case HMN_ANM_STING_BEE5:
            sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x38E);
            if (mAnm.isStop()) {
                mpToolBank->setAnmRate(0.0f);
            }
            stepBeeStingMsg();
            break;
        default:
            setAnmNoMorph(HMN_ANM_STING_BEE5, m3d::FORWARD_ONCE);
            break;
        }
    }
}

// .text 0x18374
void dAcPlayer_c::endBeeSting(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop1C();
        }
        if (mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
            removeTool();
        }
    }
}

// .text 0x183F0
void dAcPlayer_c::stepBeeStingMsg() {
    switch (mStep) {
    case 10: {
        attachDemoActor(this);
        setMessageLabel("sys_MESS/SYS_Player");
        setMessageCode(MSG_BEE_STING);
        getController()->requestStart(0);
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->play1C();
        }
        mStep = 11;
        break;
    }
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

// .text 0x184F4
void dAcPlayer_c::initEatRemoveMask() {
    setAnm(HMN_ANM_MENU_CHANGE1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mTargetAngle = 0;
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x18568
void dAcPlayer_c::executeEatRemoveMask() {
    if (mAnmId == HMN_ANM_MENU_CHANGE1) {
        if (mAnm.checkFrame(1.0f)) {
            startSound(SE_wear);
            dEffect::create("af_ply_change", &mPos, NULL, NULL);
        } else if (mAnm.checkFrame(38.0f)) {
            createChangeEffect();
        }
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xA8C);
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_DEMO_START);
        } else if (mAnm.checkFrame(24.0f)) {
            removeMaskForEating();
        }
    } else if (mAnmId != HMN_ANM_MENU_THINK1) {
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        } else if (mAnm.checkFrame(24.0f)) {
            removeMaskForEating();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
}

// .text 0x186BC
void dAcPlayer_c::endEatRemoveMask(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING)) {
        removeMaskForEating();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x18714
void dAcPlayer_c::removeMaskForEating() {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_BEE_STUNG;
    resetFaceTex();
    setFaceAnm(HMN_ANM_MENU_CHANGE1, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x18760
void dAcPlayer_c::initEat() {
    setAnm(HMN_ANM_MENU_EAT1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    if (mPlayerNo == dNet::getMyNo()) {
        getEatParam()->set(mActionItem.mId);
    } else {
        mActionItem = getEatParam()->mItem;
    }
    mTargetAngle = 0;
}

// .text 0x187E0
void dAcPlayer_c::executeEat() {
    if (mAnmId == HMN_ANM_MENU_EAT1) {
        calcEatEffect();
    }
    if (mPlayerNo == dNet::getMyNo()) {
        updateEat(TRUE);
    } else if (mAnmId != HMN_ANM_MENU_THINK1) {
        updateEat(FALSE);
    }
}

// .text 0x18850
void dAcPlayer_c::endEat(int nextState) {
    _21D0[9] = 0.0f;
    _21D0[10] = 0.0f;
}


// .text 0x18864
void dAcPlayer_c::calcEatItemPos(mVec3_c *pos) {
    static f32 l_eatDropFrame = 30.0f;
    static f32 l_eatLiftFrame = 50.0f;
    if (pos != NULL) {
        f32 drop = -4.5f;
        if (mActionItem.isSame(dItem::Item(dItem::ITEM_IDX_CHERRY))) {
            drop = -3.0f;
        }
        *pos = mMdlCallback.mHandPos;
        f32 frame = mAnm.getFrame();
        if (frame >= l_eatLiftFrame) {
            f32 t = 0.005f * (frame - l_eatLiftFrame);
            if (t > 1.0f) {
                t = 1.0f;
            }
            _21D0[10] += mAng(16384.0f * t).sin();
            if (_21D0[10] > 2.5f) {
                _21D0[10] = 2.5f;
            }
        } else if (frame >= l_eatDropFrame) {
            sLib::addCalc(&_21D0[10], drop, 0.2f, 1.0f, 0.1f);
        }
        pos->y += _21D0[10];
        if (frame >= l_eatDropFrame) {
            sLib::addCalc(&_21D0[9], -5.5f, 0.2f, 1.0f, 0.1f);
            pos->x += _21D0[9];
            pos->z += 2.0f;
        }
    }
}

// .text 0x18A1C
void dAcPlayer_c::updateEat(BOOL mine) {
    static f32 l_eatShowFrame = 20.0f;
    static f32 l_eatBiteFrame = 45.0f;
    static f32 l_eatEndFrame = 108.0f;
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x7D0);
    mVec3_c scale(0.0f, 0.0f, 0.0f);
    f32 frame = mAnm.getFrame();
    if (mAnm.isStop()) {
        if (mine) {
            setState(dPlayerMgr::STATE_DEMO_START);
            return;
        }
        if (mState == dPlayerMgr::STATE_EAT_SPECIAL) {
            setAnm(HMN_ANM_MENU_THINK2, m3d::FORWARD_LOOP);
        } else {
            setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
        }
    } else if (mAnm.checkFrame(l_eatBiteFrame)) {
        scale.set(1.0f, 1.0f, 1.0f);
        startSound(SE_eat);
    } else if (frame > l_eatBiteFrame && frame < l_eatEndFrame) {
        f32 s = 1.0f - 0.015f * (frame - l_eatBiteFrame);
        if (s < 0.3f) {
            s = 0.0f;
        }
        scale.set(s, s, s);
    } else if (frame >= l_eatShowFrame && frame < l_eatBiteFrame) {
        scale.set(1.0f, 1.0f, 1.0f);
    }
    mVec3_c pos;
    calcEatItemPos(&pos);
    setFgObjItem(mActionItem.mId, &pos, &scale);
}

// .text 0x18BD4
void dAcPlayer_c::calcEatEffect() {
    static const f32 l_effectFrame = 138.0f;

    if (mAnm.checkFrame(l_effectFrame)) {
        mMtx_c mtx;
        getNodeMtx(&mtx, "mouth");
        mEatEffectPos.set(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
        dWorld::toFieldPosition(&mEatEffectPos, &mEatEffectPos);
        mLevelEffect1.create("af_ply_eat_yodare", &mEatEffectPos, &mAngle, NULL);
    } else if (mAnm.getFrame() > l_effectFrame) {
        mMtx_c mtx;
        getNodeMtx(&mtx, "mouth");
        mEatEffectPos.set(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
        dWorld::toFieldPosition(&mEatEffectPos, &mEatEffectPos);
        mLevelEffect1.followEffect(&mEatEffectPos, &mAngle, NULL);
    }
}

// .text 0x18CD4
void dAcPlayer_c::eatParam_c::set(u16 item) {
    mItem = item;
}

// .text 0x18CDC
void dAcPlayer_c::executeShakeUmbrella() {
    if (mAnmId == HMN_ANM_UMB_ROTATE1) {
        if (mAnm.checkFrame(1.0f)) {
            dWeather_c *weather = dWeather_c::sInstance;
            if (weather != NULL && dWeather_c::isRainKind(weather->mWeather)) {
                f32 y = 0.0f;
                f32 scale = 1.0f;
                getUmbrellaEffectParam(&y, &scale);
                PSMTXIdentity(mUmbrellaEffectMtx);
                mUmbrellaEffectMtx.concat(mMtx_c::createScale(scale, scale, scale));
                mUmbrellaEffectMtx.concat(mMtx_c::createTrans(0.0f, y, 0.0f));
                mMtx_c mtx = mModelMtx;
                PSMTXConcat(mtx, mUmbrellaEffectMtx, mtx);
                mLevelEffect2.create("af_itm_umbrella", &mtx);
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EFFECT;
            }
        } else if ((mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) && mAnm.getFrame() > 1.0f) {
            mMtx_c mtx = mModelMtx;
            PSMTXConcat(mtx, mUmbrellaEffectMtx, mtx);
            mLevelEffect2.followEffect(&mtx);
        }
    }
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.getFrame() >= 42.0f && checkMoveOrAction()) {
            return;
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        }
    }
}

// .text 0x18F98
void dAcPlayer_c::executeTrip() {
    if (mPlayerNo != dNet::getMyNo()) {
        followRemoteTransform();
    }
    if (mAnmId == HMN_ANM_TUMBLE_DOWNA1 || mAnmId == HMN_ANM_TUMBLE_DOWNB1) {
        if (mAnm.checkFrame(5.0f)) {
            startWeatherSound(SE_Kokeru, SE_Kokeru_rain);
        } else if (mAnm.checkFrame(20.0f)) {
            createTripEffect();
            startRumble(dRumble::PATTERN_TRIP);
        }
        if (mAnm.checkFrame(16.0f) && mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
            mAng angle = getAngleY();
            mpToolBank->releaseBalloon(&angle);
        }
        if (mSpeedF <= 0.0f) {
            int toolType = mpToolBank->getToolType();
            if (toolType == HMN_TOOL_BALLOON) {
                mAng angle = getAngleY();
                mpToolBank->releaseBalloon(&angle);
            }
            int anmId = HMN_ANM_TUMBLE_UPC1;
            switch (toolType) {
            case HMN_TOOL_NET:
                anmId = HMN_ANM_TUMBLE_UPB1;
                mpToolBank->setNetAnm1B8(m3d::FORWARD_ONCE, 8.0f);
                break;
            case HMN_TOOL_ROD:
                anmId = HMN_ANM_TUMBLE_UPB1;
                mpToolBank->setPoleTumbleUp(m3d::FORWARD_ONCE, 8.0f);
                break;
            case HMN_TOOL_UMBRELLA:
                anmId = HMN_ANM_TUMBLE_UPB1;
                break;
            case HMN_TOOL_SCOOP:
            case HMN_TOOL_AXE:
                anmId = HMN_ANM_TUMBLE_UPA1;
                break;
            }
            setAnmNoMorph(anmId, m3d::FORWARD_ONCE);
            dBGCF::groundChk_c chk(&mMdlCallback.mHeadPos, 0, 0, 0);
            if (chk.mAttr == BG_ATTR_SAND || chk.mWater == BG_WATER_SEA ||
                dGroundAttr::getSnowWear(&mPos) > dPlayerMgr::sSnowEffectWearThreshold) {
                dEffect::create("af_ply_kokeato", &mPos, &mAngle, NULL);
            }
        }
    } else if (mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
                setState(dPlayerMgr::STATE_RELEASE_BALLOON);
            } else {
                setState(dPlayerMgr::STATE_IDLE);
            }
        } else if (setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP)) {
            if (mpToolBank->getToolType() == HMN_TOOL_BALLOON) {
                mMdl.removePartAnm(1, 0.0f);
            }
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
        }
    }
}

// .text 0x19248
void dAcPlayer_c::initCatchPose() {
    if (mPlayerNo == dNet::getMyNo()) {
        int insect = (mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0;
        if (&dBgm::l_mgr.mEvent != NULL) {
            dBgm::l_mgr.mEvent.muteC(insect);
            dBgm::l_mgr.mEvent.playC(0);
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) {
            int x;
            int z;
            if (dFgobjManager::findPutSpot(0x10, &x, &z, NULL)) {
                mStep = 14;
                mMsgStep = MSG_STEP_QUEUE_ASK_RELEASE;
            } else {
                mStep = 15;
                mMsgStep = MSG_STEP_QUEUE_LET_GO;
            }
        } else {
            mStep = 13;
        }
        startCatchPoseAnm((mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0);
        getCatchPoseParam()->set((mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) != 0);
    } else {
        startCatchPoseAnm(getCatchPoseParam()->mInsect);
        if (mInsectLink.mpInsect == NULL && g_playActorMng != NULL) {
            mInsectLink.set(g_playActorMng->mNetInsects[mPlayerNo]);
            if (mInsectLink.mpInsect != NULL) {
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->initHold();
                mVec3_c scale(1.0f, 1.0f, 1.0f);
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
                    ->setHeldScale(&scale);
            }
        }
    }
}

// .text 0x193B4
void dAcPlayer_c::executeCatchPose() {
    int playerNo = mPlayerNo;
    if (playerNo == dNet::getMyNo()) {
        stepCatchPose();
    } else if (mInsectLink.mpInsect == NULL) {
        if (g_playActorMng != NULL) {
            mInsectLink.set(g_playActorMng->mNetInsects[playerNo]);
            if (mInsectLink.mpInsect != NULL) {
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->initHold();
                mVec3_c scale(1.0f, 1.0f, 1.0f);
                static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
                    ->setHeldScale(&scale);
            }
        }
    } else {
        mVec3_c scale(1.0f, 1.0f, 1.0f);
        static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)
            ->setHeldScale(&scale);
    }
}

// .text 0x1949C
void dAcPlayer_c::endCatchPose(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL) {
            camera->mRequestedMode = camera->mDefaultMode;
        }
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stopC(0xB, 0x1A);
        }
    } else {
        switch (nextState) {
        case dPlayerMgr::STATE_IDLE:
        case dPlayerMgr::STATE_NET_POCKET:
        case dPlayerMgr::STATE_FISHING_PUT_AWAY:
            break;
        default:
            int toolType = mpToolBank->getToolType();
            if (toolType == HMN_TOOL_NET) {
                mpToolBank->setNetWait(m3d::FORWARD_LOOP, 90.0f);
            } else if (toolType == HMN_TOOL_ROD) {
                mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
            }
            break;
        }
    }
}

// .text 0x1957C
void dAcPlayer_c::startCatchPoseAnm(BOOL insect) {
    if (insect) {
        if (setAnm(HMN_ANM_NET_COMPLETE1, m3d::FORWARD_ONCE)) {
            mpToolBank->setNetAnm1A7(m3d::FORWARD_ONCE, 8.0f);
        }
    } else if (setAnm(HMN_ANM_POLE_COMPLETE1, m3d::FORWARD_ONCE)) {
        mpToolBank->setPoleComplete(m3d::FORWARD_ONCE, 8.0f);
    }
}

// .text 0x19600
void dAcPlayer_c::stepCatchPose() {
    switch (mStep) {
    case 13:
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_INSECT) {
            changeStateAfterMsg(dPlayerMgr::STATE_NET_POCKET);
        } else {
            changeStateAfterMsg(dPlayerMgr::STATE_FISHING_PUT_AWAY);
        }
        break;
    case 14:
        switch (mMsgStep) {
        case MSG_STEP_SWAP_INSECT:
        case MSG_STEP_SWAP_FISH:
            if (_2188.isInsect()) {
                changeStateAfterMsg(dPlayerMgr::STATE_NET_POCKET);
            } else {
                changeStateAfterMsg(dPlayerMgr::STATE_FISHING_PUT_AWAY);
            }
            break;
        case MSG_STEP_RELEASE:
            changeStateAfterMsg(dPlayerMgr::STATE_SHOW_CATCH_AT);
            break;
        }
        break;
    case 15:
        changeStateAfterMsg(dPlayerMgr::STATE_SHOW_CATCH_AT);
        break;
    }
}

// .text 0x196DC
void dAcPlayer_c::catchPoseParam_c::set(u8 insect) {
    mInsect = insect;
}

// .text 0x196E4
void dAcPlayer_c::initChestOpen() {
    if (mPlayerNo == dNet::getMyNo()) {
        getChestOpenParam()->set(mTargetPos.x, mTargetPos.z, mChestType);
    } else {
        chestOpenParam_c *param = getChestOpenParam();
        mTargetPos.x = param->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = param->mZ;
        mChestType = param->mType;
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, mPlayerNo)) {
            s16 target = mAngle.y.mAngle;
            switch (getAngleDirection(angle)) {
            case 2:
                target = -0x8000;
                break;
            case 3:
                target = -0x4000;
                break;
            case 0:
                target = 0;
                break;
            case 1:
                target = 0x4000;
                break;
            }
            mTargetAngle = target;
        } else {
            mTargetAngle = mAngle.y.mAngle;
        }
    }
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
    cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    int anmId = HMN_ANM_CHESTA_OPEN1;
    switch (mChestType) {
    case 0:
        anmId = HMN_ANM_CHESTA_OPEN1;
        break;
    case 1:
        anmId = HMN_ANM_CHESTB_OPEN1;
        break;
    case 2:
        anmId = HMN_ANM_CHESTC_OPEN1;
        break;
    }
    setAnm(anmId, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
}

// .text 0x1986C
void dAcPlayer_c::executeChestOpen() {
    sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 1000);
    if (mPlayerNo == dNet::getMyNo()) {
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_CHEST_WAIT);
        }
    } else {
        cLib::addCalcPosXZ2(&mPos, mTargetPos, 0.1f, 2.0f);
        if (mAnm.isStop()) {
            int anmId = HMN_ANM_CHESTA_OPEN_WAIT1;
            switch (mChestType) {
            case 0:
                anmId = HMN_ANM_CHESTA_OPEN_WAIT1;
                break;
            case 1:
                anmId = HMN_ANM_CHESTB_OPEN_WAIT1;
                break;
            case 2:
                anmId = HMN_ANM_CHESTC_OPEN_WAIT1;
                break;
            }
            setAnmNoMorph(anmId, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x19964
void dAcPlayer_c::endChestOpen(int nextState) {}

// .text 0x19968
void dAcPlayer_c::chestOpenParam_c::set(f32 x, f32 z, u8 type) {
    mX = x;
    mZ = z;
    mType = type;
}

// .text 0x19978
void dAcPlayer_c::initChestWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        getChestWaitParam()->set(mChestType);
    } else {
        mChestType = getChestWaitParam()->mType;
    }
    int anmId = HMN_ANM_CHESTA_OPEN_WAIT1;
    switch (mChestType) {
    case 0:
        anmId = HMN_ANM_CHESTA_OPEN_WAIT1;
        break;
    case 1:
        anmId = HMN_ANM_CHESTB_OPEN_WAIT1;
        break;
    case 2:
        anmId = HMN_ANM_CHESTC_OPEN_WAIT1;
        break;
    }
    setAnmNoMorph(anmId, m3d::FORWARD_LOOP);
}

// .text 0x19A18
void dAcPlayer_c::executeChestWait() {
    if (mPlayerNo != dNet::getMyNo()) {
        followRemoteTransform();
    }
}

// .text 0x19A58
void dAcPlayer_c::endChestWait(int nextState) {}

// .text 0x19A5C
void dAcPlayer_c::chestWaitParam_c::set(u8 type) {
    mType = type;
}

// .text 0x19A64
void dAcPlayer_c::initChestClose() {
    if (mPlayerNo == dNet::getMyNo()) {
        getChestCloseParam()->set(mChestType);
    } else {
        mChestType = getChestCloseParam()->mType;
    }
    int anmId = HMN_ANM_CHESTA_OPEN1;
    switch (mChestType) {
    case 0:
        anmId = HMN_ANM_CHESTA_OPEN1;
        break;
    case 1:
        anmId = HMN_ANM_CHESTB_OPEN1;
        break;
    case 2:
        anmId = HMN_ANM_CHESTC_OPEN1;
        break;
    }
    setAnm(anmId, 6.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, FALSE);
    mAnm.setFrame(mAnm.mFrameMax - 1.0f);
}

// .text 0x19B3C
void dAcPlayer_c::executeChestClose() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x19BBC
void dAcPlayer_c::endChestClose(int nextState) {}

// .text 0x19BC0
void dAcPlayer_c::chestCloseParam_c::set(u8 type) {
    mType = type;
}

// .text 0x19BC8
void dAcPlayer_c::initOpenUmbrella() {
    setAnm(HMN_ANM_UMB_OPEN1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    mpToolBank->openUmbrella();
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x19C3C
void dAcPlayer_c::executeOpenUmbrella() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.checkFrame(22.0f)) {
            mToolScale.set(1.0f, 1.0f, 1.0f);
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
        } else if (mAnm.checkFrame(45.0f)) {
            startSound(SE_UmbrellaOpen);
        } else if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        if (mAnm.checkFrame(22.0f)) {
            mToolScale.set(1.0f, 1.0f, 1.0f);
            mActorFlags = (mActorFlags | dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) & ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        } else if (mAnm.checkFrame(45.0f)) {
            startSound(SE_UmbrellaOpen);
        } else if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x19D84
void dAcPlayer_c::endOpenUmbrella(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        mActorFlags = (mActorFlags | dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE) & ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        mToolScale.set(1.0f, 1.0f, 1.0f);
    }
}

// .text 0x19DF8
void dAcPlayer_c::initCloseUmbrella() {
    setAnm(HMN_ANM_UMB_OPEN1, 15.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, TRUE);
    mAnm.setFrame(mAnm.mFrameMax - 1.0f);
    mpToolBank->closeUmbrella();
    if (mPlayerNo != dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    startSound(SE_UmbrellaClose);
}

// .text 0x19E94
void dAcPlayer_c::executeCloseUmbrella() {
    f32 closeFrame = mAnm.mFrameMax - 42.0f;
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.checkFrame(closeFrame)) {
            mToolScale.set(0.0f, 0.0f, 0.0f);
        } else if (mAnm.isStop()) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            if (dDemo::isType(dDemo::TYPE_TALK) || dDemo::isType(dDemo::TYPE_SPEAK)) {
                setState(dPlayerMgr::STATE_DEMO_WAIT);
            }
        }
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        if (mAnm.checkFrame(closeFrame)) {
            mToolScale.set(0.0f, 0.0f, 0.0f);
        } else if (mAnm.isStop()) {
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    }
}

// .text 0x19FE4
void dAcPlayer_c::endCloseUmbrella(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING)) {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        mToolScale.set(0.0f, 0.0f, 0.0f);
    }
}

// .text 0x1A048
void dAcPlayer_c::initShakeUmbrella() {
    _22A4[1] = 0;
    setAnm(HMN_ANM_UMB_ROTATE1, 7.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
    startSound(SE_UmbrellaRotate);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
}

// .text 0x1A0C4
void dAcPlayer_c::endShakeUmbrella(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
}

// .text 0x1A0E0
void dAcPlayer_c::getUmbrellaEffectParam(f32 *y, f32 *scale) {
    dItem::Item held = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->mEquipment.mHeld;
    dItem::Item umbrella(0x4D0);
    if (held.isSame(umbrella)) {
        *y = -3.5f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_ELEGANT_UMBRELLA);
    if (held.isSame(umbrella)) {
        *y = -1.5f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_LACY_PARASOL);
    if (held.isSame(umbrella)) {
        *y = -1.5f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_LEAF_UMBRELLA);
    if (held.isSame(umbrella)) {
        *y = -3.0f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_RIBBON_UMBRELLA);
    if (held.isSame(umbrella)) {
        *y = -2.0f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_PETAL_PARASOL);
    if (held.isSame(umbrella)) {
        *y = -3.0f;
        return;
    }
    umbrella.setFromIndex(dItem::ITEM_IDX_PEACHS_PARASOL);
    if (held.isSame(umbrella)) {
        *scale = 0.75f;
    }
}

// .text 0x1A264
void dAcPlayer_c::initTrip() {
    int anmId = HMN_ANM_TUMBLE_DOWNA1;
    switch (mpToolBank->getToolType()) {
    case HMN_TOOL_NET:
        anmId = HMN_ANM_TUMBLE_DOWNB1;
        mpToolBank->setNetAnm1B7(m3d::FORWARD_ONCE, 8.0f);
        break;
    case HMN_TOOL_ROD:
        anmId = HMN_ANM_TUMBLE_DOWNB1;
        mpToolBank->setPoleTumbleDown(m3d::FORWARD_ONCE, 8.0f);
        break;
    case HMN_TOOL_UMBRELLA:
        anmId = HMN_ANM_TUMBLE_DOWNB1;
        break;
    }
    setAnm(anmId, 9.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    _21B8 = 0.07f;
    mTargetSpeed = 0.0f;
    _22A4[6] = 180;
    getRHandMtx(&mToolMtx);
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NOISY;
    if (mAngle.y.mAngle > 0) {
        _22D2[1] = 0x4000;
    } else {
        _22D2[1] = -0x4000;
    }
    _22A4[7] = 0;
}

// .text 0x1A384
void dAcPlayer_c::endTrip(int nextState) {
    if (mPlayerNo != dNet::getMyNo() && nextState != dPlayerMgr::STATE_IDLE &&
        (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT)) {
        int toolType = mpToolBank->getToolType();
        if (toolType == HMN_TOOL_NET) {
            mpToolBank->setNetWait(m3d::FORWARD_LOOP, 90.0f);
        } else if (toolType == HMN_TOOL_ROD) {
            mpToolBank->setPoleWait(m3d::FORWARD_LOOP, 8.0f);
        }
    }
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NOISY;
}

static const char *l_tripEffects[] = {
    "afi_ply_koke_nml", "af_ply_koke_water", "af_ply_koke_water", "af_ply_koke_snow", "afi_ply_koke_sand",
};

// .text 0x1A42C
void dAcPlayer_c::createTripEffect() {
    createSurfaceEffect(l_tripEffects, &mPos, &mAngle);
}

// .text 0x1A440
void dAcPlayer_c::initReleaseBalloon() {
    if (setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP)) {
        mMdl.removePartAnm(1, 0.0f);
    }
    int myNo = dNet::getMyNo();
    if (mPlayerNo == myNo) {
        dPlayerMngr_c::setLocalPlayerEquip(mPlayerNo, dEquip_c::PART_HELD, dItem::ITEM_ID_NONE);
    } else {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
    mAng angle = getAngleY();
    mpToolBank->releaseBalloon(&angle);
}

// .text 0x1A4DC
void dAcPlayer_c::executeReleaseBalloon() {
    if (mpToolBank->isBalloonGone()) {
        lookAt(NULL, 0x7F, 0x7F);
        if (mPlayerNo == dNet::getMyNo()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
            removeTool();
            mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
        }
    } else {
        lookAt(&mScriptedActionPos, 0x7F, 0x7F);
    }
}

// .text 0x1A580
void dAcPlayer_c::endReleaseBalloon(int nextState) {
    lookAt(NULL, 0x7F, 0x7F);
    if (mPlayerNo == dNet::getMyNo()) {
        removeTool();
    } else if (mActorFlags & dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING) {
        removeTool();
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_EFFECT_PENDING;
    }
}

// .text 0x1A5F4
void dAcPlayer_c::executeFaint() {
    static const int l_waitFrames = 400;

    if (mAnm.checkFrame(36.0f)) {
        startSound(SE_Piyoru);
    } else if (mAnm.checkFrame(90.0f)) {
        startSound(SE_PoisonDown);
    } else if (mAnm.checkFrame(108.0f)) {
        startSound(SE_PiyoruTentou);
    }
    mMtx_c mtx = mMdlCallback.mFeelMtx;
    mVec3_c pos(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
    dWorld::toFieldPosition(&pos, &pos);
    mLevelEffect3.create("af_ply_kizetsu", &pos, NULL, NULL, 1);
    if (mPlayerNo == dNet::getMyNo()) {
        if ((int)_22A4[7] == 110) {
            dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
            if (event != NULL) {
                event->play24();
            }
        }
        if ((int)_22A4[7] == 292) {
            requestHomeScene();
        }
        if (++_22A4[7] > l_waitFrames) {
            _22A4[7] = l_waitFrames;
        }
    }
}

// .text 0x1A79C
void dAcPlayer_c::initPickUpUnitItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        initPickUpUnitItemMine();
    } else {
        initPickUpUnitItemRemote();
    }
}

// .text 0x1A7E8
void dAcPlayer_c::executePickUpUnitItem() {
    if ((mAnmId == HMN_ANM_PICKUP1 || mAnmId == HMN_ANM_KAGU_PUTAWAY1) && mAnm.checkFrame(40.0f)) {
        startSound(SE_gasagoso_return);
    }
    if (mPlayerNo == dNet::getMyNo()) {
        executePickUpUnitItemMine();
    } else if (mAnmId != HMN_ANM_NML_WAIT1) {
        updateTakeItemRemote();
    }
}

// .text 0x1A878
void dAcPlayer_c::endPickUpUnitItem(int nextState) {
    mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_NOT_ALONE;
    if (mPlayerNo != dNet::getMyNo() && (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING)) {
        takeFieldItem(FALSE);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
    }
}

// .text 0x1A8E0
void dAcPlayer_c::initPickUpUnitItemMine() {
    BOOL full = FALSE;
    mStep = 10;
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) || (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NOT_ALONE)) {
        full = TRUE;
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    } else {
        setTargetUnitItem(mPickUpKind);
        if (mTaskKind == FG_MNG_KIND_PICK_UP) {
            putInPocket(&_2188);
            if (mPickUpKind == 0) {
                setAnm(HMN_ANM_PICKUP1, m3d::FORWARD_ONCE);
                initPickUpItemPos();
            } else {
                setAnm(HMN_ANM_KAGU_PUTAWAY1, m3d::FORWARD_ONCE);
                dFgDraw::getItemPosition(&mItemPos, _218C, mTargetUnitX, mTargetUnitZ, TRUE);
                f32 frames = 13.0f;
                if (frames < 1.0f) {
                    frames = 1.0f;
                }
                _21D0[0] = mItemPos.xzDistTo(mMdlCallback.mHandPos) / frames;
            }
        }
    }
    getTakeItemParam()->set(mActionItem.mId, mTargetUnitX, mTargetUnitZ, mPickUpKind, full);
}

// .text 0x1AA3C
void dAcPlayer_c::initPickUpUnitItemRemote() {
    takeItemParam_c *param = getTakeItemParam();
    if (param->mCancel) {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        return;
    }
    u8 unitX = param->mUnitX;
    u8 unitZ = param->mUnitZ;
    mTargetUnitX = unitX;
    mTargetUnitZ = unitZ;
    mActionItem = param->mItem;
    mPickUpKind = param->mLayer;
    if (mPickUpKind == 0) {
        setAnm(HMN_ANM_PICKUP1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        initPickUpItemPos();
    } else {
        setAnm(HMN_ANM_KAGU_PUTAWAY1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
        dFgDraw::getItemPosition(&mItemPos, _218C, mTargetUnitX, mTargetUnitZ, TRUE);
        f32 frames = 13.0f;
        if (frames < 1.0f) {
            frames = 1.0f;
        }
        _21D0[0] = mItemPos.xzDistTo(mMdlCallback.mHandPos) / frames;
    }
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
}

// .text 0x1AB7C
void dAcPlayer_c::executePickUpUnitItemMine() {
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_POCKETS_FULL) || (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_NOT_ALONE)) {
        stepPickUpFullMsg();
        return;
    }
    f32 frame = mAnm.getFrame();
    if (mPickUpKind == 0) {
        f32 s = 1.0f;
        if (frame >= 23.0f && frame < 51.0f) {
            s = 1.0f - 0.05f * (frame - 23.0f);
            if (s < 0.0f) {
                s = 0.0f;
            }
        }
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(12.0f)) {
            if (mTaskKind == FG_MNG_KIND_PICK_UP) {
                takeFieldItem(FALSE);
            }
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(s, s, s));
        } else if (frame > 12.0f && frame < 20.0f) {
            cLib::chasePosXZ(&mItemPos, mShowPos, _21D0[0]);
            sLib::chase(&mItemPos.y, mShowPos.y, _21D0[1]);
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(s, s, s));
        } else if (frame >= 20.0f && frame < 51.0f) {
            setItemPosToHand();
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(s, s, s));
        }
    } else {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_IDLE);
        } else if (mAnm.checkFrame(17.0f)) {
            if (mTaskKind == FG_MNG_KIND_PICK_UP) {
                takeFieldItem(FALSE);
            }
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(1.0f, 1.0f, 1.0f));
        } else if (frame > 17.0f && frame < 27.0f) {
            cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
            setFgObjItem(mActionItem.mId, &mItemPos, &mVec3_c(1.0f, 1.0f, 1.0f));
        } else if (frame >= 27.0f && frame < 45.0f) {
            f32 s = 1.0f - 0.056f * (frame - 27.0f);
            if (s < 0.0f) {
                s = 0.0f;
            }
            mVec3_c scale(s, s, s);
            cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
            setFgObjItem(mActionItem.mId, &mItemPos, &scale);
        }
    }
}

// .text 0x1AE98
void dAcPlayer_c::updateTakeItemRemote() {
    followRemoteTransform();
    if (getTakeItemParam()->mCancel == 0 && mAnmId != HMN_ANM_NML_WAIT1) {
        mVec3_c shrinkScale;
        f32 frame = mAnm.getFrame();
        if (getTakeItemParam()->mLayer == 0) {
            f32 scale = 1.0f;
            if (frame >= 23.0f && frame < 51.0f) {
                scale = 1.0f - 0.05f * (frame - 23.0f);
                if (scale < 0.0f) {
                    scale = 0.0f;
                }
            }
            if (mAnm.isStop()) {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            } else if (mAnm.checkFrame(12.0f)) {
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                    takeFieldItem(FALSE);
                    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
                }
                mVec3_c itemScale(scale, scale, scale);
                setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
            } else if (frame > 12.0f && frame < 20.0f) {
                cLib::chasePosXZ(&mItemPos, mShowPos, _21D0[0]);
                sLib::chase(&mItemPos.y, mShowPos.y, _21D0[1]);
                mVec3_c itemScale(scale, scale, scale);
                setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
            } else if (frame >= 20.0f && frame < 51.0f) {
                setItemPosToHand();
                mVec3_c itemScale(scale, scale, scale);
                setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
            }
        } else {
            if (mAnm.isStop()) {
                setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
            } else if (mAnm.checkFrame(17.0f)) {
                if (mActorFlags & dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING) {
                    takeFieldItem(FALSE);
                    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_FIELD_ACTION_PENDING;
                }
                mVec3_c itemScale(1.0f, 1.0f, 1.0f);
                setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
            } else if (frame > 17.0f && frame < 27.0f) {
                cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
                mVec3_c itemScale(1.0f, 1.0f, 1.0f);
                setFgObjItem(mActionItem.getId(), &mItemPos, &itemScale);
            } else if (frame >= 27.0f && frame < 45.0f) {
                f32 scale = 1.0f - 0.056f * (frame - 27.0f);
                if (scale < 0.0f) {
                    scale = 0.0f;
                }
                shrinkScale.set(scale, scale, scale);
                cLib::chasePos(&mItemPos, mMdlCallback.mHandPos, _21D0[0]);
                setFgObjItem(mActionItem.getId(), &mItemPos, &shrinkScale);
            }
        }
    }
}

// .text 0x1B1CC
void dAcPlayer_c::takeItemParam_c::set(u16 item, u8 unitX, u8 unitZ, u8 layer, u8 cancel) {
    mItem = item;
    mUnitX = unitX;
    mUnitZ = unitZ;
    mLayer = layer;
    mCancel = cancel;
}

// .text 0x1B1E4
void dAcPlayer_c::initFaint() {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->mute28();
        }
    }
    int anmId = HMN_ANM_STING_DOWNA1;
    switch (mpToolBank->getToolType()) {
    case HMN_TOOL_NET:
        anmId = HMN_ANM_STING_DOWNB1;
        mpToolBank->setNetAnm1B5(m3d::FORWARD_ONCE, 40.0f);
        break;
    case HMN_TOOL_ROD:
        anmId = HMN_ANM_STING_DOWNB1;
        mpToolBank->setPoleStingDown(m3d::FORWARD_ONCE, 8.0f);
        break;
    case HMN_TOOL_UMBRELLA:
        anmId = HMN_ANM_STING_DOWNB1;
        break;
    }
    setAnm(anmId, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    if (mActorFlags2 & 1) {
        startSound(SE_TarantulaDamage);
    } else {
        startSound(SE_ScorpionDamage);
    }
    _22A4[7] = 0;
}

// .text 0x1B300
void dAcPlayer_c::endFaint(int nextState) {}

// .text 0x1B304
void dAcPlayer_c::requestHomeScene() {
    static const u16 l_homeEntrances[4] = {0xD001, 0xD002, 0xD003, 0xD004};
    static const f32 l_guestOffsetX[4] = {0.0f, 16.0f, 48.0f, 64.0f};
    f32 offsetX = 0.0f;
    u16 entrance = 0xD014;
    if (dNet::isHost()) {
        dPrivateData_c *pd = dPlayerMgr::Mgr_c::getNetPlayerRaw(mPlayerNo);
        if (pd != NULL && !pd->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
            int home = dSaveData_c::getTown()->mHomes.findCurrentPlayer();
            if (home >= 0 && home < 4) {
                entrance = l_homeEntrances[home];
            }
            offsetX = 0.0f;
        }
    } else {
        entrance = 0xD014;
        offsetX = l_guestOffsetX[mPlayerNo];
    }
    dSTR::entArea_c *area = dSTR::getColBank()->getEntArea(entrance);
    if (area != NULL) {
        mVec3_c pos = area->calcFrontPos(entrance, 1, 48.0f);
        pos.x += offsetX;
        dAPlayer::getSceneChange()->request(0, &pos, 0x75, 0, 2, 2);
    }
}

// .text 0x1B430
void dAcPlayer_c::initFaintGetUp() {
    if (mPlayerNo == dNet::getMyNo()) {
        dBgm::Event_c *event = &dBgm::l_mgr.mEvent;
        if (event != NULL) {
            event->stop24();
        }
    }
    setAnm(HMN_ANM_STING_UP1, m3d::FORWARD_ONCE);
}

// .text 0x1B48C
void dAcPlayer_c::executeFaintGetUp() {
    if (mAnmId != HMN_ANM_NML_WAIT1 && mAnm.checkFrame(60.0f)) {
        startSound(SE_StandUp);
    }
    int no = mPlayerNo;
    if (no == dNet::getMyNo()) {
        if (mAnm.isStop() && dDemo::finishSceneStart() && !dPlayerMgr::canHoldEquippedItem()) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
        }
    } else {
        f32 x;
        f32 z;
        s16 angle;
        if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
            mTargetPos.set(x, mPos.y, z);
            mPos.x = mTargetPos.x;
            mPos.z = mTargetPos.z;
        }
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x1B590
void dAcPlayer_c::endFaintGetUp(int nextState) {}

// .text 0x1B594
void dAcPlayer_c::initMailboxOpen() {
    setAnm(HMN_ANM_POST_OPEN1, 6.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, -0x8000, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    }
}

// .text 0x1B61C
void dAcPlayer_c::executeMailboxOpen() {
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, -0x8000, 0xB61);
        cLib::chasePosXZ(&mPos, mTargetPos, 0.5f);
    } else {
        followRemoteTransform();
    }
}

// .text 0x1B684
void dAcPlayer_c::endMailboxOpen(int nextState) {}

// .text 0x1B688
void dAcPlayer_c::initMailboxClose() {
    if (mPlayerNo == dNet::getMyNo()) {
        mAnm.mPlayMode = m3d::REVERSE_ONCE;
    } else if (mState == dPlayerMgr::STATE_MAILBOX_OPEN) {
        mAnm.mPlayMode = m3d::REVERSE_ONCE;
    } else {
        setAnm(HMN_ANM_POST_OPEN1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, TRUE);
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
    }
}

// .text 0x1B72C
void dAcPlayer_c::executeMailboxClose() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_DEMO_WAIT);
        }
    } else {
        followRemoteTransform();
        if (mAnm.isStop()) {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
}

// .text 0x1B7AC
void dAcPlayer_c::endMailboxClose(int nextState) {}

// .text 0x1B7B0
void dAcPlayer_c::initChat() {
    setAnm(HMN_ANM_MENU_CHAT1, 12.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, TRUE);
}

// .text 0x1B7D8
void dAcPlayer_c::executeChat() {
    if (mPlayerNo == dNet::getMyNo()) {
        if (mAnm.isStop()) {
            setState(dPlayerMgr::STATE_DEMO_START);
        }
    } else if (mAnm.isStop()) {
        setAnm(HMN_ANM_MENU_THINK1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1B850
void dAcPlayer_c::endChat(int nextState) {}

// .text 0x1B854
void dAcPlayer_c::initReceiveItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnm(item.isFish() ? HMN_ANM_TRANSF_PASS1 : HMN_ANM_TRANS_PASS1, m3d::FORWARD_ONCE);
        int kind = item.getKind();
        if (kind == dItem::KIND_GOLD_NET || kind == dItem::KIND_GOLD_FISHINGROD) {
            mReactionKind = dPlayerMgr::REACTION_SKIP;
        }
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1B8EC
void dAcPlayer_c::executeReceiveItem() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    if (mAnmId == HMN_ANM_TRANS_PASS1 || mAnmId == HMN_ANM_TRANSF_PASS1) {
        if (mAnm.isStop()) {
            dItem::Item item = dHandItem::getItem();
            setAnmNoMorph(item.isFish() ? HMN_ANM_TRANSF_PASS2 : HMN_ANM_TRANS_PASS2, m3d::FORWARD_ONCE);
            startSound(SE_gasagoso);
        }
    } else if (mAnm.mPlayMode == m3d::FORWARD_LOOP) {
        if (mAnm.checkFrame(mAnm.mFrameMax - 1.0f) && !dHandItem::isReceiving() &&
            dHandItem::request(dHandItem_c::STATE_APPEAR, this, dHandItem::cDefaultReach)) {
            setState(dPlayerMgr::STATE_PASS_ITEM);
        }
    } else if (mAnm.isStop()) {
        if (!dHandItem::isReceiving()) {
            if (dHandItem::request(dHandItem_c::STATE_APPEAR, this, dHandItem::cDefaultReach)) {
                setState(dPlayerMgr::STATE_PASS_ITEM);
            }
        } else {
            mAnm.mPlayMode = m3d::FORWARD_LOOP;
        }
    }
}

// .text 0x1BA38
void dAcPlayer_c::endReceiveItem(int nextState) {}

// .text 0x1BA3C
void dAcPlayer_c::initPassItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnm(item.isFish() ? HMN_ANM_TRANSF_PASS3 : HMN_ANM_TRANS_PASS3, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1BAB4
void dAcPlayer_c::executePassItem() {
    if (mPlayerNo == dNet::getMyNo() && mAnm.isStop() &&
        dHandItem::request(dHandItem_c::STATE_FALL, this, dHandItem::cDefaultReach)) {
        setState(dPlayerMgr::STATE_PASS_ITEM_WAIT);
    }
}

// .text 0x1BB24
void dAcPlayer_c::endPassItem(int nextState) {}

// .text 0x1BB28
void dAcPlayer_c::initPassItemWait() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnmNoMorph(item.isFish() ? HMN_ANM_TRANSF_PASS_WAIT1 : HMN_ANM_TRANS_PASS_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1BB8C
void dAcPlayer_c::executePassItemWait() {
    if (mPlayerNo == dNet::getMyNo() && !dHandItem::isHolder(this)) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x1BBE0
void dAcPlayer_c::endPassItemWait(int nextState) {}

// .text 0x1BBE4
void dAcPlayer_c::initHandItemReach() {
    static const f32 l_reachScale = 0.7f;

    setAnm(HMN_ANM_NML_WALK1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_LOOP, FALSE);
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos;
        if (!dHandItem::getPos(&pos)) {
            pos = mPos;
        }
        setMoveTarget(&pos, l_reachScale);
        mVec3_c diff = mPos - pos;
        _22A4[7] = 1.5f * EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) / l_reachScale;
        startMoveToTarget();
        if (dHandItem::isInRange(this)) {
            mTargetSpeed = 0.0f;
            mSpeedF = 0.0f;
        }
    } else {
        f32 rate = l_moveParam.mAnmRateScale * l_moveParam.mRunRateMin *
                   mAng(16384.0f * (l_reachScale / l_moveParam.mRunSpeed)).sin();
        if (rate < l_moveParam.mWalkRateMin) {
            rate = l_moveParam.mWalkRateMin;
        } else if (rate > l_moveParam.mRunRateMin) {
            rate = l_moveParam.mRunRateMin;
        }
        mAnm.setRate(rate);
    }
}

// .text 0x1BD6C
void dAcPlayer_c::executeHandItemReach() {
    syncToolAnm();
    checkFootSound();
    if (mPlayerNo == dNet::getMyNo()) {
        mVec3_c pos;
        if (!dHandItem::getPos(&pos)) {
            setState(dPlayerMgr::STATE_HAND_ITEM_TAKE);
            return;
        }
        mShowPos = pos;
        if (_22A4[7] != 0) {
            _22A4[7]--;
        }
        if (dHandItem::isInRange(this) || _22A4[7] == 0) {
            mTargetSpeed = 0.0f;
            mSpeedF = 0.0f;
            setState(dPlayerMgr::STATE_HAND_ITEM_TAKE);
            return;
        }
        s16 angle = dActor_c::targetAngleY(&mPos, &mShowPos);
        _22C2 = angle;
        mAngle3D.y = angle;
        mAngle3D.x = 0;
        mAngle3D.z = 0;
        calcTurn();
        updateMoveAnm(FALSE);
        if (mTargetSpeed) {
            _21B8 = 0.22f;
        } else {
            _21B8 = l_moveParam.mMoveAccel;
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x1BEAC
void dAcPlayer_c::endHandItemReach(int nextState) {}

// .text 0x1BEB0
void dAcPlayer_c::initHandItemTake() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnm(item.isFish() ? HMN_ANM_TRANSF_RECEIVE1 : HMN_ANM_TRANS_RECEIVE1, 12.0f, 0.0f, 1.0f, 16.0f,
               m3d::FORWARD_ONCE, FALSE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1BF4C
void dAcPlayer_c::executeHandItemTake() {
    if (mPlayerNo == dNet::getMyNo() && mAnm.isStop()) {
        if (!dHandItem::isHolder(this)) {
            if (dHandItem::swapActors(this)) {
                dHandItem::request(dHandItem_c::STATE_TAKE, this, dHandItem::cDefaultReach);
            } else {
                return;
            }
        }
        if (!dHandItem::isState(dHandItem_c::STATE_TAKE) &&
            dHandItem::request(dHandItem_c::STATE_HOLD, this, dHandItem::cDefaultReach)) {
            setState(dPlayerMgr::STATE_HAND_ITEM_HOLD);
        }
    }
}

// .text 0x1C008
void dAcPlayer_c::endHandItemTake(int nextState) {}

// .text 0x1C00C
void dAcPlayer_c::initHandItemHold() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnmNoMorph(item.isFish() ? HMN_ANM_TRANSF_RECEIVE2 : HMN_ANM_TRANS_RECEIVE2, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C084
void dAcPlayer_c::executeHandItemHold() {
    if (mPlayerNo == dNet::getMyNo() && mAnm.isStop() &&
        dHandItem::request(dHandItem_c::STATE_SHRINK, this, dHandItem::cDefaultReach)) {
        setState(dPlayerMgr::STATE_HAND_ITEM_PUT_AWAY);
    }
}

// .text 0x1C0F4
void dAcPlayer_c::endHandItemHold(int nextState) {}

// .text 0x1C0F8
void dAcPlayer_c::initHandItemPutAway() {
    if (mPlayerNo == dNet::getMyNo()) {
        dItem::Item item = dHandItem::getItem();
        setAnmNoMorph(item.isFish() ? HMN_ANM_TRANSF_PUTAWAY1 : HMN_ANM_TRANS_PUTAWAY1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C170
void dAcPlayer_c::executeHandItemPutAway() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    if (mAnm.isStop()) {
        if (mReactionKind != dPlayerMgr::REACTION_SKIP) {
            dItem::Item item = dHandItem::getItem();
            int kind = item.getKind();
            if (kind == dItem::KIND_GOLD_NET) {
                mReactionKind = dPlayerMgr::REACTION_GOLD_NET;
            } else if (kind == dItem::KIND_GOLD_FISHINGROD) {
                mReactionKind = dPlayerMgr::REACTION_GOLD_ROD;
            }
        }
        dHandItem::release(this);
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    } else if (mAnm.checkFrame(10.0f)) {
        startSound(SE_gasagoso_return);
    }
}

// .text 0x1C234
void dAcPlayer_c::endHandItemPutAway(int nextState) {}

// .text 0x1C238
void dAcPlayer_c::initGiveItem() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnm(HMN_ANM_MAIL_PASS1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C294
void dAcPlayer_c::executeGiveItem() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    if (mAnm.isStop()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    } else if (mAnm.checkFrame(20.0f)) {
        startSound(SE_gasagoso);
    }
}

// .text 0x1C310
void dAcPlayer_c::endGiveItem(int nextState) {}

// .text 0x1C314
void dAcPlayer_c::initMailReceive() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnm(HMN_ANM_MAIL_RECEIVE1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C370
void dAcPlayer_c::executeMailReceive() {
    if (mPlayerNo == dNet::getMyNo()) {
        setState(dPlayerMgr::STATE_IDLE);
    }
}

// .text 0x1C3B4
void dAcPlayer_c::endMailReceive(int nextState) {}

// .text 0x1C3B8
void dAcPlayer_c::initMailReceiveEnd() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnmNoMorph(HMN_ANM_MAIL_RECEIVE2, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C414
void dAcPlayer_c::executeMailReceiveEnd() {
    if (mPlayerNo == dNet::getMyNo()) {
        setState(dPlayerMgr::STATE_IDLE);
    }
}

// .text 0x1C458
void dAcPlayer_c::endMailReceiveEnd(int nextState) {}

// .text 0x1C45C
void dAcPlayer_c::initMailPutAway() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnmNoMorph(HMN_ANM_MAIL_PUTAWAY1, m3d::FORWARD_ONCE);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C4B8
void dAcPlayer_c::executeMailPutAway() {
    if (mPlayerNo == dNet::getMyNo() && mAnm.isStop()) {
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    }
}

// .text 0x1C50C
void dAcPlayer_c::endMailPutAway(int nextState) {}

// .text 0x1C510
void dAcPlayer_c::initHandItemReturn() {
    if (mPlayerNo == dNet::getMyNo()) {
        setAnm(HMN_ANM_MAIL_PASS1, 60.0f, 0.0f, 1.0f, 16.0f, m3d::REVERSE_ONCE, FALSE);
        mAnm.setFrame(mAnm.mFrameMax - 1.0f);
    } else {
        setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
    }
}

// .text 0x1C5A4
void dAcPlayer_c::executeHandItemReturn() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    if (mAnm.isStop()) {
        dItem::Item item = dHandItem::getItem();
        if (item.getKind() == dItem::KIND_GOLD_WATERING) {
            mReactionKind = dPlayerMgr::REACTION_GOLD_WATERING;
        }
        dHandItem::release(this);
        setState(dPlayerMgr::STATE_DEMO_WAIT);
    } else if (mAnm.checkFrame(40.0f)) {
        startSound(SE_gasagoso_return);
    }
}

// .text 0x1C648
void dAcPlayer_c::endHandItemReturn(int nextState) {}

// .text 0x1C64C
void dAcPlayer_c::initStairsTurn() {}

// .text 0x1C650
void dAcPlayer_c::executeStairsTurn() {
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        if (mExitType == 1) {
            setState(dPlayerMgr::STATE_GO_UPSTAIRS);
        } else {
            setState(dPlayerMgr::STATE_GO_DOWNSTAIRS);
        }
    } else {
        followRemoteTransform();
    }
}

// .text 0x1C6CC
void dAcPlayer_c::endStairsTurn(int nextState) {}

// .text 0x1C6D0
void dAcPlayer_c::initGoUpstairs() {
    setAnm(HMN_ANM_STAIR_UP1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    mShadowAlpha = 1.0f;
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
        int pose = encodeSeatPose(mTargetAngle, 0);
        getGoUpstairsParam()->set(mPos.x, mPos.z, pose);
    } else {
        goUpstairsParam_c *param = getGoUpstairsParam();
        mTargetPos.x = param->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = param->mZ;
        int side;
        mTargetAngle = decodeSeatPose(&side, param->mPose);
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x5DC);
        cLib::chasePosXZ(&mPos, mTargetPos, dPlayerMgr::sRemoteMoveSpeed);
    }
}

// .text 0x1C7CC
void dAcPlayer_c::executeGoUpstairs() {
    static const f32 l_exitFrame = 25.0f;
    static const f32 l_fadeStart = 18.0f;
    static const f32 l_fadeEnd = 45.0f;
    static const f32 l_shadowStep = 1.0f / 27.0f;

    // the model alpha lost per frame over the 27-frame fade out
    static const u8 l_alphaStep[] = {255 / 27};
    f32 frame = mAnm.getFrame();
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        if (mAnm.isStop()) {
            if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED) &&
                dAPlayer::getSceneChange()->requestExit(mExitNo, 2, 2)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
            }
        } else if (mAnm.checkFrame(l_exitFrame)) {
            if (dAPlayer::getSceneChange()->requestExit(mExitNo, 2, 2)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
            }
        } else if (mAnm.checkFrame(l_fadeEnd)) {
            setModelColor(0);
            mShadowAlpha = 0.0f;
        } else if (frame >= l_fadeStart && frame < l_fadeEnd) {
            u32 frameNo = frame;
            u32 step = l_alphaStep[0];
            f32 t = frameNo - l_fadeStart;
            u8 alpha = 255.0f - step * t;
            setModelColor(alpha);
            mShadowAlpha -= l_shadowStep;
            if (mShadowAlpha < 0.0f) {
                mShadowAlpha = 0.0f;
            }
        }
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x5DC);
        cLib::chasePosXZ(&mPos, mTargetPos, dPlayerMgr::sRemoteMoveSpeed);
        if (mAnm.isStop()) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
        } else if (mAnm.checkFrame(l_fadeEnd)) {
            setModelColor(0);
            mShadowAlpha = 0.0f;
        } else if (mAnm.checkFrame(l_fadeStart)) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        } else if (frame > l_fadeStart && frame < l_fadeEnd) {
            u32 frameNo = frame;
            u32 step = l_alphaStep[0];
            f32 t = frameNo - l_fadeStart;
            u8 alpha = 255.0f - step * t;
            setModelColor(alpha);
            mShadowAlpha -= l_shadowStep;
            if (mShadowAlpha < 0.0f) {
                mShadowAlpha = 0.0f;
            }
        }
    }
    checkFootSound(1.0f, 16.0f, 31.0f, 46.0f);
}

// .text 0x1CACC
void dAcPlayer_c::endGoUpstairs(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
    } else {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        setModelColor(0);
        mShadowAlpha = 0.0f;
    }
}

// .text 0x1CB38
void dAcPlayer_c::goUpstairsParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0x1CB48
void dAcPlayer_c::initArriveUpstairs() {
    setAnmNoMorph(HMN_ANM_STAIR_UP2, m3d::FORWARD_ONCE);
    mShadowAlpha = 0.0f;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    mVec3_c pos = mMdlCallback.mBasePos;
    pos.y = mPos.y;
    mObjcPos.set(this, pos, 16.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
}

// .text 0x1CBEC
void dAcPlayer_c::executeArriveUpstairs() {
    static const f32 l_fadeEnd = 18.0f;
    static const f32 l_shadowStep = 1.0f / 18.0f;

    // the model alpha gained per frame over the 18-frame fade in
    static const u8 l_alphaStep[] = {255 / 18};
    mVec3_c pos = mMdlCallback.mBasePos;
    pos.y = mPos.y;
    mObjcPos.mPos = pos;
    if ((mAnmId == HMN_ANM_STAIR_UP2 && mAnm.getFrame() > 5.0f) || mAnmId != HMN_ANM_STAIR_UP2) {
        mObjcPos.regist();
    }
    if (mPlayerNo != dNet::getMyNo()) {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_NML_WAIT1) {
            return;
        }
    }
    f32 frame = mAnm.getFrame();
    if (mAnm.checkFrame(l_fadeEnd)) {
        setModelColor(0xFF);
        mShadowAlpha = 1.0f;
    } else if (frame < l_fadeEnd) {
        setModelColor((u32)frame * l_alphaStep[0]);
        mShadowAlpha = l_shadowStep + mShadowAlpha;
        if (mShadowAlpha > 1.0f) {
            mShadowAlpha = 1.0f;
        }
    } else if (mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (!dPlayerMgr::isCockroachNoticed() && dScene::isCurrentSceneMyHouse() && isInHouseRoom() &&
                dCockroachManager_c::sInstance != NULL && dCockroachManager_c::sInstance->isSpawned()) {
                dPlayerMgr::markCockroachNoticed();
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_COCKROACH;
            }
            dDemo::finishSceneStart();
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
    checkFootSound(1.0f, 15.0f, 34.0f, 56.0f);
}

// .text 0x1CDE0
void dAcPlayer_c::endArriveUpstairs(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    if (mPlayerNo != dNet::getMyNo()) {
        setModelColor(0xFF);
        mShadowAlpha = 1.0f;
    }
}

// .text 0x1CE3C
void dAcPlayer_c::initGoDownstairs() {
    setAnm(HMN_ANM_STAIR_DOWN1, 8.0f, 0.0f, 1.0f, 16.0f, m3d::FORWARD_ONCE, FALSE);
    mShadowAlpha = 1.0f;
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
        int pose = encodeSeatPose(mTargetAngle, 0);
        getGoDownstairsParam()->set(mPos.x, mPos.z, pose);
    } else {
        goDownstairsParam_c *param = getGoDownstairsParam();
        mTargetPos.x = param->mX;
        mTargetPos.y = mPos.y;
        mTargetPos.z = param->mZ;
        int side;
        mTargetAngle = decodeSeatPose(&side, param->mPose);
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x5DC);
        cLib::chasePosXZ(&mPos, mTargetPos, dPlayerMgr::sRemoteMoveSpeed);
    }
}

// .text 0x1CF38
void dAcPlayer_c::executeGoDownstairs() {
    static const f32 l_fadeStart = 18.0f;
    static const f32 l_fadeEnd = 45.0f;
    static const f32 l_shadowStep = 1.0f / 27.0f;

    // the model alpha lost per frame over the 27-frame fade out
    static const u8 l_alphaStep[] = {255 / 27};
    f32 frame = mAnm.getFrame();
    if (mPlayerNo == dNet::getMyNo()) {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0xB61);
        if (mAnm.isStop()) {
            if (!(mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED) &&
                dAPlayer::getSceneChange()->requestExit(mExitNo, 2, 2)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
            }
        } else if (mAnm.checkFrame(l_fadeEnd)) {
            setModelColor(0);
            mShadowAlpha = 0.0f;
            if (dAPlayer::getSceneChange()->requestExit(mExitNo, 2, 2)) {
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
            }
        } else if (frame >= l_fadeStart && frame < l_fadeEnd) {
            u32 frameNo = frame;
            u32 step = l_alphaStep[0];
            f32 t = frameNo - l_fadeStart;
            u8 alpha = 255.0f - step * t;
            setModelColor(alpha);
            mShadowAlpha -= l_shadowStep;
            if (mShadowAlpha < 0.0f) {
                mShadowAlpha = 0.0f;
            }
        }
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, mTargetAngle, 0x5DC);
        cLib::chasePosXZ(&mPos, mTargetPos, dPlayerMgr::sRemoteMoveSpeed);
        if (mAnm.isStop()) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_DISABLED;
        } else if (mAnm.checkFrame(l_fadeEnd)) {
            setModelColor(0);
            mShadowAlpha = 0.0f;
        } else if (mAnm.checkFrame(l_fadeStart)) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        } else if (frame > l_fadeStart && frame < l_fadeEnd) {
            u32 frameNo = frame;
            u32 step = l_alphaStep[0];
            f32 t = frameNo - l_fadeStart;
            u8 alpha = 255.0f - step * t;
            setModelColor(alpha);
            mShadowAlpha -= l_shadowStep;
            if (mShadowAlpha < 0.0f) {
                mShadowAlpha = 0.0f;
            }
        }
    }
    checkFootSound(0.0f, 14.0f, 29.0f, 44.0f);
}

// .text 0x1D220
void dAcPlayer_c::endGoDownstairs(int nextState) {
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EXIT_REQUESTED;
    } else {
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
        setModelColor(0);
        mShadowAlpha = 0.0f;
    }
}

// .text 0x1D28C
void dAcPlayer_c::goDownstairsParam_c::set(f32 x, f32 z, u8 pose) {
    mX = x;
    mZ = z;
    mPose = pose;
}

// .text 0x1D29C
void dAcPlayer_c::initArriveDownstairs() {
    setAnmNoMorph(HMN_ANM_STAIR_DOWN2, m3d::FORWARD_ONCE);
    mShadowAlpha = 0.0f;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    mVec3_c pos;
    getPosAtAngle(&pos, mAngle.y, -64.0f);
    mObjcPos.set(this, pos, 16.0f, 32.0f, dObjc::TYPE_ACTION, dObjc::MASK_DEFAULT, 0xFF, mPlayerNo);
}

// .text 0x1D334
void dAcPlayer_c::executeArriveDownstairs() {
    static const f32 l_fadeEnd = 18.0f;
    static const f32 l_shadowStep = 1.0f / 18.0f;

    // the model alpha gained per frame over the 18-frame fade in
    static const u8 l_alphaStep[] = {255 / 18};
    mVec3_c pos = mMdlCallback.mBasePos;
    pos.y = mPos.y;
    mObjcPos.mPos = pos;
    if ((mAnmId == HMN_ANM_STAIR_DOWN2 && mAnm.getFrame() > 5.0f) || mAnmId != HMN_ANM_STAIR_DOWN2) {
        mObjcPos.regist();
    }
    if (mPlayerNo != dNet::getMyNo()) {
        followRemoteTransform();
        if (mAnmId == HMN_ANM_NML_WAIT1) {
            return;
        }
    }
    f32 frame = mAnm.getFrame();
    if (mAnm.checkFrame(l_fadeEnd)) {
        setModelColor(0xFF);
        mShadowAlpha = 1.0f;
    } else if (frame < l_fadeEnd) {
        setModelColor((u32)frame * l_alphaStep[0]);
        mShadowAlpha = l_shadowStep + mShadowAlpha;
        if (mShadowAlpha > 1.0f) {
            mShadowAlpha = 1.0f;
        }
    } else if (mAnm.isStop()) {
        if (mPlayerNo == dNet::getMyNo()) {
            if (!dPlayerMgr::isCockroachNoticed() && dScene::isCurrentSceneMyHouse() && isInHouseRoom() &&
                dCockroachManager_c::sInstance != NULL && dCockroachManager_c::sInstance->isSpawned()) {
                dPlayerMgr::markCockroachNoticed();
                mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_COCKROACH;
            }
            dDemo::finishSceneStart();
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_RESUME_BGM;
        } else {
            setAnm(HMN_ANM_NML_WAIT1, m3d::FORWARD_LOOP);
        }
    }
    checkFootSound(0.0f, 15.0f, 34.0f, 56.0f);
}

// .text 0x1D528
void dAcPlayer_c::endArriveDownstairs(int nextState) {
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_NO_OBJC;
    if (mPlayerNo != dNet::getMyNo()) {
        setModelColor(0xFF);
        mShadowAlpha = 1.0f;
    }
}

// .text 0x1D584
void dAcPlayer_c::updateWaterEffect() {
    static const f32 l_effectFrame = 25.0f;

    if (mAnmId != HMN_ANM_JYORO_RAISE1) {
        return;
    }
    if (mAnm.isStop()) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) {
            mEffect.retire();
            mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
        }
    } else if (mAnm.checkFrame(10.0f)) {
        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_EFFECT;
        mVec3_c pos;
        getPosAtAngle(&pos, mAngle.y, 30.0f);
        dBGCF::groundChk_c chk(&pos, 0, 0, 0);
        if (mPos.y < chk.getHeight(FALSE)) {
            int unitX = worldToUnit(mPos.x);
            int unitZ = worldToUnit(mPos.z);
            int toX = worldToUnit(pos.x);
            int toZ = worldToUnit(pos.z);
            int distX = cM::absI(unitX - toX);
            int distZ = cM::absI(unitZ - toZ);
            if ((distX == 0 && distZ == 0) ||
                !dBGCF::checkWalkable(unitX, unitZ, dBGCF::getUnitQuarter(&mPos), toX, toZ)) {
                mActorFlags2 &= ~dPlayerMgr::ACTOR_FLAG2_EFFECT;
            }
        }
    } else if (mAnm.checkFrame(l_effectFrame)) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) {
            mEffect.createEffect("af_itm_jyoro_water", &mScriptedActionPos, &mAngle, NULL, wateringParticleCallback, 1);
        }
    } else if (mAnm.getFrame() > l_effectFrame) {
        if (mActorFlags2 & dPlayerMgr::ACTOR_FLAG2_EFFECT) {
            mEffect.follow(&mScriptedActionPos, &mAngle, NULL);
        } else {
            mLevelEffect4.create("af_itm_jyoro_water", &mScriptedActionPos, &mAngle, NULL, 0);
        }
    }
}
