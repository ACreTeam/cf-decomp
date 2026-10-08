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

Extras (used by vregs.py / iro_why.py):
  - lazy breakpoints: run(..., lazy={addr: handler}) registers handlers that start disarmed; a handler
    turns them on/off with p.arm(addr) / p.disarm(addr) (e.g. only while one function is compiled).
  - hardware write watchpoints: run(..., watch=[(addr, handler), ...]) (up to 4, 4-byte aligned).
    handler(proc, ctx) runs right after an instruction wrote the address (ctx's EIP is the next
    instruction). They are only active while p.watching is True (set it from a breakpoint handler).
  - callers(p, ctx): the return addresses on the stack, as (address, function name), innermost first
    (a heuristic stack scan: each candidate must follow a call instruction). funcname(addr) names any
    address from mwcc30_names.txt (the Ghidra project's function list).
"""
import bisect
import ctypes
import ctypes.wintypes as W
import os
import struct
import subprocess

k = ctypes.WinDLL('kernel32', use_last_error=True) if hasattr(ctypes, 'WinDLL') else None

CTX_SIZE = 716          # WOW64_CONTEXT
CTX_EIP, CTX_EFLAGS, CTX_ESP = 184, 192, 196
CTX_EAX, CTX_EBP = 176, 180
CTX_DR = (4, 8, 12, 16)  # Dr0..Dr3
CTX_DR6, CTX_DR7 = 20, 24
CTX_FLAGS = 0x10017     # CONTEXT_i386 | CONTROL | INTEGER | SEGMENTS | DEBUG_REGISTERS
TEXT_LO, TEXT_HI = 0x401000, 0x680000
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
        self.watching = False   # hardware watchpoints active (see run's `watch`)
        self._saved = {}        # breakpoint address -> original byte
        self._armed = set()

    def arm(self, a):
        """Enable a breakpoint registered with run(..., lazy=...)."""
        if a in self._saved and a not in self._armed:
            self._armed.add(a)
            self.wr(a, b'\xcc')

    def disarm(self, a):
        if a in self._armed:
            self._armed.discard(a)
            self.wr(a, self._saved[a])

    def s32(self, a):
        return struct.unpack('<i', self.rd(a, 4))[0]

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


def eip(ctx):
    return struct.unpack_from('<I', ctx, CTX_EIP)[0]


def eax(ctx):
    return struct.unpack_from('<I', ctx, CTX_EAX)[0]


def arg(p, ctx, n):
    """n-th (1-based) stack argument at a function's first instruction (cdecl: [esp+4*n])."""
    return p.u32(esp(ctx) + 4 * n)


_names = None


def _load_names():
    global _names
    if _names is None:
        starts, names = [], []
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'mwcc30_names.txt')
        if os.path.exists(path):
            for line in open(path):
                if line.startswith('#') or not line.strip():
                    continue
                a, n = line.split(None, 1)
                starts.append(int(a, 16))
                names.append(n.strip())
        _names = (starts, names)
    return _names


def funcname(addr, offset=False):
    """Name of the compiler function containing `addr` (nearest function start at or below it)."""
    starts, names = _load_names()
    i = bisect.bisect_right(starts, addr) - 1
    if i < 0:
        return '%08X' % addr
    return '%s+%X' % (names[i], addr - starts[i]) if offset else names[i]


def is_func_start(addr):
    starts, _ = _load_names()
    i = bisect.bisect_left(starts, addr)
    return i < len(starts) and starts[i] == addr


def funcstart(addr):
    starts, _ = _load_names()
    i = bisect.bisect_right(starts, addr) - 1
    return starts[i] if i >= 0 else 0


def callers(p, ctx, depth=0x1000, limit=24):
    """The call chain on the stack, innermost first: [(return address, function name)]. A stack word
    counts if it points into .text right after a call. A direct `call rel32` must target the function
    the chain is currently in (this drops stale words left by calls that already returned); an indirect
    call can't be checked and is accepted (name marked with '?')."""
    raw = p.rd(esp(ctx), depth) or b''
    cur = funcstart(eip(ctx))
    out = []
    for off in range(0, len(raw) - 3, 4):
        ra = struct.unpack_from('<I', raw, off)[0]
        if not TEXT_LO <= ra < TEXT_HI:
            continue
        pre = p.rd(ra - 7, 7)
        if not pre or len(pre) < 7:
            continue
        direct = indirect = False
        if pre[2] == 0xE8:  # call rel32
            direct = ((ra + struct.unpack_from('<i', pre, 3)[0]) & 0xFFFFFFFF) == cur
        elif pre[5] == 0xFF and (pre[6] & 0x38) == 0x10 and pre[6] >= 0xC0:          # call reg
            indirect = True
        elif pre[4] == 0xFF and (pre[5] & 0x38) == 0x10 and 0x40 <= pre[5] < 0x80:   # call [reg+d8]
            indirect = True
        elif pre[1] == 0xFF and (pre[2] & 0x38) == 0x10 and (pre[2] & 0xC7) in (0x05, 0x80, 0x81, 0x82, 0x83, 0x86, 0x87):
            indirect = True   # call [abs32] / call [reg+d32]
        elif pre[0] == 0xFF and pre[1] == 0x14:  # call [index*4 + d32]
            indirect = True
        if direct or indirect:
            out.append((ra, funcname(ra) + ('' if direct else '?')))
            cur = funcstart(ra)
            if len(out) >= limit:
                break
    return out


