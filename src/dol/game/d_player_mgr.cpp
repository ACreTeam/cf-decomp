// Player manager: player actor and player data access for the local players
// and visitors. .text 800FBB18..8010263C.
// First pass: every function is written for equivalence; matching has not started.
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <lib/egg/math/eggMath.h>
#include <lib/nw4r/math/math_triangular.h>
#include <cstring>
#include <cmath>

// Dependencies whose owners are not recovered yet.
struct dUnk8074EBE8_c {
    u8 _0000[0x5884];
    int _5884;
};

extern "C" {
extern u8 lbl_8074EA7D;
extern dUnk8074EBE8_c *lbl_8074EBE8;

int fn_800DCF58(); // index of the current player
u32 fn_8019AFE4();
int fn_800BA890(const dItem::Item *item);
int fn_800BA888(void *obj);
BOOL fn_801625D0(int type);
void fn_801B910C(int idx, int part, u16 item);
void fn_801B8898(int idx, int part, u16 item);
void fn_801B8FD8(int idx, int type);
BOOL fn_800B7310(void *out, const dItem::Item *item);
BOOL fn_800B89CC(void *out, const dItem::Item *item);
BOOL fn_800B72A4(const dItem::Item *item);
BOOL fn_800BD134(const dItem::Item *item);
u8 *fn_800F5BA8();
BOOL fn_8018F438(int type);
f32 fn_802B0980(m3d::fanm_c *anm); // frame
void fn_802B0994(m3d::fanm_c *anm, f32 frame);
BOOL fn_802C4404(dUnk83ED_c *mii, u16 *out);
BOOL fn_802B3BD0(int a, u16 mii, int b);
int fn_802C5A0C(dMiiData_c *out, int a, int b, u16 mii);
f32 fn_80074974(const mVec3_c *pos, int a); // ground height
void fn_801710BC(int idx, int a);
BOOL fn_8018EB8C();
BOOL fn_8018ECA0();
void *fn_8006996C();
BOOL fn_80069308(void *obj);
BOOL fn_800FA724();
u32 fn_800827A8(mVec3_c *out, int arg);
void fn_801B961C();
void fn_801B9658(int value);
void fn_800BD2D8();
void fn_800BD2E4(u8 value);
void fn_800B8E7C();
void fn_800B8E88(u8 value);
void fn_800B7658();
void fn_800B7664(u8 value);
void fn_801B7C10();
BOOL fn_80194A2C(int a, int b, int c, int d, dPlayerActor_c *player, int e, f32 f1, f32 f2, f32 f3, f32 f4, f32 f5,
                 f32 f6);
BOOL fn_80194AA0(int type, dPlayerActor_c *player, f32 f);
int fn_80194B30();
void fn_801B91B8();
void fn_801B90C0();
void fn_801B835C();
void fn_801B94E8(int idx, int value);
void fn_801B9518(int idx, const u8 *value);
void fn_801B9258();
void fn_801B9304();
void fn_800A3044(u8 a, u8 b, u8 c);
void fn_800A3118(int a, u8 b, u8 c, int d);
int fn_80162548();
BOOL fn_80162558(u8 a);
void fn_80082B04(mVec3_c *out, const mVec3_c *in);
int fn_8006E1BC(dGroundCheck_c *check, const mVec3_c *pos, int a, int b, int c);
BOOL fn_8006E400(dGroundCheck_c *check, f32 y);
f32 fn_8006E31C(dGroundCheck_c *check, int a);
void fn_80087790(const char *name, const mVec3_c *pos, int a, int b);
void fn_80087844(int a, const mVec3_c *pos, int b, int c, void (*cb)(dEffectTarget_c *, u32), u32 kind);
void fn_80285110(void *obj, dEffectTarget_c *target);
u8 fn_800A8850(const mVec3_c *pos);
}

// 8074E600 / 8074E602, set by the __sinit: look-at limits (fn_800FE938, also read by d_a_player).
s16 sLookPitchMax = 10.0f * 182.04445f;
s16 sLookYawMax = 45.0f * 182.04445f;

// 8074AF10
static u8 lbl_8074AF10 = 0x3F;

// 805D2440
dPlayerMgr_c lbl_805D2440;

// 800FBB18
void *fn_800FBB18(void *p, int del) {
    // Deleting destructor of an empty class.
    if (p != NULL && del > 0) {
        operator delete(p);
    }
    return p;
}

// 800FBB58
dPlayerMgr_c::dPlayerMgr_c() {
    memset(mActors, 0, sizeof(mActors));
    for (int i = 0; i < 4; i++) {
        mChangeState[i] = NULL;
        _10[i] = 0;
        fn_80100258(i);
    }
    _20 = 0;
    fn_80101408();
}

// 800FBBEC
dPlayerMgr_c::~dPlayerMgr_c() {}

// 800FBC48
mVec3_c *fn_800FBC48(int idx) {
    dPlayerActor_c *player = fn_800FBC7C(idx);
    if (player == NULL) {
        return NULL;
    }
    return &player->mPos;
}

// 800FBC7C
dPlayerActor_c *fn_800FBC7C(int idx) {
    return fn_801019C4(idx);
}

// 800FBC80
dPlayerActor_c *fn_800FBC80(int idx) {
    return fn_80101A28(idx);
}

// 800FBC84
dPlayerActor_c *fn_800FBC84(int idx) {
    return fn_801019C4(idx);
}

// 800FBC88
void *fn_800FBC88(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return NULL;
    }
    return player->_060C;
}

// 800FBCBC
BOOL fn_800FBCBC(s16 *out) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (out != NULL && player->_22EC != 0.0f) {
        *out = player->_22F8;
        return TRUE;
    }
    return FALSE;
}

// 800FBD28
s16 fn_800FBD28(int idx) {
    dPlayerActor_c *player = fn_800FBC7C(idx);
    if (player == NULL) {
        return 0;
    }
    return player->mAngle.y;
}

// 800FBD5C
BOOL fn_800FBD5C(u32 *a, u32 *b, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (a == NULL || b == NULL) {
        return FALSE;
    }
    *a = player->_2284;
    *b = player->_2288;
    return TRUE;
}

// 800FBDD4
void *fn_800FBDD4(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return NULL;
    }
    return player->_0624;
}

// 800FBE08
void *fn_800FBE08(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return NULL;
    }
    return player->_06F0;
}

// 800FBE3C
BOOL fn_800FBE3C() {
    if (lbl_805D2440.mLocked) {
        return FALSE;
    }
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    return fn_800FBF84(player->mState);
}

// 800FBE98
BOOL fn_800FBE98() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (fn_800FBF84(state)) {
        return TRUE;
    }
    switch (state) {
    case 0x1A:
    case 0x24:
    case 0x26:
    case 0x28:
    case 0x5A:
    case 0x99:
        player->_2274 |= 0x4;
        return TRUE;
    case 0x1D:
    case 0x5B:
        return TRUE;
    }
    return FALSE;
}

// 800FBF84
BOOL fn_800FBF84(int state) {
    switch (state) {
    case 0x0:
    case 0x1:
    case 0x23:
    case 0x2E:
    case 0x38:
        return TRUE;
    }
    return FALSE;
}

// 800FBFCC
void fn_800FBFCC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player != NULL && player->mState != 0x49) {
        fn_80101A74(0x8, 4);
    }
}

// 800FC010
BOOL fn_800FC010() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x8) {
        return TRUE;
    }
    return fn_80101A74(0x8, 4);
}

// 800FC064
void fn_800FC064(BOOL on, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player != NULL) {
        if (on) {
            player->_2270 |= 0x100000;
        } else {
            player->_2270 &= ~0x100000;
        }
    }
}

// 800FC0C0
u32 fn_800FC0C0(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 20) & 1;
}

// 800FC0F8
BOOL fn_800FC0F8() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x9B) {
        player->_2274 |= 0x100;
        return TRUE;
    }
    if (state != 0x9A) {
        return FALSE;
    }
    if (fn_80101A74(0x9B, 4)) {
        player->_2274 |= 0x100;
        return TRUE;
    }
    return FALSE;
}

// 800FC194
BOOL fn_800FC194(const mVec3_c *target) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->_2274;
    player->_2274 = flags & ~0x100;
    if (flags & 0x200) {
        return TRUE;
    }
    int state = player->mState;
    if (state == 0x0 || state == 0x61) {
        return TRUE;
    }
    if (state != 0x9B) {
        return FALSE;
    }
    if (target == NULL) {
        return fn_80101A74(0x0, 4);
    }
    if (fn_80101A74(0x61, 4)) {
        s16 angle = dActor_c::targetAngleY(&player->mPos, target);
        player->_22D0 = sLib::distanceAngle(angle, player->mAngle.y) / 10;
        player->_22C4 = angle;
        return TRUE;
    }
    return fn_80101A74(0x0, 4);
}

// 800FC2A4
BOOL fn_800FC2A4() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x9B) {
        player->_2274 |= 0x200;
        return TRUE;
    }
    if (state != 0x9A) {
        return FALSE;
    }
    if (fn_80101A74(0x9B, 4)) {
        player->_2274 |= 0x200;
        return TRUE;
    }
    return FALSE;
}

