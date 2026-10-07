"""Reproduce the exact MWCC compile command ninja uses for a source file.

Shared by the compiler introspection tools in this folder (iro_dump.py, regalloc.py). The command line
is read from build.ninja, so DOL and REL units get their real flags. You can swap the compiler binary,
put an overlay include directory first, compile a scratch copy of the source, and choose where the
object goes.

    python tools/mwcc/mwcc_cmd.py src/d_fish_fieldNP/d_fish_field.cpp      # print the command
"""
import os
import re
import shlex
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))


def _ninja_statements(path):
    """Yield (outputs, rule, inputs, variables) for each `build` statement in build.ninja."""
    text = open(path, encoding='utf-8').read().replace('$\r\n', '').replace('$\n', '')
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('build '):
            head = line[len('build '):]
            outs, rest = head.split(':', 1)
            parts = rest.split()
            rule, ins = parts[0], []
            for p in parts[1:]:
                if p in ('|', '||'):
                    break
                ins.append(p)
            variables = {}
            i += 1
            while i < len(lines) and lines[i].startswith('  '):
                k, _, v = lines[i].strip().partition(' = ')
                variables[k] = v
                i += 1
            yield outs.split(), rule, ins, variables
            continue
        i += 1


def _norm(p):
    return os.path.normcase(os.path.normpath(p))


def find_compile(src):
    """Return (rule, variables) of the ninja statement that compiles `src` (path relative to the repo
    root, or absolute)."""
    want = _norm(os.path.relpath(os.path.abspath(src), ROOT))
    ninja = os.path.join(ROOT, 'build.ninja')
    top = {}
    for line in open(ninja, encoding='utf-8'):
        m = re.match(r'^(\w+) = (.*)$', line.rstrip('\n'))
        if m:
            top[m.group(1)] = m.group(2)
    for outs, rule, ins, variables in _ninja_statements(ninja):
        if rule.startswith('mwcc') and any(_norm(x) == want for x in ins):
            merged = dict(top)
            merged.update(variables)
            return rule, merged
    raise SystemExit('no mwcc compile statement for %s in build.ninja (run configure.py?)' % src)


def compile_argv(src, compiler=None, overlay=None, input_file=None, outdir=None):
    """argv list (run from ROOT) that compiles `src` like ninja does.

    compiler:   path to a mwcceppc.exe to use instead of build/compilers/<mw_version>/mwcceppc.exe
    overlay:    include directory searched before everything else (for header experiments)
    input_file: compile this file instead of `src` (e.g. a scratch copy), with `src`'s flags
    outdir:     output directory (default: a temp folder under build/mwcc_tools)
    """
    rule, v = find_compile(src)
    cflags = shlex.split(v.get('cflags', ''), posix=False)
    if overlay:
        cflags = ['-i', overlay] + cflags
    cc = compiler or os.path.join(ROOT, 'build', 'compilers', v['mw_version'], 'mwcceppc.exe')
    outdir = outdir or os.path.join(ROOT, 'build', 'mwcc_tools', 'obj')
    os.makedirs(outdir, exist_ok=True)
    argv = []
    if rule == 'mwcc_sjis':
        argv.append(os.path.join(ROOT, 'build', 'tools', 'sjiswrap.exe'))
    argv += [cc] + cflags + ['-c', input_file or src, '-o', outdir]
    return argv


def run(argv, **kw):
    """Run a compile from the repo root; returns CompletedProcess (stdout+stderr merged as text)."""
    return subprocess.run(argv, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                          errors='replace', **kw)


def object_path(argv):
    """The .o a compile_argv() command writes."""
    src = argv[argv.index('-c') + 1]
    outdir = argv[argv.index('-o') + 1]
    return os.path.join(outdir, os.path.splitext(os.path.basename(src))[0] + '.o')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    print(subprocess.list2cmdline(compile_argv(sys.argv[1])))
