#pragma once

// Hand tool models (shovel, axe, rod, net, umbrella, balloon, ...). DOL TU d_hmn_tool_mng.cpp,
// .text 800B9BDC..800BD450. One global manager (80599D0C) holds a double-buffered resource slot per
// holder (slots 0..3 are the players, 4..8 other humans); each holder owns a dHmnToolBank_c that builds
// the models/animations from its slot.
// Class names from the heap strings "dHmnToolMng_c::createHeap::data", "dHmnToolBank_c::m_heap_p" and
// the RTTI "dHmnToolBank_c::myMdlCallback_c"; every other name here is inferred.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_balloon_string.hpp>
#include <game/game/d_uki.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggFrmHeap.h>

class dDesign_c;
class dPrivateData_c;

// The tool type of an item (dHmnToolBank_c::getItemToolType). Names inferred from the item kinds.
enum dHmnToolType_e {
    HMN_TOOL_NONE,
    HMN_TOOL_SCOOP,    // shovels (digs with the two extra models)
    HMN_TOOL_AXE,
    HMN_TOOL_ROD,      // fishing rods (the float)
    HMN_TOOL_NET,
    HMN_TOOL_WATERING,
    HMN_TOOL_PACHINKO, // slingshots (the float as the shot)
    HMN_TOOL_FLOWER,   // KIND_FLOWER with BITM hide-bone 9
    HMN_TOOL_CRACKER,
    HMN_TOOL_HANABI,
    HMN_TOOL_UMBRELLA,
    HMN_TOOL_BALLOON,
    HMN_TOOL_WINDMILL,
    HMN_TOOL_SYABON,
    HMN_TOOL_NUM,
};

class dHmnToolMng_c {
public:
    enum {
        SLOT_NUM = 9, // 4 players + 5 others
        PLAYER_SLOT_NUM = 4,
        BUFFER_NUM = 2,
    };

    // Load states of one buffer (mLoadState).
    enum {
        LOAD_NONE = 0,
        LOAD_BUSY = 1,
        LOAD_DONE = 2,
    };

    // Request states of a slot (mState).
    enum {
        STATE_IDLE = 0,
        STATE_RELEASE = 1,
        STATE_LOAD = 2,
    };

    // One holder's resource slot ("::data" in the heap name).
    class data_c {
    public:
        data_c();  // 800B9BDC
        ~data_c(); // 800B9C90

        /* 0x000 */ dItem::resLoader_c mModel[BUFFER_NUM];  // the tool brres
        /* 0x0E0 */ dItem::resLoader_c mDesign[BUFFER_NUM]; // original-design umbrella texture
        /* 0x1C0 */ dItem::Item mReqItem;                   // being loaded
        /* 0x1C4 */ EGG::FrmHeap *mpHeap[BUFFER_NUM];
        /* 0x1CC */ u16 mItemId;                            // loaded and shown (no default ctor)
        /* 0x1CE */ u8 mLoadState[BUFFER_NUM];
        /* 0x1D0 */ u8 mState;
        /* 0x1D1 */ u8 mCur;    // buffer holding mItemId
        /* 0x1D2 */ u8 mLocked; // buffer in use by the bank's models (2 = none)
    }; // size 0x1D4

    dHmnToolMng_c();  // 800B9D14
    ~dHmnToolMng_c(); // 800B9D5C

    BOOL createHeap(EGG::Heap *parent); // 800B9DC0
    void clearPlayerItems();            // 800B9E54
    void clearItem(int slot);           // 800B9E70
    BOOL load(data_c *data, dPrivateData_c *priv, dDesign_c *design); // 800B9E88
    BOOL release(data_c *data);         // 800BA00C
    int getLoadBuffer(data_c *data);    // 800BA0DC
    void bindDesign(data_c *data);      // 800BD208: the design texture onto the model's "cloth"

    data_c *getData(int slot) { return &mData[slot]; }

    static int getHeapSize();              // 800BD294
    static BOOL create(EGG::Heap *parent); // 800BD2C8
    static void clearPlayers();            // 800BD2D8
    static void clear(int slot);           // 800BD2E4
    static int getBankHeapSize();          // 800BD2F4

    /* 0x000 */ data_c mData[SLOT_NUM];
}; // size 0x1074

