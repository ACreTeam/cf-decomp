#ifndef ABORT_EXIT_H
#define ABORT_EXIT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void (*__stdio_exit)(void);
extern int __aborting;

#ifdef __cplusplus
}
#endif

#endif
