// Field tasks of the FG manager (see d_fg_mng_task.hpp). #included by d_field_assessment.cpp: with
// -sym on MWCC puts the functions of this file after the TU's main .text (800A2220..800A597C).

#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/framework/f_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <nw4r/math.h>

// The item filter at 8059FF80 (fn_800C60B4).
struct dFgMngItemFilter_c {
    /* 0x0 */ u32 _00;
    /* 0x4 */ u32 _04;
    /* 0x8 */ int mNum;
};
extern "C" dFgMngItemFilter_c lbl_8059FF80;
extern "C" EGG::Heap *lbl_8074E3F8;

// Actor fields read here (fBase_c / dActor_c).
struct dFgMngActor_c {
    /* 0x0000 */ u8 _0000[8];
    /* 0x0008 */ u16 mProfName;
    /* 0x000A */ u8 _000A[0x5A];
    /* 0x0064 */ nw4r::math::VEC3 mPos;
    /* 0x0070 */ u8 _0070[0x1BC8];
    /* 0x1C38 */ u8 mState;
};

static inline BOOL fgMngInRange(u16 id, u16 lo, u16 hi) {
    BOOL in = FALSE;
    if (!(id < lo || id > hi)) {
        in = TRUE;
    }
    return in;
}

// 8046EFA0: the fruit of each fruit tree type (getFruit).
static const int sFruit[] = {dItem::ITEM_IDX_APPLE, dItem::ITEM_IDX_ORANGE, dItem::ITEM_IDX_PEAR, dItem::ITEM_IDX_PEACH, dItem::ITEM_IDX_CHERRY, dItem::ITEM_IDX_COCONUT, dItem::ITEM_IDX_30000_BELLS};
// 8046EFBC: the unit and its 8 neighbours.
static const u8 sDirOffset[] = {
    FG_UNIT_OFS(0, 0), FG_UNIT_OFS(-1, 0), FG_UNIT_OFS(1, 0), FG_UNIT_OFS(0, 1),
    FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(1, 1), FG_UNIT_OFS(0, -1), FG_UNIT_OFS(-1, -1),
    FG_UNIT_OFS(1, -1),
};
// 8046EFC8
static const int sFruitDir[] = {1, 2, 3, 4, 5, 0};
// 8046F120: the order the spots are tried in, per direction.
static const int sSpotOrder[5][8] = {
    {0, 3, 6, 1, 4, 7, 5, 8}, {0, 3, 6, 2, 5, 8, 4, 7}, {0, 1, 2, 3, 4, 5, 7, 8},
    {0, 2, 6, 3, 1, 4, 5, 7}, {0, 1, 6, 3, 2, 5, 4, 8},
};

// 800A2220
void dFgMngTask_c::reset() {
    clear();
}

// 800A2224
void dFgMngTask_c::update() {
    if (mPos != FG_MNG_POS_NONE) {
        switch (mKind) {
        case FG_MNG_KIND_PICK_UP_FULL:
        case FG_MNG_KIND_DIG_UP_FULL:
        case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
            break;
        case FG_MNG_KIND_PITFALL:
            if (!fn_800DCEDC()) {
                mNewItem = dItem::ITEM_ID_NONE;
                apply();
                clear();
            } else {
                apply();
            }
            break;
        default:
            proc();
            break;
        }
    }
}

// 800A22C4
void dFgMngTask_c::set(int player, u8 scene, u16 pos, u16 pos2, u16 item, u16 item2, int kind, u8 stage,
                       u8 b2, u8 idx, u8 layer, s8 day) {
    mPos = pos;
    mLinkPos = pos2;
    mNewItem = item;
    mLayer = layer;
    mPlayer = player;
    mPrevItem = item2;
    mDay = day;
    mStage = stage;
    mB2 = b2;
    mKind = kind;
    mDropIdx = idx;
    mSet = 1;
    mHold = 0;
    mScene = scene;
}

// 800A233C
void dFgMngTask_c::proc() {
    int stage = 0;
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd != NULL) {
        dFdAsPos_c pos(getX(), getZ());
        dItem::Item *item = fd->getItem(getX(), getZ(), 0);
        if (item != NULL) {
            switch (mKind) {
            case FG_MNG_KIND_PUT_PENDING: {
                fgMngTask_finishAt(mPlayer, dFgMngTaskPos_c(mLinkPos));
            }
            case FG_MNG_KIND_SWAP:
            case FG_MNG_KIND_BURY:
            case FG_MNG_KIND_PLANT:
            case FG_MNG_KIND_PUT_DESIGN:
                if (!item->isFlower() && item->mId != dItem::FG_LUCKY_CLOVER && !fgMngInRange(item->mId, dItem::FG_DANDELION, dItem::FG_DANDELION_PUFF) &&
                    !item->isTree() && mStage != 0) {
                    stage = 1;
                }
                break;
            case FG_MNG_KIND_CHOP_TREE:
            case FG_MNG_KIND_CHOP_TREE_SPECIAL:
                if (mB2 != 2) {
                    sFgMngState.setUnitFlag(&pos, (mStage + 1) & 3);
                }
                break;
            case FG_MNG_KIND_DIG_UP_FULL:
                stage = mStage;
                break;
            case FG_MNG_KIND_PITFALL:
                if (mDay >= 0) {
                    mNewItem = dItem::ITEM_ID_NONE;
                }
                break;
            }
        }
    }
    switch (mKind) {
    case FG_MNG_KIND_PULL:
        if (fgMngInRange(mPrevItem, dItem::FG_WEED_A, dItem::FG_WEED_D)) {
            fn_800C77D0(mPlayer);
        }
        break;
    case FG_MNG_KIND_PICK_UP_SPECIAL:
    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
        if (mPrevItem == dItem::FG_LUCKY_CLOVER) {
            fn_800C7818(mPlayer);
        }
        break;
    case FG_MNG_KIND_CHOP_TREE:
        if (fgMngInRange(mNewItem, dItem::FG_STUMP_S1, dItem::FG_PALM_STUMP)) {
            fn_800C77B8(mPlayer);
        }
        break;
    }
    mStage = stage;
    apply();
    finish();
}

static inline BOOL fgMngInRange(const dItem::Item *item, u16 lo, u16 hi) {
    BOOL in = FALSE;
    if (item->mId >= lo && item->mId <= hi) {
        in = TRUE;
    }
    return in;
}

