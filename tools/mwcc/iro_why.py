"""What did the IR optimizer do to a function, and why? (reads the compiler's own IRO log)

iro_dump.py prints the raw flowgraph after every pass (hundreds of KB per function). This tool reads the
same log (patched compiler, see iro_dump.py) and turns it into answers:

  1. Variables: for every variable of the function, how many definitions / uses it has after each
     pass, the pass that removed its last use (propagated into an expression or dead), and for each
     @ temporary the pass that created it. A named local that is gone after the optimizer gets no
     register of its own: the value lives in a codegen temp of the expression it was propagated into
     (vregs.py shows that temp).
  2. Timeline: for each pass that changed the function, the statements it changed (- before / + after,
     rendered as C-like expressions), and the optimizer's own decision messages for that pass with
     their node numbers resolved to the statement they point at ("Found propagatable assignment at: 4
     -> diff = (id != getMemberHeldItem(...))", "Removing dead assignment ...", "Replacing common sub
     ...", "CanRemoveRedundantLoop:No due to ...", "Found induction variable ...", ...).

    python tools/mwcc/iro_why.py src/dol/game/d_field_assessment.cpp "^updateFlowersLaterDay$"
    python tools/mwcc/iro_why.py <src> <func regex> --var weekly,id      # only what touches these
    python tools/mwcc/iro_why.py <src> <func regex> --pass "Propagat|DeadStore|CommonSubs"
    python tools/mwcc/iro_why.py <src> <func regex> --input variant.cpp -I overlay_dir
    python tools/mwcc/iro_why.py <src> <func regex> --log saved.log   # reuse a log from iro_dump.py -o

Comparing two variants is the main use: run it on both and look for the first pass where they differ
(the variable summary usually shows it: "weekly: last use removed by IRO_ExpressionPropagation (tick 0)"
in one and not the other).

Dump grammar (GC 3.0a5.2 IroDump): "Flowgraph node N  First = a, Last = b", then one line per linear IR
node "  i: Kind operands <flags>": Operand <name|constant> (flags: assigned = written here, ind = used
through an EINDIRECT, ...), EINDIRECT i, binary ops "EADD i j", Funccall "f(arg, ...)", EASS lhs rhs,
If / IfNot cond @label, Goto @label, Label @label, Return [i], Switch i, ECONDASS c a b, Nop. Messages
that a pass prints appear in the log after the previous pass's dump, and their node numbers refer to
that (pre-pass) flowgraph.
"""
import argparse
import difflib
import struct
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import iro_dump  # noqa: E402
import mwcc_cmd  # noqa: E402

# --- RebuildLogicalExpressions (FUN_00608490 -> per candidate FUN_00608620), traced with the debugger ---
# A candidate is `flag = K` (K = 0/1, flag a local) in a block that ends with a conditional branch. 00608620
# tries to turn `flag = K; if (!a) goto L; if (!b) goto L; flag = !K; L:` back into flag = a && b (or ||).
# Each `return 0` is a different refusal; checkpoints split the shared exits.
BP_GEN = 0x468750
RB_ENTRY, RB_OK = 0x608620, 0x60910E
# --- Propagation (IroPropagate.c in 3.0). For each use of a variable that has a candidate assignment
# `x = rhs` in the pass's candidate list, the pass walks these checks; the furthest one reached tells why
# the use was not replaced. Use node / candidate record (ESI: +0 the assignment node, +0x24 uses in the
# block) are read at the checkpoints.
#   IroPropagate_PropagateExpressions 005F4060 (within one block; "expression propagation"):
EP_MATCH, EP_AVAIL, EP_OK5480, EP_TYPEOK = 0x5F424D, 0x5F427B, 0x5F4291, 0x5F42C1
EP_OPERAND, EP_OBJREF, EP_EXPR, EP_COPIED, EP_EXPR_END, EP_NEXT = 0x5F43C0, 0x5F44B5, 0x5F42DC, 0x5F42F2, 0x5F435E, 0x5F4380
EP_AFTER5600 = 0x5F42EC
#   IRO_CopyAndConstantPropagation 005F4810 (global, reaching definitions):
CP_MATCH, CP_AVAIL, CP_LOOPCHK, CP_LOOPOK, CP_OK5480, CP_TYPEOK, CP_NEXT = 0x5F4D31, 0x5F4D55, 0x5F4D66, 0x5F4F70, 0x5F4F89, 0x5F4FB5, 0x5F4D81
PASS_AFTER = 0x5ED5E0
PROP_REASONS = {
    'match': 'the assignment does not reach this use unchanged (killed on the way: x, or a variable the '
             'value reads, is written / a call or store may change it, or another definition of x also '
             'reaches here)',
    'avail': None,   # expression pass: next check is 5480; copy pass: loop check (set below)
    'loopchk': 'the value is not a simple operand and the use is in a deeper loop than the assignment',
    'ok5480': 'the variables the value reads change before a later use of x (FUN_005F5480)',
    'type': 'the types differ (the use and the assignment are not the same type, FUN_005B7980)',
}
RB_PATH_BLOCKED = 0x6098A6   # FUN_006097a0 found a real statement on the path: ECX = that IR node
RB_PATH_LEAVES = 0x6098B0    # FUN_006097a0: the path leaves the blocks it may cross
RB_CHECKPOINTS = {0x6089D4: 'join', 0x608BA3: 'samevar', 0x608BCB: 'const', 0x608BEA: 'region',
                  0x608CA8: 'path1', 0x608CCA: 'path2', 0x608CF0: 'path3'}
