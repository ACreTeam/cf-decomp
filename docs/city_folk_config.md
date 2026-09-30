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
An empty DOL symbols file gets the verified first `.text` function's 16-byte
alignment override; other code defaults to four-byte alignment.
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
post-link byte patches. That initial baseline used extracted objects throughout.
The first source-backed object is described below.

Build and DOL comparison logs are saved under `build/matching-build.log` and
`build/matching-dol-diff.log`. A successful `dtk dol diff` produces no output.

## First shared source split: c_line.cpp

`src/dol/cLib/c_line.cpp` is the first matching source-backed object. It contains
four `cLineMg_c` linked-list methods, with no out-of-line constructors, vtables,
or data allocations. The imported source and headers compile unchanged.

[NSMBW's slice configuration](https://github.com/NSMBW-Community/NSMBW-Decomp/blob/master/slices/wiimj2d.json)
lists this file as a code-only range at `.text` offsets
`0x15A8D0..0x15AA20`, totaling `0x150` bytes. This suggests a candidate grouping;
City Folk's addresses were established independently by compiling the imported
source and searching the original DOL for its instructions.

| Method | RUUE01_00 address | Size |
| --- | --- | --- |
| `cLineMg_c::insertLineNode` | `0x802AB6D0` | `0x50` |
| `cLineMg_c::removeLineNode` | `0x802AB720` | `0x80` |
| `cLineMg_c::addLastLineNode` | `0x802AB7A0` | `0x40` |
| `cLineMg_c::addTopLineNode` | `0x802AB7E0` | `0x40` |

Each compiled function has one unique match in the DOL after accounting for
relocations. The only relocation is the tail branch at `0x802AB6D8` to
`addTopLineNode` at `0x802AB7E0`, also confirmed in Ghidra. These contiguous
functions occupy `.text` from `0x802AB6D0` through `0x802AB820` (exclusive),
with no gaps or extra functions. The split and four mangled symbols are recorded
in `config/RUUE01_00/splits.txt` and `symbols.txt`.

`configure.py` registers `dol/cLib/c_line.cpp` as `Matching`, using `Wii/1.0`
and scoped `cflags_clib` based on NSMBW's default code-generation flags.
Objdiff reports all four functions and all 336 code bytes matched, and the
source-backed build passes the original DOL plus all 187 REL checksums.
The verification log is `build/shared-file-test/build.log`.

This validates a usable source split without a linker map. It does not prove
the historical filename or original translation-unit boundaries, nor establish
compiler/RTTI flags for other engine files. Validate each additional file's
code, data, relocations, and final checksum before marking it matching.

## Remaining cLib scan

All nine imported `src/dol/cLib/*.cpp` files were compiled with Wii/1.0 and
scanned against RUUE01_00. Their order and contents are only partly shared with
NSMBW. No imported C++ source or header changes were needed for this scan.

| File | Current status | City Folk `.text` range (end exclusive) |
| --- | --- | --- |
| `c_counter.cpp` | Matching, linked from source | `0x802AA10C..0x802AA11C` |
| `c_dylink.cpp` | NonMatching, code split for comparison | `0x802AA11C..0x802AAD00` |
| `c_lib.cpp` | NonMatching, code/constants split for comparison | `0x802AAD00..0x802AB6D0` |
| `c_line.cpp` | Matching, linked from source | `0x802AB6D0..0x802AB820` |
| `c_math.cpp` | NonMatching, code/data split for comparison | `0x802AB820..0x802ABC00` |
| `c_tree.cpp` | Matching, linked from source | `0x802ABC00..0x802ABDE0` |
| `c_m3d.cpp` | NonMatching, no reliable location yet | Unassigned |
| `c_owner_set.cpp` | NonMatching, no reliable location yet | Unassigned |
| `c_random.cpp` | NonMatching, code/data split for comparison | `0x802AA07C..0x802AA10C` |

The three matching objects total 832 code bytes, 11 functions, and 8 data bytes.
All have 100% objdiff matches and are used by the normal link. The complete DOL
and all 187 RELs still pass the original checksums. NonMatching units with splits
compile for objdiff but use their extracted objects in the link. The two
unassigned entries are recorded in `configure.py`; without established splits,
they do not yet have Ninja/objdiff units. They were compiled separately for the scan.

`c_counter.cpp` accesses `m_gameFrame` and `m_exeFrame` at `0x8074EFF0` and
`0x8074EFF4`. The startup register initialization confirms `r13 = 0x807516C0`;
the stores use offsets `-0x26D0` and `-0x26CC`. `c_tree.cpp` matches unchanged
after adding `-func_align 4` following `-O4`. MWCC's `-O4` otherwise selects
16-byte function alignment and introduces different loop padding as well.

The DOL `.text` section default and the helper's initial profile now use
`align:4`. The first function at `0x800075C0` explicitly retains `align:16`:
`extabindex` ends at `0x800075B8`, so omitting that override moves the whole code
section eight bytes earlier. The original DOL header and checksum comparison
establish this exception. Preserve it when changing section or split defaults.

The NonMatching ranges are working groupings supported by function comparisons
and references, not recovered linker-map records. `c_dylink` has a matching
checksum helper and constructor/destructor runners, but different module-control
code; its data ownership still needs work. `c_lib` has additional City Folk
functions and different vector code. Its shared methods were identified using
instruction matches and Ghidra's control/data flow; their imported type names
are working names for comparison, not independent proof of the original types.
`c_math` has five exact larger-function matches and the same 1,025-entry arctangent
table at `0x80523160`, but extra functions and different RNG initialization.
Its initializer at `0x802ABB98` uses the constructor pointer at `0x804658BC`,
two RNG objects at `0x8074F010..0x8074F018`, and destructor-registration storage
at `0x806A5CA0..0x806A5CB8`. These references establish its data splits.

`c_random` now has a working `.text` split at `0x802AA07C..0x802AA10C`:
`cRandom_c::setSeed(u32)` at `0x802AA07C` (8 bytes), the no-argument raw RNG
step at `0x802AA084` (80 bytes), and `cRandom_c::getRandomF()` at `0x802AA0D4`
(56 bytes). The raw step is named `ranqdStep` to match the imported helper;
these are reconstructed names, not recovered linker-map symbols. Ghidra confirms
the function boundaries and the float routine's reference to `1.0f` at
`0x807528D8`; its `.sdata2` split includes the following four padding bytes,
ending at `0x807528E0`. The object remains `NonMatching`: the imported source
also contains scaled-integer and second-float methods, and inlines its raw step,
so configuring the split does not establish a source match.

The scanner is now saved as `tools/find_candidates.py`. It preserves instruction
opcodes and register fields when masking relocations, handles branch-only
functions, rejects unsupported relocations, and can check known branch/SDA
targets and DTK function boundaries. It only reads inputs and reports candidates;
always validate relocation targets and the final checksum before linking a file.
For example, after building the configured source objects:

```powershell
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' tools/find_candidates.py build/RUUE01_00/src/dol/cLib/c_tree.o --dol orig/RUUE01_00/sys/main.dol --symbols config/RUUE01_00/symbols.txt --r2 0x80757D40 --r13 0x807516C0
```

The nine-object scan is saved at `build/shared-file-test/clib-scan.json`;
the passing build and empty DOL diff logs are `clib-build.log` and
`clib-dol-diff.log` in the same directory.

## Helper tests

```powershell
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' -m unittest discover -s tools -p test_prepare_config.py -v
& 'C:\Users\olsen\AppData\Local\Programs\Python\Python313\python.exe' -m unittest discover -s tools -p test_find_candidates.py -v
```
