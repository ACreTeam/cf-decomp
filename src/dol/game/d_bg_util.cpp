// Bg helpers, namespace dBgUtil: the signs of the bg models, the room bg's wallpaper / carpet and
// the wallpaper / carpet of each scene. .text 800767A8..80077754, .ctors, .rodata 8046D2B0,
// .data 804A6CE0, .bss 80582E60..80582FB8, .sbss 8074E270..8074E280, .sdata2 807504B0..807504C8.
// See include/game/game/d_bg_util.hpp and notes/d_bg_util.txt.
#include <game/game/d_bg_util.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgi_draw.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

// Not split yet (C linkage keeps the target names).
extern "C" {
BOOL fn_800DCEDC();
void fn_800DD4C8();
void fn_800DD518(void *data, int size);
void fn_800DD588(int a, int b);
}

namespace dBgUtil {

// One sign of "sign.bin" (0x10 bytes), relative to the block's corner.
struct sign_s {
    /* 0x0 */ s16 mX;
    /* 0x2 */ s16 mY;
    /* 0x4 */ s16 mZ;
    /* 0x6 */ s16 mSizeX;
    /* 0x8 */ s16 mSizeZ;
    /* 0xA */ s16 mSizeY;
    /* 0xC */ s16 mAngleY; // degrees
    /* 0xE */ s8 mMsg;
    /* 0xF */ u8 mFlag : 1;
};

struct signData_s {
    /* 0x00 */ u32 mNum;
    /* 0x04 */ u8 _04[0x1C];
    /* 0x20 */ sign_s mSigns[1];
};

// The player house rooms (getSceneAttrTableIndex: the room, and the house in *home).
static const u32 l_homeScenes[] = {
    SCENE_ATTR_HOUSE0 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE1 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE2 | SCENE_ATTR_PLAYER_HOUSE,
    SCENE_ATTR_HOUSE3 | SCENE_ATTR_PLAYER_HOUSE,
    0,
    0,
};

// What the sign being created reads back (createSign).
static u16 l_signMsg;
static mVec3_c l_signSize;
static u8 l_signFlag;

static dBgiDraw_c *l_bgiDraw;

// The wallpaper / carpet of every scene that is not a player house room.
static dItem::Item l_wallpapers[SCENE_NUM];
static dItem::Item l_carpets[SCENE_NUM];
static dItem::Item l_noItem;

// 800767A8
u16 getSignMsg() {
    return l_signMsg;
}

// 800767B0
const mVec3_c *getSignSize() {
    return &l_signSize;
}

// 800767BC
u8 getSignFlag() {
    return l_signFlag;
}

// 800767C4
void createSign(const mVec3_c *pos, const mVec3_c *size, s16 angleY, int msg, int flag) {
    l_signMsg = msg;
    l_signSize = *size;
    l_signFlag = flag;
    mAng3_c angle(0, angleY, 0);
    dActor_c::construct(0xF6, getSceneParent(), 0, pos, &angle);
}

// 80076840
void createSigns(nw4r::g3d::ResFile res, int blockX, int blockZ) {
    signData_s *data = (signData_s *)res.GetExternal("sign.bin");
    if (data != NULL) {
        mVec3_c base(blockX * lbl_80750520, 0.0f, blockZ * lbl_80750524);
        u32 num = data->mNum;
        const sign_s *sign = data->mSigns;
        for (u32 i = 0; i < num; sign++, i++) {
            mVec3_c pos(base);
            pos.x = base.x + sign->mX;
            pos.y = base.y + sign->mY;
            pos.z = base.z + sign->mZ;
            mVec3_c size(sign->mSizeX, sign->mSizeY, sign->mSizeZ);
            s16 angle = 65536.0f * sign->mAngleY / 360.0f;
            createSign(&pos, &size, angle, sign->mMsg, sign->mFlag);
        }
    }
}

// 80076A64
void setBgiDraw(dBgiDraw_c *draw) {
    l_bgiDraw = draw;
}

// 80076A6C
void sendRoomItem(dItem::Item item, int type, int flag0, BOOL isWallpaper, int flag1) {
    if (fn_800DCEDC()) {
        roomItemMsg_c msg;
        msg.mItem = item.mId;
        msg.mScene = getCurrentScene();
        msg.mIsWallpaper = isWallpaper;
        msg.mType = type;
        msg.mFlag1 = flag1;
        msg.mFlag0 = flag0;
        fn_800DD4C8();
        fn_800DD518(&msg, sizeof(msg));
        fn_800DD588(0x3D, 4);
    }
}

// 80076AFC
dItem::Item getWallpaper() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->getWallpaper();
    }
    return dItem::ITEM_ID_NONE;
}

