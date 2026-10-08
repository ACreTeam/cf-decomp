#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "RUUE01_00",  # 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.3"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Verified for the Matching cLib objects; the others remain candidates.
# Start with NSMBW's code-generation flags and the imported header layout.
cflags_clib = [
    "-proc gekko",
    "-fp hard",
    "-O4",
    "-func_align 4",
    "-inline noauto",
    "-Cpp_exceptions off",
    "-enum int",
    "-RTTI off",
    "-ipa file",
    "-enc SJIS",
    "-nosyspath",
    "-i include",
    "-i include/lib",
    "-i include/lib/MSL",
    "-i include/lib/MSL/internal",
    "-DREVOLUTION",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
    cflags_clib.extend(["-sym dwarf-2", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")
    cflags_clib.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
    cflags_clib.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
    cflags_clib.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")
    cflags_clib.append("-W error")

# Identified engine and EGG code has RTTI. Other code-generation settings
# remain the cLib baseline until each imported implementation is matched.
cflags_identified_game = [
    *cflags_clib,
    "-RTTI on",
    "-i include/lib/revolution/BTE/include",
    "-i include/lib/revolution/BTE/gki/common",
    "-i include/lib/revolution/BTE/gki/platform",
    "-i include/lib/revolution/BTE/stack/include",
    "-i include/lib/revolution/BTE/stack/btm",
    "-i include/lib/revolution/BTE/bta/include",
    "-i include/lib/revolution/BTE/bta/sys",
]

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-i include/lib",
    "-i include/lib/MSL",
    "-i include/lib/MSL/internal",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

# REL flags: the game's flags, without small data (RELs have no .sdata / .sbss).
cflags_rel = [
    *cflags_identified_game,
    "-sdata 0",
    "-sdata2 0",
]

config.linker_version = "GC/3.0a5.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions








# --- imported from wii-ipl / mkw (begin) ---
# Sources under lib/ are copied verbatim from wii-ipl and mkw (both CC0)
# and built with those projects' own flags.
cflags_ext_ipl_revoex = [
    '-nodefaults',
    '-proc',
    'gekko',
    '-align',
    'powerpc',
    '-enum',
    'int',
    '-fp',
    'hardware',
    '-Cpp_exceptions',
    'off',
    '-W',
    'nomissingreturn',
    '-O4,p',
    '-inline',
    'auto',
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    '-maxerrors',
    '1',
    '-nosyspath',
    '-RTTI',
    'off',
    '-fp_contract',
    'on',
    '-str',
    'reuse',
    '-DSDK_IPL',
    '-D_REVOLUTION',
    '-DMEM_MANAGER_DIRECT',
    '-i lib/keyboard/include',
    '-i lib/common/include',
    '-i lib/common/include/global',
    '-i lib/MetroTRK/include',
    '-i lib/Runtime/include',
    '-i lib/MSL/include',
    '-i lib/RVL_SDK/include',
    '-i lib/RevoEX/include',
    '-i lib/NW4R/include',
    '-ir lib/RVL_SDK/include/private/bte',
    '-DBUILD_VERSION=0',
    '-DVERSION_43U',
    '-i lib/RVL_SDK/include/private/bte',
    '-DNDEBUG=1',
    '-DTARGET_RVL',
    
    '-ipa',
    'file',
    '-fp_contract',
    'off',
]
cflags_ext_ipl_sdk = [
    '-nodefaults',
    '-proc',
    'gekko',
    '-align',
    'powerpc',
    '-enum',
    'int',
    '-fp',
    'hardware',
    '-Cpp_exceptions',
    'off',
    '-W',
    'nomissingreturn',
    '-O4,p',
    '-inline',
    'auto',
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    '-maxerrors',
    '1',
    '-nosyspath',
    '-RTTI',
    'off',
    '-fp_contract',
    'on',
    '-str',
    'reuse',
    '-DSDK_IPL',
    '-D_REVOLUTION',
    '-DMEM_MANAGER_DIRECT',
    '-i lib/keyboard/include',
    '-i lib/common/include',
    '-i lib/common/include/global',
    '-i lib/MetroTRK/include',
    '-i lib/Runtime/include',
    '-i lib/MSL/include',
    '-i lib/RVL_SDK/include',
    '-i lib/RevoEX/include',
    '-i lib/NW4R/include',
    '-ir lib/RVL_SDK/include/private/bte',
    '-DBUILD_VERSION=0',
    '-DVERSION_43U',
    '-i lib/RVL_SDK/include/private/bte',
    '-DNDEBUG=1',
    '-DTARGET_RVL',
    '-ipa',
    'file',
    '-fp_contract',
    'off',
]
cflags_ext_mkw_gamespy = [
    '-nodefaults',
    '-align',
    'powerpc',
    '-enc',
    'SJIS',
    '-gccinc',
    '-proc',
    'gekko',
    '-enum',
    'int',
    '-O4,p',
    '-inline',
    'auto',
    '-fp',
    'hardware',
    '-Cpp_exceptions',
    'off',
    '-RTTI',
    'off',
    '-inline',
    'auto',
    '-nostdinc',
    '-DREVOKART',
    '-func_align',
    '4',
    '-i lib/gamespy/support/include',
    '-i lib/gamespy/support/MSL/include',
    '-i lib/gamespy/support/MSL/src',
    '-i lib',
    '-i lib/gamespy/support',
    '-DNDEBUG=1',
    '-lang=c99',
    '-ipa',
    'file',
    '-w',
    'nounusedexpr',
    '-w',
    'nounusedarg',
]
cflags_ext_ipl_keyboard = [
    '-nodefaults',
    '-proc',
    'gekko',
    '-align',
    'powerpc',
    '-enum',
    'int',
    '-fp',
    'hardware',
    '-Cpp_exceptions',
    'off',
    '-W',
    'nomissingreturn',
    '-O4,p',
    '-inline',
    'auto',
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    '-maxerrors',
    '1',
    '-nosyspath',
    '-RTTI',
    'off',
    '-fp_contract',
    'on',
    '-str',
    'reuse',
    '-DSDK_IPL',
    '-D_REVOLUTION',
    '-DMEM_MANAGER_DIRECT',
    '-i lib/keyboard/include',
    '-i lib/common/include',
    '-i lib/common/include/global',
    '-i lib/MetroTRK/include',
    '-i lib/Runtime/include',
    '-i lib/MSL/include',
    '-i lib/RVL_SDK/include',
    '-i lib/RevoEX/include',
    '-i lib/NW4R/include',
    '-DBUILD_VERSION=0',
    '-DVERSION_43U',
    '-DNDEBUG=1',
    '-DTARGET_RVL',
    '-ipa',
    'file',
    '-gccinc',
    '-fp_contract',
    'off',
    '-O4,s',
    '-O4,p',
]
# --- imported from wii-ipl / mkw (end) ---

config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "cLib",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_clib,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/cLib/c_counter.cpp"),
            Object(NonMatching, "dol/cLib/c_dylink.cpp"),
            Object(Matching, "dol/cLib/c_lib.cpp"),
            Object(Matching, "dol/cLib/c_line.cpp"),
            # No verified City Folk split yet for c_m3d or c_owner_set.
            Object(NonMatching, "dol/cLib/c_m3d.cpp"),
            Object(Matching, "dol/cLib/c_math.cpp"),
            Object(NonMatching, "dol/cLib/c_owner_set.cpp"),
            # Preserve getRandomF's original extab and extabindex records.
            Object(Matching, "dol/cLib/c_random.cpp", extra_cflags=["-Cpp_exceptions on"]),
            Object(Matching, "dol/cLib/c_tree.cpp"),
        ],
    },
    {
        "lib": "sLib",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/sLib/s_crc.cpp"),
            Object(Matching, "dol/sLib/s_lib.cpp"),
            Object(Matching, "dol/sLib/s_Phase.cpp"),
            Object(Matching, "dol/sLib/s_printf.cpp"),
        ],
    },
    {
        "lib": "framework",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/framework/f_arc_load.cpp"),
            Object(Matching, "dol/framework/f_base.cpp"),
            Object(Matching, "dol/framework/f_line.cpp"),
            Object(Matching, "dol/framework/f_manager.cpp"),
            Object(Matching, "dol/framework/f_tree.cpp"),
        ],
    },
    {
        "lib": "mLib",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/mLib/m_heap.cpp"),
            Object(NonMatching, "dol/mLib/m_mtx.cpp"),
            Object(NonMatching, "dol/mLib/m_allocator.cpp"),
            Object(NonMatching, "dol/mLib/m_3d.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/scn_leaf.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/calc_ratio.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/bmdl.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/smdl.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/mdl.cpp"),
            Object(Matching, "dol/mLib/m_3d/banm.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/fanm.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/anm_chr.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/anm_vis.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/anm_mat_clr.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/anm_tex_pat.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/anm_tex_srt.cpp"),
            Object(Matching, "dol/mLib/m_angle.cpp"),
            Object(Matching, "dol/mLib/m_vec.cpp"),
            Object(NonMatching, "dol/mLib/m_2d.cpp"),
            Object(NonMatching, "dol/mLib/m_fader.cpp"),
            Object(NonMatching, "dol/mLib/m_fader_base.cpp"),
            Object(NonMatching, "dol/mLib/m_wipe_fader.cpp"),
            Object(NonMatching, "dol/mLib/m_color_fader.cpp"),
            Object(NonMatching, "dol/mLib/m_frustum.cpp"),
            Object(NonMatching, "dol/mLib/m_3d/m_3d_capture.cpp"),
            Object(NonMatching, "dol/mLib/m_color.cpp"),
        ],
    },
    {
        "lib": "game",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/game/d_actor.cpp"),
            Object(Matching, "dol/game/d_base.cpp"),
            Object(Matching, "dol/game/d_bgc.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_field_block.cpp"),
            Object(Matching, "dol/game/d_demo_actor.cpp"),
            Object(Matching, "dol/game/d_field_info.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_event.cpp"),
            Object(Matching, "dol/game/d_screenshot.cpp"),
            Object(Matching, "dol/game/d_fireworks.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_fish_info.cpp"),
            Object(Matching, "dol/game/d_fg_data.cpp"),
            Object(NonMatching, "dol/game/d_field_assessment.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "dol/game/d_msg.cpp"),
            Object(Matching, "dol/game/d_fg_item.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_friend.cpp"),
            Object(Matching, "dol/game/d_insect_info.cpp"),
            Object(Matching, "dol/game/d_item.cpp", shift_jis=False, extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_nickname.cpp"),
            Object(Matching, "dol/game/d_notice.cpp"),
            Object(NonMatching, "dol/game/d_npc_notice.cpp"),
            Object(NonMatching, "dol/game/d_npc.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "dol/game/d_player_mgr.cpp"),
            Object(Matching, "dol/game/d_post_office.cpp"),
            Object(Matching, "dol/game/d_prc_mng.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_random_field.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_rec_bank.cpp"),
            Object(Matching, "dol/game/d_region.cpp"),
            Object(Matching, "dol/game/d_random.cpp"),
            Object(Matching, "dol/game/d_sv_auc.cpp"),
            Object(NonMatching, "dol/game/d_save_data.cpp"),
            Object(Matching, "dol/game/d_dsn.cpp"),
            Object(Matching, "dol/game/d_save_visitor_npc.cpp"),
            Object(Matching, "dol/game/d_save_mother_mail.cpp"),
            Object(Matching, "dol/game/d_save_main_field.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_home_room_map.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_model_room.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_save_box.cpp"),
            Object(Matching, "dol/game/d_bug_off.cpp"),
            Object(Matching, "dol/game/d_save_check.cpp"),
            Object(Matching, "dol/game/d_save_dl_item.cpp"),
            Object(Matching, "dol/game/d_save_town.cpp"),
            Object(Matching, "dol/game/d_land.cpp"),
            Object(Matching, "dol/game/d_mail.cpp"),
            Object(Matching, "dol/game/d_save_melody.cpp"),
            Object(Matching, "dol/game/d_save_money.cpp"),
            Object(Matching, "dol/game/d_museum.cpp"),
            Object(NonMatching, "dol/game/d_animal.cpp"),
            Object(Matching, "dol/game/d_animal_id.cpp"),
            Object(NonMatching, "dol/game/d_private_data.cpp"),
            Object(Matching, "dol/game/d_home.cpp"),
            Object(Matching, "dol/game/d_personal_id.cpp"),
            Object(Matching, "dol/game/d_quest.cpp"),
            Object(Matching, "dol/game/d_save_shops.cpp"),
            Object(NonMatching, "dol/game/d_save_shop.cpp"),
            Object(Matching, "dol/game/d_save_shop_stalk_market.cpp"),
            Object(Matching, "dol/game/d_save_catherine.cpp"),
            Object(Matching, "dol/game/d_save_shop_tailor.cpp"),
            Object(Matching, "dol/game/d_save_shop_gallery.cpp"),
            Object(Matching, "dol/game/d_save_shop_grace.cpp"),
            Object(Matching, "dol/game/d_save_cafe_guest.cpp"),
            Object(NonMatching, "dol/game/d_save_building.cpp"),
            Object(Matching, "dol/game/d_theater.cpp"),
            Object(Matching, "dol/game/d_time_stamp.cpp"),
            Object(Matching, "dol/game/d_ymd.cpp"),
            Object(Matching, "dol/game/d_sv_time_offset.cpp"),
            Object(Matching, "dol/game/d_search_cand.cpp"),
            Object(Matching, "dol/game/d_shop_layout.cpp"),
            Object(NonMatching, "dol/game/d_shutdown_fader.cpp"),
            Object(NonMatching, "dol/game/d_shutter_fader.cpp"),
            Object(Matching, "dol/game/d_side_wipe_fader.cpp"),
            Object(Matching, "dol/game/d_play_util.cpp"),
            Object(Matching, "dol/game/d_scene.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_police_box.cpp"),
            Object(Matching, "dol/game/d_recycle_bin.cpp"),
            Object(Matching, "dol/game/d_script.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "dol/game/d_str.cpp"),
            Object(Matching, "dol/game/d_string.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_date.cpp"),
            # Stored as CP932; pass directly to MWCC without UTF-8 conversion.
            Object(NonMatching, "dol/game/d_sv_mgr.cpp", shift_jis=False),
        ],
    },
    {
        "lib": "keyboard",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "game",
        "objects": [
            Object(NonMatching, "lib/keyboard/tiPcKeyboard.cpp"),
            Object(NonMatching, "lib/keyboard/tiCellPhone.cpp"),
            Object(NonMatching, "lib/keyboard/tiInputForm.cpp"),
            Object(NonMatching, "lib/keyboard/tiSignWindow.cpp"),
            Object(NonMatching, "lib/keyboard/tiString.cpp"),
            Object(NonMatching, "lib/keyboard/tiTextDrawer.cpp"),
            Object(NonMatching, "lib/keyboard/tiManager.cpp"),
            Object(NonMatching, "lib/keyboard/MyTiManager.cpp"),
            Object(NonMatching, "lib/keyboard/MyTiInputForm.cpp"),
            Object(NonMatching, "lib/keyboard/tiHKBManager.cpp"),
        ],
    },
    {
        "lib": "RevoEX",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24UserId.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24System.c"),
        ],
    },
    {
        "lib": "RVL_SDK",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "lib/RVL_SDK/os/OSAudioSystem.c"),
            Object(NonMatching, "lib/RVL_SDK/os/__ppc_eabi_init.cpp"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPADHIDParser.c"),
        ],
    },
    {
        "lib": "gamespy",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
        ],
    },
    # --- imported libs (begin) ---
    {
        "lib": "RevoEX (wii-ipl)",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_ext_ipl_revoex,
        "progress_category": "sdk",
        "src_dir": "lib",
        "objects": [
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Config.c", source="RevoEX/src/nwc24/NWC24Config.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24FileAPI.c", source="RevoEX/src/nwc24/NWC24FileAPI.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24FriendList.c", source="RevoEX/src/nwc24/NWC24FriendList.c"),
            Object(Matching, "lib/RevoEX/nwc24/NWC24Ipc.c", source="RevoEX/src/nwc24/NWC24Ipc.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Manage.c", source="RevoEX/src/nwc24/NWC24Manage.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24MsgObj.c", source="RevoEX/src/nwc24/NWC24MsgObj.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Parser.c", source="RevoEX/src/nwc24/NWC24Parser.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Schedule.c", source="RevoEX/src/nwc24/NWC24Schedule.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24SecretFList.c", source="RevoEX/src/nwc24/NWC24SecretFList.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24StdAPI.c", source="RevoEX/src/nwc24/NWC24StdAPI.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Time.c", source="RevoEX/src/nwc24/NWC24Time.c"),
            Object(Matching, "lib/RevoEX/nwc24/NWC24Utils.c", source="RevoEX/src/nwc24/NWC24Utils.c"),
            Object(NonMatching, "lib/RevoEX/ssl/ssl_api.c", source="RevoEX/src/ssl/ssl_api.c"),
            Object(Matching, "lib/RevoEX/vf/common/pf_clib.c", source="RevoEX/src/vf/common/pf_clib.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_code.c", source="RevoEX/src/vf/common/pf_code.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_service.c", source="RevoEX/src/vf/common/pf_service.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_str.c", source="RevoEX/src/vf/common/pf_str.c"),
            Object(Matching, "lib/RevoEX/vf/common/pf_w_clib.c", source="RevoEX/src/vf/common/pf_w_clib.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_common.c", source="RevoEX/src/vf/develop/d_common.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_hash.c", source="RevoEX/src/vf/develop/d_hash.c"),
            Object(Matching, "lib/RevoEX/vf/develop/d_time.c", source="RevoEX/src/vf/develop/d_time.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_vf.c", source="RevoEX/src/vf/develop/d_vf.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_vf_sys.c", source="RevoEX/src/vf/develop/d_vf_sys.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/nand_drv.c", source="RevoEX/src/vf/develop/nand_drv.c"),
            Object(NonMatching, "lib/RevoEX/vf/driver/pf_driver.c", source="RevoEX/src/vf/driver/pf_driver.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_bpb.c", source="RevoEX/src/vf/dskmng/pdm_bpb.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_disk.c", source="RevoEX/src/vf/dskmng/pdm_disk.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_dskmng.c", source="RevoEX/src/vf/dskmng/pdm_dskmng.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_mbr.c", source="RevoEX/src/vf/dskmng/pdm_mbr.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_partition.c", source="RevoEX/src/vf/dskmng/pdm_partition.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_cache.c", source="RevoEX/src/vf/fatfs/pf_cache.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_cluster.c", source="RevoEX/src/vf/fatfs/pf_cluster.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_entry.c", source="RevoEX/src/vf/fatfs/pf_entry.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_entry_iterator.c", source="RevoEX/src/vf/fatfs/pf_entry_iterator.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat.c", source="RevoEX/src/vf/fatfs/pf_fat.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat12.c", source="RevoEX/src/vf/fatfs/pf_fat12.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat16.c", source="RevoEX/src/vf/fatfs/pf_fat16.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat32.c", source="RevoEX/src/vf/fatfs/pf_fat32.c"),
            Object(Matching, "lib/RevoEX/vf/fatfs/pf_fatfs.c", source="RevoEX/src/vf/fatfs/pf_fatfs.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_file.c", source="RevoEX/src/vf/fatfs/pf_file.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_path.c", source="RevoEX/src/vf/fatfs/pf_path.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_sector.c", source="RevoEX/src/vf/fatfs/pf_sector.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_volume.c", source="RevoEX/src/vf/fatfs/pf_volume.c"),
            Object(NonMatching, "lib/RevoEX/vf/standard/pf_api_util.c", source="RevoEX/src/vf/standard/pf_api_util.c"),
            Object(Matching, "lib/RevoEX/vf/standard/pf_init_prfile2.c", source="RevoEX/src/vf/standard/pf_init_prfile2.c"),
            Object(Matching, "lib/RevoEX/vf/standard/pf_unmount.c", source="RevoEX/src/vf/standard/pf_unmount.c"),
            Object(Matching, "lib/RevoEX/vf/system/pf_filelock.c", source="RevoEX/src/vf/system/pf_filelock.c"),
            Object(NonMatching, "lib/RevoEX/vf/system/pf_system.c", source="RevoEX/src/vf/system/pf_system.c"),
            Object(Matching, "lib/RevoEX/wd/wd_init.c", source="RevoEX/src/wd/wd_init.c"),
            Object(NonMatching, "lib/RevoEX/wd/wd_misc.c", source="RevoEX/src/wd/wd_misc.c"),
            Object(NonMatching, "lib/RevoEX/wd/wd_request.c", source="RevoEX/src/wd/wd_request.c"),
        ],
    },
    {
        "lib": "RVL_SDK (wii-ipl)",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_ext_ipl_sdk,
        "progress_category": "sdk",
        "src_dir": "lib",
        "objects": [
            Object(NonMatching, "lib/RVL_SDK/ai/ai.c", source="RVL_SDK/src/ai/ai.c"),
            Object(NonMatching, "lib/RVL_SDK/arc/arc.c", source="RVL_SDK/src/arc/arc.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AX.c", source="RVL_SDK/src/ax/AX.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXAlloc.c", source="RVL_SDK/src/ax/AXAlloc.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXAux.c", source="RVL_SDK/src/ax/AXAux.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXCL.c", source="RVL_SDK/src/ax/AXCL.c"),
            Object(Matching, "lib/RVL_SDK/ax/AXOut.c", source="RVL_SDK/src/ax/AXOut.c"),
            Object(Matching, "lib/RVL_SDK/ax/AXProf.c", source="RVL_SDK/src/ax/AXProf.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXSPB.c", source="RVL_SDK/src/ax/AXSPB.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXVPB.c", source="RVL_SDK/src/ax/AXVPB.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHi.c", source="RVL_SDK/src/axfx/AXFXReverbHi.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHiExp.c", source="RVL_SDK/src/axfx/AXFXReverbHiExp.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHiExpDpl2.c", source="RVL_SDK/src/axfx/AXFXReverbHiExpDpl2.c"),
            Object(NonMatching, "lib/RVL_SDK/base/PPCArch.c", source="RVL_SDK/src/base/PPCArch.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bd.c", source="RVL_SDK/src/bte/bd.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_act.c", source="RVL_SDK/src/bte/bta_dm_act.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_api.c", source="RVL_SDK/src/bte/bta_dm_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_main.c", source="RVL_SDK/src/bte/bta_dm_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_pm.c", source="RVL_SDK/src/bte/bta_dm_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_act.c", source="RVL_SDK/src/bte/bta_hh_act.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_api.c", source="RVL_SDK/src/bte/bta_hh_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_main.c", source="RVL_SDK/src/bte/bta_hh_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_utils.c", source="RVL_SDK/src/bte/bta_hh_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_sys_conn.c", source="RVL_SDK/src/bte/bta_sys_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_sys_main.c", source="RVL_SDK/src/bte/bta_sys_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_hcisu.c", source="RVL_SDK/src/bte/bte_hcisu.c"),
            Object(Matching, "lib/RVL_SDK/bte/bte_init.c", source="RVL_SDK/src/bte/bte_init.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_logmsg.c", source="RVL_SDK/src/bte/bte_logmsg.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_main.c", source="RVL_SDK/src/bte/bte_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_acl.c", source="RVL_SDK/src/bte/btm_acl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_dev.c", source="RVL_SDK/src/bte/btm_dev.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_devctl.c", source="RVL_SDK/src/bte/btm_devctl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_discovery.c", source="RVL_SDK/src/bte/btm_discovery.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_inq.c", source="RVL_SDK/src/bte/btm_inq.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_main.c", source="RVL_SDK/src/bte/btm_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_pm.c", source="RVL_SDK/src/bte/btm_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_sco.c", source="RVL_SDK/src/bte/btm_sco.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_sec.c", source="RVL_SDK/src/bte/btm_sec.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_hcif.c", source="RVL_SDK/src/bte/btu_hcif.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_init.c", source="RVL_SDK/src/bte/btu_init.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_task1.c", source="RVL_SDK/src/bte/btu_task1.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_api.c", source="RVL_SDK/src/bte/gap_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_conn.c", source="RVL_SDK/src/bte/gap_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_utils.c", source="RVL_SDK/src/bte/gap_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_buffer.c", source="RVL_SDK/src/bte/gki_buffer.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_ppc.c", source="RVL_SDK/src/bte/gki_ppc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_time.c", source="RVL_SDK/src/bte/gki_time.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hcicmds.c", source="RVL_SDK/src/bte/hcicmds.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hcisu_h2.c", source="RVL_SDK/src/bte/hcisu_h2.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_api.c", source="RVL_SDK/src/bte/hidd_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_conn.c", source="RVL_SDK/src/bte/hidd_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_mgmt.c", source="RVL_SDK/src/bte/hidd_mgmt.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_pm.c", source="RVL_SDK/src/bte/hidd_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidh_api.c", source="RVL_SDK/src/bte/hidh_api.c"),
            Object(Matching, "lib/RVL_SDK/bte/hidh_conn.c", source="RVL_SDK/src/bte/hidh_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_api.c", source="RVL_SDK/src/bte/l2c_api.c"),
            Object(Matching, "lib/RVL_SDK/bte/l2c_csm.c", source="RVL_SDK/src/bte/l2c_csm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_link.c", source="RVL_SDK/src/bte/l2c_link.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_main.c", source="RVL_SDK/src/bte/l2c_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_utils.c", source="RVL_SDK/src/bte/l2c_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_api.c", source="RVL_SDK/src/bte/port_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_rfc.c", source="RVL_SDK/src/bte/port_rfc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_utils.c", source="RVL_SDK/src/bte/port_utils.c"),
            Object(Matching, "lib/RVL_SDK/bte/ptim.c", source="RVL_SDK/src/bte/ptim.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_l2cap_if.c", source="RVL_SDK/src/bte/rfc_l2cap_if.c"),
            Object(Matching, "lib/RVL_SDK/bte/rfc_mx_fsm.c", source="RVL_SDK/src/bte/rfc_mx_fsm.c"),
            Object(Matching, "lib/RVL_SDK/bte/rfc_port_fsm.c", source="RVL_SDK/src/bte/rfc_port_fsm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_port_if.c", source="RVL_SDK/src/bte/rfc_port_if.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_ts_frames.c", source="RVL_SDK/src/bte/rfc_ts_frames.c"),
            Object(Matching, "lib/RVL_SDK/bte/rfc_utils.c", source="RVL_SDK/src/bte/rfc_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_api.c", source="RVL_SDK/src/bte/sdp_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_db.c", source="RVL_SDK/src/bte/sdp_db.c"),
            Object(Matching, "lib/RVL_SDK/bte/sdp_discovery.c", source="RVL_SDK/src/bte/sdp_discovery.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_main.c", source="RVL_SDK/src/bte/sdp_main.c"),
            Object(Matching, "lib/RVL_SDK/bte/sdp_server.c", source="RVL_SDK/src/bte/sdp_server.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_utils.c", source="RVL_SDK/src/bte/sdp_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/utl.c", source="RVL_SDK/src/bte/utl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/uusb_ppc.c", source="RVL_SDK/src/bte/uusb_ppc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/wbt_ext.c", source="RVL_SDK/src/bte/wbt_ext.c"),
            Object(NonMatching, "lib/RVL_SDK/cx/CXSecureUncompression.c", source="RVL_SDK/src/cx/CXSecureUncompression.c"),
            Object(Matching, "lib/RVL_SDK/cx/CXUncompression.c", source="RVL_SDK/src/cx/CXUncompression.c"),
            Object(NonMatching, "lib/RVL_SDK/db/db.c", source="RVL_SDK/src/db/db.c"),
            Object(NonMatching, "lib/RVL_SDK/dsp/dsp.c", source="RVL_SDK/src/dsp/dsp.c"),
            Object(Matching, "lib/RVL_SDK/dsp/dsp_debug.c", source="RVL_SDK/src/dsp/dsp_debug.c"),
            Object(NonMatching, "lib/RVL_SDK/dsp/dsp_task.c", source="RVL_SDK/src/dsp/dsp_task.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvd.c", source="RVL_SDK/src/dvd/dvd.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdFatal.c", source="RVL_SDK/src/dvd/dvdFatal.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvd_broadway.c", source="RVL_SDK/src/dvd/dvd_broadway.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvderror.c", source="RVL_SDK/src/dvd/dvderror.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdfs.c", source="RVL_SDK/src/dvd/dvdfs.c"),
            Object(Matching, "lib/RVL_SDK/dvd/dvdidutils.c", source="RVL_SDK/src/dvd/dvdidutils.c"),
            Object(Matching, "lib/RVL_SDK/dvd/dvdqueue.c", source="RVL_SDK/src/dvd/dvdqueue.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encconvert.c", source="RVL_SDK/src/enc/encconvert.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encjapanese.c", source="RVL_SDK/src/enc/encjapanese.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encunicode.c", source="RVL_SDK/src/enc/encunicode.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encutility.c", source="RVL_SDK/src/enc/encutility.c"),
            Object(NonMatching, "lib/RVL_SDK/esp/esp.c", source="RVL_SDK/src/esp/esp.c"),
            Object(Matching, "lib/RVL_SDK/euart/euart.c", source="RVL_SDK/src/euart/euart.c"),
            Object(NonMatching, "lib/RVL_SDK/exi/EXIBios.c", source="RVL_SDK/src/exi/EXIBios.c"),
            Object(NonMatching, "lib/RVL_SDK/exi/EXICommon.c", source="RVL_SDK/src/exi/EXICommon.c"),
            Object(Matching, "lib/RVL_SDK/exi/EXIUart.c", source="RVL_SDK/src/exi/EXIUart.c"),
            Object(NonMatching, "lib/RVL_SDK/fs/fs.c", source="RVL_SDK/src/fs/fs.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXAttr.c", source="RVL_SDK/src/gx/GXAttr.c"),
            Object(Matching, "lib/RVL_SDK/gx/GXBump.c", source="RVL_SDK/src/gx/GXBump.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXDisplayList.c", source="RVL_SDK/src/gx/GXDisplayList.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXFifo.c", source="RVL_SDK/src/gx/GXFifo.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXFrameBuf.c", source="RVL_SDK/src/gx/GXFrameBuf.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXGeometry.c", source="RVL_SDK/src/gx/GXGeometry.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXInit.c", source="RVL_SDK/src/gx/GXInit.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXLight.c", source="RVL_SDK/src/gx/GXLight.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXMisc.c", source="RVL_SDK/src/gx/GXMisc.c"),
            Object(Matching, "lib/RVL_SDK/gx/GXPerf.c", source="RVL_SDK/src/gx/GXPerf.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXPixel.c", source="RVL_SDK/src/gx/GXPixel.c"),
            Object(Matching, "lib/RVL_SDK/gx/GXTev.c", source="RVL_SDK/src/gx/GXTev.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXTexture.c", source="RVL_SDK/src/gx/GXTexture.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXTransform.c", source="RVL_SDK/src/gx/GXTransform.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_allocator.c", source="RVL_SDK/src/mem/mem_allocator.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_expHeap.c", source="RVL_SDK/src/mem/mem_expHeap.c"),
            Object(Matching, "lib/RVL_SDK/mem/mem_frameHeap.c", source="RVL_SDK/src/mem/mem_frameHeap.c"),
            Object(Matching, "lib/RVL_SDK/mem/mem_heapCommon.c", source="RVL_SDK/src/mem/mem_heapCommon.c"),
            Object(Matching, "lib/RVL_SDK/mem/mem_list.c", source="RVL_SDK/src/mem/mem_list.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/mtx.c", source="RVL_SDK/src/mtx/mtx.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/mtx44.c", source="RVL_SDK/src/mtx/mtx44.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/vec.c", source="RVL_SDK/src/mtx/vec.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDCheck.c", source="RVL_SDK/src/nand/NANDCheck.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDCore.c", source="RVL_SDK/src/nand/NANDCore.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDLogging.c", source="RVL_SDK/src/nand/NANDLogging.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDOpenClose.c", source="RVL_SDK/src/nand/NANDOpenClose.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDSecret.c", source="RVL_SDK/src/nand/NANDSecret.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/nand.c", source="RVL_SDK/src/nand/nand.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OS.c", source="RVL_SDK/src/os/OS.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSAlarm.c", source="RVL_SDK/src/os/OSAlarm.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSAlloc.c", source="RVL_SDK/src/os/OSAlloc.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSArena.c", source="RVL_SDK/src/os/OSArena.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSCache.c", source="RVL_SDK/src/os/OSCache.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSContext.c", source="RVL_SDK/src/os/OSContext.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSError.c", source="RVL_SDK/src/os/OSError.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSExec.c", source="RVL_SDK/src/os/OSExec.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSFont.c", source="RVL_SDK/src/os/OSFont.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSInterrupt.c", source="RVL_SDK/src/os/OSInterrupt.c"),
            Object(Matching, "lib/RVL_SDK/os/OSIpc.c", source="RVL_SDK/src/os/OSIpc.c"),
            Object(Matching, "lib/RVL_SDK/os/OSLink.c", source="RVL_SDK/src/os/OSLink.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMemory.c", source="RVL_SDK/src/os/OSMemory.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMessage.c", source="RVL_SDK/src/os/OSMessage.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMutex.c", source="RVL_SDK/src/os/OSMutex.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSNandbootInfo.c", source="RVL_SDK/src/os/OSNandbootInfo.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSNet.c", source="RVL_SDK/src/os/OSNet.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSPlayRecord.c", source="RVL_SDK/src/os/OSPlayRecord.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSPlayTime.c", source="RVL_SDK/src/os/OSPlayTime.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSReset.c", source="RVL_SDK/src/os/OSReset.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSRtc.c", source="RVL_SDK/src/os/OSRtc.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSSemaphore.c", source="RVL_SDK/src/os/OSSemaphore.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSStateFlags.c", source="RVL_SDK/src/os/OSStateFlags.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSStateTM.c", source="RVL_SDK/src/os/OSStateTM.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSSync.c", source="RVL_SDK/src/os/OSSync.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSThread.c", source="RVL_SDK/src/os/OSThread.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSTime.c", source="RVL_SDK/src/os/OSTime.c"),
            Object(Matching, "lib/RVL_SDK/os/OSUtf.c", source="RVL_SDK/src/os/OSUtf.c"),
            Object(NonMatching, "lib/RVL_SDK/pad/Pad.c", source="RVL_SDK/src/pad/Pad.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scapi.c", source="RVL_SDK/src/sc/scapi.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scapi_prdinfo.c", source="RVL_SDK/src/sc/scapi_prdinfo.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scsystem.c", source="RVL_SDK/src/sc/scsystem.c"),
            Object(NonMatching, "lib/RVL_SDK/si/SIBios.c", source="RVL_SDK/src/si/SIBios.c"),
            Object(NonMatching, "lib/RVL_SDK/si/SISamplingRate.c", source="RVL_SDK/src/si/SISamplingRate.c"),
            Object(NonMatching, "lib/RVL_SDK/tpl/TPL.c", source="RVL_SDK/src/tpl/TPL.c"),
            Object(NonMatching, "lib/RVL_SDK/usb/usb.c", source="RVL_SDK/src/usb/usb.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/i2c.c", source="RVL_SDK/src/vi/i2c.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/vi.c", source="RVL_SDK/src/vi/vi.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/vi3in1.c", source="RVL_SDK/src/vi/vi3in1.c"),
            Object(Matching, "lib/RVL_SDK/wenc/wenc.c", source="RVL_SDK/src/wenc/wenc.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPAD.c", source="RVL_SDK/src/wpad/WPAD.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPADEncrypt.c", source="RVL_SDK/src/wpad/WPADEncrypt.c"),
            Object(Matching, "lib/RVL_SDK/wpad/debug_msg.c", source="RVL_SDK/src/wpad/debug_msg.c"),
            Object(NonMatching, "lib/RVL_SDK/wud/WUD.c", source="RVL_SDK/src/wud/WUD.c"),
            Object(NonMatching, "lib/RVL_SDK/wud/WUDHidHost.c", source="RVL_SDK/src/wud/WUDHidHost.c"),
            Object(Matching, "lib/RVL_SDK/wud/debug_msg.c", source="RVL_SDK/src/wud/debug_msg.c"),
        ],
    },
    {
        "lib": "gamespy (mkw)",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_ext_mkw_gamespy,
        "progress_category": "sdk",
        "src_dir": "lib",
        "objects": [
            Object(NonMatching, "lib/gamespy/GP/gp.c", source="gamespy/GP/gp.c"),
            Object(NonMatching, "lib/gamespy/GP/gpi.c", source="gamespy/GP/gpi.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiBuddy.c", source="gamespy/GP/gpiBuddy.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiBuffer.c", source="gamespy/GP/gpiBuffer.c"),
            Object(Matching, "lib/gamespy/GP/gpiCallback.c", source="gamespy/GP/gpiCallback.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiConnect.c", source="gamespy/GP/gpiConnect.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiInfo.c", source="gamespy/GP/gpiInfo.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiKeys.c", source="gamespy/GP/gpiKeys.c"),
            Object(Matching, "lib/gamespy/GP/gpiOperation.c", source="gamespy/GP/gpiOperation.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiPeer.c", source="gamespy/GP/gpiPeer.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiProfile.c", source="gamespy/GP/gpiProfile.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiSearch.c", source="gamespy/GP/gpiSearch.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiTransfer.c", source="gamespy/GP/gpiTransfer.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiUnique.c", source="gamespy/GP/gpiUnique.c"),
            Object(NonMatching, "lib/gamespy/GP/gpiUtility.c", source="gamespy/GP/gpiUtility.c"),
            Object(NonMatching, "lib/gamespy/common/gsAvailable.c", source="gamespy/common/gsAvailable.c"),
            Object(NonMatching, "lib/gamespy/common/gsUdpEngine.c", source="gamespy/common/gsUdpEngine.c"),
            Object(NonMatching, "lib/gamespy/common/revolution/gsSocketRevolution.c", source="gamespy/common/revolution/gsSocketRevolution.c"),
            Object(Matching, "lib/gamespy/darray.c", source="gamespy/darray.c"),
            Object(NonMatching, "lib/gamespy/gstats/gbucket.c", source="gamespy/gstats/gbucket.c"),
            Object(NonMatching, "lib/gamespy/gstats/gstats.c", source="gamespy/gstats/gstats.c"),
            Object(Matching, "lib/gamespy/gt2/gt2Auth.c", source="gamespy/gt2/gt2Auth.c"),
            Object(Matching, "lib/gamespy/gt2/gt2Buffer.c", source="gamespy/gt2/gt2Buffer.c"),
            Object(Matching, "lib/gamespy/gt2/gt2Callback.c", source="gamespy/gt2/gt2Callback.c"),
            Object(Matching, "lib/gamespy/gt2/gt2Connection.c", source="gamespy/gt2/gt2Connection.c"),
            Object(NonMatching, "lib/gamespy/gt2/gt2Main.c", source="gamespy/gt2/gt2Main.c"),
            Object(Matching, "lib/gamespy/gt2/gt2Socket.c", source="gamespy/gt2/gt2Socket.c"),
            Object(NonMatching, "lib/gamespy/gt2/gt2Utility.c", source="gamespy/gt2/gt2Utility.c"),
            Object(Matching, "lib/gamespy/hashtable.c", source="gamespy/hashtable.c"),
            Object(Matching, "lib/gamespy/md5c.c", source="gamespy/md5c.c"),
            Object(NonMatching, "lib/gamespy/qr2/qr2.c", source="gamespy/qr2/qr2.c"),
            Object(NonMatching, "lib/gamespy/qr2/qr2regkeys.c", source="gamespy/qr2/qr2regkeys.c"),
            Object(NonMatching, "lib/gamespy/serverbrowsing/sb_crypt.c", source="gamespy/serverbrowsing/sb_crypt.c"),
            Object(NonMatching, "lib/gamespy/serverbrowsing/sb_queryengine.c", source="gamespy/serverbrowsing/sb_queryengine.c"),
            Object(NonMatching, "lib/gamespy/serverbrowsing/sb_server.c", source="gamespy/serverbrowsing/sb_server.c"),
            Object(NonMatching, "lib/gamespy/serverbrowsing/sb_serverbrowsing.c", source="gamespy/serverbrowsing/sb_serverbrowsing.c"),
            Object(NonMatching, "lib/gamespy/serverbrowsing/sb_serverlist.c", source="gamespy/serverbrowsing/sb_serverlist.c"),
        ],
    },
    {
        "lib": "keyboard (wii-ipl)",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_ext_ipl_keyboard,
        "progress_category": "sdk",
        "src_dir": "lib",
        "objects": [
            Object(NonMatching, "lib/keyboard/MyTiBg.cpp", source="keyboard/src/MyTiBg.cpp"),
            Object(NonMatching, "lib/keyboard/tiCandidateBox.cpp", source="keyboard/src/tiCandidateBox.cpp"),
            Object(NonMatching, "lib/keyboard/tiCpData.cpp", source="keyboard/src/tiCpData.cpp", shift_jis=False),
            Object(NonMatching, "lib/keyboard/tiDebug.cpp", source="keyboard/src/tiDebug.cpp"),
            Object(NonMatching, "lib/keyboard/tiGUIManager.cpp", source="keyboard/src/tiGUIManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiHwKeyboard.cpp", source="keyboard/src/tiHwKeyboard.cpp"),
            Object(NonMatching, "lib/keyboard/tiLanguageIndependentData.cpp", source="keyboard/src/tiLanguageIndependentData.cpp", shift_jis=False),
            Object(NonMatching, "lib/keyboard/tiLayout.cpp", source="keyboard/src/tiLayout.cpp"),
            Object(Matching, "lib/keyboard/tiLayoutGather.cpp", source="keyboard/src/tiLayoutGather.cpp"),
            Object(NonMatching, "lib/keyboard/tiNw4rManager.cpp", source="keyboard/src/tiNw4rManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiPkData.cpp", source="keyboard/src/tiPkData.cpp", shift_jis=False),
            Object(NonMatching, "lib/keyboard/tiPredictLang.cpp", source="keyboard/src/tiPredictLang.cpp"),
            Object(NonMatching, "lib/keyboard/tiSwData.cpp", source="keyboard/src/tiSwData.cpp", shift_jis=False),
            Object(NonMatching, "lib/keyboard/tiTextInputBase.cpp", source="keyboard/src/tiTextInputBase.cpp"),
            Object(NonMatching, "lib/keyboard/tiToolBar.cpp", source="keyboard/src/tiToolBar.cpp"),
            Object(NonMatching, "lib/keyboard/tiUtil.cpp", source="keyboard/src/tiUtil.cpp"),
        ],
    },
    # --- imported libs (end) ---
    {
        "lib": "EGG",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_identified_game,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "lib/egg/core/eggArchive.cpp"),
            Object(NonMatching, "lib/egg/core/eggDvdFile.cpp"),
            Object(NonMatching, "lib/egg/core/eggDvdRipper.cpp"),
            Object(NonMatching, "lib/egg/core/eggDecomp.cpp"),
            Object(NonMatching, "lib/egg/core/eggAllocator.cpp"),
            Object(NonMatching, "lib/egg/core/eggHeap.cpp"),
            Object(NonMatching, "lib/egg/core/eggExpHeap.cpp"),
            Object(NonMatching, "lib/egg/core/eggFrmHeap.cpp"),
            Object(NonMatching, "lib/egg/core/eggAssertHeap.cpp"),
            Object(Matching, "lib/egg/core/eggDisposer.cpp"),
            Object(NonMatching, "lib/egg/core/eggColorFader.cpp"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(NonMatching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp", source="runtime/__init_cpp_exceptions.cpp"),
            Object(NonMatching, "runtime/class_arrays.cpp", extra_cflags=["-Cpp_exceptions on"]),
        ],
    },
    Rel(
        "d_fish_fieldNP",
        [
            Object(Matching, "d_fish_fieldNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            # The module runtime's destructor chain: its functions are stripped, but the 4-byte
            # __global_destructor_chain (force_active in config.yml) is the REL's whole .bss.
            Object(Matching, "d_fish_fieldNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_fish_fieldNP/d_fish_field.cpp", extra_cflags=["-sym on"]),
        ],
    ),
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