RB_REASONS = {
    0x608641: {None: 'the flag has no use-def information'},
    0x608C14: {None: 'the first test after `flag = K` is not one conditional-branch block of the expected shape'},
    0x6088EB: {None: 'the blocks of the tests are not nested as required (dominance check)'},
    0x60896B: {None: 'the blocks of the tests are not nested as required (dominance check)'},
    0x608C40: {None: 'the second test is not a conditional branch of the same kind to the same label'},
    0x608C50: {None: 'no block assigning the flag again after the tests (FUN_005bd930)',
               'join': 'the block after the tests is not a lone `flag = !K` to the same local',
               'samevar': "the later assignment's constant is not the complement of K",
               'const': 'the later assignment is in a different exception region (FUN_005b7e20)'},
    0x608C60: {None: 'the tests have different exception-action stacks (FUN_005afcd0): e.g. a local with a '
                     'destructor is live across only some of them'},
    0x608C9E: {None: 'a real statement sits between `flag = K` and the first test',
               'path1': "a real statement sits in the second test's block before the test",
               'path2': 'a real statement sits between the second test and `flag = !K`',
               'path3': 'a real statement sits after `flag = !K`, before the join'},
    0x609508: {None: 'the flag has other uses or assignments outside the pattern'},
}

NODE_RE = re.compile(r'^\s*(\d+): ([A-Za-z]\w*)(.*)$')   # (loop passes also print "i: 62 FN:3 ..." lists)
HEADER_RE = re.compile(r'^(Starting|Dumping|Optimizing) function (.+?)(?: (after|before) (.*?))?(?: \(tick (\d+)\))?$')
STRUCT_RE = re.compile(r'^(Flowgraph node|Flowgraph$|Succ =|Pred =|MustReach|LoopDepth|Dom:|-{10,}|\t)')
OPS = {'EMUL': '*', 'EDIV': '/', 'EMODULO': '%', 'EADD': '+', 'ESUB': '-', 'ESHL': '<<', 'ESHR': '>>',
       'ELESS': '<', 'EGREATER': '>', 'ELESSEQU': '<=', 'EGREATEREQU': '>=', 'EEQU': '==', 'ENOTEQU': '!=',
       'EAND': '&', 'EXOR': '^', 'EOR': '|', 'ELAND': '&&', 'ELOR': '||', 'EMULASS': '*=', 'EDIVASS': '/=',
       'EMODASS': '%=', 'EADDASS': '+=', 'ESUBASS': '-=', 'ESHLASS': '<<=', 'ESHRASS': '>>=', 'EANDASS': '&=',
       'EXORASS': '^=', 'EORASS': '|=', 'ECOMMA': ','}
PRE = {'EMONMIN': '-', 'EBINNOT': '~', 'ELOGNOT': '!', 'EPREINC': '++', 'EPREDEC': '--'}
NOISE = re.compile(r'^(Dumps for pass|\s*lp bit vector|\s*l1 bit vector|\s*header = |\d+(-\d+)?(,\d+(-\d+)?)*$|\s*$)')
REF_RE = re.compile(r'(?:\bat:? |\bfrom |assignment |sub at )(\d+)')


