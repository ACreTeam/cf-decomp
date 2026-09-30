# City Folk configuration setup

`tools/prepare_config.py` completes the inventory produced by `dtk dol config`.
Run it from this repository (the directory containing `configure.py`):

```powershell
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' tools/prepare_config.py
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' tools/prepare_config.py --check
```

The helper uses PyYAML. If needed, install `tools/requirements-config.txt` with
the same interpreter and `-m pip install -r tools/requirements-config.txt`.
`--version`, `--input`, `--output`, and `--build-dir` can override the defaults;
all relative paths are relative to this repository, independent of the working directory.

For RUUE01_00, the input is `config/RUUE01_00/config.generated.yml` and the
output is `config/RUUE01_00/config.yml`. The helper validates the SHA-1 hashes
of the extracted DOL and all 187 RELs, checks REL IDs and imports, and adds
separate `symbols` and `splits` destinations for every binary. Object paths
are normalized relative to `orig/RUUE01_00`. It also writes `build.sha1` with
the template's output paths: `build/RUUE01_00/main.dol` and
`build/RUUE01_00/<module>/<module>.rel`.

The generated inventory stays unchanged. Existing settings are retained by
object path on reruns, including custom symbol/split paths, relocation fixes,
and link lists. Config formatting/comments are regenerated; previous config
and hash-manifest contents are backed up under `build/config-backups/RUUE01_00`.
Existing symbol and split analysis is preserved. For the verified RUUE01_00
DOL and three REL hashes, blank/comment-only split files get initial section headers.
The DOL profile also seeds the required exception-runtime split described below.
Missing parent directories are created; DTK's initial analysis creates/populates
the remaining files.
`--check` verifies originals and output freshness without writing anything.

## Analysis settings

Useful settings from `config.example.yml` are made explicit: full analysis,
object/string detection, disassembly output, gap filling, symbol exporting and
globalization. Original exception-table bytes are preserved with
`clean_extab: false`. The project's existing `mw_comment_version: 14` is
retained; this is not a detection of the exact original compiler/linker.

There is no map file. Even complete class RTTI only identifies some data and
relationships, so `symbols_known` and `quick_analysis` stay false. DTK has no
config option that consumes the recovered RTTI inheritance JSON directly.
Verified names can be added to `symbols.txt` later. REL symbols must use their
REL section offsets, not Ghidra's chosen absolute load addresses. Class
relationships alone also do not establish original source-file boundaries.

All 187 REL IDs are unique and their imports refer to the DOL or an included
REL. The default DTK linkage is sufficient; no guessed `links` lists are added.
The helper refuses duplicate IDs or missing dependencies instead of inventing
link groups. No map, extraction recipe, relocation correction, or common-BSS
boundary is fabricated.

## BSS evidence for RUUE01_00

The original DOL's startup zero-initialization table at `0x800064A4` (file
offset `0x25A4`) agrees with DTK's section detection and Ghidra's memory blocks:

| Region | Start | Size | End (exclusive) |
| --- | --- | --- | --- |
| `.bss` | `0x80562E80` | `0x1E6824` | `0x807496A4` |
| `.sbss` | `0x8074E120` | `0x1C1C` | `0x8074FD3C` |
| `.sbss2` | `0x807539E0` | `0x60` | `0x80753A40` |

Ghidra's first two uninitialized blocks include some trailing alignment padding.
The DOL header reports an envelope starting at `0x80562E80` with size
`0x1F0BC0`; this spans the three regions and intervening initialized data.
It does not identify a linker-common BSS boundary. `common_start` remains
unset because the available evidence does not establish one. This does not
assert that the game has no common symbols. See [Common BSS](common_bss.md)
for marking confirmed common allocations in split files later.

