#!/usr/bin/env python3
"""Complete a dtk dol config inventory for the City Folk decompilation.

Run from any directory. Paths passed on the CLI are project-relative.
Requires PyYAML (tools/requirements-config.txt). Original binaries must be
extracted; this helper deliberately does not guess archive/VFS paths.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import os
import re
import struct
import tempfile
from pathlib import Path

try:
    import yaml
except ImportError:
    raise SystemExit("PyYAML is required. Install tools/requirements-config.txt with your project Python.")


PROJECT = Path(__file__).resolve().parent.parent
PLACEHOLDER_HASH = "0123456789abcdef0123456789abcdef01234567"
CITY_FOLK_DOL_HASH = "c87967b9ac0a943ba04ac16d875f5f79b4c46516"
# Verified against this revision's DOL/startup table and DTK section detection.
# DTK 1.8.3 may misname the final region as .bss2 or a second .sbss.
# Original r2-relative accesses require the linker-recognized name .sbss2.
CITY_FOLK_SECTIONS = """Sections:
\t.init       type:code align:4
\textab       type:rodata align:32
\textabindex  type:rodata align:32
\t.text       type:code align:16
\t.ctors      type:rodata align:4
\t.dtors      type:rodata align:16
\t.rodata     type:rodata align:32
\t.data       type:data align:32
\t.bss        type:bss align:32
\t.sdata      type:data align:32
\t.sbss       type:bss align:32
\t.sdata2     type:rodata align:8
\t.sbss2      type:bss align:16

Runtime.PPCEABI.H/__init_cpp_exceptions.cpp:
\t.text       start:0x8044EE58 end:0x8044EEC8
\t.ctors      start:0x80465620 end:0x80465624 rename:.ctors$10
\t.dtors      start:0x80465960 end:0x80465964 rename:.dtors$10
\t.dtors      start:0x80465964 end:0x80465968 rename:.dtors$15
\t.sdata      start:0x8074E0D8 end:0x8074E0E0
"""
REL_SECTIONS = """Sections:
\t.text       type:code align:4
\t.ctors      type:rodata align:4
\t.dtors      type:rodata align:8
\t.rodata     type:rodata align:8
\t.data       type:data align:8
\t.bss        type:bss align:8
"""
# These three RELs get four duplicate .rodata names in DTK 1.8.3.
# _prolog/_epilog pass sections 2/3 to the shared ctor/dtor runners.
# Standard names also prevent MW from creating extra .ctors/.dtors sections.
SECTION_PROFILES = {
    CITY_FOLK_DOL_HASH: CITY_FOLK_SECTIONS,
    "87a8ac3d70f23e8e5793aeb797ceb0e17fa68042": REL_SECTIONS,  # cafe
    "282792295bca50fb9f2adf0f9373d67f4897af2d": REL_SECTIONS,  # fossil
    "fae8f00b5b842207b134e7a03b4b4176bb231729": REL_SECTIONS,  # sample
}
DEFAULTS = {
    "mw_comment_version": 14,
    "extract_objects": True,
    "quick_analysis": False,
    "symbols_known": False,
    "detect_objects": True,
    "detect_strings": True,
    "write_asm": True,
    "fill_gaps": True,
    "export_all": True,
    "globalize_symbols": True,
    "clean_extab": False,
}
PREAMBLE = """# Prepared by tools/prepare_config.py from the generated binary inventory.
# All object paths are relative to object_base; symbols/splits are project-relative.
# RTTI is partial symbol evidence; symbols_known stays false for continued analysis.
# quick_analysis may be enabled after the initial symbol/split files are generated.
# No map or common_start is guessed: ordinary BSS does not identify linker-common BSS.
# mw_comment_version: 14 is the existing project setting, not a detected compiler version.
# Keep original exception-table bytes for verification (clean_extab: false).
# Default REL linking is retained after checking unique IDs and available imports.
# Edit settings here as analysis improves; reruns preserve existing settings by object path.
"""


class UniqueLoader(yaml.SafeLoader):
    """Reject duplicate YAML keys instead of silently losing a user's settings."""


def unique_mapping(loader, node, deep=False):
    loader.flatten_mapping(node)
    result = {}
    for key_node, value_node in node.value:
        key = loader.construct_object(key_node, deep=deep)
        if key in result:
            raise ValueError(f"Duplicate YAML key: {key}")
        result[key] = loader.construct_object(value_node, deep=deep)
    return result


UniqueLoader.add_constructor(yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG, unique_mapping)


def load_config(path: Path) -> dict:
    value = yaml.load(path.read_text(encoding="utf-8-sig"), Loader=UniqueLoader)
    if not isinstance(value, dict) or not isinstance(value.get("object"), str):
        raise ValueError(f"{path}: expected a configuration with an object path")
    if not isinstance(value.get("modules", []), list):
        raise ValueError(f"{path}: modules must be a list")
    if any(not isinstance(module, dict) for module in value.get("modules", [])):
        raise ValueError(f"{path}: every module must be a mapping")
    return value


def inside(root: Path, path: str | Path) -> Path:
    result = (root / path).resolve()
    if not result.is_relative_to(root.resolve()):
        raise ValueError(f"Path leaves the project: {path}")
    return result


