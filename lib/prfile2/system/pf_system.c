#include <revolution/os.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_stub.h>

struct PF_SYS_DATE {
    u32 year;   // 0x00
    u32 month;  // 0x04
    u32 day;    // 0x08
};

struct PF_SYS_TIME {
    u32 hour;  // 0x00
    u32 min;   // 0x04
    u32 sec;   // 0x08
};

s32 pfstub_init_stub(void);

s32 PFSYS_initializeSYS(void) {
    return pfstub_init_stub();
}

s32 PFSYS_GetCurrentContextID(s32* p_context_id) {
    *p_context_id = pf_vol_set.context_id;
    return 0;
}

void PFSYS_TimeStamp(struct PF_SYS_DATE* p_date, struct PF_SYS_TIME* p_time) {
    OSCalendarTime ct;

    OSTicksToCalendarTime(OSGetTime(), &ct);
    p_date->year = ct.year;
    p_date->month = ct.mon + 1;
    p_date->day = ct.mday;
    p_time->hour = ct.hour;
    p_time->min = ct.min;
    p_time->sec = ct.sec;
}
