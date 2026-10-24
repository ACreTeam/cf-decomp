// The structure actor (REL d_a_strNP). See include/game/game/d_a_str.hpp.
// REL .text 0xCC..0x57B4, .ctors 0x0..0x4, .rodata 0x0..0x258, .data
// 0x0..0x730, .bss 0x8..0x78.
#include <game/game/d_a_str.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgc.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_demo_sync.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_pad.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/game/d_theater_common.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_world.hpp>
#include <game/game/d_field_info.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <string.h>

// The player faces the door (isInDoorRange); wider for the city's angled doors.
#define DOOR_FACE_ANGLE 0x2100
#define DOOR_FACE_ANGLE_CITY 0x2800
// canInteract: the player faces the door.
#define DOOR_TALK_ANGLE 0x1000
// The town gate (gate type 0): its entrance is wider.
#define GATE_DOOR_WIDEN 7.0f
// Frames before an automatic door closes again.
#define AUTO_DOOR_CLOSE_WAIT 60
// Frames before the player walks out of the door at the scene start.
#define EXIT_DOOR_WAIT 9

// .text:0xCC
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

// .text:0xD8
namespace dAStr {
BASE_PROFILE(A_STR, daSTR_c)

const float cNpcDoorOffset = 16.0f;

// .text:0x108
void toFieldPos(mVec3_c *out, const mVec3_c *local,
                const mMtx_c *mtx) {
    mVec3_c pos;
    PSMTXMultVec(mtx->m, *local, pos);
    dWorld::toFieldPosition(out, &pos);
}

// .text:0x148
f32 getViewBack() {
    return mFI_UT_WORLDSIZE_X_F * dSTR::getHostIO()->_0C;
}

// .text:0x198
f32 getViewFront() {
    return mFI_UT_WORLDSIZE_X_F * dSTR::getHostIO()->_14;
}

// .text:0x1E8
f32 getViewHalfWidth() {
    return mFI_UT_WORLDSIZE_X_F * dSTR::getHostIO()->_16;
}

// .text:0x238
BOOL isFadeMode() {
    return FALSE;
}
} // namespace dAStr

static const char *l_doorKnobIn = "door_knob_in";
static const char *l_doorKnobOut = "door_knob_out";
static const char *l_doorAuto0 = "door_auto_L";
static const char *l_doorAuto1 = "door_auto_L";
static const char *l_doorAuto2 = "door_auto_L";
static const char *l_fadeMdlName = "obj_fade_model";

// .text:0x240
const char *daSTR_c::getFadeMdlName() {
    if (isAutoDoor()) {
        return l_fadeMdlName;
    }
    return NULL;
}

// .text:0x278
int daSTR_c::getDoorType() const {
    return dSTR::getBuildingInfo(mBuildingIdx)->mType;
}

namespace dAStr {
// .text:0x2A0
mVec3_c getNpcDoorPos(const daSTR_c *str, const mVec3_c *pos, mAng *angle,
                      float offset) {
    mVec3_c result = str->getPos();
    mAng angleBuf;
    if (angle == NULL) {
        angle = &angleBuf;
    }
    if (str->mEntArea != NULL) {
        mVec3_c min, max;
        *angle = (u32)str->mEntArea->getBox(&min, &max, str->getPos(), 0) >> 16;
        mVec3_c center = (min + max) * 0.5f;
        result = center;
        result.x += offset * angle->sin();
        result.z += offset * angle->cos();
        if (!str->isDoorCentered()) {
            result.x = pos->x;
            result.z = pos->z;
        }
    }
    return result;
}
} // namespace dAStr

// .text:0x3F8
BOOL daSTR_c::isNoDoor() {
    return getDoorType() == DOOR_NONE;
}

// .text:0x42C
BOOL daSTR_c::isAutoDoor() {
    return getDoorType() == DOOR_AUTO;
}

// .text:0x464
BOOL daSTR_c::isHingedDoor() {
    return getDoorType() == DOOR_HINGED;
}

// .text:0x49C
BOOL daSTR_c::checkDoorFront() {
    if (!isNoDoor()) {
        dAcPlayer_c *player =
            dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player != NULL) {
            const mVec3_c *pos = player->getPosP();
            s16 moveAngle;
            if (isAutoDoor()) {
                mAng angle(player->getAngle().y.mAngle);
                return isInDoorRange(pos, &angle.mAngle, 16.0f);
            }
            moveAngle = 0;
            if (dPlayerMgr::getMoveAngle(&moveAngle)) {
                mAng angle(moveAngle);
                return isInDoorRange(pos, &angle.mAngle, 16.0f);
            }
        }
    }
    return FALSE;
}

