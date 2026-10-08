"""Watch MWCC's register allocator work on one function (Windows only).

A minimal Win32 debugger: it runs the real GC 3.0a5.2 compiler on a source file (with the unit's ninja
flags), stops in the graph-colouring register allocator, and prints, for each function whose name
matches:
  - the final assignment: virtual register -> physical register, flags, degree, spill cost and the source
    object (named local, argument, or @temporary) it belongs to
  - optionally the simplify input (--simplify), the colouring order with every node's neighbours
    (--select), and the PCode with virtual registers (--pcode)
  - optionally a re-simulation of simplify + select from the dumped graph (--sim), which reproduces the
    compiler's choices and lets you try other virtual-register orders offline (see simulate()).
  - with --want, a search over renumberings: which single move, swap (or, with --movers, joint placement)
    of virtual registers would give the registers you want. Each hit tells you which value must be
    numbered before or after which other one, which you then get from declaration order, temporaries etc.

    python tools/mwcc/regalloc.py <src> "^executeNibble$" --want "this=r28,fl=r29"
    python tools/mwcc/regalloc.py <src> "^create$" --want "heap=r25,v48=r22" --movers "heap,v44"

    python tools/mwcc/regalloc.py src/d_fish_fieldNP/d_fish_field.cpp "executeNibble"
    python tools/mwcc/regalloc.py <src> <func regex> --select --sim
    python tools/mwcc/regalloc.py <src> <func regex> --input variant.cpp -I overlay_dir --pcode

Only GPRs (register class 4) are reported for simplify/select. The rules these dumps revealed
(numbering, simplify with K=29, lowest-free colour selection, callee-saved r31 downwards) are written
up in notes/mwcc_ghidra.md section 1b. Addresses are for mwcceppc.exe GC 3.0a5.2 (Ghidra names in
notes/mwcc30_functions.csv): allocate_object_registers-adjacent entry 00596DF0,
Coloring_CommitAssignments 00596F90, Coloring_SelectColors 005970E0, Coloring_SimplifyGraph 00597300.
Originally built in the fish REL phase 4 (agent D).
"""
import argparse
import ctypes
import itertools
import ctypes.wintypes as W
import os
import re
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mwcc_cmd  # noqa: E402

# --- compiler internals (GC 3.0a5.2) ---
BP_ALLOC, BP_COMMIT, BP_SELECT, BP_SIMP = 0x596DF0, 0x596F90, 0x5970E0, 0x597300
NODES = 0x711AFC            # IGNode array base pointer (0x1E bytes per node)
REG_CLASS = 0x726060        # current register class (4 = GPR)
VREG_LO, VREG_HI = 0x710274, 0x716EA0   # per-class first / end virtual register
USED_PHYS = 0x70F2E0        # per-class 32-byte "physical register used" table
PCODE_BLOCKS = 0x7108C8     # first PCode basic block
OPTAB = 0x6A290C            # opcode info table (name pointer, index)
NAME_OFF, NAMESTR_OFF = 0xC, 0xA   # Object -> HashNameNode -> chars
CTX_SIZE = 716              # WOW64_CONTEXT
GPR_CLASS = 4

k = ctypes.WinDLL('kernel32', use_last_error=True)


class SI(ctypes.Structure):
    _fields_ = [('cb', W.DWORD), ('r', W.LPWSTR), ('d', W.LPWSTR), ('t', W.LPWSTR), ('x', W.DWORD * 8),
                ('f', W.WORD), ('s', W.WORD), ('r2', ctypes.c_void_p), ('h1', W.HANDLE), ('h2', W.HANDLE),
                ('h3', W.HANDLE)]


class PI(ctypes.Structure):
    _fields_ = [('hp', W.HANDLE), ('ht', W.HANDLE), ('pid', W.DWORD), ('tid', W.DWORD)]


class DE(ctypes.Structure):
    _fields_ = [('code', W.DWORD), ('pid', W.DWORD), ('tid', W.DWORD), ('pad', W.DWORD), ('u', ctypes.c_byte * 160)]


class Proc:
    def __init__(self, hp):
        self.hp = hp

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

    def u32(self, a):
        b = self.rd(a, 4)
        return struct.unpack('<I', b)[0] if b else 0

    def s16(self, a):
        return struct.unpack('<h', self.rd(a, 2))[0]

    def u16(self, a):
        return struct.unpack('<H', self.rd(a, 2))[0]

    def s8(self, a):
        return struct.unpack('b', self.rd(a, 1))[0]

    def cstr(self, a, n=200):
        return (self.rd(a, n) or b'').split(b'\0')[0].decode('latin1')

    def objname(self, o):
        if not o:
            return ''
        nm = self.u32(o + NAME_OFF)
        return self.cstr(nm + NAMESTR_OFF) if nm else '?'


