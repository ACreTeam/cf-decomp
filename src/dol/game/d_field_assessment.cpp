// FG manager process (dFgMngProc_c, name from the "dFgMngProc_c::m_heap" string): the day-change
// processing of the field's objects (tree growth, flowers, weeds, cross-breeding, ...), run on a
// background thread, plus the field assessment (town rating, d_field_assessment.hpp).
// See notes/d_field_assessment.txt.
//
// .text 80091AF8..800A5D3C (__sinit 8009FFEC; built with -sym on, so the code of the #included
// d_fg_mng_task.inc lands after it, 800A2220..), .ctors 80465670..80465674, .rodata
// 8046EFA0..8046F1E8, .data 804E1808..804E3C40, .bss 80588D58..8058B5F8, .sdata 80749FB8..8074A3C8,
// .sbss 8074E338..8074E378, .sdata2 80750658..80750708. d_search_cand.hpp first: vtables come out
// in reverse definition order, and its templates' vtables follow the HostIO ones in the target.
#include <game/game/d_search_cand.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_home_room_map.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_museum.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_notice.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_police_box.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_random.hpp>
#include <game/game/d_recycle_bin.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_theater.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_fader.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggHeap.h>
#include <revolution/OS/OSThread.h>
#include <revolution/OS/OSTime.h>
#include <nw4r/math.h>
#include <string.h>

// 80750658: the first money rock of each player (5 each).
static const u16 sStoneKindBase[4] = {
    dItem::FG_MONEY_ROCK_P0_A, dItem::FG_MONEY_ROCK_P1_A, dItem::FG_MONEY_ROCK_P2_A, dItem::FG_MONEY_ROCK_P3_A,
};
// 80750660: the 8 neighbours.
static const u8 sAround8[8] = {
    FG_UNIT_OFS(-1, -1), FG_UNIT_OFS(0, -1), FG_UNIT_OFS(1, -1), FG_UNIT_OFS(-1, 0),
    FG_UNIT_OFS(1, 0), FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(0, 1), FG_UNIT_OFS(1, 1),
};
// 80750668: 5 of them (forward half).
static const u8 sAround5[5] = {
    FG_UNIT_OFS(-1, 0), FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(0, 1), FG_UNIT_OFS(1, 1),
    FG_UNIT_OFS(1, 0),
};

// Not split yet (C linkage keeps the target names).
extern "C" {
void fn_8018EF28();
int fn_8016CF08(int idx);
void fn_802B8D30(OSThread *thread, int arg);
void fn_802B8D90(OSThread *thread);
BOOL fn_800DCEDC();
int fn_800DCF58();
BOOL fn_800DCF2C(int player);
BOOL fn_800DCF90();
void fn_800DD4C8();
void fn_800DD518(void *data, int size);
void fn_800DD588(int a, int b);
void fn_8014D3F4(void *obj, int rank, int days);
int fn_8014D844(void *obj, int id);
void fn_80169ED8();
void fn_80169C38();
void fn_80169BF4();
void fn_800D11C8(dTime_c *last, int days);
void fn_80081238(u16 *flags, int x, int z);
u32 fn_80081324(int type);
void fn_80169AC4();
void fn_80169ADC();
void fn_80169AEC();
void fn_80169B04();
void fn_80169B14();
void fn_80169B2C();
void fn_80169B3C();
void fn_80169B54();
void fn_80169B64();
void fn_80169BB4();
BOOL fn_8014CF9C(void *stamp, const dTime_c *time);
void fn_8014CC98(void *stamp, const dTime_c *time);
void fn_8014D89C(void *events, int id);
f32 fn_80074D64(int x, int z);
f32 fn_80074974(const nw4r::math::VEC3 *pos, int a);
int fn_800812C8(int blockType);
void fn_8014F248(void *obj, int days);
void fn_80151CBC(void *obj);
void fn_8014FD48(void *obj, dTime_c last, dTime_c *now, int *days);
void fn_80151CF0(void *obj);
BOOL fn_8014D740(void *obj);
void fn_8014D69C(void *obj);
void fn_800F5AE8();
void fn_80150524(void *obj);
void fn_801505E4(void *obj);
void fn_80150628(void *obj);
void fn_801510A0(void *obj);
void fn_800DD5F8(int a, void *b, int c);
void fn_8015384C(void *obj);
void fn_801541D8(void *obj);
void fn_8014F998(void *obj, dTime_c *now);
void fn_8014F96C(void *obj);
void fn_80186838();
void fn_8014F030(void *obj, u8 a);
void fn_8014EFC0(void *obj, u8 a);
BOOL fn_8018EEEC();
void fn_8018EEF4();
void fn_8018EE7C();
nw4r::math::VEC3 *fn_8016A28C();
void fn_800755A0(int bg);
f32 fn_80073510(const nw4r::math::VEC3 *pos);
void fn_800D16E8();
int fn_801017B8();
u8 *fn_800AC28C(int idx);
void fn_800C77E8(void *obj);
void fn_800C7800(void *obj);
void fn_800C7830(void *obj);
u32 fn_8008299C(const mVec3_c *pos);
BOOL fn_80087820(dEffect_c *effect, const char *name, const mVec3_c *pos, const mAng3_c *ang, const mVec3_c *scale);
f32 fn_8044B83C(EGG::Effect *effect);
int fn_8006E1BC(void *check, const nw4r::math::VEC3 *pos, int, int, int);
int fn_80072E94(const nw4r::math::VEC3 *pos);
BOOL fn_800737E4(int x, int z, int attr, int x2, int z2);
int fn_80190C58(int a);
void fn_800C77B8(int player);
void fn_800C77D0(int player);
void fn_800C7818(int player);
int fn_800C60B4(dItem::Item *out, int num, const void *table, int tableNum, const void *filter, const dItem::Item *exclude, int excludeNum, int);
extern void *lbl_8074E9A0[2];
extern u16 *lbl_8074E800;
}

extern EGG::Heap *lbl_8074E478;

typedef BOOL (*dFgMngAroundFunc)(dFdBase_c *fd, const int *pos, int x, int z, int *size);
struct dFgMngGroundCheck_c;
// Helpers of the tree, weed and flower processing (some are also called by other TUs).
void killTree(dFdBase_c *fd, dItem::Item *item, int x, int z);
void growTree(dFdBase_c *fd, dItem::Item *item, int x, int z);
BOOL forEachAroundUnit(dFdBase_c *fd, int *size, int x, int z, dFgMngAroundFunc func);
BOOL isTreeObstacleAt(dFdBase_c *fd, const int *pos, int x, int z, int *size);
BOOL killSaplingAt(dFdBase_c *fd, const int *pos, int x, int z, int *size);
BOOL isBlockedUnit(dFdBase_c *fd, int x, int z);
BOOL isPondEdgeUnit(dFdBase_c *fd, int x, int z);
BOOL isSapling(dItem::Item *item);
BOOL tryCrossBreedAt(dFdBase_c *fd, const int *pos, int x, int z, int *size);
int getFlowerKind(dItem::Item *item);
int getFlowerColor(dItem::Item *item);

// Objects of this TU, in .sbss / .bss order (the __sinit 8009FFEC constructs them).
void *sFgObjMgr;                          // 8074E338: the d_fgobj_managerNP actor (set by its create)
u8 sDayChangeBlocked;                     // 8074E33C: fgMngProc_blockDayChange
static u8 sFgMngProcFlags;                // 8074E33D: 1 save flag 6 deferred, 2 day change, 4 thread
static void *sFgMngProcThreadStack;       // 8074E340: 0x3000 bytes from lbl_8074E478
dFgMngLitterFlags_c sFgMngLitterFlags;    // 8074E344
u8 sFgMngCmdRetryBits[FG_MNG_CMD_NUM / 8];          // 8074E348: a retry bit per command
u16 *sFgMngUnitMask;                      // 8074E350: blocked-unit bit rows
dRandom_c *sFgMngRandom;                  // 8074E354: the day-change processing's random numbers
dFgObjCallback sFgObjCallback;            // 8074E358

static OSThread sFgMngProcThread;         // 80588D58
static dFgMngProc_c sFgMngProc;           // 80589080
dFdAssess_c sFdAssess;                    // 8058909C
dFgMngState_c sFgMngState;                // 805894C8 (also used by d_fgobj_managerNP)
dFgMngTaskList_c sFgMngTaskList;          // 80589B7C
dFgMngLock_c sFgMngLocks[FG_MNG_LOCK_NUM];           // 80589D80
static dFgMngCmdEntry_c sFgMngCmds[FG_MNG_CMD_NUM]; // 80589E90
// 8058A310: the 30 fg 0x56 units (dFdAsPos_c, (-1, -1): free). Raw words: dFdAsPos_c's constructor
// would add a construction loop to the __sinit.
u32 sFg56Units[FG56_UNIT_NUM * 2];

static CBTulipRedRedHostIO_c sCBTulipRedRedHostIO; // 8058A40C
static CBTulipRedWhiteHostIO_c sCBTulipRedWhiteHostIO; // 8058A434
static CBTulipRedYellowHostIO_c sCBTulipRedYellowHostIO; // 8058A45C
static CBTulipRedPinkHostIO_c sCBTulipRedPinkHostIO; // 8058A484
static CBTulipRedPurpleHostIO_c sCBTulipRedPurpleHostIO; // 8058A4AC
static CBTulipRedBlackHostIO_c sCBTulipRedBlackHostIO; // 8058A4D4
static CBTulipWhiteWhiteHostIO_c sCBTulipWhiteWhiteHostIO; // 8058A4FC
static CBTulipWhiteYellowHostIO_c sCBTulipWhiteYellowHostIO; // 8058A524
static CBTulipWhitePinkHostIO_c sCBTulipWhitePinkHostIO; // 8058A54C
static CBTulipWhitePurpleHostIO_c sCBTulipWhitePurpleHostIO; // 8058A574
static CBTulipWhiteBlackHostIO_c sCBTulipWhiteBlackHostIO; // 8058A59C
static CBTulipYellowYellowHostIO_c sCBTulipYellowYellowHostIO; // 8058A5C4
static CBTulipYellowPinkHostIO_c sCBTulipYellowPinkHostIO; // 8058A5EC
static CBTulipYellowPurpleHostIO_c sCBTulipYellowPurpleHostIO; // 8058A614
static CBTulipYellowBlackHostIO_c sCBTulipYellowBlackHostIO; // 8058A63C
static CBTulipPinkPinkHostIO_c sCBTulipPinkPinkHostIO; // 8058A664
static CBTulipPinkPurpleHostIO_c sCBTulipPinkPurpleHostIO; // 8058A68C
static CBTulipPinkBlackHostIO_c sCBTulipPinkBlackHostIO; // 8058A6B4
static CBTulipPurplePurpleHostIO_c sCBTulipPurplePurpleHostIO; // 8058A6DC
static CBTulipPurpleBlackHostIO_c sCBTulipPurpleBlackHostIO; // 8058A704
static CBTulipBlackBlackHostIO_c sCBTulipBlackBlackHostIO; // 8058A72C
static CBPansyWhiteWhiteHostIO_c sCBPansyWhiteWhiteHostIO; // 8058A754
static CBPansyWhiteYellowHostIO_c sCBPansyWhiteYellowHostIO; // 8058A77C
static CBPansyWhiteRedHostIO_c sCBPansyWhiteRedHostIO; // 8058A7A4
static CBPansyWhitePurpleHostIO_c sCBPansyWhitePurpleHostIO; // 8058A7CC
static CBPansyWhiteOrangeHostIO_c sCBPansyWhiteOrangeHostIO; // 8058A7F4
static CBPansyWhiteBlueHostIO_c sCBPansyWhiteBlueHostIO; // 8058A81C
static CBPansyYellowYellowHostIO_c sCBPansyYellowYellowHostIO; // 8058A844
static CBPansyYellowRedHostIO_c sCBPansyYellowRedHostIO; // 8058A86C
static CBPansyYellowPurpleHostIO_c sCBPansyYellowPurpleHostIO; // 8058A894
static CBPansyYellowOrangeHostIO_c sCBPansyYellowOrangeHostIO; // 8058A8BC
static CBPansyYellowBlueHostIO_c sCBPansyYellowBlueHostIO; // 8058A8E4
static CBPansyRedRedHostIO_c sCBPansyRedRedHostIO; // 8058A90C
static CBPansyRedPurpleHostIO_c sCBPansyRedPurpleHostIO; // 8058A934
static CBPansyRedOrangeHostIO_c sCBPansyRedOrangeHostIO; // 8058A95C
static CBPansyRedBlueHostIO_c sCBPansyRedBlueHostIO; // 8058A984
static CBPansyPurplePurpleHostIO_c sCBPansyPurplePurpleHostIO; // 8058A9AC
static CBPansyPurpleOrangeHostIO_c sCBPansyPurpleOrangeHostIO; // 8058A9D4
static CBPansyPurpleBlueHostIO_c sCBPansyPurpleBlueHostIO; // 8058A9FC
static CBPansyOrangeOrangeHostIO_c sCBPansyOrangeOrangeHostIO; // 8058AA24
static CBPansyOrangeBlueHostIO_c sCBPansyOrangeBlueHostIO; // 8058AA4C
static CBPansyBlueBlueHostIO_c sCBPansyBlueBlueHostIO; // 8058AA74
static CBCosmosWhiteWhiteHostIO_c sCBCosmosWhiteWhiteHostIO; // 8058AA9C
static CBCosmosWhiteRedHostIO_c sCBCosmosWhiteRedHostIO; // 8058AAC4
static CBCosmosWhiteYellowHostIO_c sCBCosmosWhiteYellowHostIO; // 8058AAEC
static CBCosmosWhitePinkHostIO_c sCBCosmosWhitePinkHostIO; // 8058AB14
static CBCosmosWhiteOrangeHostIO_c sCBCosmosWhiteOrangeHostIO; // 8058AB3C
static CBCosmosWhiteBlackHostIO_c sCBCosmosWhiteBlackHostIO; // 8058AB64
static CBCosmosRedRedHostIO_c sCBCosmosRedRedHostIO; // 8058AB8C
static CBCosmosRedYellowHostIO_c sCBCosmosRedYellowHostIO; // 8058ABB4
static CBCosmosRedPinkHostIO_c sCBCosmosRedPinkHostIO; // 8058ABDC
static CBCosmosRedOrangeHostIO_c sCBCosmosRedOrangeHostIO; // 8058AC04
static CBCosmosRedBlackHostIO_c sCBCosmosRedBlackHostIO; // 8058AC2C
static CBCosmosYellowYellowHostIO_c sCBCosmosYellowYellowHostIO; // 8058AC54
static CBCosmosYellowPinkHostIO_c sCBCosmosYellowPinkHostIO; // 8058AC7C
static CBCosmosYellowOrangeHostIO_c sCBCosmosYellowOrangeHostIO; // 8058ACA4
static CBCosmosYellowBlackHostIO_c sCBCosmosYellowBlackHostIO; // 8058ACCC
static CBCosmosPinkPinkHostIO_c sCBCosmosPinkPinkHostIO; // 8058ACF4
static CBCosmosPinkOrangeHostIO_c sCBCosmosPinkOrangeHostIO; // 8058AD1C
static CBCosmosPinkBlackHostIO_c sCBCosmosPinkBlackHostIO; // 8058AD44
static CBCosmosOrangeOrangeHostIO_c sCBCosmosOrangeOrangeHostIO; // 8058AD6C
static CBCosmosOrangeBlackHostIO_c sCBCosmosOrangeBlackHostIO; // 8058AD94
static CBCosmosBlackBlackHostIO_c sCBCosmosBlackBlackHostIO; // 8058ADBC
static CBRoseRedRedHostIO_c sCBRoseRedRedHostIO; // 8058ADE4
static CBRoseRedWhiteHostIO_c sCBRoseRedWhiteHostIO; // 8058AE14
static CBRoseRedYellowHostIO_c sCBRoseRedYellowHostIO; // 8058AE44
static CBRoseRedPinkHostIO_c sCBRoseRedPinkHostIO; // 8058AE74
static CBRoseRedOrangeHostIO_c sCBRoseRedOrangeHostIO; // 8058AEA4
static CBRoseRedPurpleHostIO_c sCBRoseRedPurpleHostIO; // 8058AED4
static CBRoseRedBlackHostIO_c sCBRoseRedBlackHostIO; // 8058AF04
static CBRoseRedBlueHostIO_c sCBRoseRedBlueHostIO; // 8058AF34
static CBRoseWhiteWhiteHostIO_c sCBRoseWhiteWhiteHostIO; // 8058AF64
static CBRoseWhiteYellowHostIO_c sCBRoseWhiteYellowHostIO; // 8058AF94
static CBRoseWhitePinkHostIO_c sCBRoseWhitePinkHostIO; // 8058AFC4
static CBRoseWhiteOrangeHostIO_c sCBRoseWhiteOrangeHostIO; // 8058AFF4
static CBRoseWhitePurpleHostIO_c sCBRoseWhitePurpleHostIO; // 8058B024
static CBRoseWhiteBlackHostIO_c sCBRoseWhiteBlackHostIO; // 8058B054
static CBRoseWhiteBlueHostIO_c sCBRoseWhiteBlueHostIO; // 8058B084
static CBRoseYellowYellowHostIO_c sCBRoseYellowYellowHostIO; // 8058B0B4
static CBRoseYellowPinkHostIO_c sCBRoseYellowPinkHostIO; // 8058B0E4
static CBRoseYellowOrangeHostIO_c sCBRoseYellowOrangeHostIO; // 8058B114
static CBRoseYellowPurpleHostIO_c sCBRoseYellowPurpleHostIO; // 8058B144
static CBRoseYellowBlackHostIO_c sCBRoseYellowBlackHostIO; // 8058B174
static CBRoseYellowBlueHostIO_c sCBRoseYellowBlueHostIO; // 8058B1A4
static CBRosePinkPinkHostIO_c sCBRosePinkPinkHostIO; // 8058B1D4
static CBRosePinkOrangeHostIO_c sCBRosePinkOrangeHostIO; // 8058B204
static CBRosePinkPurpleHostIO_c sCBRosePinkPurpleHostIO; // 8058B234
static CBRosePinkBlackHostIO_c sCBRosePinkBlackHostIO; // 8058B264
static CBRosePinkBlueHostIO_c sCBRosePinkBlueHostIO; // 8058B294
static CBRoseOrangeOrangeHostIO_c sCBRoseOrangeOrangeHostIO; // 8058B2C4
static CBRoseOrangePurpleHostIO_c sCBRoseOrangePurpleHostIO; // 8058B2F4
static CBRoseOrangeBlackHostIO_c sCBRoseOrangeBlackHostIO; // 8058B324
static CBRoseOrangeBlueHostIO_c sCBRoseOrangeBlueHostIO; // 8058B354
static CBRosePurplePurpleHostIO_c sCBRosePurplePurpleHostIO; // 8058B384
static CBRosePurpleBlackHostIO_c sCBRosePurpleBlackHostIO; // 8058B3B4
static CBRosePurpleBlueHostIO_c sCBRosePurpleBlueHostIO; // 8058B3E4
static CBRoseBlackBlackHostIO_c sCBRoseBlackBlackHostIO; // 8058B414
static CBRoseBlackBlueHostIO_c sCBRoseBlackBlueHostIO; // 8058B444
static CBRoseBlueBlueHostIO_c sCBRoseBlueBlueHostIO; // 8058B474
static CBCarnationRedRedHostIO_c sCBCarnationRedRedHostIO; // 8058B4A8
static CBCarnationRedPinkHostIO_c sCBCarnationRedPinkHostIO; // 8058B4C8
static CBCarnationRedWhiteHostIO_c sCBCarnationRedWhiteHostIO; // 8058B4E8
static CBCarnationPinkPinkHostIO_c sCBCarnationPinkPinkHostIO; // 8058B508
static CBCarnationPinkWhiteHostIO_c sCBCarnationPinkWhiteHostIO; // 8058B528
static CBCarnationWhiteWhiteHostIO_c sCBCarnationWhiteWhiteHostIO; // 8058B548

static crossBreedTulipHostIO_c sCrossBreedTulipHostIO;         // 8074E35C
static crossBreedPansyHostIO_c sCrossBreedPansyHostIO;         // 8074E360
static crossBreedCosmosHostIO_c sCrossBreedCosmosHostIO;       // 8074E364
static crossBreedRoseHostIO_c sCrossBreedRoseHostIO;           // 8074E368
static crossBreedCarnationHostIO_c sCrossBreedCarnationHostIO; // 8074E36C
static crossBreedHostIO_c sCrossBreedHostIO;                   // 8074E370
static shellHostIO_c sShellHostIO;                             // 8058B5B0

// 804E1808..804E1ADC (.data): the cross-breeding tables, [color a][color b] -> rate object.
static dFgCBRate_c *sCBTulip[CBTulipBaseHostIO_c::TULIP_COLOR_NUM][CBTulipBaseHostIO_c::TULIP_COLOR_NUM] = {
    {(dFgCBRate_c *)&sCBTulipRedRedHostIO, (dFgCBRate_c *)&sCBTulipRedWhiteHostIO, (dFgCBRate_c *)&sCBTulipRedYellowHostIO, (dFgCBRate_c *)&sCBTulipRedPinkHostIO, (dFgCBRate_c *)&sCBTulipRedPurpleHostIO, (dFgCBRate_c *)&sCBTulipRedBlackHostIO},
    {(dFgCBRate_c *)&sCBTulipRedWhiteHostIO, (dFgCBRate_c *)&sCBTulipWhiteWhiteHostIO, (dFgCBRate_c *)&sCBTulipWhiteYellowHostIO, (dFgCBRate_c *)&sCBTulipWhitePinkHostIO, (dFgCBRate_c *)&sCBTulipWhitePurpleHostIO, (dFgCBRate_c *)&sCBTulipWhiteBlackHostIO},
    {(dFgCBRate_c *)&sCBTulipRedYellowHostIO, (dFgCBRate_c *)&sCBTulipWhiteYellowHostIO, (dFgCBRate_c *)&sCBTulipYellowYellowHostIO, (dFgCBRate_c *)&sCBTulipYellowPinkHostIO, (dFgCBRate_c *)&sCBTulipYellowPurpleHostIO, (dFgCBRate_c *)&sCBTulipYellowBlackHostIO},
    {(dFgCBRate_c *)&sCBTulipRedPinkHostIO, (dFgCBRate_c *)&sCBTulipWhitePinkHostIO, (dFgCBRate_c *)&sCBTulipYellowPinkHostIO, (dFgCBRate_c *)&sCBTulipPinkPinkHostIO, (dFgCBRate_c *)&sCBTulipPinkPurpleHostIO, (dFgCBRate_c *)&sCBTulipPinkBlackHostIO},
    {(dFgCBRate_c *)&sCBTulipRedPurpleHostIO, (dFgCBRate_c *)&sCBTulipWhitePurpleHostIO, (dFgCBRate_c *)&sCBTulipYellowPurpleHostIO, (dFgCBRate_c *)&sCBTulipPinkPurpleHostIO, (dFgCBRate_c *)&sCBTulipPurplePurpleHostIO, (dFgCBRate_c *)&sCBTulipPurpleBlackHostIO},
    {(dFgCBRate_c *)&sCBTulipRedBlackHostIO, (dFgCBRate_c *)&sCBTulipWhiteBlackHostIO, (dFgCBRate_c *)&sCBTulipYellowBlackHostIO, (dFgCBRate_c *)&sCBTulipPinkBlackHostIO, (dFgCBRate_c *)&sCBTulipPurpleBlackHostIO, (dFgCBRate_c *)&sCBTulipBlackBlackHostIO},
};
static dFgCBRate_c *sCBPansy[CBPansyBaseHostIO_c::PANSY_COLOR_NUM][CBPansyBaseHostIO_c::PANSY_COLOR_NUM] = {
    {(dFgCBRate_c *)&sCBPansyWhiteWhiteHostIO, (dFgCBRate_c *)&sCBPansyWhiteYellowHostIO, (dFgCBRate_c *)&sCBPansyWhiteRedHostIO, (dFgCBRate_c *)&sCBPansyWhitePurpleHostIO, (dFgCBRate_c *)&sCBPansyWhiteOrangeHostIO, (dFgCBRate_c *)&sCBPansyWhiteBlueHostIO},
    {(dFgCBRate_c *)&sCBPansyWhiteYellowHostIO, (dFgCBRate_c *)&sCBPansyYellowYellowHostIO, (dFgCBRate_c *)&sCBPansyYellowRedHostIO, (dFgCBRate_c *)&sCBPansyYellowPurpleHostIO, (dFgCBRate_c *)&sCBPansyYellowOrangeHostIO, (dFgCBRate_c *)&sCBPansyYellowBlueHostIO},
    {(dFgCBRate_c *)&sCBPansyWhiteRedHostIO, (dFgCBRate_c *)&sCBPansyYellowRedHostIO, (dFgCBRate_c *)&sCBPansyRedRedHostIO, (dFgCBRate_c *)&sCBPansyRedPurpleHostIO, (dFgCBRate_c *)&sCBPansyRedOrangeHostIO, (dFgCBRate_c *)&sCBPansyRedBlueHostIO},
    {(dFgCBRate_c *)&sCBPansyWhitePurpleHostIO, (dFgCBRate_c *)&sCBPansyYellowPurpleHostIO, (dFgCBRate_c *)&sCBPansyRedPurpleHostIO, (dFgCBRate_c *)&sCBPansyPurplePurpleHostIO, (dFgCBRate_c *)&sCBPansyPurpleOrangeHostIO, (dFgCBRate_c *)&sCBPansyPurpleBlueHostIO},
    {(dFgCBRate_c *)&sCBPansyWhiteOrangeHostIO, (dFgCBRate_c *)&sCBPansyYellowOrangeHostIO, (dFgCBRate_c *)&sCBPansyRedOrangeHostIO, (dFgCBRate_c *)&sCBPansyPurpleOrangeHostIO, (dFgCBRate_c *)&sCBPansyOrangeOrangeHostIO, (dFgCBRate_c *)&sCBPansyOrangeBlueHostIO},
    {(dFgCBRate_c *)&sCBPansyWhiteBlueHostIO, (dFgCBRate_c *)&sCBPansyYellowBlueHostIO, (dFgCBRate_c *)&sCBPansyRedBlueHostIO, (dFgCBRate_c *)&sCBPansyPurpleBlueHostIO, (dFgCBRate_c *)&sCBPansyOrangeBlueHostIO, (dFgCBRate_c *)&sCBPansyBlueBlueHostIO},
};
static dFgCBRate_c *sCBCosmos[CBCosmosBaseHostIO_c::COSMOS_COLOR_NUM][CBCosmosBaseHostIO_c::COSMOS_COLOR_NUM] = {
    {(dFgCBRate_c *)&sCBCosmosWhiteWhiteHostIO, (dFgCBRate_c *)&sCBCosmosWhiteRedHostIO, (dFgCBRate_c *)&sCBCosmosWhiteYellowHostIO, (dFgCBRate_c *)&sCBCosmosWhitePinkHostIO, (dFgCBRate_c *)&sCBCosmosWhiteOrangeHostIO, (dFgCBRate_c *)&sCBCosmosWhiteBlackHostIO},
    {(dFgCBRate_c *)&sCBCosmosWhiteRedHostIO, (dFgCBRate_c *)&sCBCosmosRedRedHostIO, (dFgCBRate_c *)&sCBCosmosRedYellowHostIO, (dFgCBRate_c *)&sCBCosmosRedPinkHostIO, (dFgCBRate_c *)&sCBCosmosRedOrangeHostIO, (dFgCBRate_c *)&sCBCosmosRedBlackHostIO},
    {(dFgCBRate_c *)&sCBCosmosWhiteYellowHostIO, (dFgCBRate_c *)&sCBCosmosRedYellowHostIO, (dFgCBRate_c *)&sCBCosmosYellowYellowHostIO, (dFgCBRate_c *)&sCBCosmosYellowPinkHostIO, (dFgCBRate_c *)&sCBCosmosYellowOrangeHostIO, (dFgCBRate_c *)&sCBCosmosYellowBlackHostIO},
    {(dFgCBRate_c *)&sCBCosmosWhitePinkHostIO, (dFgCBRate_c *)&sCBCosmosRedPinkHostIO, (dFgCBRate_c *)&sCBCosmosYellowPinkHostIO, (dFgCBRate_c *)&sCBCosmosPinkPinkHostIO, (dFgCBRate_c *)&sCBCosmosPinkOrangeHostIO, (dFgCBRate_c *)&sCBCosmosPinkBlackHostIO},
    {(dFgCBRate_c *)&sCBCosmosWhiteOrangeHostIO, (dFgCBRate_c *)&sCBCosmosRedOrangeHostIO, (dFgCBRate_c *)&sCBCosmosYellowOrangeHostIO, (dFgCBRate_c *)&sCBCosmosPinkOrangeHostIO, (dFgCBRate_c *)&sCBCosmosOrangeOrangeHostIO, (dFgCBRate_c *)&sCBCosmosOrangeBlackHostIO},
    {(dFgCBRate_c *)&sCBCosmosWhiteBlackHostIO, (dFgCBRate_c *)&sCBCosmosRedBlackHostIO, (dFgCBRate_c *)&sCBCosmosYellowBlackHostIO, (dFgCBRate_c *)&sCBCosmosPinkBlackHostIO, (dFgCBRate_c *)&sCBCosmosOrangeBlackHostIO, (dFgCBRate_c *)&sCBCosmosBlackBlackHostIO},
};
static dFgCBRate_c *sCBRose[CBRoseBaseHostIO_c::ROSE_COLOR_NUM][CBRoseBaseHostIO_c::ROSE_COLOR_NUM] = {
    {(dFgCBRate_c *)&sCBRoseRedRedHostIO, (dFgCBRate_c *)&sCBRoseRedWhiteHostIO, (dFgCBRate_c *)&sCBRoseRedYellowHostIO, (dFgCBRate_c *)&sCBRoseRedPinkHostIO, (dFgCBRate_c *)&sCBRoseRedOrangeHostIO, (dFgCBRate_c *)&sCBRoseRedPurpleHostIO, (dFgCBRate_c *)&sCBRoseRedBlackHostIO, (dFgCBRate_c *)&sCBRoseRedBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedWhiteHostIO, (dFgCBRate_c *)&sCBRoseWhiteWhiteHostIO, (dFgCBRate_c *)&sCBRoseWhiteYellowHostIO, (dFgCBRate_c *)&sCBRoseWhitePinkHostIO, (dFgCBRate_c *)&sCBRoseWhiteOrangeHostIO, (dFgCBRate_c *)&sCBRoseWhitePurpleHostIO, (dFgCBRate_c *)&sCBRoseWhiteBlackHostIO, (dFgCBRate_c *)&sCBRoseWhiteBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedYellowHostIO, (dFgCBRate_c *)&sCBRoseWhiteYellowHostIO, (dFgCBRate_c *)&sCBRoseYellowYellowHostIO, (dFgCBRate_c *)&sCBRoseYellowPinkHostIO, (dFgCBRate_c *)&sCBRoseYellowOrangeHostIO, (dFgCBRate_c *)&sCBRoseYellowPurpleHostIO, (dFgCBRate_c *)&sCBRoseYellowBlackHostIO, (dFgCBRate_c *)&sCBRoseYellowBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedPinkHostIO, (dFgCBRate_c *)&sCBRoseWhitePinkHostIO, (dFgCBRate_c *)&sCBRoseYellowPinkHostIO, (dFgCBRate_c *)&sCBRosePinkPinkHostIO, (dFgCBRate_c *)&sCBRosePinkOrangeHostIO, (dFgCBRate_c *)&sCBRosePinkPurpleHostIO, (dFgCBRate_c *)&sCBRosePinkBlackHostIO, (dFgCBRate_c *)&sCBRosePinkBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedOrangeHostIO, (dFgCBRate_c *)&sCBRoseWhiteOrangeHostIO, (dFgCBRate_c *)&sCBRoseYellowOrangeHostIO, (dFgCBRate_c *)&sCBRosePinkOrangeHostIO, (dFgCBRate_c *)&sCBRoseOrangeOrangeHostIO, (dFgCBRate_c *)&sCBRoseOrangePurpleHostIO, (dFgCBRate_c *)&sCBRoseOrangeBlackHostIO, (dFgCBRate_c *)&sCBRoseOrangeBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedPurpleHostIO, (dFgCBRate_c *)&sCBRoseWhitePurpleHostIO, (dFgCBRate_c *)&sCBRoseYellowPurpleHostIO, (dFgCBRate_c *)&sCBRosePinkPurpleHostIO, (dFgCBRate_c *)&sCBRoseOrangePurpleHostIO, (dFgCBRate_c *)&sCBRosePurplePurpleHostIO, (dFgCBRate_c *)&sCBRosePurpleBlackHostIO, (dFgCBRate_c *)&sCBRosePurpleBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedBlackHostIO, (dFgCBRate_c *)&sCBRoseWhiteBlackHostIO, (dFgCBRate_c *)&sCBRoseYellowBlackHostIO, (dFgCBRate_c *)&sCBRosePinkBlackHostIO, (dFgCBRate_c *)&sCBRoseOrangeBlackHostIO, (dFgCBRate_c *)&sCBRosePurpleBlackHostIO, (dFgCBRate_c *)&sCBRoseBlackBlackHostIO, (dFgCBRate_c *)&sCBRoseBlackBlueHostIO},
    {(dFgCBRate_c *)&sCBRoseRedBlueHostIO, (dFgCBRate_c *)&sCBRoseWhiteBlueHostIO, (dFgCBRate_c *)&sCBRoseYellowBlueHostIO, (dFgCBRate_c *)&sCBRosePinkBlueHostIO, (dFgCBRate_c *)&sCBRoseOrangeBlueHostIO, (dFgCBRate_c *)&sCBRosePurpleBlueHostIO, (dFgCBRate_c *)&sCBRoseBlackBlueHostIO, (dFgCBRate_c *)&sCBRoseBlueBlueHostIO},
};
static dFgCBRate_c *sCBCarnation[CBCarnationBaseHostIO_c::CARNATION_COLOR_NUM][CBCarnationBaseHostIO_c::CARNATION_COLOR_NUM] = {
    {(dFgCBRate_c *)&sCBCarnationRedRedHostIO, (dFgCBRate_c *)&sCBCarnationRedPinkHostIO, (dFgCBRate_c *)&sCBCarnationRedWhiteHostIO},
    {(dFgCBRate_c *)&sCBCarnationRedPinkHostIO, (dFgCBRate_c *)&sCBCarnationPinkPinkHostIO, (dFgCBRate_c *)&sCBCarnationPinkWhiteHostIO},
    {(dFgCBRate_c *)&sCBCarnationRedWhiteHostIO, (dFgCBRate_c *)&sCBCarnationPinkWhiteHostIO, (dFgCBRate_c *)&sCBCarnationWhiteWhiteHostIO},
};