// .text:0x56C
BOOL daSTR_c::isInDoorRange(const mVec3_c *pos, const s16 *angle, f32 range) {
    if (mEntArea != NULL) {
        mVec3_c min, max;
        const u32 &dir = (u32)mEntArea->getBox(&min, &max, getPos(), 0) >> 16;
        if (mBuildingIdx == BUILDING_INDEX(BUILDING_GATE) &&
            dSaveBuilding_c::get()->getGateType() == 0) {
            min.x -= GATE_DOOR_WIDEN;
            max.x += GATE_DOOR_WIDEN;
        }
        mVec3_c center = (min + max) * 0.5f;
        mVec3_c toMin = min - center;
        mVec3_c toMax = max - center;
        f32 distMin =
            EGG::Math<f32>::sqrt(toMin.x * toMin.x + toMin.z * toMin.z);
        f32 distMax =
            EGG::Math<f32>::sqrt(toMax.x * toMax.x + toMax.z * toMax.z);
        toMin.normalizeRS();
        toMax.normalizeRS();
        f32 margin = dSTR::getHostIO()->_10;
        if (!isDoorCentered()) {
            margin = 10.4f;
        }
        min = center + toMin * (distMin - margin);
        max = center + toMax * (distMax - margin);
        s16 doorAngle = (s16)dir + DEG_TO_ANGLE(180);
        int diff = sLib::distanceAngle(doorAngle, *angle);
        BOOL wide = FALSE;
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) &&
            doorAngle != DEG_TO_ANGLE(180)) {
            wide = TRUE;
        }
        if (diff <= (wide ? DOOR_FACE_ANGLE_CITY : DOOR_FACE_ANGLE)) {
            dBGC::vecXZ_c start(min.x, min.z);
            dBGC::vecXZ_c end(max.x, max.z);
            dBGC::lineXZ_c line;
            line.set(start, end);
            dBGC::vecXZ_c point(pos->x, pos->z);
            if (line.checkInside(point)) {
                f32 dist = line.calcDist(point);
                static const f32 cRangeMargin = 1.6f;
                if (dist >= 0.0f && dist <= cRangeMargin + range) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// .text:0x8B8
nw4r::g3d::ScnMdl *daSTR_c::getScnMdl() {
    return nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
        nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(mMdl.getScn()));
}

// The materials of a light and their lit colours.
struct lightMat_s {
    const char *mName;
    mColor mColor;
    u8 mPolyColor; // uses the building's dBuildingInfo_c::mPolyColor
};

static mColor l_twLightColor(0xFF, 0xFF, 0xFF, 0xFF);

static lightMat_s l_windowMats[17] = {
    {"m_letter", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_light", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_board", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_special", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_bus_ind", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_museum", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_spotlight", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_window", mColor(0xFF, 0xFF, 0xA0, 0xFF), FALSE},
    {"m_tw_light", l_twLightColor, TRUE},
    {"m_tw_light_00", l_twLightColor, TRUE},
    {"m_tw_light_01", l_twLightColor, TRUE},
    {"m_tw_light_02", l_twLightColor, TRUE},
    {"m_tw_light_03", l_twLightColor, TRUE},
    {"m_tw_light_04", l_twLightColor, TRUE},
    {"m_tw_light_05", l_twLightColor, TRUE},
    {"m_tw_light_06", l_twLightColor, TRUE},
    {"m_tw_light_07", l_twLightColor, TRUE},
};

// .text:0x984
daSTR_c::windowLight_c::windowLight_c() {
    for (s8 *id = mMatID; id < mMatID + ARRAY_SIZE(mMatID); id++) {
        *id = -1;
    }
    mScnMdl = NULL;
    mBuildingIdx = 0;
}

// .text:0xA40
BOOL daSTR_c::windowLight_c::updateColor() {
    static mColor l_offColor(0, 0, 0, 0);
    BOOL result = FALSE;
    if (mScnMdl != NULL) {
        mColor polyColor = dSTR::getBuildingInfo(mBuildingIdx)->getPolyColor();
        lightMat_s *mat = l_windowMats;
        for (u32 i = 0; i < ARRAY_SIZE(l_windowMats); i++, mat++) {
            if (mMatID[i] != -1) {
                mColor matColor = mat->mColor;
                mColor color = mat->mPolyColor ? polyColor : matColor;
                nw4r::g3d::ScnMdl::CopiedMatAccess access(mScnMdl, mMatID[i]);
                nw4r::g3d::ResMatTevColor tevColor =
                    access.GetResMatTevColor(false);
                f32 value = mValue;
                u8 r = (int)dSTR::calcLerp(value, l_offColor.r, color.r);
                u8 g = (int)dSTR::calcLerp(value, l_offColor.g, color.g);
                u8 b = (int)dSTR::calcLerp(value, l_offColor.b, color.b);
                u8 a = (int)dSTR::calcLerp(value, l_offColor.a, color.a);
                nw4r::ut::Color kColor;
                kColor.Set(r, g, b, a);
                tevColor.GXSetTevColor(GX_TEVREG0, kColor);
                tevColor.DCStore(false);
                result = TRUE;
            }
        }
    }
    return result;
}

// .text:0xCD8
void daSTR_c::windowLight_c::set(nw4r::g3d::ResMdl mdl,
                                 nw4r::g3d::ScnMdl *scnMdl, BOOL on,
                                 int buildingIdx, u8 sub) {
    mSub = sub;
    mBuildingIdx = buildingIdx;
    if (scnMdl != NULL) {
        BOOL found = FALSE;
        for (u32 i = 0; i < ARRAY_SIZE(l_windowMats); i++) {
            mMatID[i] = m3d::getMatID(mdl, l_windowMats[i].mName);
            if (mMatID[i] != -1) {
                found = TRUE;
            }
        }
        if (found) {
            mScnMdl = scnMdl;
            dSTR::light_c::set(on);
            updateColor();
        }
    }
}

// .text:0xD94
void daSTR_c::windowLight_c::execute(BOOL on, BOOL flicker) {
    if (mScnMdl != NULL) {
        dSTR::light_c::set(on, TRUE, flicker, 0.1f);
        if (update()) {
            updateColor();
        }
    }
}

static lightMat_s l_neonMats[1] = {
    {"m_tw_neon", mColor(0xFF, 0x00, 0x00, 0xFF), FALSE},
};

// .text:0xDF4
daSTR_c::neonLight_c::neonLight_c() {
    for (s8 *id = mMatID; id < mMatID + ARRAY_SIZE(mMatID); id++) {
        *id = -1;
    }
    mScnMdl = NULL;
    mBuildingIdx = 0;
}

// .text:0xE5C
BOOL daSTR_c::neonLight_c::updateColor() {
    static mColor l_offColor(0, 0, 0, 0);
    BOOL result = FALSE;
    if (mScnMdl != NULL) {
        lightMat_s *mat = l_neonMats;
        for (u32 i = 0; i < ARRAY_SIZE(l_neonMats); i++, mat++) {
            if (mMatID[i] != -1) {
                mColor color = mat->mColor;
                nw4r::g3d::ScnMdl::CopiedMatAccess access(mScnMdl, mMatID[i]);
                nw4r::g3d::ResMatTevColor tevColor =
                    access.GetResMatTevColor(false);
                f32 value = mValue;
                u8 r = (int)dSTR::calcLerp(value, l_offColor.r, color.r);
                u8 g = (int)dSTR::calcLerp(value, l_offColor.g, color.g);
                u8 b = (int)dSTR::calcLerp(value, l_offColor.b, color.b);
                u8 a = (int)dSTR::calcLerp(value, l_offColor.a, color.a);
                nw4r::ut::Color kColor;
                kColor.Set(r, g, b, a);
                tevColor.GXSetTevColor(GX_TEVREG0, kColor);
                tevColor.DCStore(false);
                result = TRUE;
            }
        }
    }
    return result;
}

// .text:0x1098
void daSTR_c::neonLight_c::set(nw4r::g3d::ResMdl mdl, nw4r::g3d::ScnMdl *scnMdl,
                               BOOL on, int buildingIdx) {
    mBuildingIdx = buildingIdx;
    if (scnMdl != NULL) {
        BOOL found = FALSE;
        for (u32 i = 0; i < ARRAY_SIZE(l_neonMats); i++) {
            mMatID[i] = m3d::getMatID(mdl, l_neonMats[i].mName);
            if (mMatID[i] != -1) {
                found = TRUE;
            }
        }
        if (found) {
            mScnMdl = scnMdl;
            dSTR::light_c::set(on);
            updateColor();
        }
    }
}

// .text:0x1138
void daSTR_c::neonLight_c::execute(BOOL on, BOOL flicker) {
    if (mScnMdl != NULL) {
        dSTR::light_c::set(on, TRUE, flicker, 0.1f);
        if (update()) {
            updateColor();
        }
    }
}

static lightMat_s l_etcMats[5] = {
    {"m_tw_etc", mColor(0xFF, 0xFF, 0x80, 0xFF), FALSE},
    {"m_tw_etc_00", mColor(0xFF, 0xFF, 0x80, 0xFF), FALSE},
    {"m_tw_etc_01", mColor(0xFF, 0xFF, 0x80, 0xFF), FALSE},
    {"m_tw_etc_02", mColor(0xFF, 0xFF, 0x80, 0xFF), FALSE},
    {"m_tw_etc_03", mColor(0xFF, 0xFF, 0x80, 0xFF), FALSE},
};

// .text:0x1198
daSTR_c::etcLight_c::etcLight_c() {
    for (s8 *id = mMatID; id < mMatID + ARRAY_SIZE(mMatID); id++) {
        *id = -1;
    }
    mScnMdl = NULL;
    mBuildingIdx = 0;
}

// .text:0x1200
BOOL daSTR_c::etcLight_c::updateColor() {
    static mColor l_offColor(0, 0, 0, 0);
    BOOL result = FALSE;
    if (mScnMdl != NULL) {
        lightMat_s *mat = l_etcMats;
        for (u32 i = 0; i < ARRAY_SIZE(l_etcMats); i++, mat++) {
            if (mMatID[i] != -1) {
                mColor color = mat->mColor;
                nw4r::g3d::ScnMdl::CopiedMatAccess access(mScnMdl, mMatID[i]);
                nw4r::g3d::ResMatTevColor tevColor =
                    access.GetResMatTevColor(false);
                f32 value = mValue;
                u8 r = (int)dSTR::calcLerp(value, l_offColor.r, color.r);
                u8 g = (int)dSTR::calcLerp(value, l_offColor.g, color.g);
                u8 b = (int)dSTR::calcLerp(value, l_offColor.b, color.b);
                u8 a = (int)dSTR::calcLerp(value, l_offColor.a, color.a);
                nw4r::ut::Color kColor;
                kColor.Set(r, g, b, a);
                tevColor.GXSetTevColor(GX_TEVREG0, kColor);
                tevColor.DCStore(false);
                result = TRUE;
            }
        }
    }
    return result;
}

// .text:0x1448
void daSTR_c::etcLight_c::set(nw4r::g3d::ResMdl mdl, nw4r::g3d::ScnMdl *scnMdl,
                              BOOL on, int buildingIdx) {
    mBuildingIdx = buildingIdx;
    if (scnMdl != NULL) {
        BOOL found = FALSE;
        for (u32 i = 0; i < ARRAY_SIZE(l_etcMats); i++) {
            mMatID[i] = m3d::getMatID(mdl, l_etcMats[i].mName);
            if (mMatID[i] != -1) {
                found = TRUE;
            }
        }
        if (found) {
            mScnMdl = scnMdl;
            dSTR::light_c::set(on);
            updateColor();
        }
    }
}

// .text:0x1500
void daSTR_c::etcLight_c::execute(BOOL on, BOOL flicker) {
    if (mScnMdl != NULL) {
        dSTR::light_c::set(on, TRUE, flicker, 0.1f);
        if (update()) {
            updateColor();
        }
    }
}

// .text:0x1560
BOOL daSTR_c::isNight() {
    dTime_c now = *dTime_c::getCurrent();
    if (now.hour >= TIME_DAY_START_HOUR && now.hour < TIME_NIGHT_START_HOUR) {
        return FALSE;
    }
    return TRUE;
}

// .text:0x15EC
BOOL daSTR_c::isWindowLit() {
    BOOL lit = FALSE;
    if (isNight() && getCheckMsg() == DOOR_MSG_NONE) {
        lit = TRUE;
    }
    return lit;
}

// .text:0x165C: the window lights flicker on (dBuildingInfo_c::mLightFlicker).
BOOL daSTR_c::isLightFlicker() const {
    return dSTR::getBuildingInfo(mBuildingIdx)->mLightFlicker != 0;
}

// .text:0x1690
void daSTR_c::calcMtx(f32 offsetY, f32 offsetZ, mMtx_c *mtx) {
    mMtx_c buf;
    if (mtx == NULL) {
        mtx = &buf;
    }
    mVec3_c pos = mPos;
    pos.x += offsetZ;
    pos.y += offsetY;
    mVec3_c curved;
    mAng tilt = dWorld::curvePositionSimple(&curved, &pos);
    PSMTXTrans(*mtx, curved.x, curved.y, curved.z);
    mtx->XrotM(tilt);
    mtx->YrotM(mAngle.y);
}

// .text:0x1740
void daSTR_c::drawMdl() {
    static const f32 cFortuneScaleY = 1.2f;
    static const f32 cShadowFrameNum = 120.0f;
    f32 offsetY;
    if (dStrm_c::isUseHostIO()) {
        offsetY = dStrm_c::getOffsetY();
    } else {
        offsetY = 0.0f;
    }
    mMtx_c mtx;
    calcMtx(offsetY + getModelOffsetY(), 0.0f, &mtx);
    mMdl.setLocalMtx(&mtx);
    mMdl.calc(false);
    f32 scale;
    if (dStrm_c::isUseHostIO()) {
        scale = dStrm_c::getScale();
    } else {
        scale = getMdlScale();
    }
    mMdl.setScale(scale, scale, scale);
    if (dStrm_c::isEntryMdl()) {
        mMdl.entry();
    }
    if (hasFadeMdl()) {
        if (mBuildingIdx == BUILDING_INDEX(BUILDING_CITY_FORTUNE)) {
            mFadeMdl.setScale(scale, cFortuneScaleY * scale, scale);
        } else {
            mFadeMdl.setScale(scale, scale, scale);
        }
        mFadeMdl.setLocalMtx(&mtx);
        mFadeMdl.calc(false);
        mFadeMdl.entry();
    }
    f32 shadowSize = getRoundShadowSize();
    if (0.0f != shadowSize) {
        mAng angle = 0;
        dShadow::requestRound(&mPos, 0, &angle, shadowSize, 1.0f, 1.0f, 0.0f,
                              1.0f);
    }
    if (mShadowMatID != -1) {
        f32 frame = cShadowFrameNum * dShadow::getSunTime();
        if (mBuildingIdx == BUILDING_INDEX(BUILDING_CITY_BUS)) {
            mShadowAnm.setFrame(cShadowFrameNum * (1.0f - frame));
        } else {
            mShadowAnm.setFrame(frame);
        }
        u32 alpha = (u8)dShadow::getAlpha(&mPos);
        if (alpha >= 3) {
            nw4r::g3d::ScnMdl *scnMdl =
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                    nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                        mShadowMdl.getScn()));
            nw4r::g3d::ScnMdl::CopiedMatAccess access(scnMdl, mShadowMatID);
            nw4r::g3d::ResMatTevColor tevColor =
                access.GetResMatTevColor(false);
            nw4r::ut::Color kColor;
            kColor.Set(0x20, 0x20, 0x30, alpha);
            tevColor.GXSetTevColor(GX_TEVREG0, kColor);
            tevColor.DCStore(false);
            if (0.0f != dSTR::getHostIO()->_38) {
                mMtx_c shadowMtx;
                calcMtx(dSTR::getHostIO()->_38, 0.0f, &shadowMtx);
                mShadowMdl.setLocalMtx(&shadowMtx);
            } else {
                mShadowMdl.setLocalMtx(&mtx);
            }
            mMtx_c *ownMtx = getShadowMtx();
            if (ownMtx != NULL) {
                mShadowMdl.setLocalMtx(ownMtx);
            }
            nw4r::math::VEC3 *ownScale = getShadowScale();
            if (ownScale != NULL) {
                mShadowMdl.setScale(ownScale->x, ownScale->y, ownScale->z);
            } else {
                mShadowMdl.setScale(dSTR::getHostIO()->_34,
                                    dSTR::getHostIO()->_34,
                                    dSTR::getHostIO()->_34);
            }
            mShadowMdl.calc(false);
            mShadowMdl.calcVtx(false);
            if (dStrm_c::isEntryMdl()) {
                mShadowMdl.entry();
            }
        }
    }
    drawShadow(&mtx);
}

