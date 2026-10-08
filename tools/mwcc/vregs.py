"""Where does each virtual register come from? (Windows only)

regalloc.py says *which* renumbering would give the target's registers ("id must be numbered above
v54"). This tool says *what* every virtual register is, so you can tell which source construct to
change. It runs the real GC 3.0a5.2 compiler under the debugger and, for each matching function, prints
every GPR (or FPR) virtual register with:

  - its final physical register and its place in the colouring order (#1 = coloured first)
  - what it belongs to:
      object   a variable that got its own register: an argument, a named local, or an @ temporary.
               For @ temporaries, who created the temporary: the inliner (an inline function's parameter
               or result), an IRO pass (named after the pass that created it: CSE, strength reduction,
               lifetime splitting, ...), or a front-end rewrite.
      codegen  a temporary made by instruction selection while generating one statement. Shows the
               statement (index, source line, the statement as an expression), the expression node being
               generated (innermost first) and the code generator function that made it.
      backend  made after instruction selection (backend optimizer: loop strength reduction, array to
               register, ...), with the creating function.
  - with --chain, the full call chain of the creating site.

    python tools/mwcc/vregs.py src/dol/game/d_field_assessment.cpp "^fgMngProc_getBuriedMoneyFg$"
    python tools/mwcc/vregs.py <src> <func regex> --input variant.cpp -I overlay_dir
    python tools/mwcc/vregs.py <src> <func regex> --fpr            # float registers instead
    python tools/mwcc/vregs.py <src> <func regex> --only v54,v59   # just these

Reading it together with regalloc.py --want: if --want says "renumber id above v54" and v54 is a codegen
temp of statement S12's ENOTEQU, then `id` would have to be a codegen temp too (e.g. its named local
copy-propagated away into an expression), or v54's expression moved earlier. iro_why.py then shows what
the optimizer did to `id`.

How numbering follows creation (notes/mwcc_ghidra.md 1b): objects are numbered first (arguments, named
locals in reverse declaration order, @ temps in reverse creation order), then codegen temps in creation
order, then backend temps. "created" below is the order the counter was incremented in.

GC 3.0a5.2 addresses (Ghidra program mwcceppc.exe, names in mwcc30_names.txt / notes/mwcc30_functions.csv):
  CodeGen_Generator 00468750 (arg1 = {Statement *head; Object *func}); current statement global 007109AC
  (set for each statement of the codegen loop, 0 outside it); after the TOC call 0046899B ([esp] = head).
  Virtual register counters: used_virtual_registers[class] at 00716EA0 + 4*class (GPR = 4 -> 00716EB0,
  FPR = 3 -> 00716EAC), incremented in place (`mov r,[c]; add [c],1`) at 100+ sites: watched with a
  hardware write watchpoint. Expression generator table 0070FD98 (ENode kind -> handler(ENode *, ...)),
  filled at startup. create_temp_object 0052DFB0, its RET 0052E035 (EAX = the new @ object).
  IRO pass boundaries: 005ED5E0(pass name, 0) after each pass, 005ED680(name, 0) before the pass loop /
  IRO_Delinearize. Coloring_AllocateRegisters entry 00596DF0 (arg1 = function), Coloring_SelectColors
  005970E0 (arg1 = colouring stack), Coloring_CommitAssignments 00596F90; IGNode array 00711AFC
  (0x1E bytes: Object* +4, vreg +0x10, phys +0x12, flags +0x14, degree +0x16); current class 00726060.
"""
import argparse
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dbgcore  # noqa: E402
import mwcc_cmd  # noqa: E402
from mwcc_hooks import TempTracker  # noqa: E402
from enode import Dumper, ENODE_KINDS, K, MONADIC, DIADIC, TRIADIC, CALLS, ST_KINDS  # noqa: E402

BP_GENERATOR, BP_POST_TOC = 0x468750, 0x46899B
CUR_STMT = 0x7109AC
VREG_COUNT = 0x716EA0
EXPR_TABLE = 0x70FD98
BP_ALLOC, BP_COMMIT, BP_SELECT = 0x596DF0, 0x596F90, 0x5970E0
NODES, REG_CLASS = 0x711AFC, 0x726060
GPR, FPR = 4, 3

OPS = {'EMUL': '*', 'EDIV': '/', 'EMODULO': '%', 'EADD': '+', 'ESUB': '-', 'ESHL': '<<', 'ESHR': '>>',
       'ELESS': '<', 'EGREATER': '>', 'ELESSEQU': '<=', 'EGREATEREQU': '>=', 'EEQU': '==', 'ENOTEQU': '!=',
       'EAND': '&', 'EXOR': '^', 'EOR': '|', 'ELAND': '&&', 'ELOR': '||', 'EASS': '=', 'EMULASS': '*=',
       'EDIVASS': '/=', 'EMODASS': '%=', 'EADDASS': '+=', 'ESUBASS': '-=', 'ESHLASS': '<<=',
       'ESHRASS': '>>=', 'EANDASS': '&=', 'EXORASS': '^=', 'EORASS': '|=', 'ECOMMA': ','}