class Flowgraph:
    """One dump: linear nodes, the flow node each belongs to, and statement roots."""

    def __init__(self, lines):
        self.nodes, self.block, self._memo = {}, {}, {}
        blk = None
        for line in lines:
            if line.startswith('Flowgraph node'):
                blk = int(line.split()[2])
                continue
            m = NODE_RE.match(line)
            if m:
                i, kind, rest = int(m.group(1)), m.group(2), m.group(3)
                flags = re.findall(r'<([^>]*)>', rest)
                ops = rest.split('<', 1)[0].strip() if not rest.strip().startswith('"') else rest.strip().rsplit('" ', 1)[0] + '"'
                self.nodes[i] = (kind, ops, flags)
                self.block[i] = blk
        refd = set()
        for kind, ops, _ in self.nodes.values():
            if kind == 'Operand':
                continue
            for t in re.findall(r'(?<![@\w])(\d+)', ops):
                refd.add(int(t))
        self.roots = [i for i in sorted(self.nodes) if i not in refd and self.nodes[i][0] not in ('Nop', 'End')
                      and not (self.nodes[i][0] == 'Operand')]
        self.parent = {}
        for i, (kind, ops, _) in self.nodes.items():
            if kind == 'Operand':
                continue
            for t in re.findall(r'(?<![@\w])(\d+)', ops):
                self.parent.setdefault(int(t), i)

    def r(self, i, depth=0):
        """Render node i (memoised: after CSE the IR is a DAG, so plain recursion is exponential)."""
        if i in self._memo:
            return self._memo[i]
        if i not in self.nodes:
            return '#%d' % i
        if depth > 40:
            return '...'
        self._memo[i] = '<cycle #%d>' % i
        txt = self._render(i, depth)
        if len(txt) > 400:
            txt = txt[:397] + '...'
        self._memo[i] = txt
        return txt

    def _render(self, i, depth):
        kind, ops, flags = self.nodes[i]
        args = [int(t) for t in re.findall(r'(?<![@\w])(\d+)', ops)]
        sub = lambda k: self.r(args[k], depth + 1) if k < len(args) else '?'
        if kind == 'Operand':
            if re.match(r'^[A-Za-z_@$]', ops):
                return ops if 'ind' in flags else '&' + ops
            return ops
        if kind == 'EINDIRECT':
            c = self.nodes.get(args[0]) if args else None
            if c and c[0] == 'Operand' and re.match(r'^[A-Za-z_@$]', c[1]):
                return c[1]
            return '*' + sub(0)
        if kind == 'EASS':
            return '%s = %s' % (sub(0), sub(1))
        if kind in OPS:
            return '(%s %s %s)' % (sub(0), OPS[kind], sub(1))
        if kind in PRE:
            return PRE[kind] + sub(0)
        if kind in ('EPOSTINC', 'EPOSTDEC'):
            return sub(0) + ('++' if kind == 'EPOSTINC' else '--')
        if kind == 'ETYPCON':
            return '(cvt)' + sub(0)
        if kind == 'Funccall':
            m = re.match(r'(\d+)\((.*)\)', ops)
            if m:
                fn = self.nodes.get(int(m.group(1)))
                name = fn[1] if fn and fn[0] == 'Operand' else self.r(int(m.group(1)), depth + 1)
                al = [self.r(int(x), depth + 1) for x in m.group(2).split(',') if x.strip()]
                return '%s(%s)' % (name, ', '.join(al))
        if kind in ('If', 'IfNot'):
            lab = ops.split()[-1]
            return 'if (%s%s) goto %s' % ('' if kind == 'If' else '!', sub(0), lab)
        if kind == 'Goto':
            return 'goto ' + ops
        if kind == 'Label':
            return ops + ':'
        if kind == 'Return':
            return 'return' + (' ' + sub(0) if args else '')
        if kind == 'Switch':
            return 'switch (%s)' % sub(0)
        if kind == 'ECOND':
            return '(%s ? %s : %s)' % (sub(0), sub(1), sub(2))
        if kind == 'ECONDASS':
            return 'ECONDASS(%s, %s, %s)' % (sub(0), sub(1), sub(2))
        if kind == 'EBITFIELD':
            return 'bitfield(%s)' % sub(0)
        return '%s(%s)' % (kind, ', '.join(self.r(a, depth + 1) for a in args))

    def statements(self):
        return [(self.block.get(i), i, self.r(i)) for i in self.roots]

    def root_of(self, i):
        seen = 0
        while i in self.parent and seen < 100:
            i, seen = self.parent[i], seen + 1
        return i

    def var_counts(self):
        """{name: [defs, uses]} for every named operand."""
        c = {}
        for i, (kind, ops, flags) in self.nodes.items():
            if kind != 'Operand' or not re.match(r'^[A-Za-z_@$]', ops):
                continue
            e = c.setdefault(ops, [0, 0])
            par = self.parent.get(i)
            if 'assigned' in flags:
                e[0] += 1
            elif par is not None and self.nodes[par][0] == 'EINDIRECT':
                e[1] += 1
            elif par is not None:
                e[1] += 1
        return c