def run(argv, handlers, cwd=None, lazy=None, watch=None):
    """Run the compile under the debugger, calling handlers[address](proc, ctx) at each hit.
    lazy: more {address: handler} that start disarmed (p.arm / p.disarm). watch: [(address, handler)]
    hardware write watchpoints, active while p.watching. Handlers may modify ctx (it is written back).
    Returns the compiler's exit code."""
    if k is None:
        raise SystemExit('dbgcore needs Windows (it debugs the Win32 compiler binary)')
    if argv[0].lower().endswith('sjiswrap.exe'):
        argv = argv[1:]  # debug the compiler itself, not the wrapper
    lazy = {} if lazy is None else lazy   # may grow while running (register p._saved too)
    watch = list(watch or [])[:4]
    si, pi = _SI(), _PI()
    si.cb = ctypes.sizeof(_SI)
    if not k.CreateProcessW(None, ctypes.create_unicode_buffer(subprocess.list2cmdline(argv)), None, None, False, 2,
                            None, cwd, ctypes.byref(si), ctypes.byref(pi)):
        raise SystemExit('CreateProcess failed %d' % ctypes.get_last_error())
    p = Proc(pi.hp)
    saved, stepping, started = p._saved, {}, False
    exit_code = W.DWORD(0)
    de = _DE()

    def get_ctx(tid):
        ht = k.OpenThread(0x1FFFFF, False, tid)
        ctx = ctypes.create_string_buffer(CTX_SIZE)
        struct.pack_into('<I', ctx, 0, CTX_FLAGS)
        k.Wow64GetThreadContext(W.HANDLE(ht), ctx)
        return ht, ctx

    def put_ctx(ht, ctx):
        if watch:   # (re)load the debug registers on every stop, so p.watching takes effect
            dr7 = 0
            for i, (a, _) in enumerate(watch):
                struct.pack_into('<I', ctx, CTX_DR[i], a)
                if p.watching:
                    dr7 |= (1 << (2 * i)) | (0b1101 << (16 + 4 * i))  # local enable; RW=01 write, LEN=11 4 bytes
            struct.pack_into('<I', ctx, CTX_DR6, 0)
            struct.pack_into('<I', ctx, CTX_DR7, dr7)
        k.Wow64SetThreadContext(W.HANDLE(ht), ctx)
        k.CloseHandle(ht)

    while k.WaitForDebugEvent(ctypes.byref(de), 0xFFFFFFFF):
        status = 0x00010002  # DBG_CONTINUE
        if de.code == 1:  # exception
            code, = struct.unpack_from('<I', bytes(de.u), 0)
            addr, = struct.unpack_from('<Q', bytes(de.u), 16)
            if code in (0x80000003, 0x4000001F):  # breakpoint (native / WOW64)
                if not started and addr not in saved:
                    started = True  # the loader's initial breakpoint: arm ours
                    for a in list(handlers) + list(lazy):
                        saved[a] = p.rd(a, 1)
                    for a in handlers:
                        p.arm(a)
                elif addr in saved:
                    ht, ctx = get_ctx(de.tid)
                    if addr in p._armed:
                        (handlers.get(addr) or lazy[addr])(p, ctx)
                    if addr in p._armed:   # restore, single-step, re-arm
                        p.wr(addr, saved[addr])
                        stepping[de.tid] = addr
                        fl, = struct.unpack_from('<I', ctx, CTX_EFLAGS)
                        struct.pack_into('<I', ctx, CTX_EFLAGS, fl | 0x100)
                    struct.pack_into('<I', ctx, CTX_EIP, addr)
                    put_ctx(ht, ctx)
                else:
                    status = 0x80010001
            elif code in (0x80000004, 0x4000001E):  # single step / hardware watchpoint
                ht, ctx = get_ctx(de.tid)
                dr6, = struct.unpack_from('<I', ctx, CTX_DR6)
                if de.tid in stepping:
                    a = stepping.pop(de.tid)
                    if a in p._armed:
                        p.wr(a, b'\xcc')
                for i, (_, h) in enumerate(watch):
                    if dr6 & (1 << i):
                        h(p, ctx)
                put_ctx(ht, ctx)
            else:
                status = 0x80010001  # DBG_EXCEPTION_NOT_HANDLED
        elif de.code == 5:  # process exit
            exit_code = struct.unpack_from('<I', bytes(de.u), 0)[0]
            k.ContinueDebugEvent(de.pid, de.tid, status)
            break
        k.ContinueDebugEvent(de.pid, de.tid, status)
    return exit_code