PRE = {'EMONMIN': '-', 'EBINNOT': '~', 'ELOGNOT': '!', 'EPREINC': '++', 'EPREDEC': '--'}


def expr_str(p, n, depth=0, dumper=None):
    """One-line C-like rendering of an ENode tree (types shown only on conversions)."""
    if not n:
        return '?'
    if depth > 12:
        return '...'
    kind = p.u8(n)
    name = ENODE_KINDS[kind] if kind < len(ENODE_KINDS) else 'E?%d' % kind
    sub = lambda off: expr_str(p, p.u32(n + off), depth + 1, dumper)
    if name == 'EINDIRECT':
        c = p.u32(n + 0x10)
        if c and p.u8(c) == K['EOBJREF']:
            return p.objname(p.u32(c + 0x10))
        return '*' + sub(0x10)
    if name == 'EOBJREF':
        return '&' + p.objname(p.u32(n + 0x10))
    if name in ('EINTCONST', 'EFLOATCONST', 'ESTRINGCONST'):
        return dumper.node_data(n, kind)
    if name == 'ETYPCON':
        return '(%s)%s' % (dumper.type_name(p.u32(n + 4)), sub(0x10))
    if name in PRE:
        return PRE[name] + sub(0x10)
    if name in ('EPOSTINC', 'EPOSTDEC'):
        return sub(0x10) + ('++' if name == 'EPOSTINC' else '--')
    if name in OPS:
        return '(%s %s %s)' % (sub(0x10), OPS[name], sub(0x14))
    if kind in CALLS:
        f = p.u32(n + 0x10)
        fn = p.objname(p.u32(f + 0x10)) if f and p.u8(f) == K['EOBJREF'] else '(%s)' % sub(0x10)
        args, a, i = [], p.u32(n + 0x14), 0
        while a and i < 16:
            args.append(expr_str(p, p.u32(a + 4), depth + 1, dumper))
            a, i = p.u32(a), i + 1
        return '%s(%s)' % (fn, ', '.join(args))
    if name == 'ECOND':
        return '(%s ? %s : %s)' % (sub(0x10), sub(0x14), sub(0x18))
    if kind in MONADIC:
        return '%s(%s)' % (name, sub(0x10))
    if kind in DIADIC:
        return '%s(%s, %s)' % (name, sub(0x10), sub(0x14))
    if kind in TRIADIC:
        return '%s(%s, %s, %s)' % (name, sub(0x10), sub(0x14), sub(0x18))
    return name


def node_label(p, n, dumper):
    kind = p.u8(n)
    name = ENODE_KINDS[kind] if kind < len(ENODE_KINDS) else 'E?%d' % kind
    s = expr_str(p, n, 0, dumper)
    return '%s %s' % (name, s if len(s) <= 70 else s[:67] + '...')


PCODE_BLOCKS = 0x7108C8


def fmt_operands(p, instr, cnt):
    """Operands of a PCode instruction: vNN for virtual registers (>= 32), rN/fN/crN for physical ones,
    #imm, mem refs; internal bookkeeping operands are left out."""
    out = []
    for j in range(min(cnt, 8)):
        o = p.rd(instr + 0x2C + j * 0xE, 0xE)
        kind, kc = o[0], o[1]
        if kind == 0:
            reg = struct.unpack_from('<h', o, 4)[0]
            pre = {4: 'r', 3: 'f', 1: 'cr'}.get(kc, 'k%d_' % kc)
            out.append(('v%d' if reg >= 32 and kc in (3, 4) else pre + '%d') % reg)
        elif kind == 2:
            v = struct.unpack_from('<i', o, 2)[0]
            out.append(str(v) if -1024 < v < 1024 else '0x%X' % (v & 0xFFFFFFFF))
        elif kind == 4:
            out.append('mem')
        elif kind in (8,):
            continue
        else:
            out.append('<%d>' % kind)
    return out


