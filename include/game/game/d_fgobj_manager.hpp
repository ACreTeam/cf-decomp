#pragma once

// d_fgobj_managerNP (REL, profile FGOBJ_MANAGER): the field objects that play
// the short-lived animations of field units (falling fruit, felled trees, dug
// holes, shaken trees, plants swaying ...) and the API the player / NPCs /
// other RELs use to request field changes (dFgMngReq_c, see d_fg_mng_task.hpp).
// Class names from the RTTI (all at global scope): dFgObjMng_c, dFgObj_c,
// dFgObjAnm_c (+ dFgObjAnm_c::mdlCallback_c), dFgObjSwingPlant_c, dFgDecoAnm_c.
// Everything else (members, functions) is inferred.
// Class order: MWCC emits the vtables in reverse class-completion order.

#include <types.h>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_snd_obj.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <game/framework/f_base.hpp> // after m_3d: fBase_c's RTTI follows this TU's classes

class dFdBase_c;
class dFgDecoAnm_c;
struct dFdAsPos_c;
struct dFgMngCmd_c;
class dFgMngState_c;
namespace nw4r {
namespace ef {
class Particle;
}
} // namespace nw4r

// A field object (0xBC). mType selects its proc (sProcs in execute).
class dFgObj_c {
  public:
    dFgObj_c();  // .text 214
    ~dFgObj_c(); // .text 538

    virtual void setup(int idx); // .text A00
    virtual void remove();       // .text 7A4
    virtual void execute();      // .text B10
    virtual void draw();         // .text D30

    void clear() {
        mActive = FALSE;
        mPlayer = 4;
        mItem = mPrevItem = 0xFFF1;
        mType = 20;
        _28.x = 0.0f;
        _28.y = 0.0f;
        _28.z = 0.0f;
        mUnitX = mUnitZ = -1;
        mSpeed.x = 0.0f;
        mSpeed.y = 0.0f;
        mSpeed.z = 0.0f;
        mScale.x = 1.0f;
        mScale.y = 1.0f;
        mScale.z = 1.0f;
        mAngleY = 0;
        _56 = 0;
        mStep = 0;
        mProcTask = 0;
        mReleaseTask = 0;
        mDrawMode = 0;
    }

    void finishTask(); // .text EC8

    void init(int player, int x, int z, mVec3_c pos, int type, u16 item,
              dFdBase_c *fd, s16 angle, int arg,
              u8 layer); // .text F18
    void kill(); // .text 1134: finishes the unit task and removes the object
    void playLandSound(); // .text 1180
    void playDropSound(); // .text 12B8
    void setDraw(const mVec3_c *pos, const mVec3_c *scale,
                 u8 mode);                       // .text 1314
    void initDrop(int effect);                   // .text 15F0
    void initDigUp(dFdBase_c *fd, BOOL release); // .text 1838
    void initPull();                             // .text 19D8
    void initDigHole(dFdBase_c *fd);             // .text 1A6C
    void initPickUp();                           // .text 1B18
    void initToss(BOOL quiet);                   // .text 1BC8
    void initSlide();                            // .text 1C98
    void initKnockOff(dFdBase_c *fd);            // .text 1D58
    void initBuryMoney();                        // .text 1E38
    void initFillHole(dFdBase_c *fd);            // .text 1F74
    void initClear(dFdBase_c *fd);               // .text 2010
    void initDigHoleTask(dFdBase_c *fd);         // .text 2078
    void initThrow();                            // .text 214C
    void procDrop();                             // .text 3604
    void procGrow();                             // .text 382C
    void procShrink();                           // .text 38A4
    void procShrinkFast();                       // .text 3A10
    void procPickUp();                           // .text 3B4C
    void procPop();                              // .text 3C10
    void procThrow();                            // .text 3C88
    void procToss();                             // .text 3F60
    void procSlide();                            // .text 413C
    void procKnockOff();                         // .text 4238
    void procBuryMoney();                        // .text 4318
    void procFillHole();                         // .text 442C
    void procSpin();                             // .text 44EC

