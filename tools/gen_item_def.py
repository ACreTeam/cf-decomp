#!/usr/bin/env python3
"""Generate include/game/game/d_item_def.hpp from the game's item table.

Reads item/item.bin out of Item/item.arc (a U8 archive) and writes one enum entry per BITM
record, in table order, so code can name item indices (the values infoBank_c::getBITM(u16),
seeker_c and indexTable_c work with) instead of using raw numbers.

Names come from the US item name (BITM::m_nameUs). Original-design slots, whose US name is
an untranslated placeholder, are named after their kind and slot number. Repeated names get
the kind as a suffix when the kinds differ, then a two-digit ordinal.

Usage: python tools/gen_item_def.py [--arc orig/RUUE01_00/files/Item/item.arc]
                                    [--out include/game/game/d_item_def.hpp]
"""

from __future__ import annotations

import argparse
import re
import struct
import unicodedata
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BITM_MAGIC = 0x4249544D  # 'BITM'
PLACEHOLDER_US = "ブランクえいご"  # "blank English": original-design slots
# Shorter suffixes for kinds used to tell repeated names apart.
SUFFIX_ALIAS = {"FAKE_PICTURE_BEFORE": "FAKE", "EGG_FAKE_BEFORE": "FAKE"}


def read_u8_file(arc: bytes, want: str) -> bytes:
    magic, root, _, _ = struct.unpack(">IIII", arc[:16])
    if magic != 0x55AA382D:
        raise ValueError("not a U8 archive")
    total = struct.unpack(">I", arc[root + 8 : root + 12])[0]
    strtab = root + total * 12
    nodes = [struct.unpack(">BxHII", arc[root + i * 12 : root + i * 12 + 12]) for i in range(total)]

    def name(off: int) -> str:
        end = arc.index(b"\0", strtab + off)
        return arc[strtab + off : end].decode("latin-1")

    stack: list[tuple[int, str]] = []
    for i, (typ, name_off, a, b) in enumerate(nodes):
        while stack and i >= stack[-1][0]:
            stack.pop()
        if typ == 1:
            if i:
                stack.append((b, name(name_off)))
        elif "/".join([s for _, s in stack] + [name(name_off)]) == want:
            return arc[a : a + b]
    raise FileNotFoundError(want)


def kind_names() -> dict[int, str]:
    """dItem kind enum (d_item.hpp): value -> name without the KIND_ prefix."""
    src = (ROOT / "include/game/game/d_item.hpp").read_text(encoding="utf-8", errors="replace")
    body = re.search(r"enum Kind \{(.*?)\};", src, re.S) or re.search(r"\n\s*KIND_MONEY,.*?KIND_COUNT", src, re.S)
    names = re.findall(r"\bKIND_(\w+)\s*(?:=\s*(\w+))?\s*,", body.group(0))
    out, value = {}, 0
    for name, explicit in names:
        if explicit:
            value = int(explicit, 0)
        out[value] = name
        value += 1
    return out


def ident(text: str) -> str:
    text = unicodedata.normalize("NFKD", text).encode("ascii", "ignore").decode()
    text = text.replace("&", " and ").replace("'", "").replace(".", "").replace(",", "")
    return re.sub(r"[^A-Za-z0-9]+", "_", text).strip("_").upper()


def load_items(arc_path: Path) -> list[dict]:
    data = read_u8_file(arc_path.read_bytes(), "item/item.bin")
    count, entry_size = struct.unpack(">II", data[:8])
    items = []
    for i in range(count):
        e = data[0x20 + i * entry_size : 0x20 + (i + 1) * entry_size]
        magic, _price, base_id = struct.unpack(">Iih", e[:10])
        us = e[0x34 : 0x34 + 34].decode("utf-16-be", "replace").split("\0")[0]
        kind = struct.unpack(">b", e[0x166:0x167])[0]
        items.append(dict(index=i, valid=magic == BITM_MAGIC, base=base_id, kind=kind, us=us))
    return items


def assign_names(items: list[dict], kinds: dict[int, str]) -> None:
    slot = Counter()
    for it in items:
        if it["index"] == 0:
            it["name"] = "DUMMY"
        elif it["us"] == PLACEHOLDER_US or not ident(it["us"]):
            kind = kinds.get(it["kind"], f"KIND{it['kind']}")
            it["name"] = f"{kind}_{slot[kind]:02d}"
            slot[kind] += 1
        else:
            it["name"] = ident(it["us"])
    groups = defaultdict(list)
    for it in items:
        groups[it["name"]].append(it)
    for name, group in groups.items():
        if len(group) < 2:
            continue
        if len({it["kind"] for it in group}) > 1:
            first_kind = group[0]["kind"]
            for it in group:
                if it["kind"] != first_kind:
                    kind = kinds.get(it["kind"], str(it["kind"]))
                    it["name"] = f"{name}_{SUFFIX_ALIAS.get(kind, kind)}"
    groups = defaultdict(list)
    for it in items:
        groups[it["name"]].append(it)
    for name, group in groups.items():
        if len(group) > 1:
            for n, it in enumerate(group):
                it["name"] = f"{name}_{n:02d}"
    assert len({it["name"] for it in items}) == len(items)


def render(items: list[dict], kinds: dict[int, str]) -> str:
    width = max(len(it["name"]) for it in items) + len("ITEM_IDX_")
    lines = [
        "#pragma once",
        "",
        "// Item indices: the order of the BITM table in item/item.bin (Item/item.arc).",
        "// Generated by tools/gen_item_def.py from the game data; do not edit by hand.",
        "//",
        "// An index is what infoBank_c::getBITM(u16), seeker_c and indexTable_c use. An item id",
        "// (dItem::Item::mId) is 0x9000 + (baseId << 2) plus the variant bits; indexTable_c maps a base",
        "// id to its index. Names come from the US item names; original-design slots are named after",
        "// their kind, and repeated names get the kind and/or a two-digit ordinal.",
        "// Each comment gives the entry's base id and kind.",
        "",
        "namespace dItem {",
        "",
        "enum ItemIndex {",
    ]
    for it in items:
        base = "----" if it["base"] == -1 else f"{it['base'] & 0xFFFF:03X}"
        kind = kinds.get(it["kind"], str(it["kind"]))
        name = f"ITEM_IDX_{it['name']}"
        lines.append(f"    {name:<{width}} = 0x{it['index']:03X}, // base {base}, {kind}")
    lines += [
        "",
        f"    ITEM_IDX_NUM = 0x{len(items):03X}, // == ITEM_COUNT",
        "};",
        "",
        "} // namespace dItem",
        "",
    ]
    return "\n".join(lines)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--arc", default="orig/RUUE01_00/files/Item/item.arc")
    ap.add_argument("--out", default="include/game/game/d_item_def.hpp")
    args = ap.parse_args()
    items = load_items(ROOT / args.arc)
    bad = [it["index"] for it in items if not it["valid"]]
    if bad:
        raise SystemExit(f"entries without the BITM magic: {bad[:10]}")
    kinds = kind_names()
    assign_names(items, kinds)
    (ROOT / args.out).write_text(render(items, kinds), encoding="utf-8", newline="\n")
    print(f"wrote {len(items)} entries to {args.out}")


if __name__ == "__main__":
    main()
