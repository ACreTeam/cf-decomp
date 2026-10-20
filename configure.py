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

# Matching builds keep objects that exist only to reproduce the original layout
if not config.non_matching:
    cflags_base.append("-DMUST_MATCH")
    cflags_clib.append("-DMUST_MATCH")

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
    "-cwd source",  # quoted #includes (the src .inc files) are found next to the including source
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

# Game (DOL) flags: every game TU is built with -sym on (weak/inline functions from other files go to
# per-file .text sections after the TU's own code; see notes/mwcc_ghidra.md).
cflags_game = [
    *cflags_identified_game,
    "-sym on",
]

# REL flags: the game's flags (with -sym on, like cflags_game), without small data (RELs have no
# .sdata / .sbss).
cflags_rel = [
    *cflags_game,
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
# NW4R (src/lib/nw4r, headers in include/lib/nw4r): the NW4R library flags (base flags plus the NW4R extras;
# the pedantic warnings are left out). VERSION_RSPE01_01 selects the later library code paths, and
# NW4R_LIB_BUILD gives the library its inline placement new.
cflags_nw4r_base = [
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
    "-i include",
    "-i include/lib",
    "-i include/lib/MSL",
    "-i include/lib/MSL/internal",
    "-DBUILD_VERSION=1",
    "-DVERSION_RSPE01_01",
    "-ir include/lib/revolution/BTE",
    "-DREVOLUTION",
    "-DNW4R_LIB_BUILD",
    "-DNDEBUG=1",
    "-enc SJIS",
    "-i include/lib/nw4r",
]
cflags_nw4r = [
    *cflags_nw4r_base,
    "-fp_contract off",
    "-ipa file",
]
cflags_nw4r_scnrfl = [
    *cflags_nw4r_base,
    "-fp_contract on",
    "-use_lmw_stmw on",
    "-inline deferred",
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
            Object(Matching, "dol/cLib/c_dylink.cpp", extra_cflags=["-RTTI on", "-sym on"]),
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
            Object(Matching, "dol/mLib/m_dvd.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_heap.cpp"),
            Object(Matching, "dol/mLib/m_mtx.cpp"),
            Object(Matching, "dol/mLib/m_allocator.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_3d.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_3d/face_ex.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_angle.cpp"),
            Object(Matching, "dol/mLib/m_face.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_vec.cpp"),
            Object(Matching, "dol/mLib/m_quat.cpp"),
            Object(Matching, "dol/mLib/m_2d.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_pad.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_tex.cpp"),
            Object(Matching, "dol/mLib/m_fader.cpp"),
            Object(Matching, "dol/mLib/m_fader_base.cpp"),
            Object(Matching, "dol/mLib/m_wipe_fader.cpp"),
            Object(Matching, "dol/mLib/m_color_fader.cpp"),
            Object(Matching, "dol/mLib/m_frustum.cpp"),
            Object(Matching, "dol/mLib/m_3d/m_3d_capture.cpp"),
            Object(Matching, "dol/mLib/m_ef.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/mLib/m_color.cpp"),
            Object(Matching, "dol/mLib/m_perf.cpp"),
        ],
    },
    {
        "lib": "game",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/game/d_m2d.cpp", shift_jis=False),
            Object(Matching, "dol/game/d_m3d.cpp"),
            Object(Matching, "dol/game/d_actor.cpp"),
            Object(Matching, "dol/game/d_analog_select.cpp"),
            Object(Matching, "dol/game/d_snd_util.cpp"),
            Object(Matching, "dol/game/d_audio_obj.cpp"),
            Object(Matching, "dol/game/d_a_npc.cpp", shift_jis=False),
            Object(Matching, "dol/game/d_a_npc_nml.cpp"),
            Object(Matching, "dol/game/d_npc_talk_approach.cpp"),
            Object(Matching, "dol/game/d_npc_talk_arbeit.cpp"),
            Object(Matching, "dol/game/d_npc_talk_free.cpp"),
            Object(Matching, "dol/game/d_npc_talk_carnival.cpp"),
            Object(Matching, "dol/game/d_npc_talk_countdown.cpp"),
            Object(Matching, "dol/game/d_npc_talk_halloween.cpp"),
            Object(Matching, "dol/game/d_npc_talk_harvest.cpp"),
            Object(Matching, "dol/game/d_npc_talk_bug.cpp"),
            Object(Matching, "dol/game/d_npc_talk_fireworks.cpp"),
            Object(Matching, "dol/game/d_npc_talk_fishing.cpp"),
            Object(Matching, "dol/game/d_npc_talk_fmarket.cpp"),
            Object(Matching, "dol/game/d_npc_talk_birthday.cpp"),
            Object(Matching, "dol/game/d_npc_talk_mood.cpp"),
            Object(Matching, "dol/game/d_a_npc_sp.cpp"),
            Object(Matching, "dol/game/d_a_npc_town.cpp"),
            Object(Matching, "dol/game/d_a_rmobj.cpp"),
            Object(Matching, "dol/game/d_a_rmobj_model.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q04.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q02.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q03.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q05.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q01.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q12.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q11.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_delivery.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q07.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q06.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q10.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q13.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q09.cpp"),
            Object(Matching, "dol/game/d_npc_talk_quest_q08.cpp"),
            Object(Matching, "dol/game/d_npc_talk_reaction.cpp"),
            Object(Matching, "dol/game/d_npc_talk_rollan.cpp"),
            Object(Matching, "dol/game/d_npc_talk_town.cpp"),
            Object(Matching, "dol/game/d_banner.cpp"),
            Object(Matching, "dol/game/d_balloon.cpp"),
            Object(Matching, "dol/game/d_balloon_string.cpp"),
            Object(Matching, "dol/game/d_base.cpp"),
            Object(Matching, "dol/game/d_bg.cpp"),
            Object(Matching, "dol/game/d_bgc.cpp"),
            Object(Matching, "dol/game/d_bgcf.cpp"),
            Object(Matching, "dol/game/d_bg_util.cpp"),
            Object(Matching, "dol/game/d_bgm.cpp"),
            Object(Matching, "dol/game/d_ds_player_data.cpp"),
            Object(Matching, "dol/game/d_travel_data.cpp"),
            Object(Matching, "dol/game/d_dvd.cpp"),
            Object(Matching, "dol/game/d_mother_mail.cpp"),
            Object(Matching, "dol/game/d_nand.cpp"),
            Object(Matching, "dol/game/d_caution.cpp"),
            Object(Matching, "dol/game/d_drum.cpp"),
            Object(Matching, "dol/game/d_field_block.cpp"),
            Object(Matching, "dol/game/d_fd_unit_flags.cpp"),
            Object(Matching, "dol/game/d_demo_actor.cpp"),
            Object(Matching, "dol/game/d_light.cpp"),
            Object(Matching, "dol/game/d_demo_actor_list.cpp"),
            Object(Matching, "dol/game/d_mii_icon.cpp"),
            Object(Matching, "dol/game/d_fader.cpp"),
            Object(Matching, "dol/game/d_field_info.cpp"),
            Object(Matching, "dol/game/d_effect.cpp"),
            Object(Matching, "dol/game/d_effect_util.cpp"),
            Object(Matching, "dol/game/d_effect_manager.cpp"),
            Object(Matching, "dol/game/d_event.cpp"),
            Object(Matching, "dol/game/d_screenshot.cpp"),
            Object(Matching, "dol/game/d_fireworks.cpp"),
            Object(Matching, "dol/game/d_star_mgr.cpp"),
            Object(Matching, "dol/game/d_star.cpp"),
            Object(Matching, "dol/game/d_fish_info.cpp"),
            Object(Matching, "dol/game/d_fg_data.cpp"),
            Object(Matching, "dol/game/d_field_assessment.cpp"),
            Object(Matching, "dol/game/d_msg.cpp"),
            Object(Matching, "dol/game/d_npc_mng.cpp"),
            Object(Matching, "dol/game/d_objc.cpp"),
            Object(Matching, "dol/game/d_player_mngr.cpp"),
            Object(Matching, "dol/game/d_root.cpp"),
            Object(Matching, "dol/game/d_fg_item.cpp"),
            Object(Matching, "dol/game/d_footmark.cpp"),
            Object(Matching, "dol/game/d_friend.cpp"),
            Object(Matching, "dol/game/d_hmn_anm.cpp"),
            Object(Matching, "dol/game/d_hmn_body_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_cloth_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_face_anm_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_face_tex_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_head_item_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_head_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_name.cpp"),
            Object(Matching, "dol/game/d_hmn_palette_mng.cpp"),
            Object(Matching, "dol/game/d_hmn_tool_mng.cpp"),
            Object(Matching, "dol/game/d_heap.cpp"),
            Object(Matching, "dol/game/d_hr.cpp"),
            Object(Matching, "dol/game/d_insect_info.cpp"),
            Object(Matching, "dol/game/d_insect_util.cpp"),
            Object(Matching, "dol/game/d_item.cpp", shift_jis=False),
            Object(Matching, "dol/game/d_net.cpp"),
            Object(Matching, "dol/game/d_net_ds_download.cpp"),
            Object(Matching, "dol/game/d_net_mgr.cpp"),
            Object(Matching, "dol/game/d_net_friend.cpp"),
            Object(Matching, "dol/game/d_net_dwc.cpp"),
            Object(Matching, "dol/game/d_net_wc24.cpp"),
            Object(Matching, "dol/game/d_net_wus.cpp"),
            Object(Matching, "dol/game/d_nickname.cpp"),
            Object(Matching, "dol/game/d_npc_info.cpp"),
            Object(Matching, "dol/game/d_sv_notice.cpp"),
            Object(Matching, "dol/game/d_notice.cpp"),
            Object(Matching, "dol/game/d_npc.cpp"),
            Object(Matching, "dol/game/d_player_mgr.cpp"),
            Object(Matching, "dol/game/d_pad.cpp"),
            Object(Matching, "dol/game/d_rumble.cpp"),
            Object(Matching, "dol/game/d_paper.cpp"),
            Object(Matching, "dol/game/d_photo.cpp"),
            Object(Matching, "dol/game/d_post_office.cpp"),
            Object(Matching, "dol/game/d_prc_mng.cpp"),
            Object(Matching, "dol/game/d_random_field.cpp"),
            Object(Matching, "dol/game/d_rec_bank.cpp"),
            Object(Matching, "dol/game/d_region.cpp"),
            Object(Matching, "dol/game/d_reset.cpp"),
            Object(Matching, "dol/game/d_random.cpp"),
            Object(Matching, "dol/game/d_s_boot_static.cpp"),
            Object(Matching, "dol/game/d_sv_auc.cpp"),
            Object(Matching, "dol/game/d_save_option.cpp"),
            Object(Matching, "dol/game/d_save_data.cpp"),
            Object(Matching, "dol/game/d_dsn.cpp"),
            Object(Matching, "dol/game/d_save_visitor_npc.cpp"),
            Object(Matching, "dol/game/d_save_mother_mail.cpp"),
            Object(Matching, "dol/game/d_save_main_field.cpp"),
            Object(Matching, "dol/game/d_home_room_map.cpp"),
            Object(Matching, "dol/game/d_model_room.cpp"),
            Object(Matching, "dol/game/d_save_box.cpp"),
            Object(Matching, "dol/game/d_bug_off.cpp"),
            Object(Matching, "dol/game/d_ev_fishing.cpp"),
            Object(Matching, "dol/game/d_save_check.cpp"),
            Object(Matching, "dol/game/d_sv_extra_header.cpp"),
            Object(Matching, "dol/game/d_sv_extra.cpp"),
            Object(Matching, "dol/game/d_save_dl_item.cpp"),
            Object(Matching, "dol/game/d_save_town.cpp"),
            Object(Matching, "dol/game/d_land.cpp"),
            Object(Matching, "dol/game/d_mail.cpp"),
            Object(Matching, "dol/game/d_save_melody.cpp"),
            Object(Matching, "dol/game/d_save_money.cpp"),
            Object(Matching, "dol/game/d_museum.cpp"),
            Object(Matching, "dol/game/d_animal.cpp"),
            Object(Matching, "dol/game/d_animal_id.cpp"),
            Object(Matching, "dol/game/d_private_data.cpp"),
            Object(Matching, "dol/game/d_home.cpp"),
            Object(Matching, "dol/game/d_personal_id.cpp"),
            Object(Matching, "dol/game/d_post_box.cpp"),
            Object(Matching, "dol/game/d_quest.cpp"),
            Object(Matching, "dol/game/d_save_shops.cpp"),
            Object(Matching, "dol/game/d_save_shop.cpp"),
            Object(Matching, "dol/game/d_save_shop_stalk_market.cpp"),
            Object(Matching, "dol/game/d_save_catherine.cpp"),
            Object(Matching, "dol/game/d_save_shop_tailor.cpp"),
            Object(Matching, "dol/game/d_save_shop_gallery.cpp"),
            Object(Matching, "dol/game/d_save_shop_grace.cpp"),
            Object(Matching, "dol/game/d_save_cafe_guest.cpp"),
            Object(Matching, "dol/game/d_save_building.cpp"),
            Object(Matching, "dol/game/d_theater.cpp"),
            Object(Matching, "dol/game/d_time_stamp.cpp"),
            Object(Matching, "dol/game/d_ymd.cpp"),
            Object(Matching, "dol/game/d_sv_time_offset.cpp"),
            Object(Matching, "dol/game/d_sv_town_info.cpp"),
            Object(Matching, "dol/game/d_weather_rain.cpp"),
            Object(Matching, "dol/game/d_weather_snow.cpp"),
            Object(Matching, "dol/game/d_weather_sakura.cpp"),
            Object(Matching, "dol/game/d_weather_confetti.cpp"),
            Object(Matching, "dol/game/d_shadow.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_weather.cpp"),
            Object(Matching, "dol/game/d_s_boot.cpp"),
            Object(Matching, "dol/game/d_theater_common.cpp"),
            Object(Matching, "dol/game/d_scene_check.cpp"),
            Object(Matching, "dol/game/d_fg_obj.cpp"),
            Object(Matching, "dol/game/d_home_button.cpp"),
            Object(Matching, "dol/game/d_a_c_uitag.cpp"),
            Object(Matching, "dol/game/d_a_snowman.cpp"),
            Object(Matching, "dol/game/d_bg_debug.cpp"),
            Object(Matching, "dol/game/d_sv_week.cpp"),
            Object(Matching, "dol/game/d_sv_activity.cpp"),
            Object(Matching, "dol/game/d_sv_date_event.cpp"),
            Object(Matching, "dol/game/d_sv_lang_flag.cpp"),
            Object(Matching, "dol/game/d_ground_attr.cpp"),
            Object(Matching, "dol/game/d_reset_save.cpp"),
            Object(Matching, "dol/game/d_camera.cpp"),
            Object(Matching, "dol/game/d_clock.cpp"),
            Object(Matching, "dol/game/d_cd.cpp"),
            Object(Matching, "dol/game/d_demo.cpp"),
            Object(Matching, "dol/game/d_fd_block_type.cpp"),
            Object(Matching, "dol/game/d_npc_mdl_mng.cpp"),
            Object(Matching, "dol/game/d_thunder.cpp"),
            Object(Matching, "dol/game/d_fltg.cpp"),
            Object(Matching, "dol/game/d_font.cpp"),
            Object(Matching, "dol/game/d_menu_base.cpp"),
            Object(Matching, "dol/game/d_menu_btn.cpp"),
            Object(Matching, "dol/game/d_hand_item.cpp"),
            Object(Matching, "dol/game/d_environment.cpp"),
            Object(Matching, "dol/game/d_menu_mng.cpp"),
            Object(Matching, "dol/game/d_menu_slct_b.cpp"),
            Object(Matching, "dol/game/d_menu_slct_c.cpp"),
            Object(Matching, "dol/game/d_menu_res.cpp"),
            Object(Matching, "dol/game/d_dylink.cpp"),
            Object(Matching, "dol/game/d_item_debug.cpp"),
            Object(Matching, "dol/game/d_item_sel.cpp"),
            Object(Matching, "dol/game/d_activity_log.cpp"),
            Object(Matching, "dol/game/d_sv_proc.cpp"),
            Object(Matching, "dol/game/d_scene_base.cpp"),
            Object(Matching, "dol/game/d_obj_bal.cpp"),
            Object(Matching, "dol/game/d_obj_chat.cpp"),
            Object(Matching, "dol/game/d_obj_cursor.cpp"),
            Object(Matching, "dol/game/d_obj_keyboard.cpp"),
            Object(Matching, "dol/game/d_obj_staffroll.cpp"),
            Object(Matching, "dol/game/d_obj_telop.cpp"),
            Object(Matching, "dol/game/d_obj_tutorial.cpp"),
            Object(Matching, "dol/game/d_sys.cpp", extra_cflags=["-sym on"]),
            Object(Matching, "dol/game/d_tag_processor.cpp"),
            Object(Matching, "dol/game/d_pattern_copy.cpp"),
            Object(Matching, "dol/game/d_ds_dl.cpp"),
            Object(Matching, "dol/game/d_sv_star_sign.cpp"),
            Object(Matching, "dol/game/d_sv_auc_list.cpp"),
            Object(Matching, "dol/game/d_event_mng.cpp"),
            Object(Matching, "dol/game/d_fi_mdl_mgr.cpp"),
            Object(Matching, "dol/game/d_field.cpp"),
            Object(Matching, "dol/game/d_money.cpp"),
            Object(Matching, "dol/game/d_obj_voice.cpp"),
            Object(Matching, "dol/game/d_kbd.cpp"),
            Object(Matching, "dol/game/d_letter.cpp"),
            Object(Matching, "dol/game/d_line_draw.cpp"),
            Object(Matching, "dol/game/d_main.cpp"),
            Object(Matching, "dol/game/d_map.cpp"),
            Object(Matching, "dol/game/d_demo_sync.cpp"),
            Object(Matching, "dol/game/d_sv_villager_move.cpp"),
            Object(Matching, "dol/game/d_npc_list.cpp"),
            Object(Matching, "dol/game/d_black_wait.cpp"),
            Object(Matching, "dol/game/d_sv_town_list.cpp"),
            Object(Matching, "dol/game/d_sv_npc_pos.cpp"),
            Object(Matching, "dol/game/d_ftr.cpp"),
            Object(Matching, "dol/game/d_net_player_info.cpp"),
            Object(Matching, "dol/game/d_sys_gfx.cpp"),
            Object(Matching, "dol/game/d_npc_setup.cpp"),
            Object(Matching, "dol/game/d_net_req_pool.cpp"),
            Object(Matching, "dol/game/d_sv_daily.cpp"),
            Object(Matching, "dol/game/d_sv_player_flag.cpp"),
            Object(Matching, "dol/game/d_sv_daily_rand.cpp"),
            Object(Matching, "dol/game/d_sv_wc24.cpp"),
            Object(Matching, "dol/game/d_wifi.cpp"),
            Object(Matching, "dol/game/d_wifi_err.cpp"),
            Object(Matching, "dol/game/d_wipe_fader.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_common.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_dm_bus_pl_crt.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_dm_pl_sel.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_dm_save.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_dm_title.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_field.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_auction.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_barber.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_broker.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_chkp.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_fortune.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_grace.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_hs0_bm.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_hs0_f1.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_hs0_f2.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_hs0_loft.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_happy_ent.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_happy_mdl.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_astro.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_cafe.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_ent.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_fish.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_fossil0.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_fossil1.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_ins.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_pic0.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_mm_pic1.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_npc0.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_office.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_reset.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_sample.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_shop0.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_shop1.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_shop2.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_shop3_f1.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_shop3_f2.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_tailor.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_theater_ent.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_rm_theater_show.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_town.cpp"),
            Object(Matching, "dol/game/sc_data/d_sc_data_check_field.cpp"),
            Object(Matching, "dol/game/d_search_cand.cpp"),
            Object(Matching, "dol/game/d_shop_layout.cpp"),
            Object(Matching, "dol/game/d_shutdown_fader.cpp"),
            Object(Matching, "dol/game/d_shutter_fader.cpp"),
            Object(Matching, "dol/game/d_side_wipe_fader.cpp"),
            Object(Matching, "dol/game/d_play_util.cpp"),
            Object(Matching, "dol/game/d_scene.cpp"),
            Object(Matching, "dol/game/d_police_box.cpp"),
            Object(Matching, "dol/game/d_recycle_bin.cpp"),
            Object(Matching, "dol/game/d_script.cpp"),
            Object(Matching, "dol/game/d_script_bank.cpp"),
            Object(Matching, "dol/game/d_select.cpp"),
            Object(Matching, "dol/game/d_star_draw.cpp"),
            Object(Matching, "dol/game/d_str.cpp"),
            Object(Matching, "dol/game/d_string.cpp"),
            Object(Matching, "dol/game/d_date.cpp"),
            Object(Matching, "dol/game/d_uki.cpp"),
            Object(Matching, "dol/game/d_voice_chat.cpp"),
            # Stored as CP932; pass directly to MWCC without UTF-8 conversion.
            Object(Matching, "dol/game/d_sv_demo.cpp", shift_jis=False),
            Object(Matching, "dol/game/d_sv_demo_scene.cpp"),
            Object(Matching, "dol/game/d_sv_mgr.cpp", shift_jis=False),
        ],
    },
    {
        "lib": "sound",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_game,
        "progress_category": "game",
        "objects": [
            Object(Matching, "dol/sound/SoundEffectReverb.cpp"),
            Object(Matching, "dol/sound/SoundFader.cpp"),
            Object(Matching, "dol/sound/SoundTv.cpp"),
            Object(Matching, "dol/sound/SoundConductor.cpp"),
            Object(Matching, "dol/sound/SoundHaniwa.cpp"),
            Object(Matching, "dol/sound/SoundObjFurniture.cpp"),
            Object(Matching, "dol/sound/SoundSystem.cpp"),
            Object(Matching, "dol/sound/SoundManager.cpp"),
            Object(Matching, "dol/sound/SoundBgm.cpp"),
            Object(Matching, "dol/sound/SoundVoice.cpp"),
            Object(Matching, "dol/sound/SoundAmbient.cpp"),
            Object(Matching, "dol/sound/SoundObj.cpp"),
            Object(Matching, "dol/sound/SoundAudioFrameCallback.cpp"),
            Object(Matching, "dol/sound/SoundLoader.cpp"),
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
    {
        "lib": "keyboard",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_ext_ipl_keyboard,
        "progress_category": "sdk",
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
        "lib": "NW4R",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_nw4r,
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "lib/nw4r/ut/ut_list.cpp"),
            Object(Matching, "lib/nw4r/ut/ut_LinkList.cpp"),
            Object(Matching, "lib/nw4r/ut/ut_binaryFileFormat.cpp"),
            Object(Matching, "lib/nw4r/ut/ut_CharStrmReader.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_TagProcessorBase.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_IOStream.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_FileStream.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_DvdFileStream.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_DvdLockedFileStream.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_LockedCache.cpp"),
            Object(Matching, "lib/nw4r/ut/ut_Font.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_RomFont.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_ResFontBase.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_ResFont.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_CharWriter.cpp"),
            Object(NonMatching, "lib/nw4r/ut/ut_TextWriterBase.cpp"),
            Object(NonMatching, "lib/nw4r/math/math_arithmetic.cpp"),
            Object(NonMatching, "lib/nw4r/math/math_triangular.cpp"),
            Object(NonMatching, "lib/nw4r/math/math_types.cpp"),
            Object(Matching, "lib/nw4r/math/math_geometry.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_rescommon.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resdict.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resfile.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resmdl.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resshp.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_restev.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resmat.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resvtx.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_restex.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resnode.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resanm.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resanmvis.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resanmclr.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resanmtexpat.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resanmtexsrt.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/res/g3d_resanmchr.cpp"),
            Object(Matching, "lib/nw4r/g3d/res/g3d_resanmshp.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmvis.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmclr.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmtexpat.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmtexsrt.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmchr.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_anmshp.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_anmscn.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_obj.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_anmobj.cpp"),
            Object(Matching, "lib/nw4r/g3d/platform/g3d_gpu.cpp"),
            Object(Matching, "lib/nw4r/g3d/platform/g3d_tmem.cpp"),
            Object(Matching, "lib/nw4r/g3d/platform/g3d_cpu.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_state.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_draw1mat1shp.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_calcview.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_dcc.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_workmem.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_calcworld.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_draw.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_camera.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_scnobj.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_scnroot.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_scnmdlsmpl.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_scnmdl.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_calcmaterial.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_init.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_scnproc.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_fog.cpp"),
            Object(NonMatching, "lib/nw4r/g3d/g3d_light.cpp"),
            Object(Matching, "lib/nw4r/g3d/g3d_calcvtx.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_AxManager.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_AxVoice.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_AxVoiceManager.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_AxfxImpl.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Bank.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_BankFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_BasicPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_BasicSound.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Channel.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_DisposeCallbackManager.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_DvdSoundArchive.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_EnvGenerator.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_ExternalSoundPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_FrameHeap.cpp"),
            Object(Matching, "lib/nw4r/snd/snd_InstancePool.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Lfo.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_MidiSeqPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_MmlParser.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_MmlSeqTrack.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_MmlSeqTrackAllocator.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_RemoteSpeaker.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_RemoteSpeakerManager.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SeqFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SeqPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SeqSound.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SeqSoundHandle.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SeqTrack.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundArchive.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundArchiveFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundArchiveLoader.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundArchivePlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundHandle.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundHeap.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundStartable.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundSystem.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_SoundThread.cpp"),
            Object(Matching, "lib/nw4r/snd/snd_StrmChannel.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_StrmFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_StrmPlayer.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_StrmSound.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_StrmSoundHandle.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Task.cpp"),
            Object(Matching, "lib/nw4r/snd/snd_TaskManager.cpp"),
            Object(Matching, "lib/nw4r/snd/snd_TaskThread.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Voice.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_VoiceManager.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_Util.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_WaveFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_WaveSound.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_WaveSoundHandle.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_WsdFile.cpp"),
            Object(NonMatching, "lib/nw4r/snd/snd_WsdPlayer.cpp"),
            Object(Matching, "lib/nw4r/snd/snd_adpcm.cpp"),
            Object(Matching, "lib/nw4r/ef/ef_draworder.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_effect.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_effectsystem.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_emitter.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_particle.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_particlemanager.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_resource.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_util.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_handle.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_emitterform.cpp"),
            Object(NonMatching, "lib/nw4r/ef/ef_creationqueue.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_emform.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_point.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_line.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_disc.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_sphere.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_cylinder.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_torus.cpp"),
            Object(NonMatching, "lib/nw4r/ef/emform/ef_cube.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawstrategybuilder.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawstrategyimpl.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawbillboardstrategy.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawfreestrategy.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawlinestrategy.cpp"),
            Object(NonMatching, "lib/nw4r/ef/drawstrategy/ef_drawpointstrategy.cpp"),
            Object(Matching, "lib/nw4r/ef/ef_res_emitter_ac.cpp"),
            Object(Matching, "lib/nw4r/ef/ef_res_animcurve_ac.cpp"),
            Object(Matching, "lib/nw4r/lyt/lyt_init.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_pane.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_group.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_layout.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_picture.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_textBox.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_window.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_bounding.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_material.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_texMap.cpp"),
            Object(Matching, "lib/nw4r/lyt/lyt_drawInfo.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_animation.cpp"),
            Object(Matching, "lib/nw4r/lyt/lyt_resourceAccessor.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_arcResourceAccessor.cpp"),
            Object(NonMatching, "lib/nw4r/lyt/lyt_common.cpp"),
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
            Object(NonMatching, "lib/egg/gfx/eggDrawHelper.cpp"),
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
            Object(Matching, "d_fish_fieldNP/d_fish_field.cpp"),
        ],
    ),
    Rel(
        "d_skyNP",
        [
            Object(Matching, "d_skyNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_skyNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_skyNP/d_sky.cpp"),
        ],
    ),
    Rel(
        "d_sky_auroraNP",
        [
            Object(Matching, "d_sky_auroraNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_sky_auroraNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_sky_auroraNP/d_sky_aurora.cpp"),
        ],
    ),
    Rel(
        "d_sky_rainbowNP",
        [
            Object(Matching, "d_sky_rainbowNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_sky_rainbowNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_sky_rainbowNP/d_sky_rainbow.cpp"),
        ],
    ),
    Rel(
        "d_fg_drawNP",
        [
            Object(Matching, "d_fg_drawNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_fg_drawNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_fg_drawNP/d_fg_draw.cpp"),
        ],
    ),
    Rel(
        "d_menu_debug00NP",
        [
            Object(Matching, "d_menu_debug00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_debug00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_debug00NP/d_menu_debug00.cpp"),
        ],
    ),
    # Remaining REL scaffolds use extracted objects until their sources match.
    # Runtime TUs are separate; game-code groups in splits.txt are provisional
    # and may contain multiple original TUs. Do not treat them as confirmed boundaries.
    Rel(
        "d_profileNP",
        [
            Object(Matching, "d_profileNP/rel_init.cpp"),
            Object(Matching, "d_profileNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_profileNP/d_profile.cpp"),
        ],
    ),
    Rel(
        "d_menu_desiNP",
        [
            Object(Matching, "d_menu_desiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_desiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_desiNP/d_menu_desi.cpp"),
        ],
    ),
    Rel(
        "d_menu_itemNP",
        [
            Object(Matching, "d_menu_itemNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_itemNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_itemNP/d_menu_item.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_menu_itemxxNP",
        [
            Object(Matching, "d_menu_itemxxNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_itemxxNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_itemxxNP/d_menu_itemxx.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_menu_ltrNP",
        [
            Object(Matching, "d_menu_ltrNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_ltrNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_ltrNP/d_menu_ltr.cpp"),
        ],
    ),
    Rel(
        "d_menu_mapNP",
        [
            Object(Matching, "d_menu_mapNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_mapNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_mapNP/d_menu_map.cpp"),
        ],
    ),
    Rel(
        "d_menu_numberNP",
        [
            Object(Matching, "d_menu_numberNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_numberNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_numberNP/d_menu_number.cpp"),
        ],
    ),
    Rel(
        "d_menu_lcrsrNP",
        [
            Object(Matching, "d_menu_lcrsrNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_lcrsrNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_lcrsrNP/d_menu_lcrsr.cpp"),
        ],
    ),
    Rel(
        "d_menu_starNP",
        [
            Object(Matching, "d_menu_starNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_starNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_starNP/d_menu_star.cpp"),
        ],
    ),
    Rel(
        "d_menu_titleNP",
        [
            Object(Matching, "d_menu_titleNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_titleNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_titleNP/d_menu_title.cpp"),
        ],
    ),
    Rel(
        "d_paperNP",
        [
            Object(Matching, "d_paperNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_paperNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_paperNP/d_paper.cpp"),
        ],
    ),
    Rel(
        "d_a_balloonNP",
        [
            Object(Matching, "d_a_balloonNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_balloonNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_balloonNP/d_a_balloon.cpp", shift_jis=False),
        ],
    ),
    Rel(
        "d_a_bbs_birdNP",
        [
            Object(Matching, "d_a_bbs_birdNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_bbs_birdNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_bbs_birdNP/d_a_bbs_bird.cpp"),
        ],
    ),
    Rel(
        "d_a_cockroachNP",
        [
            Object(Matching, "d_a_cockroachNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_cockroachNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_cockroachNP/d_a_cockroach.cpp"),
        ],
    ),
    Rel(
        "d_a_ftrNP",
        [
            Object(Matching, "d_a_ftrNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_ftrNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_ftrNP/d_a_ftr.cpp"),
        ],
    ),
    Rel(
        "d_a_ftr_actNP",
        [
            Object(Matching, "d_a_ftr_actNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_ftr_actNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_ftr_actNP/d_a_ftr_act.cpp"),
        ],
    ),
    Rel(
        "d_a_growup_msgNP",
        [
            Object(Matching, "d_a_growup_msgNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_growup_msgNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_growup_msgNP/d_a_growup_msg.cpp"),
        ],
    ),
    Rel(
        "d_a_nameplateNP",
        [
            Object(Matching, "d_a_nameplateNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_nameplateNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_nameplateNP/d_a_nameplate.cpp"),
        ],
    ),
    Rel(
        "d_a_nameplate_museumNP",
        [
            Object(Matching, "d_a_nameplate_museumNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_nameplate_museumNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_nameplate_museumNP/d_a_nameplate_museum.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_cafeNP",
        [
            Object(Matching, "d_a_npc_cafeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_cafeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_cafeNP/d_a_npc_cafe.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_inNP",
        [
            Object(Matching, "d_a_npc_inNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_inNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_inNP/d_a_npc_in.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_a_npc_outNP",
        [
            Object(Matching, "d_a_npc_outNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_outNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_outNP/d_a_npc_out.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_a_npc_sp_sampleNP",
        [
            Object(Matching, "d_a_npc_sp_sampleNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_sampleNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_sampleNP/d_a_npc_sp_sample.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_a_npc_sp_player_selectNP",
        [
            Object(Matching, "d_a_npc_sp_player_selectNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_player_selectNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_player_selectNP/d_a_npc_sp_player_select.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_a_npc_sp_player_createNP",
        [
            Object(Matching, "d_a_npc_sp_player_createNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_player_createNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_player_createNP/d_a_npc_sp_player_create.cpp", extra_cflags=["-sym on"]),
        ],
    ),
    Rel(
        "d_a_npc_sp_demo_resetsanNP",
        [
            Object(Matching, "d_a_npc_sp_demo_resetsanNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_demo_resetsanNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_demo_resetsanNP/d_a_npc_sp_demo_resetsan.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_haniwaNP",
        [
            Object(Matching, "d_a_npc_sp_haniwaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_haniwaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_haniwaNP/d_a_npc_sp_haniwa.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_bugNP",
        [
            Object(Matching, "d_a_npc_sp_bugNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_bugNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_bugNP/d_a_npc_sp_bug.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_missing_childNP",
        [
            Object(Matching, "d_a_npc_sp_missing_childNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_missing_childNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_missing_childNP/d_a_npc_sp_missing_child.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_missing_motherNP",
        [
            Object(Matching, "d_a_npc_sp_missing_motherNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_missing_motherNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_missing_motherNP/d_a_npc_sp_missing_mother.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_dancerNP",
        [
            Object(Matching, "d_a_npc_sp_dancerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_dancerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_dancerNP/d_a_npc_sp_dancer.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_pyontaroNP",
        [
            Object(Matching, "d_a_npc_sp_pyontaroNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_pyontaroNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_pyontaroNP/d_a_npc_sp_pyontaro.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_resetsanNP",
        [
            Object(Matching, "d_a_npc_sp_resetsanNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_resetsanNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_resetsanNP/d_a_npc_sp_resetsan.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_perikoNP",
        [
            Object(Matching, "d_a_npc_sp_perikoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_perikoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_perikoNP/d_a_npc_sp_periko.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_sonchoNP",
        [
            Object(Matching, "d_a_npc_sp_sonchoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_sonchoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_sonchoNP/d_a_npc_sp_soncho.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_event_sonchoNP",
        [
            Object(Matching, "d_a_npc_sp_event_sonchoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_event_sonchoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_event_sonchoNP/d_a_npc_sp_event_soncho.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_ceremony_sonchoNP",
        [
            Object(Matching, "d_a_npc_sp_ceremony_sonchoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_ceremony_sonchoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_ceremony_sonchoNP/d_a_npc_sp_ceremony_soncho.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_tanukichiNP",
        [
            Object(Matching, "d_a_npc_sp_tanukichiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_tanukichiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_tanukichiNP/d_a_npc_sp_tanukichi.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_op_tanukichiNP",
        [
            Object(Matching, "d_a_npc_sp_op_tanukichiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_op_tanukichiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_op_tanukichiNP/d_a_npc_sp_op_tanukichi.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_kaburibaNP",
        [
            Object(Matching, "d_a_npc_sp_kaburibaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_kaburibaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_kaburibaNP/d_a_npc_sp_kaburiba.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_totakekeNP",
        [
            Object(Matching, "d_a_npc_sp_totakekeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_totakekeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_totakekeNP/d_a_npc_sp_totakeke.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_tunekichiNP",
        [
            Object(Matching, "d_a_npc_sp_tunekichiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_tunekichiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_tunekichiNP/d_a_npc_sp_tunekichi.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_asamiNP",
        [
            Object(Matching, "d_a_npc_sp_asamiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_asamiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_asamiNP/d_a_npc_sp_asami.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_kinuyoNP",
        [
            Object(Matching, "d_a_npc_sp_kinuyoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_kinuyoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_kinuyoNP/d_a_npc_sp_kinuyo.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_cafeNP",
        [
            Object(Matching, "d_a_npc_sp_cafeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_cafeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_cafeNP/d_a_npc_sp_cafe.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_cafe_guestNP",
        [
            Object(Matching, "d_a_npc_sp_cafe_guestNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_cafe_guestNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_cafe_guestNP/d_a_npc_sp_cafe_guest.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_catherineNP",
        [
            Object(Matching, "d_a_npc_sp_catherineNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_catherineNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_catherineNP/d_a_npc_sp_catherine.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_futaNP",
        [
            Object(Matching, "d_a_npc_sp_futaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_futaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_futaNP/d_a_npc_sp_futa.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_fukoNP",
        [
            Object(Matching, "d_a_npc_sp_fukoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_fukoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_fukoNP/d_a_npc_sp_fuko.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_franklinNP",
        [
            Object(Matching, "d_a_npc_sp_franklinNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_franklinNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_franklinNP/d_a_npc_sp_franklin.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_graceNP",
        [
            Object(Matching, "d_a_npc_sp_graceNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_graceNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_graceNP/d_a_npc_sp_grace.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_jijyoNP",
        [
            Object(Matching, "d_a_npc_sp_jijyoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_jijyoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_jijyoNP/d_a_npc_sp_jijyo.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_jonnyNP",
        [
            Object(Matching, "d_a_npc_sp_jonnyNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_jonnyNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_jonnyNP/d_a_npc_sp_jonny.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_kappeiNP",
        [
            Object(Matching, "d_a_npc_sp_kappeiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_kappeiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_kappeiNP/d_a_npc_sp_kappei.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_perioNP",
        [
            Object(Matching, "d_a_npc_sp_perioNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_perioNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_perioNP/d_a_npc_sp_perio.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_raccoNP",
        [
            Object(Matching, "d_a_npc_sp_raccoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_raccoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_raccoNP/d_a_npc_sp_racco.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_roranNP",
        [
            Object(Matching, "d_a_npc_sp_roranNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_roranNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_roranNP/d_a_npc_sp_roran.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_seiichiNP",
        [
            Object(Matching, "d_a_npc_sp_seiichiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_seiichiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_seiichiNP/d_a_npc_sp_seiichi.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_fortuneNP",
        [
            Object(Matching, "d_a_npc_sp_fortuneNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_fortuneNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_fortuneNP/d_a_npc_sp_fortune.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_sisyoNP",
        [
            Object(Matching, "d_a_npc_sp_sisyoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_sisyoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_sisyoNP/d_a_npc_sp_sisyo.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_honmaNP",
        [
            Object(Matching, "d_a_npc_sp_honmaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_honmaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_honmaNP/d_a_npc_sp_honma.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_jingleNP",
        [
            Object(Matching, "d_a_npc_sp_jingleNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_jingleNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_jingleNP/d_a_npc_sp_jingle.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_uomasaNP",
        [
            Object(Matching, "d_a_npc_sp_uomasaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_uomasaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_uomasaNP/d_a_npc_sp_uomasa.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_pumpkingNP",
        [
            Object(Matching, "d_a_npc_sp_pumpkingNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_pumpkingNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_pumpkingNP/d_a_npc_sp_pumpking.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_kutsumigakiNP",
        [
            Object(Matching, "d_a_npc_sp_kutsumigakiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_kutsumigakiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_kutsumigakiNP/d_a_npc_sp_kutsumigaki.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_gatekeeper_aNP",
        [
            Object(Matching, "d_a_npc_sp_gatekeeper_aNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_gatekeeper_aNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_gatekeeper_aNP/d_a_npc_sp_gatekeeper_a.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_gatekeeper_bNP",
        [
            Object(Matching, "d_a_npc_sp_gatekeeper_bNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_gatekeeper_bNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_gatekeeper_bNP/d_a_npc_sp_gatekeeper_b.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_goddessNP",
        [
            Object(Matching, "d_a_npc_sp_goddessNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_goddessNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_goddessNP/d_a_npc_sp_goddess.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_mysterycatNP",
        [
            Object(Matching, "d_a_npc_sp_mysterycatNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_mysterycatNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_mysterycatNP/d_a_npc_sp_mysterycat.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_yuutaroNP",
        [
            Object(Matching, "d_a_npc_sp_yuutaroNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_yuutaroNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_yuutaroNP/d_a_npc_sp_yuutaro.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_yuutaro_loftNP",
        [
            Object(Matching, "d_a_npc_sp_yuutaro_loftNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_yuutaro_loftNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_yuutaro_loftNP/d_a_npc_sp_yuutaro_loft.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_fuusenNP",
        [
            Object(Matching, "d_a_npc_sp_fuusenNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_fuusenNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_fuusenNP/d_a_npc_sp_fuusen.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_sp_golditem_sonchoNP",
        [
            Object(Matching, "d_a_npc_sp_golditem_sonchoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_sp_golditem_sonchoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_sp_golditem_sonchoNP/d_a_npc_sp_golditem_soncho.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_town_graceNP",
        [
            Object(Matching, "d_a_npc_town_graceNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_town_graceNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_town_graceNP/d_a_npc_town_grace.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_town_outNP",
        [
            Object(Matching, "d_a_npc_town_outNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_town_outNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_town_outNP/d_a_npc_town_out.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_town_theaterNP",
        [
            Object(Matching, "d_a_npc_town_theaterNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_town_theaterNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_town_theaterNP/d_a_npc_town_theater.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_visit_npc_houseNP",
        [
            Object(Matching, "d_a_npc_visit_npc_houseNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_visit_npc_houseNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_visit_npc_houseNP/d_a_npc_visit_npc_house.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_visit_player_houseNP",
        [
            Object(Matching, "d_a_npc_visit_player_houseNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_visit_player_houseNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_visit_player_houseNP/d_a_npc_visit_player_house.cpp"),
        ],
    ),
    Rel(
        "d_a_npc_visit_player_house_fleamarketNP",
        [
            Object(Matching, "d_a_npc_visit_player_house_fleamarketNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_npc_visit_player_house_fleamarketNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_npc_visit_player_house_fleamarketNP/d_a_npc_visit_player_house_fleamarket.cpp"),
        ],
    ),
    Rel(
        "d_a_playerNP",
        [
            Object(Matching, "d_a_playerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_playerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_playerNP/d_a_player.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_atmNP",
        [
            Object(Matching, "d_a_rmobj_atmNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_atmNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_atmNP/d_a_rmobj_atm.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_busNP",
        [
            Object(Matching, "d_a_rmobj_busNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_busNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_busNP/d_a_rmobj_bus.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_barricadeNP",
        [
            Object(Matching, "d_a_rmobj_barricadeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_barricadeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_barricadeNP/d_a_rmobj_barricade.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_cafeNP",
        [
            Object(Matching, "d_a_rmobj_cafeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_cafeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_cafeNP/d_a_rmobj_cafe.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_checkpNP",
        [
            Object(Matching, "d_a_rmobj_checkpNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_checkpNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_checkpNP/d_a_rmobj_checkp.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_pointNP",
        [
            Object(Matching, "d_a_rmobj_pointNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_pointNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_pointNP/d_a_rmobj_point.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_fossilNP",
        [
            Object(Matching, "d_a_rmobj_fossilNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_fossilNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_fossilNP/d_a_rmobj_fossil.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_graceNP",
        [
            Object(Matching, "d_a_rmobj_graceNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_graceNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_graceNP/d_a_rmobj_grace.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_perm_machineNP",
        [
            Object(Matching, "d_a_rmobj_perm_machineNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_perm_machineNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_perm_machineNP/d_a_rmobj_perm_machine.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_picNP",
        [
            Object(Matching, "d_a_rmobj_picNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_picNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_picNP/d_a_rmobj_pic.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_recycleboxNP",
        [
            Object(Matching, "d_a_rmobj_recycleboxNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_recycleboxNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_recycleboxNP/d_a_rmobj_recyclebox.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_resetNP",
        [
            Object(Matching, "d_a_rmobj_resetNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_resetNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_resetNP/d_a_rmobj_reset.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_sampleNP",
        [
            Object(Matching, "d_a_rmobj_sampleNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_sampleNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_sampleNP/d_a_rmobj_sample.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_sewing_machineNP",
        [
            Object(Matching, "d_a_rmobj_sewing_machineNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_sewing_machineNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_sewing_machineNP/d_a_rmobj_sewing_machine.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_telNP",
        [
            Object(Matching, "d_a_rmobj_telNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_telNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_telNP/d_a_rmobj_tel.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_theaterNP",
        [
            Object(Matching, "d_a_rmobj_theaterNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_theaterNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_theaterNP/d_a_rmobj_theater.cpp"),
        ],
    ),
    Rel(
        "d_a_rmobj_mirrorballNP",
        [
            Object(Matching, "d_a_rmobj_mirrorballNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_rmobj_mirrorballNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_rmobj_mirrorballNP/d_a_rmobj_mirrorball.cpp"),
        ],
    ),
    Rel(
        "d_a_sign_tagNP",
        [
            Object(Matching, "d_a_sign_tagNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_sign_tagNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_sign_tagNP/d_a_sign_tag.cpp"),
        ],
    ),
    Rel(
        "d_a_strNP",
        [
            Object(Matching, "d_a_strNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_strNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_strNP/d_a_str.cpp"),
        ],
    ),
    Rel(
        "d_a_str_cdbbsNP",
        [
            Object(Matching, "d_a_str_cdbbsNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_str_cdbbsNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_str_cdbbsNP/d_a_str_cdbbs.cpp"),
        ],
    ),
    Rel(
        "d_a_str_commonNP",
        [
            Object(Matching, "d_a_str_commonNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_str_commonNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_str_commonNP/d_a_str_common.cpp"),
        ],
    ),
    Rel(
        "d_a_str_fieldNP",
        [
            Object(Matching, "d_a_str_fieldNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_str_fieldNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_str_fieldNP/d_a_str_field.cpp"),
        ],
    ),
    Rel(
        "d_a_str_townNP",
        [
            Object(Matching, "d_a_str_townNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_str_townNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_str_townNP/d_a_str_town.cpp"),
        ],
    ),
    Rel(
        "d_a_ticket_counterNP",
        [
            Object(Matching, "d_a_ticket_counterNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_ticket_counterNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_ticket_counterNP/d_a_ticket_counter.cpp"),
        ],
    ),
    Rel(
        "d_a_ufoNP",
        [
            Object(Matching, "d_a_ufoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_ufoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_ufoNP/d_a_ufo.cpp"),
        ],
    ),
    Rel(
        "d_balloon_managerNP",
        [
            Object(Matching, "d_balloon_managerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_balloon_managerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_balloon_managerNP/d_balloon_manager.cpp", shift_jis=False),
        ],
    ),
    Rel(
        "d_bgi_drawNP",
        [
            Object(Matching, "d_bgi_drawNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_bgi_drawNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_bgi_drawNP/d_bgi_draw.cpp", extra_cflags=["-DDECLARE_STRCMP_EARLY"]),
        ],
    ),
    Rel(
        "d_bgo_drawNP",
        [
            Object(Matching, "d_bgo_drawNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_bgo_drawNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_bgo_drawNP/d_bgo_draw.cpp"),
        ],
    ),
    Rel(
        "d_bgt_drawNP",
        [
            Object(Matching, "d_bgt_drawNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_bgt_drawNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_bgt_drawNP/d_bgt_draw.cpp", shift_jis=False),
        ],
    ),
    Rel(
        "d_cockroach_managerNP",
        [
            Object(Matching, "d_cockroach_managerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_cockroach_managerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_cockroach_managerNP/d_cockroach_manager.cpp"),
        ],
    ),
    Rel(
        "d_fgobj_managerNP",
        [
            Object(Matching, "d_fgobj_managerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_fgobj_managerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_fgobj_managerNP/d_fgobj_manager.cpp"),
        ],
    ),
    Rel(
        "d_fish_museumNP",
        [
            Object(Matching, "d_fish_museumNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_fish_museumNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_fish_museumNP/d_fish_museum.cpp"),
        ],
    ),
    Rel(
        "d_ftr_managerNP",
        [
            Object(Matching, "d_ftr_managerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_ftr_managerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_ftr_managerNP/d_ftr_manager.cpp"),
        ],
    ),
    Rel(
        "d_insect_fieldNP",
        [
            Object(Matching, "d_insect_fieldNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_insect_fieldNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_insect_fieldNP/d_insect_field.cpp"),
        ],
    ),
    Rel(
        "d_insect_museumNP",
        [
            Object(Matching, "d_insect_museumNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_insect_museumNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_insect_museumNP/d_insect_museum.cpp"),
        ],
    ),
    Rel(
        "d_org_objNP",
        [
            Object(Matching, "d_org_objNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_org_objNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_org_objNP/d_org_obj.cpp", shift_jis=False),
        ],
    ),
    Rel(
        "d_shop_fgNP",
        [
            Object(Matching, "d_shop_fgNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_shop_fgNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_shop_fgNP/d_shop_fg.cpp"),
        ],
    ),
    Rel(
        "d_str_managerNP",
        [
            Object(Matching, "d_str_managerNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_str_managerNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_str_managerNP/d_str_manager.cpp"),
        ],
    ),
    Rel(
        "d_s_selectNP",
        [
            Object(NonMatching, "d_s_selectNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_s_selectNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_s_selectNP/d_s_select.cpp"),
        ],
    ),
    Rel(
        "d_s_stageNP",
        [
            Object(Matching, "d_s_stageNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_s_stageNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_s_stageNP/d_s_stage.cpp"),
        ],
    ),
    Rel(
        "d_title_demoNP",
        [
            Object(Matching, "d_title_demoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_title_demoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_title_demoNP/d_title_demo.cpp"),
        ],
    ),
    Rel(
        "d_flagNP",
        [
            Object(Matching, "d_flagNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_flagNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_flagNP/d_flag.cpp"),
        ],
    ),
    Rel(
        "d_countdown_flagNP",
        [
            Object(Matching, "d_countdown_flagNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_countdown_flagNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_countdown_flagNP/d_countdown_flag.cpp"),
        ],
    ),
    Rel(
        "d_organ_balloonNP",
        [
            Object(Matching, "d_organ_balloonNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_organ_balloonNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_organ_balloonNP/d_organ_balloon.cpp"),
        ],
    ),
    Rel(
        "d_menu_bankNP",
        [
            Object(Matching, "d_menu_bankNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_bankNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_bankNP/d_menu_bank.cpp"),
        ],
    ),
    Rel(
        "d_menu_catalogNP",
        [
            Object(NonMatching, "d_menu_catalogNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_catalogNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_catalogNP/d_menu_catalog.cpp"),
        ],
    ),
    Rel(
        "d_menu_chatNP",
        [
            Object(NonMatching, "d_menu_chatNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_chatNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_chatNP/d_menu_chat.cpp"),
        ],
    ),
    Rel(
        "d_menu_desi00NP",
        [
            Object(NonMatching, "d_menu_desi00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_desi00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_desi00NP/d_menu_desi00.cpp"),
        ],
    ),
    Rel(
        "d_menu_desi01NP",
        [
            Object(Matching, "d_menu_desi01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_desi01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_desi01NP/d_menu_desi01.cpp"),
        ],
    ),
    Rel(
        "d_menu_desi02NP",
        [
            Object(NonMatching, "d_menu_desi02NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_desi02NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_desi02NP/d_menu_desi02.cpp"),
        ],
    ),
    Rel(
        "d_menu_desi03NP",
        [
            Object(Matching, "d_menu_desi03NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_desi03NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_desi03NP/d_menu_desi03.cpp"),
        ],
    ),
    Rel(
        "d_menu_editNP",
        [
            Object(NonMatching, "d_menu_editNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_editNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_editNP/d_menu_edit.cpp"),
        ],
    ),
    Rel(
        "d_menu_friendNP",
        [
            Object(Matching, "d_menu_friendNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friendNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friendNP/d_menu_friend.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend01NP",
        [
            Object(Matching, "d_menu_friend01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend01NP/d_menu_friend01.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend02NP",
        [
            Object(Matching, "d_menu_friend02NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend02NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend02NP/d_menu_friend02.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend03NP",
        [
            Object(Matching, "d_menu_friend03NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend03NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend03NP/d_menu_friend03.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend04NP",
        [
            Object(Matching, "d_menu_friend04NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend04NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend04NP/d_menu_friend04.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend05NP",
        [
            Object(Matching, "d_menu_friend05NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend05NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend05NP/d_menu_friend05.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend06NP",
        [
            Object(Matching, "d_menu_friend06NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend06NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend06NP/d_menu_friend06.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend07NP",
        [
            Object(Matching, "d_menu_friend07NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend07NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend07NP/d_menu_friend07.cpp"),
        ],
    ),
    Rel(
        "d_menu_friend08NP",
        [
            Object(Matching, "d_menu_friend08NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_friend08NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_friend08NP/d_menu_friend08.cpp"),
        ],
    ),
    Rel(
        "d_menu_hanNP",
        [
            Object(NonMatching, "d_menu_hanNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_hanNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_hanNP/d_menu_han.cpp"),
        ],
    ),
    Rel(
        "d_menu_haniwaNP",
        [
            Object(NonMatching, "d_menu_haniwaNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_haniwaNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_haniwaNP/d_menu_haniwa.cpp"),
        ],
    ),
    Rel(
        "d_menu_item00NP",
        [
            Object(NonMatching, "d_menu_item00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item00NP/d_menu_item00.cpp"),
        ],
    ),
    Rel(
        "d_menu_item01NP",
        [
            Object(NonMatching, "d_menu_item01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item01NP/d_menu_item01.cpp"),
        ],
    ),
    Rel(
        "d_menu_item02NP",
        [
            Object(NonMatching, "d_menu_item02NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item02NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item02NP/d_menu_item02.cpp"),
        ],
    ),
    Rel(
        "d_menu_item03NP",
        [
            Object(NonMatching, "d_menu_item03NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item03NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item03NP/d_menu_item03.cpp"),
        ],
    ),
    Rel(
        "d_menu_item04NP",
        [
            Object(NonMatching, "d_menu_item04NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item04NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item04NP/d_menu_item04.cpp"),
        ],
    ),
    Rel(
        "d_menu_item05NP",
        [
            Object(NonMatching, "d_menu_item05NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item05NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item05NP/d_menu_item05.cpp"),
        ],
    ),
    Rel(
        "d_menu_item06NP",
        [
            Object(NonMatching, "d_menu_item06NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_item06NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_item06NP/d_menu_item06.cpp"),
        ],
    ),
    Rel(
        "d_menu_inscNP",
        [
            Object(NonMatching, "d_menu_inscNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_inscNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_inscNP/d_menu_insc.cpp"),
        ],
    ),
    Rel(
        "d_menu_kabuNP",
        [
            Object(Matching, "d_menu_kabuNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_menu_kabuNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_menu_kabuNP/d_menu_kabu.cpp"),
        ],
    ),
    Rel(
        "d_menu_keytestNP",
        [
            Object(NonMatching, "d_menu_keytestNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_keytestNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_keytestNP/d_menu_keytest.cpp"),
        ],
    ),
    Rel(
        "d_menu_map00NP",
        [
            Object(NonMatching, "d_menu_map00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_map00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_map00NP/d_menu_map00.cpp"),
        ],
    ),
    Rel(
        "d_menu_map01NP",
        [
            Object(NonMatching, "d_menu_map01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_map01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_map01NP/d_menu_map01.cpp"),
        ],
    ),
    Rel(
        "d_menu_map02NP",
        [
            Object(NonMatching, "d_menu_map02NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_map02NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_map02NP/d_menu_map02.cpp"),
        ],
    ),
    Rel(
        "d_menu_map03NP",
        [
            Object(NonMatching, "d_menu_map03NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_map03NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_map03NP/d_menu_map03.cpp"),
        ],
    ),
    Rel(
        "d_menu_melodyNP",
        [
            Object(NonMatching, "d_menu_melodyNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_melodyNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_melodyNP/d_menu_melody.cpp"),
        ],
    ),
    Rel(
        "d_menu_miiNP",
        [
            Object(NonMatching, "d_menu_miiNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_miiNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_miiNP/d_menu_mii.cpp"),
        ],
    ),
    Rel(
        "d_menu_museumNP",
        [
            Object(NonMatching, "d_menu_museumNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_museumNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_museumNP/d_menu_museum.cpp"),
        ],
    ),
    Rel(
        "d_menu_musicNP",
        [
            Object(NonMatching, "d_menu_musicNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_musicNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_musicNP/d_menu_music.cpp"),
        ],
    ),
    Rel(
        "d_menu_mslctNP",
        [
            Object(NonMatching, "d_menu_mslctNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_mslctNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_mslctNP/d_menu_mslct.cpp"),
        ],
    ),
    Rel(
        "d_menu_noticeNP",
        [
            Object(NonMatching, "d_menu_noticeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_noticeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_noticeNP/d_menu_notice.cpp"),
        ],
    ),
    Rel(
        "d_menu_notice00NP",
        [
            Object(NonMatching, "d_menu_notice00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_notice00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_notice00NP/d_menu_notice00.cpp"),
        ],
    ),
    Rel(
        "d_menu_notice01NP",
        [
            Object(NonMatching, "d_menu_notice01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_notice01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_notice01NP/d_menu_notice01.cpp"),
        ],
    ),
    Rel(
        "d_menu_number00NP",
        [
            Object(NonMatching, "d_menu_number00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_number00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_number00NP/d_menu_number00.cpp"),
        ],
    ),
    Rel(
        "d_menu_paperNP",
        [
            Object(NonMatching, "d_menu_paperNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_paperNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_paperNP/d_menu_paper.cpp"),
        ],
    ),
    Rel(
        "d_menu_photoNP",
        [
            Object(NonMatching, "d_menu_photoNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_photoNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_photoNP/d_menu_photo.cpp"),
        ],
    ),
    Rel(
        "d_menu_photo01NP",
        [
            Object(NonMatching, "d_menu_photo01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_photo01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_photo01NP/d_menu_photo01.cpp"),
        ],
    ),
    Rel(
        "d_menu_pointNP",
        [
            Object(NonMatching, "d_menu_pointNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_pointNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_pointNP/d_menu_point.cpp"),
        ],
    ),
    Rel(
        "d_menu_post00NP",
        [
            Object(NonMatching, "d_menu_post00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_post00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_post00NP/d_menu_post00.cpp"),
        ],
    ),
    Rel(
        "d_menu_post01NP",
        [
            Object(NonMatching, "d_menu_post01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_post01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_post01NP/d_menu_post01.cpp"),
        ],
    ),
    Rel(
        "d_menu_star00NP",
        [
            Object(NonMatching, "d_menu_star00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_star00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_star00NP/d_menu_star00.cpp"),
        ],
    ),
    Rel(
        "d_menu_star01NP",
        [
            Object(NonMatching, "d_menu_star01NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_star01NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_star01NP/d_menu_star01.cpp"),
        ],
    ),
    Rel(
        "d_menu_time00NP",
        [
            Object(NonMatching, "d_menu_time00NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_time00NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_time00NP/d_menu_time00.cpp"),
        ],
    ),
    Rel(
        "d_menu_writeNP",
        [
            Object(NonMatching, "d_menu_writeNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(NonMatching, "d_menu_writeNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(NonMatching, "d_menu_writeNP/d_menu_write.cpp"),
        ],
    ),
    Rel(
        "d_a_layoutNP",
        [
            Object(Matching, "d_a_layoutNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_layoutNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_layoutNP/d_a_layout.cpp"),
        ],
    ),
    Rel(
        "d_a_mdlNP",
        [
            Object(Matching, "d_a_mdlNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_mdlNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_mdlNP/d_a_mdl.cpp"),
        ],
    ),
    Rel(
        "d_a_sampleNP",
        [
            Object(Matching, "d_a_sampleNP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_sampleNP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_sampleNP/d_a_sample.cpp"),
        ],
    ),
    Rel(
        "d_a_sample2NP",
        [
            Object(Matching, "d_a_sample2NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_sample2NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_sample2NP/d_a_sample2.cpp"),
        ],
    ),
    Rel(
        "d_a_sample3NP",
        [
            Object(Matching, "d_a_sample3NP/rel_init.cpp", source="runtime/rel_init.cpp"),
            Object(Matching, "d_a_sample3NP/global_destructor_chain.c", source="runtime/global_destructor_chain.c",
                   cflags=[*cflags_runtime, "-sdata 0", "-sdata2 0"]),
            Object(Matching, "d_a_sample3NP/d_a_sample3.cpp"),
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
