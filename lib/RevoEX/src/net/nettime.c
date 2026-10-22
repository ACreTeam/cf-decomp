#include <revolution/net.h>
#include <revolution/nwc24.h>
#include <revolution/os.h>

#include <private/nwc24.h>
#include <private/os/OSTime.h>

static inline BOOL NETGetUniversalSeconds(s64* utcSec) {
    static s64 whenCached;
    NWC24Err err;

    if (whenCached == 0 || whenCached + OSSecondsToTicks(60) < __OSGetSystemTime()) {
        NWC24iSynchronizeRtcCounter(FALSE);
        whenCached = __OSGetSystemTime();
    }

    err = NWC24iGetUniversalTime(utcSec);
    if (err < 0) {
        return FALSE;
    }
    return TRUE;
}

BOOL NETGetUniversalCalendar(OSCalendarTime* time) {
    NWC24Date nwcDate;
    s64 utcSec;

    if (NETGetUniversalSeconds(&utcSec) && NWC24iEpochSecondsToDate(&nwcDate, utcSec) >= 0 &&
        NWC24iDateToOSCalendarTime(time, &nwcDate) >= 0) {
        return TRUE;
    }

    OSTicksToCalendarTime(OSGetTime(), time);
    return FALSE;
}
