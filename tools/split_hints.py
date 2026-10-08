"""Hints for splitting an unassigned (auto_*) .text range of the DOL into real translation units.

    python tools/split_hints.py 0x80163AA0 0x80167C08             # per-function table + suggested cuts
    python tools/split_hints.py 0x80163AA0 0x80167C08 --cuts-only
    python tools/split_hints.py 0x80163AA0 0x80167C08 --data      # also the data objects each function uses

It reads the asm dtk wrote (build/RUUE01_00/asm, newest file wins when stale files overlap) and
config/RUUE01_00/symbols.txt, and prints one line per function: address, size, name, and the
address range it references in each data section (.sdata2/.rodata/.data/.sdata/.bss/.sbss).
Calls to functions inside the range are listed too (static helpers stay in their TU).

Why that finds TU boundaries: MWCC emits each object's constants/strings/jump tables in its own
contiguous block per section, and the linker lays the blocks out in the same object order as the
.text. So walking the functions in address order, the private data they touch only moves
forward; a boundary between two TUs is a place where every section's "max referenced so far"
is below the next functions' "min referenced". Other evidence to weigh (the tool prints what it
can see):
  - __sinit_* functions end a TU's normal code (with -sym on, weak inlines follow them);
    .ctors has one entry per TU with static init.
  - jumptable_* in .data and float constants in .sdata2 are always private to their TU.
  - class/RTTI names in mangled symbols (the main class of a TU), and file name strings.
  - Data shared between TUs (globals referenced from many TUs) is ignored by --cuts: only
    objects referenced from a single cluster count.
"""
import argparse
import bisect
import collections
import glob
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SECS = ['.sdata2', '.rodata', '.data', '.sdata', '.bss', '.sbss']


def load_symbols():
    syms = {}
    for line in open(os.path.join(ROOT, 'config/RUUE01_00/symbols.txt'), encoding='utf-8', errors='replace'):
        m = re.match(r'(\S+) = (\.\w+):0x([0-9A-Fa-f]+);(?:.*size:0x([0-9A-Fa-f]+))?', line)
        if m:
            syms[m.group(1)] = (m.group(2), int(m.group(3), 16), int(m.group(4) or '0', 16))
    return syms


def section_ranges():
    """Section ranges from symbols.txt-independent source: the splits' 'Sections' header is not
    always present, so derive from symbol addresses."""
    return None


def load_functions(lo, hi):
    files = glob.glob(os.path.join(ROOT, 'build/RUUE01_00/asm/**/*.s'), recursive=True)
    files.sort(key=os.path.getmtime)  # newest last: overrides stale overlapping files
    funcs = {}
    for f in files:
        head = open(f, encoding='utf-8', errors='replace').read(400)
        m = re.search(r'# 0x([0-9A-F]+)\.\.0x([0-9A-F]+)', head)
        if not m or '.text' not in head:
            continue
        a, b = int(m.group(1), 16), int(m.group(2), 16)
        if b <= lo or a >= hi:
            continue
        cur = None
        for line in open(f, encoding='utf-8', errors='replace'):
            m = re.match(r'\.fn (\S+),', line)
            if m:
                cur = {'name': m.group(1), 'addr': None, 'refs': set(), 'calls': set(), 'size': 0}
                continue
            if cur is None:
                continue
            if line.startswith('.endfn'):
                if cur['addr'] is not None and lo <= cur['addr'] < hi:
                    funcs[cur['addr']] = cur
                cur = None
                continue
            m = re.match(r'/\* ([0-9A-F]{8}) ', line)
            if m:
                ad = int(m.group(1), 16)
                if cur['addr'] is None:
                    cur['addr'] = ad
                cur['size'] = ad + 4 - cur['addr']
                for r in re.findall(r'([A-Za-z_$@.][\w$@.<>:,~\-]*)@(?:ha|l|sda21|h)\b', line):
                    cur['refs'].add(r)
                mb = re.search(r'\bb(?:l)?\s+([A-Za-z_][\w$@.<>:,~\-]*)\s*$', line)
                if mb and not mb.group(1).startswith('.L'):
                    cur['calls'].add(mb.group(1))
    return [funcs[k] for k in sorted(funcs)]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('start', type=lambda x: int(x, 16))
    ap.add_argument('end', type=lambda x: int(x, 16))
    ap.add_argument('--cuts-only', action='store_true')
    ap.add_argument('--data', action='store_true', help='list the data objects each function references')
    ap.add_argument('--min-score', type=int, default=2, help='sections that must agree for a suggested cut')
    a = ap.parse_args()
    syms = load_symbols()
    funcs = load_functions(a.start, a.end)
    if not funcs:
        raise SystemExit('no functions found in range (run ninja so the asm exists)')
    names_in = {f['name'] for f in funcs}
    # who references each data object (to skip shared globals)
    users = collections.defaultdict(set)
    for i, f in enumerate(funcs):
        for r in f['refs']:
            users[r].add(i)
    for f in funcs:
        f['sec'] = collections.defaultdict(list)
        for r in f['refs']:
            s = syms.get(r)
            if s and s[0] in SECS:
                f['sec'][s[0]].append((s[1], r))
    # suggested cuts
    cuts = []
    for i in range(1, len(funcs)):
        score, detail = 0, []
        for sec in SECS:
            prev = [ad for f in funcs[:i] for ad, r in f['sec'][sec] if max(users[r]) < i]
            nxt = [ad for f in funcs[i:] for ad, r in f['sec'][sec] if min(users[r]) >= i]
            if prev and nxt:
                if max(prev) < min(nxt):
                    score += 1
                    detail.append(sec)
                else:
                    score -= 1
        sinit = funcs[i - 1]['name'].startswith('__sinit')
        if score >= a.min_score or sinit:
            cuts.append((funcs[i]['addr'], score, detail, sinit))
    if not a.cuts_only:
        cutset = {c[0] for c in cuts}
        for i, f in enumerate(funcs):
            if f['addr'] in cutset:
                c = [c for c in cuts if c[0] == f['addr']][0]
                print('---- possible TU boundary (score %d: %s%s)' % (c[1], ','.join(c[2]), ', after __sinit' if c[3] else ''))
            parts = []
            for sec in SECS:
                ads = sorted(ad for ad, r in f['sec'][sec])
                if ads:
                    parts.append('%s %08X..%08X' % (sec[1:], ads[0], ads[-1]) if len(ads) > 1 else '%s %08X' % (sec[1:], ads[0]))
            local = sorted(c for c in f['calls'] if c in names_in)
            print('%08X %5X %-50s %s%s' % (f['addr'], f['size'], f['name'][:50], ' | '.join(parts),
                                            ('  calls:' + ','.join(local)[:80]) if local else ''))
            if a.data:
                for sec in SECS:
                    for ad, r in sorted(f['sec'][sec]):
                        print('            %-7s %08X %s' % (sec, ad, r))
    print('\nsuggested cuts (score = data sections that agree, minus those that disagree):')
    for ad, sc, det, sinit in cuts:
        print('  %08X  score %d  %s%s' % (ad, sc, ','.join(det), '  (after __sinit)' if sinit else ''))


if __name__ == '__main__':
    main()
