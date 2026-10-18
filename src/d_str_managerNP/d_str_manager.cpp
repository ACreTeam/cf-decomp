// The structure manager (REL d_str_managerNP). See
// include/game/game/d_str_manager.hpp. REL .text 0xCC..0xCB8, .ctors 0x0..0x4,
// .rodata 0x0..0x10, .data 0x0..0xC0, .bss 0x8..0x50.
#include <game/game/d_str_manager.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_fltg.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <revolution/MEM.h>

// Where the street organ stands in the city on visit days (unit coordinates).
#define CITY_ORGAN_UNIT_X 19
#define CITY_ORGAN_UNIT_Z 15

// The unit of the ATM machine, in front of the city ATM building.
#define CITY_ATM_UNIT_X 12
#define CITY_ATM_UNIT_Z 13

// dFdBase_c::wearGrass: wears the grass of a unit away completely.
#define GRASS_WEAR_ALL 0xFF

class hostIO_c {
  public:
    hostIO_c() {
        _0C = 0;
        _0D = 0;
        _0E = 0;
        _0F = 0;
        _10 = 0;
        _16 = -1;
        _18 = -1;
        _11 = 0;
        _12 = 0;
        mOffsetY = 0.0f;
        mScale = 1.0f;
        _13 = 1;
        _14 = 0;
    }
    virtual ~hostIO_c() {
    }

    /* 0x04 */ f32 mOffsetY;
    /* 0x08 */ f32 mScale;
    /* 0x0C */ u8 _0C;
    /* 0x0D */ u8 _0D;
    /* 0x0E */ u8 _0E;
    /* 0x0F */ u8 _0F;
    /* 0x10 */ u8 _10;
    /* 0x11 */ u8 _11;
    /* 0x12 */ u8 _12;
    /* 0x13 */ u8 _13;
    /* 0x14 */ u8 _14;
    /* 0x16 */ s16 _16;
    /* 0x18 */ s16 _18;
}; // size 0x1C

// 0xCC
namespace dStrManager {
BASE_PROFILE(STR_MANAGER, dStrm_c)
}

u16 dStrm_c::sBuildSiteNo;
u8 dStrm_c::sDoorEntered;
u8 dStrm_c::sToyDay;
static hostIO_c l_hostIO;

// 0x120
void dStrm_c::createBuildings() {
    u8 scene = dScene::getCurrentScene();
    if (scene == dScene::SCENE_CHECK_FIELD) {
        return;
    }
    int field = dScene::isSceneAttr(scene, dScene::SCENE_ATTR_CITY_FIELD)
                    ? BUILDING_FIELD_CITY
                    : BUILDING_FIELD_TOWN;
    u16 id;
    BOOL placed;
    dSaveBuildingList_c *list = dSaveBuildingList_c::get();
    for (int i = 0; i < ARRAY_SIZE(list->mPos); i++) {
        id = (u32)i < BUILDING_NUM ? BUILDING_BUILD_SITE + i
                                   : BUILDING_BUILD_SITE;
        dItem::Item item(id);
        int x, z;
        placed = FALSE;
        if (!item.getExtFlag()) {
            dItem::Item copy(id);
            if (list->getPos(&x, &z, copy, field)) {
                placed = TRUE;
            }
        }
        if (placed) {
            mVec3_c pos;
            dFdBase_c::getUnitCenterPos(&pos, x, z);
            pos.y = dBGCF::getUnitBaseY(x, z);
            dActor_c *actor = dActor_c::construct(
                dSTR::getBuildingInfo(i)->mProfile, this, i, &pos, NULL);
            if (i == BUILDING_INDEX(BUILDING_CITY_ATM)) {
                mAng3_c atmAngle(0, DEG_TO_ANGLE(45), 0);
                mVec3_c atmPos = pos;
                atmPos.x = CITY_ATM_UNIT_X * mFI_UT_WORLDSIZE_X_F +
                           mFI_UT_WORLDSIZE_HALF_X_F;
                atmPos.z = CITY_ATM_UNIT_Z * mFI_UT_WORLDSIZE_Z_F +
                           mFI_UT_WORLDSIZE_HALF_Z_F;
                dActor_c::construct(fProf::A_RMOBJ_ATM, actor, 0, &atmPos,
                                    &atmAngle);
            }
        }
    }
}