#include <game/game/d_fg_mng_task.inc>

// 80091AF8: returns &gSceneChange, like getSceneChange (80161CC4); identical 3-insn getters exist
// in 6+ other TUs: 800573C4, 800D3584, 80166990, 8018DABC, 801B97A8.
dSceneChange_c *fn_80091AF8() {
    return &gSceneChange;
}

// 80091B04: returns 0x3400: size of the "createGrowUpHeap" EGG::ExpHeap lbl_8074E478 (created at
// 800B6270); the FG manager thread's 0x3000-byte stack comes from it.
int fgMngProc_getGrowUpHeapSize() {
    return 0x3400;
}

// 80091B0C: returns 0x2617C0: size of the "createFgHeap" FrmHeap lbl_8074E3F8 (created at
// 800B54B4); dFgMngProc_c::m_heap is made from it.
int fgMngProc_getFgHeapSize() {
    return 0x2617C0;
}

// 80091B18: days from the last processed day (fgMngProc_getLastDayTime, 6:00) to today 6:00 (now -
// 6 h); >= 1 means a day change is pending (fgMngProc_updateFrame). Also called by
// d_a_npc_sp_player_selectNP.
int fgMngProc_getElapsedDays() {
    dTime_c now;
    dTime_c procTime = fgMngProc_getLastDayTime();
    now = *dTime_c::getCurrent();
    now.add(0, -TIME_DAY_START_HOUR, 0, 0);
    now.hour = TIME_DAY_START_HOUR;
    now.usec = 0;
    now.msec = 0;
    now.sec = 0;
    now.min = 0;
    return dTime_c::diffDays(&now, &procTime, TRUE);
}

// 80091C24: returns sFgObjMgr; called ~50 times by d_fgobj_managerNP.
void *fgMngProc_getFgObjMgr() {
    return sFgObjMgr;
}

// 80091C2C: clears bit 2 of lbl_8074E9D0 (fn_8018EF28), clears sDayChangeBlocked
// (fgMngProc_unblockDayChange) and resets the unit locks / commands (fgMngSync_reset). Called from
// 80106DF0 right after dSvMgr_c::cancelRequest.
void fgMngProc_resetSync() {
    fn_8018EF28();
    fgMngProc_unblockDayChange();
    fgMngSync_reset();
}

// 80091C54: the thread function (passed to startFgMngProcThread by 801544D8 / 801545AC)
void *fgMngProcThreadFunc(void *arg) {
    fgMngProc_threadMain();
    return NULL;
}

// 80091C78
void startFgMngProcThread(OSThreadFunc func) {
    if (dSaveData_c::getTown()->isFlag(0x1A)) {
        dSaveData_c::getTown()->setFlag(6);
    } else {
        dSaveData_c::getTown()->clearFlag(6);
    }
    sFgMngProcFlags |= FG_MNG_PROC_FLAG_THREAD;
    sFgMngProcThreadStack = lbl_8074E478->alloc(FG_MNG_THREAD_STACK_SIZE, 0x20);
    OSCreateThread(&sFgMngProcThread, func, NULL, (u8 *)sFgMngProcThreadStack + FG_MNG_THREAD_STACK_SIZE, FG_MNG_THREAD_STACK_SIZE, 0x19, 1);
    fn_802B8D30(&sFgMngProcThread, fn_8016CF08(0xC));
    OSResumeThread(&sFgMngProcThread);
    dTime_c::stop();
}

static inline void freeThreadStack() {
    if (sFgMngProcThreadStack != NULL) {
        lbl_8074E478->free(sFgMngProcThreadStack);
        sFgMngProcThreadStack = NULL;
        fn_802B8D90(&sFgMngProcThread);
    }
}

// 80091D4C: TRUE (after freeing the thread's stack) once the thread has finished
BOOL isFgMngProcThreadDone() {
    if (OSIsThreadTerminated(&sFgMngProcThread)) {
        freeThreadStack();
        dTime_c::start();
        return TRUE;
    }
    return FALSE;
}

// 80091DC4
void stopFgMngProcThread() {
    if (!isFgMngProcThreadDone()) {
        OSCancelThread(&sFgMngProcThread);
        freeThreadStack();
        dTime_c::start();
    }
}

// 80091E34: sFgMngTaskList.reset(): resets all 32 tasks. Called together with fgMngSync_reset by
// 800E3708 / 800E372C (network code).
void fgMngTask_resetAll() {
    sFgMngTaskList.reset();
}

// ---------------------------------------------------------------------------------------------
// Field assessment

static inline BOOL inRange(dItem::Item *item, u16 lo, u16 hi) {
    BOOL result = FALSE;
    if (item->mId >= lo && item->mId <= hi) {
        result = TRUE;
    }
    return result;
}

// 80091E40
void dFdAsBlock_c::assess(dFdBase_c *fd, int blockX, int blockZ) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mTreeNum[j][i] = 0;
        }
    }
    mGrassNum = 0;
    mItemNum = 0;
    mDustNum = 0;
    mFlowerNum = 0;
    mFlags.mRaw = 0;
    mRafflesia.mX = -1;
    mRafflesia.mZ = -1;
    mStoneNum = 0;
    mCedarB56Num = 0;
    mShellNum = 0;
    mFossilNum = 0;
    mMushroomNum = 0;
    mMushFtrNum = 0;

    u16 id;
    dItem::Item *item;
    int x;
    int z;
    item = fd->getItem(blockX, blockZ, 0, 0, 0);
    for (z = 0; z < UT_Z_NUM; z++) {
        for (x = 0; x < UT_X_NUM; x++) {
            id = item->mId;
            switch (ITEM_NAME_TYPE(id)) {
            case 0: {
                dItem::FgInfo *info = item->getFgInfo();
                if (info->mTreeStage >= 0) {
                    mTreeNum[x >> 3][z >> 3]++;
                    switch (id) {
                    case dItem::FG_TREE_FTR:
                    case dItem::FG_CEDAR_FTR:
                        mFlags.mBits.mHasTreeA = TRUE;
                        break;
                    case dItem::FG_TREE_BEES:
                    case dItem::FG_CEDAR_BEES:
                        mFlags.mBits.mHasTreeB = TRUE;
                        break;
                    case dItem::FG_TREE_BELLS:
                    case dItem::FG_CEDAR_BELLS:
                        mFlags.mBits.mHasTreeC = TRUE;
                        break;
                    }
                    if (inRange(item, dItem::FG_TREE_SAPLING, dItem::FG_TREE_BELLS) && id != dItem::FG_TREE_FTR && id != dItem::FG_TREE_BEES && id != dItem::FG_TREE_BELLS) {
                        mFlags.mBits.mHasTree = TRUE;
                    } else if (inRange(item, dItem::FG_CEDAR_SAPLING, dItem::FG_CEDAR_LIGHTS) && info->mTreeStage >= 3) {
                        mFlags.mBits.mHasCedar = TRUE;
                    } else if (id == dItem::FG_CEDAR_LIGHTS) {
                        mCedarB56Num++;
                    }
                } else if (item->isWiltedFlower()) {
                    if (fd->isWatered(blockX, blockZ, x, z) || id == dItem::FG_WILTED_ROSE_GOLD) {
                        mFlags.mBits.mHasFlower = TRUE;
                        mFlowerNum++;
                    }
                } else if (id == dItem::FG_RAFFLESIA) {
                    mRafflesia.mX = x;
                    mRafflesia.mZ = z;
                    mFlags.mBits.mHasRafflesia = TRUE;
                } else if (item->isFlower()) {
                    mFlags.mBits.mHasFlower = TRUE;
                    mFlowerNum++;
                    if (item->mId == dItem::FG_JACOBS_LADDER) {
                        mFlags.mBits.mHasLily = TRUE;
                    }
                } else if (inRange(item, dItem::FG_WEED_A, dItem::FG_WEED_D)) {
                    mGrassNum++;
                } else if (inRange(item, dItem::FG_DANDELION, dItem::FG_DANDELION_PUFF)) {
                    mFlowerNum++;
                } else if (inRange(item, dItem::FG_STONE_A, dItem::FG_STONE_E)) {
                    mStoneNum++;
                } else if (inRange(item, dItem::FG_MONEY_ROCK_P0_A, dItem::FG_MONEY_ROCK_P3_E)) {
                    mFlags.mBits.mStoneKinds |= 1 << ((u32)(item->mId - dItem::FG_MONEY_ROCK_P0_A) / 5);
                }
                break;
            }
            case 9:
            case 10:
            case 11:
            case 12: {
                dItem::Item coconut(dItem::ITEM_IDX_COCONUT);
                if (id == coconut.mId) {
                    mFlags.mBits.mHasCoconut = TRUE;
                } else if (item->isDust()) {
                    if (!fd->isBuried(blockX, blockZ, x, z)) {
                        mFlags.mBits.mHasDust = TRUE;
                    }
                    mDustNum++;
                }
                dItem::Item badKabu(dItem::ITEM_IDX_SPOILED_TURNIPS);
                if (id == badKabu.mId) {
                    if (!fd->isBuried(blockX, blockZ, x, z)) {
                        mFlags.mBits.mHasBadKabu = TRUE;
                    }
                    mDustNum++;
                } else if (item->isMushroom()) {
                    mMushroomNum++;
                } else if (!fd->isBuried(blockX, blockZ, x, z)) {
                    if (item->isShell()) {
                        mShellNum++;
                    } else {
                        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                        if (bitm != NULL) {
                            switch (bitm->getKind()) {
                            case dItem::KIND_FRUIT:
                                break;
                            case dItem::KIND_MUSH_FTR:
                                mMushFtrNum++;
                                break;
                            case dItem::KIND_CANDY:
                                mFlags.mBits.mHasCandy = TRUE;
                            default:
                                mItemNum++;
                                break;
                            }
                        }
                    }
                } else if (item->isSame(dItem::Item(dItem::ITEM_IDX_FOSSIL))) {
                    mFossilNum++;
                } else if (item->isSame(dItem::Item(dItem::ITEM_IDX_PITFALL_SEED))) {
                    mFlags.mBits.mHasPitfall = TRUE;
                }
                break;
            }
            }
            item++;
        }
    }
    calcPoint();
}

// 80092420
void dFdAsBlock_c::assessLive(dFdBase_c *fd, int blockX, int blockZ) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            mTreeNum[j][i] = 0;
        }
    }
    mGrassNum = 0;
    mItemNum = 0;
    mDustNum = 0;
    mFlowerNum = 0;
    mFlags.mRaw = 0;
    mRafflesia.mX = -1;
    mRafflesia.mZ = -1;
    mStoneNum = 0;
    mCedarB56Num = 0;
    mShellNum = 0;
    mFossilNum = 0;
    mMushroomNum = 0;
    mMushFtrNum = 0;

    u16 id;
    dItem::Item *item;
    int x;
    int z;
    item = fd->getItem(blockX, blockZ, 0, 0, 0);
    for (z = 0; z < UT_Z_NUM; z++) {
        for (x = 0; x < UT_X_NUM; x++) {
            id = item->mId;
            switch (ITEM_NAME_TYPE(id)) {
            case 0: {
                dItem::FgInfo *info = item->getFgInfo();
                if (info->mTreeStage >= 0) {
                    mTreeNum[x >> 3][z >> 3]++;
                    switch (id) {
                    case dItem::FG_TREE_FTR:
                    case dItem::FG_CEDAR_FTR:
                        mFlags.mBits.mHasTreeA = TRUE;
                        break;
                    case dItem::FG_TREE_BEES:
                    case dItem::FG_CEDAR_BEES:
                        mFlags.mBits.mHasTreeB = TRUE;
                        break;
                    case dItem::FG_TREE_BELLS:
                    case dItem::FG_CEDAR_BELLS:
                        mFlags.mBits.mHasTreeC = TRUE;
                        break;
                    }
                    if (inRange(item, dItem::FG_TREE_SAPLING, dItem::FG_TREE_BELLS) && id != dItem::FG_TREE_FTR && id != dItem::FG_TREE_BEES && id != dItem::FG_TREE_BELLS) {
                        mFlags.mBits.mHasTree = TRUE;
                    } else if (inRange(item, dItem::FG_CEDAR_SAPLING, dItem::FG_CEDAR_LIGHTS) && info->mTreeStage == 4) {
                        mFlags.mBits.mHasCedar = TRUE;
                    } else if (id == dItem::FG_CEDAR_LIGHTS) {
                        mCedarB56Num++;
                    }
                } else if (inRange(item, dItem::FG_WEED_A, dItem::FG_WEED_D)) {
                    mGrassNum++;
                } else if ((item->isWiltedFlower() && id != dItem::FG_WILTED_RAFFLESIA) || item->isFlower()) {
                    mFlags.mBits.mHasFlower = TRUE;
                    mFlowerNum++;
                    if (item->mId == dItem::FG_JACOBS_LADDER) {
                        mFlags.mBits.mHasLily = TRUE;
                    }
                } else if (inRange(item, dItem::FG_DANDELION, dItem::FG_DANDELION_PUFF)) {
                    mFlowerNum++;
                } else if (item->mId == dItem::FG_RAFFLESIA) {
                    mRafflesia.mX = x;
                    mRafflesia.mZ = z;
                    mFlags.mBits.mHasRafflesia = TRUE;
                }
                break;
            }
            case 9:
            case 10:
            case 11:
            case 12: {
                if (item->isDust()) {
                    if (!fd->isBuried(blockX, blockZ, x, z)) {
                        mFlags.mBits.mHasDust = TRUE;
                    }
                    mDustNum++;
                }
                dItem::Item badKabu(dItem::ITEM_IDX_SPOILED_TURNIPS);
                if (id == badKabu.mId) {
                    if (!fd->isBuried(blockX, blockZ, x, z)) {
                        mFlags.mBits.mHasBadKabu = TRUE;
                    }
                    mDustNum++;
                } else if (!fd->isBuried(blockX, blockZ, x, z)) {
                    if (item->isShell()) {
                        mShellNum++;
                    } else {
                        const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
                        if (bitm != NULL) {
                            switch (bitm->getKind()) {
                            case dItem::KIND_CANDY:
                                mFlags.mBits.mHasCandy = TRUE;
                            default:
                                mItemNum++;
                                break;
                            case dItem::KIND_FRUIT:
                            case dItem::KIND_MUSHROOM:
                                break;
                            }
                        }
                    }
                } else if (item->isSame(dItem::Item(dItem::ITEM_IDX_FOSSIL))) {
                    mFossilNum++;
                } else if (item->isSame(dItem::Item(dItem::ITEM_IDX_PITFALL_SEED))) {
                    mFlags.mBits.mHasPitfall = TRUE;
                }
                break;
            }
            }
            item++;
        }
    }
    calcPoint();
}

// 800928E8
void dFdAsBlock_c::calcPoint() {
    int trees = getTreeNum();
    mPoint = POINT_BASE;
    if (trees > TREE_NUM_MAX) {
        mPoint = POINT_BASE - (trees - TREE_NUM_MAX) * 2;
    } else if (trees < TREE_NUM_MIN) {
        mPoint = POINT_BASE - (TREE_NUM_MIN - trees) * 2;
    }
    if (mGrassNum >= 3) {
        mPoint -= mGrassNum - 2;
    }
    if (mItemNum >= 3) {
        mPoint -= mItemNum - 2;
    }
    mPoint -= mDustNum * DUST_PENALTY;
    if (mFlags.mBits.mHasRafflesia) {
        mPoint -= RAFFLESIA_PENALTY;
    }
    if (mFlowerNum >= 3) {
        mPoint += mFlowerNum - 2;
    }
    if (mPoint < 0) {
        mPoint = 0;
    } else if (mPoint > POINT_MAX) {
        mPoint = POINT_MAX;
    }
}

// 800929E8
dFdAsWorst_c::dFdAsWorst_c() {
    dFdAsPos_c pos(-1, -1);
    setPos(pos);
    mReason = REASON_NONE;
}

// 80092A14
dFdAsWorst_c::~dFdAsWorst_c() {}

// 80092A54
void dFdAsWorst_c::set(dFdAsPos_c pos, dFdAsBlock_c *block) {
    mPos = pos;
    if (mPos.mX < 0) {
        mReason = REASON_NONE;
        return;
    }

    int penalty[REASON_NUM];
    for (int i = 0; i < REASON_NUM; i++) {
        penalty[i] = 0;
    }
    int trees = block->getTreeNum();
    if (trees > dFdAsBlock_c::TREE_NUM_MAX) {
        penalty[REASON_TREE_MANY] = (trees - dFdAsBlock_c::TREE_NUM_MAX) * 2;
    } else if (trees < dFdAsBlock_c::TREE_NUM_MIN) {
        penalty[REASON_TREE_FEW] = (dFdAsBlock_c::TREE_NUM_MIN - trees) * 2;
    }
    int num = block->mGrassNum;
    if (num >= 3) {
        penalty[REASON_GRASS] = num - 2;
    }
    num = block->mItemNum;
    if (num >= 3) {
        penalty[REASON_ITEM] = num - 2;
    }
    penalty[REASON_DUST] = block->mDustNum * dFdAsBlock_c::DUST_PENALTY;
    if (block->mFlags.mBits.mHasRafflesia) {
        penalty[REASON_RAFFLESIA] = dFdAsBlock_c::RAFFLESIA_PENALTY;
    }

    int max = -1;
    int reason = REASON_NONE;
    for (int i = 0; i < REASON_NUM; i++) {
        if (penalty[i] > max) {
            max = penalty[i];
            reason = i;
        }
    }
    mReason = reason;
}

// 80092B70
void dFdAssess_c::assessTown(dFdBase_c *fd, int blockW, int blockH) {
    int x;
    int z;
    int minPoint;
    dFdAsPos_c worst;
    int point;

    minPoint = 100;
    worst.mX = -1;
    worst.mZ = -1;
    mLilyBlockNum = 0;
    mFlowerNum = 0;
    mFlowerBlockNum = 0;
    mGrassNum = 0;
    mTreeNum = 0;
    mTreeABlockNum = 0;
    mFossilNum = 0;
    mMushroomNum = 0;
    mMushFtrNum = 0;
    mRafflesiaBlock.mX = -1;
    mRafflesiaBlock.mZ = -1;
    mRafflesiaUnit.mX = -1;
    mRafflesiaUnit.mZ = -1;
    mStoneNum = 0;
    mFlags.mRaw = 0;
    for (int i = 0; i < dFdAsBlock_c::RANK_NUM; i++) {
        sFdAssess.mBlockRankNum[i] = 0;
    }

    for (z = 0; z < blockH; z++) {
        for (x = 0; x < blockW; x++) {
            dFdAsBlock_c *block = getBlock(x, z);
            block->assess(fd, x + 1, z + 1);
            if (block->mFlags.mBits.mHasLily) {
                mLilyBlockNum++;
            }
            if (block->mFlags.mBits.mHasFlower) {
                mFlowerBlockNum++;
            }
            if (block->mFlags.mBits.mHasTreeA) {
                mTreeABlockNum++;
            }
            if (block->mFossilNum != 0) {
                mFossilNum += block->mFossilNum;
            }
            mMushroomNum += block->mMushroomNum;
            mMushFtrNum += block->mMushFtrNum;
            mStoneNum += block->mStoneNum;
            mFlags.mBits.mStoneKinds |= block->mFlags.mBits.mStoneKinds;
            mFlags.mBits.mHasCoconut |= block->mFlags.mBits.mHasCoconut;
            mFlags.mBits.mHasPitfall |= block->mFlags.mBits.mHasPitfall;
            mTreeNum += block->getTreeNum();
            mFlowerNum += block->mFlowerNum;
            mGrassNum += block->mGrassNum;
            mFlags.mBits.mHasDust |= block->mFlags.mBits.mHasDust;
            mFlags.mBits.mHasBadKabu |= block->mFlags.mBits.mHasBadKabu;
            mFlags.mBits.mHasCandy |= block->mFlags.mBits.mHasCandy;

            point = block->mPoint;
            sFdAssess.mBlockRankNum[getBlockRank(point)]++;
            if (point < minPoint) {
                worst.mX = x;
                minPoint = point;
                worst.mZ = z;
            }
            if (block->mFlags.mBits.mHasRafflesia) {
                mRafflesiaBlock.mX = x;
                mRafflesiaBlock.mZ = z;
                mRafflesiaUnit = block->getRafflesia();
            }
        }
    }

    mWorst.set(worst, &mBlocks[worst.mX][worst.mZ]);
    mRank = getTownRank(sFdAssess.mBlockRankNum, blockW, blockH);
}

// 80092E88
int dFdAssess_c::assessLiveTown() {
    dFdBase_c *fd;
    dFdAsBlock_c *row;
    dFdAsBlock_c *block;
    int x;
    int z;
    int blockW;
    int blockH;
    int minPoint;
    dFdAsPos_c worst;
    int point;

    minPoint = 10000;
    worst.mX = -1;
    worst.mZ = -1;
    mLilyBlockNum = 0;
    mFlowerNum = 0;
    mFlowerBlockNum = 0;
    mGrassNum = 0;
    mTreeNum = 0;
    mTreeABlockNum = 0;
    mFossilNum = 0;
    mMushroomNum = 0;
    mMushFtrNum = 0;
    mRafflesiaBlock.mX = -1;
    mRafflesiaBlock.mZ = -1;
    mRafflesiaUnit.mX = -1;
    mRafflesiaUnit.mZ = -1;
    mStoneNum = 0;
    mFlags.mRaw = 0;
    for (int i = 0; i < dFdAsBlock_c::RANK_NUM; i++) {
        sFdAssess.mBlockRankNum[i] = 0;
    }

    fd = fn_80190C44(1);
    blockW = fd->mBlockW - 2;
    blockH = fd->mBlockH - 2;
    row = mBlocks[0];
    for (x = 0; x < blockW; x++) {
        block = row;
        for (z = 0; z < blockH; z++) {
            block->assessLive(fd, x + 1, z + 1);
            if (block->mFlags.mBits.mHasLily) {
                mLilyBlockNum++;
            }
            if (block->mFlags.mBits.mHasFlower) {
                mFlowerBlockNum++;
            }
            if (block->mFlags.mBits.mHasTreeA) {
                mTreeABlockNum++;
            }
            if (block->mFossilNum != 0) {
                mFossilNum += block->mFossilNum;
            }
            mMushroomNum += block->mMushroomNum;
            mMushFtrNum += block->mMushFtrNum;
            mFlags.mBits.mHasPitfall |= block->mFlags.mBits.mHasPitfall;
            mTreeNum += block->getTreeNum();
            mFlowerNum += block->mFlowerNum;
            mGrassNum += block->mGrassNum;
            mFlags.mBits.mHasDust |= block->mFlags.mBits.mHasDust;
            mFlags.mBits.mHasBadKabu |= block->mFlags.mBits.mHasBadKabu;
            mFlags.mBits.mHasCandy |= block->mFlags.mBits.mHasCandy;

            point = block->mPoint;
            sFdAssess.mBlockRankNum[getBlockRank(point)]++;
            if (point < minPoint) {
                worst.mX = x;
                minPoint = point;
                worst.mZ = z;
            }
            if (block->mFlags.mBits.mHasRafflesia) {
                mRafflesiaBlock.mX = x;
                mRafflesiaBlock.mZ = z;
                mRafflesiaUnit = block->getRafflesia();
            }
            block++;
        }
        row += FG_BLOCK_Z_NUM;
    }

    mWorst.set(worst, getBlock(worst.mX, worst.mZ));
    return mRank = getTownRank(sFdAssess.mBlockRankNum, blockW, blockH);
}

