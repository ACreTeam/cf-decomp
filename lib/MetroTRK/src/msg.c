#include <MetroTRK/msg.h>
#include <MetroTRK/dolphin_trk_glue.h>

DSError TRKMessageSend(TRKBuffer* msg)
{
    DSError write_err = TRKWriteUARTN(msg->data, msg->length);
    return kNoError;
}
