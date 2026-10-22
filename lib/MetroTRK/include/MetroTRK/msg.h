#ifndef METROTRK_MSG_H
#define METROTRK_MSG_H

#include <MetroTRK/msgbuf.h>

#ifdef __cplusplus
extern "C"
{
#endif

    DSError TRKMessageSend(TRKBuffer* msg);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_MSG_H
