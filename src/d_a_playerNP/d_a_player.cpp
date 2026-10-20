#include <game/mLib/m_3d/face_ex.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/framework/f_profile.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_fgobj_manager.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_assessment.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_ftr.hpp>
#include <game/game/d_ground_attr.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mngr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_shadow.hpp>
#include <game/game/d_uki.hpp>
#include <game/game/d_weather.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/egg/math/eggMath.h>
#include <cmath>
#include <game/cLib/c_lib.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/game/d_a_snowman.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_catalog.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_ftr_manager.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_obj_bal.hpp>
#include <game/game/d_play_util.hpp>
#include <game/game/d_str.hpp>
#include <lib/nw4r/math/math_arithmetic.h>
#include <game/game/d_a_str.hpp>
#include <game/game/d_activity_log.hpp>
#include <game/game/d_clock.hpp>
#include <game/game/d_insect_field.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_fg_mng_task.hpp>
#include <cstdlib>
#include <game/game/d_fg_draw.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_menu_mng.hpp>
#include <game/game/d_pad.hpp>
#include <game/mLib/m_pad.hpp>
#include <game/game/d_bg_attr.hpp>
#include <game/game/d_cockroach_manager.hpp>
#include <game/game/d_a_rmobj_tel.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_hand_item.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_npc_info.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_hmn_face_tex_mng.hpp>
#include <game/framework/f_profile_name.hpp>
#include <lib/nw4r/ef/ef_particle.h>
#include <lib/nw4r/ef/ef_particlemanager.h>
#include <lib/nw4r/math/math_triangular.h>
#include <game/game/d_date.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_hmn_cloth_mng.hpp>
#include <lib/revolution/OS/OSCache.h>
#include <game/game/d_bg.hpp>
#include <game/game/d_light.hpp>
#include <game/game/d_mii_icon.hpp>
#include <game/game/d_str_manager.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <lib/egg/gfxe/eggFog.h>
#include <lib/egg/math/eggMatrix.h>
#include <nw4r/g3d/g3d_state.h>
#include <nw4r/ut/ut_Color.h>
#include <nw4r/g3d/res/g3d_restev.h>
#include <revolution/GX.h>
#include <revolution/RFL.h>

namespace dAPlayer {
// .text 0x5C
static dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}
// .text 0x68
BASE_PROFILE(A_PLAYER, dAcPlayer_c);
} // namespace dAPlayer

// .data 0x8: the id of the actor the player talks to (-1 = none).
static fBaseID_e l_talkActorID = (fBaseID_e)-1;

// .data 0xC: the Mii mask expression tracks, one per dHmnAnm_c::getSubIdx index (l_faceExprTracks).
// Some objects are followed by a zero word in the target (alignment or a longer array, unverified).
static dAcPlayer_c::faceExprKey_s l_faceExpr00[] = {{0, 0}, {21, 5}, {27, 4}, {54, 0}, {-1, 0}}; // .data 0xC
static dAcPlayer_c::faceExprKey_s l_faceExpr01[] = {{0, 0}, {21, 4}, {60, 0}, {-1, 0}}; // .data 0x20
static dAcPlayer_c::faceExprKey_s l_faceExpr02[] = {{0, 0}, {21, 5}, {27, 4}, {50, 5}, {81, 4}, {-1, 0}}; // .data 0x30
static dAcPlayer_c::faceExprKey_s l_faceExpr03[] = {{0, 3}, {-1, 0}}; // .data 0x48
static dAcPlayer_c::faceExprKey_s l_faceExpr04[] = {{0, 0}, {6, 1}, {76, 0}, {-1, 0}}; // .data 0x50
static dAcPlayer_c::faceExprKey_s l_faceExpr05[] = {{0, 0}, {39, 3}, {-1, 0}}; // .data 0x60 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr06[] = {{0, 0}, {6, 4}, {27, 5}, {36, 4}, {63, 0}, {-1, 0}}; // .data 0x70
static dAcPlayer_c::faceExprKey_s l_faceExpr07[] = {{0, 0}, {9, 1}, {72, 5}, {78, 0}, {96, 1}, {114, 0}, {-1, 0}}; // .data 0x88 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr08[] = {{0, 0}, {4, 5}, {10, 0}, {44, 5}, {58, 0}, {-1, 0}}; // .data 0xA8
static dAcPlayer_c::faceExprKey_s l_faceExpr09[] = {{0, 0}, {69, 5}, {-1, 0}}; // .data 0xC0
static dAcPlayer_c::faceExprKey_s l_faceExpr0A[] = {{0, 0}, {69, 5}, {-1, 0}}; // .data 0xCC
static dAcPlayer_c::faceExprKey_s l_faceExpr0B[] = {{0, 5}, {-1, 0}}; // .data 0xD8
static dAcPlayer_c::faceExprKey_s l_faceExpr0C[] = {{0, 5}, {-1, 0}}; // .data 0xE0
static dAcPlayer_c::faceExprKey_s l_faceExpr0D[] = {{0, 5}, {-1, 0}}; // .data 0xE8
static dAcPlayer_c::faceExprKey_s l_faceExpr0E[] = {{0, 5}, {-1, 0}}; // .data 0xF0
static dAcPlayer_c::faceExprKey_s l_faceExpr0F[] = {{0, 5}, {-1, 0}}; // .data 0xF8
static dAcPlayer_c::faceExprKey_s l_faceExpr10[] = {{0, 5}, {21, 4}, {42, 5}, {-1, 0}}; // .data 0x100
static dAcPlayer_c::faceExprKey_s l_faceExpr11[] = {{0, 5}, {21, 4}, {42, 5}, {-1, 0}}; // .data 0x110
static dAcPlayer_c::faceExprKey_s l_faceExpr12[] = {{0, 5}, {6, 0}, {-1, 0}}; // .data 0x120
static dAcPlayer_c::faceExprKey_s l_faceExpr13[] = {{0, 5}, {6, 0}, {-1, 0}}; // .data 0x12C
static dAcPlayer_c::faceExprKey_s l_faceExpr14[] = {{0, 0}, {12, 5}, {18, 4}, {46, 0}, {-1, 0}}; // .data 0x138 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr15[] = {{0, 0}, {14, 4}, {55, 0}, {-1, 0}}; // .data 0x150
static dAcPlayer_c::faceExprKey_s l_faceExpr16[] = {{0, 0}, {4, 5}, {10, 0}, {45, 5}, {59, 0}, {-1, 0}}; // .data 0x160
static dAcPlayer_c::faceExprKey_s l_faceExpr17[] = {{0, 0}, {63, 5}, {75, 1}, {114, 0}, {-1, 0}}; // .data 0x178 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr18[] = {{0, 5}, {-1, 0}}; // .data 0x190
static dAcPlayer_c::faceExprKey_s l_faceExpr19[] = {{0, 5}, {-1, 0}}; // .data 0x198
static dAcPlayer_c::faceExprKey_s l_faceExpr1A[] = {{0, 5}, {-1, 0}}; // .data 0x1A0
static dAcPlayer_c::faceExprKey_s l_faceExpr1B[] = {{0, 0}, {9, 4}, {36, 3}, {-1, 0}}; // .data 0x1A8
static dAcPlayer_c::faceExprKey_s l_faceExpr1C[] = {{0, 3}, {45, 5}, {54, 0}, {-1, 0}}; // .data 0x1B8
static dAcPlayer_c::faceExprKey_s l_faceExpr1D[] = {{0, 0}, {9, 4}, {36, 3}, {-1, 0}}; // .data 0x1C8
static dAcPlayer_c::faceExprKey_s l_faceExpr1E[] = {{0, 3}, {45, 5}, {54, 0}, {-1, 0}}; // .data 0x1D8
static dAcPlayer_c::faceExprKey_s l_faceExpr1F[] = {{0, 3}, {45, 5}, {54, 0}, {-1, 0}}; // .data 0x1E8
static dAcPlayer_c::faceExprKey_s l_faceExpr20[] = {{0, 4}, {-1, 0}}; // .data 0x1F8
static dAcPlayer_c::faceExprKey_s l_faceExpr21[] = {{0, 4}, {24, 5}, {36, 3}, {-1, 0}}; // .data 0x200
static dAcPlayer_c::faceExprKey_s l_faceExpr22[] = {{0, 4}, {-1, 0}}; // .data 0x210
static dAcPlayer_c::faceExprKey_s l_faceExpr23[] = {{0, 4}, {12, 2}, {-1, 0}}; // .data 0x218
static dAcPlayer_c::faceExprKey_s l_faceExpr24[] = {{0, 4}, {12, 2}, {-1, 0}}; // .data 0x224
static dAcPlayer_c::faceExprKey_s l_faceExpr25[] = {{0, 0}, {3, 4}, {9, 5}, {60, 3}, {99, 5}, {-1, 0}}; // .data 0x230
static dAcPlayer_c::faceExprKey_s l_faceExpr26[] = {{0, 0}, {3, 4}, {9, 5}, {60, 3}, {99, 5}, {-1, 0}}; // .data 0x248
static dAcPlayer_c::faceExprKey_s l_faceExpr27[] = {{0, 5}, {9, 0}, {-1, 0}}; // .data 0x260 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr28[] = {{0, 0}, {9, 5}, {81, 5}, {-1, 0}}; // .data 0x270
static dAcPlayer_c::faceExprKey_s l_faceExpr29[] = {{0, 5}, {-1, 0}}; // .data 0x280
static dAcPlayer_c::faceExprKey_s l_faceExpr2A[] = {{0, 5}, {-1, 0}}; // .data 0x288
static dAcPlayer_c::faceExprKey_s l_faceExpr2B[] = {{0, 5}, {-1, 0}}; // .data 0x290
static dAcPlayer_c::faceExprKey_s l_faceExpr2C[] = {{0, 5}, {32, 3}, {-1, 0}}; // .data 0x298 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr2D[] = {{0, 0}, {6, 5}, {45, 3}, {-1, 0}}; // .data 0x2A8
static dAcPlayer_c::faceExprKey_s l_faceExpr2E[] = {{0, 3}, {-1, 0}}; // .data 0x2B8
static dAcPlayer_c::faceExprKey_s l_faceExpr2F[] = {{0, 0}, {9, 1}, {74, 5}, {78, 0}, {96, 1}, {-1, 0}}; // .data 0x2C0
static dAcPlayer_c::faceExprKey_s l_faceExpr30[] = {{0, 0}, {6, 5}, {15, 4}, {42, 0}, {-1, 0}}; // .data 0x2D8 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr31[] = {{0, 0}, {57, 1}, {108, 5}, {120, 1}, {159, 0}, {-1, 0}}; // .data 0x2F0
static dAcPlayer_c::faceExprKey_s l_faceExpr32[] = {{0, 0}, {3, 5}, {12, 0}, {-1, 0}}; // .data 0x308
static dAcPlayer_c::faceExprKey_s l_faceExpr33[] = {{0, 0}, {24, 1}, {105, 0}, {-1, 0}}; // .data 0x318
static dAcPlayer_c::faceExprKey_s l_faceExpr34[] = {{0, 0}, {6, 5}, {15, 0}, {66, 1}, {-1, 0}}; // .data 0x328 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr35[] = {{0, 1}, {-1, 0}}; // .data 0x340
static dAcPlayer_c::faceExprKey_s l_faceExpr36[] = {{0, 0}, {21, 5}, {42, 4}, {56, 1}, {84, 0}, {-1, 0}}; // .data 0x348
static dAcPlayer_c::faceExprKey_s l_faceExpr37[] = {{0, 0}, {57, 1}, {120, 5}, {203, 1}, {-1, 0}}; // .data 0x360 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr38[] = {{0, 1}, {-1, 0}}; // .data 0x378
static dAcPlayer_c::faceExprKey_s l_faceExpr39[] = {{0, 0}, {139, 5}, {264, 0}, {-1, 0}}; // .data 0x380
static dAcPlayer_c::faceExprKey_s l_faceExpr3A[] = {{0, 0}, {-1, 0}}; // .data 0x390
static dAcPlayer_c::faceExprKey_s l_faceExpr3B[] = {{0, 5}, {3, 4}, {9, 5}, {-1, 0}}; // .data 0x398
static dAcPlayer_c::faceExprKey_s l_faceExpr3C[] = {{0, 5}, {-1, 0}}; // .data 0x3A8
static dAcPlayer_c::faceExprKey_s l_faceExpr3D[] = {{0, 5}, {63, 0}, {-1, 0}}; // .data 0x3B0 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr3E[] = {{0, 0}, {4, 5}, {6, 4}, {66, 5}, {80, 3}, {-1, 0}}; // .data 0x3C0
static dAcPlayer_c::faceExprKey_s l_faceExpr3F[] = {{0, 0}, {4, 5}, {10, 0}, {44, 5}, {58, 0}, {-1, 0}}; // .data 0x3D8
static dAcPlayer_c::faceExprKey_s l_faceExpr40[] = {{0, 0}, {6, 5}, {15, 2}, {-1, 0}}; // .data 0x3F0
static dAcPlayer_c::faceExprKey_s l_faceExpr41[] = {{0, 2}, {-1, 0}}; // .data 0x400
static dAcPlayer_c::faceExprKey_s l_faceExpr42[] = {{0, 0}, {6, 5}, {9, 4}, {-1, 0}}; // .data 0x408
static dAcPlayer_c::faceExprKey_s l_faceExpr43[] = {{0, 4}, {-1, 0}}; // .data 0x418
static dAcPlayer_c::faceExprKey_s l_faceExpr44[] = {{0, 0}, {9, 5}, {15, 1}, {-1, 0}}; // .data 0x420
static dAcPlayer_c::faceExprKey_s l_faceExpr45[] = {{0, 1}, {-1, 0}}; // .data 0x430
static dAcPlayer_c::faceExprKey_s l_faceExpr46[] = {{0, 0}, {6, 5}, {-1, 0}}; // .data 0x438 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr47[] = {{0, 4}, {-1, 0}}; // .data 0x448
static dAcPlayer_c::faceExprKey_s l_faceExpr48[] = {{0, 0}, {9, 5}, {15, 2}, {-1, 0}}; // .data 0x450
static dAcPlayer_c::faceExprKey_s l_faceExpr49[] = {{0, 2}, {-1, 0}}; // .data 0x460
static dAcPlayer_c::faceExprKey_s l_faceExpr4A[] = {{0, 0}, {3, 5}, {9, 3}, {-1, 0}}; // .data 0x468
static dAcPlayer_c::faceExprKey_s l_faceExpr4B[] = {{0, 3}, {-1, 0}}; // .data 0x478
static dAcPlayer_c::faceExprKey_s l_faceExpr4C[] = {{0, 0}, {3, 5}, {6, 2}, {-1, 0}}; // .data 0x480
static dAcPlayer_c::faceExprKey_s l_faceExpr4D[] = {{0, 2}, {-1, 0}}; // .data 0x490
static dAcPlayer_c::faceExprKey_s l_faceExpr4E[] = {{0, 0}, {9, 5}, {12, 3}, {-1, 0}}; // .data 0x498
static dAcPlayer_c::faceExprKey_s l_faceExpr4F[] = {{0, 3}, {-1, 0}}; // .data 0x4A8
static dAcPlayer_c::faceExprKey_s l_faceExpr50[] = {{0, 0}, {6, 5}, {-1, 0}}; // .data 0x4B0 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr51[] = {{0, 5}, {-1, 0}}; // .data 0x4C0
static dAcPlayer_c::faceExprKey_s l_faceExpr52[] = {{0, 0}, {6, 5}, {12, 1}, {-1, 0}}; // .data 0x4C8
static dAcPlayer_c::faceExprKey_s l_faceExpr53[] = {{0, 1}, {-1, 0}}; // .data 0x4D8
static dAcPlayer_c::faceExprKey_s l_faceExpr54[] = {{0, 0}, {6, 5}, {21, 0}, {-1, 0}}; // .data 0x4E0
static dAcPlayer_c::faceExprKey_s l_faceExpr55[] = {{0, 0}, {-1, 0}}; // .data 0x4F0
static dAcPlayer_c::faceExprKey_s l_faceExpr56[] = {{0, 0}, {9, 5}, {27, 4}, {-1, 0}}; // .data 0x4F8
static dAcPlayer_c::faceExprKey_s l_faceExpr57[] = {{0, 4}, {-1, 0}}; // .data 0x508
static dAcPlayer_c::faceExprKey_s l_faceExpr58[] = {{0, 0}, {3, 5}, {6, 1}, {-1, 0}}; // .data 0x510
static dAcPlayer_c::faceExprKey_s l_faceExpr59[] = {{0, 1}, {-1, 0}}; // .data 0x520
static dAcPlayer_c::faceExprKey_s l_faceExpr5A[] = {{0, 0}, {6, 5}, {12, 0}, {36, 3}, {-1, 0}}; // .data 0x528 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr5B[] = {{0, 3}, {-1, 0}}; // .data 0x540
static dAcPlayer_c::faceExprKey_s l_faceExpr5C[] = {{0, 3}, {-1, 0}}; // .data 0x548
static dAcPlayer_c::faceExprKey_s l_faceExpr5D[] = {{0, 3}, {-1, 0}}; // .data 0x550
static dAcPlayer_c::faceExprKey_s l_faceExpr5E[] = {{0, 0}, {6, 5}, {21, 4}, {54, 5}, {72, 3}, {-1, 0}}; // .data 0x558
static dAcPlayer_c::faceExprKey_s l_faceExpr5F[] = {{0, 3}, {-1, 0}}; // .data 0x570
static dAcPlayer_c::faceExprKey_s l_faceExpr60[] = {{0, 0}, {9, 5}, {15, 2}, {-1, 0}}; // .data 0x578
static dAcPlayer_c::faceExprKey_s l_faceExpr61[] = {{0, 2}, {-1, 0}}; // .data 0x588
static dAcPlayer_c::faceExprKey_s l_faceExpr62[] = {{0, 0}, {27, 5}, {-1, 0}}; // .data 0x590 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr63[] = {{0, 5}, {-1, 0}}; // .data 0x5A0
static dAcPlayer_c::faceExprKey_s l_faceExpr64[] = {{0, 0}, {6, 5}, {33, 1}, {-1, 0}}; // .data 0x5A8
static dAcPlayer_c::faceExprKey_s l_faceExpr65[] = {{0, 1}, {-1, 0}}; // .data 0x5B8
static dAcPlayer_c::faceExprKey_s l_faceExpr66[] = {{0, 1}, {-1, 0}}; // .data 0x5C0
static dAcPlayer_c::faceExprKey_s l_faceExpr67[] = {{0, 2}, {-1, 0}}; // .data 0x5C8
static dAcPlayer_c::faceExprKey_s l_faceExpr68[] = {{0, 3}, {-1, 0}}; // .data 0x5D0
static dAcPlayer_c::faceExprKey_s l_faceExpr69[] = {{0, 0}, {6, 5}, {24, 1}, {-1, 0}}; // .data 0x5D8
static dAcPlayer_c::faceExprKey_s l_faceExpr6A[] = {{0, 1}, {-1, 0}}; // .data 0x5E8
static dAcPlayer_c::faceExprKey_s l_faceExpr6B[] = {{0, 0}, {3, 5}, {15, 4}, {-1, 0}}; // .data 0x5F0
static dAcPlayer_c::faceExprKey_s l_faceExpr6C[] = {{0, 4}, {-1, 0}}; // .data 0x600
static dAcPlayer_c::faceExprKey_s l_faceExpr6D[] = {{0, 0}, {24, 5}, {-1, 0}}; // .data 0x608 (+1 zero word)
static dAcPlayer_c::faceExprKey_s l_faceExpr6E[] = {{0, 5}, {-1, 0}}; // .data 0x618
static dAcPlayer_c::faceExprKey_s l_faceExpr6F[] = {{0, 1}, {-1, 0}}; // .data 0x620
static dAcPlayer_c::faceExprKey_s l_faceExpr70[] = {{0, 0}, {21, 5}, {60, 3}, {-1, 0}}; // .data 0x628
static dAcPlayer_c::faceExprKey_s l_faceExpr71[] = {{0, 3}, {-1, 0}}; // .data 0x638
static dAcPlayer_c::faceExprKey_s l_faceExpr72[] = {{0, 0}, {21, 5}, {60, 3}, {-1, 0}}; // .data 0x640
static dAcPlayer_c::faceExprKey_s l_faceExpr73[] = {{0, 3}, {-1, 0}}; // .data 0x650
static dAcPlayer_c::faceExprKey_s l_faceExpr74[] = {{0, 0}, {9, 5}, {15, 4}, {-1, 0}}; // .data 0x658
static dAcPlayer_c::faceExprKey_s l_faceExpr75[] = {{0, 4}, {-1, 0}}; // .data 0x668

