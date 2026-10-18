// The balloon manager (REL d_balloon_managerNP). The same source as the
// manager half of d_a_balloon.cpp (REL d_a_balloonNP, which adds the balloon
// actor); see include/game/game/d_a_balloon.hpp.
#include <game/game/d_a_balloon.hpp>
#include <game/cLib/c_math.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_str.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <cstring>

// Debug parameters of the balloons' sway (the HIO tool's values).
class dBalloonMng_HIO_c {
  public:
    dBalloonMng_HIO_c() {
        mForceSpawn = 0;
        _05 = 0;
        mBobAmp = 0.0625f;
        mSwayXAmp = 0.05f;
        mSwayAngleAmp = 0.15f;
        mSwayZAmp = 0.05f;
        mBobPeriod = 4.0f;
        mSwayXPeriod = 5.0f;
        _20 = 5.0f;
        mSwayZPeriod = 4.5f;
        _06 = 0;
    }
    virtual ~dBalloonMng_HIO_c() {
    }

    /* 0x04 */ u8 mForceSpawn; // spawn a balloon at the next minute check
    /* 0x05 */ u8 _05;
    /* 0x06 */ u8 _06;
    /* 0x08 */ f32 mBobAmp;   // up and down
    /* 0x0C */ f32 mSwayXAmp; // one balloon: sideways
    /* 0x10 */ f32
        mSwayAngleAmp;         // three balloons: their strings' angle (degrees)
    /* 0x14 */ f32 mSwayZAmp;  // one balloon: back and forth
    /* 0x18 */ f32 mBobPeriod; // seconds
    /* 0x1C */ f32 mSwayXPeriod;
    /* 0x20 */ f32 _20;
    /* 0x24 */ f32 mSwayZPeriod;
}; // size 0x28

namespace dABalloon {
BASE_PROFILE(BALLOON_MNG, dBalloonMng_c);
} // namespace dABalloon

dBalloonMng_c *dBalloonMng_c::s_instance;

static dBalloonMng_HIO_c l_HIO;

// .text:0xFC
dBalloonMng_c::dBalloonMng_c()
    : m_frmHeap_p(nullptr), mColor(0), mChance(0.125f), mMinute(-1), mSent(0),
      mGold(0) {
}

// .text:0x168
dBalloonMng_c::~dBalloonMng_c() {
}

// .text:0x1D8
int dBalloonMng_c::create() {
    if (m_frmHeap_p == nullptr) {
        // The balloon manager's heap ("Baloon" sic).
        m_frmHeap_p =
            mHeap::createFrmHeap(0x7800, dHeap::localHeap_p,
                                 "dBaloonMng_c::m_frmHeap_p：風船マネージャー",
                                 0x20, mHeap::OPT_NONE);
        mAllocator.attach(m_frmHeap_p, 0x20);
    }
    s_instance = this;
    mChance = 0.125f * cM::rndRange(1, 8);
    return SUCCEEDED;
}

// .text:0x288
int dBalloonMng_c::execute() {
    calc();
    return SUCCEEDED;
}

// .text:0x2AC
int dBalloonMng_c::draw() {
    return SUCCEEDED;
}

// .text:0x2B4
int dBalloonMng_c::doDelete() {
    if (m_frmHeap_p != nullptr) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = nullptr;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x304
void dBalloonMng_c::calc() {
    dTime_c now = *dTime_c::getCurrent();
    int min = now.min;
    if (dNet::isSessionActive()) {
        return;
    }
    if (mSent) {
        return;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_DEMO)) {
        return;
    }
    if (dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(
            PRIVATE_FLAG0_PART_TIME_JOB)) {
        return;
    }
    if (!updateMinute(min) && !l_HIO.mForceSpawn) {
        return;
    }
    if (min % 10 != 4 && !l_HIO.mForceSpawn) {
        return;
    }
    l_HIO.mForceSpawn = 0;
    if (cM::rndRange(0.0f, 0.999999f) < mChance) {
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
        if (!player->isFlag0(PRIVATE_FLAG0_BALLOON_GOLD_PACHINKO) &&
            player->mShotBalloonNum == 8 && cM::rndRange(0.0f, 1.0f) < 0.25f) {
            mGold = 1;
        }
        mColor = cM::rndRange(0, 8);
        dActor_c::construct(fProf::BALLOON, this, 0, nullptr, nullptr);
        mSent = 1;
        mChance = 0.125f;
    } else {
        mChance += 0.125f;
    }
}

// .text:0x4F0
BOOL dBalloonMng_c::updateMinute(int minute) {
    if (mMinute == minute) {
        return FALSE;
    }
    mMinute = minute;
    return TRUE;
}