def simulate(order, neighbours, volatile=(0,) + tuple(range(3, 13)), callee_saved=tuple(range(31, 13, -1))):
    """Re-run Coloring_SelectColors: `order` is the colouring order (first coloured first), `neighbours`
    maps vreg -> list of vregs. Returns {vreg: physical}. Each node takes the lowest free register; when
    none is free the next callee-saved register (r31 downwards) is claimed and joins the pool."""
    colors = sum(1 << r for r in volatile)
    phys = {r: r for r in range(32)}
    ci = 0
    for v in order:
        while True:
            avail = colors
            for n in neighbours.get(v, ()):
                p = phys.get(n, -1)
                if 0 <= p < 32:
                    avail &= ~(1 << p)
            if avail:
                phys[v] = min(r for r in range(32) if avail >> r & 1)
                break
            colors |= 1 << callee_saved[ci]
            ci += 1
    return phys


def simplify(order_asc, neighbours, K=29):
    """Re-run Coloring_SimplifyGraph: scan in ascending vreg order, in passes; a node is pushed once its
    current degree < K (removing it lowers its neighbours' degrees). Returns the colouring order."""
    deg = {v: len(neighbours[v]) for v in order_asc}
    done, stack = set(), []
    while True:
        removed, remaining = False, []
        for v in order_asc:
            if v in done:
                continue
            if deg[v] < K:
                for n in neighbours[v]:
                    if n in deg:
                        deg[n] -= 1
                done.add(v)
                stack.append(v)
                removed = True
            else:
                remaining.append(v)
        if not removed:
            if remaining:
                raise RuntimeError('a spill would be needed')
            break
    return stack[::-1]


MAX_MOVER_COMBOS = 2000000  # placements of the --movers set; more would run for hours


def search_numbering(asc, neighbours, want, movers=(), limit=30, max_combos=MAX_MOVER_COMBOS):
    """Which renumberings of the virtual registers would give the `want` assignment ({vreg: phys})?

    Re-runs simplify + select (exactly as the compiler does, see simplify/simulate) for:
      - every single node moved to every other position in the numbering,
      - every swap of two nodes' numbers,
      - with `movers`, every placement of those nodes together (the two-node insertion search that solved
        the fish's create).
    Returns [(kind, description, order)]. The numbering comes from the source (named locals in reverse
    declaration order, then @ temporaries, ...: see notes/mwcc_ghidra.md 1b), so each hit says which value
    has to be created earlier or later than which other one."""
    def ok(order):
        try:
            stack = simplify(order, neighbours)
        except RuntimeError:
            return False
        phys = simulate(stack, neighbours)
        return all(phys.get(v) == r for v, r in want.items())

    hits = []
    n = len(asc)
    for v in asc:
        rest = [x for x in asc if x != v]
        for p in range(len(rest) + 1):
            order = rest[:p] + [v] + rest[p:]
            if order != asc and ok(order):
                hits.append(('move', (v, rest[p - 1] if p else None, rest[p] if p < len(rest) else None), order))
    for i in range(n):
        for j in range(i + 1, n):
            order = list(asc)
            order[i], order[j] = order[j], order[i]
            if ok(order):
                hits.append(('swap', (asc[i], asc[j]), order))
    if movers:
        rest = [x for x in asc if x not in movers]
        combos = (len(rest) + 1) ** len(movers)
        if combos > max_combos:
            print('  --movers: %d nodes over %d positions = %d placements (limit %d); skipped. Use 2-3 movers,'
                  % (len(movers), len(rest) + 1, combos, max_combos))
            print('  or raise the limit with --max-combos (each placement is one colouring simulation).')
            return hits
        for places in itertools.product(range(len(rest) + 1), repeat=len(movers)):
            order = list(rest)
            for p, v in sorted(zip(places, movers), key=lambda t: -t[0]):
                order.insert(p, v)
            if order != asc and ok(order):
                hits.append(('movers', tuple((v, order[order.index(v) - 1] if order.index(v) else None) for v in movers), order))
            if len(hits) > 5000:
                break
    return hits