// The texture shadows of a structure (shadow.bin in its brres).
struct shadowTex_s {
    f32 mX;
    f32 mZ;
    f32 mWidth;
    f32 mLength;
    f32 mParam2;
    f32 mParam3;
    u8 mKind; // 'G'

    f32 getX() const {
        return mX;
    }
    f32 getZ() const {
        return mZ;
    }
    f32 getWidth() const {
        return mWidth;
    }
    f32 getLength() const {
        return mLength;
    }
    f32 getParam2() const {
        return mParam2;
    }
    f32 getParam3() const {
        return mParam3;
    }
};

struct shadowBin_s {
    u32 mNum;
    u8 _04[0x1C];
    shadowTex_s mTex[1];
};

static char l_shadowBin[] = "shadow.bin";
static char l_shadowMat[] = "m_shadow0";
static char l_hourNode[] = "kh_j";
static char l_minuteNode[] = "km_j";
static char l_curveNode[] = "joint1";

// .text:0x1B78
void daSTR_c::drawShadow(mMtx_c *mtx) {
    shadowBin_s *bin = (shadowBin_s *)mResFile.GetExternal(l_shadowBin);
    if (bin != NULL) {
        u32 num = bin->mNum;
        shadowTex_s *tex = bin->mTex;
        for (u32 i = 0; i < num; tex++, i++) {
            mVec3_c pos;
            mVec3_c local(tex->getX(), 0.0f, tex->getZ());
            dAStr::toFieldPos(&pos, &local, mtx);
            pos.y += 1.0f;
            int texture = 1;
            switch (tex->mKind) {
            case 'G':
                texture = 1;
                break;
            }
            dShadow::requestTexture(&pos, texture, tex->getWidth(),
                                    tex->getLength(), tex->getParam2(),
                                    tex->getParam3());
        }
    }
}

// .text:0x1C80
ulong daSTR_c::getMdlBufferOption() {
    return nw4r::g3d::ScnMdl::BUFFER_RESTEXOBJ |
           nw4r::g3d::ScnMdl::BUFFER_RESTLUTOBJ |
           nw4r::g3d::ScnMdl::BUFFER_RESCHAN |
           nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR;
}