// .rodata 0x0: the skin colours (dAcPlayer_c::_221C).
static const u8 l_skinColors[8][3] = {
    {0xFF, 0xBD, 0x94}, {0xFF, 0xB5, 0x8C}, {0xF7, 0xAD, 0x84}, {0xE7, 0x9C, 0x73},
    {0xD6, 0x8C, 0x63}, {0xC6, 0x7B, 0x52}, {0xB5, 0x6B, 0x42}, {0xA5, 0x5A, 0x31},
};

// .rodata 0x18: the move parameters.
struct dAcPlayerMoveParam_c {
    /* 0x00 */ f32 m00;
    /* 0x04 */ f32 m04;
    /* 0x08 */ f32 mNearDist;   // 0.4
    /* 0x0C */ f32 m0C;
    /* 0x10 */ f32 mWalkSpeed;  // 1.6
    /* 0x14 */ f32 mRunSpeed;   // 2.0
    /* 0x18 */ f32 mDashSpeed;  // 2.7
    /* 0x1C */ f32 mMaxSpeed;   // 3.0
    /* 0x20 */ f32 mMoveAccel;  // 0.2
    /* 0x24 */ f32 m24;
    /* 0x28 */ f32 m28;
    /* 0x2C */ s16 mTurnMax;    // 0x1112
    /* 0x2E */ s16 mTurnMin;    // 0x0B61
    /* 0x30 */ s16 m30;
    /* 0x32 */ s16 mTurnBackAngle; // 0x4001
    /* 0x34 */ f32 m34;
    /* 0x38 */ f32 m38;
    /* 0x3C */ f32 m3C;
    /* 0x40 */ f32 m40;
    /* 0x44 */ f32 m44;
    /* 0x48 */ f32 m48;
    /* 0x4C */ f32 m4C;
    /* 0x50 */ f32 mBlockedWalkSpeed; // 0.2
    /* 0x54 */ f32 mBlockedRunSpeed;  // 1.4
    /* 0x58 */ f32 mBlockedDashSpeed; // 1.9
    /* 0x5C */ f32 mWaitMorph;  // 32
    /* 0x60 */ f32 mWalkMorph;  // 32
    /* 0x64 */ f32 mWalkToRunMorph;  // 16
    /* 0x68 */ f32 mRunToDashMorph;  // 16
    /* 0x6C */ f32 mDashMorph;       // 16
    /* 0x70 */ f32 mRunToWalkMorph;  // 16
    /* 0x74 */ f32 mWalkToDashMorph; // 8
    /* 0x78 */ f32 mRunStartMorph;   // 22
    /* 0x7C */ f32 mWalkRateMin;  // 0.44
    /* 0x80 */ f32 mRunRateMin;   // 0.99
    /* 0x84 */ f32 mDashRateMin;  // 1.15
    /* 0x88 */ f32 mDashSpeedMin; // 2.5
    /* 0x8C */ f32 mRunStopAccel; // 0.12
    /* 0x90 */ s16 mLeanMaxAlt;   // 3500
    /* 0x92 */ s16 mLeanMax;      // 3000
    /* 0x94 */ s16 m94;
    /* 0x96 */ s16 m96;
    /* 0x98 */ f32 mRunStopFrames; // 6
    /* 0x9C */ f32 mAnmRateScale;  // 1.05
    /* 0xA0 */ f32 mDashStepFrame0; // 8
    /* 0xA4 */ f32 mDashStepFrame1; // 24
    /* 0xA8 */ f32 mDashStopFrame0; // 8
    /* 0xAC */ f32 mDashStopFrame1; // 24
    /* 0xB0 */ s16 mBodyTurnMax;   // 0x0CCD
    /* 0xB2 */ s16 mBodyTurnMin;   // 0x07D3
    /* 0xB4 */ f32 mB4;
};

static const dAcPlayerMoveParam_c l_moveParam = {
    2.0f, 2.7f, 0.4f, 0.4f, 1.6f, 2.0f, 2.7f, 3.0f, 0.2f, 0.1f, 0.4f,
    0x1112, 0x0B61, 0x0B61, 0x4001,
    0.0f, 1.0f, 1.1f, 1.3f, 0.4f, 1.5f, 1.9f, 0.2f, 1.4f, 1.9f, 32.0f, 32.0f, 16.0f, 16.0f, 16.0f, 16.0f, 8.0f,
    22.0f, 0.44f, 0.99f, 1.15f, 2.5f, 0.12f,
    3500, 3000, 350, 0,
    6.0f, 1.05f, 8.0f, 24.0f, 8.0f, 24.0f,
    0x0CCD, 0x07D3, 0.0f,
};

// .rodata 0xD0: the hair colour's alpha in the "h" material (mHairColor).
static const u8 l_hairColorAlpha[8] = {0x8C, 0x6E, 0x50, 0x6E, 0x3C, 0x5A, 0x3C, 0x3C};
// .rodata 0xD8: the two "skin" palette colours of each face (mFace), RGB.
static const u8 l_faceColors[16][6] = {
    {0xB0, 0x58, 0x38, 0xD0, 0x98, 0x78}, {0x00, 0x00, 0x00, 0x50, 0x50, 0x50},
    {0x50, 0x68, 0x70, 0x88, 0xA0, 0xA8}, {0x38, 0x78, 0x60, 0x90, 0xB0, 0xB0},
    {0xB0, 0x58, 0x38, 0xD0, 0x98, 0x70}, {0x00, 0x00, 0x00, 0x50, 0x50, 0x50},
    {0x38, 0x58, 0xB0, 0x80, 0xA0, 0xE0}, {0x20, 0x68, 0xB0, 0x78, 0xB0, 0xE8},
    {0x00, 0x00, 0x00, 0x63, 0x63, 0x63}, {0x00, 0x00, 0x00, 0x63, 0x63, 0x63},
    {0x38, 0x58, 0xB0, 0x80, 0xA0, 0xE8}, {0x50, 0x68, 0x70, 0x88, 0xA0, 0xA8},
    {0x38, 0x70, 0x60, 0x90, 0xB0, 0x90}, {0xB0, 0x50, 0x38, 0xD0, 0x88, 0x68},
    {0x60, 0x78, 0x88, 0xA0, 0xB0, 0xB8}, {0x48, 0x78, 0x60, 0x78, 0xB0, 0x88},
};