// 8009316C
int dFdAssess_c::getTownRank(int *blockRankNum, int blockW, int blockH) {
    int total = blockW * blockH;
    if (blockRankNum[dFdAsBlock_c::RANK_BAD] == total) {
        return TOWN_RANK_BAD;
    }
    if (blockRankNum[dFdAsBlock_c::RANK_BAD] > 0 || blockRankNum[dFdAsBlock_c::RANK_POOR] > 0) {
        return TOWN_RANK_POOR;
    }
    if (blockRankNum[dFdAsBlock_c::RANK_FAIR] > 0) {
        return TOWN_RANK_FAIR;
    }
    return blockRankNum[dFdAsBlock_c::RANK_GREAT] < PERFECT_GREAT_BLOCK_NUM;
}

// 800931D0
int dFdAssess_c::getBlockRank(int point) {
    if (point < dFdAsBlock_c::POINT_POOR) {
        return dFdAsBlock_c::RANK_BAD;
    }
    if (point < dFdAsBlock_c::POINT_FAIR) {
        return dFdAsBlock_c::RANK_POOR;
    }
    if (point < dFdAsBlock_c::POINT_GOOD) {
        return dFdAsBlock_c::RANK_FAIR;
    }
    return point < dFdAsBlock_c::POINT_GREAT;
}

// 80093218
void dFdAssess_c::assessLiveBlock(dFdBase_c *fd, int blockX, int blockZ) {
    mBlocks[blockX][blockZ].assessLive(fd, blockX + 1, blockZ + 1);
}

// 80101DC4 / 80101DC0 are declared without parameters in d_player_mgr.hpp (tail calls to
// 801B9304 / 801B9258); called here with their real arguments.
typedef BOOL (*dFgMngPlInfoFunc)(void *a, u8 *scene, u8 *b, u32 idx);
typedef BOOL (*dFgMngPlPosFunc)(void *a, f32 *pos, u8 *b, u32 idx);

// The cross-breeding searches (RTTI CBBkSearch_c / CBUtSearch_c): the usable blocks, then the
// flowers of one block.
class CBBkSearch_c : public dFdBkSearchCand_c {
public:
    CBBkSearch_c() : dFdBkSearchCand_c(BLOCK_X_NUM, BLOCK_Z_NUM) {}
};

class CBUtSearch_c : public dSearchCandXZ_c<UT_X_NUM, UT_Z_NUM> {
public:
    CBUtSearch_c(dFdBase_c *fd, int blockX, int blockZ) : mFd(fd), mBlockX(blockX), mBlockZ(blockZ) {}
    virtual BOOL check(int x, int z); // 800961D0

    /* 0x3C */ dFdBase_c *mFd;
    /* 0x40 */ int mBlockX;
    /* 0x44 */ int mBlockZ;
}; // size 0x48

// 80093238: frees all unit locks and commands.
void fgMngSync_reset() {
    int i;
    for (i = 0; i < FG_MNG_LOCK_NUM; i++) {
        sFgMngLocks[i].mPendingPlayers = 0;
    }
    for (i = 0; i < FG_MNG_CMD_NUM; i++) {
        sFgMngCmds[i].init();
    }
    for (i = 0; i < FG_MNG_CMD_NUM / 8; i++) {
        sFgMngCmdRetryBits[i] = 0;
    }
}

// 80093388: the index of the lock of (scene, unit, layer), or -1.
int fgMngLock_find(u32 scene, dFdAsPos_c *pos, u32 layer) {
    dFgMngLock_c *lock = fgMngLock_get(0);
    for (int i = 0; i < FG_MNG_LOCK_NUM; lock++, i++) {
        if (lock->mPendingPlayers != 0 && scene == lock->mScene && layer == lock->mLayer && (lock->mPos.mRaw >> 8) == pos->mX &&
            (lock->mPos.mRaw & 0xFF) == pos->mZ) {
            return i;
        }
    }
    return -1;
}

// 80093490: the index of the command (player, kind, scene, unit, layer), or -1.
int fgMngCmd_find(int player, u32 kind, u32 scene, dFdAsPos_c *pos, u32 layer) {
    BOOL same;
    BOOL found;
    BOOL sameZ;
    BOOL sameX;
    dFgMngCmd_c *cmd = fgMngCmd_get(0);
    for (int i = 0; i < FG_MNG_CMD_NUM; i++, cmd++) {
        found = FALSE;
        sameZ = FALSE;
        sameX = FALSE;
        same = FALSE;
        if (cmd->mNewItem != dFgMngCmd_c::ITEM_FREE && player == cmd->mPlayer && kind == cmd->mKind && scene == cmd->mScene) {
            same = TRUE;
        }
        if (same && (cmd->mPos >> 8) == pos->mX) {
            sameX = TRUE;
        }
        if (sameX && (cmd->mPos & 0xFF) == pos->mZ) {
            sameZ = TRUE;
        }
        if (sameZ && layer == cmd->mLayer) {
            found = TRUE;
        }
        if (found) {
            return i;
        }
    }
    return -1;
}

// 800935A8: a free lock, or -1 (used unchecked by fgMngLock_add).
int fgMngLock_findFree() {
    for (int i = 0; i < FG_MNG_LOCK_NUM; i++) {
        if (fgMngLock_get(i)->mPendingPlayers == 0) {
            return i;
        }
    }
    return -1;
}

// 800935FC
dFgMngLock_c *fgMngLock_get(int idx) {
    return &sFgMngLocks[idx];
}

// 80093610: runs a command (accepted: stores and executes it, retried later if that fails) and
// sends it to the other consoles.
void fgMngCmd_apply(dFgMngCmd_c *cmd, u32 arg) {
    dFgMngPlayerState_c *slot = &sFgMngState.mPlayers[cmd->mReqPlayer];
    if (!cmd->mLocal) {
        int player = cmd->mPlayer;
        if (player == fn_800DCF58()) {
            slot->mState = dFgMngPlayerState_c::STATE_REJECTED;
        }
    } else {
        int kind = cmd->mKind;
        int idx = fgMngCmd_findFree();
        (dFgMngCmd_c &)sFgMngCmds[idx] = *cmd;
        if (!cmd->exec()) {
            fgMngCmd_setRetry(idx);
        }
        int player = cmd->mPlayer;
        if (player == fn_800DCF58()) {
            slot->mState = dFgMngPlayerState_c::STATE_ACCEPTED;
            switch (kind) {
            case FG_MNG_KIND_BLOCKED:
            case FG_MNG_KIND_HIT_ROCK:
            case FG_MNG_KIND_HIT_MONEY_ROCK:
            case FG_MNG_KIND_WATER:
            case FG_MNG_KIND_PUT:
            case FG_MNG_KIND_SWAP:
            case FG_MNG_KIND_PUT_PENDING:
            case FG_MNG_KIND_MISS:
            case FG_MNG_KIND_PLANT:
            case FG_MNG_KIND_PUT_DESIGN:
                break;
            default:
                if (arg <= dFgMngCmd_c::SEND_SIZE_FULL) {
                    fn_800DD4C8();
                    fn_800DD518(cmd, arg);
                    fn_800DD588(FG_MNG_PACKET_CMD, FG_MNG_NET_ALL);
                }
                break;
            }
        }
    }
}

// 80093778
dFgMngCmd_c *fgMngCmd_get(int idx) {
    return &sFgMngCmds[idx];
}

// 8009378C: locks (scene, unit, layer) for every player present. The task test can never be true.
void fgMngLock_add(u32 scene, dFdAsPos_c *pos, u32 layer) {
    if (fgMngLock_find(scene, pos, layer) < 0 || fgMngTask_find(scene, pos, layer) < -1) {
        dFgMngLock_c *lock = fgMngLock_get(fgMngLock_findFree());
        if (!fn_800DCEDC()) {
            lock->mPendingPlayers |= 1;
        } else {
            for (int i = 0; i < 4; i++) {
                if (fn_800DCF2C(i)) {
                    lock->mPendingPlayers |= 1 << i;
                }
            }
        }
        dFgMngPos16_c p;
        p.mPos.mX = pos->mX;
        p.mPos.mZ = pos->mZ;
        lock->mPos = p;
        lock->mLayer = layer;
        lock->mScene = scene;
    }
}

// 8009387C
void fgMngCmd_setRetry(int idx) {
    sFgMngCmdRetryBits[idx >> 3] |= 1 << (idx & 7);
}

// 800938A0
void fgMngCmd_clearRetry(int idx) {
    sFgMngCmdRetryBits[idx >> 3] &= ~(1 << (idx & 7));
}

// 800938C4
BOOL fgMngCmd_isRetry(int idx) {
    return (sFgMngCmdRetryBits[idx >> 3] & (1 << (idx & 7))) != 0;
}

// 800938F0: a free command (0x3F if none).
int fgMngCmd_findFree() {
    dFgMngCmd_c *cmd = sFgMngCmds;
    for (int i = 0; i < FG_MNG_CMD_NUM; i++, cmd++) {
        if (cmd->mNewItem == dFgMngCmd_c::ITEM_FREE) {
            return i;
        }
    }
    return FG_MNG_CMD_NUM - 1;
}

// 80093994
void dFgMngProc_c::destroyHeap() {
    if (m_heap != NULL) {
        m_heap->free(3);
        mHeap::destroyFrmHeap(m_heap);
        m_heap = NULL;
        mBuffer = NULL;
    }
}

// 800939E8: the day-change processing of the field for `days` days (last..now).
void dFgMngProc_c::processDays(dTime_c *now, dTime_c *last, int days, BOOL flag, BOOL arg) {
    int seed = *(int *)((u8 *)dSaveData_c::getTown() + 0x5EC60);
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    int w = fd->mBlockW - 2;
    int h = fd->mBlockH - 2;
    if (flag) {
        markFlowerUnits(fd, w, h);
    }
    resetStones(fd, w, h);
    sFdAssess.assessTown(fd, w, h);
    int rank = sFdAssess.mRank;
    dSaveTown_c *town = dSaveData_c::getTown();
    fn_8014D3F4(town->_068372, rank, days);
    fgMngProc_seedRandom(now, seed + 0x2221);
    int num = days;
    if (days > 5) {
        num = 5;
    }
    removeDeadSaplings(fd);
    for (int i = 0; i < num; i++) {
        updateTrees(fd, w, h);
    }
    fn_80169ED8();
    dSaveData_c::getTown()->mHomes.updateAll(days);
    createHeap();
    dFgMngProc_c::setupUnitMask(fd, (u16 *)mBuffer);
    procDay(fd, last);
    if (fn_8014D844(dSaveData_c::getTown()->_068372, EVENT_BUNNY_DAY) >= 0) {
        updateEggs();
    }
    fgMngProc_seedRandom(now, seed + 0x2222);
    plantTrees(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x2224);
    tryPutRafflesia(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x2225);
    buryFossils(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x2226);
    buryPitfall(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x2227);
    changeStones(fd, w, h);
    growRedKabu(fd, days);
    fgMngProc_seedRandom(now, seed + 0x2228);
    putCoconut(fd, w);
    fgMngProc_seedRandom(now, seed + 0x2229);
    buryGyroids(fd, w, h, flag);
    if (isMushroomSeason(last)) {
        fgMngProc_seedRandom(now, seed + 0x222A);
        putMushrooms(fd);
    }
    makeGoldenShovels(fd, w, h);
    fd->fn_8008D70C((const nw4r::math::VEC3 *)days, 3);
    dSaveData_c::getTown()->mPoliceBox.refill(days);
    dSaveData_c::getTown()->mRecycleBin.update(last, days);
    clearShopRoomMaps();
    if ((u8)dEvent::getTodayVisitor() == 6) {
        dSaveData_c::getTown()->clearFlag(0x12);
        dSaveData_c::getTown()->clearFlag(0x13);
    }
    if (dSaveData_c::getRaw()->isFlag(0x12)) {
        dSaveData_c::getTown()->setFlag(0x13);
    }
    dSaveData_c::getTown()->clearFlag(0x12);
    fn_80169C38();
    dSaveData_c::getTown()->clearFlag(0x19);
    if (dSaveData_c::getRaw()->isFlag(0x13)) {
        fn_80169BF4();
    }
    dSaveData_c::getTown()->clearFlag(0x16);
    fgMngProc_seedRandom(now, seed + 0x2232);
    fn_800D11C8(last, days);
    dSaveData_c::getTown()->mShops.update();
    dSaveData_c::getExtra()->mDesignBoard.decrease(days);
    if (days >= TIME_DAYS_PER_WEEK || last->wday < now->wday) {
        spoilAllKabu();
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag1(0x44)) {
        player->_55F8 = dSaveData_c::getExtra()->mDistPattern.mVersion;
    }
    num = days;
    if ((u32)days > 50) {
        num = 50;
    }
    fgMngProc_seedRandom(now, seed + 0x222B);
    spreadWeeds(fd, num);
    num = days;
    if ((u32)days > 50) {
        num = 50;
    }
    fgMngProc_seedRandom(now, seed + 0x222C);
    plantClovers(fd, *last, num, w, h);
    if (dSaveData_c::getRaw()->isFlag(0x11)) {
        removeWeeds(fd);
        dSaveData_c::getTown()->clearFlag(0x11);
    }
    num = days;
    if ((u32)days > 50) {
        num = 50;
    }
    fgMngProc_seedRandom(now, seed + 0x222E);
    updateFlowers(fd);
    fgMngProc_seedRandom(now, seed + 0x2223);
    tryPutLily(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x222F);
    crossBreedFlowers(fd);
    fgMngProc_seedRandom(now, seed + 0x2230);
    plantRandomFlower(fd, w, h);
    fgMngProc_seedRandom(now, seed + 0x2231);
    if (num > 1) {
        for (int i = 1; i < num; i++) {
            updateFlowersLaterDay(fd, i);
            plantRandomFlower(fd, w, h);
        }
    }
    num = days;
    if ((u32)days > 30) {
        num = 30;
    }
    fgMngProc_seedRandom(now, seed + 0x222D);
    plantDandelions(fd, *last, num, w, h);
    fgMngState_clearUnitFlags(&sFgMngState);
    fd->clearWatered();
    sFgMngUnitMask = 0;
}

// 80094034: removes the dead saplings (fg 0x01..0x04, fg_treeA/B/C_x) from the usable blocks.
void dFgMngProc_c::removeDeadSaplings(dFdBase_c *fd) {
    int z0;
    int x0;
    int z;
    dItem::Item *item;
    int blockX;
    int blockZ;
    int unitX;
    int unitZ;
    for (blockZ = 1, z0 = UT_Z_NUM; blockZ < FG_BLOCK_Z_NUM + 1; blockZ++, z0 += UT_Z_NUM) {
        for (blockX = 1, x0 = UT_X_NUM; blockX < FG_BLOCK_X_NUM + 1; blockX++, x0 += UT_X_NUM) {
            item = fd->getItem(blockX, blockZ, 0, 0, 0);
            for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
                z = z0 + unitZ;
                for (unitX = 0; unitX < UT_X_NUM; unitX++, item++) {
                    if (inRange(item, dItem::FG_DEAD_SAPLING, dItem::FG_DEAD_PALM_SAPLING)) {
                        dFgMngProc_c::setUnitItem(fd, x0 + unitX, z, dItem::ITEM_ID_NONE, 0);
                    }
                }
            }
        }
    }
}

// 80094120
BOOL dFgMngProc_c::isLastTime(int hour, int min) {
    if (hour == (mLastTime.mRaw >> 8) && min == (mLastTime.mRaw & 0xFF)) {
        return TRUE;
    }
    return FALSE;
}

// 8009414C
void dFgMngProc_c::setLastTime(int hour, int min) {
    mLastTime.mRaw = (hour << 8) | (min & 0xFF);
}

// 8009415C: picks a random shell (item index 0x3B + i) by the nine weights of sShellHostIO;
// falls back to index 0x43.
u16 dFgMngProc_c::getRandomShell() {
    f32 r = fgMngProc_rndF(sShellHostIO.mWeightTotal);
    for (int i = 0; i < FG_SHELL_KIND_NUM; i++) {
        r -= sShellHostIO.mWeight[i];
        if (r < 0.0f) {
            return dItem::Item(dItem::ITEM_IDX_PEARL_OYSTER, i, FALSE).mId;
        }
    }
    return dItem::Item(dItem::ITEM_IDX_SAND_DOLLAR).mId;
}

// 800941E4: puts a random shell on a free beach unit (field block row 5) of block column blockX
// if it has fewer than 3 shells; avoidPlayer skips units within 8 units in x of the local
// player.
void dFgMngProc_c::putShellOnBeach(dFdBase_c *fd, int blockX, BOOL avoidPlayer) {
    int pos[UT_TOTAL_NUM][2];
    u16 item;
    int (*p)[2];
    int x0;
    int num;
    int bx;
    int unitX;
    int unitZ;
    int x;
    int z;
    int plUnitX;
    int plBlockZ;
    sFdAssess.assessLiveBlock(fd, blockX, FG_BEACH_BLOCK_Z);
    if (sFdAssess.mBlocks[blockX][FG_BEACH_BLOCK_Z].mShellNum < 3) {
        item = getRandomShell();
        bx = blockX + 1;
        num = 0;
        if (avoidPlayer) {
            dPlayerActor_c *player = fn_800FBC7C(4);
            if (player == NULL) {
                return;
            }
            f32 pz = player->mPos.z;
            f32 px = player->mPos.x;
            plBlockZ = (int)pz >> 9;
            plUnitX = (int)px >> 5;
        } else {
            plBlockZ = 4;
            plUnitX = -0xFF;
        }
        p = pos;
        x0 = bx * UT_X_NUM;
        for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
            for (unitX = 0; unitX < UT_X_NUM; unitX++) {
                dItem::Item *it = fd->getItem(bx, BEACH_BLOCK_Z, unitX, unitZ, 0);
                if (it != NULL && it->mId == dItem::ITEM_ID_NONE) {
                    x = x0 + unitX;
                    z = unitZ + BEACH_BLOCK_Z * UT_Z_NUM;
                    if (fd->isBeachGround(x, z)) {
                        if (plBlockZ < BEACH_BLOCK_Z || plUnitX < x - 8 || plUnitX > x + 8) {
                            (*p)[0] = x;
                            num++;
                            (*p)[1] = z;
                            p++;
                        }
                    }
                }
            }
        }
        if (num > 0) {
            int i = (int)fgMngProc_rndF(num);
            fgMngProc_setUnitFgSync(pos[i][0], pos[i][1], item);
        }
    }
}

// 800943A0: not online: once per minute ending in 3, puts a shell on a random beach block away
// from the player.
void dFgMngProc_c::trySpawnShellOffline() {
    dTime_c now = *dTime_c::getCurrent();
    if (!isLastTime(now.hour, now.min)) {
        switch (now.min % 10) {
        case 3:
            dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
            putShellOnBeach(fd, (int)cM::rndF(FG_BLOCK_X_NUM), TRUE);
            setLastTime(now.hour, now.min);
            break;
        }
    }
}

// 800944B8: online (if fn_800DCF90): once per minute ending in 3, puts a shell on a random beach
// block unless some player is on the beach.
void dFgMngProc_c::trySpawnShellOnline() {
    if (fn_800DCF90()) {
        dTime_c now = *dTime_c::getCurrent();
        if (!isLastTime(now.hour, now.min)) {
            switch (now.min % 10) {
            case 3: {
                BOOL onBeach = FALSE;
                dFdBase_c *fd;
                for (u32 i = 0; i < 4; i++) {
                    u8 c;
                    u8 scene;
                    u8 b;
                    u32 info;
                    if (((dFgMngPlInfoFunc)fn_80101DC4)(&info, &scene, &b, i) && isSceneAttr(scene, SCENE_ATTR_TOWN)) {
                        f32 pos[2];
                        u8 posInfo[8];
                        if (((dFgMngPlPosFunc)fn_80101DC0)(posInfo, pos, &c, i) && ((int)pos[0] >> 9) >= 5) {
                            onBeach = TRUE;
                            break;
                        }
                    }
                }
                if (!onBeach) {
                    fd = fn_80190C44(FD_ID_TOWN);
                    putShellOnBeach(fd, (int)cM::rndF(FG_BLOCK_X_NUM), FALSE);
                    setLastTime(now.hour, now.min);
                }
                break;
            }
            }
        }
    }
}

// 80094658: runs the online or local shell spawn depending on fn_800DCEDC (online session
// active).
void dFgMngProc_c::updateShellSpawn() {
    if (fn_800DCEDC()) {
        trySpawnShellOnline();
    } else {
        trySpawnShellOffline();
    }
}

// 800946A0: tops each beach block up to two shells (after assessLive) and records the current
// time as the last spawn time.
void dFgMngProc_c::fillBeachShells(dFdBase_c *fd) {
    dFdAsBlock_c *row;
    int x;
    int w;
    w = fd->mBlockW - 2;
    row = sFdAssess.mBlocks[0];
    for (x = 0; x < w; row += FG_BLOCK_Z_NUM, x++) {
        for (int n = 2 - row[FG_BEACH_BLOCK_Z].mShellNum; n > 0; n--) {
            putInBlock(fd, x, FG_BEACH_BLOCK_Z, getRandomShell(), dFgMngProc_c::isSandUnit, 0);
        }
    }
    dTime_c now = *dTime_c::getCurrent();
    setLastTime(now.hour, now.min);
}

// 800947A4: sets the bit of every flower / red turnip unit (isFlowerOrRedKabu) in the town's
// per-block watered-unit grids (dSaveMainField_c::mWater).
void dFgMngProc_c::markFlowerUnits(dFdBase_c *fd, int w, int h) {
    int unitX;
    int unitZ;
    int blockX;
    int blockZ;
    u16 *flags;
    for (blockZ = 1; blockZ < h + 1; blockZ++) {
        for (blockX = 1; blockX < w + 1; blockX++) {
            flags = (u16 *)dSaveData_c::getTown()->mMainField.getBlockWater(blockX, blockZ);
            if (flags != NULL) {
                for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
                    for (unitX = 0; unitX < UT_X_NUM; unitX++) {
                        if (isFlowerOrRedKabu(fd->getItem(blockX, blockZ, unitX, unitZ, 0))) {
                            fn_80081238(flags, unitX, unitZ);
                        }
                    }
                }
            }
        }
    }
}

// 8009488C: a red turnip plant (0x95..0x9D), a flower, or a wilted flower other than the
// rafflesia (0xDD).
BOOL dFgMngProc_c::isFlowerOrRedKabu(dItem::Item *item) {
    if (item == NULL) {
        return FALSE;
    }
    if (inRange(item, dItem::FG_RED_TURNIP_WILTED, dItem::FG_RED_TURNIP_WILTED_WATERED) || item->isFlower() || (item->isWiltedFlower() && !inRange(item, dItem::FG_WILTED_RAFFLESIA, dItem::FG_WILTED_RAFFLESIA))) {
        return TRUE;
    }
    return FALSE;
}

// 80094934: kills the saplings (tree stage 0) planted where their kind can't grow (per-kind
// checks below).
void dFgMngProc_c::killBadSaplings(dFdBase_c *fd, int *size) {
    int x;
    int z;
    dItem::Item *item;
    for (z = 0; z < size[1]; z++) {
        for (x = 0; x < size[0]; x++) {
            item = fd->getItem(x, z, 0);
            if (item != NULL) {
                dItem::FgInfo *info = item->getFgInfo();
                if (info != NULL && info->mTreeStage == 0) {
                    if (inRange(item, dItem::FG_CEDAR_SAPLING, dItem::FG_CEDAR_LIGHTS)) {
                        checkCedarSapling(fd, item, size, x, z);
                    } else if (inRange(item, dItem::FG_PALM_SAPLING, dItem::FG_PALM_FRUIT)) {
                        checkPalmSapling(fd, item, size, x, z);
                    } else {
                        checkSapling(fd, item, size, x, z);
                    }
                }
            }
        }
    }
}

// 80094A74: a palm sapling (0x41..0x48) dies north of the beach row (unit z < 5 * 16), else the
// common check.
void dFgMngProc_c::checkPalmSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z) {
    if (z < BEACH_BLOCK_Z * UT_Z_NUM) {
        killTree(fd, item, x, z);
        return;
    }
    checkSapling(fd, item, size, x, z);
}

// 80094A94: a cedar sapling (0x4E..0x56) dies south of field row 3 (unit z >= 3 * 16), else the
// common check.
void dFgMngProc_c::checkCedarSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z) {
    if (z >= (FG_CEDAR_BLOCK_Z_NUM + 1) * UT_Z_NUM) {
        killTree(fd, item, x, z);
        return;
    }
    checkSapling(fd, item, size, x, z);
}

// 80094AB4: a sapling dies on ground other than type 2 (getPlantType) or with an obstacle in
// the 8 units around it.
void dFgMngProc_c::checkSapling(dFdBase_c *fd, dItem::Item *item, int *size, int x, int z) {
    if (fd->getPlantType(x, z) != 2) {
        killTree(fd, item, x, z);
    } else if (!isSaplingSpaceFree(fd, size, x, z)) {
        killTree(fd, item, x, z);
    }
}

// 80094B5C: replaces a tree with its dead object (FgInfo +0x04) and decrements the assessed tree
// count of its block quarter.
void killTree(dFdBase_c *fd, dItem::Item *item, int x, int z) {
    int deadId = *(s16 *)&item->getFgInfo()->_04;
    dItem::Item dead((u16)deadId);
    fd->setItem(&dead, x, z, 0);
    int blockX = x >> 4;
    int blockZ = z >> 4;
    sFdAssess.getBlock(blockX - 1, blockZ - 1)->mTreeNum[(x - (blockX << 4)) >> 3][(z - (blockZ << 4)) >> 3]--;
}

// 80094C1C: kills the saplings in the 8 units around pos.
BOOL dFgMngProc_c::killSaplingsAround(dFdBase_c *fd, int *size, int *pos) {
    return forEachAroundUnit(fd, size, pos[0], pos[1], killSaplingAt);
}

// 80094C38: no tree obstacle (isTreeObstacleAt) in the 8 units around (x, z).
BOOL dFgMngProc_c::isSaplingSpaceFree(dFdBase_c *fd, int *size, int x, int z) {
    return forEachAroundUnit(fd, size, x, z, isTreeObstacleAt) == 0;
}

// 8046EFE0
static const int sTreeCheckStart[3][2] = {{0, 2}, {1, 3}, {2, 0}};

// 80094C78: three staggered passes (every 4th unit) killing the saplings next to a sapling.
void dFgMngProc_c::thinOutSaplings(dFdBase_c *fd, int *size) {
    for (int i = 0; i < 3; i++) {
        int pos[2];
        for (pos[1] = 0; pos[1] < size[1]; pos[1]++) {
            for (pos[0] = sTreeCheckStart[i][pos[1] & 1]; pos[0] < size[0]; pos[0] += 4) {
                if (isSapling(fd->getItem(pos[0], pos[1], 0))) {
                    killSaplingsAround(fd, size, pos);
                }
            }
        }
    }
}

// 8046F010: the 7 x 7 units around one (without the centre).
static const u8 sAround7x7[0x30] = {
    FG_UNIT_OFS(-3, -3), FG_UNIT_OFS(-2, -3), FG_UNIT_OFS(-1, -3), FG_UNIT_OFS(0, -3),
    FG_UNIT_OFS(1, -3), FG_UNIT_OFS(2, -3), FG_UNIT_OFS(3, -3), FG_UNIT_OFS(-3, -2),
    FG_UNIT_OFS(-2, -2), FG_UNIT_OFS(-1, -2), FG_UNIT_OFS(0, -2), FG_UNIT_OFS(1, -2),
    FG_UNIT_OFS(2, -2), FG_UNIT_OFS(3, -2), FG_UNIT_OFS(-3, -1), FG_UNIT_OFS(-2, -1),
    FG_UNIT_OFS(-1, -1), FG_UNIT_OFS(0, -1), FG_UNIT_OFS(1, -1), FG_UNIT_OFS(2, -1),
    FG_UNIT_OFS(3, -1), FG_UNIT_OFS(-3, 0), FG_UNIT_OFS(-2, 0), FG_UNIT_OFS(-1, 0),
    FG_UNIT_OFS(1, 0), FG_UNIT_OFS(2, 0), FG_UNIT_OFS(3, 0), FG_UNIT_OFS(-3, 1),
    FG_UNIT_OFS(-2, 1), FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(0, 1), FG_UNIT_OFS(1, 1),
    FG_UNIT_OFS(2, 1), FG_UNIT_OFS(3, 1), FG_UNIT_OFS(-3, 2), FG_UNIT_OFS(-2, 2),
    FG_UNIT_OFS(-1, 2), FG_UNIT_OFS(0, 2), FG_UNIT_OFS(1, 2), FG_UNIT_OFS(2, 2),
    FG_UNIT_OFS(3, 2), FG_UNIT_OFS(-3, 3), FG_UNIT_OFS(-2, 3), FG_UNIT_OFS(-1, 3),
    FG_UNIT_OFS(0, 3), FG_UNIT_OFS(1, 3), FG_UNIT_OFS(2, 3), FG_UNIT_OFS(3, 3),
};