// One holder's tool models (non-polymorphic).
class dHmnToolBank_c {
public:
    // Turns the windmill's "rot_windmill" node.
    class myMdlCallback_c : public m3d::mdl_c::callback_c {
    public:
        virtual ~myMdlCallback_c() {}
        virtual void timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl); // 800BA0F8
        virtual void timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip, nw4r::g3d::ResMdl resMdl); // 800BA178
        virtual void timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl);                        // 800BA17C

        /* 0x4 */ ulong mNodeId;
        /* 0x8 */ s16 mRot;
    }; // size 0xC

    dHmnToolBank_c();  // 800BA180
    ~dHmnToolBank_c(); // 800BA2C4

    void create(int slot);                                   // 800BA3C8
    BOOL request(const dItem::Item *item, dPrivateData_c *priv, dDesign_c *design); // 800BA474: load the tool of item; TRUE when ready
    void lock();                                             // 800BA798
    void *getResFile(BOOL locked);                           // 800BA7C0
    BOOL isAnmLoop();                                        // 800BA84C
    int getToolType();                                       // 800BA888
    void remove();                                           // 800BA9AC
    void calc(const mMtx_c *mtx);                            // 800BAA00: at mtx (the balloon writes it back)
    void draw();                                             // 800BAC9C
    void change();                                           // 800BAD74: build the models
    void setupAnm(int type, nw4r::g3d::ResFile *file, nw4r::g3d::ResMdl *mdl); // 800BAFB4
    void setupAxe(nw4r::g3d::ResFile *file, nw4r::g3d::ResMdl *mdl);         // 800BB2E4
    void setupDig(nw4r::g3d::ResFile *file);                                  // 800BB410
    void putAway();                                          // 800BB63C
    void setAnmRate(f32 rate);                               // 800BB690
    f32 getAnmRate();                                        // 800BB6A8
    void setAnmFrame(f32 frame);                             // 800BB6C8
    f32 getAnmFrame();                                       // 800BB6E0
    f32 getAnmFrameMax();                                    // 800BB700
    bool isAnmStop();                                        // 800BB71C
    bool checkAnmFrame(f32 frame);                           // 800BB73C
    BOOL getNodeMtx(mMtx_c *mtx, ulong idx);                 // 800BB75C
    void setLocalMtx(const mMtx_c *mtx);                     // 800BB804
    void setAnm(const char *name, m3d::playMode_e mode, f32 blend); // 800BB878
    void setBodyAnm(u32 anmId, m3d::playMode_e mode, f32 blend);    // 800BB96C: net animations 0x1A7..0x1BA
    void setNetWait(m3d::playMode_e mode, f32 blend);        // 800BBA10
    void setNetAnm1B6(m3d::playMode_e mode, f32 blend);      // 800BBA74
    void setNetAnm1AA(m3d::playMode_e mode, f32 blend);      // 800BBA80
    void setNetAnm1A9(m3d::playMode_e mode, f32 blend);      // 800BBA8C
    void setNetAnm1A7(m3d::playMode_e mode, f32 blend);      // 800BBA98
    void setNetAnm1B3(m3d::playMode_e mode, f32 blend);      // 800BBAA4
    void setNetAnm1B7(m3d::playMode_e mode, f32 blend);      // 800BBAB0
    void setNetAnm1B8(m3d::playMode_e mode, f32 blend);      // 800BBABC
    void setNetAnm1AF(m3d::playMode_e mode, f32 blend);      // 800BBAC8
    void setNetAnm1B0(m3d::playMode_e mode, f32 blend);      // 800BBAD4
    void setNetAnm1B1(m3d::playMode_e mode, f32 blend);      // 800BBAE0
    void setNetAnm1B2(m3d::playMode_e mode, f32 blend);      // 800BBAEC
    void setNetAnm1B5(m3d::playMode_e mode, f32 blend);      // 800BBAF8
    void setNetAnm1AD(m3d::playMode_e mode, f32 blend);      // 800BBB04
    void setNetAnm1AB(m3d::playMode_e mode, f32 blend);      // 800BBB10
    void setNetAnm1AC(m3d::playMode_e mode, f32 blend);      // 800BBB1C
    void setNetAnm1AE(m3d::playMode_e mode, f32 blend);      // 800BBB28
    void setNetAnm1B9(m3d::playMode_e mode, f32 blend);      // 800BBB34
    void setNetAnm1B4(m3d::playMode_e mode, f32 blend);      // 800BBB40
    void setNetAnm1A8(m3d::playMode_e mode, f32 blend);      // 800BBB4C
    void setPoleWait(m3d::playMode_e mode, f32 blend);       // 800BBB58
    void setPoleSwing(m3d::playMode_e mode, f32 blend);      // 800BBBC8
    void setPoleSuka(m3d::playMode_e mode, f32 blend);       // 800BBC38
    void setPoleWaitNibble(m3d::playMode_e mode, f32 blend); // 800BBCA8
    void setPoleHit(m3d::playMode_e mode, f32 blend);        // 800BBD0C
    void setPolePutback(m3d::playMode_e mode, f32 blend);    // 800BBD7C
    void setPoleFail(m3d::playMode_e mode, f32 blend);       // 800BBDEC
    void setPoleGet1(m3d::playMode_e mode, f32 blend);       // 800BBE5C
    void setPoleGet2(m3d::playMode_e mode, f32 blend);       // 800BBECC
    void setPolePutaway(m3d::playMode_e mode, f32 blend);    // 800BBF30
    void setPoleComplete(m3d::playMode_e mode, f32 blend);   // 800BBF94
    void setPoleTumbleDown(m3d::playMode_e mode, f32 blend); // 800BBFF8
    void setPoleTumbleUp(m3d::playMode_e mode, f32 blend);   // 800BC05C
    void setPoleStingDown(m3d::playMode_e mode, f32 blend);  // 800BC0C0
    void openUmbrella();                                     // 800BC124
    void closeUmbrella();                                    // 800BC178
    void startHanabi();                                      // 800BC1D4
    void setSlingShoot1(m3d::playMode_e mode, f32 blend);    // 800BC270
    void setSlingShoot2(m3d::playMode_e mode, f32 blend);    // 800BC2D4
    void setSlingShoot3(m3d::playMode_e mode, f32 blend);    // 800BC338
    void setSlingWait(m3d::playMode_e mode, f32 blend);      // 800BC39C
    void setBalloonItem(const dItem::Item *item);            // 800BC400
    void calcBalloonInit(const mMtx_c *mtx);                 // 800BC448
    u8 isBalloonFly();                                       // 800BC468
    BOOL isBalloonGone();                                    // 800BC484
    void startFlower();                                      // 800BC4BC
    void calcWind(const mVec3_c *pos);                       // 800BC51C
    void stopWind();                                         // 800BC64C
    void calcWindmill();                                     // 800BC688
    f32 getWindRate();                                       // 800BC710
    void setDigAnm(const mVec3_c *pos, const mAng3_c *angle, int anm, BOOL dig, u8 alpha); // 800BC720
    BOOL hasResMdl();                                        // 800BCA48
    void removeModels();                                     // 800BCAC8
    void resetHeap();                                        // 800BCD28
    void setBalloonColor(u8 alpha);                          // 800BCD84
    s8 getAxeIdx(const dItem::Item &item);                   // 800BCEF8: -1 if not an axe
    void setAxeItem(const dItem::Item &item);                // 800BCF84
    u16 getNextAxe();                                        // 800BD010
    static BOOL isOrgUmbrella(u32 *design, const dItem::Item *item); // 800BD084
    static BOOL isHandTool(const dItem::Item *item);         // 800BD134
    static int getItemToolType(const dItem::Item *item);     // 800BA890: dHmnToolType_e of an item
    BOOL syncLoad();                                         // 800BD170

    int getType() const { return mType; } // int return: toolBase_c::isEnable's full signed compare

    /* 0x000 */ mAllocator_c mAllocator;
    /* 0x01C */ dUki_c mFloat; // rod float / slingshot shot
    /* 0x46C */ m3d::mdl_c mMdl;
    /* 0x4AC */ m3d::mdl_c mDigMdl0; // shovel: dug-up block models (players only)
    /* 0x4EC */ m3d::mdl_c mDigMdl1;
    /* 0x52C */ m3d::anmChr_c mAnm;
    /* 0x564 */ m3d::anmChr_c mDigAnm0;
    /* 0x59C */ m3d::anmChr_c mDigAnm1;
    /* 0x5D4 */ m3d::anmVis_c mVis; // axe (wear)
    /* 0x60C */ m3d::anmVis_c mDigVis0;
    /* 0x644 */ m3d::anmVis_c mDigVis1;
    /* 0x67C */ m3d::anmTexSrt_c mTexSrt; // hanabi / windmill
    /* 0x6A8 */ m3d::anmMatClr_c mMatClr; // balloon colour
    /* 0x6D4 */ myMdlCallback_c mCallback;
    /* 0x6E0 */ mVec3_c mWindPos;
    /* 0x6EC */ mVec3_c mDigPos;
    /* 0x6F8 */ mAng3_c mDigAngle;
    /* 0x6FE */ dItem::Item mItem;
    /* 0x700 */ EGG::FrmHeap *mpHeap; // "dHmnToolBank_c::m_heap_p"
    /* 0x704 */ dBalloonString_c *mpBalloon;
    /* 0x708 */ f32 mWindSpeed;
    /* 0x70C */ s16 mWindAngle;
    /* 0x70E */ u8 mType; // manager slot, 9 = none
    /* 0x70F */ u8 mWindState;
    /* 0x710 */ u8 mColor; // balloon colour frame
    /* 0x711 */ u8 mDigMode; // 1: mDigMdl0, 2: mDigMdl1
    /* 0x712 */ u8 mMdlCreated;
    /* 0x713 */ u8 mAnmCreated;
    /* 0x714 */ u8 mAxe;
    /* 0x715 */ u8 mVisCreated;
    /* 0x716 */ u8 mTexSrtCreated;
    /* 0x717 */ u8 mDigMdlCreated;
    /* 0x718 */ u8 mDigAnmCreated;
    /* 0x719 */ u8 mDigVisCreated;
}; // size 0x71C

// The bank heaps' parent (d_hmn's heap, not identified).