// .rodata 0x138: the state table (init / execute / end of each dPlayerMgr::State).
const dAcPlayer_c::stateProc_s dAcPlayer_c::sStateProcs[dPlayerMgr::STATE_COUNT] = {
    {&dAcPlayer_c::initIdle, &dAcPlayer_c::executeIdle, &dAcPlayer_c::endIdle}, // 0x00
    {&dAcPlayer_c::initWalk, &dAcPlayer_c::executeWalk, &dAcPlayer_c::endWalk}, // 0x01
    {&dAcPlayer_c::initRun, &dAcPlayer_c::executeRun, &dAcPlayer_c::endRun}, // 0x02
    {&dAcPlayer_c::initMoveTo, &dAcPlayer_c::executeMoveTo, &dAcPlayer_c::endMoveTo}, // 0x03
    {&dAcPlayer_c::initTurnTo, &dAcPlayer_c::executeTurnTo, &dAcPlayer_c::endTurnTo}, // 0x04
    {&dAcPlayer_c::initChangeEquipment, &dAcPlayer_c::executeChangeEquipment, &dAcPlayer_c::endChangeEquipment}, // 0x05
    {&dAcPlayer_c::initChangeHeldItem, &dAcPlayer_c::executeChangeHeldItem, &dAcPlayer_c::endChangeHeldItem}, // 0x06
    {&dAcPlayer_c::initDemoStart, &dAcPlayer_c::executeDemoStart, &dAcPlayer_c::endDemoStart}, // 0x07
    {&dAcPlayer_c::initDemoWait, &dAcPlayer_c::executeDemoWait, &dAcPlayer_c::endDemoWait}, // 0x08
    {&dAcPlayer_c::initEnterDoor, &dAcPlayer_c::executeEnterDoor, &dAcPlayer_c::endEnterDoor}, // 0x09
    {&dAcPlayer_c::initExitDoor, &dAcPlayer_c::executeExitDoor, &dAcPlayer_c::endExitDoor}, // 0x0A
    {&dAcPlayer_c::initReturnHome, &dAcPlayer_c::executeReturnHome, &dAcPlayer_c::endReturnHome}, // 0x0B
    {&dAcPlayer_c::initWalkThroughDoor, &dAcPlayer_c::executeWalkThroughDoor, &dAcPlayer_c::endWalkThroughDoor}, // 0x0C
    {&dAcPlayer_c::initNetSwing, &dAcPlayer_c::executeNetSwing, &dAcPlayer_c::endNetSwing}, // 0x0D
    {&dAcPlayer_c::initNetCatch, &dAcPlayer_c::executeNetCatch, &dAcPlayer_c::endNetCatch}, // 0x0E
    {&dAcPlayer_c::initNetPocket, &dAcPlayer_c::executeNetPocket, &dAcPlayer_c::endNetPocket}, // 0x0F
    {&dAcPlayer_c::initAxeSwing, &dAcPlayer_c::executeAxeSwing, &dAcPlayer_c::endAxeSwing}, // 0x10
    {&dAcPlayer_c::initAxeStrike, &dAcPlayer_c::executeAxeStrike, &dAcPlayer_c::endAxeStrike}, // 0x11
    {&dAcPlayer_c::initAxeChop, &dAcPlayer_c::executeAxeChop, &dAcPlayer_c::endAxeChop}, // 0x12
    {&dAcPlayer_c::initAxeHit, &dAcPlayer_c::executeAxeHit, &dAcPlayer_c::endAxeHit}, // 0x13
    {&dAcPlayer_c::initAxeMiss, &dAcPlayer_c::executeAxeMiss, &dAcPlayer_c::endAxeMiss}, // 0x14
    {&dAcPlayer_c::initAxeBreak, &dAcPlayer_c::executeAxeBreak, &dAcPlayer_c::endAxeBreak}, // 0x15
    {&dAcPlayer_c::initAxeBreakMsg, &dAcPlayer_c::executeAxeBreakMsg, &dAcPlayer_c::endAxeBreakMsg}, // 0x16
    {&dAcPlayer_c::initAxeHitObject, &dAcPlayer_c::executeAxeHitObject, &dAcPlayer_c::endAxeHitObject}, // 0x17
    {&dAcPlayer_c::initFishingCastMiss, &dAcPlayer_c::executeFishingCastMiss, &dAcPlayer_c::endFishingCastMiss}, // 0x18
    {&dAcPlayer_c::initFishingCast, &dAcPlayer_c::executeFishingCast, &dAcPlayer_c::endFishingCast}, // 0x19
    {&dAcPlayer_c::initFishingWait, &dAcPlayer_c::executeFishingWait, &dAcPlayer_c::endFishingWait}, // 0x1A
    {&dAcPlayer_c::initFishingHooked, &dAcPlayer_c::executeFishingHooked, &dAcPlayer_c::endFishingHooked}, // 0x1B
    {&dAcPlayer_c::initFishingFail, &dAcPlayer_c::executeFishingFail, &dAcPlayer_c::endFishingFail}, // 0x1C
    {&dAcPlayer_c::initFishingPutBack, &dAcPlayer_c::executeFishingPutBack, &dAcPlayer_c::endFishingPutBack}, // 0x1D
    {&dAcPlayer_c::initFishingLand, &dAcPlayer_c::executeFishingLand, &dAcPlayer_c::endFishingLand}, // 0x1E
    {&dAcPlayer_c::initFishingGet, &dAcPlayer_c::executeFishingGet, &dAcPlayer_c::endFishingGet}, // 0x1F
    {&dAcPlayer_c::initFishingPutAway, &dAcPlayer_c::executeFishingPutAway, &dAcPlayer_c::endFishingPutAway}, // 0x20
    {&dAcPlayer_c::initTakeOutTool, &dAcPlayer_c::executeTakeOutTool, &dAcPlayer_c::endTakeOutTool}, // 0x21
    {&dAcPlayer_c::initPutAwayTool, &dAcPlayer_c::executePutAwayTool, &dAcPlayer_c::endPutAwayTool}, // 0x22
    {&dAcPlayer_c::initFtrGrab, &dAcPlayer_c::executeFtrGrab, &dAcPlayer_c::endFtrGrab}, // 0x23
    {&dAcPlayer_c::initFtrHold, &dAcPlayer_c::executeFtrHold, &dAcPlayer_c::endFtrHold}, // 0x24
    {&dAcPlayer_c::initFtrRotate, &dAcPlayer_c::executeFtrRotate, &dAcPlayer_c::endFtrRotate}, // 0x25
    {&dAcPlayer_c::initFtrPushStart, &dAcPlayer_c::executeFtrPushStart, &dAcPlayer_c::endFtrPushStart}, // 0x26
    {&dAcPlayer_c::initFtrPush, &dAcPlayer_c::executeFtrPush, &dAcPlayer_c::endFtrPush}, // 0x27
    {&dAcPlayer_c::initFtrPullStart, &dAcPlayer_c::executeFtrPullStart, &dAcPlayer_c::endFtrPullStart}, // 0x28
    {&dAcPlayer_c::initFtrPull, &dAcPlayer_c::executeFtrPull, &dAcPlayer_c::endFtrPull}, // 0x29
    {&dAcPlayer_c::initLieDown, &dAcPlayer_c::executeLieDown, &dAcPlayer_c::endLieDown}, // 0x2A
    {&dAcPlayer_c::initGetIntoBed, &dAcPlayer_c::executeGetIntoBed, &dAcPlayer_c::endGetIntoBed}, // 0x2B
    {&dAcPlayer_c::initGetUp, &dAcPlayer_c::executeGetUp, &dAcPlayer_c::endGetUp}, // 0x2C
    {&dAcPlayer_c::initGetOutOfBed, &dAcPlayer_c::executeGetOutOfBed, &dAcPlayer_c::endGetOutOfBed}, // 0x2D
    {&dAcPlayer_c::initBedWait, &dAcPlayer_c::executeBedWait, &dAcPlayer_c::endBedWait}, // 0x2E
    {&dAcPlayer_c::initBedRoll, &dAcPlayer_c::executeBedRoll, &dAcPlayer_c::endBedRoll}, // 0x2F
    {&dAcPlayer_c::initBedRollOver, &dAcPlayer_c::executeBedRollOver, &dAcPlayer_c::endBedRollOver}, // 0x30
    {&dAcPlayer_c::initBedRollBlocked, &dAcPlayer_c::executeBedRollBlocked, &dAcPlayer_c::endBedRollBlocked}, // 0x31
    {&dAcPlayer_c::initChestOpen, &dAcPlayer_c::executeChestOpen, &dAcPlayer_c::endChestOpen}, // 0x32
    {&dAcPlayer_c::initChestWait, &dAcPlayer_c::executeChestWait, &dAcPlayer_c::endChestWait}, // 0x33
    {&dAcPlayer_c::initChestClose, &dAcPlayer_c::executeChestClose, &dAcPlayer_c::endChestClose}, // 0x34
    {&dAcPlayer_c::initSitDown, &dAcPlayer_c::executeSitDown, &dAcPlayer_c::endSitDown}, // 0x35
    {&dAcPlayer_c::initSitDownFront, &dAcPlayer_c::executeSitDownFront, &dAcPlayer_c::endSitDownFront}, // 0x36
    {&dAcPlayer_c::initSitDownSide, &dAcPlayer_c::executeSitDownSide, &dAcPlayer_c::endSitDownSide}, // 0x37
    {&dAcPlayer_c::initChairWait, &dAcPlayer_c::executeChairWait, &dAcPlayer_c::endChairWait}, // 0x38
    {&dAcPlayer_c::initChairPose, &dAcPlayer_c::executeChairPose, &dAcPlayer_c::endChairPose}, // 0x39
    {&dAcPlayer_c::initChairStandUp, &dAcPlayer_c::executeChairStandUp, &dAcPlayer_c::endChairStandUp}, // 0x3A
    {&dAcPlayer_c::initChairFaceFront, &dAcPlayer_c::executeChairFaceFront, &dAcPlayer_c::endChairFaceFront}, // 0x3B
    {&dAcPlayer_c::initShakeTree, &dAcPlayer_c::executeShakeTree, &dAcPlayer_c::endShakeTree}, // 0x3C
    {&dAcPlayer_c::initShakeTreeEnd, &dAcPlayer_c::executeShakeTreeEnd, &dAcPlayer_c::endShakeTreeEnd}, // 0x3D
    {&dAcPlayer_c::initShovelStart, &dAcPlayer_c::executeShovelStart, &dAcPlayer_c::endShovelStart}, // 0x3E
    {&dAcPlayer_c::initShovelWait, &dAcPlayer_c::executeShovelWait, &dAcPlayer_c::endShovelWait}, // 0x3F
    {&dAcPlayer_c::initShovelDig, &dAcPlayer_c::executeShovelDig, &dAcPlayer_c::endShovelDig}, // 0x40
    {&dAcPlayer_c::initShovelHit, &dAcPlayer_c::executeShovelHit, &dAcPlayer_c::endShovelHit}, // 0x41
    {&dAcPlayer_c::initShovelMiss, &dAcPlayer_c::executeShovelMiss, &dAcPlayer_c::endShovelMiss}, // 0x42
    {&dAcPlayer_c::initShovelSwingLow, &dAcPlayer_c::executeShovelSwingLow, &dAcPlayer_c::endShovelSwingLow}, // 0x43
    {&dAcPlayer_c::initPutItemInHole, &dAcPlayer_c::executePutItemInHole, &dAcPlayer_c::endPutItemInHole}, // 0x44
    {&dAcPlayer_c::initShovelBury, &dAcPlayer_c::executeShovelBury, &dAcPlayer_c::endShovelBury}, // 0x45
    {&dAcPlayer_c::initShovelDigUpItem, &dAcPlayer_c::executeShovelDigUpItem, &dAcPlayer_c::endShovelDigUpItem}, // 0x46
    {&dAcPlayer_c::initShowDugItem, &dAcPlayer_c::executeShowDugItem, &dAcPlayer_c::endShowDugItem}, // 0x47
    {&dAcPlayer_c::initShovelHitObject, &dAcPlayer_c::executeShovelHitObject, &dAcPlayer_c::endShovelHitActor}, // 0x48
    {&dAcPlayer_c::initExitRoom, &dAcPlayer_c::executeExitRoom, &dAcPlayer_c::endExitRoom}, // 0x49
    {&dAcPlayer_c::initEnterRoom, &dAcPlayer_c::executeEnterRoom, &dAcPlayer_c::endEnterRoom}, // 0x4A
    {&dAcPlayer_c::initMiiMaskImmediate, &dAcPlayer_c::executeMiiMaskImmediate, &dAcPlayer_c::endMiiMaskImmediate}, // 0x4B
    {&dAcPlayer_c::initMiiMask, &dAcPlayer_c::executeMiiMask, &dAcPlayer_c::endMiiMask}, // 0x4C
    {&dAcPlayer_c::initChangeDesign, &dAcPlayer_c::executeChangeDesign, &dAcPlayer_c::endChangeDesign}, // 0x4D
    {&dAcPlayer_c::initChangeEquipmentQuick, &dAcPlayer_c::executeChangeEquipmentQuick, &dAcPlayer_c::endChangeEquipmentQuick}, // 0x4E
    {&dAcPlayer_c::initChangeHair, &dAcPlayer_c::executeChangeHair, &dAcPlayer_c::endChangeHair}, // 0x4F
    {&dAcPlayer_c::initChangeMiiMask, &dAcPlayer_c::executeChangeMiiMask, &dAcPlayer_c::endChangeMiiMask}, // 0x50
    {&dAcPlayer_c::initPickUpReach, &dAcPlayer_c::executePickUpReach, &dAcPlayer_c::endPickUpReach}, // 0x51
    {&dAcPlayer_c::initPickUpItem, &dAcPlayer_c::executePickUpItem, &dAcPlayer_c::endPickUpItem}, // 0x52
    {&dAcPlayer_c::initPickUpItemMsg, &dAcPlayer_c::executePickUpItemMsg, &dAcPlayer_c::endPickUpItemMsg}, // 0x53
    {&dAcPlayer_c::initShowPickedItem, &dAcPlayer_c::executeShowPickedItem, &dAcPlayer_c::endShowPickedItem}, // 0x54
    {&dAcPlayer_c::initPullWeed, &dAcPlayer_c::executePullWeed, &dAcPlayer_c::endPullWeed}, // 0x55
    {&dAcPlayer_c::initFillHoleWait, &dAcPlayer_c::executeFillHoleWait, &dAcPlayer_c::endFillHoleWait}, // 0x56
    {&dAcPlayer_c::initFillHole, &dAcPlayer_c::executeStampHole, &dAcPlayer_c::endFillHole}, // 0x57
    {&dAcPlayer_c::initPitfallTrigger, &dAcPlayer_c::executePitfallTrigger, &dAcPlayer_c::endPitfallTrigger}, // 0x58
    {&dAcPlayer_c::initPitfallFall, &dAcPlayer_c::executePitfallFall, &dAcPlayer_c::endPitfallFall}, // 0x59
    {&dAcPlayer_c::initPitfallStruggle, &dAcPlayer_c::executePitfallStruggle, &dAcPlayer_c::endPitfallStruggle}, // 0x5A
    {&dAcPlayer_c::initPitfallExit, &dAcPlayer_c::executePitfallClimbOut, &dAcPlayer_c::endPitfallExit}, // 0x5B
    {&dAcPlayer_c::initPocketFull, &dAcPlayer_c::executePocketFull, &dAcPlayer_c::endPocketFull}, // 0x5C
    {&dAcPlayer_c::initPickUp, &dAcPlayer_c::executePickUp, &dAcPlayer_c::endPickUp}, // 0x5D
    {&dAcPlayer_c::initSwapItem, &dAcPlayer_c::executeSwapItem, &dAcPlayer_c::endSwapItem}, // 0x5E
    {&dAcPlayer_c::initShowCatch, &dAcPlayer_c::executeShowCatch, &dAcPlayer_c::endShowCatch}, // 0x5F
    {&dAcPlayer_c::initShowCatchAt, &dAcPlayer_c::executeShowCatchAt, &dAcPlayer_c::endShowCatchAt}, // 0x60
    {&dAcPlayer_c::initSurprised, &dAcPlayer_c::executeSurprised, &dAcPlayer_c::endSurprised}, // 0x61
    {&dAcPlayer_c::initBeeSting, &dAcPlayer_c::executeBeeSting, &dAcPlayer_c::endBeeSting}, // 0x62
    {&dAcPlayer_c::initReceiveItem, &dAcPlayer_c::executeReceiveItem, &dAcPlayer_c::endReceiveItem}, // 0x63
    {&dAcPlayer_c::initPassItem, &dAcPlayer_c::executePassItem, &dAcPlayer_c::endPassItem}, // 0x64
    {&dAcPlayer_c::initPassItemWait, &dAcPlayer_c::executePassItemWait, &dAcPlayer_c::endPassItemWait}, // 0x65
    {&dAcPlayer_c::initHandItemReach, &dAcPlayer_c::executeHandItemReach, &dAcPlayer_c::endHandItemReach}, // 0x66
    {&dAcPlayer_c::initHandItemTake, &dAcPlayer_c::executeHandItemTake, &dAcPlayer_c::endHandItemTake}, // 0x67
    {&dAcPlayer_c::initHandItemHold, &dAcPlayer_c::executeHandItemHold, &dAcPlayer_c::endHandItemHold}, // 0x68
    {&dAcPlayer_c::initHandItemPutAway, &dAcPlayer_c::executeHandItemPutAway, &dAcPlayer_c::endHandItemPutAway}, // 0x69
    {&dAcPlayer_c::initGiveItem, &dAcPlayer_c::executeGiveItem, &dAcPlayer_c::endGiveItem}, // 0x6A
    {&dAcPlayer_c::initMailReceive, &dAcPlayer_c::executeMailReceive, &dAcPlayer_c::endMailReceive}, // 0x6B
    {&dAcPlayer_c::initMailReceiveEnd, &dAcPlayer_c::executeMailReceiveEnd, &dAcPlayer_c::endMailReceiveEnd}, // 0x6C
    {&dAcPlayer_c::initMailPutAway, &dAcPlayer_c::executeMailPutAway, &dAcPlayer_c::endMailPutAway}, // 0x6D
    {&dAcPlayer_c::initHandItemReturn, &dAcPlayer_c::executeHandItemReturn, &dAcPlayer_c::endHandItemReturn}, // 0x6E
    {&dAcPlayer_c::initInsectBite, &dAcPlayer_c::executeInsectBite, &dAcPlayer_c::endInsectBite}, // 0x6F
    {&dAcPlayer_c::initChat, &dAcPlayer_c::executeChat, &dAcPlayer_c::endChat}, // 0x70
    {&dAcPlayer_c::initEmotion, &dAcPlayer_c::executeEmotion, &dAcPlayer_c::endEmotion}, // 0x71
    {&dAcPlayer_c::initMailboxOpen, &dAcPlayer_c::executeMailboxOpen, &dAcPlayer_c::endMailboxOpen}, // 0x72
    {&dAcPlayer_c::initMailboxClose, &dAcPlayer_c::executeMailboxClose, &dAcPlayer_c::endMailboxClose}, // 0x73
    {&dAcPlayer_c::initFaint, &dAcPlayer_c::executeFaint, &dAcPlayer_c::endFaint}, // 0x74
    {&dAcPlayer_c::initFaintGetUp, &dAcPlayer_c::executeFaintGetUp, &dAcPlayer_c::endFaintGetUp}, // 0x75
    {&dAcPlayer_c::initTurnToTalker, &dAcPlayer_c::executeTurnToTalker, &dAcPlayer_c::endTurnToTalker}, // 0x76
    {&dAcPlayer_c::initTrip, &dAcPlayer_c::executeTrip, &dAcPlayer_c::endTrip}, // 0x77
    {&dAcPlayer_c::initEat, &dAcPlayer_c::executeEat, &dAcPlayer_c::endEat}, // 0x78
    {&dAcPlayer_c::initEatRemoveMask, &dAcPlayer_c::executeEatRemoveMask, &dAcPlayer_c::endEatRemoveMask}, // 0x79
    {&dAcPlayer_c::initCatchPose, &dAcPlayer_c::executeCatchPose, &dAcPlayer_c::endCatchPose}, // 0x7A
    {&dAcPlayer_c::initOpenUmbrella, &dAcPlayer_c::executeOpenUmbrella, &dAcPlayer_c::endOpenUmbrella}, // 0x7B
    {&dAcPlayer_c::initCloseUmbrella, &dAcPlayer_c::executeCloseUmbrella, &dAcPlayer_c::endCloseUmbrella}, // 0x7C
    {&dAcPlayer_c::initShakeUmbrella, &dAcPlayer_c::executeShakeUmbrella, &dAcPlayer_c::endShakeUmbrella}, // 0x7D
    {&dAcPlayer_c::initPickUpUnitItem, &dAcPlayer_c::executePickUpUnitItem, &dAcPlayer_c::endPickUpUnitItem}, // 0x7E
    {&dAcPlayer_c::initStairsTurn, &dAcPlayer_c::executeStairsTurn, &dAcPlayer_c::endStairsTurn}, // 0x7F
    {&dAcPlayer_c::initGoUpstairs, &dAcPlayer_c::executeGoUpstairs, &dAcPlayer_c::endGoUpstairs}, // 0x80
    {&dAcPlayer_c::initArriveUpstairs, &dAcPlayer_c::executeArriveUpstairs, &dAcPlayer_c::endArriveUpstairs}, // 0x81
    {&dAcPlayer_c::initGoDownstairs, &dAcPlayer_c::executeGoDownstairs, &dAcPlayer_c::endGoDownstairs}, // 0x82
    {&dAcPlayer_c::initArriveDownstairs, &dAcPlayer_c::executeArriveDownstairs, &dAcPlayer_c::endArriveDownstairs}, // 0x83
    {&dAcPlayer_c::initReact, &dAcPlayer_c::executeReact, &dAcPlayer_c::endReact}, // 0x84
    {&dAcPlayer_c::initPhonePickUp, &dAcPlayer_c::executePhonePickUp, &dAcPlayer_c::endPhonePickUp}, // 0x85
    {&dAcPlayer_c::initPhoneTalk, &dAcPlayer_c::executePhoneTalk, &dAcPlayer_c::endPhoneTalk}, // 0x86
    {&dAcPlayer_c::initPhoneHangUp, &dAcPlayer_c::executePhoneHangUp, &dAcPlayer_c::endPhoneHangUp}, // 0x87
    {&dAcPlayer_c::initFlinch, &dAcPlayer_c::executeFlinch, &dAcPlayer_c::endFlinch}, // 0x88
    {&dAcPlayer_c::initWater, &dAcPlayer_c::executeWater, &dAcPlayer_c::endWater}, // 0x89
    {&dAcPlayer_c::initBlowDandelion, &dAcPlayer_c::executeBlowDandelion, &dAcPlayer_c::endBlowDandelion}, // 0x8A
    {&dAcPlayer_c::initTrickChange, &dAcPlayer_c::executeTrickChange, &dAcPlayer_c::endTrickChange}, // 0x8B
    {&dAcPlayer_c::initSceneStart, &dAcPlayer_c::executeSceneStart, &dAcPlayer_c::endSceneStart}, // 0x8C
    {&dAcPlayer_c::initFireworks, &dAcPlayer_c::executeFireworks, &dAcPlayer_c::endFireworks}, // 0x8D
    {&dAcPlayer_c::initCracker, &dAcPlayer_c::executeCracker, &dAcPlayer_c::endCracker}, // 0x8E
    {&dAcPlayer_c::initBusBoard, &dAcPlayer_c::executeBusBoard, &dAcPlayer_c::endBusBoard}, // 0x8F
    {&dAcPlayer_c::initBusBoarded, &dAcPlayer_c::executeBusBoarded, &dAcPlayer_c::endBusBoarded}, // 0x90
    {&dAcPlayer_c::initBusRide, &dAcPlayer_c::executeBusRide, &dAcPlayer_c::endBusRide}, // 0x91
    {&dAcPlayer_c::initBusGetOff, &dAcPlayer_c::executeBusGetOff, &dAcPlayer_c::endBusGetOff}, // 0x92
    {&dAcPlayer_c::initBusGotOff, &dAcPlayer_c::executeBusGotOff, &dAcPlayer_c::endBusGotOff}, // 0x93
    {&dAcPlayer_c::initBusLeave, &dAcPlayer_c::executeBusLeave, &dAcPlayer_c::endBusLeave}, // 0x94
    {&dAcPlayer_c::initGoddessItem, &dAcPlayer_c::executeGoddessItem, &dAcPlayer_c::endGoddessItem}, // 0x95
    {&dAcPlayer_c::initReleaseBalloon, &dAcPlayer_c::executeReleaseBalloon, &dAcPlayer_c::endReleaseBalloon}, // 0x96
    {&dAcPlayer_c::initEatSpecial, &dAcPlayer_c::executeEatSpecial, &dAcPlayer_c::endEatSpecial}, // 0x97
    {&dAcPlayer_c::initChangeHeldItemDemo, &dAcPlayer_c::executeChangeHeldItemDemo, &dAcPlayer_c::endChangeHeldItemDemo}, // 0x98
    {&dAcPlayer_c::initSlingshotAim, &dAcPlayer_c::executeSlingshotAim, &dAcPlayer_c::endSlingshotAim}, // 0x99
    {&dAcPlayer_c::initSlingshotShoot, &dAcPlayer_c::executeSlingshotShoot, &dAcPlayer_c::endSlingshotShoot}, // 0x9A
    {&dAcPlayer_c::initSlingshotWatch, &dAcPlayer_c::executeSlingshotWatch, &dAcPlayer_c::endSlingshotWatch}, // 0x9B
    {&dAcPlayer_c::initDrinkCoffee, &dAcPlayer_c::executeDrinkCoffee, &dAcPlayer_c::endDrinkCoffee}, // 0x9C
    {&dAcPlayer_c::initFinishCoffee, &dAcPlayer_c::executeFinishCoffee, &dAcPlayer_c::endFinishCoffee}, // 0x9D
    {&dAcPlayer_c::initTurnFront, &dAcPlayer_c::executeTurnFront, &dAcPlayer_c::endTurnFront}, // 0x9E
    {&dAcPlayer_c::initBedHair, &dAcPlayer_c::executeBedHair, &dAcPlayer_c::endBedHair}, // 0x9F
    {&dAcPlayer_c::initChangeOutfitAndHeld, &dAcPlayer_c::executeChangeOutfitAndHeld, &dAcPlayer_c::endChangeOutfitAndHeld}, // 0xA0
    {&dAcPlayer_c::initListenKK, &dAcPlayer_c::executeListenKK, &dAcPlayer_c::endListenKK}, // 0xA1
    {&dAcPlayer_c::initBlowBubble, &dAcPlayer_c::executeBlowBubbles, &dAcPlayer_c::endBlowBubble}, // 0xA2
    {&dAcPlayer_c::initPutHeldItem, &dAcPlayer_c::executePutHeldItem, &dAcPlayer_c::endPutHeldItem}, // 0xA3
    {&dAcPlayer_c::initFortuneHit, &dAcPlayer_c::executeFortuneHit, &dAcPlayer_c::endFortuneHit}, // 0xA4
    {&dAcPlayer_c::initFortuneFaint, &dAcPlayer_c::executeFortuneFaint, &dAcPlayer_c::endFortuneFaint}, // 0xA5
    {&dAcPlayer_c::initShoeShineStart, &dAcPlayer_c::executeShoeShineStart, &dAcPlayer_c::endShoeShineStart}, // 0xA6
    {&dAcPlayer_c::initShoeShine, &dAcPlayer_c::executeShoeShine, &dAcPlayer_c::endShoeShine}, // 0xA7
    {&dAcPlayer_c::initShoeShineEnd, &dAcPlayer_c::executeShoeShineEnd, &dAcPlayer_c::endShoeShineEnd}, // 0xA8
    {&dAcPlayer_c::initPutAwayToolFull, &dAcPlayer_c::executePutAwayToolFull, &dAcPlayer_c::endPutAwayToolFull}, // 0xA9
    {&dAcPlayer_c::initSceneStartHidden, &dAcPlayer_c::executeSceneStartHidden, &dAcPlayer_c::endSceneStartHidden}, // 0xAA
};

