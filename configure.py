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

# REL flags
cflags_rel = [
    *cflags_base,
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
            Object(Matching, "dol/game/d_demo_actor.cpp"),
            Object(Matching, "dol/game/d_msg_rcpt.cpp"),
            Object(Matching, "dol/game/d_fg_item.cpp", extra_cflags=["-sym on"]),
            Object(NonMatching, "dol/game/d_item.cpp", shift_jis=False),
            Object(NonMatching, "dol/game/d_player_mgr.cpp"),
            Object(NonMatching, "dol/game/d_save_data.cpp"),
            Object(Matching, "dol/game/d_dsn.cpp"),
            Object(Matching, "dol/game/d_mail.cpp"),
            Object(NonMatching, "dol/game/d_animal.cpp"),
            Object(Matching, "dol/game/d_animal_id.cpp"),
            Object(NonMatching, "dol/game/d_private_data.cpp"),
            Object(Matching, "dol/game/d_personal_id.cpp"),
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
            Object(NonMatching, "lib/keyboard/tiCandidateBox.cpp"),
            Object(NonMatching, "lib/keyboard/tiToolBar.cpp"),
            Object(NonMatching, "lib/keyboard/tiPredictLang.cpp"),
            Object(NonMatching, "lib/keyboard/tiSignWindow.cpp"),
            Object(NonMatching, "lib/keyboard/tiString.cpp"),
            Object(NonMatching, "lib/keyboard/tiTextDrawer.cpp"),
            Object(NonMatching, "lib/keyboard/tiNw4rManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiUtil.cpp"),
            Object(NonMatching, "lib/keyboard/tiCpData.cpp"),
            Object(NonMatching, "lib/keyboard/tiSwData.cpp"),
            Object(NonMatching, "lib/keyboard/tiPkData.cpp"),
            Object(NonMatching, "lib/keyboard/tiLanguageIndependentData.cpp"),
            Object(NonMatching, "lib/keyboard/tiTextInputBase.cpp"),
            Object(NonMatching, "lib/keyboard/MyTiManager.cpp"),
            Object(NonMatching, "lib/keyboard/MyTiInputForm.cpp"),
            Object(NonMatching, "lib/keyboard/MyTiBg.cpp"),
            Object(NonMatching, "lib/keyboard/tiDebug.cpp"),
            Object(NonMatching, "lib/keyboard/tiGUIManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiHKBManager.cpp"),
            Object(NonMatching, "lib/keyboard/tiHwKeyboard.cpp"),
            Object(NonMatching, "lib/keyboard/tiLayout.cpp"),
            Object(NonMatching, "lib/keyboard/tiLayoutGather.cpp"),
        ],
    },
    {
        "lib": "RevoEX",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "lib/RevoEX/ssl/ssl_api.c"),
            Object(NonMatching, "lib/RevoEX/wd/wd_misc.c"),
            Object(NonMatching, "lib/RevoEX/wd/wd_request.c"),
            Object(NonMatching, "lib/RevoEX/wd/wd_init.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24StdAPI.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24FileAPI.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Config.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Utils.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Manage.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24MsgObj.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Parser.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Schedule.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24FriendList.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24SecretFList.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24UserId.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Time.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24Ipc.c"),
            Object(NonMatching, "lib/RevoEX/nwc24/NWC24System.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_clib.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_code.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_service.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_str.c"),
            Object(NonMatching, "lib/RevoEX/vf/common/pf_w_clib.c"),
            Object(NonMatching, "lib/RevoEX/vf/driver/pf_driver.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_bpb.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_disk.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_partition.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_mbr.c"),
            Object(NonMatching, "lib/RevoEX/vf/dskmng/pdm_dskmng.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_cache.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_cluster.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_entry.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_entry_iterator.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat12.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat16.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fat32.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_fatfs.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_file.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_path.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_sector.c"),
            Object(NonMatching, "lib/RevoEX/vf/fatfs/pf_volume.c"),
            Object(NonMatching, "lib/RevoEX/vf/standard/pf_api_util.c"),
            Object(NonMatching, "lib/RevoEX/vf/standard/pf_init_prfile2.c"),
            Object(NonMatching, "lib/RevoEX/vf/standard/pf_unmount.c"),
            Object(NonMatching, "lib/RevoEX/vf/system/pf_filelock.c"),
            Object(NonMatching, "lib/RevoEX/vf/system/pf_system.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_vf.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_vf_sys.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_hash.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_time.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/d_common.c"),
            Object(NonMatching, "lib/RevoEX/vf/develop/nand_drv.c"),
        ],
    },
    {
        "lib": "RVL_SDK",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(NonMatching, "lib/RVL_SDK/ai/ai.c"),
            Object(NonMatching, "lib/RVL_SDK/arc/arc.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AX.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXAlloc.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXAux.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXCL.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXOut.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXSPB.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXVPB.c"),
            Object(NonMatching, "lib/RVL_SDK/ax/AXProf.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHi.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHiExp.c"),
            Object(NonMatching, "lib/RVL_SDK/axfx/AXFXReverbHiExpDpl2.c"),
            Object(NonMatching, "lib/RVL_SDK/base/PPCArch.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OS.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSAlarm.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSAlloc.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSArena.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSAudioSystem.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSCache.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSContext.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSError.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSExec.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSFont.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSInterrupt.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSLink.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMessage.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMemory.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSMutex.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSReset.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSRtc.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSSemaphore.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSSync.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSThread.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSTime.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSUtf.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSIpc.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSStateTM.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSPlayRecord.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSStateFlags.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSNet.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSNandbootInfo.c"),
            Object(NonMatching, "lib/RVL_SDK/os/OSPlayTime.c"),
            Object(NonMatching, "lib/RVL_SDK/os/__ppc_eabi_init.cpp"),
            Object(NonMatching, "lib/RVL_SDK/exi/EXIBios.c"),
            Object(NonMatching, "lib/RVL_SDK/exi/EXIUart.c"),
            Object(NonMatching, "lib/RVL_SDK/exi/EXICommon.c"),
            Object(NonMatching, "lib/RVL_SDK/si/SIBios.c"),
            Object(NonMatching, "lib/RVL_SDK/si/SISamplingRate.c"),
            Object(NonMatching, "lib/RVL_SDK/db/db.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/vi.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/i2c.c"),
            Object(NonMatching, "lib/RVL_SDK/vi/vi3in1.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/mtx.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/mtx44.c"),
            Object(NonMatching, "lib/RVL_SDK/mtx/vec.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXInit.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXFifo.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXAttr.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXMisc.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXGeometry.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXFrameBuf.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXLight.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXTexture.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXBump.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXTev.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXPixel.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXDisplayList.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXTransform.c"),
            Object(NonMatching, "lib/RVL_SDK/gx/GXPerf.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdfs.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvd.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdqueue.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvderror.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdidutils.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvdFatal.c"),
            Object(NonMatching, "lib/RVL_SDK/dvd/dvd_broadway.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_heapCommon.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_expHeap.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_frameHeap.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_allocator.c"),
            Object(NonMatching, "lib/RVL_SDK/mem/mem_list.c"),
            Object(NonMatching, "lib/RVL_SDK/dsp/dsp.c"),
            Object(NonMatching, "lib/RVL_SDK/dsp/dsp_debug.c"),
            Object(NonMatching, "lib/RVL_SDK/dsp/dsp_task.c"),
            Object(NonMatching, "lib/RVL_SDK/esp/esp.c"),
            Object(NonMatching, "lib/RVL_SDK/cx/CXUncompression.c"),
            Object(NonMatching, "lib/RVL_SDK/cx/CXSecureUncompression.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/nand.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDOpenClose.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDCore.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDSecret.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDCheck.c"),
            Object(NonMatching, "lib/RVL_SDK/nand/NANDLogging.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scsystem.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scapi.c"),
            Object(NonMatching, "lib/RVL_SDK/sc/scapi_prdinfo.c"),
            Object(NonMatching, "lib/RVL_SDK/pad/Pad.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPAD.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPADHIDParser.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/WPADEncrypt.c"),
            Object(NonMatching, "lib/RVL_SDK/wpad/debug_msg.c"),
            Object(NonMatching, "lib/RVL_SDK/usb/usb.c"),
            Object(NonMatching, "lib/RVL_SDK/wud/WUD.c"),
            Object(NonMatching, "lib/RVL_SDK/wud/WUDHidHost.c"),
            Object(NonMatching, "lib/RVL_SDK/wud/debug_msg.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_buffer.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_time.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gki_ppc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hcisu_h2.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/uusb_ppc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_hcisu.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_init.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_logmsg.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bte_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_task1.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bd.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_sys_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_sys_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/ptim.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/utl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_act.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_dm_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_act.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/bta_hh_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_acl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_dev.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_devctl.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_discovery.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_inq.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_sco.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btm_sec.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_hcif.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/btu_init.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/wbt_ext.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/gap_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hcicmds.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_mgmt.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidd_pm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidh_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/hidh_conn.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_csm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_link.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/l2c_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_rfc.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/port_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_l2cap_if.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_mx_fsm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_port_fsm.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_port_if.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_ts_frames.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/rfc_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_api.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_db.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_discovery.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_main.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_server.c"),
            Object(NonMatching, "lib/RVL_SDK/bte/sdp_utils.c"),
            Object(NonMatching, "lib/RVL_SDK/fs/fs.c"),
            Object(NonMatching, "lib/RVL_SDK/euart/euart.c"),
            Object(NonMatching, "lib/RVL_SDK/wenc/wenc.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encutility.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encunicode.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encjapanese.c"),
            Object(NonMatching, "lib/RVL_SDK/enc/encconvert.c"),
            Object(NonMatching, "lib/RVL_SDK/tpl/TPL.c"),
        ],
    },
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