def parse(text, fpat):
    """[(pass label, Flowgraph, messages printed by that pass)] for the first function matching fpat."""
    lines = text.splitlines()
    out, cur, func, msgs_pending = [], None, None, []
    i = 0
    while i < len(lines):
        m = HEADER_RE.match(lines[i])
        if m:
            name = m.group(2)
            if m.group(1) == 'Starting':
                if func is not None and out:
                    break
                func = name if fpat.search(name) else None
                msgs_pending = []
                i += 1
                continue
            if func is None:
                i += 1
                continue
            j = i + 1
            while j < len(lines) and not HEADER_RE.match(lines[j]):
                j += 1
            body = lines[i + 1:j]
            graph_lines, trailing = [], []
            for ln in body:
                if NODE_RE.match(ln) or STRUCT_RE.match(ln):
                    graph_lines.append(ln)
                    trailing = []   # messages only count after the last flowgraph line
                else:
                    trailing.append(ln)
            label = '%s %s' % (m.group(3) or '', m.group(4) or '')
            label = label.strip() + (' (tick %s)' % m.group(5) if m.group(5) else '')
            if m.group(1) == 'Dumping' and graph_lines:
                out.append((label, Flowgraph(graph_lines), msgs_pending))
                msgs_pending = [t for t in trailing if not NOISE.match(t)]
            else:
                msgs_pending = msgs_pending + [t for t in trailing if not NOISE.match(t)]
            i = j
            continue
        i += 1
    return func, out


def prop_hooks(st, props):
    """Debugger handlers for the two propagation passes; decisions go to props as dicts."""
    import dbgcore

    def var_of(p, node):
        e = p.u32(p.u32(p.u32(node + 0x2A) + 0x2A) + 0x2A)
        return p.objname(p.u32(e + 0x10)) if e else '?'

    def start(kind, use_off):
        def h(p, ctx):
            fin(None)
            esi = struct.unpack_from('<I', ctx, 160)[0]
            use = p.u32(dbgcore.esp(ctx) + use_off)
            dnode = p.u32(esi)
            st['prop'] = {'kind': kind, 'def': p.s32(dnode + 0xA), 'use': p.s32(use + 0xA), 'var': var_of(p, dnode),
                          'stage': 'match', 'uses': None, 'outside': None, 'pass_seq': st['passes']}
        return h

    def stage(name):
        def h(p, ctx):
            if st.get('prop'):
                st['prop']['stage'] = name
                if name == 'expr':
                    esi = struct.unpack_from('<I', ctx, 160)[0]
                    st['prop']['uses'] = p.s32(esi + 0x24)
        return h

    def after5600(p, ctx):
        if st.get('prop'):
            st['prop']['outside'] = bool(struct.unpack_from('<I', ctx, 176)[0] & 0xFF)

    def fin(outcome):
        pr = st.get('prop')
        if not pr:
            return
        if outcome is None:
            sg = pr['stage']
            if sg == 'avail':
                outcome = 'NOT replaced: ' + (PROP_REASONS['ok5480'] if pr['kind'] == 'expr' else PROP_REASONS['loopchk'])
            elif sg == 'loopok':
                outcome = 'NOT replaced: ' + PROP_REASONS['ok5480']
            elif sg == 'ok5480':
                outcome = 'NOT replaced: ' + PROP_REASONS['type']
            elif pr['kind'] == 'expr':
                outcome = ('NOT replaced: not available here: a call, store or asm between the assignment and this '
                           'use (in IR order: in `x op f()` the call nodes come before the load of x) may change x '
                           'or a variable the value reads')
            else:
                outcome = 'NOT replaced: ' + PROP_REASONS['match']
        pr['outcome'] = outcome
        props.append(pr)
        st['prop'] = None

    def ep_next(p, ctx):
        fin(None)

    def ep_end(p, ctx):
        pr = st.get('prop')
        if not pr:
            return
        if pr['stage'] == 'expr':
            if pr['uses'] != 1:
                fin('NOT replaced: x is used %s times in this block (an expression is only copied into a single '
                    'use; the log still prints "Found expression propagation")' % pr['uses'])
            else:
                fin('NOT replaced: x is also read in another block (FUN_005F5600; the log still prints '
                    '"Found expression propagation")')
        else:
            fin('replaced' + {'copied': ' (expression copied into the use)', 'operand': ' (constant / operand)',
                              'objref': ' (variable)'}.get(pr['stage'], ''))

    def cp_done(p, ctx):
        fin('replaced')

    return {EP_MATCH: start('expr', 0x18), EP_AVAIL: stage('avail'), EP_OK5480: stage('ok5480'),
            EP_TYPEOK: stage('typeok'), EP_OPERAND: stage('operand'), EP_OBJREF: stage('objref'),
            EP_EXPR: stage('expr'), EP_AFTER5600: after5600, EP_COPIED: stage('copied'), EP_EXPR_END: ep_end,
            EP_NEXT: ep_next,
            CP_MATCH: start('copy', 0x150), CP_AVAIL: stage('avail'), CP_LOOPCHK: stage('loopchk'),
            CP_LOOPOK: stage('loopok'), CP_OK5480: stage('ok5480'), CP_TYPEOK: cp_done, CP_NEXT: ep_next}


