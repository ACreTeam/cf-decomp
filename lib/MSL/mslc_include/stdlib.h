#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stddef.h>
#include <wchar_t.h>

#ifdef __cplusplus
extern "C" {
#endif

inline int _abs(int __x)
{
    return __x > 0 ? __x : -__x;
}

/* Integer arithmetic (C89 4.10.6). CodeWarrior lowers abs/labs to inline
   intrinsics, so the library forms are macros rather than calls. */
#ifndef abs
#define abs(x) __abs(x)
#endif
#ifndef labs
#define labs(x) __labs(x)
#endif

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);

/* String conversion (C89 4.10.1). */
double atof(const char* nptr);
int atoi(const char* nptr);
long atol(const char* nptr);
double strtod(const char* nptr, char** endptr);
long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);
int mbtowc(wchar_t* pwc, const char* s, size_t n);
size_t mbstowcs(wchar_t* pwcs, const char* s, size_t n);

/* Pseudo-random sequence generation (C89 4.10.2). */
#define RAND_MAX 32767

int rand(void);
void srand(unsigned int seed);

/* Memory management (C89 4.10.3). */
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);

/* Communication with the environment (C89 4.10.4). */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void abort(void);
void exit(int status);
char* getenv(const char* name);

/* Searching and sorting (C89 4.10.5). */
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size,
              int (*compar)(const void*, const void*));
typedef int (*_compare_function)(const void*, const void*);
void qsort(void* table_base, size_t num_members, size_t member_size,
           _compare_function compare_members);

#ifdef __cplusplus
}
#endif

#endif
