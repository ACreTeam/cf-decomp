#!/usr/bin/env python3
"""Port the RUUE01_00 dtk configuration to another City Folk version.

Usage: python tools/port_version.py RUUP01_00 [--base RUUE01_00]

Needs a fresh dtk analysis of both versions (written to --work, default
../port/an/<version>) so relocations can be paired between the binaries.
"""

from __future__ import annotations

import argparse
import bisect
import collections
import hashlib
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

ROOT = Path(__file__).resolve().parent.parent
DTK = ROOT / "build" / "tools" / "dtk.exe"

R_PPC_ADDR32 = 1
R_PPC_ADDR16_LO = 4
R_PPC_ADDR16_HI = 5
R_PPC_ADDR16_HA = 6
R_PPC_REL24 = 10
R_PPC_REL14 = 11
R_PPC_EMB_SDA21 = 109
R_DOLPHIN_NOP = 201
R_DOLPHIN_SECTION = 202
R_DOLPHIN_END = 203

MASKS = {
    R_PPC_ADDR32: 0xFFFFFFFF,
    R_PPC_REL24: 0x03FFFFFC,
    R_PPC_REL14: 0x0000FFFC,
    R_PPC_EMB_SDA21: 0x001FFFFF,
}

SYM_RE = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+);(?: // (.*))?$")
SPLIT_RE = re.compile(r"^\t(\S+)\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)(.*)$")


class Space:
    """One section of one binary: the DOL (absolute addresses) or a REL (offsets)."""

    def __init__(self, unit, name, index, start, size, data, code):
        self.unit = unit
        self.name = name
        self.index = index
        self.start = start
        self.size = size
        self.data = data
        self.code = code
        self.masks = {}
        self.blocks = []

    @property
    def end(self):
        return self.start + self.size

    def contains(self, addr):
        return self.start <= addr < self.end


class Binary:
    def __init__(self, version, work):
        self.version = version
        self.work = work
        self.spaces = {}
        self.relocs = []
        self.module_ids = {}

    def space_at(self, unit, addr, name=None):
        best = None
        for sp in self.spaces.get(unit, []):
            if sp.start <= addr < sp.end or (addr == sp.end and best is None):
                if name is None or sp.name == name:
                    if sp.contains(addr):
                        return sp
                    best = sp
        return best

    def space_named(self, unit, name):
        for sp in self.spaces.get(unit, []):
            if sp.name == name:
                return sp
        return None


def read_symbols(path):
    out = []
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        m = SYM_RE.match(line)
        if not m:
            continue
        attrs = {}
        flags = []
        for tok in (m.group(4) or "").split():
            if ":" in tok:
                k, v = tok.split(":", 1)
                attrs[k] = v
            else:
                flags.append(tok)
        out.append({"name": m.group(1), "section": m.group(2), "addr": int(m.group(3), 16),
                    "attrs": attrs, "flags": flags, "comment": m.group(4) or "", "line": line})
    return out


def read_splits(path):
    header = []
    units = []
    cur = None
    in_sections = False
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        if line == "Sections:":
            in_sections = True
            header.append(line)
            continue
        if line and not line.startswith("\t"):
            in_sections = False
            cur = {"header": line, "entries": []}
            units.append(cur)
            continue
        if in_sections:
            header.append(line)
            continue
        if cur is None:
            if line:
                header.append(line)
            continue
        m = SPLIT_RE.match(line)
        if m:
            cur["entries"].append({"section": m.group(1), "start": int(m.group(2), 16),
                                   "end": int(m.group(3), 16), "rest": m.group(4)})
        elif line.strip():
            cur["entries"].append({"raw": line})
    return header, units


def section_names(header):
    names = []
    for line in header[1:]:
        parts = line.split()
        if parts:
            names.append(parts[0])
    return names


def load_dol(binary, root):
    dol = (root / "sys" / "main.dol").read_bytes()
    offs = struct.unpack(">18I", dol[0:72])
    addrs = struct.unpack(">18I", dol[72:144])
    info = subprocess.run([str(DTK), "dol", "info", str(root / "sys" / "main.dol")],
                          capture_output=True, text=True, check=True).stdout
    spaces = []
    in_sections = False
    for line in info.splitlines():
        if line.strip() == "Sections:":
            in_sections = True
            continue
        if in_sections:
            parts = [p.strip() for p in line.split("|")]
            if len(parts) != 4 or parts[0] == "Name":
                if not line.strip():
                    break
                continue
            name, addr, size, _ = parts
            addr = int(addr, 16)
            size = int(size, 16)
            if name == ".bss2":
                name = ".sbss2"
            data = None
            for o, a in zip(offs, addrs):
                if a == addr and o:
                    data = dol[o:o + size]
            code = name in (".init", ".text")
            spaces.append(Space("dol", name, len(spaces), addr, size, data, code))
    binary.spaces["dol"] = spaces


def load_rel(binary, unit, path, names):
    rel = path.read_bytes()
    mid, _, _, nsec, secoff = struct.unpack(">IIIII", rel[0:20])
    binary.module_ids[unit] = mid
    binary.module_ids[mid] = unit
    secs = []
    nonempty = []
    for i in range(nsec):
        off, size = struct.unpack(">II", rel[secoff + i * 8:secoff + i * 8 + 8])
        secs.append((off, size))
        if size:
            nonempty.append(i)
    if len(nonempty) != len(names):
        raise SystemExit(f"{unit}: {len(nonempty)} sections but {len(names)} names")
    spaces = []
    for i, name in zip(nonempty, names):
        off, size = secs[i]
        data = rel[off & ~3:(off & ~3) + size] if off else None
        spaces.append(Space(unit, name, i, 0, size, data, bool(off & 1)))
    binary.spaces[unit] = spaces
    relocs = []
    reloff, impoff, impsize = struct.unpack(">III", rel[0x24:0x30])
    for j in range(impsize // 8):
        imp, offset = struct.unpack(">II", rel[impoff + j * 8:impoff + j * 8 + 8])
        pos = offset
        cur_sec = None
        cur = 0
        while True:
            delta, typ, sec, addend = struct.unpack(">HBBI", rel[pos:pos + 8])
            pos += 8
            if typ == R_DOLPHIN_END:
                break
            if typ == R_DOLPHIN_SECTION:
                cur_sec = sec
                cur = 0
                continue
            cur += delta
            if typ == R_DOLPHIN_NOP:
                continue
            relocs.append((cur_sec, cur, typ, imp, sec, addend))
    binary.raw_rel_relocs = getattr(binary, "raw_rel_relocs", {})
    binary.raw_rel_relocs[unit] = relocs


def finish_rel_relocs(binary):
    for unit, relocs in binary.raw_rel_relocs.items():
        by_index = {sp.index: sp for sp in binary.spaces[unit]}
        for cur_sec, off, typ, imp, sec, addend in relocs:
            src = by_index.get(cur_sec)
            if src is None:
                continue
            if imp == 0:
                tsp = binary.space_at("dol", addend)
                if tsp is None:
                    continue
                binary.relocs.append((src, off, typ, tsp, addend))
            else:
                tunit = binary.module_ids.get(imp)
                if tunit is None:
                    continue
                tsp = {sp.index: sp for sp in binary.spaces[tunit]}.get(sec)
                if tsp is None:
                    continue
                binary.relocs.append((src, off, typ, tsp, addend))


def load_dol_relocs(binary):
    work = binary.work
    symbols = read_symbols(work / "symbols.txt")
    counts = collections.Counter(s["name"] for s in symbols)
    addr_of = {s["name"]: s["addr"] for s in symbols if counts[s["name"]] == 1}
    suffix = re.compile(r"_([0-9A-F]{8})$")

    def resolve(name):
        if name in addr_of:
            return addr_of[name]
        m = suffix.search(name)
        return int(m.group(1), 16) if m else None
    for obj in sorted((work / "out" / "obj").glob("*.o")):
        with open(obj, "rb") as f:
            elf = ELFFile(f)
            symtab = elf.get_section_by_name(".symtab")
            syms = list(symtab.iter_symbols())
            bases = {}
            for s in syms:
                idx = s["st_shndx"]
                addr = resolve(s.name) if isinstance(idx, int) and idx not in bases else None
                if addr is not None:
                    bases[idx] = addr - s["st_value"]
            for sec in elf.iter_sections():
                if not isinstance(sec, RelocationSection):
                    continue
                target_idx = sec["sh_info"]
                if target_idx not in bases:
                    continue
                base = bases[target_idx]
                for r in sec.iter_relocations():
                    s = syms[r["r_info_sym"]]
                    idx = s["st_shndx"]
                    if isinstance(idx, int) and idx in bases:
                        tgt = bases[idx] + s["st_value"]
                    else:
                        tgt = resolve(s.name)
                        if tgt is None:
                            continue
                    tgt += r["r_addend"]
                    src = base + r["r_offset"]
                    ssp = binary.space_at("dol", src)
                    tsp = binary.space_at("dol", tgt)
                    if ssp is None or tsp is None:
                        continue
                    binary.relocs.append((ssp, src, r["r_info_type"], tsp, tgt))


def load_binary(version, base_config_names, work):
    root = ROOT / "orig" / version
    b = Binary(version, work)
    load_dol(b, root)
    for unit, names in base_config_names.items():
        load_rel(b, unit, root / "files" / "rels" / f"{unit}.rel", names)
    finish_rel_relocs(b)
    load_dol_relocs(b)
    for sp_list in b.spaces.values():
        for sp in sp_list:
            sp.masks = {}
            sp.tags = {}
    fresh = read_symbols(work / "symbols.txt")
    starts = sorted({s["addr"] for s in fresh if "size" in s["attrs"]})
    b.funcs = {}
    for unit, path in [("dol", work / "symbols.txt")] + [(u, work / "m" / u / "symbols.txt") for u in base_config_names]:
        syms = fresh if unit == "dol" else read_symbols(path)
        b.funcs[unit] = {s["addr"]: int(s["attrs"].get("size", "0"), 16) for s in syms
                         if s["attrs"].get("type") == "function"}
        b.jumptables = getattr(b, "jumptables", {})
        b.jumptables[unit] = {s["addr"] for s in syms if s["name"].startswith("jumptable_")}
    for ssp, off, typ, tsp, tgt in b.relocs:
        word = off & ~3
        m = MASKS.get(typ, 0xFFFF)
        if typ in (R_PPC_ADDR16_LO, R_PPC_ADDR16_HI, R_PPC_ADDR16_HA) and (off & 3) == 0:
            m = 0xFFFF0000
        ssp.masks[word] = ssp.masks.get(word, 0) | m
        if ssp.code:
            continue
        if tsp.unit == "dol":
            i = bisect.bisect_right(starts, tgt) - 1
            ssp.tags[word] = f"{tsp.name}+{tgt - starts[i]:X}" if i >= 0 else tsp.name
        else:
            ssp.tags[word] = tsp.name
    return b


def tokens(sp):
    if sp.data is None:
        return []
    out = []
    n = len(sp.data) // 4
    words = struct.unpack(f">{n}I", sp.data[:n * 4])
    for i, w in enumerate(words):
        a = sp.start + i * 4
        m = sp.masks.get(a)
        if m:
            w &= ~m & 0xFFFFFFFF
            out.append(f"{w:08X}R{m:08X}{sp.tags.get(a, '')}")
        else:
            out.append(f"{w:08X}")
    return out


HUNK_RE = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")


def diff_blocks(a, b):
    """Equal runs between token lists a and b: list of (ai, bi, length)."""
    if a == b:
        return [(0, 0, len(a))]
    with tempfile.TemporaryDirectory() as tmp:
        pa = Path(tmp) / "a"
        pb = Path(tmp) / "b"
        pa.write_text("\n".join(a) + "\n")
        pb.write_text("\n".join(b) + "\n")
        res = subprocess.run(["git", "diff", "--no-index", "--histogram", "-U0", "--no-color", str(pa), str(pb)],
                             capture_output=True, text=True)
    hunks = []
    for line in res.stdout.splitlines():
        m = HUNK_RE.match(line)
        if m:
            s1 = int(m.group(1))
            n1 = int(m.group(2)) if m.group(2) is not None else 1
            s2 = int(m.group(3))
            n2 = int(m.group(4)) if m.group(4) is not None else 1
            a0 = s1 - 1 if n1 else s1
            b0 = s2 - 1 if n2 else s2
            hunks.append((a0, n1, b0, n2))
    blocks = []
    ai = bi = 0
    for a0, n1, b0, n2 in hunks:
        if a0 > ai:
            blocks.append((ai, bi, a0 - ai))
        ai = a0 + n1
        bi = b0 + n2
    if ai < len(a):
        blocks.append((ai, bi, len(a) - ai))
    return blocks


class Mapper:
    def __init__(self, base, target):
        self.base = base
        self.target = target
        self.pairs = {}
        self.blocks = {}
        self.anchors = collections.defaultdict(lambda: collections.defaultdict(collections.Counter))
        self.constants = set()

    def tspace(self, sp):
        return self.target.space_named(sp.unit, sp.name) if sp.unit != "dol" else \
            next((t for t in self.target.spaces["dol"] if t.name == sp.name), None)

    def build(self):
        for unit, spaces in self.base.spaces.items():
            for sp in spaces:
                tsp = self.tspace(sp)
                if tsp is None:
                    raise SystemExit(f"{self.target.version}: no section {unit} {sp.name}")
                self.pairs[id(sp)] = tsp
                if sp.data is not None and tsp.data is not None:
                    blocks = diff_blocks(tokens(sp), tokens(tsp))
                    minimum = 6 if sp.code else 3
                    self.blocks[id(sp)] = [(sp.start + a * 4, tsp.start + b * 4, n * 4) for a, b, n in blocks
                                           if n >= minimum or len(blocks) == 1]
                else:
                    self.blocks[id(sp)] = []
        tindex = collections.defaultdict(list)
        for ssp, off, typ, tsp, tgt in self.target.relocs:
            tindex[(id(ssp), off)].append((typ, tsp, tgt))
        same = []
        for ssp, off, typ, tsp, tgt in self.base.relocs:
            mo = self.map_exact(ssp, off)
            if mo is None:
                continue
            tssp = self.pairs[id(ssp)]
            for ttyp, ttsp, ttgt in tindex.get((id(tssp), mo), []):
                if ttyp == typ and self.pairs.get(id(tsp)) is ttsp:
                    if typ == R_PPC_ADDR32 and ttgt == tgt and tsp.unit == "dol" and not ssp.code:
                        same.append((ssp, tssp, mo, tsp, tgt))
                        continue
                    self.anchors[id(tsp)][tgt][ttgt - tgt] += 100 if ssp.code else 1
        strong = collections.defaultdict(list)
        for key, by_addr in self.anchors.items():
            for addr, votes in by_addr.items():
                d, w = votes.most_common(1)[0]
                if w >= 100:
                    strong[key].append((addr, d))
        for key in strong:
            strong[key].sort()
        for ssp, tssp, mo, tsp, tgt in same:
            moved = self.map_exact(tsp, tgt)
            if moved is None:
                near = strong.get(id(tsp), [])
                i = bisect.bisect_right(near, (tgt, 1 << 40))
                cands = [near[k] for k in (i - 1, i) if 0 <= k < len(near)]
                if cands:
                    addr, d = min(cands, key=lambda x: abs(x[0] - tgt))
                    moved = tgt + d
                else:
                    moved = self.nearest_delta(tsp, tgt)
            if moved is not None and moved != tgt:
                if tssp.unit == "dol":
                    self.constants.add(mo)
                continue
            self.anchors[id(tsp)][tgt][0] += 1

    def nearest_delta(self, sp, addr):
        blocks = self.blocks.get(id(sp))
        if not blocks:
            return None
        i = bisect.bisect_right(blocks, (addr, 0xFFFFFFFFFFFF, 0)) - 1
        if i < 0:
            return None
        a, b, n = blocks[i]
        return addr + (b - a)

    def map_exact(self, sp, addr):
        blocks = self.blocks.get(id(sp))
        if not blocks:
            return None
        i = bisect.bisect_right(blocks, (addr, 0xFFFFFFFFFFFF, 0)) - 1
        if i < 0:
            return None
        a, b, n = blocks[i]
        if a <= addr < a + n:
            return b + (addr - a)
        return None

    def block_len_at(self, sp, addr):
        blocks = self.blocks.get(id(sp))
        if not blocks:
            return 0
        i = bisect.bisect_right(blocks, (addr, 0xFFFFFFFFFFFF, 0)) - 1
        if i < 0:
            return 0
        a, b, n = blocks[i]
        if a <= addr < a + n:
            return a + n - addr
        return 0

    def anchor_delta(self, sp, start, end):
        anchors = self.anchors.get(id(sp))
        if not anchors:
            return None, None
        exact = anchors.get(start)
        if exact:
            d, w = exact.most_common(1)[0]
            return d, "exact" if w >= 100 else "exactweak"
        votes = collections.Counter()
        for addr in self.anchor_keys(sp, start, end):
            for d, c in anchors[addr].items():
                votes[d] += c
        if votes:
            d, w = votes.most_common(1)[0]
            return d, "inner" if w >= 100 else "weak"
        return None, None

    def anchor_keys(self, sp, start, end):
        keys = getattr(sp, "_anchor_keys", None)
        if keys is None:
            keys = sorted(self.anchors.get(id(sp), {}).keys())
            sp._anchor_keys = keys
        lo = bisect.bisect_left(keys, start)
        hi = bisect.bisect_left(keys, end)
        return keys[lo:hi]


def port_symbols(mapper, base_bin, unit, symbols, report):
    """Map each symbol of one unit; returns list of (sym, new_addr, new_size)."""
    by_space = collections.defaultdict(list)
    for s in symbols:
        sp = base_bin.space_named(unit, s["section"]) if unit != "dol" else base_bin.space_at("dol", s["addr"])
        if unit == "dol" and sp is not None and not (sp.start <= s["addr"] <= sp.end):
            sp = None
        if sp is None:
            raise SystemExit(f"{unit}: no section for {s['line']}")
        by_space[id(sp)].append((sp, s))
    results = {}
    for key, items in by_space.items():
        sp = items[0][0]
        tsp = mapper.pairs[id(sp)]
        items.sort(key=lambda x: x[1]["addr"])
        starts = sorted({s["addr"] for _, s in items})
        mapped = {}
        how = {}
        prev = None
        for i, a in enumerate(starts):
            nxt = starts[i + 1] if i + 1 < len(starts) else sp.end
            d_anchor, kind = mapper.anchor_delta(sp, a, max(nxt, a + 1))
            m = mapper.map_exact(sp, a)
            if kind == "exact" or (kind == "exactweak" and (m is None or not sp.code)):
                mapped[a] = a + d_anchor
                how[a] = "anchor"
            elif m is not None and (d_anchor is None or m - a == d_anchor or sp.code):
                mapped[a] = m
                how[a] = "diff"
            elif d_anchor is not None and (kind in ("inner", "exactweak") or prev is None):
                mapped[a] = a + d_anchor
                how[a] = kind
            elif prev is not None:
                mapped[a] = a + (mapped[prev] - prev)
                how[a] = "interp"
            else:
                mapped[a] = tsp.start + (a - sp.start)
                how[a] = "interp"
            prev = a
        dropped = set()
        if sp.code:
            fix_code(mapper, unit, sp, tsp, starts, mapped, how, report, dropped)
        soft = set()
        for _, s in items:
            a = s["addr"]
            na = mapped[a]
            if a in dropped:
                continue
            if a in mapper.base.jumptables[unit] and na not in mapper.target.jumptables[unit] and any(
                    abs(t - na) < 0x40 for t in mapper.target.jumptables[unit]):
                dropped.add(a)
                soft.add(a)
                report[how_key(unit)]["jumptable-dropped"] += 1
                report["dropnames"].append(f"{unit} {s['name']} -> {na:X}")
                continue
            size = int(s["attrs"].get("size", "0"), 16)
            if s["attrs"].get("data") == "string" and size and sp.data is not None and tsp.data is not None:
                raw = sp.data[a - sp.start:a - sp.start + size]
                base_str = raw.split(bytes(1))[0] if bytes(1) in raw else b""
                got = tsp.data[na - tsp.start:na - tsp.start + len(base_str) + 1]
                if got != base_str + bytes(1) and len(base_str) >= 4 and s["name"].startswith(("lbl_", "@")):
                    found = find_near(tsp, base_str + bytes(1), na, a & 3)
                    if found is not None:
                        mapped[a] = found
                        how[a] = "string-moved"
                    elif s["name"].startswith("lbl_"):
                        dropped.add(a)
                        soft.add(a)
                        report[how_key(unit)]["string-dropped"] += 1
                        report["dropnames"].append(f"{unit} {s['name']} {base_str[:40]!r} -> {got[:40]!r}")
        report[how_key(unit)].update(how.values())
        report[how_key(unit)]["dropped"] += len(dropped)
        live = [a for a in starts if a not in dropped]
        for i, a in enumerate(live):
            if i + 1 < len(live) and mapped[live[i + 1]] < mapped[a]:
                report["order"].append(f"{unit} {sp.name} {a:X}->{mapped[a]:X} after next {live[i+1]:X}->{mapped[live[i+1]]:X}")
        sized = sorted({s["addr"] for _, s in items if "size" in s["attrs"] and s["addr"] not in dropped})
        tsorted = sorted({mapped[a] for a in sized})
        for sp_, s in items:
            a = s["addr"]
            if a in dropped:
                results[id(s)] = None
                continue
            na = mapped[a]
            k = bisect.bisect_right(tsorted, na)
            tnext = tsorted[k] if k < len(tsorted) else tsp.end
            size = s["attrs"].get("size")
            nsize = None
            if size is not None:
                size = int(size, 16)
                end = a + size
                j = bisect.bisect_right(sized, a)
                nxt = sized[j] if j < len(sized) else None
                bfresh = mapper.base.funcs[unit]
                tfresh = mapper.target.funcs[unit]
                if mapper.block_len_at(sp, a) >= size and mapper.map_exact(sp, a) == na:
                    nsize = size
                elif sp.code and bfresh.get(a) == size and na in tfresh:
                    nsize = tfresh[na]
                elif nxt is not None and end == nxt:
                    nsize = tnext - na
                elif nxt is None and end == sp.end:
                    nsize = tnext - na
                elif nxt is not None and end < nxt:
                    gap = nxt - end
                    nsize = size if na + size + gap <= tnext else tnext - gap - na
                else:
                    nsize = size
                if nsize <= 0:
                    report["badsize"].append(f"{unit} {s['name']} {a:X} size {size:X} -> {nsize}")
                    nsize = size
                kind = s["attrs"].get("data")
                if kind in ("string", "wstring") and tsp.data is not None and not (
                        mapper.block_len_at(sp, a) >= size and mapper.map_exact(sp, a) == na):
                    nsize = string_size(tsp, na, kind, tnext) or nsize
                if na + nsize > tnext and tnext > na:
                    nsize = tnext - na
            results[id(s)] = (na, nsize)
        ends = sorted((r[0], r[0] + r[1]) for r in (results.get(id(x)) for _, x in items) if r and r[1])
        mapper.__dict__.setdefault("target_ends", {})[(unit, sp.name)] = ends
        mapper.symbol_map = getattr(mapper, "symbol_map", {})
        live = [a for a in starts if a not in dropped or a in soft]
        mapper.symbol_map[(unit, sp.name)] = (sp, tsp, live, {a: mapped[a] for a in live}, dropped)
    return results


def find_near(tsp, needle, addr, align, window=0x2000):
    lo = max(0, addr - tsp.start - window)
    hi = min(len(tsp.data), addr - tsp.start + window)
    best = None
    i = tsp.data.find(needle, lo, hi)
    while i >= 0:
        if (tsp.start + i) & 3 == align and (i == 0 or tsp.data[i - 1] == 0):
            if best is None or abs(tsp.start + i - addr) < abs(best - addr):
                best = tsp.start + i
        i = tsp.data.find(needle, i + 1, hi)
    return best


def string_size(tsp, addr, kind, limit):
    off = addr - tsp.start
    end = min(limit, tsp.end) - tsp.start
    if kind == "string":
        i = tsp.data.find(bytes(1), off, end)
        return i + 1 - off if i >= 0 else None
    for i in range(off, end - 1, 2):
        if tsp.data[i] == 0 and tsp.data[i + 1] == 0:
            return i + 2 - off
    return None


def fix_code(mapper, unit, sp, tsp, starts, mapped, how, report, dropped):
    bfresh = mapper.base.funcs[unit]
    tfresh = mapper.target.funcs[unit]
    tstarts = sorted(a for a in tfresh if tsp.start <= a < tsp.end)
    visible = [a for a in starts if a in bfresh]
    score = {}
    for a in visible:
        run = mapper.block_len_at(sp, a) if mapper.map_exact(sp, a) == mapped[a] else 0
        if mapped[a] not in tfresh and not (run >= max(bfresh[a], 0x20)):
            continue
        if how[a] == "anchor" or run >= min(bfresh[a] or 4, 0x20):
            score[a] = run + (0x100000 if how[a] == "anchor" else 0)
    best = {}
    for a, sc in score.items():
        t = mapped[a]
        if t not in best or sc > score[best[t]]:
            best[t] = a
    cand = sorted(best.values())
    tails = []
    prevs = {}
    for a in cand:
        k = bisect.bisect_left([mapped[x] for x in tails], mapped[a])
        prevs[a] = tails[k - 1] if k else None
        if k == len(tails):
            tails.append(a)
        else:
            tails[k] = a
    chain = []
    a = tails[-1] if tails else None
    while a is not None:
        chain.append(a)
        a = prevs[a]
    valid = set(chain)
    i = 0
    while i < len(visible):
        if visible[i] in valid:
            i += 1
            continue
        j = i
        while j < len(visible) and visible[j] not in valid:
            j += 1
        lo = mapped[visible[i - 1]] if i > 0 else tsp.start - 1
        hi = mapped[visible[j]] if j < len(visible) else tsp.end
        run = visible[i:j]
        cands = tstarts[bisect.bisect_right(tstarts, lo):bisect.bisect_left(tstarts, hi)]
        if len(cands) == len(run):
            for a, t in zip(run, cands):
                if mapped[a] != t:
                    mapped[a] = t
                    how[a] = "fnorder"
                valid.add(a)
        else:
            pairs = align_sizes([bfresh[a] for a in run], [tfresh[t] for t in cands])
            paired = set()
            for x, y in pairs:
                a, t = run[x], cands[y]
                if mapped[a] != t:
                    mapped[a] = t
                    how[a] = "fnsize"
                valid.add(a)
                paired.add(a)
            for a in run:
                if a not in paired:
                    dropped.add(a)
            report["unpaired"].append(f"{unit} {sp.name} {run[0]:X}..{run[-1]:X}: {len(run)} base vs {len(cands)} target functions, {len(run) - len(paired)} dropped")
        i = j
    owner = {mapped[a]: a for a in visible if a in valid and a not in dropped}
    for k in range(len(visible) - 1):
        prev, succ = visible[k], visible[k + 1]
        if prev not in valid or prev in dropped or succ not in dropped:
            continue
        tprev = mapped[prev]
        if tprev not in tfresh:
            continue
        t = tprev + tfresh[tprev]
        claim = owner.get(t)
        if claim is None or claim == succ or bfresh.get(claim) != bfresh.get(succ):
            continue
        if bytes_of(sp, claim, bfresh[claim]) != bytes_of(sp, succ, bfresh[succ]):
            continue
        mapped[succ] = t
        how[succ] = "fnorder"
        dropped.discard(succ)
        valid.add(succ)
        dropped.add(claim)
        valid.discard(claim)
        owner[t] = succ
    for a in starts:
        if a in bfresh or a in dropped:
            continue
        k = bisect.bisect_right(visible, a) - 1
        if k >= 0 and visible[k] in dropped and a < visible[k] + max(bfresh[visible[k]], 1):
            dropped.add(a)
    keep = sorted(valid)
    for a in starts:
        if a in valid or a in dropped:
            continue
        k = bisect.bisect_left(keep, a)
        lo = keep[k - 1] if k else None
        hi = keep[k] if k < len(keep) else None
        lo_t = mapped[lo] if lo is not None else tsp.start
        hi_t = mapped[hi] if hi is not None else tsp.end
        if lo_t <= mapped[a] < hi_t or (mapped[a] == hi_t and a == hi):
            continue
        mapped[a] = a + (mapped[lo] - lo) if lo is not None else tsp.start + (a - sp.start)
        how[a] = "interp"
    last = None
    for a in starts:
        if a in dropped:
            continue
        if last is not None and mapped[a] < mapped[last]:
            mapped[a] = mapped[last] + (a - last)
            how[a] = "interp"
        last = a


def bytes_of(sp, addr, size):
    if sp.data is None:
        return None
    out = []
    for a in range(addr, addr + size, 4):
        w = struct.unpack(">I", sp.data[a - sp.start:a - sp.start + 4])[0]
        out.append(w & ~sp.masks.get(a, 0) & 0xFFFFFFFF)
    return tuple(out)


def align_sizes(xs, ys):
    n, m = len(xs), len(ys)
    score = [[0] * (m + 1) for _ in range(n + 1)]
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            hit = score[i + 1][j + 1] + (3 if xs[i] == ys[j] else 1)
            score[i][j] = max(hit, score[i + 1][j], score[i][j + 1])
    pairs = []
    i = j = 0
    while i < n and j < m:
        if score[i][j] == score[i + 1][j + 1] + (3 if xs[i] == ys[j] else 1):
            pairs.append((i, j))
            i += 1
            j += 1
        elif score[i][j] == score[i + 1][j]:
            i += 1
        else:
            j += 1
    return pairs


def how_key(unit):
    return "dol" if unit == "dol" else "rel"


def map_boundary(mapper, base_bin, unit, section, addr, is_end):
    if unit == "dol":
        sp = None
        for cand in base_bin.spaces["dol"]:
            if cand.name == section:
                sp = cand
    else:
        sp = base_bin.space_named(unit, section)
    tsp = mapper.pairs[id(sp)]
    if addr == sp.end:
        return tsp.end
    if addr == sp.start:
        return tsp.start
    _, _, starts, mapped, dropped = mapper.symbol_map[(unit, sp.name)]
    if addr in mapped:
        return mapped[addr]
    if addr in dropped:
        i = bisect.bisect_left(starts, addr)
        return mapped[starts[i]] if i < len(starts) else tsp.end
    m = mapper.map_exact(sp, addr)
    if m is not None:
        return m
    i = bisect.bisect_right(starts, addr) - 1
    if i >= 0:
        return addr + (mapped[starts[i]] - starts[i])
    return tsp.start + (addr - sp.start)


def format_symbol(s, addr, size):
    attrs = []
    for tok in s["comment"].split():
        if tok.startswith("size:") and size is not None:
            attrs.append(f"size:0x{size:X}")
        else:
            attrs.append(tok)
    return f"{s['name']} = {s['section']}:0x{addr:08X}; // {' '.join(attrs)}"


AUTO_NAME = re.compile(r"^(fn|lbl|jumptable|gap|switch|auto|@etb|@eti|@stringBase)_|^@\d+$")


def read_overrides(path):
    deletes = set()
    entries = []
    if path.is_file():
        for line in path.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if line.startswith("-"):
                deletes.add(line[1:].strip())
            elif line:
                entries.extend(read_symbols_text(line))
    return deletes, entries


def read_symbols_text(text):
    out = []
    for line in text.splitlines():
        m = SYM_RE.match(line)
        if m:
            out.append({"name": m.group(1), "section": m.group(2), "addr": int(m.group(3), 16), "line": line})
    return out


def write_symbols(path, symbols, results):
    deletes, entries = read_overrides(path.parent / "port_overrides.txt")
    replaced = {e["name"] for e in entries} | deletes
    taken = {(e["section"], e["addr"]) for e in entries}
    lines = []
    names = {s["name"] for s in symbols} | replaced
    starts = set(taken)
    for s in symbols:
        if results[id(s)] is None or s["name"] in replaced:
            continue
        addr, size = results[id(s)]
        if (s["section"], addr) in taken and "size" in s["attrs"]:
            continue
        lines.append(format_symbol(s, addr, size))
        starts.add((s["section"], addr))
    if path.is_file():
        for s in read_symbols(path):
            if s["name"] in names or AUTO_NAME.search(s["name"]) or (s["section"], s["addr"]) in starts:
                continue
            lines.append(s["line"])
    lines.extend(e["line"] for e in entries)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def object_align(mapper, header, section):
    cache = mapper.__dict__.setdefault("object_aligns", {})
    name = header.rstrip(":")
    if name not in cache:
        aligns = {}
        path = ROOT / "build" / mapper.base.version / "src" / (str(Path(name).with_suffix("")) + ".o")
        if path.is_file():
            with open(path, "rb") as f:
                for sec in ELFFile(f).iter_sections():
                    aligns[sec.name] = sec["sh_addralign"]
        cache[name] = aligns
    return min(cache[name].get(section, 0), 256)


def gap_align(mapper, unit, section, start):
    ends = mapper.__dict__.get("target_ends", {}).get((unit, section))
    if not ends:
        return 0
    i = bisect.bisect_left(ends, (start, 0)) - 1
    if i < 0:
        return 0
    prev_end = max(e for _, e in ends[max(0, i - 8):i + 1])
    if prev_end >= start:
        return 0
    p = 4
    while p <= 64:
        if (prev_end + p - 1) & ~(p - 1) == start:
            return p
        p *= 2
    return 0


def write_splits(path, mapper, base_bin, unit, header, units):
    out = list(header)
    if unit == "dol":
        out = [re.sub(r"^(\t\.ctors\s+type:rodata) align:4$", r"\1 align:32", h) for h in out]
        out = [re.sub(r"^(\t\.rodata\s+type:rodata) align:32$", r"\1 align:64", h) for h in out]
    out.append("")
    ends = collections.defaultdict(set)
    for u in units:
        for e in u["entries"]:
            if "raw" not in e:
                ends[e["section"]].add(map_boundary(mapper, base_bin, unit, e["section"], e["end"], True))
    for u in units:
        out.append(u["header"])
        for e in u["entries"]:
            if "raw" in e:
                out.append(e["raw"])
                continue
            s = map_boundary(mapper, base_bin, unit, e["section"], e["start"], False)
            t = map_boundary(mapper, base_bin, unit, e["section"], e["end"], True)
            rest = e["rest"]
            m = re.search(r"\balign:(\d+)", rest)
            cur = int(m.group(1)) if m else 4
            want = max(object_align(mapper, u["header"], e["section"]), gap_align(mapper, unit, e["section"], s))
            if want > cur and s % want == 0 and s in ends[e["section"]]:
                rest = re.sub(r"\balign:\d+", f"align:{want}", rest) if m else rest + f" align:{want}"
            out.append(f"\t{e['section']:<11} start:0x{s:08X} end:0x{t:08X}{rest}")
        out.append("")
    path.write_text("\n".join(out), encoding="utf-8")


def sha1(path):
    return hashlib.sha1(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("version")
    ap.add_argument("--base", default="RUUE01_00")
    ap.add_argument("--work", type=Path, default=ROOT.parent / "port" / "an")
    args = ap.parse_args()
    import yaml

    base_cfg_path = ROOT / "config" / args.base / "config.yml"
    base_cfg_text = base_cfg_path.read_text(encoding="utf-8")
    base_cfg = yaml.safe_load(base_cfg_text)
    module_names = {}
    module_splits = {}
    for m in base_cfg["modules"]:
        header, units = read_splits(ROOT / m["splits"])
        module_splits[m["name"]] = (header, units)
        module_names[m["name"]] = section_names(header)
    base_bin = load_binary(args.base, module_names, args.work / args.base)
    blocked = {b["source"] for b in base_cfg.get("block_relocations") or [] if "source" in b}
    base_bin.relocs = [r for r in base_bin.relocs if not (r[0].unit == "dol" and r[1] in blocked)]
    target_bin = load_binary(args.version, module_names, args.work / args.version)
    mapper = Mapper(base_bin, target_bin)
    mapper.build()

    out_dir = ROOT / "config" / args.version
    out_dir.mkdir(parents=True, exist_ok=True)
    report = collections.defaultdict(collections.Counter)
    report["order"] = []
    report["badsize"] = []
    report["unpaired"] = []
    report["dropnames"] = []

    syms = read_symbols(ROOT / base_cfg["symbols"])
    res = port_symbols(mapper, base_bin, "dol", syms, report)
    write_symbols(out_dir / "symbols.txt", syms, res)
    header, units = read_splits(ROOT / base_cfg["splits"])
    write_splits(out_dir / "splits.txt", mapper, base_bin, "dol", header, units)
    dol_map = {s["addr"]: res[id(s)][0] for s in syms if res[id(s)] is not None}

    for m in base_cfg["modules"]:
        name = m["name"]
        msyms = read_symbols(ROOT / m["symbols"])
        mres = port_symbols(mapper, base_bin, name, msyms, report)
        (out_dir / name).mkdir(exist_ok=True)
        write_symbols(out_dir / name / "symbols.txt", msyms, mres)
        header, units = module_splits[name]
        write_splits(out_dir / name / "splits.txt", mapper, base_bin, name, header, units)

    def map_dol_addr(addr):
        sp = base_bin.space_at("dol", addr)
        if addr in dol_map:
            return dol_map[addr]
        m = mapper.map_exact(sp, addr)
        if m is not None:
            return m
        return map_boundary(mapper, base_bin, "dol", sp.name, addr, False)

    root = ROOT / "orig" / args.version
    text = base_cfg_text.replace(f"orig/{args.base}", f"orig/{args.version}").replace(f"config/{args.base}/", f"config/{args.version}/")
    text = text.replace(base_cfg["hash"], sha1(root / "sys" / "main.dol"), 1)
    for m in base_cfg["modules"]:
        text = text.replace(m["hash"], sha1(root / m["object"]), 1)

    def addr_sub(match):
        key, val = match.group(1), int(match.group(2), 16)
        if 0x80000000 <= val < 0x81800000:
            return f"{key}0x{map_dol_addr(val):08X}"
        return match.group(0)

    text = re.sub(r"((?:source|target): )0x([0-9A-Fa-f]{8})", addr_sub, text)
    already = {int(v, 16) for v in re.findall(r"source: 0x([0-9A-Fa-f]{8})", text)}
    extra = "".join(f"- source: 0x{a:08X}\n" for a in sorted(mapper.constants - already))
    if extra:
        text = text.replace("\nmodules:\n", "\n" + extra + "modules:\n", 1)
    (out_dir / "config.yml").write_text(text, encoding="utf-8")

    sha_lines = [f"{sha1(root / 'sys' / 'main.dol')}  build/{args.version}/main.dol"]
    for m in base_cfg["modules"]:
        sha_lines.append(f"{sha1(root / m['object'])}  build/{args.version}/{m['name']}/{m['name']}.rel")
    (out_dir / "build.sha1").write_text("\n".join(sha_lines) + "\n", encoding="utf-8")

    for k in ("dol", "rel"):
        print(k, dict(report[k]))
    for line in report["order"][:50]:
        print("order:", line)
    print(len(report["order"]), "order problems")
    for line in report["badsize"][:50]:
        print("size:", line)
    print(len(report["badsize"]), "size problems")
    for line in report["unpaired"][:50]:
        print("unpaired:", line)
    print(len(report["unpaired"]), "unpaired function runs")
    print(len(mapper.constants), "pointer-like constants blocked")
    for line in report["dropnames"]:
        print("dropped:", line)


if __name__ == "__main__":
    main()