def run_traced(argv, fpat, rebuilds, props=None):
    """Run the (patched) compile under the debugger and record RebuildLogicalExpressions decisions for the
    functions matching fpat into `rebuilds` as (node index, flag name, K, outcome)."""
    import dbgcore
    st = {'cand': None, 'cp': None, 'blk': None, 'prop': None, 'passes': 0}
    props = [] if props is None else props
    ph = prop_hooks(st, props)
    addrs = [RB_ENTRY, RB_OK, RB_PATH_BLOCKED, RB_PATH_LEAVES, PASS_AFTER] + list(RB_CHECKPOINTS) + list(RB_REASONS) + list(ph)

    def on_pass_after(p, ctx):
        if st.get('prop'):
            pass
        st['passes'] += 1

    def on_gen(p, ctx):
        fo = p.u32(dbgcore.arg(p, ctx, 1) + 4)
        name = p.objname(fo) if fo else '__sinit'
        st['passes'] = 0
        for ad in addrs:
            (p.arm if fpat.search(name) else p.disarm)(ad)

    def on_entry(p, ctx):
        node = dbgcore.arg(p, ctx, 2)
        idx = p.s32(node + 0xA)
        enode = p.u32(p.u32(p.u32(node + 0x2A) + 0x2A) + 0x2A)
        var = p.objname(p.u32(enode + 0x10)) if enode else '?'
        rhs = p.u32(p.u32(node + 0x2E) + 0x2A)
        st['cand'] = [idx, var, p.u32(rhs + 0x14) if rhs else '?', None, None]
        st['cp'] = st['blk'] = None

    def on_blocked(p, ctx):
        import struct
        ecx = struct.unpack_from('<I', ctx, 172)[0]
        st['blk'] = p.s32(ecx + 0xA)

    def on_leaves(p, ctx):
        st['blk'] = 'leaves'

    def on_cp(name):
        def h(p, ctx):
            st['cp'] = name
        return h

    def on_exit(addr):
        def h(p, ctx):
            if st['cand']:
                why = RB_REASONS[addr].get(st['cp']) or RB_REASONS[addr].get(None)
                st['cand'][3] = 'NOT rebuilt: %s  [exit %08X%s]' % (why, addr, ', after ' + st['cp'] if st['cp'] else '')
                if addr == 0x608C9E:
                    st['cand'][4] = st['blk']
                rebuilds.append(tuple(st['cand']))
                st['cand'] = None
        return h

    def on_ok(p, ctx):
        if st['cand']:
            st['cand'][3] = 'REBUILT into an &&/|| expression'
            rebuilds.append(tuple(st['cand']))
            st['cand'] = None

    lazy = {RB_ENTRY: on_entry, RB_OK: on_ok, RB_PATH_BLOCKED: on_blocked, RB_PATH_LEAVES: on_leaves,
            PASS_AFTER: on_pass_after}
    lazy.update(ph)
    lazy.update({ad: on_cp(n) for ad, n in RB_CHECKPOINTS.items()})
    lazy.update({ad: on_exit(ad) for ad in RB_REASONS})
    return dbgcore.run(argv, {BP_GEN: on_gen}, cwd=mwcc_cmd.ROOT, lazy=lazy)


