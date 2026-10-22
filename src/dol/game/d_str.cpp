// dSTR: town structures (banks, flatness/copy checks, collision bank) and the per-unit field
// attribute map dFdUnitAttr_c. .text 80166990..8016AE68.
#include <game/game/d_str.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_save_building.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <game/game/d_bg.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_color.hpp>
#include <game/sLib/s_lib.hpp>
#include <lib/egg/math/eggMath.h>
#include <cstdio>
#include <cstring>
#include <cmath>


// 80166990
dScene::dSceneChange_c *dSTR::getSceneChange() {
    return &dScene::gSceneChange;
}

namespace dSTR {

// 8016699C
hostIO_c::hostIO_c() {
    _08 = 0;
    _0A = 0;
    _16 = 14;
    _0C = -22;
    _14 = 7;
    _18 = 0;
    _05 = 0;
    _04 = 0;
    _06 = 0;
    _38 = 0.8728f;
    _34 = 1.0f;
    _3C = 0x4C;
    _3E = 0x44;
    _10 = 0.0f;
    _1E = 0xFF;
    _20 = 0xFF;
    _22 = 0x80;
    _1A = 0;
    _26 = 0xFF;
    _28 = 0;
    _2A = 0;
    _24 = 0;
    _2E = 0xFF;
    _30 = 0;
    _32 = 0;
    _2C = 0;
    _19 = 0;
    _1C = 45;
    _40 = 13;
    _44 = 44.0f;
    _48 = 0xD5;
}

} // namespace dSTR

// 80166A7C
dSTR::hostIO_c *dSTR::getHostIO() {
    static dSTR::hostIO_c s_hostIO;
    return &s_hostIO;
}

namespace dSTR {

const char *cNameTbl[STR_NUM] = {
    "obj_balloon",
    "obj_bus",
    "obj_str_common",
    "obj_balloonC",
    "obj_bbs",
    "obj_bbs_bird",
    "obj_buildsite",
    "obj_busstop0",
    "obj_carnival_bord",
    "obj_cdnum",
    "obj_check0",
    "obj_check1",
    "obj_check2",
    "obj_fountain",
    "obj_harvest0",
    "obj_harvest1",
    "obj_lighthouse",
    "obj_museum",
    "obj_myhome0",
    "obj_myhome1",
    "obj_myhome2",
    "obj_myhome3",
    "obj_myhome4",
    "obj_myhome_common",
    "obj_npc_common",
    "obj_office",
    "obj_post",
    "obj_tent0",
    "obj_tent1",
    "obj_windmill",
    "ufo0",
    "ufo1",
    "obj_cdbbs",
    "obj_nc",
    "obj_ng",
    "obj_nk",
    "obj_nw",
    "obj_tailor",
    "TN_obj_house0",
    "TN_obj_house1",
    "TN_obj_house2",
    "TN_obj_house3",
    "TN_obj_house4",
    "obj_house_tex",
    "obj_busstop1",
    "obj_tw_ATM",
    "obj_tw_HRA",
    "obj_tw_auction",
    "obj_tw_cut",
    "obj_tw_etc00",
    "obj_tw_etc01",
    "obj_tw_etc02",
    "obj_tw_etc03",
    "obj_tw_etc04",
    "obj_tw_fortune",
    "obj_tw_fountain",
    "obj_tw_grace",
    "obj_tw_lamp",
    "obj_tw_organ",
    "obj_tw_reset",
    "obj_tw_shoeblack",
    "obj_tw_theater",
    "obj_tw_tsunekichi",
};

// By getTownRegion.
static const char *l_regionName[] = {"Jpn", "Usa", "Eur", "Kor"};
// By getTownLanguage.
static const char *l_langName[] = {"Jpn", "Eng", "Spa", "Fra", "Eng", "Spa", "Fra", "Ita", "Ger", "Kor"};

// 80166AE0
const char *getName(u32 idx) {
    if (idx < STR_NUM) {
        return cNameTbl[idx];
    }
    return cNameTbl[0];
}

// 80166B08
int getIndex(const char *name) {
    for (u32 i = 0; i < STR_NUM; i++) {
        if (strcmp(name, getName(i)) == 0) {
            return i;
        }
    }
    return -1;
}

// 80166B70
viewArea_c::viewArea_c(f32 front, f32 back, f32 halfWidth) {
    set(front, back, halfWidth);
}

// 80166BA0
void viewArea_c::set(f32 front, f32 back, f32 halfWidth) {
    mFront = front;
    mBack = back;
    mHalfWidth = 4.0f / 3.0f * halfWidth;
}

// 80166BB8
BOOL viewArea_c::isIn(const mVec3_c &pos) const {
    f32 dx = pos.x - dCamera_c::sCenter.x;
    f32 dz = pos.z - dCamera_c::sCenter.z;
    return !isOutside(std::fabs(dx), dz);
}

// 80166C14
BOOL strFlag_c::is(const char *name) const {
    int idx = getIndex(name);
    if (idx != -1) {
        return mFlags[idx] != 0;
    }
    return FALSE;
}

// 80166C64
void strFlag_c::set(const char *name) {
    int idx = getIndex(name);
    if (idx != -1) {
        mFlags[idx] = TRUE;
    }
}

// 80166CA4
BOOL townTexBank_c::load(void *heap) {
    char path[0x32];
    dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_CITY_FIELD);
    int region = dSTR::getTownRegion();
    int lang = dSTR::getTownLanguage();
    s8 season = dScene::getSeasonType();
    snprintf(path, sizeof(path), "/Str/Brres/%s%sTownTex%c.brres", l_regionName[region], l_langName[lang],
             season);
    return bank_c::load(path, heap, 0);
}

static strFlag_c l_flag0;
static strFlag_c l_flag1;
static entArea_c l_entArea[STR_NUM];
static colArea_c l_colArea[STR_NUM];

// 80166D4C
brresBank_c::brresBank_c() {
    clear();
}

// 80166DC4
void brresBank_c::clearFlags() {
    l_flag0.clear();
    l_flag1.clear();
}

// 80166E08
void brresBank_c::clear() {
    mRegion = 0xFF;
    mLang = 0xFF;
    mSeason = 0;
    mIsField = 0;
    _78 = 0;
    clearFlags();
}

// 80166E28
BOOL brresBank_c::isFlag0(const char *name) {
    return l_flag0.is(name);
}

// 80166E34
void brresBank_c::setFlag0(const char *name) {
    l_flag0.set(name);
}

// 80166E40
BOOL brresBank_c::isFlag1(const char *name) {
    return l_flag1.is(name);
}

// 80166E4C
void brresBank_c::setFlag1(const char *name) {
    l_flag1.set(name);
}

// 80166E58
BOOL brresBank_c::isSame(u32 isField, s8 season, int region, int lang) {
    BOOL same = FALSE;
    if (isField == mIsField && season == mSeason && region == mRegion && lang == mLang) {
        same = TRUE;
    }
    return same;
}

static const char *l_fieldName[] = {"field", "town"};

// 80166EA0
BOOL brresBank_c::load(void *heap) {
    char path[0x32];
    BOOL city = dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_CITY_FIELD);
    const char *field = l_fieldName[city];
    int region = dSTR::getCurRegion();
    int lang = dSTR::getCurLanguage();
    s8 season = dScene::getSeasonType();
    BOOL needTex = FALSE;
    if (mpData == NULL && city) {
        needTex = TRUE;
    }
    BOOL ok = TRUE;
    if (needTex) {
        ok = FALSE;
        if (mTownTex.load(heap)) {
            ok = TRUE;
            mTownTexRes = nw4r::g3d::ResFile(mTownTex.getData());
        } else {
            mTownTexRes = nw4r::g3d::ResFile(NULL);
        }
    }
    if (ok) {
        snprintf(path, sizeof(path), "/Str/Brres/%s%s%sStrBrres%c.arc", field, l_regionName[region],
                 l_langName[lang], season);
        if (bank_c::load(path, heap, 0)) {
            mSeason = season;
            mRegion = dSTR::getTownRegion();
            mLang = dSTR::getTownLanguage();
            mIsField = !city;
            return TRUE;
        }
    }
    return FALSE;
}