// .rodata 0x1944: setStateObjc: the states that set ACTOR_FLAG_DEMO.
static const u8 l_stateObjcFlag[dPlayerMgr::STATE_COUNT] = {
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01,
};
// .rodata 0x19F0: setStateObjc: the states with a fixed objc weight (0xFF).
static const u8 l_stateObjcFixed[dPlayerMgr::STATE_COUNT] = {
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

// .rodata 0x1A9C: the bg check kind of each state, the switch of calcBgCheck.
static const int l_bgCheckKind[dPlayerMgr::STATE_COUNT] = {
    0, 0, 0, 5, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 4, 0, 0, 0, 2, 0, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0,
};

// .rodata 0x1D48: checkToolUseInput: the state each tool's use starts.
static const int l_toolStates[HMN_TOOL_NUM] = {
    dPlayerMgr::STATE_IDLE, dPlayerMgr::STATE_SHOVEL_START, dPlayerMgr::STATE_AXE_SWING, dPlayerMgr::STATE_IDLE,
    dPlayerMgr::STATE_NET_SWING, dPlayerMgr::STATE_WATER, dPlayerMgr::STATE_SLINGSHOT_AIM, dPlayerMgr::STATE_BLOW_DANDELION,
    dPlayerMgr::STATE_CRACKER, dPlayerMgr::STATE_FIREWORKS, dPlayerMgr::STATE_SHAKE_UMBRELLA,
    dPlayerMgr::STATE_IDLE, dPlayerMgr::STATE_IDLE, dPlayerMgr::STATE_BLOW_BUBBLES,
};
// .rodata 0x1D80: setStateObjc: how the move stops on entering each state.
static const int l_stateMoveReset[dPlayerMgr::STATE_COUNT] = {
    0, 3, 3, 3, 3, 1, 1, 1, 1, 3, 3, 1, 3, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 0, 1, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 1, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 2, 1, 2, 2,
    2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 2, 2, 1, 2, 1, 1, 1, 3, 2, 1, 2, 1, 1, 0, 1, 1,
    2, 2, 2, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 1, 1, 1, 3, 1, 1, 1, 1,
};

// .rodata 0x202C: the frames an emotion (mEmotion, 30 kinds) plays before STATE_EMOTION ends.
static const u8 l_emotionFrames[30] = {
    0x00, 0x2D, 0x2D, 0x3C, 0x2D, 0x66, 0x66, 0x4B, 0x66, 0x66, 0x84, 0x3C, 0x3C, 0x6C, 0x84,
    0x54, 0x48, 0x4E, 0x74, 0x5A, 0x2D, 0x2D, 0x2D, 0x78, 0x2D, 0x51, 0x3C, 0x68, 0x78, 0x3C,
};
// .rodata 0x204C: the morph frames back to the wait animation after each emotion.
static const u8 l_emotionEndMorph[30] = {
    0x00, 0x27, 0x18, 0x1E, 0x1B, 0x1E, 0x15, 0x1B, 0x21, 0x1E, 0x24, 0x1E, 0x1B, 0x18, 0x18,
    0x27, 0x24, 0x1E, 0x2A, 0x27, 0x24, 0x21, 0x2A, 0x24, 0x24, 0x2A, 0x15, 0x21, 0x24, 0x21,
};

// .rodata 0x2070: the Mii mask expression track of each dHmnAnm_c::getSubIdx index (setFaceExprAnm).
static const dAcPlayer_c::faceExprKey_s *const l_faceExprTracks[] = {
    l_faceExpr00, l_faceExpr01, l_faceExpr02, l_faceExpr03, l_faceExpr04, l_faceExpr05,
    l_faceExpr06, l_faceExpr07, l_faceExpr08, l_faceExpr09, l_faceExpr0A, l_faceExpr0B,
    l_faceExpr0C, l_faceExpr0D, l_faceExpr0E, l_faceExpr0F, l_faceExpr10, l_faceExpr11,
    l_faceExpr12, l_faceExpr13, l_faceExpr14, l_faceExpr15, l_faceExpr16, l_faceExpr17,
    l_faceExpr18, l_faceExpr19, l_faceExpr1A, l_faceExpr1B, l_faceExpr1C, l_faceExpr1D,
    l_faceExpr1E, l_faceExpr1F, l_faceExpr20, l_faceExpr21, l_faceExpr22, l_faceExpr23,
    l_faceExpr24, l_faceExpr25, l_faceExpr26, l_faceExpr27, l_faceExpr28, l_faceExpr29,
    l_faceExpr2A, l_faceExpr2B, l_faceExpr2C, l_faceExpr2D, l_faceExpr2E, l_faceExpr2F,
    l_faceExpr30, l_faceExpr31, l_faceExpr32, l_faceExpr33, l_faceExpr34, l_faceExpr35,
    l_faceExpr36, l_faceExpr37, l_faceExpr38, l_faceExpr39, l_faceExpr3A, l_faceExpr3B,
    l_faceExpr3C, l_faceExpr3D, l_faceExpr3E, l_faceExpr3F, l_faceExpr40, l_faceExpr41,
    l_faceExpr42, l_faceExpr43, l_faceExpr44, l_faceExpr45, l_faceExpr46, l_faceExpr47,
    l_faceExpr48, l_faceExpr49, l_faceExpr4A, l_faceExpr4B, l_faceExpr4C, l_faceExpr4D,
    l_faceExpr4E, l_faceExpr4F, l_faceExpr50, l_faceExpr51, l_faceExpr52, l_faceExpr53,
    l_faceExpr54, l_faceExpr55, l_faceExpr56, l_faceExpr57, l_faceExpr58, l_faceExpr59,
    l_faceExpr5A, l_faceExpr5B, l_faceExpr5C, l_faceExpr5D, l_faceExpr5E, l_faceExpr5F,
    l_faceExpr60, l_faceExpr61, l_faceExpr62, l_faceExpr63, l_faceExpr64, l_faceExpr65,
    l_faceExpr66, l_faceExpr67, l_faceExpr68, l_faceExpr69, l_faceExpr6A, l_faceExpr6B,
    l_faceExpr6C, l_faceExpr6D, l_faceExpr6E, l_faceExpr6F, l_faceExpr70, l_faceExpr71,
    l_faceExpr72, l_faceExpr73, l_faceExpr74, l_faceExpr75,
};

// File-scope values computed at startup by __sinit (.text 0x2EE0; .bss 0x0..0x15, in this order).
static const f32 l_frontCheckDist = dPlayerMgr::sPlayerValue80750A7C; // .bss 0x0: calcPosAtAngle distance
static s16 l_angle45 = mAng::fromDegree(45.0f);                // .bss 0x4
static s16 l_angle15 = mAng::fromDegree(15.0f);                // .bss 0x6
static s16 l_angle75 = mAng::fromDegree(75.0f);                // .bss 0x8: sLib::distanceAngle limit
static s16 l_angle22_5 = mAng::fromDegree(22.5f);              // .bss 0xA
static s16 l_angle30 = mAng::fromDegree(30.0f);                // .bss 0xC
static s16 l_angle75b = mAng::fromDegree(75.0f);               // .bss 0xE: sLib::distanceAngle limit
static s16 l_angle10 = mAng::fromDegree(10.0f);                // .bss 0x10
static s16 l_angle3 = mAng::fromDegree(3.0f);                  // .bss 0x12
static u8 l_roomSeFrame = dPlayerMgr::sRemoteActionDelayFrames - 0x34; // .bss 0x14: dSTR::playRoomSe timing



// .text 0x98
void dAcPlayer_c::myMdlCallback_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes,
                                           nw4r::g3d::ResMdl resMdl) {
    mMtx_c mtx;
    if (nodeId == mBaseNode) {
        anmRes->GetRotTrans(mtx);
        mtx.YrotM(mAng(mBaseRotY));
        if (dHmnAnm_c::isCurve(mAnmId)) {
            dHmnAnm_c::curveMtx(&mtx, &mtx, mCurveBaseY);
        }
        anmRes->SetRotTrans(mtx);
    } else if (nodeId == mFeelNode) {
        anmRes->GetRotTrans(mtx);
        nw4r::math::_VEC3 trans = resMdl.GetResNode(nodeId).ref().translate;
        mtx._03 = trans.x;
        mtx._13 = trans.y;
        mtx._23 = trans.z;
        anmRes->SetRotTrans(mtx);
    } else if (nodeId == mHeadNode) {
        anmRes->GetRotTrans(mtx);
        mtx.XrotM(mAng(mHeadRotX));
        mtx.YrotM(mAng(mHeadRotY));
        anmRes->SetRotTrans(mtx);
    } else if (nodeId == mRootNode) {
        anmRes->GetRotTrans(mtx);
        mtx.XrotM(mAng(mRootRotX));
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM) && dHmnAnm_c::getFlag(mAnmId)) {
            nw4r::math::_VEC3 trans = resMdl.GetResNode(nodeId).ref().translate;
            mtx._03 = trans.x;
            mtx._23 = trans.z;
        }
        anmRes->SetRotTrans(mtx);
    }
}