// .text:0x1C88
void daSTR_c::setup() {
    if (!(mFlags & FLAG_CREATED)) {
        mMsgCode = -1;
        mBuildingIdx = (u16)mParam;
        mBuildingId = dSaveBuildingList_c::getId(mBuildingIdx);
        mParamHi = mParam >> 16;
        if (mBuildingIdx >= BUILDING_INDEX(BUILDING_CITY_LAMP_A) &&
            mBuildingIdx <= BUILDING_INDEX(BUILDING_CITY_LAMP_L)) {
            mPos.y = 56.0f;
        }
        dSTR::getAcList()->add(this);
        strncpy(mResName, getResName(), 30);
        f32 x = mPos.x;
        f32 z = mPos.z;
        mUnitX = worldToUnit(x);
        mUnitZ = worldToUnit(z);
        if (isCopyCol()) {
            dSTR::copyCol(mUnitX, mUnitZ, mResName);
        }
        mEntArea = dSTR::getColBank()->getEntArea(mResName);
        mColArea = dSTR::getColBank()->getColArea(mResName);
        mName = dSTR::getBuildingInfo(mBuildingIdx)->mName;
        if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_DEMO)) {
            mFlags |= FLAG_STATES;
        }
        mFlags |= FLAG_CREATED;
    }
}

// .text:0x1DFC
void daSTR_c::createMdl() {
    static const int cFadePrioXlu = 9;
    static const int cFirstAnm = 0;
    static const ulong cShadowBufferOption =
        nw4r::g3d::ScnMdl::BUFFER_RESCHAN |
        nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR |
        nw4r::g3d::ScnMdl::BUFFER_RESVTXPOS |
        nw4r::g3d::ScnMdl::BUFFER_RESVTXNRM |
        nw4r::g3d::ScnMdl::BUFFER_RESVTXCLR;
    mAllocator_c *allocator = dStrm_c::getAllocator();
    mResFile = nw4r::g3d::ResFile(dSTR::getBrresBank()->getBrres(mResName));
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
    nw4r::g3d::ResFile common = dStrm_c::sCommonBrres;
    nw4r::g3d::ResMdl fadeMdl(NULL);
    const char *fadeName = getFadeMdlName();
    if (fadeName != NULL) {
        fadeMdl = common.GetResMdl(fadeName);
    }
    if (mResFile.GetResMdlNumEntries() == 0) {
        return;
    }
    BOOL curve = fixDoorVerts();
    size_t mdlSize = 0;
    mMdl.create(mdl, allocator, getMdlBufferOption(), 1, &mdlSize);
    if (hasFadeMdl()) {
        mFadeMdl.create(fadeMdl, allocator,
                        nw4r::g3d::ScnMdl::BUFFER_RESCHAN |
                            nw4r::g3d::ScnMdl::BUFFER_RESTEVCOLOR,
                        1, NULL);
        mFadeMdl.setPriorityDraw(0x7F, cFadePrioXlu);
    }
    u32 anmNum = mResFile.GetResAnmChrNumEntries();
    if (anmNum != 0) {
        for (u32 i = 0; i < anmNum; i++) {
            if (i < ARRAY_SIZE(mDoorAnm)) {
                mDoorAnm[i] = mResFile.GetResAnmChr(i);
            }
        }
        if (anmNum == 1) {
            mDoorAnm[1] = mDoorAnm[0];
        }
    } else if (mEntArea != NULL) {
        if (isAutoDoor()) {
            mDoorAnm[0] = common.GetResAnmChr(l_doorKnobIn);
            mDoorAnm[1] = common.GetResAnmChr(l_doorKnobOut);
        } else if (isHingedDoor()) {
            switch (mEntArea->getDirType()) {
            case 1:
                mDoorAnm[0] = common.GetResAnmChr(l_doorAuto0);
                mDoorAnm[1] = common.GetResAnmChr(l_doorAuto0);
                break;
            case 2:
                mDoorAnm[0] = common.GetResAnmChr(l_doorAuto1);
                mDoorAnm[1] = common.GetResAnmChr(l_doorAuto1);
                break;
            case 0:
                mDoorAnm[0] = common.GetResAnmChr(l_doorAuto2);
                mDoorAnm[1] = common.GetResAnmChr(l_doorAuto2);
                break;
            }
        }
    }
    if (!isNoDoor() && getDoorAnmNum() != 0) {
        nw4r::g3d::ResAnmChr anm(NULL);
        getDoorAnm(&anm, cFirstAnm);
        mDoorAnmChr.create(mdl, anm, allocator, NULL);
        mDoorAnmChr.mPlayMode = m3d::FORWARD_ONCE;
        mMdl.setAnm(mDoorAnmChr);
        setDoorState(DOOR_STATE_CLOSED);
    }
    if (mResFile.GetResMdlNumEntries() >= 2) {
        nw4r::g3d::ResMdl shadowMdl = mResFile.GetResMdl(1);
        if (shadowMdl.IsValid()) {
            mShadowMdl.create(shadowMdl, allocator, cShadowBufferOption, 1,
                              NULL);
            mShadowMdl.setPriorityDraw(6, 8);
            if (getShadowAnmNum() != 0) {
                nw4r::g3d::ResAnmShp anm(NULL);
                getShadowAnm(&anm, 0);
                mShadowAnm.create(shadowMdl, anm, allocator, NULL);
                mShadowAnm.mPlayMode = m3d::FORWARD_LOOP;
                mShadowMdl.setAnm(mShadowAnm);
            }
            mShadowMatID = m3d::getMatID(shadowMdl, l_shadowMat);
        }
    }
    mWindowLight.set(mdl, getScnMdl(), isWindowLit(), mBuildingIdx, FALSE);
    mNeonLight.set(mdl, getScnMdl(), isNeonOn(), mBuildingIdx);
    mEtcLight.set(mdl, getScnMdl(), isEtcLightOn(), mBuildingIdx);
    if (hasFadeMdl()) {
        nw4r::g3d::ScnMdl *scnMdl =
            nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdl>(
                nw4r::g3d::G3dObj::DynamicCast<nw4r::g3d::ScnMdlSimple>(
                    mFadeMdl.getScn()));
        mFadeWindowLight.set(fadeMdl, scnMdl, isWindowLit(), mBuildingIdx,
                             TRUE);
    }
    mNodeCallBack.mOwner = this;
    mNodeCallBack.mHourNode = m3d::getNodeID(mdl, l_hourNode);
    mNodeCallBack.mMinuteNode = m3d::getNodeID(mdl, l_minuteNode);
    mNodeCallBack.mCurveNode = m3d::getNodeID(mdl, l_curveNode);
    mNodeCallBack.mCurve = curve;
    mMdl.setCallback(&mNodeCallBack);
    setDemoID((u16)getDemoCode());
    if (!isNoDoor()) {
        setDemoFlag8();
    }
    if (isAutoDoor() && mEntArea != NULL) {
        mVec3_c min, max;
        mEntArea->getBox(&min, &max, mPos, 0);
        mVec3_c top0 = min;
        mVec3_c top1 = max;
        mVec3_c center = min + max;
        center *= 0.5f;
        top0.y += 48.0f;
        top1.y += 48.0f;
        mDpdQuad.set(top0, min, max, top1, dObjc::HIT_STR_DOOR, this, &center,
                     0);
    }
    setState(STATE_WAIT);
}

