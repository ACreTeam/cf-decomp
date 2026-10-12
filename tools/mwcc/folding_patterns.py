"""Survey constant-folding barriers in the canHide matching context.

Run from the repository root. Uses the configured compiler and score.py;
does not change production sources. Results are specific to this context.
"""
import argparse
import concurrent.futures
import json
from pathlib import Path

import score

SOURCE = 'src/dol/game/d_animal.cpp'
FUNCTION = 'canHide__14dAnimalBlock_cFiP14dPrivateData_c'


def cases():
    out = []

    def add(family, name, statement, argument='BOOL value', returns='BOOL', declarations=''):
        helper = declarations + '\nstatic inline ' + returns + ' makeHideResult(' + argument + ') { ' + statement + ' }\n'
        out.append(dict(family=family, name=name, helper=helper))

    for expression in ['value', '*(&value)', 'static_cast<BOOL>(value)',
                       'static_cast<const BOOL&>(value)', '!!value',
                       'value ? TRUE : FALSE', 'value + 0', 'value | 0',
                       'value ^ 0', 'value & 1', 'value * 1', '(0, value)']:
        add('scalar', expression, 'return ' + expression + ';')
    for typ in ['BOOL', 'bool', 's32', 'u32', 's16', 'u16', 's8', 'u8', 's64', 'u64', 'float', 'double']:
        add('return type', typ, 'return value;', returns=typ)
    for arg in ['const BOOL value', 'BOOL const value', 'const BOOL &value', 'volatile BOOL value']:
        add('argument', arg, 'return value;', argument=arg)
    for cast in ['static_cast', 'reinterpret_cast', 'const_cast']:
        for typ in ['BOOL', 'const BOOL', 'volatile BOOL', 'const volatile BOOL']:
            add('pointer cast', cast + '<' + typ + '*>', 'return *' + cast + '<' + typ + '*>(&value);')
    for typ in ['BOOL', 'const BOOL', 'volatile BOOL', 'const volatile BOOL', 'unsigned int', 'const unsigned int', 'const s16', 'const u16', 'const s8', 'const u8']:
        expr = '*(' + typ + '*)&value'
        if typ in ['const s16', 'const u16']:
            expr = '((' + typ + '*)&value)[1]'
        if typ in ['const s8', 'const u8']:
            expr = '((' + typ + '*)&value)[3]'
        add('C pointer cast', typ, 'return ' + expr + ';')
    for arg in ['const BOOL value', 'const BOOL &value', 'BOOL value']:
        for expr in ['*static_cast<const BOOL*>(&value)', '*reinterpret_cast<const BOOL*>(&value)',
                     '*static_cast<BOOL*>(static_cast<void*>(&value))',
                     '*static_cast<const BOOL*>(static_cast<const void*>(&value))']:
            # Avoid intentional const-removal errors in the void* case.
            if 'static_cast<void*>' in expr and arg != 'BOOL value':
                continue
            add('cast context', arg + ' / ' + expr, 'return ' + expr + ';', argument=arg)
    for statement in ['const BOOL *p = &value; return *p;',
                      'const BOOL *p = static_cast<const BOOL*>(&value); return *p;',
                      'BOOL *p = &value; return *static_cast<const BOOL*>(p);',
                      'const BOOL &r = value; return r;',
                      'BOOL copy = value; return *static_cast<const BOOL*>(&copy);',
                      'const BOOL copy = value; return *(&copy);',
                      'const BOOL copy = value; return *static_cast<const BOOL*>(&copy);',
                      'BOOL a[1] = {value}; return a[0];',
                      'BOOL a[1] = {value}; return *static_cast<const BOOL*>(a);',
                      'BOOL a[2] = {0, value}; return a[1];',
                      'BOOL a[2] = {0, value}; return static_cast<const BOOL*>(a)[1];',
                      'volatile BOOL copy = value; return copy;']:
        add('local storage', statement, statement)
    for expression in ['reinterpret_cast<const BOOL&>(value)', 'const_cast<const BOOL&>(value)']:
        add('reference cast', expression, 'return ' + expression + ';')
    for expression in ['*static_cast<const BOOL*>(&value)', 'static_cast<const BOOL&>(value)', 'value']:
        add('reference return', expression, 'return ' + expression + ';', argument='const BOOL &value', returns='const BOOL &')
    for expr in ['*static_cast<const BOOL*>(&value)', 'value']:
        for stmt in ['if (value) return EXPR; return FALSE;',
                     'if (!value) return FALSE; return EXPR;',
                     'switch (value) { case 0: return FALSE; default: return EXPR; }',
                     'return value ? EXPR : FALSE;']:
            add('multiple returns', stmt + ' / ' + expr, stmt.replace('EXPR', expr))
    for typ in ['BOOL', 'bool', 'u8', 'u16', 'u32', 's64', 'float', 'double']:
        for method in ['direct', 'getter', 'conversion', 'by value']:
            decl = 'struct FoldBox { ' + typ + ' v; FoldBox(BOOL x):v(x){} ' + typ + ' get() const {return v;} operator BOOL() const {return v;} };'
            stmt = {'direct': 'FoldBox b(value); return b.v;',
                    'getter': 'FoldBox b(value); return b.get();',
                    'conversion': 'FoldBox b(value); return b;',
                    'by value': 'return FoldBox(value);'}[method]
            add('aggregate', typ + ' / ' + method, stmt, returns='FoldBox' if method == 'by value' else 'BOOL', declarations=decl)
    for decl, stmt in [
        ('struct FoldBox { BOOL v; };', 'FoldBox b = {value}; return b.v;'),
        ('struct FoldBox { BOOL v; };', 'FoldBox b = {value}; return *static_cast<const BOOL*>(&b.v);'),
        ('struct FoldBox { BOOL v; BOOL spare; FoldBox(BOOL x):v(x),spare(0){} operator BOOL()const{return v;} };', 'return FoldBox(value);'),
        ('struct FoldBox { BOOL v; FoldBox(BOOL x):v(x){} ~FoldBox(){} operator BOOL()const{return v;} };', 'return FoldBox(value);'),
        ('struct FoldBox { unsigned v:1; FoldBox(BOOL x):v(x){} operator BOOL()const{return v;} };', 'return FoldBox(value);'),
        ('union FoldBox { BOOL v; unsigned u; };', 'FoldBox b; b.v=value; return b.v;'),
        ('union FoldBox { BOOL v; unsigned u; };', 'FoldBox b; b.v=value; return b.u;'),
    ]:
        add('aggregate extras', decl + ' / ' + stmt, stmt, declarations=decl, returns='FoldBox' if stmt == 'return FoldBox(value);' else 'BOOL')
    for helper in [
        'static inline BOOL foldRead(const BOOL *p) { return *p; }\nstatic inline BOOL makeHideResult(BOOL value) { return foldRead(&value); }',
        'static inline BOOL foldRead(BOOL *p) { return *static_cast<const BOOL*>(p); }\nstatic inline BOOL makeHideResult(BOOL value) { return foldRead(&value); }',
        'template<class T> static inline T foldRead(T value) { return *static_cast<const T*>(&value); }\nstatic inline BOOL makeHideResult(BOOL value) { return foldRead(value); }',
        'static inline BOOL foldRead(BOOL value) { return *static_cast<const BOOL*>(&value); }\nstatic inline BOOL makeHideResult(BOOL value) { return foldRead(value); }',
    ]:
        out.append(dict(family='helper nesting', name=helper, helper=helper + '\n'))
    for size in [1, 2, 3, 4]:
        for index in range(size):
            values = ['0'] * size
            values[index] = 'value'
            for read in ['a[' + str(index) + ']', '*(a + ' + str(index) + ')']:
                add('array offset', str(size) + ' elements / ' + read,
                    'BOOL a[' + str(size) + '] = {' + ','.join(values) + '}; return ' + read + ';')
    for field in ['first', 'second']:
        for read in ['b.' + field, 'b.get()']:
            decl = 'struct FoldPair { BOOL first,second; BOOL get()const{return ' + field + ';} };'
            add('member offset', field + ' / ' + read,
                'FoldPair b = {' + ('value,0' if field == 'first' else '0,value') + '}; return ' + read + ';', declarations=decl)
    for source_type, read_type in [('BOOL','BOOL'), ('BOOL','u32'), ('u32','BOOL'), ('BOOL','bool'), ('BOOL','u8')]:
        add('union member', source_type + ' -> ' + read_type,
            'FoldUnion b; b.a=value; return b.b;',
            declarations='union FoldUnion { ' + source_type + ' a; ' + read_type + ' b; };')
    for expr in ['static_cast<float>(value)', 'static_cast<double>(value)',
                 'static_cast<s64>(value)', 'static_cast<u64>(value)',
                 'static_cast<u8>(value)', 'value / 1', 'value % 2',
                 '(value << 1) >> 1', 'value == TRUE', 'value != FALSE']:
        add('scalar conversion', expr, 'return ' + expr + ';')
    for statement in ['static const BOOL x = TRUE; return x;',
                      'static BOOL x = TRUE; return x;',
                      'enum { x = TRUE }; return x;']:
        add('constant storage', statement, statement)
    cast_helper = 'static inline BOOL makeHideResult(BOOL value) { return *static_cast<const BOOL*>(&value); }\n'
    original = 'while (!makeHideResult(TRUE)) {\n        if (active) {\n            break;\n        }\n        return active;\n    }'
    for name, replacement in [
        ('for loop', 'for (; !makeHideResult(TRUE);) { if(active)break; return active; }'),
        ('local before while', 'BOOL result=makeHideResult(TRUE); while(!result) { if(active)break; return active; }'),
        ('if return active', 'if(!makeHideResult(TRUE))return active;'),
        ('if return false', 'if(!makeHideResult(TRUE))return FALSE;'),
        ('while return active', 'while(!makeHideResult(TRUE))return active;'),
        ('while return false', 'while(!makeHideResult(TRUE)){if(active)break;return FALSE;}'),
    ]:
        out.append(dict(family='caller shape', name=name, helper=cast_helper,
                        replace=[original, replacement]))
    return out


