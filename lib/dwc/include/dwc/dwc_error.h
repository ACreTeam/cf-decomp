#pragma once

#include <revolution/types.h>

#include <dwc/dwc_error_fwd.h>

#define DWC_ECODE_SEQ_LOGIN      (-60000)
#define DWC_ECODE_SEQ_FRIEND     (-70000)
#define DWC_ECODE_SEQ_MATCH      (-80000)
#define DWC_ECODE_SEQ_ETC        (-90000)
#define DWC_ECODE_GS_GP          (-1000)
#define DWC_ECODE_GS_PERS        (-2000)
#define DWC_ECODE_GS_QR2         (-4000)
#define DWC_ECODE_GS_SB          (-5000)
#define DWC_ECODE_GS_NN          (-6000)
#define DWC_ECODE_GS_GT2         (-7000)
#define DWC_ECODE_TYPE_ALLOC     (-1)
#define DWC_ECODE_TYPE_PARAM     (-2)
#define DWC_ECODE_TYPE_SO_SOCKET (-3)
#define DWC_ECODE_TYPE_NETWORK   (-10)
#define DWC_ECODE_TYPE_SERVER    (-20)
#define DWC_ECODE_TYPE_DNS       (-30)
#define DWC_ECODE_TYPE_DATA      (-40)
#define DWC_ECODE_TYPE_SOCKET    (-50)
#define DWC_ECODE_TYPE_BIND      (-60)
#define DWC_ECODE_TYPE_TIMEOUT   (-70)
#define DWC_ECODE_TYPE_PEER      (-80)
#define DWC_ECODE_TYPE_CONN_OVER (-100)
#define DWC_ECODE_TYPE_STATS_AUTH (-200)
#define DWC_ECODE_TYPE_STATS_LOAD (-210)
#define DWC_ECODE_TYPE_STATS_SAVE (-220)
#define DWC_ECODE_TYPE_OTHER     (-410)
#define DWC_ECODE_TYPE_MUCH_FAILURE (-420)
#define DWC_ECODE_TYPE_SC_CL_FAIL (-430)
#define DWC_ECODE_TYPE_CLOSE     (-600)
#define DWC_ECODE_TYPE_TRANS_HEADER (-610)
#define DWC_ECODE_TYPE_TRANS_BODY   (-620)
#define DWC_ECODE_TYPE_UNEXPECTED   (-9)


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif
