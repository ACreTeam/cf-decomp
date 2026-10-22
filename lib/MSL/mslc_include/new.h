#ifndef MSL_NEW_H
#define MSL_NEW_H

#include <size_t.h>

inline void* operator new(size_t, void* ptr) { return ptr; }

#endif // MSL_NEW_H