def pcode_defs_uses(p, cls_code=4):
    """Before register assignment: {vreg: first defining instruction text}, {vreg: use count}, from
    the PCode list (block: next +0, first instruction +0x14; instruction: next +0; operand effect u16 +2,
    bit 2 = write, bit 1 = read)."""
    defs, uses = {}, {}
    b, guard = p.u32(PCODE_BLOCKS), 0
    while b and guard < 100000:
        i = p.u32(b + 0x14)
        while i and guard < 100000:
            guard += 1
            mn, ops, _ = p.pcode(i)
            cnt = p.s16(i + 0x2A)
            for j in range(min(cnt, 8)):
                o = p.rd(i + 0x2C + j * 0xE, 0xE)
                if o[0] != 0 or o[1] != cls_code:
                    continue
                eff, vr = struct.unpack_from('<H', o, 2)[0], struct.unpack_from('<h', o, 4)[0]
                if eff & 2 and vr not in defs:
                    defs[vr] = '%s %s' % (mn, ', '.join(fmt_operands(p, i, cnt)))
                if eff & 1:
                    uses[vr] = uses.get(vr, 0) + 1
            i = p.u32(i)
        b = p.u32(b)
    return defs, uses


def main():
    if sys.platform != 'win32':
        raise SystemExit('vregs.py debugs the Windows compiler binary: run it on Windows')
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--fpr', action='store_true', help='float registers instead of GPRs')
    ap.add_argument('--only', help='comma separated vregs (v54) or object names to show')
    ap.add_argument('--chain', action='store_true', help='print the whole creating call chain')
    ap.add_argument('--all', action='store_true', help='also show vregs without a physical register / unused')
    a = ap.parse_args()
    fpat = re.compile(a.func)
    cls = FPR if a.fpr else GPR
    only = set(x.strip() for x in a.only.split(',')) if a.only else None

    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'vregs', str(os.getpid()))
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)

    st = {'func': '', 'match': False, 'stmts': {}, 'estack': [], 'last': None, 'vregs': {},
          'phase': 'setup', 'order': None, 'out': 0,
          'handlers': []}
    counter = VREG_COUNT + 4 * cls

    def chain_names(p, ctx):
        names = [dbgcore.funcname(dbgcore.eip(ctx))]
        for _, nm in dbgcore.callers(p, ctx):
            if nm == 'CodeGen_Generator':
                break
            if nm.rstrip('?') != names[-1].rstrip('?'):
                names.append(nm)
        return names

    tt = TempTracker()   # who made each @ object, and in which IRO pass

    # --- code generation ---
    def on_generator(p, ctx):
        fo = p.u32(dbgcore.arg(p, ctx, 1) + 4)
        st['func'] = p.objname(fo) if fo else '__sinit'
        st['match'] = bool(fpat.search(st['func']))
        tt.at_generator()
        if st['match']:
            st.update(stmts={}, estack=[], vregs={}, phase='setup', last=p.u32(counter), order=None)
            if not st['handlers']:
                seen = set()
                for i in range(len(ENODE_KINDS)):
                    h = p.u32(EXPR_TABLE + 4 * i)
                    if dbgcore.TEXT_LO <= h < dbgcore.TEXT_HI:
                        seen.add(h)
                st['handlers'] = sorted(seen)
                for h in st['handlers']:
                    p._saved.setdefault(h, p.rd(h, 1))
                    lazy_handlers[h] = on_expr
            for h in st['handlers']:
                p.arm(h)
            p.watching = True
        else:
            for h in st['handlers']:
                p.disarm(h)
            p.watching = False

    def on_post_toc(p, ctx):
        if not st['match']:
            return
        head = p.u32(dbgcore.esp(ctx))
        d = Dumper(p)
        s, idx = p.u32(head), 0
        while s and idx < 100000:
            kind = p.u8(s + 4)
            line = struct.unpack('<i', p.rd(s + 0x1E, 4))[0]
            text = expr_str(p, p.u32(s + 0xA), 0, d) if kind in (4, 5, 6, 7, 8, 15) and p.u32(s + 0xA) else ''
            st['stmts'][s] = (idx, line, ST_KINDS.get(kind, 'ST%d' % kind), text)
            s, idx = p.u32(s), idx + 1
        st['last'] = p.u32(counter)

    def on_expr(p, ctx):
        sp = dbgcore.esp(ctx)
        es = st['estack']
        stmt = p.u32(CUR_STMT)
        while es and (es[-1][0] <= sp or es[-1][2] != stmt):
            es.pop()
        es.append((sp, dbgcore.arg(p, ctx, 1), stmt))

    def on_counter(p, ctx):
        if not st['match']:
            return
        new = p.u32(counter)
        last = st['last'] if st['last'] is not None else new - 1
        if not (last < new <= last + 4):
            st['last'] = new        # a reset (new function / class), not an allocation
            return
        stmt = p.u32(CUR_STMT)
        if stmt:
            st['phase'] = 'codegen'
        elif st['phase'] == 'codegen':
            st['phase'] = 'backend'
        sp = dbgcore.esp(ctx)
        enodes = [n for s_, n, sm in reversed(st['estack']) if s_ > sp and sm == stmt]
        rec = {'phase': st['phase'], 'stmt': stmt, 'enodes': enodes[:4], 'chain': chain_names(p, ctx),
               'seq': None}
        d = Dumper(p)
        rec['enode_text'] = [node_label(p, n, d) for n in enodes[:3]]
        for vr in range(last, new):
            r = dict(rec)
            r['seq'] = len(st['vregs'])
            st['vregs'][vr] = r
        st['last'] = new

    # --- register allocation results ---
    def on_alloc(p, ctx):
        pass

    def on_select(p, ctx):
        if not st['match'] or p.s8(REG_CLASS) != cls:
            return
        node, order = p.u32(dbgcore.arg(p, ctx, 1)), []
        while node:
            order.append(p.s16(node + 0x10))
            node = p.u32(node)
        st['order'] = {vr: i + 1 for i, vr in enumerate(order)}

    def on_commit(p, ctx):
        if not st['match'] or p.s8(REG_CLASS) != cls:
            return
        p.watching = False
        lo, hi, base = p.u32(0x710274 + cls * 4), p.u32(VREG_COUNT + cls * 4), p.u32(NODES)
        d = Dumper(p)
        defs, uses = pcode_defs_uses(p, 4 if cls == GPR else 3)
        rows = []
        for x in range(lo, hi):
            n = base + x * 0x1E
            vr, ph, fl, deg, ob = p.s16(n + 0x10), p.s16(n + 0x12), p.u16(n + 0x14), p.s16(n + 0x16), p.u32(n + 4)
            oname = p.objname(ob) if ob else ''
            if only and ('v%d' % vr) not in only and oname not in only:
                continue
            rank = (st['order'] or {}).get(vr)
            if not a.all and rank is None and not ob:
                continue
            rows.append((vr, ph, deg, rank, ob, oname))
        print('=== %s: %s virtual registers %d..%d (#n = colouring order, 1 = first coloured; deg = degree)'
              % (st['func'], 'FPR' if cls == FPR else 'GPR', lo, hi - 1))
        for vr, ph, deg, rank, ob, oname in rows:
            head = '  v%-4d %s%-3d %-5s deg %-3d' % (vr, 'f' if cls == FPR else 'r', ph, '#%d' % rank if rank else '-', deg)
            info = st['vregs'].get(vr)
            dtext = ('  def: %s' % defs[vr]) if vr in defs else ''
            if ob and vr not in defs and not uses.get(vr):
                dtext = ('  NO INSTRUCTION USES IT: propagated into expressions by IRO (iro_why.py) or merged by '
                         'the back end (peephole.py)')
            if ob:
                tr = tt.describe(ob, a.chain)
                if oname.startswith('@') or oname.startswith('$'):
                    what = '@ temporary %s : %s' % (oname, d.type_name(p.u32(ob + 0x10)))
                    if tr:
                        what += '  (%s)' % tr
                    else:
                        what += '  (creator not seen: not made by create_temp_object)'
                else:
                    what = 'object %s : %s' % (oname, d.type_name(p.u32(ob + 0x10)))
                print(head + what + dtext)
                continue
            if not info:
                print(head + 'codegen temp (creation not recorded)')
                continue
            if info['phase'] == 'codegen' and info['stmt'] in st['stmts']:
                idx, line, kind, text = st['stmts'][info['stmt']]
                stext = text if len(text) <= 90 else text[:87] + '...'
                print(head + 'codegen temp, S%d %s line %d: %s' % (idx, kind, line, stext))
            else:
                print(head + '%s temp' % info['phase'])
            if dtext:
                print(' ' * 30 + dtext.strip())
            elif not uses.get(vr):
                print(' ' * 30 + 'no instruction left: merged away by PCode copy propagation / coalescing '
                      '(peephole.py shows where)')
            if info['enode_text']:
                print(' ' * 30 + 'generating: ' + '  <-  '.join(info['enode_text']))
            ch = info['chain'] if a.chain else info['chain'][:3]
            print(' ' * 30 + 'made by: ' + ' <- '.join(ch))
        print()
        st['out'] += 1

    lazy_handlers = {}
    handlers = {BP_GENERATOR: on_generator, BP_POST_TOC: on_post_toc, BP_SELECT: on_select, BP_COMMIT: on_commit}
    handlers.update(tt.handlers())
    code = dbgcore.run(argv, handlers, cwd=mwcc_cmd.ROOT, lazy=lazy_handlers, watch=[(counter, on_counter)])
    if not st['out']:
        print('no function matched %r (compiler exit code %d)' % (a.func, code), file=sys.stderr)


if __name__ == '__main__':
    main()
