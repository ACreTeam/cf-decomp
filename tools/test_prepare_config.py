"""Regression checks for configuration migration and preservation of analysis."""

import contextlib
import copy
import hashlib
import io
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import prepare_config as helper


class PrepareConfigTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        self.version = "RUUE01_00"
        self.folder = self.root / "config" / self.version
        self.folder.mkdir(parents=True)
        dol = bytearray(0x100)
        struct.pack_into(">II", dol, 0xD8, 0x80562E80, 0x1F0BC0)
        self.inventory = self.original("sys/main.dol", dol)
        self.inventory["modules"] = [self.rel("alpha", 1, [0, 1, 2]), self.rel("beta", 2, [0])]
        self.existing = {
            "object_base": f"orig/{self.version}", "object": "sys/main.dol",
            "hash": helper.PLACEHOLDER_HASH, "mw_comment_version": 14,
            "modules": [{"object": "files/module.rel", "hash": helper.PLACEHOLDER_HASH}],
        }

    def original(self, suffix, data):
        path = self.root / "orig" / self.version / suffix
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return {"object": path.relative_to(self.root).as_posix(), "hash": hashlib.sha1(data).hexdigest()}

    def rel(self, name, module_id, imports):
        data = bytearray(0x80 + len(imports) * 8)
        struct.pack_into(">I", data, 0, module_id)
        struct.pack_into(">II", data, 0x28, 0x80, len(imports) * 8)
        for index, imported in enumerate(imports):
            struct.pack_into(">II", data, 0x80 + index * 8, imported, 0)
        return self.original(f"files/rels/{name}.rel", data)

    def prepare(self, existing=None):
        return helper.prepare(self.root, self.inventory, self.existing if existing is None else existing,
                              self.version, Path("build"))

    def test_migrates_inventory_and_build_manifest(self):
        config, manifest, audit = self.prepare()
        self.assertEqual(config["object"], "sys/main.dol")
        self.assertEqual(config["object_base"], "orig/RUUE01_00")
        self.assertEqual([m["name"] for m in config["modules"]], ["alpha", "beta"])
        self.assertEqual(config["modules"][0]["object"], "files/rels/alpha.rel")
        self.assertEqual(config["modules"][0]["symbols"], "config/RUUE01_00/alpha/symbols.txt")
        self.assertEqual(config["modules"][1]["splits"], "config/RUUE01_00/beta/splits.txt")
        self.assertEqual([line.split()[1] for line in manifest.splitlines()], [
            "build/RUUE01_00/main.dol", "build/RUUE01_00/alpha/alpha.rel", "build/RUUE01_00/beta/beta.rel"])
        self.assertEqual(audit["bss_envelope"], [0x80562E80, 0x80753A40])
        self.assertNotIn("common_start", config)
        self.assertFalse(config["symbols_known"])

    def test_preserves_manual_settings_and_is_idempotent(self):
        config, _, _ = self.prepare()
        config.update({"common_start": 0x80740000, "detect_strings": False, "mw_comment_version": 11})
        config["modules"][0].update({"symbols": "config/custom_symbols.txt", "links": ["beta"],
                                      "block_relocations": [{"source": ".text:0x40"}]})
        original = copy.deepcopy(config)
        again, _, _ = self.prepare(config)
        self.assertEqual(again, original)
        self.assertEqual(config, original)

    def test_rejects_duplicate_ids_and_missing_imports(self):
        self.inventory["modules"][1] = self.rel("beta", 1, [0])
        with self.assertRaisesRegex(ValueError, "Duplicate REL ID"):
            self.prepare()
        self.inventory["modules"][1] = self.rel("beta", 3, [0])
        with self.assertRaisesRegex(ValueError, "missing imported REL IDs"):
            self.prepare()

    def test_rejects_hash_mismatch(self):
        (self.root / self.inventory["modules"][0]["object"]).write_bytes(b"changed")
        with self.assertRaisesRegex(ValueError, "SHA-1 mismatch"):
            self.prepare()

    def test_does_not_silently_drop_existing_modules(self):
        config, _, _ = self.prepare()
        self.inventory["modules"].pop()
        with self.assertRaisesRegex(ValueError, "Refusing to drop"):
            self.prepare(config)

    def test_rejects_output_collisions_and_paths_outside_project(self):
        config, _, _ = self.prepare()
        config["modules"][0]["symbols"] = config["symbols"]
        with self.assertRaisesRegex(ValueError, "Conflicting symbol/split"):
            self.prepare(config)
        config["modules"][0]["symbols"] = "../outside.txt"
        with self.assertRaisesRegex(ValueError, "Path leaves the project"):
            self.prepare(config)

    def test_rejects_duplicate_yaml_keys(self):
        path = self.folder / "invalid.yml"
        path.write_text("object: main.dol\nobject: second.dol\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "Duplicate YAML key"):
            helper.load_config(path)

    def test_seeds_verified_sections_only_in_empty_splits(self):
        config, _, _ = self.prepare()
        self.assertEqual(helper.initial_sections(self.root, config), {})
        config["hash"] = helper.CITY_FOLK_DOL_HASH
        path = self.root / config["splits"]
        path.write_text("// initial comment\n", encoding="utf-8")
        outputs = helper.initial_sections(self.root, config)
        seeded = outputs[path]
        self.assertTrue(seeded.startswith("// initial comment\n"))
        self.assertIn(".text       type:code align:4", seeded)
        self.assertIn("0x800075C0; // type:function size:0x4C align:16",
                      outputs[self.root / config["symbols"]])
        self.assertIn(".sbss2      type:bss align:16", seeded)
        self.assertNotIn(".bss2", seeded)
        self.assertIn("Runtime.PPCEABI.H/__init_cpp_exceptions.cpp:", seeded)
        for subsection in (".ctors$10", ".dtors$10", ".dtors$15"):
            self.assertIn(f"rename:{subsection}", seeded)
        names = [line.split()[0] for line in seeded.splitlines() if "type:" in line]
        self.assertEqual(len(names), len(set(names)))
        for destination, text in outputs.items():
            destination.write_text(text, encoding="utf-8")
        self.assertEqual(helper.initial_sections(self.root, config), {})
        symbols = self.root / config["symbols"]
        symbols.write_text("custom_name = .text:0x800075C0; // type:function size:0x4C align:16\n",
                           encoding="utf-8")
        self.assertEqual(helper.initial_sections(self.root, config), {})
        config["modules"][0]["hash"] = "87a8ac3d70f23e8e5793aeb797ceb0e17fa68042"
        rel_path = self.root / config["modules"][0]["splits"]
        rel_seed = helper.initial_sections(self.root, config)[rel_path]
        self.assertIn(".ctors      type:rodata align:4", rel_seed)
        self.assertIn(".dtors      type:rodata align:8", rel_seed)
        self.assertIn(".data       type:data align:8", rel_seed)
        self.assertNotIn(".rodata5", rel_seed)

    def test_cli_preserves_analysis_backs_up_and_checks_without_writing(self):
        source = self.folder / "config.generated.yml"
        source.write_text(helper.yaml.safe_dump(self.inventory), encoding="utf-8")
        target = self.folder / "config.yml"
        old_config = helper.yaml.safe_dump(self.existing).encode()
        target.write_bytes(old_config)
        symbols = self.folder / "symbols.txt"
        symbols.write_bytes(b"// manually named functions\n")
        splits = self.folder / "splits.txt"
        splits.write_bytes(b"// hand-edited translation units\n")
        before = {p: p.read_bytes() for p in (source, symbols, splits)}
        with patch.object(helper, "PROJECT", self.root), contextlib.redirect_stdout(io.StringIO()):
            with patch("sys.argv", ["prepare_config.py", "--check"]), self.assertRaises(SystemExit) as raised:
                helper.main()
            self.assertEqual(raised.exception.code, 1)
            self.assertEqual(target.read_bytes(), old_config)
            self.assertFalse((self.root / "build").exists())
            with patch("sys.argv", ["prepare_config.py"]):
                helper.main()
            after = {p: p.read_bytes() for p in (target, self.folder / "build.sha1")}
            with patch("sys.argv", ["prepare_config.py", "--check"]):
                helper.main()
            with patch("sys.argv", ["prepare_config.py"]):
                helper.main()
        self.assertEqual(before, {p: p.read_bytes() for p in before})
        self.assertEqual(after, {p: p.read_bytes() for p in after})
        backups = list((self.root / "build/config-backups/RUUE01_00").glob("config.yml.*.bak"))
        self.assertEqual(len(backups), 1)
        self.assertEqual(backups[0].read_bytes(), old_config)
        self.assertTrue((self.folder / "alpha").is_dir())
        self.assertFalse((self.folder / "alpha/symbols.txt").exists())


if __name__ == "__main__":
    unittest.main()
