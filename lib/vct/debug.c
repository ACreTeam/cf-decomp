#include <revolution/os.h>
#include <stdarg.h>

#include "vct.h"
#include "vct_main.h"

static VCTReportLevel sReportLevel = VCT_REPORTLEVEL_ALL;

void VCT_SetReportLevel(VCTReportLevel level) {
    sReportLevel = level;
}

void VCTi_Report(const char* fmt, ...) {
    va_list args;

    if (sReportLevel == VCT_REPORTLEVEL_ALL) {
        OSReport("DWC_VCT     :");
        va_start(args, fmt);
        OSVReport(fmt, args);
        va_end(args);
    }
}

u32 VCTi_GetAudioDataSize(void);

u32 VCT_GetAudioDataSize(void) {
    return VCTi_GetAudioDataSize();
}

// Called with the result of PMIC_StartAsync; nothing is done with it.
void VCTi_NotifyMicStartResult(s32 result) {}
