import struct
import unittest

from find_candidates import candidates, relocation_mask, relocation_target


class CandidateTests(unittest.TestCase):
    def test_branch_only_matches_preserve_opcode_and_link_bit(self):
        reloc = dict(offset=0, type=10, addend=0)
        body = bytes.fromhex("48000000")
        raw = bytes.fromhex("480000104800001160000010")
        found = list(candidates(body, relocation_mask(4, [reloc]),
                                [(0x80001000, raw, True)], None, 0, 0x100000000))
        self.assertEqual([address for address, _ in found], [0x80001000])
        self.assertEqual(relocation_target(reloc, found[0][1], 0, found[0][0], {}), 0x80001010)

    def test_sda_reference_keeps_opcode_and_destination(self):
        reloc = dict(offset=4, type=109, addend=0)
        body = bytes.fromhex("38000000900000004e800020")
        raw = bytes.fromhex("38000000900dd9304e800020"
                            "38000000800dd9304e800020"
                            "38000000902dd9304e800020")
        found = list(candidates(body, relocation_mask(12, [reloc]),
                                [(0x802AA10C, raw, True)], None, 0, 0x100000000))
        self.assertEqual(len(found), 1)
        self.assertEqual(relocation_target(reloc, found[0][1], 0, found[0][0],
                                           {13: 0x807516C0}), 0x8074EFF0)
        with self.assertRaises(ValueError):
            relocation_target(dict(offset=0, type=109, addend=0),
                              bytes.fromhex("90010008"), 0, 0, {13: 0x807516C0})

    def test_negative_branch_displacement_and_addend(self):
        target = relocation_target(dict(offset=0, type=10, addend=4),
                                   struct.pack(">I", 0x4BFFFFF1), 0, 0x80001020, {})
        self.assertEqual(target, 0x8000100C)

    def test_data_and_wrong_function_sizes_are_excluded(self):
        body = bytes.fromhex("4e800020")
        sections = [(0x80001000, body, False), (0x80002000, body, True)]
        self.assertEqual(list(candidates(body, b"\xff" * 4, sections,
                                        {0x80001000: 4, 0x80002000: 8}, 0, 0x100000000)), [])

    def test_unknown_relocations_fail_closed(self):
        with self.assertRaises(ValueError):
            relocation_mask(4, [dict(offset=0, type=255)])


if __name__ == "__main__":
    unittest.main()
