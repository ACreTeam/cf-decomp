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
| `mwcc_cmd.py` | Shared helper: the exact ninja compile command for any source. Run it directly to print the command. |
| `dbgcore.py` | Shared helper: a tiny Win32 debugger. Give it the compile argv and `{address: handler}`, and your handler runs every time the compiler executes that address. Build new introspection tools on it. |

## Typical loop

```sh
# 1. how far off is it, and where?
python tools/mwcc/score.py src/d_fish_fieldNP/d_fish_field.cpp -f "spawn" --diff
# 2. register swap? ask which renumbering would give the target's registers
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
