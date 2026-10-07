"""Watch MWCC's peephole passes change one function's PCode (Windows only).

For each peephole pass (and the PCode optimizer passes around them) that runs on a function whose name
matches, this prints a compact diff of the function's PCode before and after the pass, per block:
removed `-`, added `+`, changed `~` instructions. For the forward peephole (Peephole_OptimizeBlocks) it
also prints every rule that fired: the rule's handler (address, plus a name where known), the instruction
it ran on, and what it changed in that block. Passes that change nothing are not printed (use --all).

    python tools/mwcc/peephole.py src/d_fish_fieldNP/d_fish_field.cpp "^executeRelease$"
    python tools/mwcc/peephole.py <src> <func regex> --input variant.cpp -I overlay_dir --pass "peep"
    python tools/mwcc/peephole.py <src> <func regex> --full      # also the whole listing before each pass
    python tools/mwcc/peephole.py <src> <func regex> --rules     # the rule table each forward pass uses

Registers print as rN (physical) or vN (virtual, before register allocation; vfN for FPRs); a trailing
`cr0` operand marks a record form (`extsh.`). Blocks are numbered B0.. in list order at each pass start.

Pass ids (--pass is a regex on them), in the order they run for one function at -O4:
  gopt-merge    Peephole_MergeAdjacentBlocks(f, 0)                      [COptimizer_Optimize]
  gopt-fwd0     Peephole_VisitBlocksWithMultipleInstructions(f, 0)      forward block pass: rlwinm/extsh/
                addi folding and store->load forwarding (0059A060 per block; 1.2.5 optimize_rlwinm_and_addi)
  opt-copy      COpt_CopyPropagation(f, mode) (00621D00). Replaces uses of `mr vA, rB` copies by rB when rB
                is not redefined in between; the mr then dies. THIS is the `mr vA,r3; extsh vB,vA` ->
                `extsh vB,r3` fusion: an `addi r3` between them (argument placement) blocks it.
  gopt-peep     Peephole_OptimizeBlocks, pre-RA, basic rule set
  opt-vn        value numbering (005976E0; 1.2.5 ValueNumbering_PerformValueNumbering); also runs post-RA
  opt-add       COpt_AddPropagation (00622070)
  lvl-merge, lvl-fwd   the merge / forward pass again, inside the -O3/-O4 pipeline (005966F0/005964C0)
  gopt-fwd1     Peephole_VisitBlocksWithMultipleInstructions(f, 1)
  pre-merge     Peephole_MergeAdjacentBlocks(f, 0) before the pre-RA scheduler       [CodeGen_Generator]
  pre-fwd       Peephole_VisitBlocksWithMultipleInstructions(f, 1) after the pre-RA scheduler
  pre-peep      Peephole_OptimizeBlocks, pre-RA, extended rule set (00725F42 = 1: + bt/bf, isel, addi rules)
  pre-dead      dead-instruction removal (00596D70) right before register allocation
  post-dead     dead-instruction removal right after register allocation
  post-merge    Peephole_MergeAdjacentBlocks(f, 1) after prologue/epilogue generation
  post-peep     Peephole_OptimizeBlocks, post-RA, full rule set (physical registers)

How the forward peephole works (1.2.5 Peephole.c framework; the rules themselves are 3.0 code at
00633000-00638600): register_peephole_rules (006325D0) fills per-opcode handler lists {next, func} at
006E6110 with AddPeepholeRule (00596A90, prepends, so a list runs in reverse registration order). The
pre-RA set is chosen while the virtual-register flag 00710394 is set. For each block, Peephole_OptimizeBlocks
(00596850) repeats build_reaching_def_table (00596C40) + peephole_optimize_block (00596AC0) until nothing
changes. peephole_optimize_block walks the block backwards: an instruction whose results are dead
(LiveRegisters 00621260) is unlinked, otherwise each handler for its opcode is called (`call [esi+4]` at
00596B11, EBX = instruction). A handler that returns nonzero restarts the list. Some handlers rewrite an
operand and still return 0; those are reported as "(returned 0)".
Addresses (GC 3.0a5.2): current function object 007109B4, first PCode block 007108C8, pre-RA flag 00710394,
rule lists 006E6110, rule call/return 00596B11/00596B14, dead unlink 00596B47. Pass call sites are in
CALLS below (CodeGen_Generator 00468750, COptimizer_Optimize 00596400, level-4 pipeline 005964C0, level-3
pipeline 005966F0). Each pass is bracketed by a breakpoint on its call and one on the return address.
"""
import argparse
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dbgcore  # noqa: E402
import mwcc_cmd  # noqa: E402