// 0x2A8
void dStrm_c::createBuildSites() {
    dFdBase_c *field = getCurrentField();
    if (field == NULL) {
        return;
    }
    dFdUnitAttr_c *attr = field->_24;
    if (dScene::getCurrentScene() == dScene::SCENE_FIELD ||
        dScene::getCurrentScene() == dScene::SCENE_DM_TITLE) {
        dSaveBuildingList_c *list = dSaveBuildingList_c::get();
        for (u32 i = 0; i < BUILD_SITE_NUM; i++) {
            int x, z;
            if (list->getBuildSite(&x, &z, i)) {
                if (attr == NULL || attr->getAttr(x, z) == 0) {
                    mVec3_c pos;
                    dFdBase_c::getUnitCenterPos(&pos, x, z);
                    pos.y = dBGCF::getUnitBaseY(x, z);
                    dActor_c::construct(dSTR::getBuildingInfo(
                                            BUILDING_INDEX(BUILDING_BUILD_SITE))
                                            ->mProfile,
                                        this, 0, &pos, NULL);
                }
                sBuildSiteNo++;
            }
        }
    }
}

// 0x3AC
void dStrm_c::setupHeap() {
    dSTR::brresBank_c *bank = dSTR::getBrresBank();
    BOOL isTown = !dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD);
    int region = dSTR::getTownRegion();
    int lang = dSTR::getTownLanguage();
    if (!bank->isSame(isTown, dScene::getSeasonType(), region, lang)) {
        bank->unload();
        dHeap::strResHeap_p->free(MEM_FRM_HEAP_FREE_ALL);
        bank->clear();
    }
    dHeap::strHeap_p->free(MEM_FRM_HEAP_FREE_ALL);
    mAllocator.attach(dHeap::strHeap_p, 0x20);
}

// 0x46C
void dStrm_c::init() {
    if (!mCreated) {
        dSTR::getAcList()->init();
        sInstance = this;
        dHeap::createStrHeap(dHeap::localHeap_p);
        setupHeap();
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD) &&
            dNpcMng_c::isWeekVisitDay()) {
            dSaveBuildingList_c::get()->setPos(
                CITY_ORGAN_UNIT_X, CITY_ORGAN_UNIT_Z,
                dItem::Item((u16)BUILDING_CITY_ORGAN));
            mOrganPlaced = 1;
        }
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
            int x, z;
            for (u32 i = 0; i < ANIMAL_NUM; i++) {
                if (dSaveBuildingList_c::get()->getNpcHousePos(&x, &z, i)) {
                    dSTR::getTownUnitAttr()->setAttr(
                        x - 1, z + 2,
                        dFdUnitAttr_c::STR_DEL | dFdUnitAttr_c::STR_INS);
                    dSTR::getTownUnitAttr()->setAttr(
                        x - 2, z + 2,
                        dFdUnitAttr_c::STR_DEL | dFdUnitAttr_c::STR_INS);
                }
            }
            if (dSaveBuildingList_c::get()->getPos(
                    &x, &z, dItem::Item((u16)BUILDING_TOWN_HALL),
                    BUILDING_FIELD_TOWN)) {
                dSTR::getTownUnitAttr()->setAttr(x - 1, z + 2,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x - 2, z + 2,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x - 3, z + 1,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x + 1, z + 2,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x + 2, z + 2,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x + 3, z + 1,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
            }
            if (dSaveBuildingList_c::get()->getPos(
                    &x, &z, dItem::Item((u16)BUILDING_GATE),
                    BUILDING_FIELD_TOWN)) {
                dSTR::getTownUnitAttr()->setAttr(x - 2, z + 1,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
                dSTR::getTownUnitAttr()->setAttr(x + 3, z + 1,
                                                 dFdUnitAttr_c::STR_DEL |
                                                     dFdUnitAttr_c::STR_INS);
            }
        }
        clearStrUnits();
    }
    mCreated = 1;
}

