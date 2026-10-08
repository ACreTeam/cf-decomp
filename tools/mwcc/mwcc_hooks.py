"""Shared debugger hooks for the MWCC introspection tools (GC 3.0a5.2, see dbgcore.py).

TempTracker records who made every @ temporary: create_temp_object (0052DFB0) is hooked at its RET
(0052E035, EAX = the new object, [esp] = return address into the caller), and the IRO pass boundaries
(005ED5E0 after a pass, 005ED680 before) label each temp with the pass that made it. Temps made before
CodeGen_Generator runs for a function come from the front end / inliner.

    tt = TempTracker()
    handlers.update(tt.handlers())          # always armed: temps are made before codegen starts
    ...
    tt.at_generator()                       # call from your CodeGen_Generator (00468750) handler
    tt.describe(p, obj)                     # 'inliner' / 'IRO_CommonSubs' / ... + creator function
"""
import dbgcore

BP_TEMP_RET = 0x52E035
BP_PASS_AFTER, BP_PASS_BEFORE = 0x5ED5E0, 0x5ED680


class TempTracker:
    def __init__(self):
        self.temps = {}      # Object* -> {'by': [function names], 'pass': str or None}
        self.pending = []

    def _on_temp_ret(self, p, ctx):
        obj = dbgcore.eax(ctx)
        names = [dbgcore.funcname(p.u32(dbgcore.esp(ctx)))]
        for _, nm in dbgcore.callers(p, ctx, depth=0x600, limit=8)[1:]:
            if nm.rstrip('?') != names[-1].rstrip('?'):
                names.append(nm)
        rec = {'by': names[:6], 'pass': None}
        self.temps[obj] = rec
        self.pending.append(rec)

    def _on_pass(self, before):
        def h(p, ctx):
            name = p.cstr(dbgcore.arg(p, ctx, 1), 80)
            for rec in self.pending:
                rec['pass'] = ('before ' if before else '') + name
            self.pending = []
        return h

    def handlers(self):
        return {BP_TEMP_RET: self._on_temp_ret, BP_PASS_AFTER: self._on_pass(False),
                BP_PASS_BEFORE: self._on_pass(True)}

    def at_generator(self):
        """Temps still unlabelled when codegen of a function starts were made by the front end."""
        self.pending = []
        for rec in self.temps.values():
            if rec['pass'] is None:
                rec['pass'] = 'front end'

    def describe(self, obj, chain=False):
        """'made by <function>, in <inliner | pass | front end>' for an @ object, or None."""
        tr = self.temps.get(obj)
        if not tr:
            return None
        src = tr['pass'] or 'front end'
        if any('inline' in f.lower() or f.startswith('CInline') for f in tr['by']):
            src = 'inliner'
        txt = 'made by %s, in %s' % (tr['by'][0], src)
        if chain:
            txt += '; chain: ' + ' <- '.join(tr['by'])
        return txt
