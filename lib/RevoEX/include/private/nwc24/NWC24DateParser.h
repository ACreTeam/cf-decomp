#ifndef PRIVATE_NWC24_DATE_PARSER_H
#define PRIVATE_NWC24_DATE_PARSER_H

#include <revolution/types.h>

#include <revolution/nwc24/NWC24Err.h>
#include <revolution/nwc24/NWC24Types.h>
#include <revolution/os/OSTime.h>

#include <private/nwc24/NWC24Utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24i_SEC_MAX 60
#define NWC24i_MIN_MAX 60
#define NWC24i_HOUR_MAX 24
#define NWC24i_DAY_MIN 1
#define NWC24i_MONTH_MIN 1
#define NWC24i_MONTH_MAX 12
#define NWC24i_YEAR_MIN 1900
#define NWC24i_WEEK_DAY_MIN 1
#define NWC24i_WEEK_DAY_MAX 7
#define NWC24i_YEAR_DAY_MIN 1
#define NWC24i_YEAR_DAY_MAX 365

NWC24Err NWC24iDateToMinutes(s32* outMinutes, const NWC24Date* date);
NWC24Err NWC24iMinutesToDate(NWC24Date* date, s32 minutes);
NWC24Err NWC24iEpochSecondsToDate(NWC24Date* date, s64 timestamp);
NWC24Err NWC24iEpochSecondsToMinutes(s32* minutes, s64 timestamp);
NWC24Err NWC24iDateToOSCalendarTime(OSCalendarTime* time, const NWC24Date* date);

s32 ConvertDateToDays(u16 year, u16 month, u16 day);
void ConvertDaysToDate(u16* year, u8* month, u8* day, s32 days);

#ifdef __cplusplus
}
#endif

#endif  // PRIVATE_NWC24_DATE_PARSER_H
