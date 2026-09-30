#!/usr/bin/env python3
"""Find compiled PowerPC ELF functions in a DOL without modifying either file.

Matches ignore only supported relocation fields, preserving opcodes/registers.
They are candidates, not proof of a source split or a matching linked object.
Use --symbols to restrict candidates to DTK function starts/sizes and check
relocation targets whose names are already known. JSON is written to stdout.
"""

from __future__ import annotations

import argparse
import json
import re
import struct
from pathlib import Path


def cstring(data, offset):
    return data[offset:data.index(b"\0", offset)].decode("utf-8")


class Elf:
    def __init__(self, path):
        self.raw = Path(path).read_bytes()
        if self.raw[:6] != b"\x7fELF\x01\x02" or struct.unpack_from(">H", self.raw, 18)[0] != 20:
            raise ValueError(f"{path}: expected big-endian ELF32 PowerPC")
        start = struct.unpack_from(">I", self.raw, 32)[0]
        stride, count, strings = struct.unpack_from(">HHH", self.raw, 46)
        self.sections = [struct.unpack_from(">10I", self.raw, start + i * stride) for i in range(count)]
        self.names = [cstring(self.data(strings), s[0]) for s in self.sections]
        self.symbols = []
        symbol_tables = {}
        for i, section in enumerate(self.sections):
            if section[1] != 2:
                continue
            table = []
            for offset in range(0, section[5], section[9]):
                name, value, size, info, other, sec = struct.unpack_from(">IIIBBH", self.data(i), offset)
                table.append(dict(name=cstring(self.data(section[6]), name), value=value,
                                  size=size, info=info, section=sec))
            symbol_tables[i] = table
            self.symbols.extend(table)
        self.relocations = {}
        for i, section in enumerate(self.sections):
            if section[1] not in (4, 9):
                continue
            if section[1] == 9:
                raise ValueError("ELF REL relocations have implicit addends; only RELA is supported")
            for offset in range(0, section[5], section[9]):
                site, info, addend = struct.unpack_from(">IIi", self.data(i), offset)
                symbol = symbol_tables[section[6]][info >> 8]
                self.relocations.setdefault(section[7], []).append(
                    dict(offset=site, type=info & 255, symbol=symbol["name"], addend=addend))

    def data(self, index):
        section = self.sections[index]
        if section[1] == 8:
            return bytes(section[5])
        return self.raw[section[4]:section[4] + section[5]]


def dol_sections(path):
    raw = Path(path).read_bytes()
    result = []
    for i in range(18):
        offset, address, size = (struct.unpack_from(">I", raw, base + i * 4)[0]
                                 for base in (0, 0x48, 0x90))
        if size:
            if offset < 0x100 or offset + size > len(raw):
                raise ValueError("Invalid DOL section bounds")
            result.append((address, raw[offset:offset + size], i < 7))
    return result


def relocation_mask(size, relocations, start=0):
    mask = bytearray(b"\xff" * size)
    for reloc in relocations:
        at, kind = reloc["offset"] - start, reloc["type"]
        if kind == 0:
            continue
        if kind == 1:  # R_PPC_ADDR32
            field = b"\0" * 4
        elif kind in (3, 4, 5, 6):  # ADDR16, LO, HI, HA
            field = b"\0" * 2
        elif kind == 10:  # REL24: keep opcode, AA and LK
            field = bytes.fromhex("fc000003")
        elif kind == 109:  # EMB_SDA21: keep opcode and destination register
            field = bytes.fromhex("ffe00000")
        else:
            raise ValueError(f"Unsupported relocation type {kind}")
        if at < 0 or at + len(field) > size:
            raise ValueError("Relocation extends outside function")
        for i, value in enumerate(field):
            mask[at + i] &= value
    return mask


def signed(value, bits):
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def relocation_target(reloc, body, start, address, sda_bases):
    at, kind = reloc["offset"] - start, reloc["type"]
    if kind not in (1, 10, 109):
        return None
    word = struct.unpack_from(">I", body, at)[0]
    if kind == 1:
        target = word
    elif kind == 10:
        target = address + at + signed(word & 0x03FFFFFC, 26)
    else:
        register = (word >> 16) & 31
        if register not in (0, 2, 13):
            raise ValueError("SDA21 reference uses a non-SDA base register")
        base = sda_bases.get(register)
        if base is None:
            return None
        target = base + signed(word & 0xFFFF, 16)
    return (target - reloc["addend"]) & 0xFFFFFFFF