// 800FC340
BOOL fn_800FC340() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    u32 flags = player->_2274;
    player->_2274 = flags & ~0x200;
    if (flags & 0x100) {
        return TRUE;
    }
    if (player->mState != 0x9B) {
        return FALSE;
    }
    return fn_80101A74(0x0, 4);
}

// 800FC3B0
BOOL fn_800FC3B0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    return fn_80101A74(0xA1, 4);
}

// 800FC404
void fn_800FC404() {
    if (fn_801019C4(4) == NULL) {
        return;
    }
    if (fn_800FEEB0()) {
        return;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL || lbl_8074EA7D != 1) {
        return;
    }
    dItem::Item held = current->mEquipment.mHeld;
    if (!fn_80101584(fn_8019AFE4() & 7, &held)) {
        return;
    }
    if (fn_800BA890(&held) == 0xA && fn_801625D0(0x10)) {
        fn_801B910C(fn_800DCF58(), 3, held.mId);
        fn_801B8898(fn_800DCF58(), 3, held.mId);
    }
}

// 800FC4C0
BOOL fn_800FC4C0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    return fn_80101A74(0xA4, 4);
}

// 800FC514
BOOL fn_800FC514() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0xA4) {
        return FALSE;
    }
    return fn_80101A74(0xA5, 4);
}

// 800FC568
void fn_800FC568() {
    lbl_805D2440.mLocked = TRUE;
}

// 800FC580
void fn_800FC580() {
    lbl_805D2440.mLocked = FALSE;
}

// 800FC598
void fn_800FC598() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player != NULL && (player->_2270 & 0x10)) {
        int kind = fn_800BA888(player->_0C34);
        if (kind != 0xA && kind != 0xB && kind != 0xC) {
            lbl_805D2440._194AA = TRUE;
        }
    }
}

// 800FC604
void fn_800FC604() {
    lbl_805D2440._194AA = FALSE;
}

// 800FC61C
u8 fn_800FC61C() {
    return lbl_805D2440._194AA;
}

// 800FC630
BOOL fn_800FC630() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    player->_2270 &= ~0x2000000;
    return TRUE;
}

// 800FC674
BOOL fn_800FC674(int slot) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    slot &= 7;
    BOOL shirt = fn_80101584(slot, &current->mEquipment.mShirt);
    BOOL hat = fn_80101584(slot, &current->mEquipment.mHat);
    BOOL held = fn_80101584(slot, &current->mEquipment.mHeld);
    if (!shirt && !hat && !held) {
        return TRUE;
    }
    if (!shirt && !hat && held) {
        player->_2274 |= 0x20;
        if (!fn_800FECF8(&current->mEquipment.mHeld)) {
            player->_2274 &= ~0x20;
        }
    } else {
        dItem::Item *pShirt = shirt ? &current->mEquipment.mShirt : NULL;
        dItem::Item *pHat = hat ? &current->mEquipment.mHat : NULL;
        dItem::Item *pHeld = held ? &current->mEquipment.mHeld : NULL;
        player->_2274 |= 0x20;
        if (!fn_800FCDFC(pShirt, pHat, pHeld)) {
            player->_2274 &= ~0x20;
        }
    }
    return TRUE;
}

// 800FC7E8
BOOL fn_800FC7E8() {
    if (lbl_805D2440.mLocked) {
        return FALSE;
    }
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState <= 1;
}

// 800FC858
BOOL fn_800FC858() {
    return fn_801019C4(4) != NULL;
}

// 800FC888
BOOL fn_800FC888() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x0 || state == 0x1 || state == 0x38 || state == 0x2E) {
        return TRUE;
    }
    return FALSE;
}

// 800FC8EC
BOOL fn_800FC8EC(BOOL a) {
    if (a) {
        fn_800FBFCC();
        return TRUE; // the target tail-calls the void fn_800FBFCC; r3 is whatever it left
    }
    return fn_800FD17C();
}

// 800FC8FC
BOOL fn_800FC8FC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != 0x8 && state != 0x88) {
        return FALSE;
    }
    return fn_80101A74(0x88, 4);
}

// 800FC958
BOOL fn_800FC958() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    return fn_80101A74(0x9E, 4);
}

// 800FC9AC
BOOL fn_800FC9AC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9E && player->mAngle.y == -0x8000) {
        return TRUE;
    }
    return FALSE;
}

// 800FCA04
BOOL fn_800FCA04() {
    fn_800FBFCC();
    return TRUE;
}

// 800FCA28
BOOL fn_800FCA28() {
    if (lbl_805D2440.mLocked) {
        return FALSE;
    }
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state >= 0 && state < 3) {
        return TRUE;
    }
    return FALSE;
}

// 800FCAA0
BOOL fn_800FCAA0() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    if (!fn_800FCA28()) {
        return FALSE;
    }
    return fn_80101A74(0x7, 4);
}

// 800FCAF4
BOOL fn_800FCAF4(const dItem::Item *item) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x5F) {
        return FALSE;
    }
    int kind = item->getKind();
    if (kind == 7) {
        player->_2270 |= 0x8000;
    } else if (kind == 8) {
        player->_2270 &= ~0x8000;
    } else {
        return FALSE;
    }
    player->_218A = item->mId;
    return fn_80101A74(0x5F, 4);
}

// 800FCBA8
BOOL fn_800FCBA8() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x5F) {
        return FALSE;
    }
    player->_2234 = 6;
    return TRUE;
}

// 800FCBFC
BOOL fn_800FCBFC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x5F) {
        return FALSE;
    }
    if ((u32)(player->mAnmId - 0x53) <= 1) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800FCC74
BOOL fn_800FCC74(const dItem::Item *item, const mVec3_c *pos) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x60) {
        return TRUE;
    }
    int kind = item->getKind();
    if (kind == 7) {
        player->_2270 |= 0x8000;
    } else if (kind == 8) {
        player->_2270 &= ~0x8000;
    } else {
        return FALSE;
    }
    player->_218A = item->mId;
    player->_2270 |= 0x1000000;
    if (pos != NULL) {
        player->_2074 = *pos;
    }
    return fn_80101A74(0x60, 4);
}

// 800FCD60
BOOL fn_800FCD60(u32 a, u32 b, BOOL c) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    player->_2190 = a;
    player->_2194 = b;
    player->_2270 |= 0x1000000;
    if (c) {
        player->_2270 |= 0x80000000;
    } else {
        player->_2270 &= ~0x80000000;
    }
    return fn_80101A74(0x44, 4);
}

// BITM::m_hideBone, clamped like the target (values >= 0xB become 0xA).
static inline int getHideBone(dItem::BITM *bitm) {
    int v = (s8)bitm->m_hideBone;
    return (u32)v < 0xB ? v : 0xA;
}

// 800FCDFC
BOOL fn_800FCDFC(const dItem::Item *shirt, const dItem::Item *hat, const dItem::Item *held) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (fn_800FEEB0()) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }

    if (shirt != NULL && shirt->mId != dItem::ITEM_ID_NONE) {
        u8 tmp[0x10];
        if (fn_800B7310(tmp, shirt)) {
            player->_22B4 |= 0x100;
            fn_801B910C(fn_800DCF58(), 0, shirt->mId);
            fn_801B8898(fn_800DCF58(), 0, shirt->mId);
        }
    }

    if (hat != NULL) {
        int bone = 0xA;
        if (hat->mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*hat);
            if (bitm != NULL) {
                bone = getHideBone(bitm);
            }
        }
        if (fn_80100358(bone, player)) {
            if (current->_83F5 != 0) {
                fn_801B8FD8(fn_800DCF58(), 0x12);
            }
            fn_801B910C(fn_800DCF58(), 1, hat->mId);
            fn_801B8898(fn_800DCF58(), 1, hat->mId);
        }
    }

    if (held != NULL) {
        if (held->mId == dItem::ITEM_ID_NONE) {
            if (fn_801625D0(1)) {
                player->_22DA = 2;
            }
            fn_801B910C(fn_800DCF58(), 3, held->mId);
            fn_801B8898(fn_800DCF58(), 3, held->mId);
        } else {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
            if (bitm != NULL && getHideBone(bitm) == 9) {
                if (fn_801625D0(1)) {
                    player->_22DA = 1;
                }
                fn_801B910C(fn_800DCF58(), 3, held->mId);
                fn_801B8898(fn_800DCF58(), 3, held->mId);
            }
        }
    }

    player->_223C = 1;
    player->_2238 = 1;
    player->_2234 = 8;
    return fn_80101A74(0xA0, 4);
}

// 800FD058
BOOL fn_800FD058(const dItem::Item *item) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    int edibility = (s8)bitm->m_edibility;
    if ((u32)edibility >= 3) {
        edibility = 0;
    }
    switch (edibility) {
    case 1:
        player->_218A = item->mId;
        return fn_80101A74(0x78, 4);
    case 2:
        player->_218A = item->mId;
        return fn_80101A74(0x97, 4);
    }
    return FALSE;
}

// 800FD130
BOOL fn_800FD130() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    fn_801B8FD8(fn_800DCF58(), 0x10);
    return fn_80101A74(0x79, 4);
}