// .text:0x2548
BOOL daSTR_c::isCulled() {
    if (isCheckCamera()) {
        dCamera_c *camera = dCamera_c::sInstance;
        if (camera != NULL && mColArea != NULL) {
            mVec3_c pos = mPos + mColArea->getCenter();
            if (dStrm_c::isDrawDebug()) {
                mVec3_c top = pos;
                top.y += 160.0f;
                dSceneCheck::drawLine(top, pos, mColor(100, 100, 0xFF, 0xFF), 1,
                                      0, 10);
            }
            if (camera->isPastTarget(
                    &pos, std::fabs(mColArea->mMax.z - mColArea->mMin.z))) {
                return TRUE;
            }
        }
    }
    if (isCheckViewArea()) {
        dSTR::viewArea_c area(dAStr::getViewFront(), dAStr::getViewBack(),
                              dAStr::getViewHalfWidth());
        return !area.isIn(mPos);
    }
    return FALSE;
}

// .text:0x26E0
daSTR_c::daSTR_c() {
    mShadowMatID = -1;
    mResFile = nw4r::g3d::ResFile();
    mDoorAnm[0] = nw4r::g3d::ResAnmChr();
    mDoorAnm[1] = nw4r::g3d::ResAnmChr();
}

// .text:0x293C
int daSTR_c::create() {
    setup();
    if (dSTR::getBrresBank()->getData() != NULL) {
        beforeCreateMdl();
        createMdl();
        afterCreateMdl();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x29B4
int daSTR_c::preExecute() {
    if (!dDemoActor_c::preExecute()) {
        return NOT_READY;
    }
    if (isAutoDoor()) {
        mDpdQuad.regist();
    }
    updateSoundPos();
    executeState();
    executeDoorState();
    mWindowLight.execute(isWindowLit(), isLightFlicker());
    mNeonLight.execute(isNeonOn(), isLightFlicker());
    mEtcLight.execute(isEtcLightOn(), isLightFlicker());
    mFadeWindowLight.execute(isWindowLit(), isLightFlicker());
    executeExtra();
    return SUCCEEDED;
}

// .text:0x2B00
void daSTR_c::postExecute(MAIN_STATE_e state) {
    dActor_c::postExecute(state);
}

// .text:0x2B04
int daSTR_c::draw() {
    if (!dActor_c::preDraw()) {
        return NOT_READY;
    }
    if (isDraw() && !isCulled()) {
        drawMdl();
    }
    return SUCCEEDED;
}

// .text:0x2B84
int daSTR_c::preDelete() {
    dSTR::getAcList()->remove(this);
    if (mMdl.getScn() != NULL) {
        mMdl.remove();
    }
    if (mDoorAnmChr.IsBound()) {
        mDoorAnmChr.remove();
    }
    if (mShadowMdl.getScn() != NULL) {
        mShadowMdl.remove();
    }
    if (mShadowAnm.IsBound()) {
        mShadowAnm.remove();
    }
    if (hasFadeMdl()) {
        mFadeMdl.remove();
    }
    if ((mFlags >> 4) & 1) {
        dDemoSync::release();
    }
    dSTR::flattenCol(mUnitX, mUnitZ, mResName);
    return SUCCEEDED;
}

// .text:0x2C94
int daSTR_c::getDoorAnmNum() {
    if (mDoorAnm[1].IsValid()) {
        return 2;
    }
    return mDoorAnm[0].IsValid();
}

// .text:0x2CBC
int daSTR_c::getShadowAnmNum() {
    if (mResFile.IsValid()) {
        return mResFile.GetResAnmShpNumEntries();
    }
    return 0;
}

// .text:0x2CD8
BOOL daSTR_c::getDoorAnm(nw4r::g3d::ResAnmChr *anm, u32 idx) {
    if (idx < getDoorAnmNum()) {
        *anm = mDoorAnm[idx];
        return TRUE;
    }
    *anm = nw4r::g3d::ResAnmChr(NULL);
    return FALSE;
}

// .text:0x2D54
BOOL daSTR_c::getShadowAnm(nw4r::g3d::ResAnmShp *anm, u32 idx) {
    if (idx < getShadowAnmNum()) {
        *anm = mResFile.GetResAnmShp(idx);
        return TRUE;
    }
    *anm = nw4r::g3d::ResAnmShp(NULL);
    return FALSE;
}

static BOOL (daSTR_c::*const l_doorInit[daSTR_c::DOOR_STATE_NUM])() = {
    &daSTR_c::initDoorClosed,    &daSTR_c::initDoorOpenIn,
    &daSTR_c::initDoorOpenOut,   &daSTR_c::initAutoDoorOpen,
    &daSTR_c::initAutoDoorClose, &daSTR_c::initDoorWaitIn,
    &daSTR_c::initDoorWaitOut,
};

static void (daSTR_c::*const l_doorExecute[daSTR_c::DOOR_STATE_NUM])() = {
    &daSTR_c::executeDoorClosed,    &daSTR_c::executeDoorOpenIn,
    &daSTR_c::executeDoorOpenOut,   &daSTR_c::executeAutoDoorOpen,
    &daSTR_c::executeAutoDoorClose, &daSTR_c::executeDoorWaitIn,
    &daSTR_c::executeDoorWaitOut,
};

// .text:0x2DD0
BOOL daSTR_c::setDoorState(int state) {
    if ((u32)state < DOOR_STATE_NUM && (this->*l_doorInit[state])()) {
        mDoorState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x2E3C
void daSTR_c::executeDoorState() {
    if (mDoorState < DOOR_STATE_NUM) {
        (this->*l_doorExecute[mDoorState])();
    }
}

// .text:0x2E7C
BOOL daSTR_c::initDoorClosed() {
    switch (getDoorType()) {
    case DOOR_AUTO: {
        nw4r::g3d::ResAnmChr anm(NULL);
        getDoorAnm(&anm, 0);
        mDoorAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
        mMdl.setAnm(mDoorAnmChr);
        break;
    }
    case DOOR_HINGED: {
        nw4r::g3d::ResAnmChr anm(NULL);
        if (getDoorAnm(&anm, 0)) {
            mDoorAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
            mMdl.setAnm(mDoorAnmChr);
        }
        break;
    }
    }
    return TRUE;
}

// Asks for the door's message, then starts talking.
inline void daSTR_c::requestDoorTalk() {
    int msg = getCheckMsg();
    if (msg == DOOR_MSG_NONE) {
        mFlags &= ~FLAG_MSG;
    } else {
        mMsgCode = msg;
        mFlags |= FLAG_MSG;
    }
    clearDemoState();
    dDemo::requestSpeak(this, NULL);
}

// .text:0x2F8C
void daSTR_c::executeDoorClosed() {
    if (mState == STATE_WAIT && checkDoorFront()) {
        switch (getDoorType()) {
        case DOOR_HINGED:
        case DOOR_WALK:
            requestDoorTalk();
            break;
        case DOOR_AUTO: {
            BOOL enter = FALSE;
            if (mPad::g_currentCore->getDpdValidFlag() > 0) {
                const dObjc::dpdHit_c *hit = dObjc::getDpdHit();
                if (hit != NULL) {
                    if (hit->mType == dObjc::HIT_STR_DOOR &&
                        hit->mBase.get() == this) {
                        if (dPlayerMgr::isATriggeredFacing(&hit->mPos, 0x2000,
                                                           32.0f)) {
                            enter = TRUE;
                        }
                    } else if (hit->mType == dObjc::TYPE_PLAYER &&
                               dPad::isATriggered()) {
                        enter = TRUE;
                    }
                }
                if (dPad::isConfirmTriggered() && !dPad::isATriggered()) {
                    enter = TRUE;
                }
            } else if (dPad::isConfirmTriggered()) {
                enter = TRUE;
            }
            if (enter) {
                requestDoorTalk();
            }
            break;
        }
        }
    }
}

// .text:0x3174
BOOL daSTR_c::initDoorOpenIn() {
    nw4r::g3d::ResAnmChr anm(NULL);
    getDoorAnm(&anm, 0);
    if (anm.IsValid()) {
        getDoorAnm(&anm, 0);
        mDoorAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
        mMdl.setAnm(mDoorAnmChr);
        mDoorTimer = 1;
        return TRUE;
    }
    return FALSE;
}

// .text:0x322C
void daSTR_c::executeDoorOpenIn() {
    if (mDoorTimer == 0) {
        mDoorAnmChr.play();
        if (mDoorAnmChr.isStop()) {
            setDoorState(DOOR_STATE_CLOSED);
        } else if (mDoorAnmChr.checkFrame(8.0f)) {
            mSnd.startSound(getDoorOpenSe());
            mSnd.startSound(getDoorCreakOpenSe());
        } else if (mDoorAnmChr.checkFrame(60.0f)) {
            mSnd.startSound(getDoorCreakCloseSe());
        } else if (mDoorAnmChr.checkFrame(93.0f)) {
            mSnd.startSound(getDoorCloseSe());
        }
    }
    if (mDoorTimer != 0 && --mDoorTimer == 0) {
        int se = getDoorEndSe();
        if (se != DOOR_MELODY_NONE) {
            playSeAt(&mPos, se);
        }
    }
}

// .text:0x33B4
BOOL daSTR_c::initDoorOpenOut() {
    nw4r::g3d::ResAnmChr anm(NULL);
    getDoorAnm(&anm, 1);
    if (anm.IsValid()) {
        mDoorAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
        mMdl.setAnm(mDoorAnmChr);
        mDoorTimer = 1;
        return TRUE;
    }
    return FALSE;
}

// .text:0x3450
void daSTR_c::executeDoorOpenOut() {
    if (mDoorTimer == 0) {
        mDoorAnmChr.play();
        if (mDoorAnmChr.isStop()) {
            setDoorState(DOOR_STATE_CLOSED);
        } else if (mDoorAnmChr.checkFrame(8.0f)) {
            mSnd.startSound(getDoorOpenSe());
            mSnd.startSound(getDoorCreakOpenSe());
        } else if (mDoorAnmChr.checkFrame(dSTR::getHostIO()->_1C)) {
            mSnd.startSound(getDoorCreakCloseSe());
        } else if (mDoorAnmChr.checkFrame(87.0f)) {
            mSnd.startSound(getDoorCloseSe());
        }
    }
    if (mDoorTimer != 0 && --mDoorTimer == 0) {
        int se = getDoorEndSe();
        if (se != DOOR_MELODY_NONE) {
            playSeAt(&mPos, se);
        }
        mDoorTimer = 0;
    }
}

// .text:0x35FC
BOOL daSTR_c::initAutoDoorOpen() {
    nw4r::g3d::ResAnmChr anm(NULL);
    if (getDoorAnm(&anm, 0)) {
        mDoorAnmChr.setAnm(mMdl, anm, m3d::FORWARD_ONCE);
        mMdl.setAnm(mDoorAnmChr);
        if (getAutoDoorOpenSe() != -1) {
            mSnd.startSound(getAutoDoorOpenSe());
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0x36C8
void daSTR_c::executeAutoDoorOpen() {
    mDoorAnmChr.play();
    if (mDoorAnmChr.isStop()) {
        setDoorState(DOOR_STATE_AUTO_CLOSE);
    }
}

// .text:0x372C
BOOL daSTR_c::initAutoDoorClose() {
    mAutoDoorTimer = AUTO_DOOR_CLOSE_WAIT;
    nw4r::g3d::ResAnmChr anm(NULL);
    if (getDoorAnm(&anm, 0)) {
        mDoorAnmChr.setAnm(mMdl, anm, m3d::REVERSE_ONCE);
        mMdl.setAnm(mDoorAnmChr);
        return TRUE;
    }
    return FALSE;
}

// .text:0x37C4
void daSTR_c::executeAutoDoorClose() {
    if (mAutoDoorTimer != 0 && --mAutoDoorTimer == 0 &&
        !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) &&
        getAutoDoorCloseSe() != -1) {
        mSnd.startSound(getAutoDoorCloseSe());
    }
    if (mAutoDoorTimer == 0) {
        mDoorAnmChr.play();
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            f32 end = mDoorAnmChr.mFrameMax;
            if (mDoorAnmChr.checkFrame(end - dSTR::getHostIO()->_40) &&
                getAutoDoorCloseSe() != -1) {
                mSnd.startSound(getAutoDoorCloseSe());
            }
        }
        if (mDoorAnmChr.isStop()) {
            setDoorState(DOOR_STATE_CLOSED);
        }
    }
}

// .text:0x3938
BOOL daSTR_c::initDoorWaitIn() {
    return TRUE;
}

// .text:0x3940
void daSTR_c::executeDoorWaitIn() {
    setDoorState(DOOR_STATE_CLOSED);
}

// .text:0x3954
BOOL daSTR_c::initDoorWaitOut() {
    return TRUE;
}

// .text:0x395C
void daSTR_c::executeDoorWaitOut() {
    setDoorState(DOOR_STATE_CLOSED);
}

static BOOL (daSTR_c::*const l_stateInit[daSTR_c::STATE_NUM])() = {
    &daSTR_c::initWait,
    &daSTR_c::initCheckLock,
    &daSTR_c::initTalkPutAway,
    &daSTR_c::initTalkWalk,
    &daSTR_c::initTalk,
    &daSTR_c::initTalkEnd,
    &daSTR_c::initEnterPutAway,
    &daSTR_c::initEnter,
    &daSTR_c::initEnterDirectPutAway,
    &daSTR_c::initEnterDirect,
    &daSTR_c::initEnterWait,
    &daSTR_c::initExit,
};

static void (daSTR_c::*const l_stateExecute[daSTR_c::STATE_NUM])() = {
    &daSTR_c::executeWait,
    &daSTR_c::executeCheckLock,
    &daSTR_c::executeTalkPutAway,
    &daSTR_c::executeTalkWalk,
    &daSTR_c::executeTalk,
    &daSTR_c::executeTalkEnd,
    &daSTR_c::executeEnterPutAway,
    &daSTR_c::executeEnter,
    &daSTR_c::executeEnterDirectPutAway,
    &daSTR_c::executeEnterDirect,
    &daSTR_c::executeEnterWait,
    &daSTR_c::executeExit,
};

// .text:0x3970
BOOL daSTR_c::setState(int state) {
    if (((mFlags >> 3) & 1) && state < STATE_NUM &&
        (this->*l_stateInit[state])()) {
        mState = state;
        return TRUE;
    }
    return FALSE;
}

// .text:0x39E8
void daSTR_c::executeState() {
    if (((mFlags >> 3) & 1) && mState < STATE_NUM) {
        (this->*l_stateExecute[mState])();
    }
}

// .text:0x3A34
BOOL daSTR_c::initWait() {
    return TRUE;
}

// .text:0x3A3C
void daSTR_c::executeWait() {
}

// .text:0x3A40
BOOL daSTR_c::initCheckLock() {
    if (!hasMsg()) {
        dSTR::lock(dItem::Item(mBuildingId));
    }
    return TRUE;
}

// .text:0x3A7C
void daSTR_c::executeCheckLock() {
    if (hasMsg()) {
        mTalkKind = TALK_KIND_MSG;
        setState(STATE_TALK_PUT_AWAY);
    } else if (dSTR::getLockState(dItem::Item(mBuildingId)) ==
               dSTR::LOCK_STATE_LOCKED) {
        int msg = getLockedMsg();
        if (msg != DOOR_MSG_NONE) {
            mMsgCode = msg;
            mTalkKind = TALK_KIND_LOCKED;
            setState(STATE_TALK_PUT_AWAY);
        } else {
            setState(isHingedDoor() ? STATE_ENTER_DIRECT_PUT_AWAY
                                    : STATE_ENTER_PUT_AWAY);
        }
    }
}

// .text:0x3B3C
BOOL daSTR_c::initTalkPutAway() {
    if (isWalkThrough()) {
        dPlayerMgr::putAwayToolForTalk(0);
    }
    return TRUE;
}

// .text:0x3B7C
void daSTR_c::executeTalkPutAway() {
    if (isWalkThrough()) {
        if (dPlayerMgr::canFinishToolAction()) {
            setState(STATE_TALK_WALK);
        }
    } else {
        setState(STATE_TALK_WALK);
    }
}

// .text:0x3BE4
BOOL daSTR_c::initTalkWalk() {
    if (isWalkThrough()) {
        dAcPlayer_c *player =
            dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player != NULL) {
            mAng angle;
            mDoorPos =
                dAStr::getNpcDoorPos(this, player->getPosP(), &angle, 16.0f);
            mDoorAngle = angle.mAngle + DEG_TO_ANGLE(180);
            int se = isWalkThrough() ? getKnockSe() : -1;
            if (dPlayerMgr::walkThroughDoor(mDoorPos.x, mDoorPos.z, mDoorAngle,
                                            se)) {
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
}

// .text:0x3CE4
void daSTR_c::executeTalkWalk() {
    if (isWalkThrough()) {
        if (dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
            setState(STATE_TALK);
        }
    } else {
        setState(STATE_TALK);
    }
}

// .text:0x3D50
BOOL daSTR_c::initTalk() {
    attachDemoActor(this);
    dMsg::Comp_c *comp = getController();
    setMessageLabel("sys_MESS/SYS_Door");
    setMessageCode(mMsgCode);
    onTalkStart(mTalkKind, comp);
    if (comp != NULL) {
        comp->requestStart(0);
    }
    return TRUE;
}

// .text:0x3DF0
void daSTR_c::executeTalk() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (!comp->isState(&dMsg::Comp_c::executeHidden)) {
            setState(STATE_TALK_END);
        }
    }
}

// .text:0x3E64
BOOL daSTR_c::initTalkEnd() {
    return TRUE;
}

// .text:0x3E6C
void daSTR_c::executeTalkEnd() {
    dMsg::Comp_c *comp = getController();
    if (comp != NULL) {
        if (comp->isState(&dMsg::Comp_c::executeHidden)) {
            detachDemoActor(this);
            if (mTalkKind == TALK_KIND_LOCKED) {
                setState(isHingedDoor() ? STATE_ENTER_DIRECT_PUT_AWAY
                                        : STATE_ENTER_PUT_AWAY);
            } else {
                dDemo::finishTalk(this);
            }
        }
    }
}

// .text:0x3F24
BOOL daSTR_c::initEnterPutAway() {
    dPlayerMgr::putAwayToolForTalk(0);
    return TRUE;
}

// .text:0x3F4C
void daSTR_c::executeEnterPutAway() {
    if (dPlayerMgr::canFinishToolAction()) {
        setState(STATE_ENTER);
    }
}

// .text:0x3F8C
BOOL daSTR_c::initEnter() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mAng angle;
        mDoorPos = dAStr::getNpcDoorPos(this, player->getPosP(), &angle, 16.0f);
        mDoorAngle = angle.mAngle + DEG_TO_ANGLE(180);
        int se = hasKnockSe() ? getKnockSe() : -1;
        if (dPlayerMgr::enterDoor(mDoorPos.x, mDoorPos.z, mDoorAngle,
                                  getDoorType(), se)) {
            dStrm_c::sDoorEntered = 1;
            return TRUE;
        }
    }
    return FALSE;
}

// .text:0x409C
void daSTR_c::executeEnter() {
    if (dPlayerMgr::isDoorAnimation()) {
        switch (getDoorType()) {
        case DOOR_AUTO:
        case DOOR_WALK:
            if (dCamera_c::sInstance != NULL) {
                dCamera_c::sInstance->mRequestedMode =
                    dCamera_c::MODE_PLAYER_VIEW_8;
            }
            if (getDoorType() == DOOR_WALK) {
                mEnterTimer = dSTR::getHostIO()->_3E;
            } else {
                mEnterTimer = dSTR::getHostIO()->_3C;
            }
            setState(STATE_ENTER_WAIT);
            break;
        }
    }
}

// .text:0x4154
BOOL daSTR_c::initEnterWait() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mAng angle;
        mDoorPos = dAStr::getNpcDoorPos(this, player->getPosP(), &angle, 16.0f);
        mDoorAngle = angle.mAngle + DEG_TO_ANGLE(180);
        mEnterTimer += dAStr::getEnterWait(this);
        return TRUE;
    }
    return FALSE;
}

// .text:0x41F4
void daSTR_c::executeEnterWait() {
    if (mEnterTimer != 0) {
        mEnterTimer--;
    }
    if (mEnterTimer == 0) {
        int offset = getSceneOffset();
        u32 baseScene = dScene::findSceneByBuilding(mBuildingId);
        u8 scene = baseScene + offset;
        mVec3_c pos(256.0f, 0.0f, 464.0f);
        if (getSceneChange()->request(scene, &pos, 0x4A, -0x8000, 2, 2)) {
            if (mBuildingId == BUILDING_CITY_BROKER) {
                dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
                if (player != NULL) {
                    player->setFlag0(PRIVATE_FLAG0_REDD_VISITED);
                }
            }
            mFlags |= FLAG_SCENE_CHANGE;
            onSceneChange(scene);
            setState(STATE_WAIT);
        }
    }
}

// .text:0x4300
BOOL daSTR_c::initExit() {
    mExitTimer = 0;
    return TRUE;
}

// .text:0x4310
void daSTR_c::executeExit() {
    if (mExitTimer == EXIT_DOOR_WAIT && dPlayerMgr::exitDoor(getDoorType())) {
        if (setState(STATE_WAIT) && isSummerMorningStart()) {
            if (dWeather_c::sInstance != NULL) {
                if (!dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
                    playSe(SE_sky_BIRD);
                }
            }
            clearSummerMorningStart();
        }
        mExitTimer = EXIT_DOOR_WAIT + 1;
    }
    if (mExitTimer < EXIT_DOOR_WAIT) {
        mExitTimer++;
    }
}

// .text:0x43DC
BOOL daSTR_c::initEnterDirectPutAway() {
    dPlayerMgr::putAwayToolForTalk(0);
    return TRUE;
}

// .text:0x4404
void daSTR_c::executeEnterDirectPutAway() {
    if (dPlayerMgr::canFinishToolAction()) {
        setState(STATE_ENTER_DIRECT);
    }
}

// .text:0x4444
BOOL daSTR_c::initEnterDirect() {
    return TRUE;
}

// .text:0x444C
void daSTR_c::executeEnterDirect() {
    dAcPlayer_c *player =
        dPlayerMgr::getPlayerActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (player != NULL) {
        mAng angle;
        mDoorPos = dAStr::getNpcDoorPos(this, player->getPosP(), &angle, 16.0f);
        mDoorAngle = angle.mAngle + DEG_TO_ANGLE(180);
        if (dPlayerMgr::enterDoorDirect(mDoorPos.x, mDoorPos.z, mDoorAngle,
                                        getDoorType(), FALSE)) {
            mEnterTimer = dSTR::getHostIO()->_3E;
            setState(STATE_ENTER_WAIT);
        }
    }
}

// .text:0x4518
void daSTR_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_ENTER:
        dSTR::unlock(dItem::Item(mBuildingId), 0);
        dSTR::clearPartyFlag();
        setState(STATE_EXIT);
        break;
    case DEMO_CMD_SPEAK:
        mFlags &= ~FLAG_TALK;
        setState(STATE_CHECK_LOCK);
        break;
    case DEMO_CMD_TALK:
        mFlags |= FLAG_TALK;
        setState(STATE_TALK_WALK);
        break;
    case DEMO_CMD_END:
        setState(STATE_WAIT);
        mMsgCode = -1;
        break;
    }
}