def object_path(root: Path, module: dict, config: dict) -> Path:
    name = module.get("object")
    if not isinstance(name, str) or ":" in name:
        raise ValueError(f"Expected an extracted, project-relative object path: {name!r}")
    return inside(root, Path(config.get("object_base") or "") / name)


def output_name(value: str) -> str:
    if (not isinstance(value, str) or not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", value)
            or value.endswith(".") or re.fullmatch(r"CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9]", value.split(".")[0], re.I)):
        raise ValueError(f"Unsafe module/version name: {value!r}")
    return value


def read_object(path: Path, expected_hash: str | None) -> tuple[bytes, str]:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    if expected_hash is not None and (not isinstance(expected_hash, str) or expected_hash.lower() != digest):
        raise ValueError(f"SHA-1 mismatch for {path}: generated inventory does not match the original")
    return data, digest


def rel_imports(data: bytes, path: Path) -> tuple[int, set[int]]:
    if len(data) < 0x40:
        raise ValueError(f"Truncated REL header: {path}")
    module_id = struct.unpack_from(">I", data)[0]
    if module_id == 0:
        raise ValueError(f"REL ID zero is reserved for the DOL: {path}")
    start, size = struct.unpack_from(">II", data, 0x28)
    if size % 8 or start + size > len(data):
        raise ValueError(f"Invalid REL import table: {path}")
    return module_id, {struct.unpack_from(">I", data, offset)[0] for offset in range(start, start + size, 8)}


def prepare(root: Path, inventory: dict, existing: dict, version: str, build_dir: Path) -> tuple[dict, str, dict]:
    """Validate everything before producing output; never modify symbol/split files."""
    version = output_name(version)
    root = root.resolve()
    original_base = existing.get("object_base") or inventory.get("object_base") or f"orig/{version}"
    base = inside(root, original_base)
    build_dir = inside(root, build_dir).relative_to(root)
    config_dir = Path("config") / version
    result = {**copy.deepcopy(inventory), **copy.deepcopy(existing)}
    result.pop("modules", None)
    result["name"] = output_name(result.get("name", "main"))
    result["object_base"] = base.relative_to(root).as_posix()

    dol_path = object_path(root, inventory, inventory)
    if dol_path.suffix.lower() != ".dol":
        raise ValueError("The main object must be an extracted DOL")
    dol, digest = read_object(dol_path, inventory.get("hash"))
    if len(dol) < 0x100:
        raise ValueError("Truncated DOL header")
    result["object"] = dol_path.relative_to(base).as_posix()
    result["hash"] = digest
    result.setdefault("symbols", (config_dir / "symbols.txt").as_posix())
    result.setdefault("splits", (config_dir / "splits.txt").as_posix())
    for key, value in DEFAULTS.items():
        result.setdefault(key, value)

    existing_modules = {}
    for module in existing.get("modules", []):
        if module.get("object") == "files/module.rel" and module.get("hash") == PLACEHOLDER_HASH:
            continue  # The untouched dtk-template example is not a real module.
        path = object_path(root, module, existing)
        if path in existing_modules:
            raise ValueError(f"Repeated existing module object: {path}")
        existing_modules[path] = module

    modules = []
    used_paths = set()
    names = {result["name"].casefold()}
    module_ids = {}
    imports_by_name = {}
    for source in inventory.get("modules", []):
        path = object_path(root, source, inventory)
        if path in used_paths or path.suffix.lower() != ".rel":
            raise ValueError(f"Duplicate or non-REL object: {path}")
        used_paths.add(path)
        data, digest = read_object(path, source.get("hash"))
        module_id, imports = rel_imports(data, path)
        if module_id in module_ids:
            raise ValueError(f"Duplicate REL ID {module_id}; link-group configuration needs manual review")
        module = {**copy.deepcopy(source), **copy.deepcopy(existing_modules.get(path, {}))}
        name = output_name(module.get("name", path.stem))
        if name.casefold() in names:
            raise ValueError(f"Duplicate module output name: {name}")
        names.add(name.casefold())
        module_ids[module_id] = name
        imports_by_name[name] = imports
        module.update({"name": name, "object": path.relative_to(base).as_posix(), "hash": digest})
        module.setdefault("symbols", (config_dir / name / "symbols.txt").as_posix())
        module.setdefault("splits", (config_dir / name / "splits.txt").as_posix())
        modules.append((module_id, module))
    missing = existing_modules.keys() - used_paths
    if missing:
        raise ValueError("Refusing to drop existing modules absent from inventory: " + ", ".join(map(str, sorted(missing))))
    result["modules"] = [module for _, module in sorted(modules)]
    available_ids = {0, *module_ids}
    for module in result["modules"]:
        missing_imports = imports_by_name[module["name"]] - available_ids
        if missing_imports:
            raise ValueError(f"{module['name']}: missing imported REL IDs {sorted(missing_imports)}")
        if "links" in module:
            if not isinstance(module["links"], list) or any(name not in module_ids.values() for name in module["links"]):
                raise ValueError(f"{module['name']}: unknown or invalid module links")
            required = {module_ids[mid] for mid in imports_by_name[module["name"]] if mid}
            if not required <= set(module["links"]) | {module["name"]}:
                raise ValueError(f"{module['name']}: explicit links omit an imported module")

    destinations = set()
    for module in [result, *result["modules"]]:
        for key in ("symbols", "splits"):
            if not isinstance(module[key], str) or not module[key]:
                raise ValueError(f"{module['name']}: {key} needs a nonempty output path")
            path = inside(root, module[key])
            if path in destinations or path == dol_path or path in used_paths:
                raise ValueError(f"Conflicting symbol/split destination: {path}")
            destinations.add(path)
    manifest = [f"{result['hash']}  {(build_dir / version / (result['name'] + '.dol')).as_posix()}"]
    manifest.extend(f"{module['hash']}  {(build_dir / version / module['name'] / (module['name'] + '.rel')).as_posix()}"
                    for module in result["modules"])
    bss_start, bss_size = struct.unpack_from(">II", dol, 0xD8)
    audit = {"modules": len(modules), "checked_hashes": len(modules) + 1,
             "bss_envelope": [bss_start, bss_start + bss_size], "destinations": sorted(destinations)}
    return result, "\n".join(manifest) + "\n", audit