// .text 0x280
void dAcPlayer_c::myMdlCallback_c::timingB(ulong nodeId, nw4r::g3d::WorldMtxManip *manip,
                                           nw4r::g3d::ResMdl resMdl) {
    mMtx_c mtx;
    mVec3_c pos(0.0f, 0.0f, 0.0f);
    if (nodeId == mRootNode) {
        manip->GetMatrix(mtx);
        pos.set(mtx._03, mtx._13, mtx._23);
        dWorld::toFieldPosition(&mSoundPos, &pos);
        mRootY = mSoundPos.y;
        switch (mAnmId) {
        case HMN_ANM_STAIR_DOWN1:
        case HMN_ANM_STAIR_DOWN2:
        case HMN_ANM_STAIR_UP1:
        case HMN_ANM_STAIR_UP2:
            mSoundPos.y = dBGCF::getGroundY(&mSoundPos, FALSE);
            break;
        case HMN_ANM_BUS_IN1:
        case HMN_ANM_BUS_IN2:
        case HMN_ANM_BUS_IN_WAIT1:
        case HMN_ANM_BUS_OUT1:
        case HMN_ANM_BUS_OUT2:
        case HMN_ANM_BUS_OUT_WAIT1: {
            dAcPlayer_c *player = dPlayerMgr::getPlayerActorRaw(4);
            if (player != NULL) {
                mSoundPos.x = player->mPos.x;
                mSoundPos.y = player->mPos.y;
                mSoundPos.z = player->mPos.z;
            }
            break;
        }
        }
    } else if (nodeId == mBaseNode) {
        manip->GetMatrix(mtx);
        pos.set(mtx._03, mtx._13, mtx._23);
        dWorld::toFieldPosition(&mBasePos, &pos);
    } else if (nodeId == mHandNode) {
        manip->GetMatrix(mHandMtx);
        pos.set(mHandMtx._03, mHandMtx._13, mHandMtx._23);
        dWorld::toFieldPosition(&mHandPos, &pos);
    } else if (nodeId == mRHandNode) {
        manip->GetMatrix(mtx);
        pos.set(mtx._03, mtx._13, mtx._23);
        dWorld::toFieldPosition(&mRHandPos, &pos);
    } else if (nodeId == mRFootNode) {
        manip->GetMatrix(mRFootMtx);
        pos.set(mRFootMtx._03, mRFootMtx._13, mRFootMtx._23);
        dWorld::toFieldPosition(&mRFootPos, &pos);
    } else if (nodeId == mLFootNode) {
        manip->GetMatrix(mLFootMtx);
        pos.set(mLFootMtx._03, mLFootMtx._13, mLFootMtx._23);
        dWorld::toFieldPosition(&mLFootPos, &pos);
    } else if (nodeId == mHeadNode) {
        manip->GetMatrix(mtx);
        pos.set(mtx._03, mtx._13, mtx._23);
        dWorld::toFieldPosition(&mHeadPos, &pos);
    } else if (nodeId == mFeelNode) {
        manip->GetMatrix(mFeelMtx);
    }
}

