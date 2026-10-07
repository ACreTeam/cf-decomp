"""Tiny Win32 debugger core for instrumenting the MWCC compiler (32-bit, runs under WOW64).

A tool gives it the compile argv (from mwcc_cmd.compile_argv) and a dict {address: handler}. Every
time the compiler executes one of those addresses, handler(proc, ctx) runs: proc reads the compiler's
memory, and ctx is the WOW64 thread context (use esp()/arg() to read stack arguments). The breakpoint is
then stepped over and re-armed, so each address fires every time.

    from dbgcore import run, esp, arg
    def on_codegen(p, ctx):
        print('generating', p.objname(arg(p, ctx, 2)))
    run(argv, {0x468750: on_codegen})

Addresses are for mwcceppc.exe GC 3.0a5.2. See notes/mwcc30_functions.csv for names, and
notes/mwcc_ghidra.md for the compiler's data structures found so far.
"""
import ctypes
import ctypes.wintypes as W
import struct
import subprocess

k = ctypes.WinDLL('kernel32', use_last_error=True) if hasattr(ctypes, 'WinDLL') else None

CTX_SIZE = 716          # WOW64_CONTEXT
CTX_EIP, CTX_EFLAGS, CTX_ESP = 184, 192, 196
NAME_OFF, NAMESTR_OFF = 0xC, 0xA   # Object -> HashNameNode* -> chars
OPTAB = 0x6A290C        # PCode opcode info table (name ptr, index), 0x18 per entry


class _SI(ctypes.Structure):
    _fields_ = [('cb', W.DWORD), ('r', W.LPWSTR), ('d', W.LPWSTR), ('t', W.LPWSTR), ('x', W.DWORD * 8),
                ('f', W.WORD), ('s', W.WORD), ('r2', ctypes.c_void_p), ('h1', W.HANDLE), ('h2', W.HANDLE),
                ('h3', W.HANDLE)]


class _PI(ctypes.Structure):
    _fields_ = [('hp', W.HANDLE), ('ht', W.HANDLE), ('pid', W.DWORD), ('tid', W.DWORD)]


class _DE(ctypes.Structure):
    _fields_ = [('code', W.DWORD), ('pid', W.DWORD), ('tid', W.DWORD), ('pad', W.DWORD), ('u', ctypes.c_byte * 160)]


class Proc:
    """Memory access into the debugged compiler."""

    def __init__(self, hp):
        self.hp = hp
        self._ops = {}

    def rd(self, a, n):
        b = ctypes.create_string_buffer(n)
        r = ctypes.c_size_t()
        if not k.ReadProcessMemory(self.hp, ctypes.c_void_p(a), b, n, ctypes.byref(r)):
            return None
        return b.raw[:r.value]

    def wr(self, a, data):
        r = ctypes.c_size_t()
        k.WriteProcessMemory(self.hp, ctypes.c_void_p(a), data, len(data), ctypes.byref(r))
        k.FlushInstructionCache(self.hp, ctypes.c_void_p(a), len(data))

    def u8(self, a):
        b = self.rd(a, 1)
        return b[0] if b else 0

    def s8(self, a):
        return struct.unpack('b', self.rd(a, 1))[0]

    def u16(self, a):
        return struct.unpack('<H', self.rd(a, 2))[0]

    def s16(self, a):
        return struct.unpack('<h', self.rd(a, 2))[0]

    def u32(self, a):
        b = self.rd(a, 4)
        return struct.unpack('<I', b)[0] if b else 0

    def cstr(self, a, n=200):
        return (self.rd(a, n) or b'').split(b'\0')[0].decode('latin1')

    def objname(self, o):
        """Name of a compiler Object (function, variable, @temporary)."""
        if not o:
            return ''
        nm = self.u32(o + NAME_OFF)
        return self.cstr(nm + NAMESTR_OFF) if nm else '?'

    def opname(self, op):
        """PCode opcode mnemonic."""
        if not self._ops:
            for i in range(520):
                ent = OPTAB + i * 0x18
                nmp, idx = self.u32(ent), self.u32(ent + 4)
                if 0x690000 < nmp < 0x6C0000 and idx < 1000 and idx not in self._ops:
                    self._ops[idx] = self.cstr(nmp, 20)
        return self._ops.get(op, '?%d' % op)

    def pcode(self, instr):
        """(mnemonic, [operand strings], flags) of one PCode instruction (list link at +0, opcode at
        +0x28, operand count at +0x2A, 0xE-byte operands from +0x2C; flags at +0xC)."""
        raw = self.rd(instr, 0x2C)
        op, cnt = struct.unpack_from('<h', raw, 0x28)[0], struct.unpack_from('<h', raw, 0x2A)[0]
        fl = struct.unpack_from('<I', raw, 0xC)[0]
        ops = []
        for j in range(min(cnt, 8)):
            o = self.rd(instr + 0x2C + j * 0xE, 0xE)
            kind, kc = o[0], o[1]
            if kind == 0:
                ops.append('%s%d' % ({4: 'r', 1: 'cr', 3: 'f'}.get(kc, 'k%d_' % kc), struct.unpack_from('<h', o, 4)[0]))
            elif kind == 2:
                ops.append('#%x' % (struct.unpack_from('<i', o, 2)[0] & 0xFFFFFFFF))
            elif kind == 4:
                ops.append('M%s' % o[2:].hex())
            else:
                ops.append('<%d:%s>' % (kind, o[2:].hex()))
        return self.opname(op), ops, fl