def fixture():
    source = Path(SOURCE).read_text(encoding='utf-8')
    includes = '\n'.join(line for line in source.splitlines() if line.startswith('#include')) + '\n'
    start = source.index('static inline BOOL isAppointmentIdx(')
    end = source.index('// HACK: Preserve', start)
    appointment = source[start:end]
    start = source.index('BOOL dAnimalBlock_c::canHide(')
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return includes + appointment, source[start:end] + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', default='build/mwcc_tools/folding_patterns')
    parser.add_argument('--jobs', type=int, default=6)
    parser.add_argument('--fixture', help='saved fixture.json, to avoid depending on current production source')
    parser.add_argument('--only', help='comma-separated case IDs (default: all cases)')
    args = parser.parse_args()
    root = Path(args.out)
    root.mkdir(parents=True, exist_ok=True)
    if args.fixture:
        saved = json.loads(Path(args.fixture).read_text(encoding='utf-8'))
        prefix, body = saved['prefix'], saved['body']
    else:
        prefix, body = fixture()
    (root / 'fixture.json').write_text(json.dumps(dict(prefix=prefix, body=body), indent=2))

    def run(item):
        index, case = item
        directory = root / str(index)
        directory.mkdir(exist_ok=True)
        path = directory / 'candidate.cpp'
        variant_body = body
        if 'replace' in case:
            before, after = case['replace']
            assert variant_body.count(before) == 1
            variant_body = variant_body.replace(before, after)
        path.write_text(prefix + case['helper'] + '\n' + variant_body, encoding='utf-8')
        result = score.score_unit(SOURCE, str(path), diff_funcs=[FUNCTION], work=str(directory))
        rows = [row for row in result.get('results', []) if row[1] == FUNCTION]
        record = dict(id=index, **case, path=str(path))
        if not result['ok'] or not rows:
            record['error'] = result.get('error', 'no function')
        else:
            record['score'] = rows[0][0]
            (directory / 'diff.txt').write_text('\n'.join(rows[0][3]), encoding='utf-8')
        return record

    selected = list(enumerate(cases()))
    if args.only:
        wanted = {int(n) for n in args.only.split(',')}
        selected = [(index, case) for index, case in selected if index in wanted]
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as executor:
        results = list(executor.map(run, selected))
    (root / 'results.json').write_text(json.dumps(results, indent=2), encoding='utf-8')
    for family in dict.fromkeys(row['family'] for row in results):
        rows = [r for r in results if r['family'] == family]
        print(family, 'exact', sum(r.get('score') == 100 for r in rows), '/', len(rows),
              'errors', sum('error' in r for r in rows))
    print('TOTAL', len(results), 'EXACT', sum(r.get('score') == 100 for r in results))


if __name__ == '__main__':
    main()
