"""Byte-identity check for cleanup passes.

usage: python tools/cleanup/obj_identity.py <before.o> <after.o> [old=new ...]

Compares every allocated section's bytes, the defined-symbol layout (section, offset, size) and every relocation
(offset, type, addend, target). A relocation target that is a defined symbol is compared by (section, offset);
an undefined (external) one by name, after applying the old=new renames given on the command line. Debug sections
(.debug/.line/.comment) are skipped: -sym on puts names and line numbers there.
"""
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

SKIP = (".debug", ".line", ".comment", ".rela", ".symtab", ".strtab", ".shstrtab", ".mwcats")


def load(path):
    f = open(path, "rb")
    elf = ELFFile(f)
    secs = {}
    for i, s in enumerate(elf.iter_sections()):
        if s.name and not s.name.startswith(SKIP) and s["sh_type"] != "SHT_NULL":
            data = b"" if s["sh_type"] == "SHT_NOBITS" else s.data()
            secs[s.name] = (data, s["sh_size"])
    symtab = elf.get_section_by_name(".symtab")
    syms = list(symtab.iter_symbols())
    defined = {}
    for s in syms:
        idx = s["st_shndx"]
        if isinstance(idx, int) and s["st_info"]["type"] in ("STT_OBJECT", "STT_FUNC"):
            sec = elf.get_section(idx).name
            if not sec.startswith(SKIP):
                defined[(sec, s["st_value"])] = (s.name, s["st_size"])
    relocs = {}
    for rs in elf.iter_sections():
        if not isinstance(rs, RelocationSection):
            continue
        target = elf.get_section(rs["sh_info"]).name
        if target.startswith(SKIP):
            continue
        out = []
        for r in rs.iter_relocations():
            sym = syms[r["r_info_sym"]]
            idx = sym["st_shndx"]
            if isinstance(idx, int):
                tgt = ("def", elf.get_section(idx).name, sym["st_value"])
            else:
                tgt = ("ext", sym.name)
            out.append((r["r_offset"], r["r_info_type"], r.entry.get("r_addend", 0), tgt))
        relocs[target] = sorted(out)
    return secs, defined, relocs


def main():
    before, after = sys.argv[1], sys.argv[2]
    ren = dict(a.split("=", 1) for a in sys.argv[3:])
    s1, d1, r1 = load(before)
    s2, d2, r2 = load(after)
    ok = True
    for name in sorted(set(s1) | set(s2)):
        if name not in s1 or name not in s2:
            print(f"section {name}: only in {'before' if name in s1 else 'after'}")
            ok = False
        elif s1[name] != s2[name]:
            a, b = s1[name][0], s2[name][0]
            first = next((i for i in range(min(len(a), len(b))) if a[i] != b[i]), min(len(a), len(b)))
            print(f"section {name}: differs (sizes {s1[name][1]:#x}/{s2[name][1]:#x}, first diff at {first:#x})")
            ok = False
    lay1 = {k: v[1] for k, v in d1.items()}
    lay2 = {k: v[1] for k, v in d2.items()}
    if lay1 != lay2:
        for k in sorted(set(lay1) | set(lay2)):
            if lay1.get(k) != lay2.get(k):
                print(f"symbol layout {k}: {d1.get(k)} vs {d2.get(k)}")
        ok = False
    for name in sorted(set(r1) | set(r2)):
        a = [(o, t, ad, ("ext", ren.get(tg[1], tg[1])) if tg[0] == "ext" else tg) for o, t, ad, tg in r1.get(name, [])]
        b = r2.get(name, [])
        if a != b:
            ok = False
            print(f"relocs {name}: {len(a)} vs {len(b)}")
            for x, y in zip(a, b):
                if x != y:
                    print(f"   {x} != {y}")
                    break
    renamed = sorted((d1[k][0], d2[k][0]) for k in d1 if k in d2 and d1[k][0] != d2[k][0])
    for o, n in renamed:
        print(f"renamed {o} -> {n}")
    print("IDENTICAL" if ok else "DIFFERENT")
    sys.exit(0 if ok else 1)


main()
