// The insect rooms of the museum (REL d_insect_museumNP). See include/game/game/d_insect_museum.hpp.
#include <game/game/d_insect_museum.hpp>
#include <game/game/d_ins_mus_beetle.hpp>
#include <game/game/d_ins_mus_butterfly.hpp>
#include <game/game/d_ins_mus_cicada.hpp>
#include <game/game/d_ins_mus_dragonfly.hpp>
#include <game/game/d_ins_mus_grasshopper.hpp>
#include <game/game/d_ins_mus_flower.hpp>
#include <game/game/d_ins_mus_bee.hpp>
#include <game/game/d_ins_mus_moth.hpp>
#include <game/game/d_ins_mus_strider.hpp>
#include <game/game/d_ins_mus_firefly.hpp>
#include <game/game/d_ins_mus_mole_cricket.hpp>
#include <game/game/d_ins_mus_flea.hpp>
#include <game/game/d_ins_mus_pill_bug.hpp>
#include <game/game/d_ins_mus_ant.hpp>
#include <game/game/d_ins_mus_tumble_bug.hpp>
#include <game/game/d_ins_mus_hornet.hpp>
#include <game/game/d_ins_mus_mosquito.hpp>
#include <game/game/d_ins_mus_spider.hpp>
#include <game/game/d_ins_mus_tarantula.hpp>
#include <game/game/d_ins_mus_diving_beetle.hpp>
#include <game/game/d_ins_mus_stump.hpp>
#include <game/game/d_ins_mus_leaf.hpp>
#include <game/game/d_ins_mus_beetle_battle.hpp>
#include <game/game/d_ins_mus_create.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_shadow.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <lib/egg/math/eggMath.h>
#include <lib/egg/math/eggQuat.h>
#include <nw4r/ut.h>
#include <revolution/mtx.h>
#include <float.h>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace dInsectMuseum;

namespace dInsectMuseum {

struct areaData_s {
    /* 0x00 */ const char *mNode;
    /* 0x04 */ Vec mOffset;
    /* 0x10 */ f32 mRadiusX;
    /* 0x14 */ f32 mRadiusZ;
}; // size 0x18

areaData_s sAreaData[InsectRoomAreaManager_c::AREA_NUM] = {
    {"flower03", {0.0f, 0.0f, 0.0f}, 70.0f, 70.0f},
    {"flower08", {-0.8f, 7.8f, 4.9f}, 2.0f, 2.0f},
    {"arechi_ike00", {-44.0f, 5.0f, -30.0f}, 138.0f, 45.0f},
    {"arechi_ike00", {-117.0f, 5.0f, -30.0f}, 30.0f, 30.0f},
    {"arechi_ike00", {-57.0f, 5.0f, -50.0f}, 10.0f, 10.0f},
    {"arechi_ike00", {-95.0f, 5.0f, -10.0f}, 50.0f, 10.0f},
    {"ike00", {-25.0f, 0.0f, -12.0f}, 15.0f, 20.0f},
    {"ike00", {25.0f, 0.0f, -3.0f}, 28.0f, 18.0f},
    {"ike00", {-20.0f, 5.0f, -16.0f}, 60.0f, 45.0f},
    {"ike_flower01", {-23.0f, 0.0f, 6.5f}, 60.0f, 45.0f},
    {"ike_flower01", {0.0f, 0.0f, -30.0f}, 10.0f, 10.0f},
    {"ike_flower00", {2.0f, 10.0f, 4.0f}, 2.0f, 2.0f},
    {"nettai_rafflesia", {0.0f, 0.0f, 0.0f}, 25.0f, 25.0f},
    {"nettai_rafflesia", {25.0f, 0.0f, 0.0f}, 10.0f, 10.0f},
    {"nettai_yashi02", {5.0f, 0.0f, 47.0f}, 15.0f, 15.0f},
    {"nettai_yashi03", {10.0f, 0.0f, 100.0f}, 5.0f, 40.0f},
    {"F2_wood00", {15.0f, 0.0f, 58.0f}, 36.0f, 35.0f},
    {"F2_wood01", {10.0f, 0.0f, -70.0f}, 15.0f, 50.0f},
    {"F2_kirikabu01", {-6.5f, 1.0f, -1.0f}, 3.0f, 8.0f},
    {"F2_kirikabu01", {5.5f, 1.0f, -1.0f}, 3.0f, 8.0f},
    {"F2_kirikabu00", {0.0f, 15.0f, 0.0f}, 3.0f, 3.0f},
    {"F3_wood02", {155.0f, 0.0f, 90.0f}, 150.0f, 70.0f},
    {"F3_wood06", {30.0f, 0.0f, -10.0f}, 200.0f, 20.0f},
    {"F3_lamp", {0.0f, 0.0f, 0.0f}, 15.0f, 15.0f},
    {"nettai_yashi02", {-42.0f, 0.0f, -48.0f}, 30.0f, 38.0f},
};

} // namespace dInsectMuseum

