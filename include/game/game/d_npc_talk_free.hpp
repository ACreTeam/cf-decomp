#pragma once

// d_npc_talk_free.cpp (.text 80039B38..8003F354, .rodata 8046C1C0..8046C630, .data 804A1F20..804A2308,
// .sdata2 8074FFB0..80750060). Not decompiled yet. A villager's free conversation ("free talk"): the
// topic group is picked by weight (fn_80039E38 over the 10 group selects of 8046C5A0), then a topic
// of the group:
//   FreeA (nml getMsgLabel(0xF, 0, i)): Clothes, Always, Dress, Friend, Memory, Rumor, Jinx
//   A2    (0xF, 1, i): special days 0229, 0401, Weather, MoonJPN / MoonKOR / MoonUSA / MoonEUR
//   FreeH (0xF, 2, i): coming events Harvest, Halloween, Countdown, Fmarket, Xmas, Easter
//   FreeB (0xF, 3, i): the villager's hobby Bug, Fassion, Fish, Fossil, Gardening, Interior, Party, Hint
//   ApC   (0xF, 4, i): item offers Present, Sell, Trade, Want (procedures of d_npc_talk_approach)
//   FreeD (0xF, 5, 0): Moving2 (villager moving out)
//   FreeE (0xF, 6, i): Event (next town event), Snpc (special npc in town)
//   FreeF (0xF, 7, i): Building, House1, House2, Inside (where the villager stands)
//   FreeG (0xF, 8, i): Host, Visitor, JinxV (online play)
//   FreeI (0xF, 9, i): Tunekichi (K.K. letter)
// and the "Etc_Hit" / "Etc_Push" / "Etc_Flea" lines (records of nml's table 804A0784).
// The label tables 804A1F84 .. 804A2220 of this TU's .data are read by nml getMsgLabel only.
//
// Its functions are talk procedures stored in dAcNpcNml_c::talk_c's member-function pointers. Slots, as
// seen from the d_a_npc_nml helpers (provisional):
//   EC   message select (setMsgProc / setProcSet / setProcs): fills the msgInfo_s through
//        setLooksMsg (label, code) and returns TRUE when it picked a message.
//   F8   hook called once by fn_800313B0 (setHookProc / setProcSet): no result.
//   104  wait/step procedure (setStepProc / setProcs, called by fn_8003209C): BOOL; most only act
//        once the message controller is idle (Rcpt_c::mpController+0x6C84 == 0).
//   140  after-talk procedure (stored directly at talk_c+0x140, called with no argument).
// Most topic selects also record the topic with nml setTopic(this, 2, group, idx).
// Condition procedures of the PTMF tables 8046C1C0 / 8046C4F8 take no argument and return BOOL.
// Parameter and return types are inferred from the asm; unused incoming registers are left out.

#include <types.h>
#include <game/game/d_a_npc_nml.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_event.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_personal_id.hpp>

// The functions of this TU are members of dAcNpcNml_c::talk_c (d_a_npc_nml.hpp, section
// "d_npc_talk_free"): its procedures are stored in talk_c's member-function pointers.