// 80166FF0
BOOL brresBank_c::isLoaded() {
    bool ok = true;
    ok &= checkLoaded();
    ok &= mTownTex.checkLoaded();
    return ok;
}

// 80167040
BOOL brresBank_c::unload() {
    bool ok = true;
    ok &= closeArc();
    ok &= mTownTex.unload(FALSE);
    return ok;
}

// 80167094
void brresBank_c::onLoaded() {
    arcBank_c::onLoaded();
    dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD);
    for (u32 i = 0; i < STR_NUM; i++) {
        nw4r::g3d::ResFile res(getBrres(getName(i)));
        if (res.IsValid()) {
            res.Init();
            if (mTownTexRes.IsValid()) {
                res.Bind(mTownTexRes);
            }
            res.Bind();
        }
    }
    nw4r::g3d::ResFile npc(getBrres("obj_npc_common"));
    if (npc.IsValid()) {
        npc.Bind(nw4r::g3d::ResFile(getBrres("obj_house_tex")));
        for (u32 i = 0; i < dSTR::getNpcHouseVariantNum(); i++) {
            char name[0x1E];
            snprintf(name, sizeof(name), "TN_obj_house%d", i);
            nw4r::g3d::ResFile(getBrres(name)).Bind(nw4r::g3d::ResFile(getBrres("obj_house_tex")));
        }
    }
}

// 801671EC
void *brresBank_c::getBrres(const char *name) {
    char path[0x32];
    snprintf(path, sizeof(path), "strBrres/%s.brres", name);
    u32 size;
    void *data = getFile(path, &size);
    if (data != NULL) {
        return data;
    }
    return NULL;
}

static const char *l_colArcPath = "/Str/Col/strCol.arc";

// 80167250
BOOL colBank_c::load(void *heap) {
    return bank_c::load(l_colArcPath, heap, 0);
}

// 80167260
void colBank_c::onLoaded() {
    arcBank_c::onLoaded();
    for (u32 i = 0; i < STR_NUM; i++) {
        dBGCF::unitDat_c *data = getCol(getName(i));
        if (data == NULL) {
            continue;
        }
        int entMinX, entMinZ, entMaxX, entMaxZ;
        int colMinX, colMinZ, colMaxX, colMaxZ;
        BOOL toRU, toLU;
        int doorX, doorZ;
        toRU = FALSE;
        toLU = FALSE;
        entMaxZ = -1;
        entMinZ = -1;
        entMaxX = -1;
        entMinX = -1;
        colMaxZ = -1;
        colMinZ = -1;
        colMaxX = -1;
        colMinX = -1;
        doorZ = -1;
        doorX = -1;
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                switch (dBGCF::getBlockUnitAttr(data, x, z)) {
                case BG_ATTR_STR_ENT_TO_RU:
                    doorX = x;
                    doorZ = z;
                    toRU = TRUE;
                    break;
                case BG_ATTR_STR_ENT_TO_LU:
                    doorX = x;
                    doorZ = z;
                    toLU = TRUE;
                    break;
                case BG_ATTR_STR_ENT:
                    if (entMaxX == -1 || x > entMaxX) {
                        entMaxX = x;
                    }
                    if (entMaxZ == -1 || z > entMaxZ) {
                        entMaxZ = z;
                    }
                    if (entMinX == -1 || x < entMinX) {
                        entMinX = x;
                    }
                    if (entMinZ == -1 || z < entMinZ) {
                        entMinZ = z;
                    }
                    break;
                case BG_ATTR_STR_COL:
                    if (colMaxX == -1 || x > colMaxX) {
                        colMaxX = x;
                    }
                    if (colMaxZ == -1 || z > colMaxZ) {
                        colMaxZ = z;
                    }
                    if (colMinX == -1 || x < colMinX) {
                        colMinX = x;
                    }
                    if (colMinZ == -1 || z < colMinZ) {
                        colMinZ = z;
                    }
                    break;
                }
            }
        }
        if (colMaxX != -1) {
            l_colArea[i].mMin = mVec3_c(32.0f * (colMinX - 7) - 16.0f, 0.0f, 32.0f * (colMinZ - 7) - 16.0f);
            l_colArea[i].mMax = mVec3_c(32.0f * (colMaxX - 6) - 16.0f, 0.0f, 32.0f * (colMaxZ - 6) - 16.0f);
            l_colArea[i].mValid = TRUE;
        }
        if (toRU) {
            mVec3_c pos;
            dBGCF::unitToPos(&pos, doorX - 7, doorZ - 7);
            pos.x -= 16.0f;
            pos.z -= 16.0f;
            l_entArea[i].mMin = pos;
            l_entArea[i].mMax = pos;
            l_entArea[i].mMin.x -= 16.0f;
            l_entArea[i].mMin.z -= 16.0f;
            l_entArea[i].mMax.x += 16.0f;
            l_entArea[i].mMax.z += 16.0f;
            l_entArea[i].mAngle = -0x2000;
            l_entArea[i].mValid = TRUE;
        } else if (toLU) {
            mVec3_c pos;
            dBGCF::unitToPos(&pos, doorX - 7, doorZ - 7);
            pos.x -= 16.0f;
            pos.z -= 16.0f;
            l_entArea[i].mMin = pos;
            l_entArea[i].mMax = pos;
            l_entArea[i].mMin.x -= 16.0f;
            l_entArea[i].mMin.z += 16.0f;
            l_entArea[i].mMax.x += 16.0f;
            l_entArea[i].mMax.z -= 16.0f;
            l_entArea[i].mAngle = 0x2000;
            l_entArea[i].mValid = TRUE;
        } else if (entMaxX != -1) {
            l_entArea[i].mMin = mVec3_c(32.0f * (entMinX - 7) - 16.0f, 0.0f, 32.0f * (entMinZ - 7) - 16.0f);
            l_entArea[i].mMax = mVec3_c(32.0f * (entMaxX - 6) - 16.0f, 0.0f, 32.0f * (entMinZ - 7) - 16.0f);
            l_entArea[i].mAngle = 0;
            l_entArea[i].mValid = TRUE;
        }
    }
}

// 801676E4
entArea_c *colBank_c::getEntArea(const char *name) {
    int idx = getIndex(name);
    if (idx != -1) {
        entArea_c *area = &l_entArea[idx];
        if (area->mValid) {
            return area;
        }
    }
    return NULL;
}

// 80167734
entArea_c *colBank_c::getEntArea(u16 building) {
    dItem::Item item(building);
    if (item.isExtId()) {
        int idx = item.getExtIndex();
        if (idx != -1) {
            return getEntArea(dSTR::getBuildingResName(idx));
        }
    }
    return NULL;
}

// 801677C8
colArea_c *colBank_c::getColArea(const char *name) {
    int idx = getIndex(name);
    if (idx != -1) {
        colArea_c *area = &l_colArea[idx];
        if (area->mValid) {
            return area;
        }
    }
    return NULL;
}

// 80167818
colArea_c *colBank_c::getColArea(u16 building) {
    dItem::Item item(building);
    if (item.isExtId()) {
        int idx = item.getExtIndex();
        if (idx != -1) {
            return getColArea(dSTR::getBuildingResName(idx));
        }
    }
    return NULL;
}

// 801678AC
dBGCF::unitDat_c *colBank_c::getCol(int building) {
    const char *name = dSTR::getBuildingResName(building);
    if (name != NULL) {
        return getCol(name);
    }
    return NULL;
}

// 801678F8
dBGCF::unitDat_c *colBank_c::getCol(const char *name) {
    char path[0x32];
    snprintf(path, sizeof(path), "strCol/%s.bin", name);
    u32 size;
    dBGCF::unitDat_c *data = (dBGCF::unitDat_c *)getFile(path, &size);
    if (data != NULL) {
        return data;
    }
    return NULL;
}

} // namespace dSTR

// 8016795C
void dFdUnitAttr_c::clear() {
    memset(this, 0, sizeof(mBlocks));
}