CUR_FUNC = 0x7109B4       # Object* of the function being generated (CodeGen_Generator's local_14)
PCODE_BLOCKS = 0x7108C8   # first PCode block: next +0, first instr +0x14, last instr +0x18
PRE_RA_FLAG = 0x710394    # nonzero while code still uses virtual registers
RULES = 0x6E6110          # per-opcode peephole handler lists {next, func}
BP_PEEPBLOCK = 0x596AC0   # peephole_optimize_block(block)
BP_RULE_CALL, BP_RULE_RET = 0x596B11, 0x596B14   # call [esi+4] (EBX = instr, ESI = list node) / return
BP_DEAD = 0x596B47        # call PCode_UnlinkInstruction(instr) for a dead instruction (EBX = instr)
RELOC = {4: '', 6: '@l', 8: '@ha', 7: '@h', 1: '@sda21'}   # memory operand relocation kinds
EAX, EBX, ESI = 0xB0, 0xA4, 0xA0   # WOW64_CONTEXT register offsets

_P = {   # pass id -> description
    'gopt-merge': 'Peephole_MergeAdjacentBlocks(0) [COptimizer_Optimize]',
    'gopt-fwd0': 'Peephole_VisitBlocksWithMultipleInstructions(0) [COptimizer_Optimize]',
    'gopt-peep': 'Peephole_OptimizeBlocks, pre-RA basic rules [COptimizer_Optimize]',
    'gopt-fwd1': 'Peephole_VisitBlocksWithMultipleInstructions(1) [COptimizer_Optimize]',
    'lvl-merge': 'Peephole_MergeAdjacentBlocks(0) [-O3/-O4 pipeline]',
    'lvl-fwd': 'Peephole_VisitBlocksWithMultipleInstructions(0) [-O3/-O4 pipeline]',
    'pre-merge': 'Peephole_MergeAdjacentBlocks(0) before the pre-RA scheduler [CodeGen_Generator]',
    'pre-fwd': 'Peephole_VisitBlocksWithMultipleInstructions(1) [CodeGen_Generator]',
    'pre-peep': 'Peephole_OptimizeBlocks, pre-RA extended rules [CodeGen_Generator]',
    'pre-dead': 'dead-instruction removal before register allocation [CodeGen_Generator]',
    'post-dead': 'dead-instruction removal after register allocation [CodeGen_Generator]',
    'post-merge': 'Peephole_MergeAdjacentBlocks(1) after prologue/epilogue [CodeGen_Generator]',
    'post-peep': 'Peephole_OptimizeBlocks, post-RA full rules [CodeGen_Generator]',
    'opt-copy': 'COpt_CopyPropagation (00621D00)',
    'opt-add': 'COpt_AddPropagation (00622070)',
    'opt-vn': 'value numbering (005976E0)',
}
# call site -> pass id. The "after" breakpoint is the call's return address (call + 5).
CALLS = {
    0x59641A: 'gopt-merge', 0x596424: 'gopt-fwd0', 0x59643D: 'gopt-peep', 0x59647F: 'gopt-fwd1',
    0x5965BD: 'lvl-merge', 0x5965C7: 'lvl-fwd', 0x5967C0: 'lvl-merge', 0x5967CA: 'lvl-fwd',
    0x4693A8: 'pre-merge', 0x4695E9: 'pre-fwd', 0x4695FD: 'pre-peep',
    0x4693D6: 'pre-dead', 0x4693F3: 'post-dead', 0x469537: 'post-merge', 0x469542: 'post-peep',
}
for _a in (0x59642E, 0x596489, 0x59649D, 0x5964D6, 0x59654F, 0x596569, 0x596591, 0x5965F8, 0x596602,
           0x596659, 0x596683, 0x5966B5, 0x5966C4, 0x596706, 0x59676D, 0x596785, 0x59679F, 0x596808):
    CALLS[_a] = 'opt-copy'
