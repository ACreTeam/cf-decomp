"""Try many source/header rewrites (and their combinations) in parallel, and rank them by score.

Every variant is compiled with the unit's real flags and scored in objdiff project mode (score.py), in its
own temp dir, so runs are independent and parallel. Nothing in the repo or in build/RUUE01_00 changes.

    python tools/mwcc/variants.py src/d_fish_fieldNP/d_fish_field.cpp rewrites.py -f "isRecChanged"
    python tools/mwcc/variants.py <src> rewrites.py -f "create__12" --combine 2 --jobs 8
    python tools/mwcc/variants.py <src> rewrites.py -f "spawn" --best-out best.cpp

The rewrites file is Python and defines V, a list of variants. Each variant is (name, edits) or
{'name': ..., 'edits': [...]}. An edit is either
    ("old text", "new text")                                  # applied to the source file
    ("include/game/game/x.hpp", "old text", "new text")       # applied to a header (via an overlay)
Each `old text` must occur exactly once (newlines are normalised to the file's style). For example:

    V = [
        ("int-local", [("s32 target = -pitch;", "int target = -pitch;")]),
        ("getter", [("include/game/game/d_fish_field.hpp", "u8 getRecId() const", "int getRecId() const")]),
    ]

Output, best first: the summed score of the -f functions, each -f function's score, how many functions in the
whole unit are exact, and any function that got worse than the unmodified source (a "regression").
--combine N also tries every combination of up to N variants (edits from different variants must not
overlap). The unmodified source is always run first as the baseline.
"""
import argparse
import concurrent.futures
import itertools
import os
import re
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mwcc_cmd  # noqa: E402
import score  # noqa: E402

ROOT = mwcc_cmd.ROOT


def load_variants(path):
    ns = {}
    exec(compile(open(path, encoding='utf-8').read(), path, 'exec'), ns)
    out = []
    for v in ns['V']:
        if isinstance(v, dict):
            name, edits = v['name'], v['edits']
        else:
            name, edits = v
        out.append((name, [tuple(e) for e in edits]))
    return out


def _read(path):
    return open(path, encoding='cp932', errors='surrogateescape', newline='').read()


def _write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, 'w', encoding='cp932', errors='surrogateescape', newline='').write(text)


def _apply(text, old, new, where):
    nl = '\r\n' if '\r\n' in text else '\n'
    old = old.replace('\r\n', '\n').replace('\n', nl)
    new = new.replace('\r\n', '\n').replace('\n', nl)
    n = text.count(old)
    if n != 1:
        raise ValueError('%s: expected exactly 1 match, found %d: %r' % (where, n, old[:60]))
    return text.replace(old, new, 1)


def overlay_path(header):
    """Repo header path -> path inside an overlay include root (include/lib/X and include/X map to X)."""
    h = header.replace('\\', '/')
    for root in ('include/lib/', 'include/'):
        if h.startswith(root):
            return h[len(root):]
    raise ValueError('header edits must be under include/: %s' % header)


def build_variant(src, edits, work):
    """Write the variant's source copy and header overlay into `work`. Returns (input_file, overlay)."""
    text = _read(os.path.join(ROOT, src))
    headers = {}
    for e in edits:
        if len(e) == 2:
            text = _apply(text, e[0], e[1], src)
        else:
            hp, old, new = e
            if hp not in headers:
                headers[hp] = _read(os.path.join(ROOT, hp))
            headers[hp] = _apply(headers[hp], old, new, hp)
    inp = os.path.join(work, os.path.basename(src))
    _write(inp, text)
    overlay = None
    if headers:
        overlay = os.path.join(work, 'ov')
        for hp, t in headers.items():
            _write(os.path.join(overlay, overlay_path(hp)), t)
    return inp, overlay