    /* 0x04 */ BOOL mActive;
    /* 0x08 */ u8 mIdx;
    /* 0x0C */ int mPlayer; // the player whose request started it
    /* 0x10 */ u16 mItem;
    /* 0x12 */ u16 mPrevItem;
    /* 0x14 */ int mType;
    /* 0x18 */ mVec3_c mSndPos;
    /* 0x24 */ f32
        mParam; // ground height (drop on furniture) / animation parameter
    /* 0x28 */ mVec3_c _28;
    /* 0x34 */ int mUnitX;
    /* 0x38 */ int mUnitZ;
    /* 0x3C */ mVec3_c mSpeed;
    /* 0x48 */ mVec3_c mScale;
    /* 0x54 */ s16 mAngleY;
    /* 0x56 */ s16 _56;
    /* 0x58 */ mVec3_c mPos;
    /* 0x64 */ u8 mLayer;
    /* 0x66 */ s16 mStep;  // the step within the current proc
    /* 0x68 */ u16 mFrame; // frame counter of the grow / pop animations
    /* 0x6A */ s16 mTimer;
    /* 0x6C */ s16 mSeCount;    // sounds played by the shake animation
    /* 0x6E */ u8 mProcTask;    // EC8: fgMngTask_procAt the unit
    /* 0x6F */ u8 mReleaseTask; // EC8: fgMngTask_releaseAt the unit
    /* 0x70 */ dSndObjSimple_c mSnd;
    /* 0xA0 */ mVec3_c mDrawPos;
    /* 0xAC */ mVec3_c mDrawScale;
    /* 0xB8 */ u8 mDrawMode;
}; // size 0xBC

// A field object with a model and animations (0x31C).
class dFgObjAnm_c : public dFgObj_c {
  public:
    // The effects of an animated object (0x13C).
    class effect_c {
      public:
        effect_c() {
            mCount = -1;
            mSnowCount = -1;
        }

        void start(int treeAnm, int kind, int season, int effKind,
                   const mVec3_c *pos, BOOL follow);  // .text 5198
        void update(const mVec3_c *pos, int effKind); // .text 54F0
        void restart(); // .text 5670: resets the running effect countdowns

        /* 0x000 */ dLevelEffect_c mEffect;     // the leaves
        /* 0x094 */ dLevelEffect_c mSnowEffect; // the snow on the tree
        /* 0x128 */ mVec3_c mScale;
        /* 0x134 */ s8 mCount; // frames mEffect is still created for, -1: none
        /* 0x135 */ s8 mSnowCount; // the same for mSnowEffect
        /* 0x136 */ u8 mName;      // sTreeEffectNames index
        /* 0x137 */ u8 mSnowName;  // sTreeSnowEffectNames index, 3: none
        /* 0x138 */ u8 mIdx; // the effect kind + the tree animation's offset
    }; // size 0x13C

    class mdlCallback_c : public m3d::mdl_c::callback_c {
      public:
        mdlCallback_c();          // .text 3494
        virtual ~mdlCallback_c(); // .text 34AC
        virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip,
                             nw4r::g3d::ResMdl resMdl);

        /* 0x04 */ dFgObjAnm_c *mpOwner;
        /* 0x08 */ ulong mNodeId; // the node whose top timingB tracks
        /* 0x0C */ f32 mOffsetY;  // the node's height above its origin
    }; // size 0x10

    dFgObjAnm_c();  // .text 2D4
    ~dFgObjAnm_c(); // .text 594

    virtual void setup(int idx); // .text A08
    virtual void remove();       // .text 828
    virtual void execute();      // .text B70
    virtual void draw();         // .text DF0

    void initTree(int kind);                      // .text 22D8
    void initHoneycomb();                         // .text 2DB4
    void initFellTree(dFdBase_c *fd);             // .text 2F2C
    void startEffects(BOOL follow);               // .text 327C
    void updateEffects(BOOL restart);             // .text 344C
    void dropShakeItems(const dItem::Item *tree); // .text 4B5C
    void endLoop(); // .text 4900: the shake animation stops looping
    void updatePrevItem(const dFdAsPos_c *pos); // .text 492C
    void cancelTree();                          // .text 4994
    void finishTree();                          // .text 4A3C
    void procTree();                            // .text 45F4
    void procHoneycomb();                       // .text 4B74
    void procFellTree();                        // .text 4DE8
    void drawTree();                            // .text 4A9C
    void drawHoneycomb();                       // .text 4DD8
    void drawFellTree();                        // .text 4F0C

    /* 0x0BC */ m3d::mdl_c mMdl;
    /* 0x0FC */ m3d::anmChr_c mAnmChr;
    /* 0x134 */ m3d::anmMatClr_c mAnmClr;
    /* 0x160 */ m3d::anmVis_c mAnmVis;
    /* 0x198 */ mAllocator_c mAllocator;
    /* 0x1B4 */ int mTreeAnm;  // initTree's kind: 0..3 shake, 4..7 chop
    /* 0x1B8 */ int mHideSlot; // dFgDraw::hideUnit's slot, -1: none
    /* 0x1BC */ dFgDecoAnm_c
        *mDeco; // the lights of a decorated cedar (FG_CEDAR_LIGHTS)
    /* 0x1C0 */ effect_c mEffect;
    /* 0x2FC */ mdlCallback_c mCallback;
    /* 0x30C */ mVec3_c
        mTopPos; // the tracked node's top (field position), set by mCallback
    /* 0x318 */ u8 mEffKind; // the tree's effect kind: stage - 1 (+4 palm-like,
                             // +8 cedar-like)
}; // size 0x31C

