"""Dump MWCC's expression trees (ENodes) for one function as they reach code generation (Windows only).

The front end and the IR optimizer hand the back end a list of statements, each with an expression tree.
Before any PCode is generated, the TOC pass (TOC.c `fn_0049d420` -> `TOC_0049d710`) walks every tree
and rewrites it: it narrows arithmetic to a smaller result type, strips conversions, folds constant
conversions, turns float/string constants into memory loads, and so on. Instruction selection then
generates code from the rewritten trees, statement by statement. This tool stops the real compiler at
both points and prints the trees:

  pre-toc   at the entry of fn_0049d420: the trees after the IR optimizer, before TOC rewrites them
  post-toc  right after fn_0049d420 returns: exactly what CodeGen_Generator's statement loop hands to
            instruction selection (fn_00436390 for expressions, generate_comparison_branch, ...)

    python tools/mwcc/enode.py src/d_fish_fieldNP/d_fish_field.cpp "^executeRelease$"
    python tools/mwcc/enode.py <src> <func regex> --stage post-toc
    python tools/mwcc/enode.py <src> <func regex> --input build/mwcc_tools/variant.cpp -I overlay_dir
    python tools/mwcc/enode.py <src> <func regex> --grep addCalcAngle      # only statements mentioning it

Output: one block per statement (`S<n> EXPR  line N`, `IFGOTO -> label`, `RETURN`, ...), then its tree,
one node per line, children indented below their parent. Fish executeRelease, `s32 target = -pitch;
sLib::addCalcAngle(&mTargetAngle.x.mAngle, target, 16, 0x71C);` (pitch is s16):

  S77 EXPR  line 1831
    EFUNCCALL  void  call addCalcAngle
      arg0: EADD  ptr(s16)
        ...
      arg1: ETYPCON  s16                       <- parameter conversion to s16
        ETYPCON  long                          <- int -> long (s32) initialises `target`
          EMONMIN  int                         <- -pitch is computed in int (C promotion)
            ETYPCON  int
              EINDIRECT  s16
                EOBJREF  ptr(s16)  pitch [local]
      arg2: EINTCONST  s16  16
      arg3: EINTCONST  s16  1820 (0x71C)

TOC leaves this one unchanged. With `int target` the ETYPCON long is absent and TOC rewrites arg1 to
ETYPCON s16 <- EMONMIN s16 <- EINDIRECT s16 (a 16-bit neg, no extsh in between).

Each node shows its kind, its result type (rtype), and kind-specific data: EINTCONST value (hex for
large values), EFLOATCONST value, EOBJREF object name and storage class, EFUNCCALL call target (the
compiler's plain name). With --flags, the raw node flags and the ignored/hascall/cost bytes are added.
With --stage both (the default) the post-toc dump repeats only statements TOC changed (--full repeats
all of them).

Rule of thumb for reading the output:
  - Read each ETYPCON as "convert the child to this type". TOC narrows an expression only through
    EMONMIN/EBINNOT, the arithmetic binops, and ETYPCONs to an integral type smaller than 4 bytes; an
    EMONMIN drops a direct ETYPCON(int) child. When TOC narrows, the post-toc tree shows the inner op
    retyped (EMONMIN s16) and the conversion gone, and the extsh/clrlwi in between disappears from the code.
  - `long` and `int` are distinct types in MWCC, both 4 bytes, and s32 is `long`. A conversion between
    them is still an ETYPCON node, and a 4-byte ETYPCON blocks the narrowing (notes/mwcc_ghidra.md 1b).
    So an `ETYPCON long` (or `ETYPCON int`) sitting between a small-typed parent and its operand marks a
    place where the target's extra extend comes from, and which local type to change.
  - A node's type is the type its code is generated in: an op typed s16/u8 is a 16/8-bit op plus an
    extend where the value is used; an op typed int/long is a full-word op.
  - EINDIRECT(EOBJREF x) is a load of variable x; a bare EOBJREF is the address of x. After TOC, an EASS
    has a bare EOBJREF on the left (TOC drops the EINDIRECT of the store target). Locals the allocator
    will put in registers still look like this; `@nnn` names are compiler temporaries. Inlined code is
    already expanded and carries the call site's line.

GC 3.0a5.2 addresses and layouts (Ghidra program mwcceppc.exe; names in notes/mwcc30_functions.csv):
  CodeGen_Generator        00468750  arg1 = struct {Statement *head; Object *func; u8 ...}; func 0 = __sinit
  fn_0049d420 (TOC entry)  0057D560  arg1 = Statement list head (dummy; first statement = head->next)
  after the TOC call       0046899B  `cmp byte [0070F1CF],0` after `push edi; call 0057D560`; [esp] = head
  TOC_0049d710             0057D8E0  the per-node rewriter (node, narrowing target Type*, ignored)
  ENode kind names         006E915C  char*[89], filled by 005B89C0 (same order as the ENODE_KINDS below)
  format_type              005B5AC0  3.0's own type printer (type kinds below come from its switch)
  intconstnode             005242B0  shows EINTCONST stores the CInt64 as hi @+0x10, lo @+0x14
TOC runs inside CodeGen_Generator, after the IR optimizer (00593280) and before any PCode exists, so unlike
the scheduler (sched.py) the function is known exactly at both stops; no buffering is needed.
Statement (packed; 1.2.5's layout up to +0x16): next +0, kind u8 +4, flags u8 +6, value s16 +8, expr ENode*
  +0x0A, label/switch +0x0E, dobjstack +0x12, then 3.0's source position record: file record* +0x16
  (CodeGen_Generator passes &stmt->+0x16 to 005820E0), char offset s32 +0x1A, line s32 +0x1E (inlined
  statements carry the call site's line). Kinds: 1 NOP 2 LABEL 3 GOTO 4 EXPR 5 SWITCH
  6 IFGOTO 7 IFNGOTO 8 RETURN ... 0xF GOTOEXPR 0x10 ASM. CLabel: name HashNameNode* +0x0C.
ENode (0x2E bytes; 1.2.5's was 0x1A): kind u8 +0, cost u8 +1, ignored u8 +2 (TOC writes its 3rd arg here),
  hascall u8 +3, rtype Type* +4, flags u32 +8 (bit 1 set on TOC-made loads, bit 8 tested on EOBJREF),
  +0x0C unknown, data +0x10: monadic child / diadic left,right (+0x10,+0x14) / ECOND cond,e1,e2
  (+0x10,+0x14,+0x18) / EFUNCCALL funcref,args ENodeList{next,node},functype (+0x10,+0x14,+0x18) /
  EOBJREF Object* +0x10 / EINTCONST CInt64 hi,lo +0x10,+0x14 / EFLOATCONST double +0x10 / ESTRINGCONST
  size,data (+0x10,+0x14) / ECONDASS three children like ECOND (TOC treats it so; roles not traced).
  The kind enum is 1.2.5's with EMIN/EMAX added after ECOMMA, so ETYPCON is 50 (0x32), not 48.
Type (packed): kind u8 +0, size s32 +2. Kinds (3.0, from format_type): 0 void 1 int 2 float 4 enum
  (name +0x12) 5 struct (name +6, vector if byte +0x10 in 4..14) 6 class (name +0x0A) 7 func (return
  type +0x0E) 8 bitfield (base +6, offset +0x0A, width +0x0B) 9 label 0xB memberpointer 0xC pointer
  (target +6) 0xD array (element +6). Integral code u8 +6: 0 bool 1 char 2 schar 3 uchar 4 wchar_t 5 short
  6 ushort 7 int 8 uint 9 long 10 ulong 11 longlong 12 ulonglong 13 float 14 short double 15 double
  16 long double. Object: datatype u8 +2 (0 data 1 local 2 absolute 3 func 4 vfunc 5 inline ...),
  name +0x0C, type +0x10.
"""
import argparse
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dbgcore  # noqa: E402
import mwcc_cmd  # noqa: E402

BP_GENERATOR = 0x468750   # CodeGen_Generator entry: names the function
BP_PRE_TOC = 0x57D560     # fn_0049d420 entry
BP_POST_TOC = 0x46899B    # first instruction after `call fn_0049d420` in CodeGen_Generator

ENODE_KINDS = [
    'EPOSTINC', 'EPOSTDEC', 'EPREINC', 'EPREDEC', 'EINDIRECT', 'EMONMIN', 'EBINNOT', 'ELOGNOT', 'EFORCELOAD',
    'EMUL', 'EMULV', 'EDIV', 'EMODULO', 'EADDV', 'ESUBV', 'EADD', 'ESUB', 'ESHL', 'ESHR', 'ELESS', 'EGREATER',
    'ELESSEQU', 'EGREATEREQU', 'EEQU', 'ENOTEQU', 'EAND', 'EXOR', 'EOR', 'ELAND', 'ELOR', 'EASS', 'EMULASS',
    'EDIVASS', 'EMODASS', 'EADDASS', 'ESUBASS', 'ESHLASS', 'ESHRASS', 'EANDASS', 'EXORASS', 'EORASS', 'ECOMMA',
    'EMIN', 'EMAX', 'EPMODULO', 'EROTL', 'EROTR', 'EBCLR', 'EBTST', 'EBSET', 'ETYPCON', 'EBITFIELD',
    'EINTCONST', 'EFLOATCONST', 'E?54', 'ESTRINGCONST', 'ECOND', 'EFUNCCALL', 'EFUNCCALLP', 'EOBJREF',
    'ENULLCHECK', 'EPRECOMP', 'ELABEL', 'EGCCASM', 'ESCOPEBEGIN', 'ESCOPEEND', 'EINFO', 'EMFPOINTER', 'ETEMP',
    'ELOCOBJ', 'EARGOBJ', 'ESETCONST', 'ENEWEXCEPTION', 'ENEWEXCEPTIONARRAY', 'EINITTRYCATCH', 'EOBJACCESS',
    'ETEMPLDEP', 'EPOINTERSTAR', 'EDOTSTAR', 'ECTORINIT', 'ESTMT', 'E?81', 'E?82', 'EINSTRUCTION', 'EDEFINE',
    'EREUSE', 'EASSBLK', 'EVECTORCONST', 'ECONDASS']