static inline f32 dotXZ(const mVec3_c &a, const mVec3_c &b) {
    return a.x * b.x + a.z * b.z;
}

static inline void lerpTo(mVec3_c *v, const mVec3_c &to, f32 rate) {
    *v = *v * (1.0f - rate) + to * rate;
}

static inline f32 clamp01(f32 rate) {
    return rate < 0.0f ? 0.0f : (rate > 1.0f ? 1.0f : rate);
}

static inline bool isZero(f32 v) {
    return std::fabs(v) <= FLT_EPSILON;
}

// 0xCC
InsectRoomAreaManager_c::InsectRoomAreaManager_c() {}

// 0x114
InsectRoomAreaManager_c::~InsectRoomAreaManager_c() {}

// 0x178
void InsectRoomAreaManager_c::init() {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    for (u32 i = 0; i < AREA_NUM; i++) {
        areaData_s *data = &sAreaData[i];
        dBgUtil::getNodePos(&pos, data->mNode, -1, -1);
        pos.x += data->mOffset.x;
        pos.y += data->mOffset.y;
        pos.z += data->mOffset.z;
        mAreas[i].set(pos, data->mRadiusX, data->mRadiusZ);
    }
}

// 0x23C
void InsectRoomAreaManager_c::execute() {}

// 0x240
InsectRoomArea_c::InsectRoomArea_c() : mDrawColor(0), mCenter(0.0f, 0.0f, 0.0f), mRadiusX(0.0f), mRadiusZ(0.0f) {}

// 0x268
InsectRoomArea_c::~InsectRoomArea_c() {}

// 0x2A8
void InsectRoomArea_c::set(const mVec3_c &center, f32 radiusX, f32 radiusZ) {
    mCenter = center;
    mRadiusX = radiusX;
    mRadiusZ = radiusZ;
    calcMtx();
}

// 0x2CC
void InsectRoomArea_c::clampTarget(mVec3_c *out, const mVec3_c *from, const mVec3_c *target) const {
    mVec3_c localFrom(0.0f, 0.0f, 0.0f);
    mVec3_c localTarget(0.0f, 0.0f, 0.0f);
    PSMTXMultVec(mInvMtx, *from, localFrom);
    PSMTXMultVec(mInvMtx, *target, localTarget);
    localFrom.y = 0.0f;
    localTarget.y = 0.0f;
    if (1.0f <= localTarget.xzLenSq()) {
        mVec3_c dir = localTarget - localFrom;
        f32 a = dir.xzLenSq();
        f32 c = localFrom.xzLenSq() - 1.0f;
        f32 b = dotXZ(localFrom, dir);
        f32 absA = nw4r::math::FAbs(a);
        f32 disc = b * b - c * a;
        if (0.01f < disc) {
            f32 t = 0.8f * ((-b - nw4r::math::FSqrt(disc)) / absA);
            mVec3_c pos = localFrom + dir * t;
            PSMTXMultVec(mMtx, pos, pos);
            *out = pos;
        } else {
            mAng angle = cM::rndRange(DEG_TO_ANGLE(180), -DEG_TO_ANGLE(180));
            mVec3_c pos(0.0f, 0.0f, 0.3f); // dead initialiser needed to match
            pos.x = angle.cos();
            pos.y = 0.0f;
            pos.z = angle.sin();
            PSMTXMultVec(mMtx, pos, pos);
            *out = pos;
        }
    } else {
        *out = *target;
    }
}

// 0x540
bool InsectRoomArea_c::isInside(const mVec3_c *pos) const {
    mVec3_c local(0.0f, 0.0f, 0.0f);
    PSMTXMultVec(mInvMtx, *pos, local);
    local.y = 0.0f;
    if (local.xzLenSq() < 1.0f) {
        return true;
    }
    return false;
}