def print_props(props, dumps, want):
    """Propagation decisions grouped by assignment. pass_seq = number of "after pass" dumps done when the
    decision was made, so the flowgraph it refers to is the dump with that many 'after' dumps before it."""
    after_idx = [k for k, (label, _, _) in enumerate(dumps) if label.startswith('after')]
    groups = {}
    for pr in props:
        if want and pr['var'] not in want:
            continue
        seq = pr['pass_seq']
        k = after_idx[seq - 1] if 0 < seq <= len(after_idx) else 0
        g = dumps[k][1]
        nxt = dumps[after_idx[seq]][0] if seq < len(after_idx) else '?'
        dtxt = g.r(g.root_of(pr['def'])) if pr['def'] in g.nodes else '#%d' % pr['def']
        utxt = g.r(g.root_of(pr['use'])) if pr['use'] in g.nodes else '#%d' % pr['use']
        key = (pr['kind'], pr['var'], dtxt)
        groups.setdefault(key, {}).setdefault((utxt, pr['outcome']), []).append(nxt)
    # expression-propagation candidates with no decision at all: no use of x in the assignment's block
    decided = {(pr['kind'], pr['def']) for pr in props}
    for k, (label, g, msgs) in enumerate(dumps):
        if not k or 'ExpressionPropagation' not in label:
            continue
        prev = dumps[k - 1][1]
        for msg in msgs:
            m = re.match(r'Found propagatable expression assignment at: (\d+)', msg.strip())
            if not m:
                continue
            idx = int(m.group(1))
            if ('expr', idx) in decided or idx not in prev.nodes:
                continue
            kind, ops, _ = prev.nodes[idx]
            lhs = [int(t) for t in re.findall(r'\d+', ops)][:1]
            var = prev.r(lhs[0]) if lhs else '?'
            if want and var not in want:
                continue
            key = ('expr', var, prev.r(idx))
            groups.setdefault(key, {}).setdefault(
                ('(none)', 'never tried: no use of %s in the same block as the assignment (expression propagation '
                 'only replaces uses inside one block)' % var), []).append(label)
    if not groups:
        return
    print('Propagation decisions (each use of a variable that has a candidate assignment; "copy" = copy/constant')
    print('propagation over the whole function, "expr" = expression propagation within one block):')
    for (kind, var, dtxt), uses in groups.items():
        print('  [%s] %s   (from `%s`)' % (kind, var, dtxt))
        for (utxt, outcome), passes in uses.items():
            where = passes[0].replace('after ', '') + (' (+%d more runs)' % (len(passes) - 1) if len(passes) > 1 else '')
            if utxt == '(none)':
                print('      %s   [%s]' % (outcome, where))
            else:
                print('      use in `%s`: %s   [%s]' % (utxt, outcome, where))
    print('  A local whose uses are all replaced (and whose assignment then dies in DeadStore) gets no register:')
    print('  its value lives in the codegen temp of the expression it was copied into.')
    print()


LOOP_RE = re.compile(r'IRO_DoCommonLoopTransformations:transforming loop with header (\d+)')


