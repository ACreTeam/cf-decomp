#ifndef RVL_SDK_SO_SOBASIC_H
#define RVL_SDK_SO_SOBASIC_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SOLibraryConfig {
    void *(*alloc)(int name, u32 size);
    void (*free)(int name, void *ptr);
} SOLibraryConfig;

int SOInit(SOLibraryConfig *config);
int SOFinish(void);
int SOStartup(void);
int SOCleanup(void);

#ifdef __cplusplus
}
#endif
#endif