// 80094D50: kills a sapling with 8 or more trees (any stage) in the 7 x 7 units around it.
void dFgMngProc_c::killCrowdedSaplings(dFdBase_c *fd, int *size) {
    dItem::Item *item;
    const u8 *around;
    int x;
    int z;
    int num;
    int i;
    for (z = 0; z < size[1]; z++) {
        for (x = 0; x < size[0]; x++) {
            item = fd->getItem(x, z, 0);
            if (isSapling(item)) {
                around = sAround7x7;
                num = 0;
                for (i = 0; i < 0x30; i++, around++) {
                    int ax = x - ((*around >> 4) - 8);
                    int az = z - ((*around & 0xF) - 8);
                    if (ax >= 0 && ax < size[0] && az >= 0 && az < size[1]) {
                        dItem::Item *other = fd->getItem(ax, az, 0);
                        if (other != NULL) {
                            dItem::FgInfo *info = other->getFgInfo();
                            if (info != NULL && info->mTreeStage >= 0) {
                                num++;
                            }
                        }
                    }
                }
                if (num >= 8) {
                    killTree(fd, item, x, z);
                }
            }
        }
    }
}

// 80094E84: one day of tree processing: kill bad / adjacent / crowded saplings, then grow every
// tree.
void dFgMngProc_c::updateTrees(dFdBase_c *fd, int w, int h) {
    int size[2];
    size[0] = fd->mBlockW << 4;
    size[1] = fd->mBlockH << 4;
    killBadSaplings(fd, size);
    thinOutSaplings(fd, size);
    killCrowdedSaplings(fd, size);
    growTrees(fd, size);
}

// 80094F08: grows every tree (tree stage >= 0) of the field one step.
void dFgMngProc_c::growTrees(dFdBase_c *fd, int *size) {
    dItem::Item *item;
    int x;
    int z;
    for (z = 0; z < size[1]; z++) {
        for (x = 0; x < size[0]; x++) {
            item = fd->getItem(x, z, 0);
            if (item != NULL) {
                dItem::FgInfo *info = item->getFgInfo();
                if (info != NULL && info->mTreeStage >= 0) {
                    growTree(fd, item, x, z);
                }
            }
        }
    }
}

// 80094FB8: grows a tree one step: fruit trees / palms go from stage 3 to their fruit-bearing
// variant (+4) and regrow fruit (+1) when it has none; other trees +1 below stage 4.
void growTree(dFdBase_c *fd, dItem::Item *item, int x, int z) {
    u16 id = item->mId;
    dItem::FgInfo *info = item->getFgInfo();
    if (inRange(item, dItem::FG_PEACH_TREE_SAPLING, dItem::FG_PEACH_TREE_FRUIT) || inRange(item, dItem::FG_APPLE_TREE_SAPLING, dItem::FG_APPLE_TREE_FRUIT) || inRange(item, dItem::FG_ORANGE_TREE_SAPLING, dItem::FG_ORANGE_TREE_FRUIT) ||
        inRange(item, dItem::FG_PEAR_TREE_SAPLING, dItem::FG_PEAR_TREE_FRUIT) || inRange(item, dItem::FG_CHERRY_TREE_SAPLING, dItem::FG_CHERRY_TREE_FRUIT) || inRange(item, dItem::FG_PALM_SAPLING, dItem::FG_PALM_FRUIT)) {
        if (info->mTreeStage == 3) {
            id += 4;
        } else {
            int drop = *(s16 *)&info->mDropItem;
            if ((u16)drop == dItem::ITEM_ID_NONE) {
                id += 1;
            }
        }
    } else if (info->mTreeStage < 4) {
        id += 1;
    }
    dItem::Item next(id);
    fd->setItem(&next, x, z, 0);
}

// 80750660: the 8 units around one, ((dx + 8) << 4) | (dz + 8).

// 8009512C: calls func on each in-field unit of the 8 around (x, z) until it returns nonzero;
// returns that result.
BOOL forEachAroundUnit(dFdBase_c *fd, int *size, int x, int z, dFgMngAroundFunc func) {
    BOOL ret = FALSE;
    const u8 *around = sAround8;
    for (int i = 0; i < 8; i++, around++) {
        int pos[2];
        pos[0] = x - ((*around >> 4) - 8);
        pos[1] = z - ((*around & 0xF) - 8);
        if (pos[0] >= 0 && pos[0] < size[0] && pos[1] >= 0 && pos[1] < size[1] && !(pos[0] == x && pos[1] == z)) {
            ret = func(fd, pos, x, z, size);
            if (ret) {
                break;
            }
        }
    }
    return ret;
}

// 80095208: forEachAroundUnit callback: pos is blocked (isBlockedUnit), a stone (0x5B..0x73) or
// a stump (0x05..0x10).
BOOL isTreeObstacleAt(dFdBase_c *fd, const int *pos, int x, int z, int *size) {
    BOOL ret = FALSE;
    dItem::Item *item = fd->getItem(pos[0], pos[1], 0);
    if (item != NULL) {
        if (isBlockedUnit(fd, pos[0], pos[1])) {
            ret = TRUE;
        } else if (inRange(item, dItem::FG_STONE_A, dItem::FG_STONE_E) || inRange(item, dItem::FG_MONEY_ROCK_P0_A, dItem::FG_MONEY_ROCK_P3_E) || inRange(item, dItem::FG_STUMP_S1, dItem::FG_PALM_STUMP)) {
            ret = TRUE;
        }
    }
    return ret;
}

// 800952F8: forEachAroundUnit callback: kills the sapling at pos, if any; never stops the walk.
BOOL killSaplingAt(dFdBase_c *fd, const int *pos, int x, int z, int *size) {
    dItem::Item *item = fd->getItem(pos[0], pos[1], 0);
    if (isSapling(item)) {
        killTree(fd, item, pos[0], pos[1]);
    }
    return FALSE;
}

// 8009536C: a structure collision unit (dFdUnitAttr_c::STR_COL), a build site (dSaveBuildingList_c::isBuildSite), no item
// slot, or a grown tree (stage > 0). Also used by setupUnitMask and 80185F60.
BOOL isBlockedUnit(dFdBase_c *fd, int x, int z) {
    if (fd->_24 != NULL && (fd->_24->getAttr(x, z) & dFdUnitAttr_c::STR_COL)) {
        return TRUE;
    }
    if (dSaveBuildingList_c::get()->isBuildSite(x, z)) {
        return TRUE;
    }
    dItem::Item *item = fd->getItem(x, z, 0);
    if (item != NULL) {
        dItem::FgInfo *info = item->getFgInfo();
        if (info != NULL && info->mTreeStage > 0) {
            return TRUE;
        }
        return FALSE;
    }
    return TRUE;
}

// 80095438: the unit's bg attribute is BG_ATTR_CLIFF_N, CLIFF_NW or CLIFF_NE (a subset of the water-edge attributes
// isLilyUnit checks). Also used by setupUnitMask and 80185F78.
BOOL isPondEdgeUnit(dFdBase_c *fd, int x, int z) {
    int attr = fd->getBgAttr(x, z);
    if (attr == BG_ATTR_CLIFF_NW || attr == BG_ATTR_CLIFF_NE || attr == BG_ATTR_CLIFF_N) {
        return TRUE;
    }
    return FALSE;
}

// 8009547C: a tree at stage 0.
BOOL isSapling(dItem::Item *item) {
    if (item == NULL) {
        return FALSE;
    }
    dItem::FgInfo *info = item->getFgInfo();
    if (info != NULL && info->mTreeStage == 0) {
        return TRUE;
    }
    return FALSE;
}

// 800954CC: plants days * 2 weeds spread evenly (rest at random) over the 5 x 5 blocks,
// redistributing what full blocks can't take.
void dFgMngProc_c::spreadWeeds(dFdBase_c *fd, int days) {
    int perBlock[FG_BLOCK_TOTAL_NUM];
    int freeBlocks[FG_BLOCK_TOTAL_NUM];
    int pos[UT_TOTAL_NUM][2];
    u32 fullBlocks = 0;
    int left = days * 2;
    do {
        int freeNum = 0;
        int i;
        int *p = freeBlocks;
        for (i = 0; i < FG_BLOCK_TOTAL_NUM; i++) {
            perBlock[i] = 0;
            if (!(fullBlocks & (1 << i))) {
                *p++ = i;
                freeNum++;
            }
        }
        if (freeNum == 0) {
            break;
        }
        int per = left / freeNum;
        left -= per * freeNum;
        for (i = 0; i < freeNum; i++) {
            perBlock[freeBlocks[i]] = per;
        }
        while (left != 0) {
            int k = (int)fgMngProc_rndF(freeNum);
            freeNum--;
            perBlock[freeBlocks[k]]++;
            for (i = k; i < freeNum; i++) {
                freeBlocks[i] = freeBlocks[i + 1];
            }
            left--;
        }
        int *cnt = perBlock;
        int idx = 0;
        left = 0;
        for (int blockZ = 0; blockZ < FG_BLOCK_Z_NUM; blockZ++) {
            for (int blockX = 0; blockX < FG_BLOCK_X_NUM; blockX++, idx++, cnt++) {
                if (*cnt != 0) {
                    int got = collectUnits((dFdAsPos_c *)pos, fd, blockX, blockZ, dFgMngProc_c::isGrassUnit);
                    int rest = *cnt - plantGrass(fd, got, (dFdAsPos_c *)pos, *cnt);
                    left += rest;
                    if (rest > 0) {
                        fullBlocks |= 1 << idx;
                    }
                }
            }
        }
    } while (left > 0);
}

// 80095920: removes all weeds (0x57..0x5A) from the field.
void dFgMngProc_c::removeWeeds(dFdBase_c *fd) {
    int x;
    int z;
    int w;
    int h;
    w = fd->mBlockW << 4;
    h = fd->mBlockH << 4;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && inRange(item, dItem::FG_WEED_A, dItem::FG_WEED_D)) {
                dFgMngProc_c::setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, 0);
            }
        }
    }
}

// 800959E8: for each skipped day outside terms 0..2 / 0x16: 50% to put a clover (0xE0, or 0xE1
// at 1%) in a random block.
void dFgMngProc_c::plantClovers(dFdBase_c *fd, dTime_c time, int days, int w, int h) {
    time.add(-days, 0, 0, 0);
    for (int i = 0; i < days; i++) {
        int term = time.getTerm();
        switch (term) {
        case TERM_JAN_01_FEB_03:
        case TERM_FEB_04_FEB_17:
        case TERM_FEB_18_FEB_24:
        case TERM_DEC_11_DEC_31: {
            dTime_c prev;
            prev = time;
            dTime_c::setTermDate(&term, &time);
            i += dTime_c::diffDays(&time, &prev, FALSE);
            break;
        }
        default:
            if (fgMngProc_rndF(100.0f) < 50.0f) {
                int x = (int)fgMngProc_rndF(w);
                int z = (int)fgMngProc_rndF(h);
                putInBlock(fd, x, z, fgMngProc_rndF(100.0f) < 1.0f ? dItem::FG_LUCKY_CLOVER : dItem::FG_CLOVER, dFgMngProc_c::isGrassUnit, 0);
            }
            break;
        }
        time.add(1, 0, 0, 0);
    }
}

// 80095BE4: for each skipped day outside terms 0..2 / 0x16: 20% to put a dandelion (0xDE) in a
// random block.
void dFgMngProc_c::plantDandelions(dFdBase_c *fd, dTime_c time, int days, int w, int h) {
    int x;
    int i;
    int z;
    time.add(-days, 0, 0, 0);
    for (i = 0; i < days; i++) {
        int term = time.getTerm();
        switch (term) {
        case TERM_JAN_01_FEB_03:
        case TERM_FEB_04_FEB_17:
        case TERM_FEB_18_FEB_24:
        case TERM_DEC_11_DEC_31:
            dTime_c::setTermDate(&term, &time);
            break;
        default:
            if (fgMngProc_rndF(100.0f) < 20.0f) {
                x = (int)fgMngProc_rndF(w);
                z = (int)fgMngProc_rndF(h);
                putInBlock(fd, x, z, dItem::FG_DANDELION, dFgMngProc_c::isGrassUnit, 0);
            }
            break;
        }
        time.add(1, 0, 0, 0);
    }
}

// 80095D4C: first day: wilted gold rose recovers, watered (flag B) wilted flowers recover else
// vanish, unwatered flowers may wilt, dandelions puff (30%) and puffs vanish (50%); then wilts
// the rafflesia unless the town is rated bad.
void dFgMngProc_c::updateFlowers(dFdBase_c *fd) {
    u16 id;
    int w;
    int h;
    dItem::Item *item;
    int x;
    u16 next;
    int z;
    w = fd->mBlockW << 4;
    h = fd->mBlockH << 4;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            item = fd->getItem(x, z, 0);
            if (item != NULL) {
                id = item->mId;
                next = 0xFFFF;
                if (id == dItem::FG_WILTED_ROSE_GOLD) {
                    next = dItem::FG_ROSE_GOLD;
                } else if (item->isWiltedFlower()) {
                    if (fd->isWatered(x, z)) {
                        next = id - 0x20;
                    } else {
                        next = dItem::ITEM_ID_NONE;
                    }
                } else if (item->isFlower()) {
                    if (!fd->isWatered(x, z) && !(*(u16 *)((u8 *)dSaveData_c::getTown() + 0x5EC74) & 0x80)) {
                        u32 chance = getWiltChance(id);
                        if ((u32)(int)fgMngProc_rndF(100.0f) < chance) {
                            next = id + 0x20;
                        }
                    }
                } else if (id == dItem::FG_DANDELION) {
                    if ((int)fgMngProc_rndF(100.0f) < 30) {
                        next = dItem::FG_DANDELION_PUFF;
                    }
                } else if (id == dItem::FG_DANDELION_PUFF) {
                    if ((int)fgMngProc_rndF(100.0f) < 50) {
                        next = dItem::ITEM_ID_NONE;
                    }
                }
                if (next != 0xFFFF) {
                    dFgMngProc_c::setUnitItem(fd, x, z, next, 0);
                }
            }
        }
    }
    if (sFdAssess.mRank != dFdAssess_c::TOWN_RANK_BAD) {
        wiltRafflesia(fd);
    }
}

// 80095F30: each further skipped day: flowers may wilt, dandelions puff; every 7th day wilted
// flowers and (50%) puffs vanish.
void dFgMngProc_c::updateFlowersLaterDay(dFdBase_c *fd, int day) {
    int w;
    int h;
    int z;
    dItem::Item *item;
    u16 next;
    BOOL weekly;
    int x;
    u16 id;
    weekly = day % TIME_DAYS_PER_WEEK == 0;
    w = fd->mBlockW << 4;
    h = fd->mBlockH << 4;
    for (z = 0; z < h; z++) {
        for (x = 0; x < w; x++) {
            item = fd->getItem(x, z, 0);
            if (item != NULL) {
                id = item->mId;
                next = 0xFFFF;
                if (item->isWiltedFlower()) {
                    if (weekly) {
                        next = dItem::ITEM_ID_NONE;
                    }
                } else if (id == dItem::FG_DANDELION_PUFF) {
                    if (weekly && (int)fgMngProc_rndF(100.0f) < 50) {
                        next = dItem::ITEM_ID_NONE;
                    }
                } else if (item->isFlower()) {
                    u32 chance = getWiltChance(id);
                    if ((u32)(int)fgMngProc_rndF(100.0f) < chance) {
                        next = id + 0x20;
                    }
                } else if (id == dItem::FG_DANDELION) {
                    if ((int)fgMngProc_rndF(100.0f) < 30) {
                        next = dItem::FG_DANDELION_PUFF;
                    }
                }
                if (next != 0xFFFF) {
                    dFgMngProc_c::setUnitItem(fd, x, z, next, 0);
                }
            }
        }
    }
}

// 8046F050: the wilting chance in percent of the flowers 0x9E..0xBB.
static const u8 sFlowerWiltChance[0x20] = {
    0x05, 0x05, 0x05, 0x0F, 0x0F, 0x1E, 0x05, 0x05, 0x05, 0x0F, 0x0F, 0x1E, 0x05, 0x05, 0x05, 0x0F,
    0x0F, 0x1E, 0x05, 0x05, 0x05, 0x0F, 0x19, 0x19, 0x1E, 0x28, 0x00, 0x0A, 0x0A, 0x14, 0x1E, 0x00,
};

// 800960D4: daily wilt chance in percent of a flower (0x9E..0xBD); 0 for the gold rose.
u32 dFgMngProc_c::getWiltChance(u16 id) {
    if (id == dItem::FG_ROSE_GOLD) {
        return 0;
    }
    return sFlowerWiltChance[id - dItem::FG_TULIP_RED];
}

// 8046EFF8: the flowers planted at random.
static const u16 sRandomFlowers[12] = {dItem::FG_TULIP_RED, dItem::FG_TULIP_WHITE, dItem::FG_TULIP_YELLOW, dItem::FG_PANSY_WHITE, dItem::FG_PANSY_YELLOW, dItem::FG_PANSY_RED, dItem::FG_COSMOS_WHITE, dItem::FG_COSMOS_RED, dItem::FG_COSMOS_YELLOW, dItem::FG_ROSE_RED, dItem::FG_ROSE_WHITE, dItem::FG_ROSE_YELLOW};

// 800960F8: puts one random base-color tulip / pansy / cosmos / rose in a random block.
void dFgMngProc_c::plantRandomFlower(dFdBase_c *fd, int w, int h) {
    // A loop of one (the count is probably a constant; written as a loop for the register use).
    for (int i = 0; i < 1; i++) {
        int x = (int)fgMngProc_rndF(w);
        int z = (int)fgMngProc_rndF(h);
        putInBlock(fd, x, z, sRandomFlowers[(int)fgMngProc_rndF(12.0f)], dFgMngProc_c::isFlowerUnit, 0);
    }
}