def esp(ctx):
    return struct.unpack_from('<I', ctx, CTX_ESP)[0]


def arg(p, ctx, n):
    """n-th (1-based) stack argument at a function's first instruction (cdecl: [esp+4*n])."""
    return p.u32(esp(ctx) + 4 * n)


def run(argv, handlers, cwd=None):
    """Run the compile under the debugger, calling handlers[address](proc, ctx) at each hit.
    Returns the compiler's exit code."""
    if k is None:
        raise SystemExit('dbgcore needs Windows (it debugs the Win32 compiler binary)')
    if argv[0].lower().endswith('sjiswrap.exe'):
        argv = argv[1:]  # debug the compiler itself, not the wrapper
    si, pi = _SI(), _PI()
    si.cb = ctypes.sizeof(_SI)
    if not k.CreateProcessW(None, ctypes.create_unicode_buffer(subprocess.list2cmdline(argv)), None, None, False, 2,
                            None, cwd, ctypes.byref(si), ctypes.byref(pi)):
        raise SystemExit('CreateProcess failed %d' % ctypes.get_last_error())
    p = Proc(pi.hp)
    saved, stepping, armed = {}, {}, False
    exit_code = W.DWORD(0)
    de = _DE()
    while k.WaitForDebugEvent(ctypes.byref(de), 0xFFFFFFFF):
        status = 0x00010002  # DBG_CONTINUE
        if de.code == 1:  # exception
            code, = struct.unpack_from('<I', bytes(de.u), 0)
            addr, = struct.unpack_from('<Q', bytes(de.u), 16)
            if code in (0x80000003, 0x4000001F):  # breakpoint (native / WOW64)
                if not armed and addr not in saved:
                    armed = True  # the loader's initial breakpoint: arm ours
                    for a in handlers:
                        saved[a] = p.rd(a, 1)
                        p.wr(a, b'\xcc')
                elif addr in saved:
                    ht = k.OpenThread(0x1FFFFF, False, de.tid)
                    ctx = ctypes.create_string_buffer(CTX_SIZE)
                    struct.pack_into('<I', ctx, 0, 0x10007)
                    k.Wow64GetThreadContext(W.HANDLE(ht), ctx)
                    handlers[addr](p, ctx)
                    p.wr(addr, saved[addr])                     # restore, single-step, re-arm
                    struct.pack_into('<I', ctx, CTX_EIP, addr)
                    fl, = struct.unpack_from('<I', ctx, CTX_EFLAGS)
                    struct.pack_into('<I', ctx, CTX_EFLAGS, fl | 0x100)
                    k.Wow64SetThreadContext(W.HANDLE(ht), ctx)
                    stepping[de.tid] = addr
                    k.CloseHandle(ht)
            elif code in (0x80000004, 0x4000001E) and de.tid in stepping:
                p.wr(stepping.pop(de.tid), b'\xcc')
            else:
                status = 0x80010001  # DBG_EXCEPTION_NOT_HANDLED
        elif de.code == 5:  # process exit
            exit_code = struct.unpack_from('<I', bytes(de.u), 0)[0]
            k.ContinueDebugEvent(de.pid, de.tid, status)
            break
        k.ContinueDebugEvent(de.pid, de.tid, status)
    return exit_code
