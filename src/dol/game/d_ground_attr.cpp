// d_ground_attr: the ground under a position (dGroundAttr). .text 800A858C..800A88CC, .rodata 8046F268..8046F378.
// See include/game/game/d_ground_attr.hpp.
#include <game/game/d_ground_attr.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_bg_debug.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_scene.hpp>

namespace dGroundAttr {

// 8046F268: the footstep sound of a room's wall attribute, by scene.
static const int l_roomWallSe[dScene::SCENE_NUM] = {
    GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_CARPET,  GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET, GROUND_SE_ROOM_CARPET,
    GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_FLOOR,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_FLOOR,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_DEFAULT,
    GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_FLOOR,
    GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_WOOD,   GROUND_SE_ROOM_WOOD,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_WOOD,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_FLOOR,
    GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,   GROUND_SE_ROOM_FLOOR,  GROUND_SE_ROOM_DEFAULT,
    GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT, GROUND_SE_ROOM_DEFAULT,
};

// 800A858C
int getAttr(const mVec3_c *pos) {
    int attr = dBGCF::getGroundAttr(pos, 0);
    if (attr == BG_ATTR_WAVE) {
        dBGCF::groundChk_c chk(pos, 0, 0, 0);
        attr = chk.mAttr;
    }
    if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM) && attr == BG_ATTR_CHANGE) {
        dItem::Item carpet = dBgUtil::getCarpetRaw();
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(carpet);
        if (bitm != NULL) {
            int ret = BG_ATTR_NONE;
            int carpetAttr = bitm->m_carpetSfx;
            if ((u32)carpetAttr < BG_ATTR_NUM) {
                ret = carpetAttr;
            }
            return ret;
        }
    }
    return attr;
}

// 800A8648
int getFootstepSe(const mVec3_c *pos, u8 *wear) {
    u32 se = dBGCF::getFootstepSe(getAttr(pos));
    if (wear != NULL) {
        *wear = 0;
        dFdBase_c *fd = fn_80190C44(FD_ID_CURRENT);
        if (fd != NULL) {
            *wear = fd->getGrassWear(pos, FALSE);
        }
    }
    if (se == GROUND_SE_GRASS_ALT) {
        se = GROUND_SE_GRASS;
    }
    u8 scene = dScene::getCurrentScene();
    if (dScene::isSceneAttr(scene, dScene::SCENE_ATTR_ROOM)) {
        int unitX, unitZ;
        dBGCF::posToUnit(&unitX, &unitZ, pos);
        if (dBGCF::getAttr(unitX, unitZ) == BG_ATTR_WALL && scene < dScene::SCENE_NUM) {
            return l_roomWallSe[scene];
        }
    } else if (se == GROUND_SE_GRASS) {
        u32 season = dScene::getCurrentSeasonBres();
        switch (dBgDebug_c::getGroundSeType()) {
        case 0:
            se = GROUND_SE_GRASS;
            break;
        case 1:
            se = GROUND_SE_LEAVES;
            break;
        case 2:
            se = GROUND_SE_SNOW;
            break;
        default:
            if (season == SCENE_SEASON_BRES_SNOW) {
                se = GROUND_SE_SNOW;
            } else if (season >= 4 && season <= 8) {
                se = GROUND_SE_LEAVES;
            } else {
                se = GROUND_SE_GRASS;
            }
            break;
        }
    }
    return se;
}

// 800A87B0
int getFootprint(const mVec3_c *pos) {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM)) {
        switch (getAttr(pos)) {
        case BG_ATTR_SOIL:
            if (dScene::isWinterSeasonType()) {
                dFdBase_c *fd = fn_80190C44(FD_ID_CURRENT);
                if (fd != NULL) {
                    return fd->getGrassWear(pos, FALSE);
                }
            }
            break;
        case BG_ATTR_SAND:
        case BG_ATTR_WAVE:
            return 0xFF;
        }
    }
    return 0;
}

// 800A8850
int getSnowWear(const mVec3_c *pos) {
    if (!dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM) && getAttr(pos) == BG_ATTR_SOIL && dScene::isWinterSeasonType()) {
        dFdBase_c *fd = fn_80190C44(FD_ID_CURRENT);
        if (fd != NULL) {
            return fd->getGrassWear(pos, FALSE);
        }
    }
    return 0;
}

} // namespace dGroundAttr