K = {n: i for i, n in enumerate(ENODE_KINDS)}
MONADIC = {K[n] for n in ('EPOSTINC', 'EPOSTDEC', 'EPREINC', 'EPREDEC', 'EINDIRECT', 'EMONMIN', 'EBINNOT',
                          'ELOGNOT', 'EFORCELOAD', 'ETYPCON', 'EBITFIELD', 'EDEFINE')}
DIADIC = set(range(K['EMUL'], K['EBSET'] + 1)) | {K['ENULLCHECK'], K['EASSBLK']}
TRIADIC = {K['ECOND'], K['ECONDASS']}
CALLS = {K['EFUNCCALL'], K['EFUNCCALLP']}

ST_KINDS = {1: 'NOP', 2: 'LABEL', 3: 'GOTO', 4: 'EXPR', 5: 'SWITCH', 6: 'IFGOTO', 7: 'IFNGOTO', 8: 'RETURN',
            9: 'OVF', 10: 'EXIT', 11: 'ENTRY', 12: 'BEGINCATCH', 13: 'ENDCATCH', 14: 'ENDCATCHDTOR',
            15: 'GOTOEXPR', 16: 'ASM', 17: 'BEGINLOOP', 18: 'ENDLOOP'}
ST_HAS_EXPR = {4, 5, 6, 7, 8, 15}
ST_HAS_LABEL = {2, 3, 6, 7}

INTEGRAL = ['bool', 'char', 's8', 'u8', 'wchar_t', 's16', 'u16', 'int', 'uint', 'long', 'ulong', 'llong',
            'ullong', 'float', 'sdouble', 'double', 'ldouble']
DATATYPE = {0: 'global', 1: 'local', 2: 'abs', 3: 'func', 4: 'vfunc', 5: 'inline', 6: 'alias', 7: 'expr'}