// 800A2584
void dFgMngTask_c::procGroup() {
    switch (mKind) {
    case FG_MNG_KIND_SHAKE_TREE:
    case FG_MNG_KIND_SHAKE_TREE_SPECIAL:
    case FG_MNG_KIND_CHOP_TREE:
    case FG_MNG_KIND_CHOP_TREE_SPECIAL: {
        int num = 0;
        u16 id = mPrevItem;
        dItem::Item item(id);
        dItem::FgInfo *info = item.getFgInfo();
        if (info != NULL && (u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
            if (fgMngInRange(&item, dItem::FG_CEDAR_SAPLING, dItem::FG_CEDAR_LIGHTS) ||
                fgMngInRange(&item, dItem::FG_PALM_SAPLING, dItem::FG_PALM_FRUIT)) {
                num = 2;
            } else {
                num = 3;
            }
        } else if (id == dItem::FG_TREE_FTR || id == dItem::FG_TREE_BELLS || id == dItem::FG_CEDAR_FTR ||
                   id == dItem::FG_CEDAR_BELLS) {
            num = 1;
        }
        for (int i = 0; i < num; i++) {
            int idx = fgMngTask_findGroup(mPlayer, i, FALSE);
            if (idx >= 0) {
                dFgMngTask_c *task = &sFgMngTaskList.mTasks[idx];
                if (i == task->mDropIdx) {
                    task->proc();
                }
            }
        }
        break;
    }
    case FG_MNG_KIND_PITFALL:
        mNewItem = dItem::ITEM_ID_NONE;
        break;
    }
    proc();
}

// 800A2718
void dFgMngTask_c::cancel() {
    switch (mKind) {
    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
        if (mPrevItem == dItem::FG_LUCKY_CLOVER) {
            fn_800C7818(mPlayer);
        }
    case FG_MNG_KIND_PICK_UP_FULL:
    case FG_MNG_KIND_DIG_UP_FULL:
    case FG_MNG_KIND_PITFALL:
        apply();
        mActive = 1;
        mSet = 0;
        break;
    default:
        proc();
        break;
    }
}

// 800A27AC
void dFgMngTask_c::clear() {
    *(u32 *)this = 0;
    mPos = FG_MNG_POS_NONE;
    mLinkPos = FG_MNG_POS_NONE;
    mNewItem = dItem::ITEM_ID_NONE;
    mPlayer = PLAYER_NONE;
    mDropIdx = DROP_IDX_NONE;
    mLayer = 0;
    mDay = -1;
    mScene = FG_MNG_SCENE_ANY;
}

// 800A2800
void dFgMngTask_c::finish() {
    dFdAsPos_c pos(getX(), getZ());
    int idx = fgMngCmd_find(mPlayer, mKind, mScene, (dFdAsPos_c *)&pos, mLayer);
    if (idx >= 0) {
        fgMngCmd_get(idx)->init();
    }
    if (fn_800DCF90()) {
        dFdAsPos_c pos2(getX(), getZ());
        int i = fgMngLock_find(mScene, (dFdAsPos_c *)&pos2, mLayer);
        if (i >= 0) {
            dFgMngLock_c *lock = fgMngLock_get(i);
            lock->mPendingPlayers &= ~1;
        }
        clear();
    } else {
        dFgMngTaskMsg_c msg;
        msg.mPlayer = mPlayer;
        msg.mLayer = mLayer;
        msg.mPos = mPos;
        msg.mScene = mScene;
        fn_800DD4C8();
        fn_800DD518(&msg, sizeof(msg));
        fn_800DD588(FG_MNG_PACKET_TASK_DONE, 0);
        clear();
    }
}

// 800A2918
void dFgMngTask_c::apply() {
    if (isSceneAttr(mScene, SCENE_ATTR_TOWN)) {
        dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
        dFgMngProc_c::setUnitItem(fd, getX(), getZ(), mNewItem, mStage);
        notify(mStage != 0);
    } else {
        dFdBase_c *fd = fn_80190C44(fn_80190C58(mScene));
        if (fd != NULL) {
            dItem::Item item(mNewItem);
            fd->setItem(&item, getX(), getZ(), mLayer);
        }
    }
}

// 800A29D0
void dFgMngTask_c::notify(BOOL keep) {
    if (!keep) {
        dItem::Item item;
        item = mNewItem;
        switch (item.getKind()) {
        case dItem::KIND_BAD_KABU:
            sFgMngLitterFlags.mHasBadKabu = 1;
            break;
        case dItem::KIND_DUST:
            sFgMngLitterFlags.mHasDust = 1;
            break;
        case dItem::KIND_CANDY:
            sFgMngLitterFlags.mHasCandy = 1;
            break;
        }
    }
}

// 800A2A5C
void dFgMngTaskList_c::init() {
    if (!mInit) {
        mInit = 1;
        for (int i = 0; i < FG_MNG_TASK_NUM; i++) {
            mTasks[i].reset();
        }
    }
}

// 800A2ABC
void dFgMngTaskList_c::reset() {
    for (int i = 0; i < FG_MNG_TASK_NUM; i++) {
        mTasks[i].reset();
    }
}

// 800A2B08
void dFgMngTaskList_c::update() {
    fgMngTask_updateAll();
}

// 800A2B0C
BOOL fgMngTask_add(u8 player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind, u8 stage,
                   u8 b2, u8 idx, u8 layer, s8 day) {
    BOOL ok = FALSE;
    int i = fgMngTask_find(scene, &pos, layer);
    if (i >= 0) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if ((player & 3) == task->mPlayer && task->mActive) {
            fgMngTask_set(i, player, scene, pos, pos2, item, item2, kind, stage, b2, idx, layer, day);
            ok = TRUE;
        }
    } else {
        ok = fgMngTask_new(player, scene, pos, pos2, item, item2, kind, stage, b2, idx, layer, day);
    }
    if (ok == TRUE && fn_800DCF90()) {
        fgMngLock_add(scene, (dFdAsPos_c *)&pos, layer);
    }
    return ok;
}