// 80076B30
BOOL setWallpaper(dItem::Item item, int type, int flag0, int flag1, BOOL send) {
    if (l_bgiDraw != NULL) {
        dBGI::alwaysAc_c *ac = l_bgiDraw;
        BOOL ret = ac->setWallpaper(item, type, flag0, flag1);
        if (send) {
            sendRoomItem(item, type, flag0, TRUE, flag1);
        }
        return ret;
    }
    return TRUE;
}

// 80076BE8: one of the current player's 8 designs
BOOL setDesignWallpaper(int slot, BOOL type2, int flag1, BOOL send) {
    int player = dPlayerMgr_c::getCurrentPlayer()->findInSave();
    dItem::Item item(0x295, (slot & 7) + (player & 3) * 8, FALSE);
    int type = 1;
    if (type2) {
        type = 2;
    }
    return setWallpaper(item, type, 0, flag1, send);
}

// 80076C84
BOOL isWallpaperDone() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->isWallpaperDone();
    }
    return TRUE;
}

// 80076CB0
dItem::Item getCarpet() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->getCarpet();
    }
    return dItem::ITEM_ID_NONE;
}

// 80076CE4
dItem::Item getCarpetRaw() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->getCarpetRaw();
    }
    return dItem::ITEM_ID_NONE;
}

// 80076D18
BOOL setCarpet(dItem::Item item, int type, int flag0, int flag1, BOOL send) {
    if (l_bgiDraw != NULL) {
        dBGI::alwaysAc_c *ac = l_bgiDraw;
        BOOL ret = ac->setCarpet(item, type, flag0, flag1);
        if (send) {
            sendRoomItem(item, type, flag0, FALSE, flag1);
        }
        return ret;
    }
    return TRUE;
}

// 80076DD0
BOOL setDesignCarpet(int slot, BOOL type2, int flag1, BOOL send) {
    int player = dPlayerMgr_c::getCurrentPlayer()->findInSave();
    dItem::Item item(0x295, (slot & 7) + (player & 3) * 8, FALSE);
    int type = 1;
    if (type2) {
        type = 2;
    }
    return setCarpet(item, type, 0, flag1, send);
}

// 80076E6C
BOOL isCarpetDone() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->isCarpetDone();
    }
    return TRUE;
}

// 80076E98: packet 0x3D
void recvRoomItem(const roomItemMsg_c *msg) {
    dItem::Item item(msg->mItem);
    BOOL isWallpaper = msg->mIsWallpaper != 0;
    u32 scene = msg->mScene;
    int type = msg->mType;
    BOOL flag0 = msg->mFlag0 != 0;
    BOOL flag1 = msg->mFlag1 != 0;
    if (scene == getCurrentScene()) {
        if (isWallpaper) {
            setSceneWallpaper(scene, item, type);
            setWallpaper(item, type, flag0, flag1, FALSE);
        } else {
            setSceneCarpet(scene, item, type);
            setCarpet(item, type, flag0, flag1, FALSE);
        }
    } else {
        if (isWallpaper) {
            setSceneWallpaper(scene, item, type);
        } else {
            setSceneCarpet(scene, item, type);
        }
    }
}