// 0x5BC
void InsectRoomArea_c::calcMtx() {
    mMtx_c scale, trans;
    PSMTXScale(scale, getRadiusX(), 1.0f, getRadiusZ());
    PSMTXTrans(trans, mCenter.x, mCenter.y, mCenter.z);
    PSMTXIdentity(mMtx);
    PSMTXConcat(mMtx, trans, mMtx);
    PSMTXConcat(mMtx, scale, mMtx);
    mInvMtx = mMtx;
    PSMTXInverse(mInvMtx, mInvMtx);
}

// 0x6A4
BOOL InsectRoomLocatorManager_c::create() {
    if (!isBgReady()) {
        return FALSE;
    }
    if (!mAllocator.hasHeap()) {
        mFrmHeap = mHeap::createFrmHeap(0x2000, dHeap::localHeap_p, "InsectRoomLocatorManager_c::heap", 0x20,
                                        mHeap::OPT_NONE);
        setHeap(mFrmHeap);
    }
    createFlower();
    createPond();
    createWasteland();
    createTropical();
    createF1();
    createF2();
    createF3();
    return TRUE;
}

// 0x768
void InsectRoomLocatorManager_c::addLocators(const char *name, InsectRoomLocator_c **list, bool numbered) {
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    int idx = 0;
    InsectRoomLocator_c **link = list;
    do {
        char nodeName[InsectRoomLocator_c::NAME_LEN];
        if (numbered == true) {
            snprintf(nodeName, sizeof(nodeName), "%s%02d", name, idx);
        } else {
            snprintf(nodeName, sizeof(nodeName), "%s", name);
        }
        if (!dBgUtil::getNodePos(&pos, nodeName, -1, -1)) {
            break;
        }
        *link = (InsectRoomLocator_c *)mAllocator.alloc(sizeof(InsectRoomLocator_c));
        (*link)->mPos = pos;
        strcpy((*link)->mName, nodeName);
        (*link)->mNext = NULL;
        idx++;
        link = &(*link)->mNext;
    } while (numbered);
}

// 0x880
void InsectRoomLocatorManager_c::createFlower() {
    addLocators("flower", &mFlower, true);
}

// 0x894
void InsectRoomLocatorManager_c::createPond() {
    addLocators("ike_flower", &mPond, true);
    addLocators("ike", &getLast(mPond)->mNext, true);
    addLocators("ike_etc", &getLast(mPond)->mNext, true);
    addLocators("ike_hotaru", &getLast(mPond)->mNext, true);
}

// 0x93C
void InsectRoomLocatorManager_c::createWasteland() {
    addLocators("arechi", &mWasteland, true);
    addLocators("arechi_ike", &getLast(mWasteland)->mNext, true);
}

// 0x99C
void InsectRoomLocatorManager_c::createTropical() {
    addLocators("nettai_yashi", &mTropical, true);
    addLocators("nettai_rafflesia", &getLast(mTropical)->mNext, false);
}

// 0x9FC
void InsectRoomLocatorManager_c::createF1() {
    addLocators("F1_panel", &mF1, true);
}

// 0xA10
void InsectRoomLocatorManager_c::createF2() {
    addLocators("F2_wood", &mF2, true);
    addLocators("F2_kirikabu", &getLast(mF2)->mNext, true);
    addLocators("F2_panel", &getLast(mF2)->mNext, true);
}

// 0xA98
void InsectRoomLocatorManager_c::createF3() {
    addLocators("F3_wood", &mF3, true);
    addLocators("F3_lamp", &getLast(mF3)->mNext, false);
    addLocators("F3_panel", &getLast(mF3)->mNext, true);
}