def print_loops(text, fpat):
    """Per loop seen by IRO_DoCommonLoopTransformations: preheader created or not, and which loop passes
    then ran (from the order of messages and dump headers in the log)."""
    loops, cur, infunc = [], None, False
    for line in text.splitlines():
        m = HEADER_RE.match(line)
        if m and m.group(1) == 'Starting':
            if infunc:
                break
            infunc = bool(fpat.search(m.group(2)))
            continue
        if not infunc:
            continue
        lm = LOOP_RE.search(line)
        if lm:
            cur = {'header': int(lm.group(1)), 'pre': None, 'passes': []}
            loops.append(cur)
            continue
        if cur is None:
            continue
        if line.startswith('No predecessor outside the loop'):
            cur['pre'] = 'not created: no predecessor outside the loop'
        elif m and m.group(1) == 'Dumping':
            pas = m.group(4) or ''
            if pas == 'Creating PreHeader':
                cur['pre'] = cur['pre'] or 'created'
            elif pas == 'IRO_DoCommonLoopTransformations':
                cur = None
            else:
                cur['passes'].append(pas)
    if not loops:
        return
    print('Loops (IRO_DoCommonLoopTransformations, processed in reverse layout order):')
    for lp in loops:
        pre = lp['pre'] or ('NOT created: FUN_005c6a60 refused (an outside predecessor of the header ends in '
                            'node kind 9 or 0x14, notes/mwcc_ghidra.md)')
        ran = ', '.join(dict.fromkeys(x.replace('IRO_', '') for x in lp['passes']))
        print('  header flow node %d: preheader %s; then: %s' % (lp['header'], pre, ran or 'nothing'))
    print('  Without a preheader there is no invariant motion or strength reduction for that loop.')
    print()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name as the log prints it (usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--log', help='use this IRO log (from iro_dump.py) instead of compiling')
    ap.add_argument('--var', help='comma separated variables: only show what touches them')
    ap.add_argument('--pass', dest='passes', help='regex: only these passes in the timeline')
    ap.add_argument('--no-timeline', action='store_true', help='only the summaries, no timeline')
    ap.add_argument('--no-debug', action='store_true', help="don't trace decisions with the debugger (log only)")
    ap.add_argument('--repeats', action='store_true', help='repeat messages identical to an earlier run of the same pass')
    ap.add_argument('--messages', choices=('resolved', 'all', 'none'), default='resolved',
                    help='decision messages: only those that point at a statement (default), all, none')
    a = ap.parse_args()
    fpat = re.compile(a.func)
    rebuilds, props = [], []
    traced = False
    if a.log:
        text = open(a.log, encoding='latin1').read()
    elif sys.platform == 'win32' and not a.no_debug:
        traced = True
        text, _ = iro_dump.compile_log(a.src, a.input, a.overlay, keep=False,
                                       runner=lambda argv, cwd: run_traced(argv, fpat, rebuilds, props))
    else:
        text, _ = iro_dump.compile_log(a.src, a.input, a.overlay, keep=False)
    func, dumps = parse(text, fpat)
    if not dumps:
        raise SystemExit('no IRO dumps for a function matching %r' % a.func)
    want = set(v.strip() for v in a.var.split(',')) if a.var else None
    touches = (lambda s: any(re.search(r'(?<![\w@$])%s(?![\w])' % re.escape(v), s) for v in want)) if want else (lambda s: True)

    # --- variable summary ---
    print('=== %s: %d optimizer dumps' % (func, len(dumps)))
    hist = {}
    for k, (label, g, _) in enumerate(dumps):
        for name, (d, u) in g.var_counts().items():
            hist.setdefault(name, {})[k] = (d, u)
    first_label = dumps[0][0]
    last = len(dumps) - 1
    rows = []
    for name in sorted(hist, key=lambda n: (n.startswith('@'), n)):
        h = hist[name]
        if want and name not in want:
            continue
        if not want and not (name.startswith('@') or any(d for d, _ in h.values())):
            continue   # globals / functions only read: not interesting
        k0 = min(h)
        d0, u0 = h[k0]
        events = []
        if k0 > 0:
            events.append('created by the pass before the dump %s' % dumps[k0][0])
        prev = (d0, u0)
        for k in range(k0 + 1, len(dumps)):
            cur = h.get(k, (0, 0))
            if cur != prev:
                if cur == (0, 0):
                    events.append('gone %s' % dumps[k][0])
                elif cur[1] == 0 and prev[1] > 0:
                    events.append('last use removed %s (defs %d)' % (dumps[k][0], cur[0]))
                elif cur[0] == 0 and prev[0] > 0:
                    events.append('last def removed %s' % dumps[k][0])
                elif cur[0] < prev[0] or cur[1] < prev[1]:
                    events.append('%d/%d -> %d/%d defs/uses %s' % (prev + cur + (dumps[k][0],)))
            prev = cur
        end = h.get(last, (0, 0))
        state = 'gone' if end == (0, 0) else 'defs %d uses %d at the end' % end
        rows.append('  %-22s start defs %d uses %d -> %s%s' % (name, d0, u0, state,
                    ('; ' + '; '.join(events)) if events else ''))
    print('Variables (named locals/parameters that are written, and every @ temporary):')
    print('\n'.join(rows) if rows else '  (none)')
    print('  A variable that is gone, or has no uses left, gets no register of its own: its value was')
    print('  propagated into the expressions that used it (see vregs.py for the codegen temp holding it).')
    print('  One that survives the optimizer but has no code in vregs.py was removed later, by the')
    print('  back end (TOC / instruction selection / PCode copy propagation: enode.py, peephole.py).')
    print()
    print_loops(text, fpat)
    if props:
        print_props(props, dumps, want)
    if rebuilds:
        print('&&/|| rebuild (RebuildLogicalExpressions) decisions, one per attempt:')
        g = None
        for k, (label, gr, _) in enumerate(dumps):
            if 'RebuildLogicalExpressions' in label and k:
                g = dumps[k - 1][1]
        seen = {}
        for r in rebuilds:
            seen[r] = seen.get(r, 0) + 1
        for (idx, var, kval, outcome, blk), n in seen.items():
            if want and var not in want:
                continue
            if n > 1:
                outcome += '  (x%d: the pass repeats until nothing changes)' % n
            stx = g.r(g.root_of(idx)) if g and idx in g.nodes else '#%d' % idx
            print('  node %d `%s` (%s = %s): %s' % (idx, stx, var, kval, outcome))
            if blk == 'leaves':
                print('      the path between them leaves the blocks the pattern may cross')
            elif blk is not None:
                bt = g.r(g.root_of(blk)) if g and blk in g.nodes else '#%d' % blk
                print('      blocking statement: node %s `%s`' % (blk, bt))
        print('  A rebuilt flag becomes one expression generated into codegen temps (coloured before every @ temp);')
        print('  a flag that is not rebuilt stays a variable with its own `li rN,K`.')
        print()
    elif traced:
        print('&&/|| rebuild: no candidates (no `flag = 0/1` before a conditional branch reached the check)')
        print()
    if a.no_timeline:
        return

    # --- timeline ---
    ppat = re.compile(a.passes) if a.passes else None
    print('Timeline (%s = first dump; each pass: its decision messages, then - / + statements):' % first_label)
    shown = set()
    for k in range(1, len(dumps)):
        label, g, msgs = dumps[k]
        base = re.sub(r' \(tick \d+\)$', '', label)
        if ppat and not ppat.search(label):
            continue
        prev_g = dumps[k - 1][1]
        before = [s for _, _, s in prev_g.statements()]
        after = [s for _, _, s in g.statements()]
        lines = []
        if a.messages != 'none':
            for msg in msgs:
                refs = [int(x) for x in REF_RE.findall(msg)]
                res = []
                if msg.startswith('Removing unreachable code at'):   # this one names a flow node (block)
                    blk = refs[0] if refs else None
                    sts = [prev_g.r(i) for i in prev_g.roots if prev_g.block.get(i) == blk]
                    res.append('flow node %s: %s' % (blk, '; '.join(sts[:3]) if sts else '(empty)'))
                    refs = []
                for r in refs:
                    if r in prev_g.nodes:
                        root = prev_g.root_of(r)
                        node_txt = prev_g.r(r)
                        st_txt = prev_g.r(root)
                        res.append('%d: %s%s' % (r, node_txt, '' if root == r else '   in   ' + st_txt))
                if a.messages == 'resolved' and not res:
                    continue
                text = msg.strip() + ('  ->  ' + ' | '.join(res) if res else '')
                if not a.repeats:
                    if (base, text) in shown:
                        continue
                    shown.add((base, text))
                if touches(text):
                    lines.append('    ! ' + text)
        sm = difflib.SequenceMatcher(None, before, after, autojunk=False)
        for op, i1, i2, j1, j2 in sm.get_opcodes():
            if op == 'equal':
                continue
            for s in before[i1:i2]:
                if touches(s):
                    lines.append('    - ' + s)
            for s in after[j1:j2]:
                if touches(s):
                    lines.append('    + ' + s)
        if lines:
            print('  %s' % label)
            print('\n'.join(lines))
    print()
    print('Final statements (input to code generation, before TOC; enode.py shows the typed trees):')
    for blk, _, s in dumps[-1][1].statements():
        if touches(s):
            print('  [B%s] %s' % (blk, s))


if __name__ == '__main__':
    main()