def write_changed(root: Path, path: Path, content: str, version: str) -> bool:
    path = inside(root, path)
    data = content.encode("utf-8")
    previous = path.read_bytes() if path.exists() else None
    if previous == data:
        return False
    if previous is not None:
        backup = inside(root, Path("build/config-backups") / version /
                        (path.name + "." + hashlib.sha256(previous).hexdigest()[:16] + ".bak"))
        backup.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            backup.write_bytes(previous)
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as stream:
        temporary = Path(stream.name)
        stream.write(data)
    try:
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)
    return True


def initial_sections(root: Path, config: dict) -> dict[Path, str]:
    """Seed blank/comment-only splits for binaries with verified section profiles."""
    outputs = {}
    for module in [config, *config.get("modules", [])]:
        sections = SECTION_PROFILES.get(module["hash"])
        if sections is None:
            continue
        path = inside(root, module["splits"])
        previous = path.read_text(encoding="utf-8-sig") if path.exists() else ""
        if any(line.strip() and not line.lstrip().startswith(("//", "#")) for line in previous.splitlines()):
            continue
        # Preserve the user's initial comments, and never replace real split records.
        prefix = previous.rstrip() + "\n\n" if previous.strip() else ""
        outputs[path] = prefix + sections
    return outputs


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", default="RUUE01_00")
    parser.add_argument("--input", type=Path, help="Default: config/VERSION/config.generated.yml")
    parser.add_argument("--output", type=Path, help="Default: config/VERSION/config.yml")
    parser.add_argument("--build-dir", type=Path, default=Path("build"), help="Build output prefix for build.sha1")
    parser.add_argument("--check", action="store_true", help="Validate and check for stale output without writing")
    args = parser.parse_args()
    try:
        version = output_name(args.version)
        folder = Path("config") / version
        source = inside(PROJECT, args.input or folder / "config.generated.yml")
        target = inside(PROJECT, args.output or folder / "config.yml")
        if source == target:
            raise ValueError("Input inventory and output config must be different files")
        inventory = load_config(source)
        existing = load_config(target) if target.exists() else {}
        config, manifest, audit = prepare(PROJECT, inventory, existing, version, args.build_dir)
        outputs = {target: PREAMBLE + yaml.safe_dump(config, sort_keys=False, width=120, allow_unicode=True),
                   inside(PROJECT, folder / "build.sha1"): manifest}
        if len(outputs) != 2 or source in outputs or any(path in outputs for path in audit["destinations"]):
            raise ValueError("Conflicting configuration/manifest/input/symbol paths")
        outputs.update(initial_sections(PROJECT, config))
        stale = [path for path, text in outputs.items() if not path.exists() or path.read_bytes() != text.encode("utf-8")]
        if args.check:
            for path in stale:
                print(f"Out of date: {path.relative_to(PROJECT)}")
        else:
            for path, text in outputs.items():
                changed = write_changed(PROJECT, path, text, version)
                print(f"{'Updated' if changed else 'Unchanged'}: {path.relative_to(PROJECT)}")
            for path in audit["destinations"]:
                path.parent.mkdir(parents=True, exist_ok=True)
        print(f"{audit['modules']} RELs; {audit['checked_hashes']} original hashes checked; unique IDs and complete imports")
        print("Existing analysis is preserved; empty splits with known binary hashes get unique section names.")
        print("DTK analysis creates/populates the remaining symbol/split files.")
        common = "existing common_start preserved" if "common_start" in config else "common BSS unresolved"
        print(f"DOL BSS envelope: 0x{audit['bss_envelope'][0]:08x}..0x{audit['bss_envelope'][1]:08x} (end exclusive); {common}.")
        if args.check and stale:
            raise SystemExit(1)
    except (ValueError, OSError, yaml.YAMLError) as error:
        parser.exit(2, f"Configuration error: {error}\n")


if __name__ == "__main__":
    main()