// .text:0x45D8
bool daSTR_c::canInteract(dDemoActor_c *actor) {
    if (actor == NULL) {
        return false;
    }
    if (!mDemoPos.isZero()) {
        if (sLib::distanceAngle(mAngle.y.mAngle + DEG_TO_ANGLE(180),
                                actor->getAngle().y) <= DOOR_TALK_ANGLE) {
            mVec3_c diff = mDemoPos - *actor->getDemoPosition();
            if (EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z) <=
                48.0f) {
                clearDemoState();
                return true;
            }
        }
    }
    return false;
}

// .text:0x46D8
mVec3_c *daSTR_c::getDemoPosition() {
    if (mDemoPos.isZero()) {
        return &mPos;
    }
    return &mDemoPos;
}

// A building and where it stands, packed (as dSTR::getCurBuildingCode).
static inline u32 makeBuildingCode(int x, int z, int idx) {
    return (x & 0xFF) | ((z & 0xFF) << 8) |
           (dSTR::getBuildingInfo(idx)->mProfile << 16);
}

// .text:0x4724
u16 daSTR_c::getDemoCode() const {
    return makeBuildingCode(mUnitX, mUnitZ, mBuildingIdx);
}

namespace dAStr {
// .text:0x4774
BOOL requestNpcEnter(daSTR_c *str) {
    switch (str->getDoorType()) {
    case daSTR_c::DOOR_AUTO:
        return str->setDoorState(daSTR_c::DOOR_STATE_OPEN_IN);
    case daSTR_c::DOOR_WALK:
        return str->setDoorState(daSTR_c::DOOR_STATE_WAIT_IN);
    case daSTR_c::DOOR_HINGED:
        return str->setDoorState(daSTR_c::DOOR_STATE_AUTO_OPEN);
    }
    return FALSE;
}

// .text:0x4828
BOOL requestNpcExit(daSTR_c *str) {
    switch (str->getDoorType()) {
    case daSTR_c::DOOR_AUTO:
        return str->setDoorState(daSTR_c::DOOR_STATE_OPEN_OUT);
    case daSTR_c::DOOR_WALK:
        return str->setDoorState(daSTR_c::DOOR_STATE_WAIT_OUT);
    case daSTR_c::DOOR_HINGED:
        return str->setDoorState(daSTR_c::DOOR_STATE_AUTO_OPEN);
    }
    return FALSE;
}

// .text:0x48DC
int getEnterWait(daSTR_c *str) {
    return str->isHingedDoor() ? 0x24 : 0;
}

// .text:0x490C
int getExitWait(daSTR_c *str) {
    if (str->isHingedDoor()) {
        nw4r::g3d::ResAnmChr anm(NULL);
        if (str->getDoorAnm(&anm, 0)) {
            return anm.ref().info.numFrame;
        }
    }
    return 0;
}
} // namespace dAStr