// 800961D0
BOOL CBUtSearch_c::check(int x, int z) {
    int unitX = (mBlockX << 4) + x;
    int unitZ = (mBlockZ << 4) + z;
    dItem::Item *item = mFd->getItem(unitX, unitZ, 0);
    if (item != NULL && inRange(item, dItem::FG_TULIP_RED, dItem::FG_CARNATION_WHITE) && mFd->isWatered(unitX, unitZ)) {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL pickXZ(dSearchCandXZCore_c *cand, int *x, int *z) {
    int idx = cand->getNth((int)fgMngProc_rndF(cand->mCount));
    if (idx < 0) {
        return FALSE;
    }
    *z = idx / cand->mWidth;
    *x = idx - *z * cand->mWidth;
    return TRUE;
}

// 80096284: up to 5 times: picks a random block with a watered flower, a random watered flower
// in it, and tries to cross-breed it with its neighbours.
void dFgMngProc_c::crossBreedFlowers(dFdBase_c *fd) {
    int blockX;
    int blockZ;
    u32 i;
    int unitX;
    int unitZ;
    CBBkSearch_c bk;
    CBUtSearch_c ut(fd, 1, 1);
    int size[2];
    size[0] = BLOCK_X_NUM * UT_X_NUM;
    size[1] = BLOCK_Z_NUM * UT_Z_NUM;
    bk.clear();
    bk.search();
    bk.removeBorder();
    for (i = 0; i < 5; i++) {
        if (pickXZ(&bk, &blockX, &blockZ)) {
            ut.clear();
            ut.mBlockX = blockX;
            ut.mBlockZ = blockZ;
            ut.search();
            if (pickXZ(&ut, &unitX, &unitZ)) {
                forEachAroundUnit(fd, size, (blockX << 4) + unitX, (blockZ << 4) + unitZ, tryCrossBreedAt);
            }
            bk.remove(blockX + blockZ * bk.mWidth);
        }
    }
}

// 80096468: forEachAroundUnit callback: if pos holds a watered flower of the same kind as (x,
// z), puts their hybrid around (x, z) or else around pos.
BOOL tryCrossBreedAt(dFdBase_c *fd, const int *pos, int x, int z, int *size) {
    int kind;
    dItem::Item *a;
    BOOL ret;
    dItem::Item *b;
    ret = FALSE;
    a = fd->getItem(pos[0], pos[1], 0);
    if (a != NULL && inRange(a, dItem::FG_TULIP_RED, dItem::FG_CARNATION_WHITE) && fd->isWatered(pos[0], pos[1])) {
        b = fd->getItem(x, z, 0);
        kind = getFlowerKind(b);
        if (kind == getFlowerKind(a)) {
            u16 child = dFgMngProc_c::getCrossBreed(b, a);
            if (child != dItem::ITEM_ID_NONE) {
                int p[2];
                p[0] = x;
                p[1] = z;
                if (dFgMngProc_c::putAround(fd, (const dFdAsPos_c *)size, (const dFdAsPos_c *)p, child)) {
                    ret = TRUE;
                } else {
                    ret = dFgMngProc_c::putAround(fd, (const dFdAsPos_c *)size, (const dFdAsPos_c *)pos, child);
                }
            }
        }
    }
    return ret;
}

// 80096594: flower kind (live or wilted): 0 tulip, 1 pansy, 2 cosmos, 3 rose, 4 carnation, 5
// lily, 6 dandelion, 7 dandelion puff, else 8.
int getFlowerKind(dItem::Item *item) {
    int kind = FLOWER_KIND_NONE;
    if (inRange(item, dItem::FG_TULIP_RED, dItem::FG_TULIP_BLACK) || inRange(item, dItem::FG_WILTED_TULIP_RED, dItem::FG_WILTED_TULIP_BLACK)) {
        return FLOWER_KIND_TULIP;
    }
    if (inRange(item, dItem::FG_PANSY_WHITE, dItem::FG_PANSY_BLUE) || inRange(item, dItem::FG_WILTED_PANSY_WHITE, dItem::FG_WILTED_PANSY_BLUE)) {
        return FLOWER_KIND_PANSY;
    }
    if (inRange(item, dItem::FG_COSMOS_WHITE, dItem::FG_COSMOS_BLACK) || inRange(item, dItem::FG_WILTED_COSMOS_WHITE, dItem::FG_WILTED_COSMOS_BLACK)) {
        return FLOWER_KIND_COSMOS;
    }
    if (inRange(item, dItem::FG_ROSE_RED, dItem::FG_ROSE_GOLD) || inRange(item, dItem::FG_WILTED_ROSE_RED, dItem::FG_WILTED_ROSE_GOLD)) {
        return FLOWER_KIND_ROSE;
    }
    if (inRange(item, dItem::FG_CARNATION_RED, dItem::FG_CARNATION_WHITE) || inRange(item, dItem::FG_WILTED_CARNATION_RED, dItem::FG_WILTED_CARNATION_WHITE)) {
        return FLOWER_KIND_CARNATION;
    }
    if (inRange(item, dItem::FG_JACOBS_LADDER, dItem::FG_JACOBS_LADDER) || inRange(item, dItem::FG_WILTED_JACOBS_LADDER, dItem::FG_WILTED_JACOBS_LADDER)) {
        return FLOWER_KIND_LILY;
    }
    switch (item->mId) {
    case dItem::FG_DANDELION:
        kind = FLOWER_KIND_DANDELION;
        break;
    case dItem::FG_DANDELION_PUFF:
        kind = FLOWER_KIND_DANDELION_PUFF;
        break;
    }
    return kind;
}

// 80096768: color index within the flower's kind (gold rose 8, wilted gold rose 6).
int getFlowerColor(dItem::Item *item) {
    u16 id = item->mId;
    int color = 0;
    if (id == dItem::FG_ROSE_GOLD) {
        color = CBRoseBaseHostIO_c::ROSE_GOLD;
    } else if (id == dItem::FG_WILTED_ROSE_GOLD) {
        color = 6;
    } else if (inRange(item, dItem::FG_TULIP_RED, dItem::FG_CARNATION_WHITE)) {
        switch (getFlowerKind(item)) {
        case FLOWER_KIND_TULIP:
            color = item->mId - dItem::FG_TULIP_RED;
            break;
        case FLOWER_KIND_PANSY:
            color = item->mId - dItem::FG_PANSY_WHITE;
            break;
        case FLOWER_KIND_COSMOS:
            color = item->mId - dItem::FG_COSMOS_WHITE;
            break;
        case FLOWER_KIND_ROSE:
            color = item->mId - dItem::FG_ROSE_RED;
            break;
        case FLOWER_KIND_CARNATION:
            color = item->mId - dItem::FG_CARNATION_RED;
            break;
        }
    } else if (inRange(item, dItem::FG_WILTED_TULIP_RED, dItem::FG_WILTED_CARNATION_WHITE)) {
        switch (getFlowerKind(item)) {
        case FLOWER_KIND_TULIP:
            color = item->mId - dItem::FG_WILTED_TULIP_RED;
            break;
        case FLOWER_KIND_PANSY:
            color = item->mId - dItem::FG_WILTED_PANSY_WHITE;
            break;
        case FLOWER_KIND_COSMOS:
            color = item->mId - dItem::FG_WILTED_COSMOS_WHITE;
            break;
        case FLOWER_KIND_ROSE:
            color = item->mId - dItem::FG_WILTED_ROSE_RED;
            break;
        case FLOWER_KIND_CARNATION:
            color = item->mId - dItem::FG_WILTED_CARNATION_WHITE; // a bug: the wilted carnations start at 0xD9
            break;
        }
    }
    return color;
}

// 8046F070 / 8046F084: mushroom kind rates (normal town / perfect town).
static const f32 sMushRate[FG_MUSHROOM_KIND_NUM] = {10.0f, 30.0f, 30.0f, 30.0f, 0.0f};
static const f32 sMushRatePerfect[FG_MUSHROOM_KIND_NUM] = {10.0f, 25.0f, 30.0f, 25.0f, 10.0f};

static inline int rndInt(int max) {
    return fgMngProc_rndF(max);
}

// 800968E0
u16 dFgMngProc_c::getCrossBreed(const dItem::Item *a, const dItem::Item *b) {
    f32 roll = fgMngProc_rndF(100.0f);
    f32 sum = 0.0f;
    u16 result = dItem::ITEM_ID_NONE;
    int type = getFlowerKind((dItem::Item *)a);
    int colorA = getFlowerColor((dItem::Item *)a);
    int colorB = getFlowerColor((dItem::Item *)b);
    int i;
    switch (type) {
    case FLOWER_KIND_TULIP: {
        dFgCBRate_c *rate = sCBTulip[colorA][colorB];
        for (i = 0; i < CBTulipBaseHostIO_c::TULIP_COLOR_NUM; i++) {
            sum += rate->mRate[i];
            if (sum > roll) {
                break;
            }
        }
        result = dItem::FG_TULIP_RED + i;
        break;
    }
    case FLOWER_KIND_PANSY: {
        dFgCBRate_c *rate = sCBPansy[colorA][colorB];
        for (i = 0; i < CBPansyBaseHostIO_c::PANSY_COLOR_NUM; i++) {
            sum += rate->mRate[i];
            if (sum > roll) {
                break;
            }
        }
        result = dItem::FG_PANSY_WHITE + i;
        break;
    }
    case FLOWER_KIND_COSMOS: {
        dFgCBRate_c *rate = sCBCosmos[colorA][colorB];
        for (i = 0; i < CBCosmosBaseHostIO_c::COSMOS_COLOR_NUM; i++) {
            sum += rate->mRate[i];
            if (sum > roll) {
                break;
            }
        }
        result = dItem::FG_COSMOS_WHITE + i;
        break;
    }
    case FLOWER_KIND_ROSE: {
        dFgCBRate_c *rate = sCBRose[colorA][colorB];
        for (i = 0; i < CBRoseBaseHostIO_c::ROSE_COLOR_NUM; i++) {
            sum += rate->mRate[i];
            if (sum > roll) {
                break;
            }
        }
        result = dItem::FG_ROSE_RED + i;
        break;
    }
    case FLOWER_KIND_CARNATION: {
        dFgCBRate_c *rate = sCBCarnation[colorA][colorB];
        for (i = 0; i < CBCarnationBaseHostIO_c::CARNATION_COLOR_NUM; i++) {
            sum += rate->mRate[i];
            if (sum > roll) {
                break;
            }
        }
        result = dItem::FG_CARNATION_RED + i;
        break;
    }
    }
    return result;
}

// 80096C98
BOOL dFgMngProc_c::putAround(dFdBase_c *fd, const dFdAsPos_c *size, const dFdAsPos_c *center, u16 item) {
    int i;
    int start;
    BOOL done;
    dFdAsPos_c pos;
    done = FALSE;
    start = rndInt(8);
    for (i = 0; i < 8; i++) {
        u8 ofs = sAround8[start];
        pos.mX = center->mX - ((ofs >> 4) - 8);
        pos.mZ = center->mZ - ((ofs & 0xF) - 8);
        if (pos.mX >= 0 && pos.mX < size->mX && pos.mZ >= 0 && pos.mZ < size->mZ && put(fd, &pos, item)) {
            done = TRUE;
            break;
        }
        start = (start + 1) % 8;
    }
    return done;
}

// 80096D8C
BOOL dFgMngProc_c::put(dFdBase_c *fd, const dFdAsPos_c *pos, u16 item) {
    BOOL done = FALSE;
    dItem::Item *cur = fd->getItem(pos->mX, pos->mZ, 0);
    if (cur != NULL && cur->mId == dItem::ITEM_ID_NONE && isFlowerUnit(fd, pos->mX, pos->mZ)) {
        dFgMngProc_c::setUnitItem(fd, pos->mX, pos->mZ, item, FALSE);
        done = TRUE;
    }
    return done;
}

// 80096E34
void dFgMngProc_c::putLily(dFdBase_c *fd, int blockW, int blockH) {
    dFdAsPos_c pos = findLilyBlock(fd, blockW, blockH);
    if (pos.mX != -1) {
        putInBlock(fd, pos.mX, pos.mZ, dItem::FG_JACOBS_LADDER, isLilyUnit, FALSE);
    }
}

// 80096EA0
dFdAsPos_c dFgMngProc_c::findLilyBlock(dFdBase_c *fd, int blockW, int blockH) {
    dFdAsPos_c pos(-1, -1);
    dFdAsPos_c cands[FG_BLOCK_TOTAL_NUM];
    dFdAsPos_c *cand = cands;
    int num = 0;
    for (pos.mZ = 0; pos.mZ < blockH; pos.mZ++) {
        for (pos.mX = 0; pos.mX < blockW; pos.mX++) {
            if (!sFdAssess.mBlocks[pos.mX][pos.mZ].mFlags.mBits.mHasLily) {
                dFdBlock_c *block = fd->getBlock(pos.mX + 1, pos.mZ + 1);
                if (block != NULL && (fn_80081324(block->mType) & 0x0FE00000)) {
                    num++;
                    *cand++ = pos;
                }
            }
        }
    }
    if (num == 0) {
        return pos;
    }
    return cands[rndInt(num)];
}

// 80096FE8
void dFgMngProc_c::tryPutLily(dFdBase_c *fd, int blockW, int blockH) {
    if (sFdAssess.mRank == dFdAssess_c::TOWN_RANK_PERFECT) {
        int lily = sFdAssess.mLilyBlockNum;
        if (fgMngProc_rndF(100.0f) < 100.0f / ((lily + 1) * 2)) {
            putLily(fd, blockW, blockH);
        }
    }
}

// 8009708C
void dFgMngProc_c::putRafflesia(dFdBase_c *fd, int blockW, int blockH) {
    int x = rndInt(blockW);
    int z = rndInt(blockH);
    putInBlock(fd, x, z, dItem::FG_RAFFLESIA, isRafflesiaUnit, FALSE);
}

// 80097148
static inline dItem::Item *getItemAt(dFdBase_c *fd, dFdAsPos_c block, dFdAsPos_c unit) {
    return fd->getItem(block.mX + 1, block.mZ + 1, unit.mX, unit.mZ, 0);
}
static inline void setItemAt(dFdBase_c *fd, dFdAsPos_c block, dFdAsPos_c unit, u16 item) {
    dFgMngProc_c::setUnitItem(fd, block.mX + 1, block.mZ + 1, unit.mX, unit.mZ, item, FALSE);
}
void dFgMngProc_c::wiltRafflesia(dFdBase_c *fd) {
    if (getItemAt(fd, sFdAssess.getRafflesiaBlock(), sFdAssess.getRafflesiaUnit()) != NULL) {
        setItemAt(fd, sFdAssess.getRafflesiaBlock(), sFdAssess.getRafflesiaUnit(), dItem::FG_WILTED_RAFFLESIA);
    }
}

// 80097234
void dFgMngProc_c::tryPutRafflesia(dFdBase_c *fd, int blockW, int blockH) {
    if (sFdAssess.mRank == dFdAssess_c::TOWN_RANK_BAD) {
        if (sFdAssess.getRafflesiaBlock0().mX < 0 && sFdAssess.getRafflesiaBlock0().mZ < 0) {
            putRafflesia(fd, blockW, blockH);
        }
    }
}

// 800972A8
void dFgMngProc_c::removeTreesInWater(dFdBase_c *fd, int blockW, int blockH) {
    int bx;
    int bz;
    int ux;
    int uz;
    for (bz = 1; bz < blockH + 1; bz++) {
        for (bx = 1; bx < blockW + 1; bx++) {
            for (uz = 0; uz < UT_Z_NUM; uz++) {
                for (ux = 0; ux < UT_X_NUM; ux++) {
                    if (fd->getPlantType(bx, bz, ux, uz) != 2) {
                        dItem::Item *item = fd->getItem(bx, bz, ux, uz, 0);
                        if (item != NULL) {
                            dItem::FgInfo *info = item->getFgInfo();
                            if (info != NULL && info->mTreeStage > 0) {
                                dItem::Item none;
                                fd->setItem(&none, bx, bz, ux, uz, 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

// 800973B0
void dFgMngProc_c::procEvents(dFdBase_c *fd, const dTime_c *time) {
    if (dEvent::isEventOn(EVENT_BUNNY_DAY, time)) {
        removeEggs(fd);
        addEvent(EVENT_BUNNY_DAY);
    } else if (dEvent::isEventOn(EVENT_COUNTDOWN, time)) {
        fn_80169AC4();
        addEvent(EVENT_COUNTDOWN);
    } else if (dEvent::isEventOn(EVENT_HARVEST_FESTIVAL, time)) {
        fn_80169B64();
        addEvent(EVENT_HARVEST_FESTIVAL);
    } else if (dEvent::isEventOn(EVENT_FESTIVALE, time)) {
        fn_80169AEC();
        addEvent(EVENT_FESTIVALE);
    }
    if (dEvent::isEventOn(EVENT_FISHING_TOURNEY, time)) {
        fn_80169B14();
        addEvent(EVENT_FISHING_TOURNEY);
    } else if (dEvent::isEventOn(EVENT_BUG_OFF, time)) {
        fn_80169B3C();
        addEvent(EVENT_BUG_OFF);
    }
    if (dEvent::getTodayVisitor() == VISITOR_WISP) {
        dTime_c *now = dTime_c::getCurrent();
        u8 *date = dSaveData_c::getTown()->_0683C4;
        u8 month = now->month;
        u8 day = now->mday;
        BOOL same = FALSE;
        if (date[0] == month && date[1] == day) {
            same = TRUE;
        }
        if (same) {
            addEvent(EVENT_LAMP_SAME_DAY);
        } else if (buryLamp(fd)) {
            addEvent(EVENT_LAMP_BURIED);
        }
    }
}

// The town's event object (dSaveData_c+0x68372, ctor 8014D0BC; fn_8014D89C adds an event).
struct dFgSaveEvents_c {
    /* 0x00 */ u8 _00[0x44];
    /* 0x44 */ u8 mStamp[4];   // last processed day (fn_8014CF9C / fn_8014CC98)
    /* 0x48 */ u16 mEnd[4];    // events to end the next day (0xFFFF: none)
};

// 80097568
void dFgMngProc_c::endEvents(dFdBase_c *fd) {
    dSaveTown_c *save = dSaveData_c::getTown();
    dFgSaveEvents_c *events = (dFgSaveEvents_c *)save->_068372;
    for (int i = 0; i < 4; i++) {
        int id = events->mEnd[i];
        if (id != 0xFFFF) {
            switch (id) {
            case EVENT_BUNNY_DAY:
                endEggs(fd);
                break;
            case EVENT_COUNTDOWN:
                fn_80169ADC();
                break;
            case EVENT_FISHING_TOURNEY:
                fn_80169B2C();
                break;
            case EVENT_BUG_OFF:
                fn_80169B54();
                break;
            case EVENT_HARVEST_FESTIVAL:
                fn_80169BB4();
                break;
            case EVENT_FESTIVALE:
                fn_80169B04();
                break;
            case EVENT_LAMP_BURIED:
            case EVENT_LAMP_SAME_DAY:
                removeLamps(fd);
                break;
            }
            events->mEnd[i] = 0xFFFF;
        }
    }
}

// 80097640
void dFgMngProc_c::procDay(dFdBase_c *fd, dTime_c *time) {
    dSaveTown_c *save = dSaveData_c::getTown();
    dTime_c t = *time;
    t.add(0, -TIME_DAY_START_HOUR, 0, 0);
    u8 *stamp = save->_068372 + 0x44;
    BOOL due = FALSE;
    if (*(u16 *)stamp == 0 || stamp[3] == 0) {
        due = TRUE;
    }
    if (due || fn_8014CF9C(stamp, &t)) {
        endEvents(fd);
        procEvents(fd, &t);
        fn_8014CC98(stamp, &t);
    }
}

// 80097760
void dFgMngProc_c::addEvent(int id) {
    fn_8014D89C(dSaveData_c::getTown()->_068372, id);
}

// 8009779C
void dFgMngProc_c::procLiveDay(BOOL arg) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    dTime_c now = *dTime_c::getCurrent();
    sFdAssess.assessLiveTown();
    procDay(fd, &now);
}

// 80097848
void dFgMngProc_c::plantTreesB(dFdBase_c *fd, int blockW, int blockH) {
    for (int bx = 0; bx < blockW; bx++) {
        BOOL none = TRUE;
        for (int bz = 0; bz < blockH; bz++) {
            if (sFdAssess.getBlock(bx, bz)->mFlags.mBits.mHasTreeB) {
                none = FALSE;
                break;
            }
        }
        if (none == TRUE) {
            plantTree(fd, bx + 1, rndInt(blockH) + 1, 1);
        }
    }
}

// 80097934
void dFgMngProc_c::plantTreesA(dFdBase_c *fd, int blockW, int blockH) {
    for (int n = 2 - sFdAssess.mTreeABlockNum; n > 0; n--) {
        int x = rndInt(blockW);
        int z = rndInt(blockH);
        plantTree(fd, x + 1, z + 1, 0);
    }
}

// 80097A00
void dFgMngProc_c::plantTreesC(dFdBase_c *fd, int blockW, int blockH) {
    int bx;
    int bz;
    for (bz = 0; bz < blockH; bz++) {
        for (bx = 0; bx < blockW; bx++) {
            if (!sFdAssess.getBlock(bx, bz)->mFlags.mBits.mHasTreeC) {
                plantTree(fd, bx + 1, bz + 1, 2);
            }
        }
    }
}

// 80097AA4
void dFgMngProc_c::plantTrees(dFdBase_c *fd, int blockW, int blockH) {
    plantTreesB(fd, blockW, blockH);
    plantTreesA(fd, blockW, blockH);
    plantTreesC(fd, blockW, blockH);
}

// 80097B1C
void dFgMngProc_c::buryFossils(dFdBase_c *fd, int blockW, int blockH) {
    int x;
    int z;
    int max;
    int n;
    max = 3;
    if (*(u16 *)&dSaveData_c::getTown()->_05EC68[0xC] & 8) {
        max = 5;
    }
    n = max - sFdAssess.mFossilNum;
    x = rndInt(blockW - 1);
    z = rndInt(blockH - 1);
    dItem::Item fossil(dItem::ITEM_IDX_FOSSIL);
    for (; n > 0; n--) {
        x = (x + rndInt(blockW - 1) + 1) % blockW;
        z = (z + rndInt(blockH - 1) + 1) % blockH;
        putInBlock(fd, x, z, fossil.mId, isDigUnit, TRUE);
    }
}

// 80097CA4
void dFgMngProc_c::buryPitfall(dFdBase_c *fd, int blockW, int blockH) {
    if (!sFdAssess.mFlags.mBits.mHasPitfall) {
        int x = rndInt(blockW);
        int z = rndInt(blockH);
        dItem::Item pitfall(dItem::ITEM_IDX_PITFALL_SEED);
        putInBlock(fd, x, z, pitfall.mId, isGroundUnit, TRUE);
    }
}

// 80097D84
BOOL dFgMngProc_c::changeStone(dFdBase_c *fd, dFdAsBlock_c *block, int kind, int blockX, int blockZ) {
    int num;
    int n;
    int ux;
    int uz;
    num = block->mStoneNum;
    n = rndInt(num);
    block->mStoneNum = num - 1;
    for (uz = 0; uz < UT_Z_NUM; uz++) {
        for (ux = 0; ux < UT_X_NUM; ux++) {
            dItem::Item *item = fd->getItem(blockX + 1, blockZ + 1, ux, uz, 0);
            if (item != NULL && inRange(item, dItem::FG_STONE_A, dItem::FG_STONE_E) && --n < 0) {
                dFgMngProc_c::setUnitItem(fd, ((blockX + 1) << 4) + ux, ((blockZ + 1) << 4) + uz,
                            item->mId + sStoneKindBase[kind] - dItem::FG_STONE_A, FALSE);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80097EB4
BOOL dFgMngProc_c::changeStoneOf(dFdBase_c *fd, int blockW, int blockH, int kind, int num) {
    int n;
    int bx;
    int bz;
    n = rndInt(num);
    for (bz = 0; bz < blockH; bz++) {
        for (bx = 0; bx < blockW; bx++) {
            dFdAsBlock_c *block = &sFdAssess.mBlocks[bx][bz];
            int stones = block->mStoneNum;
            if (stones > 0 && (n -= stones) < 0 && changeStone(fd, block, kind, bx, bz)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80097FA4
void dFgMngProc_c::resetStones(dFdBase_c *fd, int blockW, int blockH) {
    int bx;
    int bz;
    int ux;
    int uz;
    for (bz = 0; bz < blockH; bz++) {
        for (bx = 0; bx < blockW; bx++) {
            for (uz = 0; uz < UT_Z_NUM; uz++) {
                for (ux = 0; ux < UT_X_NUM; ux++) {
                    dItem::Item *item = fd->getItem(bx + 1, bz + 1, ux, uz, 0);
                    if (item != NULL && inRange(item, dItem::FG_MONEY_ROCK_P0_A, dItem::FG_MONEY_ROCK_P3_E)) {
                        dFgMngProc_c::setUnitItem(fd, bx + 1, bz + 1, ux, uz, (item->mId - dItem::FG_MONEY_ROCK_P0_A) % 5 + dItem::FG_STONE_A, FALSE);
                    }
                }
            }
        }
    }
}

// 800980B4
void dFgMngProc_c::changeStones(dFdBase_c *fd, int blockW, int blockH) {
    int n = sFdAssess.mStoneNum;
    for (int kind = 0; kind < 4; kind++) {
        if (n <= 0) {
            return;
        }
        if (!((1 << kind) & sFdAssess.mFlags.mBits.mStoneKinds) && changeStoneOf(fd, blockW, blockH, kind, n)) {
            n--;
        }
    }
    sFgMngState.mStone.init();
}

// 80098164: ages the planted red turnips (FG_RED_TURNIP_*) and removes the spoiled ones.
void dFgMngProc_c::growRedKabu(dFdBase_c *fd, int num) {
    u16 *row;
    int bx;
    int bz;
    int ux;
    int uz;
    for (bz = 1; bz < FG_BLOCK_Z_NUM + 1; bz++) {
        for (bx = 1; bx < FG_BLOCK_X_NUM + 1; bx++) {
            u16 *rows = (u16 *)dSaveData_c::getTown()->mMainField.getBlockWater(bx, bz);
            if (rows != NULL) {
                row = rows;
                for (uz = 0; uz < UT_Z_NUM; uz++, row++) {
                    for (ux = 0; ux < UT_X_NUM; ux++) {
                        dItem::Item *item = fd->getItem(bx, bz, ux, uz, 0);
                        if (item != NULL) {
                            u16 id = item->mId;
                            if (inRange(item, dItem::FG_RED_TURNIP_WILTED, dItem::FG_RED_TURNIP_WILTED_WATERED)) {
                                if (id == dItem::FG_RED_TURNIP_WILTED) {
                                    dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, dItem::ITEM_ID_NONE, FALSE);
                                } else if (num > 1) {
                                    dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, dItem::FG_RED_TURNIP_WILTED, FALSE);
                                } else if (num == 1) {
                                    if (*row & (1 << ux)) {
                                        if (id == dItem::FG_RED_TURNIP_WILTED_WATERED) {
                                            id = dItem::FG_RED_TURNIP_0;
                                        } else if (id != dItem::FG_RED_TURNIP_6) {
                                            id++;
                                        }
                                        dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, id, FALSE);
                                    } else {
                                        dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, dItem::FG_RED_TURNIP_WILTED, FALSE);
                                    }
                                }
                            } else if (item->isSame(dItem::Item(dItem::ITEM_IDX_SPOILED_TURNIPS))) {
                                dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, dItem::ITEM_ID_NONE, FALSE);
                            }
                        }
                    }
                }
            }
        }
    }
}

// 80098370
void dFgMngProc_c::putCoconut(dFdBase_c *fd, int blockW) {
    if (!sFdAssess.mFlags.mBits.mHasCoconut && fgMngProc_rndF(100.0f) < 10.0f) {
        int x = rndInt(blockW);
        dItem::Item coconut(dItem::ITEM_IDX_COCONUT);
        putInBlock(fd, x, FG_BEACH_BLOCK_Z, coconut.mId, isBeachUnit, FALSE);
    }
}

// 80098434
void dFgMngProc_c::buryGyroids(dFdBase_c *fd, int blockW, int blockH, BOOL bury) {
    int x;
    int z;
    int n;
    int i;
    n = 0;
    if (bury) {
        n = 3;
    }
    if (*(u16 *)&dSaveData_c::getTown()->_05EC68[0xC] & 4) {
        n += 2;
    }
    if (n != 0) {
        dItem::Item item;
        x = rndInt(blockW - 1);
        z = rndInt(blockH - 1);
        for (i = 0; i < n; i++) {
            item.setFromIndex(dItem::ITEM_IDX_MEGA_CLANKOID, (u32)fgMngProc_rndF(FG_GYROID_NUM), FALSE);
            x = (x + rndInt(blockW - 1) + 1) % blockW;
            z = (z + rndInt(blockH - 1) + 1) % blockH;
            putInBlock(fd, x, z, item.mId, isDigUnit, TRUE);
        }
    }
}

// 800985E4
BOOL dFgMngProc_c::isMushroomSeason(const dTime_c *time) {
    BOOL ok = FALSE;
    u16 date = (time->month << 8) | (u8)time->mday;
    if (date >= MONTHDAY(MONTH_NOVEMBER, 1) && date <= MONTHDAY(MONTH_NOVEMBER, 30)) {
        ok = TRUE;
    }
    return ok;
}

// 80098614
void dFgMngProc_c::putMushrooms(dFdBase_c *fd) {
    dFdAsPos_c unit;
    dFdAsPos_c block1;
    dFdAsPos_c block2;
    int mushNum;
    u32 tried;
    u32 bit;
    int ftrNum;
    tried = 0;
    ftrNum = FG_TOWN_MUSH_FTR_NUM - sFdAssess.mMushFtrNum;
    mushNum = FG_TOWN_MUSHROOM_NUM - sFdAssess.mMushroomNum;
    while (ftrNum > 0) {
        if (tried == FG_ALL_BLOCKS_MASK) {
            break;
        }
        block1.mX = rndInt(FG_BLOCK_X_NUM);
        block1.mZ = rndInt(FG_BLOCK_Z_NUM);
        bit = 1 << (block1.mX * FG_BLOCK_Z_NUM + block1.mZ);
        if (!(tried & bit)) {
            if (findMushroomUnit(fd, &block1, &unit)) {
                dItem::Item ftr(dItem::ITEM_IDX_FOREST_WALL, rndInt(FG_MUSH_FTR_NUM), FALSE);
                dFgMngProc_c::setUnitItem(fd, unit.mX, unit.mZ, ftr.mId, FALSE);
                ftrNum--;
            } else {
                tried |= bit;
            }
        }
    }
    while (mushNum > 0) {
        if (tried == FG_ALL_BLOCKS_MASK) {
            break;
        }
        block2.mX = rndInt(FG_BLOCK_X_NUM);
        block2.mZ = rndInt(FG_BLOCK_Z_NUM);
        bit = 1 << (block2.mX * FG_BLOCK_Z_NUM + block2.mZ);
        if (!(tried & bit)) {
            if (findMushroomUnit(fd, &block2, &unit)) {
                u16 id = getMushroomKind();
                dItem::Item rare(dItem::ITEM_IDX_RARE_MUSHROOM);
                dFgMngProc_c::setUnitItem(fd, unit.mX, unit.mZ, id, id == rare.mId);
                mushNum--;
            } else {
                tried |= bit;
            }
        }
    }
}

// 800987FC
BOOL dFgMngProc_c::findMushroomUnit(dFdBase_c *fd, const dFdAsPos_c *block, dFdAsPos_c *out) {
    u16 done[UT_Z_NUM];
    dFdAsPos_c cands[UT_TOTAL_NUM];
    dFdAsPos_c *cand;
    int z;
    const u8 *ofs;
    u16 *flags;
    int baseX;
    int baseZ;
    int nx;
    int nz;
    int num;
    int uz;
    int ux;
    int i;
    dItem::Item *item;
    baseX = (block->mX + 1) * UT_X_NUM;
    baseZ = (block->mZ + 1) * UT_Z_NUM;
    memset(done, 0, sizeof(done));
    cand = cands;
    flags = done;
    num = 0;
    for (uz = -1; uz < UT_Z_NUM; uz++) {
        z = baseZ + uz;
        for (ux = -1; ux <= UT_X_NUM; ux++) {
            item = fd->getItem(baseX + ux, z, 0);
            if (item == NULL) {
                continue;
            }
            dItem::FgInfo *info = item->getFgInfo();
            if (!((info != NULL && info->mTreeStage >= 1) || inRange(item, dItem::FG_STUMP_S1, dItem::FG_PALM_STUMP))) {
                continue;
            }
            if (inRange(item, dItem::FG_PALM_SAPLING, dItem::FG_PALM_FRUIT) || (item->mId >= dItem::FG_PALM_STUMP_S1 && item->mId <= dItem::FG_PALM_STUMP)) {
                continue;
            }
            for (i = 0, ofs = sAround5; i < 5; i++, ofs++) {
                nx = ux + (*ofs >> 4) - 8;
                nz = uz + (*ofs & 0xF) - 8;
                if (nx >= 0 && nx < UT_X_NUM && nz >= 0 && nz < UT_Z_NUM && !(flags[nz] & (1 << nx))) {
                    flags[nz] |= 1 << nx;
                    nz += baseZ;
                    nx += baseX;
                    if (isGrassUnit(fd, nx, nz)) {
                        dItem::Item *it = fd->getItem(nx, nz, 0);
                        if (it != NULL && it->mId == dItem::ITEM_ID_NONE) {
                            cand->mX = nx;
                            num++;
                            cand->mZ = nz;
                            cand++;
                        }
                    }
                }
            }
        }
    }
    if (num == 0) {
        return FALSE;
    }
    int idx = rndInt(num);
    out->mX = cands[idx].mX;
    out->mZ = cands[idx].mZ;
    return TRUE;
}

// 80098A40
u16 dFgMngProc_c::getMushroomKind() {
    const f32 *rate;
    f32 max;
    if (sFdAssess.mRank == dFdAssess_c::TOWN_RANK_PERFECT) {
        rate = sMushRatePerfect;
        max = 100.0f;
    } else {
        rate = sMushRate;
        max = 100.0f;
    }
    f32 roll = fgMngProc_rndF(max);
    for (int i = 0; i < FG_MUSHROOM_KIND_NUM; rate++, i++) {
        roll -= *rate;
        if (roll < 0.0f) {
            return dItem::Item(dItem::ITEM_IDX_ELEGANT_MUSHROOM, i, FALSE).mId;
        }
    }
    return dItem::ITEM_ID_NONE;
}

// 80098AE0: a buried shovel turns into a golden shovel.
void dFgMngProc_c::makeGoldenShovels(dFdBase_c *fd, int blockW, int blockH) {
    dItem::Item *item;
    BOOL grow;
    BOOL found;
    int bx;
    int bz;
    int ux;
    int uz;
    for (bz = 1; bz < blockH + 1; bz++) {
        for (bx = 1; bx < blockW + 1; bx++) {
            for (uz = 0; uz < UT_Z_NUM; uz++) {
                for (ux = 0; ux < UT_X_NUM; ux++) {
                    item = fd->getItem(bx, bz, ux, uz, 0);
                    grow = FALSE;
                    found = FALSE;
                    if (item != NULL) {
                        if (item->mId == dItem::Item(dItem::ITEM_IDX_SHOVEL).mId) {
                            found = TRUE;
                        }
                    }
                    if (found && fd->isBuried(bx, bz, ux, uz)) {
                        grow = TRUE;
                    }
                    if (grow) {
                        dFgMngProc_c::setUnitItem(fd, bx, bz, ux, uz, dItem::Item(dItem::ITEM_IDX_GOLDEN_SHOVEL).mId, TRUE);
                    }
                }
            }
        }
    }
}

// 80098C10
BOOL dFgMngProc_c::isGrassUnit(dFdBase_c *fd, int x, int z) {
    if (fd->isGrassGround(x, z)) {
        return dFgMngProc_c::isUnitFree(x, z);
    }
    return FALSE;
}

// 80098C64
BOOL dFgMngProc_c::isGroundUnit(dFdBase_c *fd, int x, int z) {
    BOOL ok = FALSE;
    switch (fd->getDigType(x, z)) {
    case 0:
    case 1:
        ok = dFgMngProc_c::isUnitFree(x, z);
        break;
    }
    return ok;
}

// 80098CD0
BOOL dFgMngProc_c::isLilyUnit(dFdBase_c *fd, int x, int z) {
    if (fd->isGrassGround(x, z)) {
        int attr = fd->getBgAttr(x, z);
        if (attr >= BG_ATTR_CLIFF_N && attr <= BG_ATTR_CLIFF_SE2) {
            return TRUE;
        }
        attr = fd->getBgAttr(x - 1, z - 1);
        if (attr == BG_ATTR_CLIFF_NW || attr == BG_ATTR_CLIFF_NW2) {
            return TRUE;
        }
        if (fd->getBgAttr(x, z - 1) == BG_ATTR_CLIFF_N) {
            return TRUE;
        }
        attr = fd->getBgAttr(x + 1, z - 1);
        if (attr == BG_ATTR_CLIFF_NE || attr == BG_ATTR_CLIFF_NE2) {
            return TRUE;
        }
        if (fd->getBgAttr(x - 1, z) == BG_ATTR_CLIFF_W) {
            return TRUE;
        }
        if (fd->getBgAttr(x + 1, z) == BG_ATTR_CLIFF_E) {
            return TRUE;
        }
        attr = fd->getBgAttr(x - 1, z + 1);
        if (attr == BG_ATTR_CLIFF_SW || attr == BG_ATTR_CLIFF_SW2) {
            return TRUE;
        }
        if (fd->getBgAttr(x, z + 1) == BG_ATTR_CLIFF_S) {
            return TRUE;
        }
        attr = fd->getBgAttr(x + 1, z + 1);
        if (attr == BG_ATTR_CLIFF_SE || attr == BG_ATTR_CLIFF_SE2) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

// 80098E70
BOOL dFgMngProc_c::isRafflesiaUnit(dFdBase_c *fd, int x, int z) {
    int nx;
    int nz;
    const u8 *ofs;
    BOOL ok;
    int i;
    ok = FALSE;
    switch (fd->getPlantType(x, z)) {
    case 2:
        if (dFgMngProc_c::isUnitFree(x, z)) {
            ok = TRUE;
            for (i = 0, ofs = sAround8; i < 8; i++, ofs++) {
                nx = x + (*ofs >> 4) - 8;
                nz = z + (*ofs & 0xF) - 8;
                if (fd->getItem(nx, nz, 0) != NULL) {
                    f32 height = fn_80074D64(nx, nz);
                    nw4r::math::VEC3 pos;
                    dFdBase_c::getUnitCenterPos(&pos, nx, nz);
                    if (height != fn_80074974(&pos, 1)) {
                        ok = FALSE;
                        break;
                    }
                }
            }
        }
        break;
    }
    return ok;
}

// 80098F6C
BOOL dFgMngProc_c::isFlowerUnit(dFdBase_c *fd, int x, int z) {
    return isGrassUnit(fd, x, z);
}

// 80098F70
BOOL dFgMngProc_c::isDigUnit(dFdBase_c *fd, int x, int z) {
    BOOL ok = FALSE;
    switch (fd->getDigType(x, z)) {
    case 0:
    case 1:
        ok = dFgMngProc_c::isUnitFree(x, z);
        break;
    }
    return ok;
}

// 80098FDC
BOOL dFgMngProc_c::isBeachUnit(dFdBase_c *fd, int x, int z) {
    return isSandUnit(fd, x, z);
}

// 80098FE0
BOOL dFgMngProc_c::isSandUnit(dFdBase_c *fd, int x, int z) {
    BOOL ok = FALSE;
    if (fd->isBeachGround(x, z)) {
        ok = TRUE;
    }
    return ok;
}

// 8009901C
BOOL dFgMngProc_c::plantTree(dFdBase_c *fd, int blockX, int blockZ, int kind) {
    u16 base[UT_TOTAL_NUM];
    dFdAsPos_c units[UT_TOTAL_NUM];
    u16 *b;
    dFdAsPos_c *u;
    int num;
    int ux;
    int uz;
    b = base;
    u = units;
    num = 0;
    for (uz = 0; uz < UT_Z_NUM; uz++) {
        for (ux = 0; ux < UT_X_NUM; ux++) {
            dItem::Item *item = fd->getItem(blockX, blockZ, ux, uz, 0);
            if (item != NULL) {
                u16 id = item->mId;
                *b = dItem::FG_CEDAR;
                switch (id) {
                case dItem::FG_TREE:
                    *b = dItem::FG_TREE;
                case dItem::FG_CEDAR:
                    u->mX = ux;
                    b++;
                    num++;
                    u->mZ = uz;
                    u++;
                    break;
                }
            }
        }
    }
    if (num > 0) {
        int i = rndInt(num);
        dFgMngProc_c::setUnitItem(fd, blockX * UT_X_NUM + units[i].mX, blockZ * UT_Z_NUM + units[i].mZ, kind + base[i] + 1, FALSE);
        return TRUE;
    }
    return FALSE;
}

// 80099170
BOOL dFgMngProc_c::putInBlock(dFdBase_c *fd, int blockX, int blockZ, u16 item, UnitFunc func, BOOL flag) {
    dFdAsPos_c units[UT_TOTAL_NUM];
    int num = collectUnits(units, fd, blockX, blockZ, func);
    return putRandom(fd, num, units, item, flag) >= 0;
}

// 800991FC
int dFgMngProc_c::collectUnits(dFdAsPos_c *out, dFdBase_c *fd, int blockX, int blockZ, UnitFunc func) {
    int z;
    int baseX;
    int num;
    int bx;
    int bz;
    int ux;
    int uz;
    int x;
    bx = blockX + 1;
    bz = blockZ + 1;
    z = bz * UT_Z_NUM;
    baseX = bx * UT_X_NUM;
    num = 0;
    for (uz = 0; uz < UT_Z_NUM; uz++, z++) {
        for (ux = 0; ux < UT_X_NUM; ux++) {
            dItem::Item *item = fd->getItem(bx, bz, ux, uz, 0);
            if (item != NULL && item->mId == dItem::ITEM_ID_NONE) {
                x = baseX + ux;
                if (func(fd, x, z) == TRUE) {
                    out->mX = x;
                    num++;
                    out->mZ = z;
                    out++;
                }
            }
        }
    }
    return num;
}

// 800992D4
int dFgMngProc_c::putRandom(dFdBase_c *fd, int num, const dFdAsPos_c *units, u16 item, BOOL flag) {
    int i = -1;
    if (num > 0) {
        i = rndInt(num);
        dFgMngProc_c::setUnitItem(fd, units[i].mX, units[i].mZ, item, flag);
    }
    return i;
}

// Empty units of one block where an egg can be hidden (EE: Easter egg).
class EECandBkUnit_c : public dSearchCandXZ_c<UT_X_NUM, UT_Z_NUM> {
public:
    EECandBkUnit_c(dFdBase_c *fd) : mFd(fd), mBlockX(1), mBlockZ(1) {}
    virtual BOOL check(int x, int z); // 80099AEC

    /* 0x3C */ dFdBase_c *mFd;
    /* 0x40 */ int mBlockX;
    /* 0x44 */ int mBlockZ;
}; // size 0x48

// Field units of one block holding a grown cedar (FG_CEDAR): the cedars that get the lights
// (FG_CEDAR_LIGHTS) from Dec 15 to Jan 3.
class DTCandBkUnit_c : public dSearchCandXZ_c<UT_X_NUM, UT_Z_NUM> {
public:
    DTCandBkUnit_c(dFdBase_c *fd) : mFd(fd), mBlockX(1), mBlockZ(1) {}
    virtual BOOL check(int x, int z); // 8009AAB8

    /* 0x3C */ dFdBase_c *mFd;
    /* 0x40 */ int mBlockX;
    /* 0x44 */ int mBlockZ;
}; // size 0x48

// Blocks where the lamp can be buried.
class LampBkSearch_c : public dFdBkSearchCand_c {
public:
    LampBkSearch_c(dFdBase_c *fd) : dFdBkSearchCand_c(BLOCK_X_NUM, BLOCK_Z_NUM), mFd(fd) {}
    virtual BOOL check(int x, int z); // 8009ADE4

    /* 0x24 */ dFdBase_c *mFd;
}; // size 0x28

// Units of one block where the lamp can be buried.
class LampUtSearch_c : public dSearchCandXZ_c<UT_X_NUM, UT_Z_NUM> {
public:
    LampUtSearch_c(dFdBase_c *fd) : mFd(fd), mBlockX(1), mBlockZ(1) {}
    virtual BOOL check(int x, int z); // 8009AE70

    /* 0x3C */ dFdBase_c *mFd;
    /* 0x40 */ int mBlockX;
    /* 0x44 */ int mBlockZ;
}; // size 0x48

#define EGG_KIND_NUM 13 // 12 egg kinds and the fake egg

// 8009936C: plants weeds (fg 0x57..0x5A) on num of the spotNum spots, at random if fewer.
int dFgMngProc_c::plantGrass(dFdBase_c *fd, int spotNum, dFdAsPos_c *spots, int num) {
    int left;
    int planted = 0;
    if (spotNum <= 0) {
        return 0;
    }
    if (spotNum <= num) {
        for (int i = 0; i < spotNum; i++) {
            setUnitItem(fd, spots->mX, spots->mZ, dItem::FG_WEED_A + (int)fgMngProc_rndF(4.0f), FALSE);
            planted++;
            spots++;
        }
    } else {
        u16 used[16];
        for (int i = 0; i < 16; i++) {
            used[i] = 0;
        }
        left = num;
        while (left > 0) {
            int idx = fgMngProc_rndF(spotNum);
            if (!(used[idx >> 4] & (1 << (idx & 15)))) {
                dFdAsPos_c *spot = &spots[idx];
                setUnitItem(fd, spot->mX, spot->mZ, dItem::FG_WEED_A + (int)fgMngProc_rndF(4.0f), FALSE);
                planted++;
                left--;
                used[idx >> 4] |= 1 << (idx & 15);
            }
        }
    }
    return planted;
}

// 80099514
void dFgMngProc_c::setUnitItem(dFdBase_c *fd, int blockX, int blockZ, int unitX, int unitZ, u16 id, BOOL buried) {
    setUnitItem(fd, (blockX << 4) + unitX, (blockZ << 4) + unitZ, id, buried);
}

// 80099538
void dFgMngProc_c::setUnitItem(dFdBase_c *fd, int unitX, int unitZ, u16 id, BOOL buried) {
    dItem::Item item(id);
    fd->setItem(&item, unitX, unitZ, 0);
    if (buried) {
        fd->setBuried(unitX, unitZ);
    } else {
        fd->clearBuried(unitX, unitZ);
    }
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, unitX, unitZ);
    dInsectInfo::setPos(pos);
}

// 800995E0
BOOL dFgMngProc_c::isUnitFree(int unitX, int unitZ) {
    if (unitX < UT_X_NUM || unitZ < UT_Z_NUM) {
        return FALSE;
    }
    int idx = (unitX - UT_X_NUM) * FG_BLOCK_Z_NUM + ((unitZ - UT_Z_NUM) >> 4);
    if (idx >= FG_UNIT_MASK_NUM) {
        return FALSE;
    }
    return (sFgMngUnitMask[idx] & (1 << (unitZ & 15))) == 0;
}

// 80099648: clears fg 0x94 from the usable field.
void dFgMngProc_c::clearFg94() {
    dFdBase_c *fd;
    int blockW;
    int blockH;
    int blockX;
    int blockZ;
    int unitX;
    int unitZ;
    fd = fn_80190C44(FD_ID_TOWN);
    blockW = fd->mBlockW - 2;
    blockH = fd->mBlockH - 2;
    for (blockZ = 1; blockZ <= blockH; blockZ++) {
        for (blockX = 1; blockX <= blockW; blockX++) {
            for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
                for (unitX = 0; unitX < UT_X_NUM; unitX++) {
                    dItem::Item *item = fd->getItem(blockX, blockZ, unitX, unitZ, 0);
                    if (item != NULL && inRange(item, dItem::FG_HOLE, dItem::FG_HOLE)) {
                        setUnitItem(fd, blockX, blockZ, unitX, unitZ, dItem::ITEM_ID_NONE, FALSE);
                    }
                }
            }
        }
    }
}

// 80099744: turnips on a field go bad.
void dFgMngProc_c::spoilKabu(dFdBase_c *fd) {
    dItem::Item spoiled(dItem::ITEM_IDX_SPOILED_TURNIPS);
    if (fd != NULL) {
        int unitW = fd->mUnitW;
        int unitH = fd->mUnitH;
        int x;
        int z;
        for (z = 0; z < unitH; z++) {
            for (x = 0; x < unitW; x++) {
                dItem::Item *item = fd->getItem(x, z, 0);
                if (item != NULL && item->isKabu()) {
                    fd->setItem(&spoiled, x, z, 0);
                }
                item = fd->getItem(x, z, 1);
                if (item != NULL && item->isKabu()) {
                    fd->setItem(&spoiled, x, z, 1);
                }
            }
        }
    }
}

// 80099838: turnips in the players' pockets go bad.
void dFgMngProc_c::spoilPocketKabu() {
    dItem::Item spoiled(dItem::ITEM_IDX_SPOILED_TURNIPS);
    dItem::Item *pocket;
    dPrivateData_c *p;
    u32 i;
    int j;
    for (i = 0; i < PLAYER_NUM; i++) {
        p = dPlayerMgr_c::getPlayer(i);
        if (p->mPID.isValid()) {
            pocket = p->mPockets;
            for (j = 0; j < PLAYER_POCKETS_COUNT; j++, pocket++) {
                if (pocket != NULL && pocket->isKabu()) {
                    p->setPocket(&spoiled, j, FALSE);
                }
            }
        }
    }
}

// 800998F0
void dFgMngProc_c::spoilTownKabu() {
    spoilKabu(fn_80190C44(FD_ID_TOWN));
}

// 8009992C: the rooms (fields 2..13).
void dFgMngProc_c::spoilRoomKabu() {
    for (int i = 2; i < 14; i++) {
        spoilKabu(fn_80190C44(i));
    }
}

// 80099980
void dFgMngProc_c::spoilRecycleBinKabu() {
    dRecycleBin_c *bin = &dSaveData_c::getTown()->mRecycleBin;
    dItem::Item item;
    for (int i = 0; i < RECYCLE_BIN_ITEM_NUM; i++) {
        item.mId = bin->get(i);
        if (item.isKabu()) {
            dItem::Item spoiled(dItem::ITEM_IDX_SPOILED_TURNIPS);
            bin->set(i, spoiled.mId);
        }
    }
}

// 80099A10
void dFgMngProc_c::spoilPoliceBoxKabu() {
    dPoliceBox_c *box = &dSaveData_c::getTown()->mPoliceBox;
    dItem::Item item;
    for (int i = 0; i < POLICE_BOX_ITEM_NUM; i++) {
        item.mId = box->get(i);
        if (item.isKabu()) {
            dItem::Item spoiled(dItem::ITEM_IDX_SPOILED_TURNIPS);
            box->set(i, spoiled.mId);
        }
    }
}

// 80099AA0: every turnip in the town goes bad.
void dFgMngProc_c::spoilAllKabu() {
    spoilPocketKabu();
    spoilTownKabu();
    spoilRoomKabu();
    spoilRecycleBinKabu();
    spoilPoliceBoxKabu();
}

// 80099AEC
BOOL EECandBkUnit_c::check(int x, int z) {
    int unitX = (mBlockX << 4) + x;
    int unitZ = (mBlockZ << 4) + z;
    dItem::Item *item = mFd->getItem(unitX, unitZ, 0);
    if (item == NULL || item->mId != dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    return dFgMngProc_c::isDigUnit(mFd, unitX, unitZ) != 0;
}

// 80099B88: one egg of kind *kind for a block; drops the kind from the list once none is left.
void dFgMngProc_c::takeEgg(int *pick, int *kind, int *blockEggs, int *total, int *kinds, int *left, int *kindNum) {
    blockEggs[*kind]++;
    left[*kind]--;
    if (left[*kind] <= 0) {
        (*kindNum)--;
        for (int i = *pick; i < *kindNum; i++) {
            kinds[i] = kinds[i + 1];
        }
    }
    (*total)--;
}

// 80099C0C: Bunny Day: collects the eggs, then hides new ones.
void dFgMngProc_c::updateEggs() {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    collectEggs(fd);
    hideEggs(fd);
}

// 80099C60: removes the eggs from the field and credits them to the town owner.
// The loops are not fully unrolled in the target (unlike everywhere else in the TU).
#pragma push
#pragma opt_unroll_loops off
void dFgMngProc_c::collectEggs(dFdBase_c *fd) {
    u16 found[EGG_KIND_NUM];
    dPrivateData_c *player;
    int x;
    int z;
    int unitW;
    int unitH;
    dPersonalID_c *pid;
    dPrivateData_c *p;
    dPersonalID_c *owner;
    int i;
    memset(found, 0, sizeof(found));
    player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL && !player->isFlag0(0x28) && !player->isFlag1(0x43)) {
        for (int i = 0; i < EGG_KIND_NUM - 1; i++) {
            player->_83CC[i] = 1;
        }
        player->_83CC[EGG_KIND_NUM - 1] = 18;
    }
    unitW = fd->mBlockW * UT_X_NUM;
    unitH = fd->mBlockH * UT_Z_NUM;
    for (z = 0; z < unitH; z++) {
        for (x = 0; x < unitW; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && dItem::isRealItemId(item->mId)) {
                const dItem::BITM *bitm = dItem::getBITM(item->mId);
                if (bitm->getKind() == dItem::KIND_EGG_BINGO_BEFORE) {
                    dItem::Item first(dItem::ITEM_IDX_BUNNY_EGG_00);
                    found[(item->mId - first.mId) >> 2]++;
                    setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, FALSE);
                } else if (bitm->getKind() == dItem::KIND_EGG_FAKE_BEFORE) {
                    setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, FALSE);
                    found[EGG_KIND_NUM - 1]++;
                }
            }
        }
    }
    owner = (dPersonalID_c *)&dSaveData_c::getTown()->_068372[8];
    for (i = 0; i < PLAYER_NUM; i++) {
        p = dPlayerMgr_c::getPlayer(i);
        if (p->mPID.isValid()) {
            pid = &p->mPID;
            if (*pid == *owner) {
                for (int j = 0; j < EGG_KIND_NUM; j++) {
                    p->_83CC[j] += found[j];
                }
                break;
            }
        }
    }
}
#pragma pop

// 80099F4C: Bunny Day: hides the current player's eggs in the usable blocks: as evenly as possible
// over the blocks with free units, then the rest at random; the real eggs (by kind) first, then the
// fake ones.
// The locals are declared up front in this order for the target's register allocation.
void dFgMngProc_c::hideEggs(dFdBase_c *fd) {
    int n;
    int fakeBlock;
    int fakeBz;
    int realBz;
    int fakeBx;
    dPrivateData_c *player;
    int block;
    int bx;
    int realBx;
    int unitNum;
    u32 restMask;
    int bz;
    int kindNum;
    u32 mask;
    int *eggs;
    u32 bit;
    int *left;
    int realBlock;
    int realNum;
    int x;
    int j;
    int *restEntry;
    int per;
    int *restEggs;
    int z;
    int fakeX;
    int pick;
    int *entry;
    int fakeZ;
    int k;
    int kind;
    int m;
    int i;
    int kindLeftNum;
    player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return;
    }
    int blocks[FG_BLOCK_TOTAL_NUM];
    int blockLeft[FG_BLOCK_TOTAL_NUM];
    unitNum = 0;
    EECandBkUnit_c cand(fd);
    int blockNum = 0;
    block = 0;
    left = blockLeft;
    for (bz = 1; bz < FG_BLOCK_Z_NUM + 1; bz++) {
        for (bx = 1; bx < FG_BLOCK_X_NUM + 1; bx++, left++, block++) {
            cand.clear();
            cand.mBlockX = bx;
            cand.mBlockZ = bz;
            cand.search();
            *left = cand.mCount;
            if (cand.mCount > 0) {
                unitNum += cand.mCount;
                blocks[blockNum] = block;
                blockNum++;
            }
        }
    }
    if (unitNum <= 0) {
        return;
    }
    int realEggs[FG_BLOCK_TOTAL_NUM];
    int fakeEggs[FG_BLOCK_TOTAL_NUM];
    memset(realEggs, 0, sizeof(realEggs));
    memset(fakeEggs, 0, sizeof(fakeEggs));
    int kindLeft[EGG_KIND_NUM - 1];
    int kinds[EGG_KIND_NUM - 1];
    kindNum = 0;
    realNum = 0;
    for (k = 0; k < EGG_KIND_NUM - 1; k++) {
        n = player->_83CC[k];
        kindLeft[k] = n;
        realNum += n;
        if (n != 0) {
            kinds[kindNum] = k;
            kindNum++;
        }
    }
    int eggNum = realNum + player->_83CC[EGG_KIND_NUM - 1];
    if (eggNum > unitNum) {
        eggNum = unitNum;
    }
    // As many eggs in every block as possible.
    for (per = eggNum / blockNum; per != 0; per = eggNum / blockNum) {
        mask = 0;
        for (i = 0; i < blockNum; i++) {
            mask |= 1 << blocks[i];
        }
        while (mask != 0) {
            if (realNum != 0) {
                eggs = realEggs;
                realNum--;
            } else {
                eggs = fakeEggs;
            }
            int pick;
            do {
                pick = cM::rndInt(blockNum);
                entry = &blocks[pick];
                bit = 1 << *entry;
            } while (!(mask & bit));
            mask &= ~bit;
            takeEgg(&pick, entry, eggs, &eggNum, blocks, blockLeft, &blockNum);
        }
    }
    // The rest at random.
    while (eggNum != 0) {
        restMask = 0;
        for (j = 0; j < blockNum; j++) {
            restMask |= 1 << blocks[j];
        }
        if (realNum != 0) {
            restEggs = realEggs;
            realNum--;
        } else {
            restEggs = fakeEggs;
        }
        int restPick;
        do {
            restPick = cM::rndInt(blockNum);
            restEntry = &blocks[restPick];
        } while (!(restMask & (1 << *restEntry)));
        takeEgg(&restPick, restEntry, restEggs, &eggNum, blocks, blockLeft, &blockNum);
    }
    dItem::Item egg;
    blockNum = 0;
    for (realBlock = 0; realBlock < FG_BLOCK_TOTAL_NUM; realBlock++) {
        if (realEggs[realBlock] != 0) {
            realBx = realBlock % FG_BLOCK_X_NUM + 1;
            realBz = realBlock / FG_BLOCK_X_NUM + 1;
            cand.clear();
            cand.mBlockX = realBx;
            cand.mBlockZ = realBz;
            cand.search();
            while (realEggs[realBlock] != 0) {
                if (cand.getRandomXZ(&x, &z)) {
                    pick = cM::rndInt(kindNum);
                    kind = kinds[pick];
                    cand.remove(x + z * cand.mWidth);
                    egg.setFromIndex(dItem::ITEM_IDX_BUNNY_EGG_00, kind, FALSE);
                    setUnitItem(fd, realBx, realBz, x, z, egg.mId, TRUE);
                    player->_83CC[kind]--;
                    kindLeftNum = --kindLeft[kind];
                    realEggs[realBlock]--;
                    if (kindLeftNum == 0) {
                        kindNum--;
                        for (m = pick; m < kindNum; m++) {
                            kinds[m] = kinds[m + 1];
                        }
                    }
                }
            }
        }
    }
    dItem::Item fake(dItem::ITEM_IDX_BUNNY_EGG_FAKE);
    for (fakeBlock = 0; fakeBlock < FG_BLOCK_TOTAL_NUM; fakeBlock++) {
        cand.clear();
        fakeBx = fakeBlock % FG_BLOCK_X_NUM + 1;
        fakeBz = fakeBlock / FG_BLOCK_X_NUM + 1;
        cand.mBlockX = fakeBx;
        cand.mBlockZ = fakeBz;
        cand.search();
        while (fakeEggs[fakeBlock] != 0) {
            if (cand.getRandomXZ(&fakeX, &fakeZ)) {
                cand.remove(fakeX + fakeZ * cand.mWidth);
                setUnitItem(fd, fakeBx, fakeBz, fakeX, fakeZ, fake.mId, TRUE);
                player->_83CC[EGG_KIND_NUM - 1]--;
                fakeEggs[fakeBlock]--;
            }
        }
    }
}

// 8009A820: removes every egg from a field.
void dFgMngProc_c::removeEggs(dFdBase_c *fd) {
    int x;
    int z;
    int unitW;
    int unitH;
    unitW = fd->mBlockW * UT_X_NUM;
    unitH = fd->mBlockH * UT_Z_NUM;
    for (z = 0; z < unitH; z++) {
        for (x = 0; x < unitW; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && dItem::isRealItemId(item->mId)) {
                const dItem::BITM *bitm = dItem::getBITM(item->mId);
                if (bitm->getKind() == dItem::KIND_EGG_BINGO_BEFORE || bitm->getKind() == dItem::KIND_EGG_FAKE_BEFORE) {
                    setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, FALSE);
                }
            }
        }
    }
}

// 8009A938: after Bunny Day: resets the players' egg counts and removes the hidden eggs.
void dFgMngProc_c::endEggs(dFdBase_c *fd) {
    dPrivateData_c *p;
    int i;
    for (i = 0; i < PLAYER_NUM; i++) {
        p = dPlayerMgr_c::getPlayer(i);
        if (p->mPID.isValid()) {
            memset(p->_83CC, 0, sizeof(p->_83CC));
            p->clearFlag1(0x43);
        }
    }
    int x;
    int z;
    int unitW;
    int unitH;
    unitW = fd->mBlockW * UT_X_NUM;
    unitH = fd->mBlockH * UT_Z_NUM;
    for (z = 0; z < unitH; z++) {
        for (x = 0; x < unitW; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL && fd->isBuried(x, z) && dItem::isRealItemId(item->mId)) {
                const dItem::BITM *bitm = dItem::getBITM(item->mId);
                if (bitm->getKind() == dItem::KIND_EGG_BINGO_BEFORE || bitm->getKind() == dItem::KIND_EGG_FAKE_BEFORE) {
                    setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, FALSE);
                }
            }
        }
    }
}

// 8009AAB8
BOOL DTCandBkUnit_c::check(int x, int z) {
    dItem::Item *item = mFd->getItem(mBlockX, mBlockZ, x, z, 0);
    if (item == NULL || item->mId != dItem::FG_CEDAR) {
        return FALSE;
    }
    return TRUE;
}

// 8009AB14: Dec 15 .. Jan 3: turns fg 0x52 into 0x56 (up to 3 per block); else back.
void dFgMngProc_c::updateFg56() {
    dFdBase_c *fd;
    int blockX;
    u8 month;
    int blockZ;
    int blockW;
    u8 day;
    fd = fn_80190C44(FD_ID_TOWN);
    fgMngProc_collectFg56Units();
    dTimeStamp_c today(dTime_c::getCurrent());
    today.toDayStart();
    month = today.get().month;
    day = today.get().mday;
    if ((month == MONTH_DECEMBER && day >= 15) || (month == MONTH_JANUARY && day <= 3)) {
        blockW = fd->mBlockW - 2;
        for (blockZ = 0; blockZ < FG_CEDAR_BLOCK_Z_NUM; blockZ++) {
            for (blockX = 0; blockX < blockW; blockX++) {
                growFg56(fd, blockX, blockZ);
            }
        }
    } else {
        revertFg56(fd);
    }
}

// 8009AC04
void dFgMngProc_c::growFg56(dFdBase_c *fd, int blockX, int blockZ) {
    int num = sFdAssess.mBlocks[blockX][blockZ].mCedarB56Num;
    if (num < FG56_BLOCK_MAX) {
        int bx = blockX + 1;
        int bz = blockZ + 1;
        DTCandBkUnit_c cand(fd);
        cand.clear();
        cand.mBlockX = bx;
        cand.mBlockZ = bz;
        cand.search();
        int max = cand.mCount;
        if (max > FG56_BLOCK_MAX) {
            max = FG56_BLOCK_MAX;
        }
        for (; num < max; num++) {
            int x, z;
            if (cand.getRandomXZ(&x, &z)) {
                cand.remove(x + z * cand.mWidth);
                int unitX = (bx << 4) + x;
                int unitZ = (bz << 4) + z;
                setUnitItem(fd, unitX, unitZ, dItem::FG_CEDAR_LIGHTS, FALSE);
                fgMngProc_addFg56Unit(unitX, unitZ);
            }
        }
    }
}

// 8009AD4C
void dFgMngProc_c::revertFg56(dFdBase_c *fd) {
    dFdAsPos_c *pos = fgMngProc_getFg56Units();
    for (int i = 0; i < FG56_UNIT_NUM; i++, pos++) {
        dItem::Item *item = fd->getItem(pos->mX, pos->mZ, 0);
        if (item != NULL && item->mId == dItem::FG_CEDAR_LIGHTS) {
            setUnitItem(fd, pos->mX, pos->mZ, dItem::FG_CEDAR, FALSE);
        }
    }
    fgMngProc_clearFg56Units();
}

// 8009ADE4
BOOL LampBkSearch_c::check(int x, int z) {
    dFdBlock_c *block = mFd->getBlock(x, z);
    if (fn_800812C8(block->mType) == 0xC || fn_800812C8(block->mType) == 0xF ||
        fn_800812C8(block->mType) == 0xB || fn_800812C8(block->mType) == 0x2D ||
        (fn_80081324(block->mType) & 0x0FE00000)) {
        return FALSE;
    }
    return TRUE;
}

// 8009AE70
BOOL LampUtSearch_c::check(int x, int z) {
    int unitX = (mBlockX << 4) + x;
    int unitZ = (mBlockZ << 4) + z;
    if (dFgMngProc_c::isUnitFree(unitX, unitZ)) {
        dItem::Item *item = mFd->getItem(unitX, unitZ, 0);
        if (item == NULL || item->mId != dItem::ITEM_ID_NONE) {
            return FALSE;
        }
        return mFd->canPutItem(unitX, unitZ);
    }
    return FALSE;
}

// 8009AF1C: buries the (empty) lamp on a random unit.
static inline void seedRandomAt(const dTime_c &time, int seed) {
    fgMngProc_seedRandom(&time, seed);
}
BOOL dFgMngProc_c::buryLamp(dFdBase_c *fd) {
    int seed = (int)dSaveData_c::getTown()->mTownChecksum;
    seedRandomAt(fgMngProc_getLastDayTime(), seed + 0x2233);
    LampBkSearch_c blocks(fd);
    LampUtSearch_c units(fd);
    int x, z;
    BOOL buried = FALSE;
    dFdAsPos_c block;
    dFdAsPos_c *pos = &block;
    blocks.clear();
    blocks.search();
    blocks.removeBorder();
    while (blocks.getNthXZ(fgMngProc_rndF(blocks.mCount), &pos->mX, &pos->mZ)) {
        units.clear();
        units.mBlockZ = pos->mZ;
        units.mBlockX = pos->mX;
        units.search();
        if (units.getNthXZ(fgMngProc_rndF(units.mCount), &x, &z)) {
            dItem::Item lamp(dItem::ITEM_IDX_EMPTY_LAMP);
            setUnitItem(fd, pos->mX, pos->mZ, x, z, lamp.mId, FALSE);
            buried = TRUE;
            break;
        }
        blocks.remove(pos->mX + pos->mZ * blocks.mWidth);
    }
    return buried;
}

// 8009B13C: removes the lamps from a field.
void dFgMngProc_c::removeLamps(dFdBase_c *fd) {
    int x;
    int z;
    int unitW;
    int unitH;
    unitW = fd->mBlockW * UT_X_NUM;
    unitH = fd->mBlockH * UT_Z_NUM;
    for (z = 0; z < unitH; z++) {
        for (x = 0; x < unitW; x++) {
            dItem::Item *item = fd->getItem(x, z, 0);
            if (item != NULL) {
                dItem::Item lamp(dItem::ITEM_IDX_EMPTY_LAMP);
                if (item->mId == lamp.mId) {
                    setUnitItem(fd, x, z, dItem::ITEM_ID_NONE, FALSE);
                }
            }
        }
    }
}

// 8009B200: one of 8 directions (0 = +z, counterclockwise in 0x2000 steps) for an angle.
int dFgMngProc_c::getDirection(int angle) {
    int dir = 4;
    if (angle <= -0x7556) {
        dir = 4;
    } else if (angle <= -0x4AAA) {
        dir = 5;
    } else if (angle <= -0x3556) {
        dir = 6;
    } else if (angle <= -0xAAA) {
        dir = 7;
    } else if (angle <= 0xAAA) {
        dir = 0;
    } else if (angle <= 0x3556) {
        dir = 1;
    } else if (angle <= 0x4AAA) {
        dir = 2;
    } else if (angle <= 0x7556) {
        dir = 3;
    }
    return dir;
}

// 8009B288
void dFgMngProc_c::create() {
    sFgMngState.init();
    sFgMngTaskList.init();
    fgMngProc_clearBusy();
    sFgMngProc.setLastTime(0, 0);
    fgMngProc_initOnCreate();
}

// 8009B2DC: per-frame update between create/destroy: updates the stone state and sends the
// pending ant/fly requests (fgMngProc_attractInsects); called by d_s_stage's execute just before
// fgMngProc_updateFrame.
void dFgMngProc_c::execute() {
    sFgMngState.updateStone();
    fgMngProc_attractInsects(&sFgMngLitterFlags);
}

// 8009B30C
void dFgMngProc_c::destroy() {
    sFgMngTaskList.update();
    stopFgMngProcThread();
    sFgMngRandom = NULL;
    sFgMngProc.destroyHeap();
    sFgMngUnitMask = NULL;
    fgMngProc_setFgObjCallback(0);
}

// 8009B360: marks the units next to a blocked unit (isBlockedUnit / isPondEdgeUnit) in buf.
void dFgMngProc_c::setupUnitMask(dFdBase_c *fd, u16 *buf) {
    int unitX;
    int unitZ;
    int blockZ;
    int z;
    u16 bits;
    memset(buf, 0, FG_MNG_BUFFER_SIZE);
    sFgMngUnitMask = buf;
    bits = 0;
    for (unitX = UT_X_NUM; unitX < (FG_BLOCK_X_NUM + 1) * UT_X_NUM; unitX++) {
        unitZ = UT_Z_NUM;
        for (blockZ = 0; blockZ < FG_BLOCK_Z_NUM; blockZ++) {
            for (z = 0; z < UT_Z_NUM; z++, unitZ++) {
                if (isBlockedUnit(fd, unitX, unitZ) == 1 || isPondEdgeUnit(fd, unitX, unitZ) == 1) {
                    if (z >= 2) {
                        bits |= 3 << (z - 2);
                    } else if (z == 1) {
                        bits |= 1;
                        if (blockZ != 0) {
                            buf[-1] |= 0x8000;
                        }
                    } else if (blockZ != 0) {
                        buf[-1] |= 0xC000;
                    }
                }
            }
            *buf = bits;
            bits = 0;
            buf++;
        }
    }
}

// 8009B47C: the 6:00 day-change pass over the whole save (events, animals, players, then
// processDays or the same-day path); arg = also run procLiveDay; called by the thread work
// (TRUE) and d_s_stage's create (FALSE) when the day is not up to date.
void fgMngProc_procDayChange(BOOL arg) {
    dRandom_c rnd(0x9D);
    sFgMngRandom = &rnd;
    dSaveTown_c *save = dSaveData_c::getTown();
    dTime_c now = *dTime_c::getCurrent();
    dTime_c last = fgMngProc_getLastDayTime();
    dTime_c today = now;
    today.add(0, -TIME_DAY_START_HOUR, 0, 0);
    today.hour = TIME_DAY_START_HOUR;
    today.usec = 0;
    today.msec = 0;
    today.sec = 0;
    today.min = 0;
    int days = dTime_c::diffDays(&today, &last, TRUE);
    if (days != 0) {
        dSaveData_c::getTown()->setNewConstruction(NEW_CONSTRUCTION_NONE, 0, 0);
        fn_8014F248(&dSaveData_c::getTown()->_0640C8, days);
        dFgMngProc_c::clearFg94();
    }
    dEvent::updateSchedule(today, days != 0);
    if (days != 0) {
        fn_800EC7C8(&today);
    }
    fn_80151CBC(dSaveData_c::getTown()->_0632F0);
    dSaveData_c::getTown()->mBugOff.update();
    fn_8014FD48(&dSaveData_c::getTown()->_05EC64, last, &today, &days);
    if (days != 0) {
        dPrivateData_c::fn_8013B474(dSaveData_c::getTown()->mPlayers, (int)&now);
    }
    save->mAnimals.updateMoves(days);
    save->mAnimals.mTown.updateDaily(days);
    BOOL newDay = days != 0;
    save->mAnimals.mTown.tryStartLostItem(newDay);
    save->mAnimals.mTown.updateOutdoorAnimals(newDay);
    save->mAnimals.mTown.sendBirthdayHostPresent();
    save->mAnimals.mTown.syncHouses();
    save->mAnimals.mTown.updateAnimalPlaces();
    save->mAnimals.mTown.resetDailyTalkCounts();
    fn_800EFCCC();
    fn_800EFD48();
    fn_800EFDD4();
    dPrivateData_c::fn_8013BC38(days);
    sFgMngProc.updateFg56();
    fn_80151CF0(dSaveData_c::getTown()->_0632F0);
    dSaveData_c::getTown()->mBugOff.checkDay();
    dSaveData_c::getTown()->mShops.processDays(days);
    if (days != 0) {
        dSvAuc_c::get()->update(5, dSvAuc_c::sToday, NULL);
    }
    if (days >= 1) {
        sFgMngProc.processDays(&last, &today, days, dSaveData_c::getRaw()->isFlag(6), arg);
    } else {
        sFgMngProc.createHeap();
        u16 *buf = (u16 *)sFgMngProc.mBuffer;
        dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
        dFgMngProc_c::setupUnitMask(fd, buf);
        if (arg) {
            sFgMngProc.procLiveDay(1);
        }
        if (days < 0) {
            sFgMngProc.spoilAllKabu();
            dSaveData_c::getTown()->clearFlag(0x12);
            dSaveData_c::getTown()->clearFlag(0x13);
            dSaveData_c::getTown()->clearFlag(0x19);
            fn_80169C38();
        }
        if (fn_8014D844(dSaveData_c::getTown()->_068372, EVENT_BUNNY_DAY) >= 0) {
            sFgMngProc.updateEggs();
        }
        sFgMngUnitMask = 0;
    }
    if (days != 0) {
        dSaveData_c::getTown()->clearFlag(3);
        dSaveData_c::getTown()->clearFlag(0xF);
        dSaveData_c::getTown()->clearFlag(0x10);
        fn_800F5AE8();
        dSaveData_c::getTown()->clearFlag(0x14);
        dSaveData_c::getTown()->clearFlag(4);
        dSaveData_c::getTown()->clearFlag(5);
        fn_80150524(dSaveData_c::getTown()->_06673C);
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player != NULL && player->mPID.isValid()) {
                player->_55CE.clear();
            }
        }
        fn_801505E4(&dSaveData_c::getTown()->_066740[1]);
        fn_80150628(&dSaveData_c::getTown()->_066740[5]);
        fn_801510A0(dSaveData_c::getTown()->_072CC0);
        ((dPersonalID_c *)&dSaveData_c::getTown()->_072CC0[0x2E])->clear();
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player != NULL && player->mPID.isValid()) {
                player->_83F9 = 0;
            }
        }
        u16 arg2 = 0;
        fn_800DD5F8(0x2A, &arg2, 0);
        save->clearFlag(6);
        if (sFgMngProcFlags & FG_MNG_PROC_FLAG_SAVE_FLAG6) {
            dSaveData_c::getTown()->setFlag(6);
        }
        fn_80190C44(FD_ID_TOWN)->clearWatered();
        dSaveData_c::getTown()->mTownHost.decrease(days);
        dSaveData_c::getTown()->mMuseum.sendCompleteMail();
        fn_8015384C(&dSaveData_c::getTown()->_0735AE);
    }
    dSaveData_c::getExtra()->mTheater.update();
    fn_801541D8(dSaveData_c::getTown()->_0735B7);
    dSaveData_c::getTown()->mShops.mStalkMarket.checkDate();
    ((dTimeStamp_c *)save->_068372)->set(OSCalendarTimeToTicks(&today));
    sFgMngProcFlags = 0;
    int lastWeekday = dTime_c::getWeekday(last.year, last.month, last.mday);
    int weekday = dTime_c::getWeekday(today.year, today.month, today.mday);
    if (days < 0 || days >= TIME_DAYS_PER_WEEK || weekday - lastWeekday < 0) {
        dSaveData_c::getTown()->mShops.decideStalkPrices();
    }
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
        if (player != NULL && player->mPID.isValid()) {
            int flag = i + 0x1B;
            if (player->isFlag0(0x28)) {
                if (days != 0) {
                    dSaveData_c::getTown()->setFlag(flag);
                }
            } else if (dSaveData_c::getTown()->isFlag(flag)) {
                dSaveData_c::getTown()->clearFlag(flag);
                memset((u8 *)player + 0x865F, 0, 0x10);
            }
            player->dailyUpdate(days);
        }
    }
    if (fn_8014D844(dSaveData_c::getTown()->_068372, EVENT_BUNNY_DAY) >= 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL && !player->isFlag0(0x28)) {
            player->setFlag1(0x43);
        }
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        if (!fn_80101558()) {
            fn_8010156C();
            fn_801002BC();
        }
        if (player->isFlag0(0x28)) {
            player->fn_80137CD0(days);
        }
    }
    if (days != 0) {
        fn_8014F998(&dSaveData_c::getTown()->_05EC64, &today);
    }
    fn_80186838();
    u32 v = dSaveData_c::getTown()->mShops.mShop.mStage;
    fn_8014F030(&dSaveData_c::getTown()->_0640C8, v);
    dSaveTown_c *town = dSaveData_c::getTown();
    fn_8014EFC0(&town->_0640C8, fgMngProc_assessLiveTown());
    sFgMngRandom = NULL;
    if (days != 0) {
        fn_800ECC78();
    }
    dTime_c::resetOffset();
}