// 800A2C98
BOOL fgMngTask_replace(int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, u32 kind,
                       u8 stage, u8 b2, u8 idx, u8 layer, s8 day) {
    dFgMngTask_c *task;
    int i = fgMngTask_find(scene, &pos, layer);
    if (i >= 0) {
        task = &sFgMngTaskList.mTasks[i];
        if (!task->mActive) {
            switch (task->mKind) {
            case FG_MNG_KIND_SHAKE_TREE:
            case FG_MNG_KIND_SHAKE_TREE_SPECIAL:
            case FG_MNG_KIND_CHOP_TREE:
            case FG_MNG_KIND_CHOP_TREE_SPECIAL: {
                int num = 0;
                u16 id = task->mPrevItem;
                dItem::Item item(id);
                dItem::FgInfo *info = item.getFgInfo();
                if (info != NULL && (u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
                    if (fgMngInRange(&item, dItem::FG_CEDAR_SAPLING, dItem::FG_CEDAR_LIGHTS) ||
                        fgMngInRange(&item, dItem::FG_PALM_SAPLING, dItem::FG_PALM_FRUIT)) {
                        num = 2;
                    } else {
                        num = 3;
                    }
                } else if (id == dItem::FG_TREE_FTR || id == dItem::FG_TREE_BELLS || id == dItem::FG_CEDAR_FTR ||
                           id == dItem::FG_CEDAR_BELLS) {
                    num = 1;
                }
                for (int j = 0; j < num; j++) {
                    int k = fgMngTask_findGroup(player, j, FALSE);
                    if (k >= 0) {
                        dFgMngTask_c *other = &sFgMngTaskList.mTasks[k];
                        if (j == other->mDropIdx) {
                            other->proc();
                        }
                    }
                }
                break;
            }
            case FG_MNG_KIND_DIG_UP_FULL:
                if (kind == FG_MNG_KIND_BURY) {
                    return FALSE;
                }
                break;
            case FG_MNG_KIND_PICK_UP_FULL:
            case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
                if (kind == FG_MNG_KIND_SWAP) {
                    return FALSE;
                }
                break;
            }
            task->proc();
        }
        fgMngTask_set(i, player, scene, pos, pos2, item, item2, kind, stage, b2, idx, layer, day);
    } else {
        fgMngTask_new(player, scene, pos, pos2, item, item2, kind, stage, b2, idx, layer, day);
    }
    if (fn_800DCF90()) {
        fgMngLock_add(scene, (dFdAsPos_c *)&pos, layer);
    }
    return TRUE;
}

// 800A2F58
void fgMngTask_procAt(u8 scene, const dFdAsPos_c *pos, u32 layer) {
    fgMngTask_proc(fgMngTask_find(scene, pos, layer));
}

// 800A2F7C
void fgMngTask_procAt(int x, int z, u32 layer) {
    dFdAsPos_c pos(x, z);
    fgMngTask_procAt(getCurrentScene(), &pos, layer);
}

// 800A2FC0
void fgMngTask_proc(int idx) {
    if (idx >= 0) {
        sFgMngTaskList.mTasks[idx].proc();
    }
}

// 800A2FE4
void fgMngTask_procOwn(u8 scene, const dFdAsPos_c *pos, u32 player, u32 layer) {
    int idx = fgMngTask_find(scene, pos, layer);
    if (idx >= 0) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[idx];
        if ((player & 3) == task->mPlayer) {
            task->proc();
        }
    }
}

// 800A3044
void fgMngTask_procGroupAt(u8 x, u8 z, u8 layer) {
    dFdAsPos_c pos(x, z);
    int idx = fgMngTask_find(FG_MNG_SCENE_ANY, &pos, layer);
    if (idx >= 0) {
        sFgMngTaskList.mTasks[idx].procGroup();
    }
}

// 800A3094
void fgMngTask_releaseAt(u8 scene, const dFdAsPos_c *pos, u32 layer) {
    int idx = fgMngTask_find(scene, pos, layer);
    if (idx >= 0) {
        sFgMngTaskList.mTasks[idx].release();
    }
}

// 800A30D4
void fgMngTask_releaseAt(int x, int z, u32 layer) {
    dFdAsPos_c pos(x, z);
    fgMngTask_releaseAt(getCurrentScene(), &pos, layer);
}

// 800A3118
void fgMngTask_releaseAt(u8 scene, int x, int z, u32 layer) {
    dFdAsPos_c pos(x, z);
    fgMngTask_releaseAt(scene, &pos, layer);
}

// 800A314C
void dFgMngTask_c::release() {
    for (int i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (task->isLinkOf(this)) {
            task->mActive = 0;
            break;
        }
    }
    cancel();
}

// 800A3200
void fgMngTask_finishActive(u8 player) {
    int idx = fgMngTask_findActive(player);
    if (idx >= 0) {
        sFgMngTaskList.mTasks[idx].finish();
    }
}

// 800A3240
void fgMngTask_finishAt(int player, dFgMngTaskPos_c pos) {
    player &= 3;
    for (int i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (task->isOwnerAtP(player, &pos)) {
            task->finish();
            break;
        }
    }
}

// 800A338C
int fgMngTask_findMarked(u8 scene, const dFdAsPos_c *pos, u32 layer) {
    if (fgMngLock_find(scene, (dFdAsPos_c *)pos, layer) < 0) {
        return -1;
    }
    return fgMngTask_find(scene, pos, layer);
}

// 800A33F0
void fgMngTask_remove(int player, const dFdAsPos_c *pos) {
    int idx = fgMngTask_findOwn(player, 0, pos, 0);
    if (idx >= 0) {
        sFgMngTaskList.mTasks[idx].clear();
        dFgMngLock_c *lock = fgMngLock_get(fgMngLock_find(0, (dFdAsPos_c *)pos, 0));
        lock->mPendingPlayers = 0;
    }
}

// 800A3468
void fgMngTask_setBusy() {
    sFgMngTaskList.mBusy = 1;
}

// 800A347C
void fgMngTask_updateAll() {
    dFgMngTask_c *task = fgMngTask_get(0);
    for (int i = 0; i < FG_MNG_TASK_NUM; i++) {
        task->update();
        task++;
    }
    sFgMngTaskList.mBusy = 0;
}

// 800A34E0
int fgMngTask_find(u32 scene, const dFdAsPos_c *pos, u32 layer) {
    int i;
    int found = -1;
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (task->isAtScene(scene, pos, layer)) {
            found = i;
            break;
        }
    }
    return found;
}

// 800A35B0
int fgMngTask_findOwn(int player, u32 scene, const dFdAsPos_c *pos, u32 layer) {
    int i;
    int found = -1;
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (task->isOwnAtScene(player, scene, pos, layer)) {
            found = i;
            break;
        }
    }
    return found;
}

