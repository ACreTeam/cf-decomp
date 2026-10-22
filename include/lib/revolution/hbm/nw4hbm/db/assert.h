#ifndef NW4HBM_DB_ASSERT_H
#define NW4HBM_DB_ASSERT_H

#include "decomp.h"
#include "revolution/hbm/HBMAssert.hpp"

#include <stdarg.h>

namespace nw4hbm {
namespace db {

DECL_WEAK void VPanic(char const* file, int line, char const* fmt, va_list vlist);
DECL_WEAK void Panic(char const* file, int line, char const* fmt, ...);

DECL_WEAK void VWarning(char const* file, int line, char const* fmt, va_list vlist);
DECL_WEAK void Warning(char const* file, int line, char const* fmt, ...);

void Assertion_ShowConsole(unsigned long time);

} // namespace db
} // namespace nw4hbm

#endif
