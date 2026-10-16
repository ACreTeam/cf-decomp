#pragma once

// The furniture manager (RTTI dFtrm_c, REL d_ftr_managerNP).

#include <game/game/d_dvd.hpp>
#include <game/game/d_rec_bank.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_vec.hpp>

class daFTR_c;

class dFtrm_c {
public:
    class loader_c : public dDvd::arcBank_c {
    public:
        void *getFile(const char *name);
    };

    class pool_c {
    public:
        void clear();
        BOOL create(u32 num);
        void *alloc();
        BOOL free(void *p);

        /* 0x00 */ u32 mNum;
        /* 0x04 */ u32 mSize;
        /* 0x08 */ u8 *mpData;
        /* 0x0C */ u8 mUsed[8];
    }; // size 0x14

    struct current_c {
        void remove(daFTR_c *ftr);
        void replace(daFTR_c *ftr);

        /* 0x00 */ daFTR_c *mOld;
        /* 0x04 */ daFTR_c *mNew;
        /* 0x08 */ void *mpData;
    };

    class transAnm_c {
    public:
        BOOL startPush(const u32 *owner, const mVec3_c *pos, mAng angle);
        BOOL startPull(const u32 *owner, const mVec3_c *pos, mAng angle);
        BOOL calc(const u32 *owner, mVec3_c *pos);
    };

    static pool_c *getPool();
    static current_c *getCurrent();
    static u16 getRotateSpeed();
    static u16 getRotateWait();
    static f32 getDpdHeight();
    static s16 getPushAngle();
    static f32 getPushDist();
    static BOOL isLampScene();
    static int getDir(mAng angle);
    static void setEffectPos(const mVec3_c *pos);
    static void appearItem(u16 item, const mVec3_c *pos);
    static BOOL getWallUnit(int *x, int *z, u32 i);

    static const u32 sMinResSize;
    static u8 sLampOn;
    static dRecBank::bankTbl_c sRecBank;
    static loader_c sLoader;
    static transAnm_c sTransAnm;
};