// .text:0x497C
int daSTR_c::getKnockSe() {
    return 0x1758;
}

// .text:0x4984
void daSTR_c::nodeCallBack_c::timingA(ulong nodeId,
                                      nw4r::g3d::ChrAnmResult *anmRes,
                                      nw4r::g3d::ResMdl resMdl) {
    nw4r::g3d::ResNode node = resMdl.GetResNode(nodeId);
    dTime_c now = *dTime_c::getCurrent();
    int hour = now.hour;
    int min = now.min;
    if (mOwner != NULL && mOwner->isAutoDoor()) {
        nw4r::math::MTX34 mtx;
        anmRes->GetRotTrans(&mtx);
        mVec3_c trans(node.ref().translate);
        mtx._03 = trans.x;
        mtx._13 = trans.y;
        mtx._23 = trans.z;
        anmRes->SetRotTrans(&mtx);
    }
    if (mCurve && mOwner != NULL) {
        nw4r::math::MTX34 mtx;
        anmRes->GetRotTrans(&mtx);
        mVec3_c trans(mtx._03, mtx._13, mtx._23);
        if (!trans.isZero()) {
            mVec3_c curved;
            mAng tilt = dWorld::curvePositionSimple(&curved, &trans);
            curved.y -= dWorld::getRadius();
            ((mMtx_c *)&mtx)->YrotM(-tilt);
            mtx._03 = curved.x;
            mtx._13 = curved.y;
            mtx._23 = curved.z;
            anmRes->SetRotTrans(&mtx);
        }
    }
    if (nodeId == mHourNode) {
        int minutes = hour % 12;
        minutes *= 60;
        minutes += min;
        nw4r::math::MTX34 mtx;
        anmRes->GetRotTrans(&mtx);
        ((mMtx_c *)&mtx)->ZrotM(-(s16)((minutes << 16) / 720));
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            mtx._23 += 6.0f;
        }
        anmRes->SetRotTrans(&mtx);
    } else if (nodeId == mMinuteNode) {
        nw4r::math::MTX34 mtx;
        anmRes->GetRotTrans(&mtx);
        ((mMtx_c *)&mtx)->ZrotM(-(s16)((min << 16) / 60));
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
            mtx._23 += 6.0f;
        }
        anmRes->SetRotTrans(&mtx);
    }
}