// .text 0x524
void dAcPlayer_c::myMdlCallback_c::timingC(nw4r::math::MTX34 *mtx, nw4r::g3d::ResMdl resMdl) {}

// The state handlers and helpers. Each .inc gets its own .text section after this file's code (-sym on),
// but its data and literals are generated here, before the rest of the actor's own functions. The split
// points come from the target: header weak copies between the sections (mVec3_c::operator-/+ after
// move, dObjc::acpos_c after demo, myFace_c's after event) and functions with new string literals that
// were placed in the section of the file whose data object was generated last (run / tool / item).
#include "d_a_player_move.inc"
#include "d_a_player_run.inc"
#include "d_a_player_demo.inc"
#include "d_a_player_tool.inc"
#include "d_a_player_item.inc"
#include "d_a_player_event.inc"
#include "d_a_player_net.inc"

// A stripped player function referenced these headers before the player ctor.
// This witness preserves their weak-section order; the original body is unknown.
void dAPlayer_retainAnimationOrder(dMsg::Rcpt_c *rcpt) {
    if (rcpt->isSpeakerItchy()) {
        m3d::anmChr_c anm;
    }
}

// .text 0x528
dAcPlayer_c::dAcPlayer_c() {
    mInsectLink.mpInsect = NULL;
    mInsectLink.mpNext = NULL;
    mFishLink.mFish = NULL;
    mFishLink.mNext = NULL;
    _2188 = dItem::ITEM_ID_NONE;
    mActionItem = dItem::ITEM_ID_NONE;
    _218C = dItem::ITEM_ID_NONE;
    mpHeap = NULL;
    mpMiiHeap = NULL;
    mpHeadHeap = NULL;
    mpAccessoryHeap = NULL;
    mShowInsect = 0;
    mShowFish = 0;
    mpCloth = NULL;
    mpHeadItem = NULL;
    mpToolBank = NULL;
    _21B4 = 0;
    mEquipStep = EQUIP_STEP_NONE;
}

// .text 0x830
dAcPlayer_c::~dAcPlayer_c() {}

// .text 0x2BFB0: generated here (after the .inc files), but its new string literals put it in the
// section of event.inc, the file of the last data object generated before it.
int dAcPlayer_c::create() {
    if (mpHeap == NULL) {
        mpHeap = mHeap::createFrmHeap(dPlayerMgr::getGeneralHeapSize(), dHeap::playerHeap_p,
                                      "プレイヤー汎用ヒープ(dAcPlayer_c::m_heap)", 0x20, mHeap::OPT_NONE);
    }
    if (mpHeadHeap == NULL) {
        mpHeadHeap = mHeap::createFrmHeap(dPlayerMgr::getHeadHeapSize(), dHeap::playerHeap_p,
                                          "プレイヤー頭用ヒープ(dAcPlayer_c::m_heap_head_p)", 0x20, mHeap::OPT_NONE);
    }
    if (mpAccessoryHeap == NULL) {
        mpAccessoryHeap = mHeap::createFrmHeap(dPlayerMgr::getAccessoryHeapSize(), dHeap::playerHeap_p,
                                               "プレイヤーアクセサリ用ヒープ(dAcPlayer_c::m_heap_accessory_p)", 0x20,
                                               mHeap::OPT_NONE);
    }
    mPlayerNo = getParamPlayerNo(mParam);
    if (mpCloth == NULL) {
        mpCloth = dPlayerMngr_c::getCloth(mPlayerNo);
        if (mpCloth == NULL) {
            return 0;
        }
    }
    if (mpHeadItem == NULL) {
        mpHeadItem = dPlayerMngr_c::getHeadItem(mPlayerNo);
        if (mpHeadItem == NULL) {
            return 0;
        }
    }
    if (mpToolBank == NULL) {
        mpToolBank = dPlayerMngr_c::getToolBank(mPlayerNo);
        if (mpToolBank == NULL) {
            return 0;
        }
    }
    initEquipment();
    initHeldTool();
    u32 bit29 = getParamBit29(mParam);
    BOOL a = checkToolLoaded(bit29);
    BOOL b = checkEquipLoaded(bit29);
    if ((!a || !b) && bit29) {
        return 0;
    }
    if (a && mHeldEquipStep == EQUIP_STEP_LOAD_INIT) {
        mHeldEquipStep = EQUIP_STEP_INIT_LOADED;
    }
    if (mPlayerNo == dNet::getMyNo() &&
        dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) &&
        !dStrm_c::isLoaded()) {
        return 0;
    }
    setDemoID((u16)mPlayerNo);
    mAllocator.attach(mpHeap, 0x20);
    mHeadAllocator.attach(mpHeadHeap, 0x20);
    mAccessoryAllocator.attach(mpAccessoryHeap, 0x20);
    createMiiHeap();
    _2214 = getHairNo();
    _2218 = getSkinColorNo();
    _221C = getTanLevel();
    if (dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo)->isFlag0(PRIVATE_FLAG0_BEE_STUNG)) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_BEE_STUNG;
    }
    createModels();
    initActor();
    setModelMtxDirect();
    calcSound();
    setStartState();
    setStartAnm();
    if (mPlayerNo == dNet::getMyNo()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_LOCAL_PLAYER;
    }
    dPlayerMgr::setActor(this, mPlayerNo);
    dPlayerMgr::setChangeStateCallback(changeStateCallback, mPlayerNo);
    return 1;
}

// .text 0xABC
int dAcPlayer_c::doDelete() {
    if (mInsectLink.mpInsect != NULL) {
        static_cast<dInsFldBase_c *>(mInsectLink.mpInsect)->cancelHold();
        if (mInsectLink.mpInsect != NULL) {
            dInsectField::removeNpcLink(mInsectLink.mpInsect, &mInsectLink);
        }
    }
    if (mFishLink.mFish != NULL) {
        mFishLink.mFish->requestDelete();
        if (mFishLink.mFish != NULL) {
            mFishLink.mFish->removeLink(&mFishLink);
        }
    }
    if (mPlayerNo != dNet::getMyNo()) {
        dPlayerMgr::updateActionFieldTasks(mPlayerNo, dScene::getCurrentScene(), (u8)mState,
                                           (const u8 *)&mRemoteActionParams);
    }
    dPlayerMngr_c::resetEquipStates(mPlayerNo);
    dPlayerMgr::setActor(NULL, mPlayerNo);
    dPlayerMgr::setChangeStateCallback(NULL, mPlayerNo);
    if (mpHeap != NULL) {
        mMdl.remove();
        mAnm.remove();
        mPartAnm.remove();
        mHeap::destroyFrmHeap(mpHeap);
        mpHeap = NULL;
    }
    if (mpMiiHeap != NULL) {
        mFace.remove();
        mHeap::destroyFrmHeap(mpMiiHeap);
        mpMiiHeap = NULL;
    }
    if (mpHeadHeap != NULL) {
        releaseHairModel();
        mHeap::destroyFrmHeap(mpHeadHeap);
        mpHeadHeap = NULL;
    }
    if (mpAccessoryHeap != NULL) {
        releaseAccessoryModel();
        mHeap::destroyFrmHeap(mpAccessoryHeap);
        mpAccessoryHeap = NULL;
    }
    mpCloth = NULL;
    mpHeadItem = NULL;
    mpToolBank = NULL;
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_LOCAL_PLAYER) && mPlayerNo == 0 && mPlayerNo != dNet::getMyNo()) {
        dPlayerMngr_c::requestSync();
    }
    return 1;
}

// .text 0xC90
int dAcPlayer_c::draw() {
    drawMain();
    return 1;
}

// .text 0xCB4
int dAcPlayer_c::execute() {
    executeMain();
    return 1;
}

// .text 0xCD8
void dAcPlayer_c::executeMain() {
    if (mPlayerNo == dNet::getMyNo()) {
        calcPadInput();
    } else {
        syncRemoteState();
    }
    const mVec3_c *push = mObjc.getPush();
    mPos.x += push->x;
    mPos.y += push->y;
    mPos.z += push->z;
    clearFrameRequests();
    if (sStateProcs[mState].mExecute) {
        (this->*sStateProcs[mState].mExecute)();
    }
    calcMove();
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_NO_OBJC) {
        mObjc.init();
    } else {
        mObjc.regist();
    }
    calcState77Bg();
    calcBgCheck();
    mMdl.play();
    updateFaceAnm();
    calcModelMtx();
    calcShadow();
    checkExit();
    updateNetRecords();
}

// .text 0xE20
void dAcPlayer_c::drawMain() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED) {
        return;
    }
    if (mPlayerNo != dNet::getMyNo()) {
        if (isOutOfCameraRange()) {
            return;
        }
        if (dPlayerMngr_c::isEquipChecked(mPlayerNo)) {
            return;
        }
    }
    mMdl.entry();
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
            mFace.entry();
        }
    } else {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
            mHeadMdl.entry();
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL) {
            mHairMdl.entry();
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
            mAccessoryMdl.entry();
        }
    }
    if ((mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && !(mActorFlags & dPlayerMgr::ACTOR_FLAG_HIDE_TOOL)) {
        mpToolBank->draw();
    }
    if (mFgObjItem != dItem::ITEM_ID_NONE) {
        mAng3_c angle(0, 0, 0);
        dFgobjManager::fgObjCallback(mFgObjItem, (int)&_20C8, (int)&_20D4, &angle, 0);
    }
    mAng shadowAngle(0);
    f32 scale = mShadowScale;
    f32 alpha = mShadowAlpha;
    dShadow::requestRound(&mShadowPos, mShadowType, &shadowAngle, 16.0f, scale, alpha,
                          0.0f, 1.0f);
}

// .text 0xFAC
void dAcPlayer_c::calcState77Bg() {
    if (mState == dPlayerMgr::STATE_TRIP) {
        f32 scale = 2.5f;
        f32 pushRate = 1.0f;
        if (mAcch2.mHitFlags & 0x80) {
            if (mAnmId == HMN_ANM_TUMBLE_DOWNA1 || mAnmId == HMN_ANM_TUMBLE_DOWNB1) {
                if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CITY_FIELD)) {
                    scale = 10.5f;
                    pushRate = 0.3f;
                }
            }
        }
        calcPosAtAngle(&_20B0, &mMdlCallback.mHeadPos, mAngle.y.mAngle, scale);
        mAcch2.check(15.0f, &_20B0, &_20B0, mAngle.y, 0, 0x17, 1);
        mPos.x += pushRate * mAcch2.mPushOut.x;
        mPos.z += pushRate * mAcch2.mPushOut.z;
    }
}

// .text 0x10BC
void dAcPlayer_c::calcBgCheck() {
    if (mPlayerNo != dNet::getMyNo()) {
        mAcch.clear();
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN) ||
            (mState != dPlayerMgr::STATE_EXIT_ROOM &&
             dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_ROOM | dScene::SCENE_ATTR_FTR_ROOM |
                                        dScene::SCENE_ATTR_MUSEUM))) {
            mPos.y = dPlayerMgr::getIntermediateGroundHeight(&mTargetPos, &mPos, &mLastPos);
        }
        return;
    }
    switch (l_bgCheckKind[mState]) {
    case 0:
        dFTR::getHoldList()->clear();
        snapToGround();
        break;
    case 1:
        dFTR::getHoldList()->clear();
        mAcch.clear();
        break;
    case 2:
        dFTR::getHoldList()->clear();
        snapToGround();
        if (_2098.x == mMdlCallback.mSoundPos.x) {
            mPos.z = _2098.z;
        } else {
            mPos.x = _2098.x;
        }
        break;
    case 3:
        break;
    case 4:
        mAcch.clear();
        break;
    case 5:
        mPos.y -= 5.0f;
        mAcch.check(15.0f, &mPos, &mLastPos, mAngle.y, (int)this, 0x97, 1);
        break;
    case 6:
        dFTR::getHoldList()->clear();
        snapToGround();
        mPos.y = dBGCF::getUnitY(&mPos);
        break;
    case 7:
        dFTR::getHoldList()->clear();
        mAcch.clear();
        mPos.y = dPlayerMgr::getIntermediateGroundHeight(&mTargetPos, &mPos, &mLastPos);
        break;
    }
    if (mPlayerNo == dNet::getMyNo()) {
        dObjc::ins_c::clearList();
    }
}