// 800FD17C
BOOL fn_800FD17C() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return TRUE;
    }

    switch (player->mState) {
    case 0x8:
        if (player->mAnmId == 0x31) {
            if (fn_80101A74(0x38, 4)) {
                fn_80100410(player);
                return TRUE;
            }
        } else if (player->mAnmId == 0x37) {
            if (fn_80101A74(0x2E, 4)) {
                fn_80100410(player);
                return TRUE;
            }
        }
        switch (player->_22DD) {
        case 2:
        case 3:
        case 4:
            if (fn_80101A74(0x84, 4)) {
                fn_80100410(player);
                return TRUE;
            }
            break;
        case 7:
            player->_22DD = 8;
            break;
        }
        break;
    case 0xA:
        if (*fn_800F5BA8() != 0) {
            if (fn_80101A74(0x84, 4)) {
                player->_22DD = 1;
                fn_80100410(player);
                *fn_800F5BA8() = 0;
                player->_2270 |= 0x200000;
                return TRUE;
            }
        } else if (fn_801014BC()) {
            fn_801014E8();
            fn_80100410(player);
            return fn_80101A74(0x9F, 4);
        }
        break;
    case 0x98:
        fn_80100410(player);
        return TRUE;
    case 0x2C:
    case 0x2D:
    case 0x34:
    case 0x39:
    case 0x3A:
    case 0x60:
    case 0x87:
    case 0xA8:
        fn_80100410(player);
        return TRUE;
    case 0x4A:
    case 0x81:
    case 0x83:
        if (player->_2274 & 0x400) {
            player->_2274 &= ~0x400;
            if (fn_80101A74(0x61, 4)) {
                fn_80100410(player);
                return TRUE;
            }
        }
        break;
    case 0x8C:
    case 0xAA:
        return TRUE;
    }

    dPlayerMgr_c::getCurrentPlayer();
    u32 flags = player->_2270;
    if ((flags & 0x10) && !(flags & 0x20) && !fn_800FC61C()) {
        if (player->_208C >= 1.0f) {
            player->_2270 |= 0x20;
            if (fn_80101A74(0x0, 4)) {
                fn_80100410(player);
                return TRUE;
            }
        } else {
            int state = fn_800BA888(player->_0C34) == 0xA ? 0x7B : 0x21;
            if (fn_80101A74(state, 4)) {
                fn_80100410(player);
                return TRUE;
            }
        }
    }
    if (fn_80101A74(0x0, 4)) {
        fn_80100410(player);
        player->_2270 |= 0x200000;
        return TRUE;
    }
    return FALSE;
}

// 800FD510
BOOL fn_800FD510(mVec3_c *out, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x38) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// Shared by the "walk to (x, z)" requests: target position at the player's height.
static inline void setTargetXZ(dPlayerActor_c *player, f32 x, f32 z) {
    mVec3_c target(x, player->mPos.y, z);
    player->_2068 = target;
}

// 800FD588
BOOL fn_800FD588(s16 angle, int type, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == 0x0 || state == 0x1 || (state == 0x23 && (player->_2274 & 0x4000)))) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_224C = type;
    return fn_80101A74(0x35, 4);
}

// 800FD664
BOOL fn_800FD664() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x31;
}

// 800FD6A8
BOOL fn_800FD6A8(mVec3_c *out, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mAnmId != 0x37) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// 800FD720
BOOL fn_800FD720(mVec3_c *out, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x2E) {
        return FALSE;
    }
    if (out != NULL) {
        *out = player->mPos;
    }
    return TRUE;
}

// 800FD798
BOOL fn_800FD798(s16 angle, int type, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (!(state == 0x0 || state == 0x1 || (state == 0x23 && (player->_2274 & 0x4000)))) {
        return FALSE;
    }
    if (type != 1 && type != 2) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_224C = type;
    return fn_80101A74(0x2A, 4);
}

// 800FD88C
BOOL fn_800FD88C(int type) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != 0x2E && state != 0x8) {
        return FALSE;
    }
    if (type != 1 && type != 2) {
        return FALSE;
    }
    player->_224C = type;
    player->_2248 = type == 1 ? 1 : 3;
    return fn_80101A74(0x2C, 4);
}

// 800FD924
BOOL fn_800FD924(int type) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state != 0x8 && state != 0x4F && state != 0x50) {
        return FALSE;
    }
    int value;
    switch (type) {
    case 0:
        value = 0;
        break;
    case 1:
        value = 1;
        break;
    case 2:
        value = 3;
        break;
    default:
        return FALSE;
    }
    player->_2248 = value;
    player->_224C = type;
    return fn_80101A74(0x39, 4);
}

// 800FD9E0
BOOL fn_800FD9E0(int type, s16 angle, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x32) <= 2) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2254 = type;
    return fn_80101A74(0x32, 4);
}

// 800FDAA4
BOOL fn_800FDAA4(int type) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x34 || state != 0x33) {
        return FALSE;
    }
    player->_2254 = type;
    return fn_80101A74(0x34, 4);
}

// 800FDB10
BOOL fn_800FDB10() {
    return fn_801019C4(4) != NULL;
}

// 800FDB40
void fn_800FDB40() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player != NULL) {
        player->_2270 |= 0x100;
    }
}

// 800FDB78
BOOL fn_800FDB78(int type) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0xA) {
        return TRUE;
    }
    player->_2244 = type;
    return fn_80101A74(0xA, 4);
}

// 800FDBDC
void fn_800FDBDC(u32 mode) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL || !(player->_2270 & 0x10)) {
        return;
    }
    if (mode >= 2) {
        player->_2270 &= ~0x20;
        return;
    }
    if (fn_800FC61C()) {
        player->_2270 &= ~0x20;
        fn_800FC604();
        return;
    }
    int kind = fn_800BA888(player->_0C34);
    if ((u32)(kind - 0xA) <= 2) {
        if (mode == 0) {
            if (kind == 0xA) {
                fn_80101A74(0x7C, 4);
            } else {
                fn_80101A74(0x22, 4);
            }
        } else {
            player->_2270 &= ~0x20;
        }
    } else {
        fn_80101A74(0x22, 4);
    }
}

// 800FDCC0
BOOL fn_800FDCC0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return TRUE;
    }
    return (player->_2270 & 0x20) == 0;
}

// 800FDD04
BOOL fn_800FDD04() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x8 && (fn_8018F438(4) || fn_8018F438(5))) {
        return TRUE;
    }
    if ((u32)(state - 0x49) <= 1) {
        return TRUE;
    }
    return FALSE;
}

// 800FDD8C
BOOL fn_800FDD8C(u32 hair, u32 color) {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    if (hair >= 0x1A) {
        return FALSE;
    }
    if (color >= 8) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    current->mHair = hair;
    current->mHairColor = color;
    if (current->_83F5 != 0) {
        fn_801B8FD8(fn_800DCF58(), 0x12);
    }
    return fn_80101A74(0x4F, 4);
}

// 800FDE3C
BOOL fn_800FDE3C() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x9C, 4);
}

// 800FDE7C
BOOL fn_800FDE7C() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9C && player->mAnmId == 0xB2) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800FDEE8
BOOL fn_800FDEE8() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x9D, 4);
}

// 800FDF28
f32 fn_800FDF28() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return 0.0f;
    }
    if (player->mAnmId == 0xB2) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            return fn_802B0980(anm);
        }
    }
    return 0.0f;
}

// 800FDF7C
BOOL fn_800FDF7C() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    u16 mii = 0;
    if (!current->isFlag0(0x28)) {
        if (!fn_800FE360()) {
            return FALSE;
        }
        if (!fn_802C4404(&current->_83ED, &mii)) {
            return FALSE;
        }
    }
    player->_22B6 = mii;
    fn_801B8FD8(fn_800DCF58(), 0x11);
    fn_801B910C(fn_800DCF58(), 1, dItem::ITEM_ID_NONE);
    fn_801B8898(fn_800DCF58(), 1, dItem::ITEM_ID_NONE);
    fn_801B910C(fn_800DCF58(), 2, dItem::ITEM_ID_NONE);
    fn_801B8898(fn_800DCF58(), 2, dItem::ITEM_ID_NONE);
    return TRUE;
}

// 800FE094
BOOL fn_800FE094() {
    if (fn_800FDF7C()) {
        return fn_80101A74(0x4C, 4);
    }
    return FALSE;
}

// 800FE0D0
BOOL fn_800FE0D0() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (current->_83F5 != 0) {
        fn_801B8FD8(fn_800DCF58(), 0x12);
    }
    return fn_80101A74(0x4C, 4);
}

// 800FE140
BOOL fn_800FE140() {
    if (fn_800FDF7C()) {
        return fn_80101A74(0x4B, 4);
    }
    return FALSE;
}

// 800FE17C
BOOL fn_800FE17C(u16 mii) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    if (!fn_800FE360()) {
        return FALSE;
    }
    if (!fn_802B3BD0(0, mii, 0)) {
        return FALSE;
    }
    dMiiData_c data;
    if (fn_802C5A0C(&data, 0, 0, mii) != 0) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    memcpy(&current->_83ED, data._2C, sizeof(data._2C));
    player->_22B6 = mii;
    fn_801B8FD8(fn_800DCF58(), 0x11);
    fn_801B910C(fn_800DCF58(), 1, dItem::ITEM_ID_NONE);
    fn_801B910C(fn_800DCF58(), 2, dItem::ITEM_ID_NONE);
    return fn_80101A74(0x50, 4);
}

