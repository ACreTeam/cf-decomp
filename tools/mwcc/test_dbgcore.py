"""Address-space regressions; no Windows debugger or compiler required."""
import ctypes
import struct
import unittest
from unittest.mock import patch

import dbgcore


class MemoryProc(dbgcore.Proc):
    def __init__(self, base):
        super().__init__(None)
        self.image_base = base
        self.image_size = 0x340000
        self.memory = {}

    def rd(self, address, size):
        return self.memory.get(address, b'')[:size]

    def wr(self, address, data):
        self.memory[address] = data


class RelocationTests(unittest.TestCase):
    def test_addresses_and_scalars(self):
        for base in (0x400000, 0x500000, 0x790000):
            with self.subTest(base=base):
                p = MemoryProc(base)
                address = 0x7109AC
                self.assertEqual(p.preferred(p.runtime(address)), address)
                self.assertEqual(p.preferred(0x10000000), 0x10000000)
                self.assertEqual(p.preferred(base + p.image_size), base + p.image_size)
                # A scalar that happens to look like an image pointer stays intact.
                value = base + 0x1234
                p.wr(p.runtime(address), struct.pack('<I', value))
                self.assertEqual(p.u32(p.runtime(address)), value)
                # Raw memory addresses are not guessed from their numeric range.
                p.wr(0x401234, b'raw')
                self.assertEqual(p.rd(0x401234, 3), b'raw')

    def test_breakpoint_restore_and_rearm(self):
        for base in (0x400000, 0x500000, 0x790000):
            with self.subTest(base=base):
                p = MemoryProc(base)
                address = 0x468750
                p._saved[address] = b'\x55'
                p.arm(address)
                self.assertEqual(p.rd(p.runtime(address), 1), b'\xcc')
                p.disarm(address)
                self.assertEqual(p.rd(p.runtime(address), 1), b'\x55')
                p.arm(address)
                self.assertEqual(p.rd(p.runtime(address), 1), b'\xcc')

    def test_relocated_direct_call_chain(self):
        for base in (0x400000, 0x500000, 0x790000):
            with self.subTest(base=base):
                p = MemoryProc(base)
                current, caller, sp = 0x468750, 0x500010, 0x10000000
                ctx = ctypes.create_string_buffer(dbgcore.CTX_SIZE)
                struct.pack_into('<I', ctx, dbgcore.CTX_EIP, p.runtime(current))
                struct.pack_into('<I', ctx, dbgcore.CTX_ESP, sp)
                p.wr(sp, struct.pack('<I', p.runtime(caller)))
                p.wr(p.runtime(caller - 7), b'\x90\x90\xe8' + struct.pack('<i', current - caller))
                with patch.object(dbgcore, 'funcstart', side_effect=lambda a: a), \
                        patch.object(dbgcore, 'funcname', return_value='caller'):
                    self.assertEqual(dbgcore.callers(p, ctx), [(caller, 'caller')])


if __name__ == '__main__':
    unittest.main()