// A plant that sways (0xE8).
class dFgObjSwingPlant_c : public dFgObj_c {
  public:
    dFgObjSwingPlant_c();  // .text 3F4
    ~dFgObjSwingPlant_c(); // .text 6A0

    virtual void setup(int idx); // .text A7C
    virtual void remove();       // .text 8DC
    virtual void execute();      // .text BF4
    virtual void draw();         // .text E38

    void initSwing(); // .text 314C
    void calcSwing(); // .text 4F1C
    void drawSwing(); // .text 513C
    void endSwing();  // .text 514C

    /* 0x0BC */ m3d::smdl_c mMdl;
    /* 0x0C8 */ mAllocator_c mAllocator;
    /* 0x0E4 */ int mHiddenUnit; // dFgDraw::hideUnit handle (showUnit on end)
}; // size 0xE8

// A decoration animation (0x118).
class dFgDecoAnm_c {
  public:
    dFgDecoAnm_c();  // .text 440
    ~dFgDecoAnm_c(); // .text 714

    virtual void setup(int idx); // .text ACC
    virtual void remove();       // .text 93C
    virtual void execute();      // .text C3C
    virtual void draw();         // .text E4C

    void setTreeAnm(int kind);          // .text 286C
    void endTreeAnm();                  // .text 2BC0
    void init(int x, int z, f32 frame); // .text 2C60
    BOOL isPlayingTreeAnm();            // .text 2D58
    void endLoop(); // .text 4920: the light animation stops looping

    BOOL isEnding() const {
        return mEndCount != 0;
    }

    void clear() {
        mActive = FALSE;
        mItem = 0xFFF1;
        mUnitX = mUnitZ = -1;
        mIsCut = 0;
        _20 = 0;
    }

    /* 0x004 */ BOOL mActive;
    /* 0x008 */ u8 mIdx;
    /* 0x00A */ u16 mItem;
    /* 0x00C */ mVec3_c mPos;
    /* 0x018 */ int mUnitX;
    /* 0x01C */ int mUnitZ;
    /* 0x020 */ s16 _20;
    /* 0x022 */ s16 mIsCut; // the tree animation is a cut one (kind >= 4)
    /* 0x024 */ m3d::mdl_c mMdl;
    /* 0x064 */ m3d::anmChr_c mAnmChr;
    /* 0x09C */ m3d::anmMatClr_c mAnmClr;
    /* 0x0C8 */ m3d::anmTexSrt_c mAnmSrt;
    /* 0x0F4 */ mAllocator_c mAllocator;
    /* 0x110 */ int mTreeAnmKind; // setTreeAnm kind (-1: none)
    /* 0x114 */ u8 mEndCount; // frames since it was told to end (removed at 2)
}; // size 0x118

// The manager (profile FGOBJ_MANAGER, 0xE5E0). sFgObjMgr (d_field_assessment)
// points at it while it exists.
class dFgObjMng_c : public fBase_c {
  public:
    enum {
        OBJ_NUM = 0x30,
        ANM_NUM = 0x20,
        SWING_NUM = 0x1E,
        DECO_NUM = 0x3C,
    };

    dFgObjMng_c() {
    }
    virtual ~dFgObjMng_c(); // .text BDC8

    virtual int create();   // .text 5798
    virtual int doDelete(); // .text 5B5C
    virtual int execute();  // .text 5940
    virtual int draw();     // .text 5A48

    dFgObjAnm_c *getAnmObj(int i) {
        return &mAnmObjs[i];
    }
    dFgObj_c *getFreeObj();                // .text 5CA4
    dFgObjAnm_c *getFreeAnmObj();          // .text 5CD0
    dFgObjSwingPlant_c *getFreeSwingObj(); // .text 5CFC
    dFgDecoAnm_c *getFreeDecoObj();        // .text 5D2C

