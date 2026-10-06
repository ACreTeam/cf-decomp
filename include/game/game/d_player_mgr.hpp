#pragma once

#include <types.h>
#include <game/game/d_actor.hpp>
#include <game/game/d_private_data.hpp>
#include <game/mLib/m_3d/fanm.hpp>

// Player manager. Source: src/dol/game/d_player_mgr.cpp (.text 800FBB18..8010263C).
// Notes: notes/d_player_mgr.txt. Names are inferred; fn_ ones are not understood yet.
//
// Player index arguments: 0-3 are local controllers, 4 means "the current one"
// (fn_800DCF58). Player slots (mSlots): 0-3 town players (dSaveData_c::mPlayers),
// 4-6 visiting players (dGuestPlayers_c), 7 none.

// The player actor (lives in the d_a_player REL). Only the fields this TU uses.
class dPlayerActor_c : public dActor_c {
public:
    using dActor_c::mPos;
    using dActor_c::mAngle;

    m3d::fanm_c *getAnm() { return (m3d::fanm_c *)_03C4; }

    /* 0x00A0 */ u8 _00A0[0x324];
    /* 0x03C4 */ u8 _03C4[0x8];
    /* 0x03CC */ u8 _03CC[0x240];
    /* 0x060C */ u8 _060C[0x18];
    /* 0x0624 */ u8 _0624[0x30];
    /* 0x0654 */ mVec3_c _0654;
    /* 0x0660 */ u8 _0660[0x90];
    /* 0x06F0 */ u8 _06F0[0x4];
    /* 0x06F4 */ u8 _06F4[0x540];
    /* 0x0C34 */ void *_0C34;
    /* 0x0C38 */ u8 _0C38[0x1430];
    /* 0x2068 */ mVec3_c _2068;
    /* 0x2074 */ mVec3_c _2074;
    /* 0x2080 */ u8 _2080[0xC];
    /* 0x208C */ f32 _208C;
    /* 0x2090 */ u8 _2090[0x5C];
    /* 0x20EC */ mVec3_c _20EC;
    /* 0x20F8 */ u8 _20F8[0x92];
    /* 0x218A */ u16 _218A;
    /* 0x218C */ u8 _218C[0x4];
    /* 0x2190 */ int _2190;
    /* 0x2194 */ int _2194;
    /* 0x2198 */ u8 _2198[0x24];
    /* 0x21BC */ f32 _21BC;
    /* 0x21C0 */ u8 _21C0[0x60];
    /* 0x2220 */ int mAnmId;
    /* 0x2224 */ u8 _2224[0x8];
    /* 0x222C */ int mState;
    /* 0x2230 */ int _2230;
    /* 0x2234 */ int _2234;
    /* 0x2238 */ int _2238;
    /* 0x223C */ int _223C;
    /* 0x2240 */ u8 _2240[0x4];
    /* 0x2244 */ int _2244;
    /* 0x2248 */ int _2248;
    /* 0x224C */ int _224C;
    /* 0x2250 */ u8 _2250[0x4];
    /* 0x2254 */ int _2254;
    /* 0x2258 */ u8 _2258[0x14];
    /* 0x226C */ int _226C;
    /* 0x2270 */ u32 _2270;
    /* 0x2274 */ u32 _2274;
    /* 0x2278 */ u8 _2278[0x8];
    /* 0x2280 */ int _2280;
    /* 0x2284 */ u32 _2284;
    /* 0x2288 */ u32 _2288;
    /* 0x228C */ int _228C;
    /* 0x2290 */ int _2290;
    /* 0x2294 */ u8 _2294[0x4];
    /* 0x2298 */ int _2298;
    /* 0x229C */ u8 _229C[0x18];
    /* 0x22B4 */ u16 _22B4;
    /* 0x22B6 */ u16 _22B6;
    /* 0x22B8 */ u8 _22B8[0xC];
    /* 0x22C4 */ s16 _22C4;
    /* 0x22C6 */ s16 _22C6;
    /* 0x22C8 */ s16 _22C8;
    /* 0x22CA */ s16 _22CA;
    /* 0x22CC */ s16 _22CC;
    /* 0x22CE */ u8 _22CE[0x2];
    /* 0x22D0 */ s16 _22D0;
    /* 0x22D2 */ u8 _22D2[0x8];
    /* 0x22DA */ u8 _22DA;
    /* 0x22DB */ u8 _22DB[0x2];
    /* 0x22DD */ u8 _22DD;
    /* 0x22DE */ u8 _22DE;
    /* 0x22DF */ u8 _22DF[0x5];
    /* 0x22E4 */ u8 _22E4;
    /* 0x22E5 */ u8 _22E5[0x7];
    /* 0x22EC */ f32 _22EC;
    /* 0x22F0 */ u8 _22F0[0x8];
    /* 0x22F8 */ s16 _22F8;
    /* 0x22FA */ u8 _22FA[0xE];
    /* 0x2308 */ mVec3_c _2308;
    /* 0x2314 */ u8 _2314[0x4];
}; // size >= 0x2318