// 800FE2D4
BOOL fn_800FE2D4() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (current->_83F5 == 0) {
        return FALSE;
    }
    fn_801B8FD8(fn_800DCF58(), 0x12);
    return fn_80101A74(0x4B, 4);
}

// 800FE360
u32 fn_800FE360() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 10) & 1;
}

// 800FE39C
BOOL fn_800FE39C(s16 angle, int type, int target, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2244 = type;
    player->_2298 = target;
    if (target == -1) {
        return fn_80101A74(0x9, 4);
    }
    return fn_80101A74(0xC, 4);
}

// 800FE484
BOOL fn_800FE484(s16 angle, int target, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0xC) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2274 |= 0x20;
    player->_2298 = target;
    return fn_80101A74(0xC, 4);
}

// 800FE550
BOOL fn_800FE550(s16 angle, int type, BOOL useC, f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x9) {
        return TRUE;
    }
    setTargetXZ(player, x, z);
    player->_22C4 = angle;
    player->_2244 = type;
    if (useC) {
        return fn_80101A74(0xC, 4);
    }
    return fn_80101A74(0x9, 4);
}

// 800FE634
BOOL fn_800FE634() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int anm = player->mAnmId;
    if (anm == 0x5 || anm == 0x8) {
        return TRUE;
    }
    return FALSE;
}

// 800FE688
void fn_800FE688(int *outState, mVec3_c *outPos, s16 *outAngle) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return;
    }
    int state = player->mState;
    switch (state) {
    case 0x2E:
    case 0x38:
        break;
    case 0x8:
        if (player->mAnmId == 0x31) {
            state = 0x38;
        } else if (player->mAnmId == 0x37) {
            state = 0x2E;
        }
        break;
    default:
        state = 0x8;
        break;
    }
    *outState = state;
    *outPos = player->mPos;
    *outAngle = player->mAngle.y;
}

// 800FE74C
BOOL fn_800FE74C(const mVec3_c *pos, int idx, f32 speed) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    player->_2274 |= 0x8000;
    BOOL result = fn_800FE7E0(pos, idx, speed);
    if (!result) {
        player->_2274 &= ~0x8000;
    }
    return result;
}

// 800FE7E0
BOOL fn_800FE7E0(const mVec3_c *pos, int idx, f32 speed) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x3) {
        return TRUE;
    }
    if (state != 0x8) {
        return FALSE;
    }
    player->_2068 = *pos;
    player->_21BC = speed;
    return fn_80101A74(0x3, idx);
}

// 800FE88C
BOOL fn_800FE88C(s16 angle, int idx) {
    return fn_800FE89C(angle, 500, idx);
}

// 800FE89C
BOOL fn_800FE89C(s16 angle, int frames, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x4) {
        return TRUE;
    }
    if (state != 0x8) {
        return FALSE;
    }
    if (frames <= 0) {
        frames = 500;
    }
    player->_22C4 = angle;
    player->_22D0 = frames;
    return fn_80101A74(0x4, idx);
}

// 800FE938
BOOL fn_800FE938(const mVec3_c *target, int a, BOOL pitch, int c) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    if (a <= 0) {
        a = 200;
    }
    if (c <= 0) {
        c = 200;
    }
    s16 yaw = 0;
    s16 tilt = 0;
    if (target != NULL) {
        yaw = (s16)(dActor_c::targetAngleY(&player->mPos, target) - player->mAngle.y);
        if (yaw > sLookYawMax) {
            yaw = sLookYawMax;
        } else if (yaw < -sLookYawMax) {
            yaw = -sLookYawMax;
        }
        if (pitch) {
            mVec3_c diff = player->_0654 - *target;
            f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
            tilt = cM::atan2s(target->y - player->_0654.y, dist);
            if (tilt > sLookPitchMax) {
                tilt = sLookPitchMax;
            } else if (tilt < -sLookPitchMax) {
                tilt = -sLookPitchMax;
            }
        }
    }
    player->_22C8 = yaw;
    player->_22C6 = tilt;
    player->_22CA = a;
    player->_22CC = c;
    player->_2270 |= 0x40;
    return TRUE;
}

// 800FEA9C
BOOL fn_800FEA9C(int idx) {
    return fn_800FEAE4(0x3, idx);
}

// 800FEAA8
BOOL fn_800FEAA8(int idx) {
    return fn_800FEAE4(0x4, idx);
}

// 800FEAB4
BOOL fn_800FEAB4(int idx) {
    return fn_800FEAE4(0x0, idx);
}

// 800FEAC0
BOOL fn_800FEAC0(int idx) {
    return fn_800FEAE4(0x1, idx);
}

// 800FEACC
BOOL fn_800FEACC(int idx) {
    return fn_800FEAE4(0x8, idx);
}

// 800FEAD8
BOOL fn_800FEAD8(int idx) {
    return fn_800FEAE4(0x7, idx);
}

// 800FEAE4
BOOL fn_800FEAE4(int state, int idx) {
    dPlayerActor_c *player = fn_80101A28(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == state;
}

// 800FEB34
int fn_800FEB34(int idx) {
    dPlayerActor_c *player = fn_80101A28(idx);
    if (player == NULL) {
        return 0xAB;
    }
    return player->mState;
}

// 800FEB68
BOOL fn_800FEB68(const dItem::Item *shirt) {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mShirt = *shirt;
    return fn_800FF444(&equip, 1, 0);
}

// 800FEBE4
BOOL fn_800FEBE4(const dItem::Item *hat) {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mHat = *hat;
    return fn_800FF444(&equip, 2, 0);
}

// 800FEC60
BOOL fn_800FEC60(const dItem::Item *acc) {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    dEquip_c equip;
    equip.setFromPlayer();
    equip.mAcc = *acc;
    if (!equip.fn_8013A9C8()) {
        equip.mHat.mId = dItem::ITEM_ID_NONE;
    }
    return fn_800FF444(&equip, 3, 0);
}

// 800FECF8
BOOL fn_800FECF8(const dItem::Item *held) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (fn_800FEEB0()) {
        return FALSE;
    }
    if (held->mId == dItem::ITEM_ID_NONE) {
        if (fn_801625D0(5) || (fn_801625D0(9) && fn_800BD134(held))) {
            player->_22DA = 2;
            player->_223C = 1;
        }
    } else {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
        if (bitm == NULL) {
            return FALSE;
        }
        if (getHideBone(bitm) != 9) {
            return FALSE;
        }
        if (fn_801625D0(5) || (fn_801625D0(9) && fn_800BD134(held))) {
            player->_22DA = 1;
            player->_223C = 1;
        }
    }
    fn_801B910C(fn_800DCF58(), 3, held->mId);
    fn_801B8898(fn_800DCF58(), 3, held->mId);
    player->_2234 = 8;
    player->_2270 |= 0x1000000;
    return fn_80101A74(0x6, 4);
}

// 800FEEB0
BOOL fn_800FEEB0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    switch (player->mState) {
    case 0x5:
        if (player->mAnmId == 0x24) {
            return TRUE;
        }
        if (player->mAnmId == 0x26 && fn_802B0980(player->getAnm()) < 57.0f) {
            return TRUE;
        }
        break;
    case 0x6:
    case 0x4B:
    case 0x4D:
    case 0x4E:
    case 0x8B:
    case 0x98:
    case 0xA0:
        return TRUE;
    case 0x4C:
    case 0x79:
        if (fn_802B0980(player->getAnm()) < 57.0f) {
            return TRUE;
        }
        break;
    case 0x4F:
    case 0x50:
        if (!player->getAnm()->isStop() || (player->mAnmId != 0xA6 && player->mAnmId != 0xA3)) {
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// 800FEFF4
mVec3_c *fn_800FEFF4(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return NULL;
    }
    u8 *obj = player->_0C34 != NULL ? (u8 *)player->_0C34 + 0x1C : NULL;
    if (obj != NULL && obj[0x447] != 0) {
        return (mVec3_c *)(obj + 0x270);
    }
    return &player->mPos;
}

// 800FF048
void *fn_800FF048() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return NULL;
    }
    return player->_060C;
}

// 800FF080
BOOL fn_800FF080(mVec3_c *out) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    s16 angle = player->mAngle.y;
    out->x = player->mPos.x + 12.0f * nw4r::math::SinFIdx((1.0f / 256.0f) * angle);
    out->z = player->mPos.z + 12.0f * nw4r::math::CosFIdx((1.0f / 256.0f) * angle);
    mVec3_c pos(out->x, player->mPos.y, out->z);
    out->y = fn_80074974(&pos, 0);
    return TRUE;
}

// 800FF150
u32 fn_800FF150(int idx) {
    dPlayerActor_c *player = fn_80101A28(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2270 >> 13) & 1;
}

// 800FF188
u32 fn_800FF188(int idx) {
    dPlayerActor_c *player = fn_80101A28(idx);
    if (player == NULL) {
        return 0;
    }
    return (player->_2274 >> 7) & 1;
}