// .text:0x4C6C
BOOL daSTR_c::fixDoorVerts() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        return FALSE;
    }
    if (!dSTR::getBrresBank()->isFlag0(mResName)) {
        mResFile.GetResMdl(0);
        dBuildingInfo_c::polygons_s polygons =
            dSTR::getBuildingInfo(mBuildingIdx)->mPolygons;
        BOOL curved = FALSE;
        for (u32 i = 0; i < ARRAY_SIZE(polygons.mNames); i++) {
            const char *name = polygons.mNames[i];
            if (name != NULL) {
                nw4r::g3d::ResMdl mdl = mResFile.GetResMdl(0);
                nw4r::g3d::ResShp shp = mdl.GetResShp(name);
                if (shp.IsValid()) {
                    nw4r::g3d::ResVtxPos vtx = shp.GetResVtxPos();
                    u16 num = vtx.ref().numPos;
                    if (num != -1) {
                        mVec3_c *pos = (mVec3_c *)vtx.GetData();
                        for (u32 j = 0; j < num; j++, pos++) {
                            f32 y = mPos.y;
                            mVec3_c p = *pos;
                            p.y += y;
                            mVec3_c curvedPos;
                            dWorld::curvePositionSimple(&curvedPos, &p);
                            curvedPos.y -= y + dWorld::getRadius();
                            *pos = curvedPos;
                        }
                        curved = TRUE;
                    }
                }
            }
        }
        dSTR::getBrresBank()->setFlag0(mResName);
        if (curved) {
            dSTR::getBrresBank()->setFlag1(mResName);
        }
    }
    return dSTR::getBrresBank()->isFlag1(mResName);
}

// .text:0x4E58
void daSTR_c::updateSoundPos() {
    if (!isNoDoor()) {
        if (mSoundPos.isZero()) {
            if (mEntArea != NULL) {
                mVec3_c min, max;
                mEntArea->getBox(&min, &max, mPos, 0);
                mVec3_c mid = (min + max) * 0.5f;
                mSoundPos = mid;
            } else {
                mSoundPos = mPos;
            }
            mSnd.setPos(&mSoundPos);
        } else {
            mSnd.setPos(&mSoundPos);
        }
    }
}

// .text:0x4F58
void daSTR_c::executeExtra() {
}

// .text:0x4F5C
void daSTR_c::onSceneChange(u8 scene) {
    dTheater::common_c *common = &dTheater::g_common;
    if (common != NULL) {
        common->clear();
    }
}

#ifdef MUST_MATCH
// Unused (dead-stripped by the linker): only needs m3d's weak inlines in the
// target's order (callback_c::timingC, timingB, anmChr_c::getType), ahead of
// the end-of-TU vtable pass that adds anmShp_c::getType and
// callback_c::timingA.
static void orderM3dWeakInlines(m3d::mdl_c::callback_c *cb, m3d::anmChr_c *anm,
                                nw4r::g3d::ResMdl resMdl) {
    cb->m3d::mdl_c::callback_c::timingC(NULL, resMdl);
    cb->m3d::mdl_c::callback_c::timingB(0, NULL, resMdl);
    anm->m3d::anmChr_c::getType();
}
#endif