def main():
    if sys.platform != 'win32':
        raise SystemExit('regalloc.py debugs the Windows compiler binary: run it on Windows')
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--simplify', action='store_true', help='print the simplify input (degrees)')
    ap.add_argument('--select', action='store_true', help='print the colouring order and neighbours')
    ap.add_argument('--pcode', action='store_true', help='print the PCode with virtual registers')
    ap.add_argument('--sim', action='store_true', help='re-simulate select from the dumped graph (implies --select)')
    ap.add_argument('--all-classes', action='store_true', help='also print the FPR/other class assignments')
    ap.add_argument('--want', help='search: wanted registers, e.g. "this=r28,fl=r29,v48=r22" (object name or vNN)')
    ap.add_argument('--movers', help='search: also place these nodes together anywhere (names or vNN, comma separated); '
                    'the search grows as positions^movers, so keep it to 2-3 nodes')
    ap.add_argument('--max-combos', type=int, default=MAX_MOVER_COMBOS,
                    help='search: skip --movers when it would try more placements than this (default %(default)s)')
    ap.add_argument('--limit', type=int, default=30, help='search: how many hits to print per kind')
    ap.add_argument('--dump-graph', help='write each matching function GPR graph (colouring order, neighbours, '
                    'object names, real assignment) as JSON lines to this file, for offline searches')
    a = ap.parse_args()
    collect = a.select or a.sim or a.want or a.dump_graph
    fpat = re.compile(a.func)

    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'regalloc')
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)
    if argv[0].lower().endswith('sjiswrap.exe'):
        argv = argv[1:]  # debug the compiler itself (sjiswrap would be the debuggee otherwise)
    cmdline = subprocess.list2cmdline(argv)

    si, pi = SI(), PI()
    si.cb = ctypes.sizeof(SI)
    if not k.CreateProcessW(None, ctypes.create_unicode_buffer(cmdline), None, None, False, 2, None,
                            mwcc_cmd.ROOT, ctypes.byref(si), ctypes.byref(pi)):
        raise SystemExit('CreateProcess failed %d' % ctypes.get_last_error())
    p = Proc(pi.hp)
    bps, cur_func, stepping, graph = {}, [0], {}, {}

    def setbp(addr):
        bps[addr] = p.rd(addr, 1)
        p.wr(addr, b'\xcc')

    def neighbours(vr):
        n = p.u32(NODES) + vr * 0x1E
        me, e, out = p.s16(n + 0x10), p.u32(n + 0x1A), []
        while e:
            x, y = p.s16(e + 0xC), p.s16(e + 0xE)
            out.append(y if y > me else x)
            e = p.u32(e + 4) if me < y else p.u32(e + 8)
        return out

    def name():
        return p.objname(cur_func[0])

    def on_simplify(ctx):
        if not fpat.search(name()) or p.s8(REG_CLASS) != GPR_CLASS:
            return
        used = p.rd(USED_PHYS + GPR_CLASS * 0x20, 32)
        lo, hi, base = p.u32(VREG_LO + GPR_CLASS * 4), p.u32(VREG_HI + GPR_CLASS * 4), p.u32(NODES)
        print('=== simplify %s: free physical %s, first vreg %d' % (name(), [i for i in range(32) if used[i] == 0], lo))
        print('   ', ' '.join('v%d:%d%s' % (x, p.s16(base + x * 0x1E + 0x16), 'C' if p.u16(base + x * 0x1E + 0x14) & 4 else '')
                             for x in range(lo, hi)
                             if p.s16(base + x * 0x1E + 0x16) > 0 or p.u16(base + x * 0x1E + 0x14) & 4))

    def on_select(ctx):
        if not fpat.search(name()) or p.s8(REG_CLASS) != GPR_CLASS:
            return
        esp = struct.unpack_from('<I', ctx, 196)[0]
        node, order = p.u32(esp + 4), []
        while node:
            order.append(p.s16(node + 0x10))
            node = p.u32(node)
        nb = {x: neighbours(x) for x in order}
        if a.select:
            print('=== select %s: colouring order (first -> last)' % name())
            print('   ', ' '.join('v%d' % x for x in order))
            for x in order:
                print('   v%d (%d): %s' % (x, len(nb[x]), ' '.join(str(y) for y in sorted(nb[x]))))
        graph[name()] = (order, nb)

    def opname(op, cache={}):
        if not cache:
            for i in range(520):
                ent = OPTAB + i * 0x18
                nmp, idx = p.u32(ent), p.u32(ent + 4)
                if 0x690000 < nmp < 0x6C0000 and idx < 1000 and idx not in cache:
                    cache[idx] = p.cstr(nmp, 20)
        return cache.get(op, '?%d' % op)

    def dump_pcode():
        b, bn = p.u32(PCODE_BLOCKS), 0
        while b:
            print('  B%d:' % bn)
            bn += 1
            i = p.u32(b + 0x14)
            while i:
                raw = p.rd(i, 0x2C)
                op, cnt = struct.unpack_from('<h', raw, 0x28)[0], struct.unpack_from('<h', raw, 0x2A)[0]
                ops = []
                for j in range(min(cnt, 8)):
                    o = p.rd(i + 0x2C + j * 0xE, 0xE)
                    kind, kc = o[0], o[1]
                    reg, val = struct.unpack_from('<h', o, 4)[0], struct.unpack_from('<i', o, 4)[0]
                    ops.append('%s%d' % ({4: 'r', 1: 'f'}.get(kc, 'c%d_' % kc), reg) if kind == 0
                               else '<%d:%d:%x>' % (kind, kc, val & 0xFFFFFFFF))
                print('    %-8s %s' % (opname(op), ', '.join(ops)))
                i = p.u32(i)
            b = p.u32(b)

    def on_commit(ctx):
        if not fpat.search(name()):
            return
        cls = p.s8(REG_CLASS)
        if cls != GPR_CLASS and not a.all_classes:
            return
        lo, hi, base = p.u32(VREG_LO + cls * 4), p.u32(VREG_HI + cls * 4), p.u32(NODES)
        print('=== assignment %s, register class %d, vregs %d..%d' % (name(), cls, lo, hi - 1))
        if a.pcode and cls == GPR_CLASS:
            dump_pcode()
        for x in range(lo, hi):
            n = base + x * 0x1E
            vr, ph, fl, deg, ob, cost = (p.s16(n + 0x10), p.s16(n + 0x12), p.u16(n + 0x14), p.s16(n + 0x16),
                                         p.u32(n + 4), p.u32(n + 0xC))
            print('  v%-4d -> r%-3d flags=%04X degree=%-3d cost=%-6d %s' % (vr, ph, fl, deg, cost, p.objname(ob) if ob else ''))
        if a.dump_graph and name() in graph and cls == GPR_CLASS:
            import json
            g_order, g_nb = graph[name()]
            with open(a.dump_graph, 'a') as f:
                f.write(json.dumps({'func': name(), 'order': list(g_order),
                                    'nb': {str(k): list(v) for k, v in g_nb.items()},
                                    'names': {str(p.s16(base + x * 0x1E + 0x10)): p.objname(p.u32(base + x * 0x1E + 4)) or ''
                                              for x in range(lo, hi)},
                                    'real': {str(p.s16(base + x * 0x1E + 0x10)): p.s16(base + x * 0x1E + 0x12)
                                             for x in range(lo, hi)}}) + '\n')
        if a.sim and name() in graph and cls == GPR_CLASS:
            order, nb = graph[name()]
            sim = simulate(order, nb)
            real = {p.s16(base + x * 0x1E + 0x10): p.s16(base + x * 0x1E + 0x12) for x in range(lo, hi)}
            bad = [x for x in order if sim.get(x) != real.get(x)]
            print('  re-simulation: %s' % ('matches the compiler' if not bad else 'DIFFERS for %s' % bad))
        if a.want and name() in graph and cls == GPR_CLASS:
            run_search(name(), lo, hi, base)

    def run_search(fname, lo, hi, base):
        order, nb = graph[fname]
        names = {p.s16(base + x * 0x1E + 0x10): (p.objname(p.u32(base + x * 0x1E + 4)) or '') for x in range(lo, hi)}
        real = {p.s16(base + x * 0x1E + 0x10): p.s16(base + x * 0x1E + 0x12) for x in range(lo, hi)}
        label = lambda v: 'v%d' % v if v is not None else '(end)'
        named = lambda v: '%s%s' % (label(v), (' ' + names.get(v)) if v is not None and names.get(v) else '')

        def resolve(tok):
            tok = tok.strip()
            if re.match(r'v\d+$', tok):
                return int(tok[1:])
            hits = [v for v in order if names.get(v) == tok]
            if not hits:
                raise SystemExit('--want/--movers: no node named %r in the colouring graph of %s' % (tok, fname))
            if len(hits) > 1:
                print('  note: %r is %s; using %s' % (tok, ', '.join(label(h) for h in hits), label(hits[0])))
            return hits[0]
        want = {}
        for item in a.want.split(','):
            k_, r_ = item.split('=')
            want[resolve(k_)] = int(r_.strip().lstrip('r'))
        movers = [resolve(t) for t in a.movers.split(',')] if a.movers else []
        asc = sorted(order)
        try:
            base_ok = simulate(simplify(asc, nb), nb) == simulate(order, nb)
        except RuntimeError:
            base_ok = False
        print('=== register search %s: want %s' % (fname, ', '.join('%s=r%d' % (named(v), r) for v, r in want.items())))
        print('  now: %s' % ', '.join('%s=r%d' % (named(v), real.get(v, -1)) for v in want))
        if not base_ok:
            print('  WARNING: simplify(ascending numbering) does not reproduce the compiler order; hits may be unreliable')
        hits = search_numbering(asc, nb, want, movers, max_combos=a.max_combos)
        if not hits:
            print('  no single move, swap or mover placement gives that. Try --movers with the nodes you suspect.')
            return
        for kind in ('move', 'swap', 'movers'):
            hk = [h for h in hits if h[0] == kind]
            if not hk:
                continue
            print('  %d %s hit(s)%s' % (len(hk), kind, '' if len(hk) <= a.limit else ', first %d:' % a.limit))
            for _, d, _ in hk[:a.limit]:
                if kind == 'move':
                    v, before, after = d
                    print('    renumber %s between %s and %s' % (named(v), named(before) if before is not None else '(start)',
                                                              named(after)))
                elif kind == 'swap':
                    print('    swap the numbers of %s and %s' % (named(d[0]), named(d[1])))
                else:
                    print('    ' + '; '.join('%s right after %s' % (named(v), named(prev) if prev is not None else '(start)')
                                          for v, prev in d))
        print('  how numbering follows the source: arguments first (this = lowest), then named locals in REVERSE')
        print('  declaration order (declared later = lower number), then @ temporaries (inline results, CSE,')
        print('  strength reduction...) in reverse creation order. See notes/mwcc_ghidra.md 1b.')

    first = True
    de = DE()
    while k.WaitForDebugEvent(ctypes.byref(de), 0xFFFFFFFF):
        status = 0x00010002  # DBG_CONTINUE
        if de.code == 1:  # exception
            code, = struct.unpack_from('<I', bytes(de.u), 0)
            addr, = struct.unpack_from('<Q', bytes(de.u), 16)
            if code in (0x80000003, 0x4000001F):  # breakpoint (native / WOW64)
                if first and addr not in bps:
                    first = False
                    for bp in (BP_ALLOC, BP_COMMIT, BP_SELECT, BP_SIMP):
                        setbp(bp)
                elif addr in bps:
                    ht = k.OpenThread(0x1FFFFF, False, de.tid)
                    ctx = ctypes.create_string_buffer(CTX_SIZE)
                    struct.pack_into('<I', ctx, 0, 0x10007)
                    k.Wow64GetThreadContext(W.HANDLE(ht), ctx)
                    if addr == BP_ALLOC:
                        esp = struct.unpack_from('<I', ctx, 196)[0]
                        cur_func[0] = p.u32(esp + 4)
                    elif addr == BP_SELECT and collect:
                        on_select(ctx)
                    elif addr == BP_SIMP and a.simplify:
                        on_simplify(ctx)
                    elif addr == BP_COMMIT:
                        on_commit(ctx)
                    p.wr(addr, bps[addr])                       # restore, single-step, re-arm
                    struct.pack_into('<I', ctx, 184, addr)      # Eip
                    fl, = struct.unpack_from('<I', ctx, 192)
                    struct.pack_into('<I', ctx, 192, fl | 0x100)  # TF
                    k.Wow64SetThreadContext(W.HANDLE(ht), ctx)
                    stepping[de.tid] = addr
                    k.CloseHandle(ht)
            elif code in (0x80000004, 0x4000001E) and de.tid in stepping:  # single step done
                p.wr(stepping.pop(de.tid), b'\xcc')
            else:
                status = 0x80010001  # DBG_EXCEPTION_NOT_HANDLED
        elif de.code == 5:  # process exit
            k.ContinueDebugEvent(de.pid, de.tid, status)
            break
        k.ContinueDebugEvent(de.pid, de.tid, status)


if __name__ == '__main__':
    main()
