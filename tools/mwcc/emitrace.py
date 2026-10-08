"""Which compiler code emitted each PCode instruction of a function? (Windows only)

Breaks on PCodeUtilities_EmitInstruction (00582EC0) while a function whose name matches is generated
and prints the opcode with the compiler call chain that asked for it (innermost first, names from
mwcc30_names.txt). Use it to find out why an instruction exists at all, e.g. a dead `fcmpo` or an extra
`rlwinm`: the chain shows whether a condition generator, an ECOND/branchless path or an inline
expansion made it.

    python tools/mwcc/emitrace.py src/dol/game/d_bgcf.cpp "setSlope" --ops fcmpo,cror,mfcr
    python tools/mwcc/emitrace.py <src> <func regex> --ops all --depth 10 --input variant.cpp -I overlay

Only instructions created through EmitInstruction show up (most of the code generator's output; the
peephole passes, prologue/epilogue and register allocator spill code make theirs elsewhere).
"""
import argparse
import os
import re
import sys

import dbgcore
import mwcc_cmd

BP_EMIT = 0x582EC0        # PCodeUtilities_EmitInstruction(opcode, ...)
CUR_FUNC = 0x7109B4       # Object* of the function being generated


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('func', help='regex on the function name (as the compiler names it, usually unmangled)')
    ap.add_argument('--ops', default='fcmpo,fcmpu,cmpw,cmpwi,cmplw,cmplwi,mfcr,cror,rlwinm',
                    help='comma separated opcode names to report, or "all"')
    ap.add_argument('--depth', type=int, default=6, help='how many callers to print per instruction')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    a = ap.parse_args()
    if sys.platform != 'win32':
        raise SystemExit('emitrace.py debugs the Windows compiler binary: run it on Windows')
    _, v = mwcc_cmd.find_compile(a.src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('breakpoint addresses are only known for GC/3.0a5.2')
    fpat = re.compile(a.func)
    ops = None if a.ops == 'all' else set(a.ops.split(','))
    outdir = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools', 'emitrace')
    argv = mwcc_cmd.compile_argv(a.src, overlay=a.overlay, input_file=a.input, outdir=outdir)

    st = {'fname': None}

    def on_emit(p, ctx):
        fname = p.objname(p.u32(CUR_FUNC))
        if not fname or not fpat.search(fname):
            return
        if fname != st['fname']:
            st['fname'] = fname
            print('== %s' % fname)
        nm = p.opname(dbgcore.arg(p, ctx, 1) & 0xFFFF)
        if ops is None or nm in ops:
            chain = dbgcore.callers(p, ctx, limit=a.depth)
            print('  %-8s <- %s' % (nm, ' <- '.join(n for _, n in chain)))

    rc = dbgcore.run(argv, {BP_EMIT: on_emit}, cwd=mwcc_cmd.ROOT)
    if rc:
        print('compiler exited with %d' % rc)


if __name__ == '__main__':
    main()
