"""Rename symbols.txt entries from a compiled unit object, paired by position.

usage: python tools/cleanup/sync_symbols.py <unit name in splits.txt> <object> [--apply]
Only lbl_/fn_ entries are renamed; compiler temporaries (@N) and section symbols are skipped.
"""
import os
import re
import subprocess
import sys

unit, obj = sys.argv[1], sys.argv[2]
apply = "--apply" in sys.argv
SYM = "config/RUUE01_00/symbols.txt"

splits = open("config/RUUE01_00/splits.txt", encoding="utf-8").read()
m = re.search(r"^" + re.escape(unit) + r":\n((?:\t.*\n)+)", splits, re.M)
starts = {s: int(a, 16) for s, a in re.findall(r"\t(\.\w+)\s+start:0x([0-9A-F]+)", m.group(1))}

out = subprocess.run([os.path.abspath("build/binutils/powerpc-eabi-objdump.exe"), "-t", obj], capture_output=True, text=True).stdout
want = {}
for line in out.splitlines():
    mm = re.match(r"([0-9a-f]{8}) (l|g| )\s+(\w*)\s+(\.\w+)\s+([0-9a-f]{8}) (\S+)$", line)
    if not mm:
        continue
    val, bind, kind, sec, size, name = mm.groups()
    if kind not in ("O", "F") or name.startswith("@") or sec not in starts:
        continue
    want[(sec, starts[sec] + int(val, 16))] = (name, bind == "l")

lines = open(SYM, encoding="utf-8").read().split("\n")
changes = 0
for i, l in enumerate(lines):
    mm = re.match(r"(\S+) = (\.\w+):0x([0-9A-F]+); // (.*)", l)
    if not mm:
        continue
    key = (mm.group(2), int(mm.group(3), 16))
    if key in want and mm.group(1) != want[key][0] and re.match(r"(lbl_|fn_)", mm.group(1)):
        name, local = want[key]
        rest = mm.group(4)
        if local and "scope:local" not in rest:
            rest += " scope:local"
        lines[i] = f"{name} = {mm.group(2)}:0x{mm.group(3)}; // {rest}"
        print(f"{mm.group(1):14s} -> {name}{' (local)' if local else ''}")
        changes += 1
if apply:
    open(SYM, "w", encoding="utf-8", newline="\n").write("\n".join(lines))
print(changes, "renames", "applied" if apply else "(dry run)")
