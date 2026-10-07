"""Compile a source (or a variant of it) without ninja and score its functions against the target.

Parallel-safe: nothing in build/RUUE01_00 is touched, so several agents can test variants at once.
The compile uses the unit's real ninja flags (mwcc_cmd.py), and the target object and symbol mappings
come from objdiff.json. Each matching function is diffed with objdiff-cli.

    python tools/mwcc/score.py src/d_fish_fieldNP/d_fish_field.cpp                 # every function
    python tools/mwcc/score.py <src> -f "spawn|calcMtx" --diff                     # + instruction diff
    python tools/mwcc/score.py <src> --input variant.cpp -I overlay_dir -f spawn --diff
    python tools/mwcc/score.py <src> --map "oldTargetName=ourName" ...             # extra name pairs

Output: "score  name" per function, below-100 ones last; with --diff, the target | ours instruction
lines (">>" marks a difference). Run ninja for the real report.json; this is for fast iteration.
"""
import argparse
import json
import os
import re
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mwcc_cmd  # noqa: E402

ROOT = mwcc_cmd.ROOT


def unit_for(src):
    want = os.path.normcase(os.path.normpath(os.path.relpath(os.path.abspath(src), ROOT)))
    d = json.load(open(os.path.join(ROOT, 'objdiff.json'), encoding='utf-8'))
    for u in d['units']:
        sp = u.get('metadata', {}).get('source_path')
        if sp and os.path.normcase(os.path.normpath(sp)) == want:
            return u
    raise SystemExit('%s is not a unit in objdiff.json (run configure.py?)' % src)


def functions(path):
    d = open(path, 'rb').read()
    shoff, = struct.unpack('>I', d[0x20:0x24])
    es, n, _ = struct.unpack('>HHH', d[0x2E:0x34])
    secs = [struct.unpack('>IIIIIIIIII', d[shoff + i * es:shoff + i * es + 40]) for i in range(n)]
    out = []
    for s in secs:
        if s[1] != 2:
            continue
        strtab = secs[s[6]]
        for k in range(0, s[5], 16):
            nm, _, _, info, _, shn = struct.unpack('>IIIBBH', d[s[4] + k:s[4] + k + 16])
            if info & 0xF == 2 and shn:
                o = strtab[4] + nm
                out.append(d[o:d.index(b'\0', o)].decode('latin1'))
    return out


def rename_symbols(path, mapping, dst):
    """Copy the ELF `path` to `dst` with symbols renamed per `mapping` (ours -> target), by appending
    a new string table."""
    d = bytearray(open(path, 'rb').read())
    shoff, = struct.unpack('>I', d[0x20:0x24])
    es, n, _ = struct.unpack('>HHH', d[0x2E:0x34])
    secs = [list(struct.unpack('>IIIIIIIIII', d[shoff + i * es:shoff + i * es + 40])) for i in range(n)]
    for s in secs:
        if s[1] != 2:
            continue
        st = secs[s[6]]
        names = []
        for k in range(0, s[5], 16):
            nm, = struct.unpack('>I', d[s[4] + k:s[4] + k + 4])
            o = st[4] + nm
            names.append(d[o:d.index(b'\0', o)].decode('latin1'))
        new, offs = bytearray(), []
        for nmx in names:
            offs.append(len(new))
            new += mapping.get(nmx, nmx).encode('latin1') + b'\0'
        for k in range(len(names)):
            struct.pack_into('>I', d, s[4] + k * 16, offs[k])
        while len(d) % 4:
            d += b'\0'
        st[4], st[5] = len(d), len(new)
        d += new
        struct.pack_into('>IIIIIIIIII', d, shoff + s[6] * es, *st)
    open(dst, 'wb').write(d)


