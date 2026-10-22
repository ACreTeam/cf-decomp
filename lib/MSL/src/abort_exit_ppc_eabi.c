#include "abort_exit.h"
#include "signal.h"
#include "stdlib.h"

void (*__stdio_exit)(void);
int __aborting;

void abort(void)
{
    raise(1);
    __aborting = 1;
    exit(1);
}
