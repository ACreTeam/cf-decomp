# Compiler introspection tools (MWCC GC 3.0a5.2)

Use these when a function won't match and you need to know *why* the compiler does what it does.
Read `notes/mwcc_ghidra.md` first: its section 1 lists cheap explanations that need no tools, and 1b
lists the codegen rules found so far. Run everything from the repo root after a `configure.py`.
Nothing here touches `build/RUUE01_00`, so it's safe to run in parallel with other work and with ninja.

| Tool | What it answers |
|---|---|
| `score.py` | "What does this variant score?" Compiles a source or variant with the unit's real flags, no ninja, and diffs functions against the target with objdiff. Supports an overlay include dir for header experiments. |
| `iro_dump.py` | "What does the optimizer do to this function?" Prints the IRO flowgraph after every optimizer pass (CSE, propagation, dead stores, scalarization, ...), from a patched copy of the compiler. |
| `variants.py` | "Which of these rewrites (or which combination) fixes it, without breaking anything else?" Compiles every source/header rewrite in a list, and optionally their combinations, in parallel. It ranks them by the score of the functions you care about, and flags any other function in the unit that got worse. |
| `regalloc.py` | "Why did this value get that register?" Debugs the real compiler and prints the virtual-register numbering, the interference graph, the colouring order and the final assignment. `--sim` re-runs the colouring offline. `--want "this=r28,fl=r29"` searches for which renumbering (move, swap, or `--movers` placement) would give the registers you want. Windows only. |
| `enode.py` | "What expression tree does the back end see for this statement?" Dumps each statement's ENode tree before and after TOC (the pass that narrows types and strips conversions): node kinds, result types, constants, objects, call targets. The tool for extra or missing extends and conversions (`extsh`, `clrlwi`, `neg` width). Windows only. |
| `peephole.py` | "What did the peephole (or PCode copy propagation) change?" Prints a per-pass PCode diff for the peephole passes, copy and add propagation, and value numbering, before and after register allocation, with the peephole rule that fired. The tool for "one instruction too many/few" and fused-vs-unfused pairs (e.g. `mr; extsh` vs `extsh rX,r3`). Windows only. |
| `sched.py` | "Why are these two instructions in this order?" Debugs the real compiler and prints each scheduled block of a function (PCode plus alias records), then the scheduler's ready list every cycle, for both passes (before and after register allocation). Windows only. |
| `vregs.py` | "What *is* each virtual register?" For every GPR (or `--fpr`) vreg of a function: its final register, colouring rank and degree, and its origin. Objects: argument / named local / `@` temp, with who made the `@` temp (inliner, which IRO pass, front end) and a flag when no instruction uses the object (its value was propagated away). Codegen temps: the statement (index, source line, rendered expression), the ENode being generated, the creating code generator function, and the defining PCode instruction (`def: rlwinm v54, v53, 27, 5, 31`) to match against the target asm. Pair it with `regalloc.py --want`: --want says which vreg must move, vregs says what that vreg is in the source. |
| `iro_why.py` | "What did the IR optimizer do to my function, and why?" Reads the compiler's own IRO log (patched compiler, like iro_dump) and the debugger together: a per-variable summary (defs/uses after each pass, which pass removed a local's last use, which pass created each `@` temp), the loops (preheader created / refused, which loop passes ran), every `&&`/`||` rebuild attempt with its exact refusal reason and the blocking statement, every copy/constant and expression propagation decision (replaced, or why not: killed on the way, used twice in the block, read in another block, types differ, deeper loop, never tried because no use in the same block), and a timeline of each pass's decisions (`Found propagatable assignment at: 10 -> weekly = ...`, `Removing dead assignment ...`, `Replacing common sub ...`) and changed statements. `--var` filters to some variables; `--log` reuses a saved log. ~5 s for one function of a big TU. |
| `stackslots.py` | "Why is this local at that stack offset?" The final frame of a function: each object's r1 offset, size, what it is (named local, argument, `@` temp and its maker, codegen temp and its statement), the size pass that placed it and its creation rank. Slots are given in size passes (<=1, <=2, <=4 ... bytes), within a pass newest-created first; objects no instruction addresses are dropped. |
| `mwcc_cmd.py` | Shared helper: the exact ninja compile command for any source. Run it directly to print the command. |
| `dbgcore.py` | Shared helper: a tiny Win32 debugger. Give it the compile argv and `{address: handler}`, and your handler runs every time the compiler executes that address. Also: breakpoints armed per function (`lazy=`, `p.arm/p.disarm`), hardware write watchpoints (`watch=`, active while `p.watching`), and a validated call-chain scan (`callers`) with function names from `mwcc30_names.txt`. Build new introspection tools on it. |
| `mwcc30_names.txt` | Function starts and names of mwcceppc.exe exported from the Ghidra project (FUN_ = unnamed); dbgcore uses it to name addresses. Regenerate with the curl line at its top after renaming in Ghidra. |

