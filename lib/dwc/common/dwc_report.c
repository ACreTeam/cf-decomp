#include <dwc/dwc_report.h>

#include <revolution/types.h>
#include <stdarg.h>
#include <revolution/os.h>

static u32 sReportLevel = 0;

void DWC_SetReportLevel(u32 level) {
    sReportLevel = level;
}

void DWC_Printf(DWCReportFlag level, const char* fmt, ...) {
    va_list vlist;
    if ((level & sReportLevel) == 0) return;
    switch (level) {
        case DWC_REPORTFLAG_INFO:   OSReport("DWC_INFO     :"); break;
        case DWC_REPORTFLAG_ERROR:   OSReport("++DWC_ERROR  :"); break;
        case DWC_REPORTFLAG_DEBUG:   OSReport("DWC_DEBUG    :"); break;
        case DWC_REPORTFLAG_WARNING:   OSReport("DWC_WARN     :"); break;
        case DWC_REPORTFLAG_ACHECK:  OSReport("DWC_ACHECK   :"); break;
        case DWC_REPORTFLAG_LOGIN:  OSReport("DWC_LOGIN    :"); break;
        case DWC_REPORTFLAG_MATCH_NN:  OSReport("DWC_MATCH_NN :"); break;
        case DWC_REPORTFLAG_MATCH_GT2:  OSReport("DWC_MATCH_GT2:"); break;
        case DWC_REPORTFLAG_TRANSPORT: OSReport("DWC_TRANSPORT:"); break;
        case DWC_REPORTFLAG_QR2_REQ: OSReport("DWC_QR2_REQ  :"); break;
        case DWC_REPORTFLAG_SB_UPDATE: OSReport("DWC_SB_UPDATE:"); break;
        case DWC_REPORTFLAG_SEND_INFO:OSReport("DWC_SEND     :"); break;
        case DWC_REPORTFLAG_RECV_INFO:   OSReport("DWC_RECV     :"); break;
        case DWC_REPORTFLAG_UPDATE_SV:   OSReport("DWC_UPDATE_SV:"); break;
        case DWC_REPORTFLAG_CONNECTINET:   OSReport("DWC_CONNECTINET:"); break;
        case DWC_REPORTFLAG_AUTH: OSReport("DWC_AUTH     :"); break;
        case DWC_REPORTFLAG_AC: OSReport("DWC_AC       :"); break;
        case DWC_REPORTFLAG_BM: OSReport("DWC_BM       :"); break;
        case DWC_REPORTFLAG_UTIL: OSReport("DWC_UTIL     :"); break;
        case DWC_REPORTFLAG_OPTION_CF:OSReport("DWC_OPTION_CF:"); break;
        case DWC_REPORTFLAG_OPTION_CONNTEST: OSReport("DWC_OPTION_CONNTEST:"); break;
        case DWC_REPORTFLAG_GAMESPY:OSReport("DWC_GAMESPY  :"); break;

        default: OSReport("DWC_UNKNOWN  :"); break;
    }
    va_start(vlist, fmt);
    OSVReport(fmt, vlist);
    va_end(vlist);
}
