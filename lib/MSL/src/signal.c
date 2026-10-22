#include <signal.h>
#include <stdlib.h>

sig_func signal_funcs[7];

int raise(int sig) {
    sig_func handler;
    u32 index;

    if (sig < 1 || sig > 7) {
        return -1;
    }

    index = sig - 1;
    handler = signal_funcs[index];
    if ((u32)handler != 1) {
        signal_funcs[index] = 0;
    }
    if ((u32)handler == 1 || ((int)handler == 0 && sig == 1)) {
        return 0;
    }
    if ((u32)handler == 0) {
        exit(0);
    }
    handler(sig);
    return 0;
}
