#ifndef METROTRK_CC_GDEV_H
#define METROTRK_CC_GDEV_H


#include <private/os/OSInterrupt.h>
#include <MetroTRK/DebuggerDriver.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum UnkGdevEnum
    {
        GDEV_RESULT_10009 = -10009,
        GDEV_RESULT_10005 = -10005,
        GDEV_RESULT_10001 = -10001,
    } UnkGdevEnum;

    void OutputData(u8* arg0, s32 arg1);
    int gdev_cc_initialize(void* flagOut, __OSInterruptHandler handler);
    int gdev_cc_shutdown(void);
    int gdev_cc_open(void);
    int gdev_cc_close(void);
    int gdev_cc_read(u8* dest, int size);
    int gdev_cc_write(const u8* src, int size);
    int gdev_cc_pre_continue(void);
    int gdev_cc_post_stop(void);
    int gdev_cc_peek(void);
    int gdev_cc_initinterrupts(void);

#ifdef __cplusplus
}
#endif

#endif // METROTRK_CC_GDEV_H
