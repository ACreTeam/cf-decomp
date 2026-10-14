// The lights of the light settings. See include/game/game/d_light.hpp.
#include <game/game/d_light.hpp>
#include <game/mLib/m_3d.hpp>

namespace dLight {

static int sLightObjNum;
static int sAmbLightObjNum;
static int sLightNum[LIGHT_SET_NUM];
static GXLightID sLightMask[LIGHT_SET_NUM];
static int sLightSet;
static LightReq_c *sLightReqs;
static u32 sLightReqNum;
static int sPrevLightSet = LIGHT_SET_NUM;

// 800CCCD8
BOOL init() {
    sLightSet = 0;
    sPrevLightSet = LIGHT_SET_NUM;
    resetLightNum();
    nw4r::g3d::LightSetting *setting = m3d::getLightSettingP();
    for (int i = 0; i < LIGHT_SET_NUM; i++) {
        sLightMask[i] = GX_LIGHT_NULL;
        nw4r::g3d::LightSet lightSet = setting->GetLightSet(i);
        lightSet.SelectAmbLightObj(-1);
        for (int j = 0; j < 8; j++) {
            lightSet.SelectLightObj(j, -1);
        }
    }
    nw4r::g3d::LightObj *obj = setting->GetLightObjArray();
    for (u32 i = 0; i < setting->GetNumLightObj(); i++) {
        obj[i].Clear();
    }
    return TRUE;
}

// 800CCDA4
BOOL setLightReq(void *reqs, u32 num) {
    sLightReqs = static_cast<LightReq_c *>(reqs);
    sLightReqNum = num;
    return TRUE;
}

// 800CCDB4
BOOL execLightReq(LightReqFunc func, void *arg) {
    LightReq_c *req = sLightReqs;
    if (req == NULL || sLightReqNum == 0) {
        sLightReqs = NULL;
        sLightReqNum = 0;
        return FALSE;
    }
    for (u32 i = 0; i < sLightReqNum; i++) {
        func(req, arg);
        req++;
    }
    sLightReqs = NULL;
    sLightReqNum = 0;
    return TRUE;
}

// 800CCE64
void resetLightNum() {
    sLightNum[0] = 0;
    sLightNum[1] = 0;
    sLightObjNum = 0;
    sAmbLightObjNum = 0;
}

// 800CCE80
int getNextLightId() {
    return sLightNum[sLightSet]++;
}

// 800CCE9C
int getNextLightObjIdx() {
    return sLightObjNum++;
}

// 800CCEAC
void changeLightSet(int lightSet) {
    sPrevLightSet = sLightSet;
    sLightSet = lightSet;
}

// 800CCEBC
void restoreLightSet() {
    sLightSet = sPrevLightSet;
}

static inline nw4r::g3d::LightObj *getLightObj(u32 lightId, int lightSet) {
    return getLightSet(lightSet).GetLightObj(lightId);
}

// 800CCEC8
BOOL setPointLight(u32 lightId, mColor &color, const mVec3_c &pos, f32 k0, f32 k1, f32 k2, int lightSet) {
    nw4r::g3d::LightObj *obj = getLightObj(lightId, lightSet);
    obj->InitLightColor(color);
    mVec3_c dir(1.0f, 0.0f, 0.0f);
    obj->InitLightDir(dir.x, dir.y, dir.z);
    obj->InitLightPos(pos.x, pos.y, pos.z);
    obj->InitLightAttnA(1.0f, 0.0f, 0.0f);
    obj->InitLightAttnK(k0, k1, k2);
    return TRUE;
}

// 800CCFE8
BOOL setSpotLight(u32 lightId, mColor &color, const mVec3_c &pos, const mVec3_c &dir, f32 k0, f32 k1, f32 k2,
                  f32 cutoff, GXSpotFn spotFn, int lightSet) {
    nw4r::g3d::LightObj *obj = getLightObj(lightId, lightSet);
    obj->InitLightColor(color);
    obj->InitLightDir(dir.x, dir.y, dir.z);
    obj->InitLightPos(pos.x, pos.y, pos.z);
    obj->InitLightSpot(cutoff, spotFn);
    obj->InitLightAttnK(k0, k1, k2);
    return TRUE;
}

// 800CD10C
BOOL setAmbColor(mColor &color, int lightSet) {
    nw4r::g3d::AmbLightObj *amb = getLightSet(lightSet).GetAmbLightObj();
    amb->r = color.r;
    amb->g = color.g;
    amb->b = color.b;
    amb->a = color.a;
    return TRUE;
}

// 800CD168
mColor getAmbColor(int lightSet) {
    nw4r::g3d::AmbLightObj *amb = getLightSet(lightSet).GetAmbLightObj();
    return mColor(amb->r, amb->g, amb->b, amb->a);
}

// 800CD1C8
nw4r::g3d::LightSet getLightSet(int lightSet) {
    return m3d::getLightSettingP()->GetLightSet(lightSet);
}

// 800CD210
nw4r::g3d::LightObj *getLightObj(u32 lightId) {
    return getLightObj(lightId, sLightSet);
}

// 800CD250
GXLightID getLightMask() {
    return sLightMask[sLightSet];
}

// 800CD264
void AbstractLight_c::regist() {
    mLightId = getNextLightId();
    mLightSetIdx = sLightSet;
    mLightObjIdx = getNextLightObjIdx();
    getLightSet(mLightSetIdx).SelectLightObj(mLightId, mLightObjIdx);
    getLightObj(mLightId)->Enable();
    sLightMask[sLightSet] = (GXLightID)(sLightMask[sLightSet] | (1 << mLightId));
}

// 800CD2F8
void AmbLight_c::apply() {
    setAmbColor(mLightColor, mLightSetIdx);
}

// 800CD304
void AmbLight_c::regist() {
    mLightObjIdx = mLightId = sAmbLightObjNum++;
    mLightSetIdx = sLightSet;
    getLightSet(mLightSetIdx).SelectAmbLightObj(mLightObjIdx);
}

// 800CD35C
void FarLight_c::apply() {
    nw4r::g3d::LightObj *obj = getLightObj(mLightId, mLightSetIdx);
    obj->InitLightColor(mLightColor);
    mDir.normalize();
    obj->InitLightDir(mDir.x, mDir.y, mDir.z);
    mVec3_c pos = -8000000.0f * mDir;
    obj->InitLightPos(pos.x, pos.y, pos.z);
    obj->InitLightAttnA(1.0f, 0.0f, 0.0f);
    obj->InitLightAttnK(1.0f, 0.0f, 0.0f);
}

// 800CD43C
void PointLight_c::apply() {
    setPointLight(mLightId, mLightColor, mPos, mDistAttn[0], mDistAttn[1], mDistAttn[2], mLightSetIdx);
}

// 800CD460
void SpotLight_c::apply() {
    setSpotLight(mLightId, mLightColor, mPos, mDir, mDistAttn[0], mDistAttn[1], mDistAttn[2], mCutoff, mSpotFn,
                 mLightSetIdx);
}

} // namespace dLight
