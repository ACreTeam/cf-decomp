#include <game/game/d_field.hpp>
#include <game/game/d_field_process.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_str.hpp>
#include <string.h>

dFdBase_c *dField::sInfo[14];
bool dField::sSceneReady;

// 80190BE4
fBase_c *dField_c::construct() { return new dField_c; }

// 80190C30
void dField::setInfo(dFdBase_c *field, int idx) { dField::sInfo[idx] = field; }

// 80190C44
dFdBase_c *dField::getInfo(int idx) { return dField::sInfo[idx]; }

// 80190C58
int dField::getSceneFieldId(u8 scene) {
    if (dScene::isSceneAttr(scene, 5)) {
        return scene != dScene::SCENE_CHECK_FIELD;
    }
    if (dScene::isSceneAttr(scene, 0x450)) {
        int room = -1;
        u32 home = dHomeList_c::getHomeFromScene(&room, scene);
        return dField::getHomeFieldId(home, room);
    }
    return 0;
}

// 80190CDC
int dField::getHomeFieldId(u32 home, int room) {
    if (home < 4 && dHome_c::isValidRoom(room)) {
        return home * 3 + room + 2;
    }
    return 0;
}

// 80190D40
int dField_c::create() {
    u8 scene = dScene::getCurrentScene();
    int idx = dField::getSceneFieldId(scene);
    dFdBase_c *field = dField::getInfo(0);
    if (idx == 1) {
        field = dField::getInfo(idx);
        if (field) static_cast<dFdInfoSv_c *>(field)->updateTown();
    } else if (idx >= 2 && idx < 14) {
        field = dField::getInfo(idx);
    } else if (dScene::isSceneAttr(scene, 0x850)) {
        dNpcMng_c::applyHouseVillagerItems();
        EGG::Heap *heap = EGG::Heap::getCurrentHeap();
        if (heap) field = dFdInfoNpcHs_c::create(dScene::getSceneAttrIndex(scene, 0x850), heap);
    } else if (scene == dScene::SCENE_RM_HAPPY_MDL) {
        EGG::Heap *heap = EGG::Heap::getCurrentHeap();
        if (heap) field = dFdInfoSvMdlRm_c::createWithBg0(&dSaveData_c::getTown()->mModelRoom, heap);
    } else {
        EGG::Heap *heap = EGG::Heap::getCurrentHeap();
        dFieldSceneData_c *data = reinterpret_cast<dFieldSceneData_c *>(
            dScene::getSceneData(dScene::getCurrentScene()));
        if (data && data->mField) {
            dFieldSceneMap_c *map = data->mField;
            dFdInfo_c *info = NULL;
            if (!field) {
                info = static_cast<dFdInfo_c *>(heap->alloc(sizeof(dFdInfo_c), 4));
                new (info) dFdInfo_c;
            }
            if (info) {
                const dFdBlockId_c *ids = map->mBlockIds;
                int width = map->mBlockW;
                int height = map->mBlockH;
                dFdBlockId_c block;
                if (width == 1 && height == 1) {
                    memcpy(&block, ids, sizeof(block));
                    if (dScene::isSceneAttr(scene, 0x450)) {
                        int room = -1;
                        u32 home = dHomeList_c::getHomeFromScene(&room, scene);
                        if (home < 4 && room == 3) {
                            dHome_c *house = dSaveData_c::getTown()->mHomes.getHome(home);
                            if (house) {
                                block.mId = house->getExteriorId();
                                ids = &block;
                            }
                        }
                    }
                }
                dFdUnitAttr_c *attr = dScene::isSceneAttr(dScene::getCurrentScene(), 9)
                    ? dSTR::getCityUnitAttr() : NULL;
                if (!info->create(ids, width, height, heap, attr)) {
                    info->release(heap);
                    heap->free(info);
                    info = NULL;
                }
                field = info;
            }
        }
    }
    if (field) field->registerBg();
    dField::setInfo(field, 0);
    dField::sSceneReady = true;
    return SUCCEEDED;
}

// 8019102C
int dField_c::doDelete() {
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    dFdBase_c *field = dField::getInfo(0);
    if (field) {
        field->release(heap);
        switch (field->getKind()) {
        case 0: heap->free(field); break;
        case 3: {
            dFdInfoNpcHs_c *info = static_cast<dFdInfoNpcHs_c *>(field);
            dFdInfoNpcHs_c::remove(&info, heap);
            break;
        }
        case 4: {
            dFdInfoSvMdlRm_c *info = static_cast<dFdInfoSvMdlRm_c *>(field);
            dFdInfoSvMdlRm_c::remove(&info, heap);
            break;
        }
        }
        dBGCF::releaseBg(0);
        dField::setInfo(NULL, 0);
    }
    dField::sSceneReady = false;
    return SUCCEEDED;
}

// 8019111C
int dField_c::execute() { return SUCCEEDED; }
// 80191124
int dField_c::draw() { return SUCCEEDED; }

// 8019112C
u32 dField::getHeapSize() {
    u32 town = dFdInfo_c::getHeapSize();
    u32 room = dFdInfo_c::getRoomHeapSize();
    u32 bg = dFdBase_c::getBgDataSize();
    return room * 12 + (bg + town);
}

// 80191178
dFdBase_c *dField::getHomeInfo(u32 home, int room) {
    int idx = dField::getHomeFieldId(home, room);
    return idx ? dField::getInfo(idx) : NULL;
}

// 801911AC
void dField::reloadHomes() { for (int home = 0; home < 4; ++home) dField::reloadHome(home); }

// 801911E8
void dField::reloadHome(int home) {
    for (int room = 0; room < 3; ++room) {
        dFdBase_c *field = dField::getHomeInfo(home, room);
        if (field) field->reloadHomeRoom(home, room);
    }
}

// 80191248
BOOL dField::createSaveFields() {
    // Keep this local order: MWCC reuses the town register for the house index base.
    EGG::Heap *heap;
    int first;
    dFdBase_c *town;
    dFdBase_c *field;
    int home;
    int room;
    int idx;
    heap = dHeap::fieldHeap_p;
    town = dField::getInfo(1);
    if (!town) {
        town = static_cast<dFdInfoSv_c *>(heap->alloc(sizeof(dFdInfoSv_c), 4));
        new (town) dFdInfoSv_c;
        dField::setInfo(town, 1);
    }
    if (town && !static_cast<dFdInfoSv_c *>(town)->createTown(heap)) return FALSE;
    for (home = 0, first = 2; home < 4; ++home, first += 3) {
        for (room = 0; room < 3; ++room) {
            idx = room + first;
            field = dField::getInfo(idx);
            if (!field) {
                field = static_cast<dFdInfoSvHs_c *>(heap->alloc(sizeof(dFdInfoSvHs_c), 4));
                new (field) dFdInfoSvHs_c;
                dField::setInfo(field, idx);
            }
            if (field && !field->buildHomeRoom(home, room, heap)) return FALSE;
        }
    }
    return TRUE;
}

// 80191414
int dFdInfoSvHs_c::getKind() { return 2; }
// 8019141C
dFdInfoSvHs_c::~dFdInfoSvHs_c() {}
// 8019145C
int dFdInfoSv_c::getKind() { return 1; }
// 80191464
dFdInfoSv_c::~dFdInfoSv_c() {}
// 801914A4
dField_c::~dField_c() {}
