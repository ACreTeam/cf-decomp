#pragma once

// The special npc actor base dAcNpcSp_c. DOL TU d_a_npc_sp.cpp (.text 80061ADC..80063438). Class names from
// the RTTI ("dAcNpcSp_c", "dAcNpcSp_c::resMng_c", "dAcNpcSp_c::receptSp_c"); everything below class level
// is named from behaviour. The special npcs (RELs d_a_npc_sp_*NP: bug, franklin, catherine, ...) derive
// from it; its constructor is inlined into theirs (it is not in the DOL). See notes/d_a_npc_sp.txt.
//
// Vtables in .data: dAcNpcSp_c 804A4C30, resMng_c 804A4D48, receptSp_c 804A4D88 (reverse class-completion
// order), so receptSp_c is defined first, then resMng_c.

#include <types.h>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>

class dAcNpcSp_c : public dAcNpc_c {
public:
    // The message receptor of the special npcs (RTTI "dAcNpcSp_c::receptSp_c", vtable 804A4D88; code
    // 80062780..80062814). Abstract (getMsgInfo): the RELs derive their talk classes from it.
    class receptSp_c : public dAcNpc_c::recept_c {
    public:
        receptSp_c();          // 80062780
        virtual ~receptSp_c(); // 800627BC
    }; // size 0xEC

    // The model resource of a special npc (RTTI "dAcNpcSp_c::resMng_c", vtable 804A4D48; code
    // 80062814..80062984): "/Npc/Special/Model/%d.brres" of the npc index (loadRes).
    class resMng_c : public dAcNpc_c::resBase_c {
    public:
        resMng_c();                         // 80062814
        virtual ~resMng_c();                // 80062858
        virtual BOOL create(dAcNpc_c *npc); // 800628B0: load + bind the model file
        virtual void *getMdlRes();          // 80062974
        virtual void *getTexRes();          // 8006297C (the same file)

        /* 0x4 */ void *mpData; // the .brres (loadRes)
    }; // size 0x8

    // The net (daub) state word the RELs pass to executeWander / executeRemoteWander / resetWanderDaub
    // (their actor +0x1C80): the top byte is the emotion the npc plays.
    struct wanderDaub_s {
        u32 mEmotion : 8; // emotion (action_c::requestEmotion), 0 = none
        u32 _0 : 24;
    }; // size 0x4

    // ---- vtable 804A4C30 (0xE8) ----
    // Inline virtuals come out in the weak tail after __sinit in reverse declaration order (getWanderMinDist,
    // getWanderMaxDist, then the dtor), so the dtor is declared first.
    virtual ~dAcNpcSp_c() {}                       // weak 8006331C (+0x48)
    virtual int create();                          // 800622C8
    virtual int preCreate();                       // 80062280
    virtual int doDelete();                        // 8006237C
    virtual bool canInteract(dDemoActor_c *actor); // 800623AC (+0x4C)
    virtual void demoHook50();                     // 800623A8 (+0x50)
    virtual void getName(dHmnName::Word_c *name, int len); // 80061AFC (+0x88)
    virtual u8 getNameKind();                      // 80061B0C (+0x8C): 2 (none)
    virtual f32 getHandItemOfsX() const;                      // 80061B14 (+0x90)
    virtual f32 getHandItemOfsZ() const;                      // 80061B20 (+0x94)
    virtual int getEarType();                      // 80061B2C (+0x98): 60 (no ears)
    virtual int getSoundId();                      // 800620A4 (+0xA0)
    virtual void playSound();                      // 800620AC (+0xA4): once
    virtual int getVoiceType() const;              // 80061B34 (+0xA8)
    virtual void getManpuOfs(mVec3_c *ofs, mVec3_c *ofsL, mVec3_c *ofsR, u8 type); // 80062168 (+0xAC)
    virtual u32 getHeapSize();                     // 80061ADC (+0xB4)
    virtual u32 addToNpcList();                    // 80061AE4 (+0xB8)
    virtual void removeFromNpcList();              // 80061AEC (+0xBC)
    virtual int getFaceType();                     // 80061AF0 (+0xC0)
    // new virtuals (+0xC4..+0xE4)
    virtual BOOL getPointedShopItem(dItem::Item *item, int *unitX, int *unitZ); // 80062414 (+0xC4)
    virtual BOOL searchWanderPos(mVec3_c *pos);    // 800629D0 (+0xC8)
    virtual int getWanderWaitBase();               // 800630CC (+0xCC)
    virtual int getWanderWaitRand();               // 800630D0 (+0xD0)
    virtual f32 getWanderMaxDist() { return mWanderMaxDist; } // weak 800632F4 (+0xD4) (non-const: scheduling)
    virtual f32 getWanderMinDist() { return mWanderMinDist; } // weak 800632CC (+0xD8) (non-const: scheduling)
    virtual BOOL isWanderWalk();                   // 80062B40 (+0xDC): random walk chance
    virtual int executeWander(wanderDaub_s *daub); // 80062B88 (+0xE0)
    virtual int vtE4();                            // 80062F08 (+0xE4)

    mVec3_c getRouteStep(const mVec3_c *target);    // 80061B3C: next unit center on the route toward target
    static const Vec *searchPosTable(const posTable97_s *table, int num, int key, const dItem::Item *item); // 800620EC
    mVec3_c calcFreePosToward(const mVec3_c *target); // 80062504: last free unit center on the line to target
    static u32 calcHeapSize(BOOL withFrm);         // 80062984
    BOOL checkStuck();                             // 80062A44: pushed for 120 frames while moving
    void initWander();                             // 80062A9C
    BOOL resetWanderDaub(wanderDaub_s *daub);          // 80062ED8
    void resetLookTimer();                         // 80062F10
    void executeRemoteWander(wanderDaub_s *daub);  // 80062F50
    void moveFtrToPoliceBox();                     // 800630D4: the furniture under the npc -> police box

    /* 0x1C48 */ u32 mWaitTimer;      // frames until the next wander walk
    /* 0x1C4C */ u32 mLookTimer;      // frames until the next look-around
    /* 0x1C50 */ u32 mWallTimer;      // frames walking against a wall (checkStuck)
    /* 0x1C54 */ u32 mWanderMaxDist;  // 128
    /* 0x1C58 */ u32 mWanderMinDist;  // 46
    /* 0x1C5C */ mVec3_c mHomePos;    // mPos at create
    /* 0x1C68 */ int mRouteDir;       // getRouteStep: index into l_routeOfs (0..3)
    /* 0x1C6C */ int _1C6C;
    /* 0x1C70 */ resMng_c mRes;
    /* 0x1C78 */ u8 mSoundPlayed;     // playSound
}; // size 0x1C7C
