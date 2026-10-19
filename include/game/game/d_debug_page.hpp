#ifndef D_DEBUG_PAGE_HPP
#define D_DEBUG_PAGE_HPP

#include <types.h>

// Debug-menu page interface. The method names are inferred.
// Pages that use the default implementations include d_debug_page_inline.hpp.
class dDebugPage_c {
  public:
    virtual void open();
    virtual void close();
    virtual void execute();
    virtual BOOL isActive();
}; // size 0x4

#endif
