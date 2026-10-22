#ifndef METROTRK_DISPATCH_H
#define METROTRK_DISPATCH_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TRK_DISPATCH_CMD_CONNECT        1
#define TRK_DISPATCH_CMD_DISCONNECT     2
#define TRK_DISPATCH_CMD_RESET          3
#define TRK_DISPATCH_CMD_GETVERSION     4
#define TRK_DISPATCH_CMD_GETSUPPORTMASK 5
#define TRK_DISPATCH_CMD_OVERRIDE       7
#define TRK_DISPATCH_CMD_READMEM        16
#define TRK_DISPATCH_CMD_WRITEMEM       17
#define TRK_DISPATCH_CMD_READREGS       18
#define TRK_DISPATCH_CMD_WRITEREGS      19
#define TRK_DISPATCH_CMD_SETOPTION      23
#define TRK_DISPATCH_CMD_CONTINUE       24
#define TRK_DISPATCH_CMD_STEP           25
#define TRK_DISPATCH_CMD_STOP           26

    typedef struct TRKBuffer TRKBuffer;

    BOOL TRKDispatchMessage(TRKBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_DISPATCH_H