// 80076FB8: blockX / blockZ -1: the first block that has the node
BOOL getNodePos(mVec3_c *pos, const char *name, int blockX, int blockZ) {
    if (l_bgiDraw != NULL) {
        dBGI::alwaysAc_c *ac = l_bgiDraw;
        if (blockX == -1 || blockZ == -1) {
            dFdBase_c *fd = fn_80190C44(FD_ID_CURRENT);
            if (fd != NULL) {
                for (int z = 0; z < fd->mBlockH; z++) {
                    for (int x = 0; x < fd->mBlockW; x++) {
                        if (ac->getNodePos(pos, x, z, name)) {
                            return TRUE;
                        }
                    }
                }
            }
        } else {
            return ac->getNodePos(pos, blockX, blockZ, name);
        }
    }
    return FALSE;
}

// 800770B0
BOOL isBgiReady() {
    if (l_bgiDraw != NULL) {
        return ((dBGI::alwaysAc_c *)l_bgiDraw)->isReady();
    }
    return FALSE;
}

// 800770DC
void initSceneItems() {
    for (int i = 0; i < SCENE_NUM; i++) {
        l_wallpapers[i] = l_carpets[i] = dItem::ITEM_ID_NONE;
    }
}

// 800771D8
BOOL setSceneWallpaper(u8 scene, dItem::Item item, int type) {
    int home;
    int room = getSceneAttrTableIndex(scene, l_homeScenes, &home);
    if (room != -1) {
        if (dHome_c::isValidRoom(room)) {
            dSaveData_c::getHomeRoom(home, room)->setWallpaper(item);
            dSaveData_c::getHomeRoom(home, room)->_454 = type;
            return TRUE;
        }
        return FALSE;
    }
    if (scene < SCENE_NUM) {
        l_wallpapers[scene] = item;
        return TRUE;
    }
    return FALSE;
}

// 800772D8
dItem::Item getSceneWallpaper(u8 scene, int *type) {
    int dummy;
    int home;
    if (type == NULL) {
        type = &dummy;
    }
    int room = getSceneAttrTableIndex(scene, l_homeScenes, &home);
    if (room != -1 && dHome_c::isValidRoom(room)) {
        *type = dSaveData_c::getHomeRoom(home, room)->_454;
        return dSaveData_c::getHomeRoom(home, room)->getWallpaper();
    }
    if (scene < SCENE_NUM) {
        *type = 1;
        return l_wallpapers[scene];
    }
    return l_noItem;
}

// 800773E4
BOOL setSceneCarpet(u8 scene, dItem::Item item, int type) {
    int home;
    int room = getSceneAttrTableIndex(scene, l_homeScenes, &home);
    if (room != -1 && dHome_c::isValidRoom(room)) {
        dSaveData_c::getHomeRoom(home, room)->setCarpet(item);
        dSaveData_c::getHomeRoom(home, room)->_455 = type;
        return TRUE;
    }
    if (scene < SCENE_NUM) {
        l_carpets[scene] = item;
        return TRUE;
    }
    return FALSE;
}

// 800774DC
dItem::Item getSceneCarpet(u8 scene, int *type) {
    int dummy;
    int home;
    if (type == NULL) {
        type = &dummy;
    }
    int room = getSceneAttrTableIndex(scene, l_homeScenes, &home);
    if (room != -1 && dHome_c::isValidRoom(room)) {
        *type = dSaveData_c::getHomeRoom(home, room)->_455;
        return dSaveData_c::getHomeRoom(home, room)->getCarpet();
    }
    if (scene < SCENE_NUM) {
        if (isSceneAttr(scene, SCENE_ATTR_STORE_ROOM) || scene == SCENE_RM_HAPPY_MDL) {
            *type = 2;
        } else {
            *type = 0;
        }
        return l_carpets[scene];
    }
    return l_noItem;
}

} // namespace dBgUtil