// Data of the TU (file-local statics in the .cpp):
//   8046C1C0 .rodata 0x58  PTMF table, 7 x {0,-1,fn} (+4 pad): fn_80039B38, fn_80039B7C, fn_80039BAC,
//                          fn_80039C34, fn_80039C64, fn_80039C94, fn_80039CC4 (fn_80039CF4)
//   8046C218 .rodata 0x78  dQuestEvent_e[30] (fn_80039D7C)
//   8046C290 .rodata 0x90  dQuestEvent_e[5] {0x1D, 0x2B, 0x23, 0x27, 0x2C} (fn_80039D7C), then
//                          10 rows of 12 bytes (10 x u8 weights + pad) for 8046C5A0, returned by
//                          fn_80039E38 (8046C2C8 .. 8046C310); dtk merged them into one object
//   8046C320 .rodata 0x30  function pointers [12]: NULL, fn_8003AD10 .. fn_8003AF74 (fn_8003B174)
//   8046C350 .rodata 0x54  PTMF table, 7 x {0,-1,fn}: fn_8003A3DC, fn_8003A814, fn_8003A998,
//                          fn_8003AB40, fn_8003B174, fn_8003B3B4, fn_8003B960 (FreeA, fn_8003BA84)
//   8046C3A4 .rodata 0x54  PTMF table, 7 x {0,-1,fn}: fn_8003BBC8, fn_8003BC48, fn_8003BE24,
//                          fn_8003BF70, fn_8003C01C, fn_8003C0C8, fn_8003C174 (A2, fn_8003C220)
//   8046C3F8 .rodata 0x30  PTMF table, 4 x {0,-1,fn}: fn_8003CAB8, fn_8003CB58, fn_8003CBF8,
//                          fn_8003CC98 (ApC, fn_8003CE58)
//   8046C428 .rodata 0x18  6 x {u16 npc item code (0x80xx), u16 message code} (fn_8003D5C0, fn_801AD174)
//   8046C440 .rodata 0x28  10 x {u16 npc item code, u16 message code} (fn_8003D5C0, fn_801AD304)
//                          (dtk shows some pairs of both tables as bogus relocations)
//   8046C468 .rodata 0x18  PTMF table, 2 x {0,-1,fn}: fn_8003D180, fn_8003D5C0 (fn_8003D710)
//   8046C480 .rodata 0x18  PTMF table, 2 x {0,-1,fn}: fn_8003BA84, fn_8003D5C0 (fn_8003D710)
//   8046C498 .rodata 0x3C  5 x {dItem::Item building (0xD015, 0xD016, 0xD013, 0xD017, 0xD041), pad,
//                          s32 xMax adjust, s32 xMin adjust} (fn_8003D894)
//   8046C4D4 .rodata 0x24  PTMF table, 3 x {0,-1,fn}: fn_8003DCDC, fn_8003DE3C, fn_8003DEF8 (FreeG)
//   8046C4F8 .rodata 0x48  PTMF table, 6 x {0,-1,fn}: fn_8003E13C, fn_8003E184, fn_8003E1CC,
//                          fn_8003E214, fn_8003E26C, fn_8003E2B4 (FreeH conditions, fn_8003E308)
//   8046C540 .rodata 0x48  PTMF table, 6 x {0,-1,fn}: fn_8003E3F0, fn_8003E708, fn_8003E49C,
//                          fn_8003E544, fn_8003E5C4, fn_8003E688 (FreeH topics, fn_8003E7B4)
//   8046C588 .rodata 0x18  PTMF table, 2 x {0,-1,fn}: fn_8003E838, fn_8003E994 (FreeI, fn_8003EA5C)
//   8046C5A0 .rodata 0x78  PTMF table, 10 x {0,-1,fn}: fn_8003BA84, fn_8003C220, fn_8003E7B4,
//                          fn_8003C9A0, fn_8003CE58, fn_8003CF40, fn_8003D710, fn_8003D894,
//                          fn_8003DFE8, fn_8003EA5C (groups, fn_8003EC4C)
//   8046C618 .rodata 0x15  strings "Etc_Push", "Etc_Flea"
//   804A1F20 .data   0x64  strings FreeA_Clothes .. FreeA_Jinx; 804A1F84 const char *[7] of them
//   804A1FA0 .data   0x68  strings FreeA_0229 .. FreeA_MoonEUR; 804A2008 const char *[7] of them
//   804A2024 .data   0x74  strings FreeB_Bug .. FreeB_Hint; 804A2098 const char *[8] of them
//   804A20B8 .data   0x30  strings ApC_Present .. ApC_Want; 804A20E8 const char *[4] of them
//   804A20F8 .data   0x28  strings "FreeD_Moving2", "FreeE_Event", "FreeE_Snpc" (pointed to by .sdata)
//   804A2120 .data   0x40  strings FreeF_Building .. FreeF_Inside; 804A2160 const char *[4] of them
//   804A2170 .data   0x28  strings FreeG_Host, FreeG_Visitor, FreeG_JinxV; 804A2198 const char *[3]
//   804A21A4 .data   0x64  strings FreeH_Harvest .. FreeH_Easter; 804A2208 const char *[6] of them
//   804A2220 .data   0x10  string "FreeI_Tunekichi" (pointed to by .sdata)
//                          (all label tables above are read by nml getMsgLabel(0xF, group, idx))
//   804A2230 .data   0xC   PTMF {0,-1,fn_8003F184}
//   804A223C .data   0x2B  strings "sys_STRING/STR_Unit", "sys_STRING/STR_Impress"
//   804A2268 .data   0xC   PTMF {0,-1,fn_8003EE44}
//   804A2274 .data   0x1C  switch jump table of fn_8003C9A0 (7 hobbies)
//   804A2290 .data   0xC   PTMF {0,-1,fn_80037B58}   804A229C .. fn_80037BE0
//   804A22A8 .data   0xC   PTMF {0,-1,fn_80037E10}   804A22B4 .. fn_80037F1C
//   804A22C0 .data   0xC   PTMF {0,-1,fn_800363A4}
//   804A22CC .data   0xC   PTMF {0,-1,fn_8003EE44}   804A22D8 .data 0xC PTMF {0,-1,fn_8003E8E4}
//   804A22E4 .data   0xC   PTMF {0,-1,fn_8003EF14}   804A22F0 .data 0xC PTMF {0,-1,fn_8003EFF8}
//   804A22FC .data   0xC   PTMF {0,-1,fn_8003F0A0}
//                          (the single PTMFs are local copies of member-function pointer constants)
//   8074FFB0 .sdata2 0x10  100.0f, int->float magic double
//   8074FFC0 .sdata2 0x58  11 x u8[8] FreeA weight rows (fn_8003A284)
//   80750018 .sdata2 0xC   3.0f, 50.0f, 70.0f
//   80750024 .sdata2 0x18  u8[4] / u8[2] ApC weight rows (fn_8003CD38; dtk shows 80750034 as "d")
//   8075003C .sdata2 0xC   u8 FreeE weight rows (fn_8003D04C; "22" = {50, 50})
//   80750048 .sdata2 0x8   u8 FreeG weight rows (fn_8003DC3C)
//   80750050 .sdata2 0x5   u8[5] {80, 70, 60, 50, 0}: FreeI chance by player count (fn_8003EA5C; "PF<2")
//   80750058 .sdata2 0x8   string "Etc_Hit"
//   80749978 .sdata  0x18  const char *[2] {FreeD_Moving2, NULL}, {FreeE_Event, FreeE_Snpc},
//                          {FreeI_Tunekichi x2} (nml getMsgLabel groups 5, 6, 9); in the unsplit
//                          .sdata (auto_09_80749908_sdata), probably this TU's
//   80749990 .sdata  0xA0  dQuestEvent_e literals bound to const references (dEvent calls):
//                          anonymous temporaries, not source objects (see d_event.hpp)

