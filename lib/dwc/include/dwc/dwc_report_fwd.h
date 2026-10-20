#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

void DWC_SetReportLevel(unsigned long level);
void DWC_Printf(unsigned long level, const char* format, ...);

#ifdef __cplusplus
}
#endif