def score_unit(src, input_file=None, overlay=None, maps=(), diff_funcs=None, work=None, func_regex='.'):
    """Compile `src` (or `input_file` with src's flags, plus an optional overlay include dir) and score it in
    objdiff project mode. Returns {'ok': bool, 'error': str, 'results': [(score|None, our_name, note,
    diff_lines)]}. Instruction diffs are produced for our function names in `diff_funcs` (a set) that
    score below 100. Usable as a library (variants.py)."""
    unit = unit_for(src)
    target = os.path.join(ROOT, unit['target_path'])
    ours_to_target = {v: k for k, v in unit.get('symbol_mappings', {}).items()}
    for m in maps:
        t, o = m.split('=', 1)
        ours_to_target[o] = t
    if work is None:
        os.makedirs(os.path.join(ROOT, 'build', 'mwcc_tools'), exist_ok=True)
        work = tempfile.mkdtemp(prefix='score_', dir=os.path.join(ROOT, 'build', 'mwcc_tools'))
    argv = mwcc_cmd.compile_argv(src, overlay=overlay, input_file=input_file, outdir=work)
    r = mwcc_cmd.run(argv)
    obj = mwcc_cmd.object_path(argv)
    if r.returncode != 0 or not os.path.exists(obj):
        return {'ok': False, 'error': r.stdout[-3000:], 'results': []}
    tnames = set(functions(target))
    # a mapping only applies when our name isn't already in the target (objdiff.json can carry stale ones)
    ours_to_target = {o: t for o, t in ours_to_target.items() if o not in tnames and t in tnames}
    renames = {o: t for o, t in ours_to_target.items() if o != t}
    diffobj = obj
    if renames:
        diffobj = os.path.join(work, 'renamed.o')
        rename_symbols(obj, renames, diffobj)
    objdiff = os.path.join(ROOT, 'build', 'tools', 'objdiff-cli.exe' if os.name == 'nt' else 'objdiff-cli')
    # Score in objdiff *project* mode, exactly like the build's report.json (one-shot `diff -1 -2` pairs
    # data relocations by name and flags label-only differences). A one-unit temp project points at the
    # real target and at our object.
    proj_dir = os.path.join(work, 'proj')
    os.makedirs(proj_dir, exist_ok=True)
    rel = lambda p: os.path.relpath(p, proj_dir).replace(os.sep, '/')
    full = json.load(open(os.path.join(ROOT, 'objdiff.json'), encoding='utf-8'))
    u = dict(unit, target_path=rel(target), base_path=rel(diffobj), symbol_mappings={})
    u.pop('scratch', None)
    proj = {k: v for k, v in full.items() if k not in ('units', 'custom_make', 'build_target', 'watch_patterns')}
    proj['units'] = [u]
    json.dump(proj, open(os.path.join(proj_dir, 'objdiff.json'), 'w'), indent=1)
    rep_path = os.path.join(work, 'report.json')
    r = subprocess.run([objdiff, 'report', 'generate', '-p', proj_dir, '-o', rep_path],
                       cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors='replace')
    if r.returncode != 0:
        return {'ok': False, 'error': 'objdiff report failed:\n' + r.stdout[-2000:], 'results': []}
    rep = json.load(open(rep_path))
    scores = {f['name']: f.get('fuzzy_match_percent', 0) for ru in rep['units'] for f in ru.get('functions', [])}
    fre = re.compile(func_regex)
    results = []
    for ours in functions(obj):
        if not fre.search(ours):
            continue
        tname = ours_to_target.get(ours, ours)
        if tname not in tnames:
            results.append((None, ours, 'not in target (unreferenced weak copy, or pass --map target=ours)', []))
            continue
        score = scores.get(tname)
        if score is None:
            results.append((None, ours, 'not paired by objdiff', []))
            continue
        lines = []
        if diff_funcs and ours in diff_funcs and score < 100:
            out = os.path.join(work, 'diff.json')
            subprocess.run([objdiff, 'diff', '-p', proj_dir, '-u', u['name'], '-o', out, tname],
                           cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                j = json.load(open(out))
                L = next((x for x in j['left']['symbols'] if x.get('name') == tname), None)
                R = next((x for x in j['right']['symbols'] if x.get('name') == tname), None)
                for x, y in zip(L.get('instructions', []), R.get('instructions', [])):
                    fa = x.get('instruction', {}).get('formatted', '')
                    fb = y.get('instruction', {}).get('formatted', '')
                    mark = '>>' if (x.get('diff_kind') or y.get('diff_kind')) else '  '
                    lines.append('%s %-44s| %s' % (mark, fa, fb))
            except Exception:
                lines.append('   (objdiff diff failed)')
        results.append((score, ours, '', lines))
    return {'ok': True, 'error': '', 'results': results}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as objdiff.json/ninja know it')
    ap.add_argument('--input', help='compile this file instead (a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('-f', '--func', default='.', help='regex on our mangled function names')
    ap.add_argument('--diff', action='store_true', help='print instruction diffs for functions below 100')
    ap.add_argument('--map', action='append', default=[], help='extra "targetName=ourName" pairs')
    a = ap.parse_args()
    fre = re.compile(a.func)
    res = score_unit(a.src, a.input, a.overlay, a.map, func_regex=a.func,
                     diff_funcs=_AllNames() if a.diff else None)
    if not res['ok']:
        sys.stderr.write(res['error'])
        raise SystemExit('compile or scoring failed')
    results = res['results']
    results.sort(key=lambda t: (t[0] is not None and t[0] < 100, -(t[0] or 0) if t[0] is not None else 1))
    for score, name, note, lines in results:
        if score is None:
            print('   ---- %s (%s)' % (name, note))
            continue
        print('%7.2f %s' % (score, name))
        if a.diff and score < 100:
            print('\n'.join(lines))


class _AllNames:
    def __contains__(self, item):
        return True

    def __bool__(self):
        return True


if __name__ == '__main__':
    main()