namespace dInsectMuseum {

const insectPlace_s sInsectPlace[INSECT_NUM] = {
    {"flower06", dInsMusBase_c::ROOM_FLOWER},
    {"flower00", dInsMusBase_c::ROOM_FLOWER},
    {"flower01", dInsMusBase_c::ROOM_FLOWER},
    {"flower05", dInsMusBase_c::ROOM_FLOWER},
    {"F3_wood04", dInsMusBase_c::ROOM_F3},
    {"flower02", dInsMusBase_c::ROOM_FLOWER},
    {"flower04", dInsMusBase_c::ROOM_FLOWER},
    {"ike_etc00", dInsMusBase_c::ROOM_POND},
    {"F2_panel01", dInsMusBase_c::ROOM_F2},
    {"F3_lamp", dInsMusBase_c::ROOM_F3},
    {"F2_wood02", dInsMusBase_c::ROOM_F2},
    {"ike_flower01", dInsMusBase_c::ROOM_POND},
    {"F3_wood02", dInsMusBase_c::ROOM_F3},
    {"arechi00", dInsMusBase_c::ROOM_WASTELAND},
    {"arechi00", dInsMusBase_c::ROOM_WASTELAND},
    {"flower07", dInsMusBase_c::ROOM_FLOWER},
    {"flower03", dInsMusBase_c::ROOM_FLOWER},
    {"F2_wood01", dInsMusBase_c::ROOM_F2},
    {"F2_wood01", dInsMusBase_c::ROOM_F2},
    {"F2_wood00", dInsMusBase_c::ROOM_F2},
    {"F2_wood00", dInsMusBase_c::ROOM_F2},
    {"F3_wood05", dInsMusBase_c::ROOM_F3},
    {"F1_panel01", dInsMusBase_c::ROOM_F1},
    {"arechi_ike00", dInsMusBase_c::ROOM_WASTELAND},
    {"arechi_ike00", dInsMusBase_c::ROOM_WASTELAND},
    {"F3_panel00", dInsMusBase_c::ROOM_F3},
    {"F2_wood02", dInsMusBase_c::ROOM_F2},
    {"ike00", dInsMusBase_c::ROOM_POND},
    {"ike00", dInsMusBase_c::ROOM_POND},
    {"flower08", dInsMusBase_c::ROOM_FLOWER},
    {"F2_panel02", dInsMusBase_c::ROOM_F2},
    {"F2_panel02", dInsMusBase_c::ROOM_F2},
    {"F2_kirikabu01", dInsMusBase_c::ROOM_F2},
    {"arechi00", dInsMusBase_c::ROOM_WASTELAND},
    {"F2_kirikabu01", dInsMusBase_c::ROOM_F2},
    {"F3_wood00", dInsMusBase_c::ROOM_F3},
    {"F3_wood00", dInsMusBase_c::ROOM_F3},
    {"ike_flower00", dInsMusBase_c::ROOM_POND},
    {"F2_kirikabu01", dInsMusBase_c::ROOM_F2},
    {"F2_kirikabu01", dInsMusBase_c::ROOM_F2},
    {"arechi00", dInsMusBase_c::ROOM_WASTELAND},
    {"ike_hotaru00", dInsMusBase_c::ROOM_POND},
    {"F3_wood04", dInsMusBase_c::ROOM_F3},
    {"F3_wood04", dInsMusBase_c::ROOM_F3},
    {"F3_wood07", dInsMusBase_c::ROOM_F3},
    {"F3_wood03", dInsMusBase_c::ROOM_F3},
    {"F3_wood02", dInsMusBase_c::ROOM_F3},
    {"F3_wood01", dInsMusBase_c::ROOM_F3},
    {"F3_wood07", dInsMusBase_c::ROOM_F3},
    {"nettai_yashi02", dInsMusBase_c::ROOM_TROPICAL},
    {"F2_kirikabu00", dInsMusBase_c::ROOM_F2},
    {"F3_wood06", dInsMusBase_c::ROOM_F3},
    {"nettai_yashi01", dInsMusBase_c::ROOM_TROPICAL},
    {"nettai_yashi00", dInsMusBase_c::ROOM_TROPICAL},
    {"F2_kirikabu00", dInsMusBase_c::ROOM_F2},
    {"nettai_yashi03", dInsMusBase_c::ROOM_TROPICAL},
    {"nettai_yashi02", dInsMusBase_c::ROOM_TROPICAL},
    {"arechi00", dInsMusBase_c::ROOM_WASTELAND},
    {"flower07", dInsMusBase_c::ROOM_FLOWER},
    {"nettai_rafflesia", dInsMusBase_c::ROOM_TROPICAL},
    {"nettai_rafflesia", dInsMusBase_c::ROOM_TROPICAL},
    {"F2_wood02", dInsMusBase_c::ROOM_F2},
    {"nettai_yashi00", dInsMusBase_c::ROOM_TROPICAL},
    {"nettai_yashi00", dInsMusBase_c::ROOM_TROPICAL},
};

} // namespace dInsectMuseum

#include "d_insect_mus.inc"