class Dumper:
    def __init__(self, p, show_flags=False, max_depth=40):
        self.p, self.show_flags, self.max_depth = p, show_flags, max_depth

    def hname(self, h):
        return self.p.cstr(h + 0xA) if h else ''

    def type_name(self, t, depth=0):
        p = self.p
        if not t:
            return '<null>'
        kind, size = p.u8(t), struct.unpack('<i', p.rd(t + 2, 4))[0]
        if kind == 0:
            return 'void'
        if kind in (1, 2):
            code = p.u8(t + 6)
            return INTEGRAL[code] if code < len(INTEGRAL) else 'int?%d/%d' % (code, size)
        if kind == 4:
            return 'enum %s' % self.hname(p.u32(t + 0x12))
        if kind == 5:
            st = p.s8(t + 0x10)
            return ('vector %s' if 4 <= st <= 14 else 'struct %s') % self.hname(p.u32(t + 6))
        if kind == 6:
            return 'class %s' % self.hname(p.u32(t + 0xA))
        if kind == 7:
            return 'func'
        if kind == 8:
            return 'bitfield(%s:%d@%d)' % (self.type_name(p.u32(t + 6), depth + 1), p.s8(t + 0xB), p.s8(t + 0xA))
        if kind == 9:
            return 'label'
        if kind == 0xB:
            return 'memberptr'
        if kind in (0xC, 0xD):
            inner = self.type_name(p.u32(t + 6), depth + 1) if depth < 2 else '...'
            return '%s(%s)' % ('ptr' if kind == 0xC else 'array[%d]' % size, inner)
        return 'type%d/%d' % (kind, size)

    def node_data(self, n, kind):
        p = self.p
        if kind == K['EINTCONST']:
            hi, lo = p.u32(n + 0x10), p.u32(n + 0x14)
            v = struct.unpack('<q', struct.pack('<II', lo, hi))[0]
            return str(v) if -256 <= v <= 256 else '%d (0x%X)' % (v, v & 0xFFFFFFFFFFFFFFFF if v < 0 else v)
        if kind == K['EFLOATCONST']:
            return repr(struct.unpack('<d', p.rd(n + 0x10, 8))[0])
        if kind == K['ESTRINGCONST']:   # {s32 size; char *data} as in 1.2.5
            size, data = p.u32(n + 0x10), p.u32(n + 0x14)
            return repr(p.rd(data, min(size, 40)).split(b'\0')[0].decode('latin1')) if data and size < 1 << 20 else ''
        if kind == K['EOBJREF']:
            o = p.u32(n + 0x10)
            return '%s [%s]' % (p.objname(o), DATATYPE.get(p.u8(o + 2), 'dt%d' % p.u8(o + 2))) if o else '<null>'
        if kind == K['ELABEL']:
            lab = p.u32(n + 0x10)
            return self.hname(p.u32(lab + 0xC)) if lab else ''
        if kind in CALLS:
            f = p.u32(n + 0x10)
            if f and p.u8(f) == K['EOBJREF']:
                return 'call ' + p.objname(p.u32(f + 0x10))
            return 'call (indirect)'
        return ''

    def lines(self, n, prefix='', depth=0, seen=None, out=None):
        """Render the tree under ENode `n` as a list of indented lines."""
        p = self.p
        out = [] if out is None else out
        seen = set() if seen is None else seen
        ind = '  ' * depth
        if not n:
            out.append('%s%s<null>' % (ind, prefix))
            return out
        kind = p.u8(n)
        name = ENODE_KINDS[kind] if kind < len(ENODE_KINDS) else 'E?%d' % kind
        text = '%s%s%s  %s' % (ind, prefix, name, self.type_name(p.u32(n + 4)))
        data = self.node_data(n, kind)
        if data:
            text += '  ' + data
        if self.show_flags:
            text += '  {fl=%X ign=%d call=%d cost=%d}' % (p.u32(n + 8), p.u8(n + 2), p.u8(n + 3), p.u8(n + 1))
        if n in seen and kind not in (K['EOBJREF'], K['EINTCONST']):
            out.append(text + '  (shared node, shown above)')
            return out
        seen.add(n)
        out.append(text)
        if depth >= self.max_depth:
            out.append(ind + '  ...')
            return out
        if kind in MONADIC:
            self.lines(p.u32(n + 0x10), '', depth + 1, seen, out)
        elif kind in DIADIC:
            self.lines(p.u32(n + 0x10), '', depth + 1, seen, out)
            self.lines(p.u32(n + 0x14), '', depth + 1, seen, out)
        elif kind in TRIADIC:   # ECOND: cond ? expr1 : expr2. ECONDASS's operand roles are not traced: op0..2
            labs = ('if: ', 'then: ', 'else: ') if kind == K['ECOND'] else ('op0: ', 'op1: ', 'op2: ')
            for off, lab in zip((0x10, 0x14, 0x18), labs):
                self.lines(p.u32(n + off), lab, depth + 1, seen, out)
        elif kind in CALLS:
            f = p.u32(n + 0x10)
            if f and p.u8(f) != K['EOBJREF']:
                self.lines(f, 'fn: ', depth + 1, seen, out)
            a, i = p.u32(n + 0x14), 0
            while a and i < 64:
                self.lines(p.u32(a + 4), 'arg%d: ' % i, depth + 1, seen, out)
                a, i = p.u32(a), i + 1
        elif kind == K['EREUSE']:
            out[-1] += '  (reuses EDEFINE %08X)' % p.u32(n + 0x10)
        return out

    def statements(self, head):
        """[(stmt address, header, [tree lines])] for the statement list after `head`."""
        p, res, s, idx = self.p, [], self.p.u32(head), 0
        while s and idx < 100000:
            kind = p.u8(s + 4)
            hdr = 'S%d %s' % (idx, ST_KINDS.get(kind, 'ST%d' % kind))
            if kind in ST_HAS_LABEL:
                lab = p.u32(s + 0xE)
                if lab:
                    hdr += (' ' if kind == 2 else ' -> ') + self.hname(p.u32(lab + 0xC))
            hdr += '  line %d' % struct.unpack('<i', p.rd(s + 0x1E, 4))[0]
            tree = []
            if kind in ST_HAS_EXPR and p.u32(s + 0xA):
                tree = self.lines(p.u32(s + 0xA), depth=1)
            res.append((s, hdr, tree))
            s, idx = p.u32(s), idx + 1
        return res


