// The New Year's countdown board (REL d_a_str_cdbbsNP). See
// include/game/game/d_a_str_cdbbs.hpp.
// REL .text 0xCC..0x135C, .ctors 0x0..0x4, .rodata 0x0..0x38, .data
// 0x0..0x380, .bss 0x8..0x2C.
#include <game/game/d_a_str_cdbbs.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_world.hpp>

// Debug parameters (the HIO tool's values; RTTI "hostIO_c"). The vtable
// pointer follows _00 (declared before the first virtual function).
class hostIO_c {
  public:
    hostIO_c() : _00(0) {
    }

    /* 0x00 */ u8 _00;

    virtual ~hostIO_c() {
    }
}; // size 0x8

namespace dAStrCdbbs {

static hostIO_c l_HIO;
// The countdown's time left (getRemainTime, every frame in execute).
static u32 l_remainHour;
static u32 l_remainMin;
static u32 l_remainSec;

// .text:0xCC
BASE_PROFILE(A_STR_CDBBS, daStrCount_c)

// .text:0x1A8
void getRemainTime(u32 *hour, u32 *min, u32 *sec) {
    dTime_c now = *dTime_c::getCurrent();
    if (now.month != MONTH_DECEMBER || now.mday != COUNTDOWN_MDAY) {
        *sec = 0;
        *min = 0;
        *hour = 0;
        return;
    }
    u32 remain =
        TIME_SECONDS_PER_DAY - (now.sec + now.min * TIME_SECONDS_PER_MINUTE +
                                now.hour * TIME_SECONDS_PER_HOUR);
    *hour = remain / TIME_SECONDS_PER_HOUR;
    remain -= *hour * TIME_SECONDS_PER_HOUR;
    *min = remain / TIME_SECONDS_PER_MINUTE;
    remain -= *min * TIME_SECONDS_PER_MINUTE;
    *sec = remain;
}

// .text:0x2B8
void calcPartMtx(mMtx_c *mtx, const mVec3_c *pos, const mVec3_c *offset) {
    mVec3_c curved;
    mAng angle = dWorld::curvePositionSimple(&curved, pos);
    mtx->trans(curved);
    mtx->XrotM(angle);
    mtx->concat(mMtx_c::createTrans(*offset));
}

// The digits' and colons' x on the board.
static const f32 l_numberPosX[DIGIT_NUM] = {-27.2f, -17.6f, -4.8f,
                                            4.8f,   17.6f,  27.2f};

// .text:0x348
BOOL number_c::create(nw4r::g3d::ResFile file, u32 idx) {
    if (idx < DIGIT_NUM && !mResFile.IsValid()) {
        mResFile = file;
        mIdx = idx;
        mPosX = l_numberPosX[idx];
        nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("obj_cdbbs_number");
        mAllocator_c *allocator = dStrm_c::getAllocator();
        mMdl.create(mdl, allocator, nw4r::g3d::ScnMdl::ANM_TEXSRT, 1, NULL);
        nw4r::g3d::ResAnmTexSrt anm =
            mResFile.GetResAnmTexSrt("obj_cdbbs_number");
        mAnm.create(mdl, anm, allocator, NULL, 1);
        mAnm.setPlayMode(m3d::FORWARD_LOOP, 0);
        mMdl.setAnm(mAnm);
        update();
        mAnm.setFrame(mDigit, 0);
        return TRUE;
    }
    return FALSE;
}

// .text:0x4A0
BOOL number_c::execute() {
    update();
    mAnm.setFrame(mDigit, 0);
    return TRUE;
}

// .text:0x4FC
BOOL number_c::draw(const mVec3_c *pos) {
    mVec3_c offset(mPosX, PART_OFFSET_Y, PART_OFFSET_Z);
    mMtx_c mtx;
    calcPartMtx(&mtx, pos, &offset);
    mMdl.setLocalMtx(&mtx);
    mMdl.entry();
    return TRUE;
}

// .text:0x574
BOOL number_c::remove() {
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    if (mAnm.IsBound()) {
        mAnm.remove();
    }
    return TRUE;
}

// .text:0x5E0
void number_c::update() {
    dTime_c now = *dTime_c::getCurrent();
    // (Not in February: the reason is unknown.)
    if (now.month == MONTH_FEBRUARY) {
        return;
    }
    switch (mIdx) {
    case DIGIT_HOUR_TENS:
        // (% 2: the target's extrwi keeps one bit.)
        mDigit = l_remainHour / 10 % 2;
        break;
    case DIGIT_HOUR_ONES:
        mDigit = l_remainHour % 10;
        break;
    case DIGIT_MIN_TENS:
        mDigit = l_remainMin / 10;
        break;
    case DIGIT_MIN_ONES:
        mDigit = l_remainMin % 10;
        break;
    case DIGIT_SEC_TENS:
        mDigit = l_remainSec / 10;
        break;
    case DIGIT_SEC_ONES:
        mDigit = l_remainSec % 10;
        break;
    }
}

static const f32 l_colonPosX[COLON_NUM] = {-11.2f, 11.2f};

// .text:0x768
BOOL colon_c::create(nw4r::g3d::ResFile file, u32 idx) {
    if (idx < COLON_NUM && !mResFile.IsValid()) {
        mResFile = file;
        mIdx = idx;
        mPosX = l_colonPosX[idx];
        nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("obj_cdbbs_colon");
        mMdl.create(mdl, dStrm_c::getAllocator(), 0);
        return TRUE;
    }
    return FALSE;
}

// .text:0x818
BOOL colon_c::execute() {
    return TRUE;
}

// .text:0x820
BOOL colon_c::draw(const mVec3_c *pos) {
    mVec3_c offset(mPosX, PART_OFFSET_Y, PART_OFFSET_Z);
    mMtx_c mtx;
    calcPartMtx(&mtx, pos, &offset);
    mMdl.setLocalMtx(&mtx);
    mMdl.entry();
    return TRUE;
}

// .text:0x898
BOOL colon_c::remove() {
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    return TRUE;
}

} // namespace dAStrCdbbs

