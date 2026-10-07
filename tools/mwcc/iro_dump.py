"""Dump MWCC's optimizer (IRO) flowgraph after every pass, for one source file.

GC 3.0a5.2 has a built-in IRO dump that is switched off in the release binary. This tool makes a
patched copy of the compiler under build/mwcc_tools/iro_compiler/ (the original is never modified),
compiles a scratch copy of the source with the unit's real ninja flags, and filters the log.

    python tools/mwcc/iro_dump.py src/d_fish_fieldNP/d_fish_field.cpp -f "spawn__12dFishField"
    python tools/mwcc/iro_dump.py <src> -f <func regex> -p "ExpressionPropagation|DeadStore"
    python tools/mwcc/iro_dump.py <src> --input my_variant.cpp -I overlay_dir -f ... -o out.txt
    python tools/mwcc/iro_dump.py <src> --list            # functions and passes in the log

The log has `Starting function <name>` and `Dumping function <name> after <PASS> (tick N)` headers.
`-f` matches the function name as the log prints it (mangled or not, depending on the function).
At -O4 the pass loop runs twice ("tick"). Full logs are large (~95 MB for a big TU), so filter.

Patches (file offsets in mwcceppc.exe 3.0a5.2; see notes/mwcc_ghidra.md section 1c):
    0x1769B8  00 -> 01   VA 005775B2  mov byte [00725E5C],1   turns the IRO log on
    0x1EC9F4  75 -> EB   VA 005ED5F4  jnz -> jmp              dump after every pass
Use the patched copy for diagnosis only: never build with it.
"""
import argparse
import os
import re
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mwcc_cmd  # noqa: E402

PATCHES = [(0x1769B8, 0x00, 0x01), (0x1EC9F4, 0x75, 0xEB)]
WORK = os.path.join(mwcc_cmd.ROOT, 'build', 'mwcc_tools')


def patched_compiler(src):
    """Path of the patched mwcceppc.exe for `src`'s compiler version (created on first use)."""
    _, v = mwcc_cmd.find_compile(src)
    if v['mw_version'].replace('/', '\\') != 'GC\\3.0a5.2':
        raise SystemExit('the IRO patch offsets are only known for GC/3.0a5.2, not %s' % v['mw_version'])
    orig_dir = os.path.join(mwcc_cmd.ROOT, 'build', 'compilers', v['mw_version'])
    dst_dir = os.path.join(WORK, 'iro_compiler')
    exe = os.path.join(dst_dir, 'mwcceppc.exe')
    if os.path.exists(exe):
        return exe
    data = bytearray(open(os.path.join(orig_dir, 'mwcceppc.exe'), 'rb').read())
    for off, old, new in PATCHES:
        if data[off] != old:
            raise SystemExit('unexpected byte %02X at 0x%X (expected %02X): not the known 3.0a5.2 binary'
                             % (data[off], off, old))
        data[off] = new
    os.makedirs(dst_dir, exist_ok=True)
    for f in os.listdir(orig_dir):  # license DLLs etc. must sit next to the exe
        if f.lower() != 'mwcceppc.exe' and os.path.isfile(os.path.join(orig_dir, f)):
            shutil.copy2(os.path.join(orig_dir, f), dst_dir)
    open(exe, 'wb').write(data)
    return exe


def split_log(text):
    """[(function, header, body)] for every 'Dumping function' section, plus other lines attached to
    the preceding section."""
    out = []
    cur_fn, cur_head, buf = None, None, []
    for line in text.splitlines():
        m = re.match(r'(Starting|Dumping|Optimizing) function (.+?)( after (.*?))?( before .*)? \(tick \d+\)$', line) \
            or re.match(r'(Starting) function (.+)$', line)
        if m:
            if cur_head is not None:
                out.append((cur_fn, cur_head, '\n'.join(buf)))
            cur_fn, cur_head, buf = m.group(2), line, []
        else:
            buf.append(line)
    if cur_head is not None:
        out.append((cur_fn, cur_head, '\n'.join(buf)))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('src', help='source file as ninja knows it (its flags are used)')
    ap.add_argument('--input', help='compile this file instead (e.g. a variant), with src\'s flags')
    ap.add_argument('-I', dest='overlay', help='overlay include dir, searched first')
    ap.add_argument('-f', '--func', help='regex: only these functions')
    ap.add_argument('-p', '--pass', dest='passes', help='regex: only dumps after these passes')
    ap.add_argument('-o', '--out', help='write the (filtered) dump here instead of stdout')
    ap.add_argument('--list', action='store_true', help='list functions and their passes, then exit')
    a = ap.parse_args()

    cc = patched_compiler(a.src)
    tmp = os.path.join(WORK, 'iro')
    os.makedirs(tmp, exist_ok=True)
    copy = os.path.join(tmp, os.path.basename(a.input or a.src))
    shutil.copy2(os.path.join(mwcc_cmd.ROOT, a.input or a.src), copy)
    log = os.path.splitext(copy)[0] + '.log'
    for stale in (log, copy + '.log'):
        if os.path.exists(stale):
            os.remove(stale)
    argv = mwcc_cmd.compile_argv(a.src, compiler=cc, overlay=a.overlay, input_file=copy, outdir=tmp)
    r = mwcc_cmd.run(argv)
    if r.returncode != 0:
        sys.stderr.write(r.stdout[-3000:])
        raise SystemExit('compile failed')
    if not os.path.exists(log):
        log = copy + '.log' if os.path.exists(copy + '.log') else log
    text = open(log, encoding='latin1').read()
    sections = split_log(text)
    if a.list:
        seen = {}
        for fn, head, _ in sections:
            seen.setdefault(fn, []).append(head.split(' after ')[-1] if ' after ' in head else head.split()[0])
        for fn, ps in seen.items():
            print('%s  (%d dumps)' % (fn, len(ps)))
        return
    fre = re.compile(a.func) if a.func else None
    pre = re.compile(a.passes) if a.passes else None
    out = []
    for fn, head, body in sections:
        if fre and not fre.search(fn or ''):
            continue
        if pre and not pre.search(head):
            continue
        out.append(head + '\n' + body)
    res = '\n'.join(out)
    if a.out:
        open(a.out, 'w', encoding='utf-8').write(res)
        print('%d sections, %d bytes -> %s (full log: %s)' % (len(out), len(res), a.out, log))
    else:
        sys.stdout.write(res + '\n')


if __name__ == '__main__':
    main()