## Typical loop

```sh
# 1. how far off is it, and where?
python tools/mwcc/score.py src/d_fish_fieldNP/d_fish_field.cpp -f "spawn" --diff
# 2. register swap? ask which renumbering would give the target's registers, then what those vregs are
python tools/mwcc/vregs.py src/dol/game/d_field_assessment.cpp "^updateFlowersLaterDay$" --only id,weekly,v54
# 2a. a local that should have its own register but doesn't (or a flag that became an expression)? ask the optimizer
python tools/mwcc/iro_why.py src/dol/game/d_field_assessment.cpp "^fgMngTask_findFree$" --no-timeline
# 2b. stack offsets swapped? see the frame layout and why each object is where it is
python tools/mwcc/stackslots.py src/dol/game/d_field_assessment.cpp "^collectEggs$"
python tools/mwcc/regalloc.py src/d_fish_fieldNP/d_fish_field.cpp "^executeNibble$" --want "this=r28,fl=r29"
# 2b. two instructions in the wrong order? see the scheduler's ready lists
python tools/mwcc/sched.py src/d_fish_fieldNP/d_fish_field.cpp "^getMouthPos$" --post-ra-only
# 2c. extra/missing extend or conversion? see the expression tree before and after TOC
python tools/mwcc/enode.py src/d_fish_fieldNP/d_fish_field.cpp "^executeRelease$" --grep addCalcAngle
# 2d. one instruction too many/few, or a pair fused differently? see what the peephole / copy propagation did
python tools/mwcc/peephole.py src/d_fish_fieldNP/d_fish_field.cpp "^executeRelease$" --pass "copy|peep"
# 3. extra/missing load, store, extend? see the IR through the passes
python tools/mwcc/iro_dump.py src/d_fish_fieldNP/d_fish_field.cpp -f "^spawn$" -p "Propagat|CommonSub|DeadStore" -o spawn_iro.txt
# 4. try a variant without touching the real source
python tools/mwcc/score.py src/d_fish_fieldNP/d_fish_field.cpp --input /tmp/variant.cpp -I /tmp/ov -f spawn --diff
# 5. or try a whole list of rewrites (and pairs of them) at once, ranked, with regressions flagged
python tools/mwcc/variants.py src/d_fish_fieldNP/d_fish_field.cpp rewrites.py -f "spawn" --combine 2
```

## Notes
- `score.py` scores in objdiff project mode through a one-unit temp project, so its numbers equal the build's
  report.json (e.g. the fish: 146/146). Functions listed as "not in target" are our unreferenced weak copies,
  which the link strips.
- `iro_dump.py` keeps the patched compiler in `build/mwcc_tools/iro_compiler/`. It checks the original
  bytes before patching and refuses any other compiler version. Logs are large (~95 MB for a big TU) and
  name functions by their plain name, so filter with `-f` / `-p`, or use `--list`. Never build real
  objects with the patched copy.
- `regalloc.py` reports GPRs (class 4). K is 29, and callee-saved registers are claimed from r31
  downwards. The addresses it uses are documented at the top of the script and in
  `notes/mwcc30_functions.csv`.
- To find compiler code, grep `notes/mwcc30_functions.csv` (3.0 address to 1.2.5 name and source file),
  then read the GC 1.2.5 decompilation (github.com/rayanht/mwcc) next to Ghidra's `mwcceppc.exe`.