// 800A3694
int fgMngTask_findFree() {
    dFgMngTask_c *task = sFgMngTaskList.mTasks;
    int i;
    int found = -1;
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        if (task->isFree()) {
            found = i;
            break;
        }
        task++;
    }
    return found;
}

// 800A379C
int fgMngTask_findActive(u8 player) {
    int i;
    int found = -1;
    player &= 3;
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (player == task->mPlayer && task->mActive) {
            found = i;
            break;
        }
    }
    return found;
}

// 800A3868
int fgMngTask_findGroup(u32 player, u8 idx, BOOL notHeld) {
    int i;
    int found = -1;
    if (!fn_800DCEDC()) {
        player = 0;
    }
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (player == task->mPlayer && idx == task->mDropIdx && (!notHeld || !task->mHold)) {
            found = i;
            break;
        }
    }
    return found;
}

// 800A39E8
dFgMngTask_c *fgMngTask_get(int idx) {
    if (idx < 0 || idx >= FG_MNG_TASK_NUM) {
        return NULL;
    }
    return &sFgMngTaskList.mTasks[idx];
}

// 800A3A18
u16 *fgMngTask_getActive(int player) {
    int i;
    u16 *found = NULL;
    if (!fn_800DCEDC()) {
        player = 0;
    }
    for (i = 0; i < FG_MNG_TASK_NUM; i++) {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[i];
        if (player == task->mPlayer && task->mActive) {
            found = &task->mPos;
            break;
        }
    }
    return found;
}

// 800A3B14
void fgMngTask_set(int idx, int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind,
                   u8 stage, u8 b2, u8 idx2, u8 layer, s8 day) {
    dFgMngTaskPos_c at(pos);
    dFgMngTaskPos_c link(pos2);
    sFgMngTaskList.mTasks[idx].set(player, scene, at.mPos, link.mPos, item, item2,
                                 kind, stage, b2, idx2, layer, day);
}

// 800A3BAC
BOOL fgMngTask_new(int player, u8 scene, dFdAsPos_c pos, dFdAsPos_c pos2, u16 item, u16 item2, int kind, u8 stage,
                   u8 b2, u8 idx, u8 layer, s8 day) {
    BOOL ok = FALSE;
    if (fgMngTask_canAdd(scene, pos)) {
        int i = fgMngTask_findFree();
        if (i >= 0) {
            fgMngTask_set(i, player, scene, pos, pos2, item, item2, kind, stage, b2, idx, layer, day);
            ok = TRUE;
        }
    }
    return ok;
}

// 800A3C9C
BOOL fgMngTask_canAdd(u8 scene, dFdAsPos_c pos) {
    int idx;
    BOOL ok = TRUE;
    idx = fgMngTask_find(scene, &pos, 0);
    if (idx >= 0) {
        switch (fgMngTask_get(idx)->mKind) {
        case FG_MNG_KIND_PUT_DESIGN:
            fgMngTask_proc(idx);
            break;
        default:
            ok = FALSE;
            break;
        }
    }
    return ok;
}

// 800A3D0C
void dFgMngStone_c::reset() {
    init();
}

// 800A3D10
void dFgMngStone_c::init() {
    mHit = 0;
    mTimer = TIMER_NONE;
}

// 800A3D24
void dFgMngStone_c::update() {
    if (!fn_800DCEDC()) {
        if (mTimer > 0) {
            mTimer--;
        }
        if (mTimer == 0) {
            dFdBase_c *fd;
            dFdAsPos_c pos(mPos.mPos >> 8, mPos.mPos & 0xFF);
            if (fgMngTask_find(0, &pos, 0) < 0) {
                u16 item = dItem::FG_STONE_A;
                fd = fn_80190C44(FD_ID_TOWN);
                mTimer = TIMER_NONE;
                if (fd != NULL) {
                    dItem::Item *cur = fd->getItem(pos.mX, pos.mZ, 0);
                    if (cur != NULL) {
                        item = (u32)(cur->mId - dItem::FG_MONEY_ROCK_P0_A) % 5 + dItem::FG_STONE_A;
                    }
                }
                dFgMngProc_c::setUnitItem(fd, mPos.mPos >> 8, mPos.mPos & 0xFF, item, 0);
            }
        }
    }
}

// 800A3E30
void dFgMngStone_c::hit(dFdAsPos_c pos) {
    u8 *townFlags;
    if (mTimer < 0) {
        mPos.set(pos.mX, pos.mZ);
        mTimer = ((u32)(*dHR::getRate(4) * 0x898) >> 12) + 0x1D1;
    }
    int max = HIT_MAX;
    if (mHit == 0) {
        townFlags = dSaveData_c::getTown()->_05EC68;
        if (*(u16 *)&townFlags[0xC] & 0x100) {
            int player = fn_801017B8();
            u8 players = townFlags[0x11];
            if (players & (1 << player)) {
                max = HIT_MAX_BONUS;
                mHit++;
            }
        }
    }
    mHit++;
    if (mHit >= max) {
        mHit = max;
    }
    spawn(&pos);
}

// 800A3F20
void dFgMngStone_c::spawn(const dFdAsPos_c *pos) {
    int w;
    int h;
    dFdBase_c *fd;
    const u8 *dir;
    int i;
    fd = fn_80190C44(FD_ID_TOWN);
    if (fd != NULL) {
        w = fd->mBlockW * UT_X_NUM;
        h = fd->mBlockH * UT_Z_NUM;
        dir = sAround8;
        for (i = 0; i < 8; i++, dir++) {
            dFdAsPos_c p;
            p.mX = pos->mX - ((*dir >> 4) - 8);
            p.mZ = pos->mZ - ((*dir & 0xF) - 8);
            if (p.mX >= 0 && p.mX < w && p.mZ >= 0 && p.mZ < h && (p.mX != pos->mX || p.mZ != pos->mZ)) {
                if (drop(fd, &p)) {
                    break;
                }
            }
        }
    }
}

