/* <assert.h> is deliberately not idempotent (C89 4.2): it is re-read on every
   inclusion so that NDEBUG can be redefined between them. */

#undef assert

#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#ifndef _MSL_ASSERT_H_
#define _MSL_ASSERT_H_

#ifdef __cplusplus
extern "C" {
#endif

void __assertion_failed(char const* expression, char const* file, int line);

#ifdef __cplusplus
}
#endif

#endif
#define assert(expression) \
    ((expression) ? (void)0 : __assertion_failed(#expression, __FILE__, __LINE__))
#endif