    BOOL spawn(int player, int type, u16 item, int x, int z, mVec3_c pos,
               s16 angle, int arg,
               u8 layer);                // .text 5D74
    void getDrawItem(dItem::Item *item); // .text 67D8
    void addFgDraw(int id, const mVec3_c *pos, const mVec3_c *scale,
                   mAng3_c *ang, BOOL present); // .text 68B0
    void startDecoAnm(int x, int z, f32 frame); // .text 7C18
    void endDecoAnm(int x, int z);              // .text 7CD4
    int findDecoIdx(int x, int z);              // .text 7D2C
    dFgDecoAnm_c *findDeco(int x, int z);       // .text 7E30
    void addSwing();                            // .text 5770
    void removeSwing();                         // .text 5784

    void procCommands();         // .text B464
    void swayPlantsNearPlayer(); // .text 82F8

    /* 0x0064 */ dFgObj_c mObjs[OBJ_NUM];
    /* 0x23A4 */ dFgObjAnm_c mAnmObjs[ANM_NUM];
    /* 0x8724 */ dFgObjSwingPlant_c mSwingObjs[SWING_NUM];
    /* 0xA254 */ dFgDecoAnm_c mDecoObjs[DECO_NUM];
    /* 0xE3F4 */ EGG::FrmHeap *mAnmHeaps[ANM_NUM];
    /* 0xE474 */ EGG::FrmHeap *mSwingHeaps[SWING_NUM];
    /* 0xE4EC */ EGG::FrmHeap *mDecoHeaps[DECO_NUM];
    /* 0xE5DC */ int mSwingNum; // swaying plants alive (addSwing / removeSwing)
}; // size 0xE5E0

namespace dFgobjManager {

EGG::FrmHeap *getAnmHeap(int idx);   // .text 56BC
EGG::FrmHeap *getSwingHeap(int idx); // .text 56F8
EGG::FrmHeap *getDecoHeap(int idx);  // .text 5734

void moveOffWater(mVec3_c *pos, BOOL onWater); // .text 134C

void dropFruit(const dItem::Item *tree, int player, int x, int z); // .text 6094
void dropHoneycomb(int player, int x, int z);                      // .text 6278
void dropShakeItem(const dItem::Item *tree, int player, int x,
                   int z); // .text 6360
void dropTreeItems(const dItem::Item *tree, int player, int x,
                   int z); // .text 65A4
// .text 6770: the fgMngProc_setFgObjCallback callback (d_hand_item passes the
// pointers as ints)
void fgObjCallback(int item, int pos, int scale, mAng3_c *ang, int present);
int getTreeAnmKind(const dItem::Item *tree, int a, int b); // .text 6948
void spawnFruitDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
                    s16 drop, int idx); // .text 6DF4
void spawnShakeItemDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
                        s16 drop);                            // .text 6F78
void spawnHoneycomb(int player, const mVec3_c *pos);          // .text 7000
BOOL spawnDigUp(int player, u16 item, int x, int z, int arg); // .text 6ACC
BOOL spawnDigHole(int player, int x, int z);                  // .text 6BEC
BOOL landBalloonItem(u16 item, const mVec3_c *pos); // .text 6E80 (d_a_balloon)
void spawnFellTree(int player, u16 item, int x, int z, s16 param); // .text 7070
BOOL spawnFillHole(int player, int x, int z);                      // .text 7188
void spawnClear(int player, u16 item, int x, int z, s16 param);    // .text 7220
void spawnDropAt(int player, u16 item, int x, int z, const mVec3_c *pos,
                 int arg);                                 // .text 72A4
void procChopResult(int x, int z, s16 param);              // .text 732C
BOOL hasStumpTask(int player, int x, int z);               // .text 74A4
BOOL checkTreeVisible(int player, int x, int z, int kind); // .text 751C
int restartTreeAnm(int player, int x, int z, int kind);    // .text 7660
BOOL startShakeTree(int player, int x, int z, int kind);   // .text 7784
BOOL stopShakeTree(int player, int x, int z);              // .text 78A4
BOOL startChopTree(int player, int x, int z, int kind);    // .text 7968
void startDecoAnm(int x, int z, f32 frame); // .text 7BC4 (d_fg_draw)
void endDecoAnm(int x, int z);              // .text 7C90 (d_fg_draw)
inline BOOL isDecoAt(const dFgDecoAnm_c *obj, int x, int z) {
    BOOL ret = FALSE;
    if (obj->mUnitX == x && obj->mUnitZ == z) {
        ret = TRUE;
    }
    return ret;
}
BOOL killObjAt(int x, int z); // .text 5F14
BOOL spawnDrop(int player, u16 item, int x, int z, const mVec3_c *pos,
               u8 layer);    // .text 69D0
