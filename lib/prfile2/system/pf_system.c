#include <revolution/os.h>
#include <prfile2/pf_api.h>
#include <prfile2/pf_stub.h>
#include <prfile2/pf_system.h>

PF_SYSTEM_SET pf_sys_set;

void PFSYS_initializeSYS(void) {
    pfstub_init_stub();
}

s32 PFSYS_GetCurrentContextID(s32* context_id) {
    *context_id = pf_vol_set.context_id;
    return 0;
}

void PFSYS_TimeStamp(PF_SYS_DATE* sdate, PF_SYS_TIME* stime) {
    OSCalendarTime ct;

    OSTicksToCalendarTime(OSGetTime(), &ct);
    sdate->sys_year = ct.year;
    sdate->sys_month = ct.mon + 1;
    sdate->sys_day = ct.mday;
    stime->sys_hour = ct.hour;
    stime->sys_min = ct.min;
    stime->sys_sec = ct.sec;
}
