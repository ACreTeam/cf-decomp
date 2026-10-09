"""Why is this local / temporary at that stack offset? (Windows only)

Runs the real GC 3.0a5.2 compiler under the debugger and prints the stack frame of each matching function:
every object that got a slot, its final r1 offset and size, what it is (named local, argument, @ temporary
and who made it, codegen temporary and the statement it was made for), and the facts that decide its place.

How 3.0 lays out locals (traced in mwcceppc.exe; see notes/mwcc_ghidra.md 1b):
  1. Arguments that live in memory get slots first (FUN_005878C0).
  2. Locals and @ temps that are not in registers (FUN_0058A3F0) are handed out in SIZE PASSES: first every
     object of size <= 1, then <= 2, <= 4, ... <= 512, then the rest. Within a pass, in the order of the
     function's locals list, which is REVERSE creation order (each new local / @ temp is prepended):
     so among same-size objects the one created last (declared last, or made later by the inliner)
     gets the lowest offset.
  3. Temporaries made during code generation (struct copies, spills: FUN_0046B9C0 / FUN_0046BB30) and by
     the back-end optimizer come after, in creation order.
  4. At FinalizeLayout (00586AF0) the frame is laid out again in the same order, keeping only objects that
     some instruction still addresses: an object whose every access was optimized away takes no space.
  Offsets grow from r1 + linkage (8) + outgoing argument area.

    python tools/mwcc/stackslots.py src/dol/game/d_field_assessment.cpp "^collectEggs$"
    python tools/mwcc/stackslots.py <src> <func regex> --input variant.cpp -I overlay_dir
    python tools/mwcc/stackslots.py <src> <func regex> --all     # also the objects dropped at the end

GC 3.0a5.2 addresses: slot allocator 0058A4E0 (arg Object*; first pass while byte 006E5D66 == 0, the final
re-layout with it set); the slot is s32 obj+0x48; local Object info = obj+0x40 (byte +9 = addressed by
some instruction, set at 00586AF0; byte +0xE bit 0x20 = has a slot). Frame: 006E5E34 linkage size (8 when
the function has a frame), 006E5E30 outgoing argument area, 006E5E28 local data size, 006E5E40 max
alignment. Locals list walked by 0058A3F0 (arg = list: next +0, Object* +4). FinalizeLayout 00589EB0;
the tool reads the result at 00589600 (StackFrameEABI_GeneratePrologueEpilogue, called right after).
"""
import argparse
import os
import re
import shutil
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dbgcore  # noqa: E402
import mwcc_cmd  # noqa: E402
from enode import Dumper  # noqa: E402
from mwcc_hooks import TempTracker  # noqa: E402

BP_GENERATOR = 0x468750
BP_SLOT = 0x58A4E0
BP_LOCALS = 0x58A3F0
BP_AFTER_LAYOUT = 0x589600
CUR_STMT = 0x7109AC
FIRST_PASS_FLAG = 0x6E5D66
LINKAGE, OUTGOING = 0x6E5E34, 0x6E5E30

ORIGIN = {
    'FUN_005878c0': 'argument kept in memory',
    'FUN_0058a3f0': 'local',
    'FUN_0046b9c0': 'codegen temporary',
    'FUN_0046bb30': 'codegen temporary (reused per type)',
    'FUN_00586a30': 'varargs area',
    'FUN_006206f0': 'back-end optimizer temporary',
}


def size_pass(size):
    b = 1
    while b < 0x400:
        if size <= b:
            return '<=%d' % b
        b *= 2
    return 'rest'