// 80167968
dFdUnitAttr_c::dFdUnitAttr_c(int field) {
    clear();
    mField = field;
}

// 801679A8
BOOL dFdUnitAttr_c::setBuilding(u16 building) {
    dItem::Item item(building);
    if (item.isExtId()) {
        dSaveBuildingList_c *list = dSaveBuildingList_c::get();
        int idx = item.getExtIndex();
        int x, z;
        dItem::Item id(building);
        if (list->getPos(&x, &z, id, mField)) {
            dBGCF::unitDat_c *data = dSTR::getColBank()->getCol(idx);
            if (data != NULL) {
                for (int uz = 0; uz < 16; uz++) {
                    for (int ux = 0; ux < 16; ux++) {
                        int attr = dBGCF::getBlockUnitAttr(data, ux, uz);
                        int unitX = ux + x - 7;
                        int unitZ = uz + z - 7;
                        switch (attr) {
                        case BG_ATTR_STR_COL:
                            setAttr(unitX, unitZ, STR_COL);
                            break;
                        case BG_ATTR_STR_ENT:
                            setAttr(unitX, unitZ, STR_ENT);
                            break;
                        case BG_ATTR_STR_ENT_TO_RU:
                        case BG_ATTR_STR_ENT_TO_LU:
                            setAttr(unitX, unitZ, STR_COL | STR_ENT);
                            break;
                        case BG_ATTR_STR_INS:
                            setAttr(unitX, unitZ, STR_DEL | STR_INS);
                            break;
                        case BG_ATTR_STR_DEL:
                            setAttr(unitX, unitZ, STR_DEL);
                            break;
                        }
                    }
                }
            } else {
                setAttr(x, z, STR_COL);
            }
            if (building == BUILDING_BUS_STOP) {
                setAttr(x - 1, z, STR_COL);
                setAttr(x - 2, z, STR_COL);
                setAttr(x - 3, z, STR_COL);
            }
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 80167BAC
void dFdUnitAttr_c::setAll() {
    clear();
    dSaveBuildingList_c::get();
    for (u32 i = 0; i < BUILDING_NUM; i++) {
        setBuilding(BUILDING_BUILD_SITE + i);
    }
}

// 80167C08
void dFdUnitAttr_c::setAttr(int unitX, int unitZ, u32 attr) {
    int blockX;
    int blockZ;
    dBGCF::unitToBlock(&blockX, &blockZ, unitX, unitZ);
    if (blockX >= 0 && (u32)blockX < BLOCK_X_NUM && blockZ >= 0 && (u32)blockZ < BLOCK_Z_NUM) {
        int idx = (unitX & (UT_X_NUM - 1)) + (unitZ & (UT_Z_NUM - 1)) * UT_X_NUM;
        mBlocks[blockZ][blockX].set(idx, attr);
    }
}

// 80167CD8
u32 dFdUnitAttr_c::getAttr(int unitX, int unitZ) {
    int blockX;
    int blockZ;
    dBGCF::unitToBlock(&blockX, &blockZ, unitX, unitZ);
    if (blockX >= 0 && (u32)blockX < BLOCK_X_NUM && blockZ >= 0 && (u32)blockZ < BLOCK_Z_NUM) {
        int idx = (unitX & (UT_X_NUM - 1)) + (unitZ & (UT_Z_NUM - 1)) * UT_X_NUM;
        return mBlocks[blockZ][blockX].get(idx);
    }
    return 0;
}

// 80167D8C
BOOL dFdUnitAttr_c::isGroundFree(int unitX, int unitZ) {
    return (getAttr(unitX, unitZ) & (STR_COL | STR_ENT)) == 0;
}

// 80167DB8
BOOL dFdUnitAttr_c::isNotStrCol(int unitX, int unitZ) {
    return !(getAttr(unitX, unitZ) & STR_COL);
}

// 80167DE4
BOOL dFdUnitAttr_c::isNoPlantUnit(int unitX, int unitZ) {
    return (getAttr(unitX, unitZ) & STR_DEL) != 0;
}

#include <game/game/d_a_str.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_region.hpp>

// 80167E08
void dSTR::light_c::init() {
    mFlicker = 0;
    mValue = 0.0f;
    mTarget = 0.0f;
}

// 80167E20
BOOL dSTR::light_c::set(BOOL on) {
    return set(on, FALSE, FALSE, 0.1f);
}

// The flicker of a light coming on (light_c::update).
static const f32 l_flickerTable[22] = {
    0.0f, 0.0f, 0.1f, 0.1f, 0.2f, 0.2f, 0.3f, 0.3f, 0.2f, 0.2f, 0.1f,
    0.1f, 0.3f, 0.3f, 0.4f, 0.4f, 0.5f, 0.5f, 0.4f, 0.4f, 0.3f, 0.3f,
};

// 80167E30
BOOL dSTR::light_c::update() {
    f32 old = mValue;
    if (mFlicker == 1) {
        if (mFlickerWait != 0) {
            mFlickerWait--;
            mValue = 0.0f;
        }
        if (mFlickerWait == 0) {
            if (mFlickerIdx < ARRAY_SIZE(l_flickerTable)) {
                mValue = l_flickerTable[mFlickerIdx];
                mFlickerIdx++;
            } else {
                mFlicker = 0;
                mFlickerIdx = 0;
            }
        }
    } else if (mTarget != old) {
        sLib::chase(&mValue, mTarget, mSpeed);
    }
    return old != mValue;
}

// 80167F04
BOOL dSTR::light_c::set(BOOL on, BOOL fade, BOOL flicker, f32 speed) {
    mSpeed = speed;
    if (on) {
        if (0.0f == mTarget) {
            mTarget = 1.0f;
            if (!fade) {
                mValue = 1.0f;
                return FALSE;
            }
            if (flicker) {
                mFlicker = 1;
                mFlickerIdx = 0;
                mFlickerWait = 15;
            }
            return TRUE;
        }
    } else {
        mFlicker = 0;
        if (0.0f != mTarget) {
            mTarget = 0.0f;
            if (!fade) {
                mValue = 0.0f;
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80167FA4
f32 dSTR::calcLerp(f32 t, f32 a, f32 b) {
    return a + t * (b - a);
}

// The 0xD000 buildings (dItem::Item::isExtId), by index.
static const dBuildingInfo_c l_buildingInfo[BUILDING_NUM] = {
    {"obj_buildsite", "建設予定地", 0x0101, 0, 1, 0, 0, 1, 0x10, 0x28, 1, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_BUILD_SITE
    {"obj_myhome0", "プレイヤの家０", 0x00FB, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_PLAYER_HOUSE_0
    {"obj_myhome0", "プレイヤの家１", 0x00FB, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_PLAYER_HOUSE_1
    {"obj_myhome0", "プレイヤの家２", 0x00FB, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_PLAYER_HOUSE_2
    {"obj_myhome0", "プレイヤの家３", 0x00FB, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_PLAYER_HOUSE_3
    {"obj_post", "ポスト０", 0x00FC, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_MAILBOX_0
    {"obj_post", "ポスト１", 0x00FC, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_MAILBOX_1
    {"obj_post", "ポスト２", 0x00FC, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_MAILBOX_2
    {"obj_post", "ポスト３", 0x00FC, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_MAILBOX_3
    {"obj_myhome0", "ＮＰＣの家０", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_0
    {"obj_myhome0", "ＮＰＣの家１", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_1
    {"obj_myhome0", "ＮＰＣの家２", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_2
    {"obj_myhome0", "ＮＰＣの家３", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_3
    {"obj_myhome0", "ＮＰＣの家４", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_4
    {"obj_myhome0", "ＮＰＣの家５", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_5
    {"obj_myhome0", "ＮＰＣの家６", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_6
    {"obj_myhome0", "ＮＰＣの家７", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_7
    {"obj_myhome0", "ＮＰＣの家８", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_8
    {"obj_myhome0", "ＮＰＣの家９", 0x00FD, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_NPC_HOUSE_9
    {"obj_office", "役所", 0x00FE, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_TOWN_HALL
    {"obj_myhome0", "関所", 0x0102, 3, 0, 0, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_GATE
    {"obj_nc", "お店", 0x00FF, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_SHOP
    {"obj_tailor", "仕立て屋", 0x0103, 2, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_TAILOR
    {"obj_museum", "博物館", 0x0100, 3, 1, 0, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_MUSEUM
    {"obj_bbs", "掲示板", 0x0105, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_BULLETIN_BOARD
    {"obj_cdbbs", "カウントダウン", 0x0104, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_COUNTDOWN_BOARD
    {"obj_tent0", "魚テント", 0x0108, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_FISH_TENT
    {"obj_tent1", "虫テント", 0x0108, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_INSECT_TENT
    {"obj_lighthouse", "灯台", 0x0106, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_LIGHTHOUSE
    {"obj_windmill", "風車", 0x0107, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_WINDMILL
    {"obj_fountain", "噴水", 0x010A, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_FOUNTAIN
    {"obj_harvest0", "ハーベスト０", 0x0109, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_HARVEST_0
    {"obj_harvest1", "ハーベスト１", 0x0109, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_HARVEST_1
    {"ufo1", "UFO", 0x010B, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_UFO
    {"obj_tw_cut", "床屋", 0x0110, 2, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon5", "polygon6", "polygon7", "polygon8", "polygon9", "polygon10"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_BARBER
    {"obj_tw_HRA", "ハッピールーム", 0x0111, 3, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon5"}, {0x7D, 0x74, 0x50, 0x00}}, // BUILDING_CITY_HAPPY_ROOM
    {"obj_tw_tsunekichi", "ブローカー", 0x0112, 2, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_BROKER
    {"obj_tw_fortune", "ハッケミィ", 0x0113, 2, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon2", "polygon3"}, {0xFF, 0xFF, 0x40, 0x00}}, // BUILDING_CITY_FORTUNE
    {"obj_tw_grace", "グレース", 0x0114, 1, 0, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon5", "polygon6", "polygon7", "polygon8"}, {0xFF, 0x80, 0x34, 0x00}}, // BUILDING_CITY_GRACE
    {"obj_tw_auction", "オークション", 0x0117, 3, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon4", "polygon5", "polygon6"}, {0x7D, 0x74, 0x50, 0x00}}, // BUILDING_CITY_AUCTION
    {"obj_tw_theater", "劇場", 0x0118, 3, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon4", "polygon5", "polygon6"}, {0xFF, 0xA4, 0x00, 0x00}}, // BUILDING_CITY_THEATER
    {"obj_tw_ATM", "ATM", 0x010F, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2"}, {0xFF, 0xC0, 0x40, 0x00}}, // BUILDING_CITY_ATM
    {"obj_tw_shoeblack", "靴磨き", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_SHOE_SHINE
    {"obj_tw_etc00", "その他０", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon4"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_ETC_0
    {"obj_tw_etc01", "その他１", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_ETC_1
    {"obj_tw_etc02", "その他２", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_ETC_2
    {"obj_tw_etc03", "その他３", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_ETC_3
    {"obj_tw_etc04", "その他４", 0x010E, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_ETC_4
    {"obj_tw_reset", "リセットセンター入り口", 0x0115, 0, 1, 0, 0, 2, 0x00, 0x00, 0, {"polygon0", "polygon1", "polygon2", "polygon3", "polygon4", "polygon5"}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_RESET_CENTER
    {"obj_tw_fountain", "街の噴水", 0x0119, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {}, {0xFF, 0xC0, 0x20, 0x00}}, // BUILDING_CITY_FOUNTAIN
    {"obj_tw_lamp", "街灯Ａ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_A
    {"obj_tw_lamp", "街灯Ｂ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_B
    {"obj_tw_lamp", "街灯Ｃ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_C
    {"obj_tw_lamp", "街灯Ｄ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_D
    {"obj_tw_lamp", "街灯Ｅ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_E
    {"obj_tw_lamp", "街灯Ｆ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_F
    {"obj_tw_lamp", "街灯Ｇ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_G
    {"obj_tw_lamp", "街灯Ｈ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_H
    {"obj_tw_lamp", "街灯Ｉ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_I
    {"obj_tw_lamp", "街灯Ｊ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_J
    {"obj_tw_lamp", "街灯Ｋ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_K
    {"obj_tw_lamp", "街灯Ｌ", 0x0116, 0, 1, 1, 0, 2, 0x0B, 0x40, 0, {}, {0x80, 0x60, 0x20, 0x00}}, // BUILDING_CITY_LAMP_L
    {"obj_tw_organ", "自動オルガン", 0x011A, 0, 1, 1, 0, 2, 0x0F, 0x20, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_CITY_ORGAN
    {"obj_bus", "バス", 0x010C, 0, 1, 0, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_BUS
    {"obj_bus", "バス(街)", 0x010C, 0, 1, 0, 0, 2, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_CITY_BUS
    {"obj_busstop0", "バス停", 0x010D, 0, 1, 1, 0, 1, 0x10, 0x50, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_BUS_STOP
    {"obj_busstop1", "街のバス停", 0x010D, 0, 1, 1, 0, 2, 0x10, 0x50, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_CITY_BUS_STOP
    {"obj_tw_organ", "タンス", 0x011B, 0, 1, 1, 0, 2, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_CITY_DRESSER
    {"obj_carnival_bord", "カーニバル", 0x011C, 0, 1, 1, 0, 1, 0x00, 0x00, 0, {}, {0xFF, 0xFF, 0x80, 0x00}}, // BUILDING_CARNIVAL
};

// Out-of-range indices (805FBEC0).
static dBuildingInfo_c l_noBuildingInfo;

// 80167FB4
const dBuildingInfo_c *dSTR::getBuildingInfo(int idx) {
    const dBuildingInfo_c *info = &l_noBuildingInfo;
    if (idx < BUILDING_NUM) {
        info = &l_buildingInfo[idx];
    }
    return info;
}

// 80167FDC
int dSTR::entArea_c::getDirType() const {
    s16 angle = mAngle;
    switch ((u16)angle) {
    case 0:
        return 0;
    case 0x2000:
        return 2;
    case 0xE000:
        return 1;
    }
    return 0;
}

// 80168030
mVec3_c dSTR::entArea_c::calcFrontPos(const mVec3_c &base, int bg, f32 dist) {
    mVec3_c min;
    mVec3_c max;
    getBox(&min, &max, base, bg);
    mVec3_c pos = min + max;
    pos *= 0.5f;
    f32 dx = dist * nw4r::math::SinIdx((s16)mAngle);
    f32 dz = dist * nw4r::math::CosIdx((s16)mAngle);
    pos.x += dx;
    pos.z += dz;
    return pos;
}

// 80168128
mVec3_c dSTR::entArea_c::calcFrontPos(int unitX, int unitZ, int bg, f32 dist) {
    mVec3_c base;
    dFdBase_c::getUnitCenterPos(base, unitX, unitZ);
    dBGCF::setCurrentBg(bg);
    base.y = dBGCF::getUnitBaseY(&base);
    dBGCF::setCurrentBg(0);
    return calcFrontPos(base, bg, dist);
}

// 801681B8
mVec3_c dSTR::entArea_c::calcFrontPos(u16 id, int bg, f32 dist) {
    dSaveBuildingList_c *list = dSaveBuildingList_c::get();
    if (dItem::Item(id).isExtId()) {
        const dBuildingInfo_c *info = getBuildingInfo(dItem::Item(id).getExtIndex());
        dItem::Item item(id);
        int x;
        int z;
        if (list->getPos(&x, &z, item, info->mField)) {
            return calcFrontPos(x, z, bg, dist);
        }
    }
    return mVec3_c(0.0f, 0.0f, 0.0f);
}

// 801682B4
int dSTR::entArea_c::getBox(mVec3_c *min, mVec3_c *max, const mVec3_c &base, int bg) {
    f32 minX;
    f32 minZ;
    f32 maxX;
    f32 maxZ;

    minZ = base.z + mMin.z;
    maxZ = base.z + mMax.z;
    minX = base.x + mMin.x;
    maxX = base.x + mMax.x;
    mVec3_c a(minX, 0.0f, minZ);
    mVec3_c b(maxX, 0.0f, maxZ);
    mVec3_c center((a.x + b.x) * 0.5f, 0.0f, (a.z + b.z) * 0.5f);
    dBGCF::setCurrentBg(bg);
    f32 y = dBGCF::getUnitBaseY(&center);
    dBGCF::setCurrentBg(0);
    a.y = y;
    b.y = y;
    *min = a;
    *max = b;
    return mAngle << 16;
}

static dSTR::brresBank_c l_brresBank; // 805FBF10
static dSTR::colBank_c l_colBank;     // 805FBFF4
static dFdUnitAttr_c l_townUnitAttr(1); // 805FC068
static dFdUnitAttr_c l_cityUnitAttr(2); // 805FD8EC

// 801683C0
dSTR::brresBank_c *dSTR::getBrresBank() {
    return &l_brresBank;
}

// 801683CC
dSTR::colBank_c *dSTR::getColBank() {
    return &l_colBank;
}

// 801683D8
dFdUnitAttr_c *dSTR::getTownUnitAttr() {
    return &l_townUnitAttr;
}

// 801683E4
dFdUnitAttr_c *dSTR::getCityUnitAttr() {
    return &l_cityUnitAttr;
}

// 801683F0
const char *dSTR::getBuildingModel(int idx) {
    return getBuildingInfo(idx)->mModel;
}

// 80168414
const char *dSTR::getBuildingResName(int idx) {
    static const char *l_gateName[] = {"obj_check0", "obj_check1", "obj_check2"};
    static const char *l_playerHouseName[] = {"obj_myhome0", "obj_myhome1", "obj_myhome2", "obj_myhome3", "obj_myhome4"};
    static const char *l_shopName[] = {"obj_nc", "obj_ng", "obj_nw", "obj_nk"};
    static char l_npcHouseName[50];

    if (idx == BUILDING_GATE - BUILDING_BUILD_SITE) {
        u32 type = dSaveData_c::getRaw()->mBuilding.getGateType();
        return type < ARRAY_SIZE(l_gateName) ? l_gateName[type] : l_gateName[0];
    }
    if (dSaveBuildingList_c::isNpcHouse(idx)) {
        u32 no = dSaveBuildingList_c::getNpcHouseNo(idx);
        u32 type[2];
        getNpcHouseType(type, no);
        if (no < 10) {
            snprintf(l_npcHouseName, sizeof(l_npcHouseName), "TN_obj_house%d", type[0]);
            return l_npcHouseName;
        }
        snprintf(l_npcHouseName, sizeof(l_npcHouseName), "TN_obj_house%d", 0);
        return l_npcHouseName;
    }
    if (dSaveBuildingList_c::isPlayerHouse(idx)) {
        int no = dSaveBuildingList_c::getPlayerHouseNo(idx);
        const dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
        u32 size = homes->getHome(no)->mSize;
        return size < ARRAY_SIZE(l_playerHouseName) ? l_playerHouseName[size] : l_playerHouseName[0];
    }
    if (idx == BUILDING_SHOP - BUILDING_BUILD_SITE) {
        u32 stage = dSaveData_c::getTown()->mShops.mShop.mStage & 3;
        return stage < ARRAY_SIZE(l_shopName) ? l_shopName[stage] : l_shopName[0];
    }
    return getBuildingModel(idx);
}

// 801685A4
void dSTR::copyCol(int unitX, int unitZ, const char *name) {
    static copyChk_c l_copyChk;
    dBGCF::unitDat_c *col = getColBank()->getCol(name);
    if (col != NULL) {
        dBGCF::copyUnits(col, unitX - 7, unitZ - 7, &l_copyChk);
    }
}

// 80168628
void dSTR::flattenCol(int unitX, int unitZ, const char *name) {
    static flatChk_c l_flatChk;
    dBGCF::unitDat_c *col = getColBank()->getCol(name);
    if (col != NULL) {
        dBGCF::flattenUnits(col, unitX - 7, unitZ - 7, &l_flatChk);
    }
}

// 801686AC
int dSTR::getCurBuilding(int *unitX, int *unitZ) {
    if (unitX != NULL) {
        *unitX = 0;
    }
    if (unitZ != NULL) {
        *unitZ = 0;
    }
    u8 scene = dScene::getCurrentScene();
    if (!dScene::isSceneAttr(scene, 0x10) && !dScene::isSceneAttr(scene, 0x20) && !getSceneChange()->mUseDefaultPos) {
        if (getSceneChange()->mType != 0xB) {
            return -1;
        }
        getSceneChange();
        dScene::dSceneExit_c *exit = &dScene::gSceneBuildingExit;
        int x = exit->mBlockX;
        int z = exit->mBlockZ;
        BOOL city = dScene::isSceneAttr(scene, 9);
        dSaveBuildingList_c *list = dSaveBuildingList_c::get();
        const dItem::Item item = list->getAt(x, z, city ? BUILDING_FIELD_CITY : BUILDING_FIELD_TOWN);
        if (item.isExtId() && exit->mType == 0xB) {
            int idx = item.getExtIndex();
            if (unitX != NULL) {
                *unitX = x;
            }
            if (unitZ != NULL) {
                *unitZ = z;
            }
            return idx;
        }
    }
    return -1;
}

// A building and where it stands, packed (getCurBuildingCode).
static inline u32 makeBuildingCode(int x, int z, int idx) {
    return (x & 0xFF) | ((z & 0xFF) << 8) |
           (dSTR::getBuildingInfo(idx)->mProfile << 16);
}

// 80168824
u32 dSTR::getCurBuildingCode() {
    int x;
    int z;
    int idx = getCurBuilding(&x, &z);
    if (idx != -1) {
        return makeBuildingCode(x, z, idx);
    }
    return 0;
}

// 80168888
int dSTR::getCurBuildingType() {
    int idx = getCurBuilding(NULL, NULL);
    if (idx != -1) {
        if (idx == BUILDING_SHOP - BUILDING_BUILD_SITE) {
            return (dSaveData_c::getTown()->mShops.mShop.mStage & 3) == SHOP_STAGE_CRANNY ? BUILDING_TYPE_2 : BUILDING_TYPE_1;
        }
        return getBuildingInfo(idx)->mType;
    }
    return 0;
}

// 801688F0
u32 dSTR::countMembers(u8 mask) {
    u32 num = 0;
    for (int i = 0; i < 4; i++) {
        if ((mask >> i) & 1) {
            num++;
        }
    }
    return num;
}

// 80168948
int dSTR::getSlotIdx(const dItem::Item *item) {
    if (item->isExtId()) {
        return item->getExtIndex();
    }
    return 0;
}

// A structure's lock: the net members inside it (by building index).
struct lock_c {
    u8 mMembers : 4; // bit n: member n
    u8 mState : 4;
    u8 _1;
};

// NET_PACKET_STR_LOCK
struct lockPacket_c {
    u8 mState : 2;
    u8 mIdx : 6;
};

static lock_c l_lock[BUILDING_NUM];
static bool l_partyFlag;


// 801689A0
void dSTR::recvLock(const u8 *data, int member) {
    lockPacket_c packet = *(const lockPacket_c *)data;
    int state = packet.mState;
    int idx = packet.mIdx;
    switch (state) {
    case 1:
        l_lock[idx].mState = state;
        break;
    default:
        l_lock[idx].mMembers = l_lock[idx].mMembers | (1 << member);
        l_lock[idx].mState = 1;
        packet.mState = l_lock[idx].mState;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_STR_LOCK, member);
        break;
    }
}

// 80168A4C
void dSTR::recvUnlock(const u8 *data, int member) {
    for (int i = 0; i < BUILDING_NUM; i++) {
        l_lock[i].mMembers &= ~(1 << member);
    }
}

// 80168B90
void dSTR::resetLocks() {
    for (lock_c *lock = l_lock; lock < l_lock + BUILDING_NUM; lock++) {
        lock->mMembers = 0;
        lock->mState = 0;
        lock->_1 = 0xFF;
    }
    l_partyFlag = FALSE;
}

// 80168CAC
void dSTR::lock(const dItem::Item &item) {
    int idx = getSlotIdx(&item);
    if (dNet::isHost() || !dNet::isSessionActive()) {
        l_lock[idx].mMembers |= 1;
        l_lock[idx].mState = 1;
    } else {
        lockPacket_c packet;
        packet.mState = 0;
        l_lock[idx].mState = 0;
        packet.mIdx = idx;
        dNet::beginPacket();
        dNet::addPacket(&packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_STR_LOCK, 0);
    }
}

// 80168D60
int dSTR::getLockState(const dItem::Item &item) {
    if (item.isExtId()) {
        return l_lock[getSlotIdx(&item)].mState;
    }
    return 0;
}

// 80168DC0
void dSTR::unlock(const dItem::Item &item, int member) {
    if (dNet::isHost() || !dNet::isSessionActive() || item == dItem::ITEM_ID_NONE) {
        for (int i = 0; i < BUILDING_NUM; i++) {
            l_lock[i].mMembers &= ~(1 << member);
        }
    } else {
        u8 idx = getSlotIdx(&item);
        dNet::beginPacket();
        dNet::addPacket(&idx, sizeof(idx));
        dNet::sendPacket(NET_PACKET_STR_UNLOCK, 0);
    }
}

// 80168F84
BOOL dSTR::isPartyFlag() {
    return l_partyFlag;
}

// 80168F8C
void dSTR::setPartyFlag() {
    l_partyFlag = TRUE;
}

// 80168F98
void dSTR::clearPartyFlag() {
    l_partyFlag = FALSE;
}

// 80168FA4
BOOL dSTR::isAlone() {
    return countCurVisitors() <= 1;
}

// 80168FDC
u8 dSTR::getVisitors(u8 scene) {
    if (!dNet::isSessionActive() || dNet::isHost()) {
        int id = dScene::getSceneBuilding(scene);
        if (dItem::Item((u16)id).isExtId()) {
            dItem::Item item((u16)id);
            return l_lock[getSlotIdx(&item)].mMembers;
        }
    }
    return 0;
}

// 80169070
u32 dSTR::countVisitors(u8 scene) {
    u32 num = countMembers(getVisitors(scene));
    if (dScene::isSceneAttr(scene, 0x450) && isPartyFlag()) {
        return num + 1;
    }
    return num;
}

// 801690DC
u32 dSTR::countCurVisitors() {
    if (dScene::isCurrentSceneAttr(0x10)) {
        return countVisitors(dScene::getCurrentScene());
    }
    return 0;
}

// 8016911C
int dSTR::fn_8016911C() {
    return 0;
}

static int l_debugHouseType = -1;

// 80169124
int dSTR::getDebugHouseType() {
    return l_debugHouseType;
}

// 8016912C
BOOL dSTR::getNpcHouseType(u32 *out, u32 no) {
    static const int l_houseType[5][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}};
    out[0] = 0;
    out[1] = 0;
    u32 variant = dSaveData_c::getRaw()->mBuilding.mHouseVariant;
    variant = variant < 5 ? variant : 0;
    if (variant >= 5) {
        variant = 0;
    }
    if (l_debugHouseType == -1) {
        if (no < 10) {
            dAnimal_c *animal = dSaveData_c::getAnimalBlock()->getAnimalConst(no);
            if (animal != NULL) {
                out[0] = animal->getHouseVariant();
                int kind = (animal->getHouseType() / 3) & 1;
                out[1] = animal->getHouseType() % 3 + l_houseType[variant][kind] * 3;
                return TRUE;
            }
        }
    } else {
        out[0] = no % getNpcHouseVariantNum();
        out[1] = l_debugHouseType / 2;
        return TRUE;
    }
    return FALSE;
}

// 80169290
u32 dSTR::getNpcHouseVariantNum() {
    return 5;
}

// 80169298
u32 dSTR::getNpcHouseTypeNum() {
    return 6;
}

static dSTR::acList_c l_acList; // 805FF230

// 801692A0
dSTR::acList_c *dSTR::getAcList() {
    return &l_acList;
}

// 801692AC
void dSTR::acList_c::init() {
    for (int i = 0; i < ARRAY_SIZE(mList); i++) {
        mList[i] = NULL;
    }
}

// 801692E4
BOOL dSTR::acList_c::add(daSTR_c *ac) {
    for (int i = 0; i < ARRAY_SIZE(mList); i++) {
        if (ac == mList[i]) {
            return TRUE;
        }
    }
    for (int i = ARRAY_SIZE(mList) - 1; i >= 0; i--) {
        if (mList[i] == NULL) {
            mList[i] = ac;
            return TRUE;
        }
    }
    return FALSE;
}

// 801693E0
BOOL dSTR::acList_c::remove(daSTR_c *ac) {
    for (int i = 0; i < ARRAY_SIZE(mList); i++) {
        if (ac == mList[i]) {
            mList[i] = NULL;
            return TRUE;
        }
    }
    return FALSE;
}

// 80169424
daSTR_c *dSTR::acList_c::search(int unitX, int unitZ, BOOL any) {
    for (u32 i = 0; i < ARRAY_SIZE(mList); i++) {
        daSTR_c *ac = mList[i];
        if (ac != NULL) {
            alwaysAc_c *str = ac;
            if (unitX == str->getUnitX() && unitZ == str->getUnitZ() && (any || str->isActive())) {
                return ac;
            }
        }
    }
    return NULL;
}

// 801694F4
daSTR_c *dSTR::acList_c::searchBuilding(u16 id, BOOL any) {
    if (dItem::Item(id).isExtId()) {
        int idx = dItem::Item(id).getExtIndex();
        for (u32 i = 0; i < ARRAY_SIZE(mList); i++) {
            daSTR_c *ac = mList[i];
            if (ac != NULL) {
                alwaysAc_c *str = ac;
                if (idx == str->getBuildingIdx() && (any || str->isActive())) {
                    return ac;
                }
            }
        }
    }
    return NULL;
}

// 801695E8
daSTR_c *dSTR::acList_c::searchNearest(int unitX, int unitZ, BOOL any) {
    f32 minDist = 20480.0f;
    daSTR_c *nearest = NULL;
    mVec3_c center;
    dFdBase_c::getUnitCenterPos(center, unitX, unitZ);
    for (u32 i = 0; i < ARRAY_SIZE(mList); i++) {
        daSTR_c *ac = mList[i];
        if (ac != NULL) {
            alwaysAc_c *str = ac;
            if (str->getUnitZ() <= unitZ && (any || str->isActive())) {
                mVec3_c pos;
                dFdBase_c::getUnitCenterPos(pos, str->getUnitX(), str->getUnitZ());
                mVec3_c d = pos - center;
                f32 dist = d.x * d.x + d.z * d.z;
                if (dist < minDist) {
                    minDist = dist;
                    nearest = ac;
                }
            }
        }
    }
    return nearest;
}

// 80169720
daSTR_c *dSTR::acList_c::searchNearest(const mVec3_c &pos, BOOL any) {
    f32 x = pos.x;
    f32 z = pos.z;
    return searchNearest((int)x >> 5, (int)z >> 5, any);
}

// 8016976C
int dSTR::getTownRegion() {
    return getLanguageArea(getTownLanguage());
}

// 80169790
int dSTR::getTownLanguage() {
    return dSaveData_c::getTownLanguage();
}

// 801697BC
int dSTR::getCurRegion() {
    if (dScene::isCurrentSceneAttr(9)) {
        return 0;
    }
    return getTownRegion();
}

// 801697F4
int dSTR::getCurLanguage() {
    if (dScene::isCurrentSceneAttr(9)) {
        return 0;
    }
    return getTownLanguage();
}

// 8016982C
void dSTR::requestBus(busInfo_c *info, BOOL toTown, BOOL arg2, bool showDisp, BOOL arg4) {
    int type = 2;
    if (arg2) {
        type = 1;
    }
    info->mType = type;
    info->mShowDisp = showDisp;
    int x;
    int z;
    if (toTown) {
        dItem::Item item = BUILDING_BUS_STOP;
        dSaveBuildingList_c::get()->getPos(&x, &z, item, 1);
        mVec3_c pos;
        dBGCF::unitToPos(&pos, x, z);
        pos.x -= 32.0f;
        dBGCF::setCurrentBg(1);
        pos.y = dBGCF::getUnitBaseY(x, z);
        dBGCF::setCurrentBg(0);
        int wipeOut;
        int wipeIn = 0;
        wipeOut = 3;
        if (arg4) {
            wipeOut = 1;
            wipeIn = 1;
        }
        getSceneChange()->request(dScene::SCENE_FIELD, &pos, 8, 0, wipeIn, wipeOut);
    } else {
        dItem::Item item = BUILDING_CITY_BUS_STOP;
        dSaveBuildingList_c::get()->getPos(&x, &z, item, 2);
        mVec3_c pos;
        dBGCF::unitToPos(&pos, x, z);
        pos.x += 32.0f;
        pos.y = 28.0f;
        int wipeOut;
        dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
        BOOL first = FALSE;
        if (player != NULL && !player->isFlag0(0x70)) {
            first = TRUE;
        }
        int type;
        wipeOut = 4;
        if (first) {
            wipeOut = 0;
        }
        type = 8;
        if (first) {
            type = 0xAA;
        }
        getSceneChange()->request(0x2D, &pos, type, 0, 0, wipeOut);
    }
}


// 801699CC
dSTR::busInfo_c *dSTR::getBusInfo() {
    static busInfo_c s_busInfo;
    return &s_busInfo;
}

namespace dSTR {

// 801699F0: puts a building at a unit of the town hall's acre (16 units per acre).
static void setBuildingNearTownHall(u16 id, int unitX, int unitZ) {
    dFdBlock_c *block = dField::getInfo(FD_ID_TOWN)->findBlock(BLOCK_KIND_FLAG_TOWN_HALL);
    if (block != NULL) {
        int blockX = block->mBlockX;
        int blockZ = block->mBlockZ;
        dSaveBuildingList_c::get()->setPos((blockX << 4) + unitX, (blockZ << 4) + unitZ, id);
    }
}

// 80169A68: removes a building if it stands.
static void removeBuilding(u16 id) {
    int x, z;
    dItem::Item item(id);
    if (dSaveBuildingList_c::get()->getPos(&x, &z, item, 1)) {
        dSaveBuildingList_c::get()->remove(id);
    }
}

// 80169AC4
void putCountdownBoard() { setBuildingNearTownHall(BUILDING_COUNTDOWN_BOARD, 10, 9); }
// 80169ADC
void removeCountdownBoard() { removeBuilding(BUILDING_COUNTDOWN_BOARD); }
// 80169AEC
void putCarnival() { setBuildingNearTownHall(BUILDING_CARNIVAL, 10, 9); }
// 80169B04
void removeCarnival() { removeBuilding(BUILDING_CARNIVAL); }
// 80169B14
void putFishTent() { setBuildingNearTownHall(BUILDING_FISH_TENT, 10, 10); }
// 80169B2C
void removeFishTent() { removeBuilding(BUILDING_FISH_TENT); }
// 80169B3C
void putInsectTent() { setBuildingNearTownHall(BUILDING_INSECT_TENT, 10, 10); }
// 80169B54
void removeInsectTent() { removeBuilding(BUILDING_INSECT_TENT); }

// 80169B64
void putHarvest() {
    setBuildingNearTownHall(BUILDING_HARVEST_0, 6, 11);
    setBuildingNearTownHall(BUILDING_HARVEST_1, 10, 9);
}

// 80169BB4
void removeHarvest() {
    removeBuilding(BUILDING_HARVEST_0);
    removeBuilding(BUILDING_HARVEST_1);
}

// 80169BF4
void putUfo() {
    dItem::Item ufo = BUILDING_UFO;
    removeBuilding(ufo.mId);
    dSaveBuildingList_c::get()->setAtUfoSpot(ufo);
}

// 80169C38
void removeUfo() { removeBuilding(BUILDING_UFO); }

// Where a building stands (its Item a temporary at each call).
static inline BOOL getBuildingPos(int *x, int *z, const dItem::Item &id) {
    return dSaveBuildingList_c::get()->getPos(x, z, id, 1);
}

// 80169C48
void buildPublicWorks() {
    dSaveData_c::getTown()->mMainField.buildBridge();
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUILD_WINDMILL)) {
        int x, z;
        if (getBuildingPos(&x, &z, BUILDING_LIGHTHOUSE)) {
            dSaveBuildingList_c::get()->remove(BUILDING_LIGHTHOUSE);
            dSaveBuildingList_c::get()->setPos(x, z, BUILDING_WINDMILL);
            dSaveData_c::getTown()->mVillagerMove.setWindmillBuilt();
        } else {
            if (!getBuildingPos(&x, &z, BUILDING_WINDMILL)) {
                dSaveBuildingList_c::get()->setAtLighthouseSpot(BUILDING_WINDMILL);
                dSaveData_c::getTown()->mVillagerMove.setWindmillBuilt();
            }
        }
        dSaveData_c::getTown()->clearPublicWorksBells();
        dSaveData_c::getTown()->setNewConstruction(NEW_CONSTRUCTION_WINDMILL, 0, 0);
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_BUILD_WINDMILL);
    }
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUILD_LIGHTHOUSE)) {
        int x, z;
        if (getBuildingPos(&x, &z, BUILDING_WINDMILL)) {
            dSaveBuildingList_c::get()->remove(BUILDING_WINDMILL);
            dSaveBuildingList_c::get()->setPos(x, z, BUILDING_LIGHTHOUSE);
            dSaveData_c::getTown()->mVillagerMove.setLighthouseBuilt();
        } else {
            if (!getBuildingPos(&x, &z, BUILDING_LIGHTHOUSE)) {
                dSaveBuildingList_c::get()->setAtLighthouseSpot(BUILDING_LIGHTHOUSE);
                dSaveData_c::getTown()->mVillagerMove.setLighthouseBuilt();
            }
        }
        dSaveData_c::getTown()->clearPublicWorksBells();
        dSaveData_c::getTown()->setNewConstruction(NEW_CONSTRUCTION_LIGHTHOUSE, 0, 0);
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_BUILD_LIGHTHOUSE);
    }
    if (dSaveData_c::getRaw()->isFlag(SAVE_FLAG_BUILD_FOUNTAIN)) {
        dSaveBuildingList_c::get()->setFountain();
        dSaveData_c::getTown()->clearPublicWorksBells();
        dSaveData_c::getTown()->setNewConstruction(NEW_CONSTRUCTION_FOUNTAIN, 0, 0);
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_BUILD_FOUNTAIN);
        dSaveData_c::getTown()->mVillagerMove.setFountainBuilt();
    }
}

// 80169ED8
void updateResetOpen() {
    if (cM::rndInt(10) == 6) {
        dSaveData_c::getTown()->setFlag(SAVE_FLAG_RESET_OPEN);
    } else {
        dSaveData_c::getTown()->clearFlag(SAVE_FLAG_RESET_OPEN);
    }
}

// 80169F20
BOOL requestWindmill() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_BUILD_WINDMILL);
    return TRUE;
}

// 80169F4C
BOOL requestLighthouse() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_BUILD_LIGHTHOUSE);
    return TRUE;
}

// 80169F78
BOOL requestFountain() {
    dSaveData_c::getTown()->setFlag(SAVE_FLAG_BUILD_FOUNTAIN);
    return TRUE;
}

// 80169FA4
BOOL getEntranceSidePos(mVec3_c *out, const dItem::Item &item, int side) {
    if (item.mId == BUILDING_GATE) {
        return FALSE;
    }
    if (item.isExtId()) {
        dBuildingInfo_c info = *getBuildingInfo(item.getExtIndex());
        if (info.mField & BUILDING_FIELD_TOWN) {
            const dSTR::colArea_c *box = dSTR::getColBank()->getColArea(item.mId);
            if (box != NULL) {
                mVec3_c ofs = box->getCenter();
                f32 dx = 0.5f * std::fabs(box->mMax.x - box->mMin.x) - 16.0f;
                f32 dz = 16.0f + 0.5f * std::fabs(box->mMax.z - box->mMin.z);
                if (side == 0) {
                    ofs.x -= dx;
                    ofs.z -= dz;
                } else {
                    ofs.x += dx;
                    ofs.z -= dz;
                }
                ofs.y = 0.0f;
                int x, z;
                dItem::Item id = item;
                if (dSaveBuildingList_c::get()->getPos(&x, &z, id, 1)) {
                    mVec3_c pos;
                    dBGCF::unitToPos(&pos, x, z);
                    dBGCF::setCurrentBg(1);
                    pos.y = dBGCF::getUnitY(&pos);
                    dBGCF::setCurrentBg(0);
                    *out = pos + ofs;
                    return TRUE;
                }
            }
            return FALSE;
        }
        return FALSE;
    }
    return FALSE;
}

static u32 l_count; // 8074E878
static u8 l_flagA;  // 8074E87C
static u8 l_flagB;  // 8074E87D

// 8016A208
void resetCount() {
    clearCount();
    clearFlagA();
    clearFlagB();
}

// 8016A230
void clearCount() { l_count = 0; }
// 8016A23C
u32 getCount() { return l_count; }
// 8016A244
void incCount() {
    if (l_count < 0x100) {
        l_count++;
    }
}
// 8016A25C
void clearFlagA() { setFlagA(FALSE); }
// 8016A264 (the target's symbols merge it into clearFlagA, which tail-calls it)
void setFlagA(u8 flag) { l_flagA = flag; }
// 8016A26C
BOOL getFlagA() { return l_flagA; }
// 8016A274
void clearFlagB() { setFlagB(FALSE); }
// 8016A27C (merged into clearFlagB likewise)
void setFlagB(u8 flag) { l_flagB = flag; }
// 8016A284
BOOL getFlagB() { return l_flagB; }

// 8016A28C
mVec3_c *getCityReturnPos() {
    static mVec3_c s_pos(768.0f, dBG::cGroundY, 448.0f);
    return &s_pos;
}

// 8016A2F8
BOOL isAtFountainFront(const mVec3_c &pos, const s16 &angle, mVec3_c *front, mVec3_c *base, BOOL draw) {
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        int x, z;
        if (getBuildingPos(&x, &z, BUILDING_FOUNTAIN)) {
            mVec3_c unitPos;
            dBGCF::unitToPos(&unitPos, x, z);
            unitPos.y = dBGCF::getUnitY(&unitPos);
            mVec3_c center(16.0f + unitPos.x, unitPos.y, 64.0f + unitPos.z);
            if (front != NULL) {
                *front = center;
                front->z += 32.0f;
            }
            mVec3_c ground = unitPos;
            ground.x += 16.0f;
            ground.z += 16.0f;
            ground.y = dBGCF::getUnitY(&ground);
            if (base != NULL) {
                *base = ground;
            }
            f32 range = getHostIO()->_44;
            if (center.xzDistTo(pos) < range && sLib::distanceAngle(-0x8000, angle) < 0x1800) {
                if (draw) {
                    dSceneCheck::drawCylinder(center, mColor(0xFF, 0x64, 0x64, 0x80), 1, mAng3_c(0, 0, 0), 0, range, 16.0f);
                }
                return TRUE;
            }
            if (draw) {
                dSceneCheck::drawCylinder(center, mColor(0xFF, 0xFF, 0x64, 0x80), 1, mAng3_c(0, 0, 0), 0, range, 16.0f);
            }
        }
    }
    return FALSE;
}

// 8016A548
void playRoomSe() {
    int se = -1;
    switch (dScene::getCurrentScene()) {
    case dScene::SCENE_RM_HS0_F1:
    case dScene::SCENE_RM_HS1_F1:
    case dScene::SCENE_RM_HS2_F1:
    case dScene::SCENE_RM_HS3_F1:
        se = 0;
        break;
    case dScene::SCENE_RM_SHOP0:
        se = 1;
        break;
    case dScene::SCENE_RM_OFFICE:
        se = 2;
        break;
    case dScene::SCENE_RM_TAILOR:
        se = 3;
        break;
    }
    if (se != -1) {
        static mVec3_c s_pos(256.0f, 0.0f, 480.0f);
        playSeAt(&s_pos, se);
    }
}

// 8016A614
BOOL getBusStopBox(mVec3_c *min, mVec3_c *max) {
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD) {
        int x, z;
        if (getBuildingPos(&x, &z, BUILDING_BUS_STOP)) {
            mVec3_c pos;
            dBGCF::unitToPos(&pos, x, z);
            mVec3_c lo = pos;
            lo.x -= 256.0f;
            lo.y -= 32.0f;
            lo.z -= 96.0f;
            mVec3_c hi = pos;
            hi.x += 160.0f;
            hi.y += 32.0f;
            hi.z -= 16.0f;
            *min = lo;
            *max = hi;
            return TRUE;
        }
    }
    return FALSE;
}

area_c gArea;

// 8016A704
void area_c::clear() {
    mMin.set(0.0f, 0.0f, 0.0f);
    mMax.set(0.0f, 0.0f, 0.0f);
}

// 8016A724
BOOL area_c::isSet() const {
    return mMin != mVec3_c::Zero || mMax != mVec3_c::Zero;
}

// 8016A790
void area_c::set(const mVec3_c &min, const mVec3_c &max) {
    mMin = min;
    mMax = max;
}

// 8016A7C4
BOOL area_c::isInside(const mVec3_c &pos) const {
    if (isSet() && pos.x >= mMin.x && pos.x <= mMax.x && pos.y >= mMin.y && pos.y <= mMax.y && pos.z >= mMin.z &&
        pos.z <= mMax.z) {
        return TRUE;
    }
    return FALSE;
}

// 8016A87C
BOOL isForceClosed() { return FALSE; }

// 8016A884
int getCurBuildingKind(u8 *other) {
    u8 dummy = FALSE;
    if (other == NULL) {
        other = &dummy;
    }
    *other = FALSE;
    dItem::Item building(dScene::getSceneBuilding(dScene::getCurrentScene()));
    if (building.isExtId()) {
        int idx = building.getExtIndex();
        const dBuildingInfo_c *info = dSTR::getBuildingInfo(idx);
        if (idx == (BUILDING_SHOP & 0xFFF)) {
            if ((dSaveData_c::getTown()->mShops.mShop.mStage & 3) == 0) {
                return BUILDING_TYPE_2;
            }
            return BUILDING_TYPE_1;
        }
        if (dSaveBuildingList_c::isPlayerHouse(idx)) {
            int no = dSaveBuildingList_c::getPlayerHouseNo(idx);
            if (no != dSaveData_c::getRaw()->mHomes.findCurrentPlayer()) {
                *other = TRUE;
            }
            return BUILDING_TYPE_2;
        }
        if (dSaveBuildingList_c::isNpcHouse(idx)) {
            *other = TRUE;
            return BUILDING_TYPE_2;
        }
        return info->mType;
    }
    return BUILDING_TYPE_0;
}

} // namespace dSTR

// 8016A9D8
BOOL flatChk_c::check(int attr) {
    return (attr == BG_ATTR_STR_COL || attr == BG_ATTR_STR_ENT_TO_RU ||
            attr == BG_ATTR_STR_ENT_TO_LU)
               ? FLATTEN
               : NO_FLATTEN;
}

// 8016AA0C
void copyChk_c::check(int attr, u8 *copyTri, u8 *copyAttr, u8 *copyRoute) {
    *copyTri = FALSE;
    *copyAttr = FALSE;
    *copyRoute = FALSE;
    if (attr == BG_ATTR_STR_COL || attr == BG_ATTR_STR_ENT_TO_RU || attr == BG_ATTR_STR_ENT_TO_LU) {
        *copyTri = TRUE;
    }
}

// 8016AA3C
dSTR::brresBank_c::~brresBank_c() {}

// 8016AABC
dSTR::colBank_c::~colBank_c() {}