using namespace dAStrCdbbs;

// .text:0x8D4: the board's flag shows the town flag's design.
void daStrCount_c::beforeCreateMdl() {
    dSaveTown_c *town = dSaveData_c::getTown();
    if (!mFlagLoader.loadDesignInPlace(
            &town->mBuilding.mTownFlag, dHeap::strHeap_p, ORG_DESIGN_WIDTH,
            ORG_DESIGN_HEIGHT * ORG_DESIGN_TEX_NUM)) {
        return;
    }
    nw4r::g3d::ResFile file(dSTR::getBrresBank()->getBrres(
        dSTR::getBuildingResName(BUILDING_INDEX(BUILDING_COUNTDOWN_BOARD))));
    file.Release();
    file.Bind(nw4r::g3d::ResFile(mFlagLoader.getData()));
    file.Bind(file);
}

// .text:0x97C
void daStrCount_c::afterCreateMdl() {
    void *brres = dSTR::getBrresBank()->getBrres("obj_cdnum");
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    u8 hide = FALSE;
    if (player != NULL && player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)) {
        hide = TRUE;
    }
    mHideCount = hide;
    if (!hide) {
        for (u32 i = 0; i < DIGIT_NUM; i++) {
            mNumbers[i].create(nw4r::g3d::ResFile(brres), i);
        }
        for (u32 i = 0; i < COLON_NUM; i++) {
            mColons[i].create(nw4r::g3d::ResFile(brres), i);
        }
    }
    mAllocator_c *allocator = dStrm_c::getAllocator();
    nw4r::g3d::ResAnmTexSrt anm = mResFile.GetResAnmTexSrt("obj_cdbbs_cd");
    mBoardAnm.create(mResFile.GetResMdl(0), anm, allocator, NULL, 1);
    mBoardAnm.setPlayMode(m3d::FORWARD_LOOP, 0);
    mMdl.setAnm(mBoardAnm);
    dBase_c::createBase(fProf::COUNTDOWN_FLAG, this, 0, 0);
}

// .text:0xAF0
void daStrCount_c::drawMdl() {
    daSTR_c::drawMdl();
    dTime_c now = *dTime_c::getCurrent();
    if (now.month == MONTH_DECEMBER && now.mday == COUNTDOWN_MDAY &&
        !mHideCount) {
        for (u32 i = 0; i < DIGIT_NUM; i++) {
            mNumbers[i].draw(&mPos);
        }
        for (u32 i = 0; i < COLON_NUM; i++) {
            mColons[i].draw(&mPos);
        }
    }
}

// .text:0xBE4
int daStrCount_c::execute() {
    getRemainTime(&l_remainHour, &l_remainMin, &l_remainSec);
    if (!mHideCount) {
        for (u32 i = 0; i < DIGIT_NUM; i++) {
            mNumbers[i].execute();
        }
        for (u32 i = 0; i < COLON_NUM; i++) {
            mColons[i].execute();
        }
    }
    dTime_c now = *dTime_c::getCurrent();
    if (now.month == MONTH_JANUARY && now.mday <= NEW_YEAR_LAST_MDAY &&
        !mHideCount) {
        if (!mIsNewYear) {
            mBoardAnm.setAnm(mMdl, mResFile.GetResAnmTexSrt("obj_cdbbs_ny"), 0,
                             m3d::FORWARD_LOOP);
            mBoardAnm.setFrame(0.0f, 0);
            mMdl.setAnm(mBoardAnm);
            mIsNewYear = TRUE;
        }
    } else if (mIsNewYear) {
        mBoardAnm.setAnm(mMdl, mResFile.GetResAnmTexSrt("obj_cdbbs_cd"), 0,
                         m3d::FORWARD_LOOP);
        mBoardAnm.setFrame(0.0f, 0);
        mMdl.setAnm(mBoardAnm);
        mIsNewYear = FALSE;
    }
    mBoardAnm.play();
    return SUCCEEDED;
}

// .text:0xDE4
int daStrCount_c::doDelete() {
    for (u32 i = 0; i < DIGIT_NUM; i++) {
        mNumbers[i].remove();
    }
    for (u32 i = 0; i < COLON_NUM; i++) {
        mColons[i].remove();
    }
    if (mBoardAnm.IsBound()) {
        mBoardAnm.remove();
    }
    if (mFlagLoader.release()) {
        return SUCCEEDED;
    }
    return NOT_READY;
}
