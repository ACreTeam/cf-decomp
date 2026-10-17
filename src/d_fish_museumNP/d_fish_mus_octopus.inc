// 0xD1FC
BOOL dFishMusOctopus_c::create(nw4r::g3d::ResFile res) {
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.m3d::smdl_c::create(mdl, &mAllocator, 0, 1, NULL);
    mAnm.create(mdl, res.GetResAnmChr("swim"), &mAllocator, NULL);
    mMdl.setAnm(mAnm);
    mpMdl = &mMdl;
    calcMtx();
    dBgUtil::getNodePos(&mPos, "locator_takotsubo_00", -1, -1);
    init();
    return TRUE;
}

// 0x103C4
dFishMusOctopus_c::dFishMusOctopus_c() {}

// 0x10400
dFishMusOctopus_c::~dFishMusOctopus_c() {}

// 0x10458
BOOL dFishMusOctopus_c::execute() {
    mAnmRate = mpData->mAnmRateSlow;
    mAnm.setRate(mAnmRate);
    calcMtx();
    mMdl.play();
    mMdl.calc(false);
    return TRUE;
}

// 0x104C0
BOOL dFishMusOctopus_c::draw() {
    mMdl.entry();
    return TRUE;
}

// 0x104F0
BOOL dFishMusOctopus_c::doDelete() {
    mMdl.remove();
    mAnm.remove();
    return TRUE;
}

// 0x10544
void dFishMusOctopus_c::init() {
    mpData = &dFishMuseum::sFishData[mFishIdx];
    mAngle.y = 0;
}