for _a in (0x5964A5, 0x5964E8, 0x596557, 0x596599, 0x596718, 0x59678D, 0x596838):
    CALLS[_a] = 'opt-add'
for _a in (0x596494, 0x5964C8, 0x5965D0, 0x5966E1, 0x5966F8, 0x5967D3, 0x469415):
    CALLS[_a] = 'opt-vn'

# Rule handlers named from their behaviour (read in Ghidra and seen firing); others print as addresses.
RULE_NAMES = {
    0x635920: 'mr_retarget_def',          # `op vA,...; mr vB,vA` -> `op vB,...` (1.2.5 retarget_reaching_def_register)
    0x6380D0: 'cmpi0_to_record_form',     # `op rA; cmpwi cr0,rA,0` -> `op. rA` (1.2.5 make_record_form_and_unlink_instruction)
    0x637E10: 'branch_cmp0_to_record_form',  # pre-RA: `op vA; cmpwi crN,vA,0; bc crN` -> `op. vA; bc cr0`
    0x636CC0: 'lis_addi_same_reg',        # pre-RA: `lis vA,x@ha; addi vB,vA,x@l` -> both use vB
    0x6368C0: 'load_base_through_mr',     # load/store base defined by `mr rA,rB`: use rB (1.2.5 fold_addi_or_mr_reaching_def, mr branch; returns 0)
    0x637170: 'indexed_to_offset',        # `li r0,d; psq_stx f,r1,r0` -> `psq_st f,d(r1)` (opcodes 404..409)
}


def rule_name(fn):
    return '%08X %s' % (fn, RULE_NAMES[fn]) if fn in RULE_NAMES else '%08X' % fn


def reg(cls, n, pre):
    pfx = {4: 'r', 3: 'f', 1: 'cr'}.get(cls, 'k%d_' % cls)
    if pre and cls in (3, 4) and n >= 32:
        pfx = {4: 'v', 3: 'vf'}[cls]
    return '%s%d' % (pfx, n)


def fmt_instr(p, instr, pre, labels):
    """One PCode instruction as text. Operands are 0xE bytes from +0x2C: kind, class/reloc, then
    register (+4), immediate (+2), memory offset (+2) and Object* (+6), or label (+2, block at label+4)."""
    raw = p.rd(instr, 0x2C + 8 * 0xE)
    op, cnt = struct.unpack_from('<h', raw, 0x28)[0], struct.unpack_from('<h', raw, 0x2A)[0]
    ops = []
    for j in range(min(cnt, 8)):
        o = raw[0x2C + j * 0xE: 0x2C + (j + 1) * 0xE]
        kind, kc = o[0], o[1]
        if kind == 0:
            ops.append(reg(kc, struct.unpack_from('<h', o, 4)[0], pre))
        elif kind == 2:
            ops.append('%d' % struct.unpack_from('<i', o, 2)[0])
        elif kind == 4:
            off, obj = struct.unpack_from('<iI', o, 2)
            nm = p.objname(obj) if obj else '?'
            ops.append('%s%s%s' % (nm, ('%+d' % off) if off else '', RELOC.get(kc, '@%d' % kc)))
        elif kind == 6:
            lab = struct.unpack_from('<I', o, 2)[0]
            blk = p.u32(lab + 4) if lab else 0
            ops.append(labels.get(blk, 'L%X' % lab))
        elif kind == 8 and not any(o[1:]):
            continue      # empty trailing operand (e.g. on mr)
        else:
            ops.append('<%d:%s>' % (kind, o[1:].hex()))
    return '%-8s %s' % (p.opname(op), ', '.join(ops))