def main():
    if sys.platform != 'win32':
        raise SystemExit('stackslots.py debugs the Windows compiler binary: run it on Windows')
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--all', action='store_true', help='also list objects dropped from the final frame')
    a = ap.parse_args()
    fpat = re.compile(a.func)

    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'stackslots', str(os.getpid()))
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)
    tt = TempTracker()
    st = {'func': '', 'match': False, 'slots': [], 'list': {}, 'out': 0}

    def on_generator(p, ctx):
        fo = p.u32(dbgcore.arg(p, ctx, 1) + 4)
        st['func'] = p.objname(fo) if fo else '__sinit'
        st['match'] = bool(fpat.search(st['func']))
        st['slots'], st['list'] = [], {}
        tt.at_generator()
        for ad in (BP_SLOT, BP_LOCALS, BP_AFTER_LAYOUT):
            (p.arm if st['match'] else p.disarm)(ad)

    def on_locals(p, ctx):
        node, objs = dbgcore.arg(p, ctx, 1), []
        while node and len(objs) < 10000:
            objs.append(p.u32(node + 4))
            node = p.u32(node)
        n = len(objs)
        # list position 0 = head = created last; creation rank 1 = created first
        st['list'] = {o: (i, n - i) for i, o in enumerate(objs)}

    def on_slot(p, ctx):
        if p.u8(FIRST_PASS_FLAG):
            return       # the final re-layout: read at BP_AFTER_LAYOUT instead
        obj = dbgcore.arg(p, ctx, 1)
        info = p.u32(obj + 0x40) if p.u8(obj + 2) == 1 else 0
        if info and p.u8(info + 0xE) & 0x20:
            return       # already has a slot (the allocator returns at once)
        caller = dbgcore.funcname(p.u32(dbgcore.esp(ctx)))
        stmt = p.u32(CUR_STMT)
        line = struct.unpack('<i', p.rd(stmt + 0x1E, 4))[0] if stmt else None
        st['slots'].append({'obj': obj, 'caller': caller, 'line': line, 'seq': len(st['slots'])})

    def on_after_layout(p, ctx):
        if not st['match']:
            return
        d = Dumper(p)
        base = p.u32(LINKAGE) + p.u32(OUTGOING)
        rows = []
        for s in st['slots']:
            obj = s['obj']
            name = p.objname(obj)
            t = p.u32(obj + 0x10)
            size = p.s32(t + 2) if t else 0
            info = p.u32(obj + 0x40) if p.u8(obj + 2) == 1 else 0
            used = bool(info and p.u8(info + 9))
            off = base + p.s32(obj + 0x48)
            origin = ORIGIN.get(s['caller'], s['caller'])
            if origin == 'local':
                if name.startswith('@') or name.startswith('$'):
                    why = tt.describe(obj)
                    origin = '@ temporary' + (' (%s)' % why if why else
                                              ' (not from create_temp_object: probably an inlined function\'s '
                                              'local or parameter copy, made by the inliner)')
                else:
                    origin = 'named local'
            elif s['line'] is not None and 'codegen' in origin:
                origin += ' for the statement at line %d' % s['line']
            pos = st['list'].get(obj)
            rows.append((off if used else 1 << 30, off, used, size, name, d.type_name(t), origin, pos, s['seq']))
        rows.sort(key=lambda r: (r[0], r[8]))
        print('=== %s: stack objects (r1 offsets; locals start at r1+0x%X = linkage + outgoing args)' % (st['func'], base))
        print('  %-7s %-5s %-24s %-28s %-7s %-9s %s' % ('offset', 'size', 'object', 'type', 'pass', 'created', 'what'))
        for _, off, used, size, name, tname, origin, pos, seq in rows:
            if not used and not a.all:
                continue
            created = '#%d' % pos[1] if pos else '-'
            print('  %-7s %-5d %-24s %-28s %-7s %-9s %s' % (
                ('0x%X' % off) if used else 'dropped', size, name[:24], tname[:28],
                size_pass(size) if pos else '-', created, origin))
        dropped = sum(1 for r in rows if not r[2])
        if dropped and not a.all:
            print('  (%d more objects got a slot first but no instruction addresses them any more: --all lists them)' % dropped)
        print('  pass = the size pass that placed a local; created = creation rank in the locals list (#1 = made first).')
        print('  Within a pass the object created LAST gets the LOWEST offset.')
        print()
        st['out'] += 1

    handlers = {BP_GENERATOR: on_generator}
    handlers.update(tt.handlers())
    lazy = {BP_SLOT: on_slot, BP_LOCALS: on_locals, BP_AFTER_LAYOUT: on_after_layout}
    try:
        code = dbgcore.run(argv, handlers, cwd=mwcc_cmd.ROOT, lazy=lazy)
    finally:
        shutil.rmtree(outdir, ignore_errors=True)
    if not st['out']:
        print('no function matched %r (compiler exit code %d)' % (a.func, code), file=sys.stderr)


if __name__ == '__main__':
    main()