// 800FF1C0
int fn_800FF1C0(int *x, int *z, int idx) {
    dPlayerActor_c *player = fn_80101A28(idx);
    if (player == NULL) {
        return 7;
    }
    if (x == NULL || z == NULL) {
        return 7;
    }
    if (!(player->_2274 & 0x80) && !(player->_2270 & 0x2000)) {
        return 7;
    }
    switch (player->mState) {
    case 0x1:
        *x = player->_228C;
        *z = player->_2290;
        return 0;
    case 0x3D:
        fn_8010042C(player, x, z);
        return 1;
    case 0x12:
        fn_8010042C(player, x, z);
        return 2;
    case 0x13:
        fn_8010042C(player, x, z);
        return 3;
    case 0xD:
        *x = (int)player->mPos.x >> 5;
        *z = (int)player->mPos.z >> 5;
        return 4;
    case 0x40:
        fn_8010042C(player, x, z);
        return 5;
    case 0x41:
        fn_8010042C(player, x, z);
        return 6;
    }
    return 7;
}

// 800FF354
BOOL fn_800FF354(int *outState) {
    int tmp;
    if (outState == NULL) {
        outState = &tmp;
    }
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        *outState = 0xAB;
        return FALSE;
    }
    *outState = player->mState;
    if (player->_2270 & 1) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x1A) {
        fn_801710BC(fn_800DCF58(), 0);
        *outState = 0x1D;
        fn_80101A74(0x1D, 4);
        return FALSE;
    }
    if (state != 0x0 && state != 0x1) {
        return FALSE;
    }
    if (!fn_8018EB8C()) {
        return FALSE;
    }
    dPlayerMgr_c::getCurrentPlayer()->setFlag0(2);
    *outState = 0x62;
    return fn_80101A74(0x62, 4);
}

// 800FF444
BOOL fn_800FF444(const dEquip_c *equip, int mode, int kind) {
    if (equip == NULL) {
        return FALSE;
    }
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (fn_800FEEB0()) {
        return FALSE;
    }
    if (kind == 1) {
        player->_22E4 = mode;
        return fn_800FE140();
    }
    if (kind == 2) {
        player->_22E4 = mode;
        return fn_800FE2D4();
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }

    u8 tmp[0x10];
    BOOL changeShirt = FALSE;
    BOOL changeHat = FALSE;
    BOOL changeAcc = FALSE;

    dItem::Item shirt = equip->mShirt;
    if (!shirt.isSame(current->mEquipment.mShirt) || ((mode == 1 || mode == 5) && fn_800B7310(tmp, &shirt))) {
        changeShirt = TRUE;
    }
    dItem::Item hat = equip->mHat;
    if (!hat.isSame(current->mEquipment.mHat) || ((mode == 2 || mode == 5) && fn_800B89CC(tmp, &hat))) {
        changeHat = TRUE;
    }
    dItem::Item acc = equip->mAcc;
    if (!acc.isSame(current->mEquipment.mAcc)) {
        changeAcc = TRUE;
    }

    if (changeShirt && shirt.mId != dItem::ITEM_ID_NONE) {
        u8 tmp2[0x4];
        if (fn_800B72A4(&shirt) || fn_800B7310(tmp2, &shirt)) {
            player->_22B4 |= 0x100;
            fn_801B910C(fn_800DCF58(), 0, shirt.mId);
            fn_801B8898(fn_800DCF58(), 0, shirt.mId);
        }
    }

    if (changeHat) {
        int bone = 0xA;
        if (hat.mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(hat);
            if (bitm != NULL) {
                bone = getHideBone(bitm);
            }
        }
        if (fn_80100358(bone, player)) {
            if (current->_83F5 != 0) {
                fn_801B8FD8(fn_800DCF58(), 0x12);
            }
            fn_801B910C(fn_800DCF58(), 1, hat.mId);
            fn_801B8898(fn_800DCF58(), 1, hat.mId);
        }
    }

    if (changeAcc) {
        if (acc.mId == dItem::ITEM_ID_NONE) {
            player->_22B4 = (player->_22B4 & ~0x40) | 0x80;
            fn_801B910C(fn_800DCF58(), 2, acc.mId);
            fn_801B8898(fn_800DCF58(), 2, acc.mId);
        } else {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(acc);
            if (bitm != NULL && getHideBone(bitm) == 8) {
                player->_22B4 = (player->_22B4 & ~0x80) | 0x40;
                if (current->_83F5 != 0) {
                    fn_801B8FD8(fn_800DCF58(), 0x12);
                }
                fn_801B910C(fn_800DCF58(), 2, acc.mId);
                fn_801B8898(fn_800DCF58(), 2, acc.mId);
            }
        }
    }

    switch (mode) {
    case 0:
        player->_2238 = 1;
        return fn_80101A74(0x4E, 4);
    case 1:
    case 2:
    case 3:
        player->_2238 = 1;
        player->_2234 = 8;
        return fn_80101A74(0x5, 4);
    case 4:
    case 5:
        player->_2238 = 1;
        player->_2234 = 8;
        return fn_80101A74(0x4D, 4);
    case 6:
        player->_2238 = 1;
        player->_2234 = 8;
        return fn_80101A74(0x8B, 4);
    }
    return FALSE;
}

// 800FF840
BOOL fn_800FF840() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return TRUE;
    }
    if (player->mState == 0x8C) {
        return TRUE;
    }
    if ((player->_2270 & 0x200000) && fn_80069308(fn_8006996C())) {
        return TRUE;
    }
    return FALSE;
}

// 800FF8B0
BOOL fn_800FF8B0() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current != NULL && current->mEquipment.mHeld.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (fn_801625D0(5) || (fn_801625D0(9) && fn_800BD134(&current->mEquipment.mHeld))) {
        return TRUE;
    }
    return FALSE;
}

// 80475A00: per hairstyle bit mask (fn_800FF94C).
static const u16 sHairBits[26] = {
    0x0C0, 0x060, 0x180, 0x208, 0x102, 0x00A, 0x005, 0x081, 0x210, 0x006, 0x044, 0x030, 0x400,
    0x028, 0x084, 0x021, 0x240, 0x003, 0x084, 0x300, 0x110, 0x042, 0x011, 0x108, 0x0A0, 0x400,
};

// 800FF94C
BOOL fn_800FF94C(u8 hair, int bit) {
    return (sHairBits[hair] & (1 << bit)) != 0;
}

// 800FF978
BOOL fn_800FF978(int bit) {
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    return fn_800FF94C(current->mHair, bit);
}

// 80475A34 / 80475A40: fn_800FF9C4, by gender.
static const u8 lbl_80475A34[10] = {9, 1, 8, 2, 7, 4, 6, 5, 3, 0};
static const u8 lbl_80475A40[10] = {9, 4, 8, 0, 6, 1, 7, 3, 5, 2};

// 800FF9C4
u8 fn_800FF9C4(u32 idx, u32 gender) {
    if (idx >= 10) {
        return 0;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return 0;
    }
    if (gender == 2) {
        gender = player->_83F7;
    }
    if (gender == 0) {
        return lbl_80475A34[idx];
    }
    return lbl_80475A40[idx];
}

// 800FFA50
BOOL fn_800FFA50(const mVec3_c *a, int b, f32 f) {
    if (fn_800FA724() && fn_800FFDC0(a, b, f)) {
        return TRUE;
    }
    return FALSE;
}

// 800FFAC0
BOOL fn_800FFAC0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if ((player->_2270 & 0x40000000) && player->mState == 0x1) {
        return TRUE;
    }
    return FALSE;
}

// 800FFB18
BOOL fn_800FFB18() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0xD && state < 0x17) || (state >= 0x18 && state < 0x21) || (state >= 0x3E && state < 0x48) ||
        state == 0x7D || (state >= 0x89 && state < 0x8B) || (state >= 0x8D && state < 0x8F) ||
        (state >= 0x99 && state < 0x9C) || state == 0xA2) {
        return TRUE;
    }
    return FALSE;
}

// 800FFBE4
BOOL fn_800FFBE4() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    return (u32)(player->mState - 0x3C) <= 1;
}

// 800FFC38
BOOL fn_800FFC38() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x51 && state < 0x54) || state == 0x5D || state == 0x7E) {
        return TRUE;
    }
    return FALSE;
}

// 800FFCA8
BOOL fn_800FFCA8() {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x23 && state < 0x32) || (state >= 0x35 && state < 0x3C)) {
        return TRUE;
    }
    return FALSE;
}

// 800FFD10
BOOL fn_800FFD10() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0xE || state == 0x1F) {
        return TRUE;
    }
    return FALSE;
}

// 800FFD64
u16 fn_800FFD64() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return dItem::ITEM_ID_NONE;
    }
    int state = player->mState;
    if (state != 0xE && state != 0x1F) {
        return dItem::ITEM_ID_NONE;
    }
    return player->_218A;
}

// 800FFDC0
BOOL fn_800FFDC0(const mVec3_c *target, int maxAngle, f32 maxDist) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    maxDist = maxDist * maxDist;
    mVec3_c diff = player->mPos - *target;
    f32 dist = diff.x * diff.x + diff.z * diff.z;
    if (dist == 0.0f) {
        return TRUE;
    }
    if (dist > maxDist) {
        return FALSE;
    }
    s16 angle = dActor_c::targetAngleY(&player->mPos, target);
    return sLib::distanceAngle(angle, player->mAngle.y) <= maxAngle;
}

