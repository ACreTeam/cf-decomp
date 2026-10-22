#ifndef METROTRK_USR_PUT_H
#define METROTRK_USR_PUT_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void usr_put_initialize(void);
BOOL usr_puts_serial(const char* msg);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_USR_PUT_H