DTK 1.8.3 initially calls the final region `.bss2` in `dol info`, but its full
analysis can rename it to a second `.sbss`. That produces split/symbol files
which fail on the next run with `Multiple sections with name .sbss`. The helper
seeds explicit, unique section names before initial analysis to prevent this.
The checked-in main splits and symbols use `.sbss2` for addresses `0x807539E0`
through `0x80753A3F`. The original instructions confirm r2-relative access:
at `0x803FA8E8`, instruction `0x8062BCE8` is `lwz r3,-0x4318(r2)`;
`_SDA2_BASE_ = 0x80757D40`, so it accesses `0x80753A28`. Naming this section
`.bss2` lets DTK split it, but causes MW linker error 109 because it is not
recognized as a small-data section. `.sbss2` preserves the r2 relocation
semantics. This does not establish a linker-common BSS boundary.

Three RELs (`d_a_rmobj_cafeNP`, `d_a_rmobj_fossilNP`, and `d_a_rmobj_sampleNP`)
similarly acquire duplicate `.rodata` names. Their section profiles use
`.ctors`, `.dtors`, `.rodata`, and `.data` for REL indices 2 through 5.
Their `_prolog` and `_epilog` pass sections 2 and 3 to the shared constructor
and destructor runners at DOL addresses `0x802AAC34` and `0x802AAC78`.
Using arbitrary `.rodata2`/`.rodata3` names instead makes MW generate extra
`.ctors`/`.dtors` sections and fail internally during partial linking.

The initial symbol files also include two verified boundary corrections needed
to reload DTK's generated analysis:

- The DOL function at `0x8033AD2C` extends through `0x8033B6AF` (size `0x984`).
  Ghidra agrees; the tail at `0x8033B644` is a switch label, not another function.
- In `d_fg_drawNP`, the table at `.data:0x2A0` has three relocated code pointers
  (size `0xC`). The bytes at `0x2AC` begin strings, so they must be a separate
  object, not extra jump-table entries.

Both full initial analysis and reloading the corrected symbol/split files were
checked with DTK 1.8.3. Keep the generated analysis files: the helper preserves
these corrections on reruns. `quick_analysis` remains false.

The helper prepares DTK analysis configuration. Matching compiled source still
needs the real compiler choices and per-library flags in `configure.py`.
The project's linker is set to `Wii/1.0`; the remaining template library
compiler choices and `-RTTI off` flags are not established by RTTI.

## Matching rebuild

The DOL and all 187 RELs pass the original SHA-1 manifest with the `Wii/1.0`
linker. The final fix was the exception-runtime split required for GC 2.7+
and Wii linkers in [Getting Started](getting_started.md#gc-27-and-wii-linkers):

```text
Runtime.PPCEABI.H/__init_cpp_exceptions.cpp:
    .text   start:0x8044EE58 end:0x8044EEC8
    .ctors  start:0x80465620 end:0x80465624 rename:.ctors$10
    .dtors  start:0x80465960 end:0x80465964 rename:.dtors$10
    .dtors  start:0x80465964 end:0x80465968 rename:.dtors$15
    .sdata  start:0x8074E0D8 end:0x8074E0E0
```

These ranges come from the DOL's identified runtime functions and references.
The subsection names preserve the special constructor/destructor ordering
expected by the linker. Without them, the missing runtime entries shifted DOL
data addresses, affecting references from many RELs. Correcting this split
resolved all 184 remaining checksum mismatches together.

Petari's configuration was reviewed as a reference, but its compiler settings
and map-based source layout were not copied. No `configure.py` changes were
needed for this final fix. Original binaries, expected hashes, and verification
rules remain unchanged; outputs are produced by DTK and the linker without
post-link byte patches. A passing hash here means a matching reconstruction
from the extracted objects, not that any game source has been decompiled yet.

Build and DOL comparison logs are saved under `build/matching-build.log` and
`build/matching-dol-diff.log`. A successful `dtk dol diff` produces no output.

## Helper tests

```powershell
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' -m unittest discover -s tools -p test_prepare_config.py -v
```
