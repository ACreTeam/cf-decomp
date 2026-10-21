#ifndef RVL_SDK_NHTTP_NHTTP_H
#define RVL_SDK_NHTTP_NHTTP_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

int NHTTPStartup(void *(*alloc)(u32, int), void (*free)(void *), int priority);
void NHTTPCleanupAsync(int);
int NHTTPAddHeaderField(void *request, const char *name, const char *value);
int NHTTPAddPostDataAscii(void *request, const char *name, const char *value);
int NHTTPGetResultCode(void *request);
int NHTTPSetVerifyOption(void *request, int option);

#ifdef __cplusplus
}
#endif
#endif