def candidates(body, mask, sections, starts, begin, end):
    # Search using the longest completely fixed byte span, then verify all bits.
    spans = list(re.finditer(b"\xff+", bytes(mask)))
    anchor = max(spans, key=lambda span: span.end() - span.start(), default=None)
    for base, data, executable in sections:
        if not executable:
            continue
        low, high = max(0, begin - base), min(len(data) - len(body), end - base - len(body))
        if starts is not None:
            positions = (addr - base for addr, size in starts.items()
                         if size == len(body) and low <= addr - base <= high)
        elif anchor:
            def positions_by_anchor():
                a, b = anchor.span()
                pos = data.find(body[a:b], low + a, high + b)
                while pos >= 0:
                    yield pos - a
                    pos = data.find(body[a:b], pos + 1, high + b)
            positions = positions_by_anchor()
        else:
            positions = range((low + 3) & ~3, high + 1, 4)
        for pos in positions:
            if pos % 4 or not low <= pos <= high:
                continue
            raw = data[pos:pos + len(body)]
            if all((a ^ b) & m == 0 for a, b, m in zip(body, raw, mask)):
                yield base + pos, raw


def read_symbols(path):
    names, starts = {}, {}
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        match = re.match(r"(.+?) = ([^:]+):0x([\dA-Fa-f]+);.*", line)
        if not match:
            continue
        name, section, address = match.groups()
        address = int(address, 16)
        names[name] = address
        size = re.search(r"size:0x([\dA-Fa-f]+)", line)
        if "type:function" in line and size:
            starts[address] = int(size[1], 16)
    return names, starts


def scan(path, sections, names, starts, args):
    elf, report = Elf(path), []
    for symbol in elf.symbols:
        sec, start, size = symbol["section"], symbol["value"], symbol["size"]
        if symbol["info"] & 15 != 2 or not size or not 0 < sec < len(elf.sections):
            continue
        body = elf.data(sec)[start:start + size]
        rels = [r for r in elf.relocations.get(sec, []) if start <= r["offset"] < start + size]
        entry = dict(name=symbol["name"], section=elf.names[sec], offset=hex(start), size=hex(size),
                     relocations=rels, matches=[])
        try:
            mask = relocation_mask(size, rels, start)
        except ValueError as error:
            entry["unsupported"] = str(error)
            report.append(entry)
            continue
        count = 0
        for address, raw in candidates(body, mask, sections, starts, args.start, args.end):
            targets = {}
            valid = True
            for reloc in rels:
                try:
                    target = relocation_target(reloc, raw, start, address, {0: 0, 2: args.r2, 13: args.r13})
                except ValueError:
                    valid = False
                    break
                name = reloc["symbol"]
                if target is not None:
                    if name in targets and targets[name] != target:
                        valid = False
                    if name in names and names[name] != target:
                        valid = False
                    targets[name] = target
            if valid:
                count += 1
                if count <= args.limit:
                    entry["matches"].append(dict(address=f"0x{address:08X}",
                        targets={name: f"0x{target:08X}" for name, target in targets.items()}))
        entry["match_count"] = count
        report.append(entry)
    return dict(object=str(path), functions=report)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("objects", nargs="+", type=Path)
    parser.add_argument("--dol", required=True, type=Path)
    parser.add_argument("--symbols", type=Path)
    number = lambda value: int(value, 0)
    parser.add_argument("--start", type=number, default=0)
    parser.add_argument("--end", type=number, default=0x100000000)
    parser.add_argument("--r2", type=number, help="verified SDA2 base (optional)")
    parser.add_argument("--r13", type=number, help="verified SDA base (optional)")
    parser.add_argument("--limit", type=int, default=20, help="maximum candidates shown per function")
    args = parser.parse_args()
    if args.limit < 1 or args.start >= args.end:
        parser.error("limit must be positive and start must precede end")
    names, starts = read_symbols(args.symbols) if args.symbols else ({}, None)
    sections = dol_sections(args.dol)
    print(json.dumps([scan(path, sections, names, starts, args) for path in args.objects], indent=2))


if __name__ == "__main__":
    main()