// 8046F098
static const f32 sShellWeights[] = {2.0f, 3.0f, 5.0f, 10.0f, 10.0f, 5.0f, 10.0f, 25.0f, 30.0f, 0.0f};

// 8009BCDC: per-frame from d_s_stage's execute: sets up the shell rates once, skips while locked
// (fgMngProc_blockDayChange) or in busy scenes, notices 6:00 passing during play (flag 2, spawns
// an actor at the player), then runs updateShellSpawn.
// The shell weight loop is unrolled x3 in the target, as MWCC does with loop unrolling off.
#pragma push
#pragma opt_unroll_loops off
void fgMngProc_updateFrame() {
    if (sDayChangeBlocked == 1) {
        return;
    }
    if (dSvMgr_c::isFullTransferComplete()) {
        return;
    }
    if (!sShellHostIO.mInit) {
        sShellHostIO.mInit = TRUE;
        sShellHostIO.mWeightTotal = 0.0f;
        for (int i = 0; i < FG_SHELL_KIND_NUM; i++) {
            sShellHostIO.mWeight[i] = sShellWeights[i];
            sShellHostIO.mWeightTotal += sShellWeights[i];
        }
    }
    if (!fgMngProc_canCheckDayChange()) {
        return;
    }
    if (isSceneAttr(getCurrentScene(), SCENE_ATTR_ROOM)) {
        sFgMngProc.updateShellSpawn();
        return;
    }
    if (!fn_800DCEDC() && fgMngProc_getElapsedDays() >= 1) {
        sFgMngProcFlags |= FG_MNG_PROC_FLAG_DAY_CHANGE;
        if (fn_8018EEEC()) {
            fn_8018EEF4();
            BOOL inside = isCityScene(getCurrentScene());
            fn_80091AF8()->setReturnExitHere(0.0f);
            nw4r::math::VEC3 pos;
            if (inside) {
                nw4r::math::VEC3 *p = fn_8016A28C();
                f32 z = p->z;
                f32 y = p->y;
                f32 x = p->x;
                nw4r::math::VEC3 top(x, y, z);
                top.z += 64.0f;
                fn_80091AF8()->request(SCENE_TOWN, (mVec3_c *)&top, 0x8C, 0, 0, 0);
            } else {
                dItem::Item item((u16)0xD013);
                int x, z;
                dSaveBuildingList_c::get()->getPos(&x, &z, &item, 1);
                dFdBase_c::getUnitCenterPos(&pos, x, z);
                fn_800755A0(1);
                pos.y = fn_80073510(&pos);
                fn_800755A0(0);
                fn_80091AF8()->request(SCENE_FIELD, (mVec3_c *)&pos, 0x8C, 0, 0, 0);
            }
        } else {
            fn_8018EE7C();
            return;
        }
    }
    sFgMngProc.updateShellSpawn();
}