// 0x19440 at dPlayerMgr_c+0x60: data for up to three visiting players (slots 4-6).
class dGuestPlayers_c {
public:
    dGuestPlayers_c();  // 80101AEC
    ~dGuestPlayers_c(); // 80101B38
    void clear();                    // 80101BA0
    int getIndex(int slot);          // 80101BF0: slot 4-6 -> 0-2, else -1
    dPrivateData_c *get(int slot);   // 80101C1C
    dPrivateData_c *getRaw(int slot); // 80101C68

    dPrivateData_c mPlayers[3];
};

// lbl_805D2440. 0x194C0.
class dPlayerMgr_c {
public:
    typedef void (*ChangeStateFn)(int idx, int state);

    dPlayerMgr_c();  // 800FBB58
    ~dPlayerMgr_c(); // 800FBBEC

    // Player save data by player index: 0-3 town players, 4-6 visitors online.
    static dPrivateData_c *getPlayer(int player);                 // 80101624
    static dPrivateData_c *getPlayer(const dPersonalID_c *pid);   // 801016B8
    static dPrivateData_c *getPlayerRaw(int player);              // 801016DC
    // By network member index (0-3, 4 = this console), through mSlots.
    static dPrivateData_c *getNetPlayer(int member);              // 80101694
    static dPrivateData_c *getNetPlayerRaw(int member);           // 8010174C
    // This console's player.
    static dPrivateData_c *getCurrentPlayer();                    // 80101770
    static dPrivateData_c *getCurrentPlayerRaw();                 // 80101794

    /* 0x00000 */ ChangeStateFn mChangeState[4]; // set by fn_80101990, called by fn_80101A74
    /* 0x00010 */ int _10[4];
    /* 0x00020 */ int _20;
    /* 0x00024 */ dPlayerActor_c *mActors[4];    // set by fn_801019A4
    /* 0x00034 */ int mSlots[4];                 // player index per network member (fn_80100330), 7 = none
    /* 0x00044 */ u8 _44[0x1C];
    /* 0x00060 */ dGuestPlayers_c mGuests;
    /* 0x194A0 */ u32 _194A0;                    // 0xD2F0 (fn_80101408)
    /* 0x194A4 */ u8 _194A4;
    /* 0x194A5 */ u8 _194A5;
    /* 0x194A6 */ u8 _194A6;
    /* 0x194A7 */ u8 _194A7;
    /* 0x194A8 */ u8 _194A8;
    /* 0x194A9 */ u8 mLocked;                    // fn_800FC568 / fn_800FC580
    /* 0x194AA */ u8 _194AA;
    /* 0x194AB */ u8 _194AB;
    /* 0x194AC */ u8 _194AC[0x14];
}; // size 0x194C0

extern dPlayerMgr_c lbl_805D2440;
extern s16 sLookPitchMax; // 8074E600: 10 degrees
extern s16 sLookYawMax;   // 8074E602: 45 degrees