f32 getFgAnmParam(u16 item); // .text 6A58: FgInfo byte 0x08
BOOL spawnPickUp(int player, u16 item, int x, int z);        // .text 6C84
BOOL spawnPickUpSpecial(int player, u16 item, int x, int z); // .text 6D64
void spawnBuryMoney(int player, u16 item, int x, int z,
                    const mVec3_c *pos); // .text 70F4
BOOL swayPlantAt(int player, int x, int z, const mVec3_c *pos,
                 f32 radius); // .text 7FAC

inline BOOL isObjAt(const dFgObj_c *obj, int x, int z) {
    BOOL ret = FALSE;
    if (obj->mUnitX == x && obj->mUnitZ == z) {
        ret = TRUE;
    }
    return ret;
}

inline f32 getDistXZSq(const mVec3_c &a, const mVec3_c &b) {
    return (a - b).xzLenSq();
}

BOOL isSwingPlantAt(int x, int z); // .text 83C8 (also d_insect_field)
void rippleParticleCallback(nw4r::ef::Particle *particle,
                            u32 arg);                         // .text 84EC
void procTaskAt(u32 scene, const dFdAsPos_c *pos, u32 layer); // .text 85DC
void procCmdTasks(dFgMngCmd_c *cmd);                          // .text 8678
void applyCmdToField(dFgMngCmd_c *cmd);                       // .text 8778
void procCmd(dFgMngCmd_c *cmd);                               // .text 889C
void procRemoteCmd(dFgMngCmd_c *cmd);                         // .text 8934
void playCmdEffect(dFgMngCmd_c *cmd);                         // .text 8BC8
int requestAction(dFgMngState_c *state, int player, int kind, dFdAsPos_c pos,
                  u16 item, u16 baseItem, u8 stage, u8 layer, int b2,
                  s8 day);        // .text 8C30
void sendRequest(u8 idx, int b2); // .text 8D8C
int useTool(int player, int x, int z, int tool, int b2,
            u16 heldItem);                                     // .text 90C4
int useToolAtTask(int player, int tool, int b2, u16 heldItem); // .text 91E8
int shakeTree(int player, const dItem::Item *item, const dFdAsPos_c *pos,
              int b2); // .text 9274
int chopTree(int player, const dItem::Item *item, const dFdAsPos_c *pos,
             int b2); // .text 94AC
int dig(int player, dFdBase_c *fd, const dItem::Item *item, dFdAsPos_c pos,
        u16 heldItem, int b2); // .text 9880
int water(int player, const dItem::Item *item, const dFdAsPos_c *pos,
          int b2);                              // .text 9CE8
int pickUp(int player, int x, int z);           // .text 9D80
int pickUpTown(int player, int x, int z);       // .text 9DFC
int pickUpRoom(int player, int x, int z);       // .text A088
int fillHole(int player, int x, int z, int b2); // .text A204
int requestPitfall(int player, int x, int z);   // .text A300
int requestNpcPitfall(int npc, int x, int z);   // .text A424 (d_a_npc_out)
int getRequestResult(int idx);                  // .text A548 (d_a_npc_out)
int putItem(int player, int kind, u16 item);    // .text A788
int putHeldItem(int player, u16 item);          // .text A878
int swapItem(int player, u16 item);             // .text A8D4
int putItemAside(int player, u16 item);         // .text A994
int putDesign(int player, int slot);            // .text AA00 (d_menu_desi00)
int playPutEffect(int idx);                     // .text AA54
void clearRequest(int idx);                     // .text ACF8 (d_a_npc_out)
dItem::Item *getPutSpotItem(int kind, dFdBase_c *fd, int x, int z,
                            const mVec3_c *playerPos); // .text AD14
BOOL findPutSpot(int kind, int *outX, int *outZ,
                 u8 *outLayer); // .text AEA4 (d_fish_field)
BOOL findDropSpot(const mVec3_c *pos, int *x,
                  int *z); // .text B178 (d_a_balloon)
void playGrassEffect(int x, int z, u16 id, const mAng3_c *ang,
                     BOOL wind);               // .text B4FC (d_a_player)
BOOL dropUfoPart(int unused, int z, u32 side); // .text B7EC (d_a_ufo)
BOOL isInCameraView(int x, int z, const mVec3_c *center); // .text BA98
BOOL isBadDropSpot(dFdBase_c *fd, int x, int z);          // .text BC54
void stepOnUnit(int player, const mVec3_c *pos); // .text BD1C (d_a_player)

// Functions called from other modules (names inferred there first).
BOOL requestPitfallEnter(int player, int x, int z); // .text 6B5C (d_a_npc_out)
BOOL requestPitfallExit(int player, int x, int z);  // .text 7414 (d_a_npc_out)

} // namespace dFgobjManager