// .text 0x1280
void dAcPlayer_c::calcShadow() {
    static f32 sMoveLimitSq = 12.25f;
    BOOL setPos = TRUE;
    f32 groundY = dBGCF::getGroundY(&mMdlCallback.mBasePos, FALSE);
    switch (mState) {
    case dPlayerMgr::STATE_STAIRS_TURN:
    case dPlayerMgr::STATE_GO_UPSTAIRS:
    case dPlayerMgr::STATE_ARRIVE_UPSTAIRS:
    case dPlayerMgr::STATE_GO_DOWNSTAIRS:
    case dPlayerMgr::STATE_ARRIVE_DOWNSTAIRS:
        groundY = mMdlCallback.mRootY;
        mShadowType = 0;
        break;
    case dPlayerMgr::STATE_BUS_BOARD:
        groundY = mMdlCallback.mRootY;
        mShadowType = dShadow::DRAW_QUAD;
        if (mAnm.getFrame() <= 55.0f && dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
            mShadowType = dShadow::DRAW_QUAD | dShadow::NO_DEPTH_TEST;
        }
        break;
    case dPlayerMgr::STATE_BUS_LEAVE:
        groundY = mMdlCallback.mRootY;
        mShadowType = dShadow::DRAW_QUAD;
        if (mAnm.getFrame() >= 118.0f && dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_TOWN)) {
            mShadowType = dShadow::DRAW_QUAD | dShadow::NO_DEPTH_TEST;
        }
        break;
    case dPlayerMgr::STATE_BUS_BOARDED:
    case dPlayerMgr::STATE_BUS_RIDE:
    case dPlayerMgr::STATE_BUS_GET_OFF:
    case dPlayerMgr::STATE_BUS_GOT_OFF:
        groundY = mMdlCallback.mRootY;
        mShadowType = dShadow::DRAW_QUAD;
        break;
    case dPlayerMgr::STATE_AXE_SWING:
    case dPlayerMgr::STATE_AXE_STRIKE:
    case dPlayerMgr::STATE_AXE_CHOP:
    case dPlayerMgr::STATE_AXE_HIT:
    case dPlayerMgr::STATE_AXE_MISS:
    case dPlayerMgr::STATE_AXE_HIT_OBJECT:
    case dPlayerMgr::STATE_SHOVEL_START:
    case dPlayerMgr::STATE_SHOVEL_WAIT:
    case dPlayerMgr::STATE_SHOVEL_DIG:
    case dPlayerMgr::STATE_SHOVEL_HIT:
    case dPlayerMgr::STATE_SHOVEL_SWING_LOW:
    case dPlayerMgr::STATE_SHOVEL_BURY:
    case dPlayerMgr::STATE_SHOVEL_DIG_UP_ITEM:
    case dPlayerMgr::STATE_SHOVEL_HIT_OBJECT: {
        mVec3_c base(mMdlCallback.mBasePos.x, groundY, mMdlCallback.mBasePos.z);
        mVec3_c move = mPos - mLastPos;
        mVec3_c next = base + move;
        mVec3_c diff = next - mPos;
        if (diff.x * diff.x + diff.z * diff.z >= sMoveLimitSq) {
            setPos = FALSE;
            mShadowPos += move;
            mShadowPos.y = groundY;
        }
        mShadowType = 0;
        break;
    }
    default:
        mShadowType = 0;
        break;
    }
    if (setPos) {
        if (mPlayerNo != dNet::getMyNo()) {
            if (groundY - mPos.y > 32.0f) {
                groundY = mPos.y;
                mShadowType = dShadow::DRAW_QUAD;
            }
        }
        mShadowPos.set(mMdlCallback.mBasePos.x, groundY, mMdlCallback.mBasePos.z);
    }
}

// .text 0x1520
void dAcPlayer_c::checkExit() {
    if (mPlayerNo != dNet::getMyNo()) {
        return;
    }
    switch (mState) {
    case dPlayerMgr::STATE_IDLE:
    case dPlayerMgr::STATE_WALK:
    case dPlayerMgr::STATE_RUN: {
        int exit = dBGCF::getOpenNext(&mPos);
        if (exit == -1) {
            return;
        }
        mExitNo = exit;
        u32 type = 5;
        mVec3_c pos = mPos;
        s16 angle = 0;
        dAPlayer::getSceneChange()->getExitPos(exit, &type, &pos, &angle, &mPos);
        mExitType = type;
        mTargetPos = pos;
        mTargetAngle = angle;
        switch (type) {
        case 3:
        case 4:
            if (dDemo::request10()) {
                setState(dPlayerMgr::STATE_EXIT_ROOM);
            }
            break;
        case 1:
        case 2:
            if (dDemo::request10()) {
                setState(dPlayerMgr::STATE_STAIRS_TURN);
            }
            break;
        case 0:
            dDemo::requestExitScene(this, exit);
            break;
        default:
            dDemo::requestExitScene(this, exit);
            break;
        }
        break;
    }
    }
}

// .text 0x1674
void dAcPlayer_c::initActor() {
    mManpu.init();
    mState = dPlayerMgr::STATE_NONE;
    mScale = 1.0f;

    for (int i = 0; i < 6; i++) {
        _22FE[i] = 0;
    }

    mAnmId = HMN_ANM_NUM;
    _2224 = 0x1D3;
    _2228 = 0x1D3;
    mObjc.set(this, 16.0f, 32.0f, dObjc::TYPE_PLAYER, dObjc::MASK_DEFAULT, 0x80, 0);
    mObjcPos.set(this, mPos, 16.0f, 32.0f, 4, 0x8000003B, 0, 0);
    mObjc.mParam = mPlayerNo;
    mObjcPos.mParam = mPlayerNo;
    mObjcPos2.mParam = mPlayerNo;
    mUnitX = worldToUnit(mPos.x);
    mUnitZ = worldToUnit(mPos.z);
    mActionUnitX = worldToUnit(mPos.x);
    mActionUnitZ = worldToUnit(mPos.z);
    _229C = -1;
    _22A0 = -1;
    mExitNo = -1;
    mMsgStep = MSG_STEP_NONE;
    mStep = 0x22;
    _2188 = dItem::ITEM_ID_NONE;
    mActionItem = dItem::ITEM_ID_NONE;
    _218C = dItem::ITEM_ID_NONE;
    mTaskKind = FG_MNG_KIND_DROP;
    mTaskResult = dFgobjManager::REQUEST_RESULT_WAIT;
    _2266 = dItem::ITEM_ID_NONE;
    _2278 = 0;
    _227C = 0;
    _22E7[0] = 0;
    _22E7[1] = 0;
    mFgObjItem = dItem::ITEM_ID_NONE;
    mMdlCallback.mSoundPos.set(mPos);
    mMdlCallback.mBasePos = mPos;
    mMdlCallback.mHandPos = mPos;
    mMdlCallback.mRHandPos = mPos;
    mMdlCallback.mRFootPos = mPos;
    mMdlCallback.mLFootPos = mPos;
    mMdlCallback.mHeadPos = mPos;
    mMdlCallback.mCurveBaseY = mMdlCallback.mHeadPos.y;
    mShadowPos.x = mPos.x;
    mShadowPos.y = mPos.y;
    mShadowPos.z = mPos.z;
    mShadowAlpha = 1.0f;
    mShadowScale = 1.0f;
    mShadowType = 0;
    mToolScale.set(0.0f, 0.0f, 0.0f);
    mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_TOOL_ACTIVE;
    mFaceAnmMng.resetBlink();
    mFaceAnmMng.calcBlink();
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
        mFace.setPriorityDraw(0x7F, 8);
    }
    mFace.mBrightness = 0xFF;
    if (mPlayerNo == dNet::getMyNo()) {
        mAudioObj.setMaxSpeed(l_moveParam.mMaxSpeed);
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE | dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO;
    } else {
        mVoiceObj.setMaxSpeed(l_moveParam.mMaxSpeed);
        mActorFlags &= ~dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO;
        if (dScene::isCurrentSceneAttr(dScene::SCENE_ATTR_CHKP_ROOM)) {
            mActorFlags |= dPlayerMgr::ACTOR_FLAG_SCRIPTED_MOVE;
        }
        mObjc.mWeight = 0xFF;
    }
    if (dScene::isWinterSeasonType()) {
        mActorFlags |= dPlayerMgr::ACTOR_FLAG_WINTER;
    }
    u8 prev = dScene::getPrevScene();
    if (prev == dScene::SCENE_DM_BUS_TO_TOWN || prev == dScene::SCENE_DM_BUS_TO_LAND) {
        mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_FROM_BUS;
    }
    mRemoteSyncA = dPlayerMgr::STATE_NONE;
}

// .text 0x1990
void dAcPlayer_c::calcModelMtx() {
    mMtx_c mtx;
    mMtx_c scale;
    mMdlCallback.mBaseRotY = -mAngle.x.mAngle;
    mAng angle = getAngleY();
    dPlayerMgr::makeCurvedYawMatrix(&mtx, &mPos, &angle);
    PSMTXScale(scale, mScale, mScale, mScale);
    PSMTXConcat(mtx, scale, mtx);
    mMdl.setLocalMtx(&mtx);
    mMdl.calc(false);
    mModelMtx = mtx;
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
            setFaceMtxFromHead();
            mFace.calc(false);
        }
    } else {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
            setHeadPartMtx(&mHeadMdl);
            mHeadMdl.calc(false);
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL) {
            setHeadPartMtx(&mHairMdl);
            mHairMdl.calc(false);
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
            setHeadPartMtx(&mAccessoryMdl);
            mAccessoryMdl.calc(false);
        }
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
        if (mState != dPlayerMgr::STATE_GODDESS_ITEM) {
            mMtx_c toolScale;
            getRHandMtx(&mToolMtx);
            PSMTXScale(toolScale, mToolScale.x, mToolScale.y, mToolScale.z);
            PSMTXConcat(mToolMtx, toolScale, mToolMtx);
        }
        mpToolBank->calc(&mToolMtx);
        if (mpToolBank->getNodeMtx(&mModelMtx, getToolNodeIdx())) {
            mScriptedActionPos.set(mModelMtx._03, mModelMtx._13, mModelMtx._23);
            dWorld::toFieldPosition(&mScriptedActionPos, &mScriptedActionPos);
        } else {
            mScriptedActionPos = mMdlCallback.mRHandPos;
        }
    } else {
        mScriptedActionPos = mMdlCallback.mRHandPos;
    }
}

// .text 0x1BDC
void dAcPlayer_c::setHeadPartMtx(m3d::mdl_c *mdl) {
    mMtx_c mtx;
    getHeadMtx(&mtx);
    mdl->setLocalMtx(&mtx);
}

// .text 0x1C18
void dAcPlayer_c::getNodeMtx(mMtx_c *mtx, const char *name) {
    nw4r::g3d::ResMdl mdl = mMdl.getResMdl();
    mMdl.getNodeWorldMtx(mdl.GetResNode(name).GetID(), mtx);
}

// .text 0x1C90
void dAcPlayer_c::getRootMtx(mMtx_c *mtx) {
    getNodeMtx(mtx, "root");
}

// .text 0x1C9C
void dAcPlayer_c::getHeadMtx(mMtx_c *mtx) {
    getNodeMtx(mtx, "head");
}

// .text 0x1CA8
void dAcPlayer_c::getRHandMtx(mMtx_c *mtx) {
    getNodeMtx(mtx, "Rhand");
}

// .text 0x1CB4
void dAcPlayer_c::setModelMtxDirect() {
    static const f32 l_headY = 15.7766f;
    static const f32 l_rodX = -4.0793f;
    static const f32 l_rodY = 13.358f;
    static const f32 l_rodZ = 8.6625f;

    mMtx_c mtx;
    mAng angle = getAngleY();
    dPlayerMgr::makeCurvedYawMatrix(&mtx, &mPos, &angle);
    mMdl.setLocalMtx(&mtx);
    mModelMtx = mtx;
    mMtx_c headMtx;
    mVec3_c headPos(mPos.x, l_headY + mPos.y, mPos.z);
    mAng headAngle = getAngleY();
    dPlayerMgr::makeCurvedYawMatrix(&headMtx, &headPos, &headAngle);
    mMdlCallback.mFeelMtx = headMtx;
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_MASK) {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_MII_FACE) {
            setFaceMtx(&headMtx);
        }
    } else {
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HEAD_MDL) {
            mHeadMdl.setLocalMtx(&headMtx);
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_HAIR_MDL) {
            mHairMdl.setLocalMtx(&headMtx);
        }
        if (mActorFlags & dPlayerMgr::ACTOR_FLAG_ACCESSORY_MDL) {
            mAccessoryMdl.setLocalMtx(&headMtx);
        }
    }
    mMtx_c rodMtx;
    mVec3_c rodPos(l_rodX + mPos.x, l_rodY + mPos.y, l_rodZ + mPos.z);
    mAng rodAngle = getAngleY();
    dPlayerMgr::makeCurvedYawMatrix(&rodMtx, &rodPos, &rodAngle);
    mpToolBank->mFloat.setRodMtx(&rodMtx);
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) {
        mpToolBank->setLocalMtx(&mtx);
    }
}

// .text 0x1EE0
u32 dAcPlayer_c::getParamPlayerNo(u32 param) {
    return param >> 30;
}

// .text 0x1EE8
u32 dAcPlayer_c::getParamBit29(u32 param) {
    return (param >> 29) & 1;
}

// .text 0x1EF0
mVec3_c *dAcPlayer_c::demoHook58() {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED) {
        return NULL;
    }
    return &mMdlCallback.mHeadPos;
}

// .text 0x1F0C
int dAcPlayer_c::demoHook64(mVec3_c *pos) {
    if (pos == NULL || (mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED)) {
        return 0;
    }
    pos->set(mMdlCallback.mHandPos.x, mMdlCallback.mHandPos.y, mMdlCallback.mHandPos.z);
    return 1;
}

// .text 0x1F48
void dAcPlayer_c::setLookAngles(s16 yaw, s16 tilt, s16 a, s16 b) {
    mLookYaw = yaw;
    mLookTilt = tilt;
    _22CA = a;
    _22CC = b;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_LOOK_AT;
}

