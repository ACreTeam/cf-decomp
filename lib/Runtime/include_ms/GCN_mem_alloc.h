#ifndef RUNTIME_GCN_MEM_ALLOC_H
#define RUNTIME_GCN_MEM_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void* __sys_alloc(unsigned int size);
void __sys_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_GCN_MEM_ALLOC_H
