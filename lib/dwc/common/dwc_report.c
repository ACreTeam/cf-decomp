#include <dwc/dwc_report.h>

#include <revolution/types.h>
#include <stdarg.h>
#include <stdio.h>

static u32 sReportLevel = 0;

void DWC_SetReportLevel(u32 level) {
    sReportLevel = level;
}

void DWC_Printf(u32 id, const char* format, ...) {
    va_list args;
    if ((id & sReportLevel) == 0) return;
    switch (id) {
        case 0x1:   printf("DWC_INFO     :"); break;
        case 0x2:   printf("++DWC_ERROR  :"); break;
        case 0x4:   printf("DWC_DEBUG    :"); break;
        case 0x8:   printf("DWC_WARN     :"); break;
        case 0x10:  printf("DWC_ACHECK   :"); break;
        case 0x20:  printf("DWC_LOGIN    :"); break;
        case 0x40:  printf("DWC_MATCH_NN :"); break;
        case 0x80:  printf("DWC_MATCH_GT2:"); break;
        case 0x100: printf("DWC_TRANSPORT:"); break;
        case 0x200: printf("DWC_QR2_REQ  :"); break;
        case 0x400: printf("DWC_SB_UPDATE:"); break;
        case 0x8000:printf("DWC_SEND     :"); break;
        case 0x10000:   printf("DWC_RECV     :"); break;
        case 0x20000:   printf("DWC_UPDATE_SV:"); break;
        case 0x40000:   printf("DWC_CONNECTINET:"); break;
        case 0x1000000: printf("DWC_AUTH     :"); break;
        case 0x2000000: printf("DWC_AC       :"); break;
        case 0x4000000: printf("DWC_BM       :"); break;
        case 0x8000000: printf("DWC_UTIL     :"); break;
        case 0x10000000:printf("DWC_OPTION_CF:"); break;
        case 0x80000000:printf("DWC_GAMESPY  :"); break;

        default: printf("DWC_UNKNOWN  :"); break;
    }
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