#pragma pop

// 8009BEF0: body of the FG manager background thread (run by fgMngProcThreadFunc): assesses the
// live blocks, refills the beach shells, records fg 0x56 units, spoils turnips if the clock went
// back, then does the day change.
void fgMngProc_threadMain() {
    fgMngSync_reset();
    dSaveData_c::getTown();
    dTime_c last = fgMngProc_getLastDayTime();
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    fgMngProc_updateFrame();
    if (fd != NULL) {
        int w = fd->mBlockW - 2;
        int h = fd->mBlockH - 2;
        for (int x = 0; x < w; x++) {
            for (int z = 0; z < h; z++) {
                sFdAssess.getBlock(x, z)->assessLive(fd, x + 1, z + 1);
            }
        }
        dRandom_c rnd(0x9D);
        sFgMngRandom = &rnd;
        fgMngProc_seedRandom(&last, (int)dSaveData_c::getTown()->mTownChecksum + 0x2220);
        sFgMngProc.fillBeachShells(fd);
        sFgMngRandom = NULL;
    }
    fgMngProc_collectFg56Units();
    if (!fn_800DCEDC()) {
        if (fgMngProc_isClockSetBack()) {
            sFgMngProc.spoilAllKabu();
            fn_800D16E8();
            dSaveData_c::getTown()->clearFlag(0x12);
            dSaveData_c::getTown()->clearFlag(0x13);
        }
        switch (getCurrentScene()) {
        case SCENE_NONE:
            sFgMngProc.procLiveDay(1);
            break;
        default:
            fgMngProc_procDayChange(TRUE);
            break;
        }
    }
    fgMngState_clearUnitFlags(&sFgMngState);
    sFgMngTaskList.init();
    sFgMngState.mStone.reset();
}

// 8009C0CC: counts fg 0x56 per block in the first two block rows (mCedarB56Num) and records
// their units in the fgMngProc_getFg56Units list; used by updateFg56 and the thread.
void fgMngProc_collectFg56Units() {
    dFdBase_c *fd;
    dFdAsPos_c *pos;
    int x;
    int z;
    int w;
    fd = fn_80190C44(FD_ID_TOWN);
    fgMngProc_clearFg56Units();
    w = fd->mBlockW - 2;
    pos = fgMngProc_getFg56Units();
    for (z = 0; z < FG_CEDAR_BLOCK_Z_NUM; z++) {
        for (x = 0; x < w; x++) {
            dItem::Item *items = fd->getItem(x + 1, z + 1, 0, 0, 0);
            int ux;
            int uz;
            int num = 0;
            for (uz = 0; uz < UT_Z_NUM; uz++) {
                for (ux = 0; ux < UT_X_NUM; ux++) {
                    if (items->mId == dItem::FG_CEDAR_LIGHTS) {
                        num++;
                        pos->mX = ((x + 1) << 4) + ux;
                        pos->mZ = ((z + 1) << 4) + uz;
                        pos++;
                    }
                    items++;
                }
            }
            dFdAsBlock_c *block = sFdAssess.getBlock(x, z);
            block->mCedarB56Num = num;
        }
    }
}

// 8009C244: one-off field setup with the date-seeded random: removes trees in water, assesses,
// buries fossils / pitfalls, plants trees, changes stones, stamps _05EC64; only caller
// fn_8014D27C (likely new-town creation).
void fgMngProc_initTownField() {
    int salt = (int)dSaveData_c::getTown()->mTownChecksum;
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd != NULL) {
        int w = fd->mBlockW - 2;
        int h = fd->mBlockH - 2;
        sFgMngProc.removeTreesInWater(fd, w, h);
        sFgMngProc.createHeap();
        dFgMngProc_c::setupUnitMask(fd, (u16 *)sFgMngProc.mBuffer);
        dSaveData_c::getTown();
        dTime_c last = fgMngProc_getLastDayTime();
        dRandom_c rnd(0x9D);
        sFgMngRandom = &rnd;
        sFdAssess.assessTown(fd, w, h);
        fgMngProc_seedRandom(&last, salt + 0x2CCA81C);
        sFgMngProc.buryFossils(fd, w, h);
        fgMngProc_seedRandom(&last, salt + 0x2CCA81D);
        sFgMngProc.buryPitfall(fd, w, h);
        fgMngProc_seedRandom(&last, salt + 0x2CCA819);
        sFgMngProc.plantTrees(fd, w, h);
        fgMngProc_seedRandom(&last, salt + 0x2CCA81E);
        sFgMngProc.changeStones(fd, w, h);
        fgMngProc_resetUnitState();
        fn_8014F96C(&dSaveData_c::getTown()->_05EC64);
        fn_8014F998(&dSaveData_c::getTown()->_05EC64, &last);
        sFgMngUnitMask = 0;
        sFgMngRandom = NULL;
    }
}

// 8009C41C: called by dFgMngProc_c::create: assesses the live town, latches the litter
// flags, sets the task list busy, and clears fg 0x94 in the home town when the player has no
// house yet.
void fgMngProc_initOnCreate() {
    fgMngProc_assessLiveTown();
    fgMngProc_getLitterFlags(&sFgMngLitterFlags);
    fgMngTask_setBusy();
    if (!fn_800DCEDC() && dPlayerMgr_c::getCurrentPlayer() != NULL) {
        dLandID_c *town = &dSaveData_c::getTown()->mLandID;
        BOOL home = dPlayerMgr_c::getCurrentPlayer()->mPID.land == *town;
        if (home && isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
            dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
            if (homes->getHome(homes->findCurrentPlayer()) == NULL && isSceneAttr(getPrevScene(), SCENE_ATTR_PLAYER_HOUSE)) {
                fgMngProc_clearFg94();
            }
        }
    }
}

// 8009C520: wrapper for sFdAssess.assessLiveTown(); used by the day change, fn_8003B3B4 and
// Pelly (d_a_npc_sp_periko).
int fgMngProc_assessLiveTown() {
    return sFdAssess.assessLiveTown();
}

// 8009C52C: returns &sFdAssess.mWorst (lowest-rated block and its main fault); only caller Pelly
// (d_a_npc_sp_periko).
dFdAsWorst_c *fgMngProc_getWorstBlock() {
    return &sFdAssess.mWorst;
}

// 8009C53C: re-assesses the live town and returns its weed count (sFdAssess.mGrassNum); caller
// d_a_npc_sp_yuutaro_loft.
u16 fgMngProc_getWeedNum() {
    fgMngProc_assessLiveTown();
    return sFdAssess.mGrassNum;
}

// 8009C568: TRUE offline, else whether net member `member` is this console (fn_800DCF58); used
// by d_fgobj_manager and the task code.
BOOL fgMngProc_isSelfMember(int member) {
    return !fn_800DCEDC() ? TRUE : fn_800DCF58() == member;
}

// 8009C5B4: TRUE when offline, the player lacks flag0 0xD, the FG manager is not busy and town
// bit _05EC74&1 is clear; gate used by d_fgobj_manager and the task code (hedged name).
BOOL fgMngProc_canEditField() {
    BOOL ok = FALSE;
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (!fn_800DCEDC() && !player->isFlag0(0xD) && sFgMngProc.mBusy == 0 &&
        !(*(u16 *)&dSaveData_c::getTown()->_05EC68[0xC] & 1)) {
        ok = TRUE;
    }
    return ok;
}

// 8009C638: item id held by net member `member` (ITEM_ID_NONE without a player).
u16 fgMngProc_getMemberHeldItem(int member) {
    u16 id = dItem::ITEM_ID_NONE;
    dPrivateData_c *player = dPlayerMgr_c::getNetPlayer(member);
    if (player != NULL) {
        id = player->mEquipment.mHeld.mId;
    }
    return id;
}

static inline BOOL isMyHeld(int index) {
    u16 id = dItem::Item(index).mId;
    BOOL diff = id != fgMngProc_getMemberHeldItem(fn_800DCF58());
    return !diff;
}

// 8009C678: with the golden shovel held, buried money becomes fg 0x11 and, by a price/luck-based
// chance (doubled when _83F9 == 3), fg 0x49 (FG_MONEY_TREE_SAPLING).
void fgMngProc_getBuriedMoneyFg(u16 *outFg, u8 *outFlag, u16 itemId) {
    if (!isMyHeld(dItem::ITEM_IDX_GOLDEN_SHOVEL)) {
        return;
    }
    dItem::Item item(itemId);
    if (!item.isMoney()) {
        return;
    }
    *outFg = dItem::FG_TREE_SAPLING;
    *outFlag = 0;
    if (fn_801017B8() >= 4) {
        return;
    }
    dSaveTown_c *save = dSaveData_c::getTown();
    if (!fn_8014D740(save->_068372)) {
        return;
    }
    u8 v = *fn_800AC28C(4);
    f32 rate = (10.0f * (4.0f * v) + item.getPrice()) / 1000.0f;
    if (dPlayerMgr_c::getCurrentPlayer()->_83F9 == 3) {
        rate *= 2.0f;
    }
    if (rate > 100.0f) {
        rate = 100.0f;
    }
    if (cM::rndF(100.0f) < rate) {
        *outFg = dItem::FG_MONEY_TREE_SAPLING;
    }
    fn_8014D69C(save->_068372);
}

// 8009C7FC: field object (and base fg) an item plants as, by BITM kind (saplings, flower
// seeds/bags, etc.); FALSE if the item cannot be planted; used by fgMngProc_getBuryFg and
// d_fgobj_manager.
BOOL fgMngProc_getPlantedFg(u16 *outFg, u16 *outBase, u16 itemId, void *obj) {
    dItem::Item item(itemId);
    BOOL ok = TRUE;
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    if (bitm == NULL) {
        return FALSE;
    }
    *outFg = item.getPlantedFg();
    switch (bitm->getKind()) {
    case dItem::KIND_FRUIT:
        *outBase = *outFg;
        break;
    case dItem::KIND_SEEDLING:
        *outBase = *outFg;
        fn_800C77E8(obj);
        break;
    case dItem::KIND_SEED:
        *outBase = dItem::FG_SEED;
        if (item.mId >= dItem::Item(dItem::ITEM_IDX_RED_CARN_BAG).mId) {
            *outFg = ((item.mId - dItem::Item(dItem::ITEM_IDX_RED_CARN_BAG).mId) >> 2) + dItem::FG_CARNATION_RED;
        } else {
            *outFg = ((item.mId - dItem::Item(dItem::ITEM_IDX_RED_TULIP_BAG).mId) >> 2) + dItem::FG_TULIP_RED;
        }
        fn_800C7800(obj);
        break;
    case dItem::KIND_RKABU_SEED:
        *outBase = dItem::FG_SEED;
        *outFg = dItem::FG_RED_TURNIP_0;
        fn_800C7830(obj);
        break;
    case dItem::KIND_RKABU:
        *outBase = item.mId;
        *outFg = ((item.mId - dItem::Item(dItem::ITEM_IDX_RED_TURNIP_00).mId) >> 2) + dItem::FG_RED_TURNIP_0;
        break;
    case dItem::KIND_FLOWER: {
        u16 id = item.mId;
        *outBase = dItem::FG_SEED;
        if (id == dItem::Item(dItem::ITEM_IDX_LUCKY_CLOVER).mId) {
            *outFg = dItem::FG_LUCKY_CLOVER;
        } else if (id == dItem::Item(dItem::ITEM_IDX_DANDELIONS).mId) {
            *outFg = dItem::FG_DANDELION;
        } else if (id == dItem::Item(dItem::ITEM_IDX_DANDELION_PUFFS).mId) {
            *outFg = dItem::FG_DANDELION_PUFF;
        } else {
            *outFg = ((id - dItem::Item(dItem::ITEM_IDX_RED_TULIPS).mId) >> 2) + dItem::FG_TULIP_RED;
        }
        break;
    }
    default:
        ok = FALSE;
        break;
    }
    return ok;
}

// 8009CA58: fg produced by putting `item` into the ground: the planted form if plantable
// (outFlag FALSE), else buried (outFlag TRUE, money via fgMngProc_getBuriedMoneyFg); caller
// d_fgobj_manager.
void fgMngProc_getBuryFg(void *obj, u16 *outFg, u16 *outBase, u8 *outFlag, u16 item) {
    *outFg = item;
    *outFlag = TRUE;
    if (fgMngProc_getPlantedFg(outFg, outBase, item, obj)) {
        *outFlag = FALSE;
    } else {
        fgMngProc_getBuriedMoneyFg(outFg, outFlag, item);
    }
}

// 8009CAE0: returns sFgMngProc.mBusy; caller fn_8005E888.
u8 fgMngProc_isBusy() {
    return sFgMngProc.mBusy;
}

// 8009CAEC: sFgMngProc.mBusy = TRUE; caller d_fgobj_manager.
void fgMngProc_setBusy() {
    sFgMngProc.mBusy = TRUE;
}

// 8009CAFC: sFgMngProc.mBusy = FALSE; called by dFgMngProc_c::create and d_insect_field.
void fgMngProc_clearBusy() {
    sFgMngProc.mBusy = FALSE;
}

// 8009CB0C: offline only: clears the per-unit state flags and removes all fg 0x94 (clearFg94);
// called at the end of fgMngProc_initTownField.
void fgMngProc_resetUnitState() {
    if (!fn_800DCEDC()) {
        fgMngState_clearUnitFlags(&sFgMngState);
        dFgMngProc_c::clearFg94();
    }
}

static inline BOOL isHeld(int member, int index) {
    u16 id = dItem::Item(index).mId;
    return id - fgMngProc_getMemberHeldItem(member) == 0;
}