// Arguments of the ground check used by the splash effects (fn_8006E1BC).
struct dGroundCheck_c {
    /* 0x00 */ u8 _00[0x34];
    /* 0x34 */ int _34; // ground attribute
    /* 0x38 */ u8 _38[0x4];
    /* 0x3C */ f32 _3C; // height
    /* 0x40 */ u8 _40[0x10];
};

// Object passed to the splash effect callback (fn_801022B8).
struct dEffectTarget_c {
    /* 0x00 */ u8 _00[0xAC];
    /* 0xAC */ mVec3_c _AC;
    /* 0xB8 */ u8 _B8[0x10];
    /* 0xC8 */ void *_C8;
};

// Output of fn_802C5A0C (Mii data); 8 bytes at 0x2C go to dPrivateData_c::_83ED.
struct dMiiData_c {
    /* 0x00 */ u8 _00[0x2C];
    /* 0x2C */ u8 _2C[8];
    /* 0x34 */ u8 _34[0x14];
};

extern "C" {
void *fn_800FBB18(void *p, int del); // 800FBB18
mVec3_c *fn_800FBC48(int idx); // 800FBC48
dPlayerActor_c *fn_800FBC7C(int idx); // 800FBC7C
dPlayerActor_c *fn_800FBC80(int idx); // 800FBC80
dPlayerActor_c *fn_800FBC84(int idx); // 800FBC84
void *fn_800FBC88(int idx); // 800FBC88
BOOL fn_800FBCBC(s16 *out); // 800FBCBC
s16 fn_800FBD28(int idx); // 800FBD28
BOOL fn_800FBD5C(u32 *a, u32 *b, int idx); // 800FBD5C
void *fn_800FBDD4(int idx); // 800FBDD4
void *fn_800FBE08(int idx); // 800FBE08
BOOL fn_800FBE3C(); // 800FBE3C
BOOL fn_800FBE98(); // 800FBE98
BOOL fn_800FBF84(int state); // 800FBF84
void fn_800FBFCC(); // 800FBFCC
BOOL fn_800FC010(); // 800FC010
void fn_800FC064(BOOL on, int idx); // 800FC064
u32 fn_800FC0C0(int idx); // 800FC0C0
BOOL fn_800FC0F8(); // 800FC0F8
BOOL fn_800FC194(const mVec3_c *target); // 800FC194
BOOL fn_800FC2A4(); // 800FC2A4
BOOL fn_800FC340(); // 800FC340
BOOL fn_800FC3B0(); // 800FC3B0
void fn_800FC404(); // 800FC404
BOOL fn_800FC4C0(); // 800FC4C0
BOOL fn_800FC514(); // 800FC514
void fn_800FC568(); // 800FC568
void fn_800FC580(); // 800FC580
void fn_800FC598(); // 800FC598
void fn_800FC604(); // 800FC604
u8 fn_800FC61C(); // 800FC61C
BOOL fn_800FC630(); // 800FC630
BOOL fn_800FC674(int slot); // 800FC674
BOOL fn_800FC7E8(); // 800FC7E8
BOOL fn_800FC858(); // 800FC858
BOOL fn_800FC888(); // 800FC888
BOOL fn_800FC8EC(BOOL a); // 800FC8EC
BOOL fn_800FC8FC(); // 800FC8FC
BOOL fn_800FC958(); // 800FC958
BOOL fn_800FC9AC(); // 800FC9AC
BOOL fn_800FCA04(); // 800FCA04
BOOL fn_800FCA28(); // 800FCA28
BOOL fn_800FCAA0(); // 800FCAA0
BOOL fn_800FCAF4(const dItem::Item *item); // 800FCAF4
BOOL fn_800FCBA8(); // 800FCBA8
BOOL fn_800FCBFC(); // 800FCBFC
BOOL fn_800FCC74(const dItem::Item *item, const mVec3_c *pos); // 800FCC74
BOOL fn_800FCD60(u32 a, u32 b, BOOL c); // 800FCD60
BOOL fn_800FCDFC(const dItem::Item *shirt, const dItem::Item *hat, const dItem::Item *held); // 800FCDFC
BOOL fn_800FD058(const dItem::Item *item); // 800FD058
BOOL fn_800FD130(); // 800FD130
BOOL fn_800FD17C(); // 800FD17C
BOOL fn_800FD510(mVec3_c *out, int idx); // 800FD510
BOOL fn_800FD588(s16 angle, int type, f32 x, f32 z); // 800FD588
BOOL fn_800FD664(); // 800FD664
BOOL fn_800FD6A8(mVec3_c *out, int idx); // 800FD6A8
BOOL fn_800FD720(mVec3_c *out, int idx); // 800FD720
BOOL fn_800FD798(s16 angle, int type, f32 x, f32 z); // 800FD798
BOOL fn_800FD88C(int type); // 800FD88C
BOOL fn_800FD924(int type); // 800FD924
BOOL fn_800FD9E0(int type, s16 angle, f32 x, f32 z); // 800FD9E0
BOOL fn_800FDAA4(int type); // 800FDAA4
BOOL fn_800FDB10(); // 800FDB10
void fn_800FDB40(); // 800FDB40
BOOL fn_800FDB78(int type); // 800FDB78
void fn_800FDBDC(u32 mode); // 800FDBDC
BOOL fn_800FDCC0(); // 800FDCC0
BOOL fn_800FDD04(); // 800FDD04
BOOL fn_800FDD8C(u32 hair, u32 color); // 800FDD8C
BOOL fn_800FDE3C(); // 800FDE3C
BOOL fn_800FDE7C(); // 800FDE7C
BOOL fn_800FDEE8(); // 800FDEE8
f32 fn_800FDF28(); // 800FDF28
BOOL fn_800FDF7C(); // 800FDF7C
BOOL fn_800FE094(); // 800FE094
BOOL fn_800FE0D0(); // 800FE0D0
BOOL fn_800FE140(); // 800FE140
BOOL fn_800FE17C(u16 mii); // 800FE17C
BOOL fn_800FE2D4(); // 800FE2D4
u32 fn_800FE360(); // 800FE360
BOOL fn_800FE39C(s16 angle, int type, int target, f32 x, f32 z); // 800FE39C
BOOL fn_800FE484(s16 angle, int target, f32 x, f32 z); // 800FE484
BOOL fn_800FE550(s16 angle, int type, BOOL useC, f32 x, f32 z); // 800FE550
BOOL fn_800FE634(); // 800FE634
void fn_800FE688(int *outState, mVec3_c *outPos, s16 *outAngle); // 800FE688
BOOL fn_800FE74C(const mVec3_c *pos, int idx, f32 speed); // 800FE74C
BOOL fn_800FE7E0(const mVec3_c *pos, int idx, f32 speed); // 800FE7E0
BOOL fn_800FE88C(s16 angle, int idx); // 800FE88C
BOOL fn_800FE89C(s16 angle, int frames, int idx); // 800FE89C
BOOL fn_800FE938(const mVec3_c *target, int a, BOOL pitch, int c); // 800FE938
BOOL fn_800FEA9C(int idx); // 800FEA9C
BOOL fn_800FEAA8(int idx); // 800FEAA8
BOOL fn_800FEAB4(int idx); // 800FEAB4
BOOL fn_800FEAC0(int idx); // 800FEAC0
BOOL fn_800FEACC(int idx); // 800FEACC
BOOL fn_800FEAD8(int idx); // 800FEAD8
BOOL fn_800FEAE4(int state, int idx); // 800FEAE4
int fn_800FEB34(int idx); // 800FEB34
BOOL fn_800FEB68(const dItem::Item *shirt); // 800FEB68
BOOL fn_800FEBE4(const dItem::Item *hat); // 800FEBE4
BOOL fn_800FEC60(const dItem::Item *acc); // 800FEC60
BOOL fn_800FECF8(const dItem::Item *held); // 800FECF8
BOOL fn_800FEEB0(); // 800FEEB0
mVec3_c *fn_800FEFF4(int idx); // 800FEFF4
void *fn_800FF048(); // 800FF048
BOOL fn_800FF080(mVec3_c *out); // 800FF080
u32 fn_800FF150(int idx); // 800FF150
u32 fn_800FF188(int idx); // 800FF188
int fn_800FF1C0(int *x, int *z, int idx); // 800FF1C0
BOOL fn_800FF354(int *outState); // 800FF354
BOOL fn_800FF444(const dEquip_c *equip, int mode, int kind); // 800FF444
BOOL fn_800FF840(); // 800FF840
BOOL fn_800FF8B0(); // 800FF8B0
BOOL fn_800FF94C(u8 hair, int bit); // 800FF94C
BOOL fn_800FF978(int bit); // 800FF978
u8 fn_800FF9C4(u32 idx, u32 gender); // 800FF9C4
BOOL fn_800FFA50(const mVec3_c *a, int b, f32 f); // 800FFA50
BOOL fn_800FFAC0(); // 800FFAC0
BOOL fn_800FFB18(); // 800FFB18
BOOL fn_800FFBE4(); // 800FFBE4
BOOL fn_800FFC38(); // 800FFC38
BOOL fn_800FFCA8(); // 800FFCA8
BOOL fn_800FFD10(); // 800FFD10
u16 fn_800FFD64(); // 800FFD64
BOOL fn_800FFDC0(const mVec3_c *target, int maxAngle, f32 maxDist); // 800FFDC0
void fn_800FFEAC(mMtx_c *mtx, int arg, const mAng *angle); // 800FFEAC
void fn_800FFF2C(mMtx_c *mtx, int arg, const mAng3_c *angle); // 800FFF2C
u16 fn_800FFFCC(int gender); // 800FFFCC
void fn_8010003C(); // 8010003C
void fn_80100068(int value); // 80100068
BOOL fn_801000AC(int slot); // 801000AC
BOOL fn_80100128(f32 *out, int idx); // 80100128
f32 fn_80100194(const mVec3_c *a, const mVec3_c *b, const mVec3_c *c); // 80100194
void fn_80100234(); // 80100234
void fn_80100238(u32 idx, int value); // 80100238
void fn_80100258(u32 idx); // 80100258
void fn_8010027C(); // 8010027C
void fn_801002BC(); // 801002BC
int fn_80100330(u32 idx); // 80100330
BOOL fn_80100358(u32 bone, dPlayerActor_c *player); // 80100358
void fn_80100410(dPlayerActor_c *player); // 80100410
void fn_8010042C(dPlayerActor_c *player, int *x, int *z); // 8010042C
BOOL fn_80100440(int a, int b, int c, int d, int e, f32 f1, f32 f2, f32 f3); // 80100440
f32 fn_80100548(); // 80100548
BOOL fn_80100550(int a, int b, int c, int d, int e, f32 f1, f32 f2); // 80100550
BOOL fn_80100644(BOOL a); // 80100644
BOOL fn_801006A8(); // 801006A8
BOOL fn_80100714(u8 shoeColor, u8 b); // 80100714
BOOL fn_801007AC(); // 801007AC
BOOL fn_80100804(); // 80100804
BOOL fn_80100860(); // 80100860
BOOL fn_801008E4(int idx); // 801008E4
BOOL fn_8010094C(int idx); // 8010094C
BOOL fn_8010098C(); // 8010098C
BOOL fn_801009E0(u32 type); // 801009E0
BOOL fn_80100A54(); // 80100A54
BOOL fn_80100AB0(); // 80100AB0
BOOL fn_80100B24(); // 80100B24
BOOL fn_80100B80(); // 80100B80
BOOL fn_80100B84(f32 x, f32 z); // 80100B84
BOOL fn_80100C28(); // 80100C28
BOOL fn_80100C84(BOOL a); // 80100C84
BOOL fn_80100D44(); // 80100D44
BOOL fn_80100DD0(int idx); // 80100DD0
BOOL fn_80100E3C(); // 80100E3C
BOOL fn_80100E7C(); // 80100E7C
BOOL fn_80100EC0(); // 80100EC0
BOOL fn_80100F00(s16 angle, f32 frame); // 80100F00
BOOL fn_80100F98(); // 80100F98
BOOL fn_80100FDC(); // 80100FDC
u8 fn_8010101C(); // 8010101C
void fn_80101030(); // 80101030
void fn_80101048(); // 80101048
BOOL fn_80101060(const dItem::Item *held); // 80101060
BOOL fn_80101168(); // 80101168
BOOL fn_801011BC(); // 801011BC
BOOL fn_801011C4(mVec3_c *out); // 801011C4
BOOL fn_8010125C(s16 frames, f32 speed); // 8010125C
BOOL fn_801012FC(); // 801012FC
BOOL fn_80101350(); // 80101350
BOOL fn_801013B4(); // 801013B4
u32 *fn_801013F4(); // 801013F4
void fn_80101408(); // 80101408
void fn_80101424(); // 80101424
BOOL fn_80101490(); // 80101490
void fn_801014A4(); // 801014A4
u8 fn_801014BC(); // 801014BC
void fn_801014D0(); // 801014D0
void fn_801014E8(); // 801014E8
BOOL fn_80101500(); // 80101500
void fn_80101514(); // 80101514
u8 fn_8010152C(); // 8010152C
void fn_80101540(); // 80101540
BOOL fn_80101558(); // 80101558
void fn_8010156C(); // 8010156C
BOOL fn_80101584(int slot, const dItem::Item *item); // 80101584
u32 fn_801015F4(); // 801015F4
u32 fn_80101600(); // 80101600
u32 fn_80101608(); // 80101608
u32 fn_80101610(); // 80101610
u32 fn_80101618(); // 80101618
int fn_801017B8(); // 801017B8
void fn_801017DC(); // 801017DC
int fn_801017EC(const dPersonalID_c *pid); // 801017EC
int fn_801018DC(const dPlayerID_c *id); // 801018DC
void fn_80101990(dPlayerMgr_c::ChangeStateFn fn, int idx); // 80101990
void fn_801019A4(dPlayerActor_c *actor, u32 idx); // 801019A4
dPlayerActor_c *fn_801019C4(int idx); // 801019C4
dPlayerActor_c *fn_80101A28(int idx); // 80101A28
BOOL fn_80101A74(int state, int idx); // 80101A74
void fn_80101CB4(); // 80101CB4
void fn_80101CB8(); // 80101CB8
void fn_80101CBC(); // 80101CBC
void fn_80101CC0(int idx, const mVec3_c *value); // 80101CC0
void fn_80101D0C(int idx, int a, int b); // 80101D0C
void fn_80101D70(int idx, int value); // 80101D70
void fn_80101DA8(int idx, int value); // 80101DA8
void fn_80101DAC(int idx, const u8 *value); // 80101DAC
BOOL fn_80101DB0(int idx, int state); // 80101DB0
void fn_80101DC0(); // 80101DC0
void fn_80101DC4(); // 80101DC4
BOOL fn_80101DC8(int idx, u8 b, int state, const u8 *param); // 80101DC8
BOOL fn_80101E3C(int idx, u8 b, int state, const u8 *param); // 80101E3C
BOOL fn_80101EB0(int idx, u8 b, int state, const u8 *param); // 80101EB0
BOOL fn_80102138(int a, int b); // 80102138
u8 fn_801021B0(u32 state); // 801021B0
int fn_801021D0(const dItem::Item *a, const dItem::Item *b); // 801021D0
void fn_801022B8(dEffectTarget_c *obj, u32 kind); // 801022B8
int fn_80102434(const mVec3_c *pos); // 80102434
void fn_801024F8(const char *const *names, const mVec3_c *pos, int b, int c, int d); // 801024F8
BOOL fn_801025B8(); // 801025B8
}