// Message label tables of this TU read by dAcNpcNml_c::talk_c::getMsgLabel (globals; quest
// descriptors are typed by their label part only).
extern const char *l_freeALabels[7]; // "FreeA_Clothes", "FreeA_Always", "FreeA_Dress", "FreeA_Friend", "FreeA_Memory", "FreeA_Rumor", "FreeA_Jinx"
extern const char *l_freeASeasonLabels[7]; // "FreeA_0229", "FreeA_0401", "FreeA_Weather", "FreeA_MoonJPN" / KOR / USA / EUR
extern const char *l_freeBLabels[8]; // "FreeB_Bug", "FreeB_Fassion", "FreeB_Fish", ... "FreeB_Hint"
extern const char *l_apcLabels[4]; // "ApC_Present", "ApC_Sell", "ApC_Trade", "ApC_Want"
extern const char *l_freeDLabels[2]; // "FreeD_Moving2" (.sdata)
extern const char *l_freeELabels[2]; // "FreeE_Event", "FreeE_Snpc" (.sdata)
extern const char *l_freeFLabels[4]; // "FreeF_Building", "FreeF_House1", "FreeF_House2", "FreeF_Inside"
extern const char *l_freeGLabels[3]; // "FreeG_Host", "FreeG_Visitor", "FreeG_JinxV"
extern const char *l_freeHLabels[6]; // "FreeH_Harvest", "FreeH_Halloween", ... "FreeH_Easter"
extern const char *l_freeILabels[2]; // "FreeI_Tunekichi" x2 (.sdata)