// 0x6D4
void dStrm_c::createActors() {
    sCommonBrres =
        nw4r::g3d::ResFile(dSTR::getBrresBank()->getBrres("obj_str_common"));
    dFltg::clear();
    createBalloonMng();
    createBuildings();
    createBuildSites();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    if (!(player != NULL ? player->isFlag0(PRIVATE_FLAG0_PART_TIME_JOB)
                         : FALSE) &&
        dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        dActor_c::construct(fProf::A_UFO, this, 0, NULL, NULL);
    }
    clearStrUnits();
}

// 0x784
void dStrm_c::createBalloonMng() {
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
        dBase_c::createBase(fProf::BALLOON_MNG, this, 0, fBase_c::OTHER);
    }
}

// 0x7D0
void dStrm_c::clearStrUnits() {
    static dItem::Item none;
    dFdBase_c *field = getCurrentField();
    dFdUnitAttr_c *attr = field != NULL ? field->_24 : NULL;
    int id;
    int x;
    if (attr == NULL) {
        return;
    }
    // Every unit but the border blocks.
    int endX = (field->getBlockW() - 1) * UT_X_NUM;
    int endZ = (field->getBlockH() - 1) * UT_Z_NUM;
    for (x = UT_X_NUM; x < endX; x++) {
        for (int z = UT_Z_NUM; z < endZ; z++) {
            u32 unitAttr = attr->getAttr(x, z);
            if ((unitAttr & dFdUnitAttr_c::STR_COL) ||
                (unitAttr & dFdUnitAttr_c::STR_ENT)) {
                field->wearGrass(x, z, GRASS_WEAR_ALL, 0);
                dItem::Item *item = field->getItem(x, z, 0);
                if (item != NULL) {
                    id = item->getId();
                    if (dItem::isRealItemId(id)) {
                        dSaveData_c::addToPoliceBox(id);
                    }
                    if (item->getId() != dItem::ITEM_ID_STR_UNIT) {
                        field->setItem(&none, x, z, 0);
                    }
                }
            } else if (unitAttr & dFdUnitAttr_c::STR_DEL) {
                f32 radius, height;
                int colAttr;
                if (dBGCF::getColumnAttr(x, z, &radius, &height, &colAttr) &&
                    colAttr != BG_ATTR_ATTRW) {
                    field->setItem(&none, x, z, 0);
                }
            }
        }
    }
}

dStrm_c *dStrm_c::sInstance;
nw4r::g3d::ResFile dStrm_c::sCommonBrres;

// 0x9A8
int dStrm_c::create() {
    static dQuestEvent_e l_toyDay = EVENT_TOY_DAY;
    init();
    if (dSTR::getBrresBank()->load(dHeap::strResHeap_p)) {
        createActors();
        mLoaded = 1;
        sToyDay = dEvent::isActive(l_toyDay) != FALSE;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0xA2C
int dStrm_c::execute() {
    return SUCCEEDED;
}

// 0xA34
int dStrm_c::draw() {
    return SUCCEEDED;
}

// 0xA3C
int dStrm_c::doDelete() {
    sInstance = NULL;
    if (dSTR::getBrresBank()->isLoaded()) {
        dHeap::destroyStrHeap();
        if (mOrganPlaced) {
            dSaveBuildingList_c::get()->remove(
                dItem::Item((u16)BUILDING_CITY_ORGAN));
        }
        sDoorEntered = 0;
        return SUCCEEDED;
    }
    return NOT_READY;
}

// 0xAC0
BOOL dStrm_c::isDrawDebug() {
    return FALSE;
}

// 0xAC8
BOOL dStrm_c::isToyDay() {
    return sToyDay;
}

// 0xAD4
BOOL dStrm_c::isUseHostIO() {
    return TRUE;
}

// 0xADC
f32 dStrm_c::getOffsetY() {
    return l_hostIO.mOffsetY;
}

// 0xAEC
f32 dStrm_c::getScale() {
    return l_hostIO.mScale;
}

// 0xAFC
BOOL dStrm_c::isEntryMdl() {
    return TRUE;
}