// .text 0x1F68
void dAcPlayer_c::lookAt(const mVec3_c *target, s16 a, s16 b) {
    s16 tilt = 0;
    s16 yaw = 0;
    if (target != NULL) {
        mVec3_c diff = mMdlCallback.mHeadPos - *target;
        f32 dist = EGG::Math<f32>::sqrt(diff.x * diff.x + diff.z * diff.z);
        f32 dy = target->y - mMdlCallback.mHeadPos.y;
        tilt = cM::atan2s(dy, dist);
        if (tilt > dPlayerMgr::sLookPitchMax) {
            tilt = dPlayerMgr::sLookPitchMax;
        } else if (tilt < -dPlayerMgr::sLookPitchMax) {
            tilt = -dPlayerMgr::sLookPitchMax;
        }
        yaw = dActor_c::targetAngleY(&mPos, target) - mAngle.y.mAngle;
        if (yaw > dPlayerMgr::sLookYawMax) {
            yaw = dPlayerMgr::sLookYawMax;
        } else if (yaw < -dPlayerMgr::sLookYawMax) {
            yaw = -dPlayerMgr::sLookYawMax;
        }
    }
    mLookTilt = tilt;
    mLookYaw = yaw;
    _22CA = a;
    _22CC = b;
    mActorFlags |= dPlayerMgr::ACTOR_FLAG_LOOK_AT;
}

// .text 0x2080
int dAcPlayer_c::getTanLevel() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0;
    }
    int tan = priv->mTan >> 1;
    if (tan < 0 || tan >= 8) {
        tan = 0;
    }
    return tan;
}

// .text 0x20D0
int dAcPlayer_c::getSkinColorNo() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 8;
    }
    u8 color = priv->mHairColor;
    int no = color * 8 + getTanLevel() + 8;
    if (no < 8 || no >= 0x48) {
        no = 8;
    }
    return no;
}

// .text 0x2140
int dAcPlayer_c::getShoeNo() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0x48;
    }
    int no = priv->mShoeColor + 0x48;
    if (priv->mShoeKind == 1) {
        no += 10;
    }
    if (no < 0x48 || no >= 0x5C) {
        if (priv->mPID.player.mGender == 0) {
            return 0x48;
        }
        return 0x52;
    }
    return no;
}

// .text 0x21C0
u32 dAcPlayer_c::getFaceNo() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0;
    }
    u32 no = priv->mFace;
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_BEE_STUNG) {
        no += 0x10;
    }
    if (no >= 0x20) {
        no = 0;
    }
    return no;
}

// .text 0x2224
int dAcPlayer_c::getGenderBodyNo() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0x20;
    }
    int no = 0x21;
    if (priv->mPID.player.getGender() == 0) {
        no = 0x20;
    }
    return no;
}

// .text 0x226C
int dAcPlayer_c::getShoeNo2() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0x22;
    }
    int no = priv->mShoeColor + 0x22;
    if (priv->mShoeKind == 1) {
        no += 10;
    }
    if (no < 0x22 || no >= 0x36) {
        if (priv->mPID.player.mGender == 0) {
            return 0x22;
        }
        return 0x2C;
    }
    return no;
}

// .text 0x22EC
u32 dAcPlayer_c::getHairNo() {
    dPrivateData_c *priv = dPlayerMgr::Mgr_c::getNetPlayer(mPlayerNo);
    if (priv == NULL) {
        return 0;
    }
    u32 no = priv->mHair;
    if (no >= 0x1A) {
        no = 0;
    }
    return no;
}

// .text 0x2334
void dAcPlayer_c::checkUnitChange() {
    int unitX = worldToUnit(mPos.x);
    int unitZ = worldToUnit(mPos.z);
    if (unitX == mUnitX && unitZ == mUnitZ) {
        return;
    }
    BOOL winter = (mActorFlags & dPlayerMgr::ACTOR_FLAG_WINTER) != 0;
    int min = winter ? 8 : 4;
    int max = winter ? 12 : 8;
    u8 wear = min + cM::rndInt(max - min);
    dFdBase_c *field = dField::getInfo(0);
    if (field != NULL) {
        field->wearGrass(&mLastPos, wear, 0);
    }
    if (mPlayerNo == dNet::getMyNo() && mSpeedF > l_moveParam.mRunSpeed) {
        field = dField::getInfo(0);
        if (field != NULL) {
            dItem::Item *item = field->getItem(unitX, unitZ, 0);
            if (item != NULL && item->isAnyFlower()) {
                if (!fgMngProc_trampleFlowerAt(unitX, unitZ)) {
                    startRumble(dRumble::PATTERN_TRAMPLE_FLOWER);
                }
            }
        }
    }
    mUnitX = unitX;
    mUnitZ = unitZ;
}

// .text 0x248C
BOOL dAcPlayer_c::isOutOfCameraRange() {
    const mVec3_c &center = dCamera_c::sCenter;
    if (mPos.x < center.x - 320.0f || mPos.x > center.x + 320.0f || mPos.z < center.z - 576.0f ||
        mPos.z > center.z + 224.0f) {
        return TRUE;
    }
    return FALSE;
}

// .text 0x24FC
u8 dAcPlayer_c::objcWeightTalk(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    if (other->getID() == l_talkActorID) {
        *rate = 0.1f;
        return 0x80;
    }
    if (!dNet::isSessionActive() && dDemo::isType(dDemo::TYPE_EVENT) &&
        (other->mType & dObjc::TYPE_NPC) == dObjc::TYPE_NPC && other->mWeight == 0xFF) {
        *rate = 0.3f;
        return 0x80;
    }
    return self->mWeight;
}

// .text 0x25CC
u8 dAcPlayer_c::objcWeightTalkOnly(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    if (other->getID() == l_talkActorID) {
        *rate = 0.1f;
        return 0x80;
    }
    return self->mWeight;
}

// .text 0x263C
u8 dAcPlayer_c::objcWeightEnter(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    if ((other->mType & dObjc::TYPE_ACTION) == dObjc::TYPE_ACTION) {
        dAcPlayer_c *player = dPlayerMgr::getActorRaw(other->mParam);
        if (player != NULL && player->mState == dPlayerMgr::STATE_ENTER_ROOM) {
            dAcPlayer_c *me = dPlayerMgr::getActorRaw(self->mParam);
            if (me != NULL && me->mState == dPlayerMgr::STATE_IDLE) {
                me->mActorFlags2 |= dPlayerMgr::ACTOR_FLAG2_PUSHED;
            }
        }
        return 0x80;
    }
    return self->mWeight;
}

// .text 0x26C4
u8 dAcPlayer_c::objcWeightHeight(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    if (!dNet::isSessionActive()) {
        dAcPlayer_c *me = dPlayerMgr::getActorRaw(self->mParam);
        if (me != NULL && me->_2200 <= 0.0f && (other->mType & dObjc::TYPE_SNOWBALL) == dObjc::TYPE_SNOWBALL) {
            me->_2200 = other->getHeight();
            if ((other->mType & dObjc::TYPE_NPC) == dObjc::TYPE_NPC ||
                (other->mType & dObjc::TYPE_PLAYER) == dObjc::TYPE_PLAYER) {
                if (me->_2200 <= other->getHeight()) {
                    me->_2200 = 0.0f;
                }
            }
        }
    }
    return self->mWeight;
}

// .text 0x2794
u8 dAcPlayer_c::objcWeightRemote(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    u32 type = other->mType;
    if ((type & dObjc::TYPE_ACTION) == dObjc::TYPE_ACTION) {
        return 0x80;
    }
    if ((type & dObjc::TYPE_PLAYER) == dObjc::TYPE_PLAYER) {
        int no = other->mParam;
        dAcPlayer_c *player = dPlayerMgr::getActorRaw(no);
        if (player != NULL && (u32)player->mState <= dPlayerMgr::STATE_WALK) {
            dAcPlayer_c *me = dPlayerMgr::getActorRaw(self->mParam);
            if (me != NULL && me->mState == dPlayerMgr::STATE_IDLE) {
                f32 x;
                f32 z;
                s16 angle;
                if (dPlayerMgr::getRemoteTransform(&x, &z, &angle, no)) {
                    f32 dist = nw4r::math::FAbs(player->mPos.x - me->mPos.x);
                    f32 remoteDist = nw4r::math::FAbs(player->mPos.x - x);
                    if (remoteDist > dist && remoteDist >= 32.0f) {
                        return 0xFF;
                    }
                    dist = nw4r::math::FAbs(player->mPos.z - me->mPos.z);
                    remoteDist = nw4r::math::FAbs(player->mPos.z - z);
                    if (remoteDist > dist && remoteDist >= 32.0f) {
                        return 0xFF;
                    }
                }
            }
        }
    }
    return self->mWeight;
}

// .text 0x28E0
u8 dAcPlayer_c::objcWeightFixed(const dObjc::base_c *self, const dObjc::base_c *other, f32 *rate) {
    if ((other->mType & dObjc::TYPE_NPC) == dObjc::TYPE_NPC && other->mWeight == 0xFF) {
        *rate = 0.3f;
        return 0x80;
    }
    return self->mWeight;
}

// .text 0x2920
void dAcPlayer_c::objcHit(dObjc::base_c *other) {
    if (dNet::isSessionActive()) {
        return;
    }
    if ((other->mType & dObjc::TYPE_SNOWBALL) == dObjc::TYPE_SNOWBALL) {
        dAcPlayer_c *player = dPlayerMgr::getActorRaw(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player != NULL) {
            int tool = player->mpToolBank->getToolType();
            if (tool == HMN_TOOL_SCOOP || tool == HMN_TOOL_AXE) {
                u32 param = other->mParam;
                if (player->_2278 == 0) {
                    player->_2278 = param;
                } else if (param != player->_2278 && player->_227C == 0) {
                    player->_227C = param;
                }
            }
        }
    }
    if ((other->mType & dObjc::TYPE_NPC) == dObjc::TYPE_NPC) {
        dAcPlayer_c *player = dPlayerMgr::getActorRaw(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player != NULL && player->_2278 == 0 && player->_227C == 0 &&
            player->_2266 == dItem::ITEM_ID_NONE) {
            f32 dy = other->getPos()->y - player->mPos.y;
            if (dy < 25.0f && dy > -30.0f) {
                player->_2266 = other->mParam;
            }
        }
    }
}

// .text 0x2A64
void dAcPlayer_c::startSound(u32 id) {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO) {
        mAudioObj.startSound(id);
    } else {
        mVoiceObj.startSound(id);
    }
}

// .text 0x2A90
void dAcPlayer_c::startWeatherSound(u32 id, u32 rainId) {
    if (dWeather_c::sInstance != NULL && dWeather_c::isRainKind(dWeather_c::sInstance->mWeather)) {
        startSound(rainId);
    } else {
        startSound(id);
    }
}

// .text 0x2AD0
void dAcPlayer_c::holdSound(u32 id) {
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO) {
        mAudioObj.holdSound(id);
    } else {
        mVoiceObj.holdSound(id);
    }
}

// .text 0x2AFC
void dAcPlayer_c::startToolSound(u32 id) {
    mSndObj.startSound(id);
}

// .text 0x2B04
void dAcPlayer_c::startFootSound(int se, f32 speed) {
    u8 wear;
    mVec3_c pos(0.5f * (mMdlCallback.mRFootPos.x + mMdlCallback.mLFootPos.x),
                0.5f * (mMdlCallback.mRFootPos.y + mMdlCallback.mLFootPos.y),
                0.5f * (mMdlCallback.mRFootPos.z + mMdlCallback.mLFootPos.z));
    int id;
    if (se == -1) {
        id = dGroundAttr::getFootstepSe(&pos, &wear);
    } else {
        id = se;
    }
    if (speed <= 0.0f) {
        speed = mSpeedF;
    }
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO) {
        mAudioObj.startFootSound(id, wear, speed);
    } else {
        mVoiceObj.startFootSound(id, wear, speed);
    }
}

// .text 0x2C04
void dAcPlayer_c::checkFootSound() {
    if (mAnm.checkFrame(0.0f) || mAnm.checkFrame(16.0f)) {
        startFootSound(-1, 0.0f);
    }
}

// .text 0x2C70
void dAcPlayer_c::checkFootSound(f32 f0, f32 f1, f32 f2, f32 f3) {
    if (mAnm.checkFrame(f0) || mAnm.checkFrame(f1) || mAnm.checkFrame(f2) || mAnm.checkFrame(f3)) {
        startFootSound(-1, 0.0f);
    }
}

// .text 0x2D34
void dAcPlayer_c::calcSound() {
    BOOL visible = FALSE;
    if (mActorFlags & dPlayerMgr::ACTOR_FLAG_LOCAL_AUDIO) {
        mAudioObj.setPos(&mPos);
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED)) {
            visible = TRUE;
        }
    } else {
        mVoiceObj.setPos(&mPos);
        if (!(mActorFlags & dPlayerMgr::ACTOR_FLAG_DISABLED) && !isOutOfCameraRange()) {
            visible = TRUE;
        }
    }
    if (visible && (mActorFlags & dPlayerMgr::ACTOR_FLAG_TOOL_OUT) && mToolScale.x > 0.0f) {
        mSndObj.setPos(&mScriptedActionPos);
        int tool = mpToolBank->getToolType();
        if (tool == HMN_TOOL_WINDMILL) {
            f32 rate = mpToolBank->getWindRate();
            mPinwheelSnd.update(&mScriptedActionPos, rate);
            if (rate > 0.0f) {
                mPinwheelSnd.holdSound(SE_Kazaguruma_Lev);
            }
        } else if (tool == HMN_TOOL_UMBRELLA) {
            if (dWeather_c::sInstance != NULL) {
                int weather = dWeather_c::sInstance->mWeather;
                if (weather == dWeather::RAIN) {
                    holdSound(SE_LEV_RainUmbrella);
                } else if (weather == dWeather::HEAVY_RAIN) {
                    holdSound(SE_LEV_HeavyRainUmbrella);
                }
            }
        } else {
            mPinwheelSnd.update(&mPos, 0.0f);
        }
    } else {
        mSndObj.setPos(&mPos);
        mPinwheelSnd.update(&mPos, 0.0f);
    }
}