def run_one(src, name, edits, fre, base_dir):
    work = tempfile.mkdtemp(prefix='v_', dir=base_dir)
    try:
        inp, ov = build_variant(src, edits, work)
    except ValueError as ex:
        return {'name': name, 'ok': False, 'error': str(ex)}
    res = score.score_unit(src, inp, ov, work=work)
    if not res['ok']:
        return {'name': name, 'ok': False, 'error': res['error'].strip().splitlines()[-1:] or ['compile error']}
    scores = {n: s for s, n, _, _ in res['results'] if s is not None}
    focus = {n: s for n, s in scores.items() if fre.search(n)}
    return {'name': name, 'ok': True, 'scores': scores, 'focus': focus, 'edits': edits, 'work': work}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as objdiff.json/ninja know it')
    ap.add_argument('rewrites', help='Python file defining V (see the docstring)')
    ap.add_argument('-f', '--func', required=True, help='regex on our mangled names: the functions to optimise')
    ap.add_argument('--combine', type=int, default=1, help='also try combinations of up to N variants')
    ap.add_argument('--max', type=int, default=500, help='cap on the number of runs')
    ap.add_argument('--jobs', type=int, default=max(1, (os.cpu_count() or 4) // 2))
    ap.add_argument('--top', type=int, default=20, help='how many results to print')
    ap.add_argument('--best-out', help='write the best variant\'s source here (and its overlay next to it)')
    a = ap.parse_args()
    fre = re.compile(a.func)
    singles = load_variants(a.rewrites)
    runs = [('(base)', [])] + [(n, e) for n, e in singles]
    for k in range(2, a.combine + 1):
        for combo in itertools.combinations(singles, k):
            runs.append((' + '.join(n for n, _ in combo), [e for _, es in combo for e in es]))
    if len(runs) > a.max:
        print('note: %d runs, capped to %d (--max)' % (len(runs), a.max))
        runs = runs[:a.max]
    base_dir = tempfile.mkdtemp(prefix='variants_', dir=os.path.join(ROOT, 'build', 'mwcc_tools'))
    print('%d runs, %d jobs, work dir %s' % (len(runs), a.jobs, base_dir))
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        results = list(ex.map(lambda r: run_one(a.src, r[0], r[1], fre, base_dir), runs))
    base = results[0]
    if not base['ok']:
        raise SystemExit('the unmodified source failed: %s' % base['error'])
    names = sorted(base['focus'])
    for r in results:
        if r['ok']:
            r['total'] = sum(r['focus'].get(n, 0) for n in names)
            r['exact'] = sum(1 for s in r['scores'].values() if s >= 100)
            # worse than the baseline, or no longer paired with the target (e.g. its mangled name changed)
            r['regress'] = sorted([n for n, s in r['scores'].items()
                                   if n in base['scores'] and s < base['scores'][n] - 1e-9 and not fre.search(n)] +
                                  ['%s (unpaired)' % n for n in base['scores']
                                   if n not in r['scores'] and not fre.search(n)])
    ok = [r for r in results if r['ok']]
    ok.sort(key=lambda r: (-r['total'], len(r['regress']), r['name'] != '(base)'))
    short = lambda n: n.split('__')[0] if '__' in n[2:] else n
    print('\n%8s  %-30s %s  exact  regressions' % ('total', 'variant', '  '.join('%10s' % short(n)[:10] for n in names)))
    for r in ok[:a.top]:
        print('%8.2f  %-30s %s  %5d  %s' % (r['total'], r['name'][:30],
                                            '  '.join('%10.2f' % r['focus'].get(n, 0) for n in names), r['exact'],
                                            ', '.join(short(n) for n in r['regress'][:4]) + (' ...' if len(r['regress']) > 4 else '')))
    bad = [r for r in results if not r['ok']]
    for r in bad:
        print('  FAILED %-30s %s' % (r['name'][:30], r['error']))
    if a.best_out and ok:
        best = ok[0]
        shutil.copy2(os.path.join(best['work'], os.path.basename(a.src)), a.best_out)
        ov = os.path.join(best['work'], 'ov')
        if os.path.isdir(ov):
            dst = os.path.splitext(a.best_out)[0] + '_ov'
            shutil.copytree(ov, dst, dirs_exist_ok=True)
            print('best: %s -> %s (overlay %s)' % (best['name'], a.best_out, dst))
        else:
            print('best: %s -> %s' % (best['name'], a.best_out))


if __name__ == '__main__':
    main()
