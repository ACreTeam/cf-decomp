#ifndef D_DEBUG_PAGE_HPP
#define D_DEBUG_PAGE_HPP

#include <types.h>

// A page of the debug menu (RTTI "dDebugPage_c"). The special npcs (bug,
// catherine, fortune, franklin, cafe ...) derive their own page from it; the
// menu is stripped from this build, so every page method is an empty inline
// and nothing registers the pages (the RELs keep only the vtables). The
// method names are provisional.
class dDebugPage_c {
  public:
    // Weak copies come out in reverse declaration order.
    virtual void open() {
    }
    virtual void close() {
    }
    virtual void execute() {
    }
    virtual BOOL isActive() {
        return FALSE;
    }
}; // size 0x4

#endif