def block_instrs(p, blk, pre, labels):
    res, i = [], p.u32(blk + 0x14)
    while i and len(res) < 5000:
        res.append((i, fmt_instr(p, i, pre, labels)))
        i = p.u32(i)
    return res


def snapshot(p, labels=None):
    """([(block, [(instr, text)])], labels) for the whole function."""
    pre = p.u8(PRE_RA_FLAG) != 0
    blocks, b = [], p.u32(PCODE_BLOCKS)
    while b and len(blocks) < 5000:
        blocks.append(b)
        b = p.u32(b)
    if labels is None:
        labels = {blk: 'B%d' % i for i, blk in enumerate(blocks)}
    else:
        labels = dict(labels)
        for blk in blocks:
            labels.setdefault(blk, 'Bnew%X' % blk)
    return [(blk, block_instrs(p, blk, pre, labels)) for blk in blocks], labels


def diff_lists(before, after, all_before=None, all_after=None):
    """'-', '+', '~' lines between two [(instr, text)] lists, keyed by instruction pointer."""
    all_before = all_before if all_before is not None else dict(before)
    all_after = all_after if all_after is not None else dict(after)
    lines = []
    for ptr, t in before:
        if ptr not in all_after:
            lines.append('    - %s' % t)
        elif all_after[ptr] != t:
            lines.append('    ~ %-34s -> %s' % (t, all_after[ptr]))
    for ptr, t in after:
        if ptr not in all_before:
            lines.append('    + %s' % t)
    return lines


def diff_snap(before, after, labels):
    """Per-block diff of two snapshots. Instructions moved to another block (merging) are not reported;
    removed blocks are."""
    out = []
    all_after = {ptr: t for _, ins in after for ptr, t in ins}
    all_before = {ptr: t for _, ins in before for ptr, t in ins}
    after_blocks = dict(after)
    for blk, ins in before:
        if blk not in after_blocks:
            out.append('  %s: block removed (merged)' % labels.get(blk, '?'))
        lines = diff_lists(ins, after_blocks.get(blk, []), all_before, all_after)
        if lines:
            out.append('  %s:' % labels.get(blk, '?'))
            out.extend(lines)
    before_blocks = dict(before)
    for blk, ins in after:
        if blk not in before_blocks:
            out.append('  %s: new block' % labels.get(blk, '?'))
            out.extend('    + %s' % t for _, t in ins)
    return out


