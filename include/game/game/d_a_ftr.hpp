#pragma once

// The furniture actor base (RTTI daFTR_c, profile 0x21); the furniture of d_a_ftr_act derive from it.

#include <game/mLib/m_3d.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_objc.hpp>
#include <game/game/d_rec_bank.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_str.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_mtx.hpp>
#include <nw4r/g3d.h>

namespace dFTR {

struct unitList_c {
    struct unit_c {
        int x;
        int z;
    };

    unitList_c() : mNum(0) {}

    BOOL add(int x, int z);
    unit_c *get(u32 i);

    /* 0x00 */ u32 mNum;
    /* 0x04 */ unit_c mUnit[4];
}; // size 0x24

struct ftrList_c {
    ftrList_c(unitList_c &units, BOOL layer);

    void clear();
    void set(unitList_c &units, BOOL layer);
    BOOL add(daFTR_c *ftr);
    daFTR_c *get(u32 i) { return mFtr[i & 3]; }

    /* 0x00 */ u32 mNum;
    /* 0x04 */ daFTR_c *mFtr[4];
}; // size 0x14

struct counter_c {
    void clear();
    void set(int count);
    BOOL calc();

    /* 0x0 */ int mCount;
}; // size 0x4

} // namespace dFTR

class daFTR_c : public dDemoActor_c, public dBGCF::mvbg_c, public dFTR::alwaysAc_c, public dMsg::Rcpt_c {
public:
    enum STATE_e {
        STATE_APPEAR,
        STATE_WAIT,
        STATE_ROTATE,
        STATE_PUSH,
        STATE_PULL,
        STATE_REMOVE,
        STATE_SHRINK,
        STATE_VANISHED,
        STATE_HIDDEN,
        STATE_TALK,
        STATE_TALK_END,
        STATE_DELETE,
        STATE_NUM,
    };

    struct flags_c {
        u32 _00 : 17;
        u32 mSound : 1;
        u32 mIcon : 1;
        u32 mDraw : 1;
        u32 mAppearSmoke : 1;
        u32 mAppearEffect : 1;
        u32 mSpoken : 1;
        u32 mSpeakDemo : 1;
        u32 mSpeak : 1;
        u32 mToggle : 1;
        u32 mShadow : 1;
        u32 mLamp : 1;
        u32 mCreated : 1;
        u32 mDeleteReady : 1;
        u32 mRemoved : 1;
        u32 mInited : 1;
    };

    union info_c {
        u32 mValue;
        struct {
            u32 mItem : 16;
            u32 mX : 5;
            u32 mRot : 2;
            u32 mLayer : 1;
            u32 mZ : 6;
            u32 mKind : 2;
        };
    };

    union demoIdA_u {
        u16 mValue;
        struct {
            u16 mHandle : 9;
            u16 mScene : 7;
        };
    };

    union demoIdB_u {
        u16 mValue;
        struct {
            u16 mX : 4;
            u16 mZ : 4;
            u16 mLayer : 1;
            u16 mScene : 6;
        };
    };

    class nodeCallBack_c : public m3d::mdl_c::callback_c {
    public:
        virtual ~nodeCallBack_c() {}
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl);