// 8046F1C0 / 8046F1CC: 3 x 3 around the unit; 807506C8 / 807506D0: the unit and its 4 neighbours.
static const s8 sAroundX9[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
static const s8 sAroundZ9[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
static const s8 sAroundX5[] = {0, -1, 1, 0, 0};
static const s8 sAroundZ5[] = {0, 0, 0, -1, 1};

// 8009CB44: watering from the task code: sets flagB on the 5 units around pos (3x3 with the
// golden/silver can); golden can turns fg 0xD6 into 0xD8, silver can fg 0x95 into 0x9D.
void fgMngProc_waterAround(int member, const dFdAsPos_c *pos) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    dPlayerActor_c *player = fn_800FBC7C(4);
    const s8 *dx;
    const s8 *dz;
    BOOL a;
    BOOL b;
    int i;
    int num;
    if (fd == NULL || player == NULL) {
        return;
    }
    a = FALSE;
    b = FALSE;
    if (isHeld(member, dItem::ITEM_IDX_GOLDEN_CAN)) {
        a = TRUE;
    } else if (isHeld(member, dItem::ITEM_IDX_SILVER_CAN)) {
        b = TRUE;
    }
    if (a || b) {
        dx = sAroundX9;
        dz = sAroundZ9;
        num = 9;
    } else {
        dx = sAroundX5;
        dz = sAroundZ5;
        num = 5;
    }

    for (i = 0; i < num; i++) {
        dFdAsPos_c unit(pos->mX + *dx, pos->mZ + *dz);
        if (fgMngTask_find(0, &unit, 0) < 0) {
            dItem::Item *item = fd->getItem(unit.mX, unit.mZ, 0);
            if (item != NULL && sFgMngProc.isFlowerOrRedKabu(item)) {
                if (item->mId == dItem::FG_WILTED_ROSE_BLACK && a == TRUE) {
                    dFgMngProc_c::setUnitItem(fd, unit.mX, unit.mZ, dItem::FG_WILTED_ROSE_GOLD, 0);
                } else if (item->mId == dItem::FG_RED_TURNIP_WILTED && b == TRUE) {
                    dFgMngProc_c::setUnitItem(fd, unit.mX, unit.mZ, dItem::FG_RED_TURNIP_WILTED_WATERED, 0);
                }
            }
        }
        fd->setWatered(unit.mX, unit.mZ);
        dx++;
        dz++;
    }
}

// 8009CD3C: returns the 30-entry dFdAsPos_c list of fg 0x56 units (sFg56Units).
dFdAsPos_c *fgMngProc_getFg56Units() {
    return (dFdAsPos_c *)sFg56Units;
}

// 8009CD48: stores (x, z) in the first free (-1, -1) slot of the fg 0x56 unit list.
void fgMngProc_addFg56Unit(int x, int z) {
    dFdAsPos_c *list = fgMngProc_getFg56Units();
    for (int i = 0; i < FG56_UNIT_NUM; i++) {
        if (list->mX == -1 && list->mZ == -1) {
            list->mX = x;
            list->mZ = z;
            break;
        }
        list++;
    }
}

// 8009CE70: sets all 30 entries of the fg 0x56 unit list to (-1, -1).
void fgMngProc_clearFg56Units() {
    dFdAsPos_c *list = fgMngProc_getFg56Units();
    for (int i = 0; i < FG56_UNIT_NUM; i++) {
        list[i].mX = -1;
        list[i].mZ = -1;
    }
}

// 8009CF84: TRUE while the save time mSaveTime lies after now (clock moved back); the thread
// then spoils all turnips.
BOOL fgMngProc_isClockSetBack() {
    BOOL ret = FALSE;
    dSaveTown_c *save = dSaveData_c::getTown();
    dTime_c now = *dTime_c::getCurrent();
    if (!save->mSaveTime.isNone()) {
        const dTime_c &t = save->mSaveTime.get();
        if (dTime_c::diffTicks(&now, &t, TRUE) < 0) {
            ret = TRUE;
        }
    }
    return ret;
}

// 8009D06C: world position of the rafflesia found by the assessment; FALSE if none; caller
// d_insect_field (flies).
BOOL fgMngProc_getRafflesiaPos(nw4r::math::VEC3 *pos) {
    BOOL ret = FALSE;
    dFdAsPos_c block = sFdAssess.getRafflesiaBlock0();
    dFdAsPos_c unit = sFdAssess.getRafflesiaUnit0();
    if (block.mX != -1 && block.mZ != -1 && unit.mX != -1 && unit.mZ != -1) {
        dFdBase_c::getUnitPos(pos, block.mX + 1, block.mZ + 1, unit.mX, unit.mZ);
        ret = TRUE;
    }
    return ret;
}

// 8009D0E8: from d_a_player: damages the flower at unit (x, z) with petals (mode 1) or, 12.5%,
// destroys it (mode 0); returns the mode (3 = nothing / not outdoors).
int fgMngProc_trampleFlowerAt(int x, int z) {
    if (!isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
        return 3;
    }
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    int mode = 3;
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            f32 r = cM::rndF(100.0f);
            mode = 1;
            if (r < 12.5f) {
                mode = 0;
            }
            fgMngProc_damageFlower(item, x, z, mode);
        }
    }
    return mode;
}

// 8009D1B4: if item is a flower: plays its falling-petal effect and, for mode 0, removes it
// (synced via fgMngProc_setUnitFgSync).
void fgMngProc_damageFlower(dItem::Item *item, int x, int z, int mode) {
    if (item->isAnyFlower()) {
        fgMngProc_playFlowerFallEffect(item, x, z, mode, NULL, FALSE);
        if (mode == 0) {
            fgMngProc_setUnitFgSync(x, z, dItem::ITEM_ID_NONE);
        }
    }
}

// 8009D248: fgMngProc_damageFlower on the town item at unit (x, z); caller d_fgobj_manager.
void fgMngProc_damageFlowerAt(int x, int z, int mode) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    if (fd != NULL) {
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL) {
            fgMngProc_damageFlower(item, x, z, mode);
        }
    }
}

// 804E1D40..804E22F8: the falling-flower effects (fgMngProc_playFlowerFallEffect), by flower kind
// (getFlowerKind).
static const char *sTulipFall[] = {
    "afm_tulip_fall_rd_st", "afm_tulip_fall_wt_st", "afm_tulip_fall_ye_st",
    "afm_tulip_fall_pi_st", "afm_tulip_fall_vi_st", "afm_tulip_fall_bk_st",
};
static const char *sPansyFall[] = {
    "afm_pansy_fall_wt_st", "afm_pansy_fall_ye_st", "afm_pansy_fall_rd_st",
    "afm_pansy_fall_vi_st", "afm_pansy_fall_ry_st", "afm_pansy_fall_bl_st",
};
static const char *sCosmosFall[] = {
    "afm_cosmos_fall_wt_st", "afm_cosmos_fall_rd_st", "afm_cosmos_fall_ye_st",
    "afm_cosmos_fall_pi_st", "afm_cosmos_fall_or_st", "afm_cosmos_fall_bk_st",
};
static const char *sRoseFall[] = {
    "afm_rose_fall_rd_st", "afm_rose_fall_wt_st", "afm_rose_fall_ye_st",
    "afm_rose_fall_pi_st", "afm_rose_fall_or_st", "afm_rose_fall_vi_st",
    "afm_rose_fall_bk_st", "afm_rose_fall_bl_st", "afm_rose_fall_gl_st",
};
static const char *sCarnaFall[] = {"afm_carna_fall_rd_st", "afm_carna_fall_pi_st", "afm_carna_fall_wt_st"};
static const char *sLilyFall[] = {"afm_lily_fall_st"};
static const char *sDandeFall[] = {"afm_dande_fall_st"};
static const char *sDandeDownFall[] = {"afm_dande_down_fall_st"};
static const char **sFallTable[] = {
    sTulipFall, sPansyFall, sCosmosFall, sRoseFall, sCarnaFall, sLilyFall, sDandeFall, sDandeDownFall,
};
static const char *sLeafFallTable[] = {
    "afm_tulip_leaf_fall_st", "afm_pansy_leaf_fall_st", "afm_cosmos_leaf_fall_st", "afm_rose_leaf_fall_st",
    "afm_carna_leaf_fall_st", "afm_lily_leaf_fall_st", "afm_dande_leaf_fall_st", "afm_dande_leaf_fall_st",
};
static const char *sWiltFallTable[FLOWER_KIND_NUM] = {
    "afm_tulip_fall_x_st", "afm_pansy_fall_x_st", "afm_cosmos_fall_x_st",
    "afm_rose_fall_x_st", "afm_carna_fall_x_st", "afm_lily_fall_x_st",
};
static const char *sWiltLeafFallTable[FLOWER_KIND_NUM] = {
    "afm_tulip_leaf_x_fall_st", "afm_pansy_leaf_x_fall_st", "afm_cosmos_leaf_x_fall_st",
    "afm_rose_leaf_x_fall_st", "afm_carna_leaf_x_fall_st", "afm_lily_leaf_x_fall_st",
};
// 8046F1D8: the first fg id of each flower kind.
static const u16 sFlowerFirstFg[] = {dItem::FG_TULIP_RED, dItem::FG_PANSY_WHITE, dItem::FG_COSMOS_WHITE, dItem::FG_ROSE_RED, dItem::FG_CARNATION_RED, dItem::FG_JACOBS_LADDER, dItem::FG_DANDELION, dItem::FG_DANDELION_PUFF};

// 8009D2C0: spawns the afm_*_fall / *_leaf_fall effects for the flower at unit (x, z) (wilted
// variants, wind direction, scale by mode).
void fgMngProc_playFlowerFallEffect(dItem::Item *item, int x, int z, int mode, const mAng3_c *ang, BOOL wind) {
    if (!isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
        return;
    }
    int kind = getFlowerKind(item);
    u16 id = item->mId;
    mVec3_c pos;
    fgMngProc_getUnitGroundPos(&pos, x, z);
    f32 scale;
    if (mode == 1) {
        scale = 1.0f;
    } else {
        scale = 3.0f;
    }
    dEffect_c petals;
    dEffect_c leaves;
    static mVec3_c sSpread(10.0f, 1.0f, 10.0f);
    mVec3_c dir;
    if (wind) {
        dir.set(-0.5f, 0.0f, 0.0f);
    } else {
        dir.set(0.25f, 0.0f, 0.0f);
    }
    u32 tilt = fn_8008299C(&pos) >> 16;
    if (ang != NULL) {
        dir.rotY(ang->y);
        dir.rotX(mAng((s16)tilt));
    }
    const char *name;
    if (item->isWiltedFlower()) {
        name = sWiltFallTable[kind];
    } else {
        name = sFallTable[kind][id - sFlowerFirstFg[kind]];
    }
    if (fn_80087820(&petals, name, &pos, ang, NULL)) {
        petals.vf2C(scale * fn_8044B83C(&petals));
        if (ang != NULL) {
            petals.vf48(&dir);
            if (!inRange(item, dItem::FG_JACOBS_LADDER, dItem::FG_JACOBS_LADDER) && item->mId != dItem::FG_DANDELION_PUFF) {
                petals.vf6C(&sSpread, 0);
            }
            if (wind) {
                if (item->mId == dItem::FG_DANDELION_PUFF) {
                    petals.vf3C(0.8f);
                } else {
                    petals.vf3C(2.0f);
                }
            }
        }
    }
    BOOL ok;
    if (item->mId == dItem::FG_ROSE_GOLD) {
        ok = fn_80087820(&leaves, "afm_rose_leaf_fall_gl_st", &pos, ang, NULL);
    } else {
        if (item->isWiltedFlower()) {
            name = sWiltLeafFallTable[kind];
        } else {
            name = sLeafFallTable[kind];
        }
        ok = fn_80087820(&leaves, name, &pos, ang, NULL);
    }
    if (ok) {
        leaves.vf2C(scale * fn_8044B83C(&leaves));
        if (ang != NULL) {
            if (!inRange(item, dItem::FG_JACOBS_LADDER, dItem::FG_JACOBS_LADDER)) {
                leaves.vf6C(&sSpread, 0);
            }
            leaves.vf48(&dir);
            if (wind) {
                leaves.vf3C(2.0f);
            }
        }
    }
}

// Result of the ground check fn_8006E1BC.
struct dFgMngGroundCheck_c {
    /* 0x00 */ u8 _00[0x30];
    /* 0x30 */ int _30;
    /* 0x34 */ int mAttr;
    /* 0x38 */ u8 _38[0x54];
}; // size 0x8C

// 8009D68C: ground check at the unit's position: TRUE for attribute 0x17 or result _30 == 2;
// caller d_fg_draw (hedged: meaning of the attribute unknown).
BOOL fgMngProc_isUnitSpecialGround(int x, int z) {
    nw4r::math::VEC3 pos;
    fgMngProc_getUnitGroundPos(&pos, x, z);
    dFgMngGroundCheck_c check;
    fn_8006E1BC(&check, &pos, 0, 0, 0);
    if (check.mAttr == 0x17 || check._30 == 2) {
        return TRUE;
    }
    return FALSE;
}

static inline dTimeStamp_c getLastStamp() {
    return *(dTimeStamp_c *)dSaveData_c::getTown()->_068372;
}

static inline BOOL isUpToDate(dTimeStamp_c stamp) {
    if (stamp.isNone()) {
        return TRUE;
    }
    dTime_c now = *dTime_c::getCurrent();
    now.add(0, -TIME_DAY_START_HOUR, 0, 0);
    dTime_c t = stamp.get();
    return dTime_c::diffDays(&now, &t, TRUE) <= 0;
}

// 8009D6F4: TRUE when the last day-change stamp (_068372) is unset or not before today 6:00;
// d_s_stage runs fgMngProc_procDayChange when FALSE.
BOOL fgMngProc_isDayUpToDate() {
    return isUpToDate(getLastStamp());
}

// 8009D874: sets the callback sFgObjCallback (installed by d_fgobj_manager, cleared by
// dFgMngProc_c::destroy).
void fgMngProc_setFgObjCallback(dFgObjCallback func) {
    sFgObjCallback = func;
}

// 8009D87C: calls the d_fgobj_manager callback (if set) with a zero angle; caller fn_80192128.
void fgMngProc_callFgObjCallback(int a, int b, int c, int d) {
    if (sFgObjCallback != NULL) {
        mAng3_c ang(0, 0, 0);
        sFgObjCallback(a, b, c, &ang, d);
    }
}

// 8807506EC / 8046F040: the units in front of the player (by facing).
static const s16 sFrontAngle[] = {0, 0x1800, -0x1800};
static const u8 sFrontUnit[] = {
    FG_UNIT_OFS(0, 1), FG_UNIT_OFS(1, 1), FG_UNIT_OFS(1, 1), FG_UNIT_OFS(1, 0),
    FG_UNIT_OFS(1, 0), FG_UNIT_OFS(1, -1), FG_UNIT_OFS(1, -1), FG_UNIT_OFS(0, -1),
    FG_UNIT_OFS(0, -1), FG_UNIT_OFS(-1, -1), FG_UNIT_OFS(-1, -1), FG_UNIT_OFS(-1, 0),
    FG_UNIT_OFS(-1, 0), FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(-1, 1), FG_UNIT_OFS(0, 1),
};

// 8009D8C4: with a shovel (normal/silver/golden) held, finds a fg 0x94 unit (likely a dug hole)
// in front of the player not under a task or another player; caller the item menu
// (d_menu_item00).
BOOL fgMngProc_findHoleInFront(int *outX, int *outZ) {
    dFdAsPos_c pu;
    int angle;
    int i;
    BOOL ok = TRUE;
    BOOL ok2 = TRUE;
    dFdBase_c *fd;
    int x, z;
    u16 held;
    int attr;
    held = fgMngProc_getMemberHeldItem(fn_800DCF58());
    if (held != dItem::Item(dItem::ITEM_IDX_SHOVEL).mId) {
        if (held != dItem::Item(dItem::ITEM_IDX_SILVER_SHOVEL).mId) {
            ok2 = FALSE;
        }
    }
    if (!ok2) {
        if (held != dItem::Item(dItem::ITEM_IDX_GOLDEN_SHOVEL).mId) {
            ok = FALSE;
        }
    }
    if (!ok) {
        return FALSE;
    }
    dPlayerActor_c *player = fn_800FBC7C(4);
    if (player == NULL) {
        return FALSE;
    }
    mVec3_c pos = player->mPos;
    angle = player->mAngle.y;
    attr = fn_80072E94(&pos);
    pu.mX = (int)pos.x >> 5;
    pu.mZ = (int)pos.z >> 5;
    fd = fn_80190C44(FD_ID_CURRENT);
    int ax, az, bz, bx;
    dPlayerActor_c **actors = (dPlayerActor_c **)lbl_8074E9A0;
    if (actors[0] != NULL) {
        ax = (int)actors[0]->mPos.x >> 5;
        az = (int)actors[0]->mPos.z >> 5;
    } else {
        az = -1;
        ax = -1;
    }
    if (actors[1] != NULL) {
        bx = (int)actors[1]->mPos.x >> 5;
        bz = (int)actors[1]->mPos.z >> 5;
    } else {
        bz = -1;
        bx = -1;
    }
    for (i = 0; i < 3; i++) {
        u8 off = sFrontUnit[((angle + sFrontAngle[i]) >> 12) & 0xF];
        x = (off >> 4) + pu.mX - 8;
        z = (off & 0xF) + pu.mZ - 8;
        dItem::Item *item = fd->getItem(x, z, 0);
        if (item != NULL && inRange(item, dItem::FG_HOLE, dItem::FG_HOLE) && fn_800737E4(pu.mX, pu.mZ, attr, x, z) &&
            !(x == ax && z == az) && !(x == bx && z == bz)) {
            dFdAsPos_c unit;
            unit.mX = x;
            unit.mZ = z;
            int idx = fgMngTask_find(0, &unit, 0);
            if (idx < 0 || !fgMngTask_get(idx)->mActive) {
                *outX = x;
                *outZ = z;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 8009DB80: validates a field-change request from net member `member` (unit, fg, kind, type)
// against the field and running tasks; callers net receive fn_800DF7A0 and d_fgobj_manager.
BOOL fgMngProc_isChangeRequestValid(const dFgMngReq_c *req, int member) {
    BOOL ok = FALSE;
    dFdAsPos_c unit;
    unit.mX = req->mPos >> 8;
    unit.mZ = req->mPos & 0xFF;
    int idx = fgMngLock_find(member, &unit, req->mLayer);
    if (idx >= 0 && !(BOOL)fgMngTask_getActive(req->mMember)) {
        return FALSE;
    }
    dFdBase_c *fd = fn_80190C44(fn_80190C58(member));
    dItem::Item *item = fd->getItem(req->mPos >> 8, req->mPos & 0xFF, req->mLayer);
    if (item != NULL && item->mId == req->mTarget) {
        if (idx < 0) {
            if (fgMngState_isFreeOfPlayers(unit.mX, unit.mZ, req->mMember)) {
                ok = TRUE;
            }
        } else {
            int idx2 = fgMngTask_find(member, &unit, req->mLayer);
            if (idx2 >= 0) {
                dFgMngTask_c *task = fgMngTask_get(idx2);
                if (task->mPlayer == req->mMember && task->mActive && !task->mSet) {
                    ok = TRUE;
                    switch (task->mKind) {
                    case FG_MNG_KIND_PICK_UP_FULL:
                    case FG_MNG_KIND_PICK_UP_SPECIAL_FULL:
                        if (req->mKind != FG_MNG_KIND_SWAP) {
                            ok = FALSE;
                        }
                        break;
                    case FG_MNG_KIND_DIG_UP_FULL:
                        if (req->mKind != FG_MNG_KIND_BURY || req->mItem == dItem::ITEM_ID_NONE) {
                            ok = FALSE;
                        }
                        break;
                    default:
                        ok = FALSE;
                        break;
                    }
                }
            }
        }
    }
    return ok;
}

// 8009DD40: sets the town unit's fg and, online, sends it to the other consoles (packet 0x2B).
void fgMngProc_setUnitFgSync(int x, int z, u16 fg) {
    if (fn_800DCEDC()) {
        struct {
            u8 x;
            u8 z;
            u16 fg;
        } data;
        data.x = x;
        data.z = z;
        data.fg = fg;
        fn_800DD4C8();
        fn_800DD518(&data, 4);
        fn_800DD588(FG_MNG_PACKET_UNIT_FG, FG_MNG_NET_ALL);
    }
    dFgMngProc_c::setUnitItem(fn_80190C44(FD_ID_TOWN), x, z, fg, 0);
}

// 8009DDD0: applies a received 0x2B unit-fg packet (playing the flower effect when a flower is
// removed); caller net receive fn_800DF8E4.
void fgMngProc_recvUnitFg(const u16 *data) {
    dFdBase_c *fd = fn_80190C44(FD_ID_TOWN);
    u16 fg = data[1];
    dFdAsPos_c unit;
    unit.mX = data[0] >> 8;
    unit.mZ = data[0] & 0xFF;
    if (fn_800DCF90() && fgMngLock_find(0, &unit, 0) >= 0) {
        return;
    }
    if (fg == dItem::ITEM_ID_NONE) {
        dItem::Item *item = fd->getItem(unit.mX, unit.mZ, 0);
        if (item == NULL) {
            return;
        }
        if (item->isFlower() || inRange(item, dItem::FG_DANDELION, dItem::FG_DANDELION_PUFF)) {
            fgMngProc_playFlowerFallEffect(item, unit.mX, unit.mZ, 0, NULL, FALSE);
        } else {
            return;
        }
    }
    dFgMngProc_c::setUnitItem(fd, unit.mX, unit.mZ, fg, 0);
}

// 8009DEE4: FALSE while fading, in SCENE_DM_TITLE..SCENE_RM_SAMPLE / SCENE_NONE, or without the 8074E800 state 1;
// gate of fgMngProc_updateFrame.
BOOL fgMngProc_canCheckDayChange() {
    BOOL fading = TRUE;
    if (!mFader_c::isStatus(mFaderBase_c::FADE_IN) && !mFader_c::isStatus(mFaderBase_c::FADE_OUT)) {
        fading = FALSE;
    }
    if (fading) {
        return FALSE;
    }
    switch (getCurrentScene()) {
    case SCENE_NONE:
    case SCENE_DM_TITLE:
    case SCENE_DM_SAVE:
    case SCENE_DM_LOAD:
    case SCENE_DM_PL_SEL:
    case SCENE_DM_BUS_PL_CRT:
    case SCENE_DM_BUS_TO_TOWN:
    case SCENE_DM_BUS_TO_LAND:
    case SCENE_DM_CHKP_CNNCT:
    case SCENE_DM_CHKP_DCNNCT:
    case SCENE_DM_CHKP_MISS:
    case SCENE_CHECK_FIELD:
    case SCENE_RM_SAMPLE:
        return FALSE;
    }
    if (lbl_8074E800 == NULL || lbl_8074E800[4] != 1) {
        return FALSE;
    }
    return TRUE;
}

// 8009DFBC: sets sDayChangeBlocked = 1 so fgMngProc_updateFrame does nothing; used by special NPCs'
// events (resetsan, seiichi, perio, op_tanukichi, golditem_soncho, npc_out).
void fgMngProc_blockDayChange() {
    fn_800DCEDC();
    sDayChangeBlocked = 1;
}

// 8009DFE4: clears sDayChangeBlocked (pairs with fgMngProc_blockDayChange).
void fgMngProc_unblockDayChange() {
    sDayChangeBlocked = 0;
}

// 8009DFF0: the last processed day (save stamp _068372) at 6:00 as dTime_c; also used by
// fn_801442F0 / fn_8014435C / fn_80144548.
dTime_c fgMngProc_getLastDayTime() {
    dTimeStamp_c stamp = *(dTimeStamp_c *)dSaveData_c::getTown()->_068372;
    dTime_c t;
    OSTicksToCalendarTime(stamp.getTicks(), &t);
    t.hour = TIME_DAY_START_HOUR;
    t.usec = 0;
    t.msec = 0;
    t.sec = 0;
    t.min = 0;
    return t;
}

// 8009E0D4: sets save flag 6 directly, or defers it (sFgMngProcFlags & 1) while a day change
// is pending/running; caller fn_801C91CC.
void fgMngProc_setSaveFlag6() {
    u8 flags = sFgMngProcFlags;
    if (!(flags & (FG_MNG_PROC_FLAG_DAY_CHANGE | FG_MNG_PROC_FLAG_THREAD))) {
        dSaveData_c::getTown()->setFlag(6);
    } else if (flags & FG_MNG_PROC_FLAG_DAY_CHANGE) {
        sFgMngProcFlags = flags | FG_MNG_PROC_FLAG_SAVE_FLAG6;
    }
}

// 8009E11C: save flag 6, or the deferred bit while a day change is pending/running; caller
// fn_801C91CC.
BOOL fgMngProc_isSaveFlag6() {
    u8 flags = sFgMngProcFlags;
    return !(flags & (FG_MNG_PROC_FLAG_DAY_CHANGE | FG_MNG_PROC_FLAG_THREAD)) ? dSaveData_c::getRaw()->isFlag(6) : flags & FG_MNG_PROC_FLAG_SAVE_FLAG6;
}

// 8009E158: wrapper for dFgMngProc_c::clearFg94 (removes all fg 0x94); callers d_save_data and
// the 80154080 TU.
void fgMngProc_clearFg94() {
    dFgMngProc_c::clearFg94();
}

// 8009E15C: ground position of town unit (x, z) (dFdBase_c::getUnitGroundPos with fn_800755A0
// set around it).
void fgMngProc_getUnitGroundPos(nw4r::math::VEC3 *pos, int x, int z) {
    fn_800755A0(1);
    dFdBase_c::getUnitGroundPos(pos, x, z);
    fn_800755A0(0);
}

// 8009E1BC: copies the assessment's has-trash / has-spoiled-turnip / has-candy bits into the
// 3-byte flag set.
void fgMngProc_getLitterFlags(dFgMngLitterFlags_c *flags) {
    if (sFdAssess.mFlags.mBits.mHasDust) {
        flags->mHasDust = TRUE;
    } else {
        flags->mHasDust = FALSE;
    }
    if (sFdAssess.mFlags.mBits.mHasBadKabu) {
        flags->mHasBadKabu = TRUE;
    } else {
        flags->mHasBadKabu = FALSE;
    }
    if (sFdAssess.mFlags.mBits.mHasCandy) {
        flags->mHasCandy = TRUE;
    } else {
        flags->mHasCandy = FALSE;
    }
}

// 8009E238: offline outdoors: turnips/candy attract ants, turnips/trash flies (flags
// d_insect_field reads; likely ants / flies), then clears the flags.
void fgMngProc_attractInsects(dFgMngLitterFlags_c *flags) {
    if (!fn_800DCEDC() && isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
        if (flags->mHasBadKabu || flags->mHasCandy) {
            dInsectInfo::setAntsAttracted();
        }
        if (flags->mHasBadKabu || flags->mHasDust) {
            dInsectInfo::setFliesAttracted();
        }
        flags->mHasDust = FALSE;
        flags->mHasBadKabu = FALSE;
        flags->mHasCandy = FALSE;
    }
}

// 8009E2CC: re-assesses the live town and re-latches the litter flags; caller
// d_insect_field.
void fgMngProc_updateLitterFlags() {
    fgMngProc_assessLiveTown();
    fgMngProc_getLitterFlags(&sFgMngLitterFlags);
}

// 8009E2F4: seeds the day-change random (sFgMngRandom) from a date and a salt.
void fgMngProc_seedRandom(const dTime_c *time, int salt) {
    sFgMngRandom->initFromDateAndLandId(*time, salt);
}

// 8009E308: random float below max from the day-change random when set, else cM::rndF; used
// throughout the day change and by the 800CCC54 TU.
f32 fgMngProc_rndF(f32 max) {
    if (sFgMngRandom == NULL) {
        return cM::rndF(max);
    }
    return sFgMngRandom->rndF(max);
}

// 8009E31C: remembers save flag 0x1A in sFgMngProc.mSaveFlag; paired with fgMngProc_stashFg94 in
// fn_80084B88 / fn_80084F18.
void fgMngProc_backupSaveFlag1A() {
    sFgMngProc.mSaveFlag = dSaveData_c::getTown()->isFlag(0x1A);
}

// 8009E350: restores save flag 0x1A from sFgMngProc.mSaveFlag; in fn_80084CA4 / fn_80084FA0.
void fgMngProc_restoreSaveFlag1A() {
    if (sFgMngProc.mSaveFlag) {
        dSaveData_c::getTown()->setFlag(0x1A);
    } else {
        dSaveData_c::getTown()->clearFlag(0x1A);
    }
}

// 8009E39C: removes every 0x94 field object of the town (border blocks excluded), remembering its
// units in a bit grid in sFgMngProc's buffer (one u16 row per unit z of each block).
void fgMngProc_stashFg94() {
    u16 *bits;
    dFdBase_c *fd;
    int blockW;
    int blockH;
    int blockX;
    int blockZ;
    int unitX;
    int unitZ;
    sFgMngProc.createHeap();
    bits = (u16 *)sFgMngProc.mBuffer;
    memset(bits, 0, FG_MNG_BUFFER_SIZE);
    fd = fn_80190C44(FD_ID_TOWN);
    blockW = fd->mBlockW - 2;
    blockH = fd->mBlockH - 2;
    for (blockZ = 1; blockZ <= blockH; blockZ++) {
        for (blockX = 1; blockX <= blockW; blockX++) {
            for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
                for (unitX = 0; unitX < UT_X_NUM; unitX++) {
                    dItem::Item *item = fd->getItem(blockX, blockZ, unitX, unitZ, 0);
                    if (item != NULL && inRange(item, dItem::FG_HOLE, dItem::FG_HOLE)) {
                        *bits |= 1 << unitX;
                        dFgMngProc_c::setUnitItem(fd, blockX, blockZ, unitX, unitZ, dItem::ITEM_ID_NONE, 0);
                    }
                }
                bits++;
            }
        }
    }
}

// 8009E4D8: puts back the fg 0x94 units remembered by fgMngProc_stashFg94; callers fn_80084CA4 /
// fn_80084FA0.
void fgMngProc_restoreFg94() {
    dFdBase_c *fd;
    u16 *bits;
    int blockW;
    int blockH;
    int blockX;
    int blockZ;
    int unitX;
    int unitZ;
    fd = fn_80190C44(FD_ID_TOWN);
    bits = (u16 *)sFgMngProc.mBuffer;
    blockW = fd->mBlockW - 2;
    blockH = fd->mBlockH - 2;
    for (blockZ = 1; blockZ <= blockH; blockZ++) {
        for (blockX = 1; blockX <= blockW; blockX++) {
            for (unitZ = 0; unitZ < UT_Z_NUM; unitZ++) {
                for (unitX = 0; unitX < UT_X_NUM; unitX++) {
                    if (*bits & (1 << unitX)) {
                        dFgMngProc_c::setUnitItem(fd, blockX, blockZ, unitX, unitZ, dItem::FG_HOLE, 0);
                    }
                }
                bits++;
            }
        }
    }
}
