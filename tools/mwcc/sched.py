"""Watch MWCC's instruction scheduler on one function (Windows only).

For each basic block the scheduler handles in a matching function, this prints the block's PCode
(with each instruction's alias record, which decides memory dependencies), then the ready list at every
cycle: node, height, latency, latest cycle, and the number of successors it would make ready. The
scheduler runs twice, before and after register allocation; each block header says which pass.

    python tools/mwcc/sched.py src/d_fish_fieldNP/d_fish_field.cpp "^getMouthPos$"
    python tools/mwcc/sched.py <src> <func regex> --input variant.cpp -I overlay_dir --post-ra-only

Selection order (same as 1.2.5 Scheduler.c select_ready_coloring_node): critical path, then successors
made ready, then height, then opcode rank (pre-RA only), then list order. 3.0 takes memory dependencies
from the Alias.c query (005A0150 -> 005A0170) rather than 1.2.5's PtrOp lists. A load through a pointer to
non-const (including `this` in a non-const method) aliases every local whose address escaped to a call,
which can flip a tie-break; see notes/mwcc_ghidra.md 1b. Ported from the fish REL phase-5 debugger.
Addresses (GC 3.0a5.2): register allocator entry 00596DF0 (names the function: its pre-RA blocks run
just before it, so they are buffered until then), schedule_block 00599740, ready-select 00599920.
"""
import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dbgcore  # noqa: E402
import mwcc_cmd  # noqa: E402

BP_ALLOC, BP_BLOCK, BP_SELECT = 0x596DF0, 0x599740, 0x599920
PRE_RA_FLAG = 0x710394   # nonzero while code still uses virtual registers


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--post-ra-only', action='store_true', help='only the pass after register allocation')
    ap.add_argument('--pre-ra-only', action='store_true', help='only the pass before register allocation')
    ap.add_argument('--no-cycles', action='store_true', help='print the blocks only, not the ready lists')
    a = ap.parse_args()
    fpat = re.compile(a.func)
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'sched')
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)
    # pre-RA blocks of a function are scheduled before the allocator names it: buffer them until then
    state = {'func': '', 'active': False, 'ids': {}, 'pending': [], 'pre': True}

    def emit(text):
        if state['pre']:
            state['pending'].append(text)
        else:
            print(text)

    def on_alloc(p, ctx):
        state['func'] = p.objname(dbgcore.arg(p, ctx, 1))
        if fpat.search(state['func']) and not a.post_ra_only:
            for t in state['pending']:
                print(t.replace('<func>', state['func']))
        state['pending'] = []

    def alias_info(p, instr):
        al = p.u32(instr + 0x1C)
        if not al:
            return ''
        kind, obj = p.u8(al + 0x2C), p.u32(al + 0x10)
        s = 'alias%d:%s+%X/%d' % (kind, p.objname(obj) if obj else '-', p.u32(al + 0x14), p.u32(al + 0x18))
        return s

    def on_block(p, ctx):
        state['active'] = False
        pre = p.u8(PRE_RA_FLAG) != 0
        state['pre'] = pre
        if not pre and (not fpat.search(state['func']) or a.pre_ra_only):
            return
        if pre and a.post_ra_only:
            return
        block = dbgcore.arg(p, ctx, 1)
        instr, n, ids = p.u32(block + 0x14), 0, {}
        lines = []
        while instr:
            mn, ops, fl = p.pcode(instr)
            ids[instr] = n
            lines.append('  %2d %-8s %-36s fl=%08X %s' % (n, mn, ', '.join(ops), fl, alias_info(p, instr)))
            n += 1
            instr = p.u32(instr)
        if not lines:
            return
        state['active'], state['ids'] = True, ids
        emit('=== %s: block %08X, %s register allocation' % ('<func>' if pre else state['func'], block,
                                                             'before' if pre else 'after'))
        emit('\n'.join(lines))

    def on_select(p, ctx):
        if not state['active'] or a.no_cycles:
            return
        sp = dbgcore.esp(ctx)
        node, cyc = p.u32(sp + 4), p.u32(sp + 8) & 0xFFFF
        ids, out = state['ids'], []
        while node:
            if p.s16(node + 0x18) == 0 and p.u16(node + 0x12) <= cyc:   # not scheduled, earliest <= now
                c, succ, ready = p.u32(node + 8), [], 0
                while c:
                    o = p.u32(c + 4)
                    succ.append('%d/%d' % (ids.get(p.u32(o + 0xC), -1), p.u16(c + 8)))
                    if p.s16(o + 0x18) == 1:
                        ready += 1
                    c = p.u32(c)
                out.append('#%d h=%d lat=%d late=%d ready=%d succ=[%s]' % (
                    ids.get(p.u32(node + 0xC), -1), p.u16(node + 0x16), p.u16(node + 0x10), p.u16(node + 0x14),
                    ready, ' '.join(succ)))
            node = p.u32(node)
        emit('  cycle %d: %s' % (cyc, ' | '.join(out)))

    dbgcore.run(argv, {BP_ALLOC: on_alloc, BP_BLOCK: on_block, BP_SELECT: on_select}, cwd=mwcc_cmd.ROOT)


if __name__ == '__main__':
    main()
