mMtx_c dFishMusObjLotus_c::sBaseMtx;

// 0x13F2C
dFishMusObjLotus_c::dFishMusObjLotus_c() : mBaseNodeName("base") {}

// 0x13F74
dFishMusObjLotus_c::~dFishMusObjLotus_c() {}

// 0x13FCC
BOOL dFishMusObjLotus_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_lotus"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    sBaseNodeID = m3d::getNodeID(mdl, mBaseNodeName);
    if (sBaseNodeID != -1) {
        mMdl.calc(false);
        mMdl.getNodeWorldMtx(sBaseNodeID, &sBaseMtx);
    }
    return TRUE;
}

// 0x140E4
BOOL dFishMusObjLotus_c::execute() {
    mMdl.play();
    mMdl.calc(false);
    mMdl.getNodeWorldMtx(sBaseNodeID, &sBaseMtx);
    return TRUE;
}

// 0x14148
BOOL dFishMusObjWakame00_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_wakame00"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    mAnm.setFrame(cM::rndRange(0.0f, mAnmFrameMax));
    return TRUE;
}

// 0x14238
BOOL dFishMusObjWakame01_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_wakame01"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    mAnm.setFrame(cM::rndRange(0.0f, mAnmFrameMax));
    return TRUE;
}

// 0x14328
BOOL dFishMusObjWaterWeed00_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_mizukusa00"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    mAnm.setFrame(cM::rndRange(0.0f, mAnmFrameMax));
    return TRUE;
}

// 0x14418
BOOL dFishMusObjWaterWeed01_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_mizukusa01"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    mAnm.setFrame(cM::rndRange(0.0f, mAnmFrameMax));
    return TRUE;
}

// 0x14508
BOOL dFishMusObjSeaAnemone_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("m_anemone"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    mAnm.setFrame(cM::rndRange(0.0f, mAnmFrameMax));
    return TRUE;
}

// 0x145F8
dFishMusObjWakame00_c::dFishMusObjWakame00_c() {}

// 0x14634
dFishMusObjWakame00_c::~dFishMusObjWakame00_c() {}

// 0x1468C
BOOL dFishMusObjWakame00_c::execute() {
    mAnm.setRate(mAnmRate);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x146E4
dFishMusObjWakame01_c::dFishMusObjWakame01_c() {}

// 0x14720
dFishMusObjWakame01_c::~dFishMusObjWakame01_c() {}

// 0x14778
BOOL dFishMusObjWakame01_c::execute() {
    mAnm.setRate(mAnmRate);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x147D0
dFishMusObjWaterWeed00_c::dFishMusObjWaterWeed00_c() {}

// 0x1480C
dFishMusObjWaterWeed00_c::~dFishMusObjWaterWeed00_c() {}

// 0x14864
BOOL dFishMusObjWaterWeed00_c::execute() {
    mAnm.setRate(mAnmRate);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x148BC
dFishMusObjWaterWeed01_c::dFishMusObjWaterWeed01_c() {}

// 0x148F8
dFishMusObjWaterWeed01_c::~dFishMusObjWaterWeed01_c() {}

// 0x14950
BOOL dFishMusObjWaterWeed01_c::execute() {
    mAnm.setRate(mAnmRate);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x149A8
dFishMusObjSeaAnemone_c::dFishMusObjSeaAnemone_c() {}

// 0x149E4
dFishMusObjSeaAnemone_c::~dFishMusObjSeaAnemone_c() {}

// 0x14A3C
BOOL dFishMusObjSeaAnemone_c::execute() {
    mAnm.setRate(mAnmRate);
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x14A94
dFishMusObjOctopusPod_c::dFishMusObjOctopusPod_c() {}

// 0x14AD0
dFishMusObjOctopusPod_c::~dFishMusObjOctopusPod_c() {}

// 0x14B28
BOOL dFishMusObjOctopusPod_c::create(nw4r::g3d::ResFile res) {
    mMdl.create(res.GetResMdl(0), &mAllocator, 0, 1, NULL);
    mMtx_c mtx;
    mtx.trans(mPos.x, mPos.y, mPos.z);
    mMdl.setLocalMtx(&mtx);
    return TRUE;
}

// 0x14BA0
BOOL dFishMusObjOctopusPod_c::execute() {
    mMdl.calc(false);
    return TRUE;
}