def main():
    if sys.platform != 'win32':
        raise SystemExit('enode.py debugs the Windows compiler binary: run it on Windows')
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('--stage', choices=('pre-toc', 'post-toc', 'both'), default='both',
                    help='trees before TOC rewrites them, after (what instruction selection sees), or both')
    ap.add_argument('--full', action='store_true', help='with --stage both, repeat unchanged statements post-toc')
    ap.add_argument('--flags', action='store_true', help='also print node flags, ignored/hascall bytes and cost')
    ap.add_argument('--grep', help='only statements whose tree text matches this regex (either stage)')
    ap.add_argument('--no-labels', action='store_true', help='hide NOP/LABEL/GOTO statements without a tree')
    a = ap.parse_args()
    fpat = re.compile(a.func)
    gpat = re.compile(a.grep) if a.grep else None

    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'enode')
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)
    state = {'func': '', 'pre': {}, 'matched': []}

    def keep(hdr, tree):
        if a.no_labels and not tree:
            return False
        return True

    def emit(stage, sts, pre=None):
        print('=== %s: %s (%d statements)' % (state['func'], stage, len(sts)))
        for s, hdr, tree in sts:
            if not keep(hdr, tree):
                continue
            if gpat and not (gpat.search('\n'.join(tree)) or (pre and s in pre and gpat.search(pre[s]))):
                continue
            if pre is not None and not a.full:
                if pre.get(s) == '\n'.join(tree):
                    if tree:
                        print('  %s  (unchanged by TOC)' % hdr)
                    continue
                hdr += '  (changed by TOC)'
            print('  ' + hdr)
            for line in tree:
                print('  ' + line)
        print()

    def on_generator(p, ctx):
        fo = p.u32(dbgcore.arg(p, ctx, 1) + 4)
        state['func'] = p.objname(fo) if fo else '__sinit'
        state['pre'] = {}

    def on_pre(p, ctx):
        if not fpat.search(state['func']):
            return
        sts = Dumper(p, a.flags).statements(dbgcore.arg(p, ctx, 1))
        state['pre'] = {s: '\n'.join(tree) for s, _, tree in sts}
        state['matched'].append(state['func'])
        if a.stage in ('pre-toc', 'both'):
            emit('before TOC (after the IR optimizer)', sts)

    def on_post(p, ctx):
        if not fpat.search(state['func']) or a.stage == 'pre-toc':
            return
        head = p.u32(dbgcore.esp(ctx))   # the pushed argument is still on the stack
        sts = Dumper(p, a.flags).statements(head)
        emit('after TOC (input to instruction selection)', sts, state['pre'] if a.stage == 'both' else None)

    code = dbgcore.run(argv, {BP_GENERATOR: on_generator, BP_PRE_TOC: on_pre, BP_POST_TOC: on_post},
                       cwd=mwcc_cmd.ROOT)
    if not state['matched']:
        print('no function matched %r (compiler exit code %d)' % (a.func, code), file=sys.stderr)
    elif code:
        print('compiler exit code %d' % code, file=sys.stderr)


if __name__ == '__main__':
    main()