        /* 0x04 */ ulong mHourNode;
        /* 0x08 */ ulong mMinNode;
        /* 0x0C */ int mFtrFunc;
    }; // size 0x10

    class loader_c : public dItem::resLoader_c {
    public:
        virtual void onLoaded();

        BOOL load(dItem::Item item, void *heap) { return loadItem(item, heap); }
    };

    class lamp_c : public dSTR::light_c {
    public:
        lamp_c();

        BOOL init(nw4r::g3d::ResMdl mdl, const char *matName, BOOL on);
        BOOL set(BOOL on, BOOL fade, BOOL flicker);
        void update();

        /* 0x14 */ nw4r::g3d::ResMat mMat;
    }; // size 0x18

    class lampList_c {
    public:
        BOOL init(nw4r::g3d::ResMdl mdl, BOOL on);
        BOOL set(BOOL on, BOOL fade, BOOL flicker);
        void update();
        f32 getValue();

        /* 0x00 */ lamp_c mLamp[6];
        /* 0x90 */ bool mActive;
    }; // size 0x91

    struct switchNode_c {
        void init(nw4r::g3d::ResMdl mdl, BOOL on);
        void set(BOOL on);
        BOOL isNode(u32 nodeId);
        BOOL isOn();

        /* 0x0 */ s8 mNode[4];
        /* 0x4 */ u8 mOn;
    }; // size 0x5

    struct topItem_c {
        topItem_c() { init(); }

        void init();
        BOOL set(dItem::Item item, const mVec3_c &pos);
        void draw(daFTR_c *ftr);
        dItem::Item getItem() { return mItem; }
        BOOL isEmpty() { return mItem.isSame(dItem::Item()); }

        /* 0x0 */ u8 _0[2];
        /* 0x2 */ dItem::Item mItem;
        /* 0x4 */ mVec3_c mPos;
    }; // size 0x10

    struct topItemList_c {
        topItemList_c() { init(); }

        void init();
        void draw(daFTR_c *ftr);
        BOOL add(dItem::Item item, const mVec3_c &pos);
        BOOL canGet(daFTR_c *ftr);
        BOOL pickUp(daFTR_c *ftr);
        void put(daFTR_c *ftr);
        topItem_c *get(u32 i) { return &mItem[i & 3]; }

        /* 0x00 */ u8 _00[4];
        /* 0x04 */ topItem_c mItem[4];
    }; // size 0x44

    typedef BOOL (daFTR_c::*initProc_t)();
    typedef void (daFTR_c::*executeProc_t)();

    using dActor_c::mPos;
    using dActor_c::mAngle;

    virtual ~daFTR_c();

    static void *operator new(size_t size);
    static void operator delete(void *p);

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int preExecute();
    virtual int draw();
    virtual void deleteReady();
    virtual void onDemoCmd(int cmd, u8 member);
    virtual mVec3_c *getDemoPosition();

    virtual int getLayer();
    virtual BOOL canWalkOn();
    virtual BOOL isSyncDisabled();
    virtual f32 getTopY() const;
    virtual int getTopKind();
    virtual int getFtrFunc();
    virtual dItem::Item getItem();
    virtual mVec3_c getScale() const { return mScale; }
    virtual int getSwitchParam() { return mSwitchParam; }
    virtual void onSwitchSent() { mSwitchWait = TRUE; }
    virtual BOOL getSwitch();
    virtual const dItem::BITM *getBITM() { return mpBITM; }
    virtual dItem::NpcMsg *getNpcMsg();
    virtual BOOL isMusic();
    virtual BOOL isLamp();
    virtual void changeSwitch(u32 sw, BOOL keep);
    virtual BOOL isIdle() { return mState == STATE_WAIT; }
    virtual BOOL isWait() { return mState == STATE_WAIT; }
    virtual BOOL startRotate(const mAng &angle);
    virtual BOOL startPush(const mAng &angle);
    virtual BOOL startPull(const mAng &angle);
    virtual int sendEvent(int state);
    virtual BOOL startHold();
    virtual BOOL canRotate(const mAng &angle);
    virtual BOOL canMove(const mVec3_c *dir);
    virtual BOOL tryMove(const mVec3_c *dir, mAng angle);
    virtual BOOL canSetSwitch() { return mFlags.mCreated && !mFlags.mRemoved; }
    virtual void onRemove() {}
    virtual BOOL onMoveStart(BOOL push) { return FALSE; }
    virtual void onRotateStart(BOOL left) {}
    virtual void onRotate(BOOL left) {}
    virtual BOOL initModel() { return TRUE; }
    virtual BOOL loadRes() { return TRUE; }
    virtual BOOL createRes() { return TRUE; }
    virtual nw4r::g3d::ResMdl getResMdl();
    virtual BOOL executeMain() { return TRUE; }
    virtual BOOL executeIdle() { return TRUE; }
    virtual BOOL isDraw() { return mFlags.mDraw; }
    virtual void calcMdl(const mMtx_c *mtx);
    virtual void entryMdl();
    virtual void drawExtra() {}
    virtual BOOL deleteRes() { return TRUE; }
    virtual void onHit(const dBGCF::wall_c *wall, int ownerId, f32 dist);
    virtual BOOL getUnit(int *x, int *z, const mVec3_c *ofs, mAng angle);
    virtual void onMessageStart(int arg);
    virtual void onMessageEnd(int kind);
    virtual BOOL setSwitch(int sw, int param);
    virtual int getSwitchBit(int sw);

    static const dItem::BITM *getItemBITM(dItem::Item item) {
        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
        if (bitm == NULL) {
            bitm = dItem::infoBank_c::get()->getBITM((u16)0x640);
        }
        return bitm;
    }

    mAng getTargetAngle() const { return mTargetAngle; }
    BOOL isSpoken() const { return mFlags.mSpoken; }
    void setTargetAngle(mAng angle) { mTargetAngle = angle; }
    int getKind();
    int getFtrSize();
    int getItemKind();
    BOOL isDummy();
    int getFtrLamp();
    EGG::FrmHeap *getHeap();
    static u32 makeInfo(int x, int z, const dItem::Item &item, int rot, int layer, int kind);
    void initSwitch();
    void applySwitch();
    void saveSwitch();
    BOOL isSwitchChanging();
    void toggleSwitch(BOOL keep);
    u32 getAnmFrameMax(u32 idx, int types);
    void createAnm(f32 rate, u32 idx, int playMode, u32 frame, int types);
    void changeAnm(f32 rate, u32 idx, int playMode, u32 frame, int types);
    void setAnmEnd(f32 rate, u32 idx, int playMode, int types);
    bool createAnmChr(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode, u32 frame);
    void changeAnmChr(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode, u32 frame);
    void setAnmChrEnd(f32 rate, nw4r::g3d::ResAnmChr anm, int playMode);
    BOOL playAnm(f32 rate, int types);
    BOOL isAnmNum(u32 num, int types);
    f32 getAnmFrame();
    BOOL isParentIdle();
    void startShrink();
    void executeState();

    BOOL initAppear();
    void executeAppear();
    BOOL initWait();
    void executeWait();
    BOOL initRotate();
    void executeRotate();
    BOOL initPush();
    void executePush();
    BOOL initPull();
    void executePull();
    BOOL initRemove();
    void executeRemove();
    BOOL initShrink();
    void executeShrink();
    BOOL initHidden();
    void executeHidden();
    BOOL initTalk();
    void executeTalk();
    BOOL initTalkEnd();
    void executeTalkEnd();
    BOOL initDelete();
    void executeDelete();

    void executeChildren();
    void speak();
    void setDpdBox();
    int getWallSlot();
    void executeHook();
    BOOL initFtr();
    BOOL loadModel();
    BOOL createModel();
    void initPos();
    void pushPlayer(dActor_c *player);
    void addBg();
    void calcBg();
    void removeBg();
    void calcMtx(mMtx_c *mtx, const mVec3_c *ofs, mAng angle, BOOL target);
    void setMtx(BOOL noMdl);
    void getBgSize(mVec3_c *size);
    void multMtx(mVec3_c *out, const mVec3_c *in);
    void getCorner(mVec3_c *out, u32 i);
    void getCenter(mVec3_c *out);
    void clearUnit();
    BOOL setUnit(const mVec3_c *ofs, mAng angle);
    void getUnits(dFTR::unitList_c *list, const mVec3_c *ofs, mAng angle = mAng(0), mMtx_c *mtx = NULL);
    dItem::Item getDirItem(const mAng &angle) const;
    void putUnits(bool send, const mVec3_c *ofs = NULL, mAng angle = mAng(0), BOOL setDemo = FALSE);
    void removeUnits(BOOL send, BOOL upper);
    dItem::Item getPutItem();
    void moveUnits(const mVec3_c *ofs, mAng angle);
    static BOOL isNear(const mVec3_c *pos, const mVec3_c *other, f32 radius, f32 otherRadius);
    static BOOL isFree(const mVec3_c *pos, BOOL npc, f32 playerRadius, f32 npcRadius, f32 playerSize, f32 npcSize);

    static const f32 sFreeRadius;
    BOOL hasTopItem();
    u16 makeDemoID(const mVec3_c *ofs, mAng angle);
    static BOOL getParentOfs(int *handle, mVec3_c *ofs, mAng *angle, int x, int z, mAng ownAngle);
    mVec3_c *getParentPos();
    mAng getParentAngle();
    BOOL attachParent();
    BOOL attachParent(int x, int z, mAng angle);
    BOOL setParent(int handle, const mVec3_c *ofs, mAng angle);
    void appearEffect(const mVec3_c *pos);
    void setSound(bool on);
    void startAppearSe();
    void startVanishSe();
    BOOL updateSound();
    BOOL turnOnSound();
    BOOL turnOffSound();
    BOOL startMoveSe();
    static int getMoveSe();
    static BOOL isLampOn(daFTR_c *ftr);
    static BOOL isMusicFtr(daFTR_c *ftr);
    BOOL isMusicOn();
    BOOL isTvOn();
    static bool isSeatFtr(daFTR_c *ftr);
    BOOL isLampFunc();
    BOOL isLit();
    BOOL isFtrFuncD();
    BOOL isFtrFunc10();
    BOOL isFtrFunc36();
    BOOL isFtrFunc39();
    bool isSeat();
    static BOOL setRoomLight(int lamp);
    void initLamp();
    void executeLamp();
    void deleteLamp();
    f32 getCatalogScale() const;
    f32 getHeight() const;
    void adjustPos();
    BOOL demoCmd(int cmd);
    void calcSwitchNode();
    void calcSound();
    void initShadow();
    void drawShadow(const mVec3_c *scale);

    /* 0x000 dDemoActor_c, 0x0BC dBGCF::mvbg_c, 0x158 dFTR::alwaysAc_c, 0x15C dMsg::Rcpt_c */
    /* 0x1C0 */ int mUnitX;
    /* 0x1C4 */ int mUnitZ;
    /* 0x1C8 */ m3d::anmChr_c mAnmChr;
    /* 0x200 */ m3d::anmMatClr_c mAnmClr;
    /* 0x22C */ m3d::anmTexPat_c mAnmTexPat;
    /* 0x258 */ m3d::anmTexSrt_c mAnmTexSrt;
    /* 0x284 */ m3d::anmVis_c mAnmVis;
    /* 0x2BC */ loader_c mRes;
    /* 0x32C */ mAllocator_c mAllocator;
    /* 0x348 */ nodeCallBack_c mNodeCallBack;
    /* 0x358 */ mAng mTargetAngle;
    /* 0x35C */ mVec3_c mTargetPos;
    /* 0x368 */ u32 mHoldId;
    /* 0x36C */ void *mpMgrData;
    /* 0x370 */ mVec3_c mScale;
    /* 0x37C */ u8 mSwitchParam;
    /* 0x380 */ lamp_c mLamp;
    /* 0x398 */ switchNode_c mSwitchNode;
    /* 0x3A0 */ const dItem::BITM *mpBITM;
    /* 0x3A4 */ dRecBank::slot_c mRecSlot;
    /* 0x3A8 */ info_c mInfo;
    /* 0x3AC */ int mState;
    /* 0x3B0 */ f32 mBgScale;
    /* 0x3B4 */ flags_c mFlags;
    /* 0x3B8 */ mVec3_c mPushOfs;
    /* 0x3C4 */ mMtx_c mMtx;
    /* 0x3F4 */ u16 mAppearTimer;
    /* 0x3F6 */ u16 mAppearAngle;
    /* 0x3F8 */ s16 mAppearSpeed;
    /* 0x3FC */ f32 mAppearAmp;
    /* 0x400 */ u16 mStateTimer;
    /* 0x402 */ u16 mRotateWait;
    /* 0x404 */ u8 _404[0x408 - 0x404];
    /* 0x408 */ u8 mSwitch;
    /* 0x409 */ u8 mSwitchNext;
    /* 0x40A */ u8 mSwitchWait;
    /* 0x40C */ s16 mParent;
    /* 0x40E */ mAng mParentAngle;
    /* 0x410 */ mVec3_c mParentOfs;
    /* 0x41C */ topItemList_c mTopItems;
    /* 0x460 */ mVec3_c mEffectPos;
    /* 0x46C */ u8 _46C[0x470 - 0x46C];
    /* 0x470 */ dSndObjSimple_c mSndObj;
    /* 0x4A0 */ u8 mEnabled;
    /* 0x4A4 */ mVec3_c mDemoPos;
    /* 0x4B0 */ dItem::Item mMsgItem0;
    /* 0x4B2 */ dItem::Item mMsgItem1;
    /* 0x4B4 */ dItem::Item mMsgItem2;
    /* 0x4B8 */ dObjc::dpdBox_c mDpdBox;
    /* 0x828 */ mVec3_c mCenter;
    /* 0x834 */ dSndObjFurniture_c mSndFtr;
    /* 0x888 */ m3d::mdl_c mMdl;
    /* 0x8C8 */ u8 _8C8[0x8CC - 0x8C8];
}; // size 0x8CC