// 800FFEAC
void fn_800FFEAC(mMtx_c *mtx, int arg, const mAng *angle) {
    mVec3_c pos;
    u32 tilt = fn_800827A8(&pos, arg);
    PSMTXTrans(*mtx, pos.x, pos.y, pos.z);
    mtx->XrotM(mAng(tilt >> 16));
    mtx->YrotM(*angle);
}

// 800FFF2C
void fn_800FFF2C(mMtx_c *mtx, int arg, const mAng3_c *angle) {
    mVec3_c pos;
    u32 tilt = fn_800827A8(&pos, arg);
    PSMTXTrans(*mtx, pos.x, pos.y, pos.z);
    mtx->XrotM(mAng((s16)(tilt >> 16) + angle->x.mAngle));
    mtx->ZrotM(angle->z);
    mtx->YrotM(angle->y);
}

// 80475A50: random item indices per gender (fn_800FFFCC).
static const int lbl_80475A50[8][2] = {
    {0x376, 0x37D}, {0x37C, 0x3A8}, {0x383, 0x3AF}, {0x384, 0x3C2},
    {0x391, 0x3D6}, {0x3A6, 0x3F2}, {0x410, 0x44B}, {0x425, 0x44D},
};

// 800FFFCC
u16 fn_800FFFCC(int gender) {
    int r = cM::rndInt(800);
    dItem::Item item(lbl_80475A50[r & 7][gender & 1]);
    if (item.getKind() != 4) {
        item.setFromIndex(dItem::ITEM_IDX_FOUR_BALL_SHIRT);
    }
    return item.mId;
}

// 8010003C
void fn_8010003C() {
    fn_801B961C();
    fn_800BD2D8();
    fn_800B8E7C();
    fn_800B7658();
}

// 80100068
void fn_80100068(int value) {
    fn_801B9658(value);
    fn_800BD2E4((u8)value);
    fn_800B8E88((u8)value);
    fn_800B7664((u8)value);
}

// 801000AC
BOOL fn_801000AC(int slot) {
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    if (fn_80101584(slot, &current->mEquipment.mShirt) || fn_80101584(slot, &current->mEquipment.mHat)) {
        return TRUE;
    }
    return FALSE;
}

// 80100128
BOOL fn_80100128(f32 *out, int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mAnmId - 0x3C) <= 1) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            *out = fn_802B0980(anm);
            return TRUE;
        }
    }
    return FALSE;
}

// 80100194
f32 fn_80100194(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c) {
    f32 hb = fn_80074974(b, 0);
    f32 ha = fn_80074974(a, 0);
    f32 hc = c->y;
    if ((hb <= ha && hb >= hc) || (hb >= ha && hb <= hc)) {
        return hb;
    }
    return hc;
}

// 80100234
void fn_80100234() {
    fn_801B7C10();
}

// 80100238
void fn_80100238(u32 idx, int value) {
    if (idx < 4) {
        lbl_805D2440.mSlots[idx] = value;
    }
}

// 80100258
void fn_80100258(u32 idx) {
    if (idx < 4) {
        lbl_805D2440.mSlots[idx] = 7;
    }
}

// 8010027C
void fn_8010027C() {
    fn_801017DC();
    for (int i = 0; i < 4; i++) {
        fn_80100258(i);
    }
}

// 801002BC
void fn_801002BC() {
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, i);
        if (player != NULL && player->mPID.isValid()) {
            player->clearFlag0(2);
        }
    }
}

// 80100330
int fn_80100330(u32 idx) {
    if (idx >= 4) {
        return 7;
    }
    return lbl_805D2440.mSlots[idx];
}

// 80100358
BOOL fn_80100358(u32 bone, dPlayerActor_c *player) {
    if (player == NULL) {
        return FALSE;
    }
    switch (bone) {
    case 0:
    case 1:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x1;
        break;
    case 2:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x2;
        break;
    case 3:
    case 4:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x4;
        break;
    case 5:
    case 6:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x8;
        break;
    case 7:
        player->_22B4 = (player->_22B4 & ~0x3F) | 0x10;
        break;
    case 10:
        player->_22B4 = (player->_22B4 & ~0x1F) | 0x20;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// 80100410
void fn_80100410(dPlayerActor_c *player) {
    if (player != NULL) {
        player->_2270 &= ~1;
        player->_2270 &= ~0x100;
    }
}

// 8010042C
void fn_8010042C(dPlayerActor_c *player, int *x, int *z) {
    *x = player->_2190;
    *z = player->_2194;
}

// 80100440
BOOL fn_80100440(int a, int b, int c, int d, int e, f32 f1, f32 f2, f32 f3) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194A2C(a, b, c, d, player, e, fn_80100548(), f1, 0.0f, f2, 0.0f, f3)) {
        return FALSE;
    }
    if (!fn_80194AA0(0, player, 16.0f)) {
        return FALSE;
    }
    return fn_80101A74(0x63, 4);
}

// 80100548
f32 fn_80100548() {
    return -4.0f;
}

// 80100550
BOOL fn_80100550(int a, int b, int c, int d, int e, f32 f1, f32 f2) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194A2C(a, b, c, d, player, e, fn_80100548(), f1, 0.0f, f2, 0.0f, 0.0f)) {
        return FALSE;
    }
    if (!fn_80194AA0(1, player, 16.0f)) {
        return FALSE;
    }
    return fn_80101A74(0x6A, 4);
}

// 80100644
BOOL fn_80100644(BOOL a) {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    if (a) {
        return fn_80101A74(0x66, 4);
    }
    return fn_80101A74(0x67, 4);
}

// 801006A8
BOOL fn_801006A8() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (!fn_80194AA0(fn_80194B30(), player, 16.0f)) {
        return FALSE;
    }
    return fn_80101A74(0x6E, 4);
}

// 80100714
BOOL fn_80100714(u8 shoeColor, u8 b) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0xA7) {
        return FALSE;
    }
    dPrivateData_c *current = dPlayerMgr_c::getCurrentPlayer();
    if (current == NULL) {
        return FALSE;
    }
    current->mShoeColor = shoeColor;
    current->_83F7 = b;
    player->_2234 = 0x1E;
    return TRUE;
}

// 801007AC
BOOL fn_801007AC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x85) <= 2) {
        return FALSE;
    }
    return fn_80101A74(0x85, 4);
}

// 80100804
BOOL fn_80100804() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x87 || state != 0x86) {
        return FALSE;
    }
    return fn_80101A74(0x87, 4);
}

// 80100860
BOOL fn_80100860() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x1A) {
        fn_801710BC(fn_800DCF58(), 0);
        fn_80101A74(0x1D, 4);
        return FALSE;
    }
    if (state != 0x0 && state != 0x1) {
        return FALSE;
    }
    return fn_80101A74(0x6F, 4);
}

// 801008E4
BOOL fn_801008E4(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->_2270 & 0x20)) {
        return FALSE;
    }
    if (player->_0C34 != NULL && fn_800BA888(player->_0C34) == 4) {
        return TRUE;
    }
    return FALSE;
}

// 8010094C
BOOL fn_8010094C(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    return player->mAnmId == 0x3;
}

// 8010098C
BOOL fn_8010098C() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x70) {
        return TRUE;
    }
    return fn_80101A74(0x70, 4);
}

// 801009E0
BOOL fn_801009E0(u32 type) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (type >= 0x1E) {
        return FALSE;
    }
    if (player->mState == 0x71) {
        return FALSE;
    }
    player->_22DE = type;
    return fn_80101A74(0x71, 4);
}

// 80100A54
BOOL fn_80100A54() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x84) {
        return FALSE;
    }
    player->_22DD = 0;
    return fn_80101A74(0x84, 4);
}

// 80100AB0
BOOL fn_80100AB0() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x84) {
        return FALSE;
    }
    if (player->mAnmId == 0x95) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL && anm->isStop()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80100B24
BOOL fn_80100B24() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState == 0x84) {
        return FALSE;
    }
    player->_22DD = 6;
    return fn_80101A74(0x84, 4);
}

// 80100B80
BOOL fn_80100B80() {
    return fn_80100AB0();
}

// 80100B84
BOOL fn_80100B84(f32 x, f32 z) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if ((u32)(player->mState - 0x72) <= 1) {
        return FALSE;
    }
    setTargetXZ(player, x, z);
    return fn_80101A74(0x72, 4);
}

// 80100C28
BOOL fn_80100C28() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x73 || state != 0x72) {
        return FALSE;
    }
    return fn_80101A74(0x73, 4);
}

// 80100C84
BOOL fn_80100C84(BOOL a) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->_2270 & 1) {
        return FALSE;
    }
    int state = player->mState;
    if (state != 0x0 && state != 0x1) {
        return FALSE;
    }
    if (!fn_8018ECA0()) {
        return FALSE;
    }
    if (!a) {
        player->_2274 |= 1;
    } else {
        player->_2274 &= ~1;
    }
    return fn_80101A74(0x74, 4);
}

// 80100D44
BOOL fn_80100D44() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x7 && state < 0x9) || (state >= 0x58 && state < 0x5C)) {
        return TRUE;
    }
    if (player->_2270 & 1) {
        return TRUE;
    }
    return fn_8018F438(9) != 0;
}

