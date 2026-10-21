#include <dwc/dwc_cfriend.h>
#include <dwc/dwc_memfunc.h>

// received messages waiting to be read
typedef struct tagDWCiCfMsgList {
    DWCiCfMsgFormat msg;
    struct tagDWCiCfMsgList* next;
} DWCiCfMsgList; // size 0x20

static DWCiCfMsgList* s_msglist;

DWCCfError DWCi_CfReset(void)
{
    DWCiCfMsgList* tmp;

    while (s_msglist != NULL)
    {
        tmp = s_msglist->next;
        DWC_Free(DWC_ALLOCTYPE_OPTION_CF, s_msglist, 0);
        s_msglist = tmp;
    }

    return DWC_CF_ERROR_NONE;
}