// 800A4000
BOOL dFgMngStone_c::drop(dFdBase_c *fd, const dFdAsPos_c *pos) {
    static const int sMoney[] = {dItem::ITEM_IDX_100_BELLS, dItem::ITEM_IDX_100_BELLS, dItem::ITEM_IDX_200_BELLS, dItem::ITEM_IDX_300_BELLS, dItem::ITEM_IDX_500_BELLS, dItem::ITEM_IDX_1000_BELLS, dItem::ITEM_IDX_2000_BELLS, dItem::ITEM_IDX_4000_BELLS, dItem::ITEM_IDX_8000_BELLS, dItem::ITEM_IDX_16000_BELLS};
    static const int sMoneyGold[] = {dItem::ITEM_IDX_100_BELLS, dItem::ITEM_IDX_200_BELLS, dItem::ITEM_IDX_400_BELLS, dItem::ITEM_IDX_600_BELLS, dItem::ITEM_IDX_1000_BELLS, dItem::ITEM_IDX_2000_BELLS, dItem::ITEM_IDX_4000_BELLS, dItem::ITEM_IDX_8000_BELLS, dItem::ITEM_IDX_16000_BELLS, dItem::ITEM_IDX_32000_BELLS};
    if (fgMngTask_find(0, pos, 0) >= 0) {
        return FALSE;
    }
    BOOL ok = FALSE;
    dItem::Item *cur = fd->getItem(pos->mX, pos->mZ, 0);
    if (cur != NULL && cur->mId == dItem::ITEM_ID_NONE && fd->canPutItem(pos->mX, pos->mZ)) {
        dItem::Item shovel(dItem::ITEM_IDX_SILVER_SHOVEL);
        BOOL gold = fgMngProc_getMemberHeldItem(fn_800DCF58()) == shovel.getId() && cM::rndF(100.0f) < 10.0f;
        u16 item;
        u8 stage;
        if (gold) {
            item = dItem::Item(sMoneyGold[mHit]).mId;
            stage = 1;
        } else {
            item = dItem::Item(sMoney[mHit]).mId;
            stage = 0;
        }
        dFdAsPos_c none(-1, -1);
        fgMngTask_add(0, 0, *pos, none, item, dItem::ITEM_ID_NONE, FG_MNG_KIND_DROP, stage, 0, dFgMngTask_c::DROP_IDX_STONE, 0, -1);
        ok = TRUE;
    }
    return ok;
}

// 800A41A8
void dFgMngState_c::clearPlayer(int player) {
    mPlayers[player].mState = dFgMngPlayerState_c::STATE_NONE;
    mPlayers[player].mKind = 0;
}

// 800A41C0
void dFgMngState_c::init() {
    for (int i = 0; i < 3; i++) {
        clearPlayer(i);
    }
    mStone.reset();
}

// 800A4214
void dFgMngState_c::updateStone() {
    mStone.update();
}

// 800A421C
u32 dFgMngState_c::getUnitFlag(const dFdAsPos_c *pos) {
    int shift = (pos->mX & 0xF) * 2;
    int idx = ((pos->mX - UT_X_NUM) >> 4) + (pos->mZ - UT_Z_NUM) * FG_BLOCK_X_NUM;
    if (idx < 0 || idx >= FG_UNIT_MASK_NUM) {
        idx = 0;
        shift = 0;
    }
    return (mUnitFlags[idx] >> shift) & 3;
}

// 800A426C
void dFgMngState_c::setUnitFlag(const dFdAsPos_c *pos, u32 flag) {
    int shift = (pos->mX & 0xF) * 2;
    int idx = ((pos->mX - UT_X_NUM) >> 4) + (pos->mZ - UT_Z_NUM) * FG_BLOCK_X_NUM;
    if (idx < 0 || idx >= FG_UNIT_MASK_NUM) {
        idx = 0;
        shift = 0;
    }
    mUnitFlags[idx] = (flag << shift) | (~(3 << shift) & mUnitFlags[idx]);
}

// 800A42D0
void fgMngState_clearUnitFlags(dFgMngState_c *state) {
    u32 *flags = sFgMngState.mUnitFlags;
    for (int i = 0; i < FG_UNIT_MASK_NUM; i++) {
        flags[i] = 0;
    }
}

// 800A4394
dFgMngPlayerState_c *fgMngState_getPlayer(int player) {
    return &sFgMngState.mPlayers[player];
}

// 800A43A8
u32 fgMngState_getUnitFlag(int x, int z) {
    dFdAsPos_c pos(x, z);
    return sFgMngState.getUnitFlag(&pos);
}

// 800A43DC: whether the player is left of *dist (picks the side a shaken tree drops onto).
BOOL fgMngState_isPlayerLeftOf(int player, const f32 *dist) {
    BOOL near = FALSE;
    nw4r::math::VEC3 pos;
    int a, b;
    // fn_80101DC0 is an empty stub (d_player_mgr); the original called it with these arguments.
    if (((BOOL (*)(nw4r::math::VEC3 *, int *, int *, int))fn_80101DC0)(&pos, &a, &b, player)) {
        if (pos.x < *dist) {
            near = TRUE;
        }
    } else {
        mVec3_c *p = fn_800FBC48(player);
        if (p != NULL && p->x < *dist) {
            near = TRUE;
        }
    }
    return near;
}