// 80100DD0
BOOL fn_80100DD0(int idx) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if ((state >= 0x1 && state < 0x3) || state == 0x27 || state == 0x29) {
        return TRUE;
    }
    return FALSE;
}

// 80100E3C
BOOL fn_80100E3C() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x8F, 4);
}

// 80100E7C
BOOL fn_80100E7C() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == 0x90;
}

// 80100EC0
BOOL fn_80100EC0() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x91, 4);
}

// 80100F00
BOOL fn_80100F00(s16 angle, f32 frame) {
    dPlayerActor_c *player = fn_80101A28(4);
    if (player == NULL) {
        return FALSE;
    }
    player->mAngle.x = 0;
    player->mAngle.y = angle;
    player->mAngle.z = 0;
    if (fn_80101A74(0x92, 4)) {
        m3d::fanm_c *anm = player->getAnm();
        if (anm != NULL) {
            fn_802B0994(anm, frame);
        }
        return TRUE;
    }
    return FALSE;
}

// 80100F98
BOOL fn_80100F98() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    return player->mState == 0x93;
}

// 80100FDC
BOOL fn_80100FDC() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x94, 4);
}

// 8010101C
u8 fn_8010101C() {
    return lbl_805D2440._194A8;
}

// 80101030
void fn_80101030() {
    lbl_805D2440._194A8 = TRUE;
}

// 80101048
void fn_80101048() {
    lbl_805D2440._194A8 = FALSE;
}

// 80101060
BOOL fn_80101060(const dItem::Item *held) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    if (!fn_801625D0(5)) {
        return FALSE;
    }
    if (held->mId != dItem::ITEM_ID_NONE) {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*held);
        if (bitm == NULL) {
            return FALSE;
        }
        if (getHideBone(bitm) != 9) {
            return FALSE;
        }
        player->_22DA = 1;
        player->_223C = 1;
        fn_801B910C(fn_800DCF58(), 3, held->mId);
        fn_801B8898(fn_800DCF58(), 3, held->mId);
    }
    return fn_80101A74(0x98, 4);
}

// 80101168
BOOL fn_80101168() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    return fn_80101A74(0x95, 4);
}

// 801011BC
BOOL fn_801011BC() {
    return TRUE;
}

// 801011C4
BOOL fn_801011C4(mVec3_c *out) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (!(player->_2270 & 0x10)) {
        return FALSE;
    }
    if (player->mState != 0x95) {
        return FALSE;
    }
    if (player->_2234 != 0x20) {
        return FALSE;
    }
    *out = player->_20EC;
    return TRUE;
}

// 8010125C
BOOL fn_8010125C(s16 frames, f32 speed) {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0x8) {
        return FALSE;
    }
    mVec3_c target(1082.0f, 0.0f, 390.0f);
    player->_2068 = target;
    player->_21BC = speed;
    player->_22D0 = frames;
    return fn_80101A74(0xA6, 4);
}

// 801012FC
BOOL fn_801012FC() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    if (player->mState != 0xA7) {
        return FALSE;
    }
    return fn_80101A74(0xA8, 4);
}

// 80101350
BOOL fn_80101350() {
    dPlayerActor_c *player = fn_801019C4(4);
    if (player == NULL) {
        return FALSE;
    }
    int state = player->mState;
    if (state == 0x76) {
        return TRUE;
    }
    if (state != 0x8) {
        return FALSE;
    }
    return fn_80101A74(0x76, 4);
}

// 801013B4
BOOL fn_801013B4() {
    if (fn_801019C4(4) == NULL) {
        return FALSE;
    }
    return fn_80101A74(0x7, 4);
}

// 801013F4
u32 *fn_801013F4() {
    return &lbl_805D2440._194A0;
}

// 80101408
void fn_80101408() {
    lbl_805D2440._194A0 = 0xD2F0;
}

// 80101424
void fn_80101424() {
    fn_8010027C();
    fn_80101408();
    lbl_805D2440._194A4 = 0;
    fn_801014E8();
    lbl_805D2440._194A6 = 0;
    lbl_805D2440._194A7 = 0;
    lbl_805D2440._194A8 = 0;
    lbl_805D2440.mLocked = 0;
    lbl_805D2440._194AA = 0;
    lbl_805D2440._194AB = 0;
    fn_8010003C();
}

// 80101490
BOOL fn_80101490() {
    return lbl_805D2440._194A4;
}

// 801014A4
void fn_801014A4() {
    lbl_805D2440._194A4 = TRUE;
}

// 801014BC
u8 fn_801014BC() {
    return lbl_805D2440._194A5;
}

// 801014D0
void fn_801014D0() {
    lbl_805D2440._194A5 = TRUE;
}

// 801014E8
void fn_801014E8() {
    lbl_805D2440._194A5 = FALSE;
}

// 80101500
BOOL fn_80101500() {
    return lbl_805D2440._194A6;
}

// 80101514
void fn_80101514() {
    lbl_805D2440._194A6 = TRUE;
}

// 8010152C
u8 fn_8010152C() {
    return lbl_805D2440._194A7;
}

// 80101540
void fn_80101540() {
    lbl_805D2440._194A7 = TRUE;
}

// 80101558
u8 fn_80101558() {
    return lbl_805D2440._194AB;
}

// 8010156C
void fn_8010156C() {
    lbl_805D2440._194AB = TRUE;
}

// 80101584
BOOL fn_80101584(int slot, const dItem::Item *item) {
    if (item->isOrgDesign() && (dItem::seeker_c::get()->findLike(*item) & 7) == slot) {
        return TRUE;
    }
    return FALSE;
}

// 801015F4
u32 fn_801015F4() {
    return 0xEE880;
}

// 80101600
u32 fn_80101600() {
    return 0x1800;
}

// 80101608
u32 fn_80101608() {
    return 0xC00;
}

// 80101610
u32 fn_80101610() {
    return 0x800;
}

// 80101618
u32 fn_80101618() {
    return 0x38820;
}

// 80101624
dPrivateData_c *dPlayerMgr_c::getPlayer(int player) {
    BOOL inTown = FALSE;
    if (player >= 0 && player < 4) {
        inTown = TRUE;
    }
    if (inTown) {
        return dPrivateData_c::getChecked(dSaveData_c::getTown()->mPlayers, player);
    }
    return lbl_805D2440.mGuests.get(player);
}

// 80101694
dPrivateData_c *dPlayerMgr_c::getNetPlayer(int member) {
    return getPlayer(fn_80100330(member));
}

// 801016B8
dPrivateData_c *dPlayerMgr_c::getPlayer(const dPersonalID_c *pid) {
    return getPlayer(fn_801017EC(pid));
}

// 801016DC
dPrivateData_c *dPlayerMgr_c::getPlayerRaw(int player) {
    BOOL inTown = FALSE;
    if (player >= 0 && player < 4) {
        inTown = TRUE;
    }
    if (inTown) {
        return dPrivateData_c::getRaw(dSaveData_c::getRaw()->mPlayers, player);
    }
    return lbl_805D2440.mGuests.getRaw(player);
}

// 8010174C
dPrivateData_c *dPlayerMgr_c::getNetPlayerRaw(int member) {
    return getPlayerRaw(fn_80100330(member));
}

// 80101770
dPrivateData_c *dPlayerMgr_c::getCurrentPlayer() {
    return getNetPlayer(fn_800DCF58());
}

// 80101794
dPrivateData_c *dPlayerMgr_c::getCurrentPlayerRaw() {
    return getNetPlayerRaw(fn_800DCF58());
}

// 801017B8
int fn_801017B8() {
    return fn_80100330(fn_800DCF58());
}

// 801017DC
void fn_801017DC() {
    lbl_805D2440.mGuests.clear();
}

// 801017EC
int fn_801017EC(const dPersonalID_c *pid) {
    int slot = 7;
    if (pid->isValid()) {
        int idx = dPrivateData_c::find(dSaveData_c::getTown()->mPlayers, pid);
        if (idx == -1) {
            for (int i = 0; i < 3; i++) {
                const dPersonalID_c *guest = &lbl_805D2440.mGuests.mPlayers[i].mPID;
                BOOL same = FALSE;
                if (pid->land == guest->land && pid->isSamePlayer(guest)) {
                    same = TRUE;
                }
                if (same) {
                    slot = i + 4;
                    break;
                }
            }
        } else {
            slot = idx;
        }
    }
    return slot;
}

// 801018DC
int fn_801018DC(const dPlayerID_c *id) {
    int slot = 7;
    if (id->isValid()) {
        int idx = dPrivateData_c::findByPlayerID(dSaveData_c::getTown()->mPlayers, id);
        if (idx == -1) {
            for (int i = 0; i < 3; i++) {
                if (id->isSame(&lbl_805D2440.mGuests.mPlayers[i].mPID.player)) {
                    slot = i + 4;
                    break;
                }
            }
        } else {
            slot = idx;
        }
    }
    return slot;
}

// 80101990
void fn_80101990(dPlayerMgr_c::ChangeStateFn fn, int idx) {
    lbl_805D2440.mChangeState[idx] = fn;
}

// 801019A4
void fn_801019A4(dPlayerActor_c *actor, u32 idx) {
    if (idx < 4) {
        lbl_805D2440.mActors[idx] = actor;
    }
}