def listing(snap, labels):
    out = []
    for blk, ins in snap:
        out.append('  %s:' % labels[blk])
        out.extend('      %s' % t for _, t in ins)
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--pass', dest='passes', default='.', help='regex on pass ids (default: all), e.g. "peep|copy"')
    ap.add_argument('--full', action='store_true', help='print the whole listing before each pass that changes something')
    ap.add_argument('--all', action='store_true', help='also report passes that change nothing')
    ap.add_argument('--rules', action='store_true', help='dump the forward peephole rule table per pass')
    a = ap.parse_args()
    if sys.platform != 'win32':
        raise SystemExit('peephole.py debugs the Windows compiler binary: run it on Windows')
    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    fpat, ppat = re.compile(a.func), re.compile(a.passes)
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'peephole')
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)

    st = {'site': None, 'fname': '', 'before': None, 'labels': {}, 'fires': [], 'cache': {}, 'call': None,
          'dead': None, 'pre': True, 'rules_seen': set()}

    def ctxreg(ctx, off):
        return struct.unpack_from('<I', ctx, off)[0]

    def on_call(site):
        def h(p, ctx):
            st['site'] = None
            fname = p.objname(p.u32(CUR_FUNC))
            if not ppat.search(CALLS[site]) or not fpat.search(fname):
                return
            snap, labels = snapshot(p)
            st.update({'site': site, 'fname': fname, 'before': snap, 'labels': labels, 'fires': [],
                       'cache': {}, 'pre': p.u8(PRE_RA_FLAG) != 0})
        return h

    def on_return(site):
        def h(p, ctx):
            if st['site'] != site:
                return
            st['site'] = None
            after, labels = snapshot(p, st['labels'])
            d = diff_snap(st['before'], after, labels)
            if not d and not st['fires'] and not a.all:
                return
            pid = CALLS[site]
            n0 = sum(len(i) for _, i in st['before'])
            n1 = sum(len(i) for _, i in after)
            print('=== %s: %s  %s, %s RA, call %08X (%d -> %d instructions, %d -> %d blocks)' % (
                st['fname'], pid, _P[pid], 'before' if st['pre'] else 'after', site, n0, n1,
                len(st['before']), len(after)))
            if a.full and (d or st['fires']):
                print(' before:')
                print('\n'.join(listing(st['before'], st['labels'])))
            if st['fires']:
                print(' rules fired (in order):')
                print('\n'.join(st['fires']))
                print(' net change:')
            print('\n'.join(d) if d else '  (no change)')
        return h

    def on_block(p, ctx):
        if not st['site']:
            return
        blk = dbgcore.arg(p, ctx, 1)
        st['cache'][blk] = block_instrs(p, blk, st['pre'], st['labels'])
        key = (CALLS[st['site']], st['pre'])
        if a.rules and key not in st['rules_seen']:
            st['rules_seen'].add(key)
            print('--- rule table for %s (opcode: handlers in call order)' % CALLS[st['site']])
            for opc in range(0x1F0):   # Peephole_OptimizeBlocks clears 0x1F0 lists
                node, hs = p.u32(RULES + 4 * opc), []
                while node and len(hs) < 50:
                    hs.append(rule_name(p.u32(node + 4)))
                    node = p.u32(node)
                if hs:
                    print('  %3d %-8s %s' % (opc, p.opname(opc), ' | '.join(hs)))

    def record(p, what, blk, at_text):
        before = st['cache'].get(blk, [])
        after = block_instrs(p, blk, st['pre'], st['labels']) if blk else []
        st['cache'][blk] = after
        st['fires'].append('  %s: [%s] at %s' % (st['labels'].get(blk, '?'), what, at_text))
        st['fires'].extend('  ' + ln for ln in diff_lists(before, after))

    def on_rule_call(p, ctx):
        if not st['site']:
            return
        instr = ctxreg(ctx, EBX)
        st['call'] = (instr, p.u32(ctxreg(ctx, ESI) + 4), p.u32(instr + 8),
                      fmt_instr(p, instr, st['pre'], st['labels']))

    def on_rule_ret(p, ctx):
        if not st['site'] or not st['call']:
            return
        instr, fn, blk, text = st['call']
        st['call'] = None
        if ctxreg(ctx, EAX):
            record(p, 'rule %s' % rule_name(fn), blk, text)
        elif p.u32(instr + 8) and fmt_instr(p, instr, st['pre'], st['labels']) != text:
            record(p, 'rule %s (returned 0)' % rule_name(fn), blk, text)

    def on_dead(p, ctx):
        if st['site']:
            instr = ctxreg(ctx, EBX)
            st['dead'] = (p.u32(instr + 8), fmt_instr(p, instr, st['pre'], st['labels']))

    def on_dead_done(p, ctx):
        if st['site'] and st['dead']:
            blk, text = st['dead']
            st['dead'] = None
            record(p, 'dead result (00621260)', blk, text)

    handlers = {BP_PEEPBLOCK: on_block, BP_RULE_CALL: on_rule_call, BP_RULE_RET: on_rule_ret,
                BP_DEAD: on_dead, BP_DEAD + 5: on_dead_done}
    for site in CALLS:
        handlers[site] = on_call(site)
        handlers[site + 5] = on_return(site)
    rc = dbgcore.run(argv, handlers, cwd=mwcc_cmd.ROOT)
    if rc:
        print('compiler exited with %d' % rc)


if __name__ == '__main__':
    main()