// 800A4478
BOOL fgMngState_isFreeOfPlayers(int x, int z, int exclude) {
    if (!fn_800DCEDC()) {
        return TRUE;
    }
    nw4r::math::VEC3 pos;
    dFdBase_c::getUnitCenterPos(&pos, x, z);
    for (int i = 0; i < 4; i++) {
        if (i != exclude) {
            dFgMngActor_c *actor = (dFgMngActor_c *)fn_800FBC7C(i);
            if (actor != NULL) {
                f32 dx = pos.x - actor->mPos.x;
                f32 dz = pos.z - actor->mPos.z;
                if (dx * dx + dz * dz < 256.0f) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

// 800A4550
BOOL fgMngState_isFreeOfNpcs(int x, int z) {
    static const u16 sProfs[] = {0x6F, 0x79, 0x97, 0x9B, 0x95, 0x9F, 0x6E, 0x7C};
    const u16 *prof = sProfs;
    for (int i = 0; i < 8; i++, prof++) {
        dFgMngActor_c *actor = (dFgMngActor_c *)fManager_c::searchBaseByProfName(*prof, NULL);
        if (actor != NULL && (actor->mState == 1 || actor->mProfName == 0x9F)) {
            f32 fx = actor->mPos.x;
            f32 fz = actor->mPos.z;
            int ux = (int)fx >> 5;
            int uz = (int)fz >> 5;
            if (ux == x && uz == z) {
                if (actor->mProfName != 0x6E || !dSaveData_c::getRaw()->isFlag(4)) {
                    return FALSE;
                }
            }
        }
    }
    int construction = dSaveData_c::getTown()->mNewConstruction;
    if (construction == NEW_CONSTRUCTION_FOUNTAIN) {
        dFgMngActor_c *actor = (dFgMngActor_c *)fManager_c::searchBaseByProfName(0x93, NULL);
        if (actor != NULL && actor->mState == 1) {
            f32 fx = actor->mPos.x;
            f32 fz = actor->mPos.z;
            int ux = (int)fx >> 5;
            int uz = (int)fz >> 5;
            if (ux == x && uz == z) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

// 800A46B4
BOOL fgMngState_isFree(int x, int z) {
    return fgMngState_isFreeOfPlayers(x, z, fn_800DCF58());
}

// 800A46FC
BOOL fgMngState_canAct(int player, int x, int z) {
    dFgMngPlayerState_c *st = &sFgMngState.mPlayers[player];
    dFdAsPos_c pos(x, z);
    BOOL ok = FALSE;
    u8 kind = st->mKind;
    if (kind == FG_MNG_KIND_WATER) {
        return TRUE;
    }
    int idx = fgMngTask_findMarked(getCurrentScene(), &pos, st->mLayer);
    if (idx < 0) {
        if (kind == FG_MNG_KIND_SWAP) {
            ok = TRUE;
        } else if (fgMngState_isFree(x, z)) {
            ok = TRUE;
        }
    } else {
        dFgMngTask_c *task = &sFgMngTaskList.mTasks[idx];
        if (task->mPlayer == (st->mPlayer & 3) && task->mDay == st->mDay && task->mActive) {
            switch (task->mKind) {
            case FG_MNG_KIND_PICK_UP_FULL:
            case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
                if (st->mKind != FG_MNG_KIND_SWAP) {
                    return FALSE;
                }
                break;
            case FG_MNG_KIND_DIG_UP_FULL:
                if (st->mKind != FG_MNG_KIND_BURY || st->mItem == dItem::ITEM_ID_NONE) {
                    return FALSE;
                }
                break;
            default:
                return FALSE;
            }
            ok = TRUE;
        }
    }
    return ok;
}

// 800A4868
static inline void fgMngWaterAround(int player, dFdAsPos_c pos) {
    fgMngProc_waterAround(player, &pos);
}

BOOL dFgMngReq_c::proc(dFgMngPlayerState_c *st) {
    BOOL ok = FALSE;
    dItem::Item item;
    item = mItem;
    switch (mKind) {
    case FG_MNG_KIND_BURY:
        if (item.mId < dItem::FG_COUNT) {
            mItem = item.getPlantedFg();
        }
        break;
    case FG_MNG_KIND_WATER:
        u32 p = st->mPlayer;
        fgMngWaterAround(p, st->mPos);
        ok = TRUE;
        break;
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_MISS:
        ok = TRUE;
        break;
    case FG_MNG_KIND_HIT_MONEY_ROCK:
        sFgMngState.mStone.hit(st->mPos);
        break;
    }
    return ok;
}

// 800A4948
void dFgMngCmd_c::init() {
    mKind = FG_MNG_KIND_NONE;
    mPos = FG_MNG_POS_NONE;
    mNewItem = ITEM_FREE;
    mItem = dItem::ITEM_ID_NONE;
}

// 800A4974
BOOL dFgMngCmd_c::exec() {
    dFdAsPos_c pos(mPos >> 8, mPos & 0xFF);
    dFdAsPos_c pos2;
    BOOL ok;
    switch (mKind) {
    case FG_MNG_KIND_BLOCKED:
    case FG_MNG_KIND_HIT_ROCK:
    case FG_MNG_KIND_WATER:
    case FG_MNG_KIND_MISS:
        return TRUE;
    case FG_MNG_KIND_SWAP:
    case FG_MNG_KIND_PUT_PENDING:
        pos2 = dFdAsPos_c(mDropPos[0].mPos >> 8, mDropPos[0].mPos & 0xFF);
        break;
    }
    dItem::Item *item = fn_80190C44(fn_80190C58(mScene))->getItem(pos.mX, pos.mZ, 0);
    u8 b2 = mB2;
    u8 layer = mLayer;
    u8 kind = mKind;
    u8 stage = mStage;
    u16 prev = item->mId;
    ok = fgMngTask_replace(mPlayer, mScene, pos, pos2, mNewItem, prev, kind, stage, b2, dFgMngTask_c::DROP_IDX_MAIN, layer, -1);
    if (fn_800DCF90()) {
        int player = mPlayer;
        if (player == fn_800DCF58()) {
            goto done;
        }
    }
    if (mHasDrops) {
        u16 group = mDropItem;
        pos2 = dFdAsPos_c(-1, -1);
        for (int i = 0; i < 3; i++) {
            pos = dFdAsPos_c(*(u16 *)&mDropPos[i] >> 8, *(u16 *)&mDropPos[i] & 0xFF);
            if (mDropPos[i].mPos != FG_MNG_POS_NONE) {
                u8 layer = mLayer;
                fgMngTask_replace(mPlayer, mScene, pos, pos2, group, dItem::ITEM_ID_NONE, FG_MNG_KIND_DROP, 0, 0, i, layer, -1);
            }
        }
    }
done:
    return ok;
}

// 800A4B78
void dFgMngReq_c::send(BOOL local, int player, u8 scene) {
    dFdAsPos_c pos(mPos >> 8, mPos & 0xFF);
    if (mKind == FG_MNG_KIND_WATER) {
        fgMngProc_waterAround(player, &pos);
        return;
    }
    dFgMngCmd_c cmd;
    cmd.mHasDrops = 0;
    cmd.mPlayer = player;
    cmd.mKind = mKind;
    cmd.mReqPlayer = mPlayer;
    cmd.mLocal = local;
    cmd.mB2 = mB2;
    cmd.mLayer = mLayer;
    cmd.mStage = mStage;
    cmd.mPos = mPos;
    cmd.mNewItem = mItem;
    cmd.mItem = mItem2;
    cmd.mScene = scene;
    cmd.mDropPos[0].mPos = FG_MNG_POS_NONE;
    cmd.mDropPos[1].mPos = FG_MNG_POS_NONE;
    cmd.mDropPos[2].mPos = FG_MNG_POS_NONE;
    int size = dFgMngCmd_c::SEND_SIZE_SHORT;
    if (local) {
        getCurrentScene();
        switch (cmd.mKind) {
        case FG_MNG_KIND_BLOCKED:
        case FG_MNG_KIND_HIT_ROCK:
        case FG_MNG_KIND_MISS:
        case FG_MNG_KIND_PLANT:
        case FG_MNG_KIND_PUT_DESIGN:
            break;
        default:
            switch (cmd.plan(this, player)) {
            case dFgMngCmd_c::PLAN_NO_DROPS:
                switch (cmd.mKind) {
                case FG_MNG_KIND_SWAP:
                case FG_MNG_KIND_PUT_PENDING:
                    size = dFgMngCmd_c::SEND_SIZE_FULL;
                    cmd.mDropPos[0].mPos = sFgMngTaskList.mTasks[fgMngTask_findActive(player)].mPos;
                    break;
                }
                break;
            case dFgMngCmd_c::PLAN_DROPS:
                size = dFgMngCmd_c::SEND_SIZE_FULL;
                cmd.mHasDrops = 1;
                break;
            default:
                cmd.mLocal = 0;
                goto remote;
            }
            break;
        }
        if (fgMngProc_isSelfMember(player)) {
            sFgMngState.mPlayers[mPlayer].mState = dFgMngPlayerState_c::STATE_ACCEPTED;
        }
    } else {
    remote:
        if (fgMngProc_isSelfMember(player)) {
            sFgMngState.mPlayers[mPlayer].mState = dFgMngPlayerState_c::STATE_REJECTED;
        }
    }
    fgMngCmd_apply(&cmd, size);
    if (!mSend) {
        if (player != fn_800DCF58()) {
            fn_800DD4C8();
            fn_800DD518(&cmd, size);
            fn_800DD588(FG_MNG_PACKET_CMD, player);
        }
    } else {
        fn_800DD4C8();
        fn_800DD518(&cmd, size);
        fn_800DD588(FG_MNG_PACKET_CMD, FG_MNG_NET_ALL);
    }
}

// 800A4E10
dFgMngCmdEntry_c::~dFgMngCmdEntry_c() {}

// 800A4E50
int dFgMngCmd_c::searchSpot(dFdAsPos_c *pos, int dir, dFdBase_c *fd) {
    const int *order;
    int i;
    nw4r::math::VEC3 center;
    dFdBase_c::getUnitCenterPos(&center, pos->mX, pos->mZ);
    int ground = dBGCF::getUnitQuarter((const mVec3_c *)&center);
    BOOL blocked = FALSE;
    order = sSpotOrder[dir];
    for (i = 0; i < 8; i++, order++) {
        dFdAsPos_c p;
        u8 d = sDirOffset[*order];
        s32 dx = (d >> 4) - 8;
        s32 dz = (d & 0xF) - 8;
        p.mX = pos->mX + dx;
        p.mZ = pos->mZ + dz;
        if (fgMngLock_find(mScene, (dFdAsPos_c *)&p, 0) < 0) {
            dItem::Item *cur = fd->getItem(p.mX, p.mZ, 0);
            if (cur != NULL && cur->mId == dItem::ITEM_ID_NONE && fd->canPutItem(p.mX, p.mZ)) {
                dBGCF::setCurrentBg(1);
                BOOL found = dBGCF::checkWalkable(pos->mX, pos->mZ, ground, p.mX, p.mZ);
                dBGCF::setCurrentBg(0);
                if (!found) {
                    dItem::Item *a = NULL;
                    dItem::Item *b = NULL;
                    switch (*order) {
                    case 4:
                        a = fd->getItem(p.mX - 1, p.mZ, 0);
                        b = fd->getItem(p.mX, p.mZ + 1, 0);
                        break;
                    case 5:
                        a = fd->getItem(p.mX + 1, p.mZ, 0);
                        b = fd->getItem(p.mX, p.mZ + 1, 0);
                        break;
                    case 7:
                        a = fd->getItem(p.mX - 1, p.mZ, 0);
                        b = fd->getItem(p.mX, p.mZ - 1, 0);
                        break;
                    case 8:
                        a = fd->getItem(p.mX + 1, p.mZ, 0);
                        b = fd->getItem(p.mX, p.mZ - 1, 0);
                        break;
                    }
                    dBGCF::groundChk_c g1(p.mX, p.mZ, dBGCF::LAYER_TOP, 0, 0);
                    f32 h1 = g1.getHeight(FALSE);
                    dBGCF::groundChk_c g0(pos->mX, pos->mZ, dBGCF::LAYER_TOP, 0, 0);
                    f32 diff = g0.getHeight(FALSE) - h1;
                    if (diff >= -10.0f && diff <= 10.0f) {
                        if (a != NULL && a->mId < dItem::FG_COUNT) {
                            found = TRUE;
                        } else if (b != NULL && b->mId < dItem::FG_COUNT) {
                            found = TRUE;
                        }
                    }
                }
                if (found && fgMngTask_find(mScene, &p, 0) < 0 && !isPlanned(p.mX, p.mZ)) {
                    *pos = p;
                    return SPOT_FOUND;
                }
            }
        } else {
            int idx = fgMngTask_find(mScene, &p, 0);
            if (idx >= 0) {
                if (sFgMngTaskList.mTasks[idx].mDropIdx > 2) {
                    blocked = TRUE;
                }
            } else {
                blocked = TRUE;
            }
        }
    }
    u8 d = sDirOffset[sSpotOrder[dir][0]];
    s32 dx = (d >> 4) - 8;
    s32 dz = (d & 0xF) - 8;
    pos->mX += dx;
    pos->mZ += dz;
    switch (blocked) {
    case FALSE:
        break;
    default:
        return SPOT_BLOCKED;
    }
    return SPOT_NONE;
}

// 800A5278: the fruit a fruit tree drops when shaken.
u16 dFgMngCmd_c::getFruit(u16 id) {
    return dItem::Item(sFruit[dItem::Item(id).getFruitTreeType()]).mId;
}

// 800A52BC
BOOL dFgMngCmd_c::planFruit(int player, u16 id) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return TRUE;
    }
    int num;
    switch (id) {
    case dItem::FG_PALM_FRUIT:
        num = 2;
        break;
    default:
        num = 3;
        break;
    }
    u16 fruit = getFruit(id);
    dFdAsPos_c none(-1, -1);
    dFdAsPos_c pos[3];
    int res[3];
    for (int i = 0; i < num; i++) {
        u8 d = sDirOffset[sFruitDir[i]];
        const u16 p = mPos;
        s32 dx = (d >> 4) - 8;
        s32 dz = (d & 0xF) - 8;
        pos[i] = dFdAsPos_c((p >> 8) + dx, (p & 0xFF) + dz);
        res[i] = searchSpot(&pos[i], i, fd);
        switch (res[i]) {
        case SPOT_NONE:
            mDropPos[i].mPos = FG_MNG_POS_NONE;
            break;
        case SPOT_FOUND:
            if (player == fn_800DCF58()) {
                fgMngTask_add(player, 0, pos[i], none, fruit, dItem::ITEM_ID_NONE, FG_MNG_KIND_DROP, 0, 0, i, 0, -1);
            }
            mDropPos[i].set(pos[i].mX, pos[i].mZ);
            break;
        default:
            for (int j = 0; j < i; j++) {
                fgMngTask_remove(player, &pos[j]);
            }
            return FALSE;
        }
    }
    mDropItem = fruit;
    return TRUE;
}

// 800A54B0: the one item a shaken tree drops (a random item, or money), beside the tree on the
// player's side.
BOOL dFgMngCmd_c::planShakeItem(dFgMngReq_c *req, int player) {
    dFdBase_c *fd;
    u16 item;
    int base = 0;
    nw4r::math::VEC3 at;
    dFdAsPos_c pos;
    fd = fn_80190C44(FD_ID_TOWN);
    if (fd == NULL) {
        return TRUE;
    }
    switch (req->mTarget) {
    case dItem::FG_TREE_FTR:
    case dItem::FG_TREE_BEES:
    case dItem::FG_CEDAR_FTR:
    case dItem::FG_CEDAR_BEES: {
        dItem::Item got;
        dFgMngItemFilter_c filter;
        filter = lbl_8059FF80;
        if (*(u16 *)&dSaveData_c::getTown()->_05EC68[0xC] & 0x10) {
            filter.mNum = 8;
        }
        int range[2];
        range[0] = 3;
        range[1] = 0;
        fn_800C60B4(&got, 1, range, 1, &filter, 0, 0, 0);
        item = got.mId;
        break;
    }
    case dItem::FG_TREE_BELLS:
    case dItem::FG_CEDAR_BELLS:
    default:
        if (((u8 *)dPlayerMgr_c::getCurrentPlayer())[0x83F9] == 3) {
            item = dItem::Item(dItem::ITEM_IDX_300_BELLS).mId;
        } else {
            item = dItem::Item(dItem::ITEM_IDX_100_BELLS).mId;
        }
        base = 3;
        break;
    }
    fgMngProc_getUnitGroundPos(&at, req->mPos >> 8, req->mPos & 0xFF);
    int idx = base + fgMngState_isPlayerLeftOf(player, &at.x);
    u8 d = sDirOffset[sFruitDir[idx]];
    const u16 p = mPos;
    s32 dx = (d >> 4) - 8;
    s32 dz = (d & 0xF) - 8;
    pos = dFdAsPos_c((p >> 8) + dx, (p & 0xFF) + dz);
    int res = searchSpot(&pos, idx, fd);
    dFdAsPos_c none(-1, -1);
    switch (res) {
    case SPOT_NONE:
        mDropPos[0].mPos = FG_MNG_POS_NONE;
        break;
    case SPOT_FOUND:
        if (player == fn_800DCF58()) {
            fgMngTask_add(player, 0, pos, none, item, dItem::ITEM_ID_NONE, FG_MNG_KIND_DROP, 0, 0, 0, 0, -1);
        }
        mDropPos[0].set(pos.mX, pos.mZ);
        break;
    default:
        return FALSE;
    }
    mDropItem = item;
    return TRUE;
}

// 800A5738
int dFgMngCmd_c::plan(dFgMngReq_c *req, int player) {
    u16 id;
    int res = PLAN_NO_DROPS;
    switch (req->mKind) {
    case FG_MNG_KIND_SHAKE_TREE:
    case FG_MNG_KIND_CHOP_TREE: {
        id = req->mTarget;
        dItem::Item item;
        mDropPos[0].mPos = FG_MNG_POS_NONE;
        mDropPos[1].mPos = FG_MNG_POS_NONE;
        mDropPos[2].mPos = FG_MNG_POS_NONE;
        item = req->mTarget;
        dItem::FgInfo *info = item.getFgInfo();
        if (info != NULL && (u16)(int)info->mDropItem != dItem::ITEM_ID_NONE) {
            if (planFruit(player, req->mTarget)) {
                res = PLAN_DROPS;
            } else {
                res = PLAN_FAILED;
            }
        } else if (id == dItem::FG_TREE_BEES || id == dItem::FG_CEDAR_BEES) {
            if (fgMngProc_canEditField()) {
                if (planShakeItem(req, player)) {
                    res = PLAN_DROPS;
                } else {
                    res = PLAN_FAILED;
                }
            }
        } else if (id == dItem::FG_TREE_FTR || id == dItem::FG_CEDAR_FTR || id == dItem::FG_TREE_BELLS || id == dItem::FG_CEDAR_BELLS) {
            BOOL ok = planShakeItem(req, player);
            res = PLAN_FAILED;
            if (ok) {
                res = PLAN_DROPS;
            }
        }
        break;
    }
    }
    return res;
}

// 800A588C
BOOL dFgMngCmd_c::isPlanned(int x, int z) {
    for (int i = 0; i < 3; i++) {
        if (x == mDropPos[i].mPos >> 8 && z == (mDropPos[i].mPos & 0xFF)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800A5900
void dFgMngProc_c::createHeap() {
    if (m_heap == NULL) {
        m_heap = mHeap::createFrmHeap(FG_MNG_BUFFER_SIZE, lbl_8074E3F8, "dFgMngProc_c::m_heap", 0x20, mHeap::OPT_NONE);
        mBuffer = m_heap->alloc(FG_MNG_BUFFER_SIZE, 0x20);
    }
}

// 800A597C
int fn_800A597C() {
    return 0;
}