// 801019C4
dPlayerActor_c *fn_801019C4(int idx) {
    if ((u32)idx > 4) {
        return NULL;
    }
    if (idx == 4) {
        idx = fn_800DCF58();
    }
    dPlayerActor_c *actor = lbl_805D2440.mActors[idx];
    if (actor != NULL && (actor->_2270 & 0x2000000)) {
        return NULL;
    }
    return actor;
}

// 80101A28
dPlayerActor_c *fn_80101A28(int idx) {
    if ((u32)idx > 4) {
        return NULL;
    }
    if (idx == 4) {
        idx = fn_800DCF58();
    }
    return lbl_805D2440.mActors[idx];
}

// 80101A74
BOOL fn_80101A74(int state, int idx) {
    if (idx > 4) {
        return FALSE;
    }
    if (idx == 4) {
        idx = fn_800DCF58();
    }
    dPlayerMgr_c::ChangeStateFn fn = lbl_805D2440.mChangeState[idx];
    if (fn == NULL) {
        return FALSE;
    }
    fn(idx, state);
    return TRUE;
}

// 80101AEC
dGuestPlayers_c::dGuestPlayers_c() {}

// 80101B38
dGuestPlayers_c::~dGuestPlayers_c() {}

// 80101BA0
void dGuestPlayers_c::clear() {
    for (int i = 0; i < 3; i++) {
        mPlayers[i].clear();
    }
}

// 80101BF0
int dGuestPlayers_c::getIndex(int slot) {
    BOOL guest = FALSE;
    if (slot >= 4 && slot < 7) {
        guest = TRUE;
    }
    if (!guest) {
        return -1;
    }
    return slot - 4;
}

// 80101C1C
dPrivateData_c *dGuestPlayers_c::get(int slot) {
    int i = getIndex(slot);
    if (i == -1) {
        return NULL;
    }
    return &mPlayers[i];
}

// 80101C68
dPrivateData_c *dGuestPlayers_c::getRaw(int slot) {
    int i = getIndex(slot);
    if (i == -1) {
        return NULL;
    }
    return &mPlayers[i];
}

// 80101CB4
void fn_80101CB4() {
    fn_801B91B8();
}

// 80101CB8
void fn_80101CB8() {
    fn_801B90C0();
}

// 80101CBC
void fn_80101CBC() {
    fn_801B835C();
}

// 80101CC0
void fn_80101CC0(int idx, const mVec3_c *value) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player != NULL) {
        player->_2308 = *value;
    }
}

// 80101D0C
void fn_80101D0C(int idx, int a, int b) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player != NULL && !fn_801021B0(player->mState)) {
        player->_2230 = a;
        player->_226C = b;
    }
}

// 80101D70
void fn_80101D70(int idx, int value) {
    dPlayerActor_c *player = fn_801019C4(idx);
    if (player != NULL) {
        player->_2280 = value;
    }
}

// 80101DA8
void fn_80101DA8(int idx, int value) {
    fn_801B94E8(idx, value);
}

// 80101DAC
void fn_80101DAC(int idx, const u8 *value) {
    fn_801B9518(idx, value);
}

// 80101DB0
BOOL fn_80101DB0(int idx, int state) {
    return fn_80101A74(state, idx);
}

// 80101DC0
void fn_80101DC0() {
    fn_801B9258();
}

// 80101DC4
void fn_80101DC4() {
    fn_801B9304();
}

// 80101DC8
BOOL fn_80101DC8(int idx, int b, int state, const u8 *param) {
    fn_80101DA8(idx, state);
    fn_80101DAC(idx, param);
    return fn_80101EB0(idx, b, state, param);
}

// 80101E3C
BOOL fn_80101E3C(int idx, int b, int state, const u8 *param) {
    fn_80101DA8(idx, state);
    fn_80101DAC(idx, param);
    return fn_80101EB0(idx, b, state, param);
}

// 80101EB0
BOOL fn_80101EB0(int idx, int b, int state, const u8 *param) {
    switch (state) {
    case 0x12:
    case 0x3C:
    case 0x3D:
    case 0x55:
    case 0x57:
    case 0x5B:
        fn_800A3044(param[0], param[1], 0);
        break;
    case 0x13:
    case 0x41:
        if (param[5] != 0) {
            fn_800A3044(param[2], param[3], 0);
        }
        break;
    case 0x40:
        fn_800A3044(param[4], param[5], 0);
        break;
    case 0x45:
        fn_800A3044(param[2], param[3], 0);
        break;
    case 0x46:
    case 0x52:
    case 0x53:
        if (param[4] != 0) {
            fn_800A3118(b, param[2], param[3], 0);
        } else {
            fn_800A3044(param[2], param[3], 0);
        }
        break;
    case 0x59:
        fn_800A3118(b, param[0], param[1], 0);
        break;
    case 0x7E:
        fn_800A3044(param[2], param[3], param[4]);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// 80102138
BOOL fn_80102138(int a, int b) {
    if (b > 0 && a > 0) {
        if (a > b) {
            return TRUE;
        }
        return FALSE;
    }
    if (b < 0 && a < 0) {
        if (a > b) {
            return TRUE;
        }
        return FALSE;
    }
    if ((b > 0 && a < 0) || (b < 0 && a > 0)) {
        if (b - a < 0) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80475A90: one flag per player state (fn_801021B0).
static const u8 lbl_80475A90[0xAB] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// 801021B0
u8 fn_801021B0(u32 state) {
    if (state >= 0xAB) {
        return 0;
    }
    return lbl_80475A90[state];
}

// Fashion types 0x28 and 0x36..0x38 (fn_801021D0).
static inline BOOL isSpecialFashion(const dItem::Item *item) {
    int fashion = item->getFashion();
    if (fashion == 0x28 || (fashion >= 0x36 && fashion < 0x39)) {
        return TRUE;
    }
    return FALSE;
}

// 801021D0
int fn_801021D0(const dItem::Item *a, const dItem::Item *b) {
    if (a->mId != dItem::ITEM_ID_NONE && isSpecialFashion(a)) {
        return 0x1773;
    }
    if (b->mId == dItem::ITEM_ID_NONE) {
        return 0x1744;
    }
    if (isSpecialFashion(b)) {
        return 0x1773;
    }
    dItem::Item item(dItem::ITEM_IDX_PUMPKIN_HEAD);
    if (b->isSame(item)) {
        return 0x1772;
    }
    item.setFromIndex(dItem::ITEM_IDX_KING_TUT_MASK);
    if (b->isSame(item)) {
        return 0x1746;
    }
    return 0x1744;
}

// 801022B8: splash/snow effect callback for fn_801024F8.
void fn_801022B8(dEffectTarget_c *obj, u32 kind) {
    if (obj == NULL) {
        return;
    }
    mVec3_c pos;
    if (fn_80162558((u8)fn_80162548())) {
        mVec3_c src = obj->_AC;
        fn_80082B04(&pos, &src);
    } else {
        pos = obj->_AC;
    }

    dGroundCheck_c check;
    // The target tests r0 after this call, not r3 (see notes/d_player_mgr.txt).
    if (fn_8006E1BC(&check, &pos, 0, 0, 0) != 0) {
        if (fn_8006E400(&check, pos.y)) {
            pos.y = check._3C;
            fn_80087790("afi_hny_watersplash_b", &pos, 0, 0);
            if (obj->_C8 != NULL) {
                fn_80285110(obj->_C8, obj);
            }
        }
    } else {
        f32 ground = fn_8006E31C(&check, 1);
        if (pos.y <= ground) {
            const char *name = kind == 3 ? "afi_hny_snowbreak" : "afi_hny_watersplash_a";
            if (fabsf(ground - pos.y) <= 5.0f) {
                pos.y = ground;
                fn_80087790(name, &pos, 0, 0);
                if (obj->_C8 != NULL) {
                    fn_80285110(obj->_C8, obj);
                }
            }
        }
    }
}

// 80102434
int fn_80102434(const mVec3_c *pos) {
    dGroundCheck_c check;
    // The target tests r0 after this call, not r3 (see notes/d_player_mgr.txt).
    if (fn_8006E1BC(&check, pos, 0, 0, 0) == 2) {
        return 1;
    }
    int attr = check._34;
    if (lbl_8074EBE8 != NULL) {
        int mode = lbl_8074EBE8->_5884;
        BOOL special = FALSE;
        if (mode == 4 || mode == 3) {
            special = TRUE;
        }
        if (special) {
            return 2;
        }
    }
    if (attr == 0x17) {
        return 4;
    }
    if ((u8)fn_800A8850(pos) > lbl_8074AF10) {
        return 3;
    }
    return 0;
}

// 801024F8
void fn_801024F8(const char *const *names, const mVec3_c *pos, int b, int c, int d) {
    u32 kind = fn_80102434(pos);
    fn_80087790(names[kind], pos, b, 0);
    if (kind == 0 || kind == 4) {
        return;
    }
    BOOL wet = FALSE;
    if (kind == 1 || kind == 2) {
        wet = TRUE;
    }
    fn_80087844(wet ? c : d, pos, b, 0, fn_801022B8, kind);
}

// 801025B8
BOOL fn_801025B8() {
    return FALSE;
}

// 801025C0: __sinit_d_player_mgr_cpp (the look-angle limits and lbl_805D2440).

